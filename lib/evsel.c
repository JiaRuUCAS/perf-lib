#include "util.h"
#include "evsel.h"
#include "evlist.h"
#include "array.h"
#include "pmu.h"
#include "threadmap.h"

struct prof_evsel *
prof_evsel__new(struct perf_event_attr *attr,
				const char *name)
{
	struct prof_evsel *evsel = NULL;

	evsel = zalloc(sizeof(struct prof_evsel));
	if (evsel == NULL) {
		LOG_ERROR("Failed to alloc memory for prof_evsel");
		return NULL;
	}

	evsel->name = strdup(name);

	if (attr)
		prof_evsel__init(evsel, attr, 0);

	return evsel;
}

void
prof_evsel__init(struct prof_evsel *evsel,
				struct perf_event_attr *attr, int idx)
{
	if (evsel == NULL)
		return;

	evsel->idx = idx;
	evsel->attr = *attr;
	evsel->evlist = NULL;
	evsel->is_open = false;
	INIT_LIST_HEAD(&evsel->node);
}

static void
__evsel__free_fd(struct prof_evsel *evsel)
{
	array__delete(evsel->fd);
	evsel->fd = NULL;
}

static int
__evsel__alloc_fd(struct prof_evsel *evsel, int nthreads)
{
	struct array *arr = NULL;
	int thread;

	arr = array__new(nthreads, sizeof(int));
	if (arr == NULL) {
		LOG_ERROR("Failed to create FD array");
		evsel->fd = NULL;
		return -ENOMEM;
	}

	// init
	evsel->fd = arr;
	for (thread = 0; thread < nthreads; thread++)
		FD(evsel, thread)->fd = -1;
	return 0;
}

void
prof_evsel__disable(struct prof_evsel *evsel, int nthreads)
{
	int thread;

	for (thread = 0; thread < nthreads; thread++) {
		if (FD(evsel, thread)->fd == -1)
			continue;
		ioctl(FD(evsel, thread)->fd, PERF_EVENT_IOC_DISABLE, 0);
	}
}

void
prof_evsel__enable(struct prof_evsel *evsel, int nthreads)
{
	int thread;

	for (thread = 0; thread < nthreads; thread++) {
		if (FD(evsel, thread)->fd == -1)
			continue;
		ioctl(FD(evsel, thread)->fd, PERF_EVENT_IOC_RESET, 0);
		ioctl(FD(evsel, thread)->fd, PERF_EVENT_IOC_ENABLE, 0);
	}
}

void
prof_evsel__close_fd(struct prof_evsel *evsel, int nthreads)
{
	int thread;
	struct prof_fd_info *info = NULL;

	prof_evsel__disable(evsel, nthreads);

	for (thread = 0; thread < nthreads; ++thread) {
		info = FD(evsel, thread);
		if (info->fd >= 0) {
			close(info->fd);
			info->fd = -1;
		}
	}
}

void
prof_evsel__delete(struct prof_evsel *evsel)
{
	__evsel__free_fd(evsel);
	thread_map__free(evsel->threads);
	if (evsel->name)
		free(evsel->name);
	free(evsel);
}

int
prof_evsel__open(struct prof_evsel *evsel,
				struct thread_map *threads)
{
	int nthread = 0, thread, pid = 0, err = 0;
	struct prof_fd_info *info = NULL;
	enum {
		NO_CHANGE, SET_TO_MAX, INCREASED_MAX,
	} set_rlimit = NO_CHANGE;

	if (threads == NULL)
		return -EINVAL;

	nthread = thread_map__nr(threads);

	if (evsel->fd == NULL &&
					__evsel__alloc_fd(evsel, nthread) < 0) {
		LOG_ERROR("Failed to create FD");
		return -ENOMEM;
	}

	for (thread = 0; thread < nthread; thread++) {
		pid = thread_map__pid(threads, thread);
		info = FD(evsel, thread);

		LOG_DEBUG("Open event %s %u %lu for pid %d",
						evsel->name,
						(unsigned int)evsel->attr.type,
						(unsigned long)evsel->attr.config,
						pid);
retry_open:
		info->fd = syscall(__NR_perf_event_open,
						&evsel->attr, pid, -1, -1, 0);
		if (info->fd < 0) {
			err = -errno;
			if (errno != 2)
				LOG_ERROR("sys_perf_event_open failed, errno %d",
								errno);
			goto try_fallback;
		}

		set_rlimit = NO_CHANGE;
	}

	evsel->is_open = true;
	return 0;

try_fallback:
	if (err == -EMFILE && set_rlimit < INCREASED_MAX) {
		struct rlimit l;
		int old_errno = errno;

		if (getrlimit(RLIMIT_NOFILE, &l) == 0) {
			if (set_rlimit == NO_CHANGE)
				l.rlim_cur = l.rlim_max;
			else {
				l.rlim_cur = l.rlim_max + 1000;
				l.rlim_max = l.rlim_cur;
			}

			if (setrlimit(RLIMIT_NOFILE, &l) == 0) {
				set_rlimit++;
				errno = old_errno;
				goto retry_open;
			}
		}
		errno = old_errno;
	}

	while (--thread >= 0) {
		if (FD(evsel, thread)->fd < 0)
			continue;
		ioctl(FD(evsel, thread)->fd, PERF_EVENT_IOC_DISABLE, 0);
		close(FD(evsel, thread)->fd);
		FD(evsel, thread)->fd = -1;
	}
	evsel->is_open = false;
	return err;
}

void
prof_evsel__close(struct prof_evsel *evsel, int nthreads)
{
	if (evsel->fd == NULL)
		return;

	prof_evsel__close_fd(evsel, nthreads);
	__evsel__free_fd(evsel);
}

/* Options:
 * - u: count in user space
 * - k: count in kernel space
 */
static void
__evsel__parse_opt(char *opt,
				struct perf_event_attr *attr)
{
	int i = 0, opt_len = 0;
	bool user = false, kernel = false;

	opt_len = strlen(opt);
	for (i = 0; i < opt_len; i++) {
		switch(opt[i]) {
		case 'u':
			user = true;
			break;
		case 'k':
			kernel = true;
			break;
		default:
			LOG_ERROR("Unknown argument %c", opt[i]);
		}
	}

	if (user && !kernel)
		attr->exclude_kernel = 1;
	else if (!user && kernel)
		attr->exclude_user = 1;
}

/* Usage: <event_name>:<opt1><opt2>... */
char *
prof_evsel__parse(const char *str, struct perf_event_attr *attr)
{
	char *s = NULL, *p = NULL;
	char *name = NULL, *opt = NULL;
	struct prof_pmu *pmu = NULL;

	s = strdup(str);
	p = strchr(s, ':');
	if (p != NULL) {
		if (strlen(p) > 1) {
			opt = p + 1;
		}
		*p = '\0';
	}

	name = s;
	pmu = prof_pmu__find(name);
	if (pmu == NULL) {
		free(s);
		LOG_ERROR("No event named %s", name);
		return NULL;
	}

	memset(attr, 0, sizeof(*attr));
	attr->type = pmu->type;
	attr->config = pmu->config;
	attr->size = sizeof(*attr);
	attr->disabled = 1;

	if (opt != NULL)
		__evsel__parse_opt(opt, attr);
	return name;
}

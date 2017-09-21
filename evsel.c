#include "util.h"
#include "evsel.h"
#include "evlist.h"
#include "array.h"
#include "pmu.h"
#include "threadmap.h"
#include "rdpmc.h"

#define FD(e, x) ((struct perf_fd_info*)array__entry(e->fd, x))


static struct {
	bool exclude_guest;
} perf_missing_features;


struct perf_evsel *perf_evsel__new(struct perf_event_attr *attr,
				const char *name)
{
	struct perf_evsel *evsel = NULL;

	evsel = zalloc(sizeof(struct perf_evsel));
	if (evsel == NULL) {
		LOG_ERROR("Failed to alloc memory for perf_evsel");
		return NULL;
	}

	evsel->name = strdup(name);
	if (attr != NULL)
		perf_evsel__init(evsel, attr, 0);
	return evsel;
}

void perf_evsel__init(struct perf_evsel *evsel,
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

static void perf_evsel__free_fd(struct perf_evsel *evsel)
{
	array__delete(evsel->fd);
	evsel->fd = NULL;
}

static int perf_evsel__alloc_fd(struct perf_evsel *evsel, int nthreads)
{
	struct array *arr = NULL;

	arr = array__new(nthreads, sizeof(struct perf_fd_info));
	if (arr == NULL) {
		LOG_ERROR("Failed to create FD array");
		evsel->fd = NULL;
		return -ENOMEM;
	}
	evsel->fd = arr;
	return 0;
}

void perf_evsel__disable(struct perf_evsel *evsel, int nthreads)
{
	int thread;

	for (thread = 0; thread < nthreads; thread++) {
		if (FD(evsel, thread)->fd == -1)
			continue;
		ioctl(FD(evsel, thread)->fd, PERF_EVENT_IOC_DISABLE, 0);
	}
}

void perf_evsel__enable(struct perf_evsel *evsel, int nthreads)
{
	int thread;

	for (thread = 0; thread < nthreads; thread++) {
		if (FD(evsel, thread)->fd == -1)
			continue;
		ioctl(FD(evsel, thread)->fd, PERF_EVENT_IOC_RESET, 0);
		ioctl(FD(evsel, thread)->fd, PERF_EVENT_IOC_ENABLE, 0);
	}
}

void perf_evsel__close_fd(struct perf_evsel *evsel, int nthreads)
{
	int thread;
	struct perf_fd_info *info = NULL;

	perf_evsel__disable(evsel, nthreads);

	for (thread = 0; thread < nthreads; ++thread) {
		info = FD(evsel, thread);
		if (info->mmap_addr != NULL)
			perf_rdpmc__delete(info->mmap_addr);
		close(info->fd);
		info->fd = -1;
	}
}

void perf_evsel__delete(struct perf_evsel *evsel)
{
	perf_evsel__free_fd(evsel);
	thread_map__free(evsel->threads);
	if (evsel->name)
		free(evsel->name);
	free(evsel);
}

int perf_evsel__open(struct perf_evsel *evsel, struct thread_map *threads)
{
	int nthread = 0, thread, pid = 0, err = 0;
	unsigned long flags = 0;
	struct perf_fd_info *info = NULL;
	enum {
		NO_CHANGE, SET_TO_MAX, INCREASED_MAX
	} set_rlimit = NO_CHANGE;

	if (threads == NULL)
		return -EINVAL;

	nthread = thread_map__nr(threads);

	if (evsel->fd == NULL &&
		perf_evsel__alloc_fd(evsel, nthread) < 0) {
		LOG_ERROR("Failed to create FD");
		return -ENOMEM;
	}

fallback_missing_features:
	if (perf_missing_features.exclude_guest)
		evsel->attr.exclude_guest = evsel->attr.exclude_host = 0;

//	LOG_INFO("open event %s %u %lu", evsel->name,
//					(unsigned int)evsel->attr.type,
//					(unsigned long)evsel->attr.config);
	for (thread = 0; thread < nthread; thread++) {
		pid = thread_map__pid(threads, thread);

//		LOG_INFO("pid %d", pid);
retry_open:
		info = FD(evsel, thread);
		info->fd = syscall(__NR_perf_event_open,
								&evsel->attr,
								pid, -1, -1, flags);
		if (info->fd < 0) {
			err = -errno;
			LOG_ERROR("sys_perf_event_open failed, errno %d", errno);
			goto try_fallback;
		}

		set_rlimit = NO_CHANGE;
#ifdef _PERF_EVSEL_USE_RDPMC_
		perf_rdpmc__init(info->fd, &(info->mmap_addr));
#else
		info->mmap_addr = NULL;
#endif
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


	if (err != -EINVAL || thread > 0)
		goto out_close;

	if (!perf_missing_features.exclude_guest &&
		   (evsel->attr.exclude_guest || evsel->attr.exclude_host)) {
		perf_missing_features.exclude_guest = true;
		goto fallback_missing_features;
	}

out_close:
	while (--thread >= 0) {
		ioctl(FD(evsel, thread)->fd, PERF_EVENT_IOC_DISABLE, 0);
		if (FD(evsel, thread)->mmap_addr != NULL)
			perf_rdpmc__delete(FD(evsel, thread)->mmap_addr);
		close(FD(evsel, thread)->fd);
		FD(evsel, thread)->fd = -1;
	}
	evsel->is_open = false;
	return err;
}

void perf_evsel__close(struct perf_evsel *evsel, int nthreads)
{
	if (evsel->fd == NULL)
		return;

	perf_evsel__close_fd(evsel, nthreads);
	perf_evsel__free_fd(evsel);
}

int perf_evsel__read(struct perf_evsel *evsel, int thread,
				uint64_t *count)
{
	struct perf_fd_info *info = FD(evsel, thread);

//	memset(count, 0, sizeof(uint64_t));

	if (info->fd < 0)
		return -EINVAL;

	if (info->mmap_addr != NULL) {
//		LOG_ERROR("RDPMC");
		perf_rdpmc__read(evsel, thread, count);
		return 0;
	} else {
//		LOG_ERROR("READN");
		if (readn(FD(evsel, thread)->fd, count, sizeof(uint64_t)) < 0)
			return -errno;
	}

	return 0;
}

//int perf_evsel__read(struct perf_evsel *evsel, uint64_t *count,
//				int nthreads, int nmembers)
//{
////	uint64_t *vals = NULL;
//	uint64_t nr = 0;
//
//	if (!perf_evsel__is_leader(evsel))
//		return -EINVAL;
//	else
//		LOG_INFO("leader %d, nthread %d, nmember %d",
//						FD(evsel, 0)->fd,
//						nthreads, nmembers);
//
//	if (readn(FD(evsel, 0)->fd, &nr, sizeof(uint64_t)) < 0) {
//		LOG_ERROR("errno %d", errno);
//		return -errno;
//	}
//
//	if (nthreads == 1 && nmembers == 1) {
//		count[0] = nr;
//		return 1;
//	}
//
//	if (nr <= 0 || nr > (nthreads * nmembers)) {
//		LOG_ERROR("Wrong nr of values %lu",
//						(unsigned long)nr);
//		return -ERANGE;
//	}
//
//	LOG_INFO("Read nr %lu", (unsigned long)nr);
//
//	if (readn(FD(evsel, 0)->fd, count, sizeof(uint64_t) * nr) < 0)
//		return -errno;
//
//	return (int)nr;
//}

/* Options:
 * - u: count in user space
 * - k: count in kernel space
 * - h: count in host mode
 * - g: count in guest mode
 */
static void __perf_evsel__parse_opt(char *opt,
				struct perf_event_attr *attr)
{
	int i = 0, opt_len = 0;
	bool user = false, kernel = false, host = false, guest = false;

	opt_len = strlen(opt);
	for (i = 0; i < opt_len; i++) {
		switch(opt[i]) {
		case 'u':
			user = true;
			break;
		case 'k':
			kernel = true;
			break;
		case 'h':
			host = true;
			break;
		case 'g':
			guest = true;
			break;
		default:
			LOG_ERROR("Unknown argument %c", opt[i]);
		}
	}

	if (user && !kernel)
		attr->exclude_kernel = 1;
	else if (!user && kernel)
		attr->exclude_user = 1;

	if (host && !guest)
		attr->exclude_guest = 1;
	else if (!host && guest)
		attr->exclude_host = 1;
}

/* Usage: <event_name>:<opt1><opt2>... */
char *perf_evsel__parse(const char *str, struct perf_event_attr *attr)
{
	char *s = NULL, *p = NULL;
	char *name = NULL, *opt = NULL;
	struct perf_pmu *pmu = NULL;

	s = strdup(str);
	p = strchr(s, ':');
	if (p != NULL) {
		if (strlen(p) > 1) {
			opt = p + 1;
		}
		*p = '\0';
	}

	name = s;
	pmu = perf_pmu__find(name);
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
//	attr->inherit = 1;

	if (opt != NULL)
		__perf_evsel__parse_opt(opt, attr);
	return name;
}

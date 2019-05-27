#include "util.h"
#include "evlist.h"
#include "evsel.h"
#include "array.h"
#include "builtin.h"

static struct prof_evlist *
__event_create(const char *evlist_str)
{
	struct prof_evlist *evlist = NULL;

	evlist = prof_evlist__new();
	if (evlist == NULL) {
		LOG_ERROR("Failed to create evlist");
		return NULL;
	}

	// add event
	if (prof_evlist__add_from_str(evlist, evlist_str) < 0) {
		LOG_ERROR("Wrong event list: %s", evlist_str);
		goto fail_destroy_evlist;
	}

	if (prof_evlist__create_threadmap(evlist, 0) < 0) {
		LOG_ERROR("Failed to create thread map");
		goto fail_destroy_evlist;
	}

	return evlist;

fail_destroy_evlist:
	prof_evlist__delete(evlist);

	return NULL;
}

static int
__try_mmap(struct prof_evlist *evlist)
{
	struct prof_evsel *evsel = NULL;
	int fd = -1;
	struct perf_event_mmap_page *pc;
	void *ptr = NULL;

	evsel = prof_evlist__first(evlist);
	fd = FD(evsel, 0)->fd;
	if (fd < 0) {
		LOG_ERROR("Failed to get fd of event %s", evsel->name);
		return -1;
	}

	ptr = mmap(NULL, page_size, PROT_READ, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED) {
		LOG_ERROR("Failed to mmap perf event, err %d", errno);
		ptr = NULL;
		return -1;
	}

	pc = (struct perf_event_mmap_page *)ptr;

	LOG_INFO("version %u, index %u, time_enabled %llu, cap %llx",
					pc->version, pc->index,
					pc->time_enabled, pc->capabilities);

	munmap(ptr, page_size);

	return 0;
}

int
cmd_test(int argc, const char **argv)
{
	struct prof_evlist *evlist = NULL;

	if (argc < 1)
		return 1;

	LOG_INFO("Command test: %s", argv[1]);

	evlist = __event_create(argv[1]);
	if (evlist == NULL) {
		LOG_ERROR("Failed to create events");
		return 1;
	}

	prof_evlist__dump(evlist);

	prof_evlist__start(evlist);

	__try_mmap(evlist);

	prof_evlist__stop(evlist);

	prof_evlist__delete(evlist);
	return 0;
}

#ifndef _PERF_EVSEL_H_
#define _PERF_EVSEL_H_

#include "util.h"
#include "list.h"

struct perf_evlist;
struct perf_evsel;
struct thread_map;

#define _PERF_EVSEL_USE_RDPMC_

struct perf_fd_info {
	int fd;
	void *mmap_addr;
};

struct perf_evsel {
	struct list_head node;
	struct perf_evlist *evlist;
	struct perf_event_attr attr;
	struct array *fd;
	struct thread_map *threads;
	int idx;
	bool is_open;
	char *name;

//	/* group */
//	int nr_members;
//	struct perf_evsel *leader;
};

struct perf_evsel *perf_evsel__new(
				struct perf_event_attr *attr, const char *name);

int perf_evsel__open(struct perf_evsel *evsel,
				struct thread_map *threads);

void perf_evsel__disable(struct perf_evsel *evsel, int nthreads);

void perf_evsel__enable(struct perf_evsel *evsel, int nthreads);

void perf_evsel__close(struct perf_evsel *evsel, int nthreads);

void perf_evsel__init(struct perf_evsel *evsel,
				struct perf_event_attr *attr, int idx);

void perf_evsel__delete(struct perf_evsel *evsel);

char *perf_evsel__parse(const char *str,
				struct perf_event_attr *attr);

int perf_evsel__read(struct perf_evsel *evsel, int thread,
				uint64_t *count);

#endif /* _PERF_EVSEL_H_ */

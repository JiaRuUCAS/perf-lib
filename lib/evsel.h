#ifndef _PROFILE_EVSEL_H_
#define _PROFILE_EVSEL_H_

#include "util.h"
#include "list.h"

struct prof_fd_info {
	int fd;
};

#define FD(e, x) ((struct prof_fd_info *)array__entry(e->fd, x))

struct prof_evlist;
struct prof_evsel;
struct thread_map;

struct prof_evsel {
	struct list_head node;
	struct prof_evlist *evlist;
	struct perf_event_attr attr;
	struct array *fd;
	struct thread_map *threads;

	bool is_open;
	int idx;
	char *name;
};

struct prof_evsel *prof_evsel__new(struct perf_event_attr *attr,
				const char *name);

void prof_evsel__init(struct prof_evsel *evsel,
				struct perf_event_attr *attr, int idx);

void prof_evsel__enable(struct prof_evsel *evsel, int nthread);
void prof_evsel__disable(struct prof_evsel *evsel, int nthread);

void prof_evsel__close_fd(struct prof_evsel *evsel, int nthread);

void prof_evsel__delete(struct prof_evsel *evsel);

int prof_evsel__open(struct prof_evsel *evsel,
				struct thread_map *threads);

void prof_evsel__close(struct prof_evsel *evsel, int nthreads);

char *prof_evsel__parse(const char *str,
				struct perf_event_attr *attr);

#endif /* _PROFILE_EVSEL_H_ */

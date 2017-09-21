#ifndef _PERF_EVLIST_H_
#define _PERF_EVLIST_H_

#define PERF_EVLIST__HLIST_BITS 8
#define PERF_EVLIST__HLIST_SIZE (1 << PERF_EVLIST__HLIST_BITS)

#include <stdio.h>
#include "util.h"

struct thread_map;
struct perf_evsel;
struct list_head;

struct perf_evlist {
	struct list_head entries;
//	struct hlist_head heads[PERF_EVLIST__HLIST_SIZE];

	int nr_entries;
//	bool overwrite;
//	bool enabled;
//	bool is_group;

	struct thread_map *threads;
	struct perf_evsel *selected;
};

struct perf_evlist *perf_evlist__new(void);

void perf_evlist__init(struct perf_evlist *evlist);

void perf_evlist__delete(struct perf_evlist *evlist);

int perf_evlist__add_from_str(struct perf_evlist *evlist,
				char *str);

void perf_evlist__dump(struct perf_evlist *evlist);

void perf_evlist__set_threads(struct perf_evlist *evlist,
				struct thread_map *threads);

void perf_evlist__start(struct perf_evlist *evlist);

void perf_evlist__stop(struct perf_evlist *evlist);

int perf_evlist__read_all(struct perf_evlist *evlist,
				uint64_t *vals);

int perf_evlist__counter_nr(struct perf_evlist *evlist);

/**
 * __evlist__for_each - iterate thru all the evsels
 * @list: list_head instance to iterate
 * @evsel: struct evsel iterator
 */
#define __evlist__for_each(list, evsel) \
        list_for_each_entry(evsel, list, node)

/**
 * evlist__for_each - iterate thru all the evsels
 * @evlist: evlist instance to iterate
 * @evsel: struct evsel iterator
 */
#define evlist__for_each(evlist, evsel) \
	__evlist__for_each(&(evlist)->entries, evsel)

/**
 * __evlist__for_each_continue - continue iteration thru all the evsels
 * @list: list_head instance to iterate
 * @evsel: struct evsel iterator
 */
#define __evlist__for_each_continue(list, evsel) \
        list_for_each_entry_continue(evsel, list, node)

/**
 * evlist__for_each_continue - continue iteration thru all the evsels
 * @evlist: evlist instance to iterate
 * @evsel: struct evsel iterator
 */
#define evlist__for_each_continue(evlist, evsel) \
	__evlist__for_each_continue(&(evlist)->entries, evsel)

/**
 * __evlist__for_each_reverse - iterate thru all the evsels in reverse order
 * @list: list_head instance to iterate
 * @evsel: struct evsel iterator
 */
#define __evlist__for_each_reverse(list, evsel) \
        list_for_each_entry_reverse(evsel, list, node)

/**
 * evlist__for_each_reverse - iterate thru all the evsels in reverse order
 * @evlist: evlist instance to iterate
 * @evsel: struct evsel iterator
 */
#define evlist__for_each_reverse(evlist, evsel) \
	__evlist__for_each_reverse(&(evlist)->entries, evsel)

/**
 * __evlist__for_each_safe - safely iterate thru all the evsels
 * @list: list_head instance to iterate
 * @tmp: struct evsel temp iterator
 * @evsel: struct evsel iterator
 */
#define __evlist__for_each_safe(list, tmp, evsel) \
        list_for_each_entry_safe(evsel, tmp, list, node)

/**
 * evlist__for_each_safe - safely iterate thru all the evsels
 * @evlist: evlist instance to iterate
 * @evsel: struct evsel iterator
 * @tmp: struct evsel temp iterator
 */
#define evlist__for_each_safe(evlist, tmp, evsel) \
	__evlist__for_each_safe(&(evlist)->entries, tmp, evsel)

#endif /* _PERF_EVLIST_H_ */

#include "util.h"
#include "list.h"
#include "evlist.h"
#include "evsel.h"
#include "threadmap.h"

struct perf_evlist *perf_evlist__new(void)
{
	struct perf_evlist *evlist = NULL;

	evlist = zalloc(sizeof(struct perf_evlist));
	if (evlist != NULL)
		perf_evlist__init(evlist);

	return evlist;
}

void perf_evlist__init(struct perf_evlist *evlist)
{
//	int i = 0;
//
//	for (i = 0; i < PERF_EVLIST__HLIST_SIZE; i++)
//		INIT_HLIST_HEAD(&evlist->heads[i]);
	INIT_LIST_HEAD(&evlist->entries);
}

void perf_evlist__delete(struct perf_evlist *evlist)
{
	struct perf_evsel *evsel = NULL, *pos = NULL;
	int nthreads = thread_map__nr(evlist->threads);
//	int n;

	evlist__for_each_reverse(evlist, evsel) {
		perf_evsel__close(evsel, nthreads);
	}

	thread_map__free(evlist->threads);
	evlist->threads = NULL;

	evsel = NULL;
	evlist__for_each_safe(evlist, evsel, pos) {
		list_del_init(&pos->node);
		pos->evlist = NULL;
		perf_evsel__delete(pos);
	}
	evlist->nr_entries = 0;
	free(evlist);
	evlist = NULL;
}

static int __perf_evlist__add(struct perf_evlist *evlist, char *str)
{
	struct perf_event_attr attr;
	char *name = NULL;
	struct perf_evsel *evsel = NULL;

	name = perf_evsel__parse(str, &attr);
	if (name == NULL) {
		LOG_ERROR("Failed to parse event %s", str);
	} else {
		evsel = perf_evsel__new(NULL, name);
		if (evsel == NULL) {
			LOG_ERROR("Failed to create perf_evsel for %s",
							name);
		} else {
			perf_evsel__init(evsel, &attr, evlist->nr_entries);
			evsel->evlist = evlist;
			list_add_tail(&evsel->node, &evlist->entries);
			evlist->nr_entries++;
			return 1;
		}
	}
	return 0;
}

//static void __perf_evlist__setup_group(struct perf_evlist *evlist)
//{
//	struct perf_evsel *evsel = NULL, *leader = NULL;
//
//	if (evlist->nr_entries <= 1)
//		return;
//
//	evlist__for_each(evlist, evsel) {
//		if (evsel->idx == 0) {
//			leader = evsel;
//		} else {
//			evsel->leader = leader;
//		}
//		evsel->attr.inherit = 0;
//		evsel->attr.read_format |= PERF_FORMAT_GROUP;
//		evsel->nr_members = evlist->nr_entries;
//	}
//	evlist->is_group = true;
//}

/* Usage: <event1>:<opt_list1>,<event2>:<opt_list2>,...*/
int perf_evlist__add_from_str(struct perf_evlist *evlist,
				char *str)
{
	char *pcur = NULL, *pnext = NULL, *tmp;
	int len = 0, added = 0;

	if (str == NULL || strlen(str) == 0)
		return -EINVAL;

	len = strlen(str);
	tmp = strdup(str);
	pcur = tmp;
	while ((pnext = strchr(pcur, ',')) != NULL) {
		*pnext = '\0';

		added += __perf_evlist__add(evlist, pcur);

		if ((pnext - tmp) == (len - 1))
			break;
		pcur = pnext + 1;
	}

	if ((pcur - tmp) < (len - 1))
		added += __perf_evlist__add(evlist, pcur);

	free(tmp);
	return added;
}

void perf_evlist__dump(struct perf_evlist *evlist)
{
	struct perf_evsel *evsel;

	evlist__for_each(evlist, evsel) {
		LOG_INFO("Event[%d] %s, type %u, config %lu",
						evsel->idx, evsel->name,
						evsel->attr.type,
						(unsigned long)evsel->attr.config);
	}
}

void perf_evlist__set_threads(struct perf_evlist *evlist,
				struct thread_map *threads)
{
	evlist->threads = threads;
}

static void perf_evlist__enable(struct perf_evlist *evlist,
				int nthreads)
{
	struct perf_evsel *evsel = NULL;

	evlist__for_each(evlist, evsel) {
		perf_evsel__enable(evsel, nthreads);
	}
}

void perf_evlist__start(struct perf_evlist *evlist)
{
	struct perf_evsel *evsel = NULL;
	int nthreads = 0;

	nthreads = thread_map__nr(evlist->threads);
	evlist__for_each(evlist, evsel) {
		if (!evsel->is_open &&
				perf_evsel__open(evsel, evlist->threads) < 0) {
			LOG_ERROR("Failed to open event %s", evsel->name);
			continue;
		}
	}

	perf_evlist__enable(evlist, nthreads);
}

void perf_evlist__stop(struct perf_evlist *evlist)
{
	struct perf_evsel *evsel = NULL;
	int nthreads = 0;

	nthreads = thread_map__nr(evlist->threads);
	evlist__for_each(evlist, evsel) {
		perf_evsel__disable(evsel, nthreads);
	}
}

int perf_evlist__counter_nr(struct perf_evlist *evlist)
{
//	int nthreads = 0;
//
//	nthreads = thread_map__nr(evlist->threads);
	return evlist->nr_entries;
}

int perf_evlist__read_all(struct perf_evlist *evlist, uint64_t *vals)
{
	struct perf_evsel *evsel = NULL;
	int nthreads = 0, i = 0;
	uint64_t val = 0;
//	struct perf_count_values count;

	nthreads = thread_map__nr(evlist->threads);

	memset(vals, 0, sizeof(uint64_t) * evlist->nr_entries);

	evlist__for_each(evlist, evsel) {
		for (i = 0; i < nthreads; i++) {
			perf_evsel__read(evsel, i, &val);
			vals[evsel->idx] += val;
		}
	}

	return evlist->nr_entries;
}

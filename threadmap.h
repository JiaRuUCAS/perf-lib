#ifndef _PERF_THREAD_MAP_H_
#define _PERF_THREAD_MAP_H_

#include <sys/types.h>
#include <stdio.h>

struct thread_map {
	int nr;
	pid_t map[];
};

struct thread_map *thread_map__new_dummy(void);
struct thread_map *thread_map__new(pid_t pid);
void thread_map__free(struct thread_map *map);
void thread_map__dump(struct thread_map *map);


static inline int thread_map__nr(struct thread_map *map)
{
	return map ? map->nr : 1;
}

static inline pid_t thread_map__pid(struct thread_map *map, int idx)
{
	return map->map[idx];
}

static inline void thread_map__set_pid(struct thread_map *map,
				int idx, pid_t pid)
{
	if (idx >= map->nr)
		return;

	map->map[idx] = pid;
}

#endif /* _PERF_THREAD_MAP_H_ */

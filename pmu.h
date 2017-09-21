#ifndef _PERF_PMU_H_
#define _PERF_PMU_H_

#include <stdint.h>
#include <linux/perf_event.h>

#define PERF_PMU_NAME_MAX 30

struct perf_pmu {
	char name[PERF_PMU_NAME_MAX];
	uint32_t type;
	uint64_t config;
	bool is_support;
};

struct perf_pmu *perf_pmu__find(char *str);

void perf_pmu__dump(void);

#endif /* _PERF_PMU_H_ */

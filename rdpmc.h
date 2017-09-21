#ifndef _PERF_RDPMC_H_
#define _PERF_RDPMC_H_

struct perf_evsel;

int perf_rdpmc__init(int fd, void **addr);

int perf_rdpmc__read(struct perf_evsel *evsel, int thread,
				uint64_t *count);

void perf_rdpmc__delete(void *addr);

#endif /* _PERF_RDPMC_H_ */

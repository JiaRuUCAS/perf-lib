#include "util.h"
#include "evsel.h"
#include "rdpmc.h"
#include "array.h"

static uint64_t rdpmc(unsigned int counter)
{
	unsigned int low, high;

	asm volatile("rdpmc" : "=a" (low), "=d" (high) : "c" (counter));

	return low | ((uint64_t)high) << 32;
}

static uint64_t mmap_read_self(void *addr, uint64_t *count)
{
	struct perf_event_mmap_page *pc = addr;
	uint32_t seq, idx;
	uint64_t ret = 0, val = 0;
//	uint64_t cyc = 0, time_offset = 0, enabled, running, delta;
//	uint32_t time_multi = 0, time_shift = 0;

	do {
		seq = pc->lock;
		barrier();

//		count->ena = pc->time_enabled;
//		count->run = pc->time_running;

		idx = pc->index;
//		count->val = pc->offset;
//		LOG_INFO("pc->offset %lu", (unsigned long)pc->offset);
//		if (enabled != running) {
//			cyc = rdtsc();
//			time_multi = pc->time_mult;
//			time_shift = pc->time_shift;
//			time_offset = pc->time_offset;
//		}

		if (pc->cap_user_rdpmc && idx) {
			ret = rdpmc(idx - 1);
//			LOG_INFO("pmc[%u] %lu", (unsigned int)(idx-1),
//							(unsigned long)ret);
			val += ret;
		}

		barrier();
	} while (pc->lock != seq);

//	if (enabled != running) {
//		uint64_t quot, rem;
//
//		quot = (cyc >> time_shift);
//		rem = cyc & ((1 << time_shift) - 1);
//		delta = time_offset + quot * time_multi +
//				((rem * time_multi) >> time_shift);
//
//		enabled += delta;
//		if (idx)
//			running += delta;
//
//		quot = count->val / running;
//		rem = count->val & running;
//		count->val = quot * enabled + (rem * enabled) / running;
//	}

	*count = val;
	return val;
}

int perf_rdpmc__init(int fd, void **addr)
{
	void *mmap_addr = NULL;
	struct perf_event_mmap_page *pc = NULL;

	if (fd == -1)
		return -EINVAL;

	mmap_addr = mmap(NULL, PAGE_SIZE, PROT_READ,
						MAP_SHARED, fd, 0);
	if (mmap_addr == MAP_FAILED) {
		LOG_ERROR("Failed to mmap fd %d", fd);
		*addr = NULL;
		return -ENOMEM;
	}

	pc = mmap_addr;
	if (!pc->cap_user_rdpmc) {
		LOG_INFO("No support for user space rdpmc");
		*addr = NULL;
		munmap(mmap_addr, PAGE_SIZE);
		return -ENOTSUP;
	}

	*addr = mmap_addr;
	return 0;
}

void perf_rdpmc__delete(void *addr)
{
	munmap(addr, PAGE_SIZE);
	addr = NULL;
}

int perf_rdpmc__read(struct perf_evsel *evsel, int thread,
				uint64_t *count)
{
	struct perf_fd_info *fd = NULL;

	fd = (struct perf_fd_info*)array__entry(evsel->fd, thread);

	if (fd == NULL || fd->fd == -1 || fd->mmap_addr == NULL)
		return -EINVAL;

	memset(count, 0, sizeof(*count));

	mmap_read_self(fd->mmap_addr, count);
	return 0;
}

#ifndef _PERF_UTIL_H_
#define _PERF_UTIL_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#define LOG_ERROR(format, ...) \
		fprintf(stderr, "[ERROR] %s %d: " format "\n", \
						__FILE__, __LINE__, ##__VA_ARGS__);

#define LOG_INFO(format, ...) \
		fprintf(stdout, "[INFO] %s %d: " format "\n", \
						__FILE__, __LINE__, ##__VA_ARGS__);

#define PAGE_SIZE 4096

static inline void *zalloc(size_t size)
{
	void *ptr = NULL;

	ptr = malloc(size);
	if (ptr == NULL)
		return NULL;

	memset(ptr, 0, size);
	return ptr;
}

static inline ssize_t readn(int fd, void *buf, size_t n)
{
	size_t left = n;

	while (left) {
		ssize_t ret = 0;

		ret = read(fd, buf, left);
		if (ret < 0 && errno == EINTR)
			continue;
		if (ret <= 0)
			return ret;

		left -= ret;
		buf += ret;
	}
	return n;
}

static inline unsigned int __roundup_2(unsigned int num)
{
	num--;

	num |= (num >> 1);
	num |= (num >> 2);
	num |= (num >> 4);
	num |= (num >> 8);
	num |= (num >> 16);
	num++;
	return num;
}

#define offsetof(type, member) ((size_t) & ((type*)0)->member)

#define container_of(ptr, type, member) ({ \
			const typeof(((type *)0)->member) *__mptr = (ptr); \
			(type*)((char*)__mptr - offsetof(type,member)); })

static inline unsigned long long rdtsc(void)
{
	unsigned hi, lo;

	asm volatile("rdtsc":"=a"(lo), "=d"(hi));
	return (((unsigned long long)lo) |
					(((unsigned long long)hi) << 32));
}

static inline void barrier(void)
{
	asm volatile("":::"memory");
}

#endif /* _PERF_UTIL_H_ */

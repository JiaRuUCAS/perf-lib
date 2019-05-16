#ifndef _PERF_PROFILE_UTIL_H_
#define _PERF_PROFILE_UTIL_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#ifndef __maybe_unused
#define __maybe_unused __attribute__((unused))
#endif

#ifndef __packed
#define __packed __attribute__((__packed__))
#endif

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#ifndef is_dir_sep
#define is_dir_sep(c) ((c) == '/')
#endif

#define LOG_ERROR(format, ...) \
		fprintf(stderr, "[ERROR] %s %d: " format "\n", \
						__FILE__, __LINE__, ##__VA_ARGS__);

#define LOG_INFO(format, ...) \
		fprintf(stdout, "[INFO] %s %d: " format "\n", \
						__FILE__, __LINE__, ##__VA_ARGS__);

#ifdef PF_DEBUG
#define LOG_DEBUG(format, ...) \
		fprintf(stderr, "[DEBUG] %s %d: " format "\n", \
						__FILE__, __LINE__, ##__VA_ARGS__);
#else
#define LOG_DEBUG(format, ...)
#endif

extern unsigned int page_size;
extern int cacheline_size;

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

/* For the buffer size of strerror_r */
#define STRERR_BUFSIZE	128

#define offsetof(type, member) ((size_t) & ((type*)0)->member)

#define container_of(ptr, type, member) ({ \
			const typeof(((type *)0)->member) *__mptr = (ptr); \
			(type*)((char*)__mptr - offsetof(type,member)); })

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#endif /* _PERF_PROFILE_UTIL_H_ */

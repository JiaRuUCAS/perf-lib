#include <stdio.h>
#include <errno.h>

#include "util.h"
#include "inst.h"

int
main(int argc __maybe_unused, char **argv __maybe_unused) {
	uint64_t start = 0, end = 0;

	start = rdtsc();
	usleep(10);
	end = rdtsc();

	fprintf(stdout, "%lu - %lu = %lu\n",
					end, start, (end - start));

	return 0;
}

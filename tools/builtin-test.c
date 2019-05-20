#include "util.h"
#include "pmu.h"
#include "builtin.h"

int
cmd_test(int argc, const char **argv)
{
	LOG_INFO("Command test");

	prof_pmu__dump();

	return 0;
}

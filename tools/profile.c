#include "util.h"
#include "cmd.h"

#include "builtin.h"

const char profile_usage_string[] =
	"profile [--help] COMMAND [ARGS]";

struct cmd_struct {
	const char *cmd;
	int (*fn)(int, const char **);
};

static struct cmd_struct commands[] = {
	{"test", cmd_test},
};

static void
__print_usage(void)
{
	unsigned int i;

	printf("\n Usage: %s\n", profile_usage_string);
	printf("\t COMMANDs: ");
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		struct cmd_struct *p = commands + i;

		printf("%s ", p->cmd);
	}
	putchar('\n');
}

static int
__handle_options(const char ***argv, int *argc)
{
	while (*argc > 0) {
		const char *cmd = (*argv)[0];

		if (cmd[0] != '-')
			break;

		if (!strcmp(cmd, "--help") || !strcmp(cmd, "-h")) {
			__print_usage();
			putchar('\n');
			exit(0);
		}
		else {
			LOG_ERROR("Unknown options: %s\n", cmd);
			exit(1);
		}
	}
	return 0;
}

static int
__run_builtin(struct cmd_struct *p, int argc, const char **argv)
{
	int status;
	struct stat st;
	char sbuf[STRERR_BUFSIZE];

	status = p->fn(argc, argv);
//	perf_env__exit(&perf_env);

	if (status)
		return status & 0xff;

	/* Somebody closed stdout? */
	if (fstat(fileno(stdout), &st))
		return 0;
	/* Ignore write errors for pipes and sockets.. */
	if (S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode))
		return 0;

	status = 1;
	/* Check for ENOSPC and EIO errors.. */
	if (fflush(stdout)) {
		LOG_ERROR("write failure on standard output: %s",
			strerror_r(errno, sbuf, sizeof(sbuf)));
		goto out;
	}
	if (ferror(stdout)) {
		LOG_ERROR("unknown write failure on standard output");
		goto out;
	}
	if (fclose(stdout)) {
		LOG_ERROR("close failed on standard output: %s",
			strerror_r(errno, sbuf, sizeof(sbuf)));
		goto out;
	}
	status = 0;
out:
	return status;
}

int
main(int argc, const char **argv)
{
	const char *cmd;
	unsigned int i = 0;

	page_size = sysconf(_SC_PAGE_SIZE);
	cacheline_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

	LOG_DEBUG("page size %u, cacheline size %d",
					page_size, cacheline_size);

	cmd__extract_argv0_path(argv[0]);

	/* looking for options */
	argv++;
	argc--;
	__handle_options(&argv, &argc);

	// execute command
	if (argc <= 0) {
		__print_usage();
		return 1;
	}
	else {
		cmd = argv[0];

		for (i = 0; i < ARRAY_SIZE(commands); i++) {
			struct cmd_struct *p = commands + i;

			if (strcmp(p->cmd, cmd))
				continue;
			exit(__run_builtin(p, argc, argv));
		}

		LOG_DEBUG("Unknown command: %s\n", cmd);
		__print_usage();
	}

	return 0;
}

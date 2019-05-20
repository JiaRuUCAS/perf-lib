#include "util.h"

#include "cmd.h"

static const char *argv0_path;


const char *
cmd__extract_argv0_path(const char *argv0)
{
	const char *slash;

	if (!argv0 || !*argv0)
		return NULL;
	slash = argv0 + strlen(argv0);

	while (argv0 < slash && !is_dir_sep(*slash))
		slash --;

	if (slash >= argv0) {
		argv0_path = strndup(argv0, slash - argv0);
		return argv0_path ? slash + 1 : NULL;
	}

	return argv0;
}

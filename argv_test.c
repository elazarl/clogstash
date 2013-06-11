#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argv.h"
#include "tap.h"

void argv_test() {
	argv_t args[] = { {"--test", "file1"}, {"--nice", "1"}, {"--null", NULL} , ARGV_T_END};
	ok1(strcmp(argv_get(args, "--test"), "file1") == 0);
	ok1(argv_get(args, "--test1") == NULL);

	if (1) {
		char *missing_argv[] = {"--test", "b"};
		char *argv[] = {"--test", "a", "--null", "hopa"};
		ok(!argv_parse(2, missing_argv, args), "missing null argument");
		ok(argv_parse(4, argv, args), "all required arguments set");
		ok1(strcmp(argv_get(args, "--test"), "a") == 0);
	}
}

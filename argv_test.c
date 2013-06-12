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
		/* TODO: find a way to test panics */
		char *argv[] = {"./a.out", "--test", "a", "--null", "hopa"};
		ok(argv_parse(5, argv, args), "all required arguments set");
		ok1(strcmp(argv_get(args, "--test"), "a") == 0);
	}
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "argv.h"
#include "panic.h"
#include "tap.h"

static jmp_buf j;

void jmponpanic() {
    longjmp(j, 1);
}

void argv_test() {
    argv_t args[] = { {"--test", "file1"}, {"--nice", "1"}, {"--null", NULL} , ARGV_T_END};
    ok1(strcmp(argv_get(args, "--test"), "file1") == 0);
    ok1(argv_get(args, "--test1") == NULL);

    if (1) {
        int hadpanic = 0;
        char *missing_argv[] = {"--test", "b"};
    	char *argv[] = {"./a.out", "--test", "a", "--null", "hopa"};
    	ok(argv_parse(5, argv, args), "all required arguments set");
    	ok1(strcmp(argv_get(args, "--test"), "a") == 0);
        onpanic = jmponpanic;
        if (setjmp(j) == 0) {
            argv_parse(2, missing_argv, args);
        } else {
            hadpanic = 1;
        }
        ok(hadpanic, "missing argument did not panic");
    }
    onpanic = NULL;
}

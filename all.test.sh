#!/bin/bash
#C_FILES=$(find . -maxdepth 1 -name \*.c|sed '/_test\.c/d;/tap\./d;/logcat\.c/d'| tr '\n' ' ')
WERROR="-Werror"
[ "$SLOPPY" = "y" ] && WERROR=""
C_FLAGS="$C_FLAGS -ggdb3 -Wall -Wextra $WERROR"
[ -f _main_test.c ] && rm _main_test.c
TEST_FILES=$(find . -maxdepth 1 -name \*_test.c)
for testfile in $TEST_FILES; do
	TESTFUN=$(basename $testfile .c)
	echo "void $TESTFUN();" >> _main_test.c
done
echo '
#include <string.h>
#include <stdlib.h>
#include "tap.h"
char **tests = NULL;
int testnr = 0;
int test_relevant(char *testname) {
    int i;
    for (i = 0; i < testnr; i++) {
        if (strcmp(tests[i], testname) == 0) {
            return 1;
        }
    }
    return testnr == 0;
}
int main(int argc, char **argv) {
    if (argc > 1) {
        tests = argv + 1;
        testnr = argc - 1;
    }' >>_main_test.c
for testfile in $TEST_FILES; do
	TESTFUN=$(basename $testfile .c)
        echo "if (test_relevant(\"$TESTFUN\")) $TESTFUN();" >> _main_test.c
done
echo 'return exit_status();}' >> _main_test.c
gcc $C_FLAGS $(python ./files.py tests)  -o _test_main.out|| exit -1
BASENAME="$(basename $0)"
if [ "$BASENAME" != "all.test.sh" ]; then
	RUNWITH=${BASENAME%%.test.sh}
	RUNWITH=${RUNWITH//__/ }
fi
$RUNWITH ./_test_main.out $@ || (echo TEST FAILED;exit -1) && echo SUCCESS
[ -z "$KEEP" ] && rm _test_main.out
rm _main_test.c

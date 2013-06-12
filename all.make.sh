C_FILES=$(find . -maxdepth 1 -name \*.c|sed /_test.c/d| tr '\n' ' ')
C_FLAGS="$C_FLAGS -g -Wall -Wextra -Werror"
find . -maxdepth 1 -name \*_test.c|while read testfile; do
	gcc $C_FLAGS $C_FILES $testfile -o $testfile.out|| exit -1
	$testfile.out || exit -1
	rm $testfile.out
done

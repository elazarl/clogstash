#ifndef __H_ARGV
#define __H_ARGV

#define ARGV_T_END {NULL, NULL}

typedef struct {
	char *arg;
	char *val;
} argv_t;

/**
 * parses argument lists.
 * Given
 *     argv_t args[] = { {"--input", ""}, {"--nice", "default"}, ARGV_T_END };
 * it will fill `args` according to input arguments;
 *     char *argv[] = {"--input", "file1", "--nice", "1"};
 *     argv_parse(4, argv, args);
 *     printf("i=%s, n=%s\n", args[0].val, args[1].val); // i=file1, n=1
 */
int argv_parse(int argc, char **argv, argv_t *args);

/**
 * Given argv_t structure, returns the value of a certain key
 *     argv_t args[] = { {"--input", "file1"} };
 *     printf("%s\n", argv_get(args)); // file1
 */
char *argv_get(argv_t *args, char *key);
#endif

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "argv.h"
#include "panic.h"

int argv_parse(int argc, char **argv, argv_t *args) {
	int i = 1;
	while (i < argc) {
		argv_t *p = args;
		for (; p->arg != NULL; p++) {
			if (strcmp(argv[i], p->arg) == 0) {
				i++;
				p->val = argv[i];
				break;
			}
		}
		if (p->arg == NULL) {
			panicf("Unexpected argument %s\n", argv[i]);
		}
		i++;
	}
	for (; args->arg != NULL; args++) {
		int has_null = 0;
		if (args->val == NULL) {
			panicf("required argument not given: %s\n", args->arg);
		}
		if (has_null) {
			return 0;
		}
	}
	return 1;
}

char *argv_get(argv_t *args, char *key) {
	argv_t *p = args;
	for (; p->arg != NULL; p++) {
		if (strcmp(key, p->arg) == 0) {
			return p->val;
		}
	}
	return NULL;
}

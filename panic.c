#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "panic.h"

void perrpanic(char *msg) {
    perror(msg);
    exit(1);
}

void panic(char *msg) {
    fputs(msg, stderr);
    exit(1);
}

void panicf(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputs("", stderr);
    va_end(ap);
}

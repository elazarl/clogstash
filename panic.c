#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include "panic.h"

void (*onpanic)(char *msg) = NULL;

static void dopanic(char *msg) {
    if (onpanic != NULL) {
        onpanic(msg);
    } else {
        fputs(msg, stderr);
        fputs("\n", stderr);
        fflush(stderr);
        exit(1);
    }
}

#define BUFSZ 1000
void perrpanic(char *msg) {
    char panicbuf[BUFSZ];
    snprintf(panicbuf, BUFSZ -1, "%s: %s", msg, strerror(errno));
    panicbuf[BUFSZ-1] = '\0';
    dopanic(panicbuf);
}

void panic(char *msg) {
    dopanic(msg);
}

void panicf(char *fmt, ...) {
    /* TODO: consider making thread local */
    char panicbuf[BUFSZ];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(panicbuf, BUFSZ, fmt, ap);
    va_end(ap);
    panicbuf[BUFSZ-1] = '\0';
    dopanic(panicbuf);
}

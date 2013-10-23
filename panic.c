#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include "panic.h"

void (*onpanic)(char *msg) = NULL;

static void dopanic(char *msg) {
    if (onpanic != NULL) {
        onpanic(msg);
    } else {
        char systembuf[200];
        fputs(msg, stderr);
        fputs("\n", stderr);
        fflush(stderr);
        snprintf(systembuf, sizeof(systembuf), "export T=$(mktemp /tmp/clogcat.XXXXXX);echo 'attach %d\nbt\nquit' >$T; gdb -quiet _test_main.out -x $T; rm $T", getpid());
        systembuf[sizeof(systembuf)-1] = '\0';
        system(systembuf);
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

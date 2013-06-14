#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "tap.h"
#include "panic.h"
#include "sig.h"

void sig_test() {
    char buf[1001];
    int sighupfd = sigfd(SIGHUP);
    unsigned char result;
    buf[1000] = '\0';
    snprintf(buf, 1000, "kill -HUP %d", getpid());
    if (system(buf) != 0) {
        panic("cannot send sig");
    }
    ok1(read(sighupfd, &result, 1) == 1);
    ok(result == SIGHUP, "%d <> %d", result, SIGHUP);
    ok1(read(sighupfd, &result, 1) == -1);
    ok1(errno == EWOULDBLOCK);
}

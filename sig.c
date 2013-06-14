#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "panic.h"
#include "sig.h"

static int signals[SIGUSR2 + 1];

void write_to_pipe(int sig) {
    unsigned char sigbyte = sig;
    write(signals[sig], &sigbyte, 1);
}

int sigfd(int sig) {
    int fds[2];
    if (sig > SIGUSR2 || signals[sig] != 0) {
        panicf("Illegal sig %d to sigfd, or sigfd called twice on same signal", sig);
    }
    if (pipe(fds) == -1) {
        perrpanic("sigfd.pipe");
    }
    if (fcntl(fds[0], F_SETFL, O_NONBLOCK) == -1) {
        perrpanic("fcntl:O_NONBLOCK");
    }
    signals[sig] = fds[1];
    signal(sig, write_to_pipe);
    return fds[0];
}

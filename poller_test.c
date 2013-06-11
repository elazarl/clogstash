#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "tap.h"
#include "poller.h"
#include "panic.h"

struct readpoller {
    char b[100];
    int read;
};
struct writepoller {
    char w[100];
    int maxw;
    int written;
    int chunksize;
};

int mylistener_read(struct poll_cb *cb) {
    struct readpoller *l = cb->data;
    int nr = read(cb->fd, l->b + l->read, 100-l->read);
    if (nr < 0) perrpanic("readpoller");
    if (nr == 0) {
        close(cb->fd);
        return POLLER_DEL;
    }
    l->read += nr;
    return POLLER_KEEP;
}

int mylistener_write(struct poll_cb *cb) {
    struct writepoller *l = cb->data;
    int writesize = l->maxw - l->written;
    int rc;
    if (writesize > l->chunksize) {
        writesize = l->chunksize;
    }
    rc = write(cb->fd, &l->w[l->written], writesize);
    if (rc == -1) perror("write");
    l->written += rc;
    if (l->written == l->maxw) {
        close(cb->fd);
        return POLLER_DEL;
    }
    return POLLER_KEEP;
}

static const struct readpoller emptyreadpoller;
static const struct writepoller emptywritepoller;

void test1() {
    struct poller *p = poller_new(100);
    int pipefds[2];
    struct readpoller rl = emptyreadpoller;
    struct writepoller wl = emptywritepoller;
    struct poll_cb rcb = poll_cb_new(), wcb = poll_cb_new();
    char *data = "supercalifregalistiexpialidoches";
    if (pipe(pipefds) != 0) perrpanic("pipe");
    if (fcntl(pipefds[0], F_SETFL, O_NONBLOCK) == -1) perrpanic("fcntl");
    if (fcntl(pipefds[1], F_SETFL, O_NONBLOCK) == -1) perrpanic("fcntl");
    wcb.fd = pipefds[1];
    wcb.data = &wl;
    wcb.write = &mylistener_write;
    rcb.fd = pipefds[0];
    rcb.data = &rl;
    rcb.read = &mylistener_read;
    strcpy(wl.w, data);
    wl.maxw = strlen(wl.w) + 1;
    wl.written = 0;
    wl.chunksize = 5;
    poller_add(p, rcb);
    poller_add(p, wcb);
    while (poller_poll(p, -1));
    poller_delete(p);

    ok1(strcmp(rl.b, data) == 0);
    if (strcmp(rl.b, data) != 0) {
        printf("    expected:\n   %s\n   got:\n   %s", data, rl.b);
    }
}

void poller_test() {
    test1();
}

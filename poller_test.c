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
    if (nr < 0) {
        return nr;
    }
    if (nr == 0) {
        close(cb->fd);
        cb->read = NULL;
        return nr;
    }
    l->read += nr;
    return nr;
}

int mylistener_write(struct poll_cb *cb) {
    struct writepoller *l = cb->data;
    int writesize = l->maxw - l->written;
    int nr;
    if (writesize > l->chunksize) {
        writesize = l->chunksize;
    }
    nr = write(cb->fd, &l->w[l->written], writesize);
    if (nr == -1) {
        return nr;
    }
    l->written += nr;
    if (l->written == l->maxw) {
        close(cb->fd);
        cb->write = NULL;
        return nr;
    }
    return nr;
}

static const struct readpoller emptyreadpoller;
static const struct writepoller emptywritepoller;

static void test1() {
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

static int socket_closed = 0;

static void close_socket(void *ctx) {
    int *socket = ctx;
    socket_closed = 1;
    close(*socket);
}

static void testEventScheduling() {
    struct poller *p = poller_new(100);
    int pipefds[2];
    struct poll_cb rcb = poll_cb_new();
    if (pipe(pipefds) != 0) perrpanic("pipe");
    if (fcntl(pipefds[0], F_SETFL, O_NONBLOCK) == -1) perrpanic("fcntl");
    if (fcntl(pipefds[1], F_SETFL, O_NONBLOCK) == -1) perrpanic("fcntl");
    rcb.fd = pipefds[0];
    poller_schedule(p, timespec_ms(10), &pipefds[0], close_socket);
    poller_add(p, rcb);
    while(poller_poll(p, -1));
    poller_delete(p);
    ok(socket_closed, "scheduling socket_close event worked");
}

void poller_test() {
    test1();
    testEventScheduling();
}

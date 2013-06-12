#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>

#include "argv.h"
#include "poller.h"
#include "panic.h"

#define MS_IN_SEC 1000

struct buf {
    unsigned char *buf;
    int len;
};

int buf_read(int fd, struct buf b) {
    return read(fd, b.buf, b.len);
}

int buf_write(int fd, struct buf b) {
    return write(fd, b.buf, b.len);
}

struct buf buf_slice_from(struct buf b, int from) {
    struct buf rv = { b.buf + from, b.len - from };
    if (from > b.len) {
        panicf("Cannot illegal slice_from %d > %d", from, b.len);
    }
    return rv;
}

struct buf buf_slice_to(struct buf b, int to) {
    struct buf rv = { b.buf, to };
    if (to > b.len) {
        panicf("Cannot illegal slice_to %d > %d", to, b.len);
    }
    return rv;
}

struct buf buf_copy(struct buf b) {
    struct buf rv = { b.buf, b.len };
    return rv;
}

struct buf buf_wrap(unsigned char *b, int len) {
    struct buf rv = { b, len };
    return rv;
}

int buf_empty(struct buf b) {
    return b.len == 0;
}

struct sourcesinkfds {
    int source;
    int sink;
};

struct copier {
    struct poller *p;
    struct sourcesinkfds sourcesink;

    struct buf raw_reader;
    struct buf raw_writer;

    struct buf reader;
    struct buf writer;
};

struct sourcesinkfds sourcesink(int source, int sink) {
    struct sourcesinkfds ssfds = { source, sink };
    return ssfds;
}

/**
 * copier_add adds to poller p functions that copies all data from fds.source to fds.sink
 */
struct copier copier_add(struct poller *p, struct sourcesinkfds fds, int bufsize) {
    unsigned char *buf1 = malloc(bufsize);
    unsigned char *buf2 = malloc(bufsize);
    struct copier c = { p, fds, buf_wrap(buf1, bufsize), buf_wrap(buf2, bufsize),
        buf_wrap(buf1, bufsize), buf_wrap(buf2, bufsize) };
    return c;
}

void copier_delete(struct copier c) {
    free(c.raw_reader.buf);
    free(c.raw_writer.buf);
}

int copier_read(struct poll_cb *cb) {
    struct copier *c = (struct copier *)cb->data;
    int nr = 0;
    if (buf_empty(c->reader)) {
        cb->read = NULL;
        return nr;
    }
    nr = buf_read(cb->fd, c->reader);
    if (nr < 0) {
        return nr;
    }
    c->reader = buf_slice_from(c->reader, nr);
    return nr;
}

int copier_write(struct poll_cb *cb) {
    struct copier *c = (struct copier *)cb->data;
    int nr;
    if (buf_empty(c->reader) && buf_empty(c->writer)) {
        cb->write = NULL;
        return nr;
    }
    if (buf_empty(c->writer)) {
        struct buf tmp;
        c->writer = buf_slice_to(c->raw_reader, c->raw_reader.len - c->reader.len);
        c->reader = c->raw_writer;
        tmp = c->raw_reader;
        c->raw_reader = c->raw_writer;
        c->raw_writer = tmp;
    }
    nr = buf_read(cb->fd, c->reader);
    if (nr < 0) {
        return nr;
    }
    c->reader = buf_slice_from(c->reader, nr);
    return nr;
}

int main(int argc, char **argv) {
    /* struct poller *p = poller_new(100); */
    argv_t args[] = { {"--output", NULL}, ARGV_T_END };
    char *outputfile;

    argv_parse(argc, argv, args);
    outputfile = argv_get(args, "--output");

    if (open(outputfile, O_APPEND | O_WRONLY) == -1) {
        perrpanic("open");
    }

    return 0;
}

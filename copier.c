#include <stdlib.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "copier.h"
#include "panic.h"
#include "poller.h"

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

struct sourcesinkfds sourcesink(int source, int sink) {
    struct sourcesinkfds ssfds = { source, sink };
    return ssfds;
}


int copier_delete(struct poll_cb *cb) {
    struct copier *c = (struct copier *)cb->data;
    free(c->raw_reader.buf);
    free(c->raw_writer.buf);
    free(c);
    return 0;
}

int copier_read(struct poll_cb *cb) {
    struct copier *c = (struct copier *)cb->data;
    int nr = 0;
    if (buf_empty(c->reader)) {
        poller_disable(c->p, c->sourcesink.source);
        return nr;
    }
    poller_enable(c->p, c->sourcesink.sink);
    nr = buf_read(cb->fd, c->reader);
    if (nr < 0) {
        perrpanic("copier_read");
        return nr;
    }
    if (nr == 0) {
        cb->read = NULL;
        c->closing = 1;
        return nr;
    }
    c->reader = buf_slice_from(c->reader, nr);
    return nr;
}

int copier_write(struct poll_cb *cb) {
    struct copier *c = (struct copier *)cb->data;
    int nr;
    if (buf_empty(c->writer)) {
        struct buf tmp;
        c->writer = buf_slice_to(c->raw_reader, c->raw_reader.len - c->reader.len);
        c->reader = c->raw_writer;
        tmp = c->raw_reader;
        c->raw_reader = c->raw_writer;
        c->raw_writer = tmp;
    }
    if (buf_empty(c->writer)) {
        if (c->closing) {
            cb->write = NULL;
        } else {
            poller_disable(c->p, c->sourcesink.sink);
            poller_enable(c->p, c->sourcesink.source);
        }
        return 0;
    }
    nr = buf_write(cb->fd, c->writer);
    if (nr < 0) {
        perrpanic("copier_write");
        return nr;
    }
    c->writer = buf_slice_from(c->writer, nr);
    return nr;
}

struct copier copier_add(struct poller *p, struct sourcesinkfds fds, int bufsize) {
    return copier_add_f(p, fds, bufsize, buf_read, buf_write);
}

struct copier copier_add_f(struct poller *p, struct sourcesinkfds fds, int bufsize, buf_reader br, buf_writer bw) {
    unsigned char *buf1 = malloc(bufsize);
    unsigned char *buf2 = malloc(bufsize);
    struct poll_cb readercb = poll_cb_new();
    struct poll_cb writercb = poll_cb_new();
    struct copier *pc = malloc(sizeof(*pc));
    struct copier c = { p, fds, br, bw, buf_wrap(buf1, bufsize), buf_wrap(buf2, bufsize),
        buf_wrap(buf1, bufsize), buf_wrap(buf2, 0), 0 };
    *pc = c;
    readercb.data = pc;
    readercb.fd = fds.source;
    readercb.read = copier_read;
    writercb.fd = fds.sink;
    writercb.data = pc;
    writercb.write = copier_write;
    writercb.cleanup = copier_delete;

    poller_add(p, readercb);
    poller_add(p, writercb);
    poller_disable(p, fds.sink);
    return c;
}

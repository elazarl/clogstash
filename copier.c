#include <stdlib.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "helpers.h"
#include "copier.h"
#include "panic.h"
#include "poller.h"

int copier_delete(struct poll_cb *cb) {
    struct copier *c = (struct copier *)cb->data;
    if (c->destroy != NULL) {
        c->destroy(c);
    }
    free(c->raw_reader.buf);
    free(c->raw_writer.buf);
    free(c);
    return 0;
}

int copier_read(struct poll_cb *cb) {
    struct copier *c = (struct copier *)cb->data;
    int nr = 0;
    if (buf_empty(c->reader)) {
        poller_disable(c->p, c->r.fd);
        return nr;
    }
    poller_enable(c->p, c->w.fd);
    nr = reader_read(c->r, c->reader);
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
            poller_disable(c->p, c->w.fd);
            poller_enable(c->p, c->r.fd);
        }
        return 0;
    }
    nr = writer_write(c->w, c->writer);
    if (nr < 0) {
        perrpanic("copier_write");
        return nr;
    }
    c->writer = buf_slice_from(c->writer, nr);
    return nr;
}

static struct reader constant_reader_maker_make(void *ctx) {
    struct reader r = *(struct reader *)ctx;
    return r;
}

struct reader_maker constant_reader_maker_new(struct reader r) {
    struct reader *pr = malloc(sizeof(*pr));
    struct reader_maker m = { pr, constant_reader_maker_make };
    *pr = r;
    return m;
}

static struct writer constant_writer_maker_make(void *ctx) {
    struct writer r = *(struct writer *)ctx;
    return r;
}

struct writer_maker constant_writer_maker_new(struct writer r) {
    struct writer *pr = malloc(sizeof(*pr));
    struct writer_maker m = { pr, constant_writer_maker_make };
    *pr = r;
    return m;
}

static void copier_static_maker_delete(struct copier *c) {
    free(c->rm.ctx);
    free(c->wm.ctx);
}

struct copier *copier_add_maker(struct poller *p, struct reader_maker rm, struct writer_maker wm, int bufsize) {
    char *buf1 = malloc(bufsize);
    char *buf2 = malloc(bufsize);
    struct poll_cb readercb = poll_cb_new();
    struct poll_cb writercb = poll_cb_new();
    struct copier *pc = malloc(sizeof(*pc));
    struct copier c = { p, rm, wm,
        reader_make(rm), writer_make(wm),
        buf_wrap(buf1, bufsize), buf_wrap(buf2, bufsize),
        buf_wrap(buf1, bufsize), buf_wrap(buf2, 0), 0, NULL };
    *pc = c;
    readercb.data = pc;
    readercb.fd = c.r.fd;
    readercb.read = copier_read;
    writercb.fd = c.w.fd;
    writercb.data = pc;
    writercb.write = copier_write;
    writercb.cleanup = copier_delete;

    poller_add(p, readercb);
    poller_add(p, writercb);
    poller_disable(p, c.w.fd);
    return pc;
}

struct copier *copier_add(struct poller *p, struct reader r, struct writer w, int bufsize) {
    struct reader_maker rm = constant_reader_maker_new(r);
    struct writer_maker wm = constant_writer_maker_new(w);
    struct copier *c = copier_add_maker(p, rm, wm, bufsize);
    c->destroy = copier_static_maker_delete;
    return c;
}

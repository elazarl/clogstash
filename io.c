#include <stdlib.h>

#include "io.h"
#include "helpers.h"

int reader_read(struct reader r, struct buf b) {
    return r.read(r.ctx, r.fd, b);
}

int writer_write(struct writer w, struct buf b) {
    return w.write(w.ctx, w.fd, b);
}

static int reader_read_buf(void *UNUSED(ctx), int fd, struct buf b) {
    return buf_read(fd, b);
}

static int writer_write_buf(void *UNUSED(ctx), int fd, struct buf b) {
    return buf_write(fd, b);
}

struct reader reader_wrap(int fd) {
    struct reader r = { NULL, fd, reader_read_buf };
    return r;
}

struct writer writer_wrap(int fd) {
    struct writer w = { NULL, fd, writer_write_buf };
    return w;
}

struct reader reader_make(struct reader_maker m) {
    return m.make(m.ctx);
}

struct writer writer_make(struct writer_maker m) {
    return m.make(m.ctx);
}


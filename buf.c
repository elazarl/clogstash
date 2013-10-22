#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "buf.h"
#include "panic.h"

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

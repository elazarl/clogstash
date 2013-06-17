#ifndef __H_IO_
#define __H_IO_

#include "buf.h"

typedef int (*buf_action)(void *ctx, int fd, struct buf b);

struct reader {
    void *ctx;
    int fd;
    buf_action read;
};

struct writer {
    void *ctx;
    int fd;
    buf_action write;
};

int reader_read(struct reader r, struct buf b);
int writer_write(struct writer w, struct buf b);

struct reader reader_make(int fd);
struct writer writer_make(int fd);

#endif

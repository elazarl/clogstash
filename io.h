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

struct reader reader_wrap(int fd);
struct writer writer_wrap(int fd);

struct reader_maker {
    void *ctx;
    struct reader_or_error (*make)(void *ctx);
};

struct reader_or_error {
    int error;
    struct reader reader;
};

struct writer_maker {
    void *ctx;
    struct writer_or_error (*make)(void *ctx);
};

struct writer_or_error {
    int error;
    struct writer writer;
};

struct reader_or_error reader_make(struct reader_maker m);

struct writer_or_error writer_make(struct writer_maker m);

#endif

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "reopen_writer.h"
#include "poller.h"
#include "timespec.h"

struct file_reopener {
    struct poller *p;
    struct writer_maker maker;
    struct timespec duration;
    int nerrors;
    int ms_wait;
};

static void file_reopener_open(void *ctx) {
    struct file_reopener *fr = ctx;
    int fd = open(fr->path, O_WRONLY|O_APPEND|O_CREAT);
    if (fd == -1) {
        fr->nerrors++;
        poller_schedule(fr->p, timespec_ms(fr->nerrors*10), fr, file_reopener_open);
    }
}

static int file_reopen_writer_write(void *ctx, int fd, struct buf b) {
    struct file_reopener *fr = (struct file_reopener *)ctx;
    int n = fd;
    if (n >= 0) {
        n = buf_write(fd, b);
    }
    if (n == -1) {
        fr->nerrors++;
        poller_schedule(fr->p, timespec_ms(fr->nerrors*10), fr, file_reopener_open);
    }
    return n;
}

struct writer reopen_writer_make(struct poller *p, struct writer_maker maker, struct timespec duration) {
    struct file_reopener fr_base = { p, maker, duration, 0, 0 };
    struct file_reopener *fr = malloc(sizeof(*fr));
    struct writer w = writer_make(maker);
    *fr = fr_base;
    poller_schedule(p, timespec_ms(fr->nerrors*10), fr, file_reopener_open);
    return w;
}


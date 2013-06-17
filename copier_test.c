#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "helpers.h"
#include "tap.h"
#include "io.h"
#include "copier.h"
#include "poller.h"
#include "panic.h"

static void simple_copier_test() {
    int src;
    int sink;
    struct poller *p = poller_new(1000);
    system("echo bobo >/tmp/test_copier");
    src = open("/tmp/test_copier", O_RDONLY);
    if (src < 0) {
        perrpanic("open src");
    }
    sink = open("/tmp/test_copier_out", O_CREAT|O_TRUNC|O_WRONLY, 0777);
    if (sink < 0) {
        perrpanic("open dst");
    }
    copier_add(p, reader_make(src), writer_make(sink), 1000);
    while (poller_poll(p, -1));
    ok1(system("[ \"`cat /tmp/test_copier_out`\" = 'bobo' ]") == 0);
    system("rm /tmp/test_copier /tmp/test_copier_out");
    close(src);
    close(sink);
    poller_delete(p);
}

static void emulate_full_buf() {
    int src;
    int sink;
    struct poller *p = poller_new(1000);
    system("echo 1234567890abcdefghijklmnopqrstuvwxyz >/tmp/test_copier_full_buf");
    src = open("/tmp/test_copier_full_buf", O_RDONLY);
    if (src < 0) {
        perrpanic("open src");
    }
    sink = open("/tmp/test_copier_full_buf_out", O_CREAT|O_TRUNC|O_WRONLY, 0777);
    if (sink < 0) {
        perrpanic("open dst");
    }
    copier_add(p, reader_make(src), writer_make(sink), 1000);
    while (poller_poll(p, -1));
    ok1(system("A=\"`cat /tmp/test_copier_full_buf_out`\";"
               " [ \"$A\" = '1234567890abcdefghijklmnopqrstuvwxyz' ] || (echo $A;false)") == 0);
    system("rm /tmp/test_copier_full_buf /tmp/test_copier_full_buf_out");
    close(src);
    close(sink);
    poller_delete(p);
}

static int a_reader(void *_count, int UNUSED(fd), struct buf b) {
    int *count = _count;
    if (*count < b.len) {
        struct buf tmp = { b.buf, *count };
        b = tmp;
    }
    memset(b.buf, 'a', b.len);
    return b.len;
}

static int buf_writer(void *_pb, int UNUSED(fd), struct buf b) {
    struct buf *pb = _pb;
    if (pb->len < b.len) panic("out of memory");
    memmove(pb->buf, b.buf, b.len);
    *pb = buf_slice_from(*pb, b.len);
    return b.len;
}

struct writer buf_writer_make(struct buf *pb) {
    int devnull = open("/dev/null", O_RDONLY);
    struct writer w = { pb, devnull, buf_writer };
    if (devnull == -1) {
        perrpanic("open /dev/null");
    }
    return w;
}

struct reader a_reader_make(int *counter) {
    int devzero = open("/dev/zero", O_RDONLY);
    struct reader r = { counter, devzero, a_reader };
    if (devzero == -1) {
        perrpanic("open /dev/zero");
    }
    return r;
}

void custom_reader() {
    struct poller *p = poller_new(1000);
    int counter = 13;
    char buf[1000];
    struct buf b = buf_wrap((void *)buf, 1000);
    copier_add(p, a_reader_make(&counter), buf_writer_make(&b), 1000);
    while (poller_poll(p, -1));
    ok1(strcmp(buf, "aaaaaaaaaaaaa") == 0);
    poller_delete(p);
}

void copier_test() {
    if (0) {simple_copier_test();
    emulate_full_buf();}
    custom_reader();
}


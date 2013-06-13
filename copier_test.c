#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "tap.h"
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
    copier_add(p, sourcesink(src, sink), 1000);
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
    copier_add(p, sourcesink(src, sink), 4);
    while (poller_poll(p, -1));
    ok1(system("A=\"`cat /tmp/test_copier_full_buf_out`\";"
               " [ \"$A\" = '1234567890abcdefghijklmnopqrstuvwxyz' ] || (echo $A;false)") == 0);
    system("rm /tmp/test_copier_full_buf /tmp/test_copier_full_buf_out");
    close(src);
    close(sink);
    poller_delete(p);
}

void copier_test() {
    simple_copier_test();
    emulate_full_buf();
}


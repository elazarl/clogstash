#include "reopen_writer.h"
#include "mock_test.h"
#include "io.h"
#include "buf.h"
#include "poller.h"
#include "timespec.h"
#include "tap.h"
#include "panic.h"
#include "copier.h"
#include "helpers.h"
#include <fcntl.h>
#include <string.h>

#include <stdio.h>

static void closer(void *ctx) {
    int *fd = ctx;
    puts("Closing");
    if (close(*fd) != 0) perrpanic("can't close");
}
/*static void sched_write(void *ctx) {
    struct writer *pw = ctx;
    writer_write(*pw, buf_from_str("hohoho"));
}*/

static void reopen_writer_test_1() {
    struct poller *p = poller_new(1000);
    int pipefds[2];
    char *msg = "message";
    struct writer w = file_reopen_writer_make(p, "/tmp/reopen_test", timespec_ms(10));
    if (pipe(pipefds) != 0) perrpanic("pipe");
    if (fcntl(pipefds[0], F_SETFL, O_NONBLOCK) == -1) perrpanic("fcntl");
    if (fcntl(pipefds[1], F_SETFL, O_NONBLOCK) == -1) perrpanic("fcntl");
    if (write(pipefds[1], msg, strlen(msg)) != (int)strlen(msg)) perrpanic("write asyn pipe"); /* make sure pipe never blocks */
    poller_schedule(p, timespec_ms(10), &pipefds[1], closer);
    copier_add(p, reader_wrap(pipefds[0]), w, 1000);
    while (poller_poll(p, -1));
    poller_delete(p);
}

void reopen_writer_test() {
    reopen_writer_test_1();
    /*struct poller *p = poller_new(1000);
    struct writer w = file_reopen_writer_make(p, "/tmp/reopen_test", timespec_ms(10));
    poller_schedule(p, timespec_ms(10), &w, sched_write);
    while (poller_poll(p, -1));*/
}

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "copier.h"
#include "poller.h"
#include "panic.h"

void copier_test() {
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
    system("rm /tmp/test_copier");
    close(src);
    close(sink);
}

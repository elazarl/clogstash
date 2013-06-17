#ifndef __H_COPIER_
#define __H_COPIER_

#include "io.h"

struct sourcesinkfds {
    int source;
    int sink;
};

struct copier {
    struct poller *p;

    struct reader r;
    struct writer w;

    struct buf raw_reader;
    struct buf raw_writer;

    struct buf reader;
    struct buf writer;
    int closing;
};

struct sourcesinkfds sourcesink(int source, int sink);

/**
 * copier_add adds to poller p functions that copies all data from fds.source to fds.sink
 */
struct copier copier_add(struct poller *p, struct reader r, struct writer w, int bufsize);

#endif

#ifndef __H_COPIER_
#define __H_COPIER_

#include "io.h"

struct copier {
    struct poller *p;

    struct reader_maker rm;
    struct writer_maker wm;

    struct reader r;
    struct writer w;

    struct buf raw_reader;
    struct buf raw_writer;

    struct buf reader;
    struct buf writer;
    int closing;

    void (*destroy)(struct copier *);
};

/**
 * copier_add adds to poller p functions that copies all data from fds.source to fds.sink
 */
struct copier *copier_add(struct poller *p, struct reader r, struct writer w, int bufsize);

struct copier *copier_add_maker(struct poller *p, struct reader_maker rm, struct writer_maker wm, int bufsize);

#endif

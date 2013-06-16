#ifndef __H_COPIER_
#define __H_COPIER_

struct buf {
    unsigned char *buf;
    int len;
};

struct sourcesinkfds {
    int source;
    int sink;
};

/**
 * A buf_writer is a function that writes to struct buf b
 */
typedef int (*copier_writer)(void *ctx, int fd, struct buf b);
/**
 * A buf_readr is a function that reads from struct buf b
 */
typedef int (*copier_reader)(void *ctx, int fd, struct buf b);

struct copier {
    struct poller *p;
    struct sourcesinkfds sourcesink;

    void *ctx;
    copier_reader cr;
    copier_writer cw;

    struct buf raw_reader;
    struct buf raw_writer;

    struct buf reader;
    struct buf writer;
    int closing;
};

/**
 * reads from fd to buffer, returns number of bytes read or -1 on error.
 */
int buf_read(int fd, struct buf b);

/**
 * reads from buffer to fd, returns number of bytes written or -1 on error.
 */
int buf_write(int fd, struct buf b);

struct buf buf_slice_from(struct buf b, int from);
struct buf buf_slice_to(struct buf b, int to);
struct buf buf_copy(struct buf b);
struct buf buf_wrap(unsigned char *b, int len);
int buf_empty(struct buf b);
struct sourcesinkfds sourcesink(int source, int sink);

/**
 * copier_add adds to poller p functions that copies all data from fds.source to fds.sink
 */
struct copier copier_add(struct poller *p, struct sourcesinkfds fds, int bufsize);

/**
 * copier_add_f adds to poller p functions that copies all data from fds.source to fds.sink,
 * the read and write functions to be used on the fd are br and bw respectively.
 */
struct copier copier_add_f(struct poller *p, struct sourcesinkfds fds, int bufsize, void *ctx, copier_reader br, copier_writer bw);

#endif

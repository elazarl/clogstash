#ifndef __H_BUF_
#define __H_BUF_
struct buf {
    unsigned char *buf;
    int len;
};

struct buf buf_slice_from(struct buf b, int from);
struct buf buf_slice_to(struct buf b, int to);
struct buf buf_copy(struct buf b);
struct buf buf_wrap(unsigned char *b, int len);
int buf_empty(struct buf b);


/**
 * reads from fd to buffer, returns number of bytes read or -1 on error.
 */
int buf_read(int fd, struct buf b);

/**
 * reads from buffer to fd, returns number of bytes written or -1 on error.
 */
int buf_write(int fd, struct buf b);
#endif

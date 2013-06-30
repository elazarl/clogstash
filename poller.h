#ifndef __H_POLLER_
#define __H_POLLER_

#include "timespec.h"

struct poll_cb {
    int fd;
    int (*write)(struct poll_cb *);
    int (*read)(struct poll_cb *);
    int (*cleanup)(struct poll_cb *);
    void *data;
};

struct poll_cb poll_cb_new();

struct poller;

/**
 * polls all fds in poller once, calls respective callbacks.
 * It is not performance oriented, expect slowness with large number of FDs.
 * Usual usage:
 *     poller *p = poller_new(MAX_FDS);
 *     poller_add(p, poll_cb{fd, write_cb1, write_cb2});
 *     while (poll_cb(poller));
 *     poller_delete(p);
 */
int poller_poll(struct poller *p, int ms);

/**
 * poller_new allocates a new poller instance with maxsize fds to poll.
 * Must call poller_delete after poller_new.
 */
struct poller *poller_new(int maxsize);

/**
 * poller_delete frees resources taken by poller.
 */
void poller_delete(struct poller *p);

/**
 * poller_add adds an fd to poll to poller instance
 */
void poller_add(struct poller *p, struct poll_cb cb);

/**
 * poller_disable will not poll for events for the given fd.
 */
void poller_disable(struct poller *p, int fd);

/**
 * poller_enable will start polling for events for the given fd,
 * relevant if this fd was disabled.
 */
void poller_enable(struct poller *p, int fd);

/**
 * poller_replace_fd will change the filedescriptor of this particular event.
 */
void poller_change_fd(struct poller *p, int oldfd, int newfd);

void poller_schedule(struct poller *p, struct timespec ts_delta, void *ctx, void (*action)());
#endif

#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "panic.h"
#include "poller.h"

struct poller {
    int max;
    int n;
    struct pollfd *pollfds;
    struct poll_cb cbs[];
};

int poller_poll(struct poller *p, int ms) {
	int rc = poll(p->pollfds, p->n, ms);
	int i;
	switch (rc) {
	case EAGAIN:
	case EFAULT:
	case EINTR:
	case EINVAL:
		return 0;
	}
	short anyleft = 0;
	for (i = 0; i < p->n; i++) {
		if (p->pollfds[i].revents & POLLOUT) {
			if (p->cbs[i].write(&p->cbs[i]) == POLLER_DEL) {
				p->pollfds[i].events &= ~((short)POLLOUT);
			}
		}
		if (p->pollfds[i].revents & POLLIN) {
			if (p->cbs[i].read(&p->cbs[i]) == POLLER_DEL) {
				p->pollfds[i].events &= ~((short)POLLIN);
			}
		}
		anyleft |= p->pollfds[i].events;
	}
	return anyleft != 0;
}

const struct poll_cb emptycb;

struct poll_cb poll_cb_new() {
	return emptycb;
}

struct poller *poller_new(int maxsize) {
    struct poller *p = (struct poller *) calloc(1, sizeof(struct poller) + 
		    sizeof(struct poll_cb) * maxsize);
    p->pollfds = calloc(maxsize, sizeof(struct pollfd));
    p->max = maxsize;
    return p;
}

void poller_add(struct poller *p, struct poll_cb cb) {
    if (p->n == p->max) {
        panicf("poller_add >max, %d > %d", p->n, p->max);
    }
    p->cbs[p->n] = cb;
    p->pollfds[p->n].fd = cb.fd;
    if (cb.read != NULL) {
        p->pollfds[p->n].events |= POLLIN;
    }
    if (cb.write != NULL) {
        p->pollfds[p->n].events |= POLLOUT;
    }
    p->n++;
}

void poller_delete(struct poller *p) {
    free(p->pollfds);
    free(p);
}


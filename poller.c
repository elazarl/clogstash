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
            p->cbs[i].write(&p->cbs[i]);
			if (p->cbs[i].write == NULL) {
				p->pollfds[i].events &= ~((short)POLLOUT);
			}
		}
		if (p->pollfds[i].revents & POLLIN) {
            p->cbs[i].read(&p->cbs[i]);
			if (p->cbs[i].read == NULL) {
				p->pollfds[i].events &= ~((short)POLLIN);
			}
		}
        if (p->cbs[i].write == NULL && p->cbs[i].read == NULL && p->cbs[i].cleanup != NULL) {
            p->cbs[i].cleanup(&p->cbs[i]);
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

static int poller_search(struct poller *p, int fd) {
    int i;
    for (i=0; i < p->n; i++) {
        if (p->cbs[i].fd == fd) {
            return i;
        }
    }
    panicf("Searched for nonexistant fd %d in poller", fd);
    return -1;
}

void poller_disable(struct poller *p, int fd) {
    int i = poller_search(p, fd);
    p->pollfds[i].events = 0;
}

static void update_cb(struct poller *p, int i) {
    if (p->cbs[i].read != NULL) {
        p->pollfds[i].events |= POLLIN;
    }
    if (p->cbs[i].write != NULL) {
        p->pollfds[i].events |= POLLOUT;
    }
}

void poller_enable(struct poller *p, int fd) {
    int i = poller_search(p, fd);
    update_cb(p, i);
}

static const struct pollfd emptypollfd;

void poller_add(struct poller *p, struct poll_cb cb) {
    if (p->n == p->max) {
        panicf("poller_add >max, %d > %d", p->n, p->max);
    }
    p->cbs[p->n] = cb;
    p->pollfds[p->n] = emptypollfd;
    p->pollfds[p->n].fd = cb.fd;
    update_cb(p, p->n);
    p->n++;
}

void poller_delete(struct poller *p) {
    free(p->pollfds);
    free(p);
}


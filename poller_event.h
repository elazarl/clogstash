#ifndef __H_POLLER_EVENT_
#define __H_POLLER_EVENT_

/**
 * poller_event describes an event that should happen in the future.
 * ms is the number of ms after which the event should occur.
 * ctx is the context in which
 * action should happen.
 */
struct poller_event {
    int ms;
    void *ctx;
    void (*action)(void *ctx);
};

struct poller_events {
    int max;
    int n;
    struct poller_event events[];
};

struct poller_events *poller_events_new(int size);

void poller_events_delete(struct poller_events *list);

/**
 * poller_events_add adds an event to the event list
 */
void poller_events_add(struct poller_events *list, struct poller_event evt);

/**
 * poller_event_pop returns the event with earliest schedule time, and removes it from queue.
 */
struct poller_event poller_events_pop(struct poller_events *list);

/**
 * poller_event_pop returns the event with earliest schedule time.
 */
struct poller_event poller_events_peek(struct poller_events *list);

#endif

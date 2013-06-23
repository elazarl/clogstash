#include <stdlib.h>

#include "poller_event.h"
#include "panic.h"

struct poller_events *poller_events_new(int size) {
    struct poller_events *list = malloc(sizeof(struct poller_events) + sizeof(struct poller_event)*size);
    list->max = size;
    list->n   = 0;
    return list;
}

void poller_events_delete(struct poller_events *list) {
    free(list);
}

static int sort_by_ms(const void *lhs, const void *rhs) {
    struct poller_event *e1 = (struct poller_event *)lhs;
    struct poller_event *e2 = (struct poller_event *)rhs;
    return e2->ms - e1->ms;
}

void poller_events_add(struct poller_events *list, struct poller_event evt) {
    if (list->n == list->max) {
        panicf("too much events added %d", list->max);
    }
    list->events[list->n] = evt;
    list->n++;
    qsort(list->events, list->n, sizeof(struct poller_event), sort_by_ms);
}

struct poller_event poller_events_peek(struct poller_events *list) {
    return list->events[list->n-1];
}

struct poller_event poller_events_pop(struct poller_events *list) {
    struct poller_event evt = poller_events_peek(list);
    list->n--;
    return evt;
}

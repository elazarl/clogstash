#include <stdlib.h>
#include <string.h>
#include "poller_event.h"
#include "tap.h"

static struct poller_event evt(int ms, char *str) {
    struct poller_event evt = { ms, str, NULL };
    return evt;
}

static void simple() {
    struct poller_events *l = poller_events_new(10);
    poller_events_add(l, evt(10, "ten"));
    poller_events_add(l, evt(11, "eleven"));
    poller_events_add(l, evt(9, "nine"));
    poller_events_add(l, evt(10, "ten2"));
    ok1(poller_events_pop(l).ms == 9);
    ok1(poller_events_pop(l).ms == 10);
    ok1(poller_events_pop(l).ms == 10);
    ok1(poller_events_pop(l).ms == 11);
    ok1(l->n == 0);
    poller_events_delete(l);
}

void poller_event_test() {
    simple();
}

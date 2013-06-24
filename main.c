#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>

#include "argv.h"
#include "poller.h"
#include "panic.h"

#define MS_IN_SEC 1000

int main(int argc, char **argv) {
    /* struct poller *p = poller_new(100); */
    argv_t args[] = { {"--output", NULL}, ARGV_T_END };
    char *outputfile;

    argv_parse(argc, argv, args);
    outputfile = argv_get(args, "--output");

    if (open(outputfile, O_APPEND | O_WRONLY) == -1) {
        perrpanic("open");
    }

    return 0;
}

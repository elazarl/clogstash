#include <stdlib.h>
#include <dlfcn.h>
#include "mock_test.h"

read_func mock_read = NULL;

ssize_t read(int fildes, void *buf, size_t nbyte) {
    if (mock_read == NULL) {
        mock_read = dlsym(RTLD_NEXT, "read");
    }
    return mock_read(fildes, buf, nbyte);
}

void mock_test(){}

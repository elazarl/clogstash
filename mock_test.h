#ifndef __H_MOCK_TEST_
#define __H_MOCK_TEST_
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
typedef ssize_t (*read_func)(int fildes, void *buf, size_t nbyte);
extern read_func mock_read;
#endif

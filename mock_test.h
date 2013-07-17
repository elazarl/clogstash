#ifndef __H_MOCK_TEST_
#define __H_MOCK_TEST_
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
typedef ssize_t (*read_func)(int fildes, void *buf, size_t nbyte);
ssize_t orig_read(int fildes, void *buf, size_t nbyte);
extern read_func mock_read;
/** mock read_fildes will mock the "read" method to read the data in the data string
 * when reading from the file descriptor fildes. When done, returnvalue is returned,
 * and errno is set to experror 
 */
void mock_read_fildes(int fildes, char *data, int experrno);
#endif

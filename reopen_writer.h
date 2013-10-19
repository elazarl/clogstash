#ifndef __H_REOPEN_WRITER_
#define __H_REOPEN_WRITER_
/**
 * Design:
 * At the most basic level a reopen-writer would immediately reopen the connection
 * on error.
 *
 * For example, the system call sequence of events might look like:
 *
 *     poll( {{fd, ...}, ...}, ... );
 *     read(fd, ... ) = -1;
 *     fd = open("filename");
 *     errno = EINTR; // caller need to issue read again
 *     return -1;
 *     poll( {{fd, ...}}, ...}, ... };
 *     read(fd, ... ) = 1300;
 *     poll( {{fd, ...}}, ...}, ... };
 *     ...
 * Or
 *
 *     poll( {{fd, ...}, ...}, ... );
 *     read(fd, ... ) = -1;
 *     fd = socket(AF_INET, SOCK_STREAM, 0);
 *     fcntl(fd, F_SETFL, O_NONBLOCK);
 *     connect(fd, (struct sockaddr *)addr, sizeof(*addr));
 *     errno = EINTR; // caller need to issue read again
 *     return -1;
 *     poll( {{fd, ...}}, ...}, ... };
 *     read(fd, ... ) = 1300;
 *     poll( {{fd, ...}}, ...}, ... };
 *     ...
 *     
 * A real world reopener also allows to limit reconnections in time. So that it will not connect too
 * frequently. This is implemented by the poll loop keeping track on how much time elapsed
 * and triggering events when registered.
 *
 * For example:
 *
 *     poll( ... );
 *     read(fd, ... ) = -1;
 *     poller_disable(fd); // do not poll for this fd
 *     poller_call({ poller_enable(fd); }, 5*SECONDS);
 *     errno = EINTR;
 *     poll( ..., 5000);
 *     if ( time_elapsed > 5*SECONDS) { poller_enable(fd); }();
 *     poll( ..., -1);
 *
 */

#include <time.h>
#include "poller.h"
#include "io.h"

struct file_reopener;
struct writer file_reopen_writer_make(struct poller *p, char *path, struct timespec duration);

#endif

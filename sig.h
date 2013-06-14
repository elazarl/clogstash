#ifndef __H_SIG_
#define __H_SIG_

/**
 * sigfd creates an fd, and sets signal handler that would write to
 * this fd when the signal is called
 */
int sigfd(int sig);

#endif

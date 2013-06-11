#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>
#include <unistd.h>

#define MS_IN_SEC 1000
typedef unsigned char byte;

int main(int argc, char **argv) {
	struct pollfd evts = {0, POLLIN, 0};
	ssize_t nr;
	byte buf[1000];

	if (poll(&evts, 1, -1) < 0) {
		perror("poll()");
		return -1;
	}

	nr = read(0, buf, sizeof(buf)-1);
	if (nr < 0) {
		perror("read()");
		return -2;
	}
	buf[nr] = '\0';
	printf("got %s\n", buf);

	return 0;
}

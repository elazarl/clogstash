#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include "mock_test.h"
#include "panic.h"
#include "tap.h"

ssize_t orig_read(int fildes, void *buf, size_t nbyte) {
    static read_func orig_read_func = NULL;
    if (orig_read_func == NULL) {
        orig_read_func = dlsym(RTLD_NEXT, "read");
    }
    return orig_read_func(fildes, buf, nbyte);
}

read_func mock_read = NULL;
ssize_t read(int fildes, void *buf, size_t nbyte) {
    if (mock_read != NULL) {
        return mock_read(fildes, buf, nbyte);
    }
    return orig_read(fildes, buf, nbyte);
}

struct fildes2mock {
    int fildes;
    char *data;
    int experrno;
    int size;
};

#define FILDES2MOCK 100

int nfildes2mock = 0;
struct fildes2mock fildes2mock_array[FILDES2MOCK];

ssize_t read_ascii_mock(int fildes, void *buf, size_t nbyte) {
    int i = 0;
    for (; i < nfildes2mock; i++) {
        struct fildes2mock *ent = &fildes2mock_array[i];
        if (ent->fildes == fildes) {
            if (ent->size == 0) {
                if (ent->experrno != 0) {
                    errno = ent->experrno;
                    return -1;
                }
                return 0;
            }
            if ((size_t)ent->size < nbyte) {
                nbyte = ent->size;
            }
            memcpy(buf, ent->data, nbyte);
            ent->size -= nbyte;
            ent->data += nbyte;
            return nbyte;
        }
    }
    panicf("no such fildes %d", fildes);
    return -1;
}

void mock_read_fildes(int fildes, char *data, int experrno) {
    struct fildes2mock m = { fildes, data, experrno, 0 };
    if (data == NULL) {
        m.size = 0;
    } else {
        m.size = strlen(data) + 1;
    }
    mock_read = read_ascii_mock;
    if (nfildes2mock == FILDES2MOCK) panic("too much filedes mocks");
    fildes2mock_array[nfildes2mock] = m;
    nfildes2mock++;
}

void mock_read_destroy() {
    mock_read = NULL;
}

void read_all_test() {
    int mocked_fildes = 12;
    int tmp;
    char b[100] = {};
    mock_read_fildes(mocked_fildes, "koko", 0);
    tmp = read(mocked_fildes, b, sizeof(b));
    ok(tmp == strlen("koko")+1, "expected read %d got %d", strlen("koko")+1, tmp);
    ok(strcmp(b, "koko") == 0, "expected koko got %s", b);
    tmp = read(mocked_fildes, b, sizeof(b));
    ok(tmp == 0, "expected 0 got %d", tmp);
}

void read_parts_test() {
    int mocked_fildes = 13;
    char *mock_data = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int chunkSize = 10;
    char b[200] = {};
    char *p = b;
    mock_read_fildes(mocked_fildes, mock_data, 0);
    while (1) {
        int nbyte = read(mocked_fildes, p, chunkSize);
        p += nbyte;
        if (nbyte == 0) {
            break;
        }
    }
    ok(memcmp(mock_data, b, strlen(mock_data) + 1) == 0, "expected:\n\t%s\n\tread:\n\t%s", mock_data, b);
}

void read_error_test() {
    int mocked_fildes = 17;
    mock_read_fildes(mocked_fildes, NULL, EIO);
    char b[100];
    ok1(-1 == read(mocked_fildes, b, sizeof(b)));
    ok1(errno == EIO);
}

void mock_test() {
    read_all_test();
    read_parts_test();
    read_error_test();
    mock_read_destroy();
}

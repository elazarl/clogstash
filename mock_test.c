#define _GNU_SOURCE
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>
#include "mock_test.h"
#include "helpers.h"
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

int orig_open(const char *path, int oflag, ...) {
    static open_func orig_open_func = NULL;
    if (orig_open_func == NULL) {
        orig_open_func = dlsym(RTLD_NEXT, "open");
    }
    if (oflag | O_CREAT) {
        va_list ap;
        int mask;
        va_start(ap, oflag);
        mask = va_arg(ap, int);
        va_end(ap);
        return orig_open_func(path, oflag, mask);
    }
    return orig_open_func(path, oflag);
}

static open_func mock_open = NULL;
int open(const char *path, int oflag, ...) {
    int mask;
    if (oflag | O_CREAT) {
        va_list ap;
        va_start(ap, oflag);
        mask = va_arg(ap, int);
        va_end(ap);
    }
    if (mock_open == NULL) {
        return (oflag | O_CREAT) ? orig_open(path, oflag, mask) : orig_open(path, oflag);
    }
    return (oflag | O_CREAT) ? mock_open(path, oflag, mask) : orig_open(path, oflag);
}

struct mock_open_table {
    char *path;
    int fildes;
    int experrno;
};

#define OPEN_TABLE_SIZE 1000
struct mock_open_table open_table[OPEN_TABLE_SIZE];
int open_table_size = 0;

int open_from_table(const char *path, int UNUSED(oflag), ...) {
    int i = 0;
    for (; i < open_table_size; i++) {
        if (strcmp(open_table[i].path, path) == 0) {
            if (open_table[i].experrno != 0) {
                errno = open_table[i].experrno;
            }
            return open_table[i].fildes;
        }
    }
    panicf("Cannot find path %s", path);
    return -1;
}

void mock_open_path(char *path, int fildes, int experrno) {
    struct mock_open_table cell = { path, fildes, experrno };
    mock_open = open_from_table;
    if (open_table_size == OPEN_TABLE_SIZE) {
        panic("overflow at open_table");
    }
    open_table[open_table_size] = cell;
    open_table_size++;
}

void mock_destroy() {
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

void open_test() {
    int actual;
    mock_open_path("/nowhere", 42, 0);
    actual = open("/nowhere", O_CREAT, 0777);
    ok(actual == 42, "expected 42 got %d", actual);
    actual = open("/nowhere", O_RDONLY);
    ok(actual == 42, "expected 42 got %d", actual);
}

void mock_test() {
    read_all_test();
    read_parts_test();
    read_error_test();
    open_test();
    mock_destroy();
}

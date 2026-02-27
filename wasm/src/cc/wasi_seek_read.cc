#include "bench_common.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int main() {
    const char* path = "u2bench_seek_read.bin";
    {
        const int fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0644);
        if (fd < 0) {
            printf("open failed: %s\n", strerror(errno));
            return 1;
        }
        uint8_t buf[4096];
        for (size_t i = 0; i < sizeof(buf); ++i) {
            buf[i] = (uint8_t)(i * 13u + 7u);
        }
        if (write(fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf)) {
            printf("write failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
        close(fd);
    }

    const int fd = open(path, O_RDONLY, 0);
    if (fd < 0) {
        printf("open(read) failed: %s\n", strerror(errno));
        return 1;
    }

    constexpr int kOps = 200000;
    uint32_t state = 1;
    uint64_t acc = 0;
    uint8_t buf[16];

    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kOps; ++i) {
        const uint32_t r = u2bench_xorshift32(&state);
        const off_t off = (off_t)(r & (4096u - 16u));
        if (lseek(fd, off, SEEK_SET) < 0) {
            printf("lseek failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
        const ssize_t n = read(fd, buf, sizeof(buf));
        if (n != (ssize_t)sizeof(buf)) {
            printf("read failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
        for (int j = 0; j < 16; ++j) {
            acc += buf[j];
        }
    }
    const uint64_t t1 = u2bench_now_ns();

    close(fd);
    {
        const int fd2 = open(path, O_WRONLY | O_TRUNC, 0644);
        if (fd2 >= 0) close(fd2);
    }

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


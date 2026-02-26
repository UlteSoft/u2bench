#include "bench_common.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int main() {
    const char* path = "u2bench_openclose.tmp";
    {
        const int fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0644);
        if (fd < 0) {
            printf("open(create) failed: %s\n", strerror(errno));
            return 1;
        }
        const uint8_t b[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
        (void)write(fd, b, sizeof(b));
        close(fd);
    }

    constexpr int kIters = 20000;
    uint64_t acc = 0;
    uint8_t buf[16];

    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kIters; ++i) {
        const int fd = open(path, O_RDONLY, 0);
        if (fd < 0) {
            printf("open failed: %s\n", strerror(errno));
            return 1;
        }
        const ssize_t n = read(fd, buf, sizeof(buf));
        if (n < 0) {
            printf("read failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
        for (size_t j = 0; j < (size_t)n; ++j) {
            acc ^= (uint64_t)buf[j] << ((j & 7u) * 8u);
        }
        close(fd);
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}

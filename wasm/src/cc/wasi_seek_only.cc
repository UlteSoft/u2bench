#include "bench_common.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int main() {
    const char* path = "u2bench_seek_only.bin";
    const int fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd < 0) {
        printf("open failed: %s\n", strerror(errno));
        return 1;
    }

    // Ensure the file exists and has some size.
    uint8_t buf[4096];
    for (size_t i = 0; i < sizeof(buf); ++i) {
        buf[i] = (uint8_t)(i * 17u + 3u);
    }
    if (write(fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf)) {
        printf("write failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    constexpr int kIters = 500000;
    uint64_t acc = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kIters; ++i) {
        const off_t off = (off_t)(((uint32_t)i * 97u) & 4095u);
        const off_t pos = lseek(fd, off, SEEK_SET);
        if (pos < 0) {
            printf("lseek failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
        acc += (uint64_t)pos;
    }
    const uint64_t t1 = u2bench_now_ns();

    close(fd);
    {
        const int fd2 = open(path, O_WRONLY | O_TRUNC, 0644);
        if (fd2 >= 0) {
            close(fd2);
        }
    }

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


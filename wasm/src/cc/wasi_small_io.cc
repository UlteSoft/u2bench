#include "bench_common.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static inline bool write_all(int fd, const uint8_t* buf, size_t len) {
    size_t off = 0;
    while (off < len) {
        const ssize_t rc = write(fd, buf + off, len - off);
        if (rc < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        off += (size_t)rc;
    }
    return true;
}

static inline bool read_full(int fd, uint8_t* buf, size_t len, uint64_t* sum) {
    size_t off = 0;
    while (off < len) {
        const ssize_t rc = read(fd, buf + off, len - off);
        if (rc < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (rc == 0) {
            return false;
        }
        for (ssize_t i = 0; i < rc; ++i) {
            *sum += buf[off + (size_t)i];
        }
        off += (size_t)rc;
    }
    return true;
}

int main() {
    const char* path = "u2bench_small_io.bin";
    const int fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd < 0) {
        printf("open failed: %s\n", strerror(errno));
        return 1;
    }

    alignas(16) static uint8_t buf[64];
    for (size_t i = 0; i < sizeof(buf); ++i) {
        buf[i] = (uint8_t)(i * 17u + 3u);
    }

    constexpr int kOps = 100000; // 6.4 MiB

    const uint64_t t0 = u2bench_now_ns();

    for (int i = 0; i < kOps; ++i) {
        buf[i & 63] ^= (uint8_t)i;
        if (!write_all(fd, buf, sizeof(buf))) {
            printf("write failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        printf("lseek failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    uint64_t sum = 0;
    for (int i = 0; i < kOps; ++i) {
        if (!read_full(fd, buf, sizeof(buf), &sum)) {
            printf("read failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
    }

    const uint64_t t1 = u2bench_now_ns();

    close(fd);
    {
        const int fd2 = open(path, O_WRONLY | O_TRUNC, 0644);
        if (fd2 >= 0) {
            close(fd2);
        }
    }

    u2bench_sink_u64(sum);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}

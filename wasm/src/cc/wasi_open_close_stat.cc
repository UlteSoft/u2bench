#include "bench_common.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
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
    struct stat st;

    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kIters; ++i) {
        const int fd = open(path, O_RDONLY, 0);
        if (fd < 0) {
            printf("open failed: %s\n", strerror(errno));
            return 1;
        }
        if (fstat(fd, &st) != 0) {
            printf("fstat failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
        acc ^= (uint64_t)st.st_size + (uint64_t)(uint32_t)st.st_mode;
        close(fd);
    }
    const uint64_t t1 = u2bench_now_ns();

    (void)unlink(path);
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


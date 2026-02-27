#include "bench_common.h"

#include <stdint.h>
#include <wasi/wasip1.h>

int main() {
    constexpr int kIters = 100000;

    static const uint8_t dummy = 0;
    __wasi_ciovec_t iov;
    iov.buf = &dummy;
    iov.buf_len = 0;

    __wasi_size_t nwritten = 0;
    uint64_t acc = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kIters; ++i) {
        const __wasi_errno_t rc = __wasi_fd_write(1, &iov, 1, &nwritten);
        acc += (uint64_t)rc + (uint64_t)nwritten;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


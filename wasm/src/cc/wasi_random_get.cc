#include "bench_common.h"

#include <stdint.h>
#include <wasi/api.h>

int main() {
    alignas(16) static uint8_t buf[8192];
    constexpr int kIters = 2000; // 16 MiB total

    uint64_t acc = 0;
    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kIters; ++i) {
        const __wasi_errno_t rc = __wasi_random_get(buf, sizeof(buf));
        if (rc != 0) {
            printf("random_get failed: %u\n", (unsigned)rc);
            return 1;
        }
        const uint32_t idx = (uint32_t)i * 131u;
        acc ^= (uint64_t)buf[idx & (sizeof(buf) - 1)];
        acc ^= (uint64_t)buf[(idx + 123) & (sizeof(buf) - 1)] << 8;
        acc ^= (uint64_t)buf[(idx + 777) & (sizeof(buf) - 1)] << 16;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


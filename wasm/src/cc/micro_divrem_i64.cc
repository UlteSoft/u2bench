#include "bench_common.h"

#include <stdint.h>

int main() {
    constexpr uint32_t kIters = 3000000;

    uint64_t state = 1;
    uint64_t acc = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        state = u2bench_splitmix64(state);
        const uint64_t a = state;
        state = u2bench_splitmix64(state);
        const uint64_t b = state | 1ull;
        acc ^= a / b;
        acc += a % b;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


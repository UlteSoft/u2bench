#include "bench_common.h"

#include <stdint.h>

int main() {
    constexpr uint32_t kIters = 20000000;

    uint32_t state = 1;
    uint32_t acc = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        uint32_t x = u2bench_xorshift32(&state);
        x |= 1u; // avoid UB for clz/ctz

        acc += (uint32_t)__builtin_popcount(x);
        acc += (uint32_t)__builtin_clz(x);
        acc += (uint32_t)__builtin_ctz(x);

        x = (x << 7) | (x >> 25);
        x ^= (x << 3);
        acc ^= x + i;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


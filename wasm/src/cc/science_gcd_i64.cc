#include "bench_common.h"

#include <stdint.h>

static inline uint64_t gcd_u64(uint64_t a, uint64_t b) {
    while (b != 0) {
        const uint64_t t = a % b;
        a = b;
        b = t;
    }
    return a;
}

int main() {
    constexpr int kN = 500000;
    uint64_t seed = 1;
    uint64_t acc = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kN; ++i) {
        seed = u2bench_splitmix64(seed);
        const uint64_t a = (seed | 1ull) ^ (seed >> 17);
        seed = u2bench_splitmix64(seed);
        const uint64_t b = (seed | 1ull) ^ (seed >> 23);
        acc ^= gcd_u64(a, b);
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


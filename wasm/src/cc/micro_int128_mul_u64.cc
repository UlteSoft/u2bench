#include "bench_common.h"

#include <stdint.h>

int main() {
    uint64_t x = 1;
    unsigned __int128 acc = 1;

    constexpr uint32_t kIters = 2000000u;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        x = u2bench_splitmix64(x + (uint64_t)i);
        const uint64_t y = x ^ 0x9e3779b97f4a7c15ull;
        acc += (unsigned __int128)x * (unsigned __int128)y;
        acc ^= (acc << 13);
        acc += (acc >> 7);
    }
    const uint64_t t1 = u2bench_now_ns();

    const uint64_t lo = (uint64_t)acc;
    const uint64_t hi = (uint64_t)(acc >> 64);
    u2bench_sink_u64(lo ^ hi);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


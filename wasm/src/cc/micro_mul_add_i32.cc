#include "bench_common.h"

#include <stdint.h>

int main() {
    uint32_t x = 1u;
    uint64_t acc = 0;

    constexpr uint32_t kIters = 50000000u;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        x = x * 1664525u + 1013904223u;
        x ^= (x << 13);
        x += (x >> 7);
        x ^= (x << 9);
        x = (x << 5) | (x >> (32u - 5u));
        acc += (uint64_t)(x ^ (i * 0x9e3779b9u));
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


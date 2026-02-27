#include "bench_common.h"

#include <stdint.h>
#include <stdlib.h>

int main() {
    constexpr uint32_t kMask = (1u << 18) - 1u; // 262,144 entries (1 MiB as u32)
    constexpr uint32_t kN = kMask + 1u;
    constexpr uint32_t kIters = 16000000;

    uint32_t* a = (uint32_t*)malloc((size_t)kN * sizeof(uint32_t));
    if (!a) {
        printf("malloc failed\n");
        return 1;
    }

    uint32_t state = 1;
    for (uint32_t i = 0; i < kN; ++i) {
        state = u2bench_xorshift32(&state);
        a[i] = state ^ (i * 0x9e3779b9u);
    }

    uint32_t idx = 1;
    uint64_t acc = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        idx = idx * 1664525u + 1013904223u;
        const uint32_t j = idx & kMask;
        uint32_t v = a[j];
        v += (idx ^ i) + (v >> 7);
        a[j] = v;
        acc += (uint64_t)v;
    }
    const uint64_t t1 = u2bench_now_ns();

    free(a);
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


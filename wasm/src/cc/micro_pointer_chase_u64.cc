#include "bench_common.h"

#include <stdint.h>
#include <stdlib.h>

int main() {
    constexpr uint32_t kMask = (1u << 19) - 1u; // 524,288 entries (4 MiB as u64)
    constexpr uint32_t kN = kMask + 1u;
    constexpr uint32_t kIters = 12000000;

    uint64_t* next = (uint64_t*)malloc((size_t)kN * sizeof(uint64_t));
    if (!next) {
        printf("malloc failed\n");
        return 1;
    }

    uint64_t state = 1;
    for (uint32_t i = 0; i < kN; ++i) {
        state = u2bench_splitmix64(state);
        next[i] = (uint64_t)((uint32_t)state & kMask);
    }

    uint64_t idx = 0;
    uint64_t acc = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        idx = next[(uint32_t)idx];
        acc += idx;
    }
    const uint64_t t1 = u2bench_now_ns();

    free(next);
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


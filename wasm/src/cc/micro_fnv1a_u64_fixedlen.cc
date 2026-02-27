#include "bench_common.h"

#include <stdint.h>
#include <stdlib.h>

static inline uint64_t fnv1a_64_fixed(const uint8_t* p, size_t n) {
    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < n; ++i) {
        h ^= (uint64_t)p[i];
        h *= 1099511628211ull;
    }
    return h;
}

int main() {
    constexpr size_t kStrLen = 32;
    constexpr size_t kCount = 80000; // 2.56 MiB total
    constexpr size_t kTotal = kStrLen * kCount;
    constexpr int kReps = 10;

    uint8_t* data = (uint8_t*)malloc(kTotal);
    if (!data) {
        printf("malloc failed\n");
        return 1;
    }

    uint32_t state = 1;
    for (size_t i = 0; i < kTotal; ++i) {
        data[i] = (uint8_t)u2bench_xorshift32(&state);
    }

    uint64_t acc = 0;
    const uint64_t t0 = u2bench_now_ns();
    for (int r = 0; r < kReps; ++r) {
        for (size_t i = 0; i < kCount; ++i) {
            acc += fnv1a_64_fixed(data + i * kStrLen, kStrLen);
        }
        acc ^= (uint64_t)r * 0x9e3779b97f4a7c15ull;
    }
    const uint64_t t1 = u2bench_now_ns();

    free(data);
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}

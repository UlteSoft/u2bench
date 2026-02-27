#include "bench_common.h"

#include <stdint.h>
#include <stdlib.h>

int main() {
    constexpr uint32_t kIters = 1000000;
    constexpr uint32_t kLive = 1024;

    uint32_t state = 1;
    uint64_t acc = 0;
    void* ptrs[kLive] = {};

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        const uint32_t slot = i & (kLive - 1u);
        if (ptrs[slot]) {
            const uint8_t* q = (const uint8_t*)ptrs[slot];
            acc += (uint64_t)q[0];
            free(ptrs[slot]);
            ptrs[slot] = nullptr;
        }
        const uint32_t r = u2bench_xorshift32(&state);
        const size_t sz = (size_t)(16u + (r & 511u)); // 16..527
        uint8_t* p = (uint8_t*)malloc(sz);
        if (!p) {
            printf("malloc failed\n");
            return 1;
        }
        p[0] = (uint8_t)r;
        p[sz - 1] = (uint8_t)(r >> 8);
        acc += (uint64_t)p[0] + (uint64_t)p[sz - 1];
        ptrs[slot] = p;
    }
    for (uint32_t i = 0; i < kLive; ++i) {
        if (ptrs[i]) {
            const uint8_t* q = (const uint8_t*)ptrs[i];
            acc += (uint64_t)q[0];
            free(ptrs[i]);
        }
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}

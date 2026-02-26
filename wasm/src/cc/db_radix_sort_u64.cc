#include "bench_common.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static inline void radix_sort_u64(uint64_t* a, uint64_t* tmp, size_t n) {
    for (int pass = 0; pass < 8; ++pass) {
        uint32_t hist[256] = {};
        const uint32_t shift = (uint32_t)pass * 8u;
        for (size_t i = 0; i < n; ++i) {
            hist[(uint32_t)((a[i] >> shift) & 0xffu)]++;
        }

        uint32_t offs[256];
        uint32_t sum = 0;
        for (uint32_t b = 0; b < 256; ++b) {
            offs[b] = sum;
            sum += hist[b];
        }

        for (size_t i = 0; i < n; ++i) {
            const uint32_t b = (uint32_t)((a[i] >> shift) & 0xffu);
            tmp[offs[b]++] = a[i];
        }

        uint64_t* swap = a;
        a = tmp;
        tmp = swap;
    }

    // 8 passes => sorted data ends up back in original array.
}

int main() {
    constexpr size_t kN = 200000;

    uint64_t* a = (uint64_t*)malloc(kN * sizeof(uint64_t));
    uint64_t* tmp = (uint64_t*)malloc(kN * sizeof(uint64_t));
    if (!a || !tmp) {
        printf("malloc failed\n");
        return 1;
    }

    uint64_t seed = 1;
    for (size_t i = 0; i < kN; ++i) {
        seed = u2bench_splitmix64(seed);
        a[i] = seed;
    }

    const uint64_t t0 = u2bench_now_ns();
    radix_sort_u64(a, tmp, kN);
    const uint64_t t1 = u2bench_now_ns();

    uint64_t acc = 0;
    for (size_t i = 0; i < kN; i += 997) {
        acc ^= a[i];
    }

    free(a);
    free(tmp);
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


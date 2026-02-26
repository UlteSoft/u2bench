#include "bench_common.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static inline void bloom_set(uint8_t* bits, uint32_t idx) {
    bits[idx >> 3] |= (uint8_t)(1u << (idx & 7u));
}

static inline bool bloom_get(const uint8_t* bits, uint32_t idx) {
    return (bits[idx >> 3] & (uint8_t)(1u << (idx & 7u))) != 0;
}

int main() {
    constexpr uint32_t kBits = 1u << 24; // 16,777,216 bits (2 MiB)
    constexpr uint32_t kMask = kBits - 1;
    constexpr size_t kBytes = (size_t)kBits / 8u;
    constexpr int kK = 7;

    uint8_t* bits = (uint8_t*)calloc(kBytes, 1);
    if (!bits) {
        printf("calloc failed\n");
        return 1;
    }

    constexpr uint32_t kN = 250000;
    uint64_t seed = 1;
    uint64_t* keys = (uint64_t*)malloc((size_t)kN * sizeof(uint64_t));
    if (!keys) {
        printf("malloc failed\n");
        free(bits);
        return 1;
    }
    for (uint32_t i = 0; i < kN; ++i) {
        seed = u2bench_splitmix64(seed);
        keys[i] = seed | 1ull;
    }

    const uint64_t t0 = u2bench_now_ns();

    for (uint32_t i = 0; i < kN; ++i) {
        uint64_t h = u2bench_splitmix64(keys[i]);
        for (int j = 0; j < kK; ++j) {
            h = u2bench_splitmix64(h + (uint64_t)j * 0x9e3779b97f4a7c15ull);
            bloom_set(bits, (uint32_t)h & kMask);
        }
    }

    uint32_t hits = 0;
    constexpr uint32_t kQ = 500000;
    for (uint32_t i = 0; i < kQ; ++i) {
        const uint64_t k = (i & 1u) ? keys[i % kN] : (keys[i % kN] ^ 0xdeadbeefcafebabeull);
        uint64_t h = u2bench_splitmix64(k);
        bool ok = true;
        for (int j = 0; j < kK; ++j) {
            h = u2bench_splitmix64(h + (uint64_t)j * 0x9e3779b97f4a7c15ull);
            ok &= bloom_get(bits, (uint32_t)h & kMask);
        }
        hits += (uint32_t)ok;
    }

    const uint64_t t1 = u2bench_now_ns();

    free(keys);
    free(bits);

    u2bench_sink_u64((uint64_t)hits);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


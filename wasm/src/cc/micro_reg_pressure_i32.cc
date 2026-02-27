#include "bench_common.h"

#include <stdint.h>

static inline uint32_t rotl32(uint32_t x, unsigned k) {
    return (x << k) | (x >> (32u - k));
}

static inline uint32_t rotr32(uint32_t x, unsigned k) {
    return (x >> k) | (x << (32u - k));
}

int main() {
    uint32_t a0 = 1, a1 = 2, a2 = 3, a3 = 4;
    uint32_t a4 = 5, a5 = 6, a6 = 7, a7 = 8;
    uint32_t a8 = 9, a9 = 10, a10 = 11, a11 = 12;
    uint32_t a12 = 13, a13 = 14, a14 = 15, a15 = 16;

    uint64_t acc = 0;
    constexpr uint32_t kIters = 20000000u;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        const uint32_t x = i * 0x9e3779b9u + (a0 ^ a7);

        a0 += x + a1;
        a1 = rotl32(a1 ^ x, 7) + a2;
        a2 = rotr32(a2 + x, 13) ^ a3;
        a3 = a3 * 1664525u + x + a4;

        a4 ^= a5 + x;
        a5 += (a6 ^ (x >> 3));
        a6 = rotl32(a6 + a7, 17) ^ x;
        a7 = rotr32(a7 * 1013904223u + x, 11);

        a8 += a9 ^ x;
        a9 = rotl32(a9 + a10, 9) + (x ^ a0);
        a10 ^= a11 + (x << 1);
        a11 = rotr32(a11 + a12, 19) ^ x;

        a12 = a12 * 2246822519u + (x ^ a13);
        a13 ^= rotl32(a14 + x, 3);
        a14 += rotr32(a15 ^ x, 5);
        a15 = rotl32(a15 + x + a3, 27);

        acc += (uint64_t)(a0 ^ a5 ^ a10 ^ a15) + (uint64_t)x;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc ^ (uint64_t)a0 ^ (uint64_t)a5 ^ (uint64_t)a10 ^ (uint64_t)a15);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


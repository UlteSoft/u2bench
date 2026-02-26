#include "bench_common.h"

#include <stdint.h>

static inline uint64_t rotl64(uint64_t x, uint32_t n) {
    return (x << n) | (x >> (64u - n));
}

static inline void sip_round(uint64_t& v0, uint64_t& v1, uint64_t& v2, uint64_t& v3) {
    v0 += v1;
    v1 = rotl64(v1, 13);
    v1 ^= v0;
    v0 = rotl64(v0, 32);

    v2 += v3;
    v3 = rotl64(v3, 16);
    v3 ^= v2;

    v0 += v3;
    v3 = rotl64(v3, 21);
    v3 ^= v0;

    v2 += v1;
    v1 = rotl64(v1, 17);
    v1 ^= v2;
    v2 = rotl64(v2, 32);
}

static uint64_t siphash24_u64(uint64_t k0, uint64_t k1, uint64_t m) {
    uint64_t v0 = 0x736f6d6570736575ull ^ k0;
    uint64_t v1 = 0x646f72616e646f6dull ^ k1;
    uint64_t v2 = 0x6c7967656e657261ull ^ k0;
    uint64_t v3 = 0x7465646279746573ull ^ k1;

    v3 ^= m;
    sip_round(v0, v1, v2, v3);
    sip_round(v0, v1, v2, v3);
    v0 ^= m;

    const uint64_t b = (uint64_t)8u << 56;
    v3 ^= b;
    sip_round(v0, v1, v2, v3);
    sip_round(v0, v1, v2, v3);
    v0 ^= b;

    v2 ^= 0xff;
    sip_round(v0, v1, v2, v3);
    sip_round(v0, v1, v2, v3);
    sip_round(v0, v1, v2, v3);
    sip_round(v0, v1, v2, v3);
    return v0 ^ v1 ^ v2 ^ v3;
}

int main() {
    const uint64_t k0 = 0x0706050403020100ull;
    const uint64_t k1 = 0x0f0e0d0c0b0a0908ull;

    constexpr int kN = 250000;
    uint64_t acc = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kN; ++i) {
        const uint64_t m = u2bench_splitmix64((uint64_t)i + 1);
        acc ^= siphash24_u64(k0, k1, m);
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


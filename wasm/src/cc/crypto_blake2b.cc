#include "bench_common.h"

#include <stdint.h>

static inline uint64_t rotr64(uint64_t x, uint64_t n) {
    return (x >> n) | (x << (64u - n));
}

static constexpr uint64_t kIV[8] = {
    0x6a09e667f3bcc908ull,
    0xbb67ae8584caa73bull,
    0x3c6ef372fe94f82bull,
    0xa54ff53a5f1d36f1ull,
    0x510e527fade682d1ull,
    0x9b05688c2b3e6c1full,
    0x1f83d9abfb41bd6bull,
    0x5be0cd19137e2179ull,
};

static constexpr uint8_t kSigma[12][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
    {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
    {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
    {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
    {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
    {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
    {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
    {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
    {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
};

static inline void G(uint64_t v[16], int a, int b, int c, int d, uint64_t x, uint64_t y) {
    v[a] = v[a] + v[b] + x;
    v[d] = rotr64(v[d] ^ v[a], 32);
    v[c] = v[c] + v[d];
    v[b] = rotr64(v[b] ^ v[c], 24);
    v[a] = v[a] + v[b] + y;
    v[d] = rotr64(v[d] ^ v[a], 16);
    v[c] = v[c] + v[d];
    v[b] = rotr64(v[b] ^ v[c], 63);
}

static inline void blake2b_compress(uint64_t h[8], const uint64_t m[16], uint64_t t0) {
    uint64_t v[16];
    for (int i = 0; i < 8; ++i) {
        v[i] = h[i];
        v[i + 8] = kIV[i];
    }
    v[12] ^= t0;

    for (int r = 0; r < 12; ++r) {
        const uint8_t* s = kSigma[r];
        G(v, 0, 4, 8, 12, m[s[0]], m[s[1]]);
        G(v, 1, 5, 9, 13, m[s[2]], m[s[3]]);
        G(v, 2, 6, 10, 14, m[s[4]], m[s[5]]);
        G(v, 3, 7, 11, 15, m[s[6]], m[s[7]]);
        G(v, 0, 5, 10, 15, m[s[8]], m[s[9]]);
        G(v, 1, 6, 11, 12, m[s[10]], m[s[11]]);
        G(v, 2, 7, 8, 13, m[s[12]], m[s[13]]);
        G(v, 3, 4, 9, 14, m[s[14]], m[s[15]]);
    }

    for (int i = 0; i < 8; ++i) {
        h[i] ^= v[i] ^ v[i + 8];
    }
}

int main() {
    uint64_t h[8];
    for (int i = 0; i < 8; ++i) {
        h[i] = kIV[i];
    }

    uint64_t m[16];
    uint64_t seed = 1;
    for (int i = 0; i < 16; ++i) {
        seed = u2bench_splitmix64(seed);
        m[i] = seed;
    }

    constexpr int kIters = 20000;
    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kIters; ++i) {
        m[i & 15] ^= u2bench_splitmix64((uint64_t)i) + (uint64_t)i * 0x9e3779b97f4a7c15ull;
        blake2b_compress(h, m, (uint64_t)i * 128ull);
    }
    const uint64_t t1 = u2bench_now_ns();

    uint64_t acc = 0;
    for (int i = 0; i < 8; ++i) {
        acc ^= u2bench_splitmix64(h[i] + (uint64_t)i);
    }
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


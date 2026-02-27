#include "bench_common.h"

#include <stdint.h>

static inline uint64_t rotl64(uint64_t x, unsigned k) {
    return (x << k) | (x >> (64u - k));
}

static inline void keccakf1600(uint64_t st[25]) {
    static constexpr uint64_t kRC[24] = {
        0x0000000000000001ull, 0x0000000000008082ull, 0x800000000000808Aull, 0x8000000080008000ull,
        0x000000000000808Bull, 0x0000000080000001ull, 0x8000000080008081ull, 0x8000000000008009ull,
        0x000000000000008Aull, 0x0000000000000088ull, 0x0000000080008009ull, 0x000000008000000Aull,
        0x000000008000808Bull, 0x800000000000008Bull, 0x8000000000008089ull, 0x8000000000008003ull,
        0x8000000000008002ull, 0x8000000000000080ull, 0x000000000000800Aull, 0x800000008000000Aull,
        0x8000000080008081ull, 0x8000000000008080ull, 0x0000000080000001ull, 0x8000000080008008ull,
    };

    static constexpr int kRotc[24] = {
        1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14, 27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44,
    };
    static constexpr int kPiln[24] = {
        10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4, 15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1,
    };

    uint64_t bc[5];
    for (int round = 0; round < 24; ++round) {
        // theta
        for (int i = 0; i < 5; ++i) {
            bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];
        }
        for (int i = 0; i < 5; ++i) {
            const uint64_t t = bc[(i + 4) % 5] ^ rotl64(bc[(i + 1) % 5], 1);
            st[i] ^= t;
            st[i + 5] ^= t;
            st[i + 10] ^= t;
            st[i + 15] ^= t;
            st[i + 20] ^= t;
        }

        // rho + pi
        uint64_t t = st[1];
        for (int i = 0; i < 24; ++i) {
            const int j = kPiln[i];
            const uint64_t v = st[j];
            st[j] = rotl64(t, (unsigned)kRotc[i]);
            t = v;
        }

        // chi
        for (int y = 0; y < 25; y += 5) {
            const uint64_t a0 = st[y + 0];
            const uint64_t a1 = st[y + 1];
            const uint64_t a2 = st[y + 2];
            const uint64_t a3 = st[y + 3];
            const uint64_t a4 = st[y + 4];

            st[y + 0] = a0 ^ ((~a1) & a2);
            st[y + 1] = a1 ^ ((~a2) & a3);
            st[y + 2] = a2 ^ ((~a3) & a4);
            st[y + 3] = a3 ^ ((~a4) & a0);
            st[y + 4] = a4 ^ ((~a0) & a1);
        }

        // iota
        st[0] ^= kRC[round];
    }
}

int main() {
    uint64_t st[25];
    uint64_t state = 1;
    for (int i = 0; i < 25; ++i) {
        state = u2bench_splitmix64(state);
        st[i] = state;
    }

    constexpr int kIters = 20000;
    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kIters; ++i) {
        st[i % 25] ^= (uint64_t)i * 0x9e3779b97f4a7c15ull;
        keccakf1600(st);
    }
    const uint64_t t1 = u2bench_now_ns();

    uint64_t acc = 0;
    for (int i = 0; i < 25; ++i) {
        acc ^= st[i] + (uint64_t)i * 0xD6E8FEB86659FD93ull;
        acc = rotl64(acc, 17);
    }
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


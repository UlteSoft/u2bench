#include "bench_common.h"

#include <stddef.h>

static inline uint32_t rotl32(uint32_t x, uint32_t n) {
    return (x << n) | (x >> (32u - n));
}

static inline void qr(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d) {
    a += b;
    d ^= a;
    d = rotl32(d, 16);

    c += d;
    b ^= c;
    b = rotl32(b, 12);

    a += b;
    d ^= a;
    d = rotl32(d, 8);

    c += d;
    b ^= c;
    b = rotl32(b, 7);
}

static inline void chacha20_block(uint32_t out[16], const uint32_t in[16]) {
    uint32_t x[16];
    for (int i = 0; i < 16; ++i) {
        x[i] = in[i];
    }

    for (int i = 0; i < 10; ++i) {
        qr(x[0], x[4], x[8], x[12]);
        qr(x[1], x[5], x[9], x[13]);
        qr(x[2], x[6], x[10], x[14]);
        qr(x[3], x[7], x[11], x[15]);

        qr(x[0], x[5], x[10], x[15]);
        qr(x[1], x[6], x[11], x[12]);
        qr(x[2], x[7], x[8], x[13]);
        qr(x[3], x[4], x[9], x[14]);
    }

    for (int i = 0; i < 16; ++i) {
        out[i] = x[i] + in[i];
    }
}

int main() {
    uint32_t st[16] = {
        0x61707865u,
        0x3320646eu,
        0x79622d32u,
        0x6b206574u,
        0x03020100u,
        0x07060504u,
        0x0b0a0908u,
        0x0f0e0d0cu,
        0x13121110u,
        0x17161514u,
        0x1b1a1918u,
        0x1f1e1d1cu,
        1u,
        0u,
        0u,
        0u,
    };

    uint32_t out[16];
    uint64_t acc = 0;

    constexpr int kBlocks = 200000;
    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kBlocks; ++i) {
        st[12] += 1;
        chacha20_block(out, st);
        acc ^= ((uint64_t)out[i & 15] << 32) | out[(i + 3) & 15];
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


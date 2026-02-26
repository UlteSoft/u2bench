#include "bench_common.h"

#include <stddef.h>
#include <stdint.h>

static constexpr uint8_t kSbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16,
};

static inline uint8_t xtime(uint8_t x) {
    return (uint8_t)((x << 1) ^ ((x >> 7) * 0x1b));
}

static inline void sub_bytes(uint8_t s[16]) {
    for (int i = 0; i < 16; ++i) {
        s[i] = kSbox[s[i]];
    }
}

static inline void shift_rows(uint8_t s[16]) {
    uint8_t t[16];
    t[0] = s[0];
    t[1] = s[5];
    t[2] = s[10];
    t[3] = s[15];

    t[4] = s[4];
    t[5] = s[9];
    t[6] = s[14];
    t[7] = s[3];

    t[8] = s[8];
    t[9] = s[13];
    t[10] = s[2];
    t[11] = s[7];

    t[12] = s[12];
    t[13] = s[1];
    t[14] = s[6];
    t[15] = s[11];
    for (int i = 0; i < 16; ++i) {
        s[i] = t[i];
    }
}

static inline void mix_columns(uint8_t s[16]) {
    for (int c = 0; c < 4; ++c) {
        uint8_t* col = &s[c * 4];
        const uint8_t a0 = col[0];
        const uint8_t a1 = col[1];
        const uint8_t a2 = col[2];
        const uint8_t a3 = col[3];

        const uint8_t t = (uint8_t)(a0 ^ a1 ^ a2 ^ a3);
        const uint8_t u0 = a0;
        col[0] = (uint8_t)(a0 ^ t ^ xtime((uint8_t)(a0 ^ a1)));
        col[1] = (uint8_t)(a1 ^ t ^ xtime((uint8_t)(a1 ^ a2)));
        col[2] = (uint8_t)(a2 ^ t ^ xtime((uint8_t)(a2 ^ a3)));
        col[3] = (uint8_t)(a3 ^ t ^ xtime((uint8_t)(a3 ^ u0)));
    }
}

static inline void add_round_key(uint8_t s[16], const uint8_t* rk) {
    for (int i = 0; i < 16; ++i) {
        s[i] ^= rk[i];
    }
}

static inline void key_expand_128(uint8_t round_keys[176], const uint8_t key[16]) {
    for (int i = 0; i < 16; ++i) {
        round_keys[i] = key[i];
    }

    uint8_t rcon = 1;
    int bytes = 16;
    while (bytes < 176) {
        uint8_t t[4];
        t[0] = round_keys[bytes - 4];
        t[1] = round_keys[bytes - 3];
        t[2] = round_keys[bytes - 2];
        t[3] = round_keys[bytes - 1];

        if ((bytes % 16) == 0) {
            const uint8_t tmp = t[0];
            t[0] = t[1];
            t[1] = t[2];
            t[2] = t[3];
            t[3] = tmp;

            t[0] = kSbox[t[0]];
            t[1] = kSbox[t[1]];
            t[2] = kSbox[t[2]];
            t[3] = kSbox[t[3]];

            t[0] ^= rcon;
            rcon = xtime(rcon);
        }

        for (int i = 0; i < 4; ++i) {
            round_keys[bytes] = (uint8_t)(round_keys[bytes - 16] ^ t[i]);
            ++bytes;
        }
    }
}

static inline void aes128_encrypt(uint8_t s[16], const uint8_t round_keys[176]) {
    add_round_key(s, &round_keys[0]);
    for (int r = 1; r <= 9; ++r) {
        sub_bytes(s);
        shift_rows(s);
        mix_columns(s);
        add_round_key(s, &round_keys[r * 16]);
    }
    sub_bytes(s);
    shift_rows(s);
    add_round_key(s, &round_keys[10 * 16]);
}

int main() {
    uint8_t key[16];
    for (int i = 0; i < 16; ++i) {
        key[i] = (uint8_t)(i * 11u + 7u);
    }
    uint8_t round_keys[176];
    key_expand_128(round_keys, key);

    constexpr int kBlocks = 100000; // 1.6 MiB
    uint64_t acc = 0;

    uint8_t s[16];
    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kBlocks; ++i) {
        uint32_t x = (uint32_t)i * 2654435761u;
        for (int j = 0; j < 16; ++j) {
            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 5;
            s[j] = (uint8_t)x;
        }
        aes128_encrypt(s, round_keys);
        acc ^= (uint64_t)s[i & 15] | ((uint64_t)s[(i + 7) & 15] << 8) | ((uint64_t)s[(i + 11) & 15] << 16);
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


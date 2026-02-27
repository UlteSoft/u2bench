#include "bench_common.h"

#include <stdint.h>
#include <stdlib.h>

static inline void base64_init_dec(uint8_t dec[256], const char* enc) {
    for (int i = 0; i < 256; ++i) {
        dec[i] = 0xffu;
    }
    for (int i = 0; i < 64; ++i) {
        dec[(uint8_t)enc[i]] = (uint8_t)i;
    }
}

int main() {
    static constexpr char kEnc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static uint8_t kDec[256];
    base64_init_dec(kDec, kEnc);

    constexpr size_t kN = 3u * 1024u * 1024u; // multiple of 3 => no '=' padding
    constexpr size_t kEncN = (kN / 3u) * 4u;
    constexpr int kReps = 12;

    uint8_t* in = (uint8_t*)malloc(kN);
    uint8_t* enc = (uint8_t*)malloc(kEncN);
    uint8_t* out = (uint8_t*)malloc(kN);
    if (!in || !enc || !out) {
        printf("malloc failed\n");
        return 1;
    }

    uint32_t state = 1;
    for (size_t i = 0; i < kN; ++i) {
        in[i] = (uint8_t)u2bench_xorshift32(&state);
    }

    uint64_t acc = 0;
    const uint64_t t0 = u2bench_now_ns();

    for (int rep = 0; rep < kReps; ++rep) {
        // Encode.
        size_t j = 0;
        for (size_t i = 0; i < kN; i += 3) {
            const uint32_t v = ((uint32_t)in[i] << 16) | ((uint32_t)in[i + 1] << 8) | (uint32_t)in[i + 2];
            enc[j + 0] = (uint8_t)kEnc[(v >> 18) & 63u];
            enc[j + 1] = (uint8_t)kEnc[(v >> 12) & 63u];
            enc[j + 2] = (uint8_t)kEnc[(v >> 6) & 63u];
            enc[j + 3] = (uint8_t)kEnc[v & 63u];
            j += 4;
        }

        // Decode.
        size_t o = 0;
        for (size_t i = 0; i < kEncN; i += 4) {
            const uint32_t a = kDec[enc[i + 0]];
            const uint32_t b = kDec[enc[i + 1]];
            const uint32_t c = kDec[enc[i + 2]];
            const uint32_t d = kDec[enc[i + 3]];
            const uint32_t v = (a << 18) | (b << 12) | (c << 6) | d;
            out[o + 0] = (uint8_t)((v >> 16) & 0xffu);
            out[o + 1] = (uint8_t)((v >> 8) & 0xffu);
            out[o + 2] = (uint8_t)(v & 0xffu);
            acc += out[o + 0] + out[o + 1] + out[o + 2];
            o += 3;
        }

        // Minor mixing to keep acc live.
        acc ^= (uint64_t)enc[rep & 63] << ((rep & 7) * 8);
    }

    const uint64_t t1 = u2bench_now_ns();

    free(out);
    free(enc);
    free(in);
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}

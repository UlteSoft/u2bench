#include "bench_common.h"

#include <stdint.h>
#include <stdlib.h>

int main() {
    constexpr size_t kN = 4u * 1024u * 1024u;
    constexpr int kReps = 10;

    uint8_t* in = (uint8_t*)malloc(kN);
    uint8_t* out = (uint8_t*)malloc(kN * 2u);
    if (!in || !out) {
        printf("malloc failed\n");
        return 1;
    }

    // Build an input with varied run-lengths (not timed).
    uint32_t state = 1;
    size_t pos = 0;
    while (pos < kN) {
        const uint32_t r = u2bench_xorshift32(&state);
        const uint8_t v = (uint8_t)r;
        const size_t run = 1u + (size_t)((r >> 8) & 63u); // 1..64
        const size_t n = (pos + run <= kN) ? run : (kN - pos);
        for (size_t i = 0; i < n; ++i) {
            in[pos + i] = v;
        }
        pos += n;
    }

    uint64_t acc = 0;
    const uint64_t t0 = u2bench_now_ns();

    for (int rep = 0; rep < kReps; ++rep) {
        // Encode (RLE: [len,u8][byte,u8])
        size_t o = 0;
        for (size_t i = 0; i < kN;) {
            const uint8_t v = in[i];
            uint32_t run = 1;
            while (i + (size_t)run < kN && in[i + (size_t)run] == v && run < 255u) {
                ++run;
            }
            out[o++] = (uint8_t)run;
            out[o++] = v;
            i += (size_t)run;
        }

        // Decode + checksum
        uint64_t sum = 0;
        size_t j = 0;
        for (size_t p = 0; p < o; p += 2) {
            const uint32_t run = out[p];
            const uint8_t v = out[p + 1];
            for (uint32_t k = 0; k < run; ++k) {
                sum += v;
                ++j;
            }
        }
        acc ^= sum + (uint64_t)o + (uint64_t)j;
    }

    const uint64_t t1 = u2bench_now_ns();

    free(out);
    free(in);
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


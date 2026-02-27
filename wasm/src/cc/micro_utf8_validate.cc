#include "bench_common.h"

#include <stdint.h>
#include <stdlib.h>

static inline bool is_cont(uint8_t b) { return (b & 0xc0u) == 0x80u; }

static inline uint64_t validate_utf8_count(const uint8_t* p, size_t n) {
    size_t i = 0;
    uint64_t cps = 0;
    while (i < n) {
        const uint8_t c0 = p[i];
        if (c0 < 0x80u) {
            i += 1;
            cps += 1;
            continue;
        }
        if ((c0 & 0xe0u) == 0xc0u) {
            if (i + 1 >= n) return cps;
            const uint8_t c1 = p[i + 1];
            if (!is_cont(c1) || c0 < 0xc2u) return cps;
            i += 2;
            cps += 1;
            continue;
        }
        if ((c0 & 0xf0u) == 0xe0u) {
            if (i + 2 >= n) return cps;
            const uint8_t c1 = p[i + 1];
            const uint8_t c2 = p[i + 2];
            if (!is_cont(c1) || !is_cont(c2)) return cps;
            if (c0 == 0xe0u && c1 < 0xa0u) return cps; // overlong
            i += 3;
            cps += 1;
            continue;
        }
        if ((c0 & 0xf8u) == 0xf0u) {
            if (i + 3 >= n) return cps;
            const uint8_t c1 = p[i + 1];
            const uint8_t c2 = p[i + 2];
            const uint8_t c3 = p[i + 3];
            if (!is_cont(c1) || !is_cont(c2) || !is_cont(c3)) return cps;
            if (c0 == 0xf0u && c1 < 0x90u) return cps; // overlong
            if (c0 > 0xf4u) return cps;
            i += 4;
            cps += 1;
            continue;
        }
        return cps;
    }
    return cps;
}

int main() {
    constexpr size_t kN = 1u << 20; // 1 MiB
    constexpr int kReps = 80;

    uint8_t* buf = (uint8_t*)malloc(kN);
    if (!buf) {
        printf("malloc failed\n");
        return 1;
    }

    // Fill with a repeating mix of valid UTF-8 sequences (1/2/3/4 bytes).
    // Not timed.
    static const uint8_t s1[] = {0x41u};                         // 'A'
    static const uint8_t s2[] = {0xc2u, 0xa9u};                   // U+00A9
    static const uint8_t s3[] = {0xe2u, 0x82u, 0xacu};            // U+20AC
    static const uint8_t s4[] = {0xf0u, 0x9fu, 0x98u, 0x80u};     // U+1F600
    size_t off = 0;
    while (off < kN) {
        const uint8_t* s = nullptr;
        size_t sl = 0;
        switch ((off >> 4) & 3u) {
            case 0: s = s1; sl = sizeof(s1); break;
            case 1: s = s2; sl = sizeof(s2); break;
            case 2: s = s3; sl = sizeof(s3); break;
            default: s = s4; sl = sizeof(s4); break;
        }
        if (off + sl > kN) break;
        for (size_t i = 0; i < sl; ++i) buf[off + i] = s[i];
        off += sl;
    }
    for (; off < kN; ++off) buf[off] = 0x41u;

    uint64_t acc = 0;
    const uint64_t t0 = u2bench_now_ns();
    for (int r = 0; r < kReps; ++r) {
        acc += validate_utf8_count(buf, kN);
    }
    const uint64_t t1 = u2bench_now_ns();

    free(buf);
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


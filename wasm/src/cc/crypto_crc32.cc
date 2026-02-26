#include "bench_common.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static void crc32_init(uint32_t table[256]) {
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t c = i;
        for (int k = 0; k < 8; ++k) {
            c = (c >> 1) ^ (0xedb88320u & (uint32_t)-(int)(c & 1u));
        }
        table[i] = c;
    }
}

static inline uint32_t crc32_update(const uint32_t table[256], uint32_t crc, const uint8_t* p, size_t n) {
    crc = ~crc;
    for (size_t i = 0; i < n; ++i) {
        crc = table[(crc ^ p[i]) & 0xffu] ^ (crc >> 8);
    }
    return ~crc;
}

int main() {
    uint32_t table[256];
    crc32_init(table);

    constexpr size_t kBuf = 4u * 1024u * 1024u;
    constexpr int kIters = 8; // 32 MiB total

    uint8_t* buf = (uint8_t*)malloc(kBuf);
    if (!buf) {
        printf("malloc failed\n");
        return 1;
    }

    uint32_t rng = 1;
    for (size_t i = 0; i < kBuf; ++i) {
        buf[i] = (uint8_t)u2bench_xorshift32(&rng);
    }

    uint32_t crc = 0;
    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kIters; ++i) {
        crc ^= (uint32_t)i * 0x9e3779b9u;
        crc = crc32_update(table, crc, buf, kBuf);
    }
    const uint64_t t1 = u2bench_now_ns();

    free(buf);
    u2bench_sink_u64((uint64_t)crc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


#include "bench_common.h"

#include <stdint.h>

int main() {
    uint32_t x = 1u;
    constexpr uint32_t kIters = 10000000u;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        switch (x & 63u) {
#define C(N, EXPR) \
    case (N):      \
        x = (EXPR); \
        break;
            C(0, x + 0x9e3779b9u)
            C(1, x ^ 0x7f4a7c15u)
            C(2, x * 0x85ebca6bu)
            C(3, (x << 1) | (x >> 31))
            C(4, (x << 7) | (x >> 25))
            C(5, (x >> 3) | (x << 29))
            C(6, (x ^ (x >> 16)) * 0xc2b2ae35u)
            C(7, (x + (x << 5)) + 0x165667b1u)
            C(8, x - 0x3c6ef372u)
            C(9, (x ^ (x << 13)) + 0x7ed55d16u)
            C(10, (x ^ (x >> 11)) * 0x1b873593u)
            C(11, (x + 0x52dce729u) ^ (x >> 7))
            C(12, x * 3u + 1u)
            C(13, x * 5u + 0x7f4a7c15u)
            C(14, x ^ (x << 9))
            C(15, x ^ (x >> 5))
            C(16, (x + 0x9e3779b9u) ^ (x << 6))
            C(17, (x * 0x27d4eb2du) ^ (x >> 15))
            C(18, (x + (x >> 2)) ^ 0x85ebca6bu)
            C(19, (x ^ 0xdeadbeefu) * 0x9e3779b1u)
            C(20, (x + 0x6d2b79f5u) ^ (x << 13))
            C(21, (x * 0x51d7348du) + (x >> 11))
            C(22, (x ^ (x >> 7)) + 0x94d049bbu)
            C(23, (x + (x << 3)) ^ (x >> 13))
            C(24, (x * x) + 1u)
            C(25, (x * x) ^ (x >> 16))
            C(26, (x + 0x01234567u) * 0x9e3779b9u)
            C(27, ((x << 16) | (x >> 16)) + 0x7f4a7c15u)
            C(28, (x + (x << 10)) + (x >> 6))
            C(29, (x ^ (x << 5)) - (x >> 3))
            C(30, (x + 0x7f4a7c15u) ^ ((x << 7) | (x >> 25)))
            C(31, (x ^ ((x << 11) | (x >> 21))) * 0x85ebca6bu)
            C(32, x + 0x243f6a88u)
            C(33, x ^ 0x13198a2eu)
            C(34, x * 0x9e3779b1u)
            C(35, (x << 2) | (x >> 30))
            C(36, (x << 9) | (x >> 23))
            C(37, (x >> 7) | (x << 25))
            C(38, (x ^ (x >> 13)) * 0x4cf5ad43u)
            C(39, (x + (x << 7)) + 0x7f4a7c15u)
            C(40, x - 0x9e3779b9u)
            C(41, (x ^ (x << 11)) + 0x85ebca6bu)
            C(42, (x ^ (x >> 9)) * 0x27d4eb2du)
            C(43, (x + 0x3c6ef372u) ^ (x >> 17))
            C(44, x * 7u + 0x6d2b79f5u)
            C(45, x * 9u + 0x94d049bbu)
            C(46, x ^ (x << 3))
            C(47, x ^ (x >> 12))
            C(48, (x + 0x165667b1u) ^ (x << 8))
            C(49, (x * 0x1b873593u) ^ (x >> 16))
            C(50, (x + (x >> 1)) ^ 0x27d4eb2du)
            C(51, (x ^ 0xba5eba11u) * 0x85ebca6bu)
            C(52, (x + 0x7ed55d16u) ^ (x << 5))
            C(53, (x * 0xc2b2ae35u) + (x >> 7))
            C(54, (x ^ (x >> 3)) + 0x3c6ef372u)
            C(55, (x + (x << 2)) ^ (x >> 9))
            C(56, (x * x) + 0x9e3779b9u)
            C(57, (x * x) ^ (x >> 11))
            C(58, (x + 0xf00ba4d5u) * 0x27d4eb2du)
            C(59, ((x << 8) | (x >> 24)) + 0x13198a2eu)
            C(60, (x + (x << 6)) + (x >> 5))
            C(61, (x ^ (x << 7)) - (x >> 2))
            C(62, (x + 0x94d049bbu) ^ ((x << 9) | (x >> 23)))
            C(63, (x ^ ((x << 5) | (x >> 27))) * 0x4cf5ad43u)
#undef C
        }
        x ^= i * 0x27d4eb2du;
        x = x * 1664525u + 1013904223u;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64((uint64_t)x);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


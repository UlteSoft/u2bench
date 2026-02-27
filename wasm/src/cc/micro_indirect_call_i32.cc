#include "bench_common.h"

#include <stdint.h>

__attribute__((noinline)) static uint32_t f0(uint32_t x) {
    return x + 1u;
}
__attribute__((noinline)) static uint32_t f1(uint32_t x) {
    return (x ^ 0x9e3779b9u) + 0x7f4a7c15u;
}
__attribute__((noinline)) static uint32_t f2(uint32_t x) {
    return x * 1664525u + 1013904223u;
}
__attribute__((noinline)) static uint32_t f3(uint32_t x) {
    return (x << 7) | (x >> (32u - 7u));
}
__attribute__((noinline)) static uint32_t f4(uint32_t x) {
    return (x >> 3) ^ (x << 11);
}
__attribute__((noinline)) static uint32_t f5(uint32_t x) {
    return x + (x >> 16) + 0x85ebca6bu;
}
__attribute__((noinline)) static uint32_t f6(uint32_t x) {
    return (x ^ (x >> 15)) * 0x2c1b3c6du;
}
__attribute__((noinline)) static uint32_t f7(uint32_t x) {
    return (x ^ (x >> 13)) * 0xc2b2ae35u;
}

int main() {
    using Fn = uint32_t (*)(uint32_t);

    static Fn funcs[8] = {f0, f1, f2, f3, f4, f5, f6, f7};

    uint32_t x = 1u;
    constexpr uint32_t kIters = 4000000u;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        Fn fp = funcs[x & 7u];
        x = fp(x);
        fp = funcs[(x >> 3) & 7u];
        x = fp(x);
        x ^= i;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64((uint64_t)x);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


#pragma once

#include <stdint.h>
#include <stdio.h>
#include <time.h>

static inline uint64_t u2bench_now_ns() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static inline void u2bench_print_time_ns(uint64_t ns) {
    const double ms = (double)ns / 1000000.0;
    printf("Time: %.3f ms\n", ms);
}

static inline void u2bench_print_time_ms(double ms) {
    printf("Time: %.3f ms\n", ms);
}

static inline void u2bench_sink_u64(uint64_t v) {
    static volatile uint64_t sink = 0;
    sink ^= v + 0x9e3779b97f4a7c15ull;
}

static inline void u2bench_sink_f64(double v) {
    static volatile double sink = 0.0;
    sink += v;
}

static inline uint32_t u2bench_xorshift32(uint32_t* state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static inline uint64_t u2bench_splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ull;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
    return x ^ (x >> 31);
}


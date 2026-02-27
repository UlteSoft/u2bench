#include "bench_common.h"

#include <stdint.h>

int main() {
    constexpr uint32_t kIters = 12000000;

    uint32_t state = 1;
    double acc = 0.0;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        const uint32_t r = u2bench_xorshift32(&state);
        const int32_t v = (int32_t)(r & 0x3ffffu); // keep in-range for trunc

        const double d = (double)v * 0.000001 + acc;
        const int32_t w = (int32_t)d;

        acc += d * 1.0000001;
        acc -= (double)w * 0.000001;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_f64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


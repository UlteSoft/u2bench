#include "bench_common.h"

#include <math.h>
#include <stdint.h>

int main() {
    constexpr uint32_t kIters = 6000000;

    uint32_t state = 1;
    double x = 1.0;
    double acc = 0.0;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        const uint32_t r = u2bench_xorshift32(&state);
        const double a = (double)((r & 0xffffu) + 1u);

        x = x * 1.0000001 + (double)(i & 1023) * 0.0000001;
        acc += 1.0 / sqrt(a + x);
        acc *= 0.9999999;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_f64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


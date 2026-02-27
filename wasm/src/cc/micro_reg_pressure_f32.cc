#include "bench_common.h"

#include <stdint.h>

int main() {
    float a0 = 1.0f, a1 = 2.0f, a2 = 3.0f, a3 = 4.0f;
    float a4 = 5.0f, a5 = 6.0f, a6 = 7.0f, a7 = 8.0f;
    float a8 = 9.0f, a9 = 10.0f, a10 = 11.0f, a11 = 12.0f;

    float acc = 0.0f;
    constexpr uint32_t kIters = 12000000u;

    const uint64_t t0 = u2bench_now_ns();
    for (uint32_t i = 0; i < kIters; ++i) {
        const float x = (float)i * 0.000001f + (a0 - a7) * 0.01f;

        a0 = a0 + x + a1;
        a1 = a1 * 1.0000001f + x + a2;
        a2 = a2 - x * 0.9999997f + a3;
        a3 = a3 * 0.9999999f + x + a4;

        a4 = a4 + (a5 - x) * 0.5f;
        a5 = a5 * 1.0000003f + (a6 + x) * 0.25f;
        a6 = a6 - (a7 - x) * 0.125f + a0 * 0.001f;
        a7 = a7 * 0.9999991f + (a8 + x) * 0.0625f;

        a8 = a8 + (a9 + x) * 0.03125f;
        a9 = a9 * 1.0000002f + (a10 - x) * 0.015625f;
        a10 = a10 - (a11 + x) * 0.0078125f + a3 * 0.0001f;
        a11 = a11 * 0.9999998f + (a4 - x) * 0.00390625f;

        acc += (a0 + a5 + a10 + a11) * 0.00001f + x;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_f64((double)acc + (double)a0 + (double)a5 + (double)a10 + (double)a11);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


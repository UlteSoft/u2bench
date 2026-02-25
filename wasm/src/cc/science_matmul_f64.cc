#include "bench_common.h"

#include <stdint.h>

static constexpr int kN = 48;

alignas(16) static double a[kN * kN];
alignas(16) static double b[kN * kN];
alignas(16) static double c[kN * kN];

int main() {
    uint32_t rng = 1;
    for (int i = 0; i < kN * kN; ++i) {
        a[i] = (double)((int32_t)(u2bench_xorshift32(&rng) % 2001u) - 1000) * 0.001;
        b[i] = (double)((int32_t)(u2bench_xorshift32(&rng) % 2001u) - 1000) * 0.001;
        c[i] = 0.0;
    }

    constexpr int kReps = 25;
    const uint64_t t0 = u2bench_now_ns();
    for (int rep = 0; rep < kReps; ++rep) {
        for (int i = 0; i < kN; ++i) {
            for (int k = 0; k < kN; ++k) {
                const double aik = a[i * kN + k];
                for (int j = 0; j < kN; ++j) {
                    c[i * kN + j] += aik * b[k * kN + j];
                }
            }
        }
    }
    const uint64_t t1 = u2bench_now_ns();

    double sum = 0.0;
    for (int i = 0; i < kN * kN; ++i) {
        sum += c[i];
    }
    u2bench_sink_f64(sum);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


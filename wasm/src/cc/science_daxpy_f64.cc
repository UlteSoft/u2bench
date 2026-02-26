#include "bench_common.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

int main() {
    constexpr size_t kN = 150000;
    constexpr int kReps = 80;
    constexpr double a = 1.000001;

    double* x = (double*)malloc(kN * sizeof(double));
    double* y = (double*)malloc(kN * sizeof(double));
    if (!x || !y) {
        printf("malloc failed\n");
        return 1;
    }

    uint64_t seed = 1;
    for (size_t i = 0; i < kN; ++i) {
        seed = u2bench_splitmix64(seed);
        const int32_t v = (int32_t)(seed & 0xffff) - 32768;
        x[i] = (double)v * 0.001;
        y[i] = (double)(v ^ 0x5a5a) * 0.001;
    }

    const uint64_t t0 = u2bench_now_ns();
    for (int rep = 0; rep < kReps; ++rep) {
        for (size_t i = 0; i < kN; ++i) {
            y[i] = a * x[i] + y[i];
        }
    }
    const uint64_t t1 = u2bench_now_ns();

    double sum = 0.0;
    for (size_t i = 0; i < kN; i += 97) {
        sum += y[i];
    }

    free(x);
    free(y);
    u2bench_sink_f64(sum);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


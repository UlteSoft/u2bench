#include "bench_common.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

static inline double norm_cdf(double x) {
    // Abramowitz & Stegun 7.1.26 approximation.
    // max error ~ 7e-8 for typical inputs; good enough for benchmarking.
    const double a1 = 0.319381530;
    const double a2 = -0.356563782;
    const double a3 = 1.781477937;
    const double a4 = -1.821255978;
    const double a5 = 1.330274429;
    const double p = 0.2316419;
    const double inv_sqrt_2pi = 0.39894228040143267793994605993438; // 1/sqrt(2*pi)

    const double sign = x < 0.0 ? -1.0 : 1.0;
    const double ax = fabs(x);
    const double t = 1.0 / (1.0 + p * ax);
    const double poly = (((a5 * t + a4) * t + a3) * t + a2) * t + a1;
    const double pdf = inv_sqrt_2pi * exp(-0.5 * ax * ax);
    double cdf = 1.0 - pdf * poly * t;
    if (sign < 0.0) {
        cdf = 1.0 - cdf;
    }
    return cdf;
}

int main() {
    constexpr int kN = 20000;
    constexpr int kReps = 25;
    const double r = 0.03;

    double* S = (double*)malloc((size_t)kN * sizeof(double));
    double* K = (double*)malloc((size_t)kN * sizeof(double));
    double* T = (double*)malloc((size_t)kN * sizeof(double));
    double* V = (double*)malloc((size_t)kN * sizeof(double));
    if (!S || !K || !T || !V) {
        printf("malloc failed\n");
        return 1;
    }

    uint32_t rng = 1;
    for (int i = 0; i < kN; ++i) {
        const double u0 = (double)(u2bench_xorshift32(&rng) & 0xffffu) / 65535.0;
        const double u1 = (double)(u2bench_xorshift32(&rng) & 0xffffu) / 65535.0;
        const double u2 = (double)(u2bench_xorshift32(&rng) & 0xffffu) / 65535.0;
        const double u3 = (double)(u2bench_xorshift32(&rng) & 0xffffu) / 65535.0;

        S[i] = 80.0 + 40.0 * u0;
        K[i] = 80.0 + 40.0 * u1;
        T[i] = 0.10 + 2.00 * u2;
        V[i] = 0.05 + 0.50 * u3;
    }

    double sum = 0.0;
    const uint64_t t0 = u2bench_now_ns();
    for (int rep = 0; rep < kReps; ++rep) {
        for (int i = 0; i < kN; ++i) {
            const double s = S[i] * (1.0 + (double)rep * 1e-12);
            const double k = K[i];
            const double t = T[i];
            const double v = V[i];
            const double sqrt_t = sqrt(t);
            const double vt = v * sqrt_t;
            const double d1 = (log(s / k) + (r + 0.5 * v * v) * t) / vt;
            const double d2 = d1 - vt;
            const double disc = exp(-r * t);

            const double nd1 = norm_cdf(d1);
            const double nd2 = norm_cdf(d2);
            const double call = s * nd1 - k * disc * nd2;
            const double put = call + k * disc - s;
            sum += call + put;
        }
        sum *= 0.999999999;
    }
    const uint64_t t1 = u2bench_now_ns();

    free(V);
    free(T);
    free(K);
    free(S);
    u2bench_sink_f64(sum);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


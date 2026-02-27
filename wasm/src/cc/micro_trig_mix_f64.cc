#include "bench_common.h"

#include <math.h>
#include <stdint.h>

int main() {
    double x = 0.1;
    double y = 0.2;
    double acc = 0.0;
    uint32_t rng = 1;

    constexpr int kIters = 200000;
    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kIters; ++i) {
        const uint32_t r = u2bench_xorshift32(&rng);
        const double t = x + (double)(r & 0xFFFFu) * 0.000001;

        // sin/cos-heavy loop to stress libm + FP pipelines.
        const double s = sin(t);
        const double c = cos(t * 1.0000003);
        acc += s * c;

        // Keep inputs moving to avoid becoming periodic / constant-folded.
        x = x * 1.0000001 + 0.0000001 * (double)((r >> 16) & 0xFFFFu);
        y = y * 0.9999999 + 0.0000002 * (double)(r & 0xFFFFu);
        if (x > 10.0) {
            x -= 10.0;
        }
        if (y > 10.0) {
            y -= 10.0;
        }
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_f64(acc + x + y);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


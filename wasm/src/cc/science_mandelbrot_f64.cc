#include "bench_common.h"

#include <stdint.h>

int main() {
    constexpr int kW = 320;
    constexpr int kH = 240;
    constexpr int kMax = 200;

    const double x0 = -2.0;
    const double x1 = 1.0;
    const double y0 = -1.2;
    const double y1 = 1.2;

    uint64_t sum = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (int y = 0; y < kH; ++y) {
        const double ci = y0 + (y1 - y0) * (double)y / (double)(kH - 1);
        for (int x = 0; x < kW; ++x) {
            const double cr = x0 + (x1 - x0) * (double)x / (double)(kW - 1);
            double zr = 0.0;
            double zi = 0.0;
            int it = 0;
            while (it < kMax) {
                const double zr2 = zr * zr;
                const double zi2 = zi * zi;
                if (zr2 + zi2 > 4.0) {
                    break;
                }
                const double zr_new = zr2 - zi2 + cr;
                zi = (zr + zr) * zi + ci;
                zr = zr_new;
                ++it;
            }
            sum += (uint64_t)it;
        }
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(sum);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


#include "bench_common.h"

#include <math.h>
#include <stdint.h>

struct Body {
    double x;
    double y;
    double z;
    double vx;
    double vy;
    double vz;
    double m;
};

static inline double frand(uint64_t* s) {
    *s = u2bench_splitmix64(*s);
    const uint64_t x = *s;
    const double u = (double)(x & 0xffffffffu) / 4294967296.0;
    return u * 2.0 - 1.0;
}

int main() {
    constexpr int kN = 128;
    constexpr int kSteps = 20;
    constexpr double kDt = 0.01;
    constexpr double kEps = 1e-9;

    Body b[kN];
    uint64_t seed = 1;
    for (int i = 0; i < kN; ++i) {
        b[i].x = frand(&seed);
        b[i].y = frand(&seed);
        b[i].z = frand(&seed);
        b[i].vx = frand(&seed) * 0.1;
        b[i].vy = frand(&seed) * 0.1;
        b[i].vz = frand(&seed) * 0.1;
        b[i].m = 0.5 + (frand(&seed) + 1.0) * 0.25;
    }

    const uint64_t t0 = u2bench_now_ns();
    for (int step = 0; step < kSteps; ++step) {
        for (int i = 0; i < kN; ++i) {
            double ax = 0.0;
            double ay = 0.0;
            double az = 0.0;
            const double xi = b[i].x;
            const double yi = b[i].y;
            const double zi = b[i].z;

            for (int j = 0; j < kN; ++j) {
                if (j == i) {
                    continue;
                }
                const double dx = b[j].x - xi;
                const double dy = b[j].y - yi;
                const double dz = b[j].z - zi;
                const double d2 = dx * dx + dy * dy + dz * dz + kEps;
                const double inv = 1.0 / sqrt(d2);
                const double inv3 = inv * inv * inv;
                const double s = b[j].m * inv3;
                ax += dx * s;
                ay += dy * s;
                az += dz * s;
            }

            b[i].vx += ax * kDt;
            b[i].vy += ay * kDt;
            b[i].vz += az * kDt;
        }

        for (int i = 0; i < kN; ++i) {
            b[i].x += b[i].vx * kDt;
            b[i].y += b[i].vy * kDt;
            b[i].z += b[i].vz * kDt;
        }
    }
    const uint64_t t1 = u2bench_now_ns();

    double sum = 0.0;
    for (int i = 0; i < kN; ++i) {
        sum += b[i].x + b[i].y + b[i].z;
    }

    u2bench_sink_f64(sum);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


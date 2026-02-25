#include "bench_common.h"

int main() {
    constexpr int kIters = 200000;

    struct timespec ts;
    uint64_t acc = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kIters; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        acc += (uint64_t)ts.tv_nsec;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


#include "bench_common.h"

#include <stdint.h>

static constexpr int kLimit = 2000000;
static uint8_t is_prime[kLimit + 1];

int main() {
    for (int i = 0; i <= kLimit; ++i) {
        is_prime[i] = 1;
    }
    is_prime[0] = 0;
    is_prime[1] = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (int p = 2; (int64_t)p * (int64_t)p <= kLimit; ++p) {
        if (!is_prime[p]) {
            continue;
        }
        for (int j = p * p; j <= kLimit; j += p) {
            is_prime[j] = 0;
        }
    }
    uint32_t count = 0;
    for (int i = 2; i <= kLimit; ++i) {
        count += (uint32_t)is_prime[i];
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64((uint64_t)count);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


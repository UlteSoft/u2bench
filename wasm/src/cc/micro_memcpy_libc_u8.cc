#include "bench_common.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main() {
    constexpr size_t kN = 4u * 1024u * 1024u;
    constexpr int kReps = 32; // total copy: 128 MiB (plus small mutations)

    uint8_t* a = (uint8_t*)malloc(kN);
    uint8_t* b = (uint8_t*)malloc(kN);
    if (!a || !b) {
        printf("malloc failed\n");
        return 1;
    }

    uint32_t state = 1;
    for (size_t i = 0; i < kN; ++i) {
        a[i] = (uint8_t)u2bench_xorshift32(&state);
    }

    uint64_t acc = 0;
    const uint64_t t0 = u2bench_now_ns();
    for (int rep = 0; rep < kReps; ++rep) {
        memcpy(b, a, kN);

        // Make each iteration's source depend on the previous output.
        const size_t i0 = ((size_t)rep * 1315423911u) & (kN - 1);
        const size_t i1 = ((size_t)rep * 2654435761u + 97u) & (kN - 1);
        b[i0] ^= (uint8_t)(rep * 17);
        b[i1] += (uint8_t)(rep * 3);
        acc += (uint64_t)b[i0] + (uint64_t)b[i1];

        uint8_t* tmp = a;
        a = b;
        b = tmp;
    }
    const uint64_t t1 = u2bench_now_ns();

    // One more pass to ensure the buffer is observable.
    for (size_t i = 0; i < kN; i += 64) {
        acc += a[i];
    }

    free(b);
    free(a);
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


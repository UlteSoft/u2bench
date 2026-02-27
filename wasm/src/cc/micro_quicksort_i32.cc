#include "bench_common.h"

#include <stdint.h>
#include <stdlib.h>

static inline void swap_i32(int32_t* a, int32_t* b) {
    const int32_t t = *a;
    *a = *b;
    *b = t;
}

static inline int32_t median3(int32_t a, int32_t b, int32_t c) {
    if (a < b) {
        if (b < c) {
            return b;
        }
        return (a < c) ? c : a;
    }
    if (a < c) {
        return a;
    }
    return (b < c) ? c : b;
}

static void quicksort_iter(int32_t* a, int n) {
    struct Range {
        int l;
        int r;
    };
    Range stack[64];
    int sp = 0;
    stack[sp++] = {0, n - 1};

    while (sp > 0) {
        Range rg = stack[--sp];
        int l = rg.l;
        int r = rg.r;

        while (l < r) {
            int i = l;
            int j = r;
            const int m = l + ((r - l) >> 1);
            const int32_t pivot = median3(a[l], a[m], a[r]);

            while (i <= j) {
                while (a[i] < pivot) {
                    ++i;
                }
                while (a[j] > pivot) {
                    --j;
                }
                if (i <= j) {
                    swap_i32(&a[i], &a[j]);
                    ++i;
                    --j;
                }
            }

            // Sort smaller side first; push larger.
            if (j - l < r - i) {
                if (i < r) {
                    stack[sp++] = {i, r};
                }
                r = j;
            } else {
                if (l < j) {
                    stack[sp++] = {l, j};
                }
                l = i;
            }
        }
    }
}

int main() {
    constexpr int kN = 200000;
    constexpr int kReps = 3;

    int32_t* base = (int32_t*)malloc((size_t)kN * sizeof(int32_t));
    int32_t* work = (int32_t*)malloc((size_t)kN * sizeof(int32_t));
    if (!base || !work) {
        printf("malloc failed\n");
        return 1;
    }

    uint32_t rng = 1;
    for (int i = 0; i < kN; ++i) {
        base[i] = (int32_t)(u2bench_xorshift32(&rng) ^ (uint32_t)i);
    }

    uint64_t acc = 0;
    const uint64_t t0 = u2bench_now_ns();
    for (int rep = 0; rep < kReps; ++rep) {
        for (int i = 0; i < kN; ++i) {
            work[i] = base[i] ^ (rep * 0x9e3779b9);
        }

        quicksort_iter(work, kN);

        acc += (uint64_t)(uint32_t)work[(rep * 9973) % kN];
        acc += (uint64_t)(uint32_t)work[kN / 2];
        acc += (uint64_t)(uint32_t)work[kN - 1];
    }
    const uint64_t t1 = u2bench_now_ns();

    free(work);
    free(base);
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


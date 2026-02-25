#include "bench_common.h"

#include <stdlib.h>

static inline uint32_t hash32(uint64_t x) {
    return (uint32_t)u2bench_splitmix64(x);
}

struct Table {
    uint32_t cap;
    uint32_t mask;
    uint64_t* keys;
    uint64_t* vals;
};

static constexpr uint64_t kEmpty = 0;

static void table_init(Table* t, uint32_t cap_pow2) {
    t->cap = cap_pow2;
    t->mask = cap_pow2 - 1;
    t->keys = (uint64_t*)calloc((size_t)cap_pow2, sizeof(uint64_t));
    t->vals = (uint64_t*)calloc((size_t)cap_pow2, sizeof(uint64_t));
}

static void table_free(Table* t) {
    free(t->keys);
    free(t->vals);
}

static inline void table_put(Table* t, uint64_t key, uint64_t val) {
    uint32_t i = hash32(key) & t->mask;
    for (;;) {
        const uint64_t k = t->keys[i];
        if (k == kEmpty || k == key) {
            t->keys[i] = key;
            t->vals[i] = val;
            return;
        }
        i = (i + 1) & t->mask;
    }
}

static inline uint64_t table_get(const Table* t, uint64_t key) {
    uint32_t i = hash32(key) & t->mask;
    for (;;) {
        const uint64_t k = t->keys[i];
        if (k == key) {
            return t->vals[i];
        }
        if (k == kEmpty) {
            return 0;
        }
        i = (i + 1) & t->mask;
    }
}

int main() {
    Table t;
    table_init(&t, 1u << 18);

    constexpr uint32_t kN = 90000;
    constexpr uint32_t kOps = 600000;

    uint64_t seed = 1;
    uint64_t* keys = (uint64_t*)malloc((size_t)kN * sizeof(uint64_t));
    for (uint32_t i = 0; i < kN; ++i) {
        seed = u2bench_splitmix64(seed);
        keys[i] = seed | 1ull;
    }

    const uint64_t t0 = u2bench_now_ns();

    for (uint32_t i = 0; i < kN; ++i) {
        table_put(&t, keys[i], (uint64_t)i * 2654435761ull);
    }

    uint64_t sum = 0;
    for (uint32_t i = 0; i < kOps; ++i) {
        const uint64_t k = keys[(uint32_t)(u2bench_splitmix64((uint64_t)i) % kN)];
        sum += table_get(&t, k);
        if ((i & 7u) == 0u) {
            table_put(&t, k, sum);
        }
        if ((i & 31u) == 0u) {
            sum += table_get(&t, k ^ 0xfeedbeefcafebabeull); // miss
        }
    }

    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(sum);
    u2bench_print_time_ns(t1 - t0);

    table_free(&t);
    free(keys);
    return 0;
}


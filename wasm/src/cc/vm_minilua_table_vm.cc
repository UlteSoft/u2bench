#include "bench_common.h"

#include <stdlib.h>

enum Op : uint8_t {
    OP_LOADI = 0,
    OP_ADD = 1,
    OP_SUB = 2,
    OP_MUL = 3,
    OP_GETTAB = 4,
    OP_SETTAB = 5,
    OP_JNZ = 6,
    OP_HALT = 7,
};

struct Inst {
    uint8_t op;
    uint8_t a;
    uint8_t b;
    uint8_t c;
    int32_t imm;
};

struct Table {
    uint32_t cap;
    uint32_t mask;
    uint64_t* keys;
    uint64_t* vals;
};

static constexpr uint64_t kEmpty = 0;

static inline uint32_t hash32(uint64_t x) {
    return (uint32_t)u2bench_splitmix64(x);
}

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

static inline uint64_t run_vm(const Inst* prog, uint32_t len, Table* tab) {
    uint64_t r[16] = {};
    uint32_t pc = 0;
    for (;;) {
        const Inst ins = prog[pc++];
        switch (ins.op) {
        case OP_LOADI:
            r[ins.a] = (uint64_t)(int64_t)ins.imm;
            break;
        case OP_ADD:
            r[ins.a] = r[ins.b] + r[ins.c];
            break;
        case OP_SUB:
            r[ins.a] = r[ins.b] - r[ins.c];
            break;
        case OP_MUL:
            r[ins.a] = r[ins.b] * r[ins.c];
            break;
        case OP_SETTAB:
            table_put(tab, r[ins.a], r[ins.b]);
            break;
        case OP_GETTAB:
            r[ins.a] = table_get(tab, r[ins.b]);
            break;
        case OP_JNZ:
            if (r[ins.a] != 0) {
                pc = (uint32_t)((int32_t)pc + ins.imm);
            }
            break;
        case OP_HALT:
            return r[ins.a];
        default:
            return r[0];
        }
        if (pc >= len) {
            return r[0];
        }
    }
}

int main() {
    // A tiny Lua-like workload: loop, arithmetic, table set/get, and a running sum.
    static constexpr Inst kProg[] = {
        {OP_LOADI, 7, 0, 0, 1},       // r7 = 1
        {OP_LOADI, 0, 0, 0, 1},       // r0 = key (starts at 1)
        {OP_LOADI, 1, 0, 0, 60000},   // r1 = remaining
        {OP_LOADI, 4, 0, 0, 3},       // r4 = 3
        {OP_LOADI, 5, 0, 0, 1},       // r5 = 1
        {OP_LOADI, 6, 0, 0, 0},       // r6 = sum

        // loop:
        {OP_MUL, 2, 0, 4, 0},         // r2 = key * 3
        {OP_ADD, 2, 2, 5, 0},         // r2 = r2 + 1
        {OP_SETTAB, 0, 2, 0, 0},      // tab[key] = r2
        {OP_GETTAB, 3, 0, 0, 0},      // r3 = tab[key]
        {OP_ADD, 6, 6, 3, 0},         // sum += r3
        {OP_ADD, 0, 0, 7, 0},         // key++
        {OP_SUB, 1, 1, 7, 0},         // remaining--
        {OP_JNZ, 1, 0, 0, -8},        // if remaining != 0 goto loop
        {OP_HALT, 6, 0, 0, 0},        // return sum
    };

    Table tab;
    table_init(&tab, 1u << 18);

    const uint64_t t0 = u2bench_now_ns();
    const uint64_t sum = run_vm(kProg, (uint32_t)(sizeof(kProg) / sizeof(kProg[0])), &tab);
    const uint64_t t1 = u2bench_now_ns();

    table_free(&tab);
    u2bench_sink_u64(sum);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


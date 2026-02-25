#include "bench_common.h"

#include <stdint.h>

enum Op : uint8_t {
    OP_LOADI = 0,
    OP_ADD = 1,
    OP_XOR = 2,
    OP_MUL = 3,
    OP_SHR = 4,
    OP_SUB = 5,
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

static inline int64_t run_vm(const Inst* prog, uint32_t len, int64_t seed) {
    int64_t r[8] = {};
    r[0] = seed;
    r[1] = 0x123456789abcdefll;

    uint32_t pc = 0;
    for (;;) {
        const Inst ins = prog[pc++];
        switch (ins.op) {
        case OP_LOADI:
            r[ins.a] = (int64_t)ins.imm;
            break;
        case OP_ADD:
            r[ins.a] = r[ins.b] + r[ins.c];
            break;
        case OP_XOR:
            r[ins.a] = r[ins.b] ^ r[ins.c];
            break;
        case OP_MUL:
            r[ins.a] = r[ins.b] * r[ins.c];
            break;
        case OP_SHR:
            r[ins.a] = (int64_t)((uint64_t)r[ins.b] >> (uint32_t)(r[ins.c] & 63));
            break;
        case OP_SUB:
            r[ins.a] = r[ins.b] - r[ins.c];
            break;
        case OP_JNZ:
            if (r[ins.a] != 0) {
                pc = (uint32_t)((int32_t)pc + ins.imm);
            }
            break;
        case OP_HALT:
            return r[ins.a] + r[0] + r[1];
        default:
            return 0;
        }
        if (pc >= len) {
            return r[0];
        }
    }
}

int main() {
    static constexpr Inst kProg[] = {
        {OP_LOADI, 7, 0, 0, 1},          // r7 = 1
        {OP_LOADI, 2, 0, 0, 1500},       // r2 = loop
        {OP_LOADI, 3, 0, 0, 13},         // r3 = 13
        {OP_LOADI, 4, 0, 0, 1664525},    // r4 = mul
        {OP_LOADI, 5, 0, 0, 1013904223}, // r5 = add

        // loop:
        {OP_MUL, 0, 0, 4, 0},            // r0 = r0 * r4
        {OP_ADD, 0, 0, 5, 0},            // r0 = r0 + r5
        {OP_SHR, 6, 0, 3, 0},            // r6 = r0 >> r3
        {OP_XOR, 1, 1, 6, 0},            // r1 ^= r6
        {OP_SUB, 2, 2, 7, 0},            // r2--
        {OP_JNZ, 2, 0, 0, -6},           // if r2 != 0 jump back to loop
        {OP_HALT, 1, 0, 0, 0},           // return r1
    };

    uint64_t seed = 1;
    constexpr int kOuter = 1200;

    const uint64_t t0 = u2bench_now_ns();
    uint64_t acc = 0;
    for (int i = 0; i < kOuter; ++i) {
        seed = u2bench_splitmix64(seed);
        acc ^= (uint64_t)run_vm(kProg, (uint32_t)(sizeof(kProg) / sizeof(kProg[0])), (int64_t)seed);
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}

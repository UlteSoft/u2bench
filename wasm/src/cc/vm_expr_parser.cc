#include "bench_common.h"

#include <stdint.h>

enum TokKind : uint8_t {
    TOK_NUM = 0,
    TOK_VAR = 1, // a,b,c
    TOK_OP = 2,  // + - * /
};

struct Tok {
    TokKind kind;
    char op; // for TOK_OP or TOK_VAR
    int64_t num;
};

static inline int prec(char op) {
    if (op == '*' || op == '/') {
        return 2;
    }
    if (op == '+' || op == '-') {
        return 1;
    }
    return 0;
}

static inline int64_t var_value(char v, int64_t a, int64_t b, int64_t c) {
    if (v == 'a') {
        return a;
    }
    if (v == 'b') {
        return b;
    }
    return c;
}

static inline bool is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

static inline bool is_space(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

static int to_rpn(const char* expr, Tok* out, int out_cap, char* opstk, int op_cap) {
    int out_n = 0;
    int op_n = 0;
    for (int i = 0; expr[i] != 0; ++i) {
        const char ch = expr[i];
        if (is_space(ch)) {
            continue;
        }
        if (is_digit(ch)) {
            int64_t v = 0;
            while (is_digit(expr[i])) {
                v = v * 10 + (expr[i] - '0');
                ++i;
            }
            --i;
            if (out_n < out_cap) {
                out[out_n++] = Tok{TOK_NUM, 0, v};
            }
            continue;
        }
        if (ch == 'a' || ch == 'b' || ch == 'c') {
            if (out_n < out_cap) {
                out[out_n++] = Tok{TOK_VAR, ch, 0};
            }
            continue;
        }
        if (ch == '(') {
            if (op_n < op_cap) {
                opstk[op_n++] = ch;
            }
            continue;
        }
        if (ch == ')') {
            while (op_n > 0 && opstk[op_n - 1] != '(') {
                const char op = opstk[--op_n];
                if (out_n < out_cap) {
                    out[out_n++] = Tok{TOK_OP, op, 0};
                }
            }
            if (op_n > 0 && opstk[op_n - 1] == '(') {
                --op_n;
            }
            continue;
        }
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            while (op_n > 0) {
                const char top = opstk[op_n - 1];
                if (top == '(') {
                    break;
                }
                if (prec(top) >= prec(ch)) {
                    --op_n;
                    if (out_n < out_cap) {
                        out[out_n++] = Tok{TOK_OP, top, 0};
                    }
                    continue;
                }
                break;
            }
            if (op_n < op_cap) {
                opstk[op_n++] = ch;
            }
            continue;
        }
    }
    while (op_n > 0) {
        const char op = opstk[--op_n];
        if (op == '(') {
            continue;
        }
        if (out_n < out_cap) {
            out[out_n++] = Tok{TOK_OP, op, 0};
        }
    }
    return out_n;
}

static int64_t eval_rpn(const Tok* rpn, int n, int64_t a, int64_t b, int64_t c) {
    int64_t stk[64];
    int sp = 0;
    for (int i = 0; i < n; ++i) {
        const Tok t = rpn[i];
        if (t.kind == TOK_NUM) {
            stk[sp++] = t.num;
        } else if (t.kind == TOK_VAR) {
            stk[sp++] = var_value(t.op, a, b, c);
        } else {
            const int64_t rhs = stk[--sp];
            const int64_t lhs = stk[--sp];
            int64_t r = 0;
            switch (t.op) {
            case '+':
                r = lhs + rhs;
                break;
            case '-':
                r = lhs - rhs;
                break;
            case '*':
                r = lhs * rhs;
                break;
            case '/':
                r = lhs / (rhs == 0 ? 1 : rhs);
                break;
            default:
                r = lhs;
                break;
            }
            stk[sp++] = r;
        }
    }
    return sp ? stk[sp - 1] : 0;
}

int main() {
    // A small Lua-ish workload: repeatedly parse+eval an arithmetic expression with variables.
    const char* expr = "((a*3 + b*5) * (c+7) - (a*b) + (c*c) - 12345) / 3";

    Tok rpn[96];
    char opstk[64];

    constexpr int kIters = 30000;
    uint64_t seed = 1;
    uint64_t acc = 0;

    const uint64_t t0 = u2bench_now_ns();
    for (int i = 0; i < kIters; ++i) {
        seed = u2bench_splitmix64(seed);
        const int64_t a = (int64_t)(seed & 0xffff);
        const int64_t b = (int64_t)((seed >> 16) & 0xffff);
        const int64_t c = (int64_t)((seed >> 32) & 0xffff);

        const int n = to_rpn(expr, rpn, (int)(sizeof(rpn) / sizeof(rpn[0])), opstk, (int)(sizeof(opstk) / sizeof(opstk[0])));
        const int64_t v = eval_rpn(rpn, n, a, b, c);
        acc ^= (uint64_t)v;
    }
    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


#include "bench_common.h"

#include <stdint.h>
#include <stdlib.h>

static inline bool is_ws(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

static inline bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static inline uint64_t tokenize_json(const char* p, size_t n) {
    uint64_t strings = 0;
    uint64_t numbers = 0;
    uint64_t structurals = 0;
    uint64_t literals = 0;
    uint64_t escapes = 0;

    size_t i = 0;
    while (i < n) {
        const char c = p[i];
        if ((uint8_t)c <= 0x20u && is_ws(c)) {
            ++i;
            continue;
        }
        switch (c) {
            case '{':
            case '}':
            case '[':
            case ']':
            case ':':
            case ',':
                ++structurals;
                ++i;
                break;
            case '"': {
                ++strings;
                ++i;
                while (i < n) {
                    const char ch = p[i++];
                    if (ch == '"') {
                        break;
                    }
                    if (ch == '\\') {
                        ++escapes;
                        if (i >= n) break;
                        const char e = p[i++];
                        if (e == 'u') {
                            // Skip 4 hex digits (assume well-formed input).
                            i += 4;
                        }
                    }
                }
                break;
            }
            default:
                if (c == '-' || is_digit(c)) {
                    ++numbers;
                    ++i;
                    while (i < n && is_digit(p[i])) ++i;
                    if (i < n && p[i] == '.') {
                        ++i;
                        while (i < n && is_digit(p[i])) ++i;
                    }
                    if (i < n && (p[i] == 'e' || p[i] == 'E')) {
                        ++i;
                        if (i < n && (p[i] == '+' || p[i] == '-')) ++i;
                        while (i < n && is_digit(p[i])) ++i;
                    }
                    break;
                }
                // true/false/null
                if (c == 't' && i + 3 < n && p[i + 1] == 'r' && p[i + 2] == 'u' && p[i + 3] == 'e') {
                    ++literals;
                    i += 4;
                    break;
                }
                if (c == 'f' && i + 4 < n && p[i + 1] == 'a' && p[i + 2] == 'l' && p[i + 3] == 's' && p[i + 4] == 'e') {
                    ++literals;
                    i += 5;
                    break;
                }
                if (c == 'n' && i + 3 < n && p[i + 1] == 'u' && p[i + 2] == 'l' && p[i + 3] == 'l') {
                    ++literals;
                    i += 4;
                    break;
                }
                ++i;
                break;
        }
    }

    // Mix counts.
    return (strings * 1315423911ull) ^ (numbers * 2654435761ull) ^ (structurals * 97531ull) ^ (literals * 99991ull) ^
           (escapes * 1337ull);
}

int main() {
    static const char kChunk[] =
        "{\"id\":123456,\"name\":\"alice\\\\n\\u263A\",\"vals\":[1,2,3,4,5,6,7,8],\"ok\":true,\"n\":null},";
    constexpr size_t kTarget = 2u * 1024u * 1024u;
    constexpr int kReps = 25;

    char* buf = (char*)malloc(kTarget + 2);
    if (!buf) {
        printf("malloc failed\n");
        return 1;
    }

    size_t off = 0;
    buf[off++] = '[';
    while (off + sizeof(kChunk) < kTarget) {
        for (size_t i = 0; i < sizeof(kChunk) - 1; ++i) {
            buf[off++] = kChunk[i];
        }
    }
    if (off > 0 && buf[off - 1] == ',') {
        buf[off - 1] = ']';
    } else {
        buf[off++] = ']';
    }
    buf[off] = '\0';

    uint64_t acc = 0;
    const uint64_t t0 = u2bench_now_ns();
    for (int rep = 0; rep < kReps; ++rep) {
        acc ^= tokenize_json(buf, off);
        acc += (uint64_t)rep * 0x9e3779b97f4a7c15ull;
    }
    const uint64_t t1 = u2bench_now_ns();

    free(buf);
    u2bench_sink_u64(acc);
    u2bench_print_time_ns(t1 - t0);
    return 0;
}


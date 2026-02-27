#include "bench_common.h"

#include <stdint.h>
#include <stdlib.h>

static constexpr int kT = 16;                       // minimum degree
static constexpr int kMaxKeys = 2 * kT - 1;         // 31
static constexpr int kMaxChildren = 2 * kT;         // 32
static constexpr uint32_t kInvalidNode = 0xffffffffu;

struct Node {
    uint16_t n;
    uint8_t leaf;
    uint8_t _pad;
    uint64_t keys[kMaxKeys];
    uint32_t child[kMaxChildren];
};

struct BTree {
    Node* nodes;
    uint32_t cap;
    uint32_t next;
    uint32_t root;
};

static uint32_t new_node(BTree* t, bool leaf) {
    if (t->next >= t->cap) {
        printf("node pool exhausted\n");
        return kInvalidNode;
    }
    const uint32_t idx = t->next++;
    Node* n = &t->nodes[idx];
    n->n = 0;
    n->leaf = leaf ? 1 : 0;
    for (int i = 0; i < kMaxChildren; ++i) {
        n->child[i] = 0;
    }
    return idx;
}

static void split_child(BTree* t, uint32_t x_idx, int i, uint32_t y_idx) {
    Node* x = &t->nodes[x_idx];
    Node* y = &t->nodes[y_idx];

    const uint32_t z_idx = new_node(t, y->leaf != 0);
    if (z_idx == kInvalidNode) {
        return;
    }
    Node* z = &t->nodes[z_idx];

    z->n = (uint16_t)(kT - 1);

    for (int j = 0; j < kT - 1; ++j) {
        z->keys[j] = y->keys[j + kT];
    }
    if (!y->leaf) {
        for (int j = 0; j < kT; ++j) {
            z->child[j] = y->child[j + kT];
        }
    }

    y->n = (uint16_t)(kT - 1);

    for (int j = (int)x->n; j >= i + 1; --j) {
        x->child[j + 1] = x->child[j];
    }
    x->child[i + 1] = z_idx;

    for (int j = (int)x->n - 1; j >= i; --j) {
        x->keys[j + 1] = x->keys[j];
    }
    x->keys[i] = y->keys[kT - 1];
    x->n = (uint16_t)(x->n + 1);
}

static void insert_nonfull(BTree* t, uint32_t x_idx, uint64_t k) {
    Node* x = &t->nodes[x_idx];
    int i = (int)x->n - 1;

    if (x->leaf) {
        while (i >= 0 && k < x->keys[i]) {
            x->keys[i + 1] = x->keys[i];
            --i;
        }
        x->keys[i + 1] = k;
        x->n = (uint16_t)(x->n + 1);
        return;
    }

    while (i >= 0 && k < x->keys[i]) {
        --i;
    }
    ++i;

    const uint32_t c_idx = x->child[i];
    Node* c = &t->nodes[c_idx];
    if ((int)c->n == kMaxKeys) {
        split_child(t, x_idx, i, c_idx);
        x = &t->nodes[x_idx];
        if (k > x->keys[i]) {
            ++i;
        }
    }
    insert_nonfull(t, x->child[i], k);
}

static void btree_init(BTree* t, uint32_t node_cap) {
    t->nodes = (Node*)calloc((size_t)node_cap, sizeof(Node));
    t->cap = node_cap;
    t->next = 0;
    t->root = new_node(t, true);
}

static void btree_free(BTree* t) {
    free(t->nodes);
    t->nodes = nullptr;
    t->cap = 0;
    t->next = 0;
    t->root = 0;
}

static void btree_insert(BTree* t, uint64_t k) {
    const uint32_t r_idx = t->root;
    Node* r = &t->nodes[r_idx];
    if ((int)r->n != kMaxKeys) {
        insert_nonfull(t, r_idx, k);
        return;
    }

    const uint32_t s_idx = new_node(t, false);
    if (s_idx == kInvalidNode) {
        return;
    }
    Node* s = &t->nodes[s_idx];
    s->child[0] = r_idx;
    t->root = s_idx;
    split_child(t, s_idx, 0, r_idx);
    insert_nonfull(t, s_idx, k);
}

static inline bool btree_search(const BTree* t, uint64_t k, uint64_t* out) {
    uint32_t x_idx = t->root;
    for (;;) {
        const Node* x = &t->nodes[x_idx];
        int i = 0;
        const int n = (int)x->n;
        while (i < n && k > x->keys[i]) {
            ++i;
        }
        if (i < n && k == x->keys[i]) {
            if (out) {
                *out = x->keys[i];
            }
            return true;
        }
        if (x->leaf) {
            return false;
        }
        x_idx = x->child[i];
    }
}

int main() {
    BTree t;
    btree_init(&t, 20000);
    if (!t.nodes) {
        printf("calloc failed\n");
        return 1;
    }

    constexpr uint32_t kNKeys = 100000;
    constexpr uint32_t kOps = 600000;

    uint64_t* keys = (uint64_t*)malloc((size_t)kNKeys * sizeof(uint64_t));
    if (!keys) {
        printf("malloc failed\n");
        btree_free(&t);
        return 1;
    }

    uint64_t seed = 1;
    for (uint32_t i = 0; i < kNKeys; ++i) {
        seed = u2bench_splitmix64(seed);
        keys[i] = (seed ^ ((uint64_t)i << 1)) | 1ull;
    }

    const uint64_t t0 = u2bench_now_ns();

    for (uint32_t i = 0; i < kNKeys; ++i) {
        btree_insert(&t, keys[i]);
    }

    uint64_t sum = 0;
    for (uint32_t i = 0; i < kOps; ++i) {
        const uint64_t k = keys[(uint32_t)(u2bench_splitmix64((uint64_t)i) % kNKeys)];
        uint64_t out = 0;
        if (btree_search(&t, k, &out)) {
            sum += out;
        }
        if ((i & 31u) == 0u) {
            if (!btree_search(&t, k ^ 0xfeedbeefcafebabeull, &out)) {
                sum ^= out + 0x9e3779b97f4a7c15ull;
            }
        }
    }

    const uint64_t t1 = u2bench_now_ns();

    u2bench_sink_u64(sum);
    u2bench_print_time_ns(t1 - t0);

    btree_free(&t);
    free(keys);
    return 0;
}


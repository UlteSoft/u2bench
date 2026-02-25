# wasm/src

Sources for the generated MVP-ish WASI benchmark corpus under `wasm/corpus/`.

All benchmarks report **internal time** by printing a line like:

```
Time: <ms> ms
```

The time is measured inside the wasm guest using WASI clocks (`clock_gettime` in C/C++, or `clock_time_get` in WAT),
so `runbench.py --metric=internal` can compare engines while minimizing host-side timing noise.

## Benchmarks

### C/C++ (`wasm/src/cc/`)

- `wasi_clock_gettime.cc`: syscall-ish overhead of repeated `clock_gettime`.
- `wasi_file_rw.cc`: simple WASI filesystem workload (write+read 8 MiB).
- `crypto_sha256.cc`: SHA-256 compression rounds.
- `crypto_chacha20.cc`: ChaCha20 block function.
- `db_kv_hash.cc`: in-memory open-addressing KV hash table (puts/gets, hits+misses).
- `vm_tinybytecode.cc`: tiny bytecode interpreter (Lua-like VM style: switch dispatch + jumps).
- `science_matmul_i32.cc`: int32 matrix multiply.
- `science_matmul_f64.cc`: float64 matrix multiply.
- `science_sieve_i32.cc`: prime sieve up to 2,000,000.

### WAT (`wasm/src/wat/`)

- `loop_i32.wat`: hand-written tight loop micro-benchmark + WASI timing + `fd_write` output.


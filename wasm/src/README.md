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
- `wasi_small_io.cc`: many small WASI reads/writes (64B * 100k).
- `wasi_open_close_stat.cc`: repeated open+fstat+close of a small file.
- `wasi_random_get.cc`: WASI `random_get` throughput/overhead (16 MiB total).
- `crypto_sha256.cc`: SHA-256 compression rounds.
- `crypto_chacha20.cc`: ChaCha20 block function.
- `crypto_aes128.cc`: AES-128 encrypt many blocks.
- `crypto_crc32.cc`: CRC32 over a 4 MiB buffer, repeated 8 times (32 MiB).
- `crypto_siphash24.cc`: SipHash-2-4 over many 64-bit keys.
- `db_kv_hash.cc`: in-memory open-addressing KV hash table (puts/gets, hits+misses).
- `db_radix_sort_u64.cc`: radix sort 200k u64 values.
- `db_bloom_filter.cc`: Bloom filter insert + membership queries.
- `vm_tinybytecode.cc`: tiny bytecode interpreter (Lua-like VM style: switch dispatch + jumps).
- `vm_minilua_table_vm.cc`: Lua-ish bytecode VM with table set/get.
- `vm_expr_parser.cc`: parse+eval a small arithmetic expression (Lua-ish frontend work).
- `science_matmul_i32.cc`: int32 matrix multiply.
- `science_matmul_f64.cc`: float64 matrix multiply.
- `science_sieve_i32.cc`: prime sieve up to 2,000,000.
- `science_gcd_i64.cc`: Euclid GCD over many random pairs (integer compute).
- `science_daxpy_f64.cc`: DAXPY-like vector kernel (float compute + memory bandwidth).
- `science_mandelbrot_f64.cc`: Mandelbrot set iterations (branchy float compute).
- `science_nbody_f64.cc`: simple N-body simulation (float + sqrt).

### WAT (`wasm/src/wat/`)

- `loop_i32.wat`: hand-written tight loop micro-benchmark + WASI timing + `fd_write` output.
- `loop_f64.wat`: hand-written float loop micro-benchmark.
- `mem_sum_i32.wat`: memory read bandwidth (sum 1 MiB of i32s for many reps).
- `mem_fill_i32.wat`: memory write bandwidth (store 1 MiB of i32s for many reps).
- `local_dense_i32.wat`: `local.get`/`local.set` heavy loop.
- `operand_stack_dense_i32.wat`: deep operand-stack push/reduce loop.
- `call_dense_i32.wat`: indirect call overhead micro-benchmark.
- `control_flow_dense_i32.wat`: branch-heavy control-flow micro-benchmark.

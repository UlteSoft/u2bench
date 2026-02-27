(module
  (import "wasi_snapshot_preview1" "fd_write"
    (func $fd_write (param i32 i32 i32 i32) (result i32)))
  (import "wasi_snapshot_preview1" "clock_time_get"
    (func $clock_time_get (param i32 i64 i32) (result i32)))
  (import "wasi_snapshot_preview1" "proc_exit"
    (func $proc_exit (param i32)))

  (memory (export "memory") 2)

  (func $write (param $ptr i32) (param $len i32)
    (i32.store (i32.const 0) (local.get $ptr))
    (i32.store (i32.const 4) (local.get $len))
    (call $fd_write (i32.const 1) (i32.const 0) (i32.const 1) (i32.const 8))
    drop)

  (func $write_u64_dec (param $n i64) (param $out i32) (result i32)
    (local $scratch i32)
    (local $p i32)
    (local $len i32)
    (local $i i32)

    (if (i64.eq (local.get $n) (i64.const 0))
      (then
        (i32.store8 (local.get $out) (i32.const 48))
        (return (i32.const 1))))

    (local.set $scratch (i32.add (local.get $out) (i32.const 32)))
    (local.set $p (local.get $scratch))

    (block $done
      (loop $loop
        (br_if $done (i64.eq (local.get $n) (i64.const 0)))
        (local.set $p (i32.sub (local.get $p) (i32.const 1)))
        (i32.store8
          (local.get $p)
          (i32.add
            (i32.wrap_i64 (i64.rem_u (local.get $n) (i64.const 10)))
            (i32.const 48)))
        (local.set $n (i64.div_u (local.get $n) (i64.const 10)))
        (br $loop)))

    (local.set $len (i32.sub (local.get $scratch) (local.get $p)))
    (local.set $i (i32.const 0))
    (block $copy_done
      (loop $copy
        (br_if $copy_done (i32.ge_u (local.get $i) (local.get $len)))
        (i32.store8
          (i32.add (local.get $out) (local.get $i))
          (i32.load8_u (i32.add (local.get $p) (local.get $i))))
        (local.set $i (i32.add (local.get $i) (i32.const 1)))
        (br $copy)))
    (local.get $len))

  (func $write_u32_pad3 (param $v i32) (param $out i32)
    (i32.store8
      (local.get $out)
      (i32.add (i32.div_u (local.get $v) (i32.const 100)) (i32.const 48)))
    (i32.store8
      (i32.add (local.get $out) (i32.const 1))
      (i32.add
        (i32.rem_u (i32.div_u (local.get $v) (i32.const 10)) (i32.const 10))
        (i32.const 48)))
    (i32.store8
      (i32.add (local.get $out) (i32.const 2))
      (i32.add (i32.rem_u (local.get $v) (i32.const 10)) (i32.const 48))))

  (func $xorshift32 (param $x i32) (result i32)
    (local.set $x (i32.xor (local.get $x) (i32.shl (local.get $x) (i32.const 13))))
    (local.set $x (i32.xor (local.get $x) (i32.shr_u (local.get $x) (i32.const 17))))
    (local.set $x (i32.xor (local.get $x) (i32.shl (local.get $x) (i32.const 5))))
    (local.get $x))

  (func (export "_start")
    (local $i i32)
    (local $x i32)
    (local $idx i32)
    (local $t0 i64)
    (local $t1 i64)
    (local $diff i64)
    (local $ms_int i64)
    (local $ms_frac i32)
    (local $p i32)
    (local $nlen i32)

    (local.set $x (i32.const 1))

    ;; start timing
    (call $clock_time_get (i32.const 1) (i64.const 0) (i32.const 16))
    drop
    (local.set $t0 (i64.load (i32.const 16)))

    (local.set $i (i32.const 0))
    (block $done
      (loop $loop
        (br_if $done (i32.ge_u (local.get $i) (i32.const 2000000)))

        (local.set $x (call $xorshift32 (local.get $x)))
        (local.set $idx (i32.and (local.get $x) (i32.const 31)))

        (block $after
          (block $default
            (block $c15
              (block $c14
                (block $c13
                  (block $c12
                    (block $c11
                      (block $c10
                        (block $c9
                          (block $c8
                            (block $c7
                              (block $c6
                                (block $c5
                                  (block $c4
                                    (block $c3
                                      (block $c2
                                        (block $c1
                                          (block $c0
                                            (local.get $idx)
                                            (br_table
                                              $c0 $c1 $c2 $c3 $c4 $c5 $c6 $c7
                                              $c8 $c9 $c10 $c11 $c12 $c13 $c14 $c15
                                              $default)
                                          )
                                          ;; case 0
                                          (local.set $x (i32.add (local.get $x) (i32.const 1)))
                                          (br $after)
                                        )
                                        ;; case 1
                                        (local.set $x (i32.sub (local.get $x) (i32.const 1)))
                                        (br $after)
                                      )
                                      ;; case 2
                                      (local.set $x (i32.xor (local.get $x) (i32.const 0x9e3779b9)))
                                      (br $after)
                                    )
                                    ;; case 3
                                    (local.set $x (i32.add (i32.rotl (local.get $x) (i32.const 1)) (i32.const 3)))
                                    (br $after)
                                  )
                                  ;; case 4
                                  (local.set $x (i32.add (i32.rotr (local.get $x) (i32.const 3)) (i32.const 5)))
                                  (br $after)
                                )
                                ;; case 5
                                (local.set $x (i32.mul (local.get $x) (i32.const 3)))
                                (br $after)
                              )
                              ;; case 6
                              (local.set $x (i32.add (local.get $x) (i32.const 7)))
                              (br $after)
                            )
                            ;; case 7
                            (local.set $x (i32.xor (local.get $x) (i32.shr_u (local.get $x) (i32.const 1))))
                            (br $after)
                          )
                          ;; case 8
                          (local.set $x (i32.add (local.get $x) (i32.const 11)))
                          (br $after)
                        )
                        ;; case 9
                        (local.set $x (i32.sub (local.get $x) (i32.const 13)))
                        (br $after)
                      )
                      ;; case 10
                      (local.set $x (i32.mul (local.get $x) (i32.const 5)))
                      (br $after)
                    )
                    ;; case 11
                    (local.set $x (i32.xor (local.get $x) (i32.const 0x7f4a7c15)))
                    (br $after)
                  )
                  ;; case 12
                  (local.set $x (i32.add (local.get $x) (i32.const 17)))
                  (br $after)
                )
                ;; case 13
                (local.set $x (i32.sub (local.get $x) (i32.const 19)))
                (br $after)
              )
              ;; case 14
              (local.set $x (i32.add (i32.rotl (local.get $x) (i32.const 5)) (i32.const 23)))
              (br $after)
            )
            ;; case 15
            (local.set $x (i32.xor (local.get $x) (i32.rotr (local.get $x) (i32.const 7))))
            (br $after)
          )
          ;; default
          (local.set $x (i32.xor (local.get $x) (i32.const 0xdeadbeef)))
        )

        (local.set $i (i32.add (local.get $i) (i32.const 1)))
        (br $loop)))

    (i32.store (i32.const 12) (local.get $x))

    ;; end timing
    (call $clock_time_get (i32.const 1) (i64.const 0) (i32.const 24))
    drop
    (local.set $t1 (i64.load (i32.const 24)))

    (local.set $diff (i64.sub (local.get $t1) (local.get $t0)))
    (local.set $ms_int (i64.div_u (local.get $diff) (i64.const 1000000)))
    (local.set $ms_frac
      (i32.wrap_i64
        (i64.div_u
          (i64.rem_u (local.get $diff) (i64.const 1000000))
          (i64.const 1000))))

    (local.set $p (i32.const 256))
    (i32.store8 (i32.add (local.get $p) (i32.const 0)) (i32.const 84))
    (i32.store8 (i32.add (local.get $p) (i32.const 1)) (i32.const 105))
    (i32.store8 (i32.add (local.get $p) (i32.const 2)) (i32.const 109))
    (i32.store8 (i32.add (local.get $p) (i32.const 3)) (i32.const 101))
    (i32.store8 (i32.add (local.get $p) (i32.const 4)) (i32.const 58))
    (i32.store8 (i32.add (local.get $p) (i32.const 5)) (i32.const 32))

    (local.set $nlen (call $write_u64_dec (local.get $ms_int) (i32.add (local.get $p) (i32.const 6))))
    (i32.store8 (i32.add (local.get $p) (i32.add (i32.const 6) (local.get $nlen))) (i32.const 46))
    (call $write_u32_pad3
      (local.get $ms_frac)
      (i32.add (local.get $p) (i32.add (i32.const 7) (local.get $nlen))))
    (i32.store8 (i32.add (local.get $p) (i32.add (i32.const 10) (local.get $nlen))) (i32.const 32))
    (i32.store8 (i32.add (local.get $p) (i32.add (i32.const 11) (local.get $nlen))) (i32.const 109))
    (i32.store8 (i32.add (local.get $p) (i32.add (i32.const 12) (local.get $nlen))) (i32.const 115))
    (i32.store8 (i32.add (local.get $p) (i32.add (i32.const 13) (local.get $nlen))) (i32.const 10))

    (call $write (local.get $p) (i32.add (i32.const 14) (local.get $nlen)))
    (call $proc_exit (i32.const 0)))
)

#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class BuildUnit:
    kind: str  # "cc" | "wat"
    src: Path
    out: Path


def _run(cmd: list[str], *, cwd: Path, verbose: bool) -> None:
    if verbose:
        print("+ " + " ".join(cmd), flush=True)
    subprocess.run(cmd, cwd=str(cwd), check=True)


def _default_sysroot() -> Path | None:
    env = os.environ.get("WASI_SYSROOT")
    if env and Path(env).is_dir():
        return Path(env)
    p = Path("/Users/liyinan/Documents/MacroModel/src/wasi-libc/build-mvp/sysroot")
    if p.is_dir():
        return p
    return None


def build_cc(
    *,
    clangxx: str,
    sysroot: Path,
    unit: BuildUnit,
    verbose: bool,
) -> None:
    unit.out.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        clangxx,
        "-o",
        str(unit.out),
        str(unit.src),
        "-Ofast",
        "-Wno-deprecated-ofast",
        "-s",
        "-flto",
        "-fuse-ld=lld",
        "-fno-rtti",
        "-fno-unwind-tables",
        "-fno-asynchronous-unwind-tables",
        "-fno-exceptions",
        "--target=wasm32-wasip1",
        f"--sysroot={sysroot}",
        "-std=c++26",
        "-mno-bulk-memory",
        "-mno-bulk-memory-opt",
        "-mno-nontrapping-fptoint",
        "-mno-sign-ext",
        "-mno-mutable-globals",
        "-mno-multivalue",
        "-mno-reference-types",
        "-mno-call-indirect-overlong",
    ]
    _run(cmd, cwd=unit.src.parent, verbose=verbose)


def build_wat(*, unit: BuildUnit, verbose: bool) -> None:
    unit.out.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        "wat2wasm",
        str(unit.src),
        "-o",
        str(unit.out),
        "--disable-bulk-memory",
        "--disable-sign-extension",
        "--disable-saturating-float-to-int",
        "--disable-multi-value",
        "--disable-reference-types",
    ]
    _run(cmd, cwd=unit.src.parent, verbose=verbose)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description="Build the u2bench wasm corpus (C++ + WAT).")
    ap.add_argument("--sysroot", default="", help="WASI sysroot (default: $WASI_SYSROOT or wasi-libc build-mvp sysroot)")
    ap.add_argument("--clangxx", default="clang++", help="clang++ path (default: clang++)")
    ap.add_argument("--out", default="wasm/corpus", help="output directory (default: wasm/corpus)")
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args(argv)

    repo_root = Path(__file__).resolve().parents[1]
    out_root = (repo_root / args.out).resolve()

    sysroot: Path | None
    if args.sysroot:
        sysroot = Path(args.sysroot)
    else:
        sysroot = _default_sysroot()
    if not sysroot or not sysroot.is_dir():
        raise SystemExit(
            "WASI sysroot not found. Pass --sysroot or set $WASI_SYSROOT. "
            "Expected a wasi-libc sysroot with include/ and lib/."
        )

    units: list[BuildUnit] = [
        BuildUnit(kind="cc", src=repo_root / "wasm/src/cc/wasi_clock_gettime.cc", out=out_root / "wasi/clock_gettime.wasm"),
        BuildUnit(kind="cc", src=repo_root / "wasm/src/cc/wasi_file_rw.cc", out=out_root / "wasi/file_rw_8m.wasm"),
        BuildUnit(kind="cc", src=repo_root / "wasm/src/cc/crypto_sha256.cc", out=out_root / "crypto/sha256.wasm"),
        BuildUnit(kind="cc", src=repo_root / "wasm/src/cc/crypto_chacha20.cc", out=out_root / "crypto/chacha20.wasm"),
        BuildUnit(kind="cc", src=repo_root / "wasm/src/cc/db_kv_hash.cc", out=out_root / "db/kv_hash.wasm"),
        BuildUnit(kind="cc", src=repo_root / "wasm/src/cc/vm_tinybytecode.cc", out=out_root / "vm/mini_lua_like_vm.wasm"),
        BuildUnit(kind="cc", src=repo_root / "wasm/src/cc/science_matmul_i32.cc", out=out_root / "science/matmul_i32.wasm"),
        BuildUnit(kind="cc", src=repo_root / "wasm/src/cc/science_matmul_f64.cc", out=out_root / "science/matmul_f64.wasm"),
        BuildUnit(kind="cc", src=repo_root / "wasm/src/cc/science_sieve_i32.cc", out=out_root / "science/sieve_i32_2m.wasm"),
        BuildUnit(kind="wat", src=repo_root / "wasm/src/wat/loop_i32.wat", out=out_root / "micro/loop_i32.wasm"),
    ]

    built = 0
    for u in units:
        if not u.src.exists():
            raise SystemExit(f"missing source: {u.src}")
        if u.kind == "cc":
            build_cc(clangxx=args.clangxx, sysroot=sysroot, unit=u, verbose=args.verbose)
        elif u.kind == "wat":
            build_wat(unit=u, verbose=args.verbose)
        else:
            raise SystemExit(f"unknown kind: {u.kind}")
        built += 1

    print(f"built {built} wasm files under: {out_root}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

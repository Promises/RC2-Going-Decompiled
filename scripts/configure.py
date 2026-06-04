#!/usr/bin/env python3
"""
configure.py - single entry point that turns the target boot ELFs into a
disassembled, (eventually) buildable tree.

Phase 1 (works today): run splat to (re)split a region into asm + linker
script + symbol maps under going-decompiled/.

Phase 2 (gated on the EE toolchain): once a matching ee-gcc / ee-as / ee-ld
is registered in tools/ee/, emit a build.ninja that assembles the asm,
compiles any hand-written C in going-decompiled/src/, links an ELF, and
verifies it against the original .rom (the "matching" loop).

Usage:
    python scripts/configure.py            # split both regions (usa, eu)
    python scripts/configure.py --region usa
    python scripts/configure.py --region eu --no-cache

Run inside the decomp venv:  . .venv-decomp/bin/activate
"""
from __future__ import annotations
import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CONFIGS = {
    "usa": ROOT / "going-decompiled/config/usa/SCUS_972.68.yaml",
    "eu": ROOT / "going-decompiled/config/eu/SCES_516.07.yaml",
}
ROMS = {
    "usa": ROOT / "extracted/usa/SCUS_972.68.rom",
    "eu": ROOT / "extracted/eu/SCES_516.07.rom",
}
EE_TOOLCHAIN_DIR = ROOT / "tools/ee"  # ee-gcc/ee-as/ee-ld get registered here


def ensure_inputs(region: str) -> None:
    rom = ROMS[region]
    if not rom.exists():
        sys.exit(
            f"[configure] missing {rom}\n"
            f"            run scripts/extract_isos.sh first (needs the source ISOs)."
        )
    # splat reads these even when empty; create them so a fresh checkout works.
    for name in ("symbol_addrs.txt", "reloc_addrs.txt"):
        p = ROOT / f"going-decompiled/symbol_addrs/{region}/{name}"
        p.parent.mkdir(parents=True, exist_ok=True)
        p.touch(exist_ok=True)


def split_region(region: str, use_cache: bool) -> None:
    cfg = CONFIGS[region]
    ensure_inputs(region)
    print(f"[configure] splitting {region}: {cfg.relative_to(ROOT)}")
    cmd = [sys.executable, "-m", "splat", "split", str(cfg)]
    if use_cache:
        cmd.append("--use-cache")  # only re-split segments whose config changed
    res = subprocess.run(cmd, cwd=ROOT)
    if res.returncode != 0:
        sys.exit(f"[configure] splat failed for {region} (exit {res.returncode})")


def ee_toolchain_present() -> bool:
    return (EE_TOOLCHAIN_DIR / "bin").is_dir()


def main() -> None:
    ap = argparse.ArgumentParser(description="Going Commando decomp configure")
    ap.add_argument("--region", choices=["usa", "eu", "both"], default="both")
    ap.add_argument("--no-cache", action="store_true", help="force full re-disassembly")
    args = ap.parse_args()

    regions = ["usa", "eu"] if args.region == "both" else [args.region]
    for r in regions:
        split_region(r, use_cache=not args.no_cache)

    if ee_toolchain_present():
        print("[configure] EE toolchain found - ninja emission not yet implemented (Phase 2).")
    else:
        print(
            "[configure] NOTE: no EE toolchain in tools/ee/ yet.\n"
            "            asm/linker-script/symbols are regenerated, but a matching\n"
            "            ELF build is gated on registering ee-gcc/ee-as/ee-ld.\n"
            "            See docs/TOOLCHAIN.md."
        )
    print("[configure] done.")


if __name__ == "__main__":
    main()

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
    python scripts/configure.py            # split both regions (usa, eu), full split
    python scripts/configure.py --region usa
    python scripts/configure.py --region eu --use-cache   # opt-in incremental split

CACHE POLICY (task #449, FACT #7247/#7248). The default is a FULL split: splat's
--use-cache is passed only when asked for with --use-cache. splat's .splache
keys each segment on its (yaml line, rom_end) only — a symbol_addrs, subalign,
vram or bss_size change never invalidates it, and a PARTIAL hit skips the scan
of the hit segments, so the symbols they would have registered are missing
when a missed segment is disassembled: `.word Name+off` degrades to a raw word
that assembles byte-identically, and no byte gate can see it. When --use-cache
IS given, the cache is discarded first if the inputs splat does not key on have
changed since it was written (their fingerprint is stored inside the cache
file under "__configure_inputs__"). --no-cache is kept as an explicit spelling
of the default.

Run inside the decomp venv:  . .venv-decomp/bin/activate
"""
from __future__ import annotations
import argparse
import hashlib
import pickle
import subprocess
import sys
from pathlib import Path

import yaml

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


INPUTS_KEY = "__configure_inputs__"
# Top-level segment keys splat's per-segment cache key does NOT cover (it keys a
# group on its subsegment (yaml, rom_end) list only — splat/segtypes/common/
# group.py cache()). A change here re-lays the segment without a cache miss.
SEGMENT_KEYS = ("vram", "bss_size", "subalign")


def _sha256(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest() if p.exists() else "absent"


def cache_inputs(region: str) -> dict:
    """Fingerprint of the split inputs the .splache is blind to."""
    cfg = CONFIGS[region]
    doc = yaml.safe_load(cfg.read_text())
    opts = doc.get("options", {})
    segs = []
    for seg in doc.get("segments", []):
        if isinstance(seg, dict):
            segs.append({k: seg.get(k) for k in ("name", "start") + SEGMENT_KEYS})
    inputs = {"segments": segs}
    for key in ("symbol_addrs_path", "reloc_addrs_path"):
        for rel in opts.get(key, []) or []:
            inputs[rel] = _sha256(ROOT / rel)
    return inputs


def cache_path(region: str) -> Path:
    doc = yaml.safe_load(CONFIGS[region].read_text())
    return ROOT / doc["options"]["cache_path"]


def _load_cache(p: Path):
    try:
        with p.open("rb") as fh:
            return pickle.load(fh)
    except Exception:
        return None


def invalidate_cache_if_inputs_changed(region: str, inputs: dict) -> None:
    p = cache_path(region)
    if not p.exists():
        print(f"[configure] {region}: no cache at {p.relative_to(ROOT)} (full split)")
        return
    cache = _load_cache(p)
    stored = cache.get(INPUTS_KEY) if isinstance(cache, dict) else None
    if stored == inputs:
        print(f"[configure] {region}: cache inputs unchanged, cache kept")
        return
    if stored is None:
        why = "cache carries no input fingerprint"
    else:
        changed = sorted(k for k in set(stored) | set(inputs) if stored.get(k) != inputs.get(k))
        why = "changed: " + ", ".join(changed)
    p.unlink()
    print(f"[configure] {region}: cache DISCARDED ({why}) -> full split")


def record_cache_inputs(region: str, inputs: dict) -> None:
    p = cache_path(region)
    cache = _load_cache(p) if p.exists() else None
    if not isinstance(cache, dict):
        return
    cache[INPUTS_KEY] = inputs
    with p.open("wb") as fh:
        pickle.dump(cache, fh)


def split_region(region: str, use_cache: bool) -> None:
    cfg = CONFIGS[region]
    ensure_inputs(region)
    print(f"[configure] splitting {region}: {cfg.relative_to(ROOT)}"
          f" ({'--use-cache' if use_cache else 'full split, no cache'})")
    # Pass the config path RELATIVE to ROOT so splat's base_path stays relative,
    # keeping generated INCLUDE_ASM paths portable (not machine-absolute).
    # run_splat.py is splat's own CLI plus the INCLUDE_ASM_FRAGMENT spelling, so a
    # fresh split emits every leaf the C files include.
    cmd = [sys.executable, str(ROOT / "tools" / "splat_ext" / "run_splat.py"),
           "split", str(cfg.relative_to(ROOT))]
    inputs = None
    if use_cache:
        inputs = cache_inputs(region)
        invalidate_cache_if_inputs_changed(region, inputs)
        cmd.append("--use-cache")  # only re-split segments whose config changed
    res = subprocess.run(cmd, cwd=ROOT)
    if res.returncode != 0:
        sys.exit(f"[configure] splat failed for {region} (exit {res.returncode})")
    if use_cache:
        record_cache_inputs(region, inputs)


def ee_toolchain_present() -> bool:
    return (EE_TOOLCHAIN_DIR / "bin").is_dir()


def main() -> None:
    ap = argparse.ArgumentParser(description="Going Commando decomp configure")
    ap.add_argument("--region", choices=["usa", "eu", "both"], default="both")
    ap.add_argument("--use-cache", action="store_true",
                    help="incremental split via splat's .splache (opt-in; see CACHE POLICY)")
    ap.add_argument("--no-cache", action="store_true",
                    help="full re-disassembly (the default; kept as an explicit spelling)")
    args = ap.parse_args()
    if args.use_cache and args.no_cache:
        ap.error("--use-cache and --no-cache are mutually exclusive")

    regions = ["usa", "eu"] if args.region == "both" else [args.region]
    for r in regions:
        split_region(r, use_cache=args.use_cache)

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

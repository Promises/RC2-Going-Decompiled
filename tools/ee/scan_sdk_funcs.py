#!/usr/bin/env python3
"""
Find SCE/newlib library functions that are linked VERBATIM into the Going
Commando binaries, using relocation-masked full-function matching.

For each SDK .o (extracted from libc.a/libm.a), we take its .text bytes and the
relocation offsets, zero the relocated instruction fields (HI16/LO16 -> low 16
bits; R_MIPS_26 -> low 26 bits) in BOTH the library bytes and any candidate
window in the binary, then verify the whole function matches under that mask.
Candidates are anchored on the longest leading relocation-free run.

These functions are "free": they need no decompilation — they match by linking
the original .a (or by dropping in the known newlib source). Run after extracting
objects (see find_libc_funcs.py header). Host venv: . .venv-decomp/bin/activate
"""
from __future__ import annotations
import glob, struct
from pathlib import Path
from elftools.elf.elffile import ELFFile

ROOT = Path(__file__).resolve().parent.parent.parent
OBJ_DIRS = [ROOT / "tools/ee/sdk-lib/obj", ROOT / "tools/ee/sdk-lib/obj-m"]
SEG1_DELTA = 0xFF080
TARGETS = {"usa": ROOT / "extracted/usa/SCUS_972.68",
           "eu": ROOT / "extracted/eu/SCES_516.07"}


def func_sig(path: Path):
    """Return (masked_bytes, mask, anchor_prefix) for the object's .text, or None."""
    elf = ELFFile(path.open("rb"))
    text = elf.get_section_by_name(".text")
    if not text:
        return None
    data = bytearray(text.data())
    n = len(data)
    if n < 16:
        return None
    mask = bytearray(b"\xff" * n)  # 0xff = compared, 0x00 = masked out
    rs = elf.get_section_by_name(".rel.text") or elf.get_section_by_name(".rela.text")
    reloc_offs = []
    if rs:
        symtab = elf.get_section(rs["sh_link"])
        for r in rs.iter_relocations():
            off = r["r_offset"]
            t = r["r_info_type"]
            reloc_offs.append(off)
            if off + 4 > n:
                continue
            if t in (5, 6):      # R_MIPS_HI16 / R_MIPS_LO16: low 16 bits vary
                mask[off] = 0; mask[off + 1] = 0
            elif t == 4:         # R_MIPS_26: low 26 bits vary -> mask low 4 bytes' 26 bits
                mask[off] = 0; mask[off + 1] = 0; mask[off + 2] = 0
                mask[off + 3] &= 0xFC
            else:                # be conservative: mask the whole word
                for k in range(4):
                    if off + k < n: mask[off + k] = 0
    # anchor = longest leading run with no masked byte
    a = 0
    while a < n and mask[a] == 0xff:
        a += 1
    if a < 12:
        # fall back: use first 12 bytes if the very start has a reloc (rare)
        a = min(12, n)
    return bytes(data), bytes(mask), bytes(data[:a])


def find_masked(hay: bytes, sig, mask, anchor) -> int:
    start = 0
    L = len(sig)
    while True:
        i = hay.find(anchor, start)
        if i < 0 or i + L > len(hay):
            return -1
        ok = True
        for k in range(L):
            if mask[k] and hay[i + k] != sig[k]:
                ok = False
                break
        if ok:
            return i
        start = i + 1


def main():
    bins = {k: v.read_bytes() for k, v in TARGETS.items()}
    seen = {}
    for d in OBJ_DIRS:
        for o in sorted(glob.glob(str(d / "*.o"))):
            sig = func_sig(Path(o))
            if not sig:
                continue
            data, mask, anchor = sig
            iu = find_masked(bins["usa"], data, mask, anchor)
            if iu < 0:
                continue
            ie = find_masked(bins["eu"], data, mask, anchor)
            name = Path(o).stem
            if name in seen:
                continue
            seen[name] = (iu + SEG1_DELTA, len(data), ie >= 0)
    rows = sorted(seen.items(), key=lambda kv: kv[1][0])
    print(f"# {len(rows)} SDK library functions linked verbatim in USA (EU too where noted)")
    for name, (vram, size, in_eu) in rows:
        print(f"  0x{vram:08x}  {name:18} {size:5d}B  {'EU✓' if in_eu else 'EU—'}")


if __name__ == "__main__":
    main()

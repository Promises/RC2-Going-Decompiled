#!/usr/bin/env python3
# detect_reloc_shift.py - empirically map the in-level data-block RELOCATION.
#
# The in-level engine relocates a contiguous data block by a small +offset (the
# tester byte-mapped 2 sub-blocks at +0x350 / +0x340). This independently
# recovers the shift profile by matching the in-level dump against the static
# boot ELF: for each block, inlevel[A] == ELF[A - shift] for the relocation
# shift. Rodata/string content (which is relocated verbatim) gives a clean match;
# mutable/zero globals don't match at any shift (reported as 'mutable/zero').
#
# Usage: detect_reloc_shift.py [inlevel_dir]

import os, sys, struct, json

ROOT = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(ROOT, "../../../.."))
ELF = os.path.join(REPO, "extracted/usa/SCUS_972.68")
STATE = os.path.join(REPO, "tools/ee/eetest/state")
DIR = os.path.join(STATE, sys.argv[1] if len(sys.argv) > 1 else "inlevel")

BLK = 0x100               # block granularity
SHIFTS = range(-0x80, 0x401, 0x10)  # candidate shifts to test


def elf_loads(elf):
    f = open(elf, "rb").read()
    le = "<" if f[5] == 1 else ">"
    phoff = struct.unpack(le + "I", f[28:32])[0]
    n = struct.unpack(le + "H", f[44:46])[0]
    sz = struct.unpack(le + "H", f[42:44])[0]
    segs = []
    for i in range(n):
        o = phoff + i * sz
        t, off, va, pa, fz, mz, fl, al = struct.unpack(le + "8I", f[o:o + 32])
        if t == 1:
            segs.append((va, off, fz))
    return f, segs


def elf_at(f, segs, va, n):
    for vb, off, fz in segs:
        if vb <= va < vb + fz:
            return f[off + (va - vb):off + (va - vb) + n]
    return None


def main():
    f, segs = elf_loads(ELF)
    snap = json.load(open(os.path.join(DIR, "snapshot.json")))
    greg = next(r for r in snap["regions"] if r["name"] == "globals")
    base, gb = greg["addr"], open(os.path.join(DIR, "globals.bin"), "rb").read()

    def match(a, shift):
        cur = gb[a - base:a - base + BLK]
        ref = elf_at(f, segs, a - shift, BLK)
        if not ref or len(cur) < BLK:
            return -1
        return sum(1 for x, y in zip(cur, ref) if x == y)

    rows = []
    for a in range(base, base + len(gb) - BLK, BLK):
        # skip all-zero blocks (mutable/uninit - no shift signal)
        if not any(gb[a - base:a - base + BLK]):
            rows.append((a, None, 0)); continue
        best_s, best_m = 0, match(a, 0)
        for s in SHIFTS:
            m = match(a, s)
            if m > best_m:
                best_m, best_s = m, s
        rows.append((a, best_s, best_m))

    # collapse into runs of equal shift (only count confident matches >=200/256)
    print(f"# reloc shift profile for {os.path.basename(DIR)} (state={snap.get('state')})")
    print(f"# globals window 0x{base:08X}..0x{base+len(gb):08X}; match>=200/256 = confident")
    runs = []
    for a, s, m in rows:
        tag = s if (s is not None and m >= 200) else ("zero" if s is None else "?")
        if runs and runs[-1][2] == tag:
            runs[-1][1] = a + BLK
        else:
            runs.append([a, a + BLK, tag])
    for lo, hi, tag in runs:
        if tag == "zero":
            label = "mutable/zero (no rodata signal)"
        elif tag == "?":
            label = "no confident shift (mixed/mutable)"
        else:
            label = f"SHIFT +0x{tag:X}" if tag else "unshifted (+0x0)"
        print(f"  0x{lo:08X}..0x{hi:08X}  {label}")


if __name__ == "__main__":
    main()

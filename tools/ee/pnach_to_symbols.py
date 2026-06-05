#!/usr/bin/env python3
"""
Parse PCSX2 .pnach cheat files into a sorted map of named game globals for the
USA v2.00 binary (CRC B3A71D10). Cheat patch addresses encode the write-width
in the leading nibble (2=32b, 1=16b, 0=8b, 4=multiwrite); the real EE address is
field & 0x0FFFFFFF. We keep only addresses inside the binary's static data range.

These are NAMING HYPOTHESES for the anonymous D_/DAT_ globals. The clustering
reveals structs/arrays (see reference/cheat_globals.md). Confirm each by tracing
its writer in Ghidra before treating a name as authoritative.

Usage:  python tools/ee/pnach_to_symbols.py [pnach...]   (default: reference/cheats/*.pnach)
"""
from __future__ import annotations
import re, sys, glob
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
WIDTH = {"0": 1, "1": 2, "2": 4, "4": 16}
LO, HI = 0x100080, 0x2C0500  # static data range of the loaded image


def parse(files):
    ent = {}
    for f in files:
        name = None
        for ln in Path(f).read_text(encoding="latin1").splitlines():
            m = re.match(r"\[(.+)\]", ln)
            if m:
                name = m.group(1).replace("\\", " / ").strip()
            m = re.match(r"patch=1,EE,([0-9A-Fa-f]{8}),extended,[0-9A-Fa-f]{8}", ln)
            if m and name:
                t = m.group(1)[0]
                if t.upper() == "F" or m.group(1) == "01010101":
                    continue  # CB enabler / multiwrite continuation
                addr = int(m.group(1), 16) & 0x0FFFFFFF
                if LO <= addr <= HI:
                    ent.setdefault(addr, (WIDTH.get(t, 4), name))
    return ent


def main():
    files = sys.argv[1:] or sorted(glob.glob(str(ROOT / "going-decompiled/reference/cheats/*.pnach")))
    ent = parse(files)
    for a in sorted(ent):
        w, n = ent[a]
        print(f"0x{a:08X}\t{w}B\t{n}")
    print(f"# {len(ent)} globals in static range from {len(files)} pnach file(s)", file=sys.stderr)


if __name__ == "__main__":
    main()

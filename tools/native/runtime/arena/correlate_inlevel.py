#!/usr/bin/env python3
# correlate_inlevel.py - in-level global layout delta.
#
# Correlates the native globals-arena placement map (arena_map.txt, the 441
# globals seeded from NEW-GAME state) against the tester's ground-truth in-level
# memory dumps (tools/ee/eetest/state/inlevel/*.bin), to classify each arena
# global as LIVE (the address still holds its real global in-level) vs OVERLAID
# (in-level the address is clobbered by level/string/image/profiler data, so the
# arena seed there is INVALID and the tester's effect-diff must exclude it).
#
# Discriminators:
#  1) ASCII-overlay: a run of printable bytes at the address in-level => OVERLAID.
#  2) menu-vs-inlevel diff over the overlapping 0x1A6400..0x1B2400 window:
#     menu holds a sane global, in-level holds string/binary junk => OVERLAID.
#  3) zero in-level => INERT (not seeded with anything meaningful).
#
# Usage: correlate_inlevel.py            (prints the delta report)

import os, re, json, string

import sys
ROOT = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(ROOT, "../../../.."))
ARENA_MAP = os.path.join(ROOT, "arena_map.txt")
STATE = os.path.join(REPO, "tools/ee/eetest/state")
# which in-level capture dir to correlate (default the canonical inlevel/)
_dir = "inlevel"
for _a in sys.argv[1:]:
    if _a.startswith("--dir="):
        _dir = _a.split("=", 1)[1]
INLEVEL = os.path.join(STATE, _dir)

PRINTABLE = set(bytes(string.printable[:-5], "ascii"))  # drop \t\n\r\v\f


def load_regions(snapshot_path):
    j = json.load(open(snapshot_path))
    regs = []
    for r in j["regions"]:
        regs.append((r["name"], r["addr"], r["len"]))
    return regs, j.get("state")


def load_bin(d, name):
    p = os.path.join(d, f"{name}.bin")
    return open(p, "rb").read() if os.path.exists(p) else None


def read_at(regs, bins, addr, n=16):
    """Read n bytes at EE addr from whichever captured region contains it."""
    for name, base, length in regs:
        if base <= addr < base + length:
            b = bins.get(name)
            if b is None:
                return None, name
            off = addr - base
            return b[off:off + n], name
    return None, None


def ascii_run(bb):
    """length of leading printable-ASCII run in bb."""
    n = 0
    for c in bb:
        if c in PRINTABLE and c != 0:
            n += 1
        else:
            break
    return n


def classify(val16, menu16):
    if val16 is None:
        return "uncaptured"
    if all(b == 0 for b in val16):
        return "zero"
    run = ascii_run(val16)
    if run >= 4:
        return "overlaid:ascii"
    # pointer-ish first word in EE data/bss/text range?
    w = int.from_bytes(val16[:4], "little")
    if 0x00100000 <= w < 0x02000000 or 0x70000000 <= w < 0x70004000:
        # if menu had a very different non-pointer here it may still be a real ptr
        return "live:ptr"
    return "scalar"


def main():
    in_regs, in_state = load_regions(os.path.join(INLEVEL, "snapshot.json"))
    menu_regs, menu_state = load_regions(os.path.join(STATE, "snapshot.json"))
    in_bins = {n: load_bin(INLEVEL, n) for n, _, _ in in_regs}
    menu_bins = {n: load_bin(STATE, n) for n, _, _ in menu_regs}

    print(f"# in-level layout delta  (in-level state={in_state}  menu state={menu_state})")
    print("# captured in-level regions:")
    for n, a, l in in_regs:
        print(f"#   {n:12s} 0x{a:08X}..0x{a+l:08X}  ({l} bytes)")
    print()

    # --- 1. map contiguous ASCII-overlay spans in the in-level globals dump ---
    gb = in_bins["globals"]
    gbase = next(a for n, a, l in in_regs if n == "globals")
    spans = []
    i = 0
    while i < len(gb):
        if gb[i] in PRINTABLE and gb[i] != 0:
            j = i
            while j < len(gb) and (gb[j] in PRINTABLE and gb[j] != 0 or
                                   (gb[j] == 0 and j + 1 < len(gb) and gb[j + 1] in PRINTABLE)):
                j += 1
            if j - i >= 8:
                spans.append((gbase + i, gbase + j))
            i = j
        else:
            i += 1
    # merge spans separated by < 0x40 bytes (same overlay structure)
    merged = []
    for s in spans:
        if merged and s[0] - merged[-1][1] < 0x40:
            merged[-1] = (merged[-1][0], s[1])
        else:
            merged.append(list(s))
    print(f"## ASCII/string overlay spans in globals region ({len(merged)} blocks):")
    for a, b in merged:
        sample = gb[a - gbase:a - gbase + 24]
        txt = "".join(chr(c) if c in PRINTABLE and c != 0 else "." for c in sample)
        print(f"   0x{a:08X}..0x{b:08X}  ({b-a:5d}B)  '{txt}'")
    print()

    # --- 2. classify each arena global in the captured ranges ---
    rows = []
    for ln in open(ARENA_MAP):
        if ln.startswith("#") or not ln.strip():
            continue
        parts = ln.split()
        if len(parts) < 2:
            continue
        sym, addr = parts[0], int(parts[1], 16)
        v, reg = read_at(in_regs, in_bins, addr)
        if reg is None:
            continue  # not captured
        mv, _ = read_at(menu_regs, menu_bins, addr)
        cls = classify(v, mv)
        rows.append((addr, sym, reg, cls, v, mv))

    # tag globals that fall inside an ASCII overlay span
    def in_span(a):
        return any(s <= a < e for s, e in merged)

    counts = {}
    overlaid_named = []
    live_named = []
    for addr, sym, reg, cls, v, mv in rows:
        if in_span(addr):
            cls = "overlaid:span"
        counts[cls] = counts.get(cls, 0) + 1
        named = not sym.startswith("D_")
        if cls.startswith("overlaid") and named:
            overlaid_named.append((addr, sym, cls, v))
        if cls.startswith("live") and named:
            live_named.append((addr, sym, cls, v))

    print(f"## classification of {len(rows)} arena globals in captured ranges:")
    for k in sorted(counts):
        print(f"   {k:16s} {counts[k]}")
    print()

    def hexw(v):
        return "0x%08X" % int.from_bytes(v[:4], "little") if v else "----"

    print("## NAMED globals OVERLAID in-level (seed INVALID - exclude from effect-diff):")
    for addr, sym, cls, v in sorted(overlaid_named):
        print(f"   0x{addr:08X} {sym:34s} {cls:14s} inlevel={hexw(v)}")
    print()
    print("## NAMED pointer globals LIVE in-level (seed valid):")
    for addr, sym, cls, v in sorted(live_named):
        print(f"   0x{addr:08X} {sym:34s} {hexw(v)}")
    print()

    # --- 3. emit the exclude byte-ranges (the tester's consumable) ---
    # An arena address is SEED-INVALID in-level iff it lands in a string-overlay
    # span OR (it is in the overlap window and its menu value was a sane global
    # but in-level it changed into the string-dense neighbourhood).
    if "--ranges" in sys.argv:
        print("## EXCLUDE byte-ranges (in-level string-overlay spans; EE addrs):")
        for a, b in merged:
            print(f"   0x{a:08X} 0x{b:08X}")

    # --- 4. full named-global delta table (menu word vs in-level word) ---
    if "--table" in sys.argv:
        print("## FULL named-global delta (addr | sym | menu | inlevel | class | live?):")
        for addr, sym, reg, cls, v, mv in sorted(rows):
            if sym.startswith("D_"):
                continue
            if in_span(addr):
                cls = "overlaid:span"
            live = "LIVE" if cls.startswith("live") or (
                cls == "scalar" and mv is not None and v[:4] == mv[:4]) else (
                "OVERLAID" if cls.startswith("overlaid") else "CHECK")
            print(f"   0x{addr:08X} {sym:34s} menu={hexw(mv):>10s} "
                  f"inlevel={hexw(v):>10s} {cls:14s} {live}")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
overlay_calt_size_table.py — comprehensive LENGTH-DELTA table for the C-only
calt OVERLAY (the proven main-segment nm-scan, applied to the overlay).

For EVERY #else (TARGET_NATIVE) function: its COMPILED length (from the calt
object) vs its ORIGINAL length (the `nonmatching func_X, 0xSIZE` asm decl = the
matching arm's true byte size). A nonzero delta = a behaviorally-faithful #else
that compiles to a different byte-length, which shifts the calt overlay layout
(the suspected length/layout root-cause behind the wild-pointer trap).

Enumerates ALL discrepancies in ONE table -> route:
  - BOUNDED handful  -> length-fit them all as one batch (each keeps cmp-oracle).
  - MANY / spread    -> calt-overlay-layout tractability question -> STOP, escalate.

Read-only; runs on captured objdump output + the committed asm .s files.
NO build of its own.

WORKFLOW (after ONE clean build, tester FULLY STOPPED, window cleared):
  1) capture calt-object symbol tables in the VM (single command):
       tools/ee/vm.sh 'for o in $(cat going-decompiled/build/usa/conly_calt_units.txt); do \
            echo "==OBJ $o=="; mips-linux-gnu-objdump -t "$o"; done' > /tmp/calt_syms.txt
  2) emit the table (host, no VM):
       tools/ee/overlay_calt_size_table.py /tmp/calt_syms.txt going-decompiled/asm/usa/nonmatchings
"""
import sys, re, os, glob


def orig_sizes(asm_root):
    """func name -> original byte size, from `nonmatching <name>, <size>` (matching arm)."""
    sz = {}
    for s in glob.glob(os.path.join(asm_root, "**", "*.s"), recursive=True):
        try:
            head = open(s, "r", errors="ignore").read(512)
        except OSError:
            continue
        m = re.search(r"nonmatching\s+(\w+)\s*,\s*(0x[0-9A-Fa-f]+|\d+)", head)
        if m:
            sz[m.group(1)] = int(m.group(2), 0)
    return sz


def calt_sizes(dumpfile):
    """parse `mips-linux-gnu-objdump -t` output, grouped by '==OBJ <path>==' markers.
       returns name -> (compiled_size, object_path).  Only 'F .text' function symbols."""
    out = {}
    cur = None
    for ln in open(dumpfile, "r", errors="ignore"):
        m = re.match(r"==OBJ (.+)==", ln)
        if m:
            cur = m.group(1).strip()
            continue
        # objdump -t: "<addr> g/l ... F .text\t<size> <name>"
        if " F " in ln and ".text" in ln:
            p = ln.split()
            try:
                size = int(p[-2], 16)
                name = p[-1]
            except (ValueError, IndexError):
                continue
            if name.endswith(".NON_MATCHING"):
                continue
            # first definition wins (a name should be unique per calt object)
            out.setdefault(name, (size, cur))
    return out


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    dumpfile, asm_root = sys.argv[1], sys.argv[2]
    orig = orig_sizes(asm_root)
    calt = calt_sizes(dumpfile)

    rows, missing = [], []
    for name, (csize, obj) in calt.items():
        osize = orig.get(name)
        if osize is None:
            missing.append((name, obj))
        else:
            rows.append((name, obj, osize, csize, csize - osize))

    off = sorted((r for r in rows if r[4] != 0), key=lambda r: -abs(r[4]))
    shorter = [r for r in off if r[4] < 0]   # likely tail-call / sibling-call lowered
    longer = [r for r in off if r[4] > 0]    # literal/.lit expansion, save-layout, etc.
    print("== C-only calt OVERLAY length-delta table ==")
    print(f"#else functions compiled: {len(calt)} | matched to an original size: {len(rows)} | "
          f"LENGTH-OFF: {len(off)}  (shorter={len(shorter)}  longer={len(longer)})")

    def dump(title, group, note):
        print(f"\n-- {title} ({len(group)}) -- {note}")
        print(f"   {'func':32} {'unit':16} {'orig':>8} {'calt':>8} {'delta':>8}")
        for name, obj, osize, csize, d in group:
            unit = os.path.basename(obj).replace(".calt.o", "") if obj else "?"
            print(f"   {name:32} {unit:16} 0x{osize:06x} 0x{csize:06x} {d:+#8x}")

    # SHORTER first: the proven systemic class (tail-call-lowered -> add the
    # __asm__ __volatile__("") function-end guard -> restores jal+epilogue).
    dump("SHORTER than original  [TAIL-CALL-LOWERED class]", shorter,
         "fix: function-end empty-asm guard (suppresses sibling-call), keep cmp-oracle PASS")
    dump("LONGER than original", longer,
         "fix: per-body (literal/.lit/save-layout) -- inspect individually")
    tot = sum(r[4] for r in off)
    print(f"\nsum(delta): {tot:+#x}  (cumulative overlay shift = this, modulo placement)")
    # routing hint
    if len(off) == 0:
        print("ROUTE: zero length deltas -> NOT a length/layout bug; re-examine binding/link-resolution.")
    elif len(off) <= 12:
        print(f"ROUTE: BOUNDED ({len(off)}) -> batch-apply the guard to the SHORTER set + per-body the "
              f"LONGER set; each keeps cmp-oracle PASS; ONE tester re-boot.")
    else:
        print(f"ROUTE: MANY/SPREAD ({len(off)}) -> calt-overlay-layout tractability question -> STOP + escalate.")
    if missing:
        names = [n for n, _ in missing][:12]
        print(f"\n[warn] {len(missing)} compiled #else funcs had no `nonmatching` size match "
              f"(real-C-matched / renamed?): {names}")


if __name__ == "__main__":
    main()

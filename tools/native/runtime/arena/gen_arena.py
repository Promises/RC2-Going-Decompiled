#!/usr/bin/env python3
# gen_arena.py - M2 globals-arena generator (production, Mechanic B).
#
# Emits a generated linker-script fragment that places every native data global
# at its ROM-relative offset inside one contiguous .gamedata block, so the
# TARGET_NATIVE link resolves them with ZERO source edits (existing
# `extern T SYM;` decls bind to the PROVIDEd symbols) and exact ROM adjacency.
# See docs/HLE.md (M2) and the prototype A/B comparison that chose Mechanic B
# over macro-deref (no source churn, no per-symbol types - placement only).
#
# Address sourcing (no per-symbol type needed - placement is pure offset):
#   * D_xxxxxx  -> ROM address is the hex in the name (D_1A8A60 -> 0x1A8A60).
#   * named     -> symbol_addrs `SYM = 0x...;`.
#   * alias     -> symbol_addrs comment `... alias SYM ...` on a canonical line
#                  (evidence-based only; documented Ghidra aliases, not guessed).
#   * co-located-> a COMMENTED binding `// SYM = 0x...;` whose address is also a
#                  LIVE entry's address. Splat allows one active symbol per vram,
#                  so a second name for the same object is recorded commented
#                  (g_mapCache over g_mapVertexData, 0x1C4F20). Both names then
#                  PROVIDE the SAME arena offset, i.e. one object, as on the EE.
#                  A commented binding with no live entry at its address is NOT
#                  placed (it is often an UNCONFIRMED hedge); it goes unresolved.
# Only addresses inside the ELF's data window are placed (DATA_LO/DATA_HI);
# anything else (VU microcode, .text, overlay or off-image addresses) is listed
# unresolved with the reason, never stretched into the block.
# Block size = (max placed addr - base) + TAIL_PAD. Per-symbol sizing isn't
# needed: the block is contiguous, symbols are placed by offset, and adjacency
# (indexing past a named global into its neighbour) just lands in the block.
#
# Usage: gen_arena.py <names.txt> <symbol_addrs.txt> <outdir>
#   names.txt = the data-global gap list (linkgap.sh runtime-global bucket).
#   linkgap.sh inventories USA units only by default. An EU arena needs
#   `linkgap.sh --with-eu`, which adds the src/eu units (the gap list is then
#   USA+EU) while its classifier still reads USA's tables.

import sys, os, re

TAIL_PAD = 0x10000  # slack past the highest placed global for its own extent
ALIGN = 16          # ROM data segment is 16-byte aligned

# USA SCUS_972.68 section map (`readelf -S`). The data window runs from
# core.data to the start of the game .text; everything an #else body can read
# as a placed global lies in it.
DATA_LO = 0x133B80  # core.data
DATA_HI = 0x26EA00  # .text (game code) - first byte past lvl.sndvtbl
SECTIONS = [  # (start, end, name) for naming an out-of-window address
    (0x100080, 0x1151E0, ".vutext"),
    (0x115200, 0x133B80, "core.text"),
    (0x26EA00, 0x352D08, ".text"),
    (0x1800000, 0x1815570, "overlay data"),
]


def section_of(a):
    for lo, hi, name in SECTIONS:
        if lo <= a < hi:
            return name
    return "outside the ELF image"


def load_symbol_addrs(path):
    """Return (addr_by_name, addr_by_alias, commented_by_name).

    commented_by_name holds `// SYM = 0x...` bindings (inactive in splat)."""
    addr = {}
    alias = {}
    commented = {}
    line_re = re.compile(r'^([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]+)\s*;(.*)$')
    comm_re = re.compile(r'^//\s*([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]+)\b')
    alias_re = re.compile(r'\balias\s+([A-Za-z_]\w*)')
    with open(path) as f:
        for line in f:
            m = line_re.match(line.strip())
            if not m:
                c = comm_re.match(line.strip())
                if c:
                    commented.setdefault(c.group(1), int(c.group(2), 16))
                continue
            name, hexv, comment = m.group(1), m.group(2), m.group(3)
            a = int(hexv, 16)
            addr[name] = a
            for am in alias_re.finditer(comment):
                # first documented alias wins; canonical addr of this line
                alias.setdefault(am.group(1), a)
    return addr, alias, commented


def load_names(path):
    """Read one symbol name per line, stripping `#` comments and blanks."""
    out = set()
    with open(path) as f:
        for ln in f:
            ln = ln.split("#", 1)[0].strip()
            if ln:
                out.add(ln)
    return out


def resolve(name, addr_by_name, addr_by_alias, commented, live_at):
    """Return (addr, None, colocated_with) or (None, reason, None)."""
    m = re.fullmatch(r'D_([0-9A-Fa-f]{6,8})', name)
    if m:
        return int(m.group(1), 16), None, None
    if name in addr_by_name:
        return addr_by_name[name], None, None
    if name in addr_by_alias:
        return addr_by_alias[name], None, None
    if name in commented:
        a = commented[name]
        if a in live_at:
            return a, None, live_at[a]
        return None, ("symbol_addrs has only a commented binding (0x%06X) and no "
                      "live symbol at that address" % a), None
    return None, "no symbol_addrs entry and not a D_xxxxxx name", None


def main():
    if len(sys.argv) != 4:
        sys.exit("usage: gen_arena.py <names.txt> <symbol_addrs.txt> <outdir>")
    names_path, syms_path, outdir = sys.argv[1:4]
    os.makedirs(outdir, exist_ok=True)

    names = load_names(names_path)
    # const/rodata entries (string literals etc.) live in a sibling data_const.txt;
    # they are still PLACED (referenced by #else bodies, seeded from the snapshot)
    # but tagged `const` so the tester's effect-diff excludes them. See that file.
    const_path = os.path.join(os.path.dirname(names_path), "data_const.txt")
    const_names = load_names(const_path) if os.path.exists(const_path) else set()
    names = names | const_names  # place the union; const ones just carry a tag
    addr_by_name, addr_by_alias, commented = load_symbol_addrs(syms_path)
    live_at = {}
    for n, a in sorted(addr_by_name.items()):
        live_at.setdefault(a, n)

    placed = []      # (name, addr, is_const, colocated_with)
    unresolved = []  # (name, reason)
    for n in sorted(names):
        a, why, coloc = resolve(n, addr_by_name, addr_by_alias, commented, live_at)
        if a is not None and not (DATA_LO <= a < DATA_HI):
            a, why = None, ("address 0x%06X is outside the data window "
                            "[0x%06X,0x%06X) (%s)" % (a, DATA_LO, DATA_HI, section_of(a)))
        if a is None:
            unresolved.append((n, why))
        else:
            placed.append((n, a, n in const_names, coloc))
    placed.sort(key=lambda t: (t[1], t[0]))

    # Round the base down so every global keeps its ROM address modulo ALIGN
    # inside the ALIGN-aligned block (a 16-byte qword stays 16-byte aligned).
    base = placed[0][1] & ~(ALIGN - 1)
    span = (placed[-1][1] - base) + TAIL_PAD

    # ---- arena_storage.c : the contiguous backing block (the .gamedata bytes)
    with open(os.path.join(outdir, "arena_storage.c"), "w") as f:
        f.write("/* GENERATED by gen_arena.py - M2 globals-arena backing block. */\n")
        f.write("/* One contiguous block mirroring the ROM data-segment layout;  */\n")
        f.write("/* the generated arena.ld PROVIDEs every global at its ROM-       */\n")
        f.write("/* relative offset inside this block. A PINE state snapshot loads */\n")
        f.write("/* via memcpy into __gamedata_start (== &g_dataArena[0]).         */\n")
        f.write("#ifdef TARGET_NATIVE\n")
        f.write("__attribute__((aligned(%d), section(\".gamedata\")))\n" % ALIGN)
        f.write("unsigned char g_dataArena[0x%XU];\n" % span)
        f.write("#endif\n")

    # ---- arena.ld : place .gamedata after .bss, PROVIDE each symbol by offset
    with open(os.path.join(outdir, "arena.ld"), "w") as f:
        f.write("/* GENERATED by gen_arena.py - M2 globals arena (Mechanic B).\n")
        f.write(" * ROM base 0x%06X, span 0x%X (%d placed, %d unresolved).\n"
                % (base, span, len(placed), len(unresolved)))
        f.write(" * INSERT fragment: augments the default link script. */\n")
        f.write("SECTIONS {\n")
        f.write("  .gamedata (NOLOAD) : ALIGN(%d) {\n" % ALIGN)
        f.write("    __gamedata_start = .;\n")
        f.write("    *(.gamedata)\n")
        f.write("    . = __gamedata_start + 0x%X;\n" % span)
        f.write("    __gamedata_end = .;\n")
        f.write("  }\n")
        f.write("}\n")
        f.write("INSERT AFTER .bss;\n\n")
        f.write("/* ROM base = 0x%06X */\n" % base)
        for name, a, is_const, coloc in placed:
            tag = "  /* const */" if is_const else ""
            if coloc:
                tag += "  /* same object as %s */" % coloc
            f.write("PROVIDE(%s = __gamedata_start + 0x%X);%s\n" % (name, a - base, tag))

    # ---- arena_map.txt : machine-readable placement map for the PINE snapshot
    # seeder (tester's state-seeding harness). Seeding only needs base/span (the
    # whole [base, base+span) ROM range copies into &g_dataArena[0]); the
    # per-symbol rows let the harness validate a named global's value against the
    # snapshot byte at its rom_addr.
    with open(os.path.join(outdir, "arena_map.txt"), "w") as f:
        f.write("# native globals-arena placement map (Mechanic B).\n")
        f.write("# storage symbol: g_dataArena  ==  __gamedata_start (linker)\n")
        f.write("# seed: memcpy snapshot[rom_addr] -> g_dataArena[rom_addr - base]\n")
        f.write("#       for rom_addr in [base, base+span).\n")
        n_const = sum(1 for _, _, c, _ in placed if c)
        f.write("# base 0x%06X  span 0x%X  placed %d  (const %d)\n"
                % (base, span, len(placed), n_const))
        f.write("# columns: <symbol> <rom_addr> <arena_offset> [const]\n")
        f.write("#   'const' tag = static rodata/string literal (see data_const.txt);\n")
        f.write("#   NOT mutable state - the tester's effect-diff must EXCLUDE these.\n")
        for name, a, is_const, _ in placed:
            f.write("%s 0x%06X 0x%X%s\n" % (name, a, a - base, " const" if is_const else ""))

    # ---- arena_unresolved.txt : the tail needing canonical addresses (Track-B)
    with open(os.path.join(outdir, "arena_unresolved.txt"), "w") as f:
        f.write("# Native data globals referenced by #else bodies that the arena\n")
        f.write("# does NOT place: no canonical address (not a D_xxxxxx name, not\n")
        f.write("# live in symbol_addrs, not a documented or co-located alias), or\n")
        f.write("# an address outside the data window. Each line names its reason;\n")
        f.write("# a missing address needs Track-B recovery into symbol_addrs.\n")
        f.write("# Format: <symbol>  # <reason>.  Count: %d\n" % len(unresolved))
        for n, why in unresolved:
            f.write("%s  # %s\n" % (n, why))

    print("base=0x%06X span=0x%X (%d bytes)" % (base, span, span))
    print("placed=%d  (const %d)  unresolved=%d  (of %d)"
          % (len(placed), sum(1 for _, _, c, _ in placed if c), len(unresolved), len(names)))
    print("wrote arena.ld, arena_storage.c, arena_map.txt, arena_unresolved.txt to", outdir)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
# classify_const_globals.py - flag data_globals.txt entries that are actually
# static const/rodata (string literals) in the boot ELF, not mutable globals.
#
# Rationale (see ../inlevel_layout_delta.md): the in-level RE showed a category of
# arena entries placed onto static rodata string literals (e.g. "render setup"
# @0x1A8860, g_vendorCaptionFmt @0x1AD338). These are CONSTANTS, never live state:
# the native build + the tester's effect-diff must treat them as const (exclude
# from state validation). This scanner reads each data_globals entry's bytes from
# the static boot ELF and flags those that point into a printable string literal.
#
# Usage: classify_const_globals.py [data_globals.txt] [symbol_addrs.txt] [elf]

import sys, os, re, struct, string

ROOT = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(ROOT, "../../../.."))
DG = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "data_globals.txt")
SYMS = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
    REPO, "going-decompiled/symbol_addrs/usa/symbol_addrs.txt")
ELF = sys.argv[3] if len(sys.argv) > 3 else os.path.join(REPO, "extracted/usa/SCUS_972.68")

PRINTABLE = set(bytes(string.printable[:-5], "ascii"))


def load_elf_loads(elf):
    f = open(elf, "rb").read()
    le = "<" if f[5] == 1 else ">"
    phoff = struct.unpack(le + "I", f[28:32])[0]
    phentsize = struct.unpack(le + "H", f[42:44])[0]
    phnum = struct.unpack(le + "H", f[44:46])[0]
    segs = []
    for i in range(phnum):
        o = phoff + i * phentsize
        t, off, va, pa, fsz, msz, fl, al = struct.unpack(le + "8I", f[o:o + 32])
        if t == 1:
            segs.append((va, off, fsz))
    return f, segs


def elf_bytes(f, segs, va, n):
    for vbase, off, fsz in segs:
        if vbase <= va < vbase + fsz:
            o = off + (va - vbase)
            return f[o:o + n]
    return None


def load_named(path):
    addr = {}
    rx = re.compile(r'^([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]+)\s*;')
    for ln in open(path):
        m = rx.match(ln.strip())
        if m:
            addr[m.group(1)] = int(m.group(2), 16)
    return addr


def resolve(name, named):
    m = re.fullmatch(r'D_([0-9A-Fa-f]{6,8})', name)
    if m:
        return int(m.group(1), 16)
    return named.get(name)


def string_around(f, segs, va):
    """If va points into a printable run (>=4) in the ELF, return (start_va, text)."""
    # walk back to run start (bounded), then forward
    b0 = elf_bytes(f, segs, va, 1)
    if not b0 or b0[0] not in PRINTABLE or b0[0] == 0:
        return None
    start = va
    for _ in range(64):
        prev = elf_bytes(f, segs, start - 1, 1)
        if prev and prev[0] in PRINTABLE and prev[0] != 0:
            start -= 1
        else:
            break
    # forward run STOPS at the first NUL (a C string terminator). Do NOT bridge
    # NUL gaps - otherwise a non-string scalar that sits a few NUL-padded bytes
    # before a rodata string would be mis-attributed to that string (the D_1A8630
    # bug: scalar 64 at 0x1A8630, 7 NULs, then "Camera_..." at 0x1A8638).
    run = b""
    v = start
    for _ in range(96):
        c = elf_bytes(f, segs, v, 1)
        if not c or c[0] not in PRINTABLE:  # NUL (not in PRINTABLE) terminates
            break
        run += c
        v += 1
    txt = "".join(chr(c) if c in PRINTABLE and c != 0 else "." for c in run)
    # require genuine text: a run of >=3 CONSECUTIVE ASCII letters (a "word").
    # This rejects float/int data whose bytes happen to be printable letters in
    # isolation (e.g. the 0x42/0x50/0x54 = B/P/T bytes of 40.0f / pointer scalars)
    # while keeping real C string literals, which always contain words.
    best = cur = 0
    for c in run:
        if (65 <= c <= 90) or (97 <= c <= 122):
            cur += 1
            best = max(best, cur)
        else:
            cur = 0
    return (start, txt) if best >= 3 else None


def main():
    f, segs = load_elf_loads(ELF)
    named = load_named(SYMS)
    names = sorted({ln.strip() for ln in open(DG) if ln.strip()})
    const_hits = []
    for n in names:
        a = resolve(n, named)
        if a is None:
            continue
        s = string_around(f, segs, a)
        if s:
            const_hits.append((a, n, s[0], s[1]))
    const_hits.sort()
    print(f"# data_globals entries pointing into a static ELF string literal: "
          f"{len(const_hits)} of {len(names)}")
    print("# (these are CONST/rodata, not mutable state — exclude from effect-diff)")
    for a, n, st, txt in const_hits:
        off = f" (+0x{a-st:X} into str@0x{st:08X})" if st != a else ""
        print(f"  0x{a:08X} {n:30s} '{txt[:40]}'{off}")
    # emit just the names (for diffing data_globals)
    if "--names" in sys.argv:
        print("---NAMES---")
        for a, n, st, txt in const_hits:
            print(n)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""symtab_extent_compare.py — compare every word of each compiled function
against the ROM, taking the function's extent from the object's OWN symbol table.

WHAT IT COMPARES
----------------
For each function in a unit object (an `objdiff_build.sh` base object,
`going-decompiled/build/<region>/obj/<unit>.o`), the words at
`st_value .. st_value + st_size` of the `.text` section, with relocations
applied, against the flat ROM at `file offset = vaddr - 0x100080`.

The extent is the symtab `value..value+size` that `objdump -t` prints, read
straight from the ELF (`.symtab`, `.text` bytes and `.rel.text`). objdump's
disassembler is never consulted for the compared words. `objdump -dr` without
`-z` prints a run of zero words as a single `\t...` line, so any offset it did
not print inside an extent was zero in the object by definition; a tool that
compares only the printed words never sees those offsets. Reading the ELF
directly makes this comparison elision-proof in every position — LEADING
(before the first printed word), INTERIOR and TRAILING.

`verify_match_unit.sh` slices the `objdump -dr` text and so skips elided words:
with a zero seeded next to a nop pair it returns a false MATCH (FACT #7936, and
this tool's --selftest below). This is a SECOND INSTRUMENT, not a flag on that
one. The two are meant to be run side by side.

WHAT IT DOES NOT COVER
----------------------
* It is NOT a replacement for `landing_gate.sh`'s whole-image `cmp`. It checks
  each function in place against the ROM at that function's own address; a pad
  dropped or added between functions, shifting everything after it, is a
  linked-image property this tool cannot see.
* Relocated fields are only as right as the relocation arithmetic, and that is
  the same rule as `verify_match_unit.sh` (`resolve()`, `sign16()`, the
  HI16/LO16/26/PC16/GPREL16 cases, the per-region `_gp`, HI16/LO16 pairing in
  .rel.text table order, section-symbol placement). A resolution defect common
  to both is invisible to a differential between them, and this tool says
  nothing about whether that arithmetic is correct. That is not hypothetical:
  both carried the HI16 carry defect of FACT #8027 (the paired LO16 addend was
  dropped; it errs toward DIFFERS and, on a lui+1 base, toward MATCH — FACT
  #8414) and the section-symbol gap of FACT #8410 until each was fixed on its
  own evidence (verify_match_unit: tasks #977, #966; this tool: task #990).
  Only the whole-image cmp in landing_gate.sh is independent of this arithmetic.
* The symtab `st_size` is trusted as the extent. Words after it (alignment
  padding) are not compared.
* A function still under `INCLUDE_ASM` in the base object MATCHES BY
  CONSTRUCTION. Such functions are dropped from the population: any function
  with a `<fn>.NON_MATCHING` marker in the object is skipped. One output row =
  one STT_FUNC symbol in `.text`, size > 0, with no marker.

USAGE
-----
  symtab_extent_compare.py <obj.o> [--fn NAME] [--only FILE] [--dis FILE]
  symtab_extent_compare.py --all [--dis-dir DIR]
  symtab_extent_compare.py --selftest [--write-seed DIR]
  common: [--region usa|eu] [--rom PATH]

  --fn NAME     one function only. If NAME selects no function (INCLUDE_ASM,
                a size-0 `alabel` inside another function, not in the object),
                it prints one NOROW row saying why, and exits 2.
  --only FILE   restrict to the names in FILE (an engine96 object's owned set,
                `tools/ee/.objdiff/<region>/<unit>/engine_funcs.txt`).
  --all         every object under going-decompiled/build/<region>/obj; an
                `<unit>.engine96.o` is restricted to its engine_funcs.txt.
  --dis FILE    `mips-linux-gnu-objdump -dr --section=.text <obj.o>` of the SAME
                object (the command verify_match_unit.sh runs, WITHOUT -z). Adds
                columns showing which words verify_match_unit's own slicing
                compares, emulated with its regexes, its `unit_off = words[0][0]`
                and its stop at the next block header. objdump is not on the host
                PATH; produce the dump in the VM:
                  tools/ee/vm.sh 'mips-linux-gnu-objdump -dr --section=.text \\
                      going-decompiled/build/usa/obj/text/1A8180.o' > 1A8180.dis
  --dis-dir DIR with --all: the dump of <obj path> is DIR/<obj path, '/'->'_'>.dis
  Paths are relative to the repo root; run from there. The ROM defaults to
  extracted/<region>/<SCUS_972.68|SCES_516.07>.rom (override with --rom).

  The objects must exist: run `tools/ee/objdiff_build.sh <region> <unit>` first.
  Output is TSV on stdout, one header line then one row per function. Nothing
  is written unless --write-seed is given. The tool modifies nothing and no
  build or gate reads it.

  Columns: object fn vaddr size_words extent_verdict extent_diffs
    and with --dis/--dis-dir also:
           vmu_words vmu_first_off uncompared_offs uncompared_nonzero_in_rom
           vmu_verdict
  `uncompared_offs` are offsets inside the extent that verify_match_unit's slice
  never sees; `uncompared_nonzero_in_rom` are those whose ROM word is non-zero,
  i.e. where a skipped word would really differ.

EXIT STATUS (the bands of verify_match_unit.sh from 8b2c0190 on)
  0  MATCH         every row MATCH
  1  DIFFERS       at least one row DIFFERS
  2  UNVERIFIABLE  no row DIFFERS, but at least one could not be decided
                   (unresolvable symbol, a section symbol that cannot be placed,
                   a HI16 with no paired LO16, unmodelled relocation type, no
                   vaddr, outside the ROM, engine96 object with no owned-set
                   list, a NOROW, or no rows at all). NOT a pass and NOT a fail.
  3  USAGE/ARG     bad arguments or a missing input file

THE CONTROL (--selftest)
------------------------
Seeds five copies of the BASE object's `func_002AC058` (text/1A8180, landed
byte-exact by #659, 12 words) in memory and requires:
  unseeded                  -> MATCH
  +0x10 zeroed (30420020)   -> DIFFERS at 0x2ac068   the #7936 elision class
  +0x14 set to 1 (zero pad) -> DIFFERS at 0x2ac06c   a word objdump elides
  +0x00 bit 31 flipped      -> DIFFERS at 0x2ac058   leading
  +0x2c bit 31 flipped      -> DIFFERS at 0x2ac084   trailing
func_002AC058 has no relocations, so those five cannot see the relocation
arithmetic (FACT #8103). Two more subjects cover it:
  StreamSceneSegment (text/191238, real C), HI16/LO16 pairing:
  unseeded                  -> MATCH                 the carry of FACT #8027;
                                                     the old rule read DIFFERS
  +0x4 lui immediate +1     -> DIFFERS at 0x29455c   the old rule read MATCH
                                                     (FACT #8414)
  +0xc paired LO16 ^ 0x100  -> DIFFERS at 0x294564
  +0xc paired LO16 -> 0x150 -> DIFFERS at 0x29455c   %lo just short of the carry
  EvaluateProgressCondition (text/198FA0, real C), section symbol `.rodata`:
  unseeded                  -> MATCH                 the old tool: UNVERIFIABLE
  +0x20 %lo(.rodata) +1     -> DIFFERS at 0x29e828   (FACT #8410)
It exits 0 only if every expectation holds, 1 if any does not. Needs
`objdiff_build.sh usa text/1A8180` (and text/191238, text/198FA0) to have run.

⚠️ THE TRAP INSIDE A ZERO SEED. Zeroing a word that is already zero changes
nothing, so the comparator reports MATCH and the control has not fired — which
looks exactly like a clean subject. The selftest therefore checks the ORIGINAL
word of every seed (+0x10 must be non-zero, +0x14 must be zero, the flips must
not touch a relocated offset, a relocation seed must sit on the relocation it
names, and the carry subject must really carry) and fails as "CONTROL INVALID"
if it is not what the seed assumes. Prefer a non-zero seed, or a word confirmed
to be printed.

--write-seed DIR also writes the +0x10 zero-seeded object and a one-function
target for the SAME seed against the real verify_match_unit.sh, and prints the
commands. Measured at master 138d1c1c: verify_match_unit returns rc 0 BYTE
IDENTICAL 9/9 (a false MATCH) on that object while this tool reports DIFFERS at
0x2ac068. Default DIR: tools/ee/.audit_symtab_extent/ (gitignored by
`tools/ee/.audit*/`). DIR must be inside the repo: the VM mounts only the repo.
"""
import argparse
import os
import re
import struct
import sys

MATCH, DIFFERS, UNVERIFIABLE, ARGERR = 0, 1, 2, 3
ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
ROM_NAME = {"usa": "SCUS_972.68.rom", "eu": "SCES_516.07.rom"}

# ---- the relocation rule of verify_match_unit.sh -----------------------------
# Keep in step with it: this tool is a differential against that one, so a
# divergence here would show up as a verdict difference that is not a
# word-selection difference. The HI16 pairing and section placement live in Elf
# below, because this tool reads the ELF where verify_match_unit reads objdump.
ROM_BASE = 0x100080
GP_BY_REGION = {"usa": 0x1AEFF0, "eu": 0x1AF070}
syms = {}
GP = None


def load_syms(path):
    for line in open(path):
        m = re.match(r"\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)", line)
        if m:
            syms[m.group(1)] = int(m.group(2), 16)


def sign16(v):
    return v - 0x10000 if v & 0x8000 else v


def resolve(name):
    if name in syms:
        return syms[name]
    m = re.fullmatch(r"\.L([0-9A-Fa-f]{6,8})", name)
    if m:
        return int(m.group(1), 16)
    m = re.fullmatch(r"D_([0-9A-Fa-f]{1,5})", name)
    if m:
        return int(m.group(1), 16)
    m = re.fullmatch(r"(?:func|D|jtbl|L)_([0-9A-Fa-f]{6,8})", name)
    if m:
        return int(m.group(1), 16)
    return None


def apply_relocs(va, w, rels, hi_lo, place_section):
    """Returns (word, unresolved_names, unmodelled_types, unpaired_hi16s).

    `hi_lo` is sign16 of the paired LO16 immediate for a HI16 at this word, or
    a reason string when it has none (see Elf.hi16_pairs). `place_section(name)`
    places a section symbol (`.rodata`, `.text`) or returns None."""
    unres, unmod, unpaired = [], set(), []
    for rtype, rname in rels:
        S = resolve(rname)
        if S is None and rname.startswith("."):
            S = place_section(rname)
        if S is None:
            unres.append(rname)
            continue
        if rtype == "R_MIPS_HI16":
            if not isinstance(hi_lo, int):
                unpaired.append("0x%08x %s (%s)" % (va, rname, hi_lo))
                continue
            A = (sign16(w & 0xFFFF) << 16) + hi_lo
            w = (w & 0xFFFF0000) | (((S + A + 0x8000) >> 16) & 0xFFFF)
        elif rtype == "R_MIPS_LO16":
            w = (w & 0xFFFF0000) | ((S + sign16(w & 0xFFFF)) & 0xFFFF)
        elif rtype == "R_MIPS_26":
            A = (w & 0x03FFFFFF) << 2
            w = (w & 0xFC000000) | (((S + A) >> 2) & 0x03FFFFFF)
        elif rtype == "R_MIPS_PC16":
            w = (w & 0xFFFF0000) | (((S - (va + 4)) >> 2) & 0xFFFF)
        elif rtype == "R_MIPS_GPREL16" and GP is not None:
            w = (w & 0xFFFF0000) | ((S + sign16(w & 0xFFFF) - GP) & 0xFFFF)
        else:
            unmod.add(rtype)
    return w, unres, unmod, unpaired
# ------------------------------------------------------------------------------


class Elf:
    """The parts of an ELF32 LE relocatable object this tool reads."""

    RELTYPE = {2: "R_MIPS_32", 4: "R_MIPS_26", 5: "R_MIPS_HI16", 6: "R_MIPS_LO16",
               7: "R_MIPS_GPREL16", 10: "R_MIPS_PC16"}

    def __init__(self, data):
        d = self.data = bytes(data)
        shoff, = struct.unpack_from("<I", d, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from("<HHH", d, 0x2E)
        secs = self.secs = [struct.unpack_from("<IIIIIIIIII", d, shoff + i * shentsize) for i in range(shnum)]
        secname = self.secname = [self._cstr(secs[shstrndx][4] + s[0]) for s in secs]
        self.text_idx = secname.index(".text")
        self.text_off = secs[self.text_idx][4]
        self.text = d[self.text_off:self.text_off + secs[self.text_idx][5]]
        st = [s for s in secs if s[1] == 2][0]  # SHT_SYMTAB
        strtab_off = secs[st[6]][4]
        self.symbols = []
        for i in range(st[5] // 16):
            n, v, sz, info, _other, shndx = struct.unpack_from("<IIIBBH", d, st[4] + i * 16)
            typ = info & 0xF
            name = secname[shndx] if typ == 3 and shndx < len(secname) else self._cstr(strtab_off + n)
            self.symbols.append((name, v, sz, typ, shndx))
        self.rel = {}
        for s in secs:
            if s[1] == 9 and s[7] == self.text_idx:  # SHT_REL applying to .text
                for k in range(s[5] // 8):
                    r_off, r_info = struct.unpack_from("<II", d, s[4] + k * 8)
                    rtype = self.RELTYPE.get(r_info & 0xFF, "R_%d" % (r_info & 0xFF))
                    self.rel.setdefault(r_off, []).append((rtype, self.symbols[r_info >> 8][0]))
        self.nonmatching = {n[:-len(".NON_MATCHING")] for n, *_ in self.symbols
                            if n.endswith(".NON_MATCHING")}
        self.hi16_lo = self.hi16_pairs()
        self._placed = {}

    # HI16/LO16 PAIRING — the same rule as verify_match_unit.sh (task #977). A
    # REL HI16's addend is AHL = (hi_imm << 16) + sign16(lo_imm) of the LO16 it
    # pairs with; the HI16's own immediate alone loses the carry whenever the low
    # half pushes %lo past 0x7FFF (FACT #8027: `lui %hi(g_cameraSlotActive +
    # 0x990)` came out 0x1B where the ROM has 0x1C). Dropping it errs BOTH ways:
    # a base whose lui is +1 then reads MATCH (FACT #8414).
    # The pairing is GNU ld's: the NEXT R_MIPS_LO16 in .rel.text TABLE order with
    # the same symbol index (mips_elf_next_relocation, elfxx-mips.c) — not the
    # next LO16 in the instruction stream, which misses 8 ROM sites and picks the
    # wrong LO16 where one HI16's pair lies in the next function (NOTE #8412).
    # The LO16 immediate is read from .text, so a LO16 outside the function's
    # extent still counts. A HI16 with no such LO16 is UNVERIFIABLE: ld refuses
    # to link it too, so there is no value to check.
    def hi16_pairs(self):
        """{HI16 r_offset: sign16(paired lo_imm), or a reason string if none}."""
        d, secs = self.data, self.secs
        texts = [i for i, n in enumerate(self.secname) if n == ".text"]
        rels = [s for s in secs if s[1] == 9 and s[7] == self.text_idx]
        why = ("%d sections named .text in the object" % len(texts) if len(texts) != 1 else
               "%d SHT_REL sections for .text in the object" % len(rels) if len(rels) > 1 else None)
        table = [struct.unpack_from("<II", d, s[4] + k * 8) for s in rels for k in range(s[5] // 8)]
        pairs = {}
        for i, (r_off, r_info) in enumerate(table):
            if r_info & 0xFF != 5:                  # R_MIPS_HI16
                continue
            pairs[r_off] = why or "no later LO16 on the same symbol in .rel.text"
            if why:
                continue
            for lo_off, lo_info in table[i + 1:]:
                if lo_info & 0xFF == 6 and lo_info >> 8 == r_info >> 8:   # LO16, same symbol
                    pairs[r_off] = sign16(self.word(lo_off) & 0xFFFF)
                    break
        return pairs

    # SECTION SYMBOLS — the same placement as verify_match_unit.sh (task #966).
    # GAS rewrites a relocation against a LOCAL symbol (a switch table, a float
    # literal, a `static`) as one against its SECTION, which this parser names
    # `.rodata`/`.data`/`.text`. That name is per-unit, so symbol_addrs cannot
    # place it, and every such function read UNVERIFIABLE (FACT #8410).
    #   .text  - the unit's .text starts at fn_va - st_value: the same position
    #            assumption every word compare here already makes.
    #   other  - the section's bytes, with their own R_MIPS_32 relocations
    #            resolved, must occur EXACTLY ONCE in the ROM, at any byte offset;
    #            that hit is its address. No hit, several hits, a NOBITS section or
    #            an unresolvable relocation inside it leaves the symbol unresolved.
    # This cannot make a wrong function MATCH: each word is still compared against
    # the ROM, and a wrong placement gives a wrong immediate. A defect in the
    # section's own bytes makes the search miss (UNVERIFIABLE, not MATCH).
    def place_section(self, name, text_base, rom):
        if name == ".text":
            return text_base
        key = (name, text_base)
        if key not in self._placed:
            self._placed[key] = self._search_section(name, text_base, rom)
        return self._placed[key]

    def _search_section(self, name, text_base, rom):
        d, secs = self.data, self.secs
        if self.secname.count(name) != 1:
            return None
        idx = self.secname.index(name)
        sec = secs[idx]
        if sec[1] == 8:                             # SHT_NOBITS: nothing to search for
            return None
        body = bytearray(d[sec[4]:sec[4] + sec[5]])
        for rel in secs:
            if rel[1] != 9 or rel[7] != idx:        # SHT_REL for this section
                continue
            for k in range(rel[5] // 8):
                r_off, r_info = struct.unpack_from("<II", d, rel[4] + k * 8)
                sname, _v, _sz, styp, _sh = self.symbols[r_info >> 8]
                S = text_base if styp == 3 and sname == ".text" else (None if styp == 3 else resolve(sname))
                if r_info & 0xFF != 2 or S is None:  # R_MIPS_32 only
                    return None
                A, = struct.unpack_from("<I", body, r_off)
                struct.pack_into("<I", body, r_off, (S + A) & 0xFFFFFFFF)
        hit = rom.find(bytes(body))
        if hit == -1 or rom.find(bytes(body), hit + 1) != -1:
            return None
        return hit + ROM_BASE

    def _cstr(self, off):
        return self.data[off:self.data.index(b"\0", off)].decode()

    def functions(self):
        """(name, st_value, st_size) of every STT_FUNC in .text, size > 0, real C."""
        return sorted(((n, v, sz) for n, v, sz, typ, sh in self.symbols
                       if typ == 2 and sh == self.text_idx and sz > 0 and n not in self.nonmatching),
                      key=lambda f: f[1])

    def word(self, off):
        return struct.unpack_from("<I", self.text, off)[0]


def verdict(rom, pairs, unres, unmod, unpaired):
    if unmod or unres or unpaired:
        return "UNVERIFIABLE", []
    bad = []
    for va, w in pairs:
        o = va - ROM_BASE
        if o < 0 or o + 4 > len(rom):
            return "UNVERIFIABLE", []
        if struct.unpack_from("<I", rom, o)[0] != w:
            bad.append(va)
    return ("DIFFERS" if bad else "MATCH"), bad


def extent_compare(rom, elf, value, size, fn_va):
    """Every word of st_value..st_value+st_size, relocated, against the ROM."""
    return relocated_verdict(rom, elf, [(off, elf.word(off)) for off in range(value, value + size, 4)],
                             elf.rel, fn_va - value, fn_va - value)


def relocated_verdict(rom, elf, words, rels, text_base, va_base):
    """Resolve relocations into (.text offset, word) pairs and compare them
    against the ROM at va_base + offset. Relocations are read from `rels`, keyed
    by .text offset; HI16 pairing and section placement always from the ELF."""
    pairs, unres, unmod, unpaired = [], [], set(), []
    for off, w in words:
        va = va_base + off
        w, u1, u2, u3 = apply_relocs(va, w, rels.get(off, []), elf.hi16_lo.get(off),
                                     lambda n: elf.place_section(n, text_base, rom))
        unres += u1
        unmod |= u2
        unpaired += u3
        pairs.append((va, w))
    return verdict(rom, pairs, unres, unmod, unpaired)


HDR = re.compile(r"^[0-9a-f]+ <(\S+)>:")


def vmu_slices(dis_text):
    """verify_match_unit.sh's own per-function slice of `objdump -dr` text."""
    lines = dis_text.splitlines()
    first = {}
    for i, line in enumerate(lines):
        m = HDR.match(line)
        if m and m.group(1) not in first:
            first[m.group(1)] = i

    def slice_of(fn):
        i = first.get(fn)
        if i is None:
            return None, None
        words, relocs = [], {}
        for line in lines[i + 1:]:
            if HDR.match(line):
                break
            m = re.match(r"^\s+([0-9a-f]+):\t([0-9a-f]{8}) ", line)
            if m:
                words.append((int(m.group(1), 16), int(m.group(2), 16)))
                continue
            m = re.match(r"^\s+([0-9a-f]+): (R_MIPS_\w+)\s+(\S+)", line)
            if m:
                relocs.setdefault(int(m.group(1), 16), []).append((m.group(2), m.group(3)))
        return words, relocs
    return slice_of


def why_no_row(elf, fn):
    """Why `--fn fn` selects no function, so the answer is never an empty table."""
    hits = [(v, sz, typ, sh) for n, v, sz, typ, sh in elf.symbols if n == fn]
    if not hits:
        return "no symbol %s in the object" % fn
    v, sz, typ, sh = hits[0]
    if typ != 2 or sh != elf.text_idx:
        return "%s is not an STT_FUNC in .text" % fn
    if fn in elf.nonmatching:
        return "%s is INCLUDE_ASM (%s.NON_MATCHING); it would MATCH by construction" % (fn, fn)
    # An `alabel` (.aent): typed @function but no .size, so st_size is 0. It is an
    # entry label inside another function, not a function of its own
    # (func_002F33E8 in EmitTieDrawPackets, FACT #7359).
    owner = [n for n, ov, osz, otyp, osh in elf.symbols
             if otyp == 2 and osh == elf.text_idx and osz > 0 and ov <= v < ov + osz]
    return "%s has st_size 0 at .text+0x%x: an alias label inside %s, not a function" % (
        fn, v, ", ".join(owner) or "no sized function")


def compare_object(rom, obj_path, only=None, fn=None, dis_path=None):
    """Yields one TSV row (list of str) and its verdict per function."""
    elf = Elf(open(obj_path, "rb").read())
    slice_of = vmu_slices(open(dis_path).read()) if dis_path else None
    if fn is not None and fn not in {n for n, _v, _sz in elf.functions()}:
        row = [obj_path, fn, "-", "-", "NOROW", why_no_row(elf, fn)]
        yield row + ["-"] * 5 if slice_of else row, "UNVERIFIABLE"
        return
    for name, value, size in elf.functions():
        if (only is not None and name not in only) or (fn is not None and name != fn):
            continue
        fn_va = resolve(name)
        if fn_va is None:
            row = [obj_path, name, "-", str(size // 4), "NOVADDR", "-"]
            if slice_of:
                row += ["-"] * 5
            yield row, "UNVERIFIABLE"
            continue
        ev, ebad = extent_compare(rom, elf, value, size, fn_va)
        row = [obj_path, name, "0x%08x" % fn_va, str(size // 4), ev,
               ",".join("0x%08x" % va for va in ebad[:8]) or "-"]
        if slice_of:
            words, relocs = slice_of(name)
            if not words:
                row += ["0", "-", "-", "-", "NOTFOUND"]
            else:
                unit_off = words[0][0]
                # verify_match_unit places the unit (and so `.text`) from the
                # first PRINTED word, not from st_value.
                vv, _ = relocated_verdict(rom, elf, words, relocs, fn_va - unit_off, fn_va - unit_off)
                seen = {off for off, _ in words}
                unc = [o for o in range(value, value + size, 4) if o not in seen]
                unc_nz = [o for o in unc if 0 <= fn_va + o - value - ROM_BASE <= len(rom) - 4 and
                          struct.unpack_from("<I", rom, fn_va + o - value - ROM_BASE)[0] != 0]
                fmt = lambda offs: ",".join("+0x%x" % (o - value) for o in offs) or "-"
                row += [str(len(words)), "0x%x" % (unit_off - value), fmt(unc), fmt(unc_nz), vv]
        yield row, ev


def exit_band(verdicts):
    if "DIFFERS" in verdicts:
        return DIFFERS
    if not verdicts or any(v != "MATCH" for v in verdicts):
        return UNVERIFIABLE
    return MATCH


def header(with_dis):
    cols = ["object", "fn", "vaddr", "size_words", "extent_verdict", "extent_diffs"]
    if with_dis:
        cols += ["vmu_words", "vmu_first_off", "uncompared_offs", "uncompared_nonzero_in_rom", "vmu_verdict"]
    return "\t".join(cols)


def read_only_list(path):
    return {l.strip() for l in open(path) if l.strip()}


def run_all(rom, region, dis_dir):
    base = os.path.join("going-decompiled", "build", region, "obj")
    objs = sorted(os.path.join(dp, f) for dp, _, fs in os.walk(base) for f in fs if f.endswith(".o"))
    if not objs:
        print("ARG ERROR: no objects under %s — run objdiff_build.sh first" % base, file=sys.stderr)
        return ARGERR
    print(header(dis_dir is not None))
    verdicts = []
    for o in objs:
        unit = os.path.relpath(o, base)[:-len(".o")]
        only = None
        if unit.endswith(".engine96"):
            unit = unit[:-len(".engine96")]
            lst = os.path.join("tools", "ee", ".objdiff", region, unit, "engine_funcs.txt")
            if not os.path.isfile(lst):
                print("UNVERIFIABLE: %s has no owned-set list %s; object skipped" % (o, lst), file=sys.stderr)
                verdicts.append("UNVERIFIABLE")
                continue
            only = read_only_list(lst)
        dis = os.path.join(dis_dir, o.replace("/", "_") + ".dis") if dis_dir else None
        if dis and not os.path.isfile(dis):
            print("ARG ERROR: missing dump %s" % dis, file=sys.stderr)
            return ARGERR
        for row, v in compare_object(rom, o, only=only, dis_path=dis):
            print("\t".join(row))
            verdicts.append(v)
    return exit_band(verdicts)


# ---- the control -------------------------------------------------------------
SELFTEST_OBJ = "going-decompiled/build/usa/obj/text/1A8180.o"
SELFTEST_FN = "func_002AC058"
SELFTEST_ASM = "going-decompiled/asm/usa/nonmatchings/text/1A8180/func_002AC058.s"
# (label, offset, how to seed, required ORIGINAL word predicate, expected verdict, expected first diff va)
SELFTEST_SEEDS = [
    ("unseeded", None, None, None, "MATCH", None),
    ("+0x10 zeroed (the #7936 elision class)", 0x10, lambda w: 0, lambda w: w != 0, "DIFFERS", 0x2AC068),
    ("+0x14 set to 1 (a word objdump elides)", 0x14, lambda w: 1, lambda w: w == 0, "DIFFERS", 0x2AC06C),
    ("+0x00 bit 31 flipped (leading)", 0x00, lambda w: w ^ 0x80000000, None, "DIFFERS", 0x2AC058),
    ("+0x2c bit 31 flipped (trailing)", 0x2C, lambda w: w ^ 0x80000000, None, "DIFFERS", 0x2AC084),
]


def lo16_imm_plus(n):
    return lambda w: (w & 0xFFFF0000) | ((w + n) & 0xFFFF)


# The relocation subjects. Each seed names the relocation (type, symbol) that
# must sit on the seeded word; the unseeded row of the carry subject names the
# (HI16, LO16) offsets that must really carry (see carries()).
# (object, fn, [(label, offset, how, required reloc, expected verdict, expected first diff va)])
SELFTEST_RELOC = [
    ("going-decompiled/build/usa/obj/text/191238.o", "StreamSceneSegment", [
        ("unseeded (HI16 carry, FACT #8027)", None, None, (0x4, 0xC), "MATCH", None),
        ("+0x4 lui immediate +1 (FACT #8414)", 0x4, lo16_imm_plus(1), ("R_MIPS_HI16", "g_cameraSlotActive"),
         "DIFFERS", 0x29455C),
        ("+0xc paired LO16 immediate ^ 0x100", 0xC, lambda w: w ^ 0x100, ("R_MIPS_LO16", "g_cameraSlotActive"),
         "DIFFERS", 0x294564),
        # g_cameraSlotActive is 0x1B7E30: an addend of 0x150 puts %lo at 0x7F80,
        # just short of the carry, so %hi must drop to 0x1B at the lui. A pairing
        # whose addend is too high by 0x80..0x807F carries and misses it there.
        ("+0xc paired LO16 immediate -> 0x150", 0xC, lambda w: (w & 0xFFFF0000) | 0x150,
         ("R_MIPS_LO16", "g_cameraSlotActive"), "DIFFERS", 0x29455C),
    ]),
    ("going-decompiled/build/usa/obj/text/198FA0.o", "EvaluateProgressCondition", [
        ("unseeded (.rodata section symbol, FACT #8410)", None, None, None, "MATCH", None),
        ("+0x20 %lo(.rodata) immediate +1", 0x20, lo16_imm_plus(1), ("R_MIPS_LO16", ".rodata"),
         "DIFFERS", 0x29E828),
    ]),
]


def carries(elf, hi_off, lo_off):
    """True if the HI16 at .text+hi_off gets a different %hi with the addend of
    the LO16 at .text+lo_off than without it, i.e. the subject exercises the
    carry at all. Read from the raw words, not from Elf.hi16_pairs, so a defect
    in the pairing cannot also disable its own guard."""
    hi = [n for t, n in elf.rel.get(hi_off, []) if t == "R_MIPS_HI16"]
    if not hi or ("R_MIPS_LO16", hi[0]) not in elf.rel.get(lo_off, []) or resolve(hi[0]) is None:
        return False
    S, A = resolve(hi[0]), sign16(elf.word(hi_off) & 0xFFFF) << 16
    return ((S + A + 0x8000) >> 16) != ((S + A + sign16(elf.word(lo_off) & 0xFFFF) + 0x8000) >> 16)


def selftest_relocs(rom):
    """The relocation subjects of SELFTEST_RELOC. Returns True if all hold."""
    ok = True
    for obj, fn, seeds in SELFTEST_RELOC:
        elf = Elf(open(obj, "rb").read())
        fns = {n: (v, sz) for n, v, sz in elf.functions()}
        if fn not in fns:
            print("CONTROL INVALID: %s not a real-C function in %s" % (fn, obj))
            ok = False
            continue
        value, size = fns[fn]
        fn_va = resolve(fn)
        for label, off, how, need, want, want_va in seeds:
            subject = elf
            if off is None and need is not None and not carries(elf, value + need[0], value + need[1]):
                print("CONTROL INVALID  %-42s the HI16 at +0x%x does not carry with the LO16 at +0x%x"
                      % (label, need[0], need[1]))
                ok = False
                continue
            if off is not None:
                if off + 4 > size or need not in elf.rel.get(value + off, []):
                    print("CONTROL INVALID  %-42s +0x%x does not carry %s %s" % (label, off, need[0], need[1]))
                    ok = False
                    continue
                subject = Elf(seeded(elf, value, off, how))
            got, bad = extent_compare(rom, subject, value, size, fn_va)
            hit = got == want and (want_va is None or (bad and bad[0] == want_va))
            ok &= hit
            print("%-5s %-42s -> %s%s" % ("ok" if hit else "FAIL", "%s %s" % (fn, label), got,
                                           " @0x%x" % bad[0] if bad else ""))
    return ok


def seeded(elf, value, off, how):
    d = bytearray(elf.data)
    p = elf.text_off + value + off
    struct.pack_into("<I", d, p, how(struct.unpack_from("<I", d, p)[0]) & 0xFFFFFFFF)
    return d


def selftest(rom, write_seed):
    for obj in [SELFTEST_OBJ] + [o for o, _fn, _s in SELFTEST_RELOC]:
        if not os.path.isfile(obj):
            print("ARG ERROR: %s missing — run: bash tools/ee/objdiff_build.sh usa %s"
                  % (obj, os.path.relpath(obj, "going-decompiled/build/usa/obj")[:-len(".o")]), file=sys.stderr)
            return ARGERR
    elf = Elf(open(SELFTEST_OBJ, "rb").read())
    if SELFTEST_FN in elf.nonmatching:
        print("CONTROL INVALID: %s is INCLUDE_ASM in %s (carries .NON_MATCHING); "
              "a MATCH would be by construction" % (SELFTEST_FN, SELFTEST_OBJ))
        return DIFFERS
    fns = {n: (v, sz) for n, v, sz in elf.functions()}
    if SELFTEST_FN not in fns:
        print("CONTROL INVALID: %s not a real-C function in %s" % (SELFTEST_FN, SELFTEST_OBJ))
        return DIFFERS
    value, size = fns[SELFTEST_FN]
    fn_va = resolve(SELFTEST_FN)
    ok = True
    for label, off, how, pre, want, want_va in SELFTEST_SEEDS:
        subject = elf
        if off is not None:
            orig = elf.word(value + off)
            if off + 4 > size or (pre is not None and not pre(orig)) or (value + off) in elf.rel:
                print("CONTROL INVALID  %-42s original word 0x%08x at +0x%x is not what the seed assumes"
                      % (label, orig, off))
                ok = False
                continue
            subject = Elf(seeded(elf, value, off, how))
        got, bad = extent_compare(rom, subject, value, size, fn_va)
        hit = got == want and (want_va is None or (bad and bad[0] == want_va))
        ok &= hit
        print("%-5s %-42s -> %s%s" % ("ok" if hit else "FAIL", label, got,
                                       " @0x%x" % bad[0] if bad else ""))
    ok &= selftest_relocs(rom)
    if write_seed:
        os.makedirs(write_seed, exist_ok=True)
        seed_o = os.path.join(write_seed, "seed_zero.o")
        with open(seed_o, "wb") as f:
            f.write(seeded(elf, value, 0x10, lambda w: 0))
        tgt_s = os.path.join(write_seed, "target.s")
        with open(tgt_s, "w") as f:
            f.write('.include "macro.inc"\n.section .text, "ax"\n.set noat\n.set noreorder\n'
                    '.include "%s"\n.set reorder\n.set at\n' % SELFTEST_ASM)
        tgt_o = os.path.join(write_seed, "target.o")
        print("\nwrote %s (base, arg 2: +0x10 zeroed) and %s. The same seed against the real tool:" % (seed_o, tgt_s))
        print("  tools/ee/vm.sh 'mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL -G0 "
              "-Igoing-decompiled/build/usa/include -o %s %s'" % (tgt_o, tgt_s))
        print("  bash tools/ee/verify_match_unit.sh %s %s %s usa; echo rc=$?" % (SELFTEST_FN, seed_o, tgt_o))
        print("  measured at 138d1c1c: rc=0 BYTE IDENTICAL 9/9 — a false MATCH on the object this tool calls DIFFERS")
    print("SELFTEST %s" % ("PASS" if ok else "FAIL"))
    return MATCH if ok else DIFFERS


def main():
    global GP
    ap = argparse.ArgumentParser(description="Compare each function's symtab extent against the ROM "
                                             "(see the module docstring).")
    ap.add_argument("obj", nargs="?")
    ap.add_argument("--fn")
    ap.add_argument("--only")
    ap.add_argument("--dis")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--dis-dir")
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--write-seed", nargs="?", const="tools/ee/.audit_symtab_extent")
    ap.add_argument("--region", default="usa", choices=sorted(GP_BY_REGION))
    ap.add_argument("--rom")
    try:
        a = ap.parse_args()
    except SystemExit as e:
        sys.exit(ARGERR if e.code else 0)
    modes = sum([bool(a.obj), a.all, a.selftest])
    if modes != 1 or (a.selftest and a.region != "usa") or (a.dis_dir and not a.all) \
            or (a.write_seed and not a.selftest) or ((a.dis or a.fn or a.only) and not a.obj):
        ap.print_usage(sys.stderr)
        sys.exit(ARGERR)

    os.chdir(ROOT)
    GP = GP_BY_REGION[a.region]
    symfile = os.path.join("going-decompiled", "symbol_addrs", a.region, "symbol_addrs.txt")
    rom_path = a.rom or os.path.join("extracted", a.region, ROM_NAME[a.region])
    for p in [symfile, rom_path] + [p for p in (a.obj, a.dis, a.only) if p]:
        if not os.path.isfile(p):
            print("ARG ERROR: not a file: %s" % p, file=sys.stderr)
            sys.exit(ARGERR)
    load_syms(symfile)
    rom = open(rom_path, "rb").read()

    if a.selftest:
        sys.exit(selftest(rom, a.write_seed))
    if a.all:
        sys.exit(run_all(rom, a.region, a.dis_dir))
    only = read_only_list(a.only) if a.only else None
    print(header(a.dis is not None))
    verdicts = []
    for row, v in compare_object(rom, a.obj, only=only, fn=a.fn, dis_path=a.dis):
        print("\t".join(row))
        verdicts.append(v)
    sys.exit(exit_band(verdicts))


if __name__ == "__main__":
    main()

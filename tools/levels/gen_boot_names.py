#!/usr/bin/env python3
"""Per-level boot name map for placing the boot's C functions in a level program.

Our own tool (task #2516, RULING #13219 conditions 2-4, NOTE #13218 option (c)).

    gen_boot_names.py places --level 24 --catalogue <functions.tsv> --boot-catalogue <boot functions.tsv>
    gen_boot_names.py names  --level 24 [--check]
    gen_boot_names.py bootld --level 24 <out.ld>
    gen_boot_names.py bootcmp --level 24 <linked boot-address ELF>
    gen_boot_names.py placed --level 24 <LNN.elf> <LNN.map> <extracted text.bin> <assembled asm dir>
    gen_boot_names.py users --level 24 <name>

places: writes going-decompiled/levels/usa/LNN/config/LNN_boot_places.tsv, the
  committed boot<->place table: one `boot_addr place size` row per boot-copy
  place of level NN in OpenRAC rac2-decomp's us2 catalogue (config/us2/levels/
  functions.tsv rows of kind `boot`, their boot address from config/us2/boot/
  functions.tsv). Read as DATA, addresses and sizes only; no name is copied
  (as LNN_functions.txt). Run once; the output is committed.

names: writes two committed files from committed inputs and the gated boot build:
  going-decompiled/levels/usa/LNN/config/LNN_boot_c.tsv
      one row per PLACED function: `func_LNN_<place> <boot name> <unit> <size>`;
  going-decompiled/levels/usa/LNN/symbol_addrs/LNN_boot_names.txt
      `name = 0xVALUE; // <source>` for every name a placed function's code
      relocates against, plus every placed function's own boot name, plus the
      reverse alias `func_LNN_<place> = <boot name>;` for each placed function
      so the level's remaining asm resolves to it.
  Inputs, all read-only:
    * LNN_boot_places.tsv (committed, above);
    * the level's committed split asm (asm/text/*.s) and core_externs.txt;
    * the boot build: SCUS_972.68.elf's symbol table and SCUS_972.68.map's
      unit .text ranges ("boot names"), and the sectioned boot objects of
      tools/levels/boot_objects.sh (going-decompiled/build/usa_levels/
      boot_objs/: <unit>.o, sections.tsv, refused.tsv) for each function's
      relocations and in-place addends.
  NO disc or ROM bytes are read: a level value comes from the operand our
  committed split asm writes at the same offset (`%hi(D_L24_001B1AB0 + 0x27CD0)`,
  `jal func_L24_002FCCE0`, `(0x1A7290 >> 16)`), never from an instruction word.
  Rules:
    * BOUNDARY. A catalogue boot address that falls INSIDE one of our compiled
      functions (the catalogue splits it where our C does not) is refused: the
      level carries only part of our function.
    * NAME RESOLUTION. A boot-copy place is ours when the boot map puts the
      boot address inside a C unit and exactly one function of that unit's
      sections.tsv has a boot-ELF symbol at that address. The function's name
      is that symbol's (the `.ent` name cc1 wrote), not symbol_addrs'.
    * Each relocation of `.text.<fn>`: the level target T is read from the
      split operand; the in-place addend A from the object (R_MIPS_26:
      (insn & 0x3FFFFFF) << 2; HI16/LO16: the (hi << 16) + sext(lo) pair;
      GPREL16: sext(imm)); the name's level value is T - A.
      Source of the value, recorded per name:
        core          its boot value is below 0x1A7C80 (the resident core):
                      the value is the boot value, and the split must agree;
        placed        a placed function's own name: its place;
        catalogue     a call to a boot function the catalogue pairs with a
                      place in this level, the split agreeing;
        split-operand otherwise (the committed split's operand).
      A relocation against a section symbol is allowed only for another
      placed function's `.text.<G>` (target = G's place + A); against any
      other section, a local symbol, an unknown split operand, or an
      unsupported type, the function is refused.
    * REFUSALS, by member, never silent: a function whose block was not
      sectioned (refused.tsv), whose section size != catalogue size, with no
      single split block at its place, with an unresolvable relocation; and
      EVERY user of a name that needs two values in this program. Iterated to
      a fixed point (refusing a function can strand a section-symbol caller).
  --check regenerates in memory and exits 1 if either committed file differs.

bootld: writes a GNU ld script placing every LNN_boot_c.tsv function's
  `.text.<fn>` at its BOOT address, with every LNN_boot_names.txt name at its
  BOOT value (boot ELF symtab) - the same objects linked as the boot links them.
bootcmp: compares each placed function's bytes in that link with the boot
  build's SCUS_972.68.elf (the image the boot gate proved == ROM) at its boot
  address. Prints `bootcmp: N of N equal` and every member that is not.
placed: after the level link, one row per LNN_boot_c.tsv function:
  `PLACED ok <func_LNN> <boot name>: map .text.<boot> <addr> <size> <obj>; names map <value>; glabel in assembled asm 0`
  needing map address == place == names-map value, map size == catalogue
  size, and the glabel func_LNN_<place> in none of the assembled asm. Then
  `PLACED-DIFFERS <func> (+off, n words)` for every placed function whose
  bytes differ from the extracted level .text (members, for the seeded arm).
users: the LNN_boot_c.tsv functions whose `.text.<fn>` relocation table names
  <name> (read from the boot objects' relocations, not from any bytes): the
  members a seeded value of <name> must make differ, and only those.
"""
import collections
import hashlib
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CORE_TOP = 0x1A7C80
BOOT = "going-decompiled/build/usa"
BOOT_ELF = BOOT + "/SCUS_972.68.elf"
BOOT_MAP = BOOT + "/SCUS_972.68.map"
BOBJ = "going-decompiled/build/usa_levels/boot_objs"
ADDR_NAME = re.compile(r"^(?:func|D|jtbl|B|RO|T|FLT|DBL|STR|PSTR)_(?:L\d\d_)?([0-9A-F]{1,8})$")
INSN = re.compile(r"^\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/\s+(\S+)\s*(.*?)\s*$")
R_32, R_26, R_HI16, R_LO16, R_GPREL16 = 2, 4, 5, 6, 7


def P(*a):
    return os.path.join(ROOT, *a)


def lvl_paths(nn):
    tag = "L%02d" % nn
    d = P("going-decompiled", "levels", "usa", tag)
    return tag, d, {
        "places": os.path.join(d, "config", "%s_boot_places.tsv" % tag),
        "c": os.path.join(d, "config", "%s_boot_c.tsv" % tag),
        "names": os.path.join(d, "symbol_addrs", "%s_boot_names.txt" % tag),
        "core": os.path.join(d, "symbol_addrs", "core_externs.txt"),
        "text": os.path.join(d, "asm", "text"),
    }


def sha1f(p):
    return hashlib.sha1(open(p, "rb").read()).hexdigest()


# ---------------------------------------------------------------- ELF ----
def read_elf(path):
    b = open(path, "rb").read()
    shoff, = struct.unpack_from("<I", b, 0x20)
    shent, shn, shstr = struct.unpack_from("<HHH", b, 0x2E)
    sh = [struct.unpack_from("<10I", b, shoff + shent * i) for i in range(shn)]
    so = sh[shstr][4]
    nm = lambda off, base: b[base + off:b.index(b"\0", base + off)].decode()
    secs = [dict(name=nm(s[0], so), type=s[1], flags=s[2], addr=s[3], off=s[4], size=s[5], link=s[6], info=s[7])
            for s in sh]
    syms = []
    for s in secs:
        if s["type"] == 2:
            strb = secs[s["link"]]["off"]
            for i in range(s["size"] // 16):
                n, v, sz, info, oth, shx = struct.unpack_from("<IIIBBH", b, s["off"] + 16 * i)
                syms.append(dict(name=nm(n, strb) if n else "", value=v, size=sz, bind=info >> 4,
                                 type=info & 15, shndx=shx))
    rel = collections.defaultdict(list)
    for s in secs:
        if s["type"] == 9:
            for i in range(s["size"] // 8):
                off, info = struct.unpack_from("<II", b, s["off"] + 8 * i)
                rel[s["info"]].append((off, info & 0xFF, info >> 8))
    phoff, = struct.unpack_from("<I", b, 0x1C)
    phn, = struct.unpack_from("<H", b, 0x2C)
    ph = [struct.unpack_from("<8I", b, phoff + 32 * i) for i in range(phn)]
    return dict(b=b, secs=secs, syms=syms, rel=rel, ph=ph)


def elf_bytes(E, va, n):
    for t, o, v, _, fs, *_ in E["ph"]:
        if t == 1 and v <= va and va + n <= v + fs:
            return E["b"][o + va - v:o + va - v + n]
    return None


def sext16(x):
    return x - 0x10000 if x & 0x8000 else x


# ----------------------------------------------------------- inputs ----
def read_places(path):
    rows = []
    for l in open(path):
        if not l.strip() or l.startswith("#") or l.startswith("//"):
            continue
        ba, pl, sz = l.split()
        rows.append((int(ba, 16), int(pl, 16), int(sz)))
    return rows


def read_core(path):
    out = {}
    for l in open(path):
        m = re.match(r"\s*([A-Za-z_][\w.$]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;", l)
        if m:
            out[m.group(1)] = int(m.group(2), 16)
    return out


def read_split(text_dir):
    """address -> (mnemonic, operands); glabel -> (nonmatching size)."""
    ins, blocks = {}, {}
    for f in sorted(os.listdir(text_dir)):
        if not f.endswith(".s"):
            continue
        for l in open(os.path.join(text_dir, f)):
            m = INSN.match(l)
            if m:
                ins[int(m.group(1), 16)] = (m.group(2), m.group(3))
                continue
            m = re.match(r"^nonmatching (\S+), 0x([0-9A-Fa-f]+)", l)
            if m:
                blocks[m.group(1)] = blocks.get(m.group(1), []) + [int(m.group(2), 16)]
    return ins, blocks


def boot_units(map_path):
    """unit -> (text start, size) from the boot map's `.text ADDR SIZE .../src/usa/text/<unit>.o` rows."""
    out = {}
    for l in open(map_path):
        m = re.match(r"^ \.text\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+\S*/src/usa/text/([0-9A-F]+)\.o\s*$", l)
        if m:
            out[m.group(3)] = (int(m.group(1), 16), int(m.group(2), 16))
    return out


def boot_symbols(E):
    glob, byaddr = {}, collections.defaultdict(set)
    for s in E["syms"]:
        if not s["name"] or s["shndx"] == 0 or s["type"] == 3 or s["type"] == 4:
            continue
        if s["bind"] in (1, 2):
            glob.setdefault(s["name"], s["value"])
        byaddr[s["value"]].add(s["name"])
    return glob, byaddr


def read_tsv(path, n):
    return [l.rstrip("\n").split("\t") for l in open(path) if l.strip() and not l.startswith("#")
            and len(l.rstrip("\n").split("\t")) == n]


# ------------------------------------------------------ split operand ----
def sym_value(tok, core):
    m = ADDR_NAME.match(tok)
    if m:
        return int(m.group(1), 16)
    return core.get(tok)


def expr_value(e, core):
    """`SYM`, `SYM + 0xN`, `SYM - 0xN`, `0xV` -> int, or None."""
    e = e.strip()
    m = re.fullmatch(r"([A-Za-z_][\w.$]*)\s*([+-])\s*(0x[0-9A-Fa-f]+|\d+)", e)
    if m:
        v = sym_value(m.group(1), core)
        return None if v is None else (v + (1 if m.group(2) == "+" else -1) * int(m.group(3), 0)) & 0xFFFFFFFF
    if re.fullmatch(r"[A-Za-z_][\w.$]*", e):
        return sym_value(e, core)
    if re.fullmatch(r"-?(0x[0-9A-Fa-f]+|\d+)", e):
        return int(e, 0) & 0xFFFFFFFF
    return None


def strip_ops(ops):
    """spimdisasm appends `/* Failed to symbolize ... */` to some operands."""
    return re.sub(r"/\*.*?\*/", "", ops).strip()


RAW_HI = re.compile(r"^\$\w+,\s*(-?0x[0-9A-Fa-f]+|-?\d+)$")                  # lui $v1, 0x1A
RAW_LO = re.compile(r"(?:^|,)\s*(-?0x[0-9A-Fa-f]+|-?\d+)(\(\$\w+\))?$")       # lw $a0, -0x7CBC($v1) / addiu $a0, $a0, -0x10


def hi_imm(ops, core):
    """The upper 16-bit immediate a split `lui` operand writes, or (None, why):
    `%hi(EXPR)` -> ((EXPR + 0x8000) >> 16), `(0xV >> 16)` -> V >> 16, raw `0x1A`."""
    m = re.search(r"%hi\((.+)\)$", ops)
    if m:
        v = expr_value(m.group(1), core)
        return (((v + 0x8000) >> 16) & 0xFFFF, None) if v is not None else (None, "%%hi operand %r" % m.group(1))
    m = re.search(r"\((0x[0-9A-Fa-f]+) >> 16\)$", ops)
    if m:
        return (int(m.group(1), 16) >> 16) & 0xFFFF, None
    m = RAW_HI.match(ops)
    if m:
        return int(m.group(1), 0) & 0xFFFF, None
    return None, "hi operand %r" % ops


def lo_imm(ops, core):
    """The signed low 16-bit immediate a split operand writes, or (None, why):
    `%lo(EXPR)` -> sext(EXPR & 0xFFFF), `(0xV & 0xFFFF)` -> sext(V & 0xFFFF), raw `-0x7CBC`."""
    m = re.search(r"%lo\((.+?)\)(\(\$\w+\))?$", ops)
    if m:
        v = expr_value(m.group(1), core)
        return (sext16(v & 0xFFFF), None) if v is not None else (None, "%%lo operand %r" % m.group(1))
    m = re.search(r"\((0x[0-9A-Fa-f]+) & 0xFFFF\)(\(\$\w+\))?$", ops)
    if m:
        return sext16(int(m.group(1), 16) & 0xFFFF), None
    m = RAW_LO.search(ops)
    if m:
        return sext16(int(m.group(1), 0) & 0xFFFF), None
    return None, "lo operand %r" % ops


def split_target(kind, ops, core, pair_ops=None, gp=0x1AEFF0):
    """The full level target the split operand(s) name for this relocation, or (None, why).
    R_MIPS_26: the jump operand's value. HI16/LO16: (hi << 16) + sext(lo), each
    half's immediate read from ITS OWN split operand (pair_ops = the other
    half's): a symbolic half (`%hi(D_L24_001B1AB0 + 0x27CD0)`, `(0x1A7290 >> 16)`)
    and a raw one (`lui $v1, 0x1A` / `lw $a0, -0x7CBC($v1)`) are read alike, so
    spimdisasm's choice of spelling cannot change the value. GPREL16:
    `%gp_rel(EXPR)`, or raw `-0x76EC($gp)` = _gp + sext(imm)."""
    ops = strip_ops(ops)
    if kind == R_26:
        last = ops.split(",")[-1].strip()
        v = expr_value(last, core)
        return (v, None) if v is not None else (None, "jump operand %r" % last)
    if kind in (R_HI16, R_LO16):
        if pair_ops is None:
            return None, "no HI16/LO16 pair in the split"
        hi_ops, lo_ops = (ops, strip_ops(pair_ops)) if kind == R_HI16 else (strip_ops(pair_ops), ops)
        h, eh = hi_imm(hi_ops, core)
        l, el = lo_imm(lo_ops, core)
        if h is None or l is None:
            return None, eh or el
        return ((h << 16) + l) & 0xFFFFFFFF, None
    if kind == R_GPREL16:
        m = re.search(r"%gp_rel\((.+?)\)(\(\$gp\))?$", ops)
        if m:
            v = expr_value(m.group(1), core)
            return (v, None) if v is not None else (None, "%%gp_rel operand %r" % m.group(1))
        m = re.search(r"(?:^|,)\s*(-?0x[0-9A-Fa-f]+|-?\d+)\(\$gp\)$", ops)
        if m:
            return (gp + sext16(int(m.group(1), 0) & 0xFFFF)) & 0xFFFFFFFF, None
        return None, "gp_rel operand %r" % ops
    return None, "relocation type %d" % kind


# ------------------------------------------------------------ names ----
def analyse(nn, quiet=False):
    tag, d, F = lvl_paths(nn)
    places = read_places(F["places"])
    core = read_core(F["core"])
    split, blocks = read_split(F["text"])
    E = read_elf(P(BOOT_ELF))
    glob, byaddr = boot_symbols(E)
    units = boot_units(P(BOOT_MAP))
    sect = collections.defaultdict(dict)
    for unit, fn, al in read_tsv(P(BOBJ, "sections.tsv"), 3):
        sect[unit][fn] = int(al)
    cutref = collections.defaultdict(dict)
    for unit, fn, why in read_tsv(P(BOBJ, "refused.tsv"), 3):
        cutref[unit][fn] = why
    funcs = collections.defaultdict(list)  # unit -> sectioned C functions' (boot value, size, name)
    for s_ in E["syms"]:
        if s_["type"] == 2 and s_["name"]:
            for unit in units:
                if s_["name"] in sect[unit] and units[unit][0] <= s_["value"] < units[unit][0] + units[unit][1]:
                    funcs[unit].append((s_["value"], s_["size"], s_["name"]))
    pair = {}  # boot address -> level places
    for ba, pl, sz in places:
        pair.setdefault(ba, []).append(pl)
    objs = {}

    cls = collections.Counter()
    refused = []  # (func_LNN, boot name, reason)
    cand = {}     # boot name -> dict(place, unit, size, ba)
    for ba, pl, sz in places:
        lname = "func_%s_%08X" % (tag, pl)
        unit = next((u for u, (s, n) in units.items() if s <= ba < s + n), None)
        if unit is None:
            cls["boot address outside every C unit's .text (asm segment)"] += 1
            continue
        names = byaddr.get(ba, set())
        fs = sorted(names & set(sect[unit]))
        if not fs:
            cr = sorted(names & set(cutref[unit]))
            inside = [(v, sz_, n) for v, sz_, n in funcs.get(unit, []) if v < ba < v + sz_]
            if cr:
                refused.append((lname, cr[0], "not sectioned: %s" % cutref[unit][cr[0]]))
            elif inside:
                v, sz_, n = inside[0]
                refused.append((lname, n, "catalogue boundary: boot 0x%08X (0x%X bytes) starts inside our C function %s "
                                "(0x%08X, 0x%X bytes); the level carries only part of it" % (ba, sz, n, v, sz_)))
            else:
                cls["inside a C unit, no compiled function covers it (INCLUDE_ASM)"] += 1
            continue
        if len(fs) > 1:
            refused.append((lname, "/".join(fs), "two sectioned functions at boot 0x%08X" % ba))
            continue
        fn = fs[0]
        if fn in cand:
            refused.append((lname, fn, "boot function placed twice in this level (also at 0x%08X)" % cand[fn]["place"]))
            continue
        if unit not in objs:
            objs[unit] = read_elf(P(BOBJ, unit + ".o"))
        O = objs[unit]
        si = next((i for i, s in enumerate(O["secs"]) if s["name"] == ".text." + fn), None)
        if si is None:
            refused.append((lname, fn, "no .text.%s in %s.o" % (fn, unit)))
            continue
        if O["secs"][si]["size"] != sz:
            refused.append((lname, fn, "section size 0x%X != catalogue size 0x%X" % (O["secs"][si]["size"], sz)))
            continue
        if blocks.get(lname) != [sz]:
            refused.append((lname, fn, "split block %s: nonmatching sizes %s, want [0x%X]" % (lname, blocks.get(lname), sz)))
            continue
        if ba < CORE_TOP:
            refused.append((lname, fn, "boot address 0x%08X is in the resident core (its name has a core value)" % ba))
            continue
        cand[fn] = dict(place=pl, unit=unit, size=sz, ba=ba, si=si, lname=lname)

    # relocations of each candidate: (name, value, source) or section refs
    uses = {}
    for fn, c in cand.items():
        O = objs[c["unit"]]
        si = c["si"]
        sec = O["secs"][si]
        code = O["b"][sec["off"]:sec["off"] + sec["size"]]
        rl = O["rel"].get(si, [])
        w = lambda at: struct.unpack_from("<I", code, at)[0]
        # HI16 -> its LO16: the next LO16 of the same symbol in relocation order,
        # else (an orphan HI GNU as lists out of order, FACT #13215) the next by address
        lo_of_hi, hi_of_lo = {}, {}
        for k, (at, kind, s) in enumerate(rl):
            if kind == R_HI16:
                lo_of_hi[at] = next((o for o, t, ss in rl[k + 1:] if t == R_LO16 and ss == s), None)
        for at, kind, s in rl:
            if kind == R_HI16 and lo_of_hi.get(at) is None:
                cands = sorted(o for o, t, ss in rl if t == R_LO16 and ss == s and o > at)
                lo_of_hi[at] = cands[0] if cands else None
        for at, kind, s in rl:
            if kind == R_LO16:
                his = [h for h, lo in lo_of_hi.items() if lo == at]
                if not his:
                    his = sorted(o for o, t, ss in rl if t == R_HI16 and ss == s and o < at)[-1:]
                hi_of_lo[at] = his[-1] if his else None
        u, why = [], None
        for at, kind, s in rl:
            sym = O["syms"][s]
            x = w(at)
            if kind == R_26:
                A = (x & 0x3FFFFFF) << 2
            elif kind == R_HI16:
                lo = lo_of_hi.get(at)
                if lo is None:
                    why = "HI16 at +0x%X has no LO16 pair" % at
                    break
                A = ((x & 0xFFFF) << 16) + sext16(w(lo) & 0xFFFF)
            elif kind == R_LO16:
                hi = hi_of_lo.get(at)
                if hi is None:
                    why = "LO16 at +0x%X has no HI16 pair" % at
                    break
                A = ((w(hi) & 0xFFFF) << 16) + sext16(x & 0xFFFF)
            elif kind == R_GPREL16:
                A = sext16(x & 0xFFFF)
            else:
                why = "relocation type %d at +0x%X" % (kind, at)
                break
            ins = split.get(c["place"] + at)
            if ins is None:
                why = "no split instruction at 0x%08X" % (c["place"] + at)
                break
            pair_at = lo_of_hi.get(at) if kind == R_HI16 else hi_of_lo.get(at) if kind == R_LO16 else None
            pins = split.get(c["place"] + pair_at) if pair_at is not None else None
            T, e = split_target(kind, ins[1], core, pins[1] if pins else None)
            if T is None:
                why = "split operand at 0x%08X: %s" % (c["place"] + at, e)
                break
            if sym["type"] == 3:  # section symbol
                sname = O["secs"][sym["shndx"]]["name"]
                if sname.startswith(".text.") and sname[6:] != fn:
                    u.append(("section", sname[6:], T, A, at))
                elif sname == ".text." + fn:
                    if (T - A) & 0xFFFFFFFF != c["place"]:
                        why = "self-section reloc at +0x%X: split target 0x%08X != place + 0x%X" % (at, T, A)
                        break
                else:
                    why = "relocation against section %s at +0x%X" % (sname, at)
                    break
            elif sym["bind"] == 0:
                why = "relocation against local symbol %s at +0x%X" % (sym["name"], at)
                break
            else:
                n = sym["name"]
                if n not in glob:
                    why = "%s is not defined in the boot ELF" % n
                    break
                v = (T - A) & 0xFFFFFFFF
                bv = glob[n]
                if bv < CORE_TOP:
                    src = "core"
                elif kind == R_26 and bv in pair:
                    src = "catalogue" if v in pair[bv] else "split-operand (catalogue pairs 0x%08X with %s)" % (
                        bv, ",".join("0x%08X" % p for p in pair[bv]))
                else:
                    src = "split-operand"
                u.append(("name", n, v, src, at))
        if why:
            refused.append((c["lname"], fn, why))
        else:
            uses[fn] = u

    # fixed point: one value per name; section refs to placed functions only
    placed = {fn for fn in uses}
    while True:
        vals = collections.defaultdict(lambda: collections.defaultdict(set))
        srcs = collections.defaultdict(set)
        for fn in placed:
            vals[fn][cand[fn]["place"]].add("(placed) " + fn)
            srcs[fn].add("placed")
        for fn in placed:
            for r in uses[fn]:
                if r[0] == "name":
                    _, n, v, src, at = r
                    vals[n][v].add(fn)
                    srcs[n].add(src)
                    if src == "core" and v != glob[n]:
                        vals[n][glob[n]].add("(core value)")
        drop = {}
        for n, vs in vals.items():
            if len(vs) > 1:
                desc = "; ".join("0x%08X by %s" % (v, ",".join(sorted(us))) for v, us in sorted(vs.items()))
                for v, us in vs.items():
                    for f in us:
                        if not f.startswith("("):
                            drop.setdefault(f, "name %s needs %d values in %s: %s" % (n, len(vs), tag, desc))
                if n in placed:
                    drop.setdefault(n, "its own name %s needs %d values in %s: %s" % (n, len(vs), tag, desc))
        for fn in placed:
            for r in uses[fn]:
                if r[0] == "section":
                    _, g, T, A, at = r
                    if g not in placed:
                        drop.setdefault(fn, "relocation at +0x%X against .text.%s, which is not placed" % (at, g))
                    elif (T - A) & 0xFFFFFFFF != cand[g]["place"]:
                        drop.setdefault(fn, "relocation at +0x%X against .text.%s: split target 0x%08X != its place 0x%08X + 0x%X"
                                        % (at, g, T, cand[g]["place"], A))
        if not drop:
            break
        for f, why in drop.items():
            placed.discard(f)
            refused.append((cand[f]["lname"], f, why))
    rows = sorted((cand[f]["place"], cand[f]["lname"], f, cand[f]["unit"], cand[f]["size"]) for f in placed)
    names = sorted((n, next(iter(vs)), "+".join(sorted(srcs[n]))) for n, vs in vals.items())
    info = dict(tag=tag, F=F, places=places, cls=cls, refused=sorted(refused), rows=rows, names=names,
                core=core, glob=glob)
    return info


def render(info):
    tag, F = info["tag"], info["F"]
    rel = lambda p: os.path.relpath(p, ROOT)
    hdr_inputs = "%s (sha1 %s), %s (sha1 %s)" % (rel(F["places"]), sha1f(F["places"]), rel(F["core"]), sha1f(F["core"]))
    c = ["# GENERATED by tools/levels/gen_boot_names.py names --level %s -- do not edit; re-run it." % tag[1:],
         "# Boot C functions PLACED in level program %s (RULING #13219, task #2516): each row's" % tag,
         "# .text.<boot name> section of the sectioned boot object <unit>.o (tools/levels/boot_objects.sh)",
         "# is linked at the place in its name instead of the split asm block (tools/levels/gen_ld.py).",
         "# Values of the names they reference: symbol_addrs/%s_boot_names.txt. Refusals: printed by the generator." % tag,
         "# function\tboot name\tunit\tsize (bytes)"]
    for pl, lname, fn, unit, sz in info["rows"]:
        c.append("%s\t%s\t%s\t%d" % (lname, fn, unit, sz))
    nm = ["// GENERATED by tools/levels/gen_boot_names.py names --level %s -- do not edit; re-run it." % tag[1:],
          "// Link-time values of boot names in level program %s (RULING #13219 conditions 2 and 4, task #2516)." % tag,
          "// Read by tools/levels/level_build.sh as an ld script (comments stripped); never merged into",
          "// symbol_addrs/usa or include/ (RULING #7226). Inputs: %s, the level's split asm," % hdr_inputs,
          "// and the gated boot build's symbol table and sectioned objects. No disc or ROM bytes are read.",
          "// Source per name: core (boot value, below 0x1A7C80) | placed (its own place) | catalogue (a call the",
          "// committed boot<->place table predicts) | split-operand (the committed split's operand at that offset).",
          "// %d names, %d placed functions." % (len(info["names"]), len(info["rows"]))]
    for n, v, s in info["names"]:
        nm.append("%s = 0x%08X; // %s" % (n, v, s))
    nm.append("// Reverse aliases: the level identity of each placed function, for the remaining level asm.")
    for pl, lname, fn, unit, sz in info["rows"]:
        nm.append("%s = %s; // alias" % (lname, fn))
    return "\n".join(c) + "\n", "\n".join(nm) + "\n"


def report(info):
    print("gen_boot_names %s: %d boot-copy places in %s" % (info["tag"], len(info["places"]), os.path.relpath(info["F"]["places"], ROOT)))
    for k, v in sorted(info["cls"].items()):
        print("   not ours: %-70s %d" % (k, v))
    print("   REFUSED %d (members follow; they stay asm in %s):" % (len(info["refused"]), info["tag"]))
    for l, f, why in info["refused"]:
        print("REFUSED %s %s: %s" % (l, f, why))
    src = collections.Counter()
    for n, v, s in info["names"]:
        src[s.split(" ")[0]] += 1
    print("   PLACED %d; names %d by source %s" % (len(info["rows"]), len(info["names"]), dict(sorted(src.items()))))


def cmd_names(nn, check):
    info = analyse(nn)
    report(info)
    ctext, ntext = render(info)
    F = info["F"]
    if check:
        bad = [p for p, t in ((F["c"], ctext), (F["names"], ntext))
               if not os.path.isfile(p) or open(p).read() != t]
        for p in bad:
            print("gen_boot_names: %s is STALE (re-run tools/levels/gen_boot_names.py names --level %d)" % (os.path.relpath(p, ROOT), nn))
        if not bad:
            print("gen_boot_names: %s and %s up to date" % (os.path.relpath(F["c"], ROOT), os.path.relpath(F["names"], ROOT)))
        return 1 if bad else 0
    for p, t in ((F["c"], ctext), (F["names"], ntext)):
        os.makedirs(os.path.dirname(p), exist_ok=True)
        open(p, "w").write(t)
        print("gen_boot_names: wrote %s" % os.path.relpath(p, ROOT))
    return 0


def cmd_places(nn, catalogue, bootcat):
    tag, d, F = lvl_paths(nn)
    boot = {}
    for l in open(bootcat):
        if l.startswith("#"):
            continue
        p = l.rstrip("\n").split("\t")
        boot[p[0]] = (int(p[1], 16), int(p[2]))
    rows = []
    for l in open(catalogue):
        if l.startswith("#"):
            continue
        name, kind, size, fp, nlev, places = l.rstrip("\n").split("\t")
        if kind != "boot":
            continue
        for x in places.split(","):
            lv, a = x.split(":")
            if int(lv) == nn:
                ba, bsz = boot[name]
                if bsz != int(size):
                    sys.exit("places: %s boot size %d != level size %s" % (name, bsz, size))
                rows.append((int(a, 16), ba, int(size)))
    rows.sort()
    o = ["# GENERATED by tools/levels/gen_boot_names.py places --level %d -- do not edit." % nn,
         "# Boot-copy places of level program %s: OpenRAC rac2-decomp config/us2/levels/functions.tsv" % tag,
         "# (file sha1 %s) rows of kind `boot` with a %02d: place, their boot address" % (sha1f(catalogue), nn),
         "# from config/us2/boot/functions.tsv (file sha1 %s), read as DATA: addresses and" % sha1f(bootcat),
         "# sizes only, no names copied (task #2516, RULING #13219 condition 2). %d places." % len(rows),
         "# boot_addr\tplace\tsize (bytes)"]
    for pl, ba, sz in rows:
        o.append("%08X\t%08X\t%d" % (ba, pl, sz))
    open(F["places"], "w").write("\n".join(o) + "\n")
    print("places: wrote %s (%d rows)" % (os.path.relpath(F["places"], ROOT), len(rows)))
    return 0


def read_names(path):
    vals, alias = {}, {}
    for l in open(path):
        l = l.split("//", 1)[0].strip()
        m = re.match(r"^([A-Za-z_][\w.$]*) = 0x([0-9A-Fa-f]+);$", l)
        if m:
            vals[m.group(1)] = int(m.group(2), 16)
            continue
        m = re.match(r"^([A-Za-z_][\w.$]*) = ([A-Za-z_][\w.$]*);$", l)
        if m:
            alias[m.group(1)] = m.group(2)
    return vals, alias


def cmd_bootld(nn, out):
    tag, d, F = lvl_paths(nn)
    rows = read_tsv(F["c"], 4)
    vals, alias = read_names(F["names"])
    E = read_elf(P(BOOT_ELF))
    glob, byaddr = boot_symbols(E)
    placed = {r[1] for r in rows}
    o = ["/* GENERATED by tools/levels/gen_boot_names.py bootld: %s's placed sections at their BOOT addresses. */" % tag,
         "_gp = 0x%08X; /* the boot ELF's own _gp */" % glob["_gp"]]
    for n in sorted(vals):
        if n not in placed and n != "_gp":
            o.append("%s = 0x%08X;" % (n, glob[n]))
    o.append("SECTIONS {")
    for lname, fn, unit, sz in sorted(rows, key=lambda r: glob[r[1]]):
        o.append("  .b.%s 0x%08X : { %s/%s.o(.text.%s) }" % (fn, glob[fn], BOBJ, unit, fn))
    o.append("  /DISCARD/ : { *(*) }")
    o.append("}")
    open(out, "w").write("\n".join(o) + "\n")
    print("bootld: %d sections, %d names at boot values -> %s" % (len(rows), len(vals) - len(placed), out))
    return 0


def cmd_bootcmp(nn, elf):
    tag, d, F = lvl_paths(nn)
    rows = read_tsv(F["c"], 4)
    B = read_elf(P(BOOT_ELF))
    glob, _ = boot_symbols(B)
    L = read_elf(elf)
    secs = {s["name"]: s for s in L["secs"]}
    eq, bad = 0, []
    for lname, fn, unit, sz in rows:
        s = secs.get(".b." + fn)
        sz = int(sz)
        if s is None or s["addr"] != glob[fn] or s["size"] != sz:
            bad.append("%s(section %s)" % (fn, "missing" if s is None else "0x%08X/0x%X" % (s["addr"], s["size"])))
            continue
        got = L["b"][s["off"]:s["off"] + sz]
        ref = elf_bytes(B, glob[fn], sz)
        dw = [i for i in range(0, sz, 4) if got[i:i + 4] != ref[i:i + 4]]
        if dw:
            bad.append("%s(+0x%X, %d words)" % (fn, dw[0], len(dw)))
        else:
            eq += 1
    print("bootcmp: %d of %d placed sections linked at their boot addresses == %s at those addresses" % (eq, len(rows), BOOT_ELF))
    if bad:
        print("bootcmp: DIFFERS %d: %s" % (len(bad), " ".join(bad)))
    return 0 if not bad and eq == len(rows) else 1


def map_sections(map_path):
    """input section rows `.text.<fn> ADDR SIZE OBJ` (ld wraps long names onto the next line)."""
    out = {}
    lines = open(map_path).read().split("\n")
    for i, l in enumerate(lines):
        m = re.match(r"^ (\.text\.\S+)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+)\s*$", l)
        if not m:
            m2 = re.match(r"^ (\.text\.\S+)\s*$", l)
            if m2 and i + 1 < len(lines):
                m3 = re.match(r"^\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+)\s*$", lines[i + 1])
                if m3:
                    out[m2.group(1)] = (int(m3.group(1), 16), int(m3.group(2), 16), m3.group(3))
            continue
        out[m.group(1)] = (int(m.group(2), 16), int(m.group(3), 16), m.group(4))
    return out


def cmd_placed(nn, elf, mapp, textbin, asmdir):
    tag, d, F = lvl_paths(nn)
    rows = read_tsv(F["c"], 4)
    vals, alias = read_names(F["names"])
    ms = map_sections(mapp)
    E = read_elf(elf)
    tsec = next(s for s in E["secs"] if s["name"] == ".text")
    built = E["b"][tsec["off"]:tsec["off"] + tsec["size"]]
    ext = open(textbin, "rb").read()
    base = tsec["addr"]
    glabels = collections.Counter()
    for dp, _, fs in os.walk(asmdir):
        for f in fs:
            if f.endswith(".s"):
                for l in open(os.path.join(dp, f), errors="replace"):
                    if l.startswith("glabel "):
                        glabels[l.split()[1]] += 1
    ok = fail = 0
    differs = []
    for lname, fn, unit, sz in rows:
        sz = int(sz)
        pl = int(lname.rsplit("_", 1)[1], 16)
        mrow = ms.get(".text." + fn)
        v = vals.get(fn)
        g = glabels.get(lname, 0)
        al = alias.get(lname)
        if mrow and mrow[0] == pl and mrow[1] == sz and v == pl and g == 0 and al == fn \
                and mrow[2].endswith("/%s.o" % unit):
            ok += 1
            print("PLACED ok %s %s: map .text.%s 0x%08X 0x%X %s; names map 0x%08X; alias %s = %s; glabel in assembled asm %d"
                  % (lname, fn, fn, mrow[0], mrow[1], mrow[2], v, lname, al, g))
        else:
            fail += 1
            print("PLACED FAIL %s %s: map %s (want 0x%08X 0x%X .../%s.o); names map %s; alias %s; glabel in assembled asm %d (want 0)%s"
                  % (lname, fn, "0x%08X 0x%X %s" % mrow if mrow else "none", pl, sz, unit,
                     "0x%08X" % v if v is not None else "none", al, g,
                     "; MISPLACED" if mrow and mrow[0] != pl else ""))
        a = pl - base
        dw = [i for i in range(0, sz, 4) if built[a + i:a + i + 4] != ext[a + i:a + i + 4]]
        if dw:
            differs.append((lname, fn, dw))
    for lname, fn, dw in differs:
        print("PLACED-DIFFERS %s %s (+0x%X, %d words)" % (lname, fn, dw[0], len(dw)))
    print("placed: %d PLACED ok, %d PLACED FAIL, %d placed functions whose bytes differ from %s" % (ok, fail, len(differs), textbin))
    return 0 if fail == 0 else 1


def cmd_users(nn, name):
    tag, d, F = lvl_paths(nn)
    objs = {}
    out = []
    for lname, fn, unit, sz in read_tsv(F["c"], 4):
        if unit not in objs:
            objs[unit] = read_elf(P(BOBJ, unit + ".o"))
        O = objs[unit]
        si = next(i for i, s in enumerate(O["secs"]) if s["name"] == ".text." + fn)
        if any(O["syms"][s]["name"] == name for _, _, s in O["rel"].get(si, [])):
            out.append(lname)
    print(" ".join(sorted(out)))
    return 0


def main():
    a = sys.argv[1:]
    def opt(k, default=None):
        if k in a:
            i = a.index(k)
            v = a[i + 1]
            del a[i:i + 2]
            return v
        return default
    lev = opt("--level")
    cat = opt("--catalogue")
    bcat = opt("--boot-catalogue")
    check = "--check" in a
    a = [x for x in a if x != "--check"]
    if not a or lev is None:
        sys.exit(__doc__)
    nn = int(lev)
    if a[0] == "places" and cat and bcat:
        return cmd_places(nn, cat, bcat)
    if a[0] == "names" and len(a) == 1:
        return cmd_names(nn, check)
    if a[0] == "bootld" and len(a) == 2:
        return cmd_bootld(nn, a[1])
    if a[0] == "bootcmp" and len(a) == 2:
        return cmd_bootcmp(nn, a[1])
    if a[0] == "users" and len(a) == 2:
        return cmd_users(nn, a[1])
    if a[0] == "placed" and len(a) == 5:
        return cmd_placed(nn, *a[1:])
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())

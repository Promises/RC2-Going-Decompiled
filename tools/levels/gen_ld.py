#!/usr/bin/env python3
"""Write the per-level GNU ld script that links a split level program back together.

    python3 -I tools/levels/gen_ld.py going-decompiled/levels/usa/L24/config/L24.yaml [--check]

Our own tool (task #2457, NOTE #13098 row L4). Inputs are committed files only:
the level's generated splat config (LNN.yaml), its core_externs.txt and its split
asm. Output: going-decompiled/levels/usa/LNN/linker_scripts/LNN.ld (committed).
With --check it writes nothing and exits 1 if the committed script differs.

The script it writes:
  * one output section per program section, at that section's dest, in program
    order (.lit .bss .data lvl.vtbl lvl.camvtbl lvl.sndvtbl .text). No ALIGN and
    no FILL between input and end: a section is exactly its object's bytes, so
    its size is the asm's, never padded (splat's own generated .ld pads every
    section to 16, which would change sizes such as .lit's 0x9254);
  * one PT_LOAD program header holding all seven, as the stored program is one
    absolute overlay (no relocation);
  * _gp from the config's gp_value (0x1AEFF0 in every USA level);
  * ENTRY(<the glabel whose first word is at the program's entry>): the entry
    address is read from the config header, the symbol from the asm, so the
    linked e_entry is the address that function actually lands at;
  * every core_externs.txt symbol (the boot's names below 0x1A7C80, the resident
    core the level runs against) as an ABSOLUTE assignment, read-only from the
    boot via core_externs.txt. Address-named references with no symbol_addrs
    entry (`D_XXXXXX`, `D_L24_XXXXXXXX`, ...) are NOT here: level_build.sh
    resolves those from `nm -u` at build time, from the address in the name.
  * /DISCARD/ for everything else (.reginfo, .MIPS.abiflags, .pdr, ...).

Splat's generated linker script went to this same path at L3; it is replaced:
split_level.py now points splat's copy into the gitignored build dir.

C functions (task #2495, NOTE #13098 row L6). When the level has a
config/LNN_c.tsv (`function source arm gflag [as cc1flags]`, one row per
function built from C), each listed function's block is cut out of the split
.text asm and its C object (`<build>/obj/c/<fn>.o(.text)`) linked at its place.
The two optional columns (task #2552, RULING #13291 section 1) come together or
not at all; a 4-column row means `as -`, the route every row had before them:
  * as:       `as` (plain GNU as after tools/ee/move_fixup.sed) or `asm_unit`
              (the boot's tools/ee/asm_unit.sh, our model of SN's assembler);
              any other value is refused;
  * cc1flags: extra cc1 flags, comma-separated with no spaces, or `-` for none.
              Each must look like an option (`-` then [A-Za-z0-9_=+.-]). Refused:
              any -G flag (the gflag column owns it), -O2 and, for s136,
              -fopt-stack (the arm's own flags, which level_build.sh passes
              already), and a flag given twice.
The ld script's comment names the route only for a row that is not `as -`,
so a level whose rows are all `as -` gets byte-identical output.

Boot C placed in the level (task #2516, RULING #13219, multi-placement). When
the level has a config/LNN_boot_c.tsv (`function boot-name unit size`, written
by tools/levels/gen_boot_names.py names), each listed function's block is cut
out the same way and the boot's own sectioned object is linked at its place:
`going-decompiled/build/usa_levels/boot_objs/<unit>.o(.text.<boot name>)`
(tools/levels/boot_objects.sh; one object per boot unit, shared by every level).

The cut. A block runs from the `.align` before its `nonmatching` line through
its `endlabel`. A cut refuses (exit 1, naming the cause) unless:
  * the function has exactly one glabel and one endlabel, its `nonmatching`
    size equals 4 x its instruction lines, and those lines' addresses run
    contiguously from the address in its name;
  * no `.L` label defined inside the block is referenced outside it.
The pieces between the cuts go into ONE assembler file,
<build>/cut/<path under asm>.cut.s: the file's own preamble (through its
`.section` line), then piece k under `.section .text.cut<kkkk>`. The .text
output section lists, in address order:
    <obj>/text/X.cut.o(.text.cut0000) <C or boot object>(...) <obj>/text/X.cut.o(.text.cut0001) ...
One `as` call per cut file (1,000 per-piece calls would cost minutes). In the
pieces every `.align N` line is DROPPED and every piece section is given
alignment 4 (level_build.sh): a piece often starts right after a cut block, at
an address that is 4 mod 8 (the padding `nop` spimdisasm writes after an
`endlabel`), where an `.align 3`, measured from the SECTION start, would insert
4 bytes the program does not have. Dropping them is byte-neutral only because
each one is already satisfied by the address of the next instruction, which
--cut CHECKS for every dropped line (exit 1 naming the line otherwise).
`--cut` writes the cut files and prints one `CUT <asm file> <pieces> <cut file>`
line per cut file, one `CFN <function> <addr> <size> <source> <arm> <gflag> <as>
<cc1flags>` line per C row (`as -` for a 4-column row) and one `BFN <function> <addr> <size> <boot name> <unit>` line
per boot row, for level_build.sh.
"""
import os
import re
import sys

# segment name -> (ELF output section name, archive record order is the yaml order)
OUT_NAME = {"lit": ".lit", "bss": ".bss", "data": ".data", "text": ".text"}


def parse_yaml(path):
    """The generated LNN.yaml has a fixed shape (tools/levels/gen_yaml.py); parse it
    without a YAML library so this runs under a bare `python3 -I` in ee-build."""
    t = open(path).read()
    opt = lambda k: re.search(r"^  %s: (\S+)" % k, t, re.M).group(1)
    m = re.search(r"entry 0x([0-9A-Fa-f]{8}), program sha1 ([0-9a-f]{40})", t)
    if not m:
        sys.exit("gen_ld: no 'entry 0x..., program sha1 ...' header in %s" % path)
    entry, prog_sha1 = int(m.group(1), 16), m.group(2)
    segs = []
    for blk in re.split(r"^  - name: ", t, flags=re.M)[1:]:
        name = blk.split("\n", 1)[0].strip()
        vram = int(re.search(r"^    vram: 0x([0-9A-Fa-f]+)", blk, re.M).group(1), 16)
        sub = re.search(r"^      - \[0x[0-9A-Fa-f]+, (\w+), ([\w./]+)\]", blk, re.M)
        if sub:
            kind, sname = sub.group(1), sub.group(2)
        else:
            sub = re.search(r"^      - \{ type: (\w+), vram: 0x[0-9A-Fa-f]+, name: ([\w./]+) \}", blk, re.M)
            kind, sname = sub.group(1), sub.group(2)
        segs.append((name, vram, kind, sname))
    return {"basename": opt("basename"), "gp": int(opt("gp_value"), 16),
            "asm_path": opt("asm_path"), "build_path": opt("build_path"),
            "entry": entry, "prog_sha1": prog_sha1, "segs": segs}


def asm_file(cfg, kind, sname):
    # splat 0.41's layout: asm subsegments at <asm>/<name>.s, data/bss under <asm>/data/.
    rel = {"asm": "%s.s", "data": "data/%s.data.s", "bss": "data/%s.bss.s"}[kind] % sname
    return os.path.join(cfg["asm_path"], rel)


def obj_file(cfg, s):
    # level_build.sh's rule: <build>/obj/<path under asm_path, .s -> .o>
    return os.path.join(cfg["build_path"], "obj", os.path.relpath(s, cfg["asm_path"])[:-2] + ".o")


def entry_symbol(path, entry):
    want = "%08X" % entry
    cur = None
    for line in open(path):
        m = re.match(r"glabel (\S+)", line)
        if m:
            cur = m.group(1)
            continue
        m = re.match(r"\s*/\* [0-9A-F]+ ([0-9A-F]{8}) ", line)
        if m and cur:
            if m.group(1) == want:
                return cur
            cur = None
    sys.exit("gen_ld: no glabel starts at the entry 0x%s in %s" % (want, path))


def core_externs(path):
    out = []
    for line in open(path):
        m = re.match(r"\s*([A-Za-z_][\w.$]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;", line)
        if m:
            out.append((m.group(1), int(m.group(2), 16)))
    return out


C_ARMS = ("sdk29", "s136", "sn2952")
C_GFLAGS = ("-G0", "-G8")
C_ASSEMBLERS = ("as", "asm_unit")
# the flags level_build.sh passes for every row (-O2 <gflag>) and per arm
C_ARM_FLAGS = {"sdk29": ("-O2",), "s136": ("-O2", "-fopt-stack"), "sn2952": ("-O2",)}
C_FLAG = re.compile(r"^-[A-Za-z0-9_=+.-]+$")
FN_ADDR = re.compile(r"^func_L\d\d_([0-9A-F]{8})$")
INSN = re.compile(r"^\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/")


def c_flags_error(arm, flags):
    """Why a cc1flags field is refused, or None. `-` is none."""
    if flags == "-":
        return None
    fl = flags.split(",")
    for x in fl:
        if not C_FLAG.match(x):
            return "%r is not an option (- then [A-Za-z0-9_=+.-], comma-separated, no spaces)" % x
        if x.startswith("-G"):
            return "%s: -G belongs to the gflag column" % x
        if x in C_ARM_FLAGS[arm]:
            return "%s repeats a flag level_build.sh already passes for arm %s (%s)" % (x, arm, " ".join(C_ARM_FLAGS[arm]))
    if len(set(fl)) != len(fl):
        return "a flag is given twice"
    return None


def c_rows(level_dir, tag):
    """config/LNN_c.tsv: (function, source, arm, gflag, as, cc1flags) rows; [] when
    absent. A 4-column row reads as `as -` (the route before task #2552)."""
    path = os.path.join(level_dir, "config", "%s_c.tsv" % tag)
    rows = []
    if not os.path.isfile(path):
        return rows
    for n, line in enumerate(open(path), 1):
        if not line.strip() or line.startswith("#"):
            continue
        f = line.rstrip("\n").split("\t")
        if len(f) == 4:
            f = f + ["as", "-"]
        if len(f) != 6:
            sys.exit("gen_ld: %s:%d: want 4 or 6 tab-separated fields, got %d" % (path, n, len(f)))
        fn, src, arm, g, asm, fl = f
        if not FN_ADDR.match(fn) or fn[5:8] != tag:
            sys.exit("gen_ld: %s:%d: %s is not a func_%s_XXXXXXXX name" % (path, n, fn, tag))
        if arm not in C_ARMS or g not in C_GFLAGS:
            sys.exit("gen_ld: %s:%d: arm %s / gflag %s not in %s / %s" % (path, n, arm, g, C_ARMS, C_GFLAGS))
        if asm not in C_ASSEMBLERS:
            sys.exit("gen_ld: %s:%d: assembler %r not in %s" % (path, n, asm, C_ASSEMBLERS))
        why = c_flags_error(arm, fl)
        if why:
            sys.exit("gen_ld: %s:%d: cc1flags %r refused: %s" % (path, n, fl, why))
        if not os.path.isfile(os.path.join(level_dir, src)):
            sys.exit("gen_ld: %s:%d: no source %s" % (path, n, os.path.join(level_dir, src)))
        rows.append((fn, src, arm, g, asm, fl))
    names = [r[0] for r in rows]
    if len(set(names)) != len(names):
        sys.exit("gen_ld: %s: a function is listed twice" % path)
    return rows


BOOT_OBJ_DIR = "going-decompiled/build/usa_levels/boot_objs"


def boot_rows(level_dir, tag):
    """config/LNN_boot_c.tsv: (function, boot name, unit, size) rows; [] when absent."""
    path = os.path.join(level_dir, "config", "%s_boot_c.tsv" % tag)
    rows = []
    if not os.path.isfile(path):
        return rows
    for n, line in enumerate(open(path), 1):
        if not line.strip() or line.startswith("#"):
            continue
        f = line.rstrip("\n").split("\t")
        if len(f) != 4:
            sys.exit("gen_ld: %s:%d: want 4 tab-separated fields, got %d" % (path, n, len(f)))
        fn, boot, unit, size = f
        if not FN_ADDR.match(fn) or fn[5:8] != tag:
            sys.exit("gen_ld: %s:%d: %s is not a func_%s_XXXXXXXX name" % (path, n, fn, tag))
        if not re.match(r"^[A-Za-z_][\w.$]*$", boot) or not re.match(r"^[0-9A-F]+$", unit) or not size.isdigit():
            sys.exit("gen_ld: %s:%d: bad boot name / unit / size %r %r %r" % (path, n, boot, unit, size))
        rows.append((fn, boot, unit, int(size)))
    names = [r[0] for r in rows]
    if len(set(names)) != len(names):
        sys.exit("gen_ld: %s: a function is listed twice" % path)
    return rows


def boot_obj(unit):
    return os.path.join(BOOT_OBJ_DIR, unit + ".o")


def cut_text(path, fns):
    """Cut each function's block out of one split .text asm file.
    Returns (preamble lines, [piece body lines...], {fn: (addr, size)})."""
    lines = open(path).read().splitlines(keepends=True)
    sec = next((i for i, l in enumerate(lines) if l.startswith(".section")), None)
    if sec is None:
        sys.exit("gen_ld: cut %s: no .section line (no preamble)" % path)
    pre = lines[:sec + 1]
    glab = {}
    endl = {}
    want = set(fns)
    for i, l in enumerate(lines):
        if l.startswith("glabel ") and l.split()[1] in want:
            glab.setdefault(l.split()[1], []).append(i)
        elif l.startswith("endlabel ") and l.split()[1] in want:
            endl.setdefault(l.split()[1], []).append(i)
    blocks = []
    info = {}
    for fn in fns:
        g, e = glab.get(fn, []), endl.get(fn, [])
        if len(g) != 1 or len(e) != 1 or e[0] < g[0]:
            sys.exit("gen_ld: cut %s: %s has %d glabel / %d endlabel lines (want 1 / 1, in order)" % (path, fn, len(g), len(e)))
        g, e = g[0], e[0]
        j = g - 1
        while j > sec and not lines[j].strip():
            j -= 1
        m = re.match(r"^nonmatching %s, 0x([0-9A-Fa-f]+)\s*$" % re.escape(fn), lines[j])
        if not m:
            sys.exit("gen_ld: cut %s: no `nonmatching %s, 0x...` line before its glabel" % (path, fn))
        size = int(m.group(1), 16)
        j -= 1
        # blank lines and spimdisasm's comment-only lines (`/* Handwritten function */`)
        while j > sec and (not lines[j].strip() or re.match(r"^/\*.*\*/\s*$", lines[j])):
            j -= 1
        if not re.match(r"^\.align \d+\s*$", lines[j]):
            sys.exit("gen_ld: cut %s: no `.align` line before `nonmatching %s`" % (path, fn))
        a = j
        addr = int(FN_ADDR.match(fn).group(1), 16)
        words = [int(INSN.match(l).group(1), 16) for l in lines[g:e] if INSN.match(l)]
        if words != [addr + 4 * k for k in range(len(words))] or 4 * len(words) != size:
            sys.exit("gen_ld: cut %s: %s's %d instruction lines do not run 0x%08X.. for its nonmatching size 0x%X"
                     % (path, fn, len(words), addr, size))
        if any(l.startswith("glabel ") for l in lines[g + 1:e]):
            sys.exit("gen_ld: cut %s: another glabel inside %s's block" % (path, fn))
        blocks.append((a, e, fn))
        info[fn] = (addr, size)
    blocks.sort()
    for (a1, e1, f1), (a2, e2, f2) in zip(blocks, blocks[1:]):
        if a2 <= e1:
            sys.exit("gen_ld: cut %s: blocks of %s and %s overlap" % (path, f1, f2))
    # a .L label defined inside a cut block must not be referenced outside it
    inside = set()
    for a, e, _ in blocks:
        for l in lines[a:e + 1]:
            m = re.match(r"^\s*(\.L\w+):", l)
            if m:
                inside.add(m.group(1))
    cut_idx = set()
    for a, e, _ in blocks:
        cut_idx.update(range(a, e + 1))
    if inside:
        for i, l in enumerate(lines):
            if i not in cut_idx and ".L" in l:
                hit = inside.intersection(re.findall(r"\.L\w+", l))
                if hit:
                    sys.exit("gen_ld: cut %s: line %d references %s, defined inside a cut block" % (path, i + 1, sorted(hit)))
    pieces = []
    start = sec + 1
    for a, e, _ in blocks:
        pieces.append(lines[start:a])
        start = e + 1
    pieces.append(lines[start:])
    return pre, pieces, info


def cut_src(cfg, s):
    return os.path.join(cfg["build_path"], "cut", os.path.relpath(s, cfg["asm_path"])[:-2] + ".cut.s")


def cut_obj(cfg, s):
    return obj_file(cfg, s)[:-2] + ".cut.o"


ALIGN = re.compile(r"^\.align (\d+)\s*$")


def strip_aligns(path, body):
    """Drop every `.align N` line of a piece, checking the next instruction's address is
    already 2^N-aligned (so the line inserted 0 bytes in the whole-file assembly)."""
    nxt = [None] * len(body)  # address of the next instruction at or after each line
    a = None
    for i in range(len(body) - 1, -1, -1):
        m = INSN.match(body[i])
        if m:
            a = int(m.group(1), 16)
        nxt[i] = a
    out = []
    for i, l in enumerate(body):
        m = ALIGN.match(l)
        if not m:
            out.append(l)
            continue
        if nxt[i] is not None and nxt[i] % (1 << int(m.group(1))):
            sys.exit("gen_ld: cut %s: `%s` before 0x%08X is not already satisfied; dropping it would move code"
                     % (path, l.strip(), nxt[i]))
    return out


def c_obj(cfg, fn):
    return os.path.join(cfg["build_path"], "obj", "c", fn + ".o")


def render(cfg_path, write_cut=False):
    cfg = parse_yaml(cfg_path)
    tag = cfg["basename"]
    level_dir = os.path.dirname(os.path.dirname(cfg_path))
    asm_root = cfg["asm_path"]
    crows = c_rows(level_dir, tag)
    brows = boot_rows(level_dir, tag)
    dup = set(r[0] for r in crows) & set(r[0] for r in brows)
    if dup:
        sys.exit("gen_ld: %s in both %s_c.tsv and %s_boot_c.tsv" % (sorted(dup), tag, tag))
    cut_lines = []
    listed = []
    secs = []
    for name, vram, kind, sname in cfg["segs"]:
        s = asm_file(cfg, kind, sname)
        if not os.path.isfile(s):
            sys.exit("gen_ld: %s segment %s: no asm file %s" % (tag, name, s))
        listed.append(os.path.normpath(s))
        insec = {"asm": ".text", "data": ".data", "bss": ".bss"}[kind]
        objs = ["%s(%s)" % (obj_file(cfg, s), insec)]
        if kind == "asm" and (crows or brows):
            pre, pieces, info = cut_text(s, [r[0] for r in crows] + [r[0] for r in brows])
            cobj = {r[0]: "%s(.text)" % c_obj(cfg, r[0]) for r in crows}
            cobj.update({r[0]: "%s(.text.%s)" % (boot_obj(r[2]), r[1]) for r in brows})
            order = sorted(cobj, key=lambda f: info[f][0])
            co = cut_obj(cfg, s)
            objs = ["%s(.text.cut%04d)" % (co, 0)]
            for k, fn in enumerate(order, 1):
                objs += [cobj[fn], "%s(.text.cut%04d)" % (co, k)]
            cut_lines.append("CUT %s %d %s" % (s, len(pieces), cut_src(cfg, s)))
            for r in sorted(crows, key=lambda r: info[r[0]][0]):
                cut_lines.append("CFN %s 0x%08X 0x%X %s %s %s %s %s" % (r[0], info[r[0]][0], info[r[0]][1],
                                                                       os.path.join(level_dir, r[1]), r[2], r[3], r[4], r[5]))
            for r in sorted(brows, key=lambda r: info[r[0]][0]):
                if info[r[0]][1] != r[3]:
                    sys.exit("gen_ld: %s: split block size 0x%X != %s_boot_c.tsv size 0x%X" % (r[0], info[r[0]][1], tag, r[3]))
                cut_lines.append("BFN %s 0x%08X 0x%X %s %s" % (r[0], info[r[0]][0], info[r[0]][1], r[1], r[2]))
            if write_cut:
                body = []
                for k, piece in enumerate(pieces):
                    body.append('.section .text.cut%04d, "ax"\n' % k)
                    body += strip_aligns(s, piece)
                p = cut_src(cfg, s)
                os.makedirs(os.path.dirname(p), exist_ok=True)
                open(p, "w").write("".join(pre + body))
        secs.append((OUT_NAME.get(name, name), vram, objs))
    if (crows or brows) and not cut_lines:
        sys.exit("gen_ld: %s has C or boot rows but no asm (.text) segment to cut them from" % tag)
    # every .s in the level's asm tree must be linked (an assembled-but-unlinked
    # file is how the boot grew orphan objects; FACT #8617)
    on_disk = sorted(os.path.normpath(os.path.join(d, f)) for d, _, fs in os.walk(asm_root)
                     for f in fs if f.endswith(".s"))
    stray = sorted(set(on_disk) - set(listed))
    if stray:
        sys.exit("gen_ld: asm not named by any segment: %s" % " ".join(stray))
    text_s = [asm_file(cfg, k, n) for _, _, k, n in cfg["segs"] if k == "asm"]
    esym = entry_symbol(text_s[0], cfg["entry"])
    core = core_externs(os.path.join(level_dir, "symbol_addrs", "core_externs.txt"))
    bad = [(n, a) for n, a in core if a >= cfg["segs"][0][1]]
    if bad:
        sys.exit("gen_ld: core extern at or above the program base: %r" % bad[:5])

    L = []
    w = L.append
    w("/* GENERATED by tools/levels/gen_ld.py %s -- do not edit; re-run it. */" % cfg_path)
    w("/* USA v2.00 level program %s: 7 sections at their dests, one PT_LOAD, entry 0x%08X," % (tag[1:], cfg["entry"]))
    w("   program sha1 %s (archive form). Built by tools/levels/level_build.sh. */" % cfg["prog_sha1"])
    if crows:
        w("/* From C (config/%s_c.tsv), cut out of the .text asm and linked at their place:" % tag)
        for l in cut_lines:
            if l.startswith("CFN "):
                f = l.split()
                route = "" if (f[7], f[8]) == ("as", "-") else ", assembler %s, extra cc1 flags %s" % (f[7], f[8])
                w("   %s at %s, 0x%s bytes, %s, %s %s%s" % (f[1], f[2], f[3][2:], os.path.relpath(f[4], level_dir), f[5], f[6], route))
        w("*/")
    if brows:
        w("/* Boot C placed (config/%s_boot_c.tsv, RULING #13219): %d functions, each the .text.<boot name>" % (tag, len(brows)))
        w("   section of %s/<unit>.o, cut out of the .text asm and linked at its place;" % BOOT_OBJ_DIR)
        w("   their names' values: symbol_addrs/%s_boot_names.txt (level_build.sh passes it to ld). */" % tag)
    w("OUTPUT_ARCH(mips)")
    w("ENTRY(%s)" % esym)
    w("PHDRS")
    w("{")
    w("    program PT_LOAD;")
    w("}")
    w("SECTIONS")
    w("{")
    w("    _gp = 0x%08X;" % cfg["gp"])
    for out, vram, objs in secs:
        if len(objs) == 1:
            w("    %s 0x%08X : { %s } :program" % (out, vram, objs[0]))
        else:
            w("    %s 0x%08X : {" % (out, vram))
            for o in objs:
                w("        %s" % o)
            w("    } :program")
    w("    /DISCARD/ : { *(*) }")
    w("}")
    w("")
    w("/* The resident core below 0x%08X: the boot's names, as absolute values" % cfg["segs"][0][1])
    w("   (symbol_addrs/core_externs.txt, %d entries). */" % len(core))
    for n, a in core:
        w("%s = ABSOLUTE(0x%08X);" % (n, a))
    return os.path.join(level_dir, "linker_scripts", "%s.ld" % tag), "\n".join(L) + "\n", cut_lines


def main():
    args = [a for a in sys.argv[1:] if a not in ("--check", "--cut")]
    if len(args) != 1:
        sys.exit("usage: gen_ld.py <LNN.yaml> [--check | --cut]")
    if "--cut" in sys.argv:
        _, _, cut_lines = render(args[0], write_cut=True)
        for l in cut_lines:
            print(l)
        return 0
    out, text, _ = render(args[0])
    if "--check" in sys.argv:
        cur = open(out).read() if os.path.isfile(out) else None
        if cur != text:
            print("gen_ld: %s is STALE (re-run tools/levels/gen_ld.py %s)" % (out, args[0]))
            return 1
        print("gen_ld: %s up to date" % out)
        return 0
    os.makedirs(os.path.dirname(out), exist_ok=True)
    open(out, "w").write(text)
    print("gen_ld: wrote %s" % out)
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Generate the splat inputs for one USA v2.00 level program.

Our own tool (task #2456, claudiverse NOTE #13098 row L3). It reads only what
tools/levels/extract_levels.py wrote for the level and our boot symbol_addrs.

    gen_yaml.py --level 24 [--levels-dir extracted/usa_levels]
                [--boot-symaddrs going-decompiled/symbol_addrs/usa/symbol_addrs.txt]
                [--places <file of hex addresses> | --catalogue <functions.tsv>]

It writes:

  extracted/usa_levels/NN_<name>/LNN.split.bin   (gitignored, untrusted data)
      The flat image splat reads: the 7 sections' stored bytes concatenated in
      record order with no gaps, i.e. the archive-form program with its 16-byte
      record headers removed. The .bss bytes are the archive's own (all zero,
      checked). Section k's file offset is the sum of the sizes before it.

  going-decompiled/levels/usa/LNN/config/LNN.yaml
      One top-level splat segment per section at its own dest: `.text` as asm,
      `.bss` as a bss subsegment (NOBITS, as the boot's level-slot .bss is), the
      other five as data. $gp 0x1AEFF0. Split it with tools/levels/split_level.py,
      which gives every auto-generated name inside the program the level:
      `func_LNN_XXXXXXXX`, `D_LNN_XXXXXXXX`.

  going-decompiled/levels/usa/LNN/symbol_addrs/core_externs.txt
      Every boot symbol_addrs entry below 0x1A7C80 (the resident core, shared by
      every program), re-emitted as `name = 0xADDR; // type:.. size:..`. Only
      the `type:` and `size:` attributes are carried; the boot's prose comments
      are dropped. Nothing at or above 0x1A7C80 is taken: the level overwrites
      that range, so a boot name there would be wrong for the level (RULING
      #7226: never merge level and boot symbols).

  going-decompiled/levels/usa/LNN/symbol_addrs/LNN_functions.txt
      With --places or --catalogue: one `func_LNN_XXXXXXXX = 0xXXXXXXXX; // type:func`
      per function start (supplied as DATA; no names are copied). --places is a
      file of hex addresses; --catalogue is OpenRAC rac2-decomp's
      config/us2/levels/functions.tsv (`name kind size fingerprint nlevels
      places`), of which ONLY the `NN:address` places of this level are read.
      Without either the file holds only its header, and function starts are
      splat's own. splat may still add starts of its own (e.g. at linker debris
      between catalogued functions); they get the same level-prefixed names.

Everything it writes is deterministic from its inputs, so re-running it and
then splat reproduces the committed tree byte for byte.
"""
import argparse
import hashlib
import json
import os
import re
import sys

LIT_BASE = 0x1A7C80
GP_VALUE = 0x1AEFF0
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# section name in the manifest -> (segment name, splat subsegment type)
KIND = {
    ".lit": ("lit", "data"),
    ".bss": ("bss", "bss"),
    ".data": ("data", "data"),
    "lvl.vtbl": ("lvl.vtbl", "data"),
    "lvl.camvtbl": ("lvl.camvtbl", "data"),
    "lvl.sndvtbl": ("lvl.sndvtbl", "data"),
    ".text": ("text", "asm"),
}

SYM_RE = re.compile(r"\s*([A-Za-z_.$][\w.$]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;(.*)$")
ATTR_RE = re.compile(r"\b(type|size):(\S+)")


def die(msg):
    print("gen_yaml: ERROR: " + msg, file=sys.stderr)
    sys.exit(1)


def find_level_dir(levels_dir, level):
    hits = []
    for dn in sorted(os.listdir(levels_dir)):
        m = os.path.join(levels_dir, dn, "manifest.json")
        if os.path.isfile(m) and json.load(open(m))["level"] == level:
            hits.append(os.path.join(levels_dir, dn))
    if len(hits) != 1:
        die("level %d: expected exactly one extracted directory under %s, found %r" % (level, levels_dir, hits))
    return hits[0]


def rel(p):
    return os.path.relpath(p, ROOT)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--level", type=int, required=True)
    ap.add_argument("--levels-dir", default=os.path.join(ROOT, "extracted", "usa_levels"))
    ap.add_argument("--boot-symaddrs", default=os.path.join(ROOT, "going-decompiled", "symbol_addrs", "usa", "symbol_addrs.txt"))
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--places", help="file of hex function-start addresses (one per line, '#' comments)")
    g.add_argument("--catalogue", help="OpenRAC us2 levels functions.tsv; only this level's places are read")
    a = ap.parse_args()

    tag = "L%02d" % a.level
    ldir = find_level_dir(a.levels_dir, a.level)
    man = json.load(open(os.path.join(ldir, "manifest.json")))
    secs = man["sections"]
    if [s["name"] for s in secs] != list(KIND):
        die("unexpected section list %r" % [s["name"] for s in secs])
    if int(secs[0]["dest"], 16) != LIT_BASE:
        die(".lit not at 0x%X" % LIT_BASE)

    # flat image: sections back to back, in record order
    flat = bytearray()
    layout = []  # (name, file offset, vram, size)
    for s in secs:
        b = open(os.path.join(ldir, s["file"]), "rb").read()
        if len(b) != s["size"] or hashlib.sha1(b).hexdigest() != s["sha1"]:
            die("%s: size/sha1 differ from the manifest" % s["file"])
        if s["name"] == ".bss" and any(b):
            die(".bss has nonzero bytes; a NOBITS split would drop them")
        layout.append((s["name"], len(flat), int(s["dest"], 16), len(b)))
        flat += b
    flat_path = os.path.join(ldir, "%s.split.bin" % tag)
    open(flat_path, "wb").write(flat)
    flat_sha1 = hashlib.sha1(flat).hexdigest()
    # sections must not overlap in vram (they are placed independently)
    by_vram = sorted(layout, key=lambda t: t[2])
    for x, y in zip(by_vram, by_vram[1:]):
        if x[2] + x[3] > y[2]:
            die("sections %s and %s overlap in vram" % (x[0], y[0]))

    out = os.path.join(ROOT, "going-decompiled", "levels", "usa", tag)
    os.makedirs(os.path.join(out, "config"), exist_ok=True)
    os.makedirs(os.path.join(out, "symbol_addrs"), exist_ok=True)
    build = "going-decompiled/build/usa_levels/%s" % tag

    # core externs from the boot, below the level slot only
    core = []
    for line in open(a.boot_symaddrs):
        m = SYM_RE.match(line)
        if not m:
            continue
        addr = int(m.group(2), 16)
        if addr >= LIT_BASE:
            continue
        comment = m.group(3).split("//", 1)[1] if "//" in m.group(3) else ""
        attrs = " ".join("%s:%s" % kv for kv in ATTR_RE.findall(comment))
        core.append("%s = 0x%08X;%s" % (m.group(1), addr, (" // " + attrs) if attrs else ""))
    core_path = os.path.join(out, "symbol_addrs", "core_externs.txt")
    with open(core_path, "w") as f:
        f.write("// GENERATED by tools/levels/gen_yaml.py --level %d -- do not edit.\n" % a.level)
        f.write("// The boot's symbol_addrs entries below 0x%X (the resident core every level\n" % LIT_BASE)
        f.write("// program links against), read-only. Only type:/size: are carried.\n")
        f.write("// Source: %s (sha1 %s)\n" % (rel(a.boot_symaddrs), hashlib.sha1(open(a.boot_symaddrs, "rb").read()).hexdigest()))
        for l in core:
            f.write(l + "\n")

    fn_path = os.path.join(out, "symbol_addrs", "%s_functions.txt" % tag)
    text = layout[-1]
    places = []
    if a.places:
        for line in open(a.places):
            line = line.split("#", 1)[0].strip()
            if line:
                places.append(int(line, 16))
    if a.catalogue:
        for line in open(a.catalogue):
            if line.startswith("#") or not line.strip():
                continue
            for pl in line.rstrip("\n").split("\t")[5].split(","):
                lv, addr = pl.split(":")
                if int(lv) == a.level:
                    places.append(int(addr, 16))
    if places:
        places = sorted(set(places))
        bad = [p for p in places if not (text[2] <= p < text[2] + text[3]) or p % 4]
        if bad:
            die("%d places outside .text or misaligned: %s" % (len(bad), " ".join("%08X" % p for p in bad[:20])))
    with open(fn_path, "w") as f:
        f.write("// GENERATED by tools/levels/gen_yaml.py --level %d -- do not edit.\n" % a.level)
        src = a.places or a.catalogue
        if src:
            f.write("// Function starts in %s .text, supplied as DATA (addresses only, no names copied).\n" % tag)
            sha = hashlib.sha1(open(src, "rb").read()).hexdigest()
            if a.catalogue:
                f.write("// Source: the level-%d places of OpenRAC rac2-decomp config/us2/levels/functions.tsv\n" % a.level)
                f.write("// (file sha1 %s), read as data: %d addresses.\n" % (sha, len(places)))
            else:
                f.write("// Source: an address list (file sha1 %s): %d addresses.\n" % (sha, len(places)))
        else:
            f.write("// No function starts supplied: splat/spimdisasm's own analysis decides them.\n")
        for p in places:
            f.write("func_%s_%08X = 0x%08X; // type:func\n" % (tag, p, p))

    y = []
    w = y.append
    w("# GENERATED by tools/levels/gen_yaml.py --level %d -- do not edit; re-run it." % a.level)
    w("# USA v2.00 level program %d (%s), entry %s, program sha1 %s." % (a.level, man["wad"], man["entry"], man["program_sha1"]))
    w("# The target is the program's 7 stored sections back to back (record headers")
    w("# removed); each section is its own segment at its own dest. See tools/levels/README.md.")
    w("sha1: %s" % flat_sha1)
    w("options:")
    w("  basename: %s" % tag)
    w("  target_path: %s" % rel(flat_path))
    w("  base_path: ../../../../..")
    w("  platform: ps2")
    w("  compiler: EEGCC")
    w("")
    w("  gp_value: 0x%08X" % GP_VALUE)
    w("")
    w("  asm_path: going-decompiled/levels/usa/%s/asm" % tag)
    w("  src_path: going-decompiled/levels/usa/%s/src" % tag)
    w("  build_path: %s" % build)
    w("  asset_path: %s/assets" % build)
    w("  cache_path: %s/.splache" % build)
    w("  generated_asm_macros_directory: %s/include" % build)
    w("  undefined_funcs_auto_path: %s/undefined_funcs_auto.txt" % build)
    w("  undefined_syms_auto_path: %s/undefined_syms_auto.txt" % build)
    # splat's own .ld goes to the gitignored build dir: the committed linker_scripts/LNN.ld
    # is tools/levels/gen_ld.py's (task #2457), and a re-split must not overwrite it.
    w("  ld_script_path: %s/splat_%s.ld" % (build, tag))
    w("  ld_bss_contains_common: True")
    w("  find_file_boundaries: False")
    w("  o_as_suffix: True")
    w("")
    w("  # splat's default symbol_name_format(_no_rom), as the boot's: references below")
    w("  # 0x%X keep the boot's names. Auto names INSIDE the program get %s_ from" % (LIT_BASE, tag))
    w("  # tools/levels/split_level.py, which is how this config must be split.")
    w("  symbol_addrs_path:")
    w("    - going-decompiled/levels/usa/%s/symbol_addrs/core_externs.txt" % tag)
    w("    - going-decompiled/levels/usa/%s/symbol_addrs/%s_functions.txt" % (tag, tag))
    w("")
    w("  string_encoding: ASCII")
    w("  data_string_encoding: ASCII")
    w("  rodata_string_guesser_level: 2")
    w("  data_string_guesser_level: 2")
    w("")
    w("segments:")
    for name, off, vram, size in layout:
        seg, kind = KIND[name]
        w("  - name: %s" % seg)
        w("    type: code")
        w("    start: 0x%06X" % off)
        w("    vram: 0x%08X" % vram)
        w("    bss_size: 0x0")
        w("    subalign: null")
        w("    subsegments:")
        if kind == "bss":
            # rom span == bss size, so splat infers the subsegment's size exactly
            w("      - { type: bss, vram: 0x%08X, name: %s/%08X } # %s 0x%X..0x%X" % (vram, seg, vram, name, vram, vram + size))
        else:
            w("      - [0x%06X, %s, %s/%08X] # %s 0x%X..0x%X" % (off, kind, seg, vram, name, vram, vram + size))
    w("  - [0x%06X]" % len(flat))
    yaml_path = os.path.join(out, "config", "%s.yaml" % tag)
    open(yaml_path, "w").write("\n".join(y) + "\n")
    print("wrote %s (flat sha1 %s, %d bytes)" % (rel(yaml_path), flat_sha1, len(flat)))
    print("wrote %s (%d entries)" % (rel(core_path), len(core)))
    print("wrote %s (%d function starts)" % (rel(fn_path), len(places)))
    print("wrote %s" % rel(flat_path))


if __name__ == "__main__":
    main()

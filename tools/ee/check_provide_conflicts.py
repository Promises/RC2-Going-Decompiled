#!/usr/bin/env python3
"""check_provide_conflicts.py — cross-file PROVIDE conflict gate for the C-only link.

WHY THIS EXISTS
---------------
The C-only overlay link feeds GNU ld several `-T` scripts, each full of
`PROVIDE(name = value);` lines, under `--allow-multiple-definition`.  ld 2.40
resolves a PROVIDEd symbol from the FIRST script that supplies it, silently.
So a second PROVIDE on a symbol that a later script already binds correctly can
only do one of two things:

  * AGREE with it  -> pure noise, and
  * DISAGREE       -> it REBINDS an already-correct resolution, with no
                      diagnostic anywhere.

The second case shipped: `d3/conly-alias-32` added
`PROVIDE(SignalSema = func_00352000);` to tools/ee/conly_provides.ld, while
tools/ee/eetest/ghidra_named_funcs.ld already had `PROVIDE(SignalSema =
0x0011ac40);`.  func_00352000 is not SignalSema — it is a function that CALLS
SignalSema.  conly_provides.ld is passed first (overlay_package.sh:113 before
:104's $SYMS_LD; conly_finish.sh:77 before $SYMS), so the wrong binding wins.

Nothing in the tree checked which of the two cases a new PROVIDE was.  This does.

USAGE
    /usr/bin/python3 tools/ee/check_provide_conflicts.py <new.ld> <baseline.ld>
    (files may be paths, or `-` to read stdin — use `git show REF:path` to check
     any revision without a checkout)

    exit 0 = no conflicts; exit 1 = DISAGREE or UNCOMPARABLE overlaps found.

TWO DESIGN HAZARDS, AND HOW EACH IS HANDLED
-------------------------------------------
1. VALUE FORMS DIFFER.  The baseline uses hex literals (`0x0011ac40`); the new
   file uses symbol names (`func_00352000`).  A textual value comparison would
   flag every symbolic-vs-literal pair as a disagreement and the gate would cry
   wolf on all 6 overlaps instead of 1.
   Resolution: `func_<HEX>` / `D_<HEX>` names ARE their own address, and that is
   not a convention we assume — it is generated mechanically by
   conly_finish.sh:59, which scrapes `(D_|func_)[0-9A-Fa-f]{4,}` out of the asm
   tree and emits `func_XXXX = 0xXXXX;` into $ALLSYMS, the very script that
   defines these names at link time.  So we resolve those names to ints and
   compare numerically.
   Anything we CANNOT resolve (e.g. `PROVIDE(g_dialogBusy = g_dialogVoiceActive)`)
   is reported as UNCOMPARABLE, never silently counted as agreement.  Silence
   about an overlap we could not evaluate is exactly the failure this gate is
   for.
2. NAME PREFIXES COLLIDE.  `iSignalSema` sits one line from `SignalSema`.  The
   overlap is computed on EXACT whole-symbol-name keys (dict lookup), never a
   substring or regex-prefix match, so the two can never be conflated.

REGION
------
Both input files are region-agnostic by PATH but carry USA ADDRESSES: the
baseline has `snd_Pump = 0x00132028`, which is the USA address (EU's snd_Pump is
0x00132088 — going-decompiled/symbol_addrs/eu/symbol_addrs.txt).  This checker
therefore reports conflicts *within the USA address space*.  It does NOT and
cannot tell you that a USA address is wrong for EU: neither input file holds an
EU address to compare against.  See the note at the bottom of this file.
"""
import re
import sys

# PROVIDE(name = value) / PROVIDE_HIDDEN(name = value), with an optional trailing ';'
PROVIDE_RE = re.compile(
    r'\bPROVIDE(?:_HIDDEN)?\s*\(\s*([A-Za-z_.$][A-Za-z0-9_.$]*)\s*=\s*([^)]*?)\s*\)'
)
# A name that encodes its own address, per conly_finish.sh:59.
ADDR_NAME_RE = re.compile(r'^(?:D_|func_)([0-9A-Fa-f]{4,})$')
HEX_RE = re.compile(r'^0[xX][0-9A-Fa-f]+$')
DEC_RE = re.compile(r'^[0-9]+$')


def strip_comments(text):
    """Remove /* ... */ block comments so trailing `/* 0x352000 */` notes are not
    mistaken for values.  Deliberately does NOT trust those comments as a value
    source: a comment records what someone believed, the expression is what ld
    links.

    Each comment is replaced by its own newlines, not by a space: collapsing a
    multi-line header comment shifts every later line number, and a gate that
    points at the wrong line is a gate people stop believing."""
    return re.sub(r'/\*.*?\*/',
                  lambda m: '\n' * m.group(0).count('\n'),
                  text, flags=re.S)


def resolve(value):
    """-> (int_address, None) if the value denotes a concrete address,
       else (None, reason)."""
    v = value.strip()
    if HEX_RE.match(v):
        return int(v, 16), None
    if DEC_RE.match(v):
        return int(v, 10), None
    m = ADDR_NAME_RE.match(v)
    if m:
        return int(m.group(1), 16), None
    return None, 'not an address literal and not a func_/D_ address-encoding name'


def parse(path):
    """-> (dict name -> (value_str, addr_or_None, lineno), [intra-file dup records])"""
    if path == '-':
        text = sys.stdin.read()
    else:
        with open(path, 'r', errors='replace') as fh:
            text = fh.read()
    provides, dups = {}, []
    for lineno, raw in enumerate(strip_comments(text).splitlines(), 1):
        for name, value in PROVIDE_RE.findall(raw):
            addr, _ = resolve(value)
            if name in provides:
                dups.append((name, provides[name], (value, addr, lineno)))
            else:
                provides[name] = (value, addr, lineno)
    return provides, dups


def fmt(entry):
    value, addr, lineno = entry
    shown = value if addr is None else '%s (0x%08x)' % (value, addr)
    return '%s @line %d' % (shown, lineno)


def main(argv):
    if len(argv) != 3:
        sys.stderr.write(__doc__.split('USAGE')[1].split('TWO DESIGN')[0])
        return 2
    new_path, base_path = argv[1], argv[2]
    new, new_dups = parse(new_path)
    base, base_dups = parse(base_path)

    print('NEW      %-46s %4d PROVIDEs' % (new_path, len(new)))
    print('BASELINE %-46s %4d PROVIDEs' % (base_path, len(base)))

    # Exact whole-name keys: `SignalSema` and `iSignalSema` are distinct keys and
    # cannot be conflated (hazard 2).
    overlap = sorted(set(new) & set(base))
    disagree, agree, uncomparable = [], [], []
    for name in overlap:
        n, b = new[name], base[name]
        if n[1] is None or b[1] is None:
            uncomparable.append(name)
        elif n[1] != b[1]:
            disagree.append(name)
        else:
            agree.append(name)

    print('\nOVERLAP (%d): %s' % (len(overlap), ' '.join(overlap) or '(none)'))

    print('\n== DISAGREE (%d) — a new PROVIDE REBINDS an existing one ==' % len(disagree))
    for name in disagree:
        print('  %-24s NEW %s' % (name, fmt(new[name])))
        print('  %-24s BASE %s' % ('', fmt(base[name])))
    if not disagree:
        print('  (none)')

    print('\n== UNCOMPARABLE overlap (%d) — value form not resolvable to an address ==' % len(uncomparable))
    for name in uncomparable:
        print('  %-24s NEW %s | BASE %s' % (name, fmt(new[name]), fmt(base[name])))
    if not uncomparable:
        print('  (none)')

    print('\n== AGREE (%d) — redundant but harmless ==' % len(agree))
    for name in agree:
        print('  %-24s = 0x%08x' % (name, new[name][1]))
    if not agree:
        print('  (none)')

    # Same defect class, one file inward.  INFO only: these live in files this
    # gate does not ask you to change, and failing on them would train people to
    # switch the gate off.
    info = [(new_path, d) for d in new_dups] + [(base_path, d) for d in base_dups]
    conflicting = [(p, d) for (p, d) in info if d[1][1] != d[2][1]]
    print('\n== INFO: intra-file duplicate PROVIDEs that DISAGREE (%d) ==' % len(conflicting))
    for path, (name, first, later) in conflicting:
        print('  %-24s %s: %s vs %s' % (name, path, fmt(first), fmt(later)))
    if not conflicting:
        print('  (none)')

    bad = len(disagree) + len(uncomparable)
    print('\nRESULT: %d disagreeing + %d uncomparable overlap(s) -> exit %d'
          % (len(disagree), len(uncomparable), 1 if bad else 0))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))

# ── EU COVERAGE, STATED EXPLICITLY ────────────────────────────────────────────
# NOT COVERED, and here is exactly what is missing.  Both inputs are USA-address
# files (baseline: snd_Pump = 0x00132028 = USA; EU is 0x00132088), and neither
# has an EU sibling anywhere in the tree:
#     /usr/bin/find tools/ee -name '*.ld' -not -path '*/.*'
# gives only conly_provides.ld and eetest/{ee,ee_lib,ee_state,ghidra_named_funcs}.ld.
# Yet conly_finish.sh takes a REGION argument (`eu` -> SCES_516.07, :10) and
# loads BOTH region-agnostic scripts unchanged (:77), while its $ALLSYMS is built
# from the REGION's own asm tree (:59).  So under `conly_finish.sh eu`,
# `func_00352000` resolves to the EU function at 0x352000 —
# going-decompiled/asm/eu/nonmatchings/text/251520/func_00352000.s, a DIFFERENT
# unit from USA's going-decompiled/asm/usa/nonmatchings/text/250080/func_00352000.s
# — so the same alias line is wrong in EU for a second, independent reason.
# This gate cannot see that: it compares two files that contain no EU address.
# What it would take: an EU address source to compare against — either an
# EU-generated ghidra_named_funcs.ld, or a rule that resolves each `func_<HEX>`
# through going-decompiled/symbol_addrs/<region>/ and flags any alias whose
# resolved target is not the same function in both regions.

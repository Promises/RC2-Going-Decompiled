#!/usr/bin/env python3
"""Run splat with this project's include-macro vocabulary.

splat decides which functions of a `c` segment to emit under nonmatchings/ by a
literal search of the C file for `INCLUDE_ASM(`. This tree has more spellings
of a leaf than that, all in include/include_asm.h:

  INCLUDE_ASM_FRAGMENT(FOLDER, NAME)  a debris leaf that is not a match target,
                                      expanding identically to INCLUDE_ASM;
  S136OS_SLOT(NAME)                   a function compiled by the s136os arm
                                      (tools/ee/s136os_functions.txt). It emits
                                      no bytes, but tools/ee/s136os_splice.sh
                                      reads the ROM length of every selector row
                                      from asm/<region>/nonmatchings/<unit>/
                                      <NAME>.s (task #1531), so the leaf must
                                      exist (FACT #9813, task #1882).

Neither substring contains `INCLUDE_ASM(`, so a plain `python -m splat split`
never emits those leaves. The tree then builds only while stale copies of them
are tracked, and a fresh split cannot build.

S136OS_SLOT takes ONE argument, the name. Its leaf path is not read from the
macro: splat writes every leaf of a segment to
<nonmatchings_path>/<segment dir>/<segment name>/<NAME>.s, the same directory
the unit's INCLUDE_ASM leaves are in. With this tree's yaml that is
asm/<region>/nonmatchings/<unit>/<NAME>.s, the path the splice reads.

The list of spellings is checked against include_asm.h on every run: each
function-like macro the header #defines must be classified below, as a leaf
(with the argument that names it) or as not a leaf, and the split FAILS naming
any it does not know. A new spelling therefore stops the split instead of
silently breaking a clean checkout's build while the private tree, which still
tracks the stale leaves, keeps building (INCLUDE_ASM_FRAGMENT, task #1255, then
S136OS_SLOT, 2026-10-04..08, were both found that way, late).

Otherwise this runs splat's own CLI unchanged:

    python tools/splat_ext/run_splat.py split <config.yaml> [--use-cache]
"""
import re
import sys
from pathlib import Path

from splat.__main__ import splat_main
from splat.segtypes.common.c import CommonSegC

INCLUDE_ASM_H = Path(__file__).resolve().parents[2] / "going-decompiled" / "include" / "include_asm.h"

# Leaf macros: a use names a function splat must emit a nonmatchings .s for.
# Value = (arity, index of the argument that is the function name).
LEAF_MACROS = {
    "INCLUDE_ASM": (2, 1),
    "INCLUDE_ASM_FRAGMENT": (2, 1),
    "S136OS_SLOT": (1, 0),
}
# Function-like macros of include_asm.h that are NOT function leaves.
NOT_LEAF_MACROS = {
    "INCLUDE_RODATA": "a rodata leaf; splat finds it itself (find_include_rodata)",
    "INCLUDE_ASM_ALIAS": "a symbol equate between two names; no body, no leaf",
}

DEFINE_RE = re.compile(r"^[ \t]*#[ \t]*define[ \t]+(\w+)\(([^)]*)\)", re.MULTILINE)


def check_macro_vocabulary(header: Path = INCLUDE_ASM_H) -> list:
    """Every reason the tables above disagree with <header>; empty = agree."""
    try:
        text = header.read_text(encoding="utf-8")
    except OSError as e:
        return [f"cannot read {header}: {e}"]
    defined = {}
    for m in DEFINE_RE.finditer(text):
        params = [p.strip() for p in m.group(2).split(",") if p.strip()]
        defined.setdefault(m.group(1), set()).add(len(params))
    bad = []
    if not defined:
        bad.append(f"no function-like #define found in {header}; the vocabulary check would admit anything")
    for name, arities in sorted(defined.items()):
        if name in LEAF_MACROS:
            want = LEAF_MACROS[name][0]
            if arities != {want}:
                bad.append(f"{name} is #defined with {sorted(arities)} argument(s), run_splat.py expects {want}")
        elif name not in NOT_LEAF_MACROS:
            bad.append(f"{name} is #defined in {header} but run_splat.py does not know whether it is an asm leaf")
    for name in sorted(set(LEAF_MACROS) | set(NOT_LEAF_MACROS)):
        if name not in defined:
            bad.append(f"{name} is listed in run_splat.py but not #defined in {header}")
    return bad


def _find_macro_arg(text: str, macro: str, index: int):
    # splat's own substring search and parenthesis matching, any argument.
    for pos in CommonSegC.find_all_instances(text, f"{macro}("):
        start = pos + len(f"{macro}(")
        args = text[start:CommonSegC.get_close_parenthesis(text, start) - 1].split(",")
        if len(args) > index:
            yield args[index].strip()


def _find_include_asm(text: str):
    for macro, (_, index) in LEAF_MACROS.items():
        if index == 1:
            # splat's own finder, so its use_legacy_include_asm option still holds
            yield from CommonSegC.find_include_macro(text, macro)
        else:
            yield from _find_macro_arg(text, macro, index)


CommonSegC.find_include_asm = staticmethod(_find_include_asm)

if __name__ == "__main__":
    problems = check_macro_vocabulary()
    if problems:
        sys.exit("run_splat.py: FATAL — the include-macro vocabulary disagrees with "
                 "include_asm.h, so a split could silently omit leaves a build needs:\n"
                 + "\n".join(f"    {p}" for p in problems))
    sys.argv[0] = "splat"
    splat_main()

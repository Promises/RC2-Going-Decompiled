#!/usr/bin/env python3
"""Run splat with this project's include-macro vocabulary.

splat decides which functions of a `c` segment to emit under nonmatchings/ by a
literal search of the C file for `INCLUDE_ASM(`. This tree also uses
INCLUDE_ASM_FRAGMENT (include/include_asm.h: a debris leaf that is not a match
target, expanding identically to INCLUDE_ASM). The substring `INCLUDE_ASM(` does
not occur inside `INCLUDE_ASM_FRAGMENT(`, so a plain `python -m splat split`
never emits those leaves. The tree then builds only while stale copies of them
are tracked, and a fresh split cannot link.

This wrapper teaches splat the second spelling and otherwise runs splat's own
CLI unchanged:

    python tools/splat_ext/run_splat.py split <config.yaml> [--use-cache]
"""
import sys

from splat.__main__ import splat_main
from splat.segtypes.common.c import CommonSegC

INCLUDE_ASM_MACROS = ("INCLUDE_ASM", "INCLUDE_ASM_FRAGMENT")


def _find_include_asm(text: str):
    for macro in INCLUDE_ASM_MACROS:
        yield from CommonSegC.find_include_macro(text, macro)


CommonSegC.find_include_asm = staticmethod(_find_include_asm)

if __name__ == "__main__":
    sys.argv[0] = "splat"
    splat_main()

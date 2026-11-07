#!/usr/bin/env python3
"""Run splat on a level config so that every auto-generated level name carries the level.

    .venv-decomp/bin/python tools/levels/split_level.py going-decompiled/levels/usa/L24/config/L24.yaml

Our own wrapper (task #2456, NOTE #13098 row L3), the same pattern as
tools/splat_ext/run_splat.py: splat's CLI unchanged, plus one naming rule.

Why it exists: auto names come from two places, splat's Symbol.default_name
(formatted by `symbol_name_format`, or for a symbol with no rom address by
`symbol_name_format_no_rom`, whose `$SEG` falls back to a bare `{vram:X}`) and
spimdisasm's ContextSymbol.getDefaultName (branch/jump-table labels). Neither
format option can prefix only the level's own range: a global `L24_$VRAM`
would also rename references into the resident core. Left alone, both give
`D_1AA958` -- the boot's naming. In a level those addresses are the level's own
(the program overwrites 0x1A7C80 upward), so the boot-style name would be the
same string as a DIFFERENT boot symbol at the same address. RULING #7226 and the
L3 brief require level-prefixed names.

The rule: an auto-generated name whose address lies inside the level program's
extent [lowest segment vram, end of .text) gets `L<NN>_` before its 8-digit
address, so `D_1AA958` becomes `D_L24_001AA958` and `.L002A2CA0` becomes
`.LL24_002A2CA0`. Addresses outside the extent are left exactly as spimdisasm
names them: below 0x1A7C80 that is the resident core, whose names stay the
boot's (core_externs.txt); above it are hardware/scratchpad constants.
Names given in symbol_addrs are never touched (the rule only replaces the
default name).

The extent and the tag are read from the yaml itself (options.basename, the
segment list), so a re-split needs nothing but the committed config and the
flat image it names.
"""
import sys

import yaml

from spimdisasm.common.ContextSymbols import ContextSymbol
from splat.__main__ import splat_main
from splat.util.symbols import Symbol


def level_extent(cfg):
    segs = cfg["segments"]
    starts = [(s["start"], s["vram"]) for s in segs if isinstance(s, dict)]
    end_rom = segs[-1][0]
    last_start, last_vram = starts[-1]
    lo = min(v for _, v in starts)
    hi = last_vram + (end_rom - last_start)
    return lo, hi


def main():
    if len(sys.argv) != 2:
        print(__doc__.strip().splitlines()[2].strip(), file=sys.stderr)
        sys.exit(2)
    path = sys.argv[1]
    cfg = yaml.safe_load(open(path))
    tag = cfg["options"]["basename"]
    if not (len(tag) == 3 and tag[0] == "L" and tag[1:].isdigit()):
        sys.exit("split_level: options.basename must be the level tag LNN, got %r" % tag)
    lo, hi = level_extent(cfg)
    print("split_level: %s extent 0x%08X..0x%08X; auto names inside it get %s_" % (tag, lo, hi, tag))

    orig = ContextSymbol._defaultName_uniqueIdentifier

    def unique_identifier(self, symType):
        ident = orig(self, symType)
        if lo <= self.address < hi:
            for form in ("%08X", "%06X"):
                a = form % self.address
                if ident.startswith(a):
                    return "%s_%08X%s" % (tag, self.address, ident[len(a):])
        return ident

    ContextSymbol._defaultName_uniqueIdentifier = unique_identifier

    # splat's own default names (the ones it hands back to spimdisasm through
    # its name callback): the same rule, applied to the formatted address.
    orig_format = Symbol.format_name

    def format_name(self, fmt):
        if lo <= self.vram_start < hi:
            return "%s_%08X" % (tag, self.vram_start)
        return orig_format(self, fmt)

    Symbol.format_name = format_name
    sys.argv = [sys.argv[0], "split", path]
    splat_main()


if __name__ == "__main__":
    main()

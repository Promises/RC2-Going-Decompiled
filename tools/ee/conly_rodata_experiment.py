#!/usr/bin/env python3
"""Inject a .calt_rodata output section into a GENERATED C-only linker script.

EXPERIMENT, NOT A LANDED FIX. Reproduces decomper-3-m1 /5241: giving the C arm's
constant pools a home eliminates all 111 "defined in discarded section" relocation
errors, with the undefined-symbol class provably unperturbed.

WHY A SCRIPT AND NOT A ONE-LINE DIRECTIVE
The directive alone is not the change. Two file-level facts are load-bearing and no
one-liner carries them:

  1. POSITION. The generated script ends with a CATCH-ALL
         /DISCARD/ : { *(*); }
     which swallows every input section not explicitly placed earlier. The new
     section MUST be inserted BEFORE it, or it is discarded exactly like the
     .rodata it is meant to rescue.

  2. ADDRESS. It must sit ABOVE the highest existing section. .legal_data is at
     0x01800000 (+0x5d80), so by the insertion point the location counter is
     already past 0x00C00000. Placing it at 0x00C00000 -- the address
     build_conly.sh's stale comment names -- still removes all 111 errors but
     emits 3 "dot moved backwards" warnings, which is how sections silently
     overlap. Measured, v1 vs v2.

SCOPE
The glob is *.calt.o so only the C arm is affected. The asm arm and the four rodata
SEGMENTS (cod/03A180, lvl.vtbl, lvl.camvtbl, lvl.sndvtbl) keep their existing
explicit placements and are not matched here.

USAGE
    conly_rodata_experiment.py <generated.ld> <out.ld>

then re-run ONLY the link -- the objects do not change, so a full build_conly.sh
rebuild (~83 min) is unnecessary:

    mips-linux-gnu-ld -EL --allow-multiple-definition \\
        -T <out.ld> -T $BUILD/undefined_syms_auto.txt -T $BUILD/all_addr_syms.ld \\
        -Map $BUILD/EXPERIMENT2.map -o $BUILD/EXPERIMENT2.elf 2> $BUILD/ld.EXPERIMENT2.log

VERIFYING THE RESULT -- read this before believing a zero
ZERO discarded-section errors is ALSO what "the calt objects were never linked"
produces (wrong glob, empty CALT_LIST, provisioning miss), and the log looks
identical. So a zero is only evidence if the section RECEIVED BYTES. Check the map:

    .calt_rodata    0x01820000      0x870     <- non-zero SIZE is the proof
     .rodata        0x01820000      0x124  .../cod/015180.calt.o
     .rodata        0x01820130       0x74  .../text/16E980.calt.o
     ... 12 objects total

0x870 = 2160 bytes against 2056 bytes of raw .rodata; the difference is alignment
fill, which the map shows as explicit *fill* rows.
"""
from __future__ import annotations
import sys

# Above .legal_data (0x01800000 + 0x5d80). NOT a designed address -- the first free
# spot above the highest section. A real landing should choose deliberately against
# the ROM memory map, overlay regions and any loader assumptions.
CALT_RODATA_VMA = "0x01820000"

CATCH_ALL = "    /DISCARD/ :"

BLOCK = f"""    .calt_rodata {CALT_RODATA_VMA} :
    {{
        *.calt.o(.rodata*)
    }}

"""


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__.strip().splitlines()[0], file=sys.stderr)
        print("usage: conly_rodata_experiment.py <generated.ld> <out.ld>", file=sys.stderr)
        return 2

    src, dst = sys.argv[1], sys.argv[2]
    text = open(src).read()

    # Fail loudly rather than emit a script whose placement is not what the caller
    # thinks. An unmatched or duplicated marker means the generator changed shape.
    n = text.count(CATCH_ALL)
    if n != 1:
        print(f"CONTROL FAILED: expected exactly 1 '{CATCH_ALL.strip()}' in {src}, found {n}. "
              "The generated script's shape has changed; re-check the insertion point.",
              file=sys.stderr)
        return 2

    out = text.replace(CATCH_ALL, BLOCK + CATCH_ALL)
    open(dst, "w").write(out)

    print(f"wrote {dst}")
    print(f"  .calt_rodata at {CALT_RODATA_VMA}, inserted BEFORE the catch-all /DISCARD/")
    print(f"  bytes added to script: {len(out) - len(text)}")
    print("  NEXT: re-link, then confirm .calt_rodata has NON-ZERO SIZE in the map "
          "before trusting a zero error count.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

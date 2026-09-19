#!/bin/bash
# blanket_orphans.sh <region> [--root TREE] [-v] — the ORPHAN class of
# tools/ee/landing_gate.sh (task #457 ENFORCE-2): D_/func_ tokens that compiled
# C under src/<region> references and that only build.sh's blanket grep of
# asm/<region> keeps linkable. Descends from tools/ee/.t452/12_blanket_orphans.sh
# (preserve/evidence/t452 4f66530e, FACT #7301: `comm -23 <(tokens in src)
# <(tokens in asm)`), which listed tokens, not references — 101 rows at 8b536f0c,
# mostly comments, #else arms and C definitions. This one evaluates the
# preprocessor arm build.sh compiles, drops comments/strings/extern
# declarations/INCLUDE_ASM arguments, subtracts compiled C definitions and
# symbol_addrs lines under the exact spelling, and classifies the holders:
#
#   ORPHAN        <token> <file:line>[+N] holders=none      undefined at link NOW
#   ORPHAN_LATENT <token> <file:line>[+N] holders=<f.s,..>  every holder is a
#                 nonmatchings/**/func_*.s — a file a class-1/class-2 rename
#                 deletes (#452 deleted 278 and orphaned four tokens)
#
# Controls: at 6aaac34e USA prints 0 ORPHAN / 123 ORPHAN_LATENT and EU prints
# exactly the 7 D_/func_ members of landing_baseline/ldundef_eu.txt as ORPHAN;
# 8b536f0c with #452's 72 deleted USA leftovers removed prints exactly FACT
# #7301's four (tools/ee/.t457/03_control_*). The gate's --selftest deletes one
# holder in a scratch copy and requires ORPHAN to GROW.
set -u
R=${1:?region}; shift
ROOT=.; EXTRA=()
while [ $# -gt 0 ]; do
  case "$1" in
    --root) ROOT=$2; shift 2 ;;
    *) EXTRA+=("$1"); shift ;;
  esac
done
HERE=$(cd "$(dirname "$0")" && pwd)
exec python3 "$HERE/blanket_orphans.py" "$ROOT" "$R" ${EXTRA[@]+"${EXTRA[@]}"}

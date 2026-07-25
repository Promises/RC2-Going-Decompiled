#!/usr/bin/env bash
# verify_match_unit.sh — RAW byte gate that scopes ONE function out of a
# WHOLE-UNIT object and compares it against the ORIGINAL ROM BYTES.
#
#   verify_match_unit.sh <func> <whole_unit_base.o> <single_func_target.o> [region]
#
# WHAT IT COMPARES (the decisive test)
# ------------------------------------
# diff96.sh/diff.sh's base.o is the whole compiled unit (the MATCH function as
# real cc1 code, every sibling INCLUDE_ASM'd). Its instruction words still carry
# ZEROED immediates wherever a relocation applies (HI16/LO16/jal targets are
# filled in by the linker, not the assembler). This tool slices the target
# function's `objdump -dr` block out of base.o, RESOLVES those relocations with
# the real symbol addresses (symbol_addrs/<region>/symbol_addrs.txt plus the
# address encoded in func_/D_ names), and compares the resulting WORDS against
# the flat ROM at `file offset = vaddr - 0x100080`.
#
# The ROM is ground truth, so this is immune to two things that made the older
# reloc-LINE diff report false DIFFERS:
#   * targets that are pre-linked `.word` blobs with NO relocations at all
#     (their immediates are already resolved, so there were no reloc lines to
#     match against base.o's — permanently unmatchable by a line diff), and
#   * a callee being spelled `sceDmaSyncChan` on one side and `func_00126E60`
#     on the other. Both resolve to 0x00126E60, so the compared word is equal.
#     Name-level differences are not byte-level differences.
#
# The target.o is used only for its SHAPE (which function, how many words) and
# as the argument-contract check — never as the byte oracle.
#
# RELOCATION ADDENDS: MIPS o32 is REL, not RELA — the addend lives IN PLACE in
# the instruction's immediate field. A resolver that overwrites the immediate
# with the bare symbol address silently drops it, which is a FALSE DIFFERS on any
# `sym + off` reference (measured: `%lo(func_001248B0 + 0x8)`). Each type adds its
# own in-place addend back; GPREL16 additionally needs the per-region _gp.
#
# EXIT STATUS (a misuse must never look like a verdict)
#   0  MATCH        — every word equals the ROM
#   1  DIFFERS      — a real byte difference (this, and only this, is a failure)
#   2  USAGE/ARG    — bad arguments; e.g. a whole-unit .o passed as the target
#   3  UNVERIFIABLE — the tool cannot decide (unresolvable symbol, reloc type it
#                     does not model, function absent from the ROM window). NOT
#                     a pass and NOT a fail; it is its own visible state.
#
# COUNTING FUNCTIONS (what a sweep's denominator must be)
# ------------------------------------------------------
# Sweeps of one unit disagreed (622 vs 684/685) purely by SELECTION, so measured
# once for usa cod/015180 and recorded here. `objdump -d` on the BASE object
# yields 794 blocks, which is NOT a function count - subtract 107 `.L` local
# branch labels, 1 `.NON_MATCHING` object marker and 7 `D_` pre-linked word
# blobs to get:
#
#   679 code functions  <-- the correct denominator
#     482 still INCLUDE_ASM -> those match BY CONSTRUCTION (the gate re-asserts
#         the original bytes against themselves; they are not decomp progress)
#     197 real decompiled C bodies  <-- the number that actually means anything
#
# Cross-checked independently: 482 == the `^INCLUDE_ASM` line count in
# src/usa/cod/015180.c. (A bare `grep -c INCLUDE_ASM` says 510 - 28 of those are
# comment prose, not directives.)
#
# Do NOT take the denominator from the TARGET object: it lists only 666 code
# blocks because 13 already-matched functions had their frozen `.s` DELETED, so
# they exist as C in the base but have no target-side block at all. Its 672
# `F .text` symtab entries are a third, also-wrong number (24 are zero-size).
# Quoting a big MATCH total without the INCLUDE_ASM/real-C split overstates
# progress by roughly 3.4x on this unit.
set -u

usage() {
  echo "usage: $(basename "$0") <func> <whole_unit_base.o> <single_func_target.o> [region]" >&2
  exit 2
}

[ $# -ge 3 ] && [ $# -le 4 ] || usage
FN="$1"; BASE="$2"; TGT="$3"; REGION="${4:-usa}"

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"  # script-relative = worktree-portable

for f in "$BASE" "$TGT"; do
  [ -f "$f" ] || { echo "ARG ERROR: not a file: $f" >&2; exit 2; }
done

SYMS="going-decompiled/symbol_addrs/$REGION/symbol_addrs.txt"
[ -f "$SYMS" ] || { echo "ARG ERROR: no symbol map for region '$REGION': $SYMS" >&2; exit 2; }

# The flat .rom is gitignored and lives in the MAIN repo, which is not
# necessarily this worktree. Search this root first, then sibling checkouts.
# The boot-ELF basename is REGION-SPECIFIC: keying every region off the USA
# name made region 'eu' permanently unfindable, so an EU invocation exited
# UNVERIFIABLE no matter how correct the decomp was.
case "$REGION" in
  usa|usa_v101) ROM_NAME="SCUS_972.68.rom"; ROM_ENV="${EE_ROM_USA:-}";;
  eu)           ROM_NAME="SCES_516.07.rom"; ROM_ENV="${EE_ROM_EU:-}";;
  *)            echo "ARG ERROR: unknown region '$REGION' (expected usa, usa_v101 or eu)" >&2; exit 2;;
esac
ROM=""
for cand in \
  "$ROOT/extracted/$REGION/$ROM_NAME" \
  "$ROOT/../ps2-gc-re/extracted/$REGION/$ROM_NAME" \
  "$ROM_ENV" ; do
  [ -n "$cand" ] && [ -f "$cand" ] && { ROM="$cand"; break; }
done
if [ -z "$ROM" ]; then
  echo "UNVERIFIABLE: flat ROM '$ROM_NAME' for region '$REGION' not found" >&2
  exit 3
fi

# ---- ARGUMENT CONTRACT (defect class: a whole-unit .o passed where a
# single-function .o is expected used to yield a 17k-line diff and a confident
# "DIFFERS"). A single-function target.o has exactly ONE `F .text` symbol and a
# .text no larger than one function; the whole unit has hundreds. Refuse loudly.
SHAPE="$(docker --context colima-ee-x86 run --rm -v "$ROOT":/work -w /work ee-build sh -c "
  mips-linux-gnu-objdump -t '$TGT' 2>/dev/null | awk '\$3==\"F\" && \$4==\".text\"' | wc -l
  mips-linux-gnu-objdump -h '$TGT' 2>/dev/null | awk '\$2==\".text\"{print \$3}'
")" || { echo "ARG ERROR: could not read '$TGT' as an object file" >&2; exit 2; }

TGT_FUNCS="$(echo "$SHAPE" | sed -n 1p)"
TGT_TEXT_HEX="$(echo "$SHAPE" | sed -n 2p)"
TGT_TEXT="$(printf '%d' "0x${TGT_TEXT_HEX:-0}" 2>/dev/null)"
[ -n "${TGT_FUNCS:-}" ] && [ -n "${TGT_TEXT:-}" ] || { echo "ARG ERROR: '$TGT' is not a readable ELF object" >&2; exit 2; }

if [ "$TGT_FUNCS" -eq 0 ]; then
  echo "ARG ERROR: '$TGT' contains no .text function symbol — not a single-function target object" >&2
  exit 2
fi
if [ "$TGT_FUNCS" -gt 1 ]; then
  echo "ARG ERROR: '$TGT' contains $TGT_FUNCS .text functions ($TGT_TEXT bytes) — that is a WHOLE-UNIT object." >&2
  echo "           Argument 3 must be the single-function target.o. Refusing to emit a verdict." >&2
  exit 2
fi
# A one-symbol object whose .text is unit-sized is also not a per-function target.
if [ "$TGT_TEXT" -gt 65536 ]; then
  echo "ARG ERROR: '$TGT' has a ${TGT_TEXT}-byte .text — too large to be a single-function target object." >&2
  exit 2
fi

# ---- Slice + resolve + compare against the ROM.
DIS_FILE="$(mktemp -t verify_match_unit)"
trap 'rm -f "$DIS_FILE"' EXIT
docker --context colima-ee-x86 run --rm -v "$ROOT":/work -w /work ee-build sh -c \
  "mips-linux-gnu-objdump -dr --section=.text '$BASE' 2>/dev/null" >"$DIS_FILE" \
  || { echo "ARG ERROR: could not disassemble '$BASE'" >&2; exit 2; }
[ -s "$DIS_FILE" ] || { echo "ARG ERROR: '$BASE' produced no .text disassembly" >&2; exit 2; }

FN="$FN" ROM="$ROM" SYMS="$SYMS" REGION="$REGION" DIS_FILE="$DIS_FILE" python3 - <<'PY'
import os, re, struct, sys

FN     = os.environ["FN"]
ROM    = os.environ["ROM"]
SYMS   = os.environ["SYMS"]
REGION = os.environ["REGION"]
# Flat .rom convention: file offset = vaddr - 0x100080. MEASURED for BOTH
# regions rather than assumed from USA: locating two independent known anchors
# in the EU rom (the reloc-free prologue of the fn at 0x11D3A0, and memset's
# 96-byte body at 0x115484) each yields a UNIQUE hit whose implied delta is
# 0x100080. Cross-checked end-to-end - all 1524 words of eu cod/0321A0's frozen
# asm equal the EU rom at this offset.
ROM_BASE = 0x100080

# Per-region _gp. GPREL16 resolves as `symbol + addend - _gp`, sign-extended to
# 16 bits. NEITHER value is assumed from CLAUDE.md - each was SOLVED, and each
# gp-relative slot independently implies `_gp = symbol_vaddr - signext16(imm)`,
# so a real gp is the value every slot agrees on:
#   usa 0x1AEFF0 - unique across all 86 GPREL16 slots in usa cod/0321A0
#   eu  0x1AF070 - unique across all 92 GPREL16 slots in eu  cod/0321A0
# Both solved from immediates verified equal to their region's rom bytes, so the
# input is the binary itself. A region with no entry here keeps GPREL16
# UNVERIFIABLE rather than guessing.
GP = {"usa": 0x1AEFF0, "eu": 0x1AF070}.get(REGION)

MATCH, DIFFERS, ARGERR, UNVERIFIABLE = 0, 1, 2, 3

# Symbol map: "name = 0xADDR; // comment"
syms = {}
for line in open(SYMS):
    m = re.match(r"\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)", line)
    if m:
        syms[m.group(1)] = int(m.group(2), 16)

def sign16(v):
    """Interpret a 16-bit immediate as signed (MIPS REL in-place addend)."""
    return v - 0x10000 if v & 0x8000 else v

def resolve(name):
    """Symbol -> vaddr. The map wins; otherwise fall back to the address baked
    into splat's generated names (func_00127E48, D_00141B00, jtbl_0012ABC0,
    .L00118D6C)."""
    if name in syms:
        return syms[name]
    # splat's local branch labels are `.L` + hex with NO underscore, so they are
    # not matched by the underscore-separated generated-name form below. They
    # encode their own vaddr just the same.
    m = re.fullmatch(r"\.L([0-9A-Fa-f]{6,8})", name)
    if m:
        return int(m.group(1), 16)
    # `D_<hex>` with FEWER than 6 digits is not an address at all: splat spells a
    # bare absolute IMMEDIATE that way, and the value is the hex in the name.
    # Verified against the ROM for every such symbol in usa cod/015180 — D_1000,
    # D_4000, D_FFFF, D_FFFFF, D_FFFFFF each reproduce both halves of their
    # hi/lo carry split exactly (e.g. D_FFFFF -> lui 0x0010 / addiu 0xFFFF).
    m = re.fullmatch(r"D_([0-9A-Fa-f]{1,5})", name)
    if m:
        return int(m.group(1), 16)
    m = re.fullmatch(r"(?:func|D|jtbl|L)_([0-9A-Fa-f]{6,8})", name)
    if m:
        return int(m.group(1), 16)
    return None

# The function's own vaddr, needed to index the ROM and to resolve PC16.
fn_va = resolve(FN)
if fn_va is None:
    print(f"{FN}: UNVERIFIABLE — no address for '{FN}' in {SYMS} and none encoded in the name")
    sys.exit(UNVERIFIABLE)

# Slice the function's block out of `objdump -dr`.
words, relocs = [], {}
inside = False
for line in open(os.environ["DIS_FILE"]).read().splitlines():
    m = re.match(r"^[0-9a-f]+ <(\S+)>:", line)
    if m:
        if inside:
            break
        inside = (m.group(1) == FN)
        continue
    if not inside:
        continue
    m = re.match(r"^\s+([0-9a-f]+):\t([0-9a-f]{8}) ", line)
    if m:
        words.append([int(m.group(1), 16), int(m.group(2), 16)])
        continue
    m = re.match(r"^\s+([0-9a-f]+): (R_MIPS_\w+)\s+(\S+)", line)
    if m:
        relocs.setdefault(int(m.group(1), 16), []).append((m.group(2), m.group(3)))

if not words:
    print(f"{FN}: NOT FOUND in base.o (still INCLUDE_ASM?)")
    sys.exit(ARGERR)

unit_off = words[0][0]                       # the fn's offset inside the unit .text

# Resolve relocations into the instruction words.
unresolved, unmodelled = [], set()
resolved = []
for off, w in words:
    va = fn_va + (off - unit_off)
    for rtype, rname in relocs.get(off, []):
        S = resolve(rname)
        if S is None:
            unresolved.append(rname)
            continue
        # MIPS o32 uses REL, not RELA: the ADDEND is stored IN PLACE in the
        # instruction's immediate field, not in the relocation entry. Overwriting
        # the immediate with the bare symbol address therefore DESTROYS the
        # addend, which is how `%lo(func_001248B0 + 0x8)` was mis-resolved to
        # func_001248B0 + 0 and reported as a byte difference against a ROM that
        # was right all along. Every type must add its in-place addend back.
        if rtype == "R_MIPS_HI16":
            # The addend of a HI16 is its immediate scaled by 16, and it pairs
            # with a sign-extended LO16, hence the +0x8000 carry.
            A = sign16(w & 0xFFFF) << 16
            w = (w & 0xFFFF0000) | (((S + A + 0x8000) >> 16) & 0xFFFF)
        elif rtype == "R_MIPS_LO16":
            w = (w & 0xFFFF0000) | ((S + sign16(w & 0xFFFF)) & 0xFFFF)
        elif rtype == "R_MIPS_26":
            # The addend is the stored 26-bit target scaled by 4.
            A = (w & 0x03FFFFFF) << 2
            w = (w & 0xFC000000) | (((S + A) >> 2) & 0x03FFFFFF)
        elif rtype == "R_MIPS_PC16":
            w = (w & 0xFFFF0000) | (((S - (va + 4)) >> 2) & 0xFFFF)
        elif rtype == "R_MIPS_GPREL16" and GP is not None:
            w = (w & 0xFFFF0000) | ((S + sign16(w & 0xFFFF) - GP) & 0xFFFF)
        else:
            unmodelled.add(rtype)
    resolved.append((va, w))

# A reloc we do not model, or a symbol we cannot place, means we CANNOT decide.
# Say so; never fold it into a pass or a fail.
if unmodelled:
    print(f"{FN}: UNVERIFIABLE — unmodelled relocation type(s): {', '.join(sorted(unmodelled))}")
    sys.exit(UNVERIFIABLE)
if unresolved:
    uniq = sorted(set(unresolved))
    print(f"{FN}: UNVERIFIABLE — {len(uniq)} symbol(s) have no known address: {', '.join(uniq[:8])}")
    sys.exit(UNVERIFIABLE)

rom = open(ROM, "rb").read()
lo, hi = resolved[0][0] - ROM_BASE, resolved[-1][0] - ROM_BASE + 4
if lo < 0 or hi > len(rom):
    print(f"{FN}: UNVERIFIABLE — vaddr range 0x{resolved[0][0]:08x}..0x{resolved[-1][0]:08x} outside the flat ROM")
    sys.exit(UNVERIFIABLE)

bad = []
for va, w in resolved:
    rw = struct.unpack_from("<I", rom, va - ROM_BASE)[0]
    if rw != w:
        bad.append((va, w, rw))

nrel = sum(len(v) for v in relocs.values())
if not bad:
    print(f"{FN}: BYTE IDENTICAL TO ROM ✅ ({len(resolved)}/{len(resolved)} words, {nrel} relocs resolved)")
    sys.exit(MATCH)

print(f"{FN}: DIFFERS ❌ — {len(bad)}/{len(resolved)} words differ from the ROM")
for va, w, rw in bad[:40]:
    print(f"  0x{va:08x}: built {w:08x}   rom {rw:08x}")
if len(bad) > 40:
    print(f"  ... and {len(bad)-40} more")
sys.exit(DIFFERS)
PY

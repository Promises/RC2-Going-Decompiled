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
# address encoded in func_/D_ names; a unit's own `.text`/`.rodata`/... section
# symbol is placed as described at SECTION SYMBOLS below), and compares the
# resulting WORDS against the flat ROM at `file offset = vaddr - 0x100080`.
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
#   2  UNVERIFIABLE — the tool cannot decide (unresolvable symbol, reloc type it
#                     does not model, function absent from the ROM window). NOT
#                     a pass and NOT a fail; it is its own visible state.
#   3  USAGE/ARG    — bad arguments; e.g. a whole-unit .o passed as the target
#
# ⚠️ THESE BANDS ARE COMMIT-KEYED — CHECK YOUR CHECKOUT BEFORE TRUSTING THEM.
# The 2/3 assignment above holds only from `8b2c0190` onward. BEFORE that commit
# the code used the OPPOSITE mapping (ARG ERROR→2, UNVERIFIABLE→3) *and* carried a
# header agreeing with it — so a pre-8b2c0190 tree is INTERNALLY SELF-CONSISTENT
# and the difference is invisible from inside it. Two seats therefore return
# OPPOSITE answers to the same band screen and both look correct. A harness that
# retries on 2 and aborts on 3 does the right thing in one tree and the wrong
# thing in BOTH directions in the other.
#   Verify with:  git merge-base --is-ancestor 8b2c0190 HEAD   (rc=0 ⇒ bands above)
#   Measured 2026-07-28: 5 of 8 live seats LACKED 8b2c0190 (rate over live seats;
#   the ~120-worktree agent-* pool was NOT swept, so this is not a fleet total).
# 📌 Do NOT re-key this note to `7b6972a9`: that SHA is NOT an ancestor of master
# (it is a pre-rebase copy with an identical patch-id), so any lock keyed to it
# never binds and reads as satisfied.
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
  exit 3
}

[ $# -ge 3 ] && [ $# -le 4 ] || usage
FN="$1"; BASE="$2"; TGT="$3"; REGION="${4:-usa}"

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"  # script-relative = worktree-portable

for f in "$BASE" "$TGT"; do
  [ -f "$f" ] || { echo "ARG ERROR: not a file: $f" >&2; exit 3; }
done

SYMS="going-decompiled/symbol_addrs/$REGION/symbol_addrs.txt"
[ -f "$SYMS" ] || { echo "ARG ERROR: no symbol map for region '$REGION': $SYMS" >&2; exit 3; }

# The flat .rom is gitignored and lives in the MAIN repo, which is not
# necessarily this worktree. Search this root first, then sibling checkouts.
# The boot-ELF basename is REGION-SPECIFIC: keying every region off the USA
# name made region 'eu' permanently unfindable, so an EU invocation exited
# UNVERIFIABLE no matter how correct the decomp was.
case "$REGION" in
  usa|usa_v101) ROM_NAME="SCUS_972.68.rom"; ROM_ENV="${EE_ROM_USA:-}";;
  eu)           ROM_NAME="SCES_516.07.rom"; ROM_ENV="${EE_ROM_EU:-}";;
  *)            echo "ARG ERROR: unknown region '$REGION' (expected usa, usa_v101 or eu)" >&2; exit 3;;
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
  exit 2
fi

# ---- ARGUMENT CONTRACT (defect class: a whole-unit .o passed where a
# single-function .o is expected used to yield a 17k-line diff and a confident
# "DIFFERS"). A single-function target.o has exactly ONE `F .text` symbol and a
# .text no larger than one function; the whole unit has hundreds. Refuse loudly.
SHAPE="$(docker --context colima-ee-x86 run --rm -v "$ROOT":/work -w /work ee-build sh -c "
  mips-linux-gnu-objdump -t '$TGT' 2>/dev/null | awk '\$3==\"F\" && \$4==\".text\"' | wc -l
  mips-linux-gnu-objdump -h '$TGT' 2>/dev/null | awk '\$2==\".text\"{print \$3}'
")" || { echo "ARG ERROR: could not read '$TGT' as an object file" >&2; exit 3; }

TGT_FUNCS="$(echo "$SHAPE" | sed -n 1p)"
TGT_TEXT_HEX="$(echo "$SHAPE" | sed -n 2p)"
TGT_TEXT="$(printf '%d' "0x${TGT_TEXT_HEX:-0}" 2>/dev/null)"
[ -n "${TGT_FUNCS:-}" ] && [ -n "${TGT_TEXT:-}" ] || { echo "ARG ERROR: '$TGT' is not a readable ELF object" >&2; exit 3; }

if [ "$TGT_FUNCS" -eq 0 ]; then
  echo "ARG ERROR: '$TGT' contains no .text function symbol — not a single-function target object" >&2
  exit 3
fi
if [ "$TGT_FUNCS" -gt 1 ]; then
  echo "ARG ERROR: '$TGT' contains $TGT_FUNCS .text functions ($TGT_TEXT bytes) — that is a WHOLE-UNIT object." >&2
  echo "           Argument 3 must be the single-function target.o. Refusing to emit a verdict." >&2
  exit 3
fi
# A one-symbol object whose .text is unit-sized is also not a per-function target.
if [ "$TGT_TEXT" -gt 65536 ]; then
  echo "ARG ERROR: '$TGT' has a ${TGT_TEXT}-byte .text — too large to be a single-function target object." >&2
  exit 3
fi

# ---- Slice + resolve + compare against the ROM.
#
# ⚠️ TWO THINGS THIS COMPARE CANNOT SEE. Neither is fixed here.
#
# 1. ZERO WORDS THAT objdump ELIDES. Without `-z`, objdump prints a run of zero
#    words as one `...` line. The slicer below only reads instruction lines, so
#    it never compares the ROM against those words. If a real instruction was
#    zeroed next to a nop pair, it reads BYTE IDENTICAL with a SHORT word count
#    (FACT #7936: a zeroed `andi` gave rc 0, 9/9 on a 12-word function). A
#    LEADING run moves `unit_off` (`unit_off = words[0][0]` below is the first
#    PRINTED word) and gives a false DIFFERS instead (doc #6381,
#    `verify-match-unit-zero-run-elision-blindspot`; read it with cv_doc_get,
#    because cv_fact_get 6381 returns an unrelated post).
#    Check: `N/N words` must equal splat's `nonmatching <fn>, 0x<size>` / 4.
#    Adding `-z` here is HUMAN-ONLY (fleet-control/19734). It is NOT applied.
#
# 2. WHERE THE FUNCTION IS IN THE UNIT. Each word is compared at the function's
#    ROM vaddr from symbol_addrs, plus its offset from the start of its own
#    block. The function's position in the built unit is never used. So if a
#    promotion drops post-`endlabel` pad words (FACT #7982) and every later
#    function lands 8 bytes low, no function's verdict changes.
#    `-z` would NOT fix this: the dropped words are not in the base object at
#    all, so there is nothing for it to stop eliding. Measured at task #765 on
#    text/191238 with the pad removed (.text 0x7d68 -> 0x7d60):
#    StartFrontendSegmentLoad 32/32 and MapGetLevelOrderIndex (0x5008 -> 0x5000)
#    28/28 both read BYTE IDENTICAL. Task #780 swept all 16 real-C functions of
#    the unit: verdicts are the same with and without the pad (FACT #8082).
#    The exception to "BYTE IDENTICAL" is StreamSceneSegment, which reads DIFFERS
#    1/34 at 0x29455c both ways. That is not a pad effect. It is this tool's
#    HI16 carry bug (FACT #8027: the paired LO16 addend is dropped, so the lui
#    immediate comes out 1 low). Only a unit-level check sees the pad drop:
#    tools/ee/text_size_check.sh (same seed: off 56, rc 1; master: off 0,
#    rc 0) or the whole-image cmp in landing_gate.sh.
DIS_FILE="$(mktemp -t verify_match_unit)"
trap 'rm -f "$DIS_FILE"' EXIT
docker --context colima-ee-x86 run --rm -v "$ROOT":/work -w /work ee-build sh -c \
  "mips-linux-gnu-objdump -dr --section=.text '$BASE' 2>/dev/null" >"$DIS_FILE" \
  || { echo "ARG ERROR: could not disassemble '$BASE'" >&2; exit 3; }
[ -s "$DIS_FILE" ] || { echo "ARG ERROR: '$BASE' produced no .text disassembly" >&2; exit 3; }

FN="$FN" BASE="$BASE" ROM="$ROM" SYMS="$SYMS" REGION="$REGION" DIS_FILE="$DIS_FILE" python3 - <<'PY'
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

MATCH, DIFFERS, UNVERIFIABLE, ARGERR = 0, 1, 2, 3

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

# SECTION SYMBOLS. GAS rewrites a relocation against a LOCAL symbol (a switch
# table, a float literal, a `static`) as one against the SECTION, with the
# offset left in place as the addend. objdump then names the section: `.rodata`,
# `.data`, `.text`. That name is per-unit, so symbol_addrs cannot carry it, and
# naming the table there (`jtbl_0026CA70`) changes nothing because the relocation
# never mentions it. Before this, every such function was UNVERIFIABLE, and
# stayed so with a seeded base. Measured on the usa build of 2433bb02 (task
# #966): 34 functions, 5 of them real C - EvaluateProgressCondition (.rodata),
# func_002A1DB8, func_002F2E58, func_002F33E8 (.text), __divdi3 (.rodata).
# All but __divdi3 now decide, and a base seed makes each of those 4 real-C
# functions DIFFER. __divdi3 stays UNVERIFIABLE: its __clz_tab is in the ROM
# four times.
#
# Placement, all from the base object and the ROM, never from a link map:
#   .text  - the unit's own .text starts at fn_va - unit_off. The same position
#            assumption every word compare below already makes.
#   other  - the section's bytes, with their own R_MIPS_32 relocations resolved,
#            must occur EXACTLY ONCE in the ROM, at any byte offset. That hit is
#            the section's address. The object's sh_addralign is NOT used: the
#            linker script places sections, and libgcc's __divdi3 .rodata
#            (__clz_tab, 2**4 in the object) is linked at 0x13AC58, 8-aligned.
#            Searching every offset only adds candidates, so it is the stricter
#            uniqueness test. No hit, several hits, a NOBITS section
#            (.bss/.sbss) or a relocation inside it that cannot be resolved all
#            leave the symbol unresolved: UNVERIFIABLE, with the reason printed.
# This cannot make a wrong function MATCH: a word is still compared against the
# ROM, and a placement at a wrong address gives a wrong immediate. A defect in
# the section's own bytes makes the search miss, so it reads UNVERIFIABLE, not
# MATCH; the whole-image cmp in landing_gate.sh is what judges those bytes.
def load_elf(path):
    d = open(path, "rb").read()
    if d[:4] != b"\x7fELF" or d[4] != 1 or d[5] != 1:
        return None                          # not ELF32 little-endian
    shoff, = struct.unpack_from("<I", d, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from("<HHH", d, 0x2E)
    secs = []
    for i in range(shnum):
        nm, typ, _, _, off, size, link, info, _, _ = struct.unpack_from("<10I", d, shoff + i * shentsize)
        secs.append({"nm": nm, "type": typ, "off": off, "size": size,
                     "link": link, "info": info})
    strtab = secs[shstrndx]
    for s in secs:
        e = d.index(b"\0", strtab["off"] + s["nm"])
        s["name"] = d[strtab["off"] + s["nm"]:e].decode()
    return d, secs

def elf_symbol(d, secs, symtab, idx):
    """(name, is_section_symbol) for symbol `idx` of `symtab`."""
    st_name, _, _, st_info, _, st_shndx = struct.unpack_from("<IIIBBH", d, symtab["off"] + idx * 16)
    if st_info & 0xF == 3:                   # STT_SECTION
        return secs[st_shndx]["name"], True
    strs = secs[symtab["link"]]
    e = d.index(b"\0", strs["off"] + st_name)
    return d[strs["off"] + st_name:e].decode(), False

ELF = load_elf(os.environ["BASE"])
text_base = fn_va - unit_off
section_addr, section_why = {}, {}

def resolve_section(name, rom):
    """Vaddr of the unit's own section `name`, or None (reason in section_why)."""
    if name in section_addr or name in section_why:
        return section_addr.get(name)
    if name == ".text":
        section_addr[name] = text_base
        return text_base
    if ELF is None:
        section_why[name] = "base is not an ELF32 LE object"; return None
    d, secs = ELF
    sec = next((s for s in secs if s["name"] == name), None)
    if sec is None:
        section_why[name] = "no such section in the base object"; return None
    if sec["type"] == 8:                     # SHT_NOBITS: nothing to search for
        section_why[name] = "NOBITS section, no content to place"; return None
    body = bytearray(d[sec["off"]:sec["off"] + sec["size"]])
    for rel in secs:
        if rel["type"] != 9 or secs[rel["info"]] is not sec:   # SHT_REL for it
            continue
        for k in range(rel["size"] // 8):
            r_off, r_info = struct.unpack_from("<II", d, rel["off"] + k * 8)
            sym, is_sec = elf_symbol(d, secs, secs[rel["link"]], r_info >> 8)
            S = text_base if is_sec and sym == ".text" else (None if is_sec else resolve(sym))
            if r_info & 0xFF != 2 or S is None:                 # R_MIPS_32 only
                section_why[name] = f"relocation in {name} at +0x{r_off:x} not resolvable"
                return None
            A, = struct.unpack_from("<I", body, r_off)
            struct.pack_into("<I", body, r_off, (S + A) & 0xFFFFFFFF)
    hits, pos = [], rom.find(bytes(body))
    while pos != -1 and len(hits) < 2:
        hits.append(pos)
        pos = rom.find(bytes(body), pos + 1)
    if len(hits) != 1:
        section_why[name] = f"{len(body)}-byte content found {'more than once' if hits else 'nowhere'} in the ROM"
        return None
    section_addr[name] = hits[0] + ROM_BASE
    return section_addr[name]

rom = open(ROM, "rb").read()

# Resolve relocations into the instruction words.
unresolved, unmodelled = [], set()
resolved = []
for off, w in words:
    va = fn_va + (off - unit_off)
    for rtype, rname in relocs.get(off, []):
        S = resolve(rname)
        if S is None and rname.startswith("."):
            S = resolve_section(rname, rom)
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
    print(f"{FN}: UNVERIFIABLE — {len(uniq)} symbol(s) have no known address: "
          + ", ".join(f"{n} ({section_why[n]})" if n in section_why else n for n in uniq[:8]))
    sys.exit(UNVERIFIABLE)

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
    placed = "".join(f"; {n} at 0x{a:08x}" for n, a in sorted(section_addr.items()))
    print(f"{FN}: BYTE IDENTICAL TO ROM ✅ ({len(resolved)}/{len(resolved)} words, {nrel} relocs resolved{placed})")
    sys.exit(MATCH)

print(f"{FN}: DIFFERS ❌ — {len(bad)}/{len(resolved)} words differ from the ROM")
for va, w, rw in bad[:40]:
    print(f"  0x{va:08x}: built {w:08x}   rom {rw:08x}")
if len(bad) > 40:
    print(f"  ... and {len(bad)-40} more")
sys.exit(DIFFERS)
PY

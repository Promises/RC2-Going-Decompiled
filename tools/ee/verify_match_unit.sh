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
# SELFTEST (task #1004; until then it had none, as task #1000 recorded here).
#   verify_match_unit.sh --selftest
# runs THIS script's normal mode on committed subjects with BASE (arg 2) seeds,
# and then on 15 one-line mutants of its own normal-mode code, and prints
# `#### VMU-SELFTEST usa: PASS|FAIL`. Exit 0 PASS, 1 FAIL, 2 CANNOT RUN (fixture
# assembly failed, or the usa flat ROM is unreachable). It never prints a
# `<fn>: BYTE IDENTICAL|DIFFERS|UNVERIFIABLE` line: those are verdicts on a
# function and a selftest is not one (checked by its own output guard).
# Needs the colima-ee-x86 VM and the usa flat ROM, as the normal mode does.
#
# Subjects: tools/ee/verify_match_unit_selftest/*.s — ROM words written as
# `.word`, relocated words as instructions with address-encoded names (D_/func_),
# assembled into gitignored build/ at run time. What keeps them from rotting:
# they depend on no symbol_addrs row, no splat .s, no src/ C and no build
# object, only on the ROM (CRC-pinned) and GAS. What would break them: a change
# to how resolve() reads an address-encoded name, a new ee-build GAS that orders
# or encodes REL relocations differently, or a different ROM. Each breaks a row
# loudly (FAIL or CONTROL INVALID), not silently: every unseeded row demands
# an exact word count, every seeded row an exact set of differing addresses.
#
# Seed table (every seed is guarded: it must sit on the relocation it names,
# must change the word, and the carry subject must really carry):
#   SelectSceneSubChunk 0x294920   unseeded rc 0 20/20 (RULING #8014 known
#     answer, the LO16 is not the next instruction); w0 bit flip, lui+1 (FACT
#     #8414), LO16^0x100, LO16=0x01D0 (%lo exactly 0x8000), LO16=0x8990
#     (negative addend) and jal+1 each rc 1 at the exact word(s); .rel.text
#     nested pair rc 0 (an INVARIANCE row: kept only because mutant M4 moves it);
#     HI16 re-pointed at a symbol with no later LO16 rc 2 "unresolvable
#     HI16/LO16 pairing"; R_MIPS_26 retyped GOT16 rc 2; a two-function arg 3 rc 3
#   EvaluateProgressCondition 0x29E808   rc 0 86/86 `.rodata at 0x0026ca70`
#     (task #966); w0 and w8 (%lo(.rodata)) rc 1; jump-table word +4 rc 2
#   GetSavePromptPending 0x2897A8   GPREL16 rc 0 2/2; %gp_rel+4 rc 1
#   __divdi3 slice 0x11FD98   rc 2: its .rodata (__clz_tab) is in the ROM 4 times
# Mutants it must reject (measured, each by the row named): M1 HI16 ignores the
# paired LO16 (the pre-#977 rule; must reproduce BOTH known answers: A0 rc 1
# `built 3c10001b rom 3c10001c`, and A2 rc 0, the #8414 false MATCH), M2 LO16
# addend unsigned, M3 section symbols unplaced (pre-#966), M4 pairing on any
# symbol, M5 +0x7FFF rounding, M6 unpaired HI16 given lo 0, M7 unmodelled type
# skipped, M8 compare blinded, M9 R_MIPS_26 addend dropped, M10 GPREL16 addend
# dropped, M11 _gp off by 4, M12 LO16 addend dropped, M13 section R_MIPS_32 left
# unresolved, M14 first-of-several ROM hits taken, M15 DIFFERS exiting 0.
# A mutant whose text is no longer in the code is a FAIL, not a skip.
# NOT covered: R_MIPS_PC16 (no subject carries one), the zero-run elision and
# unit-position blind spots below (the tool cannot see them, so no seed can
# move its verdict), symbol_addrs lookup (subjects use address-encoded names),
# EU (no EU subject). A wrong rule not in the mutant list has not been tried.
# ⚠️ A normal-mode python crash exits 1, which is the DIFFERS band.
# Seed the BASE (arg 2), never the target (arg 3): arg 3's bytes are never
# compared (FACT ledger-26262), so a target-seeded control cannot fail.
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
  echo "       $(basename "$0") --selftest" >&2
  exit 3
}

# ---- --selftest (task #1004). Everything the normal mode does is below this
# block and untouched by it; this block only runs on a lone `--selftest`.
if [ "${1:-}" = "--selftest" ]; then
  [ $# -eq 1 ] || usage
  SELF="$(cd "$(dirname "$0")" && pwd)/$(basename "$0")"
  ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"
  SELF="$SELF" python3 - <<'SELFTEST_PY'
import os, re, struct, subprocess, sys

SELF = os.environ["SELF"]
FIX = "tools/ee/verify_match_unit_selftest"
OUT = "going-decompiled/build/vmu_selftest.%d" % os.getpid()   # gitignored, inside the docker mount
DOCKER = ["docker", "--context", "colima-ee-x86", "run", "--rm", "-v", os.getcwd() + ":/work",
          "-w", "/work", "ee-build", "sh", "-c"]
AS = "mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL -G0"
# A line this tool's normal mode prints as a per-function verdict. The selftest
# prints NONE of these (checked at the end); the inner runs' lines are parsed,
# never echoed except behind "    | ".
VERDICT_RE = re.compile(r"^[A-Za-z_.$][\w.$]*: (BYTE IDENTICAL|DIFFERS|UNVERIFIABLE|NOT FOUND)")
printed = []

def say(s=""):
    printed.append(s)
    print(s, flush=True)

# ---- the base objects, assembled from the committed fixtures ----------------
# a  = SelectSceneSubChunk alone              (its own target)
# b  = EvaluateProgressCondition + GetSavePromptPending, one .text, one .rodata
# be = EvaluateProgressCondition alone        (target for b's first function)
# bg = GetSavePromptPending alone             (target for b's second function)
# c  = the __divdi3 slice + __clz_tab         (its own target)
OBJS = {"a": ["select_scene_sub_chunk"], "b": ["evaluate_progress_condition", "get_save_prompt_pending"],
        "be": ["evaluate_progress_condition"], "bg": ["get_save_prompt_pending"], "c": ["divdi3_clz_slice"]}

def obj(k):
    return "%s/%s.o" % (OUT, k)

def build():
    os.makedirs(OUT, exist_ok=True)
    cmds = []
    for k, parts in OBJS.items():
        with open("%s/%s.s" % (OUT, k), "w") as f:
            for p in parts:
                f.write(open("%s/%s.s" % (FIX, p)).read() + "\n")
        cmds.append("%s -o %s %s/%s.s" % (AS, obj(k), OUT, k))
    p = subprocess.run(DOCKER + [" && ".join(cmds)], capture_output=True, text=True)
    return p.returncode == 0 and all(os.path.isfile(obj(k)) for k in OBJS), p.stderr.strip()[-400:]

# ---- a minimal ELF32 LE reader, independent of the tool's own ---------------
# Written separately on purpose: a defect in the tool's load_elf/hi16_pairs
# must not also disable the guard that checks the seed is what it claims.
class Elf:
    def __init__(self, data):
        self.d = bytearray(data)
        shoff, = struct.unpack_from("<I", self.d, 0x20)
        n, shstr = struct.unpack_from("<HH", self.d, 0x30)
        self.secs = [struct.unpack_from("<10I", self.d, shoff + i * 40) for i in range(n)]
        so = self.secs[shstr][4]
        self.names = [self.d[so + s[0]:self.d.index(b"\0", so + s[0])].decode() for s in self.secs]
        st = next(i for i, s in enumerate(self.secs) if s[1] == 2)          # SHT_SYMTAB
        self.syms = []
        for k in range(self.secs[st][5] // 16):
            nm, val, size, info, _, shndx = struct.unpack_from("<IIIBBH", self.d, self.secs[st][4] + k * 16)
            stro = self.secs[self.secs[st][6]][4]
            name = self.names[shndx] if info & 0xF == 3 else self.d[stro + nm:self.d.index(b"\0", stro + nm)].decode()
            self.syms.append((name, val, info & 0xF))
    def sec(self, name):
        return self.names.index(name)
    def fn_value(self, fn):
        return next(v for n, v, t in self.syms if n == fn and t == 2)
    def rel(self, secname):
        """(file offset of the table, [(r_offset, type, symbol index)]) of SHT_REL for secname."""
        i = self.sec(secname)
        r = next(s for s in self.secs if s[1] == 9 and s[7] == i)
        return r[4], [(o, inf & 0xFF, inf >> 8) for o, inf in
                      (struct.unpack_from("<II", self.d, r[4] + k * 8) for k in range(r[5] // 8))]
    def write_rel(self, secname, table):
        base, _ = self.rel(secname)
        for k, (o, t, s) in enumerate(table):
            struct.pack_into("<II", self.d, base + k * 8, o, (s << 8) | t)
    def word(self, secname, off):
        return struct.unpack_from("<I", self.d, self.secs[self.sec(secname)][4] + off)[0]
    def put(self, secname, off, w):
        struct.pack_into("<I", self.d, self.secs[self.sec(secname)][4] + off, w & 0xFFFFFFFF)

TYPES = {"R_MIPS_32": 2, "R_MIPS_26": 4, "R_MIPS_HI16": 5, "R_MIPS_LO16": 6, "R_MIPS_GPREL16": 7}

def reloc_at(e, off, rtype, sym):
    return any(o == off and t == TYPES[rtype] and e.syms[s][0] == sym for o, t, s in e.rel(".text")[1])

def s16(v):
    return v - 0x10000 if v & 0x8000 else v

# ---- seeds. Each returns (Elf, None) or (None, why the seed is not a valid control)
def w_seed(fn, off, how, need=None):
    """Rewrite the .text word at fn+off. `need` = (type, symbol) the word must
    carry, or None = it must carry no relocation (so the flip is a plain word)."""
    def seed(e):
        at = e.fn_value(fn) + off
        here = [(t, e.syms[s][0]) for o, t, s in e.rel(".text")[1] if o == at]
        if need is None and here:
            return None, "+0x%x carries a relocation" % off
        if need is not None and not reloc_at(e, at, *need):
            return None, "+0x%x does not carry %s %s" % (off, need[0], need[1])
        old = e.word(".text", at)
        e.put(".text", at, how(old))
        if e.word(".text", at) == old:
            return None, "the seed leaves +0x%x unchanged" % off
        return e, None
    return seed

def rodata_seed(off, how):
    def seed(e):
        old = e.word(".rodata", off)
        e.put(".rodata", off, how(old))
        return (e, None) if e.word(".rodata", off) != old else (None, "unchanged")
    return seed

def lo_imm(v):
    return lambda w: (w & 0xFFFF0000) | v

def ld_pair(table, i):
    """Index of the LO16 GNU ld pairs with the HI16 at table[i] (next LO16, same symbol)."""
    return next((j for j in range(i + 1, len(table)) if table[j][1] == 6 and table[j][2] == table[i][2]), None)

def hi_of(e, table, i, j):
    S = int(e.syms[table[i][2]][0][-8:], 16)            # fixtures use address-encoded names
    return (S + (s16(e.word(".text", table[i][0]) & 0xFFFF) << 16)
            + s16(e.word(".text", table[j][0]) & 0xFFFF) + 0x8000) >> 16 & 0xFFFF

def nest_seed(fn, hi_a, hi_b):
    """.rel.text only: move the HI16 at fn+hi_b and its LO16 to sit right after
    the HI16 at fn+hi_a, giving HI(a) HI(b) LO(b) LO(a). ld pairs by symbol, so
    the linked bytes do not change; a rule pairing with the next LO16 on ANY
    symbol does. Valid only if that wrong LO16 gives HI(a) a different %hi."""
    def seed(e):
        v = e.fn_value(fn)
        _, t = e.rel(".text")
        ia = next(i for i, r in enumerate(t) if r[0] == v + hi_a and r[1] == 5)
        ib = next(i for i, r in enumerate(t) if r[0] == v + hi_b and r[1] == 5)
        ja, jb = ld_pair(t, ia), ld_pair(t, ib)
        if None in (ja, jb) or t[ia][2] == t[ib][2]:
            return None, "no two HI16/LO16 pairs on different symbols"
        if hi_of(e, t, ia, ja) == hi_of(e, t, ia, jb):
            return None, "LO(b) gives HI(a) the same %hi, so a wrong pairing is invisible"
        moved = [t[ib], t[jb]]
        rest = [r for k, r in enumerate(t) if k not in (ib, jb)]
        k = rest.index(t[ia]) + 1
        e.write_rel(".text", rest[:k] + moved + rest[k:])
        return e, None
    return seed

def repoint_seed(fn, hi_off, to_sym):
    """.rel.text only: point the HI16 at fn+hi_off at `to_sym`, which must have no
    later LO16 in the table, so ld would have no addend to pair it with."""
    def seed(e):
        v = e.fn_value(fn)
        _, t = e.rel(".text")
        i = next(i for i, r in enumerate(t) if r[0] == v + hi_off and r[1] == 5)
        s = next(k for k, sy in enumerate(e.syms) if sy[0] == to_sym)
        t[i] = (t[i][0], 5, s)
        if ld_pair(t, i) is not None:
            return None, "%s has a later LO16, so the HI16 is still paired" % to_sym
        e.write_rel(".text", t)
        return e, None
    return seed

def retype_seed(fn, off, frm, to):
    def seed(e):
        v = e.fn_value(fn)
        _, t = e.rel(".text")
        hit = [i for i, r in enumerate(t) if r[0] == v + off and r[1] == frm]
        if len(hit) != 1:
            return None, "no single relocation of type %d at +0x%x" % (frm, off)
        t[hit[0]] = (t[hit[0]][0], to, t[hit[0]][2])
        e.write_rel(".text", t)
        return e, None
    return seed

def carries(fn, hi_off, lo_off):
    """Guard for the unseeded carry row: the HI16 at fn+hi_off really needs the
    addend of the LO16 at fn+lo_off, i.e. dropping it gives a different %hi."""
    def check(e):
        v = e.fn_value(fn)
        _, t = e.rel(".text")
        i = next((i for i, r in enumerate(t) if r[0] == v + hi_off and r[1] == 5), None)
        j = ld_pair(t, i) if i is not None else None
        if j is None or t[j][0] != v + lo_off:
            return "the HI16 at +0x%x is not ld-paired with the LO16 at +0x%x" % (hi_off, lo_off)
        S = int(e.syms[t[i][2]][0][-8:], 16)
        if hi_of(e, t, i, j) == (S + (s16(e.word(".text", t[i][0]) & 0xFFFF) << 16) + 0x8000) >> 16 & 0xFFFF:
            return "the HI16 at +0x%x does not carry" % hi_off
        return None
    return check

# ---- THE SEED TABLE. (id, fn, base, target, seed or None, guard or None,
#      expected rc, expected detail). Detail: rc 0 -> (words, substring or None);
#      rc 1 -> the EXACT tuple of differing vaddrs; rc 2/3 -> a substring.
A, B, C, D = "func_00294920", "func_0029E808", "func_002897A8", "func_0011FD98"
ROWS = [
    # SelectSceneSubChunk: HI16/LO16 pairing, both directions of the carry defect.
    ("A0", A, "a", "a", None, carries(A, 0x0C, 0x14), 0, (20, None),
     "unseeded: %hi carries through the paired LO16 (RULING #8014 known answer)"),
    ("A1", A, "a", "a", w_seed(A, 0x00, lambda w: w ^ 1), None, 1, (0x294920,),
     "+0x00 bit 0 flipped (plain word)"),
    ("A2", A, "a", "a", w_seed(A, 0x0C, lambda w: w + 1, ("R_MIPS_HI16", "D_001B7E30")), None, 1, (0x29492C,),
     "+0x0c lui immediate +1 (FACT #8414: the old rule said MATCH)"),
    ("A3", A, "a", "a", w_seed(A, 0x14, lambda w: w ^ 0x100, ("R_MIPS_LO16", "D_001B7E30")), None, 1, (0x294934,),
     "+0x14 paired LO16 immediate ^ 0x100"),
    ("A4", A, "a", "a", w_seed(A, 0x14, lo_imm(0x01D0), ("R_MIPS_LO16", "D_001B7E30")), None, 1, (0x294934,),
     "+0x14 LO16 -> 0x01D0: %lo is exactly 0x8000, %hi still 0x1C"),
    ("A5", A, "a", "a", w_seed(A, 0x14, lo_imm(0x8990), ("R_MIPS_LO16", "D_001B7E30")), None, 1,
     (0x29492C, 0x294934), "+0x14 LO16 -> 0x8990: a NEGATIVE addend, %hi drops to 0x1B"),
    ("A6", A, "a", "a", nest_seed(A, 0x0C, 0x2C), None, 0, (20, None),
     ".rel.text: D_001B2230 pair nested inside the D_001B7E30 pair (ld: no change)"),
    ("A7", A, "a", "a", repoint_seed(A, 0x0C, "func_002945E0"), None, 2, "unresolvable HI16/LO16 pairing",
     ".rel.text: HI16 at +0x0c re-pointed at a symbol with no later LO16"),
    ("A8", A, "a", "a", retype_seed(A, 0x20, 4, 9), None, 2, "unmodelled relocation type(s): R_MIPS_GOT16",
     ".rel.text: the jal's R_MIPS_26 retyped R_MIPS_GOT16"),
    ("A9", A, "a", "a", w_seed(A, 0x20, lambda w: w + 1, ("R_MIPS_26", "func_002945E0")), None, 1, (0x294940,),
     "+0x20 jal in-place target +1 (the R_MIPS_26 addend)"),
    ("A10", A, "a", "b", None, None, 3, "WHOLE-UNIT object",
     "argument 3 is a two-function object"),
    # EvaluateProgressCondition: a unit-local .rodata section symbol (task #966).
    ("B0", B, "b", "be", None, None, 0, (86, ".rodata at 0x0026ca70"),
     "unseeded: %hi/%lo(.rodata) placed by content"),
    ("B1", B, "b", "be", w_seed(B, 0x00, lambda w: w ^ 1), None, 1, (0x29E808,),
     "+0x00 (w0) bit 0 flipped"),
    ("B2", B, "b", "be", w_seed(B, 0x20, lambda w: w + 1, ("R_MIPS_LO16", ".rodata")), None, 1, (0x29E828,),
     "+0x20 (w8) %lo(.rodata) immediate +1"),
    ("B3", B, "b", "be", rodata_seed(0x00, lambda w: w + 4), None, 2, "found nowhere in the ROM",
     ".rodata jump-table entry 0 +4: placement must refuse, not guess"),
    # GetSavePromptPending: GPREL16 against the usa _gp.
    ("C0", C, "b", "bg", None, None, 0, (2, None), "unseeded: %gp_rel with _gp 0x1AEFF0"),
    ("C1", C, "b", "bg", w_seed(C, 0x04, lambda w: w + 4, ("R_MIPS_GPREL16", "D_001A7B94")), None, 1,
     (0x2897AC,), "+0x04 %gp_rel immediate +4"),
    # The __divdi3 slice: genuinely undecidable, must STAY rc 2.
    ("D0", D, "c", "c", None, None, 2, "found more than once in the ROM",
     "unseeded: __clz_tab is in the ROM 4 times"),
]

def run(script, row, base):
    rid, fn, _b, tgt = row[:4]
    p = subprocess.run(["bash", script, fn, base, obj(tgt), "usa"], capture_output=True, text=True, timeout=600)
    out = (p.stdout + p.stderr).strip()
    diffs = tuple(int(m, 16) for m in re.findall(r"^  0x([0-9a-f]{8}): built", out, re.M))
    m = re.search(r"\((\d+)/(\d+) words", out)
    return p.returncode, out, diffs, (int(m.group(1)) if m and m.group(1) == m.group(2) else None)

def holds(row, got):
    rc, out, diffs, words = got
    want_rc, want = row[6], row[7]
    if rc != want_rc:
        return False
    if rc == 0:
        return words == want[0] and (want[1] is None or want[1] in out)
    if rc == 1:
        return diffs == want
    return want in out

def show(got):
    rc, out, diffs, words = got
    if rc == 0 and words is not None:
        return "rc 0, %s/%s words" % (words, words)
    if rc == 1 and diffs:
        return "rc 1 at " + ",".join("0x%08x" % d for d in diffs)
    return "rc %d, %s" % (rc, out.splitlines()[0].split(" — ", 1)[-1][:90] if out else "")

bases = {}

def base_for(row):
    rid, seed, guard = row[0], row[4], row[5]
    if rid in bases:
        return bases[rid]
    e = Elf(open(obj(row[2]), "rb").read())
    why = guard(e) if guard else None
    if why is None and seed is not None:
        e, why = seed(e)
    if why is not None:
        bases[rid] = (None, why)
        return bases[rid]
    path = obj(row[2]) if seed is None else "%s/seed_%s.o" % (OUT, rid)
    if seed is not None:
        open(path, "wb").write(bytes(e.d))
    bases[rid] = (path, None)
    return bases[rid]

# ---- MUTANTS: the meta-control. Each is a one-line change to THIS script's
# normal-mode code (the `python3 - <<'PY'` block), written to a copy beside it.
# The seed table must reject every one: some row must deviate. A mutant whose
# text is no longer found is a FAIL (MUTANT INAPPLICABLE), so a refactor of the
# tool cannot silently retire a control. `must` = exact outcomes a mutant has
# to produce (the historical known answers), beyond "some row deviates".
MUTANTS = [
    ("M1", "HI16 ignores the paired LO16 addend (the rule before task #977)",
     "A = (sign16(w & 0xFFFF) << 16) + lo", "A = (sign16(w & 0xFFFF) << 16)", ["A0", "A2"],
     [("A0", 1, (0x29492C,), "built 3c10001b   rom 3c10001c"), ("A2", 0, None, None)]),
    ("M2", "paired LO16 addend read unsigned",
     "pairs[r_off] = sign16(lo_w & 0xFFFF)", "pairs[r_off] = lo_w & 0xFFFF", ["A5"], []),
    ("M3", "section symbols not placed (the rule before task #966)",
     "S = resolve_section(rname, rom)", "S = None", ["B0"], []),
    ("M4", "HI16 pairs with the next LO16 on ANY symbol",
     "if lo_info & 0xFF == 6 and lo_info >> 8 == r_info >> 8:", "if lo_info & 0xFF == 6:", ["A6"], []),
    ("M5", "%hi rounded with +0x7FFF, not +0x8000",
     "(((S + A + 0x8000) >> 16) & 0xFFFF)", "(((S + A + 0x7FFF) >> 16) & 0xFFFF)", ["A4"], []),
    ("M6", "an unpaired HI16 resolved with a zero LO16 instead of refused",
     "lo = HI16_PAIRS.get(off) if isinstance(HI16_PAIRS, dict) else None",
     "lo = (HI16_PAIRS.get(off) if isinstance(HI16_PAIRS, dict) else None) or 0", ["A7"], []),
    ("M7", "an unmodelled relocation type skipped silently",
     "unmodelled.add(rtype)", "pass", ["A8"], []),
    ("M8", "the ROM compare blinded",
     "if rw != w:", "if rw != w and False:", ["A1"], []),
    ("M9", "R_MIPS_26 in-place addend dropped",
     "A = (w & 0x03FFFFFF) << 2", "A = 0", ["A9"], []),
    ("M10", "GPREL16 in-place addend dropped",
     "((S + sign16(w & 0xFFFF) - GP) & 0xFFFF)", "((S - GP) & 0xFFFF)", ["C1"], []),
    ("M11", "the usa _gp off by 4",
     '"usa": 0x1AEFF0', '"usa": 0x1AEFF4', ["C0"], []),
    ("M12", "LO16 in-place addend dropped",
     "((S + sign16(w & 0xFFFF)) & 0xFFFF)", "((S) & 0xFFFF)", ["A0"], []),
    ("M13", "a section's own R_MIPS_32 relocations left unresolved before the ROM search",
     "(S + A) & 0xFFFFFFFF)", "(A) & 0xFFFFFFFF)", ["B0"], []),
    ("M14", "section placement takes the FIRST of several ROM hits",
     "if len(hits) != 1:", "if not hits:", ["D0"], []),
    ("M15", "DIFFERS printed but exit status 0",
     "sys.exit(DIFFERS)", "sys.exit(MATCH)", ["A1"], []),
]

def main_span(text):
    start = text.rfind("python3 - <<'" + "PY'\n")
    end = text.find("\n" + "PY\n", start)
    return start, end

def mutate(text, old, new):
    s, e = main_span(text)
    if s < 0 or e < 0:
        return None, "normal-mode heredoc not found"
    n = text[s:e].count(old)
    if n != 1:
        return None, "text found %d times in the normal-mode code, expected once" % n
    return text[:s] + text[s:e].replace(old, new) + text[e:], None

# ---- run ------------------------------------------------------------------
say("== verify_match_unit --selftest: an INSTRUMENT check, NOT a function verdict")
say("== subjects: %s/*.s assembled now; ROM words from the flat usa ROM" % FIX)
ok = True
try:
    built, err = build()
    if not built:
        say("SELFTEST CANNOT RUN: fixture assembly failed in the ee-build container: " + err)
        sys.exit(2)
    say("-- seed table (BASE = arg 2 seeded, never the target), %d rows" % len(ROWS))
    result, first = {}, {}
    for row in ROWS:
        path, why = base_for(row)
        if path is None:
            say("  CONTROL INVALID  %-4s %s: %s" % (row[0], row[8], why))
            ok = False
            continue
        got = run(SELF, row, path)
        if row[0] == "A0" and got[0] == 2 and "flat ROM" in got[1]:
            say("SELFTEST CANNOT RUN: the usa flat ROM is not reachable from this checkout")
            sys.exit(2)
        result[row[0]] = got
        good = holds(row, got)
        ok &= good
        say("  %-5s %-4s %-72s -> %s" % ("ok" if good else "FAIL", row[0], row[8][:72], show(got)))
        if not good:
            for l in got[1].splitlines()[:6]:
                say("    | " + l)
    # every seeded row must MOVE its subject's verdict away from the unseeded
    # row's, unless a mutant is shown below to move it (an invariance row).
    unseeded = {}
    for r in ROWS:
        if r[4] is None:
            unseeded.setdefault(r[1], r)            # a subject's first unseeded row
    invariance = [r[0] for r in ROWS if r[4] is not None and r[1] in unseeded
                  and (r[6], r[7]) == (unseeded[r[1]][6], unseeded[r[1]][7])]

    say("-- meta-control: %d mutants of this script's own normal-mode code; each must be REJECTED" % len(MUTANTS))
    text = open(SELF).read()
    caught_by = {}
    for mid, desc, old, new, prio, must in MUTANTS:
        mtext, why = mutate(text, old, new)
        if mtext is None:
            say("  FAIL  %-4s MUTANT INAPPLICABLE (%s): %s" % (mid, desc, why))
            ok = False
            continue
        mpath = os.path.join(os.path.dirname(SELF), ".vmu_selftest.%d.%s.sh" % (os.getpid(), mid))
        open(mpath, "w").write(mtext)
        try:
            dev, mres = [], {}
            order = [r for r in ROWS if r[0] in prio] + [r for r in ROWS if r[0] not in prio]
            for row in order:
                path, why = bases[row[0]]
                if path is None:
                    continue
                g = run(mpath, row, path)
                mres[row[0]] = g
                if not holds(row, g):
                    dev.append(row[0])
                    caught_by.setdefault(row[0], []).append(mid)
                if dev and all(p in mres for p in prio) and all(m[0] in mres for m in must):
                    break
            bad_must = []
            for rid, rc, diffs, sub in must:
                g = mres.get(rid)
                if g is None or g[0] != rc or (diffs is not None and g[2] != diffs) or (sub and sub not in g[1]):
                    bad_must.append("%s gave %s" % (rid, show(g) if g else "not run"))
            good = bool(dev) and not bad_must
            ok &= good
            say("  %-5s %-4s %-60s -> %s" % ("ok" if good else "FAIL", mid, desc[:60],
                ("rejected by " + ", ".join("%s (%s)" % (r, show(mres[r])) for r in dev[:3])) if dev
                else "PASSED EVERY ROW: the seed table cannot see this class"))
            for b in bad_must:
                say("        known answer not reproduced: " + b)
            for rid, rc, diffs, sub in must:
                if rid in mres and not bad_must:
                    say("        known answer: %s -> %s" % (rid, show(mres[rid])))
        finally:
            os.remove(mpath)
    for rid in invariance:
        if rid in caught_by:
            say("  ok    %-4s does not move the verdict by design; it rejects %s" % (rid, ", ".join(caught_by[rid])))
        else:
            say("  FAIL  %-4s moves no verdict and rejects no mutant: decoration, not a control" % rid)
            ok = False
    # the output guard: the selftest must not print a per-function verdict line.
    a0 = result.get("A0")
    if not (a0 and VERDICT_RE.match(a0[1].splitlines()[0])):
        say("  FAIL  output guard: its pattern does not match a real verdict line (A0), so it checks nothing")
        ok = False
    leaked = [l for l in printed if VERDICT_RE.match(l) or "landing_gate" in l]
    if leaked:
        say("  FAIL  output guard: the selftest printed %d verdict-shaped line(s)" % len(leaked))
        ok = False
    else:
        say("  ok    output guard: no line above has the shape '<fn>: BYTE IDENTICAL|DIFFERS|UNVERIFIABLE'")
finally:
    for f in os.listdir(OUT) if os.path.isdir(OUT) else []:
        os.remove(os.path.join(OUT, f))
    if os.path.isdir(OUT):
        os.rmdir(OUT)
print("#### VMU-SELFTEST usa: %s (instrument check, not a function verdict)" % ("PASS" if ok else "FAIL"))
sys.exit(0 if ok else 1)
SELFTEST_PY
  exit $?
fi

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
#    (StreamSceneSegment read DIFFERS 1/34 at 0x29455c both ways then: the HI16
#    carry bug of FACT #8027, fixed at HI16/LO16 PAIRING below, not a pad
#    effect.) Only a unit-level check sees the pad drop:
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

# HI16/LO16 PAIRING. A REL HI16's addend is not its own immediate alone: it is
# AHL = (hi_imm << 16) + sign16(lo_imm) of the LO16 it pairs with. Resolving
# the HI16 from hi_imm alone loses the carry whenever the low half of the
# addend pushes %lo past 0x7FFF: `lui %hi(g_cameraSlotActive + 0x990)` carries
# its whole 0x990 in the addiu, so it came out 0x1B where the ROM has 0x1C
# (FACT #8027, RULING #8014; 32 USA functions read a false DIFFERS on their own
# ROM asm, task #977).
# The pairing is the linker's, taken from the object's own .rel.text in TABLE
# order, not from the instruction stream: GNU ld pairs a HI16 with the NEXT
# LO16 relocation in the table carrying the same symbol index
# (mips_elf_next_relocation in elfxx-mips.c). That covers a LO16 that is not
# the next instruction, several HI16s sharing one LO16, and a LO16 past the end
# of the function, and it is the rule whose output landing_gate.sh's whole-image
# cmp 0 measures against the ROM. A HI16 with no such LO16 is refused as
# UNVERIFIABLE below: ld refuses to link it too, so there is no value to check.
def hi16_pairs():
    """{HI16 r_offset: sign16(paired lo_imm) or None if unpaired}, or a reason
    string when the object's .text relocations cannot be read."""
    if ELF is None:
        return "base is not an ELF32 LE object"
    d, secs = ELF
    texts = [s for s in secs if s["name"] == ".text"]
    if len(texts) != 1:
        return f"{len(texts)} sections named .text in the base object"
    text = texts[0]
    rels = [s for s in secs if s["type"] == 9 and secs[s["info"]] is text]
    if len(rels) != 1:
        return f"{len(rels)} SHT_REL sections for .text in the base object"
    rel = rels[0]
    table = [struct.unpack_from("<II", d, rel["off"] + k * 8) for k in range(rel["size"] // 8)]
    pairs = {}
    for i, (r_off, r_info) in enumerate(table):
        if r_info & 0xFF != 5:                 # R_MIPS_HI16
            continue
        pairs[r_off] = None
        for lo_off, lo_info in table[i + 1:]:
            if lo_info & 0xFF == 6 and lo_info >> 8 == r_info >> 8:   # LO16, same symbol
                lo_w, = struct.unpack_from("<I", d, text["off"] + lo_off)
                pairs[r_off] = sign16(lo_w & 0xFFFF)
                break
    return pairs

HI16_PAIRS = hi16_pairs()
unpaired = []

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
            # AHL = own immediate << 16 plus the paired LO16's sign-extended
            # immediate (HI16/LO16 PAIRING above); the +0x8000 is the carry the
            # sign-extended %lo needs.
            lo = HI16_PAIRS.get(off) if isinstance(HI16_PAIRS, dict) else None
            if lo is None:
                why = HI16_PAIRS if isinstance(HI16_PAIRS, str) else "no later LO16 on the same symbol in .rel.text"
                unpaired.append(f"0x{va:08x} {rname} ({why})")
                continue
            A = (sign16(w & 0xFFFF) << 16) + lo
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
if unpaired:
    print(f"{FN}: UNVERIFIABLE — unresolvable HI16/LO16 pairing at {len(unpaired)} HI16(s): "
          + ", ".join(unpaired[:8]))
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

#include "common.h"

/* Local 16-byte VU-lane vector for the structure-exact #else bodies (common.h
   doesn't pull in vec.h). Byte-neutral: a typedef emits no code. Unconditional
   (was #ifdef TARGET_NATIVE) so a MATCH_<fn>-promoted body can be compiled on
   the engine96 arm (task #468). */
typedef struct Vec4 { f32 x, y, z, w; } __attribute__((aligned(16))) Vec4;

/*
 * text/1CA080 — front-end / pause-menu screens band, part A (vaddr
 * 0x2CA100..0x2D553F). Carved out of the big text/1B21E8 asm tile as
 * ranked-carve-pipeline pick #5 ("menu-screens"); part B is text/1D54C0.
 *
 * Built at -O2 -G8 -fno-gcse (see the per-unit GFLAG/CC1EXTRA overrides in
 * tools/ee/objdiff_build.sh / diff.sh / build.sh): these gameplay/UI TUs were
 * compiled by the later SN cc1 without the load-PRE pass (-fno-gcse reproduces
 * it) and use the -G8 small-data + assembler-absolute extern model.
 *
 * -G8 extern-sizing model for this file (same as text/198FA0 / text/1907F0):
 *   - a complete extern object of size <= 8 bytes goes in small data
 *     (gp-relative);
 *   - an object the original reads with the ADJACENT lui/%lo "assembler macro"
 *     shape gets a `.extern sym,16` override so cc1 emits the one-insn
 *     symbolic macro (matching the SN prologue scheduling) and GNU as expands
 *     it absolutely.
 *
 * Walls left as INCLUDE_ASM (documented per function): the 8-byte-packed
 * callee-save wall (every function saving 2+ GPRs — the later cc1 packs save
 * slots 8-byte vs our 16-byte), switch/jtbl functions (splat jtbl reloc gap),
 * and assorted register-coloring / reload-artifact near-misses.
 *
 * Task #468 (2026-09-20) promotion sweep — every #else arm measured on BOTH the
 * engine96 arm (MATCH_<fn> guards, cc1 2.96-001003-1) and, as plain C, on this
 * unit's 2.9 arm; each arm carries its two percentages, residual class and
 * first differing row. Unit-wide findings:
 *  - OBJECT-SIZING is the dominant lever, not the compiler: the ROM addresses
 *    ~30 small scalars with a compiler-split lui/%lo pair (ROM_SPLIT below)
 *    and ~25 more with the assembler macro (`.extern X, 16` list); fixing the
 *    declarations moved 35 arms up and produced the 4 new byte-exact matches
 *    (func_002D3388, func_002D3B88, func_002D4370, ListScrollerSelectNext —
 *    all on the 2.9 arm; the engine96 arm closed none).
 *  - The 2.9 arm is the better instrument for this TU: it reproduces the ROM's
 *    large-displacement split (& ~0x7fff), delay-slot fills, register choice
 *    for the split high half and block layout; 39 of the 75 remaining arms
 *    differ from the ROM on 2.9 ONLY by the 16-byte callee-save slots
 *    (PACKED-SAVE), which no 2.9 flag reproduces. The engine96 arm packs the
 *    slots but changes the high-half register (SPLIT-HIREG, 14 arms), the
 *    prologue/epilogue emission order (SCHED-TIEBREAK, FACT #7345), if-converts
 *    `if (c) r = 1` to sltu and `c ? -1 : 0` to movn where the ROM branches
 *    (IFCONV), and emits the large-displacement macro (BIGDISP-SPLIT).
 *  - g_guiInstance is absolute in 35 functions and gp-relative in 5 of this
 *    band (g_swapGadgetItemIndex 14/1, g_frameCounter 3/1): the band spans more
 *    than one original TU; one declaration cannot serve both.
 */

/* cc1-small / assembler-absolute symbols (see header). */
__asm__(".extern g_guiInstance, 16");
__asm__(".extern g_padButtonsPressed, 16");
__asm__(".extern D_1A7318, 16");
__asm__(".extern D_2617C0, 16");
__asm__(".extern D_0025BA70, 16");
__asm__(".extern D_1ABA2C, 16");
__asm__(".extern D_1AB9E4, 16");
__asm__(".extern g_miscExtras, 16");
__asm__(".extern D_1ABA84, 16");
__asm__(".extern D_1ABA88, 16");
__asm__(".extern D_1ABA8C, 16");
__asm__(".extern D_1ABA90, 16");
__asm__(".extern D_138180, 16");
/* func_002CDEB0 cheat-flag mirror: source byte flags + s16 menu mirrors. */
__asm__(".extern D_1A7BD1, 16");
__asm__(".extern D_1A7BD2, 16");
__asm__(".extern D_1A7BD3, 16");
__asm__(".extern D_1A7BD4, 16");
__asm__(".extern D_1A7BD6, 16");
__asm__(".extern D_1AA5A2, 16");
__asm__(".extern D_1AA5BA, 16");
__asm__(".extern D_1AA5D2, 16");
__asm__(".extern D_1AA5EA, 16");
__asm__(".extern D_1AA602, 16");
/* func_002D1850 extras-availability latch targets. */
__asm__(".extern g_lastMenuScreenId, 16");
__asm__(".extern D_1ABA50, 16");
__asm__(".extern D_1ABA54, 16");
__asm__(".extern D_1ABA58, 16");
__asm__(".extern D_1ABA4C, 16");
/* Task #468 (tools/ee/.t468/05_symbol_shapes.txt): the ROM addresses these
 * absolutely with the assembler-macro shape ($at store / destination-reuse load)
 * while a <=8-byte extern would go gp-relative here. A few are MIXED in the ROM
 * (gp-relative in a minority of this band's functions — g_swapGadgetItemIndex
 * 14 abs/1 gp, g_frameCounter 3/1, g_cinematicExitPending 3/2): the band spans
 * more than one original TU and the minority sites cannot be served by the
 * same declaration; the majority form is taken. */
__asm__(".extern D_1A7A10, 16");
__asm__(".extern D_1A7BF2, 16");
__asm__(".extern D_1A7BF4, 16");
__asm__(".extern D_1A7BFB, 16");
__asm__(".extern D_1A7C06, 16");
__asm__(".extern D_1A7C07, 16");
__asm__(".extern D_1A7C09, 16");
__asm__(".extern D_1A7C0A, 16");
__asm__(".extern D_1AB9DC, 16");
__asm__(".extern g_equippedArmor, 16");
__asm__(".extern g_nGameState, 16");
__asm__(".extern g_screenWidth, 16");
__asm__(".extern g_screenHeight, 16");
__asm__(".extern g_frameCounter, 16");
__asm__(".extern g_cinematicExitPending, 16");
__asm__(".extern g_swapGadgetItemIndex, 16");
__asm__(".extern D_1A7908, 16");
__asm__(".extern D_1A790C, 16");
__asm__(".extern D_1ABA71, 16");
__asm__(".extern D_1ABA72, 16");
__asm__(".extern D_1ABA73, 16");
__asm__(".extern D_1ABA74, 16");
__asm__(".extern D_1ABA75, 16");
__asm__(".extern D_1ABA76, 16");
__asm__(".extern D_1ABA77, 16");
__asm__(".extern g_cameraCallbackCount, 16");
__asm__(".extern g_nLevelExitRequested, 16");

/* ROM-split externs (task #468, tools/ee/.t468/05_symbol_shapes.txt): in the ROM
 * every reference to these is a compiler-split lui/%lo pair — the high half in
 * its own register, often with other instructions scheduled between — so the
 * ROM's compiler did not treat them as -G8 small data. For a <=8-byte extern
 * cc1 (2.9 and 2.96 alike) instead emits the one-instruction macro, which the
 * assembler can only expand through the destination register or $at. The
 * attribute takes the object out of cc1's small-data class so it splits the
 * address the way the ROM does. Declarations only: no code, no definition. */
#define ROM_SPLIT __attribute__((section(".data")))
extern s32 g_padButtonsPressed ROM_SPLIT;
extern s32 g_padButtonsHeld ROM_SPLIT;
extern s16 g_fileLoadState ROM_SPLIT;
extern u8 g_skillPointFlags ROM_SPLIT;
extern u8 *g_pNextMenuScreen ROM_SPLIT;
extern u8 *g_pCurrentMenuScreen ROM_SPLIT;
extern s32 g_nLevelSelectListCount ROM_SPLIT;
extern f32 g_cameraProjScale ROM_SPLIT;
extern s32 g_mapCurrentLevel ROM_SPLIT;
extern s32 g_lastMenuScreenId ROM_SPLIT;
extern s32 D_259E34 ROM_SPLIT;
extern s32 D_259C24 ROM_SPLIT;
extern s32 D_25E264 ROM_SPLIT;
extern s32 D_25E444 ROM_SPLIT;
extern s32 D_25C074 ROM_SPLIT;
extern s32 D_25B5D8 ROM_SPLIT;
extern s32 D_25C940 ROM_SPLIT;
extern s32 D_25C8C4 ROM_SPLIT;
extern s32 D_25CB30 ROM_SPLIT;
extern s32 D_25CABC ROM_SPLIT;
extern s32 D_25CCC8 ROM_SPLIT;
extern u8 *D_25C004 ROM_SPLIT;
extern u8 *D_25C1F0 ROM_SPLIT;
extern u8 *D_25C388 ROM_SPLIT;
extern u8 *D_25C520 ROM_SPLIT;
extern u8 *D_25C6B8 ROM_SPLIT;
extern u8 D_1395E9 ROM_SPLIT;
extern u8 D_2617C0 ROM_SPLIT;
extern u8 D_259CC0 ROM_SPLIT;
extern u8 D_2615D8 ROM_SPLIT;
extern u8 D_261678 ROM_SPLIT;
extern u8 D_261730 ROM_SPLIT;

/* Singleton GUI-manager instance (null until the GUI is up). The wrappers here
 * only ever forward `instance + fixed-widget-offset` to widget methods. */
extern char *g_guiInstance;

/* Per-level effect-def blob; the menu code reuses the slot at +0x100 as a
 * small front-end screen-state scratch struct. */
extern u8 g_particleFxBlob[];

/* Menu-screen manager block (== g_particleFxBlob + 0x100). The same scratch the
 * functions above reach via `g_particleFxBlob + 0x100`; the SN cc1 anchors it on
 * its own symbol. */
extern u8 g_menuScreenBlock[];

/* Selected catalog/list cursors (small-data scalars). */
extern s32 g_bestiaryCursor;     /* selected bestiary entry (1..0x3f) */
extern u8 g_pTextTableLoadBuf[]; /* active language text-table load buffer (also a base for menu screen-state words at +0xD8/+0x118) */

/* 2D draw-batch begin/end fence used by every menu draw function. */
extern void Begin2dDrawBatch(s32 mode);
extern void End2dDrawBatch(void);

/* Widget-method targets forwarded to by the g_guiInstance wrappers. */
extern void func_0033A7B8(char *widget, s32 arg);
extern void func_0033F3B8(char *widget, void *arg);

/* Per-screen draw/update helpers in the preceding text/1A00F0 asm band. */
extern void func_0029CF70(void);
extern void func_0029D010(void);
extern void func_0029DA58(void);
extern void func_0029D288(void);
extern void func_0029D448(void);
extern void func_0029D4B8(void);
extern void func_0029D568(void);
extern void func_0029D5D8(void);
extern void func_0029D798(void);
extern void func_0029D808(void);
extern void func_0029D6B8(void);
extern void func_0029D728(void);
extern void func_0029D3D8(void);
extern void func_0029D648(void);
extern void func_0029D878(void);

extern void MenuScreenLoad(void);

/* Menu sub-screen error/status latch (absolute %hi/%lo data word). */
extern s32 D_25CABC;

/* Pad button state + camera state used by a couple menu helpers. */
extern s32 g_padButtonsPressed;

/* Localized-string + 2D label draw helpers in adjacent text bands. */
extern char *GetLocalizedString(s32 id);
extern void func_00280090(s32 x, s32 y, u64 color, char *str, s64 wrap);
extern void func_0029D368(void);
extern void func_0029D478(s32 buttons);
extern s32 func_002D67A0(s32 a, void *b);

/* gp-relative + absolute label position words consumed by func_002D0240. */
extern s32 D_1ABA28;
extern s32 D_1ABA2C;
extern char D_0025BA70[];

/* Global input/UI flags object; the menu code reads the flag word at +0x1C4. */
extern u8 D_138180[];

/* Bestiary/list base-pointer pair selected by D_1A7318. */
extern s32 D_1A7318;
extern u8 D_1AB678[];
extern u8 D_1AB648[];

/* Widget-data blob forwarded by the func_002D4370 GUI wrapper. */
extern u8 D_2617C0;

/* Camera/projection scratch (g_sceneActorMobys + 0x674) + frame-matrix builders
 * (func_002CAFD8). */
extern u8 g_sceneActorMobys[];
extern void BuildCameraProjection(void);
extern void BuildFrameViewMatrices(void);
extern void func_002CABC0(void);

/* World camera default-pose targets (func_002CABC0). */
extern f32 g_cameraPos[4];     /* 0x1B52C0 - camera world position vec4 */
extern f32 g_cameraMatrix[12]; /* 0x1B54F0 - camera rotation matrix, 3 vec4 rows */

/* Twin of func_002D0240 for the alternate label slot (func_002CE9D8). */
extern void func_0029D2F8(void);
extern s32 D_1AB9E0;
extern s32 D_1AB9E4;

/* Extras-menu "new content" flags latched by func_002D2C60 when the GUI is up. */
extern u8 g_miscExtras;            /* master extras-unlocked byte */
extern s32 D_1AA450, D_1AA454, D_1AA458; /* per-feature availability (gp-rel) */
extern s32 D_1ABA80;               /* skill-points highlight flag (gp-rel) */
extern s32 D_1ABA84, D_1ABA88, D_1ABA8C, D_1ABA90; /* per-menu "new" flags (abs) */

/* Mis-split fragment: orphaned stack-pointer adjusts (addiu $sp / nops) with
 * no jr $ra — not a real function entry; left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA100);

/* func_002CA138: 3D vector-geometry helper (menu 3D cursor / pick math). Uses
 * the VU0 primitives Vec3CrossVu0 + Vec3DotVu0 + Vec3LengthVu0 +
 * Vec3RescaleToLenVu0 + Vec4Sub/AddVu0 and GetFloatAbs, with 128-bit lq/sq
 * vertex copies and five FP compares (c.lt.s/c.eq.s) driving bc1t/bc1f
 * branches. PARK (INCLUDE_ASM): high transcription risk per the blast-radius
 * guardrail — the dense VU0 vector math plus delay-slot-sensitive FP branches
 * make a hand-written structure-exact #else unverifiable without an EE oracle;
 * not worth a silent op-for-op error in coverage-only C. The matching arm is
 * byte-frozen regardless (later cc1 8-byte-packed saves + qword ops). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA138);

/* func_002CA3E8: nearest ray/segment-vs-polygon-edge intersection. Indexes an
 * edge-list table (D_001F0000 + 0x1680, arg0*4) whose header holds the edge
 * count, builds a normalized direction (Vec4SubVu0 + Vec3LengthVu0 +
 * Vec3RescaleToLenVu0), then loops the edges computing per-edge intersection
 * parameters (func_00283638/func_00283A48 helpers) and tracks the nearest hit
 * parameter + a hit flag returned in $v0. PARK (INCLUDE_ASM): high
 * transcription risk per the blast-radius guardrail — the per-edge FP
 * intersection test uses multiple c.eq.s/c.lt.s compares with delay-slot
 * bc1f/bc1t branches (non-likely branches run their delay slot; a misread is a
 * silent op-for-op bug), unverifiable without an EE oracle in coverage-only C.
 * Matching arm is byte-frozen (8-byte-packed saves + qword ops) regardless. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA3E8);

/* func_002CA618: sample a piecewise-linear Vec4 path at distance `dist`.
 * The path is a leading s32 count followed by a Vec4 array at +0x10 (0x10
 * stride). Segment index = floor(dist / segLen); the output index (*outSeg),
 * fractional remainder (*outFrac) and interpolated point (*outVec) are written.
 * Index is clamped to [0, count-1): below 0 -> segment 0, at/above the last
 * interpolable segment -> the last one, and in both clamp cases the frac is 0
 * and the point is the segment's start vertex. In range, the point is
 * path[seg] + normalize(path[seg+1] - path[seg]) * frac (VU0 vector helpers).
 * Matching arm stays INCLUDE_ASM (128-bit lq/sq vertex copies the scalar
 * matcher can't emit); the #else is the structure-exact model. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA618);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 36.59% -> STRUCTURAL,
 * first differing row @1: ROM `swc1 $f21,72(sp)` vs `swc1 $f21,64(sp)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 34.88% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-80` vs `addiu sp,sp,-112`. */
extern s32 FloatToInt(f32 v);
extern f32 IntToFloat(s32 v);
extern void Vec4SubVu0(void *dst, void *a, void *b);
extern void Vec4AddVu0(void *dst, void *a, void *b);
extern void Vec3RescaleToLenVu0(void *dst, void *src, f32 len);
void func_002CA618(u8 *path, s32 *outSeg, f32 *outFrac, Vec4 *outVec, f32 dist,
                   f32 segLen) {
    s32 seg = FloatToInt(dist / segLen);
    s32 last = *(s32 *)path - 1;

    *outSeg = seg;
    if (seg < last) {
        if (seg >= 0) {
            Vec4 *start = (Vec4 *)(path + seg * 0x10 + 0x10);
            Vec4 *next  = (Vec4 *)(path + seg * 0x10 + 0x20);
            f32 frac = dist - IntToFloat(seg) * segLen;
            Vec4 tmp;

            *outFrac = frac;
            Vec4SubVu0(&tmp, next, start);
            *outVec = tmp;
            Vec3RescaleToLenVu0(outVec, outVec, frac);
            Vec4AddVu0(&tmp, outVec, start);
            *outVec = tmp;
            return;
        }
        *outSeg = 0;
    } else {
        *outSeg = last;
    }
    *outFrac = 0.0f;
    *outVec = *(Vec4 *)(path + *outSeg * 0x10 + 0x10);
}
#endif

/* func_002CA740: 8 bytes of dead pad (addiu $sp,0x10; nop) carved off the real
 * entry func_002CA748 in task #472. func_002CA748: menu helper using 128-bit
 * sq/lq (vector) loads/stores (the EE quadword ops are not emitted from scalar C). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA740);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA748);

/* Mis-split fragment: orphaned stack-pointer adjusts (addiu $sp / nops) with
 * no jr $ra — not a real function entry; left as INCLUDE_ASM. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA960);

/* Read the screen-state word stashed at g_pTextTableLoadBuf+0xD8. */
s32 func_002CA970(void) {
    return *(s32 *)(g_pTextTableLoadBuf + 0xD8);
}

/* Reset two state words in the menu screen-state scratch. */
void func_002CA980(void) {
    u8 *p = g_particleFxBlob + 0x100;
    *(s32 *)(p + 0xF0) = 0;
    *(s32 *)(p + 0x1F0) = 0;
}

/* Install the screen-state ptr/flag pair: blob[0xF0] = &buf[0x118], flag=1. */
void func_002CA998(void) {
    u8 *p = g_particleFxBlob + 0x100;
    *(u8 **)(p + 0xF0) = g_pTextTableLoadBuf + 0x118;
    *(s32 *)(p + 0x1F0) = 1;
}

/* List-scroller "select previous": decrement the cursor (list[1]); when it drops
 * to 0 or below, wrap to the limit (list[0]). Then skip backwards over empty
 * (==0) slots in the entry array (list+0xC, one s32 per row), leaving the cursor
 * on the first non-empty slot.
 * list: scroller record {limit, cursor, pad, entries[]}. Returns nothing: the
 * ROM leaves v0 = &entries[cursor] at jr ra only as a by-product of the address
 * computation — spelling that as a pointer return makes cc1 move it into v0 in
 * the epilogue, which the ROM does not do. The one caller discards it.
 * Byte-exact (task #633) on this unit's 2.9 arm. Two spellings carry it:
 *  - `decremented = --list[1]; idx = decremented;` keeps the value that is
 *    STORED and the value that is TESTED in two pseudos, which gives the ROM's
 *    `daddu v1,v0,zero` copy ahead of `bgtz v1` (one variable folds them);
 *  - the byte-offset entry read `(u8 *)entries + (idx << 2)` gives the ROM's
 *    base-first `addu`, as in ListScrollerSelectNext. */
void ListScrollerSelectPrev(s32 *list) {
    s32 *entries = list + 3;
    s32 idx;
    do {
        s32 decremented = --list[1];
        idx = decremented;
        if (idx <= 0) {
            idx = list[0];
        }
        list[1] = idx;
    } while (*(s32 *)((u8 *)entries + (idx << 2)) == 0);
}

/* List-scroller "select next": increment the cursor (list[1]); when it passes
 * the limit (list[0]) wrap to 0, then skip forward over empty (==0) slots in the
 * entry array (list+0xC). Returns the entry value the cursor lands on.
 * Byte-exact (task #468): the entry read is spelled as a byte offset
 * (`(u8 *)entries + (idx << 2)`) so cc1 forms `addu base, idx<<2` in the
 * ROM's operand order; the `entries[idx]` spelling swaps the operands. */
s32 ListScrollerSelectNext(s32 *list) {
    s32 limit = list[0];
    s32 *entries = list + 3;
    s32 entry;
    do {
        s32 idx = list[1] + 1;
        if (limit < idx) {
            idx = 0;
        }
        list[1] = idx;
        entry = *(s32 *)((u8 *)entries + (idx << 2));
    } while (entry == 0);
    return entry;
}

/* GUI wrapper: forward the bestiary widget to its hide/show method. */
s32 func_002CAA38(void) {
    if (g_guiInstance) {
        func_0033A7B8(g_guiInstance + 0x3CEA0, 0);
    }
    return 0;
}

/* GUI wrapper: same widget, opposite visibility flag. */
s32 func_002CAA70(void) {
    if (g_guiInstance) {
        func_0033A7B8(g_guiInstance + 0x3CEA0, 1);
    }
    return 0;
}

/* Draw a centered two-line header inside a 2D batch: from the screen-rect at
 * `p` (center x = p[0x18], y = p[0x1C], width = p[0x20]) it half-splits the
 * width and draws localized string 0x31C8 left-justified at center-half and
 * 0x31C4 at center+half, both in 0x80F0F0F0.
 * Wall: 8-byte-packed-save (saves $16/$17/$18/$31). Preserved as portable C. */
extern void DrawDebugString(s32 x, s32 y, u64 color, char *str, s64 wrap);
extern void func_00280120(s32 x, s32 y, u64 color, char *str, s64 wrap);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CAAA8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 33.74% -> STRUCTURAL,
 * first differing row @1: ROM `(none)` vs `sd ra,24(sp)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 23.24% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-64`. */
/* TODO(match): functional equivalent - not byte-exact; 3-GPR packed-save frame. */
s32 func_002CAAA8(s32 *p) {
    s32 cx = p[6];   /* p[0x18] */
    s32 half = p[8] >> 1; /* p[0x20] width / 2 */
    s32 y = p[7];    /* p[0x1C] */
    char *s;
    Begin2dDrawBatch(0);
    s = GetLocalizedString(0x31C8);
    DrawDebugString(cx - half, y, 0x80F0F0F0, s, -1);
    s = GetLocalizedString(0x31C4);
    func_00280120(cx + half, y, 0x80F0F0F0, s, -1);
    End2dDrawBatch();
    return 0;
}
#endif

/* Clear a 20-entry s32 array (g_menuScreenBlock+0x16C..+0x1BC) to -1, back to
 * front. Wall: cc1 folds %lo(g_menuScreenBlock)+0x1BC into one addiu, but the
 * original keeps the symbol-%lo and the +0x1BC offset as two separate addiu (the
 * SN assembler-absolute macro shape). Preserved as portable C. (g_menuScreenBlock
 * 0x1F27C0 == the old D_1F27C0 - same address, named here.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CAB50);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 85.13% -> STRUCTURAL: `la`+offset shape and prologue order are reproduced by the index form `s32 v = -1; for (i = 0x13; i >= 0; i--) block[i + 0x5B] = v;` (tools/ee/.t468/probe/la3.c h), the residual is then the loop body `sw; addiu v1,-1` vs cc1 `addiu; sw` = ORDER-ONLY (SCHED-TIEBREAK, FACT #7345),
 * first differing row @1: ROM `(none)` vs `addiu v1,zero,19`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 77.87% -> STRUCTURAL, first differing row @1: ROM `(none)` vs `addiu v1,zero,19`. */
void func_002CAB50(void) {
    s32 *p = (s32 *)(g_menuScreenBlock + 0x1BC);
    s32 i = 0x13;
    do {
        *p = -1;
        i--;
        p--;
    } while (i >= 0);
}
#endif

/* Store a reciprocal into the menu scratch: [0x1C0]=1.0f, [0x1C8]=0, [0x1C4]=1/x.
 * Instruction-exact on both arms once the stale alias D_1F27C0 (== g_menuScreenBlock)
 * was retired, but the ROM carries TWO nops between `addiu v0` and `div.s $f12,$f0,$f12`
 * (0x2CABA0/A4 — a compiler-inserted mtc1->div.s hazard pad; the unit has 12 other
 * mtc1 sites with 2 instructions between and NO pad, so it is not an assembler rule).
 * Neither arm emits them: SCHED-NOP-PAD. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CAB90);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 81.82% -> SCHED-NOP-PAD,
 * first differing row @4: ROM `(none)` vs `swc1 $f0,448(v0)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 81.82% -> DSLOT-FILL, first differing row @4: ROM `sll zero,zero,0x0` vs `(none)`. */
void func_002CAB90(float x) {
    float one = 1.0f;
    u8 *p = g_menuScreenBlock;
    *(float *)(p + 0x1C0) = one;
    *(s32 *)(p + 0x1C8) = 0;
    *(float *)(p + 0x1C4) = one / x;
}
#endif

/* ResetWorldCamera: snap the world camera back to its default pose. Writes
 * g_cameraPos = (256, 256, 64) and rebuilds g_cameraMatrix as a 3x4 basis that
 * is identity on the diagonal (matrix[0]=matrix[5]=matrix[10]=1) plus a 1.0 in
 * the row-2 translation slot (matrix[11]). Takes no inputs and calls nothing -
 * a pure constant-store, so the native shim is byte-faithful to the asm. The
 * matching build keeps the asm: the original zeroes the matrix with 128-bit
 * `sq` writes that scalar C does not emit. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CABC0);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 42.42% -> STRUCTURAL,
 * first differing row @2: ROM `lui v0,0x0  [HI16 0x001B5180]` vs `lui v0,0x0  [HI16 0x001B52C0]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 48.85% -> STRUCTURAL, first differing row @2: ROM `lui v0,0x0  [HI16 0x001B5180]` vs `lui v1,0x0  [HI16 0x001B52C0]`. */
void func_002CABC0(void) {
    s32 i;
    g_cameraPos[0] = 256.0f;
    g_cameraPos[1] = 256.0f;
    g_cameraPos[2] = 64.0f;
    for (i = 0; i < 12; i++) {
        g_cameraMatrix[i] = 0.0f;
    }
    g_cameraMatrix[0]  = 1.0f;
    g_cameraMatrix[5]  = 1.0f;
    g_cameraMatrix[10] = 1.0f;
    g_cameraMatrix[11] = 1.0f;
}
#endif

/* RequestMenuScreenChange: open or switch a front-end / pause screen (`screen` is
 * the target screen id). If we are leaving the in-game pause overlay for anything
 * other than the audio-options sub-screen (prev game state 4 AND overlay mode != 8),
 * the block's overlay-active flag (+0x1F4) is cleared. When no pending change is
 * queued (+0x1F4 == 0) it silences the dialog voice (func_00132AF8(0x5D) +
 * SetDialogVoiceVolumesMax(0) + snd_Pump). It marks the screen live (block[0] = 1)
 * and, when not already in-game (g_nGameState == 0), builds the pause prompt. It
 * selects the screen's list-vtable flag block[0xE8] from the high half of D_1A7A10
 * and block[0x108], wires the two list records (D_002598F8/D_00259A88 <->
 * D_00259AD8), latches the active screen id into block[0x8] (block[0x104], or
 * `screen` when that is 0), clears a batch of transition fields, and — unless
 * returning to the map (screen == 6) — re-syncs the galactic map to g_playerProgress
 * (MapSetCurrentLevel + UpdateLevelObjectiveStates + func_002DFE60). Finally it marks
 * the change committed (block[0x14C] = 1).
 * Wall: 8-byte-packed-save (4 GPRs) + branch-likely dialog/list-flag shapes — later
 * cc1 save-slot packing not reproduced. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", RequestMenuScreenChange);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 59.52% -> STRUCTURAL,
 * first differing row @1: ROM `(none)` vs `lui v0,0x0  [HI16 0x001F27C0]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 65.06% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-64`. */
extern s32 GetPrevGameState(void);
extern s32 GetMenuOverlayMode(void);
extern void func_00132AF8(s32 arg);
extern void SetDialogVoiceVolumesMax(s32 arg);
extern void snd_Pump(void);
extern void BuildPausePromptPopup(void);
extern void MapSetCurrentLevel(s32 level);
extern void UpdateLevelObjectiveStates(void);
extern void func_002DFE60(void);
extern s32 g_nGameState;
extern s32 D_1A7A10;
extern u8 D_002598F8[], D_00259AD8[], D_00259A88[];
extern s32 D_25B5D8;
extern s32 g_playerProgress;
void RequestMenuScreenChange(s32 screen) {
    u8 *mb = g_menuScreenBlock;
    s32 cur;

    /* clear the overlay-active flag unless returning to the audio-options sub-screen */
    if (!(GetPrevGameState() == 4 && GetMenuOverlayMode() == 8)) {
        *(s32 *)(mb + 0x1F4) = 0;
    }

    /* no pending change queued: silence the dialog voice */
    if (*(s32 *)(mb + 0x1F4) == 0) {
        func_00132AF8(0x5D);
        SetDialogVoiceVolumesMax(0);
        *(u8 *)(mb + 0xDB) = 0;
        if (*(s32 *)(mb + 0x1F4) == 0) {
            snd_Pump();
        }
    } else {
        *(u8 *)(mb + 0xDB) = 0;
    }

    *(s32 *)(mb + 0) = 1;
    if (g_nGameState == 0) {
        BuildPausePromptPopup();
    }

    /* pick the list-vtable flag, then wire the two list records accordingly */
    *(s32 *)(mb + 0x144) = 0;
    *(s32 *)(mb + 0x148) = 0;
    if ((D_1A7A10 & 0xFFFF0000) != 0 || *(s32 *)(mb + 0x108) != 0) {
        *(s32 *)(mb + 0xE8) = 1;
    } else {
        *(s32 *)(mb + 0xE8) = 0;
    }
    if (*(s32 *)(mb + 0xE8) != 0) {
        *(u8 **)(D_002598F8 + 0x38) = D_00259AD8;
        *(u8 **)(D_00259A88 + 0x3C) = D_00259AD8;
    } else {
        *(u8 **)(D_002598F8 + 0x38) = D_00259A88;
        *(u8 **)(D_00259A88 + 0x3C) = D_002598F8;
    }

    cur = *(s32 *)(mb + 0x104);
    *(s32 *)(mb + 0x1C8) = 0;
    *(s32 *)(mb + 0x1C) = 0;
    *(s32 *)(mb + 0x8) = (cur == 0) ? screen : cur;
    *(s32 *)(mb + 0x20) = 0;
    *(s32 *)(mb + 0x120) = 0;
    *(s32 *)(mb + 0x1C0) = 0;
    *(s32 *)(mb + 0x1C4) = 0;
    if (screen != 6) {
        MapSetCurrentLevel(g_playerProgress);
        UpdateLevelObjectiveStates();
        D_25B5D8 = 0;
        func_002DFE60();
    }
    *(s32 *)(mb + 0x1F4) = 0;
    *(s32 *)(mb + 0x14C) = 1;
    *(s32 *)(mb + 0x150) = 0;
}
#endif

/* screen-capture/restore routine: 8-byte-packed-save wall (saves 5 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", CaptureScreenToVram);

/* Restore the captured screen image from VRAM back to the framebuffer. Fences the
 * frame DMA + waits a vblank field, bumps the render-layer counter, then blits the
 * screen (g_screenHeight * 0x600 bytes) in 0x40x0x40 tiles: for the two-buffer case
 * (menuScreenBlock[0xD0] >= 2) the source starts at 0x3FB000 - size, else at
 * g_vramDynamicBase; each tile is uploaded via func_00126470 + kicked via func_00126730
 * to the dest at menuScreenBlock[0x20], advancing both by 0x4000. Engine-2.96
 * (8-byte-packed saves) -> faithful #else; matching arm INCLUDE_ASM. NEEDS-ORACLE. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", RestoreScreenFromVram);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 57.71% -> FRAME-SIZE,
 * first differing row @0: ROM `addiu sp,sp,-160` vs `addiu sp,sp,-144`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 78.42% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-160` vs `addiu sp,sp,-176`. */
extern s32  g_screenHeight;
extern s32  g_vramDynamicBase;
extern u8   g_renderLayerMask[];
extern void WaitFrameDmaFence(s32 mask);
extern s32  WaitVblankGetField(s32 arg);
extern void func_00126470(void *dst, s32 tbp, s32 a, s32 b, s32 c, s32 d, s32 w, s32 h);
extern void func_0011AEA0(s32 a);
extern void func_00126730(void *packet, s32 addr);
extern void WaitGsPathsIdle(s32 arg);

void RestoreScreenFromVram(void) {
    u8  packet[0x70];
    s32 remaining, src, dst;

    WaitFrameDmaFence(1);
    WaitVblankGetField(0);
    *(s32 *)(g_renderLayerMask + 0x4) += 1;
    remaining = g_screenHeight * 0x600;
    dst = *(s32 *)(g_menuScreenBlock + 0x20);
    src = g_vramDynamicBase;
    if (*(s32 *)(g_menuScreenBlock + 0xD0) >= 2) {
        src = 0x3FB000 - remaining;
    }
    if (remaining >= 0) {
        do {
            func_00126470(packet, (src << 8) >> 16, 1, 0, 0, 0, 0x40, 0x40);
            src += 0x4000;
            remaining -= 0x4000;
            func_0011AEA0(0);
            func_00126730(packet, dst);
            dst += 0x4000;
            WaitGsPathsIdle(0);
        } while (remaining >= 0);
    }
    *(s32 *)(g_menuScreenBlock + 0xD0) = 0;
}
#endif

/* Reset the front-end camera: seed the camera/projection scratch at
 * g_sceneActorMobys + 0x674 (the old D_1B8FC0 alias; +0x21C = 524288.0,
 * +0xB0 aspect = 0.62, +0x228 = 255.0, +0x218 / +0x22C cleared), then rebuild
 * the camera projection (BuildCameraProjection), the default camera pose
 * (func_002CABC0) and the frame view matrices (BuildFrameViewMatrices).
 *
 * Returns: nothing meaningful. Callers treat this as void (1D54C0.c declares it
 * so); v0 is whatever BuildFrameViewMatrices leaves behind, exactly as in the ROM.
 *
 * MATCH (task #641), three spellings the bytes depend on:
 * - The s32 return through a cast call on the function NAME. cc1 2.9 turns
 *   EVERY trailing void call into a sibling call (`ld ra; j callee`), but it
 *   never does so for a value-returning `return f();`. The ROM ends
 *   `jal; nop; ld ra; jr`. A trailing `__asm__ __volatile__("")` (the 188858.c
 *   / 1EFFC0.c guard) also stops the tail call. Measured here, it moves
 *   `li.s $f1` above the `addiu sp` and misses by that one word.
 * - The three float constants as locals declared 524288, 255, 0.62. That gives
 *   the ROM's $f1/$f2/$f0 assignment.
 * - The +0x22C store written BEFORE the +0x218 store. The 2.9 scheduler swaps
 *   the pair, so +0x218 lands before the jal and +0x22C lands in its delay slot. */
s32 func_002CAFD8(void) {
    u8 *scratch = g_sceneActorMobys + 0x674;
    f32 farScale = 524288.0f, colorMax = 255.0f, aspect = 0.62f;
    *(f32 *)(scratch + 0x21C) = farScale;
    *(f32 *)(scratch + 0xB0) = aspect;
    *(f32 *)(scratch + 0x228) = colorMax;
    *(s32 *)(scratch + 0x22C) = 0;
    *(s32 *)(scratch + 0x218) = 0;
    BuildCameraProjection();
    func_002CABC0();
    return ((s32 (*)(void))BuildFrameViewMatrices)();
}

/* menu-screen lifecycle routine: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", MenuScreenLoad);

/* menu helper: 8-byte-packed-save wall (saves 5 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CB560);

/* TickFrontEndScreenIdle: enter/refresh a menu screen's render context. Resets the
 * screen state word, publishes the screen's stored camera position (two Vec4s
 * at +0x50/+0x60) and view block to the live camera globals, rebuilds the
 * camera projection + frame view matrices, swaps to moby table 0, clears the
 * per-frame scratch fields, and — for the fade-in screen kinds {3,4,5,6} —
 * fades from black. When not already shut down (+0x1F4 == 0) it also runs the
 * frame's sound service: a fixed sound event (0x5D), dialog-voice mute (unless
 * a level exit is pending on kind 2), the sound pump, emitter update, and a
 * one-shot dialog-voice pump gated by the +0xDB flag. Matching arm stays
 * INCLUDE_ASM (128-bit lq/sq camera-vec copies); #else is structure-exact. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", TickFrontEndScreenIdle);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 74.43% -> STRUCTURAL,
 * first differing row @5: ROM `(none)` vs `jal L  [26 0x002FCFC8]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 67.27% -> SCHED-TIEBREAK, first differing row @1: ROM `sd s1,8(sp)` vs `(none)`. */
extern void func_002FCFC8(void);
extern void func_00283460(void *dst, void *src, s32 len);
extern void func_0027A550(void);
extern void func_00132B28(s32 id);
extern s32  IsLevelExitRequested(void);
extern void SetDialogVoiceVolumesMute(void);
extern void FadeOutToBlackBlocking(s32 frames);
extern void func_002B9438(void);
extern void SwapMobyTableContext(s32 tableId);
extern void UpdateSoundEmitters(void);      /* file-scope decl is below L545 */
extern void PumpDialogVoiceSystem(s32 blocking);
extern f32  g_cameraProjScale;
void TickFrontEndScreenIdle(void) {
    s32 kind;

    func_002FCFC8();
    *(s32 *)g_menuScreenBlock = 0;
    *(Vec4 *)g_cameraPos = *(Vec4 *)(g_menuScreenBlock + 0x50);
    *(Vec4 *)((u8 *)g_cameraPos + 0x10) = *(Vec4 *)(g_menuScreenBlock + 0x60);
    func_00283460((u8 *)g_cameraPos + 0x230, g_menuScreenBlock + 0x200, 0x30);
    g_cameraProjScale = *(f32 *)(g_menuScreenBlock + 0xF8);
    BuildCameraProjection();
    BuildFrameViewMatrices();
    SwapMobyTableContext(0);
    func_0027A550();
    *(s32 *)(g_menuScreenBlock + 0x118) = 0;
    *(s32 *)(g_menuScreenBlock + 0x11C) = 0;
    *(s32 *)(g_menuScreenBlock + 0x114) = 0;
    *(s32 *)(g_menuScreenBlock + 0x20) = 0;

    kind = *(s32 *)(g_menuScreenBlock + 0x1C);
    if ((u32)(kind - 3) < 2 || kind == 6 || kind == 5) {
        FadeOutToBlackBlocking(0x10);
    }

    if (*(s32 *)(g_menuScreenBlock + 0x1F4) == 0) {
        func_00132B28(0x5D);
        if (*(s32 *)(g_menuScreenBlock + 0x1C) != 2 || !IsLevelExitRequested()) {
            SetDialogVoiceVolumesMute();
        }
        snd_Pump();
    }

    UpdateSoundEmitters();
    if (g_menuScreenBlock[0xDB] != 0) {
        PumpDialogVoiceSystem(1);
        g_menuScreenBlock[0xDB] = 0;
    }
    func_002B9438();
}
#endif

/* Front-end screen-machine per-frame tick (TickFrontEndScreenMachine).
 * Advances the menu-idle counter (saturating at 0x7D00) and the transition
 * countdown, runs the area-transition fade timer, lets the save/load driver
 * (func_002DECE0) run, and — if a save/load op just finished while the screen is
 * in the "commit" state (4) with a level-exit queued — kicks the game-state
 * change to bring up the next level. Then dispatches the current screen state
 * (g_menuScreenBlock[0], 1..5) to its per-state handler and, for the non-terminal
 * states, runs the shared moby + sound-emitter tick and the optional post-hook.
 * Matching arm stays INCLUDE_ASM (cc1 jump-table reloc layout not reproduced). */
extern u8 g_areaTable[];
extern u8 g_nNanotechBonusHealTimer[];
extern u8 g_nSaveLoadStatusCode[];
extern u8 g_nLevelExitDestination[];
extern u8 g_hudClutSlots[];
extern s32 RequestGameStateChange(s32 stateId, s32 push, s32 c, s32 d, s32 e);
extern void func_002DECE0(void);
extern void UpdateSoundEmitters(void);
extern void UpdateActiveMobys(void);
extern void MenuScreenUpdate(void);
extern void func_002CBD68(void);
extern void func_002CB560(void);
extern void MenuScreenBeginLoad(void);
extern void func_002CBA40(void);
extern void MenuScreenCommitTransition(void);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", TickFrontEndScreenMachine);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 70.66% -> STRUCTURAL,
 * first differing row @3: ROM `addiu a3,v0,0  [LO16 0x001F27C0]` vs `addiu s0,v0,0  [LO16 0x001F27C0]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 79.65% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. */
s32 TickFrontEndScreenMachine(void) {
    u8 *mb = g_menuScreenBlock;
    s32 counter;

    *(s32 *)(mb + 0x168) = 0;
    counter = *(s32 *)(mb + 0x120);
    *(s32 *)(mb + 0x120) = (counter > 0x7CFF) ? 0x7D00 : counter + 1;
    if (*(s32 *)(mb + 0x158) != 0)
        *(s32 *)(mb + 0x158) -= 1;

    /* area-transition fade timer: tick while the area controller is idle
     * (g_areaTable+0x15C < 3 and its +0x164 timer negative), else reset. */
    if (*(s32 *)(g_areaTable + 0x15C) < 3 &&
        *(s32 *)(g_areaTable + 0x164) < 0)
        *(s32 *)(mb + 0x164) += 1;
    else
        *(s32 *)(mb + 0x164) = 0;

    func_002DECE0();

    if ((*(s32 *)(g_nSaveLoadStatusCode + 4) & 1) &&
        *(s32 *)mb == 4 &&
        *(s32 *)(g_nLevelExitDestination + 4) >= 8)
        RequestGameStateChange(4, 1, 1, *(s32 *)(mb + 0x14), 0);

    switch (*(s32 *)mb) {
    case 1:
        MenuScreenBeginLoad();
        break;
    case 2:
        func_002CBA40();
        break;
    case 3:
        MenuScreenCommitTransition();
        break;
    case 4:
        MenuScreenUpdate();
        break;
    case 5:
        func_002CBD68();
        UpdateSoundEmitters();
        *(s16 *)(g_nNanotechBonusHealTimer + 4) = 0xA;
        *(s32 *)(g_hudClutSlots + 0x10) = 0;
        return 0;
    default:
        break;
    }

    UpdateActiveMobys();
    UpdateSoundEmitters();
    if (*(s32 *)(mb + 0x1C) != 0)
        func_002CB560();
    return 0;
}
#endif

/* MenuScreenLoad then mark the screen-state scratch ready (state=2). */
void MenuScreenBeginLoad(void) {
    u8 *p;
    MenuScreenLoad();
    p = g_particleFxBlob + 0x100;
    *(s32 *)(p + 0x0) = 2;
    *(s32 *)(p + 0x4) = 0;
}

/* Mark the screen-state scratch (state=3, clear sub-state). */
void func_002CBA40(void) {
    u8 *p = g_particleFxBlob + 0x100;
    *(s32 *)(p + 0x0) = 3;
    *(s32 *)(p + 0x4) = 0;
}

/* Commit a menu transition: full-screen tint then latch the screen-state
 * scratch (state=4, capture pending sub-state). Near-miss: the original emits a
 * dead conditional store (p[0x4]=1 then unconditional =0) the later cc1
 * load-PRE keeps but ours eliminates — preserved as portable C. */
extern void DrawFullScreenTint(s32 r, s32 g, s32 b, s32 a);
/* NEAR-MISS (91%, not byte-exact): real C reaches this far — the body (bnel
 * branch-likely dead store, address-rematerialise-after-call, final-store order)
 * all match — but cc1 schedules the callee-save `sd ra` after only ONE of the
 * three zeroed-arg `move`s for DrawFullScreenTint, where the original interleaves
 * it after two (`daddu a0; daddu a1; sd ra; daddu a2`). That frame-save
 * scheduling slot has no C-level lever, so this stays INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", MenuScreenCommitTransition);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 32.17% -> STRUCTURAL,
 * first differing row @2: ROM `(none)` vs `sd s0,0(sp)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 32.57% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-48`. */
/* TODO(match): functional equivalent - not byte-exact; the original keeps a dead
 * conditional store (block[0x4]=1 then unconditionally =0) the later cc1 load-PRE
 * retains but ours eliminates. The transient block[0x4]=1 has no observable effect
 * (no intervening call), so the captured block[0x18] -> block[0x14] latch and the
 * state=4 store are byte-faithful. */
void MenuScreenCommitTransition(void) {
    s32 *block = (s32 *)g_menuScreenBlock;
    DrawFullScreenTint(0, 0, 0, 0x38);
    block[0x14 / 4] = block[0x18 / 4]; /* latch pending sub-state */
    block[0]        = 4;               /* state = commit */
    block[0x4 / 4]  = 0;
    block[0x18 / 4] = 0;
}
#endif

/* menu-screen lifecycle routine: 8-byte-packed-save wall (saves 8 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", MenuScreenUpdate);

/* Screen-state-5 handler (the terminal "leaving the front-end" state, dispatched
 * from TickFrontEndScreenMachine). Runs a per-frame countdown (g_menuScreenBlock
 * +0x24); once it hits zero and no file load is in flight, commits the queued
 * transition by its sub-state (g_menuScreenBlock+0x1C): 2 = pop back to the map,
 * 3/4/6 = push game-state 1 with the queued arg (+0xF4), 5 = push game-state 2,
 * anything else = plain pop. Then, if the equipped-weapon slot changed
 * (+0x40 != 0 and != +0x30), plays that weapon's voice line via its
 * g_weaponTable entry, bracketed by dialog-voice pumps.
 * Matching arm stays INCLUDE_ASM (later cc1 packs 8-byte save slots vs our 16). */
extern s16 g_fileLoadState;
extern s32 g_mapCurrentLevel;
extern u8 g_itemEquippedSlot[];
extern u8 g_weaponTable[];
extern void PopGameState(s32 a, s32 b);
extern void PumpDialogVoiceSystem(s32 blocking);
extern void func_00294CD0(s32 arg);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CBD68);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 56.44% -> STRUCTURAL,
 * first differing row @3: ROM `addiu a0,v0,0  [LO16 0x001F27C0]` vs `addiu s0,v0,0  [LO16 0x001F27C0]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 68.92% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. */
void func_002CBD68(void) {
    u8 *mb = g_menuScreenBlock;
    s32 sub;

    if (*(s32 *)(mb + 0x24) != 0)
        *(s32 *)(mb + 0x24) -= 1;
    if (*(s32 *)(mb + 0x24) != 0)
        return;
    if (g_fileLoadState != 0)
        return;

    sub = *(s32 *)(mb + 0x1C);
    if (sub == 2) {
        PopGameState(0, g_mapCurrentLevel);
    } else if (sub == 3 || sub == 4 || sub == 6) {
        RequestGameStateChange(1, 1, 0, *(s32 *)(mb + 0xF4), 0);
    } else if (sub == 5) {
        RequestGameStateChange(2, 1, *(s32 *)(mb + 0xF4), 0, 0);
    } else {
        PopGameState(0, 0);
    }

    if (*(s32 *)(mb + 0x40) != 0 &&
        *(s32 *)(mb + 0x30) != *(s32 *)(mb + 0x40)) {
        u8 *weapon;
        PumpDialogVoiceSystem(1);
        weapon = g_weaponTable + g_itemEquippedSlot[*(s32 *)(mb + 0x40)] * 0xE0;
        func_00294CD0(*(s32 *)(weapon + 0x14));
        PumpDialogVoiceSystem(1);
    }
    PumpDialogVoiceSystem(1);
}
#endif

/* menu data/list builder: 8-byte-packed-save wall (saves 3 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", BuildPausePromptPopup);

/* Level-select slot-index validity gate (idx < 0x15 or idx == 0x18). */
s32 IsLevelListEntryEnabled(s32 idx) {
    return (idx < 0x15 || idx == 0x18);
}

/* LevelSelectListHandleInput: per-frame input for the level-select scroller (rooted
 * at g_nLevelSelectListCount). First refreshes each entry's enabled flag (+0xC
 * array) from g_abLevelAvailableFlags for entries IsLevelListEntryEnabled reports,
 * then resets +0xC. Then dispatches held-button `flags`: up (0x1000) /down (0x4000)
 * move the selection (ListScrollerSelectPrev/Next); confirm (0x40) requests the exit
 * to the selected destination via RequestLevelExit(sel, 1) — except the special
 * label 0xB47 with D_1A7C09 clear, which exits to 0x19 instead. Returns the selected
 * index on confirm, else -1.
 * (matching arm left INCLUDE_ASM: 8-byte-packed-save wall.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", LevelSelectListHandleInput);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 75.64% -> STRUCTURAL,
 * first differing row @1: ROM `lui v0,0x0  [HI16 0x002109F0]` vs `sd s4,32(sp)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 76.14% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-64` vs `addiu sp,sp,-128`. */
extern s32 g_nLevelSelectListCount;
extern u8 g_abLevelAvailableFlags[];
extern u8 g_levelSelectEntries[];
extern u8 D_1A7C09;
extern void RequestLevelExit(s32 destination, s32 commitSave);
s32 LevelSelectListHandleInput(s32 flags) {
    s32 *scroller = &g_nLevelSelectListCount;
    s32 result = -1;
    s32 i;

    if (g_nLevelSelectListCount >= 0) {
        for (i = 0; i <= g_nLevelSelectListCount; i++) {
            if (IsLevelListEntryEnabled(i)) {
                *(s32 *)((char *)scroller + 0xC + i * 4) =
                    (g_abLevelAvailableFlags[i] != 0);
            }
        }
    }
    *(s32 *)((char *)scroller + 0xC) = 0;

    if (flags & 0x1000) {           /* up */
        ListScrollerSelectPrev(scroller);
    } else if (flags & 0x4000) {    /* down */
        ListScrollerSelectNext(scroller);
    } else if (flags & 0x40) {      /* confirm */
        s32 sel = *(s32 *)((char *)scroller + 0x4);
        result = sel;
        if (*(s32 *)&g_levelSelectEntries[sel * 8] == 0xB47 && D_1A7C09 == 0) {
            RequestLevelExit(0x19, 1);
        } else {
            RequestLevelExit(sel, 1);
        }
    }
    return result;
}
#endif

/* LevelSelectListRender: draws the galactic-map level-select list. Dims the screen
 * (DrawFullScreenTint), then for each enabled entry (scroller flag at +0xC+i*4 != 0)
 * of the g_nLevelSelectListCount-rooted scroller, formats "<name> <detail>" into a
 * local buffer (func_00115DA8 sprintf with format D_1AB920, from the two localized
 * strings at entry+0x4 and entry+0x0 of the scroller's entry array at +0x8), draws a
 * row background bar (func_0027F208) and the row text (func_002801B8) at
 * y = D_1AB918 + row*D_1AB914 where row = 2*i (or 0x24 for the special last row
 * i==0x18); the row color is the highlight 0x80FFDE8D when it is the selected row
 * (scroller +0x4) else 0x80808080.
 * Wall: 8-byte-packed-save (9 GPRs) — later cc1 save-slot packing not reproduced.
 * Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", LevelSelectListRender);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 47.76% -> STRUCTURAL,
 * first differing row @2: ROM `(none)` vs `sd s6,304(sp)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 58.31% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-336` vs `addiu sp,sp,-416`. */
extern void func_002801B8(s32 x, s32 y, u64 color, char *str, s64 sel);
extern void func_0027F208(s32 y0, s32 y1, s32 x0, s32 x1, s32 h, s32 color);
extern void func_00115DA8(char *dst, const char *fmt, ...); /* SDK sprintf */
extern s32 D_1AB914;             /* row pitch */
extern s32 D_1AB918;             /* base row y */
extern char D_1AB920[];          /* "<name> <detail>" format string */
s32 LevelSelectListRender(void) {
    char buf[0x100];
    s32 *scroller = &g_nLevelSelectListCount;
    s32 i;

    DrawFullScreenTint(0, 0, 0, 0x60);
    for (i = 0; i <= g_nLevelSelectListCount; i++) {
        s32 *entry;
        s32 flag = *(s32 *)((char *)scroller + 0xC + i * 4);
        s32 row, y;
        char *name, *detail;
        u32 color;
        if (flag == 0) {
            continue;
        }
        entry = (s32 *)((char *)scroller[2] + i * 8);   /* scroller[0x8] = entry array */
        row = (i != 0x18) ? i * 2 : 0x24;
        name = GetLocalizedString(entry[1]);            /* entry+0x4 */
        detail = GetLocalizedString(entry[0]);          /* entry+0x0 */
        func_00115DA8(buf, D_1AB920, name, detail);
        y = D_1AB918 + row * D_1AB914;
        func_0027F208(y, y + 0xF, 0x40, 0x1C0, 0x60, 0x60442D00);
        color = (scroller[1] == i) ? 0x80FFDE8D : 0x80808080;
        func_002801B8(0x100, y, color, buf, -1);
    }
    return 0;
}
#endif

/* Dispatch a menu action via a 2-case switch: action 0 -> result 1, action 1 ->
 * func_002D67A0(3, &D_0025BA70), default -> result 0. (MATCHED: the `switch`
 * reproduces cc1's early-out layout — the two cases emitted out-of-line after
 * the default fall-through that merges to a single tail return — which the
 * earlier if/else-if chain could not.) */
s32 func_002CC788(s32 action) {
    s32 result = 0;
    switch (action) {
    case 0:
        result = 1;
        break;
    case 1:
        result = func_002D67A0(3, D_0025BA70);
        break;
    }
    return result;
}

/* menu helper: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC7D8);

/* func_002CC858: dispatch one of seven map-cell config descriptors (D_1AA6D8,
 * stride 0x18, selected by `q` in 0..6) through func_002D6AD8, returning its result
 * into a scratch buffer; out-of-range `q` returns 0. Selecting descriptor 0 also
 * snapshots the sound-bank handle (g_soundBankHandlesBlk +0x1248 -> +0x22C8).
 * (matching arm left INCLUDE_ASM: cc1 jump-table layout not reproduced.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC858);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 24.55% -> STRUCTURAL,
 * first differing row @0: ROM `(none)` vs `daddu a1,a0,zero`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 0.00% -> STRUCTURAL, first differing row @1: ROM `daddu a1,zero,zero` vs `daddu a1,a0,zero`. */
extern s32 func_002D6AD8(void *config, void *buf);
extern u8 D_1AA6D8[];
extern u8 g_soundBankHandlesBlk[];
s32 func_002CC858(s32 q) {
    s32 buf[4];

    buf[0] = 0;
    if ((u32)q >= 7) {
        return 0;
    }
    if (q == 0) {
        *(s32 *)(g_soundBankHandlesBlk + 0x22C8) =
            *(s32 *)(g_soundBankHandlesBlk + 0x1248);
    }
    return func_002D6AD8(&D_1AA6D8[q * 0x18], buf);
}
#endif

/* menu helper: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC908);

/* 3-case menu action dispatch via switch, result merged to one tail return:
 *   action==0: arm g_nNanotechBonusHealTimer mirror (s16 at +4 = 0xA), result 1
 *   action==1: result = func_002D67A0(3, &D_0025BA70)
 *   action==2: func_0029DCB8(), result 1
 *   default:   result 0
 * (MATCHED: written as a `switch` so cc1 emits the comparison-tree dispatch with
 * out-of-line case bodies and a single-register result threaded to the tail; the
 * earlier ordered if-chain produced a structurally different body.) */
extern u8 g_nNanotechBonusHealTimer[];
extern void func_0029DCB8(void);
s32 func_002CCA18(s32 action) {
    s32 result = 0;
    switch (action) {
    case 0:
        *(s16 *)(g_nNanotechBonusHealTimer + 4) = 0xA;
        result = 1;
        break;
    case 1:
        result = func_002D67A0(3, D_0025BA70);
        break;
    case 2:
        func_0029DCB8();
        result = 1;
        break;
    }
    return result;
}

/* TickActiveMenuScreen: ticks the currently-active menu screen (the MenuScreen
 * object at g_menuScreenBlock+0x14) and returns whether it stays open (1) or is
 * done (0). Level-select list: on a nav/confirm press (0x910) it latches
 * block+0x1C (2 if block+0x8==4 else 1) and returns 1; otherwise it calls the
 * list's per-frame handler (function pointer at g_pLevelSelectListEntries+0x84)
 * and returns whether it succeeded (>=0). Screen "Id10" ticks func_0029DC70.
 * Map-back-target and galactic-map screens tick only while D_1AB930 is clear:
 * the former calls func_0029D0C8(0); the latter runs GalacticMapScreenTick(0)
 * (latching block+0x1C as above when it fires), then func_0029D138(pad), and
 * returns whether the block's back-target (block+0x18) is no longer the
 * map-back-target screen. All other active screens (incl. those two while
 * D_1AB930 is set, and the inert D_0025A038/D_00259B28 ids) tick to no-op -> 0.
 *
 * TODO(match): functional equivalent - not byte-exact. 8-byte-packed-save wall
 * (saves 4 GPRs incl $31; later cc1 packs save slots 8-byte vs our 16-byte);
 * preserved as portable C, the matching arm stays INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", TickActiveMenuScreen);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 69.00% -> SPLIT-HIREG,
 * first differing row @1: ROM `lui v1,0x0  [HI16 0x001F27C0]` vs `lui v0,0x0  [HI16 0x001F27C0]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 63.14% -> SCHED-TIEBREAK, first differing row @2: ROM `sd s1,8(sp)` vs `(none)`. */
extern s32 func_0029D0C8(s32 arg);
extern s32 func_0029D138(s32 arg);
extern s32 func_0029DC70(void);
extern s32 GalacticMapScreenTick(s32 arg);
extern s32 g_padButtonsPressed;
extern s32 D_1AB930;
extern u8 g_MenuScreen_LevelSelectList[];
extern u8 g_MenuScreen_Id10[];
extern u8 g_MenuScreen_MapBackTarget[];
extern u8 g_MenuScreen_GalacticMap[];
extern u8 g_pLevelSelectListEntries[];
s32 TickActiveMenuScreen(void) {
    u8 *screen = *(u8 **)(g_menuScreenBlock + 0x14);
    s32 result = 0;

    if (screen == g_MenuScreen_LevelSelectList) {
        if (g_padButtonsPressed & 0x910) {
            *(s32 *)(g_menuScreenBlock + 0x1C) =
                (*(s32 *)(g_menuScreenBlock + 0x8) == 4) ? 2 : 1;
            result = 1;
        } else {
            s32 (*tick)(void) = *(s32 (**)(void))(g_pLevelSelectListEntries + 0x84);
            result = (tick() >= 0);
        }
    } else if (screen == g_MenuScreen_Id10) {
        func_0029DC70();
    } else if (screen == g_MenuScreen_MapBackTarget && D_1AB930 == 0) {
        func_0029D0C8(0);
    } else if (screen == g_MenuScreen_GalacticMap && D_1AB930 == 0) {
        if (GalacticMapScreenTick(0) != 0) {
            *(s32 *)(g_menuScreenBlock + 0x1C) =
                (*(s32 *)(g_menuScreenBlock + 0x8) == 4) ? 2 : 1;
        }
        func_0029D138(g_padButtonsPressed);
        result = (*(u8 **)(g_menuScreenBlock + 0x18) != g_MenuScreen_MapBackTarget);
    }
    return result;
}
#endif

/* menu-screen lifecycle routine: 8-byte-packed-save wall (saves 9 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", RenderMenuScreenWidgets);

/* Menu screen-state query: only acts when the active screen (ss[0x14]) is the
 * one passed in. Returns a tri-state confirm/cancel code driven by the global
 * input flags (D_138180[0x1C4]) and the screen's pending-result fields.
 * Near-miss (~48%): the original keeps the second D_138180[0x1C4] reload and the
 * branch-likely (beql/bnel) loop shape from the load-PRE-present SN cc1; our cc1
 * CSEs the reload and the andi 0x10 test, producing a structurally different
 * (shorter) body. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CD450);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 49.43% -> STRUCTURAL,
 * first differing row @0: ROM `lui v0,0x0  [HI16 0x001F27C0]` vs `lui v0,0x0  [HI16 0x001F26C0]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 48.22% -> STRUCTURAL, first differing row @0: ROM `lui v0,0x0  [HI16 0x001F27C0]` vs `lui v0,0x0  [HI16 0x001F26C0]`. */
/* TODO(match): functional equivalent - not byte-exact; cc1 CSEs the global-flag
 * reload + andi the original re-emits under branch-likely. */
s32 func_002CD450(s32 screen) {
    u8 *ss = g_particleFxBlob + 0x100;
    s32 flags;
    s32 v;
    if (*(s32 *)(*(u8 **)(ss + 0x14) + 0xE8) != screen) {
        return 0;
    }
    flags = *(s32 *)(D_138180 + 0x1C4);
    if (flags & 0x900) {
        if (*(s32 *)(ss + 0x134) == 0) {
            return 1;
        }
        flags = *(s32 *)(D_138180 + 0x1C4);
    }
    if (!(flags & 0x10)) {
        return 0;
    }
    v = *(s32 *)(*(u8 **)(ss + 0x14) + 0xE0);
    if (v != 0) {
        *(s32 *)(ss + 0x18) = v;
        return 0;
    }
    if (*(s32 *)(ss + 0x134) == 0) {
        return -1;
    }
    return 0;
}
#endif

/* Stores D_1A7318 ? &D_1AB648 : &D_1AB678 into list[0x34], returns 0. Left as
 * INCLUDE_ASM: the original has an anomalous +0x60 stack adjust prologue with no
 * matching restore (frame artifact) that our cc1 won't reproduce from clean C.
 * Preserved as portable C for the native target. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CD4E8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 0.00% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,96` vs `lui v0,0x0  [HI16 D_1A7318]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 0.00% -> STRUCTURAL, first differing row @0: ROM `addiu sp,sp,96` vs `lui v0,0x0  [HI16 D_1A7318]`. */
/* TODO(match): functional equivalent - not byte-exact; anomalous +0x60 frame
 * prologue without matching restore not reproduced from clean C. The selector
 * is D_1A7318 (== g_vramTextureBase_28 + 0xC); movn picks &D_1AB648 when nonzero,
 * else &D_1AB678. Stores into list[0x34/4] (= list[0xD]) and returns 0. */
s32 func_002CD4E8(s32 *list) {
    list[0xD] = (s32)(D_1A7318 ? D_1AB648 : D_1AB678);
    return 0;
}
#endif

/* BuildCheatMenuItemList: build the cheat-menu item list. Copies two contiguous
 * parallel tables (D_1AB958 = 8 cheat-entry indices, -1 terminated; D_1AB978 = the
 * parallel display values) into one buffer, then for each entry still unlocked
 * (D_1A7A58[entry] != 0) emits a 0x14-byte menu record into D_0025FC70:
 *   {+0x0 value = D_1AB978[i], +0x4 label = &g_cheatFlags[entry], +0x8 = 0x2C5C,
 *    +0xC = 0x2C5D, +0x10 = 0}, and finally a zero terminator at record[count].
 * Blocked (match): the original ldl/ldr/sdl/sdr unaligned 64-bit copy idiom cc1 won't
 * reproduce from clean C — #else fallback.
 * NEEDS-TESTER-ORACLE: the parallel-array wiring + record layout are modeled from the
 * asm; the contiguous buffer copy is required (the ROM lays D_1AB958/D_1AB978 0x20 apart,
 * which separate host externs don't guarantee) — the oracle should confirm faithfulness. */
/* Declarations only (no code); unconditional so the engine96 arm sees them. */
typedef struct {
    s32  value;    /* +0x0 */
    u8  *label;    /* +0x4  &g_cheatFlags[entry] */
    s32  kind1;    /* +0x8  0x2C5C */
    s32  kind2;    /* +0xC  0x2C5D */
    s32  flags;    /* +0x10 */
} CheatMenuItem;   /* 0x14 */
extern s32 D_1AB958[];   /* 8 cheat-entry indices, -1 terminated */
extern s32 D_1AB978[];   /* parallel display values */
extern u8  D_1A7A58[];   /* per-cheat unlock flag, indexed by entry */
extern u8  g_cheatFlags[];
extern CheatMenuItem D_0025FC70[];

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", BuildCheatMenuItemList);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 6.52% -> STRUCTURAL,
 * first differing row @0: ROM `(none)` vs `lui t2,0x0  [HI16 D_0025FC70]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 33.86% -> STRUCTURAL, first differing row @1: ROM `lui v0,0x0  [HI16 D_1AB958]` vs `lui v0,0x0  [HI16 D_0025FC70]`. */
void BuildCheatMenuItemList(void) {
    s32 buf[16];   /* contiguous: [0..7] entries, [8..14] parallel values */
    CheatMenuItem *rec = D_0025FC70;
    s32 count = 0;
    s32 i;

    for (i = 0; i < 8; i++) {
        buf[i] = D_1AB958[i];
    }
    for (i = 0; i < 7; i++) {
        buf[i + 8] = D_1AB978[i];
    }
    if (buf[0] != -1) {
        i = 0;
        do {
            s32 entry = buf[i];
            if (D_1A7A58[entry] != 0) {
                rec->value = buf[i + 8];
                rec->label = &g_cheatFlags[entry];
                rec->kind1 = 0x2C5C;
                rec->kind2 = 0x2C5D;
                rec->flags = 0;
                rec++;
                count++;
            }
            i++;
        } while (i < 12 && buf[i] != -1);
    }
    D_0025FC70[count].value = 0;
}
#endif

/* return 0 stub. */
s32 func_002CD650(void) {
    return 0;
}

/* return 0 stub. */
s32 func_002CD658(void) {
    return 0;
}

/* Clear a list-state field (entry +0x44 = -1) and return 0. */
s32 func_002CD660(s32 *list) {
    list[0x11] = -1;
    return 0;
}

/* func_002CD670: a UI screen renderer (~2.1 KB). Dispatches on a screen/mode
 * selector through a jump table (sltiu + jr $3) and builds GS register packets
 * (AppendGsRegPacket) for the selected panel — localized strings
 * (GetLocalizedString), UI textures (GetUiTextureTex0), the active-objectives
 * list (GatherActiveObjectives), etc. — with ldl/ldr/sdl/sdr unaligned copies.
 * PARK (INCLUDE_ASM): high transcription risk per the blast-radius guardrail —
 * combines a splat-jtbl-reloc switch, raw-GS packet assembly (raw-GS-HLE), and
 * a large unverifiable body; a hand-written #else here is not worth the silent-
 * error surface in coverage-only C. Matching arm is byte-frozen regardless. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CD670);

/* Cheat-flag mirror: for each source toggle byte (D_1A7BDn) write 3 ("on") or 0
 * ("off") into the matching cheat-menu item-state halfword (D_1AA5xx/D_1AA602).
 * Leaf, pure data shuffle.
 * Near-miss (~83%): the original schedules each ternary's `move rd,zero` into
 * the beqz delay slot while emitting the previous result's store before the
 * branch; our cc1 hoists the move ahead of the store and leaves a nop in the
 * delay slot (branch-fill scheduling the later cc1 won't reproduce from clean
 * C). Preserved as portable C. */
extern u8 D_1A7BD1, D_1A7BD2, D_1A7BD3, D_1A7BD4, D_1A7BD6;
extern s16 D_1AA5A2, D_1AA5BA, D_1AA5D2, D_1AA5EA, D_1AA602;

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CDEB0);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 64.32% -> BRANCH-SHAPE,
 * first differing row @2: ROM `beq v0,zero,L` vs `bne v0,zero,L`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 82.70% -> STRUCTURAL, first differing row @7: ROM `(none)` vs `daddu a0,zero,zero`. */
/* TODO(match): functional equivalent - not byte-exact; ternary move/store
 * delay-slot scheduling not reproduced by cc1. */
void func_002CDEB0(void) {
    D_1AA5A2 = D_1A7BD1 ? 3 : 0;
    D_1AA5BA = D_1A7BD2 ? 3 : 0;
    D_1AA5D2 = D_1A7BD3 ? 3 : 0;
    D_1AA5EA = D_1A7BD4 ? 3 : 0;
    D_1AA602 = D_1A7BD6 ? 3 : 0;
}
#endif

/* Level-exit confirm dispatch for the active menu screen: validates the pending
 * pick against the global input flags + per-screen tables and sets g_nLevelExit*.
 * Only acts when `item` is the screen's currently-active widget (screen+0xE8) and
 * the widget's table entry (item+0x34[item+0x40], stride 0xC, +0x2) marks it a
 * "level exit" (kind 3); then maps the widget identity (one of the six menu-item
 * records) to a level-exit destination code and arms g_nLevelExitRequested from
 * that destination's enabled byte (D_1A7BD1..). If the byte is clear the pending
 * destination is rolled back to its saved value. Anything not handled here is
 * forwarded to func_002D6B28 (whose result is returned).
 * Matching arm stays INCLUDE_ASM (later cc1 lays out the chained pointer compares
 * differently). */
extern u8 *g_pCurrentMenuScreen;
extern s32 g_nLevelExitRequested;
extern s32 func_002D6B28(void *item);
extern u8 D_00259128[], D_00259178[], D_002591C8[], D_00259218[], D_00259268[], D_002592B8[];
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CDF48);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 44.72% -> STRUCTURAL,
 * first differing row @5: ROM `lw a0,232(v1)` vs `lw a1,232(v1)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 63.75% -> STRUCTURAL, first differing row @9: ROM `daddu a3,v0,zero` vs `lw v1,452(v0)  [LO16 D_138180]`. */
s32 func_002CDF48(void *item) {
    u8 *screen = g_pCurrentMenuScreen;
    s32 savedDest;
    u8 *table;
    s32 dest;
    u8 enabled;

    if (*(void **)(screen + 0xE8) != item)
        return func_002D6B28(item);

    /* item inert while any input flag other than 0x40 is held */
    if ((*(s32 *)(D_138180 + 0x1C4) & ~0x40) != 0 && item == (void *)D_00259128)
        return 0;
    if (g_nLevelExitRequested != 0)
        return func_002D6B28(item);

    savedDest = *(s32 *)g_nLevelExitDestination;

    /* the widget must be a "level exit" entry (kind 3) in its own table */
    table = *(u8 **)((u8 *)item + 0x34);
    if (*(s16 *)(table + *(s32 *)((u8 *)item + 0x40) * 0xC + 0x2) != 3)
        return func_002D6B28(item);

    if (item == (void *)D_00259128) {
        *(s32 *)g_nLevelExitDestination = 1;
        g_nLevelExitRequested = 1;
        return 0;
    }
    if (!(*(s32 *)(D_138180 + 0x1C4) & 0x40))
        return func_002D6B28(item);

    if (item == (void *)D_00259178)      { dest = 1; enabled = D_1A7BD1; }
    else if (item == (void *)D_002591C8) { dest = 2; enabled = D_1A7BD2; }
    else if (item == (void *)D_00259218) { dest = 3; enabled = D_1A7BD3; }
    else if (item == (void *)D_00259268) { dest = 4; enabled = D_1A7BD4; }
    else if (item == (void *)D_002592B8) { dest = 6; enabled = D_1A7BD6; }
    else {
        /* unrecognised widget: leave the destination untouched */
        if (g_nLevelExitRequested == 0)
            *(s32 *)g_nLevelExitDestination = savedDest;
        return 0;
    }

    *(s32 *)g_nLevelExitDestination = dest;
    g_nLevelExitRequested = (enabled != 0);
    if (g_nLevelExitRequested == 0)
        *(s32 *)g_nLevelExitDestination = savedDest;
    return 0;
}
#endif

/* Galactic-map / level-select input dispatcher. Reads g_padButtonsPressed:
 *   - any nav-button bit (mask 0x910) set: returns 1 (the key is swallowed);
 *   - confirm bit (0x40) pressed: refresh the panel (func_0029CFA0) and capture
 *     its return as the SELECTION, then route to the matching confirm handler —
 *     func_002CC908, func_002CC7D8, func_002CCA18, or the func_002CC788/
 *     func_002CC858 dispatch chosen by the 0x31/0x1C map-cell codes from
 *     func_0026F800/func_0026F7F8 — passing the selection and returning the
 *     handler's result. The func_0026F7D0/D8/F0/E8 queries only GATE which
 *     handler runs; they are NOT the handler argument.
 *   - otherwise just refreshes the panel (func_0029CFA0).
 * If a handler accepted (result != 0), plays the confirm SFX (id 0x12).
 * EU twin func_002CE0B0 (byte-identical; region-shifted call targets).
 * Routes to tester-EE: drives live map/GUI state + PlayGlobalSound; not
 * standalone cmp-oracle'able.
 * Wall: the selection (func_0029CFA0's return) is moved into a saved reg in the
 * delay slot of the NEXT call (jal func_0026F7D0; daddu $16,$2,$0 — captures $2
 * BEFORE func_0026F7D0 runs, i.e. func_0029CFA0's result), via the EE 64-bit
 * `daddu rd,rs,zero` move idiom (plus a 1-GPR packed save) — not reproduced from
 * clean C. Preserved as portable C. */
extern s32 func_0029CFA0(s32 buttons);
extern s32 func_0026F7D0(void);
extern s32 func_0026F7D8(void);
extern s32 func_0026F7E8(void);
extern s32 func_0026F7F0(void);
extern s32 func_0026F7F8(void);
extern s32 func_0026F800(void);
extern s32 func_002CC788(s32 q);
extern s32 func_002CC7D8(s32 q);
extern s32 func_002CC858(s32 q);
extern s32 func_002CC908(s32 q);
extern s32 func_002CCA18(s32 q);
extern void PlayGlobalSound(s32 id, s32 a, s32 b);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE0C8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 97.25% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 99.95% -> PACKED-SAVE, first differing row @1: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. */
/* TODO(match): functional equivalent - not byte-exact. On the sdk29 arm (-O2 -G8
 * -fno-gcse, solo) the whole residual is 4 words, all save-stride: 0x2CE0CC/0x2CE1F8
 * frame -16/+16 vs -32/+32, 0x2CE0D8/0x2CE1EC `sd/ld ra` at 8 vs 16; the daddu
 * moves and every other word already match (task #823). The 16-byte GPR slot is
 * fixed in cc1 2.9-ee-991111 (19 -m/-mabi/-mcpu variants, -mgp32 included, all keep
 * it; the held 2.95.3/2.95.2 cc1s save with sq), so no C or unit flag reaches it. */
s32 func_002CE0C8(void) {
    s32 buttons = g_padButtonsPressed;
    s32 result = 0;

    if (buttons & 0x910) {
        result = 1;
    } else if (buttons & 0x40) {
        /* sel = func_0029CFA0(buttons)'s return ($16): refresh the popup panel,
         * forwarding the pad-buttons word ($a0 = g_padButtonsPressed at the asm
         * jal site), and reuse its return as the argument to every confirm
         * handler. The func_0026F7D0/D8/F0/E8 queries only gate which handler
         * runs. (Dropping the buttons arg drove a phantom nav sound at
         * 0x1886D0 — the BUG #22 residual.) */
        s32 sel = func_0029CFA0(buttons);
        if (func_0026F7D0() != 0 && func_0026F7D8() != 0) {
            result = func_002CC908(sel);
        } else if (func_0026F7F0() != 0) {
            result = func_002CC7D8(sel);
        } else if (func_0026F7E8() != 0) {
            result = func_002CCA18(sel);
        } else if (func_0026F800() == 0x31 || func_0026F7F8() != 0 ||
                   func_0026F800() == 0x1C) {
            result = func_002CC788(sel);
        } else {
            result = func_002CC858(sel);
        }
    } else {
        func_0029CFA0(buttons);
    }

    if (result != 0) {
        PlayGlobalSound(0x12, 0, 0);
    }
    return result;
}
#endif

/* Draw-batch wrapper: render one menu sub-element inside a 2D batch. */
s32 func_002CE200(void) {
    Begin2dDrawBatch(0);
    func_0029CF70();
    End2dDrawBatch();
    return 0;
}

/* Present-record fence variant driving the front-end "leave" navigation. Samples
 * the frame timestamp (func_00337D98), then on the pad "confirm" bit (0x10) points
 * g_pNextMenuScreen at the destination screen (D_00259308) and — if the GUI's
 * pending widget handle (g_guiInstance+0x38000 .+0x79EC) is live — hands it to
 * func_0034F868; on a "cancel" bit (0x900) does the same handle hand-off and
 * returns 1; otherwise ticks the idle handler func_0029D040. Then, with the GUI
 * up, runs the shared present-record redraw fence (D_00259E50 / live object
 * *D_259E34) exactly like func_002CE498: clear the "needs redraw" bit 0x4 when the
 * two timestamps agree, the file-load is idle and the record shows this frame
 * already presented; else set it. Finally stamps the record (D_00259E50[0x58]=now).
 * Matching arm stays INCLUDE_ASM (later cc1 packs 8-byte save slots vs our 16). */
extern s32 func_00337D98(void);
extern void func_0034F868(s32 handle);
extern void func_0029D040(s32 buttons);
extern u8 *g_pNextMenuScreen;
extern u8 D_00259308[];
extern u8 D_00259E50[];   /* per-screen present record */
extern s32 D_259E34;      /* ptr-to-live-object global */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE230);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 54.46% -> STRUCTURAL,
 * first differing row @9: ROM `andi v1,a0,0x10` vs `andi v0,a0,0x10`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 59.88% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-48`. */
s32 func_002CE230(void) {
    s32 t0 = func_00337D98();
    s32 flags = g_padButtonsPressed;
    s32 result = 0;

    if (flags & 0x10) {
        g_pNextMenuScreen = D_00259308;
        if (g_guiInstance == NULL)
            return result;
        if (*(s32 *)(g_guiInstance + 0x38000 + 0x79EC) != 0)
            func_0034F868(*(s32 *)(g_guiInstance + 0x38000 + 0x79EC));
    } else if (flags & 0x900) {
        if (g_guiInstance != NULL &&
            *(s32 *)(g_guiInstance + 0x38000 + 0x79EC) != 0)
            func_0034F868(*(s32 *)(g_guiInstance + 0x38000 + 0x79EC));
        result = 1;
    } else {
        func_0029D040(flags);
    }

    if (g_guiInstance) {
        s32 now = func_00337D98();
        u8 *rec = D_00259E50;
        s32 *live = (s32 *)D_259E34;
        s32 clear = 0;
        if (now == t0 && g_fileLoadState == 0) {
            if (*(s32 *)(rec + 0x50) == now && *(s32 *)(rec + 0x44) == 2) {
                clear = 1;
            } else if (*(s32 *)(rec + 0x54) == now &&
                       *(s32 *)(rec + 0x44) == 4) {
                clear = 1;
            }
        }
        if (clear) {
            live[0x10 / 4] &= ~0x4;
        } else {
            live[0x10 / 4] |= 0x4;
        }
        *(s32 *)(rec + 0x58) = now;
    }
    return result;
}
#endif

/* Insomniac-museum (or sibling extras) screen draw: inside a 2D batch, run the
 * per-screen overlay (func_0029CFE0) and draw localized title string 0x2BE5 at
 * (D_1AB9D8, D_1AB9DC) in 0x80F0F0F0; then, if the GUI is up and its museum
 * widget handle (g_guiInstance+0x38000 .+0x79EC) is non-null, render that moby
 * model (BeginMobyDrawSegment .. FinishMobyRenderChain + the func_0034F9xx model
 * setup chain), wait one DMA fence and patch the moby packet's TEX0.
 * EU twin func_002CE388 (byte-identical; string id 0xB60, widget +0x7A9C,
 * region-shifted call/data targets).
 * Routes to tester-EE: drives the live GUI moby + DMA render path; not
 * standalone cmp-oracle'able.
 * Wall: `beql` branch-likely on the null-handle guard + EE 64-bit `daddu rd,rs,
 * zero` handle-copy idiom (1-GPR packed save) — not reproduced from clean C.
 * Preserved as portable C. */
extern void func_0029CFE0(void);
extern void func_002801B8(s32 x, s32 y, u64 color, char *str, s64 sel);
extern s32 D_1AB9D8, D_1AB9DC;
extern void BeginMobyDrawSegment(void);
extern void func_002A1000(void);
extern void func_002A1028(void);
extern void func_002A1058(void);
extern void FinishMobyRenderChain(void);
extern void func_0034F928(s32 handle);
extern void func_0034F9B8(s32 handle);
extern void func_0034F9F8(s32 handle);
extern void func_0034FAF8(s32 handle, s32 arg);
extern void WaitFrameDmaFence(s32 mask);
extern void PatchMobyPacketTex0(void);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE3A0);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 88.39% -> BIGDISP-SPLIT (0x38000 constant: 2.96 emits the large-displacement macro, the ROM and 2.9 split it & ~0x7fff),
 * first differing row @10: ROM `daddu a3,v0,zero` vs `(none)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 91.93% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. */
/* TODO(match): functional equivalent - not byte-exact; `beql` branch-likely null
 * guard + 64-bit `daddu` handle-copy idiom not reproduced by cc1. */
s32 func_002CE3A0(void) {
    s32 handle;

    Begin2dDrawBatch(0);
    func_0029CFE0();
    func_002801B8(D_1AB9D8, D_1AB9DC, 0x80F0F0F0, GetLocalizedString(0x2BE5), -1);
    End2dDrawBatch();

    if (g_guiInstance != NULL &&
        *(s32 *)(g_guiInstance + 0x38000 + 0x79EC) != 0) {
        BeginMobyDrawSegment();
        func_002A1000();
        func_002A1028();
        handle = *(s32 *)(g_guiInstance + 0x38000 + 0x79EC);
        func_0034F928(handle);
        func_0034F9B8(handle);
        func_0034F9F8(handle);
        func_0034FAF8(handle, handle + 0xC00);
        func_002A1058();
        FinishMobyRenderChain();
        WaitFrameDmaFence(0x10);
        PatchMobyPacketTex0();
    }
    return 0;
}
#endif

/* Per-screen menu tick + render-fence latch (one of the func_002CE498 family,
 * the cleanest with the standard menuScreenBlock confirm latch). Confirm (0x10)
 * latches the active screen's pending result (block[0x14]->0xE0 into block[0x18],
 * else -1/0); cancel (0x900) returns 1; otherwise it ticks the idle handler
 * func_0029D080(buttons, &scratch). Then, with the GUI up, it samples the frame
 * timestamp twice via func_00337D98 (a gp-relative frame counter); when the two
 * reads agree, the file-load is idle, and the per-screen present record
 * (D_00259C58) shows this frame already presented (offsets 0x50/0x54 == now and
 * state 0x44 in {2,4}) it CLEARS the "needs redraw" bit 0x4 of the live object
 * (*D_259C24)[0x10]; otherwise it SETS that bit. Finally it stamps the present
 * record (D_00259C58[0x58] = now).
 * Wall: 8-byte-packed-save ($16 + $17 + $31). Preserved as portable C. */
extern s32 func_00337D98(void);
extern void func_0029D080(s32 buttons, void *scratch);
extern s16 g_fileLoadState;
extern s32 D_259C24;       /* ptr-to-live-object global */
extern u8 D_00259C58[];    /* per-screen present record */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE498);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 60.84% -> FRAME-SIZE,
 * first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-80`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 58.47% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-96`. */
/* TODO(match): functional equivalent - not byte-exact; 3-GPR packed-save frame +
 * branch-likely present-record fence shape. */
s32 func_002CE498(void) {
    s32 t0 = func_00337D98();
    s32 flags = g_padButtonsPressed;
    s32 result = 0;
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        result = 1;
    } else {
        u8 scratch[0x30];
        func_0029D080(flags, scratch);
    }
    if (g_guiInstance) {
        s32 now = func_00337D98();
        u8 *rec = D_00259C58;
        s32 *live = (s32 *)D_259C24;
        s32 clear = 0;
        if (now == t0 && g_fileLoadState == 0) {
            if (*(s32 *)(rec + 0x50) == now && *(s32 *)(rec + 0x44) == 2) {
                clear = 1;
            } else if (*(s32 *)(rec + 0x54) == now &&
                       *(s32 *)(rec + 0x44) == 4) {
                clear = 1;
            }
        }
        if (clear) {
            live[0x10 / 4] &= ~0x4;
        } else {
            live[0x10 / 4] |= 0x4;
        }
        *(s32 *)(rec + 0x58) = now;
    }
    return result;
}
#endif

/* Draw-batch wrapper. */
s32 func_002CE5D8(void) {
    Begin2dDrawBatch(0);
    func_0029D010();
    End2dDrawBatch();
    return 0;
}

/* return 0 stub. */
s32 func_002CE608(void) {
    return 0;
}

/* return 0 stub. */
s32 func_002CE610(void) {
    return 0;
}

/* Confirm/cancel poll variant: on the "confirm" pad bit (0x10) acknowledges the
 * input (func_0028C7A8) then returns the active screen's pending result (latched
 * into block[0x18]) or -1 when the screen has no pending sub-result; on a
 * "back/cancel" bit (0x900) acknowledges + returns 1; otherwise ticks the idle
 * handler func_0029DA18 and returns 0.
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern void func_0028C7A8(void);
extern s32 func_0029DA18(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE618);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 78.14% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 80.03% -> SPLIT-HIREG, first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely confirm shape. */
s32 func_002CE618(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v;
        func_0028C7A8();
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        func_0028C7A8();
        return 1;
    }
    func_0029DA18(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002CE6A8(void) {
    Begin2dDrawBatch(0);
    func_0029DA58();
    End2dDrawBatch();
    return 0;
}

/* Confirm/cancel poll + present-record fence (record D_0025E298 / live *D_25E264),
 * the standard menuScreenBlock confirm latch. Samples the frame timestamp
 * (func_00337D98); confirm (0x10) latches the active screen's pending result
 * (block[0x14]->0xE0 into block[0x18], else -1/0); cancel (0x900) returns 1; else
 * ticks idle handler func_0029D2B8. With the GUI up, clears the "needs redraw" bit
 * 0x4 when the two timestamps agree, the file-load is idle and the record shows
 * this frame already presented; else sets it. When the timestamp advanced during
 * the tick it also latches D_25E444 = -0x22C, then re-samples the timestamp to
 * stamp the record (D_0025E298[0x58]).
 * Matching arm stays INCLUDE_ASM (later cc1 packs 8-byte save slots vs our 16). */
extern void func_0029D2B8(s32 buttons);
extern u8 D_0025E298[];   /* per-screen present record */
extern s32 D_25E264;      /* ptr-to-live-object global */
extern s32 D_25E444;
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE6D8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 65.25% -> SCHED-TIEBREAK,
 * first differing row @1: ROM `sd s0,0(sp)` vs `(none)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 59.39% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-64`. */
s32 func_002CE6D8(void) {
    s32 t0 = func_00337D98();
    s32 flags = g_padButtonsPressed;
    s32 result = 0;

    if (flags & 0x10) {
        s32 *block = (s32 *)g_menuScreenBlock;
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            result = 0;
        } else {
            result = (block[0x134 / 4] == 0) ? -1 : 0;
        }
    } else if (flags & 0x900) {
        result = 1;
    } else {
        func_0029D2B8(flags);
    }

    if (g_guiInstance) {
        s32 now = func_00337D98();
        u8 *rec = D_0025E298;
        s32 *live = (s32 *)D_25E264;
        s32 clear = 0;
        if (now == t0 && g_fileLoadState == 0) {
            if (*(s32 *)(rec + 0x50) == t0 && *(s32 *)(rec + 0x44) == 2) {
                clear = 1;
            } else if (*(s32 *)(rec + 0x54) == t0 &&
                       *(s32 *)(rec + 0x44) == 4) {
                clear = 1;
            }
        }
        if (clear) {
            live[0x10 / 4] &= ~0x4;
        } else {
            live[0x10 / 4] |= 0x4;
        }
        if (now != t0) {
            D_25E444 = -0x22C;
        }
        *(s32 *)(rec + 0x58) = func_00337D98();
    }
    return result;
}
#endif

/* GUI wrapper: when the GUI is up, register a widget (instance + 0x3A000) and
 * stash the returned handle in widget[0x34].
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern s32 func_00345298(void *widget);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE830);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 67.35% -> STRUCTURAL,
 * first differing row @1: ROM `lui v0,0x0  [HI16 0x001A8D04]` vs `(none)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 99.12% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002CE830(s32 *out) {
    if (g_guiInstance) {
        out[0xD] = func_00345298(g_guiInstance + 0x3A000);
    }
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002CE878(void) {
    Begin2dDrawBatch(0);
    func_0029D288();
    End2dDrawBatch();
    return 0;
}

/* GUI accessor: when the GUI is up, mark the widget at instance+0x3C160 active
 * (func_00342460(w, 1)) and store its queried value (func_00342468(w)) into
 * out[0x34]. Returns 0.
 * Wall: 8-byte-packed-save ($16 + $17 + $31). Preserved as portable C. */
extern void func_00342460(void *widget, s32 arg);
extern s32 func_00342468(void *widget);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE8A8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 64.12% -> STRUCTURAL,
 * first differing row @1: ROM `lui v0,0x0  [HI16 0x001A8D04]` vs `sd s0,0(sp)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 75.29% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-48`. */
/* TODO(match): functional equivalent - not byte-exact; 3-GPR packed-save frame. */
s32 func_002CE8A8(s32 *out) {
    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C160;
        func_00342460(w, 1);
        out[0x34 / 4] = func_00342468(w);
    }
    return 0;
}
#endif

/* Menu confirm/cancel poll + command builder, twin of func_002D0158 without the
 * arg==0 sound. Confirm (0x10) latches the active screen's pending result
 * (block[0x14]->0xE0 into block[0x18], else -1/0); cancel (0x900) returns 1;
 * otherwise it ticks the idle handler func_0029D328 and, on the confirm pad bit
 * (0x40) with the GUI up, reads the selected entry of the list widget at
 * g_guiInstance+0x3C160 (func_003424C8) and builds an 8-byte command record
 * (op = (u16)entry[0x8] at rec+0x2, arg = entry[0xC] at rec+0x4) handed to
 * func_002D6B00 (-> MenuScreenDoAction).
 * Wall: 8-byte-packed-save ($16 + $17 + $31). Preserved as portable C. */
extern s32 func_0029D328(s32 padPressed);
extern void *func_003424C8(void *widget);
extern void func_002D6B00(void *record);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE908);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 67.61% -> FRAME-SIZE,
 * first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-64`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 61.65% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-80`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely confirm shape / single-register result threading. */
s32 func_002CE908(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D328(flags);
    if ((*(s32 *)(D_138180 + 0x1C4) & 0x40) && g_guiInstance) {
        u8 record[0x30];
        u8 *entry = (u8 *)func_003424C8(g_guiInstance + 0x3C160);
        *(u16 *)(record + 0x2) = *(u16 *)(entry + 0x8);
        *(s32 *)(record + 0x4) = *(s32 *)(entry + 0xC);
        func_002D6B00(record);
    }
    return 0;
}
#endif

/* Draw the localized string 0x2BE5 right-justified at the alternate label slot. */
s32 func_002CE9D8(void) {
    char *str;
    Begin2dDrawBatch(0);
    func_0029D2F8();
    str = GetLocalizedString(0x2BE5);
    func_00280090(D_1AB9E0, D_1AB9E4, 0x80F0F0F0, str, -1);
    End2dDrawBatch();
    return 0;
}

/* GUI wrapper: when the GUI is up, configure the widget at instance+0x3C160 —
 * mark it active (func_00342460(w,1)), bind its three data blobs
 * (func_00342450(w, &D_2615D8, &D_261678, &D_261730)) and clear its selection
 * (func_00342520(w,0)). Returns 0.
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern void func_00342460(void *widget, s32 arg);
extern void func_00342450(void *widget, void *a, void *b, void *c);
extern void func_00342520(void *widget, s32 arg);
extern u8 D_2615D8, D_261678, D_261730;
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CEA38);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 63.23% -> STRUCTURAL,
 * first differing row @1: ROM `lui a0,0x0  [HI16 0x001A8D04]` vs `(none)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 73.35% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002CEA38(void) {
    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C160;
        func_00342460(w, 1);
        func_00342450(w, &D_2615D8, &D_261678, &D_261730);
        func_00342520(w, 0);
    }
    return 0;
}
#endif

/* UpdateBestiaryMenuInput: per-frame input for the bestiary browser (g_bestiaryCursor
 * over the catalog, 1..0x3F). Back (0x10) pops the menu-screen block (returns the
 * popped value). Exit (0x900) returns 1. Left (0x8000)/right (0x2000) move the cursor
 * to the precomputed previous/next entry (clamped to 1 / 0x3F), with the move sound
 * (or the edge-deny sound when already at that entry). It then recomputes the nearest
 * defeated neighbours by scanning g_bestiaryKillCounts (u16[2] per entry) down for the
 * previous entry with kills (floor 1) and up for the next (ceil = cursor), and updates
 * the panel display (D_0025ABA0+0x58 / D_0025AC08+0x34) from whether the current entry
 * has been seen, poking D_0025AC08+0x3C when the cursor moved. Returns 1 on exit, the
 * popped value on back, else 0.
 * (matching arm left INCLUDE_ASM: 8-byte-packed-save wall.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateBestiaryMenuInput);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 48.75% -> STRUCTURAL,
 * first differing row @3: ROM `(none)` vs `sd s1,8(sp)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 45.98% -> PACKED-SAVE, first differing row @1: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-64`. */
extern s32 g_bestiaryPrevEntry;
extern s32 g_bestiaryNextEntry;
extern u8 g_bestiaryKillCounts[];
extern u8 g_bestiaryEntryTable[];
extern u8 D_0025ABA0[];
extern u8 D_0025AC08[];
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
s32 UpdateBestiaryMenuInput(void) {
    s32 buttons = g_padButtonsPressed;
    s32 cursor0 = g_bestiaryCursor;
    s32 ret = 0;
    s32 cursor, i;

    if (buttons & 0x10) {           /* back */
        s32 e0 = *(s32 *)(*(u8 **)(g_menuScreenBlock + 0x14) + 0xE0);
        if (e0 != 0) {
            *(s32 *)(g_menuScreenBlock + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(g_menuScreenBlock + 0x134) == 0) ? -1 : 0;
    }

    if (buttons & 0x900) {          /* exit */
        ret = 1;
    } else if (buttons & 0x8000) {  /* left: move to the previous entry */
        s32 prev = g_bestiaryPrevEntry;
        PlayGlobalSound((cursor0 == prev) ? 5 : 3, 0, 0);
        g_bestiaryCursor = (prev > 0) ? prev : 1;
    } else if (buttons & 0x2000) {  /* right: move to the next entry */
        s32 next = g_bestiaryNextEntry;
        PlayGlobalSound((cursor0 == next) ? 5 : 3, 0, 0);
        g_bestiaryCursor = (next < 0x40) ? next : 0x3F;
    }

    /* recompute the nearest defeated entries around the cursor */
    cursor = g_bestiaryCursor;
    g_bestiaryPrevEntry = cursor - 1;
    g_bestiaryNextEntry = cursor + 1;

    /* scan down for the previous entry with any kills (floor at 1) */
    i = cursor - 1;
    for (;;) {
        if (*(u16 *)(g_bestiaryKillCounts + i * 4) != 0 ||
            *(u16 *)(g_bestiaryKillCounts + i * 4 + 2) != 0) {
            break;
        }
        i--;
        if (i <= 0) {
            i = 1;
            break;
        }
    }
    g_bestiaryPrevEntry = i;

    /* scan up for the next entry with any kills (back to cursor if none) */
    i = cursor + 1;
    for (;;) {
        if (*(u16 *)(g_bestiaryKillCounts + i * 4) != 0 ||
            *(u16 *)(g_bestiaryKillCounts + i * 4 + 2) != 0) {
            break;
        }
        i++;
        if (i >= 0x40) {
            i = cursor;
            break;
        }
    }
    g_bestiaryNextEntry = i;

    /* update the panel display state for the current entry */
    if (*(u16 *)(g_bestiaryKillCounts + cursor * 4) != 0 ||
        *(u16 *)(g_bestiaryKillCounts + cursor * 4 + 2) != 0) {
        *(s32 *)(D_0025ABA0 + 0x58) = cursor - 1;
        *(s32 *)(D_0025AC08 + 0x34) =
            *(s16 *)(g_bestiaryEntryTable + cursor * 0x18 + 0xA);
    } else {
        *(s32 *)(D_0025ABA0 + 0x58) = 0x3F;
        *(s32 *)(D_0025AC08 + 0x34) = 0;
    }
    if (g_bestiaryCursor != cursor0) {
        *(s32 *)(D_0025AC08 + 0x3C) = -0x240;
    }
    return ret;
}
#endif

/* DrawBestiaryEntry: renders the bestiary ("Monsterpedia") detail page for the
 * selected enemy (g_bestiaryCursor). Four title glyphs (0xAE..0xB1), six static
 * stat labels (0x3089/0x308A/0x3088/0x308B plus two composed via D_1AB9F8), and a
 * couple of section captions. If the enemy has been encountered (either half of
 * its g_bestiaryKillCounts entry is non-zero) the detail is drawn: four stat bars
 * (func_002904B0 fills whose length is the entry's stat halfword *0x10/0x12/0x14/
 * 0x16* scaled by 2.44 and clamped to 244), two paging arrows (glyphs 0x4A/0x4B
 * that bob with the shared gadget animation and light up in the colour-pulse
 * colour from func_002AA3F0 unless their pad direction is held), the species name
 * (entry +0x8, centred, drop-shadow suppressed), a caption (entry +0xC), an
 * optional note (entry +0xE, or D_1ABA00 when -1), and the kill count line
 * (D_1ABA10 formatted with the count). Twin of DrawExtrasMenu.
 *
 * TODO(match): functional equivalent - not byte-exact. 8-byte-packed-save wall
 * (saves 9 GPRs incl $31; later cc1 packs save slots 8-byte vs our 16-byte) plus
 * FP-arg scheduling; preserved as portable C, the matching arm stays INCLUDE_ASM.
 * (Stat-bar length uses (s32)(v) for cvt.w.s, per the in-unit convention.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawBestiaryEntry);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 39.40% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-160` vs `lui v1,0x0  [HI16 0x001A8D04]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 49.63% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-160` vs `addiu sp,sp,-320`. */
extern u32 func_002AA3F0(u32 color1, u32 color2, s32 period, s32 counterSel, s32 reset);
extern void func_003017F8(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                          f32 px, f32 py, f32 sx, f32 syg, f32 v38);
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void func_002904B0(s32 x0, s32 y0, s32 x1, s32 y1, s32 color, s32 flag);
extern void func_00280250(s32 x, s32 y, u32 color, const char *str, s32 flag);
extern s32 func_001157AC(const char *s);             /* SDK strlen */
extern s32 func_0027F790(void);                      /* set the sprite drop-shadow flag */
extern void func_0027F7A0(void);                     /* clear the sprite drop-shadow flag */
extern s32 g_screenWidth;
extern s32 g_swapGadgetItemIndex;                    /* +0x8E y-fudge (f32), +0x86 anim phase (s32) */
extern s32 g_padButtonsHeld;
extern s32 D_1AB9F4;                                 /* title glyph row (int -> float) */
extern char D_1AB9F8[];                              /* stat-label sprintf format */
extern char D_1ABA00[];                              /* fallback note string */
extern char D_1ABA10[];                              /* kill-count sprintf format */
s32 DrawBestiaryEntry(void) {
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(g_screenWidth / 2);
    f32 titleRow = (f32)D_1AB9F4;
    f32 yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    s32 animPhase = *(s32 *)((u8 *)&g_swapGadgetItemIndex + 0x86);
    s32 cursor = g_bestiaryCursor;
    u16 *kills = (u16 *)(g_bestiaryKillCounts + cursor * 4);
    u32 pulseColor;
    char buf[0x40];

    pulseColor = func_002AA3F0(0x60442D00, 0x80FFDE8D, 0x19, 0, 0);
    Begin2dDrawBatch(0);

    /* four title glyphs (v38 = 0.0) */
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xAE), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, titleRow, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xAF), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, titleRow, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xB0), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, titleRow, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xB1), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, titleRow, 1.0f, yfudge, 0.0f);

    /* static stat captions */
    DrawDebugString(0x1F, 0x3E, 0x80F0F0F0, GetLocalizedString(0x3089), -1);
    DrawDebugString(0x1F, 0x5C, 0x80F0F0F0, GetLocalizedString(0x308A), -1);
    DrawDebugString(0x1F, 0x7A, 0x80F0F0F0, GetLocalizedString(0x3088), -1);
    DrawDebugString(0x1F, 0x98, 0x80F0F0F0, GetLocalizedString(0x308B), -1);
    func_00115DA8(buf, D_1AB9F8, GetLocalizedString(0x308C));
    DrawDebugString(0x1F, 0xB8, 0x80F0F0F0, buf, func_001157AC(buf));
    func_00115DA8(buf, D_1AB9F8, GetLocalizedString(0x308D));
    DrawDebugString(0x1F, 0xC9, 0x80F0F0F0, buf, func_001157AC(buf));

    func_00280120(0x1DB, 0x175, 0x80F0F0F0, GetLocalizedString(0x2DD8), -1);
    func_002801B8(0x9A, 0x1A, 0x80F0F0F0, GetLocalizedString(0x305C), -1);

    /* enemy encountered? (either half of the kill-count entry set) */
    if (kills[0] != 0 || kills[1] != 0) {
        u8 *entry = g_bestiaryEntryTable + cursor * 0x18;
        const s16 statOff[4] = { 0x10, 0x12, 0x14, 0x16 };
        const s16 barY0[4]   = { 0x4E, 0x6C, 0x8B, 0xA9 };
        const s16 barY1[4]   = { 0x58, 0x76, 0x96, 0xB4 };
        s32 k;

        /* four stat bars: length = stat * 2.44, clamped to 244 */
        for (k = 0; k < 4; k++) {
            s32 len = (s32)((f32)(*(s16 *)(entry + statOff[k])) * 2.44f);
            if (len >= 0xF5) {
                len = 0xF4;
            }
            func_002904B0(0x1F, barY0[k], len + 0x1F, barY1[k], 0x55F0C070, 0);
        }

        /* "next" paging arrow */
        if (cursor < g_bestiaryNextEntry) {
            u32 color = (g_padButtonsHeld & 0x2000) ? 0x80F0F0F0 : pulseColor;
            s32 glyph = GuiFontAtlasLookupGlyph(atlas, 0x4A);
            f32 bob = yfudge * 0.8f;
            if (animPhase == 1) {
                bob += -0.05f;
            }
            func_003017F8(glyph, color, (f32 *)0, (f32 *)0, 316.0f, 278.0f, 1.0f, bob, 0.0f);
            func_00280120(0x1D6, 0x114, 0x80F0F0F0, GetLocalizedString(0x2DD7), -1);
        }

        /* "prev" paging arrow */
        if (cursor >= 2) {
            u32 color = (g_padButtonsHeld & 0x8000) ? 0x80F0F0F0 : pulseColor;
            s32 glyph = GuiFontAtlasLookupGlyph(atlas, 0x4B);
            f32 bob = yfudge * 0.8f;
            if (animPhase == 1) {
                bob += -0.05f;
            }
            func_003017F8(glyph, color, (f32 *)0, (f32 *)0, 207.0f, 278.0f, 1.0f, bob, 0.0f);
            DrawDebugString(0x37, 0x114, 0x80F0F0F0, GetLocalizedString(0x2DD6), -1);
        }

        /* species name (centred), drop-shadow suppressed */
        func_0027F7A0();
        func_00280250(g_screenWidth / 2, 0x114, 0x80F0F0F0,
                      GetLocalizedString(*(s16 *)(entry + 0x8)), -1);
        func_0027F790();

        DrawDebugString(0x9C, 0xB9, 0x80F0F0F0, GetLocalizedString(*(s16 *)(entry + 0xC)), -1);

        if (*(s16 *)(entry + 0xE) == -1) {
            func_00115DA8(buf, D_1ABA00);
            DrawDebugString(0x9C, 0xCA, 0x80F0F0F0, buf, func_001157AC(buf));
        } else {
            DrawDebugString(0x9C, 0xCA, 0x80F0F0F0,
                            GetLocalizedString(*(s16 *)(entry + 0xE)), -1);
        }

        /* kill count line */
        func_00115DA8(buf, D_1ABA10, kills[0], GetLocalizedString(0x2DF7));
        func_00280250(0x98, 0xE4, 0x80F0F0F0, buf, func_001157AC(buf));
    }
    End2dDrawBatch();
    return 0;
}
#endif

/* Reset the bestiary cursor to entry 1. */
s32 func_002CF540(void) {
    g_bestiaryCursor = 1;
    return 0;
}

/* return 0 stub. */
s32 func_002CF550(void) {
    return 0;
}

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 4 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawBestiaryPagingArrows);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 6 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawMenuPagingChrome);

/* DrawMenuItemSelectionBox: draw the four-sided highlight box around a menu item,
 * horizontally centred on screen. `width` sets the half-extent (width/2 + 5 either
 * side of screen centre); four func_002904B0 fills form the top (y 0x138..0x13A),
 * bottom (0x14D..0x14F), left and right (0x139..0x14E) borders, all in `color`.
 * (matching arm left INCLUDE_ASM: 8-byte-packed-save wall — cc1 packs the 6-GPR
 * save frame 8-byte vs our 16-byte.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawMenuItemSelectionBox);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 76.00% -> STRUCTURAL,
 * first differing row @3: ROM `addu v0,v0,a0` vs `addu v0,a0,v0`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 74.75% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-96`. */
extern s32 g_screenWidth;
extern void func_002904B0(s32 x0, s32 y0, s32 x1, s32 y1, s32 color, s32 flag);
void DrawMenuItemSelectionBox(s32 width, s32 color) {
    s32 half = width / 2 + 5;
    s32 cx = g_screenWidth / 2;
    s32 right = cx + half;
    s32 left = cx - half;
    s32 x0 = left - 2;
    s32 x1 = right + 4;

    func_002904B0(x0, 0x138, x1, 0x13A, color, 0);        /* top */
    func_002904B0(x0, 0x14D, x1, 0x14F, color, 0);        /* bottom */
    func_002904B0(x0, 0x139, left, 0x14E, color, 0);      /* left */
    func_002904B0(right + 2, 0x139, x1, 0x14E, color, 0); /* right */
}
#endif

/* GUI wrapper: when the GUI is up, register a widget (instance + 0x3C160) and
 * stash the returned handle in widget[0x34].
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern s32 func_00342468(void *widget);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D0110);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 67.35% -> STRUCTURAL,
 * first differing row @1: ROM `lui v0,0x0  [HI16 0x001A8D04]` vs `(none)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 99.12% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002D0110(s32 *out) {
    if (g_guiInstance) {
        out[0xD] = func_00342468(g_guiInstance + 0x3C160);
    }
    return 0;
}
#endif

/* Menu confirm/cancel poll + command builder, twin of func_002D4270. Confirm
 * (0x10) latches the active screen's pending result (block[0x14]->0xE0 into
 * block[0x18], else -1/0); cancel (0x900) returns 1; otherwise it ticks the idle
 * handler func_0029D398 and, on the confirm pad bit (0x40) with the GUI up, reads
 * the selected entry of the list widget at g_guiInstance+0x3C160 (func_003424C8)
 * and builds an 8-byte command record (op = (u16)entry[0x8] at rec+0x2, arg =
 * entry[0xC] at rec+0x4). When arg is 0 it plays UI sound 5, then hands the
 * record to func_002D6B00 (-> MenuScreenDoAction).
 * Wall: 8-byte-packed-save ($16 + $17 + $31). Preserved as portable C. */
extern s32 func_0029D398(s32 padPressed);
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D0158);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 66.89% -> FRAME-SIZE,
 * first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-64`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 73.84% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-80`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely confirm shape / single-register result threading. */
s32 func_002D0158(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D398(flags);
    if ((*(s32 *)(D_138180 + 0x1C4) & 0x40) && g_guiInstance) {
        u8 record[0x30];
        u8 *entry = (u8 *)func_003424C8(g_guiInstance + 0x3C160);
        s32 arg = *(s32 *)(entry + 0xC);
        *(u16 *)(record + 0x2) = *(u16 *)(entry + 0x8);
        *(s32 *)(record + 0x4) = arg;
        if (arg == 0) {
            PlayGlobalSound(5, 0, 0);
        }
        func_002D6B00(record);
    }
    return 0;
}
#endif

/* Draw the localized string 0x2BE5 as a right-justified label inside a 2D batch. */
s32 func_002D0240(void) {
    char *str;
    Begin2dDrawBatch(0);
    func_0029D368();
    str = GetLocalizedString(0x2BE5);
    func_00280090(D_1ABA28, D_1ABA2C, 0x80F0F0F0, str, -1);
    End2dDrawBatch();
    return 0;
}

/* Refreshes the weapon/gadget-vendor availability record (D_261730) and three
 * global availability flags from the current inventory. Scans the 0x38 item slots
 * (skipping empty g_weaponTable entries and the two non-vendor slots 9/10): any
 * slot the player does not own clears "all owned" (D_1AA450); for owned slots, if
 * any of the three weapon-variant fields (+0x98/+0x9C/+0xA0) exists but its
 * corresponding g_itemStateFlags bit (1<<(2*variant)) is not yet set, it clears the
 * "all variants seen" flags (D_1AA454/D_1AA458). Then populates the D_261730 record
 * with the caption string-ids / sub-page pointers for either the base set (when no
 * extras are unlocked, g_miscExtras==0) or the extras set, and lays out + resets the
 * vendor widget (func_00342450 / func_00342520 on g_guiInstance+0x3C160). Returns 0.
 * Matching arm stays INCLUDE_ASM (8-byte-packed-save + gp-rel scratch scheduling). */
extern u8 g_inventoryOwned[];   /* u8[0x38] per-item have-flag */
extern u8 g_itemStateFlags[];   /* u8[0x38] per-item persistent state bits */
extern u8 D_0025C430[], D_0025C5C8[], D_0025CCD8[];  /* extras sub-page records */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D02A0);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 38.85% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-64` vs `lui t6,0x0  [HI16 0x001A8D04]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 57.44% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-64` vs `addiu sp,sp,-32`. */
s32 func_002D02A0(void) {
    u8 *record;
    s32 i;

    if (g_guiInstance == NULL)
        return 0;

    D_1AA450 = 1;
    D_1AA454 = 1;
    D_1AA458 = 1;

    for (i = 0; i < 0x38; i++) {
        s32 slot = g_itemEquippedSlot[i];
        u8 *weapon = g_weaponTable + slot * 0xE0;
        s32 seen454, seen458;
        s32 j;

        if (*(s32 *)weapon == 0 || i == 9 || i == 0xA)
            continue;

        if (g_inventoryOwned[i] == 0) {
            D_1AA450 = 0;
            D_1AA454 = 0;
            D_1AA458 = 0;
            continue;
        }

        seen454 = D_1AA454;
        seen458 = D_1AA458;
        for (j = 0; j < 3; j++) {
            if (*(s32 *)(weapon + 0x98 + j * 4) != 0 &&
                (g_itemStateFlags[i] & (1 << (j * 2))) == 0) {
                seen454 = 0;
                seen458 = 0;
            }
        }
        D_1AA458 = seen458;
        D_1AA454 = seen454;
    }

    record = (u8 *)&D_261730;
    if (g_miscExtras == 0) {
        *(s32 *)(record + 0x68) = 0x2C56;
        *(s32 *)(record + 0x70) = 0;
        *(s32 *)(record + 0x40) = 0x2C56;
        *(s32 *)(record + 0x48) = 0;
        *(s32 *)(record + 0x54) = 0x2C56;
        *(s32 *)(record + 0x5C) = 0;
    } else {
        *(s32 *)(record + 0x70) = (s32)D_0025CCD8;
        *(s32 *)(record + 0x40) = 0x3098;
        *(s32 *)(record + 0x48) = (s32)D_0025C430;
        *(s32 *)(record + 0x54) = 0x30A2;
        *(s32 *)(record + 0x5C) = (s32)D_0025C5C8;
        *(s32 *)(record + 0x68) = 0x30D5;
    }

    func_00342450(g_guiInstance + 0x3C160, &D_2615D8, &D_261678, &D_261730);
    func_00342520(g_guiInstance + 0x3C160, 2);
    return 0;
}
#endif

/* UpdateCheatMenuInput: per-frame input for the cheats menu (cursor D_1ABA30 over
 * the 12-entry table D_1ABD50, stride 4: +0x2 cheat id, +0x3 unlock gate). Reads the
 * screen's held buttons (D_138180+0x1C4). Back (0x10) resets the cursor and pops the
 * menu-screen block. Exit (0x900) resets the cursor and returns 1. Up (0x1000)/down
 * (0x4000) move to the previous/next non-disabled entry (gate 0x64 = disabled),
 * wrapping, with the move sound. Confirm (0x40) is allowed when the entry is
 * unlocked — the special gate 0x5A requires g_skillPointFlags, otherwise the player's
 * completed skill-point count must reach the gate value; on allow it plays the accept
 * sound and toggles g_cheatFlags[cheatId], reloading the player display model for the
 * model-changing cheats (2/9/10/0xB); on deny it plays the reject sound. Returns 1 on
 * exit, the popped value on back, else 0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateCheatMenuInput);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 40.93% -> BIGDISP-SPLIT + STRUCTURAL,
 * first differing row @1: ROM `(none)` vs `sd s1,8(sp)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 57.15% -> PACKED-SAVE, first differing row @2: ROM `sd s1,8(sp)` vs `sd ra,16(sp)`. */
extern s32 CountSkillPointsCompleted(void);
extern u8 D_138180[];
extern u8 g_menuScreenBlock[];
extern s32 D_1ABA30;
extern u8 D_1ABD50[];
extern u8 g_cheatFlags[];
extern u8 g_skillPointFlags;
extern s16 g_equippedArmor;
extern void LoadPlayerDisplayModel(s16 armor);
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
s32 UpdateCheatMenuInput(void) {
    s32 skillPts = CountSkillPointsCompleted();
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 gate, cheatId, allow;

    if (flags & 0x10) {                 /* back */
        s32 e0 = *(s32 *)(*(u8 **)(g_menuScreenBlock + 0x14) + 0xE0);
        D_1ABA30 = 0;
        if (e0 != 0) {
            *(s32 *)(g_menuScreenBlock + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(g_menuScreenBlock + 0x134) == 0) ? -1 : 0;
    }

    if (flags & 0x900) {                /* exit */
        D_1ABA30 = 0;
        return 1;
    }

    if (flags & 0x1000) {               /* up: previous enabled entry */
        PlayGlobalSound(3, 0, 0);
        do {
            D_1ABA30--;
            if (D_1ABA30 < 0) {
                D_1ABA30 = 0xB;
            }
        } while (D_1ABD50[D_1ABA30 * 4 + 3] == 0x64);
    } else if (flags & 0x4000) {        /* down: next enabled entry */
        PlayGlobalSound(3, 0, 0);
        do {
            D_1ABA30++;
            if (D_1ABA30 >= 0xC) {
                D_1ABA30 = 0;
            }
        } while (D_1ABD50[D_1ABA30 * 4 + 3] == 0x64);
    }

    /* confirm */
    if (!(*(s32 *)(D_138180 + 0x1C4) & 0x40)) {
        return 0;
    }
    gate = D_1ABD50[D_1ABA30 * 4 + 3];
    if (gate == 0x5A) {
        allow = (g_skillPointFlags != 0);
    } else {
        allow = !(skillPts < gate);
    }
    if (!allow) {
        PlayGlobalSound(5, 0, 0);       /* reject */
        return 0;
    }
    PlayGlobalSound(4, 0, 0);           /* accept */

    cheatId = D_1ABD50[D_1ABA30 * 4 + 2];
    g_cheatFlags[cheatId] = (g_cheatFlags[cheatId] != 0) ? 0 : 1;

    cheatId = D_1ABD50[D_1ABA30 * 4 + 2];
    if (cheatId == 9 || cheatId == 10 || cheatId == 2 || cheatId == 0xB) {
        LoadPlayerDisplayModel(g_equippedArmor);
    }
    return 0;
}
#endif

/* DrawCheatMenu: renders the cheats menu. Three title glyphs (codepoints
 * 0x8B/0x8C/0x8D) and three header strings, then the 12-entry table D_1ABD50
 * (stride 4: +0x0 name string id, +0x2 cheat-flag index, +0x3 unlock gate).
 * Entries with gate 0x64 are hidden (skipped). An entry is "available" when its
 * gate is met — the special gate 0x5A needs g_skillPointFlags, otherwise the
 * player's completed skill-point count (CountSkillPointsCompleted) must reach the
 * gate value. Available rows draw the localized cheat name at x=0x2B and its
 * ON/OFF state (GetLocalizedString of 0x2C5C/0x2C5D by g_cheatFlags[id]) at
 * x=0x195. Locked rows draw a "???" placeholder (string 0x2C56) plus a right-
 * aligned hint composed with sprintf ("needs N skill points" via D_1ABA10, or the
 * special-gate hint D_1ABA38 for 0x5A), drawn with the drop-shadow flag briefly
 * cleared (func_0027F7A0 / func_0027F790). The row under the cursor (D_1ABA30) is
 * drawn in the highlight color 0x7029A1FF. Twin of DrawExtrasMenu; input sibling
 * is UpdateCheatMenuInput above.
 *
 * TODO(match): functional equivalent - not byte-exact. 8-byte-packed-save wall
 * (saves 9 GPRs incl $31; later cc1 packs save slots 8-byte vs our 16-byte) plus
 * FP-arg scheduling; preserved as portable C, the matching arm stays INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawCheatMenu);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 5.29% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-160` vs `lui v0,0x0  [HI16 0x001A7340]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 14.47% -> STRUCTURAL, first differing row @0: ROM `addiu sp,sp,-160` vs `lui v0,0x0  [HI16 0x001A7340]`. */
extern void func_003017F8(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                          f32 px, f32 py, f32 sx, f32 syg, f32 v38);
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void func_00280250(s32 x, s32 y, u32 color, const char *str, s32 flag);
extern s32 func_001157AC(const char *s);          /* SDK strlen */
extern s32 func_0027F790(void);                   /* set the sprite drop-shadow flag */
extern void func_0027F7A0(void);                  /* clear the sprite drop-shadow flag */
extern s32 g_swapGadgetItemIndex;                 /* +0x8E holds the sprite y-fudge (f32) */
extern s32 D_1ABA34;                              /* title glyph row (int -> float) */
extern char D_1ABA10[];                           /* "needs N skill points" sprintf format */
extern char D_1ABA38[];                           /* special-gate hint sprintf format */
s32 DrawCheatMenu(void) {
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(g_screenWidth / 2);
    f32 titleRow = (f32)D_1ABA34;
    f32 yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    s32 skillPts = CountSkillPointsCompleted();
    s32 cursor = D_1ABA30;
    u8 *entry = D_1ABD50;
    s32 y = 0x70;
    s32 i;
    char buf[0x40];

    Begin2dDrawBatch(0);
    /* three title glyphs */
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0x8B), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, titleRow, 1.0f, yfudge, 0.84f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0x8C), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, titleRow, 1.0f, yfudge, 0.84f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0x8D), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, titleRow, 1.0f, yfudge, 0.84f);
    /* three header strings */
    func_002801B8(g_screenWidth / 2, 0x41,  0x80F0F0F0, GetLocalizedString(0x2CA8), -1);
    func_002801B8(g_screenWidth / 2, 0x141, 0x80F0F0F0, GetLocalizedString(0x2BE4), -1);
    func_002801B8(g_screenWidth / 2, 0x15A, 0x80F0F0F0, GetLocalizedString(0x2BE5), -1);

    for (i = 0; i < 0xC; i++, entry += 4) {
        u8 gate = entry[3];
        u32 color;
        s32 available;

        if (gate == 0x64) {             /* hidden slot */
            continue;
        }
        if (gate == 0x5A) {
            available = (g_skillPointFlags != 0);
        } else {
            available = !(skillPts < gate);
        }

        if (available) {
            color = (i == cursor) ? 0x7029A1FF : 0x80F0F0F0;
            DrawDebugString(0x2B, y, color, GetLocalizedString(*(s16 *)entry), -1);
            func_00280250(0x195, y, color,
                          GetLocalizedString(g_cheatFlags[entry[2]] ? 0x2C5C : 0x2C5D), -1);
        } else {
            color = (i == cursor) ? 0x7029A1FF : 0x80808080;
            DrawDebugString(0x2B, y, color, GetLocalizedString(0x2C56), -1);
            if (gate == 0x5A) {
                func_00115DA8(buf, D_1ABA38, GetLocalizedString(0x2CBE));
            } else {
                func_00115DA8(buf, D_1ABA10, gate, GetLocalizedString(0x2CA6));
            }
            func_0027F7A0();            /* suppress drop-shadow for the small hint */
            func_00280250(0x195, y, color, buf, func_001157AC(buf));
            func_0027F790();            /* restore drop-shadow */
        }
        y += 0x13;
    }
    End2dDrawBatch();
    return 0;
}
#endif

/* return 0 stub. */
s32 func_002D0B40(void) {
    return 0;
}

/* UpdateSkillPointsMenu: per-frame input for the skill-points menu (cursor
 * g_nSkillPointsMenuCursor over the 30 skill points). Back (0x10) resets the cursor
 * and pops the menu-screen block. Exit (0x900) resets the cursor and returns 1. Up
 * (0x1000)/down (0x4000) move the cursor (wrapping 0..0x1D) with the move sound. It
 * then updates the info panel for the current entry (D_0025C098+0x58 = cursor): when
 * the skill point is completed (g_skillPointFlags[cursor]) it records its meta id
 * (g_skillPointMetaTable[cursor*6 + 2] -> D_25C074) and, if the cursor did not move
 * and no file load is pending and the panel is showing this same entry
 * (D_0025C098+0x50/+0x54 == cursor with mode +0x44 == 2/4), clears bit 0x4 of the
 * render object (*D_25C004 + 0x10); when not completed it sets that bit and zeroes
 * D_25C074. Returns 1 on exit, the popped value on back, else 0.
 * (matching arm left INCLUDE_ASM: 8-byte-packed-save wall.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateSkillPointsMenu);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 14.20% -> GPREL-vs-ABS,
 * first differing row @1: ROM `(none)` vs `lw a1,0(gp)  [GPREL16 0x001ABA3C]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 42.36% -> PACKED-SAVE, first differing row @1: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-48`. */
extern s32 g_nSkillPointsMenuCursor;
extern u8 g_skillPointMetaTable[];
extern u8 D_0025C098[];
extern u8 *D_25C004;
extern s32 D_25C074;
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
s32 UpdateSkillPointsMenu(void) {
    s32 buttons = g_padButtonsPressed;
    s32 cursor0 = g_nSkillPointsMenuCursor;
    s32 ret = 0;
    s32 cursor;

    if (buttons & 0x10) {           /* back */
        s32 e0 = *(s32 *)(*(u8 **)(g_menuScreenBlock + 0x14) + 0xE0);
        g_nSkillPointsMenuCursor = 0;
        if (e0 != 0) {
            *(s32 *)(g_menuScreenBlock + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(g_menuScreenBlock + 0x134) == 0) ? -1 : 0;
    }

    if (buttons & 0x900) {          /* exit */
        g_nSkillPointsMenuCursor = 0;
        ret = 1;
    } else if (buttons & 0x1000) {  /* up */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = g_nSkillPointsMenuCursor - 1;
        g_nSkillPointsMenuCursor = (v >= 0) ? v : 0x1D;
    } else if (buttons & 0x4000) {  /* down */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = g_nSkillPointsMenuCursor + 1;
        g_nSkillPointsMenuCursor = (v < 0x1E) ? v : 0;
    }

    /* refresh the info panel for the current cursor */
    cursor = g_nSkillPointsMenuCursor;
    *(s32 *)(D_0025C098 + 0x58) = cursor;

    if ((&g_skillPointFlags)[cursor] != 0) {
        /* completed skill point */
        s32 clear = 0;
        if (cursor0 == cursor && g_fileLoadState == 0) {
            s32 mode = *(s32 *)(D_0025C098 + 0x44);
            if (*(s32 *)(D_0025C098 + 0x50) == cursor0 && mode == 2) {
                clear = 1;
            } else if (*(s32 *)(D_0025C098 + 0x54) == cursor0 && mode == 4) {
                clear = 1;
            }
        }
        if (clear) {
            *(s32 *)(D_25C004 + 0x10) &= ~0x4;
        }
        D_25C074 = *(s16 *)(g_skillPointMetaTable + cursor * 6 + 2);
    } else {
        /* not yet completed */
        *(s32 *)(D_25C004 + 0x10) |= 0x4;
        D_25C074 = 0;
    }
    return ret;
}
#endif

/* DrawSkillPointsMenu: renders the skill-points ("Trophies") menu. Four title
 * glyphs (codepoints 0xD7..0xDA), a header (string 0x2CA6) and the running
 * completed/total count line ("N / 30", format string 0x2DC5, count from
 * CountSkillPointsCompleted) drawn with the drop-shadow flag cleared, a second
 * header (0x2BE5), and a divider fill (func_002904B0). If the cursor's meta entry
 * has a detail label (g_skillPointMetaTable stride 6, +0x4 halfword != -1) it is
 * drawn at (0x1A0,0xE0). Finally a five-slot carousel centred on
 * g_nSkillPointsMenuCursor: each slot's index wraps modulo 30 (0x1E), its RGB is
 * gold (0x7029A1FF) when that skill point is completed (g_skillPointFlags[idx])
 * else white (0x80F0F0F0), and its alpha byte encodes focus distance
 * (edges 0x10, neighbours 0x50, centre 0x70). The centre slot also gets a
 * text-width selection box. Each slot's label is drawn with the drop-shadow
 * cleared. Twin of DrawExtrasMenu.
 *
 * TODO(match): functional equivalent - not byte-exact. 8-byte-packed-save wall
 * (saves 8 GPRs incl $31; later cc1 packs save slots 8-byte vs our 16-byte) plus
 * FP-arg scheduling; preserved as portable C, the matching arm stays INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawSkillPointsMenu);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 35.61% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-144` vs `lui v0,0x0  [HI16 0x001A7340]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 49.84% -> STRUCTURAL, first differing row @0: ROM `addiu sp,sp,-144` vs `lui v0,0x0  [HI16 0x001A7340]`. */
extern void func_003017F8(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                          f32 px, f32 py, f32 sx, f32 syg, f32 v38);
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void func_00280250(s32 x, s32 y, u32 color, const char *str, s32 flag);
extern void func_0027FBA8(s32 x, s32 y, u64 color, char *str, s64 wrap);
extern s32 func_0027F818(const char *str, s32 len);  /* menu text pixel width */
extern s32 func_001157AC(const char *s);             /* SDK strlen */
extern s32 func_0027F790(void);                      /* set the sprite drop-shadow flag */
extern void func_0027F7A0(void);                     /* clear the sprite drop-shadow flag */
extern void DrawMenuPagingChrome(void);
extern s32 g_swapGadgetItemIndex;                    /* +0x8E holds the sprite y-fudge (f32) */
extern s32 D_1ABA40;                                 /* title glyph row (int -> float) */
s32 DrawSkillPointsMenu(void) {
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(g_screenWidth / 2);
    f32 titleRow = (f32)D_1ABA40;
    f32 yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    u8 *flags = &g_skillPointFlags;                  /* base of the 30 per-skill completion flags */
    s32 cursor = g_nSkillPointsMenuCursor;
    s32 y;
    s32 i;
    char *fmt;
    s32 count;
    char buf[0x40];

    Begin2dDrawBatch(0);
    /* four title glyphs (v38 = 0.0) */
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD7), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, titleRow, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD8), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, titleRow, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD9), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, titleRow, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xDA), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, titleRow, 1.0f, yfudge, 0.0f);

    /* header + "N / 30" completed-count line, drop-shadow suppressed */
    func_0027F7A0();
    func_002801B8(0xB9, 0x1B, 0x80F0F0F0, GetLocalizedString(0x2CA6), -1);
    fmt = GetLocalizedString(0x2DC5);
    count = CountSkillPointsCompleted();
    func_00115DA8(buf, fmt, count, 0x1E);
    func_0027FBA8(0x92, 0x177, 0x80F0F0F0, buf, -1);
    func_0027F790();

    func_002801B8(0x1AD, 0x177, 0x80F0F0F0, GetLocalizedString(0x2BE5), -1);
    func_002904B0(0x15E, 0xDA, 0x1E6, 0xDC, 0x55F0C070, 0);

    /* selected entry's detail label, if present */
    if (*(s16 *)(g_skillPointMetaTable + cursor * 6 + 4) != -1) {
        func_0027F7A0();
        func_002801B8(0x1A0, 0xE0, 0x80F0F0F0,
                      GetLocalizedString(*(s16 *)(g_skillPointMetaTable + cursor * 6 + 4)), -1);
        func_0027F790();
    }

    DrawMenuPagingChrome();

    /* five-slot carousel centred on the cursor */
    y = 0x112;
    for (i = 0; i < 5; i++) {
        s32 idx = i + cursor - 2;
        char *label;
        u32 color;

        if (idx < 0) {
            idx += 0x1E;
        }
        if (idx >= 0x1E) {
            idx -= 0x1E;
        }
        label = GetLocalizedString(*(s16 *)(g_skillPointMetaTable + idx * 6));
        color = (flags[idx] ? 0x7029A1FF : 0x80F0F0F0) & 0xFFFFFF;
        if (i == 0 || i == 4) {
            color |= 0x10000000;                     /* edge slots, dim */
        } else if (i == 1 || i == 3) {
            color |= 0x50000000;                     /* neighbours */
        } else {                                     /* i == 2: focused slot */
            color |= 0x70000000;
            DrawMenuItemSelectionBox(func_0027F818(label, func_001157AC(label)), color);
        }
        func_0027F7A0();
        func_00280250(g_screenWidth / 2, y, color, label, -1);
        func_0027F790();
        y += 0x14;
    }
    End2dDrawBatch();
    return 0;
}
#endif

/* return 0 stub. */
s32 func_002D1150(void) {
    return 0;
}

/* UpdateExtrasMenuInput: per-frame input for the Extras menu, a 5-item carousel
 * (g_extrasMenuCursor in [0,4]). Back (0x10) resets the cursor and latches the
 * active screen's pending sub-result (block[0x14]->0xE0 into block[0x18], else
 * -1/0). Cancel (0x900) resets the cursor and returns 1. Up (0x1000)/down (0x4000)
 * play UI sound 3 and move the cursor with wrap in [0,4]. Confirm (0x40) dispatches
 * via jtbl_0026CCE0_text: cursor 0 requests the sub-screen g_pNextMenuScreen =
 * D_25C298; cursors 1..4 act only when enabled (D_1ABA48[cursor] != 0, else play
 * sound 5) — cursor 1 freezes the screen and enters the making-of state
 * (CaptureScreenToVram + RequestGameStateChange(9,1,...)); cursors 2/3/4 queue
 * making-of/credits reels (0xBF / 0xBD+0xBE / 0xC1). Queued reels are then enqueued
 * and started (EnqueueCinematic x1..2 + StartCinematicFromQueue on g_cinematicQueue,
 * after stashing the screen fields like CinematicsMenuTick). A deferred UI sound
 * (4 on success, 5 when disabled) is played once. Every non-back path then runs the
 * present-record redraw fence (D_0025C230 record, live object *D_25C1F0): SET redraw
 * bit 0x4 when disabled, else CLEAR it only when the cursor is unchanged, the
 * file-load is idle, and the present record shows this cursor already presented
 * (offsets 0x50/0x54 == cursor and state 0x44 in {2,4}).
 * Wall: switch/jump-table dispatch (splat jtbl reloc gap) + branch-likely fence —
 * cc1 jtbl layout not reproduced. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateExtrasMenuInput);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 45.39% -> FRAME-SIZE,
 * first differing row @0: ROM `addiu sp,sp,-80` vs `addiu sp,sp,-64`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 46.13% -> STRUCTURAL, first differing row @0: ROM `addiu sp,sp,-80` vs `(none)`. */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
extern void CaptureScreenToVram(s32 mode);
extern s32 EnqueueCinematic(void *queue, s32 reelId);
extern s32 StartCinematicFromQueue(void *queue);
extern s32 RequestGameStateChange(s32 stateId, s32 push, s32 c, s32 d, s32 e);
extern s32 g_extrasMenuCursor;   /* extras carousel cursor 0..4 */
extern s32 D_1ABA48[];           /* per-item enabled flags */
extern u8 *g_pNextMenuScreen;    /* 0x1F27D8 - requested next screen */
extern u8 D_25C298[];            /* extras sub-screen record */
extern s32 g_cinematicExitPending; /* 0x1A7478 - exit-cinematic-pending flag */
extern u8 g_cinematicQueue[];    /* 0x1BACC0 - pending-cinematic reel queue record */
extern s32 g_cameraCallbackCount; /* 0x1B1480 - the making-of path writes +0x80 */
extern u8 D_0025C230[];          /* per-screen present record */
extern u8 *D_25C1F0;             /* pointer to the live menu object */
s32 UpdateExtrasMenuInput(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 cursor0 = g_extrasMenuCursor;   /* cursor at entry, for the fence check */
    s32 result = 0;
    s32 sound = -1;                     /* deferred UI sound (4 ok, 5 disabled) */
    s32 reel0 = -1, reel1 = -1;         /* queued cinematic reel ids */
    u8 *mb = g_menuScreenBlock;
    s32 cursor;

    if (flags & 0x10) {                 /* back */
        s32 e0 = *(s32 *)(*(u8 **)(mb + 0x14) + 0xE0);
        g_extrasMenuCursor = 0;
        if (e0 != 0) {
            *(s32 *)(mb + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(mb + 0x134) == 0) ? -1 : 0;
    }

    if (flags & 0x900) {                /* cancel */
        g_extrasMenuCursor = 0;
        return 1;
    } else if (flags & 0x1000) {        /* up */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = g_extrasMenuCursor - 1;
        g_extrasMenuCursor = (v >= 0) ? v : 4;
    } else if (flags & 0x4000) {        /* down */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = g_extrasMenuCursor + 1;
        g_extrasMenuCursor = (v < 5) ? v : 0;
    }

    flags = *(s32 *)(D_138180 + 0x1C4);
    if ((flags & 0x40) && (u32)g_extrasMenuCursor < 5) {   /* confirm */
        cursor = g_extrasMenuCursor;
        switch (cursor) {
        case 0:
            g_pNextMenuScreen = D_25C298;
            sound = 4;
            break;
        case 1:
            if (D_1ABA48[cursor] == 0) {
                sound = 5;
            } else {
                /* freeze the screen + enter the making-of state */
                CaptureScreenToVram(1);
                *(s32 *)((u8 *)&g_cameraCallbackCount + 0x80) = 2;
                *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
                *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
                RequestGameStateChange(9, 1, 0, 0, 0);
                sound = 4;
            }
            break;
        case 2:
            if (D_1ABA48[cursor] == 0) { sound = 5; }
            else { sound = 4; reel0 = 0xBF; }
            break;
        case 3:
            if (D_1ABA48[cursor] == 0) { sound = 5; }
            else { sound = 4; reel0 = 0xBD; reel1 = 0xBE; }
            break;
        case 4:
            if (D_1ABA48[cursor] == 0) { sound = 5; }
            else { sound = 4; reel0 = 0xC1; }
            break;
        }
    }

    /* enqueue + start any requested reels */
    if (reel0 != -1) {
        CaptureScreenToVram(0);
        *(s32 *)(mb + 0x140) = g_cinematicExitPending;
        *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
        *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
        g_cinematicExitPending = 2;
        EnqueueCinematic(g_cinematicQueue, reel0);
        if (reel1 != -1) {
            EnqueueCinematic(g_cinematicQueue, reel1);
        }
        StartCinematicFromQueue(g_cinematicQueue);
    }
    if (sound != -1) {
        PlayGlobalSound(sound, 0, 0);
    }

    /* present-record redraw fence */
    cursor = g_extrasMenuCursor;
    *(s32 *)(D_0025C230 + 0x58) = cursor;
    if (D_1ABA48[cursor] == 0) {
        *(s32 *)(D_25C1F0 + 0x10) |= 0x4;
    } else if (cursor0 == cursor && g_fileLoadState == 0) {
        s32 mode = *(s32 *)(D_0025C230 + 0x44);
        if ((*(s32 *)(D_0025C230 + 0x50) == cursor0 && mode == 2) ||
            (*(s32 *)(D_0025C230 + 0x54) == cursor0 && mode == 4)) {
            *(s32 *)(D_25C1F0 + 0x10) &= ~0x4;
        }
    }
    return result;
}
#endif

/* DrawExtrasMenu: renders the Extras (5-item) menu screen — same chrome as
 * DrawPlanetWarpMenu. Opens a 2D batch, blits four title glyphs (atlas
 * g_guiInstance+0x8710, codepoints 0xD7/0xD8/0xD9/0xDD, colors 0x60442D00/0x60241700/
 * 0x55F0C070/0x55F0C070, centred at row D_1ABA5C, scale 1.0, y-fudge, v38 0.0); draws
 * three header strings (0x3095 @0xB3,0x1B; 0x2BE5 @0x161,0x177; 0x2C0B @0xB5,0x177;
 * color 0x80F0F0F0); draws the paging chrome; then a five-slot cursor-relative
 * carousel (cursor g_extrasMenuCursor, 5 items). Each slot i shows item at wrapped
 * index (i + cursor - 2) mod 5, labelled with the localized D_1ABD80[idx] (stride 4,
 * low halfword) when enabled (D_1ABA48[idx] != 0) else 0x2C56; slot color fades by
 * distance from centre (edges 0x10/mid 0x50/centre 0x70 F0F0F0), and the centre slot
 * gets a text-width selection box.
 * (func_003017F8 ignores its scale/vec38 params — see its body — so glyphs pass NULL.)
 * Wall: 8-byte-packed-save (6 GPRs) + FP-arg scheduling — later cc1 save-slot packing
 * not reproduced. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawExtrasMenu);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 36.23% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-64` vs `lui v0,0x0  [HI16 0x001A7340]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 36.41% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-64` vs `addiu sp,sp,-208`. */
extern void func_003017F8(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                          f32 px, f32 py, f32 sx, f32 syg, f32 v38);
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void DrawMenuPagingChrome(void);
extern s32 func_001157AC(const char *s);          /* SDK strlen */
extern s32 func_0027F818(const char *str, s32 len); /* menu text pixel width */
extern void func_00280250(s32 x, s32 y, u32 color, const char *str, s32 flag);
extern s32 g_extrasMenuCursor;   /* extras carousel cursor 0..4 */
extern s32 D_1ABA48[];           /* per-item enabled flags */
extern u8 D_1ABD80[];            /* per-item label string ids (stride 4) */
extern s32 D_1ABA5C;             /* title glyph row (int, converted to float) */
extern s32 g_swapGadgetItemIndex; /* +0x8E holds the global sprite y-fudge (f32) */
s32 DrawExtrasMenu(void) {
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(g_screenWidth / 2);
    f32 row = (f32)D_1ABA5C;
    f32 yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    s32 cursor;
    s32 i;
    s32 y;

    Begin2dDrawBatch(0);
    /* four title glyphs (v38 = 0.0) */
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD7), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD8), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD9), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xDD), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    /* three header strings */
    func_002801B8(0xB3, 0x1B, 0x80F0F0F0, GetLocalizedString(0x3095), -1);
    func_002801B8(0x161, 0x177, 0x80F0F0F0, GetLocalizedString(0x2BE5), -1);
    func_002801B8(0xB5, 0x177, 0x80F0F0F0, GetLocalizedString(0x2C0B), -1);
    DrawMenuPagingChrome();
    /* five cursor-relative carousel slots */
    cursor = g_extrasMenuCursor;
    y = 0x112;
    for (i = 0; i < 5; i++) {
        s32 idx = i + cursor - 2;
        s32 strId;
        char *str;
        u32 color;
        if (idx < 0) idx += 5;
        if (idx >= 5) idx -= 5;
        strId = (D_1ABA48[idx] == 0) ? 0x2C56 : *(s16 *)(D_1ABD80 + idx * 4);
        str = GetLocalizedString(strId);
        if (i == 0 || i == 4) {
            color = 0x10F0F0F0;
        } else if (i == 1 || i == 3) {
            color = 0x50F0F0F0;
        } else {  /* i == 2: focused slot gets a text-width selection box */
            color = 0x70F0F0F0;
            DrawMenuItemSelectionBox(func_0027F818(str, func_001157AC(str)), 0x70F0F0F0);
        }
        func_00280250(g_screenWidth / 2, y, color, str, -1);
        y += 0x14;
    }
    End2dDrawBatch();
    return 0;
}
#endif

/* When the GUI is up, latch the extras-menu availability flags: always mark the
 * last screen id (=1), then set the per-feature "new" flags for each unlocked
 * extras feature (D_1AA450/D_1AA458) and, if any extras are unlocked
 * (g_miscExtras), the museum + master flags. Leaf.
 * Near-miss (~69%): the original fills each beqz delay slot with the next flag
 * store (and emits the museum/master pair in the opposite commutative order);
 * our cc1 leaves nops in the delay slots and stores in source order — the
 * later-cc1 branch-fill scheduling we can't reproduce from clean C. Preserved
 * as portable C. */
extern s32 g_lastMenuScreenId;
extern s32 D_1ABA50, D_1ABA54, D_1ABA58, D_1ABA4C;

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D1850);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 57.40% -> STRUCTURAL,
 * first differing row @3: ROM `lui v1,0x0  [HI16 0x001F27C8]` vs `(none)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 84.00% -> STRUCTURAL, first differing row @11: ROM `beq v0,zero,L` vs `(none)`. */
/* TODO(match): functional equivalent - not byte-exact; delay-slot branch-fill +
 * commutative store order not reproduced by cc1. */
s32 func_002D1850(void) {
    if (g_guiInstance) {
        g_lastMenuScreenId = 1;
        if (D_1AA450) {
            D_1ABA50 = 1;
        }
        if (D_1AA458) {
            D_1ABA54 = 1;
        }
        if (g_miscExtras) {
            D_1ABA58 = 1;
            D_1ABA4C = 1;
        }
    }
    return 0;
}
#endif

/* CinematicsMenuTick: per-frame input for the Goodies "Cinematics" screen, a
 * 33-reel carousel over g_cinematicsMenuTable (6-byte entries: u16 nameStringId,
 * u16 reelId @+2, u16 unlocked @+4). Back (0x10) resets the cursor and latches the
 * active screen's pending sub-result (block[0x14]->0xE0 into block[0x18], else
 * -1/0). Cancel (0x900) resets the cursor and returns 1. Up (0x1000)/down (0x4000)
 * play UI sound 3 and move the cursor with wrap in [0,0x20]. Confirm (0x40) on an
 * unlocked reel plays sound 4, freezes the screen (CaptureScreenToVram), stashes
 * the old g_cinematicExitPending + the screen block's fields 0x14/0x8 into
 * block[0x140/0x100/0x104], sets g_cinematicExitPending = 2, then enqueues and
 * starts the reel (EnqueueCinematic/StartCinematicFromQueue on g_cinematicQueue);
 * a locked reel just plays sound 5. Every non-back/non-cancel path then runs the
 * present-record redraw fence (D_0025C3C8 record, live object *D_25C388): SET
 * redraw bit 0x4 when the reel is locked, else CLEAR it only when the cursor is
 * unchanged, the file-load is idle, and the present record shows this cursor
 * already presented (offsets 0x50/0x54 == cursor and state 0x44 in {2,4}).
 * Wall: 8-byte-packed-save (6 GPRs) + branch-likely present-record fence — later
 * cc1 save-slot packing not reproduced. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", CinematicsMenuTick);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 30.52% -> FRAME-SIZE,
 * first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-32`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 55.12% -> STRUCTURAL, first differing row @0: ROM `addiu sp,sp,-48` vs `lui v1,0x0  [HI16 D_138180]`. */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
extern void CaptureScreenToVram(s32 mode);
extern s32 EnqueueCinematic(void *queue, s32 reelId);
extern s32 StartCinematicFromQueue(void *queue);
extern s32 D_1ABA60;             /* 0x1ABA60 - cinematics carousel cursor 0..0x20 */
extern u8 g_cinematicsMenuTable[]; /* 0x261838 - 33 x 6-byte reel entries */
extern s32 g_cinematicExitPending; /* 0x1A7478 - exit-cinematic-pending flag */
extern u8 g_cinematicQueue[];    /* 0x1BACC0 - pending-cinematic reel queue record */
extern u8 D_0025C3C8[];          /* per-screen present record */
extern u8 *D_25C388;             /* pointer to the live menu object */
s32 CinematicsMenuTick(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 cursor0 = D_1ABA60;              /* cursor at entry, for the fence check */
    s32 result = 0;
    s32 cursor;

    if (flags & 0x10) {                 /* back */
        s32 e0 = *(s32 *)(*(u8 **)(g_menuScreenBlock + 0x14) + 0xE0);
        D_1ABA60 = 0;
        if (e0 != 0) {
            *(s32 *)(g_menuScreenBlock + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(g_menuScreenBlock + 0x134) == 0) ? -1 : 0;
    }

    if (flags & 0x900) {                /* cancel */
        D_1ABA60 = 0;
        return 1;
    } else if (flags & 0x1000) {        /* up */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = D_1ABA60 - 1;
        D_1ABA60 = (v >= 0) ? v : 0x20;
    } else if (flags & 0x4000) {        /* down */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = D_1ABA60 + 1;
        D_1ABA60 = (v < 0x21) ? v : 0;
    }

    flags = *(s32 *)(D_138180 + 0x1C4);
    if (flags & 0x40) {                 /* confirm */
        cursor = D_1ABA60;
        if (*(s16 *)(g_cinematicsMenuTable + cursor * 6 + 4) != 0) {
            s16 reelId;                 /* unlocked reel: enqueue + play */
            PlayGlobalSound(4, 0, 0);
            CaptureScreenToVram(0);
            cursor = D_1ABA60;
            reelId = *(s16 *)(g_cinematicsMenuTable + cursor * 6 + 2);
            *(s32 *)(g_menuScreenBlock + 0x140) = g_cinematicExitPending;
            *(s32 *)(g_menuScreenBlock + 0x100) = *(s32 *)(g_menuScreenBlock + 0x14);
            *(s32 *)(g_menuScreenBlock + 0x104) = *(s32 *)(g_menuScreenBlock + 0x8);
            g_cinematicExitPending = 2;
            EnqueueCinematic(g_cinematicQueue, reelId);
            StartCinematicFromQueue(g_cinematicQueue);
        } else {
            PlayGlobalSound(5, 0, 0);   /* locked reel */
        }
    }

    /* present-record redraw fence */
    cursor = D_1ABA60;
    *(s32 *)(D_0025C3C8 + 0x58) = cursor;
    if (*(s16 *)(g_cinematicsMenuTable + cursor * 6 + 4) == 0) {
        *(s32 *)(D_25C388 + 0x10) |= 0x4;
    } else if (cursor0 == cursor && g_fileLoadState == 0) {
        s32 mode = *(s32 *)(D_0025C3C8 + 0x44);
        if ((*(s32 *)(D_0025C3C8 + 0x50) == cursor0 && mode == 2) ||
            (*(s32 *)(D_0025C3C8 + 0x54) == cursor0 && mode == 4)) {
            *(s32 *)(D_25C388 + 0x10) &= ~0x4;
        }
    }
    return result;
}
#endif

/* DrawCinematicsMenu: renders the cinematics (reel carousel) menu screen — same
 * chrome as DrawPlanetWarpMenu. Opens a 2D batch, blits four title glyphs (atlas
 * g_guiInstance+0x8710, codepoints 0xD7/0xD8/0xD9/0xDD, colors 0x60442D00/0x60241700/
 * 0x55F0C070/0x55F0C070, centred at row D_1ABA64, scale 1.0, y-fudge, v38 0.0); draws
 * three header strings (0x2CA9 @0xB3,0x1B; 0x2BE5 @0x161,0x177; 0x2C0B @0xB5,0x177;
 * color 0x80F0F0F0); draws the paging chrome; then a five-slot cursor-relative
 * carousel over g_cinematicsMenuTable (33 x 6-byte reels, cursor D_1ABA60). Each slot
 * i shows the reel at wrapped index (i + D_1ABA60 - 2) mod 0x21, labelled with the
 * localized reel name (entry+0x0) when unlocked (entry+0x4 != 0) else 0x2C56; slot
 * color fades by distance from centre (edges 0x10/mid 0x50/centre 0x70 F0F0F0), and
 * the centre slot gets a text-width selection box.
 * (func_003017F8 ignores its scale/vec38 params — see its body — so glyphs pass NULL.)
 * Wall: 8-byte-packed-save (6 GPRs) + FP-arg scheduling — later cc1 save-slot packing
 * not reproduced. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawCinematicsMenu);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 31.08% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-64` vs `lui v0,0x0  [HI16 0x001A7340]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 31.96% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-64` vs `addiu sp,sp,-208`. */
extern void func_003017F8(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                          f32 px, f32 py, f32 sx, f32 syg, f32 v38);
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void DrawMenuPagingChrome(void);
extern s32 func_001157AC(const char *s);          /* SDK strlen */
extern s32 func_0027F818(const char *str, s32 len); /* menu text pixel width */
extern void func_00280250(s32 x, s32 y, u32 color, const char *str, s32 flag);
extern u8 g_cinematicsMenuTable[]; /* 0x261838 - 33 x 6-byte reel entries */
extern s32 D_1ABA60;             /* cinematics carousel cursor */
extern s32 D_1ABA64;             /* title glyph row (int, converted to float) */
extern s32 g_swapGadgetItemIndex; /* +0x8E holds the global sprite y-fudge (f32) */
s32 DrawCinematicsMenu(void) {
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(g_screenWidth / 2);
    f32 row = (f32)D_1ABA64;
    f32 yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    s32 cursor;
    s32 i;
    s32 y;

    Begin2dDrawBatch(0);
    /* four title glyphs (v38 = 0.0) */
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD7), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD8), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD9), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xDD), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    /* three header strings */
    func_002801B8(0xB3, 0x1B, 0x80F0F0F0, GetLocalizedString(0x2CA9), -1);
    func_002801B8(0x161, 0x177, 0x80F0F0F0, GetLocalizedString(0x2BE5), -1);
    func_002801B8(0xB5, 0x177, 0x80F0F0F0, GetLocalizedString(0x2C0B), -1);
    DrawMenuPagingChrome();
    /* five cursor-relative carousel slots */
    cursor = D_1ABA60;
    y = 0x112;
    for (i = 0; i < 5; i++) {
        s32 idx = i + cursor - 2;
        s32 base;
        s32 strId;
        char *str;
        u32 color;
        if (idx < 0) idx += 0x21;
        if (idx >= 0x21) idx -= 0x21;
        base = idx * 6;
        strId = (*(s16 *)(g_cinematicsMenuTable + base + 4) != 0)
                    ? *(s16 *)(g_cinematicsMenuTable + base + 0) : 0x2C56;
        str = GetLocalizedString(strId);
        if (i == 0 || i == 4) {
            color = 0x10F0F0F0;
        } else if (i == 1 || i == 3) {
            color = 0x50F0F0F0;
        } else {  /* i == 2: focused slot gets a text-width selection box */
            color = 0x70F0F0F0;
            DrawMenuItemSelectionBox(func_0027F818(str, func_001157AC(str)), 0x70F0F0F0);
        }
        func_00280250(g_screenWidth / 2, y, color, str, -1);
        y += 0x14;
    }
    End2dDrawBatch();
    return 0;
}
#endif

/* When the GUI is up, set g_lastMenuScreenId=1 then walk the 32-entry, 6-byte-
 * stride cinematics-menu row table (D_26183A): for each row, if any extras are
 * unlocked (g_miscExtras) mark the row available (+8 field = 1); otherwise test
 * the row's cinematic id (the +6 field) against g_cinematicUnlockedFlags and
 * write the bit result (1 set / 0 clear) into the same +8 availability field.
 * EU twin func_002D1E10 (byte-identical; table D_2615E2, region-shifted).
 * Wall: `bnel` branch-likely (the extras-set fast path's available-store sits in
 * the nullified delay slot) — the later cc1's branch-likely emission isn't
 * reproduced from clean C. Preserved as portable C. */
extern u8 g_cinematicUnlockedFlags[]; /* 0x139768 - cinematic-watched/unlocked bitfield */
extern u8 D_26183A[];                 /* cinematics-menu row table (6-byte stride records) */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D1E88);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 39.40% -> STRUCTURAL,
 * first differing row @3: ROM `(none)` vs `lui v1,0x0  [HI16 D_26183A]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 34.75% -> STRUCTURAL, first differing row @3: ROM `addiu v0,zero,1` vs `lui v0,0x0  [HI16 D_26183A]`. */
/* TODO(match): functional equivalent - not byte-exact; `bnel` branch-likely
 * delay-slot store on the extras-unlocked fast path not reproduced by cc1. */
void func_002D1E88(void) {
    u8 *row;
    if (g_guiInstance == NULL) {
        return;
    }
    g_lastMenuScreenId = 1;
    for (row = D_26183A; row < D_26183A + 0xC0; row += 6) {
        if (g_miscExtras != 0) {
            *(s16 *)(row + 8) = 1;
        } else {
            s16 id = *(s16 *)(row + 6);
            u32 mask = 1u << (id & 0x1F);
            if (*(u32 *)(g_cinematicUnlockedFlags + (id & ~3)) & mask) {
                *(s16 *)(row + 8) = 1;
            } else {
                *(s16 *)(row + 8) = 0;
            }
        }
    }
}
#endif

/* UpdatePlanetWarpMenuInput: per-frame input for the planet-warp (retail warp)
 * menu, an 8-item cursor list. Back (0x10) resets the cursor and latches the
 * active screen's pending sub-result (block[0x14]->0xE0 into block[0x18], else
 * -1/0). Cancel (0x900) resets the cursor and returns 1. Up (0x1000)/down
 * (0x4000) play UI sound 3 and move the cursor with wrap in [0,7]. Confirm (0x40)
 * plays sound 4 for an enabled item (sound 5 for a disabled one) and, via the
 * cursor jump table, sets the warp mode D_1A7904 and calls RequestLevelExit for a
 * fixed planet/level id (cursor 7 also stashes g_playerProgress into D_1A7908 when
 * not already 0x15). Every non-back/non-cancel path then runs the present-record
 * redraw fence: stamp D_0025C560[0x58] = cursor; SET the live object's redraw bit
 * 0x4 (*D_25C520[0x10]) when the item is disabled, else CLEAR it only when the
 * cursor is unchanged, the file-load is idle, and the present record shows this
 * cursor already presented (offsets 0x50/0x54 == cursor and state 0x44 in {2,4}).
 * Cursor jump table is jtbl_0026CD00_text (in cursor order).
 * Wall: switch/jump-table dispatch (splat jtbl reloc gap) + branch-likely present-
 * record fence — cc1 jtbl layout not reproduced. Preserved as portable C. */
extern s32 g_planetWarpCursor;   /* 0x1ABA68 - planet-warp cursor 0..7 */
extern s32 D_1A7904;             /* warp-mode selector written before the exit */
extern s32 D_1A7908;             /* stashed prior progress slot (cursor-7 case) */
extern u8 D_0025C560[];          /* per-screen present record */
extern u8 *D_25C520;             /* pointer to the live menu object */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdatePlanetWarpMenuInput);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 36.04% -> STRUCTURAL,
 * first differing row @1: ROM `lui v0,0x0  [HI16 D_138180]` vs `(none)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 51.21% -> STRUCTURAL, first differing row @0: ROM `(none)` vs `lui v1,0x0  [HI16 D_138180]`. */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
extern u8 g_planetWarpEnabled;   /* 0x1ABA70 - per-item enabled flags (indexed) */
extern s32 g_playerProgress;     /* 0x1A79F8 - current save progress slot */
s32 UpdatePlanetWarpMenuInput(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 cursor0 = g_planetWarpCursor;   /* cursor at entry, for the fence check */
    s32 result = 0;
    s32 cursor;

    if (flags & 0x10) {                 /* back */
        s32 e0 = *(s32 *)(*(u8 **)(g_menuScreenBlock + 0x14) + 0xE0);
        g_planetWarpCursor = 0;
        if (e0 != 0) {
            *(s32 *)(g_menuScreenBlock + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(g_menuScreenBlock + 0x134) == 0) ? -1 : 0;
    }

    if (flags & 0x900) {                /* cancel */
        g_planetWarpCursor = 0;
        return 1;
    } else if (flags & 0x1000) {        /* up */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = g_planetWarpCursor - 1;
        g_planetWarpCursor = (v >= 0) ? v : 7;
    } else if (flags & 0x4000) {        /* down */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = g_planetWarpCursor + 1;
        g_planetWarpCursor = (v < 8) ? v : 0;
    }

    flags = *(s32 *)(D_138180 + 0x1C4);
    if (flags & 0x40) {                 /* confirm */
        cursor = g_planetWarpCursor;
        if ((&g_planetWarpEnabled)[cursor] == 0) {
            PlayGlobalSound(5, 0, 0);   /* disabled item */
        } else {
            PlayGlobalSound(4, 0, 0);
            cursor = g_planetWarpCursor;
            if ((u32)cursor < 8) {
                switch (cursor) {
                case 0: D_1A7904 = 2; RequestLevelExit(0x2, 1); break;
                case 1: D_1A7904 = 3; RequestLevelExit(0x4, 1); break;
                case 2: D_1A7904 = 3; RequestLevelExit(0xB, 1); break;
                case 3: D_1A7904 = 2; RequestLevelExit(0xB, 1); break;
                case 4: D_1A7904 = 1; RequestLevelExit(0x1A, 1); break;
                case 5: D_1A7904 = 1; RequestLevelExit(0x16, 1); break;
                case 6: D_1A7904 = 1; RequestLevelExit(0x17, 1); break;
                case 7:
                    D_1A7904 = 1;
                    if (g_playerProgress != 0x15) {
                        D_1A7908 = g_playerProgress;
                    }
                    RequestLevelExit(0x15, 1);
                    break;
                }
            }
        }
    }

    /* present-record redraw fence */
    cursor = g_planetWarpCursor;
    *(s32 *)(D_0025C560 + 0x58) = cursor;
    if ((&g_planetWarpEnabled)[cursor] == 0) {
        *(s32 *)(D_25C520 + 0x10) |= 0x4;
    } else if (cursor0 == cursor && g_fileLoadState == 0) {
        s32 mode = *(s32 *)(D_0025C560 + 0x44);
        if ((*(s32 *)(D_0025C560 + 0x50) == cursor0 && mode == 2) ||
            (*(s32 *)(D_0025C560 + 0x54) == cursor0 && mode == 4)) {
            *(s32 *)(D_25C520 + 0x10) &= ~0x4;
        }
    }
    return result;
}
#endif

/* DrawPlanetWarpMenu: renders the planet-warp menu screen. Opens a 2D batch, blits
 * four title glyphs (atlas g_guiInstance+0x8710, codepoints 0xD7/0xD8/0xD9/0xDD,
 * colors 0x60442D00/0x60241700/0x55F0C070/0x55F0C070, centred at row D_1ABA78, scale
 * 1.0, y-fudge, v38 0.0); draws three header strings (0x3098 @0xB3,0x1B; 0x2BE5
 * @0x161,0x177; 0x2C0B @0xB5,0x177; color 0x80F0F0F0); draws the L1/R1 paging chrome;
 * then draws a five-slot cursor-relative carousel. Each visible slot i shows the
 * planet at wrapped index (i + g_planetWarpCursor - 2) mod 8, labelled with the
 * localized D_1ABD98[idx] when enabled (g_planetWarpEnabled[idx] != 0) else 0x2C56;
 * slot color fades by distance from centre (edges 0x10F0F0F0, mid 0x50F0F0F0, centre
 * 0x70F0F0F0), and the centre slot also gets a text-width-sized selection box.
 * (func_003017F8 ignores its scale/vec38 params — see its body — so glyphs pass NULL.)
 * Wall: 8-byte-packed-save (6 GPRs) + FP-arg scheduling — later cc1 save-slot packing
 * not reproduced. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawPlanetWarpMenu);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 33.46% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-64` vs `lui v0,0x0  [HI16 0x001A7340]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 38.55% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-64` vs `addiu sp,sp,-208`. */
extern void func_003017F8(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                          f32 px, f32 py, f32 sx, f32 syg, f32 v38);
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void DrawMenuPagingChrome(void);
extern s32 func_001157AC(const char *s);          /* SDK strlen */
extern s32 func_0027F818(const char *str, s32 len); /* menu text pixel width */
extern void func_00280250(s32 x, s32 y, u32 color, const char *str, s32 flag);
extern s16 D_1ABD98[];           /* per-planet label string ids */
extern s32 D_1ABA78;             /* title glyph row (int, converted to float) */
extern s32 g_swapGadgetItemIndex; /* +0x8E holds the global sprite y-fudge (f32) */
s32 DrawPlanetWarpMenu(void) {
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(g_screenWidth / 2);
    f32 row = (f32)D_1ABA78;
    f32 yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    s32 cursor;
    s32 i;
    s32 y;

    Begin2dDrawBatch(0);
    /* four title glyphs (v38 = 0.0) */
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD7), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD8), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xD9), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xDD), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    /* three header strings */
    func_002801B8(0xB3, 0x1B, 0x80F0F0F0, GetLocalizedString(0x3098), -1);
    func_002801B8(0x161, 0x177, 0x80F0F0F0, GetLocalizedString(0x2BE5), -1);
    func_002801B8(0xB5, 0x177, 0x80F0F0F0, GetLocalizedString(0x2C0B), -1);
    DrawMenuPagingChrome();
    /* five cursor-relative carousel slots */
    cursor = g_planetWarpCursor;
    y = 0x112;
    for (i = 0; i < 5; i++) {
        s32 idx = i + cursor - 2;
        s32 strId;
        char *str;
        u32 color;
        if (idx < 0) idx += 8;
        if (idx >= 8) idx -= 8;
        strId = ((&g_planetWarpEnabled)[idx] != 0) ? D_1ABD98[idx] : 0x2C56;
        str = GetLocalizedString(strId);
        if (i == 0 || i == 4) {
            color = 0x10F0F0F0;
        } else if (i == 1 || i == 3) {
            color = 0x50F0F0F0;
        } else {  /* i == 2: the focused slot gets a text-width selection box */
            color = 0x70F0F0F0;
            DrawMenuItemSelectionBox(func_0027F818(str, func_001157AC(str)), 0x70F0F0F0);
        }
        func_00280250(g_screenWidth / 2, y, color, str, -1);
        y += 0x14;
    }
    End2dDrawBatch();
    return 0;
}
#endif

/* When the GUI is up and extras unlocked, latch the per-extra-feature
 * availability flags (g_planetWarpEnabled @0x1ABA70 + D_1ABA71..D_1ABA77) from a
 * pair of inputs: a per-feature "menu enabled" toggle byte (D_1A7BF2/F4/FB/C06/
 * C07/C0A) gating each block, and the relevant save-data completion bytes in the
 * progress block D_1395B8. Each available block stores a 1; only the planet-warp
 * block clears (the rest leave the flag untouched when their guard fails).
 * EU twin func_002D24C0 (byte-identical; toggles D_1A7C72.., block D_139638,
 * targets D_1ABAD8.., all region-shifted).
 * Wall: `bnel`/`beql` branch-likely (the value-1 move sits in the nullified
 * delay slot of the OR short-circuit tests) — the later cc1's branch-likely
 * emission isn't reproduced from clean C. Preserved as portable C. */
extern u8 g_planetWarpEnabled;   /* 0x1ABA70 - planet-warp menu enabled flag */
extern u8 D_1ABA71, D_1ABA72, D_1ABA73, D_1ABA74, D_1ABA75, D_1ABA76, D_1ABA77;
extern u8 D_1A7BF2, D_1A7BF4, D_1A7BFB, D_1A7C06, D_1A7C07, D_1A7C0A;
extern u8 D_1395B8[];            /* save-data per-feature completion block */
extern u8 D_1395E9;              /* planet-warp prerequisite completion byte */
extern s32 g_playerProgress;     /* 0x1A79F8 - current save progress slot */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D2538);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 90.44% -> STRUCTURAL,
 * first differing row @2: ROM `beq v0,zero,L` vs `(none)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 90.95% -> STRUCTURAL, first differing row @2: ROM `beq v0,zero,L` vs `(none)`. */
/* TODO(match): functional equivalent - not byte-exact; `bnel`/`beql` branch-
 * likely delay-slot value moves not reproduced by cc1. */
void func_002D2538(void) {
    if (g_guiInstance == NULL) {
        return;
    }
    if (g_miscExtras == 0) {
        return;
    }
    if (D_1A7BF2 != 0) {
        if (D_1395E9 != 0) {
            g_planetWarpEnabled = 1;
        }
    }
    if (D_1A7BF4 != 0) {
        if (D_1395B8[0x4D] != 0 || D_1395B8[0x52] != 0) {
            D_1ABA71 = 1;
        }
    }
    if (D_1A7BFB != 0) {
        if (D_1395B8[0x57] != 0 || D_1395B8[0x5C] != 0) {
            D_1ABA72 = 1;
        }
    }
    if (D_1A7BFB != 0) {
        if (D_1395B8[0x3D] != 0) {
            D_1ABA73 = 1;
        }
    }
    if (D_1A7C0A != 0) {
        D_1ABA74 = 1;
    }
    if (D_1A7C06 != 0) {
        D_1ABA75 = 1;
    }
    if (D_1A7C07 != 0) {
        D_1ABA76 = 1;
    }
    if (D_1AA458 != 0 && g_playerProgress > 0) {
        D_1ABA77 = 1;
    }
}
#endif

/* UpdateInsomniacMuseumInput: per-frame input for the Insomniac Museum menu, a
 * 5-item carousel (g_museumMenuCursor in [0,4]). Back (0x10) resets the cursor and
 * latches the active screen's pending sub-result (block[0x14]->0xE0 into
 * block[0x18], else -1/0). Cancel (0x900) resets the cursor and returns 1. Up
 * (0x1000)/down (0x4000) play UI sound 3 and move the cursor with wrap in [0,4].
 * Confirm (0x40) on an enabled item (D_1ABA80[cursor] != 0) plays sound 4 and, via
 * jtbl_0026CD20_text, either requests a sub-screen (g_pNextMenuScreen = D_25C760 /
 * D_25C950 / D_25CB40 for cursors 0/3/4) or, for cursors 1/2, spawns the exhibit
 * moby into the HUD shadow table (SwapMobyTableContext(0)/SpawnMoby(id)/
 * SwapMobyTableContext(1); id = (cursor^2)!=0 ? 0x1335 : 0x88D), flags it 0xFF at
 * +0x30 and copies the hero position into +0x10; a disabled item plays sound 5.
 * Every non-back path then runs the present-record redraw fence (D_0025C6F8 record,
 * live object *D_25C6B8): SET redraw bit 0x4 when disabled, else CLEAR it only when
 * the cursor is unchanged, the file-load is idle, and the present record shows this
 * cursor already presented (offsets 0x50/0x54 == cursor and state 0x44 in {2,4}).
 * Wall: switch/jump-table dispatch (splat jtbl reloc gap) + lq/sq hero-pos copy +
 * branch-likely fence — cc1 jtbl layout not reproduced. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateInsomniacMuseumInput);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 28.88% -> STRUCTURAL,
 * first differing row @1: ROM `lui v0,0x0  [HI16 D_138180]` vs `(none)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 48.28% -> STRUCTURAL, first differing row @0: ROM `addiu sp,sp,-48` vs `(none)`. */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
extern void SwapMobyTableContext(s32 tableId);
extern void *SpawnMoby(s32 classId);
extern s32 g_museumMenuCursor;   /* museum carousel cursor 0..4 */
extern u8 *g_pNextMenuScreen;    /* 0x1F27D8 - requested next screen */
extern u8 g_heroPos[];           /* 0x189EA0 - hero world position vec4 */
extern u8 D_25C760[], D_25C950[], D_25CB40[]; /* museum sub-screen records */
extern u8 D_0025C6F8[];          /* per-screen present record */
extern u8 *D_25C6B8;             /* pointer to the live menu object */
s32 UpdateInsomniacMuseumInput(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 cursor0 = g_museumMenuCursor;   /* cursor at entry, for the fence check */
    s32 result = 0;
    s32 cursor;

    if (flags & 0x10) {                 /* back */
        s32 e0 = *(s32 *)(*(u8 **)(g_menuScreenBlock + 0x14) + 0xE0);
        g_museumMenuCursor = 0;
        if (e0 != 0) {
            *(s32 *)(g_menuScreenBlock + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(g_menuScreenBlock + 0x134) == 0) ? -1 : 0;
    }

    if (flags & 0x900) {                /* cancel */
        g_museumMenuCursor = 0;
        return 1;
    } else if (flags & 0x1000) {        /* up */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = g_museumMenuCursor - 1;
        g_museumMenuCursor = (v >= 0) ? v : 4;
    } else if (flags & 0x4000) {        /* down */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = g_museumMenuCursor + 1;
        g_museumMenuCursor = (v < 5) ? v : 0;
    }

    flags = *(s32 *)(D_138180 + 0x1C4);
    if (flags & 0x40) {                 /* confirm */
        cursor = g_museumMenuCursor;
        if ((&D_1ABA80)[cursor] == 0) {
            PlayGlobalSound(5, 0, 0);   /* disabled item */
        } else {
            PlayGlobalSound(4, 0, 0);
            cursor = g_museumMenuCursor;
            if ((u32)cursor < 5) {
                switch (cursor) {
                case 0:
                    g_pNextMenuScreen = D_25C760;
                    break;
                case 1:
                case 2: {
                    /* spawn the exhibit moby into the HUD shadow table */
                    void *moby;
                    s32 id = ((cursor ^ 2) != 0) ? 0x1335 : 0x88D;
                    SwapMobyTableContext(0);
                    moby = SpawnMoby(id);
                    SwapMobyTableContext(1);
                    if (moby != 0) {
                        u8 *m = (u8 *)moby;
                        m[0x30] = 0xFF;
                        /* 128-bit (lq/sq) copy of the hero position into +0x10 */
                        *(u32 *)(m + 0x10) = *(u32 *)(g_heroPos + 0);
                        *(u32 *)(m + 0x14) = *(u32 *)(g_heroPos + 4);
                        *(u32 *)(m + 0x18) = *(u32 *)(g_heroPos + 8);
                        *(u32 *)(m + 0x1C) = *(u32 *)(g_heroPos + 12);
                        result = 1;
                    }
                    break;
                }
                case 3:
                    g_pNextMenuScreen = D_25C950;
                    break;
                case 4:
                    g_pNextMenuScreen = D_25CB40;
                    break;
                }
            }
        }
    }

    /* present-record redraw fence */
    cursor = g_museumMenuCursor;
    *(s32 *)(D_0025C6F8 + 0x58) = cursor;
    if ((&D_1ABA80)[cursor] == 0) {
        *(s32 *)(D_25C6B8 + 0x10) |= 0x4;
    } else if (cursor0 == cursor && g_fileLoadState == 0) {
        s32 mode = *(s32 *)(D_0025C6F8 + 0x44);
        if ((*(s32 *)(D_0025C6F8 + 0x50) == cursor0 && mode == 2) ||
            (*(s32 *)(D_0025C6F8 + 0x54) == cursor0 && mode == 4)) {
            *(s32 *)(D_25C6B8 + 0x10) &= ~0x4;
        }
    }
    return result;
}
#endif

/* DrawInsomniacMuseumMenu: renders the Insomniac Museum menu screen. Opens a 2D
 * batch, blits three title glyphs (atlas g_guiInstance+0x8710, codepoints
 * 0x8B/0x8C/0x8D, colors 0x60442D00/0x55F0C070/0x55F0C070, centred at row D_1ABA94,
 * scale 1.0, y-fudge, v38 0.775); draws three centred header strings (0x30A2 @row
 * 0x41, 0x2C0B @0x141, 0x2BE5 @0x15A, color 0x80F0F0F0); draws four framing lines
 * (func_002904B0) in color 0x55F0C070; then draws the five menu items in a column
 * (x 0xA5, y 0x7D stepping 0x1F): each item's color is 0x7000FFFF when it is the
 * selected row (g_museumMenuCursor) else 0x80F0F0F0, and its label is the localized
 * D_1ABDA8[i] when the item is enabled (D_1ABA80[i] != 0) else the fallback 0x2C56.
 * (func_003017F8 ignores its scale/vec38 params — see its body — so glyphs pass NULL.)
 * Wall: 8-byte-packed-save (7 GPRs) + FP-arg scheduling — later cc1 save-slot packing
 * not reproduced. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawInsomniacMuseumMenu);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 23.83% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-80` vs `lui v0,0x0  [HI16 0x001A7340]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 47.77% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-80` vs `addiu sp,sp,-176`. */
extern void func_003017F8(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                          f32 px, f32 py, f32 sx, f32 syg, f32 v38);
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void func_002904B0(s32 x0, s32 y0, s32 x1, s32 y1, s32 color, s32 flag);
extern s32 g_museumMenuCursor;   /* museum carousel cursor 0..4 */
extern s32 D_1ABA94;             /* title glyph row (int, converted to float) */
extern s32 D_1ABDA8[];           /* per-item label string ids */
extern s32 g_swapGadgetItemIndex; /* +0x8E holds the global sprite y-fudge (f32) */
s32 DrawInsomniacMuseumMenu(void) {
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(g_screenWidth / 2);
    f32 row = (f32)D_1ABA94;
    f32 yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    s32 cursor;
    s32 i;
    s32 y;

    Begin2dDrawBatch(0);
    /* three title glyphs (v38 = 0.775f = 0x3F466666) */
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0x8B), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.775f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0x8C), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.775f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0x8D), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.775f);
    /* three centred header strings */
    func_002801B8(g_screenWidth / 2, 0x41, 0x80F0F0F0, GetLocalizedString(0x30A2), -1);
    func_002801B8(g_screenWidth / 2, 0x141, 0x80F0F0F0, GetLocalizedString(0x2C0B), -1);
    func_002801B8(g_screenWidth / 2, 0x15A, 0x80F0F0F0, GetLocalizedString(0x2BE5), -1);
    /* framing lines */
    func_002904B0(0x138, 0x80, 0x1C1, 0x82, 0x55F0C070, 0);
    func_002904B0(0x138, 0x109, 0x1C3, 0x10B, 0x55F0C070, 0);
    func_002904B0(0x138, 0x80, 0x13A, 0x109, 0x55F0C070, 0);
    func_002904B0(0x1C1, 0x80, 0x1C3, 0x109, 0x55F0C070, 0);
    /* five menu items */
    cursor = g_museumMenuCursor;
    y = 0x7D;
    for (i = 0; i < 5; i++) {
        u32 color = (i == cursor) ? 0x7000FFFF : 0x80F0F0F0;
        s32 strId = ((&D_1ABA80)[i] != 0) ? D_1ABDA8[i] : 0x2C56;
        func_002801B8(0xA5, y, color, GetLocalizedString(strId), -1);
        y += 0x1F;
    }
    End2dDrawBatch();
    return 0;
}
#endif

/* When the GUI is up, latch the "new content" flags for each extras-menu entry.
 * Near-miss: cc1 hoists the g_miscExtras load above (and CSEs it into) the
 * g_guiInstance short-circuit test, reordering the two guard loads vs the
 * original's separate guiInstance check. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D2C60);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 96.43% -> DSLOT-FILL,
 * first differing row @24: ROM `(none)` vs `sll zero,zero,0x0`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 85.71% -> ORDER-ONLY, first differing row @2: ROM `beq v0,zero,L` vs `(none)`. */
s32 func_002D2C60(void) {
    if (g_guiInstance) {
        if (g_miscExtras) {
            D_1ABA8C = 1;
            D_1ABA88 = 1;
        }
        if (D_1AA450) {
            D_1ABA84 = 1;
        }
        if (D_1AA454) {
            D_1ABA90 = 1;
        }
        if (D_1AA458) {
            D_1ABA80 = 1;
        }
    }
    return 0;
}
#endif

/* UpdateHelpTopicMenuInput: per-frame input for the help-topics screen, an
 * 18-entry page list (g_helpTopicCursor in [0,0x11]). Back (0x10) resets the
 * cursor and latches the active screen's pending sub-result (block[0x14]->0xE0
 * into block[0x18], else -1/0). Cancel (0x900) resets the cursor and returns 1.
 * 0x8000 steps to the previous topic (cursor-1, floored at 0); 0x2000 steps to the
 * next (cursor+1, capped at 0x11) — each plays UI sound 3 when the move stays in
 * range, sound 5 at the edge. Every non-back path then stamps the active topic:
 * D_25C940 = cursor and D_25C8C4 = the topic's string id D_1ABDC0[cursor].
 * Wall: 8-byte-packed-save (2 GPRs) — later cc1 save-slot packing not reproduced.
 * Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateHelpTopicMenuInput);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 62.81% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 73.12% -> PACKED-SAVE, first differing row @1: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
extern s32 g_helpTopicCursor;    /* 0x1ABA98 - help topic page index 0..0x11 */
extern s32 D_25C940;             /* active topic index mirror */
extern s16 D_1ABDC0[];           /* per-topic string-id table (halfwords) */
extern s32 D_25C8C4;             /* active topic's string id */
s32 UpdateHelpTopicMenuInput(void) {
    s32 buttons = g_padButtonsPressed;
    s32 result = 0;
    s32 cursor;

    if (buttons & 0x10) {               /* back */
        s32 e0 = *(s32 *)(*(u8 **)(g_menuScreenBlock + 0x14) + 0xE0);
        g_helpTopicCursor = 0;
        if (e0 != 0) {
            *(s32 *)(g_menuScreenBlock + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(g_menuScreenBlock + 0x134) == 0) ? -1 : 0;
    }

    if (buttons & 0x900) {              /* cancel */
        g_helpTopicCursor = 0;
        result = 1;
    } else if (buttons & 0x8000) {      /* previous topic */
        s32 v;
        if (g_helpTopicCursor > 0) {
            PlayGlobalSound(3, 0, 0);
        } else {
            PlayGlobalSound(5, 0, 0);
        }
        v = g_helpTopicCursor - 1;
        g_helpTopicCursor = (v >= 0) ? v : 0;
    } else if (buttons & 0x2000) {      /* next topic */
        s32 v;
        if (g_helpTopicCursor < 0x11) {
            PlayGlobalSound(3, 0, 0);
        } else {
            PlayGlobalSound(5, 0, 0);
        }
        v = g_helpTopicCursor + 1;
        g_helpTopicCursor = (v < 0x12) ? v : 0x11;
    }

    /* stamp the active topic + its string id */
    cursor = g_helpTopicCursor;
    D_25C940 = cursor;
    D_25C8C4 = D_1ABDC0[cursor];
    return result;
}
#endif

/* DrawHelpTopicMenu: renders the help-topics screen chrome. Opens a 2D draw batch,
 * blits three fixed HUD glyphs (font atlas at g_guiInstance+0x8710, codepoints
 * 0xDE/0xDF/0xE0 with colors 0x60442D00/0x60241700/0x55F0C070) horizontally centred
 * (screen width / 2) at row D_1ABA9C, scale 1.0 with the global sprite y-fudge; draws
 * the left/right paging arrows (left shown when the cursor isn't at the first page,
 * right when it isn't at the last, index 0x11); then draws the localized footer
 * string 0x2BE5 at (0x1B0,0x177) in color 0x80F0F0F0, and closes the batch.
 * (func_003017F8's scale/vec38 pointer params are unused by the callee — verified in
 * its body @0x3017F8 — so the glyph draws pass NULL.)
 * Wall: 8-byte-packed-save (2 GPRs) + FP-arg scheduling — later cc1 save-slot
 * packing not reproduced. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawHelpTopicMenu);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 20.82% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-32` vs `lui v0,0x0  [HI16 0x001A7340]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 12.59% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-80`. */
extern void func_003017F8(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                          f32 px, f32 py, f32 sx, f32 syg, f32 v38);
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void DrawBestiaryPagingArrows(s32 leftEnabled, s32 rightEnabled);
extern s32 g_helpTopicCursor;    /* 0x1ABA98 - help topic page index 0..0x11 */
extern s32 g_swapGadgetItemIndex; /* +0x8E holds the global sprite y-fudge (f32) */
extern s32 D_1ABA9C;             /* glyph row (int, converted to float) */
s32 DrawHelpTopicMenu(void) {
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(g_screenWidth / 2);
    f32 row = (f32)D_1ABA9C;
    f32 yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    s32 cursor;
    char *text;

    Begin2dDrawBatch(0);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xDE), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xDF), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xE0), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    cursor = g_helpTopicCursor;
    DrawBestiaryPagingArrows(cursor != 0, cursor != 0x11);
    text = GetLocalizedString(0x2BE5);
    func_002801B8(0x1B0, 0x177, 0x80F0F0F0, text, -1);
    End2dDrawBatch();
    return 0;
}
#endif

/* return 0 stub. */
s32 func_002D2FC0(void) {
    return 0;
}

/* Options sub-screen input handler. Confirm (0x10) latches the screen result
 * (resets g_optionsSubCursor) like the other polls. Cancel (0x900) resets the
 * cursor and returns 1. Up (0x8000) decrements the 6-entry cursor (clamp at 0);
 * Down (0x2000) increments (clamp at 5). Each move plays sound 3 (moved) or 5
 * (blocked at an edge). After a move, if the cursor changed it latches an error
 * code (-0x12C) into D_25CA80[0x3C]; then mirrors the cursor into D_25CB30 and
 * stores the s16 entry from the D_1ABDEA table into D_25CA80[0x34]. Returns the
 * confirm/cancel tri-state.
 * Wall: 8-byte-packed-save (saves $16 + $17 + $31). Preserved as portable C. */
extern s32 g_optionsSubCursor;
extern u8 D_25CA80[];
extern s32 D_25CB30;
extern u8 D_1ABDEA[]; /* s16 entries on a 4-byte stride */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D2FC8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 51.74% -> GPREL-vs-ABS,
 * first differing row @1: ROM `(none)` vs `lw a1,0(gp)  [GPREL16 0x001ABAA0]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 61.23% -> PACKED-SAVE, first differing row @1: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-48`. */
/* TODO(match): functional equivalent - not byte-exact; 3-GPR packed-save frame +
 * branch-likely / reload scheduling not reproduced by cc1. */
s32 func_002D2FC8(void) {
    s32 flags = g_padButtonsPressed;
    s32 old = g_optionsSubCursor;
    s32 result = 0;
    s32 cur;

    if (flags & 0x10) {
        s32 *block;
        s32 v;
        g_optionsSubCursor = 0;
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }

    if (flags & 0x900) {
        g_optionsSubCursor = 0;
        result = 1;
    } else if (flags & 0x8000) {
        PlayGlobalSound(old <= 0 ? 5 : 3, 0, 0);
        cur = g_optionsSubCursor - 1;
        g_optionsSubCursor = cur;
        if (cur < 0) {
            g_optionsSubCursor = 0;
        }
    } else if (flags & 0x2000) {
        PlayGlobalSound(old < 5 ? 3 : 5, 0, 0);
        cur = g_optionsSubCursor + 1;
        g_optionsSubCursor = cur;
        if (cur >= 6) {
            g_optionsSubCursor = 5;
        }
    }

    if (old != g_optionsSubCursor) {
        *(s32 *)(D_25CA80 + 0x3C) = -0x12C;
    }
    cur = g_optionsSubCursor;
    D_25CB30 = cur;
    *(s32 *)(D_25CA80 + 0x34) = *(s16 *)(D_1ABDEA + cur * 4);
    return result;
}
#endif

/* Options-menu screen draw: inside a 2D batch, draws the three header glyphs
 * (0xDE/0xDF/0xE0 in the standard 0x60442D00 / 0x60241700 / 0x55F0C070 colours,
 * centred at row D_1ABAA4), the left/right paging arrows keyed on the option
 * cursor (g_optionsSubCursor, 0..5), a slider fill (func_002904B0) whose vertical
 * endpoints come from the slider fraction *(g_swapGadgetItemIndex+0x8A) scaled by
 * 330/332, then the localized label for the current option (D_1ABDE8[cursor],
 * screen-centred) and the footer prompt (string 0x2BE5). Returns 0.
 * (func_003017F8 ignores its scale/vec38 pointer args — glyph draws pass NULL.)
 * Wall: 8-byte-packed-save + FP-arg scheduling — later cc1 save-slot packing not
 * reproduced. Preserved as portable C. */
extern void func_002904B0(s32 x0, s32 y0, s32 x1, s32 y1, s32 color, s32 flag);
extern s32 D_1ABAA4;      /* glyph row (int, converted to float) */
extern u8 D_1ABDE8[];     /* per-option label string-id table, s16 on 4-byte stride */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3138);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 25.62% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-48` vs `lui v0,0x0  [HI16 0x001A7340]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 18.61% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-80`. */
/* TODO(match): functional equivalent - not byte-exact; the slider endpoint ints
 * model the original's `mul; add 0.5; cvt.w.s` as round-half-up. */
s32 func_002D3138(void) {
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(g_screenWidth / 2);
    f32 row = (f32)D_1ABAA4;
    f32 yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    f32 frac = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8A);
    s32 cursor;
    char *text;

    Begin2dDrawBatch(0);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xDE), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xDF), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xE0), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    cursor = g_optionsSubCursor;
    DrawBestiaryPagingArrows(cursor != 0, cursor != 5);
    func_002904B0(0x42, (s32)(frac * 330.0f + 0.5f), 0x1CF,
                  (s32)(frac * 332.0f + 0.5f), 0x55F0C070, 0);
    text = GetLocalizedString(*(s16 *)(D_1ABDE8 + g_optionsSubCursor * 4));
    func_002801B8(g_screenWidth / 2, 0x137, 0x80F0F0F0, text, -1);
    text = GetLocalizedString(0x2BE5);
    func_002801B8(0x1B0, 0x177, 0x80F0F0F0, text, -1);
    End2dDrawBatch();
    return 0;
}
#endif

/* GUI null-check gate: if the GUI is up, latch error code -0x12C; return 0. */
s32 func_002D3388(void) {
    if (g_guiInstance) {
        D_25CABC = -0x12C;
    }
    return 0;
}

/* Help-topic sub-browser input handler. Confirm (0x10) latches the screen's
 * pending result like the other confirm polls (resets g_helpPageCursor to 0).
 * Cancel (0x900) returns 1. Up (0x8000) decrements the 7-page cursor clamped at
 * 0; Down (0x2000) increments clamped at 6. Each move plays sound 3 (moved) or
 * sound 5 (blocked at an edge). The landed page is mirrored into D_25CCC8.
 * Returns the confirm/cancel tri-state (1 / -1 / 0).
 * Wall: 8-byte-packed-save (saves $16 + $31) + branch-likely shape. Portable C. */
extern s32 g_helpPageCursor;
extern s32 D_25CCC8;
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D33A8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 55.13% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 51.35% -> SPLIT-HIREG, first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely / reload scheduling not reproduced by cc1. */
s32 func_002D33A8(void) {
    s32 flags = g_padButtonsPressed;
    s32 result = 0;
    s32 page;

    if (flags & 0x10) {
        s32 *block = (s32 *)g_menuScreenBlock;
        s32 v;
        g_helpPageCursor = 0;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }

    if (flags & 0x900) {
        g_helpPageCursor = 0;
        result = 1;
        page = g_helpPageCursor;
        D_25CCC8 = page;
        return result;
    }

    if (flags & 0x8000) {
        s32 cur = g_helpPageCursor - 1;
        g_helpPageCursor = cur;
        PlayGlobalSound(cur < 0 ? 5 : 3, 0, 0);
        cur = g_helpPageCursor;
        if (cur < 0) {
            g_helpPageCursor = 0;
        }
    } else if (flags & 0x2000) {
        s32 cur = g_helpPageCursor + 1;
        g_helpPageCursor = cur;
        PlayGlobalSound(cur < 7 ? 3 : 5, 0, 0);
        cur = g_helpPageCursor;
        if (cur >= 7) {
            g_helpPageCursor = 6;
        }
    }

    page = g_helpPageCursor;
    D_25CCC8 = page;
    return result;
}
#endif

/* Help/hint-page screen draw: inside a 2D batch, draws the two header glyphs
 * (0xE5/0xE4, centred at row D_1ABAAC), then the left/right page arrows when the
 * page cursor (g_helpPageCursor, 0..6) allows: a "prev" arrow (glyph 0x4B at
 * 216,373) when cursor>0 and a "next" arrow (glyph 0x4A at 295,373) when cursor<6,
 * each drawn white while its pad direction is held (L1 0x8000 / R1 0x2000) else in
 * the pulsing inactive colour from func_002AA3F0. The arrows carry their prompt
 * labels (0x2DD6 / 0x2DD7), and the screen title (0x312A) + footer (0x2BE5) are
 * centred. Returns 0. (func_003017F8 ignores scale/vec38 — glyph draws pass NULL.)
 * Wall: 8-byte-packed-save + FP-arg scheduling — later cc1 save-slot packing not
 * reproduced. Preserved as portable C. */
extern u32 func_002AA3F0(u32 color1, u32 color2, s32 period, s32 counterSel, s32 reset);
extern void func_0027FBA8(s32 x, s32 y, u64 color, char *str, s64 wrap);
extern s32 g_padButtonsHeld;
extern s32 D_1ABAAC;      /* glyph row (int, converted to float) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D34E8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 41.88% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-48` vs `lui v1,0x0  [HI16 0x001A8D04]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 43.96% -> STRUCTURAL, first differing row @0: ROM `addiu sp,sp,-48` vs `lui v0,0x0  [HI16 0x001A7340]`. */
s32 func_002D34E8(void) {
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(g_screenWidth / 2);
    f32 row = (f32)D_1ABAAC;
    f32 yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    u32 inactiveArrow;
    s32 cursor;
    char *text;

    Begin2dDrawBatch(0);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xE5), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0xE4), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    inactiveArrow = func_002AA3F0(0x60241700, 0x55F0C070, 0x19, 0, 0);

    cursor = g_helpPageCursor;
    if (cursor > 0) {
        u32 color = (g_padButtonsHeld & 0x8000) ? 0x80F0F0F0 : inactiveArrow;
        func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0x4B), color, (f32 *)0, (f32 *)0,
                      216.0f, 373.0f, 1.0f, yfudge * 0.8f, 0.0f);
        text = GetLocalizedString(0x2DD6);
        func_0027FBA8(0x40, 0x171, 0x80F0F0F0, text, -1);
        cursor = g_helpPageCursor;
    }
    if (cursor < 6) {
        u32 color = (g_padButtonsHeld & 0x2000) ? 0x80F0F0F0 : inactiveArrow;
        func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0x4A), color, (f32 *)0, (f32 *)0,
                      295.0f, 373.0f, 1.0f, yfudge * 0.8f, 0.0f);
        text = GetLocalizedString(0x2DD7);
        func_00280090(0x1C1, 0x171, 0x80F0F0F0, text, -1);
    }

    text = GetLocalizedString(0x312A);
    func_002801B8(g_screenWidth / 2, 0x1D, 0x80F0F0F0, text, -1);
    text = GetLocalizedString(0x2BE5);
    func_002801B8(g_screenWidth / 2, 0x173, 0x80F0F0F0, text, -1);
    End2dDrawBatch();
    return 0;
}
#endif

/* return 0 stub. */
s32 func_002D37E0(void) {
    return 0;
}

/* Planet-warp / cinematic-camera confirm input handler. Reads g_padButtonsPressed:
 *   - Triangle (0x10): the same menu-screen "back/confirm" latch as the other
 *     menu input handlers — if the active screen instance (block[0x14]) has a
 *     pending result at +0xE0, store it into block[0x18] and return 0; else
 *     return -1 when block[0x134] is clear, 0 otherwise;
 *   - {L1|R1}=0x900: returns 1 (consume the page nav);
 *   - X (0x40): plays the confirm SFX (id 4); then if a cinematic camera is
 *     already active (D_1A790C) tears it down (clears D_1A790C + the two sound-
 *     bank cinematic-channel words at g_soundBankHandlesBlk+0x14F8/+0x1500),
 *     otherwise arms it (D_1A790C=1) and seeds the cinematic camera vector via
 *     Vec3RescaleToLenVu0(camBlock+0x10, soundBlkBase, 1.0f);
 *   - otherwise returns 0.
 * EU twin func_002D3770 (byte-identical; region-shifted symbols).
 * Routes to tester-EE: drives PlayGlobalSound + the VU0 Vec3RescaleToLenVu0
 * micro-op + live cinematic-camera state; not standalone cmp-oracle'able.
 * Wall: `beql` branch-likely on the +0xE0 latch + VU0/float-arg scheduling — not
 * reproduced from clean C. Preserved as portable C. */
extern s32 D_1A790C;             /* cinematic-camera-active latch */
extern u8 g_soundBankHandlesBlk[];   /* 0x189E20 - sound-bank handle block */
extern u8 g_cinematicCameraBlock[];  /* 0x18B2F0 - cinematic camera override block */
extern void Vec3RescaleToLenVu0(void *dst, void *src, f32 len);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D37E8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 70.29% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 80.33% -> SPLIT-HIREG, first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. */
/* TODO(match): functional equivalent - not byte-exact; `beql` branch-likely latch
 * + VU0 float-arg scheduling not reproduced by cc1. */
s32 func_002D37E8(void) {
    s32 flags = g_padButtonsPressed;
    s32 result = 0;

    if (flags & 0x10) {
        s32 *block = (s32 *)g_menuScreenBlock;
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }

    if (flags & 0x900) {
        return 1;
    }

    if (flags & 0x40) {
        PlayGlobalSound(4, 0, 0);
        if (D_1A790C != 0) {
            D_1A790C = 0;
            *(s32 *)(g_soundBankHandlesBlk + 0x14F8) = 0;
            *(s32 *)(g_soundBankHandlesBlk + 0x1500) = 0;
        } else {
            D_1A790C = 1;
            Vec3RescaleToLenVu0(g_cinematicCameraBlock + 0x10,
                                g_cinematicCameraBlock + 0x10 - 0x14E0, 1.0f);
        }
    }

    return result;
}
#endif

/* Summary/confirmation screen draw: inside a 2D batch, draws three header glyphs
 * (0x8B/0x8C/0x8D, centred at row D_1ABAB0), three centred lines (localized 0x30D5,
 * 0x2BE4, 0x2BE5), then two composed lines built with the SDK sprintf
 * (func_00115DA8): the left line formats D_1AB9F8 with the localized 0x30D5 string
 * (drawn at 0x86,0xBA via func_0027FBA8) and the right line formats D_1ABA38 with a
 * localized string chosen by the cinematic-active latch D_1A790C (0x2C5C when set,
 * else 0x2C5D; drawn at 0x15D,0xBA via func_002801B8). Both composed draws pass the
 * string length (func_001157AC = strlen) as the clip arg. Returns 0.
 * (func_003017F8 ignores scale/vec38 — glyph draws pass NULL.)
 * Wall: 8-byte-packed-save + FP-arg scheduling — later cc1 save-slot packing not
 * reproduced. Preserved as portable C. */
extern s32 D_1ABAB0;     /* glyph row (int, converted to float) */
extern char D_1AB9F8[];  /* sprintf format string (left composed line) */
extern char D_1ABA38[];  /* sprintf format string (right composed line) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D38C8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 25.33% -> STRUCTURAL,
 * first differing row @0: ROM `addiu sp,sp,-96` vs `lui v0,0x0  [HI16 0x001A7340]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 31.36% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-96` vs `addiu sp,sp,-144`. */
s32 func_002D38C8(void) {
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(g_screenWidth / 2);
    f32 row = (f32)D_1ABAB0;
    f32 yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    char buf[0x40];
    char *text;

    Begin2dDrawBatch(0);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0x8B), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.56f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0x8C), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.56f);
    func_003017F8(GuiFontAtlasLookupGlyph(atlas, 0x8D), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.56f);

    text = GetLocalizedString(0x30D5);
    func_002801B8(g_screenWidth / 2, 0x41, 0x80F0F0F0, text, -1);
    text = GetLocalizedString(0x2BE4);
    func_002801B8(g_screenWidth / 2, 0x141, 0x80F0F0F0, text, -1);
    text = GetLocalizedString(0x2BE5);
    func_002801B8(g_screenWidth / 2, 0x15A, 0x80F0F0F0, text, -1);

    func_00115DA8(buf, D_1AB9F8, GetLocalizedString(0x30D5));
    func_0027FBA8(0x86, 0xBA, 0x80F0F0F0, buf, func_001157AC(buf));

    func_00115DA8(buf, D_1ABA38, GetLocalizedString(D_1A790C != 0 ? 0x2C5C : 0x2C5D));
    func_002801B8(0x15D, 0xBA, 0x80F0F0F0, buf, func_001157AC(buf));

    End2dDrawBatch();
    return 0;
}
#endif

/* Forward the currently-pressed pad buttons to the menu input handler; return 0.
 * Byte-exact (task #468) once g_padButtonsPressed is declared ROM_SPLIT: the
 * "SN scheduling" of the old note was the compiler-split lui/%lo pair with the
 * frame addiu between, which the sized-extern macro could never produce. */
s32 func_002D3B88(void) {
    func_0029D478(g_padButtonsPressed);
    return 0;
}

/* Draw-batch wrapper. */
s32 func_002D3BB0(void) {
    Begin2dDrawBatch(0);
    func_0029D448();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll variant (same shape as func_002D3F78): confirm bit
 * (0x10) latches the active screen's pending result into block[0x18] (or returns
 * -1 when no pending sub-result); cancel bit (0x900) returns 1; otherwise ticks
 * the idle handler func_0029D4E8 and returns 0.
 * Wall: 8-byte-packed-save ($16 + $31) with the result-threaded-$16 /
 * branch-likely merge shape the later cc1 emits. Preserved as portable C. */
extern s32 func_0029D4E8(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3BE0);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 70.27% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 70.42% -> SPLIT-HIREG, first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. */
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame +
 * branch-likely confirm shape / single-register result threading. */
s32 func_002D3BE0(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D4E8(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3C68(void) {
    Begin2dDrawBatch(0);
    func_0029D4B8();
    End2dDrawBatch();
    return 0;
}

/* Cinematics/cutscene confirm poll: on the confirm (0x10) and cancel (0x900) pad
 * bits it first "unlocks" the current cutscene record (struct at g_health+0x464):
 * clamps the play-count at +0x1D4 toward g_gsPixelOffsetY+0x3C, raises the
 * high-water mark at +0x1D8 to that same value, and sets the bit for the current
 * g_playerProgress slot (plus the 0x80000000 sentinel) in the seen-mask at +0x1DC.
 * Confirm then returns the active screen's pending result tri-state (latched into
 * block[0x18]); cancel returns 1; the idle path ticks func_0029D528 and returns 0.
 * Wall: 8-byte-packed-save ($16 + $31) + bnel branch-likely dead-store shape.
 * Preserved as portable C. */
extern s32 func_0029D528(s32 padPressed);
extern s32 g_health;            /* 0x18C2EC - base of the per-cutscene unlock records at +0x464 */
extern s32 g_gsPixelOffsetY;    /* 0x1A7354 - play-count source at +0x3C */
extern s32 g_playerProgress;    /* 0x1A79F8 - current progress slot (seen-mask bit index) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3C98);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 32.48% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 33.88% -> SPLIT-HIREG, first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. */
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame +
 * bnel branch-likely dead stores / single-register result threading. */
static void MenuCutsceneUnlockCurrent(void) {
    u8 *rec = (u8 *)&g_health + 0x464;
    s32 target = *(s32 *)((u8 *)&g_gsPixelOffsetY + 0x3C);
    if (*(u16 *)(rec + 0x1D4) <= 0xFFFE) {
        *(u16 *)(rec + 0x1D4) = (u16)(target + 1);
    }
    if (*(s32 *)(rec + 0x1D8) < target) {
        *(s32 *)(rec + 0x1D8) = target;
    }
    *(u32 *)(rec + 0x1DC) |= (1u << g_playerProgress) | 0x80000000u;
}
s32 func_002D3C98(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v;
        MenuCutsceneUnlockCurrent();
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        MenuCutsceneUnlockCurrent();
        return 1;
    }
    func_0029D528(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3DD8(void) {
    Begin2dDrawBatch(0);
    func_0029D568();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll variant (idle handler func_0029D598). Same shape as
 * func_002D3F78. Wall: 8-byte-packed-save ($16 + $31) + branch-likely confirm
 * shape / single-register result threading. Preserved as portable C. */
extern s32 func_0029D598(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3E08);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 70.27% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 70.42% -> SPLIT-HIREG, first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. */
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame. */
s32 func_002D3E08(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D598(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3E90(void) {
    Begin2dDrawBatch(0);
    func_0029D5D8();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll variant (idle handler func_0029D608). Same shape as
 * func_002D3F78. Wall: 8-byte-packed-save ($16 + $31) + branch-likely confirm
 * shape / single-register result threading. Preserved as portable C. */
extern s32 func_0029D608(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3EC0);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 70.27% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 70.42% -> SPLIT-HIREG, first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. */
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame. */
s32 func_002D3EC0(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D608(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3F48(void) {
    Begin2dDrawBatch(0);
    func_0029D648();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll: on the "confirm" pad bit (0x10) returns the active
 * screen's pending result (latches it into screen[0x18]) or -1 when the screen
 * has no pending sub-result; on a "back/cancel" bit (0x900) returns 1; otherwise
 * ticks the idle handler func_0029D838 and returns 0.
 * Wall: 8-byte-packed-save (saves $16 + $31) + the load-PRE/branch-likely shape.
 * Preserved as portable C. */
extern s32 func_0029D838(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3F78);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 70.27% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 70.42% -> SPLIT-HIREG, first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002D3F78(void) {
    s32 flags = g_padButtonsPressed;
    s32 *ss = (s32 *)(g_particleFxBlob + 0x100);
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)ss + 0x14) + 0xE0);
        if (v != 0) {
            ss[6] = v; /* screen[0x18] */
            return 0;
        }
        if (ss[0x4D] == 0) { /* screen[0x134] */
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D838(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4000(void) {
    Begin2dDrawBatch(0);
    func_0029D878();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll variant (idle handler func_0029D7C8). Same shape as
 * func_002D3F78. Wall: 8-byte-packed-save ($16 + $31) + branch-likely confirm
 * shape / single-register result threading. Preserved as portable C. */
extern s32 func_0029D7C8(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4030);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 70.27% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 70.42% -> SPLIT-HIREG, first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. */
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame. */
s32 func_002D4030(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D7C8(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D40B8(void) {
    Begin2dDrawBatch(0);
    func_0029D808();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll variant with a deferred game-state action: confirm
 * (0x10) and cancel (0x900) behave like func_002D3F78; the idle path ticks
 * func_0029D678 and, when that signals (nonzero), fires MenuScreenDoAction(0xD,0,
 * &outFlag) — opcode 0xD is a RequestGameStateChange — with the out-flag slot
 * pre-zeroed. Wall: 8-byte-packed-save ($16 + $31). Preserved as portable C. */
extern s32 func_0029D678(s32 padPressed);
extern s32 MenuScreenDoAction(s32 opcode, s32 arg, s32 *outFlag);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D40E8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 68.15% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 74.95% -> SPLIT-HIREG, first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. */
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame +
 * branch-likely confirm shape. */
s32 func_002D40E8(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    if (func_0029D678(flags) != 0) {
        s32 outFlag = 0;
        MenuScreenDoAction(0xD, 0, &outFlag);
    }
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4188(void) {
    Begin2dDrawBatch(0);
    func_0029D6B8();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll variant (idle handler func_0029D6E8). Same shape as
 * func_002D3F78. Wall: 8-byte-packed-save ($16 + $31) + branch-likely confirm
 * shape / single-register result threading. Preserved as portable C. */
extern s32 func_0029D6E8(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D41B8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 70.27% -> SPLIT-HIREG,
 * first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 70.42% -> SPLIT-HIREG, first differing row @0: ROM `lui v1,0x0  [HI16 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. */
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame. */
s32 func_002D41B8(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D6E8(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4240(void) {
    Begin2dDrawBatch(0);
    func_0029D728();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll driven by the global input flag word (D_138180[0x1C4])
 * that also forwards a selected GUI-list entry as a menu command. Confirm (0x10)
 * latches the active screen's pending result like the other polls; cancel (0x900)
 * returns 1; otherwise it ticks the idle handler func_0029D758 and, on the
 * confirm pad bit (0x40) with the GUI up, reads the selected entry of the list
 * widget at g_guiInstance+0x3F3F0 (func_0033F360) and builds a command record
 * (opcode = entry[0x8], arg = entry[0xC]) handed to func_002D6B00 (-> MenuScreenDoAction).
 * Wall: 8-byte-packed-save ($16 + $17 + $31). Preserved as portable C. */
extern s32 func_0029D758(s32 padPressed);
extern void *func_0033F360(void *widget);
extern void func_002D6B00(void *record);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4270);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 67.61% -> FRAME-SIZE,
 * first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-64`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 61.65% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-80`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely confirm shape / single-register result threading. */
s32 func_002D4270(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D758(flags);
    if ((*(s32 *)(D_138180 + 0x1C4) & 0x40) && g_guiInstance) {
        u8 record[0x30];
        u8 *entry = (u8 *)func_0033F360(g_guiInstance + 0x3F3F0);
        *(u16 *)(record + 0x2) = *(u16 *)(entry + 0x8);
        *(s32 *)(record + 0x4) = *(s32 *)(entry + 0xC);
        func_002D6B00(record);
    }
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4340(void) {
    Begin2dDrawBatch(0);
    func_0029D798();
    End2dDrawBatch();
    return 0;
}

/* GUI wrapper: forward a widget at instance+0x3F3F0 to func_0033F3B8(&D_2617C0).
 * Byte-exact (task #468) once D_2617C0 is declared ROM_SPLIT: the interleave of
 * the old note is the compiler-split %lo(D_2617C0) scheduled against the
 * 0x3F3F0 ori, which the sized-extern macro left as one late `la`. */
s32 func_002D4370(void) {
    if (g_guiInstance) {
        func_0033F3B8(g_guiInstance + 0x3F3F0, &D_2617C0);
    }
    return 0;
}

/* Confirm/cancel poll driven by the global input flag word (D_138180[0x1C4]).
 * Confirm (0x10) latches the active screen's pending result like the other
 * polls; cancel (0x900) returns 1; otherwise ticks the idle handler
 * func_0029D408 with D_138180[0x1C0] and returns 0.
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern void func_0029D408(s32 arg);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D43B0);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 74.65% -> STRUCTURAL,
 * first differing row @2: ROM `sd s0,0(sp)` vs `sd ra,0(sp)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 66.26% -> STRUCTURAL, first differing row @0: ROM `(none)` vs `lui v1,0x0  [HI16 D_138180]`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely confirm shape. */
s32 func_002D43B0(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D408(*(s32 *)(D_138180 + 0x1C0));
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4438(void) {
    Begin2dDrawBatch(0);
    func_0029D3D8();
    End2dDrawBatch();
    return 0;
}

/* GUI wrapper: when the GUI is up, configure the list widget at instance+0x3C480
 * — func_003444D0(w, 0) (mode), func_00344458(w, &D_259F38) (bind data),
 * func_003444A0(w) (rebuild), func_003444C0(w, &D_25D0C0) (bind labels). Returns 0.
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern void func_003444D0(void *widget, s32 mode);
extern void func_00344458(void *widget, void *data);
extern void func_003444A0(void *widget);
extern void func_003444C0(void *widget, void *labels);
extern u8 D_259F38, D_25D0C0, D_259CC0, D_25D268;
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4468);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 50.16% -> STRUCTURAL,
 * first differing row @1: ROM `lui a0,0x0  [HI16 0x001A8D04]` vs `(none)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 66.38% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002D4468(void) {
    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C480;
        func_003444D0(w, 0);
        func_00344458(w, &D_259F38);
        func_003444A0(w);
        func_003444C0(w, &D_25D0C0);
    }
    return 0;
}
#endif

/* GUI wrapper: twin of func_002D4468 for the same widget (instance+0x3C480) with
 * the alternate mode/data/labels (func_003444D0(w,1), &D_259CC0, &D_25D268).
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D44E8);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 53.59% -> STRUCTURAL,
 * first differing row @1: ROM `lui a0,0x0  [HI16 0x001A8D04]` vs `(none)`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 59.19% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. */
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002D44E8(void) {
    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C480;
        func_003444D0(w, 1);
        func_00344458(w, &D_259CC0);
        func_003444A0(w);
        func_003444C0(w, &D_25D268);
    }
    return 0;
}
#endif

/* Refresh a weapon-swap widget's two caption/price fields from the currently
 * selected weapon. With the GUI up, lays out the swap gadget (func_00342460 on
 * g_guiInstance+0x3C160), then reads the selected weapon slot for the gadget at
 * g_guiInstance+0x3C480 (slot = g_itemEquippedSlot[func_00343AD0(gadget+0x2C8)])
 * and stores that weapon's g_weaponTable field +0x42 into the widget arg's +0x34;
 * then fetches the gadget's sub-widget (func_003444C8 = the pointer at +0x360) and
 * stores the selected weapon's g_weaponTable field +0x6 into that sub-widget's
 * +0x58. Returns 0.
 * (The original calls the void forwarder func_00344480, whose only effect is
 * func_00343AD0(p+0x2C8); we call func_00343AD0 directly to read its index return
 * — the same idiom the sister unit uses at func_00344808. Matching arm stays
 * INCLUDE_ASM: 8-byte-packed-save wall.) */
extern void func_00342460(void *widget, s32 arg);
extern s32 func_00343AD0(void *p);
extern s32 func_003444C8(void *p);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4568);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 43.95% -> STRUCTURAL,
 * first differing row @1: ROM `lui v0,0x0  [HI16 0x001A8D04]` vs `daddu v0,zero,zero`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 54.23% -> PACKED-SAVE, first differing row @0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-112`. */
s32 func_002D4568(void *widget) {
    u8 *gadget;
    u8 *subWidget;
    s32 slot;

    if (g_guiInstance == NULL)
        return 0;

    func_00342460(g_guiInstance + 0x3C160, 1);

    gadget = (u8 *)g_guiInstance + 0x3C480;
    slot = g_itemEquippedSlot[func_00343AD0(gadget + 0x2C8)];
    *(s32 *)((u8 *)widget + 0x34) = *(s16 *)(g_weaponTable + slot * 0xE0 + 0x42);

    subWidget = (u8 *)func_003444C8(gadget);
    slot = g_itemEquippedSlot[func_00343AD0(gadget + 0x2C8)];
    *(s32 *)(subWidget + 0x58) = *(s16 *)(g_weaponTable + slot * 0xE0 + 0x6);
    return 0;
}
#endif

/* menu input/update handler: uses 128-bit sq/lq (vector) loads/stores — left as INCLUDE_ASM
 * (the EE quadword ops are not emitted from scalar C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateShipCustomizeInput);

/* Draws a two-segment horizontal gradient bar (an audio/level meter) for the
 * D_26CFD0[idx] record. Each segment's fill width comes from `val` offset by +2 and
 * +0x28: width = 0x80 - toInt(|256 - (val+off)| * 0.75), clamped >= 0, packed as the
 * high (alpha) byte of colour 0x00F0F0B0. func_0028F2C0 renders the quad (corners
 * alternate the two colours) into the element handle from
 * func_0028EDF0(0xEA97, record[0x10]). The magnitude/scale math runs through the
 * SDK soft-double helpers (float->double func_001234F0, ordered compare
 * func_00123028, subtract func_00122A98, multiply func_00122B00, double->int
 * func_00123130). Return value is unused (the original leaves v0 as the void
 * func_0028F2C0's leftover). Matching arm stays INCLUDE_ASM (soft-float call
 * scheduling + 9-GPR packed save). */
extern s32 func_0028EDF0(s32 u, s32 v);
extern s64 func_001234F0(float f);
extern s32 func_00123028(s64 a, s64 b);
extern s64 func_00122A98(s64 a, s64 b);
extern s64 func_00122B00(s64 a, s64 b);
extern s32 func_00123130(s64 x);
extern void func_0028F2C0(s32 handle, s32 x0, s32 y0, s32 x1, s32 y1, void *quad);
extern u8 D_26CFD0[];    /* meter record table, stride 0x14 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4D38);
#else
/* t468 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) 23.74% -> STRUCTURAL,
 * first differing row @0: ROM `addiu v1,zero,20` vs `sll v0,a0,0x2`;
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 40.03% -> STRUCTURAL, first differing row @0: ROM `addiu v1,zero,20` vs `addiu a2,zero,20`. */
#define METER_FILL_SCALE 0x3FE8000000000000LL   /* 0.75 (built ori 0xFFA0; dsll32 14) */

/* fill width for one meter segment: 0x80 - round(|256 - v| * 0.75), soft-double. */
static s32 MeterSegmentWidth(s32 v) {
    s64 d = func_001234F0(256.0f - (f32)v);
    if (func_00123028(d, 0) < 0)
        d = func_00122A98(0, d);
    return 0x80 - func_00123130(func_00122B00(d, METER_FILL_SCALE));
}

void func_002D4D38(s32 idx, s32 val) {
    u8 *rec = D_26CFD0 + idx * 0x14;
    s32 handle = func_0028EDF0(0xEA97, *(s32 *)(rec + 0x10));
    s32 w1 = MeterSegmentWidth(val + 2);
    s32 w2 = MeterSegmentWidth(val + 0x28);
    u32 color1, color2;
    s32 quad[4];

    if (w2 < 0) w2 = 0;
    if (w1 < 0) w1 = 0;
    color1 = ((u32)w1 << 24) | 0xF0F0B0;
    color2 = ((u32)w2 << 24) | 0xF0F0B0;
    quad[0] = color1;
    quad[1] = color2;
    quad[2] = color1;
    quad[3] = color2;
    func_0028F2C0(handle, val + 2, 0x14E, 0x28, 0x28, quad);
}
#endif

/* menu/HUD draw routine: ldl/ldr/sdl/sdr unaligned struct/const copy — left as INCLUDE_ASM
 * (cc1 won't reproduce the unaligned 64-bit copy idiom from clean C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawShipCustomizeMenu);

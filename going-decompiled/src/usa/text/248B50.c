#include "common.h"

/*
 * text/248B50 — first GUI-block sub-chunk (carve-pipeline pick #3a,
 * 2026-06-13; vaddr 0x348BD0..0x34C007, 65 fns): a run of single-field GUI
 * widget accessors (setters/getters over the widget object at $a0) plus the
 * pop-up menu widget and a couple of screen-init helpers.
 *
 * The matcher builds THIS unit at -O2 -G8 -fno-gcse (per-unit GFLAG/CC1EXTRA
 * override in tools/ee/objdiff_build.sh / diff.sh / build.sh) — the later-SN-
 * cc1 TU model shared by the other gameplay-text units.
 *
 * SAVE-LAYOUT WALL: the larger methods here save two or more callee GPRs at
 * 8-byte slot spacing (the pinned cc1 reserves 16 bytes/save) and stay
 * INCLUDE_ASM. The giant switch function func_0034C008 that follows this
 * range is left in the tail asm (the jtbl reloc-identity gap blocks every
 * switch function in a carved unit). Several single-field accessors are
 * sub-100% near-misses (instruction-identical but for delay-slot fill /
 * register allocation / branch-likely layout) and are kept INCLUDE_ASM with
 * a per-function WALL note.
 *
 * The accessors take a widget pointer in $a0 and read/write a field at a
 * fixed offset; the struct is a flat byte view with only the touched fields
 * typed.
 */

/* GUI widget — flat field view (offsets are the byte displacements the
 * accessors use; only the fields touched by the matched leaf accessors are
 * named/typed). */
typedef struct GuiWidget {
    /* 0x00 */ f32 unk00;
    /* 0x04 */ u8 pad04[0x4];
    /* 0x08 */ union { f32 *p; } extentVec;
    /* 0x0C */ u8 pad0C[0xC];
    /* 0x18 */ f32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ u8 pad30[0x2C];
    /* 0x5C */ union { f32 *p; } originVec;
    /* 0x60 */ u8 pad60[0x20];
    /* 0x80 */ s32 unk80;
    /* 0x84 */ s32 unk84;
    /* 0x88 */ u8 pad88[0xB8];
    /* 0x140 */ s32 unk140;
    /* 0x144 */ s32 unk144;
    /* 0x148 */ u8 pad148[8];
    /* 0x150 */ s32 unk150;
} GuiWidget;

/* Compile-time layout verification, same convention as include/gui.h: ACTIVE ONLY
 * under a 4-byte-pointer compile (native -m32 linter). ee-gcc 2.9 predates
 * __SIZEOF_POINTER__, so these are INERT in the matching build and are NOT what
 * holds the offsets there — the byte-exact verify is: a wrong displacement lands
 * in the emitted lw/sw and verify_match_unit.sh reports DIFFERS. */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(__builtin_offsetof(GuiWidget, extentVec) == 0x08, "GuiWidget.extentVec @ +0x08");
_Static_assert(__builtin_offsetof(GuiWidget, originVec) == 0x5C, "GuiWidget.originVec @ +0x5C");
_Static_assert(__builtin_offsetof(GuiWidget, unk80)     == 0x80, "GuiWidget.unk80 @ +0x80");
_Static_assert(sizeof(GuiWidget) == 0x154, "GuiWidget size unchanged");
#endif

/* GuiAnim — the animated sub-widget the func_0034A7F8 / func_0034A860 / func_0034A3C0
 * family drives (a SEPARATE field layout from GuiWidget: its progress lives at
 * +0x10, not +0x18). It is a scalar Hermite-eased transition over a small table
 * of keyframe records.
 *   +0x00 c0    first control point (Hermite arg 2)
 *   +0x04 c1    second control point (Hermite arg 3)
 *   +0x08 c2    extra control point (used by the +0x20 callback path)
 *   +0x0C c3    extra control point
 *   +0x10 progress   eased phase in [0,1] (f32; reset stores integer 0)
 *   +0x14 dir        play direction (+1 forward / -1 reverse / 0 idle)
 *   +0x18 step       per-frame phase increment (f32)
 *   +0x1C active     non-zero while the transition is running
 *   +0x20 sink       optional s32* the blended colour word is written through
 *   +0x24 keyframe table base — records the per-channel lerp endpoints walk
 *   +0x8C colorA / +0x90 colorB   packed colour words blended by ColorLerpPacked */
typedef struct GuiAnim {
    /* 0x00 */ f32 c0;
    /* 0x04 */ f32 c1;
    /* 0x08 */ f32 c2;
    /* 0x0C */ f32 c3;
    /* 0x10 */ f32 progress;
    /* 0x14 */ s32 dir;
    /* 0x18 */ f32 step;
    /* 0x1C */ s32 active;
    /* 0x20 */ s32 *sink;
    /* 0x24 */ u8 keyframes[0x68];
    /* 0x8C */ u32 colorA;
    /* 0x90 */ u32 colorB;
} GuiAnim;

/* Guard the layout assert on C11: the ee-gcc 2.9 (C89) matching toolchain lacks
 * _Static_assert and would emit a parse error; the host clang gate (C11) checks it. */
#if defined(TARGET_NATIVE) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(GuiAnim) == 0x94, "GuiAnim layout");
#endif

/* player character/control mode (0x18C0D4): 0 Ratchet, 1 Clank-solo, 2 Giant Clank. */
extern u8 g_bPlayerMode;

/* GUI instance root (g_guiInstance): the glyph-font owner; widget glyph setters
 * index it at +0x8710 to reach the glyph atlas. */
extern u8 *g_guiInstance;

#ifdef TARGET_NATIVE
/* forward decls for the matched-but-defined-later helpers the #else bodies call,
 * so the ILP32 compile gate sees their real signatures (not an implicit int()). */
/* Empty-paren externs for GUI primitives referenced only by #else bodies whose
 * call-site arg types vary (unspecified args -> no arg-check -> no prototype
 * conflict); resolves this unit's latent implicit-declaration warning-debt.
 * Byte-neutral: the matching arms never reference these C symbols. */
extern char *GetLocalizedString();
extern void *func_003368D0();
extern void GuiElementInit();
extern void GuiElementInitTypeB();
extern void GuiElementInitTypeC();
extern void GuiElementSetGlyph();
extern void GuiElementSetText();
extern void GuiListRowElementInit();
extern void *func_003374D8();

void func_0034A318(GuiWidget *w, s32 idx, f32 a, f32 b, f32 c, f32 d, f32 e);
void func_0034A350(GuiWidget *w, s32 idx, f32 a, f32 b);
void func_0034A7B0(GuiWidget *w, s32 idx1, s32 idx2, f32 a, f32 b, f32 c, f32 d);

/* game callees used by GuiMenuListDraw (GuiMenuListDraw). Declared here (not in a
 * central header) with their real signatures so the ILP32 gate sees them; they
 * stay INCLUDE_ASM / runtime stubs natively. */
f32 *func_00336C18(GuiWidget *e);                                /* scratch vec2 */
void GuiElementSetScale(GuiWidget *e, f32 x, f32 y, f32 z, f32 w);
/* GuiElementSetText / GetLocalizedString are declared empty-paren in the block
 * above: their char* call sites vary, so an empty-paren prototype resolves the
 * implicit-decls without the conflict a fully-typed prototype would cause. */
s32 *GuiElementGetColor(GuiWidget *e);
s32 GuiTextElementMeasure(GuiWidget *e);
void GuiTextElementDraw(GuiWidget *e);
/* DrawFlatRect2d: corners (x1,y1)-(x2,y2) at depth z, fill = pointer to a packed
 * 64-bit GS colour word (tex0 is a pointer despite the historic u64 typing). */
void func_0027F168(s32 x1, s32 y1, s32 x2, s32 y2, s64 z, u64 tex0);
/* func_0027F790: the unconditional tail call; real defined symbol (returns s32,
 * called for side effect). Identity not yet confirmed - keep the func_ name. */
s32 func_0027F790(void);

/* PlayGlobalSound(id, a, b): fire-and-forget global UI sound (0x1D9B24). */
void PlayGlobalSound(s32 id, s32 a, s32 b);

/* callees used by the GuiAnim transition family (func_0034A860 / func_0034A3C0). */
/* GuiHermiteInterp(t, c0, c1, c2, c3): cubic Hermite blend (0x34FBC0). */
f32 GuiHermiteInterp(f32 t, f32 c0, f32 c1, f32 c2, f32 c3);
/* ColorLerpPacked(colorA, colorB, t): per-channel byte lerp of two packed colour
 * words by phase t (0x2846E8); returns the blended word. */
u32 ColorLerpPacked(u32 colorA, u32 colorB, f32 t);
#endif

/* func_00348BD0: run the type-C element init on the widget and return it.
 * w: the widget; returns w. The original packs the two callee saves ($16,$31)
 * into a 0x10 frame (8-byte slots), which cc1 2.9 cannot emit (0x20 frame,
 * 16-byte slots: 99.6%, the wall recorded below); SN 2.95.3 v1.36 with
 * -fopt-stack emits it byte-exact (FACT #8810).
 * GUARD (task #1257): on EE this C is the image's body, compiled alone by the
 * s136os arm (tools/ee/s136os_functions.txt) and spliced over the S136OS_SLOT
 * line by tools/ee/s136os_splice.sh; the 2.9 compile sees only the slot. On
 * native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00348BD0)
S136OS_SLOT(func_00348BD0);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 99.60% / engine96 80.00%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 4/10 words differ;
 * frozen-.s census: 2 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE ONLY — all 4 differing words are frame/save-slot (addiu sp -0x20 vs -0x10, sd/ld ra 0x10 vs 0x8); every other word is identical, so this function is byte-exact the moment the 8-byte callee-save slot model is available. */
extern void GuiElementInitTypeC(void *element);
void *func_00348BD0(void *w) {
    GuiElementInitTypeC(w);
    return w;
}
#endif

/* func_00348BF8: build/init a GUI text element — alloc its backing object via
 * GuiPoolAlloc + GuiPlacementNew, run GuiTextElementInit, then seed the colour/
 * style fields (+0xAC..+0xCC). WALL (matching arm): 3 callee saves ($16,$17,$31)
 * — the pinned cc1's 0x20 frame with 16-byte save slots diverges from the
 * original's packed 0x10/8-byte layout; matching arm stays asm. The #else is the
 * functional model. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348BF8);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 92.60% / engine96 67.17%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 31/48 words differ;
 * frozen-.s census: 3 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE (3 callee GPR saves) + REGALLOC: 8 frame/save-slot words incl. save ORDER (s0/s1 swapped), plus s0/s1-vs-a0/a1 colouring. */
extern void *GuiPoolAlloc(void *pool);
extern void *GuiPlacementNew(s32 size, void *at);
extern void GuiTextElementInit(void *element, void *tmpl, void *pool);
extern u8 D_1AE568[]; /* text-element init template/config */
void func_00348BF8(void *w, void *pool) {
    char *el = (char *)w;
    void *obj;

    *(s32 *)(el + 0x60) = 0;
    *(s32 *)(el + 0xC0) = 0;
    *(void **)(el + 0x58) = pool;
    if (pool != 0) {
        obj = GuiPoolAlloc(pool);
        obj = GuiPlacementNew(0x10, obj);
        *(void **)(el + 0x5C) = obj;
        *(s32 *)((char *)obj + 0x0) = 0;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
    }
    GuiTextElementInit(el, D_1AE568, pool);
    *(s32 *)(el + 0xCC) = -1;
    *(s32 *)(el + 0xAC) = 0x70FFFEED;
    *(s32 *)(el + 0xB0) = 0x80F0F0F0;
    *(s32 *)(el + 0xB4) = 0x80808080;
    *(s32 *)(el + 0xBC) = 0x20;
    *(s32 *)(el + 0x54) = 0;
    *(s32 *)(el + 0xB8) = 0;
    *(s32 *)(el + 0xC4) = 0;
    *(s32 *)(el + 0xC8) = 0;
}
#endif

/* GuiMenuListHandleInput: menu selection-advance driven by the per-frame input mask.
 * The widget keeps the current row index at +0x60, the row-enable table (one
 * non-zero word per selectable row) at +0x6C, and the row count at +0xC0.
 *
 *  - mask & 0x1000 (LEFT/PREV): dir = -1
 *  - mask & 0x4000 (RIGHT/NEXT): dir = +1
 *  - mask & 0x40   (CONFIRM, only when neither nav bit set): if the current row
 *      is selectable (+0x6C[cur] != 0) return 1 immediately; otherwise dir stays
 *      0 and the function returns 0 below.
 *  - no relevant bit / dir == 0: return 0 (no sound, no nav).
 *
 * On a nav, play the move sound (PlayGlobalSound(3,0,0)), then step the index by
 * dir with wrap (past the last row -> 0, before row 0 -> count-1), writing each
 * candidate to +0x60, until a selectable row is found or the scan returns to the
 * starting row. Returns 0 from every nav path; 1 only from the confirm-hit path.
 *
 * WALL: 4 callee saves ($16,$17,$18,$31) — frame-layout divergence. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", GuiMenuListHandleInput);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 37.52% / engine96 63.52%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 49/56 words differ;
 * frozen-.s census: 4 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE (4 callee GPR saves) — 5 of the 49 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
s32 GuiMenuListHandleInput(GuiWidget *w, u32 inputMask) {
    s32 *curIdx = (s32 *)((char *)w + 0x60);
    s32 *rowEnable = (s32 *)((char *)w + 0x6C);   /* rowEnable[idx] != 0 => selectable */
    s32 count = *(s32 *)((char *)w + 0xC0);
    s32 dir;
    s32 start;
    s32 cand;

    if (inputMask & 0x1000) {
        dir = -1;
    } else if (inputMask & 0x4000) {
        dir = 1;
    } else {
        dir = 0;
        if (inputMask & 0x40) {
            /* confirm: a selectable current row returns 1 immediately; an
             * unselectable one leaves dir 0 and falls to the no-nav return. */
            if (rowEnable[*curIdx] != 0) {
                return 1;
            }
        }
    }

    /* no nav direction (no nav bit, or confirm-miss): return without sound. */
    if (dir == 0) {
        return 0;
    }

    PlayGlobalSound(3, 0, 0);

    start = *curIdx;
    for (;;) {
        cand = *curIdx + dir;
        if ((count - 1) < cand) {
            cand = 0;            /* wrapped past the last row */
        }
        if (cand < 0) {
            cand = count - 1;    /* wrapped before the first row */
        }
        *curIdx = cand;
        if (rowEnable[cand] != 0) {
            return 0;            /* landed on a selectable row */
        }
        if (start == cand) {
            return 0;            /* scanned every row, none selectable */
        }
    }
}
#endif

/* func_00348D98: handwritten epilogue-only stump (`addiu $sp,$sp,0x10; nop`,
 * no prologue, no `jr ra`) — the trailing half of a hand-split asm routine.
 * No C body can reproduce a function with no return. WALL: split-artifact stub. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348D98);

/* A float read with alias set 0 (see GuiMenuListSetOrigin): the union type
 * stops cc1's strict-aliasing scheduler from moving the read across an
 * integer store, the way the ROM's no-strict-aliasing compiler never did. */
typedef union {
    f32 f;
    s32 i;
} GuiAliasWord;

/*
 * GuiMenuListSetRows - point the menu list at a keyframe table and count its
 * rows.
 *
 *   w      the menu-list widget
 *   table  keyframe table, stride 5 floats (0x14 bytes); a row counts while
 *          its first float is > 0, at most 80 floats' worth
 * Stores table at +0x68 and the row count at +0xC0 (bumped in place while
 * scanning). If the cursor at +0x60 is no longer below the count, it is reset
 * to 0. No return. A NaN first float ends the scan, as `0 < x` fails for NaN.
 *
 * Byte-exact on the sdk29 arm (task #894; was 47.33% solo). The ROM is the
 * rotated loop of the do-while below, in near source order:
 *  - the empty asm keeps the two leading stores ahead of the float setup;
 *  - the count increment is written before `table += 5`, so the count load
 *    heads the body and reorg steals it into the annulled `bnezl` slot;
 *  - the loop-test read of *table goes through GuiAliasWord, so it stays
 *    after the count store;
 *  - the final test is written `cursor >= count`, which loads +0x60 first.
 * Oracle: cmp_func_00348DA0 (cmp_248B50.c), bit-exact on real R5900.
 */
void GuiMenuListSetRows(GuiWidget *w, f32 *table) {
    f32 *end;

    *(f32 **)((char *)w + 0x68) = table;
    *(s32 *)((char *)w + 0xC0) = 0;
    __asm__ __volatile__("");
    end = table + 0x50;
    if (0.0f < *table) {
        do {
            *(s32 *)((char *)w + 0xC0) += 1;
            table += 5;
        } while (0.0f < ((GuiAliasWord *)table)->f && (s32)table < (s32)end);
    }
    if (*(s32 *)((char *)w + 0x60) >= *(s32 *)((char *)w + 0xC0)) {
        *(s32 *)((char *)w + 0x60) = 0;
    }
}

/* func_00348E10: set the +0xB8 / +0xBC field pair (a1 -> +0xB8, a2 -> +0xBC).
 * The ROM stores +0xBC first, then fills the jr delay slot with the +0xB8 store.
 * NOT A WALL: the #else body below reproduces exactly that and is byte-exact --
 * 12 B cmp IDENTICAL, 3/3 words, 0 relocs in range (audit /19066, independently
 * re-derived /19147). The "Best 96%" this comment used to carry was a real
 * measurement of the DESCENDING source form, which this file has never
 * committed; writing the two stores in ascending source order is what matches.
 * ARM FLIPPED (/19536, /19537): the guard is gone and the EE build now compiles
 * this body. Zero code change -- the text below is character-for-character what
 * the #else held. target_relocs == base_relocs == 0, trivially so (no operand
 * here needs a relocation), which rules out reloc-normalisation over-reporting
 * without itself being the evidence. */
void func_00348E10(GuiWidget *w, s32 a, s32 b) {
    *(s32 *)((char *)w + 0xB8) = a;
    *(s32 *)((char *)w + 0xBC) = b;
}

/* func_00348E20: store a1 to the +0xC8 field. */
void func_00348E20(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0xC8) = v;
}

/* GuiMenuListSetOrigin: set the menu list's layout origin.
 *   w - the menu-list widget; x, y - the new origin in 2D screen units.
 * Returns nothing. Writes x,y into [0],[1] of the vec4 the widget points at
 * from +0x5C and zeroes [2],[3] (the z/w lanes), so the whole vec4 is defined.
 * Non-obvious: the origin pointer is RE-READ from +0x5C before each of the four
 * stores, alternating two scratch registers. That is not redundancy to optimise
 * away -- it is what the ROM's no-strict-aliasing compiler emitted, and
 * reproducing it is what makes this byte-exact. The union-typed field gives the
 * access alias set 0 so this cc1 cannot CSE the reload away.
 * t562 @e3f50d43: the former "WALL: just-in-time reload register alternation"
 * (dated to before 2026-09-21) is FALSE -- the wall was the reload, not the
 * register alternation. -> UNION-RELOAD, 100.00% (unit objdiff report,
 * objdiff_build.sh, clean), verify_match_unit.sh BYTE IDENTICAL 10/10 words. */
void GuiMenuListSetOrigin(GuiWidget *w, f32 x, f32 y) {
    w->originVec.p[0] = x;
    w->originVec.p[1] = y;
    ((s32 *)w->originVec.p)[2] = 0;
    ((s32 *)w->originVec.p)[3] = 0;
}

/* func_00348E50: store a1 to the +0x60 field. */
void func_00348E50(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0x60) = v;
}

/* func_00348E58: store a1 to the +0xCC field. */
void func_00348E58(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0xCC) = v;
}

/* func_00348E60: return the +0x60 field. */
s32 func_00348E60(GuiWidget *w) {
    return *(s32 *)((char *)w + 0x60);
}

/* func_00348E68: return the +0xC0 field. */
s32 func_00348E68(GuiWidget *w) {
    return *(s32 *)((char *)w + 0xC0);
}

/* GuiMenuListDraw / GuiMenuListDraw: per-frame draw of a vertical text-menu/list
 * widget. For each of +0xC0 rows it positions a shared text element via the
 * +0x5C origin (origin[0]=x in scratch[0], origin[1]+rowYBase+rowYStep*i in
 * scratch[1]), sets its scale (+0x68 entry stride 0x14, entry[0]=scale) and text
 * (raw id when +0xC4, else localized), nudges the y by 3px for the 0x307A
 * sentinel id, colours it (disabled +0xB4 when the +0x6C row flag is 0, else
 * normal +0xB0 / selected +0xAC when +0x60 highlight == i) and optionally draws
 * underline rects (left underline when +0xCC == i; selected underline when
 * highlighted and +0xC8 set), then GuiTextElementDraw. PURE/PORTABLE — only the
 * draw callees touch hardware. Functional equivalent, not byte-exact. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", GuiMenuListDraw);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 28.20% / engine96 24.17%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 210/216 words differ;
 * frozen-.s census: 8 callee GPR saves, 4 fp saves.
 * Residual: PACKED-SAVE (8 callee GPR saves, 4 fp) — 12 of the 210 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
void GuiMenuListDraw(GuiWidget *self) {
    char *p = (char *)self;
    f32 *scratch = func_00336C18(self);
    s32 rowCount;
    s32 i;

    scratch[0] = ((f32 *)(*(void **)(p + 0x5C)))[0];   /* origin[0] -> scratch[0] */

    rowCount = *(s32 *)(p + 0xC0);
    if (rowCount > 0) {
        for (i = 0; i < rowCount; i++) {
            char *entry = *(char **)(p + 0x68) + i * 0x14;
            s32 rowYBase = *(s32 *)(p + 0xB8);
            s32 rowYStep = *(s32 *)(p + 0xBC);
            f32 originY = ((f32 *)(*(void **)(p + 0x5C)))[1];

            scratch[1] = (f32)(rowYBase + rowYStep * i) + originY;

            GuiElementSetScale((GuiWidget *)self, *(f32 *)entry, 0.0f, 0.0f, 0.0f);

            if (*(s32 *)(entry + 4) == 0x307A) {
                scratch[1] += 3.0f;
            }

            if (*(s32 *)(p + 0xC4)) {
                GuiElementSetText((GuiWidget *)self, *(s32 *)(entry + 4));
            } else {
                GuiElementSetText((GuiWidget *)self,
                                  (s32)GetLocalizedString(*(s32 *)(entry + 4)));
            }

            if (*(s32 *)(p + i * 4 + 0x6C) == 0) {
                /* disabled row */
                *GuiElementGetColor((GuiWidget *)self) = *(s32 *)(p + 0xB4);
            } else {
                f32 originX = scratch[0];
                f32 rowY = scratch[1];

                if (*(s32 *)(p + 0xCC) == i) {
                    /* left underline rect (selected colour fill) */
                    u32 fill = (*(u32 *)(p + 0xAC) & 0x00FFFFFF) | 0x20000000;
                    u64 packed = ((u64)fill << 32) | fill;
                    s32 w1 = GuiTextElementMeasure((GuiWidget *)self) >> 1;
                    s32 y1 = (s32)(rowY + 2.0f);
                    s32 x1 = (s32)(originX - (f32)w1 - 2.0f - 16.0f);
                    s32 w2 = GuiTextElementMeasure((GuiWidget *)self) >> 1;
                    s32 y2 = (s32)(rowY - 2.0f + 16.0f);
                    s32 x2 = (s32)(originX - (f32)w2 - 2.0f - 16.0f + 8.0f);
                    func_0027F168(x1, y1, x2, y2, 0, (u64)(unsigned long)&packed);
                }

                if (*(s32 *)(p + 0x60) != i) {
                    /* normal (unhighlighted) row */
                    *GuiElementGetColor((GuiWidget *)self) = *(s32 *)(p + 0xB0);
                } else {
                    /* highlighted (selected) row */
                    *GuiElementGetColor((GuiWidget *)self) = *(s32 *)(p + 0xAC);
                    if (*(s32 *)(p + 0xC8)) {
                        u32 fill = (*(u32 *)(p + 0xAC) & 0x00FFFFFF) | 0x20000000;
                        u64 packed = ((u64)fill << 32) | fill;
                        s32 w1 = GuiTextElementMeasure((GuiWidget *)self) >> 1;
                        s32 y1 = (s32)(rowY - 2.0f);
                        s32 x1 = (s32)(originX - (f32)w1 - 8.0f);
                        s32 w2 = GuiTextElementMeasure((GuiWidget *)self) >> 1;
                        s32 y2 = (s32)(rowY + 2.0f + 16.0f);
                        s32 x2 = (s32)(originX + (f32)w2 + 9.0f);
                        func_0027F168(x1, y1, x2, y2, 0, (u64)(unsigned long)&packed);
                    }
                }
            }

            GuiTextElementDraw((GuiWidget *)self);
        }
    }

    func_0027F790();
}
#endif

/* func_00349200: build a 6-row list widget — init seven type-B sub-elements
 * (the row container at +0x218 plus six rows at +0x4C..+0x1C8, stride 0x58 is
 * irrelevant here as each is a fixed offset) then run type-C init across the
 * seven 0x58-stride row slots starting at +0x218; returns the widget.
 * WALL: 4 callee saves ($16,$17,$18,$19) — the original packs them into a 0x30
 * frame at 8-byte slot spacing; the pinned cc1 reserves 16-byte save slots. NOT
 * oracle-seedable: its GuiElementInitTypeB/C callees store &D_1ADA38/&D_1AD9F8
 * (absolute-address %hi/%lo symbols) which crash the cmp runner's ld
 * --gc-sections — the same absolute-address-symbol wall as GuiQuitDialogInit. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349200);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 81.67% / engine96 76.00%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 17/36 words differ;
 * frozen-.s census: 5 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE (5 callee GPR saves) — 4 of the 17 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
GuiWidget *func_00349200(GuiWidget *w) {
    char *row;
    s32 i;

    GuiElementInitTypeB((char *)w + 0x218);
    GuiElementInitTypeB((char *)w + 0x4C);
    GuiElementInitTypeB((char *)w + 0x98);
    GuiElementInitTypeB((char *)w + 0xE4);
    GuiElementInitTypeB((char *)w + 0x130);
    GuiElementInitTypeB((char *)w + 0x17C);
    GuiElementInitTypeB((char *)w + 0x1C8);

    row = (char *)w + 0x218;
    for (i = 6; i >= 0; i--) {
        GuiElementInitTypeC(row);
        row += 0x58;
    }
    return w;
}
#endif

/* func_003492A0: handwritten epilogue-only stump (`addiu $sp,$sp,0x30; nop`,
 * no prologue, no `jr ra`) — the trailing half of a hand-split asm routine.
 * No C body can reproduce a function with no return. WALL: split-artifact stub. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/248B50", func_003492A0);

/* SetPopupVisible: store the visibility flag at popup +0x4B8. */
void SetPopupVisible(GuiWidget *w, s32 visible) {
    *(s32 *)((char *)w + 0x4B8) = visible;
}

/* SetPopupLayoutMode: store the layout mode at popup +0x4BC. */
void SetPopupLayoutMode(GuiWidget *w, s32 mode) {
    *(s32 *)((char *)w + 0x4BC) = mode;
}

/* SetPopupTitleText: bind the five glyph slots of a popup's title/border decor
 * to glyph IDs taken from the source record `src` — title (+0x0) onto the popup
 * body, then four border pieces (src+0x8/+0x10/+0x20/+0x18) onto the four corner
 * sub-elements (+0x130/+0x4C/+0x98/+0xE4). Each glyph is looked up in the GUI
 * instance's atlas (g_guiInstance + 0x8710).
 * WALL: 3 callee saves ($16,$17,$18) — 0x20-vs-packed frame divergence. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", SetPopupTitleText);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 38.14% / engine96 48.98%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 32/34 words differ;
 * frozen-.s census: 4 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE (4 callee GPR saves) — 4 of the 32 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
void SetPopupTitleText(GuiWidget *w, s32 *src) {
    u8 *atlas = g_guiInstance + 0x8710;
    GuiElementSetGlyph(w, atlas, src[0]);            /* src+0x0  */
    GuiElementSetGlyph((char *)w + 0x130, atlas, src[2]); /* src+0x8  */
    GuiElementSetGlyph((char *)w + 0x4C, atlas, src[4]);  /* src+0x10 */
    GuiElementSetGlyph((char *)w + 0x98, atlas, src[8]);  /* src+0x20 */
    GuiElementSetGlyph((char *)w + 0xE4, atlas, src[6]);  /* src+0x18 */
}
#endif

/* SetPopupItemEnabled: store the enabled flag for popup item idx at +0x488
 * (s32-stride item table). */
void SetPopupItemEnabled(GuiWidget *w, s32 idx, s32 enabled) {
    *(s32 *)((char *)w + (idx << 2) + 0x488) = enabled;
}

/* SetPopupItemText: record the item count at +0x4B4, then for each of `count`
 * popup rows (row slots at +0x218, stride 0x58) resolve the string id in
 * `ids[i]` via GetLocalizedString and apply it to that row's text element.
 * WALL: 3 callee saves ($16,$17,$18) — 0x20-vs-packed frame divergence. NOT
 * oracle-seedable in the current cmp link: its GetLocalizedString callee collides
 * with the co-linked text/188858 strong body, whose own #else pulls FindTextTableEntry
 * (INCLUDE_ASM) + &D_1A8D28 (absolute) — the ld --gc-sections absolute-symbol wall;
 * a private mock cannot win the multiple-definition. Logic verified by asm trace. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", SetPopupItemText);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 90.08% / engine96 76.00%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 13/26 words differ;
 * frozen-.s census: 4 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE (4 callee GPR saves) + REGALLOC: 9 frame/save-slot words (frame 0x40 vs 0x20, save layout reordered). */
void SetPopupItemText(GuiWidget *w, s32 count, s32 *ids) {
    char *row;
    s32 i;

    *(s32 *)((char *)w + 0x4B4) = count;
    if (count > 0) {
        row = (char *)w + 0x218;
        for (i = count; i != 0; i--) {
            char *text = GetLocalizedString(*ids);
            GuiElementSetText(row, text);
            ids++;
            row += 0x58;
        }
    }
}
#endif
/* GuiScreenWithPlanetNameInit(screen, pool): build the planet-name popup screen.
 * Sets the layout mode, optionally allocs the 0x10-byte config object at +0x4A4
 * (title position 250,190), inits seven header GuiElements from fixed templates
 * (D_1ADBE8/BF0/C00, D_1ADF98/FA0, D_1AE570/578) at their offsets, then a run of
 * seven text rows (w+0x218, stride 0x58) - each text-inited, blanked, unit-scaled,
 * coloured 0x80F0F0F0, and flagged at w+0x488+i*4. Recolours the seven header
 * elements and assigns their glyphs (codes 0x1C-0x20/0x73/0x74 from the shared
 * atlas g_guiInstance+0x8710), then runs UpdatePopupMenu. Matching arm stays asm
 * (0x80 frame, many callee saves - frame-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", GuiScreenWithPlanetNameInit);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 59.04% / engine96 57.26%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 179/194 words differ;
 * frozen-.s census: 9 callee GPR saves, 1 fp saves.
 * Residual: PACKED-SAVE (9 callee GPR saves, 1 fp) — 10 of the 179 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
extern void UpdatePopupMenu(GuiWidget *w, s32 arg);
extern u8 D_1ADBE8[], D_1ADBF0[], D_1ADC00[], D_1ADF98[], D_1ADFA0[];
extern u8 D_1AE570[], D_1AE578[], D_1AE588[];
void GuiScreenWithPlanetNameInit(void *screen, void *pool) {
    char *w = (char *)screen;
    char *cfg;
    void *obj;
    s32 i;

    SetPopupLayoutMode((GuiWidget *)w, 0);
    *(s32 *)(w + 0x4AC) = 0;
    *(void **)(w + 0x4A8) = pool;
    if (pool != 0) {
        obj = GuiPoolAlloc(pool);
        obj = GuiPlacementNew(0x10, obj);
        *(void **)(w + 0x4A4) = obj;
        *(s32 *)((char *)obj + 0x0) = 0;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
    }
    *(s32 *)(w + 0x4B0) = 1;
    cfg = *(char **)(w + 0x4A4);
    *(f32 *)(cfg + 0x0) = 250.0f; /* 0x437A0000 */
    *(f32 *)(cfg + 0x4) = 190.0f; /* 0x433E0000 */

    GuiElementInit((GuiWidget *)(w + 0x0),   D_1ADBE8, pool);
    GuiElementInit((GuiWidget *)(w + 0x130), D_1ADBF0, pool);
    GuiElementInit((GuiWidget *)(w + 0x4C),  D_1ADC00, pool);
    GuiElementInit((GuiWidget *)(w + 0x98),  D_1ADF98, pool);
    GuiElementInit((GuiWidget *)(w + 0xE4),  D_1ADFA0, pool);
    GuiElementInit((GuiWidget *)(w + 0x17C), D_1AE570, pool);
    GuiElementInit((GuiWidget *)(w + 0x1C8), D_1AE578, pool);

    *(s32 *)(w + 0x480) = 0;
    *(s32 *)(w + 0x4B4) = 0;

    for (i = 0; i < 7; i++) {
        char *elem = w + 0x218 + i * 0x58;
        GuiTextElementInit(elem, D_1AE588, pool);
        GuiElementSetText((GuiWidget *)elem, 0);
        GuiElementSetScale((GuiWidget *)elem, 1.0f, 0.0f, 0.0f, 0.0f);
        *(s32 *)(elem + 0x54) = 0;
        *GuiElementGetColor((GuiWidget *)elem) = 0x80F0F0F0;
        *(s32 *)(w + 0x488 + i * 4) = 1;
    }

    *GuiElementGetColor((GuiWidget *)(w + 0x0))   = 0x60442D00;
    *GuiElementGetColor((GuiWidget *)(w + 0x130)) = 0x55F0C070;
    *GuiElementGetColor((GuiWidget *)(w + 0x4C))  = 0x55F0C070;
    *GuiElementGetColor((GuiWidget *)(w + 0x98))  = 0x80FFDE8D;
    *GuiElementGetColor((GuiWidget *)(w + 0xE4))  = 0x70FFFEED;
    *GuiElementGetColor((GuiWidget *)(w + 0x17C)) = 0x60442D00;
    *GuiElementGetColor((GuiWidget *)(w + 0x1C8)) = 0x55F0C070;

    GuiElementSetGlyph((GuiWidget *)(w + 0x0),   g_guiInstance + 0x8710, 0x1C);
    GuiElementSetGlyph((GuiWidget *)(w + 0x130), g_guiInstance + 0x8710, 0x1D);
    GuiElementSetGlyph((GuiWidget *)(w + 0x4C),  g_guiInstance + 0x8710, 0x1E);
    GuiElementSetGlyph((GuiWidget *)(w + 0x98),  g_guiInstance + 0x8710, 0x1F);
    GuiElementSetGlyph((GuiWidget *)(w + 0xE4),  g_guiInstance + 0x8710, 0x20);
    GuiElementSetGlyph((GuiWidget *)(w + 0x17C), g_guiInstance + 0x8710, 0x73);
    GuiElementSetGlyph((GuiWidget *)(w + 0x1C8), g_guiInstance + 0x8710, 0x74);

    UpdatePopupMenu((GuiWidget *)w, 0);
}
#endif

/* func_00349720: handwritten epilogue-only stump (`addiu $sp,$sp,0x30; nop`,
 * no prologue, no `jr ra`) — the trailing half of a hand-split asm routine.
 * No C body can reproduce a function with no return. WALL: split-artifact stub. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349720);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", UpdatePopupMenu);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349C00);

/* func_00349E88: return the widget pointer unchanged (identity accessor). */
GuiWidget *func_00349E88(GuiWidget *w) {
    return w;
}

/* func_00349E90: reset the widget's selection table — clear +0x0 and +0x148,
 * fill the 64-entry s32 array at +0x44..+0x140 with -1, then clear +0x150/+0x154.
 * Best 86.75%: the original drives the fill with a single induction pointer at
 * w+0x50 using negative store offsets and fills the branch delay slot with the
 * pointer increment; the pinned cc1 splits the base into two registers (one for
 * the offset-0 store, one for the negative offsets). WALL: reloaded-ptr CSE /
 * base-register split.
 * Oracle: cmp_func_00349E90 (cmp_248B50.c) — bit-exact on real R5900. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349E90);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 88.81% / engine96 79.31%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 14/18 words differ;
 * frozen-.s census: 0 callee GPR saves, 0 fp saves.
 * Residual: BASE-REGISTER SPLIT (no callee saves): the ROM drives one induction pointer at w+0x50 with negative store offsets (-0xC/-0x8/-0x4/0x0) and fills the bgez delay slot with the increment; cc1 2.9 splits the base into two registers. REFUTED LEVER (task #564): spelling the ROM's shape literally in C (p = w+0x50; p[-3..0]) is WORSE, 88.81% -> 86.75% sdk29, measured against a control differing by nothing else (tools/ee/.t564/src/23_* vs 24_*). */
void func_00349E90(GuiWidget *w) {
    s32 *p;
    s32 i;
    *(s32 *)((char *)w + 0x0) = 0;
    *(s32 *)((char *)w + 0x148) = 0;
    p = (s32 *)((char *)w + 0x44);
    for (i = 15; i >= 0; i--) {
        p[0] = -1;
        p[1] = -1;
        p[2] = -1;
        p[3] = -1;
        p += 4;
    }
    *(s32 *)((char *)w + 0x154) = 0;
    *(s32 *)((char *)w + 0x150) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349ED0);

/* func_0034A1C8: clear the +0x150 field. */
void func_0034A1C8(GuiWidget *w) {
    w->unk150 = 0;
}

/* func_0034A1D0: store a1 to the +0x144 field. */
void func_0034A1D0(GuiWidget *w, s32 v) {
    w->unk144 = v;
}

/*
 * func_0034A1D8 - zero the four 4-word (0x10-byte) records at widget +0x30.
 *
 *   w  the widget
 * Returns w. The ROM's first instruction is `move v0,a0`, i.e. the widget is
 * the return value (its sibling func_0034A210 returns w the same way); the old
 * "base pointer preserved in a fresh register" wall was that return value.
 *
 * Byte-exact on the sdk29 arm (task #894). The counter runs 3 down to -1 and
 * the loop closes `bne i,-1` with the pointer step in the delay slot. Without
 * the empty asm reading `p`, cc1 2.9 schedules the step before the branch and
 * reorg leaves the slot as a `nop`; the asm is a scheduling barrier, which is
 * why the decrement is written before it, right after the first store, where
 * the ROM has it. The asm emits no instruction.
 */
GuiWidget *func_0034A1D8(GuiWidget *w) {
    s32 *p = (s32 *)((char *)w + 0x30);
    s32 i = 3;

    do {
        p[0] = 0;
        i--;
        p[1] = 0;
        p[2] = 0;
        p[3] = 0;
        __asm__ __volatile__("" : : "r"(p));
        p += 4;
    } while (i != -1);
    return w;
}

/* func_0034A210: reset a slider/animation widget to its rest state — clear the
 * value/flag fields (+0x18,+0x28,+0x2C,+0x80), seed the step constant at +0x24
 * (0.005), zero record slot 0 and fill record slot 1 with 1.0 via func_0034A318,
 * clear the +0x4-stride index entry 0 (func_0034A350), mark "2 records" at +0x84,
 * clear +0x20, and set the "armed" flag +0x1C to 1. The +0x18 field is reloaded
 * after being zeroed, so the slot-0 fill writes 0.0.
 * WALL: 2 callee saves ($16,$31) plus a saved $f20 — 0x20-vs-packed frame. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A210);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 96.58% / engine96 72.40%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 16/46 words differ;
 * frozen-.s census: 2 callee GPR saves, 1 fp saves.
 * Residual: PACKED-SAVE (2 callee GPR + 1 fp save) + FP REGNUM: 4 frame/save-slot words, remainder is $f-register numbering (46006346 vs 46006386) — REGNUM-COLORING. */
GuiWidget *func_0034A210(GuiWidget *w) {
    f32 zero;

    w->unk18 = 0.0f;
    w->unk28 = 0;
    w->unk2C = 0;
    w->unk80 = 0;
    zero = w->unk18;                 /* reload: now 0.0 */
    w->unk24 = 0.005f;               /* 0x3BA3D70A */

    func_0034A318(w, 0, zero, zero, zero, zero, zero);
    func_0034A318(w, 1, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    w->unk84 = 2;
    func_0034A350(w, 0, zero, zero);

    w->unk20 = 0;
    w->unk1C = 1;
    return w;
}
#endif

/* func_0034A2C8: handwritten epilogue-only stump (`addiu $sp,$sp,0x10; nop`,
 * no prologue, no `jr ra`). WALL: split-artifact stub. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A2C8);

/* func_0034A2D0: store a1 to the +0x2C field. */
void func_0034A2D0(GuiWidget *w, s32 v) {
    w->unk2C = v;
}

/* func_0034A2D8: the original is a handwritten no-return store fragment
 * (swc1 $f12,0x18(a0); nop - no jr ra), not a real leaf, so a C accessor with
 * its jr-ra epilogue can't reproduce the bytes. WALL: handwritten tail fragment. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A2D8);

/*
 * func_0034A2E0 - arm the +0x18/+0x28 pair: flag +0x28 = 1 and clear +0x18.
 *
 *   w  the widget
 * Returns the float that was at +0x18 before it was cleared. The ROM loads it
 * into $f0 (`lwc1 $f0,0x18(a0)`) and never touches $f0 again, so the old value
 * IS the return value. The callers seen (func_0034DBE0) ignore it. The earlier
 * "dead load" reading came from declaring the function void.
 *
 * Byte-exact on the sdk29 arm (task #894). The ROM is plain source order: the
 * load, `li v0,1`, the +0x28 store, and the +0x18 store in the jr slot. Left
 * alone, cc1 2.9's second scheduler hoists the `li` above the load. The empty
 * volatile asm keeps it below (it emits no instruction), and with the two
 * stores written in this order reorg puts the +0x18 store in the slot, as in
 * the ROM. +0x18 is cleared with an integer 0 store (`sw zero`).
 */
f32 func_0034A2E0(GuiWidget *w) {
    f32 old = *(f32 *)((char *)w + 0x18);

    __asm__ __volatile__("");
    *(s32 *)((char *)w + 0x18) = 0;
    w->unk28 = 1;
    return old;
}

/* func_0034A2F8: same handwritten no-return store fragment as func_0034A2D8
 * (bare swc1 $f1,0x18(a0), no jr ra). WALL: handwritten tail fragment. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A2F8);

/* func_0034A300: store a1 to the +0x80 field. */
void func_0034A300(GuiWidget *w, s32 v) {
    w->unk80 = v;
}

/* func_0034A308: read-modify the +0x28 field — return the old value, store a1. */
s32 func_0034A308(GuiWidget *w, s32 v) {
    s32 old = w->unk28;
    w->unk28 = v;
    return old;
}

/* func_0034A318: write a 4-float record (f13,f14,f15,f16) into the 0x10-stride
 * slot idx at +0x30, and store f12 into the parallel s32-stride slot at +0x70.
 * Best 58%: the original copies the record base into a fresh register before
 * every swc1 (4 daddu copies) and emits the stores in ascending order with the
 * +0x70 store in the jr delay slot; the pinned cc1 folds to one base and stores
 * +0x3C first. WALL: per-store base copy / dual-register slot fill. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A318);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 57.77% / engine96 43.85%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 8/10 words differ;
 * frozen-.s census: 0 callee GPR saves, 0 fp saves.
 * Residual: no callee saves and no frame/save-slot words in the diff — residual is REGALLOC/SCHED on the 8 differing words; not iterated. */
void func_0034A318(GuiWidget *w, s32 idx, f32 a, f32 b, f32 c, f32 d, f32 e) {
    f32 *rec = (f32 *)((char *)w + idx * 0x10);
    rec[0xC] = b;
    rec[0xD] = c;
    rec[0xE] = d;
    rec[0xF] = e;
    *(f32 *)((char *)w + idx * 4 + 0x70) = a;
}
#endif

/* func_0034A350: store the float pair into +0/+0xC of the +0x4-stride index
 * entry idx. Instructions match, but the original copies the entry pointer to a
 * second register and fills the jr slot with the +0 store; the pinned cc1 folds
 * to one register, ascending order. Best 80%. WALL: dual-register + slot fill. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A350);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 79.67% / engine96 81.67%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 5/6 words differ;
 * frozen-.s census: 0 callee GPR saves, 0 fp saves.
 * Residual: no callee saves and no frame/save-slot words in the diff — residual is REGALLOC/SCHED on the 5 differing words; not iterated. */
void func_0034A350(GuiWidget *w, s32 idx, f32 a, f32 b) {
    char *entry = (char *)w + (idx << 2);
    *(f32 *)(entry + 0x0) = a;
    *(f32 *)(entry + 0xC) = b;
}
#endif

/* func_0034A368: store a1 to the +0x84 field. */
void func_0034A368(GuiWidget *w, s32 v) {
    w->unk84 = v;
}

/**
 * func_0034A370: latch the widget's target value and arm its transition once.
 *
 * @param w  GUI widget whose transition/animation block is being armed.
 * @param v  new target value; stored unconditionally at +0x1C.
 *
 * +0x1C is always written. Only while the armed flag +0x28 is still 0 are the
 * start value +0x18 seeded and +0x28 set to 1, so a widget arms exactly once
 * per disarm cycle.
 *
 * NOTE the polarity, and that the two stores to +0x18 have DIFFERENT TYPES:
 * v == 1 stores the INTEGER 0, any other value stores the FLOAT 1.0f
 * (asm: bne $5,1 -> swc1 1.0; fallthrough -> sw $0). The integer arm is
 * therefore written through an explicit s32 lvalue rather than w->unk18.
 *
 * Byte-exact on the sdk29 arm (cc1 2.9-ee-991111, -O2 -G8 -fno-gcse; task
 * #564): unit objdiff 100.00% (objdiff_build.sh + unit_report.sh) and
 * verify_match_unit.sh BYTE IDENTICAL, 16/16 words, 0 relocs. The engine96 arm
 * also reads 100.00%, so no MATCH_ guard is needed and the plain-C form serves
 * both. RETIRED CLAIM: the previous "Best 62%: branch-likely block layout +
 * register-allocation deltas. WALL: branch-likely block layout" note described
 * a body this file does not carry — task #564's screen measures 100.00%.
 */
void func_0034A370(GuiWidget *w, s32 v) {
    w->unk1C = v;
    if (w->unk28 == 0) {
        if (v == 1) {
            *(s32 *)((char *)w + 0x18) = 0;
        } else {
            w->unk18 = 1.0f;
        }
        w->unk28 = 1;
    }
}

/* func_0034A3B0: store a1 to the +0x20 field. */
void func_0034A3B0(GuiWidget *w, s32 v) {
    w->unk20 = v;
}

/* func_0034A3B8: store the float arg to the +0x24 field. */
void func_0034A3B8(GuiWidget *w, f32 v) {
    w->unk24 = v;
}

/* func_0034A3C0: per-frame tick of a keyframe-bracketed GuiWidget transition —
 * the multi-keyframe cousin of func_0034A860. Layout differs from GuiAnim: this
 * one keeps progress at +0x18, the per-frame step at +0x24, the play direction
 * at +0x1C, the active flag at +0x28, the keyframe-time count at +0x84, and the
 * ascending keyframe-time table (f32) at +0x70.
 *
 * No-op while inactive (+0x28 == 0). Otherwise:
 *  1. Advance the eased phase (+0x18). When applyStep is set it adds (forward,
 *     dir +0x1C == 1) or subtracts (reverse) the step (+0x24). Forward clamps the
 *     top to 1.0 and clears active on the frame it saturates; reverse clamps the
 *     bottom to 0.0 likewise. (The asm writes the raw phase in the compare's delay
 *     slot then overwrites it with the clamped value, so only the clamp survives.)
 *  2. Bracket-search the keyframe table for the [lo,hi] pair straddling the phase:
 *     forward walks UP from index 0 advancing while table[hi] <= phase; reverse
 *     walks DOWN from count-1 advancing while phase <= table[lo]. (Each side
 *     short-circuits on the count / monotonicity guards exactly as the asm does,
 *     so a non-monotone or too-short table leaves lo/hi at their seeds.)
 *  3. t = (phase - table[lo]) / (table[hi] - table[lo]); then
 *     ease = GuiHermiteInterp(t, 0, w->unk00, w->unk0C, 1).
 *  4. When the callback object (+0x2C) is set, dispatch the keyframe pair through
 *     the target object's vtable: obj = w->unk80; call
 *     (*vtbl->fn14)(obj + (s16)vtbl->off10, w->unk2C,
 *                    w + (lo<<4) + 0x30, w + (hi<<4) + 0x30, ease).
 *  5. If the phase saturated this frame (active just cleared), run the end-of-
 *     transition dispatch keyed on the mode (+0x20): mode 1 re-latches via
 *     func_0034A370(w, w->unk1C); mode 2 flips the direction (unk1C ^ 1 ? +1 : -1),
 *     stores it, and re-latches; other modes do nothing observable.
 *
 * WALL: the original threads two index registers ($17 hi / $18 lo) through
 * branch-likely bracket updates (bltzl/beql), a min.s/max.s clamp pair and the
 * bc1t/beql delay-slot raw-phase stores; the pinned cc1 reorders the clamp and
 * spills differently. Scalar f32 throughout (no VU0), so the #else is bit-exact. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A3C0);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 55.98% / engine96 37.84%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 176/186 words differ;
 * frozen-.s census: 4 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE (4 callee GPR saves) — 4 of the 176 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
/* keyframe-callback target vtable (obj at w->unk80): the dispatch reads a
 * half-word field offset at +0x10 and the method pointer at +0x14. */
typedef struct GuiKeyframeTargetVtbl {
    /* 0x00 */ u8 pad00[0x10];
    /* 0x10 */ s16 fieldOff;
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ void (*apply)(void *target, s32 *cbObj, f32 *recLo, f32 *recHi, f32 ease);
} GuiKeyframeTargetVtbl;

void func_0034A3C0(GuiWidget *w, s32 applyStep) {
    f32 phase;
    f32 *table;
    s32 count;
    s32 lo;
    s32 hi;
    s32 stillActive;
    f32 t;
    f32 ease;

    if (w->unk28 == 0) {
        return;
    }

    table = (f32 *)((char *)w + 0x70);
    lo = 0;
    hi = 0;

    if (w->unk1C == 1) {
        /* forward */
        phase = w->unk18;
        if (applyStep != 0) {
            phase = phase + w->unk24;
        }
        stillActive = (phase <= 1.0f) ? 1 : 0;
        w->unk18 = phase;            /* raw store (compare delay slot) */
        phase = (phase <= 1.0f) ? phase : 1.0f;   /* min(phase, 1.0) */
        count = w->unk84;
        w->unk28 = stillActive;
        w->unk18 = phase;            /* clamped store (overwrites) */

        if (count > 0 && 1 < count && table[0] <= phase) {
            s32 i = 0;
            lo = 0;
            hi = 1;
            for (;;) {
                i++;
                if (!(i < count)) break;
                if (!((hi + 1) < count)) break;
                if (!(table[hi] <= phase)) break;
                lo = hi;
                hi = hi + 1;
            }
        }
    } else {
        /* reverse */
        phase = w->unk18;
        if (applyStep != 0) {
            phase = phase - w->unk24;
        }
        stillActive = (0.0f <= phase) ? 1 : 0;
        w->unk18 = phase;            /* raw store (compare delay slot) */
        phase = (0.0f <= phase) ? phase : 0.0f;   /* max(phase, 0.0) */
        count = w->unk84;
        w->unk28 = stillActive;
        hi = count - 1;
        lo = count - 1;
        w->unk18 = phase;            /* clamped store (overwrites) */

        if (hi > 0 && (count - 2) >= 0 && phase <= table[count - 1]) {
            s32 i = hi;
            lo = count - 2;
            for (;;) {
                i--;
                if (i <= 0) break;
                if ((lo - 1) < 0) break;
                if (!(phase <= table[lo])) break;
                hi = lo;
                lo = lo - 1;
            }
        }
    }

    /* normalize the phase inside the [table[lo], table[hi]] bracket and ease it.
     * unk00 / +0x0C are the two Hermite control points. */
    t = (phase - table[lo]) / (table[hi] - table[lo]);
    ease = GuiHermiteInterp(t, 0.0f, w->unk00, *(f32 *)((char *)w + 0x0C), 1.0f);

    if (w->unk2C != 0) {
        void *obj = *(void **)((char *)w + 0x80);
        GuiKeyframeTargetVtbl *vtbl = *(GuiKeyframeTargetVtbl **)obj;
        f32 *recLo = (f32 *)((char *)w + (lo << 4) + 0x30);
        f32 *recHi = (f32 *)((char *)w + (hi << 4) + 0x30);
        vtbl->apply((char *)obj + vtbl->fieldOff, (s32 *)w->unk2C, recLo, recHi, ease);
    }

    /* end-of-transition dispatch (only when the phase saturated this frame). */
    if (w->unk28 != 0) {
        return;
    }
    if (w->unk20 == 1) {
        func_0034A370(w, w->unk1C);
    } else if (w->unk20 < 2) {
        /* mode 0 / negative: nothing. */
    } else if (w->unk20 == 2) {
        s32 dir = ((w->unk1C ^ 1) != 0) ? 1 : -1;
        w->unk1C = dir;
        func_0034A370(w, dir);
    } else {
        /* mode > 2: read unk1C, discard. */
    }
}
#endif

/*
 * func_0034A658 - zero a 2 x 12-word block at widget +0x2C (two outer passes,
 * each three inner passes of four words).
 *
 *   w  the widget
 * Returns w (the ROM opens with `move v0,a0`, as func_0034A1D8 does).
 *
 * Byte-exact on the sdk29 arm (task #894). Two things had to be spelled:
 *  - the inner step `p += 4` sits in the `bne` delay slot in the ROM; the empty
 *    asm reading `p` keeps cc1 2.9 from scheduling it before the branch, where
 *    reorg would leave a `nop` (see func_0034A1D8);
 *  - the ROM computes outer-1 into $7 at the top of the pass, reuses $5 for the
 *    inner counter, and copies $7 back (`move $5,$7`) right before the outer
 *    test. That is the decremented count held in its own variable (`left`) and
 *    assigned back to `outer` after the inner loop.
 * Oracle: cmp_func_0034A658 (cmp_248B50.c), bit-exact on real R5900.
 */
GuiWidget *func_0034A658(GuiWidget *w) {
    u32 *p;
    u32 *row;
    s32 outer;
    s32 left;
    s32 inner;

    outer = 1;
    p = (u32 *)((char *)w + 0x2C);
    do {
        left = outer - 1;
        row = p + 0xC;
        inner = 2;
        do {
            p[0] = 0;
            inner--;
            p[1] = 0;
            p[2] = 0;
            p[3] = 0;
            __asm__ __volatile__("" : : "r"(p));
            p += 4;
        } while (inner != -1);
        p = row;
        outer = left;
    } while (outer != -1);
    return w;
}

/* func_0034A6A8: handwritten epilogue-only stump (`addiu $sp,$sp,0x10; nop`,
 * no prologue, no `jr ra`). WALL: split-artifact stub. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A6A8);

/* func_0034A6B0: reset a 2x2 GuiWidget transform/animation record to its
 * neutral pose. It clears the integer header (+0x10 cursor, +0x1C..+0x28 state
 * to 0), seeds +0xC = 1.0, the three scale fields +0x0/+0x4/+0x8 = 1.0, and the
 * per-frame step +0x18 = 0.066668 (0x3D88882F); then it zeroes the four 4-float
 * sub-records via func_0034A7B0 for each (idx1,idx2) in {(0,0),(0,1),(1,0),
 * (1,1)} (all four record vectors written 0).
 *
 * WALL: callee saves ($16,$31) plus a saved $f20 reloaded as the 0.0 constant —
 * the original keeps the just-stored +0x10 zero live in $f20 across all four
 * calls; the pinned cc1 reserves a 0x20 frame and re-materialises 0.0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A6B0);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 69.19% / engine96 73.28%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 45/56 words differ;
 * frozen-.s census: 2 callee GPR saves, 1 fp saves.
 * Residual: PACKED-SAVE (2 callee GPR saves, 1 fp) — 2 of the 45 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
void func_0034A6B0(GuiWidget *w) {
    union { u32 u; f32 f; } step;
    step.u = 0x3D88882Fu;   /* per-frame animation step, exact bits from the .s */

    *(s32 *)((char *)w + 0x10) = 0;     /* cursor (integer 0) */
    w->unk1C = 0;
    w->unk20 = 0;
    w->unk24 = 0.0f;
    w->unk28 = 0;
    *(f32 *)((char *)w + 0xC) = 1.0f;
    w->unk18 = step.f;
    w->unk00 = 1.0f;
    *(f32 *)((char *)w + 0x4) = 1.0f;
    *(f32 *)((char *)w + 0x8) = 1.0f;

    func_0034A7B0(w, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f);
    func_0034A7B0(w, 0, 1, 0.0f, 0.0f, 0.0f, 0.0f);
    func_0034A7B0(w, 1, 0, 0.0f, 0.0f, 0.0f, 0.0f);
    func_0034A7B0(w, 1, 1, 0.0f, 0.0f, 0.0f, 0.0f);
}
#endif

/* func_0034A798: handwritten epilogue-only stump (`addiu $sp,$sp,0x10; nop`,
 * no prologue, no `jr ra`). WALL: split-artifact stub. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A798);

/* func_0034A7A0: store a2 at +0x20 of the +0x4-stride index entry idx. */
void func_0034A7A0(GuiWidget *w, s32 idx, s32 v) {
    *(s32 *)((char *)w + (idx << 2) + 0x20) = v;
}

/* func_0034A7B0: write a 4-float record (f12..f15) at +0x2C of the slot indexed
 * by (idx1<<4) + idx2*0x30 in the widget. Same shape as func_0034A318 — the
 * original copies the base before each swc1 and stores ascending with the last
 * in the jr delay slot; the pinned cc1 folds the base. WALL: per-store base
 * copy / dual-register slot fill. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A7B0);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 64.31% / engine96 0.00%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 9/12 words differ;
 * frozen-.s census: 0 callee GPR saves, 0 fp saves.
 * Residual: no callee saves and no frame/save-slot words in the diff — residual is REGALLOC/SCHED on the 9 differing words; not iterated. */
void func_0034A7B0(GuiWidget *w, s32 idx1, s32 idx2, f32 a, f32 b, f32 c, f32 d) {
    f32 *rec = (f32 *)((char *)w + (idx1 << 4) + idx2 * 0x30 + 0x2C);
    rec[0] = a;
    rec[1] = b;
    rec[2] = c;
    rec[3] = d;
}
#endif

/* func_0034A7E8: store a2 at +0x8C of the +0x4-stride index entry idx. */
void func_0034A7E8(GuiWidget *w, s32 idx, s32 v) {
    *(s32 *)((char *)w + (idx << 2) + 0x8C) = v;
}

/*
 * func_0034A7F8 - arm a GuiAnim transition forward. The mirror of
 * func_0034A820.
 *
 *   a     the transition
 *   flag  0 -> progress (+0x10) = integer 0; nonzero -> progress = 1.0f
 * Then marks it active (+0x1C = 1) and forward-playing (+0x14 = 1). No return.
 *
 * Byte-exact on the sdk29 arm (task #894). The ROM tests with a branch-likely,
 * `beqzl a1` with the integer-0 store in the annulled slot, and falls through
 * to the 1.0f store. cc1 2.9 gives that shape only with the nonzero case
 * written first. With the flag==0 case first it emits bnez/nop/b and a
 * non-annulled slot. The two trailing stores are written dir-then-active,
 * which is what puts +0x1C first and +0x14 in the jr slot, as in the ROM. (The
 * old wall note, "+0x14 first regardless of statement order", held only for
 * the other branch sense.)
 */
void func_0034A7F8(GuiAnim *a, s32 flag) {
    if (flag != 0) {
        a->progress = 1.0f;
    } else {
        *(s32 *)((char *)a + 0x10) = 0;
    }
    a->dir = 1;
    a->active = 1;
}

/*
 * func_0034A820 - arm a GuiAnim-style transition backward. The mirror of
 * func_0034A7F8.
 *
 *   w     the widget holding the transition at +0x10..+0x1C
 *   flag  0 -> +0x10 = 1.0f; nonzero -> +0x10 = integer 0
 * Then +0x1C = 1 (active) and +0x14 = -1 (reverse). No return.
 *
 * Byte-exact on the sdk29 arm (task #894). The ROM branches `beqz a1` to the
 * 1.0f store and runs the integer-0 store in the delay slot of the fall-through
 * `b`. cc1 2.9 gives that layout with the nonzero case written first; with the
 * flag==0 case first it emits `bnezl`. As in func_0034A7F8, the trailing
 * stores are written in the reverse of the ROM's issue order.
 */
void func_0034A820(GuiWidget *w, s32 flag) {
    if (flag != 0) {
        *(s32 *)((char *)w + 0x10) = 0;
    } else {
        *(f32 *)((char *)w + 0x10) = 1.0f;
    }
    *(s32 *)((char *)w + 0x14) = -1;
    *(s32 *)((char *)w + 0x1C) = 1;
}

/* func_0034A858: store the float arg to the +0x18 field. */
void func_0034A858(GuiWidget *w, f32 v) {
    w->unk18 = v;
}

/* func_0034A860: per-frame tick of a GuiAnim transition. No-op while inactive
 * (+0x1C == 0). Otherwise it advances the eased phase, blends two keyframe
 * 4-vectors, and (optionally) blends a colour word:
 *
 *  1. Advance the phase by the per-frame step (+0x18): forward (dir +0x14 == 1)
 *     adds and clamps the top to 1.0; reverse adds nothing — it subtracts and
 *     clamps the bottom to 0.0. The active flag (+0x1C) stays set only while the
 *     phase is still inside [0,1]; it clears on the frame the phase saturates.
 *     (The asm writes the un-clamped phase then immediately overwrites it with
 *     the clamped value, so only the clamped phase survives.)
 *  2. t = GuiHermiteInterp(phase, 0, c0, c1, 1) — the eased blend factor.
 *  3. For each of the two keyframe channels i (dst ptr at +0x24/+0x28): when the
 *     dst pointer is non-null, lerp the 4-vector  dst[k] = (1-t)*from[k] + t*to[k]
 *     where  from = anim + 0x3C + i*0x10  and  to = anim + 0x6C + i*0x10.
 *  4. If the colour sink (+0x20) is non-null, blend the two packed colour words
 *     (+0x8C,+0x90) by a SECOND Hermite ease (over c2,c3) via ColorLerpPacked and
 *     store the result through the sink.
 *
 * WALL: 99.x near-miss — the original keeps the saved $f20 (1.0) live across the
 * whole body and fills both bc1t/beql delay slots with the dead un-clamped store
 * / pointer advance; the pinned cc1 reloads 1.0 and reorders the clamp stores.
 * Scalar f32 throughout (no VU0) so the #else is bit-exact. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A860);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 81.26% / engine96 59.26%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 89/104 words differ;
 * frozen-.s census: 2 callee GPR saves, 1 fp saves.
 * Residual: PACKED-SAVE (2 callee GPR saves, 1 fp) — 3 of the 89 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
void func_0034A860(GuiAnim *a) {
    f32 phase;
    f32 clamped;
    f32 t;
    s32 stillActive;
    s32 i;

    if (a->active == 0) {
        return;
    }

    if (a->dir == 1) {
        phase = a->progress + a->step;
        stillActive = (phase <= 1.0f) ? 1 : 0;
        clamped = (phase <= 1.0f) ? phase : 1.0f;   /* min(phase, 1.0) */
    } else {
        phase = a->progress - a->step;
        stillActive = (0.0f <= phase) ? 1 : 0;
        clamped = (0.0f <= phase) ? phase : 0.0f;   /* max(phase, 0.0) */
    }
    a->active = stillActive;
    a->progress = clamped;

    t = GuiHermiteInterp(clamped, 0.0f, a->c0, a->c1, 1.0f);

    for (i = 0; i < 2; i++) {
        f32 *dst = *(f32 **)((char *)a + 0x24 + i * 4);
        if (dst != 0) {
            f32 *from = (f32 *)((char *)a + 0x3C + i * 0x10);
            f32 *to   = (f32 *)((char *)a + 0x6C + i * 0x10);
            f32 coF = 1.0f - t;
            dst[0] = coF * from[0] + t * to[0];
            dst[1] = coF * from[1] + t * to[1];
            dst[2] = coF * from[2] + t * to[2];
            dst[3] = coF * from[3] + t * to[3];
        }
    }

    if (a->sink != 0) {
        f32 ct = GuiHermiteInterp(clamped, 0.0f, a->c2, a->c3, 1.0f);
        *a->sink = (s32)ColorLerpPacked(a->colorA, a->colorB, ct);
    }
}
#endif

/* func_0034A9F8: build a list-row widget — init three type-B sub-elements
 * (+0x10/+0x5C/+0xA8) and a list-row element (+0xF4) via GuiListRowElementInit,
 * clear the +0x140 record (func_0034A1D8), then install two vtable pointers and
 * run func_003368D0 over the +0x1CC/+0x1D0 slots: store &D_1AD8E8 at +0x1CC and
 * &D_1AD908 at +0x1D0, finally clear four more records (+0x1D4/+0x25C/+0x2E4/
 * +0x36C via func_0034A1D8). Returns the widget.
 * WALL (matching arm): byte-match — 2 callee saves ($16,$31) packed by the
 * original into a 0x10 frame; the pinned cc1 reserves a 0x20 frame. NOT
 * oracle-seedable: the two &D_1AD8E8 / &D_1AD908 stores are absolute-address
 * %hi/%lo symbols that crash the cmp runner's ld --gc-sections (the
 * absolute-address-symbol wall). Matching arm stays asm; #else is the functional
 * model (GuiElementInitTypeB/GuiListRowElementInit/func_003368D0 left implicit
 * per the unit convention, line 104). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A9F8);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 98.54% / engine96 75.62%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 8/38 words differ;
 * frozen-.s census: 2 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE (2 callee GPR saves) + operand ORDER: 4 frame/save-slot words, plus the &D_1AD8E8/&D_1AD908 addiu pairs emitted in the opposite order from the ROM (SCHED). */
extern u8 D_1AD8E8[]; /* list-row vtable installed at widget +0x1CC */
extern u8 D_1AD908[]; /* list-row vtable installed at widget +0x1D0 */
void *func_0034A9F8(void *widget) {
    char *w = (char *)widget;

    GuiElementInitTypeB(w + 0x10);
    GuiElementInitTypeB(w + 0x5C);
    GuiElementInitTypeB(w + 0xA8);
    GuiListRowElementInit(w + 0xF4);
    func_0034A1D8((GuiWidget *)(w + 0x140));
    func_003368D0(w + 0x1CC);
    *(u8 **)(w + 0x1CC) = D_1AD8E8;
    func_003368D0(w + 0x1D0);
    *(u8 **)(w + 0x1D0) = D_1AD908;
    func_0034A1D8((GuiWidget *)(w + 0x1D4));
    func_0034A1D8((GuiWidget *)(w + 0x25C));
    func_0034A1D8((GuiWidget *)(w + 0x2E4));
    func_0034A1D8((GuiWidget *)(w + 0x36C));
    return widget;
}
#endif

/* func_0034AA90: handwritten epilogue-only stump (`addiu $sp,$sp,0x20; nop`,
 * no prologue, no `jr ra`). WALL: split-artifact stub. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034AA90);

/* func_0034AA98: large GUI screen/menu construction routine (0x70 frame, 8+
 * callee saves). WALL: many callee saves — frame-layout divergence. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034AA98);

/* func_0034B1E0: handwritten epilogue-only stump (`addiu $sp,$sp,0x20; nop`,
 * no prologue, no `jr ra`). WALL: split-artifact stub. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B1E0);

/* func_0034B1E8: store a1 to the +0x0 field. */
void func_0034B1E8(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0x0) = v;
}

/* g_bPlayerMode seen by cc1 as a 16-byte object. At -G8 cc1 then addresses it
 * with a split %hi/%lo pair of its own instead of %gp_rel, and the scheduler
 * can place the %hi half in a branch delay slot, as the ROM does in
 * func_0034B1F0. The native build reads the scalar directly. */
#ifndef TARGET_NATIVE
extern u8 g_bPlayerModeAbs[16] __asm__("g_bPlayerMode");
#else
#define g_bPlayerModeAbs (&g_bPlayerMode)
#endif

/*
 * func_0034B1F0 - set a widget's +0x4 target value, arming a transition when
 * it changes.
 *
 *   w  the widget
 *   v  the new target value
 * When v differs from the current +0x4 value and the player is in normal
 * (Ratchet) mode (g_bPlayerMode == 0), the previous value is latched at +0x3FC
 * and the transition timers are armed (+0x3F8 = 180, +0x3F4 = 300). +0x4 = v
 * either way. No return.
 *
 * Byte-exact on the sdk29 arm (task #894; was 90.00% solo). The ROM splits the
 * g_bPlayerMode load across the `beq`: `lui v0,%hi` in the delay slot, `lbu
 * v1,%lo(v0)` after it. Through the gp-small declaration cc1 2.9 emits one
 * `%gp_rel` lbu instead. The assembler-only `.extern g_bPlayerMode, 16`
 * (#564) was worse, 74.17%, because cc1 still emits one macro insn. Only the
 * C-level 16-byte view (g_bPlayerModeAbs) makes cc1 emit the split pair.
 */
void func_0034B1F0(GuiWidget *w, s32 v) {
    s32 old = *(s32 *)((char *)w + 0x4);

    if (old != v && g_bPlayerModeAbs[0] == 0) {
        *(s32 *)((char *)w + 0x3FC) = old;
        *(s32 *)((char *)w + 0x3F8) = 0xB4;
        *(s32 *)((char *)w + 0x3F4) = 0x12C;
    }
    *(s32 *)((char *)w + 0x4) = v;
}

/* func_0034B220: GUI value/gauge update routine reading the D_1AE5E8/D_1AE5F8
 * tables (0x80 frame, 5 GPR + 2 fp callee saves). WALL: many callee saves —
 * frame-layout divergence. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B220);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B548);
/* func_0034B770: refresh the ammo/nanotech HUD list sub-element (w+0xF4). Hidden
 * while g_maxHealth >= 0x50 (full/no-list state); otherwise shown and populated:
 * item count = soundBank[0x252C]-[0x2530] and scroll = (g_nanotech>>5)-[0x2530]
 * (func_0026F818 is a no-op stub in this build, so its out-params leave those
 * unchanged), the +0x140 vec is positioned from the source object (w+0x8) with a
 * biased/rounded y (index-flag at g_swapGadgetItemIndex+0x86 subtracts 5 before
 * truncation), and the row colour pairs are chosen by the soundBank[0x22B4] flag.
 * Matching arm stays asm (many callee-saves, frame-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B770);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 72.40% / engine96 59.83%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 105/112 words differ;
 * frozen-.s census: 5 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE (5 callee GPR saves) — 4 of the 105 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
extern s32 g_maxHealth;
extern s32 g_nanotech;
extern u8 g_soundBankHandlesBlk[];
extern u8 g_swapGadgetItemIndex[];
extern f32 D_1AE750, D_1AE754;
extern void func_0026F818(void *a, void *b, void *c); /* no-op stub in this build */
extern void GuiElementSetVisible(GuiWidget *e, s32 visible);
extern void GuiListSetItemCount(GuiWidget *e, s32 count);
extern void GuiListSetScrollPos(GuiWidget *e, s32 pos);
extern void GuiListSetColorPair0(GuiWidget *e, s32 c0, s32 c1);
extern void GuiListSetColorPair1(GuiWidget *e, s32 c0, s32 c1);
void func_0034B770(GuiWidget *w) {
    char *b = (char *)w;
    GuiWidget *list = (GuiWidget *)(b + 0xF4);
    f32 *vec;
    f32 *obj;
    s32 t0, t1, t2;
    f32 f3, base;
    s32 indexFlag, iv;

    vec = func_00336C18(list);
    if (g_maxHealth >= 0x50) {
        GuiElementSetVisible(list, 0);
        return;
    }
    GuiElementSetVisible(list, 1);

    t2 = 0;
    t1 = *(s32 *)(g_soundBankHandlesBlk + 0x252C);
    t0 = *(s32 *)(g_soundBankHandlesBlk + 0x2530);
    if (t0 == 0) {
        func_0026F818(&t0, &t1, &t2);
    }
    GuiListSetItemCount(list, t1 - t0);
    GuiListSetScrollPos(list, (g_nanotech >> 5) - t0);

    obj = *(f32 **)(b + 0x8);
    vec[0] = D_1AE750 + obj[0];
    f3 = *(f32 *)(g_swapGadgetItemIndex + 0x8A);
    indexFlag = *(s32 *)(g_swapGadgetItemIndex + 0x86);
    obj = *(f32 **)(b + 0x8);
    base = (D_1AE754 + obj[1]) * f3 + 0.5f;
    if (indexFlag == 1) {
        iv = (s32)(base - 5.0f);
    } else {
        iv = (s32)base;
    }
    vec[1] = (f32)iv;

    if (g_soundBankHandlesBlk[0x22B4] == 1) {
        GuiListSetColorPair0(list, 0x60808080, 0x60808080);
        GuiListSetColorPair1(list, 0x40808080, 0x40808080);
    } else {
        GuiListSetColorPair0(list, 0x8049C1FF, 0x80001EFF);
        GuiListSetColorPair1(list, 0x50F0C070, 0x50F0C070);
    }
}
#endif
/* func_0034B950: per-frame update for a reveal/dismiss screen. Repositions two
 * layout rects (scratch vec2s from func_00336C18 at the +0x5C and +0xA8 sub-
 * elements) to the source object's (w+0x8) x/y plus fixed offsets
 * (D_1AE740/744 for the first, D_1AE748/74C for the second), runs the row-layout
 * update (func_0034B770) and the +0x140 element step (func_0034A3C0). Then a
 * dismiss countdown at +0x3F4: while non-zero, decrement; on reaching zero it
 * clears +0xC/+0x3F8, arms +0x400, and retracts the four sub-panels
 * (func_0034A370(-1) at +0x1D4/+0x25C/+0x2E4/+0x36C). Matching arm stays asm
 * (frame-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B950);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 69.91% / engine96 74.02%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 55/60 words differ;
 * frozen-.s census: 2 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE (2 callee GPR saves) — 2 of the 55 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
extern void func_0034B770(GuiWidget *w);
extern f32 D_1AE740, D_1AE744, D_1AE748, D_1AE74C;
void func_0034B950(GuiWidget *w) {
    char *b = (char *)w;
    f32 *obj;
    f32 *r;
    s32 counter;

    r = func_00336C18((GuiWidget *)(b + 0x5C));
    obj = *(f32 **)(b + 0x8);
    r[0] = D_1AE740 + obj[0];
    r[1] = D_1AE744 + obj[1];

    r = func_00336C18((GuiWidget *)(b + 0xA8));
    obj = *(f32 **)(b + 0x8);
    r[0] = D_1AE748 + obj[0];
    r[1] = D_1AE74C + obj[1];

    func_0034B770(w);
    func_0034A3C0((GuiWidget *)(b + 0x140), 1);

    counter = *(s32 *)(b + 0x3F4);
    if (counter != 0) {
        counter--;
        *(s32 *)(b + 0x3F4) = counter;
        if (counter == 0) {
            *(s32 *)(b + 0xC) = 0;
            *(s32 *)(b + 0x3F8) = 0;
            *(s32 *)(b + 0x400) = 1;
            func_0034A370((GuiWidget *)(b + 0x1D4), -1);
            func_0034A370((GuiWidget *)(b + 0x25C), -1);
            func_0034A370((GuiWidget *)(b + 0x2E4), -1);
            func_0034A370((GuiWidget *)(b + 0x36C), -1);
        }
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BA50);

/* func_0034BD28: set the widget's extent/size vector.
 *   w - the widget; x, y - the new extent in 2D screen units.
 * Returns nothing. Writes x,y into [0],[1] of the vec4 the widget points at
 * from +0x8 (the embedded GuiElement's third transform vec), leaving [2],[3]
 * untouched -- unlike GuiMenuListSetOrigin, which zeroes them.
 * Non-obvious: the pointer is RE-READ from +0x8 between the two stores; see
 * GuiMenuListSetOrigin for why that is load-bearing.
 * t562 @e3f50d43: the former "WALL: just-in-time reload register alternation"
 * (dated to before 2026-09-21) is FALSE. -> UNION-RELOAD, 100.00% (unit objdiff
 * report, objdiff_build.sh, clean), verify_match_unit.sh BYTE IDENTICAL 6/6. */
void func_0034BD28(GuiWidget *w, f32 x, f32 y) {
    w->extentVec.p[0] = x;
    w->extentVec.p[1] = y;
}

/* GuiScreenSetEventAndReveal: stash the pending event id at +0x3F4. If the screen
 * is currently armed-for-reveal (+0x400 set), consume that flag (+0x400=0), raise
 * the "dirty/redraw" flag (+0xC=1), and trigger the four corner reveal animations
 * (sub-widgets at +0x1D4/+0x25C/+0x2E4/+0x36C) forward via func_0034A370(.,1).
 * WALL (cc1 2.9 and 2.96): 2 callee saves ($16,$31) — 0x10-frame-vs-packed divergence. */
/* GUARD (task #1271): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiScreenSetEventAndReveal)
S136OS_SLOT(GuiScreenSetEventAndReveal);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 69.58% / engine96 78.04%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 24/30 words differ;
 * frozen-.s census: 2 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE (2 callee GPR saves) — 4 of the 24 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
void GuiScreenSetEventAndReveal(GuiWidget *w, s32 event) {
    *(s32 *)((char *)w + 0x3F4) = event;
    if (*(s32 *)((char *)w + 0x400) != 0) {
        *(s32 *)((char *)w + 0x400) = 0;
        *(s32 *)((char *)w + 0xC) = 1;
        func_0034A370((GuiWidget *)((char *)w + 0x1D4), 1);
        func_0034A370((GuiWidget *)((char *)w + 0x25C), 1);
        func_0034A370((GuiWidget *)((char *)w + 0x2E4), 1);
        func_0034A370((GuiWidget *)((char *)w + 0x36C), 1);
    }
}
#endif

/* func_0034BDA8: handwritten store fragment (`sw $2,0x3F4($4); nop`, no
 * prologue, no `jr ra`, source value in an undefined $2). WALL: split-artifact
 * stub. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BDA8);

/* func_0034BDB0: large GUI screen construction routine — initialises the screen's
 * full sub-element tree in place and returns it. Three type-B elements + a type-C
 * + a func_003374D8 sub-screen at the head fixed offsets, a second such group,
 * then a run of 8 type-B elements (stride 0x4C from +0x378) and a trailing one at
 * +0x5D8, then 8 func_003374D8 sub-screens (stride 0x3C from +0x624). Finally a
 * fixed batch of list/row inits (func_0034A9F8 / func_0034A1D8 / func_0034A658)
 * across ~20 offsets, installs two vtable pointers (&D_1AD908 at +0x1464,
 * &D_1AD8E8 at +0x1468, each after func_003368D0), and two more func_0034A1D8.
 * Matching arm stays asm (0x90 frame, 10+ callee saves — frame-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BDB0);
#else
/* MEASURED (task #564, 2026-09-21, whole-unit both-arms screen at origin/master e3f50d43,
 * objdiff_build.sh + unit_report.sh; sdk29 = all 31 arms promoted together on cc1
 * 2.9-ee-991111 -O2 -G8 -fno-gcse, engine96 = all 31 arms MATCH_-guarded together on
 * cc1 2.96-ee-001003-1): sdk29 51.70% / engine96 44.13%.
 * RAW (verify_match_unit.sh vs the ROM, rc=1 DIFFERS): 100/101 words differ;
 * frozen-.s census: 9 callee GPR saves, 0 fp saves.
 * Residual: PACKED-SAVE (9 callee GPR saves) — 6 of the 100 differing words are frame/save-slot; remainder REGALLOC/SCHED, not iterated. */
extern u8 D_1AD908[]; /* vtable installed at +0x1464 */
extern u8 D_1AD8E8[]; /* vtable installed at +0x1468 */
void *func_0034BDB0(void *widget) {
    char *w = (char *)widget;
    s32 i;

    GuiElementInitTypeB(w + 0x0);
    GuiElementInitTypeB(w + 0x4C);
    GuiElementInitTypeB(w + 0x98);
    GuiElementInitTypeC(w + 0x100);
    func_003374D8((GuiWidget *)(w + 0x158));
    GuiElementInitTypeB(w + 0x194);
    GuiElementInitTypeB(w + 0x1E0);
    GuiElementInitTypeC(w + 0x248);
    func_003374D8((GuiWidget *)(w + 0x2A0));
    GuiElementInitTypeB(w + 0x2E0);
    GuiElementInitTypeB(w + 0x32C);

    for (i = 0; i < 8; i++) {
        GuiElementInitTypeB(w + 0x378 + i * 0x4C);
    }
    GuiElementInitTypeB(w + 0x5D8);

    for (i = 0; i < 8; i++) {
        func_003374D8((GuiWidget *)(w + 0x624 + i * 0x3C));
    }

    func_0034A9F8(w + 0x808);
    func_0034A1D8((GuiWidget *)(w + 0xC0C));
    func_0034A1D8((GuiWidget *)(w + 0xC94));
    func_0034A1D8((GuiWidget *)(w + 0xD1C));
    func_0034A1D8((GuiWidget *)(w + 0xDA4));
    func_0034A1D8((GuiWidget *)(w + 0xE2C));
    func_0034A658((GuiWidget *)(w + 0xEB4));
    func_0034A658((GuiWidget *)(w + 0xF3C));
    func_0034A658((GuiWidget *)(w + 0xFD0));
    func_0034A1D8((GuiWidget *)(w + 0x1064));
    func_0034A658((GuiWidget *)(w + 0x10F8));
    func_0034A658((GuiWidget *)(w + 0x1180));
    func_0034A658((GuiWidget *)(w + 0x1214));
    func_0034A658((GuiWidget *)(w + 0x12A8));
    func_0034A658((GuiWidget *)(w + 0x133C));
    func_0034A658((GuiWidget *)(w + 0x13D0));
    func_003368D0(w + 0x1464);
    *(u8 **)(w + 0x1464) = D_1AD908;
    func_003368D0(w + 0x1468);
    *(u8 **)(w + 0x1468) = D_1AD8E8;
    func_0034A1D8((GuiWidget *)(w + 0x146C));
    func_0034A1D8((GuiWidget *)(w + 0x14F4));

    return widget;
}
#endif

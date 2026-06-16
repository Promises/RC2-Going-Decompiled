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
    /* 0x04 */ u8 pad04[0x14];
    /* 0x18 */ f32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ u8 pad30[0x50];
    /* 0x80 */ s32 unk80;
    /* 0x84 */ s32 unk84;
    /* 0x88 */ u8 pad88[0xB8];
    /* 0x140 */ s32 unk140;
    /* 0x144 */ s32 unk144;
    /* 0x148 */ u8 pad148[8];
    /* 0x150 */ s32 unk150;
} GuiWidget;

/* player character/control mode (0x18C0D4): 0 Ratchet, 1 Clank-solo, 2 Giant Clank. */
extern u8 g_bPlayerMode;

/* func_00348BD0: run the type-C element init on the widget and return it.
 * Best 99.6%: the original packs the two callee saves ($16,$31) into a 0x10
 * frame (8-byte slots); the pinned cc1 reserves a 0x20 frame (16-byte slots).
 * WALL: 0x20-vs-0x10 frame for 2 callee-saves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348BD0);

/* func_00348BF8: build/init a GUI text element — alloc its backing object via
 * GuiPoolAlloc + GuiPlacementNew, run GuiTextElementInit, then seed the colour/
 * style fields (+0xAC..+0xCC). WALL: 3 callee saves ($16,$17,$31) — the pinned
 * cc1's 0x20 frame with 16-byte save slots diverges from the original's packed
 * 0x10/8-byte layout. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348BF8);

/* func_00348CB8: page/selection-advance logic gated on the input mask bits
 * (0x1000/0x4000/0x40); plays a sound and walks the +0x6C entry table.
 * WALL: 4 callee saves ($16,$17,$18,$31) — frame-layout divergence. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348CB8);

/* func_00348D98: handwritten epilogue-only stump (`addiu $sp,$sp,0x10; nop`,
 * no prologue, no `jr ra`) — the trailing half of a hand-split asm routine.
 * No C body can reproduce a function with no return. WALL: split-artifact stub. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348D98);

/* func_00348DA0: store the keyframe table pointer at +0x68, then scan it to
 * count how many leading entries (stride 0x14, capped at 80) have a positive
 * first float; the count lands in +0xC0. Finally, if the current cursor +0x60
 * has run past the new count, reset it to 0.
 * Best 47%: the original emits the two prologue stores (+0x68,+0xC0) first and
 * computes the loop-end pointer in the bc1f delay slot, then keeps the float
 * compare result live across the loop; the pinned cc1 hoists the float setup
 * above the stores and re-shapes the loop entry with an extra branch. WALL:
 * instruction scheduling + branch-likely loop layout. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348DA0);
#else
void func_00348DA0(GuiWidget *w, f32 *table) {
    f32 *end;
    s32 count;

    *(f32 **)((char *)w + 0x68) = table;
    *(s32 *)((char *)w + 0xC0) = 0;
    end = table + 0x50;
    if (0.0f < *table) {
        count = *(s32 *)((char *)w + 0xC0);
        while (1) {
            table += 5;
            *(s32 *)((char *)w + 0xC0) = count + 1;
            if (*table <= 0.0f || (s32)end <= (s32)table) {
                break;
            }
            count = *(s32 *)((char *)w + 0xC0);
        }
    }
    if (*(s32 *)((char *)w + 0xC0) <= *(s32 *)((char *)w + 0x60)) {
        *(s32 *)((char *)w + 0x60) = 0;
    }
}
#endif

/* func_00348E10: set the +0xB8 / +0xBC field pair. Best 96%: the original
 * stores +0xBC first then fills the jr delay slot with the +0xB8 store; the
 * pinned cc1 schedules the two independent struct stores in ascending-offset
 * order. WALL: ascending-offset store scheduling. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348E10);

/* func_00348E20: store a1 to the +0xC8 field. */
void func_00348E20(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0xC8) = v;
}

/* func_00348E28: write the two float args into the block at *(w+0x5C) (+0/+4)
 * and zero +8/+0xC; the +0x5C pointer is re-read per store. Best 37%: the
 * original alternates two scratch registers reloaded just-in-time; the pinned
 * cc1 hoists the volatile reloads and reuses one register. WALL: just-in-time
 * reload register alternation. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348E28);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348E70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349200);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_003492A0);

/* SetPopupVisible: store the visibility flag at popup +0x4B8. */
void SetPopupVisible(GuiWidget *w, s32 visible) {
    *(s32 *)((char *)w + 0x4B8) = visible;
}

/* SetPopupLayoutMode: store the layout mode at popup +0x4BC. */
void SetPopupLayoutMode(GuiWidget *w, s32 mode) {
    *(s32 *)((char *)w + 0x4BC) = mode;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", SetPopupTitleText);

/* SetPopupItemEnabled: store the enabled flag for popup item idx at +0x488
 * (s32-stride item table). */
void SetPopupItemEnabled(GuiWidget *w, s32 idx, s32 enabled) {
    *(s32 *)((char *)w + (idx << 2) + 0x488) = enabled;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", SetPopupItemText);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", GuiScreenWithPlanetNameInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349720);

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
 * base-register split. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349E90);
#else
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

/* func_0034A1D8: zero four 4-word (0x10-byte) records starting at widget +0x30
 * (counter 3 down to -1). Instructions match, but the original preserves the
 * base pointer in a fresh register (move v0,a0) while the pinned cc1 reuses a0
 * as the cursor. Best 67%. WALL: base-pointer preservation / register alloc. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A1D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A210);

/* func_0034A2C8: handwritten epilogue-only stump (`addiu $sp,$sp,0x10; nop`,
 * no prologue, no `jr ra`). WALL: split-artifact stub. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A2C8);

/* func_0034A2D0: store a1 to the +0x2C field. */
void func_0034A2D0(GuiWidget *w, s32 v) {
    w->unk2C = v;
}

/* func_0034A2D8: the original is a handwritten no-return store fragment
 * (swc1 $f12,0x18(a0); nop - no jr ra), not a real leaf, so a C accessor with
 * its jr-ra epilogue can't reproduce the bytes. WALL: handwritten tail fragment. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A2D8);

/* func_0034A2E0: arm the +0x18/+0x28 pair — flag +0x28 = 1 and clear +0x18.
 * The original emits a dead lwc1 +0x18 before the li 1; under -fno-gcse the
 * pinned cc1 swaps that order. Best 60%. WALL: dead-load vs immediate
 * scheduling order. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A2E0);

/* func_0034A2F8: same handwritten no-return store fragment as func_0034A2D8
 * (bare swc1 $f1,0x18(a0), no jr ra). WALL: handwritten tail fragment. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A2F8);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A350);

/* func_0034A368: store a1 to the +0x84 field. */
void func_0034A368(GuiWidget *w, s32 v) {
    w->unk84 = v;
}

/* func_0034A370: latch +0x1C to a1 once (while +0x28 is 0), set +0x18 to 1.0
 * when a1==1 else 0.0, then +0x28 = 1. Best 62%: branch-likely block layout +
 * register-allocation deltas. WALL: branch-likely block layout. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A370);

/* func_0034A3B0: store a1 to the +0x20 field. */
void func_0034A3B0(GuiWidget *w, s32 v) {
    w->unk20 = v;
}

/* func_0034A3B8: store the float arg to the +0x24 field. */
void func_0034A3B8(GuiWidget *w, f32 v) {
    w->unk24 = v;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A3C0);

/* func_0034A658: zero a 2x12-word block at widget +0x2C (two outer passes, each
 * three inner passes of four words).
 * Best 72%: the original preserves the widget base in a fresh register and fills
 * both bne delay slots with the pointer advances; the pinned cc1 folds the base
 * into the cursor and emits the advances before the branches. WALL: base
 * preservation + delay-slot fill. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A658);
#else
void func_0034A658(GuiWidget *w) {
    u32 *p;
    u32 *row;
    s32 outer;
    s32 inner;

    outer = 1;
    p = (u32 *)((char *)w + 0x2C);
    do {
        outer--;
        row = p + 0xC;
        inner = 2;
        do {
            p[0] = 0;
            inner--;
            p[1] = 0;
            p[2] = 0;
            p[3] = 0;
            p += 4;
        } while (inner != -1);
        p = row;
    } while (outer != -1);
}
#endif

/* func_0034A6A8: handwritten epilogue-only stump (`addiu $sp,$sp,0x10; nop`,
 * no prologue, no `jr ra`). WALL: split-artifact stub. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A6A8);

/* func_0034A6B0: initialise a transform/animation record (+0x0..+0x28) and call
 * func_0034A7B0 four times to seed its float sub-records. WALL: callee saves
 * ($16,$31) plus a saved $f20 — frame-layout divergence (0x20 vs packed). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A6B0);

/* func_0034A798: handwritten epilogue-only stump (`addiu $sp,$sp,0x10; nop`,
 * no prologue, no `jr ra`). WALL: split-artifact stub. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A798);

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

/* func_0034A7F8: set +0x10 enable float (1.0 when flag set else 0.0), then
 * +0x1C = 1 and +0x14 = 1. Best 99.8%: every instruction matches; the original
 * fills the jr delay slot with the +0x14 store. WALL: trailing-store slot fill. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A7F8);

/* func_0034A820: set +0x10 enable float (1.0 when flag clear else 0.0), then
 * +0x1C = 1 and +0x14 = -1. Best 80%: same trailing-store slot fill plus a
 * constant-load schedule delta. WALL: trailing-store delay-slot fill. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A820);

/* func_0034A858: store the float arg to the +0x18 field. */
void func_0034A858(GuiWidget *w, f32 v) {
    w->unk18 = v;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A860);

/* func_0034A9F8: build a list-row widget — init three type-B sub-elements
 * (+0x10/+0x5C/+0xA8) and a list-row element (+0xF4). WALL: 2 callee saves
 * ($16,$31) packed by the original into a 0x10 frame; the pinned cc1 reserves a
 * 0x20 frame. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A9F8);

/* func_0034AA90: handwritten epilogue-only stump (`addiu $sp,$sp,0x20; nop`,
 * no prologue, no `jr ra`). WALL: split-artifact stub. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034AA90);

/* func_0034AA98: large GUI screen/menu construction routine (0x70 frame, 8+
 * callee saves). WALL: many callee saves — frame-layout divergence. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034AA98);

/* func_0034B1E0: handwritten epilogue-only stump (`addiu $sp,$sp,0x20; nop`,
 * no prologue, no `jr ra`). WALL: split-artifact stub. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B1E0);

/* func_0034B1E8: store a1 to the +0x0 field. */
void func_0034B1E8(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0x0) = v;
}

/* func_0034B1F0: set the +0x4 target value; when it actually changes and the
 * player is in normal (Ratchet) mode, latch the previous value at +0x3FC and
 * arm the transition timers (+0x3F8 = 180, +0x3F4 = 300).
 * Best 90%: every instruction matches except the g_bPlayerMode load — the
 * original uses absolute `lui %hi / lbu %lo`, but under this unit's -G8 build the
 * pinned cc1 emits a gp-relative `lbu 0(gp)`. WALL: gp-relative vs absolute
 * global addressing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B1F0);
#else
void func_0034B1F0(GuiWidget *w, s32 v) {
    s32 old = *(s32 *)((char *)w + 0x4);
    if (old != v && g_bPlayerMode == 0) {
        *(s32 *)((char *)w + 0x3FC) = old;
        *(s32 *)((char *)w + 0x3F8) = 0xB4;
        *(s32 *)((char *)w + 0x3F4) = 0x12C;
    }
    *(s32 *)((char *)w + 0x4) = v;
}
#endif

/* func_0034B220: GUI value/gauge update routine reading the D_1AE5E8/D_1AE5F8
 * tables (0x80 frame, 5 GPR + 2 fp callee saves). WALL: many callee saves —
 * frame-layout divergence. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B220);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B548);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B770);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B950);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BA50);

/* func_0034BD28: write the two float args into +0/+4 of the block at widget
 * +0x8 (pointer re-read per store). Best 60%: the original alternates two
 * scratch registers reloaded just-in-time; the pinned cc1 hoists/reuses one.
 * WALL: just-in-time reload register alternation. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BD28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", GuiScreenSetEventAndReveal);

/* func_0034BDA8: handwritten store fragment (`sw $2,0x3F4($4); nop`, no
 * prologue, no `jr ra`, source value in an undefined $2). WALL: split-artifact
 * stub. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BDA8);

/* func_0034BDB0: large GUI screen construction routine — inits a run of type-B
 * elements plus a type-C element and sub-screens (0x90 frame, 10+ callee saves).
 * WALL: many callee saves — frame-layout divergence. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BDB0);

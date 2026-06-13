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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348BD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348BF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348CB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348D98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348DA0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", SetPopupItemEnabled);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", SetPopupItemText);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", GuiScreenWithPlanetNameInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349720);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", UpdatePopupMenu);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349C00);

/* func_00349E88: return the widget pointer unchanged (identity accessor). */
GuiWidget *func_00349E88(GuiWidget *w) {
    return w;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349E90);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A318);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A658);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A6A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A6B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A798);

/* func_0034A7A0: store a2 at +0x20 of the +0x4-stride index entry idx. */
void func_0034A7A0(GuiWidget *w, s32 idx, s32 v) {
    *(s32 *)((char *)w + (idx << 2) + 0x20) = v;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A7B0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A9F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034AA90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034AA98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B1E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B1E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034B1F0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BDA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BDB0);

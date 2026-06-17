#include "common.h"

/*
 * EU text/249FE8 — region-axis twin of USA text/248B50 (the first GUI-block
 * sub-chunk: single-field GUI widget accessors plus the pop-up menu widget and
 * a couple of screen-init helpers). EU base SCES_516.07; this unit mirrors the
 * USA unit's matched bodies verbatim. The accessor bodies touch only struct
 * fields via the $a0 widget pointer (no extern globals), so they are fully
 * region-agnostic — objdiff masks the gp/reloc deltas and no extern retarget is
 * needed. USA function -> EU twin (matched by byte-identical body):
 *   func_00348E20->func_0034A2B0  func_00348E50->func_0034A2E0
 *   func_00348E58->func_0034A2E8  func_00348E60->func_0034A2F0
 *   func_00348E68->func_0034A2F8  func_00349E88->func_0034B310
 *   func_0034A1C8->func_0034B650  func_0034A1D0->func_0034B658
 *   func_0034A2D0->func_0034B758  func_0034A300->func_0034B788
 *   func_0034A308->func_0034B790  func_0034A368->func_0034B7F0
 *   func_0034A3B0->func_0034B838  func_0034A3B8->func_0034B840
 *   func_0034A7E8->func_0034BC70  func_0034A858->func_0034BCE0
 *   SetPopupVisible/SetPopupLayoutMode share their names across regions.
 *
 * Built at -O2 -G8 -fno-gcse (the later-SN-cc1 TU model), same as the USA twin.
 *
 * REGION-DELTA HELD: USA func_0034A7A0 (store a2 at (idx<<2)+0x20) matches USA
 * byte-exact, but its EU twin func_0034BC20 was split with the previous
 * function's epilogue padding (`addiu $sp,+0x10; nop`) merged onto its start —
 * a clean C accessor can't reproduce that leading padding and we don't re-split
 * here, so it stays INCLUDE_ASM. The remaining USA-only matches are themselves
 * INCLUDE_ASM in USA (toolchain walls), so they carry no body to port.
 *
 * REGION-DELTA HELD (func_0034B1E8): USA func_0034B1E8 (leaf `sw a1,0x0(a0)`)
 * matches byte-exact in USA, but its EU twin store has NO standalone splat
 * symbol — the EU split merged the preceding stub's epilogue (`addiu $sp,+0x20;
 * nop`, USA twin of func_0034B1E0) AND the store body into a single symbol
 * func_0034C668, with the store starting at the internal label func_0034C670
 * (`jr ra; sw a1,0x0(a0)`). A clean C accessor can't reproduce a function whose
 * first instruction is that orphaned stub epilogue. Splitting func_0034C670 off
 * needs a symbol_addrs size pin + a configure.py re-split (out of this lane), so
 * the func_0034B1E8 twin stays in func_0034C668's INCLUDE_ASM. Its successor
 * func_0034C678 (= USA func_0034B1F0, the +0x4 transition with the +0x3FC/+0x3F8/
 * +0x3F4 latch) is the gp-vs-absolute g_bPlayerMode WALL on the USA side — and
 * EU emits the ABSOLUTE `lui %hi/lbu %lo` form here, so the EU twin would in fact
 * match where USA does not (a region addressing-model divergence worth a future
 * EU-only attempt, separate from this leaf-store batch).
 */

/* GUI widget — flat field view (offsets are the byte displacements the
 * accessors use; only the fields touched by the matched leaf accessors are
 * named/typed). Identical layout in both regions. */
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A068);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A090);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A148);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A228);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A230);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A2A0);

/* func_0034A2B0 (USA func_00348E20): store a1 to the +0xC8 field. */
void func_0034A2B0(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0xC8) = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A2B8);

/* func_0034A2E0 (USA func_00348E50): store a1 to the +0x60 field. */
void func_0034A2E0(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0x60) = v;
}

/* func_0034A2E8 (USA func_00348E58): store a1 to the +0xCC field. */
void func_0034A2E8(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0xCC) = v;
}

/* func_0034A2F0 (USA func_00348E60): return the +0x60 field. */
s32 func_0034A2F0(GuiWidget *w) {
    return *(s32 *)((char *)w + 0x60);
}

/* func_0034A2F8 (USA func_00348E68): return the +0xC0 field. */
s32 func_0034A2F8(GuiWidget *w) {
    return *(s32 *)((char *)w + 0xC0);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A300);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A690);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A730);

/* SetPopupVisible: store the visibility flag at popup +0x4B8. */
void SetPopupVisible(GuiWidget *w, s32 visible) {
    *(s32 *)((char *)w + 0x4B8) = visible;
}

/* SetPopupLayoutMode: store the layout mode at popup +0x4BC. */
void SetPopupLayoutMode(GuiWidget *w, s32 mode) {
    *(s32 *)((char *)w + 0x4BC) = mode;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", SetPopupTitleText);

/* SetPopupItemEnabled: store the enabled flag for popup item idx at +0x488
 * (s32-stride item table). Region-agnostic leaf store; byte-identical to USA
 * (text/248B50). */
void SetPopupItemEnabled(GuiWidget *w, s32 idx, s32 enabled) {
    *(s32 *)((char *)w + (idx << 2) + 0x488) = enabled;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", SetPopupItemText);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A870);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034ABA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", UpdatePopupMenu);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B088);

/* func_0034B310 (USA func_00349E88): return the widget pointer unchanged. */
GuiWidget *func_0034B310(GuiWidget *w) {
    return w;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B318);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B358);

/* func_0034B650 (USA func_0034A1C8): clear the +0x150 field. */
void func_0034B650(GuiWidget *w) {
    w->unk150 = 0;
}

/* func_0034B658 (USA func_0034A1D0): store a1 to the +0x144 field. */
void func_0034B658(GuiWidget *w, s32 v) {
    w->unk144 = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B660);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B698);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B750);

/* func_0034B758 (USA func_0034A2D0): store a1 to the +0x2C field. */
void func_0034B758(GuiWidget *w, s32 v) {
    w->unk2C = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B760);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B780);

/* func_0034B788 (USA func_0034A300): store a1 to the +0x80 field. */
void func_0034B788(GuiWidget *w, s32 v) {
    w->unk80 = v;
}

/* func_0034B790 (USA func_0034A308): read-modify the +0x28 field — return the
 * old value, store a1. */
s32 func_0034B790(GuiWidget *w, s32 v) {
    s32 old = w->unk28;
    w->unk28 = v;
    return old;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B7A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B7D8);

/* func_0034B7F0 (USA func_0034A368): store a1 to the +0x84 field. */
void func_0034B7F0(GuiWidget *w, s32 v) {
    w->unk84 = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B7F8);

/* func_0034B838 (USA func_0034A3B0): store a1 to the +0x20 field. */
void func_0034B838(GuiWidget *w, s32 v) {
    w->unk20 = v;
}

/* func_0034B840 (USA func_0034A3B8): store the float arg to the +0x24 field. */
void func_0034B840(GuiWidget *w, f32 v) {
    w->unk24 = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B848);

/* func_0034BC20: leading 0x8 padding pair (orphaned epilogue of the preceding
 * function), split off via the symbol_addrs pin so the real body below starts
 * clean. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BC20);

/* func_0034BC28 (USA func_0034A7A0): store a2 at +0x20 of the +0x4-stride index
 * entry idx. Recovered from the EU padding mis-split. */
void func_0034BC28(GuiWidget *w, s32 idx, s32 v) {
    *(s32 *)((char *)w + (idx << 2) + 0x20) = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BC38);

/* func_0034BC70 (USA func_0034A7E8): store a2 at +0x8C of the +0x4-stride index
 * entry idx. */
void func_0034BC70(GuiWidget *w, s32 idx, s32 v) {
    *(s32 *)((char *)w + (idx << 2) + 0x8C) = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BC80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BCA8);

/* func_0034BCE0 (USA func_0034A858): store the float arg to the +0x18 field. */
void func_0034BCE0(GuiWidget *w, f32 v) {
    w->unk18 = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BCE8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BE80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BF18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034C668);

/* func_0034C670 (USA func_0034B1E8): store a1 to the +0x0 field. Recovered from
 * the EU padding mis-split (split off func_0034C668's 0x8 epilogue stump via the
 * symbol_addrs pin). Region-agnostic leaf store. */
void func_0034C670(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0x0) = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034C678);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034C6A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034C9D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034CBF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034CDD8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034CED8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034D1B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", GuiScreenSetEventAndReveal);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034D230);

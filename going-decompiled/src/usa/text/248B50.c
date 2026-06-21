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
 *   +0x8C colorA / +0x90 colorB   packed colour words blended by func_002846E8 */
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
void func_0034A318(GuiWidget *w, s32 idx, f32 a, f32 b, f32 c, f32 d, f32 e);
void func_0034A350(GuiWidget *w, s32 idx, f32 a, f32 b);
void func_0034A7B0(GuiWidget *w, s32 idx1, s32 idx2, f32 a, f32 b, f32 c, f32 d);

/* game callees used by GuiMenuListDraw (func_00348E70). Declared here (not in a
 * central header) with their real signatures so the ILP32 gate sees them; they
 * stay INCLUDE_ASM / runtime stubs natively. */
f32 *func_00336C18(GuiWidget *e);                                /* scratch vec2 */
void GuiElementSetScale(GuiWidget *e, f32 x, f32 y, f32 z, f32 w);
/* GuiElementSetText / GetLocalizedString are intentionally left implicit here:
 * they are already called with char* args elsewhere in this unit, so a file-
 * scope prototype would conflict. Implicit decls are accepted by the gate. */
s32 *GuiElementGetColor(GuiWidget *e);
s32 GuiTextElementMeasure(GuiWidget *e);
void GuiTextElementDraw(GuiWidget *e);
/* DrawFlatRect2d: corners (x1,y1)-(x2,y2) at depth z, fill = pointer to a packed
 * 64-bit GS colour word (tex0 is a pointer despite the historic u64 typing). */
void func_0027F168(s32 x1, s32 y1, s32 x2, s32 y2, s64 z, u64 tex0);
/* func_0027F790: the unconditional tail call; real defined symbol (returns s32,
 * called for side effect). Identity not yet confirmed - keep the func_ name. */
s32 func_0027F790(void);

/* callees used by the GuiAnim transition family (func_0034A860 / func_0034A3C0). */
/* GuiHermiteInterp(t, c0, c1, c2, c3): cubic Hermite blend (0x34FBC0). */
f32 GuiHermiteInterp(f32 t, f32 c0, f32 c1, f32 c2, f32 c3);
/* func_002846E8(colorA, colorB, t): per-channel byte lerp of two packed colour
 * words by phase t (0x2846E8); returns the blended word. */
u32 func_002846E8(u32 colorA, u32 colorB, f32 t);
#endif

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

/* func_00348E10: set the +0xB8 / +0xBC field pair (a1 -> +0xB8, a2 -> +0xBC).
 * Best 96%: the original stores +0xBC first then fills the jr delay slot with
 * the +0xB8 store; the pinned cc1 schedules the two independent struct stores in
 * ascending-offset order. WALL: ascending-offset store scheduling. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348E10);
#else
void func_00348E10(GuiWidget *w, s32 a, s32 b) {
    *(s32 *)((char *)w + 0xB8) = a;
    *(s32 *)((char *)w + 0xBC) = b;
}
#endif

/* func_00348E20: store a1 to the +0xC8 field. */
void func_00348E20(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0xC8) = v;
}

/* func_00348E28: write the two float args into the block at *(w+0x5C) (+0/+4)
 * and zero +8/+0xC; the +0x5C pointer is re-read per store. Best 37%: the
 * original alternates two scratch registers reloaded just-in-time; the pinned
 * cc1 hoists the volatile reloads and reuses one register. WALL: just-in-time
 * reload register alternation. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348E28);
#else
void func_00348E28(GuiWidget *w, f32 x, f32 y) {
    f32 *block = *(f32 **)((char *)w + 0x5C);
    block[0] = x;
    block[1] = y;
    *(s32 *)(block + 2) = 0;
    *(s32 *)(block + 3) = 0;
}
#endif

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

/* func_00348E70 / GuiMenuListDraw: per-frame draw of a vertical text-menu/list
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00348E70);
#else
void func_00348E70(GuiWidget *self) {
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
 * frame at 8-byte slot spacing; the pinned cc1 reserves 16-byte save slots. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_00349200);
#else
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_003492A0);

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
 * WALL: 3 callee saves ($16,$17,$18) — 0x20-vs-packed frame divergence. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", SetPopupItemText);
#else
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", GuiScreenWithPlanetNameInit);

/* func_00349720: handwritten epilogue-only stump (`addiu $sp,$sp,0x30; nop`,
 * no prologue, no `jr ra`) — the trailing half of a hand-split asm routine.
 * No C body can reproduce a function with no return. WALL: split-artifact stub. */
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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A1D8);
#else
void func_0034A1D8(GuiWidget *w) {
    s32 *p = (s32 *)((char *)w + 0x30);
    s32 i;
    for (i = 3; i >= 0; i--) {
        p[0] = 0;
        p[1] = 0;
        p[2] = 0;
        p[3] = 0;
        p += 4;
    }
}
#endif

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
 * scheduling order. (+0x18 is stored as integer 0 — the dead load is read as
 * float but discarded, so the only functional effect is +0x28=1, +0x18=0.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A2E0);
#else
void func_0034A2E0(GuiWidget *w) {
    w->unk28 = 1;
    *(s32 *)((char *)w + 0x18) = 0;
}
#endif

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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A350);
#else
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

/* func_0034A370: always latch +0x1C to a1; then, only while +0x28 is still 0,
 * arm the widget: set +0x18 (integer 0 when a1==1, float 1.0 otherwise) and
 * set the armed flag +0x28 = 1. NOTE the polarity — a1==1 stores 0, any other
 * value stores 1.0f (asm: bne $5,1 -> swc1 1.0; fallthrough -> sw $0). Best 62%:
 * branch-likely block layout + register-allocation deltas. WALL: branch-likely
 * block layout. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A370);
#else
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
#endif

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

/* func_0034A7F8: arm the GuiAnim transition forward. Set the progress field
 * +0x10 (integer 0 when flag is 0, else 1.0f), then mark it active (+0x1C = 1)
 * and forward-playing (+0x14 = 1). The polarity is the mirror of func_0034A820
 * (which stores 1.0 when flag clear and sets dir = -1).
 *
 * The flag test is a branch-likely (`beql $5,$0`): the int-0 store sits in the
 * taken delay slot, so flag==0 stores an *integer* 0 and flag!=0 stores 1.0f.
 *
 * Best 99.8% byte-match: every instruction matches; the original fills the jr
 * delay slot with the +0x14 store (+0x1C stored first), but this unit's cc1
 * always emits +0x14 first and sinks +0x1C into the delay slot regardless of C
 * statement order. WALL: trailing-store delay-slot fill. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A7F8);
#else
void func_0034A7F8(GuiAnim *a, s32 flag) {
    if (flag == 0) {
        *(s32 *)((char *)a + 0x10) = 0;
    } else {
        a->progress = 1.0f;
    }
    a->active = 1;
    a->dir = 1;
}
#endif

/* func_0034A820: set +0x10 enable float (1.0 when flag clear else integer 0),
 * then +0x1C = 1 and +0x14 = -1. Best 80%: same trailing-store slot fill plus a
 * constant-load schedule delta. WALL: trailing-store delay-slot fill. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A820);
#else
void func_0034A820(GuiWidget *w, s32 flag) {
    if (flag == 0) {
        *(f32 *)((char *)w + 0x10) = 1.0f;
    } else {
        *(s32 *)((char *)w + 0x10) = 0;
    }
    *(s32 *)((char *)w + 0x1C) = 1;
    *(s32 *)((char *)w + 0x14) = -1;
}
#endif

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
 *     (+0x8C,+0x90) by a SECOND Hermite ease (over c2,c3) via func_002846E8 and
 *     store the result through the sink.
 *
 * WALL: 99.x near-miss — the original keeps the saved $f20 (1.0) live across the
 * whole body and fills both bc1t/beql delay slots with the dead un-clamped store
 * / pointer advance; the pinned cc1 reloads 1.0 and reorders the clamp stores.
 * Scalar f32 throughout (no VU0) so the #else is bit-exact. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034A860);
#else
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
        *a->sink = (s32)func_002846E8(a->colorA, a->colorB, ct);
    }
}
#endif

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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BD28);
#else
void func_0034BD28(GuiWidget *w, f32 x, f32 y) {
    f32 *block = *(f32 **)((char *)w + 0x8);
    block[0] = x;
    block[1] = y;
}
#endif

/* GuiScreenSetEventAndReveal: stash the pending event id at +0x3F4. If the screen
 * is currently armed-for-reveal (+0x400 set), consume that flag (+0x400=0), raise
 * the "dirty/redraw" flag (+0xC=1), and trigger the four corner reveal animations
 * (sub-widgets at +0x1D4/+0x25C/+0x2E4/+0x36C) forward via func_0034A370(.,1).
 * WALL: 2 callee saves ($16,$31) — 0x10-frame-vs-packed divergence. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", GuiScreenSetEventAndReveal);
#else
void GuiScreenSetEventAndReveal(GuiWidget *w, s32 event) {
    *(s32 *)((char *)w + 0x3F4) = event;
    if (*(s32 *)((char *)w + 0x400) != 0) {
        *(s32 *)((char *)w + 0x400) = 0;
        *(s32 *)((char *)w + 0xC) = 1;
        func_0034A370((char *)w + 0x1D4, 1);
        func_0034A370((char *)w + 0x25C, 1);
        func_0034A370((char *)w + 0x2E4, 1);
        func_0034A370((char *)w + 0x36C, 1);
    }
}
#endif

/* func_0034BDA8: handwritten store fragment (`sw $2,0x3F4($4); nop`, no
 * prologue, no `jr ra`, source value in an undefined $2). WALL: split-artifact
 * stub. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BDA8);

/* func_0034BDB0: large GUI screen construction routine — inits a run of type-B
 * elements plus a type-C element and sub-screens (0x90 frame, 10+ callee saves).
 * WALL: many callee saves — frame-layout divergence. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/248B50", func_0034BDB0);

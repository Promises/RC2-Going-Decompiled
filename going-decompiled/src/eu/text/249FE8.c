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

#ifdef TARGET_NATIVE
/* game callees used by the func_0034A300 (EU twin of USA GuiMenuListDraw)
 * portable body — declared here (defined in other units / still INCLUDE_ASM /
 * runtime-stubbed natively) so the ILP32 gate sees real signatures. */
f32 *func_0027F608(void);                                        /* scratch vec */
f32 *func_00337AF0(void *e);                                     /* pos-vec ptr (consumed by func_0034CBF8/CDD8) */
void func_00337C68(GuiWidget *e, f32 x, f32 y, f32 z, f32 w);    /* USA GuiElementSetScale */
void func_00338730(GuiWidget *e, s32 text);                      /* USA GuiElementSetText */
char *GetLocalizedString(s32 textId);
s32 *func_00337B00(GuiWidget *e);                                /* USA GuiElementGetColor */
s32 func_00338738(GuiWidget *e);                                 /* USA GuiTextElementMeasure */
void func_00338770(GuiWidget *e);                                /* USA GuiTextElementDraw */
void func_0027EFD0(s32 x1, s32 y1, s32 x2, s32 y2, s64 z, u64 tex0); /* USA DrawFlatRect2d */
s32 func_0027F5F8(void);                                         /* tail call (USA func_0027F790) */
#endif

/* func_0034A068 (EU twin of USA func_00348BD0): run the type-C element init on
 * the widget and return it. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_00348BD0: GuiElementInitTypeC ->
 * EU func_00338648 (raw EU twin; jal @0034A074). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A068);
#else
void func_00338648(void *element);              /* EU twin of GuiElementInitTypeC */
void *func_0034A068(void *w) {
    func_00338648(w);
    return w;
}
#endif

/* func_0034A090 (EU twin of USA func_00348BF8): build/init a GUI text element —
 * alloc its backing object via GuiPoolAlloc + GuiPlacementNew, run the text-
 * element init, then seed the colour/style fields (+0xAC..+0xCC). Matching arm
 * stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00348BF8: GuiPoolAlloc/GuiPlacementNew KEEP their
 * names (present verbatim in EU .s); GuiTextElementInit -> EU func_00338688
 * (3rd jal); template D_1AE568 -> EU D_1AE618 (+0xB0 data-lane delta).
 * REGION DELTA (FLAGGED): EU OMITS the USA `el+0x54 = 0` store (USA .s has
 * sw $0,0x54; EU has none) — the rest of the store block (0xB8/0xC4/0xC8) and
 * order are identical. Modeled faithfully. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A090);
#else
void *GuiPoolAlloc(void *pool);
void *GuiPlacementNew(s32 size, void *at);
void func_00338688(void *element, void *tmpl, void *pool);  /* EU GuiTextElementInit */
extern u8 D_1AE618[];                                       /* text-element init template */
void func_0034A090(void *w, void *pool) {
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
    func_00338688(el, D_1AE618, pool);
    *(s32 *)(el + 0xCC) = -1;
    *(s32 *)(el + 0xAC) = 0x70FFFEED;
    *(s32 *)(el + 0xB0) = 0x80F0F0F0;
    *(s32 *)(el + 0xB4) = 0x80808080;
    *(s32 *)(el + 0xBC) = 0x20;
    *(s32 *)(el + 0xB8) = 0;
    *(s32 *)(el + 0xC4) = 0;
    *(s32 *)(el + 0xC8) = 0;
}
#endif

/* func_0034A148 (EU twin of USA func_00348CB8): menu selection-advance driven by
 * the per-frame input mask. Row index at +0x60, row-enable table at +0x6C, row
 * count at +0xC0. LEFT/PREV (0x1000): dir -1; RIGHT/NEXT (0x4000): dir +1;
 * CONFIRM (0x40, only when neither nav bit set): selectable current row returns 1
 * immediately, else dir stays 0 and returns 0. On a nav, play the move sound then
 * step with wrap until a selectable row is found or the scan returns to start.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00348CB8: PlayGlobalSound -> EU func_002E6C28
 * (jal @0034A1B4, args 3,0,0); offsets (0x60/0x6C/0xC0) + masks (0x1000/0x4000/
 * 0x40) identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A148);
#else
void func_002E6C28(s32 id, s32 a, s32 b);       /* EU twin of PlayGlobalSound */
s32 func_0034A148(GuiWidget *w, u32 inputMask) {
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
            if (rowEnable[*curIdx] != 0) {
                return 1;
            }
        }
    }

    if (dir == 0) {
        return 0;
    }

    func_002E6C28(3, 0, 0);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A228);

/* func_0034A230 (EU twin of USA func_00348DA0): store the keyframe table pointer
 * at +0x68, then scan it to count how many leading entries (stride 0x14, capped
 * at 80) have a positive first float; the count lands in +0xC0. Finally, if the
 * current cursor +0x60 has run past the new count, reset it to 0. Matching arm
 * stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00348DA0: no external symbols — pure field access;
 * offsets (0x68/0xC0/0x60), stride 0x14, end = table+0x140 all identical.
 * Region-agnostic. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A230);
#else
void func_0034A230(GuiWidget *w, f32 *table) {
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

/* func_0034A2A0 (EU twin of USA func_00348E10): set the +0xB8 / +0xBC field pair
 * (a -> +0xB8, b -> +0xBC). NOT a structure model: the #else body below is
 * byte-exact. Taken verbatim with only the TARGET_NATIVE guard removed it gives
 * 12 B cmp IDENTICAL, 3/3 words, 0 relocs, 22/67 matched in the unit (/19147).
 * EU C is therefore a preprocessor-arm flip with ZERO code change; the EE build
 * still takes INCLUDE_ASM, pending an authorised arm flip (/19091).
 * Word-verified vs USA func_00348E10: no external symbols; EU .s
 * stores $5->0xB8, $6->0xBC — offsets identical. Region-agnostic. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A2A0);
#else
void func_0034A2A0(GuiWidget *w, s32 a, s32 b) {
    *(s32 *)((char *)w + 0xB8) = a;
    *(s32 *)((char *)w + 0xBC) = b;
}
#endif

/* func_0034A2B0 (USA func_00348E20): store a1 to the +0xC8 field. */
void func_0034A2B0(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0xC8) = v;
}

/* func_0034A2B8 (EU twin of USA func_00348E28): write the two float args into the
 * block at *(w+0x5C) (+0/+4) and zero +8/+0xC; the +0x5C pointer is re-read per
 * store. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00348E28: no external symbols; EU .s reloads 0x5C
 * per store, offsets identical. Region-agnostic. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A2B8);
#else
void func_0034A2B8(GuiWidget *w, f32 x, f32 y) {
    f32 *block = *(f32 **)((char *)w + 0x5C);
    block[0] = x;
    block[1] = y;
    *(s32 *)(block + 2) = 0;
    *(s32 *)(block + 3) = 0;
}
#endif

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

/* func_0034A300 (EU twin of USA func_00348E70 / GuiMenuListDraw): per-frame draw
 * of a vertical text-menu/list widget. Structurally identical to the USA body;
 * EU deltas: an extra leading func_00337AF0(self) call before the scratch alloc
 * (func_0027F608), the row sentinel id is 0x10FC (USA 0x307A), and the callees
 * are the EU func_ symbols. Functional equivalent, not byte-exact. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A300);
#else
void func_0034A300(void *self) {
    char *p = (char *)self;
    f32 *scratch;
    s32 rowCount;
    s32 i;

    func_00337AF0(self);            /* EU-only leading call; result discarded */
    scratch = func_0027F608();

    scratch[0] = ((f32 *)(*(void **)(p + 0x5C)))[0];   /* origin[0] -> scratch[0] */

    rowCount = *(s32 *)(p + 0xC0);
    if (rowCount > 0) {
        for (i = 0; i < rowCount; i++) {
            char *entry = *(char **)(p + 0x68) + i * 0x14;
            s32 rowYBase = *(s32 *)(p + 0xB8);
            s32 rowYStep = *(s32 *)(p + 0xBC);
            f32 originY = ((f32 *)(*(void **)(p + 0x5C)))[1];

            scratch[1] = (f32)(rowYBase + rowYStep * i) + originY;

            func_00337C68((GuiWidget *)self, *(f32 *)entry, 0.0f, 0.0f, 0.0f);

            if (*(s32 *)(entry + 4) == 0x10FC) {
                scratch[1] += 3.0f;
            }

            if (*(s32 *)(p + 0xC4)) {
                func_00338730((GuiWidget *)self, *(s32 *)(entry + 4));
            } else {
                func_00338730((GuiWidget *)self,
                              (s32)GetLocalizedString(*(s32 *)(entry + 4)));
            }

            if (*(s32 *)(p + i * 4 + 0x6C) == 0) {
                /* disabled row */
                *func_00337B00((GuiWidget *)self) = *(s32 *)(p + 0xB4);
            } else {
                f32 originX = scratch[0];
                f32 rowY = scratch[1];

                if (*(s32 *)(p + 0xCC) == i) {
                    /* left underline rect (selected colour fill) */
                    u32 fill = (*(u32 *)(p + 0xAC) & 0x00FFFFFF) | 0x20000000;
                    u64 packed = ((u64)fill << 32) | fill;
                    s32 w1 = func_00338738((GuiWidget *)self) >> 1;
                    s32 y1 = (s32)(rowY + 2.0f);
                    s32 x1 = (s32)(originX - (f32)w1 - 2.0f - 16.0f);
                    s32 w2 = func_00338738((GuiWidget *)self) >> 1;
                    s32 y2 = (s32)(rowY - 2.0f + 16.0f);
                    s32 x2 = (s32)(originX - (f32)w2 - 2.0f - 16.0f + 8.0f);
                    func_0027EFD0(x1, y1, x2, y2, 0, (u64)(unsigned long)&packed);
                }

                if (*(s32 *)(p + 0x60) != i) {
                    /* normal (unhighlighted) row */
                    *func_00337B00((GuiWidget *)self) = *(s32 *)(p + 0xB0);
                } else {
                    /* highlighted (selected) row */
                    *func_00337B00((GuiWidget *)self) = *(s32 *)(p + 0xAC);
                    if (*(s32 *)(p + 0xC8)) {
                        u32 fill = (*(u32 *)(p + 0xAC) & 0x00FFFFFF) | 0x20000000;
                        u64 packed = ((u64)fill << 32) | fill;
                        s32 w1 = func_00338738((GuiWidget *)self) >> 1;
                        s32 y1 = (s32)(rowY - 2.0f);
                        s32 x1 = (s32)(originX - (f32)w1 - 8.0f);
                        s32 w2 = func_00338738((GuiWidget *)self) >> 1;
                        s32 y2 = (s32)(rowY + 2.0f + 16.0f);
                        s32 x2 = (s32)(originX + (f32)w2 + 9.0f);
                        func_0027EFD0(x1, y1, x2, y2, 0, (u64)(unsigned long)&packed);
                    }
                }
            }

            func_00338770((GuiWidget *)self);
        }
    }

    func_0027F5F8();
}
#endif

/* func_0034A690 (EU twin of USA func_00349200): build a 6-row list widget — init
 * seven type-B sub-elements (at +0x0/+0x4C/+0x98/+0xE4/+0x130/+0x17C/+0x1C8) then
 * run type-C init across the seven 0x58-stride row slots starting at +0x218;
 * returns the widget. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00349200: GuiElementInitTypeB -> EU func_00337E88
 * (7 jals), GuiElementInitTypeC -> EU func_00338648 (loop jal; same twin as
 * func_0034A068); embed offsets and 0x58 stride identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A690);
#else
void func_00337E88(void *element);              /* EU twin of GuiElementInitTypeB */
void func_00338648(void *element);              /* EU twin of GuiElementInitTypeC */
GuiWidget *func_0034A690(GuiWidget *w) {
    char *row;
    s32 i;

    func_00337E88((char *)w + 0x0);
    func_00337E88((char *)w + 0x4C);
    func_00337E88((char *)w + 0x98);
    func_00337E88((char *)w + 0xE4);
    func_00337E88((char *)w + 0x130);
    func_00337E88((char *)w + 0x17C);
    func_00337E88((char *)w + 0x1C8);

    row = (char *)w + 0x218;
    for (i = 6; i >= 0; i--) {
        func_00338648(row);
        row += 0x58;
    }
    return w;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A730);

/* SetPopupVisible: store the visibility flag at popup +0x4B8. */
void SetPopupVisible(GuiWidget *w, s32 visible) {
    *(s32 *)((char *)w + 0x4B8) = visible;
}

/* SetPopupLayoutMode: store the layout mode at popup +0x4BC. */
void SetPopupLayoutMode(GuiWidget *w, s32 mode) {
    *(s32 *)((char *)w + 0x4BC) = mode;
}

/* SetPopupTitleText: bind the five glyph slots of a popup's title/border decor
 * to glyph IDs taken from the source record `src` — title (src+0x0) onto the
 * popup body, then four border pieces (src+0x8/+0x10/+0x20/+0x18) onto the four
 * corner sub-elements (+0x130/+0x4C/+0x98/+0xE4). Each glyph is looked up in the
 * GUI instance atlas (g_guiInstance + 0x8710). Matching arm stays INCLUDE_ASM;
 * #else is the structure model.
 * Word-verified vs USA SetPopupTitleText: GuiElementSetGlyph -> EU func_00338070
 * (5x jal); g_guiInstance KEEPS its name. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", SetPopupTitleText);
#else
extern u8 *g_guiInstance;
void func_00338070(GuiWidget *e, u8 *atlas, s32 code);   /* EU GuiElementSetGlyph */
void SetPopupTitleText(GuiWidget *w, s32 *src) {
    u8 *atlas = g_guiInstance + 0x8710;
    func_00338070(w, atlas, src[0]);                                /* src+0x0  */
    func_00338070((GuiWidget *)((char *)w + 0x130), atlas, src[2]); /* src+0x8  */
    func_00338070((GuiWidget *)((char *)w + 0x4C),  atlas, src[4]); /* src+0x10 */
    func_00338070((GuiWidget *)((char *)w + 0x98),  atlas, src[8]); /* src+0x20 */
    func_00338070((GuiWidget *)((char *)w + 0xE4),  atlas, src[6]); /* src+0x18 */
}
#endif

/* SetPopupItemEnabled: store the enabled flag for popup item idx at +0x488
 * (s32-stride item table). Region-agnostic leaf store; byte-identical to USA
 * (text/248B50). */
void SetPopupItemEnabled(GuiWidget *w, s32 idx, s32 enabled) {
    *(s32 *)((char *)w + (idx << 2) + 0x488) = enabled;
}

/* SetPopupItemText: record the item count at +0x4B4, then for each of `count`
 * popup rows (row slots at +0x218, stride 0x58) resolve the string id in ids[i]
 * via GetLocalizedString and apply it to that row's text element. Matching arm
 * stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA SetPopupItemText: GetLocalizedString KEEPS its name;
 * GuiElementSetText -> EU func_00338730. Both reused from file scope. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", SetPopupItemText);
#else
void SetPopupItemText(GuiWidget *w, s32 count, s32 *ids) {
    char *row;
    s32 i;

    *(s32 *)((char *)w + 0x4B4) = count;
    if (count > 0) {
        row = (char *)w + 0x218;
        for (i = count; i != 0; i--) {
            char *text = GetLocalizedString(*ids);
            func_00338730((GuiWidget *)row, (s32)text);
            ids++;
            row += 0x58;
        }
    }
}
#endif

/* func_0034A870 (EU twin of USA GuiScreenWithPlanetNameInit): build the planet-
 * name popup screen. Sets the layout mode, optionally allocs the 0x10-byte config
 * object at +0x4A4 (title position 250,190), inits seven header GuiElements from
 * fixed templates at their offsets, then a run of seven text rows (w+0x218, stride
 * 0x58) — each text-inited, blanked, unit-scaled, coloured 0x80F0F0F0, flagged at
 * w+0x488+i*4. Recolours the seven header elements and assigns their glyphs (codes
 * 0x1C-0x20/0x73/0x74 from the shared atlas g_guiInstance+0x8710), then runs
 * UpdatePopupMenu. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA GuiScreenWithPlanetNameInit (callees by EU jal order,
 * templates by %hi/%lo appearance order — NON-uniform deltas):
 *   SetPopupLayoutMode/GuiPoolAlloc/GuiPlacementNew/UpdatePopupMenu KEEP names;
 *   GuiElementInit    -> func_00337EC0   GuiTextElementInit -> func_00338688;
 *   GuiElementSetText -> func_00338730   GuiElementSetScale -> func_00337C68;
 *   GuiElementGetColor-> func_00337B00   GuiElementSetGlyph -> func_00338070;
 *   templates D_1ADBE8/BF0/C00 -> D_1ADC88/C90/CA0 (+0xA0), D_1ADF98/FA0 ->
 *   D_1AE038/040 (+0xA0), D_1AE570/578/588 -> D_1AE620/628/638 (+0xB0).
 * REGION DELTA (FLAGGED): EU OMITS the USA per-row *(elem+0x54)=0 store in the
 * seven-row loop (same class as batch-1 func_0034A090). Modeled by omission. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034A870);
#else
extern u8 *g_guiInstance;
void *GuiPoolAlloc(void *pool);
void *GuiPlacementNew(s32 size, void *at);
void func_00337EC0(GuiWidget *elem, u8 *tmpl, void *pool);   /* EU GuiElementInit */
void func_00338688(void *element, void *tmpl, void *pool);  /* EU GuiTextElementInit */
void func_00338070(GuiWidget *e, u8 *atlas, s32 code);      /* EU GuiElementSetGlyph */
void SetPopupLayoutMode(GuiWidget *w, s32 mode);
void UpdatePopupMenu(GuiWidget *w, s32 arg);
extern u8 D_1ADC88[], D_1ADC90[], D_1ADCA0[], D_1AE038[], D_1AE040[];
extern u8 D_1AE620[], D_1AE628[], D_1AE638[];
void func_0034A870(void *screen, void *pool) {
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

    func_00337EC0((GuiWidget *)(w + 0x0),   D_1ADC88, pool);
    func_00337EC0((GuiWidget *)(w + 0x130), D_1ADC90, pool);
    func_00337EC0((GuiWidget *)(w + 0x4C),  D_1ADCA0, pool);
    func_00337EC0((GuiWidget *)(w + 0x98),  D_1AE038, pool);
    func_00337EC0((GuiWidget *)(w + 0xE4),  D_1AE040, pool);
    func_00337EC0((GuiWidget *)(w + 0x17C), D_1AE620, pool);
    func_00337EC0((GuiWidget *)(w + 0x1C8), D_1AE628, pool);

    *(s32 *)(w + 0x480) = 0;
    *(s32 *)(w + 0x4B4) = 0;

    for (i = 0; i < 7; i++) {
        char *elem = w + 0x218 + i * 0x58;
        func_00338688(elem, D_1AE638, pool);
        func_00338730((GuiWidget *)elem, 0);
        func_00337C68((GuiWidget *)elem, 1.0f, 0.0f, 0.0f, 0.0f);
        /* REGION DELTA: EU omits the USA *(elem+0x54)=0 store here. */
        *func_00337B00((GuiWidget *)elem) = 0x80F0F0F0;
        *(s32 *)(w + 0x488 + i * 4) = 1;
    }

    *func_00337B00((GuiWidget *)(w + 0x0))   = 0x60442D00;
    *func_00337B00((GuiWidget *)(w + 0x130)) = 0x55F0C070;
    *func_00337B00((GuiWidget *)(w + 0x4C))  = 0x55F0C070;
    *func_00337B00((GuiWidget *)(w + 0x98))  = 0x80FFDE8D;
    *func_00337B00((GuiWidget *)(w + 0xE4))  = 0x70FFFEED;
    *func_00337B00((GuiWidget *)(w + 0x17C)) = 0x60442D00;
    *func_00337B00((GuiWidget *)(w + 0x1C8)) = 0x55F0C070;

    func_00338070((GuiWidget *)(w + 0x0),   g_guiInstance + 0x8710, 0x1C);
    func_00338070((GuiWidget *)(w + 0x130), g_guiInstance + 0x8710, 0x1D);
    func_00338070((GuiWidget *)(w + 0x4C),  g_guiInstance + 0x8710, 0x1E);
    func_00338070((GuiWidget *)(w + 0x98),  g_guiInstance + 0x8710, 0x1F);
    func_00338070((GuiWidget *)(w + 0xE4),  g_guiInstance + 0x8710, 0x20);
    func_00338070((GuiWidget *)(w + 0x17C), g_guiInstance + 0x8710, 0x73);
    func_00338070((GuiWidget *)(w + 0x1C8), g_guiInstance + 0x8710, 0x74);

    UpdatePopupMenu((GuiWidget *)w, 0);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034ABA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", UpdatePopupMenu);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B088);

/* func_0034B310 (USA func_00349E88): return the widget pointer unchanged. */
GuiWidget *func_0034B310(GuiWidget *w) {
    return w;
}

/* func_0034B318 (EU twin of USA func_00349E90): reset the widget's selection
 * table — clear +0x0 and +0x148, fill the 64-entry s32 array at +0x44..+0x140
 * with -1, then clear +0x154/+0x150. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_00349E90: no externs — pure struct
 * stores; base w+0x50, stores at -0xC/-0x8/-0x4/0x0, 16 iterations. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B318);
#else
void func_0034B318(GuiWidget *w) {
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B358);

/* func_0034B650 (USA func_0034A1C8): clear the +0x150 field. */
void func_0034B650(GuiWidget *w) {
    w->unk150 = 0;
}

/* func_0034B658 (USA func_0034A1D0): store a1 to the +0x144 field. */
void func_0034B658(GuiWidget *w, s32 v) {
    w->unk144 = v;
}

/* func_0034B660 (EU twin of USA func_0034A1D8): zero four 4-word (0x10-byte)
 * records starting at widget +0x30 (counter 3 down to -1). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0034A1D8:
 * no externs — pure struct stores; base w+0x30, stores at 0/4/8/C, 4 iterations. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B660);
#else
void func_0034B660(GuiWidget *w) {
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

/* func_0034B698 (EU twin of USA func_0034A210): reset a slider/animation widget
 * to its rest state — clear the value/flag fields (+0x18,+0x28,+0x2C,+0x80), seed
 * the step constant at +0x24 (0.005), zero record slot 0 and fill record slot 1
 * with 1.0 via func_0034B7A0, clear the +0x4-stride index entry 0 via func_0034B7D8,
 * mark "2 records" at +0x84, clear +0x20, set the "armed" flag +0x1C to 1. The
 * +0x18 field is reloaded after being zeroed, so the slot-0 fill writes 0.0.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0034A210: func_0034A318 -> EU func_0034B7A0 (1st/2nd
 * jal), func_0034A350 -> EU func_0034B7D8 (3rd jal); step const 0x3BA3D70A in EU .s.
 * Like USA, EU leaves $2=1 at exit from the +0x1C store — NO region delta; the
 * `return w` mirrors the USA structure model. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B698);
#else
void func_0034B7A0(GuiWidget *w, s32 idx, f32 a, f32 b, f32 c, f32 d, f32 e);
void func_0034B7D8(GuiWidget *w, s32 idx, f32 a, f32 b);
GuiWidget *func_0034B698(GuiWidget *w) {
    f32 zero;

    w->unk18 = 0.0f;
    w->unk28 = 0;
    w->unk2C = 0;
    w->unk80 = 0;
    zero = w->unk18;                 /* reload: now 0.0 */
    w->unk24 = 0.005f;               /* 0x3BA3D70A */

    func_0034B7A0(w, 0, zero, zero, zero, zero, zero);
    func_0034B7A0(w, 1, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    w->unk84 = 2;
    func_0034B7D8(w, 0, zero, zero);

    w->unk20 = 0;
    w->unk1C = 1;
    return w;
}
#endif

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

/* func_0034B7A0 (EU twin of USA func_0034A318): write a 4-float record
 * (b,c,d,e -> +0x30/+0x34/+0x38/+0x3C of the 0x10-stride slot idx) and store the
 * first float (a) into the parallel s32-stride slot at +0x70. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0034A318:
 * no externs — pure float stores; slot base w+(idx<<4), +0x70 base w+(idx<<2). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B7A0);
#else
void func_0034B7A0(GuiWidget *w, s32 idx, f32 a, f32 b, f32 c, f32 d, f32 e) {
    f32 *rec = (f32 *)((char *)w + idx * 0x10);
    rec[0xC] = b;
    rec[0xD] = c;
    rec[0xE] = d;
    rec[0xF] = e;
    *(f32 *)((char *)w + idx * 4 + 0x70) = a;
}
#endif

/* func_0034B7D8 (EU twin of USA func_0034A350): store the float pair a,b into
 * +0x0 / +0xC of the +0x4-stride index entry idx. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA func_0034A350: pure field
 * stores, no externs / no jal. Signature matches func_0034B698's forward-decl. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B7D8);
#else
void func_0034B7D8(GuiWidget *w, s32 idx, f32 a, f32 b) {
    char *entry = (char *)w + (idx << 2);
    *(f32 *)(entry + 0x0) = a;
    *(f32 *)(entry + 0xC) = b;
}
#endif

/* func_0034B7F0 (USA func_0034A368): store a1 to the +0x84 field. */
void func_0034B7F0(GuiWidget *w, s32 v) {
    w->unk84 = v;
}

/* func_0034B7F8 (EU twin of USA func_0034A370): latch +0x1C = v; then, only while
 * +0x28 is still 0, arm the widget — set +0x18 (integer 0 when v==1, else 1.0f)
 * and set +0x28 = 1. NOTE the polarity: v==1 stores integer 0, any other value
 * stores 1.0f. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0034A370: pure field access, no externs / no jal. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B7F8);
#else
void func_0034B7F8(GuiWidget *w, s32 v) {
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

/* func_0034B838 (USA func_0034A3B0): store a1 to the +0x20 field. */
void func_0034B838(GuiWidget *w, s32 v) {
    w->unk20 = v;
}

/* func_0034B840 (USA func_0034A3B8): store the float arg to the +0x24 field. */
void func_0034B840(GuiWidget *w, f32 v) {
    w->unk24 = v;
}

/* func_0034B848 (EU twin of USA func_0034A3C0): per-frame tick of a keyframe-
 * bracketed GuiWidget transition — progress at +0x18, per-frame step at +0x24,
 * direction at +0x1C, active flag at +0x28, keyframe-time count at +0x84, and the
 * ascending f32 keyframe-time table at +0x70. No-op while inactive (+0x28 == 0);
 * otherwise advance+clamp the eased phase, bracket-search the [lo,hi] keyframe
 * pair, Hermite-ease t, optionally dispatch the keyframe pair through the callback
 * object's vtable (+0x2C / obj at +0x80), and on the saturating frame run the
 * end-of-transition dispatch keyed on +0x20. Scalar f32 throughout.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0034A3C0: GuiHermiteInterp KEEPS its name; USA
 * func_0034A370 -> EU func_0034B7F8. No region delta. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034B848);
#else
f32 GuiHermiteInterp(f32 t, f32 c0, f32 c1, f32 c2, f32 c3);
void func_0034B7F8(GuiWidget *w, s32 v);
/* keyframe-callback target vtable (obj at w->unk80): the dispatch reads a
 * half-word field offset at +0x10 and the method pointer at +0x14. */
typedef struct GuiKeyframeTargetVtbl {
    /* 0x00 */ u8 pad00[0x10];
    /* 0x10 */ s16 fieldOff;
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ void (*apply)(void *target, s32 *cbObj, f32 *recLo, f32 *recHi, f32 ease);
} GuiKeyframeTargetVtbl;

void func_0034B848(GuiWidget *w, s32 applyStep) {
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
        func_0034B7F8(w, w->unk1C);
    } else if (w->unk20 < 2) {
        /* mode 0 / negative: nothing. */
    } else if (w->unk20 == 2) {
        s32 dir = ((w->unk1C ^ 1) != 0) ? 1 : -1;
        w->unk1C = dir;
        func_0034B7F8(w, dir);
    } else {
        /* mode > 2: read unk1C, discard. */
    }
}
#endif

/* func_0034BC20: leading 0x8 padding pair (orphaned epilogue of the preceding
 * function), split off via the symbol_addrs pin so the real body below starts
 * clean. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BC20);

/* func_0034BC28 (USA func_0034A7A0): store a2 at +0x20 of the +0x4-stride index
 * entry idx. Recovered from the EU padding mis-split. */
void func_0034BC28(GuiWidget *w, s32 idx, s32 v) {
    *(s32 *)((char *)w + (idx << 2) + 0x20) = v;
}

/* func_0034BC38 (EU twin of USA func_0034A7B0): write a 4-float record (a,b,c,d)
 * at +0x2C of the slot indexed by (idx1<<4) + idx2*0x30 in the widget. Matching
 * arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_0034A7B0: pure float stores, no externs / no jal. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BC38);
#else
void func_0034BC38(GuiWidget *w, s32 idx1, s32 idx2, f32 a, f32 b, f32 c, f32 d) {
    f32 *rec = (f32 *)((char *)w + (idx1 << 4) + idx2 * 0x30 + 0x2C);
    rec[0] = a;
    rec[1] = b;
    rec[2] = c;
    rec[3] = d;
}
#endif

/* func_0034BC70 (USA func_0034A7E8): store a2 at +0x8C of the +0x4-stride index
 * entry idx. */
void func_0034BC70(GuiWidget *w, s32 idx, s32 v) {
    *(s32 *)((char *)w + (idx << 2) + 0x8C) = v;
}

/* func_0034BC80 (EU twin of USA func_0034A7F8): arm the GuiAnim transition
 * forward. Set the progress field +0x10 (integer 0 when flag is 0, else 1.0f),
 * then mark it active (+0x1C = 1) and forward-playing (+0x14 = 1). Matching arm
 * stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_0034A7F8: pure field stores, no externs / no jal. (The anim object is
 * accessed by raw offset off a void* base — GuiAnim is not a file-scope type
 * here.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BC80);
#else
void func_0034BC80(void *anim, s32 flag) {
    char *a = (char *)anim;
    if (flag == 0) {
        *(s32 *)(a + 0x10) = 0;         /* progress = integer 0 */
    } else {
        *(f32 *)(a + 0x10) = 1.0f;      /* progress = 1.0f */
    }
    *(s32 *)(a + 0x1C) = 1;             /* active = 1 */
    *(s32 *)(a + 0x14) = 1;             /* dir = 1 (forward) */
}
#endif

/* func_0034BCA8 (EU twin of USA func_0034A820): set +0x10 enable float (1.0 when
 * flag clear, else integer 0), then +0x1C = 1 and +0x14 = -1. Mirror polarity of
 * func_0034BC80. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0034A820: pure field stores, no externs / no jal. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BCA8);
#else
void func_0034BCA8(GuiWidget *w, s32 flag) {
    if (flag == 0) {
        *(f32 *)((char *)w + 0x10) = 1.0f;
    } else {
        *(s32 *)((char *)w + 0x10) = 0;
    }
    *(s32 *)((char *)w + 0x1C) = 1;
    *(s32 *)((char *)w + 0x14) = -1;
}
#endif

/* func_0034BCE0 (USA func_0034A858): store the float arg to the +0x18 field. */
void func_0034BCE0(GuiWidget *w, f32 v) {
    w->unk18 = v;
}

/* func_0034BCE8 (EU twin of USA func_0034A860): per-frame tick of a GuiAnim
 * transition. No-op while inactive (+0x1C == 0). Otherwise advance the eased phase
 * by the per-frame step (+0x18) — forward (dir +0x14 == 1) adds and clamps the top
 * to 1.0; reverse subtracts and clamps the bottom to 0.0; the active flag clears on
 * the frame the phase saturates. Then t = GuiHermiteInterp(phase,0,c0,c1,1); for
 * each of the two keyframe channels (dst ptr at +0x24/+0x28) lerp the 4-vector
 * dst[k] = (1-t)*from[k] + t*to[k] (from = +0x3C+i*0x10, to = +0x6C+i*0x10). If the
 * colour sink (+0x20) is non-null, blend the two packed colour words (+0x8C,+0x90)
 * by a SECOND Hermite ease (over c2,c3) and store through the sink. Scalar f32.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0034A860: GuiHermiteInterp KEEPS its name (2 jals).
 * REGION DELTA (FLAGGED): the colour-blend callee is EU func_002845F8, NOT USA
 * func_002846E8 — non-uniform delta (-0xF0), confirmed by EU .s `jal func_002845F8`;
 * same (u32,u32,f32)->u32 contract. (GuiAnim accessed by raw offset off void*.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BCE8);
#else
f32 GuiHermiteInterp(f32 t, f32 c0, f32 c1, f32 c2, f32 c3);
u32 func_002845F8(u32 colorA, u32 colorB, f32 t);   /* EU twin of USA func_002846E8 */
void func_0034BCE8(void *anim) {
    char *a = (char *)anim;
    s32 *sink;
    f32 phase;
    f32 clamped;
    f32 t;
    s32 stillActive;
    s32 i;

    if (*(s32 *)(a + 0x1C) == 0) {      /* active */
        return;
    }

    if (*(s32 *)(a + 0x14) == 1) {      /* dir forward */
        phase = *(f32 *)(a + 0x10) + *(f32 *)(a + 0x18);
        stillActive = (phase <= 1.0f) ? 1 : 0;
        clamped = (phase <= 1.0f) ? phase : 1.0f;   /* min(phase, 1.0) */
    } else {
        phase = *(f32 *)(a + 0x10) - *(f32 *)(a + 0x18);
        stillActive = (0.0f <= phase) ? 1 : 0;
        clamped = (0.0f <= phase) ? phase : 0.0f;   /* max(phase, 0.0) */
    }
    *(s32 *)(a + 0x1C) = stillActive;
    *(f32 *)(a + 0x10) = clamped;

    t = GuiHermiteInterp(clamped, 0.0f, *(f32 *)(a + 0x0), *(f32 *)(a + 0x4), 1.0f);

    for (i = 0; i < 2; i++) {
        f32 *dst = *(f32 **)(a + 0x24 + i * 4);
        if (dst != 0) {
            f32 *from = (f32 *)(a + 0x3C + i * 0x10);
            f32 *to   = (f32 *)(a + 0x6C + i * 0x10);
            f32 coF = 1.0f - t;
            dst[0] = coF * from[0] + t * to[0];
            dst[1] = coF * from[1] + t * to[1];
            dst[2] = coF * from[2] + t * to[2];
            dst[3] = coF * from[3] + t * to[3];
        }
    }

    sink = *(s32 **)(a + 0x20);
    if (sink != 0) {
        f32 ct = GuiHermiteInterp(clamped, 0.0f, *(f32 *)(a + 0x8), *(f32 *)(a + 0xC), 1.0f);
        *sink = (s32)func_002845F8(*(u32 *)(a + 0x8C), *(u32 *)(a + 0x90), ct);
    }
}
#endif

/* func_0034BE80 (EU twin of USA func_0034A9F8): build a list-row widget — init
 * three type-B sub-elements (+0x10/+0x5C/+0xA8) and a list-row element (+0xF4),
 * clear the +0x140 record, install two vtable pointers over the +0x1CC/+0x1D0
 * slots (func_003377A8 then store), and clear four more records (+0x1D4/+0x25C/
 * +0x2E4/+0x36C). Returns the widget. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_0034A9F8: GuiElementInitTypeB ->
 * func_00337E88, GuiListRowElementInit -> func_003380B0, func_0034A1D8 ->
 * func_0034B660 (file-scope), func_003368D0 -> func_003377A8; D_1AD8E8 -> D_1AD988
 * (+0xA0), D_1AD908 -> D_1AD9A8 (+0xA0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BE80);
#else
void func_00337E88(void *element);      /* EU twin of GuiElementInitTypeB */
void func_003380B0(void *element);      /* EU twin of GuiListRowElementInit */
void func_003377A8(void *slot);         /* EU twin of func_003368D0 */
extern u8 D_1AD988[];                   /* list-row vtable at widget +0x1CC (EU twin D_1AD8E8) */
extern u8 D_1AD9A8[];                   /* list-row vtable at widget +0x1D0 (EU twin D_1AD908) */
void *func_0034BE80(void *widget) {
    char *w = (char *)widget;

    func_00337E88(w + 0x10);
    func_00337E88(w + 0x5C);
    func_00337E88(w + 0xA8);
    func_003380B0(w + 0xF4);
    func_0034B660((GuiWidget *)(w + 0x140));
    func_003377A8(w + 0x1CC);
    *(u8 **)(w + 0x1CC) = D_1AD988;
    func_003377A8(w + 0x1D0);
    *(u8 **)(w + 0x1D0) = D_1AD9A8;
    func_0034B660((GuiWidget *)(w + 0x1D4));
    func_0034B660((GuiWidget *)(w + 0x25C));
    func_0034B660((GuiWidget *)(w + 0x2E4));
    func_0034B660((GuiWidget *)(w + 0x36C));
    return widget;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034BF18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034C668);

/* func_0034C670 (USA func_0034B1E8): store a1 to the +0x0 field. Recovered from
 * the EU padding mis-split (split off func_0034C668's 0x8 epilogue stump via the
 * symbol_addrs pin). Region-agnostic leaf store. */
void func_0034C670(GuiWidget *w, s32 v) {
    *(s32 *)((char *)w + 0x0) = v;
}

/* func_0034C678 (EU twin of USA func_0034B1F0): set the +0x4 target value; when it
 * actually changes and the player is in normal (Ratchet) mode, latch the previous
 * value at +0x3FC and arm the transition timers (+0x3F8, +0x3F4). Matching arm stays
 * INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0034B1F0: g_bPlayerMode KEEPS its name (EU emits it
 * ABSOLUTE lui %hi/lbu %lo, so the EU matching arm may byte-match where USA hits
 * the gp-vs-absolute wall — a future EU-only attempt).
 * REGION DELTA (FLAGGED, PAL/NTSC timing): EU arms +0x3F8 = 0x96 (150) and
 * +0x3F4 = 0xFA (250) vs USA 0xB4 (180) / 0x12C (300) — 50/60 Hz frame-count
 * scaling of the same 3 s / 5 s transition. Modeled faithfully from EU .s. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034C678);
#else
extern u8 g_bPlayerMode;
void func_0034C678(GuiWidget *w, s32 v) {
    s32 old = *(s32 *)((char *)w + 0x4);
    if (old != v && g_bPlayerMode == 0) {
        *(s32 *)((char *)w + 0x3FC) = old;
        *(s32 *)((char *)w + 0x3F8) = 0x96;   /* PAL 150 frames (NTSC 0xB4=180) */
        *(s32 *)((char *)w + 0x3F4) = 0xFA;   /* PAL 250 frames (NTSC 0x12C=300) */
    }
    *(s32 *)((char *)w + 0x4) = v;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034C6A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034C9D0);

/* func_0034CBF8 (EU twin of USA func_0034B770): refresh the ammo/nanotech HUD list
 * sub-element (w+0xF4). Hidden while g_maxHealth >= 0x50; otherwise shown and
 * populated: item count = snd[0x252C]-[0x2530], scroll = (g_nanotech>>5)-[0x2530]
 * (func_0026F678 is a no-op stub), the +0x140 vec is positioned from the source
 * object (w+0x8) with a biased/rounded y, and the row colour pairs are chosen by the
 * snd[0x22B4] flag. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0034B770: func_00336C18 (pos-vec getter) -> func_00337AF0
 * (file-scope, f32*); GuiElementSetVisible -> func_00337B48; func_0026F818 ->
 * func_0026F678; GuiListSetItemCount -> func_003388E8; GuiListSetScrollPos ->
 * func_003388F0; GuiListSetColorPair0/1 -> func_00338178/func_00338190; g_maxHealth/
 * g_nanotech KEEP names. REGION SYMBOL-BASE DELTAS (same data, follow EU .s):
 * g_soundBankHandlesBlk -> g_sndChannelVolumes+0x1778; g_swapGadgetItemIndex+0x86/
 * 0x8A -> g_nVendorBuyQuantity+0x158/0x15C; layout floats D_1AE750/754 -> D_1AE808/
 * 80C (+0xB8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034CBF8);
#else
extern s32 g_maxHealth;
extern s32 g_nanotech;
extern u8 g_sndChannelVolumes[];
extern u8 g_nVendorBuyQuantity[];
extern f32 D_1AE808, D_1AE80C;
void func_0026F678(void *a, void *b, void *c);           /* no-op stub (EU twin func_0026F818) */
void func_00337B48(GuiWidget *e, s32 visible);           /* EU GuiElementSetVisible */
void func_003388E8(GuiWidget *e, s32 count);             /* EU GuiListSetItemCount */
void func_003388F0(GuiWidget *e, s32 pos);               /* EU GuiListSetScrollPos */
void func_00338178(GuiWidget *e, s32 c0, s32 c1);        /* EU GuiListSetColorPair0 */
void func_00338190(GuiWidget *e, s32 c0, s32 c1);        /* EU GuiListSetColorPair1 */
void func_0034CBF8(GuiWidget *w) {
    char *b = (char *)w;
    GuiWidget *list = (GuiWidget *)(b + 0xF4);
    u8 *snd = g_sndChannelVolumes + 0x1778;
    f32 *vec;
    f32 *obj;
    s32 t0, t1, t2;
    f32 f3, base;
    s32 indexFlag, iv;

    vec = func_00337AF0(list);
    if (g_maxHealth >= 0x50) {
        func_00337B48(list, 0);
        return;
    }
    func_00337B48(list, 1);

    t2 = 0;
    t1 = *(s32 *)(snd + 0x252C);
    t0 = *(s32 *)(snd + 0x2530);
    if (t0 == 0) {
        func_0026F678(&t0, &t1, &t2);
    }
    func_003388E8(list, t1 - t0);
    func_003388F0(list, (g_nanotech >> 5) - t0);

    obj = *(f32 **)(b + 0x8);
    vec[0] = D_1AE808 + obj[0];
    f3 = *(f32 *)(g_nVendorBuyQuantity + 0x15C);
    indexFlag = *(s32 *)(g_nVendorBuyQuantity + 0x158);
    obj = *(f32 **)(b + 0x8);
    base = (D_1AE80C + obj[1]) * f3 + 0.5f;
    if (indexFlag == 1) {
        iv = (s32)(base - 5.0f);
    } else {
        iv = (s32)base;
    }
    vec[1] = (f32)iv;

    if (snd[0x22B4] == 1) {
        func_00338178(list, 0x60808080, 0x60808080);
        func_00338190(list, 0x40808080, 0x40808080);
    } else {
        func_00338178(list, 0x8049C1FF, 0x80001EFF);
        func_00338190(list, 0x50F0C070, 0x50F0C070);
    }
}
#endif

/* func_0034CDD8 (EU twin of USA func_0034B950): per-frame update for a reveal/dismiss
 * screen. Repositions two layout rects (scratch vec2s from the pos-vec getter at the
 * +0x5C and +0xA8 sub-elements) to the source object's (w+0x8) x/y plus fixed offsets,
 * runs the row-layout update (func_0034CBF8) and the +0x140 element step
 * (func_0034B848). Then a dismiss countdown at +0x3F4: while non-zero, decrement; on
 * reaching zero it clears +0xC/+0x3F8, arms +0x400, and retracts the four sub-panels
 * (func_0034B7F8(-1)). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0034B950: func_00336C18 -> func_00337AF0 (file-scope,
 * f32*); func_0034B770 -> func_0034CBF8 (this batch); func_0034A3C0 -> func_0034B848;
 * func_0034A370 -> func_0034B7F8; layout floats D_1AE740/744/748/74C -> D_1AE7F8/7FC/
 * 800/804 (+0xB8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034CDD8);
#else
extern f32 D_1AE7F8, D_1AE7FC, D_1AE800, D_1AE804;
void func_0034CDD8(GuiWidget *w) {
    char *b = (char *)w;
    f32 *obj;
    f32 *r;
    s32 counter;

    r = func_00337AF0((GuiWidget *)(b + 0x5C));
    obj = *(f32 **)(b + 0x8);
    r[0] = D_1AE7F8 + obj[0];
    r[1] = D_1AE7FC + obj[1];

    r = func_00337AF0((GuiWidget *)(b + 0xA8));
    obj = *(f32 **)(b + 0x8);
    r[0] = D_1AE800 + obj[0];
    r[1] = D_1AE804 + obj[1];

    func_0034CBF8(w);
    func_0034B848((GuiWidget *)(b + 0x140), 1);

    counter = *(s32 *)(b + 0x3F4);
    if (counter != 0) {
        counter--;
        *(s32 *)(b + 0x3F4) = counter;
        if (counter == 0) {
            *(s32 *)(b + 0xC) = 0;
            *(s32 *)(b + 0x3F8) = 0;
            *(s32 *)(b + 0x400) = 1;
            func_0034B7F8((GuiWidget *)(b + 0x1D4), -1);
            func_0034B7F8((GuiWidget *)(b + 0x25C), -1);
            func_0034B7F8((GuiWidget *)(b + 0x2E4), -1);
            func_0034B7F8((GuiWidget *)(b + 0x36C), -1);
        }
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034CED8);

/* func_0034D1B0 (EU twin of USA func_0034BD28): write the two float args into +0/+4
 * of the block at widget +0x8 (pointer re-read per store). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0034BD28:
 * no externs, no region delta (leaf float-pair store). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034D1B0);
#else
void func_0034D1B0(GuiWidget *w, f32 x, f32 y) {
    f32 *block = *(f32 **)((char *)w + 0x8);
    block[0] = x;
    block[1] = y;
}
#endif

/* GuiScreenSetEventAndReveal: stash the pending event id at +0x3F4. If the screen is
 * currently armed-for-reveal (+0x400 set), consume that flag (+0x400=0), raise the
 * dirty/redraw flag (+0xC=1), and trigger the four corner reveal animations
 * (sub-widgets at +0x1D4/+0x25C/+0x2E4/+0x36C) forward via func_0034B7F8(.,1).
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA GuiScreenSetEventAndReveal: func_0034A370 -> func_0034B7F8 (file-scope). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", GuiScreenSetEventAndReveal);
#else
void GuiScreenSetEventAndReveal(GuiWidget *w, s32 event) {
    char *b = (char *)w;
    *(s32 *)(b + 0x3F4) = event;
    if (*(s32 *)(b + 0x400) != 0) {
        *(s32 *)(b + 0x400) = 0;
        *(s32 *)(b + 0xC) = 1;
        func_0034B7F8((GuiWidget *)(b + 0x1D4), 1);
        func_0034B7F8((GuiWidget *)(b + 0x25C), 1);
        func_0034B7F8((GuiWidget *)(b + 0x2E4), 1);
        func_0034B7F8((GuiWidget *)(b + 0x36C), 1);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/249FE8", func_0034D230);

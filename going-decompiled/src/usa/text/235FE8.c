#include "common.h"

/*
 * text/235FE8 - GUI widget-method band (carve-pipeline pick #3b, 2026-06-13;
 * vaddr 0x336068..0x348BCF, 344 functions): the backward continuation of the
 * already-matched text/248B50 GuiWidget accessor band (0x348BD0+). Same shape:
 * vtable installers, single-field getters/setters over a widget object passed
 * in $a0, plus the GUI element draw/layout primitives (GuiElementSetPos /
 * GetColor / SetVisible / SetScale), the sprite/text/list element helpers, and
 * the dialog-box / screen constructors.
 *
 * Built at -O2 -G8 -fno-gcse (per-unit GFLAG/CC1EXTRA override in
 * tools/ee/objdiff_build.sh / diff.sh / build.sh) - the later-SN-cc1 GUI TU
 * model shared by text/248B50 / text/198FA0. Under -G8 the small sdata globals
 * (g_waterPool, D_1ADAD8/D_1ADAF0) get their one-insn %gp_rel form; the
 * absolute-addressed globals (g_GuiElementVtable, D_1AD988) are declared with
 * `.extern sym, 16` so cc1 keeps the two-insn %hi/%lo macro.
 *
 * WALLS (left INCLUDE_ASM, per-function notes where relevant):
 *  - 8-byte-packed callee saves: any method saving two or more GPRs hits the
 *    later-cc1 8-byte-slot vs pinned-cc1 16-byte-slot wall.
 *  - switch functions: blocked by the splat jtbl reloc-identity gap.
 *  - the handwritten 8-byte stub-table fragments (func_00336068/00336218/...
 *    = bare addiu-sp/nop runs that are their own functions) stay INCLUDE_ASM.
 */

/* Keep the absolute-addressed globals on the two-insn %hi/%lo macro under -G8. */
__asm__(".extern g_GuiElementVtable, 16");
__asm__(".extern D_1AD988, 16");
__asm__(".extern D_1AD9A8, 16");
__asm__(".extern D_1AD9F8, 16");
__asm__(".extern g_GuiListRowVtable, 16");

extern void *g_GuiElementVtable; /* base GuiElement vtable installed at +0x30 */
extern void *D_1AD988;           /* vtable installed by func_003368D0 / func_003368E8 */
extern void *D_1AD9A8;           /* vtable installed by func_00336678 */
extern void *D_1AD9F8;           /* vtable installed by func_00337830 */
extern void *g_GuiListRowVtable; /* GuiListRow element vtable installed by func_00337278 */

extern void GuiSpriteElementDraw(void *p);
extern void GuiDialogBoxInitElements(void *p);

/* Small-data (gp_rel) vtable globals installed by the func_00336B88/BA8 ctors. */
extern void *D_1AD908;
extern void *D_1AD8E8;

/* A GUI element: first words are pointers into a shared float-vector pool
 * (pos +0x0, scale +0x4), a color block +0xC, a visibility-scalar pointer
 * +0x10; the vtable lives at +0x30. Only touched fields are typed. */
typedef struct GuiElement {
    /* 0x00 */ f32 *pos;     /* -> [x,y,z,w] */
    /* 0x04 */ f32 *scale;   /* -> [x,y,z,w] */
    /* 0x08 */ s32 unk08;
    /* 0x0C */ s32 *color;   /* -> color block */
    /* 0x10 */ f32 *visible; /* -> visibility scalar (>0 shown) */
} GuiElement;


/* callees of the tail-call wrappers below (return values are discarded). */
extern void func_002704E0(void);
extern void func_00336F00(void *p);
extern void func_00336678(void *p, s32 flag);
extern void func_003368E8(void *p, s32 flag);
extern void func_00337C48(void);
extern s32 func_00343AD0(void *p);
extern void func_00343E80(void *p);
extern void func_00343F68(void *p);
extern void func_0033BE60(void *p, s32 v);
extern void func_0033BF90(void *p);
extern void func_00348DA0(void *p);
extern void func_00348E60(void *p);
extern void func_00348E70(void *p);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336068);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336168);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003361C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336218);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336230);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003363A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003364E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003365A0);

/* func_00336648: forward to func_002704E0 (no args). */
void func_00336648(void) {
    func_002704E0();
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336668);

/* func_00336678: if (flag & 1) install the D_1AD9A8 vtable at p+0x4, then call
 * func_00337C48(). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336678);
#else
/* TODO(match): functional equivalent - not byte-exact; the original hoists the
   %hi/%lo address computation above the branch; cc1 sinks it into the
   conditional store. Same wall as func_003368E8. */
void func_00336678(void *p, s32 flag) {
    if (flag & 1) {
        *(void **)((char *)p + 0x4) = &D_1AD9A8;
    }
    func_00337C48();
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiComputeBlendWeights);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336720);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336768);

/* func_003368D0: install the D_1AD988 vtable at +0x0 and return the object. */
void *func_003368D0(void *p) {
    *(void **)p = &D_1AD988;
    return p;
}

/* func_003368E8: if (flag & 1) install the D_1AD988 vtable at p+0x0, then call
 * func_00337C48(). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003368E8);
#else
/* TODO(match): functional equivalent - not byte-exact; the original hoists the
   %hi/%lo address computation above the branch; cc1 sinks it into the
   conditional store. 50% best. */
void func_003368E8(void *p, s32 flag) {
    if (flag & 1) {
        *(void **)p = &D_1AD988;
    }
    func_00337C48();
}
#endif

/* func_00336918: 4-component linear interpolation dst = (1-t)*a + t*b. The
 * leading object pointer (a0) is unused by the body - this is a widget method
 * whose `this` carries no state into the blend. */
void func_00336918(void *self, f32 t, f32 *dst, f32 *a, f32 *b) {
    f32 it = 1.0f - t;
    dst[0] = it * a[0] + t * b[0];
    dst[1] = it * a[1] + t * b[1];
    dst[2] = it * a[2] + t * b[2];
    dst[3] = it * a[3] + t * b[3];
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336988);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336A18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336A28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336A68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336A78);

/* func_00336AC8: 4-component lerp between two interleaved vec4s packed in src
 * (a[i] at src+0x8+8i, b[i] at src+0xC+8i): dst[i] = (1-t)*a[i] + t*b[i]. */
void func_00336AC8(f32 *src, f32 t, f32 *dst) {
    f32 it = 1.0f - t;
    dst[0] = it * src[2] + t * src[3];
    dst[1] = it * src[4] + t * src[5];
    dst[2] = it * src[6] + t * src[7];
    dst[3] = it * src[8] + t * src[9];
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336B38);

/* func_00336B68: forward to func_00336678. */
void func_00336B68(void *p, s32 flag) {
    func_00336678(p, flag);
    __asm__ __volatile__("");
}

/* func_00336B88: install &D_1AD908 at p+0x0 then forward to func_003368E8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336B88);
#else
/* TODO(match): functional equivalent - not byte-exact; the original fills the
   jal delay slot with the field store; cc1 fills it with a nop and stores
   before the call. 77% best. */
void func_00336B88(void *p, s32 flag) {
    *(void **)p = &D_1AD908;
    func_003368E8(p, flag);
}
#endif

/* func_00336BA8: as func_00336B88 with &D_1AD8E8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336BA8);
#else
/* TODO(match): functional equivalent - not byte-exact; same delay-slot fill
   wall as func_00336B88. */
void func_00336BA8(void *p, s32 flag) {
    *(void **)p = &D_1AD8E8;
    func_003368E8(p, flag);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336BC8);

/* func_00336C10: return the word at g_waterPool + 0xC0, reached via a one-insn
 * %gp_rel load. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336C10);
#else
/* TODO(match): functional equivalent - not byte-exact; g_waterPool (0x1B2260)
   is outside the -G8 small-data window, so cc1 emits the two-insn absolute
   %hi/%lo macro instead of the original's one-insn %gp_rel($28). */
extern s32 g_waterPool[];
s32 func_00336C10(void) {
    return g_waterPool[0xC0 / 4];
}
#endif

/* func_00336C18: return the element's pos-vector pointer (+0x0). */
f32 *func_00336C18(GuiElement *e) {
    return e->pos;
}

/* GuiElementGetScaleVec: return the element's scale-vector pointer (+0x4). */
f32 *GuiElementGetScaleVec(GuiElement *e) {
    return e->scale;
}

/* GuiElementGetColor: return the element's color pointer (+0xC). */
s32 *GuiElementGetColor(GuiElement *e) {
    return e->color;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336C30);

/* GuiElementSetPos: write four floats into *(e+0x0), re-reading the vector
 * pointer before every store. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementSetPos);
#else
/* TODO(match): functional equivalent - not byte-exact; this cc1 always CSEs the
   four same-block `lw` reloads into one (the original reloads e->pos before each
   store, alternating two registers). 76% best. */
void GuiElementSetPos(GuiElement *e, f32 x, f32 y, f32 z, f32 w) {
    e->pos[0] = x;
    e->pos[1] = y;
    e->pos[2] = z;
    e->pos[3] = w;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336C68);

/* GuiElementSetVisible: write the visibility scalar at *(e+0x10): 1.0 shown,
 * 0.0 hidden. */
void GuiElementSetVisible(GuiElement *e, s32 show) {
    if (show) {
        *e->visible = 1.0f;
    } else {
        *e->visible = 0.0f;
    }
}

/* GuiElementIsVisible: true when the visibility scalar is > 0. */
s32 GuiElementIsVisible(GuiElement *e) {
    return *e->visible > 0.0f;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementShareScaleVec);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336D20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336D28);

/* GuiElementSetScale: as GuiElementSetPos for the scale vector at +0x4. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementSetScale);
#else
/* TODO(match): functional equivalent - not byte-exact; same reloaded-pointer
   CSE wall as GuiElementSetPos. 76% best. */
void GuiElementSetScale(GuiElement *e, f32 x, f32 y, f32 z, f32 w) {
    e->scale[0] = x;
    e->scale[1] = y;
    e->scale[2] = z;
    e->scale[3] = w;
}
#endif

/* GuiElementInstallBaseVtable: install the base vtable at +0x30, return e. */
GuiElement *GuiElementInstallBaseVtable(GuiElement *e) {
    *(void **)((char *)e + 0x30) = &g_GuiElementVtable;
    return e;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementBaseInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336EF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336F00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementInitTypeB);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337090);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337098);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337110);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiSpriteElementDraw);

/* GuiElementSetGlyph: look up the glyph for (codepoint, font) and store the
 * resulting handle at +0x40. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementSetGlyph);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame wall
   ($16/$31 16-byte vs 8-byte slot packing). */
extern s32 GuiFontAtlasLookupGlyph(s32 codepoint, s32 font);
void GuiElementSetGlyph(GuiElement *e, s32 codepoint, s32 font) {
    *(s32 *)((char *)e + 0x40) = GuiFontAtlasLookupGlyph(codepoint, font);
}
#endif

/* GuiElementSetAlpha: write the alpha float through the element's +0x38 ptr. */
void GuiElementSetAlpha(GuiElement *e, f32 alpha) {
    **(f32 **)((char *)e + 0x38) = alpha;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiListRowElementInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiListElementInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337270);

/* func_00337278: install the GuiListRow vtable at p+0x30, then call
 * func_00336F00(p). */
void func_00337278(void *p) {
    *(void **)((char *)p + 0x30) = &g_GuiListRowVtable;
    func_00336F00(p);
    __asm__ __volatile__("");
}

/* GuiListSetColorPair0: write two colors into *(e+0xC) at +0/+4, re-reading the
 * block pointer between stores. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiListSetColorPair0);
#else
/* TODO(match): functional equivalent - not byte-exact; reloaded-pointer CSE
   wall. 76% best. */
void GuiListSetColorPair0(GuiElement *e, s32 c0, s32 c1) {
    e->color[0] = c0;
    e->color[1] = c1;
}
#endif

/* GuiListSetColorPair1: as GuiListSetColorPair0 at +8/+0xC. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiListSetColorPair1);
#else
/* TODO(match): functional equivalent - not byte-exact; reloaded-pointer CSE
   wall. 76% best. */
void GuiListSetColorPair1(GuiElement *e, s32 c0, s32 c1) {
    e->color[2] = c0;
    e->color[3] = c1;
}
#endif

/* func_003372D0: write the high (alpha) byte of color words [0] and [1] of the
 * element's color block (+0xC), preserving the low 24 RGB bits. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003372D0);
#else
/* TODO(match): functional equivalent - not byte-exact; the original reloads
   e->color before the second word and interleaves the two halves; cc1 CSEs the
   pointer and batches the masks. ~24% best. */
void func_003372D0(GuiElement *e, s32 a0, s32 a1) {
    s32 *c = e->color;
    c[0] = (c[0] & 0xFFFFFF) | (a0 << 24);
    c = e->color;
    c[1] = (c[1] & 0xFFFFFF) | (a1 << 24);
}
#endif

/* func_00337310: write the high (alpha) byte of color words [2] and [3] of the
 * element's color block (+0xC), preserving the low 24 RGB bits. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337310);
#else
/* TODO(match): functional equivalent - not byte-exact; same reloaded-pointer /
   interleave mismatch as func_003372D0. ~24% best. */
void func_00337310(GuiElement *e, s32 a0, s32 a1) {
    s32 *c = e->color;
    c[2] = (c[2] & 0xFFFFFF) | (a0 << 24);
    c = e->color;
    c[3] = (c[3] & 0xFFFFFF) | (a1 << 24);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337350);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003374D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337510);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiSpriteElementInit);

/* GuiSpriteGetTextureVec: return the sprite's texture-vec pointer (+0x34). */
f32 *GuiSpriteGetTextureVec(GuiElement *e) {
    return *(f32 **)((char *)e + 0x34);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003375C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003375D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337630);

/* GuiSpriteSetTexture: write two int->float coords through *(e+0x34),
 * re-reading the pointer per store. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiSpriteSetTexture);
#else
/* TODO(match): functional equivalent - not byte-exact; reloaded-pointer CSE wall
   (cc1 collapses the two *(e+0x34) reloads). 90% best. */
void GuiSpriteSetTexture(GuiElement *e, s32 u, s32 v) {
    f32 fu = (f32)u;
    f32 fv = (f32)v;
    (*(f32 **)((char *)e + 0x34))[0] = fu;
    (*(f32 **)((char *)e + 0x34))[1] = fv;
}
#endif

/* func_00337758: return the first float of the vector at p+0x34 converted to
 * int (cvt.w.s). */
s32 func_00337758(void *p) {
    f32 *vec = *(f32 **)((char *)p + 0x34);
    return (s32)vec[0];
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementInitTypeC);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003377A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiTextElementInit);

/* func_00337830: install the D_1AD9F8 vtable at p+0x30, then call func_00336F00(p). */
void func_00337830(void *p) {
    *(void **)((char *)p + 0x30) = &D_1AD9F8;
    func_00336F00(p);
    __asm__ __volatile__("");
}

/* GuiElementSetTextFlag: store the flag word at +0x44. */
void GuiElementSetTextFlag(GuiElement *e, s32 flag) {
    *(s32 *)((char *)e + 0x44) = flag;
}

/* GuiElementSetText: store the text handle at +0x40. */
void GuiElementSetText(GuiElement *e, s32 text) {
    *(s32 *)((char *)e + 0x40) = text;
}

/* GuiTextElementMeasure: measure the text element's string - forwards the text
 * handle (+0x40), a -1 max-width sentinel, the +0x34 field, and the scale.x
 * (*(scale+0)) to the shared text-measure helper func_0027F858. */
extern s32 func_0027F858(s32 text, s32 maxWidth, s32 arg2, f32 scaleX);
s32 GuiTextElementMeasure(GuiElement *e) {
    return func_0027F858(*(s32 *)((char *)e + 0x40), -1,
                         *(s32 *)((char *)e + 0x34), e->scale[0]);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337898);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiTextElementDraw);

/* GuiListSetItemCount: store the item count at +0x40. */
void GuiListSetItemCount(GuiElement *e, s32 count) {
    *(s32 *)((char *)e + 0x40) = count;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiListSetScrollPos);

/* func_00337B68: store an int at +0x38. */
void func_00337B68(GuiElement *e, s32 v) {
    *(s32 *)((char *)e + 0x38) = v;
}

/* GuiListSetVisibleRows: store the row count (as a float) through the list's
 * +0x4 scale-vector pointer at +0x4. */
void GuiListSetVisibleRows(GuiElement *e, s32 rows) {
    e->scale[1] = (f32)rows;
}

/* func_00337B88: store an int at +0x44. */
void func_00337B88(GuiElement *e, s32 v) {
    *(s32 *)((char *)e + 0x44) = v;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiFontAtlasRelocate);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiFontAtlasLookupGlyph);

/* func_00337C48: no-op stub (empty body - registered/overridable hook). */
void func_00337C48(void) {
}

/* GuiPlacementNew: placement-new passthrough - return the supplied buffer. */
void *GuiPlacementNew(s32 size, void *buf) {
    return buf;
}

/* func_00337C58: identity passthrough - return the first arg. */
void *func_00337C58(void *p) {
    return p;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiPoolInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337CE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiPoolAlloc);

/* func_00337D78: push a node onto the pool free-list at p+0x14 and decrement
 * the live count at p+0x10. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337D78);
#else
/* TODO(match): functional equivalent - not byte-exact; the original stores the
   old head into *node before writing head=node; cc1 reorders the two
   independent stores. 96% best. */
void func_00337D78(void *pool, void **node) {
    *node = *(void **)((char *)pool + 0x14);
    *(void **)((char *)pool + 0x14) = node;
    *(s32 *)((char *)pool + 0x10) -= 1;
}
#endif

/* func_00337D98: return the small-data global D_1ADAF0. */
extern s32 D_1ADAF0;
/* func_00337D98: accessor - return the D_1ADAF0 small-data global. */
s32 func_00337D98(void) {
    return D_1ADAF0;
}

/* func_00337DA0: bounds-checked lookup into the 4-entry small-data table
 * D_1ADAD8 (idx<4 ? D_1ADAD8[idx] : 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337DA0);
#else
/* TODO(match): functional equivalent - not byte-exact; the original takes the
   table base via one-insn %gp_rel($28); cc1 emits the two-insn absolute %hi/%lo
   for the indexed array base under -G8. 88% best. */
extern s32 D_1ADAD8[4];
s32 func_00337DA0(u32 idx) {
    if (idx < 4) {
        return D_1ADAD8[idx];
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337DC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003380B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338A80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338AB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338CD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338CD8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338D48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338F18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338F80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338F88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00339398);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003395F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00339678);

/* func_003396E0: no-op stub (empty body - registered/overridable hook). */
void func_003396E0(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003396E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00339740);

/* func_00339A80: no-op stub (empty body - registered/overridable hook). */
void func_00339A80(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00339A88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00339B00);

/* func_00339F90: no-op stub (empty body - registered/overridable hook). */
void func_00339F90(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00339F98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A048);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A0B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiConfirmPopupInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiConfirmPopupTick);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiConfirmPopupDraw);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A640);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A678);

/* func_0033A7A8: set the +0x330 flag to 1. */
void func_0033A7A8(void *p) {
    *(s32 *)((char *)p + 0x330) = 1;
}

/* func_0033A7B8: forward p+0x8 to func_0033BE60 (the wrapper sets only $a0; the
 * callee's second slot is left untouched). */
void func_0033A7B8(void *p) {
    ((void (*)(void *))func_0033BE60)((char *)p + 0x8);
    __asm__ __volatile__("");
}

/* func_0033A7D8: store an int at +0x328. */
void func_0033A7D8(void *p, s32 v) {
    *(s32 *)((char *)p + 0x328) = v;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A7E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A860);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A8E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A8F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A9F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033AA80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033AB10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiLevelInfoPanelInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033AF68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiLevelInfoPanelTick);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B428);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B4E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B560);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiProgressBarWidgetInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B6D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B720);

/* func_0033BA10: store a float at +0x214. */
void func_0033BA10(void *p, f32 v) {
    *(f32 *)((char *)p + 0x214) = v;
}

/* func_0033BA18: store v into the int array at p+0x1C8, element index idx. */
void func_0033BA18(void *p, s32 idx, s32 v) {
    s32 *row = (s32 *)((char *)p + idx * 4);
    row[0x72] = v;
}

/* func_0033BA28: clamp - if idx is below the count at p+0x208, store it at
 * p+0x218. */
void func_0033BA28(void *p, s32 idx) {
    if (idx < *(s32 *)((char *)p + 0x208)) {
        *(s32 *)((char *)p + 0x218) = idx;
    }
}

/* func_0033BA40: store an int at +0x21C. */
void func_0033BA40(void *p, s32 v) {
    *(s32 *)((char *)p + 0x21C) = v;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033BA48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxInitElements);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033BC18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxInitBorder);

/* func_0033BE60: store an int at +0x2C0. */
void func_0033BE60(void *p, s32 v) {
    *(s32 *)((char *)p + 0x2C0) = v;
}

/* func_0033BE68: store an int at +0x2C4. */
void func_0033BE68(void *p, s32 v) {
    *(s32 *)((char *)p + 0x2C4) = v;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033BE70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033BF90);

/* func_0033C060: write two floats through *(p+0x0) at +0/+4, re-reading the
 * pointer between stores. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C060);
#else
/* TODO(match): functional equivalent - not byte-exact; reloaded-pointer CSE
   wall. 76% best. */
void func_0033C060(void *p, f32 a, f32 b) {
    (*(f32 **)p)[0] = a;
    (*(f32 **)p)[1] = b;
}
#endif

/* GuiDialogBoxSetBounds: write six dialog-box bound floats. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxSetBounds);
#else
/* TODO(match): functional equivalent - not byte-exact; the original keeps source
   store order; cc1 reschedules the independent stores ascending. 95% best. */
void GuiDialogBoxSetBounds(void *p, f32 a, f32 b, f32 c, f32 d, f32 e, f32 f) {
    *(f32 *)((char *)p + 0x2B4) = f;
    *(f32 *)((char *)p + 0x2A8) = a;
    *(f32 *)((char *)p + 0x2AC) = b;
    *(f32 *)((char *)p + 0x2B8) = c;
    *(f32 *)((char *)p + 0x2BC) = d;
    *(f32 *)((char *)p + 0x2B0) = e;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxSetText3);

/* GuiDialogBoxSetScale: store the dialog-box scale float at +0x2A4. */
void GuiDialogBoxSetScale(void *p, f32 scale) {
    *(f32 *)((char *)p + 0x2A4) = scale;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C0F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C100);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C1A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiIconListScreenInit);

/* func_0033C580: a bare `daddu $2,$4,$0` fall-through fragment (NO jr $31) -
 * a handwritten stub-table entry, not a real C function. WALL: stays asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C580);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C588);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C958);

/* func_0033CD80: init the embedded dialog-box (at p+0x8), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033CD80);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame wall
   (the body matches but the pinned cc1 lays the $16/$31 saves in 16-byte slots
   vs the original's 8-byte packing). 99.64% best. */
void *func_0033CD80(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033CDB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxVariantCInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033CEE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D070);

/* func_0033D1C0: init the embedded dialog-box (at p+0x8), return the object
 * (identical body to func_0033CD80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D1C0);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. 99.64% best. */
void *func_0033D1C0(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D1F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D1F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D320);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D3C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D478);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D4A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D4B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D5D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D780);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DA00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DA30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxVariantBInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DB60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DC68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DDC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DE08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DE10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E070);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E308);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E488);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E4B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E4C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E5E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E680);

/* GuiQuitDialogInitElements: init the embedded dialog-box (at p+0x8), return the
 * object (identical body to func_0033CD80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiQuitDialogInitElements);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. 99.64% best. */
void *GuiQuitDialogInitElements(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E780);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiQuitDialogInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E8B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E9B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EB20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EB50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EB58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EC80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033ED18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EDD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EE00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EE08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EF30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EFC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EFF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F000);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F128);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F1C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F1F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F200);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F360);

/* func_0033F398: forward p+0x2E0 to func_00348E60. */
void func_0033F398(void *p) {
    func_00348E60((char *)p + 0x2E0);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F3B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F3F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F478);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F4D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F508);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F510);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F610);

/* func_0033F670: forward p+0x8 to func_0033BF90. */
void func_0033F670(void *p) {
    func_0033BF90((char *)p + 0x8);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F690);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F718);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiStatsPanelScreenInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FAB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FCE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FDD8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FEF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FF68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003400D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiQuickSelectWheelInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiQuickSelectWheelTick);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341160);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003413A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341548);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341708);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003418D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341A80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341C28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiScrollListScreenInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341F40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003420C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiIconScreenInit);

/* func_00342450: write three ints at +0x318/+0x310/+0x314 in that source order. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342450);
#else
/* TODO(match): functional equivalent - not byte-exact; cc1 reorders the three
   independent stores ascending. 95% best. */
void func_00342450(void *p, s32 a, s32 b, s32 c) {
    *(s32 *)((char *)p + 0x318) = c;
    *(s32 *)((char *)p + 0x310) = a;
    *(s32 *)((char *)p + 0x314) = b;
}
#endif

/* func_00342460: store an int at +0x230. */
void func_00342460(void *p, s32 v) {
    *(s32 *)((char *)p + 0x230) = v;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342468);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003424C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342520);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342670);

/* func_003426D8: no-op stub (empty body - registered/overridable hook). */
void func_003426D8(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003426E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003427D0);

/* func_00342958: forward p+0x130 to GuiSpriteElementDraw. */
void func_00342958(void *p) {
    GuiSpriteElementDraw((char *)p + 0x130);
    __asm__ __volatile__("");
}

/* func_00342978: forward p+0x238 to func_00348E70. */
void func_00342978(void *p) {
    func_00348E70((char *)p + 0x238);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342998);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342B30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342BA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342BE0);

/* func_00342BE8: store an int at +0x8. */
void func_00342BE8(void *p, s32 v) {
    *(s32 *)((char *)p + 0x8) = v;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiHelpPromptWidgetInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342D60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342D68);

/* func_00342DA0: forward p+0x10 to func_00348E60. */
void func_00342DA0(void *p) {
    func_00348E60((char *)p + 0x10);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342DC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342DF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342E48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342FD8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiIconScreenInit2);

/* func_00343290: store v at p+0x260 then call func_00348DA0(p+0x188). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343290);
#else
/* TODO(match): functional equivalent - not byte-exact; the original keeps a $a0
   copy and fills the jal delay slot with the store; cc1 stores before the call.
   68% best. */
void func_00343290(void *p, s32 v) {
    *(s32 *)((char *)p + 0x260) = v;
    func_00348DA0((char *)p + 0x188);
}
#endif

/* func_003432B8: store an int at +0x170. */
void func_003432B8(void *p, s32 v) {
    *(s32 *)((char *)p + 0x170) = v;
}

/* func_003432C0: return base (p+0x260) + index (p+0x16C) * 0x14 - a pointer
 * into a 20-byte-stride array. */
s32 func_003432C0(void *p) {
    return *(s32 *)((char *)p + 0x260) + *(s32 *)((char *)p + 0x16C) * 0x14;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003432D8);

/* func_00343330: no-op stub (empty body - registered/overridable hook). */
void func_00343330(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343338);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003433D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003434C8);

/* func_00343558: forward p+0x188 to func_00348E70. */
void func_00343558(void *p) {
    func_00348E70((char *)p + 0x188);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343578);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343668);

/* func_003436C0: identity passthrough - return the first arg. */
void *func_003436C0(void *p) {
    return p;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003436C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003436D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003437F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343888);

/* func_00343AD0: indexed +0x28 lookup gated on the +0x10 flag: if (p[0x10]==0)
 * return 0; else return *(p + p[0x14]*4 + 0x28). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343AD0);
#else
/* TODO(match): functional equivalent - not byte-exact; the original lowers the
   guard as a branch-likely (bnel + nullified load in the delay slot); cc1 emits
   beqz + a separate load. 38% best. */
s32 func_00343AD0(void *p) {
    if (*(s32 *)((char *)p + 0x10) == 0) {
        return 0;
    }
    return *(s32 *)((char *)p + *(s32 *)((char *)p + 0x14) * 4 + 0x28);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343AF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343E80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343F30);

/* func_00343F38: store two floats at +0x8 and +0xC. */
void func_00343F38(void *p, f32 a, f32 b) {
    *(f32 *)((char *)p + 0x8) = a;
    *(f32 *)((char *)p + 0xC) = b;
}

/* func_00343F48: store a float at +0x4. */
void func_00343F48(void *p, f32 v) {
    *(f32 *)((char *)p + 0x4) = v;
}

/* func_00343F50: as func_0033C060: write two floats through *(p+0x0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343F50);
#else
/* TODO(match): functional equivalent - not byte-exact; reloaded-pointer CSE
   wall. 76% best. */
void func_00343F50(void *p, f32 a, f32 b) {
    (*(f32 **)p)[0] = a;
    (*(f32 **)p)[1] = b;
}
#endif

/* func_00343F68: clear the int at +0x90. */
void func_00343F68(void *p) {
    *(s32 *)((char *)p + 0x90) = 0;
}

/* func_00343F70: return the int at +0x10. */
s32 func_00343F70(void *p) {
    return *(s32 *)((char *)p + 0x10);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343F78);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344110);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiTitledSpriteScreenInit);

/* func_00344458: forward p+0x2C8 to func_00343E80. */
void func_00344458(void *p) {
    func_00343E80((char *)p + 0x2C8);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344478);

/* func_00344480: forward p+0x2C8 to func_00343AD0. */
void func_00344480(void *p) {
    func_00343AD0((char *)p + 0x2C8);
    __asm__ __volatile__("");
}

/* func_003444A0: forward p+0x2C8 to func_00343F68. */
void func_003444A0(void *p) {
    func_00343F68((char *)p + 0x2C8);
    __asm__ __volatile__("");
}

/* func_003444C0: store an int at +0x360. */
void func_003444C0(void *p, s32 v) {
    *(s32 *)((char *)p + 0x360) = v;
}

/* func_003444C8: return the int at +0x360. */
s32 func_003444C8(void *p) {
    return *(s32 *)((char *)p + 0x360);
}

/* func_003444D0: if idx is in range (<4) record it at p+0x2C4, then reset the
 * field at p+0x1A0 to zero. */
void func_003444D0(void *p, u32 idx) {
    if (idx < 4) {
        *(s32 *)((char *)p + 0x2C4) = idx;
    }
    *(s32 *)((char *)p + 0x1A0) = 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003444E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344558);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003446B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344800);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344808);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003448C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiInfoPanelScreenInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344E08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344F18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00345080);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003451B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00345298);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003453D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003457A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00345890);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00345F00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00345FF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003460E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003461D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003462C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00346368);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiMapScreenInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00346878);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00346AF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00346CD8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiMapScreenTick);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00347228);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00347348);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00347450);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00347550);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003475F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiWeaponGridScreenInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiWeaponGridTick);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003481E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00348628);

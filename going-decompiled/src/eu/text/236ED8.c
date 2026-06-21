#include "common.h"
/*
 * text/236ED8 (EU) - GUI widget-method band; the EU (SCES_516.07) re-tile of the
 * USA text/235FE8 GUI accessor band (carve-pipeline). Region-axis port: the C
 * bodies are byte-identical to their USA counterparts (objdiff masks the gp/reloc
 * deltas); only the referenced extern global/function NAMES are retargeted to
 * their EU addresses. Built at -O2 -G8 -fno-gcse (per-unit GFLAG/CC1EXTRA).
 *
 * USA<->EU name map for the ported bodies is recorded per-function below
 * ("USA <name>" in each doc comment). The matched set mirrors USA text/235FE8.
 */

/* Keep the absolute-addressed globals on the two-insn %hi/%lo macro under -G8. */
__asm__(".extern D_1ADA28, 16");
__asm__(".extern D_1ADA78, 16");
__asm__(".extern D_1ADA98, 16");
__asm__(".extern D_1ADAF8, 16");

extern void *D_1ADA28;  /* vtable installed by func_003377A8 (USA D_1AD988) */
extern void *D_1ADA78;  /* GuiListRow vtable installed by func_00338150 (USA g_GuiListRowVtable) */
extern void *D_1ADA98;  /* vtable installed by func_00338700 (USA D_1AD9F8) */
extern void *D_1ADAF8;  /* base GuiElement vtable, +0x30 (USA g_GuiElementVtable) */
extern s32 D_1ADB90;    /* small-data accessor target (USA D_1ADAF0) */

/* A GUI element: pos +0x0, scale +0x4, color +0xC, visibility-scalar ptr +0x10;
 * vtable at +0x30. Only touched fields are typed. (Same layout as USA.) */
typedef struct GuiElement {
    /* 0x00 */ f32 *pos;     /* -> [x,y,z,w] */
    /* 0x04 */ f32 *scale;   /* -> [x,y,z,w] */
    /* 0x08 */ s32 unk08;
    /* 0x0C */ s32 *color;   /* -> color block */
    /* 0x10 */ f32 *visible; /* -> visibility scalar (>0 shown) */
} GuiElement;

/* callees referenced by the ported wrapper bodies (EU addresses). */
extern void func_00270340(void);
extern void func_00337550(void *p, s32 flag);
extern void func_00337DD8(void *p);
extern void func_00338000(void *p);
extern void func_0033CD40(void *p, s32 v);
extern void func_0033CE70(void *p);
extern void func_0034A2F0(void *p);
extern void func_0034A300(void *p);
extern void func_00344FD8(void *p);
extern void func_00344C28(void *p);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00336F58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337048);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003370A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003370F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337110);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337280);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003373C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337478);

/* func_00337520: forward to func_00270340 (no args). USA func_00336648. */
void func_00337520(void) {
    func_00270340();
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337540);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337550);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", GuiComputeBlendWeights);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003375F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337640);

/* func_003377A8: install the D_1ADA28 vtable at +0x0 and return the object.
 * USA func_003368D0. */
void *func_003377A8(void *p) {
    *(void **)p = &D_1ADA28;
    return p;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003377C0);

/* func_003377F0: 4-component linear interpolation dst = (1-t)*a + t*b. The
 * leading object pointer (a0) is unused by the body. USA func_00336918. */
void func_003377F0(void *self, f32 t, f32 *dst, f32 *a, f32 *b) {
    f32 it = 1.0f - t;
    dst[0] = it * a[0] + t * b[0];
    dst[1] = it * a[1] + t * b[1];
    dst[2] = it * a[2] + t * b[2];
    dst[3] = it * a[3] + t * b[3];
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337860);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003378F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337940);

/* func_003379A0: 4-component lerp between two interleaved vec4s packed in src
 * (a[i] at src+0x8+8i, b[i] at src+0xC+8i): dst[i] = (1-t)*a[i] + t*b[i].
 * USA func_00336AC8. */
void func_003379A0(f32 *src, f32 t, f32 *dst) {
    f32 it = 1.0f - t;
    dst[0] = it * src[2] + t * src[3];
    dst[1] = it * src[4] + t * src[5];
    dst[2] = it * src[6] + t * src[7];
    dst[3] = it * src[8] + t * src[9];
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337A10);

/* func_00337A40: forward to func_00337550. USA func_00336B68. */
void func_00337A40(void *p, s32 flag) {
    func_00337550(p, flag);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337A60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337A80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337AA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337AE8);

/* func_00337AF0: return the element's pos-vector pointer (+0x0).
 * USA func_00336C18. */
f32 *func_00337AF0(GuiElement *e) {
    return e->pos;
}

/* func_00337AF8: return the element's scale-vector pointer (+0x4).
 * USA GuiElementGetScaleVec. */
f32 *func_00337AF8(GuiElement *e) {
    return e->scale;
}

/* func_00337B00: return the element's color pointer (+0xC).
 * USA GuiElementGetColor. */
s32 *func_00337B00(GuiElement *e) {
    return e->color;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337B08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337B18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337B40);

/* func_00337B48: write the visibility scalar at *(e+0x10): 1.0 shown, 0.0
 * hidden. USA GuiElementSetVisible. */
void func_00337B48(GuiElement *e, s32 show) {
    if (show) {
        *e->visible = 1.0f;
    } else {
        *e->visible = 0.0f;
    }
}

/* func_00337B68: true when the visibility scalar is > 0.
 * USA GuiElementIsVisible. */
s32 func_00337B68(GuiElement *e) {
    return *e->visible > 0.0f;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337B90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337BF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337C68);

/* func_00337C90: install the base vtable at +0x30, return e.
 * USA GuiElementInstallBaseVtable. */
GuiElement *func_00337C90(GuiElement *e) {
    *(void **)((char *)e + 0x30) = &D_1ADAF8;
    return e;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337CA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337DD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337DD8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337E88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337EC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337F68);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337FE8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338000);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338070);

/* func_003380A0: write the alpha float through the element's +0x38 ptr.
 * USA GuiElementSetAlpha. */
void func_003380A0(GuiElement *e, f32 alpha) {
    **(f32 **)((char *)e + 0x38) = alpha;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003380B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003380E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338148);

/* func_00338150: install the GuiListRow vtable (D_1ADA78) at p+0x30, then call
 * func_00337DD8(p). Recovered from the EU padding mis-split (split off
 * func_00338148's 0x8 epilogue stump via the symbol_addrs pin). USA func_00337278
 * (installs g_GuiListRowVtable, calls func_00336F00). */
void func_00338150(void *p) {
    *(void **)((char *)p + 0x30) = &D_1ADA78;
    func_00337DD8(p);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338178);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338190);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003381A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003381E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338228);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003383B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003383E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003383F0);

/* func_00338498: return the sprite's texture-vec pointer (+0x34).
 * USA GuiSpriteGetTextureVec. */
f32 *func_00338498(GuiElement *e) {
    return *(f32 **)((char *)e + 0x34);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003384A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338508);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338600);

/* func_00338630: return the first float of the vector at p+0x34 converted to
 * int (cvt.w.s). USA func_00337758. */
s32 func_00338630(void *p) {
    f32 *vec = *(f32 **)((char *)p + 0x34);
    return (s32)vec[0];
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338648);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338680);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338688);

/* func_00338700: install the D_1ADA98 vtable at p+0x30, then call
 * func_00337DD8(p). USA func_00337830. */
void func_00338700(void *p) {
    *(void **)((char *)p + 0x30) = &D_1ADA98;
    func_00337DD8(p);
    __asm__ __volatile__("");
}

/* func_00338728: store the flag word at +0x44. USA GuiElementSetTextFlag. */
void func_00338728(GuiElement *e, s32 flag) {
    *(s32 *)((char *)e + 0x44) = flag;
}

/* func_00338730: store the text handle at +0x40. USA GuiElementSetText. */
void func_00338730(GuiElement *e, s32 text) {
    *(s32 *)((char *)e + 0x40) = text;
}

/* func_00338738: measure the text element's string - forwards the text handle
 * (+0x40), a -1 max-width sentinel, the +0x34 field, and the scale.x (*(scale+0))
 * to the shared text-measure helper func_0027F6C0. USA GuiTextElementMeasure. */
extern s32 func_0027F6C0(s32 text, s32 maxWidth, s32 arg2, f32 scaleX);
s32 func_00338738(GuiElement *e) {
    return func_0027F6C0(*(s32 *)((char *)e + 0x40), -1,
                         *(s32 *)((char *)e + 0x34), e->scale[0]);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338768);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338770);

/* func_003388E8: store the item count at +0x40. USA GuiListSetItemCount. */
void func_003388E8(GuiElement *e, s32 count) {
    *(s32 *)((char *)e + 0x40) = count;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003388F0);

/* func_00338A18: store an int at +0x38. USA func_00337B68. */
void func_00338A18(GuiElement *e, s32 v) {
    *(s32 *)((char *)e + 0x38) = v;
}

/* func_00338A20: store the row count (as a float) through the list's +0x4
 * scale-vector pointer at +0x4. USA GuiListSetVisibleRows. */
void func_00338A20(GuiElement *e, s32 rows) {
    e->scale[1] = (f32)rows;
}

/* func_00338A38: store an int at +0x44. USA func_00337B88. */
void func_00338A38(GuiElement *e, s32 v) {
    *(s32 *)((char *)e + 0x44) = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338A40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338AA8);

void func_00338AF8(void) {
}

/* GuiPlacementNew: placement-new passthrough - return the supplied buffer. */
void *GuiPlacementNew(s32 size, void *buf) {
    return buf;
}

/* func_00338B08: identity passthrough - return the first arg. USA func_00337C58. */
void *func_00338B08(void *p) {
    return p;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", GuiPoolInit);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338B90);

/* GuiPoolAlloc: allocate one node from a GUI fixed-size pool (EU twin of USA
 * GuiPoolAlloc). See the USA unit for the field map. Pop the free list head if
 * present, else bump-allocate base+cursor; capacity overflow trips an assert. */
typedef struct GuiPool {
    /* 0x00 */ char *base;
    /* 0x04 */ u32 capacity;
    /* 0x08 */ u32 elemSize;
    /* 0x0C */ u32 cursor;
    /* 0x10 */ s32 count;
    /* 0x14 */ void *freeList;
} GuiPool;
extern void AssertFail(const char *file, s32 line, const char *expr);
extern const char D_1ADB18[];
extern const char D_1ADB60[];
void *GuiPoolAlloc(GuiPool *pool) {
    void *node = pool->freeList;
    if (node != 0) {
        pool->freeList = *(void **)node;
        pool->count++;
        return node;
    }
    if (pool->cursor + pool->elemSize <= pool->capacity) {
        char *result = pool->base + pool->cursor;
        pool->cursor += pool->elemSize;
        pool->count++;
        return result;
    }
    AssertFail(D_1ADB18, 0x53, D_1ADB60);
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338C28);

/* func_00338C48: accessor - return the D_1ADB90 small-data global.
 * USA func_00337D98 (D_1ADAF0). */
s32 func_00338C48(void) {
    return D_1ADB90;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338C50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338C78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338F68);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339930);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339968);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339B80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339B88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339BF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339DC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339E30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339E38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A248);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A4A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A528);

void func_0033A590(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A598);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A5F0);

void func_0033A930(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A938);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A9B0);

void func_0033AE40(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033AE48);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033AEF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033AF60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B218);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B4B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B520);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B558);

/* func_0033B688: set the +0x330 flag to 1. USA func_0033A7A8. */
void func_0033B688(void *p) {
    *(s32 *)((char *)p + 0x330) = 1;
}

/* func_0033B698: forward p+0x8 to func_0033CD40 (the wrapper sets only $a0).
 * USA func_0033A7B8. */
void func_0033B698(void *p) {
    ((void (*)(void *))func_0033CD40)((char *)p + 0x8);
    __asm__ __volatile__("");
}

/* func_0033B6B8: store an int at +0x328. USA func_0033A7D8. */
void func_0033B6B8(void *p, s32 v) {
    *(s32 *)((char *)p + 0x328) = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B6C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B740);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B7C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B7D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B8D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B960);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B9F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033BE48);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033BE50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033C308);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033C3C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033C440);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033C5B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033C600);

/* func_0033C8F0: store a float at +0x214. USA func_0033BA10. */
void func_0033C8F0(void *p, f32 v) {
    *(f32 *)((char *)p + 0x214) = v;
}

/* func_0033C8F8: store v into the int array at p+0x1C8, element index idx.
 * USA func_0033BA18. */
void func_0033C8F8(void *p, s32 idx, s32 v) {
    s32 *row = (s32 *)((char *)p + idx * 4);
    row[0x72] = v;
}

/* func_0033C908: clamp - if idx is below the count at p+0x208, store it at
 * p+0x218. USA func_0033BA28. */
void func_0033C908(void *p, s32 idx) {
    if (idx < *(s32 *)((char *)p + 0x208)) {
        *(s32 *)((char *)p + 0x218) = idx;
    }
}

/* func_0033C920: store an int at +0x21C. USA func_0033BA40. */
void func_0033C920(void *p, s32 v) {
    *(s32 *)((char *)p + 0x21C) = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033C928);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CA78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CAF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CB00);

/* func_0033CD40: store an int at +0x2C0. USA func_0033BE60. */
void func_0033CD40(void *p, s32 v) {
    *(s32 *)((char *)p + 0x2C0) = v;
}

/* func_0033CD48: store an int at +0x2C4. USA func_0033BE68. */
void func_0033CD48(void *p, s32 v) {
    *(s32 *)((char *)p + 0x2C4) = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CD50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CE70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CF40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CF58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CF78);

/* func_0033CFD0: store the dialog-box scale float at +0x2A4.
 * USA GuiDialogBoxSetScale. */
void func_0033CFD0(void *p, f32 scale) {
    *(f32 *)((char *)p + 0x2A4) = scale;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CFD8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033D088);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033D460);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033D468);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033D838);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033DC60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033DC90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033DDC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033DF90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E000);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E030);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E160);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E248);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E300);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E330);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E460);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E608);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E888);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E8B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E9E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033EAF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033EC50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033EC90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033EEF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F190);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F310);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F340);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F470);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F508);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F5D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F608);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F738);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F840);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F9A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F9D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FB00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FBE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FC98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FCC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FDF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FED8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FF08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340038);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340118);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340150);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003402B8);

/* func_003402F0: forward p+0x2E0 to func_0034A2F0. USA func_0033F398. */
void func_003402F0(void *p) {
    func_0034A2F0((char *)p + 0x2E0);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340310);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340348);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340418);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340470);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003404A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003405B0);

/* func_00340610: forward p+0x8 to func_0033CE70. USA func_0033F670. */
void func_00340610(void *p) {
    func_0033CE70((char *)p + 0x8);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340630);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003406B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340A50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340C88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340D78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00341020);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00341090);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00341200);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00341C18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003422D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00342518);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003426B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00342878);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00342A48);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00342BF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00342D98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00342DE8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003430B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343230);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003432A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003435C0);

/* func_003435D0: store an int at +0x230. USA func_00342460. */
void func_003435D0(void *p, s32 v) {
    *(s32 *)((char *)p + 0x230) = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003435D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343638);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343690);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003437C8);

void func_00343830(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343838);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343928);

/* func_00343AB0: forward p+0x130 to func_00338000. USA func_00342958. */
void func_00343AB0(void *p) {
    func_00338000((char *)p + 0x130);
    __asm__ __volatile__("");
}

/* func_00343AD0: forward p+0x238 to func_0034A300. USA func_00342978. */
void func_00343AD0(void *p) {
    func_0034A300((char *)p + 0x238);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343AF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343C88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343CF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343D38);

/* func_00343D40: store an int at +0x8. USA func_00342BE8. */
void func_00343D40(void *p, s32 v) {
    *(s32 *)((char *)p + 0x8) = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343D48);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343EB8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343EC0);

/* func_00343EF8: forward p+0x10 to func_0034A2F0. USA func_00342DA0. */
void func_00343EF8(void *p) {
    func_0034A2F0((char *)p + 0x10);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343F18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343F50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00343FA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344130);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344180);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003443E8);

/* func_00344410: store an int at +0x170. USA func_003432B8. */
void func_00344410(void *p, s32 v) {
    *(s32 *)((char *)p + 0x170) = v;
}

/* func_00344418: return base (p+0x260) + index (p+0x16C) * 0x14 - a pointer
 * into a 20-byte-stride array. USA func_003432C0. */
s32 func_00344418(void *p) {
    return *(s32 *)((char *)p + 0x260) + *(s32 *)((char *)p + 0x16C) * 0x14;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344430);

void func_00344488(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344490);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344528);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344620);

/* func_003446B0: forward p+0x188 to func_0034A300. USA func_00343558. */
void func_003446B0(void *p) {
    func_0034A300((char *)p + 0x188);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003446D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003447C0);

/* func_00344818: identity passthrough - return the first arg. USA func_003436C0. */
void *func_00344818(void *p) {
    return p;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344820);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344828);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344948);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003449E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344C28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344C50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00344FD8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00345088);

/* func_00345090: store two floats at +0x8 and +0xC. USA func_00343F38. */
void func_00345090(void *p, f32 a, f32 b) {
    *(f32 *)((char *)p + 0x8) = a;
    *(f32 *)((char *)p + 0xC) = b;
}

/* func_003450A0: store a float at +0x4. USA func_00343F48. */
void func_003450A0(void *p, f32 v) {
    *(f32 *)((char *)p + 0x4) = v;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003450A8);

/* func_003450C0: clear the int at +0x90. USA func_00343F68. */
void func_003450C0(void *p) {
    *(s32 *)((char *)p + 0x90) = 0;
}

/* func_003450C8: return the int at +0x10. USA func_00343F70. */
s32 func_003450C8(void *p) {
    return *(s32 *)((char *)p + 0x10);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003450D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00345268);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003452C8);

/* func_003455B0: forward p+0x2C8 to func_00344FD8. USA func_00344458. */
void func_003455B0(void *p) {
    func_00344FD8((char *)p + 0x2C8);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003455D0);

/* func_003455D8: forward p+0x2C8 to func_00344C28. USA func_00344480. */
void func_003455D8(void *p) {
    func_00344C28((char *)p + 0x2C8);
    __asm__ __volatile__("");
}

/* func_003455F8: forward p+0x2C8 to func_003450C0. USA func_003444A0. */
void func_003455F8(void *p) {
    func_003450C0((char *)p + 0x2C8);
    __asm__ __volatile__("");
}

/* func_00345618: store an int at +0x360. USA func_003444C0. */
void func_00345618(void *p, s32 v) {
    *(s32 *)((char *)p + 0x360) = v;
}

/* func_00345620: return the int at +0x360. USA func_003444C8. */
s32 func_00345620(void *p) {
    return *(s32 *)((char *)p + 0x360);
}

/* func_00345628: if idx is in range (<4) record it at p+0x2C4, then reset the
 * field at p+0x1A0 to zero. USA func_003444D0. */
void func_00345628(void *p, u32 idx) {
    if (idx < 4) {
        *(s32 *)((char *)p + 0x2C4) = idx;
    }
    *(s32 *)((char *)p + 0x1A0) = 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00345640);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003456B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003457F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00345938);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00345940);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003459F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00345A88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00345F98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003460A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00346210);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00346348);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00346428);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00346528);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003468F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003469E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00347078);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00347168);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00347258);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00347348);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00347438);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003474E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00347570);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00347A50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00347CD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00347EB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003480C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00348400);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00348520);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00348628);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00348728);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003487C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00348868);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00348DF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00349450);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00349600);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00349AB8);

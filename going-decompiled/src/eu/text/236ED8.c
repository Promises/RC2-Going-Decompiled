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
    /* 0x08 */ f32 *unk08;   /* -> 16-byte vector block (allocated 2nd by base init) */
    /* 0x0C */ s32 *color;   /* -> color block */
    /* 0x10 */ f32 *visible; /* -> visibility scalar (>0 shown) */
} GuiElement;

/* GUI fixed-size node pool (allocator at GuiPoolAlloc, initialiser at GuiPoolInit
 * below). Defined up front so the element constructor (func_00337CA8, USA
 * GuiElementBaseInit) and GuiPoolInit all see the field layout regardless of
 * source order. Pure type info: byte-neutral.
 *   +0x00 base / +0x04 capacity / +0x08 elemSize / +0x0C cursor /
 *   +0x10 count / +0x14 freeList (next ptr at node+0). */
typedef struct GuiPool {
    /* 0x00 */ char *base;
    /* 0x04 */ u32 capacity;
    /* 0x08 */ u32 elemSize;
    /* 0x0C */ u32 cursor;
    /* 0x10 */ s32 count;
    /* 0x14 */ void *freeList;
} GuiPool;
extern void *GuiPoolAlloc(GuiPool *pool);
extern void *GuiPlacementNew(s32 size, void *buf);
extern void func_00337B48(GuiElement *e, s32 show); /* USA GuiElementSetVisible */

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

/* func_00337048: copy a 16-byte (quadword) vector from *src into the active
 * camera's render block at +0x80. The camera is g_activeCamera unless its mode
 * word (+0x86) differs from 5, in which case func_002700F0(5) supplies a
 * substitute camera; the render block lives at *(camera+0x70). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_00336168:
 * g_activeCamera KEPT, func_00270290->func_002700F0, offsets +0x86/==5/+0x70/+0x80. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337048);
#else
/* A 16-byte (128-bit) quadword record, copied with a single lq/sq. */
typedef struct GuiQword { s32 w[4]; } GuiQword;
extern void *g_activeCamera;
extern void *func_002700F0(s32 mode);
void func_00337048(GuiQword *src) {
    void *cam = g_activeCamera;
    void *base = (*(s16 *)((char *)cam + 0x86) == 5) ? cam : func_002700F0(5);
    GuiQword *dst = (GuiQword *)((char *)*(void **)((char *)base + 0x70) + 0x80);
    *dst = *src;
}
#endif

/* func_003370A0: as func_00337048 but writes the quadword to the camera render
 * block at +0x90 instead of +0x80. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_003361C0: g_activeCamera KEPT,
 * func_00270290->func_002700F0, store offset +0x90. Reuses GuiQword. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003370A0);
#else
extern void *g_activeCamera;
extern void *func_002700F0(s32 mode);
void func_003370A0(GuiQword *src) {
    void *cam = g_activeCamera;
    void *base = (*(s16 *)((char *)cam + 0x86) == 5) ? cam : func_002700F0(5);
    GuiQword *dst = (GuiQword *)((char *)*(void **)((char *)base + 0x70) + 0x90);
    *dst = *src;
}
#endif

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

/* func_00337550: ALWAYS install the D_1ADA48 vtable at p+0x4 (the store sits in
 * the beqz delay slot, so it is unconditional), then call func_00338AF8() only
 * when (flag & 1). Base GuiElement ctor. Matching arm stays INCLUDE_ASM; #else is
 * the structure model. Word-verified vs USA func_00336678: D_1AD9A8->D_1ADA48,
 * func_00337C48->func_00338AF8, store offset +0x4. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337550);
#else
extern void *D_1ADA48;
extern void func_00338AF8(void);
void func_00337550(void *p, s32 flag) {
    *(void **)((char *)p + 0x4) = &D_1ADA48;   /* delay-slot store: unconditional */
    if (flag & 1) {
        func_00338AF8();
    }
}
#endif

/* GuiComputeBlendWeights: from a single blend factor t, fill a 4-float weight
 * vector for a two-control-point blend: out[0]=t, out[1]=t*0.5, out[2]=1-t,
 * out[3]=(1-t)*0.5. Asserts the output pointer is non-NULL (AssertFail with the
 * D_1AD930 path, line 0x3D, predicate D_1AD948). The leading object pointer (a0)
 * is unused; t arrives in $f12 and the output pointer in $a1. USA twin
 * GuiComputeBlendWeights (byte-identical logic). */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; the 8-byte-packed
   callee-save frame wall (s0/ra/f20 packed into a -0x20 frame). Body otherwise
   bit-identical to the asm. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", GuiComputeBlendWeights);
#else
extern void AssertFail(const char *file, s32 line, const char *expr);
extern const char D_1AD930[]; /* assert source-file path */
extern const char D_1AD948[]; /* assert predicate text */
void GuiComputeBlendWeights(void *self, f32 t, f32 *out) {
    (void)self;
    if (out == 0) {
        AssertFail(D_1AD930, 0x3D, D_1AD948);
    }
    out[0] = t;
    out[2] = 1.0f - t;
    out[1] = t * 0.5f;
    out[3] = (1.0f - t) * 0.5f;
}
#endif

/* func_003375F8: install the D_1ADA08 vtable at p+0x4, then run the base ctor
 * func_00337550(p, flag) (the field store sits in the jal delay slot). USA twin
 * func_00336720 (USA installs D_1AD968 / calls func_00336678). */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; jal-delay-slot-store
   wall (the original fills the func_00337550 delay slot with the +0x4 store). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003375F8);
#else
extern void *D_1ADA08;
void func_003375F8(void *p, s32 flag) {
    *(void **)((char *)p + 0x4) = &D_1ADA08;
    func_00337550(p, flag);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337640);

/* func_003377A8: install the D_1ADA28 vtable at +0x0 and return the object.
 * USA func_003368D0. */
void *func_003377A8(void *p) {
    *(void **)p = &D_1ADA28;
    return p;
}

/* func_003377C0: ALWAYS install the D_1ADA28 vtable at p+0x0 (the store sits in
 * the beqz delay slot, so it is unconditional), then call func_00338AF8() only
 * when (flag & 1). Same store-always/call-conditional shape as func_00337550.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA func_003368E8 asm: D_1AD988->D_1ADA28, func_00337C48->func_00338AF8, store
 * offset +0x0. (NOTE: the USA func_003368E8 #else body mis-models the semantics as
 * conditional-store/unconditional-call; the true asm - both regions - is
 * store-always/call-if-flag. Modeled faithfully to the asm. D_1ADA28 file-scope.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003377C0);
#else
extern void func_00338AF8(void);
void func_003377C0(void *p, s32 flag) {
    *(void **)p = &D_1ADA28;   /* delay-slot store: unconditional */
    if (flag & 1) {
        func_00338AF8();
    }
}
#endif

/* func_003377F0: 4-component linear interpolation dst = (1-t)*a + t*b. The
 * leading object pointer (a0) is unused by the body. USA func_00336918. */
void func_003377F0(void *self, f32 t, f32 *dst, f32 *a, f32 *b) {
    f32 it = 1.0f - t;
    dst[0] = it * a[0] + t * b[0];
    dst[1] = it * a[1] + t * b[1];
    dst[2] = it * a[2] + t * b[2];
    dst[3] = it * a[3] + t * b[3];
}

/* func_00337860: per-channel blend of a 4-int vector. For each component i in
 * {0,1,2,3}, out[i] = func_002845F8(t, aSrc[i], bSrc[i]) - the scalar color/alpha
 * interpolation helper applied component-wise. The leading object pointer (a0)
 * is unused. USA twin func_00336988 (USA calls func_002846E8; EU's region-
 * specific commit helper is func_002845F8). */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee-save
   frame wall (s0/s1/s2/ra/f20). Body insn stream otherwise identical. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337860);
#else
extern s32 func_002845F8(f32 t, s32 a, s32 b);
void func_00337860(void *self, s32 *out, s32 *aSrc, s32 *bSrc, f32 t) {
    out[0] = func_002845F8(t, aSrc[0], bSrc[0]);
    out[1] = func_002845F8(t, aSrc[1], bSrc[1]);
    out[2] = func_002845F8(t, aSrc[2], bSrc[2]);
    out[3] = func_002845F8(t, aSrc[3], bSrc[3]);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003378F0);

/* func_00337940: install the D_1AD968 vtable at p+0x4, free the pooled node
 * (*(p+0x2C) is the pool, *(p+0x28) the node) via func_00338C28, then run the base
 * ctor func_00337550(p, flag). Matching arm stays INCLUDE_ASM; #else is the
 * structure model.
 * TWIN CORRECTION: the twin-map assigned func_00336A28 (a Hermite blend), a
 * mis-map; the real USA twin is func_00336A78 (verified op-for-op). Word-verified
 * vs USA func_00336A78: D_1AD8C8->D_1AD968, func_00337D78->func_00338C28,
 * func_00336678->func_00337550. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337940);
#else
extern void *D_1AD968;
extern void func_00338C28(void *pool, void **node);
void func_00337940(void *p, s32 flag) {
    *(void **)((char *)p + 0x4) = &D_1AD968;
    func_00338C28(*(void **)((char *)p + 0x2C), *(void ***)((char *)p + 0x28));
    func_00337550(p, flag);
}
#endif

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

/* func_00337A60: install &D_1AD9A8 at p+0x0 then forward to the ctor
 * func_003377C0(p, flag) (the field store sits in the jal delay slot). Matching arm
 * stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_00336B88:
 * D_1AD908->D_1AD9A8 (gp_rel), func_003368E8->func_003377C0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337A60);
#else
extern void *D_1AD9A8;
extern void func_003377C0(void *p, s32 flag);
void func_00337A60(void *p, s32 flag) {
    *(void **)p = &D_1AD9A8;
    func_003377C0(p, flag);
}
#endif

/* func_00337A80: as func_00337A60 with &D_1AD988. Install &D_1AD988 at p+0x0 then
 * forward to func_003377C0(p, flag). Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_00336BA8: D_1AD8E8->D_1AD988 (gp_rel),
 * func_003368E8->func_003377C0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337A80);
#else
extern void *D_1AD988;
extern void func_003377C0(void *p, s32 flag);
void func_00337A80(void *p, s32 flag) {
    *(void **)p = &D_1AD988;
    func_003377C0(p, flag);
}
#endif

/* func_00337AA0: seed the gadget-swap zoom animation. Store the swap flag/index
 * (a0) as a word at g_nVendorBuyQuantity+0x158, then two zoom factors: at +0x15C
 * the start scale (1.0 when flag==0, else 1.076923) and at +0x160 the end scale
 * (1.0 when flag==0, else 0.9). Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_00336BC8: g_swapGadgetItemIndex
 * (+0x86/+0x8A/+0x8E) -> g_nVendorBuyQuantity (+0x158/+0x15C/+0x160); consts
 * 0x3F89D89D=1.076923, 0x3F666666=0.9, 0x3F800000=1.0 identical.
 * REGION-DELTA FLAG: EU sub-field offsets (+0x158/+0x15C/+0x160) differ from USA's
 * (+0x86/+0x8A/+0x8E) - a genuine data-layout shift, not just the +0x80 base. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337AA0);
#else
extern s32 g_nVendorBuyQuantity;
void func_00337AA0(s32 flag) {
    char *base = (char *)&g_nVendorBuyQuantity;
    *(s32 *)(base + 0x158) = flag;
    *(f32 *)(base + 0x15C) = (flag != 0) ? 1.076923f : 1.0f;
    *(f32 *)(base + 0x160) = (flag != 0) ? 0.9f : 1.0f;
}
#endif

/* func_00337AE8: return the word at g_nVendorBuyQuantity+0x158 (the swap flag
 * stored by func_00337AA0), reached via a one-insn %gp_rel load. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_00336C10:
 * USA reads g_waterPool+0xC0; EU reads g_nVendorBuyQuantity+0x158.
 * REGION-DELTA FLAG: the read target is a DIFFERENT global than the USA twin's -
 * the EU accessor pairs with its own func_00337AA0 writer (region-specific
 * data wiring, not an address shift). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337AE8);
#else
extern s32 g_nVendorBuyQuantity;
s32 func_00337AE8(void) {
    return *(s32 *)((char *)&g_nVendorBuyQuantity + 0x158);
}
#endif

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

/* func_00337B90: rebind the element's scale vector (+0x4) to an externally-owned
 * vector. If the new pointer differs from the current scale, and the element owns
 * a pool (+0x2C) and has not yet released its own scale node (the +0x18 latch is
 * clear), free the old scale node back to the pool (func_00338C28) and set +0x18;
 * then install the new pointer. USA twin GuiElementShareScaleVec (USA pool-free
 * is func_00337D78). */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; beql/bnel early-out +
   8-byte-packed callee-save frame wall. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337B90);
#else
extern void func_00338C28(void *pool, void **node);
void func_00337B90(GuiElement *e, f32 *newScale) {
    if (newScale == e->scale) {
        return;
    }
    if (*(void **)((char *)e + 0x2C) != 0 && *(s32 *)((char *)e + 0x18) == 0) {
        func_00338C28(*(void **)((char *)e + 0x2C), (void **)e->scale);
        *(s32 *)((char *)e + 0x18) = 1;
    }
    e->scale = newScale;
}
#endif

/* func_00337BF8: rebind the element's visibility-scalar vector (+0x10) to an
 * externally-owned vector. Same shape as func_00337B90 with the +0x10/+0x24
 * field/latch pair. USA twin func_00336D28. */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; same beql/bnel early-out
   + 8-byte-packed callee-save frame wall as func_00337B90. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337BF8);
#else
void func_00337BF8(GuiElement *e, f32 *newVisible) {
    if (newVisible == e->visible) {
        return;
    }
    if (*(void **)((char *)e + 0x2C) != 0 && *(s32 *)((char *)e + 0x24) == 0) {
        func_00338C28(*(void **)((char *)e + 0x2C), (void **)e->visible);
        *(s32 *)((char *)e + 0x24) = 1;
    }
    e->visible = newVisible;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337C68);

/* func_00337C90: install the base vtable at +0x30, return e.
 * USA GuiElementInstallBaseVtable. */
GuiElement *func_00337C90(GuiElement *e) {
    *(void **)((char *)e + 0x30) = &D_1ADAF8;
    return e;
}

/* func_00337CA8: construct the shared base of a GUI element (USA GuiElementBaseInit).
 * When a pool is supplied (a2), stash it at +0x2C and carve five zeroed 16-byte
 * vector blocks from it (GuiPoolAlloc + GuiPlacementNew), wiring them into the
 * element in allocation order at +0x0 (pos), +0x8 (unk08), +0x4 (scale), +0xC
 * (color), +0x10 (visible). Then record the element tag (a1) at +0x28, zero the
 * scalar fields +0x14/+0x18/+0x1C/+0x20/+0x24, and mark the element visible. */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; the same 8-byte-packed
   callee-save frame wall + per-block store scheduling as USA GuiElementBaseInit. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337CA8);
#else
void func_00337CA8(GuiElement *e, s32 tag, GuiPool *pool) {
    *(GuiPool **)((char *)e + 0x2C) = pool;
    if (pool != 0) {
        e->pos    = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        e->pos[0] = 0.0f; e->pos[1] = 0.0f; e->pos[2] = 0.0f; e->pos[3] = 0.0f;
        e->unk08  = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        e->unk08[0] = 0.0f; e->unk08[1] = 0.0f; e->unk08[2] = 0.0f; e->unk08[3] = 0.0f;
        e->scale  = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        e->scale[0] = 0.0f; e->scale[1] = 0.0f; e->scale[2] = 0.0f; e->scale[3] = 0.0f;
        e->color  = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        e->color[0] = 0; e->color[1] = 0; e->color[2] = 0; e->color[3] = 0;
        e->visible = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        e->visible[0] = 0.0f; e->visible[1] = 0.0f; e->visible[2] = 0.0f; e->visible[3] = 0.0f;
    }
    *(s32 *)((char *)e + 0x28) = tag;
    *(s32 *)((char *)e + 0x20) = 0;
    *(s32 *)((char *)e + 0x14) = 0;
    *(s32 *)((char *)e + 0x1C) = 0;
    *(s32 *)((char *)e + 0x18) = 0;
    *(s32 *)((char *)e + 0x24) = 0;
    func_00337B48(e, 1);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337DD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337DD8);

/* func_00337E88: TypeB GuiElement ctor. Install the base GuiElement vtable via
 * func_00337C90, then overwrite the +0x30 vtable slot with the TypeB vtable
 * D_1ADAD8. Returns the element (discarded by callers). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * GuiElementInitTypeB: base-install -> func_00337C90; TypeB vtable D_1ADA38 ->
 * D_1ADAD8; store offset +0x30. (EU .s retains the base-vtable call the USA #else
 * elided; modeled faithfully.) Fleet-pinned sig void(void*). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337E88);
#else
extern void *D_1ADAD8;
void func_00337E88(void *p) {
    func_00337C90((GuiElement *)p);
    *(void **)((char *)p + 0x30) = &D_1ADAD8;
}
#endif

/* func_00337EC0: GuiElement init. Run the base init func_00337CA8, then, when a
 * pool is present (+0x2C != 0), carve two zeroed 16-byte vector blocks (GuiPoolAlloc
 * + GuiPlacementNew) into +0x34 and +0x38, and set the scale vector to {1,1,1}.
 * +0x48 is zeroed on both paths, +0x44 at the shared tail. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA GuiElementInit:
 * GuiElementBaseInit -> func_00337CA8; GuiPoolAlloc/GuiPlacementNew KEPT; offsets
 * +0x2C/+0x34/+0x38/+0x44/+0x48/+0x4 + const 1.0 identical. Fleet-pinned sig
 * void(void* elem, void* tmpl, void* pool); tmpl->(s32) tag, pool->(GuiPool*). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337EC0);
#else
void func_00337EC0(void *elem, void *tmpl, void *pool) {
    GuiElement *e = (GuiElement *)elem;
    func_00337CA8(e, (s32)tmpl, (GuiPool *)pool);
    if (*(GuiPool **)((char *)e + 0x2C) == 0) {
        *(s32 *)((char *)e + 0x48) = 0;
    } else {
        f32 *blk0, *blk1, *scale;
        blk0 = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        *(f32 **)((char *)e + 0x34) = blk0;
        blk0[0] = 0.0f; blk0[1] = 0.0f; blk0[2] = 0.0f; blk0[3] = 0.0f;
        blk1 = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        *(f32 **)((char *)e + 0x38) = blk1;
        blk1[0] = 0.0f; blk1[1] = 0.0f; blk1[2] = 0.0f; blk1[3] = 0.0f;
        scale = *(f32 **)((char *)e + 0x4);
        scale[0] = 1.0f;
        scale = *(f32 **)((char *)e + 0x4);
        scale[1] = 1.0f;
        scale = *(f32 **)((char *)e + 0x4);
        scale[2] = 1.0f;
        *(s32 *)((char *)e + 0x48) = 0;
    }
    *(s32 *)((char *)e + 0x44) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337F68);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00337FE8);

/* func_00338000: GuiSprite element draw. If the element is visible
 * (*(e->visible) != 0.0) and has a live texture handle (+0x40), submit the sprite
 * to the 2D blitter func_00301AC0: handle, color[0] (via *(e+0xC)), scale and the
 * +0x38 vec as pointer args, and position [0..1], scale[0], scale[1]*y-fudge
 * (g_nVendorBuyQuantity+0x160), vec38[0] as floats. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA GuiSpriteElementDraw: blitter
 * func_003017F8 -> func_00301AC0; y-fudge g_swapGadgetItemIndex+0x8E ->
 * g_nVendorBuyQuantity+0x160; offsets/arg-order identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338000);
#else
extern s32 g_nVendorBuyQuantity;
extern void func_00301AC0(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                          f32 px, f32 py, f32 sx, f32 syg, f32 v38);
void func_00338000(void *p) {
    GuiElement *e = (GuiElement *)p;
    s32 handle;
    f32 *pos, *scale, *vec38;
    s32 *color;
    if (e->visible[0] == 0.0f) {
        return;
    }
    handle = *(s32 *)((char *)e + 0x40);
    if (handle == 0) {
        return;
    }
    pos = e->pos;
    scale = e->scale;
    color = *(s32 **)((char *)e + 0xC);
    vec38 = *(f32 **)((char *)e + 0x38);
    func_00301AC0(handle, color[0], scale, vec38,
                  pos[0], pos[1], scale[0],
                  scale[1] * *(f32 *)((char *)&g_nVendorBuyQuantity + 0x160),
                  vec38[0]);
    __asm__ __volatile__("");
}
#endif

/* func_00338070: GuiElement set-glyph. Look up the glyph for (atlas, code) via
 * func_00338AA8 and store the resulting handle at +0x40. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA GuiElementSetGlyph:
 * lookup GuiFontAtlasLookupGlyph -> func_00338AA8; store offset +0x40. func_00338AA8
 * defined later in-file, re-declared inline. Fleet-pinned sig void(void* e, u8* atlas,
 * s32 code). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338070);
#else
struct GuiFontAtlas;
extern s32 func_00338AA8(struct GuiFontAtlas *atlas, s32 codepoint);
void func_00338070(void *e, u8 *atlas, s32 code) {
    *(s32 *)((char *)e + 0x40) =
        func_00338AA8((struct GuiFontAtlas *)atlas, code);
}
#endif

/* func_003380A0: write the alpha float through the element's +0x38 ptr.
 * USA GuiElementSetAlpha. */
void func_003380A0(GuiElement *e, f32 alpha) {
    **(f32 **)((char *)e + 0x38) = alpha;
}

/* func_003380B0: GuiListRow element ctor. Install the base GuiElement vtable via
 * func_00337C90, then overwrite the +0x30 vtable slot with the GuiListRow vtable
 * D_1ADA78. Returns the element (discarded by callers). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * GuiListRowElementInit: base-install -> func_00337C90; list-row vtable
 * g_GuiListRowVtable -> D_1ADA78; store offset +0x30. Fleet-pinned sig void(void*). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003380B0);
#else
void func_003380B0(void *p) {
    GuiElement *e = (GuiElement *)p;
    func_00337C90(e);
    *(void **)((char *)e + 0x30) = &D_1ADA78;
}
#endif

/* func_003380E8: run the base GuiElement init using the 4th/5th args as its
 * tag/pool, then store the 2nd arg at +0x3C and the 3rd at +0x34, seed the
 * sentinel word +0x44 = 0x80000000 and the count/limit word +0x40 = 100.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA GuiListElementInit: GuiElementBaseInit->func_00337CA8 (EU jal), offsets
 * +0x3C/+0x34/+0x44/+0x40 and consts 0x80000000/0x64 identical (EU raw imms). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003380E8);
#else
void func_003380E8(GuiElement *e, s32 v3C, s32 v34, s32 tag, GuiPool *pool) {
    func_00337CA8(e, tag, pool);
    *(s32 *)((char *)e + 0x3C) = v3C;
    *(s32 *)((char *)e + 0x34) = v34;
    *(s32 *)((char *)e + 0x44) = (s32)0x80000000;
    *(s32 *)((char *)e + 0x40) = 0x64;
}
#endif

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

/* func_003381A8: write the high (alpha) byte of color words [0] and [1] of the
 * element's color block (+0xC), preserving the low 24 RGB bits.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA func_003372D0: color block at +0xC, mask 0xFFFFFF, shift 24, words [0]/[1];
 * no called symbols (pure loads/stores) - no retargets. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003381A8);
#else
void func_003381A8(GuiElement *e, s32 a0, s32 a1) {
    s32 *c = e->color;
    c[0] = (c[0] & 0xFFFFFF) | (a0 << 24);
    c = e->color;
    c[1] = (c[1] & 0xFFFFFF) | (a1 << 24);
}
#endif

/* func_003381E8: write the high (alpha) byte of color words [2] and [3] of the
 * element's color block (+0xC), preserving the low 24 RGB bits.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA func_00337310: color block at +0xC, mask 0xFFFFFF, shift 24, words [2]/[3]
 * (EU raw offsets 0x8/0xC on the +0xC block); no called symbols - no retargets. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003381E8);
#else
void func_003381E8(GuiElement *e, s32 a0, s32 a1) {
    s32 *c = e->color;
    c[2] = (c[2] & 0xFFFFFF) | (a0 << 24);
    c = e->color;
    c[3] = (c[3] & 0xFFFFFF) | (a1 << 24);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338228);

/* func_003383B0: run the base GuiElement vtable install then overwrite the +0x30
 * vtable slot with the D_1ADAB8 vtable; return the object.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA func_003374D8: GuiElementInstallBaseVtable->func_00337C90 (EU jal),
 * D_1ADA18->D_1ADAB8 (EU %hi/%lo, data-lane delta), store offset +0x30. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003383B0);
#else
extern void *D_1ADAB8;
void *func_003383B0(void *p) {
    func_00337C90((GuiElement *)p);
    *(void **)((char *)p + 0x30) = &D_1ADAB8;
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003383E8);

/* func_003383F0: run the base GuiElement init (forwarding tag/pool unchanged),
 * then when a pool is present carve a zeroed 16-byte vector block from the
 * element's +0x2C pool into +0x34. Always: zero the +0x34 vec's [0],[1] words,
 * seed the position vector (e->pos) to {100.0, 100.0} and the scale vector
 * (e->scale) to {64.0, 64.0}, and clear +0x38.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA GuiSpriteElementInit: GuiElementBaseInit->func_00337CA8, GuiPoolAlloc and
 * GuiPlacementNew KEPT (named, EU jal), consts 0x42C80000=100.0/0x42800000=64.0,
 * offsets +0x2C/+0x34/+0x38 identical (EU raw imms). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003383F0);
#else
void func_003383F0(GuiElement *e, s32 tag, GuiPool *pool) {
    f32 *vec, *pos, *scale;
    func_00337CA8(e, tag, pool);
    if (pool != 0) {
        vec = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        *(f32 **)((char *)e + 0x34) = vec;
        vec[0] = 0.0f; vec[1] = 0.0f; vec[2] = 0.0f; vec[3] = 0.0f;
    }
    vec = *(f32 **)((char *)e + 0x34);
    vec[0] = 0.0f;
    vec = *(f32 **)((char *)e + 0x34);
    vec[1] = 0.0f;
    pos = e->pos;
    pos[0] = 100.0f;
    pos = e->pos;
    pos[1] = 100.0f;
    scale = e->scale;
    scale[0] = 64.0f;
    scale = e->scale;
    scale[1] = 64.0f;
    *(s32 *)((char *)e + 0x38) = 0;
}
#endif

/* func_00338498: return the sprite's texture-vec pointer (+0x34).
 * USA GuiSpriteGetTextureVec. */
f32 *func_00338498(GuiElement *e) {
    return *(f32 **)((char *)e + 0x34);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003384A0);

/* func_00338508: draw one GUI sprite element (EU twin of USA func_00337630).
 * Bails if the element is hidden (*(e->0x10) == 0). When the global tint
 * override g_guiTintEnabled is set, the sprite's primitive colour word
 * (*(e->0xC) -> [0]) has its low 24 bits (RGB) replaced from g_guiTintRgb,
 * keeping the top 8 (alpha); that word is replicated across a 4-vertex stack
 * quad, the texture coords (*(e->0x34) -> [0,1]) are converted to int via
 * func_0028EE08, and the quad is emitted through func_0028F2D8 using the
 * element's screen position (e->0x0) and scale (e->0x4). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338508);
#else
/* WALL + LIVE-STATE: functional-equivalent #else routed to tester-EE (drives the
   live GS quad-emit path, reads the g_guiTint* render globals; bc1tl early-out,
   raw cvt.w.s, %gp_rel tint loads). */
extern s32 g_guiTintEnabled;          /* D_1ADA6C: non-zero -> apply RGB tint */
extern u32 g_guiTintRgb;              /* D_1ADA70: 0x00RRGGBB tint colour */
extern s32 func_0028EE08(s32 u, s32 v);
extern void func_0028F2D8(s32 handle, s32 x0, s32 y0, s32 x1, s32 y1, void *quad);
void func_00338508(GuiElement *e) {
    f32 *primColorBlock;
    f32 *texCoord;
    f32 *pos;
    f32 *scale;
    s32 quad[4];
    s32 handle;

    if (*e->visible == 0.0f) {
        return;
    }
    if (g_guiTintEnabled) {
        u32 *prim = *(u32 **)((char *)e + 0x0C);
        *prim = (*prim & 0xFF000000) | (g_guiTintRgb & 0x00FFFFFF);
    }
    primColorBlock = *(f32 **)((char *)e + 0x0C);
    texCoord = *(f32 **)((char *)e + 0x34);
    quad[0] = *(s32 *)primColorBlock;
    quad[1] = *(s32 *)primColorBlock;
    quad[2] = *(s32 *)primColorBlock;
    quad[3] = *(s32 *)primColorBlock;
    handle = func_0028EE08((s32)texCoord[0], (s32)texCoord[1]);
    scale = *(f32 **)((char *)e + 0x04);
    pos = *(f32 **)((char *)e + 0x00);
    func_0028F2D8(handle, (s32)pos[0], (s32)pos[1], (s32)scale[0], (s32)scale[1], quad);
}
#endif

/* func_00338600: write two int->float coords (cvt.s.w) through e+0x34,
 * re-reading the +0x34 pointer per store; lands at (e+0x34)[0],[1].
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA GuiSpriteSetTexture: vec pointer at +0x34, stores [0]/[1]; no called
 * symbols (pure mtc1/cvt/swc1) - no retargets. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338600);
#else
void func_00338600(GuiElement *e, s32 u, s32 v) {
    f32 fu = (f32)u;
    f32 fv = (f32)v;
    (*(f32 **)((char *)e + 0x34))[0] = fu;
    (*(f32 **)((char *)e + 0x34))[1] = fv;
}
#endif

/* func_00338630: return the first float of the vector at p+0x34 converted to
 * int (cvt.w.s). USA func_00337758. */
s32 func_00338630(void *p) {
    f32 *vec = *(f32 **)((char *)p + 0x34);
    return (s32)vec[0];
}

/* func_00338648: install the base GuiElement vtable then overwrite the +0x30
 * vtable slot with the TypeC vtable (D_1ADA98); return value (p) unused by
 * callers, so the signature is void.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA GuiElementInitTypeC asm (NOT its simplified #else): the EU asm is op-for-op
 * identical to the USA GuiElementInitTypeC asm - GuiElementInstallBaseVtable->
 * func_00337C90 (EU jal), D_1AD9F8->D_1ADA98 (EU %hi/%lo, data-lane delta), store
 * offset +0x30. FLEET-PIN signature void(void*). D_1ADA98 is file-scope. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338648);
#else
void func_00338648(void *p) {
    func_00337C90((GuiElement *)p);
    *(void **)((char *)p + 0x30) = &D_1ADA98;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338680);

/* func_00338688: run the base GuiElement init (func_00337CA8, forwarding its own
 * tag/pool args unchanged), then set up a text element: install the D_2638D0
 * glyph/format table at +0x34, mark active (+0x38 = (s64)1), clear text handle
 * (+0x40), reset scale vector (e->scale) to {1.0, 1.0}, and seed text params:
 * +0x4C = 0x200, +0x50 = 0.7f, +0x44 = 1, +0x48 = 0.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA GuiTextElementInit: GuiElementBaseInit -> func_00337CA8
 * (EU jal), D_263B10 -> D_2638D0 (EU %hi/%lo), offsets +0x34/+0x38/+0x40/scale/
 * +0x4C=0x200/+0x50=0.7f/+0x44=1/+0x48=0.
 * REGION DELTA: USA also writes +0x54 = 1 (sw at 0x54); the EU build OMITS the
 * +0x54 store entirely - modeled faithfully (no +0x54 write here). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338688);
#else
extern void *D_2638D0;
void func_00338688(GuiElement *e, s32 tag, GuiPool *pool) {
    f32 *scale;
    func_00337CA8(e, tag, pool);
    *(void **)((char *)e + 0x34) = &D_2638D0;
    *(s64 *)((char *)e + 0x38) = 1;
    *(s32 *)((char *)e + 0x40) = 0;
    scale = e->scale;
    scale[0] = 1.0f;
    scale = e->scale;
    scale[1] = 1.0f;
    *(s32 *)((char *)e + 0x4C) = 0x200;
    *(f32 *)((char *)e + 0x50) = 0.7f;
    *(s32 *)((char *)e + 0x44) = 1;
    *(s32 *)((char *)e + 0x48) = 0;
}
#endif

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

/* func_003388F0: position the list's scroll-thumb (EU twin of USA
 * GuiListSetScrollPos). Clamp the requested row `pos` to the total row count
 * (+0x40), then write through the +0x04 scale-vector pointer
 *   thumb = (clamp / totalRows) * trackLength       (trackLength = +0x3C)
 * collapsing to 0 when the list has zero rows. All int->float conversions are
 * unsigned. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003388F0);
#else
/* WALL: functional-equivalent #else; matching arm stays INCLUDE_ASM (movz clamp,
   (f32)(u32) unsigned-conversion idiom, +0x40 materialised twice). */
void func_003388F0(GuiElement *e, s32 pos) {
    s32 totalRows = *(s32 *)((char *)e + 0x40);
    s32 trackLength = *(s32 *)((char *)e + 0x3C);
    s32 clamp = totalRows;
    if ((u32)totalRows >= (u32)pos) {
        clamp = pos; /* clamp = min(totalRows, pos), unsigned */
    }
    if ((f32)(u32)totalRows == 0.0f) {
        e->scale[0] = 0.0f;
        return;
    }
    e->scale[0] = ((f32)(u32)clamp / (f32)(u32)totalRows) * (f32)(u32)trackLength;
}
#endif

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

/* Font atlas glyph table (EU twin of USA GuiFontAtlas): glyph count at +0x18,
 * inline {codepoint, value} pairs from +0x1C. The `value` field doubles as a
 * load-relative offset that func_00338A40 rebases to an absolute pointer. */
typedef struct GuiFontGlyph {
    /* 0x00 */ s32 codepoint;
    /* 0x04 */ s32 value;
} GuiFontGlyph;
typedef struct GuiFontAtlas {
    /* 0x00 */ char header[0x18];
    /* 0x18 */ s32 glyphCount;
    /* 0x1C */ GuiFontGlyph glyphs[1];
} GuiFontAtlas;

/* func_00338A40: rebase each glyph's `value` field from a load-relative offset
 * to an absolute pointer (atlas base + offset), handing each to func_00301770.
 * USA twin GuiFontAtlasRelocate. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338A40);
#else
/* WALL: functional-equivalent #else (4-GPR-save 8-byte vs 16-byte slot stride);
   matching arm stays INCLUDE_ASM. */
extern void func_00301770(void *p);
void func_00338A40(GuiFontAtlas *atlas) {
    s32 count = atlas->glyphCount;
    GuiFontGlyph *g = atlas->glyphs;
    s32 i;
    for (i = 0; i < count; i++) {
        g[i].value = (s32)((char *)g[i].value + (s32)atlas);
        func_00301770((void *)g[i].value);
    }
}
#endif

/* func_00338AA8: linear-scan the atlas glyph table for the entry whose codepoint
 * matches `codepoint`, returning its glyph value (0 if not found). Hoisting the
 * glyph base into a local pointer keeps the index-addressed scan the original
 * uses. USA twin GuiFontAtlasLookupGlyph - byte-exact MATCH. */
s32 func_00338AA8(GuiFontAtlas *atlas, s32 codepoint) {
    s32 count = atlas->glyphCount;
    GuiFontGlyph *glyphs = atlas->glyphs;
    s32 i = 0;
    s32 result = 0;
    while (i < count) {
        if (glyphs[i].codepoint == codepoint) {
            result = glyphs[i].value;
            break;
        }
        i++;
    }
    return result;
}

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

/* rodata assert strings shared by the GUI pool routines (EU symbols):
 *   D_1ADB18 = source file path, D_1ADB38 = elemKind-bound predicate (GuiPoolInit),
 *   D_1ADB60 = capacity predicate (GuiPoolAlloc). */
extern const char D_1ADB18[];
extern const char D_1ADB38[];
extern const char D_1ADB60[];

/* GuiPoolInit: initialise a GUI fixed-size pool header (EU twin of USA
 * GuiPoolInit): base = storage (a2), capacity = byteLimit (a3), elemSize =
 * elemSize (a1, asserted >= 4 - a node must hold the free-list next ptr),
 * cursor/count/freeList zeroed. The asm `sltiu a1,4; beqz->stores` fires the
 * assert for elemSize in [0,4) (same inverted-looking guard as USA). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", GuiPoolInit);
#else
/* WALL: functional-equivalent #else (4-GPR-save 8-byte vs 16-byte slot stride);
   matching arm stays INCLUDE_ASM. Twin of USA GuiPoolInit; the assert-direction
   bug (was `>= 4`) was caught by the USA cmp-oracle (cmp_GuiPoolInit) and fixed
   identically here. */
void GuiPoolInit(GuiPool *pool, s32 elemSize, void *storage, u32 byteLimit) {
    if ((u32)elemSize < 4) {
        AssertFail(D_1ADB18, 0x25, D_1ADB38);
    }
    pool->base = (char *)storage;
    pool->capacity = byteLimit;
    pool->elemSize = elemSize;
    pool->count = 0;
    pool->freeList = 0;
    pool->cursor = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338B90);

/* GuiPoolAlloc: allocate one node from a GUI fixed-size pool (EU twin of USA
 * GuiPoolAlloc). See the USA unit for the field map. Pop the free list head if
 * present, else bump-allocate base+cursor; capacity overflow trips an assert. */
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

/* func_00338C50: bounds-checked lookup into the 4-entry small-data table
 * D_1ADB78 (idx < 4 ? D_1ADB78[idx] : 0).
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00337DA0: table D_1ADAD8 -> D_1ADB78 (EU %gp_rel).
 * (TODO(match): original loads the base via one-insn %gp_rel($28); cc1 emits the
 * two-insn absolute %hi/%lo for the indexed array base under -G8.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338C50);
#else
extern s32 D_1ADB78[4];
s32 func_00338C50(u32 idx) {
    if (idx < 4) {
        return D_1ADB78[idx];
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338C78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00338F68);

/* func_00339930: init two embedded type-B GUI elements (func_00337E88 at p+0x10
 * and p+0x5C), return the object.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00338A80: GuiElementInitTypeB -> func_00337E88 (EU
 * jal), offsets +0x10/+0x5C, return p. func_00337E88 reused (defined earlier in
 * this unit). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339930);
#else
void *func_00339930(void *p) {
    func_00337E88((char *)p + 0x10);
    func_00337E88((char *)p + 0x5C);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339968);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339B80);

/* func_00339B88: configure a scrolling-list widget.
 *  - Store row-count (a1) at +0x1C4 and page-size (a2) at +0x1B8.
 *  - Seed the widget pos vector (p+0x0) with (x,y), clear the +0x1A7 byte
 *    flag, set the +0x1B0 "active" flag.
 *  - Build the embedded sub-list at p+0xA8: record its address at +0x1D8, run
 *    func_0027F660(p+0xA8, -1) and keep its handle at +0x1D4.
 *  - Initialise the widget scale vector (p+0x4) x-component to 1.0.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00338CD8: func_0027F7F8 -> func_0027F660 (EU jal),
 * offsets +0x1C4/+0xA8/+0x1A7/+0x1B8/+0x1D8/+0x1D4/+0x1B0, scale.x = 1.0f. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339B88);
#else
extern s32 func_0027F660(void *p, s32 mode);
void func_00339B88(void *p, s32 a1, f32 x, f32 y, s32 a2) {
    void *sub = (char *)p + 0xA8;
    *(s32 *)((char *)p + 0x1C4) = a1;
    (*(f32 **)p)[0] = x;
    (*(f32 **)p)[1] = y;
    *(u8 *)((char *)p + 0x1A7) = 0;
    *(s32 *)((char *)p + 0x1B8) = a2;
    *(s32 *)((char *)p + 0x1D8) = (s32)sub;
    *(s32 *)((char *)p + 0x1D4) = func_0027F660(sub, -1);
    *(s32 *)((char *)p + 0x1B0) = 1;
    (*(f32 **)((char *)p + 0x4))[0] = 1.0f;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339BF8);

/* func_00339DC8: init a widget sub-block: copy a 0xFF-byte template into p+0x2084
 * (func_00115AC0), then run the setup helper func_00339BF8 on p+0x1FDC with kind
 * 2, a caller id (or the default when id == -1), and fixed 255.0/80.0 params.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00338F18: func_00338D48 -> func_00339BF8 (EU jal),
 * func_00115AC0 KEPT, offsets p+0x2084 / 0xFF bytes / p+0x1FDC, params
 * 255.0f(0x437F0000)/80.0f(0x42A00000), kind literal 2.
 * REGION DELTA: default id when id == -1 is 0x12C on EU vs 0x168 on USA. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339DC8);
#else
extern void func_00115AC0(void *dst, const void *src, s32 n);
extern void func_00339BF8(void *p, s32 a1, f32 x, f32 y, s32 a2);
void func_00339DC8(void *p, const void *src, s32 id) {
    s32 kind = (id != -1) ? id : 0x12C;
    func_00115AC0((char *)p + 0x2084, src, 0xFF);
    func_00339BF8((char *)p + 0x1FDC, 2, 255.0f, 80.0f, kind);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339E30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00339E38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A248);

/* func_0033A4A0: format a localized caption into a widget sub-block: sprintf the
 * text of localized string strId (used as the format) with fmtArg into p+0x1E34
 * (func_00115DA8), then run the setup helper func_00339B88 on p+0x1D8C with kind
 * 2, a caller id (or the default when id == -1), and fixed 255.0/135.0 params.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_003395F0: func_00338CD8 -> func_00339B88 (EU jal),
 * GetLocalizedString and func_00115DA8 KEPT, offsets p+0x1E34 / p+0x1D8C, params
 * 255.0f(0x437F0000)/135.0f(0x43070000), kind literal 2.
 * REGION DELTA: default id when id == -1 is 0x64 on EU vs 0x78 on USA. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A4A0);
#else
extern char *GetLocalizedString(s32 id);
extern int func_00115DA8(char *dst, const char *fmt, ...); /* SDK sprintf (char count); matches USA - variadic */
void func_0033A4A0(void *p, s32 strId, s32 fmtArg, s32 id) {
    s32 kind = (id != -1) ? id : 0x64;
    func_00115DA8((char *)p + 0x1E34, GetLocalizedString(strId), fmtArg);
    func_00339B88((char *)p + 0x1D8C, 2, 255.0f, 135.0f, kind);
}
#endif

/* func_0033A528: sibling of func_00339DC8 - copy a 0xFE-byte template into
 * p+0x1E34 (func_00115AC0), then run func_00339BF8 on p+0x1D8C with kind 2, a
 * caller id (or the default when id == -1), and fixed 255.0/123.0 params.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00339678: func_00338D48 -> func_00339BF8 (EU jal),
 * func_00115AC0 KEPT, offsets p+0x1E34 / 0xFE bytes / p+0x1D8C, params
 * 255.0f(0x437F0000)/123.0f(0x42F60000), kind literal 2.
 * REGION DELTA: default id when id == -1 is 0x96 on EU vs 0xB4 on USA. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A528);
#else
extern void func_00115AC0(void *dst, const void *src, s32 n);
extern void func_00339BF8(void *p, s32 a1, f32 x, f32 y, s32 a2);
void func_0033A528(void *p, const void *src, s32 id) {
    s32 kind = (id != -1) ? id : 0x96;
    func_00115AC0((char *)p + 0x1E34, src, 0xFE);
    func_00339BF8((char *)p + 0x1D8C, 2, 255.0f, 123.0f, kind);
}
#endif

void func_0033A590(void) {
}

/* func_0033A598: map a small selector to a scale constant - selector 0 -> 0.7,
 * 1 -> 0.8, anything else -> 1.0. The first argument is ignored. Matching arm
 * stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_003396E8: leaf, no called symbols; float consts 0x3F4CCCCD=0.8 (sel==1),
 * 0x3F333333=0.7 (sel==0), 0x3F800000=1.0 (default) identical (EU raw imms). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A598);
#else
/* TODO(match): functional equivalent - not byte-exact; multi-way branch +
   float-constant materialization wall (same as USA func_003396E8). */
f32 func_0033A598(s32 unused, s32 sel) {
    if (sel == 1) {
        return 0.8f;
    }
    if (sel == 0) {
        return 0.7f;
    }
    return 1.0f;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A5F0);

void func_0033A930(void) {
}

/* func_0033A938: refresh a widget with a live sub-value (*(w+0x4)[0] != 0):
 * dispatch on its mode (+0x1C8) - mode 0 runs the sub-updater func_0033A5F0,
 * mode 2 the (empty) func_0033A930 - then always run the (empty) tail
 * func_0033A590. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00339A88: mode-0 sub-updater func_00339740 ->
 * func_0033A5F0; mode-2 stub func_00339A80 -> func_0033A930; tail stub
 * func_003396E0 -> func_0033A590; gate field +0x4, mode field +0x1C8 identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A938);
#else
extern void func_0033A5F0(void *w);
void func_0033A938(void *w) {
    f32 *p = *(f32 **)((char *)w + 0x4);
    if (p[0] == 0.0f) {
        return;
    }
    switch (*(s32 *)((char *)w + 0x1C8)) {
    case 0: func_0033A5F0(w); break;
    case 2: func_0033A930();  break;
    default: break;
    }
    func_0033A590();
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033A9B0);

void func_0033AE40(void) {
}

/* func_0033AE48: temporarily override a camera-projection parameter, rebuild the
 * projection, run one per-mode scene update, then restore the parameter and
 * rebuild again. *(w+0x4) is a curve/animator whose first float gates the whole
 * op: if it is 0 the widget is inactive and nothing happens. The overridden
 * parameter is a float at (g_nVendorBuyQuantity + 0x6DF8) + 0xB0 (an unnamed
 * projection sub-struct); it is forced to 0.62 for the duration. Mode = *(w+0x1C8):
 * 0 -> func_0033A9B0(w) (the full per-frame update); 2 -> func_0033AE40() (stub
 * hook); any other value -> no update. Matching arm stays INCLUDE_ASM; #else is
 * the structure model. Word-verified vs USA func_00339F98: cam-param base
 * g_sceneActorMobys+0x674 -> g_nVendorBuyQuantity+0x6DF8 (EU %hi/%lo), sub-offset
 * +0xB0 identical; BuildCameraProjection -> func_0027AF28; mode-0 update
 * func_00339B00 -> func_0033A9B0; mode-2 stub func_00339F90 -> func_0033AE40;
 * const 0.62=0x3F1EB852 identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033AE48);
#else
extern s32 g_nVendorBuyQuantity;
extern void func_0027AF28(void);
extern void func_0033A9B0(void *w);
void func_0033AE48(void *w) {
    f32 *camParam;
    f32 saved;
    s32 mode;

    if (*(f32 *)(*(void **)((char *)w + 0x4)) == 0.0f) {
        return;
    }
    camParam = (f32 *)((char *)&g_nVendorBuyQuantity + 0x6DF8 + 0xB0);
    saved = *camParam;
    *camParam = 0.62f; /* 0x3F1EB852 */
    func_0027AF28();

    mode = *(s32 *)((char *)w + 0x1C8);
    if (mode == 0) {
        func_0033A9B0(w);
    } else if (mode == 2) {
        func_0033AE40();
    }

    *camParam = saved;
    func_0027AF28();
}
#endif

/* func_0033AEF8: init the screen's eight embedded sub-elements at their fixed
 * offsets (four TypeB then four TypeC), return the object. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0033A048:
 * GuiElementInitTypeB -> func_00337E88 (offsets +0x8/+0x54/+0xA0/+0xEC);
 * GuiElementInitTypeC -> func_00338648 (offsets +0x138/+0x190/+0x1E8/+0x240). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033AEF8);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall (same as USA func_0033A048). */
void *func_0033AEF8(void *p) {
    func_00337E88((char *)p + 0x8);
    func_00337E88((char *)p + 0x54);
    func_00337E88((char *)p + 0xA0);
    func_00337E88((char *)p + 0xEC);
    func_00338648((char *)p + 0x138);
    func_00338648((char *)p + 0x190);
    func_00338648((char *)p + 0x1E8);
    func_00338648((char *)p + 0x240);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033AF60);

/* func_0033B218 = GuiConfirmPopupTick: per-frame confirm-popup layout, then bump
 * the current cutscene unlock record. The four icon rows (+0x8/+0x54/+0xA0/+0xEC)
 * sit at the placement record origin (*(w+0x4)); the three text rows
 * (+0x138/+0x190/+0x1E8) are offset by the fixed (x,y) pairs
 * D_1ADCD8/D_1ADCE0/D_1ADCE8. func_002DFFC8 refreshes the in-place level-name
 * buffer at +0x298 for g_mapCurrentLevel. The tail touches the per-cutscene unlock
 * block at (g_health+0x464), slot +0x1C8/+0x1CC/+0x1D0: while g_gameTime>=11 it
 * increments the saturating u16 play-count at +0x1C8; it raises the u32 high-water
 * mark at +0x1CC; and sets this g_playerProgress slot's bit (plus 0x80000000) in
 * the seen-mask at +0x1D0. arg2 is unused (ABI-uniform).
 *
 * Matching arm stays INCLUDE_ASM; #else is the structure model. TWIN NOTE: this is
 * the EU twin of USA GuiConfirmPopupTick (USA 0x33A368, delta +0xEB0), CONFIRMED
 * by EU symbol_addrs pin (func_0033B218 = GuiConfirmPopupTick); the mnemonic-hash
 * twin-map mis-swapped it with GuiConfirmPopupInit (the real EU GuiConfirmPopupInit
 * is func_0033AF60 @0x33AF68). Word-verified vs USA GuiConfirmPopupTick:
 * GuiElementSetPos -> func_00337B18; func_002E0010 -> func_002DFFC8; offset pairs
 * D_1ADC38/40/48 -> D_1ADCD8/E0/E8; g_gameTime -> g_nLevelExitDestination+0x8;
 * g_gsPixelOffsetY+0x3C source -> D_001A7308+0x108; g_health / g_mapCurrentLevel /
 * g_playerProgress KEPT.
 *
 * GENUINE REGION DIFF (PAL/NTSC): the EU build inserts a PAL 50Hz integer
 * frame-count averaging stage on the +0x1CC high-water field -
 * averaged = (source*4 + source + 2) / 6  (li 6; sll/addu/addiu; div; mflo) -
 * whereas USA NTSC uses the raw source directly (plain sltu/sw). Modeled
 * faithfully below; flagged in EU symbol_addrs line 1011. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B218);
#else
/* TODO(match): functional-equivalent structure model; the sp-staged row memsets +
   the PAL (source*5+2)/6 divide are the compiler's own materialization. */
extern void func_00337B18(void *e, f32 x, f32 y, f32 z, f32 w);
extern void func_002DFFC8(void *dst, s32 level);
extern f32 D_1ADCD8[2], D_1ADCE0[2], D_1ADCE8[2];
extern s32 g_health;                 /* 0x18C36C - base of per-cutscene unlock records at +0x464 */
extern s32 g_nLevelExitDestination;  /* 0x1B1680 - EU g_gameTime slot lives at +0x8 (0x1B1688) */
extern s32 g_mapCurrentLevel;        /* 0x1C51D0 - current map level id */
extern s32 D_001A7308;               /* PAL frame-count source at +0x108 */
extern s32 g_playerProgress;         /* 0x1A7A78 - current progress slot (seen-mask bit index) */
void func_0033B218(void *w, s32 arg2) {
    void *icon0 = (char *)w + 0x8;
    void *icon1 = (char *)w + 0x54;
    void *icon2 = (char *)w + 0xA0;
    void *icon3 = (char *)w + 0xEC;
    void *text0 = (char *)w + 0x138;
    void *text1 = (char *)w + 0x190;
    void *text2 = (char *)w + 0x1E8;
    f32 *origin = *(f32 **)((char *)w + 0x4);
    u8 *rec = (u8 *)&g_health + 0x464;
    s32 source;

    (void)arg2;

    func_00337B18(icon0, origin[0], origin[1], 0.0f, 0.0f);
    func_00337B18(icon1, origin[0], origin[1], 0.0f, 0.0f);
    func_00337B18(icon2, origin[0], origin[1], 0.0f, 0.0f);
    func_00337B18(icon3, origin[0], origin[1], 0.0f, 0.0f);
    func_00337B18(text0, D_1ADCD8[0] + origin[0], D_1ADCD8[1] + origin[1], 0.0f, 0.0f);
    func_00337B18(text1, D_1ADCE0[0] + origin[0], D_1ADCE0[1] + origin[1], 0.0f, 0.0f);
    func_00337B18(text2, D_1ADCE8[0] + origin[0], D_1ADCE8[1] + origin[1], 0.0f, 0.0f);

    func_002DFFC8((char *)w + 0x298, g_mapCurrentLevel);

    source = *(s32 *)((char *)&D_001A7308 + 0x108);
    if (*(s32 *)((char *)&g_nLevelExitDestination + 0x8) >= 11 &&
        *(u16 *)(rec + 0x1C8) <= 0xFFFE) {
        *(u16 *)(rec + 0x1C8) = (u16)(*(u16 *)(rec + 0x1C8) + 1);
    }
    /* REGION DIFF: PAL 50Hz frame-count averaging (source*4 + source + 2)/6. */
    source = (source * 4 + source + 2) / 6;
    if ((u32)*(s32 *)(rec + 0x1CC) < (u32)source) {
        *(s32 *)(rec + 0x1CC) = source;
    }
    *(u32 *)(rec + 0x1D0) |= (1u << g_playerProgress) | 0x80000000u;
}
#endif

/* func_0033B4B8 = GuiConfirmPopupDraw: draw a confirm popup when it's active
 * (+0x2D8 != 0): four sprite sub-elements (+0x8/+0x54/+0xA0/+0xEC) and three text
 * sub-elements (+0x138/+0x190/+0x1E8). Matching arm stays INCLUDE_ASM; #else is
 * the structure model. Word-verified vs USA GuiConfirmPopupDraw:
 * GuiSpriteElementDraw -> func_00338000; GuiTextElementDraw -> func_00338770;
 * +0x2D8 gate + offsets identical (region-shifted jal targets only). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B4B8);
#else
extern void func_00338770(void *e);
void func_0033B4B8(void *w) {
    if (*(s32 *)((char *)w + 0x2D8) != 0) {
        func_00338000((char *)w + 0x8);
        func_00338000((char *)w + 0x54);
        func_00338000((char *)w + 0xA0);
        func_00338000((char *)w + 0xEC);
        func_00338770((char *)w + 0x138);
        func_00338770((char *)w + 0x190);
        func_00338770((char *)w + 0x1E8);
    }
}
#endif

/* func_0033B520: init the embedded dialog-box (p+0x8) and a TypeB element
 * (p+0x2D8), return the object. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_0033A640: GuiDialogBoxInitElements
 * -> func_0033CA78; GuiElementInitTypeB -> func_00337E88; offsets +0x8/+0x2D8
 * identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B520);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall (same as USA func_0033A640). */
extern void func_0033CA78(void *p);
void *func_0033B520(void *p) {
    func_0033CA78((char *)p + 0x8);
    func_00337E88((char *)p + 0x2D8);
    return p;
}
#endif

/* func_0033B558: construct a bordered dialog/popup widget (EU twin of USA
 * func_0033A678). If a pool is given, allocate its 0x10-byte placement record
 * (+0x324) and zero it. Seed the panel element (+0x8) size to 256x212, init its
 * border art from D_1ADCF0, then set its bounds (64, -164, 10, 136, 10, 99). Init
 * the icon GuiElement (+0x2D8) from D_1ADD00, force its packed colour to
 * 0x70FFFEED, and give it glyph 0xD6 from the shared atlas (g_guiInstance+0x8710).
 * Finally run func_0033B6C0 (the show hook) and clear +0x330.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033A678: GuiDialogBoxInitBorder->func_0033CB00;
 * GuiDialogBoxSetBounds->func_0033CF58; GuiElementInit->func_00337EC0;
 * GuiElementGetColor->func_00337B00; GuiElementSetGlyph->func_00338070;
 * func_0033A7E0->func_0033B6C0; GuiPlacementNew/GuiPoolAlloc KEPT.
 * DATA +0xA0 lane: D_1ADC50->D_1ADCF0, D_1ADC60->D_1ADD00; g_guiInstance KEPT. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B558);
#else
extern void func_0033CB00(void *w, void *pool, void *borderCfg);
extern void func_0033CF58(void *p, f32 a, f32 b, f32 c, f32 d, f32 e, f32 f);
extern void func_0033B6C0(void *w);
extern char *g_guiInstance;
extern u8 D_1ADCF0[];
extern u8 D_1ADD00[];
void func_0033B558(void *w, GuiPool *pool) {
    GuiElement *icon = (GuiElement *)((char *)w + 0x2D8);
    void *obj;
    s32 *color;

    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x324) = obj;
        *(s32 *)((char *)obj + 0x0) = 0;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
    }

    obj = *(void **)((char *)w + 0x324);
    *(f32 *)((char *)obj + 0x0) = 256.0f;
    *(f32 *)((char *)obj + 0x4) = 212.0f;

    *(s32 *)((char *)w + 0x328) = 0;
    func_0033CB00((char *)w + 0x8, pool, D_1ADCF0);
    func_0033CF58((char *)w + 0x8, 64.0f, -164.0f, 10.0f, 136.0f, 10.0f, 99.0f);

    func_00337EC0(icon, D_1ADD00, pool);
    color = func_00337B00(icon);
    *color = 0x70FFFEED;
    func_00338070(icon, (u8 *)(g_guiInstance + 0x8710), 0xD6);

    func_0033B6C0(w);
    *(s32 *)((char *)w + 0x330) = 0;
}
#endif

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

/* func_0033B6C0: seed this confirm-dialog screen's text (EU twin of USA
 * func_0033A7E0) - set the dialog box (p+0x8) title/body/footer to localized
 * strings, zero its scale, and clear the +0x32C "text ready" flag (sibling of
 * func_0033B740, which raises that flag with the alternate title id).
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033A7E0: GuiDialogBoxSetText3->func_0033CF78;
 * GuiDialogBoxSetScale->func_0033CFD0; GetLocalizedString KEPT; offset p+0x8/+0x32C.
 * GENUINE REGION DIFF (localized string ids): title/body/footer
 * USA 0x2C34/0x2C32/0x2BE5 -> EU 0xBAF/0xBAD/0xB60. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B6C0);
#else
extern char *GetLocalizedString(s32 id);
extern void func_0033CF78(void *p, s32 t0, s32 t1, s32 t2);
extern void func_0033CFD0(void *p, f32 scale);
void func_0033B6C0(void *p) {
    s32 t0 = (s32)GetLocalizedString(0xBAF);
    s32 t1 = (s32)GetLocalizedString(0xBAD);
    s32 t2 = (s32)GetLocalizedString(0xB60);
    func_0033CF78((char *)p + 0x8, t0, t1, t2);
    func_0033CFD0((char *)p + 0x8, 0.0f);
    *(s32 *)((char *)p + 0x32C) = 0;
}
#endif

/* func_0033B740: populate this confirm-dialog screen's text (EU twin of USA
 * func_0033A860) - set the dialog box (p+0x8) title/body/footer to localized
 * strings, zero its scale, raise the +0x32C "text ready" flag, and return 1.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033A860: GuiDialogBoxSetText3->func_0033CF78;
 * GuiDialogBoxSetScale->func_0033CFD0; GetLocalizedString KEPT; offset p+0x8/+0x32C.
 * GENUINE REGION DIFF (localized string ids): title/body/footer
 * USA 0x2C33/0x2C32/0x2BE5 -> EU 0xBAE/0xBAD/0xB60. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B740);
#else
extern char *GetLocalizedString(s32 id);
extern void func_0033CF78(void *p, s32 t0, s32 t1, s32 t2);
extern void func_0033CFD0(void *p, f32 scale);
s32 func_0033B740(void *p) {
    s32 t0 = (s32)GetLocalizedString(0xBAE);
    s32 t1 = (s32)GetLocalizedString(0xBAD);
    s32 t2 = (s32)GetLocalizedString(0xB60);
    func_0033CF78((char *)p + 0x8, t0, t1, t2);
    func_0033CFD0((char *)p + 0x8, 0.0f);
    *(s32 *)((char *)p + 0x32C) = 1;
    return 1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B7C8);

/* func_0033B7D0: update a pulsing list-cursor widget (EU twin of USA
 * func_0033A8F0). Place its frame sub-element (+0x8) at the tracked anchor
 * (*(w+0x324)) via func_0033CF40 + func_0033CD50, tick the colour pulse when dpad
 * up/down is held (mask 0x5000), set the cursor sprite's (+0x2D8) colour to the
 * pulsed blend of 0x60442D00/0x70FFFEED, then position that sprite at the anchor
 * offset by fixed constants plus a per-frame-counter (w+0x328) Y step. Returns 0.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033A8F0: func_0033C060->func_0033CF40;
 * func_0033BE70->func_0033CD50; func_002AA3F0->func_002A9FA0;
 * GuiElementGetColor->func_00337B00; GuiElementSetPos->func_00337B18.
 * DATA +0xA0 lane: D_1ADC68->D_1ADD08, D_1ADC70->D_1ADD10, D_1ADC6C->D_1ADD0C;
 * g_padButtonsPressed KEPT. func_002A9FA0 takes 5 EABI int args (arg5 in $8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B7D0);
#else
extern s32 g_padButtonsPressed;
extern void func_0033CF40(void *e, f32 x, f32 y);
extern void func_0033CD50(void *e, s32 flags);
extern s32 func_002A9FA0(s32 a, s32 b, s32 c, s32 d, s32 e);
extern void func_00337B18(void *e, f32 x, f32 y, f32 z, f32 w);
extern f32 D_1ADD08, D_1ADD0C, D_1ADD10;
s32 func_0033B7D0(void *w) {
    void *sub = (char *)w + 0x8;
    f32 *anchor = *(f32 **)((char *)w + 0x324);
    s32 counter;
    f32 *a;

    func_0033CF40(sub, anchor[0], anchor[1]);
    func_0033CD50(sub, 0);   /* 2nd arg (flags) ignored by the callee */
    if (g_padButtonsPressed & 0x5000) {
        func_002A9FA0(0, 0, 1, 0, 1);
    }
    *func_00337B00((GuiElement *)((char *)w + 0x2D8)) =
        func_002A9FA0(0x60442D00, 0x70FFFEED, 0x14, 0, 0);

    counter = *(s32 *)((char *)w + 0x328);
    a = *(f32 **)((char *)w + 0x324);
    func_00337B18((GuiElement *)((char *)w + 0x2D8),
                  a[0] + D_1ADD08,
                  a[1] + (f32)counter * D_1ADD10 + D_1ADD0C,
                  0.0f, 0.0f);
    return 0;
}
#endif

/* func_0033B8D8: draw this confirm-dialog screen (EU twin of USA func_0033A9F8).
 * Sets the dialog box's (p+0x8) +0x2C4 footer-visible flag to 1 when any global
 * menu lock (D_1A8D38 / D_1A8D3C) is engaged or the local +0x330 flag is set;
 * additionally sets it to 1 (with arg 1) when the +0x32C "text ready" flag equals
 * 1. Then draws the box (func_0033CE70) and the background sprite at p+0x2D8.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033A9F8: func_0033BE68->func_0033CD48;
 * func_0033BF90->func_0033CE70; GuiSpriteElementDraw->func_00338000.
 * DATA +0xB0 lane: D_1A8C88->D_1A8D38, D_1A8C8C->D_1A8D3C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B8D8);
#else
extern s32 D_1A8D38, D_1A8D3C;
extern void func_0033CD48(void *p, s32 v);
extern void func_0033CE70(void *p);
void func_0033B8D8(void *p) {
    void *box = (char *)p + 0x8;
    if (D_1A8D38 != 0 || D_1A8D3C != 0 || *(s32 *)((char *)p + 0x330) != 0) {
        func_0033CD48(box, 0);
    }
    if (*(s32 *)((char *)p + 0x32C) == 1) {
        func_0033CD48(box, 1);
    }
    func_0033CE70(box);
    func_00338000((char *)p + 0x2D8);
}
#endif

/* func_0033B960: init the level-info panel's thirteen embedded sub-elements at
 * their fixed offsets (EU twin of USA func_0033AA80) - four type-B at
 * p+0x8/+0x54/+0xA0/+0xEC, then nine type-C at +0x138/+0x190/+0x1E8/+0x240/+0x298/
 * +0x2F0/+0x348/+0x3A0/+0x3F8 - return the object.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033AA80: GuiElementInitTypeB->func_00337E88;
 * GuiElementInitTypeC->func_00338648; offsets identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B960);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall (same as USA func_0033AA80). */
void *func_0033B960(void *p) {
    func_00337E88((char *)p + 0x8);
    func_00337E88((char *)p + 0x54);
    func_00337E88((char *)p + 0xA0);
    func_00337E88((char *)p + 0xEC);
    func_00338648((char *)p + 0x138);
    func_00338648((char *)p + 0x190);
    func_00338648((char *)p + 0x1E8);
    func_00338648((char *)p + 0x240);
    func_00338648((char *)p + 0x298);
    func_00338648((char *)p + 0x2F0);
    func_00338648((char *)p + 0x348);
    func_00338648((char *)p + 0x3A0);
    func_00338648((char *)p + 0x3F8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033B9F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033BE48);

/* func_0033BE50 = GuiLevelInfoPanelTick: per-frame layout for the galactic-map
 * level-info panel (EU twin of USA GuiLevelInfoPanelTick, pin-CONFIRMED, USA
 * 0x33AF70, delta +0xEE0). Scales the six "value row" elements (0x190..0x348)
 * uniformly to (D_1ADD78, D_1ADD7C), then positions all thirteen sub-elements
 * relative to the panel origin (the vector at w[0x4]): the four header/icon rows
 * (0x8/0x54/0xA0/0xEC) sit on the origin, the label row (0x138) and the value
 * column share the layout offsets D_1ADD58/D_1ADD60/D_1ADD68/D_1ADD70, with the six
 * value rows (0x190..0x348) stepped 18px apart down the column (0/18/36/54/72/90
 * off the D_1ADD60 base, plus +8px on the last row's x). Refreshes the map
 * thumbnail (func_002DFFC8 at 0x450 for the current level), and sets the panel
 * caption (0x3F8) from g_levelSelectEntries[currentLevel].valueStrId - localized,
 * or the D_1ADD00 placeholder glyph when that id is negative. Returns 0.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA GuiLevelInfoPanelTick: func_002E0010->func_002DFFC8;
 * GuiElementSetPos->func_00337B18; GuiElementSetScale->func_00337C68;
 * GuiElementSetText->func_00338730; GetLocalizedString KEPT.
 * DATA +0xA0 lane: D_1ADCB8->D_1ADD58, D_1ADCC0->D_1ADD60, D_1ADCC8->D_1ADD68,
 * D_1ADCD0->D_1ADD70, D_1ADCD8->D_1ADD78, D_1ADCDC->D_1ADD7C, D_1ADC60->D_1ADD00;
 * g_mapVertexData/g_levelSelectEntries KEPT. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033BE50);
#else
extern char *GetLocalizedString(s32 id);
extern void func_00337B18(void *e, f32 x, f32 y, f32 z, f32 w);
extern void func_00337C68(void *e, f32 x, f32 y, f32 z, f32 w);
extern void func_002DFFC8(void *dst, s32 level);
extern f32 D_1ADD58[2], D_1ADD60[2], D_1ADD68[2], D_1ADD70[2];
extern f32 D_1ADD78, D_1ADD7C;
extern u8 g_mapVertexData[], g_levelSelectEntries[], D_1ADD00[];
s32 func_0033BE50(void *w) {
    f32 *origin = *(f32 **)((char *)w + 0x4);
    s32 level = *(s32 *)(g_mapVertexData + 0x230);
    s32 valueStrId;

    func_00337C68((GuiElement *)((char *)w + 0x190), D_1ADD78, D_1ADD7C, 0.0f, 0.0f);
    func_00337C68((GuiElement *)((char *)w + 0x1E8), D_1ADD78, D_1ADD7C, 0.0f, 0.0f);
    func_00337C68((GuiElement *)((char *)w + 0x240), D_1ADD78, D_1ADD7C, 0.0f, 0.0f);
    func_00337C68((GuiElement *)((char *)w + 0x298), D_1ADD78, D_1ADD7C, 0.0f, 0.0f);
    func_00337C68((GuiElement *)((char *)w + 0x2F0), D_1ADD78, D_1ADD7C, 0.0f, 0.0f);
    func_00337C68((GuiElement *)((char *)w + 0x348), D_1ADD78, D_1ADD7C, 0.0f, 0.0f);

    func_00337B18((GuiElement *)((char *)w + 0x8),   origin[0], origin[1], 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x54),  origin[0], origin[1], 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0xA0),  origin[0], origin[1], 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0xEC),  origin[0], origin[1], 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x138), D_1ADD58[0] + origin[0], D_1ADD58[1] + origin[1], 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x190), D_1ADD60[0] + origin[0], D_1ADD60[1] + origin[1], 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x1E8), D_1ADD60[0] + origin[0], D_1ADD60[1] + origin[1] + 18.0f, 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x240), D_1ADD60[0] + origin[0], D_1ADD60[1] + origin[1] + 36.0f, 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x298), D_1ADD60[0] + origin[0], D_1ADD60[1] + origin[1] + 54.0f, 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x2F0), D_1ADD60[0] + origin[0], D_1ADD60[1] + origin[1] + 72.0f, 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x348), D_1ADD60[0] + 8.0f + origin[0], D_1ADD60[1] + origin[1] + 90.0f, 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x3A0), D_1ADD68[0] + origin[0], D_1ADD68[1] + origin[1], 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x3F8), D_1ADD70[0] + origin[0], D_1ADD70[1] + origin[1], 0.0f, 0.0f);

    func_002DFFC8((char *)w + 0x450, level);

    valueStrId = ((s32 *)g_levelSelectEntries)[*(s32 *)(g_mapVertexData + 0x230) * 2 + 1];
    if (valueStrId < 0) {
        func_00338730((GuiElement *)((char *)w + 0x3F8), (s32)D_1ADD00);
    } else {
        func_00338730((GuiElement *)((char *)w + 0x3F8), (s32)GetLocalizedString(valueStrId));
    }
    return 0;
}
#endif

/* func_0033C308: draw the map-screen panel - only when the +0x490 "panel built"
 * flag is set. Draws the two background sprites (p+0x8, p+0x54), pushes two GS
 * register packets (TEST=0x42->0x44 then SCISSOR=0x47->0xB), renders the map
 * layer (MapDraw(0,1) + func_002DBC60(0)), draws the two foreground sprites
 * (p+0xA0, p+0xEC) and the nine text labels at +0x138 (stride 0x58).
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033B428: GuiSpriteElementDraw->func_00338000,
 * AppendGsRegPacket->func_002FD5B8, MapDraw KEPT, func_002DBC98->func_002DBC60,
 * GuiTextElementDraw->func_00338770; offsets identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033C308);
#else
extern void func_002FD5B8(s32 reg, s32 val);   /* USA AppendGsRegPacket */
extern void MapDraw(s32 a, s32 b);
extern void func_002DBC60(s32 a);              /* USA func_002DBC98 */
void func_0033C308(void *p) {
    if (*(s32 *)((char *)p + 0x490) != 0) {
        func_00338000((char *)p + 0x8);
        func_00338000((char *)p + 0x54);
        func_002FD5B8(0x42, 0x44);
        func_002FD5B8(0x47, 0xB);
        MapDraw(0, 1);
        func_002DBC60(0);
        func_00338000((char *)p + 0xA0);
        func_00338000((char *)p + 0xEC);
        func_00338770((char *)p + 0x138);
        func_00338770((char *)p + 0x190);
        func_00338770((char *)p + 0x1E8);
        func_00338770((char *)p + 0x240);
        func_00338770((char *)p + 0x298);
        func_00338770((char *)p + 0x2F0);
        func_00338770((char *)p + 0x348);
        func_00338770((char *)p + 0x3A0);
        func_00338770((char *)p + 0x3F8);
    }
}
#endif

/* func_0033C3C8: init the map-screen panel's six embedded sub-elements - four
 * type-B elements (p+0x4, stride 0x4C), the widget at +0x134, and one type-C
 * element at +0x170; return the object. Matching arm stays INCLUDE_ASM; #else is
 * the structure model.
 * Word-verified vs USA func_0033B4E8: GuiElementInitTypeB->func_00337E88,
 * func_003374D8->func_003383B0, GuiElementInitTypeC->func_00338648; offsets
 * identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033C3C8);
#else
void *func_0033C3C8(void *p) {
    func_00337E88((char *)p + 0x4);
    func_00337E88((char *)p + 0x50);
    func_00337E88((char *)p + 0x9C);
    func_00337E88((char *)p + 0xE8);
    func_003383B0((char *)p + 0x134);
    func_00338648((char *)p + 0x170);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033C440);

/* func_0033C5B0: when the +0x21C flag is set, position the element at p+0x170
 * from the anchor vector at *(p+0x20C): x = anchor[0] + D_1ADD80; y = anchor[1]
 * + D_1ADD84; z = w = 0. Matching arm stays INCLUDE_ASM; #else is the structure
 * model.
 * Word-verified vs USA func_0033B6D0: GuiElementSetPos->func_00337B18;
 * DATA +0xA0 lane: D_1ADCE0->D_1ADD80, D_1ADCE4->D_1ADD84. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033C5B0);
#else
extern void func_00337B18(void *e, f32 x, f32 y, f32 z, f32 w);
extern f32 D_1ADD80, D_1ADD84;
void func_0033C5B0(void *p) {
    if (*(s32 *)((char *)p + 0x21C) != 0) {
        f32 *anchor = *(f32 **)((char *)p + 0x20C);
        func_00337B18((char *)p + 0x170,
                      anchor[0] + D_1ADD80, anchor[1] + D_1ADD84,
                      0.0f, 0.0f);
    }
}
#endif

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

/* func_0033C928: build a four-glyph button row + count its menu entries. For each
 * of the four slot elements (w+0x4, stride 0x4C): if the slot's glyph id
 * srcGlyphs[i] is non-zero, init the element (D_1ADD00, pool at +0x210), set its
 * colour from D_1ADDA8[i], and assign the glyph from the shared atlas
 * (g_guiInstance+0x8710); otherwise assign glyph 0 and hide it. Then store the
 * entries pointer at +0x0, and count its valid entries (stride 0x18, terminator
 * field +4 == -1, capped at 0x10) into +0x208, zeroing the +0x1C8 row array as it
 * goes. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033BA48: GuiElementInit->func_00337EC0,
 * GuiElementGetColor->func_00337B00, GuiElementSetGlyph->func_00338070,
 * GuiElementSetVisible->func_00337B48; DATA +0xA0 lane: D_1ADC60->D_1ADD00,
 * D_1ADD08->D_1ADDA8; g_guiInstance KEPT. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033C928);
#else
extern char *g_guiInstance;
extern u8 D_1ADD00[];
extern s32 D_1ADDA8[];
void func_0033C928(void *w, s32 *srcGlyphs, void *entries) {
    s32 i, idx, off;
    s32 *row;

    for (i = 0; i <= 3; i++) {
        GuiElement *elem = (GuiElement *)((char *)w + 0x4 + i * 0x4C);
        if (srcGlyphs[i] != 0) {
            func_00337EC0(elem, D_1ADD00, *(void **)((char *)w + 0x210));
            *func_00337B00(elem) = D_1ADDA8[i];
            func_00338070(elem, (u8 *)(g_guiInstance + 0x8710), srcGlyphs[i]);
        } else {
            func_00338070(elem, (u8 *)(g_guiInstance + 0x8710), 0);
            func_00337B48(elem, 0);
        }
    }

    *(s32 *)((char *)w + 0x208) = 0;
    *(void **)((char *)w + 0x0) = entries;

    if (*(s32 *)((char *)entries + 4) != -1) {
        idx = 0;
        off = 0;
        row = (s32 *)((char *)w + 0x1C8);
        *row = 0;
        for (;;) {
            off += 0x18;
            row++;
            *(s32 *)((char *)w + 0x208) += 1;
            if (*(s32 *)((char *)entries + off + 4) == -1) {
                break;
            }
            idx++;
            if (idx >= 0x10) {
                break;
            }
            *row = 0;
        }
    }
}
#endif

/* func_0033CA78 (GuiDialogBoxInitElements twin): build the dialog-box
 * sub-elements over p - 5 TypeB border elements (p+0xC, stride 0x4C) then 3 TypeC
 * text rows (p+0x198, +0x1F0, +0x248). All field writes happen inside the
 * TypeB/TypeC ctors. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA GuiDialogBoxInitElements: GuiElementInitTypeB->func_00337E88,
 * GuiElementInitTypeC->func_00338648; offsets identical. Sig pinned void(void*)
 * (called by landed func_0033B520). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CA78);
#else
void func_0033CA78(void *p) {
    char *base = (char *)p;
    s32 i;
    for (i = 0; i < 5; i++) {
        func_00337E88(base + 0xC + i * 0x4C);
    }
    func_00338648(base + 0x198);
    func_00338648(base + 0x1F0);
    func_00338648(base + 0x248);
}
#endif

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

/* func_0033CD50: lay out the dialog box's five body rows then (when the +0x2C0
 * "show border labels" flag is set) its three border-relative text labels.
 *  - For each of the five rows i (element at p+0xC + i*0x4C, row-active flag is
 *    the halfword at p+0x188 + i*2): when active, anchor the row at the dialog's
 *    own vector (*(p+0x0)) origin and apply the shared alpha at *(p+0x2A4).
 *  - When *(p+0x2C0) is set, place the title (p+0x198), body (p+0x248) and
 *    footer (p+0x1F0) at the origin offset by the (x,y) bound pairs at
 *    +0x2A8/+0x2AC, +0x2B8/+0x2BC and +0x2B0/+0x2B4 respectively.
 * The flags second argument is unused by this method. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Sig pinned void(void*, s32) (called
 * by landed func_0033B7D0).
 * Word-verified vs USA func_0033BE70: GuiElementSetPos->func_00337B18,
 * GuiElementSetAlpha->func_003380A0; offsets identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CD50);
#else
void func_0033CD50(void *e, s32 flags) {
    s32 i;
    f32 *origin;
    GuiElement *row = (GuiElement *)((char *)e + 0xC);
    s16 *active = (s16 *)((char *)e + 0x188);
    (void)flags;
    for (i = 0; i <= 4; i++) {
        if (active[i] != 0) {
            origin = *(f32 **)e;
            func_00337B18(row, origin[0], origin[1], 0.0f, 0.0f);
            func_003380A0(row, *(f32 *)((char *)e + 0x2A4));
        }
        row = (GuiElement *)((char *)row + 0x4C);
    }
    if (*(s32 *)((char *)e + 0x2C0) != 0) {
        origin = *(f32 **)e;
        func_00337B18((GuiElement *)((char *)e + 0x198),
                      origin[0] + *(f32 *)((char *)e + 0x2A8),
                      origin[1] + *(f32 *)((char *)e + 0x2AC), 0.0f, 0.0f);
        origin = *(f32 **)e;
        func_00337B18((GuiElement *)((char *)e + 0x248),
                      origin[0] + *(f32 *)((char *)e + 0x2B8),
                      origin[1] + *(f32 *)((char *)e + 0x2BC), 0.0f, 0.0f);
        origin = *(f32 **)e;
        func_00337B18((GuiElement *)((char *)e + 0x1F0),
                      origin[0] + *(f32 *)((char *)e + 0x2B0),
                      origin[1] + *(f32 *)((char *)e + 0x2B4), 0.0f, 0.0f);
    }
}
#endif

/* func_0033CE70: draw the dialog box's five body rows then its three border
 * text labels.
 *  - For each of the five rows i: when the row-active halfword at p+0x188 + i*2
 *    is set, dispatch the row object at *(p+0x3C + i*0x4C) through its vtable -
 *    call (obj->draw)(rowBase + obj->offset) where rowBase = p+0xC + i*0x4C,
 *    obj->offset is the halfword at obj+0x8 and obj->draw the fn-ptr at obj+0xC.
 *  - When *(p+0x2C0) is set draw the title (p+0x198); additionally when
 *    *(p+0x2C4) is set draw the body (p+0x248) - but only if *(p+0x2CC) is set -
 *    and the footer (p+0x1F0) whenever *(p+0x2C8) is set. Matching arm stays
 *    INCLUDE_ASM; #else is the structure model. Sig pinned void(void*) (file-scope
 *    extern @59, called by landed func_0033B8D8).
 * Word-verified vs USA func_0033BF90: GuiTextElementDraw->func_00338770;
 * offsets identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CE70);
#else
void func_0033CE70(void *p) {
    s32 i;
    s16 *active = (s16 *)((char *)p + 0x188);
    char *obj = (char *)p + 0x3C;
    char *rowBase = (char *)p + 0xC;
    for (i = 0; i <= 4; i++) {
        if (active[i] != 0) {
            void *o = *(void **)obj;
            s16 off = *(s16 *)((char *)o + 0x8);
            void (*draw)(void *) = *(void (**)(void *))((char *)o + 0xC);
            draw(rowBase + off);
        }
        obj += 0x4C;
        rowBase += 0x4C;
    }
    if (*(s32 *)((char *)p + 0x2C0) != 0) {
        func_00338770((char *)p + 0x198);
        if (*(s32 *)((char *)p + 0x2C4) != 0) {
            s32 footer = *(s32 *)((char *)p + 0x2C8);
            if (*(s32 *)((char *)p + 0x2CC) != 0) {
                func_00338770((char *)p + 0x248);
                footer = *(s32 *)((char *)p + 0x2C8);
            }
            if (footer != 0) {
                func_00338770((char *)p + 0x1F0);
            }
        }
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CF40);

/* func_0033CF58 (GuiDialogBoxSetBounds): write six dialog-box bound floats to the
 * panel at +0x2A8..+0x2BC. Args a..f arrive in $f12..$f17; the EU store order
 * (f@+0x2B4, a@+0x2A8, b@+0x2AC, c@+0x2B8, d@+0x2BC, e@+0x2B0) is op-for-op
 * identical to the USA twin. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA GuiDialogBoxSetBounds (0x33C078, delta
 * +0xEE0): pure swc1-store leaf, no data/gp/jal refs - no retargets. Sig pinned
 * void(void*,f32×6) (called by landed func_0033B558). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CF58);
#else
void func_0033CF58(void *p, f32 a, f32 b, f32 c, f32 d, f32 e, f32 f) {
    *(f32 *)((char *)p + 0x2B4) = f;
    *(f32 *)((char *)p + 0x2A8) = a;
    *(f32 *)((char *)p + 0x2AC) = b;
    *(f32 *)((char *)p + 0x2B8) = c;
    *(f32 *)((char *)p + 0x2BC) = d;
    *(f32 *)((char *)p + 0x2B0) = e;
}
#endif

/* func_0033CF78 (GuiDialogBoxSetText3): assign the three dialog-box text labels -
 * the title (p+0x198 <- t0), the body (p+0x248 <- t1) and the footer/prompt
 * (p+0x1F0 <- t2) - each via the text setter func_00338730. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * GuiDialogBoxSetText3 (0x33C098, delta +0xEE0): GuiElementSetText -> func_00338730
 * (EU jal); offsets identical. Sig pinned void(void*,s32,s32,s32) (called by landed
 * func_0033B6C0/func_0033B740). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033CF78);
#else
void func_0033CF78(void *p, s32 t0, s32 t1, s32 t2) {
    func_00338730((GuiElement *)((char *)p + 0x198), t0); /* title */
    func_00338730((GuiElement *)((char *)p + 0x248), t1); /* body */
    func_00338730((GuiElement *)((char *)p + 0x1F0), t2); /* footer */
}
#endif

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

/* func_0033DC60: init the embedded dialog-box (at p+0x8) via func_0033CA78, then
 * return the object. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033CD80 (0x33CD80, delta +0xEE0):
 * GuiDialogBoxInitElements -> func_0033CA78 (EU jal); offset +0x8, return p. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033DC60);
#else
void *func_0033DC60(void *p) {
    func_0033CA78((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033DC90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033DDC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033DF90);

/* func_0033E000: init the embedded dialog-box (at p+0x8) via func_0033CA78, then
 * return the object (identical body to func_0033DC60). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0033D1C0
 * (0x33D1C0, delta +0xEE0): GuiDialogBoxInitElements -> func_0033CA78 (EU jal);
 * offset +0x8, return p. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E000);
#else
void *func_0033E000(void *p) {
    func_0033CA78((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E030);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E160);

/* func_0033E248 (twin of USA func_0033D3C0): draw a two-part formatted caption.
 * Lays out the box body (func_0033CE70(e+8)), then formats the D_1ADE60 template
 * with two localized strings - the fixed id 0x10FC plus one of the two ids in the
 * D_1ADE58 pair selected by the one-shot flag D_1A7C39 - into a scratch buffer and
 * draws it at (0x100, 0xAA) in colour 0x80F0F0F0 via func_00280050.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033D3C0: func_0033BF90->func_0033CE70,
 * func_002801B8->func_00280050, template D_1ADDB0->D_1ADE60, id-pair
 * D_1ADDA8->D_1ADE58, flag D_1A7BB9->D_1A7C39.
 * REGION DELTA: fixed caption string id 0x307A (USA) -> 0x10FC (EU) [localization]. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E248);
#else
extern void func_00280050(s32 x, s32 y, u32 color, const char *text, s32 flag);
extern u8 D_1ADE58[], D_1ADE60[], D_1A7C39;
void func_0033E248(void *e) {
    char buf[128];
    s32 s1, s2;

    func_0033CE70((char *)e + 8);

    s1 = (s32)GetLocalizedString(0x10FC);
    s2 = (s32)GetLocalizedString(*(s32 *)(D_1ADE58 + (D_1A7C39 ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADE60, (const char *)s1, (const char *)s2);
    func_00280050(0x100, 0xAA, 0x80F0F0F0, buf, -1);
}
#endif

/* func_0033E300 (twin of USA func_0033D478): init the embedded dialog-box (at
 * p+0x8) and return the object. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_0033D478: GuiDialogBoxInitElements->
 * func_0033CA78 (already file-scope in this unit), offset p+0x8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E300);
#else
void *func_0033E300(void *p) {
    func_0033CA78((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E330);

/* func_0033E460 (twin of USA func_0033D5D8): settings-menu re-layout + input
 * handler. Re-anchors the dialog box (+0x8) via func_0033CD50/func_0033CF40.
 * Left (0x1000) / right (0x4000) cycle the selection +0x2D8 through 0..3
 * ((sel+3)%4 / (sel+1)%4) with move sound 3. On confirm (0x40, sound 4) it toggles
 * the option for the current row: 0 -> word D_1A7C2C, 1 -> word D_1A7C30,
 * 2 -> D_1A7C34 = (D_1A7C34+1)%3, 3 -> byte D_1A7C39 + func_0027AF28 (rebuild cam).
 * Returns bit 6 of flags. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA func_0033D5D8: func_0033BE70->func_0033CD50,
 * func_0033C060->func_0033CF40, PlayGlobalSound->func_002E6C28,
 * BuildCameraProjection->func_0027AF28, D_1A7BAC/B0/B4->D_1A7C2C/30/34,
 * D_1A7BB9->D_1A7C39. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E460);
#else
extern void func_002E6C28(s32 id, s32 a, s32 b);
extern s32 D_1A7C2C, D_1A7C30, D_1A7C34;
extern u8 D_1A7C39;
s32 func_0033E460(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    f32 *anchor;
    s32 sel;

    func_0033CD50(box, flags);
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033CF40(box, anchor[0], anchor[1]);

    if (flags & 0x1000) {
        func_002E6C28(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 3) % 4;
    } else if (flags & 0x4000) {
        func_002E6C28(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 1) % 4;
    } else if (flags & 0x40) {
        func_002E6C28(4, 0, 0);
        sel = *(s32 *)((char *)w + 0x2D8);
        if (sel == 0) {
            D_1A7C2C = (D_1A7C2C == 0) ? 1 : 0;
        } else if (sel == 1) {
            D_1A7C30 = (D_1A7C30 == 0) ? 1 : 0;
        } else if (sel == 2) {
            D_1A7C34 = (D_1A7C34 + 1) % 3;
        } else if (sel == 3) {
            D_1A7C39 = (D_1A7C39 == 0) ? 1 : 0;
            func_0027AF28();
        }
    }
    return (flags >> 6) & 1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E608);

/* func_0033E888 (twin of USA func_0033DA00): init the embedded dialog-box (at
 * p+0x8) and return the object (identical body shape to func_0033E300).
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA func_0033DA00: GuiDialogBoxInitElements->func_0033CA78, offset p+0x8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E888);
#else
void *func_0033E888(void *p) {
    func_0033CA78((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E8B8);

/* func_0033E9E8 (twin of USA func_0033DB60): variant-B dialog option handler.
 * Re-anchors the dialog box (+0x8) via func_0033CD50/func_0033CF40, then on L/R
 * (0x1000|0x4000) plays move sound 3 + cycles selection +0x2D8 = (sel+1)%2; on
 * confirm (0x40) plays sound 4 + toggles the per-row option flag (D_1A7C1D for
 * row 0, D_1A7C1C for row 1). Returns bit 6 of flags. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0033DB60:
 * func_0033BE70->func_0033CD50, func_0033C060->func_0033CF40, PlayGlobalSound->
 * func_002E6C28, D_1A7B9D->D_1A7C1D, D_1A7B9C->D_1A7C1C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033E9E8);
#else
extern u8 D_1A7C1C, D_1A7C1D;
s32 func_0033E9E8(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    f32 *anchor;
    s32 sel;

    func_0033CD50(box, flags);
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033CF40(box, anchor[0], anchor[1]);

    if ((flags & 0x1000) || (flags & 0x4000)) {
        func_002E6C28(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 1) % 2;
    } else if (flags & 0x40) {
        func_002E6C28(4, 0, 0);
        sel = *(s32 *)((char *)w + 0x2D8);
        if (sel == 0) {
            D_1A7C1D = (D_1A7C1D == 0) ? 1 : 0;
        } else if (sel == 1) {
            D_1A7C1C = (D_1A7C1C == 0) ? 1 : 0;
        }
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033EAF0 (twin of USA func_0033DC68): draw a two-row toggle menu (each row
 * a formatted caption). Lays out the box body (func_0033CE70(w+8)), then draws two
 * rows via func_00280050: row 0 at (0x100, 0x96) = D_1ADE60 template with strings
 * 0xBF0 + the D_1ADE58 id picked by flag D_1A7C1D; row 1 at (0x100, 0xBE) with
 * 0xBEF + the id picked by D_1A7C1C. The currently-selected row (*(w+0x2D8)) is
 * drawn bright (0x80D0D0D0), the other dimmed (0x70808080).
 * DUP resolved: func_0033EAF0 is the true structural twin of USA func_0033DC68.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033DC68: func_0033BF90->func_0033CE70,
 * func_002801B8->func_00280050, template D_1ADDB0->D_1ADE60, id-pair
 * D_1ADDA8->D_1ADE58, flags D_1A7B9D/9C->D_1A7C1D/1C.
 * REGION DELTA: row string ids 0x2C75/0x2C74 (USA) -> 0xBF0/0xBEF (EU) [localization]. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033EAF0);
#else
void func_0033EAF0(void *w) {
    char buf[128];
    s32 sel = *(s32 *)((char *)w + 0x2D8);
    u32 color;
    s32 s0, s1;

    func_0033CE70((char *)w + 0x8);

    color = (sel == 0) ? 0x80D0D0D0 : 0x70808080;
    s0 = (s32)GetLocalizedString(0xBF0);
    s1 = (s32)GetLocalizedString(*(s32 *)(D_1ADE58 + (D_1A7C1D ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADE60, (const char *)s0, (const char *)s1);
    func_00280050(0x100, 0x96, color, buf, -1);

    color = (sel == 1) ? 0x80D0D0D0 : 0x70808080;
    s0 = (s32)GetLocalizedString(0xBEF);
    s1 = (s32)GetLocalizedString(*(s32 *)(D_1ADE58 + (D_1A7C1C ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADE60, (const char *)s0, (const char *)s1);
    func_00280050(0x100, 0xBE, color, buf, -1);
}
#endif

/* func_0033EC50 (twin of USA func_0033DDC8): init the embedded dialog-box (at
 * p+0x8) and its two GuiListRow elements (p+0x2DC, p+0x324), return the object.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA func_0033DDC8: GuiDialogBoxInitElements->func_0033CA78, GuiListRowElementInit
 * ->func_003380B0 (both already file-scope), offsets p+0x8/p+0x2DC/p+0x324. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033EC50);
#else
void *func_0033EC50(void *p) {
    func_0033CA78((char *)p + 0x8);
    func_003380B0((char *)p + 0x2DC);
    func_003380B0((char *)p + 0x324);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033EC90);

/* func_0033EEF8 (twin of USA func_0033E070): the in-game audio-options handler.
 * Re-anchors the dialog box (+0x8) and positions the two volume-slider lists
 * (music +0x2DC, sfx +0x324) at the anchor (*(w+0x36C)) plus D_1ADED0..DC.
 * Left/right (0x1000/0x4000) cycle the selected row +0x2D8 through 0..2 with move
 * sound 3. Holding up (0x8000) / down (0x2000) steps the selected slider by 6,
 * clamped to [0, 0x400] (row 0 = music vol D_1A7C28, row 1 = sfx vol D_1A7C24).
 * Confirm (0x40) on row 2 toggles stereo-mode D_1A7C20 + pushes it to the driver
 * (func_00132998), other rows play sound 5. Always mirrors the volumes into the
 * list scroll positions and recomputes the channel mix (func_002E5630). Returns
 * bit 6 of flags. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033E070: func_0033BE70->func_0033CD50,
 * func_0033C060->func_0033CF40, GuiElementSetPos->func_00337B18, PlayGlobalSound->
 * func_002E6C28, GuiListSetScrollPos->func_003388F0, func_002E5698->func_002E5630,
 * func_00132938->func_00132998; anchor floats D_1ADE30/34/38/3C->D_1ADED0/D4/D8/DC;
 * g_musicVolume->D_1A7C28, g_sfxVolume->D_1A7C24, g_audioStereoMode->D_1A7C20. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033EEF8);
#else
extern void func_00132998(s32 stereoOff);
extern void func_002E5630(void);
extern s32 g_padButtonsHeld, D_1A7C20, D_1A7C24, D_1A7C28;
extern f32 D_1ADED0, D_1ADED4, D_1ADED8, D_1ADEDC;
s32 func_0033EEF8(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    GuiElement *musicList = (GuiElement *)((char *)w + 0x2DC);
    GuiElement *sfxList = (GuiElement *)((char *)w + 0x324);
    f32 *anchor;
    s32 sel, held, v;

    func_0033CD50(box, flags);
    anchor = *(f32 **)((char *)w + 0x36C);
    func_0033CF40(box, anchor[0], anchor[1]);
    anchor = *(f32 **)((char *)w + 0x36C);
    func_00337B18(musicList, anchor[0] + D_1ADED0, anchor[1] + D_1ADED4, 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x36C);
    func_00337B18(sfxList, anchor[0] + D_1ADED8, anchor[1] + D_1ADEDC, 0.0f, 0.0f);

    if (flags & 0x1000) {
        func_002E6C28(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 2) % 3;
    } else if (flags & 0x4000) {
        func_002E6C28(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 1) % 3;
    } else {
        held = g_padButtonsHeld;
        sel = *(s32 *)((char *)w + 0x2D8);
        if (held & 0x8000) {
            if (sel == 0) {
                v = D_1A7C28 - 6;
                D_1A7C28 = (v < 0) ? 0 : v;
            } else if (sel == 1) {
                v = D_1A7C24 - 6;
                D_1A7C24 = (v < 0) ? 0 : v;
            }
        } else if (held & 0x2000) {
            if (sel == 0) {
                v = D_1A7C28 + 6;
                D_1A7C28 = (v > 0x400) ? 0x400 : v;
            } else if (sel == 1) {
                v = D_1A7C24 + 6;
                D_1A7C24 = (v > 0x400) ? 0x400 : v;
            }
        } else if (flags & 0x40) {
            if (sel == 2) {
                func_002E6C28(4, 0, 0);
                D_1A7C20 = (D_1A7C20 == 0) ? 1 : 0;
                func_00132998(D_1A7C20 ^ 1);
            } else {
                func_002E6C28(5, 0, 0);
            }
        }
    }

    func_003388F0(sfxList, D_1A7C24);
    func_003388F0(musicList, D_1A7C28);
    func_002E5630();
    return (flags >> 6) & 1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F190);

/* func_0033F310 (twin of USA func_0033E488): init the embedded dialog-box (at
 * p+0x8) and return the object. DUP note: both EU func_0033F310 and func_0033F9A0
 * are mnemonic-hash-mapped to USA func_0033E488; func_0033F310 is the TRUE twin -
 * its body (single GuiDialogBoxInitElements(p+8); return p) matches func_0033E488's
 * #else exactly. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033E488: GuiDialogBoxInitElements->func_0033CA78,
 * offset p+0x8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F310);
#else
void *func_0033F310(void *p) {
    func_0033CA78((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F340);

/* func_0033F470 (twin of USA func_0033E5E8): re-layout this dialog-box screen and
 * (on first open) chime. Runs the box layout func_0033CD50 on the embedded box
 * (p+0x8), re-feeds the two floats of the vector at *(p+0x2DC) via func_0033CF40,
 * and when (flags & 0x40) is set AND the +0x2D8 already-opened latch is clear,
 * plays the open chime (func_002E6C28(4,0,0)) and sets the one-shot flag D_1A7C1E.
 * Returns bit 6 of flags. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA func_0033E5E8: func_0033BE70->func_0033CD50,
 * func_0033C060->func_0033CF40, PlayGlobalSound->func_002E6C28, flag
 * D_1A7B9E->D_1A7C1E (byte). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F470);
#else
extern u8 D_1A7C1E;
s32 func_0033F470(void *p, s32 flags) {
    void *box = (char *)p + 0x8;
    f32 *v;
    func_0033CD50(box, flags);
    v = *(f32 **)((char *)p + 0x2DC);
    func_0033CF40(box, v[0], v[1]);
    if ((flags & 0x40) != 0 && *(s32 *)((char *)p + 0x2D8) == 0) {
        func_002E6C28(4, 0, 0);
        D_1A7C1E = (D_1A7C1E < 1);
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033F508 (twin of USA func_0033E680): draw a two-part formatted caption
 * with a state-dependent colour: 0x80D0D0D0 when *(w+0x2D8)==0 (bright) else
 * 0x70808080 (dimmed). Lays out the box body (func_0033CE70(w+8)), formats the
 * D_1ADE60 template with the fixed string id 0xBAA plus the D_1ADE58 id selected
 * by the toggle byte D_1A7C1E, and draws it at (0x100, 0xAA). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0033E680:
 * func_0033BF90->func_0033CE70, func_002801B8->func_00280050, template
 * D_1ADDB0->D_1ADE60, id-pair D_1ADDA8->D_1ADE58, flag D_1A7B9E->D_1A7C1E.
 * REGION DELTA: fixed caption string id 0x2C2F (USA) -> 0xBAA (EU) [localization]. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F508);
#else
void func_0033F508(void *w) {
    char buf[128];
    s32 s1, s2;
    u32 color;

    func_0033CE70((char *)w + 0x8);

    color = (*(s32 *)((char *)w + 0x2D8) == 0) ? 0x80D0D0D0 : 0x70808080;
    s1 = (s32)GetLocalizedString(0xBAA);
    s2 = (s32)GetLocalizedString(*(s32 *)(D_1ADE58 + (D_1A7C1E ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADE60, (const char *)s1, (const char *)s2);
    func_00280050(0x100, 0xAA, color, buf, -1);
}
#endif

/* func_0033F5D8 (GuiQuitDialogInitElements, twin of USA GuiQuitDialogInitElements
 * @0x33E750, delta +0xE88, EU pin-CONFIRMED): init the embedded dialog-box (at
 * p+0x8) and return the object. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA GuiQuitDialogInitElements:
 * GuiDialogBoxInitElements->func_0033CA78, offset p+0x8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F5D8);
#else
void *func_0033F5D8(void *p) {
    func_0033CA78((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F608);

/* func_0033F738 (twin of USA func_0033E8B0): the quit-dialog option-list handler.
 * Re-lays-out the dialog box (+0x8) at its anchor (*(w+0x2DC)) via func_0033CD50/
 * func_0033CF40, then reacts to the input mask (flags): on left/right
 * (0x1000|0x4000) plays move sound 3 and cycles the selection +0x2D8 between 0 and
 * 1; on confirm (0x40) plays confirm sound 4 and toggles the per-row option flag
 * (D_1A7C3A for row 0, D_1A7C3B for row 1). Returns bit 6 of flags. Matching arm
 * stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_0033E8B0: func_0033BE70->func_0033CD50, func_0033C060->func_0033CF40,
 * PlayGlobalSound->func_002E6C28, flags D_1A7BBA/D_1A7BBB->D_1A7C3A/D_1A7C3B. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F738);
#else
extern u8 D_1A7C3A, D_1A7C3B;
s32 func_0033F738(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    f32 *anchor;
    s32 sel;

    func_0033CD50(box, flags);
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033CF40(box, anchor[0], anchor[1]);

    if ((flags & 0x1000) || (flags & 0x4000)) {
        func_002E6C28(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 1) % 2;
    } else if (flags & 0x40) {
        func_002E6C28(4, 0, 0);
        sel = *(s32 *)((char *)w + 0x2D8);
        if (sel == 0) {
            D_1A7C3A = (D_1A7C3A == 0) ? 1 : 0;
        } else if (sel == 1) {
            D_1A7C3B = (D_1A7C3B == 0) ? 1 : 0;
        }
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033F840 (func_0033DC68-family sibling): draw a two-row toggle menu (each
 * row a formatted caption). Lays out the box body (func_0033CE70(w+8)), then draws
 * two rows via func_00280050: row 0 at (0x100, 0x96) = D_1ADE60 template with string
 * 0xBAB + the D_1ADE58 id picked by flag D_1A7C3A; row 1 at (0x100, 0xBE) with
 * 0x10FB + the id picked by D_1A7C3B. Selected row (*(w+0x2D8)) drawn bright
 * (0x80D0D0D0), the other dimmed (0x70808080).
 * DUP resolution: mnemonic-hash mapped this to USA func_0033DC68, but landed
 * func_0033EAF0 is that twin already. func_0033F840 is a distinct EU sibling of the
 * same body shape (func_0033CE70 layout, no func_0027F7A0/790 batch => not
 * func_0033E9B8). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033DC68: func_0033BF90->func_0033CE70,
 * func_002801B8->func_00280050, template D_1ADDB0->D_1ADE60, id-pair
 * D_1ADDA8->D_1ADE58, flags D_1A7B9D/9C->D_1A7C3A/D_1A7C3B.
 * REGION DELTA: row string ids 0x2C75/0x2C74 (USA) -> 0xBAB/0x10FB (EU). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F840);
#else
void func_0033F840(void *w) {
    char buf[128];
    s32 sel = *(s32 *)((char *)w + 0x2D8);
    u32 color;
    s32 s0, s1;

    func_0033CE70((char *)w + 0x8);

    color = (sel == 0) ? 0x80D0D0D0 : 0x70808080;
    s0 = (s32)GetLocalizedString(0xBAB);
    s1 = (s32)GetLocalizedString(*(s32 *)(D_1ADE58 + (D_1A7C3A ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADE60, (const char *)s0, (const char *)s1);
    func_00280050(0x100, 0x96, color, buf, -1);

    color = (sel == 1) ? 0x80D0D0D0 : 0x70808080;
    s0 = (s32)GetLocalizedString(0x10FB);
    s1 = (s32)GetLocalizedString(*(s32 *)(D_1ADE58 + (D_1A7C3B ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADE60, (const char *)s0, (const char *)s1);
    func_00280050(0x100, 0xBE, color, buf, -1);
}
#endif

/* func_0033F9A0 (func_0033E488-family sibling): init the embedded dialog-box (at
 * p+0x8) and return the object. DUP note: mnemonic-hash mapped this to USA
 * func_0033E488, but landed func_0033F310 is that twin already; func_0033F9A0 is a
 * distinct EU sibling with the identical trivial InitElements-and-return body.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA func_0033E488: GuiDialogBoxInitElements->func_0033CA78, offset p+0x8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F9A0);
#else
void *func_0033F9A0(void *p) {
    func_0033CA78((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033F9D0);

/* func_0033FB00 (EU func_0033E5E8-family handler + per-frame re-localization):
 * re-set the dialog-box's three text rows (localized 0xB72/0xB5F/0xB60) via
 * func_0033CF78, re-lay-out the box (func_0033CD50(box, flags)), re-position it at
 * its anchor (*(w+0x2DC) two floats) via func_0033CF40, and on the confirm bit
 * (flags & 0x40) while not already busy (*(w+0x2D8)==0) play the chime
 * func_002E6C28(4,0,0) and set the one-shot flag D_1A7C3A. Returns bit 6 of flags.
 * TWIN RE-DERIVED: mnemonic-hash mapped this to USA constructor func_0033E4C0, but
 * that is a GuiPlacementNew/InitBorder/SetBounds/SetScale constructor. The correct
 * structural twin is the handler USA func_0033E5E8 (func_0033BE70->func_0033CD50,
 * func_0033C060->func_0033CF40, PlayGlobalSound->func_002E6C28, same 0x40/0x2D8
 * confirm-chime guard, anchor at +0x2DC, return (flags>>6)&1).
 * REGION DELTA (behavioural): EU adds a per-frame GuiDialogBoxSetText3
 * (func_0033CF78) that re-localizes rows 0xB72/0xB5F/0xB60 every frame; USA
 * func_0033E5E8 has NO SetText3 (text set once at construction). Also EU toggles
 * flag D_1A7C3A via the (x<1) idiom where USA func_0033E5E8 toggles D_1A7B9E.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FB00);
#else
s32 func_0033FB00(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    s32 t0, t1, t2;
    f32 *anchor;

    t0 = (s32)GetLocalizedString(0xB72);
    t1 = (s32)GetLocalizedString(0xB5F);
    t2 = (s32)GetLocalizedString(0xB60);
    func_0033CF78(box, t0, t1, t2);

    func_0033CD50(box, flags);
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033CF40(box, anchor[0], anchor[1]);

    if ((flags & 0x40) != 0 && *(s32 *)((char *)w + 0x2D8) == 0) {
        func_002E6C28(4, 0, 0);
        D_1A7C3A = (D_1A7C3A < 1);
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033FBE0 (twin of USA func_0033ED18): draw a single two-part formatted
 * caption. Lays out the box body (func_0033CE70(w+8)), formats D_1ADE60 with fixed
 * string 0xBAB plus the D_1ADE58 id selected by flag D_1A7C3A, draws it at
 * (0x100, 0xAA) in colour 0x80F0F0F0. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_0033ED18: func_0033BF90->func_0033CE70,
 * func_002801B8->func_00280050, template D_1ADDB0->D_1ADE60, id-pair
 * D_1ADDA8->D_1ADE58, flag D_1A7BBA->D_1A7C3A.
 * REGION DELTA: fixed caption string id 0x2C30 (USA) -> 0xBAB (EU) [localization]. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FBE0);
#else
void func_0033FBE0(void *e) {
    char buf[128];
    s32 s1, s2;

    func_0033CE70((char *)e + 0x8);

    s1 = (s32)GetLocalizedString(0xBAB);
    s2 = (s32)GetLocalizedString(*(s32 *)(D_1ADE58 + (D_1A7C3A ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADE60, (const char *)s1, (const char *)s2);
    func_00280050(0x100, 0xAA, 0x80F0F0F0, buf, -1);
}
#endif

/* func_0033FC98 (twin of USA func_0033EDD0): init the embedded dialog-box (at
 * p+0x8) and return the object. (func_0033E488-family trivial body; several USA
 * siblings share it identically.) Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_0033EDD0: GuiDialogBoxInitElements->
 * func_0033CA78, offset p+0x8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FC98);
#else
void *func_0033FC98(void *p) {
    func_0033CA78((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FCC8);

/* func_0033FDF8 (EU func_0033EF30-family handler + per-frame re-localization):
 * sibling of func_0033FB00 with an identical body. Re-set the dialog-box's three
 * text rows (localized 0xB72/0xB5F/0xB60) via func_0033CF78, re-lay-out the box
 * (func_0033CD50(box, flags)), re-position it at its anchor (*(w+0x2DC) two floats)
 * via func_0033CF40, and on the confirm bit (flags & 0x40) while not busy
 * (*(w+0x2D8)==0) play func_002E6C28(4,0,0) + set the one-shot flag D_1A7C3A.
 * Returns bit 6 of flags.
 * TWIN RE-DERIVED: mnemonic-hash mapped this to USA constructor func_0033EE08, but
 * that is a GuiPlacementNew/InitBorder/SetBounds/SetScale constructor. The correct
 * structural twin is the handler USA func_0033EF30 (func_0033BE70->func_0033CD50,
 * func_0033C060->func_0033CF40, PlayGlobalSound->func_002E6C28, 0x40/0x2D8 guard,
 * anchor +0x2DC, return (flags>>6)&1).
 * REGION DELTA (behavioural): EU adds a per-frame GuiDialogBoxSetText3
 * (func_0033CF78) re-localizing 0xB72/0xB5F/0xB60 every frame; USA func_0033EF30
 * has NO SetText3. EU toggles D_1A7C3A via the (x<1) idiom (USA func_0033EF30
 * toggles D_1A7BBA). Matching arm stays INCLUDE_ASM; #else is the structure model. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FDF8);
#else
s32 func_0033FDF8(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    s32 t0, t1, t2;
    f32 *anchor;

    t0 = (s32)GetLocalizedString(0xB72);
    t1 = (s32)GetLocalizedString(0xB5F);
    t2 = (s32)GetLocalizedString(0xB60);
    func_0033CF78(box, t0, t1, t2);

    func_0033CD50(box, flags);
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033CF40(box, anchor[0], anchor[1]);

    if ((flags & 0x40) != 0 && *(s32 *)((char *)w + 0x2D8) == 0) {
        func_002E6C28(4, 0, 0);
        D_1A7C3A = (D_1A7C3A < 1);
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033FED8 (func_0033E488-family sibling): init the embedded dialog-box (at
 * p+0x8) and return the object. Trivial InitElements-and-return body shared by
 * several USA siblings (func_0033E488/EDD0/EFC8). Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA func_0033E488:
 * GuiDialogBoxInitElements->func_0033CA78, offset p+0x8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FED8);
#else
void *func_0033FED8(void *p) {
    func_0033CA78((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_0033FF08);

/* func_00340038 (per-frame handler for the dialog-box screen constructed by EU
 * func_0033FF08 = USA func_0033F000). Re-localizes the box's three text rows every
 * frame (GetLocalizedString x3 -> GuiDialogBoxSetText3/func_0033CF78), lays out the
 * embedded box (+0x8) via func_0033CD50, re-anchors it at *(w+0x2DC) via func_0033CF40,
 * then on the confirm bit (flags & 0x40) while not busy (+0x2D8 == 0) plays confirm
 * sound 4 and toggles the global one-shot flag D_1A7C3A. Returns bit 6 of flags.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * TWIN RE-DERIVATION: assigned twin USA func_0033F000 is a mnemonic-hash MIS-MAP
 * (that constructor is EU func_0033FF08). Correct tail-shape twin is USA func_0033F128
 * (position + confirm-toggle, D_1A7BBA). Modeled directly from the EU .s:
 * func_0033BE70->func_0033CD50, func_0033C060->func_0033CF40, PlayGlobalSound->
 * func_002E6C28, GetLocalizedString KEPT, D_1A7BBA->D_1A7C3A.
 * GENUINE REGION DIFF: USA func_0033F128 does NOT set text - the EU handler
 * re-localizes 3 dialog rows (0xB72/0xB5F/0xB60) every frame (PAL multi-language). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340038);
#else
s32 func_00340038(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    f32 *anchor;
    s32 t0, t1, t2;

    t0 = (s32)GetLocalizedString(0xB72);
    t1 = (s32)GetLocalizedString(0xB5F);
    t2 = (s32)GetLocalizedString(0xB60);
    func_0033CF78(box, t0, t1, t2);

    func_0033CD50(box, flags);   /* 2nd arg (flags) ignored by the callee */
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033CF40(box, anchor[0], anchor[1]);

    if ((flags & 0x40) && *(s32 *)((char *)w + 0x2D8) == 0) {
        func_002E6C28(4, 0, 0);
        D_1A7C3A = (D_1A7C3A < 1);
    }
    return (flags >> 6) & 1;
}
#endif

/* func_00340118 (EU twin of USA func_0033F1C0): init the embedded dialog-box (p+0x8)
 * via func_0033CA78 (GuiDialogBoxInitElements) and the list widget at p+0x2E0 via
 * func_0034A068 (USA func_00348BD0), then return the object. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0033F1C0:
 * GuiDialogBoxInitElements->func_0033CA78, func_00348BD0->func_0034A068 (out-of-TU);
 * offsets p/+0x8/+0x2E0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340118);
#else
extern void func_0034A068(void *p);
void *func_00340118(void *p) {
    func_0033CA78((char *)p + 0x8);
    func_0034A068((char *)p + 0x2E0);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340150);

/* func_003402B8 (EU twin of USA func_0033F360): get the selected row index from
 * func_003402F0 (USA func_0033F398), then return base(+0x3B8) + index*0x14.
 * PARKED (bare): the USA twin models this with a cast-to-recover-return idiom
 * (((s32(*)(void*))func_003402F0)(p)) because func_003402F0 is a lockstep-void
 * forwarder that actually returns the index in $v0. Reproducing that cast here
 * trips -Wcast-function-type (a NEW gate warning); the clean fix (retype
 * func_003402F0 -> s32) would break the EU/USA void-forwarder lockstep symmetry.
 * Deferred pending a region-wide decision on the func_003402F0/func_0034A2F0
 * (USA func_0033F398/func_00348E60=s32) return-type. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003402B8);

/* func_003402F0: forward p+0x2E0 to func_0034A2F0. USA func_0033F398. */
void func_003402F0(void *p) {
    func_0034A2F0((char *)p + 0x2E0);
    __asm__ __volatile__("");
}

/* func_00340310 (EU twin of USA func_0033F3B8): init the list widget at p+0x2E0 via
 * func_0034A230 (USA func_00348DA0) with the row-record table, then record that table
 * pointer at p+0x3B8. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033F3B8: func_00348DA0->func_0034A230 (out-of-TU);
 * offsets +0x2E0/+0x3B8. Args: p, records ($5 held live across the call). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340310);
#else
extern void func_0034A230(void *p, void *records);
void func_00340310(void *p, void *records) {
    func_0034A230((char *)p + 0x2E0, records);
    *(void **)((char *)p + 0x3B8) = records;
}
#endif

/* func_00340348 (per-frame handler for the list dialog-box). Re-localizes the box's
 * three text rows every frame (GetLocalizedString x3 -> func_0033CF78), lays out the
 * box (+0x8) via func_0033CD50, re-anchors it at *(w+0x3BC) via func_0033CF40, drives
 * the list element (+0x2E0): func_0034A148 (USA func_00348CB8) with arg1, then
 * re-anchors the list at *(w+0x3BC) via func_0034A2B8 (USA func_00348E28). Returns
 * bit 6 of arg1. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * TWIN RE-DERIVATION: assigned twin USA func_0033F3B8 is a mnemonic-hash MIS-MAP (a
 * 2-line init = EU func_00340310). Correct shape twin is USA func_0033F3F0 (the
 * list-anchor handler). Modeled from the EU .s: func_0033BE70->func_0033CD50,
 * func_0033C060->func_0033CF40, func_00348CB8->func_0034A148, func_00348E28->
 * func_0034A2B8, anchor +0x3BC.
 * GENUINE REGION DIFF: USA func_0033F3F0 does NOT set text - the EU handler
 * re-localizes 3 rows every frame (0xBB1/0xB86/0xB60, PAL multi-language). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340348);
#else
extern void func_0034A148(void *e, s32 v);
extern void func_0034A2B8(void *e, f32 x, f32 y);
s32 func_00340348(void *w, s32 arg1) {
    void *box = (char *)w + 0x8;
    void *list = (char *)w + 0x2E0;
    f32 *anchor;
    s32 t0, t1, t2;

    t0 = (s32)GetLocalizedString(0xBB1);
    t1 = (s32)GetLocalizedString(0xB86);
    t2 = (s32)GetLocalizedString(0xB60);
    func_0033CF78(box, t0, t1, t2);

    func_0033CD50(box, arg1);   /* 2nd arg ignored by the callee */
    anchor = *(f32 **)((char *)w + 0x3BC);
    func_0033CF40(box, anchor[0], anchor[1]);

    func_0034A148(list, arg1);
    anchor = *(f32 **)((char *)w + 0x3BC);
    func_0034A2B8(list, anchor[0], anchor[1]);
    return (arg1 >> 6) & 1;
}
#endif

/* func_00340418 (EU twin of USA func_0033F478): tear down the embedded dialog-box
 * (p+0x8) via func_0033CE70 (func_0033BF90), then reconfigure the list widget at
 * p+0x2E0 with the per-language resource D_1ADF50[g_currentLanguage] via func_0034A2E8
 * (USA func_00348E58) and redraw it via func_0034A300 (USA GuiMenuListDraw).
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_0033F478: func_0033BF90->func_0033CE70, func_00348E58->func_0034A2E8,
 * GuiMenuListDraw->func_0034A300, gp_rel(D_1ADEB0)->gp_rel(D_1ADF50), g_currentLanguage
 * KEPT. DATA DELTA: g_currentLanguage EU 0x1A7C3C (USA 0x1A7BBC, +0x80); per-language
 * table D_1ADF50 = EU twin of USA D_1ADEB0 (gp_rel). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340418);
#else
extern u8 g_currentLanguage;
extern s32 D_1ADF50[];
extern void func_0034A2E8(void *p, s32 res);
void func_00340418(void *p) {
    void *w = (char *)p + 0x2E0;
    func_0033CE70((char *)p + 0x8);
    func_0034A2E8(w, D_1ADF50[g_currentLanguage]);
    func_0034A300(w);
}
#endif

/* func_00340470 (EU twin of USA func_0033F4D0): init the embedded dialog-box (p+0x8)
 * via func_0033CA78 (GuiDialogBoxInitElements) and a type-C element (p+0x2E0) via
 * func_00338648 (GuiElementInitTypeC), then return the object. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0033F4D0:
 * GuiDialogBoxInitElements->func_0033CA78, GuiElementInitTypeC->func_00338648;
 * offsets p/+0x8/+0x2E0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340470);
#else
void *func_00340470(void *p) {
    func_0033CA78((char *)p + 0x8);
    func_00338648((char *)p + 0x2E0);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003404A8);

/* func_003405B0: EU twin of USA func_0033F610. Forward the embedded dialog-box
 * (p+0x8) to func_0033CD50, then push the two floats from the vector at
 * *(p+0x2D8) into it via func_0033CF40. Returns bit 6 of the flags argument.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033F610: func_0033BE70->func_0033CD50,
 * func_0033C060->func_0033CF40 (both EU file-scope); return andi flags>>6 & 1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003405B0);
#else
s32 func_003405B0(void *p, s32 flags) {
    f32 *v;
    func_0033CD50((char *)p + 0x8, flags);
    v = *(f32 **)((char *)p + 0x2D8);
    func_0033CF40((char *)p + 0x8, v[0], v[1]);
    return (flags >> 6) & 1;
}
#endif

/* func_00340610: forward p+0x8 to func_0033CE70. USA func_0033F670. */
void func_00340610(void *p) {
    func_0033CE70((char *)p + 0x8);
    __asm__ __volatile__("");
}

/* func_00340630: EU twin of USA func_0033F690. Init the screen's embedded
 * sub-elements (the dialog box at +0x8, the widget at +0x2DC, then the
 * type-B/type-C element run), and return the object. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0033F690:
 * GuiDialogBoxInitElements->func_0033CA78, func_003374D8->func_003383B0,
 * GuiElementInitTypeC->func_00338648, GuiElementInitTypeB->func_00337E88 (jal
 * order: +0x8, +0x2DC, +0x318, +0x370, +0x3C8, +0x420 [type-B], +0x470..+0x628
 * [type-C]); all EU file-scope. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340630);
#else
void *func_00340630(void *p) {
    func_0033CA78((char *)p + 0x8);
    func_003383B0((char *)p + 0x2DC);
    func_00338648((char *)p + 0x318);
    func_00338648((char *)p + 0x370);
    func_00338648((char *)p + 0x3C8);
    func_00337E88((char *)p + 0x420);
    func_00338648((char *)p + 0x470);
    func_00338648((char *)p + 0x4C8);
    func_00338648((char *)p + 0x520);
    func_00338648((char *)p + 0x578);
    func_00338648((char *)p + 0x5D0);
    func_00338648((char *)p + 0x628);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_003406B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340A50);

/* func_00340C88: EU twin of USA func_0033FCE8. Stats-panel value-row layout.
 * Positions four value elements at fixed data-driven offsets from the shared
 * anchor record *(w+0x2D8): +0x578 at (D_1ADFF0, D_1ADFF4), +0x470 at
 * (D_1ADFD8, D_1ADFDC), +0x4C8 at (D_1ADFE0, D_1ADFE4), +0x520 at
 * (D_1ADFE8, D_1ADFEC). Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA func_0033FCE8: GuiElementSetPos->func_00337B18;
 * DATA +0xA0 lane D_1ADF38..54 -> D_1ADFD8..F4 (8 new f32 decls). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340C88);
#else
extern f32 D_1ADFD8, D_1ADFDC, D_1ADFE0, D_1ADFE4;
extern f32 D_1ADFE8, D_1ADFEC, D_1ADFF0, D_1ADFF4;
void func_00340C88(void *w) {
    f32 *anchor = *(f32 **)((char *)w + 0x2D8);

    func_00337B18((GuiElement *)((char *)w + 0x578), D_1ADFF0 + anchor[0], D_1ADFF4 + anchor[1], 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x470), D_1ADFD8 + anchor[0], D_1ADFDC + anchor[1], 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x4C8), D_1ADFE0 + anchor[0], D_1ADFE4 + anchor[1], 0.0f, 0.0f);
    func_00337B18((GuiElement *)((char *)w + 0x520), D_1ADFE8 + anchor[0], D_1ADFEC + anchor[1], 0.0f, 0.0f);
}
#endif

/* func_00340D78: PARKED (bare). Mnemonic-hash mis-map + genuine region divergence.
 * The positional USA twin func_0033FDD8 is a 1-row map planet-bolt line (guarded by
 * g_mapVertexData+0x230, uses CountPlatinumBolts/func_002B1D18). EU func_00340D78 is
 * a 3-row stats-panel value drawer (localized 0xBCC/0xBCD/0xBCE, func_002B18D8/E8,
 * func_002904C8 draw, colour 0x55F0C070) - a different callee set + control flow.
 * No USA function in the unit has the EU 3-row shape (only func_0033FDD8 touches
 * CountPlatinumBolts). Real PAL/NTSC layout divergence (EU renders three localized
 * value rows where USA renders one); deferred pending its true twin / direct model. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00340D78);

/* func_00341020: EU twin of USA func_0033FEF8. Draw one composite screen - run
 * the body builder at e+0x8, blit the menu backdrop (func_002DBC60(0)), draw its
 * three header text rows, the inner panel (func_00338508 at e+0x2DC), a sprite
 * (e+0x420), two footer text rows, then the trailing builder func_00340D78(e).
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0033FEF8: func_0033BF90->func_0033CE70,
 * func_002DBC98->func_002DBC60, GuiTextElementDraw->func_00338770,
 * func_00337630->func_00338508, GuiSpriteElementDraw->func_00338000,
 * tail func_0033FDD8->func_00340D78 (the parked EU 3-row drawer, forward-declared
 * inline to match its EU .s arg shape $4=w). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/236ED8", func_00341020);
#else
void func_00340D78(void *w);
void func_00341020(void *e) {
    func_0033CE70((char *)e + 0x8);
    func_002DBC60(0);
    func_00338770((char *)e + 0x318);
    func_00338770((char *)e + 0x370);
    func_00338770((char *)e + 0x3C8);
    func_00338508((GuiElement *)((char *)e + 0x2DC));
    func_00338000((char *)e + 0x420);
    func_00338770((char *)e + 0x628);
    func_00338770((char *)e + 0x5D0);
    func_00340D78(e);
}
#endif

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

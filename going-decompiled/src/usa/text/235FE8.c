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
    /* 0x08 */ f32 *unk08;   /* -> 16-byte vector block (allocated 2nd by base init) */
    /* 0x0C */ s32 *color;   /* -> color block */
    /* 0x10 */ f32 *visible; /* -> visibility scalar (>0 shown) */
} GuiElement;

/* GUI fixed-size node pool. Full field map + allocator body are at GuiPoolAlloc
 * below; forward-declared here so the element constructors (GuiElementBaseInit,
 * GuiElementInit) that allocate their vector blocks through it see a consistent
 * prototype regardless of source order. Declaration-only: byte-neutral. */
typedef struct GuiPool GuiPool;
extern void *GuiPoolAlloc(GuiPool *pool);
extern void *GuiPlacementNew(s32 size, void *buf);
extern void GuiElementSetVisible(GuiElement *e, s32 show);


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

#ifdef TARGET_NATIVE
/* Shared GUI sub-element init/widget callees referenced by the functional-
 * equivalent #else bodies below (declared up front so each body sees a
 * consistent prototype regardless of source order). */
extern void GuiElementInitTypeB(void *p);
extern void GuiElementInitTypeC(void *p);
extern void GuiListRowElementInit(void *p);
extern void func_00348BD0(void *p);
extern void func_00348CB8(void *w);
extern void func_00348E28(void *w, f32 x, f32 y);
extern void func_00348E58(void *w, s32 res);
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336068);

/* func_00336168: copy a 16-byte (quadword) vector from *p into the active
 * camera's render block at +0x80. The camera is g_activeCamera unless its
 * mode word (+0x86) differs from 5, in which case func_00270290(5) supplies a
 * substitute camera; the render block lives at *(camera+0x70). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336168);
#else
/* TODO(match): functional equivalent - not byte-exact; sq/lq 128-bit copy +
   branch wall. */
/* A 16-byte (128-bit) quadword record. func_00336168/func_003361C0 copy it with
 * a single lq/sq (`lq v0,0x0(src); sq v0,0x0(dst)`) into the active camera's
 * render block at +0x80 / +0x90 respectively. Size pin is byte-neutral. */
typedef struct GuiQword { s32 w[4]; } GuiQword;
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(GuiQword) == 0x10, "GuiQword is a 128-bit quadword");
#endif
extern void *g_activeCamera;
extern void *func_00270290(s32 mode);
void func_00336168(GuiQword *src) {
    void *cam = g_activeCamera;
    void *base = (*(s16 *)((char *)cam + 0x86) == 5) ? cam : func_00270290(5);
    GuiQword *dst = (GuiQword *)((char *)*(void **)((char *)base + 0x70) + 0x80);
    *dst = *src;
}
#endif

/* func_003361C0: as func_00336168 but writes the quadword to the camera render
 * block at +0x90 instead of +0x80. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003361C0);
#else
/* TODO(match): functional equivalent - not byte-exact; sq/lq 128-bit copy +
   branch wall. */
void func_003361C0(GuiQword *src) {
    void *cam = g_activeCamera;
    void *base = (*(s16 *)((char *)cam + 0x86) == 5) ? cam : func_00270290(5);
    GuiQword *dst = (GuiQword *)((char *)*(void **)((char *)base + 0x70) + 0x90);
    *dst = *src;
}
#endif

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

/* func_00336678: ALWAYS install the D_1AD9A8 vtable at p+0x4, then call
 * func_00337C48() only when (flag & 1). The store sits in the delay slot of the
 * `beqz flag&1` branch, so it is unconditional; the call is the fall-through. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336678);
#else
/* TODO(match): functional equivalent - not byte-exact; the original hoists the
   %hi/%lo address computation above the branch and stores in the branch delay
   slot, a form cc1 won't reproduce here. */
void func_00336678(void *p, s32 flag) {
    *(void **)((char *)p + 0x4) = &D_1AD9A8;   /* delay-slot store: unconditional */
    if (flag & 1) {
        func_00337C48();
    }
}
#endif

/* GuiComputeBlendWeights: from a single blend factor t, fill a 4-float weight
 * vector for a two-control-point blend: out[0]=t, out[1]=t*0.5, out[2]=1-t,
 * out[3]=(1-t)*0.5 (the .5-scaled entries are the tangent weights). Asserts the
 * output pointer is non-NULL first (AssertFail with the D_1AD890 source path,
 * line 0x3D, predicate D_1AD8A8). Like the other widget-method blends in this
 * unit (func_00336918 / func_00336988) the leading object pointer (a0) is unused
 * by the body; t arrives in $f12 and the output pointer in $a1. */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; the 8-byte-packed
   callee-save frame wall (original packs s0/ra/f20 into a -0x20 frame at
   0x0/0x8/0x10; this cc1 emits a -0x30 frame at 0x0/0x10/0x20). The body insn
   stream (assert, the 1.0/0.5 li.s constants, the four stores) is otherwise
   bit-identical. 99.62% best. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiComputeBlendWeights);
#else
extern void AssertFail(const char *file, s32 line, const char *expr);
extern const char D_1AD890[]; /* assert source-file path */
extern const char D_1AD8A8[]; /* assert predicate text ("out") */
void GuiComputeBlendWeights(void *self, f32 t, f32 *out) {
    (void)self;
    if (out == 0) {
        AssertFail(D_1AD890, 0x3D, D_1AD8A8);
    }
    out[0] = t;
    out[2] = 1.0f - t;
    out[1] = t * 0.5f;
    out[3] = (1.0f - t) * 0.5f;
}
#endif

/* func_00336720: install the D_1AD968 vtable at p+0x4, then run the base ctor
 * func_00336678(p, flag) (the field store sits in the jal delay slot). */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; the original fills the
   func_00336678 jal delay slot with the +0x4 vtable store; cc1 emits the store
   ahead of the call and nops the slot. Same jal-delay-slot-store wall as
   func_00336B88. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336720);
#else
extern void *D_1AD968;
void func_00336720(void *p, s32 flag) {
    *(void **)((char *)p + 0x4) = &D_1AD968;
    func_00336678(p, flag);
}
#endif

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

/* func_00336988: per-channel blend of a 4-int vector. For each component i in
 * {0,1,2,3}, out[i] = func_002846E8(t, aSrc[i], bSrc[i]) - the shared scalar
 * color/alpha interpolation helper applied component-wise. The leading object
 * pointer (a0) carries no state into the blend and is unused by the body. */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; the 8-byte-packed
   callee-save frame wall (this cc1 emits a -0x50 frame with 16-byte-strided
   spill slots for s0/s1/s2/ra/f20; the original packs them into a -0x30 frame
   at 0x0/0x8/0x10/0x18/0x20). The body insn stream is otherwise identical.
   87.5% best. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336988);
#else
extern s32 func_002846E8(f32 t, s32 a, s32 b);
void func_00336988(void *self, s32 *out, s32 *aSrc, s32 *bSrc, f32 t) {
    out[0] = func_002846E8(t, aSrc[0], bSrc[0]);
    out[1] = func_002846E8(t, aSrc[1], bSrc[1]);
    out[2] = func_002846E8(t, aSrc[2], bSrc[2]);
    out[3] = func_002846E8(t, aSrc[3], bSrc[3]);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336A18);

/* func_00336A28: cubic Hermite blend of t between the two control values at
 * src+0x8 and src+0xC (endpoint tangents 0 and 1), writing the result to
 * *dst. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336A28);
#else
/* TODO(match): functional equivalent - not byte-exact; single-callee-save plus
   the float-constant setup hoist the original interleaves differently. */
extern f32 GuiHermiteInterp(f32 t, f32 c0, f32 c1, f32 c2, f32 c3);
void func_00336A28(f32 *src, f32 t, f32 *dst) {
    *dst = GuiHermiteInterp(t, 0.0f, src[2], src[3], 1.0f);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336A68);

/* func_00336A78: install the D_1AD8C8 vtable at p+0x4, free the pooled node
 * (*(p+0x2C) is the pool, *(p+0x28) the node) via func_00337D78, then run the
 * base ctor func_00336678(p, flag). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336A78);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame
   wall. */
extern void *D_1AD8C8;
extern void func_00337D78(void *pool, void **node);
void func_00336A78(void *p, s32 flag) {
    *(void **)((char *)p + 0x4) = &D_1AD8C8;
    func_00337D78(*(void **)((char *)p + 0x2C), *(void ***)((char *)p + 0x28));
    func_00336678(p, flag);
}
#endif

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

/* func_00336BC8: seed the gadget-swap zoom animation. Store the swap flag/index
 * (a0) as a word at g_swapGadgetItemIndex+0x86, then two zoom factors: at +0x8A
 * the start scale (1.0 when a0==0, else 1.0769) and at +0x8E the end scale (1.0
 * when a0==0, else 0.9). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336BC8);
#else
/* TODO(match): functional equivalent - not byte-exact; same-symbol gp_rel /
   absolute reload wall - the original reaches +0x86 and +0x8E through the one-
   insn %gp_rel($28) form but +0x8A through the two-insn absolute %hi/%lo macro;
   cc1 cannot reproduce that asymmetric per-field addressing mode mix. */
extern s32 g_swapGadgetItemIndex;
void func_00336BC8(s32 flag) {
    char *base = (char *)&g_swapGadgetItemIndex;
    *(s32 *)(base + 0x86) = flag;
    *(f32 *)(base + 0x8A) = (flag != 0) ? 1.076923f : 1.0f;
    *(f32 *)(base + 0x8E) = (flag != 0) ? 0.9f : 1.0f;
}
#endif

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
   store, alternating two registers). 76% best.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiElementSetPos, run_cmp_suite.sh): offset-correctness oracle confirms
   the four args land at e->pos[0..3], no 5th store, scale buffer untouched. */
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

/* GuiElementShareScaleVec: rebind the element's scale vector (+0x4) to an
 * externally-owned vector. If the new pointer differs from the current scale,
 * and the element owns a pool (+0x2C) and has not yet released its own scale
 * node (the +0x18 latch is clear), free the old scale node back to the pool
 * (func_00337D78) and set the +0x18 latch; then install the new pointer. */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; the early-out compares
   compile to branch-likely (beql/bnel) with the shared-tail store sunk into the
   delay slots, plus the 8-byte-packed callee-save frame wall (original -0x20
   frame at 0x0/0x8/0x10; cc1 emits -0x30 at 0x0/0x10/0x20). 95% best. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementShareScaleVec);
#else
void GuiElementShareScaleVec(GuiElement *e, f32 *newScale) {
    if (newScale == e->scale) {
        return;
    }
    if (*(void **)((char *)e + 0x2C) != 0 && *(s32 *)((char *)e + 0x18) == 0) {
        func_00337D78(*(void **)((char *)e + 0x2C), (void **)e->scale);
        *(s32 *)((char *)e + 0x18) = 1;
    }
    e->scale = newScale;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336D20);

/* func_00336D28: rebind the element's visibility-scalar vector (+0x10) to an
 * externally-owned vector. If the new pointer differs from the current one, and
 * the element owns a pool (+0x2C) and has not yet released its own visibility
 * node (the +0x24 latch is clear), free the old node back to the pool
 * (func_00337D78) and set the +0x24 latch; then install the new pointer. The
 * +0x10/+0x24 pairing here mirrors the +0x4/+0x18 pairing in
 * GuiElementShareScaleVec. */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; same beql/bnel early-out
   + 8-byte-packed callee-save frame wall as GuiElementShareScaleVec. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336D28);
#else
void func_00336D28(GuiElement *e, f32 *newVisible) {
    if (newVisible == e->visible) {
        return;
    }
    if (*(void **)((char *)e + 0x2C) != 0 && *(s32 *)((char *)e + 0x24) == 0) {
        func_00337D78(*(void **)((char *)e + 0x2C), (void **)e->visible);
        *(s32 *)((char *)e + 0x24) = 1;
    }
    e->visible = newVisible;
}
#endif

/* GuiElementSetScale: as GuiElementSetPos for the scale vector at +0x4. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementSetScale);
#else
/* TODO(match): functional equivalent - not byte-exact; same reloaded-pointer
   CSE wall as GuiElementSetPos. 76% best.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiElementSetScale, run_cmp_suite.sh): offset-correctness oracle confirms
   the four args land at e->scale[0..3] (struct +0x4), pos buffer untouched. */
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

/* GuiElementBaseInit: construct the shared base of a GUI element. When a pool is
 * supplied (a2), stash it at +0x2C and carve five zeroed 16-byte vector blocks
 * from it (via GuiPoolAlloc + GuiPlacementNew), wiring them into the element in
 * allocation order at +0x0 (pos), +0x8 (unk08), +0x4 (scale), +0xC (color) and
 * +0x10 (visible). Then record the element tag (a1) at +0x28, zero the scalar
 * fields +0x14/+0x18/+0x1C/+0x20/+0x24, and mark the element visible. */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; the 8-byte-packed
   callee-save frame wall (the original packs s0/s1/ra into a -0x20 frame at
   0x0/0x8/0x10; this cc1 emits a -0x30 frame at 0x0/0x10/0x20), plus the
   per-block store scheduling - the original stores the block pointer into the
   element immediately, then zeroes the block (0x0..0xC) with the trailing
   0xc(v0) zero sunk into the next GuiPlacementNew delay slot; cc1 batches them
   differently. 95.9% best. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementBaseInit);
#else
void GuiElementBaseInit(GuiElement *e, s32 tag, GuiPool *pool) {
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
    GuiElementSetVisible(e, 1);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336EF8);

/* func_00336F00: shared GuiElement base destructor. Reinstall the base vtable
 * at +0x30, then (only when the element owns a pool at +0x2C) free each of its
 * four still-owned vector nodes back to the pool, guarded by per-node release
 * latches: pos(+0x0)/latch +0x14, unk08(+0x8)/latch +0x1C, scale(+0x4)/latch
 * +0x20, color(+0xC)/latch +0x18 (each free passes a0 = the pool, not the
 * element). Finally, if (flag & 1), run the dtor tail hook func_00337C48.
 * Left INCLUDE_ASM (no #else): the destructor takes a second (flag) arg, but the
 * 1-arg `func_00336F00` extern that the matched tail-callers (func_00337278 /
 * func_00337830) rely on - they call it with a1 left untouched - is part of the
 * matched (non-TARGET_NATIVE) build and cannot coexist with a 2-arg C definition
 * in the same TU. A #else body would require region-splitting that extern, which
 * the matched callers would then fail to satisfy under TARGET_NATIVE. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336F00);

/* GuiElementInitTypeB: install the TypeB GuiElement vtable (D_1ADA38) at +0x30.
 * The asm installs the base vtable first, but that store is fully overwritten by
 * this one - functionally a single store. Return value (p) is unused by callers. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementInitTypeB);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame wall
   (the GuiElementInstallBaseVtable call holds p across $16/$31 in a -0x10 frame).
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiElementInitTypeB, run_cmp_235FE8_iso.sh): offset oracle confirms +0x30
   == &D_1ADA38 and every other element byte stays the 0xAA sentinel. */
void GuiElementInitTypeB(void *p) {
    extern void *D_1ADA38;
    *(void **)((char *)p + 0x30) = &D_1ADA38;
}
#endif

/* GuiElementInit: run the base GuiElement init (forwarding tag/pool unchanged),
 * then, when a pool is present (+0x2C != 0), carve two more zeroed 16-byte
 * vector blocks from it (GuiPoolAlloc + GuiPlacementNew) into +0x34 and +0x38,
 * and set the scale vector (*(e+0x4)) to {1.0, 1.0, 1.0}. BOTH latch words are
 * zeroed regardless of pool: +0x48=0 on the pool==0 path via the branch-likely
 * delay slot (and again on the pool!=0 fall-through), and +0x44=0 at the shared
 * tail (.L0033707C) reached by both paths.
 * (The asm leaves a1/a2 untouched across the GuiElementBaseInit call, i.e. this
 * forwards the tag and pool it was called with.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementInit);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame wall
   ($16/$31 16-byte vs 8-byte slot packing) + the trailing per-block zero stores
   sunk into the GuiPoolAlloc/GuiPlacementNew jal delay slots.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900 on BOTH paths
   (cmp_GuiElementInit pool!=0, cmp_GuiElementInit_nullpool pool==0;
   run_cmp_235FE8_iso.sh): the pool!=0 oracle confirms the two pool blocks land at
   e+0x34/+0x38 zeroed and scale[0..2]=1.0; both oracles confirm +0x44 and +0x48
   end up 0 regardless of pool. */
void GuiElementInit(GuiElement *e, s32 tag, GuiPool *pool) {
    GuiElementBaseInit(e, tag, pool);
    if (*(GuiPool **)((char *)e + 0x2C) == 0) {
        /* asm: beql $4,$0 with `sw $0,0x48($16)` in the branch-likely delay slot
         * -> on the pool==0 path +0x48 is zeroed and we skip straight to the
         * shared tail that zeroes +0x44. */
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
        /* asm L41: `sw $0,0x48($16)` on the pool!=0 fall-through */
        *(s32 *)((char *)e + 0x48) = 0;
    }
    /* asm .L0033707C (shared merge): `sw $0,0x44($16)` on BOTH paths */
    *(s32 *)((char *)e + 0x44) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337090);

/* func_00337098: list-element ctor. Install the D_1ADA38 vtable at p+0x30; when
 * the pool at p+0x2C is live, free up to two pooled nodes back to it - the node
 * at p+0x34 unless the +0x44 latch is set, and the node at p+0x38 unless the
 * +0x48 latch is set (each free passes a0 = the pool, not p). Then run the
 * shared ctor func_00336F00(p, flag). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337098);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame +
   branch-likely (bnel) guard wall. */
extern void *D_1ADA38;
void func_00337098(void *p, s32 flag) {
    *(void **)((char *)p + 0x30) = &D_1ADA38;
    if (*(s32 *)((char *)p + 0x2C) != 0) {
        if (*(s32 *)((char *)p + 0x44) == 0) {
            func_00337D78(*(void **)((char *)p + 0x2C),
                          *(void ***)((char *)p + 0x34));
        }
        if (*(s32 *)((char *)p + 0x48) == 0) {
            func_00337D78(*(void **)((char *)p + 0x2C),
                          *(void ***)((char *)p + 0x38));
        }
    }
    ((void (*)(void *, s32))func_00336F00)(p, flag);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337110);

/* GuiSpriteElementDraw: if the element is visible (*(e+0x10) scalar != 0) and
 * has a live texture handle (+0x40), submit the sprite to the 2D blitter
 * func_003017F8 - position (*(e+0x0)), scale (*(e+0x4)) with y pre-scaled by the
 * global sprite y-fudge (g_swapGadgetItemIndex+0x8E), color (*(e+0xC)), the
 * +0x38 vec and the +0x40 handle. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiSpriteElementDraw);
#else
/* TODO(match): functional equivalent - not byte-exact; 92.7%. Structure, frame
   (asm barrier defeats the tail-call), branches and reloc all match; the only
   delta is -O2 instruction scheduling of the independent argument loads - the
   original loads the gp_rel y-fudge into fv0 first then scale[1] into fa3
   (mul.s fa3,fa3,fv0) and fills the jal delay slot with pos[0]; this cc1
   schedules the global load late and hoists pos[0]. Pure scheduler artifact. */
extern s32 g_swapGadgetItemIndex;
extern void func_003017F8(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                          f32 px, f32 py, f32 sx, f32 syg, f32 v38);
void GuiSpriteElementDraw(void *p) {
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
    func_003017F8(handle, color[0], scale, vec38,
                  pos[0], pos[1], scale[0],
                  scale[1] * *(f32 *)((char *)&g_swapGadgetItemIndex + 0x8E),
                  vec38[0]);
    __asm__ __volatile__("");
}
#endif

/* GuiElementSetGlyph: look up the glyph for (codepoint, font) and store the
 * resulting handle at +0x40. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementSetGlyph);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame wall
   ($16/$31 16-byte vs 8-byte slot packing).
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiElementSetGlyph, run_cmp_suite.sh): offset-correctness oracle confirms
   the lookup result lands at e+0x40 (and nowhere else) with arg order preserved
   through a deterministic GuiFontAtlasLookupGlyph mock. */
extern s32 GuiFontAtlasLookupGlyph(s32 codepoint, s32 font);
void GuiElementSetGlyph(GuiElement *e, s32 codepoint, s32 font) {
    *(s32 *)((char *)e + 0x40) = GuiFontAtlasLookupGlyph(codepoint, font);
}
#endif

/* GuiElementSetAlpha: write the alpha float through the element's +0x38 ptr. */
void GuiElementSetAlpha(GuiElement *e, f32 alpha) {
    **(f32 **)((char *)e + 0x38) = alpha;
}

/* GuiListRowElementInit: run the base GuiElement vtable install, then overwrite
 * the +0x30 vtable slot with the GuiListRow vtable. (The original also returns
 * the object in v0, but every caller discards it.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiListRowElementInit);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed two-save
   frame wall - holding the object across the GuiElementInstallBaseVtable call
   needs s0 saved alongside ra, and this cc1 lays the two saves out in a -0x20
   frame where the original packs them into -0x10.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiListRowElementInit, run_cmp_235FE8_iso.sh): offset oracle confirms
   +0x30 == &g_GuiListRowVtable and every other element byte stays sentinel. */
extern GuiElement *GuiElementInstallBaseVtable(GuiElement *e);
void GuiListRowElementInit(void *p) {
    GuiElement *e = (GuiElement *)p;
    GuiElementInstallBaseVtable(e);
    *(void **)((char *)e + 0x30) = &g_GuiListRowVtable;
}
#endif

/* GuiListElementInit: run the base GuiElement init using the 4th/5th args as its
 * tag/pool, then store the 2nd arg at +0x3C and the 3rd at +0x34, seed the
 * sentinel word +0x44 = 0x80000000 and the count/limit word +0x40 = 100. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiListElementInit);
#else
/* TODO(match): functional equivalent - not byte-exact; 4-callee-save frame wall
   ($16/$17/$18/$31 16-byte vs 8-byte slot packing) - the four held arg values
   force the deeper save frame.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiListElementInit, run_cmp_235FE8_iso.sh): offset-correctness oracle confirms
   v3C->+0x3C, v34->+0x34, +0x44=0x80000000, +0x40=100, base tag/pool forwarded. */
void GuiListElementInit(GuiElement *e, s32 v3C, s32 v34, s32 tag, GuiPool *pool) {
    GuiElementBaseInit(e, tag, pool);
    *(s32 *)((char *)e + 0x3C) = v3C;
    *(s32 *)((char *)e + 0x34) = v34;
    *(s32 *)((char *)e + 0x44) = (s32)0x80000000;
    *(s32 *)((char *)e + 0x40) = 0x64;
}
#endif

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
   wall. 76% best.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiListSetColorPair0, run_cmp_suite.sh): offset-correctness oracle
   confirms c0,c1 land at e->color[0],[1] with [2],[3] left sentinel. */
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
   wall. 76% best.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiListSetColorPair1, run_cmp_suite.sh): offset-correctness oracle
   confirms c0,c1 land at e->color[2],[3] with [0],[1] left sentinel. */
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

/* func_003374D8: run the base GuiElement init then overwrite the +0x30 vtable
 * slot with the D_1ADA18 vtable; return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003374D8);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall ($16/$31 16-byte vs 8-byte slot packing). */
extern void *D_1ADA18;
void *func_003374D8(void *p) {
    GuiElementInstallBaseVtable(p);
    *(void **)((char *)p + 0x30) = &D_1ADA18;
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337510);

/* GuiSpriteElementInit: run the base GuiElement init (forwarding tag/pool
 * unchanged), then when a pool is present carve a zeroed 16-byte vector block
 * from it (via the element's +0x2C pool) into +0x34. Always: zero the +0x34
 * vec's [0],[1] words, seed the position vector (*(e+0x0)) to {100.0, 100.0} and
 * the scale vector (*(e+0x4)) to {64.0, 64.0}, and clear +0x38.
 * (The asm forwards a1/a2 unchanged across the GuiElementBaseInit call, so the
 * pool argument doubles as the carve guard.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiSpriteElementInit);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame wall
   ($16/$17/$31 16-byte vs 8-byte slot packing) + the float-const loads scheduled
   across the reloaded +0x34/+0x0/+0x4 pointer reads.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiSpriteElementInit, run_cmp_235FE8_iso.sh): offset-correctness oracle
   confirms the +0x34 block lands zeroed, pos={100,100}, scale={64,64}, +0x38=0. */
void GuiSpriteElementInit(GuiElement *e, s32 tag, GuiPool *pool) {
    f32 *vec, *pos, *scale;
    GuiElementBaseInit(e, tag, pool);
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

/* GuiSpriteGetTextureVec: return the sprite's texture-vec pointer (+0x34). */
f32 *GuiSpriteGetTextureVec(GuiElement *e) {
    return *(f32 **)((char *)e + 0x34);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003375C8);

/* func_003375D0: install the D_1ADA18 vtable at p+0x30; if the pool at p+0x2C is
 * live and the node slot p+0x38 is still empty, free the node *(p+0x34) back to
 * it (func_00337D78); then run the shared ctor func_00336F00(p, flag). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003375D0);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame +
   branch-likely wall. */
void func_003375D0(void *p, s32 flag) {
    *(void **)((char *)p + 0x30) = &D_1ADA18;
    if (*(s32 *)((char *)p + 0x2C) != 0 && *(s32 *)((char *)p + 0x38) == 0) {
        /* free node *(p+0x34) back to the pool at *(p+0x2C) (a0 = *(p+0x2C),
           not p - the bnel that would set a0=p is nullified on this path) */
        func_00337D78(*(void **)((char *)p + 0x2C), *(void ***)((char *)p + 0x34));
    }
    ((void (*)(void *, s32))func_00336F00)(p, flag);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337630);

/* GuiSpriteSetTexture: write two int->float coords through *(e+0x34),
 * re-reading the pointer per store. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiSpriteSetTexture);
#else
/* TODO(match): functional equivalent - not byte-exact; reloaded-pointer CSE wall
   (cc1 collapses the two *(e+0x34) reloads). 90% best.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiSpriteSetTexture, run_cmp_suite.sh): offset+width oracle confirms
   (f32)u,(f32)v (cvt.s.w) land at (*(e+0x34))[0],[1] with [2],[3] sentinel and
   the +0x34 pointer preserved. */
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

/* GuiElementInitTypeC: identical to TypeB but installs the TypeC vtable
 * (D_1AD9F8) at +0x30. Return value (p) unused by callers. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementInitTypeC);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame wall
   (same install-then-overwrite shape as GuiElementInitTypeB).
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiElementInitTypeC, run_cmp_235FE8_iso.sh): offset oracle confirms +0x30
   == &D_1AD9F8 and every other element byte stays the 0xAA sentinel. */
void GuiElementInitTypeC(void *p) {
    extern void *D_1AD9F8;
    *(void **)((char *)p + 0x30) = &D_1AD9F8;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003377A8);

/* GuiTextElementInit: run the base GuiElement init (forwarding its own tag/pool
 * arguments unchanged), then set up a text element: install the D_263B10
 * glyph/format table at +0x34, mark it active (+0x38 = 1, 64-bit), clear the
 * text handle (+0x40), reset the scale vector (*(e+0x4)) to {1.0, 1.0}, and seed
 * the text params: +0x54 = 1, +0x4C = 0x200, +0x50 = 0.7f, +0x44 = 1, +0x48 = 0.
 * (The asm leaves a1/a2 untouched across the GuiElementBaseInit call, i.e. this
 * forwards the tag and pool it was called with.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiTextElementInit);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed two-save
   frame wall (target uses a -0x10 frame with s0@0x0/ra@0x8; this cc1 emits a
   -0x20 frame) plus the -fno-gcse double-reload of *(e+0x4) collapses. 49%.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiTextElementInit, run_cmp_235FE8_iso.sh): offset-correctness oracle
   confirms +0x34=&D_263B10, +0x38=(s64)1 (both words), +0x40=0, scale={1,1},
   +0x54=1, +0x4C=0x200, +0x50=0.7f (0x3F333333), +0x44=1, +0x48=0; base init
   forwards the tag to +0x28. */
extern void *D_263B10;
void GuiTextElementInit(GuiElement *e, s32 tag, GuiPool *pool) {
    f32 *scale;
    GuiElementBaseInit(e, tag, pool);
    *(void **)((char *)e + 0x34) = &D_263B10;
    *(s64 *)((char *)e + 0x38) = 1;
    *(s32 *)((char *)e + 0x40) = 0;
    scale = *(f32 **)((char *)e + 0x4);
    scale[0] = 1.0f;
    scale = *(f32 **)((char *)e + 0x4);
    scale[1] = 1.0f;
    *(s32 *)((char *)e + 0x54) = 1;
    *(s32 *)((char *)e + 0x4C) = 0x200;
    *(f32 *)((char *)e + 0x50) = 0.7f;
    *(s32 *)((char *)e + 0x44) = 1;
    *(s32 *)((char *)e + 0x48) = 0;
}
#endif

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

/* GuiFontAtlasLookupGlyph: linear-scan a font atlas's glyph table (count at
 * +0x18, 8-byte {key,value} entries at +0x1C) for the entry whose key matches
 * `codepoint`, returning its value (0 if not found).
 * NEAR-MISS 42% (best): the original keeps index-based addressing
 * (sll i,3; addu; lw) with NO first-iteration peel. ee-gcc 2.9 forces a
 * dilemma - the for/check-then-inc form peels iteration 1 inline, while the
 * inc-then-check while form strength-reduces the scan to an incrementing
 * pointer walk. Neither reproduces the original's un-peeled index-addressed
 * loop. Loop-rotation + IV strength-reduction codegen wall. */
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

/* GuiPoolAlloc: allocate one node from a GUI fixed-size pool.
 *   +0x00 base    pointer to the backing storage
 *   +0x04 capacity (byte limit)
 *   +0x08 elemSize
 *   +0x0C cursor   bump offset into the backing storage
 *   +0x10 count    live allocation count
 *   +0x14 freeList head of the singly-linked free list (next ptr at node+0)
 * If the free list is non-empty, pop its head; otherwise bump-allocate
 * base+cursor, advancing cursor by elemSize. Overflowing the capacity trips an
 * assert and returns NULL. */
struct GuiPool {
    /* 0x00 */ char *base;
    /* 0x04 */ u32 capacity;
    /* 0x08 */ u32 elemSize;
    /* 0x0C */ u32 cursor;
    /* 0x10 */ s32 count;
    /* 0x14 */ void *freeList;
};
extern void AssertFail(const char *file, s32 line, const char *expr);
/* rodata assert strings (gui pool allocator):
 *   D_1ADA78 = source file path, D_1ADAC0 = the capacity predicate. */
extern const char D_1ADA78[];
extern const char D_1ADAC0[];
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
    AssertFail(D_1ADA78, 0x53, D_1ADAC0);
    return 0;
}

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

/* func_00338A80: init two embedded type-B GUI elements (at p+0x10 and p+0x5C),
 * return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338A80);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_00338A80(void *p) {
    GuiElementInitTypeB((char *)p + 0x10);
    GuiElementInitTypeB((char *)p + 0x5C);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338AB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338CD0);

/* func_00338CD8: configure a scrolling-list widget.
 *  - Store the row-count (a1) at +0x1C4 and the page-size (a2) at +0x1B8.
 *  - Seed the widget's pos vector (*(p+0x0)) with (x,y), clear the +0x1A7 byte
 *    flag, and set the +0x1B0 "active" flag.
 *  - Build the embedded sub-list at p+0xA8: record its address at +0x1D8, run
 *    func_0027F7F8(p+0xA8, -1) and keep its handle at +0x1D4.
 *  - Initialise the widget's scale vector (*(p+0x4)) x-component to 1.0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338CD8);
#else
/* TODO(match): functional equivalent - not byte-exact; reloaded-pointer CSE
   (the original reloads *(p+0x0) before each pos store) + jal-delay-slot store
   scheduling wall. */
extern s32 func_0027F7F8(void *p, s32 mode);
void func_00338CD8(void *p, s32 a1, f32 x, f32 y, s32 a2) {
    void *sub = (char *)p + 0xA8;
    *(s32 *)((char *)p + 0x1C4) = a1;
    (*(f32 **)p)[0] = x;
    (*(f32 **)p)[1] = y;
    *(u8 *)((char *)p + 0x1A7) = 0;
    *(s32 *)((char *)p + 0x1B8) = a2;
    *(s32 *)((char *)p + 0x1D8) = (s32)sub;
    *(s32 *)((char *)p + 0x1D4) = func_0027F7F8(sub, -1);
    *(s32 *)((char *)p + 0x1B0) = 1;
    (*(f32 **)((char *)p + 0x4))[0] = 1.0f;
}
#endif

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

/* func_003396E8: map a small selector to a scale constant - selector 0 -> 0.7,
 * 1 -> 0.8, anything else -> 1.0. The first argument is ignored. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003396E8);
#else
/* TODO(match): functional equivalent - not byte-exact; multi-way branch +
   float-constant materialization wall. */
f32 func_003396E8(s32 unused, s32 sel) {
    if (sel == 1) {
        return 0.8f;
    }
    if (sel == 0) {
        return 0.7f;
    }
    return 1.0f;
}
#endif

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

/* func_0033A048: init the screen's eight embedded sub-elements at their fixed
 * offsets (four type-B then four type-C), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A048);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033A048(void *p) {
    GuiElementInitTypeB((char *)p + 0x8);
    GuiElementInitTypeB((char *)p + 0x54);
    GuiElementInitTypeB((char *)p + 0xA0);
    GuiElementInitTypeB((char *)p + 0xEC);
    GuiElementInitTypeC((char *)p + 0x138);
    GuiElementInitTypeC((char *)p + 0x190);
    GuiElementInitTypeC((char *)p + 0x1E8);
    GuiElementInitTypeC((char *)p + 0x240);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A0B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiConfirmPopupInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiConfirmPopupTick);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiConfirmPopupDraw);

/* func_0033A640: init the embedded dialog-box (p+0x8) and a type-B element
 * (p+0x2D8), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A640);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033A640(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    GuiElementInitTypeB((char *)p + 0x2D8);
    return p;
}
#endif

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

/* func_0033A7E0: seed this confirm-dialog screen's text - set the dialog box
 * (p+0x8) title/body/footer to localized strings 0x2C34/0x2C32/0x2BE5, zero its
 * scale, and clear the +0x32C "text ready" flag (the sibling of func_0033A860,
 * which raises that flag with the 0x2C33 title variant). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A7E0);
#else
/* TODO(match): functional equivalent - not byte-exact; 4-callee-save frame
   wall. */
extern s32 GetLocalizedString(s32 id);
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
extern void GuiDialogBoxSetScale(void *p, f32 scale);
void func_0033A7E0(void *p) {
    s32 t0 = GetLocalizedString(0x2C34);
    s32 t1 = GetLocalizedString(0x2C32);
    s32 t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)p + 0x8, t0, t1, t2);
    GuiDialogBoxSetScale((char *)p + 0x8, 0.0f);
    *(s32 *)((char *)p + 0x32C) = 0;
}
#endif

/* func_0033A860: populate this confirm-dialog screen's text - set the dialog box
 * (p+0x8) title/body/footer to localized strings 0x2C33/0x2C32/0x2BE5, zero its
 * scale, raise the +0x32C "text ready" flag, and return 1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A860);
#else
/* TODO(match): functional equivalent - not byte-exact; 4-callee-save frame
   wall. */
extern s32 GetLocalizedString(s32 id);
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
extern void GuiDialogBoxSetScale(void *p, f32 scale);
s32 func_0033A860(void *p) {
    s32 t0 = GetLocalizedString(0x2C33);
    s32 t1 = GetLocalizedString(0x2C32);
    s32 t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)p + 0x8, t0, t1, t2);
    GuiDialogBoxSetScale((char *)p + 0x8, 0.0f);
    *(s32 *)((char *)p + 0x32C) = 1;
    return 1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A8E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A8F0);

/* func_0033A9F8: draw this confirm-dialog screen. Sets the dialog box's (p+0x8)
 * +0x2C4 footer-visible flag to 1 when any global menu lock (D_1A8C88 / D_1A8C8C)
 * is engaged or the local +0x330 flag is set; additionally sets it to 1 (with
 * arg 1) when the +0x32C "text ready" flag equals 1. Then draws the box
 * (func_0033BF90) and the background sprite at p+0x2D8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A9F8);
#else
/* TODO(match): functional equivalent - not byte-exact; branch-likely (beql)
   guard + reloaded-flag CSE wall. */
extern s32 D_1A8C88, D_1A8C8C;
extern void func_0033BE68(void *p, s32 v);
void func_0033A9F8(void *p) {
    void *box = (char *)p + 0x8;
    if (D_1A8C88 != 0 || D_1A8C8C != 0 || *(s32 *)((char *)p + 0x330) != 0) {
        func_0033BE68(box, 0);
    }
    if (*(s32 *)((char *)p + 0x32C) == 1) {
        func_0033BE68(box, 1);
    }
    func_0033BF90(box);
    GuiSpriteElementDraw((char *)p + 0x2D8);
}
#endif

/* func_0033AA80: init the level-info panel's thirteen embedded sub-elements at
 * their fixed offsets (four type-B at p+0x8/+0x54/+0xA0/+0xEC, then nine type-C
 * at +0x138/+0x190/+0x1E8/+0x240/+0x298/+0x2F0/+0x348/+0x3A0/+0x3F8), return the
 * object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033AA80);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033AA80(void *p) {
    GuiElementInitTypeB((char *)p + 0x8);
    GuiElementInitTypeB((char *)p + 0x54);
    GuiElementInitTypeB((char *)p + 0xA0);
    GuiElementInitTypeB((char *)p + 0xEC);
    GuiElementInitTypeC((char *)p + 0x138);
    GuiElementInitTypeC((char *)p + 0x190);
    GuiElementInitTypeC((char *)p + 0x1E8);
    GuiElementInitTypeC((char *)p + 0x240);
    GuiElementInitTypeC((char *)p + 0x298);
    GuiElementInitTypeC((char *)p + 0x2F0);
    GuiElementInitTypeC((char *)p + 0x348);
    GuiElementInitTypeC((char *)p + 0x3A0);
    GuiElementInitTypeC((char *)p + 0x3F8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033AB10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiLevelInfoPanelInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033AF68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiLevelInfoPanelTick);

/* func_0033B428: draw the map-screen panel - only when the +0x490 "panel built"
 * flag is set. Draws the two background sprites (p+0x8, p+0x54), pushes two GS
 * register packets (TEST/0x42=0x44 then SCISSOR/0x47=0xB), renders the map layer
 * (MapDraw(0,1) + func_002DBC98(0)), draws the two foreground sprites (p+0xA0,
 * p+0xEC) and the nine text labels at +0x138 (stride 0x58). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B428);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame +
   branch-likely (beql) guard wall. */
extern void AppendGsRegPacket(s32 reg, s32 val);
extern void MapDraw(s32 a, s32 b);
extern void func_002DBC98(s32 a);
extern void GuiTextElementDraw(void *e);
void func_0033B428(void *p) {
    if (*(s32 *)((char *)p + 0x490) != 0) {
        GuiSpriteElementDraw((char *)p + 0x8);
        GuiSpriteElementDraw((char *)p + 0x54);
        AppendGsRegPacket(0x42, 0x44);
        AppendGsRegPacket(0x47, 0xB);
        MapDraw(0, 1);
        func_002DBC98(0);
        GuiSpriteElementDraw((char *)p + 0xA0);
        GuiSpriteElementDraw((char *)p + 0xEC);
        GuiTextElementDraw((char *)p + 0x138);
        GuiTextElementDraw((char *)p + 0x190);
        GuiTextElementDraw((char *)p + 0x1E8);
        GuiTextElementDraw((char *)p + 0x240);
        GuiTextElementDraw((char *)p + 0x298);
        GuiTextElementDraw((char *)p + 0x2F0);
        GuiTextElementDraw((char *)p + 0x348);
        GuiTextElementDraw((char *)p + 0x3A0);
        GuiTextElementDraw((char *)p + 0x3F8);
    }
}
#endif

/* func_0033B4E8: init the map-screen panel's six embedded sub-elements - four
 * type-B elements (p+0x4, stride 0x4C), the D_1ADA18 widget at +0x134, and one
 * type-C element at +0x170; return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B4E8);
#else
/* TODO(match): functional equivalent - not byte-exact; 4-callee-save frame
   wall. */
void *func_0033B4E8(void *p) {
    GuiElementInitTypeB((char *)p + 0x4);
    GuiElementInitTypeB((char *)p + 0x50);
    GuiElementInitTypeB((char *)p + 0x9C);
    GuiElementInitTypeB((char *)p + 0xE8);
    func_003374D8((char *)p + 0x134);
    GuiElementInitTypeC((char *)p + 0x170);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B560);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiProgressBarWidgetInit);

/* func_0033B6D0: when the +0x21C flag is set, position the element at p+0x170
 * from the anchor vector at *(p+0x20C): x = anchor[0] + D_1ADCE0; y = anchor[1]
 * + D_1ADCE4; z = w = 0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B6D0);
#else
/* TODO(match): functional equivalent - not byte-exact; gp_rel/absolute float
   constant addressing mix wall. */
extern f32 D_1ADCE0, D_1ADCE4;
void func_0033B6D0(void *p) {
    if (*(s32 *)((char *)p + 0x21C) != 0) {
        f32 *anchor = *(f32 **)((char *)p + 0x20C);
        GuiElementSetPos((GuiElement *)((char *)p + 0x170),
                         anchor[0] + D_1ADCE0, anchor[1] + D_1ADCE4,
                         0.0f, 0.0f);
    }
}
#endif

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

/* GuiDialogBoxInitElements: build the dialog-box sub-elements over p - 5 TypeB
 * border elements (p+0xC, stride 0x4C) then 3 TypeC text rows (p+0x198, +0x1F0,
 * +0x248). All field writes happen inside the TypeB/TypeC ctors. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxInitElements);
#else
void GuiDialogBoxInitElements(void *p) {
    char *base = (char *)p;
    s32 i;
    for (i = 0; i < 5; i++) {
        GuiElementInitTypeB(base + 0xC + i * 0x4C);
    }
    GuiElementInitTypeC(base + 0x198);
    GuiElementInitTypeC(base + 0x1F0);
    GuiElementInitTypeC(base + 0x248);
}
#endif

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

/* func_0033BE70: lay out the dialog box's five body rows then (when the +0x2C0
 * "show border labels" flag is set) its three border-relative text labels.
 *  - For each of the five rows i (element at p+0xC + i*0x4C, row-active flag is
 *    the halfword at p+0x188 + i*2): when active, anchor the row at the dialog's
 *    own vector (*(p+0x0)) origin and apply the shared alpha at *(p+0x2A4).
 *  - When *(p+0x2C0) is set, place the title (p+0x198), body (p+0x248) and
 *    footer (p+0x1F0) at the origin offset by the (x,y) bound pairs at
 *    +0x2A8/+0x2AC, +0x2B8/+0x2BC and +0x2B0/+0x2B4 respectively.
 * The flags second argument is unused by this method. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033BE70);
#else
/* TODO(match): functional equivalent - not byte-exact; 4-callee-save + $f20
   saved-FPR frame wall. */
void func_0033BE70(void *p, s32 flags) {
    s32 i;
    f32 *origin;
    GuiElement *row = (GuiElement *)((char *)p + 0xC);
    s16 *active = (s16 *)((char *)p + 0x188);
    (void)flags;
    for (i = 0; i <= 4; i++) {
        if (active[i] != 0) {
            origin = *(f32 **)p;
            GuiElementSetPos(row, origin[0], origin[1], 0.0f, 0.0f);
            GuiElementSetAlpha(row, *(f32 *)((char *)p + 0x2A4));
        }
        row = (GuiElement *)((char *)row + 0x4C);
    }
    if (*(s32 *)((char *)p + 0x2C0) != 0) {
        origin = *(f32 **)p;
        GuiElementSetPos((GuiElement *)((char *)p + 0x198),
                         origin[0] + *(f32 *)((char *)p + 0x2A8),
                         origin[1] + *(f32 *)((char *)p + 0x2AC), 0.0f, 0.0f);
        origin = *(f32 **)p;
        GuiElementSetPos((GuiElement *)((char *)p + 0x248),
                         origin[0] + *(f32 *)((char *)p + 0x2B8),
                         origin[1] + *(f32 *)((char *)p + 0x2BC), 0.0f, 0.0f);
        origin = *(f32 **)p;
        GuiElementSetPos((GuiElement *)((char *)p + 0x1F0),
                         origin[0] + *(f32 *)((char *)p + 0x2B0),
                         origin[1] + *(f32 *)((char *)p + 0x2B4), 0.0f, 0.0f);
    }
}
#endif

/* func_0033BF90: draw the dialog box's five body rows then its three border
 * text labels.
 *  - For each of the five rows i: when the row-active halfword at p+0x188 + i*2
 *    is set, dispatch the row object at *(p+0x3C + i*0x4C) through its vtable -
 *    call (*obj->draw)(rowBase + obj->offset) where rowBase = p+0xC + i*0x4C,
 *    obj->offset is the halfword at obj+0x8 and obj->draw the fn-ptr at obj+0xC.
 *  - When *(p+0x2C0) is set draw the title (p+0x198); additionally when
 *    *(p+0x2C4) is set draw the body (p+0x248) - but only if *(p+0x2CC) is set -
 *    and the footer (p+0x1F0) whenever *(p+0x2C8) is set. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033BF90);
#else
/* TODO(match): functional equivalent - not byte-exact; 5-callee-save frame +
   branch-likely (beql) guard chain + vtable-dispatch loop wall. */
extern void GuiTextElementDraw(void *e);
void func_0033BF90(void *p) {
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
        GuiTextElementDraw((char *)p + 0x198);
        if (*(s32 *)((char *)p + 0x2C4) != 0) {
            s32 footer = *(s32 *)((char *)p + 0x2C8);
            if (*(s32 *)((char *)p + 0x2CC) != 0) {
                GuiTextElementDraw((char *)p + 0x248);
                footer = *(s32 *)((char *)p + 0x2C8);
            }
            if (footer != 0) {
                GuiTextElementDraw((char *)p + 0x1F0);
            }
        }
    }
}
#endif

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

/* GuiDialogBoxSetText3: assign the three dialog-box text labels - the title
 * (p+0x198 <- t0), the body (p+0x248 <- t1) and the prompt (p+0x1F0 <- t2). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxSetText3);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame
   wall. */
extern void GuiElementSetText(GuiElement *e, s32 text);
void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2) {
    GuiElementSetText((GuiElement *)((char *)p + 0x198), t0); /* title */
    GuiElementSetText((GuiElement *)((char *)p + 0x248), t1); /* body */
    GuiElementSetText((GuiElement *)((char *)p + 0x1F0), t2); /* footer */
}
#endif

/* GuiDialogBoxSetScale: store the dialog-box scale float at +0x2A4. */
void GuiDialogBoxSetScale(void *p, f32 scale) {
    *(f32 *)((char *)p + 0x2A4) = scale;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C0F8);

/* func_0033C100: init the screen's ten embedded sub-elements at their fixed
 * offsets (five type-B at p+0/+0x4C/+0x98/+0xE4/+0x130, three type-C at
 * +0x1D8/+0x230/+0x288, then the two D_1ADA18 widgets at +0x2E0/+0x31C), return
 * the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C100);
#else
/* TODO(match): functional equivalent - not byte-exact; 4-callee-save frame
   wall. */
void *func_0033C100(void *p) {
    GuiElementInitTypeB(p);
    GuiElementInitTypeB((char *)p + 0x4C);
    GuiElementInitTypeB((char *)p + 0x98);
    GuiElementInitTypeB((char *)p + 0xE4);
    GuiElementInitTypeB((char *)p + 0x130);
    GuiElementInitTypeC((char *)p + 0x1D8);
    GuiElementInitTypeC((char *)p + 0x230);
    GuiElementInitTypeC((char *)p + 0x288);
    func_003374D8((char *)p + 0x2E0);
    func_003374D8((char *)p + 0x31C);
    return p;
}
#endif

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

/* func_0033D320: re-layout this dialog-box screen and (on first open) chime.
 *  - Run the dialog-box layout func_0033BE70 on the embedded box at p+0x8.
 *  - Re-feed it the two floats of the vector at *(p+0x2DC) via func_0033C060.
 *  - When (flags & 0x40) is set AND the +0x2D8 "already opened" latch is still
 *    clear, toggle the global one-shot flag D_1A7BB9, play the open chime
 *    (PlayGlobalSound(4,0,0)) and rebuild the camera projection.
 * Returns bit 6 of the flags argument. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D320);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame +
   branch-likely (beql) guard wall. */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
extern void BuildCameraProjection(void);
extern u8 D_1A7BB9;
s32 func_0033D320(void *p, s32 flags) {
    void *box = (char *)p + 0x8;
    f32 *v;
    func_0033BE70(box, flags);
    v = *(f32 **)((char *)p + 0x2DC);
    func_0033C060(box, v[0], v[1]);
    if ((flags & 0x40) != 0 && *(s32 *)((char *)p + 0x2D8) == 0) {
        D_1A7BB9 = (D_1A7BB9 < 1);
        PlayGlobalSound(4, 0, 0);
        BuildCameraProjection();
    }
    return (flags >> 6) & 1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D3C0);

/* func_0033D478: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D478);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall ($16/$31 16-byte vs 8-byte slot packing). */
void *func_0033D478(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D4A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D4B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D5D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D780);

/* func_0033DA00: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DA00);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033DA00(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DA30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxVariantBInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DB60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DC68);

/* func_0033DDC8: init the embedded dialog-box (p+0x8) and two list-row elements
 * (p+0x2DC, p+0x324), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DDC8);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033DDC8(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    GuiListRowElementInit((char *)p + 0x2DC);
    GuiListRowElementInit((char *)p + 0x324);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DE08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DE10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E070);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E308);

/* func_0033E488: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E488);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033E488(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E4B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E4C0);

/* func_0033E5E8: re-layout this dialog-box screen and (on first open) chime.
 *  - Run the dialog-box layout func_0033BE70 on the embedded box at p+0x8.
 *  - Re-feed it the two floats of the vector at *(p+0x2DC) via func_0033C060.
 *  - When (flags & 0x40) is set AND the +0x2D8 "already opened" latch is still
 *    clear, play the open chime (PlayGlobalSound(4,0,0)) and toggle the global
 *    one-shot flag D_1A7B9E (set it to !D_1A7B9E).
 * Returns bit 6 of the flags argument. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E5E8);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame +
   branch-likely (beql) guard wall. */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
extern u8 D_1A7B9E;
s32 func_0033E5E8(void *p, s32 flags) {
    void *box = (char *)p + 0x8;
    f32 *v;
    func_0033BE70(box, flags);
    v = *(f32 **)((char *)p + 0x2DC);
    func_0033C060(box, v[0], v[1]);
    if ((flags & 0x40) != 0 && *(s32 *)((char *)p + 0x2D8) == 0) {
        PlayGlobalSound(4, 0, 0);
        D_1A7B9E = (D_1A7B9E < 1);
    }
    return (flags >> 6) & 1;
}
#endif

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

/* func_0033EB20: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EB20);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033EB20(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EB50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EB58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EC80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033ED18);

/* func_0033EDD0: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EDD0);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033EDD0(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EE00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EE08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EF30);

/* func_0033EFC8: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EFC8);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033EFC8(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EFF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F000);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F128);

/* func_0033F1C0: init the embedded dialog-box (p+0x8) and the GuiWidget at
 * p+0x2E0 (func_00348BD0), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F1C0);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033F1C0(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    func_00348BD0((char *)p + 0x2E0);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F1F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F200);

/* func_0033F360: call func_0033F398(p) to get a row index, then return the
 * row's address: base (+0x3B8) + index * 0x14 (20-byte stride). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F360);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void func_0033F398(void *p);
s32 func_0033F360(void *p) {
    s32 idx = ((s32 (*)(void *))func_0033F398)(p);
    return *(s32 *)((char *)p + 0x3B8) + idx * 0x14;
}
#endif

/* func_0033F398: forward p+0x2E0 to func_00348E60. */
void func_0033F398(void *p) {
    func_00348E60((char *)p + 0x2E0);
    __asm__ __volatile__("");
}

/* func_0033F3B8: init the GuiWidget at p+0x2E0, then record the row-data
 * pointer (arg) at p+0x3B8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F3B8);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame
   wall ($16/$17/$31 16-byte vs 8-byte slot packing). */
extern void func_00348DA0(void *p);
void func_0033F3B8(void *p, s32 rows) {
    func_00348DA0((char *)p + 0x2E0);
    *(s32 *)((char *)p + 0x3B8) = rows;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F3F0);

/* func_0033F478: tear down the embedded dialog-box (p+0x8), then reconfigure the
 * GuiWidget at p+0x2E0 with the per-language resource D_1ADEB0[g_currentLanguage]
 * (func_00348E58) and re-init it (func_00348E70). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F478);
#else
/* TODO(match): functional equivalent - not byte-exact; gp_rel(D_1ADEB0) vs
   absolute(g_currentLanguage) addressing mix + 2-callee-save frame wall. */
extern u8 g_currentLanguage;
extern s32 D_1ADEB0[];
void func_0033F478(void *p) {
    void *w = (char *)p + 0x2E0;
    func_0033BF90((char *)p + 0x8);
    func_00348E58(w, D_1ADEB0[g_currentLanguage]);
    func_00348E70(w);
}
#endif

/* func_0033F4D0: init the embedded dialog-box (p+0x8) and a type-C element
 * (p+0x2E0), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F4D0);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033F4D0(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    GuiElementInitTypeC((char *)p + 0x2E0);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F508);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F510);

/* func_0033F610: forward the embedded dialog-box (p+0x8) to func_0033BE70, then
 * push the two floats from the vector at *(p+0x2D8) into it via func_0033C060.
 * Returns bit 6 of the flags argument. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F610);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame
   wall. */
extern void func_0033BE70(void *p, s32 flags);
extern void func_0033C060(void *p, f32 a, f32 b);
s32 func_0033F610(void *p, s32 flags) {
    f32 *v;
    func_0033BE70((char *)p + 0x8, flags);
    v = *(f32 **)((char *)p + 0x2D8);
    func_0033C060((char *)p + 0x8, v[0], v[1]);
    return (flags >> 6) & 1;
}
#endif

/* func_0033F670: forward p+0x8 to func_0033BF90. */
void func_0033F670(void *p) {
    func_0033BF90((char *)p + 0x8);
    __asm__ __volatile__("");
}

/* func_0033F690: init the screen's thirteen embedded sub-elements (the dialog
 * box at +0x8, the D_1ADA18 widget at +0x2DC, three type-C, one type-B, then
 * seven more type-C), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F690);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_0033F690(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    func_003374D8((char *)p + 0x2DC);
    GuiElementInitTypeC((char *)p + 0x318);
    GuiElementInitTypeC((char *)p + 0x370);
    GuiElementInitTypeC((char *)p + 0x3C8);
    GuiElementInitTypeB((char *)p + 0x420);
    GuiElementInitTypeC((char *)p + 0x470);
    GuiElementInitTypeC((char *)p + 0x4C8);
    GuiElementInitTypeC((char *)p + 0x520);
    GuiElementInitTypeC((char *)p + 0x578);
    GuiElementInitTypeC((char *)p + 0x5D0);
    GuiElementInitTypeC((char *)p + 0x628);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F718);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiStatsPanelScreenInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FAB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FCE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FDD8);

/* func_0033FEF8: draw one composite screen - run the body builder at p+0x8, blit
 * the menu backdrop (func_002DBC98(0)), draw its three header text rows, the
 * inner panel (func_00337630 at p+0x2DC), a sprite (p+0x420), two footer text
 * rows, then the trailing builder func_0033FDD8(p).
 * NEAR-MISS 93.75%: body is exact; only the prologue/epilogue differ. The
 * original packs its two saved regs ($16,$31) into 8-byte slots in a 0x10 frame
 * (later-cc1 codegen); our pinned ee-gcc 2.9 over-allocates a 0x20 frame with
 * 16-byte slots. The 16-byte-slot-vs-8-byte-slot frame wall (whole 2-GPR-save
 * class in this unit). */
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

/* func_00341C28: init the screen's six embedded sub-elements at their fixed
 * offsets (two type-B, one type-C, one type-B, a list-row, and the D_1ADA18
 * widget), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341C28);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_00341C28(void *p) {
    GuiElementInitTypeB(p);
    GuiElementInitTypeB((char *)p + 0x4C);
    GuiElementInitTypeC((char *)p + 0x98);
    GuiElementInitTypeB((char *)p + 0xF0);
    GuiListRowElementInit((char *)p + 0x13C);
    func_003374D8((char *)p + 0x184);
    return p;
}
#endif

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

/* func_00342468: look up an entry from the D_265250 table for column
 * sel=*(p+0x31C): table[sel*8 + count] where count=*(p+sel*4+0x1B8). For the
 * special column 2, when the extras flag is clear and the +0x1C0 counter has
 * reached 3+, the entry is suppressed (returns 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342468);
#else
/* TODO(match): functional equivalent - not byte-exact; movz conditional-move +
   absolute table addressing wall. */
extern s32 D_265250[];
extern u8 g_miscExtras;
s32 func_00342468(void *p) {
    s32 sel = *(s32 *)((char *)p + 0x31C);
    s32 count = *(s32 *)((char *)p + sel * 4 + 0x1B8);
    s32 result = D_265250[sel * 8 + count];
    if (sel == 2 && g_miscExtras == 0 && *(s32 *)((char *)p + 0x1C0) >= 3) {
        result = 0;
    }
    return result;
}
#endif

/* func_003424C8: resolve a row address for the selected column (*(p+0x31C)).
 * Column 0 -> base(+0x310) + count(+0x1B8)*0x14; column 1 -> base(+0x314) +
 * count(+0x1BC)*0x14; otherwise -> base(+0x318) + count(+sel*4+0x1B8)*0x14
 * (the +0x1B8 array runs parallel to the column index). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003424C8);
#else
/* TODO(match): functional equivalent - not byte-exact; switch-style multi-way
   branch wall. */
s32 func_003424C8(void *p) {
    s32 sel = *(s32 *)((char *)p + 0x31C);
    if (sel == 0) {
        return *(s32 *)((char *)p + 0x310) + *(s32 *)((char *)p + 0x1B8) * 0x14;
    }
    if (sel == 1) {
        return *(s32 *)((char *)p + 0x314) + *(s32 *)((char *)p + 0x1BC) * 0x14;
    }
    return *(s32 *)((char *)p + 0x318) +
           *(s32 *)((char *)p + sel * 4 + 0x1B8) * 0x14;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342520);

/* func_00342670: position the embedded element at p+0x130 from the anchor
 * vector at *(p+0x228) and the per-column index *(p+sel*4+0x1B8) where
 * sel=*(p+0x31C): x = D_1AE0B0 + anchor[0]; y = D_1AE0B4 + D_1AE0B8*idx +
 * anchor[1]; z = w = 0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342670);
#else
/* TODO(match): functional equivalent - not byte-exact; gp_rel/absolute float
   constant addressing mix wall. */
extern f32 D_1AE0B0, D_1AE0B4, D_1AE0B8;
void func_00342670(void *p) {
    s32 sel = *(s32 *)((char *)p + 0x31C);
    f32 *anchor = *(f32 **)((char *)p + 0x228);
    f32 idx = (f32)*(s32 *)((char *)p + sel * 4 + 0x1B8);
    f32 x = D_1AE0B0 + anchor[0];
    f32 y = (D_1AE0B4 + D_1AE0B8 * idx) + anchor[1];
    GuiElementSetPos((GuiElement *)((char *)p + 0x130), x, y, 0.0f, 0.0f);
}
#endif

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

/* func_00342BA0: init the GuiWidget at p+0x10 and two type-B elements
 * (p+0xEC, p+0x138), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342BA0);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_00342BA0(void *p) {
    func_00348BD0((char *)p + 0x10);
    GuiElementInitTypeB((char *)p + 0xEC);
    GuiElementInitTypeB((char *)p + 0x138);
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342BE0);

/* func_00342BE8: store an int at +0x8. */
void func_00342BE8(void *p, s32 v) {
    *(s32 *)((char *)p + 0x8) = v;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiHelpPromptWidgetInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342D60);

/* func_00342D68: call func_00342DA0(p) to get a row index, then return the
 * row's address: base (+0xE8) + index * 0x14 (20-byte stride). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342D68);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void func_00342DA0(void *p);
s32 func_00342D68(void *p) {
    s32 idx = ((s32 (*)(void *))func_00342DA0)(p);
    return *(s32 *)((char *)p + 0xE8) + idx * 0x14;
}
#endif

/* func_00342DA0: forward p+0x10 to func_00348E60. */
void func_00342DA0(void *p) {
    func_00348E60((char *)p + 0x10);
    __asm__ __volatile__("");
}

/* func_00342DC0: init the GuiWidget at p+0x10, then record the row-data
 * pointer (arg) at p+0xE8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342DC0);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame
   wall. */
void func_00342DC0(void *p, s32 rows) {
    func_00348DA0((char *)p + 0x10);
    *(s32 *)((char *)p + 0xE8) = rows;
}
#endif

/* func_00342DF8: refresh the GuiWidget at p+0x10 (func_00348CB8), then re-feed
 * it the two floats from the vector at *(p+0x0) via func_00348E28. Returns 0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342DF8);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame
   wall. */
s32 func_00342DF8(void *p) {
    void *w = (char *)p + 0x10;
    func_00348CB8(w);
    {
        f32 *v = *(f32 **)p;
        func_00348E28(w, v[0], v[1]);
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342E48);

/* func_00342FD8: init the screen's six embedded sub-elements (four type-B, the
 * D_1ADA18 widget at +0x130, and the GuiWidget at +0x188), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342FD8);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_00342FD8(void *p) {
    GuiElementInitTypeB(p);
    GuiElementInitTypeB((char *)p + 0x4C);
    GuiElementInitTypeB((char *)p + 0x98);
    GuiElementInitTypeB((char *)p + 0xE4);
    func_003374D8((char *)p + 0x130);
    func_00348BD0((char *)p + 0x188);
    return p;
}
#endif

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

/* func_003432D8: position the embedded element at p+0xE4. Reads the anchor
 * vector at *(p+0x180): x = D_1AE150 + anchor[0]; y = D_1AE154 +
 * D_1AE158*(float)(*(p+0x16C)) + anchor[1]; z = w = 0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003432D8);
#else
/* TODO(match): functional equivalent - not byte-exact; gp_rel/absolute float
   constant addressing mix wall. */
extern f32 D_1AE150, D_1AE154, D_1AE158;
void func_003432D8(void *p) {
    f32 *anchor = *(f32 **)((char *)p + 0x180);
    f32 idx = (f32)*(s32 *)((char *)p + 0x16C);
    f32 x = D_1AE150 + anchor[0];
    f32 y = (D_1AE154 + D_1AE158 * idx) + anchor[1];
    GuiElementSetPos((GuiElement *)((char *)p + 0xE4), x, y, 0.0f, 0.0f);
}
#endif

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

/* func_00343668: when the +0x170 flag is set, draw the two sprite elements
 * (p+0x0 and p+0x4C) and run the three sub-draws (func_003434C8/00343558/
 * 00343578). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343668);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame +
   branch-likely guard wall. */
extern void func_003434C8(void *p);
extern void func_00343558(void *p);
extern void func_00343578(void *p);
void func_00343668(void *p) {
    if (*(s32 *)((char *)p + 0x170) != 0) {
        GuiSpriteElementDraw(p);
        GuiSpriteElementDraw((char *)p + 0x4C);
        func_003434C8(p);
        func_00343558(p);
        func_00343578(p);
    }
}
#endif

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

/* func_00343E80(view, records): build the list view's visible row set. Walk
 * `records` (stride 0xA, signed item-id at +0x6) up to records+0xFA; for each
 * owned item (g_inventoryOwned[id] != 0) append the id to the view's row array
 * at +0x28 (capped at 0x19 entries, count at +0x10), then set the initial
 * highlighted row +0x14 = count/2 and run layout func_00343888(view, 0).
 * BLOCKED: the body needs a 2-arg (view, records) prototype, but the
 * already-matched thin forwarder func_00344458 relies on func_00343E80 being
 * seen as 1-arg so it passes its own $a1 through untouched. Promoting the arity
 * here regresses func_00344458's byte-match - left INCLUDE_ASM to preserve it.
 * (Body is otherwise an integer ownership-filter loop; beql/bgezl branch-likely
 * placement is the secondary scheduling concern.) */
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

/* func_00344110: init the screen's eight embedded sub-elements at their fixed
 * offsets (three type-B, the D_1ADA18 widget, three type-C, and the identity
 * widget func_003436C0), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344110);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
extern void *func_003436C0(void *p);
void *func_00344110(void *p) {
    GuiElementInitTypeB(p);
    GuiElementInitTypeB((char *)p + 0x4C);
    GuiElementInitTypeB((char *)p + 0x98);
    func_003374D8((char *)p + 0xE4);
    GuiElementInitTypeC((char *)p + 0x1B0);
    GuiElementInitTypeC((char *)p + 0x208);
    GuiElementInitTypeC((char *)p + 0x260);
    func_003436C0((char *)p + 0x2C8);
    return p;
}
#endif

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

/* func_003448C0: init the info-panel screen's fourteen embedded sub-elements at
 * their fixed offsets (six type-B at p+0x0/+0x4C/+0x98/+0xE4/+0x130/+0x17C, two
 * type-C at +0x1C8/+0x220, the D_1ADA18 widget at +0x278, then five more type-C
 * at +0x2B8/+0x310/+0x368/+0x3C0/+0x418), return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003448C0);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame
   wall. */
void *func_003448C0(void *p) {
    GuiElementInitTypeB(p);
    GuiElementInitTypeB((char *)p + 0x4C);
    GuiElementInitTypeB((char *)p + 0x98);
    GuiElementInitTypeB((char *)p + 0xE4);
    GuiElementInitTypeB((char *)p + 0x130);
    GuiElementInitTypeB((char *)p + 0x17C);
    GuiElementInitTypeC((char *)p + 0x1C8);
    GuiElementInitTypeC((char *)p + 0x220);
    func_003374D8((char *)p + 0x278);
    GuiElementInitTypeC((char *)p + 0x2B8);
    GuiElementInitTypeC((char *)p + 0x310);
    GuiElementInitTypeC((char *)p + 0x368);
    GuiElementInitTypeC((char *)p + 0x3C0);
    GuiElementInitTypeC((char *)p + 0x418);
    return p;
}
#endif

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

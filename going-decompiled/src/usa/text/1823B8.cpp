#include "common.h"
#include "vec.h"

extern u64 GetUiTextureTex0(s32 id);
extern s32 D_239A90[];

/* Billboard sprite context: the four corners, colours and STs that
 * func_00282868 (DrawBillboardSprite) fills, and the GS register block that
 * func_00282798 fills; ProjectAndClipBillboardQuad (0x281540) / 0x281B04 emit
 * it (FACT #5657): 0x70 = CLAMP_1, 0x78 = TEX0_1 (from GetUiTextureTex0),
 * 0x80 = TEX1_1, 0x88 = ALPHA_1 (A/B/C/D blend selectors from the table at
 * D_239A90, stride 0x14, with FIX in bits 32..39). */
typedef struct UiSpritePacket {
    Vec4 corner[4]; /* 0x00 quad corners, model space (func_00282868 writes) */
    u32 colour[4];  /* 0x40 per-corner RGBA */
    f32 st[4][2];   /* 0x50 per-corner texture S,T */
    u64 clamp;  /* 0x70 CLAMP_1, cleared (REPEAT/REPEAT) */
    u64 tex0;   /* 0x78 TEX0_1 from GetUiTextureTex0(texId) */
    u64 tex1;   /* 0x80 TEX1_1, fixed 0xFF9000000260 */
    u64 alpha;  /* 0x88 ALPHA_1 from D_239A90[blend*5] OR (fix<<32) */
} UiSpritePacket;

/* Size pin. DrawBillboardSprite keeps the context at sp+0 and its basis matrix
 * at sp+0x90, so 0x90 is the whole context there. func_00282798
 * (InitBillboardSpriteState) writes only the GS-register sub-block at
 * 0x70..0x8F; DrawBillboardSprite writes colour 0x40..0x4C, st 0x50..0x6C and
 * corner 0x00..0x30 (all from the asm). */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(UiSpritePacket) == 0x90, "UiSpritePacket bindable view 0x90");
_Static_assert(__builtin_offsetof(UiSpritePacket, colour) == 0x40, "colour");
_Static_assert(__builtin_offsetof(UiSpritePacket, st)     == 0x50, "st");
_Static_assert(__builtin_offsetof(UiSpritePacket, clamp)  == 0x70, "clamp");
_Static_assert(__builtin_offsetof(UiSpritePacket, tex0)  == 0x78, "tex0");
_Static_assert(__builtin_offsetof(UiSpritePacket, tex1)  == 0x80, "tex1");
_Static_assert(__builtin_offsetof(UiSpritePacket, alpha) == 0x88, "alpha");
#endif

/* DrawMotionTrailRibbon (0x2823B8): emits a ribbon of billboard quads along a
 * ring of recorded positions/orientations (f12 width; verts a0, count a1,
 * positions a2, quats a3, curIdx t0, frames t1, colourStart t2, colourEnd t3).
 * Returns early with fewer than 2 frames; otherwise walks the ring backwards
 * (idx = (idx + frames - 1) % frames), rebuilds the model matrix per frame,
 * lerps the vertex colour from colourStart to colourEnd, and emits each quad
 * through ProjectAndClipBillboardQuad (0x281540) with the flag set. Callers:
 * DrawElectricArcTrailRibbon 0x31E138 and 0x326920 (slide-enemy arc trail).
 * Identity from NOTE #5681 (plate review against the ROM); an earlier comment
 * here called it a "UI-sprite batch builder", which was wrong.
 *
 * WHOLE as of the 2026-09-15 re-split (task #314): this unit's boundary was
 * 0x80 too high, so the entry prologue `addiu $sp,$sp,-0xA00` at 0x2823B8 sat
 * in the PRECEDING asm unit and this unit began mid-routine at 0x282438, on
 * the `lw $4,0x958($29)` that reloads a slot stored at 0x282410. Three live
 * dependencies crossed that seam ($f1 written 0x282434 / consumed by
 * `div.s $f22,$f1,$f0` at 0x282444, the 0x958(sp) slot, and
 * `bnez $v0,.L00282750` at 0x282414 whose target is now interior to this
 * function); it is now a whole function rather than a fragment.
 *
 * Still INCLUDE_ASM because no byte-exact C has been written, NOT because it
 * is hardware code. The ROM body has 0 COP2/VU0-macro and 0 MMI ops (its three
 * lq/sq are Vec4 struct copies), and its callee saves are 8 bytes apart, the
 * SN 1.36 -fopt-stack prologue of the s136os arm: ordinary compiled C
 * (FACT #9771; census control: the handwritten 1812A8.s blob scores 379).
 * Only the NATIVE port needs the platform render backend, through
 * ProjectAndClipBillboardQuad; that work is parked (RULING #9559) and says
 * nothing about the EE byte match. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_002823B8);

/* MIS-SPLIT fragment: a bare `addiu $29,$29,0x100; nop` stack-restore tail that
 * bled past the boundary of the preceding function — not a real function entry.
 * Leave as INCLUDE_ASM (documented mis-split). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282790);

/**
 * func_00282798 (InitBillboardSpriteState) — fill a billboard sprite context's
 * GS register block (see UiSpritePacket).
 * @param p      sprite context; only its 0x70..0x8F block is written
 * @param texId  UI texture id, forwarded to GetUiTextureTex0 for TEX0_1
 *               (FACT #6026: an id, not a count)
 * @param blend  row of the 5-int blend table D_239A90; its first four ints are
 *               the ALPHA_1 A/B/C/D selectors (2 bits each)
 * @param fix    ALPHA_1 FIX, shifted into bits 32..39 (callers pass 0x80)
 * CLAMP_1 is cleared and TEX1_1 gets the fixed 0xFF9000000260.
 * Compiled on the s136os arm (SN 1.36 -fopt-stack): the ROM's four
 * 8-byte-packed saves in a 0x20 frame are that compiler's, which 2.9 cannot
 * emit. The table row is formed AFTER the GetUiTextureTex0 call, so `blend`
 * stays in $16 across the call and the `mult` follows it, as in the ROM. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00282798)
S136OS_SLOT(func_00282798);
#else
void func_00282798(UiSpritePacket *p, s32 texId, s32 blend, u64 fix) {
    s32 *row;
    p->tex0  = GetUiTextureTex0(texId);
    row = &D_239A90[blend * 5];
    p->tex1  = ((u64)0xFF90 << 32) | 0x260;
    p->clamp = 0;
    p->alpha = (u64)row[0]
             | ((u64)row[1] << 2)
             | ((u64)row[2] << 4)
             | ((u64)row[3] << 6)
             | (fix << 32);
}
#endif

/* func_00282838: 0x30 bytes of dead debris (six bare `addiu $sp` / nop pairs
 * trailing the previous routine) carved off the real function func_00282868 in
 * task #472. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282838);

#ifdef TARGET_NATIVE
/* gcc -m32 cannot emulate mode(TI); the native arm never uses it as a value. */
typedef struct { unsigned long long _q[2]; } __attribute__((aligned(16))) u_long128;
#else
typedef unsigned long u_long128 __attribute__((mode(TI)));
#endif

extern void func_00282798(UiSpritePacket *p, s32 texId, s32 blend, u64 fix);
extern void Vec4SubVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern void Vec3RescaleToLenVu0(Vec4 *dst, f32 len, const Vec4 *src);
extern void Vec3CrossVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern void Vec4ScaleVu0(Vec4 *dst, f32 s, const Vec4 *src);
extern void Vec4AddVu0(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void func_00283DA0(Vec4 *rotOut, const Vec4 *angles);
extern void func_002840E8(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void func_00283A70(Vec4 *dst, Vec4 *v, Vec4 *mtx);
extern void func_00281540(UiSpritePacket *p, Vec4 *model, s32 flag);
extern Vec4 g_cameraPos;
extern Vec4 g_heroFacingDir;
extern Vec4 D_1A9FD0[4];    /* unit billboard quad corners */

/**
 * func_00282868 (DrawBillboardSprite) — draw one camera-facing billboard quad.
 * @param size        corner scale (the unit quad D_1A9FD0 is scaled by it)
 * @param towardCam   distance the quad is pulled toward the camera along the
 *                    view axis before it is drawn
 * @param roll        roll angle; 0 skips the roll
 * @param pos         world position (copied; the caller's vector is not written)
 * @param texId       UI texture id for func_00282798 (FACT #6026)
 * @param colour      RGBA written to all four corners
 * @param blend       blend-table row for func_00282798 (FIX 0x80)
 * Builds a basis whose first row points from the quad to g_cameraPos, whose
 * second is that row crossed with g_heroFacingDir and whose third completes
 * it, with the (moved) position as the fourth row; when roll != 0 composes
 * the basis in place (func_002840E8) with the rotation func_00283DA0 builds
 * from (roll, 0, 0, 0); transforms the four scaled unit corners through it
 * and emits the quad with ProjectAndClipBillboardQuad (func_00281540, flag 0).
 * Corner STs are (0,0) (0,1) (1,0) (1,1). Callers include
 * DrawLockOnArcFxSlots (FACT #6026) and func_002EE310 (FACT #7153).
 *
 * Compiled on the s136os arm (SN 1.36 -fopt-stack: the $16..$20/$31 saves are
 * 8 bytes apart, FACT #8810). `origin` must stay the local right after
 * `basis`: func_00283A70 reads it as the matrix's fourth row (sp+0xC0).
 * Four empty asm fences, each a SCHEDULING/ALLOCATION DEVICE that emits
 * nothing (RULING #8483). Task #1850 removed each ALONE and counted differing
 * words of the 120 (solo s136 compile, assembled, relocations masked):
 *  - `pos` tied after the stores (4/120): `pos` becomes multi-set, so alias
 *    analysis cannot prove the `lq` from it independent of the stack stores
 *    and it stays below them, as in the ROM;
 *  - `o` tied after that (11/120): hides `o = &origin` from cse, so the copy
 *    stores through $19 (`sq $2,0($19)`) instead of folding to `192($sp)`,
 *    and `o`'s set issues ahead of the colour stores;
 *  - `pos` tied again after the copy (4/120): keeps `sq` directly under `lq`;
 *  - `rollIn`, a copy of `off` behind a NON-volatile fence (2/120; the
 *    volatile form also 4/120): the copy dies at func_00283DA0's argument, so
 *    sched1 loads $5 before $4 as the ROM does.
 * Not devices but also load-bearing: `origin` as its own local rather than
 * basis[3] (31/120), `off` separate from `view` (6/120), the chained colour
 * assignment (4/120: it puts the 0x40 store first). NOTE #9786 has the route.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00282868)
S136OS_SLOT(func_00282868);
#else
void func_00282868(f32 size, f32 towardCam, f32 roll, Vec4 *pos, s32 texId,
                   u32 colour, s32 blend) {
    UiSpritePacket quad;
    Vec4 basis[3];
    Vec4 origin;
    Vec4 offset;
    Vec4 rollMtx[3];
    Vec4 *view;
    Vec4 *off;
    Vec4 *mtx;
    Vec4 *o;
    s32 i;

    func_00282798(&quad, texId, blend, 0x80);
    o = &origin;
    quad.colour[0] = quad.colour[1] = quad.colour[2] = quad.colour[3] = colour;
    quad.st[0][0] = 0.0f; quad.st[0][1] = 0.0f;
    quad.st[1][0] = 0.0f; quad.st[1][1] = 1.0f;
    quad.st[2][0] = 1.0f; quad.st[2][1] = 0.0f;
    quad.st[3][0] = 1.0f; quad.st[3][1] = 1.0f;
    __asm__ __volatile__("" : "+r"(pos));
    __asm__ __volatile__("" : "+r"(o));
    *o = *pos;
    __asm__ __volatile__("" : "+r"(pos));
    view = &basis[0];
    Vec4SubVu0(view, &g_cameraPos, o);
    Vec3RescaleToLenVu0(view, 1.0f, view);
    Vec3CrossVu0(&basis[1], view, &g_heroFacingDir);
    Vec3RescaleToLenVu0(&basis[1], 1.0f, &basis[1]);
    Vec3CrossVu0(&basis[2], &basis[1], view);
    mtx = view;
    off = &offset;
    if (roll != 0.0f) {
        Vec4 *rollIn = off;
        __asm__("" : "+r"(rollIn));
#ifndef TARGET_NATIVE
        *(u_long128 *)off = 0;
#else
        off->y = off->z = off->w = 0.0f;
#endif
        off->x = roll;
        func_00283DA0(rollMtx, rollIn);
        func_002840E8(mtx, mtx, rollMtx);
    }
    Vec4ScaleVu0(off, towardCam, mtx);
    Vec4AddVu0(o, o, off);
    for (i = 0; i < 4; i++) {
        Vec4ScaleVu0(&quad.corner[i], size, &D_1A9FD0[i]);
        func_00283A70(&quad.corner[i], &quad.corner[i], mtx);
    }
    func_00281540(&quad, 0, 0);
}
#endif

/* func_00282A48: 8 bytes of dead pad (addiu $29,$29,0xD0; nop) carved off the
 * real one-call routine func_00282A50 (jal AppendGsRegPacket with mode
 * 0x513F1 / kind 0x47) in task #472. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282A48);

extern void AppendGsRegPacket(s32 regId, u64 value);

/** func_00282A50 — set the GS TEST_1 register (reg id 0x47) to 0x513F1:
 *  ATE=1, ATST=NEVER, AREF=0x3F, AFAIL=FB_ONLY, ZTE=1, ZTST=GEQUAL — every
 *  pixel fails the alpha test and keeps only its colour write, so the draws
 *  that follow leave the Z buffer untouched. No params, no return.
 *  Sole caller: func_0032A2A8 (0x32A2D4), first thing after its prologue.
 *  The empty asm after the call stops cc1 sibcalling it (`j
 *  AppendGsRegPacket`); the ROM keeps the call + return frame (RULING #8483),
 *  as in 178E88's func_0027C020/func_0027C0A8.
 *  Retail leaves one zero pad word at 0x282A74, beyond this function's
 *  0x24-byte extent, before func_00282A78. */
void func_00282A50(void) {
    AppendGsRegPacket(0x47, 0x513F1);
    __asm__ __volatile__("");
}

/* func_00282A78: empty/no-op leaf (original compiles to jr ra; nop). */
void func_00282A78(void) {
}

/* Billboard/quad sprite packet builder: transforms a position against the
 * camera (Vec4SubVu0 / Vec3CrossVu0 / Vec3RescaleToLenVu0 / Vec4ScaleVu0 /
 * Vec4AddVu0 VU0 ops), clamps the projected corner coords to [0,1], and emits a
 * UI sprite primitive via func_00282798 + func_00281540. One lq/sq Vec4 copy;
 * $16..$23, $30, $31 saved 8 bytes apart from 0xE0 plus $f20..$f23 over a
 * 0x150 frame. Left as INCLUDE_ASM because no byte-exact C has been written,
 * NOT because it is hardware code: the body has 0 COP2/VU0-macro and 0 MMI ops
 * and the packed saves are the s136os arm's (FACT #9771) — the VU0 work is in
 * the helper functions it calls. Only the NATIVE port needs the platform
 * render backend (parked, RULING #9559). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282A80);

/* MIS-SPLIT fragment: three bare `addiu $sp` /nop stack-restore tails (0x150,
 * 0x30, 0x20) — alignment/epilogue debris, not a real function entry. Leave as
 * INCLUDE_ASM (documented mis-split). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282E50);

/* DrawGlowSprites: iterates the 16 g_glowSpriteSlots, and for each visible slot
 * computes its camera-space distance/scale, derives screen-space sprite corner
 * coords (FloatToInt/IntToFloat), and writes a full GS sprite DMA packet to
 * g_frameDmaCursor (GIFtag + TEX0 + UV/XYZ registers via sd stores), bracketed
 * by AppendGsRegPacket scissor/alpha setup. Two claims, kept apart:
 * - NATIVE: its semantics write GS packet words through g_frameDmaCursor, so a
 *   native body needs the platform render backend. That is TRUE and parked
 *   (RULING #9559); there is no speculative native body.
 * - EE byte match: does NOT need it. The packet words are written with plain
 *   integer stores; the body has 0 COP2/VU0-macro and 0 MMI ops and saves
 *   8 bytes apart (s136os arm) — ordinary compiled C (FACT #9771). It stays
 *   INCLUDE_ASM only because no byte-exact C has been written. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", DrawGlowSprites);

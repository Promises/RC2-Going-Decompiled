#include "common.h"

extern u64 GetUiTextureTex0(s32 id);
extern s32 D_239A90[];

/* GS register block of the billboard sprite context that func_00282798 fills
 * and ProjectAndClipBillboardQuad (0x281540) / 0x281B04 emit (FACT #5657):
 * 0x70 = CLAMP_1, 0x78 = TEX0_1 (from GetUiTextureTex0), 0x80 = TEX1_1,
 * 0x88 = ALPHA_1 (A/B/C/D blend selectors from the table at D_239A90,
 * stride 0x14, with FIX in bits 32..39). */
typedef struct UiSpritePacket {
    u8  pad0[0x70];
    u64 clamp;  /* 0x70 CLAMP_1, cleared (REPEAT/REPEAT) */
    u64 tex0;   /* 0x78 TEX0_1 from GetUiTextureTex0(texId) */
    u64 tex1;   /* 0x80 TEX1_1, fixed 0xFF9000000260 */
    u64 alpha;  /* 0x88 ALPHA_1 from D_239A90[blend*5] OR (fix<<32) */
} UiSpritePacket;

/* Size pin (byte-neutral). func_00282798 (InitBillboardSpriteState) only writes
 * the GS-register sub-block at 0x70..0x8F, so 0x90 is the minimum bindable size;
 * the real billboard-rasterizer context is larger but its tail is not touched by
 * this entry, so it is left as the leading pad. Confirmed write offsets
 * 0x70/0x78/0x80/0x88 (all u64) from the asm. */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(UiSpritePacket) == 0x90, "UiSpritePacket bindable view 0x90");
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
 * function). The body is GS/VIF packet construction (sq/lq copies, many
 * callee-saves, div/mfhi loop) over the 0xA00-byte frame — tier-3 hardware,
 * so it stays INCLUDE_ASM; it is now at least a whole function rather than a
 * fragment. */
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
 * task #472. func_00282868 is a billboard/sprite packet builder — sq/lq
 * 0x10-byte copies, 5 callee-saved regs incl. $f20-$f23, VU0 vector ops
 * (Vec4SubVu0/Vec3CrossVu0/Vec4ScaleVu0/Vec4AddVu0), and a GS-register append
 * loop. Tier-3 hardware (GS/VIF/VU0); TODO(hle): needs the platform render backend. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282838);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282868);

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
 * UI sprite primitive via func_00282798 + func_00281540. Uses sq/lq 0x10-byte
 * SIMD copies and 9 callee-saved regs incl. $f20-$f23 over a 0x150 frame.
 * Tier-3 hardware (VU0 + GS packet). TODO(hle): needs the platform render
 * backend; left as INCLUDE_ASM to avoid fabricating a non-byte-exact body. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282A80);

/* MIS-SPLIT fragment: three bare `addiu $sp` /nop stack-restore tails (0x150,
 * 0x30, 0x20) — alignment/epilogue debris, not a real function entry. Leave as
 * INCLUDE_ASM (documented mis-split). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282E50);

/* DrawGlowSprites: iterates the 16 g_glowSpriteSlots, and for each visible slot
 * computes its camera-space distance/scale, derives screen-space sprite corner
 * coords (FloatToInt/IntToFloat), and writes a full GS sprite DMA packet to
 * g_frameDmaCursor (GIFtag + TEX0 + UV/XYZ registers via sd stores), bracketed
 * by AppendGsRegPacket scissor/alpha setup. Tier-3 hardware: GS register packet
 * + DMA-cursor advance, sq/lq SIMD copies, 9 callee-saves. TODO(hle): needs the
 * platform render backend; left as INCLUDE_ASM (no byte-exact C, no speculative
 * native body for an opaque GS/DMA packet builder). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", DrawGlowSprites);

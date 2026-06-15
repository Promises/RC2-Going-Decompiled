#include "common.h"

/*
 * text/24D728 - GUI/HUD manager helpers (carve MEGA-BATCH PHASE A TILE D,
 * 2026-06-14; vaddr 0x34D7A8..0x3500FF): per-object setter/getter/forwarder
 * methods over a large HUD/GUI manager instance passed in $a0 (the bolt/HUD
 * counter sub-widgets live at large fixed offsets), plus the cubic-Hermite
 * blend GuiHermiteInterp and the FMV/camera-reset helpers.
 *
 * Built at -O2 -G8 -fno-gcse (later-SN-cc1 gameplay/UI TU model, per the
 * per-unit GFLAG/CC1EXTRA override in tools/ee/objdiff_build.sh / diff.sh).
 */

/* A GUI element (subset, matching text/235FE8): color block at +0xC. */
typedef struct GuiElement {
    /* 0x00 */ f32 *pos;
    /* 0x04 */ f32 *scale;
    /* 0x08 */ s32 unk08;
    /* 0x0C */ s32 *color;   /* -> RGBA color block, color[0] = packed ARGB */
} GuiElement;

extern s32 *GuiElementGetColor(GuiElement *e);
extern void GuiSpriteSetTexture(void *sprite, s32 textureId, s32 frame);
extern void func_00339A88(void *p);
extern void func_00339F98(void *p);
extern void func_00283D10(void *p);
extern void BuildFrameViewMatrices(void);

/* Keep the absolute-addressed camera state on the two-insn %hi/%lo macro under
 * -G8 (it is far outside the gp small-data window). */
__asm__(".extern g_cameraState, 16");
extern u8 g_cameraState[];

/* func_0034D7A8: read the packed color of the sub-element at +0x378 and return
 * only its top (alpha) byte. */
s32 func_0034D7A8(u8 *mgr) {
    s32 *color = GuiElementGetColor((GuiElement *)(mgr + 0x378));
    return color[0] & 0xFF000000;
}

/* func_0034D7D0: select the HUD frame sprite texture by mode (stored at
 * +0x15A4): mode 0 -> texture 0x7567, mode 1 -> 0xEAA2, else no change.
 * NEAR-MISS (89.55%): the C is structurally exact, but the original folds the
 * epilogue `ld $ra` into the case-exit branch delay slots (a cc1 epilogue-
 * scheduling artifact this toolchain won't reproduce). Kept as the portable
 * #else body; the matching build keeps the original bytes. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034D7D0);
#else
void func_0034D7D0(u8 *mgr, s32 mode) {
    *(s32 *)(mgr + 0x15A4) = mode;
    switch (mode) {
    case 0:
        GuiSpriteSetTexture(mgr + 0x2A0, 0x7567, 0);
        break;
    case 1:
        GuiSpriteSetTexture(mgr + 0x2A0, 0xEAA2, 0);
        break;
    }
    __asm__ __volatile__("");
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034D828);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034D8C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034DAB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034DB68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034DBC8);

/* func_0034DBD8: store the texture/frame handle into the manager at +0x2DC. */
void func_0034DBD8(u8 *mgr, s32 value) {
    *(s32 *)(mgr + 0x2DC) = value;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034DBE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034E8D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034ED20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", GuiManagerInitListRows);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", GuiManagerInitHudLists);

/* func_0034EF60: empty/no-op leaf (original compiles to jr ra; nop). */
void func_0034EF60(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034EF68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F028);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F1C0);

/* func_0034F200: forward the manager's HUD list (+0x1D8C) to func_00339A88.
 * The empty asm guard blocks cc1's void sibling-call (keeps jal + frame). */
void func_0034F200(u8 *mgr) {
    func_00339A88(mgr + 0x1D8C);
    __asm__ __volatile__("");
}

/* func_0034F220: forward the manager's HUD list (+0x1D8C) to func_00339F98. */
void func_0034F220(u8 *mgr) {
    func_00339F98(mgr + 0x1D8C);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F240);

/* func_0034F300: identity - return the manager pointer unchanged. */
void *func_0034F300(void *mgr) {
    return mgr;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", GuiSystemInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F868);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F928);

/* func_0034F9B8: reset the camera-state HUD sub-block (clear the three ints at
 * +0x140/+0x144/+0x148), run func_00283D10 on its +0x370 sub-object, then
 * rebuild the frame view matrices.
 * NEAR-MISS (85.00%): the C reproduces every instruction except the position of
 * the `sd $ra` prologue save, which the original schedules between the `lui` and
 * the `%lo` addiu of the global address (a cc1 prologue-scheduling artifact).
 * Kept as the portable #else; the matching build keeps the original bytes. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F9B8);
#else
void func_0034F9B8(void) {
    u8 *cam = g_cameraState;
    void *sub = cam + 0x370;
    *(s32 *)(cam + 0x140) = 0;
    *(s32 *)(cam + 0x144) = 0;
    *(s32 *)(cam + 0x148) = 0;
    func_00283D10(sub);
    BuildFrameViewMatrices();
    __asm__ __volatile__("");
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F9F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F9F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034FAF8);

/* GuiHermiteInterp: cubic-Hermite blend of the four controls a,b,c,d by t
 * (clamped to [0,1] with an assert) - mathUtil.cpp line 36. Returns
 *   a*(2t^3-3t^2+1) + b*(t^3-2t^2+t) + c*(t^3-t^2) + d*(3t^2-2t^3).
 * The matching build keeps the original (FPU instruction scheduling differs);
 * the #else is the portable equivalent. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", GuiHermiteInterp);
#else
extern void AssertFail(const char *file, s32 line, const char *expr);
extern const char D_1AE758[];
extern const char D_1AE770[];
f32 GuiHermiteInterp(f32 t, f32 a, f32 b, f32 c, f32 d) {
    f32 t2, t3;
    if (!(t >= 0.0f && t <= 1.0f)) {
        AssertFail(D_1AE758, 0x24, D_1AE770);
    }
    t2 = t * t;
    t3 = t2 * t;
    return a * (2.0f * t3 - 3.0f * t2 + 1.0f)
         + b * (t3 - 2.0f * t2 + t)
         + c * (t3 - t2)
         + d * (3.0f * t2 - 2.0f * t3);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034FCA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034FD90);

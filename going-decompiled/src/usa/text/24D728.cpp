#include "common.h"

/* R5900_SHORT_LOOP_PAD1(v, next): one `nop` emitted under noreorder, tied to the
 * loop's live value by its operands. A SCHEDULING DEVICE (RULING #8435; the pad
 * is the R5900 short-loop pad, FACT #7918 / #7937 / #8434): the ROM pads short
 * loops with nops before the backward branch, which neither cc1 nor our
 * assembler inserts. EE arm only; on native it is nothing. */
#ifndef TARGET_NATIVE
#define R5900_SHORT_LOOP_PAD1(v, next) \
    __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+r"(v) : "r"(next))
#else
#define R5900_SHORT_LOOP_PAD1(v, next) ((void)0)
#endif

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

/*
 * GuiHudManager (size 0x15B0) - the HUD/bolt-counter manager sub-object the
 * constructor at 0x34D490 builds and the per-frame updater at 0x34F068 drives.
 * It is embedded in the outer GuiInstance at offset +0x7A0 (see
 * GuiManagerInitHudLists: `$17+0x7A0` is handed to this object's helpers, and
 * the next outer field is the list at +0x1D50 -> the manager spans
 * [0x7A0, 0x1D50) of the outer = 0x1D50-0x7A0 = 0x15B0 bytes). The constructor's
 * highest store is at +0x15A8 (4-byte) -> 0x15AC, padded to 0x15B0; CONFIRMED.
 *
 * Opaque sized blob: bodies access it via raw `(char*)mgr+off` casts, so only
 * the size is pinned (byte-neutral). Fields confirmed via the constructor /
 * updater / the leaf accessors below:
 *   +0x2A0  GuiSprite sub-object (frame sprite; GuiSpriteSetTexture target)  CONFIRMED
 *   +0x2DC  texture/frame handle word                                        CONFIRMED
 *   +0x378  GuiElement (bolt/HUD counter element; GuiElementGetColor target) CONFIRMED
 *   +0x15A4 frame-sprite mode selector (0/1)                                 CONFIRMED
 *   (other offsets are sub-element arrays + flags, left as padding)
 */
typedef struct GuiHudManager { u8 _bytes[0x15B0]; } GuiHudManager;
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(GuiHudManager) == 0x15B0, "GuiHudManager spans outer[0x7A0..0x1D50)");
#endif

/*
 * GuiInstance (size 0x2238) - the outer GUI manager object (loaded via the
 * global g_guiInstance). It embeds the GuiHudManager at +0x7A0 and owns three
 * GuiList objects at +0x1D50, +0x1D8C and +0x1FDC (each ~0x250 bytes, init'd by
 * GuiManagerInitHudLists / GuiManagerInitListRows). Its two cached font glyphs
 * sit at +0x222C/+0x2230, so the object reaches 0x2234; it occupies the
 * [0x36F28, 0x39160) slot (0x2238 bytes) of the g_guiInstance singleton.
 * Size 0x2238 CONFIRMED via the sibling-object boundary in GuiSystemInit
 * (0x34F664): it hands the HUD-region base at instance+0x36F28 and the NEXT
 * sibling object (GuiScreenWithPlanetNameInit) sits at instance+0x39160, so this
 * region spans 0x39160-0x36F28 = 0x2238.
 *
 * Opaque sized blob (raw `(char*)mgr+off` access). Fields used by the forwarders
 * below:
 *   +0x1D8C  GuiList (HUD list head; func_00339A88 / func_00339F98 target)  CONFIRMED
 */
typedef struct GuiInstance { u8 _bytes[0x2238]; } GuiInstance;
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(GuiInstance) == 0x2238, "GuiInstance occupies game-state[0x36F28..0x39160)");
#endif

/* ---------------------------------------------------------------------------
 * text/24D728 - Phase 4 promotion sweep, task #566 (round 4), 2026-09-21.
 * Base: origin/master e3f50d43. Instrument for every % below: the unit objdiff
 * report (tools/ee/unit_report.sh) over tools/ee/objdiff_build.sh, clean tree.
 *
 * RESULT: 18 #else arms measured on BOTH arms; 0 reached 100.00%; all 18 stay
 * #else. Unit stays 7/31 and the NAME list is identical to the base (no
 * untouched row moved, #7369). Per-arm % and residual class are in the
 * TODO(match) block above each arm.
 *
 * THREE UNIT-WIDE FINDINGS
 * 1. ARM ELIGIBILITY IS THE SAVE STRIDE, NOT THE %. Every function in this unit
 *    saves its callee GPRs at 8-byte stride (the 2.96 engine layout). cc1 2.9
 *    emits 16-byte stride as soon as the function saves >= 1 callee GPR, so its
 *    frame is larger and it can never match. Measured: the 3 arms whose ROM
 *    frame the sdk29 arm reproduces (func_0034D7D0, func_0034F9B8,
 *    GuiHermiteInterp) are exactly the 3 with nGPR == 0 in the frozen .s -
 *    3/3 and 15/15, no exceptions. The fuzzy % ranks sdk29 above engine96 on
 *    14 of 18 arms and is MISLEADING for all 15 of those: % is not eligibility.
 *    This CORROBORATES FACT #7422 (text/16E980) on a second unit rather than
 *    narrowing it: #7422's ">= 2 callee saves" counts ra, so ">= 2 saves
 *    including ra" and "nGPR >= 1 excluding ra" are the same predicate, and it
 *    called all 18 arms here correctly.
 * 2. THE UNIT SPANS >= 2 ORIGINAL TUs WITH DIFFERENT SCHEDULING. Adding
 *    -fno-schedule-insns2 to the engine arm takes func_0034DB68 from 91.67% to
 *    100.00% and simultaneously takes func_0034F1C0 - already byte-exact and
 *    landed - from 100.00% to 87.50%. Both are in this carve unit, both on the
 *    engine arm. No single per-unit flag matches both, so 24D728 cannot be a
 *    single original translation unit. (Flag NOT landed: a per-unit flag change
 *    is a landing-gate question, and this one regresses a landed match.)
 * 3. THE DOMINANT RESIDUAL IS PROLOGUE STORE ORDER. Under both scheduler
 *    settings the difference is where `sd ra` / the incoming-argument copy sit
 *    among the register saves; neither setting reproduces the ROM across the
 *    unit.
 *
 * SOURCE FIXES KEPT (they make these arms faithful; none of them promoted):
 *  - Sibling-call guards on the 8 arms whose tail call cc1 2.96 turns into a
 *    `j <callee>` the ROM never has (#7343): func_0034D828, func_0034DB68,
 *    func_0034D8C8, func_0034DAB0, GuiManagerInitHudLists, func_0034F240,
 *    func_0034F928, func_0034F9F8. func_0034D828 83.50 -> 95.00 and
 *    func_0034DB68 76.46 -> 91.67 (engine96) on the guard alone.
 *  - Six float literals respelled to the ROM's exact bits. cc1 2.96 converts a
 *    decimal 1 ULP low (#7342), so the plain spelling is wrong in the bytes:
 *    func_0034F028 pi x2 (0x40490FDB), func_0034F928 0.4/0.8/1.2
 *    (0x3ECCCCCD/0x3F4CCCCD/0x3F99999A), func_0034F240 0.62 (0x3F1EB852). The
 *    replacement decimals were read off a direct cc1 2.96 probe
 *    (tools/ee/.t566/probe/ulp.c), not guessed - a +1 ULP decimal overshoots as
 *    often as it lands. Both pi uses must be spelled identically or cc1 stops
 *    CSE-ing them and materialises pi twice. pi is 3.1415927411f, which is
 *    0x40490FDB on cc1 2.96, cc1 2.9 and native clang alike (measured, task
 *    #1243); the earlier 3.1415929f gave FDB only on 2.96 and was 1 ULP high
 *    (0x40490FDC) on cc1 2.9 and on the native #else (FACT #8758).
 *  - GuiHermiteInterp computes a*(2t^3-3t^2) + a in the ROM's operand order;
 *    the folded "+ 1.0f" coefficient was a different expression. The FP
 *    register colouring left after that (85.59%) was closed by task #894 -
 *    see the function's own comment.
 *
 * LATER (task #1669): func_0034F240, func_0034F928 and func_0034F9B8 closed on
 * the s136os arm (SN 1.36, which converts the plain decimals exactly). Their
 * sibling-call guards, float respellings and the F928 union are gone from
 * those bodies; each body's own comment says what closed it.
 * --------------------------------------------------------------------------- */

extern s32 *GuiElementGetColor(GuiElement *e);
extern void GuiSpriteSetTexture(void *sprite, s32 textureId, s32 frame);
extern void func_00339A88(void *p);
extern void func_00339F98(void *p);
extern void func_00283D10(void *p);
extern void BuildFrameViewMatrices(void);
extern void func_0034DBE0(void *hudMgr);
extern void TickBoltCounterHud(void);
void func_0034F200(GuiInstance *mgr);  /* defined below; called by func_0034F1C0 */

/* Moby per-frame sub-update helpers used by func_0034F9F8:
 *   func_0026F790(mobySub, 0)        - reset/prep pass on the moby's +0xC00 block
 *   func_002A1F68(framePtr, frame0Ptr, moby, classFlag) - install an animation
 *     frame's geometry pointers (computed from the class seq/frame tables). */
extern void func_0026F790(void *mobySub, s32 zero);
extern void func_002A1F68(void *framePtr, void *frame0Ptr, void *moby, s32 classFlag);
extern void UpdateMobyAnimation(void *mobySub);
extern void UpdateMobyBSphereAndGrid(void *mobySub);

/* List/sprite sub-widget reset helpers (text/248B50, addr 0x34A7F8/0x34A370):
 * clear an animation slot of a sub-list / sprite by flag. Forwarders only -
 * their semantics are not under test here. func_0034A3B8 stores a float into the
 * widget's +0x24 field. */
extern void func_0034A7F8(void *list, s32 flag);
extern void func_0034A370(void *sprite, s32 mode);
extern void func_0034A3B8(void *widget, f32 v);
extern void GuiElementSetVisible(GuiElement *e, s32 show);

/* GuiElement list-row float-field + sub-element setters (text/235FE8):
 *   func_00336C18 returns the element's primary vec pointer (`*(void**)(e+0)`);
 *   func_003372D0 / func_00337310 pack two bytes into the element's two color
 *     blocks (top byte of words at +0x0/+0x4 resp. +0x8/+0xC);
 *   func_00337B88 stores a word at e+0x44. */
extern f32 *func_00336C18(GuiElement *e);
extern void func_003372D0(GuiElement *e, s32 b0, s32 b1);
extern void func_00337310(GuiElement *e, s32 b0, s32 b1);
extern void func_00337B88(GuiElement *e, s32 value);

/* Keep the absolute-addressed camera state on the two-insn %hi/%lo macro under
 * -G8 (it is far outside the gp small-data window). */
__asm__(".extern g_cameraState, 16");
extern u8 g_cameraState[];

/* func_0034D7A8: read the packed color of the sub-element at +0x378 and return
 * only its top (alpha) byte. */
s32 func_0034D7A8(GuiHudManager *mgr) {
    s32 *color = GuiElementGetColor((GuiElement *)((u8 *)mgr + 0x378));
    return color[0] & 0xFF000000;
}

/*
 * func_0034D7D0 — select the HUD frame sprite's texture by mode.
 *
 *   mgr   the HUD manager; the mode is stored at +0x15A4
 *   mode  0 -> texture 0x7567, 1 -> texture 0xEAA2, anything else stores the
 *         mode and leaves the frame sprite (+0x2A0) unchanged
 * No return value.
 *
 * Non-obvious (task #763; replaces a "NEAR-MISS 89.55%" note):
 *  - Both GuiSpriteSetTexture calls are real `jal`s in the ROM, not sibling
 *    calls, so something must stop cc1 turning them into `j`. The dead store
 *    `noTailCall = 0` does that at expand time and flow deletes it before
 *    either scheduler runs (#756's lever). The earlier empty volatile asm also
 *    blocked the sibcall, but it stayed in the block as a scheduling barrier
 *    and kept `ld ra` out of the two case-exit branch delay slots, where the
 *    ROM has it.
 */
void func_0034D7D0(GuiHudManager *mgr, s32 mode) {
    s32 noTailCall;
    *(s32 *)((u8 *)mgr + 0x15A4) = mode;
    switch (mode) {
    case 0:
        GuiSpriteSetTexture((u8 *)mgr + 0x2A0, 0x7567, 0);
        break;
    case 1:
        GuiSpriteSetTexture((u8 *)mgr + 0x2A0, 0xEAA2, 0);
        break;
    }
    noTailCall = 0;
}

/* func_0034D828: (re)assign the HUD frame-sprite texture and, on the first
 * activation after the manager's row state was torn down, reset the five
 * scrollable sub-list/sprite slots. Stores the new "active" flag (arg2) at
 * +0x1584; if the +0x1588 "needs-reset" latch is set, it clears the latch,
 * resets the four list slots (+0xF3C/+0xFD0/+0x1064/+0x1180 via func_0034A7F8)
 * and the sprite slot (+0x10F8 via func_0034A370 mode 1), then retargets the
 * frame sprite (+0x158) to texture `tex`. Independently, if the +0x159C latch is
 * set, it retargets the frame sprite too. `tex` is the GuiSprite texture id.
 * MATCHED on the s136os arm: the original packs its three callee-saved GPRs
 * (s0/s1/ra) at an 8-byte stride (frame 0x20), which the pinned cc1 2.9 cannot
 * emit (16-byte slots, frame 0x30) and SN 2.95.3 v1.36 -fopt-stack does
 * (FACT #8810). The trailing fence defeats the tail-call.
 * GUARD (task #1278): on EE this C is the image's body, compiled alone by the
 * s136os arm (tools/ee/s136os_functions.txt) and spliced over the S136OS_SLOT
 * line by tools/ee/s136os_splice.sh; the 2.9 compile sees only the slot, and
 * there is no asm fallback. On native it is plain C, as before. */
/* History - task #566 (round 4), measured on the then-committed tree (both arms
 * promoted whole-unit; instrument: tools/ee/unit_report.sh over
 * tools/ee/objdiff_build.sh, clean): sdk29 94.33%, engine96 95.00%.
 * Residual then: PRO-ORDER (one insn: `daddu s0,a0` emitted before `sd s1` instead of after) */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034D828)
S136OS_SLOT(func_0034D828);
#else
void func_0034D828(GuiHudManager *mgr, s32 tex, s32 activeFlag) {
    u8 *m = (u8 *)mgr;
    *(s32 *)(m + 0x1584) = activeFlag;
    if (*(s32 *)(m + 0x1588) != 0) {
        *(s32 *)(m + 0x1588) = 0;
        func_0034A7F8(m + 0xF3C, 0);
        func_0034A7F8(m + 0xFD0, 0);
        func_0034A7F8(m + 0x1064, 0);
        func_0034A370(m + 0x10F8, 1);
        func_0034A7F8(m + 0x1180, 0);
        GuiSpriteSetTexture(m + 0x158, tex, 0);
    }
    if (*(s32 *)(m + 0x159C) != 0) {
        GuiSpriteSetTexture(m + 0x158, tex, 0);
    }
    /* empty __asm__ __volatile__("") fence: a SCHEDULING DEVICE, emits nothing
     * (RULING #8483 rev 2); placed after the last call to block a sibcall
     * (FACT #8064's sibcall-guard class). */
    __asm__ __volatile__("");
}
#endif

/* func_0034D8C8: arm the manager's six animated HUD-frame sub-widgets. Store the
 * new active-state flag at +0x1594; then, only when the +0x1598 "needs-rearm"
 * latch is set, clear it, hide the bolt/HUD counter element (+0x5D8) and, for
 * each of the six 0x88-byte sub-widgets at +0xC0C/+0xC94/+0xD1C/+0xDA4/+0xE2C/
 * +0xEB4: first re-arm its animation slot (func_0034A370(., 1)) and then seed its
 * +0x24 per-frame step (func_0034A3B8) with a video-rate-scaled increment - PAL
 * (g_bPalMode != 0) uses 0.140 (0x3E0F5C29), NTSC uses ~0.1166 (0x3DEEEEF0), the
 * 50Hz/60Hz frame-period ratio so the on-screen animation runs at wall-clock
 * speed in both regions.
 *
 * The original re-reads g_bPalMode and re-selects the literal before each of
 * the six func_0034A3B8 calls, and so does this body. `flag` is the new
 * active-state value. Returns nothing. */
/* ADDRESSING-MODEL DEVICE (RULING #8620, the 191238.cpp equate form):
 * func_0034D8C8 reads g_bPalMode absolutely (lui/lw) six times. As a plain
 * `extern s32` cc1 writes `.extern g_bPalMode, 4` and gas makes each read one
 * %gp_rel word. Size 16 pins them absolute; the equated name keeps the size
 * off the real symbol and the relocations still name it. Top level, so the
 * s136os TU and the spliced 2.9 TU see the same lines. */
#ifndef TARGET_NATIVE
__asm__(".extern g_bPalModeAbs, 16\n\tg_bPalModeAbs = g_bPalMode");
extern s32 g_bPalModeAbs;
#else
#define g_bPalModeAbs g_bPalMode
#endif

#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034D8C8)
S136OS_SLOT(func_0034D8C8);
#else
extern s32 g_bPalMode; /* PAL video-mode flag (USA build clears it -> NTSC) */
/* MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (task #1669) written the plain way: the PAL test and its literal
   per call, so cc1 reloads g_bPalMode before each func_0034A3B8 exactly as
   the ROM does, `!= 0 ? PAL : NTSC` for the ROM's NTSC-default beqz, and no
   union or trailing fence. SN 1.36 converts the 9-digit decimals exactly.
   GUARD: on EE this C is the image's body, compiled alone by the s136os arm
   (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
   tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
   splice drops the function. On native it is plain C. */
void func_0034D8C8(GuiHudManager *mgr, s32 flag) {
    u8 *m = (u8 *)mgr;
    *(s32 *)(m + 0x1594) = flag;
    if (*(s32 *)(m + 0x1598) != 0) {
        *(s32 *)(m + 0x1598) = 0;
        GuiElementSetVisible((GuiElement *)(m + 0x5D8), 0);
        func_0034A370(m + 0xC0C, 1);
        func_0034A370(m + 0xC94, 1);
        func_0034A370(m + 0xD1C, 1);
        func_0034A370(m + 0xDA4, 1);
        func_0034A370(m + 0xE2C, 1);
        func_0034A370(m + 0xEB4, 1);
        func_0034A3B8(m + 0xC0C, g_bPalModeAbs != 0 ? 0.140000001f : 0.116666675f);
        func_0034A3B8(m + 0xC94, g_bPalModeAbs != 0 ? 0.140000001f : 0.116666675f);
        func_0034A3B8(m + 0xD1C, g_bPalModeAbs != 0 ? 0.140000001f : 0.116666675f);
        func_0034A3B8(m + 0xDA4, g_bPalModeAbs != 0 ? 0.140000001f : 0.116666675f);
        func_0034A3B8(m + 0xE2C, g_bPalModeAbs != 0 ? 0.140000001f : 0.116666675f);
        func_0034A3B8(m + 0xEB4, g_bPalModeAbs != 0 ? 0.140000001f : 0.116666675f);
    }
}
#endif

/* func_0034DAB0: drives the HUD sub-element at +0x5D8 - pad-gated "snap" tween,
 * a color-ramp tween, then writes the tween handle/alpha and sets visibility.
 * Left as bare INCLUDE_ASM: a faithful cmp-oracle is blocked because its tween
 * callee func_002AA3F0 (1A8180.c, linked whole into the cmp suite) runs a real
 * VU0 LerpByteVec4PackedVu0 + absolute g_colorPulsePhaseA/B globals (the absolute-symbol
 * ld --gc-sections wall), and it cannot be mocked instead without colliding with
 * that linked real body. Not seedable cleanly; revisit for byte-oracle when
 * func_002AA3F0 itself is oracled. */
#ifdef TARGET_NATIVE
extern s32 g_padButtonsPressed;
extern u32 func_002AA3F0(u32 color1, u32 color2, s32 period, s32 counterSel, s32 reset);
extern f32 *func_00337120(GuiElement *e);   /* -> element primary vec (float[0]) */
#endif

/* func_0034DAB0 - HISTORY: task #566 (round 4) measured on the COMMITTED tree (this file,
 * both arms promoted whole-unit; instrument: tools/ee/unit_report.sh over
 * tools/ee/objdiff_build.sh, clean): sdk29 82.07%, engine96 58.98%. Eligible arm: e96.
 * Residual: ORDER + ADDRESSING on those two arms.
 * MATCHED on the s136os arm (task #1387; screened by task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
 * masked): EXACT 45/45, relocations equal, once
 * the trailing empty fence is removed (a cc1-2.96 sibcall fence is a scheduling
 * barrier under SN 1.36, as #1347 measured) and the pad word is read in the
 * ROM's split form (declaration below). Fence kept: 5/45, first diff @24 (the
 * `move $17,$2` / `ori $5` order); split form undone: the pad word read
 * gp-relative, first diff @0 (ROM `lui $3` first). */
/* GUARD (task #1387): on EE the #else body below is the image's func_0034DAB0, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034DAB0)
S136OS_SLOT(func_0034DAB0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern u32 func_002AA3F0(u32 color1, u32 color2, s32 period, s32 counterSel, s32 reset);
extern f32 *func_00337120(GuiElement *e);
#ifndef TARGET_NATIVE
/* ADDRESSING-MODEL DEVICE (RULING #8620): `.data` on this extern declaration
 * takes the pad word out of cc1's -G8 small-data class, so cc1 splits it into
 * the ROM's lui (scheduled first) / lw %lo pair. Moves no data. */
extern s32 g_padButtonsPressed __attribute__((section(".data")));
#else
extern s32 g_padButtonsPressed;
#endif
/* (end of this body's declarations) */
/* Structure-exact model (cmp-oracle blocked as noted; matching arm stays asm).
   Drives the HUD sub-element at +0x5D8: any d-pad direction restarts the tween
   counter, then a color-ramp tween handle is written into the element's color
   block, `alpha` into its primary vector, and visibility is set from `visible`. */
void func_0034DAB0(GuiHudManager *mgr, s32 visible, f32 alpha) {
    GuiElement *elem;
    s32 *color;
    if (g_padButtonsPressed & 0xF000) {
        func_002AA3F0(0, 0, 1, 0, 1);
    }
    elem = (GuiElement *)((u8 *)mgr + 0x5D8);
    color = GuiElementGetColor(elem);
    color[0] = func_002AA3F0(0x60442D00, 0x70FFFEED, 0x14, 0, 0);
    *func_00337120(elem) = alpha;
    GuiElementSetVisible(elem, visible);
}
#endif

/* func_0034DB68: sibling of func_0034D828 for the manager's OTHER scrollable
 * region - store the "active" flag (arg) at +0x158C, and if the +0x1590
 * needs-reset latch is set, clear it and reset that region's four sub-list slots
 * (+0x1214/+0x12A8/+0x133C/+0x13D0 via func_0034A7F8, flag 0). `flag` is the new
 * active state.
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810): it emits
 * the ROM's packed save frame (ra at +0x8).
 * GUARD (task #1278): on EE this C is the image's body, compiled alone by the
 * s136os arm (tools/ee/s136os_functions.txt) and spliced over the S136OS_SLOT
 * line by tools/ee/s136os_splice.sh; the 2.9 compile sees only the slot, and
 * there is no asm fallback. On native it is plain C, as before. */
/* History - task #566 (round 4), measured on the then-committed tree (both arms
 * promoted whole-unit; instrument: tools/ee/unit_report.sh over
 * tools/ee/objdiff_build.sh, clean): sdk29 99.38%, engine96 91.67%.
 * Residual then: sdk29: PACKED-SAVE, frame only (ra@0x10 vs ROM 0x8). e96: SCHED-INSNS2 - reaches 100.00 under -fno-schedule-insns2, which regresses func_0034F1C0 100->87.50 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034DB68)
S136OS_SLOT(func_0034DB68);
#else
void func_0034DB68(GuiHudManager *mgr, s32 flag) {
    u8 *m = (u8 *)mgr;
    *(s32 *)(m + 0x158C) = flag;
    if (*(s32 *)(m + 0x1590) != 0) {
        *(s32 *)(m + 0x1590) = 0;
        func_0034A7F8(m + 0x1214, 0);
        func_0034A7F8(m + 0x12A8, 0);
        func_0034A7F8(m + 0x133C, 0);
        func_0034A7F8(m + 0x13D0, 0);
    }
    /* empty __asm__ __volatile__("") fence: a SCHEDULING DEVICE, emits nothing
     * (RULING #8483 rev 2); placed after the last call to block a sibcall
     * (FACT #8064's sibcall-guard class). */
    __asm__ __volatile__("");
}
#endif

/* func_0034DBC8: mis-split fragment (two mid-fn stores `sw $2,0x158C/0x1584($4)`,
 * no prologue/jr) - #47 resplit-pass backlog, not #else material. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034DBC8);

/* func_0034DBD8: store the texture/frame handle into the manager at +0x2DC. */
void func_0034DBD8(GuiHudManager *mgr, s32 value) {
    *(s32 *)((u8 *)mgr + 0x2DC) = value;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034DBE0);

/* func_0034E8D8(hudMgr): the weapon-select / bolt-counter HUD RENDER pass. Draws
 * the frame + label sprites/text at the sub-element offsets (+0x4C/+0x98/+0x158/
 * +0x100 base group; +0x2E0/+0x32C mirrored pair with negated scale when no notice
 * is pending, D_1A8C64==0), runs the two element-list draw loops (7 rows each,
 * dispatching each element's own draw fn via its vtable at +0x8/+0xC), then renders
 * the live bolt count: counts its decimal digits (g_boltCount / 10 loop), formats
 * it with a language-selected template (g_currentLanguage 3/5/!=4 -> D_1AE6B0 vs
 * D_1AE6B8, via func_00115DA8) and lays out the digit glyphs across the row with a
 * per-digit advance (func_0027FCB0 / GetUiTextureTex0), and finally the ">/<"
 * scroll arrows (D_1A7A3A gate, func_002802E8 x4) and the footer (func_0034BA50).
 *
 * PARK (doc-modeled INCLUDE_ASM, NOT #else - per the blast-radius guardrail): this
 * 1096-byte renderer is (a) NOT cmp-oracle-able (absolute GUI-vtable/text globals +
 * the whole draw callee set - the --gc-sections wall) AND (b) very high
 * transcription-risk: it dispatches through per-element function pointers I'd be
 * guessing the struct layout of (jalr via [+0x8]/[+0xC]), uses the FP<->int
 * cvt.w.s idiom and bc1f/bc1tl likely-branch delay-slot nullification (0x34EB68,
 * 0x34EC0C) for the digit layout, and a break-7 divide loop for the digit count -
 * a one-shot un-verified #else would silently mis-render. Model faithfully once
 * the GUI element vtables + text pipeline make it oracle-able. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034E8D8);

/* func_0034ED20: mis-split fragment (positive `addiu $sp,+0x40; nop` epilogue tail,
 * no prologue/jr) - #47 resplit-pass backlog, not #else material. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034ED20);

/* GuiManagerInitListRows: build the manager's scrollable list-row table - 26
 * contiguous 0x48-byte GuiListRow elements (GuiListRowElementInit, stride 0x48)
 * from the manager base, a trailing row at +0x750, the embedded GuiHudManager
 * sub-object (+0x7A0 via func_0034BDB0), and the three list heads at
 * +0x1D50/+0x1D8C/+0x1FDC (func_003374D8 + func_00338A80 x2); returns mgr.
 * Left bare INCLUDE_ASM: a faithful cmp-oracle is blocked because its callees'
 * real TARGET_NATIVE bodies (GuiListRowElementInit / func_003374D8 / func_00338A80
 * in text/235FE8, linked whole into the cmp suite) transitively install the
 * absolute GUI vtable data globals (g_GuiElementVtable / g_GuiListRowVtable),
 * which are undefined in the cmp link (the absolute-data --gc-sections wall), and
 * those bodies cannot be mocked instead without colliding with the linked real
 * 235FE8 definitions. Revisit for byte-oracle when the vtable globals are seeded. */
#ifdef TARGET_NATIVE
/* Sibling-unit GUI helpers used by the #else bodies below (real defs in
   text/235FE8 etc.); their returns are unused here. */
extern void GuiListRowElementInit(void *row);
extern void *func_0034BDB0(void *hudMgr);
extern void func_003374D8(void *listHead);
extern void func_00338A80(void *listHead);
#endif

/* GuiManagerInitListRows - HISTORY: task #566 (round 4) measured on the COMMITTED tree (this file,
 * both arms promoted whole-unit; instrument: tools/ee/unit_report.sh over
 * tools/ee/objdiff_build.sh, clean): sdk29 85.23%, engine96 76.97%. Eligible arm: e96.
 * Residual: BNEL (ROM `bne`+2 nops, built `bnel`) + PRO-ORDER on those two arms.
 * MATCHED on the s136os arm (task #1387; screened by task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
 * masked): EXACT 35/35, relocations equal, with
 * the loop below: the counter stepped before the call, an EMPTY fence after it
 * (RULING #8483, emits nothing) and the ROM's two R5900 short-loop pad nops
 * (R5900_SHORT_LOOP_PAD1, a scheduling device, RULING #8435). Pads removed: 18/35,
 * first diff @13; fence removed: 3/35 (sched2 hoists the pads above the jal). */
/* GUARD (task #1387): on EE the #else body below is the image's GuiManagerInitListRows, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiManagerInitListRows)
S136OS_SLOT(GuiManagerInitListRows);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void GuiListRowElementInit(void *row);
extern void *func_0034BDB0(void *hudMgr);
extern void func_003374D8(void *listHead);
extern void func_00338A80(void *listHead);
/* (end of this body's declarations) */
/* Structure-exact model (cmp-oracle blocked as noted above; matching arm stays
   asm). Inits the 26 contiguous 0x48-byte list rows from the manager base plus a
   27th at +0x750, then the embedded HUD sub-object (+0x7A0) and the three list
   heads (+0x1D50, +0x1D8C, +0x1FDC). Returns the manager. */
void *GuiManagerInitListRows(void *mgr) {
    u8 *row = (u8 *)mgr;
    s32 i;
    i = 0x19;
    do {
        i--;
        GuiListRowElementInit(row);
        __asm__ __volatile__("");
        R5900_SHORT_LOOP_PAD1(row, row);
        R5900_SHORT_LOOP_PAD1(row, row);
        row += 0x48;
    } while (i != -1);
    GuiListRowElementInit((u8 *)mgr + 0x750);
    func_0034BDB0((u8 *)mgr + 0x7A0);
    func_003374D8((u8 *)mgr + 0x1D50);
    func_00338A80((u8 *)mgr + 0x1D8C);
    func_00338A80((u8 *)mgr + 0x1FDC);
    return mgr;
}
#endif

#ifdef TARGET_NATIVE
/* Sibling-unit GUI list/sprite helpers + the font-atlas source, used only by the
   #else body below (opaque struct pointers left void* to avoid pulling their
   types into this TU). */
extern void *g_guiInstance;              /* live GUI root; +0x8710 = font atlas */
extern u8 D_1AE6C8[];                    /* list-element init template */
extern u8 D_1AE6D8[];                    /* frame-sprite init template */
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void GuiListElementInit(GuiElement *e, s32 v3C, s32 v34, void *tag, void *pool);
extern void GuiListSetVisibleRows(GuiElement *e, s32 rows);
extern void GuiListSetItemCount(GuiElement *e, s32 n);
extern void GuiListSetScrollPos(GuiElement *e, s32 pos);
extern void GuiListSetColorPair0(GuiElement *e, s32 c0, s32 c1);
extern void GuiListSetColorPair1(GuiElement *e, s32 c0, s32 c1);
extern void func_00337B68(GuiElement *e, s32 v);
extern void func_0034C008(void *listHead, void *gui, void *pool);
extern void GuiSpriteElementInit(GuiElement *e, void *tmpl, void *pool);
extern f32 *GuiSpriteGetTextureVec(GuiElement *e);
extern void func_00338AB8(void *listHead, void *pool);
#endif

/* TODO(match) GuiManagerInitHudLists - task #566 (round 4), measured on the COMMITTED tree (this file,
 * both arms promoted whole-unit; instrument: tools/ee/unit_report.sh over
 * tools/ee/objdiff_build.sh, clean): sdk29 67.93%, engine96 66.73%. Eligible arm: e96.
 * Residual: ORDER + const-materialisation regalloc (at vs v1) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", GuiManagerInitHudLists);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void GuiListElementInit(GuiElement *e, s32 v3C, s32 v34, void *tag, void *pool);
extern void GuiListSetVisibleRows(GuiElement *e, s32 rows);
extern void GuiListSetItemCount(GuiElement *e, s32 n);
extern void GuiListSetScrollPos(GuiElement *e, s32 pos);
extern void GuiListSetColorPair0(GuiElement *e, s32 c0, s32 c1);
extern void GuiListSetColorPair1(GuiElement *e, s32 c0, s32 c1);
extern void func_00337B68(GuiElement *e, s32 v);
extern void func_0034C008(void *listHead, void *gui, void *pool);
extern void GuiSpriteElementInit(GuiElement *e, void *tmpl, void *pool);
extern f32 *GuiSpriteGetTextureVec(GuiElement *e);
extern void func_00338AB8(void *listHead, void *pool);
extern void *g_guiInstance;
extern u8 D_1AE6C8[];
extern u8 D_1AE6D8[];
/* (end of this body's declarations) */
/* Structure-exact model (cmp-oracle blocked, same abs GUI-vtable --gc-sections
   wall as GuiManagerInitListRows; matching arm stays asm).
   Build the HUD manager's list widgets: cache the two selector glyphs ('W','X'
   at +0x222C/+0x2230), configure the 26 scrollable list rows (visible=5,
   items=100, two color pairs, per-row finalizers), then set up the frame-list
   sub-object (+0x7A0), the frame sprite (+0x1D50: two texture-vec constants),
   record the pool at +0x798, and init the two trailing list heads
   (+0x1D8C/+0x1FDC). `pool` is the GUI element pool/context. */
void GuiManagerInitHudLists(void *inst, void *gui, void *pool) {
    u8 *row = (u8 *)inst;
    u8 *rowEnd = (u8 *)inst + 0x750;
    GuiElement *sprite;
    f32 *vec;

    *(s32 *)((u8 *)inst + 0x222C) =
        GuiFontAtlasLookupGlyph((char *)g_guiInstance + 0x8710, 0x57);
    *(s32 *)((u8 *)inst + 0x2230) =
        GuiFontAtlasLookupGlyph((char *)g_guiInstance + 0x8710, 0x58);

    do {
        GuiElement *e = (GuiElement *)row;
        GuiListElementInit(e, 0x20, 1, D_1AE6C8, pool);
        GuiListSetVisibleRows(e, 5);
        GuiListSetItemCount(e, 0x64);
        GuiListSetScrollPos(e, 0);
        GuiListSetColorPair0(e, 0x8049C1FF, 0x80001EFF);
        GuiListSetColorPair1(e, 0x50F0C070, 0x50F0C070);
        func_00337B88(e, (s32)0x80000000);
        func_00337B68(e, 1);
        row += 0x48;
    } while (row < rowEnd);

    func_0034C008((u8 *)inst + 0x7A0, gui, pool);

    sprite = (GuiElement *)((u8 *)inst + 0x1D50);
    GuiSpriteElementInit(sprite, D_1AE6D8, pool);
    GuiElementGetColor(sprite)[0] = 0x4F008080;   /* packed float const */
    vec = GuiSpriteGetTextureVec(sprite);
    *(s32 *)vec = 0x476A6800;                      /* packed float const */
    vec = GuiSpriteGetTextureVec(sprite);
    ((s32 *)vec)[1] = 0;

    *(void **)((u8 *)inst + 0x798) = pool;
    func_00338AB8((u8 *)inst + 0x1D8C, pool);
    func_00338AB8((u8 *)inst + 0x1FDC, pool);
    __asm__ __volatile__("");
}
#endif

/* func_0034EF60: empty/no-op manager hook (original compiles to jr ra; nop).
 * Its one ROM caller, func_0034F240, passes the GuiInstance in $a0
 * (`daddu $4,$16,$0` in the jal delay slot), so it takes it and ignores it. */
void func_0034EF60(void *gui) {
    (void)gui;
}

/* func_0034EF68: initialize the `index`-th HUD list row (each row is a 0x48-byte
 * GuiElement at base + index*0x48). It sets the row's two leading floats from the
 * integer args `x` and `y` (its primary vec, fetched via func_00336C18), packs
 * `shade` into both color blocks (func_003372D0/func_00337310, byte b0==b1) and
 * its top-byte handle (func_00337B88, shade<<24), then dispatches the row's
 * type-specific finalizer through the small vtable at row+0x30: it reads a s16
 * element-offset at vtable+0x8 and the function pointer at vtable+0xC and calls
 * it on (row + offset). `x`/`y` are integer pixel coords; `shade` an 8-bit
 * intensity. */
/* Task #566 (round 4) measured this body on the 2.9 / 2.96 arms (instrument:
 * tools/ee/unit_report.sh over tools/ee/objdiff_build.sh, clean): sdk29 89.02%,
 * engine96 83.91%, residual ORDER.
 * MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
 * byte-exact (FACT #8810; census FACT #8830).
 * GUARD (task #1272): on EE this C is the image's body, compiled alone by the
 * s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
 * S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
 * that skips the splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034EF68)
S136OS_SLOT(func_0034EF68);
#else
typedef struct GuiRowVtable {
    /* 0x00 */ u8 _pad[8];
    /* 0x08 */ s16 elemOffset;
    /* 0x0A */ u8 _pad0A[2];
    /* 0x0C */ void (*finalize)(void *elem);
} GuiRowVtable;

void func_0034EF68(u8 *base, s32 index, s32 x, s32 y, s32 shade) {
    GuiElement *row = (GuiElement *)(base + index * 0x48);
    GuiRowVtable *vt;
    f32 *vec;
    vec = func_00336C18(row);
    vec[0] = (f32)x;
    vec = func_00336C18(row);
    vec[1] = (f32)y;
    func_003372D0(row, shade, shade);
    func_00337310(row, shade, shade);
    func_00337B88(row, shade << 24);
    vt = *(GuiRowVtable **)((u8 *)row + 0x30);
    vt->finalize((u8 *)row + vt->elemOffset);
}
#endif

#ifdef TARGET_NATIVE
extern f32 func_00283B30(f32 angle);   /* VU0 cosine */
extern f32 func_00283B48(f32 angle);   /* VU0 sine   */
extern f32 D_1AE6E4;   /* radial X gain */
extern f32 D_1AE6E8;   /* radial Y gain */
extern s32 D_1AE6F0;   /* currently-highlighted slot index */
extern s32 D_1AE6EC;   /* highlighted-slot vertical bump */
extern void *g_hudMobySpawnStart;   /* HUD record base; +0x28 = slot count */
#endif

/* TODO(match) func_0034F028 - task #566 (round 4), measured on the COMMITTED tree (this file,
 * both arms promoted whole-unit; instrument: tools/ee/unit_report.sh over
 * tools/ee/objdiff_build.sh, clean): sdk29 77.83%, engine96 70.34%. Eligible arm: e96.
 * Residual: ORDER (both pi literals now bit-exact) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F028);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern f32 func_00283B30(f32 angle);
extern f32 func_00283B48(f32 angle);
extern void func_0034EF68(u8 *base, s32 index, s32 x, s32 y, s32 shade);
extern void *g_hudMobySpawnStart;
extern f32 D_1AE6E4;
extern f32 D_1AE6E8;
extern s32 D_1AE6F0;
extern s32 D_1AE6EC;
/* (end of this body's declarations) */
/* Structure-exact model (cmp-oracle blocked, abs/gp GUI globals; matching arm
   stays asm). Lay out the (up to 8) weapon-wheel slots on a circle: for slot i,
   angle = 2*pi*i/count - pi/2, placing the HUD element at
   (baseX + cos*gainX*74 + 0x69, baseY + sin*gainY*74 + 0x64); the highlighted
   slot (D_1AE6F0) gets an extra vertical bump (D_1AE6EC). `shade` is forwarded
   to each element initializer. No-op if the slot count is 0. (s32 casts model
   cvt.w.s; sub-pixel rounding isn't oracle-critical for this low-blast layout.) */
void func_0034F028(u8 *base, s32 baseX, s32 baseY, s32 shade) {
    s32 i;
    if (*(s32 *)((u8 *)&g_hudMobySpawnStart + 0x28) == 0) {
        return;
    }
    for (i = 0; i < 8; i++) {
        f32 count = (f32)*(s32 *)((u8 *)&g_hudMobySpawnStart + 0x28);
        f32 angle = 2.0f * (f32)i;
        s32 x, y;
        angle = angle * 3.1415927411f; /* 0x40490FDB pi on cc1 2.96, cc1 2.9 and native (task #1243) */
        angle = angle / count;
        angle = angle - 3.1415927411f; /* same spelling as the multiply so cc1 CSEs one pi */
        angle = angle + 1.5707964f;    /* 0x3FC90FDB pi/2 */
        x = baseX + (s32)(func_00283B30(angle) * (D_1AE6E4 * 74.0f)) + 0x69;
        y = baseY + (s32)(func_00283B48(angle) * (D_1AE6E8 * 74.0f)) + 0x64;
        if (i == D_1AE6F0) {
            func_0034EF68(base, i, x, y + D_1AE6EC, shade);
        } else {
            func_0034EF68(base, i, x, y, shade);
        }
    }
}
#endif

/* func_0034F1C0: per-frame tick of the outer GuiInstance's HUD widgets - run the
 * embedded GuiHudManager's frame update (+0x7A0 via func_0034DBE0), forward the
 * HUD list (func_0034F200), tick the third (planet-name) list (+0x1FDC via
 * func_00339A88), then update the on-screen bolt counter (TickBoltCounterHud).
 * BYTE-EXACT under the ENGINE compiler (#259): ee-gcc 2.96 + the engine
 * pipeline reproduces all 16 ROM words and all 4 relocations, once the trailing
 * `__asm__ __volatile__("")` suppresses the sibling call into
 * TickBoltCounterHud. Build it with
 * `tools/ee/diff96.sh usa text/24D728 func_0034F1C0 <this file>`.
 *
 * ee-gcc 2.9 differs in exactly 5 prologue/epilogue words (0x20 frame with
 * 16-byte save slots against the ROM's 0x10 with 8-byte slots): 99.06% at the
 * unit objdiff gate, 82.81% with the guard removed. The image now takes this
 * body from the s136os arm (task #1309, below); the MATCH_ route is historical. */
/* GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0034F1C0 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034F1C0)
S136OS_SLOT(func_0034F1C0);
#else
void func_0034F1C0(GuiInstance *mgr) {
    u8 *m = (u8 *)mgr;
    func_0034DBE0(m + 0x7A0);
    func_0034F200(mgr);
    func_00339A88(m + 0x1FDC);
    TickBoltCounterHud();
    __asm__ __volatile__("");
}
#endif

/* func_0034F200: forward the outer GuiInstance's HUD list (+0x1D8C) to
 * func_00339A88. (mgr here is the OUTER GuiInstance, not the +0x7A0
 * GuiHudManager: +0x1D8C is the list field GuiManagerInitHudLists initializes at
 * `$17+0x1D8C`.) The empty asm guard blocks cc1's void sibling-call. */
void func_0034F200(GuiInstance *mgr) {
    func_00339A88((u8 *)mgr + 0x1D8C);
    __asm__ __volatile__("");
}

/* func_0034F220: forward the outer GuiInstance's HUD list (+0x1D8C) to
 * func_00339F98 (same object/field as func_0034F200). */
void func_0034F220(GuiInstance *mgr) {
    func_00339F98((u8 *)mgr + 0x1D8C);
    __asm__ __volatile__("");
}

/*
 * func_0034F240(gui) — refresh the HUD under a temporary camera-projection
 * override. Saves the scene camera's projection param (g_sceneActorMobys+0x674,
 * field +0xB0), forces it to 0.62, and rebuilds the projection
 * (BuildCameraProjection). Then, unless the suppress flag D_1A8C64 is set, it
 * re-reads the HUD manager's packed color (func_0034D7A8 on the embedded
 * GuiHudManager at gui+0x7A0) and re-applies it via func_0034F028 (with the two
 * gp-globals D_1AE6F4/D_1AE6F8 and the color's top byte). It runs the manager
 * no-op (func_0034EF60), re-lays the HUD manager (func_0034E8D8), clears the two
 * GUI manager status words at g_guiInstance+0x38000+0x79E0/+0x79DC while
 * forwarding the HUD list (func_00339F98 on gui+0x1FDC), ticks func_0029D9B8,
 * then restores the saved projection param and rebuilds once more.
 * gui: the GuiInstance. Returns nothing. */
#ifdef TARGET_NATIVE
extern void *g_guiInstance;
extern void BuildCameraProjection(void);
extern void func_0029D9B8(void);
extern void func_0034F028(u8 *obj, s32 a, s32 b, s32 c);
extern void func_0034E8D8(void *p);
extern u8   g_sceneActorMobys[];      /* 0x1B894C  scene cast moby ptrs (byte base) */
extern s32  D_1A8C64;                 /* HUD-refresh suppress flag */
extern s32  D_1AE6F4;                 /* gp-global passed to func_0034F028 */
extern s32  D_1AE6F8;                 /* gp-global passed to func_0034F028 */
#endif

/* ADDRESSING-MODEL DEVICE (RULING #8620, the 191238.cpp equate form):
 * func_0034F240 reads D_1A8C64 and g_guiInstance absolutely (lui/lw at
 * 0x34F278 and 0x34F2B4). Declared as plain ints cc1 writes `.extern <sym>, 4`
 * and gas makes both reads one-word %gp_rel (the body comes out 46 words, the
 * ROM's is 48). Size 16 pins them absolute; the equated names keep the size
 * off the real symbols, and the relocations still name them. Top level, so
 * the s136os TU and the spliced 2.9 TU see the same lines. */
#ifndef TARGET_NATIVE
__asm__(".extern D_1A8C64Abs, 16\n\tD_1A8C64Abs = D_1A8C64");
extern s32 D_1A8C64Abs;
__asm__(".extern g_guiInstanceAbs, 16\n\tg_guiInstanceAbs = g_guiInstance");
extern void *g_guiInstanceAbs;
#else
#define D_1A8C64Abs D_1A8C64
#define g_guiInstanceAbs g_guiInstance
#endif

#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034F240)
S136OS_SLOT(func_0034F240);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void BuildCameraProjection(void);
extern void func_0034F028(u8 *obj, s32 a, s32 b, s32 c);
extern void func_0034E8D8(void *p);
extern void func_0029D9B8(void);
extern u8 g_sceneActorMobys[];
extern s32 D_1A8C64;
extern s32 D_1AE6F4;
extern s32 D_1AE6F8;
extern void *g_guiInstance;
/* (end of this body's declarations) */
/* MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (task #1669). Three spellings carry it. The parameter is used
   directly: a `u8 *gui = (u8 *)guiArg` local gives cc1 a second pseudo and an
   extra callee-saved copy ($17 = $16, $18 for cam). Both status-word stores
   come BEFORE func_00339F98, +0x79DC first in source: the ROM issues +0x79E0
   and fills the jal delay slot with +0x79DC, with the base in a temp ($3), not
   a saved register. func_0034EF60 is called with gui, as the ROM does.
   GUARD: on EE this C is the image's body, compiled alone by the s136os arm
   (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
   tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
   splice drops the function. On native it is plain C. */
void func_0034F240(void *gui) {
    u8  *cam = g_sceneActorMobys + 0x674;   /* scene camera params */
    f32  saved = *(f32 *)(cam + 0xB0);

    *(f32 *)(cam + 0xB0) = 0.62f;
    BuildCameraProjection();

    if (D_1A8C64Abs == 0) {
        s32 packed = func_0034D7A8((GuiHudManager *)((u8 *)gui + 0x7A0));
        func_0034F028((u8 *)gui, D_1AE6F4, D_1AE6F8, (s32)((u32)packed >> 24));
    }

    func_0034EF60(gui);
    func_0034E8D8((u8 *)gui + 0x7A0);

    {
        u8 *mgr = (u8 *)g_guiInstanceAbs + 0x38000;
        *(s32 *)(mgr + 0x79DC) = 0;
        *(s32 *)(mgr + 0x79E0) = 0;
        func_00339F98((u8 *)gui + 0x1FDC);
    }

    func_0029D9B8();

    *(f32 *)(cam + 0xB0) = saved;
    BuildCameraProjection();
}
#endif

/* func_0034F300: identity - return the manager pointer unchanged (a vtable
 * "get self" accessor). Typed as GuiInstance * by association with the adjacent
 * func_0034F200/F220 outer-object forwarders; the body is pure `return mgr`, so
 * the choice of pointee is byte-neutral (UNCONFIRMED which object). */
GuiInstance *func_0034F300(GuiInstance *mgr) {
    return mgr;
}

/*
 * GuiSystemInit(gui) — one-time construction of the whole GUI singleton (the
 * huge g_guiInstance block, base = gui). Zeroes the 0x8700-byte working area at
 * gui+4, then registers every GUI sub-object in two passes over the fixed slot
 * layout: pass 1 calls each screen's constructor func_00xxxxxx(gui+slotOff);
 * pass 2 calls each screen's <Name>Init(gui+slotOff, ctx) where ctx = the shared
 * manager context at gui+0x36F10. In between it publishes the singleton pointer
 * (g_guiInstance = gui), aligns the pool cursor into gui[0], kicks the font-atlas
 * disc load (StartFileLoadPumpingVoice over g_discToc entries) + relocate, sizes
 * the GUI pool (GuiPoolInit, 0x8700 bytes of 0x10-granule blocks at gui+4), wires
 * the HUD lists (GuiManagerInitHudLists), clears the manager status words at
 * gui+0x38000+0x79D0.., and finally zeroes two small tables. Returns gui.
 *
 * Purely integer/pointer construction (no float, no GS/DMA). The matching build
 * keeps the asm (engine save-layout wall); the #else is a faithful op-for-op
 * transcription. GuiManagerInitListRows/GuiManagerInitHudLists live in this unit
 * as INCLUDE_ASM; the ~50 per-screen constructors live in neighbouring units.
 */
#ifdef TARGET_NATIVE
extern void *g_guiInstance;
extern s32   g_discToc[];             /* 0x14B540  disc TOC (sector tables) */
extern void  StartFileLoadPumpingVoice(void *dest, s32 startSector, s32 sectorCount);
extern void  GuiFontAtlasRelocate(void *atlas);
extern void  GuiPoolInit(void *pool, s32 granule, void *base, s32 size);
extern void *GuiManagerInitListRows(void *listRows);
extern void  GuiManagerInitHudLists(void *listRows, void *gui, void *ctx);
/* pass-1 per-screen constructors (screen sub-object pointer only) */
extern void  func_00337C58(void *ctx);
extern void  func_00349200(void *s);
extern void  func_003475F0(void *s);
extern void  func_00346368(void *s);
extern void  func_003448C0(void *s);
extern void  func_0033FF68(void *s);
extern void  func_0033AA80(void *s);
extern void  func_0033A048(void *s);
extern void  func_0033C100(void *s);
extern void  func_0033F690(void *s);
extern void  func_003420D0(void *s);
extern void  func_00344110(void *s);
extern void  func_0033F4D0(void *s);
extern void  func_00341C28(void *s);
extern void  func_00349E88(void *s);
extern void  func_0033A640(void *s);
extern void  GuiQuitDialogInitElements(void *s);
extern void  func_0033D478(void *s);
extern void  func_0033DA00(void *s);
extern void  func_0033DDC8(void *s);
extern void  func_0033E488(void *s);
extern void  func_0033CD80(void *s);
extern void  func_0033D1C0(void *s);
extern void  func_00348BD0(void *s);
extern void  func_00342FD8(void *s);
extern void  func_00342BA0(void *s);
extern void  func_0033EB20(void *s);
extern void  func_0033EDD0(void *s);
extern void  func_0033EFC8(void *s);
extern void  func_0033F1C0(void *s);
extern void  func_0033B4E8(void *s);
/* pass-2 per-screen initializers (screen sub-object + shared ctx) */
extern void  GuiScreenWithPlanetNameInit(void *s, void *ctx);
extern void  GuiWeaponGridScreenInit(void *s, void *ctx);
extern void  GuiMapScreenInit(void *s, void *ctx);
extern void  GuiInfoPanelScreenInit(void *s, void *ctx);
extern void  GuiQuickSelectWheelInit(void *s, void *ctx);
extern void  GuiLevelInfoPanelInit(void *s, void *ctx);
extern void  GuiScrollListScreenInit(void *s, void *ctx);
extern void  GuiConfirmPopupInit(void *s, void *ctx);
extern void  GuiIconScreenInit(void *s, void *ctx);
extern void  GuiTitledSpriteScreenInit(void *s, void *ctx);
extern void  GuiIconListScreenInit(void *s, void *ctx);
extern void  GuiStatsPanelScreenInit(void *s, void *ctx);
extern void  func_0033F510(void *s, void *ctx);
extern void  func_00349E90(void *s);
extern void  GuiProgressBarWidgetInit(void *s, void *ctx);
extern void  GuiQuitDialogInit(void *s, void *ctx);
extern void  func_0033D4B0(void *s, void *ctx);
extern void  func_0033A678(void *s, void *ctx);
extern void  GuiDialogBoxVariantBInit(void *s, void *ctx);
extern void  func_0033DE10(void *s, void *ctx);
extern void  func_0033E4C0(void *s, void *ctx);
extern void  GuiDialogBoxVariantCInit(void *s, void *ctx);
extern void  func_0033D1F8(void *s, void *ctx);
extern void  func_00348BF8(void *s, void *ctx);
extern void  GuiHelpPromptWidgetInit(void *s, void *ctx);
extern void  GuiIconScreenInit2(void *s, void *ctx);
extern void  func_0033EB58(void *s, void *ctx);
extern void  func_0033EE08(void *s, void *ctx);
extern void  func_0033F000(void *s, void *ctx);
extern void  func_0033F200(void *s, void *ctx);
#endif

/* TODO(match) GuiSystemInit - task #566 (round 4), measured on the COMMITTED tree (this file,
 * both arms promoted whole-unit; instrument: tools/ee/unit_report.sh over
 * tools/ee/objdiff_build.sh, clean): sdk29 81.91%, engine96 52.60%. Eligible arm: e96.
 * Residual: ORDER, 351 body rows */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", GuiSystemInit);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_00337C58(void *ctx);
extern void *GuiManagerInitListRows(void *listRows);
extern void func_00349200(void *s);
extern void func_003475F0(void *s);
extern void func_00346368(void *s);
extern void func_003448C0(void *s);
extern void func_0033FF68(void *s);
extern void func_0033AA80(void *s);
extern void func_0033A048(void *s);
extern void func_0033C100(void *s);
extern void func_0033F690(void *s);
extern void func_003420D0(void *s);
extern void func_00344110(void *s);
extern void func_0033F4D0(void *s);
extern void func_00341C28(void *s);
extern void func_00349E88(void *s);
extern void func_0033A640(void *s);
extern void GuiQuitDialogInitElements(void *s);
extern void func_0033D478(void *s);
extern void func_0033DA00(void *s);
extern void func_0033DDC8(void *s);
extern void func_0033E488(void *s);
extern void func_0033CD80(void *s);
extern void func_0033D1C0(void *s);
extern void func_00348BD0(void *s);
extern void func_00342FD8(void *s);
extern void func_00342BA0(void *s);
extern void func_0033EB20(void *s);
extern void func_0033EDD0(void *s);
extern void func_0033EFC8(void *s);
extern void func_0033F1C0(void *s);
extern void func_0033B4E8(void *s);
extern void StartFileLoadPumpingVoice(void *dest, s32 startSector, s32 sectorCount);
extern void GuiFontAtlasRelocate(void *atlas);
extern void GuiPoolInit(void *pool, s32 granule, void *base, s32 size);
extern void GuiManagerInitHudLists(void *listRows, void *gui, void *ctx);
extern void GuiScreenWithPlanetNameInit(void *s, void *ctx);
extern void GuiWeaponGridScreenInit(void *s, void *ctx);
extern void GuiMapScreenInit(void *s, void *ctx);
extern void GuiInfoPanelScreenInit(void *s, void *ctx);
extern void GuiQuickSelectWheelInit(void *s, void *ctx);
extern void GuiLevelInfoPanelInit(void *s, void *ctx);
extern void GuiScrollListScreenInit(void *s, void *ctx);
extern void GuiConfirmPopupInit(void *s, void *ctx);
extern void GuiIconScreenInit(void *s, void *ctx);
extern void GuiTitledSpriteScreenInit(void *s, void *ctx);
extern void GuiIconListScreenInit(void *s, void *ctx);
extern void GuiStatsPanelScreenInit(void *s, void *ctx);
extern void func_0033F510(void *s, void *ctx);
extern void func_00349E90(void *s);
extern void GuiProgressBarWidgetInit(void *s, void *ctx);
extern void GuiQuitDialogInit(void *s, void *ctx);
extern void func_0033D4B0(void *s, void *ctx);
extern void func_0033A678(void *s, void *ctx);
extern void GuiDialogBoxVariantBInit(void *s, void *ctx);
extern void func_0033DE10(void *s, void *ctx);
extern void func_0033E4C0(void *s, void *ctx);
extern void GuiDialogBoxVariantCInit(void *s, void *ctx);
extern void func_0033D1F8(void *s, void *ctx);
extern void func_00348BF8(void *s, void *ctx);
extern void GuiHelpPromptWidgetInit(void *s, void *ctx);
extern void GuiIconScreenInit2(void *s, void *ctx);
extern void func_0033EB58(void *s, void *ctx);
extern void func_0033EE08(void *s, void *ctx);
extern void func_0033F000(void *s, void *ctx);
extern void func_0033F200(void *s, void *ctx);
#ifndef TARGET_NATIVE
extern void *memset(void *s, int c, unsigned int n); /* native: <string.h> via common.h */
#endif
extern void *g_guiInstance;
extern s32 g_discToc[];
/* (end of this body's declarations) */
void *GuiSystemInit(void *guiArg) {
    u8 *gui = (u8 *)guiArg;
    u8 *ctx = gui + 0x36F10;   /* shared GUI manager context ($17) */
    u8 *mgr = gui + 0x38000;   /* manager status block ($18) */
    s32 *p;
    s32  i;

    /* zero the 0x8700-byte working area at gui+4 (0x870 x 0x10-byte rows) */
    p = (s32 *)(gui + 4);
    for (i = 0x86F; i != -1; i--) {
        p[0] = 0;
        p[1] = 0;
        p[2] = 0;
        p[3] = 0;
        p += 4;
    }

    /* pass 1 — construct each sub-object */
    func_00337C58(ctx);
    GuiManagerInitListRows(gui + 0x36F28);
    func_00349200(gui + 0x39160);
    func_003475F0(gui + 0x39620);
    func_00346368(gui + 0x39AF0);
    func_003448C0(gui + 0x3A000);
    func_0033FF68(gui + 0x3A4C0);
    func_0033AA80(gui + 0x3AF80);
    func_0033A048(gui + 0x3B418);
    func_0033C100(gui + 0x3B6F8);
    func_0033F690(gui + 0x3BA58);
    func_003420D0(gui + 0x3C160);
    func_00344110(gui + 0x3C480);
    func_0033F4D0(gui + 0x3C7E8);
    func_00341C28(gui + 0x3CB20);
    func_00349E88(gui + 0x3CD48);
    func_0033A640(gui + 0x3CEA0);
    GuiQuitDialogInitElements(gui + 0x3D1D8);
    func_0033D478(gui + 0x3D4B8);
    func_0033DA00(gui + 0x3D798);
    func_0033DDC8(gui + 0x3DA78);
    func_0033E488(gui + 0x3DDE8);
    func_0033CD80(gui + 0x3E0C8);
    func_0033D1C0(gui + 0x3E3A8);
    func_00348BD0(gui + 0x3E688);
    func_00342FD8(gui + 0x3E760);
    func_00342BA0(gui + 0x3E9C8);
    func_0033EB20(gui + 0x3EB50);
    func_0033EDD0(gui + 0x3EE30);
    func_0033EFC8(gui + 0x3F110);
    func_0033F1C0(gui + 0x3F3F0);
    func_0033B4E8(gui + 0x3F7B0);

    /* publish the singleton, align the pool cursor into gui[0] */
    g_guiInstance = gui;
    *(s32 *)(gui + 0) = ((s32)(gui + 0x3FB20) + 3) & ~3;
    *(s32 *)(mgr + 0x79D0) = 0x3FB20;

    /* load + relocate the font atlas from disc, size the GUI pool */
    StartFileLoadPumpingVoice(gui + 0x8710,
                              g_discToc[0x16B8 / 4] + g_discToc[0x36C / 4],
                              g_discToc[0x16BC / 4]);
    GuiFontAtlasRelocate(gui + 0x8710);
    GuiPoolInit(ctx, 0x10, gui + 4, 0x8700);
    GuiManagerInitHudLists(gui + 0x36F28, gui, ctx);

    /* clear the leading manager status words, then pass 2 — init each screen */
    *(s32 *)(mgr + 0x79D4) = 0;
    *(s32 *)(mgr + 0x79D8) = 0;
    *(s32 *)(mgr + 0x79DC) = 0;
    *(s32 *)(mgr + 0x79E0) = 0;
    GuiScreenWithPlanetNameInit(gui + 0x39160, ctx);
    GuiWeaponGridScreenInit(gui + 0x39620, ctx);
    GuiMapScreenInit(gui + 0x39AF0, ctx);
    GuiInfoPanelScreenInit(gui + 0x3A000, ctx);
    GuiQuickSelectWheelInit(gui + 0x3A4C0, ctx);
    GuiLevelInfoPanelInit(gui + 0x3AF80, ctx);
    GuiScrollListScreenInit(gui + 0x3CB20, ctx);
    GuiConfirmPopupInit(gui + 0x3B418, ctx);
    GuiIconScreenInit(gui + 0x3C160, ctx);
    GuiTitledSpriteScreenInit(gui + 0x3C480, ctx);
    GuiIconListScreenInit(gui + 0x3B6F8, ctx);
    GuiStatsPanelScreenInit(gui + 0x3BA58, ctx);
    func_0033F510(gui + 0x3C7E8, ctx);
    func_00349E90(gui + 0x3CD48);
    GuiProgressBarWidgetInit(gui + 0x3F7B0, ctx);
    GuiQuitDialogInit(gui + 0x3D1D8, ctx);
    func_0033D4B0(gui + 0x3D4B8, ctx);
    func_0033A678(gui + 0x3CEA0, ctx);
    GuiQuitDialogInit(gui + 0x3D1D8, ctx);
    func_0033D4B0(gui + 0x3D4B8, ctx);
    GuiDialogBoxVariantBInit(gui + 0x3D798, ctx);
    func_0033DE10(gui + 0x3DA78, ctx);
    func_0033E4C0(gui + 0x3DDE8, ctx);
    GuiDialogBoxVariantCInit(gui + 0x3E0C8, ctx);
    func_0033D1F8(gui + 0x3E3A8, ctx);
    func_00348BF8(gui + 0x3E688, ctx);
    GuiHelpPromptWidgetInit(gui + 0x3E9C8, ctx);
    GuiIconScreenInit2(gui + 0x3E760, ctx);
    func_0033EB58(gui + 0x3EB50, ctx);
    func_0033EE08(gui + 0x3EE30, ctx);
    func_0033F000(gui + 0x3F110, ctx);
    func_0033F200(gui + 0x3F3F0, ctx);

    /* clear the trailing manager status words, zero two small tables */
    *(s32 *)(mgr + 0x79F4) = 0;
    *(s32 *)(mgr + 0x79EC) = 0;
    *(s32 *)(mgr + 0x79F0) = 0;
    memset(gui + 0x3F9F8, 0, 0x20);
    memset(gui + 0x3FA18, 0, 0x100);
    return gui;
}
#endif

/* func_0034F868 callees. WaitGsPathsIdle is declared s32: its ROM epilogue
 * leaves 0 in $v0 (`daddu $2,$0,$0` in the bnez delay slot), and with a `void`
 * declaration the s136os compile puts the loop-tail count reload in $v0 where
 * the ROM uses $v1 (3 words, task #1620). */
extern void WaitFrameDmaFence(s32 mask);
extern s32  WaitGsPathsIdle(s32 a, s32 b);
extern void func_0011AEA0(s32 arg);                       /* pre-RPC flush/sync */
extern void KickGifImageUpload(void *packet, void *vramDest);
extern void func_00126288(void *dst, s32 texId, s32 a, s32 b, s32 c,
                        s32 d, s32 width, s32 height);    /* build a GS image-upload GIF packet */

/* The texture-upload queue func_0034F868 drains: eight 0x400-byte upload slots
 * at +0xE00, their texture ids (low halfword used) at +0x2E00 and the queued
 * count at +0x2E20. Only the fields func_0034F868 touches are named. */
struct TexUploadQueue {
    u8  pad0[0xE00];
    u8  slots[8][0x400];
    s32 ids[8];
    s32 count;
};

/** func_0034F868 — flush the queued texture image-uploads for this manager. Wait
 *  one frame-DMA fence, then for each of the q->count queued entries build a
 *  16x16 GS image-upload GIF packet (func_00126288) for the entry's texture id
 *  (the low halfword of q->ids[i]) into a stack scratch, issue it to the entry's
 *  slot q->slots[i] via KickGifImageUpload, and wait for the GS paths to idle
 *  between uploads. Clears the count when done, also when it was <= 0.
 *  base: the queue (struct TexUploadQueue). Returns nothing. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034F868)
S136OS_SLOT(func_0034F868);
#else
/* MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (task #1620). Two things closed it. The WaitGsPathsIdle
   declaration (above) moved the loop-tail lw/slt/bnel from $2 to $3. The plain
   indexed for loop lets loop strength reduction build the two cursors after the
   count test (slots in the blez delay slot, then ids at +0x2E00 with the offset
   in the cursor, lh 0x0) with base in $19 and i in $18, as the ROM does. The old
   do-while with hand-written cursors rotated s0..s3, and an indexed
   *(s16 *)(base + 0x2E00 + i * 4) folds the 0x2E00 into the lh offset instead.
   GUARD: on EE this C is the image's body, compiled alone by the s136os arm
   (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
   tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
   splice drops the function. On native it is plain C. */
void func_0034F868(u8 *base) {
    struct TexUploadQueue *q = (struct TexUploadQueue *)base;
    u8  packet[0x60];
    s32 i;

    WaitFrameDmaFence(1);
    for (i = 0; i < q->count; i++) {
        func_00126288(packet, (s16)q->ids[i], 1, 0, 0, 0, 0x10, 0x10);
        func_0011AEA0(0);
        KickGifImageUpload(packet, q->slots[i]);
        WaitGsPathsIdle(0, 0);
    }
    q->count = 0;
}
#endif

/* func_0034F928 globals/callee. */
extern u8   g_dirLightMatrices[];    /* 0x1C26C0, 0x40-stride directional-light matrices */
extern void func_00283638(void *dst);/* zero a 16-byte quadword at dst */

/** func_0034F928 — install the fixed directional light in matrix slot 14
 *  (g_dirLightMatrices + 0x380): RGB colour (0.4, 0.8, 1.2) at +0x380/+0x384/
 *  +0x388, a normalised diagonal direction (~0.577, ~0.577, ~-0.577) at +0x390/
 *  +0x394/+0x398 with zero w-components at +0x38C/+0x39C, then clear the two
 *  16-byte matrix-blend blocks at +0x3A0 and +0x3B0. Called with the light-setup
 *  context (its sibling F9B8/F9F8/FAF8 use it), but this one writes only the
 *  global directional light, so ctx is unused here. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034F928)
S136OS_SLOT(func_0034F928);
#else
/* MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (task #1669), every field stored in address order with plain
   float literals; cc1 emits each as `li.s` (lui/ori/mtc1) and schedules the
   rest itself. The old union-routed direction value, the hand-interleaved
   order and the trailing empty asm were all unnecessary.
   GUARD: on EE this C is the image's body, compiled alone by the s136os arm
   (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
   tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
   splice drops the function. On native it is plain C. */
void func_0034F928(void *ctx) {
    u8 *m = g_dirLightMatrices;
    (void)ctx;

    *(f32 *)(m + 0x380) = 0.4f;     /* colour */
    *(f32 *)(m + 0x384) = 0.8f;
    *(f32 *)(m + 0x388) = 1.2f;
    *(s32 *)(m + 0x38C) = 0;
    *(f32 *)(m + 0x390) = 0.577f;   /* direction */
    *(f32 *)(m + 0x394) = 0.577f;
    *(f32 *)(m + 0x398) = -0.577f;
    *(s32 *)(m + 0x39C) = 0;
    func_00283638(m + 0x3A0);
    func_00283638(m + 0x3B0);
}
#endif

/* func_0034F9B8: reset the camera-state HUD sub-block (clear the three ints at
 * +0x140/+0x144/+0x148), run func_00283D10 on its +0x370 sub-object, then
 * rebuild the frame view matrices. No params, returns nothing. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034F9B8)
S136OS_SLOT(func_0034F9B8);
#else
/* MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (task #1669). Closed by REMOVING the trailing empty asm: with it,
   sched2 issued the %lo addiu before `sd $ra` (2 words); without it the ROM
   order comes out.
   GUARD: on EE this C is the image's body, compiled alone by the s136os arm
   (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
   tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
   splice drops the function. On native it is plain C. */
void func_0034F9B8(void) {
    u8 *cam = g_cameraState;
    *(s32 *)(cam + 0x140) = 0;
    *(s32 *)(cam + 0x144) = 0;
    *(s32 *)(cam + 0x148) = 0;
    func_00283D10(cam + 0x370);
    BuildFrameViewMatrices();
}
#endif

/* func_0034F9F0: mis-split fragment (lone mid-fn `sh $3,0xB6($5)` store, no
 * prologue/jr) - #47 resplit-pass backlog, not #else material. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F9F0);

/* func_0034F9F8: per-frame update of the two animated HUD moby sub-objects the
 * manager owns (one at mgr+0xC00, plus a sibling region at mgr+0x600). For each
 * it runs the moby reset/prep (func_0026F790) and animation step
 * (UpdateMobyAnimation); then, if the active sequence byte (+0x42 / +0x43) is not
 * 0xFF, it resolves the current animation frame's geometry pointers out of the
 * moby's class seq/frame tables and installs them via func_002A1F68, caching the
 * owning moby base into +0x58 / +0x5C. Finally it recomputes the moby's bounding
 * sphere and grid placement (UpdateMobyBSphereAndGrid). `mgr` is the manager base.
 *
 * Frame-pointer arithmetic (mirrors UpdateActiveMobys' moby anim setup):
 *   cls = moby2->classTable (+0x24); seqDef = *(cls + seq*4 + 0x48);
 *   framePtr = (u8*)seqDef + (seqDef[0x10]<<2) + 0x1C + (seqDef[0x13]<<2);
 *   frame0Ptr = *(seqDef + frame0*4 + 0x1C); classFlag = cls[0x8]. */
/* EE register pin; a no-op on the native build. */
#ifndef TARGET_NATIVE
#define EE_REG(r) __asm__(r)
#else
#define EE_REG(r)
#endif

#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034F9F8)
S136OS_SLOT(func_0034F9F8);
#else
typedef struct MobyAnimSeqDef {
    /* 0x00 */ u8 _pad00[0x10];
    /* 0x10 */ u8 frameTableCount;   /* <<2 + 0x1C = offset to first frame entry */
    /* 0x11 */ u8 _pad11[2];
    /* 0x13 */ u8 frameEntryIndex;   /* <<2 added to frame base */
    /* 0x14 */ u8 _pad14[8];
    /* 0x1C */ u8 frameEntries[1];    /* frame-entry array (each 4 bytes); [f0]+0x1C = geom ptr */
} MobyAnimSeqDef;

typedef struct MobyAnimClass {
    /* 0x00 */ u8 _pad00[8];
    /* 0x08 */ u8 classFlag;            /* lbu -> zero-extended func_002A1F68 arg4 */
    /* 0x09 */ u8 _pad09[0x3F];
    /* 0x48 */ MobyAnimSeqDef *seqDefs[1]; /* indexed by sequence byte */
} MobyAnimClass;

typedef struct HudAnimMoby {
    /* 0x00 */ u8 _pad00[0x24];
    /* 0x24 */ MobyAnimClass *classTable;
    /* 0x28 */ u8 _pad28[0x18];
    /* 0x40 */ u8 frame0;
    /* 0x41 */ u8 frame1;
    /* 0x42 */ u8 seq0;
    /* 0x43 */ u8 seq1;
    /* 0x44 */ u8 _pad44[0x14];
    /* 0x58 */ void *framePtr0;
    /* 0x5C */ void *framePtr1;
} HudAnimMoby;

static inline void installAnimFrame(HudAnimMoby *m2, void *moby, u8 seq, u8 frame0) {
    MobyAnimClass *cls = m2->classTable;
    MobyAnimSeqDef *sd = cls->seqDefs[seq];
    void *frame0Ptr = *(void **)((u8 *)sd + (frame0 << 2) + 0x1C);
    u8 *framePtr = (u8 *)sd + ((sd->frameTableCount << 2) + 0x1C);
    framePtr += sd->frameEntryIndex << 2;
    func_002A1F68(framePtr, frame0Ptr, moby, cls->classFlag);
}

/* MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (task #1669). What carries it:
   - installAnimFrame is `static inline`; not inlined it is a real call.
   - the frame pointer is spelled (sd + ((count << 2) + 0x1C)) + (index << 2),
     the ROM's association; written as one sum cc1 regroups it.
   - the frame-0 entry is indexed `frame0 << 2`; `frame0 * 4` puts the addu
     operands the other way round.
   - REGISTER-PIN DEVICE (RULING #8598): `base` pinned to $16. Without it every
     pin-free spelling measured (eight, task #1669) puts the moby view m2 in
     $16 and base in $17, the ROM's roles swapped; all other words are equal.
   - no trailing empty asm.
   GUARD: on EE this C is the image's body, compiled alone by the s136os arm
   (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
   tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
   splice drops the function. On native it is plain C. */
void func_0034F9F8(void *mgr) {
    register u8 *base EE_REG("$16") = (u8 *)mgr;
    HudAnimMoby *m2 = (HudAnimMoby *)(base + 0xC00);
    func_0026F790(m2, 0);
    UpdateMobyAnimation(m2);
    if (m2->seq0 != 0xFF) {
        installAnimFrame(m2, base, m2->seq0, m2->frame0);
        m2->framePtr0 = base;
    }
    if (m2->seq1 != 0xFF) {
        base = base + 0x600;
        installAnimFrame(m2, base, m2->seq1, m2->frame1);
        m2->framePtr1 = base;
    }
    UpdateMobyBSphereAndGrid(m2);
}
#endif

/* func_0034FAF8 globals/callees. */
extern u8  *g_frameDmaCursor;   /* 0x1B2228 frame VIF1 chain write cursor */
extern s32  g_gsPixelOffsetX[]; /* GS screen X pixel offset (word 0) */
extern s32  g_gsPixelOffsetY[]; /* GS screen Y pixel offset (word 0) */
extern u8   D_1AC560[];         /* scratch GS packet-build buffer (screen XY at +0x20/+0x24) */
extern void CopyQwords(void *dst, const void *src, s32 nbytes);
extern void func_002A1138(void *rec, s32 flag);

/** func_0034FAF8 — emit a moby's two screen-space GS packets into the frame DMA
 *  chain. First packet is offset by the record's on-screen delta: screen XY =
 *  g_gsPixelOffset{X,Y} + rec->{+0xB4,+0xB6} (signed shorts), written into the
 *  D_1AC560 scratch packet at +0x20/+0x24, CopyQwords'd (0x30 bytes) to the DMA
 *  cursor which then advances 0x30. func_002A1138(rec, 1) fills in the moby-
 *  specific packet body, then a second packet at the plain screen origin
 *  (g_gsPixelOffset{X,Y}) is emitted and the cursor advances 0x30 again. The
 *  ctx (arg0) is the shared light/render context, unused here. */
/* ADDRESSING-MODEL DEVICE (RULING #8620, the 191238.cpp equate form). The ROM
 * reads g_frameDmaCursor and g_gsPixelOffset{X,Y} absolutely (lui/lw) but
 * stores the cursor as one %gp_rel word in the func_002A1138 delay slot
 * (0x34FB6C) and absolutely through $at at the end (0x34FBB4). Size 12 gives
 * exactly that for the cursor (asm_unit.sh's -G8 delay-slot rule) and size 16
 * absolute everywhere for the offsets. The equated names keep the size off
 * the real symbols; the relocations still name them. Top level, so the s136os
 * TU and the spliced 2.9 TU see the same lines. */
#ifndef TARGET_NATIVE
__asm__(".extern g_frameDmaCursorAbs, 12\n\tg_frameDmaCursorAbs = g_frameDmaCursor");
extern u8 *g_frameDmaCursorAbs;
__asm__(".extern g_gsPixelOffsetXAbs, 16\n\tg_gsPixelOffsetXAbs = g_gsPixelOffsetX");
extern s32 g_gsPixelOffsetXAbs;
__asm__(".extern g_gsPixelOffsetYAbs, 16\n\tg_gsPixelOffsetYAbs = g_gsPixelOffsetY");
extern s32 g_gsPixelOffsetYAbs;
#else
#define g_frameDmaCursorAbs g_frameDmaCursor
#define g_gsPixelOffsetXAbs g_gsPixelOffsetX[0]
#define g_gsPixelOffsetYAbs g_gsPixelOffsetY[0]
#endif

#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0034FAF8)
S136OS_SLOT(func_0034FAF8);
#else
/* MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (task #1669) with the addressing aliases above and the packet
   base held in a local: indexing D_1AC560 directly lets cc1 fold the +0x20
   into the %hi/%lo pair and spend three saved registers on it.
   GUARD: on EE this C is the image's body, compiled alone by the s136os arm
   (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
   tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
   splice drops the function. On native it is plain C. */
void func_0034FAF8(void *ctx, u8 *rec) {
    u8 *pkt = D_1AC560;

    (void)ctx;
    *(s32 *)(pkt + 0x20) = g_gsPixelOffsetXAbs + *(s16 *)(rec + 0xB4);
    *(s32 *)(pkt + 0x24) = g_gsPixelOffsetYAbs + *(s16 *)(rec + 0xB6);
    CopyQwords(g_frameDmaCursorAbs, pkt, 0x30);
    g_frameDmaCursorAbs += 0x30;

    func_002A1138(rec, 1);

    *(s32 *)(pkt + 0x20) = g_gsPixelOffsetXAbs;
    *(s32 *)(pkt + 0x24) = g_gsPixelOffsetYAbs;
    CopyQwords(g_frameDmaCursorAbs, pkt, 0x30);
    g_frameDmaCursorAbs += 0x30;
}
#endif

extern void AssertFail(const char *file, s32 line, const char *expr);
/* rodata assert strings (bytes verified in Ghidra, USA SCUS_972.68):
 *   D_1AE758 = "gui/mathUtil.cpp"        (source file)
 *   D_1AE770 = "t>=0.0f && t<= 1.0f"     (clamp predicate) */
extern const char D_1AE758[];
extern const char D_1AE770[];

/*
 * GuiHermiteInterp - cubic-Hermite blend of the four controls a, b, c, d by t
 * (mathUtil.cpp line 36).
 *
 *   t           blend parameter; asserted to lie in [0, 1] (AssertFail on
 *               failure, then the blend is computed anyway)
 *   a, b, c, d  the four controls
 * Returns a*(2t^3-3t^2+1) + b*(t^3-2t^2+t) + c*(t^3-t^2) + d*(3t^2-2t^3).
 *
 * Byte-exact on the sdk29 arm (task #894). The ROM saves ra and f20-f24 and
 * cc1 2.9 lays that frame out identically (0x40, f20..f24 at 0x10..0x30), so
 * the FP-save edge of NOTE #8218 is no wall here. What had to be spelled:
 *  - each coefficient product lands in its control's own register in the ROM
 *    (`mul.s $f22,$f22,$f1` for c, $f24 for b, $f23 for d), which cc1 does
 *    only when the product is assigned back to that control (b = b * ...);
 *  - the `a` term is `add.s $f21,$f2,$f21` in the ROM, product first. Written
 *    as `a = X * a + a` cc1 always emits `add.s $f21,$f21,$f2`, whatever the
 *    source operand order (3 spellings measured); assigning the sum to a block
 *    temporary first and then to `a` gives the ROM's order. The copy costs no
 *    instruction.
 *
 * WEAK (TARGET_NATIVE only): when this unit is co-linked into the eetest cmp
 * suite alongside text/235FE8, cmp_235FE8.c supplies a strong deterministic
 * GuiHermiteInterp stand-in for its func_00336A28 oracle; weak lets that
 * stand-in win the link instead of a multiple-definition error. The stand-in
 * is a NON-Hermite fake, which is harmless: its only consumer
 * (cmp_func_00336A28) jal's the same symbol on both the asm-oracle and c_
 * sides, so the fake cancels, and no cmp test checks real Hermite output.
 */
#ifdef TARGET_NATIVE
__attribute__((weak))
#endif
f32 GuiHermiteInterp(f32 t, f32 a, f32 b, f32 c, f32 d) {
    f32 t2, t3;

    if (!(t >= 0.0f && t <= 1.0f)) {
        AssertFail(D_1AE758, 0x24, D_1AE770);
    }
    t2 = t * t;
    t3 = t2 * t;
    {
        f32 blend = (2.0f * t3 - 3.0f * t2) * a + a;
        a = blend;
    }
    b = b * (t3 - 2.0f * t2 + t);
    c = c * (t3 - t2);
    d = d * (3.0f * t2 - 2.0f * t3);
    return a + b + c + d;
}

#ifdef TARGET_NATIVE
extern s32 g_swapGadgetItemIndex;    /* +0xAE = FMV aspect/letterbox scratch */
extern u8 *g_pFmvArenaBase;          /* FMV work-arena base (0x1B234C) */
extern char D_1AE7A0[];              /* FMV debug format string */
extern void DebugPrintStub(const char *fmt, ...);
extern s32 func_0011AB10(void);      /* current thread id */
extern s32 func_0011AAB0(s32 thid, s32 priority);
extern void BuildAspectBlitStrips(void *a, void *b);
extern s32 InitFmvPlaybackEngine(void *a, void *b, void *engineCtx);
extern s32 FmvStreamFeedLoop(void *dmaq, void *base, void *addq);   /* playback loop (parked) */
extern void func_003503D8(void);     /* FMV teardown */
#endif

/* ADDRESSING-MODEL DEVICES for PlayFmvMovie (RULING #8620 terms; the offset-0 gp
 * equate is ruled covered by RULING #9073; #8036 alias). They emit nothing:
 *   - `.extern ,16` on both FMV base pointers: the ROM stores them with the
 *     absolute `lui $1; sw rX,%lo(sym)($1)` macro pair, so the assembler must
 *     expand cc1's one-insn store macro that way;
 *   - the offset-0 equate alias `g_pFmvArenaBaseGp`, given no sized `.extern` of
 *     its own: it reproduces the ROM's one gp-relative read,
 *     `lw $4,%gp_rel(g_pFmvArenaBase)($28)` in the `beqz` delay slot at 0x0034FD28.
 *     The only size the alias gets is cc1's own `.extern g_pFmvArenaBaseGp, 4`
 *     (<= -G8; the s136os splice carries it in front of the block), so the
 *     assembler makes that one access gp-relative while the real symbol keeps its
 *     `, 16`. The relocation names g_pFmvArenaBase (R_MIPS_GPREL16), and GNU as
 *     never writes an equate of an undefined symbol to the symtab, so no alias
 *     reaches nm (task #1408).
 * At file scope, EE only (task #1408): the s136os splice REFUSES a member whose
 * own arm carries an `.extern ,16` (ADDRESSING) or an equate (DEFINITION) that
 * the unit's 2.9 TU never sees (FACT #9057, FACT #9067). Native reads the real
 * symbol. No other C in this unit names g_pFmvGsBase, g_pFmvArenaBase or the
 * alias. FmvStreamFeedLoop's asm spells %hi/%lo(g_pFmvArenaBase) explicitly, so
 * these directives cannot change it. */
#ifndef TARGET_NATIVE
__asm__(".extern g_pFmvGsBase, 16");
__asm__(".extern g_pFmvArenaBase, 16");
__asm__("g_pFmvArenaBaseGp = g_pFmvArenaBase");
extern u8 *g_pFmvArenaBaseGp;
#define FMV_ARENA_BASE_GP g_pFmvArenaBaseGp
#else
#define FMV_ARENA_BASE_GP g_pFmvArenaBase
#endif

/* GUARD (task #1408): on EE this C is the image's PlayFmvMovie, compiled alone by
 * the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C.
 * History: task #566 measured sdk29 91.12% / engine96 59.05% (unit objdiff
 * report). Task #1382's s136 screen read EXACT 59/59 with relocations equal. The
 * levers there: `g_pFmvGsBase` by its ROM name (it had been spelled
 * `g_swapGadgetItemIndex + 0xAE`, the same address but gp-relative). Without the
 * `.extern`s: 57/59, first diff @1. Without the gp alias: 26/59, first diff @33. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_PlayFmvMovie)
S136OS_SLOT(PlayFmvMovie);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void DebugPrintStub(const char *fmt, ...);
extern void BuildAspectBlitStrips(void *a, void *b);
extern s32 func_0011AB10(void);
extern s32 func_0011AAB0(s32 thid, s32 priority);
extern s32 InitFmvPlaybackEngine(void *a, void *b, void *engineCtx);
extern s32 FmvStreamFeedLoop(void *dmaq, void *base, void *addq);
extern void func_003503D8(void);
/* g_pFmvGsBase / g_pFmvArenaBase addressing and FMV_ARENA_BASE_GP: the
 * file-scope devices above this function's guard. */
extern s32 g_pFmvGsBase;     /* 0x1B2348 - secondary FMV base, paired with the arena base */
extern u8 *g_pFmvArenaBase;
extern char D_1AE7A0[];
/* (end of this body's declarations) */
/* MATCHED on the s136os arm (task #1408), not by cc1 2.9.
   Launch an FMV clip: record the aspect scratch (aspect) and the work-arena
   base (arena -> g_pFmvArenaBase), build the aspect blit strips, resume the FMV
   thread, and start the playback engine; if it armed, run the playback main loop
   (FmvStreamFeedLoop) over the arena's DMA-add queue (+0xD9048) and frame chain
   (+0xD9040) and keep its stop reason. Always tears down (func_003503D8) and
   clears the arena base + aspect scratch. Returns the stop reason (0 if the
   engine never armed). */
s32 PlayFmvMovie(void *a, void *b, s32 aspect, u8 *arena, void *engineCtx, void *blitCtx) {
    s32 result = 0;
    g_pFmvGsBase = aspect;
    g_pFmvArenaBase = arena;
    DebugPrintStub(D_1AE7A0, 0x38CAC0);
    BuildAspectBlitStrips(blitCtx, blitCtx);
    func_0011AAB0(func_0011AB10(), 1);
    if (InitFmvPlaybackEngine(a, b, engineCtx) != 0) {
        u8 *base = FMV_ARENA_BASE_GP;
        result = FmvStreamFeedLoop(base + 0xD9048, base, base + 0xD9040);
    }
    func_003503D8();
    g_pFmvGsBase = 0;
    g_pFmvArenaBase = 0;
    return result;
}
#endif

/* FmvStreamFeedLoop(playCtx, arg1, statsPtr): the FMV playback MAIN LOOP — reads the
 * elementary-stream length from *statsPtr, then per-vblank (WaitVblankGetField)
 * pumps the pipeline: a skip/exit decision (D_138180 pad state + g_cinematicExitPending
 * vs arg1 + D_1A7A10 + g_playerProgress + the pad's 0x800 button bit), a CD sector
 * read (func_003513F8) into the arena DMA-add queue (+0xD9048), IPU frame decode
 * (func_003526A8/func_00352780/func_00352C70 on the frame queue +0xD9168), audio
 * pump (func_00132888 mode 5 + snd_Pump), and pts-ring advance (+0xD9100). Runs
 * until the stream drains or the user skips; returns the stop reason (sp+0x8).
 *
 * PARK (doc-modeled INCLUDE_ASM, NOT #else — per the blast-radius guardrail):
 * this 880-byte real-time driver is (a) NOT cmp-oracle-able (absolute g_pFmvArenaBase
 * + the whole FMV IPU/DMA HLE callee set — the --gc-sections wall) AND (b) high
 * transcription-risk: its skip/exit decision at 0x34FE00–0x34FEBC is a subtle
 * boolean built from movz/movn + beql/bnel likely-branch delay-slot nullification
 * (0x34FE4C beql, 0x34FE6C bnel), exactly the class where a one-shot un-verified
 * #else silently mis-computes. Model it faithfully only once the FMV HLE backend
 * makes it oracle-able. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", FmvStreamFeedLoop);

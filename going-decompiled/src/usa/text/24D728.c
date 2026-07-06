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

/* func_0034D7D0: select the HUD frame sprite texture by mode (stored at
 * +0x15A4): mode 0 -> texture 0x7567, mode 1 -> 0xEAA2, else no change.
 * NEAR-MISS (89.55%): the C is structurally exact, but the original folds the
 * epilogue `ld $ra` into the case-exit branch delay slots (a cc1 epilogue-
 * scheduling artifact this toolchain won't reproduce). Kept as the portable
 * #else body; the matching build keeps the original bytes. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034D7D0);
#else
void func_0034D7D0(GuiHudManager *mgr, s32 mode) {
    *(s32 *)((u8 *)mgr + 0x15A4) = mode;
    switch (mode) {
    case 0:
        GuiSpriteSetTexture((u8 *)mgr + 0x2A0, 0x7567, 0);
        break;
    case 1:
        GuiSpriteSetTexture((u8 *)mgr + 0x2A0, 0xEAA2, 0);
        break;
    }
    __asm__ __volatile__("");
}
#endif

/* func_0034D828: (re)assign the HUD frame-sprite texture and, on the first
 * activation after the manager's row state was torn down, reset the five
 * scrollable sub-list/sprite slots. Stores the new "active" flag (arg2) at
 * +0x1584; if the +0x1588 "needs-reset" latch is set, it clears the latch,
 * resets the four list slots (+0xF3C/+0xFD0/+0x1064/+0x1180 via func_0034A7F8)
 * and the sprite slot (+0x10F8 via func_0034A370 mode 1), then retargets the
 * frame sprite (+0x158) to texture `tex`. Independently, if the +0x159C latch is
 * set, it retargets the frame sprite too. `tex` is the GuiSprite texture id.
 * WALL (94.3%): the original packs its three callee-saved GPRs (s0/s1/ra) at an
 * 8-byte stride (frame 0x20); this ee-gcc build emits a 16-byte GPR save slot
 * (frame 0x30, R5900 128-bit-register stack-slot model), an unfixable
 * frame-layout ceiling for any function saving 2+ GPRs across calls. The trailing
 * asm barrier already defeats the tail-call; only the save stride differs. Kept
 * as the portable #else; the matching build keeps the original bytes. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034D828);
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
 * The original recomputes the PAL-gated literal before each of the six
 * func_0034A3B8 calls (cc1 rematerializes the constant per call site); the value
 * is identical every iteration, so the portable #else computes it once. `flag` is
 * the new active-state value; `tex` is unused on this path (it is the shared
 * sibling signature with func_0034D828). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034D8C8);
#else
extern s32 g_bPalMode; /* PAL video-mode flag (USA build clears it -> NTSC) */
void func_0034D8C8(GuiHudManager *mgr, s32 flag) {
    u8 *m = (u8 *)mgr;
    *(s32 *)(m + 0x1594) = flag;
    if (*(s32 *)(m + 0x1598) != 0) {
        union { u32 u; f32 f; } step;
        *(s32 *)(m + 0x1598) = 0;
        GuiElementSetVisible((GuiElement *)(m + 0x5D8), 0);
        func_0034A370(m + 0xC0C, 1);
        func_0034A370(m + 0xC94, 1);
        func_0034A370(m + 0xD1C, 1);
        func_0034A370(m + 0xDA4, 1);
        func_0034A370(m + 0xE2C, 1);
        func_0034A370(m + 0xEB4, 1);
        step.u = (g_bPalMode != 0) ? 0x3E0F5C29u : 0x3DEEEEF0u;
        func_0034A3B8(m + 0xC0C, step.f);
        func_0034A3B8(m + 0xC94, step.f);
        func_0034A3B8(m + 0xD1C, step.f);
        func_0034A3B8(m + 0xDA4, step.f);
        func_0034A3B8(m + 0xE2C, step.f);
        func_0034A3B8(m + 0xEB4, step.f);
    }
}
#endif

/* func_0034DAB0: drives the HUD sub-element at +0x5D8 - pad-gated "snap" tween,
 * a color-ramp tween, then writes the tween handle/alpha and sets visibility.
 * Left as bare INCLUDE_ASM: a faithful cmp-oracle is blocked because its tween
 * callee func_002AA3F0 (1A8180.c, linked whole into the cmp suite) runs a real
 * VU0 LerpByteVec4PackedVu0 + absolute D_1A9E94/98 globals (the absolute-symbol
 * ld --gc-sections wall), and it cannot be mocked instead without colliding with
 * that linked real body. Not seedable cleanly; revisit when func_002AA3F0 itself
 * is oracled. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034DAB0);

/* func_0034DB68: sibling of func_0034D828 for the manager's OTHER scrollable
 * region - store the "active" flag (arg) at +0x158C, and if the +0x1590
 * needs-reset latch is set, clear it and reset that region's four sub-list slots
 * (+0x1214/+0x12A8/+0x133C/+0x13D0 via func_0034A7F8, flag 0). `flag` is the new
 * active state. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034DB68);
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
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034DBC8);

/* func_0034DBD8: store the texture/frame handle into the manager at +0x2DC. */
void func_0034DBD8(GuiHudManager *mgr, s32 value) {
    *(s32 *)((u8 *)mgr + 0x2DC) = value;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034DBE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034E8D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034ED20);

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
 * 235FE8 definitions. Revisit when the GUI element vtable globals are seeded. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", GuiManagerInitListRows);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", GuiManagerInitHudLists);

/* func_0034EF60: empty/no-op leaf (original compiles to jr ra; nop). */
void func_0034EF60(void) {
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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034EF68);
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F028);

/* func_0034F1C0: per-frame tick of the outer GuiInstance's HUD widgets - run the
 * embedded GuiHudManager's frame update (+0x7A0 via func_0034DBE0), forward the
 * HUD list (func_0034F200), tick the third (planet-name) list (+0x1FDC via
 * func_00339A88), then update the on-screen bolt counter (TickBoltCounterHud).
 * NEAR-MISS candidate: kept INCLUDE_ASM for the matching build; the #else is the
 * portable equivalent (sibling-call chain over the outer GuiInstance). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F1C0);
#else
void func_0034F1C0(GuiInstance *mgr) {
    u8 *m = (u8 *)mgr;
    func_0034DBE0(m + 0x7A0);
    func_0034F200(mgr);
    func_00339A88(m + 0x1FDC);
    TickBoltCounterHud();
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
 * then restores the saved projection param and rebuilds once more. The matching
 * build keeps the asm (engine save-layout wall). */
#ifdef TARGET_NATIVE
extern void *g_guiInstance;
extern void BuildCameraProjection(void);
extern void func_0029D9B8(void);
extern void func_0034F028(void *obj, s32 a, s32 b, s32 c);
extern void func_0034E8D8(void *p);
extern u8   g_sceneActorMobys[];      /* 0x1B894C  scene cast moby ptrs (byte base) */
extern s32  D_1A8C64;                 /* HUD-refresh suppress flag */
extern s32  D_1AE6F4;                 /* gp-global passed to func_0034F028 */
extern s32  D_1AE6F8;                 /* gp-global passed to func_0034F028 */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F240);
#else
void func_0034F240(void *guiArg) {
    u8  *gui = (u8 *)guiArg;
    u8  *cam = g_sceneActorMobys + 0x674;   /* scene camera params */
    f32  saved = *(f32 *)(cam + 0xB0);

    *(f32 *)(cam + 0xB0) = 0.62f;           /* 0x3F1EB852 */
    BuildCameraProjection();

    if (D_1A8C64 == 0) {
        s32 packed = func_0034D7A8((GuiHudManager *)(gui + 0x7A0));
        func_0034F028(gui, D_1AE6F4, D_1AE6F8, (s32)((u32)packed >> 24));
    }

    func_0034EF60();
    func_0034E8D8(gui + 0x7A0);

    {
        u8 *mgr = (u8 *)g_guiInstance + 0x38000;
        *(s32 *)(mgr + 0x79E0) = 0;
        func_00339F98(gui + 0x1FDC);
        *(s32 *)(mgr + 0x79DC) = 0;
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
extern void  GuiManagerInitListRows(void *listRows);
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

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", GuiSystemInit);
#else
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

/* func_0034F868 callees (declared for the TARGET_NATIVE #else only). */
extern void WaitFrameDmaFence(s32 mask);
extern void WaitGsPathsIdle(s32 a, s32 b);
extern void func_0011AEA0(s32 arg);                       /* pre-RPC flush/sync */
extern void KickGifImageUpload(void *packet, void *vramDest);
extern void func_126288(void *dst, s32 texId, s32 a, s32 b, s32 c,
                        s32 d, s32 width, s32 height);    /* build a GS image-upload GIF packet */

/** func_0034F868 — flush the queued texture image-uploads for this manager. Wait
 *  one frame-DMA fence, then for each of the base->+0x2E20 queued entries build a
 *  16x16 GS image-upload GIF packet (func_126288) for the entry's texture id (the
 *  low halfword of the stride-4 id list at base+0x2E00) into a stack scratch,
 *  issue it to the entry's VRAM slot (base+0xE00, stride 0x400) via
 *  KickGifImageUpload, and wait for the GS paths to idle between uploads. Clears
 *  the queue count when done. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F868);
#else
void func_0034F868(u8 *base) {
    u8  packet[0x60];
    u8 *idCursor;
    u8 *vramDest;
    s32 i;

    WaitFrameDmaFence(1);
    if (*(s32 *)(base + 0x2E20) <= 0) {
        return;
    }
    idCursor = base + 0x2E00;
    vramDest = base + 0xE00;
    i = 0;
    do {
        func_126288(packet, *(s16 *)idCursor, 1, 0, 0, 0, 0x10, 0x10);
        i++;
        func_0011AEA0(0);
        idCursor += 4;
        KickGifImageUpload(packet, vramDest);
        vramDest += 0x400;
        WaitGsPathsIdle(0, 0);
    } while (i < *(s32 *)(base + 0x2E20));
    *(s32 *)(base + 0x2E20) = 0;
}
#endif

/* func_0034F928 globals/callee (declared for the TARGET_NATIVE #else only). */
extern u8   g_dirLightMatrices[];    /* 0x1C26C0, 0x40-stride directional-light matrices */
extern void func_00283638(void *dst);/* zero a 16-byte quadword at dst */

/** func_0034F928 — install the fixed directional light in matrix slot 14
 *  (g_dirLightMatrices + 0x380): RGB colour (0.4, 0.8, 1.2) at +0x380/+0x384/
 *  +0x388, a normalised diagonal direction (~0.577, ~0.577, ~-0.577) at +0x390/
 *  +0x394/+0x398 with zero w-components at +0x38C/+0x39C, then clear the two
 *  16-byte matrix-blend blocks at +0x3A0 and +0x3B0. Called with the light-setup
 *  context (its sibling F9B8/F9F8/FAF8 use it), but this one writes only the
 *  global directional light, so ctx is unused here. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F928);
#else
void func_0034F928(void *ctx) {
    union { u32 u; f32 f; } dir;
    (void)ctx;

    *(f32 *)(g_dirLightMatrices + 0x380) = 0.4f; /* 0x3ECCCCCD */
    *(f32 *)(g_dirLightMatrices + 0x384) = 0.8f; /* 0x3F4CCCCD */
    *(f32 *)(g_dirLightMatrices + 0x388) = 1.2f; /* 0x3F99999A */
    *(s32 *)(g_dirLightMatrices + 0x38C) = 0;
    dir.u = 0x3F13B646u; /* ~0.577 direction component */
    *(f32 *)(g_dirLightMatrices + 0x390) = dir.f;
    *(f32 *)(g_dirLightMatrices + 0x394) = dir.f;
    dir.u = 0xBF13B646u; /* ~-0.577 */
    *(f32 *)(g_dirLightMatrices + 0x398) = dir.f;
    *(s32 *)(g_dirLightMatrices + 0x39C) = 0;
    func_00283638(g_dirLightMatrices + 0x3A0);
    func_00283638(g_dirLightMatrices + 0x3B0);
}
#endif

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
 *   frame0Ptr = *(seqDef + frame0*4 + 0x1C); classFlag = cls[0x8].
 * Kept INCLUDE_ASM for the matching build; #else is the portable equivalent. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F9F8);
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

static void installAnimFrame(HudAnimMoby *m2, void *moby, u8 seq, u8 frame0) {
    MobyAnimClass *cls = m2->classTable;
    MobyAnimSeqDef *sd = cls->seqDefs[seq];
    u8 *framePtr = (u8 *)sd + (sd->frameTableCount << 2) + 0x1C
                 + (sd->frameEntryIndex << 2);
    void *frame0Ptr = *(void **)((u8 *)sd + frame0 * 4 + 0x1C);
    func_002A1F68(framePtr, frame0Ptr, moby, cls->classFlag);
}

void func_0034F9F8(void *mgr) {
    u8 *base = (u8 *)mgr;
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

/* func_0034FAF8 globals/callees (declared for the TARGET_NATIVE #else only). */
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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034FAF8);
#else
void func_0034FAF8(void *ctx, u8 *rec) {
    (void)ctx;
    *(s32 *)(D_1AC560 + 0x20) = g_gsPixelOffsetX[0] + *(s16 *)(rec + 0xB4);
    *(s32 *)(D_1AC560 + 0x24) = g_gsPixelOffsetY[0] + *(s16 *)(rec + 0xB6);
    CopyQwords(g_frameDmaCursor, D_1AC560, 0x30);
    g_frameDmaCursor += 0x30;

    func_002A1138(rec, 1);

    *(s32 *)(D_1AC560 + 0x20) = g_gsPixelOffsetX[0];
    *(s32 *)(D_1AC560 + 0x24) = g_gsPixelOffsetY[0];
    CopyQwords(g_frameDmaCursor, D_1AC560, 0x30);
    g_frameDmaCursor += 0x30;
}
#endif

/* GuiHermiteInterp: cubic-Hermite blend of the four controls a,b,c,d by t
 * (clamped to [0,1] with an assert) - mathUtil.cpp line 36. Returns
 *   a*(2t^3-3t^2+1) + b*(t^3-2t^2+t) + c*(t^3-t^2) + d*(3t^2-2t^3).
 * The matching build keeps the original (FPU instruction scheduling differs);
 * the #else is the portable equivalent. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", GuiHermiteInterp);
#else
extern void AssertFail(const char *file, s32 line, const char *expr);
/* rodata assert strings (bytes verified in Ghidra, USA SCUS_972.68):
 *   D_1AE758 = "gui/mathUtil.cpp"        (source file)
 *   D_1AE770 = "t>=0.0f && t<= 1.0f"     (clamp predicate) */
extern const char D_1AE758[];
extern const char D_1AE770[];
/* WEAK: when this unit is co-linked into the eetest cmp suite alongside
 * text/235FE8 (whose cmp_235FE8.c supplies a strong deterministic GuiHermiteInterp
 * stand-in for its func_00336A28 oracle), both objects would otherwise define
 * the cross-unit symbol GuiHermiteInterp -> multiple-definition link error. The
 * weak attribute (TARGET_NATIVE arm only) lets that test stand-in win the link.
 * NOTE: cmp_235FE8.c's stand-in is a DETERMINISTIC NON-Hermite fake, NOT
 * equivalent to this real cubic-Hermite body - but that is harmless: its only
 * consumer (cmp_func_00336A28) jal's the same GuiHermiteInterp symbol on BOTH the
 * asm-oracle and c_ sides, so the fake cancels; no cmp test checks real Hermite
 * output, and no function under test calls it (gc-sections drops this weak body).
 * In the matching build this arm is the INCLUDE_ASM original, so weak is inert. */
__attribute__((weak))
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

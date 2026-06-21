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

/* List/sprite sub-widget reset helpers (text/248B50, addr 0x34A7F8/0x34A370):
 * clear an animation slot of a sub-list / sprite by flag. Forwarders only -
 * their semantics are not under test here. */
extern void func_0034A7F8(void *list, s32 flag);
extern void func_0034A370(void *sprite, s32 mode);

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
 * set, it retargets the frame sprite too. `tex` is the GuiSprite texture id. */
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034D8C8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F1C0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/24D728", func_0034F240);

/* func_0034F300: identity - return the manager pointer unchanged (a vtable
 * "get self" accessor). Typed as GuiInstance * by association with the adjacent
 * func_0034F200/F220 outer-object forwarders; the body is pure `return mgr`, so
 * the choice of pointee is byte-neutral (UNCONFIRMED which object). */
GuiInstance *func_0034F300(GuiInstance *mgr) {
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

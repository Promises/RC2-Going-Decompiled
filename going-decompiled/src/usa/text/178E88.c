#include "common.h"

/* Blob-shadow subsystem state at 0x1B15BC. The queue count is at +0, the
 * pending screen-grab query count at +0x1C, and the enable flag at +0x2C.
 * Declared as an incomplete array so accesses stay absolute %hi/%lo under -G8
 * (the original never reaches this cluster gp-relative). */
extern s32 g_blobShadowCount[];
#define g_blobShadowEnabled (g_blobShadowCount[0xB])
#define g_screenGrabQueryCount (g_blobShadowCount[7])

/* Render-layer enable bitmask (gp-relative under -G8). */
extern s32 g_renderLayerMask;

/* Draw-env / frame helpers (defined elsewhere in the render unit). */
extern void AppendDrawEnvContext1(void);
extern void AppendScreenClearPacket(s32 mode);
extern void RenderFrame(void);
extern void RenderMenuScreenWidgets(s32 which);
extern void func_0027B988(void);
extern void func_002832F8(void *arg);

/* Per-camera-slot flag table base (0x1B7E30). func_00279EE8 passes the slot at
 * +0xF8 (0x1B7F28); modelled as a byte base so the offset stays absolute.
 * func_00279CF0 writes a table of 0x28-byte records starting at +0x458
 * (0x1B8288). */
extern u8 g_cameraSlotActive[];
typedef struct { s32 f[10]; } CamSlotRecord; /* 0x28-byte slot descriptor */

/* Per-glyph width lookup over a fixed-stride font table. Each table entry is
 * 4 bytes; the signed byte at +3 is the glyph advance width. func_0027F7A8
 * sums integer advances; func_0027F858 sums scaled (float) advances. */
extern s32 func_0027F7A8(const char *str, s32 maxChars, const void *glyphTable);
extern s32 func_0027F858(const char *str, s32 maxChars, const void *glyphTable,
                         f32 scale);

/* Font glyph-advance tables (large; declared as incomplete arrays so they stay
 * absolute %hi/%lo under -G8). */
extern u8 g_debugFontGlyphTable[];
extern u8 D_263B10[];
extern u8 D_264250[];

/* Word-fill of `len` bytes at `dst` with a 32-bit `pattern` (SDK helper). */
extern void FillMemory32(void *dst, u32 pattern, s32 len);

/* Draw-hook callback queues. Each subsystem keeps three parallel globals: a
 * count, a function-pointer table, and an argument table (registered by the
 * Add* helpers, run by the Run* drivers). Declared as incomplete arrays so
 * their accesses stay absolute %hi/%lo (not gp-small) under -G8. */
typedef void (*DrawHookFn)(void *arg);
extern s32 g_fxHooksPreCount[];
extern DrawHookFn g_fxHooksPreFuncs[];
extern void *g_fxHooksPreArgs[];
extern s32 g_fxHooksPostCount[];
extern DrawHookFn g_fxHooksPostFuncs[];
extern void *g_fxHooksPostArgs[];
extern s32 g_fxHooksLateCount[];
extern DrawHookFn g_fxHooksLateFuncs[];
extern void *g_fxHooksLateArgs[];
extern s32 g_drawHooksAfterTiesCount[];
extern DrawHookFn g_drawHooksAfterTiesFuncs[];
extern void *g_drawHooksAfterTiesArgs[];
extern s32 g_drawHooksAfterShrubsCount[];

/* Misc per-frame render state cleared by ResetPerFrameDrawQueues. The absolute
 * globals are incomplete arrays; the four sdata one-shots (D_1A86F4/8760/8770
 * and g_bWaterPoolActive) stay gp-small scalars. g_screenFadeWhite is the
 * white-flash level; its +4/+8 words are per-frame request slots. */
extern s32 g_screenFadeWhite[];
extern s32 g_occlusionOverrideMode[];
extern s32 g_pWaterWaveGrids[];
extern s32 g_bWaterWavesActive[];
extern s32 D_1A86F4;
extern s32 D_1A8760;
extern s32 D_1A8770;
extern s32 g_bWaterPoolActive;

/* Per-frame occlusion visibility state. g_occlusionMode: 0 = no data (all
 * visible), 2 = resolve from the level occlusion grid. g_occlusionVisMask is a
 * 0x80-byte (1024-bit) vis-group mask consumed by the tfrag/tie/moby culls. */
extern s32 g_occlusionMode[];
extern u8 g_occlusionVisMask[];
extern void ResolveOcclusionVisMask(void);

/* Pause/menu render-mode query (1 = world behind pause, 0 = menu widgets). */
extern s32 func_00286200(void);
extern void RenderSaveLoadStatusPopup(void);

/* Wrapped text-box renderer (the scaled core behind DrawTextBoxDefault). */
extern void func_00280550(s32 a, s32 b, s32 c, s32 d, s32 e, f32 scale);

/* GS depth-range / register-packet helpers. AppendGsRegPacket takes a 64-bit
 * value (so the constants need the ori/dsll/ori zero-extend) plus a reg id. */
extern void func_002859E0(s32 zNearBits, s32 lo, s32 zScale, s32 enable);
extern void func_00285D48(s32 a, s32 b);
extern void AppendGsRegPacket(s32 regId, u64 value);

/* Frame VIF1/GS DMA chain write cursor (0x1B2228). Declared as an incomplete
 * array of one pointer so each access reloads it absolute (%hi/%lo), matching
 * the original's no-load-PRE codegen under -fno-gcse. */
extern u32 *g_frameDmaCursor[];
/* Prebuilt full-screen alpha-tint quad packet (referenced by address). */
extern u8 g_fullScreenTintPacket[];

/* GS window pixel offsets (absolute %hi/%lo). */
extern s32 g_gsPixelOffsetX[];
extern s32 g_gsPixelOffsetY[];

/* Active display geometry + the GS screen context the viewport is derived from
 * (func_0027B988). g_screenHeight is the base of {height, halfW, halfH}. */
extern u8  g_gsScreenContext[]; /* 0x1A6480 - dims at +0x150 (w) / +0x152 (h) */
extern s32 g_screenWidth[];     /* 0x1A7340 */
extern s32 g_screenHeight[];    /* 0x1A7344 */
extern f32 IntToFloat(s32 x);
extern void func_0027A550(void);
/* Append a 4-vertex flat (untextured-coord) sprite quad packet given a pointer
 * to four packed XYZ2 corner words and a TEX0 value. */
extern void func_0027F0A8(const u64 *corners, u64 tex0);

/* Scene-cast moby pointer table base (0x1B894C). The screen-grab/occlusion
 * query descriptor table lives at +0x44 (0x1B8990); modelled as a byte base so
 * the +0x44 offset compiles to the absolute %hi/%lo form. */
extern u8 g_sceneActorMobys[];

/* Scene-transition teardown/fade-out targets + callees. */
extern u8   g_memoryArenaTable[]; /* 0x1BAE40 memory-region table */
extern s32  g_sceneArenaCursor;   /* 0x1B2230 */
extern s32  D_1A8BC0;             /* 0x1A8BC0 scene-arena reserve */
extern s32  g_vramDynamicBase;    /* 0x1A72D4 VRAM dynamic region base */
extern f32  g_screenFadeBlack;    /* 0x1B1520 black-fade level 0..1 */
extern void StopAllSoundEmitters(void);     /* 0x2E6E18 */
extern void WaitFrameDmaFence(s32);
extern u32  WaitVblankGetField(s32);
extern void ResetFrameArenas(void);         /* text/1FCF48 */
extern void FadeOutToBlackBlocking(s32);    /* defined below as INCLUDE_ASM */
extern void func_002FCFC8(void);            /* text/1FCF48 SelectSceneArenaRegion */

/* SceneTransitionTeardownA: fence + vblank wait, bump the frame counter, set up
 * the scene-transition block at g_cameraSlotActive+0xD0 (arena halves +0x60000,
 * the work span +0xD0800, the old scene cursor -0x60000), reserve 0x2000
 * (D_1A8BC0), reset the scene cursor to 0x60000 + the frame arenas, stop all
 * sound emitters, stash g_vramDynamicBase, and force the black fade fully on. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00278EC0);
#else
void func_00278EC0(void) {
    u8 *cam = g_cameraSlotActive + 0xD0;
    s32 half0;
    WaitFrameDmaFence(1);
    WaitVblankGetField(0);
    half0 = *(s32 *)(g_memoryArenaTable + 0xC);
    (&g_renderLayerMask)[1] += 1;                         /* frame counter at +0x4 */
    *(s32 *)(cam + 0x14) = g_sceneArenaCursor - 0x60000;
    *(s32 *)(cam + 0x00) = half0 + 0x60000;
    *(s32 *)(cam + 0x04) = *(s32 *)(g_memoryArenaTable + 0x10) + 0x60000;
    *(s32 *)(cam + 0x08) = half0 + 0x60000 + 0xD0800;
    D_1A8BC0 = 0x2000;
    g_sceneArenaCursor = 0x60000;
    ResetFrameArenas();
    StopAllSoundEmitters();
    *(s16 *)(cam + 0x26) = 0;
    *(s32 *)(cam + 0x18) = g_vramDynamicBase;
    g_screenFadeBlack = 1.0f;
}
#endif

/* SceneTransitionFadeOut: fence wait, reselect the scene-arena region + reset
 * the frame arenas, save the camera-slot VRAM dynamic base into g_vramDynamicBase,
 * then run the 30-frame blocking fade to black. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00278F90);
#else
void func_00278F90(void) {
    WaitFrameDmaFence(1);
    func_002FCFC8();
    ResetFrameArenas();
    g_vramDynamicBase = *(s32 *)(g_cameraSlotActive + 0xE8);
    FadeOutToBlackBlocking(0x1E);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00278FD0);

/* func_00279CF0 - store a 0x28-byte (10-word) record into the camera-slot
 * record table at g_cameraSlotActive+0x458, indexed by `slot` (ignored when
 * >= 0x20). The ten fields arrive as the first eight register args plus two
 * stack args; `slot` is the trailing stack arg.
 * Near-miss: the original accesses the ten fields as ten separate parallel
 * globals (stride 0x28), which makes cc1 spread the record base across several
 * aliased registers and store through them in a rotating pattern; the single
 * struct-array access here keeps one base register. Correct C preserved as the
 * portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279CF0);
#else
void func_00279CF0(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g, s32 h,
                   s32 i, s32 j, u32 slot) {
    CamSlotRecord *recs = (CamSlotRecord *)(g_cameraSlotActive + 0x458);
    if (slot < 0x20) {
        recs[slot].f[0] = a;
        recs[slot].f[1] = b;
        recs[slot].f[2] = c;
        recs[slot].f[3] = d;
        recs[slot].f[4] = e;
        recs[slot].f[5] = f;
        recs[slot].f[6] = g;
        recs[slot].f[7] = h;
        recs[slot].f[8] = i;
        recs[slot].f[9] = j;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279D68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279D88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279E00);

/* func_00279EE8 - forward the camera slot at g_cameraSlotActive+0xF8 to
 * func_002832F8.
 * Near-miss: the pinned cc1 sibling-call-optimizes the lone tail call to
 * `j func_002832F8`, but the original keeps a full call+return frame. Correct C
 * preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279EE8);
#else
void func_00279EE8(void) {
    func_002832F8(g_cameraSlotActive + 0xF8);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279F08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A0C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A130);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A138);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", BuildFrameViewMatrices);

/* Camera fog/particle setup driven by the underwater state. */
extern s32 g_bCameraUnderwater;   /* 0x1B5580 */
extern s32 g_particleFarFadeMax;  /* 0x1B1D20 far-fade clamp */
extern u8  D_1AD564[];            /* 0x1AD564 underwater fog params {b,b,b,_, f,f,f,f} */

/* func_0027A550: load the camera fog block (3 colour bytes + 4 floats) into the
 * camera/projection scratch (g_sceneActorMobys+0x674 = D_1B8FC0, +0x218..+0x238)
 * and set the particle far-fade clamp - from the underwater params (D_1AD564) +
 * fade 0x40000 when g_bCameraUnderwater, else the normal block (g_blobShadowCount
 * +0x4) + fade 0x1F4000 - then rebuild the camera projection and clear the
 * screen-grab pending word (g_blobShadowCount+0x18). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A550);
#else
void func_0027A550(void) {
    u8 *vp = g_sceneActorMobys + 0x674;
    u8 *f;
    if (g_bCameraUnderwater != 0) {
        f = D_1AD564;
        g_particleFarFadeMax = 0x40000;
    } else {
        f = (u8 *)g_blobShadowCount + 0x4;
        g_particleFarFadeMax = 0x1F4000;
    }
    *(s32 *)(vp + 0x230) = f[0];
    *(f32 *)(vp + 0x22C) = *(f32 *)(f + 0x10);
    *(s32 *)(vp + 0x234) = f[1];
    *(s32 *)(vp + 0x238) = f[2];
    *(f32 *)(vp + 0x218) = *(f32 *)(f + 0x4);
    *(f32 *)(vp + 0x21C) = *(f32 *)(f + 0x8);
    *(f32 *)(vp + 0x228) = *(f32 *)(f + 0xC);
    BuildCameraProjection();
    *(s32 *)((u8 *)g_blobShadowCount + 0x18) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", LookupOcclusionGridCell);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", LookupNeighborOcclusionCell);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", ResolveOcclusionVisMask);

/* RenderFrame pre-layer pass that produces the per-frame visibility bitmask:
 * mode 0 sets every bit (no occlusion data), mode 2 resolves it from the level
 * occlusion grid, any other mode leaves the previous mask.
 * Near-miss: the pinned cc1 sibling-call-optimizes both tail calls
 * (`j FillMemory32` / `j ResolveOcclusionVisMask`) instead of keeping the
 * original's call+return frame. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", UpdateOcclusionVisMask);
#else
void UpdateOcclusionVisMask(void) {
    if (g_occlusionMode[0] == 0) {
        FillMemory32(g_occlusionVisMask, -1, 0x80);
    } else if (g_occlusionMode[0] == 2) {
        ResolveOcclusionVisMask();
    }
}
#endif

/* InitScreenGeometry: one-time screen-geometry + projection-viewport init from
 * the GS screen context pixel dims (g_gsScreenContext +0x150 width / +0x152
 * height). Sibling of func_0027B988 (same idioms): publishes width/height + the
 * two half-extents to g_screenWidth / g_screenHeight[0..2], the four 12.4
 * fixed-point GS-window offsets (centred on 0x800) to g_gsPixelOffsetX[0] /
 * g_gsPixelOffsetY[0..2], and the float viewport scale + half-extents into the
 * camera/projection scratch (g_sceneActorMobys+0x674 = D_1B8FC0). Unlike
 * func_0027B988 it also seeds the fixed projection constants (+0xA0 32, +0xA4
 * 2^20, +0x1DC 2^19, +0x1E8 255) and does NOT call func_0027A550. The matching
 * build keeps the asm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", InitScreenGeometry);
#else
void InitScreenGeometry(void) {
    u8 *vp = g_sceneActorMobys + 0x674; /* D_1B8FC0 camera/projection scratch */
    s32 w16 = *(s16 *)(g_gsScreenContext + 0x150);
    s32 h16 = *(s16 *)(g_gsScreenContext + 0x152);
    s32 halfW = w16 >> 1;
    s32 halfH = h16 >> 1;
    f32 wHalf, hHalf;

    g_screenWidth[0] = w16;
    g_screenHeight[0] = h16;
    g_screenHeight[1] = halfW;   /* g_screenCenterDefaultX 0x1A7348 */
    g_screenHeight[2] = halfH;   /* g_screenCenterDefaultY 0x1A734C */
    g_gsPixelOffsetX[0] = (0x800 - halfW) << 4;
    g_gsPixelOffsetY[0] = (0x800 - halfH) << 4;
    g_gsPixelOffsetY[1] = (halfW + 0x800) << 4;   /* scissor max X 0x1A7358 */
    g_gsPixelOffsetY[2] = (halfH + 0x800) << 4;   /* scissor max Y 0x1A735C */

    wHalf = IntToFloat(w16) * 0.5f;
    hHalf = IntToFloat(h16) * 0.5f;

    *(f32 *)(vp + 0xA0)  = 32.0f;        /* 0x42000000 */
    *(f32 *)(vp + 0xA4)  = 1048576.0f;   /* 0x49800000 */
    *(f32 *)(vp + 0xB0)  = 0.62f;        /* 0x3F1EB852 - g_cameraProjScale init */
    *(f32 *)(vp + 0x200) = wHalf;
    *(f32 *)(vp + 0x204) = hHalf;
    *(f32 *)(vp + 0x208) = wHalf * 4.0f;
    *(f32 *)(vp + 0x20C) = hHalf * 4.0f;
    *(s32 *)(vp + 0x218) = 0;
    *(f32 *)(vp + 0x21C) = 524288.0f;    /* 0x49000000 */
    *(f32 *)(vp + 0x228) = 255.0f;       /* 0x437F0000 */
    *(s32 *)(vp + 0x22C) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", BuildCameraProjection);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027B858);

/* RecomputeScreenViewportFromGsContext: derive the active display geometry and
 * the 2D draw viewport from the GS screen context's pixel dimensions
 * (g_gsScreenContext +0x150 width / +0x152 height). Publishes width/height and
 * the half-extents to g_screenWidth / g_screenHeight[0..2], the four GS-window
 * pixel offsets (12.4 fixed-point, centred on 0x800) to g_gsPixelOffsetX /
 * g_gsPixelOffsetY[0..2], and the float viewport scale/half-extents into the
 * camera/projection scratch (g_sceneActorMobys+0x674 == D_1B8FC0: +0xB0 aspect
 * 0.62, +0x200/+0x204 half-extents, +0x208/+0x20C ×4), then runs func_0027A550.
 * The matching build keeps the asm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027B988);
#else
void func_0027B988(void) {
    u8 *vp = g_sceneActorMobys + 0x674; /* D_1B8FC0 camera/projection scratch */
    s32 w16 = *(s16 *)(g_gsScreenContext + 0x150);
    s32 h16 = *(s16 *)(g_gsScreenContext + 0x152);
    s32 halfW = w16 >> 1;
    s32 halfH = h16 >> 1;
    f32 wHalf, hHalf;

    g_screenHeight[0] = h16;
    g_gsPixelOffsetX[0] = (0x800 - halfW) << 4;
    g_gsPixelOffsetY[0] = (0x800 - halfH) << 4;
    g_gsPixelOffsetY[1] = (halfW + 0x800) << 4;
    g_gsPixelOffsetY[2] = (halfH + 0x800) << 4;
    *(f32 *)(vp + 0xB0) = 0.62f;
    g_screenWidth[0] = w16;
    g_screenHeight[1] = halfW;
    g_screenHeight[2] = halfH;

    wHalf = IntToFloat(w16) * 0.5f;
    *(f32 *)(vp + 0x200) = wHalf;
    hHalf = IntToFloat(h16) * 0.5f;
    *(f32 *)(vp + 0x204) = hHalf;
    *(f32 *)(vp + 0x20C) = hHalf * 4.0f;
    *(f32 *)(vp + 0x208) = wHalf * 4.0f;
    func_0027A550();
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", SetupGsDisplayBuffers);

/* Zero every per-frame draw-callback queue and one-shot fx slot: the fx
 * pre/post/late hook counts, the after-ties/after-shrubs draw-hook counts, the
 * blob-shadow count, the two screen-fade request words, three render-frame
 * one-shot slots, the occlusion override mode, the water wave/pool state and
 * its flags. Called at the top of each frame.
 * Near-miss: the original zeroes each absolute global via the assembler
 * `sw $0, sym` ($at-macro) form, re-materialising %hi per store; the pinned cc1
 * instead allocates a pool of GP registers and hoists/reorders the %hi
 * computations. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", ResetPerFrameDrawQueues);
#else
void ResetPerFrameDrawQueues(void) {
    g_fxHooksPreCount[0] = 0;
    g_drawHooksAfterTiesCount[0] = 0;
    g_drawHooksAfterShrubsCount[0] = 0;
    g_fxHooksPostCount[0] = 0;
    g_fxHooksLateCount[0] = 0;
    g_blobShadowCount[0] = 0;
    g_screenFadeWhite[1] = 0;
    g_screenFadeWhite[2] = 0;
    D_1A86F4 = 0;
    D_1A8760 = 0;
    D_1A8770 = 0;
    g_occlusionOverrideMode[0] = 0;
    g_pWaterWaveGrids[0] = 0;
    g_pWaterWaveGrids[1] = 0;
    g_bWaterWavesActive[0] = 0;
    g_bWaterPoolActive = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AppendFrameInitGsState);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027BFA8);

/* SetupViewModelDepthRange - set a near/compressed GS depth range for the
 * held-item view-model pass so the weapon never clips into world geometry, then
 * restore the default ALPHA blend. The depth scale clamps the (zNear+lo) shift
 * amount to 0x10.
 * Near-miss: the pinned cc1 sibling-call-optimizes the trailing
 * AppendGsRegPacket(...) to `j AppendGsRegPacket` instead of the original's
 * call+return frame, and builds the 0x8000000044 constant via dsll32 rather
 * than the original's `ori 0x8000; dsll 24`. Correct C preserved as the
 * portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027C020);
#else
void func_0027C020(s32 zNearBits, s32 lo) {
    s32 shift = zNearBits + lo;
    if (shift > 0x10) {
        shift = 0x10;
    }
    func_002859E0(zNearBits, lo, ((0x3FF000 - (4 << shift)) >> 13) << 13, 1);
    AppendGsRegPacket(0x47, 0x30000);
    AppendGsRegPacket(0x42, 0x8000000044);
    func_00285D48(0x100, 0x100);
    AppendGsRegPacket(0x42, 0x8000000044);
}
#endif

/* Thin wrapper: queue the ctx1 (FRAME_1 / FB A) draw-env REF packet.
 * Near-miss: the pinned cc1 sibling-call-optimizes the lone tail call to
 * `j AppendDrawEnvContext1`, but the original keeps a full call+return frame
 * (no sibcall). Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027C0A8);
#else
void func_0027C0A8(void) {
    AppendDrawEnvContext1();
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027C0C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RenderFrame);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CAD8);

/* Render a frame with every layer except the HUD: clear the screen, set the
 * layer mask to 0x7F (all engine layers, HUD bits clear), then render.
 * Near-miss: cc1 sibling-call-optimizes the final RenderFrame() to
 * `j RenderFrame`, whereas the original keeps the call+return frame. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawFrameWithoutHud);
#else
void DrawFrameWithoutHud(void) {
    AppendScreenClearPacket(0);
    g_renderLayerMask = 0x7F;
    RenderFrame();
}
#endif

/* RenderPauseMenuOverlay - when the render-mode query returns 1, clear the
 * screen, force the layer mask to 0x100FF (full world + HUD) and re-render the
 * world behind the pause menu; when it returns 0, draw menu-screen widgets for
 * screen 1. Always finishes with the save/load status popup.
 * Near-miss: the pinned cc1 sibling-call-optimizes the trailing
 * RenderSaveLoadStatusPopup() to `j RenderSaveLoadStatusPopup` and reschedules
 * the $ra restore into the earlier branch delay slots; the original keeps the
 * call+return frame. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CB08);
#else
void func_0027CB08(void) {
    if (func_00286200() == 1) {
        AppendScreenClearPacket(0);
        g_renderLayerMask = 0x100FF;
        RenderFrame();
    } else if (func_00286200() == 0) {
        RenderMenuScreenWidgets(1);
    }
    RenderSaveLoadStatusPopup();
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CB70);

/* Render the front-end menu widgets for screen 0.
 * Near-miss: same lone-tail-call sibcall wall as func_0027C0A8 (cc1 emits
 * `j RenderMenuScreenWidgets`; the original keeps the call+return frame). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CB80);
#else
void func_0027CB80(void) {
    RenderMenuScreenWidgets(0);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", Begin2dDrawBatch);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", End2dDrawBatch);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CDC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", GetUiTextureTex0);

/* Register a (func,arg) callback in the pre-particle fx draw queue (cap 0x40),
 * run by RunFxDrawHooksPreParticles. No-op when the queue is full.
 * Near-miss: the original loads the count's %hi into the value register and
 * re-materialises it ($at-macro store) for the final count update; the pinned
 * cc1 keeps the %hi in a second register across the body and reuses it (a
 * register-allocation choice -fno-gcse does not suppress). Correct C preserved
 * as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AddFxDrawHookPreParticles);
#else
void AddFxDrawHookPreParticles(DrawHookFn func, void *arg) {
    s32 n = g_fxHooksPreCount[0];
    if (n < 0x40) {
        g_fxHooksPreFuncs[n] = func;
        g_fxHooksPreArgs[n] = arg;
        g_fxHooksPreCount[0] = n + 1;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunFxDrawHooksPreParticles);

/* Register a (func,arg) callback in the after-ties draw queue (cap 0x40), run
 * by RunDrawHooksAfterTies. No-op when the queue is full.
 * Near-miss: same count-%hi register-allocation wall as
 * AddFxDrawHookPreParticles. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027D500);
#else
void func_0027D500(DrawHookFn func, void *arg) {
    s32 n = g_drawHooksAfterTiesCount[0];
    if (n < 0x40) {
        g_drawHooksAfterTiesFuncs[n] = func;
        g_drawHooksAfterTiesArgs[n] = arg;
        g_drawHooksAfterTiesCount[0] = n + 1;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunDrawHooksAfterTies);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunDrawHooksAfterShrubs);

/* Register a (func,arg) callback in the post-particle fx draw queue (cap 0x40),
 * run by RunFxDrawHooksPostParticles. No-op when the queue is full.
 * Near-miss: same count-%hi register-allocation wall as
 * AddFxDrawHookPreParticles. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AddFxDrawHookPostParticles);
#else
void AddFxDrawHookPostParticles(DrawHookFn func, void *arg) {
    s32 n = g_fxHooksPostCount[0];
    if (n < 0x40) {
        g_fxHooksPostFuncs[n] = func;
        g_fxHooksPostArgs[n] = arg;
        g_fxHooksPostCount[0] = n + 1;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunFxDrawHooksPostParticles);

/* Register a (func,arg) callback in the small late fx draw queue (cap 4), run
 * by RunFxDrawHooksLate at the end of the RenderFrame fx layer. No-op when
 * full.
 * Near-miss: same count-%hi register-allocation wall as
 * AddFxDrawHookPreParticles. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AddFxDrawHookLate);
#else
void AddFxDrawHookLate(DrawHookFn func, void *arg) {
    s32 n = g_fxHooksLateCount[0];
    if (n < 4) {
        g_fxHooksLateFuncs[n] = func;
        g_fxHooksLateArgs[n] = arg;
        g_fxHooksLateCount[0] = n + 1;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunFxDrawHooksLate);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawBlobShadows);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", FadeOutToBlackBlocking);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027DB38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027DC40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027DF80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E1E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E368);

/* DrawFullScreenTint - append a full-screen alpha-tint draw to the frame DMA
 * chain: emit a GS RGBAQ register write packed from the (r,g,b,a) components,
 * then a 4-word DMACnt+ref chain entry pointing at the prebuilt tint quad
 * packet, and advance the cursor by 0x10 bytes.
 * Near-miss: the pinned cc1 CSEs the frame-DMA-cursor pointer load across the
 * four chain-word stores, whereas the original reloads it absolute for each
 * write (no-load-PRE). No declaration reproduces the per-store reload here.
 * Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawFullScreenTint);
#else
void DrawFullScreenTint(u64 r, s64 g, s64 b, s64 a) {
    AppendGsRegPacket(1, r | g << 8 | b << 16 | a << 24);
    g_frameDmaCursor[0][0] = 0x30000014;
    g_frameDmaCursor[0][1] = (u32)g_fullScreenTintPacket;
    g_frameDmaCursor[0][2] = 0;
    g_frameDmaCursor[0][3] = 0x50000014;
    g_frameDmaCursor[0] += 4;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E4D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E690);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawGlyphQuad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E818);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawTexturedQuad2d);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027EB20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawRotatedSprite2d);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027EFA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F0A8);

/* DrawFlatRect2d - build four packed XYZ2 corner words for an integer cell
 * rect (x1,y1)-(x2,y2) at depth `z` (each coord scaled *16, GS-window-offset,
 * biased -8) and hand them to the flat-sprite appender with TEX0 `tex0`.
 * Near-miss: the operations are byte-for-byte the same (sll/addu/dsll/or/sd),
 * but the pinned cc1 schedules the four corner builds and their stack stores in
 * a different order / register assignment than the original. Correct C
 * preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F168);
#else
void func_0027F168(s32 x1, s32 y1, s32 x2, s32 y2, s64 z, u64 tex0) {
    u64 corners[4];
    s64 vx2 = x2 * 0x10 + g_gsPixelOffsetX[0] - 8;
    s64 vy2 = (s64)(y2 * 0x10 + g_gsPixelOffsetY[0] - 8) << 0x10;
    s64 vx1 = x1 * 0x10 + g_gsPixelOffsetX[0] - 8;
    s64 vy1 = (s64)(y1 * 0x10 + g_gsPixelOffsetY[0] - 8) << 0x10;
    s64 vz = z << 0x20;
    corners[0] = vx1 | vy1 | vz;
    corners[3] = vx2 | vy2 | vz;
    corners[1] = vx2 | vy1 | vz;
    corners[2] = vx1 | vy2 | vz;
    func_0027F0A8(corners, tex0);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F208);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F348);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F4D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F4D8);

/* Enable blob shadows: set the enable flag and return 1 (success).
 * Near-miss: the pinned cc1 materialises the constant 1 twice (one reg for the
 * store, one for the return value) instead of reusing a single register as the
 * original does, and pads the jr delay slot with a nop rather than the store. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F790);
#else
s32 func_0027F790(void) {
    return (g_blobShadowEnabled = 1);
}
#endif

/* Disable blob shadows: clear the enable flag.
 * Near-miss: the pinned cc1 emits the store then `jr $31` with a nop in the
 * delay slot, while the original packs the store into the jr delay slot. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F7A0);
#else
void func_0027F7A0(void) {
    g_blobShadowEnabled = 0;
}
#endif

/* func_0027F7A8 (MeasureTextByGlyphTable): sum the signed per-glyph advance
 * widths of `str` - each glyph is 4 bytes in `glyphTable`, signed advance at +3 -
 * stopping at NUL or after `maxChars` chars (maxChars == -1 means until NUL). A
 * zero advance is skipped (movn). The current char's advance is folded in before
 * the maxChars return check (the movn sits in the beq delay slot). Worker behind
 * the three font wrappers below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F7A8);
#else
s32 func_0027F7A8(const char *str, s32 maxChars, const void *glyphTable) {
    const u8 *s = (const u8 *)str;
    const s8 *gt = (const s8 *)glyphTable;
    s32 sum = 0;
    s32 i;
    if (maxChars == 0 || s[0] == 0) return 0;
    i = 1;
    do {
        s8 advance = gt[s[0] * 4 + 3];
        s++;
        if (advance != 0) sum += advance;
        if (i == maxChars) return sum;
        i++;
    } while (s[0] != 0);
    return sum;
}
#endif

/** Measure pixel width of a string in the D_263B10 font. */
s32 func_0027F7F8(const char *str, s32 maxChars) {
    return func_0027F7A8(str, maxChars, D_263B10);
}

/** Measure pixel width of a string in the debug font. */
s32 func_0027F818(const char *str, s32 maxChars) {
    return func_0027F7A8(str, maxChars, g_debugFontGlyphTable);
}

/** Measure pixel width of a string in the D_264250 font. */
s32 func_0027F838(const char *str, s32 maxChars) {
    return func_0027F7A8(str, maxChars, D_264250);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F858);

/** Measure scaled pixel width of a string in the D_263B10 font. */
s32 func_0027F900(const char *str, s32 maxChars, f32 scale) {
    return func_0027F858(str, maxChars, D_263B10, scale);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawFixedFontString);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027FBA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawDebugString);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027FCA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027FCB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027FFF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280080);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280120);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002801B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280250);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002802E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280380);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280440);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002804C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280550);

/* DrawTextBoxDefault - the unscaled text-box entry: forward to the scaled core
 * with scale 1.0f. The five integer args pass straight through unchanged.
 * Near-miss: the pinned cc1 sibling-call-optimizes the lone tail call to
 * `j func_00280550`; the original keeps a full call+return frame. Correct C
 * preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280B20);
#else
void func_00280B20(s32 a, s32 b, s32 c, s32 d, s32 e) {
    func_00280550(a, b, c, d, e, 1.0f);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280B48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280BB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280C28);

/* InitTextBoxLayout - fill the 0x18-byte (12 s16) text-box layout descriptor
 * consumed by DrawWrappedTextBox. Fields: [0]clipX0 [1]clipX1 [2]left [3]right
 * [4]anchorX [5]y [8]lineHeight [9]flags; the out/shadow fields [6][7][10][11]
 * are zeroed. param_9 (flags) arrives on the stack. */
void func_00280C98(s16 *layout, s16 clipX0, s16 clipX1, s16 left, s16 right,
                   s16 anchorX, s16 y, s16 lineHeight, s32 flags) {
    layout[0] = clipX0;
    layout[1] = clipX1;
    layout[2] = left;
    layout[3] = right;
    layout[4] = anchorX;
    layout[5] = y;
    layout[8] = lineHeight;
    layout[9] = flags;
    layout[6] = 0;
    layout[7] = 0;
    layout[10] = 0;
    layout[11] = 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280CD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AppendVu1SphereMapContext);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280EC8);

/* Queue the ctx1 draw-env packet, then recompute the GS screen geometry.
 * Near-miss: cc1 sibling-call-optimizes the final func_0027B988() to
 * `j func_0027B988`, whereas the original keeps the call+return frame. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280FB8);
#else
void func_00280FB8(void) {
    AppendDrawEnvContext1();
    func_0027B988();
}
#endif

/* func_00280FE0 - per-frame reset of the offscreen-probe subsystem: zero the
 * 0x600-byte screen-grab/occlusion query descriptor table at 0x1B8990 and clear
 * the pending query count at 0x1B15D8.
 * Near-miss: the original clears the count via the assembler `sw $0, sym`
 * ($at-macro) absolute form and computes its %hi after restoring $ra, whereas
 * the pinned cc1 allocates a GP register for the count address and hoists the
 * %hi before the restore. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280FE0);
#else
void func_00280FE0(void) {
    FillMemory32(g_sceneActorMobys + 0x44, 0, 0x600);
    g_screenGrabQueryCount = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00281010);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00281020);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002810C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002812A8);

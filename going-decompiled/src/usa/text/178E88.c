#include "common.h"

/* Blob-shadow subsystem state. g_blobShadowCount (0x1B15BC) is the queue
 * count; the enable flag lives 0x2C bytes after it (0x1B15E8), accessed
 * gp-relative under -G8. */
extern s32 g_blobShadowCount;
#define g_blobShadowEnabled (*(&g_blobShadowCount + 0xB))

/* Render-layer enable bitmask (gp-relative under -G8). */
extern s32 g_renderLayerMask;

/* Draw-env / frame helpers (defined elsewhere in the render unit). */
extern void AppendDrawEnvContext1(void);
extern void AppendScreenClearPacket(s32 mode);
extern void RenderFrame(void);
extern void RenderMenuScreenWidgets(s32 which);
extern void func_0027B988(void);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00278F08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00278F90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00278FD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279CF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279D68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279D88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279E00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279EE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279F08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A0C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A130);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A138);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", BuildFrameViewMatrices);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A550);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", LookupOcclusionGridCell);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", LookupNeighborOcclusionCell);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", ResolveOcclusionVisMask);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", UpdateOcclusionVisMask);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", InitScreenGeometry);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", BuildCameraProjection);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027B858);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027B988);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", SetupGsDisplayBuffers);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", ResetPerFrameDrawQueues);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AppendFrameInitGsState);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027BFA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027C020);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CB08);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AddFxDrawHookPreParticles);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunFxDrawHooksPreParticles);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027D500);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunDrawHooksAfterTies);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunDrawHooksAfterShrubs);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AddFxDrawHookPostParticles);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunFxDrawHooksPostParticles);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AddFxDrawHookLate);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunFxDrawHooksLate);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawBlobShadows);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", FadeOutToBlackBlocking);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027DB38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027DC40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027DF80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E1E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E368);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawFullScreenTint);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E4D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E690);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawGlyphQuad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E818);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawTexturedQuad2d);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027EB20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawRotatedSprite2d);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027EFA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F0A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F168);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F7A8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280B20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280B48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280BB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280C28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280C98);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280FE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00281010);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00281020);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002810C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002812A8);

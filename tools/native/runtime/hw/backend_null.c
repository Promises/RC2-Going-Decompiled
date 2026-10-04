/* backend_null.c - M3 ps2hw HEADLESS backend (TARGET_NATIVE).
 *
 * No-op bodies for the PS2 GS/VIF/DMA hardware functions so the native build
 * traverses ONE rendering frame WITHOUT trapping and WITHOUT rendering. These
 * are strong defs that override the weak M4 trap-stubs (rt0/stubs.c) at link.
 * This serves the test framework (headless state validation) - it is NOT an
 * emulator; a later `backend_swgs` would actually rasterize. Per-function no-op
 * contracts were established by a ghidra-annotator pass (see docs/HLE.md M3).
 * Entirely TARGET_NATIVE-only, so the matching (PS2) build is unaffected (it
 * uses the original asm via INCLUDE_ASM).
 *
 * Args are intentionally ignored: each no-op is declared `(void)` and the
 * callers pass through their own externs; under the native cdecl ABI the caller
 * cleans the stack, so a no-op needs no real signature - only the symbol name
 * must match. The two functions whose RETURN VALUE a caller uses keep a real
 * return type and a benign constant.
 */
#ifdef TARGET_NATIVE
#include "common.h"

/* ----- headless DMA scratch -------------------------------------------------
 * GS packet builders append through g_frameDmaCursor (0x1B2228). Some builders
 * still RUN under backend_null because they have a real functional-equiv #else
 * body (e.g. DrawFlatRect2d), so the cursor must point at valid memory or those
 * writes dereference the zero-init arena slot and fault. Point it at a scratch
 * page; the bytes are never kicked to a GS or read back. Sized for one headless
 * frame. If a PINE snapshot seeds g_frameDmaCursor with a non-native address,
 * call ps2hw_null_reset_frame() after seeding (and once per frame) to repoint
 * it here. */
extern u8 *g_frameDmaCursor; /* 0x1B2228 */
static u8 s_dmaScratch[0x20000] __attribute__((aligned(16)));

void ps2hw_null_reset_frame(void)
{
    g_frameDmaCursor = s_dmaScratch;
}

__attribute__((constructor))
static void ps2hw_null_init(void)
{
    ps2hw_null_reset_frame();
}

/* ----- GS/VIF/DMA packet builders + frame ops: drop the work ---------------- */
/* glyph draw - top-of-subtree bypass. Its one state write (the auto-fit scale
 * at *(f32*)elem[1]) IS read back by GuiTextElementMeasure, but GuiMenuListDraw
 * re-seeds it via GuiElementSetScale at the top of every row iteration, so the
 * dropped write is never observed stale. Safe to no-op. */
void GuiTextElementDraw(void)      {}
/* Begin2dDrawBatch, End2dDrawBatch, DrawRotatedSprite2d, DrawGlyphQuad and
 * AppendFrameInitGsState: removed (task #1460) - text/178E88.cpp defines them
 * natively, and since #1460 its .cpp joins the link unmangled. */
void AppendGsRegPacket(void)       {}
void AppendDrawEnvContext1(void)   {}
void AppendDrawEnvContext2(void)   {}
void AppendScreenClearPacket(void) {}
void FlushCache(void)              {} /* EE cache sync - no DMA headless */

/* ----- deadlock-critical: these spin forever headless (no DMA/IRQ, no GS regs)
 * unless they no-op / return idle. Do NOT just drop their work - they must NOT
 * arm a fence or busy-wait. ------------------------------------------------- */
void KickFrameDmaChain(void)       {} /* must NOT set g_dwFrameDmaFenceFlags */
void WaitFrameDmaFence(void)       {} /* else spins on the fence flags */
u64  WaitGsPathsIdle(void)         { return 0; } /* else spins on VIF1/GIF CHCR */
u32  WaitVblankGetField(void)      { return 1; } /* field bit for frame pacing */

/* ===========================================================================
 * Batch 2 - data-driven from the tester's ranked stub-hit sweep (stub_hits.txt).
 * Triaged per no-op contract; the 6 NEEDS-SHIM hits (InitMobyFromClass,
 * MenuScreenLoad, RequestLevelExit, UpdateVendorMenuInput, func_0027B988 screen
 * dims, func_00278EC0 scene-arena) are NOT here - a blind no-op would corrupt
 * the frame; they get functional shims separately.
 * =========================================================================== */

/* --- VOID-NOOP: GS/render-packet emit, render bookkeeping, or EE/SDK sync that
 * writes no gameplay state read back in-frame. ----------------------------- */
/* func_0011AAD0 (cod/015180.c: an empty native syscall arm), func_0027E4D0 and
 * BuildFrameViewMatrices (text/178E88.cpp), CloseMobyDmaSegment (text/1A00F0.cpp):
 * removed (task #1460) - the units define them natively. */
void BuildCameraProjection(void)    {} /* GS projection matrices (render-read 0x1b908x) */
/* CullAndEmitShrubs / CullAndBinTieInstances: hand-written inline VU0 macro-mode
 * DMA-chain builders (scratchpad + SPR-FROM DMA kicks) - a faithful #else is
 * impossible (the hardware IS the logic). Their only persisted writes are
 * render-output: the frame render-DMA cursor g_frameDmaCursor(0x1B2228) and the
 * tie render-list size globals - all reset each frame, no game-logic reader (the
 * bracketing open-tag g_pShrubSegmentOpenTag 0x1B2034 is written by their caller
 * BuildShrubDrawSegment, also render-output). Tester EXCLUDED these from the
 * state watch - so a render-emitter no-op is the correct contract, like
 * RenderFrame. */
void CullAndEmitShrubs(void)        {} /* shrub cull+emit - render-output only */
void CullAndBinTieInstances(void)   {} /* tie cull+bin draw/relight lists - render-output only */
/* func_00283558 = KickDmaChannelD: writes only the SPR-FROM DMA channel regs +
 * SYNC (kicks the shrub relight list). Pure hardware, no headless DMA - no-op. */
void func_00283558(void)            {} /* KickDmaChannelD - SPR-FROM DMA kick */
/* AllocateTieTextureVram: tie-texture VRAM LRU allocator built entirely from
 * inline VU0 macro-mode + SPR scratchpad + SPR-TO/FROM DMA (like the cull pair,
 * a faithful #else is impossible - the hardware is the logic). Its only persisted
 * writes are render-VRAM state (g_tieVramLruHead/Tail/Size + the VRAM slot table),
 * consumed only by the tie render path - render-output, no game-logic reader. */
void AllocateTieTextureVram(void)   {} /* tie-VRAM LRU alloc - render-output only */
/* FlushHudDisplayValue: removed - text/198FA0.c now joins the native link (it
 * holds InitMobyFromClass's #else body) and provides the REAL bare-C forwarder
 * (g_guiInstance-guarded push to the HUD render object). A no-op here would
 * multiply-define it. */
void RenderFrame(void)              {} /* master draw chain - packets + render-support
                                       * (vis-mask/procedural-anim/fade clamps); none
                                       * is gameplay state, safe to drop one frame */
void RenderMenuScreenWidgets(void)  {} /* front-end compositor - packets only */
/* UpdateScreenFadeWhite: removed (task #1460) - text/16E980.c defines it. */
void func_00272cc0(void)            {} /* DrawLockOnReticle - lock-on sprites */
void func_002857C8(void)            {} /* SetScreenClearColor - patches clear-packet RGBA */
/* snd_BankLoadAsync (cod/0321A0.c) and FadeOutToBlackBlocking (text/178E88.cpp):
 * removed (task #1489, RULING #9179) - a stub must not shadow tree C. The real
 * bodies run natively. Their IOP/SIF/CD-layer callees are stubbed in
 * sdk/iop_null.c, and FadeOutToBlackBlocking's fence/vblank waits are the
 * no-ops above, so its loop is bounded by its frame count. */

/* --- RETURN-CONST: caller uses the return; a fixed benign value is safe. The
 * non-void no-ops below also return 0 because their declared return is used or
 * could be (UploadTieTextures int - its caller discards it, but return a defined
 * 0 not a garbage register). ------------------------------------------------ */
/* StartFileLoadPumpingVoice (text/1B4218.cpp) and SetSndPumpCallback
 * (cod/0321A0.c): removed (task #1489, RULING #9179) - the units define them. */
s32  UploadTieTextures(void)          { return 0; } /* tie tex upload - caller ignores */
u32  func_0011D620(void)              { return 0; } /* sceSifCallRpc - 0 = RPC success */
u32  func_00286200(void)              { return 0; } /* IsMenuOverlayActive - 0 = inactive (New Game) */
/* func_0028BE10 and func_00289190 (text/188858.c): removed (task #1460) - the
 * unit defines both natively (func_00289190 is matched plain C, a table scan,
 * where this stub returned a constant 1). */

/* ===========================================================================
 * Batch 3 - water-pool state machine (text/1FFBA0 func_003002E0 / func_00300C08)
 * #else bodies. These five callees are trap-stubs in native; their effects are
 * headless-irrelevant to the pool-state fields the tester's identity check
 * verifies (g_waterPool / g_cameraState writes happen in the C body, not here),
 * so a strong no-op lets the #else bodies run faithfully without aborting.
 * =========================================================================== */
void SpawnParticleType55(void) {} /* particle emit (0x2C74C0) - no GS headless;
                                   * the durable hue-counter tick is done in C */
/* StopDialogVoice (text/1B4218.cpp): removed (task #1489, RULING #9179). */
/* func_002888D8, func_00288F30 (text/188858.c) and func_002AE6C8 (text/1A8180.c):
 * removed (task #1460) - the units define them natively. */

/* ===========================================================================
 * M3 batch 4 - data-driven from the tester's refreshed stub-hit sweep. 7 no-ops
 * (6 VOID-NOOP + 1 RETURN-CONST). The 2 NEEDS-SHIM hits (func_002912B8 rebuilds
 * g_inventoryOrder; func_002CABC0 resets the world camera g_flCameraPos/Matrix)
 * are NOT here - they write in-frame-read game state and get functional shims.
 * =========================================================================== */
/* func_0011AEA0 (cod/015180.c: an empty native syscall arm): removed (task #1460). */
void AppendTexFlushDefaultTex0(void){} /* GIF default-TEX0 flush DMA tag */
void RenderSaveLoadStatusPopup(void){} /* Begin2dDrawBatch..End - GS packets only */
/* RunSprRenderPipeline (text/1A00F0.cpp), func_0026FC88 and func_00271FE8
 * (text/16E980.c): removed (task #1460) - the units define them natively. */
/* func_00133250 (cod/0321A0.c): removed (task #1489, RULING #9179) - the unit
 * defines it. */

/* ===========================================================================
 * M5 - in-level driven-frame trap list (tester FINALIZED 2026-06-19,
 * state/inlevel_trap_list.md). MenuScreenLoad is the only trap that is a
 * weak-stub-only symbol (no #else body), so it gets its strong no-op here -
 * which also un-traps its #else caller func_002CBA10 (-> MenuScreenLoad + 2
 * particle-blob writes). The other 12 traps all have TARGET_NATIVE #else bodies
 * (a strong no-op here would multiply-define), so they are shimmed in-place at
 * the #else-body level - see the NATIVE_INLEVEL_SHIM guards in their units.
 * =========================================================================== */
void MenuScreenLoad(void)           {} /* blocking menu-screen asset load (caller func_002CBA10) */

#endif /* TARGET_NATIVE */

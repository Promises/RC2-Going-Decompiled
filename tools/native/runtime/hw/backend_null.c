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
void Begin2dDrawBatch(void)        {} /* resets VRAM/upload bookkeeping only */
void End2dDrawBatch(void)          {}
void AppendGsRegPacket(void)       {}
void DrawRotatedSprite2d(void)     {}
void DrawGlyphQuad(void)           {}
void AppendDrawEnvContext1(void)   {}
void AppendDrawEnvContext2(void)   {}
void AppendFrameInitGsState(void)  {}
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
void func_0011AAD0(void)            {} /* RotateThreadReadyQueue - syscall(0x2b) yield */
void func_0027E4D0(void)            {} /* AppendScreenRectFill - GIF sprite-fill */
void BuildCameraProjection(void)    {} /* GS projection matrices (render-read 0x1b908x) */
void BuildFrameViewMatrices(void)   {} /* view/world-screen matrices (render-read 0x1b518x) */
void CloseMobyDmaSegment(void)      {} /* DMA tag splice */
/* FlushHudDisplayValue: removed - text/198FA0.c now joins the native link (it
 * holds InitMobyFromClass's #else body) and provides the REAL bare-C forwarder
 * (g_guiInstance-guarded push to the HUD render object). A no-op here would
 * multiply-define it. */
void RenderFrame(void)              {} /* master draw chain - packets + render-support
                                       * (vis-mask/procedural-anim/fade clamps); none
                                       * is gameplay state, safe to drop one frame */
void RenderMenuScreenWidgets(void)  {} /* front-end compositor - packets only */
void UpdateScreenFadeWhite(void)    {} /* drives the (visual) white-fade ramp */
void func_00272cc0(void)            {} /* DrawLockOnReticle - lock-on sprites */
void func_002857C8(void)            {} /* SetScreenClearColor - patches clear-packet RGBA */
void snd_BankLoadAsync(void)        {} /* kicks IOP RPC#3 - idle headless */

/* --- DEADLOCK-RISK -> no-op: a cosmetic vblank-blocking fade whose per-iter
 * WaitFrameDmaFence/WaitVblankGetField never advance headless. Writes only
 * fade/render state. --------------------------------------------------------*/
void FadeOutToBlackBlocking(void)   {}

/* --- RETURN-CONST: caller uses the return; a fixed benign value is safe. The
 * non-void no-ops below also return 0 because their declared return is used or
 * could be (UploadTieTextures int, StartFileLoadPumpingVoice u64 - callers
 * discard, but return a defined 0 not a garbage register). ------------------ */
s32  UploadTieTextures(void)          { return 0; } /* tie tex upload - caller ignores */
u64  StartFileLoadPumpingVoice(void)  { return 0; } /* voice-pump wrapper - discarded */
u32  SetSndPumpCallback(void)         { return 0; } /* install IOP pump cb - discarded */
u32  func_0011D620(void)              { return 0; } /* sceSifCallRpc - 0 = RPC success */
u32  func_0028BE10(void)              { return 0; } /* RegisterHudElement - handle, render-only */
u32  func_00286200(void)              { return 0; } /* IsMenuOverlayActive - 0 = inactive (New Game) */
/* IsGadgetClassValid: MUST return 1, NOT 0. Caller func_00291148 sets the
 * inventory-order slot to 0xFF when this returns 0 - a 0 here WIPES inventory
 * in-frame. 1 (valid) is the non-destructive headless default. */
u32  func_00289190(void)              { return 1; }

/* ===========================================================================
 * Batch 3 - water-pool state machine (text/1FFBA0 func_003002E0 / func_00300C08)
 * #else bodies. These five callees are trap-stubs in native; their effects are
 * headless-irrelevant to the pool-state fields the tester's identity check
 * verifies (g_waterPool / g_cameraState writes happen in the C body, not here),
 * so a strong no-op lets the #else bodies run faithfully without aborting.
 * =========================================================================== */
void SpawnParticleType55(void) {} /* particle emit (0x2C74C0) - no GS headless;
                                   * the durable hue-counter tick is done in C */
void StopDialogVoice(void)     {} /* dialog-voice fade/stop - audio bookkeeping */
void func_002888D8(void)       {} /* AdvanceWeaponVariant(itemId) - inventory bk */
void func_00288F30(void)       {} /* weapon progress-gate iterate(itemId) - bk */
void func_002AE6C8(void)       {} /* EquipGadgetItem(itemId) - equip bookkeeping */

/* ===========================================================================
 * M3 batch 4 - data-driven from the tester's refreshed stub-hit sweep. 7 no-ops
 * (6 VOID-NOOP + 1 RETURN-CONST). The 2 NEEDS-SHIM hits (func_002912B8 rebuilds
 * g_inventoryOrder; func_002CABC0 resets the world camera g_flCameraPos/Matrix)
 * are NOT here - they write in-frame-read game state and get functional shims.
 * =========================================================================== */
void func_0011AEA0(void)            {} /* FlushCache (real symbol - callers use
                                        * the func_ name/addr, not "FlushCache") */
void AppendTexFlushDefaultTex0(void){} /* GIF default-TEX0 flush DMA tag */
void RenderSaveLoadStatusPopup(void){} /* Begin2dDrawBatch..End - GS packets only */
void RunSprRenderPipeline(void)     {} /* CPU-side render packet gen (RunRenderTaskList) */
void func_0026FC88(void)            {} /* GS CLUT+texture upload packets; *outTex0 render-only */
void func_00271FE8(void)            {} /* draw/cull context-flag (write-only); UpdateCamera writes
                                        * g_flCameraPos/Matrix itself AFTER this returns */
u64  func_00133250(void)            { return 0; } /* blocking IOP snd RPC + snd_Pump drain loop -
                                        * deadlock headless; 0 = IOP-ready/success the callers want */

#endif /* TARGET_NATIVE */

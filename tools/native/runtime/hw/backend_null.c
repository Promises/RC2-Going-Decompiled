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

#endif /* TARGET_NATIVE */

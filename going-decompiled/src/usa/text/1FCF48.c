#include "common.h"

/* Frame-arena / render-task-list globals (carved from the text/1FA5A8 island). */
extern u8   g_memoryArenaTable[]; /* 0x1BAE40 memory-region table (SetupMemoryArenaTable) */
extern s32  g_frameArenaBase[];   /* 0x1B2220 double-buffered per-frame arena base pair [0]/[1] */
extern s32  g_frameArenaFlip;     /* 0x1B2234 active half index 0/1 */
extern s32  g_sceneArenaCursor;   /* 0x1B2230 scene-arena allocation cursor */
extern s32  g_renderTaskList;     /* 0x1B1630 render-task-list ptr for RunRenderTaskList */
extern s32  g_renderTaskWorkBuf;  /* 0x1B1634 render-task work-buffer ptr */
extern u8  *g_frameDmaCursor;     /* 0x1B2228 frame VIF1 chain write cursor */
extern s32  D_1A8BC0;             /* 0x1A8BC0 scene-arena reserve size (set 0x2000 by teardown) */
extern s32  g_playerProgress;     /* 0x1A79F8 story-progress level */
extern s32  D_262D30[];           /* 0x262D30 per-progress scene-arena cursor table */
extern s32  D_262D98[];           /* 0x262D98 per-progress scene-arena reserve table */
void func_002FD020(void);         /* defined below in this unit */

/* Addressing model (unit built -G8, task #889). cc1 treats every s32 scalar
 * here as small data and emits a bare `lw/sw $r,sym` macro; the assembler then
 * picks the form from the `.extern sym,size` it saw first. The ROM stores
 * g_sceneArenaCursor, g_frameArenaFlip and g_renderTaskWorkBuf with a single
 * %gp_rel op, and reaches everything else through the absolute macro pairs
 * (`lui $r; lw $r,%lo($r)` for a load, `lui $at; sw` for a store). So the
 * absolute ones are sized 16 before cc1's own directive, and the two symbols
 * the ROM reaches BOTH ways are read through assembler aliases sized 16 (the
 * #8036 construct; relocations name the real symbols). */
#ifndef TARGET_NATIVE
__asm__(".extern g_playerProgress, 16");
__asm__(".extern D_1A8BC0, 16");
__asm__(".extern g_renderTaskList, 16");
__asm__(".extern g_frameDmaCursor, 16");
__asm__(".extern g_sceneArenaCursorAbs, 16\n\tg_sceneArenaCursorAbs = g_sceneArenaCursor");
__asm__(".extern g_frameArenaFlipAbs, 16\n\tg_frameArenaFlipAbs = g_frameArenaFlip");
extern s32 g_sceneArenaCursorAbs;
extern s32 g_frameArenaFlipAbs;
#else
#define g_sceneArenaCursorAbs g_sceneArenaCursor
#define g_frameArenaFlipAbs   g_frameArenaFlip
#endif

/* SelectSceneArenaRegion: select the scene/render-arena region for the current
 * story progress. Index the two per-progress tables by g_playerProgress (or 0
 * when it is past the 0x19-entry tables), publish the reserve (D_1A8BC0) and the
 * scene-arena cursor, then rederive the render-task list (func_002FD020). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FCF48", func_002FCFC8);
#else
void func_002FCFC8(void) {
    s32 idx = ((u32)g_playerProgress < 0x19) ? g_playerProgress : 0;
    D_1A8BC0 = D_262D98[idx];
    g_sceneArenaCursor = D_262D30[idx];
    func_002FD020();
}
#endif

/* func_002FD020 (DeriveRenderTaskList): derive the render-task-list and
 * work-buffer pointers from the active frame-arena half.
 * g_renderTaskList = g_frameArenaBase[flip] + sceneCursor - reserve; the work
 * buffer sits 0x2000 below it. No params, no return; a pure leaf.
 * The flip index and the scene cursor are read through the absolute aliases
 * (the ROM loads them with `lui/lw` pairs) while g_renderTaskWorkBuf is stored
 * %gp_rel in the jr delay slot.
 * MATCHED (task #889): 100.00% sdk29 (unit objdiff, objdiff_build.sh), unit
 * built -G8 -fno-gcse. */
void func_002FD020(void) {
    s32 list = g_frameArenaBase[g_frameArenaFlipAbs] + g_sceneArenaCursorAbs - D_1A8BC0;
    g_renderTaskList = list;
    g_renderTaskWorkBuf = list - 0x2000;
}

/* ResetFrameArenas: reset the double-buffered per-frame arena pair from the
 * memory-region table (half0 = table[+0xC], half1 = table[+0x10]), point the
 * frame DMA cursor at half0, clear the flip to 0, then rederive the render-task
 * list (func_002FD020). No params, no return.
 * The table goes through a pointer temporary: the ROM materialises the table
 * base once and loads +0x10 then +0xC from it, where the direct
 * `g_memoryArenaTable + 0xC` spelling folds the offset into the %hi/%lo pair.
 * The empty asm after the final call keeps the ROM's call + return frame (cc1
 * would otherwise tail-jump; FACT #8177).
 * MATCHED (task #889): 100.00% sdk29 (unit objdiff, objdiff_build.sh), unit
 * built -G8 -fno-gcse. */
void ResetFrameArenas(void) {
    u8 *table = g_memoryArenaTable;
    s32 half0 = *(s32 *)(table + 0xC);
    g_frameArenaBase[1] = *(s32 *)(table + 0x10);
    g_frameArenaBase[0] = half0;
    g_frameDmaCursor = (u8 *)half0;
    g_frameArenaFlip = 0;
    func_002FD020();
    __asm__ __volatile__("");
}

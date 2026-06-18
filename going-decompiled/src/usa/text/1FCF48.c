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

/* SelectSceneArenaRegion: picks the scene/render-arena region by g_playerProgress
 * then calls func_002FD020. Still INCLUDE_ASM (leaf #else bodies land first). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FCF48", func_002FCFC8);

/* func_002FD020: derive the render-task-list and work-buffer pointers from the
 * active frame-arena half. g_renderTaskList = arenaHalf + sceneCursor - reserve;
 * the work buffer sits 0x2000 below it. A pure leaf (no callees). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FCF48", func_002FD020);
#else
void func_002FD020(void) {
    s32 list = g_frameArenaBase[g_frameArenaFlip] + g_sceneArenaCursor - D_1A8BC0;
    g_renderTaskList = list;
    g_renderTaskWorkBuf = list - 0x2000;
}
#endif

/* ResetFrameArenas: reset the double-buffered per-frame arena pair from the
 * memory-region table (half0 = table[+0xC], half1 = table[+0x10]), point the
 * frame DMA cursor at half0, clear the flip to 0, then rederive the render-task
 * list (func_002FD020). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FCF48", ResetFrameArenas);
#else
void ResetFrameArenas(void) {
    s32 half0 = *(s32 *)(g_memoryArenaTable + 0xC);
    g_frameArenaBase[1] = *(s32 *)(g_memoryArenaTable + 0x10);
    g_frameArenaBase[0] = half0;
    g_frameDmaCursor = (u8 *)half0;
    g_frameArenaFlip = 0;
    func_002FD020();
}
#endif

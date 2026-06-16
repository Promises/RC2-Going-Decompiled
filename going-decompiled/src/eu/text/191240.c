#include "common.h"

/*
 * EU text/191240 - REGION-AXIS twin of USA text/191238 (boot/IRX init + sky
 * render + segment/asset loader + MAP/minimap TU). Ported verbatim from
 * src/usa/text/191238.c across the REGION axis (2026-06-15); the matched bodies
 * are region-agnostic - only extern global names and unit-local callee names
 * are retargeted to their EU addresses. Built -O2 -G8 -fno-gcse (mirrors the
 * USA unit). func_00298AD0 (empty leaf) is emitted as C by the splat stub
 * generator. EU function names follow the EU vaddr.
 */

/* Two subsystem entry points in the neighbouring asm region (EU addresses;
 * USA func_00278EC0 -> EU func_00278D50, USA func_00278F90 -> EU func_00278E20). */
extern void func_00278D50(void);
extern void func_00278E20(void);

/* Per-level map-revealed bitmap base (EU 0x139638; the +0xA7 tail slice is
 * g_mapRevealedFlags 0x1396DF). USA twin base is D_1395B8 (0x1395B8). */
extern u8 D_139638[];

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002912C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291328);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291330);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002913C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002914E0);

/* Thin frame-keeping forwarder to a subsystem entry point (the original builds
 * a frame + jal rather than sibling-call; empty-asm guard suppresses cc1's
 * tail-call lowering). USA twin: func_00291980. */
void func_00291A20(void) {
    func_00278D50();
    __asm__ __volatile__("");
}

/* Thin frame-keeping forwarder (USA twin: func_002919A0). */
void func_00291A40(void) {
    func_00278E20();
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291A60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291A70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291B80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291C00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291C10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291D58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291DC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291F50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292068);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292098);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292160);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002925B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002926F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002927D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292988);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292C38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292CD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292D18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292EF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292F00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292FC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293198);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293430);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293498);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002937C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293910);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293B70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293BC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293CE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293DC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293E08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002941A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002942C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294368);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002945B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294640);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002948A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294980);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002949D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294A40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294A90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294BB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294CA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294D30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294EF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294F40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294FD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00295298);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002954D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00295550);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00295690);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00295758);

/* MapIsLevelRevealed - returns the per-level "map discovered" bit (0/1). Indexes
 * the revealed-area bitmap as byte level/8, bit level%8 (signed div/mod via the
 * shift-with-bias idiom). The (always-true) bounds guard yields 0 for the
 * degenerate case. The bitmap tail g_mapRevealedFlags (0x1396DF) is the +0xA7
 * slice of the table based at D_139638 (0x139638 + 0xA7 == 0x1396DF). */
s32 MapIsLevelRevealed(s32 level) {
    s32 byteIndex = level / 8;
    s32 bit = level - byteIndex * 8;
    s32 result = 0;
    if ((u32)bit < 8) {
        result = (D_139638[0xA7 + byteIndex] >> bit) & 1;
    }
    return result;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapInit);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapBeginUpload);

/*
 * func_00295F90 — scan the 5 map cache slots for the entry whose state word
 * (table at g_mapTextureWidth+0x48) is set and whose id word
 * (table at g_mapTextureWidth+0x5C) is still -1 (unassigned). With param==0 it
 * scans forward (slot i); otherwise it probes from the end (slot 4-i). Returns
 * the matching slot index, or -1 if none qualifies. (USA func_00295F30.)
 */
extern s32 g_mapTextureWidth[];
s32 func_00295F90(s32 fromEnd) {
    s32 *state = &g_mapTextureWidth[0x12];   /* +0x48 */
    s32 *id    = &g_mapTextureWidth[0x17];   /* +0x5C */
    s32 i;
    for (i = 0; i < 5; i++) {
        s32 idx = 4 - i;
        if (fromEnd == 0) {
            idx = i;
        }
        if (state[idx] != 0 && id[idx] == -1) {
            return idx;
        }
    }
    return -1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00295FF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00296098);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapFindCacheSlot);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapDataExistsForLevel);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapFindNearestAvailableLevel);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapGetLevelOrderIndex);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapEvictCacheSlot);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002964F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapSetCurrentLevel);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapUpdateLevelAvailability);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapUpdate);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapDraw);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00297B78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapBuildBitmapFrom4bpp);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00297EB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00297FC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapBuildBitmap);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298108);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298338);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298508);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298510);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298760);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002988F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298948);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298A28);

void func_00298AD0(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298AD8);

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

/*
 * func_00291B80 — EU twin of USA RenderSky. Draws the sky shells for the frame:
 * opens a sky segment, points the shell spin-rate table at the static rates
 * (D_255B40), draws scaled spin in the boot/title area (g_playerProgress == 0) or
 * fixed spin in-game, closes the segment, then appends SCANMSK (0x47)=0x5360B and
 * the Z-buffer base (reg 0x4E)=0x1000000 | (zbuf >> 13). See USA RenderSky for the
 * TU-flag/version wall (absolute vs gp-rel g_playerProgress + tail-call j); #else.
 */
extern void func_002E4580(void);   /* BeginSkyDrawSegment */
extern void func_00291A70(void);   /* DrawSkyShellsScaledSpin */
extern void func_002E4318(void);   /* DrawSkyShellsFixedSpin */
extern void func_002E45F0(void);   /* CloseSkyDrawSegment */
extern void func_002FD5B8(s32 reg, s32 val);   /* AppendGsRegPacket */
extern s32  g_playerProgress;
extern u8   D_001A7308[];          /* +0x58 = g_vramZBuffer (EU) */
extern u8   g_nBoltCounterDisplayed[]; /* +0x48 = g_pSkyShellSpinRates (EU) */
extern s32  D_255B40[];            /* static sky shell spin rates (EU) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291B80);
#else
void func_00291B80(void) {
    func_002E4580();
    *(s32 *)(g_nBoltCounterDisplayed + 0x48) = (s32)D_255B40;
    if (g_playerProgress != 0) {
        func_002E4318();
    } else {
        func_00291A70();
    }
    func_002E45F0();
    func_002FD5B8(0x47, 0x5360B);
    func_002FD5B8(0x4E, 0x1000000 | (*(s32 *)(D_001A7308 + 0x58) >> 13));
}
#endif

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

/*
 * func_00293B70 — EU twin of USA func_00293B10. Relocate a freshly loaded class
 * chunk's embedded chunk-relative pointers to absolute addresses: chunk =
 * table[idx] (base-ptr array at +0x48); rebase the +0x14 pointer (if non-zero)
 * and the +0x1C[count] pointer array (count = byte at +0x10) by the chunk base.
 * Region-agnostic body (no global refs). See USA twin for the daddu/move
 * single-instruction version wall; kept as #else.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293B70);
#else
void func_00293B70(s32 *table, s32 idx) {
    u8 *chunk = (u8 *)((s32 *)((u8 *)table + 0x48))[idx];
    s32 *relPtr = (s32 *)(chunk + 0x14);
    if (*relPtr != 0) {
        *relPtr = (s32)(chunk + *relPtr);
    }
    if (*(u8 *)(chunk + 0x10) != 0) {
        s32 *p = (s32 *)(chunk + 0x1C);
        s32 i = 0;
        do {
            *p = (s32)(chunk + *p);
            i++;
            p++;
        } while (i < *(u8 *)(chunk + 0x10));
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293BC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293CE0);

/*
 * func_00293DC8 (EU twin of USA func_00293D68, 0x293D68; delta +0x60) — fix up a
 * freshly-loaded display-model header in place. Copies the 4-byte tag/flags
 * block (src[0..3] -> dst[4..7]), then rebases the two embedded self-relative
 * offsets (at src+4 and src+8) to absolute pointers into the loaded buffer:
 * dst[0] = src + src[4] and dst[0x20] = src + src[8]. Pure region-agnostic leaf.
 */
void func_00293DC8(u8 *dst, u8 *src) {
    dst[4] = src[0];
    dst[5] = src[1];
    dst[6] = src[2];
    dst[7] = src[3];
    *(s32 *)(dst + 0x00) = (s32)(src + *(s32 *)(src + 4));
    *(s32 *)(dst + 0x20) = (s32)(src + *(s32 *)(src + 8));
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293E08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002941A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002942C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294368);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002945B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294640);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002948A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294980);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002949D0);

/*
 * func_00294A40 — EU twin of USA func_002949E0. Commit a pending streaming-slot
 * record: when enabled and the slot index rec[1] is non-negative, store the class
 * id rec[0] into the per-slot in-flight table (D_0014B5C0+0x7790, field +0x34,
 * stride 4 by slot). If the slot index equals the armed-slot latch D_1A93FC, that
 * latch is toggled to rec[1]^1. rec[0] is ALWAYS stamped -1 on return. See USA
 * twin for the daddu/move version wall (cmp-oracle-verified semantics); #else.
 */
extern u8  D_0014B5C0[];   /* g_discToc (EU); in-flight table at +0x7790 */
extern s32 D_1A93FC;       /* armed-slot latch (EU) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294A40);
#else
void func_00294A40(s32 *rec, s32 enable) {
    if (enable != 0 && rec[1] >= 0) {
        *(s32 *)(D_0014B5C0 + 0x7790 + rec[1] * 4 + 0x34) = rec[0];
        if (rec[1] == D_1A93FC) {
            D_1A93FC = rec[1] ^ 1;
        }
    }
    rec[0] = -1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294A90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294BB0);

/*
 * func_00294CA8 — EU twin of USA func_00294C48. Request the gadget moby-class
 * load for `slot`: look classId up in the gadget-class TOC (D_0014B5C0+0x4B40,
 * stride 5 ints, up to 0x30 entries). If absent (idx == 0x30) do nothing.
 * Otherwise, unless the per-slot in-flight record (D_0014B5C0+0x7790 + slot, field
 * +0x34) already equals the found index, kick the load via func_00294BB0 with the
 * per-slot dest buffer (slot*0xC800 + table+0x40). See USA twin for the daddu/move
 * version wall; kept as the portable #else.
 */
extern void func_00294BB0(s32 idx, s32 slot, void *dest, s32 a3);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294CA8);
#else
void func_00294CA8(s32 classId, s32 slot) {
    s32 *toc = (s32 *)D_0014B5C0;
    s32 idx = 0;
    if (toc[0x12D0] != classId) {            /* D_0014B5C0 + 0x4B40 */
        s32 *p = toc + 0x12D0;
        for (idx = 1; idx < 0x30; idx++) {
            p += 5;
            if (p[0] == classId) {
                break;
            }
        }
    }
    if (idx != 0x30) {
        u8 *rec = D_0014B5C0 + 0x7790 + slot * 4;
        if (*(s32 *)(rec + 0x34) != idx) {
            void *dest = (void *)(slot * 0xC800 + (s32)(D_0014B5C0 + 0x7790 + 0x40));
            func_00294BB0(idx, slot, dest, 0);
        }
    }
}
#endif

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

/* MapFindCacheSlot(levelAndFlag): scan the 5 map cache slots for an occupied
 * slot (slotState != 0) holding this level id. Returns the slot index, or -1.
 *
 * The matched build walks a single moving pointer `id` that starts at
 * &slotLevelId[0] (g_mapVertexData + 0x29C); slotState[i] is reached as id[-5]
 * (0x29C - 0x14 == 0x288). Materializing &g_mapVertexData as a base pointer and
 * adding 0x29C separately is what keeps cc1 from folding the two into one `la`
 * reloc — the shape the original was built with. Byte-exact USA + EU.
 * (USA text/191238 MapFindCacheSlot; EU g_mapVertexData 0x1C4FA0.) */
extern s32 g_mapVertexData[];        /* 0x1C4FA0 map cache / vertex-data base */
s32 MapFindCacheSlot(s32 levelAndFlag) {
    s32 *base = g_mapVertexData;
    s32 *id   = base + (0x29C / 4);   /* &slotLevelId[0]; slotState[i] == id[i-5] */
    s32 i = 0;
    do {
        if (id[-5] != 0 && id[0] == levelAndFlag) {
            return i;
        }
        i++;
        id++;
    } while (i < 5);
    return -1;
}

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

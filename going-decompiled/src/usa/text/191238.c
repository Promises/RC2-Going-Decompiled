#include "common.h"

/*
 * text/191238 — boot/IRX init + sky render + segment/asset loader + MAP/minimap
 * TU (carved out of the .text asm segment on 2026-06-14, ranked-carve-pipeline
 * Tier-1-B). Like the other gameplay/UI text TUs this was built by a later SN
 * cc1 that lacks the load-PRE pass and packs callee-save stack slots 8-byte;
 * the matcher builds this unit at -O2 -G8 -fno-gcse (per-unit GFLAG override in
 * tools/ee/objdiff_build.sh / diff.sh / build.sh). Every other text unit stays
 * at its own flag.
 *
 * -G8 extern-sizing rules (same as the sibling text TUs): a complete extern of
 * size <= 8 bytes lands in small data (gp-relative); a larger object the
 * original reads with the adjacent lui/%lo "assembler macro" shape is declared
 * small PLUS a file-scope `.extern sym,16` override so cc1 emits the one-insn
 * symbolic macro while GNU as expands it absolutely.
 *
 * SAVE-LAYOUT WALL: every function that saves 2+ callee registers is blocked —
 * the later cc1 packs save slots 8-byte where the pinned 2.9-ee-991111 cc1
 * reserves 16 bytes per save. Functions whose only callee save is $ra are
 * unaffected; pure leaves match freely.
 */

extern void func_00278EC0(void);
extern void func_00278F90(void);

/* Inventory-order rebuild helpers (text/188858). */
extern s32 IsItemUnlockedAtProgress(s32 itemId, s32 progress);
extern s32 AddItemToInventoryOrder(s32 itemId);

/*
 * func_00291980 / func_002919A0 — thin frame-keeping forwarders to two
 * subsystem entry points in text/16E980's neighbourhood. The original does not
 * sibling-call (it builds a frame + jal), so an empty-asm guard suppresses
 * cc1's tail-call lowering.
 */
void func_00291980(void) {
    func_00278EC0();
    __asm__ __volatile__("");
}

void func_002919A0(void) {
    func_00278F90();
    __asm__ __volatile__("");
}

/*
 * MapIsLevelRevealed — returns the per-level "map discovered" bit (0/1). Indexes
 * the revealed-area bitmap as byte level/8, bit level%8 (signed div/mod via the
 * shift-with-bias idiom). The (always-true) bounds guard yields 0 for the
 * degenerate case. The bitmap tail g_mapRevealedFlags (0x13965F) is the +0xA7
 * slice of a larger table based at D_1395B8 (0x1395B8 + 0xA7 == 0x13965F).
 */
extern u8 D_1395B8[];
s32 MapIsLevelRevealed(s32 level) {
    s32 byteIndex = level / 8;
    s32 bit = level - byteIndex * 8;
    s32 result = 0;
    if ((u32)bit < 8) {
        result = (D_1395B8[0xA7 + byteIndex] >> bit) & 1;
    }
    return result;
}

/*
 * NON-MATCHING map/loader helpers left as INCLUDE_ASM (honest measured walls
 * with this 2.9-ee-991111 cc1; the later SN cc1 that built this gameplay TU
 * differs systematically):
 *
 *   MapSetCurrentLevel (0x296500, 66.9%) — the store of the level id must land
 *     in the jal-MapUpdateLevelAvailability delay slot; our cc1 sinks it into
 *     straight-line code and the empty-asm tail-call guard then occupies the
 *     slot (store-into-jal-delay scheduling wall).
 *   func_002949E0 (0x2949E0, 89.7%) — the gp_rel D_1A933C reload is scheduled
 *     before its store, and the equality test lowers to `bnel` (branch-likely)
 *     where the original uses a plain `bne`.
 *   func_00293B10 (0x293B10, 90.0%) — register-coloring (the table base lands in
 *     $5 not $4) plus the `daddu $r,$0,$0` zero-idiom the later cc1 emits where
 *     ours emits `move` (= addu).
 *   func_00293D68 (0x293D68, 72.7%) — independent-store rescheduling: cc1 hoists
 *     the second offset load above the first pointer store (proved no-alias).
 *   MapDataExistsForLevel (0x296120, 70.9%) — the early-return `if` lowers to
 *     `beql` (branch-likely) where the original uses a plain `beqz` with the
 *     level mask computed once in the delay slot.
 *   MapGetLevelOrderIndex (0x2962C0, 75.0%) — register-coloring (the table
 *     pointer is split $3/$6 in the original, kept in $6 by ours).
 *   MapFindCacheSlot (0x2960D8, 87.9%) — cc1 folds `&g_mapVertexData + 0x29C`
 *     into one reloc (2-insn `la`) where the original keeps the base and adds
 *     0x29C separately (3 insns); plus the same `daddu` zero-idiom.
 */

/* func_002912B8(progress): rebuild the inventory quick-select order. Walks item
 * ids 0..0x37 and, for each that IsItemUnlockedAtProgress reports available at
 * the given story progress (id 0 excluded), appends it via AddItemToInventoryOrder
 * (which owns the g_inventoryOrder writes). The matching build keeps the asm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002912B8);
#else
void func_002912B8(s32 progress) {
    s32 i;
    for (i = 0; i < 0x38; i++) {
        if (IsItemUnlockedAtProgress(i, progress) != 0 && i != 0) {
            AddItemToInventoryOrder(i);
        }
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291320);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadIrxModuleFromBuffer);

/* Memory-region table consumed by ResetFrameArenas + the loaders. */
extern u8   g_memoryArenaTable[]; /* 0x1BAE40, 0x9C bytes */
extern s32  g_sceneArenaCursor;   /* 0x1B2230 */

/* SetupMemoryArenaTable: zero the 0x9C-byte g_memoryArenaTable then fill the EE
 * memory-region base addresses the loaders + ResetFrameArenas read: the scene
 * arena halves at 0x354000 (+cursor / +2*cursor), splash/loading-WAD buffers,
 * the per-asset display-model buffers (ship/held-item/player), GUI + debug-malloc
 * pools, and the boot-WAD / upper-RAM region tops. Called per level by
 * RebootIopAndInitEngine + InitLoadingSceneSystem. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", SetupMemoryArenaTable);
#else
void SetupMemoryArenaTable(void) {
    u8 *t = g_memoryArenaTable;
    s32 cursor = g_sceneArenaCursor;
    memset(t, 0, 0x9C);
    *(s32 *)(t + 0x04) = 0x100000;
    *(s32 *)(t + 0x08) = 0x354000;          /* g_relocOffsetLimit */
    *(s32 *)(t + 0x0C) = 0x354000;          /* g_sceneDecompressBase (half0) */
    *(s32 *)(t + 0x10) = cursor + 0x354000; /* g_pSceneArenaBase (half1) */
    *(s32 *)(t + 0x14) = cursor * 2 + 0x354000; /* g_pSplashImageBuffer */
    *(s32 *)(t + 0x18) = cursor * 2 + 0x454000; /* g_pLoadingSceneWad */
    *(s32 *)(t + 0x68) = cursor * 2 + 0x454000;
    *(s32 *)(t + 0x6C) = 0x1F0C000;         /* g_stagedSegmentCeiling */
    *(s32 *)(t + 0x70) = 0x1F0C000;         /* g_shipModelBufferBase */
    *(s32 *)(t + 0x74) = 0x1F20000;         /* g_heldItemModelBufferBase */
    *(s32 *)(t + 0x78) = 0x1F28000;         /* g_playerModelBufferBase */
    *(s32 *)(t + 0x7C) = 0x1F54000;         /* g_debugMallocPoolBase */
    *(s32 *)(t + 0x80) = 0x1FB8000;         /* g_guiMemoryBase */
    *(s32 *)(t + 0x84) = 0x1FF8000;         /* boot-WAD staging top */
    *(s32 *)(t + 0x88) = 0x1FFC000;
    *(s32 *)(t + 0x8C) = 0x7000000;
    *(s32 *)(t + 0x90) = 0x7100000;
    *(s32 *)(t + 0x94) = 0x7180000;
    *(s32 *)(t + 0x98) = 0x7200000;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BootSystemInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002919C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", DrawSkyShellsScaledSpin);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", RenderSky);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291B60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291B70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291CB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291D28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291EB0);

#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; save-layout wall (saves
 * s0+ra -> the pinned 2.9-ee-991111 cc1 reserves a 0x20 frame, the original's
 * later cc1 packs it to 0x10). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291FC8);
#else
/*
 * func_00291FC8 — thin wrapper: run the subsystem reset (func_00291FF8) then
 * dispatch the kept argument to func_00291EB0.
 */
extern void func_00291FF8(void);
extern void func_00291EB0(void *arg);
void func_00291FC8(void *arg) {
    func_00291FF8();
    func_00291EB0(arg);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291FF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002920C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00292510);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BindParticleFxAssets);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00292650);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BindSkyData);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadPlayerDisplayTextures);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BindPlayerDisplayModel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadPlayerDisplayModel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadHeldItemDisplayModel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00292E90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadShipDisplayModel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadShipDisplayTexture);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", ParseLoadedSegment);

__asm__(".extern g_pLoadedSegment, 16");
__asm__(".extern g_pHudAssetHeader, 16");
extern u8 *g_pLoadedSegment;
extern u8 *g_pHudAssetHeader;
extern void DecompressWad(void *src);
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; save-layout wall (saves
 * s0+ra -> pinned cc1 reserves a 0x20 frame vs the original's 0x10). Body is
 * byte-identical apart from the frame size + ra slot offset. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002933D0);
#else
/*
 * func_002933D0 — per HUD-asset-slot reset. When the rounded-up size (param2)
 * is non-zero, decompress that slot's WAD chunk from the loaded segment
 * (chunk offset at g_pLoadedSegment[slot] +0x20, relative to the segment base).
 * Then clears the slot's status word (+0x74) in the HUD asset header table.
 */
void func_002933D0(s32 slot, s32 size) {
    if (((size + 0xF) & 0xFFFFFFF0) != 0) {
        u8 *seg = g_pLoadedSegment;
        DecompressWad(*(s32 *)(seg + slot * 8 + 0x20) + seg);
    }
    *(s32 *)(g_pHudAssetHeader + slot * 4 + 0x74) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00293438);

/* TODO(match): functional equivalent - not byte-exact (48.12%); strength-reduce/
 * register-color wall - the original's later cc1 addresses the 0x18-stride entry
 * three ways (mult idx*0x18 for ev0, shift-add idx*3 for ev1/ev2) where the
 * pinned 2.9-ee-991111 cc1 CSEs them to one base+mult with immediate ld offsets,
 * shuffling the whole GIF-tag-pack register coloring. Body (3-way idx>=0 / idx<-1
 * / idx==-1 GIF-tag emit) is semantically equivalent. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00293760);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002938B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00293B10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00293B68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", RelocateMobyClassChunk);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00293D68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", FixupMobyClassHeader);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", RegisterMobyClass);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294268);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", StartFrontendSegmentLoad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294308);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", UpdateLevelStagingMachine);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294550);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", StreamSceneSegment);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BindSceneChunk);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadGlobalDialogScene);

#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact (87.5%); save-layout wall
 * (saves s0+ra -> the pinned 2.9-ee-991111 cc1 reserves a 0x20 frame where the
 * original's later cc1 packs the two 8-byte slots into 0x10) plus a gp_rel/
 * absolute divergence: g_sceneArenaBase/g_sceneArenaCursor lower to %gp_rel
 * under -G8 where the original reloads them with absolute lui/%lo. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", SelectSceneSubChunk);
#else
/*
 * SelectSceneSubChunk — repoint the scene streaming buffer at sub-chunk `which`,
 * bind that scene chunk, then restore the buffer to the live arena allocation.
 * The sub-chunk descriptor block lives at g_cameraSlotActive+0x990: +0x70 holds
 * the active streaming-buffer pointer (g_pSceneLoadBuffer), and +0x74[which] the
 * per-sub-chunk saved pointer. After BindSceneChunk consumes the temporary the
 * buffer is reset to g_sceneArenaBase + g_sceneArenaCursor.
 */
extern u8 g_cameraSlotActive[];
extern s32 g_sceneArenaBase;
extern void BindSceneChunk(void);
void SelectSceneSubChunk(s32 which) {
    u8 *t = &g_cameraSlotActive[0x990];
    *(s32 *)(t + 0x70) = *(s32 *)(t + which * 4 + 0x74);
    BindSceneChunk();
    *(s32 *)(t + 0x70) = g_sceneArenaBase + g_sceneArenaCursor;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294970);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002949E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294A30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294B50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294C48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294CD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294E98);

#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact (52%); loop-peel wall -
 * same as func_00295478: the pinned cc1 lowers the peeled first TOC-search
 * iteration to `bnel`/branch-likely where the original uses a plain `beq` then
 * a rotated do-while. Body/registers otherwise track the original. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294EE0);
#else
/*
 * func_00294EE0 — query a gadget moby-class id's load state.
 * Looks the class id up in the gadget-class TOC (g_discToc+0x4B40, stride 5
 * ints, up to 0x30 entries). If the id isn't in the TOC at all, returns 1.
 * Otherwise checks the 3-entry in-flight request list (g_respawnPlayerYaw+0x7C)
 * for the found index and returns 1 if it IS present (already in-flight), else 0
 * (asm: xori j,0x3 / sltu 0,_ at 0x294F6C). Companion query to func_00295478
 * (which enqueues the load).
 */
extern s32 g_discToc[];
extern s32 g_respawnPlayerYaw[];
s32 func_00294EE0(s32 classId) {
    s32 *toc = g_discToc;
    s32 idx;
    if (toc[0x12D0] == classId) {               /* g_discToc + 0x4B40 */
        idx = 0;
    } else {
        s32 *p = toc + 0x12D0;
        for (idx = 1; idx < 0x30; idx++) {
            p += 5;
            if (p[0] == classId) {
                break;
            }
        }
    }
    if (idx == 0x30) {
        return 1;                               /* not in the gadget TOC */
    }
    {
        s32 *req = &g_respawnPlayerYaw[0x1F];    /* g_respawnPlayerYaw + 0x7C */
        s32 j;
        if (req[0] == idx) {
            j = 0;
        } else {
            for (j = 1; j < 3; j++) {
                if (req[j] == idx) {
                    break;
                }
            }
        }
        return (j ^ 3) != 0;                      /* found -> nonzero, else 0 */
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadMobyClassFromWad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00295238);

/*
 * func_00295478 — look up a gadget moby-class id in the gadget-class TOC
 * (g_gadgetClassToc == g_discToc+0x4B40, stride 5 ints, up to 0x30 entries). On
 * a hit, record the found index in the staging halfword (g_loadingScenesPlayed
 * +0x7C) and request the class load via func_00294B50(idx, -1, dest, 1).
 */
__asm__(".extern g_discToc, 16");
__asm__(".extern g_loadingScenesPlayed, 16");
extern s32 g_discToc[];
extern u8 g_loadingScenesPlayed[];
extern void func_00294B50(s32 idx, s32 a1, void *dest, s32 a3);
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact (78.7%); loop-peel wall -
 * the pinned cc1 peels the first search iteration (folding base+0x4B54 as a
 * constant offset) where the original's later cc1 keeps the clean rotated loop.
 * Prologue/registers/tail otherwise match. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00295478);
#else
/*
 * func_00295478 — look up a gadget moby-class id in the gadget-class TOC
 * (g_gadgetClassToc == g_discToc+0x4B40, stride 5 ints, up to 0x30 entries). On
 * a hit, record the found index in the staging halfword (g_loadingScenesPlayed
 * +0x7C) and request the class load via func_00294B50(idx, -1, dest, 1).
 */
void func_00295478(s32 classId, void *dest) {
    s32 *base = g_discToc;
    s32 idx = 0;
    if (base[0x12D0] != classId) {       /* g_discToc + 0x4B40 */
        s32 *toc = base + 0x12D0;
        for (idx = 1; idx < 0x30; idx++) {
            toc += 5;
            if (toc[0] == classId) {
                break;
            }
        }
    }
    if (idx != 0x30) {
        *(s16 *)(g_loadingScenesPlayed + 0x7C) = (s16)idx;
        func_00294B50(idx, -1, dest, 1);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002954F0);

#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact (22%); 64-bit shift wall -
 * the pinned cc1 lowers each `(u64)x << n` field shift to a `dsll32`+`dsrl` pair
 * where the original emits a single `dsll`/`dsll32`, plus the gp_rel/absolute
 * divergence on g_texUploadCount (-G8 small-data vs original absolute lui/%lo).
 * Pure scalar GS-register packing; semantics verified against the asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00295630);
#else
/*
 * func_00295630 — build a GS texture-register word from the texel-format fields
 * and, if the upload queue has room (< 0x40 entries), append a 0x10-byte
 * descriptor to g_texUploadQueue. The 64-bit word packs the width-log2 (clamped
 * so the shift floor is 6), the source address (a0<<26 | 0x1300000 base), the
 * destination page field (a1<<30), the texel halfwords (a4>>8 at bit 37) and the
 * fixed 0x8000<<19 + top sign bit. The queue entry mirrors a2/a3 as raw words
 * and a0/a1/(a4>>8)/(a5>>8) as the byte/halfword fields. Returns the packed word
 * whether or not the entry was queued.
 */
extern s32 g_texUploadQueue[];
extern s32 g_texUploadCount;
u64 func_00295630(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5) {
    s32 shift = (a0 < 6) ? 0 : (a0 - 6);
    u64 packed = (u64)(u32)(a5 >> 8)
               | ((u64)(u32)(1 << shift) << 14)
               | (((u64)(u32)a0 << 26) | 0x1300000)
               | ((u64)(u32)a1 << 30)
               | ((u64)(u32)(a4 >> 8) << 37)
               | ((u64)0x8000 << 19)
               | ((u64)-1 << 63);
    if (g_texUploadCount < 0x40) {
        s32 *e = &g_texUploadQueue[g_texUploadCount * 4];
        e[0] = a2;
        *(s16 *)((u8 *)e + 6) = (s16)(a4 >> 8);
        *(s16 *)((u8 *)e + 4) = 0;
        e[2] = a3;
        *(s16 *)((u8 *)e + 0xE) = (s16)(a5 >> 8);
        *(u8 *)((u8 *)e + 0xC) = (u8)a0;
        *(u8 *)((u8 *)e + 0xD) = (u8)a1;
        g_texUploadCount++;
    }
    return packed;
}
#endif

#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact (95.97%); register-color
 * wall - the original's later cc1 lands the level/8 quotient in a temp then
 * `move`s it to the index register (a redundant daddu copy our pinned cc1
 * elides); the body is otherwise byte-identical. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002956F8);
#else
/*
 * func_002956F8 — mark a level discovered on the galactic map and return its
 * previous revealed bit. Sets bit (level%8) of byte (level/8) in the revealed-
 * area bitmap (g_mapRevealedFlags, the +0xA7 slice of D_1395B8). The signed
 * div/mod uses the shift-with-bias idiom; the (always-true) bounds guard yields
 * 0 for the degenerate case. Companion setter to MapIsLevelRevealed.
 */
s32 func_002956F8(s32 level) {
    s32 byteIndex = level / 8;
    s32 bit = level - byteIndex * 8;
    s32 prev;
    if ((u32)bit < 8) {
        prev = (D_1395B8[0xA7 + byteIndex] >> bit) & 1;
    } else {
        prev = 0;
    }
    if ((u32)bit < 8) {
        D_1395B8[0xA7 + byteIndex] |= (1 << bit);
    }
    return prev;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapBeginUpload);

/*
 * func_00295F30 — scan the 5 map cache slots for the entry whose state word
 * (table at g_mapTextureWidth+0x48) is set and whose id word
 * (table at g_mapTextureWidth+0x5C) is still -1 (unassigned). With param==0 it
 * scans forward (slot i); otherwise it probes from the end (slot 4-i). Returns
 * the matching slot index, or -1 if none qualifies.
 */
extern s32 g_mapTextureWidth[];
s32 func_00295F30(s32 fromEnd) {
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00295F98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00296038);

/*
 * ── Galactic-map cache / level-availability slice ──────────────────────────
 *
 * Shared state object g_mapCache (0x1C4F20, == g_mapVertexData base). The
 * matched build keeps the lui/%lo absolute access shape, so these declarations
 * are TARGET_NATIVE-only and never reach the byte-matched objects (the
 * INCLUDE_ASM arms below own those). Field offsets recovered from the asm +
 * confirmed against the named globals in symbol_addrs:
 *   +0x24  s32  available     (g_mapAvailable  0x1C4F44)
 *   +0x230 s32  currentLevel  (g_mapCurrentLevel 0x1C5150)
 *   +0x234 s32  activeSlot    (g_mapActiveSlot 0x1C5154)
 *   +0x288 s32  slotState[5]  per-cache-slot occupancy flag
 *   +0x29C s32  slotLevelId[5] per-cache-slot level id (-1 == unassigned)
 *
 * The level id passed to the cache/TOC lookups carries an optional 0x100 flag
 * bit: when SET it selects the primary map-data TOC, when CLEAR the secondary
 * (alternate) TOC; the low byte is the level number. g_mapDataSet (0x1A7B05)
 * is the active-set selector that drives that flag.
 *
 * g_pLevelOrder (0x1AA510) points at the 28-entry galactic-map level-order
 * array (level ids in display/unlock order; a 0 entry past index 0 means "no
 * level"). g_discToc (0x14B540) holds the per-level map-data TOC: the +0x14F4
 * (primary) / +0x15D4 (secondary) word, stride 8 by level, is the sector
 * count, >0 iff map data exists for that level.
 */
#ifdef TARGET_NATIVE
typedef struct MapCache {
    u8  _pad00[0x24];
    s32 available;         /* +0x24  */
    u8  _pad28[0x230 - 0x28];
    s32 currentLevel;      /* +0x230 */
    s32 activeSlot;        /* +0x234 */
    u8  _pad238[0x288 - 0x238];
    s32 slotState[5];      /* +0x288 */
    s32 slotLevelId[5];    /* +0x29C */
    s32 lockedSlot;        /* +0x2B0  slot index to leave untouched when evicting */
} MapCache;
/* Offset checks only on a C11+ host (ee-gcc 2.9 used by the EE-backend suite
 * predates _Static_assert). */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(__builtin_offsetof(MapCache, available)    == 0x24,  "MapCache.available");
_Static_assert(__builtin_offsetof(MapCache, currentLevel) == 0x230, "MapCache.currentLevel");
_Static_assert(__builtin_offsetof(MapCache, activeSlot)   == 0x234, "MapCache.activeSlot");
_Static_assert(__builtin_offsetof(MapCache, slotState)    == 0x288, "MapCache.slotState");
_Static_assert(__builtin_offsetof(MapCache, slotLevelId)  == 0x29C, "MapCache.slotLevelId");
_Static_assert(__builtin_offsetof(MapCache, lockedSlot)   == 0x2B0, "MapCache.lockedSlot");
#endif

extern MapCache g_mapCache;          /* 0x1C4F20 (== g_mapVertexData) */
extern s32     *g_pLevelOrder;       /* 0x1AA510 -> s32[28] level-order array */
extern u8       g_mapDataSet;        /* 0x1A7B05 active map-data-set selector */
extern s32      g_playerProgress;    /* 0x1A79F8 story progress counter */
/* g_discToc (0x14B540) already declared above; map sector counts live at
 * +0x14F4 (primary) / +0x15D4 (secondary), stride 8 bytes by level. */

s32 MapDataExistsForLevel(s32 levelAndFlag);
s32 MapFindCacheSlot(s32 levelAndFlag);
s32 MapGetLevelOrderIndex(s32 level);
s32 MapUpdateLevelAvailability(void);
extern s32 func_002835E0(s32 x);     /* integer abs() (text/183558) */
#endif

/* MapDataExistsForLevel(levelAndFlag): does map data exist for the given level?
 * The 0x100 flag bit selects the primary map-data TOC when set, the secondary
 * when clear; the low byte is the level. Reads the per-level sector count from
 * g_discToc and returns 1 if > 0.
 *
 * WALL (70.9%): the early-return `if` lowers to `beql` (branch-likely, with the
 * level mask computed once in the delay slot) where the pinned cc1 emits a
 * plain `beqz`. Logic exact; kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapDataExistsForLevel);
#else
s32 MapDataExistsForLevel(s32 levelAndFlag) {
    s32 level = levelAndFlag & 0xFF;
    s32 *toc = g_discToc + level * 2;         /* stride 8 bytes */
    if (levelAndFlag & 0x100) {
        return 0 < toc[0x14F4 / 4];           /* primary set sector count */
    }
    return 0 < toc[0x15D4 / 4];               /* secondary set sector count */
}
#endif

/* MapFindCacheSlot(levelAndFlag): scan the 5 map cache slots for an occupied
 * slot (slotState != 0) holding this level id. Returns the slot index, or -1.
 *
 * WALL (87.9%): the pinned cc1 folds `&g_mapVertexData + 0x29C` into a single
 * `la` reloc (2 insns) where the original keeps the base and adds 0x29C
 * separately (3 insns), plus a `daddu`-zero vs `move` idiom drift. Logic exact;
 * kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapFindCacheSlot);
#else
s32 MapFindCacheSlot(s32 levelAndFlag) {
    s32 i;
    for (i = 0; i < 5; i++) {
        if (g_mapCache.slotState[i] != 0 && g_mapCache.slotLevelId[i] == levelAndFlag) {
            return i;
        }
    }
    return -1;
}
#endif

/* MapFindNearestAvailableLevel(): pick the level to upload next. Try the
 * current level (with the active-set 0x100 flag) first; if it's not already
 * cached and has map data, use it. Otherwise spiral outward through the
 * level-order array (offsets +1,-1,+2,-2,+3,-3,+4) from the current level's
 * order index, returning the first ordered level that has map data and isn't
 * already cached. Returns the level id (|flag), or -1 if none qualifies.
 *
 * WALL: the multi-callee-save 0x40 frame is packed 8-byte by the later cc1
 * (the unit-wide save-layout wall), and the spiral's movz/negu step is coloured
 * differently. Logic traced op-for-op; kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapFindNearestAvailableLevel);
#else
s32 MapFindNearestAvailableLevel(void) {
    s32 flag = (g_mapDataSet == 0) ? 0 : 0x100;
    s32 candidate = g_mapCache.currentLevel + flag;
    s32 orderIndex;
    s32 step;
    s32 probe;

    if (MapFindCacheSlot(candidate) == -1 && MapDataExistsForLevel(candidate)) {
        return candidate;
    }

    orderIndex = 0;
    if (g_mapCache.currentLevel < 0x1C) {
        s32 *p = g_pLevelOrder;
        while (*p != g_mapCache.currentLevel) {
            p++;
            orderIndex++;
        }
    }

    step = 1;
    probe = orderIndex + 1;
    do {
        if (probe >= 0 && probe < 0x1C && g_pLevelOrder[probe] != 0) {
            candidate = g_pLevelOrder[probe] + flag;
            if (MapFindCacheSlot(candidate) == -1 && MapDataExistsForLevel(candidate)) {
                return candidate;
            }
        }
        step = (step < 1) ? (1 - step) : -step;
        probe = orderIndex + step;
    } while (step != 4);
    return -1;
}
#endif

/* MapGetLevelOrderIndex(level): index of `level` in the level-order array
 * (g_pLevelOrder[28]). Returns 0 if it's the first entry, the matching index up
 * to 0x1B, or -1 if not found / the resolved entry is empty while the player
 * has any story progress.
 *
 * WALL (75.0%): register-coloring — the original splits the table pointer
 * across $3/$6, the pinned cc1 keeps it in one register. Logic exact; kept as
 * the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapGetLevelOrderIndex);
#else
s32 MapGetLevelOrderIndex(s32 level) {
    s32 *order = g_pLevelOrder;
    s32 idx;
    if (order[0] == level) {
        idx = 0;
    } else {
        s32 i;
        idx = -1;
        for (i = 1; i < 0x1C; i++) {
            idx = i;
            if (order[i] == level) {
                break;
            }
            idx = -1;
        }
    }
    if (order[idx] == 0 && g_playerProgress != 0) {
        idx = -1;
    }
    return idx;
}
#endif

/* MapEvictCacheSlot(): choose a cache slot to reuse and mark it free, returning
 * its index.
 *
 * Two phases, both skipping the locked slot (g_mapCache.lockedSlot):
 *  - If the current level is the hub (0): scan slots 4..0 and return the first
 *    occupied slot (slotState != 0) that already holds an unassigned id (-1) —
 *    a free-marked slot is reused as-is, no further work.
 *  - Otherwise resolve the current level's order index. If the current level is
 *    not in the level order at all, return 1. Else scan slots 0..4: an occupied
 *    slot already holding -1 is returned immediately; among the rest pick the
 *    one whose level's order index is farthest (max |orderIndex(slot) -
 *    orderIndex(current)|) from the current level. That farthest slot (default 0
 *    if none qualified) is marked free (slotLevelId = -1) and its index returned.
 *
 * WALL: the two branch-likely (beql) slot-skip loops, the movz default-to-0 of
 * the result, and the packed 0x50 multi-save frame diverge from the pinned
 * cc1's plain-branch / save-layout shapes. Logic traced op-for-op; kept as the
 * portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapEvictCacheSlot);
#else
s32 MapEvictCacheSlot(void) {
    s32 currentOrderIndex;
    s32 bestSlot;
    s32 maxDist;
    s32 i;

    if (g_mapCache.currentLevel == 0) {
        for (i = 4; i >= 0; i--) {
            if (g_mapCache.slotState[i] != 0 &&
                i != g_mapCache.lockedSlot &&
                g_mapCache.slotLevelId[i] == -1) {
                return i;
            }
        }
    }

    currentOrderIndex = MapGetLevelOrderIndex(g_mapCache.currentLevel);
    if (currentOrderIndex == -1) {
        return 1;
    }

    bestSlot = -1;
    maxDist = 0;
    for (i = 0; i < 5; i++) {
        s32 dist;
        if (g_mapCache.slotState[i] == 0 || i == g_mapCache.lockedSlot) {
            continue;
        }
        if (g_mapCache.slotLevelId[i] == -1) {
            return i;
        }
        dist = func_002835E0(MapGetLevelOrderIndex(g_mapCache.slotLevelId[i] & 0xFF)
                             - currentOrderIndex);
        if (maxDist < dist) {
            maxDist = dist;
            bestSlot = i;
        }
    }

    if (bestSlot == -1) {
        bestSlot = 0;
    }
    g_mapCache.slotLevelId[bestSlot] = -1;
    return bestSlot;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00296490);

/* MapSetCurrentLevel(level): set the galactic-map current level and refresh the
 * availability flag. Stores `level` into g_mapCache.currentLevel then calls
 * MapUpdateLevelAvailability.
 *
 * WALL (66.9%): the level store must land in the jal-MapUpdateLevelAvailability
 * delay slot; the pinned cc1 sinks it into straight-line code and the empty-asm
 * tail-call guard then occupies the slot. Logic exact; kept as the #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapSetCurrentLevel);
#else
void MapSetCurrentLevel(s32 level) {
    g_mapCache.currentLevel = level;
    MapUpdateLevelAvailability();
}
#endif

/* MapUpdateLevelAvailability(): clamp the current level to 0..0x1B, set
 * g_mapCache.available from MapDataExistsForLevel(currentLevel), then force it
 * clear when on the hub level (0) with any story progress. Returns available!=0.
 *
 * WALL: the clamp+store/jal interleave and the `bnel` (branch-likely) hub-clear
 * test diverge from the pinned cc1's plain branches and slot scheduling. Logic
 * traced op-for-op; kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapUpdateLevelAvailability);
#else
s32 MapUpdateLevelAvailability(void) {
    if (g_mapCache.currentLevel >= 0x1C) {
        g_mapCache.currentLevel = 0x1B;
    }
    g_mapCache.available = MapDataExistsForLevel(g_mapCache.currentLevel) ? 1 : 0;
    if (g_mapCache.currentLevel == 0 && g_playerProgress != 0) {
        g_mapCache.available = 0;
    }
    return g_mapCache.available != 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapUpdate);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapDraw);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00297B48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapBuildBitmapFrom4bpp);

/* TODO(match): functional equivalent - not byte-exact (64.83%); induction-var/
 * strength-reduction wall - the original's later cc1 keeps the loop index `i`
 * live (count-up `slt i,count`, recomputing i*2 in the branch delay slot) where
 * the pinned 2.9-ee-991111 cc1 strength-reduces it to an `i*3 += 3` accumulator
 * and rewrites the bound test as a count-down, diverging the whole loop frame.
 * The div-by-3 trap idiom (beql/break 0,7) and the 4bpp nibble unpack otherwise
 * reproduce exactly. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00297E80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00297F98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapBuildBitmap);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002980D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298308);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002984D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002984E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298730);

/* TODO(match): functional equivalent - not byte-exact (85.25%); branch-likely
 * wall - the original's later cc1 lowers the `progress==0x14` test to `beql`
 * (branch-likely) with the equal-path `lui %hi(g_pointLights)` in the annulled
 * delay slot; the pinned 2.9-ee-991111 cc1 only emits a plain `beq`. Body
 * (signed n%2 -> D_1A9428 / idx<<4 -> D_255E50) otherwise matches. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002988C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298918);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002989F8);

/* func_00298AA0: empty/no-op leaf (jr ra; nop). */
void func_00298AA0(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298AA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298F20);

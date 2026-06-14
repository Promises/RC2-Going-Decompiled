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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002912B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291320);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadIrxModuleFromBuffer);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", SetupMemoryArenaTable);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BootSystemInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002919C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", DrawSkyShellsScaledSpin);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", RenderSky);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291B60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291B70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291CB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291D28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291EB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291FC8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002933D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00293438);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", SelectSceneSubChunk);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294970);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002949E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294A30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294B50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294C48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294CD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294E98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294EE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadMobyClassFromWad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00295238);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00295478);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002954F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00295630);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002956F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapBeginUpload);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00295F30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00295F98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00296038);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapDataExistsForLevel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapFindCacheSlot);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapFindNearestAvailableLevel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapGetLevelOrderIndex);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapEvictCacheSlot);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00296490);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapSetCurrentLevel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapUpdateLevelAvailability);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapUpdate);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapDraw);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00297B48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapBuildBitmapFrom4bpp);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00297E80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00297F98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapBuildBitmap);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002980D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298308);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002984D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002984E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298730);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002988C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298918);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002989F8);

/* func_00298AA0: empty/no-op leaf (jr ra; nop). */
void func_00298AA0(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298AA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298F20);

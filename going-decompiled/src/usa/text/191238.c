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

/* D_1395B8: revealed-area bitmap base; the map "discovered" bit slice is at
 * +0xA7 (g_mapRevealedFlags 0x13965F). Used by MapIsLevelRevealed (moved to
 * address order down by MapInit) and the reveal setter below. */
extern u8 D_1395B8[];

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

/* LoadIrxModuleFromBuffer: load an IRX module from an in-memory image. Build the
 * loadfile arg block on the stack ([0]=image, [4]=arg, [8]=size, [C]=0), request
 * the load (func_0011AFE0); on success spin the completion poll (func_0011AFC0)
 * until it returns negative, then start the module (func_0011ED08(arg,0,0)) and
 * return 1 iff that succeeded (>=0). If the load request itself fails (returns 0),
 * return 1 without running/starting. */
extern void *func_0011AFE0(void *argBlock, s32 mode, void *arg);
extern s32 func_0011AFC0(void *handle);
extern s32 func_0011ED08(void *arg, s32 a1, s32 a2);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadIrxModuleFromBuffer);
#else
s32 LoadIrxModuleFromBuffer(void *image, s32 size, void *arg) {
    s32 args[4];
    void *h;
    s32 result = 1;

    args[0] = (s32)image;
    args[1] = (s32)arg;
    args[2] = size;
    args[3] = 0;
    h = func_0011AFE0(args, 1, arg);
    if (h != 0) {
        s32 r;
        do {
            r = func_0011AFC0(h);
        } while (r >= 0);
        r = func_0011ED08(arg, 0, 0);
        result = (r >= 0) ? 1 : 0;
    }
    return result;
}
/* byte-walled: 4 callee-saves at 16-byte slots (this cc1) vs the original 8-byte
 * packing. Correct C kept as the portable #else; cmp-oracle'd (callees mocked). */
#endif

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

/*
 * func_00291980 / func_002919A0 (0x291980 / 0x2919A0) — thin frame-keeping
 * forwarders to two subsystem entry points in text/16E980's neighbourhood. The
 * original does not sibling-call (it builds a frame + jal), so an empty-asm guard
 * suppresses cc1's tail-call lowering. Defined HERE in address order (between
 * BootSystemInit @0x2914D8 and func_002919C0), NOT at the file top.
 */
void func_00291980(void) {
    func_00278EC0();
    __asm__ __volatile__("");
}

void func_002919A0(void) {
    func_00278F90();
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002919C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", DrawSkyShellsScaledSpin);

/*
 * RenderSky — draw the sky shells for the current frame. Opens a sky draw
 * segment, points the shell spin-rate table at the static rates, then draws the
 * shells with scaled spin in the boot/title area (g_playerProgress == 0) or with
 * fixed spin in-game. Closes the segment and appends two GS register packets:
 * SCANMSK (0x47) = 0x5360B and the Z-buffer base (reg 0x4E) packed as
 * 0x1000000 | (g_vramZBuffer >> 13).
 *
 * WALL: compiles op-for-op identical EXCEPT (a) the ROM reads g_playerProgress /
 * g_vramZBuffer with the absolute lui/%lo macro, but those symbols are small
 * (<=8) and gp-rel-accessed by 11 OTHER functions in this -G8 TU, so a TU-wide
 * `.extern ...,16` absolute override would regress them; and (b) the final
 * AppendGsRegPacket is a tail position which our cc1 lowers to a sibling-call `j`
 * where the ROM keeps `jal`+epilogue. Both are TU-flag/version artifacts, not
 * source-controllable here; kept as the portable #else.
 */
extern void BeginSkyDrawSegment(void);
extern void DrawSkyShellsScaledSpin(void);
extern void DrawSkyShellsFixedSpin(void);
extern void CloseSkyDrawSegment(void);
extern void AppendGsRegPacket(s32 reg, s32 val);
extern s32  g_playerProgress;
extern s32  g_vramZBuffer;
extern void *g_pSkyShellSpinRates;
extern s32  g_skyShellSpinTableStatic[];
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", RenderSky);
#else
void RenderSky(void) {
    BeginSkyDrawSegment();
    g_pSkyShellSpinRates = g_skyShellSpinTableStatic;
    if (g_playerProgress != 0) {
        DrawSkyShellsFixedSpin();
    } else {
        DrawSkyShellsScaledSpin();
    }
    CloseSkyDrawSegment();
    AppendGsRegPacket(0x47, 0x5360B);
    AppendGsRegPacket(0x4E, 0x1000000 | (g_vramZBuffer >> 13));
    __asm__ __volatile__("");
}
#endif

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
extern void func_00291FF8(s32 index);
extern void func_00291EB0(void *arg);
void func_00291FC8(void *arg) {
    func_00291FF8((s32)arg);
    func_00291EB0(arg);
    __asm__ __volatile__("");
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291FF8);
#else
/*
 * func_00291FF8(index) — flush one entry of the deferred light-relight request
 * table (g_pointLights+0x100, stride 0x30). Each entry holds a target buffer
 * pointer (+0xC) and three (offset,length) s16 spans (+0x0/+0x2, +0x4/+0x6,
 * +0x8/+0xA) — units of u16 elements, hence the <<1 byte scaling. When the
 * buffer pointer is non-NULL, run the three relight passes
 * (func_002F5F70/func_002E4178/func_002F1B58), each handed the span's start
 * pointer, its end pointer (start + length*2), and the entry index; then clear
 * each span back to 0 so the request is consumed exactly once.
 *
 * WALL: two callee-saves (the entry pointer + the index) across three jal sites
 * with the packed s16-span loads — the pinned cc1's 16-byte save slots and
 * register colouring diverge from the original's later cc1. Kept as the
 * portable #else body.
 */
/* Three interleaved {off,len} s16 spans at +0x0/+0x2, +0x4/+0x6, +0x8/+0xA. */
typedef struct LightSpan { s16 off; s16 len; } LightSpan;
typedef struct LightRelightRequest {
    LightSpan span[3];   /* +0x0/+0x4/+0x8 off, +0x2/+0x6/+0xA len (u16 elems) */
    u8       *buffer;    /* +0xC           target relight buffer (NULL == idle) */
    u8        _pad10[0x30 - 0x10];
} LightRelightRequest;                       /* stride 0x30 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(__builtin_offsetof(LightRelightRequest, buffer) == 0xC, "LightRelightRequest.buffer");
_Static_assert(sizeof(LightRelightRequest) == 0x30, "LightRelightRequest stride");
#endif
/* The request table starts 0x100 into g_pointLights (0x1C2AC0). */
extern u8 g_pointLights[];
extern void func_002F5F70(u8 *start, u8 *end, s32 index);
extern void func_002E4178(u8 *start, u8 *end, s32 index);
extern void func_002F1B58(u8 *start, u8 *end, s32 index);
void func_00291FF8(s32 index) {
    LightRelightRequest *e =
        &((LightRelightRequest *)(g_pointLights + 0x100))[index];
    u8 *buf = e->buffer;
    if (buf == NULL) {
        return;
    }
    /* each pass: start = buf + off*2, end = start + len*2; relight; then the
     * spans are cleared so the request is consumed exactly once. */
    func_002F5F70(buf + (s32)e->span[0].off * 2,
                  buf + (s32)e->span[0].off * 2 + (s32)e->span[0].len * 2, index);
    e->span[0].off = 0;
    e->span[0].len = 0;
    buf = e->buffer;
    func_002E4178(buf + (s32)e->span[1].off * 2,
                  buf + (s32)e->span[1].off * 2 + (s32)e->span[1].len * 2, index);
    e->span[1].off = 0;
    e->span[1].len = 0;
    buf = e->buffer;
    func_002F1B58(buf + (s32)e->span[2].off * 2,
                  buf + (s32)e->span[2].off * 2 + (s32)e->span[2].len * 2, index);
    e->span[2].len = 0;
    e->span[2].off = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002920C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00292510);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BindParticleFxAssets);

/*
 * g_uiTextureCache (0x1B96C0) — per-record GS-upload descriptor, 0x10 bytes.
 * BuildUiTextureDescriptors parses the loaded WAD UI/HUD descriptor table
 * (stride 0x10: width, height, signed dimA, signed dimB) into these records:
 *   +0x0..0x7 zeroed (the cached TEX0 register pair, filled on first upload)
 *   +0x8  s16  height >> 4   (texels in GS units)
 *   +0xA  s16  width  >> 4
 *   +0xC  u8   Log2Floor(|dimA|)   (TEX0 TW field)
 *   +0xD  u8   Log2Floor(|dimB|)   (TEX0 TH field)
 *   +0xE  s16  PSM: 0x13 (PSMT8) when dimA >= 0, else 0x14 (PSMT4)
 */
#ifdef TARGET_NATIVE
typedef struct UiTextureRecord {
    u8  tex0[8];   /* +0x0  cached TEX0 pair, zeroed here */
    s16 heightHi;  /* +0x8  height >> 4 */
    s16 widthHi;   /* +0xA  width  >> 4 */
    u8  log2W;     /* +0xC  Log2Floor(|dimA|) */
    u8  log2H;     /* +0xD  Log2Floor(|dimB|) */
    s16 psm;       /* +0xE  0x13 / 0x14 */
} UiTextureRecord;
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(UiTextureRecord) == 0x10, "UiTextureRecord 0x10");
#endif
extern UiTextureRecord g_uiTextureCache[];
extern s32 g_uiTextureCount;
extern s32 Log2Floor(s32 v);
extern s32 func_002835E0(s32 v); /* abs(s32) */
#endif

/* BuildUiTextureDescriptors(descTable, count) — parse the loaded WAD UI/HUD
 * texture descriptor table (stride 0x10: width, height, signed dimA, signed
 * dimB) into g_uiTextureCache GS-upload records and set g_uiTextureCount. Each
 * descriptor yields one record; the signed dims drive the PSM (0x13 vs 0x14)
 * and TW/TH (Log2Floor of the magnitude). Called per level by
 * LoadLevelAndInitHealth and by InitLoadingSceneSystem. The matching build
 * keeps the asm (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00292650);
#else
void func_00292650(s32 *descTable, s32 count) {
    s32 i;
    g_uiTextureCount = 0;
    for (i = 0; i < count; i++) {
        s32 width  = *descTable++;
        UiTextureRecord *rec = &g_uiTextureCache[g_uiTextureCount];
        s32 height = *descTable++;
        s32 dimA   = *descTable++;
        s32 dimB   = *descTable++;

        rec->widthHi  = (s16)(width >> 4);
        rec->heightHi = (s16)(height >> 4);
        if (dimA >= 0) {
            rec->psm = 0x13;
        } else {
            rec->psm = 0x14;
            dimA = func_002835E0(dimA);
            dimB = func_002835E0(dimB);
        }
        rec->log2W = (u8)Log2Floor(dimA);
        rec = &g_uiTextureCache[g_uiTextureCount];
        rec->log2H = (u8)Log2Floor(dimB);
        rec = &g_uiTextureCache[g_uiTextureCount];
        *(s64 *)rec->tex0 = 0;
        g_uiTextureCount++;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BindSkyData);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadPlayerDisplayTextures);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BindPlayerDisplayModel);

/*
 * LoadPlayerDisplayModel(variant) — load the armor-variant player display model
 * into the dedicated buffer (g_playerModelBufferBase, 0x1F28000) and bind it.
 * Loads the variant's textures, binds the model, fixes up the loaded header
 * (func_00293D68) so g_mobyClassHeaders[0] points at the rebased buffer header,
 * then records the now-loaded armor variant in g_loadedArmorVariant (compared
 * against g_bEquippedArmor at level exit to trigger a reload). Callers:
 * ExitVendorMenu, RefreshVendorSelection, UpdateCheatMenuInput.
 *
 * WALL: saves s0+ra across three jal sites — the pinned 2.9-ee-991111 cc1
 * reserves a 0x20 frame (16-byte save slots) where the original's later cc1
 * packs the two 8-byte slots into a 0x10 frame; it also colours the
 * g_mobyClassHeaders / g_playerModelBufferBase loads into $4/$3 vs our $4/$5,
 * shuffling the jal-delay-slot load. Body byte-identical apart from frame size +
 * load order; kept as the portable #else body. */
#ifdef TARGET_NATIVE
extern void *g_mobyClassHeaders[];        /* 0x1CDB00 loaded header ptr per slot */
extern u8   *g_playerModelBufferBase;     /* 0x1BAEB8 */
extern s32   g_loadedArmorVariant;        /* 0x1A7290 */
extern void  LoadPlayerDisplayTextures(s32 variant);
extern void  BindPlayerDisplayModel(s32 variant);
void func_00293D68(u8 *dst, u8 *src);
#endif
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadPlayerDisplayModel);
#else
void LoadPlayerDisplayModel(s32 variant) {
    LoadPlayerDisplayTextures(variant);
    BindPlayerDisplayModel(variant);
    func_00293D68((u8 *)g_mobyClassHeaders[0], g_playerModelBufferBase);
    g_loadedArmorVariant = variant;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadHeldItemDisplayModel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00292E90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadShipDisplayModel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadShipDisplayTexture);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", ParseLoadedSegment);

__asm__(".extern g_pLoadedSegment, 16");
__asm__(".extern g_pHudAssetHeader, 16");
extern u8 *g_pLoadedSegment;
extern u8 *g_pHudAssetHeader;
extern void DecompressWad(void *src, void *dest);
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; save-layout wall (saves
 * s0+ra -> pinned cc1 reserves a 0x20 frame vs the original's 0x10). Body is
 * byte-identical apart from the frame size + ra slot offset. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002933D0);
#else
/*
 * func_002933D0 (DecompressHudBankWad) — decompress one HUD-asset-slot WAD chunk
 * from the loaded segment into a caller-supplied destination buffer, then clear
 * the slot's reloc-status word.
 *   slot — HUD asset slot index.
 *   dest — destination buffer pointer. It is aligned UP to 16 bytes and used as
 *          DecompressWad's output ($5); a null result skips the decompress.
 * The compressed chunk's segment-relative offset lives at g_pLoadedSegment +
 * slot*8 + 0x20; src = segment_base + that offset.
 *
 * IMPORTANT: DecompressWad is DecompressWad(src, dest) — it writes the
 * decompressed bytes through its second arg ($5, the write cursor in
 * DecompressWad.s). arg2 here is therefore the DESTINATION POINTER, not a byte
 * length; (dest+0xF)&~0xF is a 16-byte pointer alignment + null guard. (Verified
 * vs the asm and 4 sibling callers + the ParseLoadedSegment call site; the
 * earlier "byteLen" reading was wrong and made the #else drop $5 -> a wild
 * decompress write that poisoned the live 0x754000 segment.)
 */
void func_002933D0(s32 slot, u8 *dest) {
    dest = (u8 *)(((s32)dest + 0xF) & 0xFFFFFFF0);
    if (dest != 0) {
        u8 *seg = g_pLoadedSegment;
        DecompressWad(*(s32 *)(seg + slot * 8 + 0x20) + seg, dest);
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

/*
 * func_00293B10(table, idx) — relocate the embedded pointers of a freshly
 * loaded class chunk to absolute addresses. `table` is a base-pointer array at
 * +0x48; entry idx is the chunk base `chunk`. Field +0x14 holds a chunk-relative
 * pointer (rebased to chunk + value when non-zero), +0x10 is a byte count, and
 * +0x1C[count] is an array of chunk-relative pointers each rebased to chunk +
 * value. This converts the stored file-relative offsets into live pointers.
 *
 * WALL: instruction-for-instruction identical EXCEPT the loop-counter zero-init
 * the original's later cc1 emits as 64-bit `daddu $5,$0,$0` which the pinned
 * 2.9-ee-991111 cc1 lowers to 32-bit `move $5,$0`. Single-instruction version
 * delta; kept as the portable #else.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00293B10);
#else
void func_00293B10(s32 *table, s32 idx) {
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

/*
 * func_00293B68(groups, instMode, idMap, groupCount) — walk a table of
 * groupCount group records (stride 0x10) and, for each, emit a sub-set of its
 * instances (stride 0x40). Per group: word +0x4 packs {hi:hi16, total:lo16};
 * the walked span starts at base(+0x0) + (total-hi)*0x10 and the inner loop
 * counter steps by 4 while it is < hi (so it processes every 4th of `hi` units
 * — one instance per step, advancing the instance pointer by 0x40 each time).
 * The lo16 total is written back (the high half is consumed). For each
 * instance, its material id (word +0x20) is, when non-negative, remapped
 * through idMap (byte table); a negative id passes through. Each instance is
 * then dispatched to func_00293438 (when instMode != 0, with the material block
 * arg = instMode + matId*0x10 — instMode doubles as the material-block base
 * pointer) or func_00293760 (when instMode == 0). The matching build keeps the
 * asm (save-layout wall). */
#ifdef TARGET_NATIVE
extern void func_00293438(void *inst, s32 matBlock, s32 a2, s32 a3, s32 a4, s32 a5, s32 matId);
extern void func_00293760(void *inst, s32 a1, s32 a2, s32 a3, s32 a4, s32 matId);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00293B68);
#else
void func_00293B68(u8 *groups, s32 instMode, u8 *idMap, s32 groupCount) {
    s32 g;
    for (g = 0; g < groupCount; g++) {
        s32 packed   = *(s32 *)(groups + 4);
        u8 *base     = *(u8 **)(groups + 0);
        s32 hi       = packed >> 16;
        s32 total    = packed & 0xFFFF;
        u8 *inst     = base + ((total - hi) << 4);
        s32 k;
        *(s32 *)(groups + 4) = total;
        for (k = 0; k < hi; k += 4) {
            s32 matId = *(s32 *)(inst + 0x20);
            if (matId >= 0) {
                matId = idMap[matId];
            }
            if (instMode != 0) {
                func_00293438(inst, instMode + (matId << 4),
                              *(s32 *)(inst + 0), *(s32 *)(inst + 4),
                              *(s32 *)(inst + 0x10), *(s32 *)(inst + 0x14), matId);
            } else {
                func_00293760(inst, *(s32 *)(inst + 0), *(s32 *)(inst + 4),
                              *(s32 *)(inst + 0x10), *(s32 *)(inst + 0x14), matId);
            }
            inst += 0x40;
        }
        groups += 0x10;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", RelocateMobyClassChunk);

/*
 * func_00293D68(dst, src) — fix up a freshly-loaded display-model header in
 * place. Copies the 4-byte tag/flags block (src[0..3] -> dst[4..7]), then
 * rebases the two embedded self-relative offsets (at src+4 and src+8) to
 * absolute pointers into the loaded buffer: dst[0] = src + src[4] (the data
 * block) and dst[0x20] = src + src[8] (the secondary block). Pure leaf, no
 * frame; the caller (LoadPlayerDisplayModel) passes the bound class-header dst
 * and the buffer-resident header src.
 */
void func_00293D68(u8 *dst, u8 *src) {
    dst[4] = src[0];
    dst[5] = src[1];
    dst[6] = src[2];
    dst[7] = src[3];
    *(s32 *)(dst + 0x00) = (s32)(src + *(s32 *)(src + 4));
    *(s32 *)(dst + 0x20) = (s32)(src + *(s32 *)(src + 8));
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", FixupMobyClassHeader);

/*
 * Moby-class slot registry tables (one entry per loaded class slot):
 *   g_mobyClassSlotRemap  (0x1CE460) u8[0x2000]  classId -> slot, 0xFF = unloaded
 *   g_mobyClassSlotToId   (0x1CE280) s16[]        slot -> classId (read signed)
 *   g_mobyClassHeaders    (0x1CDB00) void*[]      loaded header ptr per slot
 *   g_mobyClassDataSizes  (0x1D0D80) u32[]        per-slot data size
 *   g_mobyClassCount      (0x1B1AC0) s32          next free header-class slot
 *   g_mobyClassCountNoHeader (0x1B1AC4) s32       headerless-class slot counter
 */
#ifdef TARGET_NATIVE
extern u8    g_mobyClassSlotRemap[];
extern s16   g_mobyClassSlotToId[];
extern void *g_mobyClassHeaders[];
extern u32   g_mobyClassDataSizes[];
extern s32   g_mobyClassCount;
extern s32   g_mobyClassCountNoHeader;
extern void  BindMobyClassUpdateFunc(s32 classId, s32 headerless);
extern void  FixupMobyClassHeader(void *hdr, s32 arg2, s32 arg3, s32 classId);
#endif

/* RegisterMobyClass(hdr, arg2, arg3, classId) — assign a class slot to classId
 * and fill the registry tables. With a null header it takes a slot from
 * g_mobyClassCountNoHeader and only records the remap byte (the headerless
 * update fn is bound). With a real header it takes the next g_mobyClassCount
 * slot, records remap/reverse-map/header/data-size (data size = header byte
 * +0x2D << 10, or 0x100000 when that byte is 0xFF), binds the update fn, then
 * FixupMobyClassHeader rebases the header offsets. The matching build keeps the
 * asm (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", RegisterMobyClass);
#else
void RegisterMobyClass(u8 *hdr, s32 arg2, s32 arg3, s32 classId) {
    if (hdr == 0) {
        s32 slot = g_mobyClassCountNoHeader;
        BindMobyClassUpdateFunc(classId, 1);
        g_mobyClassSlotRemap[classId] = (u8)slot;
        g_mobyClassCountNoHeader = slot + 1;
    } else {
        s32 slot = g_mobyClassCount;
        g_mobyClassSlotRemap[classId] = (u8)slot;
        g_mobyClassSlotToId[slot] = (s16)classId;
        g_mobyClassHeaders[slot] = hdr;
        g_mobyClassDataSizes[slot] = (u32)hdr[0x2D] << 10;
        if (hdr[0x2D] == 0xFF) {
            g_mobyClassDataSizes[slot] = 0x100000;
        }
        BindMobyClassUpdateFunc(classId, 0);
        g_mobyClassCount = slot + 1;
        FixupMobyClassHeader(hdr, arg2, arg3, classId);
    }
    __asm__ __volatile__("");
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294268);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", StartFrontendSegmentLoad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294308);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", UpdateLevelStagingMachine);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294550);

/* StreamSceneSegment: kick the streaming load of scene sub-segment `idx`. The
 * scene descriptor at g_cameraSlotActive+0x990 holds the active level's base
 * index (+0x30) and load-buffer handle (+0x70). The per-segment LBN table sits at
 * g_discToc+0x6348 + base*0x14C; segment `idx` spans [toc[idx], toc[idx+1]). When
 * that span is non-empty, start the file read (dest, toc[idx]+baseLbn, sectors)
 * and pump one dialog-voice service pass. Always returns 1. */
extern u8  g_cameraSlotActive[];
extern s32 g_discToc[];
extern s32 StartFileLoad(s32 dest, s32 lbn, s32 sectors);
extern void PumpDialogVoiceSystem(s32 blocking);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", StreamSceneSegment);
#else
s32 StreamSceneSegment(s32 idx) {
    s32 *desc = (s32 *)&g_cameraSlotActive[0x990];
    s32 base = desc[0x30 / 4];
    s32 *toc = (s32 *)((u8 *)g_discToc + 0x6348 + base * 0x14C);
    s32 thisOff = toc[idx];
    s32 sectors = toc[idx + 1] - thisOff;

    if (sectors > 0) {
        StartFileLoad(desc[0x70 / 4], thisOff + g_discToc[0x6314 / 4], sectors);
        PumpDialogVoiceSystem(0);
    }
    return 1;
}
/* byte-walled 71%: GPR coloring + schedule (the descriptor/dest loads and the
 * toc-base register assignment differ from the original's). Correct C kept as the
 * portable #else; cmp-oracle'd (StartFileLoad/PumpDialogVoiceSystem mocked). */
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BindSceneChunk);

/*
 * LoadGlobalDialogScene(sceneIndex, mode) — stream a GLOBAL cinematic/dialog
 * scene chunk and build its sub-chunk pointer table. The g_globalSceneToc lives
 * inside g_discToc: per-scene {lbnOffset@+0x3E28, sectorCount@+0x3E2C} at
 * stride 8, plus the shared base LBN at +0x3E24 (g_sceneWadBaseLbn). The scene
 * descriptor block is g_cameraSlotActive+0x990: +0x70 = load buffer ptr
 * (g_pSceneLoadBuffer), +0x30 = current scene index, +0x74[] = up to 0x46
 * sub-chunk pointers (each sub-chunk starts at buffer + entry[0] + 0x800,
 * advancing through the loaded header entries until a zero entry[1]).
 *
 * mode 0 just records the scene index; nonzero additionally pumps the dialog
 * voice and fades to black before kicking. The voice pump runs once more
 * (blocking) after the load is requested. The matching build keeps the asm
 * (save-layout wall). */
#ifdef TARGET_NATIVE
extern s32  g_discToc[];          /* 0x14B540 master disc asset directory */
extern u8   g_cameraSlotActive[]; /* 0x1B7E30 (scene desc block at +0x990) */
extern s32  StartFileLoad(s32 dest, s32 lbn, s32 sectors);
extern void FadeOutToBlackBlocking(s32 frames);
/* PumpDialogVoiceSystem declared via the cmp mock / matched build extern. */
extern void PumpDialogVoiceSystem(s32 blocking);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadGlobalDialogScene);
#else
void LoadGlobalDialogScene(s32 sceneIndex, s32 mode) {
    u8 *toc  = (u8 *)g_discToc;
    s32 *desc = (s32 *)&g_cameraSlotActive[0x990];
    s32 lbnOff   = *(s32 *)(toc + sceneIndex * 8 + 0x3E28);
    s32 baseLbn  = *(s32 *)(toc + 0x3E24);
    s32 sectors  = *(s32 *)(toc + sceneIndex * 8 + 0x3E2C);
    s32 *entry;
    s32 i;

    StartFileLoad(desc[0x70 / 4], lbnOff + baseLbn, sectors);
    if (mode != 0) {
        PumpDialogVoiceSystem(0);
        FadeOutToBlackBlocking(mode);
    }
    desc[0x30 / 4] = sceneIndex;

    PumpDialogVoiceSystem(1);
    entry = (s32 *)desc[0x70 / 4];
    if (entry[1] == 0) {
        return;
    }
    {
        s32 entryWord0 = entry[0];   /* ofs source carried into the loop top */
        i = 0;
        for (;;) {
            s32 ofs;
            entry += 2;                       /* advance to the next entry */
            ofs = entryWord0 + 0x800;
            desc[0x74 / 4 + i] = desc[0x70 / 4] + ofs;
            i++;
            if (i >= 0x46) {
                break;
            }
            if (entry[1] == 0) {
                break;
            }
            entryWord0 = entry[0];
        }
    }
}
#endif

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

/* func_00294970: reset the streaming in-flight slot table. Word-clear the whole
 * table (g_respawnPlayerYaw+0x48, 0x35840 bytes), then arm the sentinels: the
 * s16 at +0x14 and the three in-flight request-list words at +0x34/+0x38/+0x3C
 * all set to -1 (empty). */
extern void FillMemory32(void *dst, u32 val, s32 len);
extern s32 g_respawnPlayerYaw[];
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294970);
#else
void func_00294970(void) {
    s32 *rec = &g_respawnPlayerYaw[0x12];   /* +0x48 */
    s32 *p;
    s32 i;

    FillMemory32(rec, 0, 0x35840);
    *(s16 *)((u8 *)rec + 0x14) = -1;
    p = (s32 *)((u8 *)rec + 0x3C);
    for (i = 2; i >= 0; i--) {
        *p = -1;
        p--;
    }
}
/* byte-walled at 95%: 2 callee-saves land at 16-byte slots (this cc1) vs the
 * original's 8-byte packing (frame 0x20 vs 0x10) + the trailing loop delay-slot
 * schedule. Correct C kept as the portable #else; cmp-oracle'd (mock FillMemory32). */
#endif

/*
 * func_002949E0(rec, enable) — commit a pending streaming-slot record. When
 * enable is set and the record's slot index rec[1] is non-negative, stores the
 * record's class id rec[0] into the per-slot in-flight table
 * (g_respawnPlayerYaw+0x48, field +0x34, stride 4 by slot). If the slot index
 * also equals the currently-armed slot D_1A933C, that latch is toggled to
 * rec[1] ^ 1. The record's word rec[0] is ALWAYS stamped -1 on return (the latch
 * toggle is the only externally visible effect of the equal case). The disabled /
 * negative-slot path just stamps rec[0] = -1. (Verified bit-exact against the asm
 * on real R5900 via cmp-oracle: the equal branch sets D_1A933C then falls into
 * the shared `result = -1` tail, so rec[0] is never the toggled value.)
 *
 * ENGINE-2.96 MATCH under per-function sched-ON (Validation A confirmed): the
 * body is byte-exact with the engine cc1 + sched-ON (`-fno-strict-aliasing
 * -fno-builtin`, no `-fno-schedule-insns`) + move_fixup for the daddu-vs-move.
 * TU CAVEAT: NOT co-committable with sched-OFF siblings in 191238 — a TU compiles
 * with ONE scheduler setting. Provable per-fn; commit only if 191238 goes
 * sched-ON-uniform. Under the pinned 2.9 cc1 (sched-OFF) it is instruction-
 * identical EXCEPT the pointer-arg copy emitted as 64-bit `daddu $6,$4,$0` vs the
 * 2.9 32-bit `move` (a single-instruction version delta).
 */
extern s32 g_respawnPlayerYaw[];
extern s32 D_1A933C;
#if !defined(TARGET_NATIVE) && !defined(MATCH_func_002949E0)
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002949E0);
#else
void func_002949E0(s32 *rec, s32 enable) {
    if (enable != 0 && rec[1] >= 0) {
        g_respawnPlayerYaw[0x1F + rec[1]] = rec[0];   /* +0x48 + slot*4 + 0x34 */
        if (rec[1] == D_1A933C) {
            D_1A933C = rec[1] ^ 1;
        }
    }
    rec[0] = -1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294A30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294B50);

/*
 * func_00294C48(classId, slot) — request the gadget moby-class load for `slot`.
 * Looks classId up in the gadget-class TOC (g_discToc+0x4B40, stride 5 ints, up
 * to 0x30 entries). If absent (idx == 0x30) it does nothing. Otherwise, unless
 * the per-slot in-flight record (g_respawnPlayerYaw+0x48 + slot, field +0x34)
 * already equals the found index, it kicks the class load via func_00294B50 with
 * the per-slot destination buffer (slot*0xC800 + g_respawnPlayerYaw+0x88).
 *
 * WALL: instruction-for-instruction identical to the original EXCEPT the
 * register-to-register copies the original's later cc1 emits as 64-bit `daddu
 * $r,$0,$0`/`daddu $6,$4,$0` (zero/arg-move idiom) which the pinned 2.9-ee-991111
 * cc1 lowers to 32-bit `move`/`addu`. Not source-controllable; kept as the
 * portable #else (verified op-for-op, the only deltas are addu<->daddu).
 */
extern s32 g_discToc[];
extern s32 g_respawnPlayerYaw[];
extern void func_00294B50(s32 idx, s32 slot, void *dest, s32 a3);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294C48);
#else
void func_00294C48(s32 classId, s32 slot) {
    s32 *toc = g_discToc;
    s32 idx = 0;
    if (toc[0x12D0] != classId) {            /* g_discToc + 0x4B40 */
        s32 *p = toc + 0x12D0;
        for (idx = 1; idx < 0x30; idx++) {
            p += 5;
            if (p[0] == classId) {
                break;
            }
        }
    }
    if (idx != 0x30) {
        s32 *rec = &g_respawnPlayerYaw[0x12] + slot;   /* g_respawnPlayerYaw+0x48 */
        if (rec[0xD] != idx) {                          /* field +0x34 */
            void *dest = (void *)(slot * 0xC800 + (s32)&g_respawnPlayerYaw[0x22]);
            func_00294B50(idx, slot, dest, 0);          /* +0x88 == [0x12]+0x40 */
        }
    }
    __asm__ __volatile__("");
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294CD0);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294E98);
#else
/*
 * func_00294E98(a, b) — service the dialog-voice stream around a blocking
 * gadget-class-load operation: pump the voice system, run the load/lookup
 * (func_00294C48 — the gadget-class TOC walk), then pump again so streaming
 * audio is serviced on both sides of the (potentially blocking) call. Mirrors
 * StartFileLoadPumpingVoice's pump/work/pump shape.
 *
 * WALL: three jal sites with two callee-saved args (a in s1, b in s0) — the
 * pinned 2.9-ee-991111 cc1 reserves a 0x20 frame and 16-byte save slots where
 * the original's later cc1 packs them; the save-layout wall. Kept as the
 * portable #else body.
 */
extern void PumpDialogVoiceSystem(s32 blocking);
extern void func_00294C48(s32 a, s32 b);
void func_00294E98(s32 a, s32 b) {
    PumpDialogVoiceSystem(1);
    func_00294C48(a, b);
    PumpDialogVoiceSystem(1);
    __asm__ __volatile__("");
}
#endif

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
    __asm__ __volatile__("");
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

/*
 * MapIsLevelRevealed (0x295778) — returns the per-level "map discovered" bit
 * (0/1). Indexes the revealed-area bitmap as byte level/8, bit level%8 (signed
 * div/mod via the shift-with-bias idiom). The (always-true) bounds guard yields 0
 * for the degenerate case. Defined HERE in address order (between func_002956F8
 * @0x2956F8 and MapInit @0x2957C0), NOT at the file top.
 */
s32 MapIsLevelRevealed(s32 level) {
    s32 byteIndex = level / 8;
    s32 bit = level - byteIndex * 8;
    s32 result = 0;
    if ((u32)bit < 8) {
        result = (D_1395B8[0xA7 + byteIndex] >> bit) & 1;
    }
    return result;
}

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

/* func_00296038 (MapMoveCacheSlot) — relocate a galactic-map cache slot's
 * contents src -> dst; the portable #else body lives in the galactic-map cache
 * slice below (after the MapCache struct it depends on). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00296038);
#endif

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
/* MapCache type is shared by the matched MapFindCacheSlot body and the
 * TARGET_NATIVE #else bodies, so it lives outside the TARGET_NATIVE guard. */
typedef struct MapCache {
    u8  _pad00[0x24];
    s32 available;         /* +0x24  */
    u8  _pad28[0x230 - 0x28];
    s32 currentLevel;      /* +0x230 */
    s32 activeSlot;        /* +0x234 */
    u8  _pad238[0x288 - 0x238];
    s32 slotState[5];      /* +0x288  per-slot pixel-data buffer pointer (0 == empty) */
    s32 slotLevelId[5];    /* +0x29C */
    s32 lockedSlot;        /* +0x2B0  slot index to leave untouched when evicting */
    s32 slotPixelCount[5]; /* +0x2B4  per-slot qword count of pixel data (CopyQwords len) */
} MapCache;
extern MapCache g_mapVertexData;     /* 0x1C4F20 map cache / vertex-data base */

#ifdef TARGET_NATIVE
/* Offset checks only on a C11+ host (ee-gcc 2.9 used by the EE-backend suite
 * predates _Static_assert). */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(__builtin_offsetof(MapCache, available)    == 0x24,  "MapCache.available");
_Static_assert(__builtin_offsetof(MapCache, currentLevel) == 0x230, "MapCache.currentLevel");
_Static_assert(__builtin_offsetof(MapCache, activeSlot)   == 0x234, "MapCache.activeSlot");
_Static_assert(__builtin_offsetof(MapCache, slotState)    == 0x288, "MapCache.slotState");
_Static_assert(__builtin_offsetof(MapCache, slotLevelId)  == 0x29C, "MapCache.slotLevelId");
_Static_assert(__builtin_offsetof(MapCache, lockedSlot)   == 0x2B0, "MapCache.lockedSlot");
_Static_assert(__builtin_offsetof(MapCache, slotPixelCount) == 0x2B4, "MapCache.slotPixelCount");
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

/*
 * func_00296038(dst, src) — MapMoveCacheSlot: relocate a galactic-map cache
 * slot's contents from `src` to `dst`. Copies the pixel-data buffer
 * (slotState[src] -> slotState[dst], slotPixelCount[src] qwords via CopyQwords),
 * carries the slot's level id and pixel-count across, and frees the source slot
 * (slotLevelId[src] = -1). Used by the cache-slot allocator/compactor.
 *
 * WALL: the pinned cc1 folds the three parallel-array base addresses
 * (&g_mapVertexData + 0x288/0x29C/0x2B4) into single relocs and colours the
 * indices differently from the original's later cc1. Kept as the portable
 * #else body (placed here so it follows the MapCache type it reads).
 */
extern void CopyQwords(void *dst, const void *src, s32 nbytes);
void func_00296038(s32 dst, s32 src) {
    CopyQwords((void *)g_mapCache.slotState[dst],
               (const void *)g_mapCache.slotState[src],
               g_mapCache.slotPixelCount[src] << 4);
    g_mapCache.slotLevelId[dst]    = g_mapCache.slotLevelId[src];
    g_mapCache.slotPixelCount[dst] = g_mapCache.slotPixelCount[src];
    g_mapCache.slotLevelId[src]    = -1;
}
#endif

/* MapFindCacheSlot(levelAndFlag) (0x2960D8): scan the 5 map cache slots for an
 * occupied slot (slotState != 0) holding this level id. Returns the slot index,
 * or -1. Defined HERE in address order (before MapDataExistsForLevel @0x296120),
 * NOT after it.
 *
 * The matched build walks a single moving pointer `id` that starts at
 * &slotLevelId[0] (g_mapVertexData + 0x29C); slotState[i] is reached as id[-5]
 * (0x29C - 0x14 == 0x288). Materializing &g_mapVertexData as a base pointer and
 * adding 0x29C separately is what keeps cc1 from folding the two into one `la`
 * reloc — the shape the original was built with. Byte-exact USA + EU. */
s32 MapFindCacheSlot(s32 levelAndFlag) {
    s32 *base = (s32 *)&g_mapVertexData;
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

/* MapDataExistsForLevel(levelAndFlag): does map data exist for the given level?
 * The 0x100 flag bit selects the primary map-data TOC when set, the secondary
 * when clear; the low byte is the level. Reads the per-level sector count from
 * g_discToc and returns 1 if > 0.
 *
 * WALL (register coloring, best 73.2%): the control-flow shape is reproduced
 * exactly with `if (levelAndFlag & 0x100) return 0 < g_discToc[(levelAndFlag &
 * 0xFF)*2 + 0x14F4/4]; else ...0x15D4/4` — flag tested first, low-byte mask in
 * the branch delay slot, the `lui %hi(g_discToc)`/`addiu`/`sll`/`addu` address
 * arithmetic duplicated (NOT CSE'd) in both arms. The ONLY residual delta is
 * the pinned cc1's register assignment: the original reuses $2 for the dead
 * flag reg as the level mask and lands the loaded word in $4 (the dead arg
 * reg); our cc1 picks v1/v0 instead. Six source phrasings (inline, hoisted
 * level, pointer-cast, inverted arms) all hold the structure but none flips the
 * coloring. Logic byte-faithful; kept as the portable #else body, cmp-oracle
 * validated (cmp_191238_mapdata). (Earlier note blamed beql vs beqz — wrong:
 * both original and our build emit beqz; the real wall is allocation order.) */
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

/* func_00296490: NOT trivially-dead. The glabel is an 8-byte orphan teardown
 * (addiu $sp,+0x20; nop) glued onto a REAL interior body at alabel func_00296498
 * (a bit-blit loop ending jr $31). It can't be carved as standalone C (the orphan
 * prefix + un-separately-labeled real loop), so it stays INCLUDE_ASM - but the
 * loop under it is genuine code, not padding, if anyone revisits the carve. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00296490);

/* MapSetCurrentLevel(level): set the galactic-map current level and refresh the
 * availability flag. Stores `level` into g_mapCache.currentLevel then calls
 * MapUpdateLevelAvailability.
 *
 * WALL (66.9%): the level store must land in the jal-MapUpdateLevelAvailability
 * delay slot; the pinned cc1 either tail-calls (when MapUpdateLevelAvailability
 * is visible in-unit) or, with the empty-asm tail-call guard, emits the store in
 * straight-line code and a `nop` in the delay slot — it will not sink the
 * independent store into the slot the way the original's later cc1 did. Logic
 * exact; kept as the #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapSetCurrentLevel);
#else
void MapSetCurrentLevel(s32 level) {
    g_mapCache.currentLevel = level;
    MapUpdateLevelAvailability();
    __asm__ __volatile__("");
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

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapBuildBitmap);
#else
/*
 * MapBuildBitmap(dst, src, arg3) — dispatch the map-outline bitmap fill. After
 * a (no-op) prep call and only when map data is loaded (g_mapHasData != 0),
 * route by the source descriptor's low flag bit: bit0 set selects the packed
 * path func_002980D8(dst, src, arg3); bit0 clear selects the plain path
 * func_00298308(dst, src).
 *
 * WALL: two callee-saves (src, arg3) across the gate + the branch-likely
 * (beql) early-out on g_mapHasData that the pinned cc1 does not reproduce. Kept
 * as the portable #else body.
 */
extern s32  g_mapHasData;
extern void func_00298AA0(void);
extern void func_002980D8(void *dst, u8 *src, s32 arg3);
extern void func_00298308(void *dst, u8 *src);
void MapBuildBitmap(void *dst, u8 *src, s32 arg3) {
    func_00298AA0();
    if (g_mapHasData != 0) {
        if ((src[0] & 1) != 0) {
            func_002980D8(dst, src, arg3);
        } else {
            func_00298308(dst, src);
        }
    }
    __asm__ __volatile__("");
}
#endif

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
extern s32 D_1A9428;      /* gp-relative UI-slot base */
extern u8  D_255E50[];    /* per-index UI element table (0x10 stride) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002988C8);
#else
void *func_002988C8(s32 idx) {
    if (g_playerProgress == 0x14) {
        s32 n = *(s32 *)(g_pointLights + 0x2400);
        return (u8 *)&D_1A9428 + ((n % 2) << 4);
    }
    return &D_255E50[idx * 0x10];
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298918);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002989F8);

/* func_00298AA0: empty/no-op leaf (jr ra; nop). */
void func_00298AA0(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298AA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298F20);

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

/* func_002919C0 — NOT a real function entry: two `addiu $29,$29,0x10`
 * stack-restore words with no `jr $31`, i.e. a shared epilogue fragment that
 * splat glabel'd from a pair of branch/jump targets into func_002919A0's tail.
 * Not portable-C expressible (no callable body); left as INCLUDE_ASM. */
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
/* GS A+D reg-write: the DATA is a 64-bit register value. Widen it to u64 for the
 * native/#else build (prevents silent truncation of bits >=32); matching-build
 * decl kept verbatim (byte-neutral). */
#ifdef TARGET_NATIVE
extern void AppendGsRegPacket(s32 reg, u64 val);
#else
extern void AppendGsRegPacket(s32 reg, s32 val);
#endif
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

/*
 * func_00291D28 — per-frame refresh of the directional + point light set.
 * Seeds the directional-light matrix at g_dirLightMatrices+0x340 from the default
 * D_1A9190, then derives its direction from the camera yaw: angle =
 * WrapAnglePiSum(g_cameraRot[2], -0.8); +0x350 = cos(angle)*0.866, +0x354 =
 * sin(angle)*0.866, +0x358 = -0.5, +0x35C = 0.
 * Then walks the 8 point-light request slots (g_pointLights: light data +0x10
 * stride 0x20, request record +0x110 stride 0x30). A slot with type 0 is skipped.
 * Otherwise, if the slot is relevant (func_002837F8(light, req) > 1.0) OR its
 * radius moved by more than 1.0 (|light[0xC] - req[0xC]|), the light data is
 * copied into the request record and, by request type, dispatched: type 1 builds
 * a relight request (func_00291EB0) and is promoted to type 2; type 2 resets +
 * re-dispatches (func_00291FC8).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291D28);
#else
extern u8  g_dirLightMatrices[];       /* 0x1C26C0 - 0x40-stride light matrices */
extern u8  g_pointLights[];            /* 0x1C2AC0 - stride 0x20 */
extern f32 g_cameraRot[];              /* 0x1B52D0 - camera euler angles */
extern u8  D_1A9190[];                 /* default directional-matrix row (16B) */
extern f32 WrapAnglePiSum(f32 a, f32 b);
extern f32 func_00283B30(f32 x);       /* cos */
extern f32 func_00283B48(f32 x);       /* sin */
extern f32 func_002837F8(void *light, void *req);
extern f32 GetFloatAbs(f32 x);         /* fabsf */
extern void func_00291EB0(s32 index);
extern void func_00291FC8(void *arg);

void func_00291D28(void) {
    u8 *dir = g_dirLightMatrices;
    f32 angle;
    s32 i;

    *(u64 *)(dir + 0x340) = *(u64 *)D_1A9190;          /* seed matrix row (lq/sq) */
    *(u64 *)(dir + 0x348) = *(u64 *)(D_1A9190 + 8);

    angle = WrapAnglePiSum(g_cameraRot[2], -0.8f);
    *(f32 *)(dir + 0x350) = func_00283B30(angle) * 0.866f;
    *(f32 *)(dir + 0x354) = func_00283B48(angle) * 0.866f;
    *(f32 *)(dir + 0x358) = -0.5f;
    *(s32 *)(dir + 0x35C) = 0;

    for (i = 0; i < 8; i++) {
        u8 *light = g_pointLights + 0x10 + i * 0x20;
        u8 *req = g_pointLights + 0x120 + i * 0x30;    /* record body; type @ -0x10 */
        s32 type;

        if (*(s32 *)(req - 0x10) == 0) {
            continue;                                   /* empty slot */
        }
        if (!(1.0f < func_002837F8(light, req)) &&
            !(1.0f < GetFloatAbs(*(f32 *)(light + 0xC) - *(f32 *)(req + 0xC)))) {
            continue;                                   /* unchanged - keep as is */
        }

        *(u64 *)req = *(u64 *)light;                     /* copy light data (lq/sq) */
        *(u64 *)(req + 8) = *(u64 *)(light + 8);

        type = *(s32 *)(req - 0x10);
        if (type == 1) {
            func_00291EB0(i);
            *(s32 *)(req - 0x10) = 2;
        } else if (type == 2) {
            func_00291FC8((void *)i);
        }
    }
}
#endif

/**
 * func_00291EB0 — build a light-relight request record for point light `index`.
 *
 * Fills the request entry (g_pointLights+0x100, stride 0x30) by running three
 * geometry builders in sequence (func_002F5DF8, func_002E4000, func_002F19D0),
 * each appending into the shared buffer from the request's cursor (+0xC) up to a
 * 0x400-byte limit and returning the advanced cursor. Records the per-stage
 * half-counts ((cursor - base) >> 1, i.e. 16-bit element counts) into the entry
 * halfwords: +0x2/+0x4 = stage-1 count, +0x6 = stage-2 delta, +0x8 = stage-2
 * total, +0xA = stage-3 delta. Stops early if any builder hits the limit.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00291EB0);
#else
extern u8 g_pointLights[];
extern s32 func_002F5DF8(s32 cursor, s32 limit, s32 index, void *src);
extern s32 func_002E4000(s32 cursor, s32 limit, s32 index, void *src);
extern s32 func_002F19D0(s32 cursor, s32 limit, s32 index, void *src);

void func_00291EB0(s32 index) {
    u8 *req = g_pointLights + 0x100 + index * 0x30;
    void *src = g_pointLights + index * 0x20 + 0x10;
    s32 limit = *(s32 *)(req + 0xC) + 0x400;
    s32 cursor, count;

    if ((u32)*(s32 *)(req + 0xC) >= (u32)limit) {
        return;
    }
    *(s16 *)(req + 0x0) = 0;

    cursor = func_002F5DF8(*(s32 *)(req + 0xC), limit, index, src);
    count = (cursor - *(s32 *)(req + 0xC)) >> 1;
    *(s16 *)(req + 0x2) = (s16)count;
    if ((u32)cursor >= (u32)limit) {
        return;
    }
    *(s16 *)(req + 0x4) = (s16)count;

    cursor = func_002E4000(cursor, limit, index, src);
    count = (cursor - *(s32 *)(req + 0xC)) >> 1;
    *(s16 *)(req + 0x6) = (s16)(count - *(u16 *)(req + 0x4));
    if ((u32)cursor >= (u32)limit) {
        return;
    }
    *(s16 *)(req + 0x8) = (s16)count;

    cursor = func_002F19D0(cursor, limit, index, src);
    count = (cursor - *(s32 *)(req + 0xC)) >> 1;
    *(s16 *)(req + 0xA) = (s16)(count - *(u16 *)(req + 0x8));
}
#endif

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
extern void func_00291EB0(s32 index);
void func_00291FC8(void *arg) {
    func_00291FF8((s32)arg);
    func_00291EB0((s32)arg);
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

/*
 * BindParticleFxAssets(hdr, texBase, texRecords, texCount) — bind a freshly
 * loaded particle-effect asset pack. First rebases the effect-def pointer table:
 * for each of hdr[0] entries (words at hdr+0x10), a nonzero self-relative offset
 * is rebased from hdr[8] onto the relocated blob (g_particleFxBlob) and stored
 * into g_particleEffectDefs; a zero entry gets the blob base. Copies the blob
 * body (func_00283460(g_particleFxBlob, hdr + hdr[8])). Then builds the particle
 * texture table: for each of texCount 4-word records at texRecords, writes an
 * 8-byte g_particleTexTable entry — word0 = ((texBase + rec[0]) << 4) + rec[1]
 * (data VRAM word addr), word1 = ((texBase + rec[2]) << 4) + Log2Floor(rec[3])
 * (CLUT addr + log2 height) — and sets g_particleTexCount. The matching build
 * keeps the asm (engine save-layout wall). */
#ifdef TARGET_NATIVE
extern u8   g_particleFxBlob[];       /* 0x1F26C0  relocated per-level blob */
extern s32  g_particleEffectDefs[];   /* 0x1F24C0  128 effect-def ptrs into blob */
extern s32  g_particleTexTable[];     /* 0x1F1EC0  8 bytes/tex: VRAM + CLUT addr */
extern s32  g_particleTexCount;       /* 0x1B1D24 */
extern void func_00283460(void *dst, void *src);
extern s32  Log2Floor(s32 x);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BindParticleFxAssets);
#else
void BindParticleFxAssets(void *hdrArg, s32 texBase, s32 *texRecords, s32 texCount) {
    u8 *hdr       = (u8 *)hdrArg;
    s32 defCount  = *(s32 *)(hdr + 0);
    s32 blobField = *(s32 *)(hdr + 8);

    if (defCount > 0) {
        s32 *entry = (s32 *)(hdr + 0x10);
        s32 *out   = g_particleEffectDefs;
        s32  delta = blobField - (s32)g_particleFxBlob;
        s32  i;
        for (i = defCount; i != 0; i--) {
            s32 off = *entry;
            *out = (off != 0) ? (off - delta) : (s32)g_particleFxBlob;
            out++;
            entry++;
        }
    }

    func_00283460(g_particleFxBlob, hdr + blobField);

    if (texCount > 0) {
        s32 *rec = texRecords;
        s32  slot;
        g_particleTexCount = 0;
        do {
            slot = g_particleTexCount;
            g_particleTexTable[slot * 2]     = ((texBase + rec[0]) << 4) + rec[1];
            g_particleTexTable[slot * 2 + 1] = ((texBase + rec[2]) << 4) + Log2Floor(rec[3]);
            g_particleTexCount = slot + 1;
            rec += 4;
        } while (slot + 1 < texCount);
    }
}
#endif

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

/* Bind + relocate a loaded sky-data blob: fix up its internal header pointers
 * (offset -> absolute by +blob base), publish it to g_pSkyData, then two passes
 * build per-entry sky-texture params (Log2Floor of dims) and relocate a second
 * pointer table. PARK (#70): the first pass overlaps a source-word-stream view
 * ($18 advancing +4 x5) with a 0x10-stride entry-array view of the same region;
 * that aliasing isn't pinned confidently enough for a faithful #else (silent-bug
 * risk on the exact field mapping). Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BindSkyData);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadPlayerDisplayTextures);

/**
 * BindPlayerDisplayModel — stage the player display model's textures and header.
 *
 * For each of g_playerTexCount entries, writes a 3-doubleword GIF/GS texture
 * register block into the display list at g_pointLights+0x2280 (stride 0x18):
 * the per-texture descriptor from g_playerTexDescriptors followed by two fixed
 * GS register values. Then relocates the player model chunk into place
 * (RelocateMobyClassChunk from g_pPlayerModelBuffer) and mirrors the first moby
 * class header's byte +0x8 into +0x9.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", BindPlayerDisplayModel);
#else
extern s32 g_playerTexCount;
extern u64 g_playerTexDescriptors[];
extern void *g_pPlayerModelBuffer;
extern void *g_mobyClassHeaders[];
extern u8 D_1A91D0[];
extern void RelocateMobyClassChunk(void *buffer, s32 slot, void *reloc);

void BindPlayerDisplayModel(void) {
    s32 count = g_playerTexCount;

    if (count > 0) {
        u64 *dst = (u64 *)(g_pointLights + 0x2280);
        u64 *desc = g_playerTexDescriptors;
        s32 i;

        for (i = 0; i < count; i++) {
            dst[0] = *desc;                     /* per-texture descriptor */
            dst[1] = 0x0000FFA0000000E0ULL;     /* fixed GS register A */
            dst[2] = 0x0040000400004000ULL;     /* fixed GS register B */
            desc++;
            dst += 3;
        }
    }
    RelocateMobyClassChunk(g_pPlayerModelBuffer, 0, D_1A91D0);
    {
        u8 *hdr = (u8 *)g_mobyClassHeaders[0];
        hdr[0x9] = hdr[0x8];
    }
}
#endif

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
extern void  BindPlayerDisplayModel(void);   /* ignores any arg (reads g_playerTexCount) */
void func_00293D68(u8 *dst, u8 *src);
#endif
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadPlayerDisplayModel);
#else
void LoadPlayerDisplayModel(s32 variant) {
    LoadPlayerDisplayTextures(variant);
    BindPlayerDisplayModel();
    func_00293D68((u8 *)g_mobyClassHeaders[0], g_playerModelBufferBase);
    g_loadedArmorVariant = variant;
}
#endif

/* Load the held-item display model+texture for `itemId` (twin of
 * LoadShipDisplayTexture): fence the DMA, kick the disc file load for the item's
 * TOC entry (stride itemId*0x10: start = +0x4FA0 biased by +0x4F04, count +0x4FA4)
 * into g_heldItemModelBufferBase, upload a 16x16 base + the loaded mip (dims from
 * the buffer header at +0x10) as two GS images, cache the packed 64-bit GS
 * texture register in g_heldItemTexDescriptor, then kick a SECOND disc load for
 * the model geometry (+0x4F98 / +0x4F9C). Engine-2.96 (save-slot walled) ->
 * faithful #else; register pack transcribed op-for-op. NEEDS-ORACLE. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadHeldItemDisplayModel);
#else
extern void WaitFrameDmaFence(s32 mode);
extern void PumpDialogVoiceSystem(s32 blocking);
extern void StartFileLoadPumpingVoice(void *dest, s32 startSector, s32 sectorCount);
extern void func_0011AEA0(s32 a);
extern s32  g_discToc[];
extern u8   g_vramTextureBase[];
extern u8  *g_heldItemModelBufferBase;    /* loaded model/texture buffer ptr */
extern u64  g_heldItemTexDescriptor;      /* cached packed GS texture register */
extern void func_126288(void *dst, s32 tbp, s32 a, s32 b, s32 c, s32 d, s32 w, s32 h);
extern void KickGifImageUpload(void *packet, void *src);
extern void WaitGsPathsIdle(s32 arg);

void LoadHeldItemDisplayModel(s32 itemId) {
    u8   packet[0x60];               /* GIF packet scratch (sp+0) */
    u8  *disc = (u8 *)g_discToc;
    s32  off  = itemId << 4;         /* itemId * 0x10 = disc TOC stride */
    u8  *buffer;
    u8  *hdr;
    s32  log2, clampW;
    s16  w, h;
    u64  reg;

    WaitFrameDmaFence(1);
    buffer = g_heldItemModelBufferBase;
    PumpDialogVoiceSystem(1);

    StartFileLoadPumpingVoice(buffer,
        *(s32 *)(disc + off + 0x4FA0) + *(s32 *)(disc + 0x4F04),
        *(s32 *)(disc + off + 0x4FA4));

    /* level 0: 16x16 base image */
    func_126288(packet, (*(s32 *)(g_vramTextureBase + 0x8) << 8) >> 16,
                1, 0, 0, 0, 0x10, 0x10);
    func_0011AEA0(0);
    KickGifImageUpload(packet, buffer + 0x30);
    WaitGsPathsIdle(0);

    /* level 1: mip sized from the loaded buffer header (+0x10) */
    hdr    = buffer + 0x10;
    log2   = Log2Floor(*(s32 *)(hdr + 0x8));
    clampW = *(s32 *)(hdr + 0x8) >> 6;
    if (clampW <= 0) {
        clampW = 1;
    }
    w = *(s16 *)(hdr + 0x8);
    h = *(s16 *)(hdr + 0xC);
    func_126288(packet, (*(s32 *)(g_vramTextureBase + 0x18) << 8) >> 16,
                (s16)clampW, 0x1B, 0, 0, w, h);
    func_0011AEA0(0);
    KickGifImageUpload(packet, buffer + 0x430);
    WaitGsPathsIdle(0);

    /* pack the 64-bit GS texture register (op-for-op from the dsll/dsra/or chain) */
    reg = (u64)((s64) * (s32 *)(g_vramTextureBase + 0x18) >> 8)
        | ((u64)clampW << 14)
        | (((u64)log2 << 26) | 0x1B00000)
        | ((u64)log2 << 30)
        | (((u64)((s64) * (s32 *)(g_vramTextureBase + 0x8) >> 8) << 37) | ((u64)0x8000 << 19))
        | ((u64)1 << 63);
    g_heldItemTexDescriptor = reg;

    /* second disc load: the model geometry */
    StartFileLoadPumpingVoice(buffer,
        *(s32 *)(disc + off + 0x4F98) + *(s32 *)(disc + 0x4F04),
        *(s32 *)(disc + off + 0x4F9C));
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00292E90);

/**
 * LoadShipDisplayModel — load the ship display model for level `index`.
 *
 * Records the index at g_levelDialogToc+0x13C8, waits for the frame DMA fence,
 * then kicks the model's disc read (StartFileLoadPumpingVoice) into
 * g_shipModelBufferBase from the level's TOC entry (g_discToc + index*8: start
 * LBN at +0x48D8 plus the base +0x3E24, sector count at +0x48DC). Sets up the
 * GS texture register block at g_pointLights+0x2280 (descriptor from
 * g_levelDialogToc+0x13E8 followed by the two fixed GS registers) and rebases
 * the loaded moby class header (FixupMobyClassHeader).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadShipDisplayModel);
#else
extern s32 g_discToc[];
extern u8 g_levelDialogToc[];
extern void *g_shipModelBufferBase;
extern u8 D_1A91E0[];
extern void WaitFrameDmaFence(s32 mode);
extern void StartFileLoadPumpingVoice(void *dest, s32 startSector, s32 sectorCount);
extern void FixupMobyClassHeader(void *hdr, s32 arg2, s32 reloc, s32 classId);

void LoadShipDisplayModel(s32 index) {
    s32 *entry = (s32 *)((u8 *)g_discToc + index * 8);

    *(s32 *)(g_levelDialogToc + 0x13B0 + 0x18) = index;
    WaitFrameDmaFence(1);
    StartFileLoadPumpingVoice(g_shipModelBufferBase,
                              entry[0x48D8 / 4] + g_discToc[0x3E24 / 4],
                              entry[0x48DC / 4]);
    {
        u64 *reg = (u64 *)(g_pointLights + 0x2280);
        reg[0] = *(u64 *)(g_levelDialogToc + 0x13B0 + 0x38);
        reg[1] = 0x0000FFA0000000E0ULL;     /* fixed GS register A */
        reg[2] = 0x0040000400004000ULL;     /* fixed GS register B */
    }
    FixupMobyClassHeader(g_shipModelBufferBase, 0, (s32)D_1A91E0, -1);
}
#endif

/* Load the ship-select display texture for `shipId`: fence the frame DMA, resolve
 * the double-buffered frame-arena slot, kick the disc file load (the shipId TOC
 * entry: start sector = g_discToc[shipId] header +0x48F0 biased by +0x3E24, count
 * +0x48F4), then upload two GS image levels (a 16x16 base + the loaded mip whose
 * dims come from the arena header) into VRAM and cache the packed 64-bit GS
 * texture register at g_levelDialogToc[0x13B0+0x38] for the draw path to load.
 * Engine-2.96 TU (prologue packs 6 saved regs at 8-byte slots, frame 0x70-class)
 * -> save-slot walled, canonical-2.9 can't byte-match; faithful #else, with the
 * trailing dsll/dsra/or register pack transcribed op-for-op. NEEDS-ORACLE. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", LoadShipDisplayTexture);
#else
extern s32  g_frameArenaFlip;             /* double-buffer index (0/1) */
extern s32  g_frameArenaBase;             /* per-frame arena base (declared later in-unit) */
extern u8   g_vramTextureBase[];          /* VRAM texture-slot descriptor (+0xC/+0x1C = level TBPs) */
extern void PumpDialogVoiceSystem(s32 blocking);   /* declared later in-unit */
extern void func_0011AEA0(s32 a);                  /* declared later in-unit */
extern void func_126288(void *dst, s32 tbp, s32 a, s32 b, s32 c, s32 d, s32 w, s32 h);
extern void KickGifImageUpload(void *packet, void *src);
extern void WaitGsPathsIdle(s32 arg);

void LoadShipDisplayTexture(s32 shipId) {
    u8    packet[0x60];               /* GIF packet scratch (sp+0) */
    u8   *base  = &g_levelDialogToc[0x13B0];
    u8   *disc  = (u8 *)g_discToc;
    void *arena;
    s32   log2, clampW;
    s16   w, h;
    u64   reg;

    *(s32 *)(base + 0x30) = shipId;
    WaitFrameDmaFence(1);
    arena = (void *)(&g_frameArenaBase)[1 - g_frameArenaFlip];
    PumpDialogVoiceSystem(1);

    StartFileLoadPumpingVoice(arena,
        *(s32 *)(disc + shipId * 8 + 0x48F0) + *(s32 *)(disc + 0x3E24),
        *(s32 *)(disc + shipId * 8 + 0x48F4));

    /* level 0: 16x16 base image */
    func_126288(packet, (*(s32 *)(g_vramTextureBase + 0xC) << 8) >> 16,
                1, 0, 0, 0, 0x10, 0x10);
    func_0011AEA0(0);
    KickGifImageUpload(packet, (u8 *)arena + 0x20);
    WaitGsPathsIdle(0);

    /* level 1: mip level sized from the loaded arena header (+0x8 w, +0xC h) */
    log2   = Log2Floor(*(s32 *)((u8 *)arena + 0x8));
    clampW = *(s32 *)((u8 *)arena + 0x8) >> 6;
    if (clampW <= 0) {
        clampW = 1;
    }
    w = *(s16 *)((u8 *)arena + 0x8);
    h = *(s16 *)((u8 *)arena + 0xC);
    func_126288(packet, (*(s32 *)(g_vramTextureBase + 0x1C) << 8) >> 16,
                (s16)clampW, 0x1B, 0, 0, w, h);
    func_0011AEA0(0);
    KickGifImageUpload(packet, (u8 *)arena + 0x420);
    WaitGsPathsIdle(0);

    /* pack the 64-bit GS texture register (op-for-op from the dsll/dsra/or chain) */
    reg = (u64)((s64) * (s32 *)(g_vramTextureBase + 0x1C) >> 8)
        | ((u64)clampW << 14)
        | (((u64)log2 << 26) | 0x1B00000)
        | ((u64)log2 << 30)
        | (((u64)((s64) * (s32 *)(g_vramTextureBase + 0xC) >> 8) << 37) | ((u64)0x8000 << 19))
        | ((u64)1 << 63);
    *(u64 *)(base + 0x38) = reg;
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", ParseLoadedSegment);
#else
extern u8 *g_pLoadedSegment;
extern u8 *g_pHudAssetHeader;
extern u8  g_memoryArenaTable[];
extern u8  g_hudMobySpawnStart[];
extern void *g_hudIconMap;
extern void *g_hudClutSlots;
extern void *g_hudTextureSlots;
extern void *DebugMalloc(s32 size, s32 arg2, void *file, s32 line);
extern void CopyQwords(void *dst, const void *src, s32 nbytes);
extern s32  UploadDataToIopRing(void *eeAddr, s32 sizeQw, s32 arg3, void *tag);
extern void func_002933D0(s32 slot, u8 *dest);
extern void func_0028BBA0(s32 a, void *b, s32 c);
extern void func_0028B8C8(s32 a, void *b);
extern void func_0011AEA0(s32 a);
extern u8 D_1A91F0[];  /* "loaders.cpp" debug __FILE__ string */
extern u8 D_1A9200[];  /* per-bank IOP-upload debug tag strings */
extern u8 D_1A9210[];
extern u8 D_1A9220[];
extern u8 D_1A9230[];
/*
 * ParseLoadedSegment (loaders.cpp @0x293138) — bind the just-loaded HUD asset
 * segment (g_pLoadedSegment): publish its rounded sub-segment sizes, make a
 * DebugMalloc'd working copy of its asset header, resolve the embedded section
 * pointers, and stage the four HUD-bank texture blocks to the IOP upload ring.
 *
 *  - Rounds the 5 sub-segment sizes at seg+0x24 (stride 8) up to a multiple of
 *    64 and stores them into the HUD-context size table g_hudMobySpawnStart+8
 *    (stride 4).
 *  - DebugMalloc(round64(seg[0x1C]), 0, "loaders.cpp", 0x305) then CopyQwords a
 *    copy of the header in from seg + seg[0x18]; publishes it as
 *    g_pHudAssetHeader (the (size,0,file,line) call proves the 4-arg retail
 *    debug-malloc signature).
 *  - Resolves the header's segment-relative section offsets to absolute:
 *    header[+4]->the +4 global slot, header[8]->g_hudIconMap,
 *    header[0xC]->g_hudClutSlots, header[0x10]->g_hudTextureSlots. iopBase =
 *    g_memoryArenaTable[0x10] + 0x60000 is the fixed IOP staging address.
 *  - Bank 0 (gate header[0x54]): decompress slot 0 into iopBase, DMA
 *    seg+seg[0x20] (round64(seg[0x24])/16 qwords) to the IOP ring tagged
 *    D_1A9200, record the GS handle at header+0x94, run func_0028BBA0.
 *  - Bank 1 (gate header[0x58]): DebugMalloc a scratch buffer sized header[0x58],
 *    decompress slot 1 into it, run func_0011AEA0(0) + func_0028B8C8; no upload.
 *  - Banks 2-4 (gates header[0x5C]/[0x60]/[0x64]): DMA seg sections
 *    0x30/0x34, 0x38/0x3C, 0x40/0x44 to the ring (tags D_1A9210/20/30),
 *    recording GS handles at header +0x9C/+0xA0/+0xA4.
 */
void ParseLoadedSegment(void) {
    u8 *seg = g_pLoadedSegment;
    u8 *header;
    u8 *iopBase;
    s32 *src;
    s32 *dst;
    s32 i;
    s32 size;
    s32 sizeQw;

    /* Round the 5 sub-segment sizes up to 64 and publish the size table. */
    src = (s32 *)(seg + 0x24);
    dst = (s32 *)(g_hudMobySpawnStart + 8);
    for (i = 4; i >= 0; i--) {
        *dst = (*src + 0x3F) & 0xFFFFFFC0;
        src += 2;
        dst += 1;
    }

    /* DebugMalloc + CopyQwords the working header copy. */
    size = (*(s32 *)(seg + 0x1C) + 0x3F) & 0xFFFFFFC0;
    header = (u8 *)DebugMalloc(size, 0, D_1A91F0, 0x305);
    CopyQwords(header, (void *)(*(s32 *)(seg + 0x18) + seg), size);
    g_pHudAssetHeader = header;

    /* Resolve the header's section pointers + the fixed IOP staging base. */
    *((u8 **)&g_pHudAssetHeader + 1) = header + *(s32 *)(header + 0x4);
    iopBase = (u8 *)(*(s32 *)(g_memoryArenaTable + 0x10) + 0x60000);
    g_hudIconMap      = header + *(s32 *)(header + 0x8);
    g_hudClutSlots    = header + *(s32 *)(header + 0xC);
    g_hudTextureSlots = header + *(s32 *)(header + 0x10);

    /* Bank 0. */
    if (*(s32 *)(header + 0x54) != 0) {
        sizeQw = ((*(s32 *)(seg + 0x24) + 0x3F) & 0xFFFFFFC0) >> 4;
        func_002933D0(0, iopBase);
        *(s32 *)(g_pHudAssetHeader + 0x94) =
            UploadDataToIopRing((void *)(*(s32 *)(seg + 0x20) + seg),
                                sizeQw, sizeQw, D_1A9200);
        func_0028BBA0(0, iopBase, 1);
    }

    /* Bank 1 (scratch decompress, no upload). */
    header = g_pHudAssetHeader;
    if (*(s32 *)(header + 0x58) != 0) {
        u8 *buf = (u8 *)DebugMalloc(*(s32 *)(header + 0x58), 0, D_1A91F0, 0x32D);
        func_002933D0(1, buf);
        func_0011AEA0(0);
        func_0028B8C8(1, buf);
    }

    /* Bank 2. */
    header = g_pHudAssetHeader;
    if (*(s32 *)(header + 0x5C) != 0) {
        sizeQw = ((*(s32 *)(seg + 0x34) + 0x3F) & 0xFFFFFFC0) >> 4;
        *(s32 *)(g_pHudAssetHeader + 0x9C) =
            UploadDataToIopRing((void *)(*(s32 *)(seg + 0x30) + seg),
                                sizeQw, sizeQw, D_1A9210);
    }

    /* Bank 3. */
    header = g_pHudAssetHeader;
    if (*(s32 *)(header + 0x60) != 0) {
        sizeQw = ((*(s32 *)(seg + 0x3C) + 0x3F) & 0xFFFFFFC0) >> 4;
        *(s32 *)(g_pHudAssetHeader + 0xA0) =
            UploadDataToIopRing((void *)(*(s32 *)(seg + 0x38) + seg),
                                sizeQw, sizeQw, D_1A9220);
    }

    /* Bank 4. */
    header = g_pHudAssetHeader;
    if (*(s32 *)(header + 0x64) != 0) {
        sizeQw = ((*(s32 *)(seg + 0x44) + 0x3F) & 0xFFFFFFC0) >> 4;
        *(s32 *)(g_pHudAssetHeader + 0xA4) =
            UploadDataToIopRing((void *)(*(s32 *)(seg + 0x40) + seg),
                                sizeQw, sizeQw, D_1A9230);
    }
}
#endif

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

/**
 * RelocateMobyClassChunk — fix up a freshly-loaded moby class chunk in place.
 *
 * The chunk header holds three section counts (bytes +0x0/+0x1/+0x2) and two
 * table offsets (+0x4, +0x8). First it rebases the pointer table at chunk+[0x4]
 * (one 0x10-byte entry per section): for each entry whose base word is below the
 * arena limit (g_memoryArenaTable+0x8), its words +0x0 and +0x8 get the chunk
 * base added. Then it walks the descriptor table at chunk+[0x8] (0x10-byte
 * stride) until a descriptor's rebased offset word (+0xC) goes negative,
 * relocating that word and translating each descriptor's 0xFF-terminated name
 * string through the `nameTable` lookup. Finally hands the pointer table to
 * func_00293B68 for group instantiation.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", RelocateMobyClassChunk);
#else
void RelocateMobyClassChunk(void *chunkArg, s32 arg2, void *nameTableArg) {
    u8 *chunk = (u8 *)chunkArg;
    u8 *nameTable = (u8 *)nameTableArg;
    s32 count = chunk[0] + chunk[1] + chunk[2];
    s32 *table1 = (s32 *)(chunk + *(s32 *)(chunk + 0x4));
    u8 *entry = chunk + *(s32 *)(chunk + 0x8);
    s32 cont;

    if (count != 0) {
        s32 *e = table1;
        s32 i;
        for (i = count; i != 0; i--) {
            if (e[0] < *(s32 *)(g_memoryArenaTable + 0x8)) {
                e[0] += (s32)chunk;
                e[2] += (s32)chunk;      /* word at +0x8 */
            }
            e = (s32 *)((u8 *)e + 0x10);
        }
    }

    do {
        s32 *offsetWord = (s32 *)(entry + 0xC);
        *offsetWord += (s32)chunk;
        if (entry[0] != 0xFF) {
            u8 *p = entry;
            while (*p != 0xFF) {
                *p = nameTable[*p];
                p++;
            }
        }
        cont = (*offsetWord >= 0);
        entry += 0x10;
    } while (cont);

    func_00293B68((u8 *)table1, arg2, nameTable, count);
}
#endif

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

/*
 * FixupMobyClassHeader(hdr, instMode, idMap, classId) — rebase a freshly-loaded
 * moby class header in place: turn every embedded self-relative offset into an
 * absolute pointer into the loaded buffer, and record derived per-slot metadata.
 *
 * When hdr[0xB] (the "special/compressed" flag) is set it first clamps the data
 * size word at hdr[0x2C] to 0x3FC00, repacks it into the byte hdr[0x2D]=size>>10
 * (zeroing 0x2C/0x2E and the flag), and stores the clamped size into
 * g_mobyClassDataSizes[slot] (slot = g_mobyClassSlotRemap[classId]).
 *
 * count = hdr[4]+hdr[5]+hdr[6] (+ the sub-count byte at hdr+hdr[0x2C]*0x10+1 when
 * hdr[0x2C] is nonzero) is the number of 0x10-byte mesh-block records at hdr[0].
 * Each record's offset words +0x0/+0x8 are rebased; for records outside the
 * middle band [hdr4+hdr5, hdr4+hdr5+hdr6) (only when the special flag was set)
 * the block at record[+8] is compacted (8 word.low16 -> 8 contiguous halfwords),
 * its +0xC size adjusted, and its tail qwords shifted down via CopyQwords.
 *
 * Then the sound-def / anim tables are rebased: single offset words +0x10/+0x14/
 * +0x18/+0x28; the +0x1C list (count in word[0], entries word[1..]); the +0x20
 * 0x10-byte descriptor list (per entry: rebase +0xC and remap its 0xFF-terminated
 * name string through idMap, until a rebased +0xC goes negative); and the +0x48
 * anim-seq pointer array (hdr[0xC] entries, each sub-record's +0x14 and its
 * sub[0x10] frame pointers at sub+0x1C). Finally, when classId >= 0 the 16-byte
 * bounds vector at idMap is copied into g_mobyClassBounds[slot], and the rebased
 * mesh table hdr[0] (when present) is handed to func_00293B68 for group
 * instantiation. The matching build keeps the asm (engine save-layout wall).
 */
#ifdef TARGET_NATIVE
extern void DebugPrintStub(const char *fmt, ...);
extern void CopyQwords(void *dst, const void *src, s32 nbytes);
extern u8   g_mobyClassSlotRemap[];   /* 0x1CE460  classId -> loaded slot */
extern u32  g_mobyClassDataSizes[];   /* 0x1D0D80  per-slot clamped data size */
extern u8   g_mobyClassBounds[];      /* 0x1D1500  16-byte bounds vec per slot */
extern char D_1A9240[];               /* debug fmt string (DebugPrintStub no-op) */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", FixupMobyClassHeader);
#else
void FixupMobyClassHeader(void *hdrArg, s32 instMode, s32 idMap, s32 classId) {
    u8 *hdr      = (u8 *)hdrArg;
    u8 *idMapPtr = (u8 *)idMap;
    s32 hasSpecial = (hdr[0xB] != 0);
    s32 count;

    if (hasSpecial) {
        s32 size = *(s32 *)(hdr + 0x2C);
        u8  slot;
        DebugPrintStub(D_1A9240, classId);          /* retail no-op */
        if (size > 0x3FC00) {
            size = 0x3FC00;
        }
        hdr[0xB]  = 0;
        hdr[0x2C] = 0;
        *(u16 *)(hdr + 0x2E) = 0;
        hdr[0x2D] = (u8)(size >> 10);
        slot = g_mobyClassSlotRemap[classId];
        g_mobyClassDataSizes[slot] = (u32)size;
    }

    count = hdr[4] + hdr[5] + hdr[6];
    if (hdr[0x2C] != 0) {
        count += *(u8 *)(hdr + hdr[0x2C] * 0x10 + 1);
    }

    {
        s32 firstOff = *(s32 *)(hdr + 0);
        if (firstOff != 0) {
            u8 *ptr = hdr + firstOff;
            *(s32 *)(hdr + 0) = (s32)ptr;
            if (count != 0) {
                s32 i;
                for (i = 0; i < count; i++) {
                    s32 abs0 = *(s32 *)(ptr + 0) + (s32)hdr;
                    s32 abs8 = *(s32 *)(ptr + 8) + (s32)hdr;
                    *(s32 *)(ptr + 0) = abs0;
                    *(s32 *)(ptr + 8) = abs8;
                    if (hasSpecial) {
                        s32 t1 = hdr[4] + hdr[5];
                        s32 t2 = t1 + hdr[6];
                        if (i < t1 || i >= t2) {
                            u16 *dst = (u16 *)abs8;
                            u16 *src = (u16 *)abs8;
                            u16  tmp;
                            s32  qc;
                            s32  k;
                            for (k = 0; k < 8; k++) {
                                u16 v = *src;
                                *dst = v;
                                src = (u16 *)((u8 *)src + 4);
                                dst = (u16 *)((u8 *)dst + 2);
                            }
                            tmp = *(u16 *)(abs8 + 0x18);
                            *(u16 *)(abs8 + 0xE) = 0;
                            *(u16 *)(abs8 + 0xC) = (u16)(tmp - 0x10);
                            qc = *(u8 *)(ptr + 0xC);
                            CopyQwords((void *)(abs8 + 0x10),
                                       (void *)(abs8 + 0x20), (qc - 2) << 4);
                        }
                    }
                    ptr += 0x10;
                }
            }
        }
    }

    if (*(s32 *)(hdr + 0x10) != 0) {
        *(s32 *)(hdr + 0x10) += (s32)hdr;
    }
    if (*(s32 *)(hdr + 0x14) != 0) {
        *(s32 *)(hdr + 0x14) += (s32)hdr;
    }
    if (*(s32 *)(hdr + 0x18) != 0) {
        *(s32 *)(hdr + 0x18) += (s32)hdr;
    }

    {
        s32 off1C = *(s32 *)(hdr + 0x1C);
        if (off1C != 0) {
            s32 *base = (s32 *)(hdr + off1C);
            s32  n;
            *(s32 *)(hdr + 0x1C) = (s32)base;
            n = base[0];
            if (n > 0) {
                s32 j;
                for (j = 0; j < n; j++) {
                    base[j + 1] += (s32)hdr;
                }
            }
        }
    }

    {
        s32 off20 = *(s32 *)(hdr + 0x20);
        if (off20 != 0) {
            u8 *p = hdr + off20;
            *(s32 *)(hdr + 0x20) = (s32)p;
            for (;;) {
                s32 relC = *(s32 *)(p + 0xC) + (s32)hdr;
                *(s32 *)(p + 0xC) = relC;
                if (p[0] != 0xFF) {
                    u8 *q = p;
                    while (*q != 0xFF) {
                        *q = idMapPtr[*q];
                        q++;
                    }
                }
                p += 0x10;
                if (relC < 0) {
                    break;
                }
            }
        }
    }

    if (*(s32 *)(hdr + 0x28) != 0) {
        *(s32 *)(hdr + 0x28) += (s32)hdr;
    }

    {
        s32 m = hdr[0xC];
        if (m != 0) {
            s32 *arr = (s32 *)(hdr + 0x48);
            s32  k;
            for (k = 0; k < m; k++) {
                s32 off = arr[k];
                if (off != 0) {
                    u8 *sub = hdr + off;
                    arr[k] = (s32)sub;
                    if (*(s32 *)(sub + 0x14) != 0) {
                        *(s32 *)(sub + 0x14) =
                            (s32)(sub + *(s32 *)(sub + 0x14));
                    }
                    {
                        s32 cnt = *(u8 *)(sub + 0x10);
                        if (cnt != 0) {
                            s32 *g = (s32 *)(sub + 0x1C);
                            s32  l;
                            for (l = 0; l < cnt; l++) {
                                g[l] += (s32)hdr;
                            }
                        }
                    }
                }
            }
        }
    }

    if (classId >= 0) {
        u8   slot = g_mobyClassSlotRemap[classId];
        s32 *bdst = (s32 *)(g_mobyClassBounds + slot * 16);
        s32 *bsrc = (s32 *)idMapPtr;
        bdst[0] = bsrc[0];
        bdst[1] = bsrc[1];
        bdst[2] = bsrc[2];
        bdst[3] = bsrc[3];
    }

    if (*(s32 *)(hdr + 0) != 0) {
        func_00293B68(*(u8 **)(hdr + 0), instMode, idMapPtr, count);
    }
}
#endif

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

/**
 * StartFrontendSegmentLoad — allocate the frontend segment buffer and kick its
 * raw disc read.
 *
 * Sizes the buffer from the segment's sector count (g_discToc +0x34C), rounded
 * to 2048-byte sectors plus a page of slack, then carves it downward from the
 * top-of-memory marker D_1FF7FF0 (16-byte aligned). Stashes the buffer in
 * g_pLoadedSegment, writes a 0x60-byte header offset at its head, and starts an
 * asynchronous raw read of the segment's sectors (start LBN = toc +0x348 plus
 * base +0x32C, count = toc +0x34C) into the buffer just past the header.
 *
 * @return always 1 (load kicked).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", StartFrontendSegmentLoad);
#else
extern s32 g_discToc[];  /* 0x14B540 master disc asset directory */
extern u8 D_1FF7FF0[];   /* top-of-memory marker; segment buffers grow downward from here */
extern s32 KickRawFileRead(void *dest, s32 startSector, s32 sectorCount, void *toc);

s32 StartFrontendSegmentLoad(void) {
    u32 size = (((u32)g_discToc[0x34C / 4] << 11) + 0x1057) & 0xFFFFF000;
    u8 *seg = (u8 *)(((u32)D_1FF7FF0 - size) & 0xFFFFFFF0);

    g_pLoadedSegment = seg;
    *(s32 *)seg = 0x60;
    KickRawFileRead(seg + *(s32 *)seg,
                    g_discToc[0x348 / 4] + g_discToc[0x32C / 4],
                    g_discToc[0x34C / 4], g_discToc);
    return 1;
}
#endif

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

/**
 * func_00294B50 — begin an asynchronous level-asset load for level `level`.
 *
 * No-ops (returns 0) if the level's TOC entry is empty (g_discToc + level*0x14,
 * field +0x4B48 == 0) or a load is already in flight (D_1A9330 != -1). Otherwise
 * latches the request state (D_1A9330 = level, D_1A9334 = arg2, D_1A9338 = ctx),
 * arms the respawn slot for arg2 (g_respawnPlayerYaw[0x1F + arg2] = -1) when
 * valid, and kicks StartFileLoadWithCallback. Two variants: when the entry's
 * +0x4B50 field is set and `variant` is 0, load the +0x4B4C span with the
 * func_00294A30 completion callback; otherwise load the +0x4B44 span with
 * func_002949E0. Start LBN is the span base plus the shared prefix
 * g_discToc[+0x4B3C]. Returns StartFileLoadWithCallback's result.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00294B50);
#else
extern s32 g_discToc[];
extern s32 g_respawnPlayerYaw[];
extern s32 D_1A9330, D_1A9334;
extern void *D_1A9338;
extern void func_00294A30(void);
extern s32 StartFileLoadWithCallback(void *dest, s32 startSector, s32 count,
                                     void *callback, void *state);

void func_00294B50(s32 level, s32 arg2, void *dest, s32 variant) {
    u8 *toc = (u8 *)g_discToc + level * 0x14;

    if (*(s32 *)(toc + 0x4B48) == 0) {
        return;
    }
    if (D_1A9330 != -1) {
        return;
    }
    D_1A9330 = level;
    D_1A9334 = arg2;
    D_1A9338 = dest;
    if (arg2 >= 0) {
        g_respawnPlayerYaw[0x1F + arg2] = -1;   /* +0x48 + arg2*4 + 0x34 */
    }
    if (*(s32 *)(toc + 0x4B50) != 0 && variant == 0) {
        StartFileLoadWithCallback(dest,
            *(s32 *)(toc + 0x4B4C) + g_discToc[0x4B3C / 4],
            *(s32 *)(toc + 0x4B50), (void *)func_00294A30, &D_1A9330);
        return;
    }
    StartFileLoadWithCallback(dest,
        *(s32 *)(toc + 0x4B44) + g_discToc[0x4B3C / 4],
        *(s32 *)(toc + 0x4B48), (void *)func_002949E0, &D_1A9330);
}
#endif

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

/*
 * func_00295238(classId) — (re)load a gadget moby-class into one of the three
 * resident class buffers and refresh its sound bank.
 *
 * 1. Look classId up in the gadget-class TOC (g_discToc+0x4B40, stride 5 ints,
 *    up to 0x30 entries); return if absent (idx == 0x30) or if it is already the
 *    current class (rec[+0x14] == idx, where rec = g_respawnPlayerYaw+0x48).
 * 2. Record idx as current (rec[+0x14]) and find which of the three resident
 *    buffers already holds it (rec[+0x34], stride 4). If none does (slot == 3)
 *    it logs (DebugPrintStub) and services the voice stream around the evicting
 *    load (func_00294E98), then reuses buffer slot 0.
 * 3. WaitFrameDmaFence(1); the destination buffer is slot*0xC800 + (rec+0x40).
 *    (The <=0xFFFFF branch relocates it into the frame arena — the buffer lives
 *    above 1MB in practice, so that path is never taken; kept for fidelity.)
 * 4. Repoint any listener-history entry (g_listenerPosHistory, stride 0x70)
 *    whose sound object (+0x78, field +0x1C == 4) to D_1A93B0.
 * 5. If a previous bank handle is live (rec[+0x30]) unload it (func_00132858)
 *    and pump the sound system until idle. If the TOC slot declares a bank
 *    (+0x10 field) load it (snd_BankLoadFromIOP into buffer slot) and store the
 *    handle to rec[+0x30] and g_listenerPosHistory[0x17B0]; else clear rec[+0x30].
 * 6. Finalise (func_00132828) and load the class (LoadMobyClassFromWad).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00295238);
#else
extern s32 g_discToc[];
extern s32 g_respawnPlayerYaw[];
extern u8 g_listenerPosHistory[];      /* 0x188660 - stride 0x70 emitter ring */
extern s32 g_frameArenaBase;           /* per-frame arena base (arena relocate) */
extern s32 g_sceneArenaCursor;         /* scene arena cursor (arena relocate) */
extern u8 D_001A7210[];                /* +0x68 -> IOP sound-bank staging base */
extern char D_1A9370[];                /* eviction debug format string */
extern s32 D_1A93B0;                   /* default sound-object stand-in */
extern void WaitFrameDmaFence(s32 mask);
extern void DebugPrintStub(const char *fmt, ...);
extern void func_00294E98(s32 id, s32 zero);
extern s32  func_0029DDE8(void *dst, void *src, s32 count);
extern void func_00132858(s32 handle);
extern void func_00132828(void);
extern s32  snd_BankLoadFromIOP(void *addr);
extern s32  snd_Pump(void);
extern void LoadMobyClassFromWad(s32 classId, s32 tocIdx, void *buf);

void func_00295238(s32 classId) {
    s32 *toc = g_discToc;
    s32 *rec = &g_respawnPlayerYaw[0x12];  /* g_respawnPlayerYaw + 0x48 */
    s32 *req;
    s32 tocIdx, slot;
    u8 *buf;

    /* 1. locate the class in the gadget-class TOC */
    if (toc[0x12D0] == classId) {                    /* g_discToc + 0x4B40 */
        tocIdx = 0;
    } else {
        s32 *p = toc + 0x12D0;
        for (tocIdx = 1; tocIdx < 0x30; tocIdx++) {
            p += 5;
            if (p[0] == classId) {
                break;
            }
        }
    }
    if (tocIdx == 0x30) {
        return;                                       /* not in the gadget TOC */
    }
    if (tocIdx == *(s16 *)((u8 *)rec + 0x14)) {
        return;                                       /* already the current class */
    }

    /* 2. record current + find its resident buffer slot (of 3) */
    *(s16 *)((u8 *)rec + 0x14) = (s16)tocIdx;
    req = &rec[0xD];                                  /* rec + 0x34, 3 entries */
    if (req[0] == tocIdx) {
        slot = 0;
    } else {
        for (slot = 1; slot < 3; slot++) {
            if (req[slot] == tocIdx) {
                break;
            }
        }
    }

    WaitFrameDmaFence(1);
    if (slot == 3) {                                  /* not resident -> evict */
        DebugPrintStub(D_1A9370, classId);
        func_00294E98(classId, 0);
        slot = 0;
    }

    /* 3. destination buffer for this slot */
    *(s16 *)((u8 *)rec + 0x16) = (s16)slot;
    buf = (u8 *)rec + 0x40 + slot * 0xC800;
    if ((s32)buf <= 0xFFFFF) {                        /* never taken in practice */
        void *scratch = (void *)(g_frameArenaBase + g_sceneArenaCursor - 0xC800);
        func_0029DDE8(scratch, buf, 0xC80);
        buf = scratch;
    }

    /* 4. repoint stale listener-history sound objects */
    {
        u8 *e = g_listenerPosHistory;
        u8 *end = g_listenerPosHistory + 0x16C0;
        do {
            void *q = *(void **)(e + 0x78);
            if (q != 0 && *(s32 *)((u8 *)q + 0x1C) == 4) {
                *(void **)(e + 0x78) = &D_1A93B0;
            }
            e += 0x70;
        } while (e < end);
    }

    /* 5. swap the sound bank */
    if (rec[0xC] != 0) {                              /* rec + 0x30 = live handle */
        func_00132858(rec[0xC]);
        while (snd_Pump() != 0) {
        }
    }
    if (*(s32 *)((u8 *)g_discToc + 0x4B50 + tocIdx * 0x14) != 0) {
        s32 h = snd_BankLoadFromIOP(*(u8 **)(D_001A7210 + 0x68) + slot * 0xC800);
        rec[0xC] = h;
        *(s32 *)(g_listenerPosHistory + 0x17B0) = h;
    } else {
        rec[0xC] = 0;
    }

    /* 6. finalise + load the class */
    func_00132828();
    LoadMobyClassFromWad(classId, tocIdx, buf);
}
#endif

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

/**
 * func_002954F0 — allocate VRAM for a texture and queue its upload.
 *
 * Reserves a VRAM region from g_vramAllocCursor (a 0x400-byte header page plus
 * 2^(log2W+log2H) for the texel data), builds the 64-bit GS TEX0 register for it
 * (same packing as func_00295630: TBP0 = cursor2>>8, TBW/TW/TH from the log2
 * dims, plus the fixed 0x1300000 / 0x8000<<19 / sign bits), and — when the
 * upload queue has room (<0x40) — appends a descriptor to g_texUploadQueue
 * (source at desc+0x20, dest at desc+0x420, both VRAM cursors, and the log2
 * dims). Returns the packed GS TEX0 register.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002954F0);
#else
extern s32 g_vramAllocCursor;
extern s32 g_texUploadQueue[];
extern s32 g_texUploadCount;

u64 func_002954F0(void *descArg) {
    u8 *desc = (u8 *)descArg;
    s32 logW = Log2Floor(*(s32 *)(desc + 0x8));
    s32 logH = Log2Floor(*(s32 *)(desc + 0xC));
    s32 cursor = g_vramAllocCursor;
    s32 cursor2 = cursor + 0x400;
    s32 shift = (logW < 6) ? 0 : (logW - 6);
    u64 packed = (u64)(u32)(cursor2 >> 8)
               | ((u64)(u32)(1 << shift) << 14)
               | (((u64)(u32)logW << 26) | 0x1300000)
               | ((u64)(u32)logH << 30)
               | ((u64)(u32)(cursor >> 8) << 37)
               | ((u64)0x8000 << 19)
               | ((u64)-1 << 63);

    g_vramAllocCursor = cursor2 + (1 << (logW + logH));

    if (g_texUploadCount < 0x40) {
        s32 *e = &g_texUploadQueue[g_texUploadCount * 4];
        e[0] = (s32)(desc + 0x20);
        *(s16 *)((u8 *)e + 0x6) = (s16)(cursor >> 8);
        *(s16 *)((u8 *)e + 0x4) = 0;
        e[2] = (s32)(desc + 0x420);
        *(s16 *)((u8 *)e + 0xE) = (s16)(cursor2 >> 8);
        *(u8 *)((u8 *)e + 0xC) = (u8)logW;
        *(u8 *)((u8 *)e + 0xD) = (u8)logH;
        g_texUploadCount++;
    }
    return packed;
}
#endif

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

/* MapBeginUpload (0x295D70) — begin streaming the current level's galactic-map
 * texture into a fresh map-cache slot. Portable #else body lives in the
 * map-cache slice below (after the MapCache type + g_discToc/g_mapDataSet it
 * depends on); the INCLUDE_ASM stays here in address order. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapBeginUpload);
#endif

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

/* func_00295F98 (MapPromoteCacheSlot) — pick a usable galactic-map cache slot
 * and move it to slot 0. First tries func_00295F30(1) (a slot with state set +
 * id -1); if that returns nonzero it is the answer. Otherwise scans slots 1..4
 * for the first occupied slot (slotState != 0) whose level id lacks bit 0x1000,
 * relocates it into slot 0 (func_00296038) and returns its index (or 5 if none).
 * #else body is in the map-cache slice below (needs the MapCache type +
 * func_00296038); the INCLUDE_ASM stays here in address order. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00295F98);
#endif

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

/* func_00295F98 #else body — placed here so it follows the MapCache type and
 * func_00296038 it depends on (its INCLUDE_ASM stays in address order above). */
s32 func_00295F98(void) {
    s32 slot = func_00295F30(1);
    s32 i;

    if (slot != 0) {
        return slot;
    }
    for (i = 1; i < 5; i++) {
        if ((g_mapCache.slotLevelId[i] & 0x1000) == 0 && g_mapCache.slotState[i] != 0) {
            break;
        }
    }
    func_00296038(0, i);
    return i;
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

/*
 * MapBeginUpload (0x295D70) — kick off streaming the current level's galactic-
 * map texture into a fresh cache slot. Advances the two menu-screen DMA cursors
 * (g_menuScreenBlock +0x10C/+0x110) to carve a scratch pixel buffer, primes the
 * map-cache bookkeeping (all 5 slots reset, slot 1 = the new pixel buffer,
 * lockedSlot cleared), then — when a map-data TOC handle is live (cache +0x238
 * != -1) — issues the disc load for this level's secondary-set map texture:
 * first upload goes through StartFileLoadPumpingVoice + a voice pump and
 * func_002EFCA8; an already-armed upload is re-issued via func_002EFD28. Records
 * the level id (with the g_mapDataSet 0x100 flag) into slotLevelId[1], marks the
 * cache available, and plays UI sound 0x11. When no TOC handle is live it just
 * clears `available` and plays the sound.
 *
 * MATCH WALL: the 0x20 multi-callee-save frame is packed 8-byte by the later
 * cc1 (the unit-wide save-layout wall) and the many cache-field stores colour
 * differently; kept as the portable #else body, placed here in the map-cache
 * slice after the MapCache type + g_discToc/g_mapDataSet it depends on. The
 * three still-unnamed cache fields (+0x238 TOC handle, +0x23C load-issued flag,
 * +0x240 pixel byte size, +0x248 saved DMA cursor) are accessed by raw offset.
 */
#ifdef TARGET_NATIVE
extern u8   g_menuScreenBlock[];   /* 0x1F27C0 menu-screen manager block */
extern void func_00298AA0(void);
extern void func_0029ECE0(s32 level, s32 flag);
extern void StartFileLoadPumpingVoice(void *dest, s32 startSector, s32 sectorCount);
extern void PumpDialogVoiceSystem(s32 blocking);
extern s32  func_002EFCA8(s32 dest, s32 handle, s32 zero, s32 byteSize);
extern s32  func_002EFD28(s32 dest, s32 handle, s32 zero, s32 byteSize, s32 zero2);
extern void PlayGlobalSound(s32 id, s32 a, s32 b);

void MapBeginUpload(void) {
    s32 *msb;
    s32  cur10C, cur110, pixelBuf, handle;

    func_00298AA0();
    func_0029ECE0(g_mapCache.currentLevel, 1);

    msb    = (s32 *)g_menuScreenBlock;
    cur10C = msb[0x10C / 4];
    cur110 = msb[0x110 / 4];
    pixelBuf = cur10C + 0x9A800;
    msb[0x110 / 4] = cur110 + 0x48000;
    msb[0x10C / 4] = pixelBuf + 0x48000;

    *(s32 *)((u8 *)&g_mapCache + 0x248) = cur10C;   /* saved DMA cursor */
    g_mapCache.slotState[0] = 0;
    g_mapCache.slotState[1] = pixelBuf;
    g_mapCache.slotState[2] = cur110;
    g_mapCache.slotState[3] = 0;
    g_mapCache.slotState[4] = 0;
    g_mapCache.slotLevelId[0] = -1;
    g_mapCache.slotLevelId[1] = -1;
    g_mapCache.slotLevelId[2] = -1;
    g_mapCache.slotLevelId[3] = -1;
    g_mapCache.slotLevelId[4] = -1;
    g_mapCache.lockedSlot = -1;

    handle = *(s32 *)((u8 *)&g_mapCache + 0x238);   /* map-data TOC handle */
    if (handle == -1) {
        g_mapCache.available = 0;
        PlayGlobalSound(0x11, 0, 0);
        return;
    }

    if (*(s32 *)((u8 *)&g_mapCache + 0x23C) == 0 && g_mapDataSet != 0) {
        /* first upload: stream this level's secondary-set map texture */
        s32 *toc      = g_discToc + g_mapCache.currentLevel * 2;  /* stride 8 */
        s32  secSize  = toc[0x15D4 / 4];
        s32  byteSize = secSize << 7;

        StartFileLoadPumpingVoice((void *)pixelBuf,
                                  toc[0x15D0 / 4] + g_discToc[0x36C / 4], /* + global WAD base LBA */
                                  secSize);
        PumpDialogVoiceSystem(1);

        *(s32 *)((u8 *)&g_mapCache + 0x23C) = 1;
        *(s32 *)((u8 *)&g_mapCache + 0x240) = byteSize;
        g_mapCache.slotPixelCount[1] = byteSize;
        func_002EFCA8(g_mapCache.slotState[1], handle, 0, byteSize);

        g_mapCache.activeSlot = -2;
        g_mapCache.available  = 1;
        g_mapCache.slotLevelId[1] = g_playerProgress + 0x100;
        PlayGlobalSound(0x11, 0, 0);
        return;
    }

    /* upload already armed (or secondary set inactive): re-issue it */
    {
        s32 byteSize = *(s32 *)((u8 *)&g_mapCache + 0x240);
        s32 levelId  = g_playerProgress;

        func_002EFD28(g_mapCache.slotState[1], handle, 0, byteSize, 0);
        g_mapCache.slotPixelCount[1] = byteSize;

        g_mapCache.activeSlot = -2;
        g_mapCache.available  = 1;
        if (g_mapDataSet != 0) {
            levelId = g_playerProgress + 0x100;
        }
        g_mapCache.slotLevelId[1] = levelId;
        PlayGlobalSound(0x11, 0, 0);
    }
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

/* Per-state writer of two f32 out-params (A=$4, B=$5) selected by g_playerProgress
 * (cases 2/0xB/7/0x14) with float-threshold gating on g_soundBankHandlesBlk+0x80/
 * +0x84; returns a written flag. PARK (#70): func_00301430 is called 4x with
 * ambiguous/unset args and its return drives bltz/slti branches (arg+return
 * semantics unrecovered), plus ~14 unnamed D_1A9xxx float globals — a faithful
 * #else would guess the callee contract (silent-bug risk). Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00297B48);

/*
 * MapBuildBitmapFrom4bpp(dest, srcA, srcB) — convert a 256x256 4bpp map image
 * into the packed 1bpp minimap bitmap `dest`, in a 16-row-band swizzled layout.
 * For each of 256 rows: decode the row's 4bpp pixels into the scratchpad at
 * 0x70000000 (func_00297E80), then for each of 32 column-blocks pack 32 pixels
 * into one 32-bit word — bit `b` is set when the pixel nibble is nonzero (even b
 * = low nibble, odd b = high nibble of scratchpad byte[b>>1]). The output word
 * for (row, col) lands at dest + 4*((row%16) + 512*(row/16)) + col*0x40 (bands of
 * 16 rows, columns 0x40 bytes apart). The matching build keeps the asm (a
 * strength-reduction near-miss, 64.83%); this #else is the portable equivalent. */
#ifdef TARGET_NATIVE
extern void func_00297E80(void *scratchpad, s32 row, void *srcA, void *srcB);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", MapBuildBitmapFrom4bpp);
#else
void MapBuildBitmapFrom4bpp(void *destArg, void *srcA, void *srcB) {
    u8 *dest = (u8 *)destArg;
    s32 row;

    for (row = 0; row < 0x100; row++) {
        s32  band = row >> 4;             /* 16-row band index */
        s32  sub  = row - (band << 4);    /* row within band (row % 16) */
        s32 *out  = (s32 *)(dest + ((sub + (band << 9)) << 2));
        s32  col;

        func_00297E80((void *)0x70000000, row, srcA, srcB);

        for (col = 0; col < 0x20; col++) {
            const u8 *sp = (const u8 *)0x70000000 + col * 16;
            s32 acc = 0;
            s32 bit;
            for (bit = 0; bit < 0x20; bit++) {
                u8  byte   = sp[bit >> 1];
                s32 nibble = (bit & 1) ? (byte >> 4) : (byte & 0xF);
                if (nibble != 0) {
                    acc |= (s32)(1U << bit);
                }
                if ((bit & 0x1F) == 0x1F) {
                    *out++ = acc;
                    acc = 0;
                }
            }
            out = (s32 *)((u8 *)out + 0x3C);
        }
    }
}
#endif

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

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298308);
#else
extern void CopyQwords(void *dst, const void *src, s32 nbytes);
/*
 * func_00298308(dst, src) — the "plain" 1bpp->4bpp map-bitmap expander (the
 * bit0-clear arm of MapBuildBitmap). Builds a 256-entry lookup that fans each
 * bit k of an input byte out to nibble k of a 32-bit word (bit set -> 0xF), then
 * for each of 128 rows expands the next 16 source bytes to sixteen 32-bit words
 * (64 bytes) and replicates that run four times (256 bytes) into dst — a 4x
 * horizontal scale. Consumes 128*16 = 2048 source bytes, writes 128*256 = 32 KB.
 */
void func_00298308(void *dst, u8 *src) {
    s32 lut[256];   /* bit-expand table: input byte -> nibble mask */
    s32 row[16];    /* one expanded 16-byte source run */
    u8 *out = (u8 *)dst;
    s32 i;
    s32 j;
    s32 k;

    for (i = 0; i < 0x100; i++) {
        lut[i] = 0;
        if (i & 0x01) lut[i]  = 0x0000000F;
        if (i & 0x02) lut[i] |= 0x000000F0;
        if (i & 0x04) lut[i] |= 0x00000F00;
        if (i & 0x08) lut[i] |= 0x0000F000;
        if (i & 0x10) lut[i] |= 0x000F0000;
        if (i & 0x20) lut[i] |= 0x00F00000;
        if (i & 0x40) lut[i] |= 0x0F000000;
        if (i & 0x80) lut[i] |= 0xF0000000;
    }

    for (j = 0; j < 0x80; j++) {
        for (k = 0; k < 16; k++) {
            row[k] = lut[*src];
            src++;
        }
        CopyQwords(out, row, 0x40); out += 0x40;
        CopyQwords(out, row, 0x40); out += 0x40;
        CopyQwords(out, row, 0x40); out += 0x40;
        CopyQwords(out, row, 0x40); out += 0x40;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002984D8);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002984E0);
#else
extern u8   *g_mapBitmapBuffer;
extern u8    D_1A9718[];  /* 64-byte per-nibble accumulation weight LUT */
extern void  FillMemory32(void *dst, u32 val, s32 len);
/*
 * func_002984E0(out) — downsample the loaded 4bpp map source at
 * g_mapBitmapBuffer[+4] into the 1bpp discovered-area bitmap `out` (the inverse
 * of func_00298308's expand). Works in bands of four 64-byte input rows: for
 * each byte both nibbles index a 16-entry weight LUT (copied from D_1A9718) and
 * accumulate into a 128-column sum buffer; every fourth row that buffer is
 * thresholded (>=8 -> set) and bit-packed 8 columns/byte to sixteen output
 * bytes. Consumes 512*64 = 32 KB, emits 128*16 = 2 KB, then clears bit0 of
 * out[0].
 */
void func_002984E0(u8 *out) {
    s32 acc[128];   /* per-column accumulator (sp[0..0x200)) */
    s32 lut[16];    /* nibble weight table (sp+0x200) */
    u8 *lb = (u8 *)lut;
    u8 *in = *((u8 **)&g_mapBitmapBuffer + 1);  /* g_mapBitmapBuffer[+4] */
    u8 *outp = out;
    s32 i;
    s32 j;

    for (j = 0; j < 0x40; j++) {
        lb[j] = D_1A9718[j];
    }

    for (i = 0; i < 0x200; i++) {
        s32 n;
        if ((i & 3) == 0) {
            FillMemory32(acc, 0, 0x200);
        }
        for (n = 0; n < 64; n++) {
            u8 b = *in++;
            acc[2 * n]     += lut[b & 0xF];
            acc[2 * n + 1] += lut[b >> 4];
        }
        if ((i & 3) == 3) {
            s32 k;
            for (k = 0; k < 128; k++) {
                if (acc[k] < 8) {
                    acc[k] = 0;
                } else {
                    acc[k] = 1 << (k & 7);
                }
            }
            for (k = 0; k < 16; k++) {
                u8 *a = (u8 *)&acc[8 * k];
                *outp++ = a[0] | a[4] | a[8] | a[12] | a[16] | a[20] | a[24] | a[28];
            }
        }
    }

    out[0] &= 0xFE;
}
#endif

/* Expand a localized template string (id from g_mapVertexData[+0x20] entry
 * idx*0x28 +0xA) into the dest buffer, substituting "%b" with the equipped
 * weapon's name via func_00115DA8(sprintf). PARK (#70): nested byte-copy loops
 * plus a deep indirection chain (mapVertexData -> entry -> slot -> equipped item
 * -> g_weaponTable[item*0xE0]+0x80 -> name) and unrecovered func_00115DA8 format
 * semantics — faithful transcription is high silent-bug risk. Bare INCLUDE_ASM. */
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

/**
 * func_00298918 — compute two scaled level-parameter outputs.
 *
 * Resolves a level index (arg `level`, or the current g_playerProgress when -1,
 * clamped to 0 when outside [0,20]), looks up its parameter block
 * (func_002988C8), and writes two blended, 1/512-scaled values:
 *   *outX = (block[0] + block[1] * fa) / 512
 *   *outY = (block[2] + block[3] * fb) / 512
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298918);
#else
void func_00298918(f32 *outX, f32 *outY, s32 level, f32 fa, f32 fb) {
    f32 v0, v1, v2, v3;

    if (level == -1) {
        level = g_playerProgress;
    }
    if (level < 0) {
        level = 0;
    }
    if (level >= 21) {
        level = 0;
    }
    v0 = ((f32 *)func_002988C8(level))[0];
    v1 = ((f32 *)func_002988C8(level))[1];
    *outX = (v0 + v1 * fa) * (1.0f / 512.0f);
    v2 = ((f32 *)func_002988C8(level))[2];
    v3 = ((f32 *)func_002988C8(level))[3];
    *outY = (v2 + v3 * fb) * (1.0f / 512.0f);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_002989F8);

/* func_00298AA0: empty/no-op leaf (jr ra; nop). */
void func_00298AA0(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298AA8);

/**
 * func_00298F20 — save/load status-machine tick.
 *
 * Raises the rain-active flag (g_pRainHeightmap+0x1C) when any area-load field
 * is pending (g_areaTable +0x16C/+0x10 nonzero, or +0x24 positive). Then honours
 * pending save/load transition bits in the status flags word
 * (g_nSaveLoadStatusCode+0x4): bit 0x80 -> enter state 0x15, bit 0x100 -> state
 * 0x14 (each clearing its bit and setting 0x40). Dispatches the per-state
 * handler from the D_256050 jump table, then bumps the tick counter
 * (g_pRainHeightmap+0x18) — or resets it to 0 if the handler changed the state.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/191238", func_00298F20);
#else
extern u8 g_areaTable[];
extern u8 g_pRainHeightmap[];
extern s32 g_nSaveLoadStatusCode[];        /* [0] = state code, [1] (+0x4) = flags */
extern void (*D_256050[])(void);           /* per-state handler jump table */

void func_00298F20(void) {
    s32 origState = g_nSaveLoadStatusCode[0];
    s32 flags;

    if (*(s32 *)(g_areaTable + 0x16C) != 0 ||
        *(s32 *)(g_areaTable + 0x10) != 0 ||
        *(s32 *)(g_areaTable + 0x24) > 0) {
        *(s32 *)(g_pRainHeightmap + 0x1C) = 1;
    }

    flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x80) {
        g_nSaveLoadStatusCode[0] = 0x15;
        g_nSaveLoadStatusCode[1] = (flags & ~0x80) | 0x40;
        flags = g_nSaveLoadStatusCode[1];
    }
    if (flags & 0x100) {
        g_nSaveLoadStatusCode[0] = 0x14;
        g_nSaveLoadStatusCode[1] = (flags & ~0x100) | 0x40;
    }

    D_256050[g_nSaveLoadStatusCode[0]]();

    if (g_nSaveLoadStatusCode[0] == origState) {
        *(s32 *)(g_pRainHeightmap + 0x18) += 1;
    } else {
        *(s32 *)(g_pRainHeightmap + 0x18) = 0;
    }
}
#endif

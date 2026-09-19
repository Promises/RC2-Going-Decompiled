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

/* func_002912C0 (EU twin of USA func_002912B8) — rebuild the inventory
 * quick-select order. Walks item ids 0..0x37 and, for each that
 * IsItemUnlockedAtProgress reports available at the given story progress
 * (id 0 excluded), appends it via AddItemToInventoryOrder. Region-agnostic
 * body (named callees only). Matching arm stays INCLUDE_ASM.
 * Word-verified vs USA func_002912B8: jals IsItemUnlockedAtProgress /
 * AddItemToInventoryOrder unchanged across regions. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002912C0);
#else
extern s32 IsItemUnlockedAtProgress(s32 itemId, s32 progress);
extern s32 AddItemToInventoryOrder(s32 itemId);
void func_002912C0(s32 progress) {
    s32 i;
    for (i = 0; i < 0x38; i++) {
        if (IsItemUnlockedAtProgress(i, progress) != 0 && i != 0) {
            AddItemToInventoryOrder(i);
        }
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291328);

/* LoadIrxModuleFromBuffer (EU twin of USA LoadIrxModuleFromBuffer) — load an IRX module
 * from an in-memory image. Builds the loadfile arg block on the stack
 * ([0]=image, [4]=arg, [8]=size, [C]=0), requests the load (func_0011AFE0);
 * on success spins the completion poll (func_0011AFC0) until it returns
 * negative, then starts the module (func_0011ED08(arg,0,0)) and returns 1 iff
 * that succeeded (>=0); returns 1 without running if the request itself failed.
 * Matching arm stays INCLUDE_ASM.
 * Word-verified vs USA LoadIrxModuleFromBuffer: SDK-region callees
 * func_0011AFE0/func_0011AFC0/func_0011ED08 are byte-identical addresses in
 * both builds (this region is shared). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", LoadIrxModuleFromBuffer);
#else
extern void *func_0011AFE0(void *argBlock, s32 mode, void *arg);
extern s32   func_0011AFC0(void *handle);
extern s32   func_0011ED08(void *arg, s32 a1, s32 a2);
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
#endif

/* SetupMemoryArenaTable (EU twin of USA SetupMemoryArenaTable) — zero the 0x9C-byte
 * memory-arena table then fill the EE memory-region base addresses the loaders
 * + ResetFrameArenas read (scene arena halves at 0x354000, splash/loading-WAD
 * buffers, per-asset display-model buffers, GUI/debug pools, boot-WAD/upper-RAM
 * tops). Matching arm stays INCLUDE_ASM.
 * Word-verified vs USA SetupMemoryArenaTable: table base retargeted to
 * g_nVendorBuyQuantity+0x8C78 (== g_memoryArenaTable 0x1BAE40, same as USA);
 * cursor at g_nVendorBuyQuantity+0x68 (== g_sceneArenaCursor 0x1B2230, same as
 * USA). EU cc1 builds the constants via inter-register deltas, but ALL stored
 * values decode identical to USA. jal memset unchanged. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", SetupMemoryArenaTable);
#else
extern u8 g_nVendorBuyQuantity[];
void SetupMemoryArenaTable(void) {
    u8 *t = g_nVendorBuyQuantity + 0x8C78;               /* g_memoryArenaTable 0x1BAE40 */
    s32 cursor = *(s32 *)(g_nVendorBuyQuantity + 0x68);  /* g_sceneArenaCursor 0x1B2230 */
    memset(t, 0, 0x9C);
    *(s32 *)(t + 0x04) = 0x100000;
    *(s32 *)(t + 0x08) = 0x354000;
    *(s32 *)(t + 0x0C) = 0x354000;
    *(s32 *)(t + 0x10) = cursor + 0x354000;
    *(s32 *)(t + 0x14) = cursor * 2 + 0x354000;
    *(s32 *)(t + 0x18) = cursor * 2 + 0x454000;
    *(s32 *)(t + 0x68) = cursor * 2 + 0x454000;
    *(s32 *)(t + 0x6C) = 0x1F0C000;
    *(s32 *)(t + 0x70) = 0x1F0C000;
    *(s32 *)(t + 0x74) = 0x1F20000;
    *(s32 *)(t + 0x78) = 0x1F28000;
    *(s32 *)(t + 0x7C) = 0x1F54000;
    *(s32 *)(t + 0x80) = 0x1FB8000;
    *(s32 *)(t + 0x84) = 0x1FF8000;
    *(s32 *)(t + 0x88) = 0x1FFC000;
    *(s32 *)(t + 0x8C) = 0x7000000;
    *(s32 *)(t + 0x90) = 0x7100000;
    *(s32 *)(t + 0x94) = 0x7180000;
    *(s32 *)(t + 0x98) = 0x7200000;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", BootSystemInit);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", DrawSkyShellsScaledSpin);

/*
 * RenderSky — EU twin of USA RenderSky. Draws the sky shells for the frame:
 * opens a sky segment, points the shell spin-rate table at the static rates
 * (D_255B40), draws scaled spin in the boot/title area (g_playerProgress == 0) or
 * fixed spin in-game, closes the segment, then appends SCANMSK (0x47)=0x5360B and
 * the Z-buffer base (reg 0x4E)=0x1000000 | (zbuf >> 13). See USA RenderSky for the
 * TU-flag/version wall (absolute vs gp-rel g_playerProgress + tail-call j); #else.
 */
extern void func_002E4580(void);   /* BeginSkyDrawSegment */
extern void DrawSkyShellsScaledSpin(void);   /* DrawSkyShellsScaledSpin */
extern void func_002E4318(void);   /* DrawSkyShellsFixedSpin */
extern void func_002E45F0(void);   /* CloseSkyDrawSegment */
extern void AppendGsRegPacket(s32 reg, s32 val);   /* AppendGsRegPacket */
extern s32  g_playerProgress;
extern u8   D_001A7308[];          /* +0x58 = g_vramZBuffer (EU) */
extern u8   g_nBoltCounterDisplayed[]; /* +0x48 = g_pSkyShellSpinRates (EU) */
extern s32  D_255B40[];            /* static sky shell spin rates (EU) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", RenderSky);
#else
void RenderSky(void) {
    func_002E4580();
    *(s32 *)(g_nBoltCounterDisplayed + 0x48) = (s32)D_255B40;
    if (g_playerProgress != 0) {
        func_002E4318();
    } else {
        DrawSkyShellsScaledSpin();
    }
    func_002E45F0();
    AppendGsRegPacket(0x47, 0x5360B);
    AppendGsRegPacket(0x4E, 0x1000000 | (*(s32 *)(D_001A7308 + 0x58) >> 13));
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291C00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291C10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291D58);

/* func_00291DC8 (EU twin of USA func_00291D28) — per-frame refresh of the
 * directional + point-light set. Seeds the directional-light matrix row at
 * g_dirLightMatrices+0x340 from the default (EU D_1A9250 / USA D_1A9190), derives
 * its direction from the camera yaw (angle = WrapAnglePiSum(g_cameraRot[2],-0.8)):
 * +0x350=cos*0.866, +0x354=sin*0.866, +0x358=-0.5, +0x35C=0. Then walks the 8
 * point-light slots; a type-0 slot is skipped; otherwise, if relevant
 * (func_00283708>1.0) OR radius moved >1.0, the light data is copied into the
 * request record and dispatched by type (1 -> func_00291F50 then promote to 2;
 * 2 -> func_00292068). Matching arm stays INCLUDE_ASM.
 * Word-verified vs USA func_00291D28 (jal ORDER): WrapAnglePiSum->func_00284458,
 * cos->func_00283A40, sin->func_00283A58, func_002837F8->func_00283708,
 * GetFloatAbs->GetFloatAbs, func_00291EB0->func_00291F50,
 * func_00291FC8->func_00292068. dir = g_nVendorBuyQuantity+0x104F8
 * (== USA g_dirLightMatrices 0x1C26C0); g_pointLights = +0x108F8 (== USA
 * 0x1C2AC0); g_cameraRot[2] = +0x3110 (== USA 0x1B52D8); matrix-row src
 * D_1A9250 (USA D_1A9190 +0xC0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291DC8);
#else
extern u8   g_nVendorBuyQuantity[];
extern u8   D_1A9250[];                       /* default directional-matrix row (16B) */
extern f32  func_00284458(f32 a, f32 b);      /* WrapAnglePiSum */
extern f32  func_00283A40(f32 x);             /* cos */
extern f32  func_00283A58(f32 x);             /* sin */
extern f32  func_00283708(void *light, void *req);
extern f32  GetFloatAbs(f32 x);             /* fabs */
extern void func_00291F50(s32 index);
extern void func_00292068(void *arg);

void func_00291DC8(void) {
    u8 *dir = g_nVendorBuyQuantity + 0x104F8;   /* g_dirLightMatrices; dir+0x340 */
    u8 *gpl = g_nVendorBuyQuantity + 0x108F8;   /* g_pointLights */
    f32 angle;
    s32 i;

    *(u64 *)(dir + 0x340) = *(u64 *)D_1A9250;
    *(u64 *)(dir + 0x348) = *(u64 *)(D_1A9250 + 8);

    angle = func_00284458(*(f32 *)(g_nVendorBuyQuantity + 0x3110), -0.8f);
    *(f32 *)(dir + 0x350) = func_00283A40(angle) * 0.866f;
    *(f32 *)(dir + 0x354) = func_00283A58(angle) * 0.866f;
    *(f32 *)(dir + 0x358) = -0.5f;
    *(s32 *)(dir + 0x35C) = 0;

    for (i = 0; i < 8; i++) {
        u8 *light = gpl + 0x10 + i * 0x20;
        u8 *req   = gpl + 0x120 + i * 0x30;
        s32 type;

        if (*(s32 *)(req - 0x10) == 0) {
            continue;
        }
        if (!(1.0f < func_00283708(light, req)) &&
            !(1.0f < GetFloatAbs(*(f32 *)(light + 0xC) - *(f32 *)(req + 0xC)))) {
            continue;
        }

        *(u64 *)req = *(u64 *)light;
        *(u64 *)(req + 8) = *(u64 *)(light + 8);

        type = *(s32 *)(req - 0x10);
        if (type == 1) {
            func_00291F50(i);
            *(s32 *)(req - 0x10) = 2;
        } else if (type == 2) {
            func_00292068((void *)i);
        }
    }
}
#endif

/* func_00291F50 (EU twin of USA func_00291EB0) — build a light-relight request
 * record for point light `index`. Fills the request entry (g_pointLights+0x100,
 * stride 0x30) by running three geometry builders in sequence, each appending
 * into the shared buffer from the request's cursor (+0xC) up to a 0x400-byte
 * limit and returning the advanced cursor; records per-stage half-counts into
 * the entry halfwords (+0x2/+0x4 stage1, +0x6 stage2 delta, +0x8 stage2 total,
 * +0xA stage3 delta), stopping early if any builder hits the limit.
 * Matching arm stays INCLUDE_ASM.
 * Word-verified vs USA func_00291EB0 (jal ORDER): func_002F5DF8->func_002F5EF0,
 * func_002E4000->func_002E3FB8, func_002F19D0->func_002F1AC8. g_pointLights =
 * g_nVendorBuyQuantity+0x108F8; request base = +0x109F8 (== gpl+0x100). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00291F50);
#else
extern u8  g_nVendorBuyQuantity[];
extern s32 func_002F5EF0(s32 cursor, s32 limit, s32 index, void *src);
extern s32 func_002E3FB8(s32 cursor, s32 limit, s32 index, void *src);
extern s32 func_002F1AC8(s32 cursor, s32 limit, s32 index, void *src);

void func_00291F50(s32 index) {
    u8 *gpl = g_nVendorBuyQuantity + 0x108F8;    /* g_pointLights */
    u8 *req = gpl + 0x100 + index * 0x30;
    void *src = gpl + index * 0x20 + 0x10;
    s32 limit = *(s32 *)(req + 0xC) + 0x400;
    s32 cursor, count;

    if ((u32)*(s32 *)(req + 0xC) >= (u32)limit) {
        return;
    }
    *(s16 *)(req + 0x0) = 0;

    cursor = func_002F5EF0(*(s32 *)(req + 0xC), limit, index, src);
    count = (cursor - *(s32 *)(req + 0xC)) >> 1;
    *(s16 *)(req + 0x2) = (s16)count;
    if ((u32)cursor >= (u32)limit) {
        return;
    }
    *(s16 *)(req + 0x4) = (s16)count;

    cursor = func_002E3FB8(cursor, limit, index, src);
    count = (cursor - *(s32 *)(req + 0xC)) >> 1;
    *(s16 *)(req + 0x6) = (s16)(count - *(u16 *)(req + 0x4));
    if ((u32)cursor >= (u32)limit) {
        return;
    }
    *(s16 *)(req + 0x8) = (s16)count;

    cursor = func_002F1AC8(cursor, limit, index, src);
    count = (cursor - *(s32 *)(req + 0xC)) >> 1;
    *(s16 *)(req + 0xA) = (s16)(count - *(u16 *)(req + 0x8));
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292068);

/* func_00292098 (EU twin of USA func_00291FF8) — flush one entry of the deferred
 * light-relight request table (g_pointLights+0x100, stride 0x30). Each entry
 * holds a target buffer pointer (+0xC) and three {off,len} s16 spans
 * (+0x0/+0x2, +0x4/+0x6, +0x8/+0xA, in u16 elements). When the buffer is
 * non-NULL, run the three relight passes, each handed the span's start pointer
 * (buf+off*2), its end (start+len*2) and the entry index; then clear each span
 * so the request is consumed exactly once. Matching arm stays INCLUDE_ASM.
 * Word-verified vs USA func_00291FF8 (jal ORDER): func_002F5F70->func_002F6068,
 * func_002E4178->func_002E4130, func_002F1B58->func_002F1C50. Entry base =
 * g_nVendorBuyQuantity+0x109F8 (== g_pointLights 0x1C2AC0 + 0x100). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292098);
#else
extern u8   g_nVendorBuyQuantity[];
extern void func_002F6068(u8 *start, u8 *end, s32 index);
extern void func_002E4130(u8 *start, u8 *end, s32 index);
extern void func_002F1C50(u8 *start, u8 *end, s32 index);

void func_00292098(s32 index) {
    u8 *e = (g_nVendorBuyQuantity + 0x109F8) + index * 0x30;   /* g_pointLights+0x100 record */
    u8 *buf = *(u8 **)(e + 0xC);
    if (buf == NULL) {
        return;
    }
    func_002F6068(buf + *(s16 *)(e + 0x0) * 2,
                  buf + *(s16 *)(e + 0x0) * 2 + *(s16 *)(e + 0x2) * 2, index);
    *(s16 *)(e + 0x0) = 0;
    *(s16 *)(e + 0x2) = 0;
    buf = *(u8 **)(e + 0xC);
    func_002E4130(buf + *(s16 *)(e + 0x4) * 2,
                  buf + *(s16 *)(e + 0x4) * 2 + *(s16 *)(e + 0x6) * 2, index);
    *(s16 *)(e + 0x4) = 0;
    *(s16 *)(e + 0x6) = 0;
    buf = *(u8 **)(e + 0xC);
    func_002F1C50(buf + *(s16 *)(e + 0x8) * 2,
                  buf + *(s16 *)(e + 0x8) * 2 + *(s16 *)(e + 0xA) * 2, index);
    *(s16 *)(e + 0xA) = 0;
    *(s16 *)(e + 0x8) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292160);

/* func_002925B0 (EU twin of USA BindParticleFxAssets) — bind a freshly loaded
 * particle-effect asset pack: rebase the effect-def pointer table (self-relative
 * offsets rebased from hdr[8] onto the relocated blob), copy the blob body, then
 * build the particle texture table (word0 = ((texBase+rec[0])<<4)+rec[1], word1 =
 * ((texBase+rec[2])<<4)+Log2Floor(rec[3])) and publish the tex count.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA BindParticleFxAssets (jal ORDER): func_00283460 (copy
 * blob) -> func_00283370, Log2Floor -> func_002832D8.
 * REGION DELTA: the 0x1F0000 particle pool is +0x80 in EU — g_particleFxBlob
 * USA 0x1F26C0 -> D_001F0000+0x2740 (0x1F2740); g_particleEffectDefs USA
 * 0x1F24C0 -> +0x2540 (0x1F2540); g_particleTexTable USA 0x1F1EC0 -> +0x1F40
 * (0x1F1F40). g_particleTexCount = g_nBoltCounterDisplayed+0x45C (0x1B1D24,
 * SAME absolute addr as USA — engine-global lane, delta 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002925B0);
#else
extern u8   D_001F0000[];                 /* particle pool base (EU +0x80 vs USA) */
extern u8   g_nBoltCounterDisplayed[];    /* +0x45C = g_particleTexCount */
extern void *func_00283370(void *dst, const void *src, s32 nbytes); /* EU byte memcpy */
extern s32  func_002832D8(s32 x);                  /* Log2Floor */

void func_002925B0(void *hdrArg, s32 texBase, s32 *texRecords, s32 texCount) {
    u8  *hdr       = (u8 *)hdrArg;
    s32  defCount  = *(s32 *)(hdr + 0);
    s32  blobField = *(s32 *)(hdr + 8);
    s32  blobLen   = *(s32 *)(hdr + 0xC);            /* asm: lw $6,0xC($4) @0x2925F8 */
    s32 *blob      = (s32 *)(D_001F0000 + 0x2740);   /* g_particleFxBlob */
    s32 *defs      = (s32 *)(D_001F0000 + 0x2540);   /* g_particleEffectDefs */
    s32 *texTable  = (s32 *)(D_001F0000 + 0x1F40);   /* g_particleTexTable */

    if (defCount > 0) {
        s32 *entry = (s32 *)(hdr + 0x10);
        s32 *out   = defs;
        s32  delta = blobField - (s32)blob;
        s32  i;
        for (i = defCount; i != 0; i--) {
            s32 off = *entry;
            *out = (off != 0) ? (off - delta) : (s32)blob;
            out++;
            entry++;
        }
    }

    func_00283370(blob, hdr + blobField, blobLen);

    if (texCount > 0) {
        s32 *rec = texRecords;
        s32  slot;
        *(s32 *)(g_nBoltCounterDisplayed + 0x45C) = 0;
        do {
            slot = *(s32 *)(g_nBoltCounterDisplayed + 0x45C);
            texTable[slot * 2]     = ((texBase + rec[0]) << 4) + rec[1];
            texTable[slot * 2 + 1] = ((texBase + rec[2]) << 4) + func_002832D8(rec[3]);
            *(s32 *)(g_nBoltCounterDisplayed + 0x45C) = slot + 1;
            rec += 4;
        } while (slot + 1 < texCount);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", BuildUiTextureDescriptors);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", BindSkyData);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", LoadPlayerDisplayTextures);

/* BindPlayerDisplayModel (EU twin of USA BindPlayerDisplayModel) — stage the player
 * display model's textures and header. For each of g_playerTexCount entries,
 * writes a 3-doubleword GS texture register block into the display list at
 * g_pointLights+0x2280 (stride 0x18): the per-texture descriptor followed by two
 * fixed GS register values. Then relocates the player model chunk into place
 * (RelocateMobyClassChunk from g_pPlayerModelBuffer) and mirrors
 * the first moby class header's byte +0x8 into +0x9. Matching arm stays
 * INCLUDE_ASM.
 * Word-verified vs USA BindPlayerDisplayModel: RelocateMobyClassChunk ->
 * RelocateMobyClassChunk; reloc table D_1A91D0 -> D_1A9290 (+0xC0). g_playerTexCount =
 * D_001A7308+0x10, g_playerTexDescriptors = D_001A7308+0x18, g_pPlayerModelBuffer
 * = D_001A7308+0xC; display list = g_nVendorBuyQuantity+0x12B78 (== g_pointLights
 * 0x1C2AC0 + 0x2280); g_mobyClassHeaders[0] via g_mapTextureWidth+0x89A0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", BindPlayerDisplayModel);
#else
extern u8   g_nVendorBuyQuantity[];
extern u8   D_001A7308[];                 /* player-model global region */
extern s32  g_mapTextureWidth[];
extern u8   D_1A9290[];                   /* moby-class reloc table (EU; USA D_1A91D0) */
extern void RelocateMobyClassChunk(void *buffer, s32 slot, void *reloc);  /* RelocateMobyClassChunk */

void BindPlayerDisplayModel(void) {
    s32 count = *(s32 *)(D_001A7308 + 0x10);       /* g_playerTexCount */

    if (count > 0) {
        u64 *dst  = (u64 *)(g_nVendorBuyQuantity + 0x12B78);  /* g_pointLights + 0x2280 */
        u64 *desc = (u64 *)(D_001A7308 + 0x18);               /* g_playerTexDescriptors */
        s32 i;

        for (i = 0; i < count; i++) {
            dst[0] = *desc;                     /* per-texture descriptor */
            dst[1] = 0x0000FFA0000000E0ULL;     /* fixed GS register A */
            dst[2] = 0x0040000400004000ULL;     /* fixed GS register B */
            desc++;
            dst += 3;
        }
    }
    RelocateMobyClassChunk(*(void **)(D_001A7308 + 0xC), 0, D_1A9290);  /* g_pPlayerModelBuffer */
    {
        u8 *hdr = *(u8 **)((u8 *)g_mapTextureWidth + 0x89A0);   /* g_mobyClassHeaders[0] */
        hdr[0x9] = hdr[0x8];
    }
}
#endif

/* LoadPlayerDisplayModel (EU twin of USA LoadPlayerDisplayModel) — load the armor-variant
 * player display model into the dedicated buffer and bind it: load the variant
 * textures (LoadPlayerDisplayTextures), bind the model (BindPlayerDisplayModel), fix up the loaded
 * header (func_00293DC8) so g_mobyClassHeaders[0] points at the rebased buffer,
 * then record the loaded armor variant. Matching arm stays INCLUDE_ASM.
 * Word-verified vs USA LoadPlayerDisplayModel (jal ORDER): LoadPlayerDisplayTextures
 * -> LoadPlayerDisplayTextures, BindPlayerDisplayModel, func_00293D68 ->
 * func_00293DC8. g_mobyClassHeaders[0] = g_mapTextureWidth+0x89A0 (0x1CDB00, delta
 * 0); g_playerModelBufferBase = g_nVendorBuyQuantity+0x8CF0 (0x1BAEB8, delta 0).
 * REGION DELTA: g_loadedArmorVariant = D_001A7308+0x8 (0x1A7310) vs USA 0x1A7290
 * (+0x80 D_001A73xx lane). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", LoadPlayerDisplayModel);
#else
extern u8   g_nVendorBuyQuantity[];
extern s32  g_mapTextureWidth[];
extern u8   D_001A7308[];                 /* +0x8 = g_loadedArmorVariant */
extern void LoadPlayerDisplayTextures(s32 variant);   /* LoadPlayerDisplayTextures */
extern void func_00293DC8(u8 *dst, u8 *src);   /* func_00293D68 */

void LoadPlayerDisplayModel(s32 variant) {
    LoadPlayerDisplayTextures(variant);
    BindPlayerDisplayModel();
    func_00293DC8(*(u8 **)((u8 *)g_mapTextureWidth + 0x89A0),   /* g_mobyClassHeaders[0] */
                  *(u8 **)(g_nVendorBuyQuantity + 0x8CF0));     /* g_playerModelBufferBase */
    *(s32 *)(D_001A7308 + 0x8) = variant;                       /* g_loadedArmorVariant */
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", LoadHeldItemDisplayModel);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00292EF0);

/* LoadShipDisplayModel (EU twin of USA LoadShipDisplayModel) — load the ship display
 * model for level `index`: record the index in the level dialog TOC, wait for the
 * frame DMA fence, kick the model disc read into g_shipModelBufferBase from the
 * level's TOC entry, set up the GS texture register block at g_pointLights+0x2280,
 * then rebase the loaded moby class header (FixupMobyClassHeader). Matching arm stays
 * INCLUDE_ASM.
 * Word-verified vs USA LoadShipDisplayModel (jal ORDER): WaitFrameDmaFence ->
 * WaitFrameDmaFence, StartFileLoadPumpingVoice -> func_002B8838, FixupMobyClassHeader
 * -> FixupMobyClassHeader. g_discToc = D_0014B5C0 (0x14B5C0, +0x80 vs USA 0x14B540);
 * g_levelDialogToc+0x13B0 = D_0014B5C0+0x76C0; g_shipModelBufferBase =
 * g_nVendorBuyQuantity+0x8CE8 (0x1BAEB0, delta 0); reg block =
 * g_nVendorBuyQuantity+0x12B78 (g_pointLights+0x2280, delta 0); reloc/bounds
 * D_1A91E0 -> D_1A92A0 (+0xC0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", LoadShipDisplayModel);
#else
extern u8   D_0014B5C0[];                 /* g_discToc; +0x76C0 = g_levelDialogToc+0x13B0 */
extern u8   g_nVendorBuyQuantity[];
extern u8   D_1A92A0[];                   /* moby-class bounds/reloc (EU; USA D_1A91E0) */
extern void WaitFrameDmaFence(s32 mode);      /* WaitFrameDmaFence */
extern void func_002B8838(void *dest, s32 startSector, s32 sectorCount);  /* StartFileLoadPumpingVoice */
extern void FixupMobyClassHeader(void *hdr, s32 instMode, void *idMap, s32 classId);  /* FixupMobyClassHeader */

void LoadShipDisplayModel(s32 index) {
    u8  *toc   = D_0014B5C0;                 /* g_discToc */
    u8  *ldt   = D_0014B5C0 + 0x76C0;        /* g_levelDialogToc + 0x13B0 */
    s32 *entry = (s32 *)(toc + index * 8);
    void *shipBuf = *(void **)(g_nVendorBuyQuantity + 0x8CE8);   /* g_shipModelBufferBase */

    *(s32 *)(ldt + 0x18) = index;
    WaitFrameDmaFence(1);
    func_002B8838(shipBuf,
                  entry[0x48D8 / 4] + *(s32 *)(toc + 0x3E24),
                  entry[0x48DC / 4]);
    {
        u64 *reg = (u64 *)(g_nVendorBuyQuantity + 0x12B78);   /* g_pointLights + 0x2280 */
        reg[0] = *(u64 *)(ldt + 0x38);
        reg[1] = 0x0000FFA0000000E0ULL;     /* fixed GS register A */
        reg[2] = 0x0040000400004000ULL;     /* fixed GS register B */
    }
    FixupMobyClassHeader(shipBuf, 0, D_1A92A0, -1);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", LoadShipDisplayTexture);

/* ParseLoadedSegment (EU twin of USA ParseLoadedSegment, loaders.cpp) — bind the
 * just-loaded HUD asset segment: publish its rounded sub-segment sizes, make a
 * DebugMalloc'd working copy of its asset header, resolve the embedded section
 * pointers, and stage the four HUD-bank texture blocks to the IOP upload ring.
 * See USA ParseLoadedSegment for the full per-bank behaviour. Matching arm stays
 * INCLUDE_ASM.
 * Word-verified vs USA ParseLoadedSegment (jal ORDER): CopyQwords -> CopyQwords,
 * func_002933D0 -> DecompressHudBankWad, UploadDataToIopRing -> func_002EFCA8,
 * func_0028BBA0 -> UploadHudBankTextures, func_0028B8C8 -> RelocateHudBankGsSlots; DebugMalloc /
 * func_0011AEA0 unchanged. Strings D_1A91F0/D_1A9200/10/20/30 -> D_1A92B0/C0/D0/
 * E0/F0 (+0xC0). g_memoryArenaTable = g_nVendorBuyQuantity+0x8C78 (delta 0);
 * g_pHudAssetHeader = g_pActiveTextTable+0x48; size-table dst = g_pActiveTextTable
 * +0x70. REGION DELTA: g_pLoadedSegment = D_001A7308 (0x1A7308) vs USA 0x1A7288
 * (+0x80 lane). REGION DELTA: DebugMalloc __LINE__ args are 0x2FF/0x327 in EU vs
 * USA 0x305/0x32D (PAL source-line shift, benign). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", ParseLoadedSegment);
#else
extern u8   D_001A7308[];              /* +0x0 = g_pLoadedSegment */
extern u8   g_pActiveTextTable[];      /* +0x48 = g_pHudAssetHeader; +0x70 = size table */
extern u8   g_nVendorBuyQuantity[];    /* +0x8C78 = g_memoryArenaTable */
extern void *DebugMalloc(s32 size, s32 arg2, void *file, s32 line);
extern void CopyQwords(void *dst, const void *src, s32 nbytes);   /* CopyQwords */
extern s32  func_002EFCA8(void *eeAddr, s32 sizeQw, s32 arg3, void *tag);  /* UploadDataToIopRing */
extern void DecompressHudBankWad(s32 slot, u8 *dest);   /* DecompressHudBankWad */
extern void UploadHudBankTextures(s32 a, void *b, s32 c);
extern void RelocateHudBankGsSlots(s32 a, void *b);
extern void func_0011AEA0(s32 a);
extern u8 D_1A92B0[];  /* "loaders.cpp" __FILE__ */
extern u8 D_1A92C0[];  /* per-bank IOP-upload debug tags */
extern u8 D_1A92D0[];
extern u8 D_1A92E0[];
extern u8 D_1A92F0[];

void ParseLoadedSegment(void) {
    u8 *seg = *(u8 **)D_001A7308;              /* g_pLoadedSegment */
    u8 *arenaTable = g_nVendorBuyQuantity + 0x8C78;   /* g_memoryArenaTable */
    u8 *header;
    u8 *iopBase;
    s32 *src;
    s32 *dst;
    s32 i;
    s32 size;
    s32 sizeQw;

    /* Round the 5 sub-segment sizes up to 64 and publish the size table. */
    src = (s32 *)(seg + 0x24);
    dst = (s32 *)(g_pActiveTextTable + 0x70);      /* g_hudMobySpawnStart + 8 */
    for (i = 4; i >= 0; i--) {
        *dst = (*src + 0x3F) & 0xFFFFFFC0;
        src += 2;
        dst += 1;
    }

    /* DebugMalloc + CopyQwords the working header copy. */
    size = (*(s32 *)(seg + 0x1C) + 0x3F) & 0xFFFFFFC0;
    header = (u8 *)DebugMalloc(size, 0, D_1A92B0, 0x2FF);
    CopyQwords(header, (void *)(*(s32 *)(seg + 0x18) + (s32)seg), size);
    *(u8 **)(g_pActiveTextTable + 0x48) = header;  /* g_pHudAssetHeader */

    /* Resolve the header's section pointers + the fixed IOP staging base. */
    *(u8 **)(g_pActiveTextTable + 0x4C) = header + *(s32 *)(header + 0x4);
    iopBase = (u8 *)(*(s32 *)(arenaTable + 0x10) + 0x60000);
    *(u8 **)(g_pActiveTextTable + 0x50) = header + *(s32 *)(header + 0x8);   /* g_hudIconMap */
    *(u8 **)(g_pActiveTextTable + 0x58) = header + *(s32 *)(header + 0xC);   /* g_hudClutSlots */
    *(u8 **)(g_pActiveTextTable + 0x54) = header + *(s32 *)(header + 0x10);  /* g_hudTextureSlots */

    /* Bank 0. */
    if (*(s32 *)(header + 0x54) != 0) {
        sizeQw = ((*(s32 *)(seg + 0x24) + 0x3F) & 0xFFFFFFC0) >> 4;
        DecompressHudBankWad(0, iopBase);
        *(s32 *)(*(u8 **)(g_pActiveTextTable + 0x48) + 0x94) =
            func_002EFCA8((void *)(*(s32 *)(seg + 0x20) + (s32)seg), sizeQw, sizeQw, D_1A92C0);
        UploadHudBankTextures(0, iopBase, 1);
    }

    /* Bank 1 (scratch decompress, no upload). */
    header = *(u8 **)(g_pActiveTextTable + 0x48);
    if (*(s32 *)(header + 0x58) != 0) {
        u8 *buf = (u8 *)DebugMalloc(*(s32 *)(header + 0x58), 0, D_1A92B0, 0x327);
        DecompressHudBankWad(1, buf);
        func_0011AEA0(0);
        RelocateHudBankGsSlots(1, buf);
    }

    /* Bank 2. */
    header = *(u8 **)(g_pActiveTextTable + 0x48);
    if (*(s32 *)(header + 0x5C) != 0) {
        sizeQw = ((*(s32 *)(seg + 0x34) + 0x3F) & 0xFFFFFFC0) >> 4;
        *(s32 *)(*(u8 **)(g_pActiveTextTable + 0x48) + 0x9C) =
            func_002EFCA8((void *)(*(s32 *)(seg + 0x30) + (s32)seg), sizeQw, sizeQw, D_1A92D0);
    }

    /* Bank 3. */
    header = *(u8 **)(g_pActiveTextTable + 0x48);
    if (*(s32 *)(header + 0x60) != 0) {
        sizeQw = ((*(s32 *)(seg + 0x3C) + 0x3F) & 0xFFFFFFC0) >> 4;
        *(s32 *)(*(u8 **)(g_pActiveTextTable + 0x48) + 0xA0) =
            func_002EFCA8((void *)(*(s32 *)(seg + 0x38) + (s32)seg), sizeQw, sizeQw, D_1A92E0);
    }

    /* Bank 4. */
    header = *(u8 **)(g_pActiveTextTable + 0x48);
    if (*(s32 *)(header + 0x64) != 0) {
        sizeQw = ((*(s32 *)(seg + 0x44) + 0x3F) & 0xFFFFFFC0) >> 4;
        *(s32 *)(*(u8 **)(g_pActiveTextTable + 0x48) + 0xA4) =
            func_002EFCA8((void *)(*(s32 *)(seg + 0x40) + (s32)seg), sizeQw, sizeQw, D_1A92F0);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", DecompressHudBankWad);

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

/* func_00293BC8 (EU twin of USA func_00293B68) — walk a table of groupCount group
 * records (stride 0x10) and, per group, emit a sub-set of its instances
 * (stride 0x40). Word +0x4 packs {hi:hi16, total:lo16}; the walked span starts at
 * base + (total-hi)*0x10, the inner loop steps by 4 while < hi; lo16 total is
 * written back. Each instance's material id (+0x20), when non-negative, is remapped
 * through idMap; dispatched to func_00293498 (instMode != 0) or func_002937C0
 * (instMode == 0). Matching arm stays INCLUDE_ASM.
 * Word-verified vs USA func_00293B68 (jal ORDER): func_00293438 -> func_00293498,
 * func_00293760 -> func_002937C0. Region-agnostic body (no global refs). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00293BC8);
#else
extern void func_00293498(void *inst, s32 matBlock, s32 a2, s32 a3, s32 a4, s32 a5, s32 matId);
extern void func_002937C0(void *inst, s32 a1, s32 a2, s32 a3, s32 a4, s32 matId);

void func_00293BC8(u8 *groups, s32 instMode, u8 *idMap, s32 groupCount) {
    s32 g;
    for (g = 0; g < groupCount; g++) {
        s32 packed = *(s32 *)(groups + 4);
        u8 *base   = *(u8 **)(groups + 0);
        s32 hi     = packed >> 16;
        s32 total  = packed & 0xFFFF;
        u8 *inst   = base + ((total - hi) << 4);
        s32 k;
        *(s32 *)(groups + 4) = total;
        for (k = 0; k < hi; k += 4) {
            s32 matId = *(s32 *)(inst + 0x20);
            if (matId >= 0) {
                matId = idMap[matId];
            }
            if (instMode != 0) {
                func_00293498(inst, instMode + (matId << 4),
                              *(s32 *)(inst + 0), *(s32 *)(inst + 4),
                              *(s32 *)(inst + 0x10), *(s32 *)(inst + 0x14), matId);
            } else {
                func_002937C0(inst, *(s32 *)(inst + 0), *(s32 *)(inst + 4),
                              *(s32 *)(inst + 0x10), *(s32 *)(inst + 0x14), matId);
            }
            inst += 0x40;
        }
        groups += 0x10;
    }
}
#endif

/* RelocateMobyClassChunk (EU twin of USA RelocateMobyClassChunk) — fix up a freshly-loaded
 * moby class chunk in place. Rebases the pointer table at chunk+[0x4] (words +0x0/
 * +0x8 of each 0x10-byte entry whose base is below the arena limit
 * g_memoryArenaTable+0x8), then walks the descriptor table at chunk+[0x8] until a
 * rebased +0xC word goes negative — relocating that word and translating each
 * descriptor's 0xFF-terminated name through nameTable — and hands the pointer
 * table to func_00293BC8 for group instantiation. Matching arm stays INCLUDE_ASM.
 * Word-verified vs USA RelocateMobyClassChunk: func_00293B68 -> func_00293BC8;
 * g_memoryArenaTable = g_nVendorBuyQuantity+0x8C78 (delta 0). Signature matches
 * batch-1 decl `void RelocateMobyClassChunk(void*, s32, void*)`. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", RelocateMobyClassChunk);
#else
extern u8   g_nVendorBuyQuantity[];   /* +0x8C78 = g_memoryArenaTable */
extern void func_00293BC8(u8 *groups, s32 instMode, u8 *idMap, s32 groupCount);

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
            if (e[0] < *(s32 *)(g_nVendorBuyQuantity + 0x8C78 + 0x8)) {
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

    func_00293BC8((u8 *)table1, arg2, nameTable, count);
}
#endif

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

/* FixupMobyClassHeader (EU twin of USA FixupMobyClassHeader) — rebase a freshly-loaded
 * moby class header in place: turn every embedded self-relative offset into an
 * absolute pointer, and record per-slot metadata. See USA FixupMobyClassHeader for
 * the full behaviour (special-flag data-size clamp, mesh-block band compaction,
 * sound/anim table rebases, bounds-vector copy, group instantiation).
 * Matching arm stays INCLUDE_ASM.
 * Word-verified vs USA FixupMobyClassHeader (jal ORDER): DebugPrintStub ->
 * func_0026FD28, CopyQwords -> CopyQwords, func_00293B68 -> func_00293BC8; fmt
 * string D_1A9240 -> D_1A9300 (+0xC0). g_mobyClassSlotRemap = g_mapTextureWidth
 * +0x9300 (0x1CE460, delta 0). REGION DELTA: g_mobyClassDataSizes = D_001D0C40
 * +0x1C0 (0x1D0E00) vs USA 0x1D0D80, and g_mobyClassBounds = D_001D0C40+0x940
 * (0x1D1580) vs USA 0x1D1500 (+0x80 D_001D0xxx lane). idMap widened to void* so
 * the callers (LoadShipDisplayModel, RegisterMobyClass) pass pointers warning-free. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", FixupMobyClassHeader);
#else
extern s32  g_mapTextureWidth[];       /* +0x9300 = g_mobyClassSlotRemap */
extern u8   D_001D0C40[];              /* +0x1C0 = g_mobyClassDataSizes; +0x940 = g_mobyClassBounds */
extern u8   D_1A9300[];                /* debug fmt string (func_0026FD28 no-op) */
extern void func_0026FD28(void *fmt, s32 classId);              /* DebugPrintStub */
extern void CopyQwords(void *dst, const void *src, s32 nbytes);  /* CopyQwords */
extern void func_00293BC8(u8 *groups, s32 instMode, u8 *idMap, s32 groupCount);

void FixupMobyClassHeader(void *hdrArg, s32 instMode, void *idMapArg, s32 classId) {
    u8  *hdr       = (u8 *)hdrArg;
    u8  *idMapPtr  = (u8 *)idMapArg;
    u8  *slotRemap = (u8 *)g_mapTextureWidth + 0x9300;   /* g_mobyClassSlotRemap */
    u32 *dataSizes = (u32 *)(D_001D0C40 + 0x1C0);         /* g_mobyClassDataSizes */
    s32  hasSpecial = (hdr[0xB] != 0);
    s32  count;

    if (hasSpecial) {
        s32 size = *(s32 *)(hdr + 0x2C);
        u8  slot;
        func_0026FD28(D_1A9300, classId);            /* retail no-op */
        if (size > 0x3FC00) {
            size = 0x3FC00;
        }
        hdr[0xB]  = 0;
        hdr[0x2C] = 0;
        *(u16 *)(hdr + 0x2E) = 0;
        hdr[0x2D] = (u8)(size >> 10);
        slot = slotRemap[classId];
        dataSizes[slot] = (u32)size;
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
        u8   slot = slotRemap[classId];
        s32 *bdst = (s32 *)(D_001D0C40 + 0x940 + slot * 16);   /* g_mobyClassBounds */
        s32 *bsrc = (s32 *)idMapPtr;
        bdst[0] = bsrc[0];
        bdst[1] = bsrc[1];
        bdst[2] = bsrc[2];
        bdst[3] = bsrc[3];
    }

    if (*(s32 *)(hdr + 0) != 0) {
        func_00293BC8(*(u8 **)(hdr + 0), instMode, idMapPtr, count);
    }
}
#endif

/* RegisterMobyClass (EU twin of USA RegisterMobyClass) — assign a class slot to
 * classId and fill the registry tables. Null header -> take a slot from
 * g_mobyClassCountNoHeader, bind the headerless update fn, record only the remap
 * byte. Real header -> take the next g_mobyClassCount slot, record remap /
 * reverse-map / header / data-size (byte +0x2D << 10, or 0x100000 when 0xFF), bind
 * the update fn, then FixupMobyClassHeader rebases the header. Matching arm stays
 * INCLUDE_ASM.
 * Word-verified vs USA RegisterMobyClass (jal ORDER): BindMobyClassUpdateFunc ->
 * func_002B7170, FixupMobyClassHeader. g_mobyClassSlotRemap =
 * g_mapTextureWidth+0x9300 (0x1CE460); g_mobyClassSlotToId = +0x9120 (0x1CE280);
 * g_mobyClassHeaders = +0x89A0 (0x1CDB00) — all delta 0; g_mobyClassCount =
 * g_nBoltCounterDisplayed+0x1F8 (0x1B1AC0); g_mobyClassCountNoHeader = +0x1FC
 * (0x1B1AC4) — delta 0. REGION DELTA: g_mobyClassDataSizes = D_001D0C40+0x1C0
 * (0x1D0E00, +0x80). arg3 widened to void* (passed straight to FixupMobyClassHeader's
 * idMap). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", RegisterMobyClass);
#else
extern s32  g_mapTextureWidth[];
extern u8   g_nBoltCounterDisplayed[];    /* +0x1F8 = g_mobyClassCount; +0x1FC = *NoHeader */
extern u8   D_001D0C40[];                 /* +0x1C0 = g_mobyClassDataSizes */
extern void func_002B7170(s32 classId, s32 headerless);   /* BindMobyClassUpdateFunc */
extern void FixupMobyClassHeader(void *hdr, s32 instMode, void *idMap, s32 classId);  /* FixupMobyClassHeader */

void RegisterMobyClass(u8 *hdr, s32 arg2, void *arg3, s32 classId) {
    u8   *slotRemap  = (u8 *)g_mapTextureWidth + 0x9300;          /* g_mobyClassSlotRemap */
    s16  *slotToId   = (s16 *)((u8 *)g_mapTextureWidth + 0x9120); /* g_mobyClassSlotToId */
    void **headers   = (void **)((u8 *)g_mapTextureWidth + 0x89A0);/* g_mobyClassHeaders */
    u32  *dataSizes  = (u32 *)(D_001D0C40 + 0x1C0);               /* g_mobyClassDataSizes */
    s32  *count      = (s32 *)(g_nBoltCounterDisplayed + 0x1F8);  /* g_mobyClassCount */
    s32  *countNoHdr = (s32 *)(g_nBoltCounterDisplayed + 0x1FC);  /* g_mobyClassCountNoHeader */

    if (hdr == NULL) {
        s32 slot = *countNoHdr;
        func_002B7170(classId, 1);
        slotRemap[classId] = (u8)slot;
        *countNoHdr = slot + 1;
    } else {
        s32 slot = *count;
        slotRemap[classId] = (u8)slot;
        slotToId[slot] = (s16)classId;
        headers[slot] = hdr;
        dataSizes[slot] = (u32)hdr[0x2D] << 10;
        if (hdr[0x2D] == 0xFF) {
            dataSizes[slot] = 0x100000;
        }
        func_002B7170(classId, 0);
        *count = slot + 1;
        FixupMobyClassHeader(hdr, arg2, arg3, classId);
    }
    __asm__ __volatile__("");
}
#endif

/* func_002942C8 (EU twin of USA StartFrontendSegmentLoad) — allocate the frontend
 * segment buffer and kick its raw disc read. Sizes the buffer from the segment's
 * sector count (g_discToc +0x34C), rounds to 2048-byte sectors + a page of slack,
 * carves it downward from the top-of-memory marker D_1FF7FF0 (16-byte aligned),
 * stashes it in g_pLoadedSegment with a 0x60 header word, then kicks the raw read
 * (start LBN = toc+0x348 + base+0x32C, count = toc+0x34C). Returns 1.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA StartFrontendSegmentLoad: g_discToc -> D_0014B5C0 (offsets
 * 0x34C/0x348/0x32C unchanged); g_pLoadedSegment -> D_001A7308 (0x1A7308, USA
 * 0x1A7288, +0x80 lane); D_1FF7FF0 same absolute (top-of-mem, region-invariant);
 * KickRawFileRead -> func_002B87C8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002942C8);
#else
extern u8   D_001A7308[];   /* +0x0 = g_pLoadedSegment */
extern u8   D_1FF7FF0[];    /* top-of-memory marker; buffers grow downward */
extern void func_002B87C8(void *dest, s32 startSector, s32 sectorCount, void *toc);  /* KickRawFileRead */

s32 func_002942C8(void) {
    u8  *toc  = D_0014B5C0;
    u32  size = (((u32)*(s32 *)(toc + 0x34C) << 11) + 0x1057) & 0xFFFFF000;
    u8  *seg  = (u8 *)(((u32)D_1FF7FF0 - size) & 0xFFFFFFF0);

    *(u8 **)D_001A7308 = seg;                 /* g_pLoadedSegment */
    *(s32 *)seg = 0x60;
    func_002B87C8(seg + *(s32 *)seg,
                  *(s32 *)(toc + 0x348) + *(s32 *)(toc + 0x32C),
                  *(s32 *)(toc + 0x34C), toc);
    return 1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294368);

/* UpdateLevelStagingMachine: interior body split at the named symbol.
 * splat only emits an .s for a name it sees in an INCLUDE_ASM, so this
 * reference is what keeps the body in the tree (and promotes its interior
 * alabel to a real glabel). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", UpdateLevelStagingMachine);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002945B0);

/* StreamSceneSegment: interior body split at the named symbol.
 * splat only emits an .s for a name it sees in an INCLUDE_ASM, so this
 * reference is what keeps the body in the tree (and promotes its interior
 * alabel to a real glabel). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", StreamSceneSegment);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", BindSceneChunk);

/* LoadGlobalDialogScene (EU twin of USA LoadGlobalDialogScene) — stream a GLOBAL
 * cinematic/dialog scene chunk and build its sub-chunk pointer table. Per-scene
 * {lbnOffset@+0x3E28, sectorCount@+0x3E2C} at stride 8 inside g_discToc, shared
 * base LBN @+0x3E24; scene descriptor block at g_cameraSlotActive+0x990 (+0x70
 * load buffer, +0x30 scene index, +0x74[] up to 0x46 sub-chunk pointers). mode 0
 * just records the index; nonzero also pumps voice + fades to black before the
 * kick; the voice pump runs once more (blocking) after. Matching arm stays
 * INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA LoadGlobalDialogScene (raw .s): g_discToc -> D_0014B5C0;
 * g_cameraSlotActive+0x990 -> g_nVendorBuyQuantity+0x65F8 (== 0x1B87C0, delta 0);
 * TOC offsets 0x3E24/0x3E28/0x3E2C and desc offsets 0x70/0x30/0x74 unchanged; jal
 * ORDER StartFileLoad->func_002B86D0, PumpDialogVoiceSystem->func_002B8898,
 * FadeOutToBlackBlocking->func_0027D818. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", LoadGlobalDialogScene);
#else
extern u8   g_nVendorBuyQuantity[];     /* +0x65F8 = scene desc block (g_cameraSlotActive+0x990) */
extern void func_002B86D0(s32 dest, s32 lbn, s32 sectors);   /* StartFileLoad */
extern void func_002B8898(s32 blocking);                     /* PumpDialogVoiceSystem */
extern void func_0027D818(s32 frames);                       /* FadeOutToBlackBlocking */

void LoadGlobalDialogScene(s32 sceneIndex, s32 mode) {
    u8  *toc  = D_0014B5C0;
    s32 *desc = (s32 *)(g_nVendorBuyQuantity + 0x65F8);
    s32  lbnOff  = *(s32 *)(toc + sceneIndex * 8 + 0x3E28);
    s32  baseLbn = *(s32 *)(toc + 0x3E24);
    s32  sectors = *(s32 *)(toc + sceneIndex * 8 + 0x3E2C);
    s32 *entry;
    s32  i;

    func_002B86D0(desc[0x70 / 4], lbnOff + baseLbn, sectors);
    if (mode != 0) {
        func_002B8898(0);
        func_0027D818(mode);
    }
    desc[0x30 / 4] = sceneIndex;

    func_002B8898(1);
    entry = (s32 *)desc[0x70 / 4];
    if (entry[1] == 0) {
        return;
    }
    {
        s32 entryWord0 = entry[0];
        i = 0;
        for (;;) {
            s32 ofs;
            entry += 2;
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", SelectSceneSubChunk);

/* func_002949D0 (EU twin of USA func_00294970) — reset the streaming in-flight
 * slot table. Word-clears the whole table (0x35840 bytes) then arms the sentinels:
 * the s16 at +0x14 and the three in-flight request words at +0x34/+0x38/+0x3C all
 * set to -1. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00294970 (raw .s): FillMemory32 unchanged (region-
 * shared name); record base g_respawnPlayerYaw+0x48 -> D_0014B5C0+0x7790 (the EU
 * convention already used by func_00294A40/func_00294CA8); size + field offsets
 * unchanged. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002949D0);
#else
extern void FillMemory32(void *dst, u32 val, s32 len);

void func_002949D0(void) {
    s32 *rec = (s32 *)(D_0014B5C0 + 0x7790);   /* in-flight record base */
    s32 *p;
    s32  i;

    FillMemory32(rec, 0, 0x35840);
    *(s16 *)((u8 *)rec + 0x14) = -1;
    p = (s32 *)((u8 *)rec + 0x3C);
    for (i = 2; i >= 0; i--) {
        *p = -1;
        p--;
    }
}
#endif

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

/* func_00294BB0 (EU twin of USA func_00294B50) — begin an asynchronous level-asset
 * load for `level`. No-ops if the level's TOC entry is empty (toc+0x4B48==0) or a
 * load is in flight (latch != -1). Otherwise latches the request (level/arg2/dest),
 * arms the respawn slot for arg2, and kicks StartFileLoadWithCallback: variant-0
 * with a live +0x4B50 field loads the +0x4B4C span with the func_002949E0-twin
 * callback, else the +0x4B44 span with the func_00294A30-twin callback (start LBN
 * = span base + g_discToc[+0x4B3C]). Matching arm stays INCLUDE_ASM; #else is the
 * structure model.
 * Word-verified vs USA func_00294B50 (raw .s): g_discToc -> D_0014B5C0; record base
 * g_respawnPlayerYaw+0x48 -> D_0014B5C0+0x7790; latch trio D_1A9330/34/38 ->
 * D_1A93F0/F4/F8 (+0xC0 small-data pool); StartFileLoadWithCallback -> func_002B8778;
 * callbacks func_002949E0 -> func_00294A40, func_00294A30 -> func_00294A90; TOC
 * offsets 0x4B44/0x4B48/0x4B4C/0x4B50/0x4B3C unchanged. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294BB0);
#else
extern s32   D_1A93F0;        /* load-request latch (gp_rel; USA D_1A9330) */
extern s32   D_1A93F4;        /* USA D_1A9334 */
extern void *D_1A93F8;        /* USA D_1A9338 */
extern void  func_002B8778(void *dest, s32 startSector, s32 count, void *callback, void *state);
extern void  func_00294A40(s32 *rec, s32 enable);   /* USA func_002949E0 */
extern void  func_00294A90(void);                    /* USA func_00294A30 */

void func_00294BB0(s32 level, s32 arg2, void *dest, s32 variant) {
    u8 *toc = D_0014B5C0 + level * 0x14;

    if (*(s32 *)(toc + 0x4B48) == 0) {
        return;
    }
    if (D_1A93F0 != -1) {
        return;
    }
    D_1A93F0 = level;
    D_1A93F4 = arg2;
    D_1A93F8 = dest;
    if (arg2 >= 0) {
        *(s32 *)(D_0014B5C0 + 0x7790 + arg2 * 4 + 0x34) = -1;
    }
    if (*(s32 *)(toc + 0x4B50) != 0 && variant == 0) {
        func_002B8778(dest,
            *(s32 *)(toc + 0x4B4C) + *(s32 *)(D_0014B5C0 + 0x4B3C),
            *(s32 *)(toc + 0x4B50), (void *)func_00294A90, &D_1A93F0);
        return;
    }
    func_002B8778(dest,
        *(s32 *)(toc + 0x4B44) + *(s32 *)(D_0014B5C0 + 0x4B3C),
        *(s32 *)(toc + 0x4B48), (void *)func_00294A40, &D_1A93F0);
}
#endif

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

/* func_00294EF8 (EU twin of USA func_00294E98) — service the dialog-voice stream
 * around a blocking gadget-class load: pump the voice system, run the load/lookup
 * (func_00294C48-twin), then pump again so streaming audio is serviced on both
 * sides. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00294E98 (raw .s): PumpDialogVoiceSystem -> func_002B8898,
 * func_00294C48 -> func_00294CA8; func_00294CA8(a,b) with a=arg0, b=arg1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294EF8);
#else
extern void func_002B8898(s32 blocking);   /* PumpDialogVoiceSystem */
extern void func_00294CA8(s32 a, s32 b);   /* func_00294C48 */

void func_00294EF8(s32 a, s32 b) {
    func_002B8898(1);
    func_00294CA8(a, b);
    func_002B8898(1);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00294F40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", LoadMobyClassFromWad);

/* func_00295298 (EU twin of USA func_00295238) — (re)load a gadget moby-class into
 * one of three resident class buffers and refresh its sound bank. Looks classId up
 * in the gadget-class TOC (g_discToc+0x4B40, stride 5 ints, up to 0x30); returns if
 * absent or already current. Records the index, finds/evicts a resident buffer slot
 * (evict logs + services the voice stream around func_00294E98-twin), waits the DMA
 * fence, repoints stale listener-history sound objects (+0x1C==4 -> stand-in),
 * swaps the sound bank (unload live handle + pump idle, then bank-load if declared),
 * finalises and loads the class. Matching arm stays INCLUDE_ASM; #else is the
 * structure model.
 * Word-verified vs USA func_00295238 (raw .s): g_discToc -> D_0014B5C0; record base
 * g_respawnPlayerYaw+0x48 -> D_0014B5C0+0x7790; g_listenerPosHistory 0x188660 ->
 * D_00180000+0x86E0 (0x1886E0, +0x80); D_1A93B0 -> D_1A9470 (+0xC0); D_1A9370 ->
 * D_1A9430 (+0xC0); D_001A7210+0x68 -> D_001A7290+0x68 (+0x80); g_frameArenaBase ->
 * g_nVendorBuyQuantity+0x58, g_sceneArenaCursor -> +0x68 (delta 0). jal ORDER:
 * WaitFrameDmaFence->WaitFrameDmaFence, DebugPrintStub->func_0026FD28, func_00294E98->
 * func_00294EF8, func_0029DDE8->func_0029D948, func_00132858->func_001328B8,
 * snd_Pump/snd_BankLoadFromIOP unchanged, func_00132828->func_00132888,
 * LoadMobyClassFromWad->LoadMobyClassFromWad. Record/TOC/listener offsets unchanged. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00295298);
#else
extern u8   D_00180000[];               /* +0x86E0 = g_listenerPosHistory (EU) */
extern u8   g_nVendorBuyQuantity[];     /* +0x58 g_frameArenaBase, +0x68 g_sceneArenaCursor */
extern u8   D_001A7290[];               /* +0x68 = IOP sound-bank staging base (EU) */
extern s32  D_1A9470;                   /* default sound-object stand-in (gp_rel; USA D_1A93B0) */
extern u8   D_1A9430[];                 /* eviction debug fmt string (USA D_1A9370) */
extern void WaitFrameDmaFence(s32 mask);            /* WaitFrameDmaFence */
extern void func_0026FD28(void *fmt, s32 classId);  /* DebugPrintStub (retail no-op) */
extern void func_00294EF8(s32 a, s32 b);        /* func_00294E98 */
extern s32  func_0029D948(void *dst, void *src, s32 count);  /* func_0029DDE8 */
extern void func_001328B8(s32 handle);          /* func_00132858 */
extern void func_00132888(void);                /* func_00132828 */
extern s32  snd_BankLoadFromIOP(void *addr);
extern s32  snd_Pump(void);
extern void LoadMobyClassFromWad(s32 classId, s32 tocIdx, void *buf);  /* LoadMobyClassFromWad */

void func_00295298(s32 classId) {
    u8  *toc     = D_0014B5C0;
    u8  *rec     = D_0014B5C0 + 0x7790;
    s32 *tocBase = (s32 *)(toc + 0x4B40);   /* gadget-class TOC */
    s32 *req;
    s32  tocIdx, slot;
    u8  *buf;

    /* 1. locate the class in the gadget-class TOC */
    if (tocBase[0] == classId) {
        tocIdx = 0;
    } else {
        s32 *p = tocBase;
        for (tocIdx = 1; tocIdx < 0x30; tocIdx++) {
            p += 5;
            if (p[0] == classId) {
                break;
            }
        }
    }
    if (tocIdx == 0x30) {
        return;
    }
    if (tocIdx == *(s16 *)(rec + 0x14)) {
        return;
    }

    /* 2. record current + find its resident buffer slot (of 3) */
    *(s16 *)(rec + 0x14) = (s16)tocIdx;
    req = (s32 *)(rec + 0x34);
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
    if (slot == 3) {                                 /* not resident -> evict */
        func_0026FD28(D_1A9430, classId);
        func_00294EF8(classId, 0);
        slot = 0;
    }

    /* 3. destination buffer for this slot */
    *(s16 *)(rec + 0x16) = (s16)slot;
    buf = rec + 0x40 + slot * 0xC800;
    if ((s32)buf <= 0xFFFFF) {                        /* never taken in practice */
        void *scratch = (void *)(*(s32 *)(g_nVendorBuyQuantity + 0x58) +
                                 *(s32 *)(g_nVendorBuyQuantity + 0x68) - 0xC800);
        func_0029D948(scratch, buf, 0xC80);
        buf = scratch;
    }

    /* 4. repoint stale listener-history sound objects */
    {
        u8 *e   = D_00180000 + 0x86E0;
        u8 *end = D_00180000 + 0x86E0 + 0x16C0;
        do {
            void *q = *(void **)(e + 0x78);
            if (q != 0 && *(s32 *)((u8 *)q + 0x1C) == 4) {
                *(void **)(e + 0x78) = &D_1A9470;
            }
            e += 0x70;
        } while (e < end);
    }

    /* 5. swap the sound bank */
    if (*(s32 *)(rec + 0x30) != 0) {
        func_001328B8(*(s32 *)(rec + 0x30));
        while (snd_Pump() != 0) {
        }
    }
    if (*(s32 *)(toc + 0x4B50 + tocIdx * 0x14) != 0) {
        s32 h = snd_BankLoadFromIOP(*(u8 **)(D_001A7290 + 0x68) + slot * 0xC800);
        *(s32 *)(rec + 0x30) = h;
        *(s32 *)(D_00180000 + 0x86E0 + 0x17B0) = h;
    } else {
        *(s32 *)(rec + 0x30) = 0;
    }

    /* 6. finalise + load the class */
    func_00132888();
    LoadMobyClassFromWad(classId, tocIdx, buf);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002954D8);

/* func_00295550 (EU twin of USA func_002954F0) — allocate VRAM for a texture and
 * queue its upload. Reserves a region from g_vramAllocCursor (0x400 header page +
 * 2^(logW+logH) texel data), builds the 64-bit GS TEX0 register (TBP0/TBW/TW/TH
 * from the log2 dims plus the fixed 0x1300000 / 0x8000<<19 / sign bits), and — when
 * the upload queue has room (<0x40) — appends a descriptor to g_texUploadQueue
 * (source desc+0x20, dest desc+0x420, both VRAM cursors, log2 dims). Returns the
 * packed TEX0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002954F0 (raw .s): Log2Floor -> func_002832D8;
 * g_vramAllocCursor -> D_001A7308+0x48; g_texUploadCount -> D_001B1380+0x27C;
 * g_texUploadQueue -> g_nVendorBuyQuantity+0x70F8; queue-record offsets
 * 0x0/0x4/0x6/0x8/0xC/0xD/0xE unchanged; constants region-invariant. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00295550);
#else
extern u8   D_001A7308[];               /* +0x48 = g_vramAllocCursor */
extern u8   D_001B1380[];               /* +0x27C = g_texUploadCount */
extern u8   g_nVendorBuyQuantity[];     /* +0x70F8 = g_texUploadQueue */
extern s32  func_002832D8(s32 v);       /* Log2Floor */

u64 func_00295550(void *descArg) {
    u8 *desc    = (u8 *)descArg;
    s32 logW    = func_002832D8(*(s32 *)(desc + 0x8));
    s32 logH    = func_002832D8(*(s32 *)(desc + 0xC));
    s32 cursor  = *(s32 *)(D_001A7308 + 0x48);
    s32 cursor2 = cursor + 0x400;
    s32 shift   = (logW < 6) ? 0 : (logW - 6);
    u64 packed  = (u64)(u32)(cursor2 >> 8)
                | ((u64)(u32)(1 << shift) << 14)
                | (((u64)(u32)logW << 26) | 0x1300000)
                | ((u64)(u32)logH << 30)
                | ((u64)(u32)(cursor >> 8) << 37)
                | ((u64)0x8000 << 19)
                | ((u64)-1 << 63);

    *(s32 *)(D_001A7308 + 0x48) = cursor2 + (1 << (logW + logH));

    if (*(s32 *)(D_001B1380 + 0x27C) < 0x40) {
        s32  count = *(s32 *)(D_001B1380 + 0x27C);
        s32 *e = (s32 *)((g_nVendorBuyQuantity + 0x70F8) + count * 0x10);
        e[0] = (s32)(desc + 0x20);
        *(s16 *)((u8 *)e + 0x6) = (s16)(cursor >> 8);
        *(s16 *)((u8 *)e + 0x4) = 0;
        e[2] = (s32)(desc + 0x420);
        *(s16 *)((u8 *)e + 0xE) = (s16)(cursor2 >> 8);
        *(u8 *)((u8 *)e + 0xC) = (u8)logW;
        *(u8 *)((u8 *)e + 0xD) = (u8)logH;
        *(s32 *)(D_001B1380 + 0x27C) = count + 1;
    }
    return packed;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", QueueGsTextureUpload);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapAllocCacheSlot);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapMoveCacheSlot);

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

/* MapDataExistsForLevel(levelAndFlag) — does map data exist for the given level?
 * The 0x100 flag selects the primary map-data TOC when set (+0x14F4 sector count),
 * the secondary (+0x15D4) when clear; low byte is the level (stride 8 into g_discToc).
 * Returns 1 if the sector count > 0. Matching arm stays INCLUDE_ASM; #else is the
 * structure model (register-coloring wall, same as USA — see USA twin).
 * Word-verified vs USA MapDataExistsForLevel (raw .s): only ref g_discToc ->
 * D_0014B5C0; offsets 0x14F4 / 0x15D4 and stride 8 unchanged (region delta is the
 * base +0x80 only). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapDataExistsForLevel);
#else
s32 MapDataExistsForLevel(s32 levelAndFlag) {
    s32  level = levelAndFlag & 0xFF;
    s32 *toc   = (s32 *)(D_0014B5C0 + level * 8);   /* g_discToc, stride 8 */
    if (levelAndFlag & 0x100) {
        return 0 < toc[0x14F4 / 4];
    }
    return 0 < toc[0x15D4 / 4];
}
#endif

/* MapFindNearestAvailableLevel — pick the level to upload next: try the current
 * level (with the active-set 0x100 flag) first; if not cached and it has map
 * data, use it. Otherwise spiral outward (+1,-1,+2,-2,+3,-3,+4) through the
 * level-order array from the current level's order index, returning the first
 * ordered level with map data that isn't already cached; else -1.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA MapFindNearestAvailableLevel: jal MapFindCacheSlot /
 * MapDataExistsForLevel (named, unchanged); cache currentLevel at
 * g_mapVertexData+0x230 (0x1C4FA0, USA g_mapCache 0x1C4F20, +0x80 lane).
 * REGION DELTA: g_mapDataSet -> D_1A7B85 (0x1A7B85, USA 0x1A7B05, +0x80);
 * g_pLevelOrder -> D_1AA590 (0x1AA590 ptr, USA 0x1AA510, +0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapFindNearestAvailableLevel);
#else
extern u8   D_1A7B85;         /* g_mapDataSet (EU; USA 0x1A7B05, +0x80) */
extern s32 *D_1AA590;         /* g_pLevelOrder (EU; USA 0x1AA510, +0x80) */
extern s32  MapFindCacheSlot(s32 levelAndFlag);

s32 MapFindNearestAvailableLevel(void) {
    s32 *cache = g_mapVertexData;
    s32  flag = (D_1A7B85 == 0) ? 0 : 0x100;
    s32  candidate = cache[0x230 / 4] + flag;
    s32  orderIndex;
    s32  step;
    s32  probe;

    if (MapFindCacheSlot(candidate) == -1 && MapDataExistsForLevel(candidate)) {
        return candidate;
    }

    orderIndex = 0;
    if (cache[0x230 / 4] < 0x1C) {
        s32 *p = D_1AA590;
        while (*p != cache[0x230 / 4]) {
            p++;
            orderIndex++;
        }
    }

    step = 1;
    probe = orderIndex + 1;
    do {
        if (probe >= 0 && probe < 0x1C && D_1AA590[probe] != 0) {
            candidate = D_1AA590[probe] + flag;
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

/* MapGetLevelOrderIndex(level) — index of `level` in the level-order array
 * (D_1AA590[28]). Returns 0 if it's the first entry, the matching index up to
 * 0x1B, or -1 if not found / the resolved entry is empty while the player has any
 * story progress. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA MapGetLevelOrderIndex: g_playerProgress -> gp_rel
 * g_playerProgress (named, unchanged). REGION DELTA: g_pLevelOrder -> D_1AA590
 * (0x1AA590, USA 0x1AA510, +0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapGetLevelOrderIndex);
#else
extern s32 *D_1AA590;        /* g_pLevelOrder (EU; USA 0x1AA510, +0x80) */

s32 MapGetLevelOrderIndex(s32 level) {
    s32 *order = D_1AA590;
    s32  idx;
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

/* MapEvictCacheSlot — choose a cache slot to reuse and mark it free, returning
 * its index. Both phases skip the locked slot (+0x2B0):
 *  - hub level (currentLevel==0): scan slots 4..0, return the first occupied
 *    (slotState!=0) slot already holding an unassigned id (-1).
 *  - otherwise resolve the current level's order index (return 1 if absent);
 *    scan slots 0..4: an occupied slot holding -1 is returned immediately; else
 *    pick the one whose level's order index is farthest from the current, mark it
 *    free (default 0) and return it. Matching arm stays INCLUDE_ASM; #else is the
 *    structure model. Word-verified vs USA MapEvictCacheSlot: jal
 *    MapGetLevelOrderIndex (named, unchanged); cache slotState/slotLevelId/
 *    lockedSlot/currentLevel at g_mapVertexData +0x288/+0x29C/+0x2B0/+0x230
 *    (offsets unchanged; base +0x80 lane). REGION DELTA (callee): abs()
 *    func_002835E0 -> func_002834F0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapEvictCacheSlot);
#else
extern s32 func_002834F0(s32 x);   /* integer abs (EU; USA func_002835E0) */

s32 MapEvictCacheSlot(void) {
    s32 *cache       = g_mapVertexData;
    s32 *slotState   = &cache[0x288 / 4];
    s32 *slotLevelId = &cache[0x29C / 4];
    s32  currentOrderIndex;
    s32  bestSlot;
    s32  maxDist;
    s32  i;

    if (cache[0x230 / 4] == 0) {
        for (i = 4; i >= 0; i--) {
            if (slotState[i] != 0 &&
                i != cache[0x2B0 / 4] &&
                slotLevelId[i] == -1) {
                return i;
            }
        }
    }

    currentOrderIndex = MapGetLevelOrderIndex(cache[0x230 / 4]);
    if (currentOrderIndex == -1) {
        return 1;
    }

    bestSlot = -1;
    maxDist = 0;
    for (i = 0; i < 5; i++) {
        s32 dist;
        if (slotState[i] == 0 || i == cache[0x2B0 / 4]) {
            continue;
        }
        if (slotLevelId[i] == -1) {
            return i;
        }
        dist = func_002834F0(MapGetLevelOrderIndex(slotLevelId[i] & 0xFF)
                             - currentOrderIndex);
        if (maxDist < dist) {
            maxDist = dist;
            bestSlot = i;
        }
    }

    if (bestSlot == -1) {
        bestSlot = 0;
    }
    slotLevelId[bestSlot] = -1;
    return bestSlot;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002964F0);

/* MapCompositeThumbnailMask: EU twin of USA 0x296498.
 * splat only emits an .s for a name it sees in an INCLUDE_ASM, so this
 * reference is what keeps the body in the tree (and promotes its interior
 * alabel to a real glabel). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapCompositeThumbnailMask);

/* MapSetCurrentLevel(level) — set the galactic-map current level and refresh the
 * availability flag: store `level` into the current-level slot, then call
 * MapUpdateLevelAvailability. Matching arm stays INCLUDE_ASM; #else is the
 * structure model (store-into-jal-delay scheduling wall, same as USA).
 * Word-verified vs USA MapSetCurrentLevel: jal MapUpdateLevelAvailability
 * (named, unchanged); the current-level field is labeled g_mapCurrentLevel in
 * the EU .s (== g_mapVertexData+0x230 = 0x1C51D0, USA g_mapCache.currentLevel
 * 0x1C5150, +0x80 lane). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapSetCurrentLevel);
#else
extern s32 g_mapCurrentLevel;         /* == g_mapVertexData+0x230 (EU 0x1C51D0) */
s32 MapUpdateLevelAvailability(void); /* defined below */

void MapSetCurrentLevel(s32 level) {
    g_mapCurrentLevel = level;
    MapUpdateLevelAvailability();
    __asm__ __volatile__("");
}
#endif

/* MapUpdateLevelAvailability — clamp the current level to 0..0x1B, set the cache
 * `available` flag from MapDataExistsForLevel(currentLevel), then force it clear
 * on the hub level (0) with any story progress. Returns available!=0. Matching
 * arm stays INCLUDE_ASM; #else is the structure model (bnel/clamp scheduling
 * wall, same as USA). Word-verified vs USA MapUpdateLevelAvailability: jal
 * MapDataExistsForLevel + gp/abs g_playerProgress (named, unchanged); cache
 * currentLevel/available at g_mapVertexData +0x230/+0x24 (offsets unchanged;
 * base +0x80 lane). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapUpdateLevelAvailability);
#else
s32 MapUpdateLevelAvailability(void) {
    s32 *cache = g_mapVertexData;
    if (cache[0x230 / 4] >= 0x1C) {
        cache[0x230 / 4] = 0x1B;
    }
    cache[0x24 / 4] = MapDataExistsForLevel(cache[0x230 / 4]) ? 1 : 0;
    if (cache[0x230 / 4] == 0 && g_playerProgress != 0) {
        cache[0x24 / 4] = 0;
    }
    return cache[0x24 / 4] != 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapUpdate);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapDraw);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00297B78);

/* MapBuildBitmapFrom4bpp(dest, srcA, srcB) — convert a 256x256 4bpp map image
 * into the packed 1bpp minimap bitmap `dest`, in a 16-row-band swizzled layout.
 * Per row: decode the row's 4bpp pixels into scratchpad 0x70000000
 * (func_00297EB0), then for each of 32 column-blocks pack 32 pixels into one
 * 32-bit word (bit set when the pixel nibble is nonzero). Output word (row,col)
 * lands at dest + 4*((row%16) + 512*(row/16)) + col*0x40. Matching arm stays
 * INCLUDE_ASM; #else is the structure model (strength-reduction near-miss, same
 * as USA). Word-verified vs USA MapBuildBitmapFrom4bpp: only callee
 * func_00297E80 -> func_00297EB0; scratchpad 0x70000000 region-invariant. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapBuildBitmapFrom4bpp);
#else
extern void func_00297EB0(void *scratchpad, s32 row, void *srcA, void *srcB);

void MapBuildBitmapFrom4bpp(void *destArg, void *srcA, void *srcB) {
    u8 *dest = (u8 *)destArg;
    s32 row;

    for (row = 0; row < 0x100; row++) {
        s32  band = row >> 4;             /* 16-row band index */
        s32  sub  = row - (band << 4);    /* row within band (row % 16) */
        s32 *out  = (s32 *)(dest + ((sub + (band << 9)) << 2));
        s32  col;

        func_00297EB0((void *)0x70000000, row, srcA, srcB);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00297EB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00297FC8);

/* MapBuildBitmap(dst, src, arg3) — dispatch the map-outline bitmap fill. After a
 * (no-op) prep call and only when map data is loaded (g_mapHasData != 0), route
 * by the source descriptor's low flag bit: bit0 set -> packed path
 * func_00298108(dst, src, arg3); bit0 clear -> plain path func_00298338(dst,
 * src). Matching arm stays INCLUDE_ASM; #else is the structure model (beql
 * early-out + callee-save wall, same as USA). Word-verified vs USA
 * MapBuildBitmap: g_mapHasData (named, unchanged); jal ORDER func_00298AA0 ->
 * func_00298AD0, func_002980D8 -> func_00298108, func_00298308 -> func_00298338. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", MapBuildBitmap);
#else
extern s32  g_mapHasData;
extern void func_00298AD0(void);                          /* no-op prep (defined below) */
extern void func_00298108(void *dst, u8 *src, s32 arg3);  /* packed path */
extern void func_00298338(void *dst, u8 *src);            /* plain path (defined below) */

void MapBuildBitmap(void *dst, u8 *src, s32 arg3) {
    func_00298AD0();
    if (g_mapHasData != 0) {
        if ((src[0] & 1) != 0) {
            func_00298108(dst, src, arg3);
        } else {
            func_00298338(dst, src);
        }
    }
    __asm__ __volatile__("");
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298108);

/* func_00298338 (EU twin of USA func_00298308) — the "plain" 1bpp->4bpp
 * map-bitmap expander (the bit0-clear arm of MapBuildBitmap). Builds a 256-entry
 * lookup that fans each bit k of an input byte out to nibble k of a 32-bit word
 * (set -> 0xF), then for each of 128 rows expands the next 16 source bytes to
 * sixteen 32-bit words (64 bytes) and replicates that run four times (256 bytes)
 * into dst — a 4x horizontal scale. Consumes 128*16 = 2048 source bytes, writes
 * 128*256 = 32 KB. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00298308: only callee CopyQwords -> CopyQwords
 * (x4); region-agnostic body otherwise. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298338);
#else
extern void CopyQwords(void *dst, const void *src, s32 nbytes);   /* CopyQwords */

void func_00298338(void *dst, u8 *src) {
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298508);

/* func_00298510 (EU twin of USA func_002984E0) — downsample the loaded 4bpp map
 * source at g_mapBitmapBuffer[+4] into the 1bpp discovered-area bitmap `out`
 * (inverse of the 4bpp expand). Bands of four 64-byte input rows accumulate through
 * a 16-entry nibble-weight LUT (copied from D_1A97D8) into a 128-column sum buffer;
 * every fourth row is thresholded (>=8 -> set) and bit-packed 8 columns/byte to
 * sixteen output bytes. Consumes 32 KB, emits 2 KB, then clears bit0 of out[0].
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002984E0: g_mapBitmapBuffer (named, delta 0),
 * FillMemory32 (named), D_1A9718 -> D_1A97D8 (D_1A9xxx pool +0xC0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298510);
#else
extern u8   *g_mapBitmapBuffer;
extern u8    D_1A97D8[];  /* 64-byte per-nibble accumulation weight LUT (USA D_1A9718 +0xC0) */
extern void  FillMemory32(void *dst, u32 val, s32 len);
void func_00298510(u8 *out) {
    s32 acc[128];   /* per-column accumulator (sp[0..0x200)) */
    s32 lut[16];    /* nibble weight table (sp+0x200) */
    u8 *lb = (u8 *)lut;
    u8 *in = *((u8 **)&g_mapBitmapBuffer + 1);  /* g_mapBitmapBuffer[+4] */
    u8 *outp = out;
    s32 i;
    s32 j;

    for (j = 0; j < 0x40; j++) {
        lb[j] = D_1A97D8[j];
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298760);

/* func_002988F8 (EU twin of USA func_002988C8) — resolve a level UI parameter block
 * pointer. In the map/planet-select state (g_playerProgress == 0x14) it returns a
 * two-entry slot from D_1A94E8 chosen by the parity of the counter at
 * g_pointLights+0x2400; otherwise the per-index entry &D_255ED0[idx*0x10].
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002988C8: g_playerProgress (named, file-scope, delta 0);
 * g_pointLights+0x2400 -> g_nVendorBuyQuantity+0x12CF8 (engine anchor, delta 0);
 * gp_rel D_1A9428 -> D_1A94E8 (+0xC0 pool); D_255E50 -> D_255ED0 (+0x80 data). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_002988F8);
#else
extern u8   g_nVendorBuyQuantity[];  /* +0x12CF8 = g_pointLights+0x2400 (delta 0) */
extern s32  D_1A94E8;                /* gp-relative UI-slot base (USA D_1A9428 +0xC0) */
extern u8   D_255ED0[];              /* per-index UI element table (USA D_255E50 +0x80) */
void *func_002988F8(s32 idx) {
    if (g_playerProgress == 0x14) {
        s32 n = *(s32 *)(g_nVendorBuyQuantity + 0x12CF8);
        return (u8 *)&D_1A94E8 + ((n % 2) << 4);
    }
    return &D_255ED0[idx * 0x10];
}
#endif

/* func_00298948 (EU twin of USA func_00298918) — compute two scaled level-parameter
 * outputs. Resolves a level index (arg `level`, or current g_playerProgress when -1,
 * clamped to 0 outside [0,20]), looks up its parameter block (func_002988F8), and
 * writes two blended, 1/512-scaled values: *outX = (block[0]+block[1]*fa)/512,
 * *outY = (block[2]+block[3]*fb)/512.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00298918: g_playerProgress (named, file-scope, delta 0);
 * callee func_002988C8 -> EU twin func_002988F8 (jal order). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298948);
#else
extern void *func_002988F8(s32 idx);
void func_00298948(f32 *outX, f32 *outY, s32 level, f32 fa, f32 fb) {
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
    v0 = ((f32 *)func_002988F8(level))[0];
    v1 = ((f32 *)func_002988F8(level))[1];
    *outX = (v0 + v1 * fa) * (1.0f / 512.0f);
    v2 = ((f32 *)func_002988F8(level))[2];
    v3 = ((f32 *)func_002988F8(level))[3];
    *outY = (v2 + v3 * fb) * (1.0f / 512.0f);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298A28);

void func_00298AD0(void) {
}

/* func_00298AD8 (EU twin of USA func_00298F20) — save/load status-machine tick.
 * Raises the rain-active flag (g_pRainHeightmap+0x1C) when any area-load field is
 * pending (g_areaTable +0x16C/+0x10 nonzero, or +0x24 positive). Then honours
 * pending save/load transition bits in the status flags word
 * (g_nSaveLoadStatusCode+0x4): bit 0x80 -> enter state 0x15, bit 0x100 -> state
 * 0x14 (each clearing its bit and setting 0x40). Dispatches the per-state handler
 * from the jump table, then bumps the tick counter (g_pRainHeightmap+0x18) — or
 * resets it to 0 if the handler changed the state.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00298F20 (EU addr < USA — confirmed NOT a mis-map: EU
 * .s is instruction-for-instruction the USA shape, field offsets +0x16C/+0x10/+0x24,
 * flag bits 0x80/0x100/0x40, states 0x15/0x14):
 *   g_areaTable (USA 0x1393E0) -> D_139460 (EU 0x139460, region +0x80, raw D_ label);
 *   g_pRainHeightmap+0x18/+0x1C (USA 0x1B19A0) -> g_nBoltCounterDisplayed+0xF0/+0xF4
 *     (engine anchor, delta 0; g_nBoltCounterDisplayed=0x1B18C8);
 *   g_nSaveLoadStatusCode (named, delta 0); D_256050 jump table -> D_2560D0 (+0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/191240", func_00298AD8);
#else
extern u8   D_139460[];                 /* g_areaTable (USA 0x1393E0, EU +0x80) */
extern u8   g_nBoltCounterDisplayed[];  /* +0xF0 tick, +0xF4 rain-flag (g_pRainHeightmap) */
extern s32  g_nSaveLoadStatusCode[];    /* [0] = state code, [1] (+0x4) = flags */
extern void (*D_2560D0[])(void);        /* per-state handler jump table (USA D_256050 +0x80) */

void func_00298AD8(void) {
    s32 origState = g_nSaveLoadStatusCode[0];
    s32 flags;

    if (*(s32 *)(D_139460 + 0x16C) != 0 ||
        *(s32 *)(D_139460 + 0x10) != 0 ||
        *(s32 *)(D_139460 + 0x24) > 0) {
        *(s32 *)(g_nBoltCounterDisplayed + 0xF4) = 1;
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

    D_2560D0[g_nSaveLoadStatusCode[0]]();

    if (g_nSaveLoadStatusCode[0] == origState) {
        *(s32 *)(g_nBoltCounterDisplayed + 0xF0) += 1;
    } else {
        *(s32 *)(g_nBoltCounterDisplayed + 0xF0) = 0;
    }
}
#endif

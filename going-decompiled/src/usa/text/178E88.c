#include "common.h"

/* Blob-shadow subsystem state at 0x1B15BC. The queue count is at +0, the
 * pending screen-grab query count at +0x1C, and the enable flag at +0x2C.
 * Declared as an incomplete array so accesses stay absolute %hi/%lo under -G8
 * (the original never reaches this cluster gp-relative). */
extern s32 g_blobShadowCount[];
#define g_blobShadowEnabled (g_blobShadowCount[0xB])
#define g_screenGrabQueryCount (g_blobShadowCount[7])

/* Render-layer enable bitmask (gp-relative under -G8). */
extern s32 g_renderLayerMask;

/* Draw-env / frame helpers (defined elsewhere in the render unit). */
extern void BuildCameraProjection(void); /* 0x27B0A0 - rebuild projection/viewport */
extern void AppendDrawEnvContext1(void);
extern void AppendScreenClearPacket(s32 mode);
extern void RenderFrame(void);
extern void RenderMenuScreenWidgets(s32 which);
extern void func_0027B988(void);
extern void func_002832F8(void *arg);

/* Per-camera-slot flag table base (0x1B7E30). func_00279EE8 passes the slot at
 * +0xF8 (0x1B7F28); modelled as a byte base so the offset stays absolute.
 * func_00279CF0 writes a table of 0x28-byte records starting at +0x458
 * (0x1B8288). */
extern u8 g_cameraSlotActive[];
typedef struct { s32 f[10]; } CamSlotRecord; /* 0x28-byte slot descriptor */

/* Active language index (u8 at 0x1A7BBC); selects the per-language slot inside
 * the streaming text-table directory relocated by func_00279D88. */
extern u8 g_currentLanguage;

/* Per-glyph width lookup over a fixed-stride font table. Each table entry is
 * 4 bytes; the signed byte at +3 is the glyph advance width. func_0027F7A8
 * sums integer advances; func_0027F858 sums scaled (float) advances. */
extern s32 func_0027F7A8(const char *str, s32 maxChars, const void *glyphTable);
extern s32 func_0027F858(const char *str, s32 maxChars, const void *glyphTable,
                         f32 scale);

/* Font glyph-advance tables (large; declared as incomplete arrays so they stay
 * absolute %hi/%lo under -G8). */
extern u8 g_debugFontGlyphTable[];
extern u8 D_263B10[];
extern u8 D_264250[];

/* Word-fill of `len` bytes at `dst` with a 32-bit `pattern` (SDK helper). */
extern void FillMemory32(void *dst, u32 pattern, s32 len);

/* Draw-hook callback queues. Each subsystem keeps three parallel globals: a
 * count, a function-pointer table, and an argument table (registered by the
 * Add* helpers, run by the Run* drivers). Declared as incomplete arrays so
 * their accesses stay absolute %hi/%lo (not gp-small) under -G8. */
typedef void (*DrawHookFn)(void *arg);
extern s32 g_fxHooksPreCount[];
extern DrawHookFn g_fxHooksPreFuncs[];
extern void *g_fxHooksPreArgs[];
extern s32 g_fxHooksPostCount[];
extern DrawHookFn g_fxHooksPostFuncs[];
extern void *g_fxHooksPostArgs[];
extern s32 g_fxHooksLateCount[];
extern DrawHookFn g_fxHooksLateFuncs[];
extern void *g_fxHooksLateArgs[];
extern s32 g_drawHooksAfterTiesCount[];
extern DrawHookFn g_drawHooksAfterTiesFuncs[];
extern void *g_drawHooksAfterTiesArgs[];
extern s32 g_drawHooksAfterShrubsCount[];
extern DrawHookFn g_drawHooksAfterShrubsFuncs[];
extern void *g_drawHooksAfterShrubsArgs[];

/* Misc per-frame render state cleared by ResetPerFrameDrawQueues. The absolute
 * globals are incomplete arrays; the four sdata one-shots (D_1A86F4/8760/8770
 * and g_bWaterPoolActive) stay gp-small scalars. g_screenFadeWhite is the
 * white-flash level; its +4/+8 words are per-frame request slots. */
extern s32 g_screenFadeWhite[];
extern s32 g_occlusionOverrideMode[];
extern s32 g_pWaterWaveGrids[];
extern s32 g_bWaterWavesActive[];
extern s32 D_1A86F4;
extern s32 D_1A8760;
extern s32 D_1A8770;
extern s32 g_bWaterPoolActive;

/* Per-frame occlusion visibility state. g_occlusionMode: 0 = no data (all
 * visible), 2 = resolve from the level occlusion grid. g_occlusionVisMask is a
 * 0x80-byte (1024-bit) vis-group mask consumed by the tfrag/tie/moby culls. */
extern s32 g_occlusionMode[];
extern u8 g_occlusionVisMask[];
extern void ResolveOcclusionVisMask(void);

/* Pause/menu render-mode query (1 = world behind pause, 0 = menu widgets). */
extern s32 func_00286200(void);
extern void RenderSaveLoadStatusPopup(void);

/* Wrapped text-box renderer (the scaled core behind DrawTextBoxDefault).
 * The real fn takes SEVEN args: the five int slots, the glyph-metrics table
 * (D_263B10, passed in $9 on the 0x280B48 path), then the f32 scale — sig
 * recovered 2026-07-06 (fable, promo-d3 @4cb6be7). The portable #else chain
 * had DROPPED the glyph-table arg (func_00348DA0 dropped-arg class), running
 * native text draws through a garbage font table; the TARGET_NATIVE prototype
 * below carries it. The matching build never calls this in C (func_00280B20's
 * matching arm is INCLUDE_ASM), so its prototype text is left untouched. */
#ifdef TARGET_NATIVE
extern void func_00280550(s32 a, s32 b, s32 c, s32 d, s32 e, u8 *glyphTable,
                          f32 scale);
#else
extern void func_00280550(s32 a, s32 b, s32 c, s32 d, s32 e, f32 scale);
#endif

/* GS depth-range / register-packet helpers. AppendGsRegPacket takes a 64-bit
 * value (so the constants need the ori/dsll/ori zero-extend) plus a reg id. */
extern void func_002859E0(s32 zNearBits, s32 lo, s32 zScale, s32 enable);
extern void func_00285D48(s32 a, s32 b);
extern void AppendGsRegPacket(s32 regId, u64 value);

/* Frame VIF1/GS DMA chain write cursor (0x1B2228). Declared as an incomplete
 * array of one pointer so each access reloads it absolute (%hi/%lo), matching
 * the original's no-load-PRE codegen under -fno-gcse. */
extern u32 *g_frameDmaCursor[];
/* Prebuilt full-screen alpha-tint quad packet (referenced by address). */
extern u8 g_fullScreenTintPacket[];

/* GS window pixel offsets (absolute %hi/%lo). */
extern s32 g_gsPixelOffsetX[];
extern s32 g_gsPixelOffsetY[];

/* Active display geometry + the GS screen context the viewport is derived from
 * (func_0027B988). g_screenHeight is the base of {height, halfW, halfH}. */
extern u8  g_gsScreenContext[]; /* 0x1A6480 - dims at +0x150 (w) / +0x152 (h) */
extern s32 g_screenWidth[];     /* 0x1A7340 */
extern s32 g_screenHeight[];    /* 0x1A7344 */
extern f32 IntToFloat(s32 x);
extern void func_0027A550(void);
/* Append a 4-vertex flat (untextured-coord) sprite quad packet given a pointer
 * to four packed XYZ2 corner words and a TEX0 value. */
extern void func_0027F0A8(const u64 *corners, u64 tex0);

/* Scene-cast moby pointer table base (0x1B894C). The screen-grab/occlusion
 * query descriptor table lives at +0x44 (0x1B8990); modelled as a byte base so
 * the +0x44 offset compiles to the absolute %hi/%lo form. */
extern u8 g_sceneActorMobys[];

/* Scene-transition teardown/fade-out targets + callees. */
extern u8   g_memoryArenaTable[]; /* 0x1BAE40 memory-region table */
extern s32  g_sceneArenaCursor;   /* 0x1B2230 */
extern s32  D_1A8BC0;             /* 0x1A8BC0 scene-arena reserve */
extern s32  g_vramDynamicBase;    /* 0x1A72D4 VRAM dynamic region base */
extern f32  g_screenFadeBlack;    /* 0x1B1520 black-fade level 0..1 */
extern void StopAllSoundEmitters(void);     /* 0x2E6E18 */
extern void WaitFrameDmaFence(s32);
extern u32  WaitVblankGetField(s32);
extern void ResetFrameArenas(void);         /* text/1FCF48 */
extern void FadeOutToBlackBlocking(s32);    /* defined below as INCLUDE_ASM */
extern void func_002FCFC8(void);            /* text/1FCF48 SelectSceneArenaRegion */

/* SceneTransitionTeardownA: fence + vblank wait, bump the frame counter, set up
 * the scene-transition block at g_cameraSlotActive+0xD0 (arena halves +0x60000,
 * the work span +0xD0800, the old scene cursor -0x60000), reserve 0x2000
 * (D_1A8BC0), reset the scene cursor to 0x60000 + the frame arenas, stop all
 * sound emitters, stash g_vramDynamicBase, and force the black fade fully on. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00278EC0);
#else
void func_00278EC0(void) {
    u8 *cam = g_cameraSlotActive + 0xD0;
    s32 half0;
    WaitFrameDmaFence(1);
    WaitVblankGetField(0);
    half0 = *(s32 *)(g_memoryArenaTable + 0xC);
    (&g_renderLayerMask)[1] += 1;                         /* frame counter at +0x4 */
    *(s32 *)(cam + 0x14) = g_sceneArenaCursor - 0x60000;
    *(s32 *)(cam + 0x00) = half0 + 0x60000;
    *(s32 *)(cam + 0x04) = *(s32 *)(g_memoryArenaTable + 0x10) + 0x60000;
    *(s32 *)(cam + 0x08) = half0 + 0x60000 + 0xD0800;
    D_1A8BC0 = 0x2000;
    g_sceneArenaCursor = 0x60000;
    ResetFrameArenas();
    StopAllSoundEmitters();
    *(s16 *)(cam + 0x26) = 0;
    *(s32 *)(cam + 0x18) = g_vramDynamicBase;
    g_screenFadeBlack = 1.0f;
}
#endif

/* SceneTransitionFadeOut: fence wait, reselect the scene-arena region + reset
 * the frame arenas, save the camera-slot VRAM dynamic base into g_vramDynamicBase,
 * then run the 30-frame blocking fade to black. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00278F90);
#else
void func_00278F90(void) {
    WaitFrameDmaFence(1);
    func_002FCFC8();
    ResetFrameArenas();
    g_vramDynamicBase = *(s32 *)(g_cameraSlotActive + 0xE8);
    FadeOutToBlackBlocking(0x1E);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00278FD0);

/* func_00279CF0 - store a 0x28-byte (10-word) record into the camera-slot
 * record table at g_cameraSlotActive+0x458, indexed by `slot` (ignored when
 * >= 0x20). The ten fields arrive as the first eight register args plus two
 * stack args; `slot` is the trailing stack arg.
 * Near-miss: the original accesses the ten fields as ten separate parallel
 * globals (stride 0x28), which makes cc1 spread the record base across several
 * aliased registers and store through them in a rotating pattern; the single
 * struct-array access here keeps one base register. Correct C preserved as the
 * portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279CF0);
#else
void func_00279CF0(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g, s32 h,
                   s32 i, s32 j, u32 slot) {
    CamSlotRecord *recs = (CamSlotRecord *)(g_cameraSlotActive + 0x458);
    if (slot < 0x20) {
        recs[slot].f[0] = a;
        recs[slot].f[1] = b;
        recs[slot].f[2] = c;
        recs[slot].f[3] = d;
        recs[slot].f[4] = e;
        recs[slot].f[5] = f;
        recs[slot].f[6] = g;
        recs[slot].f[7] = h;
        recs[slot].f[8] = i;
        recs[slot].f[9] = j;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279D68);

/* func_00279D88 - relocate the freshly-streamed per-language text table. The
 * streaming directory lives at g_cameraSlotActive+0xD0: word[2] (+0x8) is the
 * load buffer base `dataBase`; the per-language offset word at
 * dataBase[g_currentLanguage] selects this language's sub-table `tbl`. tbl[0] is
 * the record count N; the N records begin at tbl+8 (16 bytes each). The first
 * word of every record holds a buffer-relative offset which is fixed up in place
 * by adding the sub-table base `tbl` so it becomes an absolute pointer. The
 * directory's cursor (+0xC) is set to the first record and the count cached at
 * +0x10. The count cache is written unconditionally (the original stores it in
 * the blez delay slot); when N <= 0 no records are relocated.
 * Near-miss: cc1 keeps the loop bound and the record cursor live across a
 * branch-likely (bnel) reload of +0xC each iteration; expressed straight here. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279D88);
#else
void func_00279D88(void) {
    u8 *dir = g_cameraSlotActive + 0xD0;
    s32 *dataBase = *(s32 **)(dir + 0x8);
    s32 offset = dataBase[g_currentLanguage];
    s32 *tbl = (s32 *)((u8 *)dataBase + offset);
    s32 count = tbl[0];
    s32 *records = tbl + 2;            /* tbl + 8 bytes */
    s32 i;

    *(s32 **)(dir + 0xC) = records;
    *(s32 *)(dir + 0x10) = count;     /* count cache: written even when N <= 0 */
    if (count > 0) {
        for (i = 0; i < count; i++) {
            s32 *rec = records + i * 4; /* 16-byte stride */
            rec[0] += (s32)tbl;         /* relocate offset -> absolute pointer */
        }
    }
}
#endif

/* func_00279E00 - advance a horizontally-scrolling text cursor across the
 * camera-slot record table, wrapping when a glyph run runs past the visible
 * extent. The streaming directory at g_cameraSlotActive+0xD0 holds the start
 * record index (s16 at +0x20) and a base advance (s16 at +0x22); the records
 * live in the 0x28-byte table at g_cameraSlotActive+0x458 with its entry count
 * at +0x500. *pIndex (a1) is seeded with the start index and *pCursor (a2) with
 * base+`scroll` (a0). Walking records forward: if the current record still fits
 * (rec.f[2] >= rec.f[1] + cursor) the routine returns 1 (still on this record);
 * otherwise, if the next record shares the same group id (f[0]) the cursor is
 * rewound by the run width and the index advanced, looping; any boundary
 * (negative/missing index, end of table, group change) returns 0 with the index
 * and cursor left at their last values.
 * Near-miss: cc1 threads the record cursor and reloaded count through
 * branch-likely (bnel) tails; expressed as a straight loop here. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279E00);
#else
s32 func_00279E00(s32 scroll, s32 *pIndex, s32 *pCursor) {
    u8 *dir = g_cameraSlotActive + 0xD0;
    CamSlotRecord *recs = (CamSlotRecord *)(g_cameraSlotActive + 0x458);
    s32 *count = (s32 *)(g_cameraSlotActive + 0x458 + 0x500);
    s32 idx;

    *pIndex = *(s16 *)(dir + 0x20);
    *pCursor = *(s16 *)(dir + 0x22) + scroll;

    idx = *pIndex;
    if (idx < 0 || idx >= *count) {
        return 0;
    }

    for (;;) {
        s32 cursor = *pCursor;
        s32 runStart = recs[idx].f[1];
        s32 runEnd = recs[idx].f[2];

        if (runEnd >= runStart + cursor) {
            return 1;                       /* cursor still inside this record */
        }
        if (idx + 1 >= *count) {
            return 0;                       /* no following record */
        }
        if (recs[idx].f[0] != recs[idx + 1].f[0]) {
            return 0;                       /* group id changes: stop here */
        }

        *pCursor = (cursor - 1) - (runEnd - runStart);
        if (*pIndex + 1 < 0) {
            return 0;
        }
        *pIndex = *pIndex + 1;
        idx = *pIndex;
        if (idx >= *count) {
            return 0;
        }
    }
}
#endif

/* func_00279EE8 - forward the camera slot at g_cameraSlotActive+0xF8 to
 * func_002832F8.
 * Near-miss: the pinned cc1 sibling-call-optimizes the lone tail call to
 * `j func_002832F8`, but the original keeps a full call+return frame. Correct C
 * preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279EE8);
#else
void func_00279EE8(void) {
    func_002832F8(g_cameraSlotActive + 0xF8);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279F08);

/* func_0027A0C8 globals (declared for the TARGET_NATIVE #else only). */
extern s32 g_playerProgress;      /* 0x1A79F8 story progress counter */
extern u8  g_platinumBoltFlags[]; /* 0x19B278; +0x230 (0x19B4A8) = per-progress 3-bit nibble-counter table, stride 0x400 */

/** func_0027A0C8 — read a progress-gated nibble counter and scale it. Index the
 *  per-progress nibble table (g_platinumBoltFlags+0x230, stride 0x400 keyed by
 *  g_playerProgress) at index>>1; for an odd index the high nibble ((byte>>4)&7)
 *  selects a slot in `table`, for an even index the raw byte is used, then return
 *  (value * mult) / 100. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A0C8);
#else
s32 func_0027A0C8(u8 *table, s32 mult, s32 index) {
    u8 *entry = g_platinumBoltFlags + 0x230 + g_playerProgress * 0x400 + (index >> 1);
    s32 value;
    if (index & 1) {
        value = table[(*entry >> 4) & 7];
    } else {
        value = *entry;
    }
    return (value * mult) / 100;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A130);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A138);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", BuildFrameViewMatrices);

/* Camera fog/particle setup driven by the underwater state. */
extern s32 g_bCameraUnderwater;   /* 0x1B5580 */
extern s32 g_particleFarFadeMax;  /* 0x1B1D20 far-fade clamp */
extern u8  D_1AD564[];            /* 0x1AD564 underwater fog params {b,b,b,_, f,f,f,f} */

/* func_0027A550: load the camera fog block (3 colour bytes + 4 floats) into the
 * camera/projection scratch (g_sceneActorMobys+0x674 = D_1B8FC0, +0x218..+0x238)
 * and set the particle far-fade clamp - from the underwater params (D_1AD564) +
 * fade 0x40000 when g_bCameraUnderwater, else the normal block (g_blobShadowCount
 * +0x4) + fade 0x1F4000 - then rebuild the camera projection and clear the
 * screen-grab pending word (g_blobShadowCount+0x18). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A550);
#else
void func_0027A550(void) {
    u8 *vp = g_sceneActorMobys + 0x674;
    u8 *f;
    if (g_bCameraUnderwater != 0) {
        f = D_1AD564;
        g_particleFarFadeMax = 0x40000;
    } else {
        f = (u8 *)g_blobShadowCount + 0x4;
        g_particleFarFadeMax = 0x1F4000;
    }
    *(s32 *)(vp + 0x230) = f[0];
    *(f32 *)(vp + 0x22C) = *(f32 *)(f + 0x10);
    *(s32 *)(vp + 0x234) = f[1];
    *(s32 *)(vp + 0x238) = f[2];
    *(f32 *)(vp + 0x218) = *(f32 *)(f + 0x4);
    *(f32 *)(vp + 0x21C) = *(f32 *)(f + 0x8);
    *(f32 *)(vp + 0x228) = *(f32 *)(f + 0xC);
    BuildCameraProjection();
    *(s32 *)((u8 *)g_blobShadowCount + 0x18) = 0;
}
#endif

/* Occlusion BSP-grid root pointer (g_renderTaskWorkBuf+0x4C == 0x1B1680).
 * Modelled as an incomplete array so the +0x4C access stays absolute %hi/%lo
 * under -G8. The pointed-to node tree is a 3-level (z,y,x) bucket grid: each
 * node is {u16 origin, u16 extent, u16 child[extent]} (all read via lhu) and the final x-level
 * holds a 0x80-byte cell index (0xFFFF = empty). */
extern s32 g_renderTaskWorkBuf[];
#define g_occlusionGridRoot ((u8 *)g_renderTaskWorkBuf[0x13])

/* LookupOcclusionGridCell - walk the per-frame occlusion bucket grid for the
 * integer cell coords (x,y,z) and return a pointer to the matching 0x80-byte
 * occlusion cell record, or NULL when any axis falls outside its node's
 * [origin, origin+extent) span or hits an empty child. The leaf cell base is
 * the grid root plus the root's first word (the cell-array offset).
 * Near-miss (best 69.7%): the original keeps the grid root pinned in one
 * register (deriving each node ptr from it) and emits the y-level "child
 * present" test as a branch-likely (`bnezl`), an asymmetric codegen shape that
 * clean structured C with three uniform if-returns does not reproduce. Correct
 * C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", LookupOcclusionGridCell);
#else
void *LookupOcclusionGridCell(s32 x, s32 y, s32 z) {
    u8 *root = g_occlusionGridRoot;
    u8 *leafBase = root + *(s32 *)root;
    u8 *node = root + 4;
    s32 idx;

    z -= *(u16 *)(node + 0);
    if (z < 0 || z >= *(u16 *)(node + 2)) return (void *)0;
    idx = *(u16 *)(node + 4 + z * 2);
    if (idx == 0) return (void *)0;

    node = root + idx * 4;
    y -= *(u16 *)(node + 0);
    if (y < 0 || y >= *(u16 *)(node + 2)) return (void *)0;
    idx = *(u16 *)(node + 4 + y * 2);
    if (idx == 0) return (void *)0;

    node = root + idx * 4;
    x -= *(u16 *)(node + 0);
    if (x < 0 || x >= *(u16 *)(node + 2)) return (void *)0;
    idx = *(u16 *)(node + 4 + x * 2);
    if (idx == 0xFFFF) return (void *)0;
    return leafBase + (idx << 7);
}
#endif

/** LookupNeighborOcclusionCell — resolve an occlusion mask from one of two grid
 *  cells, preferring by `bias`. When bias < 0.5 cell A (ax,ay,az) is tried first,
 *  otherwise cell B (bx,by,bz); the other cell is the fallback. Returns the first
 *  cell that resolves to a non-null occlusion mask (or the fallback's result). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", LookupNeighborOcclusionCell);
#else
void *LookupNeighborOcclusionCell(s32 ax, s32 ay, s32 az,
                                  s32 bx, s32 by, s32 bz, f32 bias) {
    void *mask;
    if (bias < 0.5f) {
        mask = LookupOcclusionGridCell(ax, ay, az);
        if (mask == 0) {
            mask = LookupOcclusionGridCell(bx, by, bz);
        }
    } else {
        mask = LookupOcclusionGridCell(bx, by, bz);
        if (mask == 0) {
            mask = LookupOcclusionGridCell(ax, ay, az);
        }
    }
    return mask;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", ResolveOcclusionVisMask);

/* RenderFrame pre-layer pass that produces the per-frame visibility bitmask:
 * mode 0 sets every bit (no occlusion data), mode 2 resolves it from the level
 * occlusion grid, any other mode leaves the previous mask.
 * Near-miss: the pinned cc1 sibling-call-optimizes both tail calls
 * (`j FillMemory32` / `j ResolveOcclusionVisMask`) instead of keeping the
 * original's call+return frame. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", UpdateOcclusionVisMask);
#else
void UpdateOcclusionVisMask(void) {
    if (g_occlusionMode[0] == 0) {
        FillMemory32(g_occlusionVisMask, -1, 0x80);
    } else if (g_occlusionMode[0] == 2) {
        ResolveOcclusionVisMask();
    }
}
#endif

/* InitScreenGeometry: one-time screen-geometry + projection-viewport init from
 * the GS screen context pixel dims (g_gsScreenContext +0x150 width / +0x152
 * height). Sibling of func_0027B988 (same idioms): publishes width/height + the
 * two half-extents to g_screenWidth / g_screenHeight[0..2], the four 12.4
 * fixed-point GS-window offsets (centred on 0x800) to g_gsPixelOffsetX[0] /
 * g_gsPixelOffsetY[0..2], and the float viewport scale + half-extents into the
 * camera/projection scratch (g_sceneActorMobys+0x674 = D_1B8FC0). Unlike
 * func_0027B988 it also seeds the fixed projection constants (+0xA0 32, +0xA4
 * 2^20, +0x1DC 2^19, +0x1E8 255) and does NOT call func_0027A550. The matching
 * build keeps the asm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", InitScreenGeometry);
#else
void InitScreenGeometry(void) {
    u8 *vp = g_sceneActorMobys + 0x674; /* D_1B8FC0 camera/projection scratch */
    s32 w16 = *(s16 *)(g_gsScreenContext + 0x150);
    s32 h16 = *(s16 *)(g_gsScreenContext + 0x152);
    s32 halfW = w16 >> 1;
    s32 halfH = h16 >> 1;
    f32 wHalf, hHalf;

    g_screenWidth[0] = w16;
    g_screenHeight[0] = h16;
    g_screenHeight[1] = halfW;   /* g_screenCenterDefaultX 0x1A7348 */
    g_screenHeight[2] = halfH;   /* g_screenCenterDefaultY 0x1A734C */
    g_gsPixelOffsetX[0] = (0x800 - halfW) << 4;
    g_gsPixelOffsetY[0] = (0x800 - halfH) << 4;
    g_gsPixelOffsetY[1] = (halfW + 0x800) << 4;   /* scissor max X 0x1A7358 */
    g_gsPixelOffsetY[2] = (halfH + 0x800) << 4;   /* scissor max Y 0x1A735C */

    wHalf = IntToFloat(w16) * 0.5f;
    hHalf = IntToFloat(h16) * 0.5f;

    *(f32 *)(vp + 0xA0)  = 32.0f;        /* 0x42000000 */
    *(f32 *)(vp + 0xA4)  = 1048576.0f;   /* 0x49800000 */
    *(f32 *)(vp + 0xB0)  = 0.62f;        /* 0x3F1EB852 - g_cameraProjScale init */
    *(f32 *)(vp + 0x200) = wHalf;
    *(f32 *)(vp + 0x204) = hHalf;
    *(f32 *)(vp + 0x208) = wHalf * 4.0f;
    *(f32 *)(vp + 0x20C) = hHalf * 4.0f;
    *(s32 *)(vp + 0x218) = 0;
    *(f32 *)(vp + 0x21C) = 524288.0f;    /* 0x49000000 */
    *(f32 *)(vp + 0x228) = 255.0f;       /* 0x437F0000 */
    *(s32 *)(vp + 0x22C) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", BuildCameraProjection);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027B858);

/* RecomputeScreenViewportFromGsContext: derive the active display geometry and
 * the 2D draw viewport from the GS screen context's pixel dimensions
 * (g_gsScreenContext +0x150 width / +0x152 height). Publishes width/height and
 * the half-extents to g_screenWidth / g_screenHeight[0..2], the four GS-window
 * pixel offsets (12.4 fixed-point, centred on 0x800) to g_gsPixelOffsetX /
 * g_gsPixelOffsetY[0..2], and the float viewport scale/half-extents into the
 * camera/projection scratch (g_sceneActorMobys+0x674 == D_1B8FC0: +0xB0 aspect
 * 0.62, +0x200/+0x204 half-extents, +0x208/+0x20C ×4), then runs func_0027A550.
 * The matching build keeps the asm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027B988);
#else
void func_0027B988(void) {
    u8 *vp = g_sceneActorMobys + 0x674; /* D_1B8FC0 camera/projection scratch */
    s32 w16 = *(s16 *)(g_gsScreenContext + 0x150);
    s32 h16 = *(s16 *)(g_gsScreenContext + 0x152);
    s32 halfW = w16 >> 1;
    s32 halfH = h16 >> 1;
    f32 wHalf, hHalf;

    g_screenHeight[0] = h16;
    g_gsPixelOffsetX[0] = (0x800 - halfW) << 4;
    g_gsPixelOffsetY[0] = (0x800 - halfH) << 4;
    g_gsPixelOffsetY[1] = (halfW + 0x800) << 4;
    g_gsPixelOffsetY[2] = (halfH + 0x800) << 4;
    *(f32 *)(vp + 0xB0) = 0.62f;
    g_screenWidth[0] = w16;
    g_screenHeight[1] = halfW;
    g_screenHeight[2] = halfH;

    wHalf = IntToFloat(w16) * 0.5f;
    *(f32 *)(vp + 0x200) = wHalf;
    hHalf = IntToFloat(h16) * 0.5f;
    *(f32 *)(vp + 0x204) = hHalf;
    *(f32 *)(vp + 0x20C) = hHalf * 4.0f;
    *(f32 *)(vp + 0x208) = wHalf * 4.0f;
    func_0027A550();
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", SetupGsDisplayBuffers);

/* Zero every per-frame draw-callback queue and one-shot fx slot: the fx
 * pre/post/late hook counts, the after-ties/after-shrubs draw-hook counts, the
 * blob-shadow count, the two screen-fade request words, three render-frame
 * one-shot slots, the occlusion override mode, the water wave/pool state and
 * its flags. Called at the top of each frame.
 * Near-miss: the original zeroes each absolute global via the assembler
 * `sw $0, sym` ($at-macro) form, re-materialising %hi per store; the pinned cc1
 * instead allocates a pool of GP registers and hoists/reorders the %hi
 * computations. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", ResetPerFrameDrawQueues);
#else
void ResetPerFrameDrawQueues(void) {
    g_fxHooksPreCount[0] = 0;
    g_drawHooksAfterTiesCount[0] = 0;
    g_drawHooksAfterShrubsCount[0] = 0;
    g_fxHooksPostCount[0] = 0;
    g_fxHooksLateCount[0] = 0;
    g_blobShadowCount[0] = 0;
    g_screenFadeWhite[1] = 0;
    g_screenFadeWhite[2] = 0;
    D_1A86F4 = 0;
    D_1A8760 = 0;
    D_1A8770 = 0;
    g_occlusionOverrideMode[0] = 0;
    g_pWaterWaveGrids[0] = 0;
    g_pWaterWaveGrids[1] = 0;
    g_bWaterWavesActive[0] = 0;
    g_bWaterPoolActive = 0;
}
#endif

extern u8 D_1391D0[]; /* prebuilt GS init packet A */
extern u8 D_139120[]; /* prebuilt GS init packet B */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AppendFrameInitGsState);
#else
/**
 * Append the per-frame GS-state init to the frame DMA chain: two DMATAG-ref
 * qwords that splice in the prebuilt packets D_1391D0 and D_139120, followed by
 * a GS register write (reg 0x3D) whose value packs the three scene-render words
 * at g_sceneActorMobys+0x674 (+0x230 | +0x234<<8 | +0x238<<16).
 */
void AppendFrameInitGsState(void) {
    u8 *p = (u8 *)g_frameDmaCursor[0];
    u8 *sceneState = g_sceneActorMobys + 0x674;
    u64 value;

    *(u32 *)(p + 0x00) = 0x30000013; /* DMATAG ref -> D_1391D0 */
    *(void **)(p + 0x04) = D_1391D0;
    *(u32 *)(p + 0x08) = 0;
    *(u32 *)(p + 0x0C) = 0x50000013;
    g_frameDmaCursor[0] = (u32 *)(p + 0x10);

    *(u32 *)(p + 0x10) = 0x3000000B; /* DMATAG ref -> D_139120 */
    *(void **)(p + 0x14) = D_139120;
    *(u32 *)(p + 0x18) = 0;
    *(u32 *)(p + 0x1C) = 0x5000000B;

    value = (u64)(u32)*(s32 *)(sceneState + 0x230)
          | ((u64)(u32)*(s32 *)(sceneState + 0x234) << 8)
          | ((u64)(u32)*(s32 *)(sceneState + 0x238) << 16);

    g_frameDmaCursor[0] = (u32 *)(p + 0x20);
    AppendGsRegPacket(0x3D, value);
}
#endif

/* func_0027BFA8: program the GS privileged display registers (0x12000000 page)
 * from the saved screen-context words. PMODE(0x00)=0xFFA1 enables read-circuit 1;
 * the DISPFB/DISPLAY pairs (0x20/0x70/0x80/0x90/0xA0) are loaded from
 * g_gsScreenContext +0x8/+0x10/+0x18; BGCOLOR(0xE0) and 0xD0 are cleared. */
void func_0027BFA8(void) {
    *(volatile u64 *)0x120000E0 = 0;
    *(volatile u64 *)0x12000000 = 0xFFA1;
    *(volatile u64 *)0x12000020 = *(u64 *)(g_gsScreenContext + 0x8);
    *(volatile u64 *)0x12000070 = *(u64 *)(g_gsScreenContext + 0x10);
    *(volatile u64 *)0x12000090 = *(u64 *)(g_gsScreenContext + 0x10);
    *(volatile u64 *)0x12000080 = *(u64 *)(g_gsScreenContext + 0x18);
    *(volatile u64 *)0x120000A0 = *(u64 *)(g_gsScreenContext + 0x18);
    *(volatile u64 *)0x120000D0 = 0;
}

/* SetupViewModelDepthRange - set a near/compressed GS depth range for the
 * held-item view-model pass so the weapon never clips into world geometry, then
 * restore the default ALPHA blend. The depth scale clamps the (zNear+lo) shift
 * amount to 0x10.
 * Near-miss: the pinned cc1 sibling-call-optimizes the trailing
 * AppendGsRegPacket(...) to `j AppendGsRegPacket` instead of the original's
 * call+return frame, and builds the 0x8000000044 constant via dsll32 rather
 * than the original's `ori 0x8000; dsll 24`. Correct C preserved as the
 * portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027C020);
#else
void func_0027C020(s32 zNearBits, s32 lo) {
    s32 shift = zNearBits + lo;
    if (shift > 0x10) {
        shift = 0x10;
    }
    func_002859E0(zNearBits, lo, ((0x3FF000 - (4 << shift)) >> 13) << 13, 1);
    AppendGsRegPacket(0x47, 0x30000);
    AppendGsRegPacket(0x42, 0x8000000044);
    func_00285D48(0x100, 0x100);
    AppendGsRegPacket(0x42, 0x8000000044);
}
#endif

/* Thin wrapper: queue the ctx1 (FRAME_1 / FB A) draw-env REF packet.
 * Near-miss: the pinned cc1 sibling-call-optimizes the lone tail call to
 * `j AppendDrawEnvContext1`, but the original keeps a full call+return frame
 * (no sibcall). Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027C0A8);
#else
void func_0027C0A8(void) {
    AppendDrawEnvContext1();
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027C0C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RenderFrame);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CAD8);

/* Render a frame with every layer except the HUD: clear the screen, set the
 * layer mask to 0x7F (all engine layers, HUD bits clear), then render.
 * Near-miss: cc1 sibling-call-optimizes the final RenderFrame() to
 * `j RenderFrame`, whereas the original keeps the call+return frame. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawFrameWithoutHud);
#else
void DrawFrameWithoutHud(void) {
    AppendScreenClearPacket(0);
    g_renderLayerMask = 0x7F;
    RenderFrame();
}
#endif

/* RenderPauseMenuOverlay - when the render-mode query returns 1, clear the
 * screen, force the layer mask to 0x100FF (full world + HUD) and re-render the
 * world behind the pause menu; when it returns 0, draw menu-screen widgets for
 * screen 1. Always finishes with the save/load status popup.
 * Near-miss: the pinned cc1 sibling-call-optimizes the trailing
 * RenderSaveLoadStatusPopup() to `j RenderSaveLoadStatusPopup` and reschedules
 * the $ra restore into the earlier branch delay slots; the original keeps the
 * call+return frame. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CB08);
#else
void func_0027CB08(void) {
    if (func_00286200() == 1) {
        AppendScreenClearPacket(0);
        g_renderLayerMask = 0x100FF;
        RenderFrame();
    } else if (func_00286200() == 0) {
        RenderMenuScreenWidgets(1);
    }
    RenderSaveLoadStatusPopup();
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CB70);

/* Render the front-end menu widgets for screen 0.
 * Near-miss: same lone-tail-call sibcall wall as func_0027C0A8 (cc1 emits
 * `j RenderMenuScreenWidgets`; the original keeps the call+return frame). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CB80);
#else
void func_0027CB80(void) {
    RenderMenuScreenWidgets(0);
}
#endif

/* 2D draw-batch / HUD texture-cache state (absolute %hi/%lo + a few gp scalars). */
extern void *g_2dBatchOpenTag;    /* saved DMA cursor at batch open */
extern s32 g_vramAllocCursor;     /* running VRAM alloc cursor */
extern s32 g_uiTextureCount;      /* live UI texture-cache entry count */
extern u8  g_uiTextureCache[];    /* UI texture cache, 0x10-byte entries */
extern s32 g_texUploadCount;
extern u64 g_screenGrabTex0Full;  /* screen-grab tex0 (full / half res) */
extern u64 g_screenGrabTex0Half;
extern s32 D_1A8790;
extern s32 g_vramFrameBufB;       /* back framebuffer VRAM base */
extern u8 *g_hudTextureSlots;     /* 8-byte slots; +0x4 = VRAM block */
extern u8 *g_hudClutSlots;
extern void *g_pHudAssetHeader[]; /* [0] = header base (+0x24 clut count, +0x44 tex count) */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", Begin2dDrawBatch);
#else
/**
 * Open a 2D draw batch: stash the current frame DMA cursor as the batch open tag,
 * reserve a tag qword, reset the VRAM alloc cursor to the dynamic base, and clear
 * the texture-upload count. Then flush per-batch caches: zero every live
 * g_uiTextureCache entry and the two screen-grab tex0 handles.
 *
 * Unless `skipHudReset` is set, also evicts stale HUD textures: every
 * g_hudTextureSlots entry whose VRAM block is at/above the back framebuffer has
 * its +0x4 address cleared, and all g_hudClutSlots VRAM addresses are cleared
 * (counts from the HUD asset header +0x44 / +0x24).
 */
void Begin2dDrawBatch(s32 skipHudReset) {
    u32 *cursor = g_frameDmaCursor[0];
    s32 n;
    s32 i;

    g_2dBatchOpenTag = cursor;
    g_frameDmaCursor[0] = (u32 *)((u8 *)cursor + 0x10);
    g_vramAllocCursor = g_vramDynamicBase;
    g_texUploadCount = 0;

    n = g_uiTextureCount;
    for (i = 0; i < n; i++) {
        *(u64 *)(g_uiTextureCache + i * 0x10) = 0;
    }

    g_screenGrabTex0Full = 0;
    g_screenGrabTex0Half = 0;
    D_1A8790 = 0;

    if (skipHudReset != 0) {
        return;
    }

    n = *(s32 *)((u8 *)g_pHudAssetHeader[0] + 0x44);
    for (i = 0; i < n; i++) {
        u8 *slot = g_hudTextureSlots + i * 8;
        if (*(u16 *)(slot + 0x4) >= (g_vramFrameBufB >> 8)) {
            *(s16 *)(slot + 0x4) = 0;
        }
    }

    n = *(s32 *)((u8 *)g_pHudAssetHeader[0] + 0x24);
    for (i = 0; i < n; i++) {
        *(s16 *)(g_hudClutSlots + i * 8 + 0x4) = 0;
    }
}
#endif

extern void *g_2dBatchCloseTag; /* saved DMA cursor at batch close */
extern void FlushPendingTexUploads(void);
extern void AppendTexFlushDefaultTex0(void);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", End2dDrawBatch);
#else
/**
 * Close the 2D draw batch opened by Begin2dDrawBatch by back-patching the DMA
 * chain. Reserves a close tag at the current cursor, patches the open tag to
 * jump past it, flushes pending texture uploads (which append their own
 * packets), splices a tag that jumps back into the batch body, and finally
 * completes the close tag to continue the chain. All spliced tags are DMATAG
 * next (0x20000000) qwords.
 */
void End2dDrawBatch(void) {
    u8 *openTag = (u8 *)g_2dBatchOpenTag;
    u8 *cur;
    u8 *mid;

    g_2dBatchCloseTag = g_frameDmaCursor[0];
    cur = (u8 *)g_frameDmaCursor[0];
    g_frameDmaCursor[0] = (u32 *)(cur + 0x10); /* reserve the close tag */

    /* back-patch the open tag to jump over the reserved close tag */
    *(u32 *)(openTag + 0x0) = 0x20000000;
    *(void **)(openTag + 0x4) = g_frameDmaCursor[0];
    *(u32 *)(openTag + 0x8) = 0;
    *(u32 *)(openTag + 0xC) = 0;

    FlushPendingTexUploads();
    AppendTexFlushDefaultTex0();

    /* after the tex flush, splice a tag that jumps back into the batch body */
    mid = (u8 *)g_frameDmaCursor[0];
    *(u32 *)(mid + 0x0) = 0x20000000;
    *(void **)(mid + 0x4) = openTag + 0x10;
    *(u32 *)(mid + 0x8) = 0;
    *(u32 *)(mid + 0xC) = 0;
    g_frameDmaCursor[0] = (u32 *)(mid + 0x10);

    /* finalise the close tag to continue the chain */
    *(u32 *)((u8 *)g_2dBatchCloseTag + 0x0) = 0x20000000;
    *(void **)((u8 *)g_2dBatchCloseTag + 0x4) = g_frameDmaCursor[0];
    *(u32 *)((u8 *)g_2dBatchCloseTag + 0x8) = 0;
    *(u32 *)((u8 *)g_2dBatchCloseTag + 0xC) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CDC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", GetUiTextureTex0);

/* Register a (func,arg) callback in the pre-particle fx draw queue (cap 0x40),
 * run by RunFxDrawHooksPreParticles. No-op when the queue is full.
 * Near-miss: the original loads the count's %hi into the value register and
 * re-materialises it ($at-macro store) for the final count update; the pinned
 * cc1 keeps the %hi in a second register across the body and reuses it (a
 * register-allocation choice -fno-gcse does not suppress). Correct C preserved
 * as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AddFxDrawHookPreParticles);
#else
void AddFxDrawHookPreParticles(DrawHookFn func, void *arg) {
    s32 n = g_fxHooksPreCount[0];
    if (n < 0x40) {
        g_fxHooksPreFuncs[n] = func;
        g_fxHooksPreArgs[n] = arg;
        g_fxHooksPreCount[0] = n + 1;
    }
}
#endif

/* Run every registered pre-particle fx draw hook in order, each as
 * func(arg). The count is re-read each iteration so a hook may extend the
 * queue. Driver for AddFxDrawHookPreParticles.
 * Near-miss: the three-deep packed callee-save block (sd $16/$17/$18) + the
 * branch-likely re-test loop is a register/save-layout wall. Correct C
 * preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunFxDrawHooksPreParticles);
#else
void RunFxDrawHooksPreParticles(void) {
    s32 i;
    for (i = 0; i < g_fxHooksPreCount[0]; i++)
        g_fxHooksPreFuncs[i](g_fxHooksPreArgs[i]);
}
#endif

/* Register a (func,arg) callback in the after-ties draw queue (cap 0x40), run
 * by RunDrawHooksAfterTies. No-op when the queue is full.
 * Near-miss: same count-%hi register-allocation wall as
 * AddFxDrawHookPreParticles. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027D500);
#else
void func_0027D500(DrawHookFn func, void *arg) {
    s32 n = g_drawHooksAfterTiesCount[0];
    if (n < 0x40) {
        g_drawHooksAfterTiesFuncs[n] = func;
        g_drawHooksAfterTiesArgs[n] = arg;
        g_drawHooksAfterTiesCount[0] = n + 1;
    }
}
#endif

/* Run every registered after-ties draw hook in order as func(arg); the count is
 * re-read each iteration. Driver for func_0027D500 (AddDrawHookAfterTies).
 * Near-miss: same packed-save / branch-likely loop wall as
 * RunFxDrawHooksPreParticles. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunDrawHooksAfterTies);
#else
void RunDrawHooksAfterTies(void) {
    s32 i;
    for (i = 0; i < g_drawHooksAfterTiesCount[0]; i++)
        g_drawHooksAfterTiesFuncs[i](g_drawHooksAfterTiesArgs[i]);
}
#endif

/* Run every registered after-shrubs draw hook in order as func(arg); the count
 * is re-read each iteration.
 * Near-miss: same packed-save / branch-likely loop wall as
 * RunFxDrawHooksPreParticles. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunDrawHooksAfterShrubs);
#else
void RunDrawHooksAfterShrubs(void) {
    s32 i;
    for (i = 0; i < g_drawHooksAfterShrubsCount[0]; i++)
        g_drawHooksAfterShrubsFuncs[i](g_drawHooksAfterShrubsArgs[i]);
}
#endif

/* Register a (func,arg) callback in the post-particle fx draw queue (cap 0x40),
 * run by RunFxDrawHooksPostParticles. No-op when the queue is full.
 * Near-miss: same count-%hi register-allocation wall as
 * AddFxDrawHookPreParticles. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AddFxDrawHookPostParticles);
#else
void AddFxDrawHookPostParticles(DrawHookFn func, void *arg) {
    s32 n = g_fxHooksPostCount[0];
    if (n < 0x40) {
        g_fxHooksPostFuncs[n] = func;
        g_fxHooksPostArgs[n] = arg;
        g_fxHooksPostCount[0] = n + 1;
    }
}
#endif

/* Run every registered post-particle fx draw hook in order as func(arg); the
 * count is re-read each iteration. Driver for AddFxDrawHookPostParticles.
 * Near-miss: same packed-save / branch-likely loop wall as
 * RunFxDrawHooksPreParticles. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunFxDrawHooksPostParticles);
#else
void RunFxDrawHooksPostParticles(void) {
    s32 i;
    for (i = 0; i < g_fxHooksPostCount[0]; i++)
        g_fxHooksPostFuncs[i](g_fxHooksPostArgs[i]);
}
#endif

/* Register a (func,arg) callback in the small late fx draw queue (cap 4), run
 * by RunFxDrawHooksLate at the end of the RenderFrame fx layer. No-op when
 * full.
 * Near-miss: same count-%hi register-allocation wall as
 * AddFxDrawHookPreParticles. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AddFxDrawHookLate);
#else
void AddFxDrawHookLate(DrawHookFn func, void *arg) {
    s32 n = g_fxHooksLateCount[0];
    if (n < 4) {
        g_fxHooksLateFuncs[n] = func;
        g_fxHooksLateArgs[n] = arg;
        g_fxHooksLateCount[0] = n + 1;
    }
}
#endif

/* Run every registered late fx draw hook in order as func(arg); the count is
 * re-read each iteration. Driver for AddFxDrawHookLate (small cap-4 queue).
 * Near-miss: same packed-save / branch-likely loop wall as
 * RunFxDrawHooksPreParticles. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunFxDrawHooksLate);
#else
void RunFxDrawHooksLate(void) {
    s32 i;
    for (i = 0; i < g_fxHooksLateCount[0]; i++)
        g_fxHooksLateFuncs[i](g_fxHooksLateArgs[i]);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawBlobShadows);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", FadeOutToBlackBlocking);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027DB38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027DC40);

extern u8 D_1A7BB9;   /* fade-suppress flag byte */
extern u8 D_1AC860[]; /* GIFtag template A (16 bytes) */
extern u8 D_1AC870[]; /* GIFtag template B (16 bytes) */

/** func_0027DF80 — advance and render the white screen-fade overlay. Steps the
 *  fade counter (g_screenFadeWhite[2], at +0x8): when the fade-in flag (+0x4) is
 *  set it ramps up to 0x34, otherwise it ramps down to 0. Nothing is drawn at 0
 *  or when D_1A7BB9 suppresses it. Otherwise builds a full-screen white-quad GS
 *  packet (DMATAG + two patched GIFtag templates + a control qword + 8 XYZ2
 *  vertices spanning the GS pixel-offset screen extents), with the top/bottom
 *  edges pulled inward by counter*16 for the wipe animation. Advances
 *  g_frameDmaCursor by 0x80. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027DF80);
#else
void func_0027DF80(void) {
    s32 counter;
    u8 *p;
    u64 mask = 0x00FFFFF300000000ULL;
    s64 gsX, gsY0, gsY1, gsY2, cnt16;

    if (g_screenFadeWhite[1] != 0) {
        if (g_screenFadeWhite[2] < 0x34) {
            g_screenFadeWhite[2] = g_screenFadeWhite[2] + 1;
        }
    } else {
        if (g_screenFadeWhite[2] == 0) {
            return;
        }
        g_screenFadeWhite[2] = g_screenFadeWhite[2] - 1;
    }
    counter = g_screenFadeWhite[2];
    if (counter == 0 || D_1A7BB9 != 0) {
        return;
    }

    p = (u8 *)g_frameDmaCursor[0];
    *(u32 *)(p + 0x00) = 0x10000007; /* DMATAG cnt, 7 qwords */
    *(u32 *)(p + 0x04) = 0;
    *(u32 *)(p + 0x08) = 0;
    *(u32 *)(p + 0x0C) = 0x50000007;
    g_frameDmaCursor[0] = (u32 *)(p + 0x10);

    *(u64 *)(p + 0x10) = *(u64 *)(D_1AC860 + 0x00); /* GIFtag template A */
    *(u64 *)(p + 0x18) = *(u64 *)(D_1AC860 + 0x08);
    *(u16 *)(p + 0x10) = 0x8001;
    g_frameDmaCursor[0] = (u32 *)(p + 0x20);

    *(u64 *)(p + 0x20) = 0x104;
    *(u64 *)(p + 0x28) = 0x80000000ULL;
    g_frameDmaCursor[0] = (u32 *)(p + 0x30);

    *(u64 *)(p + 0x30) = *(u64 *)(D_1AC870 + 0x00); /* GIFtag template B */
    *(u64 *)(p + 0x38) = *(u64 *)(D_1AC870 + 0x08);
    *(u16 *)(p + 0x30) = 0x8008;
    g_frameDmaCursor[0] = (u32 *)(p + 0x40);

    gsX  = g_gsPixelOffsetX[0];
    gsY0 = g_gsPixelOffsetY[0];
    gsY1 = g_gsPixelOffsetY[1];
    gsY2 = g_gsPixelOffsetY[2];
    cnt16 = (s64)counter << 4;
    *(u64 *)(p + 0x40) = (u64)gsX  | ((u64)gsY0 << 16) | mask;
    *(u64 *)(p + 0x48) = (u64)gsX  | ((u64)(gsY0 + cnt16) << 16) | mask;
    *(u64 *)(p + 0x50) = (u64)gsY1 | ((u64)gsY0 << 16) | mask;
    *(u64 *)(p + 0x58) = (u64)gsY1 | ((u64)(gsY0 + cnt16) << 16) | mask;
    *(u64 *)(p + 0x60) = (u64)gsY1 | ((u64)gsY2 << 16) | mask;
    *(u64 *)(p + 0x68) = (u64)gsY1 | ((u64)(gsY2 - cnt16) << 16) | mask;
    *(u64 *)(p + 0x70) = (u64)gsX  | ((u64)gsY2 << 16) | mask;
    *(u64 *)(p + 0x78) = (u64)gsX  | ((u64)(gsY2 - cnt16) << 16) | mask;
    g_frameDmaCursor[0] = (u32 *)(p + 0x80);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E1E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E368);

/* DrawFullScreenTint - append a full-screen alpha-tint draw to the frame DMA
 * chain: emit a GS RGBAQ register write packed from the (r,g,b,a) components,
 * then a 4-word DMACnt+ref chain entry pointing at the prebuilt tint quad
 * packet, and advance the cursor by 0x10 bytes.
 * Near-miss: the pinned cc1 CSEs the frame-DMA-cursor pointer load across the
 * four chain-word stores, whereas the original reloads it absolute for each
 * write (no-load-PRE). No declaration reproduces the per-store reload here.
 * Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawFullScreenTint);
#else
void DrawFullScreenTint(u64 r, s64 g, s64 b, s64 a) {
    AppendGsRegPacket(1, r | g << 8 | b << 16 | a << 24);
    g_frameDmaCursor[0][0] = 0x30000014;
    g_frameDmaCursor[0][1] = (u32)g_fullScreenTintPacket;
    g_frameDmaCursor[0][2] = 0;
    g_frameDmaCursor[0][3] = 0x50000014;
    g_frameDmaCursor[0] += 4;
}
#endif

/** func_0027E4D0 — append a scissored screen-rect GS packet to the frame DMA
 *  chain: DMATAG (cnt, 5 qwords) + patched GIFtag template A (low halfword ->
 *  0x8001), a control qword (0x144 + `arg4`), patched GIFtag template B (low
 *  halfword -> 0x8004), then four rect vertices in XYZ2 format. Vertices span
 *  x in {x0,x1}, y in {y0,y1}: each coord is scaled x16, offset by the GS pixel
 *  origin, biased -8 (subpixel), with a fixed far-Z of 0x00FFFFF000000000.
 *  Advances g_frameDmaCursor by 0x60. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E4D0);
#else
void func_0027E4D0(s32 y0, s32 y1, s32 x0, s32 x1, u64 arg4) {
    u8 *p = (u8 *)g_frameDmaCursor[0];
    s32 offX = g_gsPixelOffsetX[0];
    s32 offY = g_gsPixelOffsetY[0];
    u64 z = 0x00FFFFF000000000ULL;
    s64 vx0, vx1, vy0, vy1;

    *(u32 *)(p + 0x00) = 0x10000005; /* DMATAG cnt, 5 qwords */
    *(u32 *)(p + 0x04) = 0;
    *(u32 *)(p + 0x08) = 0;
    *(u32 *)(p + 0x0C) = 0x50000005;
    g_frameDmaCursor[0] = (u32 *)(p + 0x10);

    *(u64 *)(p + 0x10) = *(u64 *)(D_1AC860 + 0x00); /* GIFtag template A */
    *(u64 *)(p + 0x18) = *(u64 *)(D_1AC860 + 0x08);
    *(u16 *)(p + 0x10) = 0x8001;                    /* patch low halfword */
    g_frameDmaCursor[0] = (u32 *)(p + 0x20);

    *(u64 *)(p + 0x20) = 0x144;
    *(u64 *)(p + 0x28) = arg4;
    g_frameDmaCursor[0] = (u32 *)(p + 0x30);

    *(u64 *)(p + 0x30) = *(u64 *)(D_1AC870 + 0x00); /* GIFtag template B */
    *(u64 *)(p + 0x38) = *(u64 *)(D_1AC870 + 0x08);
    *(u16 *)(p + 0x30) = 0x8004;                    /* patch low halfword */
    g_frameDmaCursor[0] = (u32 *)(p + 0x40);

    vx0 = x0 * 16 + offX - 8;
    vx1 = x1 * 16 + offX - 8;
    vy0 = (s64)(y0 * 16 + offY - 8) << 16;
    vy1 = (s64)(y1 * 16 + offY - 8) << 16;
    *(u64 *)(p + 0x40) = (u64)vx0 | (u64)vy0 | z; /* V0 (x0,y0) */
    *(u64 *)(p + 0x48) = (u64)vx1 | (u64)vy0 | z; /* V1 (x1,y0) */
    *(u64 *)(p + 0x50) = (u64)vx0 | (u64)vy1 | z; /* V2 (x0,y1) */
    *(u64 *)(p + 0x58) = (u64)vx1 | (u64)vy1 | z; /* V3 (x1,y1) */
    g_frameDmaCursor[0] = (u32 *)(p + 0x60);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E690);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawGlyphQuad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E818);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawTexturedQuad2d);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027EB20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawRotatedSprite2d);

extern u8 D_1AC930[]; /* prebuilt GIFtag template (16 bytes) */

/** func_0027EFA0 — append a 4-vertex textured-primitive GS packet to the frame
 *  DMA chain: a DMATAG (cnt, 9 qwords) + the prebuilt GIFtag at D_1AC930, then a
 *  leading control qword (mode ? 5 : 0, prim) and a 0x154 tag, followed by four
 *  vertices each packed as [st, uv, xyz2] (the two attribute arrays are s32[4]
 *  read stride-4; positions are u64[4] XYZ2). Advances g_frameDmaCursor by 0xA0.
 *  (st/uv naming inferred from the GS vertex layout.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027EFA0);
#else
void func_0027EFA0(const u64 *xyz2, const s32 *uv, const s32 *st, u64 prim, s32 mode) {
    u8 *p = (u8 *)g_frameDmaCursor[0];
    u8 *d;

    *(u32 *)(p + 0x00) = 0x10000009; /* DMATAG cnt, 9 qwords */
    *(u32 *)(p + 0x04) = 0;
    *(u32 *)(p + 0x08) = 0;
    *(u32 *)(p + 0x0C) = 0x50000009;
    g_frameDmaCursor[0] = (u32 *)(p + 0x10);

    *(u64 *)(p + 0x10) = *(u64 *)(D_1AC930 + 0x00); /* GIFtag template (16 bytes) */
    *(u64 *)(p + 0x18) = *(u64 *)(D_1AC930 + 0x08);
    g_frameDmaCursor[0] = (u32 *)(p + 0x20);

    d = p + 0x20;
    *(u64 *)(d + 0x00) = mode ? 5 : 0;
    *(u64 *)(d + 0x08) = prim;
    *(u64 *)(d + 0x10) = 0x154;
    *(s64 *)(d + 0x18) = st[0];
    *(s64 *)(d + 0x20) = uv[0];
    *(u64 *)(d + 0x28) = xyz2[0];
    *(s64 *)(d + 0x30) = st[1];
    *(s64 *)(d + 0x38) = uv[1];
    *(u64 *)(d + 0x40) = xyz2[1];
    *(s64 *)(d + 0x48) = st[2];
    *(s64 *)(d + 0x50) = uv[2];
    *(u64 *)(d + 0x58) = xyz2[2];
    *(s64 *)(d + 0x60) = st[3];
    *(s64 *)(d + 0x68) = uv[3];
    *(u64 *)(d + 0x70) = xyz2[3];
    *(u64 *)(d + 0x78) = 0;
    g_frameDmaCursor[0] = (u32 *)(p + 0xA0);
}
#endif

extern u8 D_1AC900[]; /* prebuilt GIFtag template (16 bytes) */

/** func_0027F0A8 — append a 4-corner sprite-quad GS packet to the frame DMA
 *  chain: a DMATAG (cnt, 5 qwords) + the prebuilt GIFtag at D_1AC900, then four
 *  interleaved data qwords built from the four `corners` XYZ2 words and two
 *  sign-extended coordinate words. NOTE: despite the `u64 tex0` param name (a
 *  reconstruction misnomer — see the forward decl above), arg1 is actually a
 *  POINTER: the callers (e.g. func_0027F168) pass an address in it and this
 *  function dereferences its low/high words. Advances g_frameDmaCursor by 0x60. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F0A8);
#else
void func_0027F0A8(const u64 *corners, u64 tex0) {
    u8 *p = (u8 *)g_frameDmaCursor[0];
    const s32 *coords = (const s32 *)(uintptr_t)tex0; /* arg1 is used as a pointer */

    *(u32 *)(p + 0x00) = 0x10000005; /* DMATAG cnt, 5 qwords */
    *(u32 *)(p + 0x04) = 0;
    *(u32 *)(p + 0x08) = 0;
    *(u32 *)(p + 0x0C) = 0x50000005;
    g_frameDmaCursor[0] = (u32 *)(p + 0x10);

    *(u64 *)(p + 0x10) = *(u64 *)(D_1AC900 + 0x00); /* GIFtag template (16 bytes) */
    *(u64 *)(p + 0x18) = *(u64 *)(D_1AC900 + 0x08);
    g_frameDmaCursor[0] = (u32 *)(p + 0x20);

    *(u64 *)(p + 0x20) = 0x4C;
    *(s64 *)(p + 0x28) = coords[0];
    *(u64 *)(p + 0x30) = corners[0];
    *(u64 *)(p + 0x38) = corners[2];
    *(s64 *)(p + 0x40) = coords[1];
    *(u64 *)(p + 0x48) = corners[1];
    *(u64 *)(p + 0x50) = corners[3];
    *(u64 *)(p + 0x58) = 0;
    g_frameDmaCursor[0] = (u32 *)(p + 0x60);
}
#endif

/* DrawFlatRect2d - build four packed XYZ2 corner words for an integer cell
 * rect (x1,y1)-(x2,y2) at depth `z` (each coord scaled *16, GS-window-offset,
 * biased -8) and hand them to the flat-sprite appender with TEX0 `tex0`.
 * Near-miss: the operations are byte-for-byte the same (sll/addu/dsll/or/sd),
 * but the pinned cc1 schedules the four corner builds and their stack stores in
 * a different order / register assignment than the original. Correct C
 * preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F168);
#else
void func_0027F168(s32 x1, s32 y1, s32 x2, s32 y2, s64 z, u64 tex0) {
    u64 corners[4];
    s64 vx2 = x2 * 0x10 + g_gsPixelOffsetX[0] - 8;
    s64 vy2 = (s64)(y2 * 0x10 + g_gsPixelOffsetY[0] - 8) << 0x10;
    s64 vx1 = x1 * 0x10 + g_gsPixelOffsetX[0] - 8;
    s64 vy1 = (s64)(y1 * 0x10 + g_gsPixelOffsetY[0] - 8) << 0x10;
    s64 vz = z << 0x20;
    corners[0] = vx1 | vy1 | vz;
    corners[3] = vx2 | vy2 | vz;
    corners[1] = vx2 | vy1 | vz;
    corners[2] = vx1 | vy2 | vz;
    func_0027F0A8(corners, tex0);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F208);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F348);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F4D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F4D8);

/* Enable blob shadows: set the enable flag and return 1 (success).
 * Wall (re-checked 2026-06-25): the enable flag lives at 0x1B15E8 and the
 * original stores it %gp_rel(g_blobShadowCount + 0x2C) - a GPREL16 reloc against
 * the g_blobShadowCount base + addend 0x2C. The rest of g_blobShadowCount (the
 * queue at +0x0/+0x4../+0x18) is accessed absolute %hi/%lo in this same unit, so
 * a single C declaration of g_blobShadowCount cannot be both gp-small (for the
 * flag) and large/absolute (for the queue) - and a separate gp-small symbol for
 * the flag emits a GPREL16 reloc against the WRONG symbol name (objdiff matches
 * relocs by symbol+addend, not resolved address). The original split the flag
 * into a distinct gp-small global that the linker happened to place at
 * g_blobShadowCount+0x2C; we cannot reproduce that reloc naming. (Secondary: cc1
 * also materialises the constant 1 twice here, one reg per use.) Genuine wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F790);
#else
s32 func_0027F790(void) {
    return (g_blobShadowEnabled = 1);
}
#endif

/* Disable blob shadows: clear the enable flag.
 * Wall (re-checked 2026-06-25): cc1 DOES pack the store into the jr delay slot
 * (`jr $31; sw $0,...`) exactly like the original - the delay-slot claim in the
 * old note was wrong. The genuine wall is the same gp_rel/symbol-naming split as
 * func_0027F790: the flag store is %gp_rel(g_blobShadowCount + 0x2C) but the
 * queue base is accessed absolute, unsplittable under one C symbol. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F7A0);
#else
void func_0027F7A0(void) {
    g_blobShadowEnabled = 0;
}
#endif

/* func_0027F7A8 (MeasureTextByGlyphTable): sum the signed per-glyph advance
 * widths of `str` - each glyph is 4 bytes in `glyphTable`, signed advance at +3 -
 * stopping at NUL or after `maxChars` chars (maxChars == -1 means until NUL). A
 * zero advance is skipped (movn). The current char's advance is folded in before
 * the maxChars return check (the movn sits in the beq delay slot). Worker behind
 * the three font wrappers below.
 * MATCHED (2026-06-25, revisit-upgrade from #else): the old near-miss split the
 * early-out into `return 0` blocks; the single merged tail (`return sum`, sum==0
 * on the early paths) plus reading the guard byte through `str` directly and
 * bumping `i` before the advance-fold reproduces the original prologue schedule
 * 1:1. Leaf, no saves - so no save-packing wall here. */
s32 func_0027F7A8(const char *str, s32 maxChars, const void *glyphTable) {
    const s8 *gt = (const s8 *)glyphTable;
    s32 sum = 0;
    s32 i = 0;
    if (maxChars != 0 && ((const u8 *)str)[0] != 0) {
        const u8 *s = (const u8 *)str;
        do {
            s8 advance = gt[s[0] * 4 + 3];
            s++;
            i++;
            if (advance != 0) sum += advance;
            if (i == maxChars) break;
        } while (s[0] != 0);
    }
    return sum;
}

/** Measure pixel width of a string in the D_263B10 font. */
s32 func_0027F7F8(const char *str, s32 maxChars) {
    return func_0027F7A8(str, maxChars, D_263B10);
}

/** Measure pixel width of a string in the debug font. */
s32 func_0027F818(const char *str, s32 maxChars) {
    return func_0027F7A8(str, maxChars, g_debugFontGlyphTable);
}

/** Measure pixel width of a string in the D_264250 font. */
s32 func_0027F838(const char *str, s32 maxChars) {
    return func_0027F7A8(str, maxChars, D_264250);
}

/* func_0027F858 (MeasureScaledTextByGlyphTable): the scaled sibling of
 * func_0027F7A8. Sum each glyph's signed advance width (4-byte entry, advance
 * at +3) as a float, multiply each by `scale`, accumulate, and round the total
 * back to an integer. Stops at NUL or after `maxChars` chars (the maxChars test
 * fires after folding the current glyph). Worker behind func_0027F900.
 * Near-miss: the four packed GPR saves ($16-$19) + two FPR saves ($f20/$f21)
 * and the IntToFloat-per-glyph call shape are a save-layout wall. Correct C
 * preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F858);
#else
extern s32 FloatToInt(f32 x);
s32 func_0027F858(const char *str, s32 maxChars, const void *glyphTable,
                  f32 scale) {
    const u8 *s = (const u8 *)str;
    const s8 *gt = (const s8 *)glyphTable;
    f32 acc = 0.0f;
    s32 i;
    if (maxChars != 0 && s[0] != 0) {
        i = 1;
        do {
            s8 advance = gt[s[0] * 4 + 3];
            s++;
            acc += IntToFloat(advance) * scale;
            if (i == maxChars) break;
            i++;
        } while (s[0] != 0);
    }
    return FloatToInt(acc);
}
#endif

/** Measure scaled pixel width of a string in the D_263B10 font. */
s32 func_0027F900(const char *str, s32 maxChars, f32 scale) {
    return func_0027F858(str, maxChars, D_263B10, scale);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawFixedFontString);

extern u64 GetUiTextureTex0(s32 slot);
extern void DrawFixedFontString(s32 a, s32 b, s32 c, s32 d, s32 e, u64 tex0, u8 *glyphTable);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027FBA8);
#else
/**
 * Draw a string with the D_263B10 font: resolve the UI texture (GetUiTextureTex0
 * slot 1) and forward the five caller args plus that tex0 and the D_263B10 glyph
 * metrics to DrawFixedFontString. Twin of DrawDebugString (which uses tex slot 2
 * and the g_debugFontGlyphTable).
 */
void func_0027FBA8(s32 a, s32 b, s32 c, s32 d, s32 e) {
    u64 tex0 = GetUiTextureTex0(1);
    DrawFixedFontString(a, b, c, d, e, tex0, D_263B10);
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawDebugString);
#else
/**
 * Draw a string with the built-in debug font: resolve the UI font texture
 * (GetUiTextureTex0 slot 2) and forward the five caller args plus that tex0 and
 * the g_debugFontGlyphTable glyph metrics to DrawFixedFontString.
 */
void DrawDebugString(s32 a, s32 b, s32 c, s32 d, s32 e) {
    u64 tex0 = GetUiTextureTex0(2);
    DrawFixedFontString(a, b, c, d, e, tex0, g_debugFontGlyphTable);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027FCA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027FCB0);

extern void func_0027FCB0(s32 a, s32 b, s32 c, u64 tex0, u8 *glyphTable, f32 f1, f32 f2, f32 f3); /* scaled/positioned font draw */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027FFF0);
#else
/**
 * Draw a string with the D_263B10 font and three float parameters (scale /
 * position): resolve the UI texture (GetUiTextureTex0 slot 1) and forward the
 * three int args, that tex0, the D_263B10 glyph metrics, and the three floats to
 * func_0027FCB0.
 */
void func_0027FFF0(s32 a, s32 b, s32 c, f32 f1, f32 f2, f32 f3) {
    u64 tex0 = GetUiTextureTex0(1);
    func_0027FCB0(a, b, c, tex0, D_263B10, f1, f2, f3);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280080);

/** func_00280120 — draw a right-aligned debug-font string. Measures the string's
 *  rendered width (func_0027F818(str, maxChars)), shifts the anchor x left by that
 *  width, resolves the UI font texture (GetUiTextureTex0 slot 2), and forwards to
 *  DrawFixedFontString with the debug glyph table. (DrawFixedFontString's d/e
 *  params carry the string/maxChars — its committed decl types them s32.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280120);
#else
void func_00280120(s32 x, s32 arg1, s32 arg2, const char *str, s32 maxChars) {
    s32 width = func_0027F818(str, maxChars);
    u64 tex0 = GetUiTextureTex0(2);
    DrawFixedFontString(x - width, arg1, arg2, (s32)(uintptr_t)str, maxChars, tex0,
                        g_debugFontGlyphTable);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002801B0);

/** func_00280250 — draw a horizontally-centered debug-font string. Measures the
 *  rendered width (func_0027F818), centers the anchor (x - width/2), draws via
 *  DrawFixedFontString (UI font slot 2, debug glyph table), and returns the
 *  centered x. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280250);
#else
s32 func_00280250(s32 x, s32 arg1, s32 arg2, const char *str, s32 maxChars) {
    s32 width = func_0027F818(str, maxChars);
    s32 cx = x - (width >> 1);
    u64 tex0 = GetUiTextureTex0(2);
    DrawFixedFontString(cx, arg1, arg2, (s32)(uintptr_t)str, maxChars, tex0,
                        g_debugFontGlyphTable);
    return cx;
}
#endif

/** func_002802E8 — draw a horizontally-centered string in the alternate UI font
 *  (glyph table D_264250, UI texture slot 3). Same centering as func_00280250:
 *  measure width (func_0027F838), center (x - width/2), draw, return centered x. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002802E8);
#else
s32 func_002802E8(s32 x, s32 arg1, s32 arg2, const char *str, s32 maxChars) {
    s32 width = func_0027F838(str, maxChars);
    s32 cx = x - (width >> 1);
    u64 tex0 = GetUiTextureTex0(3);
    DrawFixedFontString(cx, arg1, arg2, (s32)(uintptr_t)str, maxChars, tex0, D_264250);
    return cx;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280380);

extern f32 func_002804C0(f32 inputScale, const char *str, s32 maxChars, s32 count); /* text auto-scale (below) */
extern void func_00280380(s32 a, s32 b, s32 c, const char *str, s32 maxChars, f32 scale); /* scaled text draw */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280440);
#else
/**
 * Draw a string auto-scaled to fit `count` pixels: compute the horizontal fit
 * factor for (str, maxChars, count) at floor `inputScale` (func_002804C0), then
 * render the string at that scale via func_00280380.
 */
void func_00280440(f32 inputScale, s32 a, s32 b, s32 c, const char *str, s32 maxChars, s32 count) {
    f32 scale = func_002804C0(inputScale, str, maxChars, count);
    func_00280380(a, b, c, str, maxChars, scale);
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002804C0);
#else
/**
 * Compute a horizontal auto-scale factor to fit a string into `count` pixels.
 *
 * Measures the string's rendered width at scale 1.0 (func_0027F900, 0 for a null
 * string). If the string already fits (count >= width) returns 1.0; otherwise
 * returns width-fit ratio count/width, floored at `inputScale`.
 */
f32 func_002804C0(f32 inputScale, const char *str, s32 maxChars, s32 count) {
    s32 width = (str != 0) ? func_0027F900(str, maxChars, 1.0f) : 0;
    f32 result = 1.0f;

    if (count < width) {
        result = (f32)count / (f32)width;
        if (result < inputScale) {
            result = inputScale;
        }
    }
    return result;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280550);

/* DrawTextBoxDefault - the unscaled text-box entry: forward to the scaled core
 * with scale 1.0f. The real fn takes SIX args (layout, rgba, str, len, tex0,
 * glyphTable); it passes all six through and appends scale 1.0f into the SEVEN-
 * arg func_00280550. The portable #else had been 5-arg and silently DROPPED the
 * glyph-metrics table (the func_00280B48 path passes &D_263B10 in $9) — native
 * text drew with a garbage font table. #else sig corrected 2026-07-06 (fable
 * recovery, promo-d3 @4cb6be7); the arg is now forwarded.
 * Near-miss (matching arm): the pinned cc1 sibling-call-optimizes the lone tail
 * call to `j func_00280550`; the original keeps a full call+return frame.
 * Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280B20);
#else
void func_00280B20(s32 a, s32 b, s32 c, s32 d, s32 e, u8 *glyphTable) {
    func_00280550(a, b, c, d, e, glyphTable, 1.0f);
}
#endif

/* func_00280B48 - draw a string with the default UI font: resolve the font
 * page's GS TEX0 (GetUiTextureTex0 slot 1) and forward the four caller args
 * plus that tex0 and the D_263B10 glyph-metrics table to the text core
 * func_00280B20 (SIX args). Sig recovered 2026-07-06 (fable, promo-d3
 * @4cb6be7). Matching arm stays INCLUDE_ASM (byte-exact is walled by the
 * tier-wide 8-packed-callee-save frame fingerprint); the #else supplies the
 * portable body that passes the glyph table down the chain. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280B48);
#else
void func_00280B48(s32 a, s32 b, s32 c, s32 d) {
    u64 tex0 = GetUiTextureTex0(1);
    func_00280B20(a, b, c, d, (s32)tex0, D_263B10);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280BB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280C28);

/* InitTextBoxLayout - fill the 0x18-byte (12 s16) text-box layout descriptor
 * consumed by DrawWrappedTextBox. Fields: [0]clipX0 [1]clipX1 [2]left [3]right
 * [4]anchorX [5]y [8]lineHeight [9]flags; the out/shadow fields [6][7][10][11]
 * are zeroed. param_9 (flags) arrives on the stack. */
void func_00280C98(s16 *layout, s16 clipX0, s16 clipX1, s16 left, s16 right,
                   s16 anchorX, s16 y, s16 lineHeight, s32 flags) {
    layout[0] = clipX0;
    layout[1] = clipX1;
    layout[2] = left;
    layout[3] = right;
    layout[4] = anchorX;
    layout[5] = y;
    layout[8] = lineHeight;
    layout[9] = flags;
    layout[6] = 0;
    layout[7] = 0;
    layout[10] = 0;
    layout[11] = 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280CD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AppendVu1SphereMapContext);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280EC8);

/* Queue the ctx1 draw-env packet, then recompute the GS screen geometry.
 * Near-miss: cc1 sibling-call-optimizes the final func_0027B988() to
 * `j func_0027B988`, whereas the original keeps the call+return frame. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280FB8);
#else
void func_00280FB8(void) {
    AppendDrawEnvContext1();
    func_0027B988();
}
#endif

/* func_00280FE0 - per-frame reset of the offscreen-probe subsystem: zero the
 * 0x600-byte screen-grab/occlusion query descriptor table at 0x1B8990 and clear
 * the pending query count at 0x1B15D8.
 * Near-miss: the original clears the count via the assembler `sw $0, sym`
 * ($at-macro) absolute form and computes its %hi after restoring $ra, whereas
 * the pinned cc1 allocates a GP register for the count address and hoists the
 * %hi before the restore. Correct C preserved as the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280FE0);
#else
void func_00280FE0(void) {
    FillMemory32(g_sceneActorMobys + 0x44, 0, 0x600);
    g_screenGrabQueryCount = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00281010);

/** func_00281020 — occlusion visibility ratio for a scene actor. The actor's
 *  record (g_sceneActorMobys+0x44, stride 0x30, indexed by `actorIdx`) holds the
 *  query rect w/h at +0x24/+0x26, a depth threshold at +0x14, and a result-float
 *  pointer at +0x10. Count how many of the w*h read-back pixels have their 24-bit
 *  depth above the threshold, then store the *un*-exceeded fraction
 *  (total - count)/total to the result float. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00281020);
#else
void func_00281020(u32 *pixels, s32 actorIdx) {
    u8 *rec = g_sceneActorMobys + 0x44 + actorIdx * 0x30;
    s32 w = *(s16 *)(rec + 0x24);
    s32 h = *(s16 *)(rec + 0x26);
    s32 total = w * h;
    s32 count = 0;

    if (total > 0) {
        u32 threshold = *(u32 *)(rec + 0x14);
        s32 i;
        for (i = 0; i < total; i++) {
            if ((pixels[i] & 0xFFFFFF) > threshold) {
                count++;
            }
        }
    }
    **(f32 **)(rec + 0x10) = (f32)(total - count) / (f32)total;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002810C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002812A8);

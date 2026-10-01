#include "common.h"

/* Blob-shadow subsystem state at 0x1B15BC (0x2C bytes): the queue count is at
 * +0 and the pending screen-grab query count at +0x1C. Declared as an
 * incomplete array so accesses stay absolute %hi/%lo under -G8. */
extern s32 g_blobShadowCount[];
/* cc1-small / assembler-absolute view of the same cluster (the text/198FA0
 * model): an 8-byte alias makes cc1 emit the one-insn symbolic `sw $r, sym+off`
 * macro, and the .extern size override makes GNU as expand it absolutely
 * (`lui $at; sw %lo($at)`) — the original's form where func_00280FE0 clears
 * the pending query count. */
__asm__(".extern g_blobShadowCount, 16");
extern s32 g_blobShadowState[2] __asm__("g_blobShadowCount");
#define g_screenGrabQueryCount (g_blobShadowState[7])
/* The two fixed-font text flags that follow the cluster (FACT #5649; they are
 * NOT blob-shadow state): the inline colour-code enable at 0x1B15E8, written
 * by func_0027F790 / func_0027F7A0 and read by the text drawers, and the
 * "inside a wrapped text box" flag at 0x1B15EC. Both are gp-small scalars. */
extern s32 g_textColorCodeEnabled;
extern s32 g_bInWrappedTextBox;

/* Render-layer enable bitmask (gp-relative under -G8). */
extern s32 g_renderLayerMask;

/* Draw-env / frame helpers (defined elsewhere in the render unit). */
extern void BuildCameraProjection(void); /* 0x27B0A0 - rebuild projection/viewport */
extern void AppendDrawEnvContext1(void);
extern void AppendScreenClearPacket(s32 mode);
extern void RenderFrame(void);
extern void RenderMenuScreenWidgets(s32 which);
extern void RecomputeScreenViewportFromGsContext(void);
extern void TickCountdownTimer(void *arg);

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
extern void FillMemory32(void *dst, s32 pattern, s32 len);

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
/* cc1-small / assembler-absolute (see g_blobShadowState): the original reads
 * the mode with the one-insn symbolic `lw $r, sym` macro. */
__asm__(".extern g_occlusionMode, 16");
extern s32 g_occlusionMode[1];
extern u8 g_occlusionVisMask[];
extern void ResolveOcclusionVisMask(void);

/* Pause/menu render-mode query (1 = world behind pause, 0 = menu widgets). */
extern s32 IsMenuOverlayActive(void);
extern void RenderSaveLoadStatusPopup(void);

/* Wrapped text-box renderer (the scaled core behind DrawTextBoxDefault).
 * The real fn takes SEVEN args: the five int slots, the glyph-metrics table
 * (D_263B10, passed in $9 on the 0x280B48 path), then the f32 scale — sig
 * recovered 2026-07-06 (fable, promo-d3 @4cb6be7). The portable #else chain
 * had DROPPED the glyph-table arg (GuiMenuListSetRows dropped-arg class), running
 * native text draws through a garbage font table. One prototype serves both
 * builds: under EABI the six integer args go in $4..$9 and `scale` in $f12,
 * which is how the ROM's func_00280B20 calls it. */
extern void func_00280550(s32 a, s32 b, s32 c, s32 d, s32 e, u8 *glyphTable,
                          f32 scale);

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
 * (RecomputeScreenViewportFromGsContext). g_screenHeight is the base of {height, halfW, halfH}. */
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
/* TODO(match) t493: sdk29 34.59% / engine96 54.82% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): MACRO-AT (first differing
 * insn: ROM `sd s0,0(sp)` vs built `addiu a0,zero,1`). Levers: cc1-small/absolute globals model
 * RUN: 56.98% (engine96); engine96 with sched1 MEASURED (flag not landed): 39.20%. */
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

/** func_00278F90 (SceneTransitionFadeOut) — fence wait, reselect the
 *  scene-arena region + reset the frame arenas, save the camera-slot VRAM
 *  dynamic base into g_vramDynamicBase, then run the 30-frame blocking fade to
 *  black. No params, no return. The empty asm statement after the last call
 *  keeps cc1 from sibcalling it; the original keeps the call + return frame.
 *  MATCHED (task #493): 100.00% unit objdiff, sdk29 arm. */
void func_00278F90(void) {
    WaitFrameDmaFence(1);
    func_002FCFC8();
    ResetFrameArenas();
    g_vramDynamicBase = *(s32 *)(g_cameraSlotActive + 0xE8);
    FadeOutToBlackBlocking(0x1E);
    __asm__ __volatile__("");
}

/* func_00278FD0: PARKED #70 — a large (~840-instruction) game-state/frame driver (PopGameState,
 * StartFileLoad, UpdateSoundEmitters, func_00279xxx sub-steps, func_002AB150). Far too large +
 * branch/state-heavy to model confidently; a careful multi-pass Ghidra decomposition job. */
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
/* TODO(match) t493: sdk29 61.45% / engine96 63.97% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): CONST-MULT (first differing
 * insn: ROM `lw t7,16(sp)` vs built `lw t4,16(sp)`). Levers: engine96 with sched1 MEASURED (flag
 * not landed): 40.86%. */
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
/* TODO(match) t493: sdk29 68.59% / engine96 40.76% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): GPREL-FORM (first differing
 * insn: ROM `lui v0,0x0  [HI16 0x001B7E30]` vs built `lui a0,0x0  [HI16 0x001B7E30]`). Levers:
 * cc1-small/absolute globals model RUN: 53.69% (sdk29). */
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
/* TODO(match) t493: sdk29 73.30% / engine96 52.79% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-lui (first differing
 * insn: ROM `lui a3,0x0  [HI16 0x001B7E30]` vs built `lui v0,0x0  [HI16 0x001B7E30]`). */
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

/** func_00279EE8 — forward the camera slot at g_cameraSlotActive+0xF8 to
 *  TickCountdownTimer. No params, no return. The empty asm statement keeps cc1
 *  from sibcalling the lone tail call (the original keeps the call frame).
 *  MATCHED (task #493): 100.00% unit objdiff, sdk29 arm. */
void func_00279EE8(void) {
    TickCountdownTimer(g_cameraSlotActive + 0xF8);
    __asm__ __volatile__("");
}

/* func_00279F08 - advance a table-driven sequence cursor one step and report whether the
 * step "settled". State lives at g_cameraSlotActive: a s16 cursor (+0xF0) + sub-position
 * (+0xF2), a 0x28-stride entry table (+0x458, entry.field0 at +0x00, entry.scroll at +0x1C),
 * an entry count (+0x958), and a mode word at g_cameraCallbackCount+0x80.
 *   - cursor == -1 (uninitialised): reset to 0; if mode==1, jump to row 4 and set mode=2.
 *   - else: run func_00279E00 on the current entry's scroll field (it returns whether a new
 *     index/cursor was produced and writes them out). If it produced one AND the entry's
 *     scroll field is non-zero, adopt that (index,cursor) pair and return 1. Otherwise step
 *     the cursor forward, skipping runs of entries whose field0 matches the one we left
 *     (dedup); return 1 iff we stopped on a DIFFERENT entry (allSame^1). Special-case: if we
 *     land on row 4 with mode==0, back up to row 3, set mode=1, return 0.
 *
 * UN-PARK of a premature #70: the park's only cited blocker was "needs a careful control-flow
 * trace" - done here, cross-checked Ghidra against the .s (real body @ this glabel) branch by
 * branch. Engine region - faithful #else, not a byte match. FORMER-PARK: dual-gated (tester
 * oracle) + d2 heads-up per the un-park guardrails. */
/* TODO(match) t493: sdk29 46.37% / engine96 0.00% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): MACRO-AT (first differing insn:
 * ROM `addiu sp,sp,-64` vs built `addiu sp,sp,-128`). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00279F08);
#else
extern u8 g_cameraCallbackCount[];   /* +0x80 = sequence mode word (0/1/2) */

s32 func_00279F08(void) {
    s16 *cursor = (s16 *)(g_cameraSlotActive + 0xF0);   /* sequence row cursor */
    s16 *sub    = (s16 *)(g_cameraSlotActive + 0xF2);   /* sub-position */
    s32 *mode   = (s32 *)(g_cameraCallbackCount + 0x80);
    u8  *table  = g_cameraSlotActive + 0x458;           /* 0x28-stride entry table */
    s32  count  = *(s32 *)(g_cameraSlotActive + 0x958);
    s32  status;

    if (*cursor == -1) {
        status = 1;
        *cursor = 0;
        *sub = 0;
        if (*mode == 1) {
            *mode = 2;
            *cursor = 4;
            *sub = 0;
        }
    } else {
        s32 idx, cur;
        s32 produced = func_00279E00(*(s32 *)(table + *cursor * 0x28 + 0x1C), &idx, &cur);
        if (produced == 0 || *(s32 *)(table + *cursor * 0x28 + 0x1C) == 0) {
            s16 prev = *cursor;
            s32 allSame = 1;
            *cursor = *cursor + 1;
            *sub = 0;
            if (*cursor < count) {
                do {
                    if (*(s32 *)(table + *cursor * 0x28) == *(s32 *)(table + prev * 0x28)) {
                        *cursor = *cursor + 1;
                    } else {
                        allSame = 0;
                    }
                } while (*cursor < count && allSame);
            }
            status = allSame ^ 1;
            if (*cursor == 4 && *mode == 0) {
                *cursor = 3;
                status = 0;
                *mode = 1;
            }
        } else {
            *cursor = (s16)idx;
            *sub = (s16)cur;
            status = 1;
        }
    }
    return status;
}
#endif

/* func_0027A0C8: 8 bytes of dead pad (addiu $sp,0x20; nop) carved off the real
 * entry func_0027A0D0 in task #472. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A0C8);

/* func_0027A0D0 globals (declared for the TARGET_NATIVE #else only). */
extern s32 g_playerProgress;      /* 0x1A79F8 story progress counter */
extern u8  g_platinumBoltFlags[]; /* 0x19B278; +0x230 (0x19B4A8) = per-progress 3-bit nibble-counter table, stride 0x400 */

/** func_0027A0D0 — read a progress-gated nibble counter and scale it. Index the
 *  per-progress nibble table (g_platinumBoltFlags+0x230, stride 0x400 keyed by
 *  g_playerProgress) at index>>1; take the selected byte's low nibble (even index)
 *  or high nibble (odd index), mask it to 0..7, and use THAT to index `table` — both
 *  parities index table[nibble & 7]. Return (table[nibble & 7] * mult) / 100. */
/* TODO(match) t493: sdk29 23.60% / engine96 45.20% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): GPREL-FORM (first differing
 * insn: ROM `addiu sp,sp,32` vs built `lw v0,0(gp)  [GPREL16 0x001A79F8]`). Levers:
 * cc1-small/absolute globals model RUN: 33.20% (sdk29); engine96 with sched1 MEASURED (flag not
 * landed): 44.60%.
 * (t527 merge note: measured before task #472's carve, against the fused blob func_0027A0C8 =
 * the 8-byte pad + this body; the `addiu sp,sp,32` row named above is the pad's first word,
 * the %s are for the 0x64-byte blob. The GPREL-FORM residual — ROM lui/lw %hi/%lo of
 * g_playerProgress vs gp-relative — is in the body itself and remains at func_0027A0D0, 0x5C.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A0D0);
#else
s32 func_0027A0D0(u8 *table, s32 mult, s32 index) {
    u8 *entry = g_platinumBoltFlags + 0x230 + g_playerProgress * 0x400 + (index >> 1);
    s32 value;
    if (index & 1) {
        value = table[(*entry >> 4) & 7];
    } else {
        value = table[*entry & 7];
    }
    return (value * mult) / 100;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A130);

/**
 * func_0027A138 — project a world position to fixed-point screen coordinates.
 *
 * Builds a view matrix (identity with translation -cameraPos*1024 in its bottom
 * row, times the camera rotation matrix g_cameraState+0x40), transforms
 * worldPos*1024 through it, then perspective-divides x/y by the transformed
 * depth times the projection factor (g_cameraProjScale+0x160). Writes the
 * screen point in the 12.4 fixed-point convention: out[0]/out[1] =
 * (proj + 2048)*16 for x/y, out[2] = z/1024.
 */
/* TODO(match) t493: sdk29 76.57% / engine96 54.05% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-224` vs built `addiu sp,sp,-256`). Levers: engine96 with sched1 MEASURED
 * (flag not landed): 74.61%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027A138);
#else
extern u8 g_cameraState[];       /* +0x40 rotation matrix; +0x140/+0x144/+0x148 position */
extern u8 g_cameraProjScale[];   /* +0x160 depth scale */
extern void MatrixIdentityVu0(f32 *m);
extern void MatrixMultiplyVu0(f32 *dst, f32 *a, f32 *b);
extern void Vec4ScaleVu0(void *dst, f32 s, void *src);
extern f32 func_00283A70(void *out, void *in, void *matrix);

void func_0027A138(f32 *out, void *worldPos) {
    f32 m[16];      /* identity + translation */
    f32 view[16];   /* camera rotation * translation */
    f32 wp[4];      /* worldPos * 1024 */
    f32 tp[4];      /* transformed point */
    f32 s;

    MatrixIdentityVu0(m);
    m[12] = -*(f32 *)(g_cameraState + 0x140) * 1024.0f;
    m[13] = -*(f32 *)(g_cameraState + 0x144) * 1024.0f;
    m[14] = -*(f32 *)(g_cameraState + 0x148) * 1024.0f;
    MatrixMultiplyVu0(view, (f32 *)(g_cameraState + 0x40), m);

    Vec4ScaleVu0(wp, 1024.0f, worldPos);
    wp[3] = 1.0f;
    func_00283A70(tp, wp, view);

    s = *(f32 *)(g_cameraProjScale + 0x160) / tp[3];
    out[2] = tp[2] * (1.0f / 1024.0f);
    out[1] = (tp[1] * s + 2048.0f) * 16.0f;
    out[0] = (tp[0] * s + 2048.0f) * 16.0f;
}
#endif

/* BuildFrameViewMatrices — compose the per-frame view + projection matrices used
 * by the world render pass. Runs once per frame; no arguments, no return.
 *
 * Steps (op-for-op faithful to the frozen .s):
 *   1. Copy the three rows of the camera rotation matrix (g_cameraMatrix, three
 *      qwords) onto a stack scratch.
 *   2. Build a 4x4 view-basis at g_cameraState+0x0 from that scratch: the first
 *      two columns are negated, the third is straight, the 4th column is 0 and
 *      the bottom-right element is 1.0 (partial transpose / sign-flip of the
 *      camera rotation).
 *   3. viewA = MatrixMultiplyVu0(g_cameraState+0x40, g_cameraProjScale+0x10, view)
 *   4. viewB = MatrixMultiplyVu0(g_cameraState+0x80, g_cameraProjScale+0x50, view)
 *   5. Apply a perspective/skew adjustment to viewB: for each of the four rows,
 *      add (row's 4th-column element) * scalar[c] into that row's column c, for
 *      c in {0,1,2}, where the scalars come from g_sceneActorMobys+0x814/818/81C.
 *   6. MatrixMultiplyVu0(g_cameraState+0xC0, g_cameraProjScale+0x90, viewB)
 *   7. Two ScaleVec4IncludingW scalings by g_sceneActorMobys+0x834 into
 *      g_cameraState+0x100 and +0x110, then copy two more qwords in, and
 *      MatrixMultiplyVu0(g_cameraState+0x100, g_cameraState+0x100, view).
 *   8. Copy the view basis (three qwords) to the frustum-plane block
 *      g_fogColorRed+0x50, then Vec4ScaleVu0(g_fogColorRed+0x80, 1024.0f,
 *      g_cameraState+0x140) and store 1024.0f at g_fogColorRed+0x8C.
 *
 * Base-symbol map used (all absolute %hi/%lo in the .s, resolved via
 * symbol_addrs): g_cameraMatrix=0x1B54F0 ($18); g_cameraState=0x1B5180
 * ($16 = g_cameraMatrix-0x370); g_cameraProjScale=0x1B9070 (first
 * $17 = g_cameraProjScale+0x10 = 0x1B9080; $20 = +0x90 = 0x1B9100);
 * g_sceneActorMobys=0x1B894C ($19 = +0x674 = 0x1B8FC0 = g_cameraProjScale-0xB0);
 * g_fogColorRed=0x1B91F0 (second $17 = g_fogColorRed+0x50 = 0x1B9240). */
/* TODO(match) t493: sdk29 31.15% / engine96 22.92% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-160` vs built `addiu sp,sp,-176`). Levers: engine96 with sched1 MEASURED
 * (flag not landed): 27.51%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", BuildFrameViewMatrices);
#else
void BuildFrameViewMatrices(void) {
    /* block-scope externs mirroring the file's engine-#else convention */
    extern u8 g_cameraMatrix[];      /* 0x1B54F0 - 3-row camera rotation matrix */
    extern u8 g_cameraProjScale[];   /* 0x1B9070 - projection scale / GS matrices at +0x10.. */
    extern u8 g_sceneActorMobys[];   /* 0x1B894C - scratch block; +0x814.. perspective scalars */
    extern u8 g_fogColorRed[];       /* 0x1B91F0 - +0x50 frustum-plane block */
    extern u8 g_cameraState[];       /* 0x1B5180 - view-basis / result matrices */
    extern void ScaleVec4IncludingW(f32 *dst, f32 scale, f32 *src); /* 0x283710 - vec4 scale incl. w */

    f32 rows[12];        /* stack copy of the three camera-matrix rows */
    u8 *cam  = g_cameraState;          /* 0x1B5180 */
    u8 *proj = g_cameraProjScale + 0x10; /* 0x1B9080 */
    u8 *scal = g_sceneActorMobys + 0x674; /* 0x1B8FC0 - perspective scalars at +0x1A0.. */
    u8 *fog  = g_fogColorRed + 0x50;   /* 0x1B9240 */
    f32 s0, s1, s2, w;
    s32 r;

    /* 1. copy the three camera-matrix rows onto the stack (three lq/sq qwords) */
    *(u64 *)((u8 *)rows + 0x0)  = *(u64 *)(g_cameraMatrix + 0x0);
    *(u64 *)((u8 *)rows + 0x8)  = *(u64 *)(g_cameraMatrix + 0x8);
    *(u64 *)((u8 *)rows + 0x10) = *(u64 *)(g_cameraMatrix + 0x10);
    *(u64 *)((u8 *)rows + 0x18) = *(u64 *)(g_cameraMatrix + 0x18);
    *(u64 *)((u8 *)rows + 0x20) = *(u64 *)(g_cameraMatrix + 0x20);
    *(u64 *)((u8 *)rows + 0x28) = *(u64 *)(g_cameraMatrix + 0x28);

    /* 2. build the view basis at g_cameraState (cols 0/1 negated, col 2 straight) */
    *(f32 *)(cam + 0x0)  = -rows[4];   /* -rows[0x10] */
    *(f32 *)(cam + 0x10) = -rows[5];   /* -rows[0x14] */
    *(f32 *)(cam + 0x20) = -rows[6];   /* -rows[0x18] */
    *(f32 *)(cam + 0x4)  = -rows[8];   /* -rows[0x20] */
    *(f32 *)(cam + 0x14) = -rows[9];   /* -rows[0x24] */
    *(f32 *)(cam + 0x24) = -rows[10];  /* -rows[0x28] */
    *(f32 *)(cam + 0x8)  = rows[0];    /*  rows[0x0]  */
    *(f32 *)(cam + 0x18) = rows[1];    /*  rows[0x4]  */
    *(f32 *)(cam + 0x28) = rows[2];    /*  rows[0x8]  */
    *(f32 *)(cam + 0x3C) = 1.0f;
    *(s32 *)(cam + 0x30) = 0;
    *(s32 *)(cam + 0x34) = 0;
    *(s32 *)(cam + 0x38) = 0;
    *(s32 *)(cam + 0xC)  = 0;
    *(s32 *)(cam + 0x1C) = 0;
    *(s32 *)(cam + 0x2C) = 0;

    /* 3-4. two view multiplies */
    MatrixMultiplyVu0((f32 *)(cam + 0x40), (f32 *)proj, (f32 *)cam);
    MatrixMultiplyVu0((f32 *)(cam + 0x80), (f32 *)(proj + 0x40), (f32 *)cam);

    /* 5. perspective/skew adjust of the +0x80 block: for each row, add the row's
     *    4th-column element scaled by scalar[c] into columns 0/1/2 */
    s0 = *(f32 *)(scal + 0x1A0);
    s1 = *(f32 *)(scal + 0x1A4);
    s2 = *(f32 *)(scal + 0x1A8);
    for (r = 0; r < 4; r++) {
        u8 *row = cam + 0x80 + r * 0x10;
        w = *(f32 *)(row + 0xC);
        *(f32 *)(row + 0x0) = *(f32 *)(row + 0x0) + w * s0;
        *(f32 *)(row + 0x4) = *(f32 *)(row + 0x4) + w * s1;
        *(f32 *)(row + 0x8) = *(f32 *)(row + 0x8) + w * s2;
    }

    /* 6. third view multiply */
    MatrixMultiplyVu0((f32 *)(cam + 0xC0), (f32 *)(proj + 0x80), (f32 *)cam);

    /* 7. two full-width vec4 scalings, two qword copies, then a fourth multiply */
    ScaleVec4IncludingW((f32 *)(cam + 0x100), *(f32 *)(scal + 0x1C0),
                        (f32 *)(proj + 0x80));
    ScaleVec4IncludingW((f32 *)(cam + 0x110), *(f32 *)(scal + 0x1C0),
                        (f32 *)(proj + 0x90));
    *(u64 *)(cam + 0x120) = *(u64 *)(proj + 0xA0);
    *(u64 *)(cam + 0x128) = *(u64 *)(proj + 0xA8);
    *(u64 *)(cam + 0x130) = *(u64 *)(proj + 0xB0);
    *(u64 *)(cam + 0x138) = *(u64 *)(proj + 0xB8);
    MatrixMultiplyVu0((f32 *)(cam + 0x100), (f32 *)(cam + 0x100), (f32 *)cam);

    /* 8. copy the view basis to the frustum-plane block and scale the camera pos */
    *(u64 *)(fog + 0x0)  = *(u64 *)(cam + 0x0);
    *(u64 *)(fog + 0x8)  = *(u64 *)(cam + 0x8);
    *(u64 *)(fog + 0x10) = *(u64 *)(cam + 0x10);
    *(u64 *)(fog + 0x18) = *(u64 *)(cam + 0x18);
    *(u64 *)(fog + 0x20) = *(u64 *)(cam + 0x20);
    *(u64 *)(fog + 0x28) = *(u64 *)(cam + 0x28);
    Vec4ScaleVu0(fog + 0x30, 1024.0f, cam + 0x140);
    *(f32 *)(fog + 0x3C) = 1024.0f;
}
#endif

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
/* TODO(match) t493: sdk29 28.42% / engine96 24.12% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): MACRO-AT (first differing insn:
 * ROM `lui v0,0x0  [HI16 0x001B5580]` vs built `addiu sp,sp,-16`). Levers: cc1-small/absolute
 * globals model RUN: 24.12% (engine96); engine96 with sched1 MEASURED (flag not landed): 24.51%. */
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
/* TODO(match) t493: sdk29 69.72% / engine96 41.47% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-lui (first differing
 * insn: ROM `lui t0,0x0  [HI16 0x001B1634]` vs built `lui v0,0x0  [HI16 0x001B1634]`). Levers:
 * cc1-small/absolute globals model RUN: 69.72% (sdk29); engine96 with sched1 MEASURED (flag not
 * landed): 75.64%. */
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
/* TODO(match) t493: sdk29 86.04% / engine96 85.33% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-64` vs built `addiu sp,sp,-112`). Levers: engine96 with sched1 MEASURED
 * (flag not landed): 79.78%. */
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

/** UpdateOcclusionVisMask — RenderFrame pre-layer pass that produces the
 *  per-frame visibility bitmask: mode 0 sets every bit (no occlusion data),
 *  mode 2 resolves it from the level occlusion grid, any other mode leaves the
 *  previous mask. No params, no return. The fill pattern is the signed -1 the
 *  original materialises with one addiu (an unsigned prototype spells it
 *  lui/ori); the empty asm statements keep cc1 from sibcalling either call.
 *  MATCHED (task #493): 100.00% unit objdiff, sdk29 arm. */
void UpdateOcclusionVisMask(void) {
    if (g_occlusionMode[0] == 0) {
        FillMemory32(g_occlusionVisMask, -1, 0x80);
        __asm__ __volatile__("");
    } else if (g_occlusionMode[0] == 2) {
        ResolveOcclusionVisMask();
        __asm__ __volatile__("");
    }
}

/* InitScreenGeometry: one-time screen-geometry + projection-viewport init from
 * the GS screen context pixel dims (g_gsScreenContext +0x150 width / +0x152
 * height). Sibling of RecomputeScreenViewportFromGsContext (same idioms): publishes width/height + the
 * two half-extents to g_screenWidth / g_screenHeight[0..2], the four 12.4
 * fixed-point GS-window offsets (centred on 0x800) to g_gsPixelOffsetX[0] /
 * g_gsPixelOffsetY[0..2], and the float viewport scale + half-extents into the
 * camera/projection scratch (g_sceneActorMobys+0x674 = D_1B8FC0). Unlike
 * RecomputeScreenViewportFromGsContext it also seeds the fixed projection constants (+0xA0 32, +0xA4
 * 2^20, +0x1DC 2^19, +0x1E8 255) and does NOT call func_0027A550. The matching
 * build keeps the asm. */
/* TODO(match) t493: sdk29 0.00% / engine96 26.37% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): MACRO-AT (first differing
 * insn: ROM `addiu a2,zero,2048` vs built `lui v0,0x0  [HI16 0x001A6480]`). Levers:
 * cc1-small/absolute globals model RUN: 34.69% (engine96); engine96 with sched1 MEASURED (flag not
 * landed): 0.00%. */
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

/**
 * func_0027B858 — configure the viewport / camera and rebuild the projection.
 *
 * From a target width/height and five projection floats, computes the half
 * extents, publishes the fixed-point GS window offsets (centred on 0x800, <<4)
 * to g_gsPixelOffsetX[0] / g_gsPixelOffsetY[0..2], the screen dims + centres to
 * g_screenWidth/g_screenHeight[0..2], and the viewport scale/half-extents (from
 * IntToFloat(dim)*0.5 and *4, plus the five floats) into the camera scratch at
 * g_sceneActorMobys+0x674 (+0xB0/+0x200..+0x22C), then rebuilds the projection.
 */
/* TODO(match) t493: sdk29 29.47% / engine96 20.12% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `addiu sp,sp,-80` vs built `addiu sp,sp,-160`). Levers: sibcall guard RUN: sdk29 23.95% /
 * engine96 24.70%; cc1-small/absolute globals model RUN: 25.33% (engine96); engine96 with sched1
 * MEASURED (flag not landed): 15.67%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027B858);
#else
void func_0027B858(s32 width, s32 height, f32 fa, f32 fb, f32 fc, f32 fd, f32 fe) {
    u8 *cam = g_sceneActorMobys + 0x674;
    s32 halfW = width >> 1;
    s32 halfH = height >> 1;
    f32 fw = IntToFloat(width) * 0.5f;
    f32 fh = IntToFloat(height) * 0.5f;

    *(f32 *)(cam + 0xB0) = fa;
    g_gsPixelOffsetX[0] = (0x800 - halfW) << 4;
    g_gsPixelOffsetY[0] = (0x800 - halfH) << 4;
    g_gsPixelOffsetY[1] = (halfW + 0x800) << 4;
    g_gsPixelOffsetY[2] = (halfH + 0x800) << 4;
    g_screenWidth[0] = width;
    g_screenHeight[0] = height;
    g_screenHeight[1] = halfW;   /* g_screenCenterDefaultX */
    g_screenHeight[2] = halfH;   /* g_screenCenterDefaultY */
    *(f32 *)(cam + 0x200) = fw;
    *(f32 *)(cam + 0x204) = fh;
    *(f32 *)(cam + 0x208) = fw * 4.0f;
    *(f32 *)(cam + 0x20C) = fh * 4.0f;
    *(f32 *)(cam + 0x218) = fb;
    *(f32 *)(cam + 0x21C) = fc;
    *(f32 *)(cam + 0x228) = fd;
    *(f32 *)(cam + 0x22C) = fe;
    BuildCameraProjection();
}
#endif

/* RecomputeScreenViewportFromGsContext: derive the active display geometry and
 * the 2D draw viewport from the GS screen context's pixel dimensions
 * (g_gsScreenContext +0x150 width / +0x152 height). Publishes width/height and
 * the half-extents to g_screenWidth / g_screenHeight[0..2], the four GS-window
 * pixel offsets (12.4 fixed-point, centred on 0x800) to g_gsPixelOffsetX /
 * g_gsPixelOffsetY[0..2], and the float viewport scale/half-extents into the
 * camera/projection scratch (g_sceneActorMobys+0x674 == D_1B8FC0: +0xB0 aspect
 * 0.62, +0x200/+0x204 half-extents, +0x208/+0x20C ×4), then runs func_0027A550.
 * The matching build keeps the asm. */
/* TODO(match) t493: sdk29 42.90% / engine96 34.53% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `addiu sp,sp,-48` vs built `addiu sp,sp,-64`). Levers: sibcall guard RUN: sdk29 41.37% /
 * engine96 40.71%; cc1-small/absolute globals model RUN: 45.53% (engine96); engine96 with sched1
 * MEASURED (flag not landed): 46.63%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RecomputeScreenViewportFromGsContext);
#else
void RecomputeScreenViewportFromGsContext(void) {
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

/* SetupGsDisplayBuffers(clearVram) — bring up the GS display/draw environment.
 *
 * Selects the VRAM framebuffer / Z-buffer / texture-base layout by video mode:
 *   - NTSC   (pal==0, prog==0): Zbuf 0x118000, FrameBufB 0x1E8000, tex 0x2C0000,
 *                               BuildScreenDrawPackets(0x200,0x1A0,0x200,0x1C0,0,0)
 *   - 480p   (pal==0, prog!=0): Zbuf 0x118000, FrameBufB 0x1E8000, tex 0x2C0000,
 *                               BuildScreenDrawPackets(0x200,0x1A0,0x280,0x1C0,0,0)
 *   - PAL    (pal!=0):          Zbuf 0x100000, FrameBufB 0x1E0000, tex 0x2C0000,
 *                               BuildScreenDrawPackets(0x200,0x1C0,0x200,0x200,4,0)
 * FrameBufA is always 0. Then derives the screen dims / half-extents from the GS
 * screen-context display size (g_gsScreenContext +0x150 width, +0x152 height),
 * publishes the fixed-point GS window offsets (centred on 0x800, <<4) and packs
 * the GS DISPFB/FRAME/SCISSOR/XYOFFSET register images into the D_139120 block
 * (+0x10..+0x80, with the +0x20/+0x40/+0x60/+0xA0 aliases mirroring their
 * neighbours) plus the PTR aliases at D_139310 (ZBUF) and D_139380 (ZBUF+PSM).
 * The texture sub-carves land at g_vramTextureBase+4/+8/+C/+10 (base, +0x1000,
 * +0x1400, +0x1800).
 *
 * When clearVram is nonzero it also wipes VRAM: rebuild the dynamic-alloc cursor,
 * flush the draw-env / clear packets, then loop 0x20x0x20 image-upload/kick
 * blits of the zeroed g_collHitTriVert2+0x10 (0x1BAF80) scratch across the whole
 * framebuffer (count = ctx.width * ctx.height >> 10, stepping 0x100000 bytes).
 *
 * Engine region — faithful #else, whole-.s traced (Ghidra-complete; 0 lq/sq, all
 * 64-bit register images built in s64 so the <<0x10/<<0x20/<<0x30 packs don't
 * overflow native 32-bit long). The matching arm keeps the shipped bytes. */
/* TODO(match) t493: sdk29 52.08% / engine96 46.83% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): GPREL-FORM (first differing
 * insn: ROM `addiu sp,sp,-160` vs built `addiu sp,sp,-272`). Levers: cc1-small/absolute globals
 * model RUN: 43.94% (engine96); engine96 with sched1 MEASURED (flag not landed): 45.56%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", SetupGsDisplayBuffers);
#else
extern s32 g_bPalMode;         /* 0x1A7B98 PAL flag (USA hard-codes 0) */
extern s32 g_bProgressiveScan; /* 0x1A7BC0 480p flag */
extern s32 g_vramZBuffer;      /* 0x1A72E0 VRAM Z-buffer base */
extern s32 g_vramFrameBufA;    /* 0x1A72D8 front framebuffer base (always 0) */
extern s32 g_vramFrameBufB;    /* 0x1A72DC back framebuffer base */
extern u8  g_vramTextureBase[];/* 0x1A72E4 static texture base; +4/+8/+C/+10 carves */
extern s32 g_vramAllocCursor;  /* 0x1A72D0 dynamic VRAM bump cursor */

/* GS register-image / packet blocks (absolute %hi/%lo; 64-bit sd targets). */
extern u8 D_139120[];          /* DISPFB/FRAME/SCISSOR/XYOFFSET register images */
extern u8 D_139310[];          /* ZBUF PTR-reg image alias */
extern u8 D_139380[];          /* ZBUF+PSM PTR-reg image alias */
/* Zeroed VRAM-clear source scratch (g_collHitTriVert2 vec4 + 0x10 == 0x1BAF80). */
extern u8 g_collHitTriVert2[];

extern void BuildScreenDrawPackets(s32 dw, s32 dh, s32 sw, s32 sh, s32 a, s32 b);
extern void ApplyGsDisplayEnv(void);
extern void KickGifImageUpload(void *packet, void *src);
extern void WaitGsPathsIdle(s32 a, s32 b);
extern void FlushCache(s32 mode);                       /* func_0011AEA0 */
extern void BuildGsImageUploadPacket(void *packet, s32 dbp, s32 psm, s32 c,
                                     s32 d, s32 e, s32 w, s32 h); /* func_00126288 */

void SetupGsDisplayBuffers(long clearVram) {
    u8 *ctx = g_gsScreenContext;
    u8  packet[0x60];
    s32 w16, h16;
    s32 screenWidth, screenHeight;
    s32 centerX, centerY;
    s32 fbw;
    s32 zbuf;
    s64 frame, zbufPack;

    FlushCache(0);

    if (g_bPalMode == 0) {
        if (g_bProgressiveScan == 0) {
            *(s32 *)g_vramTextureBase = 0x2C0000;
            g_vramZBuffer = 0x118000;
            g_vramFrameBufB = 0x1E8000;
            g_vramFrameBufA = 0;
            BuildScreenDrawPackets(0x200, 0x1A0, 0x200, 0x1C0, 0, 0);
        } else {
            g_vramZBuffer = 0x118000;
            g_vramFrameBufB = 0x1E8000;
            *(s32 *)g_vramTextureBase = 0x2C0000;
            g_vramFrameBufA = 0;
            BuildScreenDrawPackets(0x200, 0x1A0, 0x280, 0x1C0, 0, 0);
        }
    } else {
        g_vramZBuffer = 0x100000;
        g_vramFrameBufB = 0x1E0000;
        *(s32 *)g_vramTextureBase = 0x2C0000;
        g_vramFrameBufA = 0;
        BuildScreenDrawPackets(0x200, 0x1C0, 0x200, 0x200, 4, 0);
    }

    /* Derive screen geometry + GS window offsets from the context display dims. */
    w16 = *(u16 *)(ctx + 0x150);
    h16 = *(u16 *)(ctx + 0x152);
    screenWidth  = (s32)((u32)w16 << 16) >> 16;
    screenHeight = (s32)((u32)h16 << 16) >> 16;
    centerX = (s32)((u32)w16 << 16) >> 17;
    centerY = (s32)((u32)h16 << 16) >> 17;
    fbw     = (s32)((u32)w16 << 16) >> 22;

    g_gsPixelOffsetY[0] = (0x800 - centerY) << 4;
    g_gsPixelOffsetX[0] = (0x800 - centerX) << 4;

    zbuf = g_vramZBuffer;
    frame    = ((s64)(g_vramFrameBufB >> 13)) | ((s64)fbw << 16);
    zbufPack = ((s64)(zbuf >> 13)) | 0x1000000LL;

    /* SCISSOR (max = screen-1) and XYOFFSET (window origin) register images. */
    {
        s64 scissorMax = ((s64)(screenWidth - 1) << 16) | ((s64)(screenHeight - 1) << 48);
        s64 xyOffset   = (s64)g_gsPixelOffsetX[0] | ((s64)g_gsPixelOffsetY[0] << 32);
        u8 *tex = g_vramTextureBase;

        *(s64 *)(D_139120 + 0x80) = scissorMax;
        *(s64 *)D_139380          = ((s64)(zbuf >> 13)) | 0x101000000LL;

        g_gsPixelOffsetY[1] = (centerX + 0x800) << 4;
        g_gsPixelOffsetY[2] = (centerY + 0x800) << 4;
        *(s32 *)(tex + 0x8)  = *(s32 *)tex + 0x1000;
        *(s32 *)(tex + 0xC)  = *(s32 *)tex + 0x1400;
        *(s32 *)(tex + 0x10) = *(s32 *)tex + 0x1800;

        *(s64 *)(D_139120 + 0x20) = frame;
        *(s64 *)(D_139120 + 0x50) = xyOffset;
        *(s64 *)(D_139120 + 0x60) = xyOffset;
        *(s64 *)D_139310          = zbufPack;
        *(s32 *)(tex + 0x4)  = *(s32 *)tex;
        g_screenWidth[0]  = screenWidth;
        g_screenHeight[0] = screenHeight;
        g_screenHeight[1] = centerX;   /* g_screenCenterDefaultX */
        g_screenHeight[2] = centerY;   /* g_screenCenterDefaultY */
        *(s64 *)(D_139120 + 0x10) = frame;
        *(s64 *)(D_139120 + 0x30) = zbufPack;
        *(s64 *)(D_139120 + 0x40) = zbufPack;
        *(s64 *)(D_139120 + 0x70) = scissorMax;
    }

    if (clearVram != 0) {
        s32 tileCount, tile;

        g_vramDynamicBase = *(s32 *)g_vramTextureBase;
        g_vramAllocCursor = *(s32 *)g_vramTextureBase;
        FlushCache(0);
        WaitGsPathsIdle(0, 0);
        FlushCache(0);
        AppendDrawEnvContext1();
        AppendScreenClearPacket(0);
        FlushCache(0);
        WaitGsPathsIdle(0, 0);
        ApplyGsDisplayEnv();
        FillMemory32(g_collHitTriVert2 + 0x10, 0, 0x1000);

        tileCount = (s32)(*(s16 *)(ctx + 0x158) * *(s16 *)(ctx + 0x15A)) >> 10;
        if (tileCount > 0) {
            s32 addr = 0;
            tile = 0;
            do {
                tile = tile + 1;
                BuildGsImageUploadPacket(packet, addr >> 16, 1, 0, 0, 0, 0x20, 0x20);
                FlushCache(0);
                KickGifImageUpload(packet, g_collHitTriVert2 + 0x10);
                WaitGsPathsIdle(0, 0);
                addr = tile << 20;
            } while (tile < tileCount);
        }
    }
}
#endif

/* ResetPerFrameDrawQueues: zero every per-frame draw-callback queue and
 * one-shot fx slot: the fx pre/post/late hook counts, the after-ties /
 * after-shrubs draw-hook counts, the blob-shadow count, the two screen-fade
 * request words (g_screenFadeWhite+4/+8), three render-frame one-shot slots,
 * the occlusion override mode, the water wave-grid pair and its flags, and the
 * water-pool flag. Called at the top of each frame. No params, no return.
 * Addressing: the ROM zeroes each absolute word with the assembler's
 * `lui $at; sw $0,%lo` macro pair, and D_1A86F4/D_1A8760/D_1A8770 and
 * g_bWaterPoolActive with one %gp_rel store. So every absolute word goes
 * through an assembler alias sized 16 that cc1 sees as small data (the #8036
 * construct; relocations name the real symbols). An alias must be equated at
 * offset 0: GAS sizes `alias = sym + off` as small whatever its .extern says,
 * so the +4/+8 words are reached through small (<= 8-byte) offset-0 views.
 * t493's "cc1-small/absolute globals model 64.31%" (blanket) predates that.
 * MATCHED (task #889): 100.00% sdk29 (unit objdiff, objdiff_build.sh), solo. */
#ifndef TARGET_NATIVE
__asm__(".extern g_fxHooksPreCountAbs, 16\n\tg_fxHooksPreCountAbs = g_fxHooksPreCount");
__asm__(".extern g_drawHooksAfterTiesCountAbs, 16\n\tg_drawHooksAfterTiesCountAbs = g_drawHooksAfterTiesCount");
__asm__(".extern g_drawHooksAfterShrubsCountAbs, 16\n\tg_drawHooksAfterShrubsCountAbs = g_drawHooksAfterShrubsCount");
__asm__(".extern g_fxHooksPostCountAbs, 16\n\tg_fxHooksPostCountAbs = g_fxHooksPostCount");
__asm__(".extern g_fxHooksLateCountAbs, 16\n\tg_fxHooksLateCountAbs = g_fxHooksLateCount");
__asm__(".extern g_blobShadowCountAbs, 16\n\tg_blobShadowCountAbs = g_blobShadowCount");
__asm__(".extern g_screenFadeWhiteAbs, 16\n\tg_screenFadeWhiteAbs = g_screenFadeWhite");
__asm__(".extern g_occlusionOverrideModeAbs, 16\n\tg_occlusionOverrideModeAbs = g_occlusionOverrideMode");
__asm__(".extern g_pWaterWaveGridsAbs, 16\n\tg_pWaterWaveGridsAbs = g_pWaterWaveGrids");
__asm__(".extern g_bWaterWavesActiveAbs, 16\n\tg_bWaterWavesActiveAbs = g_bWaterWavesActive");
extern s32 g_fxHooksPreCountAbs, g_drawHooksAfterTiesCountAbs, g_drawHooksAfterShrubsCountAbs;
extern s32 g_fxHooksPostCountAbs, g_fxHooksLateCountAbs, g_blobShadowCountAbs;
/* 4-byte view (cc1-small) whose zero-length tail reaches the +4/+8 words */
typedef struct { s32 level; s32 word[0]; } ScreenFadeWhiteHead;
extern ScreenFadeWhiteHead g_screenFadeWhiteAbs;
extern s32 g_occlusionOverrideModeAbs;
extern s32 g_pWaterWaveGridsAbs[2], g_bWaterWavesActiveAbs;
#else
#define g_fxHooksPreCountAbs            g_fxHooksPreCount[0]
#define g_drawHooksAfterTiesCountAbs    g_drawHooksAfterTiesCount[0]
#define g_drawHooksAfterShrubsCountAbs  g_drawHooksAfterShrubsCount[0]
#define g_fxHooksPostCountAbs           g_fxHooksPostCount[0]
#define g_fxHooksLateCountAbs           g_fxHooksLateCount[0]
#define g_blobShadowCountAbs            g_blobShadowCount[0]
#define g_screenFadeWhiteAbs            (*(struct { s32 level; s32 word[2]; } *)g_screenFadeWhite)
#define g_occlusionOverrideModeAbs      g_occlusionOverrideMode[0]
#define g_pWaterWaveGridsAbs            g_pWaterWaveGrids
#define g_bWaterWavesActiveAbs          g_bWaterWavesActive[0]
#endif

void ResetPerFrameDrawQueues(void) {
    g_fxHooksPreCountAbs = 0;
    g_drawHooksAfterTiesCountAbs = 0;
    g_drawHooksAfterShrubsCountAbs = 0;
    g_fxHooksPostCountAbs = 0;
    g_fxHooksLateCountAbs = 0;
    g_blobShadowCountAbs = 0;
    g_screenFadeWhiteAbs.word[0] = 0;
    g_screenFadeWhiteAbs.word[1] = 0;
    D_1A86F4 = 0;
    D_1A8760 = 0;
    D_1A8770 = 0;
    g_occlusionOverrideModeAbs = 0;
    g_pWaterWaveGridsAbs[0] = 0;
    g_pWaterWaveGridsAbs[1] = 0;
    g_bWaterWavesActiveAbs = 0;
    g_bWaterPoolActive = 0;
}

extern u8 D_1391D0[]; /* prebuilt GS init packet A */
extern u8 D_139120[]; /* prebuilt GS init packet B */

/* TODO(match) t493: sdk29 33.27% / engine96 20.83% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `lui a0,0x0  [HI16 0x001B2228]` vs built `lui t2,0x0  [HI16 0x001B2228]`). Levers: sibcall
 * guard RUN: sdk29 42.44% / engine96 39.32%; cc1-small/absolute globals model RUN: 38.80% (sdk29);
 * -fno-strict-aliasing MEASURED (flag not landed): 35.32% sdk29; engine96 with sched1 MEASURED
 * (flag not landed): 18.31%. */
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
/* TODO(match) t493: sdk29 63.62% / engine96 55.82% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): CONST-DLI — with the sibcall
 * guard 92.94% sdk29; the 4-row residual is the synthesis of the 64-bit constant 0x8000000044: the
 * original spells it `ori 0x8000; dsll 24; ori 0x44`, cc1 2.9 `addiu 128; dsll32 0; ori 0x44`
 * (companion of FACT #7379, constant-synthesis is not a phrasing). Levers: sibcall guard RUN:
 * sdk29 92.94% / engine96 79.09%; engine96 with sched1 MEASURED (flag not landed): 57.59%. */
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

/** func_0027C0A8 — thin wrapper: queue the ctx1 (FRAME_1 / FB A) draw-env REF
 *  packet. No params, no return. The empty asm statement keeps cc1 from
 *  sibcalling the lone tail call (the original keeps the call frame).
 *  MATCHED (task #493): 100.00% unit objdiff, sdk29 arm. */
void func_0027C0A8(void) {
    AppendDrawEnvContext1();
    __asm__ __volatile__("");
}

/* func_0027C0C8 (RenderHeldItemViewModel): PARKED #70 — TRACED, re-walled on a genuine blocker.
 * Held-item/view-model render pass. If the held item (*(void**)(g_sceneCastCount+0xC)) is null →
 * RenderMobys(). Else, fully traced .s-vs-Ghidra: read moby handle held+0x68; BeginMobyDrawSegment
 * + func_002A1000/1028; set/clear draw-flag bit0 at handle+0x34 around func_002A1138(g_mobyTableBase,-1);
 * SAVE camera matrix (CopyQwords g_cameraMatrix→stack, 0x40) + g_flCameraPos qword (g_cameraMatrix-0x230)
 * + the g_sceneActorMobys+0x674+0xB0 proj scale; SetupViewModelDepthRange(1.0f,8,8); build a 4-stage
 * view-model matrix chain (func_00129178 identity → func_001292C0/00129368/00129218 with the held+0x10/
 * +0x14/+0x18 FLOAT euler angles) and install it as the camera matrix; copy held[0] qword into
 * g_flCameraPos; BuildFrameViewMatrices; func_002A12A0(handle,0x404040,0xe,0,0); install a 4-light block
 * (g_dirLightMatrices+0x380 ← D_1A8820/8830/8840/8850 qwords at dest offsets 0,+0x20,+0x10,+0x30 —
 * INTERLEAVED); func_002A1138(handle,1); func_0027C0A8; restore proj scale + camera matrix + g_flCameraPos;
 * BuildCameraProjection/BuildFrameViewMatrices; AppendGsRegPacket(0x42,0x8000000044)+(8,5);
 * AppendTexFlushDefaultTex0; func_002A1058; FinishMobyRenderChain; then a 4-corner crosshair pass.
 *
 * GENUINE BLOCKER (why re-walled, per the un-park honesty guardrail): the terminal
 * ProjectAndClipBillboardQuad(func_00281540) consumes a 144-byte stack-context struct assembled in the
 * loop (sp+0x90.. : 4 corner qwords from held+0x20 stride 0x10, 4 colors 0x80808080, 8 uv floats
 * held+0x60/+0x64 * D_1A87F8/D_1A8808 + 0.5, then a header mode=5 / D_1A7470 / GIFtag 0xFF9000000260 /
 * alpha 0x8000000044). func_00281540 is OPAQUE (undecompiled, the same intricate GS vertex/UV-packing
 * class as the #70-parked DrawBlobShadows), so the struct's exact field layout can only be INFERRED from
 * these writes, not confirmed from the callee — a faithful #else would depend on reproducing that layout
 * byte-for-byte for an opaque consumer (silent-layout-bug risk). Everything up to the crosshair pass is
 * confidently traced; un-park becomes clean once func_00281540's context struct is decompiled/confirmed. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027C0C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RenderFrame);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CAD8);

/** DrawFrameWithoutHud — render a frame with every layer except the HUD: clear
 *  the screen, set the layer mask to 0x7F (all engine layers, HUD bits clear),
 *  then render. No params, no return. The empty asm statement keeps cc1 from
 *  sibcalling RenderFrame (the original keeps the call frame).
 *  MATCHED (task #493): 100.00% unit objdiff, sdk29 arm. */
void DrawFrameWithoutHud(void) {
    AppendScreenClearPacket(0);
    g_renderLayerMask = 0x7F;
    RenderFrame();
    __asm__ __volatile__("");
}

/** func_0027CB08 (RenderPauseMenuOverlay) — when the render-mode query
 *  returns 1, clear the screen, force the layer mask to 0x100FF (full world +
 *  HUD) and re-render the world behind the pause menu; when it returns 0, draw
 *  menu-screen widgets for screen 1. Always finishes with the save/load status
 *  popup. No params, no return. The empty asm statement keeps cc1 from
 *  sibcalling the trailing call, which is also what keeps the single shared
 *  epilogue (the sibcall variant duplicates the $ra restore into the branch
 *  delay slots). MATCHED (task #493): 100.00% unit objdiff, sdk29 arm. */
void func_0027CB08(void) {
    if (IsMenuOverlayActive() == 1) {
        AppendScreenClearPacket(0);
        g_renderLayerMask = 0x100FF;
        RenderFrame();
    } else if (IsMenuOverlayActive() == 0) {
        RenderMenuScreenWidgets(1);
    }
    RenderSaveLoadStatusPopup();
    __asm__ __volatile__("");
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CB70);

/** func_0027CB80 — render the front-end menu widgets for screen 0. No params,
 *  no return. The empty asm statement keeps cc1 from sibcalling the lone tail
 *  call (the original keeps the call frame).
 *  MATCHED (task #493): 100.00% unit objdiff, sdk29 arm. */
void func_0027CB80(void) {
    RenderMenuScreenWidgets(0);
    __asm__ __volatile__("");
}

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

/* Begin2dDrawBatch's absolute accesses. The ROM reads and writes every one of
 * these words with the assembler's `lui; lw/sw %lo` macro pair, so each goes
 * through an offset-0 alias sized 16 that cc1 sees as small data (the #8036
 * construct, as ResetPerFrameDrawQueues above; relocations name the real
 * symbols). The HUD pointers and the back-buffer base are read through
 * one-member struct views: cc1 2.9 lets a union store alias a varying struct
 * but not a fixed scalar, and the ROM re-reads these after each slot store. */
typedef struct { u8 *p; } HudPtrWord;
typedef struct { s32 v; } HudIntWord;
/* 8-byte HUD VRAM slot: +0 id, +4 VRAM block (halfword) */
typedef union { s16 vram; s32 word; } HudSlotWord;
typedef struct { s32 id; HudSlotWord block; } HudVramSlot;
#ifndef TARGET_NATIVE
__asm__(".extern g_frameDmaCursorAbs, 16\n\tg_frameDmaCursorAbs = g_frameDmaCursor");
__asm__(".extern g_vramDynamicBaseAbs, 16\n\tg_vramDynamicBaseAbs = g_vramDynamicBase");
__asm__(".extern g_2dBatchOpenTagAbs, 16\n\tg_2dBatchOpenTagAbs = g_2dBatchOpenTag");
__asm__(".extern g_uiTextureCountAbs, 16\n\tg_uiTextureCountAbs = g_uiTextureCount");
__asm__(".extern g_vramAllocCursorAbs, 16\n\tg_vramAllocCursorAbs = g_vramAllocCursor");
__asm__(".extern g_pHudAssetHeaderAbs, 16\n\tg_pHudAssetHeaderAbs = g_pHudAssetHeader");
__asm__(".extern g_hudTextureSlotsAbs, 16\n\tg_hudTextureSlotsAbs = g_hudTextureSlots");
__asm__(".extern g_vramFrameBufBAbs, 16\n\tg_vramFrameBufBAbs = g_vramFrameBufB");
__asm__(".extern g_hudClutSlotsAbs, 16\n\tg_hudClutSlotsAbs = g_hudClutSlots");
extern s32 g_frameDmaCursorAbs, g_2dBatchOpenTagAbs;
extern s32 g_vramDynamicBaseAbs, g_uiTextureCountAbs, g_vramAllocCursorAbs;
extern HudPtrWord g_pHudAssetHeaderAbs, g_hudTextureSlotsAbs, g_hudClutSlotsAbs;
extern HudIntWord g_vramFrameBufBAbs;
/* R5900_SHORT_LOOP_PAD1(v, next): SCHEDULING DEVICE (RULING #8435), one `nop`
 * before a short loop's closing branch, as text/1A8180.c (task #659). `v` is
 * the value the branch tests; `next` is the register reorg moves into the
 * branch's delay slot. Natively nothing. */
#define R5900_SHORT_LOOP_PAD1(v, next) \
    __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+r"(v) : "r"(next))
#else
#define g_frameDmaCursorAbs   (*(s32 *)&g_frameDmaCursor[0])
#define g_vramDynamicBaseAbs  g_vramDynamicBase
#define g_2dBatchOpenTagAbs   (*(s32 *)&g_2dBatchOpenTag)
#define g_uiTextureCountAbs   g_uiTextureCount
#define g_vramAllocCursorAbs  g_vramAllocCursor
#define g_pHudAssetHeaderAbs  (*(HudPtrWord *)&g_pHudAssetHeader[0])
#define g_hudTextureSlotsAbs  (*(HudPtrWord *)&g_hudTextureSlots)
#define g_hudClutSlotsAbs     (*(HudPtrWord *)&g_hudClutSlots)
#define g_vramFrameBufBAbs    (*(HudIntWord *)&g_vramFrameBufB)
#define R5900_SHORT_LOOP_PAD1(v, next) ((void)0)
#endif

/**
 * Open a 2D draw batch: stash the current frame DMA cursor as the batch open tag,
 * reserve a tag qword, reset the VRAM alloc cursor to the dynamic base, and clear
 * the texture-upload count. Then flush per-batch caches: zero every live
 * g_uiTextureCache entry and the two screen-grab tex0 handles.
 *
 * Unless `skipHudReset` is set, also evicts stale HUD textures: every
 * g_hudTextureSlots entry whose VRAM block is at/above the back framebuffer has
 * its +0x4 address cleared, and all g_hudClutSlots VRAM addresses are cleared
 * (counts from the HUD asset header +0x44 / +0x24, re-read every iteration).
 *
 * Match notes (every device below emits no instruction; NO register pin is
 * used - RULING #8598's pin was tried and dropped, NOTE #8618).
 * Percentages are the unit objdiff row for the named variant, solo, sdk29:
 *  - The VOLATILE empty fence on `cursor` (RULING #8483) keeps the head in
 *    source order; with a plain (non-volatile) fence sched1 hoists the
 *    texture-count load above the `+0x10` and swaps the cursor/alloc stores
 *    (97.08%). The dynamic base is read before the fence because the ROM
 *    loads it second (reading it after the fence: 94.38%).
 *  - Loop 1 is the ROM's count-down loop: the count stays in $5 and `left`
 *    is a copy of it (`move $3,$5`). One variable for both: 97.01%. The empty
 *    "+r" fence on `left` is needed too (without it 93.54%); three SHORT_LOOP
 *    pads give the ROM's `nop x3` before the `bnez`.
 *  - Loop 2 loads the header into `load` and copies it to `hdr` only AFTER
 *    reading +0x44 through `load`. The ROM keeps the header in its own
 *    register (`move $4,$2` / `move $4,$3` in the branch slots) and loop 3's
 *    first count read uses it. Copying first (`hdr = load` then the read)
 *    lets cse re-point the read at `hdr` and the copy disappears (82.50%, as
 *    NOTE #8596's body; seen in cc1's -ds dump). The slot address is spelled
 *    `i * 8 + base` in integer arithmetic for the ROM's `addu $4,$4,$5`
 *    operand order (the pointer spelling gives `addu $5,$5,$4`: 99.65%).
 *  - Loop 3 has its own counter (the ROM keeps it in $4, `hdr`'s register once
 *    `hdr` is dead; sharing `i`: 99.65%). The volatile empty fence at its
 *    head (RULING #8483) keeps reorg from moving the loop-top g_hudClutSlots
 *    load into the latch's delay slot, which the ROM leaves a `nop` (cc1
 *    sizes the aliased load as one instruction; without the fence 94.44%).
 * MATCHED (task #1110): 100.00% sdk29 (unit objdiff, objdiff_build.sh +
 * unit_report.sh), solo; verify_match_unit BYTE IDENTICAL. Was 43.99% (t493
 * #else, plain-promoted) / 82.50% (NOTE #8596 body).
 */
void Begin2dDrawBatch(s32 skipHudReset) {
    s32 cursor = g_frameDmaCursorAbs;
    s32 base = g_vramDynamicBaseAbs;
    s32 cacheCount;
    s32 left;
    s32 texCount;
    s32 i;
    s32 clut;
    u8 *load;
    u8 *hdr;

    g_2dBatchOpenTagAbs = cursor;
    __asm__ volatile("" : "+r"(cursor));
    g_frameDmaCursorAbs = cursor + 0x10;
    g_vramAllocCursorAbs = base;
    g_texUploadCount = 0;

    cacheCount = g_uiTextureCountAbs;
    if (cacheCount > 0) {
        u8 *p = g_uiTextureCache;
        left = cacheCount;
        do {
            *(u64 *)p = 0;
            __asm__("" : "+r"(left));
            left--;
            R5900_SHORT_LOOP_PAD1(left, left);
            R5900_SHORT_LOOP_PAD1(left, left);
            R5900_SHORT_LOOP_PAD1(left, p);
            p += 0x10;
        } while (left != 0);
    }

    g_screenGrabTex0Full = 0;
    g_screenGrabTex0Half = 0;
    D_1A8790 = 0;

    if (skipHudReset != 0) {
        return;
    }

    for (i = 0;
         i < (texCount = *(s32 *)((load = g_pHudAssetHeaderAbs.p) + 0x44), hdr = load, texCount);
         i++) {
        HudVramSlot *slot = (HudVramSlot *)(i * 8 + (u32)g_hudTextureSlotsAbs.p);
        if ((u16)slot->block.vram >= (g_vramFrameBufBAbs.v >> 8)) {
            slot->block.vram = 0;
        }
    }

    if (*(s32 *)(hdr + 0x24) > 0) {
        clut = 0;
        do {
            __asm__ volatile("" : "+r"(clut));
            ((HudVramSlot *)g_hudClutSlotsAbs.p)[clut].block.vram = 0;
            clut++;
        } while (clut < *(s32 *)(g_pHudAssetHeaderAbs.p + 0x24));
    }
}

extern void *g_2dBatchCloseTag; /* saved DMA cursor at batch close */
extern void FlushPendingTexUploads(void);
extern void AppendTexFlushDefaultTex0(void);

/* TODO(match) t493: sdk29 35.71% / engine96 32.45% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): GPREL-FORM (first differing
 * insn: ROM `lui v0,0x0  [HI16 0x001B2228]` vs built `addiu sp,sp,-64`). Levers:
 * cc1-small/absolute globals model RUN: 47.45% (engine96); engine96 with sched1 MEASURED (flag not
 * landed): 32.98%. */
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

/**
 * func_0027CDC8 — project a world position to normalized screen coordinates.
 *
 * Computes (worldPos - g_cameraPos), scales by 1024, transforms by the camera
 * matrix (func_00283A70, matrix at g_cameraPos-0x100) with w=1, then perspective-
 * divides by the transformed depth scaled by the projection factor
 * (g_cameraProjScale+0x160). Maps the result to pixel space (+ half screen
 * extent) and normalizes by the full screen dimension, writing x to *outX and y
 * to *outY (both in [0,1] across the viewport).
 */
/* TODO(match) t493: sdk29 44.33% / engine96 54.76% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): MACRO-AT (first differing
 * insn: ROM `sd s0,16(sp)` vs built `daddu v0,a0,zero`). Levers: cc1-small/absolute globals model
 * RUN: 56.59% (engine96); engine96 with sched1 MEASURED (flag not landed): 66.12%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027CDC8);
#else
extern u8 g_cameraPos[];        /* camera position Vec4; camera matrix at -0x100 */
extern u8 g_cameraProjScale[];  /* projection params; depth scale at +0x160 */
extern void Vec4SubVu0(void *dst, void *a, void *b);
extern void Vec4ScaleVu0(void *dst, f32 s, void *src);
extern f32 func_00283A70(void *out, void *in, void *matrix);

void func_0027CDC8(void *worldPos, f32 *outX, f32 *outY) {
    f32 v[4];   /* scratch Vec4 {x, y, z, w} */

    Vec4SubVu0(v, worldPos, g_cameraPos);
    Vec4ScaleVu0(v, 1024.0f, v);
    v[3] = 1.0f;
    func_00283A70(v, v, g_cameraPos - 0x100);
    Vec4ScaleVu0(v, *(f32 *)(g_cameraProjScale + 0x160) / v[3], v);
    *outX = v[0] + (f32)(g_screenWidth[0] >> 1);
    *outX = *outX / (f32)g_screenWidth[0];
    *outY = v[1] + (f32)(g_screenHeight[0] >> 1);
    *outY = *outY / (f32)g_screenHeight[0];
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", GetUiTextureTex0);

/* AddFxDrawHookPreParticles: register a (func,arg) callback in the
 * pre-particle fx draw queue (cap 0x40), run by RunFxDrawHooksPreParticles.
 * No-op when the queue is full. No return.
 * The count is read and written through the absolute alias
 * g_fxHooksPreCountAbs (the ROM's `lui; lw` / `lui $at; sw` macro pairs).
 * The two slot stores go through s32 views: that puts them in the count's
 * alias set, so cc1 keeps them ahead of the count store and schedules `n + 1`
 * after both slot addresses, as the ROM does. With the natural DrawHookFn /
 * void * stores, strict aliasing frees the count store and the order differs
 * by two rows. That residual was earlier read as a sched1 tie-break
 * (t493, FACT #7413); it is alias analysis. Pointers are 32-bit on both
 * targets, so the s32 views lose nothing.
 * MATCHED (task #946): sdk29 (-O2 -G8 -fno-gcse), solo. */
void AddFxDrawHookPreParticles(DrawHookFn func, void *arg) {
    s32 n = g_fxHooksPreCountAbs;
    if (n < 0x40) {
        s32 *funcSlot = (s32 *)&g_fxHooksPreFuncs[n];
        s32 *argSlot = (s32 *)&g_fxHooksPreArgs[n];
        *funcSlot = (s32)func;
        *argSlot = (s32)arg;
        g_fxHooksPreCountAbs = n + 1;
    }
}

/* Run every registered pre-particle fx draw hook in order, each as
 * func(arg). The count is re-read each iteration so a hook may extend the
 * queue. Driver for AddFxDrawHookPreParticles.
 * Near-miss: the three-deep packed callee-save block (sd $16/$17/$18) + the
 * branch-likely re-test loop is a register/save-layout wall. Correct C
 * preserved as the portable body. */
/* TODO(match) t493: sdk29 78.17% / engine96 57.90% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-32` vs built `addiu sp,sp,-80`). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunFxDrawHooksPreParticles);
#else
void RunFxDrawHooksPreParticles(void) {
    s32 i;
    for (i = 0; i < g_fxHooksPreCount[0]; i++)
        g_fxHooksPreFuncs[i](g_fxHooksPreArgs[i]);
}
#endif

/* func_0027D500 (AddDrawHookAfterTies): register a (func,arg) callback in the
 * after-ties draw queue (cap 0x40), run by RunDrawHooksAfterTies. No-op when
 * the queue is full. No return. Same shape and the same levers as
 * AddFxDrawHookPreParticles: the absolute count alias, and s32 slot views
 * that keep the slot stores ahead of the count store.
 * MATCHED (task #946): sdk29 (-O2 -G8 -fno-gcse), solo. */
void func_0027D500(DrawHookFn func, void *arg) {
    s32 n = g_drawHooksAfterTiesCountAbs;
    if (n < 0x40) {
        s32 *funcSlot = (s32 *)&g_drawHooksAfterTiesFuncs[n];
        s32 *argSlot = (s32 *)&g_drawHooksAfterTiesArgs[n];
        *funcSlot = (s32)func;
        *argSlot = (s32)arg;
        g_drawHooksAfterTiesCountAbs = n + 1;
    }
}

/* Run every registered after-ties draw hook in order as func(arg); the count is
 * re-read each iteration. Driver for func_0027D500 (AddDrawHookAfterTies).
 * Near-miss: same packed-save / branch-likely loop wall as
 * RunFxDrawHooksPreParticles. Correct C preserved as the portable body. */
/* TODO(match) t493: sdk29 78.17% / engine96 57.90% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-32` vs built `addiu sp,sp,-80`). */
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
/* TODO(match) t493: sdk29 78.17% / engine96 57.90% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-32` vs built `addiu sp,sp,-80`). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunDrawHooksAfterShrubs);
#else
void RunDrawHooksAfterShrubs(void) {
    s32 i;
    for (i = 0; i < g_drawHooksAfterShrubsCount[0]; i++)
        g_drawHooksAfterShrubsFuncs[i](g_drawHooksAfterShrubsArgs[i]);
}
#endif

/* AddFxDrawHookPostParticles: register a (func,arg) callback in the
 * post-particle fx draw queue (cap 0x40), run by RunFxDrawHooksPostParticles.
 * No-op when the queue is full. No return. Same shape and the same levers as
 * AddFxDrawHookPreParticles.
 * MATCHED (task #946): sdk29 (-O2 -G8 -fno-gcse), solo. */
void AddFxDrawHookPostParticles(DrawHookFn func, void *arg) {
    s32 n = g_fxHooksPostCountAbs;
    if (n < 0x40) {
        s32 *funcSlot = (s32 *)&g_fxHooksPostFuncs[n];
        s32 *argSlot = (s32 *)&g_fxHooksPostArgs[n];
        *funcSlot = (s32)func;
        *argSlot = (s32)arg;
        g_fxHooksPostCountAbs = n + 1;
    }
}

/* Run every registered post-particle fx draw hook in order as func(arg); the
 * count is re-read each iteration. Driver for AddFxDrawHookPostParticles.
 * Near-miss: same packed-save / branch-likely loop wall as
 * RunFxDrawHooksPreParticles. Correct C preserved as the portable body. */
/* TODO(match) t493: sdk29 78.17% / engine96 57.90% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-32` vs built `addiu sp,sp,-80`). */
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
/* TODO(match) t493: sdk29 51.75% / engine96 44.25% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SCHED — with the cc1-small count
 * model 68.25%/77.00% sdk29; the original computes the Funcs slot address, stores, then the Args
 * address (cap 4 variant), cc1 hoists both address computations. Levers: engine96 with sched1
 * MEASURED (flag not landed): 43.25%. */
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
/* TODO(match) t493: sdk29 71.00% / engine96 50.87% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-32` vs built `addiu sp,sp,-80`). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", RunFxDrawHooksLate);
#else
void RunFxDrawHooksLate(void) {
    s32 i;
    for (i = 0; i < g_fxHooksLateCount[0]; i++)
        g_fxHooksLateFuncs[i](g_fxHooksLateArgs[i]);
}
#endif

/* DrawBlobShadows: PARKED #70 (#else not confident) — GS blob-shadow renderer that
 * projects/scales shadow quads (Vec3RescaleToLenVu0) and emits GS sprite packets
 * (GetUiTextureTex0 + func_00281540) with the same intricate GS vertex/UV packing class
 * as DrawGlyphQuad. Needs the GS packet format + projection math traced before a faithful
 * #else. Not forcing a low-confidence body. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawBlobShadows);

/* FadeOutToBlackBlocking(frames) - synchronous fade-to-black run OUTSIDE the normal
 * game loop (level transitions block on it). Each of `frames` iterations waits the
 * frame DMA fence + vblank, resets the frame arenas, re-emits the draw environment +
 * screen clear, sets GS TEST_1 (reg 0x42) and ramps the RGBAQ (reg 1) alpha from 0 up
 * to 0x80 across the frame count, appends the prebuilt black-overlay quad
 * (g_fadeQuadPacket) to the frame chain, then kicks the DMA chain and flips the arena.
 * The per-frame fence stamp (g_renderLayerMask+0x4) is bumped after every vblank wait.
 * A final fence/vblank/reset + draw-env/clear leaves the next real frame on a cleared
 * black screen. Direction is a fade-IN of the black overlay (= fade-out of the scene).
 *
 * Engine region (ee-gcc 2.96) - faithful #else. The alpha ramp integer-divides by the
 * decreasing remaining-frame count (i+1); the original's break-on-div-zero guard is
 * implicit in C since the divisor is always >= 1. */
/* TODO(match) t493: sdk29 73.11% / engine96 65.00% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `addiu sp,sp,-48` vs built `addiu sp,sp,-112`). Levers: cc1-small/absolute globals model
 * RUN: 66.84% (engine96); engine96 with sched1 MEASURED (flag not landed): 65.10%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", FadeOutToBlackBlocking);
#else
extern u8   g_fadeQuadPacket[];       /* prebuilt black-overlay GS quad packet */
extern void AppendDrawEnvContext2(void);
extern void KickFrameDmaChain(void);
extern void FlipFrameArena(void);

void FadeOutToBlackBlocking(s32 frames)
{
    s32 i;

    WaitFrameDmaFence(1);
    WaitVblankGetField(0);
    *(s32 *)((u8 *)&g_renderLayerMask + 4) += 1;
    ResetFrameArenas();

    for (i = frames - 1; i >= 0; i--) {
        u32 *cursor;
        AppendDrawEnvContext1();
        AppendScreenClearPacket(1);
        AppendDrawEnvContext2();
        AppendGsRegPacket(0x42, 0x8000000044ULL);
        AppendGsRegPacket(1, (u64)(0x80 - (i * 0x80) / (i + 1)) << 0x18);
        cursor = g_frameDmaCursor[0];
        cursor[0] = 0x30000014;
        cursor[1] = (u32)g_fadeQuadPacket;
        cursor[2] = 0;
        cursor[3] = 0x50000014;
        g_frameDmaCursor[0] = cursor + 4;
        WaitFrameDmaFence(1);
        WaitVblankGetField(0);
        *(s32 *)((u8 *)&g_renderLayerMask + 4) += 1;
        KickFrameDmaChain();
        FlipFrameArena();
    }

    WaitFrameDmaFence(1);
    WaitVblankGetField(0);
    *(s32 *)((u8 *)&g_renderLayerMask + 4) += 1;
    ResetFrameArenas();
    AppendDrawEnvContext1();
    AppendScreenClearPacket(1);
}
#endif

/**
 * func_0027DB38 — draw a pulsing debug string.
 *
 * Drives a 100-frame triangle wave from g_sceneFrame (frame%100 mapped to
 * [-1,1]), shapes it with cos(t·pi) into a 0..1 pulse, and blends two packed
 * colors (0x7FE0E0E0 / 0x5FF0C070, ColorLerpPacked) by that pulse. Draws the
 * localized string 0x2DB6 (DrawDebugString) at x=0x3C with a y that depends on
 * the fade-suppress flag D_1A7BB9 (0x17C when set, else 0x148).
 */
/* TODO(match) t493: sdk29 63.17% / engine96 44.76% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `lui v0,0x0  [HI16 0x001B87F4]` vs built `lw v1,0(gp)  [GPREL16 0x001B87F4]`). Levers:
 * sibcall guard RUN: sdk29 74.68% / engine96 49.36%; cc1-small/absolute globals model RUN: 68.08%
 * (sdk29); engine96 with sched1 MEASURED (flag not landed): 57.05%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027DB38);
#else
extern s32 g_sceneFrame;
extern u8 D_1A7BB9;
extern f32 func_00283B30(f32 angle);                        /* cosine */
extern s32 ColorLerpPacked(f32 t, s32 colorA, s32 colorB);    /* blend packed colors by t */
extern char *GetLocalizedString(s32 id);
extern void DrawDebugString(s32 a, s32 b, s32 c, s32 d, s32 e);

void func_0027DB38(void) {
    s32 param = (D_1A7BB9 != 0) ? 0x17C : 0x148;
    f32 t = (f32)(g_sceneFrame % 100) * 0.02f - 1.0f;
    f32 pulse;
    s32 color;

    if (t < -1.0f) {
        t = -1.0f;
    }
    if (1.0f < t) {
        t = 1.0f;
    }
    pulse = (func_00283B30(t * 3.1415927f) + 1.0f) * 0.5f;
    color = ColorLerpPacked(pulse, 0x7FE0E0E0, 0x5FF0C070);
    DrawDebugString(0x3C, param, color, (s32)GetLocalizedString(0x2DB6), -1);
}
#endif

/* func_0027DC40: PARKED #70 — a debug-overlay draw routine (DrawDebugString + the
 * func_00280BB8/func_00280C98 font wrappers, func_0027F208). Modest but multi-callee with
 * layout logic; needs the string/arg wiring traced before a confident #else. */
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
/* TODO(match) t493: sdk29 17.01% / engine96 31.42% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): MACRO-AT (first differing
 * insn: ROM `lui v0,0x0  [HI16 0x001B1524]` vs built `lui a1,0x0  [HI16 0x001B1524]`). Levers:
 * cc1-small/absolute globals model RUN: 31.25% (engine96); engine96 with sched1 MEASURED (flag not
 * landed): 11.82%. */
/* NEAR-MISS t1101: sdk29 69.68% (unit objdiff, objdiff_build.sh + unit_report.sh, solo,
 * VM b, unit flags unchanged), up from this #else's 17.01%. Body in NOTE #8596. The reload
 * structure matches the ROM, with no unit flag: an integer-typed struct view of the cursor
 * (re-read after every DMA-tag word and at every block start, since only the u32 stores
 * conflict with it), packet stores through a union, struct views of the pixel offsets
 * (re-read per vertex), and a `.extern ,4` gp alias for the delay-slot %gp_rel reads of
 * g_screenFadeWhite+8 and the first g_frameDmaCursor. -fno-strict-aliasing is neutral on
 * this body: it compiles byte-identical with and without the flag (FACT #8613).
 * Residual: cc1's sched1 hoists five constants (0x8001, 0x104, 0x80000000, 0x8008, the mask)
 * to the head (+0x74..+0xB0), where the ROM computes each 2-13 words before its first use
 * (+0xBC..+0x120). count<<4 runs the other way (FACT #8660): the ROM computes it at the head
 * (+0x74, 62 words before its first use at +0x16C) and cc1 later, at +0xB4.
 * -fno-schedule-insns does not reproduce the ROM either.
 * The mask's `ori;dsll 16;ori;dsll 24` is the SN assembler's dli expansion (RULING #8549
 * class). D_1A7358/D_1A735C are the screen's far X/Y edges
 * (g_gsPixelOffsetX+8/+0xC); the #else below reads them as g_gsPixelOffsetY[1]/[2]. */
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

/**
 * func_0027E1E8 — emit the layered letterbox/scissor strips from the active
 * layout descriptor (D_1A8758).
 *
 * If the descriptor's top scissor value (+0x8) is set, writes a GS SCISSOR
 * (0x42) packet from its low+high bytes; if the top border colour (+0x4) has a
 * nonzero alpha byte, fills the whole screen with it (func_0027E4D0). Then walks
 * the screen top-to-bottom in two alternating bands per row — band A (step +0x10,
 * scissor +0x18, colour +0x14) then band B (step +0x20, scissor +0x28, colour
 * +0x24) — emitting each band's scissor packet and a colour fill clamped to the
 * screen bottom, until the cursor passes the screen height.
 */
/* TODO(match) t493: sdk29 66.35% / engine96 54.94% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-48` vs built `addiu sp,sp,-128`). Levers: engine96 with sched1 MEASURED
 * (flag not landed): 67.19%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E1E8);
#else
extern u8 *D_1A8758;   /* active scissor-strip layout descriptor */
extern void func_0027E4D0(s32 y0, s32 y1, s32 x0, s32 x1, u64 color);

void func_0027E1E8(void) {
    u8 *desc = D_1A8758;
    s32 height = *(s16 *)(g_gsScreenContext + 0x152);
    s32 y;

    if (*(u64 *)(desc + 0x8) != 0) {
        AppendGsRegPacket(0x42, *(u64 *)(desc + 0x8) & 0x000000FF000000FFULL);
    }
    if (*(u32 *)(desc + 0x4) & 0xFF000000) {
        func_0027E4D0(0, height, 0, *(s16 *)(g_gsScreenContext + 0x150),
                      (u64)*(u32 *)(desc + 0x4));
    }
    if (height > 0) {
        y = 0;
        do {
            if (*(u64 *)(desc + 0x18) != 0) {
                AppendGsRegPacket(0x42, *(u64 *)(desc + 0x18) & 0x000000FF000000FFULL);
            }
            if (*(u32 *)(desc + 0x14) & 0xFF000000) {
                s32 y1 = y + *(s32 *)(desc + 0x10);
                if (y1 >= height - 1) {
                    y1 = height - 1;
                }
                func_0027E4D0(y, y1, 0, *(s16 *)(g_gsScreenContext + 0x150),
                              (u64)*(u32 *)(desc + 0x14));
            }
            y += *(s32 *)(desc + 0x10);
            if (*(u64 *)(desc + 0x28) != 0) {
                AppendGsRegPacket(0x42, *(u64 *)(desc + 0x28) & 0x000000FF000000FFULL);
            }
            if (*(u32 *)(desc + 0x24) & 0xFF000000) {
                s32 y1 = y + *(s32 *)(desc + 0x20);
                if (y1 >= height - 1) {
                    y1 = height - 1;
                }
                func_0027E4D0(y, y1, 0, *(s16 *)(g_gsScreenContext + 0x150),
                              (u64)*(u32 *)(desc + 0x24));
            }
            y += *(s32 *)(desc + 0x20);
        } while (y < height);
    }
}
#endif

/**
 * func_0027E368 — emit the GS scissor / Z-buffer register packets for a render
 * context.
 *
 * When the context's 64-bit field at +0x8 is set, appends GS reg 0x42 (SCISSOR)
 * masked to its low + high bytes. When the +0x4 field's top byte is set, appends
 * a Z-buffer setup: reg 0x4E (ZBUF) from g_vramZBuffer>>13 with the frame + mask
 * bits, a scissored screen rect (func_0027E4D0 over the g_gsScreenContext dims),
 * and a second ZBUF variant. Finally re-appends reg 0x42 (0x44 variant) when
 * +0x8 is set.
 */
/* TODO(match) t493: sdk29 70.35% / engine96 62.24% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `addiu sp,sp,-32` vs built `addiu sp,sp,-64`). Levers: cc1-small/absolute globals model RUN:
 * 66.63% (engine96). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E368);
#else
extern s32 g_vramZBuffer;
extern void func_0027E4D0(s32 y0, s32 y1, s32 x0, s32 x1, u64 arg4);

void func_0027E368(void *ctx) {
    u8 *p = (u8 *)ctx;

    if (*(u64 *)(p + 0x8) != 0) {
        AppendGsRegPacket(0x42, *(u64 *)(p + 0x8) & 0x000000FF000000FFULL);
    }
    if ((*(u32 *)(p + 0x4) & 0xFF000000) != 0) {
        AppendGsRegPacket(0x4E,
            (u64)(g_vramZBuffer >> 13) | 0x01000000ULL | 0x100000000ULL);
        func_0027E4D0(0, *(s16 *)(g_gsScreenContext + 0x152), 0,
                      *(s16 *)(g_gsScreenContext + 0x150), *(u32 *)(p + 0x4));
        AppendGsRegPacket(0x4E, (u64)((g_vramZBuffer >> 13) | 0x01000000));
    }
    if (*(u64 *)(p + 0x8) != 0) {
        AppendGsRegPacket(0x42, 0x0000008000000044ULL);
    }
}
#endif

/* DrawFullScreenTint - append a full-screen alpha-tint draw to the frame DMA
 * chain: emit a GS RGBAQ register write packed from the (r,g,b,a) components,
 * then a 4-word DMACnt+ref chain entry pointing at the prebuilt tint quad
 * packet, and advance the cursor by 0x10 bytes.
 * Near-miss: the pinned cc1 CSEs the frame-DMA-cursor pointer load across the
 * four chain-word stores, whereas the original reloads it absolute for each
 * write (no-load-PRE). No declaration reproduces the per-store reload here.
 * Correct C preserved as the portable body. */
/* TODO(match) t493: sdk29 38.97% / engine96 54.28% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): RELOAD — the original reloads
 * g_frameDmaCursor before each of the 4 packet stores (no type-based alias analysis); cc1 2.9 -O2
 * keeps it in a register. MEASURED, not landed: with -fno-strict-aliasing on this unit (a per-unit
 * flag = landing-gate question) plus the cc1-small model and left-to-right `or`s it reads 91.11%
 * sdk29, residual SCHED (jal / last `or` order). Levers: cc1-small/absolute globals model RUN:
 * 38.69% (sdk29); -fno-strict-aliasing MEASURED (flag not landed): 91.11% sdk29; engine96 with
 * sched1 MEASURED (flag not landed): 51.25%.
 * Task #946 (cc1 2.9 probes, match.sh): the "no declaration" line above is false. The per-store
 * reload is pure C under strict aliasing: read the cursor through a 1-element s32/u32 array view
 * that cc1 sees as small, e.g. `extern u32 curAbs[1]` equated to g_frameDmaCursor at offset 0
 * with `.extern ,16` (the ResetPerFrameDrawQueues construct). Store through
 * `((u32 *)curAbs[0])[k]` and bump with `curAbs[0] += 0x10`. With that, the whole tail after the
 * jal is word-for-word the ROM's. A u32 scalar view reloads only once: cc1 2.9's fixed-scalar /
 * varying-struct rule lets the indexed stores skip it, and the array view is itself an
 * in-struct access. Head: `u64 v = r | g << 8; v |= b << 16; v |= a << 24;` then
 * `__asm__ __volatile__("" : "+r"(v));` before the call gives the ROM's or-chain and
 * `li a0,1` in the jal slot. What remains is 1 adjacent swap: the ROM issues `sd ra` between
 * `dsll a3,24` and the last `or`, cc1 after it. 384 non-volatile barrier variants (barrier
 * subsets x shift orders x a pinned first argument) did not move it. */
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
/* TODO(match) t493: sdk29 11.56% / engine96 14.67% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): GPREL-FORM (first differing
 * insn: ROM `lui v1,0x0  [HI16 0x001B2228]` vs built `lui t4,0x0  [HI16 0x001B2228]`). Levers:
 * cc1-small/absolute globals model RUN: 18.44% (sdk29); -fno-strict-aliasing MEASURED (flag not
 * landed): 19.21% sdk29; engine96 with sched1 MEASURED (flag not landed): 20.87%. */
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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E690);

/* DrawGlyphQuad(x, y, w, h, u, v, uw, uh, param9, param10) — emit a textured 2D
 * sprite/glyph quad into the frame DMA packet at g_frameDmaCursor. Leaf function: writes
 * an opening DMA/GIF tag (0x10000007 .. 0x50000007), copies the 16-byte GIFtag template
 * (D_1AC880), then the caller's two 64-bit header words (param10, param9), a GS reg-0x154
 * marker, and 4 UV+XYZ2 vertex pairs for the quad corners, and advances the cursor 0x80.
 * Screen positions are 12.4 fixed-point: x = pixel*0x10 + g_gsPixelOffsetX/Y - 8 (half-texel
 * bias); the XYZ2 vertex packs x | (y<<16) | the constant z/fog field 0xfffff000000000; the
 * UV packs u<<4 + v<<20. Corners are (x0,y0)(x1,y0)(x0,y1)(x1,y1) with x1=x0+w, etc.
 *
 * UN-PARK of a provably-false #70: the park's blockers — "which arg is x/y/w/h/u/v" and the
 * "exact GS vertex/UV packing" — are both resolved by a full .s trace (leaf, no calls, so no
 * helper-arg ambiguity). It IS 10-arg (param9/param10 are real 64-bit stack args at 0x10/0x18
 * ($sp), written verbatim to the header — NOT a coord); no divergent-arity caller in this TU,
 * so no Option-D needed. Engine region — faithful #else. FAITHFULNESS (GS-pack/silent-render):
 * XYZ pack built in s64 (native long is 32-bit, overflows the <<16 + z-field). FORMER-PARK:
 * dual-gate (tester oracle) + d2 heads-up. */
/* TODO(match) t493: sdk29 13.13% / engine96 28.17% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): MACRO-AT (first differing
 * insn: ROM `addiu sp,sp,-16` vs built `addiu sp,sp,-32`). Levers: cc1-small/absolute globals
 * model RUN: 27.24% (sdk29); -fno-strict-aliasing MEASURED (flag not landed): 41.44% sdk29;
 * engine96 with sched1 MEASURED (flag not landed): 34.96%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawGlyphQuad);
#else
extern u8  D_1AC880[];   /* GIFtag template (16-byte GS primitive header) */

void DrawGlyphQuad(s32 x, s32 y, s32 w, s32 h, s32 u, s32 v, s32 uw, s32 uh,
                   u64 param9, u64 param10) {
    u32 *base = g_frameDmaCursor[0];
    u64 *q = (u64 *)base;
    s32 offX = g_gsPixelOffsetX[0];
    s32 offY = g_gsPixelOffsetY[0];
    s32 x0 = x * 0x10 + offX - 8;
    s32 x1 = (x + w) * 0x10 + offX - 8;
    s32 y0 = y * 0x10 + offY - 8;
    s32 y1 = (y + h) * 0x10 + offY - 8;
    s32 u0 = u << 4;
    s32 u1 = (u + uw) << 4;
    s32 v0 = v << 20;
    s32 v1 = (v + uh) << 20;
    const s64 zmask = 0xfffff000000000LL;

    base[0] = 0x10000007;
    base[1] = 0;
    base[2] = 0;
    base[3] = 0x50000007;
    q[2] = ((u64 *)D_1AC880)[0];                   /* template -> C+0x10 */
    q[3] = ((u64 *)D_1AC880)[1];                   /* template -> C+0x18 */
    q[4]  = param10;                               /* C+0x20 */
    q[5]  = 0x154;                                 /* C+0x28 GS reg id */
    q[6]  = param9;                                /* C+0x30 */
    q[7]  = (u64)(v0 + u0);                        /* C+0x38 uv(u0,v0) */
    q[8]  = (s64)x0 | ((s64)y0 << 16) | zmask;     /* C+0x40 xyz(x0,y0) */
    q[9]  = (u64)(v0 + u1);                        /* C+0x48 uv(u1,v0) */
    q[10] = (s64)x1 | ((s64)y0 << 16) | zmask;     /* C+0x50 xyz(x1,y0) */
    q[11] = (u64)(v1 + u0);                        /* C+0x58 uv(u0,v1) */
    q[12] = (s64)x0 | ((s64)y1 << 16) | zmask;     /* C+0x60 xyz(x0,y1) */
    q[13] = (u64)(v1 + u1);                        /* C+0x68 uv(u1,v1) */
    q[14] = (s64)x1 | ((s64)y1 << 16) | zmask;     /* C+0x70 xyz(x1,y1) */
    q[15] = 0;                                     /* C+0x78 */
    g_frameDmaCursor[0] = base + 0x20;             /* advance to C+0x80 */
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027E818);

/* DrawTexturedQuad2d(x, y, w, h, u, v, uw, vh, colors[4], tex0) — draw a float-coordinate
 * textured quad with per-vertex RGBA (gouraud) into the frame DMA packet. Screen positions are
 * 12.4 fixed-point (FloatToInt(coord*16) + g_gsPixelOffsetX/Y[0] - 8); the quad is clipped against
 * the GS window [0x7000, 0x9000] and dropped entirely if any edge falls outside. Emits a 9-qword
 * GIF packet: DMA/GIF tag, a TEX0 + ST-clamp descriptor, tex0, then 4 {RGBA, ST, XYZ2} vertices for
 * corners (u0,v0)(u1,v0)(u0,v1)(u1,v1). ST packs u<<4 + v<<20; XYZ packs x | (y<<16) | z-field
 * 0xfffff000000000; the descriptor packs u0<<4 | u1<<0xe | 0xa | v0<<0x18 | v1<<0x22.
 *
 * Engine region — faithful #else, whole-.s traced (Ghidra-complete; 0 lq/sq so all copies are 8-byte
 * sd/4-byte sw). GS-pack safeguard: all 64-bit packs (XYZ, ST, descriptor) built in s64 — native long
 * is 32-bit and would overflow the <<16/<<20/<<0x22 fields + the z-field. */
/* TODO(match) t493: sdk29 46.59% / engine96 41.15% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): MACRO-AT (first differing insn:
 * ROM `addiu sp,sp,-144` vs built `addiu sp,sp,-224`). Levers: cc1-small/absolute globals model
 * RUN: 36.10% (engine96); engine96 with sched1 MEASURED (flag not landed): 32.70%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawTexturedQuad2d);
#else
extern s32 FloatToInt(f32 x);   /* declared file-scope below, used before it here */

void DrawTexturedQuad2d(f32 x, f32 y, f32 w, f32 h, s32 u, s32 v, s32 uw, s32 vh,
                        u32 *colors, u64 tex0) {
    s32 x0 = FloatToInt(x * 16.0f) + g_gsPixelOffsetX[0] - 8;
    s32 x1 = FloatToInt((x + w) * 16.0f) + g_gsPixelOffsetX[0] - 8;
    s32 y0 = FloatToInt(y * 16.0f) + g_gsPixelOffsetY[0] - 8;
    s32 y1 = FloatToInt((y + h) * 16.0f) + g_gsPixelOffsetY[0] - 8;

    if (x0 < 0x9001 && x1 > 0x6fff && y0 < 0x9001 && y1 > 0x6fff) {
        u32 *base = g_frameDmaCursor[0];
        s32 u1 = u + uw;
        s32 v1 = v + vh;
        s64 st_u0 = (s64)(u << 4), st_u1 = (s64)(u1 << 4);
        s64 st_v0 = (s64)(v << 20), st_v1 = (s64)(v1 << 20);
        s64 zy0 = ((s64)y0 << 16) | 0xfffff000000000LL;
        s64 zy1 = ((s64)y1 << 16) | 0xfffff000000000LL;

        base[0] = 0x10000009;
        base[1] = 0;
        base[2] = 0;
        base[3] = 0x50000009;
        base[4] = 0x8001;
        base[5] = 0x4000000;
        base[6] = 0x31531068;
        base[7] = 0x85315315;
        *(u64 *)(base + 8)    = ((s64)u << 4) | ((s64)u1 << 0xe) | 0xa | ((s64)v << 0x18) | ((s64)v1 << 0x22);
        *(u64 *)(base + 0xa)  = tex0;
        base[0xc] = 0x15c;
        base[0xd] = 0;
        *(u64 *)(base + 0xe)  = colors[0];        /* vertex 0 (u0,v0) */
        *(u64 *)(base + 0x10) = st_v0 + st_u0;
        *(u64 *)(base + 0x12) = (s64)x0 | zy0;
        *(u64 *)(base + 0x14) = colors[1];        /* vertex 1 (u1,v0) */
        *(u64 *)(base + 0x16) = st_v0 + st_u1;
        *(u64 *)(base + 0x18) = (s64)x1 | zy0;
        *(u64 *)(base + 0x1a) = colors[2];        /* vertex 2 (u0,v1) */
        *(u64 *)(base + 0x1c) = st_v1 + st_u0;
        *(u64 *)(base + 0x1e) = (s64)x0 | zy1;
        *(u64 *)(base + 0x20) = colors[3];        /* vertex 3 (u1,v1) */
        *(u64 *)(base + 0x22) = st_v1 + st_u1;
        *(u64 *)(base + 0x24) = (s64)x1 | zy1;
        base[0x26] = 5;
        base[0x27] = 0;
        g_frameDmaCursor[0] = base + 0x28;
    }
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027EB20);

/* DrawRotatedSprite2d: PARKED #70 — large (13 callee-save) rotated-sprite emitter with
 * sin/cos rotation (func_00283B30/B48), Vec4 transforms and FP rounding (FloatToInt) into
 * a GS packet. Intricate FP+GS-packing class (silent-misrender risk); needs the rotation +
 * GS format traced before a faithful #else. */
/* DrawRotatedSprite2d(cx, cy, hh, hw, angle, pivX, pivY, u, v, tex0, zField, uvDesc, mirrorX, mirrorY)
 * — draw a rotated/scaled textured HUD-icon quad. Builds two rotated edge vectors from the angle
 * (edge1 = hw*(cos,sin), edge2 = hh*(sin,-cos)), then the 4 corners around the fractional pivot
 * (pivX,pivY): c = center +/- (1-piv or piv)*edge1 +/- (1-piv or piv)*edge2 (via Vec4Scale/Add/Sub).
 * mirrorX/mirrorY swap the U/V winding (u<<4 vs 0x10 / v<<0x14 vs 0x100000). Emits a 7-qword GIF
 * packet: DMA/GIF tag + TEX0/ST descriptor + tex0 + 4 {UV, XYZ2} vertices whose screen coords are
 * FloatToInt(corner*16) + g_gsPixelOffsetX/Y[0] - 8, with the z-field zField<<0x20.
 *
 * Engine region — faithful #else, whole-.s traced (Ghidra-complete, 0 lq/sq → all 8-byte sd). GS-pack
 * safeguard: all 64-bit packs (UV, XYZ, z-field) in s64. */
/* TODO(match) t493: sdk29 45.85% / engine96 48.00% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): MACRO-AT (first differing
 * insn: ROM `andi t1,t1,0xff` vs built `sll t2,t2,0x18`). Levers: cc1-small/absolute globals model
 * RUN: 47.19% (engine96); engine96 with sched1 MEASURED (flag not landed): 46.06%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawRotatedSprite2d);
#else
extern s32  FloatToInt(f32 x);
extern f32  SinfVu0(f32 x);
extern f32  CosfVu0(f32 x);
extern void Vec4AddVu0(void *dst, void *a, void *b);

void DrawRotatedSprite2d(f32 cx, f32 cy, f32 hh, f32 hw, f32 angle, f32 pivX, f32 pivY,
                         s32 u, s32 v, u64 tex0, s32 zField, s32 uvDesc, s8 mirrorX, s8 mirrorY) {
    f32 center[4], edge1[4], edge2[4], c0[4], c1[4], c2[4], c3[4];
    u8  tmp[16];
    f32 co = CosfVu0(angle);
    f32 si = SinfVu0(angle);
    s64 z  = (s64)zField << 0x20;
    s32 uA, uB, px, py;
    s64 vA, vB;
    u32 *base;

    if (mirrorX == 0) { uB = u << 4; uA = 0x10; } else { uA = u << 4; uB = 0x10; }
    if (mirrorY == 0) { vB = (s64)(v << 0x14); vA = 0x100000; }
    else              { vA = (s64)(v << 0x14); vB = 0x100000; }

    center[0] = cx;      center[1] = cy;
    edge1[0]  = hw * co; edge1[1]  = hw * si;
    edge2[0]  = hh * si; edge2[1]  = -hh * co;

    Vec4ScaleVu0(tmp, 1.0f - pivY, edge1);  Vec4AddVu0(c0, center, tmp);
    Vec4ScaleVu0(tmp, 1.0f - pivX, edge2);  Vec4SubVu0(c0, c0, tmp);
    Vec4ScaleVu0(tmp, 1.0f - pivY, edge1);  Vec4AddVu0(c1, center, tmp);
    Vec4ScaleVu0(tmp, pivX, edge2);         Vec4AddVu0(c1, c1, tmp);
    Vec4ScaleVu0(tmp, pivY, edge1);         Vec4SubVu0(c2, center, tmp);
    Vec4ScaleVu0(tmp, 1.0f - pivX, edge2);  Vec4SubVu0(c2, c2, tmp);
    Vec4ScaleVu0(tmp, pivY, edge1);         Vec4SubVu0(c3, center, tmp);
    Vec4ScaleVu0(tmp, pivX, edge2);         Vec4AddVu0(c3, c3, tmp);

    base = g_frameDmaCursor[0];
    base[0] = 0x10000007;
    base[1] = 0;
    base[2] = 0;
    base[3] = 0x50000007;
    base[4] = 0x8001;
    base[5] = 0xb4000000;
    base[6] = 0x35353106;
    base[7] = 0x535;
    *(u64 *)(base + 8) = tex0;
    base[0xa] = 0x154;
    base[0xb] = 0;
    *(s64 *)(base + 0xc) = (s64)uvDesc;
    *(u64 *)(base + 0xe) = (s64)uA | vA;                                          /* v0 UV */
    px = FloatToInt(c0[0] * 16.0f); py = FloatToInt(c0[1] * 16.0f);
    *(u64 *)(base + 0x10) = (s64)(px + g_gsPixelOffsetX[0] - 8) | ((s64)(py + g_gsPixelOffsetY[0] - 8) << 16) | z;
    *(u64 *)(base + 0x12) = (s64)uB | vA;                                         /* v1 UV */
    px = FloatToInt(c1[0] * 16.0f); py = FloatToInt(c1[1] * 16.0f);
    *(u64 *)(base + 0x14) = (s64)(px + g_gsPixelOffsetX[0] - 8) | ((s64)(py + g_gsPixelOffsetY[0] - 8) << 16) | z;
    *(u64 *)(base + 0x16) = (s64)uA | vB;                                         /* v2 UV */
    px = FloatToInt(c2[0] * 16.0f); py = FloatToInt(c2[1] * 16.0f);
    *(u64 *)(base + 0x18) = (s64)(px + g_gsPixelOffsetX[0] - 8) | ((s64)(py + g_gsPixelOffsetY[0] - 8) << 16) | z;
    *(u64 *)(base + 0x1a) = (s64)uB | vB;                                         /* v3 UV */
    px = FloatToInt(c3[0] * 16.0f); py = FloatToInt(c3[1] * 16.0f);
    base[0x1e] = 0;
    base[0x1f] = 0;
    *(u64 *)(base + 0x1c) = (s64)(px + g_gsPixelOffsetX[0] - 8) | ((s64)(py + g_gsPixelOffsetY[0] - 8) << 16) | z;
    g_frameDmaCursor[0] = base + 0x20;
}
#endif

extern u8 D_1AC930[]; /* prebuilt GIFtag template (16 bytes) */

/** func_0027EFA0 — append a 4-vertex textured-primitive GS packet to the frame
 *  DMA chain: a DMATAG (cnt, 9 qwords) + the prebuilt GIFtag at D_1AC930, then a
 *  leading control qword (mode ? 5 : 0, prim) and a 0x154 tag, followed by four
 *  vertices each packed as [st, uv, xyz2] (the two attribute arrays are s32[4]
 *  read stride-4; positions are u64[4] XYZ2). Advances g_frameDmaCursor by 0xA0.
 *  (st/uv naming inferred from the GS vertex layout.) */
/* TODO(match) t493: sdk29 41.06% / engine96 61.38% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): GPREL-FORM (first differing
 * insn: ROM `lui v1,0x0  [HI16 0x001B2228]` vs built `lui t4,0x0  [HI16 0x001B2228]`). Levers:
 * cc1-small/absolute globals model RUN: 39.74% (sdk29); -fno-strict-aliasing MEASURED (flag not
 * landed): 66.38% sdk29; engine96 with sched1 MEASURED (flag not landed): 60.35%. */
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
/* TODO(match) t493: sdk29 31.71% / engine96 53.31% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): GPREL-FORM (first differing
 * insn: ROM `lui v1,0x0  [HI16 0x001B2228]` vs built `lui t1,0x0  [HI16 0x001B2228]`). Levers:
 * cc1-small/absolute globals model RUN: 35.83% (sdk29); -fno-strict-aliasing MEASURED (flag not
 * landed): 56.88% sdk29; engine96 with sched1 MEASURED (flag not landed): 41.69%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F0A8);
#else
void func_0027F0A8(const u64 *corners, u64 tex0) {
    u8 *p = (u8 *)g_frameDmaCursor[0];
    const s32 *coords = (const s32 *)(unsigned long)tex0; /* arg1 is used as a pointer */

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
/* TODO(match) t493: sdk29 74.79% / engine96 57.72% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SCHED+REGNUM (first differing
 * insn: ROM `lw v0,0(v0)  [LO16 0x001A7354]` vs built `lui v1,0x0  [HI16 0x001A7350]`). Levers:
 * sibcall guard RUN: sdk29 64.72% / engine96 57.72%; cc1-small/absolute globals model RUN: 76.33%
 * (sdk29); engine96 with sched1 MEASURED (flag not landed): 50.44%.
 * Task #946 (cc1 2.9 probes, match.sh): non-volatile dataflow barriers `__asm__("" : "+r"(v))`
 * take t889's 6-row floor (3,888 barrier-free variants, NOTE #8245) to 2 rows. The barriers
 * go on the final vx1 / vx2 / vy2 and on y1*16 + offY (before the -8). Coordinates are computed
 * in the order x1, y1, x2, y2, then the corners in index order (body in task #946's NOTE).
 * Remaining is 1 adjacent swap: the ROM ORs vz into corner 0 before starting corner 1 (x2|vy1),
 * cc1 the reverse. The 2-row form came from 6,144 barrier-placement variants. A further 2,304
 * corner-order / OR-form / barrier variants on top of it did not move it, and volatile barriers
 * between the corner statements scored worse (11+ rows). */
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

/**
 * func_0027F208 — draw a beveled rectangle border in a single flat color.
 *
 * Emits seven scissored-rect fills (func_0027E4D0) forming a 3-step bevel down
 * each vertical edge of the [y0,y1] x [x0,x1] box: the interior fill plus, on
 * the left (x0-2..x0, x0-3..x0-2, x0-4..x0-3) and right (x1..x1+2, x1+2..x1+3,
 * x1+3..x1+4) sides, three progressively-inset strips whose y-range shrinks by
 * 1/2/4. The color is packed as (colorHi<<24) | colorLo.
 */
/* TODO(match) t493: sdk29 94.06% / engine96 57.77% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `addiu sp,sp,-96` vs built `addiu sp,sp,-176`). Levers: sibcall guard RUN: sdk29 88.00% /
 * engine96 61.05%; engine96 with sched1 MEASURED (flag not landed): 52.05%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F208);
#else
void func_0027F208(s32 y0, s32 y1, s32 x0, s32 x1, s32 colorHi, s32 colorLo) {
    s32 color = (colorHi << 24) | colorLo;

    func_0027E4D0(y0,     y1,     x0,     x1,     color);
    func_0027E4D0(y0 + 1, y1 - 1, x0 - 2, x0,     color);
    func_0027E4D0(y0 + 2, y1 - 2, x0 - 3, x0 - 2, color);
    func_0027E4D0(y0 + 4, y1 - 4, x0 - 4, x0 - 3, color);
    func_0027E4D0(y0 + 1, y1 - 1, x1,     x1 + 2, color);
    func_0027E4D0(y0 + 2, y1 - 2, x1 + 2, x1 + 3, color);
    func_0027E4D0(y0 + 4, y1 - 4, x1 + 3, x1 + 4, color);
}
#endif

/* func_0027F348 (DrawShadowedBoxOutline) - draws a drop-shadow-style box frame around
 * the rect y=[y0,y1] x=[x0,x1]: first a solid fill of the rect itself (color's alpha
 * byte preserved, low bits forced to 4), then eight thin offset bars (each a 2-px-wide
 * edge/corner segment nudged by +/-1..5) forming the outlined + shadowed border. All
 * nine draws go through the screen-rect-fill primitive func_0027E4D0.
 *
 * UN-PARK of a premature #70: the park cited "vertex/packet build + func_0027E4D0 sig
 * unconfident", but there is NO vertex build here (that lives inside func_0027E4D0,
 * declared line 1363 as (y0,y1,x0,x1,color)) - this body is just nine calls with plain
 * int offsets. Ghidra dropped the first (fill) call's args; recovered from the .s:
 * func_0027E4D0(y0,y1,x0,x1,(color & 0xFF000000) | 4). Engine region - faithful #else. */
/* TODO(match) t493: sdk29 56.19% / engine96 38.44% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `addiu sp,sp,-96` vs built `addiu sp,sp,-176`). Levers: sibcall guard RUN: sdk29 58.12% /
 * engine96 45.98%; engine96 with sched1 MEASURED (flag not landed): 42.05%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F348);
#else
void func_0027F348(s32 y0, s32 y1, s32 x0, s32 x1, u64 color) {
    func_0027E4D0(y0, y1, x0, x1, (color & 0xFF000000) | 4);
    func_0027E4D0(y0 - 1, y0 + 1, x0 + 3, x1 + 5, color);
    func_0027E4D0(y0 - 3, y0 - 5, x0 - 1, x1 - 3, color);
    func_0027E4D0(y0 - 5, y1 - 3, x0 - 1, x0 + 1, color);
    func_0027E4D0(y0 + 3, y1 + 1, x0 - 3, x0 - 5, color);
    func_0027E4D0(y1 - 1, y1 + 1, x0 - 3, x1 - 3, color);
    func_0027E4D0(y1 + 3, y1 + 5, x0 + 3, x1 + 1, color);
    func_0027E4D0(y0 + 3, y1 + 3, x1 - 1, x1 + 1, color);
    func_0027E4D0(y0 + 1, y1 - 3, x1 + 3, x1 + 5, color);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F4D0);

/* func_0027F4D8 (DrawTexturedRingSegments) — draw a textured ring/arc as 16 gouraud quads
 * (func_0027EFA0) sweeping from startAngle to endAngle about (centerX,centerY), between inner
 * and outer radius, with the given UI texture. Each quad spans one angular step
 * (step = (endAngle-startAngle)/segCount) between two edges; ALWAYS 16 segments are emitted
 * (the count is fixed — segCount only sizes the step). Per edge it takes sin/cos (SinfVu0/CosfVu0)
 * and packs the 4 corners (edgeA/edgeB × inner/outer) into GS XYZ vertices in the 12.4-style
 * layout: x = FloatToInt(sin*r)+centerX, y = (FloatToInt(cos*r)+centerY)<<16, plus the constant
 * z/fog field 0xfffff000000000. Angles are advanced with WrapAnglePiSum (seeded +pi/2).
 *
 * UN-PARK of a provably-false #70: the park's cited blocker was "needs the rotation + GS format
 * traced" — done, .s-vs-Ghidra. Engine region — faithful #else. FAITHFULNESS NOTES: the vertex
 * pack is built in s64 (native `long` is 32-bit and would overflow the <<16 + z-field); the
 * WrapAnglePiSum seed uses the EXACT bit pattern 0x3FC90FDC (a decimal pi/2 literal can round to
 * a different ULP). FORMER-PARK (silent-render class): dual-gate (tester oracle) + d2 heads-up. */
/* TODO(match) t493: sdk29 48.06% / engine96 26.34% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-192` vs built `addiu sp,sp,-240`). Levers: engine96 with sched1 MEASURED
 * (flag not landed): 27.99%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027F4D8);
#else
extern f32 SinfVu0(f32 x);                 /* func_00283B30 */
extern f32 CosfVu0(f32 x);                 /* func_00283B48 */
extern f32 WrapAnglePiSum(f32 a, f32 b);   /* 0x284548 */
extern s32 FloatToInt(f32 x);              /* declared file-scope below, used before it here */
extern u64 GetUiTextureTex0(s32 slot);     /* declared file-scope below, used before it here */

void func_0027F4D8(f32 startAngle, f32 endAngle, s32 centerX, s32 centerY,
                   s32 innerR, s32 outerR, const s32 *st, const s32 *uv,
                   u64 texId, s32 segCount) {
    const union { u32 u; f32 f; } kHalfPi = { 0x3FC90FDC };
    u64 tex0 = GetUiTextureTex0(texId);
    f32 step = (endAngle - startAngle) / (f32)segCount;
    f32 inner = (f32)innerR;
    f32 outer = (f32)outerR;
    f32 angleA = WrapAnglePiSum(startAngle, kHalfPi.f);
    f32 angleB = WrapAnglePiSum(angleA, step);
    s32 i;

    for (i = 15; i >= 0; i--) {
        f32 sinA = SinfVu0(angleA);
        f32 cosA = CosfVu0(angleA);
        f32 sinB, cosB;
        u64 verts[4];

        angleA = angleB;
        sinB = SinfVu0(angleB);
        cosB = CosfVu0(angleB);

        verts[0] = (s64)(FloatToInt(sinA * inner) + centerX)
                 + ((s64)(FloatToInt(cosA * inner) + centerY) << 16) + 0xfffff000000000LL;
        verts[1] = (s64)(FloatToInt(sinA * outer) + centerX)
                 + ((s64)(FloatToInt(cosA * outer) + centerY) << 16) + 0xfffff000000000LL;
        verts[2] = (s64)(FloatToInt(sinB * inner) + centerX)
                 + ((s64)(FloatToInt(cosB * inner) + centerY) << 16) + 0xfffff000000000LL;
        verts[3] = (s64)(FloatToInt(sinB * outer) + centerX)
                 + ((s64)(FloatToInt(cosB * outer) + centerY) << 16) + 0xfffff000000000LL;

        func_0027EFA0(verts, uv, st, tex0, 1);
        angleB = WrapAnglePiSum(angleA, step);
    }
}
#endif

/** func_0027F790 (EnableInlineColorCodes) — enable the fixed-font text
 *  drawers' inline colour-code escapes (control bytes 8..15 select a palette
 *  entry) and return 1. No params. The flag is the gp-small
 *  g_textColorCodeEnabled (0x1B15E8), a global of its own — the tree used to
 *  spell it g_blobShadowCount+0x2C and call it a blob-shadow enable, which the
 *  ROM contradicts (FACT #5649); as its own symbol the store is the one-insn
 *  gp-relative form the original packs into the jr delay slot.
 *  MATCHED (task #493): 100.00% unit objdiff, sdk29 arm. */
s32 func_0027F790(void) {
    s32 enabled = 1;
    /* Without the barrier cc1 2.9 materialises the constant twice (one register
     * for the store, one for the return); the original uses a single one. */
    __asm__ __volatile__("");
    g_textColorCodeEnabled = enabled;
    return enabled;
}

/** func_0027F7A0 (DisableInlineColorCodes) — clear the inline colour-code
 *  enable flag (see func_0027F790). No params, no return.
 *  MATCHED (task #493): 100.00% unit objdiff, sdk29 arm. */
void func_0027F7A0(void) {
    g_textColorCodeEnabled = 0;
}

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
/* TODO(match) t493: sdk29 91.88% / engine96 77.38% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-64` vs built `addiu sp,sp,-96`). */
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

/* DrawFixedFontString(x, y, color, str, maxLen, tex0, glyphTable) — draw up to maxLen chars of a
 * string in the fixed-cell font, emitting one DrawGlyphQuad per glyph. Each char indexes a 4-byte
 * metric record {u, v, yofs(s8), advance(s8)} at glyphTable + char*4. Control codes 8..15 are color-
 * escapes: when g_blobShadowCount+0x2C is set they swap the active color's low 24 bits from an 8-entry
 * escape table (D_1A89B0[char-8]); entry [0] is latched to the passed color at entry (unless
 * g_blobShadowCount+0x30 is set). Printable chars (advance != 0): chars in 0x80..0xA7 additionally draw
 * an overlay glyph (code+0x40); chars < 0x20 draw a 24x16 cell in a grayscale (RGB-averaged) color;
 * chars > 0x20 draw a 16x16 cell in the active color; space (0x20) draws nothing. The pen advances by
 * the metric's advance byte. Sets GS TEST_1 (0x33001) once up front.
 *
 * Engine region — faithful #else, .s-traced (Ghidra's decompile OMITS the color-escape state machine).
 * Plain 10-arg DrawGlyphQuad calls: param9=active color (grayscale for the <0x20 case), param10=tex0
 * (the two are pushed as stack args; NO Option-D). */
/* TODO(match) t493: sdk29 37.17% / engine96 31.48% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-96` vs built `addiu sp,sp,-192`). Levers: engine96 with sched1 MEASURED
 * (flag not landed): 34.88%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", DrawFixedFontString);
#else
extern s32 D_1A89B0[];   /* 8-entry color-escape table; [0] = latched default color */

void DrawFixedFontString(s32 x, s32 y, s32 color, s32 str, s32 maxLen,
                         u64 tex0, u8 *glyphTable) {
    const u8 *s = (const u8 *)str;   /* str param carries the pointer as an integer */
    s32 i;

    if (g_bInWrappedTextBox == 0) {
        D_1A89B0[0] = color;
    }
    AppendGsRegPacket(0x47, 0x33001);

    if (maxLen == 0 || *s == 0) {
        return;
    }
    for (i = 0; ; ) {
        u8 c = *s;
        if ((u8)(c - 8) < 8) {
            /* control code 8..15: swap the active color's low 24 bits from the escape table */
            if (g_textColorCodeEnabled != 0) {
                color = (color & 0xFF000000) | (D_1A89B0[c - 8] & 0x00FFFFFF);
            }
        } else {
            const u8 *rec = glyphTable + c * 4;
            if ((s8)rec[3] != 0) {                     /* advance != 0 -> printable */
                if ((u8)(c + 0x80) < 0x28) {           /* 0x80..0xA7: overlay glyph (code+0x40) */
                    const u8 *orec = glyphTable + (c + 0x40) * 4;
                    DrawGlyphQuad(x + (s8)orec[3], y + (s8)orec[2], 0x10, 0x10,
                                  orec[0], orec[1], 0x10, 0x10, color, tex0);
                }
                if (c < 0x20) {                        /* control-range: grayscale (avg RGB) 24x16 */
                    s32 avg = ((color & 0xFF) + ((color >> 8) & 0xFF) + ((color >> 16) & 0xFF)) / 3;
                    s32 gray = (color & 0xFF000000) | (avg << 16) | (avg << 8) | avg;
                    DrawGlyphQuad(x, y + (s8)rec[2], 0x18, 0x10, rec[0], rec[1], 0x18, 0x10, gray, tex0);
                } else if (c > 0x20) {                 /* printable 16x16 (space draws nothing) */
                    DrawGlyphQuad(x, y + (s8)rec[2], 0x10, 0x10, rec[0], rec[1], 0x10, 0x10, color, tex0);
                }
                x += (s8)rec[3];                       /* pen advance */
            }
        }
        i++;
        if (i == maxLen) break;
        s++;
        if (*s == 0) break;
    }
}
#endif

extern u64 GetUiTextureTex0(s32 slot);
extern void DrawFixedFontString(s32 a, s32 b, s32 c, s32 d, s32 e, u64 tex0, u8 *glyphTable);

/* TODO(match) t493: sdk29 43.45% / engine96 44.52% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): SIBCALL (first differing
 * insn: ROM `sd s1,8(sp)` vs built `daddu s0,a0,zero`). Levers: sibcall guard RUN: sdk29 71.55% /
 * engine96 84.19%; engine96 with sched1 MEASURED (flag not landed): 43.13%. */
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

/* TODO(match) t493: sdk29 43.45% / engine96 44.52% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): SIBCALL (first differing
 * insn: ROM `sd s1,8(sp)` vs built `daddu s0,a0,zero`). Levers: sibcall guard RUN: sdk29 71.55% /
 * engine96 84.19%; engine96 with sched1 MEASURED (flag not landed): 43.13%. */
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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027FCA8);

/* func_0027FCB0 (DrawGlyphRun) — the scaled/float twin of DrawFixedFontString: draws up to `c`
 * chars of string `b` at (f1,f2) scaled by f3, in color `a`, one DrawTexturedQuad2d per glyph. Per
 * char: 4-byte metric {u,v,yofs(s8),advance(s8)} at glyphTable+char*4; control codes 8..15 are
 * color-escapes (swap active color low-24 from D_1A89B0[c-8] when g_blobShadowCount+0x2C set; entry
 * [0] latched to `a` unless +0x30 set); 0x80..0xA7 also draw an overlay glyph (code+0x40); <0x20
 * draw a scaled 24x16 cell in a grayscale (RGB-averaged) color; >0x20 a scaled 16x16 in the active
 * color; space draws nothing; pen advances by IntToFloat(advance)*scale. Sets GS TEST_1 once.
 * The file's L2245 decl uses generic a/b/c/f1/f2/f3 for the same registers Ghidra names
 * color/str/count/x/y/scale: a=color, b=str(ptr-as-int), c=count, f1=x, f2=y, f3=scale.
 *
 * Engine region — faithful #else, whole-.s traced. Metric yofs/advance are SIGNED (lb→IntToFloat),
 * u/v unsigned (lbu); grayscale = (color&0xFF000000) + (avgRGB)*0x10101. Calls the in-file
 * DrawTexturedQuad2d (10-arg, 4-color array + tex0). */
/* TODO(match) t493: sdk29 53.57% / engine96 51.10% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-144` vs built `addiu sp,sp,-240`). Levers: engine96 with sched1 MEASURED
 * (flag not landed): 46.16%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_0027FCB0);
#else
extern s32 D_1A89B0[];   /* 8-entry color-escape table; [0] = latched color */

void func_0027FCB0(s32 a, s32 b, s32 c, u64 tex0, u8 *glyphTable, f32 f1, f32 f2, f32 f3) {
    s32 color = a;
    const u8 *s = (const u8 *)b;
    s32 count = c;
    f32 x = f1, y = f2, scale = f3;
    f32 sz16 = scale * 16.0f;
    s32 i;

    if (g_bInWrappedTextBox == 0) {
        D_1A89B0[0] = color;
    }
    AppendGsRegPacket(0x47, 0x33001);

    if (count == 0 || *s == 0) {
        return;
    }
    for (i = 0; ; ) {
        u8 ch = *s;
        if ((u8)(ch - 8) < 8) {
            if (g_textColorCodeEnabled != 0) {
                color = (color & 0xFF000000) | (D_1A89B0[ch - 8] & 0x00FFFFFF);
            }
        } else {
            const u8 *rec = glyphTable + ch * 4;
            if ((s8)rec[3] != 0) {
                f32 yofs = IntToFloat((s8)rec[2]) * scale;
                u32 col4[4];
                if ((u8)(ch + 0x80) < 0x28) {
                    const u8 *orec = glyphTable + (ch + 0x40) * 4;
                    f32 xofs = IntToFloat((s8)orec[3]) * scale;
                    f32 oy   = IntToFloat((s8)orec[2]);
                    col4[0] = col4[1] = col4[2] = col4[3] = color;
                    DrawTexturedQuad2d(x + xofs, y + oy * scale, sz16, sz16,
                                       orec[0], orec[1], 0x10, 0x10, col4, tex0);
                }
                if (ch < 0x20) {
                    s32 avg = ((color & 0xFF) + ((color >> 8) & 0xFF) + ((color >> 0x10) & 0xFF)) / 3;
                    u32 gray = (color & 0xFF000000) + avg * 0x10101;
                    col4[0] = col4[1] = col4[2] = col4[3] = gray;
                    DrawTexturedQuad2d(x, y + yofs, scale * 24.0f, scale * 16.0f,
                                       rec[0], rec[1], 0x18, 0x10, col4, tex0);
                } else if (ch > 0x20) {
                    col4[0] = col4[1] = col4[2] = col4[3] = color;
                    DrawTexturedQuad2d(x, y + yofs, sz16, sz16,
                                       rec[0], rec[1], 0x10, 0x10, col4, tex0);
                }
                x = x + IntToFloat((s8)rec[3]) * scale;
            }
        }
        i++;
        if (i == count) break;
        s++;
        if (*s == 0) break;
    }
}
#endif

extern void func_0027FCB0(s32 a, s32 b, s32 c, u64 tex0, u8 *glyphTable, f32 f1, f32 f2, f32 f3); /* scaled/positioned font draw */

/* TODO(match) t493: sdk29 36.60% / engine96 30.94% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `addiu sp,sp,-64` vs built `addiu sp,sp,-96`). Levers: sibcall guard RUN: sdk29 63.29% /
 * engine96 64.20%; engine96 with sched1 MEASURED (flag not landed): 30.97%. */
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

/* func_00280080: 0x10 bytes of dead epilogue pad (addiu $sp,0x40; nop x2) carved
 * off the real body func_00280090 in task #472 (was PARKED #70 pending exactly
 * this re-split). func_00280090 body logic (right-justified fixed-font string
 * draw): w = func_0027F7F8(d, e); tex0 = GetUiTextureTex0(1);
 * DrawFixedFontString(a - w, b, c, d, e, (s32)tex0, D_263B10). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280080);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280090);

/** func_00280120 — draw a right-aligned debug-font string. Measures the string's
 *  rendered width (func_0027F818(str, maxChars)), shifts the anchor x left by that
 *  width, resolves the UI font texture (GetUiTextureTex0 slot 2), and forwards to
 *  DrawFixedFontString with the debug glyph table. (DrawFixedFontString's d/e
 *  params carry the string/maxChars — its committed decl types them s32.) */
/* TODO(match) t493: sdk29 50.60% / engine96 23.00% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `addiu sp,sp,-48` vs built `addiu sp,sp,-112`). Levers: engine96 with sched1 MEASURED (flag
 * not landed): 22.20%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280120);
#else
void func_00280120(s32 x, s32 arg1, s32 arg2, const char *str, s32 maxChars) {
    s32 width = func_0027F818(str, maxChars);
    u64 tex0 = GetUiTextureTex0(2);
    DrawFixedFontString(x - width, arg1, arg2, (s32)(unsigned long)str, maxChars, tex0,
                        g_debugFontGlyphTable);
}
#endif

/* func_002801B0: 8 bytes of dead epilogue pad (addiu $sp,0x30; nop) carved off
 * the real body func_002801B8 in task #472 (twin of func_00280080, was PARKED
 * #70). func_002801B8: a right-justified fixed-font string draw of the same
 * func_0027F7F8 / GetUiTextureTex0 / DrawFixedFontString shape as func_00280090. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002801B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002801B8);

/** func_00280250 — draw a horizontally-centered debug-font string. Measures the
 *  rendered width (func_0027F818), centers the anchor (x - width/2), draws via
 *  DrawFixedFontString (UI font slot 2, debug glyph table), and returns the
 *  centered x. */
/* TODO(match) t493: sdk29 89.68% / engine96 86.00% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SCHED+REGNUM (first differing
 * insn: ROM `addiu sp,sp,-48` vs built `addiu sp,sp,-96`). Levers: engine96 with sched1 MEASURED
 * (flag not landed): 86.38%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280250);
#else
s32 func_00280250(s32 x, s32 arg1, s32 arg2, const char *str, s32 maxChars) {
    s32 width = func_0027F818(str, maxChars);
    s32 cx = x - (width >> 1);
    u64 tex0 = GetUiTextureTex0(2);
    DrawFixedFontString(cx, arg1, arg2, (s32)(unsigned long)str, maxChars, tex0,
                        g_debugFontGlyphTable);
    return cx;
}
#endif

/** func_002802E8 — draw a horizontally-centered string in the alternate UI font
 *  (glyph table D_264250, UI texture slot 3). Same centering as func_00280250:
 *  measure width (func_0027F838), center (x - width/2), draw, return centered x. */
/* TODO(match) t493: sdk29 89.68% / engine96 86.00% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SCHED+REGNUM (first differing
 * insn: ROM `addiu sp,sp,-48` vs built `addiu sp,sp,-96`). Levers: engine96 with sched1 MEASURED
 * (flag not landed): 86.38%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002802E8);
#else
s32 func_002802E8(s32 x, s32 arg1, s32 arg2, const char *str, s32 maxChars) {
    s32 width = func_0027F838(str, maxChars);
    s32 cx = x - (width >> 1);
    u64 tex0 = GetUiTextureTex0(3);
    DrawFixedFontString(cx, arg1, arg2, (s32)(unsigned long)str, maxChars, tex0, D_264250);
    return cx;
}
#endif

/* func_00280380 - draw a scaled glyph run horizontally centered on x=a. Measures the
 * run's scaled pixel advance (func_0027F900 with the same scale), shifts the origin
 * left by half that advance to center it, resolves the UI texture (GetUiTextureTex0
 * slot 1), and forwards to the positioned font-draw func_0027FCB0 with the D_263B10
 * glyph metrics. The centered variant of func_0027FFF0.
 *
 * Engine region (ee-gcc 2.96) - faithful #else. */
/* TODO(match) t493: sdk29 40.65% / engine96 11.90% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `addiu sp,sp,-80` vs built `addiu sp,sp,-128`). Levers: engine96 with sched1 MEASURED (flag
 * not landed): 12.25%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280380);
#else
void func_00280380(s32 a, s32 b, s32 c, const char *str, s32 maxChars, f32 scale) {
    s32 advance = func_0027F900(str, maxChars, scale);
    u64 tex0 = GetUiTextureTex0(1);
    func_0027FCB0(c, (s32)str, maxChars, tex0, D_263B10,
                  (f32)(a - (advance >> 1)), (f32)b, scale);
}
#endif

extern f32 func_002804C0(f32 inputScale, const char *str, s32 maxChars, s32 count); /* text auto-scale (below) */
extern void func_00280380(s32 a, s32 b, s32 c, const char *str, s32 maxChars, f32 scale); /* scaled text draw */

/* TODO(match) t493: sdk29 76.13% / engine96 50.65% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `addiu sp,sp,-48` vs built `addiu sp,sp,-96`). Levers: sibcall guard RUN: sdk29 89.26% /
 * engine96 84.35%; engine96 with sched1 MEASURED (flag not landed): 50.61%. */
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

/* TODO(match) t493: sdk29 73.86% / engine96 63.61% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-32` vs built `addiu sp,sp,-48`). Levers: engine96 with sched1 MEASURED
 * (flag not landed): 74.58%. */
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

/**
 * DrawTextBoxDefault - the unscaled text-box entry: forward to the scaled core
 * func_00280550 with scale 1.0f.
 *
 *   a..e        the five int slots (layout, rgba, str, len, tex0), passed through
 *   glyphTable  the glyph-metrics table (the func_00280B48 path passes &D_263B10)
 *   ->          nothing
 *
 * The six args arrive in $4..$9 and are forwarded untouched in the same
 * registers; the only work is loading 1.0f into $f12. The portable body had once
 * been 5-arg and silently DROPPED the glyph-metrics table (sig recovered
 * 2026-07-06, fable, promo-d3 @4cb6be7).
 *
 * Non-obvious: the original keeps a full call+return frame (`jal`, then restore
 * $ra) rather than tail-jumping to func_00280550. cc1 would sibling-call-optimise
 * the lone tail call to `j func_00280550`; the empty volatile asm after the call
 * is the barrier that keeps the call a call.
 */
void func_00280B20(s32 a, s32 b, s32 c, s32 d, s32 e, u8 *glyphTable) {
    func_00280550(a, b, c, d, e, glyphTable, 1.0f);
    __asm__ __volatile__("");
}

/* func_00280B48 - draw a string with the default UI font: resolve the font
 * page's GS TEX0 (GetUiTextureTex0 slot 1) and forward the four caller args
 * plus that tex0 and the D_263B10 glyph-metrics table to the text core
 * func_00280B20 (SIX args). Sig recovered 2026-07-06 (fable, promo-d3
 * @4cb6be7). Matching arm stays INCLUDE_ASM (byte-exact is walled by the
 * tier-wide 8-packed-callee-save frame fingerprint); the #else supplies the
 * portable body that passes the glyph table down the chain. */
/* TODO(match) t493: sdk29 28.56% / engine96 55.48% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): SIBCALL (first differing
 * insn: ROM `sd s1,8(sp)` vs built `daddu s0,a0,zero`). Levers: sibcall guard RUN: sdk29 76.70% /
 * engine96 76.30%; engine96 with sched1 MEASURED (flag not landed): 54.44%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280B48);
#else
void func_00280B48(s32 a, s32 b, s32 c, s32 d) {
    u64 tex0 = GetUiTextureTex0(1);
    func_00280B20(a, b, c, d, (s32)tex0, D_263B10);
}
#endif

/* func_00280BB8 - draw a string with the debug font (twin of func_00280B48): resolve
 * the debug-font page's GS TEX0 (GetUiTextureTex0 slot 2) and forward the four caller
 * args plus that tex0 and the g_debugFontGlyphTable glyph-metrics table to the text core
 * func_00280B20. Matching arm stays INCLUDE_ASM (byte-exact walled by the tier-wide
 * 8-packed-callee-save frame fingerprint); the #else supplies the portable body. */
/* TODO(match) t493: sdk29 28.56% / engine96 55.48% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): SIBCALL (first differing
 * insn: ROM `sd s1,8(sp)` vs built `daddu s0,a0,zero`). Levers: sibcall guard RUN: sdk29 76.70% /
 * engine96 76.30%; engine96 with sched1 MEASURED (flag not landed): 54.44%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280BB8);
#else
void func_00280BB8(s32 a, s32 b, s32 c, s32 d) {
    u64 tex0 = GetUiTextureTex0(2);
    func_00280B20(a, b, c, d, (s32)tex0, g_debugFontGlyphTable);
}
#endif

/* func_00280C28 - draw a string with a third UI font page (twin of func_00280B48/
 * func_00280BB8): resolve the GS TEX0 (GetUiTextureTex0 slot 3) and forward the four
 * caller args plus that tex0 and the D_264250 glyph-metrics table to the text core
 * func_00280B20. Matching arm stays INCLUDE_ASM (8-packed-callee-save frame wall). */
/* TODO(match) t493: sdk29 28.56% / engine96 55.48% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): SIBCALL (first differing
 * insn: ROM `sd s1,8(sp)` vs built `daddu s0,a0,zero`). Levers: sibcall guard RUN: sdk29 76.70% /
 * engine96 76.30%; engine96 with sched1 MEASURED (flag not landed): 54.44%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280C28);
#else
void func_00280C28(s32 a, s32 b, s32 c, s32 d) {
    u64 tex0 = GetUiTextureTex0(3);
    func_00280B20(a, b, c, d, (s32)tex0, D_264250);
}
#endif

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280CD0);

/* AppendVu1SphereMapContext — build the VU1 sphere-map/reflection render context and append it
 * to the frame DMA chain. Lazily uploads the sphere-map microcode (D_10ED30) the first time the
 * active VU1 program isn't 8, then emits an UNPACK packet: builds a 4x4 view-translation matrix
 * (func_00283D68 sets the basis at scale 1024, its translation row = -1024 * g_cameraPos via
 * Vec4ScaleVu0, w=1), multiplies both camera matrices (g_cameraPos-0x100 and -0x80) by it into the
 * packet (each with a Z-bias D_1A86F0 added to one element), copies the GS drawing-context rows from
 * g_sceneActorMobys+0x674+0x190.., self-computes the DMA/VIF tag QWC ((packetBytes>>4)-1) into the
 * opening tag, advances g_frameDmaCursor, and appends the GS state-ref packet (func_002FD700).
 *
 * UN-PARK of a provably-false #70: the park's blocker ("needs the matrix + VIF packet format traced")
 * is resolved — fully traced .s-vs-Ghidra. It's a self-describing DMA packet (the QWC is computed from
 * the packet size, no opaque consumer of an inferred struct), so faithfully transcribable. Engine
 * region — faithful #else. Consts 1024/-1024/1.0 are exactly representable (no ULP concern). FORMER-
 * PARK (VU1/VIF silent-render class): dual-gate (tester oracle) + d2 heads-up. */
/* TODO(match) t493: sdk29 47.89% / engine96 50.38% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): MACRO-AT (first differing
 * insn: ROM `sd s0,64(sp)` vs built `daddu a0,sp,zero`). Levers: sibcall guard RUN: sdk29 43.11% /
 * engine96 50.47%; cc1-small/absolute globals model RUN: 51.98% (engine96); engine96 with sched1
 * MEASURED (flag not landed): 47.25%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", AppendVu1SphereMapContext);
#else
extern s32  g_activeVu1Program;
extern f32  D_1A86F0;                                 /* matrix-element Z-bias */
extern u16  D_10ED20;                                 /* VU1 sphere-map microcode header (first u16) */
extern u8   D_10ED30[];                               /* VU1 sphere-map microcode blob */
extern void func_00283D68(f32 scale, void *outMtx);   /* Matrix4x4SetTranslationVu0 */
extern void AppendVifCodeRefTag(void *blob, s32 hdr);
extern void func_002FD700(void);                      /* AppendGsStateRefPacket */

void AppendVu1SphereMapContext(void) {
    u8   mtx[64];   /* 4x4 view-translation matrix (basis + -1024*cameraPos translation row) */
    u32 *base;

    func_00283D68(1024.0f, mtx);
    Vec4ScaleVu0(mtx + 0x30, -1024.0f, g_cameraPos);
    *(f32 *)(mtx + 0x3C) = 1.0f;

    if (g_activeVu1Program != 8) {
        AppendVifCodeRefTag(D_10ED30, D_10ED20);
        g_activeVu1Program = 8;
    }

    base = g_frameDmaCursor[0];
    base[0] = 0x10000000;
    base[1] = 0;
    base[2] = 0x11000000;
    base[3] = 0x1000404;
    base[4] = 0;
    base[5] = 0;
    base[6] = 0;
    base[7] = 0x6C0C43A4;
    MatrixMultiplyVu0((f32 *)(base + 8), (f32 *)(g_cameraPos - 0x100), (f32 *)mtx);
    *(f32 *)(base + 0x16) = *(f32 *)(base + 0x16) + D_1A86F0;
    MatrixMultiplyVu0((f32 *)(base + 0x18), (f32 *)(g_cameraPos - 0x80), (f32 *)mtx);
    *(f32 *)(base + 0x26) = *(f32 *)(base + 0x26) + D_1A86F0;

    {
        u8 *src = g_sceneActorMobys + 0x674;
        base[0x28] = 0x8000;
        base[0x29] = 0x303EC000;
        base[0x2a] = 0x412;
        *(f32 *)(base + 0x2b) = *(f32 *)(src + 0x210);
        *(u64 *)(base + 0x2c) = *(u64 *)(src + 0x190);   /* lq/sq = full 16-byte qword copy */
        *(u64 *)(base + 0x2e) = *(u64 *)(src + 0x198);
        *(u64 *)(base + 0x30) = *(u64 *)(src + 0x1A0);   /* lq/sq = full 16-byte qword copy */
        *(u64 *)(base + 0x32) = *(u64 *)(src + 0x1A8);
        *(f32 *)(base + 0x34) = *(f32 *)(src + 0x22C);
        *(f32 *)(base + 0x35) = *(f32 *)(src + 0x228);
        base[0x36] = 0;
        base[0x37] = 0;
        base[0x38] = 0x3000000;
        base[0x39] = 0x20001D2;
        base[0x3a] = 0x15000000;
        base[0x3b] = 0;
    }

    base[0] |= (u32)((((u8 *)(base + 0x3c) - (u8 *)base) >> 4) - 1);
    g_frameDmaCursor[0] = base + 0x3c;
    func_002FD700();
}
#endif

/**
 * func_00280EC8 — configure the GS draw region / depth + alpha for a
 * (1<<zNearBits) x (1<<lo) render target.
 *
 * Sets the depth mapping (func_002859E0): in mode!=0 the z-scale comes from the
 * screen context (g_gsScreenContext+0x16E)<<13, else from the log2 formula
 * ((0x3FF000 - 4<<clamp(zNearBits+lo,16)) >> 13) << 13. Rebuilds the viewport
 * for the target dimensions (func_0027B858, with fixed 524288/255 params), then
 * emits the GS TEST (0x47, alpha ref 0x30000 when mode==0) and SCISSOR (0x42)
 * register packets.
 */
/* TODO(match) t493: sdk29 59.72% / engine96 57.27% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): SIBCALL (first differing insn:
 * ROM `addiu sp,sp,-48` vs built `addiu sp,sp,-80`). Levers: sibcall guard RUN: sdk29 78.40% /
 * engine96 61.18%; engine96 with sched1 MEASURED (flag not landed): 54.98%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00280EC8);
#else
void func_00280EC8(s32 zNearBits, s32 lo, s32 mode, f32 fa) {
    s32 zScale;

    if (mode != 0) {
        zScale = *(s16 *)(g_gsScreenContext + 0x16E) << 13;
    } else {
        s32 sum = zNearBits + lo;
        s32 shift = (sum < 17) ? sum : 16;
        zScale = ((0x3FF000 - (4 << shift)) >> 13) << 13;
    }
    func_002859E0(zNearBits, lo, zScale, 0);
    func_0027B858(1 << zNearBits, 1 << lo, fa, 0.0f, 524288.0f, 255.0f, 0.0f);
    AppendGsRegPacket(0x47, (mode != 0) ? 0 : 0x30000);
    AppendGsRegPacket(0x42, 0x0000008000000044ULL);
}
#endif

/** func_00280FB8 — queue the ctx1 draw-env packet, then recompute the GS
 *  screen geometry. No params, no return. The empty asm statement keeps cc1
 *  from sibcalling the final call (the original keeps the call frame).
 *  MATCHED (task #493): 100.00% unit objdiff, sdk29 arm. */
void func_00280FB8(void) {
    AppendDrawEnvContext1();
    RecomputeScreenViewportFromGsContext();
    __asm__ __volatile__("");
}

/** func_00280FE0 — per-frame reset of the offscreen-probe subsystem: zero the
 *  0x600-byte screen-grab/occlusion query descriptor table at 0x1B8990 and
 *  clear the pending query count at 0x1B15D8. No params, no return. The count
 *  is cleared through the cc1-small / assembler-absolute alias
 *  g_blobShadowState so the store is the one-insn `sw $0, sym+0x1C` macro that
 *  GNU as expands to `lui $at; sw` after the $ra restore, as in the original.
 *  MATCHED (task #493): 100.00% unit objdiff, sdk29 arm. */
void func_00280FE0(void) {
    FillMemory32(g_sceneActorMobys + 0x44, 0, 0x600);
    g_screenGrabQueryCount = 0;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/178E88", func_00281010);

/** func_00281020 — occlusion visibility ratio for a scene actor. The actor's
 *  record (g_sceneActorMobys+0x44, stride 0x30, indexed by `actorIdx`) holds the
 *  query rect w/h at +0x24/+0x26, a depth threshold at +0x14, and a result-float
 *  pointer at +0x10. Count how many of the w*h read-back pixels have their 24-bit
 *  depth above the threshold, then store the *un*-exceeded fraction
 *  (total - count)/total to the result float. */
/* TODO(match) t493: sdk29 62.50% / engine96 65.95% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (engine96): UNKNOWN-addiu (first
 * differing insn: ROM `addiu v1,zero,48` vs built `sll v0,a1,0x1`). Levers: engine96 with sched1
 * MEASURED (flag not landed): 67.08%. */
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

/**
 * func_002810C0 / ResolvePendingScreenGrabs — resolve the queued screen-grab
 * captures by GS local-to-local blitting each pending region into the two
 * inactive frame-arena halves, then feeding them to the occlusion-query
 * coverage pass. Produces the screen-grab-as-texture sources consumed by
 * GetUiTextureTex0 via negative ids. No-op when nothing is queued.
 *
 * When g_screenGrabQueryCount (g_blobShadowCount+0x1C) is nonzero it walks the
 * 0x20-entry descriptor table at g_sceneActorMobys+0x44 (0x1B8990, stride 0x30):
 * flag bit 2 (+0x1C & 4) marks a pending grab, the source rect is the four
 * shorts at +0x20/+0x22/+0x24/+0x26, and flag bit 0 marks a completed capture
 * whose count is then decremented (clearing bits 0 and 2 -> & 0xFFFFFFFA).
 *
 * Double-buffering: dst[0] and dst[1] point at the inactive frame-arena half
 * (g_frameArenaBase[1 - g_frameArenaFlip]) and that half + 0x40000 words. An
 * INIT pass (walking entries 0x1F..0 downward) blits every pending region into
 * dst[0] with a WaitGsPathsIdle after each, recording the highest pending index
 * in `lastPending`. Then, while a pending index remains, an OUTER loop ping-
 * pongs between the two halves: an INNER pass blits the still-pending entries
 * below the current index into dst[arenaIdx], ComputeOcclusionQueryCoverage
 * (func_00281020) then reads back the OTHER half dst[1 - arenaIdx], the
 * completed grab's count is decremented, and arenaIdx toggles for the next lap.
 *
 * Op-for-op-faithful #else transcription (verified against the frozen .s and
 * Ghidra). The two inner-loop head tests are ee-gcc `beql` LIKELY branches:
 * their delay-slot `iVar--` fires only when the branch is taken (flag bit 2
 * clear), while the not-taken (bit set) body path performs the decrement
 * explicitly at the tail — either way the counter decrements exactly once per
 * iteration, so a plain `do { if (bit2) { ...blit... } counter--; ptr -= 0x30;
 * } while (counter >= 0)` is faithful. The loop back-edges are non-likely
 * `bgez`, so their pointer-step delay slots always run. `lastPending` (iVar5)
 * captures the PRE-decrement index and is only assigned inside the bit-2 body. */
/* TODO(match) t493: sdk29 71.59% / engine96 31.64% (unit objdiff, objdiff_build.sh +
 * unit_report.sh, this #else body plain-promoted resp. MATCH_-guarded, screened together with
 * every other remaining arm). Residual on the better arm (sdk29): UNKNOWN-addiu (first differing
 * insn: ROM `addiu sp,sp,-96` vs built `addiu sp,sp,-176`). Levers: cc1-small/absolute globals
 * model RUN: 31.64% (engine96); engine96 with sched1 MEASURED (flag not landed): 62.72%. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/178E88", func_002810C0);
#else
extern u32  *g_frameArenaBase[];                        /* 0x1B2220 double-buffer half pair */
extern s32   g_frameArenaFlip;                          /* 0x1B2234 active-half index 0/1 */
extern void  func_002860B8(s16 x, s16 y, s16 w, s16 h,  /* GS local-to-local blit */
                           u32 *dst);
extern void  WaitGsPathsIdle(s32 a, s32 b);             /* GIF/VIF1/VU0 path-idle spin */

void func_002810C0(void) {
    u32 *dst[4];
    u8  *rec;
    s32  idx;
    s32  lastPending;
    s32  arenaIdx;

    if (g_screenGrabQueryCount != 0) {
        rec = g_sceneActorMobys + 0x44 + 0x5D0;         /* 0x1B8F60 = entry 0x1F */
        arenaIdx = 0;
        lastPending = -1;
        dst[0] = g_frameArenaBase[1 - g_frameArenaFlip];
        dst[1] = g_frameArenaBase[1 - g_frameArenaFlip] + 0x10000;

        /* INIT pass: blit every pending region into dst[0]. */
        idx = 0x1F;
        do {
            if ((*(u32 *)(rec + 0x1C) & 4) != 0) {
                func_002860B8(*(s16 *)(rec + 0x20), *(s16 *)(rec + 0x22),
                              *(s16 *)(rec + 0x24), *(s16 *)(rec + 0x26), dst[0]);
                WaitGsPathsIdle(0, 0);
                lastPending = idx;
            }
            idx = idx - 1;
            rec = rec - 0x30;
        } while (idx >= 0);

        if ((lastPending != -1) && (lastPending >= 0)) {
            arenaIdx = 1 - arenaIdx;                     /* -> 1 before the outer loop */
            do {
                s32 inner = lastPending - 1;
                s32 innerLast = -1;

                if (inner >= 0) {
                    rec = g_sceneActorMobys + 0x44 + inner * 0x30;
                    do {
                        if ((*(u32 *)(rec + 0x1C) & 4) != 0) {
                            func_002860B8(*(s16 *)(rec + 0x20), *(s16 *)(rec + 0x22),
                                          *(s16 *)(rec + 0x24), *(s16 *)(rec + 0x26),
                                          dst[arenaIdx]);
                            innerLast = inner;
                        }
                        inner = inner - 1;
                        rec = rec - 0x30;
                    } while (inner >= 0);
                }

                func_00281020(dst[1 - arenaIdx], lastPending);

                rec = g_sceneActorMobys + 0x44 + lastPending * 0x30;
                if ((*(u32 *)(rec + 0x1C) & 1) != 0) {
                    g_screenGrabQueryCount = g_screenGrabQueryCount - 1;
                    *(u32 *)(rec + 0x1C) = *(u32 *)(rec + 0x1C) & 0xFFFFFFFA;
                }

                if (innerLast >= 0) {
                    WaitGsPathsIdle(0, 0);
                }

                arenaIdx = 1 - arenaIdx;
                lastPending = innerLast;
            } while (lastPending >= 0);
        }
    }
}
#endif

/* func_002812A8 moved OUT of this unit by the 2026-09-15 re-split (task #314).
 * The old boundary put this unit's end 0x80 too high, severing the handwritten
 * routine at 0x2812A8 from the delay-slot nop of its own closing
 * `bnez $0,func_002812A8` at 0x281324. Both halves now live together in the
 * asm unit text/1812A8. */

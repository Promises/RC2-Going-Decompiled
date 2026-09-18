#include "common.h"

/*
 * text/1D5488 — EU (SCES_516.07) twin of USA text/1D54C0 — front-end /
 * pause-menu screens band, part B. EU file-offset / address-shifted sibling of
 * the USA unit (USA 0x2D5540.. -> EU 0x2D5508..). Ported per Phase B: the C
 * bodies are region-agnostic; only the extern global/function NAMES are
 * retargeted to their EU addresses (objdiff masks the gp/reloc deltas).
 *
 * Built at -O2 -G8 -fno-gcse (later SN cc1 TU model — see the USA part-B header
 * for the -G8 extern-sizing rules). The same per-function walls the USA unit
 * leaves as INCLUDE_ASM stay INCLUDE_ASM here.
 */

/* 2D draw-batch begin/end fence used by every menu draw function.
 * USA Begin2dDrawBatch -> EU func_0027CA28; End2dDrawBatch -> EU func_0027CB48. */
extern void func_0027CA28(s32 mode); /* Begin2dDrawBatch (EU) */
extern void func_0027CB48(void);     /* End2dDrawBatch (EU) */

/* Per-screen draw helpers in the preceding text/1A00F0 asm band (EU names). */
extern void func_0029D870(void); /* USA func_0029DD10 */
extern void func_0029D4B8(void); /* USA func_0029D958 */
extern void func_0029D448(void); /* USA func_0029D8E8 */
extern void func_0029CD08(void); /* USA func_0029D1A8 */
extern void func_0029CD78(void); /* USA func_0029D218 */
extern void func_0029CD38(s32 buttons); /* USA func_0029D1D8 */

/* Forwarding-wrapper target (EU name; USA func_002DF1B8). */
extern void func_002DF178(s32 mode);

/* 2D-batch sub-rect submit helper (EU func_002896A0; USA func_002897B0). */
extern void func_002896A0(s32 x0, s32 x1, s32 y0, s32 y1);

/* Front-end / pause screen-action dispatcher (op, arg, out-flag ptr). */
extern s32 MenuScreenDoAction(s32 op, s32 arg, void *outFlag);

/* A menu command record: opcode at +2 (s16), arg at +4 (s32). */
typedef struct MenuCmd {
    u8  _pad0[2];
    s16 op;     /* 0x2 */
    s32 arg;    /* 0x4 */
} MenuCmd;

/* Sound-channel volume snapshot/table (EU). g_sndChannelVolumes is pinned at the
 * EU address 0x188728 (USA 0x1886A8) so its slot-2 store folds into a single
 * %hi/%lo reloc (g_sndChannelVolumes+0x8). Sized >=16 to keep the address out of
 * cc1 small data, matching the USA twin. */
__asm__(".extern D_1A7C28, 16");
__asm__(".extern g_sndChannelVolumes, 16");
extern s32 D_1A7C28;              /* saved sound-channel volume snapshot (USA D_1A7BA8) */
extern s32 g_sndChannelVolumes[]; /* sound-channel volume table */

/* Galactic-map upload sequence counter (EU D_25BA80; USA D_25BA60). */
extern s32 D_25BA80[];
/* Active map cache slot index (EU sdata; USA g_mapActiveSlot). */
extern s32 g_mapActiveSlot[];
/* Newly-pressed digital pad button mask (edge-detected this frame). */
extern s32 g_padButtonsPressed[];
/* Front-end / pause-menu requested-next-screen pointer; D_25E420 is a specific
 * menu-screen instance (EU; USA g_pNextMenuScreen / D_25E660). */
extern void *g_pNextMenuScreen[];
extern u8 D_25E420[];

/* Menu widget object: an opaque 0xC0-byte bindable record (USA MenuWidget). */
typedef struct MenuWidget {
    u8 _bytes[0xC0];
} MenuWidget;

/* Active front-end / pause menu screen instance (array-modelled for the -G8
 * lui/lw address form; the same name resolves to the EU +0x80 address). */

/* Galactic-map slot allocator / toggler, in-unit (EU twins; USA
 * func_002DF368 / func_002DF428). */

/* Player character/control mode: 0=Ratchet, 1=Clank-solo, 2=Giant Clank
 * (named in both regions; resolves to the EU +0x80 address). */

/* Per-level effect-def blob base (EU segment D_001F0000); the menu code reuses
 * the slot at +0x2840 as the front-end screen-state scratch — the EU form of
 * the USA g_menuScreenBlock (= g_particleFxBlob+0x100). Same slot the sibling
 * EU unit text/1C9F58 uses. */

/* Persistent save/progress block; word 0 indexes a 19-entry lookup table
 * (named in both regions, resolves to the EU +0x80 address). */
/* 19-entry menu-background rotation table (EU D_260330; USA D_260570). */
/* GUI subsystem field query (EU twin; USA func_00342468). */
/* GUI-manager root instance (also declared later for the matching-build -G8
 * address form; compatible redeclaration for #else bodies that precede it). */
extern char *g_guiInstance;

/* Global sound trigger + its enable gate (EU twins; USA PlayGlobalSound +
 * gp-rel D_1ABD48). */
/* Dialog-voice pump (EU twin; USA PumpDialogVoiceSystem — same twin as 1C9F58). */
/* Save-page byte-accumulator commit (EU twin; USA func_002A1138). */
/* Galactic-map slot table base — 5 interleaved {id,flags} pairs at +0x40/+0x44
 * stride 8 (EU D_001B1F10; USA D_001B1E90, +0x80). */
/* 32-bit memory fill (region-neutral name). */
/* Cancel the in-flight streamed file load (EU twin; USA StopFileLoad). */
/* Map-slot backing-size / display lookup (EU twin; USA func_002DF500). */

/* --- Batch 11 callees (EU twins) --- */
/* HUD icon-id -> tex0 handle helper (EU twin; USA func_0028EDF0). Also declared
 * later for the func bodies past line 600; redundant here for func_002DAA18. */
/* Filled-rectangle primitive (EU twin; USA func_002904B0). */
/* Tiled HUD-icon quad blit (EU twin; USA DrawHudIconQuadTiled / func_0028F0D0). */
/* Textured glyph quad blit (EU twin; USA DrawGlyphQuad). */
/* DrawGlyphQuad: the glyph callers pass 8 args; the menu-bg quad caller
 * (func_002DBB88) passes two extra 64-bit tint/texture args — variadic tail keeps
 * both #else call sites gnu89-clean (the matching arm is INCLUDE_ASM regardless). */
/* EU GS screen-context anchor (USA g_gsScreenContext); glyph dims live at +0x1238/+0x123A. */
/* CD-clock RTC read (EU twin; USA sceCdReadClock). */
/* RTC BCD -> save timestamp (EU twin; USA func_00131A98). */
/* Snapshot the current level WAD for the save image (EU twin; USA func_00298A00). */
/* Close the in-flight streamed file load before a fresh save (EU twin; USA func_00299BF8). */
/* Preload the per-slot save staging block (EU twin; USA func_00297FA0). */
/* Assemble the save-image payload at dst (named in both regions). */
/* Per-slot save staging base (EU anchor g_health; slot table at +0xF8C, stride 0x800). */

/* --- Batch 12 callees / globals (EU twins) --- */
/* Recompute per-level objective completion states (named in both regions). */
/* Gather the active objective list into scratch; returns the count (named both regions). */
/* Current galactic-map level index (named; USA g_nMapCurrentLevel / g_mapCurrentLevel). */
/* Per-level ordering table indexed by map row (EU D_002616A8; USA D_00261900, -0x258). */
/* MapCache backing block base (named in both regions). */
/* Menu transition-mode side flags (named in both regions). */
/* Streamed file-load busy flag: 0 idle / nonzero CD-read in flight (named both). */
/* Save-page byte accumulator (EU twin; USA func_002DFF68). Defined later in-unit;
 * forward-declared here for func_002DA320. */

/* --- Batch 13 callees / globals (EU twins) --- */
/* Active UI language index (named in both regions). */
/* Per-language menu-background image index table (EU D_1ABB58; USA D_1ABAF0, +0x68). */
/* Menu-background image index (EU D_0025DE50; USA g_menuBgImageIndex). */
/* Leave the current level with the given result codes (named in both regions). */
/* Ship-customization confirm-screen callees (EU twins). */
extern s32  func_002D6A70(MenuCmd *cmd);  /* USA func_002D6B00: dispatch a menu command (defined later in-unit) */
/* GUI-manager root instance (declared later @-G8 form; forward decl for early bodies). */

/* --- Batch 14 callees (EU twins) — several also declared later in-unit; forward
 *     decls here for func_002DC4E8 @762 --- */
/* Append a raw GS register to the draw packet (EU twin; USA AppendGsRegPacket). */
/* Right-justified font-1 label (EU twin; USA DrawFont1RightJustifiedLabel/func_00280090). */
/* Centered font-1 label (EU twin; USA func_002801B8/DrawFont1CenteredLabel). Defined later. */
/* Left font-1 string (EU twin; USA DrawStringFont1/func_0027FBA8). Defined later. */
/* Localized-string lookup (EU twin; USA GetLocalizedString). Defined later. */

/* func_002D5508: EU twin of USA func_002D5540 — build the ship-customization preview:
 * allocate the moby scratch block (func_002DF328), spawn the fixed + optional ship
 * parts selected by the D_1A7B78 (PlayerStats+0xF8) customization bitfield through the
 * per-selector class-id tables D_1A8CD8..D_1A8D08, run a shared render-state tuning pass
 * over every spawned moby, then seed the paint cursor (D_1ABB1C) from the D_26CD98
 * {mask,value} match table. Returns 0. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_002D5540: g_menuScreenBlock->D_001F0000+
 * 0x2840, g_shipCustomization->D_1A7B78, g_shipCustomizeCursor->D_1ABB1C, class tables
 * D_1A8C28..58->D_1A8CD8..D08 (+0xB0), match table D_26CFD8->D_26CD98, D_0025D458->
 * D_0025D478; callees func_002DF368->func_002DF328, InitMobyFromClass->func_0029FA60,
 * func_002A12A0->func_002A1928, UpdateMobyBSphereAndGrid->func_002A0E28. REGION DELTA:
 * EU omits the leading func_002CAFD8() call USA makes — no twin is called in the EU asm. */
/* ===== #else-only externs (guarded) — used solely by the TARGET_NATIVE
 * structure-model arms; the matching (INCLUDE_ASM) build never sees them,
 * keeping this unit trivially byte-neutral. Retrofit of scattered file-scope
 * externs into one guarded block (matched-used externs stay unguarded above). */
#ifdef TARGET_NATIVE
extern void *g_pCurrentMenuScreen[];
extern s32 func_002DF328(s32 forceSet);  /* USA func_002DF368 */
extern s32 func_002DF3E8(s32 id);        /* USA func_002DF428 */
extern u8 g_bPlayerMode;
extern u8 D_001F0000[];
extern s32 g_playerProgress[];
extern s32 D_260330[];
extern s32 func_003435D8(void *query);
extern void func_002E6C28(s32 id, s32 a, s32 b);  /* USA PlayGlobalSound */
extern s32  D_1ABDB8;                             /* USA D_1ABD48 (gp-rel gate) */
extern void func_002B8898(s32 blocking);
/* Takes TWO args: $4 = moby table base, $5 = count. $5 is live-in — verified in
 * EU's own listing asm/eu/nonmatchings/text/19FC78/func_002A0CC0.s, where
 * `daddu $16,$5,$0` at 002A0CCC reads $5 before anything writes it (the first
 * write to $5 is the lui at 002A0CD8). The definition types the first parameter
 * as `void *tableBase` (eu/text/19FC78.c:775) — kept as s32 here because this
 * unit carries the value as an opaque handle. */
extern s32  func_002A0CC0(s32 tableBase, s32 count);
extern u8   D_001B1F10[];
extern void FillMemory32(void *dst, u32 pattern, s32 len);
extern void func_002B8688(void);
extern s32  func_002DF4C0(s32 id);
extern s32  func_0028EE08(s32 a, s32 b);
extern void func_002904C8(s32 x0, s32 y0, s32 x1, s32 y1, u32 color, s32 flag);
extern void func_0028F0E8(s32 tex, s32 x, s32 y, s32 w, s32 h, s32 alpha);
extern void func_0027E500(s32 u0, s32 v0, s32 w, s32 h, s32 a, s32 b, s32 x, s32 y, ...);
extern u8   g_saveImageArea[];
extern void func_001256D8(void *buf);
extern void func_00131AF8(void *buf);
extern void func_00298A30(void);
extern void func_002997A0(void);
extern s32  func_00297FD0(s32 addr);
extern void BuildSaveImage(void *dst);
extern u8   g_health[];
extern void UpdateLevelObjectiveStates(void);
extern s32  GatherActiveObjectives(s32 dst, s32 a, s32 idsDst, s32 flag);
extern s32  g_mapCurrentLevel;
extern s32  D_002616A8[];
extern u8   g_mapVertexData[];
extern u8   g_menuTransitionMode[];
extern s16  g_fileLoadState;
extern s32  func_002DFF28(s32 handle, s32 amount);
extern u8   g_currentLanguage;
extern s32  D_1ABB58[];
extern s32  D_0025DE50;
extern void RequestLevelExit(s32 a, s32 b);
extern s32  func_00288788(void);          /* USA func_00288898: overlay-owner query */
extern s32  func_00286070(void);          /* USA GetMenuOverlayMode */
extern void func_00288798(void);          /* USA func_002888A8: overlay-owner ack */
extern void func_0029CEF8(s32 padMask);   /* USA func_0029D398 */
extern s32  func_00343638(void *query);   /* USA func_003424C8: focused GUI list item */
extern void *g_pGuiManager;
extern void AppendGsRegPacket(s32 reg, s32 val);
extern void func_002DFF60(s32 id, s32 arg); /* menu sound player (EU; USA func_002DFFA0) */
extern void func_0027FF28(s32 x, s32 y, u32 rgba, u8 *str, s32 wrap);
extern s32  func_00280050(s32 x, s32 y, u32 rgba, char *str, s32 wrap);
extern void func_0027FA40(s32 x, s32 y, u32 rgba, u8 *str, s32 wrap);
extern char *GetLocalizedString(s32 id);
extern s32 g_nSaveLoadStatusCode;
extern void func_002DFF60(s32 id, s32 arg);
extern void func_0027FA40(s32 x, s32 y, u32 rgba, u8 *str, s32 wrap); /* DrawStringFont1 (EU) */
extern s32 func_0028EE08(s32 a, s32 b); /* USA func_0028EDF0 icon-id helper (EU) */
extern s32 func_0028EEC0(s32 iconId); /* GetHudIconTex0 (EU) */
extern void func_0028FC90(u32 x, f32 y, u32 z, u32 s, u32 rot, s32 w, s32 h, s32 tex); /* DrawHudSpriteRotated (EU) */
extern u8 D_001ABC68, D_001ABC69, D_001ABC70, D_001ABC71; /* USA D_001ABBF8/BBF9/BC00/BC01 glyphs (+0x70) */
extern s32 func_00280050(s32 cx, s32 cy, u32 rgba, char *str, s32 wrap); /* DrawFont1CenteredLabel (EU) */
extern char *GetLocalizedString(s32 id); /* GetLocalizedString (EU) */
extern s32 D_00262960[]; /* USA D_00262BA0 caption table (-0x240 lane) */
extern u8 D_001A8D38; /* USA D_001A8C88 memcard-present flag (+0xB0) */
extern void func_002CA858(void); /* USA func_002CA980 (EU) */
extern void func_0033B688(void *guiField); /* USA func_0033A7A8 (EU) */
extern s32 CountSkillPointsCompleted(void);
extern s32 func_002B19A0(void); /* USA func_002B1D40 misc-progress count (EU) */
extern u8 D_0025E970[]; /* USA D_0025EBB0 icon-tier block base (-0x240 lane) */
extern s32 D_1AB32C, D_1AB330, D_1AB334, D_1AB338; /* USA D_001AB2BC/2C0/2C4/2C8 string ids (+0x70) */
extern u8 D_001A7308[]; /* USA D_001A7278 base; +0x90 = USA D_001A7318 all-done flag (+0x80) */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5508);
#else
s32 func_002D5508(void) {
    extern s32 D_1A7B78;                 /* PlayerStats+0xF8 ship-customize bitfield (USA g_shipCustomization) */
    extern s32 D_1ABB1C;                 /* selected paint index (gp-rel; USA g_shipCustomizeCursor) */
    /* Per-selector class-id tables (s16 entries; USA D_1A8C28.. +0xB0). */
    extern s16 D_1A8CD8[], D_1A8CE0[], D_1A8CE8[], D_1A8CF0[], D_1A8CF8[], D_1A8D00[], D_1A8D08[];
    extern u8  D_26CD98[];               /* paint-match table, stride 0x14 {s32 mask, s32 value} (USA D_26CFD8) */
    extern u8  D_0025D478[];             /* USA D_0025D458 */
    extern void func_0029FA60(void *moby, s32 classId);              /* USA InitMobyFromClass */
    extern void func_002A1928(void *moby, s32 color, s32 a, s32 b, s32 c); /* USA func_002A12A0 */
    extern void func_002A0E28(void *moby);                           /* USA UpdateMobyBSphereAndGrid */
    u8 *mgr = D_001F0000 + 0x2840;
    u8 *base;
    u8 *m;
    s32 sc, n, i, cursor;
    s16 cls;

    sc = D_1A7B78;
    base = (u8 *)func_002DF328(1);
    *(void **)(mgr + 0x1CC) = base;

    /* --- Fixed base parts (slots 0-5). --- */
    func_0029FA60(base, 0xD54);
    base[0xBC] = 9;

    m = base + 0x100;
    func_0029FA60(m, D_1A8CD8[(sc >> 2) & 0x3]);
    m[0xBC] = 8;

    m = base + 0x200;
    func_0029FA60(m, D_1A8CD8[(sc >> 2) & 0x3]);
    m[0xBC] = 8;
    *(u16 *)(m + 0x34) |= 0x8000;

    m = base + 0x300;
    func_0029FA60(m, D_1A8CE0[(sc >> 4) & 0x1]);
    m[0xBC] = 0;

    m = base + 0x400;
    func_0029FA60(m, D_1A8CE8[(sc >> 6) & 0x1]);
    m[0xBC] = 1;

    m = base + 0x500;
    func_0029FA60(m, D_1A8CE8[(sc >> 6) & 0x1]);
    m[0xBC] = 1;
    *(u16 *)(m + 0x34) |= 0x8000;

    /* --- Optional / counted parts (slot n, n starts past the 6 fixed slots). --- */
    n = 6;
    cls = D_1A8CF0[(sc >> 7) & 0x3];
    if (cls != 0) {
        m = base + n * 0x100;
        func_0029FA60(m, cls);
        m[0xBC] = 2;
        n++;
    }

    m = base + n * 0x100;
    func_0029FA60(m, D_1A8CF8[(sc >> 9) & 0x1]);
    m[0xBC] = 3;
    n++;

    m = base + n * 0x100;
    func_0029FA60(m, D_1A8CF8[(sc >> 9) & 0x1]);
    m[0xBC] = 3;
    *(u16 *)(m + 0x34) |= 0x8000;
    n++;

    m = base + n * 0x100;
    func_0029FA60(m, D_1A8D00[(sc >> 10) & 0x3]);
    m[0xBC] = 4;
    n++;

    m = base + n * 0x100;
    func_0029FA60(m, D_1A8D00[(sc >> 10) & 0x3]);
    m[0xBC] = 4;
    *(u16 *)(m + 0x34) |= 0x8000;
    n++;

    if ((sc >> 12) & 0x1) {
        m = base + n * 0x100;
        func_0029FA60(m, 0x10E2);
        m[0xBC] = 5;
        n++;
    }
    if ((sc >> 13) & 0x1) {
        m = base + n * 0x100;
        func_0029FA60(m, 0x10E4);
        m[0xBC] = 6;
        n++;
    }
    if ((sc >> 14) & 0x3) {
        m = base + n * 0x100;
        func_0029FA60(m, D_1A8D08[(sc >> 14) & 0x3]);
        m[0xBC] = 7;
        n++;
    }

    /* --- Shared render-state tuning pass over every spawned moby. --- */
    for (i = 0; i < n; i++) {
        m = base + i * 0x100;
        *(u16 *)(m + 0x32) = 0x1FF;
        func_002A1928(m, 0x202020, 0xE, 0xE, 0);
        *(u16 *)(m + 0x32) = 0xFF;
        m[0x31] = 1;
        m[0x30] = 0xFF;
        if (*(s16 *)(m + 0xAA) == 0x10E3) {
            m[0x23] = 0xFF;
        }
        m[0x20] = 0;
        *(s32 *)(m + 0x98) = 0;
        *(u32 *)(m + 0x2C) = 0x3F800000;   /* 1.0f */
        *(u32 *)(m + 0xF0) = 0x3E58ED5F;
        *(u32 *)(m + 0xF4) = 0x3EAB1D93;
        *(u32 *)(m + 0xF8) = 0xC0321212;
        func_002A0E28(m);
    }

    /* --- Seed the paint cursor from the D_26CD98 {mask,value} match table. --- */
    *(s32 *)(mgr + 0x1D0) = n;
    *(s32 *)(mgr + 0x1D4) = 0x12C;
    D_1ABB1C = 0;
    cursor = 0;
    if ((sc & *(s32 *)(D_26CD98 + 0)) != *(s32 *)(D_26CD98 + 4)) {
        cursor = 1;
        while (cursor < 0x15) {
            if ((sc & *(s32 *)(D_26CD98 + cursor * 0x14)) ==
                *(s32 *)(D_26CD98 + cursor * 0x14 + 4)) {
                break;
            }
            cursor++;
        }
    }

    *(s32 *)(D_0025D478 + 0x3C) = -0x244;
    D_1ABB1C = cursor;
    return 0;
}
#endif

/* func_002D59D0: EU twin of USA func_002D5A10 — toggle the map slot at the
 * menu screen-state block +0x1CC via func_002DF3E8 and store the result back.
 * Returns 0. Matching arm stays INCLUDE_ASM (2-GPR callee-save, 8-byte-packed
 * 0x10 frame); #else is the structure-exact model. Word-verified vs USA
 * func_002D5A10: identical modulo the block base (USA g_menuScreenBlock -> EU
 * D_001F0000+0x2840, +0x80) and the jal (func_002DF428 -> EU func_002DF3E8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D59D0);
#else
s32 func_002D59D0(void) {
    u8 *blk = D_001F0000 + 0x2840;   /* EU menu screen-state block (USA g_menuScreenBlock) */
    *(s32 *)(blk + 0x1CC) = func_002DF3E8(*(s32 *)(blk + 0x1CC));
    return 0;
}
#endif

/* func_002D5A08: EU twin of USA func_002D5A48 — draw one glyph-icon legend row:
 * three colored button glyphs (0x96/0x97/0x98) from the font atlas at
 * g_guiInstance+0x8710, then a left label and a right-justified label, at a fixed
 * cursor + per-string offsets. Sibling of func_002D5CD8 over a different glyph/
 * offset/colour set. Skipped when the GUI singleton is absent. Returns 0. Matching
 * arm stays INCLUDE_ASM; the #else does not reproduce the gp/abs FP-load schedule.
 * Word-verified vs USA func_002D5A48: callees GuiFontAtlasLookupGlyph->func_00338AA8,
 * func_003017F8->func_00301AC0, func_002801B8->func_00280050, func_00280090->
 * func_0027FF28, Begin/End2dDrawBatch->func_0027CA28/func_0027CB48,
 * GetLocalizedString->GetLocalizedString; cursor/offset globals D_1ABAD0..E4 +0x68
 * (->D_1ABB38..4C), float D_1B2328->D_1B23A8 (+0x80); REGION DELTA: label string IDs
 * 0x2BF8->0x0B73 and 0x2BE5->0x0B60. Colors/glyph codes NTSC/PAL-identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5A08);
#else
s32 func_002D5A08(void) {
    extern char *g_guiInstance;
    /* Base cursor + per-string offsets for this row (EU +0x68 of USA D_1ABAD0..). */
    extern s32 D_1ABB38, D_1ABB3C, D_1ABB40, D_1ABB44, D_1ABB48, D_1ABB4C;
    /* Glyph-blit shape parameter (EU float; USA D_1B2328, +0x80). */
    extern f32 D_1B23A8;
    extern s32  func_00338AA8(char *atlas, s32 code);              /* USA GuiFontAtlasLookupGlyph */
    extern void func_00301AC0(s32 glyph, s32 color, f32 x, f32 y,
                              f32 scale, f32 sa, f32 sb);          /* USA func_003017F8 */
    char *atlas;
    s32 glyph;

    func_0027CA28(0);
    if (g_guiInstance != 0) {
        atlas = g_guiInstance + 0x8710;

        glyph = func_00338AA8(atlas, 0x96);
        func_00301AC0(glyph, 0x60442D00, (f32)D_1ABB40, (f32)D_1ABB44, 1.0f, D_1B23A8, 0.0f);
        glyph = func_00338AA8(atlas, 0x97);
        func_00301AC0(glyph, 0x60241700, (f32)D_1ABB40, (f32)D_1ABB44, 1.0f, D_1B23A8, 0.0f);
        glyph = func_00338AA8(atlas, 0x98);
        func_00301AC0(glyph, 0x55F0C070, (f32)D_1ABB40, (f32)D_1ABB44, 1.0f, D_1B23A8, 0.0f);

        func_00280050(D_1ABB40 + D_1ABB38, D_1ABB44 + D_1ABB3C, 0x80F0F0F0,
                      GetLocalizedString(0x0b73), -1);
        func_0027FF28(D_1ABB40 + D_1ABB48, D_1ABB44 + D_1ABB4C, 0x80F0F0F0,
                      (u8 *)GetLocalizedString(0x0b60), -1);
    }
    func_0027CB48();
    return 0;
}
#endif

/* Select the active language's menu-background image index from the gp-relative
 * table D_1AAAD8, stash it on the screen object (+0x34), refresh the cached
 * language snapshot (D_1ABB50) and bump the upload-sequence counter D_25BA80.
 * Returns 0. (USA twin func_002D5C08.) REGION DIFF: when the language snapshot
 * changes, EU additionally seeds obj+0x3C = -0x2BC (a scroll/timer init the USA
 * build omits) inside the snapshot-refresh branch. MATCHED. */
extern s32 D_25B9C0[];   /* current language index (USA D_25B9A0) */
extern s32 D_1ABB50;     /* cached language snapshot, gp-rel (USA D_1ABAE8) */
extern s32 D_1AAAD8[2];  /* per-language bg-index table base, gp-rel (USA D_1AAA58) */
s32 func_002D5BC8(void *obj) {
    s32 lang = D_25B9C0[0];
    if (D_1ABB50 != lang) {
        *(s32 *)((u8 *)obj + 0x3C) = -0x2BC;
        D_1ABB50 = lang;
    }
    *(s32 *)((u8 *)obj + 0x34) = D_1AAAD8[lang];
    D_25BA80[0] = lang + 1;
    return 0;
}

/* Reset the galactic-map upload sequence counter. Returns 0. (USA func_002D5C48) */
s32 func_002D5C10(void) {
    D_25BA80[0] = 0;
    return 0;
}

/* func_002D5C20: EU twin of USA func_002D5C58 — per-language menu-background
 * readiness check + screen-back driver. Selects the active language's bg image
 * index (D_0025DE50 = D_1ABB58[language]); if `focus` isn't the manager screen's
 * focused widget returns 0, else drives exit/back from g_padButtonsPressed: L1/R1
 * (0x900) returns 1 unless an override (block+0x134) is set; on back (0x10)
 * mirrors the next-screen pointer into +0x18 (returning 0), else requests parent
 * (-1) / stays (0). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA: bg table D_1ABAF0->D_1ABB58 (+0x68), g_menuBgImageIndex->
 * D_0025DE50, block base ->D_001F0000+0x2840 (+0x80); no callee retargets. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5C20);
#else
s32 func_002D5C20(void *focus) {
    u8 *blk = D_001F0000 + 0x2840;
    u8 *scr;
    s32 pressed;
    D_0025DE50 = D_1ABB58[g_currentLanguage];
    scr = *(u8 **)(blk + 0x14);
    if (*(void **)(scr + 0xE8) != focus) {
        return 0;
    }
    pressed = g_padButtonsPressed[0];
    if (pressed & 0x900) {
        if (*(s32 *)(blk + 0x134) == 0) {
            return 1;
        }
        pressed = g_padButtonsPressed[0];
    }
    if ((pressed & 0x10) == 0) {
        return 0;
    }
    scr = *(u8 **)(blk + 0x14);
    if (*(s32 *)(scr + 0xE0) != 0) {
        *(s32 *)(blk + 0x18) = *(s32 *)(scr + 0xE0);
        return 0;
    }
    if (*(s32 *)(blk + 0x134) == 0) {
        return -1;
    }
    return 0;
}
#endif

/* func_002D5CD8: EU twin of USA func_002D5D10 — draw one glyph-icon legend row:
 * three colored button glyphs (0x8B/0x8C/0x8D) from the font atlas at
 * g_guiInstance+0x8710, then two localized labels, at a fixed cursor + per-string
 * offsets. Skipped when the GUI singleton is absent. Returns 0. Matching arm stays
 * INCLUDE_ASM; the #else does not reproduce the gp/abs FP-load schedule.
 * Word-verified vs USA func_002D5D10: callees GuiFontAtlasLookupGlyph->func_00338AA8,
 * func_003017F8->func_00301AC0, func_002801B8->func_00280050, Begin/End2dDrawBatch->
 * func_0027CA28/func_0027CB48, GetLocalizedString->GetLocalizedString; cursor/offset
 * globals D_1ABB08..1C,20 +0x68 (->D_1ABB70..84,88), float D_1B2328->D_1B23A8 (+0x80);
 * REGION DELTA: label string IDs 0x2BF7->0x0B72 and 0x2BE5->0x0B60. Colors and glyph
 * codes NTSC/PAL-identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5CD8);
#else
s32 func_002D5CD8(void) {
    extern char *g_guiInstance;
    /* Base cursor + per-string offsets for this row (EU +0x68 of USA D_1ABB08..). */
    extern s32 D_1ABB70, D_1ABB74, D_1ABB78, D_1ABB7C, D_1ABB80, D_1ABB84;
    /* Glyph-blit shape parameters (EU floats; USA D_1B2328/D_1ABB20). */
    extern f32 D_1B23A8, D_1ABB88;
    extern s32  func_00338AA8(char *atlas, s32 code);              /* USA GuiFontAtlasLookupGlyph */
    extern void func_00301AC0(s32 glyph, s32 color, f32 x, f32 y,
                              f32 scale, f32 sa, f32 sb);          /* USA func_003017F8 */
    extern s32  func_00280050(s32 x, s32 y, u32 color, char *str, s32 wrap); /* USA func_002801B8 (DrawFont1CenteredLabel) */
    char *atlas;
    s32 glyph;

    func_0027CA28(0);
    if (g_guiInstance != 0) {
        atlas = g_guiInstance + 0x8710;

        glyph = func_00338AA8(atlas, 0x8B);
        func_00301AC0(glyph, 0x60442D00, (f32)D_1ABB78, (f32)D_1ABB7C, 1.0f, D_1B23A8, D_1ABB88);
        glyph = func_00338AA8(atlas, 0x8C);
        func_00301AC0(glyph, 0x55F0C070, (f32)D_1ABB78, (f32)D_1ABB7C, 1.0f, D_1B23A8, D_1ABB88);
        glyph = func_00338AA8(atlas, 0x8D);
        func_00301AC0(glyph, (s32)0x80FFDE8D, (f32)D_1ABB78, (f32)D_1ABB7C, 1.0f, D_1B23A8, 0.0f);

        func_00280050(D_1ABB78 + D_1ABB70, D_1ABB7C + D_1ABB74, 0x80F0F0F0,
                      GetLocalizedString(0x0b72), -1);
        func_00280050(D_1ABB78 + D_1ABB80, D_1ABB7C + D_1ABB84, 0x80F0F0F0,
                      GetLocalizedString(0x0b60), -1);
    }
    func_0027CB48();
    return 0;
}
#endif

/* func_002D5E90: EU twin of USA func_002D5EC8 — seed obj->0x34 from a GUI
 * subsystem query (g_guiInstance + 0x3C210) when the GUI exists. Returns 0.
 * Matching arm stays INCLUDE_ASM (2-GPR callee-save); #else is the
 * structure-exact model. Word-verified vs USA func_002D5EC8: identical modulo
 * the g_guiInstance %lo (EU +0x80, same name), the callee jal (func_00342468 ->
 * EU func_003435D8), and a GENUINE region delta — the queried GUI-struct offset
 * is 0x3C160 (USA) -> 0x3C210 (EU, +0xB0), consistent with this unit's other
 * GUI-offset shift (USA 0x3CEA0 -> EU 0x3CF50). Ported as 0x3C210, not copied. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5E90);
#else
s32 func_002D5E90(MenuWidget *obj) {
    if (g_guiInstance != 0) {
        *(s32 *)((u8 *)obj + 0x34) = func_003435D8((u8 *)g_guiInstance + 0x3C210);
    }
    return 0;
}
#endif

/* func_002D5ED8: EU twin of USA func_002D5F10 — ship-customization confirm-screen
 * input. If a forced exit is pending (overlay-owner query == 2 and overlay mode
 * == 9) leave the level. On back (0x10) mirror the next-screen pointer; L1/R1
 * (0x900) returns 1; X (0x40) reads the focused GUI list item and forwards it via
 * func_002D6A70. Returns 0/1/-1. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA (call-site order): func_00288898->
 * func_00288788, GetMenuOverlayMode->func_00286070, func_002888A8->func_00288798,
 * func_0029D398->func_0029CEF8, func_003424C8->func_00343638, func_002D6B00->
 * func_002D6A70; GUI struct offset 0x3C160->0x3C210 (+0xB0), block field
 * D_001F28F4->D_001F0000+0x2974. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5ED8);
#else
s32 func_002D5ED8(void) {
    s32 pressed = g_padButtonsPressed[0];
    if (func_00288788() == 2 && func_00286070() == 9) {
        func_00288798();
        RequestLevelExit(-1, 0);
        return 1;
    }
    func_00288798();
    if (pressed & 0x10) {
        void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && *(s32 *)(D_001F0000 + 0x2974) == 0) return -1;
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    if (pressed & 0x900) {
        return 1;
    }
    func_0029CEF8(g_padButtonsPressed[0]);   /* asm loads g_padButtonsPressed[0] into a0 */
    if ((pressed & 0x40) && g_pGuiManager != 0) {
        s32 item = func_00343638((u8 *)g_pGuiManager + 0x3C210);
        s16 cmd[8];
        cmd[1] = *(s16 *)(item + 8);
        *(s32 *)(&cmd[2]) = *(s32 *)(item + 0xc);
        func_002D6A70((MenuCmd *)cmd);
    }
    return 0;
}
#endif

/* func_002D5FF0: EU twin of USA func_002D6028 — draw the title-screen copyright /
 * legal footer: open a 2d batch, run the per-frame footer helper func_0029CEC8, then
 * right-justify the localized legal string (id 0xB60) at x=0x1C7 in colour 0x80F0F0F0,
 * y = 329 - D_1ABB8C. Returns 0. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA func_002D6028: Begin2dDrawBatch->func_0027CA28,
 * func_0029D368->func_0029CEC8, DrawFont1RightJustifiedLabel->func_0027FF28,
 * End2dDrawBatch->func_0027CB48 (GetLocalizedString named). REGION DELTAS: legal
 * string id 0x2be5->0xB60 (text-table enumeration); and EU computes the label Y as the
 * integer 0x149 - D_1ABB8C rather than USA's float D_001B2324*329+0.5 (-36 if
 * D_001B2320==1) — faithful to the EU asm (0x2D600C-18 lw/subu). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5FF0);
#else
s32 func_002D5FF0(void) {
    extern s32   D_1ABB8C;                                  /* footer Y offset (gp-rel) */
    extern void  func_0029CEC8(void);                       /* USA func_0029D368: per-frame footer helper */
    extern char *GetLocalizedString(s32 id);
    func_0027CA28(0);
    func_0029CEC8();
    func_0027FF28(0x1C7, 0x149 - D_1ABB8C, 0x80F0F0F0, (u8 *)GetLocalizedString(0xB60), -1);
    func_0027CB48();
    return 0;
}
#endif

/* Enter the ship-customization screen: prime the system (func_00288798), then
 * (if the GUI exists) bind its three data sources to the customization list and
 * show it. Returns 0. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA func_002D60E8: prime func_002888A8->func_00288798,
 * bind func_00342450->func_003435C0, show func_00342520->func_00343690; data
 * sources D_2615D8/678/730 -> D_261398/438/4D8; g_pGuiManager unchanged (named),
 * its bind-list field offset 0x3C160 -> 0x3C210 (+0xB0, EU GUI-manager layout). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6058);
#else
s32 func_002D6058(void) {
    /* GUI list bind + show helpers (EU twins; USA func_00342450 / func_00342520). */
    extern void func_003435C0(void *ctx, s32 src0, s32 src1, s32 src2);
    extern void func_00343690(void *ctx, s32 show);
    func_00288798();
    if (g_pGuiManager != 0) {
        func_003435C0((u8 *)g_pGuiManager + 0x3C210, 0x261398, 0x261438, 0x2614d8);
        func_00343690((u8 *)g_pGuiManager + 0x3C210, 1);
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D60C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D60F0);

/* Draw-batch wrapper. */
s32 func_002D6118(void) {
    func_0027CA28(0);
    func_0029D870();
    func_0027CB48();
    return 0;
}

/* GUI list show/hide for the save panel keyed off which data-source object was
 * passed: &D_1AB108 -> show (func_0033B6C0), &D_1AB0D8 -> hide (func_0033B740),
 * both on the save list at g_guiInstance + 0x3CF50. Returns 0. (USA twin
 * func_002D61D8.) REGION DIFF: list sub-offset 0x3CF50 (USA 0x3CEA0, +0xB0) and
 * source objects D_1AB108/D_1AB0D8 (USA D_1AB098/D_1AB068, +0x70). MATCHED. */
__asm__(".extern g_guiInstance, 16");
extern s32 D_1AB108;   /* save-data source object A (gp-rel; address taken) */
extern s32 D_1AB0D8;   /* save-data source object B (gp-rel; address taken) */
extern char *g_guiInstance;
extern void func_0033B6C0(void *list);
extern void func_0033B740(void *list);
s32 func_002D6148(void *which) {
    if (g_guiInstance != 0) {
        if (which == &D_1AB108) {
            func_0033B6C0(g_guiInstance + 0x3CF50);
        } else if (which == &D_1AB0D8) {
            func_0033B740(g_guiInstance + 0x3CF50);
        }
    }
    return 0;
}

/* return 0 stub. */
s32 func_002D61B0(void) {
    return 0;
}

/* func_002D61B8: EU twin of USA func_002D6248 — save/load list confirm input.
 * Sets a popup-state bit (USA D_001A7424 = the word at g_nSaveLoadStatusCode+0x4),
 * then: cancel (0x10) requests the parent; L1/R1 (0x900) returns 1; otherwise runs
 * the per-frame list tick and, on X (0x40) with the GUI up, reads the focused entry
 * and (when the card status is mid-write, code 6/0xD, item type 5) flips to overwrite
 * mode before forwarding via func_002D6A70. Returns 0/1/-1. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_002D6248:
 * func_0029D918->func_0029D478, func_00342D68->func_00343EC0, func_002D6B00->
 * func_002D6A70; GUI list field offset 0x3E9C8->0x3EA78 (+0xB0, EU GUI-manager
 * layout); block override field D_001F28F4->D_001F0000+0x2974; popup-state bit
 * D_001A7424 -> the word at g_nSaveLoadStatusCode+0x4 (EU .s names it off that base). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D61B8);
#else
s32 func_002D61B8(void) {
    extern s32  g_nSaveLoadStatusCode;        /* named; +0x4 word is the popup-state bit (USA D_001A7424) */
    extern void func_0029D478(s32 padMask);   /* USA func_0029D918: per-frame list tick */
    extern s32  func_00343EC0(void *query);   /* USA func_00342D68: focused GUI list item */
    s32 pressed = g_padButtonsPressed[0];
    s32 *popupState = (s32 *)((u8 *)&g_nSaveLoadStatusCode + 4); /* USA D_001A7424 */
    *popupState = (*popupState & ~4) | 2;
    if (pressed & 0x10) {
        void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && *(s32 *)(D_001F0000 + 0x2974) == 0) return -1;
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    if (pressed & 0x900) {
        return 1;
    }
    func_0029D478(g_padButtonsPressed[0]);
    if ((pressed & 0x40) && g_pGuiManager != 0) {
        s32 item = func_00343EC0((u8 *)g_pGuiManager + 0x3EA78);
        s16 op = *(s16 *)(item + 8);
        s16 cmd[8];
        cmd[1] = op;
        *(s32 *)(&cmd[2]) = *(s32 *)(item + 0xc);
        if ((g_nSaveLoadStatusCode == 6 || g_nSaveLoadStatusCode == 0xd) && op == 5) {
            *popupState &= ~2;
            g_nSaveLoadStatusCode = 0xc;
        }
        func_002D6A70((MenuCmd *)cmd);
    }
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D62F0(void) {
    func_0027CA28(0);
    func_0029D4B8();
    func_0027CB48();
    return 0;
}

/* func_002D6320: EU twin of USA func_002D63B0 — enable + bind a GUI save/load list
 * panel (g_pGuiManager + 0x3EA78) to its data source (D_2612E0) when the GUI exists.
 * Returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002D63B0: func_00342BE8->func_00343D40, func_00342DC0->
 * func_00343F18; GUI list field offset 0x3E9C8->0x3EA78 (+0xB0); data source
 * 0x261520->0x2612e0 (EU D_2612E0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6320);
#else
s32 func_002D6320(void) {
    extern void func_00343D40(void *ctx, s32 enable);  /* USA func_00342BE8 */
    extern void func_00343F18(void *ctx, s32 src);     /* USA func_00342DC0 */
    if (g_pGuiManager != 0) {
        func_00343D40((u8 *)g_pGuiManager + 0x3EA78, 1);
        func_00343F18((u8 *)g_pGuiManager + 0x3EA78, 0x2612e0);
    }
    return 0;
}
#endif

/* func_002D6378: EU twin of USA func_002D6408 — confirm-dialog input. L1/R1 (0x900)
 * returns 1; cancel (0x10) requests the parent; on X (0x40) with the GUI up, reads
 * the focused list item and forwards it through the command wrapper func_002D6A70.
 * Returns 0/1/-1. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002D6408: func_0029D8A8->func_0029D408, func_003432C0->
 * func_00344418, func_002D6B00->func_002D6A70; GUI list field offset 0x3E760->0x3E810
 * (+0xB0); block override field D_001F28F4->D_001F0000+0x2974. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6378);
#else
s32 func_002D6378(void) {
    extern void func_0029D408(s32 padMask);   /* USA func_0029D8A8 */
    extern s32  func_00344418(void *query);   /* USA func_003432C0: focused GUI list item */
    s32 pressed = g_padButtonsPressed[0];
    if (pressed & 0x10) {
        void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && *(s32 *)(D_001F0000 + 0x2974) == 0) return -1;
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    if (pressed & 0x900) {
        return 1;
    }
    func_0029D408(g_padButtonsPressed[0]);
    if ((pressed & 0x40) && g_pGuiManager != 0) {
        s32 item = func_00344418((u8 *)g_pGuiManager + 0x3E810);
        s16 cmd[8];
        cmd[1] = *(s16 *)(item + 8);
        *(s32 *)(&cmd[2]) = *(s32 *)(item + 0xc);
        func_002D6A70((MenuCmd *)cmd);
    }
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D6448(void) {
    func_0027CA28(0);
    func_0029D448();
    func_0027CB48();
    return 0;
}

/* func_002D6478: EU twin of USA func_002D6508 — enable + bind a different GUI list
 * panel (g_pGuiManager + 0x3E810) to its data source (D_261330) when the GUI exists.
 * Returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002D6508: func_003432B8->func_00344410, func_00343290->
 * func_003443E8; GUI list field offset 0x3E760->0x3E810 (+0xB0); data source
 * 0x261570->0x261330 (EU D_261330). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6478);
#else
s32 func_002D6478(void) {
    extern void func_00344410(void *ctx, s32 enable);  /* USA func_003432B8 */
    extern void func_003443E8(void *ctx, s32 src);     /* USA func_00343290 */
    if (g_pGuiManager != 0) {
        func_00344410((u8 *)g_pGuiManager + 0x3E810, 1);
        func_003443E8((u8 *)g_pGuiManager + 0x3E810, 0x261330);
    }
    return 0;
}
#endif

/* Forward this frame's newly-pressed buttons to a per-screen draw helper.
 * (USA func_002D6560.) */
s32 func_002D64D0(void) {
    func_0029CD38(g_padButtonsPressed[0]);
    return 0;
}

/* Draw-batch wrapper. */
s32 func_002D64F8(void) {
    func_0027CA28(0);
    func_0029CD08();
    func_0027CB48();
    return 0;
}

/* Mark no map slot active (-1). Returns 0. (USA func_002D65B8) */
s32 func_002D6528(void) {
    g_mapActiveSlot[0] = -1;
    return 0;
}

/* return 0 stub. */
s32 func_002D6540(void) {
    return 0;
}

/* func_002D6548: EU twin of USA func_002D65D8 — galactic-map level-select input.
 * Reads the newly-pressed pad mask: cancel (0x10) fades out, snaps the map camera,
 * then walks the current screen's 0xE0 child or the pending next-screen (returns -1
 * only when the modal gate at D_001F0000+0x2974 is clear); L1/R1 (0xd00) snaps the
 * selection to the player's current level; X (0x40) either re-enters the already-
 * selected level (fade + return) or requests a level change / exit, else forwards to
 * the planet-row handler func_0029CDA8. Returns 0/1/-1. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA func_002D65D8: FadeOutToBlackBlocking
 * ->func_0027D818, func_002CAB90->func_002CAA58, func_0026F7D0->func_0026F638,
 * func_0026F7D8->func_0026F640, func_0029D248->func_0029CDA8 (PlayGlobalSound/
 * RequestLevelExit/RequestGameStateChange named both regions); modal gate
 * D_001F28F4->D_001F0000+0x2974; g_abLevelVisitedMarkers[0x19]->D_1A7C89; current
 * level READ via g_mapVertexData+0x230, WRITE via g_mapCurrentLevel (EU splits the
 * two access sites). The always-true `!= 0 || .. || != 2` guard is carried verbatim
 * from the USA structure model to stay in lockstep (USA is tagged non-byte-exact). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6548);
#else
s32 func_002D6548(void) {
    extern void func_0027D818(s32 frames);   /* USA FadeOutToBlackBlocking */
    extern void func_002CAA58(f32 x);        /* USA func_002CAB90: snap map camera */
    extern s32  func_0026F638(void);         /* USA func_0026F7D0 */
    extern s32  func_0026F640(void);         /* USA func_0026F7D8 */
    extern void func_0029CDA8(void);         /* USA func_0029D248: planet-row handler */
    extern s32  RequestGameStateChange(s32 a, s32 b, s32 c, s32 d, s32 e);
    extern u8   D_1A7C89;                     /* USA g_abLevelVisitedMarkers[0x19] */
    s32 pressed = g_padButtonsPressed[0];
    s32 curLevel;

    if (pressed & 0x10) {
        void *nxt;
        func_0027D818(4);
        func_002CAA58(4.0f);
        nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && *(s32 *)(D_001F0000 + 0x2974) == 0) return -1;
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    if (pressed & 0xd00) {
        g_mapCurrentLevel = g_playerProgress[0];
        return 1;
    }
    if ((pressed & 0x40) == 0) {
        func_0029CDA8();
        return 0;
    }

    curLevel = *(s32 *)(g_mapVertexData + 0x230);
    if (g_playerProgress[0] == curLevel) {
        func_002E6C28(4, 0, 0);
        func_0027D818(4);
        return 1;
    }
    func_002E6C28(4, 0, 0);
    {
        s32 lvl;
        if (curLevel != 0 || D_1A7C89 != 0 || curLevel != 2) {
            if (func_0026F638() != 0 && func_0026F640() != 0) {
                RequestLevelExit(curLevel, 1);
                return 1;
            }
            lvl = curLevel;
        } else {
            lvl = 0x19;
        }
        RequestGameStateChange(6, 2, 1, lvl, 0);
    }
    return 1;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D66E0(void) {
    func_0027CA28(0);
    func_0029CD78();
    func_0027CB48();
    return 0;
}

/* Screen-action wrapper: MenuScreenDoAction(op, arg, &localFlag). */
s32 func_002D6710(s32 op, s32 arg) {
    s32 outFlag = 0;
    return MenuScreenDoAction(op, arg, &outFlag);
}

/* Central front-end/pause screen-action dispatcher. `op` (0-13) selects an action
 * on the menu-screen manager block (EU D_001F0000+0x2840): push/swap a screen
 * (ops 6-8,10,11 stage +0xF4/+0x100/+0x104 from the current screen fields and set
 * the +0x1C transition kind, ring the menu chrome sound func_002DFF60, clear *outFlag),
 * request the front-end game state (ops 4,5,12,13 via RequestGameStateChange with the
 * save/popup status bits at g_nSaveLoadStatusCode+0x4 toggled), set the next screen
 * (op 3) or the active language (op 9). Returns 1 when a screen push/swap was staged,
 * else 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA MenuScreenDoAction: g_menuScreenBlock->D_001F0000+0x2840;
 * func_002DFFA0->func_002DFF60, func_002888D0->func_002887C0, func_002861D8->
 * func_002860E8, func_00286138->func_00286048 (PlayGlobalSound func_002E6C28 /
 * RequestGameStateChange / g_pNextMenuScreen / g_pCurrentMenuScreen / g_currentLanguage
 * / g_nSaveLoadStatusCode / g_pTextTableLoadBuf named both regions); op-6 source table
 * g_collTriBuffer+0x1020 -> EU g_nVendorBuyQuantity+0xEFD8 (same absolute slot, other base). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", MenuScreenDoAction);
#else
s32 MenuScreenDoAction(s32 op, s32 arg, void *outFlag) {
    extern s32   g_nSaveLoadStatusCode;               /* USA g_nSaveLoadStatusCode */
    extern u8   *g_pTextTableLoadBuf;                 /* USA g_pTextTableLoadBuf */
    extern u8    g_nVendorBuyQuantity[];              /* USA g_collTriBuffer base */
    extern void  func_002887C0(void);                 /* USA func_002888D0 */
    extern void  func_002860E8(s32 a, s32 b);         /* USA func_002861D8 */
    extern void  func_00286048(s32 a, s32 b);         /* USA func_00286138 */
    extern s32   RequestGameStateChange(s32 a, s32 b, s32 c, s32 d, s32 e);
    u8 *mb = D_001F0000 + 0x2840;
    s32 *statusFlags = (s32 *)((u8 *)&g_nSaveLoadStatusCode + 4);
    s32 *popupState  = (s32 *)((u8 *)&g_pTextTableLoadBuf + 0x48);
    s32 ret = 0;

    switch (op) {
    case 0:
    case 1:
        break;

    case 2:
        func_002DFF60(2, 0x11);
        break;

    case 3:
        g_pNextMenuScreen[0] = (void *)arg;
        break;

    case 4:
        func_002E6C28(4, 0, 0);
        func_002887C0();
        *statusFlags = (*statusFlags | 2) & ~4;
        *popupState = 0;
        RequestGameStateChange(4, 1, 1, arg, 0);
        break;

    case 5:
        func_002E6C28(4, 0, 0);
        *statusFlags = (*statusFlags | 4) & ~2;
        RequestGameStateChange(4, 1, 1, arg, 0);
        *popupState = 1;
        break;

    case 6:
        if ((arg >> 16) != 0) {
            *(s32 *)(mb + 0xFC) = *(s32 *)(g_nVendorBuyQuantity + 0xEFD8 + (arg >> 16) * 4);
        }
        *(s32 *)(mb + 0xF4) = arg & 0xFFFF;
        *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
        *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
        *(s32 *)(mb + 0x1C) = 5;
        ret = 1;
        func_002DFF60(0, 0x11);
        *(s32 *)outFlag = 0;
        break;

    case 7:
        *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
        *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
        *(s32 *)(mb + 0x1C) = 3;
        *(s32 *)(mb + 0xF4) = arg;
        func_002DFF60(0, 0x11);
        *(s32 *)outFlag = 0;
        /* fall through: op 7 continues into the op 8 staging */
    case 8:
        *(s32 *)(mb + 0xF4) = arg;
        *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
        *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
        *(s32 *)(mb + 0x1C) = 4;
        ret = 1;
        func_002DFF60(0, 0x11);
        *(s32 *)outFlag = 0;
        break;

    case 9:
        g_currentLanguage = (u8)arg;
        *(s32 *)outFlag = 0;
        ret = 1;
        break;

    case 10:
        *(s32 *)(mb + 0xF4) = arg;
        *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
        *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
        *(s32 *)(mb + 0x1C) = 6;
        ret = 1;
        func_002DFF60(0, 0x11);
        *(s32 *)outFlag = 0;
        break;

    case 11:
        *(s32 *)(mb + 0x1C) = 7;
        *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
        *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
        ret = 1;
        func_002DFF60(0, 0x11);
        *(s32 *)outFlag = 0;
        break;

    case 12:
        func_002E6C28(4, 0, 0);
        func_002860E8(0, 0);
        func_00286048(0, 0);
        RequestGameStateChange(4, 1, 9, (s32)g_pCurrentMenuScreen[0], 0);
        break;

    case 13:
        RequestGameStateChange(4, 1, 0xB, (s32)g_pCurrentMenuScreen[0], 0);
        break;

    default:  /* op >= 14: no-op */
        break;
    }
    return ret;
}
#endif

/* func_002D6A10: EU twin of USA func_002D6AA0 — list-entry screen-action forwarder:
 * dispatch `op` with the arg pulled from the selected row of the widget's command
 * table (row = widget->0x40, stride 0xc off widget->0x34, arg at +4), passing the
 * caller's out-flag straight through. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_002D6AA0: no D_/global symbols; sole
 * callee MenuScreenDoAction is named in both regions. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6A10);
#else
s32 func_002D6A10(void *widget, s32 op, void *outFlag) {
    u8 *w = (u8 *)widget;
    s32 row = *(s32 *)(w + 0x40);
    s32 arg = *(s32 *)(row * 0xc + *(s32 *)(w + 0x34) + 4);
    return MenuScreenDoAction(op, arg, outFlag);
}
#endif

/* Command-record wrapper: MenuScreenDoAction(cmd->op, cmd->arg, out). */
s32 func_002D6A48(MenuCmd *cmd, void *out) {
    return MenuScreenDoAction(cmd->op, cmd->arg, out);
}

/* Command-record wrapper with a local out-flag. */
s32 func_002D6A70(MenuCmd *cmd) {
    s32 outFlag = 0;
    return MenuScreenDoAction(cmd->op, cmd->arg, &outFlag);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6A98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6E30);

/* func_002D74D8: EU twin of USA SetGalacticMapFadeAlpha — start a galactic-map
 * colour fade. Clamps a negative progress to 0, substitutes the default endpoint
 * colours (0x80FFA888 / 0x8020FFFF) for any -1 argument, and drives func_002845F8
 * (packed-RGBA colour lerp) with fade = 1 - (steps - progress)/steps, or 1.0 once
 * progress has passed the step count. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA SetGalacticMapFadeAlpha: fade step count
 * D_1AA460->D_1AA4E0 (+0x80), colour lerp func_002846E8->func_002845F8. REGION DELTA:
 * EU derives the effective step count as (D_1AA4E0*5 + 2)/6 rather than using the
 * global directly as USA does — faithful to the EU asm (0x2D74E8 sll/addu/div-by-6). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D74D8);
#else
void func_002D74D8(s32 progress, s32 color1, s32 color2) {
    extern s32  D_1AA4E0;                                        /* USA D_1AA460: fade step base */
    extern void func_002845F8(s32 color1, s32 color2, f32 fade); /* USA func_002846E8 */
    s32 clampedProgress = (progress < 0) ? 0 : progress;
    s32 steps = (D_1AA4E0 * 5 + 2) / 6;
    f32 fade;

    if (color1 == -1) {
        color1 = (s32)0x80FFA888;
    }
    if (color2 == -1) {
        color2 = (s32)0x8020FFFF;
    }

    if (steps < clampedProgress) {
        fade = 1.0f;
    } else {
        fade = 1.0f - (f32)(steps - clampedProgress) / (f32)steps;
    }
    func_002845F8(color1, color2, fade);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D75A0);

/* func_002D7AA8: EU twin of USA func_002D7AE0 — release the locked map-cache
 * slot. When idle (g_fileLoadState == 0) and a slot is locked (+0x2B0 != -1),
 * toggle that slot's 0x1000 (loaded) bit, clear the transition side flag, and
 * release lockedSlot. When a menu-driven load is in progress (block[0xDB]), abort
 * the CD read and invalidate the locked slot outright. Returns 0. MapCache fields:
 * slotLevelId[5] @+0x29C, lockedSlot @+0x2B0. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA: StopFileLoad->func_002B8688,
 * g_menuScreenBlock->D_001F0000+0x2840 (+0x80); MapCache/flag globals named both. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D7AA8);
#else
s32 func_002D7AA8(void) {
    s32 idx;
    if (g_fileLoadState == 0) {
        idx = *(s32 *)(g_mapVertexData + 0x2B0);           /* lockedSlot */
        if (idx != -1) {
            s32 *slot = (s32 *)(g_mapVertexData + 0x29C + idx * 4);  /* slotLevelId[idx] */
            g_menuTransitionMode[0xBF] = 0;
            *slot ^= 0x1000;
            *(s32 *)(g_mapVertexData + 0x2B0) = -1;
        }
    }
    if (g_fileLoadState != 0) {
        u8 *blk = D_001F0000 + 0x2840;   /* EU menu screen-state block (USA g_menuScreenBlock) */
        if (blk[0xDB] != 0) {
            func_002B8688();             /* StopFileLoad */
            blk[0xDB] = 0;
            idx = *(s32 *)(g_mapVertexData + 0x2B0);        /* lockedSlot */
            *(s32 *)(g_mapVertexData + 0x29C + idx * 4) = -1;
            *(s32 *)(g_mapVertexData + 0x2B0) = -1;
        }
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", GalacticMapScreenTick);

/* func_002D8228: EU twin of USA func_002D8270 — refresh the objectives screen:
 * recompute objective states, gather the active-objective list into scratchpad
 * (0x70000000/0x70000100), store the count in obj->0xA0, and when the current
 * map level has a valid entry seed the preview-record globals. Returns 0.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA: UpdateLevelObjectiveStates/GatherActiveObjectives named both regions;
 * preview globals +0x20 (D_0025A6FC->D_0025A71C, D_0025A6F4->D_0025A714,
 * D_0025A600->D_0025A620). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8228);
#else
s32 func_002D8228(MenuWidget *obj) {
    extern s32 D_0025A71C, D_0025A714, D_0025A620;
    u8 *o = (u8 *)obj;
    s32 *pLevel;
    UpdateLevelObjectiveStates();
    *(s32 *)(o + 0xA0) = GatherActiveObjectives(0x70000000, 0, 0x70000100, 1);
    pLevel = (s32 *)(o + 0x30 + g_mapCurrentLevel * 4);
    if (*pLevel != -1) {
        D_0025A71C = 0xfffffdc0;
        D_0025A714 = *(s32 *)(*pLevel * 4 + 0x70000000);
        D_0025A620 = *(s32 *)(0x70000100 + *pLevel * 4);
    }
    *(s32 *)(o + 0xA4) = 0;
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D82D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8700);

/* Mark no map slot active (-1). Returns 0. (Twin of func_002D6528; USA func_002D8778) */
s32 func_002D8708(void) {
    g_mapActiveSlot[0] = -1;
    return 0;
}

/* Forwarding wrapper: func_002DF178(1); return 0. */
s32 func_002D8720(void) {
    func_002DF178(1);
    return 0;
}

/**
 * func_002D8740 (USA func_002D87B0) - snapshot the saved channel volume back into
 * the live table slot 2. Recovered by pinning the EU g_sndChannelVolumes symbol
 * at 0x188728 so the slot-2 store folds into a single %hi/%lo reloc
 * (g_sndChannelVolumes+0x8) instead of section-relative D_00180000+0x8730.
 */
s32 func_002D8740(void) {
    g_sndChannelVolumes[2] = D_1A7C28;
    return 0;
}

/* func_002D8758: EU twin of USA func_002D87C8 — title-screen audio-options input.
 * L1/R1 (0x900) returns 1 unless the transition override (block+0x134) is set; cancel
 * (0x10) mirrors the next-screen pointer / requests the parent. 3-row menu in obj->0x40
 * (mod 3): Up(0x1000)/Down(0x4000) move the selection (menu chrome sound, and on a
 * screen with the 0x20 bit set, retune the previewed level via MapSetCurrentLevel);
 * rows 0/1 are the music/sfx volume sliders adjusted by held Left(0x2000)/Right(0x8000)
 * in steps of 3 clamped 0..0x400 (re-mix on change); row 2 toggles mono/stereo on X(0x40).
 * Returns 0/1/-1. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002D87C8: func_002DFFA0->func_002DFF60, ComputeAudioChannelMix
 * ->func_002E5630, func_00132938->func_00132998 (MapSetCurrentLevel / g_padButtonsPressed /
 * g_padButtonsHeld / g_anAvailableLevelOrder / g_pCurrentMenuScreen / g_pNextMenuScreen
 * named both regions); transition override D_001F28F4->D_001F0000+0x2974; volume sliders
 * g_nMusicVolume/g_nSfxVolume/g_nAudioStereoMode (0x1A7BA8/A4/A0)->D_1A7C28/D_1A7C24/
 * D_1A7C20 (+0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8758);
#else
s32 func_002D8758(MenuWidget *obj) {
    extern s32  D_1A7C24, D_1A7C20;          /* USA g_nSfxVolume / g_nAudioStereoMode; music is D_1A7C28 */
    extern s32  g_padButtonsHeld;            /* held-button mask this frame */
    extern s32  g_anAvailableLevelOrder[];   /* active level-order list */
    extern void MapSetCurrentLevel(s32 level);
    extern void func_002E5630(void);         /* USA ComputeAudioChannelMix */
    extern void func_00132998(s32 mono);     /* USA func_00132938 */
    u8 *o = (u8 *)obj;
    s32 pressed = g_padButtonsPressed[0];
    s32 sel;
    if ((pressed & 0x900) && *(s32 *)(D_001F0000 + 0x2974) == 0) {
        return 1;
    }
    if (pressed & 0x10) {
        void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && *(s32 *)(D_001F0000 + 0x2974) == 0) return -1;
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    sel = *(s32 *)(o + 0x40);
    if (pressed & 0x1000) *(s32 *)(o + 0x40) = (sel + 2) % 3;
    if (pressed & 0x4000) *(s32 *)(o + 0x40) = (*(s32 *)(o + 0x40) + 1) % 3;
    if (*(s32 *)(o + 0x40) != sel ||
        *(s32 *)((u8 *)g_pCurrentMenuScreen[0] + 0x128) != 0) {
        func_002DFF60(1, 0x11);
        if (*(u32 *)(o + 0x30) & 0x20) {
            MapSetCurrentLevel(g_anAvailableLevelOrder[*(s32 *)(o + 0x40)]);
        }
    }
    {
        s32 prevMusic = D_1A7C28;
        s32 prevSfx = D_1A7C24;
        if (g_padButtonsHeld & 0x2000) {
            if (*(s32 *)(o + 0x40) == 0) {
                s32 v = D_1A7C28 + 3;
                D_1A7C28 = (v < 0x401) ? v : 0x400;
            } else if (*(s32 *)(o + 0x40) == 1) {
                s32 v = D_1A7C24 + 3;
                D_1A7C24 = (v < 0x401) ? v : 0x400;
            }
        }
        if (g_padButtonsHeld & 0x8000) {
            if (*(s32 *)(o + 0x40) == 0) {
                D_1A7C28 -= 3;
                if (D_1A7C28 < 0) D_1A7C28 = 0;
            } else if (*(s32 *)(o + 0x40) == 1) {
                D_1A7C24 -= 3;
                if (D_1A7C24 < 0) D_1A7C24 = 0;
            }
        }
        if (prevSfx != D_1A7C24 || prevMusic != D_1A7C28) {
            func_002E5630();
        }
    }
    if (pressed & 0x40) {
        if (*(s32 *)(o + 0x40) == 2) {
            D_1A7C20 = (D_1A7C20 == 0);
        }
        func_00132998(D_1A7C20 == 0);
        func_002DFF60(0, 0x11);
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D89F8);

/* func_002D8D60: EU twin of USA func_002D8DD0 — rebuild the galactic-map planet
 * display rows: for each of the D_1A7C8C active rows, mark it active (flag=1) and
 * set its icon from D_254EC8 indexed by the (reversed) per-row source byte at
 * g_health+0xDFC; then clear the icon of the row just past the last. Returns 0.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_002D8DD0: no callees; row count D_1A7C0C->D_1A7C8C (+0x80), display records
 * D_25AD90->D_0025ADB0 (12-byte {icon@+0,flag@+2}), icon table D_254E48->D_254EC8
 * (+0x80, u32 w/ icon in low half), source-index bytes D_18D0E8->g_health+0xDFC. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8D60);
#else
s32 func_002D8D60(void) {
    /* 12-byte galactic-map planet display record (USA MapPlanetRow). */
    typedef struct MapPlanetRow { u16 icon; u16 flag; u8 _pad[8]; } MapPlanetRow;
    extern MapPlanetRow D_0025ADB0[];   /* USA D_25AD90 */
    extern u32 D_254EC8[];              /* USA D_254E48: index -> record, icon in low half */
    extern s32 D_1A7C8C[];              /* active planet-row count (USA D_1A7C0C) */
    /* Per-row (reversed) source-index bytes at g_health+0xDFC (USA D_18D0E8). */
    u8 *srcIdx = g_health + 0xDFC;
    s32 n = D_1A7C8C[0];
    s32 i;
    for (i = 0; i < n; i++) {
        D_0025ADB0[i].flag = 1;
        D_0025ADB0[i].icon = (u16)D_254EC8[srcIdx[n - 1 - i]];
    }
    D_0025ADB0[D_1A7C8C[0]].icon = 0;
    return 0;
}
#endif

/* return 0 stub. */
s32 func_002D8DE8(void) {
    return 0;
}

/* func_002D8DF0: EU twin of USA func_002D8E60 — enter the galactic-map slot
 * screen: forward-init via func_002DF178(1), clear the widget's cursor/scroll
 * fields, then for each of the 5 map slots whose id is set and below the block's
 * unlock threshold (menu-block +0x11C) mark it selectable (flags |= 2); finally
 * clear +0x50 and set the +0x10 ready bit. Returns 0. Matching arm stays
 * INCLUDE_ASM; #else is the structure-exact model. Word-verified vs USA
 * func_002D8E60: identical modulo relocs — func_002DF1B8 -> EU func_002DF178,
 * g_menuScreenBlock -> D_001F0000+0x2840, D_001B1E90 -> EU D_001B1F10 (+0x80);
 * the +0x11C threshold rides the same block base. No constant deltas. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8DF0);
#else
s32 func_002D8DF0(MenuWidget *obj) {
    s32 *ids   = (s32 *)(D_001B1F10 + 0x40);
    s32 *flags = (s32 *)(D_001B1F10 + 0x44);
    /* Slot-unlock threshold at menu-block +0x11C (USA modelled as D_001F28DC). */
    u32 limit  = *(u32 *)(D_001F0000 + 0x295C);
    s32 i;
    func_002DF178(1);
    *(s32 *)((u8 *)obj + 0x54) = 0;
    *(s32 *)((u8 *)obj + 0x38) = 0;
    for (i = 0; i <= 4; i++) {
        if (ids[i * 2] != 0 && (u32)ids[i * 2] < limit) {
            flags[i * 2] |= 2;
        }
    }
    *(s32 *)((u8 *)obj + 0x50) = 0;
    *(s32 *)((u8 *)obj + 0x10) |= 4;
    return 0;
}
#endif

/* func_002D8E90: EU twin of USA RestorePrevTextTable — pop side of the push/pop
 * text-table swap: cancel any in-flight load tied to this command (state 3), reset
 * the text-table banks via func_002DF178(1), then reinstall the saved text table
 * (cmd+0x54 base / cmd+0x38 count) into g_pActiveTextTable / g_activeTextTableCount.
 * Returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA RestorePrevTextTable: StopFileLoad->func_002B8688,
 * func_002DF1B8->func_002DF178; g_nActiveTextTableCount->g_activeTextTableCount
 * (g_pActiveTextTable named both); file-load busy flag g_nFileLoadState (0x1A63AC)
 * -> s16 at g_saveImageArea+0x1004 (EU .s base). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8E90);
#else
s32 func_002D8E90(void *cmd) {
    extern void *g_pActiveTextTable;      /* USA g_pActiveTextTable */
    extern s32   g_activeTextTableCount;  /* USA g_nActiveTextTableCount */
    u8 *c = (u8 *)cmd;
    /* File-load busy flag: s16 at g_saveImageArea+0x1004 (USA g_nFileLoadState). */
    if (*(s16 *)(g_saveImageArea + 0x1004) != 0 && *(s32 *)(c + 0x50) == 3) {
        func_002B8688();   /* StopFileLoad */
    }
    func_002DF178(1);
    if (*(void **)(c + 0x54) != 0) {
        g_activeTextTableCount = *(s32 *)(c + 0x38);
        g_pActiveTextTable = *(void **)(c + 0x54);
    }
    return 0;
}
#endif

/* StreamTextTable: EU twin of USA StreamTextTable — stream the per-language text
 * table into g_pTextTableLoadBuf, relocate the entry string pointers, install it as
 * active and save the previous table into cmd+0x54/+0x38. Sequential phase machine on
 * cmd->0x50: 0 kick the TOC header read -> 1 wait -> 2 kick the language body read
 * (byte remainder stashed in D_1ABBA0) -> 3 wait, relocate `count` entry pointers by
 * (buffer-8), swap in as g_pActiveTextTable (saving old table+count into cmd) -> 4;
 * 4/5 terminal. Returns 0. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA StreamTextTable: StartFileLoad->func_002B86D0,
 * func_00283460->func_00283370; g_discToc->D_0014B5C0, D_1ABB38->D_1ABBA0,
 * g_menuScreenBlock->D_001F0000+0x2840, file-load flag g_nFileLoadState->s16 at
 * g_saveImageArea+0x1004 (g_pTextTableLoadBuf/g_currentLanguage/g_pActiveTextTable/
 * g_subtitleState named both regions). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", StreamTextTable);
#else
s32 StreamTextTable(void *cmdArg) {
    extern u8    D_0014B5C0[];          /* master disc asset directory (USA g_discToc) */
    extern u8   *g_pTextTableLoadBuf;   /* language text-table load buffer */
    extern s32   D_1ABBA0;              /* byte remainder into the streamed TOC entry (USA D_1ABB38) */
    extern void *g_pActiveTextTable;    /* currently-installed text table */
    extern u8    g_subtitleState[];     /* +0x2C holds the active table entry count */
    extern s32   func_002B86D0(s32 buf, s32 lbn, s32 sectors);   /* USA StartFileLoad */
    extern void  func_00283370(void *dst, void *src, s32 len);   /* USA func_00283460 */
    u8 *cmd = (u8 *)cmdArg;

    switch (*(s32 *)(cmd + 0x50)) {
    case 0:
        D_1ABBA0 = 0;
        if (*(s16 *)(g_saveImageArea + 0x1004) != 0) return 0;
        if (func_002B86D0((s32)g_pTextTableLoadBuf,
                          *(s32 *)(D_0014B5C0 + 0x16B0) + *(s32 *)(D_0014B5C0 + 0x36C),
                          1) == 0) {
            *(s32 *)(cmd + 0x50) = 5;
        } else {
            *(s32 *)(cmd + 0x50) = 1;
        }
        break;

    case 1:
        if (*(s16 *)(g_saveImageArea + 0x1004) != 0) return 0;
        *(s32 *)(cmd + 0x50) = 2;
        break;

    case 2: {
        s32 entry, sectors, lbn;
        if (*(s16 *)(g_saveImageArea + 0x1004) != 0) return 0;
        entry = *(s32 *)(g_pTextTableLoadBuf + g_currentLanguage * 4);
        sectors = entry / 0x800;
        D_1ABBA0 = entry % 0x800;
        lbn = *(s32 *)(D_0014B5C0 + 0x16B0) + *(s32 *)(D_0014B5C0 + 0x36C) + sectors;
        if (func_002B86D0((s32)g_pTextTableLoadBuf, lbn, 0x64) == 0) {
            *(s32 *)(cmd + 0x50) = 5;
        } else {
            *(s32 *)(cmd + 0x50) = 3;
        }
        break;
    }

    case 3: {
        u8 *mgr = D_001F0000 + 0x2840;   /* USA g_menuScreenBlock */
        u8 *buf;
        u8 *tocStart;
        s32 count, size;
        if (*(s16 *)(g_saveImageArea + 0x1004) != 0) return 0;
        buf = *(u8 **)(mgr + 0x118);
        tocStart = buf + D_1ABBA0;
        count = *(s32 *)tocStart;
        size = *(s32 *)(tocStart + 4);
        func_00283370(buf, tocStart + 8, ((size + 3) & ~3) - 8);
        *(void **)(cmd + 0x54) = g_pActiveTextTable;
        *(s32 *)(cmd + 0x38) = *(s32 *)(g_subtitleState + 0x2C);
        *(s32 *)(g_subtitleState + 0x2C) = count;
        g_pActiveTextTable = buf;
        if (count > 0) {
            s32 reloc = (s32)buf - 8;
            s32 *p = (s32 *)buf;
            s32 k = 0;
            do {
                *p += reloc;
                k++;
                p = (s32 *)((u8 *)p + 0x10);
            } while (k < *(s32 *)(g_subtitleState + 0x2C));
        }
        *(s32 *)(cmd + 0x50) = 4;
        *(s32 *)(cmd + 0x10) &= ~4;
        break;
    }

    default:  /* states 4, 5 and any out-of-range value: no-op */
        break;
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D9108);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D96F8);

/* func_002D9BE0: EU twin of USA func_002D9C18 — position + populate one weapon-grid
 * ammo-readout cell. Computes the linear cell index (col + row*obj->0x44), fetches
 * the GUI list element for that cell, and — when the weapon in the equipped slot
 * has an ammo-capacity record (g_weaponTable[slot]!=0) — makes it visible, scrolls
 * in the current value (g_weaponXp[slot]>>5) and sets its item count; finally
 * re-lays out the cell. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA func_002D9C18: ONLY callees differ —
 * func_0034F300->func_003507A0, GuiElementSetVisible->func_00337B48,
 * GuiListSetScrollPos->func_003388F0, GuiListSetItemCount->func_003388E8,
 * func_0034EF68->func_00350408. All data globals (g_guiInstance/g_itemEquippedSlot/
 * g_weaponTable/g_weaponXp) are named + identical in both regions; the 0x36F28 GUI
 * offset and 0xE0/0x48 strides are NTSC/PAL-identical (asm-verified: NO +0xB0 shift).
 * Uses the ground-truth symbol names the asm resolves (USA C's g_pGuiManager/
 * D_00239B8C/g_weaponAmmoCapacity are aliases for g_guiInstance/g_weaponTable/g_weaponXp). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D9BE0);
#else
void func_002D9BE0(MenuWidget *obj, void *entry, s32 col, s32 row, s32 x, s32 y) {
    extern char *g_guiInstance;
    extern u8   g_itemEquippedSlot[];
    extern s32  g_weaponXp[];        /* USA C: g_weaponAmmoCapacity */
    extern u8   g_weaponTable[];     /* per-weapon record table, stride 0xE0; USA C: D_00239B8C */
    extern s32  func_003507A0(u8 *gui);                              /* USA func_0034F300 */
    extern void func_00337B48(s32 elem, s32 visible);               /* USA GuiElementSetVisible */
    extern void func_003388F0(s32 elem, s32 pos);                   /* USA GuiListSetScrollPos */
    extern void func_003388E8(s32 elem, s32 count);                 /* USA GuiListSetItemCount */
    extern void func_00350408(u8 *gui, s32 idx, s32 x, s32 y, s32 flags); /* USA func_0034EF68 */
    u8 *o = (u8 *)obj;
    s16 slot = *(s16 *)((u8 *)entry + 6);
    s32 idx = row * *(s32 *)(o + 0x44) + col;
    s32 elem = func_003507A0((u8 *)g_guiInstance + 0x36F28) + idx * 0x48;
    func_00337B48(elem, 0);
    if (*(s32 *)(g_weaponTable + g_itemEquippedSlot[slot] * 0xE0) != 0) {
        func_00337B48(elem, 1);
        func_003388F0(elem, g_weaponXp[slot] >> 5);
        func_003388E8(elem, *(s32 *)(g_weaponTable + g_itemEquippedSlot[slot] * 0xE0));
    }
    func_00350408((u8 *)g_guiInstance + 0x36F28, idx, x, y, 0x80);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D9D28);

/* func_002DA2F8: EU twin of USA func_002DA330 — set a draw record's mode field
 * (+0x2 of the object at obj->0x34): in Clank-solo (g_bPlayerMode==1) use 0,
 * otherwise 3. Always returns 0. Matching arm stays INCLUDE_ASM (register-
 * coloring near-miss); #else is the structure-exact model. Word-verified vs USA
 * func_002DA330: identical modulo the g_bPlayerMode %lo (EU +0x80, same name). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA2F8);
#else
s32 func_002DA2F8(MenuWidget *obj) {
    s16 *rec = *(s16 **)((u8 *)obj + 0x34);
    rec[1] = (g_bPlayerMode == 1) ? 0 : 3;
    return 0;
}
#endif

/* func_002DA320: EU twin of USA func_002DA358 — per-frame scan of the 24
 * galactic-map level rows (obj+0x44 stride 4): for each populated, not-yet-handled
 * row whose ordering word is set, advance its save accumulator via func_002DFF28,
 * skipping a set of rows when a 0x9999-tagged level id falls in [0x4D,0x92) and a
 * special-case row 7 guard. Returns 4. Matching arm stays INCLUDE_ASM (original
 * uses a jump-table switch); #else is the structure model. Word-verified vs USA:
 * func_002DFF68->func_002DFF28, ordering table D_00261900->D_002616A8 (-0x258). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA320);
#else
s32 func_002DA320(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 i;
    for (i = 0; i < 0x18; i++) {
        s32 *row = (s32 *)(o + 0x44 + i * 4);
        if (*row != 0 && *(s8 *)(o + 0xA4 + i) == 0 && D_002616A8[i] != 0) {
            s32 base = *(s32 *)(o + 0x44);
            if (i == 7 && *(s16 *)(*(s32 *)(o + 0x60) + 0xAA) == 0x4A &&
                *(s8 *)(base + 0x42) != *(s8 *)(base + 0x43)) {
                continue;
            }
            switch (i) {
            case 1: case 2: case 3: case 5: case 6:
            case 10: case 11: case 12: {
                u32 hdr = *(u32 *)(base + 0x40);
                if ((hdr & 0xFFFF0000) == 0x99990000) {
                    u8 lo = (u8)hdr;
                    if (lo > 0x4C) {
                        if (lo < 0x92) continue;  /* skip: locked range */
                    }
                }
                break;
            }
            default:
                break;
            }
            func_002DFF28(*row, 1);
        }
    }
    return 4;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA450);

/* func_002DA4B8: EU twin of USA func_002DA4F0 — draw the title-screen main menu.
 * Measure the widest of the fixed option strings (one extra "continue" row when a
 * save exists, g_playerProgress[0]!=0), left-align the column at >=2px, and draw
 * the rows evenly down the widget (step = obj->0x24 / rowCount). Returns 2.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_002DA4F0: AppendGsRegPacket->AppendGsRegPacket, Begin2dDrawBatch->func_0027CA28,
 * MeasureFont2Text->func_0027F680, func_0027F7A0->func_0027F608, DrawDebugString->
 * func_0027FAC0, EnableInlineColorCodes->func_0027F5F8, End2dDrawBatch->func_0027CB48;
 * GetLocalizedString stays named. REGION DELTA: localized-string IDs differ USA->EU
 * (0x2bf3->0xB6E, 0x2bff->0xB7A, save 0x2bf4->0xB6F, 0x2c00->0xB7B, 0x2c01->0xB7C,
 * 0x2be5->0xB60) — per-region text-table enumeration, not a code change. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA4B8);
#else
s32 func_002DA4B8(MenuWidget *obj) {
    extern char *GetLocalizedString(s32 id);                                /* EU 0x2898E8 */
    extern s32   func_0027F680(char *str, s32 flag);                        /* USA MeasureFont2Text */
    extern void  func_0027F608(void);                                       /* USA func_0027F7A0 */
    extern void  func_0027FAC0(s32 x, s32 y, u32 rgba, char *str, s32 wrap); /* USA DrawDebugString */
    extern void  func_0027F5F8(void);                                       /* USA EnableInlineColorCodes */
    u8 *o = (u8 *)obj;
    s32 hasSave = (g_playerProgress[0] != 0);
    s32 x, step, rowY;
    s32 maxw;
    static const s32 ids[] = {0xB6E, 0xB7A, 0xB7B, 0xB7C, 0xB60};
    s32 i, w;

    AppendGsRegPacket(0x42, 0x44);
    AppendGsRegPacket(0x47, 0xB);
    func_0027CA28(0);
    maxw = func_0027F680(GetLocalizedString(0xB6E), -1);
    { s32 t = func_0027F680(GetLocalizedString(0xB7A), -1); if (maxw <= t) maxw = t; }
    if (hasSave) {
        s32 t = func_0027F680(GetLocalizedString(0xB6F), -1);
        if (maxw <= t) maxw = t;
    }
    { s32 t = func_0027F680(GetLocalizedString(0xB7B), -1); if (maxw <= t) maxw = t; }
    { s32 t = func_0027F680(GetLocalizedString(0xB7C), -1); if (maxw <= t) maxw = t; }
    { s32 t = func_0027F680(GetLocalizedString(0xB60), -1); if (maxw <= t) maxw = t; }
    x = (*(s32 *)(o + 0x20) - maxw) >> 1;
    if (x < 2) x = 2;
    w = hasSave ? 7 : 6;
    step = *(s32 *)(o + 0x24) / w;
    func_0027F608();
    rowY = step - 6;
    for (i = 0; i < 5; i++) {
        func_0027FAC0(x, rowY, 0x80ffa888, GetLocalizedString(ids[i]), -1);
        rowY += step;
    }
    func_0027F5F8();
    func_0027CB48();
    return 2;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA708);

/* func_002DAA18: EU twin of USA func_002DAA50 — draw a small checkbox/indicator
 * at (x,y): a 10px highlight rect, an 8px inner rect (gp-rel colour), and (when
 * `on`) a 0x1E-px tick glyph on top. Matching arm stays INCLUDE_ASM; #else is the
 * structure-exact model. Word-verified vs USA: func_002904B0->func_002904C8,
 * func_0028EDF0->func_0028EE08, DrawHudIconQuadTiled(func_0028F0D0)->func_0028F0E8,
 * gp-rel colour D_1AA45C->D_1AA4DC (+0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAA18);
#else
void func_002DAA18(s32 x, s32 y, s32 on) {
    extern s32 D_1AA4DC;   /* %gp inner-rect colour (USA D_1AA45C, +0x80) */
    func_002904C8(x - 5, y - 5, x + 5, y + 5, 0x80ffa888, 0);
    func_002904C8(x - 4, y - 4, x + 4, y + 4, D_1AA4DC, 0);
    if (on) {
        func_0028F0E8(func_0028EE08(0xe99d, 1), x - 0xd, y - 0x12, 0x1e, 0x1e, 0x80);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAAC0);

/* func_002DAE38: EU twin of USA func_002DAE70 — draw the streamed full-screen
 * image widget once it has loaded (state +0x44 > 1 and cursor +0x58 >= 0): blit
 * the decoded glyph quad at the configured uv (+0x38/+0x3C) using the GS-context
 * glyph dimensions; returns 0x10 when drawn, else 0. Matching arm stays
 * INCLUDE_ASM (the original clamps the quad extent via movz; #else is the loose
 * structure model). Word-verified vs USA: DrawGlyphQuad->func_0027E500,
 * g_gsScreenContext->EU g_saveImageArea+0x10D8 (dims at +0x1238/+0x123A). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAE38);
#else
s32 func_002DAE38(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    if (*(s32 *)(o + 0x44) > 1 && *(s32 *)(o + 0x58) >= 0) {
        func_0027E500(0, 0,
                      *(s16 *)(g_saveImageArea + 0x1238),   /* ctx +0x160: glyph width  */
                      *(s16 *)(g_saveImageArea + 0x123A),   /* ctx +0x162: glyph height */
                      0, 0, *(s32 *)(o + 0x38), *(s32 *)(o + 0x3C));  /* DrawGlyphQuad */
        return 0x10;
    }
    return 0;
}
#endif

/* func_002DAF58: EU twin of USA InitMenuBgImageBuffers — allocate / initialise the
 * menu background-image double buffers. Clears the pending flag (+0x44), reserves
 * two map slots (func_002DF328) passing the "already-preloaded" bit (+0x34 & 0x200);
 * when NOT preloaded, force-allocates a real slot (func_002DF328(1)) for either
 * buffer that came back empty. Resets the stream cursor (+0x5C=0) and marks both
 * slot-state fields (+0x50/+0x54) idle (-1). Returns 0. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA InitMenuBgImageBuffers:
 * only callee retarget func_002DF368 -> func_002DF328 (x4); struct offsets identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAF58);
#else
s32 func_002DAF58(void *obj) {
    s32 preloaded = *(s32 *)((u8 *)obj + 0x34) & 0x200;

    *(s32 *)((u8 *)obj + 0x44) = 0;
    *(s32 *)((u8 *)obj + 0x48) = func_002DF328(preloaded);
    *(s32 *)((u8 *)obj + 0x4C) = func_002DF328(preloaded);

    if (preloaded == 0) {
        if (*(s32 *)((u8 *)obj + 0x48) == 0) {
            *(s32 *)((u8 *)obj + 0x48) = func_002DF328(1);
        }
        if (*(s32 *)((u8 *)obj + 0x4C) == 0) {
            *(s32 *)((u8 *)obj + 0x4C) = func_002DF328(1);
        }
    }

    *(s32 *)((u8 *)obj + 0x5C) = 0;
    *(s32 *)((u8 *)obj + 0x54) = -1;
    *(s32 *)((u8 *)obj + 0x50) = -1;
    return 0;
}
#endif

/* func_002DAFF0: EU twin of USA func_002DB028 — reset a galactic-map widget:
 * toggle the two map slots at obj+0x48/+0x4C via func_002DF3E8, clear the
 * +0x44/+0x50/+0x54 indices to -1, then pump the dialog-voice system. Returns 0.
 * Matching arm stays INCLUDE_ASM; #else is the structure-exact model.
 * Word-verified vs USA func_002DB028: identical modulo the callee jals
 * (func_002DF428 -> EU func_002DF3E8 x2; PumpDialogVoiceSystem -> func_002B8898). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAFF0);
#else
s32 func_002DAFF0(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    *(s32 *)(o + 0x48) = func_002DF3E8(*(s32 *)(o + 0x48));
    *(s32 *)(o + 0x4C) = func_002DF3E8(*(s32 *)(o + 0x4C));
    *(s32 *)(o + 0x44) = -1;
    *(s32 *)(o + 0x50) = -1;
    *(s32 *)(o + 0x54) = -1;
    func_002B8898(1);
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DB048);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DB6C8);

/* func_002DBA38: EU twin of USA LoadMenuBgImagePair — disc streamer for the menu
 * background-image pair. Per-tick phase machine on obj->0x44: phase 0 kicks a
 * StartFileLoad of the first buffer (obj+0x48) from the disc-TOC bg entry (base LBN
 * D_0014B5C0+0x36C + per-index offset at D_0014B5C0 + idx*8 + 0xDD0, sector count
 * +0xDD4); phase 1 waits for the load; phase 2 kicks the second buffer (obj+0x4C,
 * offsets +0xDF8/+0xDFC); phase 3 waits then advances to 4 (done). A kick that fails
 * to start sets phase -1. No-op (returns 0) while a buffer is empty or a load is in
 * flight. Always returns 0. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA LoadMenuBgImagePair: StartFileLoad->func_002B86D0,
 * g_discToc->D_0014B5C0 (+0x80), g_menuBgImageIndex->D_0025DE50; file-load flag
 * g_fileLoadState -> s16 at g_saveImageArea+0x1004. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBA38);
#else
s32 func_002DBA38(void *obj) {
    extern s32 func_002B86D0(s32 dest, s32 lbn, s32 sectors); /* USA StartFileLoad */
    extern u8  D_0014B5C0[];                                  /* USA g_discToc (+0x80) */
    s32 phase = *(s32 *)((u8 *)obj + 0x44);
    s32 idx;
    s32 dest, lbn, sectors;

    if (phase == 0) {
        dest = *(s32 *)((u8 *)obj + 0x48);
        if (dest == 0 || *(s16 *)(g_saveImageArea + 0x1004) != 0) {
            return 0;
        }
        idx = D_0025DE50;
        lbn = *(s32 *)(D_0014B5C0 + idx * 8 + 0xDD0) + *(s32 *)(D_0014B5C0 + 0x36C);
        sectors = *(s32 *)(D_0014B5C0 + idx * 8 + 0xDD4);
        *(s32 *)((u8 *)obj + 0x44) = (func_002B86D0(dest, lbn, sectors) == 0) ? -1 : phase + 1;
    } else if (phase == 1) {
        if (*(s16 *)(g_saveImageArea + 0x1004) != 0) {
            return 0;
        }
        *(s32 *)((u8 *)obj + 0x44) = 2;
    } else if (phase == 2) {
        dest = *(s32 *)((u8 *)obj + 0x4C);
        if (dest == 0 || *(s16 *)(g_saveImageArea + 0x1004) != 0) {
            return 0;
        }
        idx = D_0025DE50;
        lbn = *(s32 *)(D_0014B5C0 + idx * 8 + 0xDF8) + *(s32 *)(D_0014B5C0 + 0x36C);
        sectors = *(s32 *)(D_0014B5C0 + idx * 8 + 0xDFC);
        *(s32 *)((u8 *)obj + 0x44) = (func_002B86D0(dest, lbn, sectors) == 0) ? -1 : phase + 1;
    } else if (phase == 3) {
        if (*(s16 *)(g_saveImageArea + 0x1004) != 0) {
            return 0;
        }
        *(s32 *)((u8 *)obj + 0x44) = 4;
    }
    return 0;
}
#endif

/* func_002DBB88: EU twin of USA UploadMenuBgImagePair — draw the loaded menu
 * background-image pair as two textured 0x100x0x100 quads. No-op (returns 0) until
 * both disc loads have completed (obj->0x44 >= 4). Opens a 2d draw batch, then for
 * each buffer resolves its GS texture handle (func_00295550(obj+0x48 / obj+0x4C))
 * and blits a quad at the layout coords in D_1ABC20.. / D_1ABC30.. with the shared
 * tint 0x60A09080, then closes the batch. Returns 8. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. func_0027E500 (DrawGlyphQuad) reads its two trailing
 * args as 64-bit (sd marshal), so they pass as s64; it is prototyped 8-arg at file
 * scope for the glyph callers, so this 10-arg form goes through a matching fn-ptr cast
 * (as the USA sibling relies on DrawGlyphQuad's C89 implicit decl). Word-verified vs
 * USA UploadMenuBgImagePair: Begin2dDrawBatch->func_0027CA28, DrawGlyphQuad->
 * func_0027E500, End2dDrawBatch->func_0027CB48, func_002954F0->func_00295550, layout
 * tables D_1ABBB0..->D_1ABC20.. / D_1ABBC0..->D_1ABC30.. (+0x70). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBB88);
#else
s32 func_002DBB88(void *obj) {
    extern s32 func_00295550(s32 buffer);                /* USA func_002954F0: bg buffer -> GS tex handle */
    extern s32 D_1ABC20, D_1ABC24, D_1ABC28, D_1ABC2C;   /* front-quad layout (USA D_1ABBB0.., +0x70) */
    extern s32 D_1ABC30, D_1ABC34, D_1ABC38, D_1ABC3C;   /* back-quad layout  (USA D_1ABBC0.., +0x70) */
    s64 tint = (s64)0x60A09080;

    if (*(s32 *)((u8 *)obj + 0x44) < 4) {
        return 0;
    }

    func_0027CA28(0);
    func_0027E500(D_1ABC20, D_1ABC24, D_1ABC28, D_1ABC2C, 0, 0, 0x100, 0x100,
                  tint, (s64)func_00295550(*(s32 *)((u8 *)obj + 0x48)));
    func_0027E500(D_1ABC30, D_1ABC34, D_1ABC38, D_1ABC3C, 0, 0, 0x100, 0x100,
                  tint, (s64)func_00295550(*(s32 *)((u8 *)obj + 0x4C)));
    func_0027CB48();
    return 8;
}
#endif

/* func_002DBC60: EU twin of USA func_002DBC98 — draw the animated galactic-map
 * planet-cursor sprite at the active slot, with a pulsing scale driven by the global
 * frame counter and a layout that differs for the save-screen vs map-screen instances.
 * Returns 4 when drawn, else 0. Matching arm stays INCLUDE_ASM; #else is the structure
 * model (USA is tagged non-byte-exact; carried verbatim to stay in lockstep).
 * Word-verified vs USA func_002DBC98: AppendGsRegPacket->AppendGsRegPacket,
 * DrawHudSpriteTex0->func_0028FAF8; kind table D_00261978->D_261720; phase base
 * D_001B1518->D_001B1380+0x218; screen instance D_25E660->D_25E420; current level /
 * active slot READ via g_mapVertexData+0x230/+0x234; sprite tex handles D_001C5188/
 * D_001C5198->g_mapVertexData+0x268/+0x278 (g_pCurrentMenuScreen named both). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBC60);
#else
s32 func_002DBC60(void) {
    extern void func_0028FAF8(s64 tex, s32 x, s32 y, s32 a, s32 b,
                              s32 w, s32 h, s32 phase); /* USA DrawHudSpriteTex0 */
    extern s32  D_261720[];                             /* USA D_00261978 */
    extern u8   D_001B1380[];                            /* USA D_001B1518 base */
    s32 lvl  = *(s32 *)(g_mapVertexData + 0x230);
    s32 slot = *(s32 *)(g_mapVertexData + 0x234);
    s32 box[4];
    s32 x, y, w, h, kind, phase, phaseFix, off;
    if (slot < 0 || lvl == 5 || lvl == 10 || lvl == 0xf ||
        (lvl > 0x14 && lvl != 0x18)) {
        return 0;
    }
    if (g_pCurrentMenuScreen[0] == (void *)D_25E420) {
        box[1] = 0x32; box[2] = 0x26; box[3] = 0x19;
        x = 0x1d; y = 0x4d; w = 0xbf; h = 0xb2; box[0] = 0;
    } else {
        box[1] = 0x26; box[2] = 0x1c; box[3] = 0x14;
        x = 0x1e; y = 0x4c; w = 0x99; h = 0x8c; box[0] = 0;
    }
    box[0] = 0;
    kind = D_261720[slot];
    phase = *(s32 *)(D_001B1380 + 0x218) + slot * 0x2ab;
    phaseFix = (phase < 0) ? (phase + 0x7ff) : phase;
    off = box[kind];
    AppendGsRegPacket(0x47, 0);
    AppendGsRegPacket(8, 0);
    if (kind == 0) {
        func_0028FAF8(*(s64 *)(g_mapVertexData + 0x268), x << 4, y << 4, 7, 7,
                      w << 4, h << 4, 0);
    } else {
        func_0028FAF8(*(s64 *)(g_mapVertexData + 0x268), (x + off) * 0x10,
                      (y + off) * 0x10, 7, 7, (w + off * -2) * 0x10,
                      (h + off * -2) * 0x10, phase + (phaseFix >> 0xb) * -0x800);
        AppendGsRegPacket(8, 5);
        func_0028FAF8(*(s64 *)(g_mapVertexData + 0x278), x << 4, y << 4, 7, 7,
                      w << 4, h << 4, 0);
    }
    AppendGsRegPacket(0x47, 0x360b);
    return 4;
}
#endif

/* func_002DBEA8: EU twin of USA func_002DBEE0 — galactic-map list-screen cursor +
 * select input handler. While the per-obj fade-in counter (obj+0x3C) runs it
 * decrements it and drives the black fade level (g_screenFadeBlack). Only the
 * focused widget acts: L1/R1 (0x900) requests the parent when the modal gate
 * (block+0x134) is clear; cancel (0x10) walks to the current screen's 0xE0 child or
 * the pending next-screen; up/down (0x1000/0x4000) move the selection (+0x38); X
 * (0x40) either enters the level (row+0x10 bit0: RequestGameStateChange, or a
 * fade-out + confirm latch D_001B1F10[0]) or toggles the row's bool (row+4). A sound
 * plays on selection move/action. Returns 0/1/-1. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA func_002DBEE0: callees
 * func_002DFFA0->func_002DFF60 (two sites), FadeOutToBlackBlocking->func_0027D818
 * (RequestGameStateChange named both regions); g_menuScreenBlock->D_001F0000+0x2840,
 * D_001B1E90->D_001B1F10 (+0x80); pad/fade globals region-abstracted. REGION DELTA:
 * the fade-out reset frame count is 0x10 (USA/60Hz NTSC) -> 0xD (EU/50Hz PAL). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBEA8);
#else
s32 func_002DBEA8(MenuWidget *obj) {
    extern float g_screenFadeBlack;
    extern void  func_0027D818(s32 frames);   /* USA FadeOutToBlackBlocking */
    extern s32   RequestGameStateChange(s32 a, s32 b, s32 c, s32 d, s32 e);
    u8 *o = (u8 *)obj;
    u8 *blk = D_001F0000 + 0x2840;
    u8 *scr = *(u8 **)(blk + 0x14);
    s32 fadeCount = *(s32 *)(o + 0x3C);
    s32 pressed;
    s32 oldScroll;

    if (fadeCount != 0) {
        s32 dec = fadeCount - 1;
        s32 clamp = (dec < 5) ? dec : 4;
        *(s32 *)(o + 0x3C) = dec;
        g_screenFadeBlack = (float)clamp * 0.25f;
        return 0;
    }
    if (*(void **)(scr + 0xE8) != (void *)obj) {
        return 0;
    }

    pressed = g_padButtonsPressed[0];
    if (pressed & 0x900) {
        if (*(s32 *)(blk + 0x134) == 0) {
            return 1;
        }
    }
    pressed = g_padButtonsPressed[0];
    if (pressed & 0x10) {
        u8 *s = *(u8 **)(blk + 0x14);
        s32 nxt = *(s32 *)(s + 0xE0);
        if (nxt != 0) {
            *(s32 *)(blk + 0x18) = nxt;
            return 0;
        }
        if (*(s32 *)(blk + 0x134) == 0) {
            return -1;
        }
        return 0;
    }

    /* scroll value captured before any nav (for the change-sound compare). */
    oldScroll = *(s32 *)(o + 0x38);
    if (pressed & 0x1000) {
        if (oldScroll != 0) {
            *(s32 *)(o + 0x38) = oldScroll - 1;
        }
    }
    pressed = g_padButtonsPressed[0];
    if (pressed & 0x4000) {
        s32 sc = *(s32 *)(o + 0x38);
        s32 row = sc * 0x14 + *(s32 *)(o + 0x34);
        if (*(s32 *)(row + 0x14) != 0) {
            *(s32 *)(o + 0x38) = sc + 1;
        }
    }
    pressed = g_padButtonsPressed[0];
    if (pressed & 0x40) {
        s32 row;
        func_002DFF60(0, 0x11);
        row = *(s32 *)(o + 0x38) * 0x14 + *(s32 *)(o + 0x34);
        if (*(s32 *)(row + 0x10) & 0x1) {
            if (D_001B1F10[0] != 0) {
                RequestGameStateChange(4, 1, 3, *(s32 *)(blk + 0x14), 0);
            } else {
                func_0027D818(4);
                *(s32 *)(o + 0x3C) = 0xd;
                D_001B1F10[0] = (D_001B1F10[0] == 0);
            }
        } else {
            s32 ptr = *(s32 *)(row + 0x4);
            if (ptr != 0) {
                *(u8 *)ptr = (*(u8 *)ptr == 0);
            }
        }
    }
    if (*(s32 *)(o + 0x38) != oldScroll) {
        func_002DFF60(1, 0x11);
    }
    return 0;
}
#endif

/* func_002DC0B8: EU twin of USA func_002DC0F0 — draw a label/value list (rows of
 * stride 0x14 / 5 words, NUL-terminated): an optional empty-list placeholder
 * (mode bit 1 & empty list), then each row's left label and a right value string
 * chosen by the value-pointer's first byte (nonzero -> row[2], else row[3]). The
 * selected row (+0x38) is highlighted 0x8020FFFF, others 0x80FFA888. Returns 2.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA (call-site order): AppendGsRegPacket->AppendGsRegPacket,
 * Begin2dDrawBatch->func_0027CA28, GetLocalizedString->GetLocalizedString,
 * DrawStringFont1->func_0027FA40, DrawFont1RightJustifiedLabel->func_0027FF28,
 * End2dDrawBatch->func_0027CB48. REGION DELTA: empty-list placeholder localized-
 * string ID is 0x2CA2 (USA) -> 0x0C1C (EU). No struct-offset/other const deltas. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC0B8);
#else
s32 func_002DC0B8(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 *rows;
    s32 n, step, y, i;
    AppendGsRegPacket(0x47, 0x2004b);
    func_0027CA28(0);
    rows = *(s32 **)(o + 0x34);
    if ((*(u32 *)(o + 0x30) & 1) && rows[0] == 0) {
        /* empty-list placeholder text box drawn here in the original */
        GetLocalizedString(0x0c1c);
        rows = *(s32 **)(o + 0x34);
    }
    n = 0;
    while (rows[n * 5] != 0) n++;
    step = *(s32 *)(o + 0x24) / (n + 1);
    y = step - 8;
    if (rows[0] != 0) {
        i = 0;
        do {
            s32 *row = (s32 *)((u8 *)*(s32 **)(o + 0x34) + i * 0x14);
            s32 col = (i == *(s32 *)(o + 0x38)) ? 0x8020ffff : 0x80ffa888;
            u8 *vp = *(u8 **)(row + 1);
            s32 valId = (vp && *vp) ? row[2] : row[3];
            func_0027FA40(0xc, y, col, (u8 *)GetLocalizedString(row[0]), -1);
            func_0027FF28(*(s32 *)(o + 0x20) - 0xc, y, 0x80ffa888,
                          (u8 *)GetLocalizedString(valId), -1);
            i++;
            y += step;
        } while (*(s32 *)((u8 *)*(s32 **)(o + 0x34) + i * 0x14) != 0);
    }
    func_0027CB48();
    return 2;
}
#endif

/* func_002DC340: EU twin of USA func_002DC378 — options-list (stride 0x18) input
 * handler. Ignores input unless this widget is the focused one
 * (g_pCurrentMenuScreen[0]+0xE8 == obj). L1/R1 (0x900) requests the parent when
 * the modal-block gate (block+0x134) is clear; cancel (0x10) walks to the current
 * screen's 0xE0 child or the pending next-screen. Up/Down (0x1000/0x4000) move the
 * selection (+0x38, clamped to the row list), X (0x40) cycles the focused row's
 * option byte (row+4) modulo the row's option count, playing a sound on each
 * change. Returns 0/1/-1. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA func_002DC378: only data relocs (block base
 * 0x27C0->0x2840, D_001F28F4 0x28F4->0x2974, both +0x80) and callee
 * func_002DFFA0->func_002DFF60 (two call sites) differ; no NTSC/PAL constant or
 * struct-offset delta. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC340);
#else
s32 func_002DC340(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 pressed = g_padButtonsPressed[0];
    if (*(s32 *)((u8 *)g_pCurrentMenuScreen[0] + 0xE8) != (s32)o) {
        return 0;
    }
    if ((pressed & 0x900) && *(s32 *)(D_001F0000 + 0x2974) == 0) {
        return 1;
    }
    if (pressed & 0x10) {
        void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && *(s32 *)(D_001F0000 + 0x2974) == 0) {
            return -1;
        }
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    {
        s32 sel = *(s32 *)(o + 0x38);
        s32 prev = sel;
        s32 *rows = *(s32 **)(o + 0x34);
        if ((pressed & 0x1000) && sel != 0) {
            *(s32 *)(o + 0x38) = sel - 1;
        }
        if ((pressed & 0x4000) &&
            *(s32 *)((u8 *)rows + (*(s32 *)(o + 0x38)) * 0x18 + 0x18) != 0) {
            *(s32 *)(o + 0x38) += 1;
        }
        sel = *(s32 *)(o + 0x38);
        if (prev != sel) {
            func_002DFF60(1, 0x11);
        }
        {
            s32 count = 0;
            u8 *row = (u8 *)rows + sel * 0x18;
            if (*(s32 *)(row + 8) != 0) {
                s32 *opt = (s32 *)(row + 0xc);
                do {
                    count++;
                    if (*opt == 0) break;
                    opt++;
                } while (count < 4);
            }
            if ((pressed & 0x40) && *(u8 **)(row + 4) != 0) {
                u8 *val = *(u8 **)(row + 4);
                *val = (u8)((*val + 1) % count);
                func_002DFF60(0, 0x11);
            }
        }
    }
    return 0;
}
#endif

/* func_002DC4E8: EU twin of USA func_002DC520 — draw a two-column option-list
 * screen: count the rows (obj->0x34, stride 6 words / 0x18 bytes, NUL-terminated),
 * space them evenly over obj->0x24, and for each row draw the left label and a
 * right-justified value string (selected row highlighted 0x8020FFFF, else
 * 0x80FFA888). Returns 2. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA (call-site order): AppendGsRegPacket->AppendGsRegPacket,
 * Begin2dDrawBatch->func_0027CA28, DrawStringFont1->func_0027FA40,
 * DrawFont1RightJustifiedLabel->func_0027FF28, End2dDrawBatch->func_0027CB48,
 * GetLocalizedString->GetLocalizedString; no data-global/struct deltas (all obj-relative). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC4E8);
#else
s32 func_002DC4E8(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 *rows = *(s32 **)(o + 0x34);
    s32 n = 0;
    s32 step, y, i;
    AppendGsRegPacket(0x47, 0x2004b);
    func_0027CA28(0);
    while (rows[n * 6] != 0) n++;
    step = *(s32 *)(o + 0x24) / (n + 1);
    if (rows[0] != 0) {
        y = step - 8;
        for (i = 0; ; i++) {
            s32 *row = (s32 *)((u8 *)*(s32 **)(o + 0x34) + i * 0x18);
            u32 col = (i == *(s32 *)(o + 0x38)) ? 0x8020ffff : 0x80ffa888;
            u8 *optByte = *(u8 **)(row + 1);
            func_0027FA40(0xc, y, col, (u8 *)GetLocalizedString(row[0]), -1);
            func_0027FF28(*(s32 *)(o + 0x20) - 0xc, y, 0x80ffa888,
                          (u8 *)GetLocalizedString(row[*optByte + 2]), -1);
            if (*(s32 *)((u8 *)*(s32 **)(o + 0x34) + (i + 1) * 0x18) == 0) break;
            y += step;
        }
    }
    func_0027CB48();
    return 2;
}
#endif

/* func_002DC680: EU twin of USA func_002DC6B8 — rebuild the level-select list.
 * Walks g_anAvailableLevelOrder (NUL-terminated, capped at 0x1C entries); for each
 * level writes a list entry (stride 0xC) at g_pLevelSelectListEntries+0xA8: type 3,
 * the level-detail screen instance (D_25E420), plus the caption id/icon copied from
 * the caption table D_262960[lvl*0xC]. NUL-terminates the list, clears the selection
 * (+0x40), sets the "rebuilt" flag (+0x30 |= 0x8000), then selects the row whose
 * level-id list (D_1AA590) entry equals the current map level (g_mapVertexData+0x230).
 * Returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002DC6B8 (no callees): data relocs only —
 * D_262BA0->D_262960 & D_25E660->D_25E420 (both -0x240), D_1AA510->D_1AA590 (+0x80,
 * gp-rel), plus the +0x80 %lo shift of the named globals; no constant/struct delta. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC680);
#else
s32 func_002DC680(MenuWidget *obj) {
    extern s32 g_anAvailableLevelOrder[];
    extern u8  D_262960[];                /* EU level caption table (stride 0xC); USA D_262BA0 (-0x240) */
    extern u8  g_pLevelSelectListEntries[];
    extern u8  D_25E420[];                /* EU level-detail screen instance; USA D_25E660 (-0x240) */
    extern s32 *D_1AA590;                 /* EU active level-id list; USA D_1AA510 (+0x80) */
    extern u8  g_mapVertexData[];
    u8 *o = (u8 *)obj;
    u8 *e = g_pLevelSelectListEntries + 0xA8;
    s32 n = 0;
    s32 lvl = g_anAvailableLevelOrder[0];
    if (lvl != 0) {
        do {
            u8 *cap = D_262960 + lvl * 0xC;
            *(s16 *)(e + 2) = 3;
            *(s32 *)(e + 4) = (s32)D_25E420;
            *(s16 *)(e + 8) = *(u16 *)(cap + 4);
            *(s16 *)(e + 0) = *(u16 *)(cap + 0);
            n++;
            e += 0xC;
            if (n >= 0x1C) break;
            lvl = g_anAvailableLevelOrder[n];
        } while (lvl != 0);
    }
    *(s16 *)(g_pLevelSelectListEntries + 0xA8 + n * 0xC) = 0;
    *(s32 *)(o + 0x40) = 0;
    *(s32 *)(o + 0x30) |= 0x8000;
    {
        s32 *list = D_1AA590;
        s32 cur = *(s32 *)(g_mapVertexData + 0x230);
        s32 i = 0;
        if (list[0] != 0) {
            for (;;) {
                if (list[i] == cur) {
                    *(s32 *)(o + 0x40) = i;
                    break;
                }
                i++;
                if (list[i] == 0) break;
            }
        }
    }
    return 0;
}
#endif

/* If the confirm button (mask 0x40) was just pressed, request the menu screen
 * at D_25E420 as the next screen. Always returns 0. (USA func_002DC7D8) */
s32 func_002DC7A0(void) {
    if (g_padButtonsPressed[0] & 0x40) {
        g_pNextMenuScreen[0] = D_25E420;
    }
    return 0;
}

/* Submit the menu object's sub-rect (origin +0x18/+0x1C, size +0x20/+0x24) to
 * the 2D batch helper; always returns 2. (USA func_002DC800.) */
s32 func_002DC7C8(void *obj) {
    s32 x = *(s32 *)((u8 *)obj + 0x18);
    s32 y = *(s32 *)((u8 *)obj + 0x1C);
    func_002896A0(x, x + *(s32 *)((u8 *)obj + 0x20),
                  y, y + *(s32 *)((u8 *)obj + 0x24));
    return 2;
}

/* func_002DC800: EU twin of USA func_002DC838 — clear the current screen's
 * +0x12C field, then store the map-slot allocator result func_002DF328(0) into
 * obj->0x54. Returns 0. Matching arm stays INCLUDE_ASM (2-GPR callee-save,
 * 8-byte-packed 0x10 frame); #else is the structure-exact model. Word-verified
 * vs USA func_002DC838: identical modulo the g_pCurrentMenuScreen %lo (EU +0x80)
 * and the jal (func_002DF368 -> EU func_002DF328). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC800);
#else
s32 func_002DC800(MenuWidget *obj) {
    *(s32 *)((u8 *)g_pCurrentMenuScreen[0] + 0x12C) = 0;
    *(s32 *)((u8 *)obj + 0x54) = func_002DF328(0);
    return 0;
}
#endif

/* func_002DC840: EU twin of USA func_002DC878 — toggle the map-slot referenced
 * by obj->0x54 via func_002DF3E8 and store the result back. Returns 0. Matching
 * arm stays INCLUDE_ASM (2-GPR callee-save, 8-byte-packed 0x10 frame); #else is
 * the structure-exact model. Word-verified vs USA func_002DC878: identical
 * modulo the jal (func_002DF428 -> EU func_002DF3E8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC840);
#else
s32 func_002DC840(MenuWidget *obj) {
    *(s32 *)((u8 *)obj + 0x54) = func_002DF3E8(*(s32 *)((u8 *)obj + 0x54));
    return 0;
}
#endif

/* func_002DC870: EU twin of USA func_002DC8A8 — save/load list confirm/cancel
 * handler. When the status code is neither 0x10 nor 1, just mirror the active
 * screen object's +0xE0 into the manager block's +0x18; otherwise drive the
 * +0x12C commit flag from the just-pressed confirm(0x20)/cancel(0x10) buttons.
 * Returns 0. Matching arm stays INCLUDE_ASM; #else is the structure-exact model.
 * Word-verified vs USA func_002DC8A8: only +0x80 data relocs (block base
 * 0x27C0->0x2840; g_nSaveLoadStatusCode / g_padButtonsPressed) differ; no
 * constant delta. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC870);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DC870(void) {
    u8 *blk = D_001F0000 + 0x2840;   /* EU menu screen-state block (USA g_menuScreenBlock) */
    s32 status = g_nSaveLoadStatusCode;
    if (status != 0x10 && status != 1) {
        u8 *obj = *(u8 **)(blk + 0x14);
        *(s32 *)(blk + 0x18) = *(s32 *)(obj + 0xE0);
        return 0;
    }
    if (g_padButtonsPressed[0] & 0x20) {
        u8 *obj = *(u8 **)(blk + 0x14);
        *(s32 *)(blk + 0xE4) = 0;
        *(s32 *)(blk + 0x18) = *(s32 *)(obj + 0xE0);
        *(s32 *)(obj + 0x12C) = 1;
    } else if (g_padButtonsPressed[0] & 0x10) {
        u8 *obj = *(u8 **)(blk + 0x14);
        *(s32 *)(blk + 0x18) = *(s32 *)(obj + 0xE0);
        *(s32 *)(obj + 0x12C) = 0;
    }
    return 0;
}
#endif

/* func_002DC908: EU twin of USA func_002DC940 — draw the memory-card system-message
 * text box three times (shadow / shadow / face), choosing the message string from the
 * current card status code. Returns 2. Matching arm stays INCLUDE_ASM; #else is the
 * structure model (USA is tagged non-byte-exact; the flat backing-rect pass is omitted
 * here exactly as in the USA sibling, to stay in lockstep). Word-verified vs USA
 * func_002DC940: Begin2dDrawBatch->func_0027CA28, End2dDrawBatch->func_0027CB48,
 * DrawFont2TextBox->func_00280AC8; status source D_001F28A4->g_menuTransitionMode+0xC8;
 * fallback string 0x1abbe0->&D_1ABC50 (GetLocalizedString named). REGION DELTA:
 * localized-string IDs 0x2c8f/0x2c92 (USA) -> 0xC0A/0xC0D (EU) — text-table indices. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC908);
#else
s32 func_002DC908(MenuWidget *obj) {
    extern char *GetLocalizedString(s32 id);           /* named both regions */
    extern void func_00280AC8(void *box, s64 tint, s32 str, s32 mode); /* USA DrawFont2TextBox */
    extern u8   D_1ABC50[];                             /* USA fallback string 0x1abbe0 */
    u8 *o = (u8 *)obj;
    s32 status = *(s32 *)(g_menuTransitionMode + 0xC8);
    s32 str = (s32)D_1ABC50;
    func_0027CA28(0);
    if (status >= 0) {
        if (status < 3) {
            str = (s32)GetLocalizedString(0xC0A);
        } else if (status == 3) {
            str = (s32)GetLocalizedString(0xC0D);
        }
    }
    func_00280AC8(o, 0x80000000, str, -1);
    func_00280AC8(o, 0x80000000, str, -1);
    func_00280AC8(o, 0x80f0f0f0, str, -1);
    func_0027CB48();
    return 2;
}
#endif

/* func_002DCB78: EU twin of USA func_002DCBB0 — seed obj->0x34 with a rotating
 * entry from D_260330, indexed by the save block's first word modulo 19.
 * Returns 0. Matching arm stays INCLUDE_ASM (scheduling near-miss: the original
 * delays the %lo add past the divu trap); #else is the structure-exact model.
 * Word-verified vs USA func_002DCBB0: identical modulo the two data relocs —
 * g_playerProgress %lo (EU +0x80, same name) and D_260570 -> EU D_260330
 * (region-shifted table); the modulo-19 divide is byte-identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCB78);
#else
s32 func_002DCB78(MenuWidget *obj) {
    *(s32 *)((u8 *)obj + 0x34) = D_260330[(u32)g_playerProgress[0] % 19];
    return 0;
}
#endif

/* forward decl: EU sound player (defined below; USA func_002DFFA0). */

/* func_002DCBB8: EU twin of USA func_002DCBF0 — 30-entry wrap-around selector
 * (+0x40 field). L1/R1 (0x900) request the parent when the modal-block gate is
 * clear; cancel (0x10) walks to the current screen's 0xE0 child or the pending
 * next-screen; up/down d-pad (0x40 fwd, 0x20 back) cycle mod 30 with a sound on
 * each change. Returns 0/1/-1. Matching arm stays INCLUDE_ASM; #else is the
 * structure-exact model. Word-verified vs USA func_002DCBF0: only data relocs
 * (block base 0x27C0->0x2840, D_001F28F4 0x28F4->0x2974, both +0x80) and callee
 * func_002DFFA0->func_002DFF60 differ; no NTSC/PAL constant delta. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCBB8);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DCBB8(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 pressed = g_padButtonsPressed[0];
    s32 prev;
    if ((pressed & 0x900) && *(s32 *)(D_001F0000 + 0x2974) == 0) {
        return 1;
    }
    if (pressed & 0x10) {
        void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && *(s32 *)(D_001F0000 + 0x2974) == 0) {
            return -1;
        }
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    prev = *(s32 *)(o + 0x40);
    if (pressed & 0x40) {
        *(s32 *)(o + 0x40) = (prev + 1) % 0x1e;
    } else if (pressed & 0x20) {
        *(s32 *)(o + 0x40) = (prev + 0x1d) % 0x1e;
    }
    if (*(s32 *)(o + 0x40) != prev) {
        func_002DFF60(1, 0x11);
    }
    return 0;
}
#endif

/* func_002DCC90: EU twin of USA func_002DCCC8 — 12-entry wrap-around selector
 * (+0x54 field). Same L1/R1 (0x900) parent-request and cancel (0x10) child/next
 * navigation as func_002DCBB8; up/down d-pad (0x2040 fwd, 0x8020 back) cycle
 * mod 12 with a sound each step. Returns 0/1/-1. Matching arm stays INCLUDE_ASM;
 * #else is the structure-exact model. Word-verified vs USA func_002DCCC8: only
 * data relocs (block base +0x80, D_001F28F4 +0x80) and callee func_002DFFA0->
 * func_002DFF60 (two call sites) differ; no NTSC/PAL constant delta. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCC90);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DCC90(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 pressed = g_padButtonsPressed[0];
    if ((pressed & 0x900) && *(s32 *)(D_001F0000 + 0x2974) == 0) {
        return 1;
    }
    if (pressed & 0x10) {
        void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && *(s32 *)(D_001F0000 + 0x2974) == 0) {
            return -1;
        }
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    if (pressed & 0x2040) {
        *(s32 *)(o + 0x54) = (*(s32 *)(o + 0x54) + 1) % 0xc;
        func_002DFF60(1, 0x11);
    } else if (pressed & 0x8020) {
        *(s32 *)(o + 0x54) = (*(s32 *)(o + 0x54) + 0xb) % 0xc;
        func_002DFF60(1, 0x11);
    }
    return 0;
}
#endif

/* func_002DCD88: EU twin of USA func_002DCDC0 — draw a single nav arrow (left or
 * right depending on obj->0x38): a one-glyph label plus a rotated HUD arrow
 * sprite. Returns 2. Matching arm stays INCLUDE_ASM; #else is the structure-
 * exact model. Word-verified vs USA func_002DCDC0: callee retargets (Begin/End
 * 2dDrawBatch; DrawStringFont1 func_0027FBA8->func_0027FA40; icon-id func_0028EDF0
 * ->func_0028EE08; GetHudIconTex0 func_0028EEA8->func_0028EEC0; DrawHudSpriteRotated
 * func_0028FC78->func_0028FC90) and glyph-data relocs (D_001ABBF8/BC00 -> EU
 * D_001ABC68/C70, +0x70 lane) differ; float immediates and masks identical, no
 * NTSC/PAL constant delta. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCD88);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DCD88(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    u8 label[2];
    s32 h;
    s32 tex;
    func_0027CA28(0);
    if (*(s32 *)(o + 0x38) == 0) {
        label[0] = D_001ABC70;
        label[1] = D_001ABC71;
        func_0027FA40(*(s32 *)(o + 0x20) - 0x18, *(s32 *)(o + 0x24) / 2 - 8,
                      0x80ffa888, label, -1);
        h = *(s32 *)(o + 0x24);
        tex = func_0028EEC0(func_0028EE08(0xe99d, 6));
        func_0028FC90(0x43400000, (f32)(h << 3), 0x43000000, 0x43800000,
                      0x40490fdb, 0x20, 0x10, tex);
    } else {
        label[0] = D_001ABC68;
        label[1] = D_001ABC69;
        func_0027FA40(4, *(s32 *)(o + 0x24) / 2 - 8, 0x80ffa888, label, -1);
        h = *(s32 *)(o + 0x24);
        tex = func_0028EEC0(func_0028EE08(0xe99d, 6));
        func_0028FC90(0x44200000, (f32)(h << 3), 0x43000000, 0x43800000,
                      0, 0x20, 0x10, tex);
    }
    func_0027CB48();
    return 2;
}
#endif

/* func_002DCF20: EU twin of USA func_002DCF58 — draw the centered level-info
 * caption(s) for the currently-selected map row. If the row's data index is -1
 * show the generic "no info" string; otherwise show the two caption lines from
 * the D_00262960 table (idx*3 / idx*3+1). Returns 2. Matching arm stays
 * INCLUDE_ASM; #else is the structure-exact model. Word-verified vs USA
 * func_002DCF58: callee retargets (Begin/End2dDrawBatch, DrawFont1CenteredLabel
 * func_002801B8->func_00280050, GetLocalizedString func_002899F8->GetLocalizedString)
 * and data relocs (g_pCurrentMenuScreen +0x80; table D_00262BA0->D_00262960,
 * -0x240 lane). REGION CONSTANT DELTA: the "no info" string id is USA 0x2CFB /
 * EU 0xC75 (localized-string table differs per region) — ported as 0xC75. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCF20);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DCF20(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    u8 *focus = *(u8 **)((u8 *)g_pCurrentMenuScreen[0] + 0xE8);
    s32 idx = *(s32 *)(*(s32 *)(focus + 0x40) * 0xc + *(s32 *)(focus + 0x34) + 4);
    func_0027CA28(0);
    if (idx == -1) {
        func_00280050(*(s32 *)(o + 0x20) / 2, *(s32 *)(o + 0x24) / 2 - 8,
                      0x80ffa888, GetLocalizedString(0xc75), -1);
    } else {
        s32 w = *(s32 *)(o + 0x20);
        s32 h = *(s32 *)(o + 0x24);
        func_00280050(w / 2, h / 3 - 8, 0x80ffa888,
                      GetLocalizedString(D_00262960[idx * 3]), -1);
        func_00280050(w / 2, (h << 1) / 3 - 8, 0x80ffa888,
                      GetLocalizedString(D_00262960[idx * 3 + 1]), -1);
    }
    func_0027CB48();
    return 2;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD0C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD418);

/* func_002DD5F8: EU twin of USA func_002DD630 — draw the level-select row
 * label/value for the focused map screen, gated on the area-transition state
 * (area+0x15C<3, area+0x164<0, block+0x164>=0xB, area+0x8==2). Derives a level
 * index from the focused screen's area record (mod 0x1C); index -1 draws the
 * "unknown" placeholder centered, else the formatted level label (func_002DFFC8)
 * plus, when the row carries a value string id (>=0), a value line below it.
 * Returns 2. Matching arm stays INCLUDE_ASM; the #else does not model the
 * alternate-entry sp adjust or the div-by-0x1C guard. Word-verified vs USA
 * func_002DD630: callees func_002E0010->func_002DFFC8, func_002801B8->func_00280050,
 * Begin/End2dDrawBatch->func_0027CA28/func_0027CB48, GetLocalizedString->GetLocalizedString;
 * globals g_areaTable->D_139460 (+0x80), g_menuScreenBlock->D_001F0000+0x2840,
 * D_1ABC54->D_1ABCC4 (+0x70, gp-rel), g_levelSelectEntries named both regions;
 * REGION DELTA: placeholder string ID 0x2DAA->0x0E39. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD5F8);
#else
s32 func_002DD5F8(void *screenArg) {
    extern u8   D_139460[];              /* EU area table; USA g_areaTable (+0x80) */
    extern u8   g_levelSelectEntries[];  /* (labelStrId, valueStrId) pairs, stride 8 */
    extern s32  D_1ABCC4;                /* EU row vertical spacing; USA D_1ABC54 (+0x70) */
    extern char *func_002DFFC8(char *buf, s32 lvlIdx); /* USA func_002E0010: format level label */
    u8 *obj  = (u8 *)screenArg;
    u8 *area = (u8 *)D_139460;
    u8 *mgr  = D_001F0000 + 0x2840;
    u8 *focusScreen;
    s32 areaIdx, lvlIdx, x, baseY;

    focusScreen = *(u8 **)(mgr + 0x14);
    areaIdx = *(s32 *)(*(u8 **)(focusScreen + 0xE8) + 0x40);
    lvlIdx  = *(s32 *)(area + areaIdx * 0x1C + 0x30) % 0x1C;

    func_0027CA28(0);

    if (*(s32 *)(area + 0x15C) < 3 && *(s32 *)(area + 0x164) < 0 &&
        *(s32 *)(mgr + 0x164) >= 0xB && *(s32 *)(area + 0x8) == 2) {
        x     = *(s32 *)(obj + 0x18);
        baseY = *(s32 *)(obj + 0x1C);
        if (lvlIdx == -1) {
            func_00280050(x, baseY + D_1ABCC4 / 2, 0x80F0F0F0,
                          GetLocalizedString(0x0e39), -1);
        } else {
            s32 valStrId = *(s32 *)(g_levelSelectEntries + lvlIdx * 8 + 4);
            s32 labelY = (valStrId >= 0) ? baseY : baseY + (D_1ABCC4 >> 1);
            char buf[64];   /* func_002DFFC8 formats the level label here */

            func_00280050(x, labelY, 0x80F0F0F0, func_002DFFC8(buf, lvlIdx), -1);
            if (valStrId >= 0) {
                func_00280050(x, baseY + D_1ABCC4, 0x80F0F0F0,
                              GetLocalizedString(valStrId), -1);
            }
        }
    }

    func_0027CB48();
    return 2;
}
#endif

/* func_002DD7B0: EU twin of USA func_002DD7E8 — enter the galactic-map save-
 * confirm screen: reset the text-table banks (func_002DF178(1)), grab a fresh map
 * slot into obj->0x48 (obj->0x4C=0), prime the autosave/voice subsystems, and if
 * a memory card is present and the GUI exists show its panel. Returns 0. Matching
 * arm stays INCLUDE_ASM; #else is the structure-exact model. Word-verified vs USA
 * func_002DD7E8: callee retargets (func_002DF1B8->func_002DF178, func_002DF368->
 * func_002DF328, func_002CA980->func_002CA858, func_002888A8->func_00288798,
 * func_0033A7A8->func_0033B688), D_001A8C88->D_001A8D38 (+0xB0), g_pGuiManager
 * gp-rel (per-region). REGION STRUCT-OFFSET DELTA: the GUI panel field offset is
 * USA 0x3CEA0 / EU 0x3CF50 (+0xB0, the known EU GUI-struct shift) — ported EU. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD7B0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DD7B0(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    func_002DF178(1);
    *(s32 *)(o + 0x4C) = 0;
    *(s32 *)(o + 0x48) = func_002DF328(0);
    func_002CA858();
    func_00288798();
    if (D_001A8D38 != 0 && g_pGuiManager != 0) {
        func_0033B688((u8 *)g_pGuiManager + 0x3CF50);
    }
    return 0;
}
#endif

/* func_002DD820: EU twin of USA func_002DD858 — toggle the map-slot referenced
 * by obj->0x48 via func_002DF3E8 and store the result back. Returns 0. Matching
 * arm stays INCLUDE_ASM (2-GPR callee-save, 8-byte-packed 0x10 frame); #else is
 * the structure-exact model. Word-verified vs USA func_002DD858: identical
 * modulo the jal (func_002DF428 -> EU func_002DF3E8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD820);
#else
s32 func_002DD820(MenuWidget *obj) {
    *(s32 *)((u8 *)obj + 0x48) = func_002DF3E8(*(s32 *)((u8 *)obj + 0x48));
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD850);

/* func_002DDCE8: EU twin of USA func_002DDD30 — in-game / galactic-map save-slot
 * screen input + autosave-commit handler. Drives the GUI save list cursor (D_1A7C90),
 * waits for a kicked write to settle (block+0x138), then either resumes play (re-mix
 * audio, restore stereo, jump to the level / RequestGameStateChange) or requests a
 * state change; otherwise runs the 4-slot cursor (held-nav when obj+0x30 bit0 set) and,
 * on X over a valid slot, kicks a quick save (BuildSaveImage into the text-table staging
 * buffer, arm a 0xD-frame autosave) or plays the reject sound. Returns 0/1/-1. Matching
 * arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002DDD30 (callees): func_0033A7D8->func_0033B6B8,
 * func_00299960->func_00299518, func_00299968->func_00299520, ComputeAudioChannelMix->
 * func_002E5630, PumpDialogVoiceSystem->func_002B8898, func_00132938->func_00132998,
 * func_002F7328->func_002F7418, func_002D67A0->func_002D6710, func_00288898->func_00288788,
 * GetMenuOverlayMode->func_00286070, func_002CA998->func_002CA870, PlayGlobalSound->
 * func_002E6C28 (RequestGameStateChange / BuildSaveImage / g_playerProgress /
 * g_padButtonsPressed named). Data: g_menuScreenBlock->D_001F0000+0x2840 (USA D_001F28F8/
 * F4/2924/28FC = block+0x138/134/164/13c), USA save block D_00139410.. -> D_139460 (USA
 * D_00139410==+0x30, stride 0x1c), popup bit D_001A7424->g_nSaveLoadStatusCode+0x4,
 * g_nAudioStereoMode->D_1A7C20, g_abLevelVisitedMarkers[0]->D_1A7C70, D_001A7C10->D_1A7C90,
 * D_001A8C88->D_1A8D38, D_0025DB48/D_00260D98->D_0025D908/D_00260B58, D_00152C28->
 * D_0014B5C0+0x76E8, D_001C4F30->g_mapBitmapBuffer+0x8, save-image dst 0x1f29f0->
 * &g_pTextTableLoadBuf+0x118.
 * REGION DELTAS (followed the EU asm): (a) EU drops USA's `&& D_001A8C8C==0` on the L1/R1
 * return-1; (b) EU navigates screens via block+0x14 (current) / block+0x18 (next mirror),
 * not g_pCurrentMenuScreen/g_pNextMenuScreen; (c) block+0x13c stamped 0xC0E vs USA 0x2c93
 * (PAL/NTSC text-id); (d) list root g_guiInstance+0x3CF50 (USA g_pGuiManager+0x3CEA0, +0xB0
 * GUI shift); (e) held-nav mask read from D_138200+0x1B4. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DDCE8);
#else
s32 func_002DDCE8(MenuWidget *obj) {
    extern void  func_0033B6B8(void *list, s32 sel);   /* USA func_0033A7D8 */
    extern s32   func_00299518(void);                  /* USA func_00299960 */
    extern void  func_00299520(void);                  /* USA func_00299968 */
    extern void  func_002E5630(void);                  /* USA ComputeAudioChannelMix */
    extern void  func_00132998(s32 mono);              /* USA func_00132938 */
    extern void  func_002F7418(void);                  /* USA func_002F7328 */
    extern s32   func_002D6710(s32 op, s32 arg);       /* USA func_002D67A0 */
    extern s32   func_00288788(void);                  /* USA func_00288898 */
    extern s32   func_00286070(void);                  /* USA GetMenuOverlayMode */
    extern void  func_002CA870(void);                  /* USA func_002CA998 */
    extern s32   RequestGameStateChange(s32 a, s32 b, s32 c, s32 d, s32 e);
    extern void  BuildSaveImage(void *dst);
    extern char *g_guiInstance;
    extern u8    D_138200[];        /* pad block: pressed@+0x1C4, held-nav@+0x1B4 */
    extern u8    D_139460[];        /* save-record/status block (USA D_00139410 == +0x30) */
    extern s32   g_nSaveLoadStatusCode;
    extern s32   D_1A7C20;          /* USA g_nAudioStereoMode */
    extern u8    D_1A7C70;          /* USA g_abLevelVisitedMarkers[0] */
    extern s32   D_1A7C90;          /* USA D_001A7C10 GUI-list cursor */
    extern s32   D_1A8D38;          /* USA D_001A8C88 selector */
    extern u8    D_0025D908[];      /* USA D_0025DB48 */
    extern u8    D_00260B58[];      /* USA D_00260D98 */
    extern u8    D_0014B5C0[];      /* USA D_00152C28 @ +0x76E8 */
    extern u8   *g_pTextTableLoadBuf;   /* save-image staging @ +0x118 */
    extern u8    g_mapBitmapBuffer[];   /* USA D_001C4F30 @ +0x8 */
    u8  *o   = (u8 *)obj;
    u8  *blk = D_001F0000 + 0x2840;
    s32 *popupState = (s32 *)((u8 *)&g_nSaveLoadStatusCode + 4);  /* USA D_001A7424 */
    s32  prevSel = *(s32 *)(o + 0x40);
    s32  pressed, nav;

    if (g_guiInstance != 0) {
        func_0033B6B8(g_guiInstance + 0x3CF50, prevSel);
    }

    if (*(s32 *)(blk + 0x138) != 0) {
        if (*(s32 *)(D_139460 + 0x15C) >= 3 ||
            *(s32 *)(D_139460 + 0x164) >= 0 ||
            *(s32 *)(blk + 0x164) < 0xb) {
            return 0;
        }
        *(s32 *)(blk + 0x138) = 0;
        if (*(s32 *)(D_139460 + 0x16C) == 0 &&
            *(s32 *)(D_139460 + 0x10) == 0 &&
            func_00299518() == 0) {
            *(s32 *)(D_139460 + 0x17C) = 1;
            func_002E5630();
            func_002B8898(1);
            func_00132998(D_1A7C20 == 0);
            if (g_playerProgress[0] < 1 && D_1A7C70 == 0) {
                func_002F7418();
            } else {
                RequestGameStateChange(6, 2, 5, g_playerProgress[0], 0);
            }
            *(s16 *)(D_0014B5C0 + 0x76E8) = 0;
            return 0;
        }
        *(s32 *)(D_139460 + 0x17C) = 0;
        *popupState |= 0x100;
        if (RequestGameStateChange(4, 1, 1, *(s32 *)(blk + 0x14), 0) == 0) {
            return 0;
        }
    }

    pressed = g_padButtonsPressed[0];
    if ((pressed & 0x900) && *(s32 *)(blk + 0x134) == 0) {
        return 1;
    }
    if (pressed & 0x10) {
        void *nxt = *(void **)(*(u8 **)(blk + 0x14) + 0xE0);
        if (nxt != 0) {
            *(void **)(blk + 0x18) = nxt;
            return 0;
        }
        if (*(s32 *)(blk + 0x134) == 0) {
            return -1;
        }
        return 0;
    }

    if (g_nSaveLoadStatusCode == 9) {
        func_002D6710(5, (s32)(D_1A8D38 == 0 ? D_0025D908 : D_00260B58));
    } else if (g_nSaveLoadStatusCode != 0x10 && g_nSaveLoadStatusCode != 1) {
        if (func_00288788() != 0 || func_00286070() != 1) {
            void *nxt = *(void **)(*(u8 **)(blk + 0x14) + 0xE0);
            if (nxt != 0) {
                *(void **)(blk + 0x18) = nxt;
                return 0;
            }
            if (*(s32 *)(blk + 0x134) == 0) {
                return -1;
            }
            return 0;
        }
        RequestGameStateChange(4, 1, 1, *(s32 *)(blk + 0x14), 0);
    }

    if (*(s32 *)(D_139460 + 0x15C) >= 3) return 0;
    if (*(s32 *)(D_139460 + 0x164) >= 0) return 0;
    if (*(s32 *)(blk + 0x164) < 0xb) return 0;
    if (*(s32 *)(D_139460 + 0x8) != 2) return 0;

    nav = (*(u32 *)(o + 0x30) & 1) ? *(s32 *)(D_138200 + 0x1B4) : pressed;
    *(s32 *)(o + 0x40) = D_1A7C90;
    if ((nav & 0x1000) && D_1A7C90 != 0) {
        *(s32 *)(o + 0x40) = D_1A7C90 - 1;
    }
    if ((nav & 0x4000) && *(s32 *)(o + 0x40) < 3) {
        *(s32 *)(o + 0x40) = *(s32 *)(o + 0x40) + 1;
    }
    D_1A7C90 = *(s32 *)(o + 0x40);

    if (nav & 0x40) {
        if (*(s32 *)(D_139460 + 0x8) == 2 &&
            *(s32 *)(D_139460 + 0x30 + *(s32 *)(o + 0x40) * 0x1c) >= 0) {
            func_002E6C28(4, 0, 0);
            *(s32 *)(D_139460 + 0x148) = 0;
            *(s16 *)(D_139460 + 0x18) = (s16)*(s32 *)(o + 0x40);
            BuildSaveImage((u8 *)&g_pTextTableLoadBuf + 0x118);
            func_002CA870();
            if (*(s32 *)(D_139460 + 0x164) < 0) {
                *(s32 *)(D_139460 + 0x168) = 0;
                *(s32 *)(D_139460 + 0x164) = 0xd;
            }
            *(s32 *)(blk + 0x138) = 1;
            func_00299520();
            *(s32 *)(blk + 0x13c) = 0xC0E;   /* REGION DELTA: USA writes 0x2c93 */
            *(s32 *)(g_mapBitmapBuffer + 0x8) = 0;
        } else {
            func_002E6C28(5, 0, 0);
        }
    }

    if (*(s32 *)(o + 0x40) != prevSel) {
        func_002E6C28(3, 0, 0);
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DE110);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DE710);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DEC88);

/* func_002DECA0: EU twin of USA func_002DECE0 — cheat-code entry detector. While
 * the player holds the L1+R1-ish combo (held&0xf==6), records each fresh d-pad/face
 * direction into a 0x14-entry ring buffer; once full, scans the cheat table
 * (stride 0x14) for a matching 20-symbol sequence and applies the unlock (inventory
 * item, level, secret flag, or a nanotech-XP grant with a sound). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_002DECE0:
 * callees PlayGlobalSound->func_002E6C28, func_002B1880->func_002B1588
 * (MarkLevelAvailable named both regions); raw globals D_001ABD20->D_001ABD90 (+0x70,
 * gp-rel), D_001B1FA8->D_001B2028 (+0x80), D_00261BE8->D_00261990 (-0x258),
 * D_0013956E->D_001395EE (+0x80), D_001A8CEC->D_001A8D9C (+0xB0); named pad/inventory
 * globals region-abstracted. All masks/thresholds and the 0x1233 grant id are
 * NTSC/PAL-identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DECA0);
#else
void func_002DECA0(void) {
    extern u16 D_001B2028[];    /* EU entered-symbol ring buffer; USA D_001B1FA8 (+0x80) */
    extern s32 D_001ABD90;      /* EU count of symbols entered so far; USA D_001ABD20 (+0x70) */
    extern u8  D_00261990[];    /* EU cheat code table, stride 0x14; USA D_00261BE8 (-0x258) */
    extern u8  D_001395EE[];    /* EU secret-flag table; USA D_0013956E (+0x80) */
    extern s32 D_001A8D9C;      /* EU nanotech-grant latch; USA D_001A8CEC (+0xB0) */
    extern s32 g_padButtonsHeld;
    extern u8  g_inventoryNewFlag[];
    extern u8  g_inventoryOwned[];
    extern u8  g_nPendingNanotechXp;
    extern void MarkLevelAvailable(s32 level);
    extern void func_002B1588(s32 a, s32 b);   /* USA func_002B1880 */
    s32 pressed = g_padButtonsPressed[0];
    if ((g_padButtonsHeld & 0xf) != 6) {
        D_001ABD90 = 0;
        return;
    }
    if ((pressed & 0xf0a0) == 0 || D_001ABD90 >= 0x14) {
        return;
    }
    {
        u16 sym = 0;
        if ((pressed & 0x1000) == 0) {
            sym = 1;
            if ((pressed & 0x4000) == 0) {
                sym = 2;
                if ((pressed & 0x8000) == 0) {
                    sym = 3;
                    if ((pressed & 0x2000) == 0) {
                        sym = 5;
                        if (pressed & 0x80) sym = 4;
                    }
                }
            }
        }
        D_001B2028[D_001ABD90] = sym;
        D_001ABD90++;
    }
    if (D_001ABD90 == 0x14) {
        s32 found = -1;
        s32 code;
        for (code = 2; code < 0x93; code++) {
            s32 ci = code & 0xff;
            s32 ok = (D_001B2028[0] == D_00261990[ci]);
            if (ok) {
                s32 k;
                for (k = 1; k < 0x14; k++) {
                    if (D_001B2028[k] != D_00261990[(k * code + code) & 0xff]) { ok = 0; break; }
                }
            }
            if (ok) { found = code - 2; break; }
        }
        if (found != -1) {
            if (found < 0x38) {
                g_inventoryNewFlag[found] = 1;
                g_inventoryOwned[found] = 1;
            } else if ((u32)(found - 0x38) < 0x12) {
                MarkLevelAvailable(found - 0x37);
            } else if ((u32)(found - 0x4a) < 6) {
                D_001395EE[found] = 1;
            } else if ((u32)(found - 0x50) < 0x1e &&
                       *((u8 *)&g_nPendingNanotechXp + found) == 0) {
                *((u8 *)&g_nPendingNanotechXp + found) = 1;
                func_002E6C28(1, 0, 0);
                func_002B1588(0x1233, -1);
                D_001A8D9C = 1;
            }
        }
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DEEA8);

/* func_002DF178: EU twin of USA func_002DF1B8 — (re)assign the 5 map-cache slots'
 * backing addresses and flags for a text-table swap. `param` selects whether the
 * secondary banks are included: the first `base` slots point into the load buffer,
 * the next group into the second-bank base, then alternating 0x4F000-strided ranges;
 * any unused slots are zeroed. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA func_002DF1B8: slot table D_001B1E90->D_001B1F10; load
 * buffer g_pTextTableLoadBuf and second bank D_001F28DC read off the EU menu block
 * (D_001F0000+0x2840) at +0x118/+0x11C; loop counts/flags/strides byte-faithful to
 * the EU blez/beql ladder (0x11800 then 0x4F000 increments). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF178);
#else
void func_002DF178(s32 param) {
    s32 *ids   = (s32 *)(D_001B1F10 + 0x40);
    s32 *flags = (s32 *)(D_001B1F10 + 0x44);
    s32 bank2 = *(s32 *)(D_001F0000 + 0x2840 + 0x11C);
    s32 hasParam = (param != 0);
    s32 base = hasParam ? 2 : 1;
    s32 extra = hasParam ? 2 : 0;
    s32 count = base + hasParam;
    s32 buf = *(s32 *)(D_001F0000 + 0x2840 + 0x118);
    u32 filled = 0;
    u32 k;
    for (k = 0; k < (u32)base; k++) {
        ids[k * 2] = buf;
        flags[k * 2] = 0;
        buf += 0x1180 * 0x10;   /* TextTableEntry stride (0x10 bytes) => 0x11800 */
        filled = base;
    }
    while (filled < (u32)count) {
        ids[filled * 2] = bank2;
        flags[filled * 2] = 0;
        bank2 += 0x11800;
        filled++;
    }
    count += extra;
    while (filled < (u32)count) {
        ids[filled * 2] = buf;
        flags[filled * 2] = 1;
        buf += 0x4f00 * 0x10;   /* 0x4F000 */
        filled++;
    }
    count += hasParam;
    while (filled < (u32)count) {
        ids[filled * 2] = bank2;
        flags[filled * 2] = 1;
        bank2 += 0x4f000;
        filled++;
    }
    while (filled < 5) {
        ids[filled * 2] = 0;
        flags[filled * 2] = 0;
        filled++;
    }
}
#endif

/* func_002DF328: EU twin of USA func_002DF368 — reserve the first free galactic-
 * map cache slot (of 5, interleaved id/flags at D_001B1F10+0x40/+0x44 stride 8,
 * with forceSet flipping the polarity test), mark it in-use (bit 2), fill its
 * backing memory with 0xDEADBEEF sized by func_002DF4C0, and return the slot id
 * (0 if none free). Matching arm stays INCLUDE_ASM; #else is the structure-exact
 * model. Word-verified vs USA func_002DF368: identical modulo relocs —
 * D_001B1E90 -> EU D_001B1F10 (+0x80), jal func_002DF500 -> EU func_002DF4C0;
 * FillMemory32 is region-neutral. No constant deltas. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF328);
#else
s32 func_002DF328(s32 forceSet) {
    s32 *ids   = (s32 *)(D_001B1F10 + 0x40);
    u32 *flags = (u32 *)(D_001B1F10 + 0x44);
    s32 i = 0;
    for (;;) {
        u32 polarity = forceSet ? (flags[i * 2] ^ 1) : flags[i * 2];
        u32 raw = flags[i * 2];
        if ((polarity & 1) == 0 && ids[i * 2] != 0 && (raw & 2) == 0) {
            flags[i * 2] = raw | 2;
            FillMemory32((void *)ids[i * 2], 0xdeadbeef, func_002DF4C0(ids[i * 2]));
            return ids[i * 2];
        }
        i++;
        if (i > 4) return 0;
    }
}
#endif

/* func_002DF3E8: EU twin of USA func_002DF428 — release the map cache slot whose
 * id == `id`: if in-use (bit 1) and streaming (bit 4), cancel any pending stream
 * (the menu-block +0xDB gate + func_002B8688) then clear its bits. Returns 0.
 * Matching arm stays INCLUDE_ASM; #else is the structure-exact model.
 * Word-verified vs USA func_002DF428: identical modulo relocs — D_001B1E90 -> EU
 * D_001B1F10 (+0x80), the +0xDB pending gate rides the EU block base
 * (D_001F0000+0x291B; USA D_001F289B), jal StopFileLoad -> EU func_002B8688.
 * No constant deltas. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF3E8);
#else
s32 func_002DF3E8(s32 id) {
    s32 *ids     = (s32 *)(D_001B1F10 + 0x40);
    u32 *flags   = (u32 *)(D_001B1F10 + 0x44);
    u8  *pending = (u8 *)(D_001F0000 + 0x291B);   /* USA D_001F289B = block +0xDB */
    s32 i = 0;
    do {
        if (ids[i * 2] == id) {
            u32 f = flags[i * 2];
            if (f & 2) {
                if (f & 4) {
                    s32 wasPending = (*pending != 0);
                    flags[i * 2] = f ^ 4;
                    if (wasPending) {
                        func_002B8688();   /* StopFileLoad */
                        *pending = 0;
                    }
                }
                flags[i * 2] &= ~2u;
                return 0;
            }
        }
        i++;
    } while (i < 5);
    return 0;
}
#endif

/* func_002DF4C0: EU twin of USA func_002DF500 — scan the 5 map slots for id and
 * return its backing size / packed value (0x4F000 if flag bit 0 set, else
 * 0x11800); -1 if no slot matched. Matching arm stays INCLUDE_ASM; #else is the
 * structure-exact model. Word-verified vs USA func_002DF500: identical modulo the
 * D_001B1E90 -> EU D_001B1F10 relocs (+0x80). No constant deltas. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF4C0);
#else
s32 func_002DF4C0(s32 id) {
    s32 *pflag = (s32 *)(D_001B1F10 + 0x44);
    s32 *pid   = (s32 *)(D_001B1F10 + 0x40);
    s32 i = 0;
    do {
        i++;
        if (*pid == id) {
            return (*pflag & 1) ? 0x4F000 : 0x11800;
        }
        pflag += 2;
        pid += 2;
    } while (i < 5);
    return -1;
}
#endif

/* func_002DF520: EU twin of USA func_002DF560 — scan the 5-entry autosave slot
 * table (D_001B1F10+0x40 stride 8) for a slot whose id matches; on hit SET the
 * 0x4 (dirty/pending) flag and return 0, else return 1. Matching arm stays
 * INCLUDE_ASM; #else is the structure-exact model. Word-verified vs USA
 * func_002DF560: only the D_001B1E90->D_001B1F10 (+0x80) data reloc differs; no
 * constant delta. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF520);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DF520(s32 id) {
    s32 *pid   = (s32 *)(D_001B1F10 + 0x40);
    s32 *pflag = (s32 *)(D_001B1F10 + 0x44);
    s32 i = 0;
    do {
        i++;
        if (*pid == id) {
            *pflag |= 4;
            return 0;
        }
        pflag += 2;
        pid += 2;
    } while (i < 5);
    return 1;
}
#endif

/* func_002DF570: EU twin of USA func_002DF5B0 — mirror of func_002DF520 that
 * CLEARS the 0x4 flag on the matching slot; returns 0 on hit, 1 if none matched.
 * Matching arm stays INCLUDE_ASM; #else is the structure-exact model. Word-
 * verified vs USA func_002DF5B0: only the D_001B1E90->D_001B1F10 (+0x80) data
 * reloc differs; no constant delta. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF570);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DF570(s32 id) {
    s32 *pid   = (s32 *)(D_001B1F10 + 0x40);
    s32 *pflag = (s32 *)(D_001B1F10 + 0x44);
    s32 i = 0;
    do {
        i++;
        if (*pid == id) {
            *pflag &= ~4;
            return 0;
        }
        pflag += 2;
        pid += 2;
    } while (i < 5);
    return 1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF5C0);

/* func_002DF5E0: EU twin of USA func_002DF620 — fill a 16-bit sprite/quad header
 * (dst) from a source rect (src): copy src width(+0x24)/height(+0x20) into the
 * size fields and their halves into the centre fields, with fixed framing
 * constants. Matching arm stays INCLUDE_ASM; #else is the structure-exact model.
 * Word-verified vs USA func_002DF620: BYTE-IDENTICAL (region-invariant, no data
 * refs) — no reloc or constant delta. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF5E0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
void func_002DF5E0(s16 *dst, void *src) {
    dst[0] = 0;
    dst[1] = *(u16 *)((u8 *)src + 0x24);
    dst[2] = 0;
    dst[3] = *(u16 *)((u8 *)src + 0x20);
    dst[4] = (s16)(*(s32 *)((u8 *)src + 0x20) >> 1);
    dst[8] = 0x10;
    dst[5] = (s16)(*(s32 *)((u8 *)src + 0x24) >> 1);
    dst[9] = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF620);

/* func_002DF628: EU twin of USA func_002DF668 — build an in-progress save image
 * for `slot`: timestamp from the CD RTC, snapshot the current level WAD, stage
 * the per-slot buffer, build the image at `dst`, and arm the 0x13-frame autosave
 * countdown if idle. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA: sceCdReadClock->func_001256D8,
 * func_00131A98->func_00131AF8, func_00298A00->func_00298A30,
 * func_00297FA0->func_00297FD0; save-state globals +0x80 (D_00139554->D_001395D4,
 * D_001393F8->D_00139478, D_00139528->D_001395A8, D_00139544->D_001395C4,
 * D_00139548->D_001395C8), clock buf D_001A7360->D_001A73E0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF628);
#else
void func_002DF628(void *dst, s16 slot) {
    extern u8  D_001A73E0[];
    extern s32 D_001395D4, D_001395A8, D_001395C4, D_001395C8;
    extern s16 D_00139478;
    func_001256D8((void *)D_001A73E0);
    func_00131AF8((void *)D_001A73E0);
    func_00298A30();
    func_00297FD0(g_playerProgress[0] * 0x800 + (s32)(g_health + 0xF8C));
    BuildSaveImage(dst);
    D_001395D4 = (s32)dst;
    D_00139478 = slot;
    D_001395A8 = 0;
    if (D_001395C4 < 0) {
        D_001395C8 = 0;
        D_001395C4 = 0x13;
    }
}
#endif

/* func_002DF6D0: EU twin of USA func_002DF710 — build a fresh (new-game-style)
 * save image into `dst` for `slot`: close any pending stream, timestamp from the
 * CD RTC, build the image, clear the slot's saved-progress word, and arm the
 * autosave countdown if idle. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA: func_00299BF8->func_002997A0,
 * sceCdReadClock->func_001256D8, func_00131A98->func_00131AF8; save-state globals
 * +0x80 (D_00139410->D_00139490, D_00139528->D_001395A8, D_001393F8->D_00139478,
 * D_00139554->D_001395D4, D_00139544->D_001395C4, D_00139548->D_001395C8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF6D0);
#else
void func_002DF6D0(void *dst, s32 slot) {
    extern u8  D_001A73E0[];
    extern s32 D_001395D4, D_001395A8, D_001395C4, D_001395C8;
    extern s16 D_00139478;
    extern s32 D_00139490[];
    func_002997A0();
    func_001256D8((void *)D_001A73E0);
    func_00131AF8((void *)D_001A73E0);
    BuildSaveImage(dst);
    D_001395A8 = 0;
    D_00139478 = (s16)slot;
    *(s32 *)((u8 *)D_00139490 + slot * 0x1c) = 0;
    D_001395D4 = (s32)dst;
    if (D_001395C4 < 0) {
        D_001395C8 = 0;
        D_001395C4 = 0x13;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", RestorePlayerProgressState);

/* func_002DFE20: EU twin of USA func_002DFE60 — configure the skill-points /
 * unlockables summary screen: from the two skill-point counters (thresholds 15 /
 * 30) and a misc count (threshold 10), pick "complete" vs "partial" icon tiers
 * (D_0025E970 fields) and localized-string ids (D_1AB32C..338), and clear the
 * combined-tier field when the all-done flag is set. Matching arm stays
 * INCLUDE_ASM; #else is the structure-exact model. Word-verified vs USA
 * func_002DFE60: callee retargets (func_002B1D40->func_002B19A0; CountSkillPoints
 * Completed is region-neutral) and data relocs (D_0025Exxx -0x240 -> base
 * D_0025E970; D_001AB2xx +0x70 -> D_1AB32C..338; D_001A7318 +0x80 -> D_001A7308+
 * 0x90). REGION CONSTANT DELTA: all 8 summary string ids shift -0x2086 (USA
 * 0x2CB5/B6/B9/BA/BB/BC/BD -> EU 0xC2F/30/33/34/35/36/37) — localized-string
 * table differs per region; ported EU ids. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFE20);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
void func_002DFE20(void) {
    s32 sp1 = CountSkillPointsCompleted();
    s32 sp2 = CountSkillPointsCompleted();
    s32 misc = func_002B19A0();
    s32 miscDone = misc > 9;
    *(s16 *)(D_0025E970 + 0x3E) = (sp1 > 0xe) ? 3 : 2;
    *(s16 *)(D_0025E970 + 0x4A) = (sp2 > 0x1d) ? 3 : 2;
    *(s16 *)(D_0025E970 + 0x56) = miscDone ? 10 : 2;
    *(s16 *)(D_0025E970 + 0x62) = miscDone ? 3 : 2;
    D_1AB338 = miscDone ? 0xc34 : 0xc37;
    D_1AB32C = (sp1 > 0xe) ? 0xc2f : 0xc35;
    D_1AB330 = (sp2 > 0x1d) ? 0xc30 : 0xc36;
    D_1AB334 = miscDone ? 0xc33 : 0xc37;
    if (*(s32 *)(D_001A7308 + 0x90) != 0) {
        *(s16 *)(D_0025E970 + 0x60) = 0;
    }
}
#endif

/* func_002DFF28: EU twin of USA func_002DFF68 — save-page byte accumulator:
 * when the menu block's save-page-active flag (+0x168) is set, add `amount` to
 * the running byte total (+0x16C) and commit via func_002A0CC0(handle, amount);
 * else
 * return 0. Matching arm stays INCLUDE_ASM; #else is the structure-exact model.
 * Word-verified vs USA func_002DFF68: identical modulo the block base (USA
 * g_menuScreenBlock -> EU D_001F0000+0x2840, +0x80) and the jal (func_002A1138
 * -> EU func_002A0CC0); the +0x168/+0x16C field offsets are byte-identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFF28);
#else
s32 func_002DFF28(s32 handle, s32 amount) {
    u8 *blk = D_001F0000 + 0x2840;
    if (*(s32 *)(blk + 0x168) == 0) {
        return 0;
    }
    *(s32 *)(blk + 0x16C) += amount;
    /* EU asm 002DFF40: $5 (`amount`) is copied to $6 in the beqz delay slot and
     * is never rewritten before the jal at 002DFF4C, so it still reaches
     * func_002A0CC0 as the second argument. Dropping it left $5 holding
     * garbage. ($4/`handle` is never written here either.) Derived from
     * asm/eu/nonmatchings/text/1D5488/func_002DFF28.s, not from the USA twin. */
    return func_002A0CC0(handle, amount);
}
#endif

/* func_002DFF60: EU twin of USA func_002DFFA0 — play a global sound (id,arg)
 * only when the gp-rel gate D_1ABDB8 is set. Matching arm stays INCLUDE_ASM;
 * #else is the structure-exact model. Word-verified vs USA func_002DFFA0:
 * identical modulo the gate reloc (D_1ABD48 -> EU D_1ABDB8, gp-rel) and the jal
 * (PlayGlobalSound -> EU func_002E6C28). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFF60);
#else
void func_002DFF60(s32 id, s32 arg) {
    if (D_1ABDB8 != 0) {
        func_002E6C28(id, arg, 0);
    }
}
#endif

/* Always-ready gate: return 1. */
s32 func_002DFF88(void) {
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFF90);

/* func_002DFFA0: EU twin of USA func_002DFFE0 — is the current menu screen the given
 * fixed screen instance? (pointer compare lowered to xor + sltiu). Returns 0/1.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_002DFFE0: screen instance D_00259438->D_00259458 (g_pCurrentMenuScreen named). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFFA0);
#else
s32 func_002DFFA0(void) {
    extern u8 D_00259458[];                            /* USA D_00259438 */
    return g_pCurrentMenuScreen[0] == (void *)D_00259458;
}
#endif

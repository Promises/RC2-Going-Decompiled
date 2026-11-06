#include "common.h"
#include "weapon.h"

/* ROM_DATA_ADDR(sym, addr): a ROM data table passed to a callee as an address.
 * The EE arm names the symbol, so the ROM's lui/addiu %hi/%lo pair (with its
 * relocations) is reproduced; native keeps the raw ROM address it always used,
 * as these tables have no native definition. */
#ifndef TARGET_NATIVE
#define ROM_DATA_ADDR(sym, addr) ((s32)(sym))
#else
#define ROM_DATA_ADDR(sym, addr) (addr)
#endif

/* EE_REG(r) binds a local register variable to EE register `r` where the
 * compiler colours a value differently from the ROM (REGISTER-PIN DEVICE,
 * RULING #8598: cc1 still emits every instruction, the pin only steers
 * allocation); natively a plain local, as "$3" is not a register name there. */
#ifndef TARGET_NATIVE
#define EE_REG(r) __asm__(r)
#else
#define EE_REG(r)
#endif

/*
 * text/1D54C0 — front-end / pause-menu screens band, part B (vaddr
 * 0x2D5540..0x2DFFFF). Carved out of the big text/1B21E8 asm tile as
 * ranked-carve-pipeline pick #5 ("menu-screens"); part A is text/1CA080.
 * This part holds the Galactic Map screen, the text-table streamer, the menu
 * background-image loader, and the screen-command (MenuScreenDoAction)
 * forwarding wrappers.
 *
 * Built at -O2 -G8 -fno-gcse (later SN cc1 TU model — see the part-A header for
 * the full -G8 extern-sizing rules). The s136os arm (S136OS_SLOT rows) compiles
 * with gcse on, S136EXTRA="" (RULING #9491 shape, task #2252, FACT #10232); the
 * 2.9 compile keeps -fno-gcse. Walls left as INCLUDE_ASM (per function):
 * the 8-byte-packed callee-save wall (2+ GPR saves), the delay-slot store
 * scheduling wall (a lone global store + return-0 the original keeps OUT of
 * the jr delay slot while cc1 sinks it in), pad-load reorder, register
 * coloring, switch/jtbl, and handwritten frameless stub fragments.
 */

/* 2D draw-batch begin/end fence used by every menu draw function. */
extern void Begin2dDrawBatch(s32 mode);
extern void End2dDrawBatch(void);

/* #else-only draw/save callees shared across multiple structure-model bodies;
 * guarded under TARGET_NATIVE so the matching (INCLUDE_ASM) build is untouched.
 * str is an int-width localized-string handle (GetLocalizedString returns int). */
#ifdef TARGET_NATIVE
extern void DrawFont1RightJustifiedLabel(s32 x, s32 y, u32 color, s32 str, s32 wrap);
extern void BuildSaveImage(void *dst);
/* Remaining unit-wide #else-only callee prototypes. Typed from this unit's own
 * call sites (C++ reads an empty () as (void), so the old K&R-style empty-paren
 * declarations do not compile as a .cpp unit); return-typed s32 where the value
 * is consumed, void where ignored. A string argument is the int-width
 * localized-string handle GetLocalizedString returns, as above. */
/* GS A+D reg-write: the DATA is a 64-bit register value. Type + widen it to u64
 * for the native/#else build (prevents silent truncation of bits >=32 if a
 * 64-bit-value fn like func_002DD450 is #else'd here); matching-build decl kept
 * verbatim (byte-neutral — no matched-caller codegen change). */
#ifdef TARGET_NATIVE
extern void AppendGsRegPacket(s32 regId, u64 value);
#else
extern void AppendGsRegPacket();
#endif
extern void ComputeAudioChannelMix(void);
extern s32  CountSkillPointsCompleted(void);
extern void DrawDebugString(s32 x, s32 y, u32 color, s32 str, s32 wrap);
extern void DrawFont1CenteredLabel(s32 x, s32 y, u32 color, s32 str, s32 wrap);
extern void DrawGlyphQuad(s32 x, s32 y, s32 w, s32 h, s32 u, s32 v, s32 uw, s32 uh, u64 color, u64 tex0);
extern void DrawHudIconQuadTiled(s32 icon, s32 x, s32 y, s32 w, s32 h, s32 alpha);
extern void DrawHudSpriteRotated(s32 xBits, f32 y, s32 wBits, s32 hBits, s32 angleBits, s32 a5, s32 a6, s32 tex);
extern void DrawStringFont1(s32 x, s32 y, u32 color, s32 str, s32 wrap);
extern void EnableInlineColorCodes(void);
extern void FadeOutToBlackBlocking(s32 frames);
extern void FillMemory32(s32 dst, u32 pattern, s32 nbytes);
extern s32  GatherActiveObjectives(s32 outIds, s32 outMask, s32 outVals, s32 wantValues);
extern s32  GetHudIconTex0(s32 iconIndex);
extern s32  GetLocalizedString(s32 id);
extern s32  GetMenuOverlayMode(void);
extern void GuiElementSetVisible(s32 elem, s32 show);
extern s32  GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void GuiListSetItemCount(s32 elem, s32 count);
extern void GuiListSetScrollPos(s32 elem, s32 pos);
extern void MapSetCurrentLevel(s32 level);
extern s32  MeasureFont2Text(s32 str, s32 wrap);
extern void PumpDialogVoiceSystem(s32 blocking);
extern s32  RequestGameStateChange(s32 stateId, s32 push, s32 argA, s32 argB, s32 outDoneFlag);
extern void RequestLevelExit(s32 destination, s32 commitSave);
extern s32  StartFileLoad(s32 dest, s32 lbn, s32 sectors);
extern void StopFileLoad(void);
extern s32  UpdateLevelObjectiveStates(void);
extern void func_00131A98(void *clock);
extern void func_00132938(s32 flag);
extern s32  func_0026F7D0(void);
extern s32  func_0026F7D8(void);
extern void func_0027F7A0(void);
extern void func_00280090(s32 a, s32 b, s32 c, s32 d, s32 e);
extern s32 func_002801B8(s32 x, s32 y, u32 color, s32 str, s32 wrap);
extern void func_00283460(void *dst, const void *src, s32 nbytes);
extern void func_00286138(s32 a, s32 b);
extern void func_002861D8(s32 a, s32 b);
extern s32  func_00288898(void);
extern void func_002888A8(void);
extern void func_002888D0(void);
extern s32  func_0028EDF0(s32 name, s32 level);
extern void func_002904B0(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 mode);
extern void func_00297FA0(s32 saveRegion);
extern void func_00298A00(void);
extern void func_00299BF8(void);
extern s32  func_0029D248(s32 arg);
extern void func_0029D918(s32 arg);
extern s32  func_002B1D40(void);
extern void func_002CA980(void);
extern void func_002CAB90(s32 bits);
struct MenuCmd;
extern s32  func_002D6B00(struct MenuCmd *cmd);  /* locally defined below */
extern s32  GetMenuWorkBufferSize(s32 id);      /* locally defined below */
extern void func_002DFF68(s32 handle, s32 amount);  /* locally defined below */
extern s32  func_002E0010(char *dst, s32 level);
extern void func_003017F8(s32 glyph, s32 color, f32 x, f32 y, f32 scale, f32 a5, f32 a6);
extern void func_0033A7A8(void *p);
extern void func_00342450(void *p, s32 a, s32 b, s32 c);
extern s32  func_00342468(void *p);
extern s32  func_003424C8(void *p);
extern void func_00342520(void *p, s32 mode);
extern void func_00342BE8(void *p, s32 v);
extern s32  func_00342D68(void *p);
extern void func_00342DC0(void *p, s32 records);
extern void func_00343290(void *p, s32 records);
extern void func_003432B8(void *p, s32 v);
extern s32  func_003432C0(void *p);
extern void func_0034EF68(void *base, s32 index, s32 x, s32 y, s32 shade);
extern s32  func_0034F300(void *mgr);
extern s32 sceCdReadClock(void *clock);
#endif

/* Also called from the EE-compiled func_002DECE0 (cheat-code unlocks), so these
 * two are declared for both arms: C++ has no implicit declarations. */
extern void MarkLevelAvailable(s32 level);
extern s32  func_002B1880(s32 stringId, s32 arg);

/* Per-screen draw helpers in the preceding text/1A00F0 asm band. */
extern void func_0029D958(void);
extern void func_0029D8E8(void);
extern void func_0029D218(void);
extern void func_0029DD10(void);
extern void func_0029D1A8(void);

/* Per-frame GUI-list tick/input handlers (text/1A00F0 band): each takes the
 * pad pressed-mask in $a0 (the asm loads g_padButtonsPressed[0] into a0 before
 * the call). Without the arg, cc1 forwards garbage d-pad bits → phantom cursor
 * nav → spurious UI sound. */
extern s32 func_0029D398(s32 padPressed);
extern s32 func_0029D8A8(s32 padPressed);

/* Forwarding-wrapper target. */
extern void func_002DF1B8(s32 mode);

/* Front-end / pause screen-action dispatcher (op, arg, out-flag ptr). */
extern s32 MenuScreenDoAction(s32 op, s32 arg, void *outFlag);

/* A menu command record: opcode at +2 (s16), arg at +4 (s32). */
typedef struct MenuCmd {
    u8  _pad0[2];
    s16 op;     /* 0x2 */
    s32 arg;    /* 0x4 */
} MenuCmd;

/* Size pin (byte-neutral). func_002D6AD8/func_002D6B00 read only op@0x2 (lh) and
 * arg@0x4 (lw), forwarding both to MenuScreenDoAction. The minimal bindable
 * record is 0x8 (callers in this TU build it in a 0x10 stack scratch, but the
 * named type itself is only ever touched at +2/+4). */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(MenuCmd) == 0x8, "MenuCmd bindable record 0x8");
_Static_assert(__builtin_offsetof(MenuCmd, op)  == 0x2, "op");
_Static_assert(__builtin_offsetof(MenuCmd, arg) == 0x4, "arg");
#endif

/* ------------------------------------------------------------------------
 * MenuWidget — the per-screen menu content widget (the `obj` passed to every
 * per-widget tick/draw/input handler in this unit). NOT the screen-manager
 * object: that is the distinct, larger record reached through
 * g_pCurrentMenuScreen[0] (fields +0xE0 next-screen, +0xE8 focused-widget,
 * +0x128 dirty flag, +0x12C commit flag). A MenuWidget is the focused-widget
 * value stored at screen+0xE8; the 25 handlers below all operate on one.
 *
 * It is a polymorphic base whose low region is shared by every subtype and
 * whose high region (>=0x44) is reused as per-subtype overlay storage (the
 * streaming-image state machine, the objectives-level array, the slider state,
 * etc.). Because the bodies access it with raw (char*)obj + 0x.. casts and the
 * high region means different things per subtype, the type is intentionally
 * OPAQUE (a sized byte blob): the tester only needs a correct allocation SIZE
 * to bind a live instance, and over-allocating is harmless.
 *
 * Confirmed touched offsets (this unit; CONFIRMED = read+write seen in asm):
 *   +0x10  u32  base flags (func_002D8E60 ORs bit 4 = reload-request) CONFIRMED
 *   +0x18  s32  rect origin x        CONFIRMED (func_002DC800 / func_002DC940)
 *   +0x1C  s32  rect origin y        CONFIRMED
 *   +0x20  s32  rect width           CONFIRMED (many draw handlers)
 *   +0x24  s32  rect height          CONFIRMED
 *   +0x30  u32  subtype state/flags  CONFIRMED (UpdateMenuResourceStream switch driver)
 *   +0x34  ptr  data/cmd/row table   CONFIRMED (cmd table for func_002D6AA0)
 *   +0x38  s32  selected-row/scroll  CONFIRMED (func_002DC0F0/378/520)
 *   +0x3C  s32  sub-cursor / uv x    CONFIRMED (func_002DAE70/DB080)
 *   +0x40  s32  cursor / selection   CONFIRMED (func_002D6AA0 row, SoundOptionsMenuInput)
 *   +0x44  s32  subtype state / level-array base CONFIRMED
 *   +0x48  s32  map-slot id A        CONFIRMED (func_002DB028/DD7E8/DD858)
 *   +0x4C  s32  map-slot id B        CONFIRMED
 *   +0x50  s32  cached state / slot  CONFIRMED
 *   +0x54  s32  cached state / slot  CONFIRMED (func_002DC838/878/CCC8)
 *   +0x58  s32  cached state / uv    CONFIRMED (func_002DAE70/DB080)
 *   +0x5C  s32  stream frame counter CONFIRMED (TickStreamedImageWidget/UpdateMenuResourceStream)
 *   +0x60  s32  stream scroll offset CONFIRMED (func_002DA358/TickStreamedImageWidget/UpdateMenuResourceStream)
 *   +0x44..0xA0  s32[24] level/objective id array (objectives subtype) CONFIRMED
 *   +0xA0  s32  active-objective count (func_002D8270)                  CONFIRMED
 *   +0xA4..0xBB  u8[24] per-level handled-flag array (func_002DA358)    CONFIRMED
 * Gaps (e.g. +0x00..0x10, +0x28..0x30, +0x64..0xA0 in non-objectives subtypes)
 * are left as opaque padding — never invented as named fields.
 *
 * SIZE: the widest confirmed access is the +0xA4 handled-flag byte array,
 * touched through index 0x17 => last byte +0xBB. We size the bindable record at
 * 0xC0 (16-aligned, the next PS2-natural boundary above 0xBB). This is a SAFE
 * LOWER BOUND for allocation, not a claimed exact ctor sizeof (the exact ctor
 * was not located; the 25 handlers never touch beyond +0xBB). UNCONFIRMED that
 * the true object is exactly 0xC0 — it may be larger; it is never smaller.
 * ------------------------------------------------------------------------ */
typedef struct MenuWidget {
    u8 _bytes[0xC0];
} MenuWidget;

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(MenuWidget) == 0xC0, "MenuWidget bindable record 0xC0");
#endif

/* gp-addressable globals read via the assembler-absolute macro (see header). */
__asm__(".extern D_1A7BA8, 16");
__asm__(".extern g_sndChannelVolumes, 16");
extern s32 D_1A7BA8;              /* saved sound-channel volume snapshot */
extern s32 g_sndChannelVolumes[]; /* sound-channel volume table */

/* Newly-pressed digital pad button mask (edge-detected this frame). Modelled
 * as an array so the -G8 cc1 emits the explicit lui/lw address form. */
extern s32 g_padButtonsPressed[];
/* Galactic-map planet-select input handler in the preceding asm band. */
extern void func_0029DD40(s32 buttons);

/* An object the original addresses with a COMPILER-split lui/%lo pair (the high
 * half in its own register, other instructions scheduled between): a named
 * section other than .sdata/.sbss takes it out of cc1's -G8 small-data class so
 * cc1 splits the address itself instead of emitting the one-insn macro. */
#define ROM_SPLIT __attribute__((section(".data")))

/* Player character/control mode: 0=Ratchet, 1=Clank-solo, 2=Giant Clank.
 * Read with the compiler-split form (func_002DA330: lui $2; lw; lbu %lo($2)). */
extern u8 g_bPlayerMode ROM_SPLIT;

/* Map state block. The galactic-map slot table is 5 interleaved {id,flags}
 * pairs (stride 8) starting at +0x40 (id) / +0x44 (flags).
 * Modelled cc1-small / assembler-absolute (see g_playerProgress): the 8-byte
 * extent puts it in cc1's -G8 small-data class, so cc1 emits each slot-table
 * address as one `la` macro instead of splitting (and CSE-ing) a lui/%lo pair,
 * and the `.extern ,16` makes the assembler expand that macro to the absolute
 * lui/addiu pair the ROM carries (GetMenuWorkBufferSize, func_002DF560,
 * func_002DF5B0). The 8 is NOT the object's size - the block runs to at least
 * +0x170 (text/1DFF80) - so never take sizeof() of it. */
__asm__(".extern D_001B1E90, 16");
extern u8 D_001B1E90[8];

/* Front-end / pause-menu screen-manager pointers (array-modelled for the
 * explicit lui/lw address form). */
extern void *g_pCurrentMenuScreen[];  /* active screen instance */
extern void *g_pNextMenuScreen[];     /* requested next screen instance */
extern u8 D_25E660[];                 /* a specific menu-screen instance */

/* Menu-subsystem state block — a view of g_menuScreenBlock (0x1F27C0; the
 * only symbol the link holds for that address). Only the Galactic-Map
 * save-page fields are modelled here; the rest is opaque padding. */
typedef struct MenuState {
    u8  _pad0[0x168];
    s32 savePageActive;  /* 0x168 - nonzero while the save page is up */
    s32 savePageBytes;   /* 0x16C - running byte counter for the save */
} MenuState;
/* Takes TWO args: $4 = moby table base, $5 = count. $5 is live-in — verified in
 * asm/usa/nonmatchings/text/1A00F0/func_002A1138.s, where `daddu $16,$5,$0` at
 * 002A1144 reads $5 before anything writes it. The 2-arg form is already
 * declared correctly at usa/text/24D728.c:998; the definition types the first
 * parameter as `void *tableBase` (usa/text/1A00F0.c:938) — kept as s32 here
 * because this unit carries the value as an opaque handle. */
extern s32 func_002A1138(s32 tableBase, s32 count);

/* Galactic-map cache state (array-modelled for the explicit address form). */
extern s32 D_25BA60[];        /* map upload sequence counter */
extern s32 g_mapActiveSlot[]; /* active map cache slot index */

/* Persistent save block; its first word seeds a few rotating lookups. Only the
 * first word is read here, with the adjacent lui/%lo macro shape (cc1-small /
 * assembler-absolute, see the part-A header) — hence a scalar plus the size
 * override. */
__asm__(".extern g_playerProgress, 16");
extern s32 g_playerProgress;
extern s32 D_260570[];        /* 19-entry lookup table */

/* Galactic-map planet-row builder state. */
__asm__(".extern D_1A7C0C, 16");   /* read with the adjacent lui/%lo macro shape */
extern s32 D_1A7C0C;          /* number of active planet rows */
extern u8  D_18D0E8[];        /* per-row source index (reversed) */
extern u32 D_254E48[];        /* index -> 4-byte record (icon id in low half) */
typedef struct MapPlanetRow { /* 12-byte display record */
    u16 icon;   /* 0x0 */
    u16 flag;   /* 0x2 - set to 1 for active rows */
    u8  _pad[8];
} MapPlanetRow;
extern MapPlanetRow D_25AD90[];

/* --- Additional externs used by the bodies below ------------------------- */
/* Per-screen draw helper (preceding asm band) keyed off the pressed buttons. */
extern void func_0029D1D8(s32 buttons);
/* Caller view of the (x0,x1,y0,y1) rect hook. The ROM body is empty (`jr $ra;
 * nop`, text/188858.c's definition), so nothing is submitted. */
extern void func_002897B0(s32 x0, s32 x1, s32 y0, s32 y1);
/* UI/system sound trigger from the global sound-def pool. */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
/* Map-slot toggle/lookup helpers (defined later in this unit). */
extern s32 FreeMenuWorkBuffer(s32 id);
extern s32 AllocMenuWorkBuffer(s32 forceSet);
/* Menu-sound gate (nonzero => func_002DFFA0 plays the sound); g_menuSoundEnabled. */
__asm__(".extern g_menuSoundEnabled, 4");
extern s32 g_menuSoundEnabled;

/* --- Shared globals referenced by the TARGET_NATIVE #else bodies below. ----
 * Declarations only; they do not affect the matching (INCLUDE_ASM) build.
 * Functions called from those bodies are left implicit (resolved at link). */
extern s32 g_padButtonsHeld;       /* 0x138340 currently-held button mask */
extern s32 g_nMapCurrentLevel;     /* 0x... selected galactic-map level */
extern s32 g_nMapActiveSlot;       /* active map cache slot, -1 = none */
extern s32 g_nFileLoadState;       /* 0x1A63AC CD file-load busy flag */
extern s32 g_nMusicVolume;         /* 0x1A7BA8 audio-options music slider */
extern s32 g_nSfxVolume;           /* 0x1A7BA4 audio-options sfx slider */
extern s32 g_nAudioStereoMode;     /* 0x1A7BA0 audio-options mono/stereo */
extern u8  g_skillPointFlags[];    /* 0x1A7A68 per-skill-point complete flag */
extern u8  g_inventoryOwned[];     /* inventory have-flags */
extern void *g_pGuiManager;        /* GUI subsystem root */
extern u8  g_bCurrentLanguage;     /* active language index */
extern s32 g_nGlobalWadBaseLbn;    /* base LBA for global assets */
extern s32 D_001F28F4;             /* menu transition-pending override flag */
extern s32 D_001F28DC;             /* text-table second-bank base address */
extern s32 D_001F28A4;             /* memory-card status code */
extern u8  D_001ABD48;             /* (same address as g_menuSoundEnabled, byte view) */
__asm__(".extern g_nSaveLoadStatusCode, 16");   /* adjacent lui/%lo macro shape in the ROM */
extern s32 g_nSaveLoadStatusCode;  /* 0x1A7420 save/load popup status code */
extern u8  g_menuScreenBlock[];    /* g_particleFxBlob+0x100 menu-screen mgr block */
extern s32 g_playerProgress2;      /* alias for the persistent save block first word */
extern void func_002DFFA0(s32 id, s32 arg);  /* gated menu-sound helper (below) */
extern u8 *g_pTextTableLoadBuf;    /* 0x1F28D8 language text-table load buffer */

/* Build the ship-customization screen's moby set. Decodes the g_shipCustomization
 * bitfield into ship model / paint / detail selectors, allocates the moby array
 * (AllocMenuWorkBuffer(1), stored at g_menuScreenBlock+0x1CC, stride 0x100), then spawns
 * the fixed base parts (slots 0-5) plus optional detail/paint parts (slots 6+),
 * each InitMobyFromClass'd from a per-selector class-id table (D_1A8C28..58) or a
 * literal class, tagged with a per-part role byte at +0xBC and (for the mirrored
 * parts) the +0x34 0x8000 flag. A tuning pass then stamps shared render state on
 * every spawned moby (colour via func_002A12A0, alpha/scale/anim-rate fields,
 * UpdateMobyBSphereAndGrid), and finally the current paint selection is matched
 * against the D_26CFD8 table (stride 0x14 {mask,value}) to seed g_shipCustomizeCursor.
 * g_menuScreenBlock+0x1D0 = spawned count, +0x1D4 = 0x12C. Returns 0.
 *
 * Faithful transcription (offsets/class-ids/shift masks verbatim from asm); the
 * moby / class-table structs are only partially recovered so raw offsets are kept.
 * Matching arm stays INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5540);
#else
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 43.34% / engine96 34.45%; better arm sdk29; 367 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-128` vs `addiu sp,sp,-160`. Not iterated in t495.
 * Save stride re-measured on the s136os arm (SN 1.36 -fopt-stack): saves land at the ROM's
 * 8-byte stride, but the saved register set/count differs from the ROM's (NOTE #9871) - an
 * allocation/frame difference, not the stride. So the save-slot wall named above is a cc1 2.9
 * property and was measured false as the reason this member stays unmatched (FACT #9873).
 * Residual: the saved-register set; the rest UNMEASURED. */
s32 func_002D5540(void) {
    extern s32 g_shipCustomization;      /* PlayerStats+0xF8 ship-customize bitfield */
    extern s32 g_shipCustomizeCursor;    /* selected paint index (gp-rel) */
    /* Per-selector class-id tables (s16 entries). */
    extern s16 D_1A8C28[], D_1A8C30[], D_1A8C38[], D_1A8C40[], D_1A8C48[], D_1A8C50[], D_1A8C58[];
    extern u8  D_26CFD8[];   /* paint-match table, stride 0x14 {s32 mask, s32 value} */
    extern u8  D_0025D458[];
    extern void func_002CAFD8(void);                                  /* prime the moby scratch subsystem */
    extern void InitMobyFromClass(void *moby, s32 classId);           /* spawn a moby from a class id */
    extern void func_002A12A0(void *moby, s32 color, s32 a, s32 b, s32 c); /* moby colour/render tuning */
    extern void UpdateMobyBSphereAndGrid(void *moby);                 /* refresh bounding sphere + grid cell */
    u8 *mgr = (u8 *)g_menuScreenBlock;
    u8 *base;
    u8 *m;
    s32 sc, n, i, cursor;
    s16 cls;

    func_002CAFD8();

    sc = g_shipCustomization;
    base = (u8 *)AllocMenuWorkBuffer(1);
    *(void **)(mgr + 0x1CC) = base;

    /* --- Fixed base parts (slots 0-5). --- */
    InitMobyFromClass(base, 0xD54);
    base[0xBC] = 9;

    m = base + 0x100;
    InitMobyFromClass(m, D_1A8C28[(sc >> 2) & 0x3]);
    m[0xBC] = 8;

    m = base + 0x200;
    InitMobyFromClass(m, D_1A8C28[(sc >> 2) & 0x3]);
    m[0xBC] = 8;
    *(u16 *)(m + 0x34) |= 0x8000;

    m = base + 0x300;
    InitMobyFromClass(m, D_1A8C30[(sc >> 4) & 0x1]);
    m[0xBC] = 0;

    m = base + 0x400;
    InitMobyFromClass(m, D_1A8C38[(sc >> 6) & 0x1]);
    m[0xBC] = 1;

    m = base + 0x500;
    InitMobyFromClass(m, D_1A8C38[(sc >> 6) & 0x1]);
    m[0xBC] = 1;
    *(u16 *)(m + 0x34) |= 0x8000;

    /* --- Optional / counted parts (slot n, n starts past the 6 fixed slots). --- */
    n = 6;
    cls = D_1A8C40[(sc >> 7) & 0x3];
    if (cls != 0) {
        m = base + n * 0x100;
        InitMobyFromClass(m, cls);
        m[0xBC] = 2;
        n++;
    }

    m = base + n * 0x100;
    InitMobyFromClass(m, D_1A8C48[(sc >> 9) & 0x1]);
    m[0xBC] = 3;
    n++;

    m = base + n * 0x100;
    InitMobyFromClass(m, D_1A8C48[(sc >> 9) & 0x1]);
    m[0xBC] = 3;
    *(u16 *)(m + 0x34) |= 0x8000;
    n++;

    m = base + n * 0x100;
    InitMobyFromClass(m, D_1A8C50[(sc >> 10) & 0x3]);
    m[0xBC] = 4;
    n++;

    m = base + n * 0x100;
    InitMobyFromClass(m, D_1A8C50[(sc >> 10) & 0x3]);
    m[0xBC] = 4;
    *(u16 *)(m + 0x34) |= 0x8000;
    n++;

    if ((sc >> 12) & 0x1) {
        m = base + n * 0x100;
        InitMobyFromClass(m, 0x10E2);
        m[0xBC] = 5;
        n++;
    }
    if ((sc >> 13) & 0x1) {
        m = base + n * 0x100;
        InitMobyFromClass(m, 0x10E4);
        m[0xBC] = 6;
        n++;
    }
    if ((sc >> 14) & 0x3) {
        m = base + n * 0x100;
        InitMobyFromClass(m, D_1A8C58[(sc >> 14) & 0x3]);
        m[0xBC] = 7;
        n++;
    }

    /* --- Shared render-state tuning pass over every spawned moby. --- */
    for (i = 0; i < n; i++) {
        m = base + i * 0x100;
        *(u16 *)(m + 0x32) = 0x1FF;
        func_002A12A0(m, 0x202020, 0xE, 0xE, 0);
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
        UpdateMobyBSphereAndGrid(m);
    }

    /* --- Seed the paint cursor from the D_26CFD8 {mask,value} match table. --- */
    *(s32 *)(mgr + 0x1D0) = n;
    *(s32 *)(mgr + 0x1D4) = 0x12C;
    g_shipCustomizeCursor = 0;
    cursor = 0;
    if ((sc & *(s32 *)(D_26CFD8 + 0)) != *(s32 *)(D_26CFD8 + 4)) {
        cursor = 1;
        while (cursor < 0x15) {
            if ((sc & *(s32 *)(D_26CFD8 + cursor * 0x14)) ==
                *(s32 *)(D_26CFD8 + cursor * 0x14 + 4)) {
                break;
            }
            cursor++;
        }
    }

    *(s32 *)(D_0025D458 + 0x3C) = -0x244;
    g_shipCustomizeCursor = cursor;
    return 0;
}
#endif

/* Toggle the map slot at g_particleFxBlob+0x100 +0x1CC via FreeMenuWorkBuffer and
 * store the result back. Returns 0. Wall (cc1 2.9): 2-GPR callee-save (8-byte-packed 0x10
 * frame). */
/* GUARD (task #1271): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D5A10)
S136OS_SLOT(func_002D5A10);
#else
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 98.85% / engine96 68.77%; better arm sdk29; 7 differing rows
 * on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing insn:
 * ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. Not iterated in t495. */
    /* MATCHED on the s136os arm (task #1271), not by cc1 2.9. */
s32 func_002D5A10(void) {
    u8 *blk = (u8 *)g_menuScreenBlock;
    *(s32 *)(blk + 0x1CC) = FreeMenuWorkBuffer(*(s32 *)(blk + 0x1CC));
    return 0;
}
#endif

/* ADDRESSING-MODEL DEVICES (RULING #8620 class; directive-only, emit nothing)
 * for the two title-screen glyph rows below, func_002D5A48 and func_002D5D10.
 * Each ROM read of these is the assembler's absolute macro (lui into the
 * destination, or lui $1 for an FPR load); gas forms that only when the
 * symbol's `.extern` size above the use exceeds -G8, and cc1 sizes each one
 * 4. At FILE SCOPE so the unit's 2.9 TU declares them too, and the s136os
 * splice does not carry the TU's end-of-file `, 4` lines in front of the
 * blocks (the same placement rule as the g_guiInstance device above
 * func_002D5EC8, which this repeats for the earlier rows). The gp-relative
 * siblings (D_1ABAD0/D8/E0, D_1ABB08/10/18/20) keep cc1's size. Each line is
 * load-bearing: removing one alone costs 22..103 of the row's words. */
#ifndef TARGET_NATIVE
__asm__(".extern g_guiInstance, 16");
__asm__(".extern g_swapGadgetItemIndex, 16");
__asm__(".extern D_1ABAD4, 16");
__asm__(".extern D_1ABADC, 16");
__asm__(".extern D_1ABAE4, 16");
__asm__(".extern D_1ABB0C, 16");
__asm__(".extern D_1ABB14, 16");
__asm__(".extern D_1ABB1C, 16");
/* An empty volatile fence on the pinned glyph scale (RULING #8483): it fixes
 * the scale's 1.0 between the x and y conversions, as the ROM issues it, and
 * keeps cc1 from folding the 1.0 into the argument copy. Empty natively, where
 * "f" is not a register constraint. */
#define GLYPH_SCALE_FENCE(x) __asm__ __volatile__("" : "+f"(x))
#else
#define GLYPH_SCALE_FENCE(x) ((void)0)
#endif

/* Draw the title-screen language-select glyph row: three packed-colour glyphs
 * (font-atlas keys 0x96/0x97/0x98) blitted at the base cursor (D_1ABAD8,
 * D_1ABADC), followed by two localized strings (textIds 0x2BF8, 0x2BE5) drawn at
 * base + per-string offset in colour 0x80F0F0F0. The whole row is skipped when
 * the GUI singleton is absent. Returns 0.
 *
 * Every glyph blit passes scale 1.0, the sprite y-fudge
 * (g_swapGadgetItemIndex+0x8E) and a 0.0 last argument; the GUI singleton and
 * the cursor are re-read for each call, as the ROM does. The first string is
 * drawn by func_002801B8 (DrawFont1CenteredLabel), the second by func_00280090
 * (DrawFont1RightJustifiedLabel).
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, task #1929). Each
 * piece priced by removing it alone (solo s136 screen, relocated fields
 * masked, words differing of 112, built length in brackets):
 *   - the `.extern ,16` devices above: g_guiInstance 103 (109),
 *     g_swapGadgetItemIndex 90 (109), D_1ABADC 93 (107), D_1ABAD4 41 (111),
 *     D_1ABAE4 23 (111);
 *   - scale pinned to $f14 (EE_REG, RULING #8598; DrawHelpTopicMenu's device in
 *     1CA080.cpp): the ROM rebuilds 1.0 into $f14 for every call, while cc1
 *     shares one 1.0 in a callee-saved FPR. 108 (115) without it;
 *   - GLYPH_SCALE_FENCE before the first blit: without it the pin's self-copy
 *     leaves a (use $f14) at the head of the block, and sched2 then issues the
 *     atlas add before the codepoint load, against the ROM. 4 without it; 2 as
 *     a non-volatile fence, 4 as an untied barrier;
 *   - the first x conversion formed before the scale: 8;
 *   - both text colours 64-bit (the ROM's ori/dsll/ori): 32 (110).
 * A literal 0.0f last argument compiles the same as the rotation local; the
 * local stays because it is what lives in $f20 across the calls. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D5A48)
S136OS_SLOT(func_002D5A48);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void func_003017F8(s32 glyph, s32 color, f32 x, f32 y, f32 scale, f32 a5, f32 a6);
extern s32 GetLocalizedString(s32 id);
#ifndef TARGET_NATIVE
/* The ROM symbols with their ROM signatures (as 1CA080.cpp declares them): the
 * colour is a 64-bit value, built zero-extended (ori/dsll/ori). Native keeps the
 * unit-wide 32-bit declarations: a second type for one extern "C" name does not
 * compile there. */
extern void func_002801B8(s32 x, s32 y, u64 color, s32 str, s32 wrap);
extern void func_00280090(s32 x, s32 y, u64 color, s32 str, s64 wrap);
#else
extern s32 func_002801B8(s32 x, s32 y, u32 color, s32 str, s32 wrap);
extern void func_00280090(s32 a, s32 b, s32 c, s32 d, s32 e);
#endif
/* (end of this body's declarations) */
s32 func_002D5A48(void) {
    /* GUI singleton; the font atlas lives at g_guiInstance + 0x8710. */
    extern char *g_guiInstance;
    /* Base cursor + per-string offsets for this row. */
    extern s32 D_1ABAD0, D_1ABAD4, D_1ABAD8, D_1ABADC, D_1ABAE0, D_1ABAE4;
    /* +0x8E holds the global sprite y-fudge (f32). */
    extern s32 g_swapGadgetItemIndex;
    f32 rotation;
    f32 x;
    register f32 scale EE_REG("$f14");
    s32 glyph;

    Begin2dDrawBatch(0);
    if (g_guiInstance != 0) {
        rotation = 0.0f;
        glyph = GuiFontAtlasLookupGlyph(g_guiInstance + 0x8710, 0x96);
        x = (f32)D_1ABAD8;
        scale = 1.0f;
        GLYPH_SCALE_FENCE(scale);
        func_003017F8(glyph, 0x60442D00, x, (f32)D_1ABADC, scale,
                      *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E), rotation);
        glyph = GuiFontAtlasLookupGlyph(g_guiInstance + 0x8710, 0x97);
        scale = 1.0f;
        func_003017F8(glyph, 0x60241700, (f32)D_1ABAD8, (f32)D_1ABADC, scale,
                      *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E), rotation);
        glyph = GuiFontAtlasLookupGlyph(g_guiInstance + 0x8710, 0x98);
        scale = 1.0f;
        func_003017F8(glyph, 0x55F0C070, (f32)D_1ABAD8, (f32)D_1ABADC, scale,
                      *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E), rotation);

        func_002801B8(D_1ABAD8 + D_1ABAD0, D_1ABADC + D_1ABAD4, 0x80F0F0F0,
                      GetLocalizedString(0x2BF8), -1);
        func_00280090(D_1ABAD8 + D_1ABAE0, D_1ABADC + D_1ABAE4, 0x80F0F0F0,
                      GetLocalizedString(0x2BE5), -1);
    }
    End2dDrawBatch();
    return 0;
}
#endif

/* Select the active language's menu-background image index from the gp-relative
 * table D_1AAA58, stash it on the screen object (+0x34), refresh the cached
 * language snapshot (D_1ABAE8) and bump the upload-sequence counter D_25BA60.
 * Returns 0. MATCHED: the 87.5% near-miss was a symbol-sizing artifact — the
 * gp-relative table D_1AAA58 must be modeled as a small (<=8B) sized array so
 * cc1 emits the addiu $28,%gp_rel base form; with that it is byte-exact. */
extern s32 D_25B9A0[];   /* current language index (lui/%lo) */
extern s32 D_1ABAE8;     /* cached language snapshot (gp-rel) */
extern s32 D_1AAA58[2];  /* per-language bg-index table base (gp-rel) */
s32 func_002D5C08(void *obj) {
    s32 lang = D_25B9A0[0];
    if (D_1ABAE8 != lang) {
        D_1ABAE8 = lang;
    }
    *(s32 *)((u8 *)obj + 0x34) = D_1AAA58[lang];
    D_25BA60[0] = lang + 1;
    return 0;
}

/* Reset the galactic-map upload sequence counter. Returns 0. */
s32 func_002D5C48(void) {
    D_25BA60[0] = 0;
    return 0;
}

/* Per-language menu-background readiness check + screen-back driver. Selects the
 * active language's bg image index (g_menuBgImageIndex = D_1ABAF0[language]); if
 * `focus` is not the manager screen's focused widget returns 0, else drives the
 * standard exit/back navigation from the pad block's newly-pressed mask
 * (D_138180+0x1C4): L1/R1 (0x900) returns 1 unless an override
 * (g_menuScreenBlock+0x134) is set; on back (0x10) copies the screen's
 * next-screen pointer (+0xE0) into g_menuScreenBlock+0x18 and returns 0, or,
 * with no next screen, returns -1 unless the override is set (0).
 * Returns 0/1/-1. The same back/exit prelude as func_002D75D8.
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack), task #2289.
 * gcse on (task #2252) gives the ROM's kept %hi copies of g_menuScreenBlock
 * and D_138180. Two addressing models, both measured: g_menuBgImageIndex is
 * compiler-split (ROM_SPLIT, the ROM's `lui $6` / `sw %lo($6)`), and
 * g_currentLanguage is cc1-small but assembler-absolute (the `.extern ,16`
 * at file scope just above): gas expands cc1's one-insn `lbu` macro into the
 * ROM's lui/lbu in the destination register. Read compiler-split instead, its
 * %hi takes another register and the prelude reschedules (19 of 45 words
 * differ). Every g_currentLanguage access in this unit's ROM is that macro
 * form, and no other EE-compiled function here touches it.
 * GUARD: compiled alone by SN 2.95.3 v1.36 -fopt-stack and spliced over the
 * S136OS_SLOT line (tools/ee/s136os_splice.sh); plain C on native. */
#ifndef TARGET_NATIVE
__asm__(".extern g_currentLanguage, 16");
#endif
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D5C58)
S136OS_SLOT(func_002D5C58);
#else
extern s32 D_1ABAF0[2];                      /* per-language bg image index */
extern s32 g_menuBgImageIndex ROM_SPLIT;
extern u8 g_currentLanguage;                 /* assembler-absolute, above */
extern u8 D_138180[];
typedef struct { char p00[0x14]; char *owner; int value; char p1c[0x118]; int fallback; } BgMenuRoot;
typedef struct { char reserved[0x1C4]; int flags; } BgPadState;
#define bgRoot (*(BgMenuRoot *)g_menuScreenBlock)
#define bgPad  (*(BgPadState *)D_138180)

s32 func_002D5C58(void *focus) {
    int status;

    g_menuBgImageIndex = D_1ABAF0[g_currentLanguage];
    if (*(void **)(bgRoot.owner + 0xE8) != focus)
        return 0;
    if ((bgPad.flags & 0x900) && bgRoot.fallback == 0)
        return 1;
    if (bgPad.flags & 0x10) {
        int next = *(int *)(bgRoot.owner + 0xE0);
        if (next != 0)
            bgRoot.value = next;
        else if (bgRoot.fallback == 0) {
            status = -1;
            goto finish;
        }
        status = 0;
finish:
        return status;
    }
    return 0;
}
#undef bgRoot
#undef bgPad
#endif

/* Draw the title-screen menu header glyph row: three packed-colour glyphs
 * (font-atlas keys 0x8B/0x8C/0x8D) blitted at the base cursor (D_1ABB10,
 * D_1ABB14), the first two with the shape parameter D_1ABB20 and the third
 * with 0.0f, then two localized strings (textIds 0x2BF7, 0x2BE5) at base +
 * per-string offset in colour 0x80F0F0F0, both drawn by func_002801B8
 * (DrawFont1CenteredLabel). Skipped when the GUI singleton is absent.
 * Returns 0.
 *
 * Sibling of func_002D5A48 over a different cursor/offset/colour set, and
 * matched the same way (s136os arm, task #1929), with the same devices: the
 * `.extern ,16` block above func_002D5A48, scale pinned to $f14,
 * GLYPH_SCALE_FENCE before the first blit, the first x formed before the scale,
 * 64-bit text colours. Each priced by removing it alone (solo s136 screen,
 * words differing of 109, built length in brackets): g_guiInstance 101 (106),
 * g_swapGadgetItemIndex 89 (106), D_1ABB14 92 (104), D_1ABB0C 40 (108),
 * D_1ABB1C 22 (108); the pin 109 (112); the fence 2 (4 non-volatile, 2 as an
 * untied barrier); the early x 8; the u64 colour 31 (107). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D5D10)
S136OS_SLOT(func_002D5D10);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern void func_003017F8(s32 glyph, s32 color, f32 x, f32 y, f32 scale, f32 a5, f32 a6);
extern s32 GetLocalizedString(s32 id);
#ifndef TARGET_NATIVE
/* The ROM symbol with its ROM signature: 64-bit colour (see func_002D5A48). */
extern void func_002801B8(s32 x, s32 y, u64 color, s32 str, s32 wrap);
#else
extern s32 func_002801B8(s32 x, s32 y, u32 color, s32 str, s32 wrap);
#endif
/* (end of this body's declarations) */
s32 func_002D5D10(void) {
    /* GUI singleton; the font atlas lives at g_guiInstance + 0x8710. */
    extern char *g_guiInstance;
    /* Base cursor + per-string offsets for this row. */
    extern s32 D_1ABB08, D_1ABB0C, D_1ABB10, D_1ABB14, D_1ABB18, D_1ABB1C;
    /* Glyph-blit shape parameter (gp-relative float, role unconfirmed). */
    extern f32 D_1ABB20;
    /* +0x8E holds the global sprite y-fudge (f32). */
    extern s32 g_swapGadgetItemIndex;
    f32 x;
    register f32 scale EE_REG("$f14");
    s32 glyph;

    Begin2dDrawBatch(0);
    if (g_guiInstance != 0) {
        glyph = GuiFontAtlasLookupGlyph(g_guiInstance + 0x8710, 0x8B);
        x = (f32)D_1ABB10;
        scale = 1.0f;
        GLYPH_SCALE_FENCE(scale);
        func_003017F8(glyph, 0x60442D00, x, (f32)D_1ABB14, scale,
                      *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E), D_1ABB20);
        glyph = GuiFontAtlasLookupGlyph(g_guiInstance + 0x8710, 0x8C);
        scale = 1.0f;
        func_003017F8(glyph, 0x55F0C070, (f32)D_1ABB10, (f32)D_1ABB14, scale,
                      *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E), D_1ABB20);
        glyph = GuiFontAtlasLookupGlyph(g_guiInstance + 0x8710, 0x8D);
        scale = 1.0f;
        func_003017F8(glyph, (s32)0x80FFDE8D, (f32)D_1ABB10, (f32)D_1ABB14, scale,
                      *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E), 0.0f);

        func_002801B8(D_1ABB10 + D_1ABB08, D_1ABB14 + D_1ABB0C, 0x80F0F0F0,
                      GetLocalizedString(0x2BF7), -1);
        func_002801B8(D_1ABB10 + D_1ABB18, D_1ABB14 + D_1ABB1C, 0x80F0F0F0,
                      GetLocalizedString(0x2BE5), -1);
    }
    End2dDrawBatch();
    return 0;
}
#endif

/* Seed obj->0x34 from a GUI subsystem query (g_guiInstance + 0x3C160) when the
 * GUI exists. Returns 0. */
#ifndef TARGET_NATIVE
/* ADDRESSING-MODEL DEVICE for func_002D5EC8's absolute lui form of g_guiInstance (emits no
 * code). At FILE SCOPE, not in the member's #else arm, so the unit's own 2.9 TU
 * declares the symbol too: tools/ee/s136os_splice.sh never carries an `.extern`
 * for a symbol the unit declares, so cc1's end-of-file small-size line from the
 * s136os TU is not carried in front of the block, where it would make the read
 * gp-relative (task #1387: the splice REFUSED the in-arm placement as ADDRESSING).
 * The unit's one bare 2.9 read of g_guiInstance (func_002D61D8's `lw`) already
 * follows the unit's own `.extern g_guiInstance, 16` above it and is absolute
 * either way, so nothing else changes. */
__asm__(".extern g_guiInstance, 16");
#endif
/* GUARD (task #1387): on EE the #else body below is the image's func_002D5EC8, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D5EC8)
S136OS_SLOT(func_002D5EC8);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_00342468(void *p);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 92.65% / engine96 69.71%; better arm sdk29; 9 differing rows
 * on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing insn:
 * ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. Not iterated in t495.
 * MATCHED on the s136os arm (task #1387; screened by task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
 * masked): EXACT 17/17, relocations equal, once
 * the body reads g_guiInstance through the `.extern ,16` absolute form below.
 * Without the `.extern` (name alone changed): 16/17, first diff @1 `lw $2,%gp_rel`
 * vs ROM `lui $2`. */
s32 func_002D5EC8(MenuWidget *obj) {
    extern char *g_guiInstance; /* the GUI singleton (alias g_pGuiManager) */

    if (g_guiInstance != 0) {
        *(s32 *)((u8 *)obj + 0x34) = func_00342468(g_guiInstance + 0x3C160);
    }
    return 0;
}
#endif

/* Ship-customization confirm screen input. If a forced exit is pending (overlay
 * mode 9) leave the level. Cancel(0x10) requests the parent; L1/R1 returns 1;
 * X(0x40) reads the focused GUI list item and forwards it via func_002D6B00. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5F10);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_00288898(void);
extern s32 GetMenuOverlayMode(void);
extern void func_002888A8(void);
extern void RequestLevelExit(s32 destination, s32 commitSave);
extern s32 func_003424C8(void *p);
extern s32 func_002D6B00(struct MenuCmd *cmd);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 61.57% / engine96 37.17%; better arm sdk29; 36 differing
 * rows on it, class STRUCTURAL; first differing insn: ROM `sd s1,24(sp)` vs
 * `lui v0,0x0  [HI16 0x00138344]`. Not iterated in t495. */
/* SCREEN (task #1395, s136 solo, relocated fields masked; not match evidence):
 * 51/69 edit 37 -> 23/69 edit 6, written in the ROM's shape: the pad mask read
 * off the controller block D_138180 (+0x1C4) at each test, the back path an
 * inlined MenuRequestParent(), one result variable. BLOCKED on one access: the
 * ROM reads g_guiInstance %gp_rel in a branch delay slot, but this unit's
 * file-scope `.extern g_guiInstance, 16` makes it absolute, so asm_unit hoists
 * the lui/lw pair (+1 word). With the gp-relative name g_pGuiManager the body
 * screens EXACT 69/69, differing only in that relocation's name, but EE has
 * no g_pGuiManager. The path is the 9..15 delay-slot size marker on that
 * directive, which touches matched code, or the ruling-blocked gp alias. */
#ifndef MENU_REQUEST_PARENT_DEFINED
#define MENU_REQUEST_PARENT_DEFINED
/* Back-button handling shared by the menu input handlers, inlined into each:
 * switch to the active screen's parent (+0xE0) if it names one (returns 0),
 * else leave the menu (-1) unless the override at g_menuScreenBlock+0x134
 * holds it open (0). */
static inline s32 MenuRequestParent(void) {
    u8 *blk = (u8 *)g_menuScreenBlock;
    u8 *parent = *(u8 **)(*(u8 **)(blk + 0x14) + 0xE0);
    if (parent != 0) {
        *(u8 **)(blk + 0x18) = parent;
    } else if (*(s32 *)(blk + 0x134) == 0) {
        return -1;
    }
    return 0;
}
#endif
s32 func_002D5F10(void) {
    extern char *g_guiInstance;      /* the GUI singleton (alias g_pGuiManager) */
    extern u8 D_138180[];            /* controller port-0 state; +0x1C4 = g_padButtonsPressed */
    u8 *pad;
    s32 result = 0;

    if (func_00288898() == 2 && GetMenuOverlayMode() == 9) {
        func_002888A8();
        RequestLevelExit(-1, 0);
        return 1;
    }
    func_002888A8();
    pad = D_138180;
    if (*(s32 *)(pad + 0x1C4) & 0x10) {
        return MenuRequestParent();
    }
    if (*(s32 *)(pad + 0x1C4) & 0x900) {
        result = 1;
    } else {
        func_0029D398(*(s32 *)(pad + 0x1C4));
        if ((*(s32 *)(pad + 0x1C4) & 0x40) && g_guiInstance != 0) {
            s32 item = func_003424C8(g_guiInstance + 0x3C160);
            s16 cmd[8];
            cmd[1] = *(s16 *)(item + 8);
            *(s32 *)(&cmd[2]) = *(s32 *)(item + 0xc);
            func_002D6B00((MenuCmd *)cmd);
        }
    }
    return result;
}
#endif

/* Draw the ship-customization back/help line: a right-justified label whose
 * Y follows the customization-panel scroll fraction (offset by 36px when a
 * sub-panel is open). Returns 0.
 * The 8-byte-slot prologue is the s136 arm's (SN 2.95.3 v1.36 -fopt-stack,
 * FACT #8810), so the 2.9 SAVE-SLOT wall does not bind it.
 * MATCHED on the s136os arm (task #1415): spliced, image cmp 0, and
 * verify_match_unit BYTE IDENTICAL 48/48 words with a base-seeded control.
 * SCREEN (task #1395, s136 solo, relocated fields masked + relocation compare;
 * the screen that selected it): EXACT 47/47, RELOC-EQUAL 9 (base: 36/47 @11
 * `c7800000|3c010000`). Four levers, each undone alone:
 *   - the fields read as absolute g_swapGadgetItemIndex+0x8A/+0x86 through
 *     `.extern ,16` (ADDRESSING-MODEL DEVICE): undone 35/47 @11, the base pair;
 *   - the int conversion written in each arm, adding -36.0f (the ROM converts
 *     twice): undone 26/47 @18;
 *   - the colour passed 64-bit (the ROM's dli): undone 15/47 @32;
 *   - the call named by its ROM symbol func_00280090: undone, EXACT but a
 *     relocation names DrawFont1RightJustifiedLabel, which EE does not define. */
#ifndef TARGET_NATIVE
/* ADDRESSING-MODEL DEVICE (RULING #8620 class, as 1CA080.cpp's): the ROM reads
 * the panel scroll fraction and the sub-panel flag as absolute
 * g_swapGadgetItemIndex+0x8A / +0x86, each address formed afresh by the
 * assembler's macro (lui $1 / lui $2). Emits nothing. It sits at FILE SCOPE,
 * not inside func_002D6028's #else arm, so that the unit's own 2.9 TU declares
 * the symbol too: tools/ee/s136os_splice.sh never carries an `.extern` for a
 * symbol the unit already declares, so the s136os TU's end-of-file `, 4` line
 * is not carried in front of the block, where it would make both reads
 * gp-relative (task #1399: the splice REFUSED the in-arm placement as
 * ADDRESSING g_swapGadgetItemIndex). No 2.9 code in this unit names
 * g_swapGadgetItemIndex, so nothing else changes. */
__asm__(".extern g_swapGadgetItemIndex, 16");
#endif
/* GUARD (task #1415): on EE the #else body below is the image's func_002D6028, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D6028)
S136OS_SLOT(func_002D6028);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 GetLocalizedString(s32 id);
#ifndef TARGET_NATIVE
/* The ROM symbol, with its ROM signature (as 188858.c and 1CA080.cpp declare
 * it): the colour is a 64-bit value, loaded with a dli (ori/dsll/ori). EE has
 * no definition under the DrawFont1RightJustifiedLabel name. */
extern void func_00280090(s32 x, s32 y, u64 color, s32 str, s64 wrap);
#define DRAW_RIGHT_LABEL func_00280090
/* Read absolute through the file-scope `.extern g_swapGadgetItemIndex, 16`
 * device above the guard. */
extern s32 g_swapGadgetItemIndex;
#define PANEL_SCROLL  (*(float *)((u8 *)&g_swapGadgetItemIndex + 0x8A))
#define PANEL_SUBOPEN (*(s32 *)((u8 *)&g_swapGadgetItemIndex + 0x86))
#else
extern void DrawFont1RightJustifiedLabel(s32 x, s32 y, u32 color, s32 str, s32 wrap);
#define DRAW_RIGHT_LABEL DrawFont1RightJustifiedLabel
extern float D_001B2324;   /* == g_swapGadgetItemIndex+0x8A */
extern s32   D_001B2320;   /* == g_swapGadgetItemIndex+0x86 */
#define PANEL_SCROLL  D_001B2324
#define PANEL_SUBOPEN D_001B2320
#endif
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 70.72% / engine96 59.17%; better arm sdk29; 29 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. Not iterated in t495. */
/* Faithful vs asm: value*329+0.5, -36 when the sub-panel flag is 1,
 * GetLocalizedString(0x2BE5) label draw. */
s32 func_002D6028(void) {
    extern void  func_0029D368(void);   /* per-frame customization-panel helper */
    float yf;
    s32 y;
    Begin2dDrawBatch(0);
    func_0029D368();
    yf = PANEL_SCROLL * 329.0f + 0.5f;
    if (PANEL_SUBOPEN == 1) {
        y = (s32)(yf + -36.0f);
    } else {
        y = (s32)yf;
    }
#undef PANEL_SCROLL
#undef PANEL_SUBOPEN
    DRAW_RIGHT_LABEL(0x1c7, y, 0x80f0f0f0, GetLocalizedString(0x2be5), -1);
#undef DRAW_RIGHT_LABEL
    End2dDrawBatch();
    return 0;
}
#endif

/* Enter the ship-customization screen: prime the system, then (if the GUI
 * exists) bind its three data sources to the customization list and show it. */
#ifndef TARGET_NATIVE
/* ADDRESSING-MODEL DEVICE for func_002D60E8's absolute lui form of g_guiInstance (emits no
 * code). At FILE SCOPE, not in the member's #else arm, so the unit's own 2.9 TU
 * declares the symbol too: tools/ee/s136os_splice.sh never carries an `.extern`
 * for a symbol the unit declares, so cc1's end-of-file small-size line from the
 * s136os TU is not carried in front of the block, where it would make the read
 * gp-relative (task #1387: the splice REFUSED the in-arm placement as ADDRESSING).
 * The unit's one bare 2.9 read of g_guiInstance (func_002D61D8's `lw`) already
 * follows the unit's own `.extern g_guiInstance, 16` above it and is absolute
 * either way, so nothing else changes. */
__asm__(".extern g_guiInstance, 16");   /* the GUI singleton, absolute lui/lw form */
#endif
/* GUARD (task #1387): on EE the #else body below is the image's func_002D60E8, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D60E8)
S136OS_SLOT(func_002D60E8);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_002888A8(void);
extern void func_00342450(void *p, s32 a, s32 b, s32 c);
extern void func_00342520(void *p, s32 mode);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 73.54% / engine96 85.18%; better arm engine96; 10 differing
 * rows on it, class STRUCTURAL; first differing insn: ROM `lui a0,0x0  [HI16
 * 0x001A8D04]` vs `lw a0,0(gp)  [GPREL16 g_pGuiManager]`. Not iterated in
 * t495. */
/* MATCHED on the s136os arm (task #1387; screened by task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
 * masked): EXACT 28/28, relocations equal, with
 * g_guiInstance read through `.extern ,16` below (this body sits before the
 * unit's file-scope one; without it: 23/28, first diff @5 `lw $4,%gp_rel`) and
 * the three tables named (ROM_DATA_ADDR; as literals: 4/28, first diff @9). */
extern u8 D_2615D8[];
extern u8 D_261678[];
extern u8 D_261730[];
s32 func_002D60E8(void) {
    extern char *g_guiInstance; /* the GUI singleton (alias g_pGuiManager) */

    func_002888A8();
    if (g_guiInstance != 0) {
        func_00342450(g_guiInstance + 0x3C160, ROM_DATA_ADDR(D_2615D8, 0x2615d8),
                      ROM_DATA_ADDR(D_261678, 0x261678), ROM_DATA_ADDR(D_261730, 0x261730));
        func_00342520(g_guiInstance + 0x3C160, 1);
    }
    return 0;
}
#endif

/* Feed this frame's newly-pressed buttons to the planet-select handler.
 * Returns 0. MATCHED (the earlier "delay-slot scheduling wall" note was a
 * misdiagnosis: cc1 keeps the jal slot as nop here, byte-exact). */
s32 func_002D6158(void) {
    func_0029DD40(g_padButtonsPressed[0]);
    return 0;
}

/* Same planet-select input forward as func_002D6158 (sibling screen). MATCHED. */
s32 func_002D6180(void) {
    func_0029DD40(g_padButtonsPressed[0]);
    return 0;
}

/* Draw-batch wrapper. */
s32 func_002D61A8(void) {
    Begin2dDrawBatch(0);
    func_0029DD10();
    End2dDrawBatch();
    return 0;
}

/* GUI list show/hide for the save panel keyed off which data-source object was
 * passed: &D_1AB098 -> show (func_0033A7E0), &D_1AB068 -> hide (func_0033A860),
 * both on the save list at g_guiInstance + 0x3CEA0. Returns 0. MATCHED. The
 * prior near-miss compared `which` to the literal ints 0x1ab098/0x1ab068; the
 * asm actually compares it to the gp-relative ADDRESSES of those globals. */
__asm__(".extern g_guiInstance, 16");
extern s32 D_1AB098;   /* save-data source object A (gp-rel; address taken) */
extern s32 D_1AB068;   /* save-data source object B (gp-rel; address taken) */
extern char *g_guiInstance;
extern void func_0033A7E0(void *list);
extern void func_0033A860(void *list);
s32 func_002D61D8(void *which) {
    if (g_guiInstance != 0) {
        if (which == &D_1AB098) {
            func_0033A7E0(g_guiInstance + 0x3CEA0);
        } else if (which == &D_1AB068) {
            func_0033A860(g_guiInstance + 0x3CEA0);
        }
    }
    return 0;
}

/* return 0 stub. */
s32 func_002D6240(void) {
    return 0;
}

/* Save/load list confirm input. Sets a popup-state bit (D_1A7424), then: cancel
 * (0x10) requests the parent; L1/R1 returns 1; otherwise runs the per-frame list
 * tick and, on X(0x40) with the GUI up, reads the focused entry and (when the
 * card status is mid-write, code 6/0xD, item type 5) flips to overwrite mode
 * before forwarding via func_002D6B00. Returns 0/1/-1.
 * Wall: deep nested branch ladder + popup-state bit mutation. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", SaveMessageWidgetTick);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_0029D918(s32 arg);
extern s32 func_00342D68(void *p);
extern s32 func_002D6B00(struct MenuCmd *cmd);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 59.25% / engine96 52.86%; better arm sdk29; 65 differing
 * rows on it, class STRUCTURAL; first differing insn: ROM `lui v1,0x0  [HI16
 * 0x001A7424]` vs `lw a0,0(gp)  [GPREL16 D_001A7424]`. Not iterated in t495. */
    /* TODO(match): functional equivalent - not byte-exact. */
s32 SaveMessageWidgetTick(void) {
    extern s32 D_001A7424;
    s32 pressed = g_padButtonsPressed[0];
    D_001A7424 = (D_001A7424 & ~4) | 2;
    if (pressed & 0x10) {
        void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && D_001F28F4 == 0) return -1;
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    if (pressed & 0x900) {
        return 1;
    }
    func_0029D918(g_padButtonsPressed[0]);
    if ((pressed & 0x40) && g_pGuiManager != 0) {
        s32 item = func_00342D68((u8 *)g_pGuiManager + 0x3E9C8);
        s16 op = *(s16 *)(item + 8);
        s16 cmd[8];
        cmd[1] = op;
        *(s32 *)(&cmd[2]) = *(s32 *)(item + 0xc);
        if ((g_nSaveLoadStatusCode == 6 || g_nSaveLoadStatusCode == 0xd) && op == 5) {
            D_001A7424 &= ~2;
            g_nSaveLoadStatusCode = 0xc;
        }
        func_002D6B00((MenuCmd *)cmd);
    }
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D6380(void) {
    Begin2dDrawBatch(0);
    func_0029D958();
    End2dDrawBatch();
    return 0;
}

/* Enable + bind a GUI save/load list panel (g_pGuiManager + 0x3E9C8) to its
 * data source (0x261520) when the GUI exists. Returns 0. */
/* GUARD (task #1387): on EE the #else body below is the image's func_002D63B0, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D63B0)
S136OS_SLOT(func_002D63B0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_00342BE8(void *p, s32 v);
extern void func_00342DC0(void *p, s32 records);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 85.86% / engine96 62.50%; better arm sdk29; 15 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. Not iterated in t495. */
/* MATCHED on the s136os arm (task #1387; screened by task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
 * masked): EXACT 22/22, relocations equal, once
 * the body reads g_guiInstance (absolute through the unit's file-scope
 * `.extern g_guiInstance, 16`; as g_pGuiManager it is gp-relative: 21/22, first
 * diff @1) and names its record table (ROM_DATA_ADDR; as a literal: 1/22, @14
 * `ori` vs the ROM's %lo `addiu`). */
extern u8 D_261520[];
s32 func_002D63B0(void) {
    extern char *g_guiInstance; /* the GUI singleton (alias g_pGuiManager) */

    if (g_guiInstance != 0) {
        func_00342BE8(g_guiInstance + 0x3E9C8, 1);
        func_00342DC0(g_guiInstance + 0x3E9C8, ROM_DATA_ADDR(D_261520, 0x261520));
    }
    return 0;
}
#endif

/* Confirm-dialog input: L1/R1 (0x900) returns 1; cancel(0x10) requests parent;
 * on X(0x40) with the GUI up, read the focused list item and forward it through
 * the command wrapper func_002D6B00. Returns 0/1/-1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6408);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_003432C0(void *p);
extern s32 func_002D6B00(struct MenuCmd *cmd);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 49.86% / engine96 10.69%; better arm sdk29; 36 differing
 * rows on it, class STRUCTURAL; first differing insn: ROM `lui v0,0x0  [HI16
 * D_138180]` vs `lui v0,0x0  [HI16 0x00138344]`. Not iterated in t495. */
/* SCREEN (task #1395, s136 solo, relocated fields masked; not match evidence):
 * 49/51 edit 32 -> 23/51 edit 5, in func_002D5F10's shape (see there). The
 * inlined MenuRequestParent() is the lever. Written out in place it gives 35/51
 * @8, and with its tail as `if (override != 0) return 0; return -1;` 5/51 @14
 * (cc1 picks movn). BLOCKED on the same single access as func_002D5F10: the
 * delay-slot %gp_rel g_guiInstance under the unit's `.extern g_guiInstance, 16`.
 * Read through g_pGuiManager the body screens EXACT 51/51, differing only in
 * that relocation's name, and EE does not define g_pGuiManager. */
#ifndef MENU_REQUEST_PARENT_DEFINED
#define MENU_REQUEST_PARENT_DEFINED
/* Back-button handling shared by the menu input handlers, inlined into each:
 * switch to the active screen's parent (+0xE0) if it names one (returns 0),
 * else leave the menu (-1) unless the override at g_menuScreenBlock+0x134
 * holds it open (0). */
static inline s32 MenuRequestParent(void) {
    u8 *blk = (u8 *)g_menuScreenBlock;
    u8 *parent = *(u8 **)(*(u8 **)(blk + 0x14) + 0xE0);
    if (parent != 0) {
        *(u8 **)(blk + 0x18) = parent;
    } else if (*(s32 *)(blk + 0x134) == 0) {
        return -1;
    }
    return 0;
}
#endif
s32 func_002D6408(void) {
    extern char *g_guiInstance;      /* the GUI singleton (alias g_pGuiManager) */
    extern u8 D_138180[];            /* controller port-0 state; +0x1C4 = g_padButtonsPressed */
    u8 *pad = D_138180;
    s32 result = 0;

    if (*(s32 *)(pad + 0x1C4) & 0x10) {
        return MenuRequestParent();
    }
    if (*(s32 *)(pad + 0x1C4) & 0x900) {
        result = 1;
    } else {
        func_0029D8A8(*(s32 *)(pad + 0x1C4));
        if ((*(s32 *)(pad + 0x1C4) & 0x40) && g_guiInstance != 0) {
            s32 item = func_003432C0(g_guiInstance + 0x3e760);
            s16 cmd[8];
            cmd[1] = *(s16 *)(item + 8);
            *(s32 *)(&cmd[2]) = *(s32 *)(item + 0xc);
            func_002D6B00((MenuCmd *)cmd);
        }
    }
    return result;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D64D8(void) {
    Begin2dDrawBatch(0);
    func_0029D8E8();
    End2dDrawBatch();
    return 0;
}

/* Enable + bind a different GUI list panel (g_pGuiManager + 0x3E760) to its
 * data source (0x261570) when the GUI exists. Returns 0. */
/* GUARD (task #1387): on EE the #else body below is the image's func_002D6508, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D6508)
S136OS_SLOT(func_002D6508);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_003432B8(void *p, s32 v);
extern void func_00343290(void *p, s32 records);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 85.86% / engine96 62.50%; better arm sdk29; 15 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. Not iterated in t495. */
/* MATCHED on the s136os arm (task #1387; screened by task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
 * masked): EXACT 22/22, relocations equal, once
 * the body reads g_guiInstance (absolute through the unit's file-scope
 * `.extern g_guiInstance, 16`; as g_pGuiManager it is gp-relative: 21/22, first
 * diff @1) and names its record table (ROM_DATA_ADDR; as a literal: 1/22, @14
 * `ori` vs the ROM's %lo `addiu`). */
extern u8 D_261570[];
s32 func_002D6508(void) {
    extern char *g_guiInstance; /* the GUI singleton (alias g_pGuiManager) */

    if (g_guiInstance != 0) {
        func_003432B8(g_guiInstance + 0x3E760, 1);
        func_00343290(g_guiInstance + 0x3E760, ROM_DATA_ADDR(D_261570, 0x261570));
    }
    return 0;
}
#endif

/* Forward this frame's newly-pressed buttons to a per-screen draw helper. */
s32 func_002D6560(void) {
    func_0029D1D8(g_padButtonsPressed[0]);
    return 0;
}

/* Draw-batch wrapper. */
s32 func_002D6588(void) {
    Begin2dDrawBatch(0);
    func_0029D1A8();
    End2dDrawBatch();
    return 0;
}

/* Mark no map slot active (-1). Returns 0. */
s32 func_002D65B8(void) {
    g_mapActiveSlot[0] = -1;
    return 0;
}

/* return 0 stub. */
s32 func_002D65D0(void) {
    return 0;
}

/* Galactic-map "travel" confirm input. Cancel(0x10) fades out and requests the
 * parent. L1/R1 (0xd00) snaps the cursor to the current level. X(0x40) either
 * stays (already there) or initiates a level change / exit; otherwise forwards
 * to the planet-row handler. Returns 0/1/-1. Wall: deep branch ladder. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", GalacticMapConfirmTravelInput);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void FadeOutToBlackBlocking(s32 frames);
extern void func_002CAB90(s32 bits);
extern s32 func_0029D248(s32 arg);
extern s32 func_0026F7D0(void);
extern s32 func_0026F7D8(void);
extern void RequestLevelExit(s32 destination, s32 commitSave);
extern s32 RequestGameStateChange(s32 stateId, s32 push, s32 argA, s32 argB, s32 outDoneFlag);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 38.29% / engine96 5.65%; better arm sdk29; 86 differing rows
 * on it, class STRUCTURAL; first differing insn: ROM `lui v1,0x0  [HI16
 * 0x00138344]` vs `lui v0,0x0  [HI16 0x00138344]`. Not iterated in t495. */
    /* TODO(match): functional equivalent - not byte-exact. */
s32 GalacticMapConfirmTravelInput(void) {
    extern u8 g_abLevelVisitedMarkers[];
    s32 pressed = g_padButtonsPressed[0];
    if (pressed & 0x10) {
        void *nxt;
        FadeOutToBlackBlocking(4);
        func_002CAB90(0x40800000);
        nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && D_001F28F4 == 0) return -1;
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    if (pressed & 0xd00) {
        g_nMapCurrentLevel = g_playerProgress;
        return 1;
    }
    if ((pressed & 0x40) == 0) {
        func_0029D248(pressed);  /* the ROM's $a0 still holds the pad mask here */
        return 0;
    }
    if (g_playerProgress == g_nMapCurrentLevel) {
        PlayGlobalSound(4, 0, 0);
        FadeOutToBlackBlocking(4);
        return 1;
    }
    PlayGlobalSound(4, 0, 0);
    {
        s32 lvl;
        if (g_nMapCurrentLevel != 0 || g_abLevelVisitedMarkers[0x19] != 0 ||
            g_nMapCurrentLevel != 2) {
            if (func_0026F7D0() != 0 && func_0026F7D8() != 0) {
                RequestLevelExit(g_nMapCurrentLevel, 1);
                return 1;
            }
            lvl = g_nMapCurrentLevel;
        } else {
            lvl = 0x19;
        }
        RequestGameStateChange(6, 2, 1, lvl, 0);
    }
    return 1;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D6770(void) {
    Begin2dDrawBatch(0);
    func_0029D218();
    End2dDrawBatch();
    return 0;
}

/* Screen-action wrapper: MenuScreenDoAction(op, arg, &localFlag). */
s32 func_002D67A0(s32 op, s32 arg) {
    s32 outFlag = 0;
    return MenuScreenDoAction(op, arg, &outFlag);
}

/* Central front-end/pause screen-action dispatcher. `op` (0-13, via
 * jtbl_0026CD40_text) selects an action on the menu-screen manager block
 * (g_menuScreenBlock): push/swap a screen (ops 6-8,10,11 stage +0xF4/0x100/0x104
 * from the current screen fields and set the +0x1C transition kind, then ring the
 * menu chrome sound func_002DFFA0 and clear *outFlag), request the front-end game
 * state (ops 4,5,12,13 via RequestGameStateChange with the save/popup status bits
 * at g_nSaveLoadStatusCode+0x4 toggled), set the next screen (op 3) or the active
 * language (op 9). Returns 1 when a screen push/swap was staged, else 0.
 * jtbl targets recovered from data/138B80. Matching arm stays INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", MenuScreenDoAction);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_002888D0(void);
extern s32 RequestGameStateChange(s32 stateId, s32 push, s32 argA, s32 argB, s32 outDoneFlag);
extern void func_002861D8(s32 a, s32 b);
extern void func_00286138(s32 a, s32 b);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 45.51% / engine96 37.61%; better arm sdk29; 133 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-112`. Not iterated in t495.
 * Save stride re-measured on the s136os arm (SN 1.36 -fopt-stack): saves land at the ROM's
 * 8-byte stride, but the saved register set/count differs from the ROM's (NOTE #9871) - an
 * allocation/frame difference, not the stride. So the save-slot wall named above is a cc1 2.9
 * property and was measured false as the reason this member stays unmatched (FACT #9873).
 * Residual: the saved-register set; the rest UNMEASURED. */
s32 MenuScreenDoAction(s32 op, s32 arg, void *outFlag) {
    extern u8 g_currentLanguage;
    extern u8 g_collTriBuffer[];
    u8 *mb = (u8 *)g_menuScreenBlock;
    s32 *statusFlags = (s32 *)((u8 *)&g_nSaveLoadStatusCode + 4);
    s32 *popupState = (s32 *)((u8 *)&g_pTextTableLoadBuf + 0x48);
    s32 ret = 0;

    switch (op) {
    case 0:
    case 1:
        break;

    case 2:
        func_002DFFA0(2, 0x11);
        break;

    case 3:
        g_pNextMenuScreen[0] = (void *)arg;
        break;

    case 4:
        PlayGlobalSound(4, 0, 0);
        func_002888D0();
        *statusFlags = (*statusFlags | 2) & ~4;
        *popupState = 0;
        RequestGameStateChange(4, 1, 1, arg, 0);
        break;

    case 5:
        PlayGlobalSound(4, 0, 0);
        *statusFlags = (*statusFlags | 4) & ~2;
        RequestGameStateChange(4, 1, 1, arg, 0);
        *popupState = 1;
        break;

    case 6:
        if ((arg >> 16) != 0) {
            *(s32 *)(mb + 0xFC) = *(s32 *)(g_collTriBuffer + 0x1020 + (arg >> 16) * 4);
        }
        *(s32 *)(mb + 0xF4) = arg & 0xFFFF;
        *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
        *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
        *(s32 *)(mb + 0x1C) = 5;
        ret = 1;
        func_002DFFA0(0, 0x11);
        *(s32 *)outFlag = 0;
        break;

    case 7:
        *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
        *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
        *(s32 *)(mb + 0x1C) = 3;
        *(s32 *)(mb + 0xF4) = arg;
        func_002DFFA0(0, 0x11);
        *(s32 *)outFlag = 0;
        /* fall through: op 7 continues into the op 8 staging */
    case 8:
        *(s32 *)(mb + 0xF4) = arg;
        *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
        *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
        *(s32 *)(mb + 0x1C) = 4;
        ret = 1;
        func_002DFFA0(0, 0x11);
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
        func_002DFFA0(0, 0x11);
        *(s32 *)outFlag = 0;
        break;

    case 11:
        *(s32 *)(mb + 0x1C) = 7;
        *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
        *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
        ret = 1;
        func_002DFFA0(0, 0x11);
        *(s32 *)outFlag = 0;
        break;

    case 12:
        PlayGlobalSound(4, 0, 0);
        func_002861D8(0, 0);
        func_00286138(0, 0);
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

/* List-entry screen-action forwarder: dispatch `op` with the arg pulled from
 * the selected row of the widget's command table (row = widget+0x40, stride
 * 0xC, arg at +4) and return MenuScreenDoAction's result.
 * MATCHED 100.00% on the sdk29 arm (unit objdiff, objdiff_build.sh +
 * unit_report.sh; verify_match_unit.sh BYTE IDENTICAL, t495). The old
 * "97% register-coloring" note described the diff.sh instrument. */
s32 func_002D6AA0(void *widget, s32 op, void *outFlag) {
    u8 *w = (u8 *)widget;
    s32 row = *(s32 *)(w + 0x40);
    s32 arg = *(s32 *)(row * 0xc + *(s32 *)(w + 0x34) + 4);
    return MenuScreenDoAction(op, arg, outFlag);
}

/* Command-record wrapper: MenuScreenDoAction(cmd->op, cmd->arg, out). */
s32 func_002D6AD8(MenuCmd *cmd, void *out) {
    return MenuScreenDoAction(cmd->op, cmd->arg, out);
}

/* Command-record wrapper with a local out-flag. */
s32 func_002D6B00(MenuCmd *cmd) {
    s32 outFlag = 0;
    return MenuScreenDoAction(cmd->op, cmd->arg, &outFlag);
}

/* Galactic-map planet-list cursor/scroll + select input. Wall: ~190-instruction
 * branch graph with cross-screen link pointers (next/prev at +0x4C/+0x50/+0x54/
 * +0x58) and a divide-by-row-count trap. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", UpdateMenuListActions);

/* Draw the galactic-map planet list (per-row labels, highlight, level-name
 * substitution). Wall: large draw loop + many leaf calls + pointer-compare
 * ladder over fixed screen instances. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6E98);

/* Cross-fade the galactic-map planet colour toward a target over `progress` steps.
 * Clamps progress to >= 0, substitutes the two default packed RGBA colours when a
 * colour arg is -1 (0x80FFA888 / 0x8020FFFF), computes the fade fraction
 * progress/limit (limit = D_1AA460; saturated to 1.0 once progress exceeds it) and
 * hands the two colours + fraction to the colour-lerp leaf ColorLerpPacked.
 * MATCHED 100.00% on the sdk29 arm (unit objdiff; verify_match_unit.sh BYTE
 * IDENTICAL, t495). Three spellings carry the match: the clamp as a ?: (an
 * `if (progress > -1)` is canonicalised to slti/movz; the ?: keeps the
 * original's li -1 / slt / movn), the divide path first with fade = 1.0 in the
 * else, and the asm barrier that keeps the jal + $ra frame. */
extern s32  D_1AA460;                                          /* galactic-map fade step count */
extern u32  ColorLerpPacked(s32 color1, s32 color2, f32 fade);   /* 0x2846E8 packed-RGBA colour lerp */

void SetGalacticMapFadeAlpha(s32 progress, s32 color1, s32 color2) {
    s32 clampedProgress = (progress > -1) ? progress : 0;
    f32 fade;

    if (color1 == -1) {
        color1 = (s32)0x80FFA888;
    }
    if (color2 == -1) {
        color2 = (s32)0x8020FFFF;
    }

    if (clampedProgress <= D_1AA460) {
        fade = 1.0f - (f32)(D_1AA460 - clampedProgress) / (f32)D_1AA460;
    } else {
        fade = 1.0f;
    }
    ColorLerpPacked(color1, color2, fade);
    __asm__ __volatile__("");   /* keep the jal + ra frame; cc1 would tail-jump */
}

/* Galactic-map grid cursor input: moves a GridMenu's cursor with the d-pad and
 * handles select.
 *   menu: the grid (+0x30 flags, +0x3C index, +0x40 rows, +0x44 cols,
 *         +0x48 entries, +0x4C/+0x50/+0x54/+0x58 up/down/left/right links).
 * The pressed mask is D_138180+0x1C4 (g_padButtonsPressed). Returns 0 when
 * menu is not the focused grid (menu block owner+0xE8). Returns
 * 1 when pressed-mask bits 0x900 are set with no fallback. On bit 0x10 it
 * copies owner+0xE0 to the menu block's value and returns 0, or returns -1 when
 * that is 0 and there is no fallback. Otherwise Up/Down (0x1000/0x4000) step a
 * row: at an edge they move focus (owner+0x128) to the linked grid, skipping
 * links whose flags have bit 3 (when the block's wrap is set) or bit 2 (when its
 * edge is set), and map the column across 3- and 5-column grids; with no link
 * they wrap unless flags bit 15 is set. Left/Right (0x8000/0x2000) do the same
 * per column. A cursor or focus change plays menu sound 1. On bit 0x40, for
 * a selectable grid (flags bit 0 clear) with no cooldown (busy), an owned entry's item
 * is put in its weapon's equip slot (g_weaponTable[g_itemEquippedSlot[item]]
 * .equipMode), or taken out of slot 1 or 2 if already there; busy = 10 and menu
 * sound 0. An item not owned plays sound 2. Returns 0.
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack), task #2252.
 * It closes only with gcse ON (pinned -fno-gcse: 321 words against the ROM's
 * 322), which is why the unit's s136os compile is unpinned (S136EXTRA="",
 * RULING #9491 shape, FACT #10232). One device, already in this unit:
 * g_inventoryOwned is read through g_inventoryOwnedAbs, the #8036 assembler
 * alias func_002DECE0 uses (8 bytes to cc1, so it emits one `la` macro; 16 to
 * gas, so the macro expands absolutely). The ROM forms that address as an
 * adjacent lui/addiu pair; read as the plain unsized array, cc1 splits it and
 * puts the lui in a delay slot (321 words). The relocations name
 * g_inventoryOwned.
 * GUARD: compiled alone by SN 2.95.3 v1.36 -fopt-stack and spliced over the
 * S136OS_SLOT line (tools/ee/s136os_splice.sh); plain C on native.
 * Source: OpenRAC/rac2-gc-decomp @96927f872c709e13aaa0f32265bb81a5531bf7ed
 * (GPL-3.0, adopted with the authors' permission per RULING #10405),
 * src/game/002D6970-g32.c, func_002D6970 (v1.01 address; author commit
 * 0eb05a7c, Flavius), ADAPTED: (1) their D_001F2840 MenuRoot and D_00138180
 * PadState are views of our g_menuScreenBlock and D_138180 (the gridRoot and
 * gridPad macros below; same addresses, so the same relocations);
 * (2) their D_001A7AF8 (declared MACRO_ADDR there) is our g_inventoryOwned,
 * read through the alias above (gridOwned); their D_00139568, D_00239BA0 and
 * func_002DF190 are our g_itemEquippedSlot, g_weaponTable (cast to their
 * WeaponInfo) and func_002DFFA0; (3) their `if (!((menu->flags ^ 1) & 1))` is
 * split through the local `selectable`: this unit is C++, and the C++ front
 * end folds the xor away (andi + bne at 0x2D79DC: 3 of 322 words differ, vmu)
 * where C keeps the ROM's xori/andi/beqz; (4) their `-g32` file-name flag is not carried (the unit is
 * -G8 and the body has no gp-relative access). Their typedefs and the rest of
 * the body are verbatim. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D75D8)
S136OS_SLOT(func_002D75D8);
#else
typedef struct { char p00[0x14]; char *owner; int value; char p1c[0x24]; int slots[4]; char p50[0xE4]; int fallback; char p138[0xC]; int wrap; int edge; char p14c[0xC]; int busy; } MenuRoot;
typedef struct { char reserved[0x1C4]; int flags; } PadState;
typedef struct GridMenu { char p00[0x30]; int flags; char p34[8]; int index; int rows; int cols; void *entries; struct GridMenu *up; struct GridMenu *down; void *left; void *right; } GridMenu;
typedef struct { char p00[6]; short item; char p08[2]; } GridEntry;
typedef struct { char p00[0xC]; int slot; char p10[0xD0]; } WeaponInfo;

extern u8 D_138180[];
#ifndef TARGET_NATIVE
extern u8 g_inventoryOwnedAbs[8];  /* the #8036 alias below (func_002DECE0's) */
#define gridOwned g_inventoryOwnedAbs
#else
#define gridOwned g_inventoryOwned
#endif
extern u8 g_itemEquippedSlot[];
extern u8 g_weaponTable[];
#define gridRoot (*(MenuRoot *)g_menuScreenBlock)
#define gridPad  (*(PadState *)D_138180)

int func_002D75D8(GridMenu *menu)
{
    int flags;
    int cols;
    int old;
    int row;
    int rows;
    int col;
    int x;

    if (*(GridMenu **)(gridRoot.owner + 0xE8) != menu)
        return 0;
    if ((gridPad.flags & 0x900) && gridRoot.fallback == 0)
        return 1;
    flags = gridPad.flags;
    if (flags & 0x10) {
        int value = *(int *)(gridRoot.owner + 0xE0);
        int status;
        if (value != 0)
            gridRoot.value = value;
        else if (gridRoot.fallback == 0) {
            status = -1;
            goto finish;
        }
        status = 0;
finish:
        return status;
    }
    cols = menu->cols;
    old = menu->index;
    rows = menu->rows;
    col = old % cols;
    row = old / cols;
    x = col;
    if (flags & 0x1000) {
        if (row != 0)
            menu->index = old - cols;
        else if (menu->up != 0) {
            GridMenu *link = menu;
            int skip;
            int lcols;
            int lrows;
            do {
                link = link->up;
                skip = 0;
                lrows = link->rows;
                lcols = link->cols;
                if (gridRoot.wrap) {
                    if (link->flags & 8)
                        skip = 1;
                    else
                        skip = 0;
                }
                if (gridRoot.edge && (link->flags & 4))
                    skip = 1;
            } while (skip);
            *(GridMenu **)(gridRoot.owner + 0x128) = link;
            if (lcols == 5)
            {
                x = col + 1;
                if (menu->cols != 3)
                    x = col;
            }
            if (lcols == 3 && menu->cols == 5) {
                x = x - 1;
                if (x > 2)
                    x = 2;
                else if (x < 0)
                    x = 0;
            }
            link->index = (lrows - 1) * lcols + (x < lcols - 1 ? x : lcols - 1);
        } else if (!(menu->flags & 0x8000))
            menu->index = cols * (rows - 1) + old;
    }
    if (gridPad.flags & 0x4000) {
        if (row + 1 < rows)
            menu->index = menu->index + cols;
        else if (menu->down != 0) {
            GridMenu *link = menu;
            int skip;
            int lcols;
            do {
                link = link->down;
                skip = 0;
                lcols = link->cols;
                if (gridRoot.wrap) {
                    if (link->flags & 8)
                        skip = 1;
                    else
                        skip = 0;
                }
                if (gridRoot.edge && (link->flags & 4))
                    skip = 1;
            } while (skip);
            *(GridMenu **)(gridRoot.owner + 0x128) = link;
            if (lcols == 5 && menu->cols == 3)
                x = x + 1;
            if (lcols == 3 && menu->cols == 5) {
                x = x - 1;
                if (x > 2)
                    x = 2;
                else if (x < 0)
                    x = 0;
            }
            link->index = x < lcols - 1 ? x : lcols - 1;
        } else if (!(menu->flags & 0x8000))
            menu->index = menu->index - menu->cols * (menu->rows - 1);
    }
    if (gridPad.flags & 0x8000) {
        if (x != 0)
            menu->index = menu->index - 1;
        else if (menu->left != 0)
            *(void **)(gridRoot.owner + 0x128) = menu->left;
        else if (!(menu->flags & 0x8000))
        {
            int index = menu->index - 1;
            menu->index = index + menu->cols;
        }
    }
    if (gridPad.flags & 0x2000) {
        if (x + 1 < cols)
            menu->index = menu->index + 1;
        else if (menu->right != 0)
            *(void **)(gridRoot.owner + 0x128) = menu->right;
        else if (!(menu->flags & 0x8000))
        {
            int index = menu->index + 1;
            menu->index = index - menu->cols;
        }
    }
    if (menu->index != old || *(int *)(gridRoot.owner + 0x128) != 0)
        func_002DFFA0(1, 0x11);
    if (gridPad.flags & 0x40) {
        MenuRoot *root;
        GridEntry *entry;
        int slot;
        int selectable = (menu->flags ^ 1) & 1;
        if (!selectable)
            return 0;
        root = &gridRoot;
        if (root->busy != 0)
            return 0;
        entry = (GridEntry *)menu->entries + menu->index;
        if (entry->item != 0 && gridOwned[entry->item] != 0) {
            WeaponInfo *info = &((WeaponInfo *)g_weaponTable)[g_itemEquippedSlot[entry->item]];
            slot = info->slot;
            func_002DFFA0(0, 0x11);
            root->busy = 10;
            if (root->slots[slot] == entry->item && slot != 0 && slot != 3)
                root->slots[slot] = 0;
            else
                gridRoot.slots[slot] = entry->item;
        } else
            func_002DFFA0(2, 0x11);
    }
    return 0;
}
#undef gridRoot
#undef gridPad
#undef gridOwned
#endif

/* Galactic-map streaming-slot teardown. When no CD read is in flight (the
 * file-load mode halfword at g_fileLoadVoiceState+4, i.e. g_fileLoadState, is
 * 0) it unlocks the locked map-cache slot: clears the menu block's load-active
 * byte (g_menuScreenBlock+0xDB), toggles the slot's 0x1000 (loaded) bit in
 * slotLevelId[] and releases lockedSlot. Otherwise, if a menu-driven load is in
 * progress (that byte nonzero) it aborts the CD read (StopFileLoad), clears the
 * byte and invalidates the locked slot outright (slotLevelId[idx] = lockedSlot
 * = -1). No params; returns 0.
 * MapCache fields per symbol_addrs: slotLevelId[5] @+0x29C, lockedSlot @+0x2B0.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack at this
 * unit's -G8 and S136EXTRA "", so gcse on; row in tools/ee/s136os_functions.txt),
 * spliced over S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice drops the function. On native it is
 * plain C. Upstream compiles this file at -G0; at our -G8 the same addressing
 * comes from the unit's own model (NOTE #12163): the ROM reads the mode
 * halfword with a compiler-split lui / lh %lo(g_fileLoadVoiceState+4), hence
 * ROM_SPLIT on that view (RULING #8620: an extern declaration, no data moved),
 * and the map cache and menu block are aggregates, never small data. Without
 * ROM_SPLIT it builds 42 words, not 47 (FACT #12162). Task #2295.
 *
 * Source: OpenRAC/rac2-gc-decomp @96927f872c709e13aaa0f32265bb81a5531bf7ed
 * (GPL-3.0, adopted with the authors' permission per RULING #10405),
 * src/game/002D5E98.c, func_002D6E78 (v1.01 address; author commit 66978bb7,
 * ThorpenCodes). ADAPTED: (1) our symbols, as label aliases of their struct
 * views: D_001A63A8 = g_fileLoadVoiceState, D_001C4FA0 = g_mapVertexData,
 * D_001F2840 = g_menuScreenBlock; func_002B7AD0 = StopFileLoad; (2) their
 * separate byte D_001F291B is written as the menu view's `active` field, the
 * same address (g_menuScreenBlock+0xDB; the EE link has no symbol at 0x1F289B);
 * (3) ROM_SPLIT on the g_fileLoadVoiceState view; (4) their struct types and
 * fields renamed. Statements and control flow are theirs. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D7AE0)
S136OS_SLOT(func_002D7AE0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void StopFileLoad(void);
/* (end of this body's declarations) */
typedef struct FileLoadModeView {
    char pad[4];
    short mode;          /* 0x4 - g_fileLoadState: 0 idle, nonzero CD read busy */
} FileLoadModeView;
typedef struct MapSlotLockView {
    char pad[0x29C];
    int slotLevelId[5];  /* 0x29C */
    int lockedSlot;      /* 0x2B0 */
} MapSlotLockView;
typedef struct MenuLoadView {
    char pad[0xDB];
    unsigned char active;  /* 0xDB - a menu-driven load is in progress */
    char padDC[0x118 - 0xDC];
    char *loaded;
    unsigned int limit;
} MenuLoadView;
extern FileLoadModeView g_fileLoadModeView __asm__("g_fileLoadVoiceState") ROM_SPLIT;
extern MapSlotLockView g_mapSlotLockView __asm__("g_mapVertexData");
extern MenuLoadView g_menuLoadView __asm__("g_menuScreenBlock");

int func_002D7AE0(void)
{
    if (g_fileLoadModeView.mode == 0) {
        MapSlotLockView *map = &g_mapSlotLockView;
        int index = map->lockedSlot;
        if (index != -1) {
            g_menuLoadView.active = 0;
            map->slotLevelId[index] ^= 0x1000;
            map->lockedSlot = -1;
        }
    }
    if (g_fileLoadModeView.mode != 0) {
        MenuLoadView *state = &g_menuLoadView;
        if (state->active != 0) {
            MapSlotLockView *map;
            StopFileLoad();
            state->active = 0;
            map = &g_mapSlotLockView;
            map->slotLevelId[map->lockedSlot] = -1;
            map->lockedSlot = -1;
        }
    }
    return 0;
}
#endif

/* Galactic-map screen per-frame tick (advance fades, dispatch sub-screens).
 * Wall: large state machine + multi callee-save frame. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", GalacticMapScreenTick);

/* Refresh the objectives screen: recompute objective states, gather the active
 * objective list into the scratchpad, and seed the preview record for the current
 * map level.
 *
 * obj: the screen's menu widget. Calls UpdateLevelObjectiveStates, then stores
 * GatherActiveObjectives(0x70000000, 0, 0x70000100, 1) (ids and values written to
 * scratchpad 0x70000000 / 0x70000100) as the count at obj+0xA0. If the widget's
 * per-level slot (obj+0x30 + 4 * g_mapCurrentLevel) is not -1, the preview record
 * D_0025A6C0 gets +0x3C = -576 and +0x34 = the slot's scratchpad id, and D_25A600
 * gets its scratchpad value. Clears obj+0xA4. Returns 0.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, task #1917), no device.
 * Each lever's cost when removed alone (solo s136 compile, positional
 * relocation-masked words against the ROM):
 *   - g_mapCurrentLevel is declared as an array, so cc1 addresses it absolutely
 *     (lui/lw) as the ROM does. Read through the gp-relative g_nMapCurrentLevel
 *     alias: 32/44, 43 words.
 *   - The two preview stores go through one `rec` base (the ROM forms
 *     la D_0025A6C0 once and stores at +0x3C and +0x34). As two indexed stores
 *     off the array: 6/44.
 *   - `slots` (obj+0x30) is its own variable: the ROM forms obj+0x30 first and
 *     then adds level*4. Folded into one expression, cc1 reassociates it into
 *     obj + (level*4 + 0x30): 6/44.
 * The data symbols are the ROM's own labels (D_0025A6C0, D_25A600); the native
 * build resolves them through the arena (RULING #9542). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D8270)
S136OS_SLOT(func_002D8270);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32  UpdateLevelObjectiveStates(void);
extern s32 GatherActiveObjectives(s32 outIds, s32 outMask, s32 outVals, s32 wantValues);
/* (end of this body's declarations) */
s32 func_002D8270(MenuWidget *obj) {
    extern s32 g_mapCurrentLevel[];
    extern u8 D_0025A6C0[];
    extern s32 D_25A600[];
    u8 *o = (u8 *)obj;
    s32 *slots;
    s32 *pLevel;
    UpdateLevelObjectiveStates();
    *(s32 *)(o + 0xA0) = GatherActiveObjectives(0x70000000, 0, 0x70000100, 1);
    slots = (s32 *)(o + 0x30);
    pLevel = slots + g_mapCurrentLevel[0];
    if (*pLevel != -1) {
        u8 *rec = D_0025A6C0;
        *(s32 *)(rec + 0x3C) = 0xfffffdc0;
        *(s32 *)(rec + 0x34) = *(s32 *)(*pLevel * 4 + 0x70000000);
        D_25A600[0] = *(s32 *)(0x70000100 + *pLevel * 4);
    }
    *(s32 *)(o + 0xA4) = 0;
    return 0;
}
#endif

/* Galactic-map planet-select input (translate cursor row to a level id and
 * commit it). Wall: large branch graph + multi callee-save frame + divide trap.
 * Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", HandleGalacticMapPlanetSelectInput);

/* Handwritten frameless stub fragment (no jr — addiu $sp run). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8770);

/* Mark no map slot active (-1). Returns 0. (Twin of func_002D65B8.) */
s32 func_002D8778(void) {
    g_mapActiveSlot[0] = -1;
    return 0;
}

/* Forwarding wrapper: func_002DF1B8(1); return 0. */
s32 func_002D8790(void) {
    func_002DF1B8(1);
    return 0;
}

/* Snapshot the saved channel volume back into the live table slot 2. */
s32 func_002D87B0(void) {
    g_sndChannelVolumes[2] = D_1A7BA8;
    return 0;
}

/* Title-screen audio-options input (UpdateAudioOptionsMenu). 3-row menu in
 * obj->0x40 (mod 3): Up/Down move the selection (sound on change); row 0/1 are
 * the music/sfx volume sliders adjusted by held Left/Right in steps of 3 and
 * clamped 0..0x400 (re-mixing on change); row 2 toggles mono/stereo on X.
 * Cancel(0x10) requests the parent. Returns 0/1/-1.
 * Wall: long branch ladder + clamp arithmetic. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", SoundOptionsMenuInput);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void MapSetCurrentLevel(s32 level);
extern void ComputeAudioChannelMix(void);
extern void func_00132938(s32 flag);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 63.85% / engine96 55.96%; better arm sdk29; 144 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-48`. Not iterated in t495.
 * Save stride re-measured on the s136os arm (SN 1.36 -fopt-stack): this member's prologue - the
 * ROM's save set at its 8-byte stride, and its frame - is reproduced exactly (NOTE #9871), so
 * the save-slot wall named above is a cc1 2.9 property and was measured false as the reason
 * this member stays unmatched (FACT #9873). Residual: UNMEASURED. */
    /* TODO(match): functional equivalent - not byte-exact. */
s32 SoundOptionsMenuInput(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 pressed = g_padButtonsPressed[0];
    s32 sel;
    if ((pressed & 0x900) && D_001F28F4 == 0) {
        return 1;
    }
    if (pressed & 0x10) {
        void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && D_001F28F4 == 0) return -1;
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    sel = *(s32 *)(o + 0x40);
    if (pressed & 0x1000) *(s32 *)(o + 0x40) = (sel + 2) % 3;
    if (pressed & 0x4000) *(s32 *)(o + 0x40) = (*(s32 *)(o + 0x40) + 1) % 3;
    if (*(s32 *)(o + 0x40) != sel ||
        *(s32 *)((u8 *)g_pCurrentMenuScreen[0] + 0x128) != 0) {
        func_002DFFA0(1, 0x11);
        if (*(u32 *)(o + 0x30) & 0x20) {
            extern s32 g_anAvailableLevelOrder[];
            MapSetCurrentLevel(g_anAvailableLevelOrder[*(s32 *)(o + 0x40)]);
        }
    }
    {
        s32 prevMusic = g_nMusicVolume;
        s32 prevSfx = g_nSfxVolume;
        if (g_padButtonsHeld & 0x2000) {
            if (*(s32 *)(o + 0x40) == 0) {
                s32 v = g_nMusicVolume + 3;
                g_nMusicVolume = (v < 0x401) ? v : 0x400;
            } else if (*(s32 *)(o + 0x40) == 1) {
                s32 v = g_nSfxVolume + 3;
                g_nSfxVolume = (v < 0x401) ? v : 0x400;
            }
        }
        if (g_padButtonsHeld & 0x8000) {
            if (*(s32 *)(o + 0x40) == 0) {
                g_nMusicVolume -= 3;
                if (g_nMusicVolume < 0) g_nMusicVolume = 0;
            } else if (*(s32 *)(o + 0x40) == 1) {
                g_nSfxVolume -= 3;
                if (g_nSfxVolume < 0) g_nSfxVolume = 0;
            }
        }
        if (prevSfx != g_nSfxVolume || prevMusic != g_nMusicVolume) {
            ComputeAudioChannelMix();
        }
    }
    if (pressed & 0x40) {
        if (*(s32 *)(o + 0x40) == 2) {
            g_nAudioStereoMode = (g_nAudioStereoMode == 0);
        }
        func_00132938(g_nAudioStereoMode == 0);
        func_002DFFA0(0, 0x11);
    }
    return 0;
}
#endif

/* Draw the title-screen audio-options screen: three rows (music volume, SFX
 * volume, stereo/mono mode) inside a Begin/End2dDrawBatch. Each row's left
 * label is drawn in the highlight colour (0x8020FFFF) when it is the selected
 * row (screen[0x40] == row index) else normal (0x80FFA888). The two volume rows
 * additionally draw a two-layer bar (outer 0x80696969 / inner 0x80383838) and a
 * fill icon whose right edge tracks (screen[0x20] - (lx+0x4A)) * volume / 1024
 * (round-toward-zero); the stereo row draws the localized Mono/Stereo string
 * (chosen by g_audioStereoMode) instead of a bar. Row Y steps by screen[0x24]>>2
 * (ry, 2*ry, 3*ry); label/value X derive from screen[0x20]>>1 (lx). Returns 2.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, the unit's gcse-on
 * S136EXTRA, task #2383). What closed task #1395's last 12 words: one row-Y
 * local that walks the rows, formed as `ry + ry` for row 2 and advanced with
 * `y += ry` for row 3. The ROM then holds 2*ry, 3*ry and 3*ry-8 in one
 * register ($s0), and fills row 2's last jal slot with the `addu` that forms
 * 3*ry. Spelled `2 * ry`, or as a fresh row-3 expression, cc1 fills that slot
 * with the icon call's `sll` argument instead (12 words). A row-3 local set
 * before the row-2 icon call costs 55..148 words. Also worse, measured before
 * the walking local: `ry + 2 * ry` (79 words) and `lx + (fill + 8)` (187). */
/* ADDRESSING-MODEL DEVICE (RULING #8620 class; directive-only, emits nothing):
 * the ROM reads all three volumes absolutely (%hi/%lo), never gp-relative. At
 * FILE SCOPE, outside the guard, so the unit's 2.9 TU declares them before the
 * spliced block too (s136os_splice refuses ADDRESSING otherwise). */
#ifndef TARGET_NATIVE
__asm__(".extern g_musicVolume, 16");
__asm__(".extern g_sfxVolume, 16");
__asm__(".extern g_audioStereoMode, 16");
#endif
/* GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D8A68)
S136OS_SLOT(func_002D8A68);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 GetLocalizedString(s32 id);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 41.09% / engine96 41.28%; better arm engine96; 207 differing
 * rows on it, class STRUCTURAL; first differing insn: ROM `addiu sp,sp,-144`
 * vs `addiu sp,sp,-112`. Not iterated in t495. */
/* SCREEN (task #1395, s136 solo, relocated fields masked; not match evidence):
 * 215/218 edit 249 -> 12/218 edit 18 (length 218 = ROM). Levers, in order of
 * effect: the panel width and selected row re-read from the screen after every
 * call (MENU_W/MENU_SEL) with each row's colour picked at its call; the bar's
 * left edge `barX` as a named local scoped so it is spilled after the row-1
 * temporaries (stack slot 0x1C); the fill end spelled `lx + 8 + fill`; `ry`
 * before `lx`; the volume reads absolute (`.extern ,16`); the colours 64-bit
 * (the ROM's dli); the stereo-string pick as `!= 0`; the fills as `/ 0x400`.
 * The remaining 12 words (169..180) were one delay-slot choice, closed in task
 * #2383 by the walking row-Y local `y` (see the doc comment above). */
extern s32 g_musicVolume;
extern s32 g_sfxVolume;
extern s32 g_audioStereoMode;
/* The panel width (+0x20) and selected row (+0x40) are re-read from the
 * screen after every call, as the ROM does. */
#define MENU_W(s)   (*(s32 *)((s) + 0x20))
#define MENU_SEL(s) (*(s32 *)((s) + 0x40))

void func_002904B0(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 mode);
void func_0028FFF0(s32 iconIndex, s32 x0, s32 y0, s32 x1, s32 y1,
                   s32 u0, s32 v0, s32 u1, s32 v1, s32 alpha);
s32  func_0028EDF0(s32 name, s32 level);
#ifndef TARGET_NATIVE
/* The two text draws take a 64-bit colour (the ROM loads every colour with a
 * dli: ori/dsll/ori), as 1CA080.cpp declares them. */
void func_0027FBA8(s32 x, s32 y, u64 color, s32 str, s64 wrap);
void func_00280090(s32 x, s32 y, u64 color, s32 str, s64 wrap);
#else
void func_0027FBA8(s32 a, s32 b, s32 c, s32 d, s32 e);
void func_00280090(s32 a, s32 b, s32 c, s32 d, s32 e);
#endif

s32 func_002D8A68(u8 *screen) {
    s32 ry = *(s32 *)(screen + 0x24) >> 2;   /* per-row Y step */
    s32 lx = *(s32 *)(screen + 0x20) >> 1;   /* label / value-text X base */
    s32 fill, icon, y;

    Begin2dDrawBatch(0);

    /* row 1 - music volume (y = ry); the selected row is highlighted */
    func_00280090(lx - 8, ry - 8, (MENU_SEL(screen) == 0) ? 0x8020FFFF : 0x80FFA888,
                  GetLocalizedString(0x2DA5), -1);
    func_002904B0(lx + 7, ry - 8, MENU_W(screen) - 0x3F, ry + 8, 0x80696969, 0);
    func_002904B0(lx + 9, ry - 6, MENU_W(screen) - 0x41, ry + 6, 0x80383838, 0);
    /* Scoped here so barX is allocated after the row-1 temporaries (lx-8, lx+7,
     * lx+9), as the ROM's stack slots order them. */
    {
        s32 barX = lx + 0x4A;                    /* volume-bar left edge */
        fill = (MENU_W(screen) - barX) * g_musicVolume / 0x400;
        icon = func_0028EDF0(0xE99D, 8);
        func_0028FFF0(icon, (lx + 9) << 4, (ry - 6) << 4, (lx + 8 + fill) << 4,
                      (ry + 5) << 4, 0, 0xA0, 0x1F0, 0x150, 0x80);

        y = ry + ry;
        /* row 2 - SFX volume (y = 2*ry) */
        func_00280090(lx - 8, y - 8, (MENU_SEL(screen) == 1) ? 0x8020FFFF : 0x80FFA888,
                      GetLocalizedString(0x2DA6), -1);
        func_002904B0(lx + 7, y - 8, MENU_W(screen) - 0x3F, y + 8, 0x80696969, 0);
        func_002904B0(lx + 9, y - 6, MENU_W(screen) - 0x41, y + 6, 0x80383838, 0);
        fill = (MENU_W(screen) - barX) * g_sfxVolume / 0x400;
        icon = func_0028EDF0(0xE99D, 9);
        func_0028FFF0(icon, (lx + 9) << 4, (y - 6) << 4, (lx + 8 + fill) << 4,
                      (y + 5) << 4, 0, 0xA0, 0x1F0, 0x150, 0x80);

        y += ry;
        /* row 3 - stereo / mono mode (y = 3*ry) */
        func_00280090(lx - 8, y - 8, (MENU_SEL(screen) == 2) ? 0x8020FFFF : 0x80FFA888,
                      GetLocalizedString(0x2DA7), -1);
        func_0027FBA8(lx + 8, y - 8, 0x80FFA888,
                      GetLocalizedString((g_audioStereoMode != 0) ? 0x2DA9 : 0x2DA8), -1);
    }
    End2dDrawBatch();
    return 2;
}
#endif

/* Rebuild the galactic-map planet display rows: for each of the D_1A7C0C
 * active rows, mark it active (flag = 1) and set its icon from the D_254E48
 * record indexed by the (reversed) source-order byte in D_18D0E8; then clear
 * the icon of the row just past the last. No params; returns 0.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack at this
 * unit's -G8 and S136EXTRA "", so gcse on; row in tools/ee/s136os_functions.txt),
 * spliced over S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice drops the function. On native it is
 * plain C. The earlier near-miss (63.09% on the 2.9 arm, a strength-reduced
 * reverse index) was gcse, not loop phrasing (FACT #9427), and this arm has
 * gcse on. D_1A7C0C keeps the unit's `.extern D_1A7C0C, 16`, which gives the
 * ROM's adjacent lui/lw macro form, the form upstream's MACRO_ADDR gives at
 * -G0 (NOTE #12163). Task #2295.
 *
 * Source: OpenRAC/rac2-gc-decomp @96927f872c709e13aaa0f32265bb81a5531bf7ed
 * (GPL-3.0, adopted with the authors' permission per RULING #10405),
 * src/game/002D8120.c, func_002D8120 (v1.01 address; author commit 8daa0d46,
 * Flavius). ADAPTED: our symbols: D_001A7C04 = D_1A7C0C (their MACRO_ADDR
 * section attribute dropped for the unit's `.extern , 16`), D_0018D0E8 =
 * D_18D0E8, and D_00254EC8 / D_0025ADB0 = D_254E48 / D_0025AD90 as label
 * aliases with their types (the unit declares both under other types); their
 * slot struct renamed. Statements and control flow are theirs. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D8DD0)
S136OS_SLOT(func_002D8DD0);
#else
typedef struct PlanetRowSlot {
    short id;
    short active;
    char pad[8];
} PlanetRowSlot;
extern short g_planetIconRecords[][2] __asm__("D_254E48");
extern PlanetRowSlot g_planetRowSlots[] __asm__("D_0025AD90");

int func_002D8DD0(void)
{
    int i;

    for (i = 0; i < D_1A7C0C; i++) {
        g_planetRowSlots[i].active = 1;
        g_planetRowSlots[i].id = *g_planetIconRecords[D_18D0E8[D_1A7C0C - 1 - i]];
    }
    g_planetRowSlots[D_1A7C0C].id = 0;
    return 0;
}
#endif

/* return 0 stub. */
s32 func_002D8E58(void) {
    return 0;
}

/* Begin a galactic-map text-table swap: release the map slots that hold first-bank
 * text, then ask the screen to reload.
 *
 * obj: the screen's menu widget. Calls func_002DF1B8(1), clears obj+0x54 and
 * obj+0x38 (the saved text-table base and count, see RestorePrevTextTable), then for
 * each of the 5 map slots in D_001B1E90 (id at +0x40, flags at +0x44, stride 8)
 * whose id is nonzero and below the text table's second-bank base
 * (g_menuScreenBlock+0x11C), ORs the in-use bit 2 into its flags. Finally clears
 * obj+0x50 and sets bit 4 of obj+0x10 (reload request). Returns 0.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, task #1917): two
 * phrasing levers, no device. Each one's cost when removed alone (solo s136
 * compile, positional relocation-masked words against the ROM):
 *   - The bound is read as a field of g_menuScreenBlock through a base pointer.
 *     The ROM holds the block address in a register and reloads +0x11C inside the
 *     loop after the id test. Read through the gp-relative D_001F28DC alias of the
 *     same word, cc1 hoists the load out of the loop: 33/39.
 *   - `blk` is assigned after the call. Assigned at its declaration, it is live
 *     across func_002DF1B8 and costs a callee-saved $17: 18/39, 40 words. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D8E60)
S136OS_SLOT(func_002D8E60);
#else
s32 func_002D8E60(MenuWidget *obj) {
    s32 *ids   = (s32 *)(D_001B1E90 + 0x40);
    s32 *flags = (s32 *)(D_001B1E90 + 0x44);
    u8 *blk;
    s32 i;
    func_002DF1B8(1);
    blk = g_menuScreenBlock;
    *(s32 *)((u8 *)obj + 0x54) = 0;
    *(s32 *)((u8 *)obj + 0x38) = 0;
    for (i = 0; i <= 4; i++) {
        if (ids[i * 2] != 0 && (u32)ids[i * 2] < *(u32 *)(blk + 0x11C)) {
            flags[i * 2] |= 2;
        }
    }
    *(s32 *)((u8 *)obj + 0x50) = 0;
    *(s32 *)((u8 *)obj + 0x10) |= 4;
    return 0;
}
#endif

/* Pop side of the push/pop text-table swap: cancel any in-flight load tied to
 * this command (state 3), release the map slots, then reinstall the text table
 * StreamTextTable saved into cmd+0x54 (base) / cmd+0x38 (count). Returns 0. */
#ifndef TARGET_NATIVE
/* ADDRESSING-MODEL DEVICE for RestorePrevTextTable's absolute lui form of g_pActiveTextTable (emits no
 * code). At FILE SCOPE, not in the member's #else arm, so the unit's own 2.9 TU
 * declares the symbol too: tools/ee/s136os_splice.sh never carries an `.extern`
 * for a symbol the unit declares, so cc1's end-of-file small-size line from the
 * s136os TU is not carried in front of the block, where it would make the read
 * gp-relative (task #1387: the splice REFUSED the in-arm placement as ADDRESSING).
 * No 2.9 code in this unit names the symbol bare, so nothing else changes. */
__asm__(".extern g_pActiveTextTable, 16");
#endif
/* GUARD (task #1387): on EE the #else body below is the image's RestorePrevTextTable, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_RestorePrevTextTable)
S136OS_SLOT(RestorePrevTextTable);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void StopFileLoad(void);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 72.44% / engine96 75.74%; better arm engine96; 10 differing
 * rows on it, class STRUCTURAL; first differing insn: ROM `(none)` vs `lw
 * v0,0(gp)  [GPREL16 g_nFileLoadState]`. Not iterated in t495. */
/* MATCHED on the s136os arm (task #1387; screened by task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
 * masked): EXACT 27/27, relocations equal. The
 * globals are read and written by their ROM names in the ROM's address forms
 * (ADDRESSING-MODEL DEVICES, this unit's ROM_SPLIT and `.extern ,16`), each
 * measured undone: g_fileLoadState is an s16 the ROM loads with a compiler-split
 * lui/lh (plain: 25/27, first diff @1); g_pActiveTextTable is stored with the
 * absolute lui $1/sw macro pair (9/27); g_activeTextTableCount with a split
 * lui/sw (6/27). The table pointer is stored before the count (the other order:
 * 7/27). */
extern s16 g_fileLoadState ROM_SPLIT;
extern void *g_pActiveTextTable;
#ifndef TARGET_NATIVE
extern s32 g_activeTextTableCount ROM_SPLIT;
#define TEXT_TABLE_COUNT g_activeTextTableCount
#else
extern s32 g_nActiveTextTableCount;
#define TEXT_TABLE_COUNT g_nActiveTextTableCount
#endif
s32 RestorePrevTextTable(void *cmd) {
    u8 *c = (u8 *)cmd;
    void *table;

    if (g_fileLoadState != 0 && *(s32 *)(c + 0x50) == 3) {
        StopFileLoad();
    }
    func_002DF1B8(1);
    table = *(void **)(c + 0x54);
    if (table != 0) {
        TEXT_TABLE_COUNT = *(s32 *)(c + 0x38);
        g_pActiveTextTable = table;
    }
    return 0;
}
#endif

/* Stream the per-language text table into g_pTextTableLoadBuf, relocate the entry
 * string pointers, install it as active and save the previous table into
 * cmd+0x54/+0x38. Phase machine on cmd->0x50 (jtbl_0026CD80_text, sequential):
 *   0 kick the header read (disc TOC dir entry, 1 sector) -> 1 (or 5 on refusal)
 *   1 wait for the read -> 2
 *   2 kick the language table body read (sectors = size/0x800 into the TOC entry,
 *     byte remainder stashed in D_1ABB38, 0x64 sectors) -> 3 (or 5 on refusal)
 *   3 wait, then fix up the loaded table: relocate each of its `count` entry
 *     pointers by (buffer - 8), swap it in as g_pActiveTextTable (saving the old
 *     table + subtitle count into cmd->0x54 / cmd->0x38), advance -> 4
 *   4,5 terminal (no-op). Returns 0.
 * Matching arm stays INCLUDE_ASM; jtbl targets recovered from data/138B80. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", StreamTextTable);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 StartFileLoad(s32 dest, s32 lbn, s32 sectors);
extern void func_00283460(void *dst, const void *src, s32 nbytes);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 64.78% / engine96 50.75%; better arm sdk29; 118 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-64`. Not iterated in t495.
 * Save stride re-measured on the s136os arm (SN 1.36 -fopt-stack): this member's prologue - the
 * ROM's save set at its 8-byte stride, and its frame - is reproduced exactly (NOTE #9871), so
 * the save-slot wall named above is a cc1 2.9 property and was measured false as the reason
 * this member stays unmatched (FACT #9873). Residual: UNMEASURED. */
s32 StreamTextTable(void *cmdArg) {
    extern s16   g_fileLoadState;      /* 0x1A63AC: 0 idle / nonzero CD-read busy */
    extern u8    g_discToc[];          /* master disc asset directory */
    extern u8    g_currentLanguage;    /* active language index */
    extern s32   D_1ABB38;             /* byte remainder into the streamed TOC entry */
    extern void *g_pActiveTextTable;   /* currently-installed text table */
    extern u8    g_subtitleState[];    /* +0x2C holds the active table entry count */
    u8 *cmd = (u8 *)cmdArg;

    switch (*(s32 *)(cmd + 0x50)) {
    case 0:
        D_1ABB38 = 0;
        if (g_fileLoadState != 0) return 0;
        if (StartFileLoad((s32)g_pTextTableLoadBuf,
                          *(s32 *)(g_discToc + 0x16B0) + *(s32 *)(g_discToc + 0x36C),
                          1) == 0) {
            *(s32 *)(cmd + 0x50) = 5;
        } else {
            *(s32 *)(cmd + 0x50) = 1;
        }
        break;

    case 1:
        if (g_fileLoadState != 0) return 0;
        *(s32 *)(cmd + 0x50) = 2;
        break;

    case 2: {
        s32 entry, sectors, lbn;
        if (g_fileLoadState != 0) return 0;
        entry = *(s32 *)(g_pTextTableLoadBuf + g_currentLanguage * 4);
        sectors = entry / 0x800;
        D_1ABB38 = entry % 0x800;
        lbn = *(s32 *)(g_discToc + 0x16B0) + *(s32 *)(g_discToc + 0x36C) + sectors;
        if (StartFileLoad((s32)g_pTextTableLoadBuf, lbn, 0x64) == 0) {
            *(s32 *)(cmd + 0x50) = 5;
        } else {
            *(s32 *)(cmd + 0x50) = 3;
        }
        break;
    }

    case 3: {
        u8 *mgr = (u8 *)g_menuScreenBlock;
        u8 *buf;
        u8 *tocStart;
        s32 count, size;
        if (g_fileLoadState != 0) return 0;
        buf = *(u8 **)(mgr + 0x118);
        tocStart = buf + D_1ABB38;
        count = *(s32 *)tocStart;
        size = *(s32 *)(tocStart + 4);
        func_00283460(buf, tocStart + 8, ((size + 3) & ~3) - 8);
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

/* Draw a wrapped-text-box menu list (skill-points / level-select variant) with
 * per-row scroll clamping and checkbox indicators. Wall: ~250-instruction draw
 * loop over a 128-bit-packed stack text-box struct. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9178);

/* Draw the default text-box menu list (sibling of func_002D9178 with a simpler
 * box). Wall: ~250-instruction draw loop + sq/lq 128-bit struct packing. Bare. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9718);

/* Position and fill one weapon-grid cell's XP bar.
 *
 * obj: the grid widget (+0x44 = columns per row). entry: the grid entry, whose s16
 * at +6 is the weapon's item id. col/row: the cell. x/y: its screen position.
 * The cell index is row * columns + col; its GUI element is that index's 0x48-byte
 * element in the list func_0034F300 returns for the GUI manager's list at
 * g_guiInstance + 0x36F28. The element is hidden, then shown again when the
 * weapon's current variant (g_weaponTable[g_itemEquippedSlot[item]]) has a nonzero
 * xpThreshold: its scroll position is g_weaponXp[item] >> 5 and its item count is
 * that threshold (both in units of 32 XP). Finally func_0034EF68 lays the cell out
 * at (x, y) with shade 0x80.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, task #1917), no device.
 * Each lever's cost when removed alone (solo s136 compile, positional
 * relocation-masked words against the ROM):
 *   - The GUI manager is read through g_guiInstance, which this unit's file-scope
 *     `.extern g_guiInstance, 16` makes absolute (lui/lw) as in the ROM. Through
 *     the gp-relative g_pGuiManager alias: 63/81, 79 words.
 *   - The item id is re-read from entry+6 at each of its three uses, as the ROM
 *     does (entry stays in $17). Cached in a local: 70/81, 77 words.
 *   - The threshold is read as a WeaponDef field. As byte arithmetic
 *     (table + slot * 0xE0 + 0x6C), cc1 folds 0x6C into %lo(g_weaponTable) where
 *     the ROM keeps the bare base and loads +0x6C: 2/81.
 * The pre-#1917 arm called this an ammo readout and read g_weaponAmmoCapacity;
 * the ROM's relocation names g_weaponXp, and weapon.h documents +0x6C as
 * xpThreshold. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002D9C18)
S136OS_SLOT(func_002D9C18);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0034F300(void *mgr);
extern void GuiElementSetVisible(s32 elem, s32 show);
extern void GuiListSetScrollPos(s32 elem, s32 pos);
extern void GuiListSetItemCount(s32 elem, s32 count);
extern void func_0034EF68(void *base, s32 index, s32 x, s32 y, s32 shade);
/* (end of this body's declarations) */
void func_002D9C18(MenuWidget *obj, void *entry, s32 col, s32 row, s32 x, s32 y) {
    extern char *g_guiInstance;
    extern u8  g_itemEquippedSlot[];
    extern s32 g_weaponXp[];
    extern u8  g_weaponTable[];
    s32 idx = row * *(s32 *)((u8 *)obj + 0x44) + col;
    s32 elem = func_0034F300(g_guiInstance + 0x36F28) + idx * 0x48;
    GuiElementSetVisible(elem, 0);
    if (((WeaponDef *)g_weaponTable)[g_itemEquippedSlot[*(s16 *)((u8 *)entry + 6)]].xpThreshold != 0) {
        GuiElementSetVisible(elem, 1);
        GuiListSetScrollPos(elem, g_weaponXp[*(s16 *)((u8 *)entry + 6)] >> 5);
        GuiListSetItemCount(elem, ((WeaponDef *)g_weaponTable)[g_itemEquippedSlot[*(s16 *)((u8 *)entry + 6)]].xpThreshold);
    }
    func_0034EF68(g_guiInstance + 0x36F28, idx, x, y, 0x80);
}
#endif

/* Draw the weapon/inventory grid (icon cells with ownership/selection state +
 * ammo readout via func_002D9C18). Wall: very large nested draw loop + FP pulse
 * highlight + many indexed inventory tables. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9D60);

/* Set the mode (op) field of the menu command at obj->0x34: 0 in Clank-solo
 * (g_bPlayerMode == 1), otherwise 3. The store is unconditional.
 * obj: the widget whose +0x34 holds the MenuCmd. Always returns 0.
 * Byte-exact on this unit's 2.9 arm (task #681). The ROM seeds v0 = 0 in the
 * beq delay slot, overwrites it with 3 on the fall-through and stores v0 on
 * both paths, so mode 1 DOES store 0. That is the ?: body: t495's guarded
 * "store only when mode != 1" body scored 90.00% and was not the ROM. */
s32 func_002DA330(MenuWidget *obj) {
    MenuCmd *cmd = *(MenuCmd **)((u8 *)obj + 0x34);
    cmd->op = (g_bPlayerMode == 1) ? 0 : 3;
    return 0;
}

/* Per-frame scan of the 24 galactic-map level entries of the widget `o`
 * (entries[] at +0x44, handled bytes used[] at +0xA4). Each populated,
 * not-yet-handled entry whose level-order word D_261900[i] is set has its save
 * accumulator advanced by func_002DFF68(entry, 1), except: entry 7 is skipped
 * while its record's +0xAA halfword is 0x4A and entry 0's bytes +0x42/+0x43
 * differ; and entries 1-3, 5, 6 and 10-12 are skipped when entry 0's +0x40
 * word is 0x9999-tagged with a low byte in [0x4D, 0x92). Returns 4.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack at this
 * unit's -G8 and S136EXTRA "", so gcse on; row in tools/ee/s136os_functions.txt),
 * spliced over S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice drops the function. On native it is
 * plain C. The switch on i - 1 compiles to a 12-entry jump table, which
 * tools/ee/jtbl_bind.sh (task #2247, FACT #12102) checks entry by entry
 * against the ROM's jtbl_0026CDA0_text and binds to it, so no copy of the
 * table is linked. Task #2295.
 *
 * Source: OpenRAC/rac2-gc-decomp @96927f872c709e13aaa0f32265bb81a5531bf7ed
 * (GPL-3.0, adopted with the authors' permission per RULING #10405),
 * src/game/002D5E98.c, func_002D9600 (v1.01 address; author commit e54ed6d5,
 * Louis-Philippe Le Sieur). ADAPTED: (1) `unsigned long long` / `ULL` written
 * `unsigned long` / `UL`: both mean 64 bits on the EE, and SN 1.36 cc1 makes
 * `long long` 128-bit and refuses the wide mask ("unsupported wide integer
 * operation"; FACT #11295's respelling); (2) our symbols: D_00261920 =
 * D_261900, func_002DF158 = func_002DFF68 under this unit's (s32, s32)
 * prototype, so the entry is passed as `(s32)`; (3) their struct type
 * EffectSlots_2D9600 named LevelEntrySlots. Statements and control flow are
 * theirs. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DA358)
S136OS_SLOT(func_002DA358);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_002DFF68(s32 handle, s32 amount);
/* (end of this body's declarations) */
typedef struct LevelEntrySlots {
    char pad00[0x44];
    char *entries[0x18];         /* 0x44 */
    unsigned char used[0x18];    /* 0xA4 - entry already handled */
} LevelEntrySlots;
extern s32 D_261900[];           /* per-entry level-order word */

int func_002DA358(LevelEntrySlots *o)
{
    int i;

    for (i = 0; i < 0x18; i++) {
        char *entry;
        unsigned char id;

        if (o->entries[i] == 0)
            continue;
        if (o->used[i] != 0)
            continue;
        if (D_261900[i] == 0)
            continue;
        if (i == 7
            && *(short *)(o->entries[7] + 0xAA) == 0x4A
            && *(unsigned char *)(o->entries[0] + 0x42)
                   != *(unsigned char *)(o->entries[0] + 0x43))
            continue;
        switch (i - 1) {
        case 0:
        case 1:
        case 2:
        case 4:
        case 5:
        case 9:
        case 10:
        case 11:
            entry = o->entries[0];
            if ((*(unsigned long *)(entry + 0x40) & 0xFFFF0000UL) != 0x99990000UL)
                goto call;
            id = *(unsigned char *)(entry + 0x40);
            if (id < 0x4D)
                goto call;
            if (id < 0x92)
                continue;
            break;
        }
    call:
        func_002DFF68((s32)o->entries[i], 1);
    }
    return 4;
}
#endif

/* Dead epilogue-pad debris ahead of func_002DA4B0 (`sw $0,0x44($4)` and four
 * `addiu $sp,+N` words, each followed by a nop; no prologue, no jr). Not a
 * function body, so it cannot be C. Split off by a symbol_addrs size pin. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA488);

/* Galactic-map draw kick: write the two GS registers the map pass needs
 * (0x42 = 0x44, 0x47 = 0xB) and draw the map without the HUD pass and without
 * scissoring. Reached only through the screen-handler table word at 0x25A354
 * (D_0025A350's second slot, beside GalacticMapScreenTick), never by a jal.
 * Returns 8.
 * Byte-exact on this unit's 2.9 arm (task #1660). */
extern void AppendGsRegPacket(s32 regId, u64 value);
extern void MapDraw(int useHudPass, long applyScissor);
s32 func_002DA4B0(void) {
    AppendGsRegPacket(0x42, 0x44);
    AppendGsRegPacket(0x47, 0xB);
    MapDraw(1, 0);
    return 8;
}

/* Draw the title-screen main menu.
 *
 * Measures the five fixed option strings (0x2BF3, 0x2BFF, 0x2C00, 0x2C01,
 * 0x2BE5) in font 2 and keeps the widest. When a save exists
 * (g_playerProgress != 0) it also measures 0x2BF4. It centres that width in
 * the widget (+0x20, x clamped to >= 2) and draws the five options in the debug
 * font down the widget, in rows of +0x24 / (save ? 7 : 6). The save row is
 * measured and leaves a row slot, but it is never drawn here.
 *
 * obj: the menu widget (+0x20 width, +0x24 height).
 * Returns 2.
 *
 * Shape (task #1548): the ROM draws the five rows as five straight-line calls
 * and reads g_playerProgress at each of its two uses. The earlier body drew
 * them through a `static const` id table in a loop and cached the flag at entry.
 * The s136os splice REFUSES that form outright (DEFINITION: the table lives
 * outside the function block).
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, task #1623):
 * verify_match_unit BYTE IDENTICAL 148/148, st_size 592 = ROM 0x250. Devices,
 * each measured necessary: DrawDebugString's colour declared u64 below
 * (u32 gives lui/ori, one word short per row), and one empty tied fence
 * before the last call (without it, 3/148: that call's arg moves come out
 * a0,a1,a3 where the ROM has a3,a0,a1 at 0x2DA6F4..0x2DA6FC). Three
 * re-spellings had left those 3 words unchanged (task #1548): a trailing dead
 * `y += rowStep`, a named temporary for the last string, and #1510's
 * `char *str, s64 wrap` prototype. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DA4F0)
S136OS_SLOT(func_002DA4F0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void AppendGsRegPacket(s32 regId, u64 value);
extern s32 GetLocalizedString(s32 id);
extern s32 func_0027F818(const char *str, s32 maxChars); /* MeasureFont2Text */
extern void func_0027F7A0(void);                          /* DisableInlineColorCodes */
extern s32 func_0027F790(void);                           /* EnableInlineColorCodes */
/* EE arm: the ROM passes the colour zero-extended in a 64-bit register
 * (ori/dsll/ori, not lui/ori), which only a 64-bit parameter type reproduces;
 * 1CA080.cpp declares it the same way (task #1510). Native keeps the unit-wide
 * u32 declaration, so no C++ overload is introduced there. */
#ifndef TARGET_NATIVE
extern void DrawDebugString(s32 x, s32 y, u64 color, s32 str, s32 wrap); /* DECL-LEVER(#1548): defined s32 colour; the ROM caller passes it zero-extended */
#endif
/* (end of this body's declarations) */
s32 func_002DA4F0(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 widest, width, x, rowStep, y, label;

    AppendGsRegPacket(0x42, 0x44);
    AppendGsRegPacket(0x47, 0xB);
    Begin2dDrawBatch(0);

    widest = func_0027F818((const char *)GetLocalizedString(0x2BF3), -1);
    width = func_0027F818((const char *)GetLocalizedString(0x2BFF), -1);
    if (width >= widest) widest = width;
    if (g_playerProgress != 0) {
        width = func_0027F818((const char *)GetLocalizedString(0x2BF4), -1);
        if (width >= widest) widest = width;
    }
    width = func_0027F818((const char *)GetLocalizedString(0x2C00), -1);
    if (width >= widest) widest = width;
    width = func_0027F818((const char *)GetLocalizedString(0x2C01), -1);
    if (width >= widest) widest = width;
    width = func_0027F818((const char *)GetLocalizedString(0x2BE5), -1);
    if (width >= widest) widest = width;

    x = (*(s32 *)(o + 0x20) - widest) >> 1;
    if (x < 2) x = 2;
    rowStep = *(s32 *)(o + 0x24) / (g_playerProgress != 0 ? 7 : 6);

    func_0027F7A0();
    y = rowStep - 6;
    DrawDebugString(x, y, 0x80FFA888, GetLocalizedString(0x2BF3), -1);
    y += rowStep;
    DrawDebugString(x, y, 0x80FFA888, GetLocalizedString(0x2BFF), -1);
    y += rowStep;
    DrawDebugString(x, y, 0x80FFA888, GetLocalizedString(0x2C00), -1);
    y += rowStep;
    DrawDebugString(x, y, 0x80FFA888, GetLocalizedString(0x2C01), -1);
    y += rowStep;
    label = GetLocalizedString(0x2BE5);
    /* Scheduling device (RULING #8483, emits nothing): ties x, y and the last
     * label so cc1 issues the label's move to $a3 ahead of the x/y moves, as
     * the ROM does before this call only (a3,a0,a1 at 0x2DA6F4). Without it
     * the three moves come out a0,a1,a3. Operand order matters: with the label
     * listed first, the label's move sinks below the colour instead. */
    __asm__("" : "+r"(x), "+r"(y), "+r"(label));
    DrawDebugString(x, y, 0x80FFA888, label, -1);
    func_0027F790();
    End2dDrawBatch();
    return 2;
}
#endif

/* Draw the galactic map's active-objectives panel: vtable slot 1 (draw) of
 * the records at 0x25A610 and 0x25E790, whose slot 0 is
 * HandleGalacticMapPlanetSelectInput and slot 2 func_002D8270. Lists every
 * objective GatherActiveObjectives reports for the current level as a
 * localized string under the 0x2C5B header, highlights the selected one,
 * keeps it scrolled into view, and ticks the done ones.
 *
 * widget: the MenuWidget. +0x18/+0x1C origin (used unless D_1AB930 is set,
 * else 4,4), +0x20/+0x24 width/height of the clip box, +0x30 the selected
 * objective per level (indexed by g_mapVertexData+0x230, the map cache's
 * current level), +0xA4 the scroll offset in pixels. Returns 2.
 *
 * GatherActiveObjectives writes the objective string ids to the scratchpad
 * at 0x70000000 and a done bit per row to doneMask. Each row is drawn
 * through func_00280BB8 with a copy of a 0x18-byte text-box layout (clip box
 * top/bottom/left/right, text x/y), which reports the drawn block's height
 * back at +0xE; rows advance by 16 pixels per line (+0x10). For the selected
 * row the text colour is 0x8020FFFF instead of 0x80FFA888, inline colour
 * codes are switched off around it (func_0027F7A0 / func_0027F790), and the
 * scroll offset is nudged 4 pixels toward keeping the row inside the box.
 * A row's checkbox (func_002DAA50) is drawn only when it lies inside the box.
 * With D_1ABB90 set the clip box gets a 0x40-pixel 0x202020 frame
 * (func_0027F208).
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, task #2090). The
 * ROM's `addiu $23,$23,%lo(D_70000004)` is splat's symbolisation of the
 * scratchpad pointer step 4; the C emits `addiu $23,$23,4`, the same word
 * once linked. Each device below is a REGISTER-PIN DEVICE (EE_REG, RULING
 * #8598) or an empty tied fence (RULING #8483), priced by removing it alone
 * (in-tree splice, words differing of 196, built length in brackets; 1 is
 * the relocation row above):
 *   - selected pinned to $17: cc1 sets a temporary and copies it into the
 *     long-lived register; the ROM's sltiu writes $17 directly. 113 (197).
 *   - lines pinned to $16: the quotient is mflo'd straight into the
 *     long-lived register in the ROM (the AllocMenuWorkBuffer / FACT #9913
 *     quotient copy). 43 (197).
 *   - shownLevel pinned to $3: 10.
 *   - the numerator (height) pinned to $2: with lines pinned it would
 *     otherwise share $16. 7.
 *   - the line step pinned to $2: with lines pinned, `lines << 4` lands in
 *     $16 and reorg annuls the branches around the checkbox call. 7.
 *   - the empty tied fence on shownLevel after the XOR: without it the XOR
 *     result is given $17. 3.
 *   - func_00280BB8's tint declared s64 (DECL-LEVER; its C definition in
 *     178E88 takes s32): as s32, box.x and box.y are formed in each other's
 *     registers. 6. Its box declared void * or s32 makes no difference.
 * Spellings that carried it, each measured: g_mapVertexData through a local
 * (the ROM forms the base, then +0x230); the scratchpad ids as a pointer
 * local indexed by i (loop.c's giv starts after the entry test, as the
 * ROM's $23 does; a pointer bumped by the loop starts before it); the
 * per-level index added as widget + (level << 2) + 0x30 (the ROM's
 * `addu $2,$18,$2`); x + 4 formed inside the loop, so loop.c hoists it
 * past the entry test and spills it to $sp+0x44; the colour and the
 * func_0027F7A0 call in two separate ifs (one if loses the movn); y
 * initialised before x (the prologue's issue order); bottom stored before
 * right in the layout. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_DrawActiveObjectivesList)
S136OS_SLOT(DrawActiveObjectivesList);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void *memset(void *s, s32 c, u32 n);
extern s32 D_1AB930;
extern s32 D_1ABB90;
extern u8  g_mapVertexData[];
extern s32 GatherActiveObjectives(s32 outIds, s32 outMask, s32 outVals, s32 wantValues);
extern void AppendGsRegPacket(s32 regId, u64 value);
extern s32 GetLocalizedString(s32 id);
#ifndef TARGET_NATIVE
/* The ROM symbol with its ROM signature (a 64-bit colour, loaded with a dli);
 * the native build keeps the five-s32 declaration declared above. */
extern void func_0027FBA8(s32 x, s32 y, u64 color, s32 str, s64 wrap);
#endif
extern void func_0027F208(s32 top, s32 bottom, s32 left, s32 right, s32 thickness, s32 color);
extern void func_0027F7A0(void);
extern s32 func_0027F790(void);
extern void func_00280BB8(s32 box, s64 tint, s32 str, s32 mode); /* DECL-LEVER(#2090): defined s32 tint; an s64 tint is the ROM's $2/$3 colouring at the box.x/box.y stores (s32: 6 of 196) */
extern void func_002DAA50(s32 x, s32 y, s32 on);
/* (end of this body's declarations) */
typedef struct ObjectiveTextBox {
    s16 top;         /* 0x00 clip box */
    s16 bottom;      /* 0x02 */
    s16 left;        /* 0x04 */
    s16 right;       /* 0x06 */
    s16 x;           /* 0x08 text position */
    s16 y;           /* 0x0A */
    s16 _0C;
    s16 height;      /* 0x0E drawn block height, written back by func_00280BB8 */
    s16 lineHeight;  /* 0x10 */
    s16 _12[3];
} ObjectiveTextBox;
s32 DrawActiveObjectivesList(MenuWidget *widget) {
    u8 *w = (u8 *)widget;
    s32 doneMask = 0;
    s32 count;
    s32 y = 4;
    s32 x = 4;
    s32 i;
    s32 checkX;
    ObjectiveTextBox box, layout;
    s32 *objectiveIds = (s32 *)0x70000000;

    Begin2dDrawBatch(0);
    count = GatherActiveObjectives((s32)objectiveIds, (s32)&doneMask, 0, 0);
    AppendGsRegPacket(0x42, 0x44);
    AppendGsRegPacket(0x47, 0x10B);
    if (D_1AB930 == 0) {
        x = *(s32 *)(w + 0x18);
        y = *(s32 *)(w + 0x1C);
    }
    func_0027FBA8(x, y, 0x80FFA888, GetLocalizedString(0x2C5B), -1);
    y += 0x14;
    memset(&layout, 0, sizeof(layout));
    layout.bottom = *(s32 *)(w + 0x24);
    layout.right = *(s32 *)(w + 0x20);
    layout.lineHeight = 0x10;
    layout.top = y;
    layout.left = x;
    box = layout;
    if (D_1ABB90 != 0) {
        func_0027F208(y, *(s32 *)(w + 0x24), x, *(s32 *)(w + 0x20), 0x40, 0x202020);
    }
    for (i = 0; i < count; i++) {
        s32 color = 0x80FFA888;
        s32 rowY = y + 0xA;
        u8 *mapCache = g_mapVertexData;
        register s32 shownLevel EE_REG("$3") = *(s32 *)(w + (*(s32 *)(mapCache + 0x230) << 2) + 0x30);
        register s32 selected EE_REG("$17");
        register s32 lines EE_REG("$16");
        s32 checkY;

        shownLevel ^= i;
        __asm__("" : "+r"(shownLevel));
        selected = shownLevel == 0;
        checkX = x + 4;
        if (selected) {
            color = 0x8020FFFF;
        }
        if (selected) {
            func_0027F7A0();
        }
        box.x = x + 0x10;
        box.y = y - *(s32 *)(w + 0xA4);
        func_00280BB8((s32)&box, color, GetLocalizedString(objectiveIds[i]), -1);
        if (selected) {
            if (box.y < box.top + 4) {
                *(s32 *)(w + 0xA4) -= 4;
            }
            if (box.bottom - box.height - 4 < box.y) {
                *(s32 *)(w + 0xA4) += 4;
            }
            func_0027F790();
        }
        {
            register s32 height EE_REG("$2") = box.height;
            lines = height / box.lineHeight;
        }
        if (lines == 2) {
            rowY += 6;
        }
        checkY = rowY - *(s32 *)(w + 0xA4);
        if (checkY >= box.top + 4 && checkY <= box.bottom - 4) {
            func_002DAA50(checkX + 5, checkY, (doneMask >> i) & 1);
        }
        {
            register s32 step EE_REG("$2") = lines << 4;
            y += step;
        }
    }
    End2dDrawBatch();
    return 2;
}
#endif

/* Draw a small checkbox/indicator at (x,y): a 10px highlight rect, an 8px inner
 * rect, and (when `on`) a 0x1E-px tick glyph on top. */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): its three callees' prototypes
 * repeated in the arm (the unit-wide ones are native-only). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DAA50)
S136OS_SLOT(func_002DAA50);
#else
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 73.46% / engine96 68.98%; better arm sdk29; 27 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-64`. Not iterated in t495. */
/* The unit-wide callee prototypes above are native-only, so the s136os TU
 * (which compiles this arm alone) repeats the ones it calls. The tick glyph is
 * drawn by func_0028F0D0 (= DrawHudIconQuadTiled, defined under that ROM name in
 * 188858.c); the inner-rect colour is the %gp word D_1AA45C (the ROM's
 * `lw $8,%gp_rel(D_1AA45C)`; this arm read D_1ABB6C before task #1325). */
extern void func_0028F0D0(s32 iconIndex, s32 x, s32 y, s32 w, s32 h, s32 alpha);
extern s32  func_0028EDF0(s32 name, s32 level);
extern void func_002904B0(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 mode);
extern s32  D_1AA45C;
void func_002DAA50(s32 x, s32 y, s32 on) {
    func_002904B0(x - 5, y - 5, x + 5, y + 5, 0x80ffa888, 0);
    func_002904B0(x - 4, y - 4, x + 4, y + 4, D_1AA45C, 0);
    if (on) {
        func_0028F0D0(func_0028EDF0(0xe99d, 1), x - 0xd, y - 0x12, 0x1e, 0x1e, 0x80);
    }
}
#endif

/* Per-frame tick of the streamed-image widget: vtable slot 0 of the records at
 * 0x25A5A8 and 0x25E598, whose other slots are func_002DAE70 (draw),
 * InitMenuBgImageBuffers (init) and func_002DB028 (release). It picks which
 * image to show, streams it from disc into the widget's work buffer, and once
 * the read completes decompresses it and queues its texture upload.
 *
 * widget: the MenuWidget. Fields this tick uses (offsets into the opaque record):
 *   +0x30  slot table: StreamImageSlot[] {lbn, sectors}, indexed by image
 *   +0x34  flags: 0x1 index from +0x58 (-1 = nothing to show), 0x2 the
 *          current map level, 0x4 the focused widget's +0x3C, 0x100 an area
 *          record's image (see below), else the focused widget's +0x40;
 *          0x20 compressed image, 0x10000 / 0x10 select the LBN base,
 *          0x40000 hold at least 5 frames before finishing a load
 *   +0x44  load phase: 0 / 2 start a read, 1 / 3 wait for it, -1 failed
 *   +0x48  image work buffer
 *   +0x50  index of the image last read
 *   +0x5C  frame counter, saturating at 0x100
 *   +0x60  offset of the compressed image within the work buffer
 * Returns 0.
 *
 * In the 0x100 case the focused widget's +0x40 (clamped to 0..4) indexes the
 * 0x1C-byte records at g_areaTable+0x30, but only while g_areaTable+0x15C < 3,
 * g_areaTable+0x164 < 0 and g_menuScreenBlock+0x164 >= 11; otherwise the phase
 * is set to -1. A read starts only when the image changed, the buffer exists,
 * no file load is in flight and the slot has sectors; a compressed image is
 * read into the END of the buffer (GetMenuWorkBufferSize minus its size) and
 * DecompressWad expands it in place to the start. The image is a 256-colour
 * texture: width and height at +0x8 / +0xC (QueueGsTextureUpload takes their
 * log2), the CLUT at +0x20, the pixels at +0x420; the returned GS register
 * word is stored at g_mapVertexData+0x268. Both LBN-base branches that use
 * g_globalWadBaseLbn are kept separate, as the ROM has two copies of that call.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, task #2090), with no
 * device. Two spellings carry it (solo s136 compile, in-tree splice, words
 * differing of 221): g_menuScreenBlock and g_areaTable held in locals, so the
 * address is formed once and offset (inline, cse folds +0x15C into the base
 * and the 0x1C-stride record reads at -300: 217 words); the CLUT and pixel
 * pointers formed before the two Log2Floor calls (inline in the call, the
 * frame loses a callee-saved register: 217 words). The buffer copied into a
 * separate destination before the size adjustment: 3 words as one variable. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_TickStreamedImageWidget)
S136OS_SLOT(TickStreamedImageWidget);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 g_mapCurrentLevel[];
extern u8  g_menuScreenBlock[];
extern u8  g_areaTable[];
extern s16 g_fileLoadState ROM_SPLIT;
extern s32 D_14D11C ROM_SPLIT;            /* g_discToc+0x1BDC: alternate LBN base */
extern s32 g_globalWadBaseLbn ROM_SPLIT;  /* g_discToc+0x36C: global WAD LBN base */
extern u8  g_mapVertexData[];
extern s32 GetMenuWorkBufferSize(s32 id);
extern s32 StartFileLoad(s32 dest, s32 lbn, s32 sectors);
extern s32 func_002DF560(s32 id);
extern s32 func_002DF5B0(s32 id);
extern void DecompressWad(void *src, void *dest);
extern s32 Log2Floor(s32 x);
extern u64 QueueGsTextureUpload(s32 logW, s32 logH, s32 clut, s32 pixels, s32 clutDims, s32 texDims);
extern void FlushPendingTexUploads(void);
/* (end of this body's declarations) */
typedef struct StreamImageSlot {
    s32 lbn;      /* 0x0: LBN relative to the selected base */
    s32 sectors;  /* 0x4: length in 2 KB sectors, 0 = no image */
} StreamImageSlot;
#define STREAM_SLOT(w, i) (*(StreamImageSlot **)((w) + 0x30))[i]
s32 TickStreamedImageWidget(MenuWidget *widget) {
    u8 *w = (u8 *)widget;
    s32 flags = *(s32 *)(w + 0x34);
    s32 image;

    if (flags & 0x1) {
        image = *(s32 *)(w + 0x58);
        if (image == -1) {
            return 0;
        }
    } else if (flags & 0x2) {
        image = g_mapCurrentLevel[0];
    } else if (flags & 0x4) {
        image = *(s32 *)(*(u8 **)((u8 *)g_pCurrentMenuScreen[0] + 0xE8) + 0x3C);
    } else if (flags & 0x100) {
        u8 *block = g_menuScreenBlock;
        u8 *area = g_areaTable;
        image = *(s32 *)(*(u8 **)(*(u8 **)(block + 0x14) + 0xE8) + 0x40);
        if (image < 0) image = 0;
        if (image >= 5) image = 4;
        if (*(s32 *)(area + 0x15C) < 3 && *(s32 *)(area + 0x164) < 0
            && *(s32 *)(block + 0x164) >= 0xB) {
            if (*(s32 *)(w + 0x44) == -1) {
                *(s32 *)(w + 0x44) = 0;
            }
            image = *(s32 *)(area + image * 0x1C + 0x30);
        } else {
            *(s32 *)(w + 0x44) = -1;
        }
    } else {
        image = *(s32 *)(*(u8 **)((u8 *)g_pCurrentMenuScreen[0] + 0xE8) + 0x40);
        if (image < 0) image = 0;
    }

    switch (*(s32 *)(w + 0x44)) {
    case 0:
    case 2:
        if (image != *(s32 *)(w + 0x50)) {
            s32 buffer = *(s32 *)(w + 0x48);
            if (buffer != 0 && g_fileLoadState == 0 && STREAM_SLOT(w, image).sectors != 0) {
                s32 dest = buffer;
                s32 started;
                if (*(s32 *)(w + 0x34) & 0x20) {
                    *(s32 *)(w + 0x60) = GetMenuWorkBufferSize(buffer)
                                         - (STREAM_SLOT(w, image).sectors << 11);
                    dest += *(s32 *)(w + 0x60);
                }
                if (*(s32 *)(w + 0x34) & 0x10000) {
                    started = StartFileLoad(dest, STREAM_SLOT(w, image).lbn + D_14D11C,
                                            STREAM_SLOT(w, image).sectors);
                } else if (*(s32 *)(w + 0x34) & 0x10) {
                    started = StartFileLoad(dest, STREAM_SLOT(w, image).lbn + g_globalWadBaseLbn,
                                            STREAM_SLOT(w, image).sectors);
                } else {
                    started = StartFileLoad(dest, STREAM_SLOT(w, image).lbn + g_globalWadBaseLbn,
                                            STREAM_SLOT(w, image).sectors);
                }
                if (started != 0) {
                    func_002DF560(*(s32 *)(w + 0x48));
                    *(s32 *)(w + 0x50) = image;
                    *(s32 *)(w + 0x44) = *(s32 *)(w + 0x44) + 1;
                } else {
                    *(s32 *)(w + 0x44) = -1;
                }
                *(s32 *)(w + 0x5C) = 0;
                return 0;
            }
        }
        break;
    case 1:
    case 3:
        if (g_fileLoadState == 0
            && (!(*(s32 *)(w + 0x34) & 0x40000) || *(s32 *)(w + 0x5C) >= 5)) {
            u8 *img;
            u8 *clut, *pixels;
            s32 logW, logH;
            func_002DF5B0(*(s32 *)(w + 0x48));
            if (*(s32 *)(w + 0x34) & 0x20) {
                DecompressWad(*(u8 **)(w + 0x48) + *(s32 *)(w + 0x60), *(u8 **)(w + 0x48));
                *(s32 *)(w + 0x60) = 0;
            }
            img = *(u8 **)(w + 0x48);
            clut = img + 0x20;
            pixels = img + 0x420;
            logW = Log2Floor(*(s32 *)(img + 0x8));
            logH = Log2Floor(*(s32 *)(img + 0xC));
            {
                u8 *vd = g_mapVertexData;
                *(u64 *)(vd + 0x268) = QueueGsTextureUpload(logW, logH, (s32)clut, (s32)pixels,
                                                            *(s32 *)(vd + 0x250), *(s32 *)(vd + 0x254));
            }
            FlushPendingTexUploads();
            *(s32 *)(w + 0x44) = 2;
            *(s32 *)(w + 0x5C) = 0;
            return 0;
        }
        break;
    default:
        return 0;
    }
    *(s32 *)(w + 0x5C) = *(s32 *)(w + 0x5C) + 1;
    if (*(s32 *)(w + 0x5C) > 0x100) {
        *(s32 *)(w + 0x5C) = 0x100;
    }
    return 0;
}
#undef STREAM_SLOT
#endif

/* Draw the streamed full-screen image widget `object` once it has loaded
 * (state +0x44 >= 2 and count +0x58 not negative): blit the decoded glyph quad
 * at the screen size in g_gsScreenContext (+0x160/+0x162), sized by the
 * widget's width/height (+0x38/+0x3C), with the texture word at
 * g_mapTextureWidth+0x28. With flag 0x40000 set in +0x34 the quad fades: alpha
 * = value(+0x5C) * 32 capped at 0x80 in state 2 (fade in), else (4 - value) * 32
 * floored at 0 (fade out); otherwise it is drawn with the fixed colour
 * 0x80808080. Returns 0x10 when drawn, else 0.
 *
 * MATCHED on this unit's plain 2.9 arm (cc1 2.9-ee-991111 -O2 -G8 -fno-gcse;
 * no selector row). The two DrawGlyphQuad calls stay separate, as the ROM has
 * them. The screen context and the texture word are aggregates, so -G8 never
 * makes them small data and cc1 splits their addresses as upstream's -G0 build
 * does (NOTE #12163). Task #2295.
 *
 * Source: OpenRAC/rac2-gc-decomp @96927f872c709e13aaa0f32265bb81a5531bf7ed
 * (GPL-3.0, adopted with the authors' permission per RULING #10405),
 * src/game/002D5E98.c, func_002DA118 (v1.01 address; author commit b413de09,
 * Flavius). ADAPTED: (1) `unsigned long long` / `ULL` written u64 / `UL`
 * (64-bit either way on the EE); (2) our symbols: func_0027E018 =
 * DrawGlyphQuad under this unit's prototype (s32 x 8, u64, u64), D_001A6480 =
 * g_gsScreenContext as a label alias of their view, and D_001C5208 =
 * g_mapTextureWidth+0x28, element 5 of a u64 label alias; (3) their struct
 * types renamed. Statements and control flow are theirs. */
/* The unit-wide DrawGlyphQuad prototype is native-only; the EE compile needs it here. */
extern void DrawGlyphQuad(s32 x, s32 y, s32 w, s32 h, s32 u, s32 v, s32 uw, s32 uh, u64 color, u64 tex0);
typedef struct GlyphScreenSize {
    char pad[0x160];
    short x;   /* 0x160 */
    short y;   /* 0x162 */
} GlyphScreenSize;
typedef struct StreamImageWidget {
    char pad[0x34];
    int flags;    /* 0x34 */
    int width;    /* 0x38 */
    int height;   /* 0x3C */
    char pad2[4];
    int state;    /* 0x44 */
    char pad3[0x10];
    int count;    /* 0x58 */
    int value;    /* 0x5C */
} StreamImageWidget;
extern GlyphScreenSize g_glyphScreenSize __asm__("g_gsScreenContext");
extern u64 g_mapTextureWords[] __asm__("g_mapTextureWidth");

int func_002DAE70(StreamImageWidget *object)
{
    int state = object->state;
    int shade;
    u64 color;

    if (state < 2) {
        return 0;
    }
    if (object->count < 0) {
        return 0;
    }
    if ((object->flags & 0x40000) != 0) {
        if (state == 2) {
            shade = object->value << 5;
            if (shade > 0x80) {
                shade = 0x80;
            }
        } else {
            shade = (4 - object->value) << 5;
            if (shade < 0) {
                shade = 0;
            }
        }
        color = (shade << 24) | 0x808080;
        DrawGlyphQuad(0, 0, g_glyphScreenSize.x, g_glyphScreenSize.y, 0, 0, object->width, object->height, color, g_mapTextureWords[5]);
    } else {
        color = 0x80808080UL;
        DrawGlyphQuad(0, 0, g_glyphScreenSize.x, g_glyphScreenSize.y, 0, 0, object->width, object->height, color, g_mapTextureWords[5]);
    }
    return 0x10;
}

/**
 * Allocate / initialise the menu background-image double buffers of the widget
 * `obj`. Clears the pending flag (+0x44), then reserves two map slots
 * (AllocMenuWorkBuffer) passing the "already-preloaded" bit (+0x34 & 0x200).
 * When NOT preloaded, force-allocates a real slot (AllocMenuWorkBuffer(1)) for
 * either buffer (+0x48, +0x4C) that came back empty. Finally resets the stream
 * cursor (+0x5C = 0) and marks both slot-state fields (+0x50/+0x54) as -1
 * (idle). Returns 0. The release half is func_002DB028.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack; row in
 * tools/ee/s136os_functions.txt), spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. Device-free: no
 * volatile, no register pin, no inline asm. The object is addressed as an
 * integer (`self`), as OpenRAC's body has it; spelled with a pointer, the
 * same arithmetic would be no different to cc1 but the body is kept verbatim.
 *
 * Source: OpenRAC/rac2-gc-decomp @974302ba936bb455acd78034b58ad4b98bafcc6b (GPL-3.0, adopted with the authors' permission per RULING #10405), src/game/002D5E98.c, func_002DA238 (v1.01), C body adopted verbatim apart from the callee spelling (their func_002DE550 = AllocMenuWorkBuffer) and the parameter spelling (`void *obj` + `s32 self = (s32)obj` where theirs takes `s32 self`; bytes unaffected, #2161); byte-exact on our ROM: vmu BYTE IDENTICAL 38/38 words (4 relocs resolved; base-seeded bit flip rc 1 at 0x2dafb0), unit objdiff report 100.00% size=148 (objdiff_build.sh + unit_report.sh, clean), whole-image cmp 0 under landing_gate usa and eu --strict (task #2159)
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_InitMenuBgImageBuffers)
S136OS_SLOT(InitMenuBgImageBuffers);
#else
s32 InitMenuBgImageBuffers(void *obj) {
    s32 self = (s32)obj;

    *(s32 *) (self + 68) = 0;
    *(s32 *) (self + 72) = AllocMenuWorkBuffer (*(s32 *) (self + 52) & 0x200);
    *(s32 *) (self + 76) = AllocMenuWorkBuffer (*(s32 *) (self + 52) & 0x200);
    if ((*(s32 *) (self + 52) & 0x200) == 0)
      {
        if (*(s32 *) (self + 72) == 0)
          *(s32 *) (self + 72) = AllocMenuWorkBuffer (1);
        if (*(s32 *) (self + 76) == 0)
          *(s32 *) (self + 76) = AllocMenuWorkBuffer (1);
      }
    *(s32 *) (self + 92) = 0;
    *(s32 *) (self + 80) = -1;
    *(s32 *) (self + 84) = -1;
    return 0;
}
#endif

extern void PumpDialogVoiceSystem(s32 blocking);

/* Reset a two-slot streaming text widget: toggle both map slots referenced by
 * obj->0x48/0x4C, invalidate the cached state (+0x44/0x50/0x54 = -1) and kick
 * the dialog-voice pump. Returns 0.
 * MATCHED on the s136os arm (SN 1.36 cc1plus, FACT #8852).
 * GUARD (task #1313): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DB028)
S136OS_SLOT(func_002DB028);
#else
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 99.14% / engine96 70.14%; better arm sdk29; 10 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. Not iterated in t495. */
s32 func_002DB028(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    *(s32 *)(o + 0x48) = FreeMenuWorkBuffer(*(s32 *)(o + 0x48));
    *(s32 *)(o + 0x4C) = FreeMenuWorkBuffer(*(s32 *)(o + 0x4C));
    /* SN 1.36's scheduler emits the LAST of these three independent stores
     * first, so this order yields the ROM's +0x44, +0x50, +0x54 (task #1313). */
    *(s32 *)(o + 0x50) = -1;
    *(s32 *)(o + 0x54) = -1;
    *(s32 *)(o + 0x44) = -1;
    PumpDialogVoiceSystem(1);
    return 0;
}
#endif

/* UpdateMenuResourceStream (named in task #2244 from OpenRAC rac2-gc-decomp
 * @96927f87): double-buffered menu asset streamer (reader of g_skillPointFlags).
 * Six StartFileLoad calls, one DecompressWad, g_globalWadBaseLbn and
 * g_fileLoadState (FACT #12086, NOTE #5807). The earlier "help/hint/objective
 * text selector" reading is contradicted by those callees. Wall: ~350-instruction
 * nested branch + switch state graph over the partially-recovered slot table.
 * Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", UpdateMenuResourceStream);

/* Draw the streamed help/preview image (loading/autosave overlay variants).
 * PARK (flag-not-guess): builds a text-box descriptor on the stack (sh fields at
 * $sp+0x30..) then copies it via unaligned ldl/ldr/sdl/sdr into $sp+0x10 and
 * re-draws through func_00280BB8 with per-field decrements (drop-shadow) — the
 * 128-bit packed descriptor's field roles are only partially recovered, so a
 * portable #else would guess them (silent-bug risk, #else not byte-gated). The
 * bottom DrawGlyphQuad path is transparent, but the fn can't be split. The
 * descriptor was named as a blocker shared with func_002DA740/002D9178/002DD450;
 * its roles are now recovered from func_002DA740, byte-exact as
 * DrawActiveObjectivesList since task #2090 (clip box top/bottom/left/right at
 * +0/+2/+4/+6, text x/y at +8/+0xA, drawn height read back at +0xE, line height
 * at +0x10). This function's measured wall is different: the ROM keeps
 * %hi(g_menuScreenBlock) in $16 across Begin2dDrawBatch and re-forms %lo at each
 * use, which SN 1.36 cc1 emits only with gcse on (task #2090, NOTE #10224,
 * which holds the best body). This unit's s136os arm inherited -fno-gcse then;
 * it compiles with gcse on since task #2252. NOTE #10224 measured that body
 * with gcse on (a local flag edit): 218 words against 219, three residuals
 * left (return-1 tail placement, the entry index, a store order). Bare
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DB700);

/* Load a menu background-image pair (front/back) from disc into the bg buffers.
 * Per-tick phase machine on obj->0x44: phase 0 kicks a StartFileLoad of the first
 * buffer (obj+0x48) from the active language's disc-TOC bg entry (base LBN
 * g_discToc+0x36C + per-index offset at g_discToc + g_menuBgImageIndex*8 + 0xDD0,
 * sector count +0xDD4); phase 1 waits for the load to finish (g_fileLoadState==0)
 * then advances; phase 2 kicks the second buffer (obj+0x4C, offsets +0xDF8/+0xDFC);
 * phase 3 waits then advances to 4 (done). A kick that fails to start sets phase
 * -1. Each phase is a no-op (returns 0) while its buffer is empty or a load is
 * still in flight. Always returns 0.
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack), task #2289.
 * What closed it, after tasks #1917/#1929 left it at 10/83 (NOTE #9929):
 *   - gcse on (task #2252). The ROM forms the TOC entry once, reads +0xDD0 from
 *     it and reads +0xDD4 through a COPY of it (`move $2,$6`); that copy is
 *     gcse's;
 *   - the TOC read through a typed view (BgDiscToc) of g_discToc, used
 *     directly. Raw byte offsets let cse fold +0xDD0 into %lo(g_discToc), and
 *     a local holding the base merges both entry addresses so the copy goes.
 * g_fileLoadState and g_menuBgImageIndex are compiler-split (ROM_SPLIT, the
 * ROM's lui/lh and lui/lw into other registers).
 * GUARD: compiled alone by SN 2.95.3 v1.36 -fopt-stack and spliced over the
 * S136OS_SLOT line (tools/ee/s136os_splice.sh); plain C on native. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_LoadMenuBgImagePair)
S136OS_SLOT(LoadMenuBgImagePair);
#else
extern u8  g_discToc[];                    /* 0x14B540 master disc asset directory */
extern s16 g_fileLoadState ROM_SPLIT;      /* 0x1A63AC 0 idle / nonzero busy */
extern s32 g_menuBgImageIndex ROM_SPLIT;   /* active-language bg image index */
extern s32 StartFileLoad(s32 dest, s32 lbn, s32 sectors);  /* 0x2B8A38 kick async CD read; returns sectors<<11 (0 = refused) */

/* The parts of the disc TOC this loader reads. The bg tables are 8-byte
 * {lbn offset, sector count} records; the front table runs to 0xDF8, where the
 * back table starts (5 records' spacing; the index count is not established). */
typedef struct { s32 lbn; s32 sectors; } BgTocEntry;
typedef struct {
    u8 _000[0x36C];
    s32 baseLbn;                 /* 0x36C first LBN of the asset area */
    u8 _370[0xDD0 - 0x370];
    BgTocEntry front[5];         /* 0xDD0 */
    BgTocEntry back[5];          /* 0xDF8 */
} BgDiscToc;
#define bgToc ((BgDiscToc *)g_discToc)

s32 LoadMenuBgImagePair(void *obj) {
    u8 *o = (u8 *)obj;
    s32 dest;

    switch (*(s32 *)(o + 0x44)) {
    case 0:
        dest = *(s32 *)(o + 0x48);
        if (dest != 0) {
            if (g_fileLoadState != 0)
                break;
            if (StartFileLoad(dest, bgToc->front[g_menuBgImageIndex].lbn + bgToc->baseLbn,
                              bgToc->front[g_menuBgImageIndex].sectors) != 0)
                *(s32 *)(o + 0x44) = *(s32 *)(o + 0x44) + 1;
            else
                *(s32 *)(o + 0x44) = -1;
        }
        break;
    case 1:
        if (g_fileLoadState != 0)
            break;
        *(s32 *)(o + 0x44) = 2;
        break;
    case 2:
        dest = *(s32 *)(o + 0x4C);
        if (dest != 0) {
            if (g_fileLoadState != 0)
                break;
            if (StartFileLoad(dest, bgToc->back[g_menuBgImageIndex].lbn + bgToc->baseLbn,
                              bgToc->back[g_menuBgImageIndex].sectors) != 0)
                *(s32 *)(o + 0x44) = *(s32 *)(o + 0x44) + 1;
            else
                *(s32 *)(o + 0x44) = -1;
        }
        break;
    case 3:
        if (g_fileLoadState != 0)
            break;
        *(s32 *)(o + 0x44) = 4;
        break;
    }
    return 0;
}
#undef bgToc
#endif

/* Draw the loaded menu background-image pair (front/back) as two textured quads.
 * No-op (returns 0) until both disc loads have completed (obj->0x44 >= 4). Opens a
 * 2d draw batch, then for each buffer looks up its GS texture handle
 * (func_002954F0(obj+0x48 / obj+0x4C)) and blits a full 0x100x0x100 quad at the
 * layout coords in D_1ABBB0.. / D_1ABBC0.. with the shared tint 0x60A09080, then
 * closes the batch. Returns 8.
 * NOTE: DrawGlyphQuad reads its two trailing stack args as 64-bit (ld at +0x10/
 * +0x18 of its frame), so the tint + texture handle are passed as s64 to reproduce
 * the sd marshal — traced from DrawGlyphQuad.s @0x27E698.
 * WALL: GS/VIF packet build + sq/lq 128-bit DMA tags; matching arm stays
 * INCLUDE_ASM, portable #else below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", UploadMenuBgImagePair);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void DrawGlyphQuad(s32 x, s32 y, s32 w, s32 h, s32 u, s32 v, s32 uw, s32 uh, u64 color, u64 tex0);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 51.38% / engine96 59.92%; better arm engine96; 46 differing
 * rows on it, class STRUCTURAL; first differing insn: ROM `(none)` vs `sd
 * s0,16(sp)`. Not iterated in t495. */
extern s32  func_002954F0(s32 buffer);   /* 0x2954F0 resolve a bg buffer to its GS texture handle */
/* DrawGlyphQuad (0x27E698) is used unprototyped elsewhere in this unit (C89
 * implicit decl); the (s64) casts on the trailing tint/texture args force the
 * 64-bit stack marshal DrawGlyphQuad reads via ld — so no explicit prototype here
 * (it would conflict with the existing implicit uses). */
extern s32 D_1ABBB0, D_1ABBB4, D_1ABBB8, D_1ABBBC;   /* front-quad layout params */
extern s32 D_1ABBC0, D_1ABBC4, D_1ABBC8, D_1ABBCC;   /* back-quad layout params */

s32 UploadMenuBgImagePair(void *obj) {
    s64 tint = (s64)0x60A09080;

    if (*(s32 *)((u8 *)obj + 0x44) < 4) {
        return 0;
    }

    Begin2dDrawBatch(0);
    DrawGlyphQuad(D_1ABBB0, D_1ABBB4, D_1ABBB8, D_1ABBBC, 0, 0, 0x100, 0x100,
                  tint, (s64)func_002954F0(*(s32 *)((u8 *)obj + 0x48)));
    DrawGlyphQuad(D_1ABBC0, D_1ABBC4, D_1ABBC8, D_1ABBCC, 0, 0, 0x100, 0x100,
                  tint, (s64)func_002954F0(*(s32 *)((u8 *)obj + 0x4C)));
    End2dDrawBatch();
    return 8;
}
#endif

/* DrawMenuPreviewImage (named in task #2244 from OpenRAC rac2-gc-decomp
 * @96927f87): the rect is large for a cursor, 0x1D..0xBF x 0x4D..0xB2 on the
 * D_25E660 screen and 0x1E..0x99 x 0x4C..0x8C elsewhere, drawn from the two TEX0
 * records at g_mapVertexData+0x268/+0x278 through func_0028FAE0, so "preview
 * image" fits the ROM better than the cursor reading kept below.
 * Draw the animated galactic-map planet-cursor sprite at the active slot, with
 * a pulsing scale driven by the global frame counter and a layout that differs
 * for the save-screen vs map-screen instances. Returns 4 when drawn, else 0.
 * WALL (empirically confirmed 2026-07-14, canonical-2.9 objdiff): engine-2.96
 * SAVE-SLOT wall — the ROM packs 9 GPR + 4 FPR saves at 8-byte spacing (frame
 * 0x70); the pinned 2.9 cc1 reserves 16-byte slots (frame 0xA0). Not
 * C-controllable on that cc1 → no 2.9 match, #else is correct (prior "const-layout"
 * note was imprecise). Secondary near-miss: the ROM reads currentLevel/activeSlot
 * as g_mapVertexData struct fields (+0x230/+0x234, absolute) where the #else uses
 * the gp-rel g_nMapCurrentLevel/g_nMapActiveSlot aliases — a real modeling lever,
 * but on 2.9 it cannot overcome the save-slot wall.
 * Save stride re-measured on the s136os arm (SN 1.36 -fopt-stack): saves land at the ROM's
 * 8-byte stride, but the saved register set/count differs from the ROM's (NOTE #9871) - an
 * allocation/frame difference, not the stride. So the save-slot wall named above is a cc1 2.9
 * property and was measured false as the reason this member stays unmatched (FACT #9873).
 * Residual: the saved-register set; the rest UNMEASURED. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", DrawMenuPreviewImage);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void AppendGsRegPacket(s32 regId, u64 value);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 47.90% / engine96 37.83%; better arm sdk29; 96 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-112` vs `addiu sp,sp,-160`. Not iterated in t495. */
/* Matching arm stays INCLUDE_ASM; #else is the structure model (faithful vs asm,
 * cross-checked against the EU twin func_002DBC60). */
s32 DrawMenuPreviewImage(void) {
    extern s32 D_00261978[];
    extern s32 D_001B1518;
    extern void *D_001C5188, *D_001C5198;
    extern void DrawHudSpriteTex0(void *tex, s32 x, s32 y, s32 a, s32 b,
                                  s32 w, s32 h, s32 phase);
    s32 lvl = g_nMapCurrentLevel;
    s32 slot = g_nMapActiveSlot;
    s32 box[4];
    s32 x, y, w, h, kind, phase, phaseFix, off;
    if (slot < 0 || lvl == 5 || lvl == 10 || lvl == 0xf ||
        (lvl > 0x14 && lvl != 0x18)) {
        return 0;
    }
    if (g_pCurrentMenuScreen[0] == (void *)D_25E660) {
        box[1] = 0x32; box[2] = 0x26; box[3] = 0x19;
        x = 0x1d; y = 0x4d; w = 0xbf; h = 0xb2; box[0] = 0;
    } else {
        box[1] = 0x26; box[2] = 0x1c; box[3] = 0x14;
        x = 0x1e; y = 0x4c; w = 0x99; h = 0x8c; box[0] = 0;
    }
    box[0] = 0;
    kind = D_00261978[slot];
    phase = D_001B1518 + slot * 0x2ab;
    phaseFix = (phase < 0) ? (phase + 0x7ff) : phase;
    off = box[kind];
    AppendGsRegPacket(0x47, 0);
    AppendGsRegPacket(8, 0);
    if (kind == 0) {
        DrawHudSpriteTex0(D_001C5188, x << 4, y << 4, 7, 7, w << 4, h << 4, 0);
    } else {
        DrawHudSpriteTex0(D_001C5188, (x + off) * 0x10, (y + off) * 0x10, 7, 7,
                          (w + off * -2) * 0x10, (h + off * -2) * 0x10,
                          phase + (phaseFix >> 0xb) * -0x800);
        AppendGsRegPacket(8, 5);
        DrawHudSpriteTex0(D_001C5198, x << 4, y << 4, 7, 7, w << 4, h << 4, 0);
    }
    AppendGsRegPacket(0x47, 0x360b);
    return 4;
}
#endif

/* Galactic-map / list-screen cursor + select input handler. While the per-obj
 * fade-in counter (obj+0x3C) is running it decrements it and drives the black
 * screen fade (g_screenFadeBlack = min(count-1,4) * 0.25), returning 0. Once the
 * fade is done, if this obj is the focused widget it processes the pad:
 *   - L1/R1 (0x900): return 1 unless the screen-block override (+0x134) is set.
 *   - cancel (0x10): standard back-nav (screen->E0 -> mgr+0x18, return 0; else
 *     override ? 0 : -1).
 *   - dpad Up (0x1000): decrement the scroll index (obj+0x38) if > 0.
 *   - dpad Down (0x4000): increment scroll if the next row (stride 0x14, +0x14)
 *     is populated.
 *   - X (0x40): play sound 0x11; for the selected row (obj+0x34 + scroll*0x14),
 *     if flag bit0 is set either start a game-state change / fade-to-exit based
 *     on the D_001B1E90[0] flag, else toggle the row's bool at *(row+4).
 *   On any scroll change replay the move sound (func_002DFFA0(1,0x11)). Returns 0.
 * Wall: 3-GPR callee-save frame + deep nested branch ladder.
 *
 * Callee $v0/$f0 return types verified from asm: func_002DFFA0 -> void,
 * RequestGameStateChange / FadeOutToBlackBlocking -> result unused (declared
 * implicit int); no $f0-returning callee is consumed here. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DBEE0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 RequestGameStateChange(s32 stateId, s32 push, s32 argA, s32 argB, s32 outDoneFlag);
extern void FadeOutToBlackBlocking(s32 frames);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 79.76% / engine96 66.50%; better arm sdk29; 67 differing
 * rows on it, class STRUCTURAL; first differing insn: ROM `addiu sp,sp,144` vs
 * `addiu sp,sp,-80`. Not iterated in t495. */
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DBEE0(MenuWidget *obj) {
    extern float g_screenFadeBlack;
    u8 *o = (u8 *)obj;
    u8 *blk = (u8 *)g_menuScreenBlock;
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

    /* $17: scroll value captured before any nav (for the change-sound compare). */
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
        func_002DFFA0(0, 0x11);
        row = *(s32 *)(o + 0x38) * 0x14 + *(s32 *)(o + 0x34);
        if (*(s32 *)(row + 0x10) & 0x1) {
            if (D_001B1E90[0] != 0) {
                RequestGameStateChange(4, 1, 3, *(s32 *)(blk + 0x14), 0);
            } else {
                FadeOutToBlackBlocking(4);
                *(s32 *)(o + 0x3C) = 0x10;
                D_001B1E90[0] = (D_001B1E90[0] == 0);
            }
        } else {
            s32 ptr = *(s32 *)(row + 0x4);
            if (ptr != 0) {
                *(u8 *)ptr = (*(u8 *)ptr == 0);
            }
        }
    }
    if (*(s32 *)(o + 0x38) != oldScroll) {
        func_002DFFA0(1, 0x11);
    }
    return 0;
}
#endif

/* Draw a label/value list (rows of stride 0x14): an optional empty-list message
 * (mode bit 1), then each row's left label and a right value chosen by the
 * value-pointer's first byte (nonzero -> [+8], else [+0xC]). Returns 2. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC0F0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void AppendGsRegPacket(s32 regId, u64 value);
extern s32 GetLocalizedString(s32 id);
extern void DrawStringFont1(s32 x, s32 y, u32 color, s32 str, s32 wrap);
extern void DrawFont1RightJustifiedLabel(s32 x, s32 y, u32 color, s32 str, s32 wrap);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 39.81% / engine96 39.76%; better arm sdk29; 144 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-160` vs `addiu sp,sp,-144`. Not iterated in t495.
 * Save stride re-measured on the s136os arm (SN 1.36 -fopt-stack): saves land at the ROM's
 * 8-byte stride, but the saved register set/count differs from the ROM's (NOTE #9871) - an
 * allocation/frame difference, not the stride. So the save-slot wall named above is a cc1 2.9
 * property and was measured false as the reason this member stays unmatched (FACT #9873).
 * Residual: the saved-register set; the rest UNMEASURED. */
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DC0F0(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 *rows;
    s32 n, step, y, i;
    AppendGsRegPacket(0x47, 0x2004b);
    Begin2dDrawBatch(0);
    rows = *(s32 **)(o + 0x34);
    if ((*(u32 *)(o + 0x30) & 1) && rows[0] == 0) {
        /* empty-list placeholder text box drawn here in the original */
        GetLocalizedString(0x2ca2);
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
            DrawStringFont1(0xc, y, col, GetLocalizedString(row[0]), -1);
            DrawFont1RightJustifiedLabel(*(s32 *)(o + 0x20) - 0xc, y, 0x80ffa888,
                                         GetLocalizedString(valId), -1);
            i++;
            y += step;
        } while (*(s32 *)((u8 *)*(s32 **)(o + 0x34) + i * 0x14) != 0);
    }
    End2dDrawBatch();
    return 2;
}
#endif

/* Options-list (stride 0x18) input: only when this widget is the focused one.
 * Up/Down move the selection (+0x38), X cycles the focused row's value pointer
 * (+0x4 byte, modulo the row's option count). Cancel requests the parent. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", UpdateMenuChoiceList);
#else
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 66.57% / engine96 52.82%; better arm sdk29; 81 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-64`. Not iterated in t495.
 * Save stride re-measured on the s136os arm (SN 1.36 -fopt-stack): saves land at the ROM's
 * 8-byte stride, but the saved register set/count differs from the ROM's (NOTE #9871) - an
 * allocation/frame difference, not the stride. So the save-slot wall named above is a cc1 2.9
 * property and was measured false as the reason this member stays unmatched (FACT #9873).
 * Residual: the saved-register set; the rest UNMEASURED. */
    /* TODO(match): functional equivalent - not byte-exact. */
s32 UpdateMenuChoiceList(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 pressed = g_padButtonsPressed[0];
    if (*(s32 *)((u8 *)g_pCurrentMenuScreen[0] + 0xE8) != (s32)o) {
        return 0;
    }
    if ((pressed & 0x900) && D_001F28F4 == 0) {
        return 1;
    }
    if (pressed & 0x10) {
        void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && D_001F28F4 == 0) {
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
            func_002DFFA0(1, 0x11);
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
                func_002DFFA0(0, 0x11);
            }
        }
    }
    return 0;
}
#endif

/* func_002DC520 (OpenRAC rac2-gc-decomp @96927f87: DrawMenuChoiceList; the ROM's
 * two GetLocalizedString calls inside Begin/End2dDrawBatch fit, but that check
 * also passes at +0x1000, so it is weak, FACT #12086; matched, so the splat
 * name stays, task #2244).
 * Draw an options list (rows of stride 0x18): each row is a left label plus a
 * right-justified value picked by the row's current option byte. The selected
 * row (obj->0x38) is highlighted. Rows are spaced evenly. Returns 2.
 *
 * obj: the options widget. +0x20 width, +0x24 height, +0x34 the row table
 * (6 words per row, a zero first word ends it), +0x38 the selected row.
 * Each row: word 0 the label's string id, word 1 a pointer to the row's
 * current option byte, words 2.. the value string ids per option. Rows are
 * counted first; their pitch is height / (count + 1), the first row 8 px
 * above it. Labels go at x 12 in the highlight colour 0x8020FFFF when
 * selected, else 0x80FFA888; values right-justified at width - 12 in
 * 0x80FFA888.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, task #1929). What
 * each piece is worth, measured by removing it alone (solo s136 compile,
 * relocated fields masked, words differing of 101):
 *   - R5900_SHORT_LOOP_PAD3 (below): the row-count loop is four instructions
 *     and the ROM's assembler padded it with three nops before the backward
 *     branch. Without it: 78/101 at 97 words. Its "r"(next) operand on the
 *     count keeps the increment after the pad, for reorg to put it in the
 *     delay slot (80/101 at 103 words without it); a "memory" clobber is not
 *     needed here.
 *   - step pinned to $23 (EE_REG, RULING #8598): the quotient is mflo'd
 *     straight into the long-lived register in the ROM, while cc1 gives it a
 *     temporary and copies it, which shifts the other callee saves. 14/101
 *     without it. About fifteen pin-free spellings (a separate quotient
 *     local, `step /= `, `n++` before the divide, tied empty fences on step
 *     or y) reached 14/101 at best.
 *   - the numerator pinned to $3: with step pinned, the numerator would
 *     otherwise share $23 (it dies at the div). 2/101 without it.
 *   - y formed before the row-table test: 10/101 inside the if.
 *   - the row pointer stepped by the option byte (row += opt, then row[2])
 *     rather than indexed: the ROM adds into the pointer register. 2/101.
 *   - the label colour is a signed 32-bit value (lui/ori, picked by movn),
 *     the value colour a u64 constant (ori/dsll/ori), so the EE arm calls
 *     the ROM symbols with their 64-bit-colour signatures, as 1CA080.cpp and
 *     func_002D6028's arm declare them. */
#ifndef TARGET_NATIVE
/* R5900 SHORT-LOOP PAD (SCHEDULING DEVICE, RULING #8435; the same device and
 * rationale as 191238.cpp's R5900_SHORT_LOOP_PAD3): "+r"(v) pins it between
 * the load and the branch that tests it, "r"(next) keeps the next
 * instruction after it. Emits only nops; empty on native. */
#define R5900_SHORT_LOOP_PAD3(v, next) \
    __asm__(".set noreorder\n\tnop\n\tnop\n\tnop\n\t.set reorder" : "+r"(v) : "r"(next))
#else
#define R5900_SHORT_LOOP_PAD3(v, next) ((void)0)
#endif
/* GUARD (task #1929): on EE the #else body below is the image's func_002DC520,
 * compiled alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810;
 * row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DC520)
S136OS_SLOT(func_002DC520);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void AppendGsRegPacket(s32 regId, u64 value);
extern s32 GetLocalizedString(s32 id);
#ifndef TARGET_NATIVE
/* The ROM symbols, with their ROM signatures (as 1CA080.cpp declares them):
 * the colour is a 64-bit value. EE has no definition under the
 * DrawStringFont1 / DrawFont1RightJustifiedLabel names. */
extern void func_0027FBA8(s32 x, s32 y, u64 color, s32 str, s64 wrap);
extern void func_00280090(s32 x, s32 y, u64 color, s32 str, s64 wrap);
#define DRAW_LABEL       func_0027FBA8
#define DRAW_RIGHT_LABEL func_00280090
#else
extern void DrawStringFont1(s32 x, s32 y, u32 color, s32 str, s32 wrap);
extern void DrawFont1RightJustifiedLabel(s32 x, s32 y, u32 color, s32 str, s32 wrap);
#define DRAW_LABEL       DrawStringFont1
#define DRAW_RIGHT_LABEL DrawFont1RightJustifiedLabel
#endif
/* (end of this body's declarations) */
s32 func_002DC520(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 *p;
    s32 count, row, y, word;
    register s32 step EE_REG("$23");

    AppendGsRegPacket(0x47, 0x2004B);
    Begin2dDrawBatch(0);
    p = *(s32 **)(o + 0x34);
    count = 0;
    if (*p != 0) {
        do {
            p += 6;
            word = *p;
            R5900_SHORT_LOOP_PAD3(word, count);
            count++;
        } while (word != 0);
    }
    {
        register s32 height EE_REG("$3") = *(s32 *)(o + 0x24);
        step = height / (count + 1);
    }
    row = 0;
    y = step - 8;
    if (**(s32 **)(o + 0x34) != 0) {
        do {
            s32 *entry = (s32 *)(*(u8 **)(o + 0x34) + row * 0x18);
            s32 color = (row == *(s32 *)(o + 0x38)) ? 0x8020FFFF : 0x80FFA888;
            DRAW_LABEL(0xC, y, color, GetLocalizedString(entry[0]), -1);
            entry += **(u8 **)(entry + 1);
            DRAW_RIGHT_LABEL(*(s32 *)(o + 0x20) - 0xC, y, 0x80FFA888,
                             GetLocalizedString(entry[2]), -1);
            y += step;
            row++;
        } while (*(s32 *)(*(u8 **)(o + 0x34) + row * 0x18) != 0);
    }
    End2dDrawBatch();
    return 2;
}
#undef DRAW_LABEL
#undef DRAW_RIGHT_LABEL
#endif

/* Dead debris ahead of func_002DC6C8: `sw $11,0x40($3); nop; addiu $sp,$sp,0x20;
 * nop` after func_002DC520's jr and its pad. No prologue and no jr of its own,
 * and no jal, j, branch, data word or hi/lo in the ROM names any of the four
 * words, so it is not a function body and cannot be C. Split off by a
 * symbol_addrs size pin; the data word at 0x25E610 points past it, at 0x2DC6C8
 * (task #2479). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC6B8);

/* func_002DC6C8(widget): init handler of the galactic-map level-select list
 * widget D_0025E608 (its third slot, data word 0x25E610; reached by no jal).
 * Rebuilds the list in g_levelSelectListItems (0x210AA0, the widget's list
 * pointer word 0x25E63C): one 12-byte item per level of
 * g_anAvailableLevelOrder, at most 28 and stopping at the first 0, each with
 * kind 3, the level-detail screen D_25E660 as its descriptor, and the id and
 * value string ids of that level's D_262BA0 caption record. The item after
 * the last gets id 0 (the terminator). It then clears widget+0x40, sets bit
 * 0x8000 in widget+0x30, and scans the 0-terminated level-order list
 * (g_pLevelOrder) for the map cache's current level (g_mapVertexData+0x230),
 * storing the matching row index in widget+0x40. Param: the widget. Returns 0.
 * Until task #2479 this body sat under func_002DC6B8 behind 4 words of
 * debris (split above).
 *
 * MATCHED on the s136os arm, 1D54C0's S136EXTRA "" (SN 2.95.3 v1.36
 * cc1plus -O2 -G8 -fopt-stack, gcse on), task #2479; no device.
 * The 2.9 arm and -fno-gcse build 66 words (FACT #13142).
 * One spelling is load-bearing, measured by removal in the real pipeline
 * (build.sh usa): the map cache's current level is read as a FIELD of a
 * typed view (LevelSelectMapView). Read as a byte offset of the same symbol it
 * builds 66 words, not 68: cc1 folds +0x230 into %lo, where the ROM keeps the
 * base in $5 and loads 0x230($2). Also measured, not needed: upstream's
 * `volatile` on the list pointer (the ROM re-reads g_pLevelOrder through $gp
 * each step without it), a C++-specific loop spelling (NOTE #13148's
 * respelling; upstream's own loop closes here), and the 0x210AA0 symbol
 * (reading the items as g_pLevelSelectListEntries + 0xA8 also closes; the
 * symbol is kept because two ROM references name 0x210AA0 itself).
 * GUARD: on EE this C is the image's body, compiled alone by SN 2.95.3 v1.36
 * -fopt-stack and spliced over S136OS_SLOT by tools/ee/s136os_splice.sh; there
 * is no asm fallback. On native it is the same plain C.
 * Source: OpenRAC/rac2-decomp @b2a35115e0cf580a1fdab8dd1071b157799e590e
 * (GPL-3.0, adopted with the authors' permission per RULINGs #10405/#12108),
 * src/game/002DB988-init-g8.c, func_002DB988 (v1.01 address; author commit
 * a96d3126, Louis-Philippe Le Sieur). Respellings by ROM-encoded address:
 * D_00139948 = g_anAvailableLevelOrder, D_00210AF0 = g_levelSelectListItems,
 * D_0025E680 = D_25E660, D_00262BE0 = D_262BA0, D_001AA4F0 = g_pLevelOrder,
 * D_001C4FA0 = g_mapVertexData.
 * ADAPTED: (1) our signature (MenuWidget *widget), their `char *object` kept
 * as a local; (2) their MenuEntry/SourceEntry as LevelSelectListItem/
 * LevelSelectCaption, fields as theirs; (3) their struct-typed D_001C4FA0 as a
 * LevelSelectMapView cast of the unit's u8[] g_mapVertexData, field
 * `selected` as currentLevel; (4) `volatile` and `__attribute__((sda))` on
 * D_001AA4F0 dropped (the unit's -G8 makes g_pLevelOrder gp-relative);
 * (5) s32/u32/u16/u8 for int/unsigned int/unsigned short/char. Statements and
 * control flow are theirs. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DC6C8)
S136OS_SLOT(func_002DC6C8);
#else
/* One 12-byte level-select list item (g_levelSelectListItems). */
typedef struct {
    u16 id;            /* +0x0 caption string id; 0 terminates the list */
    u16 kind;          /* +0x2 entry kind, 3 = level */
    void *descriptor;  /* +0x4 screen opened on confirm */
    u16 value;         /* +0x8 second caption string id */
    u16 reserved;      /* +0xA */
} LevelSelectListItem;
/* One 12-byte record of the level-caption table at 0x262BA0. */
typedef struct {
    u16 id;            /* +0x0 */
    u16 reserved;      /* +0x2 */
    u16 value;         /* +0x4 */
    u8 rest[6];
} LevelSelectCaption;
extern s32 g_anAvailableLevelOrder[];
extern LevelSelectListItem g_levelSelectListItems[];
extern s32 *g_pLevelOrder;
extern LevelSelectCaption D_262BA0[];
/* The map cache's current-level word (g_mapVertexData+0x230). */
typedef struct {
    u8 pad[0x230];
    s32 currentLevel;  /* +0x230 */
} LevelSelectMapView;
extern u8 g_mapVertexData[];

s32 func_002DC6C8(MenuWidget *widget) {
    u8 *object = (u8 *)widget;
    s32 i, entry;
    s32 *list;

    for (i = 0; i < 28 && g_anAvailableLevelOrder[i]; i++) {
        g_levelSelectListItems[i].kind = 3;
        g_levelSelectListItems[i].descriptor = D_25E660;
        g_levelSelectListItems[i].id = D_262BA0[g_anAvailableLevelOrder[i]].id;
        g_levelSelectListItems[i].value = D_262BA0[g_anAvailableLevelOrder[i]].value;
    }
    list = g_pLevelOrder;
    g_levelSelectListItems[i].id = 0;
    *(s32 *)(object + 0x40) = 0;
    *(u32 *)(object + 0x30) |= 0x8000;
    for (i = 0, entry = list[0]; entry; entry = g_pLevelOrder[++i]) {
        if (((LevelSelectMapView *)g_mapVertexData)->currentLevel == entry) {
            *(s32 *)(object + 0x40) = i;
            break;
        }
    }
    return 0;
}
#endif

/* If the confirm button (mask 0x40) was just pressed, request the menu screen
 * at D_25E660 as the next screen. Always returns 0. */
s32 func_002DC7D8(void) {
    if (g_padButtonsPressed[0] & 0x40) {
        g_pNextMenuScreen[0] = D_25E660;
    }
    return 0;
}

/* Pass the menu object's sub-rect (origin +0x18/+0x1C, size +0x20/+0x24) to
 * func_002897B0, whose retail body is empty, so the call has no effect; always
 * returns 2. */
s32 func_002DC800(MenuWidget *obj) {
    s32 x = *(s32 *)((u8 *)obj + 0x18);
    s32 y = *(s32 *)((u8 *)obj + 0x1C);
    func_002897B0(x, x + *(s32 *)((u8 *)obj + 0x20),
                  y, y + *(s32 *)((u8 *)obj + 0x24));
    return 2;
}

/* Clear the current screen's +0x12C field, then store the result of the
 * map-slot allocator AllocMenuWorkBuffer(0) into obj->0x54. Returns 0.
 * Wall (cc1 2.9): 2-GPR callee-save (8-byte-packed 0x10 frame). */
/* GUARD (task #1271): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DC838)
S136OS_SLOT(func_002DC838);
#else
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 99.00% / engine96 69.33%; better arm sdk29; 7 differing rows
 * on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing insn:
 * ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. Not iterated in t495. */
    /* MATCHED on the s136os arm (task #1271), not by cc1 2.9. */
s32 func_002DC838(MenuWidget *obj) {
    *(s32 *)((u8 *)g_pCurrentMenuScreen[0] + 0x12C) = 0;
    *(s32 *)((u8 *)obj + 0x54) = AllocMenuWorkBuffer(0);
    return 0;
}
#endif

/* Toggle the map-slot referenced by obj->0x54 via FreeMenuWorkBuffer and store the
 * result back. Returns 0. Wall (cc1 2.9): 2-GPR callee-save (8-byte-packed 0x10 frame). */
/* GUARD (task #1271): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DC878)
S136OS_SLOT(func_002DC878);
#else
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 98.75% / engine96 66.67%; better arm sdk29; 7 differing rows
 * on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing insn:
 * ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. Not iterated in t495. */
    /* MATCHED on the s136os arm (task #1271), not by cc1 2.9. */
s32 func_002DC878(MenuWidget *obj) {
    *(s32 *)((u8 *)obj + 0x54) = FreeMenuWorkBuffer(*(s32 *)((u8 *)obj + 0x54));
    return 0;
}
#endif

/* Save/load list confirm/cancel handler. Unless the save/load status code is
 * 0x10 or 1, just mirror the active screen object's +0xE0 field into the
 * manager block's +0x18. Otherwise do the same mirror on a just-pressed
 * confirm (0x20), also clearing the block's +0xE4 and setting the object's
 * +0x12C commit flag to 1, or on cancel (0x10) with the commit flag set to 0.
 * Takes no arguments. Always returns 0.
 * blk = g_menuScreenBlock: +0x14 is g_pCurrentMenuScreen, +0x18 is
 * g_pNextMenuScreen. obj = the current screen: +0xE0 is its next-screen link,
 * +0x12C its commit flag.
 * Byte-exact on this unit's 2.9 arm (task #681), as t495 committed it. The
 * raw s32 offsets are load-bearing. Typed pointer/s32 struct fields put the
 * stores in different alias sets (this arm is -fstrict-aliasing): cc1 then
 * reorders the commit store and cross-jumps the +0x18 store, 90.30% (unit
 * objdiff, t681). The per-branch base local keeps g_menuScreenBlock as `la base` + field offsets
 * in each block, as in the ROM (FACT #7407). t495 recorded "sdk29 65.11%"
 * from its all-arms screen and iterated this body on the engine96 arm only;
 * solo on the 2.9 arm it is 100%. */
s32 func_002DC8A8(void) {
    s32 status = g_nSaveLoadStatusCode;
    if (status != 0x10 && status != 1) {
        u8 *blk = g_menuScreenBlock;
        u8 *obj = *(u8 **)(blk + 0x14);
        *(s32 *)(blk + 0x18) = *(s32 *)(obj + 0xE0);
        return 0;
    }
    if (g_padButtonsPressed[0] & 0x20) {
        u8 *blk = g_menuScreenBlock;
        u8 *obj;
        *(s32 *)(blk + 0xE4) = 0;
        obj = *(u8 **)(blk + 0x14);
        *(s32 *)(blk + 0x18) = *(s32 *)(obj + 0xE0);
        *(s32 *)(obj + 0x12C) = 1;
    } else if (g_padButtonsPressed[0] & 0x10) {
        u8 *blk = g_menuScreenBlock;
        u8 *obj = *(u8 **)(blk + 0x14);
        *(s32 *)(blk + 0x18) = *(s32 *)(obj + 0xE0);
        *(s32 *)(obj + 0x12C) = 0;
    }
    return 0;
}

/* func_002DC940: draw the memory-card system-message box. Picks the message
 * from the card status word at g_menuTransitionMode+0xC8 (= D_001F28A4):
 * 0..2 string 0x2C8F, 3 string 0x2C92, anything else the pre-localized
 * fallback buffer D_1ABBE0. Builds a text-box layout from the widget rect,
 * draws the string twice in black (the second pass vertically centred, with
 * the vertical-align bit flipped), fills the flat backing rect D_1ABBE8..F4
 * with a cached four-corner colour, then draws the face pass one pixel up
 * and left in 0x80F0F0F0. Returns 2.
 *
 * widget: +0x18 x, +0x1C y, +0x20 width, +0x24 height (s32; the clip box
 * reads their low halves). The colour cache (four copies of the %gp word
 * D_1AA45C) is filled on the first call only; its flag and block sit in
 * .sbss at g_rainFallAccel+0x50 / +0x40, read gp-relatively.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack, this unit's
 * -G8 and S136EXTRA "" so gcse on; row in tools/ee/s136os_functions.txt),
 * task #2424. Bodies before this one: NOTE #12323 (task #2383, 22 lines) and
 * NOTE #12350 (task #2398, 15). What each device is worth, measured by
 * removing it alone (solo s136 compile of the unit, positional word compare
 * against the ROM .s with relocated fields masked, of 156 words):
 *   - the switch with grouped `case 0: case 1: case 2:` is the ROM's bnezl
 *     cross-jump to one GetLocalizedString call (NOTE #12323: 138 -> 47).
 *   - `layout` in an inner block: cc1 then reuses its stack slot for the
 *     colour temp at sp+0x20, as the ROM does (NOTE #12323).
 *   - DC940_RECT_X2, the OFFSET-VIEW device (below, RULING #12394): without
 *     it the splice refuses on length (155 words plain, 157 with
 *     `.extern D_1ABBF0, 16`; the gate pipeline, task #2463).
 *   - DC940_COLOR_PIN, the EE_REG("$2") pin on the colour value: 7/156.
 *   - one empty volatile barrier between the c1 and c2 colour stores gives
 *     the ROM's store order c3,c0,c1,c2: 4/156 with no barrier (the pin
 *     kept), 2/156 with the barrier between c0 and c1 instead. It is a
 *     scheduling fence (RULING #8483: empty, emits nothing, both arms) and
 *     depends on this block's RTL, so any later edit of the block must
 *     re-gate it. The C order of the four stores is inert (seven orders and
 *     a loop measured; only the loop changes, to 83/156).
 * Removal figures are ranking probes, not the gate. */
/* ADDRESSING-MODEL DEVICE (RULING #12316 Q1; directive-only, emits nothing):
 * the ROM reads the rect's y1 and y2 words with gas's absolute same-register
 * macro (`lui $5; lw $5,%lo(D_1ABBEC)($5)`), never gp-relatively; x1
 * (D_1ABBE8) is a true -G8 small-data word. At FILE SCOPE, outside the guard,
 * so the unit's 2.9 TU sees them before the spliced block too. D_1ABBEC's
 * 16-byte size also makes gas emit DC940_RECT_X2's read of D_1ABBEC+4 as an
 * absolute lui/lw pair rather than gp-relatively. */
#ifndef TARGET_NATIVE
__asm__(".extern D_1ABBEC, 16");
__asm__(".extern D_1ABBF4, 16");
#endif
/* GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DC940)
S136OS_SLOT(func_002DC940);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 GetLocalizedString(s32 id);
extern void *memset(void *s, s32 c, u32 n);
extern void func_00280BB8(s32 box, s64 tint, s32 str, s32 mode); /* DECL-LEVER(#2424): defined s32 tint; the s64 tint is the ROM's positive 64-bit 0x80000000 (`li $5,0x8000; dsll $5,$5,0x10`), as #2090's declaration above */
extern void func_0027F168(s32 x1, s32 y1, s32 x2, s32 y2, s64 z, const s32 *colors);
/* (end of this body's declarations) */
/* The status word, read as %lo(g_menuTransitionMode+0xC8) as the ROM does
 * (an unsized view of the real symbol: no alias reaches the symtab). */
extern u8 g_menuTransitionModeBlk[] __asm__("g_menuTransitionMode");
extern char D_1ABBE0[];                 /* fallback message buffer */
extern s32 D_1ABBE8, D_1ABBEC, D_1ABBF0, D_1ABBF4; /* backing rect x1, y1, x2, y2 */
extern s32 D_1AA45C;                    /* backing-rect colour */
typedef struct CardMsgTextBox {
    s16 clip[4];   /* 0x00 top, bottom, left, right */
    s16 x;         /* 0x08 */
    s16 y;         /* 0x0A */
    s16 width;     /* 0x0C */
    s16 height;    /* 0x0E written back by func_00280BB8 */
    s16 size;      /* 0x10 */
    s16 align;     /* 0x12 bit 2: vertical centring */
    s16 _14[2];
} CardMsgTextBox;
typedef struct CardMsgBoxColors { s32 c[4]; } CardMsgBoxColors;
#ifndef TARGET_NATIVE
/* The colour cache lives in .sbss right after g_rainFallAccel (+0x40 block,
 * +0x50 flag; the ROM's %gp_rel(g_rainFallAccel+0x40/+0x50)). No symbol of
 * its own exists in the EE link, so it is read through g_rainFallAccel,
 * whose -G8 size-4 declaration keeps every access gp-relative. */
extern s32 g_rainFallAccel;
#define s_cardMsgColorsInit (*(s32 *)((u8 *)&g_rainFallAccel + 0x50))
#define s_cardMsgColors (*(CardMsgBoxColors *)((u8 *)&g_rainFallAccel + 0x40))
/* OFFSET-VIEW DEVICE (RULING #12394; EE arm only, the native arm below names
 * D_1ABBF0 directly). A = D_1ABBEC (rect y1), B = D_1ABBF0 (rect x2), offset
 * +4: B is read as `(&D_1ABBEC)[1]`, so cc1 prints `lw $6,D_1ABBEC+4`.
 * The file-scope `.extern D_1ABBEC, 16` above makes gas expand that into the
 * ROM's absolute same-register pair at 0x2DCB1C/0x2DCB20,
 * `lui $6,%hi(..); lw $6,%lo(..)($6)`: R_MIPS_HI16 + R_MIPS_LO16 against
 * D_1ABBEC with addend 4, where splat's .s names D_1ABBF0 with addend 0. The
 * address (0x1ABBF0) and the bytes are the same; only the relocation's
 * symbol name differs.
 * Why it is load-bearing: cc1 2.95 costs a symbol+offset load at 2 insns
 * even for a small symbol (simple_memory_operand never accepts a CONST
 * address). So reorg does not take it for func_0027F168's jal slot and takes
 * the gp-relative D_1ABBE8 load, as the ROM does (0x2DCB28).
 * The two other spellings fail the splice's length check (ROM 156 words):
 *   - plain `D_1ABBF0`: a -G8 small word costed at 1 insn. cc1 puts it in
 *     the jal slot as a one-word gp-relative load, 155 words.
 *   - `D_1ABBF0` plus `.extern D_1ABBF0, 16`: still costed at 1 insn and put
 *     in the slot, but gas's 2-insn macro does not fit there, so it is
 *     hoisted out and the slot gets a nop, 157 words. */
#define DC940_RECT_X2 ((&D_1ABBEC)[1])
/* REGISTER-PIN DEVICE (RULING #8598): the ROM loads the colour into $2 and
 * the cache flag's 1 into $3; cc1 colours them the other way round. */
#define DC940_COLOR_PIN EE_REG("$2")
#else
static s32 s_cardMsgColorsInit;
static CardMsgBoxColors s_cardMsgColors;
#define DC940_RECT_X2 D_1ABBF0
#define DC940_COLOR_PIN
#endif
s32 func_002DC940(MenuWidget *widget) {
    u8 *w = (u8 *)widget;
    CardMsgTextBox box;
    s32 str;
    s32 status;

    AppendGsRegPacket(0x42, 0x44);
    AppendGsRegPacket(0x47, 0x2004B);
    Begin2dDrawBatch(0);
    str = (s32)D_1ABBE0;
    status = *(s32 *)(g_menuTransitionModeBlk + 0xC8);
    switch (status) {
    case 0: case 1: case 2: str = GetLocalizedString(0x2C8F); break;
    case 3: str = GetLocalizedString(0x2C92); break;
    }
    {
        CardMsgTextBox layout;
        memset(&layout, 0, sizeof(layout));
        layout.clip[0] = *(s16 *)(w + 0x1C);
        layout.clip[1] = *(s16 *)(w + 0x1C) + *(s16 *)(w + 0x24) + 1;
        layout.clip[2] = *(s16 *)(w + 0x18);
        layout.clip[3] = *(s16 *)(w + 0x18) + *(s16 *)(w + 0x20) + 1;
        layout.x = (*(s32 *)(w + 0x18) + *(s32 *)(w + 0x20)) >> 1;
        layout.y = *(s16 *)(w + 0x1C) + 5;
        layout.width = *(s16 *)(w + 0x20) - 5;
        layout.height = *(s16 *)(w + 0x24) - 5;
        layout.size = 0x10;
        layout.align = 5;
        box = layout;
    }
    if (!s_cardMsgColorsInit) {
        CardMsgBoxColors colors;
        register s32 color DC940_COLOR_PIN = D_1AA45C;
        s_cardMsgColorsInit = 1;
        colors.c[3] = color;
        colors.c[0] = color;
        colors.c[1] = color;
        __asm__ __volatile__("");   /* scheduling fence, see the doc comment */
        colors.c[2] = color;
        s_cardMsgColors = colors;
    }
    func_00280BB8((s32)&box, 0x80000000, str, -1);
    box.y = (*(s32 *)(w + 0x24) - box.height) >> 1;
    box.align ^= 4;
    func_00280BB8((s32)&box, 0x80000000, str, -1);
    func_0027F168(D_1ABBE8, D_1ABBEC, DC940_RECT_X2, D_1ABBF4, 0, s_cardMsgColors.c);
    box.clip[0]--; box.clip[1]--; box.clip[2]--; box.clip[3]--; box.x--; box.y--;
    func_00280BB8((s32)&box, 0x80F0F0F0, str, -1);
    End2dDrawBatch();
    return 2;
}
#endif

/* Seed obj->0x34 with a rotating entry from D_260570, indexed by the save
 * block's first word modulo 19. Returns 0.
 * MATCHED 100.00% on the sdk29 arm (unit objdiff; verify_match_unit.sh BYTE
 * IDENTICAL, t495) once g_playerProgress was declared as the scalar the
 * original reads (cc1-small / assembler-absolute macro shape, `.extern ,16`). */
s32 func_002DCBB0(MenuWidget *obj) {
    *(s32 *)((u8 *)obj + 0x34) = D_260570[(u32)g_playerProgress % 19];
    return 0;
}

/* 30-entry wrap-around selector input for the widget `object` (+0x40 field).
 * With L1/R1 (0x900) pressed it returns 1 unless the override at
 * g_menuScreenBlock+0x134 is set. Cancel (0x10) requests the current page's
 * back page (page +0xE0) as the next page and returns 0; with no back page it
 * returns -1 (leave the menu) unless the override is set, then 0. Otherwise X
 * (0x40) advances and square (0x20) retreats, both modulo 30, and a menu
 * sound (func_002DFFA0(1, 17)) plays when the value changed. Returns 0.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack at this
 * unit's -G8 and S136EXTRA "", so gcse on; row in tools/ee/s136os_functions.txt),
 * spliced over S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice drops the function. On native it is
 * plain C. The ROM shares one %hi(D_138180) and recomputes the %lo at each
 * pad read, which the two separate `state` blocks reproduce; the pad block
 * and the menu block are aggregates, never -G8 small data, so cc1 splits
 * their addresses as upstream's -G0 build does (NOTE #12163). Task #2295.
 *
 * Source: OpenRAC/rac2-gc-decomp @96927f872c709e13aaa0f32265bb81a5531bf7ed
 * (GPL-3.0, adopted with the authors' permission per RULING #10405),
 * src/game/002D5E98.c, func_002DBEC0 (v1.01 address; author commit 7cb06451,
 * ThorpenCodes). ADAPTED: our symbols: D_00138180 = D_138180 as an int label
 * alias (the unit declares it u8[]), D_001F2840 = g_menuScreenBlock, their
 * D_001F2974 = g_menuScreenBlock+0x134 read through it (the EE link has no
 * symbol at 0x1F28F4), func_002DF190 = func_002DFFA0. Statements and control flow
 * are theirs. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DCBF0)
S136OS_SLOT(func_002DCBF0);
#else
extern int g_padPortWords[] __asm__("D_138180");   /* pad port 0; +0x1C4 = pressed mask */

int func_002DCBF0(char *object)
{
    int flags;
    {
        int *state = g_padPortWords;
        if (!(state[0x1C4 / 4] & 0x900))
            goto proceed;
        if (*(int *)(g_menuScreenBlock + 0x134) == 0)
            return 1;
    }
proceed:
    {
        int *state = g_padPortWords;
        flags = state[0x1C4 / 4];
    }
    if (flags & 0x10) {
        char *root = (char *)g_menuScreenBlock;
        int value = *(int *)(*(char **)(root + 0x14) + 0xE0);
        int status;
        if (value != 0) {
            *(int *)(root + 0x18) = value;
        } else {
            switch (*(int *)(root + 0x134)) {
            case 0:
                status = -1;
                goto finish;
            default:
                break;
            }
        }
        status = 0;
finish:
        return status;
    }
    {
        int old = *(int *)(object + 0x40);
        if (flags & 0x40)
            *(int *)(object + 0x40) = (old + 1) % 30;
        else if (flags & 0x20)
            *(int *)(object + 0x40) = (old + 29) % 30;
        if (*(int *)(object + 0x40) != old)
            func_002DFFA0(1, 17);
    }
    return 0;
}
#endif

/* 12-entry wrap-around selector input for the widget `object` (+0x54 field).
 * L1/R1 (0x900) and cancel (0x10) behave as in func_002DCBF0 (returns 1, or
 * 0 / -1). Otherwise 0x2040 steps forward and 0x8020 back, modulo 12, each
 * with the menu sound func_002DFFA0(1, 17). Returns 0.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack at this
 * unit's -G8 and S136EXTRA "", so gcse on; row in tools/ee/s136os_functions.txt),
 * spliced over S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice drops the function. On native it is
 * plain C. The ROM shares one %hi(D_138180) and recomputes the %lo at each
 * pad read, which the two separate `state` blocks reproduce; the pad block
 * and the menu block are aggregates, never -G8 small data, so cc1 splits
 * their addresses as upstream's -G0 build does (NOTE #12163). Task #2295.
 *
 * Source: OpenRAC/rac2-gc-decomp @96927f872c709e13aaa0f32265bb81a5531bf7ed
 * (GPL-3.0, adopted with the authors' permission per RULING #10405),
 * src/game/002D5E98.c, func_002DBF98 (v1.01 address; author commit 331fbb0a,
 * ThorpenCodes). ADAPTED: our symbols: D_00138180 = D_138180 as an int label
 * alias (the unit declares it u8[]), D_001F2840 = g_menuScreenBlock, their
 * D_001F2974 = g_menuScreenBlock+0x134 read through it (the EE link has no
 * symbol at 0x1F28F4), func_002DF190 = func_002DFFA0. Statements and control flow
 * are theirs. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DCCC8)
S136OS_SLOT(func_002DCCC8);
#else
extern int g_padPortWords[] __asm__("D_138180");   /* pad port 0; +0x1C4 = pressed mask */

int func_002DCCC8(char *object)
{
    int flags;
    {
        int *state = g_padPortWords;
        if (!(state[0x1C4 / 4] & 0x900))
            goto proceed;
        if (*(int *)(g_menuScreenBlock + 0x134) == 0)
            return 1;
    }
proceed:
    {
        int *state = g_padPortWords;
        flags = state[0x1C4 / 4];
    }
    if (flags & 0x10) {
        char *root = (char *)g_menuScreenBlock;
        int value = *(int *)(*(char **)(root + 0x14) + 0xE0);
        int status;
        if (value != 0) {
            *(int *)(root + 0x18) = value;
        } else {
            switch (*(int *)(root + 0x134)) {
            case 0:
                status = -1;
                goto finish;
            default:
                break;
            }
        }
        status = 0;
finish:
        return status;
    }
    if (flags & 0x2040) {
        *(int *)(object + 0x54) = (*(int *)(object + 0x54) + 1) % 12;
        func_002DFFA0(1, 17);
    } else if (flags & 0x8020) {
        *(int *)(object + 0x54) = (*(int *)(object + 0x54) + 11) % 12;
        func_002DFFA0(1, 17);
    }
    return 0;
}
#endif

/**
 * Draw a single nav arrow of the widget `obj` (left or right depending on
 * obj->0x38): a one-glyph (2-byte) label, drawn through func_0027FBA8 with
 * colour 0x80FFA888, plus a rotated HUD arrow sprite (HUD icon 0xE99D/6). With
 * +0x38 non-zero the label is D_1ABBF8 at x = 4 and the sprite sits at x = 640
 * with angle 0; with +0x38 zero the label is D_1ABC00 at x = obj->0x20 - 24 and
 * the sprite at x = 192 with angle pi. The sprite's y is obj->0x24 * 8. Returns
 * 2.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack; row in
 * tools/ee/s136os_functions.txt), spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. Device-free: no
 * volatile, no register pin, no inline asm.
 *
 * Source: OpenRAC/rac2-gc-decomp @974302ba936bb455acd78034b58ad4b98bafcc6b (GPL-3.0, adopted with the authors' permission per RULING #10405), src/game/002D5E98.c, func_002DC090 (v1.01), C body adopted verbatim apart from our spellings (their func_0027C540_1 = Begin2dDrawBatch, func_0027F538 = func_0027FBA8, func_0028E588 = func_0028EDF0, func_0028E640 = GetHudIconTex0, func_0028F410 = func_0028FC78, func_0027C660_1 = End2dDrawBatch, D_L01_001ABE90 = D_1ABBF8, D_L01_001ABE98 = D_1ABC00); byte-exact on our ROM: vmu BYTE IDENTICAL 102/102 words (14 relocs resolved; base-seeded bit flip rc 1 at 0x2dcde0), unit objdiff report 100.00% size=408 (objdiff_build.sh + unit_report.sh, clean), whole-image cmp 0 under landing_gate usa and eu --strict (task #2159)
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DCDC0)
S136OS_SLOT(func_002DCDC0);
#else
/* The true ROM signatures this body needs, declared here because the s136os
 * arm compiles this body alone: func_0028FC78 takes its five draw parameters
 * as floats ($f12..$f16; angle in $f16) and the extents/texture in $4..$6. */
extern s32  GetHudIconTex0(s32 iconIndex);
extern s32  func_0028EDF0(s32 name, s32 level);
#ifndef TARGET_NATIVE
extern void func_0027FBA8(s32 x, s32 y, u64 color, s32 str, s32 wrap);   /* 64-bit colour (dli), 32-bit wrap: -1 is one addiu, not a dli */
#endif
extern void func_0028FC78(f32 x, f32 y, f32 w, f32 h, f32 angle, s32 uExtent, s32 vExtent, s32 tex);
typedef struct { signed char a, b; } NavArrowLabel;
extern signed char D_1ABBF8[];
extern signed char D_1ABC00[];

s32 func_002DCDC0(char *s0) {
    signed char local[16];
    float y;

    Begin2dDrawBatch(0);
    if (*(int *)(s0 + 56) != 0) {
        *(NavArrowLabel *)local = *(NavArrowLabel *)D_1ABBF8;
        func_0027FBA8(4, *(int *)(s0 + 36) / 2 - 8, 0x80FFA888, (s32)local, -1);
        y = (float)(*(int *)(s0 + 36) * 8);
        func_0028FC78(640.0f, y, 128.0f, 256.0f, 0.0f, 32, 16, GetHudIconTex0(func_0028EDF0(0xE99D, 6)));
    } else {
        *(NavArrowLabel *)local = *(NavArrowLabel *)D_1ABC00;
        func_0027FBA8(*(int *)(s0 + 32) - 24, *(int *)(s0 + 36) / 2 - 8, 0x80FFA888, (s32)local, -1);
        y = (float)(*(int *)(s0 + 36) * 8);
        func_0028FC78(192.0f, y, 128.0f, 256.0f, 3.1415927f, 32, 16, GetHudIconTex0(func_0028EDF0(0xE99D, 6)));
    }
    End2dDrawBatch();
    return 2;
}
#endif

/* Draw the centered level-info caption(s) for the currently-selected map row.
 *
 * The focused list (current menu screen +0xE8) holds 12-byte rows at +0x34,
 * selected by +0x40; the row's word at +4 is a level-caption index. When it is
 * -1, one line, the generic "no info" string 0x2CFB, is drawn centred in the
 * widget. Otherwise two caption lines are drawn from that index's record in the
 * caption table at 0x262BA0, at 1/3 and 2/3 of the widget height. Every line is
 * in the Font1 centred-label style, colour 0x80FFA888, 8 px above its row.
 *
 * obj: the menu widget (+0x20 width, +0x24 height, re-read for every line).
 * Returns 2.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, task #1623):
 * verify_match_unit BYTE IDENTICAL 104/104, st_size 0x19C. What each piece is
 * worth (solo s136 compile, aligned text diff against the ROM):
 *   - The colour is declared u64: the ROM builds it zero-extended
 *     (ori/dsll/ori). With u32 it is lui/ori, one word short per call.
 *   - The table is an array of 12-byte records. As a flat s32 array indexed
 *     idx*3 and idx*3+1, the second index is rebuilt by shift-add; through a
 *     u8 cast, base+idx*12 is folded once. The ROM keeps idx*12 (a mult by the
 *     constant 12 already held for the row lookup) and the base apart, and
 *     reads +0 and +4 off them.
 *   - obj is used directly. A `u8 *o` copy put it in two registers.
 *   - The callee's return type is INERT here: its ROM epilogue writes $2
 *     (daddu $2,$16,$0), but void and s32 compile identically for this body,
 *     so it keeps the void every other declaration in the unit uses.
 * The record view reaches D_262BA0 through an asm-label alias because another
 * arm of this unit declares D_262BA0 with another type (func_002DC6C8's
 * LevelSelectCaption[]; u8[] before task #2479), and C++ (the native build)
 * rejects two declarations of one name with different types. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DCF58)
S136OS_SLOT(func_002DCF58);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 GetLocalizedString(s32 id);
/* DrawFont1CenteredLabel, called by its splat name so verify_match_unit can
 * resolve it (it cannot resolve a PROVIDE alias). EE arm: the colour is u64,
 * because the ROM passes it zero-extended in a 64-bit register (ori/dsll/ori,
 * not lui/ori), as 1CA080.cpp declares it. Native keeps the unit-wide u32
 * declaration of func_002801B8: a second type for one extern "C" name does
 * not compile there. */
#ifndef TARGET_NATIVE
extern void func_002801B8(s32 x, s32 y, u64 color, s32 str, s32 wrap); /* DECL-LEVER(#1623): defined s32 colour; the ROM caller passes it zero-extended */
#endif
/* The level-caption table at 0x262BA0: one 12-byte record per level, the
 * first two words being localized-string ids. */
typedef struct {
    s32 line1;  /* +0x0 first caption line */
    s32 line2;  /* +0x4 second caption line */
    s32 unk8;   /* +0x8 not read here */
} LevelCaption;
extern LevelCaption g_levelCaptions[] __asm__("D_262BA0");
/* (end of this body's declarations) */
s32 func_002DCF58(MenuWidget *obj) {
    u8 *focus = *(u8 **)((u8 *)g_pCurrentMenuScreen[0] + 0xE8);
    s32 idx = *(s32 *)(*(s32 *)(focus + 0x40) * 0xC + *(s32 *)(focus + 0x34) + 4);

    Begin2dDrawBatch(0);
    if (idx == -1) {
        func_002801B8(*(s32 *)((u8 *)obj + 0x20) / 2, *(s32 *)((u8 *)obj + 0x24) / 2 - 8,
                      0x80FFA888, GetLocalizedString(0x2CFB), -1);
    } else {
        func_002801B8(*(s32 *)((u8 *)obj + 0x20) / 2, *(s32 *)((u8 *)obj + 0x24) / 3 - 8,
                      0x80FFA888, GetLocalizedString(g_levelCaptions[idx].line1), -1);
        func_002801B8(*(s32 *)((u8 *)obj + 0x20) / 2, (*(s32 *)((u8 *)obj + 0x24) << 1) / 3 - 8,
                      0x80FFA888, GetLocalizedString(g_levelCaptions[idx].line2), -1);
    }
    End2dDrawBatch();
    return 2;
}
#endif

/* Menu screen draw helper (Ghidra merges its boundary with a neighbour). The
 * body saves no callee register: it is a frameless leaf with a `jr $4` jump
 * table (an earlier "multi callee-save" note was wrong). What blocks C is the
 * splat split: the .s opens with a stray two-word fragment (`sw $0,0x50($4);
 * nop`) ahead of the real entry at 0x2DD100, the same shape as func_002DA488.
 * It needs a re-split before C can sit at the symbol. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD0F8);

#ifdef TARGET_NATIVE
/* Types + native-only callee decls for the func_002DD450 #else body (the
 * matching INCLUDE_ASM build never sees them). Extracted from the Track-B
 * gs_draw.h recovery; field offsets verified against func_002DD450.s. */
typedef struct TextLayout2d {
    s16 y0;        /* +0x00 top+4 inset      */  s16 y1;   /* +0x02 bottom-4        */
    s16 x0;        /* +0x04 left             */  s16 x1;   /* +0x06 right           */
    s16 xCenter;   /* +0x08 left+width/2     */  s16 yCursor; /* +0x0A current line Y */
    s16 unkC;      u16 outAdvance; /* +0x0E OUT: line advance (readers (s16)-cast) */
    s16 unk10;     s16 fontId;  /* +0x12 (9 here) */  s16 unk14; /* +0x14 (0) */
    s16 align;     /* +0x16 align nibble     */  s16 unk18; s16 unk1A; s16 unk1C; s16 unk1E;
} TextLayout2d; /* 0x20 */
typedef struct IntroTextBox {
    u8  unk0[0x18];
    s32 x;         /* +0x18 */  s32 y; /* +0x1C */  s32 w; /* +0x20 */  s32 h; /* +0x24 */
    u8  unk28[0xC];
    union { s32 id; s32 *idList; } msg;  /* +0x34: single id / -1-terminated list */
    u8  unk38[4];
    s32 flags;     /* +0x3C: >>4 = first-line Y backoff; low byte >>4 = per-line align */
    s32 frameTimer;/* +0x40 */
    u8  unk44[8];
    s32 unk4C;
    s32 state;     /* +0x50 sequencer state (0x14-entry jump table) */
    s32 doneFlag;  /* +0x54 set when a further line would still fit */
} IntroTextBox; /* >= 0x58 */
void func_002DF620(s16 *dst, void *src);   /* layout-header fill (defined below) */
extern void func_00280B48(TextLayout2d *layout, u64 rgba, char *str, s32 len); /* text-draw core */
#endif

/* Draw an intro/dialog text box: emit the TEST_1 (0x47) + ALPHA_1 (0x42) GS
 * register writes, open a 2D draw batch, fill the text layout from the box
 * geometry, then render either a multi-line -1-terminated localized-string list
 * or a single centered string per box->state (0x14-entry jump table), and set
 * box->doneFlag when another line would still fit (yCursor+0x18 < y+h). Returns
 * 2. Byte-match walled: jump-table switch + 8-byte-packed callee-saves + the
 * dsll/dsrl 64-bit GS-value packing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD450);
#else
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 80.91% / engine96 47.57%; better arm sdk29; 64 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-80` vs `addiu sp,sp,-112`. Not iterated in t495.
 * Save stride NOT re-measured for this member on the s136os arm: its C arm does not compile
 * solo there (NOTE #9871). Of the 111 labeled members that were, 0 reproduce the 16-byte save
 * stride there (FACT #9873), so the stride is not evidence that this member is walled.
 * Residual: UNMEASURED. */
s32 func_002DD450(IntroTextBox *box) {
    TextLayout2d layout;
    char *str;

    AppendGsRegPacket(0x47, 0x30000);
    AppendGsRegPacket(0x42, ((u64)0x8000 << 24) | 0x44);
    Begin2dDrawBatch(0);
    func_002DF620((s16 *)&layout, box);
    layout.fontId = 9;
    layout.y0 = box->y + 4;
    layout.y1 = box->y + box->h - 4;
    layout.x0 = box->x;
    layout.x1 = box->x + box->w;
    layout.xCenter = box->x + (box->w >> 1);
    layout.unk14 = 0;
    layout.align = 0;

    switch (box->state) {
    case 0:
        break;
    case 1: case 2: case 3: case 5: case 6: case 7: case 8: case 9:
    case 0xA: case 0xC: case 0xD: case 0xE: case 0x10: case 0x11: case 0x12: {
        s32 i = 0;
        s32 backoff = (box->flags >> 4) - 4;
        s32 y = box->y - backoff;

        if (*box->msg.idList != -1) {
            do {
                layout.yCursor = y;
                layout.align = (u64)(*(u8 *)&box->flags) >> 4;
                str = (char *)GetLocalizedString(box->msg.idList[i]);
                i++;
                func_00280B48(&layout, 0x80FFA888, str, -1);
                y += (s16)layout.outAdvance + 0xA;
            } while (box->msg.idList[i] != -1);
        }
        if (y + 0x18 < box->y + box->h) {
            box->doneFlag = 1;
        }
        break;
    }
    case 4:
    case 0xB:
    case 0xF:
    case 0x13:
        layout.yCursor = box->y + 0x20;
        str = (char *)GetLocalizedString(box->msg.id);
        func_00280B48(&layout, 0x80FFA888, str, -1);
        break;
    }
    End2dDrawBatch();
    return 2;
}
#endif

/* Dead debris ahead of func_002DD638: `addiu $sp,$sp,0x90` and a nop after
 * func_002DD450's jr. No prologue and no jr of its own, so it is not a
 * function body and cannot be C. Split off by a symbol_addrs size pin; the
 * data word at 0x25DC7C points past it, at 0x2DD638 (task #2289). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD630);

/* Draw the current level-select row label/value for the focused map screen.
 * Only renders while the area transition state permits it (area+0x15C < 3,
 * area+0x164 < 0, menuBlock+0x164 >= 0xB, area+0x8 == 2). Derives a level index
 * from the focused widget's area record (the 0x1C-byte records at
 * g_areaTable+0x30, indexed by focus+0x40; its first word mod 0x1C); index -1
 * draws the "unknown" placeholder string 0x2DAA centred
 * (y + g_levelSelectRowLineHeight/2). Otherwise it draws the level label
 * (formatted by func_002E0010 into a stack buffer) and, when the row's value
 * string id (g_levelSelectEntries[lvl], second word) is >= 0, the value string
 * one line below (y + g_levelSelectRowLineHeight); a row with no value string is
 * drawn as a single centred line (y + g_levelSelectRowLineHeight >> 1). The
 * value id is re-read from the table for the second draw. Returns 2.
 * Reached only through the screen-handler word at 0x25DC7C. Until task #2289
 * this body sat under func_002DD630 behind 2 words of debris (split above).
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack), task #2289,
 * no device. Three spellings were each measured as necessary:
 *   - the value id's address as `&tbl[idx]` with `tbl` taken FIRST and
 *     `idx = lvl * 2 + 1` its own statement: the ROM computes (lvl << 3) | 4
 *     (combine's no-carry `ori`) and then adds the base. Folded into one
 *     expression, cse puts the +4 into %lo (6 words differ);
 *   - the area records through a typed AreaRec pointer: with byte arithmetic
 *     the record `addu` comes out with its operands swapped (1 word);
 *   - the half-line offset as its own variable, 0 when the value id is valid.
 * The div-by-0x1C trap guard (beql/break) is cc1's own.
 * GUARD: compiled alone by SN 2.95.3 v1.36 -fopt-stack and spliced over the
 * S136OS_SLOT line (tools/ee/s136os_splice.sh); plain C on native. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DD638)
S136OS_SLOT(func_002DD638);
#else
extern s32 GetLocalizedString(s32 id);
#ifndef TARGET_NATIVE
/* The ROM signature (as above at func_002D5A48): a 64-bit colour, built
 * zero-extended. Native keeps the unit-wide 32-bit declaration. */
extern void func_002801B8(s32 x, s32 y, u64 color, s32 str, s32 wrap);
#endif
extern s32 func_002E0010(char *dst, s32 level);
extern u8  g_areaTable[];
extern u8  g_levelSelectEntries[];       /* {label id, value id} pairs, stride 8 */
extern s32 g_levelSelectRowLineHeight;   /* row vertical spacing */
typedef struct { s32 level; s32 _04[6]; } AreaRec;  /* 0x1C-byte record at g_areaTable+0x30 */

s32 func_002DD638(void *screenArg) {
    u8 *obj = (u8 *)screenArg;
    u8 *area = g_areaTable;
    u8 *mgr = g_menuScreenBlock;
    s32 lvl;

    {
        AreaRec *recs = (AreaRec *)(area + 0x30);
        lvl = recs[*(s32 *)(*(u8 **)(*(u8 **)(mgr + 0x14) + 0xE8) + 0x40)].level % 0x1C;
    }
    Begin2dDrawBatch(0);
    if (*(s32 *)(area + 0x15C) < 3 && *(s32 *)(area + 0x164) < 0 &&
        *(s32 *)(mgr + 0x164) >= 0xB && *(s32 *)(area + 0x8) == 2) {
        if (lvl == -1) {
            func_002801B8(*(s32 *)(obj + 0x18), *(s32 *)(obj + 0x1C) + g_levelSelectRowLineHeight / 2,
                          0x80F0F0F0, GetLocalizedString(0x2DAA), -1);
        } else {
            s32 *tbl = (s32 *)g_levelSelectEntries;
            s32 idx = lvl * 2 + 1;
            s32 *valueId = &tbl[idx];
            s32 valueStr = *valueId;
            s32 halfLine = 0;
            char label[64];   /* func_002E0010 formats the level label here */

            if (valueStr < 0)
                halfLine = g_levelSelectRowLineHeight >> 1;
            func_002801B8(*(s32 *)(obj + 0x18), *(s32 *)(obj + 0x1C) + halfLine, 0x80F0F0F0,
                          func_002E0010(label, lvl), -1);
            if (valueStr >= 0)
                func_002801B8(*(s32 *)(obj + 0x18), *(s32 *)(obj + 0x1C) + g_levelSelectRowLineHeight,
                              0x80F0F0F0, GetLocalizedString(*valueId), -1);
        }
    }
    End2dDrawBatch();
    return 2;
}
#endif

/* Enter the galactic-map save-confirm screen: reset the text-table banks, grab
 * a fresh map slot into obj->0x48 (obj->0x4C=0), and prime the autosave/voice
 * subsystems; if a memory card is present and the GUI exists, show its panel.
 * Returns 0. */
/* ADDRESSING-MODEL DEVICES for func_002DD7E8 (RULING #8620 terms; the offset-0 gp
 * equate is ruled covered by RULING #9073; #8036 alias). They emit nothing:
 *   - `.extern D_1A8C88, 16`: the ROM reads D_1A8C88 with the absolute
 *     `lui $2,%hi(D_1A8C88); lw $2,%lo(D_1A8C88)($2)` pair (0x002DD81C);
 *   - the offset-0 equate alias `g_guiInstanceGp`, given no sized `.extern` of its
 *     own: it reproduces the ROM's `lw $2,%gp_rel(g_guiInstance)($28)` in the
 *     `beqz` delay slot at 0x002DD828. The only size the alias gets is cc1's own
 *     `.extern g_guiInstanceGp, 4` (<= -G8; the s136os splice carries it in front
 *     of the block), so that one access is gp-relative even though this unit
 *     declares `.extern g_guiInstance, 16` further up. The unit's other readers
 *     of g_guiInstance (func_002D5EC8, func_002D60E8, func_002D61D8,
 *     func_002D63B0, func_002D6508) stay absolute; measured, task #1408. The
 *     relocation names g_guiInstance (R_MIPS_GPREL16), and GNU as never writes an
 *     equate of an undefined symbol to the symtab, so no alias reaches nm.
 * At file scope, EE only (task #1408): the s136os splice REFUSES a member whose
 * own arm carries an `.extern ,16` (ADDRESSING) or an equate (DEFINITION) that
 * the unit's 2.9 TU never sees (FACT #9057, FACT #9067). Native reads the real
 * symbol. No other C in this unit names D_1A8C88 or the alias. */
#ifndef TARGET_NATIVE
__asm__(".extern D_1A8C88, 16");
__asm__("g_guiInstanceGp = g_guiInstance");
extern char *g_guiInstanceGp;
#define GUI_INSTANCE_GP g_guiInstanceGp
#else
extern char *g_guiInstance;
#define GUI_INSTANCE_GP g_guiInstance
#endif
/* GUARD (task #1408): on EE this C is the image's func_002DD7E8, compiled alone by
 * the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810;
 * tools/ee/s136os_functions.txt) and spliced over the S136OS_SLOT line by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DD7E8)
S136OS_SLOT(func_002DD7E8);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_002CA980(void);
extern void func_002888A8(void);
extern void func_0033A7A8(void *p);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 86.39% / engine96 82.14%; better arm sdk29; 13 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. Not iterated in t495. */
/* SCREEN (task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
 * masked; a candidate, NOT match evidence): EXACT 28/28, relocations equal, no
 * alias in nm. Three levers, each measured undone:
 *   - ADDRESSING-MODEL DEVICES (RULING #8620 terms, #8036 alias): D_1A8C88 is
 *     read as a word in the ROM's absolute lui/lw form (`.extern ,16`; else
 *     15/28, first diff @13), and the g_guiInstance read that sits in the
 *     `beqz` delay slot is gp-relative in the ROM, so it goes through an offset-0
 *     alias with no `.extern` size; the relocation still names g_guiInstance
 *     (as the real name it expands absolute: 14/28, first diff @15);
 *   - the +0x4C clear written after the alloc call (before it: 3/28, first diff
 *     @6, the original first diff).
 * The devices now sit at file scope above this function's guard. */
    /* MATCHED on the s136os arm (task #1408), not by cc1 2.9. */
s32 func_002DD7E8(MenuWidget *obj) {
    extern s32 D_1A8C88;
    u8 *o = (u8 *)obj;
    s32 slot;

    func_002DF1B8(1);
    slot = AllocMenuWorkBuffer(0);
    *(s32 *)(o + 0x4C) = 0;
    *(s32 *)(o + 0x48) = slot;
    func_002CA980();
    func_002888A8();
    if (D_1A8C88 != 0) {
        char *gui = GUI_INSTANCE_GP;
        if (gui != 0) {
            func_0033A7A8(gui + 0x3CEA0);
        }
    }
    return 0;
}
#endif

/* Toggle the map-slot referenced by obj->0x48 via FreeMenuWorkBuffer and store the
 * result back. Returns 0. Wall (cc1 2.9): 2-GPR callee-save (8-byte-packed 0x10 frame). */
/* GUARD (task #1271): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DD858)
S136OS_SLOT(func_002DD858);
#else
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 98.75% / engine96 66.67%; better arm sdk29; 7 differing rows
 * on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing insn:
 * ROM `addiu sp,sp,-16` vs `addiu sp,sp,-32`. Not iterated in t495. */
    /* MATCHED on the s136os arm (task #1271), not by cc1 2.9. */
s32 func_002DD858(MenuWidget *obj) {
    *(s32 *)((u8 *)obj + 0x48) = FreeMenuWorkBuffer(*(s32 *)((u8 *)obj + 0x48));
    return 0;
}
#endif

/* Save-slot list input + autosave commit handler for the galactic-map save
 * screen. Drives the GUI list cursor (D_001A7C10), waits for the disc to settle
 * after a kicked write (D_001F28F8), snapshots the slot's quick-resume record
 * or forces a reload, and on X opens the confirm screen / re-loads an empty
 * slot. Returns 0/1/-1.
 * Left as INCLUDE_ASM: ~130-instruction branch graph over a large set of
 * save-system globals whose exact record layout (D_00139410 stride 0x1c) is
 * only partially recovered — a functional #else here would be guesswork, so
 * honesty wins over a fabricated body. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD888);

/* Autosave-commit + slot-cursor input for the in-game save screen. After a
 * kicked write settles, either resumes play (re-mix audio, restore stereo, jump
 * to the level) or forces a state change; otherwise runs the 4-slot cursor
 * (held nav when bit 0 set), and on X starts a quick save into 0x1F29F0 with a
 * 0xD-frame autosave arm. Returns 0/1/-1.
 * Wall: large branch graph over save-system globals.
 * Byte-match: engine-2.96 SAVE-SLOT wall (prologue packs 7 regs at 8-byte
 * spacing 0x0..0x30; 2.9 reserves 16-byte slots) → no match on 2.9, #else is
 * correct. Not "TODO(match)".
 * Save stride re-measured on the s136os arm (SN 1.36 -fopt-stack): saves land at the ROM's
 * 8-byte stride, but the saved register set/count differs from the ROM's (NOTE #9871) - an
 * allocation/frame difference, not the stride. So the save-slot wall named above is a cc1 2.9
 * property and was measured false as the reason this member stays unmatched (FACT #9873).
 * Residual: the saved-register set; the rest UNMEASURED. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DDD30);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void ComputeAudioChannelMix(void);
extern void func_00132938(s32 flag);
extern s32 RequestGameStateChange(s32 stateId, s32 push, s32 argA, s32 argB, s32 outDoneFlag);
extern s32 func_00288898(void);
extern s32 GetMenuOverlayMode(void);
extern void BuildSaveImage(void *dst);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 55.35% / engine96 38.59%; better arm sdk29; 240 differing
 * rows on it, class PACKED-SAVE (2.9 16-byte slots) + rest; first differing
 * insn: ROM `addiu sp,sp,-64` vs `addiu sp,sp,-80`. Not iterated in t495. */
/* Matching arm stays INCLUDE_ASM; #else is the structure model — call-level
 * faithful, cross-checked against the thoroughly-asm-verified EU twin func_002DDCE8
 * (the EU form navigates the menu block directly; this USA form uses the
 * g_pCurrentMenuScreen/g_pNextMenuScreen globals per the USA asm). */
s32 func_002DDD30(MenuWidget *obj) {
    extern s32 D_001F28F8, D_0013953C, D_00139544, D_001F2924, D_001393E8;
    extern s32 D_0013954C, D_001393F0, D_0013955C, D_001A7424, D_001A7C10;
    extern s32 D_001A8C8C, D_00152C28, D_001C4F30, D_001F28FC;
    extern u8  D_001A8C88;
    extern s32 D_00139410[], D_00139528; extern s16 D_001393F8;
    extern u8  g_abLevelVisitedMarkers[]; extern u8 D_0025DB48[], D_00260D98[];
    extern void func_0033A7D8(void *list, s32 sel);   /* GUI save-list cursor */
    extern s32  func_00299960(void);                  /* consumed: quick-save readiness */
    extern void func_00299968(void);                  /* arm the quick-save write */
    extern void func_002F7328(void);                  /* resume-play transition */
    extern void func_002CA998(void);                  /* post-BuildSaveImage commit step */
    u8 *o = (u8 *)obj;
    s32 prevSel = *(s32 *)(o + 0x40);
    s32 pressed, nav;
    if (g_pGuiManager != 0) {
        func_0033A7D8((u8 *)g_pGuiManager + 0x3CEA0, prevSel);
    }
    if (D_001F28F8 != 0) {
        if (D_0013953C > 2 || D_00139544 >= 0 || D_001F2924 < 0xb) return 0;
        D_001F28F8 = 0;
        if (D_0013954C == 0 && D_001393F0 == 0 && func_00299960() == 0) {
            D_0013955C = 1;
            ComputeAudioChannelMix();
            PumpDialogVoiceSystem(1);
            func_00132938(g_nAudioStereoMode == 0);
            if (g_playerProgress < 1 && g_abLevelVisitedMarkers[0] == 0) {
                func_002F7328();
            } else {
                RequestGameStateChange(6, 2, 5, g_playerProgress, 0);
            }
            D_00152C28 = 0;
            return 0;
        }
        D_0013955C = 0;
        D_001A7424 |= 0x100;
        if (RequestGameStateChange(4, 1, 1, (s32)g_pCurrentMenuScreen[0], 0) == 0) return 0;
    }
    pressed = g_padButtonsPressed[0];
    if ((pressed & 0x900) && D_001F28F4 == 0 && D_001A8C8C == 0) {
        return 1;
    }
    if (pressed & 0x10) {
        void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
        if (nxt == 0 && D_001F28F4 == 0) return -1;
        if (nxt == 0) nxt = g_pNextMenuScreen[0];
        g_pNextMenuScreen[0] = nxt;
        return 0;
    }
    if (g_nSaveLoadStatusCode == 9) {
        func_002D67A0(5, (s32)(D_001A8C88 == 0 ? D_0025DB48 : D_00260D98));
    } else if (g_nSaveLoadStatusCode != 0x10 && g_nSaveLoadStatusCode != 1) {
        if (func_00288898() != 0 || GetMenuOverlayMode() != 1) {
            void *nxt = *(void **)((u8 *)g_pCurrentMenuScreen[0] + 0xE0);
            if (nxt == 0 && D_001F28F4 == 0) return -1;
            if (nxt == 0) nxt = g_pNextMenuScreen[0];
            g_pNextMenuScreen[0] = nxt;
            return 0;
        }
        RequestGameStateChange(4, 1, 1, (s32)g_pCurrentMenuScreen[0], 0);
    }
    if (D_0013953C >= 3) return 0;
    if (D_00139544 >= 0 || D_001F2924 < 0xb || D_001393E8 != 2) return 0;
    nav = (*(u32 *)(o + 0x30) & 1) ? g_padButtonsHeld : pressed;
    *(s32 *)(o + 0x40) = D_001A7C10;
    if ((nav & 0x1000) && D_001A7C10 != 0) *(s32 *)(o + 0x40) = D_001A7C10 - 1;
    D_001A7C10 = *(s32 *)(o + 0x40);
    if ((nav & 0x4000) && D_001A7C10 < 3) {
        *(s32 *)(o + 0x40) = D_001A7C10 + 1;
        D_001A7C10 = *(s32 *)(o + 0x40);
    }
    if (nav & 0x40) {
        if (D_001393E8 == 2 &&
            *(s32 *)((u8 *)D_00139410 + D_001A7C10 * 0x1c) >= 0) {
            PlayGlobalSound(4, 0, 0);
            D_00139528 = 0;
            D_001393F8 = (s16)*(s32 *)(o + 0x40);
            BuildSaveImage((void *)0x1f29f0);
            func_002CA998();
            if (D_00139544 < 0) { extern s32 D_00139548; D_00139548 = 0; D_00139544 = 0xd; }
            D_001F28F8 = 1;
            func_00299968();
            D_001F28FC = 0x2c93;
            D_001C4F30 = 0;
        } else {
            PlayGlobalSound(5, 0, 0);
        }
    }
    if (*(s32 *)(o + 0x40) != prevSel) {
        PlayGlobalSound(3, 0, 0);
    }
    return 0;
}
#endif

/* In-game save/load list input + commit (sibling of func_002DD888 / DDD30 for
 * the load path, with quick-resume restore via RestorePlayerProgressState).
 * Left as INCLUDE_ASM: ~200-instruction branch graph that snapshots the same
 * partially-recovered save record (D_00139410 stride 0x1c, fields at +4/+8/+0x10)
 * — a functional #else would require guessing the record layout, so the asm is
 * kept as the source of truth. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DE168);

/* Draw the save-slot summary list (per-slot bolt/time/level/date readouts with
 * icons). Wall: ~350-instruction draw loop + FP time/scale math + sq/lq packing
 * + language-specific layout. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DE768);

/* Splat mis-split fragment: only `addiu $sp,N; nop` runs (+0x30, +0x60, +0x10),
 * no prologue/jr — dead epilogue debris, not a function body, so it cannot be C.
 * Marked a fragment (task #1660; same bytes as INCLUDE_ASM). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DECC8);

/*
 * func_002DECE0 declarations. The cheat input buffer is 20 s16 symbols at
 * D_001B1E90+0x118; g_cheatInputCount counts the symbols entered;
 * g_cheatCodeSymbols is the 256-byte cheat symbol table (symbol k of code c is
 * byte ((k + 1) * c) & 0xFF); D_1395B8 is a 6-entry unlock-flag array; D_1A8CEC
 * is set once a skill-point cheat fires. D_138180 is the pad state block (+0x1C0 held mask as
 * a doubleword = g_padButtonsHeld, +0x1C4 pressed mask = g_padButtonsPressed),
 * declared as an ordinary array so cc1 splits its address and the prologue's
 * `sd $31` schedules between the halves, as in the ROM.
 * The three *Abs names are assembler aliases (the #8036 construct, as in
 * text/188858.c): 8 bytes to cc1 so it emits one `la` macro per use, 16 to gas
 * so the macro expands absolutely. The ROM reads those three tables as adjacent
 * lui/addiu pairs; as a two-word macro the address cannot go into a delay slot,
 * which is what makes the skill-point path's branches fill as the ROM's do.
 */
extern u8  D_138180[];
extern s32 g_cheatInputCount;
extern u8  g_cheatCodeSymbols[];
extern u8  D_1395B8[];
#ifndef TARGET_NATIVE
__asm__(".extern D_1A8CEC, 16");
__asm__(".extern g_inventoryNewFlagAbs, 16\n\tg_inventoryNewFlagAbs = g_inventoryNewFlag");
__asm__(".extern g_inventoryOwnedAbs, 16\n\tg_inventoryOwnedAbs = g_inventoryOwned");
__asm__(".extern g_skillPointFlagsAbs, 16\n\tg_skillPointFlagsAbs = g_skillPointFlags");
extern u8 g_inventoryNewFlagAbs[8];
extern u8 g_inventoryOwnedAbs[8];
extern u8 g_skillPointFlagsAbs[8];
#else
extern u8 g_inventoryNewFlag[];
#define g_inventoryNewFlagAbs g_inventoryNewFlag
#define g_inventoryOwnedAbs   g_inventoryOwned
#define g_skillPointFlagsAbs  g_skillPointFlags
#endif
extern s32 D_1A8CEC;
#define CHEAT_INPUT ((s16 *)(D_001B1E90 + 0x118))

/**
 * Cheat-code entry detector, run from the pause menu. While the held pad mask
 * has the 0x6 pattern in its low nibble, each newly pressed d-pad/face button
 * appends one symbol to a 20-entry input buffer (0x1000 -> 0, 0x4000 -> 1,
 * 0x8000 -> 2, 0x2000 -> 3, 0x80 -> 4, any other 0xF0A0 bit -> 5); releasing
 * the pattern clears the count. When the 20th symbol lands, code ids 2..0x92
 * are tested against it: symbol k of code c is table byte ((k + 1) * c) & 0xFF.
 * The first hit, as found = c - 2, unlocks:
 *   0x00..0x37  inventory item `found` (owned + new flag),
 *   0x38..0x49  level `found - 0x37` (MarkLevelAvailable),
 *   0x4A..0x4F  flag D_1395B8[found - 0x4A],
 *   0x50..0x6D  skill point `found - 0x50`, once: sets its flag, plays sound 1,
 *               raises message 0x1233 (func_002B1880) and sets D_1A8CEC.
 *
 * Matching notes (sdk29 arm, -O2 -G8 -fno-gcse):
 *  - the code loop's bound is a local: the ROM compares against 0x93 held in a
 *    register, which cc1 emits only when the constant cannot be propagated
 *    into the loop block;
 *  - the symbol loop is a goto loop over a register-bound index: the ROM
 *    recomputes k * code and k * 2 each pass, and any loop.c-visible index gets
 *    strength-reduced;
 *  - the tied empty asm copies the loaded count into `entered` without cc1
 *    seeing the equivalence, so the bound test reads the loaded register and
 *    the copy survives, as in the ROM;
 *  - the first symbol, the loop bound and the post-call constant are bound to
 *    the registers the ROM uses.
 */
void func_002DECE0(void) {
    u8 *pad = D_138180;
    s32 pressed;
    s32 count;
    s32 entered;
    s32 symbol;

    if ((*(u64 *)(pad + 0x1C0) & 0xF) == 6) {
        pressed = *(s32 *)(pad + 0x1C4);
        if ((pressed & 0xF0A0) == 0) {
            return;
        }
        count = g_cheatInputCount;
        __asm__("" : "=r"(entered) : "0"(count));
        if (count >= 0x14) {
            return;
        }
        if (pressed & 0x1000) {
            symbol = 0;
        } else if (pressed & 0x4000) {
            symbol = 1;
        } else if (pressed & 0x8000) {
            symbol = 2;
        } else if (pressed & 0x2000) {
            symbol = 3;
        } else {
            symbol = (pressed & 0x80) ? 4 : 5;
        }
        CHEAT_INPUT[entered] = symbol;
        entered++;
        g_cheatInputCount = entered;
        if (entered == 0x14) {
            s32 found = -1;
            s32 code;
            register s32 firstSymbol EE_REG("$11") = CHEAT_INPUT[0];
            register s32 codeEnd EE_REG("$12");

            for (code = 2, codeEnd = 0x93; code < codeEnd; code++) {
                s32 match = 1;
                register s32 pos EE_REG("$5") = 0;

                if (firstSymbol != g_cheatCodeSymbols[code & 0xFF]) {
                    goto mismatch;
                }
            next:
                pos++;
                if (pos < 0x14) {
                    if (CHEAT_INPUT[pos] == g_cheatCodeSymbols[(pos * code + code) & 0xFF]) {
                        goto next;
                    }
                mismatch:
                    match = 0;
                }
                if (match) {
                    found = code - 2;
                    break;
                }
            }
            if (found != -1) {
                s32 level = found - 0x37;
                s32 flagIndex = found - 0x4A;
                s32 skill = found - 0x50;

                if (found < 0x38) {
                    g_inventoryOwnedAbs[found] = 1;
                    g_inventoryNewFlagAbs[found] = 1;
                } else if ((u32)(found - 0x38) < 0x12) {
                    MarkLevelAvailable(level);
                } else if ((u32)flagIndex < 6) {
                    D_1395B8[flagIndex] = 1;
                } else if ((u32)skill < 0x1E) {
                    u8 *flag = &g_skillPointFlagsAbs[skill];

                    if (*flag == 0) {
                        *flag = 1;
                        PlayGlobalSound(1, 0, 0);
                        func_002B1880(0x1233, -1);
                        {
                            register s32 one EE_REG("$2") = 1;

                            D_1A8CEC = one;
                        }
                    }
                }
            }
        }
    } else {
        g_cheatInputCount = 0;
    }
}

/* Dead debris ahead of func_002DEEF8: `addiu $sp,$sp,0xA0; nop; addiu $sp,$sp,
 * 0x20; nop` after func_002DECE0's jr and its delay slot. No prologue and no jr
 * of its own, and no jal, j, data word or hi/lo in the ROM names any of the
 * four words, so it is not a function body and cannot be C. Split off by a
 * symbol_addrs size pin (task #2480). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DEEE8);

/* func_002DEEF8(stream): per-frame tick of a streamed moby-animation job
 * (upstream's note: it corresponds to RAC1's update_streamed_moby_animation).
 * The job's state word (+0x34):
 *   0  waits for the CD file load to go idle (g_fileLoadState == 0), then
 *      goes to 3;
 *   1  once idle: clears the menu load-active byte (g_menuScreenBlock+0xDB),
 *      decompresses the streamed WAD in place (DecompressWad from
 *      buffer+offset into buffer), installs its three animation tables into
 *      moby class 0x7A5's loaded header (header tables[1..3] = buffer + the
 *      offsets in the buffer's first three 8-byte records, each followed by
 *      func_00293B10(class, n)), then goes to 2 and starts the moby in state
 *      0, or 4 when g_miscExtras is set, with its counter cleared;
 *   2  steps the moby's own sequence (moby+0x20; frame counter at +0xBC):
 *      even states 0/2/4 idle for 120 frames, then wait for the dialog voice
 *      (g_fileLoadVoiceState phase 3, count < 6000), advance to the odd state
 *      and play animation state/2 + 1 (func_002A82D8(moby, n, 0, 24));
 *      odd states 1/3/5 stop the dialog voice once when the moby's two
 *      animation bytes (+0x42/+0x43) agree, and once flag value 2 is set in
 *      moby+0x60 go to 2 (from 1) or 6 (from 3/5) and play animation 0;
 *      state 6 waits 240 frames and restarts at 0 (4 with g_miscExtras).
 *   3  (and any other state) does nothing.
 * Param: the job. Returns 0 on every path.
 * Until task #2480 this body sat under func_002DEEE8 behind 4 words of debris
 * (split above); no static ROM reference names 0x2DEEF8.
 *
 * MATCHED on the s136os arm, 1D54C0's S136EXTRA "" (SN 2.95.3 v1.36
 * cc1plus -O2 -G8 -fopt-stack), task #2480. The 2.9 arm builds 174 words
 * (FACT #13142). The inner switch's table is the ROM's jtbl_0026CE90_text:
 * tools/ee/jtbl_bind.sh verifies the compiled table word for word against it
 * and binds %hi/%lo to it.
 * DEVICE (RULING #12316, addressing, EE arm only): the file-scope
 * `.extern g_miscExtras, 16` below. g_miscExtras is a 1-byte object, so under
 * the unit's -G8 cc1 and gas would address it gp-relative; the ROM reads it
 * with the absolute `lui $3; lbu $3,%lo(g_miscExtras)($3)` macro pair (twice).
 * Declaring it 16 bytes to gas makes gas expand cc1's one-insn `lbu` that way.
 * Nothing is moved or emitted; native does not see it. Removal probe
 * (build.sh usa): without it the block builds 173 words, not 175, and the
 * s136os splice refuses it (SHORTER).
 * DEVICE (RULING #8620, section attribute on an extern declaration): the
 * unit's ROM_SPLIT on g_fileLoadState, as the other g_fileLoadState readers
 * in this unit declare it. The ROM reads the 2-byte flag with a compiler-split
 * `lui $2; lh $3,%lo($2)` (twice). Removal probe: declared plain, this TU
 * makes the s136os splice carry `.extern g_fileLoadState, 2` in front of the
 * block, and the splice REFUSES (ADDRESSING) because that would flip three
 * already matched rows (RestorePrevTextTable, TickStreamedImageWidget,
 * LoadMenuBgImagePair) to gp-relative.
 * Source: OpenRAC/rac2-decomp @b2a35115e0cf580a1fdab8dd1071b157799e590e
 * (GPL-3.0, adopted with the authors' permission per RULINGs #10405/#12108),
 * src/game/002DDEC8-animation.c, func_002DE0D0 (v1.01 address; author commit
 * b073e803, bigbeno37). Respellings by ROM-encoded address:
 * D_001A63AC = g_fileLoadState, D_001F291B = g_menuScreenBlock[0xDB],
 * func_0029D4D0 = DecompressWad, D_001CEC85 = g_mobyClassSlotRemap[0x7A5],
 * D_001CDB80 = g_mobyClassHeaders, func_00293260 = func_00293B10,
 * D_001A7A0A[0] = g_miscExtras, D_001A63A8 = g_fileLoadVoiceState,
 * func_002B6D28 = StopDialogVoice, func_002A7470 = func_002A82D8.
 * ADAPTED: (1) their AnimationMoby/AnimationStream/AnimationClass/Dialogue
 * as StreamedAnimMoby/StreamedAnimJob/StreamedAnimClass/DialogVoiceView,
 * fields as theirs; (2) their `u8 D_001A7A0A[] __attribute__((sda))` read
 * as [0] becomes our scalar `u8 g_miscExtras` with the `.extern` device
 * above in place of `sda`; (3) g_fileLoadState through the unit's ROM_SPLIT
 * s16 declaration (the second device, above); g_fileLoadVoiceState
 * declared as a DialogVoiceView (upstream's plain declaration);
 * (4) s32/s16/u8 for int/short/unsigned char, and int return as s32.
 * Statements and control flow are theirs. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DEEF8)
S136OS_SLOT(func_002DEEF8);
#else
/* The moby a streamed-animation job drives (only the fields it touches). */
typedef struct {
    u8 pad0[0x20];
    u8 state;          /* +0x20 sequence state 0..6 */
    u8 pad21[0x21];
    u8 primary;        /* +0x42 */
    u8 secondary;      /* +0x43 */
    u8 pad44[0x1c];
    u8 flags;          /* +0x60 value 2 tested before leaving an odd state */
    u8 pad61[0x5b];
    u8 counter;        /* +0xBC frame counter */
} StreamedAnimMoby;
/* The job (its state word sits at +0x34). */
typedef struct {
    u8 pad0[0x34];
    s32 state;         /* +0x34 0 wait, 1 install, 2 run, 3 done */
    s32 reserved;      /* +0x38 */
    u8 *buffer;        /* +0x3C decompress destination */
    s32 offset;        /* +0x40 compressed data offset in buffer */
    StreamedAnimMoby *moby;  /* +0x44 */
} StreamedAnimJob;
/* A loaded moby class header (animation table pointers at +0x48). */
typedef struct {
    u8 pad[0x48];
    u8 *tables[4];
} StreamedAnimClass;
/* The dialog-voice fields of the file-load/voice block. */
typedef struct {
    u8 pad[0x6c];
    s16 count;         /* +0x6C */
    u8 pad6e[4];
    s16 phase;         /* +0x72 */
} DialogVoiceView;
extern s16 g_fileLoadState ROM_SPLIT;
extern u8 g_mobyClassSlotRemap[];
extern StreamedAnimClass *g_mobyClassHeaders[];
#ifndef TARGET_NATIVE
__asm__(".extern g_miscExtras, 16");
#endif
extern u8 g_miscExtras;
extern DialogVoiceView g_fileLoadVoiceState;
extern void DecompressWad(void *src, void *dest);
extern void func_00293B10(StreamedAnimClass *cls, s32 table);
extern s32 StopDialogVoice(void);
extern void func_002A82D8(StreamedAnimMoby *moby, s32 animation, s32 arg2, s32 arg3);

s32 func_002DEEF8(StreamedAnimJob *stream) {
    StreamedAnimMoby *moby;
    u8 *header;
    StreamedAnimClass **resource;
    s32 i, animation;
    u8 *table;
    DialogVoiceView *dialogue;

    switch (stream->state) {
    case 0:
        if (g_fileLoadState != 0) break;
        stream->state = 3;
        break;
    case 1:
        if (g_fileLoadState != 0) break;
        g_menuScreenBlock[0xDB] = 0;
        DecompressWad(stream->buffer + stream->offset, stream->buffer);
        resource = &g_mobyClassHeaders[g_mobyClassSlotRemap[0x7A5]];
        header = stream->buffer;
        for (i = 0; i < 3; i++) {
            table = stream->buffer + *(s32 *)(header + (i << 3));
            (*resource)->tables[i + 1] = table;
            func_00293B10(*resource, i + 1);
        }
        stream->state = 2;
        if (g_miscExtras == 0) stream->moby->state = 0;
        else stream->moby->state = 4;
        stream->moby->counter = 0;
        break;
    case 2:
        moby = stream->moby;
        switch (moby->state) {
        case 0: case 2: case 4:
            animation = moby->state >> 1;
            moby->counter++;
            if (moby->counter > 120) moby->counter = 120;
            if (moby->counter < 120) return 0;
            dialogue = &g_fileLoadVoiceState;
            if (dialogue->phase != 3) return 0;
            if (dialogue->count >= 6000) return 0;
            moby->counter = 0;
            moby->state++;
            func_002A82D8(moby, animation + 1, 0, 24);
            break;
        case 1: case 3: case 5:
            if (moby->counter == 0 && moby->primary == moby->secondary) {
                StopDialogVoice();
                moby->counter = 1;
            }
            if (moby->flags & 2) {
                if (moby->state == 1) moby->state = moby->state + 1;
                else moby->state = 6;
                moby->counter = 0;
                func_002A82D8(moby, 0, 0, 24);
            }
            break;
        case 6:
            moby->counter++;
            if (moby->counter > 240) moby->counter = 240;
            if (moby->counter < 240) return 0;
            if (g_miscExtras == 0) moby->state = 0;
            else moby->state = 4;
            moby->counter = 0;
            break;
        }
        break;
    }
    return 0;
}
#endif

/* (Re)assign the 5 map cache slots' backing addresses and flags for a text
 * table swap. `param` selects whether the secondary banks are included: the
 * first 1 (2 with `param`) slots point into the load buffer (+0x118) stepping
 * 0x11800, the next 0 (1) into the second-bank base (+0x11C) stepping 0x11800,
 * then 0 (2) buffer and 0 (1) second-bank slots stepping 0x4F000 with flags 1;
 * any unused slots are zeroed.
 * Not matched. The slot table is addressed absolutely (lui/addiu of
 * D_001B1E90+0x40), not gp-relative as an earlier note said. Best measured body
 * 77.57% (unit objdiff, solo, sdk29 -O2 -G8 -fno-gcse, 34 of 107 rows): four
 * `for` loops over cumulative ends, with first+second computed before loop 1.
 * Residual: every loop needs the R5900 short-loop pad nop with the pointer step
 * in the delay slot, and five mode constants take different registers. Body:
 * NOTE #8228. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF1B8);
#else
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 47.10% / engine96 0.00%; better arm sdk29; 152 differing
 * rows on it, class STRUCTURAL; first differing insn: ROM `lui v0,0x0  [HI16
 * 0x001F27C0]` vs `sltu a0,zero,a0`. Not iterated in t495. */
    /* TODO(match): functional equivalent - not byte-exact. */
void func_002DF1B8(s32 param) {
    s32 *ids   = (s32 *)(D_001B1E90 + 0x40);
    s32 *flags = (s32 *)(D_001B1E90 + 0x44);
    s32 bank2 = D_001F28DC;
    s32 hasParam = (param != 0);
    s32 base = hasParam ? 2 : 1;
    s32 extra = hasParam ? 2 : 0;
    s32 count = base + hasParam;
    s32 buf = (s32)g_pTextTableLoadBuf;
    u32 filled = 0;
    u32 k;
    for (k = 0; k < (u32)base; k++) {
        ids[k * 2] = buf;
        flags[k * 2] = 0;
        buf += 0x1180 * 0x10;   /* TextTableEntry stride (0x10 bytes) */
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
        buf += 0x4f00 * 0x10;
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

/* Acquire a free menu work buffer from the 5-slot pool at D_001B1E90+0x40
 * (per slot: the buffer address at +0, its flags at +4; flag bit 0 = size
 * class, bit 1 = in use). The first slot whose size class matches wantLarge,
 * that has a buffer, and is not in use is marked in use, its buffer filled
 * with 0xDEADBEEF for GetMenuWorkBufferSize bytes, and returned. Returns 0
 * when no slot is free.
 *
 * wantLarge: nonzero for a slot whose size-class bit is set (the 0x4F000
 * buffers, per GetMenuWorkBufferSize), zero for one whose bit is clear.
 *
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, task #1929). The
 * ROM walks the buffer and flag words with two pointers, but reads the flag
 * it marks through base + (slot << 3), formed at the top of every pass (it
 * fills the size-class branch's delay slot). Each piece priced by removing it
 * alone (solo s136 screen, relocated fields masked, words differing of 47,
 * built length in brackets):
 *   - the empty tied fence on slot (RULING #8483): without it loop.c folds
 *     slot << 3 into the walking flag pointer, 41 (44);
 *   - the offset formed at the top of the pass: 37 (45);
 *   - the in-use flag read through base + offset rather than the walking
 *     pointer: 42 (41);
 *   - the offset added as an int with the offset first (the ROM's
 *     `addu $5,$3,$8`): 1 as `(u8 *)flags + offset`;
 *   - the size class as an if/else with a load in each arm: 42 (43) as a
 *     conditional expression, which cc1 if-converts;
 *   - slot = 0 before the walking copy of the flag base: 4 in a for-init. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_AllocMenuWorkBuffer)
S136OS_SLOT(AllocMenuWorkBuffer);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 GetMenuWorkBufferSize(s32 id);
extern void FillMemory32(s32 dst, u32 pattern, s32 nbytes);
/* (end of this body's declarations) */
s32 AllocMenuWorkBuffer(s32 wantLarge) {
    s32 *buffer = (s32 *)(D_001B1E90 + 0x40);
    u32 *flags = (u32 *)(D_001B1E90 + 0x44);
    s32 slot = 0;
    u32 *flag = flags;

    for (; slot < 5; slot++, buffer += 2, flag += 2) {
        u32 sizeClass;
        s32 offset;
        __asm__("" : "+r"(slot));
        offset = slot << 3;
        if (wantLarge) {
            sizeClass = *flag ^ 1;
        } else {
            sizeClass = *flag;
        }
        if (sizeClass & 1) continue;
        if (*buffer == 0) continue;
        {
            u32 *entry = (u32 *)(offset + (s32)flags);
            u32 bits = *entry;
            if (bits & 2) continue;
            *entry = bits | 2;
            FillMemory32(*buffer, 0xDEADBEEF, GetMenuWorkBufferSize(*buffer));
            return *buffer;
        }
    }
    return 0;
}
#endif

/* Release the map cache slot whose id == `id`: if it is in-use (flag bit 1),
 * cancel any pending stream tied to it (bit 4 + the global file-load gate) and
 * clear its in-use bit. Returns 0. Slots are the interleaved id/flags pairs at
 * D_001B1E90+0x40/+0x44 (stride 8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", FreeMenuWorkBuffer);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void StopFileLoad(void);
/* (end of this body's declarations) */
/* t495 screen (all 69 arms promoted at once per arm, master 6ef5e297, unit
 * objdiff): sdk29 63.85% / engine96 63.68%; better arm sdk29; 38 differing
 * rows on it, class STRUCTURAL; first differing insn: ROM `lui t0,0x0  [HI16
 * 0x001F27C0]` vs `lui v0,0x0  [HI16 D_001B1E90]`. Not iterated in t495. */
    /* TODO(match): functional equivalent - not byte-exact. */
s32 FreeMenuWorkBuffer(s32 id) {
    extern u8 D_001F289B;
    s32 *ids   = (s32 *)(D_001B1E90 + 0x40);
    u32 *flags = (u32 *)(D_001B1E90 + 0x44);
    s32 i = 0;
    do {
        if (ids[i * 2] == id) {
            u32 f = flags[i * 2];
            if (f & 2) {
                if (f & 4) {
                    s32 wasPending = (D_001F289B != 0);
                    flags[i * 2] = f ^ 4;
                    if (wasPending) {
                        StopFileLoad();
                        D_001F289B = 0;
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

/* Map work-buffer size for the cache slot whose id == `id`: 0x4F000 bytes if
 * the slot's flag bit 0 (large buffer) is set, else 0x11800; -1 if none of the
 * 5 slots holds that id. AllocMenuWorkBuffer sizes its 0xDEADBEEF fill with it.
 * id: slot id to look up. Returns the byte size, or -1.
 * Byte-exact on this unit's 2.9 arm (task #644). The slot table is 5
 * interleaved {id, flags} words (stride 8) at D_001B1E90+0x40 / +0x44, and the
 * ROM loads the two cursors with two independent lui/addiu pairs, each in its
 * own register: that is the one-insn `la` macro that cc1 emits for a symbol it
 * believes is small data, expanded by an assembler that knows it is not (see
 * the D_001B1E90 declaration). Declaration order carries the prologue order:
 * the 0x4F000 constant, then i, then flags before ids. */
s32 GetMenuWorkBufferSize(s32 id) {
    s32 large = 0x4F000;
    s32 i = 0;
    s32 *flags = (s32 *)(D_001B1E90 + 0x44);
    s32 *ids   = (s32 *)(D_001B1E90 + 0x40);
    do {
        i++;
        if (*ids == id) {
            return (*flags & 1) ? large : 0x11800;
        }
        flags += 2;
        ids += 2;
    } while (i < 5);
    return -1;
}

/* Set the 0x4 (stream pending) flag on the map cache slot whose id == `id`.
 * id: slot id. Returns 0 on hit, 1 if none of the 5 slots holds that id.
 * Byte-exact on this unit's 2.9 arm (task #644); same slot-table addressing
 * and declaration-order notes as GetMenuWorkBufferSize. */
s32 func_002DF560(s32 id) {
    s32 i = 0;
    s32 *flags = (s32 *)(D_001B1E90 + 0x44);
    s32 *ids   = (s32 *)(D_001B1E90 + 0x40);
    do {
        i++;
        if (*ids == id) {
            *flags |= 4;
            return 0;
        }
        flags += 2;
        ids += 2;
    } while (i < 5);
    return 1;
}

/* Clear the 0x4 (stream pending) flag on the map cache slot whose id == `id`;
 * the mirror of func_002DF560.
 * id: slot id. Returns 0 on hit, 1 if none of the 5 slots holds that id.
 * Byte-exact on this unit's 2.9 arm (task #644). The mask lives in its own
 * local declared right after i, which places the ROM's `addiu $7,$0,-5` second
 * in the prologue; written as a literal `&= ~4`, cc1 hoists it out of the loop
 * and schedules it after the two `la`. */
s32 func_002DF5B0(s32 id) {
    s32 i = 0;
    s32 keep = ~4;
    s32 *flags = (s32 *)(D_001B1E90 + 0x44);
    s32 *ids   = (s32 *)(D_001B1E90 + 0x40);
    do {
        i++;
        if (*ids == id) {
            *flags &= keep;
            return 0;
        }
        flags += 2;
        ids += 2;
    } while (i < 5);
    return 1;
}

/* Splat mis-split fragment: `addiu $sp,+0x70; nop; li $2,-1; nop;
 * sw $3,gp(D_1AB104); nop; addiu $sp,+0x10` — dead epilogue debris with no
 * prologue/jr, not a function body, so it cannot be C. Marked a fragment
 * (task #1660; same bytes as INCLUDE_ASM). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF600);

/* Source rect for func_002DF620: height at +0x20, width at +0x24. Each word
 * is read both as its low u16 (lhu) and as a full s32 (lw), so it is modelled
 * as a union. That is load-bearing: this unit's 2.9 arm alias-types
 * (-fstrict-aliasing), and a plain `*(s32 *)` read gets a separate alias set
 * from the s16 stores into dst, so cc1 hoists both lw above the stores.
 * Reading through a union member gives alias set 0, which keeps the ROM's
 * source-order loads. */
typedef union RectWord {
    s32 i;
    u16 h;
} RectWord;
typedef struct RectSrc {
    u8       _pad0[0x20];
    RectWord height;  /* 0x20 */
    RectWord width;   /* 0x24 */
} RectSrc;

/* Fill a 16-bit sprite/quad layout header from a source rect: size fields
 * from the rect's width/height, centre fields from their halves, and fixed
 * framing (+0/+2 words zero, [8] = 0x10, [9] = 0).
 * dst: the s16[10] header. src: the rect (RectSrc). No return value.
 * Byte-exact on this unit's 2.9 arm (task #681). This needs the RectSrc union
 * (above) and this store order: [5] before [8]/[9]. With the union,
 * [8]/[9] before [5] reads 71.19% and t495's order ([8] between [4] and [5])
 * reads 87.12% (unit objdiff). Without it, t495's order reads 46.88%. */
void func_002DF620(s16 *dst, void *src) {
    RectSrc *rect = (RectSrc *)src;
    dst[0] = 0;
    dst[1] = rect->width.h;
    dst[2] = 0;
    dst[3] = rect->height.h;
    dst[4] = rect->height.i >> 1;
    dst[5] = rect->width.i >> 1;
    dst[8] = 0x10;
    dst[9] = 0;
}

/* Splat mis-split fragment: a single `addiu $sp,0x60; nop` epilogue tail with no
 * prologue/jr — not a real function body. Left as bare INCLUDE_ASM. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF660);

/* Build an in-progress save image for slot `slot`: timestamp it from the CD
 * RTC, snapshot the current level WAD, build the image at `dst`, and arm the
 * 0x13-frame autosave countdown if idle. */
#ifndef TARGET_NATIVE
/* ADDRESSING-MODEL DEVICE for the g_saveClock RTC buffer that func_002DF668 and
 * func_002DF710 stamp from (emits no code; RULING #8620's form on an extern
 * declaration). It sits at FILE SCOPE, not inside either member's #else arm, so
 * that the unit's own 2.9 TU declares the symbol too: tools/ee/s136os_splice.sh
 * never carries an `.extern` for a symbol the unit already declares, so the
 * s136os TU's end-of-file `, 8` line (cc1's, from the `[8]` extent) is not
 * carried in front of the block, where it would make the access gp-relative
 * (task #1394: the splice REFUSED the in-arm placement for both members as
 * ADDRESSING D_1A7360, its name then). No 2.9 code in this unit names g_saveClock, so nothing
 * else changes. */
__asm__(".extern g_saveClock, 16");
#endif
/* GUARD (task #1394): on EE the #else body below is the image's func_002DF668, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DF668)
S136OS_SLOT(func_002DF668);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 sceCdReadClock(void *clock);
extern void func_00131A98(void *clock);
extern void func_00298A00(void);
extern void func_00297FA0(s32 saveRegion);
extern void BuildSaveImage(void *dst);
/* (end of this body's declarations) */
/* The RTC timestamp buffer the save image is stamped from (0x1A7360). The EE
 * arm names it g_saveClock (symbol_addrs, task #1973; splat's D_1A7360 before),
 * and so does the native runtime. The `.extern ,16`
 * (at file scope above this function's guard) plus the 8-byte extent is an
 * ADDRESSING-MODEL DEVICE (RULING #8620 class,
 * as D_001B1E90 above): cc1 treats it as -G8 small data and emits each use as
 * a one-insn `la` macro, so the address is materialised once per call as in
 * the ROM instead of CSE'd into a callee-saved register; the assembler then
 * expands the macro to the absolute lui/addiu pair. The 8 is NOT the size.
 * D_18D278 (the per-level save-region table) stays a plain array: given the
 * same device, the lui/addiu pair lands in swapped registers. */
#ifndef SAVE_CLOCK_BUF
#ifndef TARGET_NATIVE
extern u8 g_saveClock[8];
#define SAVE_CLOCK_BUF ((void *)g_saveClock)
#else
extern u8 g_saveClock[];
#define SAVE_CLOCK_BUF ((void *)g_saveClock)
#endif
#endif
#ifndef TARGET_NATIVE
extern u8 D_18D278[];
#endif
extern u8 g_areaTable[];
/* Build an in-progress save image for slot `slot` (stored as a halfword):
 * timestamp it from the CD RTC, snapshot the current level WAD, build the
 * image at `dst`, record dst/slot in the area-transition record (g_areaTable
 * +0x174/+0x18), clear +0x148, and arm the 0x13-frame autosave countdown
 * (+0x164, with +0x168 cleared) if it is idle (negative).
 *
 * MATCHED on the s136os arm (task #1394: vmu with the base seeded, image
 * cmp 0; screened by task #1389, SN 2.95.3 v1.36 -fopt-stack, masked words
 * and relocations). Levers: the record fields through g_areaTable (the ROM's lui/addiu
 * base in $18) rather than the five gp-relative aliases D_00139554 ...
 * D_00139548; the g_saveClock device above; `slot` an s32 (the ROM stores $5
 * with no sign extension). */
void func_002DF668(void *dst, s32 slot) {
    u8 *area = (u8 *)g_areaTable;

    sceCdReadClock(SAVE_CLOCK_BUF);
    func_00131A98(SAVE_CLOCK_BUF);
    func_00298A00();
    func_00297FA0(g_playerProgress * 0x800 + ROM_DATA_ADDR(D_18D278, 0x18d278));
    BuildSaveImage(dst);
    *(s32 *)(area + 0x174) = (s32)dst;
    *(s16 *)(area + 0x18) = slot;
    *(s32 *)(area + 0x148) = 0;
    if (*(s32 *)(area + 0x164) < 0) {
        *(s32 *)(area + 0x168) = 0;
        *(s32 *)(area + 0x164) = 0x13;
    }
}
#endif

/* Build a fresh (new-game-style) save image into `dst` for slot `slot`:
 * close any pending stream, timestamp from the CD RTC, build the image, mark
 * the slot's saved-progress word 0, and arm the autosave countdown if idle. */
/* GUARD (task #1394): on EE the #else body below is the image's func_002DF710, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DF710)
S136OS_SLOT(func_002DF710);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_00299BF8(void);
extern s32 sceCdReadClock(void *clock);
extern void func_00131A98(void *clock);
extern void BuildSaveImage(void *dst);
/* (end of this body's declarations) */
/* The RTC timestamp buffer: the same ADDRESSING-MODEL DEVICE as
 * func_002DF668's (see there; its `.extern ,16` is at file scope above
 * func_002DF668's guard). The declaration is repeated because the s136os arm
 * compiles this arm alone, and guarded so native, which compiles both, sees it
 * once. */
#ifndef SAVE_CLOCK_BUF
#ifndef TARGET_NATIVE
extern u8 g_saveClock[8];
#define SAVE_CLOCK_BUF ((void *)g_saveClock)
#else
extern u8 g_saveClock[];
#define SAVE_CLOCK_BUF ((void *)g_saveClock)
#endif
#endif
extern u8 g_areaTable[];
/* Build a fresh save image for slot `slot`: close any pending stream
 * (func_00299BF8), timestamp from the CD RTC, build the image at `dst`, then
 * in the area-transition record (g_areaTable) clear +0x148, store the slot
 * halfword at +0x18, zero the slot's saved-progress word (+0x30 + slot*0x1C),
 * record dst at +0x174, and arm the 0x13-frame autosave countdown (+0x164,
 * clearing +0x168) if it is idle (negative).
 *
 * MATCHED on the s136os arm (task #1394: vmu with the base seeded, image
 * cmp 0; screened by task #1389, SN 2.95.3 v1.36 -fopt-stack, masked words
 * and relocations). Levers: the record fields through g_areaTable instead of the
 * gp-relative aliases D_00139528 ... D_00139554 / D_00139410, with the base
 * taken AFTER the calls (the ROM materialises it into a caller-saved register
 * there; taken at entry, cc1 holds it in an extra callee-saved register), and
 * the g_saveClock device. */
void func_002DF710(void *dst, s32 slot) {
    u8 *area;

    func_00299BF8();
    sceCdReadClock(SAVE_CLOCK_BUF);
    func_00131A98(SAVE_CLOCK_BUF);
    BuildSaveImage(dst);
    area = (u8 *)g_areaTable;
    *(s32 *)(area + 0x148) = 0;
    *(s16 *)(area + 0x18) = slot;
    *(s32 *)(area + slot * 0x1C + 0x30) = 0;
    *(s32 *)(area + 0x174) = (s32)dst;
    if (*(s32 *)(area + 0x164) < 0) {
        *(s32 *)(area + 0x168) = 0;
        *(s32 *)(area + 0x164) = 0x13;
    }
}
#endif

/* In-RAM post-load progress rebuild: snapshots the PlayerStats currency block,
 * rebuilds weapon/skill-point/cheat/inventory tables, calls func_00299BF8, then
 * restores the saved currency and refreshes the save image. NOT the memcard I/O.
 * Wall: ~0x3760-byte stack frame, huge memcpy/SIMD snapshot loops, many named
 * global blocks — far beyond a faithful #else. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", RestorePlayerProgressState);

/* Set the skill-points / unlockables summary page's icon frames and caption
 * strings from the three progress counters. CountSkillPointsCompleted() is
 * taken twice (>= 15 and >= 30 skill points), func_002B1D40() once (>= 10).
 * Each test picks a "complete" or "partial" value for:
 *   - four halfword icon fields in the D_0025EBB0 widget block (+0x3E, +0x4A,
 *     +0x56, +0x62; 3 or 10 when complete, 2 when partial);
 *   - four caption string ids (D_1AB2BC..D_1AB2C8, 0x2CB5/0x2CB6/0x2CB9/0x2CBA
 *     when complete, 0x2CBB/0x2CBC/0x2CBD/0x2CBD when partial).
 * When the D_1A7318 selector is nonzero it also clears the block's +0x60
 * halfword. No return value. Called from 1CA080's galactic-map refresh.
 * MATCHED on the s136os arm (SN 2.95.3 v1.36 cc1plus -fopt-stack), task #2289.
 * It needs gcse on: the ROM keeps %hi(D_0025EBB0) in $11 and re-forms %lo for
 * each halfword store, the cross-block HIGH copy only gcse makes (NOTE #9901),
 * so it could not close before task #2252 unpinned this unit's s136os arm.
 * Two devices:
 *   - the four caption ids and D_1A7318 are cc1-small but assembler-absolute
 *     (`.extern ,16`, EE arm only), as D_001B1E90 is above. cc1 emits each as
 *     one macro and gas expands it to the ROM's lui/sw (`lui $1`) or lui/lw
 *     (in the destination register). Read as a cc1-split global instead,
 *     D_1A7318's %hi is scheduled early and costs one register through the
 *     whole function (every temporary shifts up by one).
 *   - the last caption store is volatile. That is a SCHEDULING DEVICE, not a
 *     claim that the word is shared with anything asynchronous: cc1 believes
 *     the store is one instruction and would put it in the D_1A7318 branch's
 *     delay slot. The assembler then hoists the two-word expansion and leaves
 *     a nop (asm_unit.sh's SN-as slot rule). The ROM fills that slot with the
 *     +0x60 address instead, which reorg does once the store cannot move (the
 *     same volatile-memop-before-branch shape as text/250080 func_00352B90).
 *     An empty __asm__ fence before the `if` was measured instead and is worse
 *     (19 words differ).
 * GUARD: compiled alone by SN 2.95.3 v1.36 -fopt-stack and spliced over the
 * S136OS_SLOT line (tools/ee/s136os_splice.sh); plain C on native. */
/* The assembler-absolute device above. File scope, outside the guard, so the
 * 2.9 TU the block is spliced into carries the same sizes ahead of it. */
#ifndef TARGET_NATIVE
__asm__(".extern D_1AB2BC, 16");
__asm__(".extern D_1AB2C0, 16");
__asm__(".extern D_1AB2C4, 16");
__asm__(".extern D_1AB2C8, 16");
__asm__(".extern D_1A7318, 16");
#endif
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002DFE60)
S136OS_SLOT(func_002DFE60);
#else
extern s32 CountSkillPointsCompleted(void);
extern s32 func_002B1D40(void);
extern u8 D_0025EBB0[];           /* summary-page widget block */
extern s32 D_1AB2BC, D_1AB2C0, D_1AB2C4;  /* caption string ids */
extern volatile s32 D_1AB2C8;              /* volatile: the slot device above */
extern s32 D_1A7318;

void func_002DFE60(void) {
    s32 skillsHalf = CountSkillPointsCompleted() >= 15;
    s32 skillsAll = CountSkillPointsCompleted() >= 30;
    s32 extras = func_002B1D40() >= 10;
    u8 *page;

    page = D_0025EBB0; *(s16 *)(page + 0x3E) = skillsHalf ? 3 : 2;
    page = D_0025EBB0; *(s16 *)(page + 0x4A) = skillsAll ? 3 : 2;
    page = D_0025EBB0; *(s16 *)(page + 0x56) = extras ? 10 : 2;
    page = D_0025EBB0; *(s16 *)(page + 0x62) = extras ? 3 : 2;
    D_1AB2BC = skillsHalf ? 0x2CB5 : 0x2CBB;
    D_1AB2C0 = skillsAll ? 0x2CB6 : 0x2CBC;
    D_1AB2C4 = extras ? 0x2CB9 : 0x2CBD;
    D_1AB2C8 = extras ? 0x2CBA : 0x2CBD;
    if (D_1A7318 != 0) {
        u8 *block = D_0025EBB0;
        *(s16 *)(block + 0x60) = 0;
    }
}
#endif

/* Galactic-Map save/load progress accumulator: while the save page is active
 * (g_menuScreenBlock+0x168 != 0), advance its byte counter (+0x16C) by `amount` and
 * forward `handle` and `amount` to func_002A1138. No return value: the
 * original never materialises one (the inactive path leaves the loaded flag
 * in $v0), so the function is void; its one caller (func_002DA358) ignores it.
 * MATCHED 100.00% on the sdk29 arm (unit objdiff; verify_match_unit.sh BYTE
 * IDENTICAL, t495). The asm barrier keeps the jal + $ra frame — cc1 2.9 would
 * otherwise tail-jump to func_002A1138 (FACT #7343's void shape). */
void func_002DFF68(s32 handle, s32 amount) {
    MenuState *menu = (MenuState *)g_menuScreenBlock;
    if (menu->savePageActive != 0) {
        menu->savePageBytes += amount;
        func_002A1138(handle, amount);
        __asm__ __volatile__("");   /* keep the jal + ra frame; cc1 would tail-jump */
    }
}

/* Play a menu/system sound (id,arg) only when the menu-sound gate
 * g_menuSoundEnabled is nonzero. Nothing in the boot ELF stores to the gate
 * (boot value 0), so whatever enables it lies outside the split image.
 * MATCHED 100.00% on the sdk29 arm (unit objdiff; verify_match_unit.sh BYTE
 * IDENTICAL, t495). The asm barrier keeps the jal + $ra frame — cc1 2.9 would
 * otherwise tail-jump to PlayGlobalSound. */
void func_002DFFA0(s32 id, s32 arg) {
    if (g_menuSoundEnabled != 0) {
        PlayGlobalSound(id, arg, 0);
        __asm__ __volatile__("");   /* keep the jal + ra frame; cc1 would tail-jump */
    }
}

/* Always-ready gate: return 1. */
s32 func_002DFFC8(void) {
    return 1;
}

/* Splat mis-split fragment: two `addiu $sp,0x10; nop` runs, no prologue/jr —
 * not a real function body. Left as bare INCLUDE_ASM. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFFD0);

/* Is the current menu screen the fixed screen instance D_00259438? (pointer
 * compare lowered to xor + sltiu).
 * MATCHED 100.00% on the sdk29 arm as written (unit objdiff;
 * verify_match_unit.sh BYTE IDENTICAL, t495). */
s32 func_002DFFE0(void) {
    extern u8 D_00259438[];
    return g_pCurrentMenuScreen[0] == (void *)D_00259438;
}

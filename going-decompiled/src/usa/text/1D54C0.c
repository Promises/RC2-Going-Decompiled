#include "common.h"

/*
 * text/1D54C0 — front-end / pause-menu screens band, part B (vaddr
 * 0x2D5540..0x2DFFFF). Carved out of the big text/1B21E8 asm tile as
 * ranked-carve-pipeline pick #5 ("menu-screens"); part A is text/1CA080.
 * This part holds the Galactic Map screen, the text-table streamer, the menu
 * background-image loader, and the screen-command (MenuScreenDoAction)
 * forwarding wrappers.
 *
 * Built at -O2 -G8 -fno-gcse (later SN cc1 TU model — see the part-A header for
 * the full -G8 extern-sizing rules). Walls left as INCLUDE_ASM (per function):
 * the 8-byte-packed callee-save wall (2+ GPR saves), the delay-slot store
 * scheduling wall (a lone global store + return-0 the original keeps OUT of
 * the jr delay slot while cc1 sinks it in), pad-load reorder, register
 * coloring, switch/jtbl, and handwritten frameless stub fragments.
 */

/* 2D draw-batch begin/end fence used by every menu draw function. */
extern void Begin2dDrawBatch(s32 mode);
extern void End2dDrawBatch(void);

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
 *   +0x30  u32  subtype state/flags  CONFIRMED (func_002DB080 switch driver)
 *   +0x34  ptr  data/cmd/row table   CONFIRMED (cmd table for func_002D6AA0)
 *   +0x38  s32  selected-row/scroll  CONFIRMED (func_002DC0F0/378/520)
 *   +0x3C  s32  sub-cursor / uv x    CONFIRMED (func_002DAE70/DB080)
 *   +0x40  s32  cursor / selection   CONFIRMED (func_002D6AA0 row, func_002D87C8)
 *   +0x44  s32  subtype state / level-array base CONFIRMED
 *   +0x48  s32  map-slot id A        CONFIRMED (func_002DB028/DD7E8/DD858)
 *   +0x4C  s32  map-slot id B        CONFIRMED
 *   +0x50  s32  cached state / slot  CONFIRMED
 *   +0x54  s32  cached state / slot  CONFIRMED (func_002DC838/878/CCC8)
 *   +0x58  s32  cached state / uv    CONFIRMED (func_002DAE70/DB080)
 *   +0x5C  s32  stream frame counter CONFIRMED (func_002DAAF8/DB080)
 *   +0x60  s32  stream scroll offset CONFIRMED (func_002DA358/DAAF8/DB080)
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

/* Player character/control mode: 0=Ratchet, 1=Clank-solo, 2=Giant Clank. */
extern u8 g_bPlayerMode;

/* Map state block. The galactic-map slot table is 5 interleaved {id,flags}
 * pairs (stride 8) starting at +0x40 (id) / +0x44 (flags). */
extern u8 D_001B1E90[];

/* Front-end / pause-menu screen-manager pointers (array-modelled for the
 * explicit lui/lw address form). */
extern void *g_pCurrentMenuScreen[];  /* active screen instance */
extern void *g_pNextMenuScreen[];     /* requested next screen instance */
extern u8 D_25E660[];                 /* a specific menu-screen instance */

/* Menu-subsystem state block. Only the Galactic-Map save-page fields are
 * modelled here; the rest is opaque padding. */
typedef struct MenuState {
    u8  _pad0[0x168];
    s32 savePageActive;  /* 0x168 - nonzero while the save page is up */
    s32 savePageBytes;   /* 0x16C - running byte counter for the save */
} MenuState;
extern MenuState D_1F27C0;
extern s32 func_002A1138(s32 arg);

/* Galactic-map cache state (array-modelled for the explicit address form). */
extern s32 D_25BA60[];        /* map upload sequence counter */
extern s32 g_mapActiveSlot[]; /* active map cache slot index */

/* Persistent save block; its first word seeds a few rotating lookups. */
extern s32 g_playerProgress[];
extern s32 D_260570[];        /* 19-entry lookup table */

/* Galactic-map planet-row builder state. */
extern s32 D_1A7C0C[];        /* number of active planet rows */
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
/* Sprite/quad sub-rect submit (x0,x1,y0,y1). */
extern void func_002897B0(s32 x0, s32 x1, s32 y0, s32 y1);
/* UI/system sound trigger from the global sound-def pool. */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
/* Map-slot toggle/lookup helpers (defined later in this unit). */
extern s32 func_002DF428(s32 id);
extern s32 func_002DF368(s32 forceSet);
/* Sound-mute gate flag (nonzero => emit the menu sound). */
__asm__(".extern D_1ABD48, 4");
extern s32 D_1ABD48;

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
extern u8  D_001ABD48;             /* (same as D_1ABD48, byte view) */
extern s32 g_nSaveLoadStatusCode;  /* 0x1A7420 save/load popup status code */
extern u8  g_menuScreenBlock[];    /* g_particleFxBlob+0x100 menu-screen mgr block */
extern s32 g_playerProgress2;      /* alias for the persistent save block first word */
extern void func_002DFFA0(s32 id, s32 arg);  /* gated menu-sound helper (below) */
extern u8 *g_pTextTableLoadBuf;    /* 0x1F28D8 language text-table load buffer */

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5540);

/* Toggle the map slot at g_particleFxBlob+0x100 +0x1CC via func_002DF428 and
 * store the result back. Returns 0. Wall: 2-GPR callee-save (8-byte-packed 0x10
 * frame). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5A10);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D5A10(void) {
    u8 *blk = (u8 *)g_menuScreenBlock;
    *(s32 *)(blk + 0x1CC) = func_002DF428(*(s32 *)(blk + 0x1CC));
    return 0;
}
#endif

/* Draw the title-screen language-select glyph row (GuiFontAtlas lookups + two
 * localized strings). Wall: $f20 callee-save + many leaf calls + gp/abs FP
 * constant mix. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5A48);

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
 * standard exit/back navigation from g_padButtonsPressed: L1/R1 (0x900) returns
 * 1 unless an override (g_menuScreenBlock+0x134) is set; on back (0x10) mirrors
 * the next-screen pointer into +0x18 (returning 0), or requests parent (-1) /
 * stays (0). Returns 0/1/-1.
 * Wall: gp/absolute address mix + bnel/beql ladder. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5C58);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D5C58(void *focus) {
    extern s32 D_1ABAF0[];           /* per-language bg image index table */
    extern s32 g_menuBgImageIndex;
    extern u8  g_currentLanguage;
    u8 *blk = (u8 *)g_menuScreenBlock;   /* g_menuScreenBlock == D_1F27C0 */
    u8 *scr;
    s32 pressed;
    g_menuBgImageIndex = D_1ABAF0[g_currentLanguage];
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

/* Draw the title-screen menu header glyph row (GuiFontAtlas lookups + two
 * localized strings). Wall: $f20 callee-save + many leaf calls + gp/abs FP
 * constant mix. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5D10);

/* Seed obj->0x34 from a GUI subsystem query (g_pGuiManager + 0x3C160) when the
 * GUI exists. Returns 0. Wall: 2-GPR callee-save. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5EC8);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D5EC8(MenuWidget *obj) {
    if (g_pGuiManager != 0) {
        *(s32 *)((u8 *)obj + 0x34) = func_00342468((u8 *)g_pGuiManager + 0x3C160);
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
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D5F10(void) {
    s32 pressed = g_padButtonsPressed[0];
    if (func_00288898() == 2 && GetMenuOverlayMode() == 9) {
        func_002888A8();
        RequestLevelExit(-1, 0);
        return 1;
    }
    func_002888A8();
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
    func_0029D398(g_padButtonsPressed[0]);   /* pass the pad mask (asm loads g_padButtonsPressed[0] into a0 at 0x2d5f74) */
    if ((pressed & 0x40) && g_pGuiManager != 0) {
        s32 item = func_003424C8((u8 *)g_pGuiManager + 0x3C160);
        s16 cmd[8];
        cmd[1] = *(s16 *)(item + 8);
        *(s32 *)(&cmd[2]) = *(s32 *)(item + 0xc);
        func_002D6B00((MenuCmd *)cmd);
    }
    return 0;
}
#endif

/* Draw the ship-customization back/help line: a right-justified label whose
 * Y follows the customization-panel scroll fraction (offset by 36px when a
 * sub-panel is open). Returns 0. Wall: float scroll arithmetic. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6028);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D6028(void) {
    extern float D_001B2324;
    extern s32   D_001B2320;
    float yf;
    Begin2dDrawBatch(0);
    func_0029D368();
    yf = D_001B2324 * 329.0f + 0.5f;
    if (D_001B2320 == 1) {
        yf -= 36.0f;
    }
    DrawFont1RightJustifiedLabel(0x1c7, (s32)yf, 0x80f0f0f0,
                                 GetLocalizedString(0x2be5), -1);
    End2dDrawBatch();
    return 0;
}
#endif

/* Enter the ship-customization screen: prime the system, then (if the GUI
 * exists) bind its three data sources to the customization list and show it. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D60E8);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D60E8(void) {
    func_002888A8();
    if (g_pGuiManager != 0) {
        func_00342450((u8 *)g_pGuiManager + 0x3C160, 0x2615d8, 0x261678, 0x261730);
        func_00342520((u8 *)g_pGuiManager + 0x3C160, 1);
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6248);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D6248(void) {
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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D63B0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D63B0(void) {
    if (g_pGuiManager != 0) {
        func_00342BE8((u8 *)g_pGuiManager + 0x3E9C8, 1);
        func_00342DC0((u8 *)g_pGuiManager + 0x3E9C8, 0x261520);
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
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D6408(void) {
    s32 pressed = g_padButtonsPressed[0];
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
    func_0029D8A8(g_padButtonsPressed[0]);   /* pass the pad mask (asm loads g_padButtonsPressed[0] into a0 at 0x2d6420) */
    if ((pressed & 0x40) && g_pGuiManager != 0) {
        s32 item = func_003432C0((u8 *)g_pGuiManager + 0x3e760);
        s16 cmd[8];
        cmd[1] = *(s16 *)(item + 8);
        *(s32 *)(&cmd[2]) = *(s32 *)(item + 0xc);
        func_002D6B00((MenuCmd *)cmd);
    }
    return 0;
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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6508);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D6508(void) {
    if (g_pGuiManager != 0) {
        func_003432B8((u8 *)g_pGuiManager + 0x3E760, 1);
        func_00343290((u8 *)g_pGuiManager + 0x3E760, 0x261570);
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D65D8);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D65D8(void) {
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
        g_nMapCurrentLevel = g_playerProgress[0];
        return 1;
    }
    if ((pressed & 0x40) == 0) {
        func_0029D248();
        return 0;
    }
    if (g_playerProgress[0] == g_nMapCurrentLevel) {
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

/* Central front-end/pause screen-action dispatcher (op, arg, out-flag ptr).
 * Wall: large jump-table switch over the opcode + multi callee-save frame.
 * Bare INCLUDE_ASM (asm is the source of truth for the jtbl layout). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", MenuScreenDoAction);

/* Register-coloring near-miss (97%): list-entry screen-action forwarder.
 * Dispatch op `op` with the arg pulled from the selected row of the widget's
 * command table (row = widget->0x40, stride 0xc, arg at +4). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6AA0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D6AA0(void *widget, s32 op, void *outFlag) {
    u8 *w = (u8 *)widget;
    s32 row = *(s32 *)(w + 0x40);
    s32 arg = *(s32 *)(row * 0xc + *(s32 *)(w + 0x34) + 4);
    return MenuScreenDoAction(op, arg, outFlag);
}
#endif

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6B28);

/* Draw the galactic-map planet list (per-row labels, highlight, level-name
 * substitution). Wall: large draw loop + many leaf calls + pointer-compare
 * ladder over fixed screen instances. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6E98);

/* Compute the galactic-map fade-alpha colour from a pulse value. Wall: ACC-madd
 * / FP-heavy leaf the cc1 schedules differently. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", SetGalacticMapFadeAlpha);

/* Galactic-map grid cursor input (paged Up/Down/Left/Right with edge wrap into
 * linked sibling screens). Wall: very large nested branch graph + divide traps.
 * Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D75D8);

/* Galactic-map grid renderer (planet icons + selection highlight pulse). Wall:
 * heavy FP/ACC pulse math + large nested draw loop + sq/lq packing. Bare. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D7AE0);

/* Galactic-map screen per-frame tick (advance fades, dispatch sub-screens).
 * Wall: large state machine + multi callee-save frame. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", GalacticMapScreenTick);

/* Refresh the objectives screen: recompute objective states, gather the active
 * objective list into scratch buffers, store the count in obj->0xA0, and (when
 * the current map level has a valid entry) seed the preview record globals.
 * Returns 0. Wall: scratch-buffer absolute addresses + 2-GPR save. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8270);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D8270(MenuWidget *obj) {
    extern s32 D_0025A6FC, D_0025A6F4, D_0025A600;
    u8 *o = (u8 *)obj;
    s32 *pLevel;
    UpdateLevelObjectiveStates();
    *(s32 *)(o + 0xA0) = GatherActiveObjectives(0x70000000, 0, 0x70000100, 1);
    pLevel = (s32 *)(o + 0x30 + g_nMapCurrentLevel * 4);
    if (*pLevel != -1) {
        D_0025A6FC = 0xfffffdc0;
        D_0025A6F4 = *(s32 *)(*pLevel * 4 + 0x70000000);
        D_0025A600 = *(s32 *)(0x70000100 + *pLevel * 4);
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8770);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D87C8);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D87C8(MenuWidget *obj) {
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

/* Draw the title-screen audio-options sliders (music/sfx bars + mono/stereo
 * label). Wall: heavy FP bar-fill math + many DrawHudIconQuad/Font leaf calls.
 * Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8A68);

/* Rebuild the galactic-map planet display rows: for each of the D_1A7C0C
 * active rows, mark it active (flag=1) and set its icon from D_254E48 indexed
 * by the (reversed) source-order byte in D_18D0E8; then clear the icon of the
 * row just past the last. Returns 0.
 * Near-miss: our cc1 strength-reduces the reversed D_18D0E8 index into a
 * decrementing pointer; the original recomputes &D_18D0E8[n-1-i] each pass. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8DD0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D8DD0(void) {
    s32 n = D_1A7C0C[0];
    s32 i;
    for (i = 0; i < n; i++) {
        D_25AD90[i].flag = 1;
        D_25AD90[i].icon = (u16)D_254E48[D_18D0E8[n - 1 - i]];
    }
    D_25AD90[D_1A7C0C[0]].icon = 0;
    return 0;
}
#endif

/* return 0 stub. */
s32 func_002D8E58(void) {
    return 0;
}

/* Begin a galactic-map text-table swap: reset the screen object's +0x54/+0x38
 * pointers, then for each of the 5 map slots whose id is set and below the
 * second-bank base, OR the in-use bit (2) into its flags; finally clear +0x50
 * and request a reload (obj +0x10 bit 4). Returns 0.
 * Wall: 2-GPR callee-save + pointer-stride loop. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8E60);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002D8E60(MenuWidget *obj) {
    s32 *ids   = (s32 *)(D_001B1E90 + 0x40);
    s32 *flags = (s32 *)(D_001B1E90 + 0x44);
    s32 i;
    func_002DF1B8(1);
    *(s32 *)((u8 *)obj + 0x54) = 0;
    *(s32 *)((u8 *)obj + 0x38) = 0;
    for (i = 0; i <= 4; i++) {
        if (ids[i * 2] != 0 && (u32)ids[i * 2] < (u32)D_001F28DC) {
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", RestorePrevTextTable);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 RestorePrevTextTable(void *cmd) {
    extern s32 g_nActiveTextTableCount;
    extern void *g_pActiveTextTable;
    u8 *c = (u8 *)cmd;
    if (g_nFileLoadState != 0 && *(s32 *)(c + 0x50) == 3) {
        StopFileLoad();
    }
    func_002DF1B8(1);
    if (*(void **)(c + 0x54) != 0) {
        g_nActiveTextTableCount = *(s32 *)(c + 0x38);
        g_pActiveTextTable = *(void **)(c + 0x54);
    }
    return 0;
}
#endif

/* Stream the per-language text table into g_pTextTableLoadBuf, relocate the
 * entry string pointers, install it as active and save the previous table into
 * cmd+0x54/+0x38. Wall: large streaming state machine + multi callee-save +
 * pointer-relocation loop. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", StreamTextTable);

/* Draw a wrapped-text-box menu list (skill-points / level-select variant) with
 * per-row scroll clamping and checkbox indicators. Wall: ~250-instruction draw
 * loop over a 128-bit-packed stack text-box struct. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9178);

/* Draw the default text-box menu list (sibling of func_002D9178 with a simpler
 * box). Wall: ~250-instruction draw loop + sq/lq 128-bit struct packing. Bare. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9718);

/* Position + populate one weapon-grid ammo readout cell. Computes the linear
 * cell index (col + row*obj->0x44), fetches the GUI list element, and — when the
 * weapon in that grid slot has an ammo capacity record — shows it with the
 * current ammo scrolled in; finally re-lays out the cell. Wall: GUI element
 * arithmetic + indexed inventory tables. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9C18);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
void func_002D9C18(MenuWidget *obj, void *entry, s32 col, s32 row, s32 x, s32 y) {
    extern u8  g_itemEquippedSlot[];
    extern s32 g_weaponAmmoCapacity[];
    extern u8  D_00239B8C[];   /* per-weapon ammo-capacity record table (stride 0xE0) */
    u8 *o = (u8 *)obj;
    s16 slot = *(s16 *)((u8 *)entry + 6);
    s32 idx = row * *(s32 *)(o + 0x44) + col;
    s32 elem = func_0034F300((u8 *)g_pGuiManager + 0x36F28) + idx * 0x48;
    GuiElementSetVisible(elem, 0);
    if (*(s32 *)(D_00239B8C + g_itemEquippedSlot[slot] * 0xE0) != 0) {
        GuiElementSetVisible(elem, 1);
        GuiListSetScrollPos(elem, g_weaponAmmoCapacity[slot] >> 5);
        GuiListSetItemCount(elem, *(s32 *)(D_00239B8C + g_itemEquippedSlot[slot] * 0xE0));
    }
    func_0034EF68((u8 *)g_pGuiManager + 0x36F28, idx, x, y, 0x80);
}
#endif

/* Draw the weapon/inventory grid (icon cells with ownership/selection state +
 * ammo readout via func_002D9C18). Wall: very large nested draw loop + FP pulse
 * highlight + many indexed inventory tables. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9D60);

/* Set a draw record's mode field (+0x2 of the object at obj->0x34): in
 * Clank-solo (g_bPlayerMode==1) use 0, otherwise 3. Always returns 0.
 * Near-miss: register-coloring (original reuses $2 for the value + return;
 * our cc1 colours the value into $5). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA330);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DA330(MenuWidget *obj) {
    s16 *rec = *(s16 **)((u8 *)obj + 0x34);
    rec[1] = (g_bPlayerMode == 1) ? 0 : 3;
    return 0;
}
#endif

/* Per-frame scan of the 24 galactic-map level entries (obj +0x44 stride 4):
 * for each populated, not-yet-handled level whose level-order word (D_00261900)
 * is set, advance its save accumulator via func_002DFF68 — skipping a small set
 * of indices when a 0x9999-tagged level id falls in [0x4D,0x92). Returns 4.
 * Wall: jump-table switch on the row index. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA358);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DA358(MenuWidget *obj) {
    extern s32 D_00261900[];
    u8 *o = (u8 *)obj;
    s32 i;
    for (i = 0; i < 0x18; i++) {
        s32 *row = (s32 *)(o + 0x44 + i * 4);
        if (*row != 0 && *(s8 *)(o + 0xA4 + i) == 0 && D_00261900[i] != 0) {
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
            func_002DFF68(*row, 1);
        }
    }
    return 4;
}
#endif

/* Galactic-map / front-end screen draw helper (variant of the menu list draw).
 * Wall: large draw loop + multi callee-save + 128-bit struct packing. Bare. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA488);

/* Draw the title-screen main menu: measure the widest of the fixed option
 * strings (one extra "continue" row when a save exists), left-align the column,
 * and draw the rows evenly down the widget. Returns 2. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA4F0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DA4F0(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 hasSave = (g_playerProgress[0] != 0);
    s32 w, x, step, rowY;
    s32 maxw;
    static const s32 ids[] = {0x2bf3, 0x2bff, 0x2c00, 0x2c01, 0x2be5};
    s32 i;
    AppendGsRegPacket(0x42, 0x44);
    AppendGsRegPacket(0x47, 0xb);
    Begin2dDrawBatch(0);
    maxw = MeasureFont2Text(GetLocalizedString(0x2bf3), -1);
    { s32 t = MeasureFont2Text(GetLocalizedString(0x2bff), -1); if (maxw <= t) maxw = t; }
    if (hasSave) {
        s32 t = MeasureFont2Text(GetLocalizedString(0x2bf4), -1);
        if (maxw <= t) maxw = t;
    }
    { s32 t = MeasureFont2Text(GetLocalizedString(0x2c00), -1); if (maxw <= t) maxw = t; }
    { s32 t = MeasureFont2Text(GetLocalizedString(0x2c01), -1); if (maxw <= t) maxw = t; }
    { s32 t = MeasureFont2Text(GetLocalizedString(0x2be5), -1); if (maxw <= t) maxw = t; }
    x = (*(s32 *)(o + 0x20) - maxw) >> 1;
    if (x < 2) x = 2;
    w = hasSave ? 7 : 6;
    step = *(s32 *)(o + 0x24) / w;
    func_0027F7A0();
    rowY = step - 6;
    for (i = 0; i < 5; i++) {
        DrawDebugString(x, rowY, 0x80ffa888, GetLocalizedString(ids[i]), -1);
        rowY += step;
    }
    EnableInlineColorCodes();
    End2dDrawBatch();
    return 2;
}
#endif

/* Draw the active-objectives list (gathered via GatherActiveObjectives) with a
 * scrolling text box + per-row checkboxes. Wall: ~200-instruction draw loop +
 * 128-bit packed text-box struct + divide traps. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA740);

/* Draw a small checkbox/indicator at (x,y): a 10px highlight rect, an 8px inner
 * rect, and (when `on`) a 0x1E-px tick glyph on top. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DAA50);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
void func_002DAA50(s32 x, s32 y, s32 on) {
    extern s32 D_1ABB6C;  /* %gp inner-rect colour */
    func_002904B0(x - 5, y - 5, x + 5, y + 5, 0x80ffa888, 0);
    func_002904B0(x - 4, y - 4, x + 4, y + 4, D_1ABB6C, 0);
    if (on) {
        DrawHudIconQuadTiled(func_0028EDF0(0xe99d, 1), x - 0xd, y - 0x12, 0x1e, 0x1e, 0x80);
    }
}
#endif

/* Streaming-image widget state machine: drives a multi-stage WAD load/decompress
 * for the current map level's preview texture and uploads it to VRAM. Wall:
 * ~250-instruction state machine + StartFileLoad/DecompressWad orchestration +
 * the same partially-recovered slot table. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DAAF8);

/* Draw the streamed full-screen image widget once it has loaded (state>=2 and
 * not error): blit the decoded glyph quad at the configured uv (+0x38/0x3C);
 * returns 0x10 when drawn, else 0. Wall: redundant branch (both arms equal). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DAE70);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DAE70(MenuWidget *obj) {
    extern s16 D_001A65E0;
    extern s16 D_001A65E2;
    u8 *o = (u8 *)obj;
    if (*(s32 *)(o + 0x44) > 1 && *(s32 *)(o + 0x58) >= 0) {
        DrawGlyphQuad(0, 0, D_001A65E0, D_001A65E2, 0, 0,
                      *(s32 *)(o + 0x38), *(s32 *)(o + 0x3C));
        return 0x10;
    }
    return 0;
}
#endif

/* Allocate / initialise the menu background-image double buffers. Wall: multi
 * callee-save frame + buffer arithmetic + leaf alloc calls. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", InitMenuBgImageBuffers);

/* Reset a two-slot streaming text widget: toggle both map slots referenced by
 * obj->0x48/0x4C, invalidate the cached state (+0x44/0x50/0x54 = -1) and kick
 * the dialog-voice pump. Returns 0. Wall: 2-GPR callee-save. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DB028);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DB028(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    *(s32 *)(o + 0x48) = func_002DF428(*(s32 *)(o + 0x48));
    *(s32 *)(o + 0x4C) = func_002DF428(*(s32 *)(o + 0x4C));
    *(s32 *)(o + 0x44) = -1;
    *(s32 *)(o + 0x50) = -1;
    *(s32 *)(o + 0x54) = -1;
    PumpDialogVoiceSystem(1);
    return 0;
}
#endif

/* On-screen help/hint/objective text selector + reveal state machine (reader of
 * g_skillPointFlags). Wall: ~350-instruction nested branch + switch state graph
 * over the partially-recovered slot table. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DB080);

/* Draw the streamed help/preview image (loading/autosave overlay variants).
 * Wall: ~200-instruction branch graph + 128-bit packed text-box struct + the
 * DrawGlyphQuad blit. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DB700);

/* Load a menu background-image pair (front/back) from disc into the bg buffers.
 * Wall: multi callee-save + StartFileLoad orchestration. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", LoadMenuBgImagePair);

/* Upload the loaded menu background-image pair to VRAM (GS texture transfer).
 * Wall: GS/VIF packet build + sq/lq 128-bit DMA tags. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", UploadMenuBgImagePair);

/* Draw the animated galactic-map planet-cursor sprite at the active slot, with
 * a pulsing scale driven by the global frame counter and a layout that differs
 * for the save-screen vs map-screen instances. Returns 4 when drawn, else 0.
 * Wall: large constant-layout block + GS register packets. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DBC98);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DBC98(void) {
    extern s32 D_00261978[];
    extern s32 D_001B1518;
    extern void *D_001C5188, *D_001C5198;
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC378);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DC378(MenuWidget *obj) {
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

/* Draw an options list (rows of stride 0x18): each row is a left label plus a
 * right-justified value picked by the row's current option byte. The selected
 * row (obj->0x38) is highlighted. Rows are spaced evenly. Returns 2. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC520);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DC520(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 *rows = *(s32 **)(o + 0x34);
    s32 n = 0;
    s32 step, y, i;
    AppendGsRegPacket(0x47, 0x2004b);
    Begin2dDrawBatch(0);
    while (rows[n * 6] != 0) n++;
    step = *(s32 *)(o + 0x24) / (n + 1);
    if (rows[0] != 0) {
        y = step - 8;
        for (i = 0; ; i++) {
            s32 *row = (s32 *)((u8 *)*(s32 **)(o + 0x34) + i * 0x18);
            s32 col = (i == *(s32 *)(o + 0x38)) ? 0x8020ffff : 0x80ffa888;
            u8 *optByte = *(u8 **)(row + 1);
            DrawStringFont1(0xc, y, col, GetLocalizedString(row[0]), -1);
            DrawFont1RightJustifiedLabel(*(s32 *)(o + 0x20) - 0xc, y, 0x80ffa888,
                                         GetLocalizedString(row[*optByte + 2]), -1);
            if (*(s32 *)((u8 *)*(s32 **)(o + 0x34) + (i + 1) * 0x18) == 0) break;
            y += step;
        }
    }
    End2dDrawBatch();
    return 2;
}
#endif

/* Build the galactic-map level-select list: for each available level (from
 * g_anAvailableLevelOrder, up to 0x1C entries) fill a 0xC-stride list entry at
 * g_pLevelSelectListEntries+0xA8 (icon/name from the D_262BA0 caption table,
 * sub-mode 3, target screen &D_25E660); zero the entry past the last. Then mark
 * the widget dirty (obj->0x30 |= 0x8000), clear obj->0x40, and scan the active
 * level-id list (D_1AA510) for the current map level (g_mapVertexData+0x230),
 * storing its row into obj->0x40. Returns 0.
 * Wall: leading splat mis-split fragment + draw-loop schedule. Bare. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC6B8);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DC6B8(MenuWidget *obj) {
    extern s32 g_anAvailableLevelOrder[];
    extern u8  D_262BA0[];                /* level caption table (stride 0xC) */
    extern u8  g_pLevelSelectListEntries[];
    extern u8  D_25E660[];                /* level-detail screen instance */
    extern s32 *D_1AA510;                 /* active level-id list */
    extern u8  g_mapVertexData[];
    u8 *o = (u8 *)obj;
    u8 *e = g_pLevelSelectListEntries + 0xA8;
    s32 n = 0;
    s32 lvl = g_anAvailableLevelOrder[0];
    if (lvl != 0) {
        do {
            u8 *cap = D_262BA0 + lvl * 0xC;
            *(s16 *)(e + 2) = 3;
            *(s32 *)(e + 4) = (s32)D_25E660;
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
        s32 *list = D_1AA510;
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
 * at D_25E660 as the next screen. Always returns 0. */
s32 func_002DC7D8(void) {
    if (g_padButtonsPressed[0] & 0x40) {
        g_pNextMenuScreen[0] = D_25E660;
    }
    return 0;
}

/* Submit the menu object's sub-rect (origin +0x18/+0x1C, size +0x20/+0x24) to
 * the 2D batch helper; always returns 2. */
s32 func_002DC800(MenuWidget *obj) {
    s32 x = *(s32 *)((u8 *)obj + 0x18);
    s32 y = *(s32 *)((u8 *)obj + 0x1C);
    func_002897B0(x, x + *(s32 *)((u8 *)obj + 0x20),
                  y, y + *(s32 *)((u8 *)obj + 0x24));
    return 2;
}

/* Clear the current screen's +0x12C field, then store the result of the
 * map-slot allocator func_002DF368(0) into obj->0x54. Returns 0.
 * Wall: 2-GPR callee-save (8-byte-packed 0x10 frame). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC838);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DC838(MenuWidget *obj) {
    *(s32 *)((u8 *)g_pCurrentMenuScreen[0] + 0x12C) = 0;
    *(s32 *)((u8 *)obj + 0x54) = func_002DF368(0);
    return 0;
}
#endif

/* Toggle the map-slot referenced by obj->0x54 via func_002DF428 and store the
 * result back. Returns 0. Wall: 2-GPR callee-save (8-byte-packed 0x10 frame). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC878);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DC878(MenuWidget *obj) {
    *(s32 *)((u8 *)obj + 0x54) = func_002DF428(*(s32 *)((u8 *)obj + 0x54));
    return 0;
}
#endif

/* Save/load list confirm/cancel handler. Mirror the active screen object's
 * +0xE0 field into the manager block's +0x18 and drive the +0x12C commit flag
 * from the just-pressed confirm(0x20)/cancel(0x10) buttons. Returns 0.
 * Wall: multi-target branch ladder on the status code. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC8A8);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DC8A8(void) {
    u8 *blk = (u8 *)g_menuScreenBlock;   /* g_particleFxBlob + 0x100 */
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

/* Draw the memory-card system-message text box three times (shadow / shadow /
 * face) plus a flat backing rect, choosing the message string from the current
 * card status code (D_001F28A4). Returns 2. Wall: stack text-box struct built
 * with 128-bit packing + gp-relative cached colours. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC940);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DC940(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 str = 0x1abbe0;   /* default: pre-localized fallback buffer */
    Begin2dDrawBatch(0);
    if (D_001F28A4 >= 0) {
        if (D_001F28A4 < 3) {
            str = GetLocalizedString(0x2c8f);
        } else if (D_001F28A4 == 3) {
            str = GetLocalizedString(0x2c92);
        }
    }
    /* The original builds a stack text-box rect from obj->0x18..0x24 and draws
     * the string in three passes with a flat backing rect; reproduced at the
     * call level (exact rect packing omitted in this functional model). */
    (void)o;
    DrawFont2TextBox(o, 0x80000000, str, -1);
    DrawFont2TextBox(o, 0x80000000, str, -1);
    DrawFont2TextBox(o, 0x80f0f0f0, str, -1);
    End2dDrawBatch();
    return 2;
}
#endif

/* Seed obj->0x34 with a rotating entry from D_260570, indexed by the save
 * block's first word modulo 19. Returns 0.
 * Near-miss: the original delays the D_260570 %lo address add past the divu
 * trap to fill scheduling slots; our cc1 emits it earlier. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCBB0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DCBB0(MenuWidget *obj) {
    *(s32 *)((u8 *)obj + 0x34) = D_260570[(u32)g_playerProgress[0] % 19];
    return 0;
}
#endif

/* 30-entry wrap-around selector input. L1/R1 (0x900) blocks unless override.
 * Cancel(0x10) requests the parent screen. X(0x40) advances, square(0x20)
 * retreats, both modulo 30 (+0x40 field); a sound plays on change. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCBF0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DCBF0(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 pressed = g_padButtonsPressed[0];
    s32 prev;
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
    prev = *(s32 *)(o + 0x40);
    if (pressed & 0x40) {
        *(s32 *)(o + 0x40) = (prev + 1) % 0x1e;
    } else if (pressed & 0x20) {
        *(s32 *)(o + 0x40) = (prev + 0x1d) % 0x1e;
    }
    if (*(s32 *)(o + 0x40) != prev) {
        func_002DFFA0(1, 0x11);
    }
    return 0;
}
#endif

/* 12-entry wrap-around selector (+0x54 field). L1/R1 gates; cancel requests the
 * parent; up/down d-pad (0x2040 forward, 0x8020 back) cycle mod 12 with a
 * sound on each step. Returns 0/1/-1 like the sibling selectors. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCCC8);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DCCC8(MenuWidget *obj) {
    u8 *o = (u8 *)obj;
    s32 pressed = g_padButtonsPressed[0];
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
    if (pressed & 0x2040) {
        *(s32 *)(o + 0x54) = (*(s32 *)(o + 0x54) + 1) % 0xc;
        func_002DFFA0(1, 0x11);
    } else if (pressed & 0x8020) {
        *(s32 *)(o + 0x54) = (*(s32 *)(o + 0x54) + 0xb) % 0xc;
        func_002DFFA0(1, 0x11);
    }
    return 0;
}
#endif

/* Draw a single nav arrow (left or right depending on obj->0x38): a one-glyph
 * label plus a rotated HUD arrow sprite. Returns 2. Wall: float immediates +
 * store-heavy leaf. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCDC0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DCDC0(MenuWidget *obj) {
    extern u8 D_001ABC00, D_001ABC01, D_001ABBF8, D_001ABBF9;
    u8 *o = (u8 *)obj;
    u8 label[2];
    s32 h;
    s32 tex;
    Begin2dDrawBatch(0);
    if (*(s32 *)(o + 0x38) == 0) {
        label[0] = D_001ABC00;
        label[1] = D_001ABC01;
        DrawStringFont1(*(s32 *)(o + 0x20) - 0x18, *(s32 *)(o + 0x24) / 2 - 8,
                        0x80ffa888, label, -1);
        h = *(s32 *)(o + 0x24);
        tex = GetHudIconTex0(func_0028EDF0(0xe99d, 6));
        DrawHudSpriteRotated(0x43400000, (float)(h << 3), 0x43000000, 0x43800000,
                             0x40490fdb, 0x20, 0x10, tex);
    } else {
        label[0] = D_001ABBF8;
        label[1] = D_001ABBF9;
        DrawStringFont1(4, *(s32 *)(o + 0x24) / 2 - 8, 0x80ffa888, label, -1);
        h = *(s32 *)(o + 0x24);
        tex = GetHudIconTex0(func_0028EDF0(0xe99d, 6));
        DrawHudSpriteRotated(0x44200000, (float)(h << 3), 0x43000000, 0x43800000,
                             0, 0x20, 0x10, tex);
    }
    End2dDrawBatch();
    return 2;
}
#endif

/* Draw the centered level-info caption(s) for the currently-selected map row:
 * if the row's data index is -1, show the generic "no info" string; otherwise
 * show the two caption lines from the D_00262BA0 table. Returns 2. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCF58);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DCF58(MenuWidget *obj) {
    extern s32 D_00262BA0[];
    u8 *o = (u8 *)obj;
    u8 *focus = *(u8 **)((u8 *)g_pCurrentMenuScreen[0] + 0xE8);
    s32 idx = *(s32 *)(*(s32 *)(focus + 0x40) * 0xc + *(s32 *)(focus + 0x34) + 4);
    Begin2dDrawBatch(0);
    if (idx == -1) {
        DrawFont1CenteredLabel(*(s32 *)(o + 0x20) / 2, *(s32 *)(o + 0x24) / 2 - 8,
                               0x80ffa888, GetLocalizedString(0x2cfb), -1);
    } else {
        s32 w = *(s32 *)(o + 0x20);
        s32 h = *(s32 *)(o + 0x24);
        DrawFont1CenteredLabel(w / 2, h / 3 - 8, 0x80ffa888,
                               GetLocalizedString(D_00262BA0[idx * 3]), -1);
        DrawFont1CenteredLabel(w / 2, (h << 1) / 3 - 8, 0x80ffa888,
                               GetLocalizedString(D_00262BA0[idx * 3 + 1]), -1);
    }
    End2dDrawBatch();
    return 2;
}
#endif

/* Menu screen draw helper (Ghidra merges its boundary with a neighbour). Wall:
 * large draw loop + multi callee-save. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD0F8);

/* Draw a scrolling text-box list keyed off a switch over obj->0x50 state (two
 * layout modes). Wall: jump-table switch + 128-bit packed text-box struct +
 * func_002DF620 header fill. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD450);

/* Menu screen draw helper (Ghidra merges its boundary with a neighbour). Wall:
 * large draw loop + GS packets + multi callee-save. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD630);

/* Enter the galactic-map save-confirm screen: reset the text-table banks, grab
 * a fresh map slot into obj->0x48 (obj->0x4C=0), and prime the autosave/voice
 * subsystems; if a memory card is present and the GUI exists, show its panel.
 * Returns 0. Wall: 2-GPR callee-save + several leaf calls. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD7E8);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DD7E8(MenuWidget *obj) {
    extern u8 D_001A8C88;
    u8 *o = (u8 *)obj;
    func_002DF1B8(1);
    *(s32 *)(o + 0x4C) = 0;
    *(s32 *)(o + 0x48) = func_002DF368(0);
    func_002CA980();
    func_002888A8();
    if (D_001A8C88 != 0 && g_pGuiManager != 0) {
        func_0033A7A8((u8 *)g_pGuiManager + 0x3CEA0);
    }
    return 0;
}
#endif

/* Toggle the map-slot referenced by obj->0x48 via func_002DF428 and store the
 * result back. Returns 0. Wall: 2-GPR callee-save (8-byte-packed 0x10 frame). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD858);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DD858(MenuWidget *obj) {
    *(s32 *)((u8 *)obj + 0x48) = func_002DF428(*(s32 *)((u8 *)obj + 0x48));
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
 * Wall: large branch graph over save-system globals. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DDD30);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DDD30(MenuWidget *obj) {
    extern s32 D_001F28F8, D_0013953C, D_00139544, D_001F2924, D_001393E8;
    extern s32 D_0013954C, D_001393F0, D_0013955C, D_001A7424, D_001A7C10;
    extern s32 D_001A8C8C, D_00152C28, D_001C4F30, D_001F28FC;
    extern u8  D_001A8C88;
    extern s32 D_00139410[], D_00139528; extern s16 D_001393F8;
    extern u8  g_abLevelVisitedMarkers[]; extern u8 D_0025DB48[], D_00260D98[];
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
            if (g_playerProgress[0] < 1 && g_abLevelVisitedMarkers[0] == 0) {
                func_002F7328();
            } else {
                RequestGameStateChange(6, 2, 5, g_playerProgress[0], 0);
            }
            D_00152C28 = 0;
            return 0;
        }
        D_0013955C = 0;
        D_001A7424 |= 0x100;
        if (RequestGameStateChange(4, 1, 1, g_pCurrentMenuScreen[0], 0) == 0) return 0;
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
        RequestGameStateChange(4, 1, 1, g_pCurrentMenuScreen[0], 0);
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
            BuildSaveImage(0x1f29f0);
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

/* Splat mis-split fragment: only `addiu $sp,N; nop` runs, no prologue/jr — not
 * a real function body, cannot be expressed as C. Left as bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DECC8);

/* Cheat-code entry detector: while the player holds the L1+R1-ish combo
 * (held&0xf==6), record each fresh d-pad/face direction into a 0x14-entry
 * buffer; once full, scan the cheat table (D_00261BE8, stride 0x14) for a
 * matching 20-symbol sequence and apply the corresponding unlock.
 * Wall: large nested branch ladder + table scan. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DECE0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
void func_002DECE0(void) {
    extern u16 D_001B1FA8[];   /* entered-symbol ring buffer */
    extern s32 D_001ABD20;     /* number of symbols entered so far */
    extern u8  D_00261BE8[];   /* cheat code table (stride 0x14) */
    extern u8  g_inventoryNewFlag[];
    extern u8  D_0013956E[];
    extern u8  g_nPendingNanotechXp;
    extern s32 D_001A8CEC;
    s32 pressed = g_padButtonsPressed[0];
    if ((g_padButtonsHeld & 0xf) != 6) {
        D_001ABD20 = 0;
        return;
    }
    if ((pressed & 0xf0a0) == 0 || D_001ABD20 >= 0x14) {
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
        D_001B1FA8[D_001ABD20] = sym;
        D_001ABD20++;
    }
    if (D_001ABD20 == 0x14) {
        s32 found = -1;
        s32 code;
        for (code = 2; code < 0x93; code++) {
            s32 ci = code & 0xff;
            s32 ok = (D_001B1FA8[0] == D_00261BE8[ci]);
            if (ok) {
                s32 k;
                for (k = 1; k < 0x14; k++) {
                    if (D_001B1FA8[k] != D_00261BE8[(k * code + code) & 0xff]) { ok = 0; break; }
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
                D_0013956E[found] = 1;
            } else if ((u32)(found - 0x50) < 0x1e &&
                       *((u8 *)&g_nPendingNanotechXp + found) == 0) {
                *((u8 *)&g_nPendingNanotechXp + found) = 1;
                PlayGlobalSound(1, 0, 0);
                func_002B1880(0x1233, -1);
                D_001A8CEC = 1;
            }
        }
    }
}
#endif

/* Save-image WAD streaming/decompress state machine (obj->0x34 state 0..3),
 * driving DecompressWad on the save preview. Wall: the splat splice prepends an
 * `addiu $sp` mis-split prologue fragment, plus a large state machine + multi
 * callee-save. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DEEE8);

/* (Re)assign the 5 map cache slots' backing addresses and flags for a text
 * table swap. `param` selects whether the secondary banks are included: the
 * first uVar8 slots point into the load buffer, the next group into the
 * second-bank base, then alternating 0x4F00/0x11800-strided ranges; any unused
 * slots are zeroed. Wall: gp-relative slot array + branch-heavy banking. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF1B8);
#else
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

/* Acquire a free map cache slot (id != 0, not already in-use, free bit clear,
 * with `forceSet` selecting bit polarity), mark it in-use (bit 2), fill its
 * backing memory with the 0xDEADBEEF pattern sized by func_002DF500, and return
 * the slot id. Returns 0 if none free. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF368);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DF368(s32 forceSet) {
    s32 *ids   = (s32 *)(D_001B1E90 + 0x40);
    u32 *flags = (u32 *)(D_001B1E90 + 0x44);
    s32 i = 0;
    for (;;) {
        u32 polarity = forceSet ? (flags[i * 2] ^ 1) : flags[i * 2];
        u32 raw = flags[i * 2];
        if ((polarity & 1) == 0 && ids[i * 2] != 0 && (raw & 2) == 0) {
            flags[i * 2] = raw | 2;
            FillMemory32(ids[i * 2], 0xdeadbeef, func_002DF500(ids[i * 2]));
            return ids[i * 2];
        }
        i++;
        if (i > 4) return 0;
    }
}
#endif

/* Release the map cache slot whose id == `id`: if it is in-use (flag bit 1),
 * cancel any pending stream tied to it (bit 4 + the global file-load gate) and
 * clear its in-use bit. Returns 0. Slots are the interleaved id/flags pairs at
 * D_001B1E90+0x40/+0x44 (stride 8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF428);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DF428(s32 id) {
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

/* Look up the map slot with id == arg and return a packed display colour:
 * 0x4F000 if its flag bit 0 is set, else 0x11800; -1 if no slot matched.
 * Same address-base CSE near-miss as func_002DF560. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF500);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DF500(s32 id) {
    s32 *pflag = (s32 *)(D_001B1E90 + 0x44);
    s32 *pid   = (s32 *)(D_001B1E90 + 0x40);
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

/* Find the map slot whose id == arg among the 5 slots and OR 0x4 into its
 * flags; returns 0 on hit, 1 if no slot matched. The id/flags arrays are
 * interleaved (stride 8 bytes) starting at D_001B1E90+0x40/+0x44.
 * Near-miss: the original sets up the two pointers with two independent
 * lui/addiu pairs; our cc1 derives the second from the first (+4) via local
 * CSE of the address base (cannot be disabled with -fno-gcse). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF560);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DF560(s32 id) {
    s32 *pid   = (s32 *)(D_001B1E90 + 0x40);
    s32 *pflag = (s32 *)(D_001B1E90 + 0x44);
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

/* Mirror of func_002DF560 that CLEARS the 0x4 flag on the matching slot;
 * returns 0 on hit, 1 if no slot matched.
 * Same address-base CSE near-miss as func_002DF560. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF5B0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DF5B0(s32 id) {
    s32 *pid   = (s32 *)(D_001B1E90 + 0x40);
    s32 *pflag = (s32 *)(D_001B1E90 + 0x44);
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

/* Splat mis-split fragment: `addiu $sp` runs around a lone gp store with no
 * prologue/jr — not a real function body. Left as bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF600);

/* Fill a 16-bit sprite/quad header (dst) from a source rect (src): copies
 * src width(+0x24)/height(+0x20) into the size fields and their halves into
 * the centre fields, with fixed framing constants.
 * Near-miss: instruction scheduling + temp-register choice differ from the
 * original (store-heavy leaf schedule wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF620);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
void func_002DF620(s16 *dst, void *src) {
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

/* Splat mis-split fragment: a single `addiu $sp,0x60; nop` epilogue tail with no
 * prologue/jr — not a real function body. Left as bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF660);

/* Build an in-progress save image for slot `slot`: timestamp it from the CD
 * RTC, snapshot the current level WAD, build the image at `dst`, and arm the
 * 0x13-frame autosave countdown if idle. Wall: many leaf calls. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF668);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
void func_002DF668(void *dst, s16 slot) {
    extern u8  D_001A7360[];
    extern s32 D_00139554, D_00139528, D_00139544, D_00139548;
    extern s16 D_001393F8;
    sceCdReadClock((void *)D_001A7360);
    func_00131A98((void *)D_001A7360);
    func_00298A00();
    func_00297FA0(g_playerProgress[0] * 0x800 + 0x18d278);
    BuildSaveImage(dst);
    D_00139554 = (s32)dst;
    D_001393F8 = slot;
    D_00139528 = 0;
    if (D_00139544 < 0) {
        D_00139548 = 0;
        D_00139544 = 0x13;
    }
}
#endif

/* Build a fresh (new-game-style) save image into `dst` for slot `slot`:
 * close any pending stream, timestamp from the CD RTC, build the image, mark
 * the slot's saved-progress word 0, and arm the autosave countdown if idle. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF710);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
void func_002DF710(void *dst, s32 slot) {
    extern u8  D_001A7360[];
    extern s32 D_00139554, D_00139528, D_00139544, D_00139548;
    extern s16 D_001393F8;
    extern s32 D_00139410[];
    func_00299BF8();
    sceCdReadClock((void *)D_001A7360);
    func_00131A98((void *)D_001A7360);
    BuildSaveImage(dst);
    D_00139528 = 0;
    D_001393F8 = (s16)slot;
    *(s32 *)((u8 *)D_00139410 + slot * 0x1c) = 0;
    D_00139554 = (s32)dst;
    if (D_00139544 < 0) {
        D_00139548 = 0;
        D_00139544 = 0x13;
    }
}
#endif

/* In-RAM post-load progress rebuild: snapshots the PlayerStats currency block,
 * rebuilds weapon/skill-point/cheat/inventory tables, calls func_00299BF8, then
 * restores the saved currency and refreshes the save image. NOT the memcard I/O.
 * Wall: ~0x3760-byte stack frame, huge memcpy/SIMD snapshot loops, many named
 * global blocks — far beyond a faithful #else. Bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", RestorePlayerProgressState);

/* Configure the skill-points / unlockables summary screen labels & icon ids
 * from the three progress counters (skill points x2, and a misc count):
 * picks "complete" vs "partial" icon/string ids per threshold (15 / 30 / 10). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFE60);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
void func_002DFE60(void) {
    extern s16 D_0025EBEE, D_0025EBFA, D_0025EC06, D_0025EC12, D_0025EC10;
    extern s32 D_001AB2C8, D_001AB2BC, D_001AB2C0, D_001AB2C4;
    extern s32 D_001A7318;
    s32 sp1 = CountSkillPointsCompleted();
    s32 sp2 = CountSkillPointsCompleted();
    s32 misc = func_002B1D40();
    s32 miscDone = misc > 9;
    D_0025EBEE = (sp1 > 0xe) ? 3 : 2;
    D_0025EBFA = (sp2 > 0x1d) ? 3 : 2;
    D_0025EC06 = miscDone ? 10 : 2;
    D_0025EC12 = miscDone ? 3 : 2;
    D_001AB2C8 = miscDone ? 0x2cba : 0x2cbd;
    D_001AB2BC = (sp1 > 0xe) ? 0x2cb5 : 0x2cbb;
    D_001AB2C0 = (sp2 > 0x1d) ? 0x2cb6 : 0x2cbc;
    D_001AB2C4 = miscDone ? 0x2cb9 : 0x2cbd;
    if (D_001A7318 != 0) {
        D_0025EC10 = 0;
    }
}
#endif

/* Galactic-Map save/load progress accumulator: while the save page is active
 * (D_1F27C0+0x168 != 0), advance its byte counter (+0x16C) by `amount` and
 * forward `handle` to func_002A1138; returns that result, or 0 if inactive.
 * Near-miss: our cc1 picks a branch-likely (bnel) shape and moves `amount`
 * differently from the original's plain-beqz + delay-slot move. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFF68);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DFF68(s32 handle, s32 amount) {
    if (D_1F27C0.savePageActive == 0) {
        return 0;
    }
    D_1F27C0.savePageBytes += amount;
    return func_002A1138(handle);
}
#endif

/* Play a menu/system sound (id,arg) only when the menu-sound gate is enabled.
 * Near-miss: our cc1 reorders the $31 save out of the guard's delay slot. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFFA0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
void func_002DFFA0(s32 id, s32 arg) {
    if (D_1ABD48 != 0) {
        PlayGlobalSound(id, arg, 0);
    }
}
#endif

/* Always-ready gate: return 1. */
s32 func_002DFFC8(void) {
    return 1;
}

/* Splat mis-split fragment: two `addiu $sp,0x10; nop` runs, no prologue/jr —
 * not a real function body. Left as bare INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFFD0);

/* Register-coloring near-miss: is the current menu screen the given fixed
 * screen instance? (pointer compare lowered to xor + sltiu). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFFE0);
#else
    /* TODO(match): functional equivalent - not byte-exact. */
s32 func_002DFFE0(void) {
    extern u8 D_00259438[];
    return g_pCurrentMenuScreen[0] == (void *)D_00259438;
}
#endif

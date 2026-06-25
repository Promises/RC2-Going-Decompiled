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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5508);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D59D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5A08);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5C20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5CD8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5E90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5ED8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5FF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6058);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D61B8);

/* Draw-batch wrapper. */
s32 func_002D62F0(void) {
    func_0027CA28(0);
    func_0029D4B8();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6320);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6378);

/* Draw-batch wrapper. */
s32 func_002D6448(void) {
    func_0027CA28(0);
    func_0029D448();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6478);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6548);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", MenuScreenDoAction);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6A10);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D74D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D75A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D7AA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", GalacticMapScreenTick);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8228);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8758);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D89F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8D60);

/* return 0 stub. */
s32 func_002D8DE8(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8DF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8E90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", StreamTextTable);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D9108);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D96F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D9BE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D9D28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA2F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA320);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA450);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA4B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA708);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAA18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAAC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAE38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAF58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAFF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DB048);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DB6C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBA38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBB88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBC60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBEA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC0B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC340);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC4E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC680);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC800);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC840);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC870);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC908);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCB78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCBB8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCC90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCD88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCF20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD0C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD418);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD5F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD7B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD820);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD850);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DDCE8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DE110);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DE710);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DEC88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DECA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DEEA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF178);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF328);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF3E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF4C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF520);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF570);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF5C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF5E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF620);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF628);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF6D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", RestorePlayerProgressState);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFE20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFF28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFF60);

/* Always-ready gate: return 1. */
s32 func_002DFF88(void) {
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFF90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFFA0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5540);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5A10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5A48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5C08);

/* Reset the galactic-map upload sequence counter. Returns 0. */
s32 func_002D5C48(void) {
    D_25BA60[0] = 0;
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5C58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5D10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5EC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5F10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6028);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D60E8);

/* Feed this frame's newly-pressed buttons to the planet-select handler.
 * Near-miss: our cc1 fills the jal delay slot with the $31 reload that the
 * original keeps as a nop (delay-slot scheduling wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6158);
#else
s32 func_002D6158(void) {
    func_0029DD40(g_padButtonsPressed[0]);
    return 0;
}
#endif

/* Same planet-select input forward as func_002D6158 (sibling screen).
 * Same delay-slot scheduling wall near-miss. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6180);
#else
s32 func_002D6180(void) {
    func_0029DD40(g_padButtonsPressed[0]);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D61A8(void) {
    Begin2dDrawBatch(0);
    func_0029DD10();
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D61D8);

/* return 0 stub. */
s32 func_002D6240(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6248);

/* Draw-batch wrapper. */
s32 func_002D6380(void) {
    Begin2dDrawBatch(0);
    func_0029D958();
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D63B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6408);

/* Draw-batch wrapper. */
s32 func_002D64D8(void) {
    Begin2dDrawBatch(0);
    func_0029D8E8();
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6508);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6560);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D65D8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", MenuScreenDoAction);

/* Register-coloring near-miss (97%): list-entry screen-action forwarder. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6AA0);

/* Command-record wrapper: MenuScreenDoAction(cmd->op, cmd->arg, out). */
s32 func_002D6AD8(MenuCmd *cmd, void *out) {
    return MenuScreenDoAction(cmd->op, cmd->arg, out);
}

/* Command-record wrapper with a local out-flag. */
s32 func_002D6B00(MenuCmd *cmd) {
    s32 outFlag = 0;
    return MenuScreenDoAction(cmd->op, cmd->arg, &outFlag);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6B28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6E98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", SetGalacticMapFadeAlpha);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D75D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D7AE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", GalacticMapScreenTick);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8270);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D87C8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8E60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", RestorePrevTextTable);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", StreamTextTable);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9178);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9718);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9C18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9D60);

/* Set a draw record's mode field (+0x2 of the object at obj->0x34): in
 * Clank-solo (g_bPlayerMode==1) use 0, otherwise 3. Always returns 0.
 * Near-miss: register-coloring (original reuses $2 for the value + return;
 * our cc1 colours the value into $5). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA330);
#else
s32 func_002DA330(void *obj) {
    s16 *rec = *(s16 **)((u8 *)obj + 0x34);
    rec[1] = (g_bPlayerMode == 1) ? 0 : 3;
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA358);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA488);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA4F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA740);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DAA50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DAAF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DAE70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", InitMenuBgImageBuffers);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DB028);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DB080);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DB700);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", LoadMenuBgImagePair);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", UploadMenuBgImagePair);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DBC98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DBEE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC0F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC378);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC520);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC6B8);

/* If the confirm button (mask 0x40) was just pressed, request the menu screen
 * at D_25E660 as the next screen. Always returns 0. */
s32 func_002DC7D8(void) {
    if (g_padButtonsPressed[0] & 0x40) {
        g_pNextMenuScreen[0] = D_25E660;
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC800);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC838);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC878);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC8A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC940);

/* Seed obj->0x34 with a rotating entry from D_260570, indexed by the save
 * block's first word modulo 19. Returns 0.
 * Near-miss: the original delays the D_260570 %lo address add past the divu
 * trap to fill scheduling slots; our cc1 emits it earlier. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCBB0);
#else
s32 func_002DCBB0(void *obj) {
    *(s32 *)((u8 *)obj + 0x34) = D_260570[(u32)g_playerProgress[0] % 19];
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCBF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCCC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCDC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCF58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD0F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD450);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD630);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD7E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD858);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD888);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DDD30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DE168);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DE768);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DECC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DECE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DEEE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF1B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF368);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF428);

/* Look up the map slot with id == arg and return a packed display colour:
 * 0x4F000 if its flag bit 0 is set, else 0x11800; -1 if no slot matched.
 * Same address-base CSE near-miss as func_002DF560. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF500);
#else
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF600);

/* Fill a 16-bit sprite/quad header (dst) from a source rect (src): copies
 * src width(+0x24)/height(+0x20) into the size fields and their halves into
 * the centre fields, with fixed framing constants.
 * Near-miss: instruction scheduling + temp-register choice differ from the
 * original (store-heavy leaf schedule wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF620);
#else
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF660);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF668);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF710);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", RestorePlayerProgressState);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFE60);

/* Galactic-Map save/load progress accumulator: while the save page is active
 * (D_1F27C0+0x168 != 0), advance its byte counter (+0x16C) by `amount` and
 * forward `handle` to func_002A1138; returns that result, or 0 if inactive.
 * Near-miss: our cc1 picks a branch-likely (bnel) shape and moves `amount`
 * differently from the original's plain-beqz + delay-slot move. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFF68);
#else
s32 func_002DFF68(s32 handle, s32 amount) {
    if (D_1F27C0.savePageActive == 0) {
        return 0;
    }
    D_1F27C0.savePageBytes += amount;
    return func_002A1138(handle);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFFA0);

/* Always-ready gate: return 1. */
s32 func_002DFFC8(void) {
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFFD0);

/* Register-coloring near-miss: is the current menu screen the given fixed
 * screen instance? (pointer compare lowered to xor + sltiu). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFFE0);

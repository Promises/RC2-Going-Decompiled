#include "common.h"

/*
 * text/198FA0 — save-game + GUI-manager wrapper unit (pilot c-carve out of the
 * text/16E980 asm segment).
 *
 * The matcher builds THIS unit at -O2 -G8 -fno-gcse (see the per-unit GFLAG/
 * CC1EXTRA override in tools/ee/objdiff_build.sh / diff.sh / build.sh): the
 * gameplay-text TUs were built by a later SN cc1 without the load-PRE pass
 * (-fno-gcse reproduces it), and the g_guiInstance forwarding wrappers need
 * the cc1-small / assembler-absolute extern model below to reproduce the
 * original prologue scheduling (addiu $sp BEFORE the adjacent lui/lw pair).
 *
 * -G8 extern-sizing rules for this file (same as text/1907F0):
 *   - a complete extern object of size <= 8 bytes is placed in small data
 *     (gp-relative access);
 *   - an object the original reads with the ADJACENT lui/%lo "assembler
 *     macro" shape is declared as a small scalar/pointer PLUS a file-scope
 *     `.extern sym,16` override: cc1 then emits the one-insn symbolic macro
 *     (so it schedules like one insn, matching the SN codegen) and GNU as
 *     expands it absolutely.
 *
 * PRECISE FORM OF THE "reload artifact" WALL (measured 2026-06-12 on the
 * func_00299040 save-handler family): in the original, %gp_rel accesses to
 * gp-range symbols appear ONLY in branch/jr delay slots, while every
 * non-delay-slot access to the SAME symbol in the SAME function is the
 * absolute lui/$at macro. The SN toolchain materialises the 1-insn %gp_rel
 * form exactly when it fills a delay slot (a 2-insn macro cannot go there)
 * and the absolute form everywhere else. GNU as picks ONE form per symbol,
 * so every function whose original has a small-range global access in a
 * delay slot plus a non-slot access is unmatchable (func_00299040/
 * func_00299178/func_002991E8/func_00299238/func_002992B8/func_002992E8/
 * func_00299348/func_002993D8/func_00299478/func_002994B0/func_00299568/
 * func_002995E0/func_00299758/func_002997C8/func_002998D0/func_00299918/
 * func_00299960).
 */

/* Original cc1-small / assembler-absolute symbols (see header). */
__asm__(".extern g_guiInstance, 16");
__asm__(".extern g_bPalMode, 16");
__asm__(".extern g_loadedArmorVariant, 16");
__asm__(".extern g_loadedHeldItemModelId, 16");
__asm__(".extern g_levelDialogToc, 16");
__asm__(".extern g_nSaveLoadStatusCode, 16");
__asm__(".extern D_1A8C64, 16");

/* Singleton GUI-manager instance (0x3FB20-byte object allocated by
 * GuiManagerCreate; null until the GUI is up). Declared as a plain byte
 * pointer here because the wrappers below only ever forward `instance +
 * fixed-widget-offset` to the widget methods. */
extern char *g_guiInstance;
extern s32 g_bPalMode; /* PAL video-mode flag (cleared at boot in the USA build) */
extern volatile s32 g_loadedArmorVariant;     /* armor variant whose model is resident (-1 = none) */
extern volatile s32 g_loadedHeldItemModelId;  /* held-item model resident in the dialog scene (-1 = none) */
extern char g_levelDialogToc;                 /* level dialog/scene TOC block (byte-addressed here) */
extern s32 D_1A790C;                          /* small-data flag cleared by func_0029C418 */
extern s32 D_1A9A90;                          /* small-data GUI state, set to -1 by func_0029CA88 */

/* Save/load status pair at 0x1A7420: [0] = popup status code (enum selecting
 * the on-screen save/load message body), [1] = pending-action flag word. */
extern s32 g_nSaveLoadStatusCode[2];
extern s32 D_1A8C64;  /* GUI popup-busy gate (also read by the walled func_0029CCB8) */

/* Save/load engine context at 0x1393E0 (memory-card state machine scratch).
 * Field meanings recovered from the status writers below; declared as a struct
 * so the field offsets read naturally. */
typedef struct SaveLoadContext {
    s32 unk0[2];        /* 0x000 */
    s32 phase;          /* 0x008 - 2 while a card transaction is in flight */
    s32 unkC;           /* 0x00C */
    s32 busy;           /* 0x010 - transaction-active flag (= D_1393F0) */
    s32 unk14;          /* 0x014 */
    s16 slot;           /* 0x018 - active card slot (-1 = none) */
    s16 unk1A;          /* 0x01A */
    s32 unk1C[0x4A];    /* 0x01C */
    s32 unk144;         /* 0x144 */
    s32 unk148;         /* 0x148 */
    s32 unk14C[3];      /* 0x14C */
    s32 reqState;       /* 0x158 */
    s32 mode;           /* 0x15C */
    s32 unk160;         /* 0x160 */
    s32 result;         /* 0x164 - libmc result (-1 = pending) */
    s32 subResult;      /* 0x168 */
    s32 unk16C;         /* 0x16C - formatted/secondary path flag */
    s32 unk170[3];      /* 0x170 */
    s32 dirty;          /* 0x17C - save-pending flag */
} SaveLoadContext;
extern SaveLoadContext D_1393E0;

/* Alias view of D_1393E0.busy (0x1393F0): several status predicates read the
 * flag through its own symbol (the original folded the field offset into the
 * %hi/%lo pair, which splat splits as a distinct symbol). Declared >8 bytes so
 * cc1 emits the absolute address pair itself (matching the original temp-reg
 * choice) instead of the -G8 small-data form. */
extern s32 D_1393F0[4];
extern char D_1A9A60[];   /* "memory card library failed to initialise" debug string */
extern s32 McInit(void);
/* DebugPrintStub / func_0029CA98 declared value-returning: their callers below
 * propagate $v0, and a void tail call would be sibling-call optimised into a
 * plain `j` (the original uses jal + return). */
extern s32 DebugPrintStub(char *msg);
extern void func_0028E9A0(s32 arg);
extern s32 func_0029CA98(void);
extern s32 func_0033A8F0(void *widget, s32 arg);

/* One entry of a save-section descriptor table. The serialized layout each
 * entry contributes is an 8-byte header followed by `len` payload bytes,
 * padded up to a 4-byte boundary. The table is terminated by an entry whose
 * srcPtr is NULL. (Stride 0x10; tag/_pad carry per-section metadata used by
 * the (de)serializers, not by the size calculation.) */
typedef struct SaveSection {
    void *srcPtr;
    s32   len;
    s32   tag;
    s32   _pad;
} SaveSection;

/* GUI widget methods the g_guiInstance wrappers forward to (text/1A00F0
 * range). All are value-returning (which keeps this cc1 from sibling-call
 * optimising the forwarding tail into a plain `j`). */
extern s32 func_0034F1C0(void *widget);
extern s32 func_0034F240(void *widget);
extern s32 func_00349C00(void *widget);
extern s32 UpdatePopupMenu(void *popup, s32 arg);
extern s32 func_00348628(void *widget);
extern s32 func_00347550(void *widget);
extern s32 func_00347B88(void *widget, s32 arg);
extern s32 func_00346EF0(void *widget, s32 arg0, s32 arg1);
extern s32 func_0033A368(void *widget, s32 arg);
extern s32 func_0033A5D8(void *widget);
extern s32 func_0033AF70(void *widget, s32 arg);
extern s32 func_0033B428(void *widget);
extern s32 func_0033C958(void *widget);
extern s32 func_0033C588(void *widget, s32 arg);
extern s32 func_0033FEF8(void *widget);
extern s32 func_0033FAB0(void *widget, s32 arg);
extern s32 func_003462C0(void *widget);
extern s32 func_003453D0(void *widget, s32 arg);
extern s32 func_00342B30(void *widget);
extern s32 func_003427D0(void *widget, s32 arg);
extern s32 func_00344808(void *widget);
extern s32 func_003446B8(void *widget, s32 arg);
extern s32 func_0033F670(void *widget);
extern s32 func_0033F610(void *widget, s32 arg);
extern s32 func_0033E9B8(void *widget);
extern s32 func_0033E8B0(void *widget, s32 arg);
extern s32 func_0033D5D8(void *widget, s32 arg);
extern s32 func_0033D780(void *widget);
extern s32 func_0033DB60(void *widget, s32 arg);
extern s32 func_0033DC68(void *widget);
extern s32 func_0033E5E8(void *widget, s32 arg);
extern s32 func_0033E680(void *widget);
extern s32 func_0033CEE0(void *widget, s32 arg);
extern s32 func_0033D070(void *widget);
extern s32 func_0033D320(void *widget, s32 arg);
extern s32 func_0033D3C0(void *widget);
extern s32 func_0033F3F0(void *widget, s32 arg);
extern s32 func_0033F478(void *widget);
extern s32 func_0033E070(void *widget, s32 arg);
extern s32 func_0033E308(void *widget);
extern s32 func_0033EC80(void *widget, s32 arg);
extern s32 func_0033ED18(void *widget);
extern s32 func_003433D0(void *widget, s32 arg);
extern s32 func_00343668(void *widget);
extern s32 func_00342DF8(void *widget, s32 arg);
extern s32 func_00342E48(void *widget);
extern s32 UpdateBoltCounterHud(void);
extern s32 func_0028B0B0(void);
extern s32 func_0034DBD8(void *widget, s32 value);
extern s32 func_00340AA8(void *widget, s32 arg);
extern s32 func_00341160(void *widget);
extern s32 func_003395F0(void *widget, s32 arg0, s32 arg1, s32 arg2);
extern s32 func_00339678(void *widget, s32 arg0, s32 arg1);
extern s32 func_00338F18(void *widget, s32 arg0, s32 arg1);
extern s32 func_00336BC8(s32 palMode);
extern s32 func_0034DB68(void *widget, s32 arg);
extern s32 func_0034D828(void *widget, s32 arg0, s32 arg1);
extern s32 GuiScreenSetEventAndReveal(void *screen, s32 eventId);
extern s32 func_0034F200(void *widget);

/** Reset the save/load popup to status 3 (idle/none) and clear the context
 *  transaction-active + save-pending flags. */
void func_00299020(void) {
    g_nSaveLoadStatusCode[0] = 3;
    D_1393E0.dirty = 0;
    D_1393E0.busy = 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299040);

/** If no save/load action is pending (flag bit 0 clear), reset the popup
 *  status to 3 (idle). */
void func_00299128(void) {
    /* `(flags ^ 1) & 1` is bit0==0; spelled this way to reproduce the
     * original xori/andi evaluation order. */
    if ((g_nSaveLoadStatusCode[1] ^ 1) & 1) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

/** Mark the card transaction results as pending (-1) and show popup status 4. */
void func_00299150(void) {
    D_1393E0.result = -1;
    g_nSaveLoadStatusCode[0] = 4;
    D_1393E0.subResult = -1;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299178);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002991E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299238);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002992B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002992E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299348);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299398);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002993D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299478);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002994B0);

/** If a card transaction finished selecting (mode 2) with no result yet,
 *  force result 9 (cancelled) and show popup status 0xF. */
void func_00299528(void) {
    if (D_1393E0.mode == 2 && D_1393E0.result < 0) {
        D_1393E0.result = 9;
        D_1393E0.subResult = 0;
        g_nSaveLoadStatusCode[0] = 0xF;
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299568);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002995E0);

/** If no save/load confirmation is pending (flag bit 6 clear), reset the popup
 *  status to 3 (idle). First of four identical per-call-site stubs. */
void func_00299698(void) {
    if (!(g_nSaveLoadStatusCode[1] & 0x40)) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

/** Identical twin of func_00299698 (separate call-site stub). */
void func_002996C0(void) {
    if (!(g_nSaveLoadStatusCode[1] & 0x40)) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

/** If a card transaction is active (D_1393F0, the context busy flag), reset
 *  the popup status to 3 (idle). */
void func_002996E8(void) {
    if (D_1393F0[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

/** Identical twin of func_00299698 (separate call-site stub). */
void func_00299708(void) {
    if (!(g_nSaveLoadStatusCode[1] & 0x40)) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

/** Identical twin of func_00299698 (separate call-site stub). */
void func_00299730(void) {
    if (!(g_nSaveLoadStatusCode[1] & 0x40)) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299758);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002997C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002998D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299918);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299960);

/* Latched event flag at g_pSkyShellSpinRates+0xAC (0x1B19BC): an unrelated
 * bss word splat attributes to the spin-rate symbol; aliased via a gas
 * symbol equate because no symbol exists at that address (a symbol_addrs pin
 * + re-split would name it properly). */
extern s32 g_savePromptLatch;
__asm__("g_savePromptLatch = g_pSkyShellSpinRates+0xAC");

/** Consume the latched flag: read it, clear it, return whether it was set. */
s32 func_00299968(void) {
    s32 was = g_savePromptLatch;
    g_savePromptLatch = 0;
    return was != 0;
}

/** True when the save/load popup is showing status 2 (card-access prompt). */
s32 func_00299980(void) {
    return g_nSaveLoadStatusCode[0] == 2;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", BuildSaveGamePaths);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299B00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299B18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299BF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SaveLoadStateMachine);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", BuildSaveImage);

/** Game-level libmc bring-up: bind the memory-card RPC services (McInit) and
 *  log the failure string (compiled-out DebugPrintStub) when it errors.
 *  Falls off the end on success (the caller ignores the value; the fall-off
 *  keeps the original from materialising a return value). */
s32 InitMemCardLib(void) {
    if (McInit() != 0) {
        return DebugPrintStub(D_1A9A60);
    }
}

/* CalcSaveSectionsSize(table): return the number of bytes the section table
 * `table` serializes to. Layout is a leading 8-byte block, then for every
 * non-terminator entry an 8-byte header plus its (4-byte-aligned) payload, then
 * an 8-byte trailing terminator. Used to size the memory-card read/write
 * buffers for the two global save-section tables. */
s32 CalcSaveSectionsSize(SaveSection *table) {
    s32 size = 8;
    if (table->srcPtr != 0) {
        do {
            size += 8;
            size += table->len;
            table++;
            size = (size + 3) & -4;
        } while (table->srcPtr != 0);
    }
    return size + 8;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BCA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BD48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SerializeSaveSections);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BEA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", DeserializeSaveSections);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", CommitProgressCheckpoint);

/** Reset the dialog-scene model bookkeeping: mark the armor-variant and
 *  held-item models unloaded (-1), clear the two cached entries inside the
 *  level dialog TOC block (+0x13C8/+0x13E0), and clear the D_1A790C flag.
 *  (The TOC entries are written through a volatile pointer purely to pin the
 *  original interleaved store order — there is no concurrency here.) */
void func_0029C418(void) {
    volatile s32 *p;
    g_loadedArmorVariant = -1;
    p = (volatile s32 *)(&g_levelDialogToc + 0x13B0);
    p[6] = -1;
    g_loadedHeldItemModelId = -1;
    p[12] = -1;
    D_1A790C = 0;
}

/* func_0029C448(a,b): forward two args to the widget at g_guiInstance+0x36F28
 * (method func_00339398) — same shape as the matched func_0029DAD0. Best
 * attempt 97.67% (every insn/reloc exact): the residue is a pure $v0/$v1
 * register-coloring swap — the original (later SN) cc1 colours the gui load
 * $v0 and the arg copy $v1 HERE while colouring the identical shape the other
 * way in func_0029DAD0/func_0029DB10; no source shape found that flips it
 * (local-gui, copy-first, bare-return variants all probed). Same coloring
 * wall as func_002911F0. Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C448);

/** Forward `arg` to the HUD render object at g_guiInstance+0x376C8 (method
 *  func_0034DB68). */
s32 func_0029C488(s32 arg) {
    if (g_guiInstance != 0) {
        return func_0034DB68(g_guiInstance + 0x376C8, arg);
    }
}

/** Forward two args to the HUD render object at g_guiInstance+0x376C8 (method
 *  func_0034D828). */
s32 func_0029C4C0(s32 arg0, s32 arg1) {
    if (g_guiInstance != 0) {
        return func_0034D828(g_guiInstance + 0x376C8, arg0, arg1);
    }
}

/* func_0029C500: 8 bytes of dead inter-function fill (two `addiu $sp,+0x10`
 * epilogue orphans), not compiler-reachable C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C500);

/** Post a GUI page/event id to the active screen object at
 *  g_guiInstance+0x37ED0 via GuiScreenSetEventAndReveal (no-op while the GUI
 *  is down). Used by the orb-heal path and the level transition. */
s32 PostGuiScreenEvent(s32 eventId) {
    if (g_guiInstance != 0) {
        return GuiScreenSetEventAndReveal(g_guiInstance + 0x37ED0, eventId);
    }
}

/** Register the GUI pump callback pair at g_guiInstance+0x3F9F0/+0x3F9F4
 *  (b into +0x3F9F4 first, then a into +0x3F9F0 — the volatile stores pin the
 *  original descending store order; func_0029CA98/func_0029CD18 invoke these
 *  slots via jalr). */
void func_0029C548(s32 a, s32 b) {
    char *gui = g_guiInstance;
    if (gui != 0) {
        *(volatile s32 *)(gui + 0x3F9F4) = b;
        *(volatile s32 *)(gui + 0x3F9F0) = a;
    }
}

/** True when either of the GUI pump callback slots at
 *  g_guiInstance+0x3F9F0/+0x3F9F4 is registered. */
s32 func_0029C570(void) {
    char *gui = g_guiInstance;
    if (gui != 0) {
        if (*(s32 *)(gui + 0x3F9F0) != 0 || *(s32 *)(gui + 0x3F9F4) != 0) {
            return 1;
        }
    }
    return 0;
}

/* func_0029C5B0(a,b,idx): if the GUI is up and idx<2, store the (a,b) pair
 * into the slot table at g_guiInstance+0x3FA18+idx*8 and return 1; else 0.
 * Best attempt 87.6% under -G8 -fno-gcse (base-0x38000 split + volatile store
 * order reproduce exactly): the residue is pure register COLORING — the
 * original (later SN) cc1 copies BOTH args to $t0/$t1 and duplicates the slot
 * base ($a0 + a gratuitous $v1 copy) while the pinned cc1 copies only `a` and
 * keeps one base. Same coloring wall as func_002911F0. Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C5B0);

/* func_0029C600(idx): clear the slot pair at g_guiInstance+0x3FA18+idx*8 (the
 * counterpart of func_0029C5B0). UNMATCHABLE in this unit: it reads
 * g_guiInstance via %gp_rel($gp) while every other function here reads it via
 * the adjacent absolute lui/lw pair — the same symbol, both ways, inside one
 * original TU (the proven reload-artifact wall). The unit-wide extern model
 * can only express one side per symbol (g_guiInstance is modeled absolute,
 * favouring the ~60 wrappers), so this stays asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C600);

/* func_0029C638: 16 bytes of dead inter-function fill (`li $v0,0` + epilogue
 * orphan, no jr) — not compiler-reachable C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C638);

/** Forward to the widget at g_guiInstance+0x36F28 (method func_0034F200). */
s32 func_0029C648(void) {
    if (g_guiInstance != 0) {
        return func_0034F200(g_guiInstance + 0x36F28);
    }
}

/* func_0029C678 (and C700/C818/C8F0/CA98/CC48/CD18): 2+-callee-save functions
 * walled by the 8-byte-packed callee-save layout of the later SN cc1 (ours
 * reserves 16 bytes per save) - the wall characterized in the 1907F0 round. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C678);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C700);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C818);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C8F0);

/** Invalidate the D_1A9A90 GUI state (set to -1). */
void func_0029CA88(void) {
    D_1A9A90 = -1;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CA98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CC48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CCB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CD18);

/* func_0029CF08: empty jr-ra stub - installed by GuiManagerCreate as a default no-op GUI callback. */
void func_0029CF08(void) {
}

/*
 * ---- g_guiInstance forwarding-wrapper family ----------------------------
 * Each wrapper forwards to one method of a GUI widget embedded at a fixed
 * offset inside the GuiManager instance, guarded on the GUI being up. Two
 * recurring shapes:
 *   - update/draw style: `if (gui) return method(gui + WIDGET);` — falls off
 *     the end (no return value materialised) when the GUI is down, exactly
 *     like the original;
 *   - query/handler style: `if (!gui) return 0; return method(gui + WIDGET,
 *     ...);` — returns 0 when the GUI is down.
 * The widget offsets pair the wrappers up per widget (one no-arg method +
 * one arg-taking method per offset).
 */

/** Forward to the widget at g_guiInstance+0x36F28 (method func_0034F1C0). */
s32 func_0029CF10(void) {
    if (g_guiInstance != 0) {
        return func_0034F1C0(g_guiInstance + 0x36F28);
    }
}

/** Forward to the widget at g_guiInstance+0x36F28 (method func_0034F240). */
s32 func_0029CF40(void) {
    if (g_guiInstance != 0) {
        return func_0034F240(g_guiInstance + 0x36F28);
    }
}

/** Forward to the popup-menu widget at g_guiInstance+0x39160 (method func_00349C00). */
s32 func_0029CF70(void) {
    if (g_guiInstance != 0) {
        return func_00349C00(g_guiInstance + 0x39160);
    }
}

/** Run the popup-menu widget at g_guiInstance+0x39160 (UpdatePopupMenu, returns
 *  the selected item index); 0 when the GUI is down. */
s32 func_0029CFA0(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return UpdatePopupMenu(g_guiInstance + 0x39160, arg);
}

/** Forward to the widget at g_guiInstance+0x39620 (method func_00348628). */
s32 func_0029CFE0(void) {
    if (g_guiInstance != 0) {
        return func_00348628(g_guiInstance + 0x39620);
    }
}

/** Forward to the widget at g_guiInstance+0x39AF0 (method func_00347550). */
s32 func_0029D010(void) {
    if (g_guiInstance != 0) {
        return func_00347550(g_guiInstance + 0x39AF0);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x39620 (method func_00347B88);
 *  0 when the GUI is down. */
s32 func_0029D040(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00347B88(g_guiInstance + 0x39620, arg);
}

/** Forward two args to the widget at g_guiInstance+0x39AF0 (method
 *  func_00346EF0); 0 when the GUI is down. */
s32 func_0029D080(s32 arg0, s32 arg1) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00346EF0(g_guiInstance + 0x39AF0, arg0, arg1);
}

/** Forward `arg` to the widget at g_guiInstance+0x3B418 (method func_0033A368);
 *  0 when the GUI is down. */
s32 func_0029D0C8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033A368(g_guiInstance + 0x3B418, arg);
}

/** Forward to the widget at g_guiInstance+0x3B418 (method func_0033A5D8). */
s32 func_0029D108(void) {
    if (g_guiInstance != 0) {
        return func_0033A5D8(g_guiInstance + 0x3B418);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3AF80 (method func_0033AF70);
 *  0 when the GUI is down. */
s32 func_0029D138(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033AF70(g_guiInstance + 0x3AF80, arg);
}

/** Forward to the widget at g_guiInstance+0x3AF80 (method func_0033B428). */
s32 func_0029D178(void) {
    if (g_guiInstance != 0) {
        return func_0033B428(g_guiInstance + 0x3AF80);
    }
}

/** Forward to the widget at g_guiInstance+0x3B6F8 (method func_0033C958). */
s32 func_0029D1A8(void) {
    if (g_guiInstance != 0) {
        return func_0033C958(g_guiInstance + 0x3B6F8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3B6F8 (method func_0033C588);
 *  0 when the GUI is down. */
s32 func_0029D1D8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033C588(g_guiInstance + 0x3B6F8, arg);
}

/** Forward to the widget at g_guiInstance+0x3BA58 (method func_0033FEF8). */
s32 func_0029D218(void) {
    if (g_guiInstance != 0) {
        return func_0033FEF8(g_guiInstance + 0x3BA58);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3BA58 (method func_0033FAB0);
 *  0 when the GUI is down. */
s32 func_0029D248(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033FAB0(g_guiInstance + 0x3BA58, arg);
}

/** Forward to the widget at g_guiInstance+0x3A000 (method func_003462C0). */
s32 func_0029D288(void) {
    if (g_guiInstance != 0) {
        return func_003462C0(g_guiInstance + 0x3A000);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3A000 (method func_003453D0);
 *  0 when the GUI is down. */
s32 func_0029D2B8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003453D0(g_guiInstance + 0x3A000, arg);
}

/** Forward to the widget at g_guiInstance+0x3C160 (method func_00342B30). */
s32 func_0029D2F8(void) {
    if (g_guiInstance != 0) {
        return func_00342B30(g_guiInstance + 0x3C160);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C160 (method func_003427D0);
 *  0 when the GUI is down. */
s32 func_0029D328(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003427D0(g_guiInstance + 0x3C160, arg);
}

/** Forward to the widget at g_guiInstance+0x3C160 (method func_00342B30) —
 *  byte-identical twin of func_0029D2F8 (two call sites got their own stubs). */
s32 func_0029D368(void) {
    if (g_guiInstance != 0) {
        return func_00342B30(g_guiInstance + 0x3C160);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C160 (method func_003427D0)
 *  — byte-identical twin of func_0029D328; 0 when the GUI is down. */
s32 func_0029D398(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003427D0(g_guiInstance + 0x3C160, arg);
}

/** Forward to the widget at g_guiInstance+0x3C480 (method func_00344808). */
s32 func_0029D3D8(void) {
    if (g_guiInstance != 0) {
        return func_00344808(g_guiInstance + 0x3C480);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C480 (method func_003446B8);
 *  0 when the GUI is down. */
s32 func_0029D408(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003446B8(g_guiInstance + 0x3C480, arg);
}

/** Forward to the widget at g_guiInstance+0x3C7E8 (method func_0033F670). */
s32 func_0029D448(void) {
    if (g_guiInstance != 0) {
        return func_0033F670(g_guiInstance + 0x3C7E8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C7E8 (method func_0033F610);
 *  0 when the GUI is down. */
s32 func_0029D478(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033F610(g_guiInstance + 0x3C7E8, arg);
}

/** Forward to the widget at g_guiInstance+0x3D1D8 (method func_0033E9B8). */
s32 func_0029D4B8(void) {
    if (g_guiInstance != 0) {
        return func_0033E9B8(g_guiInstance + 0x3D1D8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3D1D8 (method func_0033E8B0);
 *  0 when the GUI is down. */
s32 func_0029D4E8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033E8B0(g_guiInstance + 0x3D1D8, arg);
}

/** Forward `arg` to the widget at g_guiInstance+0x3D4B8 (method func_0033D5D8);
 *  0 when the GUI is down. */
s32 func_0029D528(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033D5D8(g_guiInstance + 0x3D4B8, arg);
}

/** Forward to the widget at g_guiInstance+0x3D4B8 (method func_0033D780). */
s32 func_0029D568(void) {
    if (g_guiInstance != 0) {
        return func_0033D780(g_guiInstance + 0x3D4B8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3D798 (method func_0033DB60);
 *  0 when the GUI is down. */
s32 func_0029D598(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033DB60(g_guiInstance + 0x3D798, arg);
}

/** Forward to the widget at g_guiInstance+0x3D798 (method func_0033DC68). */
s32 func_0029D5D8(void) {
    if (g_guiInstance != 0) {
        return func_0033DC68(g_guiInstance + 0x3D798);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3DDE8 (method func_0033E5E8);
 *  0 when the GUI is down. */
s32 func_0029D608(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033E5E8(g_guiInstance + 0x3DDE8, arg);
}

/** Forward to the widget at g_guiInstance+0x3DDE8 (method func_0033E680). */
s32 func_0029D648(void) {
    if (g_guiInstance != 0) {
        return func_0033E680(g_guiInstance + 0x3DDE8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E0C8 (method func_0033CEE0);
 *  0 when the GUI is down. */
s32 func_0029D678(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033CEE0(g_guiInstance + 0x3E0C8, arg);
}

/** Forward to the widget at g_guiInstance+0x3E0C8 (method func_0033D070). */
s32 func_0029D6B8(void) {
    if (g_guiInstance != 0) {
        return func_0033D070(g_guiInstance + 0x3E0C8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E3A8 (method func_0033D320);
 *  0 when the GUI is down. */
s32 func_0029D6E8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033D320(g_guiInstance + 0x3E3A8, arg);
}

/** Forward to the widget at g_guiInstance+0x3E3A8 (method func_0033D3C0). */
s32 func_0029D728(void) {
    if (g_guiInstance != 0) {
        return func_0033D3C0(g_guiInstance + 0x3E3A8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3F3F0 (method func_0033F3F0);
 *  0 when the GUI is down. */
s32 func_0029D758(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033F3F0(g_guiInstance + 0x3F3F0, arg);
}

/** Forward to the widget at g_guiInstance+0x3F3F0 (method func_0033F478). */
s32 func_0029D798(void) {
    if (g_guiInstance != 0) {
        return func_0033F478(g_guiInstance + 0x3F3F0);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3DA78 (method func_0033E070);
 *  0 when the GUI is down. */
s32 func_0029D7C8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033E070(g_guiInstance + 0x3DA78, arg);
}

/** Forward to the widget at g_guiInstance+0x3DA78 (method func_0033E308). */
s32 func_0029D808(void) {
    if (g_guiInstance != 0) {
        return func_0033E308(g_guiInstance + 0x3DA78);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3EB50 (method func_0033EC80);
 *  0 when the GUI is down. */
s32 func_0029D838(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033EC80(g_guiInstance + 0x3EB50, arg);
}

/** Forward to the widget at g_guiInstance+0x3EB50 (method func_0033ED18). */
s32 func_0029D878(void) {
    if (g_guiInstance != 0) {
        return func_0033ED18(g_guiInstance + 0x3EB50);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E760 (method func_003433D0);
 *  0 when the GUI is down. */
s32 func_0029D8A8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003433D0(g_guiInstance + 0x3E760, arg);
}

/** Forward to the widget at g_guiInstance+0x3E760 (method func_00343668). */
s32 func_0029D8E8(void) {
    if (g_guiInstance != 0) {
        return func_00343668(g_guiInstance + 0x3E760);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E9C8 (method func_00342DF8);
 *  0 when the GUI is down. */
s32 func_0029D918(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00342DF8(g_guiInstance + 0x3E9C8, arg);
}

/** Forward to the widget at g_guiInstance+0x3E9C8 (method func_00342E48). */
s32 func_0029D958(void) {
    if (g_guiInstance != 0) {
        return func_00342E48(g_guiInstance + 0x3E9C8);
    }
}

/* func_0029D988: 2 orphan unreachable words (`addu $2,$3,$2; nop`) — a dead
 * code fragment between the wrapper bodies, not compiler-reachable C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D988);

/** Tick the on-screen bolt counter (UpdateBoltCounterHud) only when the HUD
 *  render context (g_guiInstance) is present. */
s32 TickBoltCounterHud(void) {
    if (g_guiInstance != 0) {
        return UpdateBoltCounterHud();
    }
}

/** Run the func_0028B0B0 HUD updater only when the GUI is present. */
s32 func_0029D9B8(void) {
    if (g_guiInstance != 0) {
        return func_0028B0B0();
    }
}

/** Flush a HUD display value into the render object at g_guiInstance+0x376C8
 *  (used to push g_nBoltCounterDisplayed to the renderer). */
s32 FlushHudDisplayValue(s32 value) {
    if (g_guiInstance != 0) {
        return func_0034DBD8(g_guiInstance + 0x376C8, value);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3A4C0 (method func_00340AA8);
 *  0 when the GUI is down. */
s32 func_0029DA18(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00340AA8(g_guiInstance + 0x3A4C0, arg);
}

/** Forward to the widget at g_guiInstance+0x3A4C0 (method func_00341160). */
s32 func_0029DA58(void) {
    if (g_guiInstance != 0) {
        return func_00341160(g_guiInstance + 0x3A4C0);
    }
}

/** Forward three args to the widget at g_guiInstance+0x36F28 (method
 *  func_003395F0). */
s32 func_0029DA88(s32 arg0, s32 arg1, s32 arg2) {
    if (g_guiInstance != 0) {
        return func_003395F0(g_guiInstance + 0x36F28, arg0, arg1, arg2);
    }
}

/** Forward two args to the widget at g_guiInstance+0x36F28 (method
 *  func_00339678). */
s32 func_0029DAD0(s32 arg0, s32 arg1) {
    if (g_guiInstance != 0) {
        return func_00339678(g_guiInstance + 0x36F28, arg0, arg1);
    }
}

/** Forward two args to the widget at g_guiInstance+0x36F28 (method
 *  func_00338F18). */
s32 func_0029DB10(s32 arg0, s32 arg1) {
    if (g_guiInstance != 0) {
        return func_00338F18(g_guiInstance + 0x36F28, arg0, arg1);
    }
}

/* func_0029DB50: empty jr-ra stub - installed by GuiManagerCreate as a default no-op GUI callback. */
void func_0029DB50(void) {
}

/** Forward the PAL-mode flag (as 0/1) to func_00336BC8. */
s32 func_0029DB58(void) {
    return func_00336BC8(g_bPalMode != 0);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", GuiManagerCreate);

/** If the GUI is up and the popup-busy gate (D_1A8C64) is clear, pause the
 *  game world (func_0028E9A0(1)) and run the GUI pump (func_0029CA98),
 *  propagating its result. */
s32 func_0029DC70(void) {
    if (g_guiInstance != 0) {
        if (D_1A8C64 == 0) {
            func_0028E9A0(1);
            return func_0029CA98();
        }
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DCB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD08);

/** Forward `arg` to the widget at g_guiInstance+0x3CEA0 (method func_0033A8F0);
 *  0 when the GUI is down. */
s32 func_0029DD40(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033A8F0(g_guiInstance + 0x3CEA0, arg);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD80);

/* func_0029DD90: toSPR DMA-kick helper (writes SADR/QWC/MADR at 0x1000D400,
 * CHCR 0x100) - called 4x from the sky piece stagers. Split off func_0029DD80
 * (4 orphan unreachable words). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DDB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DDE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", EnableDmacChannels);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", FlushPendingTexUploads);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E090);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E1D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", DecompressWad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E5D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E5F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", UpdateLevelObjectiveStates);

/* EvaluateProgressCondition(cond, arg): 12-case switch (0=always, 1=level
 * available, 2=item owned, 3=item NEW, 4=objective active, 5=objective
 * complete, 6=dialog state byte, 7=call arg as predicate fn, 8=platinum
 * bolt, 9=cinematic bit, 10=map predicate on g_mapCurrentLevel). RE-PROBED
 * under the unit recipe 2026-06-12: the jump table itself NOW REPRODUCES
 * exactly (12 entries incl. explicit case 11, sltiu 0xC, original block
 * order with case 9 before case 8) - the old "prologue scheduling" wall is
 * gone. Best 95.06%; two residues: (a) case 9's bit test - the pinned cc1
 * lowers `(w & (1 << (n & 0x1F))) != 0` to srav/andi-extract while the
 * original (later SN cc1) keeps sllv/and/sltu, no source shape found;
 * (b) the base object emits its jump table as a section-local .rodata label
 * while the split target references named jtbl_0026CA70_text (objdiff reloc
 * identity - needs splat rodata migration for the carved units). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", EvaluateProgressCondition);

/* GatherActiveObjectives(outIds, outMask, outVals, wantValues): walks the
 * 0x28-stride objective list (head pointer parked at the unnamed bss word
 * g_pSkyShellSpinRates+0xC8 = 0x1B19D8), emits visible objective ids/values,
 * sets completion bits in *outMask (bit 31 = all non-hidden complete),
 * returns the count. RE-PROBED 2026-06-12, best 87.59% - structure and all
 * field accesses line up; residues are later-cc1 traits: (a) register
 * coloring swaps rec/state ($t1/$t0 vs our $t0/$t1, plus the lui-temp),
 * (b) the original does NOT hoist the loop-invariant 0x31B9 constant (ours
 * preloads it to a register; theirs rematerialises it in a beql delay slot),
 * (c) the original copies the outIds cursor and reloads rec->state in the
 * value-select block where ours CSEs. Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", GatherActiveObjectives);

/* func_0029EA90 (and EAC8/EB08/EB38 below): 0/1 predicates over globals
 * (g_miscExtras + D_1397E0 bit 0x8000000 here). WALLED (probed 2026-06-12):
 * the boolean tail `if (test) return 1; return 0;` is scc-converted by the
 * pinned cc1 into `sltu $2,$0,$2` on EVERY source shape probed (if-chain,
 * nested guards, v=1/v=0 flag variable), while the original (later SN cc1)
 * emits the branch + per-path constant materialisation (`bnez; addiu $2,1 /
 * daddu $2,0`). Same class: func_0029EC70. Predicates returning 0/1/2
 * (func_0029EB68/func_0029EBF8) are NOT walled - scc cannot synthesise 2. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EA90);

/* 0/1 predicate (dialog flags D_1395C1/D_1A7B0D/D_1A7B14) - scc-tail wall,
 * see func_0029EA90. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EAC8);

/* 0/1 predicate (D_1397C4 streaming state + D_1395D5 flag) - scc-tail wall,
 * see func_0029EA90. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB08);

/* 0/1 predicate (D_1A7BDD/D_1A7B10 dialog flags) - scc-tail wall, see
 * func_0029EA90. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB38);

/* func_0029EB68: cinematic skip-prompt arbiter (posts HUD message 0x1F/0x20
 * into D_257502, returns 0/1/2 on the D_1395B8[0x1D] / cinematic word +0x5C
 * pair). RE-PROBED 2026-06-12, best 76.57%: the original keeps both %hi
 * halves live in registers and re-materialises the D_1395B8 base via addiu
 * before each reload, while the pinned cc1 folds the +0x1D element address
 * into the lui/lbu pair (pointer-local shapes scored worse). Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB68);

/* func_0029EBF8: dialog-skip arbiter twin of func_0029EB68 on the D_1A7BDD/
 * D_1A7B10 byte pair (posts 0x28/0x29 into D_2579B2, returns 0/1/2). WALLED
 * (probed 2026-06-12): the original (later SN cc1) copies the first byte to
 * $a0 and re-narrows it with a redundant `andi 0xFF` at each re-test; the
 * pinned cc1 tracks the lbu value range and deletes the narrowing on every
 * shape probed (u8 locals, s32 locals + (u8) casts, a|b joint test). Best
 * 52.77%. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EBF8);

/* 0/1 predicate over the cinematic-flag words +0x90/+0x98 - its final
 * `if (bit) return 1; return 0;` block hits the scc-tail wall, see
 * func_0029EA90. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EC70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029ECE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029FDF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SpawnMoby);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", InitMobyFromClass);

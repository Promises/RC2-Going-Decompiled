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

/* Two card-error gate words in the save-prompt small-data block adjacent to
 * D_1A8C64; read by the save/load status handlers below to choose between the
 * "card removed / fatal" and "retry" message paths.
 *   D_1A8C88 : set when the active card slot reported a hard/unrecoverable error
 *   D_1A8C8C : set when a card-removal abort is in progress
 * (Widths follow the original lw opcodes; declared for the TARGET_NATIVE #else
 * arms only — they emit no code so the matching build is unaffected.) */
extern s32 D_1A8C88;
extern s32 D_1A8C8C;

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

/* Progress/dialog flag globals read by the 0x29EAxx predicate family below.
 * Widths follow the original load opcodes (lbu = u8, lw = s32). Declared here
 * for the TARGET_NATIVE #else arms; the #ifndef arms stay INCLUDE_ASM (these
 * declarations emit no code, so the matching build is unaffected). */
extern u8  g_miscExtras;             /* 0x1A7A12 misc-extras gate byte (nonzero blocks) */
extern s32 D_1397E0;                 /* 0x1397E0 status word; bit 0x8000000 = busy */
extern u8  D_1395C1;                 /* 0x1395C1 dialog-active flag */
extern u8  D_1A7B0D;                 /* 0x1A7B0D dialog flag */
extern u8  D_1A7B14;                 /* 0x1A7B14 dialog flag */
extern s32 D_1397C4;                 /* 0x1397C4 streaming state (sign = idle) */
extern u8  D_1395D5;                 /* 0x1395D5 streaming-busy flag */
extern u8  D_1A7BDD;                 /* 0x1A7BDD dialog flag */
extern u8  D_1A7B10;                 /* 0x1A7B10 dialog flag */
extern s32 g_cinematicUnlockedFlags; /* 0x139768 cinematic bitfield (read at +0x90/+0x98) */

/* Gating globals + widget method for the func_0029CCB8 popup-poll wrapper. */
extern s32 D_1A9A88;                 /* 0x1A9A88 GUI-active gate (gp small-data) */
extern s32 D_1A9A8C;                 /* 0x1A9A8C GUI-ready gate (gp small-data) */
extern s32 g_nNanotechBonusHealTimer;/* 0x189FFC; +0x4 is a separate s16 sub-state */
extern s32 func_0033B720(void *widget); /* GUI popup-poll method */

/* Progress-condition flag arrays read by EvaluateProgressCondition's 12 cases
 * (widths follow the original lbu/lw opcodes). Declared for the TARGET_NATIVE
 * #else arm; emit no code so the matching build is unaffected. */
extern u8  g_abLevelAvailableFlags[]; /* 0x1A7BD0 per-level available flag (case 1) */
extern u8  g_inventoryOwned[];        /* 0x1A7B00 per-item have-flag (case 2) */
extern u8  g_inventoryNewFlag[];      /* 0x1A7B38 per-item newly-acquired flag (case 3) */
extern u8  D_1395B8[];                /* 0x1395B8 dialog/story flag byte-array (case 6) */
extern u8  g_platinumBoltFlags[];     /* 0x19B278 per-platinum-bolt collected flag (case 9) */
extern s32 g_mapCurrentLevel;         /* 0x1C5150 current map level id (case 10) */
extern s32 func_002FCEA0(s32 level);  /* map-progress predicate (case 10 callee) */

/* One per-level objective record (stride 0x28) walked by
 * UpdateLevelObjectiveStates / GatherActiveObjectives. The list is a flat array
 * terminated by an entry whose `id` is 0. */
typedef struct LevelObjective {
    s16 id;             /* 0x00 objective id; 0 terminates the list */
    s16 cond1Sel;       /* 0x02 first EvaluateProgressCondition selector (cond) */
    s32 cond1Arg;       /* 0x04 first EvaluateProgressCondition operand (arg) */
    s16 cond2Sel;       /* 0x08 second EvaluateProgressCondition selector (cond) */
    s16 _pad0A;         /* 0x0A */
    s32 cond2Arg;       /* 0x0C second EvaluateProgressCondition operand (arg) */
    u16 flags;          /* 0x10 bit 0x2 = hidden-when-complete, 0x4 = level-gated */
    u8  _pad12[0xA];    /* 0x12 */
    s32 (*tickFn)(s32); /* 0x1C optional per-tick callback */
    s32 tickArg;        /* 0x20 argument passed to tickFn */
    s16 state;          /* 0x24 0 = inactive, 1 = active, 2 = complete */
    s16 tickResult;     /* 0x26 last tickFn return value */
} LevelObjective;

/* Per-current-level objective-list head table, indexed by MapCache.currentLevel.
 * Each entry is a LevelObjective* (NULL = no objectives for that level).
 * (Kept as the splat auto-name D_258B20 / EU D_258BA0 so the carved-unit reloc
 * resolves without a global re-split rename.) */
extern LevelObjective *D_258B20[]; /* 0x258B20 objective-list head table */

/* MapCache.currentLevel lives at g_mapVertexData + 0x230 (see symbol comment). */
extern u8 g_mapVertexData[]; /* 0x1C4F20 MapCache base (byte-addressed here) */

/* Objective-scan scratch parked in the rain-heightmap bss block at +0x34:
 *   +0x34 outstanding = count of active-but-not-yet-satisfied objectives
 *                       (0 => the current level's objectives are all met)
 *   +0x38 head        = cached head pointer for the current level's list */
typedef struct ObjectiveScan {
    s32             outstanding; /* +0x34 (struct offset 0x0) */
    LevelObjective *head;        /* +0x38 (struct offset 0x4) */
} ObjectiveScan;
extern u8 g_pRainHeightmap[]; /* 0x1B19A0 (byte-addressed for the +0x34 scratch) */

extern u8 g_levelVisitedMarkers[]; /* 0x1A7BF0 per-level visited byte markers */

/* Per-weapon upgrade record: EvaluateProgressCondition cases 4/5 index a stride-
 * 0x10 table based at 0x139A28 and read the upgrade-level field at +0xC (that
 * field's symbol is g_weaponUpgradeLevel = 0x139A34). */
typedef struct WeaponUpgradeRecord {
    s32 _pad0[3];   /* 0x0 */
    s32 upgradeLevel; /* 0xC - level; 0 = not started, >=2 = fully upgraded */
} WeaponUpgradeRecord;
extern WeaponUpgradeRecord D_139A28[]; /* 0x139A28 per-weapon upgrade table (stride 0x10) */

/* One entry of a save-section descriptor table. The serialized layout each
 * entry contributes is an 8-byte header { tag, len } followed by `len` payload
 * bytes, padded up to a 4-byte boundary. The table is terminated by an entry
 * whose srcPtr is NULL. (Stride 0x10.) The +0xC field is scratch the SERIALIZER
 * ignores (rounds the stride) but the DESERIALIZER writes per entry to record
 * how that section reconciled against the loaded image:
 *   0  = never matched (no image section with this tag)
 *   1  = matched, image section length == descriptor length
 *  -1  = matched, image section shorter than descriptor (partial restore)
 *  -2  = matched, image section longer than descriptor (truncated to fit) */
typedef struct SaveSection {
    void *srcPtr;      /* 0x0 payload source; NULL terminates the table */
    s32   len;         /* 0x4 payload byte length (summed by CalcSaveSectionsSize) */
    s32   tag;         /* 0x8 section identity tag (matched against image headers) */
    s32   matchResult; /* 0xC deserializer reconcile result (see above) */
} SaveSection;

/* Size pin (byte-neutral). CalcSaveSectionsSize confirms the entry stride is
 * 0x10 (asm: `addiu a0,a0,0x10` per iteration), reads srcPtr@0x0 and len@0x4,
 * and stops at the first entry whose srcPtr is NULL. The tester builds a valid
 * table by laying out 0x10-byte entries terminated by a {NULL,...} entry. */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(SaveSection) == 0x10, "SaveSection stride 0x10");
_Static_assert(__builtin_offsetof(SaveSection, len) == 0x4, "len");
_Static_assert(__builtin_offsetof(SaveSection, matchResult) == 0xC, "matchResult");
#endif

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

/* func_00299040 (and the 0x299xxx save-handler family below: func_00299178/
 * func_002991E8/func_00299238/func_002992B8/func_002992E8/func_00299348/
 * func_002993D8/func_00299478/func_002994B0/func_00299568/func_002995E0/
 * func_00299758/func_002997C8/func_002998D0/func_00299918): memory-card
 * save/load status handlers. WALLED by the reload-artifact described in the
 * file header — each reads a small-range save-context global both via the
 * 1-insn %gp_rel form (in a branch/jr delay slot) AND via the absolute lui/$at
 * macro elsewhere in the same function; GNU as picks one form per symbol, so
 * the unit-wide extern model cannot reproduce both. Left as asm. */
/** Save/load top-level status arbiter. Always consumes the pending-flag's 0x2
 *  and 0x4 bits first. Then: if no save is pending (areaTable/dirty +0x17C == 0)
 *  -> status 3. Otherwise route the original flags: bit 0x80 (or secondary-path
 *  +0x16C) -> status 0x15 (consume 0x80, set 0x40); bit 0x100 -> status 0x14
 *  (consume 0x100, set 0x40); else if a card transaction is active (busy != 0)
 *  clear the pending flag and, on a hard/abort card error (D_1A8C88/D_1A8C8C)
 *  show status 3, else show status 2 (set bit 0x1); else (idle) bit 0x200 ->
 *  status 0x19 (consume 0x200).
 *  (Walled for matching by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299040);
#else
void func_00299040(void) {
    s32 flags = g_nSaveLoadStatusCode[1];
    s32 cleared = flags & ~0x6;
    g_nSaveLoadStatusCode[1] = cleared;
    if (D_1393E0.dirty == 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if ((flags & 0x80) || D_1393E0.unk16C != 0) {
        g_nSaveLoadStatusCode[0] = 0x15;
        g_nSaveLoadStatusCode[1] = (cleared & ~0x80) | 0x40;
        return;
    }
    if (flags & 0x100) {
        g_nSaveLoadStatusCode[0] = 0x14;
        g_nSaveLoadStatusCode[1] = (cleared ^ 0x100) | 0x40;
        return;
    }
    if (D_1393E0.busy != 0) {
        D_1393E0.dirty = 0;
        if (D_1A8C88 != 0 || D_1A8C8C != 0) {
            g_nSaveLoadStatusCode[0] = 3;
        } else {
            g_nSaveLoadStatusCode[0] = 2;
            g_nSaveLoadStatusCode[1] = cleared | 0x1;
        }
    } else if (flags & 0x200) {
        g_nSaveLoadStatusCode[0] = 0x19;
        g_nSaveLoadStatusCode[1] = cleared ^ 0x200;
    }
}
#endif

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

/** Save-prompt step: clear the pending-flag's 0x20 bit, then if a card
 *  transaction is in flight (phase==2) translate the libmc busy-result into a
 *  popup status. busy 0 -> status 9 (busy); busy -1 -> ack (busy=0) + status 9;
 *  busy -2 -> status 5. No transaction (phase!=2) or other busy values: no-op.
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299178);
#else
void func_00299178(void) {
    g_nSaveLoadStatusCode[1] &= ~0x20;
    if (D_1393E0.phase == 2) {
        s32 busy = D_1393E0.busy;
        if (busy == 0) {
            g_nSaveLoadStatusCode[0] = 9;
        } else if (busy == -1) {
            D_1393E0.busy = 0;
            g_nSaveLoadStatusCode[0] = 9;
        } else if (busy == -2) {
            g_nSaveLoadStatusCode[0] = 5;
        }
    }
}
#endif

/** Card-removal step: if no abort is in flight (busy != -2) show status 3
 *  (idle). Otherwise, if a hard card error is latched (D_1A8C8C set) show
 *  status 6; if the pending-flag's 0x2 bit is set show status 6.
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002991E8);
#else
void func_002991E8(void) {
    s32 cardErr = D_1A8C8C;
    if (D_1393F0[0] != -2) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if (cardErr != 0) {
        g_nSaveLoadStatusCode[0] = 6;
    } else if (g_nSaveLoadStatusCode[1] & 0x2) {
        g_nSaveLoadStatusCode[0] = 6;
    }
}
#endif

/** Card-abort step: if no abort is in flight (busy != -2) show status 3 (idle).
 *  Otherwise dispatch on the pending-flag word: bit 0x20 -> if a hard card
 *  error is latched (D_1A8C88) clear it and show status 0x17, else show status
 *  5; bit 0x8 -> clear it and show status 7.
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299238);
#else
void func_00299238(void) {
    s32 flags = g_nSaveLoadStatusCode[1];
    if (D_1393F0[0] != -2) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if (flags & 0x20) {
        /* The 0x20 bit is cleared on BOTH the error and no-error paths: the
         * original writes it back in the branch delay slot before testing
         * D_1A8C88, so the clear happens regardless of the error result. */
        g_nSaveLoadStatusCode[1] = flags ^ 0x20;
        if (D_1A8C88 != 0) {
            g_nSaveLoadStatusCode[0] = 0x17;
        } else {
            g_nSaveLoadStatusCode[0] = 5;
        }
    } else if (flags & 0x8) {
        g_nSaveLoadStatusCode[1] = flags ^ 0x8;
        g_nSaveLoadStatusCode[0] = 7;
    }
}
#endif

/** Result-timeout step: always clear the transaction busy flag; then if the
 *  libmc result is still pending (result < 0) force result 3 + subResult 0.
 *  Always show status 8 (formatting/working).
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002992B8);
#else
void func_002992B8(void) {
    D_1393E0.busy = 0;
    if (D_1393E0.result < 0) {
        D_1393E0.subResult = 0;
        D_1393E0.result = 3;
    }
    g_nSaveLoadStatusCode[0] = 8;
}
#endif

/** Format-confirm step: when a format request (mode 2) is still pending
 *  (result < 0): if the secondary/format path flag (unk16C) is set, show
 *  status 0x11 and set pending-flag bit 0x40; otherwise show status 0xE.
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002992E8);
#else
void func_002992E8(void) {
    if (D_1393E0.mode == 2 && D_1393E0.result < 0) {
        if (D_1393E0.unk16C != 0) {
            g_nSaveLoadStatusCode[0] = 0x11;
            g_nSaveLoadStatusCode[1] |= 0x40;
        } else {
            g_nSaveLoadStatusCode[0] = 0xE;
        }
    }
}
#endif

/** Load-prompt step: if no transaction is active (busy==0) dispatch on the
 *  pending-flag word: bits 0x6 -> status 0xA; else bit 0x200 -> status 0x19.
 *  If a transaction is active (busy != 0) show status 3 (idle).
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299348);
#else
void func_00299348(void) {
    s32 flags = g_nSaveLoadStatusCode[1];
    if (D_1393F0[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if (flags & 0x6) {
        g_nSaveLoadStatusCode[0] = 0xA;
    } else if (flags & 0x200) {
        g_nSaveLoadStatusCode[0] = 0x19;
    }
}
#endif

/** If a card transaction finished selecting (mode 2) with no result yet
 *  (result < 0), force result 7 and show popup status 0xB. Mirror of
 *  func_00299528 with result 7 / status 0xB. */
void func_00299398(void) {
    if (D_1393E0.mode == 2 && D_1393E0.result < 0) {
        D_1393E0.result = 7;
        D_1393E0.subResult = 0;
        g_nSaveLoadStatusCode[0] = 0xB;
    }
}

/** Save/load step (only while a card transaction finished selecting, mode 2,
 *  result < 0): secondary-path flag (+0x16C) set -> status 0xC; else a deep
 *  abort (busy < -1) -> status 3; else if the active slot is the sentinel -2,
 *  pick status 0x13 (or 0xC when the unkC + 0x20 byte total reaches 0x1DB);
 *  else (slot >= -1) -> status 0x10.
 *  (Walled for matching by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002993D8);
#else
void func_002993D8(void) {
    if (D_1393E0.mode != 2 || D_1393E0.result >= 0) {
        return;
    }
    if (D_1393E0.unk16C != 0) {
        g_nSaveLoadStatusCode[0] = 0xC;
        return;
    }
    if (D_1393E0.busy < -1) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if (D_1393E0.slot == -2) {
        if (D_1393E0.unkC + D_1393E0.unk1C[1] < 0x1DB) {
            g_nSaveLoadStatusCode[0] = 0x13;
        } else {
            g_nSaveLoadStatusCode[0] = 0xC;
        }
    } else if (D_1393E0.slot >= -1) {
        g_nSaveLoadStatusCode[0] = 0x10;
    }
}
#endif

/** Save/load status predicate: while a card transaction is active
 *  (D_1393F0/busy != 0) show popup status 3 (idle/none); otherwise, if the
 *  pending-flag's 0x2 bit is set, show status 0xD. (Walled for matching by the
 *  reload-artifact named in the file header — kept as asm + a typed #else.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299478);
#else
void func_00299478(void) {
    if (D_1393F0[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
    } else if (g_nSaveLoadStatusCode[1] & 0x2) {
        g_nSaveLoadStatusCode[0] = 0xD;
    }
}
#endif

/** Save/load status dispatch: while a card transaction is active
 *  (D_1393F0/busy != 0) show status 3. Otherwise route on the pending-flag word:
 *  bit 0x20 -> clear it, then if a hard card error is latched (D_1A8C88) show
 *  status 0x18 else status 0xC; bit 0x10 -> clear it and show status 0xE.
 *  (Walled for matching by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002994B0);
#else
void func_002994B0(void) {
    s32 flags;
    if (D_1393F0[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x20) {
        g_nSaveLoadStatusCode[1] = flags ^ 0x20;   /* consume bit 0x20 */
        if (D_1A8C88 != 0) {
            g_nSaveLoadStatusCode[0] = 0x18;
        } else {
            g_nSaveLoadStatusCode[0] = 0xC;
        }
    } else if (flags & 0x10) {
        g_nSaveLoadStatusCode[1] = flags ^ 0x10;   /* consume bit 0x10 */
        g_nSaveLoadStatusCode[0] = 0xE;
    }
}
#endif

/** If a card transaction finished selecting (mode 2) with no result yet,
 *  force result 9 (cancelled) and show popup status 0xF. */
void func_00299528(void) {
    if (D_1393E0.mode == 2 && D_1393E0.result < 0) {
        D_1393E0.result = 9;
        D_1393E0.subResult = 0;
        g_nSaveLoadStatusCode[0] = 0xF;
    }
}

/** Save/load step (only while a card transaction finished selecting, mode 2,
 *  with no result yet, result < 0): if the secondary-path flag (+0x16C) is set,
 *  show status 0x12 and set the pending-flag's 0x40 bit; otherwise commit
 *  result 7 / subResult 0, show status 0x16, and reset the transaction
 *  (unk148 + slot cleared).
 *  (Walled for matching by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299568);
#else
void func_00299568(void) {
    if (D_1393E0.mode != 2 || D_1393E0.result >= 0) {
        return;
    }
    if (D_1393E0.unk16C != 0) {
        g_nSaveLoadStatusCode[0] = 0x12;
        g_nSaveLoadStatusCode[1] |= 0x40;
    } else {
        D_1393E0.result = 7;
        D_1393E0.subResult = 0;
        g_nSaveLoadStatusCode[0] = 0x16;
        D_1393E0.unk148 = 0;
        D_1393E0.slot = 0;
    }
}
#endif

/** Save/load status: first consume the pending-flag's 0x4 and 0x2 bits if set.
 *  Then route on the remaining flags: bit 0x80 -> status 0x15 (consume 0x80, set
 *  0x40); bit 0x100 -> status 0x14 (consume 0x100, set 0x40); else if a card
 *  transaction is active (g_areaTable/busy != 0) -> status 3; else if the
 *  save-pending flag (+0x17C) is set -> status 1.
 *  (Walled for matching by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002995E0);
#else
void func_002995E0(void) {
    s32 flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x4) {
        g_nSaveLoadStatusCode[1] = flags & ~0x4;
    }
    flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x2) {
        g_nSaveLoadStatusCode[1] = flags & ~0x2;
    }
    flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x80) {
        g_nSaveLoadStatusCode[0] = 0x15;
        g_nSaveLoadStatusCode[1] = (flags ^ 0x80) | 0x40;
    } else if (flags & 0x100) {
        g_nSaveLoadStatusCode[0] = 0x14;
        g_nSaveLoadStatusCode[1] = (flags ^ 0x100) | 0x40;
    } else if (D_1393E0.busy != 0) {
        g_nSaveLoadStatusCode[0] = 3;
    } else if (D_1393E0.dirty != 0) {
        g_nSaveLoadStatusCode[0] = 1;
    }
}
#endif

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

/** Save/load step (only while a card transaction finished selecting, mode 2,
 *  result < 0): if the secondary-path flag (+0x16C) or the retry slot (+0x24)
 *  is set, clear the save-pending flag (+0x17C), show status 0x15 and set the
 *  pending-flag's 0x440 bits; otherwise mark save-pending (+0x17C = 1) and show
 *  status 1.
 *  (Walled for matching by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299758);
#else
void func_00299758(void) {
    if (D_1393E0.mode != 2 || D_1393E0.result >= 0) {
        return;
    }
    if (D_1393E0.unk16C != 0 || D_1393E0.unk1C[2] != 0) {
        g_nSaveLoadStatusCode[0] = 0x15;
        D_1393E0.dirty = 0;
        g_nSaveLoadStatusCode[1] |= 0x440;
    } else {
        g_nSaveLoadStatusCode[0] = 1;
        D_1393E0.dirty = 1;
    }
}
#endif

/** Save-complete handler (only while a card transaction finished, mode 2,
 *  result < 0). Nothing pending (secondary-path +0x16C == 0 and not busy) ->
 *  mark save-pending (+0x17C = 1) and show status 1. Secondary path
 *  (+0x16C != 0) -> clear the pending flag, show status 0x15, then drive a
 *  game-state change to the save/load screen (RequestGameStateChange(4,
 *  g_nGameState == 0 ? 1 : 2, 1, 0, 0)); if already in the level-exit state
 *  (g_nGameState == 6) flush the pending cinematic (func_00289798); finally show
 *  status 2. Else (busy, no secondary path) -> consume the pending-flag's 0x40
 *  bit, set bit 0x1, show status 2.
 *  (Walled for matching by the reload-artifact named in the file header.) */
extern s32 g_nGameState;
extern void RequestGameStateChange(s32 newState, s32 argA, s32 argB, s32 argC, s32 argD);
extern void func_00289798(void);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002997C8);
#else
void func_002997C8(void) {
    if (D_1393E0.mode != 2 || D_1393E0.result >= 0) {
        return;
    }
    if (D_1393E0.unk16C == 0 && D_1393E0.busy == 0) {
        D_1393E0.dirty = 1;
        g_nSaveLoadStatusCode[0] = 1;
        return;
    }
    D_1393E0.dirty = 0;
    if (D_1393E0.unk16C != 0) {
        g_nSaveLoadStatusCode[0] = 0x15;
        g_nSaveLoadStatusCode[1] = 0x40;
        if (g_nGameState == 0) {
            RequestGameStateChange(4, 1, 1, 0, 0);
        } else {
            RequestGameStateChange(4, 2, 1, 0, 0);
        }
        if (g_nGameState == 6) {
            func_00289798();
        }
        g_nSaveLoadStatusCode[0] = 2;
    } else {
        g_nSaveLoadStatusCode[1] = (g_nSaveLoadStatusCode[1] & ~0x40) | 0x1;
        g_nSaveLoadStatusCode[0] = 2;
    }
}
#endif

/** Save/load status: while a card-removal abort is NOT in flight
 *  (D_1393F0/busy != -2) show status 3; otherwise, if the pending-flag's 0x20
 *  bit is set, consume it and show status 5.
 *  (Walled for matching by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002998D0);
#else
void func_002998D0(void) {
    s32 flags;
    if (D_1393F0[0] != -2) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x20) {
        g_nSaveLoadStatusCode[1] = flags ^ 0x20;   /* consume bit 0x20 */
        g_nSaveLoadStatusCode[0] = 5;
    }
}
#endif

/** Save/load status: while a card transaction is active (D_1393F0/busy != 0)
 *  show status 3; otherwise, if the pending-flag's 0x20 bit is set, consume it
 *  and show status 0xC.
 *  (Walled for matching by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299918);
#else
void func_00299918(void) {
    s32 flags;
    if (D_1393F0[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x20) {
        g_nSaveLoadStatusCode[1] = flags ^ 0x20;   /* consume bit 0x20 */
        g_nSaveLoadStatusCode[0] = 0xC;
    }
}
#endif

/* func_00299960: 2-insn leaf `return g_savePromptLatch;` (the latched flag at
 * 0x1B19BC, also read by func_00299968). UNMATCHABLE: the original reads it via
 * a single %gp_rel($gp) load in the jr delay slot, but under the unit's -G0
 * model cc1 emits the absolute lui/lw pair for a normal extern (the same
 * symbol is accessed absolutely in func_00299968) — the reload-artifact wall
 * named in the header. Left as asm. */
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

/* BuildSaveGamePaths(cfg): build the per-slot memory-card path strings into the
 * bss path buffers (g_saveDirTemplate / g_saveIconSysPath / g_saveStaticIcoPath /
 * g_saveFileFmt and the interior D_1A7950 / D_1A79A8 slots). Reads the region
 * code at cfg+0x12 (0x45 'E' / 0x50 'P' / 0x4B 'K') to patch g_saveDirTemplate[2],
 * copies device-id bytes out of cfg into the dir template at fixed offsets (the
 * three byte-copy loops at 3..6 / 8..0xA / 0xB..0xC), then makes seven
 * func_00115AC0(dest, src=&g_saveDirTemplate, 0xD) string-formatter calls to
 * stamp the dir name into each full path.
 *
 * SEEDABLE but NOT YET MATCHED/oracle'd: the target buffers are .bss (all zero in
 * the ROM image), so the strings are assembled purely at runtime from cfg — a
 * faithful body is writable. The blocker is func_00115AC0's SEMANTICS: it is the
 * SDK string/path formatter (cod/015180 region, called (dst,src,0xD); its body is
 * a SIMD zero-byte scan, sprintf/strncpy/path-join family). Its per-function .s
 * exists (cod/015180/func_00115AC0.s) so it could be linked as a cmp shared callee
 * - but matching the C without first pinning its exact semantics would be a guess.
 * Left as asm pending that identity. The TARGET_NATIVE #else below is faithful
 * coverage — it calls func_00115AC0 by its traced (dst,src,len) signature. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", BuildSaveGamePaths);
#else
extern u8   g_saveDirTemplate[];     /* mutable save-dir name template (region + serial) */
extern u8   D_1A7950[];              /* save path buffer */
extern u8   D_1A79A8[];              /* save path buffer (+0x14 = a second buffer) */
extern u8   g_saveIconSysPath[];
extern u8   g_saveStaticIcoPath[];
extern u8   g_saveFileFmt[];
extern void func_00115AC0(void *dst, const void *src, s32 len); /* path-join/copy (UNCONFIRMED) */

void BuildSaveGamePaths(void *rec) {
    u8 *r    = (u8 *)rec;
    u8 *tmpl = g_saveDirTemplate;
    s32 i;

    /* region-code remap into template[2]: 'P' -> 'I', 'E'/'K' unchanged */
    if (r[0x12] == 0x45) {          /* 'E' */
        tmpl[2] = 0x45;
    } else if (r[0x12] == 0x50) {   /* 'P' */
        tmpl[2] = 0x49;             /* 'I' */
    } else if (r[0x12] == 0x4B) {   /* 'K' */
        tmpl[2] = 0x4B;
    }

    for (i = 3; i < 7; i++) {
        tmpl[i] = r[i + 0xD];
    }
    for (i = 8; i < 0xB; i++) {
        tmpl[i] = r[i + 0xD];
    }
    for (i = 0xB; i < 0xD; i++) {
        tmpl[i] = r[i + 0xE];
    }

    func_00115AC0(D_1A7950, tmpl, 0xD);
    func_00115AC0(g_saveIconSysPath, tmpl, 0xD);
    func_00115AC0(g_saveStaticIcoPath, tmpl, 0xD);
    func_00115AC0(D_1A79A8, tmpl, 0xD);
    func_00115AC0(D_1A79A8 + 0x14, tmpl, 0xD);
    func_00115AC0(g_saveFileFmt, tmpl, 0xD);
}
#endif

/* func_00299B00: 0x14 bytes of dead inter-function fill (`daddu $2,$0,$0` /
 * `daddu $2,$3,$0` / `addiu $sp,0x40` epilogue orphans, no jr) — not
 * compiler-reachable C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299B00);

/* func_00299B18 / func_00299BF8: save-buffer setup helpers (multi callee-save).
 * 8-byte-packed callee-save frame wall, see func_0029C678. Left as asm. */
/* forward decls: CalcSaveSectionsSize/DeserializeSaveSections are defined below;
 * the two section-table globals + the error string aren't declared elsewhere. */
extern s32  CalcSaveSectionsSize(SaveSection *table);
extern s32  DeserializeSaveSections(void *image, s32 slotMul, SaveSection *table);
extern void func_0029C418(void);
extern SaveSection g_saveSectionTableGlobal[];
extern SaveSection g_saveSectionTableArea[];
extern char D_1A99A8[];   /* "save size mismatch" log string */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299B18);
#else
/* func_00299B18(image): restore a save image. First validates the image's two
 * leading size words against the live section-table sizes (global then area);
 * on a mismatch it logs g_saveSizeMismatchMsg and bails without touching the
 * tables. Otherwise it skips the size header and deserializes the global block
 * (slot 0) followed by all 0x1C area slots, advancing the image cursor by each
 * block's size, then finalizes via func_0029C418. */
void func_00299B18(u8 *image) {
    s32 sizeGlobal = CalcSaveSectionsSize(g_saveSectionTableGlobal);
    s32 sizeArea = CalcSaveSectionsSize(g_saveSectionTableArea);
    s32 i;

    if (*(s32 *)(image + 0) != sizeGlobal || *(s32 *)(image + 4) != sizeArea) {
        DebugPrintStub(D_1A99A8);
        return;
    }

    image += 8;
    DeserializeSaveSections(image, 0, g_saveSectionTableGlobal);
    image += sizeGlobal;
    for (i = 0; i < 0x1C; i++) {
        DeserializeSaveSections(image, i, g_saveSectionTableArea);
        image += sizeArea;
    }
    func_0029C418();
}
#endif

/* func_00299BF8 callees/globals (declared for the TARGET_NATIVE #else only). */
extern u8   g_areaTable[];    /* 0x1393E0 per-area record table (aliases D_1393E0) */
extern u8   g_discToc[];      /* 0x14B540 master disc asset directory (byte-addressed) */
extern s32  g_playerProgress; /* 0x1A79F8 first word of the persistent save block */
extern void func_00289398(s32 sectorByteOffset, void *outBuf); /* load save file -> *outBuf */
extern void PumpDialogVoiceSystem(s32 blocking);
extern void StartFileLoadPumpingVoice(void *buf, s32 lba, s32 size);
extern void *func_00283460(void *dst, const void *src, s32 nbytes); /* memcpy — early decl (func_00299BF8 uses it before the later decl; pre-existing native-build error) */

/** func_00299BF8 — load-side save-image restore orchestrator (counterpart of
 *  BuildSaveImage). Reads the current save file off the disc TOC, restores the
 *  serialized game state through func_00299B18 while preserving the 0x28-byte
 *  g_bPalMode video-config block across it, then clears the "resident asset" and
 *  progress latches so the newly-loaded state takes effect:
 *    - load the save file (func_00289398) at g_discToc[+0x344]<<11, service the
 *      voice pump, then kick the file load (StartFileLoadPumpingVoice) with the
 *      level/global WAD base LBAs from g_discToc[+0x340]/[+0x32C]/[+0x344];
 *    - snapshot g_bPalMode -> scratch, run func_00299B18 on the image body at
 *      buf + buf[0x10], restore g_bPalMode <- scratch;
 *    - reset g_playerProgress head word, clear the save-context dirty flag
 *      (g_areaTable+0x17C) and force the active card slot (+0x18) to -1 if held,
 *      invalidate the resident armor/held-item model ids and the dialog-scene
 *      latches at g_levelDialogToc+0x13C8/+0x13E0, and drop the 0x200 pending bit
 *      in g_nSaveLoadStatusCode[1].
 *  Not isolatable as a cmp (its callees func_00299B18/func_00283460 live in-unit,
 *  so the runner's T->c_ rename would drag the whole save subsystem); the #else
 *  is native-checked + trace-verified. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299BF8);
#else
void func_00299BF8(void) {
    u8   scratch[0x28];
    void *buf;

    func_00289398(*(s32 *)(g_discToc + 0x344) << 11, &buf);
    PumpDialogVoiceSystem(1);
    StartFileLoadPumpingVoice(buf,
                              *(s32 *)(g_discToc + 0x340) + *(s32 *)(g_discToc + 0x32C),
                              *(s32 *)(g_discToc + 0x344));

    /* preserve the g_bPalMode block across the destructive image restore */
    func_00283460(scratch, &g_bPalMode, 0x28);
    func_00299B18((u8 *)buf + *(s32 *)((u8 *)buf + 0x10));
    func_00283460(&g_bPalMode, scratch, 0x28);

    g_playerProgress = 0;
    if (*(s32 *)(g_areaTable + 0x17C) != 0) {
        *(s32 *)(g_areaTable + 0x17C) = 0;
    }
    if (*(s16 *)(g_areaTable + 0x18) >= 0) {
        *(s16 *)(g_areaTable + 0x18) = -1;
    }
    *(s32 *)(&g_levelDialogToc + 0x13C8) = -1;      /* 0x13B0 + 0x18 */
    g_loadedArmorVariant = -1;
    g_nSaveLoadStatusCode[1] &= ~0x200;
    g_loadedHeldItemModelId = -1;
    *(s32 *)(&g_levelDialogToc + 0x13E0) = -1;      /* 0x13B0 + 0x30 */
}
#endif

/* SaveLoadStateMachine: the 0x1EA0-byte memory-card transaction state machine
 * (the largest function in the unit). Multi callee-save + libmc call graph;
 * 8-byte-packed callee-save frame wall, see func_0029C678. Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SaveLoadStateMachine);

/* BuildSaveImage: assembles the save payload from the section table. Multi
 * callee-save; 8-byte-packed callee-save frame wall, see func_0029C678.
 * Writes the two section-table sizes as the leading header words (out[0]=global,
 * out[1]=area), then serializes the global block (slot 0) and all 0x1C area slots
 * after the header, advancing by each block's written byte count. The
 * TARGET_NATIVE #else is faithful coverage. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", BuildSaveImage);
#else
extern s32 SerializeSaveSections(void *dst, s32 slotMul, SaveSection *table); /* defined below */

void BuildSaveImage(s32 *out) {
    u8 *p;
    s32 i;

    out[0] = CalcSaveSectionsSize(g_saveSectionTableGlobal);
    out[1] = CalcSaveSectionsSize(g_saveSectionTableArea);

    p = (u8 *)out + 8;
    p += SerializeSaveSections(p, 0, g_saveSectionTableGlobal);
    for (i = 0; i < 0x1C; i++) {
        p += SerializeSaveSections(p, i, g_saveSectionTableArea);
    }
}
#endif

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

/* func_0029BCA0: CRC-16 (poly 0xEDB88320, 8-bit-at-a-time) over the save buffer
 * sized by CalcSaveSectionsSize(g_saveSectionTableGlobal). Uses three callee-
 * saved regs ($16/$17/$31) in an 8-byte-packed 0x20 frame — the callee-save
 * frame wall (our cc1 reserves 16 bytes per saved reg), see func_0029C678. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BCA0);

extern s32 func_0029BCA0(void *buf, s32 len); /* save-buffer CRC */

/* func_0029BD48(image): verify a save image's stored CRC. The image header is
 * { s32 payloadLen; s32 storedCrc; payload[payloadLen] } (the layout written by
 * SerializeSaveSections, which stores payloadLen at +0 and the CRC at +4).
 * Recomputes the CRC over the payload (func_0029BCA0 from image+8 over
 * payloadLen bytes) and returns 1 iff it equals the stored CRC. An image whose
 * stored CRC is 0 is treated as empty/invalid and returns 0.
 *
 * WALLED at the byte level by the 8-byte-packed callee-save frame (2 saved regs
 * $16/$31; the pinned 2.9 cc1 reserves 16 bytes/save vs the original's 8 — see
 * project_matching_ceiling, func_0029C678). The portable #else below is
 * cmp-oracle-validated (cmp_198FA0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BD48);
#else
s32 func_0029BD48(void *image) {
    s32 storedCrc = ((s32 *)image)[1];
    if (storedCrc == 0) {
        return 0;
    }
    return func_0029BCA0((char *)image + 8, ((s32 *)image)[0]) == storedCrc;
}
#endif

extern void FillMemory32(void *dst, s32 pattern, s32 nbytes);
extern void *func_00283460(void *dst, const void *src, s32 nbytes); /* memcpy */
extern int memcmp(); /* K&R decl: avoids the ee-gcc builtin-prototype conflict warning */

/* g_areaTable (0x1393E0): per-area record table, stride 0xA0. The deserializer
 * touches it only at two fixed byte offsets, so it is byte-addressed here to
 * keep the offset math faithful to the asm:
 *   base + 0x14C : the currently-selected area index (s32)
 *   record + 0x24: a per-area scratch slot where the load reconcile count lands.
 * (base + 0x14C lands inside record[2]; the engine reuses that word as the
 *  selected-area index, distinct from the per-record +0x24 result slot.) */
#define AREA_RECORD_STRIDE 0xA0
#define AREA_SELECTED_INDEX_OFF 0x14C
#define AREA_LOAD_RESULT_OFF 0x24
extern u8  g_areaTable[];        /* 0x1393E0 per-area record table (stride 0xA0) */
extern s32 D_1A99A0;             /* 0x1A99A0 changed-section counter (bumped on memcmp differ) */

/* SerializeSaveSections(dst, slot, table): write the section table `table` into
 * the save-image buffer `dst` for memory-card `slot`. The image begins with an
 * 8-byte header { s32 payloadLen; s32 crc; } at dst+0, followed by the section
 * stream starting at dst+8. Each non-terminator section emits an 8-byte header
 * { s32 tag; s32 len; } then its `len` payload bytes, 4-byte aligned. The
 * per-slot payload source is `srcPtr + len*slot` (each slot's data is packed
 * contiguously). Sections tagged 0x1770 are zero-filled rather than copied. A
 * terminator header { -1, 0 } closes the stream; the CRC over the whole payload
 * (from dst+8, payloadLen bytes) is then stored in the header. Returns the
 * total image size (payloadLen + 8).
 *
 * WALLED at the byte level by the 8-byte-packed callee-save frame (this fn uses
 * 8 callee-saved regs; the pinned 2.9 cc1 reserves 16 bytes/save vs the
 * original's 8 — the mips_reg_mode=TImode wall, see project_matching_ceiling;
 * func_0029C678). Body is logic-exact (every non-prologue insn matches at 71%);
 * the portable #else below is cmp-oracle-validated (cmp_198FA0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SerializeSaveSections);
#else
s32 SerializeSaveSections(void *dst, s32 slot, SaveSection *table) {
    s32 *cursor = (s32 *)((char *)dst + 8);
    s32 size = 0;

    if (table->srcPtr != 0) {
        do {
            s32 len = table->len;
            cursor[0] = table->tag;
            cursor[1] = len;
            size += 8;
            cursor += 2;
            if (table->tag == 0x1770) {
                FillMemory32(cursor, 0, table->len);
            } else {
                func_00283460(cursor, (char *)table->srcPtr + len * slot,
                              table->len);
            }
            len = table->len;
            table++;
            cursor = (s32 *)(((s32)cursor + len + 3) & -4);
            size = (size + len + 3) & -4;
        } while (table->srcPtr != 0);
    }

    size += 8;
    cursor[1] = 0;
    cursor[0] = -1;
    ((s32 *)dst)[1] = func_0029BCA0((char *)dst + 8, size);
    ((s32 *)dst)[0] = size;
    return size + 8;
}
#endif

/* func_0029BEA0 / FillSaveSlotInfo: fill one save-slot info-display entry (table
 * 0x139410, stride slot*0xA0 + dir*0x1C) from a verified header image. WALLED by
 * BOTH the 8-byte-packed callee-save frame (4 saves) and the unaligned 64-bit
 * ldl/ldr/sdl/sdr field moves the original emits for the +0x13..+0x7 copy — GNU
 * cc1 won't generate those from portable C, so no #else either. Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BEA0);

/* DeserializeSaveSections(image, slotMul, table): restore the section table
 * `table` from a save `image` (the inverse of SerializeSaveSections), reconciling
 * each image section against the descriptor table and reporting how many sections
 * failed to reconcile.
 *
 * The image is { s32 payloadLen; s32 crc; <sections> } with each section a header
 * { s32 tag; s32 len } then `len` payload bytes, 4-byte aligned, closed by a
 * { -1, * } terminator. `slotMul` selects the destination slice inside each
 * descriptor's srcPtr (srcPtr + slotMul*descLen), mirroring the serializer's
 * per-slot packing.
 *
 * Steps: (1) CRC-verify via func_0029BD48 — a bad image returns 1 immediately.
 * (2) Clear every descriptor's matchResult. (3) Walk the image sections: for each,
 * find the descriptor with the same tag; if found, record matchResult (1 / -1 /
 * -2 by length comparison), bump the global changed-section counter D_1A99A0 when
 * the bytes differ, and copy min(lengths) bytes into srcPtr+slotMul*descLen
 * (except tag-0x1770 zero-fill sections, which are not copied back). Sections
 * with no matching descriptor count as mismatches. (4) Mismatch tally: also count
 * a mismatch if the bytes consumed don't equal CalcSaveSectionsSize(table), plus
 * one for every descriptor that never reconciled (matchResult <= 0). (5) Store the
 * tally in the selected g_areaTable record (+0x24) and return it.
 *
 * WALLED at the byte level by the 8-byte-packed callee-save frame (10 saved regs:
 * s0-s7, fp, ra; the pinned 2.9 cc1 reserves 16 bytes/save vs the original's 8 —
 * see project_matching_ceiling, func_0029C678). The portable #else below is
 * cmp-oracle-validated (cmp_198FA0 isolated suite). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", DeserializeSaveSections);
#else
s32 DeserializeSaveSections(void *image, s32 slotMul, SaveSection *table) {
    s32 *section;        /* current image section header { tag, len, payload... } */
    s32 mismatchCount;
    s32 runningOffset;
    s32 sentinel;
    SaveSection *entry;

    if (func_0029BD48(image) == 0) {
        return 1;        /* CRC invalid: nothing restored */
    }

    section = (s32 *)((char *)image + 8);   /* first section header */
    mismatchCount = 0;
    runningOffset = 8;                       /* leading 8-byte image header */

    /* Clear the reconcile result on every descriptor up to the terminator. */
    for (entry = table; entry->srcPtr != 0; entry++) {
        entry->matchResult = 0;
    }

    /* Walk the image's sections (until the { -1, * } terminator). */
    while (section[0] != -1) {
        s32 sectionTag = section[0];
        s32 sectionLen = section[1];
        SaveSection *match = 0;

        /* Find the descriptor whose tag matches this section's tag. */
        for (entry = table; entry->srcPtr != 0; entry++) {
            if (entry->tag == sectionTag) {
                match = entry;
                break;
            }
        }

        if (match != 0 && match->srcPtr != 0) {
            s32 descLen = match->len;
            char *dest = (char *)match->srcPtr + slotMul * descLen;
            char *payload = (char *)(section + 2);
            s32 copyLen;

            if (sectionLen == descLen) {
                match->matchResult = 1;
                copyLen = descLen;
            } else if (sectionLen < descLen) {
                match->matchResult = -1;
                copyLen = sectionLen;
            } else {
                match->matchResult = -2;
                copyLen = descLen;
            }

            if (memcmp(dest, payload, copyLen) != 0) {
                D_1A99A0 += 1;          /* count a section whose bytes changed */
            }

            if (match->tag == 0x1770) {
                /* zero-fill section: not restored; advance by its image length */
                runningOffset += 8 + ((sectionLen + 3) & -4);
            } else {
                func_00283460(dest, payload, copyLen);
                runningOffset += 8 + ((copyLen + 3) & -4);
            }
        } else {
            mismatchCount += 1;          /* no descriptor for this section */
        }

        /* Advance to the next image section header (payload is len-padded). */
        section = (s32 *)((char *)section + 8 + ((sectionLen + 3) & -4));
    }

    runningOffset += 8;                  /* trailing terminator */

    if (table->srcPtr != 0) {
        if (runningOffset != CalcSaveSectionsSize(table)) {
            mismatchCount += 1;          /* total byte count disagrees */
        }
        /* Count every descriptor that never reconciled (matchResult <= 0),
         * stopping at the terminator or at a descriptor whose tag equals the
         * post-terminator image sentinel word. */
        sentinel = section[2];           /* word just past the terminator header */
        if (table->tag != sentinel) {
            entry = table;
            for (;;) {
                if (entry->matchResult <= 0) {
                    mismatchCount += 1;
                }
                entry++;
                if (entry->srcPtr == 0 || entry->tag == sentinel) {
                    break;
                }
            }
        }
    }

    /* Record the tally in the currently-selected area record. */
    {
        s32 idx = *(s32 *)(g_areaTable + AREA_SELECTED_INDEX_OFF);
        *(s32 *)(g_areaTable + idx * AREA_RECORD_STRIDE + AREA_LOAD_RESULT_OFF) =
            mismatchCount;
    }
    return mismatchCount;
}
#endif

/* CommitProgressCheckpoint(flag, areaParam): stage the progress checkpoint into
 * the in-RAM save image + arm a memcard write. Multi callee-save; 8-byte-packed
 * callee-save frame wall (matching build left as asm). Reads the RTC
 * (sceCdReadClock into the g_gsPixelOffsetY+0xC scratch), refreshes area
 * bookkeeping (func_00298A00 / func_00297FA0), then — when the current area is
 * valid and this checkpoint isn't suppressed — records boltCount/progress/
 * D_1A7BC8/clock/miscExtras into the g_areaTable write-slot record
 * (g_areaTable[+0x18]*0x1C, fields +0x30..+0x40), re-serializes the global +
 * per-area save images (SerializeSaveSections), and arms the write
 * (g_areaTable+0x164=0xF, +0x168=area). When areaParam >= 0 it temporarily
 * switches g_playerProgress to that area (marking it visited) across the
 * serialize, then restores. Returns nonzero once the write is armed (or on the
 * flag==0 early-out). The TARGET_NATIVE #else is faithful coverage. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", CommitProgressCheckpoint);
#else
extern u8   g_health[];               /* +0xF8C = per-level save-region base */
extern u8   g_gsPixelOffsetY[];       /* +0xC reused as the sceCdCLOCK scratch buffer */
extern s32  g_boltCount;
extern s32  D_1A7BC8;                 /* extra checkpoint word (recorded at rec+0x3C) */
extern u8   g_saveImageGlobal[];
extern u8   g_saveImageArea[];
extern s32  sceCdReadClock(void *clock);
extern void func_00131A98(void *clock);
extern void func_00298A00(void);
extern void func_00297FA0(void *saveRegion);
extern s32  SerializeSaveSections(void *dst, s32 slotMul, SaveSection *table);

s32 CommitProgressCheckpoint(s32 flag, s32 areaParam) {
    u8 *at    = g_areaTable;
    u8 *clock = g_gsPixelOffsetY + 0xC;
    s32 areaIdx;
    u8 *rec;
    s32 savedVisited;

    sceCdReadClock(clock);
    func_00131A98(clock);
    func_00298A00();
    func_00297FA0(g_health + 0xF8C + g_playerProgress * 0x800);

    areaIdx = *(s32 *)(at + 0x148);
    if (areaIdx == -1 || *(s16 *)(at + areaIdx * 0xA0 + 0x18) < 0) {
        return flag == 0;
    }

    *(s32 *)(at + 0x17C) |= flag;
    if (*(s32 *)(at + 0x17C) == 0) {
        return *(s32 *)(at + 0x164) == 0xF;
    }
    if (flag == 0) {
        g_nSaveLoadStatusCode[1] |= 0x200;   /* +0x4 word */
    }
    if (*(s32 *)(at + 0x15C) >= 3 || *(s32 *)(at + 0x164) >= 0) {
        return *(s32 *)(at + 0x164) == 0xF;
    }

    /* commit: snapshot the live progress state into the write-slot record */
    savedVisited = 0;
    *(s32 *)(at + 0x150) = g_playerProgress;
    if (areaParam >= 0) {
        g_playerProgress = areaParam;
        savedVisited = g_levelVisitedMarkers[g_playerProgress];
        if (savedVisited == 0) {
            g_levelVisitedMarkers[g_playerProgress] = 1;
        }
    }

    rec = at + *(s16 *)(at + 0x18) * 0x1C;
    *(s32 *)(rec + 0x34) = g_boltCount;
    *(s32 *)(rec + 0x30) = g_playerProgress;
    *(s32 *)(rec + 0x3C) = D_1A7BC8;
    *(u64 *)(rec + 0x40) = *(u64 *)clock;
    *(s32 *)(rec + 0x38) = g_miscExtras;   /* asm stores the byte value as a word (sw) */

    SerializeSaveSections(g_saveImageGlobal, 0, g_saveSectionTableGlobal);
    SerializeSaveSections(g_saveImageArea, *(s32 *)(at + 0x150), g_saveSectionTableArea);

    if (areaParam >= 0) {
        g_levelVisitedMarkers[g_playerProgress] = (u8)savedVisited;
        g_playerProgress = *(s32 *)(at + 0x150);
    }
    if (*(s32 *)(at + 0x164) < 0) {
        *(s32 *)(at + 0x164) = 0xF;
        *(s32 *)(at + 0x168) = *(s32 *)(at + 0x148);
    }
    return *(s32 *)(at + 0x164) == 0xF;
}
#endif

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
extern s32 func_00339398(char *widget, s32 a, s32 b);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C448);
#else
s32 func_0029C448(s32 a, s32 b) {
    if (g_guiInstance != 0) {
        return func_00339398(g_guiInstance + 0x36F28, a, b);
    }
}
#endif

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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C5B0);
#else
s32 func_0029C5B0(s32 a, s32 b, s32 idx) {
    char *slot;
    if (g_guiInstance == 0 || (u32)idx >= 2) {
        return 0;
    }
    slot = g_guiInstance + 0x38000 + idx * 8;
    *(s32 *)(slot + 0x7A18) = a;
    *(s32 *)(slot + 0x7A1C) = b;
    return 1;
}
#endif

/* func_0029C600(idx): clear the slot pair at g_guiInstance+0x3FA18+idx*8 (the
 * counterpart of func_0029C5B0). UNMATCHABLE in this unit: it reads
 * g_guiInstance via %gp_rel($gp) while every other function here reads it via
 * the adjacent absolute lui/lw pair — the same symbol, both ways, inside one
 * original TU (the proven reload-artifact wall). The unit-wide extern model
 * can only express one side per symbol (g_guiInstance is modeled absolute,
 * favouring the ~60 wrappers), so this stays asm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C600);
#else
void func_0029C600(s32 idx) {
    char *slot;
    if ((u32)idx >= 2) {
        return;
    }
    slot = g_guiInstance + 0x38000 + idx * 8;
    *(s32 *)(slot + 0x7A18) = 0;
    *(s32 *)(slot + 0x7A1C) = 0;
}
#endif

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
 * reserves 16 bytes per save) - the wall characterized in the 1907F0 round.
 *
 * func_0029C678: when the GUI singleton exists, dispatch a text-box render
 * through func_00338F88 with the GUI's text-box context (g_guiInstance+0x36F28)
 * prepended, forwarding its 10 args verbatim (args 1-7 in $4-$10, arg8 in $11,
 * args 9-10 on the incoming stack). func_00338F88's 11-arg prototype recovered
 * via Ghidra. The TARGET_NATIVE #else is faithful coverage. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C678);
#else
extern void func_00338F88(char *ctx, void *state, void *a2, void *text, s32 font,
                          s32 centre, void *a6, s32 flag, s32 a8, s32 a9, s32 a10);

void func_0029C678(void *state, void *a1, void *text, s32 font, s32 centre,
                   void *a5, s32 flag, s32 a7, s32 a8, s32 a9) {
    if (g_guiInstance != 0) {
        func_00338F88(g_guiInstance + 0x36F28, state, a1, text, font, centre,
                      a5, flag, a7, a8, a9);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C700);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C818);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C8F0);

/** Invalidate the D_1A9A90 GUI state (set to -1). */
void func_0029CA88(void) {
    D_1A9A90 = -1;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CA98);

/* func_0029CC48: rebuild the camera projection with a temporary FOV override.
 * When the GUI singleton exists, force the camera FOV field (g_sceneActorMobys
 * +0x724) to 0.62, rebuild the projection, run the GUI camera hook
 * (func_0034F220 on g_guiInstance+0x36F28), then restore the saved FOV and
 * rebuild again. Matching build: callee-save frame wall (see func_0029C678);
 * the TARGET_NATIVE #else is faithful coverage. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CC48);
#else
extern u8   g_sceneActorMobys[];               /* +0x724 = camera FOV field */
extern void BuildCameraProjection(void);
extern void func_0034F220(void *guiCameraCtx);

void func_0029CC48(void) {
    if (g_guiInstance != 0) {
        f32 *fov = (f32 *)(g_sceneActorMobys + 0x724);
        f32  saved = *fov;
        *fov = 0.62f;
        BuildCameraProjection();
        func_0034F220(g_guiInstance + 0x36F28);
        *fov = saved;
        BuildCameraProjection();
    }
}
#endif

/* func_0029CCB8: if the GUI is up and several gating flags (D_1A9A88,
 * g_nNanotechBonusHealTimer+4, D_1A8C64, D_1A9A8C) permit, forward to the
 * widget at g_guiInstance+0x3F7B0 (func_0033B720). UNMATCHABLE: it reads
 * g_guiInstance (and D_1A9A88/D_1A9A8C) through %gp_rel($gp) while the rest of
 * the unit reads g_guiInstance via the absolute lui/lw pair — the reload-
 * artifact wall (one form per symbol), see the file header. Left as asm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CCB8);
#else
void func_0029CCB8(void) {
    /* All five gates must permit before the popup-poll runs:
     *  D_1A9A88 set, the nanotech sub-state half at +0x4 clear, the popup-busy
     *  gate D_1A8C64 clear, D_1A9A8C set, and the GUI instance up. */
    if (D_1A9A88 == 0) {
        return;
    }
    if (*(s16 *)((char *)&g_nNanotechBonusHealTimer + 0x4) != 0) {
        return;
    }
    if (D_1A8C64 != 0) {
        return;
    }
    if (D_1A9A8C == 0) {
        return;
    }
    if (g_guiInstance == 0) {
        return;
    }
    func_0033B720(g_guiInstance + 0x3F7B0);
}
#endif

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

/** UpdateAudioOptionsMenuInGameDispatch: route from the in-game pause/options screen
 *  into the audio-options tab - forward `arg` to the widget at g_guiInstance+0x3DA78
 *  (method func_0033E070 = UpdateAudioOptionsMenuInGame); 0 when the GUI is down. */
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

/* GuiManagerCreate: allocate + initialise the 0x3FB20-byte GuiManager singleton
 * (g_guiInstance). Large multi callee-save constructor; 8-byte-packed callee-save
 * frame wall (matching build left as asm). Poisons the GUI heap (memset 0xCD over
 * 0x40000 bytes at g_memoryArenaTable[0x80]), clears the input/pad state window
 * (D_138180 + 0x1A0..0x1CC), runs the mode init (func_0029DB58), placement-news +
 * inits the instance (GuiPlacementNew / GuiSystemInit), stores it to
 * g_guiInstance, then installs the two update callbacks at instance+0x3F9D4/
 * +0x3F9D8 — the special pair (func_0029CF08/func_0029DB50) when g_playerProgress
 * == 0x1F5, else the default pair (func_0029CF40/func_0029CF10). The TARGET_NATIVE
 * #else is faithful coverage. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", GuiManagerCreate);
#else
extern u8   D_138180[];              /* controller / input state block (0x138180) */
extern u8   g_memoryArenaTable[];    /* memory-arena table; +0x80 = GUI heap base ptr */
extern void *GuiPlacementNew(s32 size, void *heap);
extern void *GuiSystemInit(void *placement, s32 size);
extern void func_0029CF08(void);     /* progress==0x1F5 update callback pair */
extern void func_0029DB50(void);
/* func_0029DB58, func_0029CF40, func_0029CF10 are defined earlier in this unit. */

void GuiManagerCreate(void) {
    u8   *in   = D_138180;
    void *heap = *(void **)(g_memoryArenaTable + 0x80);
    void *instance;
    void *cb4, *cb3;

    memset(heap, 0xCD, 0x40000);
    *(s32 *)(in + 0x1CC) = 0;
    *(s32 *)(in + 0x1A0) = 0;
    *(s32 *)(in + 0x1A4) = 0;
    *(s32 *)(in + 0x1A8) = 0;
    *(s32 *)(in + 0x1AC) = 0;
    *(s32 *)(in + 0x1B0) = 0;
    *(s32 *)(in + 0x1B4) = 0;
    *(s32 *)(in + 0x1B8) = 0;
    *(s32 *)(in + 0x1BC) = 0;
    *(s32 *)(in + 0x1C0) = 0;
    *(s32 *)(in + 0x1C4) = 0;
    *(s32 *)(in + 0x1C8) = 0;

    func_0029DB58();   /* reads g_bPalMode itself (asm passes it in $4; the callee ignores the arg) */
    instance = GuiSystemInit(GuiPlacementNew(0x3FB20, heap), 0x40000);
    g_guiInstance = instance;

    if (g_playerProgress == 0x1F5) {
        cb4 = (void *)func_0029CF08;
        cb3 = (void *)func_0029DB50;
    } else {
        cb4 = (void *)func_0029CF40;
        cb3 = (void *)func_0029CF10;
    }
    *(void **)((u8 *)instance + 0x3F9D4) = cb4;
    *(void **)((u8 *)instance + 0x3F9D8) = cb3;
}
#endif

/** If the GUI is up and the popup-busy gate (D_1A8C64) is clear, pause the
 *  game world (func_0028E9A0(1)) and run the GUI pump (func_0029CA98),
 *  propagating its result.
 *
 *  The two early-exit paths (GUI down, or popup-busy) fall off the end with no
 *  explicit `return`: the pinned cc1 incidentally leaves the in-register value
 *  in $v0 (0 when g_guiInstance==0, the loaded D_1A8C64 when it is non-zero), so
 *  the matching PS2 build is byte-exact AS WRITTEN. A different host compiler
 *  resolves the fall-off-end UB differently and DIVERGES on the return value
 *  (memory effects are identical). The #else makes those two returns explicit so
 *  the native/functional-equivalence build agrees with the real R5900 result.
 *  Do not add explicit returns to the #ifndef arm - that breaks the byte match. */
#ifndef TARGET_NATIVE
s32 func_0029DC70(void) {
    if (g_guiInstance != 0) {
        if (D_1A8C64 == 0) {
            func_0028E9A0(1);
            return func_0029CA98();
        }
    }
}
#else
s32 func_0029DC70(void) {
    if (g_guiInstance == 0) {
        return 0;
    }
    if (D_1A8C64 != 0) {
        return D_1A8C64;
    }
    func_0028E9A0(1);
    return func_0029CA98();
}
#endif

/* func_0029DCB0: MIS-SPLIT — splat began the symbol one instruction early, so
 * the body carries the leaked `addiu $sp,0x10; nop` epilogue of the preceding
 * func_0029DC70 before the real entry (the internal `alabel func_0029DCB8`).
 * The real body maps g_playerProgress (0x16->9, 0x17->0x12, else 0) and calls
 * RequestLevelExit(.,1); but the prepended dead prologue can't be expressed as
 * one C function. Left as asm until the split boundary is corrected. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DCB0);

/* func_0029DD08: MIS-SPLIT (same shape as func_0029DCB0) — leaked `addiu
 * $sp,0x10; nop` epilogue of func_0029DCB0 prepended; the real body (internal
 * `alabel func_0029DD10`) is the g_guiInstance+0x3CEA0 wrapper to func_0033A9F8.
 * Not expressible as one C function with the dead prologue. Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD08);

/** Forward `arg` to the widget at g_guiInstance+0x3CEA0 (method func_0033A8F0);
 *  0 when the GUI is down. */
s32 func_0029DD40(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033A8F0(g_guiInstance + 0x3CEA0, arg);
}

/* func_0029DD80: 0xC bytes of dead inter-function fill (`sw $2,gp_rel(...)` +
 * `addiu $sp,0x50` epilogue orphan, no jr) — the split tail of func_0029DD90,
 * not compiler-reachable C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD80);

/* func_0029DD90: toSPR (SPR-TO) DMA-kick helper — program the channel registers
 * at 0x1000D400 (QWC @+0x80, sadr @+0x20, madr @+0x10) and start the transfer
 * (CHCR=0x100 @+0x00); called 4x from the sky-piece stagers. The C body is
 *   volatile u32 *ch = (volatile u32*)0x1000D400;
 *   ch[0x20]=qwc; ch[8]=sadr; ch[4]=madr; ch[0]=0x100;
 * and reproduces the single-base store sequence and exact offsets — but it is
 * WALLED by two later-cc1 traits the pinned cc1 cannot match under this unit's
 * fixed -O2 -G8 -fno-gcse recipe (best 68.75%):
 *   (a) the original materialises the CHCR 0x100 constant LATE (just before the
 *       final store, in source order), whereas the pinned cc1's insn scheduler
 *       hoists `li $2,256` to the top; only -fno-schedule-insns2 keeps it late,
 *       and that flag can't be added unit-wide without regressing the other 83.
 *   (b) the original colours the base register $1 ($at) and the constant $2,
 *       while the pinned cc1 picks $2/$3 (the same register-coloring wall as
 *       func_0029C448/func_0029C5B0). Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD90);

/* func_0029DDB0: hardware DMA busy-wait — spins on the CHCR busy bit (0x100) of
 * DMA channel register 0x1000D400 until the toSPR transfer completes. A bare
 * register poll loop (internal `alabel func_0029DDB8`, `j` back-edge, nop-padded
 * loads); not compiler-reachable from C. Left as asm (hardware/HLE). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DDB0);

/* func_0029DDE8 / func_0029E090 / func_0029E1D0 / DecompressWad / func_0029E5D8
 * / func_0029E5F8 / EnableDmacChannels / FlushPendingTexUploads: hand-written
 * DMAC/SPR/VIF assembly (splat-flagged "Handwritten function"). They poke the
 * EE peripheral registers (0x1000xxxx) with trapping `add`/`teq` and bare
 * register spin loops that no C source reproduces — not compiler-reachable.
 * Left as asm (hardware/HLE). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DDE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", EnableDmacChannels);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", FlushPendingTexUploads);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E090);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E1D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", DecompressWad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E5D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E5F8);

/* UpdateLevelObjectiveStates: re-evaluate every objective of the current level.
 *
 * Three passes over the current level's objective list (head cached in the
 * heightmap scratch at +0x38, terminator = id 0):
 *   1. score each objective's state (0/1/2) from its two progress conditions,
 *      forcing it inactive (state 0) when level-gated and the level hasn't been
 *      visited;
 *   2. run any per-objective tick callback, stashing its result at +0x26;
 *   3. tally the objectives that are active (state 1) and not hidden-when-
 *      complete (flags bit 0x2) into the +0x34 outstanding count.
 *
 * Returns 1 iff no objectives remain outstanding (the level's objectives are
 * all satisfied), else 0. Returns 0 immediately if the level has no list.
 *
 * WALLED (probed 2026-06-25, best 69.08%): the body logic reproduces
 * instruction-for-instruction, but the original (later SN cc1) (a) over-allocates
 * the frame (128 vs 48 bytes, 7 vs 5 callee-saves); (b) fills loop-back delay
 * slots with branch-likely (`beqzl`/`bnezl` + annulled next-iteration load) where
 * the pinned cc1 emits plain `beqz` + `nop`. (The function is void and both
 * builds rematerialise the lui/%lo g_pRainHeightmap+0x38 pair per pass - no
 * callee-save-base trick is involved here.) Same later-cc1 frame-packing/
 * scheduling class as GatherActiveObjectives/EvaluateProgressCondition. The
 * TARGET_NATIVE arm below is the faithful portable body (cmp-oracle'd). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", UpdateLevelObjectiveStates);
#else
s32 UpdateLevelObjectiveStates(void) {
    ObjectiveScan *scan = (ObjectiveScan *)(g_pRainHeightmap + 0x34);
    LevelObjective *rec;

    rec = D_258B20[*(s32 *)(g_mapVertexData + 0x230)];
    scan->head = rec;
    if (rec == NULL) {
        return 0;
    }

    /* Pass 1: score each objective from its progress conditions. */
    for (; rec->id != 0; rec++) {
        if ((rec->flags & 0x4) &&
            g_levelVisitedMarkers[*(s32 *)(g_mapVertexData + 0x230)] == 0) {
            rec->state = 0;
            continue;
        }
        if (EvaluateProgressCondition(rec->cond1Sel, rec->cond1Arg) == 0) {
            rec->state = 0;
            continue;
        }
        if (EvaluateProgressCondition(rec->cond2Sel, rec->cond2Arg) != 0) {
            rec->state = 2;
        } else {
            rec->state = 1;
        }
    }

    /* Pass 2: tick callbacks. */
    for (rec = scan->head; rec->id != 0; rec++) {
        if (rec->tickFn != NULL) {
            rec->tickResult = rec->tickFn(rec->tickArg);
        }
    }

    /* Pass 3: count outstanding (active, not hidden-when-complete) objectives. */
    scan->outstanding = 0;
    for (rec = scan->head; rec->id != 0; rec++) {
        if (rec->state == 1 && (rec->flags & 0x2) == 0) {
            scan->outstanding += 1;
        }
    }

    return scan->outstanding == 0;
}
#endif

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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", EvaluateProgressCondition);
#else
/* EvaluateProgressCondition(cond, arg): evaluate one progress/unlock predicate.
 * `cond` is a 16-bit selector (sign-extended); `arg` is the per-case operand
 * (an index, a function pointer for case 7, or a packed level/bit field). Each
 * case returns a 0/1 truth value (case 10 returns the map predicate verbatim).
 * Any cond outside [0,11] returns 0. See the jump-table block decode above. */
s32 EvaluateProgressCondition(s32 cond, s32 arg) {
    cond = (s32)(s16)cond;
    if ((u32)cond >= 0xC) {
        return 0;
    }
    switch (cond) {
    case 0:
        return 1;
    case 1:
        return g_abLevelAvailableFlags[arg] != 0;
    case 2:
        return g_inventoryOwned[arg] != 0;
    case 3:
        return g_inventoryNewFlag[arg] != 0;
    case 4: /* objective/weapon "started" - level field nonzero */
        if (arg >= 0x72) {
            return 0;
        }
        return D_139A28[arg].upgradeLevel != 0;
    case 5: /* objective/weapon "complete" - level field >= 2 */
        if (arg >= 0x72) {
            return 0;
        }
        return (D_139A28[arg].upgradeLevel < 2) ? 0 : 1;
    case 6:
        return D_1395B8[arg] != 0;
    case 7: /* call `arg` as a predicate function pointer */
        return ((s32 (*)(void))arg)() != 0;
    case 8: /* platinum bolt: arg packs group (high 16) and slot (low 16) */
        return g_platinumBoltFlags[(arg & 0xFFFF) + ((arg >> 16) * 4)] != 0;
    case 9: { /* cinematic bit: arg>>2 selects the word, arg&0x1F the bit */
        s32 word = *(s32 *)((char *)&g_cinematicUnlockedFlags + ((arg >> 2) << 2));
        return (word & (1 << (arg & 0x1F))) != 0;
    }
    case 10:
        return func_002FCEA0(g_mapCurrentLevel);
    default: /* case 11 */
        return 0;
    }
}
#endif

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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", GatherActiveObjectives);
#else
s32 GatherActiveObjectives(s32 *outIds, s32 *outMask, s32 *outVals, s32 wantValues) {
    u8 *rec = *(u8 **)(g_pRainHeightmap + 0x38);   /* objective list head */
    s32 count = 0;
    s32 allComplete = 1;

    *outIds = 0;
    if (outMask != 0) {
        *outMask = 0;
    }
    if (outVals != 0) {
        *outVals = -1;
    }
    if (rec == 0) {
        return 0;
    }

    while (*(s16 *)(rec + 0x0) != 0) {           /* rec->id */
        s32 state = *(s16 *)(rec + 0x24);
        u16 flags = *(u16 *)(rec + 0x10);

        if (state != 2 && (flags & 2) == 0) {
            allComplete = 0;                     /* a visible, not-complete one */
        }
        if ((flags & 2) != 0) {                  /* hidden -> skip, no count */
            rec += 0x28;
            continue;
        }
        if (state == 0) {                        /* inactive -> skip, no count */
            rec += 0x28;
            continue;
        }
        if ((flags & 1) != 0 && state == 2) {    /* completed+flag1 -> skip */
            rec += 0x28;
            continue;
        }

        /* emit: id, or (when wantValues) the display value in its place */
        outIds[0] = *(s16 *)(rec + 0x0);
        if (wantValues != 0) {
            if (state == 2) {
                outIds[0] = 0x31B9;
            } else {
                outIds[0] = *(s16 *)(rec + *(s16 *)(rec + 0x26) * 2 + 0x14);
            }
        }
        if (outMask != 0 && state == 2) {
            *outMask |= (1 << count);            /* completion bit for this slot */
        }
        if (outVals != 0) {
            *outVals = *(s16 *)(rec + 0x12) + *(s16 *)(rec + 0x26);
            outVals++;
        }
        outIds++;
        count++;
        rec += 0x28;
    }

    if (outMask != 0 && allComplete != 0) {
        *outMask |= 0x80000000;                  /* bit 31 = all non-hidden complete */
    }
    return count;
}
#endif

/* func_0029EA90 (and EAC8/EB08/EB38 below): 0/1 predicates over globals
 * (g_miscExtras + D_1397E0 bit 0x8000000 here). WALLED (probed 2026-06-12):
 * the boolean tail `if (test) return 1; return 0;` is scc-converted by the
 * pinned cc1 into `sltu $2,$0,$2` on EVERY source shape probed (if-chain,
 * nested guards, v=1/v=0 flag variable), while the original (later SN cc1)
 * emits the branch + per-path constant materialisation (`bnez; addiu $2,1 /
 * daddu $2,0`). Same class: func_0029EC70. Predicates returning 0/1/2
 * (func_0029EB68/func_0029EBF8) are NOT walled - scc cannot synthesise 2. */
/** Returns 1 iff the misc-extras gate is clear (g_miscExtras == 0) AND the
 *  D_1397E0 busy bit (0x8000000) is set; 0 otherwise. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EA90);
#else
s32 func_0029EA90(void) {
    if (g_miscExtras != 0) {
        return 0;
    }
    if ((D_1397E0 & 0x8000000) != 0) {
        return 1;
    }
    return 0;
}
#endif

/** Returns 1 iff all three dialog flags are set (D_1395C1, D_1A7B0D, D_1A7B14);
 *  0 as soon as any is clear. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EAC8);
#else
s32 func_0029EAC8(void) {
    if (D_1395C1 == 0) {
        return 0;
    }
    if (D_1A7B0D == 0) {
        return 0;
    }
    if (D_1A7B14 != 0) {
        return 1;
    }
    return 0;
}
#endif

/** Returns 1 iff a stream is in flight (D_1397C4 < 0) AND the busy flag
 *  D_1395D5 is set; 0 otherwise. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB08);
#else
s32 func_0029EB08(void) {
    if (D_1397C4 >= 0) {
        return 0;
    }
    if (D_1395D5 != 0) {
        return 1;
    }
    return 0;
}
#endif

/** Returns 1 iff both dialog flags are set (D_1A7BDD AND D_1A7B10); 0 otherwise. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB38);
#else
s32 func_0029EB38(void) {
    if (D_1A7BDD == 0) {
        return 0;
    }
    if (D_1A7B10 != 0) {
        return 1;
    }
    return 0;
}
#endif

/* func_0029EB68: cinematic skip-prompt arbiter (posts HUD message 0x1F/0x20
 * into D_257502, returns 0/1/2 on the D_1395B8[0x1D] / cinematic word +0x5C
 * pair). RE-PROBED 2026-06-12, best 76.57%: the original keeps both %hi
 * halves live in registers and re-materialises the D_1395B8 base via addiu
 * before each reload, while the pinned cc1 folds the +0x1D element address
 * into the lui/lbu pair (pointer-local shapes scored worse). Left as asm. */
extern s16 D_257502;   /* cinematic-unlock status code (0x1F / 0x20) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB68);
#else
s32 func_0029EB68(void) {
    /* cinematic word sits at &g_cinematicUnlockedFlags + 0x5C (region-anchor
     * sibling global; same base-relative access as the +0x90/+0x98 readers). */
    s32 cinWord = *(s32 *)((char *)&g_cinematicUnlockedFlags + 0x5C);

    if (D_1395B8[0x1D] == 0) {
        D_257502 = 0x1F;                    /* story flag clear */
        return (cinWord >= 0) ? 0 : 2;      /* cinematic ready vs still locked */
    }
    if (cinWord >= 0) {
        D_257502 = 0x20;                    /* story flag set + cinematic ready */
        return 1;
    }
    return 0;                               /* story flag set, cinematic locked */
}
#endif

/* func_0029EBF8: dialog-skip arbiter twin of func_0029EB68 on the D_1A7BDD/
 * D_1A7B10 byte pair (posts 0x28/0x29 into D_2579B2, returns 0/1/2). WALLED
 * (probed 2026-06-12): the original (later SN cc1) copies the first byte to
 * $a0 and re-narrows it with a redundant `andi 0xFF` at each re-test; the
 * pinned cc1 tracks the lbu value range and deletes the narrowing on every
 * shape probed (u8 locals, s32 locals + (u8) casts, a|b joint test). Best
 * 52.77%. */
extern s16 D_2579B2;   /* dialog-skip status code (0x28 / 0x29) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EBF8);
#else
s32 func_0029EBF8(void) {
    if (D_1A7BDD == 0) {
        if (D_1A7B10 == 0) {
            D_2579B2 = 0x28;   /* neither flag: base skip code, no arbitration */
            return 0;
        }
        D_2579B2 = 0x29;       /* only the second flag set */
        return 2;
    }
    if (D_1A7B10 == 0) {
        D_2579B2 = 0x29;       /* only the first flag set */
        return 1;
    }
    return 0;                  /* both set: no skip, D_2579B2 unchanged */
}
#endif

/** Returns 1 iff cinematic-flag word +0x90 has bit 0x10000 set AND word +0x98
 *  has bit 0x4000000 set; 0 otherwise. (The original has a redundant early-out
 *  testing the same two bits first; it cannot change the result, so the #else
 *  collapses to the single conjunction.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EC70);
#else
s32 func_0029EC70(void) {
    char *flags = (char *)&g_cinematicUnlockedFlags;
    if ((*(s32 *)(flags + 0x90) & 0x10000) == 0) {
        return 0;
    }
    if ((*(s32 *)(flags + 0x98) & 0x4000000) != 0) {
        return 1;
    }
    return 0;
}
#endif

/* func_0029ECE0: the 0x1118-byte cinematic/dialog driver (largest non-state-
 * machine function here). Deep multi callee-save ($16/$17/$18+); 8-byte-packed
 * callee-save frame wall, see func_0029C678. Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029ECE0);

/* func_0029FDF8: a single orphan `lh $2,0x24($3)` (no prologue/jr) — dead
 * inter-function fill spilled from the tail of func_0029ECE0, not compiler-
 * reachable C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029FDF8);

/* SpawnMoby: allocates a moby from the spawn free-list (g_mobySpawnStart..
 * g_mobyTableEnd) and initialises it. Multi callee-save; 8-byte-packed
 * callee-save frame wall, see func_0029C678. Left as asm (matching). Scans slots
 * (stride 0x100) for the first free one — state (+0x20) >= 0xFE with an expired
 * reservation (+0xA0 <= g_gameTime) — inits it (InitMobyFromClass), binds+zeroes
 * its parallel 0x80-byte aux block (g_mobyAuxBlockBase[slot] at moby+0x68), and
 * decrements the spawn credit; returns 0 (logging) when full. The TARGET_NATIVE
 * #else is faithful coverage. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SpawnMoby);
#else
extern u8  *g_mobySpawnStart;    /* 0x1B1AE0 first dynamic moby slot */
extern u8  *g_mobyTableEnd;      /* 0x1B1AE4 moby table walk bound */
extern u8  *g_mobyAuxBlockBase;  /* 0x1B1AEC parallel per-moby 0x80-byte block array */
extern s32  g_mobySpawnCredit;   /* remaining spawn budget */
extern s32  g_gameTime;          /* global frame counter */
extern char D_1A9DC8[];          /* "no free moby slot" log string */
extern void InitMobyFromClass();  /* K&R: defined below with the Moby typedef */

void *SpawnMoby(s32 classId) {
    u8 *m = g_mobySpawnStart;

    if (m < g_mobyTableEnd) {
        s32 state = m[0x20];
        for (;;) {
            if (state >= 0xFE && !(g_gameTime < *(s32 *)(m + 0xA0))) {
                if (state == 0xFF) {
                    m[0x120] = (u8)state;
                }
                InitMobyFromClass(m, classId);
                {
                    s32 slot = (s32)(m - g_mobySpawnStart) / 0x100;
                    u8 *aux = g_mobyAuxBlockBase + slot * 0x80;
                    *(u8 **)(m + 0x68) = aux;
                    FillMemory32(aux, 0, 0x80);
                }
                if (g_mobySpawnCredit != 0) {
                    g_mobySpawnCredit--;
                }
                return m;
            }
            m += 0x100;
            if (m >= g_mobyTableEnd) {
                break;
            }
            state = m[0x20];
        }
    }
    DebugPrintStub(D_1A9DC8);   /* compiled-out; original also passes g_gameTime */
    return 0;
}
#endif

/* Canonical Moby entity record (full field layout in include/moby.h, sizeof
 * 0x100). InitMobyFromClass zero-fills and stamps it; the body does its own
 * (u8*)moby offset arithmetic, so a full-size opaque view suffices. Byte-neutral
 * (a struct typedef emits no code; the matching arm is INCLUDE_ASM regardless). */
typedef struct Moby { u8 _bytes[0x100]; } Moby;
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(Moby) == 0x100, "Moby must be 0x100 under ILP32");
#endif

/* Moby-class binding tables consulted by InitMobyFromClass (all arena-placed). */
extern void FillMemory32(void *dst, s32 pattern, s32 nbytes);
extern u8    g_mobyClassSlotRemap[];            /* 0x1CE460 classId -> slot byte */
extern s16   g_mobyClassSlotToId[];             /* 0x1CE280 slot -> classId (reverse) */
extern void *g_mobyClassUpdateFuncs[];          /* 0x1CDEC0 per-slot pUpdate (header) */
extern void *g_mobyClassUpdateFuncsNoHeader[];  /* 0x1D0460 per-slot pUpdate (headerless) */
extern void *g_mobyClassHeaders[];              /* 0x1CDB00 per-slot class-header ptr */
extern u8   *g_mobyTableBase;                   /* 0x1B1ADC base of the 0x100-stride table */
extern void  ResolveMobyAnimFramePtrs(void *moby);

/* InitMobyFromClass: zero-fills a moby's 0x100-byte state (FillMemory32) and
 * binds it to a class. Stamps the defaults the spawn path expects - classSlot
 * (+0x22) = g_mobyClassSlotRemap[classId], alpha (+0x23)=0x80, default tint
 * qword (+0x38), uid (+0xAC) = (tableIndex<<16), classId (+0xAA), the 0x7F/0x80
 * colour-channel bytes, and the 1.0 anim rates (+0x48/+0x4C). It then validates
 * the slot through the reverse map g_mobyClassSlotToId: if it does NOT round-trip
 * back to classId the class has no loaded header, so it takes the headerless
 * path (flags |= 5, pUpdate from g_mobyClassUpdateFuncsNoHeader, flags |= 2 when
 * none) and returns. Otherwise it binds the header: pClass (+0x24), pUpdate from
 * g_mobyClassUpdateFuncs, scale (+0x2C) and flag bits from the header, optional
 * collision mesh (+0x78), then resolves anim-frame pointers when the class has
 * an animation set. The matching build keeps the asm (multi callee-save, 8-byte-
 * packed save-slot frame wall - see func_0029C678). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", InitMobyFromClass);
#else
void InitMobyFromClass(Moby *moby, s32 classId) {
    u8 *m = (u8 *)moby;
    u8 slot;
    s32 index;
    u8 *pc;
    void *animSet;

    FillMemory32(m, 0, 0x100);

    slot = g_mobyClassSlotRemap[classId];
    m[0x23] = 0x80;                 /* alpha */
    m[0x22] = slot;                 /* class slot */
    m[0xA8] = 0xFF;
    m[0x21] = 0xFF;
    m[0x61] = 0xFF;
    m[0x62] = 0xFF;
    *(u64 *)(m + 0x38) = 0x0040404000000000ULL; /* default tint qword */
    *(s16 *)(m + 0x36) = 0x7F80;
    index = (s32)(m - g_mobyTableBase) >> 8;     /* slot index in the 0x100 table */
    *(s32 *)(m + 0xAC) = index << 16;            /* uid */
    m[0x6D] = 0xFF;
    m[0xA5] = 0x7F;
    m[0xA7] = 0x80;
    *(s16 *)(m + 0xAA) = (s16)classId;
    m[0x6E] = 0;
    m[0x6C] = 0xFF;
    m[0xA4] = 0x7F;
    m[0xA6] = 0x80;

    if (g_mobyClassSlotToId[slot] != classId) {
        /* headerless class - no loaded header for this slot */
        u16 flags = (u16)(*(u16 *)(m + 0x34) | 0x5);
        void *upd = g_mobyClassUpdateFuncsNoHeader[slot];
        *(s32 *)(m + 0x24) = 0;
        *(s32 *)(m + 0x98) = 0;
        *(void **)(m + 0x64) = upd;
        if (upd == 0) {
            flags |= 0x2;
        }
        *(u16 *)(m + 0x34) = flags;
        return;
    }

    /* header class */
    {
        void *upd = g_mobyClassUpdateFuncs[slot];
        *(void **)(m + 0x64) = upd;
        if (upd == 0) {
            *(u16 *)(m + 0x34) |= 0x2;
        }
    }
    pc = (u8 *)g_mobyClassHeaders[slot];
    *(void **)(m + 0x24) = pc;
    m[0x62] = pc[0x0E];
    *(u16 *)(m + 0x34) |= *(u16 *)(pc + 0x44);
    *(s32 *)(m + 0x98) = *(s32 *)(pc + 0x10);
    *(f32 *)(m + 0x4C) = 1.0f;
    *(f32 *)(m + 0x2C) = *(f32 *)(pc + 0x24);    /* default scale */
    *(f32 *)(m + 0x48) = 1.0f;
    if (*(s32 *)(pc + 0x40) != 0) {
        *(u16 *)(m + 0x34) |= 0x10;
        *(s32 *)(m + 0x78) = *(s32 *)(pc + 0x40); /* collision mesh */
    }

    pc = *(u8 **)(m + 0x24);
    if (pc[0x0F] != 0) {
        m[0x6F] = 0x18;
        *(s32 *)(m + 0x70) = 0;
        *(u16 *)(m + 0x34) |= 0x400;
        *(s32 *)(m + 0x74) = 0;
        m[0xBD] = 0;
    }

    pc = *(u8 **)(m + 0x24);
    if (pc[0x06] != 0) {
        m[0x63] = 0x18;
    }

    pc = *(u8 **)(m + 0x24);
    animSet = *(void **)(pc + 0x48);
    if (animSet != 0) {
        ResolveMobyAnimFramePtrs(m);
        pc = *(u8 **)(m + 0x24);
        animSet = *(void **)(pc + 0x48);
        if (*(u8 *)((u8 *)animSet + 0x10) >= 2) {
            *(u16 *)(m + 0x34) &= 0xFFFD;
        }
        pc = *(u8 **)(m + 0x24);
        if (pc[0x0C] == 1) {
            animSet = *(void **)(pc + 0x48);
            if (*(u8 *)((u8 *)animSet + 0x10) < 2) {
                *(s32 *)(m + 0x48) = 0;
                animSet = *(void **)(pc + 0x48);
                if (*(s8 *)((u8 *)animSet + 0x11) < 0) {
                    *(u16 *)(m + 0x34) |= 0x40;
                }
            }
        }
    }
}
#endif

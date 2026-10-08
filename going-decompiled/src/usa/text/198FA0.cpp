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
 * func_002995E0/func_00299758/UpdateSaveTaskState/func_002998D0/func_00299918).
 *
 * Task #511 (2026-09-20) MEASURED that wall as an assembler property, not a
 * compiler one: cc1 2.9 already emits the bare small-data macro (`sw $2,sym`)
 * that the ROM assembler resolved to the one-insn %gp_rel form in delay slots;
 * GNU as expands it to lui/%lo (2 insns + nop) instead. Rewriting exactly those
 * delay-slot macros to `%gp_rel(sym)($28)` in the cc1 .s before assembling
 * (tools/ee/.t511/gprel_dslot_fixup2.py, an EXPERIMENT — the tree's asm step
 * does not do this; landing it is a gate-instrument ruling) makes
 * func_002991E8 / func_002992B8 / func_002992E8 / func_002993D8 / func_0029CCB8
 * BYTE IDENTICAL to the ROM (verify_match_unit.sh) and lifts 13 more of the
 * family. func_00299960 needs no such thing: its only access IS the delay-slot
 * one, and -G8 small data gives the %gp_rel form directly (matched, #511).
 *
 * Task #525 then found that emulation ALREADY in the tree for one class:
 * asm_unit.sh's -G8 awk rewrites a bare small-data macro that sits directly
 * after a branch in a noreorder block to one `%gp_rel(sym)($28)` word when the
 * symbol's `.extern` size is in the 9..15 band (the text/1A8180 "we use 12"
 * class: absolute in straight-line code, %gp_rel in a delay slot). This file
 * had marked its mixed symbols 16 (the hoist class), which that rule skips.
 * Task #656 moves g_nSaveLoadStatusCode to 12, which makes func_002991E8 /
 * func_002992B8 / func_002993D8 byte-exact under the tree's own asm step.
 * Task #888 closes func_0029CCB8 on the same lever: g_guiInstance goes to 12,
 * and its callee func_0033B720 gets a symbol_addrs line under its existing
 * name, so ORPHAN_LATENT does not grow.
 * func_002992E8 was listed here as needing #525's "rule 2" (a slot store
 * scheduled BEFORE the branch). It does not: with the flag word declared as
 * its own g_gameStateFlags scalar at 12, cc1 fills the slot itself (#888).
 * Every #else arm below carries its measured state on BOTH gate arms
 * (`t511 promotion sweep` block): the sdk29 arm is the better instrument for
 * this TU on 29 of the 43 arms.
 */

/* Original cc1-small / assembler-absolute symbols (see header). Size 16 =
 * absolute everywhere (hoisted out of delay slots); size 12 = absolute in
 * straight-line code, one-word %gp_rel when the access fills a delay slot. */
/* g_guiInstance is 12: its delay-slot read in func_0029CCB8 must assemble to
 * the ROM's one-insn %gp_rel word (asm_unit.sh -G8 awk, FACT #7461); the other
 * compiled readers are unchanged, measured by object compare in task #888. */
__asm__(".extern g_guiInstance, 12");
__asm__(".extern g_bPalMode, 16");
__asm__(".extern g_loadedArmorVariant, 16");
__asm__(".extern g_loadedHeldItemModelId, 16");
__asm__(".extern g_levelDialogToc, 16");
__asm__(".extern g_nSaveLoadStatusCode, 12");
__asm__(".extern g_gameStateFlags, 12");
__asm__(".extern D_1A8C64, 16");
__asm__(".extern D_1A8C88, 12");
__asm__(".extern D_1A8C8C, 12");

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
/* The same pending-flag word as g_nSaveLoadStatusCode[1] (0x1A7424), under the
 * name the ROM's relocations use. A function whose ROM stores it as one
 * %gp_rel word in a delay slot uses this name: cc1 sees a separate 4-byte
 * scalar, fills the slot with the store, and the .extern 12 above keeps its
 * straight-line reads absolute (task #888, func_002992E8). */
extern s32 g_gameStateFlags;
extern s32 D_1A8C64;  /* GUI popup-busy gate (also read by the walled func_0029CCB8) */

/* Two card-error gate words in the save-prompt small-data block adjacent to
 * D_1A8C64; read by the save/load status handlers below to choose between the
 * "card removed / fatal" and "retry" message paths.
 *   D_1A8C88 : set when the active card slot reported a hard/unrecoverable error
 *   D_1A8C8C : set when a card-removal abort is in progress
 * Widths follow the original lw opcodes. Both carry .extern 12 (task #888):
 * the ROM reads each as one %gp_rel word when the access fills a delay slot
 * and absolutely everywhere else (func_00299040 shows both forms of each). */
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
 * plain `j` (the original uses jal + return). func_0029CA98's own definition
 * is void in the ROM (task #1738, see it), so its s136os TU must not see this
 * declaration. */
extern s32 DebugPrintStub(char *msg);
extern void func_0028E9A0(s32 arg);
#if !defined(S136OS_func_0029CA98)
extern s32 func_0029CA98(void);
#endif
extern s32 func_0033A8F0(void *widget, s32 arg);

/* EE register pin; a no-op on the native build, where "$2" is not a register name. */
#ifndef TARGET_NATIVE
#define EE_REG(r) __asm__(r)
#else
#define EE_REG(r)
#endif

/* Progress/dialog flag globals read by the 0x29EAxx predicate family below.
 * Widths follow the original load opcodes (lbu = u8, lw = s32). Declared here
 * for the TARGET_NATIVE #else arms; the #ifndef arms stay INCLUDE_ASM (these
 * declarations emit no code, so the matching build is unaffected). */
#define ROM_SPLIT __attribute__((section(".data")))
extern u8  g_miscExtras;                       /* 0x1A7A12 misc-extras gate byte, nonzero blocks (cc1-small, see below) */
extern s32 D_1397E0 ROM_SPLIT;                 /* 0x1397E0 status word; bit 0x8000000 = busy */
extern u8  D_1395C1 ROM_SPLIT;                 /* 0x1395C1 dialog-active flag */
extern u8  D_1A7B0D;                           /* 0x1A7B0D dialog flag (cc1-small, see below) */
extern u8  D_1A7B14;                           /* 0x1A7B14 dialog flag (cc1-small, see below) */
extern s32 D_1397C4 ROM_SPLIT;                 /* 0x1397C4 streaming state (sign = idle) */
extern u8  D_1395D5 ROM_SPLIT;                 /* 0x1395D5 streaming-busy flag */
extern u8  D_1A7BDD;                           /* 0x1A7BDD dialog flag (cc1-small, see below) */
extern u8  D_1A7B10;                           /* 0x1A7B10 dialog flag (cc1-small, see below) */
extern s32 g_cinematicUnlockedFlags ROM_SPLIT; /* 0x139768 cinematic bitfield (read at +0x90/+0x98) */

/* cc1-small / assembler-absolute (ADDRESSING-MODEL DEVICE, the 1CA080.cpp form):
 * the ROM loads these bytes with the assembler-macro shape -- `lui $N; lbu $N,
 * %lo($N)`, the destination register reused as the base -- which is what GNU as
 * prints for cc1's one-insn `lbu $N,sym` when it has seen `.extern sym, 16`
 * (> -G8) before the use. cc1 still emits its own `.extern sym, 1` at the end
 * of the file; the earlier, larger size governs. No ROM_SPLIT on them: that
 * makes cc1 split the address into two pseudos, which it allocates to two
 * registers. Emits no code; the only EE-compiled readers are the s136os
 * members below. */
__asm__(".extern g_miscExtras, 16");
__asm__(".extern D_1A7B0D, 16");
__asm__(".extern D_1A7B14, 16");
__asm__(".extern D_1A7BDD, 16");
__asm__(".extern D_1A7B10, 16");

/* Gating globals + widget method for the func_0029CCB8 popup-poll wrapper. */
extern s32 D_1A9A88;                 /* 0x1A9A88 GUI-active gate (gp small-data) */
extern s32 D_1A9A8C;                 /* 0x1A9A8C GUI-ready gate (gp small-data) */
extern s32 g_nNanotechBonusHealTimer;/* 0x189FFC; +0x4 is a separate s16 sub-state */
extern s16 D_18A000 ROM_SPLIT;       /* 0x18A000 nanotech sub-state half (g_nNanotechBonusHealTimer+0x4) */
extern s32 func_0033B720(void *widget); /* GUI popup-poll method */

/* Progress-condition flag arrays read by EvaluateProgressCondition's 12 cases
 * (widths follow the original lbu/lw opcodes). Declared for the TARGET_NATIVE
 * #else arm; emit no code so the matching build is unaffected. */
extern u8  g_abLevelAvailableFlags[]; /* 0x1A7BD0 per-level available flag (case 1) */
extern u8  g_inventoryOwned[];        /* 0x1A7B00 per-item have-flag (case 2) */
extern u8  g_inventoryNewFlag[];      /* 0x1A7B38 per-item newly-acquired flag (case 3) */
extern u8  D_1395B8[];                /* 0x1395B8 progress flag byte-array (cases 6 and 10;
                                       * what the flags mean is undetermined) */
extern u8  g_platinumBoltFlags[];     /* 0x19B278 per-platinum-bolt collected flag (case 8) */
extern s32 g_mapCurrentLevel ROM_SPLIT; /* 0x1C5150 current map level id (case 10) */
extern s32 func_002FCEA0(s32 level, s32 bitIndex); /* case 10 callee: tests bit
                                       * `bitIndex` of D_1395B8[the byte its level keys to
                                       * via the table at 0x264F30] and WRITES THE MASKED
                                       * BYTE BACK, clearing the other bits, so it is not a
                                       * pure predicate (FACT #8398).
                                       * asm 0x2FCEA0 saves $5 in the delay slot
                                       * of the jal and uses it as the sllv SHIFT AMOUNT. */

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
    s16 valueBase;      /* 0x12 GatherActiveObjectives reports valueBase + tickResult */
    s16 stageTextIds[4];/* 0x14 text id shown for each tickResult (GatherActiveObjectives) */
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

/* ADDRESSING-MODEL DEVICE (RULING #8620; the FACT #8036 size-16 equate form,
 * as g_playerProgressAbs below): GatherActiveObjectives loads the list head
 * with the assembler-macro shape `lui $9; lw $9,%lo(g_pRainHeightmap+0x38)($9)`,
 * the destination register reused as the base. That is what GNU as prints for
 * cc1's one-insn `lw $9,sym` when sym is a cc1-small scalar sized 16 for the
 * assembler. Reading it through g_pRainHeightmap makes cc1 split the address
 * itself into a separate lui temp ($2). The equate names no new storage and the
 * relocation still names g_pRainHeightmap+0x38; nothing is emitted. */
#ifndef TARGET_NATIVE
__asm__(".extern g_objectiveScanHeadAbs, 16\n\tg_objectiveScanHeadAbs = g_pRainHeightmap + 0x38");
extern LevelObjective *g_objectiveScanHeadAbs;
#else
#define g_objectiveScanHeadAbs (((ObjectiveScan *)(g_pRainHeightmap + 0x34))->head)
#endif

extern u8 g_levelVisitedMarkers[]; /* 0x1A7BF0 per-level visited byte markers */

/* MISNAMED: this is a per-level MAP-BLIP record, not a weapon-upgrade record
 * (FACT #8397). The names are kept because EvaluateProgressCondition is matched
 * and g_weaponUpgradeLevel is bound in committed asm; a rename would break both.
 * EvaluateProgressCondition cases 4/5 index this stride-0x10 table at 0x139A28
 * (114 entries) and read the s32 state at +0xC (symbol g_weaponUpgradeLevel =
 * 0x139A34). func_00298A00 fills one slice of indices (bounds from D_264DD0,
 * keyed by g_playerProgress 0x1A79F8; that the key is the current level rests
 * on names and uses elsewhere, not on a measurement here): +0/+4 get an
 * object's +0x10/+0x14 floats, the x/y pair the map projects, and +8 gets a
 * copy of its +0xF8 float, which is not z (NOTE #8405). MapDraw bumps the
 * icon's sprite variant when state bit 1 is set. func_0029ECE0 zeroes the
 * icon's (g_pMapBlipList entry's) +0x24 only when the icon's own +4 halfword
 * has bit 0x1000 set AND state bit 0 is clear; without 0x1000 it keeps the
 * +0x24 value the same loop stored just before. What the blips are, and who
 * writes the state, is undetermined.
 * Those readers are the ones a static lui/%lo scan of the asm finds, which is
 * not exhaustive: it cannot see computed-pointer access. One such path exists:
 * the whole table (0x720 = 114 x 0x10 bytes) is entry 16 of
 * g_saveSectionTableGlobal (tag 0xF), so the save-section code reaches it
 * through srcPtr; whether that path writes the state was not traced.
 * Weapon upgrades: the only lead is a name. GetWeaponUpgradeLevel (0x288988,
 * g_weaponTable) is the upgrade level by its symbol name alone, UNCONFIRMED.
 * Its code follows its argument's +0x4C links to the chain root, then returns
 * the number of +0x4A hops from there to the chain's end, so its result does
 * not depend on where in the chain the argument sits (NOTE #5654; its static
 * xref scan found no writer of the links, which cannot rule out a
 * computed-pointer one). */
typedef struct WeaponUpgradeRecord {
    s32 _pad0[3];   /* 0x0 map x/y at +0/+4, a copy of an object's +0xF8 at +8 */
    s32 upgradeLevel; /* 0xC - blip state (misnamed), not an upgrade level */
} WeaponUpgradeRecord;
extern WeaponUpgradeRecord D_139A28[]; /* 0x139A28 per-level map-blip table (stride 0x10) */

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
extern s32 func_00346EF0(void *widget, s32 buttons, s32 *out);
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
 * func_00299238/func_002992E8/func_00299348/func_00299478/func_002994B0/
 * func_00299568/func_002995E0/func_00299758/UpdateSaveTaskState/func_002998D0/
 * func_00299918): memory-card save/load status handlers. Each reads a
 * small-range save-context global both via the 1-insn %gp_rel form (in a
 * branch/jr delay slot) AND via the absolute lui/$at macro elsewhere in the
 * same function — the delay-slot form described in the file header. Members of
 * the family that closed under the size-12 marker (func_002991E8,
 * func_002992B8, func_002993D8) are compiled C. Task #888 closed eight more by
 * spelling the pending-flag word as g_gameStateFlags at size 12 (func_00299178,
 * func_00299238, func_002992E8, func_00299478, func_002994B0, func_002995E0,
 * func_002998D0, func_00299918), plus func_00299348 with its flag read moved
 * after the busy test, and func_00299758 and func_00299568 with their branch stores reordered, and
 * func_00299040 and UpdateSaveTaskState (see their comments). */
/** Save/load top-level status arbiter. Always consumes the pending-flag's 0x2
 *  and 0x4 bits first. Then: if no save is pending (areaTable/dirty +0x17C == 0)
 *  -> status 3. Otherwise route the original flags: bit 0x80 (or secondary-path
 *  +0x16C) -> status 0x15 (consume 0x80, set 0x40); bit 0x100 -> status 0x14
 *  (consume 0x100, set 0x40); else if a card transaction is active (busy != 0)
 *  clear the pending flag and, on a hard/abort card error (D_1A8C88/D_1A8C8C)
 *  show status 3, else show status 2 (set bit 0x1); else (idle) bit 0x200 ->
 *  status 0x19 (consume 0x200).
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888). Four things carry it:
 *   - the flag word is spelled g_gameStateFlags (see its declaration);
 *   - D_1A8C88 and D_1A8C8C are at .extern 12, so the delay-slot read of
 *     D_1A8C88 is %gp_rel while D_1A8C8C's straight-line read stays absolute;
 *   - the 0x2/0x4 clear goes through a separate temporary. The ROM keeps two
 *     `and`s (with -5, then -3). One `& ~0x6` folds to a single and, and a
 *     `cleared &= ~0x2` on the same variable colours $v0/$a0 the other way;
 *   - in the two branches that end with the same pair of stores (tail-merged
 *     by cc1), the flag word is stored before the status code.
 *  The reload-artifact wall this comment used to name was KNOWN-FALSE for
 *  it. */
void func_00299040(void) {
    s32 flags = g_gameStateFlags;
    s32 t = flags & ~0x4;
    s32 cleared = t & ~0x2;
    g_gameStateFlags = cleared;
    if (D_1393E0.dirty == 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if ((flags & 0x80) || D_1393E0.unk16C != 0) {
        g_nSaveLoadStatusCode[0] = 0x15;
        g_gameStateFlags = (cleared & ~0x80) | 0x40;
        return;
    }
    if (flags & 0x100) {
        g_nSaveLoadStatusCode[0] = 0x14;
        g_gameStateFlags = (cleared ^ 0x100) | 0x40;
        return;
    }
    if (D_1393E0.busy != 0) {
        D_1393E0.dirty = 0;
        if (D_1A8C88 != 0 || D_1A8C8C != 0) {
            g_nSaveLoadStatusCode[0] = 3;
        } else {
            g_gameStateFlags = cleared | 0x1;
            g_nSaveLoadStatusCode[0] = 2;
        }
    } else if (flags & 0x200) {
        g_gameStateFlags = cleared ^ 0x200;
        g_nSaveLoadStatusCode[0] = 0x19;
    }
}

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
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888): the pending-flag word
 *  is spelled g_gameStateFlags, the ROM's own symbol, so its delay-slot
 *  access is one %gp_rel word and every other access is absolute (see
 *  g_gameStateFlags' declaration). The reload-artifact wall this comment
 *  used to name was KNOWN-FALSE for it. */
void func_00299178(void) {
    g_gameStateFlags &= ~0x20;
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

/** Card-removal step: if no abort is in flight (busy != -2) show status 3
 *  (idle). Otherwise, if a hard card error is latched (D_1A8C8C set) show
 *  status 6; if the pending-flag's 0x2 bit is set show status 6.
 *  Byte-exact on the sdk29 arm (task #656): the ROM stores
 *  g_nSaveLoadStatusCode as one %gp_rel word in a branch delay slot, which
 *  asm_unit.sh reproduces because the symbol's .extern size is 12; at
 *  size 16 the store is hoisted as lui/sw and the function reads 86.32%
 *  (unit objdiff). */
void func_002991E8(void) {
    if (D_1393F0[0] != -2) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if (D_1A8C8C != 0) {
        g_nSaveLoadStatusCode[0] = 6;
    } else if (g_nSaveLoadStatusCode[1] & 0x2) {
        g_nSaveLoadStatusCode[0] = 6;
    }
}

/** Card-abort step: if no abort is in flight (busy != -2) show status 3 (idle).
 *  Otherwise dispatch on the pending-flag word: bit 0x20 -> if a hard card
 *  error is latched (D_1A8C88) clear it and show status 0x17, else show status
 *  5; bit 0x8 -> clear it and show status 7.
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888): the pending-flag word
 *  is spelled g_gameStateFlags, the ROM's own symbol, so its delay-slot
 *  access is one %gp_rel word and every other access is absolute (see
 *  g_gameStateFlags' declaration). The reload-artifact wall this comment
 *  used to name was KNOWN-FALSE for it. */
void func_00299238(void) {
    s32 flags;
    if (D_1393F0[0] != -2) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    flags = g_gameStateFlags;
    if (flags & 0x20) {
        /* The 0x20 bit is cleared on BOTH the error and no-error paths: the
         * original writes it back in the branch delay slot before testing
         * D_1A8C88, so the clear happens regardless of the error result. */
        g_gameStateFlags = flags ^ 0x20;
        if (D_1A8C88 != 0) {
            g_nSaveLoadStatusCode[0] = 0x17;
        } else {
            g_nSaveLoadStatusCode[0] = 5;
        }
    } else if (flags & 0x8) {
        g_gameStateFlags = flags ^ 0x8;
        g_nSaveLoadStatusCode[0] = 7;
    }
}

/** Result-timeout step: always clear the transaction busy flag; then if the
 *  libmc result is still pending (result < 0) force result 3 + subResult 0.
 *  Always show status 8 (formatting/working).
 *  Byte-exact on the sdk29 arm (task #656): the ROM stores
 *  g_nSaveLoadStatusCode as one %gp_rel word in a branch delay slot, which
 *  asm_unit.sh reproduces because the symbol's .extern size is 12; at
 *  size 16 the store is hoisted as lui/sw and the function reads 76.36%
 *  (unit objdiff). */
void func_002992B8(void) {
    D_1393E0.busy = 0;
    if (D_1393E0.result < 0) {
        D_1393E0.subResult = 0;
        D_1393E0.result = 3;
    }
    g_nSaveLoadStatusCode[0] = 8;
}

/*
 * func_002992E8 — format-confirm step of the save/load status machine. Only
 * while a format request (mode 2) is still pending (result < 0): if the
 * secondary/format path flag (unk16C) is set, show status 0x11 and set
 * pending-flag bit 0x40; otherwise show status 0xE. No params, no return.
 *
 * Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888). The ROM reads the flag
 * word absolutely (lui/lw) BEFORE the status store and writes it back as one
 * %gp_rel word in the jr delay slot, under the symbol g_gameStateFlags. With
 * the flag word spelled g_nSaveLoadStatusCode[1], cc1 2.9 kept the store out
 * of the slot and the body read 95.42% (unit objdiff, #686). #525/#656 read
 * that as a missing assembler rule ("rule 2"). Spelling the flag word as its
 * own g_gameStateFlags scalar at .extern 12 closes it with the tree's asm step
 * unchanged, so that explanation was KNOWN-FALSE for this function.
 */
void func_002992E8(void) {
    if (D_1393E0.mode == 2 && D_1393E0.result < 0) {
        if (D_1393E0.unk16C != 0) {
            g_nSaveLoadStatusCode[0] = 0x11;
            g_gameStateFlags |= 0x40;
        } else {
            g_nSaveLoadStatusCode[0] = 0xE;
        }
    }
}

/** Load-prompt step: if a transaction is active (busy != 0) show status 3
 *  (idle). Otherwise dispatch on the pending-flag word: bits 0x6 -> status
 *  0xA; else bit 0x200 -> status 0x19.
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888). It needs two things:
 *  the flag word spelled g_gameStateFlags (see its declaration), and the flag
 *  read placed AFTER the busy test. Reading it into a local first gives the
 *  same instructions with $v0/$v1/$a0 coloured differently (98.16%, unit
 *  objdiff). The reload-artifact wall this comment used to name was KNOWN-FALSE
 *  for it. */
void func_00299348(void) {
    s32 flags;
    if (D_1393F0[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    flags = g_gameStateFlags;
    if (flags & 0x6) {
        g_nSaveLoadStatusCode[0] = 0xA;
    } else if (flags & 0x200) {
        g_nSaveLoadStatusCode[0] = 0x19;
    }
}

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
 *  Byte-exact on the sdk29 arm (task #656): the ROM stores
 *  g_nSaveLoadStatusCode as one %gp_rel word in a branch delay slot, which
 *  asm_unit.sh reproduces because the symbol's .extern size is 12; at
 *  size 16 the store is hoisted as lui/sw and the function reads 79.87%
 *  (unit objdiff). */
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

/** Save/load status predicate: while a card transaction is active
 *  (D_1393F0/busy != 0) show popup status 3 (idle/none); otherwise, if the
 *  pending-flag's 0x2 bit is set, show status 0xD.
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888): the pending-flag word
 *  is spelled g_gameStateFlags, the ROM's own symbol, so its delay-slot
 *  access is one %gp_rel word and every other access is absolute (see
 *  g_gameStateFlags' declaration). The reload-artifact wall this comment
 *  used to name was KNOWN-FALSE for it. */
void func_00299478(void) {
    if (D_1393F0[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
    } else if (g_gameStateFlags & 0x2) {
        g_nSaveLoadStatusCode[0] = 0xD;
    }
}

/** Save/load status dispatch: while a card transaction is active
 *  (D_1393F0/busy != 0) show status 3. Otherwise route on the pending-flag word:
 *  bit 0x20 -> clear it, then if a hard card error is latched (D_1A8C88) show
 *  status 0x18 else status 0xC; bit 0x10 -> clear it and show status 0xE.
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888): the pending-flag word
 *  is spelled g_gameStateFlags, the ROM's own symbol, so its delay-slot
 *  access is one %gp_rel word and every other access is absolute (see
 *  g_gameStateFlags' declaration). The reload-artifact wall this comment
 *  used to name was KNOWN-FALSE for it. */
void func_002994B0(void) {
    s32 flags;
    if (D_1393F0[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    flags = g_gameStateFlags;
    if (flags & 0x20) {
        g_gameStateFlags = flags ^ 0x20;   /* consume bit 0x20 */
        if (D_1A8C88 != 0) {
            g_nSaveLoadStatusCode[0] = 0x18;
        } else {
            g_nSaveLoadStatusCode[0] = 0xC;
        }
    } else if (flags & 0x10) {
        g_gameStateFlags = flags ^ 0x10;   /* consume bit 0x10 */
        g_nSaveLoadStatusCode[0] = 0xE;
    }
}

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
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888). It needs two things:
 *  the flag word spelled g_gameStateFlags (see its declaration), and this
 *  statement order in the else branch. cc1 2.9's scheduler emits the five
 *  stores in an order that depends on the source order. The committed order is
 *  one of 2 found closing among the first 77 of the 120 orders tried solo; the
 *  documented order reads 85.86% (unit objdiff). The reload-artifact wall this
 *  comment used to name was KNOWN-FALSE for it. */
void func_00299568(void) {
    if (D_1393E0.mode != 2 || D_1393E0.result >= 0) {
        return;
    }
    if (D_1393E0.unk16C != 0) {
        g_nSaveLoadStatusCode[0] = 0x12;
        g_gameStateFlags |= 0x40;
    } else {
        D_1393E0.result = 7;
        D_1393E0.unk148 = 0;
        D_1393E0.slot = 0;
        D_1393E0.subResult = 0;
        g_nSaveLoadStatusCode[0] = 0x16;
    }
}

/** Save/load status: first consume the pending-flag's 0x4 and 0x2 bits if set.
 *  Then route on the remaining flags: bit 0x80 -> status 0x15 (consume 0x80, set
 *  0x40); bit 0x100 -> status 0x14 (consume 0x100, set 0x40); else if a card
 *  transaction is active (g_areaTable/busy != 0) -> status 3; else if the
 *  save-pending flag (+0x17C) is set -> status 1.
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888): the pending-flag word
 *  is spelled g_gameStateFlags, the ROM's own symbol, so its delay-slot
 *  access is one %gp_rel word and every other access is absolute (see
 *  g_gameStateFlags' declaration). The reload-artifact wall this comment
 *  used to name was KNOWN-FALSE for it. */
void func_002995E0(void) {
    s32 flags = g_gameStateFlags;
    if (flags & 0x4) {
        g_gameStateFlags = flags & ~0x4;
    }
    flags = g_gameStateFlags;
    if (flags & 0x2) {
        g_gameStateFlags = flags & ~0x2;
    }
    flags = g_gameStateFlags;
    if (flags & 0x80) {
        g_nSaveLoadStatusCode[0] = 0x15;
        g_gameStateFlags = (flags ^ 0x80) | 0x40;
    } else if (flags & 0x100) {
        g_nSaveLoadStatusCode[0] = 0x14;
        g_gameStateFlags = (flags ^ 0x100) | 0x40;
    } else if (D_1393E0.busy != 0) {
        g_nSaveLoadStatusCode[0] = 3;
    } else if (D_1393E0.dirty != 0) {
        g_nSaveLoadStatusCode[0] = 1;
    }
}

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
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888). It needs two things:
 *  the flag word spelled g_gameStateFlags (see its declaration), and the
 *  save-pending clear written BEFORE the status store. In the old order
 *  (status, clear, flags) the flag word lands in $v1 instead of $v0, and the
 *  beqz slot is filled from the other path (80.32%, unit objdiff). The
 *  reload-artifact wall this comment used to name was KNOWN-FALSE for it. */
void func_00299758(void) {
    if (D_1393E0.mode != 2 || D_1393E0.result >= 0) {
        return;
    }
    if (D_1393E0.unk16C != 0 || D_1393E0.unk1C[2] != 0) {
        D_1393E0.dirty = 0;
        g_nSaveLoadStatusCode[0] = 0x15;
        g_gameStateFlags |= 0x440;
    } else {
        g_nSaveLoadStatusCode[0] = 1;
        D_1393E0.dirty = 1;
    }
}

/** Save-complete handler (only while a card transaction finished, mode 2,
 *  result < 0). Nothing pending (secondary-path +0x16C == 0 and not busy) ->
 *  mark save-pending (+0x17C = 1) and show status 1. Secondary path
 *  (+0x16C != 0) -> clear the pending flag, show status 0x15, set the
 *  pending-flag word to 0x40, then drive a game-state change to the save/load
 *  screen (RequestGameStateChange(4, g_nGameState == 0 ? 1 : 2, 1, 0, 0)); if
 *  already in the level-exit state (g_nGameState == 6) call
 *  SetSavePromptPending (0x289798, which sets g_nSavePromptPending = 1; this
 *  comment used to call it a cinematic flush); finally show status 2. Else
 *  (busy, no secondary path) -> clear the pending-flag's 0x40 bit, set
 *  bit 0x1, show status 2.
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888). Three things carry it:
 *   - the flag word is spelled g_gameStateFlags, and g_nGameState is at
 *     .extern 12 (the ROM reads it as %gp_rel in a `b` delay slot and
 *     absolutely elsewhere; without the marker it reads 96.62%, unit objdiff);
 *   - unk16C is read once into a local and tested twice, as the ROM keeps it
 *     in $v1;
 *   - one status store at the bottom, fed by a local. The ROM tail-merges it
 *     with the `status = 1` path, which it places last, so the test is written
 *     `secondary || busy` with that path in the else arm.
 *  The reload-artifact wall this comment used to name was KNOWN-FALSE for
 *  it. */
__asm__(".extern g_nGameState, 12");
extern s32 g_nGameState;
extern s32  RequestGameStateChange(s32 newState, s32 argA, s32 argB, s32 argC, s32 argD);
extern void SetSavePromptPending(void);
void UpdateSaveTaskState(void) {
    s32 secondary;
    s32 status;
    if (D_1393E0.mode != 2 || D_1393E0.result >= 0) {
        return;
    }
    secondary = D_1393E0.unk16C;
    if (secondary != 0 || D_1393E0.busy != 0) {
        D_1393E0.dirty = 0;
        if (secondary != 0) {
            g_nSaveLoadStatusCode[0] = 0x15;
            g_gameStateFlags = 0x40;
            if (g_nGameState == 0) {
                RequestGameStateChange(4, 1, 1, 0, 0);
            } else {
                RequestGameStateChange(4, 2, 1, 0, 0);
            }
            if (g_nGameState == 6) {
                SetSavePromptPending();
            }
        } else {
            g_gameStateFlags = (g_gameStateFlags & ~0x40) | 0x1;
        }
        status = 2;
    } else {
        status = 1;
        D_1393E0.dirty = status;
    }
    g_nSaveLoadStatusCode[0] = status;
}

/** Save/load status: while a card-removal abort is NOT in flight
 *  (D_1393F0/busy != -2) show status 3; otherwise, if the pending-flag's 0x20
 *  bit is set, consume it and show status 5.
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888): the pending-flag word
 *  is spelled g_gameStateFlags, the ROM's own symbol, so its delay-slot
 *  access is one %gp_rel word and every other access is absolute (see
 *  g_gameStateFlags' declaration). The reload-artifact wall this comment
 *  used to name was KNOWN-FALSE for it. */
void func_002998D0(void) {
    s32 flags;
    if (D_1393F0[0] != -2) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    flags = g_gameStateFlags;
    if (flags & 0x20) {
        g_gameStateFlags = flags ^ 0x20;   /* consume bit 0x20 */
        g_nSaveLoadStatusCode[0] = 5;
    }
}

/** Save/load status: while a card transaction is active (D_1393F0/busy != 0)
 *  show status 3; otherwise, if the pending-flag's 0x20 bit is set, consume it
 *  and show status 0xC.
 *  Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888): the pending-flag word
 *  is spelled g_gameStateFlags, the ROM's own symbol, so its delay-slot
 *  access is one %gp_rel word and every other access is absolute (see
 *  g_gameStateFlags' declaration). The reload-artifact wall this comment
 *  used to name was KNOWN-FALSE for it. */
void func_00299918(void) {
    s32 flags;
    if (D_1393F0[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    flags = g_gameStateFlags;
    if (flags & 0x20) {
        g_gameStateFlags = flags ^ 0x20;   /* consume bit 0x20 */
        g_nSaveLoadStatusCode[0] = 0xC;
    }
}

/* Declared here because the gas equate that aliases the latch word sits a few
 * lines below (with func_00299968); the definition order fixes this function's
 * placement in .text. Under the unit's -G8 model the 4-byte extern is small
 * data, so cc1 emits the single %gp_rel load the ROM has in the jr slot. */
extern s32 g_savePromptLatch;

/** Peek the latched save-prompt flag without consuming it.
 *  Companion to func_00299968, which reads-and-clears the same word;
 *  this one only reads, so repeated calls keep observing a pending prompt.
 *  @return the raw latch value at 0x1B19BC (non-zero while a prompt is armed).
 *  Matched on the sdk29 arm (task #511); the ROM addresses the word as
 *  g_pRainHeightmap+0x1C, the same 0x1B19BC the equate names. */
s32 func_00299960(void) {
    return g_savePromptLatch;
}

/* Latched event flag at g_pSkyShellSpinRates+0xAC (0x1B19BC): an unrelated
 * bss word splat attributes to the spin-rate symbol; aliased via a gas
 * symbol equate because no symbol exists at that address (a symbol_addrs pin
 * + re-split would name it properly). The equate is EE-only: under native PIC
 * the GOT load of an undefined-symbol-plus-offset alias cannot be relocated,
 * and the native link must give the latch its own storage instead. */
extern s32 g_savePromptLatch;
#ifndef TARGET_NATIVE
__asm__("g_savePromptLatch = g_pSkyShellSpinRates+0xAC");
#endif

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
/* t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 58.98% -> PACKED-SAVE, first differing row @0: ROM `addiu sp, sp, -0x20` vs `addiu sp, sp, -0x30`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 55.28% -> SCHED, first differing row @1: ROM `daddu a2, a0, zero` vs `lui v0, %hi(g_saveDirTemplate)`. */
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299B00);

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
/* t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 63.07% -> PACKED-SAVE, first differing row @0: ROM `addiu sp, sp, -0x40` vs `addiu sp, sp, -0x70`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 41.93% -> SCHED-PROEPI, first differing row @2: ROM `sd s2, 0x10(sp)` vs `sd s5, 0x28(sp)`. */
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
extern s16  PumpDialogVoiceSystem(s32 blocking); /* its definition's type (1B4218.cpp); returns in $2, FACT #9329 */
extern void StartFileLoadPumpingVoice(void *buf, s32 lba, s32 size);
#ifdef TARGET_NATIVE
/* #else-only early decl: func_00299BF8's #else uses func_00283460 before the
 * file-scope decl further down. Guarded so the matching build (unifdef
 * -UTARGET_NATIVE) is byte-identical to master. */
extern void *func_00283460(void *dst, const void *src, s32 nbytes); /* memcpy */
#endif

/* ADDRESSING-MODEL DEVICE (RULING #8620; FACT #8036's size-16 equate form,
 * as 1A00F0.cpp's g_mobySegmentOpenTagAbs): GuiManagerCreate reads
 * g_playerProgress absolutely (0x29DC08 `lui v1,%hi(g_playerProgress)`), and
 * func_00299BF8 stores it absolutely (0x299C7C), while the symbol is -G8
 * small, so those accesses name a second assembler symbol
 * EQUATED to it and sized 16; the relocation still names g_playerProgress.
 * Top level, so the s136os TU and the unit's 2.9 TU both define it; placed
 * ahead of func_00299BF8, its first user. Nothing is moved and nothing is
 * emitted; native reads the plain symbol. */
#ifndef TARGET_NATIVE
__asm__(".extern g_playerProgressAbs, 16\n\tg_playerProgressAbs = g_playerProgress");
extern s32 g_playerProgressAbs;
#else
#define g_playerProgressAbs g_playerProgress
#endif

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
 *      latches at D_152C00 (= g_levelDialogToc+0x13B0) [6] and [12], and drop
 *      the 0x200 pending bit in g_gameStateFlags (= g_nSaveLoadStatusCode[1]).
 *  No parameters, no return value.
 *  MATCHED on the s136os arm (task #1636; base body NOTE #9328, task #1631).
 *  What the ROM's shape needs, each measured necessary:
 *    - PumpDialogVoiceSystem declared value-returning (s16, its definition's
 *      type; it returns in $2 — FACT #9329; `void` reads 2/62);
 *    - the disc TOC held as one base pointer, the image pointer formed before
 *      the first memcpy, the area record held in a local, g_playerProgress
 *      stored through the size-16 equate (absolute, see the device above) —
 *      NOTE #9328's phrasing;
 *    - the dialog latches addressed through D_152C00, the ROM's own symbol,
 *      declared as an unsized array: cc1 then splits its %hi/%lo and the
 *      scheduler interleaves `li $2,-1` between them as in the ROM. Spelled
 *      through the byte-sized g_levelDialogToc instead, each latch store is
 *      its own absolute `lui $at` + `sw` at +0x13C8/+0x13E0, with no `la`
 *      and no split (FACT #9344);
 *    - the armor/held-item ids stored non-volatile (cast), so the five
 *      independent stores schedule freely, listed with D_152C00[6] LAST:
 *      cc1 issues the last store of the group first (FACT #9254), giving the
 *      ROM's order [6], armor, flags, held, [12].
 *  The flag word is spelled g_gameStateFlags for the ROM's relocation name
 *  only; g_nSaveLoadStatusCode[1] assembles to the same words.
 *  Behaviour is master's #else: the five stores are to distinct objects and
 *  no call sits between them. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00299BF8)
S136OS_SLOT(func_00299BF8);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void * func_00283460(void *dst, const void *src, s32 nbytes);
extern void func_00299B18(u8 *image);
/* g_levelDialogToc + 0x13B0, the dialog-scene latch block, under the ROM's
 * relocation name; unsized so cc1 does not treat it as small (see above). */
#ifndef TARGET_NATIVE
extern s32 D_152C00[];
#else
#define D_152C00 ((s32 *)(&g_levelDialogToc + 0x13B0))
#endif
/* (end of this body's declarations) */
void func_00299BF8(void) {
    u8  scratch[0x28];
    u8 *toc = g_discToc;
    u8 *buf;
    u8 *image;
    u8 *area;

    func_00289398(*(s32 *)(toc + 0x344) << 11, &buf);
    PumpDialogVoiceSystem(1);
    StartFileLoadPumpingVoice(buf, *(s32 *)(toc + 0x340) + *(s32 *)(toc + 0x32C),
                              *(s32 *)(toc + 0x344));
    image = buf + *(s32 *)(buf + 0x10);

    /* preserve the g_bPalMode block across the destructive image restore */
    func_00283460(scratch, &g_bPalMode, 0x28);
    func_00299B18(image);
    func_00283460(&g_bPalMode, scratch, 0x28);

    g_playerProgressAbs = 0;
    area = g_areaTable;
    if (*(s32 *)(area + 0x17C) != 0) {
        *(s32 *)(area + 0x17C) = 0;
    }
    if (*(s16 *)(area + 0x18) >= 0) {
        *(s16 *)(area + 0x18) = -1;
    }
    *(s32 *)&g_loadedArmorVariant = -1;
    g_gameStateFlags &= ~0x200;
    *(s32 *)&g_loadedHeldItemModelId = -1;
    D_152C00[12] = -1;
    D_152C00[6] = -1;       /* last in source = issued first (FACT #9254) */
}
#endif

/* SaveLoadStateMachine: the 0x1EA0-byte memory-card transaction state machine
 * (the largest function in the unit). Multi callee-save + libmc call graph;
 * 8-byte-packed callee-save frame wall, see func_0029C678. Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SaveLoadStateMachine);

/* BuildSaveImage(out): assemble the in-RAM save image from the two section
 * tables. Writes the serialized sizes of the global and area tables as the two
 * leading header words (out[0], out[1]), then serializes the global table
 * (slot 0) and all 0x1C area slots after the header, each image placed
 * directly after the previous one. No return value.
 *
 * Built on the s136os arm (task #1691). `out` itself is the write cursor: the
 * ROM advances the parameter's register (`addiu $18,$18,8`, then `addu` by each
 * written size). A separate `u8 *p` cursor gets its own register and changes
 * the allocation of every callee-saved value. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_BuildSaveImage)
S136OS_SLOT(BuildSaveImage);
#else
extern s32 SerializeSaveSections(void *dst, s32 slotMul, SaveSection *table); /* defined below */

void BuildSaveImage(s32 *out) {
    s32 i;

    out[0] = CalcSaveSectionsSize(g_saveSectionTableGlobal);
    out[1] = CalcSaveSectionsSize(g_saveSectionTableArea);

    out += 2;
    out = (s32 *)((u8 *)out + SerializeSaveSections(out, 0, g_saveSectionTableGlobal));
    for (i = 0; i < 0x1C; i++) {
        out = (s32 *)((u8 *)out + SerializeSaveSections(out, i, g_saveSectionTableArea));
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

/* ComputeSaveSectionsCrc16(buf, len): MSB-first (non-reflected) CRC-16 over
 * `len` bytes of a save payload. The accumulator is seeded with 0xEDB88320
 * (only its low 16 bits matter), each byte is XORed into bits 8..15, and each of
 * the 8 steps shifts left and XORs 0x1F45 when bit 15 was set. Returns the low
 * 16 bits, or 0 when `len` exceeds CalcSaveSectionsSize(g_saveSectionTableGlobal).
 *
 * Built on the s136os arm (task #1691); the old "packed save frame wall" label
 * held only for the 2.9 arm. The next byte pointer is formed before the bit
 * loop (`next`), as the ROM does (`addiu $4,$16,1` ahead of the loop and
 * `daddu $16,$4,$0` after it). A trailing `data++` or `*data++` instead
 * increments $16 in place. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_ComputeSaveSectionsCrc16)
S136OS_SLOT(ComputeSaveSectionsCrc16);
#else
s32 ComputeSaveSectionsCrc16(void *buf, s32 len) {
    u8 *data = (u8 *)buf;
    u8 *end;
    u32 crc;

    if (CalcSaveSectionsSize(g_saveSectionTableGlobal) < len) {
        return 0;
    }
    end = data + len;
    crc = 0xEDB88320;
    while (data < end) {
        u8 *next = data + 1;
        s32 i;
        crc ^= (u32)(*data << 8);
        for (i = 7; i >= 0; i--) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1F45;
            } else {
                crc = crc << 1;
            }
        }
        data = next;
    }
    return crc & 0xFFFF;
}
#endif

extern s32 ComputeSaveSectionsCrc16(void *buf, s32 len); /* save-buffer CRC */

/* VerifySaveHeaderChecksum(image): verify a save image's stored CRC. The image header is
 * { s32 payloadLen; s32 storedCrc; payload[payloadLen] } (the layout written by
 * SerializeSaveSections, which stores payloadLen at +0 and the CRC at +4).
 * Recomputes the CRC over the payload (ComputeSaveSectionsCrc16 from image+8 over
 * payloadLen bytes) and returns 1 iff it equals the stored CRC. An image whose
 * stored CRC is 0 is treated as empty/invalid and returns 0.
 *
 * Built on the s136os arm (task #1678): the 8-byte-packed save frame the 2.9
 * arm could not give is -fopt-stack's. The bytes depend on the spelling: the
 * result is a local preset to 0 and payloadLen is read before the test, which
 * gives the ROM's up-front `daddu $2,$0,$0` and the `lw $5` in the beqz delay
 * slot. An early `return 0` puts the zero after the call instead. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_VerifySaveHeaderChecksum)
S136OS_SLOT(VerifySaveHeaderChecksum);
#else
s32 VerifySaveHeaderChecksum(void *image) {
    s32 valid = 0;
    s32 storedCrc = ((s32 *)image)[1];
    s32 len = ((s32 *)image)[0];
    if (storedCrc != 0) {
        valid = ComputeSaveSectionsCrc16((char *)image + 8, len) == storedCrc;
    }
    return valid;
}
#endif

extern void FillMemory32(void *dst, s32 pattern, s32 nbytes);
extern void *func_00283460(void *dst, const void *src, s32 nbytes); /* memcpy */
#ifndef TARGET_NATIVE
extern int memcmp(); /* K&R decl: avoids the ee-gcc builtin-prototype conflict warning */
#endif /* native takes <string.h>'s prototype (common.h); C++ rejects the K&R redeclaration */

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
 * per-slot payload source is `srcPtr + slot*len` (each slot's data is packed
 * contiguously). Sections tagged 0x1770 are zero-filled rather than copied. A
 * terminator header { -1, 0 } closes the stream; the CRC over the whole payload
 * (from dst+8, payloadLen bytes) is then stored in the header.
 *
 * @param dst    save-image buffer (header + section stream)
 * @param slot   memory-card slot; selects the slice of each section's payload
 * @param table  section descriptors, terminated by a NULL srcPtr
 * @return       total image size (payloadLen + 8)
 *
 * Built on the s136os arm (task #1738); the old "packed save frame wall" label
 * held only for the 2.9 arm. Three spellings carry the bytes:
 *  - The payload source is formed before the header stores, from a fresh read
 *    of `len`. The ROM reads srcPtr and len ahead of `sw tag` and re-reads len
 *    after it for the header (the store may alias). Forming it inside the else
 *    arm, or from a cached len, moves the mult and the loads.
 *  - The walk uses its own descriptor pointer `sec`, set inside the if. The ROM
 *    tests the first srcPtr through the incoming $6 and copies it to $16 only
 *    on the taken path, which also places the $16 save after $21's.
 *  - The advance adds len to cursor and size first, then rounds both. Rounding
 *    `(x + len + 3) & -4` in one expression reorders the adds. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_SerializeSaveSections)
S136OS_SLOT(SerializeSaveSections);
#else
s32 SerializeSaveSections(void *dst, s32 slot, SaveSection *table) {
    s32 *cursor = (s32 *)((char *)dst + 8);
    s32 size = 0;

    if (table->srcPtr != 0) {
        SaveSection *sec = table;
        do {
            s32 len;
            char *src = (char *)sec->srcPtr + slot * sec->len;

            size += 8;
            cursor[0] = sec->tag;
            cursor[1] = sec->len;
            cursor += 2;
            if (sec->tag == 0x1770) {
                FillMemory32(cursor, 0, sec->len);
            } else {
                func_00283460(cursor, src, sec->len);
            }
            len = sec->len;
            sec++;
            cursor = (s32 *)((char *)cursor + len);
            size += len;
            cursor = (s32 *)(((s32)cursor + 3) & -4);
            size = (size + 3) & -4;
        } while (sec->srcPtr != 0);
    }

    size += 8;
    cursor[1] = 0;
    cursor[0] = -1;
    ((s32 *)dst)[1] = ComputeSaveSectionsCrc16((char *)dst + 8, size);
    ((s32 *)dst)[0] = size;
    return size + 8;
}
#endif

/* FillSaveSlotInfo(image, slot, dir): fill one entry of the save-slot info
 * display table (g_saveSlotInfoTable 0x139410 = g_areaTable+0x30; slot stride
 * 0xA0, dir stride 0x1C) from a save image. Stores whether the image fails
 * VerifySaveHeaderChecksum, then copies fields from the payloads of the first
 * four global save sections: g_playerProgress, the g_boltCount block (word 0
 * and byte 0x12), D_1A7BC8, and the 8-byte D_1A7360. No return value.
 *
 * Built on the s136os arm (task #1691). The old "ldl/ldr cannot come from C"
 * label is wrong: a byte-aligned 8-byte struct copy gives them. Two spellings
 * carry the bytes:
 *  - The entry is indexed afresh in each statement through a true array
 *    (g_areaSaveSlotRecords, an asm-label alias of g_areaTable). This gives the
 *    ROM's single `slot*0xA0 + dir*0x1C` offset and the four `daddu` copies of
 *    the entry address. A pointer cast of g_areaTable adds the slot part first;
 *    a local entry pointer gives no copies.
 *  - `image` itself walks from payload to payload. The ROM chains each address
 *    off the previous one (+0x10, +0xC, +0x48). A separate cursor local folds
 *    back to constant offsets from `image`. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_FillSaveSlotInfo)
S136OS_SLOT(FillSaveSlotInfo);
#else
typedef struct { u8 bytes[8]; } SaveSlotInfoBlock8; /* byte-aligned: ldl/ldr, sdl/sdr */
typedef struct {
    s32 level;                /* +0x00 */
    s32 bolts;                /* +0x04 */
    s32 byte12;               /* +0x08 */
    s32 word;                 /* +0x0C */
    SaveSlotInfoBlock8 block; /* +0x10 */
    s32 invalid;              /* +0x18 */
} SaveSlotInfo;               /* 0x1C */
typedef struct {
    u8 head[0x30];
    SaveSlotInfo saveSlots[4]; /* +0x30 = g_saveSlotInfoTable for record 0 */
} AreaSaveSlotRecord;          /* 0xA0, the g_areaTable stride */
#ifndef TARGET_NATIVE
extern AreaSaveSlotRecord g_areaSaveSlotRecords[] __asm__("g_areaTable");
#else
#define g_areaSaveSlotRecords ((AreaSaveSlotRecord *)g_areaTable)
#endif
extern s32 VerifySaveHeaderChecksum(void *image);

void FillSaveSlotInfo(u8 *image, s32 slot, s32 dir) {
    g_areaSaveSlotRecords[slot].saveSlots[dir].invalid =
        (VerifySaveHeaderChecksum(image) == 0);
    image += 0x10;                          /* section 0: g_playerProgress */
    g_areaSaveSlotRecords[slot].saveSlots[dir].level = *(s32 *)image;
    image += 4 + 8;                         /* section 1: g_boltCount block */
    g_areaSaveSlotRecords[slot].saveSlots[dir].bolts = *(s32 *)image;
    g_areaSaveSlotRecords[slot].saveSlots[dir].byte12 = image[0x12];
    image += 0x40 + 8;                      /* section 2: D_1A7BC8 */
    g_areaSaveSlotRecords[slot].saveSlots[dir].word = *(s32 *)image;
    g_areaSaveSlotRecords[slot].saveSlots[dir].block =
        *(SaveSlotInfoBlock8 *)(image + 4 + 8); /* section 3: D_1A7360 */
}
#endif

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
 * Steps: (1) CRC-verify via VerifySaveHeaderChecksum — a bad image returns 1 immediately.
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
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 VerifySaveHeaderChecksum(void *image);
/* (end of this body's declarations) */
/* t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 53.57% -> PACKED-SAVE, first differing row @0: ROM `addiu sp, sp, -0x60` vs `addiu sp, sp, -0xb0`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 50.64% -> SCHED-PROEPI, first differing row @1: ROM `sd s1, 0x18(sp)` vs `sd s0, 0x10(sp)`. */
s32 DeserializeSaveSections(void *image, s32 slotMul, SaveSection *table) {
    s32 *section;        /* current image section header { tag, len, payload... } */
    s32 mismatchCount;
    s32 runningOffset;
    s32 sentinel;
    SaveSection *entry;

    if (VerifySaveHeaderChecksum(image) == 0) {
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
/* t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 58.56% -> PACKED-SAVE, first differing row @0: ROM `addiu sp, sp, -0x30` vs `addiu sp, sp, -0x50`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 47.70% -> SCHED, first differing row @1: ROM `(nothing)` vs `lui v1, %hi(g_gsPixelOffsetY+0xc)`. */
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

/*
 * func_0029C448 — forward two args to the widget at g_guiInstance+0x36F28
 * (method func_00339398) when the GUI is up; same shape as func_0029DAD0.
 * Params: a, b — forwarded unchanged. Returns the method's result; when the
 * GUI is down the ROM falls off the end without setting $v0, as this does.
 *
 * Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888) with the gui pointer
 * pinned to $v0. Unpinned, cc1 2.9 emits the ROM's instructions with the gui
 * load in $v1 and the copy of `a` in $v0 (97.67%, unit objdiff), while the
 * ROM colours this function the other way round from func_0029DAD0. The old
 * comment's "no source shape flips it" was UNTESTED against a register pin;
 * the pin flips it.
 */
extern s32 func_00339398(char *widget, s32 a, s32 b);
s32 func_0029C448(s32 a, s32 b) {
    register char *gui EE_REG("$2") = g_guiInstance;
    if (gui != 0) {
        return func_00339398(gui + 0x36F28, a, b);
    }
}

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C500);

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

/* The two-entry (a,b) pair table at g_guiInstance+0x3FA18, written by
 * func_0029C5B0 and cleared by func_0029C600. Only the table is modelled; the
 * rest of the 0x3FB20-byte manager is padding here. */
typedef struct {
    s32 a;
    s32 b;
} GuiPendingPair;

typedef struct {
    char _unk0[0x3FA18];
    GuiPendingPair pairs[2];
} GuiPendingPairs;

/** func_0029C5B0 — store the (a,b) pair into entry `idx` of the GUI manager's
 *  two-entry pending-pair table (g_guiInstance+0x3FA18).
 *  @param a    first word, stored at +0x0 of the entry
 *  @param b    second word, stored at +0x4
 *  @param idx  entry index; anything outside 0..1 (unsigned compare) is ignored
 *  @return 1 if stored, 0 when the GUI is down or idx is out of range.
 *  Built on the s136os arm (task #1668). Each field is written through its own
 *  `pairs[idx]` access: cc1 legitimises each large-offset address separately
 *  (base+0x38000, then +0x7A18/+0x7A1C) and CSE leaves the ROM's copy of the
 *  base ($3 = $4) between them. A shared slot pointer has one base register
 *  and loses that copy (the "coloring wall" recorded here before #1668). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029C5B0)
S136OS_SLOT(func_0029C5B0);
#else
s32 func_0029C5B0(s32 a, s32 b, s32 idx) {
    char *gui = g_guiInstance;
    if (gui != 0) {
        if ((u32)idx < 2) {
            ((GuiPendingPairs *)gui)->pairs[idx].a = a;
            ((GuiPendingPairs *)gui)->pairs[idx].b = b;
            return 1;
        }
    }
    return 0;
}
#endif

/** func_0029C600 — clear entry `idx` of the GUI manager's pending-pair table
 *  (g_guiInstance+0x3FA18); the counterpart of func_0029C5B0.
 *  @param idx  entry index; anything outside 0..1 (unsigned compare) is ignored
 *  Built on the s136os arm (task #1668). g_guiInstance is read straight from
 *  the global in each access: cached in a typed local first, cc1 allocates
 *  the sum and the copy to $2 instead of the ROM's $3/$5/$3 (measured). The
 *  read fills the range check's delay slot, where the .extern 12 band (header)
 *  assembles it as the ROM's one-insn %gp_rel load. The per-field accesses
 *  give the base copy, as in func_0029C5B0. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029C600)
S136OS_SLOT(func_0029C600);
#else
void func_0029C600(s32 idx) {
    if ((u32)idx < 2) {
        ((GuiPendingPairs *)g_guiInstance)->pairs[idx].a = 0;
        ((GuiPendingPairs *)g_guiInstance)->pairs[idx].b = 0;
    }
}
#endif

/* func_0029C638: 16 bytes of dead inter-function fill (`li $v0,0` + epilogue
 * orphan, no jr) — not compiler-reachable C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C638);

/** Forward to the widget at g_guiInstance+0x36F28 (method func_0034F200). */
s32 func_0029C648(void) {
    if (g_guiInstance != 0) {
        return func_0034F200(g_guiInstance + 0x36F28);
    }
}

/* func_0029C678 (and C700/C818/C8F0/CA98/CC48/CD18): 2+-callee-save functions
 * walled under cc1 2.9 by the 8-byte-packed callee-save layout of the later SN
 * cc1 (cc1 2.9 reserves 16 bytes per save) - the wall characterized in the
 * 1907F0 round. FACT #8810: that layout is SN 2.95.3 v1.36 -fopt-stack's, which
 * closes func_0029C678 below (task #1269); the other functions that cite this
 * note stay asm until each is measured under it.
 *
 * func_0029C678: when the GUI singleton exists, dispatch a text-box render
 * through func_00338F88 with the GUI's text-box context (g_guiInstance+0x36F28)
 * prepended, forwarding its 10 args verbatim (args 1-7 in $4-$10, arg8 in $11,
 * args 9-10 on the incoming stack). func_00338F88's 11-arg prototype recovered
 * via Ghidra.
 * MATCHED on the s136os arm (FACT #8830): the packed save is SN 2.95.3 v1.36
 * -fopt-stack's.
 * GUARD (task #1269): on EE this C is the image's body, compiled alone by SN
 * 2.95.3 v1.36 -fopt-stack (tools/ee/s136os_functions.txt) and spliced over the
 * S136OS_SLOT line by tools/ee/s136os_splice.sh; the 2.9 compile sees only the
 * slot, so a build that skips the splice loses the function. On native it is
 * plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029C678)
S136OS_SLOT(func_0029C678);
#else
/* Record, t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 99.48% -> PACKED-SAVE, first differing row @0: ROM `addiu sp, sp, -0x30` vs `addiu sp, sp, -0x40`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 65.27% -> SCHED-PROEPI, first differing row @0: ROM `addiu sp, sp, -0x30` vs `daddu t4, a0, zero`. */
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

/* func_0029C700: refresh the four front-end menu buttons of the widget at
 * g_guiInstance+0x3F7B0 (called by func_0029C818 when the screen becomes
 * ready). For each button i, FindChainTailFreeSlot walks chain D_1A9AD0[i]
 * of the 0x28-stride record table D_262920. When it yields a record, the
 * button takes that record's label id and a colour (0x6029A1FF when the
 * record's count is positive, else 0x60F0F0B0) and is set to state 0;
 * otherwise the button is set to state 2. No-op when the GUI is down. No
 * params, no return value.
 *
 * Built on the s136os arm (task #1738). Both tables are indexed as typed
 * struct arrays, not through byte offsets: the ROM hoists D_262920+0x24 and
 * D_262920 as two separate loop bases, and walks the buttons with a single
 * pointer at +8 (`sw label,-4($18)`, `sw colour,0($18)`). A u8* cast of
 * D_256398 gives two pointers and %lo-folded offsets (13 words differ).
 * g_menuButtons is a bare asm-label alias of D_256398 (as FillSaveSlotInfo's
 * g_areaSaveSlotRecords): func_0029C818's arm declares D_256398 as u8[], and
 * the native TU sees both arms. D_1A9AD0 is declared as a 4-byte object so
 * cc1 forms its address %gp_rel, as the ROM does (`addiu $20,$28,...`). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029C700)
S136OS_SLOT(func_0029C700);
#else
typedef struct {
    u8  _pad00[0x1C];
    s16 labelId;      /* 0x1C */
    u8  _pad1E[6];
    s32 count;        /* 0x24 */
} ChainRecord;        /* 0x28, the D_262920 table */
typedef struct {
    s32 _pad00;
    s32 labelId;      /* 0x04 */
    u32 color;        /* 0x08 */
    u8  _pad0C[0xC];
} MenuButton;         /* 0x18, the D_256398 entries */
extern s32  FindChainTailFreeSlot(s32 head);
extern void func_0033BA18(void *widget, s32 index, s32 state);
extern s32  D_1A9AD0;              /* first of 4 chain heads (gp small-data) */
extern ChainRecord D_262920[];
#ifndef TARGET_NATIVE
extern MenuButton g_menuButtons[] __asm__("D_256398");
#else
extern u8 D_256398[];
#define g_menuButtons ((MenuButton *)D_256398)
#endif

void func_0029C700(void) {
    s32 i;

    if (g_guiInstance != 0) {
        for (i = 0; i < 4; i++) {
            s32 slot = FindChainTailFreeSlot((&D_1A9AD0)[i]);

            if (slot >= 0) {
                g_menuButtons[i].labelId = D_262920[slot].labelId;
                g_menuButtons[i].color = (D_262920[slot].count > 0) ? 0x6029A1FF : 0x60F0F0B0;
                func_0033BA18(g_guiInstance + 0x3F7B0, i, 0);
            } else {
                func_0033BA18(g_guiInstance + 0x3F7B0, i, 2);
            }
        }
    }
}
#endif

/* func_0029C818: front-end screen readiness poll. No-op unless the GUI is up.
 * The new ready state is func_0026F7D8() != 0 when func_0026F7D0() is set,
 * else 0; if it equals the latched gate D_1A9A8C nothing happens. On a change
 * it invalidates the GUI state (func_0029CA88) and latches the new value, and
 * only on a transition to 1 configures the front-end screen: primes
 * g_guiInstance+0x3CD48 with D_1A9A98, builds the button row of the widget at
 * g_guiInstance+0x3F7B0 (func_0033BA48 from D_1A9AC0 / D_256398), sets its
 * scale to 36.0 and its index to -1, then refreshes the menu items
 * (func_0029C700). No params, no return value. EU twin func_0029C3C0.
 *
 * Built on the s136os arm (task #1722), plain C, no devices. D_1A9A98 and
 * D_1A9AC0 are declared as 4-byte objects so cc1 forms their addresses
 * %gp_rel, as the ROM does (`addiu $5,$28,...`). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029C818)
S136OS_SLOT(func_0029C818);
#else
extern s32  func_0026F7D0(void);
extern s32  func_0026F7D8(void);
extern void func_0034A1D0(void *w, s32 *v);
extern void func_0033BA48(void *w, s32 *srcGlyphs, void *entries);
extern void func_0033BA10(void *p, f32 v);
extern void func_0033BA28(void *p, s32 idx);
extern void func_0029C700(void);
extern void func_0029CA88(void);
extern s32  D_1A9A98;
extern s32  D_1A9AC0;
extern u8   D_256398[];

void func_0029C818(void) {
    s32 ready;

    if (g_guiInstance != 0) {
        ready = 0;
        if (func_0026F7D0() != 0) {
            ready = (func_0026F7D8() != 0);
        }
        if (D_1A9A8C != ready) {
            func_0029CA88();
            D_1A9A8C = ready;
            if (ready == 1) {
                func_0034A1D0(g_guiInstance + 0x3CD48, &D_1A9A98);
                func_0033BA48(g_guiInstance + 0x3F7B0, &D_1A9AC0, D_256398);
                func_0033BA10(g_guiInstance + 0x3F7B0, 36.0f);
                func_0033BA28(g_guiInstance + 0x3F7B0, -1);
                func_0029C700();
            }
        }
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C8F0);

/** Invalidate the D_1A9A90 GUI state (set to -1). */
void func_0029CA88(void) {
    D_1A9A90 = -1;
}

/* func_0029CA98: the GUI pump, run each frame through func_0029DC70. Skipped
 * while the HUD-busy word (g_hudClutSlots+0x10) is set. With the GUI up it
 * polls the front-end menu (func_0029C8F0), then offers the frame to the
 * manager's pre-tick hook, its four handler hooks and its two pending-pair
 * hooks (g_guiInstance+0x3F9F0/+0x3F9F8/+0x3FA18); any nonzero result ends
 * the frame there. Otherwise, unless the game is in state 4 (menu) with an
 * overlay mode other than 7, it runs the manager's tick (+0x3F9D8) and
 * func_0029C648.
 *
 * Return: none in the ROM. The definition is void: the delay slot of the
 * final `beqz gui` is filled from the fall-through (`lui $2`), which cc1 does
 * only when $v0 is dead at the exit; an s32 definition steals the epilogue's
 * `ld $16` instead. func_0029DC70 nevertheless propagates $v0 through an s32
 * declaration, so the ROM hands it whatever the last callee left. The native
 * arm is s32 and returns 0, a defined value where the ROM's is not.
 *
 * Built on the s136os arm (task #1738). Each hook is read through its own
 * struct access on g_guiInstance, once to test and once to call: the ROM
 * forms two addresses per hook (gui+0x38000+i*8 for the test, i*8+gui+
 * 0x3F9F8 for the call), as func_0029C600's pair stores do (FACT #9446).
 * Single exit (`goto done`) so one body serves both arms (native is s32). */
extern u8 g_hudClutSlots[];   /* only +0x10, the HUD-busy word at 0x1B1828, is read */
/* ADDRESSING-MODEL DEVICE (RULING #8620; the FACT #8036 size-16 equate form,
 * as g_objectiveScanHeadAbs): the ROM reads the HUD-busy word with the
 * assembler-macro shape `lui $2; lw $2,%lo(g_hudClutSlots+0x10)($2)` after
 * the frame setup. Through g_hudClutSlots cc1 splits it into a separate lui
 * temp scheduled above `addiu $sp`. Emits nothing; the relocation names
 * g_hudClutSlots+0x10. */
#ifndef TARGET_NATIVE
__asm__(".extern g_hudBusyAbs, 16\n\tg_hudBusyAbs = g_hudClutSlots + 0x10");
extern s32 g_hudBusyAbs;
#else
#define g_hudBusyAbs (*(s32 *)(g_hudClutSlots + 0x10))
#endif
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029CA98)
S136OS_SLOT(func_0029CA98);
#else
typedef struct {
    s32 (*fn)(void);            /* called with no arguments; nonzero = handled */
    s32 arg;
} GuiHookSlot;

typedef struct {
    char _unk0[0x3F9D8];
    void (*tick)(void);         /* 0x3F9D8 per-frame tick */
    char _unk3F9DC[0x14];
    s32 (*preTick)(void);       /* 0x3F9F0 nonzero result skips the frame */
    s32 _unk3F9F4;
    GuiHookSlot handlers[4];    /* 0x3F9F8 */
    GuiHookSlot pending[2];     /* 0x3FA18, the GuiPendingPairs table */
} GuiTickHooks;

extern s32  func_0029C8F0(void);
extern s32  func_0029C648(void);
extern s32  GetMenuOverlayModeLive(void);

#ifndef TARGET_NATIVE
void func_0029CA98(void)
#else
s32 func_0029CA98(void)
#endif
{
    s32 any;
    s32 i;

    if (g_hudBusyAbs != 0) {
        goto done;
    }
    if (g_guiInstance != 0) {
        func_0029C8F0();
        if (((GuiTickHooks *)g_guiInstance)->preTick != 0 &&
            ((GuiTickHooks *)g_guiInstance)->preTick() != 0) {
            goto done;
        }
        any = 0;
        for (i = 0; i < 4; i++) {
            if (((GuiTickHooks *)g_guiInstance)->handlers[i].fn != 0) {
                any |= ((GuiTickHooks *)g_guiInstance)->handlers[i].fn();
            }
        }
        if (any != 0) {
            goto done;
        }
        for (i = 0; i < 2; i++) {
            if (((GuiTickHooks *)g_guiInstance)->pending[i].fn != 0) {
                any |= ((GuiTickHooks *)g_guiInstance)->pending[i].fn();
            }
        }
        if (any != 0) {
            goto done;
        }
    }
    if (g_nGameState != 0 && g_nGameState != 5 && g_nGameState == 4) {
        if (GetMenuOverlayModeLive() != 7) {
            goto done;
        }
    }
    if (g_guiInstance != 0) {
        ((GuiTickHooks *)g_guiInstance)->tick();
        func_0029C648();
    }
done:
#ifdef TARGET_NATIVE
    return 0;
#endif
    ;
}
#endif

/* func_0029CC48: rebuild the camera projection with a temporary FOV override.
 * When the GUI singleton exists, force the projection scale at +0xB0 of the
 * camera/projection scratch (g_sceneActorMobys+0x674, i.e. g_cameraProjScale
 * 0x1B9070) to 0.62, rebuild the projection, run the GUI camera hook
 * (func_0034F220 on g_guiInstance+0x36F28), then restore the saved value and
 * rebuild again. No params, no return value.
 *
 * Built on the s136os arm (task #1678). The scratch base is its own local: the
 * ROM keeps the full address `g_sceneActorMobys+0x674` in $16 across both calls
 * and addresses 0xB0($16). Spelled as one `+0x724` pointer, cc1 keeps only the
 * %hi half in $16 and puts %lo on each of the three accesses. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029CC48)
S136OS_SLOT(func_0029CC48);
#else
extern u8   g_sceneActorMobys[];               /* +0x674 = camera/projection scratch */
extern void BuildCameraProjection(void);
extern void func_0034F220(void *guiCameraCtx);

void func_0029CC48(void) {
    if (g_guiInstance != 0) {
        u8  *cam = g_sceneActorMobys + 0x674;
        f32 *projScale = (f32 *)(cam + 0xB0);
        f32  saved = *projScale;
        *projScale = 0.62f;
        BuildCameraProjection();
        func_0034F220(g_guiInstance + 0x36F28);
        *projScale = saved;
        BuildCameraProjection();
    }
}
#endif

/*
 * func_0029CCB8 — GUI popup-poll gate. When the GUI is up and every gating
 * flag permits (D_1A9A88 set, D_18A000 = the s16 sub-state half at
 * g_nNanotechBonusHealTimer+4 clear, the popup-busy gate D_1A8C64 clear,
 * D_1A9A8C set, g_guiInstance non-null), forwards the widget at
 * g_guiInstance+0x3F7B0 to func_0033B720. No params, no return value.
 *
 * Byte-exact on sdk29 (-O2 -G8 -fno-gcse, task #888). Two things carry it:
 *  - the empty asm after the call keeps cc1 2.9 from turning it into a
 *    sibling jump (the ROM's compiled code never tail-jumps, FACT #8177);
 *  - g_guiInstance's `.extern` marker is 12, not 16. The ROM reads it as a
 *    one-insn %gp_rel in the last beqz's delay slot, and asm_unit.sh's -G8
 *    awk has rewritten a delay-slot access in the 9..15 band to exactly that
 *    since 2026-06-12 (FACT #7461). #511's comment here called that a
 *    missing tree instrument; it was not missing, the marker was 16. #656
 *    then held it back because func_0033B720 would enter ORPHAN_LATENT; the
 *    symbol_addrs line for func_0033B720 settles that.
 */
void func_0029CCB8(void) {
    /* All five gates must permit before the popup-poll runs:
     *  D_1A9A88 set, the nanotech sub-state half at +0x4 clear, the popup-busy
     *  gate D_1A8C64 clear, D_1A9A8C set, and the GUI instance up. */
    if (D_1A9A88 == 0) {
        return;
    }
    if (D_18A000 != 0) {
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
    __asm__ __volatile__(""); /* sibling-call suppression (the ROM never sibcalls) */
}

/* func_0029CD18: the GUI draw pump, the draw-side twin of func_0029CA98, called
 * once per frame from RenderFrame and RenderMenuScreenWidgets. Skipped while
 * the HUD-busy word (g_hudClutSlots+0x10) is set, and in the menu state when
 * the overlay mode is 8. With the GUI up it runs the full-screen tint pump
 * (func_00290EF8) and the popup-poll gate (func_0029CCB8), then offers the
 * frame to the manager's pre-draw hook (+0x3F9F4) and to the draw half of its
 * four handler slots and two pending-pair slots (+0x3F9F8/+0x3FA18, the same
 * slots func_0029CA98 ticks). If any hook drew, it re-runs the FOV-override
 * projection rebuild (func_0029CC48) on progress levels 8 and 0x13 when
 * func_0026F7E0(2) allows it, and ends the frame. Otherwise, unless the game is
 * in state 4 with an overlay mode other than 7, it runs the manager's draw
 * (+0x3F9D4) and func_0029CC48. No params, no return value. The store has
 * this address under the former name DrawActiveGuiScreens.
 *
 * Built on the s136os arm (task #1745) as plain C at the unit's flags. It is
 * func_0029CA98's body with the draw hooks: each hook is read through its own
 * struct access, once to test and once to call, because the ROM forms two
 * addresses per hook (gui+0x38000+i*8 for the test, i*8+gui+0x3F9F8 for the
 * call). The repeated `any != 0` test is the ROM's: it re-tests after the
 * progress-level block (`bnel $19`) instead of threading the jump.
 * func_0029CC48 is declared here because its own definition sits in its
 * guarded s136os arm, which this member's s136os TU does not open. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029CD18)
S136OS_SLOT(func_0029CD18);
#else
typedef struct {
    s32 (*tick)(void);          /* called by func_0029CA98 */
    s32 (*draw)(void);          /* called with no arguments; nonzero = drew */
} GuiDrawHookSlot;

typedef struct {
    char _unk0[0x3F9D4];
    void (*draw)(void);         /* 0x3F9D4 per-frame draw */
    char _unk3F9D8[0x1C];
    s32 (*preDraw)(void);       /* 0x3F9F4 nonzero result counts as drawn */
    GuiDrawHookSlot handlers[4]; /* 0x3F9F8 */
    GuiDrawHookSlot pending[2];  /* 0x3FA18, the GuiPendingPairs table */
} GuiDrawHooks;

extern s32  GetMenuOverlayMode(void);
extern s32  GetMenuOverlayModeLive(void);
extern s32  func_00290EF8(void);
extern s32  func_0026F7E0(s32 arg);
extern void func_0029CC48(void);

void func_0029CD18(void) {
    s32 any;
    s32 i;

    if (g_hudBusyAbs != 0) {
        return;
    }
    if (g_guiInstance != 0) {
        if (g_nGameState == 4 && GetMenuOverlayMode() == 8) {
            return;
        }
        any = 0;
        func_00290EF8();
        func_0029CCB8();
        if (((GuiDrawHooks *)g_guiInstance)->preDraw != 0) {
            any = ((GuiDrawHooks *)g_guiInstance)->preDraw() != 0;
        }
        for (i = 0; i < 4; i++) {
            if (((GuiDrawHooks *)g_guiInstance)->handlers[i].draw != 0) {
                any |= ((GuiDrawHooks *)g_guiInstance)->handlers[i].draw();
            }
        }
        for (i = 0; i < 2; i++) {
            if (((GuiDrawHooks *)g_guiInstance)->pending[i].draw != 0) {
                any |= ((GuiDrawHooks *)g_guiInstance)->pending[i].draw();
            }
        }
        if (any != 0) {
            if (g_playerProgress == 8 || g_playerProgress == 0x13) {
                if (func_0026F7E0(2) != 0) {
                    func_0029CC48();
                }
            }
            if (any != 0) {
                return;
            }
        }
    }
    if (g_nGameState != 0 && g_nGameState != 5 && g_nGameState == 4) {
        if (GetMenuOverlayModeLive() != 7) {
            return;
        }
    }
    if (g_guiInstance != 0) {
        ((GuiDrawHooks *)g_guiInstance)->draw();
        func_0029CC48();
    }
}
#endif

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

/** Tick the map screen at g_guiInstance+0x39AF0 (GuiMapScreenTick,
 *  func_00346EF0): `buttons` is the pad-button mask, `out` receives the
 *  state the tick latches (write-only). Returns the tick's result (0), or 0
 *  when the GUI is down. */
s32 func_0029D080(s32 buttons, s32 *out) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00346EF0(g_guiInstance + 0x39AF0, buttons, out);
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D988);

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
 * (g_guiInstance). Poisons the GUI heap (memset 0xCD over
 * 0x40000 bytes at g_memoryArenaTable[0x80]), clears the input/pad state window
 * (D_138180 + 0x1A0..0x1CC), runs the mode init (func_0029DB58), placement-news +
 * inits the instance (GuiPlacementNew / GuiSystemInit), stores it to
 * g_guiInstance, then installs the two update callbacks at instance+0x3F9D4/
 * +0x3F9D8 — the special pair (func_0029CF08/func_0029DB50) when g_playerProgress
 * == 0x1F5, else the default pair (func_0029CF40/func_0029CF10).
 *
 * MATCHED on the s136os arm (task #1405): byte-exact image-resident under SN
 * 2.95.3 v1.36 -fopt-stack (verify_match_unit 60/60 words + image cmp 0).
 * Body found by task #1396 (FACT #8830 solo screen).
 * Levers, each undone alone: the callback stores inside each branch (25/60);
 * the size-16 g_playerProgress equate (26/60); g_bPalMode passed to
 * func_0029DB58 (50/60); the heap pointer re-read at each use (8/60); the
 * clears in address order (12/60); D_138180 taken after the memset (60/60,
 * the base's own first diff). */

/* GUARD (task #1405): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiManagerCreate)
S136OS_SLOT(GuiManagerCreate);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
#ifndef TARGET_NATIVE
extern void *memset(void *s, int c, unsigned int n); /* native: <string.h> via common.h */
#endif
/* (end of this body's declarations) */
/* t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 47.22% -> PACKED-SAVE, first differing row @0: ROM `addiu sp, sp, -0x10` vs `addiu sp, sp, -0x30`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 43.65% -> SCHED-PROEPI, first differing row @0: ROM `addiu sp, sp, -0x10` vs `addiu sp, sp, -0x20`. */
extern u8   D_138180[];              /* controller / input state block (0x138180) */
extern u8   g_memoryArenaTable[];    /* memory-arena table; +0x80 = GUI heap base ptr */
extern void *GuiPlacementNew(s32 size, void *heap);
extern void *GuiSystemInit(void *placement, s32 size);
extern void func_0029CF08(void);     /* progress==0x1F5 update callback pair */
extern void func_0029DB50(void);
/* func_0029DB58, func_0029CF40, func_0029CF10 are defined earlier in this unit. */

void GuiManagerCreate(void) {
    u8   *arena = g_memoryArenaTable;
    u8   *in;
    void *instance;

    /* the heap pointer is re-read at each use, as the ROM does (0x29DBA0/0x29DBE8) */
    memset(*(void **)(arena + 0x80), 0xCD, 0x40000);
    in = D_138180;
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
    *(s32 *)(in + 0x1CC) = 0;
#ifndef TARGET_NATIVE
    /* The ROM loads g_bPalMode into $a0 for this call (0x29DBA8); the
     * callee is (void) and ignores it. */
    ((void (*)(s32))func_0029DB58)(g_bPalMode);
#else
    func_0029DB58();
#endif
    instance = GuiSystemInit(GuiPlacementNew(0x3FB20, *(void **)(arena + 0x80)), 0x40000);
    g_guiInstance = (char *)instance;

    if (g_playerProgressAbs == 0x1F5) {
        *(void **)((u8 *)instance + 0x3F9D4) = (void *)func_0029CF08;
        *(void **)((u8 *)instance + 0x3F9D8) = (void *)func_0029DB50;
    } else {
        *(void **)((u8 *)instance + 0x3F9D4) = (void *)func_0029CF40;
        *(void **)((u8 *)instance + 0x3F9D8) = (void *)func_0029CF10;
    }
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
/* Not compiled in func_0029CA98's s136os TU (it calls func_0029CA98 for a
 * value, and that TU defines func_0029CA98 void). The 2.9 TU, where this
 * function is built, and the native build are unaffected. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029CA98)
s32 func_0029DC70(void) {
    if (g_guiInstance != 0) {
        if (D_1A8C64 == 0) {
            func_0028E9A0(1);
            return func_0029CA98();
        }
    }
}
#elif defined(TARGET_NATIVE)
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

/* func_0029DCB0: 8 bytes of dead pad (addiu $sp,0x10; nop) carved off the real
 * entry func_0029DCB8 in task #472. func_0029DCB8 maps g_playerProgress
 * (0x16->9, 0x17->0x12, else 0) and calls RequestLevelExit(.,1). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DCB0);

/** func_0029DCB8 — leave the current level for a fixed destination: level 0x16
 *  exits to 9, level 0x17 to 0x12, any other level to 0. g_playerProgress is
 *  the current level id. Calls RequestLevelExit(destination, 1), which commits a
 *  save. No params, no return value.
 *
 *  Built on the s136os arm (task #1678). The `switch` is what gives the ROM's
 *  beq/beql pair. The same mapping as an if/else-if chain compiles to a
 *  `xori`/`movn` select. g_playerProgress is read absolutely (lui/lw), through
 *  the unit's size-16 g_playerProgressAbs equate. */
extern void RequestLevelExit(s32 destination, s32 commitSave);
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029DCB8)
S136OS_SLOT(func_0029DCB8);
#else
void func_0029DCB8(void) {
    s32 destination = 0;
    switch (g_playerProgressAbs) {
    case 0x16:
        destination = 9;
        break;
    case 0x17:
        destination = 0x12;
        break;
    }
    RequestLevelExit(destination, 1);
}
#endif

/* Inter-function padding at 0x29DD00..0x29DD07: the two zero words retail
 * places between func_0029DCB8 and the func_0029DD08 fragment. They exist only
 * after `endlabel func_0029DCB8` in its nonmatchings .s, which is no longer
 * included now that the s136os arm supplies the body, so without this directive
 * every later function in the unit lands 8 bytes low (landing_gate cmp 610390 at
 * task #1678's first gate run). Not a codegen device: layout data (RULING #8467).
 * Same construct as text/188858.c's padding after func_00289190. */
#ifndef TARGET_NATIVE
__asm__(".word 0\n\t.word 0");
#endif

/* func_0029DD08: 8 bytes of dead pad (addiu $sp,0x10; nop) carved off the real
 * body func_0029DD10 in task #472; func_0029DD10 is the g_guiInstance+0x3CEA0
 * wrapper to func_0033A9F8. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD08);

/** func_0029DD10 — when the GUI is up, draw the confirm-dialog screen at
 *  g_guiInstance+0x3CEA0 (func_0033A9F8). Built on the s136os arm (task #1668). */
extern void func_0033A9F8(void *screen);
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029DD10)
S136OS_SLOT(func_0029DD10);
#else
void func_0029DD10(void) {
    if (g_guiInstance != 0) {
        func_0033A9F8(g_guiInstance + 0x3CEA0);
    }
}
#endif

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD80);

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
 *       while the pinned cc1 picks $2/$3 (the register-coloring residue
 *       func_0029C448 shows; func_0029C5B0's, once grouped here, was a
 *       source-shape residue and closed on the s136os arm in task #1668).
 *       Left as asm. */
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
/* t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 69.18% -> PACKED-SAVE, first differing row @0: ROM `addiu sp, sp, -0x30` vs `(nothing)`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 68.00% -> SCHED-PROEPI, first differing row @0: ROM `addiu sp, sp, -0x30` vs `addiu sp, sp, -0x40`. */
s32 UpdateLevelObjectiveStates(void) {
    extern s32 EvaluateProgressCondition(s32 cond, s32 arg);  /* defined later in-unit */
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

/* EvaluateProgressCondition(cond, arg): evaluate one progress/unlock predicate.
 * `cond` is a 16-bit selector (sign-extended); `arg` is the per-case operand
 * (an index, a function pointer for case 7, or a packed level/bit field). Each
 * case returns a 0/1 truth value (case 10 returns func_002FCEA0's verbatim).
 * Any cond outside [0,11] returns 0.
 * Cases: 0 always; 1 level available, 2 item owned, 3 item NEW, 8 platinum
 * bolt, 9 cinematic bit (the flag addresses are read from the ROM; these
 * meanings come from the symbol names and are not verified); 4 map-blip record
 * `arg` has a nonzero state, 5 that state is >= 2 (D_139A28 below; NOT weapon
 * upgrades, FACT #8397; the older "objective active/complete" is unsupported
 * too, since the case reads D_139A28 and not LevelObjective.state); 6 flag
 * byte D_1395B8[arg] (what the flags mean, "dialog" included, is
 * undetermined, NOTE #8400); 7 call arg as a predicate function; 10 bit `arg`
 * of the D_1395B8 byte keyed by g_mapCurrentLevel (0x1C5150; "current level"
 * is that symbol's name, and whether it holds the same id as func_00298A00's
 * g_playerProgress key is not established), through func_002FCEA0, which also
 * writes the masked byte back (FACT #8398); 11 never (the same return-0 target
 * as an out-of-range cond).
 * Matching shape (cc1 2.9, -O2 -G8 -fno-gcse, task #888): a 12-entry switch
 * with an explicit case 11 and case 9 written before case 8, case 9's mask in
 * its own variable (gives the ROM's sllv/and/sltu), and g_mapCurrentLevel
 * declared ROM_SPLIT.
 * The switch's 0x30-byte jump table is this unit's .rodata. The ROM keeps it
 * at 0x26CA70, inside .data, so the yaml links text/198FA0.o(.rodata) between
 * data/138B80 and data/16CA20 (task #947). Any other .rodata this unit grows
 * lands at that same spot and moves the table.
 * verify_match_unit.sh reads the function byte-identical, the table included
 * (rc 0, .rodata placed at 0x26CA70). It was UNVERIFIABLE (rc 2) on that
 * .rodata reloc until task #966 taught the tool to place a unit's own section
 * symbols. */
s32 EvaluateProgressCondition(s32 cond, s32 arg) {
    switch ((s16)cond) {
    case 0:
        return 1;
    case 1:
        return g_abLevelAvailableFlags[arg] != 0;
    case 2:
        return g_inventoryOwned[arg] != 0;
    case 3:
        return g_inventoryNewFlag[arg] != 0;
    case 4: /* map-blip record's state field nonzero */
        if (arg >= 0x72) {
            return 0;
        }
        return D_139A28[arg].upgradeLevel != 0;
    case 5: /* map-blip record's state field >= 2 */
        if (arg >= 0x72) {
            return 0;
        }
        return (D_139A28[arg].upgradeLevel < 2) ? 0 : 1;
    case 6:
        return D_1395B8[arg] != 0;
    case 7: /* call `arg` as a predicate function pointer */
        return ((s32 (*)(void))arg)() != 0;
    case 9: { /* cinematic bit: arg>>2 selects the word, arg&0x1F the bit */
        s32 word = *(s32 *)((char *)&g_cinematicUnlockedFlags + (((u32)arg >> 2) << 2));
        s32 bit = 1 << (arg & 0x1F);
        return (word & bit) != 0;
    }
    case 8: /* platinum bolt: arg packs group (high 16) and slot (low 16) */
        return g_platinumBoltFlags[(arg & 0xFFFF) + ((arg >> 16) * 4)] != 0;
    case 10:
        return func_002FCEA0(g_mapCurrentLevel, arg);
    case 11:
    default:
        return 0;
    }
}

/* GatherActiveObjectives(outIds, outMask, outVals, wantValues): list the current
 * level's visible objectives for display. Walks the LevelObjective list cached
 * at ObjectiveScan.head (g_pRainHeightmap+0x38) and, for each objective that is
 * not hidden (flags bit 0x2), not inactive (state 0), and not completed with
 * flags bit 0x1 set, emits one entry.
 *
 * @param outIds      receives each entry's id; with wantValues, its text id
 *                    instead (0x31B9 when complete, else stageTextIds[tickResult]).
 *                    outIds[0] is zeroed first.
 * @param outMask     optional; bit n set when entry n is complete, bit 31 set when
 *                    every visible objective is complete. Zeroed first.
 * @param outVals     optional; receives valueBase + tickResult per entry. Its
 *                    first word is set to -1 first.
 * @param wantValues  nonzero to emit text ids rather than objective ids
 * @return            number of entries emitted
 *
 * Built on the s136os arm (task #1738). The bytes need:
 *  - The entry's text id is written through a copy of outIds and state is
 *    re-read from the record after that store, as the ROM does (`daddu $3,$4,$0`;
 *    `lh $2,0x24($9)`). The store may alias the record. The ?: keeps 0x31B9 in
 *    the beql delay slot rather than hoisted out of the loop.
 *  - `flags` is an s32 local. A u16 local is reloaded at the loop top.
 *  - A for loop with the advance in its header. A while loop with
 *    `rec += 0x28; continue;` swaps rec and state ($8/$9).
 *  - The table index goes through the struct field (stageTextIds[tickResult]).
 *    A byte-offset `rec + idx*2 + 0x14` swaps the addu operands.
 *  - g_objectiveScanHeadAbs (the device above) for the head load. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GatherActiveObjectives)
S136OS_SLOT(GatherActiveObjectives);
#else
s32 GatherActiveObjectives(s32 *outIds, s32 *outMask, s32 *outVals, s32 wantValues) {
    LevelObjective *rec = g_objectiveScanHeadAbs;
    s32 count = 0;
    s32 allComplete = 1;

    *outIds = 0;
    if (outMask != 0) {
        *outMask = 0;
    }
    if (outVals != 0) {
        *outVals = -1;
    }
    if (rec == NULL) {
        return 0;
    }

    for (; rec->id != 0; rec++) {
        s32 state = rec->state;
        s32 flags = rec->flags;

        if (state != 2 && (flags & 2) == 0) {
            allComplete = 0;             /* a visible objective is not complete */
        }
        if ((flags & 2) != 0) {          /* hidden: not listed */
            continue;
        }
        if (state == 0) {                /* inactive: not listed */
            continue;
        }
        if ((flags & 1) != 0 && state == 2) { /* completed and flag 1: not listed */
            continue;
        }

        {
            s32 *dst = outIds;
            *dst = rec->id;
            if (wantValues != 0) {
                *dst = (rec->state == 2) ? 0x31B9 : rec->stageTextIds[rec->tickResult];
            }
        }
        if (outMask != 0 && rec->state == 2) {
            *outMask |= (1 << count);
        }
        if (outVals != 0) {
            *outVals = rec->valueBase + rec->tickResult;
            outVals++;
        }
        outIds++;
        count++;
    }

    if (outMask != 0 && allComplete != 0) {
        *outMask |= 0x80000000;          /* bit 31: every visible objective is complete */
    }
    return count;
}
#endif

/* func_0029EA90 (and EAC8/EB08/EB38 below): 0/1 predicates over globals.
 * EA90, EAC8, EB08 and EB38 are s136os members (SN 2.95.3 v1.36 -fopt-stack,
 * FACT #8810; rows in tools/ee/s136os_functions.txt, spliced over their
 * S136OS_SLOT lines by tools/ee/s136os_splice.sh -- a build that skips the
 * splice drops them). Two addressing forms meet here, per symbol, as the ROM
 * has them:
 *  - 0x139xxx words/bytes (D_1397E0, D_1395C1, D_1397C4, D_1395D5) are
 *    ROM_SPLIT: cc1 splits the address itself (`lui $a; lw $b,%lo($a)`, two
 *    registers).
 *  - 0x1A7xxx bytes (g_miscExtras, D_1A7B0D, D_1A7B14, D_1A7BDD, D_1A7B10) are
 *    cc1-small / assembler-absolute (the `.extern X, 16` lines above): cc1
 *    prints the one-insn `lbu $N,X` macro and GNU as expands it through the
 *    destination register, `lui $N; lbu $N,%lo($N)` -- the "load lands in $v0"
 *    residual every split spelling had (task #1344).
 * The macro is one insn to cc1, so its delay-slot filler would hoist a later
 * one into the previous branch's slot; the ROM's compiler did not (it put the
 * `daddu $2,$0,$0` return-0 there instead). A volatile access to that byte is
 * a SCHEDULING DEVICE that keeps it out of the slot (RULING #8404's terms:
 * not a claim that the byte changes asynchronously; writer census at each
 * use). The tails are the nested-guard shape (`bnez; addiu $2,1 / daddu
 * $2,0`), which SN 1.36 keeps where cc1 2.9 scc-converts to sltu.
 * func_0029EC70 (HIGH shared, LO_SUM re-materialised) and the 0/1/2 arbiters
 * func_0029EB68/func_0029EBF8 are not closed by this. */
/** Returns 1 iff the misc-extras gate is clear (g_miscExtras == 0) AND the
 *  D_1397E0 busy bit (0x8000000) is set; 0 otherwise.
 *  MATCHED on the s136os arm (task #1344): byte-identical to the ROM in the
 *  image. The closing lever is g_miscExtras' addressing model alone (macro
 *  load, see the family comment); the body is the one #511 wrote. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029EA90)
S136OS_SLOT(func_0029EA90);
#else
/* Record, t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 72.31% -> IFCONV, first differing row @0: ROM `lui v0, %hi(g_miscExtras)` vs `lui a0, %hi(g_miscExtras)`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 99.23% -> REGNUM-COLORING, first differing row @1: ROM `lbu v0, %lo(g_miscExtras)(v0)` vs `lbu v1, %lo(g_miscExtras)(v0)`. */
s32 func_0029EA90(void) {
    if (g_miscExtras == 0) {
        if ((D_1397E0 & 0x8000000) != 0) {
            return 1;
        }
    }
    return 0;
}
#endif

/** Returns 1 iff all three dialog flags are set (D_1395C1, D_1A7B0D, D_1A7B14);
 *  0 as soon as any is clear.
 *  MATCHED on the s136os arm (task #1344): byte-identical to the ROM in the
 *  image. D_1A7B0D/D_1A7B14 are macro loads; their volatile reads are the
 *  SCHEDULING DEVICE of the family comment (without them each is hoisted into
 *  the previous beqz's delay slot). Writer census: no ROM store names either
 *  symbol; both are bytes of the 0x1A7B00 item-flag array (g_inventoryOwned
 *  +0x0D/+0x14), written through it on the main thread (GiveInventoryItem
 *  0x288D34); none is traced to an interrupt or callback (ASSERTED, by name). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029EAC8)
S136OS_SLOT(func_0029EAC8);
#else
/* Record, t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 73.67% -> IFCONV, first differing row @0: ROM `lui v0, %hi(D_1395C1)` vs `lui a0, %hi(D_1395C1)`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 95.00% -> REGNUM-COLORING, first differing row @5: ROM `lbu v0, %lo(D_1A7B0D)(v0)` vs `lbu v1, %lo(D_1A7B0D)(v0)`. */
s32 func_0029EAC8(void) {
    if (D_1395C1 != 0) {
        if (*(volatile u8 *)&D_1A7B0D != 0) {
            if (*(volatile u8 *)&D_1A7B14 != 0) {
                return 1;
            }
        }
    }
    return 0;
}
#endif

/** Returns 1 iff a stream is in flight (D_1397C4 < 0) AND the busy flag
 *  D_1395D5 is set; 0 otherwise. Takes no arguments.
 * MATCHED on the s136os arm (FACT #8830).
 * GUARD (task #1269): on EE this C is the image's body, compiled alone by SN
 * 2.95.3 v1.36 -fopt-stack (tools/ee/s136os_functions.txt) and spliced over the
 * S136OS_SLOT line by tools/ee/s136os_splice.sh; the 2.9 compile sees only the
 * slot, so a build that skips the splice loses the function. On native it is
 * plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029EB08)
S136OS_SLOT(func_0029EB08);
#else
/* Record, t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 74.09% -> IFCONV, first differing row @0: ROM `lui v0, %hi(D_1397C4)` vs `lui a0, %hi(D_1397C4)`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 93.64% -> LIKELY-BRANCH, first differing row @2: ROM `bgez v1, 0x5b0c` vs `bgezl v1, 0x5b84`. */
s32 func_0029EB08(void) {
    if (D_1397C4 < 0 && D_1395D5 != 0) {
        return 1;
    }
    return 0;
}
#endif

/** Returns 1 iff both dialog flags are set (D_1A7BDD AND D_1A7B10); 0 otherwise.
 *  MATCHED on the s136os arm (task #1344): byte-identical to the ROM in the
 *  image. Both are macro loads; the volatile read of D_1A7B10 is the
 *  SCHEDULING DEVICE of the family comment. Writer census: no ROM store names
 *  D_1A7B10 (readers only: here, func_0029EBF8, RestorePlayerProgressState);
 *  it is g_inventoryOwned[0x10], written through the array on the main thread
 *  (GiveInventoryItem 0x288D34); none traced to an interrupt (ASSERTED). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0029EB38)
S136OS_SLOT(func_0029EB38);
#else
/* Record, t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 74.09% -> IFCONV, first differing row @0: ROM `lui v0, %hi(D_1A7BDD)` vs `lui a0, %hi(D_1A7BDD)`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 93.18% -> REGNUM-COLORING, first differing row @1: ROM `lbu v0, %lo(D_1A7BDD)(v0)` vs `lbu v1, %lo(D_1A7BDD)(v0)`. */
s32 func_0029EB38(void) {
    if (D_1A7BDD != 0) {
        if (*(volatile u8 *)&D_1A7B10 != 0) {
            return 1;
        }
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
extern s16 D_257502 ROM_SPLIT;   /* cinematic-unlock status code (0x1F / 0x20) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB68);
#else
/* t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 37.97% -> HIREG-LIVE, first differing row @1: ROM `daddu a1, v0, zero` vs `lui v1, %hi(g_cinematicUnlockedFlags+0x5c)`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 41.80% -> HIREG-LIVE, first differing row @1: ROM `daddu a1, v0, zero` vs `lui v1, %hi(D_1395B8+0x1d)`. */
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
extern s16 D_2579B2 ROM_SPLIT;   /* dialog-skip status code (0x28 / 0x29) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EBF8);
#else
/* t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 60.57% -> IFCONV, first differing row @1: ROM `lbu v0, %lo(D_1A7BDD)(v0)` vs `lbu v1, %lo(D_1A7BDD)(v0)`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 58.57% -> REGNUM-COLORING, first differing row @1: ROM `lbu v0, %lo(D_1A7BDD)(v0)` vs `lbu v1, %lo(D_1A7BDD)(v0)`. */
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
/* t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 42.11% -> IFCONV, first differing row @2: ROM `daddu a1, v0, zero` vs `(nothing)`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 42.11% -> IFCONV, first differing row @1: ROM `lui a0, 0x1` vs `(nothing)`. */
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029FDF8);

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
/* t511 promotion sweep (unit objdiff report, objdiff_build.sh + unit_report.sh, clean):
 * sdk29 arm (cc1 2.9 -O2 -G8 -fno-gcse, plain C) 59.02% -> PACKED-SAVE, first differing row @0: ROM `addiu sp, sp, -0x10` vs `addiu sp, sp, -0x30`;
 * engine96 arm (cc1 2.96-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing, MATCH_ guard) 63.95% -> SCHED, first differing row @1: ROM `lui v0, %hi(g_mobyTableEnd)` vs `(nothing)`. */
extern u8  *g_mobySpawnStart;    /* 0x1B1AE0 first dynamic moby slot */
extern u8  *g_mobyTableEnd;      /* 0x1B1AE4 moby table walk bound */
extern u8  *g_mobyAuxBlockBase;  /* 0x1B1AEC parallel per-moby 0x80-byte block array */
extern s32  g_mobySpawnCredit;   /* remaining spawn budget */
extern s32  g_gameTime;          /* global frame counter */
extern char D_1A9DC8[];          /* "no free moby slot" log string */
struct Moby;                      /* defined below with the Moby typedef */
extern void InitMobyFromClass(struct Moby *moby, s32 classId);

void *SpawnMoby(s32 classId) {
    u8 *m = g_mobySpawnStart;

    if (m < g_mobyTableEnd) {
        s32 state = m[0x20];
        for (;;) {
            if (state >= 0xFE && !(g_gameTime < *(s32 *)(m + 0xA0))) {
                if (state == 0xFF) {
                    m[0x120] = (u8)state;
                }
                InitMobyFromClass((struct Moby *)m, classId);
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
 * 0x100). Opaque here: InitMobyFromClass reads and writes it through its own
 * field view (MobyInitView, below). A typedef emits no code. */
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

/* ADDRESSING-MODEL DEVICE (RULING #8620): InitMobyFromClass reads
 * g_mobyTableBase absolutely (0x29FF80 `lui $2,%hi(g_mobyTableBase)`; lw) while
 * the 4-byte pointer is -G8 small, so the name is sized 16 for the assembler.
 * It is the only reference in this unit (ROM census of the unit's splat files:
 * 1 %hi, 0 %gp_rel), so no other function moves. Without it the load is
 * `lw $2,%gp_rel(g_mobyTableBase)($28)`, one word short. Nothing is emitted. */
#ifndef TARGET_NATIVE
__asm__(".extern g_mobyTableBase, 16");
#endif

/** Zero a moby slot and bind it to a class.
 *
 *  FillMemory32 clears the 0x100-byte record, then it stamps the spawn defaults:
 *  classSlot (+0x22) = g_mobyClassSlotRemap[classId], alpha 0x80, the tint qword
 *  0x0040404000000000 (+0x38), the 0x7F/0x80/0xFF channel bytes, classId (+0xAA)
 *  and the uid (+0xAC) = table index << 16 (index = (moby - g_mobyTableBase) >> 8).
 *  If g_mobyClassSlotToId[classSlot] does not map back to classId the class has
 *  no loaded header: flags |= 5, no class pointer, pUpdate from
 *  g_mobyClassUpdateFuncsNoHeader (flags |= 2 when there is none). Otherwise it
 *  binds pUpdate from g_mobyClassUpdateFuncs, the class header (+0x24), its byte
 *  +0x0E, flag bits +0x44, word +0x10, scale (+0x2C) and the optional collision
 *  mesh (+0x78, flags |= 0x10), sets 1.0 anim rates, applies the header's +0x0F
 *  (flags |= 0x400 and +0x6F = 0x18) and +0x06 (+0x63 = 0x18) switches, and when
 *  the class has an animation set resolves its frame pointers: two or more
 *  frames clear flags bit 2; a single-frame set on a class whose +0x0C is 1 zeroes
 *  anim rate +0x48 and sets flags bit 0x40 when the set's +0x11 is negative.
 *  @param moby     the 0x100-byte slot to initialise
 *  @param classId  moby class id
 *
 *  Built on the s136os arm (task #1918), byte-identical under the unit's
 *  -fno-gcse and with gcse on. The bytes need:
 *   - the tint constant's dli to use Ps2EeAs's 4-word expansion: a RULING #8549
 *     row in tools/ee/ps2eeas_dli_sites.txt (GNU as emits 3 words, so the
 *     function is one word short without it);
 *   - the slot read from g_mobyClassSlotRemap BEFORE the alpha store (the ROM
 *     loads it first; the store may alias the table);
 *   - slot as an s32 local, and the SlotToId and UpdateFuncs indices read back
 *     through m->classSlot: cse turns both into the one `andi $10` the ROM
 *     keeps, while the headerless path re-reads the field (`lbu $2,0x22($17)`).
 *     Indexing UpdateFuncs by (u8)slot instead is 5/156 words different.
 *   - the table index computed as its own statement after the +0x62 store and
 *     before the tint store (the ROM's g_mobyTableBase load sits there);
 *   - the stores in this source order. Measured here, SN 1.36 cc1 issued the
 *     last store of each constant register (0xFF, 0x7F, 0x80) first and the
 *     rest in source order (compare FACT #9254), so the source puts
 *     +0x6D/+0xA5/+0xA7 last to get the ROM's order (24 -> 18 words). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_InitMobyFromClass)
S136OS_SLOT(InitMobyFromClass);
#else
typedef struct {
    u8  _p0[0x10];
    u8  numFrames;          /* +0x10 */
    s8  flags11;            /* +0x11 */
} MobyAnimSetHdr;
typedef struct {
    u8  _p0[6];
    u8  b06;                /* +0x06 nonzero: +0x63 = 0x18 */
    u8  _p7[5];
    u8  b0C;                /* +0x0C */
    u8  _pD;
    u8  b0E;                /* +0x0E copied to moby +0x62 */
    u8  b0F;                /* +0x0F nonzero: flags |= 0x400 */
    s32 w10;                /* +0x10 copied to moby +0x98 */
    u8  _p14[0x10];
    f32 scale;              /* +0x24 default scale */
    u8  _p28[0x18];
    void *collMesh;         /* +0x40 */
    u16 flags44;            /* +0x44 flag bits OR'd into moby +0x34 */
    u16 _p46;
    MobyAnimSetHdr *animSet; /* +0x48 */
} MobyClassHdr;
typedef struct {
    u8  _p0[0x21];
    u8  b21;                /* +0x21 */
    u8  classSlot;          /* +0x22 */
    u8  alpha;              /* +0x23 */
    MobyClassHdr *pClass;   /* +0x24 */
    u8  _p28[4];
    f32 scale;              /* +0x2C */
    u8  _p30[4];
    u16 flags;              /* +0x34 */
    s16 h36;                /* +0x36 */
    u64 tint;               /* +0x38 */
    u8  _p40[8];
    f32 animRate0;          /* +0x48 */
    f32 animRate1;          /* +0x4C */
    u8  _p50[0x11];
    u8  b61;                /* +0x61 */
    u8  b62;                /* +0x62 */
    u8  b63;                /* +0x63 */
    void *pUpdate;          /* +0x64 */
    u8  _p68[4];
    u8  b6C;                /* +0x6C */
    u8  b6D;                /* +0x6D */
    u8  b6E;                /* +0x6E */
    u8  b6F;                /* +0x6F */
    s32 w70;                /* +0x70 */
    s32 w74;                /* +0x74 */
    void *collMesh;         /* +0x78 */
    u8  _p7C[0x1C];
    s32 w98;                /* +0x98 */
    u8  _p9C[8];
    u8  bA4, bA5, bA6, bA7, bA8; /* +0xA4..+0xA8 colour-channel bytes */
    u8  _pA9;
    s16 classId;            /* +0xAA */
    s32 uid;                /* +0xAC */
    u8  _pB0[0xD];
    u8  bBD;                /* +0xBD */
} MobyInitView;

void InitMobyFromClass(Moby *moby, s32 classId) {
    MobyInitView *m = (MobyInitView *)moby;
    s32 slot;
    s32 index;

    FillMemory32(m, 0, 0x100);

    slot = g_mobyClassSlotRemap[classId];
    m->alpha = 0x80;
    m->classSlot = slot;
    m->bA8 = 0xFF;
    m->b21 = 0xFF;
    m->b61 = 0xFF;
    m->b62 = 0xFF;
    index = (s32)((u8 *)m - g_mobyTableBase) >> 8;
    m->tint = 0x0040404000000000ULL;
    m->h36 = 0x7F80;
    m->uid = index << 16;
    m->classId = classId;
    m->b6E = 0;
    m->b6C = 0xFF;
    m->bA4 = 0x7F;
    m->bA6 = 0x80;
    m->b6D = 0xFF;
    m->bA5 = 0x7F;
    m->bA7 = 0x80;

    if (g_mobyClassSlotToId[m->classSlot] != classId) {
        m->flags |= 5;
        m->pClass = 0;
        m->w98 = 0;
        m->pUpdate = g_mobyClassUpdateFuncsNoHeader[m->classSlot];
        if (m->pUpdate == 0) {
            m->flags |= 2;
        }
    } else {
        m->pUpdate = g_mobyClassUpdateFuncs[m->classSlot];
        if (m->pUpdate == 0) {
            m->flags |= 2;
        }
        m->pClass = (MobyClassHdr *)g_mobyClassHeaders[m->classSlot];
        m->b62 = m->pClass->b0E;
        m->flags |= m->pClass->flags44;
        m->w98 = m->pClass->w10;
        m->scale = m->pClass->scale;
        m->animRate0 = 1.0f;
        m->animRate1 = 1.0f;
        if (m->pClass->collMesh != 0) {
            m->flags |= 0x10;
            m->collMesh = m->pClass->collMesh;
        }
        if (m->pClass->b0F != 0) {
            m->b6F = 0x18;
            m->flags |= 0x400;
            m->w70 = 0;
            m->w74 = 0;
            m->bBD = 0;
        }
        if (m->pClass->b06 != 0) {
            m->b63 = 0x18;
        }
        if (m->pClass->animSet != 0) {
            ResolveMobyAnimFramePtrs(m);
            if (m->pClass->animSet->numFrames >= 2) {
                m->flags &= ~2;
            }
            if (m->pClass->b0C == 1 && m->pClass->animSet->numFrames < 2) {
                m->animRate0 = 0.0f;
                if (m->pClass->animSet->flags11 < 0) {
                    m->flags |= 0x40;
                }
            }
        }
    }
}
#endif

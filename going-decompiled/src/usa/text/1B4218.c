#include "common.h"
#include "moby_motion.h"

/*
 * text/1B4218 — the "moby-bind / locomotion / target-lock / game-state-stack /
 * dialog-voice" band (carve-pipeline pick #5/moby-bind, 2026-06-13; vaddr
 * 0x2B4298..0x2B9027, 92 splat symbols). Carved out of the big text/1B21E8 asm
 * tile (now the head 0x2B2268..0x2B4297 + this c unit + the tail 0x2B9028+).
 *
 * Contents (by sub-cluster):
 *   - moby auto-target acquisition + threat classification (0x2B4298..0x2B4F3F),
 *   - moby spring-follow / local-transform helpers (0x2B4F40..0x2B58B7),
 *   - the top-level game-state request/pop/commit machine + 8-deep state stack
 *     (0x2B58B8..0x2B5F0F),
 *   - the core per-frame moby locomotion step: steer / collide / ground / lean /
 *     waypoint follow (0x2B5F10..0x2B7217),
 *   - per-frame moby draw-list + class-update binding (0x2B7218..0x2B75FF),
 *   - the sound-bank loader bridge (0x2B7610..0x2B77E7),
 *   - the file-load pump + dialog/ambient/secondary/tertiary voice playback
 *     state machines (0x2B77E8..0x2B9027).
 *
 * Built at -O2 -G8 -fno-gcse (per-unit GFLAG/CC1EXTRA override in
 * objdiff_build.sh / diff.sh / build.sh) — the same later-SN-cc1 TU model as
 * the adjacent text/1A8180 game-state cluster.
 *
 * -G8 extern-sizing rules (same model as text/1A8180 / text/198FA0):
 *   - size <= 8: true small data, %gp_rel everywhere;
 *   - size >= 16: cc1-small / assembler-absolute (lui/$at macro everywhere);
 *   - size 9..15 (we use 12): gp-addressable / assembler-absolute (absolute
 *     macro in straight-line code, 1-insn %gp_rel only in a branch delay slot).
 *
 * SAVE-LAYOUT WALL: this TU was built by the later SN cc1 that packs callee-save
 * slots 8-byte; the pinned cc1 reserves 16 bytes per save. Every function below
 * that saves two or more GPRs (incl. $ra) is blocked on that wall and stays
 * INCLUDE_ASM (check the prologue: two+ sd of s-regs/$ra at 8-byte spacing).
 * The two switch functions (UpdateGameState 0x2B5B38, StepDialogVoiceChannel
 * 0x2B82A0) also stay INCLUDE_ASM behind the splat jtbl reloc-identity gap.
 *
 * PADDING/FRAGMENT PSEUDO-FUNCTIONS: of the 92 splat symbols, 14 are pure
 * inter-function fill or unreachable handwritten fragments and keep INCLUDE_ASM
 * permanently: the 6 size-pinned 8-byte epilogue pads (func_002B50B8/5FF8/
 * 61B8/6FC8/7038/7140) and the 8 handwritten stub-table fragments (orphaned
 * `addiu $sp,+N / nop` runs: func_002B46C8/4F40/4F80/58B8/7218/7570/77E0/8FD8).
 */

typedef struct Vec4 { f32 x, y, z, w; } Vec4;

/* Minimal moby view (0x100-stride table entries); only fields this unit touches
 * are named. Same layout as the text/1A8180 Moby view. */
typedef struct Moby {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ Vec4 facingTarget; /* heading target vec (moby+0x10) */
    /* 0x20 */ u8 pad20[0x14];
    /* 0x34 */ u16 modeFlags;     /* MODE/behaviour bitfield (see moby.h); bit 0x8000
                                     mirrors the lean delta (UpdateMobyLeanFromTurn) */
    /* 0x36 */ u8 pad36[0x32];
    /* 0x68 */ s32 *pExtra;       /* extra/pvars block; motion controller at +0x18 */
    /* 0x6C */ u8 pad6C[0x52];
    /* 0xBE */ u8 animFlags;      /* one-shot latch bits set by SetMobyFlagBit* */
    /* 0xBF */ u8 padBF[0x39];
    /* 0xF8 */ f32 moveSpeed;     /* per-frame move speed (moby+0xF8) */
    /* 0xFC */ u8 padTail[0x100 - 0xFC]; /* pad this local field-view out to the
                                            canonical Moby SIZE (0x100, see moby.h)
                                            so the functional-equivalence tester can
                                            allocate + bind a full Moby. Keeps this
                                            TU's own field names; only the size is
                                            unified. Byte-neutral: trailing only. */
} Moby;
/* Verify the view spans the full canonical 0x100 record (ILP32 only — the
 * matching ee-gcc 2.9 lacks both _Static_assert and __SIZEOF_POINTER__, and a
 * 64-bit host widens pExtra; same guard as include/moby.h). */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(Moby) == 0x100, "Moby must be 0x100 under ILP32");
#endif

/* True small / gp-addressable globals (complete <=8-byte declarations -> %gp_rel). */
extern s32 g_gameStateStackDepth;          /* 0x1AA100 game-state stack depth (gated <8) */
extern s32 g_gameStatePendingArgA;         /* 0x1AA0F8 pending-transition arg A */
extern s32 g_gameStatePendingArgB;         /* 0x1AA0FC pending-transition arg B */
extern s32 g_gameStateTransitionDoneFlag;  /* 0x1AA104 out-flag set to 0 on pop */
extern s32 g_gameStateTransitionTimer;     /* 0x1AA0F0 frames stalled before switch */
extern s32 g_gameStateTransitionDelay;     /* 0x1AA0F4 stall length */
extern s32 g_nGameStatePrev;               /* 0x1A8BB4 previous game state */
extern s32 g_mobyMotionProfileCount;       /* 0x1AA114 measured moby-motion steps */
extern f32 g_mobyMotionProfileTicks;       /* 0x1AA10C accumulated RCNT0 ticks */

/* Large/absolute globals: declared sized >= 16 (via the .extern asm directive,
 * matching the text/1A8180 cc1-small / assembler-absolute model) so cc1
 * materialises the address with the lui/%lo absolute macro, matching the
 * original. g_nGameStatePending sits IN the gp range but the original accesses
 * it absolutely, so it must be sized out of small-data. */
__asm__(".extern g_nGameStatePending, 16");
__asm__(".extern g_gameStateStack, 32");
__asm__(".extern g_health, 16");
extern s32 g_nGameStatePending;            /* 0x1A8BB8 pending state (-2 = none) */
extern s32 g_gameStateStack[8];            /* 0x1B1CD0 int[8] saved game-state ids */
extern s32 g_health;                       /* 0x18C2EC current health (0 = dead) */

/* The file-load + dialog-voice manager state block lives at g_saveImageArea +
 * 0x1000 (a fixed RAM scratch area past the per-area save image). Only the
 * fields this unit touches are named. The three dialog-voice channels sit at
 * +0x50, +0x74, +0x98 (each a 0x24-byte sub-block: state half at +0x0, fade
 * target at +0x2). */
extern u8 g_saveImageArea[];               /* 0x1A53A8 per-area save image (<=0x1000) */

typedef struct DialogVoiceChannel {
    /* 0x00 */ s16 volume;        /* current/target volume (high bit = active) */
    /* 0x02 */ s16 fadeTarget;    /* fade target volume */
} DialogVoiceChannel;

/* File-load + dialog-voice manager (at g_saveImageArea + 0x1000). The three
 * voice channels sit at +0x50 (ch0/primary), +0x74 (ch1/secondary), +0x98
 * (ch2/tertiary); the queued playback parameters are packed in the gaps. */
typedef struct FileLoadVoiceState {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ s16 fileLoadActive;        /* nonzero while a CD read is in flight */
    /* 0x06 */ u8 readStopped;            /* abort flag: set after CdStopRead */
    /* 0x07 */ u8 pad07[0x1];
    /* 0x08 */ s32 fileLoadLbn;           /* active read start LBN */
    /* 0x0C */ u8 pad0C[0x4];
    /* 0x10 */ s32 fileLoadSectorCount;   /* active read sector count */
    /* 0x14 */ s32 fileLoadDest;          /* active read destination address */
    /* 0x18 */ void *pLoadCallback;       /* completion callback fn(arg, success) */
    /* 0x1C */ s32 loadCallbackArg;       /* completion callback first arg */
    /* 0x20 */ u8 pad20[0x4];
    /* 0x24 */ s32 soundBankId;           /* loaded global sound-bank id, init -1 */
    /* 0x28 */ u8 pad28[0x4];
    /* 0x2C */ s16 fileLoadPhase;         /* file-load progress phase (1 = armed, 2 = reading) */
    /* 0x2E */ u8 pad2E[0x2];
    /* 0x30 */ s32 fileLoadByteCursor;    /* current byte cursor of the active read */
    /* 0x34 */ s32 fileLoadWordCount;     /* byteCursor rounded down to words (>>2) */
    /* 0x38 */ u8 pad38[0x4];
    /* 0x3C */ s16 dialogVoiceId;         /* primary dialog-voice id, init -1 */
    /* 0x3E */ s16 dialogVoiceIdPrev;     /* mirror of dialogVoiceId, reset to -1 */
    /* 0x40 */ u8 dialogState;            /* primary dialog state, init 0x20 */
    /* 0x41 */ u8 dialogFlag1;            /* init 0 */
    /* 0x42 */ u8 dialogFlag2;            /* init 0 */
    /* 0x43 */ u8 dialogFlag3;            /* init 0 */
    /* 0x44 */ s32 ambientState;          /* ambient-voice state word, init 0 */
    /* 0x48 */ s16 ambientArg0;           /* queued ambient-voice params */
    /* 0x4A */ s16 ambientArg2;
    /* 0x4C */ s16 ambientArg1;
    /* 0x4E */ s16 ambientFlag;           /* init 0 */
    /* 0x50 */ DialogVoiceChannel ch0;    /* primary dialog voice channel */
    /* 0x54 */ s16 ambientCursor;         /* ambient-voice playback cursor, init 0 */
    /* 0x56 */ u8 pad56[0x2];
    /* 0x58 */ s32 ambientVolume;         /* ambient-voice volume (10) */
    /* 0x5C */ s32 ambientSampleRate;     /* ambient-voice sample rate (48000) */
    /* 0x60 */ u8 pad60[0x8];
    /* 0x68 */ s32 secondaryState;        /* secondary-voice state word, init 0 */
    /* 0x6C */ s16 dialogArg1;            /* queued dialog-voice params */
    /* 0x6E */ s16 dialogArg0;
    /* 0x70 */ s16 dialogArg2;
    /* 0x72 */ s16 secondaryFlag;         /* init 0 */
    /* 0x74 */ DialogVoiceChannel ch1;    /* secondary voice channel */
    /* 0x78 */ u8 pad78[0xC];
    /* 0x84 */ s32 dialogArg3;
    /* 0x88 */ u8 pad88[0x4];
    /* 0x8C */ s32 tertiaryState;         /* tertiary-voice state word, init 0 */
    /* 0x90 */ u8 pad90[0x4];
    /* 0x94 */ s16 tertiaryArg1;          /* queued tertiary-voice param, reset to 0 */
    /* 0x96 */ s16 tertiaryFlag;          /* init 0 */
    /* 0x98 */ DialogVoiceChannel ch2;    /* tertiary voice channel */
} FileLoadVoiceState;

#define g_fileLoadVoiceState (*(FileLoadVoiceState *)(g_saveImageArea + 0x1000))

/* Callees (value-returning declarations keep cc1 from sibling-call optimising
 * forwarding tails — see text/198FA0). */
extern void SetSndPumpCallback(void *cb);  /* 0x1336D0 */
extern void PumpFileLoadCompletion(s32 phase);  /* 0x2B8CA8 snd-pump tick */
extern void CdStopRead(void);              /* 0x133640 */
extern s32 CdGetLoadStatus(void);          /* 0x133688 */
extern void FlushCache(s32 mode);          /* 0x0011AEA0 */
__asm__(".extern g_fileLoadState, 16");
extern s16 g_fileLoadState;                /* 0x1A63AC 0 idle / 1 requested / 2 in progress */
extern void DebugPrintStub(const char *s); /* 0x26FEC8 retail debug no-op */
extern char D_1AA220[];                    /* tertiary-voice debug string */
extern void OnSoundBankLoaded(s32 bankId, long pOut);  /* 0x2B7690 forward decl */
extern void snd_BankLoadFromEE_CB(s32 arg, void *cb, long pOut);  /* 0x1325E8 */
extern void snd_BankLoadAsync(s32 bankAddr, s32 a1, void *cb, long pStatus);  /* 0x132498 */
__asm__(".extern g_discToc, 16");
extern u8 g_discToc[];                      /* 0x14B540 master disc asset directory */

/* The sound-bank load-status slots live at g_listenerPosHistory + 0x17A0 (s32
 * per bank id); the loader writes -1 there and lets OnSoundBankLoaded fill it. */
#define g_soundBankLoadStatus ((s32 *)(g_listenerPosHistory + 0x17A0))
extern void StepMobyMotion(Moby *moby, Vec4 *target, f32 speed);  /* 0x2B6000 */
extern s32 StartDialogVoice(s32 a0, s32 a1, s32 a2, s32 a3);      /* 0x2B7878 */
extern s32 StartAmbientVoice(s32 idx, s16 flags, s16 pan);       /* 0x2B7CA0 */
extern s32 StartSecondaryVoice(s32 idx, s16 flags, s16 pan);     /* 0x2B7D98 */
extern s32 snd_PlaySample(s64 sampleStart, s64 sampleEnd, s32 a2, s32 a3,
                          s32 pan, s32 a5, void *startCb, long context);  /* 0x133350 */
extern void func_002B8ED0(s32 voiceId, long handle);  /* secondary-voice start cb */
extern void func_002B8E78(s32 voiceId, long handle);  /* chained-voice start cb */
extern void OnAmbientVoiceStarted(s32 voiceId, long handle);  /* 0x2B8DC8 */

/* The dialog sample-address table: g_discToc + 0x5300 holds per-id sector
 * offsets (stride 8), g_discToc + 0x52FC is the global-WAD base added to each. */
#define g_dialogSampleTable ((s32 *)(g_discToc + 0x5300))
#define g_dialogSampleBase  (*(s32 *)(g_discToc + 0x52FC))

/* A playing-voice handle (returned by the snd voice allocator). The voice
 * playback callbacks below transition the +0xA state byte and bookkeep +0x0 id,
 * +0x10 gate, +0x18 sample cursor. The handle arrives as a 64-bit value whose
 * low 32 bits hold the address (the dsll32/dsra32 sign-extend prologue). */
typedef struct VoiceHandle {
    /* 0x00 */ s32 voiceId;
    /* 0x04 */ u8 pad04[0x6];
    /* 0x0A */ s16 state;
    /* 0x0C */ u8 pad0C[0x4];
    /* 0x10 */ s16 gate;
    /* 0x12 */ u8 pad12[0x6];
    /* 0x18 */ s32 sampleCursor;
    /* 0x1C */ u8 pad1C[0x4];
    /* 0x20 */ s32 sampleSlot;     /* index into g_listenerPosHistory (stride 0x70) */
} VoiceHandle;

/* Ring of per-emitter listener-position blocks (stride 0x70); the voice id is
 * mirrored at +0x70 of the slot's block. */
extern u8 g_listenerPosHistory[];          /* 0x188660 */

/* Queued dialog voice id; +0x8 (g_pendingDialogVoiceId + 0x8 = 0x1A63D4) is a
 * s16 "playing state" mirror updated by the voice callbacks. Sized >= 16 (the
 * symbol sits below the gp window, so the original addresses it absolutely). */
__asm__(".extern g_pendingDialogVoiceId, 16");
extern s32 g_pendingDialogVoiceId;         /* 0x1A63CC */
/* The s16 "playing state" mirror sits at 0x1A63D4 = g_pendingVoiceAux (0x1A63D0)
 * + 4. The voice callbacks reference it relative to g_pendingVoiceAux (its own
 * named symbol), so address it through that symbol for a byte-exact reloc. */
__asm__(".extern g_pendingVoiceAux, 16");
extern u8 g_pendingVoiceAux[];             /* 0x1A63D0 */
#define g_dialogVoicePlayingState (*(s16 *)(g_pendingVoiceAux + 4))

/* The level-object table (stride 0xA430); field +0x4 of each entry is the
 * backing moby pointer. Used by CheckMobyIsLevelObjectSlot. */
typedef struct LevelObject {
    /* 0x00 */ s32 f0;
    /* 0x04 */ s32 mobyPtr;        /* backing moby pointer for this slot */
    /* 0x08 */ u8 rest[0xA428];
} LevelObject;
extern LevelObject D_2403D0[];              /* 0x2403D0 level-object table */

/* Per-frame moby threat flash + burst update.
 * WALL: save-layout — saves 8 callee-saves + $ra at 8-byte spacing (the pinned
 * cc1 packs at 16-byte spacing), plus float register temps. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateMobyThreatFlashAndBurst);

/* Classify a candidate target by proximity band (near/mid/far threat tier).
 * WALL: save-layout — 3 callee-saves + $ra at 8-byte spacing, with fp temps. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ClassifyTargetProximity);

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B46C8);

extern void UpdateMobyBSphereAndGrid(void *moby);  /* 0x2A1D80 */

/* Field view for the parent-bind copy (offsets from moby.h). Kept local to this
 * shim so it does not perturb the unit's matching-build Moby view. */
typedef struct MobyBind {
    /* 0x00 */ u8   pad00[0x10];
    /* 0x10 */ Vec4 pos;          /* world position (copied whole from parent) */
    /* 0x20 */ u8   pad20[0x10];
    /* 0x30 */ u8   drawDist;     /* LOD/draw-distance byte; 0xFF = never culled */
    /* 0x31 */ u8   drawDistAlways;/* nonzero = bypass distance cull */
    /* 0x32 */ u16  bindFlags;    /* 0xFF on parent-bind */
    /* 0x34 */ u16  modeFlags;    /* MODE bitfield; low 3 bits cleared on bind */
    /* 0x36 */ u8   pad36[0x2];
    /* 0x38 */ s64  color;        /* packed color qword (copied whole from parent) */
    /* 0x40 */ u8   pad40[0x78];
    /* 0xB8 */ void *pParent;     /* parent moby pointer */
    /* 0xBC */ u8   padBC[0x34];
    /* 0xF0 */ Vec4 facing;       /* facing/forward vec (copied whole from parent) */
} MobyBind;

/* Bind a moby to a parent transform (locomotion attach helper). Forces the
 * moby never-distance-culled and shadow-enabled, marks it parent-bound, and —
 * when a parent is given — inherits the parent's world position, facing vector
 * and color, links the parent pointer, copies the parent's +0x38 color qword,
 * and re-derives the moby's bounding sphere / spatial-grid bucket. Always clears
 * the low three MODE bits.
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). Derived
 * register-exact from BindMobyToParent.s @0x2B4700.
 *
 * WALL (matching build): save-layout — saves $s0/$ra at 8-byte spacing (frame
 * 0x10); the pinned cc1 packs callee-save GPR slots at 16-byte spacing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", BindMobyToParent);
#else
void BindMobyToParent(void *mobyp, void *parentp) {
    MobyBind *moby = (MobyBind *)mobyp;
    MobyBind *parent = (MobyBind *)parentp;

    moby->drawDist = 0xFF;
    moby->bindFlags = 0xFF;
    moby->drawDistAlways = 1;
    if (parent != 0) {
        moby->pParent = parent;
        moby->pos = parent->pos;
        moby->facing = parent->facing;
        moby->color = parent->color;
        UpdateMobyBSphereAndGrid(moby);
    }
    moby->modeFlags &= 0xFFF8;
}
#endif

/* One-shot latch of moby anim-flag bit 0. Returns 1 if newly set, 0 if it was
 * already set (in which case the byte is not rewritten). */
s32 SetMobyFlagBit0(Moby *moby) {
    u8 flags = moby->animFlags;
    if (((flags ^ 1) & 1) == 0) {
        return 0;
    }
    moby->animFlags = flags | 1;
    return 1;
}

/* One-shot latch of moby anim-flag bit 1. Returns 1 if newly set, 0 otherwise.
 * WALL: register-coloring + store-scheduling. The original keeps the byte in
 * $v1 and sinks the write-back `sb` into the final `jr` delay slot with a plain
 * `andi` test (no xori, unlike bit0). cc1 here either colors the byte into $v0
 * or won't sink the store without an extra xori idiom (best 81.67%); genuine
 * reg-alloc wall, left as INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", SetMobyFlagBit1);
#else
/**
 * One-shot latch of moby anim-flag bit 1.
 *
 * @param moby moby whose animFlags byte (+0xBE) is latched
 * @return     1 if the bit was newly set (byte rewritten), 0 if it was already
 *             set (byte left untouched)
 *
 * Faithful to the .s: tests `flags & 2`; if already set returns 0 without
 * storing, otherwise writes `flags | 2` and returns 1. Note the asm computes
 * `flags | 2` up front (in the branch delay slot) but only commits the store
 * on the not-yet-set path.
 */
s32 SetMobyFlagBit1(Moby *moby) {
    u8 flags = moby->animFlags;
    if ((flags & 2) != 0) {
        return 0;
    }
    moby->animFlags = flags | 2;
    return 1;
}
#endif

/* Test moby anim-flag bit 2. */
s32 func_002B47D0(Moby *moby) {
    return moby->animFlags & 4;
}

/* Compute squared distance from a moby to its auto-target candidate.
 * WALL: save-layout — 4 callee-saves + $ra at 8-byte spacing, with fp temps. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CalcMobyTargetThreatDist);

/* Scan a moby group for the best auto-target by threat distance.
 * WALL: save-layout — 6 callee-saves + $ra at 8-byte spacing. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", FindTargetInGroup);

extern f32 DistXYVu0(Vec4 *a, Vec4 *b);  /* 0x283830 horizontal XY distance */
extern f32 GetFloatAbs(f32 x);           /* 0x2835F8 fabsf */

/* Test whether a target moby lies within the active range band of a reference
 * position: horizontal (XY) distance must be below `radius` AND the vertical
 * (Z) gap below `vertBand`.
 *
 * @param moby     candidate target moby (its facingTarget vec at +0x10 is the
 *                 XY-distance probe point; +0x18 is its Z)
 * @param refPos   reference position (Z at +0x8)
 * @param radius   max horizontal distance
 * @param vertBand max vertical gap
 * @return 1 if inside the band, 0 otherwise
 *
 * WALL: save-layout — saves $s0/$s1/$ra at 8-byte spacing (frame 0x30); the
 * pinned cc1 packs callee-save GPR slots at 16-byte spacing (frame 0x40),
 * confirmed by the compiled base. cmp-oracle validated.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CheckTargetInRangeBand);
#else
s32 CheckTargetInRangeBand(Moby *moby, Vec4 *refPos, f32 radius, f32 vertBand) {
    if (DistXYVu0(refPos, &moby->facingTarget) < radius) {
        if (GetFloatAbs(refPos->z - moby->facingTarget.z) < vertBand) {
            return 1;
        }
    }
    return 0;
}
#endif

/* Acquire the moby's auto-target (the top-level target-lock entry point).
 * WALL: save-layout — 8 callee-saves + $ra at 8-byte spacing, with fp temps. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", AcquireMobyAutoTarget);

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently (no prologue/jr of its own). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B4F40);

/* True if moby is the backing object of level-object slot `slot`
 * (D_2403D0[slot].mobyPtr == moby). Entry stride 0xA430, moby ptr at +0x4. */
s32 CheckMobyIsLevelObjectSlot(Moby *moby, s32 slot) {
    return D_2403D0[slot].mobyPtr == (s32)moby;
}

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B4F80);

/* Apply a moby's local-transform delta (spring-follow position step).
 * WALL: save-layout — 6 callee-saves + $ra at 8-byte spacing, with fp temps
 * and sq/lq 128-bit matrix moves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ApplyMobyLocalTransformDelta);

/* Size-pinned 8-byte epilogue pad pseudo-function — see unit header; kept
 * INCLUDE_ASM permanently. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B50B8);

/* Initialise a moby's spring-follow state block.
 * WALL: save-layout — 8 callee-saves + $ra at 8-byte spacing, with fp temps. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", InitMobySpringFollowState);

/* Per-frame moby spring-follow step (damped position/orientation chase).
 * WALL: save-layout — 8 callee-saves + $ra at 8-byte spacing, with fp/madd. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StepMobySpringFollow);

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B58B8);

/* Arm the game-state transition timer (frames to stall before the pending switch). */
void SetGameStateTransitionTimer(s32 frames) {
    g_gameStateTransitionTimer = frames;
}

/* Set the game-state transition stall length. */
void SetGameStateTransitionDelay(s32 frames) {
    g_gameStateTransitionDelay = frames;
}

/* True if `state` is the pending game-state target. */
s32 IsGameStatePending(s32 state) {
    return g_nGameStatePending == state;
}

/* Request a game-state change: pushes the target state onto the 8-deep stack and
 * arms the pending-transition slot (the push counterpart to PopGameState).
 *
 * Gate ladder (returns a reject code, 0 = accepted). Each gate is an
 * unconditional, condition-guarded assignment to `result`, applied in asm order
 * so a later gate overrides an earlier one (the original lowers them as
 * movz/movn chains):
 *   - result starts -1; if g_nGameStatePending == -2 -> result = 0 (idle);
 *   - if push==1 && g_gameStateStackDepth > 7 -> result = -2 (stack full);
 *   - slotActive = (stateId < 0) ? 0 : stateSlotTable[stateId];
 *     if slotActive != 0 -> result = -3 (already active);
 *   - if g_health == 0 -> result = -1 (death gate, applied LAST so it wins).
 * On accept (result == 0) it stashes the pending id + the two args + the
 * out-flag pointer, applies the occlusion-override player-progress special case,
 * and (if push) pushes the current game state.
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). Derived
 * register-exact from RequestGameStateChange.s @0x2B58F8.
 *
 * WALL (matching build): save-layout — 3 callee-saves + $ra at 8-byte spacing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", RequestGameStateChange);
#else
extern s32 g_nGameState;                /* 0x1A8BB0 current top-level game/screen state id */
extern s32 g_playerProgress;            /* 0x1A79F8 persistent-save player progress word */
extern s32 g_occlusionOverrideMode;     /* 0x1B168C occlusion override (1 all / 2 octant / 3 sector) */
extern s32 D_001A8B88[];                /* 0x1A8B88 per-state "slot active" table (s32 stride) */
extern s32 D_001C4EC0;                  /* 0x1C4EC0 (= g_pointLights + 0x2400) progress sub-flag */
extern void func_00286260(s32 mode);    /* 0x286260 trivial PURE: g_someGlobal = mode */

/* WEAK (TARGET_NATIVE arm only, byte-neutral): when this unit is co-linked into
 * the eetest cmp suite, cmp_188858_cine.c supplies a strong deterministic
 * RequestGameStateChange mock for its StartCinematicFromQueue oracle. Both
 * objects would otherwise multiply-define the symbol. weak lets that test mock
 * win the link; cmp_1B4218.c does not test this body, and gc-sections drops this
 * weak copy. In the matching build this arm is the INCLUDE_ASM original, so weak
 * is inert. */
__attribute__((weak))
s32 RequestGameStateChange(s32 stateId, s32 push, s32 argA, s32 argB, s8 *outDoneFlag) {
    s32 result;
    s32 slotActive;

    if (outDoneFlag != 0) {
        *outDoneFlag = 0;
    }

    /* gate 1: pending == -2 (idle) accepts; movz $17,$0 in the bne delay slot
       always evaluates regardless of the push branch. */
    result = -1;
    if (g_nGameStatePending == -2) {
        result = 0;
    }
    /* gate 2: push==1 && depth > 7 -> -2 (stack full). slti<8 then movz -2. */
    if (push == 1) {
        if (g_gameStateStackDepth >= 8) {
            result = -2;
        }
    }
    /* gate 3: slot table lookup (skipped, slotActive=0, when stateId < 0). */
    slotActive = (stateId < 0) ? 0 : D_001A8B88[stateId];
    if (slotActive != 0) {
        result = -3;
    }
    /* gate 4 (LAST, wins): death gate. */
    if (g_health == 0) {
        result = -1;
    }

    if (result != 0) {
        return result;
    }

    /* accepted: stash pending id + args + out-flag pointer. */
    g_gameStatePendingArgA = argA;
    g_gameStatePendingArgB = argB;
    g_gameStateTransitionDoneFlag = (s32)outDoneFlag;
    g_nGameStatePending = stateId;

    /* occlusion-override player-progress special case. The asm reloads
       g_playerProgress between the two blocks; reproduce both branches exactly.
       `$2` flowing into the second block (L5A04) is always the reloaded
       progress in every path, so the second test is progress-relative. */
    {
        s32 progress = g_playerProgress;
        /* first block: (progress==8 && D_001C4EC0==1) || (progress==0x13 && D_001C4EC0==0),
           then stateId in {3,4}. */
        if ((progress == 8 && D_001C4EC0 == 1) ||
            (progress == 0x13 && D_001C4EC0 == 0)) {
            if ((u32)(stateId - 3) < 2) {
                g_occlusionOverrideMode = 2;
                if (stateId == 4) {
                    func_00286260(2);
                }
            }
        }
        /* second block: reload progress; progress in {0x16,0x17} && stateId in {3,4}. */
        progress = g_playerProgress;
        if ((u32)(progress - 0x16) < 2) {
            if ((u32)(stateId - 3) < 2) {
                g_occlusionOverrideMode = 3;
                if (stateId == 4) {
                    func_00286260(3);
                }
            }
        }
    }

    /* push the current game state onto the 8-deep stack. */
    if (push == 1) {
        s32 depth = g_gameStateStackDepth;
        g_gameStateStack[depth] = g_nGameState;
        g_gameStateStackDepth = depth + 1;
    }

    return result;
}
#endif

/* Pop the top of the 8-deep game-state stack into the pending-transition slot,
 * recording the caller's two transition args. The pop is gated: it only happens
 * when a transition is currently quiescent (pending == -2), the stack is
 * non-empty, and the player is alive (g_health != 0). Returns 0 on a successful
 * pop, 1 when there is nothing to pop (empty stack / transition already pending),
 * and -1 when the player is dead.
 * WALL: boolean-materialise idiom. The original lowers the first gate
 * (pending != -2) with a preset-1 / movz-zero idiom (matching the movz chain
 * used for the depth and health gates); this cc1 lowers the same `!=` with
 * sltu, which cascades into a different register coloring (best 63.10%).
 * Genuine codegen-idiom wall — functional equivalent only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", PopGameState);
#else
/* TODO(match): functional equivalent - not byte-exact; boolean-materialise idiom wall. */
s32 PopGameState(s32 argA, s32 argB) {
    s32 depth = g_gameStateStackDepth;
    s32 status = (g_nGameStatePending != -2);
    if (depth == 0) {
        status = 1;
    }
    if (g_health == 0) {
        status = -1;
    }
    if (status != 0) {
        return status;
    }
    g_gameStatePendingArgA = argA;
    g_gameStatePendingArgB = argB;
    g_gameStateStackDepth = depth - 1;
    g_nGameStatePending = g_gameStateStack[depth - 1];
    g_gameStateTransitionDoneFlag = 0;
    return status;
}
#endif

/* Returns the game state at the top of the 8-deep state stack, or -2 if empty. */
s32 GetGameStateStackTop(void) {
    s32 depth = g_gameStateStackDepth;
    if (depth == 0) {
        return -2;
    }
    return g_gameStateStack[depth - 1];
}

/* Returns the previous (last-frame) game state. */
s32 GetPrevGameState(void) {
    return g_nGameStatePrev;
}

/* Top-level per-frame game-state machine (commits the pending transition).
 * WALL: splat jtbl reloc-identity gap (the switch dispatch table) + save-layout;
 * see unit header. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateGameState);

/* Steer a moby toward a world point (locomotion heading helper).
 * WALL: save-layout — 3 callee-saves + $ra at 8-byte spacing, with fp temps. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", DriveMobyTowardPoint);

/* Size-pinned 8-byte epilogue pad pseudo-function — see unit header. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B5FF8);

/* Core per-frame moby locomotion step (steer / collide / ground / lean).
 * WALL: save-layout — 4 callee-saves + $ra at 8-byte spacing, with fp/madd and
 * sq/lq 128-bit moves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StepMobyMotion);

/* Advance a moby's motion using its own facing target and its +0xF8 move speed.
 * Empty asm guard blocks the sibling-call so the jal+frame is reproduced. */
void DriveMobyAlongFacing(Moby *moby) {
    StepMobyMotion(moby, &moby->facingTarget, moby->moveSpeed);
    __asm__ __volatile__("");
}

/* Advance a moby's motion toward its facing target at caller-supplied speed. */
void DriveMobyInPlace(Moby *moby, f32 speed) {
    StepMobyMotion(moby, &moby->facingTarget, speed);
    __asm__ __volatile__("");
}

/* Size-pinned 8-byte epilogue pad pseudo-function — see unit header. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B61B8);

/* Integrate a moby's motion-controller velocity for the frame.
 * WALL: save-layout — 5 callee-saves + $ra at 8-byte spacing, with fp/madd. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateMobyMotionVelocity);

/* Resolve a moby's per-frame motion against collision geometry.
 * WALL: save-layout — 6 callee-saves + $ra at 8-byte spacing, with fp/madd and
 * sq/lq 128-bit moves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ResolveMobyMotionCollision);

/* Apply ground-snap and fire ground/landing events after motion resolve.
 * WALL: save-layout — 6 callee-saves + $ra at 8-byte spacing, with fp temps. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ApplyMobyGroundAndEvents);

/* Probe the ground line below a moby (raycast for the ground normal/height).
 * WALL: save-layout — 2 callee-saves + $ra at 8-byte spacing, with fp temps. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ProbeMobyGroundLine);

/* Inherit motion from the moving platform a moby is standing on. If the
 * controller recorded a moby under the ground probe last step
 * (ctrl->groundHitMoby, +0x88), re-test the moby against that platform's
 * collision (func_002ADF48, fed the moby + the platform + the moby's pos vec4
 * at +0x10 and its prev-pos/extent vec4 at +0xF0); on a hit, raise the
 * platform-rider event bit 0x40 in ctrl->eventFlags (+0x94). No groundHitMoby
 * => nothing to inherit, return.
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). Derived
 * register-exact from CheckMobyGroundMover.s @0x2B6BF8.
 *
 * WALL (matching build): save-layout — 1 callee-save + $ra at 8-byte spacing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CheckMobyGroundMover);
#else
/* func_002ADF48 (text/1A8180 @0x2ADF48): moving-platform re-collision test;
 * returns nonzero when `moby` is still resting on `platform`. Opaque here. */
extern s32 func_002ADF48(Moby *moby, void *platform, void *posVec,
                         void *extentVec, void *posVec2, void *extentVec2);

#define MOBY_MOTION_EVENT_PLATFORM 0x40 /* +0x94: riding a moving platform this step */

void CheckMobyGroundMover(Moby *moby, MobyMotionController *ctrl) {
    void *platform = ctrl->groundHitMoby;
    if (platform != 0) {
        void *posVec = (u8 *)moby + 0x10;
        void *extentVec = (u8 *)moby + 0xF0;
        if (func_002ADF48(moby, platform, posVec, extentVec,
                          posVec, extentVec) != 0) {
            ctrl->eventFlags |= MOBY_MOTION_EVENT_PLATFORM;
        }
    }
}
#endif

/* Returns the moby's motion-controller block: *(*(moby+0x68)+0x18). */
s32 GetMobyMotionController(Moby *moby) {
    return moby->pExtra[0x18 / 4];
}

/* Resolve a moby against a collision sphere (push-out + slide).
 * WALL: save-layout — 3 callee-saves + $ra at 8-byte spacing, with fp/madd. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ResolveMobySphereCollision);

/* Constrain a moby's motion to a collision edge.
 * WALL: save-layout — 3 callee-saves + $ra at 8-byte spacing, with fp/madd. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ResolveMobyEdgeConstraint);

/* Update a moby's lean angle from its turn rate (locomotion banking). Banks the
 * one or two attached sub-mobys (ctrl->leanTargetA at +0x48, ctrl->leanTargetB
 * at +0x4C) by the clamped heading-vs-facing delta, split evenly across however
 * many are present. Runs only if at least one lean target is attached AND the
 * FREE_YAW mode bit (0x10) is clear.
 *
 * Pipeline (op-faithful to the .s @0x2B6E18):
 *   delta = WrapAnglePiDiff(headingAngle, moby->facingYaw)   // signed, [-pi,pi]
 *   delta = clamp(delta, -pi/2, +pi/2)
 *   if (moby->modeFlags & 0x8000) delta = -delta             // mirror flag
 *   recip = 1 / (presentCount)                               // 1.0 or 0.5
 *   for each present target T:
 *       T->leanAngle = clamp(WrapAnglePiSum(delta * recip, T->leanAngle),
 *                            -pi/2, +pi/2)
 * Each lean target is a sub-moby carrying its accumulated lean angle as an f32
 * at +0x68 (see MobyLeanTarget below); WrapAnglePiSum/Diff are pure f32 angle
 * math (0x284548 / 0x284590, return f32 in $f0).
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). The float
 * op ORDER is preserved exactly for bit-exactness: the reciprocal is taken once,
 * each per-target weight (fanA/fanB) is scaled by it as a separate rounding step,
 * and only then multiplied by the clamped delta — matching the .s div.s + the two
 * mul.s @0x2B6EB8/0x2B6EC0 followed by mul.s @0x2B6F0C/0x2B6F5C.
 *
 * WALL (matching build): save-layout — 2 callee-saves + $ra at 8-byte spacing,
 * with $f20..$f23 fp temps; the pinned cc1's save packing does not reproduce it. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateMobyLeanFromTurn);
#else
/* The sub-moby banked by the lean pass: only its accumulated lean angle (f32 at
 * +0x68) is touched here. Evidence: lwc1/swc1 0x68($2) in UpdateMobyLeanFromTurn.s
 * (@0x2B6F14 read, @0x2B6F1C write) — a 32-bit float load/store, not a pointer. */
typedef struct MobyLeanTarget {
    u8  pad00[0x68];
    f32 leanAngle;   /* +0x68 accumulated bank angle (radians), clamped +/-pi/2 */
} MobyLeanTarget;

#define MOBY_LEAN_MIRROR_FLAG 0x8000 /* moby->modeFlags bit: negate the lean delta */

extern f32 WrapAnglePiSum(f32 a, f32 b);   /* 0x284548  wrap a+b into [-pi,pi] */
extern f32 WrapAnglePiDiff(f32 a, f32 b);  /* 0x284590  wrap a-b into [-pi,pi] */

void UpdateMobyLeanFromTurn(f32 headingAngle, Moby *moby,
                            MobyMotionController *ctrl) {
    MobyLeanTarget *targetA = (MobyLeanTarget *)ctrl->leanTargetA;
    MobyLeanTarget *targetB = (MobyLeanTarget *)ctrl->leanTargetB;
    f32 halfPi = 1.5707965f;       /* pi/2 clamp bound (lui 0x3FC90FDC) */
    f32 fanA, fanB, recip, count, delta;

    if ((targetA == 0 && targetB == 0) ||
        (ctrl->modeFlags & MOBY_MOTION_MODE_FREE_YAW) != 0) {
        return;
    }

    /* per-target presence weights + how many targets share the bank */
    fanA = (targetA != 0) ? 1.0f : 0.0f;
    if (targetB == 0) {
        fanB = 0.0f;
        count = fanA + 0.0f;
    } else {
        fanB = 1.0f;
        count = fanA + 1.0f;
    }

    /* signed heading-vs-facing delta, clamped to +/-pi/2 */
    delta = WrapAnglePiDiff(headingAngle, moby->moveSpeed /* +0xF8 facing yaw */);
    if (delta <= halfPi) {
        if (delta < -halfPi) {
            delta = -halfPi;
        }
    } else {
        delta = halfPi;
    }
    if ((moby->modeFlags & MOBY_LEAN_MIRROR_FLAG) != 0) {
        delta = -delta;
    }

    /* scale each weight by the shared reciprocal (separate rounding, then * delta) */
    recip = 1.0f / count;
    fanA = fanA * recip;
    fanB = fanB * recip;

    if (targetA != 0) {
        targetA->leanAngle = WrapAnglePiSum(delta * fanA, targetA->leanAngle);
        if (halfPi < targetA->leanAngle) {
            targetA->leanAngle = halfPi;
        } else if (targetA->leanAngle < -halfPi) {
            targetA->leanAngle = -halfPi;
        }
    }
    if (targetB != 0) {
        targetB->leanAngle = WrapAnglePiSum(delta * fanB, targetB->leanAngle);
        if (halfPi < targetB->leanAngle) {
            targetB->leanAngle = halfPi;
        } else if (targetB->leanAngle < -halfPi) {
            targetB->leanAngle = -halfPi;
        }
    }
}
#endif

/* Size-pinned 8-byte epilogue pad pseudo-function — see unit header. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B6FC8);

/* Set a moby motion-controller's velocity / accel / max-speed profile. Argument
 * order is ($f12,$f13,$f14,$f15) on the EE; mapping recovered from the swc1
 * offsets @0x2B6FD0:
 *   accel  ($f12) -> accel (+0x20) and accelArrive (+0x24)  [written twice]
 *   maxSpd ($f13) -> maxSpeed (+0x28)
 *   speed  ($f14) -> velX (+0x14) and velY (+0x18)          [planar speed, twice]
 *   velZ   ($f15) -> velZ (+0x1C)
 * No-op if the moby has no controller.
 *
 * (.s store order: velZ, accelArrive, maxSpeed, velY, accel, velX — each lane is
 * a plain overwrite, so the final state is order-independent; the C groups them
 * by source param for readability.)
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above).
 *
 * WALL (matching build): float register save-layout — saves $f20..$f23 (swc1)
 * plus the GetMobyMotionController call forces the original's 0x30 fp-save frame,
 * which the pinned cc1's fp-save packing does not reproduce. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", SetMobyMotionParams);
#else
void SetMobyMotionParams(Moby *moby, f32 accel, f32 maxSpd, f32 speed, f32 velZ) {
    MobyMotionController *ctrl = (MobyMotionController *)GetMobyMotionController(moby);
    if (ctrl != 0) {
        ctrl->accel = accel;
        ctrl->accelArrive = accel;
        ctrl->maxSpeed = maxSpd;
        ctrl->velX = speed;
        ctrl->velY = speed;
        ctrl->velZ = velZ;
    }
}
#endif

/* Size-pinned 8-byte epilogue pad pseudo-function — see unit header. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7038);

/* Drive a moby along its waypoint path (follow + arrival logic).
 * WALL: save-layout — 3 callee-saves + $ra at 8-byte spacing, with fp temps. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", DriveMobyAlongWaypoints);

/* Size-pinned 8-byte epilogue pad pseudo-function — see unit header. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7140);

/* Install a waypoint path into a moby's motion controller. `path` is the path
 * array (path[0] is its node count as a short, see waypointPath +0xC4). `endIdx`
 * selects the destination node: pass -1 to walk to the last node (count-1),
 * otherwise it is used verbatim. `startIdx` seeds the current-node cursor.
 *
 * Returns 0 if the moby has no motion controller, else 1.
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). Derived
 * register-exact from SetMobyWaypointPath.s @0x2B7148 (the asm reuses the -1
 * sentinel both as the "no controller" return-stage value and as the endIdx==-1
 * compare operand; the C below expresses the same two effects directly).
 *
 * WALL (matching build): save-layout — 3 callee-saves + $ra at 8-byte spacing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", SetMobyWaypointPath);
#else
s32 SetMobyWaypointPath(Moby *moby, short *path, s32 endIdx, s32 startIdx) {
    MobyMotionController *ctrl = (MobyMotionController *)GetMobyMotionController(moby);
    if (ctrl == 0) {
        return 0;
    }
    ctrl->waypointPath = path;
    if (endIdx == -1) {
        ctrl->waypointEnd = (s16)(path[0] - 1);
    } else {
        ctrl->waypointEnd = (s16)endIdx;
    }
    ctrl->waypointCursor = (s16)startIdx;
    return 1;
}
#endif

/* Profiling hook: add the current EE RCNT0 count (read as unsigned, hence the
 * bltz/shift float-conversion idiom) to the accumulated moby-motion tick total
 * and bump the sample count. */
void AccumMobyMotionProfile(void) {
    g_mobyMotionProfileCount++;
    g_mobyMotionProfileTicks += (f32)(u32)(*(volatile s32 *)0x10000000);
}

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7218);

/* Test whether a moby's forward path is blocked by collision geometry.
 * WALL: save-layout — 2 callee-saves + $ra at 8-byte spacing, with fp temps. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CheckMobyPathBlocked);

/* Test whether a moby is positioned over a water volume.
 * WALL: save-layout — 3 callee-saves + $ra at 8-byte spacing, with fp temps. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CheckMobyOverWater);

/* Begin the per-frame GS draw-list: writes the frame DMA chain header (GIF/DMA
 * tags) at g_frameDmaCursor, primes the screen context, and queues the frame.
 * WALL: hardware DMA-packet builder. The original reloads g_frameDmaCursor from
 * memory after every tag store and mixes %lo-absolute and %gp_rel access to the
 * same cursor (a gp/absolute-mix the pinned cc1 won't reproduce); the literal
 * 0x30000009/0x50000009/0x70000000 GIF tags also resist clean C expression.
 * Tier-3 hardware — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", BeginFrameDrawList);

/* The builtin classId->update-fn binding table (0xC-byte entries, -1-terminated;
 * empty in the shipped ELF, filled by overlay code). All four globals below sit
 * outside the gp window / are sized > 8, so the original materialises them with
 * the lui/%lo absolute macro. */
typedef struct MobyUpdateBinding {
    /* 0x0 */ s32 classId;     /* class id key (-1 = end of table) */
    /* 0x4 */ void *pUpdate;   /* per-class update fn to install */
    /* 0x8 */ s32 pad8;
} MobyUpdateBinding;
__asm__(".extern g_builtinMobyUpdateBindings, 16");
__asm__(".extern g_mobyClassUpdateFuncs, 16");
__asm__(".extern g_mobyClassUpdateFuncsNoHeader, 16");
__asm__(".extern g_mobyClassCount, 16");
__asm__(".extern g_mobyClassCountNoHeader, 16");
extern MobyUpdateBinding g_builtinMobyUpdateBindings[];  /* 0x26E880 */
extern void *g_mobyClassUpdateFuncs[];          /* 0x1CDEC0 header-class pUpdate table */
extern void *g_mobyClassUpdateFuncsNoHeader[];  /* 0x1D0460 headerless pUpdate table */
extern s32 g_mobyClassCount;                    /* 0x1B1AC0 header-class slot count */
extern s32 g_mobyClassCountNoHeader;            /* 0x1B1AC4 headerless slot count */

/* Resolve `classId` in the builtin binding table and copy its update fn into the
 * next free slot of the appropriate per-class pUpdate table. Scans the table for
 * the first entry whose key is `classId` or the -1 terminator (counting the index
 * reached); writes that entry's fn to g_mobyClassUpdateFuncs[g_mobyClassCount] —
 * or, when `headerless`, to g_mobyClassUpdateFuncsNoHeader[g_mobyClassCountNoHeader].
 * WALL: address-CSE / induction. The original holds the table base live in a
 * preserved register ($t0) across the whole function and reuses it for the final
 * bindings[index] access; this cc1 re-materialises the base (lui/%lo) at the end,
 * which cascades into a different register coloring (best 60.81%). Genuine
 * address-rematerialise-vs-preserve wall — functional equivalent only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", BindMobyClassUpdateFunc);
#else
/* TODO(match): functional equivalent - not byte-exact; table-base address-CSE wall. */
/* WEAK (TARGET_NATIVE arm only, byte-neutral): cmp_191238d.c supplies a strong
 * BindMobyClassUpdateFunc mock; weak lets it win the link when this unit is
 * co-linked into the cmp suite (this body is untested here; gc-sections drops
 * it). Inert in the matching build (INCLUDE_ASM arm). */
__attribute__((weak))
void BindMobyClassUpdateFunc(s32 classId, s32 headerless) {
    s32 index = 0;
    while (g_builtinMobyUpdateBindings[index].classId != -1 &&
           g_builtinMobyUpdateBindings[index].classId != classId) {
        index++;
    }
    if (headerless) {
        g_mobyClassUpdateFuncsNoHeader[g_mobyClassCountNoHeader] =
            g_builtinMobyUpdateBindings[index].pUpdate;
    } else {
        g_mobyClassUpdateFuncs[g_mobyClassCount] =
            g_builtinMobyUpdateBindings[index].pUpdate;
    }
}
#endif

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7570);

/* Per-frame driver: walk the active-moby list and run each moby's class update
 * fn + draw-list binding.
 * WALL: save-layout — 1 callee-save + $ra at 8-byte spacing, plus an indirect
 * per-moby call loop. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateActiveMobys);

extern void func_0011D620(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g, s32 h, s32 i);
extern void func_00133250(s32 a, s32 b, s32 c, s32 d);
extern u8 D_001A7210[];                     /* dialog sound-channel config block */

/* Set up the dialog sound channel: configure mixer slot D_001A7210+0x38 (twice,
 * around a mid-level snd setup call), establishing the dialog-voice routing.
 * WALL: address-CSE (inverse). The original re-materialises the D_001A7210+0x38
 * address (lui/%hi+addiu/%lo) for each of the two func_0011D620 calls with no
 * callee-save (frame 0x20, single $31); this cc1 hoists the shared address into
 * a preserved $16, forcing an extra save and a 0x30 frame (best 48%). Genuine
 * rematerialise-vs-CSE wall — functional equivalent only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", InitDialogSoundChannel);
#else
/* TODO(match): functional equivalent - not byte-exact; address-CSE wall. */
void InitDialogSoundChannel(void) {
    func_0011D620((s32)(D_001A7210 + 0x38), 3, 0, 0, 0, 0, 0, 0, 0);
    func_00133250(7, 0xA000, 0, 1);
    func_0011D620((s32)(D_001A7210 + 0x38), 3, 0, 0, 0, 0, 0, 0, 0);
}
#endif

/* Sound-bank load callback: stores the loaded bank id through the (optional)
 * out-pointer. The out-pointer arrives as a 64-bit value whose low 32 bits hold
 * the address (the sign-extend prologue). */
void OnSoundBankLoaded(s32 bankId, long pOut) {
    if ((s32)pOut != 0) {
        *(s32 *)pOut = bankId;
    }
}

/* Kick the async load of the boot/global 989snd sample bank into status slot 0.
 * Resets that slot to -1 (loading) and registers OnSoundBankLoaded as the
 * completion callback (the status-slot pointer is zero-extended to 64 bits for
 * the RPC). The bank's EE address is the global-WAD base plus its TOC offset.
 * WALL: address-fold (same family as LoadLevelSoundBank / OnDialogVoiceStarted).
 * The original keeps %lo(g_listenerPosHistory) in a base register and adds
 * 0x17A0 with a separate `addiu` (two-step base); this cc1 folds 0x17A0 into the
 * symbol's %lo reloc. Functional equivalent only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", LoadGlobalSoundBank);
#else
/* TODO(match): functional equivalent - not byte-exact; address-fold wall. */
void LoadGlobalSoundBank(void) {
    g_soundBankLoadStatus[0] = -1;
    snd_BankLoadAsync(*(s32 *)(g_discToc + 0x52B0) + *(s32 *)(g_discToc + 0x529C),
                      0, OnSoundBankLoaded, (long)(u32)&g_soundBankLoadStatus[0]);
}
#endif

/* Kick an async EE-side sound-bank load of `bankAddr` for status slot
 * `bankSlot`, resetting that slot to -1 (loading) and registering
 * OnSoundBankLoaded as the completion callback (the status-slot pointer is
 * zero-extended to 64 bits for the RPC). `bankAddr` is the bank's EE address,
 * forwarded verbatim to the loader.
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). Derived
 * register-exact from LoadLevelSoundBank.s @0x2B7700.
 *
 * WALL (matching build): address-fold. The original materialises
 * `%lo(g_listenerPosHistory)` then adds 0x17A0 with a separate `addiu` (two-step
 * base); this cc1 folds the 0x17A0 into the symbol's %lo reloc (one addiu), so
 * the status-slot address computation diverges (measured 53.28%, both two-step
 * and base-pointer phrasings). Same fold artifact as OnDialogVoiceStarted /
 * LoadGlobalSoundBank. Functional equivalent only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", LoadLevelSoundBank);
#else
/* TODO(match): functional equivalent - not byte-exact; address-fold wall. */
void LoadLevelSoundBank(s32 bankAddr, s32 bankSlot) {
    g_soundBankLoadStatus[bankSlot] = -1;
    snd_BankLoadFromEE_CB(bankAddr, OnSoundBankLoaded,
                          (long)(u32)&g_soundBankLoadStatus[bankSlot]);
}
#endif

/* Kick the disc load of level sound-bank `bankSlot` if its TOC entry exists:
 * resets the bank's load-status slot to -1 (loading) and registers
 * OnSoundBankLoaded; when the TOC entry is empty, clears the status slot to 0
 * (no bank). The bank's EE address is the global-WAD base plus its TOC offset.
 * WALL: address-fold (same family as LoadLevelSoundBank / OnDialogVoiceStarted)
 * — the original adds 0x17A4/0x17A0 to %lo(g_listenerPosHistory) with a separate
 * addiu; this cc1 folds it into the %lo reloc. Functional equivalent only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", KickLevelBankDiscLoad);
#else
/* TODO(match): functional equivalent - not byte-exact; address-fold wall. */
void KickLevelBankDiscLoad(s32 bankSlot) {
    s32 tocOffset = *(s32 *)(g_discToc + 0x52E0 + bankSlot * 8);
    if (tocOffset != 0) {
        g_soundBankLoadStatus[bankSlot + 1] = -1;
        snd_BankLoadAsync(tocOffset + *(s32 *)(g_discToc + 0x529C),
                          0, OnSoundBankLoaded,
                          (long)(u32)&g_soundBankLoadStatus[bankSlot + 1]);
    } else {
        g_soundBankLoadStatus[bankSlot + 1] = 0;
    }
}
#endif

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B77E0);

extern char D_1AA198[];                    /* file-load init debug string */
extern void InitDialogSoundChannel(void);  /* 0x2B7610 dialog sound-channel setup */
extern void InstallFileLoadPump(void);     /* 0x2B7E... installs the snd-pump callback */

/* Reset the entire file-load + dialog-voice manager to its idle defaults: clear
 * every per-channel state/flag word, reset the primary dialog voice id (-1) and
 * its state byte (0x20), reset the global sound-bank id (-1), then (re)initialise
 * the dialog sound channel and install the per-snd-pump file-load pump.
 * WALL: delay-slot-fill + constant-sharing. The original sinks the soundBankId
 * (+0x24) = -1 store into the InitDialogSoundChannel call's delay slot and shares
 * the single `li -1` between it and the dialogVoiceId (+0x3C) store; this cc1
 * materialises a second -1 and schedules the +0x24 store inline (best 86.30%).
 * Genuine scheduler/coloring wall — functional equivalent only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", InitFileLoadSystem);
#else
/* TODO(match): functional equivalent - not byte-exact; delay-slot-fill + const-share wall. */
void InitFileLoadSystem(void) {
    DebugPrintStub(D_1AA198);
    g_fileLoadVoiceState.dialogVoiceId = -1;
    g_fileLoadVoiceState.dialogState = 0x20;
    g_fileLoadVoiceState.dialogFlag1 = 0;
    g_fileLoadVoiceState.dialogFlag2 = 0;
    g_fileLoadVoiceState.dialogFlag3 = 0;
    g_fileLoadVoiceState.ambientState = 0;
    g_fileLoadVoiceState.ambientFlag = 0;
    g_fileLoadVoiceState.secondaryState = 0;
    g_fileLoadVoiceState.secondaryFlag = 0;
    g_fileLoadVoiceState.tertiaryState = 0;
    g_fileLoadVoiceState.tertiaryFlag = 0;
    g_fileLoadVoiceState.soundBankId = -1;
    InitDialogSoundChannel();
    InstallFileLoadPump();
    __asm__ __volatile__("");
}
#endif

/* Register the file-load completion handler as the per-snd-pump tick callback.
 * The empty asm guard blocks cc1's sibling-call (`j`) so the original jal+frame
 * is reproduced. */
void InstallFileLoadPump(void) {
    SetSndPumpCallback(PumpFileLoadCompletion);
    __asm__ __volatile__("");
}

/* Primary dialog/voice playback entry: looks up `dialogId` across the
 * language-keyed sample tables (banded by id range 1000..6000 = dialog
 * categories), and on a hit arms the primary voice block, allocates a voice
 * handle, and plays via snd_PlaySample. The subtitle SM syncs to this state.
 * WALL: save-layout — 8 callee-saves + $ra at 8-byte spacing, plus the
 * deeply-nested id-band tree, sq/lq 128-bit handle copies, and the
 * snd_PlaySample 64-bit arg marshal. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartDialogVoice);

/* Stop the secondary dialog voice channel. No-op unless the secondary voice is
 * allocated (secondaryState != 0) and is currently in its playing state
 * (secondaryFlag == 3); in that case it sends the snd stop command and advances
 * the secondary flag to 4 (stopping). Returns 1 when it issued the stop, 0
 * otherwise.
 * WALL (matching build): save-layout — saves $16 + $31 (two callee-saves at
 * 8-byte spacing), which the pinned cc1 packs at 16-byte spacing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StopDialogVoice);
#else
/* TODO(match): functional equivalent - not byte-exact; save-layout wall. */
extern void func_00133400(void);           /* 0x133400 snd voice-stop command */
s32 StopDialogVoice(void) {
    if (g_fileLoadVoiceState.secondaryState == 0) {
        return 0;
    }
    if (g_fileLoadVoiceState.secondaryFlag != 3) {
        return 0;
    }
    func_00133400();
    g_fileLoadVoiceState.secondaryFlag = 4;
    return 1;
}
#endif

/* Start an ambient/secondary-channel voice from sample-table entry `idx`.
 * No-op if the ambient channel is already busy (ambientState != 0) or the entry
 * is empty. Arms the ambient state machine (state -1, flag 1, volume 10, sample
 * rate 48000) and plays the sample pair via snd_PlaySample with
 * OnAmbientVoiceStarted as the start callback.
 * WALL: snd_PlaySample 64-bit arg marshal. The two sample addresses are passed
 * sign-extended (dsll32/dsra32) and the callback as a zero-extended 64-bit stack
 * arg; cc1 won't reproduce the exact register/stack-slot marshaling of this 8+
 * arg call from C. Functional equivalent only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartAmbientVoice);
#else
/* TODO(match): functional equivalent - not byte-exact; snd_PlaySample arg-marshal wall. */
s32 StartAmbientVoice(s32 idx, s16 flags, s16 pan) {
    if (g_fileLoadVoiceState.ambientState != 0) {
        return 0;
    }
    if (g_dialogSampleTable[idx * 2] == 0) {
        return 0;
    }
    g_fileLoadVoiceState.ambientVolume = 10;
    g_fileLoadVoiceState.ambientSampleRate = 48000;
    g_fileLoadVoiceState.ambientFlag = 1;
    g_fileLoadVoiceState.ambientArg0 = idx;
    g_fileLoadVoiceState.ambientArg2 = pan;
    g_fileLoadVoiceState.ambientArg1 = flags;
    g_fileLoadVoiceState.ambientState = -1;
    g_fileLoadVoiceState.ambientCursor = 0;
    snd_PlaySample(g_dialogSampleBase + g_dialogSampleTable[idx * 2],
                   g_dialogSampleBase + g_dialogSampleTable[(idx + 1) * 2],
                   0, 0, pan, 0, OnAmbientVoiceStarted,
                   (long)(u32)&g_fileLoadVoiceState.ambientState);
    return 0;
}
#endif

/* Like StartAmbientVoice but gated on idx >= 0 and uses func_002B8ED0 as the
 * voice-start callback (the secondary-channel variant). No-op if the ambient
 * channel is busy or the sample-table entry is empty.
 * WALL: snd_PlaySample 64-bit arg marshal (same as StartAmbientVoice). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartSecondaryVoice);
#else
/* TODO(match): functional equivalent - not byte-exact; snd_PlaySample arg-marshal wall. */
s32 StartSecondaryVoice(s32 idx, s16 flags, s16 pan) {
    if (idx < 0) {
        return 0;
    }
    if (g_fileLoadVoiceState.ambientState != 0) {
        return 0;
    }
    if (g_dialogSampleTable[idx * 2] == 0) {
        return 0;
    }
    g_fileLoadVoiceState.ambientFlag = 1;
    g_fileLoadVoiceState.ambientVolume = 10;
    g_fileLoadVoiceState.ambientSampleRate = 48000;
    g_fileLoadVoiceState.ambientArg0 = idx;
    g_fileLoadVoiceState.ambientState = -1;
    g_fileLoadVoiceState.ambientArg2 = pan;
    g_fileLoadVoiceState.ambientArg1 = flags;
    g_fileLoadVoiceState.ambientCursor = 0;
    snd_PlaySample(g_dialogSampleBase + g_dialogSampleTable[idx * 2],
                   g_dialogSampleBase + g_dialogSampleTable[(idx + 1) * 2],
                   0, 0, pan, 0, func_002B8ED0,
                   (long)(u32)&g_fileLoadVoiceState.ambientState);
    return 0;
}
#endif

/* Chain a follow-on secondary-voice segment (the seamless continuation of an
 * already-playing ambient/secondary voice). No-op unless the channel is in the
 * chainable state (ambientFlag != 9, ambientState allocated and not -1) and the
 * next sample-table entry exists. Arms ambientFlag 9 and plays the continuation
 * sample pair via snd_PlaySample with func_002B8E78 as the start callback.
 * WALL: snd_PlaySample 64-bit arg marshal (same as StartAmbientVoice). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ChainSecondaryVoice);
#else
/* TODO(match): functional equivalent - not byte-exact; snd_PlaySample arg-marshal wall. */
s32 ChainSecondaryVoice(s32 idx, s16 flags, s16 pan) {
    if (g_fileLoadVoiceState.ambientFlag == 9) {
        return 0;
    }
    if (g_fileLoadVoiceState.ambientState == 0 ||
        g_fileLoadVoiceState.ambientState == -1) {
        return 0;
    }
    if (g_dialogSampleTable[(idx + 1) * 2] == 0) {
        return 0;
    }
    g_fileLoadVoiceState.ambientSampleRate = 48000;
    g_fileLoadVoiceState.ambientArg0 = idx;
    g_fileLoadVoiceState.ambientFlag = 9;
    g_fileLoadVoiceState.ambientVolume = 10;
    g_fileLoadVoiceState.ambientArg2 = pan;
    g_fileLoadVoiceState.ambientArg1 = flags;
    g_fileLoadVoiceState.ambientCursor = 0;
    /* start = entry[idx+3], end = entry[idx+2] (the original at 0x2B7E90 passes
       them in this order - the continuation sample plays from +3 to +2). */
    snd_PlaySample(g_dialogSampleBase + g_dialogSampleTable[(idx + 3) * 2],
                   g_dialogSampleBase + g_dialogSampleTable[(idx + 2) * 2],
                   0, 0, pan, 0, func_002B8E78,
                   (long)(u32)&g_fileLoadVoiceState.ambientState);
    return 0;
}
#endif

/* Start the tertiary (third-priority) voice channel from sample-table entry
 * `idx` if its TOC bank exists. Arms the tertiary state block (state 1, volume
 * 10, sample rate 48000) and plays via snd_PlaySample with func_002B8E28 as the
 * start callback; returns 1 on a started voice, 0 otherwise.
 * WALL: snd_PlaySample 64-bit arg marshal (same as StartAmbientVoice) — the
 * sample addresses are passed sign-extended and the callback as a zero-extended
 * 64-bit stack arg. Functional equivalent only. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartTertiaryVoice);

/* Reset all three dialog-voice channels to their idle/cleared state. First
 * drains any voice still mid-allocation: ticks the snd RPC (func_00133230) then
 * spins snd_Pump while the primary(+0x44)/tertiary(+0x8C)/secondary(+0x68) state
 * words read -1 (allocation in flight); flushes the snd command queue
 * (func_00133310) and pumps until snd_Pump reports drained; releases the channel
 * (func_00133490(1)); then clears every per-channel state/flag/param word. The
 * primary dialog voice id is preserved into ambientArg0 (+0x48) only when it was
 * already allocated (dialogVoiceId != -1), and both dialogVoiceId(+0x3C) and its
 * mirror(+0x3E) are reset to -1.
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). Derived
 * register-exact from ResetDialogVoiceChannels.s @0x2B8090.
 *
 * WALL (matching build): save-layout — 3 callee-saves + $ra at 8-byte spacing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ResetDialogVoiceChannels);
#else
/* TODO(match): functional equivalent - not byte-exact; save-layout wall. */
extern void func_00133230(void);   /* 0x133230 snd RPC tick */
extern s32  snd_Pump(void);        /* 0x133280 snd queue pump (returns busy count) */
extern void func_00133310(void);   /* 0x133310 flush snd command queue */
extern void func_00133490(s32 a);  /* 0x133490 release channel */
void ResetDialogVoiceChannels(void) {
    func_00133230();
    while (g_fileLoadVoiceState.ambientState == -1) {
        snd_Pump();
    }
    while (g_fileLoadVoiceState.tertiaryState == -1) {
        snd_Pump();
    }
    while (g_fileLoadVoiceState.secondaryState == -1) {
        snd_Pump();
    }
    func_00133310();
    while (snd_Pump() != 0) {
        /* drain the snd command queue */
    }
    func_00133490(1);

    g_fileLoadVoiceState.ambientFlag = 0;
    g_fileLoadVoiceState.ambientArg1 = 0;
    g_fileLoadVoiceState.ambientState = 0;
    if (g_fileLoadVoiceState.dialogVoiceId != -1) {
        g_fileLoadVoiceState.ambientArg0 = (s16)g_fileLoadVoiceState.dialogVoiceId;
    }
    g_fileLoadVoiceState.dialogVoiceIdPrev = -1;
    g_fileLoadVoiceState.secondaryFlag = 0;
    g_fileLoadVoiceState.dialogArg2 = 0;
    g_fileLoadVoiceState.secondaryState = 0;
    g_fileLoadVoiceState.tertiaryFlag = 0;
    g_fileLoadVoiceState.tertiaryArg1 = 0;
    g_fileLoadVoiceState.tertiaryState = 0;
    g_fileLoadVoiceState.fileLoadPhase = 0;
    g_fileLoadVoiceState.dialogVoiceId = -1;
}
#endif

/* Drive all dialog-voice channels to full volume (volume state = -0x8000, the
 * high "active" bit set, fade target 0). The secondary channel (ch1) is only
 * touched when `includeSecondary` is nonzero.
 * WALL (matching build): independent-store rescheduling. The original emits the
 * unconditional ch2(+0x98)/ch0(+0x50) block in descending-offset order; this cc1's
 * scheduler always sorts the two independent same-base stores ascending (ch0 then
 * ch2), regardless of source order or an inter-store barrier (best 87.23%). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", SetDialogVoiceVolumesMax);
#else
/* TODO(match): functional equivalent - not byte-exact; store-rescheduling wall. */
void SetDialogVoiceVolumesMax(s32 includeSecondary) {
    if (includeSecondary) {
        g_fileLoadVoiceState.ch1.volume = (s16)-0x8000;
        g_fileLoadVoiceState.ch1.fadeTarget = 0;
    }
    g_fileLoadVoiceState.ch2.volume = (s16)-0x8000;
    g_fileLoadVoiceState.ch2.fadeTarget = 0;
    g_fileLoadVoiceState.ch0.volume = (s16)-0x8000;
    g_fileLoadVoiceState.ch0.fadeTarget = 0;
}
#endif

/* Mute all three dialog-voice channels (volume state = 4). Only each channel's
 * volume word is written; the fade target is left untouched.
 * WALL (matching build): same independent-store rescheduling as
 * SetDialogVoiceVolumesMax — the original stores ch1(+0x74)/ch0(+0x50)/ch2(+0x98)
 * in that order; this cc1 reschedules the three same-base stores (best 99.71%,
 * only ordering differs). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", SetDialogVoiceVolumesMute);
#else
/* TODO(match): functional equivalent - not byte-exact; store-rescheduling wall. */
void SetDialogVoiceVolumesMute(void) {
    g_fileLoadVoiceState.ch1.volume = 4;
    g_fileLoadVoiceState.ch0.volume = 4;
    g_fileLoadVoiceState.ch2.volume = 4;
}
#endif

/* Set the fade target on each currently-active dialog-voice channel (a channel
 * is active when its volume state has the high bit set). */
void SetDialogVoiceFadeTargets(s32 target) {
    if (g_fileLoadVoiceState.ch0.volume & 0x8000) {
        g_fileLoadVoiceState.ch0.fadeTarget = target;
    }
    if (g_fileLoadVoiceState.ch2.volume & 0x8000) {
        g_fileLoadVoiceState.ch2.fadeTarget = target;
    }
    if (g_fileLoadVoiceState.ch1.volume & 0x8000) {
        g_fileLoadVoiceState.ch1.fadeTarget = target;
    }
}

/* Step one dialog-voice channel's per-frame volume-fade state machine.
 * WALL: splat jtbl reloc-identity gap (the channel-state switch dispatch table)
 * + save-layout — 3 callee-saves + $ra at 8-byte spacing; see unit header. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StepDialogVoiceChannel);

/* Per-frame tick of the dialog/voice manager: drives the primary/secondary/
 * tertiary voice state machines (volume ramps via snd_SetVoiceVolumeRamp, stop
 * via snd_StopVoice), consumes queued dialog/ambient/tertiary voices, and pumps
 * the file-load completion path.
 * WALL: switch dispatch (the (state-2) jtbl) + save-layout (3 callee-saves +
 * $ra at 8-byte spacing). Functional equivalent only. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateDialogVoiceManager);

/* Abort the in-flight CD file read: if a read is active, emit the
 * "music_StopLoad" debug string (retail no-op), send the stop command, and set
 * the abort flag so the completion callback receives success=false.
 * WALL: save-layout — saves $16 + $31 (two callee-saves at 8-byte spacing),
 * which the pinned cc1 packs at 16-byte spacing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StopFileLoad);
#else
/* TODO(match): functional equivalent - not byte-exact; save-layout wall. */
extern char D_1AA1E8[];                    /* "music_StopLoad" debug string */
void StopFileLoad(void) {
    if (g_fileLoadVoiceState.fileLoadActive != 0) {
        DebugPrintStub(D_1AA1E8);
        CdStopRead();
        g_fileLoadVoiceState.readStopped = 1;
    }
}
#endif

/* Kick an async CD file read of `sectorCount` sectors from `lbn` into `dest`.
 * Refuses while a read is already active or when sectorCount is zero. Clears the
 * completion callback, records the request, sets the active flag, and returns
 * the byte size (sectorCount << 11); on CdStartRead failure emits the
 * "load file failed to start" debug string (retail no-op) and spin-waits.
 * WALL: save-layout — saves $16/$17/$18/$19/$31 (five callee-saves at 8-byte
 * spacing), which the pinned cc1 packs at 16-byte spacing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartFileLoad);
#else
/* TODO(match): functional equivalent - not byte-exact; save-layout wall. */
extern char D_1AA1F8[];                    /* "load file failed to start" debug string */
extern s32 CdStartRead(s32 lbn, s32 sectors, s32 dest, void *rmode);  /* 0x133398 */
extern void func_002833D8(void);           /* spin-wait on fatal load failure */
/* WEAK (TARGET_NATIVE arm only, byte-neutral): cmp_191238d.c supplies a strong
 * StartFileLoad mock; weak lets it win the link when this unit is co-linked into
 * the cmp suite (this body is untested here; gc-sections drops it). Inert in the
 * matching build (INCLUDE_ASM arm). */
__attribute__((weak))
s32 StartFileLoad(s32 dest, s32 lbn, s32 sectorCount) {
    if (g_fileLoadVoiceState.fileLoadActive != 0 || sectorCount == 0) {
        return 0;
    }
    g_fileLoadVoiceState.pLoadCallback = NULL;
    g_fileLoadVoiceState.loadCallbackArg = 0;
    if (CdStartRead(lbn, sectorCount, dest,
                    (void *)((u8 *)&g_fileLoadVoiceState + 0x40)) == 0) {
        DebugPrintStub(D_1AA1F8);
        func_002833D8();
        return 0;
    }
    g_fileLoadVoiceState.fileLoadDest = dest;
    g_fileLoadVoiceState.fileLoadActive = 1;
    g_fileLoadVoiceState.fileLoadLbn = lbn;
    g_fileLoadVoiceState.fileLoadSectorCount = sectorCount;
    return sectorCount << 11;
}
#endif

/* Like StartFileLoad but also registers a completion callback fn(arg, success),
 * fired by PumpFileLoadCompletion when the read finishes.
 * WALL: save-layout — saves $16/$17/$31 (three callee-saves at 8-byte spacing). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartFileLoadWithCallback);
#else
/* TODO(match): functional equivalent - not byte-exact; save-layout wall. */
s32 StartFileLoadWithCallback(s32 dest, s32 lbn, s32 sectorCount,
                              void *callback, s32 callbackArg) {
    s32 size = StartFileLoad(dest, lbn, sectorCount);
    if (size != 0) {
        g_fileLoadVoiceState.pLoadCallback = callback;
        g_fileLoadVoiceState.loadCallbackArg = callbackArg;
    }
    return size;
}
#endif

/* Kick a raw CD read directly via CdStartRead, bypassing the g_fileLoadState
 * request record: builds a local sceCdRMode from the global read mode with the
 * spindle-speed byte overridden, clears the retry counters, then pumps snd. Used
 * by the frontend/level-staging machine that polls completion itself.
 * WALL: builds a stack-local sceCdRMode via packed byte/half stores
 * (CONCAT11 idiom) that this cc1 lowers with a different store/merge sequence;
 * also reads several un-named CD-mode globals. Tier-3 hardware glue — left as
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", KickRawFileRead);

/* Start a file load while keeping the dialog-voice system pumping (the variant
 * used during streamed-cinematic loads): pumps the dialog-voice system once,
 * kicks the file load (StartFileLoad), then pumps the dialog-voice system again,
 * and returns the byte size StartFileLoad reported.
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). Derived
 * register-exact from StartFileLoadPumpingVoice.s @0x2B8BA0 (the pump argument is
 * the literal 1 on both calls; StartFileLoad is fed dest/lbn/sectorCount in arg
 * order and its return is forwarded verbatim).
 *
 * WALL (matching build): save-layout — 3 callee-saves + $ra at 8-byte spacing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartFileLoadPumpingVoice);
#else
/* TODO(match): functional equivalent - not byte-exact; save-layout wall. */
extern s16 PumpDialogVoiceSystem(s32 active);  /* 0x2B8C00 forward decl */
extern s32 StartFileLoad(s32 dest, s32 lbn, s32 sectorCount);  /* 0x2B8A18 */
s32 StartFileLoadPumpingVoice(s32 dest, s32 lbn, s32 sectorCount) {
    s32 size;
    PumpDialogVoiceSystem(1);
    size = StartFileLoad(dest, lbn, sectorCount);
    PumpDialogVoiceSystem(1);
    return size;
}
#endif

/* Pump the dialog-voice system once per snd tick (the snd-pump entry that calls
 * UpdateDialogVoiceManager under the right gating).
 * WALL: save-layout — 2 callee-saves + $ra at 8-byte spacing. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", PumpDialogVoiceSystem);

/* Per-snd-pump tick handler (installed by InstallFileLoadPump). Only acts when
 * the pump phase arg is 1 (a load is outstanding): polls CdGetLoadStatus, holds
 * g_fileLoadState at 2 while the read is still busy, and on completion flushes
 * the cache, clears the manager's active/abort flags, and fires the registered
 * completion callback fn(arg, success) where success is false iff StopFileLoad
 * aborted the read (readStopped set).
 * WALL: g_fileLoadState materialise. The whole FlushCache/callback tail is
 * byte-exact (96%); the only delta is the `g_fileLoadState = 2` store. The
 * original splits the address into an explicit GPR (lui $3,%hi hoisted into the
 * CdGetLoadStatus beqz delay slot, sh $2,%lo($3) sunk into the b delay slot);
 * this cc1 emits it either gp_rel (1-insn, size<=15) or via the $at assembler
 * macro (size>=16) — never a hoisted cc1-allocated base reg. Genuine
 * gp/absolute-mix + delay-slot-hoist wall — functional equivalent only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", PumpFileLoadCompletion);
#else
/* TODO(match): functional equivalent - not byte-exact; g_fileLoadState materialise wall. */
void PumpFileLoadCompletion(s32 phase) {
    void *callback;
    s32 arg;
    s32 success;

    if (phase != 1) {
        return;
    }
    if (CdGetLoadStatus() != 0) {
        g_fileLoadState = 2;
        return;
    }
    FlushCache(0);
    success = (g_fileLoadVoiceState.readStopped == 0);
    callback = g_fileLoadVoiceState.pLoadCallback;
    g_fileLoadVoiceState.fileLoadActive = 0;
    g_fileLoadVoiceState.readStopped = 0;
    if (callback != NULL) {
        arg = g_fileLoadVoiceState.loadCallbackArg;
        g_fileLoadVoiceState.loadCallbackArg = 0;
        g_fileLoadVoiceState.pLoadCallback = NULL;
        ((void (*)(s32, s32))callback)(arg, success);
    }
}
#endif

/* Voice-playback callback: on a non-null handle with a set flag, advance the
 * voice state 2 -> 3. */
void func_002B8D18(s32 flag, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    if (flag == 0) {
        return;
    }
    if (handle->state == 2) {
        handle->state = 3;
    }
}

/* Primary dialog-voice start callback: record the id (mirroring it into the
 * emitter's listener block at +0x70 when the handle has a valid sample slot),
 * advance state 1 -> 2, or (when the id is zero) kick the queued dialog voice
 * from the manager's stashed parameters.
 * WALL (matching build): address-fold vs displacement. The mirror write
 * `g_listenerPosHistory[slot*0x70 + 0x70]` matches to 99.97%, but the original
 * keeps +0x70 as the store displacement (`sw $4,0x70($3)`) while this cc1 folds
 * it into the materialised base address (one instruction / reg differs). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", OnDialogVoiceStarted);
#else
/* TODO(match): functional equivalent - not byte-exact; address-fold wall. */
void OnDialogVoiceStarted(s32 voiceId, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    handle->voiceId = voiceId;
    if (handle->sampleSlot >= 0) {
        *(s32 *)(g_listenerPosHistory + handle->sampleSlot * 0x70 + 0x70) = voiceId;
    }
    if (voiceId != 0) {
        if (handle->state == 1) {
            handle->state = 2;
        }
    } else {
        StartDialogVoice(g_fileLoadVoiceState.dialogArg1,
                         g_fileLoadVoiceState.dialogArg2,
                         g_fileLoadVoiceState.dialogArg3,
                         g_fileLoadVoiceState.dialogArg0);
    }
}
#endif

/* Ambient-voice start callback: record the id and advance state 1 -> 2, or (when
 * the id is zero) kick the queued ambient voice from the manager's parameters. */
void OnAmbientVoiceStarted(s32 voiceId, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    handle->voiceId = voiceId;
    if (voiceId != 0) {
        if (handle->state == 1) {
            handle->state = 2;
        }
    } else {
        StartAmbientVoice(g_fileLoadVoiceState.ambientArg0,
                          g_fileLoadVoiceState.ambientArg1,
                          g_fileLoadVoiceState.ambientArg2);
    }
}

/* Voice-playback callback: record the id, and when the voice was in state 1
 * advance it to state 4; if additionally the voice gate is open, publish the
 * prior state (1) to the playing-state mirror. A zero id instead clears the
 * voice state to 0.
 * WALL (matching build): store-into-delay-slot scheduling. The structure matches
 * to 90.79%, but the original sinks the final `g_pendingDialogVoiceId+8` store
 * into the `jr` delay slot; this cc1 emits it before the jr (with a nop delay). */
#ifndef TARGET_NATIVE
/* Voice-playback callback: record the id, and when the voice was in state 1
 * advance it to state 4; if additionally the voice gate is open, publish the
 * prior state (1) to the playing-state mirror. A zero id instead clears the
 * voice state to 0. Structuring the id==0 case as the else-tail (rather than an
 * early return) reproduces the original's plain-beqz dispatch, and publishing
 * the captured prior state (== 1) lets cc1 reuse the loaded state register and
 * sink the mirror store into the jr delay slot. */
void func_002B8E28(s32 voiceId, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    handle->voiceId = voiceId;
    if (voiceId != 0) {
        s16 state = handle->state;
        if (state == 1) {
            handle->state = 4;
            if (handle->gate != 0) {
                g_dialogVoicePlayingState = state;
            }
        }
    } else {
        handle->state = 0;
    }
}
#else
/* TODO(match): functional equivalent - not byte-exact; delay-slot-store wall. */
void func_002B8E28(s32 voiceId, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    handle->voiceId = voiceId;
    if (voiceId == 0) {
        handle->state = 0;
        return;
    }
    if (handle->state != 1) {
        return;
    }
    handle->state = 4;
    if (handle->gate != 0) {
        g_dialogVoicePlayingState = 1;
    }
}
#endif

/* Voice-playback callback: record the id but only when it is negative (the
 * positive-id store is annulled by the original's `bltzl`); a zero id clears the
 * voice state to 0. When the voice was in state 9 advance it to state 4, and if
 * the voice gate is open publish 1 to the playing-state mirror.
 * WALL (matching build): cc1's branch-likely heuristic emits the voiceId==0 test
 * as `bnezl` (branch to the state-check with the state load annulled in the
 * delay) where the original uses a plain `beqz` branching to the state-0 block
 * (best 58-63%); structurally divergent. */
#ifndef TARGET_NATIVE
/* Voice-playback callback: record the id but only when it is negative (the
 * positive-id store is annulled by the original's `bltzl`); a zero id clears the
 * voice state to 0. When the voice was in state 9 advance it to state 4, and if
 * the voice gate is open publish 1 to the playing-state mirror. Structuring the
 * id==0 case as the else-tail (not an early return) reproduces the original's
 * plain-beqz dispatch and sinks the mirror store into the jr delay slot. */
void func_002B8E78(s32 voiceId, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    if (voiceId < 0) {
        handle->voiceId = voiceId;
    }
    if (voiceId != 0) {
        if (handle->state == 9) {
            handle->state = 4;
            if (handle->gate != 0) {
                g_dialogVoicePlayingState = 1;
            }
        }
    } else {
        handle->state = 0;
    }
}
#else
/* TODO(match): functional equivalent - not byte-exact; branch-likely codegen wall. */
void func_002B8E78(s32 voiceId, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    if (voiceId < 0) {
        handle->voiceId = voiceId;
    }
    if (voiceId == 0) {
        handle->state = 0;
        return;
    }
    if (handle->state != 9) {
        return;
    }
    handle->state = 4;
    if (handle->gate != 0) {
        g_dialogVoicePlayingState = 1;
    }
}
#endif

/* Voice-playback callback: record the id and transition state 1 -> 8 (or clear
 * to 0 when the id is zero). */
void func_002B8ED0(s32 voiceId, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    handle->voiceId = voiceId;
    if (voiceId != 0) {
        if (handle->state == 1) {
            handle->state = 8;
        }
    } else {
        handle->state = 0;
    }
}

/* Tertiary-voice start callback: warns (retail no-op) if the channel was already
 * allocated (voiceId != -1); otherwise records a zero id and sets state 7. */
void OnTertiaryVoiceStarted(s32 voiceId, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    if ((u32)handle->voiceId != 0xFFFFFFFF) {
        DebugPrintStub(D_1AA220);
        __asm__ __volatile__("");
        return;
    }
    handle->voiceId = voiceId;
    if (voiceId != 0) {
        return;
    }
    handle->state = 7;
}

/* File-load read-cursor callback: stash the just-read byte cursor into the
 * voice handle, then (only while the file-load is in its "armed" phase and the
 * cursor is non-zero) advance the phase to "reading" and publish the cursor and
 * its word count (cursor / 4) into the file-load voice manager. The handle
 * arrives as a 64-bit value whose low 32 bits hold the address.
 * WALL: reloaded-ptr CSE. The original re-loads handle->sampleCursor from memory
 * for both the +0x30 publish and the +0x34 word-count divide; this cc1 CSEs the
 * load away to the just-stored byteCursor register (best 87.04%). Genuine
 * reload-vs-CSE wall — functional equivalent only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B8F68);
#else
/* TODO(match): functional equivalent - not byte-exact; reloaded-ptr CSE wall. */
void func_002B8F68(s32 byteCursor, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    handle->sampleCursor = byteCursor;
    if (handle->gate == 0) {
        return;
    }
    if (g_fileLoadVoiceState.fileLoadPhase != 1) {
        return;
    }
    if (byteCursor == 0) {
        return;
    }
    g_fileLoadVoiceState.fileLoadPhase = 2;
    g_fileLoadVoiceState.fileLoadByteCursor = handle->sampleCursor;
    g_fileLoadVoiceState.fileLoadWordCount = handle->sampleCursor / 4;
}
#endif

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B8FD8);

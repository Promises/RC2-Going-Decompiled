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
 *   - size >= 16: cc1-small / assembler-absolute (lui/%lo macro everywhere; GNU
 *     as uses $at for a store but a load's own destination register for its
 *     %hi, so a same-register `lui $r / lX $r,%lo($r)` is this model, not a
 *     compiler split — task #744, g_rawReadSpindleCtrl);
 *   - size 9..15 (we use 12): gp-addressable / assembler-absolute (absolute
 *     macro in straight-line code, 1-insn %gp_rel only in a branch delay slot);
 *   - ABSOLUTE_GLOBAL (a <=8-byte extern in a named section): cc1 does not
 *     class it as small and splits the address itself — `lui $r,%hi(sym)` in a
 *     compiler register, hoistable, `op %lo(sym)($r)` — the form the ROM has
 *     for g_fileLoadState, g_health, g_cdReadMode
 *     (tools/ee/.t510/06_store_forms.tsv lists which model each symbol takes:
 *     a `$at` store = cc1-small model, a compiler-register store or a hoisted
 *     lui = ABSOLUTE_GLOBAL).
 *
 * SAVE-LAYOUT WALL, A cc1 2.9 PROPERTY: this TU was built by the later SN cc1
 * that packs callee-save slots 8-byte; the pinned cc1 2.9 reserves 16 bytes per
 * save. Every function below that saves two or more GPRs (incl. $ra) was taken
 * as blocked on that wall on the 2.9 arm and left INCLUDE_ASM (check the
 * prologue: two+ sd of s-regs/$ra at 8-byte spacing). On the s136os arm (SN
 * 1.36 -fopt-stack) the wall does not hold. Read from the ROM prologues at
 * master 77207b5b7, the sentence covers 37 members here: 13 are now MATCHED on
 * that arm (CheckTargetInRangeBand DriveMobyTowardPoint CheckMobyGroundMover
 * ResolveMobyEdgeConstraint DriveMobyAlongWaypoints SetMobyWaypointPath
 * CheckMobyPathBlocked StopDialogVoice StopFileLoad StartFileLoad
 * StartFileLoadWithCallback StartFileLoadPumpingVoice PumpDialogVoiceSystem),
 * and 24 are still INCLUDE_ASM. Of those 24, AcquireMobyAutoTarget and
 * StartDialogVoice were in FACT #9873's census (C arm does not compile solo),
 * CheckMobyOverWater's residual is the assembler hazard-nop (FACT #9545), and
 * for the other 21 (UpdateMobyThreatFlashAndBurst ClassifyTargetProximity
 * BindMobyToParent CalcMobyTargetThreatDist FindTargetInGroup
 * ApplyMobyLocalTransformDelta InitMobySpringFollowState StepMobySpringFollow
 * RequestGameStateChange UpdateGameState StepMobyMotion
 * UpdateMobyMotionVelocity ResolveMobyMotionCollision ApplyMobyGroundAndEvents
 * ProbeMobyGroundLine ResolveMobySphereCollision UpdateMobyLeanFromTurn
 * UpdateActiveMobys ResetDialogVoiceChannels StepDialogVoiceChannel
 * UpdateDialogVoiceManager) the save stride is NOT re-measured on the s136os
 * arm and the residual is UNMEASURED: tree-wide, 0 of the 111 labeled members
 * that were run reproduce the 16-byte stride there (FACT #9873). A per-member
 * "WALL (matching build): save-layout" below describes the cc1 2.9 arm.
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

/* Globals the original TU did NOT class as gp-small although they are <= 8
 * bytes: the ROM addresses them through a compiler-allocated register
 * (`lui $r,%hi(sym); op %lo(sym)($r)`, hoistable/schedulable), not the
 * assembler macro that the `.extern sym, 16` (cc1-small / assembler-absolute)
 * model yields (`$at` for a store; for a load the macro reuses the destination,
 * so a load whose %hi and result share one register is the macro, not this
 * model). Declaring the extern in a named section
 * is the smallest thing that makes cc1 2.9 treat it as non-small (probe:
 * tools/ee/.t510/probe/split.c). No section is emitted for an extern; the
 * native build ignores it. */
#ifdef TARGET_NATIVE
#define ABSOLUTE_GLOBAL
#else
#define ABSOLUTE_GLOBAL __attribute__((section(".data")))
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
extern s32 g_nGameStatePending;            /* 0x1A8BB8 pending state (-2 = none) */
extern s32 g_gameStateStack[8];            /* 0x1B1CD0 int[8] saved game-state ids */
extern s32 g_health ABSOLUTE_GLOBAL;  /* 0x18C2EC current health (0 = dead); non-small: the ROM splits its address in a compiler register */

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
    /* 0x18 */ void *pLoadCallback;       /* completion callback fn(void *ctx, s32 status) — CONFIRMED */
    /* 0x1C */ s32 loadCallbackArg;       /* raw word holding the callback's ctx pointer (e.g. &D_1A9330) */
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
    /* 0x44 */ u32 ambientState;          /* ambient-voice state word, init 0 */
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
    /* 0x68 */ u32 secondaryState;        /* secondary-voice state word, init 0 */
    /* 0x6C */ s16 dialogArg1;            /* queued dialog-voice params */
    /* 0x6E */ s16 dialogArg0;
    /* 0x70 */ s16 dialogArg2;
    /* 0x72 */ s16 secondaryFlag;         /* init 0 */
    /* 0x74 */ DialogVoiceChannel ch1;    /* secondary voice channel */
    /* 0x78 */ u8 pad78[0xC];
    /* 0x84 */ s32 dialogArg3;
    /* 0x88 */ u8 pad88[0x4];
    /* 0x8C */ u32 tertiaryState;         /* tertiary-voice state word, init 0 */
    /* 0x90 */ u8 pad90[0x4];
    /* 0x94 */ s16 tertiaryArg1;          /* queued tertiary-voice param, reset to 0 */
    /* 0x96 */ s16 tertiaryFlag;          /* init 0 */
    /* 0x98 */ DialogVoiceChannel ch2;    /* tertiary voice channel */
} FileLoadVoiceState;

extern FileLoadVoiceState g_fileLoadVoiceState;   /* 0x1A63A8 */

/* Callees (value-returning declarations keep cc1 from sibling-call optimising
 * forwarding tails — see text/198FA0). */
extern void SetSndPumpCallback(void *cb);  /* 0x1336D0 */
extern void PumpFileLoadCompletion(s32 phase);  /* 0x2B8CA8 snd-pump tick */
extern s32 CdStopRead(void);               /* 0x133640; returns 1 on the IOP path (cod/0321A0.c) */
extern s32 CdGetLoadStatus(void);          /* 0x133688 */
extern void func_0011AEA0(s32 mode);       /* 0x0011AEA0 FlushCache (EE kernel syscall 0x64); raw-asm glabel name */
extern s16 g_fileLoadState ABSOLUTE_GLOBAL;  /* 0x1A63AC 0 idle / 1 requested / 2 in progress */
extern void DebugPrintStub(const char *s); /* 0x26FEC8 retail debug no-op */
extern char D_1AA220[];                    /* tertiary-voice debug string */
extern void OnSoundBankLoaded(s32 bankId, long pOut);  /* 0x2B7690 forward decl */
extern void snd_BankLoadFromEE_CB(s32 arg, void *cb, long pOut);  /* 0x1325E8 */
extern void snd_BankLoadAsync(s32 bankAddr, s32 a1, void *cb, long pStatus);  /* 0x132498 */
__asm__(".extern g_discToc, 16");
extern u8 g_discToc[];                      /* 0x14B540 master disc asset directory */

/* The sound-bank load-status slots live at g_listenerPosHistory + 0x17A0 (u32
 * per bank id: slot 0 = the global bank, 1.. = the level banks); the loader
 * writes -1 there and lets OnSoundBankLoaded fill it. The loaders below take
 * the block through a local pointer so cc1 keeps the +0x17A0 field offset as a
 * displacement off the materialised base (the ROM's form) instead of folding
 * it into the symbol's %lo. */
typedef struct ListenerBlock {
    /* 0x0000 */ u8 slots[0x17A0];              /* per-emitter listener-position ring */
    union {
        /* 0x17A0 */ u32 all[4];                /* every bank slot, indexed by bank id */
        struct {
            /* 0x17A0 */ u32 global;
            /* 0x17A4 */ u32 level[3];
        } by;
    } bankLoadStatus;
} ListenerBlock;
#define g_soundBankLoadStatus ((s32 *)(g_listenerPosHistory + 0x17A0))
extern s32 StepMobyMotion(Moby *moby, Vec4 *target, f32 speed);   /* 0x2B6000 returns eventFlags (+0x94) */
extern s32 StartDialogVoice(s32 a0, s32 a1, s32 a2, s32 a3);      /* 0x2B7878 */
extern void StartAmbientVoice(s32 idx, s32 flags, s32 pan);      /* 0x2B7CA0 */
extern void StartSecondaryVoice(s32 idx, s32 flags, s32 pan);    /* 0x2B7D98 */
extern s32 snd_PlaySample(s64 sampleStart, s64 sampleEnd, s32 a2, s32 a3,
                          s32 pan, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9,
                          void *startCb, long context);  /* 0x133350 (12 args, cod/0321A0) */
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
    /* 0x18 */ union { s32 v; } sampleCursor;
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

/* Per-frame moby threat-flash + warning-burst update (0x2B4298).
 *
 * Walks up to 15 threat slots on a moby, comparing a rising threat gauge
 * (arg3) against per-slot level bytes (arg2). Two passes:
 *   1. FLASH pass (only on the frame the cached gauge is still 0 and the live
 *      gauge is nonzero): lights the moby's per-slot flash bits (moby+0x7C) for
 *      every active slot whose stored level is below the current fade fraction.
 *   2. BURST pass (only when the gauge has just fallen below its cached peak):
 *      for each lit slot whose level now exceeds the fade fraction, spawns a
 *      warning-burst effect (func_00273740) aimed on a random spread around the
 *      camera yaw, clears the slot's flash bit, and plays a single global alert
 *      sound (id 7) for the first burst of the frame.
 * The cached peak (arg2->lastValue) is refreshed to the live gauge on exit.
 *
 * arg1 = moby, arg2 = per-slot threat state (15 level bytes + a cached f32 peak
 * at +0x10), arg3 = the live threat gauge (f32 value at +0, s16 slot-count at
 * +0x4). Returns 1 if any burst was spawned this frame, else 0.
 *
 * EU divergence: the per-frame rate constant is NTSC 1/60 (0x3C888889); the PAL
 * (EU) build uses 1/50 (0x3CA3D70B) — see the SpawnParticle* frame-time notes.
 * The deg->rad factor (0x3C8EFA36) and clamp/scale consts are region-invariant.
 *
 * WALL (matching build): 8-byte-spaced callee-save layout (the pinned cc1 packs
 * at 16-byte spacing) plus $f20/$f21/$f22 temps — stays INCLUDE_ASM. This #else
 * is faithful COVERAGE only; the matching arm is unchanged. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateMobyThreatFlashAndBurst);
#else
/* Per-slot threat state (arg2). */
typedef struct MobyThreatState {
    /* 0x00 */ u8  level[16];   /* per-slot threat level bytes (slots 0..14 used) */
    /* 0x10 */ f32 lastValue;   /* gauge peak cached from the previous frame */
} MobyThreatState;

/* Live threat gauge (arg3). */
typedef struct MobyThreatGauge {
    /* 0x00 */ f32 value;       /* current threat value */
    /* 0x04 */ s16 slotCount;   /* divisor: number of slots the value spans */
} MobyThreatGauge;

extern void func_002A0480(Moby *moby, s32 *outMask, s32 *out2);  /* 0x2A0480 slot dims */
extern s32  FloatToInt(f32 x);                                   /* 0x2846A0 cvt.w.s;mfc1 */
extern f32  WrapAnglePiSum(f32 a, f32 b);                        /* 0x284548 wrap a+b to [-pi,pi] */
extern f32  func_00283B30(f32 angle);                            /* 0x283B30 cos-like leaf (VU0) */
extern f32  func_00283B48(f32 angle);                            /* 0x283B48 sin-like leaf (VU0) */
extern f32  GetRandomFloatSigned(f32 lo, f32 hi);               /* 0x2A8740 random +/-mag in lo..hi */
extern f32  GetRandomFloatRange(f32 lo, f32 hi);               /* 0x2A86E0 uniform lo..hi */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);              /* 0x2E6C90 fire-and-forget UI sound */
extern u8   g_cameraState[];                                    /* 0x1B5180; +0x158 = camera yaw f32 */
extern f32  D_1AA030;                                          /* gp-rel f32 burst-spawn param */
/* Burst-effect spawner (opaque; called by its recovered register proto per the
 * physics-park guardrail — arg *semantics* past the aim vec are unconfirmed).
 * Returns the spawned effect (or 0); the caller stores an update-callback ptr at
 * effect+0x64. */
extern void *func_00273740(Moby *moby, s32 one, s32 mobyAA, s32 slot,
                           void *posVec, void *extentVec, void *velVec,
                           s32 mask, f32 f0, f32 f1);            /* 0x273740 */
extern void func_00311A80(void);                               /* 0x311A80 burst update callback */

s32 UpdateMobyThreatFlashAndBurst(Moby *moby, MobyThreatState *ts, MobyThreatGauge *gauge)
{
    s32 didBurst = 0;
    f32 gaugeVal;
    s32 i;

    /* FLASH pass: only on the frame the cached peak is still 0 and the gauge is live. */
    if (ts->lastValue == 0.0f && gauge->value != 0.0f) {
        s32 slotMask;      /* out1 from func_002A0480: per-slot active bitmask */
        s32 slotDim;       /* out2 (unused here, written by the callee) */
        f32 v = gauge->value;
        s32 fadeByte;

        func_002A0480(moby, &slotMask, &slotDim);
        if (v < 0.0f) v = 0.0f;
        fadeByte = FloatToInt(v / (f32)gauge->slotCount * 255.0f) & 0xFF;
        for (i = 0; i < 15; i++) {
            if ((slotMask & (1 << i)) && ts->level[i] != 0 && ts->level[i] < fadeByte) {
                *(u16 *)((u8 *)moby + 0x7C) |= (u16)(1 << i);
            }
        }
    }

    /* BURST pass: only when the gauge has fallen below its cached peak. */
    gaugeVal = gauge->value;
    if (gaugeVal < ts->lastValue) {
        s32 fadeByte2;
        s32 bit = 1;                       /* 1 << i, shifted each slot */
        f32 v2 = gaugeVal;

        if (v2 < 0.0f) v2 = 0.0f;
        fadeByte2 = FloatToInt(v2 / (f32)gauge->slotCount * 255.0f) & 0xFF;
        for (i = 0; i < 15; i++, bit <<= 1) {
            if ((*(u16 *)((u8 *)moby + 0x7C) & bit) && fadeByte2 < ts->level[i]) {
                f32 heading, dist, perFrame;
                f32 vel[3];
                void *fx;

                /* Aim on a random +/-90..120 deg spread about the camera yaw. */
                heading = WrapAnglePiSum(GetRandomFloatSigned(90.0f, 120.0f) * 0.017453294f,
                                         *(f32 *)(g_cameraState + 0x158));
                dist = GetRandomFloatRange(2.0f, 4.0f);
                perFrame = dist * (1.0f / 60.0f);        /* NTSC; EU = 1/50 */
                vel[0] = func_00283B30(heading) * perFrame;   /* x = cos(heading) * d/60 */
                vel[1] = func_00283B48(heading) * perFrame;   /* y = sin(heading) * d/60 */
                vel[2] = GetRandomFloatRange(5.0f, 8.0f) * (1.0f / 60.0f);  /* z */

                fx = func_00273740(moby, 1, (s32)*(s16 *)((u8 *)moby + 0xAA), i,
                                   (u8 *)moby + 0x10, (u8 *)moby + 0xF0, vel,
                                   0xFF, D_1AA030, 15.0f);
                if (fx != 0) {
                    *(void **)((u8 *)fx + 0x64) = (void *)func_00311A80;
                }
                *(u16 *)((u8 *)moby + 0x7C) &= (u16)~bit;   /* clear this slot's flash bit */
                if (!didBurst) {
                    PlayGlobalSound(7, 0, 0);
                }
                didBurst = 1;
            }
        }
    }

    ts->lastValue = gauge->value;
    return didBurst;
}
#endif

/* --- moby auto-target acquisition: shared globals + leaves ----------------
 * g_altGravityEnabled (0x1A8CA0) is the alternate-gravity flag — nonzero when the
 * level uses a non-world-Z gravity frame. The planar sub-mode is
 * g_altGravityPlanar: func_002B0E40 consults it only with this flag set, but
 * func_002FF768 (GetGravityDirectionAtPos) takes the planar path on
 * g_altGravityPlanar alone (0x2FF7B8), so planar does not imply this flag.
 * Part of the documented gravity-direction cluster (see symbol_addrs, pinned
 * by task #1736). When set, threat scoring uses
 * true 3D distance and skips the facing-bias term.
 * D_001F0000 (0x1F0000) is the per-level effect/zone data-segment base (also
 * referenced raw by the EU effect-def code); the target-zone pointer table lives
 * at +0x1680. */
extern Moby *g_pHeroMoby;        /* 0x18C0B0 hero (Ratchet) moby — lock-on anchor */
extern s32   g_nGameState;       /* 0x1A8BB0 current top-level game/screen state id */
extern s32   g_altGravityEnabled; /* 0x1A8CA0 alternate-gravity flag (see above) */
extern u8    D_001F0000[];       /* 0x1F0000 per-level effect/zone data segment */

extern f32 DistXYVu0(Vec4 *a, Vec4 *b);   /* 0x283830 horizontal XY distance, VU0 */
extern f32 Dist3DVu0(Vec4 *a, Vec4 *b);   /* 0x2837F8 3D (xyz) distance, VU0 */
extern f32 Atan2fPoly(f32 y, f32 x);      /* 0x283BF8 2-arg arctangent (VU0/FPU) */
extern f32 WrapAngleAbsDiff(f32 a, f32 b);/* 0x284630 |a-b| folded into [0,pi] */

/* Point-in-zone test: 1 if hero-pos lies inside the named target zone polygon,
 * else 0. (0x2A9958 — self-contained crossing-number test.) */
extern s32 func_002A9958(Vec4 *probePos, Vec4 *polyPoints, s32 count);

/* Target-zone pointer table (D_001F0000 + 0x1680): one pointer per zone id; each
 * points to a record whose +0x0 is the polygon vertex count and +0x10 the Vec4
 * vertex array. */
#define g_targetZoneTable ((void **)(D_001F0000 + 0x1680))

/* Classify a candidate into a proximity band relative to the hero (player).
 *
 * Writes the current hero moby into *outHero (always), then — only while in
 * gameplay (g_nGameState == 0) — measures the horizontal distance from the
 * candidate position to the hero and returns a band code:
 *   2 (near)  : zone `nearZoneId` contains the hero (polygon test) OR, when
 *               nearZoneId == -1, the distance is below `nearRadius`;
 *   1 (mid)   : likewise for `midZoneId` / `midRadius`;
 *   0 (none)  : neither band matched (or not in gameplay).
 * The candidate position is `posOverride` when non-NULL, else the candidate
 * moby's own world position (candidate+0x10).
 *
 * @param candidate   the moby being classified (its +0x10 pos is the default probe)
 * @param outHero     out: receives g_pHeroMoby
 * @param nearZoneId  zone id for the near band, or -1 to use nearRadius
 * @param midZoneId   zone id for the mid band, or -1 to use midRadius
 * @param posOverride optional explicit probe position (NULL -> candidate pos)
 * @param nearRadius  near-band radius (used when nearZoneId == -1)
 * @param midRadius   mid-band radius (used when midZoneId == -1)
 * @return proximity band 0/1/2
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). Derived
 * register-exact from ClassifyTargetProximity.s @0x2B4588.
 *
 * WALL (matching build): save-layout — 3 callee-saves + $ra at 8-byte spacing,
 * with fp temps. cmp-oracle validated. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ClassifyTargetProximity);
#else
s32 ClassifyTargetProximity(Moby *candidate, Moby **outHero, s32 nearZoneId,
                            s32 midZoneId, Vec4 *posOverride,
                            f32 nearRadius, f32 midRadius) {
    Vec4 probePos;
    Moby *hero;
    f32 dist;

    probePos = posOverride ? *posOverride : candidate->facingTarget;
    hero = g_pHeroMoby;
    *outHero = hero;
    if (g_nGameState != 0) {
        return 0;
    }

    dist = DistXYVu0(&probePos, &hero->facingTarget);

    if (nearZoneId != -1) {
        void *zone = g_targetZoneTable[nearZoneId];
        if (func_002A9958(&(*outHero)->facingTarget,
                          (Vec4 *)((u8 *)zone + 0x10), *(s32 *)zone) != 0) {
            return 2;
        }
    } else if (dist < nearRadius) {
        return 2;
    }

    if (midZoneId != -1) {
        void *zone = g_targetZoneTable[midZoneId];
        if (func_002A9958(&(*outHero)->facingTarget,
                          (Vec4 *)((u8 *)zone + 0x10), *(s32 *)zone) != 0) {
            return 1;
        }
    } else if (dist < midRadius) {
        return 1;
    }

    return 0;
}
#endif

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B46C8);

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

/**
 * One-shot latch of moby anim-flag bit 1 (`animFlags` byte at moby+0xBE).
 *
 * @param moby moby whose animFlags byte is latched
 * @return     1 if the bit was newly set (byte rewritten), 0 if it was already
 *             set (byte left untouched)
 *
 * The read-modify-write on `flags` is load-bearing, not style: writing
 * `moby->animFlags = flags | 2;` instead produces a fresh value in a second
 * register and costs the match -- that form measures 52.22% and is what this
 * file shipped in its inert #else arm until now. Updating `flags` in place lets
 * cc1 reuse $v1 for the OR result (`ori v1,v1,2`), which in turn frees the tail
 * block to sink the `sb` into the final `jr` delay slot. Both differences come
 * from the one change -- see the sibling SetMobyFlagBit0 above, which matches
 * with the `flags | 1` form because its original tests via `xori`.
 *
 * Byte-exact: 36 B cmp IDENTICAL against the ROM, 9/9 words, target_relocs ==
 * base_relocs == 0 (audit /19066; independently re-derived against the retail
 * boot ELF and master's unmodified objdiff_build.sh in /19147). The reloc
 * equality is trivially satisfied here -- this leaf carries no relocation at
 * all -- so the discriminating evidence is the byte comparison, not 0 == 0.
 */
s32 SetMobyFlagBit1(Moby *moby) {
    u8 flags = moby->animFlags;
    if ((flags & 2) != 0) {
        return 0;
    }
    flags |= 2;
    moby->animFlags = flags;
    return 1;
}

/* Test moby anim-flag bit 2. */
s32 func_002B47D0(Moby *moby) {
    return moby->animFlags & 4;
}

/* Auto-target threat/scoring metric: how "costly" `target` is for `moby` to
 * engage. Lower = more attractive. Starts as the planar XY distance between the
 * two mobys' world positions (moby+0x10 / target+0x10), then:
 *   - under alternate gravity (g_altGravityEnabled != 0) it uses the full 3D distance
 *     instead and applies no facing penalty;
 *   - otherwise it adds a facing-misalignment penalty: the angle between the
 *     direction toward the target (atan2 of the XY delta) and the moby's own
 *     facing yaw (moby+0xF8), scaled by the base distance — so targets behind
 *     the moby score worse;
 *   - finally, if the target IS the hero moby, a flat +42 bias is added so the
 *     player is de-prioritised relative to equidistant enemies.
 *
 * @param moby   the seeking moby (its world pos +0x10 and facing yaw +0xF8)
 * @param target the candidate target moby (its world pos +0x10)
 * @return the threat-distance score (f32)
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). Derived
 * register-exact from CalcMobyTargetThreatDist.s @0x2B47E0.
 *
 * WALL (matching build): save-layout — 4 callee-saves + $ra at 8-byte spacing,
 * with an $f20 fp temp. cmp-oracle validated. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CalcMobyTargetThreatDist);
#else
f32 CalcMobyTargetThreatDist(Moby *moby, Moby *target) {
    f32 score;

    if (g_altGravityEnabled != 0) {
        score = Dist3DVu0(&moby->facingTarget, &target->facingTarget);
    } else {
        f32 dx = target->facingTarget.x - moby->facingTarget.x;
        f32 dy = target->facingTarget.y - moby->facingTarget.y;
        f32 facingPenalty;
        score = DistXYVu0(&moby->facingTarget, &target->facingTarget);
        facingPenalty = WrapAngleAbsDiff(Atan2fPoly(dx, dy),
                                         moby->moveSpeed /* +0xF8 facing yaw */);
        score += facingPenalty * score;
    }
    if (target == g_pHeroMoby) {
        score += 42.0f;
    }
    return score;
}
#endif

/* A target group's candidate list: a count-prefixed array of 8-byte entries.
 * entry[i].moby is the candidate moby ptr; entry[i].tag a per-entry id byte. The
 * entry-0 +0x4 slot doubles as the list's entry count (the per-entry moby ptr
 * only occupies +0x0..+0x3, leaving +0x4/+0x5 for count/tag). */
typedef struct TargetListEntry {
    /* 0x0 */ Moby *moby;
    /* 0x4 */ u8 count;    /* meaningful only in entry 0: total entry count */
    /* 0x5 */ u8 tag;      /* per-entry id byte */
    /* 0x6 */ u8 pad[2];
} TargetListEntry;

/* Target-group descriptor (group stride 0x30). desc[0].groupCount holds the
 * number of valid groups; the per-group refresh-stamp halfword sits at +0x3A
 * (the field reaches past the 0x30 stride into the shared tail block, matching
 * the original `sh ...,0x3A(desc)`). The struct view is only large enough to
 * cover the two touched fields. */
typedef struct TargetGroupDesc {
    /* 0x00 */ s32 groupCount;   /* number of valid groups (read from desc[0]) */
    /* 0x04 */ u8 pad04[0x36];
    /* 0x3A */ s16 refreshStamp; /* re-stamped to 10 on each scan */
} TargetGroupDesc;

extern TargetGroupDesc *g_mobyTargetGroupDescs; /* 0x1B1740 group descriptor table */
extern TargetListEntry **g_mobyTargetGroupLists;/* 0x1B173C per-group list ptr array */
extern s32 g_gameTime;                          /* 0x1B1608 global frame counter */

/* Scan a moby group for the best auto-target by threat distance: walks the
 * candidate list of target group `groupIdx` and returns the moby with the lowest
 * CalcMobyTargetThreatDist score, also writing that candidate's tag byte
 * (entry +0x5) into *outTag. Returns 0 (and writes nothing) when the group index
 * is out of range or the game has been running fewer than 2 frames.
 *
 * The list head is re-stamped with a 10-frame refresh value (desc+0x3A = 10) on
 * entry. An empty list (count 0) returns 0; a single-entry list (count 1)
 * short-circuits — *outTag gets entry-0's tag and entry-0's moby is returned
 * without scoring; otherwise every non-NULL candidate is scored and the lowest
 * wins.
 *
 * @param seekingMoby the moby looking for a target (passed to the scorer)
 * @param groupIdx    target-group index
 * @param outTag      out: receives the chosen candidate's tag byte
 * @return the best candidate moby, or 0 if none/out-of-range/too-early
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). Derived
 * register-exact from FindTargetInGroup.s @0x2B48B8.
 *
 * WALL (matching build): save-layout — 6 callee-saves + $ra at 8-byte spacing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", FindTargetInGroup);
#else
Moby *FindTargetInGroup(Moby *seekingMoby, s32 groupIdx, s32 *outTag) {
    TargetGroupDesc *desc;
    TargetListEntry *list;
    s32 count;
    s32 i;
    Moby *best;
    f32 bestScore;

    if (groupIdx < 0 || groupIdx >= g_mobyTargetGroupDescs[0].groupCount) {
        return 0;
    }
    if (g_gameTime < 2) {
        return 0;
    }

    desc = (TargetGroupDesc *)((u8 *)g_mobyTargetGroupDescs + groupIdx * 0x30);
    desc->refreshStamp = 10;

    list = g_mobyTargetGroupLists[groupIdx];
    count = list[0].count;
    if (count == 0) {
        return 0;
    }
    if (count == 1) {
        *outTag = g_mobyTargetGroupLists[groupIdx][0].tag;
        return g_mobyTargetGroupLists[groupIdx][0].moby;
    }

    best = 0;
    bestScore = 9999999.0f;   /* 0x4B18967F — initial "worse than anything" score */
    for (i = 0; i < count; i++) {
        Moby *candidate = g_mobyTargetGroupLists[groupIdx][i].moby;
        if (candidate != 0) {
            f32 score = CalcMobyTargetThreatDist(seekingMoby, candidate);
            if (score < bestScore) {
                bestScore = score;
                *outTag = g_mobyTargetGroupLists[groupIdx][i].tag;
                best = g_mobyTargetGroupLists[groupIdx][i].moby;
            }
        }
        count = g_mobyTargetGroupLists[groupIdx][0].count;
    }
    return best;
}
#endif

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
 * Save layout: the ROM saves $s0/$s1/$ra at 8-byte spacing (frame 0x30), which
 * the pinned cc1 2.9 cannot emit (16-byte spacing, frame 0x40).
 * MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
 * byte-exact (FACT #8810; census FACT #8830). cmp-oracle validated.
 * GUARD (task #1270): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_CheckTargetInRangeBand)
S136OS_SLOT(CheckTargetInRangeBand);
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

/* Auto-target lock-on result block. The block POINTER is stored at
 * moby->pExtra + 0xC (i.e. lock = *(MobyLockOn **)((u8*)moby->pExtra + 0xC)).
 * The scan writes the chosen target, a small status/group code, and the lock-on
 * anchor position into it. */
typedef struct MobyLockOn {
    /* 0x00 */ Vec4 anchorPos;    /* lock-on world anchor (default vec, hero pos,
                                     or chosen target's +0x10 position) */
    /* 0x10 */ Moby *target;      /* chosen auto-target moby (or hero) */
    /* 0x14 */ s32  hasTarget;    /* 1 if a target was locked, 0 otherwise */
    /* 0x18 */ u8   pad18[0x4];
    /* 0x1C */ s32  groupCode;    /* group/tag code of the chosen target */
} MobyLockOn;

/* Candidate-list views for the three special auto-target scan pools. Groups A
 * and C are count-prefixed halfword arrays of moby-table slot indices (the moby
 * pointer is g_mobyTableBase + slot * 0x100); group B is a count + a flat array
 * of direct moby pointers. */
typedef struct TargetSlotList {
    /* 0x00 */ s16 count;
    /* 0x02 */ s16 slot[1];       /* slot[0] is array index 1 (entry[1..count]) */
} TargetSlotList;

/* Group A scan pool (g_deferredSegment2Tag + 0x130): moby-class 0xCB candidates;
 * a candidate is eligible only when its state byte (+0x20) == 3. */
#define g_targetGroupA (*(TargetSlotList *)((u8 *)&g_deferredSegment2Tag + 0x130))
/* Group C scan pool (g_deferredSegment2Tag + 0x170): moby-class 0xEB0
 * candidates; the state byte (+0x20) 0xFE/0xFD are rejected. */
#define g_targetGroupC (*(TargetSlotList *)((u8 *)&g_deferredSegment2Tag + 0x170))
/* Group B scan pool: a direct moby-pointer array (g_collTriBuffer + 0x11E8) of
 * length D_1A8CD4. */
extern s32 D_1A8CD4;                          /* 0x1A8CD4 group-B candidate count */
#define g_targetGroupB ((Moby **)((u8 *)g_collTriBuffer + 0x11E8))

extern u8 g_deferredSegment2Tag[];            /* 0x1B1B00 render anchor + scan pools */
extern u8 g_collTriBuffer[];                  /* 0x1C0180 collision tri buffer + pool */
extern Moby *g_mobyTableBase;                 /* 0x1B1ADC moby entity array base */
extern u8 g_soundBankHandlesBlk[];            /* 0x189E20 (+0x2290 hero, +0xD0 vec) */
#define g_heroStateCode (*(s32 *)(g_soundBankHandlesBlk + 0x2294)) /* 0x18C0B4 */
#define g_lockDefaultVec (*(Vec4 *)(g_soundBankHandlesBlk + 0xD0)) /* 0x189EF0 */

/* Returns the moby's pvars/extra pointer (moby->pExtra[0]) when the moby exists
 * and carries the auto-target mode flag (mode +0x34 bit 0x20); else 0. */
extern s32 *func_002AC058(Moby *moby);
/* Re-anchors the lock-on block's position from the target's transform: rotates
 * (0, 0, lock+0x10) by the target's orientation (+0xC0) and adds the target
 * position, writing into lock+0x0. */
extern void func_002B0BF0(Moby *target, MobyLockOn *lock, f32 x, f32 y, f32 z);

/* Acquire the moby's auto-target (the top-level target-lock entry point).
 *
 * Drives the auto lock-on for `moby`. Requires `moby` to carry the auto-target
 * mode flag (mode +0x34 bit 0x20) — otherwise returns 0 with no effect. The
 * result is written into the moby's lock-on block, whose POINTER is stored at
 * moby->pExtra + 0xC (lock = *(MobyLockOn **)(pExtra + 0xC)): the chosen target,
 * a status flag (+0x14), the group/tag code (+0x1C) and the lock-on anchor
 * position (+0x0).
 *
 * The probe position is `posOverride` (arg2) when non-NULL, else the moby's own
 * world position (moby+0x10). The lock-on block is first reset: target = hero,
 * anchor = the default vec, flag/code = 0.
 *
 * When `forceScan` (arg3) is 0 the function only scans while in gameplay
 * (g_nGameState == 0) and with the hero in the non-busy state (g_heroState !=
 * 0x6F); otherwise it returns 0. When `groupIdx` (arg1) is a valid group it
 * delegates to FindTargetInGroup; when groupIdx == -1 it runs the three special
 * scan pools (A: class 0xCB / state 3; B: a direct pointer list; C: class 0xEB0)
 * and keeps the lowest-threat in-range candidate, with the hero itself as an
 * eligible band-1 fallback.
 *
 * For a chosen non-hero target the anchor is re-derived from the target's
 * transform (via func_002B0BF0 under radial gravity, else by accumulating the
 * target's +0x10 z into the lock anchor z); the hero/default target keeps the
 * default vec.
 *
 * @param moby        the seeking moby (mode +0x34 bit 0x20 gates the whole fn)
 * @param groupIdx    target group index, or -1 to run the special scan pools
 * @param posOverride optional explicit probe position (NULL -> moby+0x10)
 * @param forceScan   when 0, gate the scan on gameplay + hero state
 * @param radius      range-band horizontal radius (passed to CheckTargetInRangeBand)
 * @param vertBand    range-band vertical gap
 * @return the lock-on status flag (1 if a target was locked, else 0)
 *
 * NATIVE SHIM (no byte target; matching build uses INCLUDE_ASM above). Derived
 * register-exact from AcquireMobyAutoTarget.s @0x2B4AC0.
 *
 * WALL (matching build, cc1 2.9 arm): save-layout — 8 callee-saves + $ra at 8-byte
 * spacing, with three fp temps. The pinned cc1 2.9 packs callee-save slots at
 * 16-byte spacing, so it can't reproduce this frame.
 * Save stride NOT re-measured for this member on the s136os arm: its C arm does not compile
 * solo there (NOTE #9871). Of the 111 labeled members that were, 0 reproduce the 16-byte save
 * stride there (FACT #9873), so the stride is not evidence that this member is walled.
 * Residual: UNMEASURED. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", AcquireMobyAutoTarget);
#else
s32 AcquireMobyAutoTarget(Moby *moby, s32 groupIdx, Vec4 *posOverride,
                          s32 forceScan, f32 radius, f32 vertBand) {
    Vec4 probePos;
    MobyLockOn *lock;
    Moby *hero;
    s32 tag;
    s32 *xform;

    if ((moby->modeFlags & 0x20) == 0) {
        return 0;
    }

    lock = *(MobyLockOn **)((u8 *)moby->pExtra + 0xC);
    probePos = posOverride ? *posOverride : moby->facingTarget;

    hero = g_pHeroMoby;
    lock->target = hero;
    lock->anchorPos = g_lockDefaultVec;
    lock->groupCode = 0;
    lock->hasTarget = 0;

    if (forceScan == 0) {
        if (g_nGameState != 0) {
            return 0;
        }
        if (g_heroStateCode == 0x6F) {
            return 0;
        }
    }

    if (groupIdx != -1) {
        /* Single-group lock-on. */
        Moby *target = FindTargetInGroup(moby, groupIdx, &tag);
        if (target == 0) {
            return lock->hasTarget;
        }
        lock->target = target;
        lock->hasTarget = 1;
        lock->groupCode = tag;
        if (target == g_pHeroMoby) {
            return lock->hasTarget;
        }
        lock->anchorPos = target->facingTarget;
        xform = func_002AC058(target);
        if (xform == 0) {
            return lock->hasTarget;
        }
        if (g_altGravityEnabled != 0) {
            func_002B0BF0(target, lock, 0.0f, 0.0f, *(f32 *)((u8 *)xform + 0x10));
            return lock->hasTarget;
        }
        lock->anchorPos.z += *(f32 *)((u8 *)xform + 0x10);
        return lock->hasTarget;
    } else {
        /* Multi-pool scan: keep the lowest-threat in-range candidate. */
        Moby *best = 0;
        s32 bestCode = 0;
        f32 bestScore = 9999999.0f;   /* 0x4B18967F argmin sentinel (mtc1 $f20) */
        s32 i;

        hero = (Moby *)*(s32 *)(g_soundBankHandlesBlk + 0x2290);
        if (CheckTargetInRangeBand(hero, &probePos, radius, vertBand)) {
            f32 score = CalcMobyTargetThreatDist(moby, hero);
            if (score < bestScore) {
                best = hero;
                bestScore = score;
                bestCode = 1;
            }
        }

        /* Group A: class 0xCB candidates in state 3. */
        for (i = 1; i <= g_targetGroupA.count; i++) {
            Moby *cand = (Moby *)((u8 *)g_mobyTableBase + (g_targetGroupA.slot[i - 1] << 8));
            u8 stateByte;
            if (cand == 0) {
                continue;
            }
            if (*(s16 *)((u8 *)cand + 0xAA) != 0xCB) {
                continue;
            }
            stateByte = *((u8 *)cand + 0x20);
            if (stateByte == 0xFE) {
                continue;
            }
            if (stateByte == 0xFD) {
                continue;
            }
            if (stateByte != 3) {
                continue;
            }
            if (CheckTargetInRangeBand(cand, &probePos, radius, vertBand)) {
                f32 score = CalcMobyTargetThreatDist(moby, cand);
                if (score < bestScore) {
                    best = cand;
                    bestScore = score;
                    bestCode = 2;
                }
            }
        }

        /* Group B: a direct moby-pointer list. */
        for (i = 0; i < D_1A8CD4; i++) {
            Moby *cand = g_targetGroupB[i];
            if (CheckTargetInRangeBand(cand, &probePos, radius, vertBand)) {
                f32 score = CalcMobyTargetThreatDist(moby, cand);
                if (score < bestScore) {
                    best = cand;
                    bestScore = score;
                    bestCode = 3;
                }
            }
        }

        /* Group C: class 0xEB0 candidates. */
        for (i = 1; i <= g_targetGroupC.count; i++) {
            Moby *cand = (Moby *)((u8 *)g_mobyTableBase + (g_targetGroupC.slot[i - 1] << 8));
            u8 stateByte;
            if (cand == 0) {
                continue;
            }
            if (*(s16 *)((u8 *)cand + 0xAA) != (s16)0xEB0) {
                continue;
            }
            stateByte = *((u8 *)cand + 0x20);
            if (stateByte == 0xFE) {
                continue;
            }
            if (stateByte == 0xFD) {
                continue;
            }
            if (CheckTargetInRangeBand(cand, &probePos, radius, vertBand)) {
                f32 score = CalcMobyTargetThreatDist(moby, cand);
                if (score < bestScore) {
                    best = cand;
                    bestScore = score;
                    bestCode = 4;
                }
            }
        }

        if (best == 0) {
            return lock->hasTarget;
        }
        lock->groupCode = bestCode;
        lock->hasTarget = 1;
        lock->target = best;
        if (best == (Moby *)*(s32 *)(g_soundBankHandlesBlk + 0x2290)) {
            lock->anchorPos = g_lockDefaultVec;
            return lock->hasTarget;
        }
        lock->anchorPos = best->facingTarget;
        xform = func_002AC058(best);
        if (xform == 0) {
            return lock->hasTarget;
        }
        if (g_altGravityEnabled != 0) {
            func_002B0BF0(best, lock, 0.0f, 0.0f, *(f32 *)((u8 *)xform + 0x10));
            return lock->hasTarget;
        }
        lock->anchorPos.z += *(f32 *)((u8 *)xform + 0x10);
        return lock->hasTarget;
    }
}
#endif

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently (no prologue/jr of its own). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B4F40);

/* True if moby is the backing object of level-object slot `slot`
 * (D_2403D0[slot].mobyPtr == moby). Entry stride 0xA430, moby ptr at +0x4. */
s32 CheckMobyIsLevelObjectSlot(Moby *moby, s32 slot) {
    return D_2403D0[slot].mobyPtr == (s32)moby;
}

/* func_002B4F78: 8-byte zero pad between functions (splat drops all-zero
 * inter-function regions) — raw-word filler keeps the unit size==span byte-exact
 * (06333c5 precedent; NO re-split). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B4F78);

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B4F80);

/* Apply a moby's local-transform delta (spring-follow position step).
 * WALL: save-layout — 6 callee-saves + $ra at 8-byte spacing, with fp temps
 * and sq/lq 128-bit matrix moves; matching arm stays INCLUDE_ASM, portable #else below.
 *
 * Applies a local rotation delta (rotX/rotY/rotZ) to the moby's orientation (euler at
 * +0xF0) and shifts its position (+0x10) so the local offset `posDelta` stays fixed in
 * world space across the rotation: transforms the offset by the pre- and post-delta
 * orientation matrices and adds the difference to the position (a pivot correction). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ApplyMobyLocalTransformDelta);
#else
extern void func_00283DC0(Vec4 *destMat, Vec4 *euler);       /* 0x283DC0 build a matrix from euler angles */
extern void func_00283A70(Vec4 *dst, Vec4 *vec, Vec4 *mat);  /* 0x283A70 transform a vector by a matrix */
extern void MatrixMultiplyVu0(Vec4 *dst, Vec4 *a, Vec4 *b);  /* dst = a * b */
extern void MatrixToEulerAngles(Vec4 *mat, Vec4 *dstEuler);
extern void Vec4SubVu0(Vec4 *dst, Vec4 *a, Vec4 *b);         /* dst = a - b */
extern void Vec4AddVu0(Vec4 *dst, Vec4 *a, Vec4 *b);         /* dst = a + b */
void ApplyMobyLocalTransformDelta(Moby *moby, Vec4 *posDelta, f32 rotX, f32 rotY, f32 rotZ) {
    Vec4 delta = *posDelta;
    Vec4 mat[4];         /* current orientation matrix, then combined with the delta */
    Vec4 deltaMat[4];    /* rotation matrix built from (rotX,rotY,rotZ) */
    Vec4 euler;          /* {rotX,rotY,rotZ} */
    Vec4 offsetBefore;   /* posDelta transformed by the pre-delta orientation */
    Vec4 offsetAfter;    /* posDelta transformed by the post-delta orientation */
    Vec4 pivotShift;     /* offsetBefore - offsetAfter */

    func_00283DC0(mat, (Vec4 *)((u8 *)moby + 0xF0));
    func_00283A70(&offsetBefore, &delta, mat);

    ((f32 *)&euler)[0] = rotX;
    ((f32 *)&euler)[1] = rotY;
    ((f32 *)&euler)[2] = rotZ;
    func_00283DC0(deltaMat, &euler);
    MatrixMultiplyVu0(mat, mat, deltaMat);
    func_00283A70(&offsetAfter, &delta, mat);

    Vec4SubVu0(&pivotShift, &offsetBefore, &offsetAfter);
    Vec4AddVu0((Vec4 *)((u8 *)moby + 0x10), (Vec4 *)((u8 *)moby + 0x10), &pivotShift);
    MatrixToEulerAngles(mat, (Vec4 *)((u8 *)moby + 0xF0));
}
#endif

/* Size-pinned 8-byte epilogue pad pseudo-function — see unit header; kept
 * INCLUDE_ASM permanently. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B50B8);

/* Initialise a moby's spring-follow state block (0x2B50C0).
 *
 * Populates the caller's `state` block from a moby's transform plus a set of
 * per-channel default vectors, each of which the caller may override:
 *   state[0x00] (Vec4) = base position. If `targetPos` is given, it is the
 *      target transformed by the moby orientation (moby+0xC0) then offset by the
 *      moby position (moby+0x10); otherwise it is just the moby position.
 *   state[0x10] (Vec4) = the raw target vector (only when `targetPos` given).
 *   state[0x20] (Vec4) = facing vector — `facingVec` if given, else the moby's
 *      own facing at +0xF0.
 *   state[0x48..0xA7] = eight consecutive 3-float channels. Each channel is
 *      filled from its caller override pointer when non-null, otherwise from a
 *      const default vector (D_1AA0xx). Two channels are special: +0x84 is
 *      seeded from GetRandomAngle (one draw per element) and +0x90 is scaled by
 *      a small constant (0x3998825C). Also raises the moby's spring-follow-active
 *      mode bit (0x100 at moby+0x34).
 *
 * The overrides arrive as arguments 5..11 (7 stack/reg pointers); each is named
 * here by the state offset it feeds. Region-invariant (no PAL/NTSC constants).
 *
 * WALL (matching build): 8 callee-saves + $ra at 8-byte spacing plus $f20 temp
 * — stays INCLUDE_ASM. In the original the const defaults are copied to stack
 * scratch and the override selection is a movn/movz over those buffers; that is
 * a register-materialisation artifact, so this faithful #else selects the const
 * default pointer directly (same bytes read). COVERAGE only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", InitMobySpringFollowState);
#else
/* Const per-channel default vectors (each a 3-float vec3), in the unit rodata. */
extern const f32 D_1AA060[3], D_1AA070[3], D_1AA080[3], D_1AA090[3];
extern const f32 D_1AA0A0[3], D_1AA0B0[3];

extern void func_00283A48(Vec4 *dst, Vec4 *src, Vec4 *matrix);  /* 0x283A48 transform pt by matrix (sibling of func_00283A70) */
extern void Vec4AddVu0(Vec4 *dst, Vec4 *a, Vec4 *b);            /* 0x283670 dst = a + b (VU0) */
extern f32  GetRandomAngle(void);                               /* 0x2A87A8 random angle -pi..pi */

/* Per-element scale for the +0x90 channel (raw 0x3998825C ~= 2.9089e-4f). */
static const union { u32 u; f32 f; } kChan90Scale = { 0x3998825C };

void InitMobySpringFollowState(Moby *moby, void *state,
                               Vec4 *targetPos, Vec4 *facingVec,
                               const f32 *ovr9C, const f32 *ovr90,
                               const f32 *ovr48, const f32 *ovr54,
                               const f32 *ovr6C, const f32 *ovr60,
                               const f32 *ovr78)
{
    u8 *st = (u8 *)state;
    const f32 *s48, *s54, *s60, *s6C, *s78, *s90, *s9C;
    s32 k;

    moby->modeFlags |= 0x100;   /* raise the spring-follow-active mode bit */

    /* state[0x00] = base position; state[0x10] = raw target (when supplied). */
    if (targetPos != 0) {
        func_00283A48((Vec4 *)st, targetPos, (Vec4 *)((u8 *)moby + 0xC0));
        Vec4AddVu0((Vec4 *)st, (Vec4 *)st, (Vec4 *)((u8 *)moby + 0x10));
        *(Vec4 *)(st + 0x10) = *targetPos;
    } else {
        *(Vec4 *)st = *(Vec4 *)((u8 *)moby + 0x10);
    }

    /* state[0x20] = facing vector (arg, else the moby's own facing at +0xF0). */
    if (facingVec != 0) {
        *(Vec4 *)(st + 0x20) = *facingVec;
    } else {
        *(Vec4 *)(st + 0x20) = *(Vec4 *)((u8 *)moby + 0xF0);
    }

    /* Per-channel source = caller override if non-null, else the const default. */
    s48 = ovr48 ? ovr48 : D_1AA060;
    s54 = ovr54 ? ovr54 : D_1AA070;
    s60 = ovr60 ? ovr60 : D_1AA0A0;
    s6C = ovr6C ? ovr6C : D_1AA080;
    s78 = ovr78 ? ovr78 : D_1AA090;
    s90 = ovr90 ? ovr90 : D_1AA0B0;
    s9C = ovr9C ? ovr9C : D_1AA090;

    /* Fill the eight 3-float channels at state+0x48..+0xA7 element-by-element. */
    for (k = 0; k < 3; k++) {
        *(f32 *)(st + 0x48 + k * 4) = s48[k];
        *(f32 *)(st + 0x54 + k * 4) = s54[k];
        *(f32 *)(st + 0x60 + k * 4) = s60[k];
        *(f32 *)(st + 0x6C + k * 4) = s6C[k];
        *(f32 *)(st + 0x78 + k * 4) = s78[k];
        *(f32 *)(st + 0x84 + k * 4) = GetRandomAngle();
        *(f32 *)(st + 0x90 + k * 4) = s90[k] * kChan90Scale.f;
        *(f32 *)(st + 0x9C + k * 4) = s9C[k];
    }
}
#endif

/* StepMobySpringFollow(moby, state, target) - per-frame damped spring-follow step
 * that chases `moby` toward a target position while adding a per-axis procedural
 * wobble, and orients + advances the moby by the result. `state` is the follow
 * block populated by InitMobySpringFollowState (stiffness +0x48, damping +0x54,
 * clamp limits +0x60/+0x64/+0x68, drive scale +0x6C, first-frame seed gain +0x78,
 * wobble angle +0x84 / angular-velocity +0x90 / amplitude +0x9C; runtime spring
 * position +0x30, velocity +0x3C; +0xAC first-frame counter). state+0x00 is the
 * desired target position and state+0x10 the smoothed anchor.
 *
 * When `target` is NULL the follow anchor comes from the camera-key block
 * (g_soundBankHandlesBlk): if this moby is the active key holder (+0x33C) and the
 * key has been held >= 2 frames (+0x340) it springs toward g_heroPos, otherwise it
 * ticks the first-frame arm-counter (TickCountdownTimer, state+0xAC) and skips straight
 * to the integrator without refreshing the delta. The drive
 * accel is (-delta.y, delta.x, -1) scaled by state+0x6C; on the first frame the
 * velocity is seeded by state+0x78 and the counter is armed to 10. The integrator
 * runs a 3-axis damped spring (vel += k*(accel-pos); vel -= c*vel; pos += vel) and
 * overwrites the delta with amplitude*cos(wobbleAngle) per axis. The planar offset
 * is clamped to +/-state+0x60/+0x64, transformed through the moby's 3x4 matrix
 * (VU0 micro ops), applied as a correction to moby+0x10, then func_002B0FC0
 * advances the follower with the yaw clamped to +/-state+0x68. Finally any attached
 * sub-object (func_002ADF18) is moved by this frame's position delta (func_002AE558).
 *
 * Engine region (ee-gcc 2.96); the original's 8-byte-spaced callee-save frame + fp
 * scheduling won't reproduce under the pinned 2.9 cc1, so this is a faithful #else.
 *
 * FLAGGED for audit (judgment calls, not byte-gated): (a) the camera-key block is
 * the CONFIRMED symbol g_soundBankHandlesBlk (0x189E20) - the +0x33C key-moby /
 * +0x340 hold-count field roles are inferred from the gate, not independently
 * traced; (b) helper arg orders were read per-call from the .s (Ghidra prints the
 * FP-first calls WrapAnglePiSum/func_002B0FC0 with the float arg reordered). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StepMobySpringFollow);
#else
extern s32  TickCountdownTimer(void *counter);              /* 0x2832F8 (declared again below) */
extern Vec4 g_heroPos;                /* 0x189EA0 hero world position */
extern u8   g_soundBankHandlesBlk[];  /* 0x189E20 camera-key block; head = fallback anchor Vec4 */
extern void func_002B0C40(u8 *moby, Vec4 *inDelta, Vec4 *outDelta, s32 flag);
extern void func_00283DA0(void *dst, void *src);            /* VU0 micro build/transform */
extern void func_002840E8(void *dstMtx, void *aMtx, void *bMtx);  /* VU0 3x3 matrix multiply */
extern void func_002B0FC0(f32 yawLimit, u8 *moby, Vec4 *inPos, Vec4 *outPos);
extern void *func_002ADF18(u8 *moby);                      /* -> attached sub-object or NULL */
extern void func_002AE558(void *obj, Vec4 *posDelta, Vec4 *auxDelta, void *mobyAux);
extern f32  func_00283B48(f32 angle);                      /* CosfVu0 */
extern f32  WrapAnglePiSum(f32 a, f32 b);                  /* 0x284548 */
extern void Vec4AddVu0(Vec4 *out, Vec4 *a, Vec4 *b);       /* 0x283670 */
extern void Vec4SubVu0(Vec4 *out, Vec4 *a, Vec4 *b);       /* 0x2836A0 */

void StepMobySpringFollow(u8 *moby, Vec4 *state, Vec4 *target)
{
    u8 *st = (u8 *)state;
    f32 *pos    = (f32 *)(st + 0x30);
    f32 *vel    = (f32 *)(st + 0x3C);
    f32 *stiff  = (f32 *)(st + 0x48);
    f32 *damp   = (f32 *)(st + 0x54);
    f32 *scale  = (f32 *)(st + 0x6C);
    f32 *seed   = (f32 *)(st + 0x78);
    f32 *angle  = (f32 *)(st + 0x84);
    f32 *angVel = (f32 *)(st + 0x90);
    f32 *amp    = (f32 *)(st + 0x9C);
    f32  limitX = *(f32 *)(st + 0x60);
    f32  limitY = *(f32 *)(st + 0x64);
    f32  yawLimit = *(f32 *)(st + 0x68);

    Vec4 savedPos = *(Vec4 *)(moby + 0x10);   /* pre-update world pos snapshot */
    Vec4 savedAux = *(Vec4 *)(moby + 0xF0);   /* pre-update aux vector snapshot */
    Vec4 accel;
    Vec4 delta;
    Vec4 clampPos;
    Vec4 posVec;
    u8   mtxScratch[48];
    Vec4 xformPt;
    Vec4 correction;
    Vec4 moveDelta;
    f32 *accelP = (f32 *)&accel;
    f32 *deltaP = (f32 *)&delta;
    f32  s, v;
    s32  i;

    memset(&accel, 0, 0x10);

    if (target == 0) {
        /* No explicit target: track the camera-key holder toward the hero, but
           only once this moby owns the key (+0x33C) and has held it >= 2 frames
           (+0x340). Otherwise this is a snap frame - tick the arm-counter at
           state+0xAC and run the integrator without refreshing the delta. */
        if (*(u8 **)(g_soundBankHandlesBlk + 0x33C) != moby ||
            *(s32 *)(g_soundBankHandlesBlk + 0x340) < 2) {
            TickCountdownTimer(st + 0xAC);
            goto integrate;
        }
        Vec4SubVu0(&delta, &g_heroPos, (Vec4 *)(moby + 0x10));
    } else {
        Vec4SubVu0(&delta, target, (Vec4 *)(moby + 0x10));
    }

    /* Project the target-relative delta into moby space and re-subtract the
       smoothed anchor, then derive the spring drive accel from it. */
    func_002B0C40(moby, &delta, &delta, 0);
    Vec4SubVu0(&delta, &delta, (Vec4 *)(st + 0x10));
    accelP[0] = -delta.y * scale[0];
    accelP[1] =  delta.x * scale[1];
    accelP[2] = -scale[2];

    if (*(s32 *)(st + 0xAC) == 0) {         /* first frame: seed velocity, arm counter */
        for (i = 0; i < 3; i++)
            vel[i] += accelP[i] * seed[i];
        *(s32 *)(st + 0xAC) = 10;
    }

integrate:
    /* 3-axis damped spring; delta is overwritten with the amplitude*cos wobble. */
    for (i = 0; i < 3; i++) {
        s = vel[i] + stiff[i] * (accelP[i] - pos[i]);
        s = s - damp[i] * s;
        vel[i] = s;
        pos[i] += s;
        angle[i] = WrapAnglePiSum(angle[i], angVel[i]);
        deltaP[i] = amp[i] * func_00283B48(angle[i]);
    }

    posVec.x = pos[0] + delta.x;
    posVec.y = pos[1] + delta.y;
    posVec.z = 0.0f;
    posVec.w = 0.0f;

    /* Clamp the planar offset to the per-axis limits. */
    clampPos = posVec;
    if (clampPos.x > limitX)       clampPos.x = limitX;
    else if (clampPos.x < -limitX) clampPos.x = -limitX;
    if (clampPos.y > limitY)       clampPos.y = limitY;
    else if (clampPos.y < -limitY) clampPos.y = -limitY;

    /* Transform the clamped offset through the moby's 3x4 matrix and apply it. */
    func_00283DA0(mtxScratch, &clampPos);
    func_00283DA0(moby + 0xC0, (Vec4 *)(st + 0x20));
    func_002840E8(moby + 0xC0, moby + 0xC0, mtxScratch);
    posVec = *(Vec4 *)(st + 0x10);
    posVec.z = posVec.z + delta.z;
    func_00283A48(&xformPt, &posVec, (Vec4 *)(moby + 0xC0));
    Vec4AddVu0(&xformPt, &xformPt, (Vec4 *)(moby + 0x10));
    Vec4SubVu0(&correction, state, &xformPt);
    Vec4AddVu0((Vec4 *)(moby + 0x10), (Vec4 *)(moby + 0x10), &correction);

    /* Advance the follower with the yaw clamped to +/-yawLimit. */
    v = -*(f32 *)(st + 0x38);
    if (v > yawLimit)       v = yawLimit;
    else if (v < -yawLimit) v = -yawLimit;
    func_002B0FC0(v, moby, (Vec4 *)(moby + 0x10), (Vec4 *)(moby + 0x10));

    /* Propagate this frame's motion to any attached sub-object. */
    {
        void *obj = func_002ADF18(moby);
        if (obj != 0) {
            Vec4SubVu0(&moveDelta, (Vec4 *)(moby + 0x10), &savedPos);
            func_002AE558(obj, &moveDelta, &savedAux, moby + 0xF0);
        }
    }
}
#endif

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B58B8);

/* Arm the game-state transition timer (frames to stall before the pending switch). */
void SetGameStateTransitionTimer(s32 frames) {
    g_gameStateTransitionTimer = frames;
}

/* func_002B58D0: 8-byte zero pad between functions (splat drops all-zero
 * inter-function regions) — raw-word filler keeps the unit size==span byte-exact
 * (06333c5 precedent; NO re-split). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B58D0);

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
 * The original builds the status as a movz chain off a register copy of the
 * constant 1: `li a3,1; move t0,a3; movz t0,$0,<pending^-2>; movz t0,a3,<depth>;
 * movz t0,<-1>,<health>`. Plain C does not get there with cc1 2.9: cse folds the
 * copy to the constant and combine then merges the preset into the first gate,
 * which gives `sltu` (task #655). An empty "+r" asm on `status` right after the
 * copy hides the preset value, and the chain comes out as movz/movz/movz. Loading
 * the stack top into `top` before the stores puts the depth store ahead of the
 * pending store, as in the ROM. sdk29 82.07% solo (unit objdiff report,
 * objdiff_build.sh + unit_report.sh, VM colima-ee-x86, task #792). Without `top`
 * 81.90, without the asm 71.03. The body before #792 scored 70.86 (sdk29) and
 * 49.31 (engine96).
 * Residual, measured, not a spelling question: the empty asm is an insn of its
 * own. sched2 gives it an issue slot and puts it on the dependence path between
 * the copy and the first movz. The ROM has no such insn, so the head schedule
 * stays one cycle off. The rest, read word by word against the ROM (task #808):
 * the head re-materialises the constant (`li v0,1` as the depth movz source)
 * where the ROM copies it (`daddu $8,$7,$0`) and reuses a3; the -1 goes to $3
 * where the ROM uses $2, while the head's other $2/$3 uses (pending, -2,
 * %hi(g_health)) match. The pop block has $2/$3 swapped throughout and issues two
 * pairs in the other order (addu / sw argA, lw / sw argB). `status` lives in a3
 * rather than t0, depth in t0 rather than t1, and argA is copied to a2 rather
 * than a3. Moving the copy into the asm (`"=r"(status) : "0"(blocked)`)
 * gives the ROM's colouring for the whole chain (a3/t0/t1/a2) but scores 78.03,
 * because of that same asm slot. A `register ... asm("$7")` pin (task #756, 81.72)
 * is not needed for this score. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", PopGameState);
#else
/* TODO(match): functional equivalent - not byte-exact; SCHED/REGALLOC (asm-link issue slot + $2/$3 colouring), sdk29 82.07% solo. */
s32 PopGameState(s32 argA, s32 argB) {
    s32 depth = g_gameStateStackDepth;
    s32 blocked = 1;
    s32 status = blocked;
    __asm__("" : "+r"(status));
    if (g_nGameStatePending == -2) {
        status = 0;
    }
    if (depth == 0) {
        status = blocked;
    }
    if (g_health == 0) {
        status = -1;
    }
    if (status == 0) {
        s32 top = g_gameStateStack[depth - 1];
        g_gameStatePendingArgA = argA;
        g_gameStatePendingArgB = argB;
        g_gameStateStackDepth = depth - 1;
        g_nGameStatePending = top;
        g_gameStateTransitionDoneFlag = 0;
    }
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

/* Top-level per-frame game-state machine (commits the pending transition). Runs
 * the pending state change once its stall delay elapses: rolls g_nGameStatePrev,
 * runs the LEAVE handler for the current state (g_nGameState), sets the transition
 * timer, runs the ENTER handler for the pending state (g_nGameStatePending), and —
 * unless an enter handler aborts the commit (only the level-exit fade, state 2,
 * while the fade is still ramping up) — finalizes the switch (g_nGameState =
 * pending, pending = -2) and pulses the caller's done-flag. Early-outs when there
 * is no pending change (pending == -2), while the stall delay counts down, or when
 * the player is dead (g_health == 0).
 * WALL: two splat jtbl reloc-identity gaps (jtbl_0026CB90 leave + jtbl_0026CBC0
 * enter) + save-layout; matching arm stays INCLUDE_ASM, portable #else below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateGameState);
#else
extern f32 g_cameraProjScale;   /* 0x1B9070 projection scale cot(fov/2) */
extern f32 g_exitFadeRate;      /* 0x1A8C7C exit fade ramp rate */
extern f32 g_screenFadeBlack;   /* 0x1B1520 black screen fade level 0..1 */
extern s32 g_frameGpuTime;      /* 0x1B1610 per-frame GS time measure (+0x4 = travel/revive flag) */
extern s32 D_1ABE00;            /* 0x1ABE00 default push-state mode id */

extern void EnterCinematicBeginPlayback(void);
extern void ExitCinematicSceneTeardown(void);
extern void func_0026FF18(s32 a0, s32 a1, f32 f0, f32 f1, f32 f2, f32 f3);  /* 0x26FF18 camera tween setup */
extern void BuildCameraProjection(void);                 /* 0x27B0A0 rebuild projection from g_cameraProjScale */
extern void TickFrontEndScreenIdle(void);
extern void TickPauseOverlayState(void);
extern void TeardownVendorSceneRestorePlayer(void);
extern void RunRespawnScenePlayerRestore(void);          /* 0x2E80A0 */
extern void UnhideAllMobysAndPopState(void);             /* 0x2E0198 */
extern void func_00300118(void);
extern void func_002919A0(void);
extern void func_002833D8(void);                         /* default leave/enter handler */
extern s32  GetMenuOverlayMode(void);                    /* 0x286160 */
extern void RequestMenuScreenChange(s32 screenId);       /* 0x2CAC28 */
extern void func_002F6B98(s32 arg);
extern void EnterMenuOverlayMode(s32 modeId, s32 arg);   /* 0x286270 */
extern void func_002AB150(f32 *value, f32 target, f32 step);  /* 0x2AB150 approach-value fade */
extern void PlayLevelCinematic(s32 sceneId);             /* 0x2F6220 */
extern void RequestLevelExit(s32 destination, s32 commitSave);  /* 0x2896D8 */
extern void BeginRespawnScene(void);                     /* 0x2E8010 */
extern void StartTravelToLevel(s32 level);               /* 0x2E7EE8 */
extern void RevivePlayerMinHealth(void);                 /* 0x2E7E70 */
extern void EnterVendorMenu(s32 arg);                    /* 0x2F8FF8 */
extern void HideAllMobysAndPushState(s32 modeId, s32 arg);  /* 0x2E00F8 */
extern void func_002FFF68(void);
extern void func_00291980(void);

void UpdateGameState(void) {
    /* Exact non-canonical float bit patterns, materialised via a named static so
     * ee-gcc 2.9 accepts the address-of (it rejects &(compound-literal)). */
    static const u32 kCameraArg = 0x3F8FEA69;
    static const u32 kExitFadeStep = 0x3DCCCCCE;
    s32 leaveState = g_nGameState;
    s32 pending;
    s32 abort = 0;

    g_nGameStatePrev = leaveState;
    if (g_nGameStatePending == -2) {
        return;
    }
    if (g_gameStateTransitionDelay > 0) {
        g_gameStateTransitionDelay--;
        return;
    }
    if (g_health == 0) {
        return;
    }

    /* --- LEAVE handler for the current state (jtbl_0026CB90) --- */
    switch (leaveState) {
    case 0:
        break;
    case 1:
        EnterCinematicBeginPlayback();
        goto camera_setup;
    case 2:
        ExitCinematicSceneTeardown();
    camera_setup:
        g_cameraProjScale = 0.62f;
        func_0026FF18(0, 3, *(f32 *)&kCameraArg, 0.005f, 0.2f, 0.0f);
        BuildCameraProjection();
        break;
    case 3:
        if (g_nGameStatePending != 4 || g_gameStatePendingArgA == 8) {
            TickFrontEndScreenIdle();
        }
        break;
    case 4:
        TickPauseOverlayState();
        break;
    case 5:
        TeardownVendorSceneRestorePlayer();
        break;
    case 6:
        if (g_gameStatePendingArgA != 4) {
            RunRespawnScenePlayerRestore();
        }
        break;
    case 7:
        UnhideAllMobysAndPopState();
        break;
    case 8:
        func_00300118();
        break;
    case 9:
        func_002919A0();
        break;
    default:
        func_002833D8();
        break;
    }

    SetGameStateTransitionTimer(10);
    abort = 0;

    /* --- ENTER handler for the pending state (jtbl_0026CBC0) --- */
    pending = g_nGameStatePending;
    switch (pending) {
    case 0:
        break;
    case 1:
        func_002F6B98(g_gameStatePendingArgB);
        break;
    case 2:
        if ((u32)(g_nGameState - 1) < 2) {
            PlayLevelCinematic(g_gameStatePendingArgA);
        } else {
            func_002AB150(&g_exitFadeRate, 1.2f, *(f32 *)&kExitFadeStep);
            g_screenFadeBlack = g_exitFadeRate;
            if (1.0f < g_exitFadeRate) {
                g_screenFadeBlack = 1.0f;
            }
            if (1.2f <= g_exitFadeRate) {
                PlayLevelCinematic(g_gameStatePendingArgA);
                g_exitFadeRate = 1.0f;
            } else {
                abort = 1;
            }
        }
        break;
    case 3:
        if (g_nGameStatePrev != 4 || GetMenuOverlayMode() == 8) {
            RequestMenuScreenChange(g_gameStatePendingArgA);
        }
        break;
    case 4:
        EnterMenuOverlayMode(g_gameStatePendingArgA, g_gameStatePendingArgB);
        break;
    case 5:
        EnterVendorMenu(g_gameStatePendingArgA);
        break;
    case 6:
        if (g_gameStatePendingArgA == 5) {
            RequestLevelExit(g_gameStatePendingArgB, 1);
        } else if (g_gameStatePendingArgA == 6) {
            BeginRespawnScene();
        } else {
            if (g_nGameStatePrev == 3) {
                StartTravelToLevel(g_gameStatePendingArgB);
            } else {
                RevivePlayerMinHealth();
            }
            (&g_frameGpuTime)[1] = 1;
        }
        break;
    case 7: {
        s32 modeId = g_gameStatePendingArgA;
        if (modeId == 0) {
            modeId = D_1ABE00;
        }
        HideAllMobysAndPushState(modeId, g_gameStatePendingArgB);
        break;
    }
    case 8:
        func_002FFF68();
        break;
    case 9:
        func_00291980();
        break;
    default:
        func_002833D8();
        break;
    }

    /* --- commit the transition unless an enter handler aborted it --- */
    if (abort == 0) {
        if (g_gameStateTransitionDoneFlag != 0) {
            *(u8 *)g_gameStateTransitionDoneFlag = 1;
        }
        g_nGameStatePending = -2;
        g_nGameState = pending;
        g_gameStateTransitionDoneFlag = 0;
    }
}
#endif

/* Steer a moby toward a world point (locomotion heading helper). Returns 8 when the
 * moby has no motion controller. Otherwise computes the heading to `target` from the
 * moby position (+0x10/+0x14) via Atan2fPoly (atan2), offsets it by `headingOffset`
 * (WrapAnglePiSum), and — if the controller is wandering (wanderAmplitude != 0) — adds
 * the wander offset and, when the wander timer expires (func_00283328), flips the
 * wander sign and reseeds the timer from [wanderIntervalMin, wanderIntervalMax]. Drives
 * the motion with StepMobyMotion at that heading and returns its event flags.
 * Save layout: 3 callee-saves + $ra at 8-byte spacing, with fp temps.
 * MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
 * byte-exact (FACT #8810; census FACT #8830).
 * GUARD (task #1270): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_DriveMobyTowardPoint)
S136OS_SLOT(DriveMobyTowardPoint);
#else
extern s32 GetMobyMotionController(Moby *moby);    /* defined below; forward decl */
extern f32 WrapAnglePiSum(f32 a, f32 b);           /* 0x284548 wrap a+b into [-pi,pi] */
extern f32 Atan2fPoly(f32 dx, f32 dy);          /* 0x283BF8 atan2-style heading from a planar delta */
extern s32 func_00283328(s16 *timer);              /* 0x283328 tick a frame timer; nonzero when it expires */
extern s32 RandRangeInclusive(s32 min, s32 max);   /* inclusive integer RNG */
s32 DriveMobyTowardPoint(Moby *moby, Vec4 *target, f32 headingOffset) {
    MobyMotionController *ctrl = (MobyMotionController *)GetMobyMotionController(moby);
    f32 heading;

    if (ctrl == 0) {
        return 8;
    }

    heading = WrapAnglePiSum(
        Atan2fPoly(((f32 *)target)[0] - *(f32 *)((u8 *)moby + 0x10),
                      ((f32 *)target)[1] - *(f32 *)((u8 *)moby + 0x14)),
        headingOffset);

    if (ctrl->wanderAmplitude != 0.0f) {
        heading = WrapAnglePiSum(heading, ctrl->wanderAmplitude);
        if (func_00283328(&ctrl->wanderTimer) != 0) {
            ctrl->wanderAmplitude = -ctrl->wanderAmplitude;
            ctrl->wanderTimer = (s16)RandRangeInclusive(ctrl->wanderIntervalMin, ctrl->wanderIntervalMax);
        }
    }

    return StepMobyMotion(moby, target, heading);
}
#endif

/* Size-pinned 8-byte epilogue pad pseudo-function — see unit header. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B5FF8);

/* Core per-frame moby locomotion step (steer / collide / ground / lean). Driven
 * once per moby per frame by the Drive* wrappers. Returns 8 when the moby has no
 * motion controller, and short-circuits (returning the current eventFlags) if it
 * has already stepped this frame (lastUpdateTime == g_gameTime). Otherwise it
 * rolls eventFlags into eventFlagsPrev and clears it, optionally inherits a moving
 * platform (unless MODE_NO_GROUNDMOVER), snapshots the entry position, re-seeds the
 * ground probe + per-frame state whenever it missed a frame (gap >= 2) or has never
 * run (lastUpdateTime == 0), stamps lastUpdateTime, then runs the pipeline
 * (velocity -> collision -> ground/events -> lean -> profile) and returns the
 * accumulated eventFlags.
 * WALL: save-layout — 4 callee-saves + $ra at 8-byte spacing, with fp/madd and
 * sq/lq 128-bit moves; matching arm stays INCLUDE_ASM, portable #else below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StepMobyMotion);
#else
extern s32 GetMobyMotionController(Moby *moby);                 /* defined below; forward decl */
extern void CheckMobyGroundMover(Moby *moby, MobyMotionController *ctrl);           /* below */
extern void ProbeMobyGroundLine(Moby *moby, MobyMotionController *ctrl, f32 depth); /* below */
extern void func_00283638(Vec4 *vec);                          /* 0x283638 reset vec to default up normal / clear */
extern void UpdateMobyMotionVelocity(Moby *moby, MobyMotionController *ctrl, Vec4 *target,
                                     f32 speed, Vec4 *stepOut, Vec4 *velOut);        /* 0x2B60C0 */
extern void ResolveMobyMotionCollision(Moby *moby, MobyMotionController *ctrl,
                                       Vec4 *entryPos, Vec4 *velOut);                /* 0x2B6488 */
extern void ApplyMobyGroundAndEvents(Moby *moby, MobyMotionController *ctrl,
                                     Vec4 *target, Vec4 *entryPos);                  /* 0x2B67B8 */
extern void UpdateMobyLeanFromTurn(f32 headingAngle, Moby *moby, MobyMotionController *ctrl); /* below */
extern void AccumMobyMotionProfile(void);                      /* below; RCNT0 timing accumulators */

s32 StepMobyMotion(Moby *moby, Vec4 *target, f32 speed) {
    MobyMotionController *ctrl = (MobyMotionController *)GetMobyMotionController(moby);
    Vec4 entryPos, stepOut, velOut;
    s32 lastTime;

    if (ctrl == 0) {
        return 8;
    }
    if (g_gameTime == ctrl->lastUpdateTime) {
        return ctrl->eventFlags;
    }

    ctrl->eventFlagsPrev = ctrl->eventFlags;
    ctrl->eventFlags = 0;
    if ((ctrl->modeFlags & MOBY_MOTION_MODE_NO_GROUNDMOVER) == 0) {
        CheckMobyGroundMover(moby, ctrl);
    }

    entryPos = *(Vec4 *)((u8 *)moby + 0x10);

    lastTime = ctrl->lastUpdateTime;
    if (g_gameTime - lastTime >= 2 || lastTime == 0) {
        ProbeMobyGroundLine(moby, ctrl, 0.0f);
        ctrl->animSeqActive = 0xFF;
        ctrl->stateFlags &= ~2;
        ctrl->turnDelta = 0.0f;
        ctrl->wallContactCount = 0;
        func_00283638((Vec4 *)ctrl->velAccum);
    }

    ctrl->lastUpdateTime = g_gameTime;
    UpdateMobyMotionVelocity(moby, ctrl, target, speed, &stepOut, &velOut);
    ResolveMobyMotionCollision(moby, ctrl, &entryPos, &velOut);
    ApplyMobyGroundAndEvents(moby, ctrl, target, &entryPos);
    UpdateMobyLeanFromTurn(speed, moby, ctrl);
    AccumMobyMotionProfile();
    return ctrl->eventFlags;
}
#endif

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B61B8);

/* Integrate a moby's motion-controller velocity for the frame. Called by
 * StepMobyMotion (0x2B6000) as the steer+accelerate stage:
 *   1. Unless FREE_YAW, steer the moby's yaw toward the commanded velocity
 *      (func_002AB868, writes ctrl->turnDelta).
 *   2. Pick the drive heading `angle`: if there was a wall contact this frame,
 *      face away from it (atan2 of wallContact-pos, +pi) but limit the turn to
 *      +/- g_mobyMaxTurnRate off the caller's `speed` heading; otherwise drive
 *      straight along `speed`.
 *   3. Pick the drive magnitude `arriveSpeed`: arrive profile
 *      sqrt(2*(dist-arriveRadius)*accelArrive) clamped to maxSpeed; when inside
 *      the arriveAngle dead-band or inside arriveRadius, drive a zero command
 *      (decelerate to a stop) instead.
 *   4. Resolve the planar velocity: velCmd = (cos,sin)*arriveSpeed - velAccum,
 *      length-clamped to accel (or accelArrive when reversing), integrated into
 *      the accumulator; ctrl->speed = |velAccum|.
 *   5. Unless NO_VERTICAL, run the vertical/ground step: above ground fall by
 *      decel (+ project onto the ground normal when just leaving a slope, raise
 *      FELL_STUCK if stepped off); at/below ground clamp the step and ease
 *      verticalVel toward the surface (func_002ABAE8).
 *
 * Params: moby (record; pos @+0x10, yaw @+0xF8), ctrl (MobyMotionController),
 * target (world XY goal), speed (the caller's drive heading — used as an ANGLE
 * here, not a magnitude; UNCONFIRMED whether it doubles as a speed elsewhere).
 * stepOut/velOut are passed by StepMobyMotion but this callee ignores them (the
 * asm reads no arg beyond $6/$f12) — kept in the prototype to match the caller.
 *
 * REGION (flag-not-guess): the big-turn gate constant is 30.0f (USA) / 25.0f
 * (EU 0x41C80000 @0x2b5ef8); the func_002ABAE8 ease rate is 0x3C360B61 (USA) /
 * 0x3C83126F (EU) — the EU twin unit carries its own value when carved.
 *
 * Matching arm stays INCLUDE_ASM (engine region — no byte-match). Faithful
 * transcription; helper semantics (func_002AB868/00284630/002AD860/002ABAE8/
 * 00283B30/00283B48) recovered from their prologues by prototype, marked below.
 * ORACLE-BLOCKED: the VU0 vec/trig helpers aren't loaded in the headless cmp
 * harness, so this #else is not runtime-verifiable here. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateMobyMotionVelocity);
#else
extern f32  g_mobyMaxTurnRate;                                 /* 0x1AA108 per-step yaw clamp (rad) */
extern void func_002AB868(f32 *mobyYaw, f32 *turnDeltaOut, f32 speed,
                          f32 velX, f32 velY, f32 velZ);        /* 0x2AB868 steer yaw toward velocity */
extern f32  AngleAbsDiffPi(f32 mobyYaw, f32 speed);             /* 0x284630 drive-heading angle helper */
extern f32  func_002835C0(f32 x);                              /* 0x2835C0 sqrtf */
extern void func_002AD860(Vec4 *v, f32 maxLen);                /* 0x2AD860 clamp v planar length to maxLen */
extern void func_002ABAE8(f32 *value, f32 delta, f32 rate1,
                          f32 rate2, f32 clamp);                /* 0x2ABAE8 rate-limited ease of *value by delta */
extern f32  func_00283B30(f32 angle);                          /* 0x283B30 cos-like trig leaf (VU0) */
extern f32  func_00283B48(f32 angle);                          /* 0x283B48 sin-like trig leaf (VU0) */
extern f32  Vec3DotVu0(Vec4 *a, Vec4 *b);                      /* 0x283760 3-component dot (VU0) */
extern f32  Vec2LengthXyVu0(Vec4 *v);                            /* 0x2837D0 planar/vector magnitude */
extern f32  WrapAnglePiDiff(f32 a, f32 b);                     /* 0x284590 wrap a-b into [-pi,pi] */

void UpdateMobyMotionVelocity(Moby *moby, MobyMotionController *ctrl, Vec4 *target,
                              f32 speed, Vec4 *stepOut, Vec4 *velOut) {
    f32 *mobyYaw  = (f32 *)((u8 *)moby + 0xF8);
    Vec4 *mobyPos = (Vec4 *)((u8 *)moby + 0x10);
    Vec4 stepVec;   /* commanded drive velocity (frame scratch) */
    Vec4 velCmd;    /* per-frame velocity delta applied to the accumulator */
    f32 steerAngle;
    f32 arriveSpeed;
    f32 dist;

    (void)stepOut;
    (void)velOut;

    /* 1. Steer yaw toward the commanded velocity (skipped when free-yaw). */
    if ((ctrl->modeFlags & MOBY_MOTION_MODE_FREE_YAW) == 0) {
        func_002AB868(mobyYaw, &ctrl->turnDelta, speed, ctrl->velX, ctrl->velY, ctrl->velZ);
    }

    steerAngle = AngleAbsDiffPi(*mobyYaw, speed);

    /* Inside the heading dead-band: drive a zero command (and flag steer-blocked
     * when the residual heading error outruns the max turn). */
    if ((ctrl->modeFlags & MOBY_MOTION_MODE_FREE_YAW) == 0 && ctrl->arriveAngle < steerAngle) {
        func_00283638(&stepVec);
        if ((ctrl->turnDelta * 30.0f) < (steerAngle - ctrl->arriveAngle)) {   /* EU: 25.0f */
            ctrl->stateFlags |= 0x2;
        }
        goto resolve;
    }

    ctrl->stateFlags &= ~0x2;
    dist = DistXYVu0(mobyPos, target);
    ctrl->distToTarget = dist;
    if (dist <= ctrl->arriveRadius) {   /* arrived: zero command */
        func_00283638(&stepVec);
        goto resolve;
    }

    /* Arrive-profile speed, clamped to the controller max. */
    arriveSpeed = func_002835C0(2.0f * (dist - ctrl->arriveRadius) * ctrl->accelArrive);
    if (ctrl->maxSpeed < arriveSpeed) {
        arriveSpeed = ctrl->maxSpeed;
    }

    {
        f32 angle;
        if (ctrl->wallContactCount == 0) {
            angle = speed;
        } else {
            /* Face away from the last wall contact, but limit the deflection to
             * +/- the max turn rate off the caller heading. */
            f32 baseAngle = WrapAnglePiSum(Atan2fPoly(ctrl->wallContact[0] - mobyPos->x,
                                                      ctrl->wallContact[1] - mobyPos->y),
                                           3.14159f);
            f32 turnErr = WrapAnglePiDiff(speed, baseAngle);
            if (g_mobyMaxTurnRate < turnErr) {
                turnErr = g_mobyMaxTurnRate;
            } else if (turnErr < -g_mobyMaxTurnRate) {
                turnErr = -g_mobyMaxTurnRate;
            }
            angle = WrapAnglePiSum(turnErr, baseAngle);
        }
        stepVec.x = func_00283B30(angle) * arriveSpeed;
        stepVec.y = func_00283B48(angle) * arriveSpeed;
        stepVec.z = 0.0f;
    }

resolve:
    /* 4. Fold the command into the velocity accumulator (which aliases +0xA0:
     * xy = planar accum, z = verticalVel). */
    Vec4SubVu0(&velCmd, &stepVec, (Vec4 *)&ctrl->velAccum);
    velCmd.z = 0.0f;
    if (0.0f < Vec3DotVu0(&stepVec, (Vec4 *)&ctrl->velAccum)) {
        func_002AD860(&velCmd, ctrl->accel);
    } else {
        func_002AD860(&velCmd, ctrl->accelArrive);
    }
    Vec4AddVu0((Vec4 *)&ctrl->velAccum, (Vec4 *)&ctrl->velAccum, &velCmd);
    ctrl->speed = Vec2LengthXyVu0((Vec4 *)&ctrl->velAccum);

    /* 5. Vertical / ground step. */
    if ((ctrl->modeFlags & MOBY_MOTION_MODE_NO_VERTICAL) == 0) {
        f32 posZ = *(f32 *)((u8 *)moby + 0x18);
        if (ctrl->groundHeight < posZ) {
            /* Above ground. */
            if (ctrl->airborneFrames < 2 && (posZ - ctrl->groundHeight) < 0.15f) {
                /* Just left a steep-enough slope: cancel the into-slope velocity. */
                f32 normalLen = Vec2LengthXyVu0((Vec4 *)ctrl->groundNormal);
                if (0.087266475f < Atan2fPoly(ctrl->groundNormal[2], normalLen)) {
                    f32 intoSlope = -Vec3DotVu0((Vec4 *)&ctrl->velAccum, (Vec4 *)ctrl->groundNormal);
                    if (intoSlope < ctrl->verticalVel) {
                        ctrl->verticalVel = -Vec3DotVu0((Vec4 *)&ctrl->velAccum, (Vec4 *)ctrl->groundNormal);
                    }
                }
            }
            ctrl->verticalVel -= ctrl->decel;
            if (ctrl->groundedFrames == 0) {
                ctrl->eventFlags |= MOBY_MOTION_EVENT_FELL_STUCK;
            }
        } else {
            /* At or below ground: clamp the step, ease toward the surface. */
            f32 penetration = ctrl->groundHeight - posZ;
            if (penetration < ctrl->verticalVel) {
                ctrl->verticalVel = penetration;
            } else if (ctrl->verticalVel < 0.0f) {
                ctrl->verticalVel = 0.0f;
            }
            func_002ABAE8(&ctrl->verticalVel,
                          ctrl->groundHeight - *(f32 *)((u8 *)moby + 0x18),
                          0.011111111f, 0.011111111f, 1.0f);   /* 0x3C360B61; EU 0x3C83126F */
        }
    }
}
#endif

/* Resolve a moby's per-frame motion against collision geometry. Integrates the
 * accumulated planar velocity into the position, then wall-slides up to 8 passes
 * of ResolveMobySphereCollision | ResolveMobyEdgeConstraint (stopping early once a
 * pass reports no contact). If anything was hit: recompute the frame displacement
 * (velAccum = pos - entryPos), clamp its magnitude to the tracked speed (or adopt
 * the shorter magnitude), and — when the moby is at/below ground — restore it to
 * the entry height and clamp any downward vertical velocity to 0. Finally bumps
 * the wall-contact counter when the resulting speed is below half maxSpeed (else
 * decays it) and records the net position change into posDelta.
 * WALL: save-layout — 6 callee-saves + $ra at 8-byte spacing, with fp/madd and
 * sq/lq 128-bit moves; matching arm stays INCLUDE_ASM, portable #else below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ResolveMobyMotionCollision);
#else
extern void Vec4AddVu0(Vec4 *dst, Vec4 *a, Vec4 *b);   /* dst = a + b (VU0) */
extern void Vec4SubVu0(Vec4 *dst, Vec4 *a, Vec4 *b);   /* dst = a - b (VU0) */
extern f32  Vec2LengthXyVu0(Vec4 *v);                    /* 0x2837D0 planar/vector magnitude */
extern void func_00283920(Vec4 *dst, Vec4 *src, f32 len);  /* 0x283920 rescale src to length len */
extern s32  TickCountdownTimer(void *counter);              /* 0x2832F8 tick a countdown at *p; returns nonzero while still armed (#61 uses it as void to decay the wall-contact counter, #59 branches on the gate) */
extern s32  ResolveMobySphereCollision(Moby *moby, MobyMotionController *ctrl);  /* defined below */
extern s32  ResolveMobyEdgeConstraint(void *moby, void *ec);                     /* defined below */

void ResolveMobyMotionCollision(Moby *moby, MobyMotionController *ctrl,
                                Vec4 *entryPos, Vec4 *velOut) {
    Vec4 savedPos;
    s32 anyHit = 0;
    s32 i;

    (void)velOut;   /* passed by the caller but unused by this pass */

    /* integrate the accumulated velocity, snapshot the pre-collision position */
    Vec4AddVu0((Vec4 *)((u8 *)moby + 0x10), (Vec4 *)((u8 *)moby + 0x10),
               (Vec4 *)ctrl->velAccum);
    savedPos = *(Vec4 *)((u8 *)moby + 0x10);
    ctrl->wallHitMoby = NULL;
    ctrl->wallContact[0] = 0.0f;
    ctrl->wallContact[1] = 0.0f;
    ctrl->wallContact[2] = 0.0f;
    ctrl->wallContact[3] = 0.0f;

    for (i = 0; i < 8; i++) {
        s32 hit = ResolveMobySphereCollision(moby, ctrl) | ResolveMobyEdgeConstraint(moby, ctrl);
        anyHit |= hit;
        if (hit == 0) {
            break;
        }
    }

    if (anyHit != 0) {
        f32 len;
        Vec4SubVu0((Vec4 *)ctrl->velAccum, (Vec4 *)((u8 *)moby + 0x10), entryPos);
        len = Vec2LengthXyVu0((Vec4 *)ctrl->velAccum);
        if (ctrl->speed < len) {
            func_00283920((Vec4 *)ctrl->velAccum, (Vec4 *)ctrl->velAccum, ctrl->speed);
        } else {
            ctrl->speed = len;
        }

        if (*(f32 *)((u8 *)moby + 0x18) <= ctrl->groundHeight) {
            if (*(f32 *)((u8 *)moby + 0x18) < ((f32 *)entryPos)[2]) {
                *(f32 *)((u8 *)moby + 0x18) = ((f32 *)entryPos)[2];
            }
            if (ctrl->verticalVel < 0.0f) {
                ctrl->verticalVel = 0.0f;
            }
        }

        if (ctrl->speed < ctrl->maxSpeed * 0.5f) {
            ctrl->wallContactCount++;
        } else {
            TickCountdownTimer(&ctrl->wallContactCount);
        }
    } else {
        TickCountdownTimer(&ctrl->wallContactCount);
    }

    Vec4SubVu0((Vec4 *)ctrl->posDelta, (Vec4 *)((u8 *)moby + 0x10), &savedPos);
}
#endif

/* Apply ground-snap and fire ground/landing events after motion resolve. Re-probes
 * the ground, then raises the per-step event flags: BLOCKED (0x2) when the moby is
 * below ground-clearance, AIRBORNE (0x1) when it just rose above ground+clearance
 * (and was recently grounded), SLOPE (0x4) when the ground normal's tilt exceeds
 * maxSlopeAngle. If any of those fired, the moby is restored to its entry position
 * and re-probed. Then snaps the moby onto the ground (and zeroes the grounded/
 * airborne frame counters accordingly), raises ARRIVED (0x80) within arriveRadius,
 * runs the path-blocked check (PATH_END 0x100 / clears the steer-block bit), maps
 * speed to the anim rate (TURN_ANIM mode), drives footstep/turn anim-seq events
 * (GROUND_ANIM mode), and spawns a water splash (WATER_FX mode) when over water.
 * WALL: save-layout — 6 callee-saves + $ra at 8-byte spacing, with fp temps;
 * matching arm stays INCLUDE_ASM, portable #else below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ApplyMobyGroundAndEvents);
#else
extern void ProbeMobyGroundLine(Moby *moby, MobyMotionController *ctrl, f32 depth);  /* below */
extern f32  Vec2LengthXyVu0(Vec4 *v);                 /* 0x2837D0 vector magnitude */
extern f32  Atan2fPoly(f32 y, f32 x);            /* 0x283BF8 atan2-style angle from a planar delta */
extern s32  CheckMobyPathBlocked(Moby *moby);       /* below */
extern s32  CheckMobyOverWater(void *moby, void *ctrl);  /* below */
extern void func_002A82D8(Moby *moby, s32 animSeq, s32 a2, s32 a3);      /* 0x2A82D8 trigger a move/turn anim-seq */
extern void func_002A9F30(Moby *moby, s32 a1, s32 a2, Vec4 *pos, f32 f); /* 0x2A9F30 spawn water-surface splash */

void ApplyMobyGroundAndEvents(Moby *moby, MobyMotionController *ctrl,
                              Vec4 *target, Vec4 *entryPos) {
    f32 *mobyPos = (f32 *)((u8 *)moby + 0x10);
    f32 *mobyPosZ = (f32 *)((u8 *)moby + 0x18);
    s32 eventFired = 0;

    ProbeMobyGroundLine(moby, ctrl, 0.0f);

    if (*mobyPosZ < ctrl->groundHeight - ctrl->groundClearUp) {
        ctrl->eventFlags |= 0x2;
        eventFired = 1;
    }
    if ((ctrl->modeFlags & 0x1) == 0 && ctrl->airborneFrames < 5 &&
        (ctrl->groundHeight + ctrl->groundClearDown) < *mobyPosZ) {
        ctrl->eventFlags |= 0x1;
        eventFired = 1;
    }
    if (ctrl->groundedFrames != 0) {
        f32 mag = Vec2LengthXyVu0((Vec4 *)ctrl->groundNormal);
        f32 slope = Atan2fPoly(ctrl->groundNormal[2], mag);
        if (ctrl->maxSlopeAngle < slope) {
            ctrl->eventFlags |= 0x4;
            eventFired = 1;
        }
    }

    if (eventFired) {
        *(Vec4 *)mobyPos = *entryPos;
        ProbeMobyGroundLine(moby, ctrl, 0.0f);
    }

    if (*mobyPosZ <= ctrl->groundHeight) {
        if (ctrl->groundHeight <= *mobyPosZ - ctrl->verticalVel) {
            *mobyPosZ = ctrl->groundHeight;
        }
        ctrl->airborneFrames = 0;
        ctrl->groundedFrames++;
    } else {
        ctrl->groundedFrames = 0;
        ctrl->airborneFrames++;
    }

    ctrl->distToTarget = DistXYVu0((Vec4 *)mobyPos, target);
    if (ctrl->distToTarget <= ctrl->arriveRadius) {
        ctrl->eventFlags |= 0x80;
    }

    if (ctrl->stateFlags & 0x1) {
        if (CheckMobyPathBlocked(moby) != 0) {
            ctrl->stateFlags &= ~0x1;
        } else {
            ctrl->eventFlags |= 0x100;
        }
    }

    /* anim-rate map from speed (TURN_ANIM) */
    if (ctrl->modeFlags & 0x80) {
        s32 a = *(u8 *)((u8 *)moby + 0x43);
        if (a == ctrl->animSeqIdleA || a == ctrl->animSeqIdleB) {
            f32 ref = ctrl->animSpeedRef;
            f32 rate;
            if (ref == 0.0f) {
                ref = ctrl->maxSpeed;
            }
            rate = ctrl->speed / ref;
            *(f32 *)((u8 *)moby + 0x48) = rate;
            if (ctrl->animSpeedClamp < rate) {
                *(f32 *)((u8 *)moby + 0x48) = ctrl->animSpeedClamp;
            } else if (rate < 1.0f / ctrl->animSpeedClamp) {
                *(f32 *)((u8 *)moby + 0x48) = 1.0f / ctrl->animSpeedClamp;
            }
        }
    }

    /* footstep / turn anim-seq events (GROUND_ANIM) */
    if (ctrl->modeFlags & 0x40) {
        s32 a = *(u8 *)((u8 *)moby + 0x43);
        if ((ctrl->stateFlags & 0x2) && ctrl->turnDelta != 0.0f) {
            if (a == ctrl->animSeqIdleA || a == ctrl->animSeqIdleB) {
                s32 seq;
                ctrl->animSeqActive = (u8)a;
                if (0.0f < ctrl->turnDelta) {
                    seq = ctrl->animSeqMoveA;
                } else {
                    seq = ctrl->animSeqMoveB;
                }
                func_002A82D8(moby, seq, 0, 0xA);
            }
        } else {
            if (a == ctrl->animSeqMoveA || a == ctrl->animSeqMoveB) {
                s32 active = ctrl->animSeqActive;
                if (active != 0xFF) {
                    func_002A82D8(moby, active, 0, 0xA);
                }
            }
        }
    }

    /* water-surface splash (WATER_FX) */
    if (ctrl->modeFlags & 0x200) {
        if (CheckMobyOverWater(moby, ctrl) != 0) {
            func_002A9F30(moby, 0, 0x10000, (Vec4 *)mobyPos, 255.0f);
        }
    }
}
#endif

/* Probe the ground line below a moby (raycast for the ground normal/height). Casts a
 * vertical CollLine from just above the moby (pos.z + groundClearUp + 0.1) down to
 * either pos.z - customDepth (when customDepth != 0) or pos.z - groundClearDown - 0.1.
 * On a hit, records the ground plane into the controller: groundHeight (hit z),
 * groundHitMoby, groundHitPoly, groundMaterial (GetCollHitMaterial), and the unit
 * ground normal (hit normal rescaled to length 1). On a miss, clears those and sets
 * groundMaterial = -1 with a default up normal (func_00283638).
 * WALL: save-layout — 2 callee-saves + $ra at 8-byte spacing, with fp temps; matching
 * arm stays INCLUDE_ASM, portable #else below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ProbeMobyGroundLine);
#else
extern u8 g_pCollWorldData[];   /* 0x1BAF00 collision-result block: +0x18 hitMoby, +0x1C hitPoly, +0x28 hitZ, +0x40 hitNormal */
extern s32 CollLine(Vec4 *start, Vec4 *end, s32 flags, Moby *ignoreMoby, s32 a4);
extern s32 GetCollHitMaterial(void);
extern void Vec3RescaleToLenVu0(Vec4 *dst, Vec4 *src, f32 len);
extern void func_00283638(Vec4 *normal);   /* 0x283638 reset to the default up normal */
void ProbeMobyGroundLine(Moby *moby, MobyMotionController *ctrl, f32 customDepth) {
    Vec4 probeStart = *(Vec4 *)((u8 *)moby + 0x10);
    Vec4 probeEnd = *(Vec4 *)((u8 *)moby + 0x10);

    ((f32 *)&probeStart)[2] += ctrl->groundClearUp + 0.1f;
    if (customDepth == 0.0f) {
        ((f32 *)&probeEnd)[2] -= ctrl->groundClearDown + 0.1f;
    } else {
        ((f32 *)&probeEnd)[2] -= customDepth;
    }

    if (CollLine(&probeStart, &probeEnd, 0, moby, 0) != 0) {
        ctrl->groundHeight = *(f32 *)(g_pCollWorldData + 0x28);
        ctrl->groundHitMoby = *(void **)(g_pCollWorldData + 0x18);
        ctrl->groundHitPoly = *(void **)(g_pCollWorldData + 0x1C);
        ctrl->groundMaterial = GetCollHitMaterial();
        Vec3RescaleToLenVu0((Vec4 *)ctrl->groundNormal, (Vec4 *)(g_pCollWorldData + 0x40), 1.0f);
    } else {
        ctrl->groundHeight = 0.0f;
        ctrl->groundHitMoby = NULL;
        ctrl->groundHitPoly = NULL;
        ctrl->groundMaterial = -1;
        func_00283638((Vec4 *)ctrl->groundNormal);
    }
}
#endif

/* Inherit motion from the moving platform a moby is standing on. If the
 * controller recorded a moby under the ground probe last step
 * (ctrl->groundHitMoby, +0x88), re-test the moby against that platform's
 * collision (func_002ADF48, fed the moby + the platform + the moby's pos vec4
 * at +0x10 and its prev-pos/extent vec4 at +0xF0); on a hit, raise the
 * platform-rider event bit 0x40 in ctrl->eventFlags (+0x94). No groundHitMoby
 * => nothing to inherit, return.
 *
 * Derived register-exact from CheckMobyGroundMover.s @0x2B6BF8.
 *
 * Save layout: 1 callee-save + $ra at 8-byte spacing.
 * MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
 * byte-exact (FACT #8810; census FACT #8830).
 * GUARD (task #1270): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_CheckMobyGroundMover)
S136OS_SLOT(CheckMobyGroundMover);
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

/* Resolve a moby against a collision sphere (push-out + slide). Builds a test
 * sphere centred on the moby position raised by (collRadiusBase + collRadiusStep)
 * in Z and runs CollSphere at collRadiusBase (query mode 6 when modeFlags bit1 is
 * set, else 4). On a miss returns 0. On a hit it records the hit moby (+0x84) and
 * copies the nudged contact vec into wallContact (+0x60); when the hit is a world
 * poly (hitMoby != 0) with no poly-info (hitPoly < 0) it lowers the shared nudge
 * point's Z toward the sphere centre; then slides the moby to the nudged hit point
 * minus the Z offset and returns 1.
 * WALL: save-layout — 3 callee-saves + $ra at 8-byte spacing, with fp/madd;
 * matching arm stays INCLUDE_ASM, portable #else below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ResolveMobySphereCollision);
#else
extern void Vec4AddVu0(Vec4 *dst, Vec4 *a, Vec4 *b);   /* dst = a + b (VU0) */
extern void Vec4SubVu0(Vec4 *dst, Vec4 *a, Vec4 *b);   /* dst = a - b (VU0) */
extern u8 g_pCollWorldData[];                          /* 0x1BAF00 collision-result block */
extern u8 g_collHitPointNudged[];                      /* 0x1BAF30 hit point nudged toward the query origin */
extern s32 CollSphere(Vec4 *center, s32 flags, Moby *moby, f32 radius);  /* 0x277268 */

s32 ResolveMobySphereCollision(Moby *moby, MobyMotionController *ctrl) {
    Vec4 sphereOffset;
    Vec4 center;
    s32 flags;

    ((f32 *)&sphereOffset)[0] = 0.0f;
    ((f32 *)&sphereOffset)[1] = 0.0f;
    ((f32 *)&sphereOffset)[2] = ctrl->collRadiusBase + ctrl->collRadiusStep;
    ((f32 *)&sphereOffset)[3] = 0.0f;
    Vec4AddVu0(&center, (Vec4 *)((u8 *)moby + 0x10), &sphereOffset);

    flags = (ctrl->modeFlags & 0x2) ? 6 : 4;
    if (CollSphere(&center, flags, moby, ctrl->collRadiusBase) == 0) {
        return 0;
    }

    ctrl->wallHitMoby = *(void **)(g_pCollWorldData + 0x18);
    *(Vec4 *)ctrl->wallContact = *(Vec4 *)(g_pCollWorldData + 0x20);
    if (*(s32 *)(g_pCollWorldData + 0x18) != 0) {
        if (*(s32 *)(g_pCollWorldData + 0x1C) < 0) {
            if (((f32 *)&center)[2] < *(f32 *)(g_pCollWorldData + 0x38)) {
                *(f32 *)(g_pCollWorldData + 0x38) = ((f32 *)&center)[2];
            }
        }
    }

    center = *(Vec4 *)g_collHitPointNudged;
    Vec4SubVu0((Vec4 *)((u8 *)moby + 0x10), &center, &sphereOffset);
    return 1;
}
#endif

/* Constrain a moby's motion to a collision edge. `ec` is the edge-constraint record:
 * +0x0 push length, +0x3C edge id (-1 = inactive), +0x9C flags (bit 0 disables),
 * +0x60 output push vector, +0x94 result flags. No-op (returns 0) when the edge is
 * inactive or disabled. Otherwise projects the moby position (+0x10) onto the edge
 * via func_002CA138; if that reports a hit, stores the (projected - position) delta
 * at +0x60 rescaled to the push length, marks bit 0x10 in +0x94, and returns 1.
 *
 * params: moby  the moby being constrained (position Vec4 at +0x10)
 *         ec    the edge-constraint record (layout above)
 * return: 1 when the edge pushed the moby, else 0.
 *
 * MATCHED on the s136os arm (task #1744). The old save-layout WALL was the 2.9
 * arm's; SN 1.36 packs the 3 callee saves itself. What carries the match:
 *  - the position is copied to the stack with ONE 128-bit move (lq/sq), as the
 *    ROM does; a Vec4 struct copy emits ldl/ldr/sdl/sdr (natively a plain copy);
 *  - two EMPTY tied fences around that copy (RULING #8483). Without the one
 *    before it the lq folds to 16(moby); without the one after it the call's
 *    argument setup schedules differently;
 *  - two register pins (RULING #8598): moby->$6 and pos->$18. Unpinned, cc1
 *    swaps the ec/pos s-register pair (ec $18 / pos $17; the ROM has ec $17 /
 *    pos $18) and moby is not staged in $6. Each pin and each fence was dropped
 *    alone and each drop breaks the match; an edgeId->$4 pin tried on the way
 *    was dead weight and is not here. */
#ifndef TARGET_NATIVE
#define EE_REG(r) __asm__(r)
#else
#define EE_REG(r)
#endif
#if !defined(TARGET_NATIVE) && !defined(S136OS_ResolveMobyEdgeConstraint)
S136OS_SLOT(ResolveMobyEdgeConstraint);
#else
extern s32 func_002CA138(s32 edgeId, Vec4 *point, Vec4 *pos, f32 pushLen);  /* 0x2CA138 project onto edge */
extern void Vec4SubVu0(Vec4 *dst, Vec4 *a, Vec4 *b);            /* dst = a - b (VU0) */
extern void Vec3RescaleToLenVu0(Vec4 *dst, Vec4 *src, f32 len); /* dst = src * (len / |src|) */
s32 ResolveMobyEdgeConstraint(void *mobyArg, void *ec) {
    register void *moby EE_REG("$6") = mobyArg;
    register Vec4 *pos EE_REG("$18");
    Vec4 point;
    Vec4 *push;
    s32 edgeId = *(s32 *)((u8 *)ec + 0x3C);
    if (edgeId == -1) {
        return 0;
    }
    if (*(s32 *)((u8 *)ec + 0x9C) & 0x1) {
        return 0;
    }
    pos = (Vec4 *)((u8 *)moby + 0x10);
    __asm__("" : : "r"(pos));
#ifndef TARGET_NATIVE
    {
        typedef unsigned int Quad __attribute__((mode(TI)));
        *(Quad *)&point = *(Quad *)pos;
    }
#else
    point = *pos;
#endif
    __asm__("" : : "r"(pos));
    if (func_002CA138(edgeId, &point, pos, *(f32 *)ec) != 0) {
        push = (Vec4 *)((u8 *)ec + 0x60);
        Vec4SubVu0(push, &point, pos);
        Vec3RescaleToLenVu0(push, push, *(f32 *)ec);
        *(s32 *)((u8 *)ec + 0x94) |= 0x10;
        return 1;
    }
    return 0;
}
#endif

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B6FC8);

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
 * MATCHED 100.00% on the sdk29 arm (unit objdiff report, objdiff_build.sh +
 * unit_report.sh, clean; task #510). The earlier "fp-save frame" wall note was
 * a stale claim: the pinned cc1 reproduces the four swc1 saves as-is. */
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

/* Size-pinned 8-byte epilogue pad pseudo-function — see unit header. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7038);

/* Drive a moby along its waypoint path (follow + arrival logic). No-op (returns 0) with
 * no motion controller. Binds `path` as the controller's waypoint path if it changed
 * (SetMobyWaypointPath, whole-path). Steers toward the current node (waypointCursor)
 * heading via Atan2fPoly while StepMobyMotion targets the end node (waypointEnd).
 * On arriving within arriveRadius of the current node: returns 1 if that was the end
 * node, else advances the cursor toward the end (±1 by direction) and returns 0.
 * Waypoint nodes are vec4s at path + 0x10 + i*0x10; the count is the leading short.
 * cc1 2.9 (not the image arm; see GUARD) hits the save-layout wall: 3 callee-saves
 * + $ra at 8-byte spacing, with fp temps. */
/* GUARD (task #1338): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1338 lever): the waypoint offset (idx*0x10 +
 * 0x10) grouped before the path base is added, for the StepMobyMotion target
 * and the DistXYVu0 point. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_DriveMobyAlongWaypoints)
S136OS_SLOT(DriveMobyAlongWaypoints);
#else
s32 SetMobyWaypointPath(Moby *moby, short *path, s32 endIdx, s32 startIdx);  /* defined below */
extern f32 Atan2fPoly(f32 dx, f32 dy);   /* 0x283BF8 atan2-style heading from a planar delta */
s32 DriveMobyAlongWaypoints(Moby *moby, short *path) {
    MobyMotionController *ctrl = (MobyMotionController *)GetMobyMotionController(moby);
    f32 *node;
    f32 heading;

    if (ctrl == 0) {
        return 0;
    }
    if (path != ctrl->waypointPath) {
        SetMobyWaypointPath(moby, path, -1, 0);
    }

    node = (f32 *)((u8 *)path + ctrl->waypointCursor * 0x10 + 0x10);
    heading = Atan2fPoly(node[0] - *(f32 *)((u8 *)moby + 0x10),
                            node[1] - *(f32 *)((u8 *)moby + 0x14));
    StepMobyMotion(moby, (Vec4 *)((u8 *)path + (ctrl->waypointEnd * 0x10 + 0x10)), heading);

    if (DistXYVu0((Vec4 *)((u8 *)moby + 0x10),
                  (Vec4 *)((u8 *)path + (ctrl->waypointCursor * 0x10 + 0x10))) <= ctrl->arriveRadius) {
        if (ctrl->waypointEnd == ctrl->waypointCursor) {
            return 1;
        }
        if (ctrl->waypointEnd < ctrl->waypointCursor) {
            ctrl->waypointCursor = ctrl->waypointCursor - 1;
        } else {
            ctrl->waypointCursor = ctrl->waypointCursor + 1;
        }
    }
    return 0;
}
#endif

/* Size-pinned 8-byte epilogue pad pseudo-function — see unit header. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7140);

/* Install a waypoint path into a moby's motion controller. `path` is the path
 * array (path[0] is its node count as a short, see waypointPath +0xC4). `endIdx`
 * selects the destination node: pass -1 to walk to the last node (count-1),
 * otherwise it is used verbatim. `startIdx` seeds the current-node cursor.
 *
 * Returns 0 if the moby has no motion controller, else 1.
 *
 * Derived register-exact from SetMobyWaypointPath.s @0x2B7148 (the asm reuses the -1
 * sentinel both as the "no controller" return-stage value and as the endIdx==-1
 * compare operand; the C below expresses the same two effects directly).
 *
 * Save layout: 3 callee-saves + $ra at 8-byte spacing.
 * MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
 * byte-exact (FACT #8810; census FACT #8830).
 * GUARD (task #1270): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_SetMobyWaypointPath)
S136OS_SLOT(SetMobyWaypointPath);
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7218);

/* Test whether a moby riding a spline/rail is blocked. No-op (returns 0) when the
 * moby has no motion controller or no active spline constraint (splineConstraintId
 * == -1). Otherwise fetches the constraint's spline record from the global spline
 * table (g_targetZoneTable[splineConstraintId]) and, if the moby position (+0x10)
 * clears the spline polyline (func_002A9958) AND does not project onto the edge
 * (func_002CA138 misses), reports the path clear (returns 1). Any block marks
 * stateFlags bit 0 (path-active) and returns 0.
 * WALL (cc1 2.9): save-layout — 2 callee-saves + $ra at 8-byte spacing, with fp
 * temps; the s136os arm reproduces it (GUARD below). */
/* GUARD (task #1271): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_CheckMobyPathBlocked)
S136OS_SLOT(CheckMobyPathBlocked);
#else
extern s32 func_002CA138(s32 constraintId, Vec4 *pos, Vec4 *point, f32 len);  /* 0x2CA138 project onto edge */
s32 CheckMobyPathBlocked(Moby *moby) {
    MobyMotionController *ctrl = (MobyMotionController *)GetMobyMotionController(moby);
    void *spline;
    Vec4 point;

    if (ctrl == 0 || ctrl->splineConstraintId == -1) {
        return 0;
    }
    spline = g_targetZoneTable[ctrl->splineConstraintId];
    if (func_002A9958((Vec4 *)((u8 *)moby + 0x10), (Vec4 *)((u8 *)spline + 0x10), *(s32 *)spline) != 0 &&
        func_002CA138(ctrl->splineConstraintId, (Vec4 *)((u8 *)moby + 0x10), &point, ctrl->collRadiusBase) == 0) {
        return 1;
    }
    ctrl->stateFlags |= 1;
    return 0;
}
#endif

/* Test whether a moby is submerged in a water volume. Quick-rejects (returns 0) when
 * the moby's Y (+0x18) is at or below the passed volume's top plane (+0x90). Otherwise
 * looks up the water body actually under the moby (func_002AC088); if none, returns 0.
 * Then compares the moby Y against that body's surface height (+0x40) minus 0.5:
 * returns 1 when the moby sits below it. Under alternate gravity (g_altGravityEnabled != 0) the
 * compared height is the f32 that func_002B11C8(moby+0x10) returns (the ROM uses its
 * $f0 at 0x2B7350; the arm used to discard it and compare the entry-time Y);
 * otherwise it compares the moby's current Y directly.
 * WALL: assembler hazard-nop placement (FACT #9545), not save layout. On the s136os
 * arm SN 1.36 cc1plus emits this body instruction-identical to the ROM, 3-callee-save
 * packed frame included, at 47 = 47 words. The one differing word is the `b` at
 * 0x2B7354: our gas puts the c.cond->bc1 hazard nop BEFORE the merge label, the ROM
 * has it AFTER (NOTE #8972's class), so the branch lands one word late. Closing it
 * is an asm_unit/gas change under tools/ee (RULING #7207), not a C re-spelling.
 * The old "save-layout" label described the 2.9 arm only. Matching arm stays
 * INCLUDE_ASM, portable #else below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CheckMobyOverWater);
#else
extern void *func_002AC088(void *moby);           /* 0x2AC088 water body under the moby, or NULL */
extern f32 func_002B11C8(Vec4 *pos);              /* 0x2B11C8 radial-gravity height probe (def 1A8180.c) */
s32 CheckMobyOverWater(void *moby, void *waterVol) {
    void *body;
    s32 under;

    if (*(f32 *)((u8 *)moby + 0x18) <= *(f32 *)((u8 *)waterVol + 0x90))
        return 0;

    body = func_002AC088(moby);
    if (body == NULL)
        return 0;

    under = 0;
    if (g_altGravityEnabled != 0) {
        if (func_002B11C8((Vec4 *)((u8 *)moby + 0x10)) < *(f32 *)((u8 *)body + 0x40) - 0.5f)
            under = 1;
    } else if (*(f32 *)((u8 *)moby + 0x18) < *(f32 *)((u8 *)body + 0x40) - 0.5f) {
        under = 1;
    }
    return under;
}
#endif

/* BeginFrameDrawList(dmaCursor) - opens a new per-frame GS/VIF1 DMA draw list.
 * Points g_frameDmaCursor at the caller's buffer, then emits the 8-word opening
 * DMA/GIF header: a DIRECT-mode chain (0x30000009/0x50000009) selecting the GS
 * screen context (the g_gsScreenContextPtr packet + 0xC0, masked to the 28-bit
 * physical address the DMAC needs), followed by an UNPACK (0x30000025/0x50000025)
 * of the framebuffer-setup packet at g_screenBlitPacketB+0x290. Advances the
 * cursor past the header, bumps the scene-frame counter, then appends either the
 * cinematic-queue draw path (func_0027DB38, when g_cinematicQueue is set) or the
 * normal scene path (func_0027DC40), closes the chain with a 0x70000000 end tag,
 * and flushes the data cache (func_0011AED0) so the DMAC sees the finished packet.
 *
 * Engine region (ee-gcc 2.96) - the original reloads g_frameDmaCursor after every
 * tag store and mixes %lo-absolute with %gp_rel access to the same cursor, which
 * the pinned 2.9 cc1 won't reproduce, so this is a faithful #else, not a byte match. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", BeginFrameDrawList);
#else
extern u32 *g_frameDmaCursor;      /* VIF1/GIF chain write cursor */
extern u8 *g_gsScreenContextPtr;   /* pointer to the GS screen-context packet */
extern s32 g_cinematicQueue;
extern u8 g_cameraSlotActive[];    /* scene-frame counter lives at +0x9C4 */
extern u8 g_screenBlitPacketB[];   /* framebuffer-setup packet lives at +0x290 */
extern void func_0027DB38(void);   /* cinematic-queue draw path */
extern void func_0027DC40(void);   /* normal scene draw path */
extern void func_0011AED0(s32);    /* data-cache flush */

void BeginFrameDrawList(u32 *dmaCursor)
{
    g_frameDmaCursor = dmaCursor;
    g_frameDmaCursor[0] = 0x30000009;
    g_frameDmaCursor[1] = ((u32)g_gsScreenContextPtr + 0xC0) & 0x0FFFFFFF;
    g_frameDmaCursor[2] = 0;
    g_frameDmaCursor[3] = 0x50000009;
    g_frameDmaCursor[4] = 0x30000025;
    g_frameDmaCursor[5] = (u32)&g_screenBlitPacketB[0x290];
    g_frameDmaCursor[6] = 0;
    g_frameDmaCursor[7] = 0x50000025;
    g_frameDmaCursor += 8;
    *(s32 *)&g_cameraSlotActive[0x9C4] += 1;
    if (g_cinematicQueue == 0) {
        func_0027DC40();
    } else {
        func_0027DB38();
    }
    g_frameDmaCursor[0] = 0x70000000;
    g_frameDmaCursor[1] = 0;
    g_frameDmaCursor += 4;
    func_0011AED0(0);
}
#endif

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7570);

/* Per-frame driver: rebuild the active-moby chain (BuildActiveMobyChain, cached in
 * g_activeMobyChainHead) and tick every node. For each moby whose state byte (+0x20)
 * is non-negative: unless modeFlags&0x40 is set, advance its animation
 * (UpdateMobyAnimation); if it has a per-moby update fn (+0x64), call it; then unless
 * modeFlags&0x4 is set, refresh its bounding sphere + spatial-grid cell
 * (UpdateMobyBSphereAndGrid). Nodes with a negative state byte are skipped. Walks the
 * singly-linked chain via the next pointer at +0x28.
 * WALL: save-layout — 1 callee-save + $ra at 8-byte spacing, plus an indirect
 * per-moby call loop; matching arm stays INCLUDE_ASM, portable #else below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateActiveMobys);
#else
extern Moby *BuildActiveMobyChain(void);        /* 0x2B62D8 collect live mobys into a chain */
extern Moby *g_activeMobyChainHead;             /* head cached each frame */
extern void UpdateMobyAnimation(Moby *moby);    /* 0x2A1360 */
extern void UpdateMobyBSphereAndGrid(void *moby);  /* 0x2A1D80 */
void UpdateActiveMobys(void) {
    Moby *m = BuildActiveMobyChain();
    g_activeMobyChainHead = m;
    while (m != NULL) {
        if (*(s8 *)((u8 *)m + 0x20) >= 0) {         /* skip nodes with a negative state byte */
            if ((m->modeFlags & 0x40) == 0) {
                UpdateMobyAnimation(m);
            }
            {
                void (*update)(Moby *) = *(void (**)(Moby *))((u8 *)m + 0x64);
                if (update != NULL) {
                    update(m);
                }
            }
            if ((m->modeFlags & 0x4) == 0) {
                UpdateMobyBSphereAndGrid(m);
            }
        }
        m = *(Moby **)((u8 *)m + 0x28);             /* advance to the next chain node */
    }
}
#endif

extern void func_0011D620(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g, s32 h, s32 i);
extern s32  func_00133250(s32 a, s32 b, s32 c, s32 d);
extern u8 D_001A7210[];                     /* dialog sound-channel config block */

#ifndef TARGET_NATIVE
/* The SIF RPC client InitIopUploadRing binds (0x1A7248 = D_001A7210 + 0x38).
 * ADDRESSING DEVICE (RULING #8620 family, the #1603 predicate): declared small
 * so cc1 prints a one-insn `la` and does not CSE the address into a callee-saved
 * register across the calls; `.extern ,16` makes the assembler expand that `la`
 * to the absolute lui/addiu pair the ROM re-forms at each call. Emits no
 * instruction of its own. */
extern s32 g_iopRingRpcClient __asm__("D_1A7248");
__asm__(".extern D_1A7248, 16");
#define DIALOG_RPC_CLIENT ((s32)&g_iopRingRpcClient)
#else
#define DIALOG_RPC_CLIENT ((s32)(D_001A7210 + 0x38))
#endif

/* Set up the dialog sound channel: issue RPC 3 on the IOP-ring SIF RPC client
 * (0x1A7248) twice, around func_00133250(7, 0xA000, 0, 1).
 *
 * func_0011D620 takes nine args (client, fno, then seven zeros), the
 * sceSifCallRpc shape. That reading comes from the call shape and the
 * InitIopUploadRing binding, not from a trace of the callee.
 *
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) at the -O2 default, without the 2.9 arm's
 * -fno-gcse (RULING #9004), and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. A build that skips the splice drops the function.
 * On native it is plain C. The ROM re-forms the client address at each call with
 * no callee-save (frame 0x20, $31 only). Spelled as D_001A7210 + 0x38, the
 * address is hoisted into $16 and the frame grows; the small-extern device
 * above removes the hoist (task #1703). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_InitDialogSoundChannel)
S136OS_SLOT(InitDialogSoundChannel);
#else
void InitDialogSoundChannel(void) {
    func_0011D620(DIALOG_RPC_CLIENT, 3, 0, 0, 0, 0, 0, 0, 0);
    func_00133250(7, 0xA000, 0, 1);
    func_0011D620(DIALOG_RPC_CLIENT, 3, 0, 0, 0, 0, 0, 0, 0);
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
 * the RPC). The bank's EE address is the global-WAD base (TOC +0x529C) plus the
 * global bank's TOC offset (TOC +0x52B0). No params, no return.
 * The address shape is from task #510: the listener block and the disc TOC are
 * taken through local pointers so +0x17A0 / +0x52B0 stay displacements, and the
 * status slot is cleared with the unsigned spelling (lui/ori).
 * GUARD (task #1347): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1347 lever): no empty `__asm__ __volatile__("")`
 * after the call. That was a 2.96 sibcall fence (2.95.3 has no sibcall pass to
 * fence), and on the s136os arm it was a scheduling barrier that put the 0x52B0
 * TOC load after `addiu $7,$5,0x17A0` (2 of 22 words). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_LoadGlobalSoundBank)
S136OS_SLOT(LoadGlobalSoundBank);
#else
void LoadGlobalSoundBank(void) {
    ListenerBlock *listener = (ListenerBlock *)g_listenerPosHistory;
    u8 *toc = g_discToc;
    s32 bankAddr = *(s32 *)(toc + 0x52B0) + *(s32 *)(toc + 0x529C);
    listener->bankLoadStatus.all[0] = 0xFFFFFFFF;
    snd_BankLoadAsync(bankAddr, 0, (void *)OnSoundBankLoaded, (long)(u32)listener->bankLoadStatus.all);
}
#endif

/* Kick an async EE-side sound-bank load of `bankAddr` for status slot
 * `bankSlot`, resetting that slot to -1 (loading) and registering
 * OnSoundBankLoaded as the completion callback (the status-slot pointer is
 * zero-extended to 64 bits for the RPC). `bankAddr` is the bank's EE address,
 * forwarded verbatim to the loader. No return.
 * The address shape is from task #510: `table = listener->bankLoadStatus.all`
 * as its own local keeps (base + 0x17A0) + slot*4 in the ROM's association.
 * GUARD (task #1347): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1347 lever): no empty `__asm__ __volatile__("")`
 * after the call. That was a 2.96 sibcall fence, and on the s136os arm a
 * scheduling barrier that put the status-pointer `dsll32` ahead of the callback
 * `lui` (2 of 18 words). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_LoadLevelSoundBank)
S136OS_SLOT(LoadLevelSoundBank);
#else
void LoadLevelSoundBank(s32 bankAddr, s32 bankSlot) {
    ListenerBlock *listener = (ListenerBlock *)g_listenerPosHistory;
    u32 *table = listener->bankLoadStatus.all;
    u32 *status = table + bankSlot;
    *status = 0xFFFFFFFF;
    snd_BankLoadFromEE_CB(bankAddr, (void *)OnSoundBankLoaded, (long)(u32)status);
}
#endif

/* Kick the disc load of level sound-bank `bankSlot` if its TOC entry exists:
 * resets the bank's load-status slot to -1 (loading) and starts an async load
 * with OnSoundBankLoaded as the completion callback; when the TOC entry is
 * empty, clears the status slot to 0 (no bank). The bank's EE address is the
 * global-WAD base (TOC +0x529C) plus the level bank's TOC offset (TOC +0x52E0,
 * 8-byte entries). The status pointer handed to the callback is the decayed
 * `by.level` array plus the slot, zero-extended to 64 bits for the RPC.
 *
 * Params: bankSlot — level sound-bank index (0-based; its status word is
 *         bankLoadStatus.all[bankSlot + 1], slot 0 being the global bank).
 * No return value.
 *
 * Two spellings carry the match (task #756):
 *   - `toc - -(bankSlot * 8)`: cc1 2.9 emits `toc + bankSlot * 8` with the
 *     shifted index as the first addu operand; the ROM has the TOC base first
 *     (`addu $2,$6,$2`). Subtracting the negated offset keeps the base first.
 *   - `noTailCall = 0` after the call instead of an empty volatile asm. Both
 *     stop cc1 turning the call into a sibling `j` (the ROM keeps jal + frame),
 *     but the asm stays in the block as a scheduling barrier that every insn
 *     depends on, which tips sched2's tie between the callback `lui` and the
 *     status store the wrong way. A dead store to a local is enough to keep the
 *     call out of tail position at expand time and is deleted by flow before
 *     either scheduler runs. */
void KickLevelBankDiscLoad(s32 bankSlot) {
    u8 *toc = g_discToc;
    s32 tocOffset = *(s32 *)(toc - -(bankSlot * 8) + 0x52E0);
    s32 noTailCall;
    if (tocOffset != 0) {
        ListenerBlock *listener = (ListenerBlock *)g_listenerPosHistory;
        u32 *levelTable = listener->bankLoadStatus.by.level;
        listener->bankLoadStatus.all[bankSlot + 1] = 0xFFFFFFFF;
        snd_BankLoadAsync(tocOffset + *(s32 *)(toc + 0x529C),
                          0, (void *)OnSoundBankLoaded, (long)(u32)(levelTable + bankSlot));
        noTailCall = 0;
    } else {
        ListenerBlock *listener = (ListenerBlock *)g_listenerPosHistory;
        listener->bankLoadStatus.all[bankSlot + 1] = 0;
    }
}

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B77E0);

extern char D_1AA198[];                    /* file-load init debug string */
extern void InitDialogSoundChannel(void);  /* 0x2B7610 dialog sound-channel setup */
extern void InstallFileLoadPump(void);     /* 0x2B7E... installs the snd-pump callback */

/* Reset the entire file-load + dialog-voice manager to its idle defaults: clear
 * every per-channel state/flag word, reset the primary dialog voice id (-1) and
 * its state byte (0x20), reset the global sound-bank id (-1), then (re)initialise
 * the dialog sound channel and install the per-snd-pump file-load pump.
 * No params, no return. Prints the D_1AA198 debug banner first.
 *
 * MATCHED 100.00% on the sdk29 arm (unit objdiff report, objdiff_build.sh +
 * unit_report.sh, clean, VM colima-ee-x86-b; task #650). Two orderings carry the
 * match:
 *   - the shared `li -1`: cc1 2.9's cse keys constants by mode, so one s32 local
 *     `none` feeds both the s16 dialogVoiceId and the s32 soundBankId store
 *     (task #510);
 *   - the store run: the twelve stores are independent and cc1 2.9's scheduler
 *     emits the source-LAST of them first, then the rest in source order (FACT
 *     #7436). The ROM opens with dialogVoiceId and closes with soundBankId (in the
 *     InitDialogSoundChannel delay slot), so dialogVoiceId is written LAST here.
 *     Task #510 had it first, which put soundBankId at the top (88.15%).
 * The empty asm after the last call keeps InstallFileLoadPump a jal + epilogue
 * (the ROM form) instead of cc1's sibling-call `j`. */
void InitFileLoadSystem(void) {
    s32 none;
    DebugPrintStub(D_1AA198);
    none = -1;
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
    g_fileLoadVoiceState.soundBankId = none;
    g_fileLoadVoiceState.dialogVoiceId = none;
    InitDialogSoundChannel();
    InstallFileLoadPump();
    __asm__ __volatile__("");
}

/* Register the file-load completion handler as the per-snd-pump tick callback.
 * The empty asm guard blocks cc1's sibling-call (`j`) so the original jal+frame
 * is reproduced. */
void InstallFileLoadPump(void) {
    SetSndPumpCallback((void *)PumpFileLoadCompletion);
    __asm__ __volatile__("");
}

/* Primary dialog/voice playback entry: resolves `a0` (a dialog id) to a sample
 * address across seven language-keyed TOC bands, then — on a hit — arms the
 * dialog/secondary voice control block (m+0x68..0x88), allocates a voice handle,
 * initialises the per-voice listener + sound-emitter blocks, and plays the
 * sample via snd_PlaySample. The subtitle SM syncs to this voice state.
 *
 * Id bands (signed, descending — mirrors the original's nested slti tree):
 *   >= 6000        master language table  D_00147CC0+0x4E0  (stride 4, lang<<10),
 *                  base D_00147CC0+0x4E0+0x59C4; category `code` = 6
 *   5000..5999     disc-TOC table  g_discToc+0x4990 (stride 8), base +0x3E24
 *   4000..4999     level dialog pair  D_B038/D_B03C (stride 0x14C + lang*8),
 *                  base = g_sceneWadBaseLbn (start+end)
 *   3000..3999     dialog sample table  g_dialogSampleTable, idx = a0-3000
 *   2000..2999     dialog sample table, idx = a0 + adjLang - 2000
 *   1000..1999     disc-TOC table  g_discToc+0x2628 (stride 4), base +0x2624,
 *                  idx = a0 + adjLang - 1000
 *   0..999         per-level dialog pair  g_levelDialogToc (stride 0x14C+lang*8),
 *                  base = g_levelDialogToc+4 (start+end)
 * where adjLang = (lang == 0) ? 0 : lang-1 for the language-adjusted bands.
 * No-op if the channel is busy (m+0x68 != 0) or no sample resolves.
 *
 * WALL (matching build on the cc1 2.9 arm, INCLUDE_ASM frozen): PACKED-SAVE — 8 callee-saves + $ra
 * at 8-byte spacing (cc1 2.9 reserves 16 per save); the deeply-nested id-band
 * tree; sq/lq 128-bit handle copies. The snd_PlaySample call now uses the real
 * 12-argument signature (task #510): the `code` category (2, or 6 for the
 * >=6000 band), the 0/1 sentinels and the start-cb/context stack words are
 * passed as the ROM passes them (the old 8-arg shim put the callback in $10).
 * sdk29 55.54% / engine96 48.62% (unit objdiff report).
 * Save stride NOT re-measured for this member on the s136os arm: its C arm does not compile
 * solo there (NOTE #9871). Of the 111 labeled members that were, 0 reproduce the 16-byte save
 * stride there (FACT #9873), so the stride is not evidence that this member is walled.
 * Residual: UNMEASURED. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartDialogVoice);
#else
/* TODO(match): functional equivalent - not byte-exact; PACKED-SAVE + id-band
   tree + 128-bit handle copies, sdk29 55.54% / engine96 48.62%. */
extern u8   g_currentLanguage;             /* current language id (0 = default) */
extern u8   D_00147CC0[];                  /* 0x147CC0 master language-keyed dialog TOC (>=6000 band) */
extern u8   D_B038[];                      /* 0xFFB038 per-level dialog sample-pair table (D_B03C = +4) */
extern s32  g_sceneWadBaseLbn;             /* base LBN added to the 4000-band pair entries */
extern u8   g_levelDialogToc[];            /* per-level dialog TOC (<1000 band); +4 = base LBN */
extern s32  AllocVoiceHandleSlot(void);    /* allocates a playing-voice slot; <0 = none free */
extern u8   g_soundEmitterTable[];         /* per-voice sound-emitter blocks (stride 0x70) */
extern u8   D_1AA170[];                    /* gp-rel dialog-voice config block */
extern void OnDialogVoiceStarted(s32 voiceId, long handle);  /* start callback (defined below) */

s32 StartDialogVoice(s32 a0, s32 a1, s32 a2, s32 a3) {
    u8 *m = (u8 *)&g_fileLoadVoiceState;
    s32 sampleStart = 0;   /* $17: resolved sample start address */
    s32 sampleEnd = 0;     /* $19: resolved sample end address (0 if none) */
    s32 adjLang;
    s32 handle;
    s32 code = 2;          /* $22: sample category handed to snd_PlaySample (6 for the >=6000 band) */

    if (*(s32 *)(m + 0x68) != 0) {    /* secondary/dialog channel already busy */
        return 0;
    }

    if (a0 >= 0x1770) {
        /* >= 6000: master language-keyed dialog table (category code = 6) */
        s32 e;
        code = 6;
        adjLang = g_currentLanguage ? (s32)g_currentLanguage - 1 : 0;
        e = *(s32 *)(D_00147CC0 + 0x4E0 + a0 * 4 + (adjLang << 10));
        if (e != 0) {
            sampleStart = e + *(s32 *)(D_00147CC0 + 0x4E0 + 0x59C4);
        }
    } else if (a0 >= 0x1388) {
        /* 5000..5999: disc-TOC dialog table (stride 8) + base */
        s32 e = *(s32 *)(g_discToc + 0x4990 + (a0 - 0x1388) * 8);
        if (e != 0) {
            sampleStart = *(s32 *)(g_discToc + 0x3E24) + e;
        }
    } else if (a0 >= 0xFA0) {
        /* 4000..4999: per-level dialog sample pair, base = scene-WAD LBN */
        s32 base5 = a0 * 0x14C + ((s32)g_currentLanguage << 3);
        s32 e1 = *(s32 *)(D_B038 + base5);
        if (e1 != 0) {
            s32 wad = g_sceneWadBaseLbn;
            s32 e2 = *(s32 *)(D_B038 + base5 + 4);    /* D_B03C */
            sampleStart = wad + e1;
            sampleEnd = e2 ? (wad + e2) : 0;
        }
    } else if (a0 >= 0xBB8) {
        /* 3000..3999: dialog sample table, idx = a0 - 3000 */
        s32 e = *(s32 *)(g_discToc + 0x5300 + (a0 - 0xBB8) * 8);
        if (e != 0) {
            sampleStart = g_dialogSampleBase + e;
        }
    } else if (a0 >= 0x7D0) {
        /* 2000..2999: dialog sample table, language-adjusted idx */
        adjLang = g_currentLanguage ? (s32)g_currentLanguage - 1 : 0;
        {
            s32 e = *(s32 *)(g_discToc + 0x5300 + (a0 + adjLang - 0x7D0) * 8);
            if (e != 0) {
                sampleStart = g_dialogSampleBase + e;
            }
        }
    } else if (a0 >= 0x3E8) {
        /* 1000..1999: disc-TOC table (stride 4) + base, language-adjusted idx */
        adjLang = g_currentLanguage ? (s32)g_currentLanguage - 1 : 0;
        {
            s32 e = *(s32 *)(g_discToc + 0x2628 + (a0 + adjLang - 0x3E8) * 4);
            if (e != 0) {
                sampleStart = *(s32 *)(g_discToc + 0x2624) + e;
            }
        }
    } else {
        /* < 1000: per-level dialog TOC pair (stride 0x14C + lang*8), base +4 */
        s32 base8 = a0 * 0x14C + ((s32)g_currentLanguage << 3);
        s32 e1 = *(s32 *)(g_levelDialogToc + 8 + base8);
        if (e1 != 0) {
            s32 lbn = *(s32 *)(g_levelDialogToc + 4);
            s32 e2 = *(s32 *)(g_levelDialogToc + 8 + base8 + 4);
            sampleStart = lbn + e1;
            sampleEnd = e2 ? (lbn + e2) : 0;
        }
    }

    if (sampleStart == 0) {
        return 0;
    }

    /* arm the dialog/secondary voice control block (m+0x68..0x88) */
    *(s32 *)(m + 0x68) = -1;          /* secondaryState */
    *(s16 *)(m + 0x72) = 1;           /* secondaryFlag */
    *(s16 *)(m + 0x6C) = (s16)a0;     /* dialogArg1 = id */
    *(s16 *)(m + 0x70) = (s16)a1;     /* dialogArg2 */
    *(s32 *)(m + 0x7C) = 10;          /* volume */
    *(s32 *)(m + 0x80) = 48000;       /* sample rate (0xBB80) */
    *(s16 *)(m + 0x6E) = (s16)a3;     /* dialogArg0 = pan */
    *(s16 *)(m + 0x78) = 0;
    *(s32 *)(m + 0x84) = a2;          /* dialogArg3 = emitter ptr */

    handle = AllocVoiceHandleSlot();
    *(s32 *)(m + 0x88) = handle;
    if (handle >= 0) {
        u8 *slot = g_listenerPosHistory + handle * 0x70;
        s16 status = 0;               /* $20: listener-block +0x75 status byte */

        *(s16 *)(slot + 0x7C) = *(u16 *)(D_1AA170 + 0x1A);
        *(s16 *)(slot + 0x7E) = -1;
        *(s16 *)(slot + 0x80) = (s16)a3;
        *(s32 *)(slot + 0x8C) = 0;
        *(void **)(slot + 0x78) = D_1AA170;
        *(s32 *)(slot + 0x88) = a2;
        *(s32 *)(slot + 0xA0) = 0;    /* sq $0: zero the 16 bytes at +0xA0 */
        *(s32 *)(slot + 0xA4) = 0;
        *(s32 *)(slot + 0xA8) = 0;
        *(s32 *)(slot + 0xAC) = 0;

        if (a2 != 0) {
            /* copy the emitter's 16-byte position from (emitter+0x10), then
               bump the emitter block's +0x28 float by 1.0 */
            u8 *dst = g_soundEmitterTable + 0x20 + handle * 0x70;
            u8 *src = (u8 *)a2 + 0x10;
            f32 *pw;
            *(u32 *)(dst + 0x0) = *(u32 *)(src + 0x0);
            *(u32 *)(dst + 0x4) = *(u32 *)(src + 0x4);
            *(u32 *)(dst + 0x8) = *(u32 *)(src + 0x8);
            *(u32 *)(dst + 0xC) = *(u32 *)(src + 0xC);
            pw = (f32 *)(g_soundEmitterTable + handle * 0x70 + 0x28);
            *pw = *pw + 1.0f;
        } else {
            func_00283638((Vec4 *)(g_soundEmitterTable + 0x20 + handle * 0x70));
            status = 0x11;
        }

        *(s32 *)(slot + 0x70) = -1;
        *(u8 *)(slot + 0x75) = (u8)status;
        *(u8 *)(slot + 0x74) = 8;
        *(s16 *)(slot + 0x82) = 0;
        *(s32 *)(slot + 0x84) = 0;
    }

    /* fire the sample (pan re-read from the just-armed dialogArg0 field) */
    snd_PlaySample((s64)sampleStart, (s64)sampleEnd, 0, 0,
                   g_fileLoadVoiceState.dialogArg0, 0, code, 0, 0, 1,
                   (void *)OnDialogVoiceStarted, (long)(u32)(m + 0x68));
    return 0;
}
#endif

/* Stop the secondary dialog voice channel. No-op unless the secondary voice is
 * allocated (secondaryState != 0) and is currently in its playing state
 * (secondaryFlag == 3); in that case it sends the snd stop command and advances
 * the secondary flag to 4 (stopping). Returns 1 when it issued the stop, 0
 * otherwise.
 * WALL (cc1 2.9): save-layout — saves $16 + $31 (two callee-saves at
 * 8-byte spacing), which the pinned cc1 packs at 16-byte spacing. */
/* GUARD (task #1271): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_StopDialogVoice)
S136OS_SLOT(StopDialogVoice);
#else
/* MATCHED on the s136os arm (task #1271), not by cc1 2.9; save-layout wall. */
extern void func_00133400(s32 handle);     /* 0x133400 snd_StopVoice: queues 989snd cmd 0x2E for the voice handle */
s32 StopDialogVoice(void) {
    if (g_fileLoadVoiceState.secondaryState == 0) {
        return 0;
    }
    if (g_fileLoadVoiceState.secondaryFlag != 3) {
        return 0;
    }
    func_00133400(g_fileLoadVoiceState.secondaryState);
    g_fileLoadVoiceState.secondaryFlag = 4;
    return 1;
}
#endif

/* Start an ambient/secondary-channel voice from sample-table entry `idx`.
 * No-op if the ambient channel is already busy (ambientState != 0) or the entry
 * is empty. Arms the ambient state machine (state -1, flag 1, volume 10, sample
 * rate 48000), records idx/flags/pan, and plays the sample pair
 * [entry idx, entry idx+1] (each + the global-WAD base) via snd_PlaySample with
 * OnAmbientVoiceStarted as the start callback and &ambientState as its context.
 *
 * params: idx   sample-table index (stride-8 entries at g_discToc + 0x5300)
 *         flags stashed to ambientArg1
 *         pan   stashed to ambientArg2 and passed (s16) as the pan argument
 * return: none. The ROM never sets $2 on any path and neither caller reads it
 *         (UpdateDialogVoiceManager, OnAmbientVoiceStarted), so it is void.
 *
 * MATCHED on the s136os arm (task #1744). Three spellings carry the match:
 *  - start/end are 64-bit `long` locals narrowed with (s32) at the call. That
 *    truncation is what makes cc1 emit the ROM's explicit dsll32/dsra32 after
 *    the addu; an (s64)/(s32) cast on the bare sum folds into the addu and
 *    emits no extension.
 *  - the end index is `(idx + 1) << 1`, which keeps the ROM's recomputed
 *    (idx+1)*8 + table address; `(idx + 1) * 2` folds to idx*8+8 and CSEs.
 *  - start/end are computed after the state stores, right before the call:
 *    that order gives the stack-argument temps the ROM's $9/$10/$11. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_StartAmbientVoice)
S136OS_SLOT(StartAmbientVoice);
#else
void StartAmbientVoice(s32 idx, s32 flags, s32 pan) {
    u8 *toc;
    s32 *sampleTable;
    long start, end;
    if (g_fileLoadVoiceState.ambientState != 0) {
        return;
    }
    toc = g_discToc;
    sampleTable = (s32 *)(toc + 0x5300);
    if (sampleTable[idx * 2] == 0) {
        return;
    }
    g_fileLoadVoiceState.ambientState = 0xFFFFFFFF;
    g_fileLoadVoiceState.ambientFlag = 1;
    g_fileLoadVoiceState.ambientVolume = 10;
    g_fileLoadVoiceState.ambientSampleRate = 48000;
    g_fileLoadVoiceState.ambientArg1 = flags;
    g_fileLoadVoiceState.ambientArg0 = idx;
    g_fileLoadVoiceState.ambientArg2 = pan;
    g_fileLoadVoiceState.ambientCursor = 0;
    start = sampleTable[idx * 2] + g_dialogSampleBase;
    end = sampleTable[(idx + 1) << 1] + g_dialogSampleBase;
    snd_PlaySample((s32)start, (s32)end,
                   0, 0, (s16)pan, 0, 1, 0, 0, 1, (void *)OnAmbientVoiceStarted,
                   (long)(u32)&g_fileLoadVoiceState.ambientState);
}
#endif

/* Like StartAmbientVoice but gated on idx >= 0 and uses func_002B8ED0 as the
 * voice-start callback (the secondary-channel variant), with 0 as the 10th
 * snd_PlaySample argument. No-op if idx is negative, the ambient channel is
 * busy, or the sample-table entry is empty.
 *
 * params: idx   sample-table index (stride-8 entries at g_discToc + 0x5300)
 *         flags stashed to ambientArg1
 *         pan   stashed to ambientArg2 and passed (s16) as the pan argument
 * return: none. The ROM never sets $2 on any path, and 1EFFC0 already declares
 *         it void.
 *
 * MATCHED on the s136os arm (task #1744) with StartAmbientVoice's spellings:
 * `long` start/end narrowed with (s32) at the call (the explicit dsll32/dsra32),
 * the `(idx + 1) << 1` end index, and start/end computed right before the call.
 * The state stores lead with state/flag/volume/rate, as in StartAmbientVoice. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_StartSecondaryVoice)
S136OS_SLOT(StartSecondaryVoice);
#else
void StartSecondaryVoice(s32 idx, s32 flags, s32 pan) {
    u8 *toc;
    s32 *sampleTable;
    long start, end;
    if (idx < 0) {
        return;
    }
    if (g_fileLoadVoiceState.ambientState != 0) {
        return;
    }
    toc = g_discToc;
    sampleTable = (s32 *)(toc + 0x5300);
    if (sampleTable[idx * 2] == 0) {
        return;
    }
    g_fileLoadVoiceState.ambientState = 0xFFFFFFFF;
    g_fileLoadVoiceState.ambientFlag = 1;
    g_fileLoadVoiceState.ambientVolume = 10;
    g_fileLoadVoiceState.ambientSampleRate = 48000;
    g_fileLoadVoiceState.ambientArg0 = idx;
    g_fileLoadVoiceState.ambientArg2 = pan;
    g_fileLoadVoiceState.ambientArg1 = flags;
    g_fileLoadVoiceState.ambientCursor = 0;
    start = sampleTable[idx * 2] + g_dialogSampleBase;
    end = sampleTable[(idx + 1) << 1] + g_dialogSampleBase;
    snd_PlaySample((s32)start, (s32)end,
                   0, 0, (s16)pan, 0, 1, 0, 0, 0, (void *)func_002B8ED0,
                   (long)(u32)&g_fileLoadVoiceState.ambientState);
}
#endif

/* Chain a follow-on secondary-voice segment (the seamless continuation of an
 * already-playing ambient/secondary voice). No-op unless the channel is in the
 * chainable state (ambientFlag != 9, ambientState allocated and not -1) and the
 * next sample-table entry exists. Arms ambientFlag 9 and plays the continuation
 * sample pair via snd_PlaySample with func_002B8E78 as the start callback.
 * The continuation plays from entry idx+3 to entry idx+2 (the ROM passes them in
 * that order). The 8th argument is the live voice handle being chained
 * (ambientState, still in $11 from the chainable test; the ROM never zeroes $11),
 * and the 10th is (flags & 1) << 2.
 *
 * params: idx   base sample-table index of the segment pair being chained
 *         flags stashed to ambientArg1; bit 0 selects the 10th argument's bit 2
 *         pan   stashed to ambientArg2 and passed (s16) as the pan argument
 * return: none. The ROM never sets $2 on any path.
 *
 * MATCHED on the s136os arm (task #1744) with StartAmbientVoice's spellings
 * (`long` start/end narrowed at the call, `(idx + k) << 1` indices, start/end
 * computed after the stores), end computed before start, and the handle passed
 * as the 8th argument. Master's C passed 0 there: a functional defect in the
 * #else arm as well as the residual that kept it from matching. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_ChainSecondaryVoice)
S136OS_SLOT(ChainSecondaryVoice);
#else
void ChainSecondaryVoice(s32 idx, s32 flags, s32 pan) {
    u8 *toc;
    s32 *sampleTable;
    u32 handle;
    long start, end;
    if (g_fileLoadVoiceState.ambientFlag == 9) {
        return;
    }
    handle = g_fileLoadVoiceState.ambientState;
    if (handle == 0 || handle == 0xFFFFFFFF) {
        return;
    }
    toc = g_discToc;
    sampleTable = (s32 *)(toc + 0x5300);
    if (sampleTable[(idx + 1) << 1] == 0) {
        return;
    }
    g_fileLoadVoiceState.ambientFlag = 9;
    g_fileLoadVoiceState.ambientVolume = 10;
    g_fileLoadVoiceState.ambientSampleRate = 48000;
    g_fileLoadVoiceState.ambientArg0 = idx;
    g_fileLoadVoiceState.ambientArg2 = pan;
    g_fileLoadVoiceState.ambientArg1 = flags;
    g_fileLoadVoiceState.ambientCursor = 0;
    end = sampleTable[(idx + 2) << 1] + g_dialogSampleBase;
    start = sampleTable[(idx + 3) << 1] + g_dialogSampleBase;
    snd_PlaySample((s32)start, (s32)end,
                   0, 0, (s16)pan, 0, 1, handle, 0, (flags & 1) << 2, (void *)func_002B8E78,
                   (long)(u32)&g_fileLoadVoiceState.ambientState);
}
#endif

/* Start the tertiary (third-priority) voice channel from sample-table entry
 * `idx` if its TOC bank exists. Arms the tertiary state block (state 1, volume
 * 10, sample rate 48000) and plays via snd_PlaySample with func_002B8E28 as the
 * start callback; returns 1 on a started voice, 0 otherwise.
 * Same shape and residual as StartAmbientVoice (SCHED-TIEBREAK); the nested
 * ifs reproduce the ROM's shared return-0 block at the end. sdk29 72.81% /
 * engine96 29.30% (unit objdiff report, task #510).
 *
 * a1 is the sample-table index (stride-8 g_dialogSampleTable); a0/a2/a3 are
 * stashed to the tertiary state block (+0x90/+0x94/+0x92) with volume 10 and rate
 * 48000. Uses raw offsets for the unnamed tertiary fields (+0x90/+0x92/+0x9C/
 * +0xA0/+0xA4) and struct names for tertiaryState/tertiaryArg1/tertiaryFlag.
 * Tertiary passes sampleEnd=0, pan=(s16)a3, 0x20 as the 10th argument,
 * func_002B8E28 as the start callback, and &tertiaryState as the context. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartTertiaryVoice);
#else
/* TODO(match): functional equivalent - not byte-exact; SCHED-TIEBREAK, sdk29 72.81% / engine96 29.30%. */
extern void func_002B8E28(s32 voiceId, long handle);   /* 0x2B8E28 tertiary voice-start callback (defined below) */
s32 StartTertiaryVoice(s32 a0, s32 a1, s32 a2, s32 a3) {
    u8 *m = (u8 *)&g_fileLoadVoiceState;
    u8 *toc;
    if (g_fileLoadVoiceState.tertiaryState == 0) {  /* channel free */
        toc = g_discToc;
        if (*(s32 *)(toc + a1 * 8 + 0x5300) != 0) {  /* sample bank exists for this id */
    g_fileLoadVoiceState.tertiaryArg1 = (s16)a2;
    *(s32 *)(m + 0xA0) = 10;                /* volume */
    *(s32 *)(m + 0xA4) = 48000;            /* sample rate (0xBB80) */
    g_fileLoadVoiceState.tertiaryState = 0xFFFFFFFF;
    *(s16 *)(m + 0x90) = (s16)a0;
    *(s16 *)(m + 0x9C) = 1;
    g_fileLoadVoiceState.tertiaryFlag = 1;
    *(s16 *)(m + 0x92) = (s16)a3;
    snd_PlaySample(*(s32 *)(toc + 0x52FC) + *(s32 *)(toc + a1 * 8 + 0x5300),
                   0, 0, 0, (s16)a3, 0, 1, 0, 0, 0x20, (void *)func_002B8E28,
                   (long)(u32)&g_fileLoadVoiceState.tertiaryState);
            return 1;
        }
    }
    return 0;
}
#endif

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
 * WALL (matching build, cc1 2.9 arm): save-layout — 3 callee-saves + $ra at
 * 8-byte spacing.
 * Save stride NOT re-measured for this member on the s136os arm (SN 1.36
 * -fopt-stack): NOTE #9871's census vocabulary did not match this label, so
 * FACT #9873's re-screen never ran it. Of the 111 labeled members that were
 * run, 0 reproduce the 16-byte save stride there, so the stride is not
 * evidence that this member is walled. Residual: UNMEASURED. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ResetDialogVoiceChannels);
#else
/* TODO(match): functional equivalent - not byte-exact; save-layout wall on cc1 2.9 (s136os unmeasured, see above). */
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
 *
 * @param includeSecondary nonzero to also raise ch1
 *
 * The ROM issues ONE `lui %hi(g_fileLoadVoiceState)` and keeps it in a register
 * across the ch1 block and the unconditional ch2(+0x98)/ch0(+0x50) block; that
 * sharing is the gcse (load-PRE) pass, which the unit's 2.9 -fno-gcse pin
 * suppresses (FACT #7922). The ch0 pair is written before the ch2 pair in the
 * source: SN 1.36's scheduler then emits them in the ROM's ch2-first order.
 * Neither change closes it alone (FACT #9003, counts of DIFFERING words out of
 * 13: pin + this order, 12 differ; unpinned + the old ch2-first order, 4 differ).
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) at the -O2 default, without the 2.9 arm's
 * -fno-gcse (the flag table's S136EXTRA for this unit, RULING #9004), and
 * spliced over S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice drops the function. On native it is
 * plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_SetDialogVoiceVolumesMax)
S136OS_SLOT(SetDialogVoiceVolumesMax);
#else
void SetDialogVoiceVolumesMax(s32 includeSecondary) {
    if (includeSecondary) {
        g_fileLoadVoiceState.ch1.volume = (s16)-0x8000;
        g_fileLoadVoiceState.ch1.fadeTarget = 0;
    }
    g_fileLoadVoiceState.ch0.volume = (s16)-0x8000;
    g_fileLoadVoiceState.ch0.fadeTarget = 0;
    g_fileLoadVoiceState.ch2.volume = (s16)-0x8000;
    g_fileLoadVoiceState.ch2.fadeTarget = 0;
}
#endif

/* Mute all three dialog-voice channels (volume state = 4). Only each channel's
 * volume word is written; the fade target is left untouched.
 *
 * MATCHED 100.00% on the sdk29 arm (unit objdiff report, objdiff_build.sh +
 * unit_report.sh, clean; task #510). The three stores are independent and cc1
 * 2.9's scheduler emits the source-LAST of them first, then the rest in source
 * order; the ROM's order is ch1, ch0, ch2 (ch2 in the jr delay slot), so the
 * source lists them as ch0, ch2, ch1. */
void SetDialogVoiceVolumesMute(void) {
    g_fileLoadVoiceState.ch0.volume = 4;
    g_fileLoadVoiceState.ch2.volume = 4;
    g_fileLoadVoiceState.ch1.volume = 4;
}

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

/* Step one dialog-voice channel's per-frame playback state machine. No-op (and
 * on some transitions frees the channel slot) unless a voice is allocated; drives
 * the 989snd command ring for the channel's voice handle (+0x00) based on its
 * state (+0x0A) and sub-phase (+0x0C):
 *   - state 5: send cmd 0x15 (func_00132A70) and advance to state 6;
 *   - substate wants key-on (+0x0C bit 0x8000) and not yet latched: send cmd 0x2D
 *     (func_001333D0) + latch state bit 0x8000; when the frame timer (+0x0E)
 *     expires (func_00283328 == 2) mark the sub-phase finishing (+0x0C = 4);
 *   - key-on latched but sub-phase no longer wants it: send stop cmd 0x2E
 *     (func_00133400) + clear the latch;
 *   - state 2: install the voice-event callback (cmd 0x4F, func_00133460);
 *   - default (not 1/2/3/8/9): install the sample-cursor + tertiary-started
 *     callbacks (cmd 0x32 / 0x19) and invalidate the handle (+0x00 = -1);
 *   - state 7 / no handle: clear state; when the resolved state is 0 free the
 *     slot (+0x04 = -1).
 * WALL: splat jtbl reloc-identity gap (the callback-install %hi/%lo refs) +
 * save-layout — 3 callee-saves + $ra at 8-byte spacing; matching arm stays
 * INCLUDE_ASM, portable #else below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StepDialogVoiceChannel);
#else
extern void func_00132A70(s32 handle);    /* 0x132A70 989snd cmd 0x15 for the voice handle */
extern void func_001333D0(s32 handle);    /* 0x1333D0 989snd cmd 0x2D (voice key-on) */
extern void func_00133400(s32 handle);    /* 0x133400 snd_StopVoice: 989snd cmd 0x2E */
extern void func_00133430(s32 handle, void *cb, long ctx);  /* 0x133430 989snd cmd 0x32 (install cb) */
extern void func_00132B58(s32 handle, void *cb, long ctx);  /* 0x132B58 989snd cmd 0x19 (install cb) */
extern void func_00133460(s32 handle, void *cb, long ctx);  /* 0x133460 989snd cmd 0x4F (install cb) */
extern void func_002B8F68(s32 byteCursor, long handleAddr);       /* below; sample-cursor callback */
extern void OnTertiaryVoiceStarted(s32 voiceId, long handleAddr); /* below; tertiary voice-started callback */
extern void func_002B8D18(s32 flag, long handleAddr);            /* below; voice-event callback */

/* Channel record fields (offsets observed from the asm; no dedicated struct):
 *   +0x00 s32 handle    active 989snd voice handle (0 = none, -1 = pending/invalid)
 *   +0x04 s16 slotId    channel slot id (set -1 = free once the voice resolves off)
 *   +0x0A s16 state     playback state (1/2/3/5/6/7/8/9; bit 0x8000 = key-on latched)
 *   +0x0C s16 substate  sub-phase (bit 0x8000 = wants key-on; 4 = finishing)
 *   +0x0E s16 timer     frame timer ticked by func_00283328
 */
void StepDialogVoiceChannel(u8 *ch) {
    s32 handle;
    s16 st;

    if (*(s16 *)(ch + 0xA) == 9) goto cleanup;
    if (*(s32 *)(ch + 0x0) == 0) goto cleanup;
    if (*(s32 *)(ch + 0x0) == -1) goto cleanup;

    if (*(s16 *)(ch + 0xA) == 5) {
        if (*(s32 *)(ch + 0x0) == 0) {   /* handle != 0 here; dead but faithful */
            *(s16 *)(ch + 0xA) = 0;
            goto handle_check;
        }
        func_00132A70(*(s32 *)(ch + 0x0));
        *(s16 *)(ch + 0xA) = 6;
        goto handle_check;
    }
    if (*(s16 *)(ch + 0xA) == 6) {
        if (*(s32 *)(ch + 0x0) == 0) {   /* handle != 0 here; dead but faithful */
            *(s16 *)(ch + 0xA) = 0;
            goto handle_check;
        }
        goto substate_check;
    }

handle_check:
    if (*(s32 *)(ch + 0x0) == 0) goto finalize;
substate_check:
    if ((*(s16 *)(ch + 0xC) & 0x8000) != 0) {
        if ((*(s16 *)(ch + 0xA) & 0x8000) == 0) {
            func_001333D0(*(s32 *)(ch + 0x0));
            *(u16 *)(ch + 0xA) |= 0x8000;
        }
        if (func_00283328((s16 *)(ch + 0xE)) == 2) {
            *(s16 *)(ch + 0xC) = 4;
        }
    } else {
        if ((*(s16 *)(ch + 0xA) & 0x8000) != 0) {
            func_00133400(*(s32 *)(ch + 0x0));
            *(u16 *)(ch + 0xA) ^= 0x8000;
        }
    }

    if ((*(s16 *)(ch + 0xA) & 0x8000) != 0) goto finalize;

    st = *(s16 *)(ch + 0xA);
    if (st == 1 || st == 8 || st == 9) goto finalize;
    if ((u32)(st - 2) < 2) {                 /* state 2 or 3 */
        if (*(s32 *)(ch + 0x0) == -1) goto finalize;
        if (st == 2) {
            func_00133460(*(s32 *)(ch + 0x0), (void *)&func_002B8D18, (long)(u32)ch);
        }
        goto finalize;
    }
    /* default: not 1/2/3/8/9 */
    handle = *(s32 *)(ch + 0x0);
    func_00133430(handle, (void *)&func_002B8F68, (long)(u32)ch);
    *(s32 *)(ch + 0x0) = -1;
    func_00132B58(handle, (void *)&OnTertiaryVoiceStarted, (long)(u32)ch);
    goto finalize;

cleanup:
    if (*(s16 *)(ch + 0xA) == 7) {
        *(s16 *)(ch + 0xA) = 0;
        goto finalize;
    }
    if (*(s32 *)(ch + 0x0) != 0) goto finalize;
    *(s16 *)(ch + 0xA) = 0;

finalize:
    if (*(s16 *)(ch + 0xA) == 0) {
        *(s16 *)(ch + 0x4) = -1;
    }
}
#endif

/* Per-frame tick of the dialog/voice manager (base = g_fileLoadVoiceState =
 * g_saveImageArea + 0x1000). Three cooperating pieces:
 *
 *   1. Queue-consume: if the primary channel is idle and a secondary voice is
 *      queued (+0x4C bit0), start it (StartSecondaryVoice); else chain onto an
 *      in-flight secondary (ChainSecondaryVoice) when +0x44 is armed and +0x4E==8.
 *   2. Primary-voice progress: when the aux slot at +0x38 is armed
 *      (TickCountdownTimer), either finish the primary voice (latch the next state +
 *      arm the +0x38 timer) or kick the tertiary voice (StartTertiaryVoice); and
 *      launch the queued dialog voice (StartDialogVoice) once its bank handle
 *      (+0x24) is ready.
 *   3. State machine: dispatch on state (+0x2C, jtbl_0026CBF0 over states 2..6)
 *      to advance the crossfade/timing (states 2 & 6 issue a 989snd play command
 *      func_00132BC0 with the OnTertiaryVoiceStarted callback), then step the
 *      three voice channels (+0x44, +0x8C, +0x68) and pump the file-load
 *      completion path: on an aborted read (+0x6) fire the completion callback
 *      (+0x18) with success=0; otherwise (re)kick a queued read (StartFileLoad).
 *
 * The manager reinterprets the file-load struct's fields cross-purpose (+0x2C is
 * the switch state, not fileLoadPhase; +0x44 is a voice handle, not ambientState;
 * +0x30/+0x34 are crossfade timers), and several offsets (+0x7 enable, +0x28,
 * +0x38, +0x9C, +0xA4) have no named field — so the manager body uses raw offsets
 * on the base pointer and reserves the struct-field names for the genuine
 * file-load tail. Control flow is transcribed op-for-op (goto labels = asm block
 * addresses) with branch-likely delay-slot discipline; the two crossfade divides
 * carry the original div-by-zero guards (denominator +0x34 non-zero on the taken
 * path). Region-portable: no divergent immediates; the EU twin reuses this body
 * verbatim (symbols resolve per-region via symbol_addrs).
 *
 * NATIVE #else for coverage — the matching arm stays INCLUDE_ASM (byte-frozen).
 * WALL (matching build, cc1 2.9 arm): switch dispatch (the state-2 jtbl) +
 * save-layout (3 callee-saves + $ra at 8-byte spacing).
 * Save stride NOT re-measured for this member on the s136os arm (SN 1.36
 * -fopt-stack): NOTE #9871's census vocabulary did not match this label, so
 * FACT #9873's re-screen never ran it. Of the 111 labeled members that were
 * run, 0 reproduce the 16-byte save stride there, so the stride is not
 * evidence that this member is walled. Residual: UNMEASURED. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateDialogVoiceManager);
#else
/* TODO(match): functional equivalent - not byte-exact; switch-dispatch + save-layout wall on cc1 2.9 (s136os unmeasured, see above). */
extern s32  StartTertiaryVoice(s32 a0, s32 a1, s32 a2, s32 a3);        /* 0x2B7FA8 (INCLUDE_ASM above) */
extern void func_00132BC0(s32 handle, s32 cmd, s32 sampleCount, s32 a3,
                          s32 a4, s32 a5, void *startCb, void *ctx);   /* 0x132BC0 989snd play command */
extern s32  snd_CheckLoadInProgress(s32 flag);                        /* 0x133580 tests/waits bank-load-in-progress */
extern s32  StartFileLoad(s32 dest, s32 lbn, s32 sectorCount);        /* 0x2B8A18 (defined below) */
extern char D_1AA1B0[];                    /* dialog-voice load debug strings (retail no-ops) */
extern char D_1AA1C0[];
extern char D_1AA1D0[];
void UpdateDialogVoiceManager(void) {
    u8 *m = (u8 *)&g_fileLoadVoiceState;   /* manager base = g_saveImageArea + 0x1000 */
    s16 dvid;
    s16 idx;
    s32 h;
    s32 old44;
    s32 num;
    s32 prod;
    s32 v6;
    void (*cb)(void *, s32);   /* completion callback: fn(void *ctx, s32 status) — CONFIRMED
                                * from func_002949E0(s32 *rec, s32) installed via
                                * StartFileLoadWithCallback with ctx = &D_1A9330 (a pointer) */
    s32 cbArg;                 /* raw +0x1C word; holds the callback's context pointer */
    s16 prev;
    s32 fdest, flbn, fcnt;

    if (*(u8 *)(m + 0x7) != 0) {
        return;                            /* manager disabled */
    }

    /* --- 1. consume queued secondary / chained voice --- */
    if ((*(u16 *)(m + 0x50) & 0x8000) != 0) goto L8568;   /* primary channel active */
    if ((*(u16 *)(m + 0x4E) & 0x8000) != 0) goto L8568;
    if (*(s32 *)(m + 0x44) != 0) goto L8534;
    if ((*(u16 *)(m + 0x4C) & 0x1) == 0) goto L8524;      /* no secondary queued */
    if (*(s16 *)(m + 0x3C) != -1) goto L8528;             /* primary already assigned */
    StartSecondaryVoice(*(s16 *)(m + 0x48), *(s16 *)(m + 0x4C), *(s16 *)(m + 0x4A));
    goto L8568;
L8524:
L8528:
    if (*(s32 *)(m + 0x44) == 0) goto L8564;
L8534:
    if (*(s32 *)(m + 0x44) == -1) goto L8564;
    if (*(s16 *)(m + 0x4E) != 8) goto L8568;
    ChainSecondaryVoice(*(s16 *)(m + 0x48), *(s16 *)(m + 0x4C), *(s16 *)(m + 0x4A));
L8564:
L8568:
    /* --- 2. primary-voice progress (aux slot at +0x38) --- */
    if (TickCountdownTimer(m + 0x38) == 0) goto L8608;
    dvid = *(s16 *)(m + 0x3C);
    if (dvid == -1) goto L860C;
    if (*(s16 *)(m + 0x2C) != 0) goto L860C;
    if (*(s16 *)(m + 0x48) == dvid) {
        *(s16 *)(m + 0x3C) = -1;
        goto L8608;
    }
    if (*(s16 *)(m + 0x3E) != -1) goto L85F4;
    /* primary voice finished: latch the next state + arm the +0x38 timer */
    if ((*(u16 *)(m + 0x4E) & 0x8000) != 0) {
        *(s16 *)(m + 0x4E) = 5;
        *(s16 *)(m + 0x2C) = 3;
    } else {
        *(s16 *)(m + 0x2C) = 6;
        *(s32 *)(m + 0xA4) = 15;
        *(s32 *)(m + 0x34) = 15;
    }
    *(s32 *)(m + 0x38) = 300;
    goto L8608;
L85F4:
    if (StartTertiaryVoice(dvid, *(s16 *)(m + 0x3E),
                           *(s16 *)(m + 0x4C), *(s16 *)(m + 0x4A)) != 0) {
        *(s32 *)(m + 0x38) = 420;
    }
L8608:
L860C:
    /* launch the queued dialog voice once its bank handle (+0x24) is ready */
    h = *(s32 *)(m + 0x24);
    if (h < 0) goto L8660;
    if (*(s32 *)(m + 0x68) != 0) {
        if ((u32)(*(u16 *)(m + 0x72) - 6) < 2u) goto L8660;
        *(s16 *)(m + 0x72) = 5;
        goto L8660;
    }
    StartDialogVoice(h, 0, *(s32 *)(m + 0x28), 0x400);
    *(s32 *)(m + 0x28) = 0;
    *(s32 *)(m + 0x24) = -1;
L8660:
    /* --- 3. state machine: dispatch on state (+0x2C) --- */
    if (*(s16 *)(m + 0x4E) == 9) goto L8918;
    if (*(s32 *)(m + 0x44) == -1) goto L891C;
    if ((*(u16 *)(m + 0x50) & 0x8000) != 0) goto L8918;
    if ((*(u16 *)(m + 0x4E) & 0x8000) != 0) goto L8918;
    if (*(s16 *)(m + 0x3E) == -1) goto L86D4;
    if ((*(u16 *)(m + 0x98) & 0x8000) != 0) goto L8918;
    if ((*(u16 *)(m + 0x96) & 0x8000) != 0) goto L8918;
L86D4:
    idx = (s16)(*(u16 *)(m + 0x2C) - 2);
    if ((u32)idx >= 5) goto L8914;         /* default: no active state */
    switch (idx) {                         /* jtbl_0026CBF0_text (state - 2) */
    case 0: goto L8778;                    /* state 2 */
    case 1: goto L8834;                    /* state 3 */
    case 2: goto L8880;                    /* state 4 */
    case 3: goto L88F0;                    /* state 5 */
    case 4: goto L8704;                    /* state 6 */
    default: goto L8914;
    }

L8704: /* state 6: crossfade by (+0x4A * +0xA4) / +0x34, then play + advance */
    prod = (s32)*(s16 *)(m + 0x4A) * *(s32 *)(m + 0xA4);
    old44 = *(s32 *)(m + 0x44);
    *(s32 *)(m + 0x44) = -1;
    v6 = prod / *(s32 *)(m + 0x34);
    func_00132BC0(old44, 5, v6, 0, 0, 0, (void *)&OnTertiaryVoiceStarted, m + 0x44);
    if (TickCountdownTimer(m + 0xA4) == 0) goto L8914;
    *(s16 *)(m + 0x4E) = 5;
    *(s16 *)(m + 0x2C) = 3;
    goto L8914;

L8778: /* state 2: crossfade-in by +0x4A * (+0x34 - (+0x30 - +0xA4)) / +0x34 */
    num = *(s32 *)(m + 0x34) - (*(s32 *)(m + 0x30) - *(s32 *)(m + 0xA4));
    v6 = ((s32)*(s16 *)(m + 0x4A) * num) / *(s32 *)(m + 0x34);
    if (v6 <= 0) goto L87E0;
    if (*(s16 *)(m + 0x96) != 4) {
        if (*(s32 *)(m + 0x8C) == 0) goto L87E0;
        goto L87D4;
    }
    if (*(s16 *)(m + 0x9C) != 0) goto L87D4;
    if (*(s32 *)(m + 0x8C) == 0) goto L87E0;
L87D4:
    old44 = *(s32 *)(m + 0x44);
    if (old44 == 0) goto L87E0;
    if (*(s16 *)(m + 0x4E) == 9) goto L8914;
    *(s32 *)(m + 0x44) = -1;
    func_00132BC0(old44, 5, v6, 0, 0, 0, (void *)&OnTertiaryVoiceStarted, m + 0x44);
    goto L8918;
L87E0:
    *(s16 *)(m + 0x2C) = 3;
    *(s16 *)(m + 0x4E) = 5;
    goto L8914;

L8834: /* state 3: start the ambient/looping voice, then advance to state 4 */
    if (*(s16 *)(m + 0x4E) != 0) goto L8918;
    if (*(s16 *)(m + 0x9C) != 0) goto L8860;
    if (*(s32 *)(m + 0x8C) != 0) goto L8918;
L8860:
    StartAmbientVoice(*(s16 *)(m + 0x3C), *(s16 *)(m + 0x4C), *(s16 *)(m + 0x4A));
    *(s16 *)(m + 0x2C) = 4;
    *(s16 *)(m + 0x3C) = -1;
    goto L8914;

L8880: /* state 4: once the crossfade elapses (+0xA4 >= +0x34), stop the voice */
    if (*(s16 *)(m + 0x4E) != 3) goto L8918;
    if (*(s32 *)(m + 0xA4) < *(s32 *)(m + 0x34)) goto L88D4;
    if (*(s16 *)(m + 0x96) != 4) goto L88D4;
    if (*(s16 *)(m + 0x9C) != 0) goto L8918;
    if (*(s32 *)(m + 0x8C) != 0) goto L891C;
L88D4:
    func_00133400(*(s32 *)(m + 0x44));
    *(s16 *)(m + 0x2C) = 5;
    *(s16 *)(m + 0x4E) = 8;
    goto L8914;

L88F0: /* state 5: hold until the voice has fully stopped, then clear state */
    if (*(s16 *)(m + 0x96) != 4) {
        *(s16 *)(m + 0x2C) = 0;
        goto L8914;
    }
    if (*(s16 *)(m + 0x9C) != 0) goto L8918;
    *(s16 *)(m + 0x2C) = 0;
    /* fall through to L8914 */

L8914:
L8918:
L891C:
    /* step the three dialog voice channels (0x24-byte blocks from +0x44) */
    StepDialogVoiceChannel(m + 0x44);
    StepDialogVoiceChannel(m + 0x8C);
    StepDialogVoiceChannel(m + 0x68);

    /* --- file-load completion / (re)kick --- */
    if (g_fileLoadVoiceState.readStopped != 0) {
        /* read was aborted: fire the completion callback with success=0 */
        DebugPrintStub(D_1AA1B0);
        if (snd_CheckLoadInProgress(1) != 0) {
            return;                        /* still loading */
        }
        g_fileLoadVoiceState.fileLoadActive = 0;
        g_fileLoadVoiceState.readStopped = 0;
        DebugPrintStub(D_1AA1C0);
        cb = (void (*)(void *, s32))g_fileLoadVoiceState.pLoadCallback;
        if (cb == 0) {
            return;
        }
        cbArg = g_fileLoadVoiceState.loadCallbackArg;
        g_fileLoadVoiceState.loadCallbackArg = 0;
        g_fileLoadVoiceState.pLoadCallback = NULL;
        DebugPrintStub(D_1AA1D0);
        cb((void *)cbArg, 0);
        return;
    }
    if (g_fileLoadVoiceState.fileLoadActive == 2) {
        /* a read is queued (state 2): clear the active gate so StartFileLoad
         * proceeds, kick it, and restore the request state if it refused */
        prev = g_fileLoadVoiceState.fileLoadActive;
        fdest = g_fileLoadVoiceState.fileLoadDest;
        flbn = g_fileLoadVoiceState.fileLoadLbn;
        fcnt = g_fileLoadVoiceState.fileLoadSectorCount;
        g_fileLoadVoiceState.fileLoadActive = 0;
        StartFileLoad(fdest, flbn, fcnt);
        if (g_fileLoadVoiceState.fileLoadActive == 0) {
            g_fileLoadVoiceState.fileLoadActive = prev;
        }
    }
}
#endif

/* Abort the in-flight CD file read: if a read is active, emit the
 * "music_StopLoad" debug string (retail no-op), send the stop command, and set
 * the abort flag so the completion callback receives success=false.
 * cc1 2.9 wall (the s136os arm clears it; see GUARD): save-layout — saves
 * $16 + $31 (two callee-saves at 8-byte spacing), which the pinned cc1 packs
 * at 16-byte spacing. */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): CdStopRead declared s32 (it
 * returns 1; cod/0321A0.c), so the call clobbers $2 as the ROM's allocation
 * assumes. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_StopFileLoad)
S136OS_SLOT(StopFileLoad);
#else
/* cc1 2.9 (not the image arm; see GUARD) is not byte-exact: save-layout wall. */
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
 * Saves $16/$17/$18/$19/$31 at 8-byte spacing, which cc1 2.9 packs at 16-byte
 * spacing; MATCHED on the s136os arm, which emits that packed layout (FACT #8830).
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_StartFileLoad)
S136OS_SLOT(StartFileLoad);
#else
extern char D_1AA1F8[];                    /* "load file failed to start" debug string */
extern s32 CdStartRead(s32 lbn, s32 sectors, s32 dest, void *rmode);  /* 0x133398 */
extern void func_002833D8(void);           /* spin-wait on fatal load failure */
/* WEAK on TARGET_NATIVE only: cmp_191238d.c supplies a strong StartFileLoad
 * mock; weak lets it win the link when this unit is co-linked into the cmp suite
 * (this body is untested there; gc-sections drops it). The EE body is the
 * image's (s136os arm, task #1309) and must bind GLOBAL like the ROM's, so the
 * attribute is not applied there. */
#ifdef TARGET_NATIVE
__attribute__((weak))
#endif
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
 * WALL (cc1 2.9): save-layout — saves $16/$17/$31 (three callee-saves at 8-byte spacing). */
/* GUARD (task #1271): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
extern s32  StartFileLoad(s32 dest, s32 lbn, s32 sectorCount);        /* 0x2B8A18 (above) */
#if !defined(TARGET_NATIVE) && !defined(S136OS_StartFileLoadWithCallback)
S136OS_SLOT(StartFileLoadWithCallback);
#else
/* MATCHED on the s136os arm (task #1271), not by cc1 2.9; save-layout wall. */
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

/*
 * KickRawFileRead(dest, lbn, sectors) — kick a raw CD read straight through
 * CdStartRead, bypassing the g_fileLoadState request record. Builds a local
 * sceCdRMode from the g_cdReadMode template with the spindle byte overridden by
 * g_rawReadSpindleCtrl, clears the stall watchdog and the fell-back flag, starts
 * the read, then runs one snd RPC tick and one snd queue pump. Used by the
 * level-staging machine (0x294280/0x294310), which polls completion itself.
 *
 * Params: dest — destination buffer; lbn — first sector (logical block number);
 *         sectors — sector count.
 * Returns: always 1. CdStartRead's result is discarded.
 *
 * g_rawReadSpindleCtrl is a word (text/191238 reads it with `lw` and stores it
 * %gp_rel); this function reads only its low byte, as the ROM's `lbu` does.
 *
 * MATCHED 100.00% on the sdk29 arm (unit objdiff report, objdiff_build.sh +
 * unit_report.sh, clean, VM colima-ee-x86-b; task #744). The bracket is the row
 * with that one lever removed:
 *  - g_rawReadSpindleCtrl is modelled `.extern ,16`, not ABSOLUTE_GLOBAL. The
 *    ROM's same-register `lui $7 / lbu $7,%lo($7)` is GNU as expanding a load
 *    macro through its own destination, not a compiler-split %hi [98.75];
 *  - an empty asm with lbn and sectors as "+r" operands at entry: cc1 moves them
 *    into $4/$5 at the top, as the ROM does [75.36];
 *  - dest is held in $2 (`destCopy`) and passed through $6 (`destArg`), and the
 *    spindle byte in $7, as in the ROM
 *    [99.29 / 84.64 / 98.57 with the $2 / $6 / $7 pin removed];
 *  - one barrier after the spindle store keeps the template copy ahead of the
 *    stall-timer store [92.68];
 *  - `destArg` and `modeArg` are set between the two barriers and pinned by the
 *    second, so the $6/$7 argument moves come before the two clears, and the
 *    $7 copy is not a sched2 successor of the `sb` (which would otherwise put
 *    the `lbu` above `lui %hi(g_cdReadMode)`) [90.71].
 */
#ifndef TARGET_NATIVE
#define RAW_READ_REG(r) __asm__(r)
#else
#define RAW_READ_REG(r)
#endif
typedef struct CdReadMode {         /* sceCdRMode */
    u8 tryCount;
    u8 spindleCtrl;
    u8 dataPattern;
    u8 pad;
} CdReadMode;
extern CdReadMode g_cdReadMode ABSOLUTE_GLOBAL;   /* 0x1A63E8 read-mode template */
__asm__(".extern g_rawReadSpindleCtrl, 16");
extern u8 g_rawReadSpindleCtrl;     /* 0x1A7900 spindle/speed override (low byte of a word) */
__asm__(".extern g_rawReadStallTimer, 16");
extern s32 g_rawReadStallTimer;     /* 0x1A7430 raw-read stall watchdog, reset per kick */
extern s32 g_bRawReadFellBack;      /* 0x1A7434 "read fell back to slow path" flag, reset per kick */
extern s32 CdStartRead(s32 lbn, s32 sectors, s32 dest, void *rmode);  /* 0x133398 */
extern void func_00133230(void);    /* 0x133230 snd RPC tick */
extern s32 snd_Pump(void);          /* 0x133280 snd queue pump */
s32 KickRawFileRead(s32 dest, s32 lbn, s32 sectors) {
    CdReadMode rmode;
    register s32 destCopy RAW_READ_REG("$2") = dest;
    register u8 spindle RAW_READ_REG("$7");
    register s32 destArg RAW_READ_REG("$6");
    CdReadMode *modeArg;

    __asm__ __volatile__("" : "+r"(lbn), "+r"(sectors));
    rmode = g_cdReadMode;                   /* copy the read-mode template */
    spindle = g_rawReadSpindleCtrl;
    rmode.spindleCtrl = spindle;
    __asm__ __volatile__("");
    destArg = destCopy;
    modeArg = &rmode;
    __asm__ __volatile__("" : "+r"(modeArg), "+r"(destArg));
    g_rawReadStallTimer = 0;
    g_bRawReadFellBack = 0;
    CdStartRead(lbn, sectors, destArg, modeArg);
    func_00133230();
    snd_Pump();
    return 1;
}

/* Start a file load while keeping the dialog-voice system pumping (the variant
 * used during streamed-cinematic loads): pumps the dialog-voice system once,
 * kicks the file load (StartFileLoad), then pumps the dialog-voice system again,
 * and returns the byte size StartFileLoad reported.
 *
 * Derived
 * register-exact from StartFileLoadPumpingVoice.s @0x2B8BA0 (the pump argument is
 * the literal 1 on both calls; StartFileLoad is fed dest/lbn/sectorCount in arg
 * order and its return is forwarded verbatim).
 *
 * WALL (cc1 2.9): save-layout — 3 callee-saves + $ra at 8-byte spacing. */
/* GUARD (task #1271): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_StartFileLoadPumpingVoice)
S136OS_SLOT(StartFileLoadPumpingVoice);
#else
/* MATCHED on the s136os arm (task #1271), not by cc1 2.9; save-layout wall. */
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
 * UpdateDialogVoiceManager under the right gating). Runs one manager tick +
 * snd-RPC/pump cycle (UpdateDialogVoiceManager, func_00133230, snd_Pump,
 * func_00133220). When `waitForIdle` is nonzero it repeats that cycle, spinning
 * (func_002833E8 delay) between iterations, until the file-load is no longer in
 * flight (fileLoadActive == 0). Returns the final fileLoadActive.
 * GUARD (task #1720): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) at the -O2 default (RULING #9004) and spliced
 * over S136OS_SLOT by tools/ee/s136os_splice.sh. On native it is plain C.
 * The packed save and the hoisted `lui $17` that walled the 2.9/2.96 arms
 * (task #510) are what 1.36 emits from this C as written. The one thing the
 * C has to spell is the busy-wait callee: 0x2833E8 is not a function but the
 * global label `.L002833E8` inside func_002833D8 (text/183348.s `alabel`), so
 * the EE arm binds the call to that label by an asm-label alias; there is no
 * func_002833E8 symbol for the link to resolve. Native keeps the plain name. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_PumpDialogVoiceSystem)
S136OS_SLOT(PumpDialogVoiceSystem);
#else
extern void UpdateDialogVoiceManager(void);   /* below; dialog-voice state manager tick */
extern void func_00133230(void);              /* 0x133230 snd RPC tick */
extern s32  snd_Pump(void);                   /* 0x133280 snd queue pump */
extern void func_00133220(void);              /* 0x133220 snd RPC flush */
#ifndef TARGET_NATIVE
/* ASM-LABEL ALIAS (NOTE #9476): binds the call to the ROM's `.L002833E8`
 * busy-wait entry inside func_002833D8. Emits no instruction. */
extern void func_002833E8(s32 cycles) __asm__(".L002833E8");
#else
extern void func_002833E8(s32 cycles);        /* 0x2833E8 busy-wait spin */
#endif

s16 PumpDialogVoiceSystem(s32 waitForIdle) {
    if (waitForIdle != 0) {
        do {
            UpdateDialogVoiceManager();
            func_00133230();
            snd_Pump();
            func_00133220();
            if (g_fileLoadVoiceState.fileLoadActive == 0) {
                break;
            }
            func_002833E8(0x2710);
        } while (g_fileLoadVoiceState.fileLoadActive != 0);
    } else {
        UpdateDialogVoiceManager();
        func_00133230();
        snd_Pump();
        func_00133220();
    }
    return g_fileLoadVoiceState.fileLoadActive;
}
#endif

/* Per-snd-pump tick handler (installed by InstallFileLoadPump). Only acts when
 * the pump phase arg is 1 (a load is outstanding): polls CdGetLoadStatus, holds
 * g_fileLoadState at 2 while the read is still busy, and on completion flushes
 * the cache, clears the manager's active/abort flags, and fires the registered
 * completion callback fn(arg, success) where success is false iff StopFileLoad
 * aborted the read (readStopped set).
 *
 * MATCHED 100.00% on the sdk29 arm (unit objdiff report, objdiff_build.sh +
 * unit_report.sh, clean; task #510; verify_match_unit.sh BYTE IDENTICAL).
 * Two things had to be spelled the ROM's way: g_fileLoadState is declared
 * ABSOLUTE_GLOBAL (the ROM hoists `lui $3,%hi` into the CdGetLoadStatus beqz
 * delay slot and sinks `sh $2,%lo($3)` into the b delay slot — a compiler-
 * allocated base register, which cc1 only emits for a global it does not class
 * as gp-small), and the callback slot is cleared before its argument word so
 * cc1's "source-last store first" scheduling leaves `sw $0,0x18` in the jalr
 * delay slot. FlushCache is called by its raw-asm glabel name func_0011AEA0
 * (the link binds the glabel, not the SDK header name). */
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
    func_0011AEA0(0);
    success = (g_fileLoadVoiceState.readStopped == 0);
    callback = g_fileLoadVoiceState.pLoadCallback;
    g_fileLoadVoiceState.fileLoadActive = 0;
    g_fileLoadVoiceState.readStopped = 0;
    if (callback != NULL) {
        arg = g_fileLoadVoiceState.loadCallbackArg;
        g_fileLoadVoiceState.pLoadCallback = NULL;
        g_fileLoadVoiceState.loadCallbackArg = 0;
        ((void (*)(void *, s32))callback)((void *)arg, success);
    }
}

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
 *
 * MATCHED 100.00% on the sdk29 arm (unit objdiff report, objdiff_build.sh +
 * unit_report.sh, clean; task #510; verify_match_unit.sh BYTE IDENTICAL). The
 * mirror write keeps +0x70 as the store displacement (`sw $4,0x70($3)`): the
 * slot block pointer is materialised in a local first — written inline,
 * `g_listenerPosHistory + slot*0x70 + 0x70` has cc1 fold the +0x70 into the
 * symbol's %lo. */
void OnDialogVoiceStarted(s32 voiceId, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    handle->voiceId = voiceId;
    if (handle->sampleSlot >= 0) {
        s32 *slotBlock = (s32 *)(g_listenerPosHistory + handle->sampleSlot * 0x70);
        slotBlock[0x70 / 4] = voiceId;
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
 * Non-obvious: the original re-loads handle->sampleCursor from memory for both
 * the +0x30 publish and the +0x34 word-count divide rather than reusing the
 * just-stored register. Reproducing those two reloads is what makes this
 * byte-exact, and it needs the STORE to be alias set 0 as well as the loads:
 * with the store left plain, cc1's CSE hash entry for the address survives the
 * s16 write to fileLoadPhase and only the second reload appears (90.19%).
 * t562 @e3f50d43: the former "WALL: reloaded-ptr CSE ... Genuine reload-vs-CSE
 * wall - functional equivalent only" (dated to before 2026-09-21) is FALSE.
 * -> UNION-RELOAD, 100.00% (unit objdiff report, objdiff_build.sh, clean),
 * verify_match_unit.sh BYTE IDENTICAL 28/28 words. */
void func_002B8F68(s32 byteCursor, long handleAddr) {
    VoiceHandle *handle = (VoiceHandle *)handleAddr;
    if (handle == NULL) {
        return;
    }
    handle->sampleCursor.v = byteCursor;
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
    g_fileLoadVoiceState.fileLoadByteCursor = handle->sampleCursor.v;
    g_fileLoadVoiceState.fileLoadWordCount = handle->sampleCursor.v / 4;
}

/* Handwritten stub-table fragment (orphaned addiu $sp / nop run) — see unit
 * header; kept INCLUDE_ASM permanently. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B8FD8);

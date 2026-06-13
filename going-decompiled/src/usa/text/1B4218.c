#include "common.h"

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
    /* 0x20 */ u8 pad20[0x48];
    /* 0x68 */ s32 *pExtra;       /* extra/pvars block; motion controller at +0x18 */
    /* 0x6C */ u8 pad6C[0x52];
    /* 0xBE */ u8 animFlags;      /* one-shot latch bits set by SetMobyFlagBit* */
    /* 0xBF */ u8 padBF[0x39];
    /* 0xF8 */ f32 moveSpeed;     /* per-frame move speed (moby+0xF8) */
} Moby;

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
    /* 0x06 */ u8 readStopped;            /* set after CdStopRead */
    /* 0x07 */ u8 pad07[0x41];
    /* 0x48 */ s16 ambientArg0;           /* queued ambient-voice params */
    /* 0x4A */ s16 ambientArg2;
    /* 0x4C */ s16 ambientArg1;
    /* 0x4E */ u8 pad4E[0x2];
    /* 0x50 */ DialogVoiceChannel ch0;    /* primary dialog voice channel */
    /* 0x54 */ u8 pad54[0x18];
    /* 0x6C */ s16 dialogArg1;            /* queued dialog-voice params */
    /* 0x6E */ s16 dialogArg0;
    /* 0x70 */ s16 dialogArg2;
    /* 0x72 */ u8 pad72[0x2];
    /* 0x74 */ DialogVoiceChannel ch1;    /* secondary voice channel */
    /* 0x78 */ u8 pad78[0xC];
    /* 0x84 */ s32 dialogArg3;
    /* 0x88 */ u8 pad88[0x10];
    /* 0x98 */ DialogVoiceChannel ch2;    /* tertiary voice channel */
} FileLoadVoiceState;

#define g_fileLoadVoiceState (*(FileLoadVoiceState *)(g_saveImageArea + 0x1000))

/* Callees (value-returning declarations keep cc1 from sibling-call optimising
 * forwarding tails — see text/198FA0). */
extern void SetSndPumpCallback(void *cb);  /* 0x1336D0 */
extern void PumpFileLoadCompletion(void);  /* 0x2B8CA8 snd-pump tick */
extern void CdStopRead(void);              /* 0x133640 */
extern void DebugPrintStub(const char *s); /* 0x26FEC8 retail debug no-op */
extern char D_1AA220[];                    /* tertiary-voice debug string */
extern void OnSoundBankLoaded(s32 bankId, long pOut);  /* 0x2B7690 forward decl */
extern void snd_BankLoadFromEE_CB(s32 arg, void *cb, long pOut);  /* 0x1325E8 */

/* The sound-bank load-status slots live at g_listenerPosHistory + 0x17A0 (s32
 * per bank id); the loader writes -1 there and lets OnSoundBankLoaded fill it. */
#define g_soundBankLoadStatus ((s32 *)(g_listenerPosHistory + 0x17A0))
extern void StepMobyMotion(Moby *moby, Vec4 *target, f32 speed);  /* 0x2B6000 */
extern s32 StartDialogVoice(s32 a0, s32 a1, s32 a2, s32 a3);      /* 0x2B7878 */
extern s32 StartAmbientVoice(s32 a0, s32 a1, s32 a2);            /* 0x2B7CA0 */

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
#define g_dialogVoicePlayingState (*(s16 *)((u8 *)&g_pendingDialogVoiceId + 8))

/* The level-object table (stride 0xA430); field +0x4 of each entry is the
 * backing moby pointer. Used by CheckMobyIsLevelObjectSlot. */
typedef struct LevelObject {
    /* 0x00 */ s32 f0;
    /* 0x04 */ s32 mobyPtr;        /* backing moby pointer for this slot */
    /* 0x08 */ u8 rest[0xA428];
} LevelObject;
extern LevelObject D_2403D0[];              /* 0x2403D0 level-object table */

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateMobyThreatFlashAndBurst);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ClassifyTargetProximity);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B46C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", BindMobyToParent);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", SetMobyFlagBit1);

/* Test moby anim-flag bit 2. */
s32 func_002B47D0(Moby *moby) {
    return moby->animFlags & 4;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CalcMobyTargetThreatDist);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", FindTargetInGroup);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CheckTargetInRangeBand);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", AcquireMobyAutoTarget);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B4F40);

/* True if moby is the backing object of level-object slot `slot`
 * (D_2403D0[slot].mobyPtr == moby). Entry stride 0xA430, moby ptr at +0x4. */
s32 CheckMobyIsLevelObjectSlot(Moby *moby, s32 slot) {
    return D_2403D0[slot].mobyPtr == (s32)moby;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B4F80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ApplyMobyLocalTransformDelta);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B50B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", InitMobySpringFollowState);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StepMobySpringFollow);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", RequestGameStateChange);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", PopGameState);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateGameState);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", DriveMobyTowardPoint);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B5FF8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B61B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateMobyMotionVelocity);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ResolveMobyMotionCollision);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ApplyMobyGroundAndEvents);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ProbeMobyGroundLine);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CheckMobyGroundMover);

/* Returns the moby's motion-controller block: *(*(moby+0x68)+0x18). */
s32 GetMobyMotionController(Moby *moby) {
    return moby->pExtra[0x18 / 4];
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ResolveMobySphereCollision);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ResolveMobyEdgeConstraint);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateMobyLeanFromTurn);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B6FC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", SetMobyMotionParams);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7038);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", DriveMobyAlongWaypoints);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7140);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", SetMobyWaypointPath);

/* Profiling hook: add the current EE RCNT0 count (read as unsigned, hence the
 * bltz/shift float-conversion idiom) to the accumulated moby-motion tick total
 * and bump the sample count. */
void AccumMobyMotionProfile(void) {
    g_mobyMotionProfileCount++;
    g_mobyMotionProfileTicks += (f32)(u32)(*(volatile s32 *)0x10000000);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7218);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CheckMobyPathBlocked);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", CheckMobyOverWater);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", BeginFrameDrawList);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", BindMobyClassUpdateFunc);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B7570);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateActiveMobys);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", InitDialogSoundChannel);

/* Sound-bank load callback: stores the loaded bank id through the (optional)
 * out-pointer. The out-pointer arrives as a 64-bit value whose low 32 bits hold
 * the address (the sign-extend prologue). */
void OnSoundBankLoaded(s32 bankId, long pOut) {
    if ((s32)pOut != 0) {
        *(s32 *)pOut = bankId;
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", LoadGlobalSoundBank);

/* Kick an async EE-side sound-bank load for `bankSlot`, resetting its
 * load-status slot to -1 and registering OnSoundBankLoaded as the completion
 * callback (the status pointer is zero-extended to 64 bits for the RPC).
 * WALL: address-fold. The original materialises `%lo(g_listenerPosHistory)`
 * then adds 0x17A0 with a separate `addiu` (two-step base); this cc1 folds the
 * 0x17A0 into the symbol's %lo reloc (one addiu), so the status-slot address
 * computation diverges (best 54%). Same fold artifact as OnDialogVoiceStarted.
 * Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", LoadLevelSoundBank);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", KickLevelBankDiscLoad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B77E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", InitFileLoadSystem);

/* Register the file-load completion handler as the per-snd-pump tick callback.
 * The empty asm guard blocks cc1's sibling-call (`j`) so the original jal+frame
 * is reproduced. */
void InstallFileLoadPump(void) {
    SetSndPumpCallback(PumpFileLoadCompletion);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartDialogVoice);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StopDialogVoice);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartAmbientVoice);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartSecondaryVoice);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ChainSecondaryVoice);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartTertiaryVoice);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", ResetDialogVoiceChannels);

/* Drive all dialog-voice channels to full volume (state -0x8000, fade target 0).
 * The secondary channel (ch1) is only touched when `includeSecondary` is set.
 * WALL: independent-store rescheduling. The original emits the unconditional
 * ch2(+0x98)/ch0(+0x50) block in descending-offset order; this cc1's scheduler
 * always sorts the two independent same-base stores ascending (ch0 then ch2),
 * regardless of source order or an inter-store barrier (best 87.23%). Left as
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", SetDialogVoiceVolumesMax);

/* Mute all three dialog-voice channels (volume state = 4).
 * WALL: same independent-store rescheduling as SetDialogVoiceVolumesMax — the
 * original stores ch1(+0x74)/ch0(+0x50)/ch2(+0x98) in that order; this cc1
 * reschedules the three same-base stores (best 99.71%, only ordering differs).
 * Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", SetDialogVoiceVolumesMute);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StepDialogVoiceChannel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", UpdateDialogVoiceManager);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StopFileLoad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartFileLoad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartFileLoadWithCallback);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", KickRawFileRead);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", StartFileLoadPumpingVoice);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", PumpDialogVoiceSystem);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", PumpFileLoadCompletion);

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
 * emitter's listener block at +0x70), advance state 1 -> 2, or (when the id is
 * zero) kick the queued dialog voice from the manager's parameters.
 * WALL: address-fold vs displacement. The mirror write
 * `g_listenerPosHistory[slot*0x70 + 0x70]` matches to 99.97%, but the original
 * keeps +0x70 as the store displacement (`sw $4,0x70($3)`) while this cc1 folds
 * it into the materialised base address (one instruction / reg differs). Left
 * as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", OnDialogVoiceStarted);

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

/* Voice-playback callback: record the id, transition state 1 -> 4 when the voice
 * gate is open, and publish the prior state to the playing-state mirror.
 * WALL: store-into-delay-slot scheduling. The structure matches to 90.79%, but
 * the original sinks the final `g_pendingDialogVoiceId+8` store into the `jr`
 * delay slot; this cc1 emits it before the jr (with a nop delay). Left as
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B8E28);

/* Voice-playback callback: record a negative id, transition state 9 -> 4 when
 * the voice gate is open, and publish 1 to the playing-state mirror.
 * WALL: cc1's branch-likely heuristic emits the voiceId==0 test as `bnezl`
 * (branch to the state-check with the state load annulled in the delay) where
 * the original uses a plain `beqz` branching to the state-0 block (best
 * 58-63%); structurally divergent. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B8E78);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B8F68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1B4218", func_002B8FD8);

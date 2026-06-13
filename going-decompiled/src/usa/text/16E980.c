#include "common.h"

/*
 * text/16E980 — head of the big .text segment (carve-pipeline pick #6,
 * 2026-06-13; vaddr 0x26EA00..0x274127, 90 fns): splash/attract boot helpers,
 * the camera system core (slot switching/transitions/shake/fades) and the
 * screen-space sprite FX queue (lens flare + 2D sprite draw).
 *
 * The matcher builds THIS unit at -O2 -G8 (per-unit GFLAG override in
 * tools/ee/objdiff_build.sh / diff.sh / build.sh). Plain -G8, NOT -fno-gcse:
 * for the CURRENTLY-MATCHED set the gcse flag is inert (both settings yield
 * the same 35), so we keep the simpler plain -G8. (An unmatched INCLUDE_ASM
 * here — AddScreenSpriteFx — does hold the g_screenSpriteFxQueue %hi in a
 * register across the function via load-PRE CSE, a hint that the original TU
 * had gcse on; left plain -G8 unless a future match needs the flag.) The
 * 8-byte-packed callee-save wall still applies (e.g. func_0026F850).
 *
 * -G8 extern sizing follows the documented rules (see text/198FA0 /
 * text/1A8180 headers): complete externs <= 8 bytes = true small data
 * (%gp_rel); `.extern sym,16` = cc1-small / assembler-absolute one-insn
 * macro; large/struct externs = ordinary two-insn absolute %hi/%lo.
 *
 * SAVE-LAYOUT WALL: every function below that saves two or more GPRs
 * (incl. $ra) at 8-byte slot spacing is blocked on the 8-byte-packed-save
 * wall and stays INCLUDE_ASM (the pinned cc1 reserves 16 bytes per save).
 *
 * STUB TABLE at 0x26F718: a run of 32 eight-byte stubs (empty `return;` /
 * `return 0;` bodies — debug/profiling hooks compiled out of the retail
 * build). The 0x30-byte blob func_0026F820 and the lone fill words
 * func_0026FC80/func_002701B8/func_00273320/func_002735A8 plus the
 * no-return fragments func_002702C8/func_002725D8 are NOT reachable
 * compiler output (orphaned fill / handwritten table words) and keep their
 * INCLUDE_ASM permanently.
 */

/* Cc1-small / assembler-absolute symbols (one-insn symbolic macro). */
__asm__(".extern g_screenFadeBlack, 16");

/* True small data (complete <=8-byte externs, %gp_rel). */
extern s32 D_1A8630;             /* screen-sprite-FX alpha cap (set by the HUD fade) */

/* Large / ordinary-absolute globals. */
extern s32 g_nGameState[];       /* incomplete-array decl: keeps the 4-byte int
                                  * out of cc1 small data (two-insn absolute
                                  * access like the original) */
extern f32 g_screenFadeBlack;    /* black screen fade level 0..1 */

/* One 0xA0-byte camera slot (fields per the Track-B pass; only the ones this
 * unit touches are declared). */
typedef struct Camera {
    /* 0x00 */ u8 pad0[0x7D];
    /* 0x7D */ u8 unk7D;          /* cleared on activation */
    /* 0x7E */ s16 unk7E;         /* set to 1 on activation */
    /* 0x80 */ u8 pad80[4];
    /* 0x84 */ s16 configIndex;
    /* 0x86 */ s16 type;          /* slot search key (func_00270290) */
    /* 0x88 */ u8 pad88[4];
    /* 0x8C */ s16 modeId;        /* index into g_cameraModeVtbl */
    /* 0x8E */ s16 snapFlag;
    /* 0x90 */ u8 pad90[0x10];
} Camera;

/* Camera mode vtable entry (lvl.camvtbl, 0x14-byte stride; runtime-filled by
 * level overlays). Handlers are declared value-returning so cc1 does not
 * sibling-call-optimise the conditional forwarding calls. */
typedef struct CameraModeVtblEntry {
    /* 0x00 */ s32 id;
    /* 0x04 */ s32 (*takeover)();
    /* 0x08 */ s32 (*enter)();
    /* 0x0C */ s32 (*update)();
    /* 0x10 */ s32 (*poll)();
} CameraModeVtblEntry;

extern Camera g_cameraSlots[48];
extern CameraModeVtblEntry g_cameraModeVtbl[];

/* Camera-system file-scope state block @0x1B5180 (pinned in symbol_addrs so
 * the base+offset accesses symbolize; +0x190 aliases the separately-named
 * g_activeCamera). Only fields this unit touches are declared. */
typedef struct CameraSysState {
    /* 0x000 */ u8 pad0[0x190];
    /* 0x190 */ volatile Camera *volatile activeCamera; /* fully volatile: the
                                                * original re-reads the pointer
                                                * between dereferences and keeps
                                                * the store order interleaved */
    /* 0x194 */ Camera *prevCamera;
    /* 0x198 */ u8 pad198[0xD0];
    /* 0x268 */ f32 fadeBlackRate;    /* per-tick fade step = 1/duration */
    /* 0x26C */ f32 fadeBlackTarget;  /* fade-to level */
} CameraSysState;
extern CameraSysState g_cameraState;

/* Camera transition state @0x1B5410 (+0x02 kind, Vec4 src/cur pos pairs).
 * The 16-byte Vec4 struct assignments compile to the original block-move
 * shape (address regs + offset-0 lq/sq). */
typedef unsigned long u_long128 __attribute__((mode(TI)));
typedef struct Vec4 { f32 x, y, z, w; } __attribute__((aligned(16))) Vec4;
typedef struct CameraTransitionState {
    /* 0x00 */ u8 pad0[2];
    /* 0x02 */ u8 kind;               /* g_bCameraTransitionKind */
    /* 0x03 */ u8 pad3[0x4D];
    /* 0x50 */ Vec4 cur0;
    /* 0x60 */ Vec4 cur1;
    /* 0x70 */ u8 pad70[0x50];
    /* 0xC0 */ Vec4 src0;
    /* 0xD0 */ Vec4 src1;
} CameraTransitionState;
extern CameraTransitionState g_cameraTransitionState;

/* One queued screen-space sprite effect (0x30-byte stride). */
typedef struct ScreenSpriteFx {
    /* 0x00 */ volatile u_long128 worldPos; /* world-space anchor (when hasWorldPos) */
    /* 0x10 */ f32 x;                 /* direct screen x (when no world pos) */
    /* 0x14 */ u32 color;             /* RGBA, alpha capped by D_1A8630 */
    /* 0x18 */ s32 texId;             /* UI texture id */
    /* 0x1C */ f32 y;                 /* direct screen y */
    /* 0x20 */ void *owner;           /* owner moby (draw skipped when dead) */
    /* 0x24 */ s16 hasWorldPos;
    /* 0x26 */ s16 drawFlags;         /* 4 = caller-supplied mode/angle */
    /* 0x28 */ f32 angle;
    /* 0x2C */ s32 mode;              /* 0 burst / 1 mirrored quads / 2 single */
} ScreenSpriteFx;

typedef struct ScreenSpriteFxQueue {
    /* 0x000 */ ScreenSpriteFx entries[6];
    /* 0x120 */ s32 count;
} ScreenSpriteFxQueue;
extern ScreenSpriteFxQueue g_screenSpriteFxQueue;

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", ShowSplashImage);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026EAC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026EB98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", BuildAttractReelPlaylist);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", LoadLevelAndInitHealth);

/* func_0026F718: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F718(void) {
    return 0;
}

/* func_0026F720: stub-table entry — compiled-out hook, no-op. */
void func_0026F720(void) {
}

/* func_0026F728: stub-table entry — compiled-out hook, no-op. */
void func_0026F728(void) {
}

/* func_0026F730: stub-table entry — compiled-out hook, no-op. */
void func_0026F730(void) {
}

/* func_0026F738: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F738(void) {
    return 0;
}

/* func_0026F740: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F740(void) {
    return 0;
}

/* func_0026F748: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F748(void) {
    return 0;
}

/* func_0026F750: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F750(void) {
    return 0;
}

/* func_0026F758: stub-table entry — compiled-out hook, no-op. */
void func_0026F758(void) {
}

/* func_0026F760: stub-table entry — compiled-out hook, no-op. */
void func_0026F760(void) {
}

/* func_0026F768: stub-table entry — compiled-out hook, no-op. */
void func_0026F768(void) {
}

/* func_0026F770: stub-table entry — compiled-out hook, no-op. */
void func_0026F770(void) {
}

/* func_0026F778: stub-table entry — compiled-out hook, no-op. */
void func_0026F778(void) {
}

/* func_0026F780: stub-table entry — compiled-out hook, no-op. */
void func_0026F780(void) {
}

/* func_0026F788: stub-table entry — compiled-out hook, no-op. */
void func_0026F788(void) {
}

/* func_0026F790: stub-table entry — compiled-out hook, no-op. */
void func_0026F790(void) {
}

/* func_0026F798: stub-table entry — compiled-out hook, no-op. */
void func_0026F798(void) {
}

/* func_0026F7A0: stub-table entry — compiled-out hook, no-op. */
void func_0026F7A0(void) {
}

/* func_0026F7A8: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7A8(void) {
    return 0;
}

/* func_0026F7B8: stub-table entry — compiled-out hook, no-op. */
void func_0026F7B8(void) {
}

/* func_0026F7C0: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7C0(void) {
    return 0;
}

/* func_0026F7C8: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7C8(void) {
    return 0;
}

/* func_0026F7D0: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7D0(void) {
    return 0;
}

/* func_0026F7D8: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7D8(void) {
    return 0;
}

/* func_0026F7E0: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7E0(void) {
    return 0;
}

/* func_0026F7E8: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7E8(void) {
    return 0;
}

/* func_0026F7F0: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7F0(void) {
    return 0;
}

/* func_0026F7F8: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7F8(void) {
    return 0;
}

/* func_0026F800: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F800(void) {
    return 0;
}

/* func_0026F808: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F808(void) {
    return 0;
}

/* func_0026F810: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F810(void) {
    return 0;
}

/* func_0026F818: stub-table entry — compiled-out hook, no-op. */
void func_0026F818(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026F820);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026F850);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FC80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FC88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FE58);

/* DebugPrintStub: varargs debug-print hook, compiled to a no-op in retail —
 * only the EABI varargs register spill remains, and crucially the ORIGINAL
 * also spills the FP argument registers $f12/$f14/$f16/$f18 (a printf-style
 * float-varargs prologue). The pinned cc1 emits the GPR varargs spill but
 * never the FP spill for a `(char*, ...)` body, so this cannot match from C.
 * WALL: float-varargs register-spill prologue. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", DebugPrintStub);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FF00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", StepCameraFovInterp);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002701B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002701C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270220);

/* func_00270290: find the first camera slot whose type matches `type`
 * (48-slot linear scan); returns NULL when no slot matches. */
Camera *func_00270290(s32 type) {
    Camera *cam = g_cameraSlots;

    do {
        if (cam->type == type) {
            return cam;
        }
        cam++;
    } while ((s32)cam < (s32)&g_cameraSlots[48]); /* signed compare pins the
                                                   * original slt (sltu with a
                                                   * plain pointer compare) */
    return NULL;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002702C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002702D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002703C0);

/* func_002704E0: flag the active camera slot as freshly (re)activated — sets
 * the activation halfword (+0x7E = 1) and clears the settle byte (+0x7D = 0)
 * on g_cameraState.activeCamera. The original RE-READS the active-camera
 * pointer for the second store (lw v1,400; sh; lw a0,400; sb in the jr delay
 * slot). Cc1 either CSEs the reload (plain field) or, made volatile to force
 * the reload, pins the stores in noreorder brackets so the second store can't
 * fill the jr delay slot — neither reproduces the original interleave.
 * Best 80%. WALL: volatile-reload vs delay-slot scheduling. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002704E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270500);

/* CallCameraEnterHandler: invoke the camera-mode vtbl `enter` handler
 * (slot +0x8) for the camera's mode id, if the level overlay installed one. */
void CallCameraEnterHandler(Camera *cam) {
    s32 (*handler)() = g_cameraModeVtbl[cam->modeId].enter;

    if (handler != NULL) {
        handler(cam);
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", SwitchActiveCamera);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", TestCameraTakeover);

/* CallCameraPollHandler: invoke the camera-mode vtbl `poll` handler
 * (slot +0x10) for the camera's mode id, if the level overlay installed one. */
void CallCameraPollHandler(Camera *cam) {
    s32 (*handler)() = g_cameraModeVtbl[cam->modeId].poll;

    if (handler != NULL) {
        handler(cam);
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", DispatchCameraMode);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270B68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270D60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270E40);

/* func_00270EB8: while a camera transition is pending (kind != 0), snap the
 * transition's current qword pos/target pair (+0x50/+0x60) from the saved
 * source pair (+0xC0/+0xD0). The original loads each 16-byte block through a
 * SEPARATE address register (addiu a0,+0x50; addiu v1,+0xC0; lq 0(v1);
 * sq 0(a0); …) and interleaves the two copies (store of the first before the
 * load of the second). The pinned cc1 always emits offset-form lq/sq off the
 * base register (lq 192(a2); sq 80(a2); …) and orders the copies
 * sequentially — the offset-vs-register addressing is a fixed cc1 choice no
 * source shape overrides. Best 59%. WALL: qword block-copy addressing form. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270EB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", BeginCameraTransition);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00271140);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002712E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", ApplyCameraTransition);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", ApplyCameraShakeAxis);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", TrackHeroMotionForCamera);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", CheckCameraUnderwater);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00271FE8);

/* func_002721A8: start a fade-from-black — fade level forced to 1.0, target 0,
 * rate = 1/duration (g_cameraState +0x268/+0x26C). Instructions all match, but
 * the original schedules the g_cameraState lui/addiu address early and pads
 * the div.s latency with two nops before the divide (the SN-as
 * float-pipeline-latency padding). The pinned cc1 + GNU as emit no such pad.
 * Best 83%. WALL: SN-as div.s latency nop padding. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002721A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", UpdateScreenFadeBlack);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", UpdateScreenFadeWhite);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", UpdateCamera);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00272550);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002725D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", DrawLensFlare);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", DrawScreenSpriteFxEntry);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00272CC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273320);

/* AddScreenSpriteFx(owner, color, worldPos, texId, mode, x, y, angle): queue a
 * screen-space sprite effect (cap 6 per frame; drawn by
 * DrawScreenSpriteFxQueue), skipped in cutscene state (g_nGameState==2).
 * Anchors at *worldPos when given (hasWorldPos=1) else at direct screen x/y;
 * the color alpha is capped by D_1A8630. mode -1 = derive: UI texture ids 0xD
 * and 0x1F..0x27 default to single-quad mode 2 at a fixed pi/2 angle; any
 * other explicit mode stores the caller's mode/angle (drawFlags=4).
 *
 * Best 83%, three independent structural deltas: (1) the original uses the
 * branch-LIKELY form (beqzl) on the worldPos==NULL test, which cc1 only emits
 * for the inverted block layout we can't drive from C; (2) it keeps a CSE
 * copy of the queue base in a second temp (move t4,v0) where cc1 rematerialises
 * the %lo; (3) the pi/2 immediate is the SDK PR_PI/2 macro value 0x3FC90FDC,
 * which the nearest float literal (0x3FC90FDB) misses by 1 ULP. WALL:
 * branch-likely layout + CSE-temp + macro-constant ULP. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", AddScreenSpriteFx);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", DrawScreenSpriteFxQueue);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002735A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", SampleCameraFogZone);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273740);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273988);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273B80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273D20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273EA8);

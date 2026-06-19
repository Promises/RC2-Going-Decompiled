#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"
#include "vec.h"

/* Going Commando camera system (16E980 unit, .text ~0x26FF00-0x2735xx).
 *
 * Recovered from the USA v2.00 (SCUS_972.68) camera cluster and cross-checked
 * against the matched source going-decompiled/src/usa/text/16E980.c and the
 * absolute symbol_addrs camera globals. Region-agnostic: the EU v1.00 twins
 * share this layout (modulo the usual small .text/.data offset).
 *
 * THREE distinct things, do not conflate them:
 *
 *  1. Camera OBJECT (this struct `Camera`, 0xA0 bytes). A per-camera slot. The
 *     engine keeps a bank of 48 of them (g_cameraSlots, 0xA0 stride - confirmed
 *     by the 48-iteration scan in DispatchCameraMode and func_00270290's
 *     `(s32)cam < (s32)&g_cameraSlots[48]` bound). Holds the camera's transform
 *     (matrix rows +0x00, position +0x30), its arbitration rule/priority/type,
 *     and its mode id (index into the mode vtable).
 *
 *  2. Camera MODE VTABLE (`CameraModeVtblEntry`, 0x14 stride). A runtime-filled
 *     table of mode handlers; the level overlay installs the bodies. A Camera's
 *     modeId (+0x8C) indexes it. Slots: id(+0x0), takeover(+0x4), enter(+0x8),
 *     update(+0xC), poll(+0x10). TestCameraTakeover calls the takeover test,
 *     DispatchCameraMode calls poll then update, SwitchActiveCamera/Call*Handler
 *     call enter. CONFIRMED from the matched Call*Handler bodies + the
 *     `base + modeId*0x14` call in TestCameraTakeover.
 *
 *  3. Camera SYSTEM STATE (`CameraSysState`, the manager block at g_cameraState
 *     = 0x1B5180). One large file-scope block holding the live composited camera
 *     transform, the active/prev camera pointers, the hero-motion tracking
 *     sub-block, the screen-fade state, the FOV interpolator, the transition
 *     interpolator, and the underwater flag. All field offsets below are derived
 *     from the ABSOLUTE addresses of the named camera globals in
 *     symbol_addrs/usa minus the 0x1B5180 base, so they are CONFIRMED at the
 *     offset level even where the semantic is only PARTIAL.
 *
 * IN-LEVEL CAVEAT (see tools/native/camera_map_re.md): in the disc-loaded level
 * overlay, the live camera OBJECT that `activeCamera` (+0x190) points at is
 * overlay-resident - +0x190 reads an unmapped address from the boot ELF's view.
 * The CameraSysState block itself, and especially the HeroCamMotion sub-block
 * (+0x1A0), ARE at these static offsets in-level (validated against a live
 * capture). So this header is correct as the frontend/static contract; the
 * tester must supply a valid Camera object for any function that derefs
 * activeCamera.
 *
 * Gaps below marked `padN` are genuinely un-analyzed bytes, NOT asserted
 * semantics - the struct sizes/offsets are real, the holes are just unmapped.
 */

struct Camera; /* forward decl - vtable handlers take Camera* */

/* ------------------------------------------------------------------------- *
 *  Camera mode vtable entry (lvl.camvtbl) - 0x14 stride, runtime-filled.    *
 * ------------------------------------------------------------------------- */
typedef struct CameraModeVtblEntry {
    /* 0x00 */ s32 id;            /* mode id (self) */
    /* 0x04 */ s32 (*takeover)(struct Camera *candidate, struct Camera *current);
                                  /* return -1 forbid takeover, 1 force, 0 defer
                                     to the priority rules - CONFIRMED (TestCameraTakeover) */
    /* 0x08 */ s32 (*enter)(struct Camera *cam);  /* CONFIRMED (CallCameraEnterHandler) */
    /* 0x0C */ s32 (*update)(struct Camera *cam); /* CONFIRMED (DispatchCameraMode) */
    /* 0x10 */ s32 (*poll)(struct Camera *cam);   /* CONFIRMED (CallCameraPollHandler) */
} CameraModeVtblEntry;

/* ------------------------------------------------------------------------- *
 *  CameraShakeChannel - one shake axis. 0x10 stride. (g_cameraShake*)       *
 *  Consumed by ApplyCameraShakeAxis, seeded by impact/effect triggers.      *
 * ------------------------------------------------------------------------- */
typedef struct CameraShakeChannel {
    /* 0x00 */ f32 amplitude;    /* CONFIRMED */
    /* 0x04 */ f32 outputOffset; /* CONFIRMED - applied offset/roll this frame */
    /* 0x08 */ f32 timer;        /* CONFIRMED - seeded to duration, decremented */
    /* 0x0C */ f32 duration;     /* CONFIRMED - captured frame 1 for sin window */
} CameraShakeChannel;

/* ------------------------------------------------------------------------- *
 *  Camera OBJECT - one camera slot. sizeof == 0xA0.                          *
 * ------------------------------------------------------------------------- */
typedef struct Camera {
    /* 0x00 */ Vec4 mtxRow0;     /* PROBABLE - 3x4 orientation matrix rows. The
                                    matched SwitchActiveCamera copies words
                                    +0x00..+0x2F as three vec4s. */
    /* 0x10 */ Vec4 mtxRow1;     /* PROBABLE */
    /* 0x20 */ Vec4 mtxRow2;     /* PROBABLE */
    /* 0x30 */ Vec4 pos;         /* CONFIRMED - camera world position. Copied into
                                    g_cameraPos / cs+0x140 on switch; saved into
                                    prevPos (+0x64) after the mode update. */
    /* 0x40 */ u8 pad40[0x24];   /* UNCONFIRMED gap */
    /* 0x64 */ f32 prevPos[3];   /* CONFIRMED - position before this frame's mode
                                    update; written +0x64/+0x68/+0x6C by
                                    DispatchCameraMode and SwitchActiveCamera as
                                    three separate words (NOT a 16-aligned qword,
                                    so modeled as a plain vec3, not Vec4). */
    /* 0x70 */ u8 pad70[0x4];    /* UNCONFIRMED gap */
    /* 0x74 */ s32 rule;         /* CONFIRMED - takeover rule kind (switch in
                                    TestCameraTakeover): 0 always, 1/2 trigger-flag
                                    gated, 4 hero-pos sphere vs config table, 7
                                    zone-type match. */
    /* 0x78 */ f32 transitionDuration; /* PROBABLE - read as puVar8[0x1e] in
                                    SwitchActiveCamera to seed transition timing
                                    (clamped to default 0.018). */
    /* 0x7C */ u8 priority;      /* CONFIRMED - priority byte; a candidate must
                                    out-rank the current camera's +0x7C to win
                                    (compared in TestCameraTakeover). */
    /* 0x7D */ u8 triggerFlag;   /* CONFIRMED - rule 1/2 gate; also cleared on
                                    activation, set to 2 by SwitchActiveCamera's
                                    matrix-blend path. */
    /* 0x7E */ s16 forceActive;  /* CONFIRMED - when nonzero forces takeover
                                    regardless of priority; set to 1 on activation,
                                    cleared on switch. */
    /* 0x80 */ u8 pad80[0x4];    /* UNCONFIRMED gap */
    /* 0x84 */ s16 configIndex;  /* CONFIRMED - index (*0x20) into the camera
                                    config table at DAT_001b1504+0x1c; used by
                                    rule 4/7 and func_00270290 search. */
    /* 0x86 */ s16 type;         /* CONFIRMED - zone/type id; rule-7 slots take
                                    over only when this == g_cameraZoneType.
                                    Also the search key in func_00270290. */
    /* 0x88 */ u8 pad88[0x4];    /* UNCONFIRMED gap */
    /* 0x8C */ s16 modeId;       /* CONFIRMED - index into g_cameraModeVtbl
                                    (entry = base + modeId*0x14). */
    /* 0x8E */ s16 snapFlag;     /* CONFIRMED - set to 1 by SwitchActiveCamera on
                                    the snap (no-transition) path, cleared otherwise. */
    /* 0x90 */ u8 pad90[0x10];   /* UNCONFIRMED tail to 0xA0 */
} Camera;

extern Camera g_cameraSlots[48];
extern CameraModeVtblEntry g_cameraModeVtbl[];

/* ------------------------------------------------------------------------- *
 *  CameraSysState - the camera manager. Base g_cameraState = 0x1B5180.       *
 *  Offsets = (absolute symbol_addrs address) - 0x1B5180.                     *
 * ------------------------------------------------------------------------- */
typedef struct CameraSysState {
    /* 0x000 */ u8 pad0[0x140];   /* CONFIRMED gap - sentinel-init header
                                     (0x7FFF0000-every-other-word pattern). */
    /* 0x140 */ Vec4 camPos;      /* CONFIRMED (g_cameraPos 0x1B52C0) - live
                                     composited camera world position. */
    /* 0x150 */ Vec4 camRot;      /* CONFIRMED (g_cameraRot 0x1B52D0) - euler
                                     angles from MatrixToEulerAngles(camMatrix). */
    /* 0x160 */ CameraShakeChannel shakeUp;    /* CONFIRMED (g_cameraShakeUp 0x1B52E0) */
    /* 0x170 */ CameraShakeChannel shakeRight; /* CONFIRMED (g_cameraShakeRight 0x1B52F0) */
    /* 0x180 */ CameraShakeChannel shakeRoll;  /* CONFIRMED (g_cameraShakeRoll 0x1B5300) */
    /* 0x190 */ volatile Camera *volatile activeCamera; /* CONFIRMED (g_activeCamera
                                     0x1B5310) - active 0xA0 camera slot. Fully
                                     volatile in the matched source (the engine
                                     re-reads it between dereferences). Overlay-
                                     resident in-level (see header caveat). */
    /* 0x194 */ Camera *prevCamera;/* CONFIRMED (g_prevCamera 0x1B5314) */
    /* 0x198 */ u8 pad198[0x8];   /* UNCONFIRMED gap */
    /* 0x1A0 */ u8 heroCamMotion[0xC0]; /* CONFIRMED sub-block (HeroCamMotion,
                                     0x1B5320) written every frame by
                                     TrackHeroMotionForCamera; the one in-level-
                                     live part. Modeled as raw bytes here - see
                                     the HeroCamMotion field table in
                                     tools/native/camera_map_re.md (smoothX/Y/
                                     Height at +0x1A0, facingDir Vec4 +0x1C0,
                                     velocity Vec4 +0x210, lateralDir +0x220,
                                     forwardProj +0x230, speeds +0x240/4/8,
                                     orientZRing[5] +0x24C). Runs to ~+0x260. */
    /* 0x260 */ u8 pad260[0x4];   /* UNCONFIRMED gap */
    /* 0x264 */ void *helperMoby; /* CONFIRMED (g_cameraHelperMoby 0x1B53E4) -
                                     lazily-spawned camera helper moby ptr. */
    /* 0x268 */ u8 pad268[0x28];  /* UNCONFIRMED gap */
    /* 0x290 */ u8 transitionState;/* CONFIRMED (g_cameraTransitionState 0x1B5410)
                                     - 0 none, 1/2 pending after switch, 3
                                     interpolating. */
    /* 0x291 */ u8 pad291;        /* UNCONFIRMED */
    /* 0x292 */ u8 transitionKind;/* PARTIAL (g_cameraTransitionKind 0x1B5412) -
                                     0 quat/pos blend, else matrix blend. */
    /* 0x293 */ u8 pad293[0x141]; /* UNCONFIRMED gap (spans the transition src/cur
                                     Vec4 pairs documented in 16E980.c's
                                     CameraTransitionState: cur0/cur1 at +0x2E0/
                                     +0x2F0, src0/src1 at +0x350/+0x360 relative to
                                     this base). */
    /* 0x3D4 */ f32 fovBase;      /* CONFIRMED (g_cameraFovBase 0x1B5554) - FOV
                                     interp start/base angle (radians). */
    /* 0x3D8 */ f32 fov;          /* CONFIRMED (g_cameraFov 0x1B5558) - live FOV
                                     angle; StepCameraFovInterp eases this. */
    /* 0x3DC */ u8 pad3DC[0xC];   /* UNCONFIRMED gap (proj scale / fov interp t) */
    /* 0x3E8 */ u8 fovInterpMode; /* CONFIRMED (g_cameraFovInterpMode 0x1B5568) -
                                     0 snap, 1 sin-eased, 2 angle-lerp, 3 spring. */
    /* 0x3E9 */ u8 fovInterpActive;/* CONFIRMED (g_cameraFovInterpActive 0x1B5569)
                                     - StepCameraFovInterp runs only while 1. */
    /* 0x3EA */ u8 pad3EA[0x6];   /* UNCONFIRMED gap */
    /* 0x3F0 */ f32 fovTarget;    /* CONFIRMED (g_cameraFovTarget 0x1B5570) - FOV
                                     interp destination angle (radians). */
    /* 0x3F4 */ u8 pad3F4[0xC];   /* UNCONFIRMED gap */
    /* 0x400 */ s32 underwater;   /* CONFIRMED (g_bCameraUnderwater 0x1B5580) - 1
                                     when camera below water; set by
                                     CheckCameraUnderwater. */
    /* 0x404 */ s32 framesSinceCut;/* CONFIRMED (g_cameraFramesSinceCut 0x1B5584) -
                                     incremented in UpdateCamera, zeroed on switch. */
    /* 0x408 */ u8 pad408[0x148]; /* UNCONFIRMED gap up to the slot bank */
    /* 0x550 */ Camera slots[48]; /* CONFIRMED (g_cameraSlots 0x1B56D0) - the 48
                                     0xA0-byte camera slots are the tail of this
                                     block (0x1B56D0 - 0x1B5180 = 0x550). */
} CameraSysState;

extern CameraSysState g_cameraState;

/* The camera matrix (g_cameraMatrix 0x1B54F0 = cs+0x370) overlaps the transition
 * sub-block region above; it is the live 3x4 rotation matrix (3 Vec4 rows) used
 * by ApplyCameraShakeAxis and MatrixToEulerAngles. Left inside the un-analyzed
 * transition gap rather than asserted, since its exact relation to the
 * transition state bytes is not yet traced. */

/* --- offset checks for the CONFIRMED fields. CameraSysState/Camera carry
 * pointers, so the layout only matches the EE target where pointers are 4 bytes
 * (the EE build and the native -m32 build). Inert on a 64-bit host compile. --- */
#if defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 4)
_Static_assert(sizeof(Camera) == 0xA0, "Camera must be 0xA0");
_Static_assert(__builtin_offsetof(Camera, pos)         == 0x30, "Camera.pos");
_Static_assert(__builtin_offsetof(Camera, prevPos)     == 0x64, "Camera.prevPos");
_Static_assert(__builtin_offsetof(Camera, rule)        == 0x74, "Camera.rule");
_Static_assert(__builtin_offsetof(Camera, priority)    == 0x7C, "Camera.priority");
_Static_assert(__builtin_offsetof(Camera, triggerFlag) == 0x7D, "Camera.triggerFlag");
_Static_assert(__builtin_offsetof(Camera, forceActive) == 0x7E, "Camera.forceActive");
_Static_assert(__builtin_offsetof(Camera, configIndex) == 0x84, "Camera.configIndex");
_Static_assert(__builtin_offsetof(Camera, type)        == 0x86, "Camera.type");
_Static_assert(__builtin_offsetof(Camera, modeId)      == 0x8C, "Camera.modeId");
_Static_assert(__builtin_offsetof(Camera, snapFlag)    == 0x8E, "Camera.snapFlag");

_Static_assert(sizeof(CameraModeVtblEntry) == 0x14, "vtbl entry stride");
_Static_assert(__builtin_offsetof(CameraModeVtblEntry, takeover) == 0x04, "vtbl.takeover");
_Static_assert(__builtin_offsetof(CameraModeVtblEntry, enter)    == 0x08, "vtbl.enter");
_Static_assert(__builtin_offsetof(CameraModeVtblEntry, update)   == 0x0C, "vtbl.update");
_Static_assert(__builtin_offsetof(CameraModeVtblEntry, poll)     == 0x10, "vtbl.poll");

_Static_assert(__builtin_offsetof(CameraSysState, camPos)         == 0x140, "cs.camPos");
_Static_assert(__builtin_offsetof(CameraSysState, camRot)         == 0x150, "cs.camRot");
_Static_assert(__builtin_offsetof(CameraSysState, shakeUp)        == 0x160, "cs.shakeUp");
_Static_assert(__builtin_offsetof(CameraSysState, shakeRight)     == 0x170, "cs.shakeRight");
_Static_assert(__builtin_offsetof(CameraSysState, shakeRoll)      == 0x180, "cs.shakeRoll");
_Static_assert(__builtin_offsetof(CameraSysState, activeCamera)   == 0x190, "cs.activeCamera");
_Static_assert(__builtin_offsetof(CameraSysState, prevCamera)     == 0x194, "cs.prevCamera");
_Static_assert(__builtin_offsetof(CameraSysState, heroCamMotion)  == 0x1A0, "cs.heroCamMotion");
_Static_assert(__builtin_offsetof(CameraSysState, helperMoby)     == 0x264, "cs.helperMoby");
_Static_assert(__builtin_offsetof(CameraSysState, transitionState)== 0x290, "cs.transitionState");
_Static_assert(__builtin_offsetof(CameraSysState, transitionKind) == 0x292, "cs.transitionKind");
_Static_assert(__builtin_offsetof(CameraSysState, fovBase)        == 0x3D4, "cs.fovBase");
_Static_assert(__builtin_offsetof(CameraSysState, fov)            == 0x3D8, "cs.fov");
_Static_assert(__builtin_offsetof(CameraSysState, fovInterpMode)  == 0x3E8, "cs.fovInterpMode");
_Static_assert(__builtin_offsetof(CameraSysState, fovInterpActive)== 0x3E9, "cs.fovInterpActive");
_Static_assert(__builtin_offsetof(CameraSysState, fovTarget)      == 0x3F0, "cs.fovTarget");
_Static_assert(__builtin_offsetof(CameraSysState, underwater)     == 0x400, "cs.underwater");
_Static_assert(__builtin_offsetof(CameraSysState, framesSinceCut) == 0x404, "cs.framesSinceCut");
_Static_assert(__builtin_offsetof(CameraSysState, slots)          == 0x550, "cs.slots");
#endif

#endif /* CAMERA_H */

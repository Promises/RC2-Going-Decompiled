#ifndef MOBY_MOTION_H
#define MOBY_MOTION_H

#include "common.h"

/* Moby PHYSICS / MOTION subsystem - how mobys (enemies, NPCs, props) locomote
 * each frame: heading steer, accel/arrive, wall-slide collision, ground snap,
 * footstep/material events, lean, and waypoint following.
 *
 * Entry point is StepMobyMotion (0x2B6000), driven once per moby per frame by
 * overlay behaviour code via the thin wrappers DriveMobyAlongFacing (steer to
 * own facing), DriveMobyTowardPoint (steer to a world point + optional wander),
 * DriveMobyAlongWaypoints (follow a path). The per-frame pipeline inside
 * StepMobyMotion is:
 *     CheckMobyGroundMover       (riding a moving platform?)
 *     ProbeMobyGroundLine        (re-seed ground on first frame / after a gap)
 *     UpdateMobyMotionVelocity   (steer+accel toward target, vertical step)
 *     ResolveMobyMotionCollision (CollSphere wall-slide, up to 8 passes;
 *                                 ResolveMobySphereCollision + ResolveMobyEdgeConstraint)
 *     ApplyMobyGroundAndEvents   (ground snap, grounded/airborne counters,
 *                                 footstep/turn anim+material sound events)
 *     UpdateMobyLeanFromTurn     (bank attached sub-mobys from turn rate)
 *     AccumMobyMotionProfile     (RCNT0 timing accumulators)
 *
 * The controller block lives at *(moby->pVars)+0x18 (GetMobyMotionController,
 * 0x2B6C48; moby+0x68 is the 0x80-byte aux/vars block). It is the per-moby
 * physics state record; the field map below is recovered from the readers and
 * writers listed above (each field corroborated at >=1 named call site; see the
 * per-field tags). Position/heading themselves live on the Moby record
 * (pos +0x10, heading-yaw +0xF8), NOT here - this block holds velocity, the
 * accel/arrive profile, the recovered ground state, and the event outputs.
 *
 * LAYOUT IS ILP32 (4-byte pointers) - matches the PS2 EE target and native -m32.
 * The waypoint path ptr (+0xC4) is the only pointer field; the guarded
 * _Static_assert block at the bottom is inert under a 64-bit-pointer compile.
 *
 * Offsets are CONFIRMED (driven from exact store/load offsets). Semantics are
 * tagged: CONFIRMED (>=2 sites or an unambiguous single use), PARTIAL
 * (single-site/inferred), UNCONFIRMED (guess). Unanalyzed bytes are left as gap
 * arrays - NOT asserted padding.
 */

typedef struct MobyMotionController {
    f32  collRadiusBase;   /* +0x00 collision-sphere radius base term; CollSphere uses
                              (+0x00 + +0x04) as the test radius                    PARTIAL  */
    f32  collRadiusStep;   /* +0x04 collision-sphere radius per-step add (see +0x00) PARTIAL  */
    f32  groundClearUp;    /* +0x08 up clearance: probe start = pos.z + this + 0.1; also the
                              "below ground" event-2 margin (pos.z < ground - this)  CONFIRMED */
    f32  groundClearDown;  /* +0x0C down probe depth: probe end = pos.z - this - 0.1; also the
                              "airborne" event-1 margin (ground + this < pos.z)      CONFIRMED */
    f32  maxSlopeAngle;    /* +0x10 max standable ground-slope angle (rad); atan2 of the ground
                              normal vs this raises the slip/slide event flag 4      PARTIAL  */
    f32  velX;             /* +0x14 commanded planar velocity X (SetMobyMotionParams writes
                              speed here and at +0x18; UpdateMobyMotionVelocity drive term) PARTIAL */
    f32  velY;             /* +0x18 commanded planar velocity Y / speed (see +0x14)   PARTIAL  */
    f32  velZ;             /* +0x1C commanded velocity Z (SetMobyMotionParams param4)  PARTIAL  */
    f32  accel;            /* +0x20 acceleration (SetMobyMotionParams param1, mirror of +0x24) CONFIRMED */
    f32  accelArrive;      /* +0x24 accel used in the arrive sqrt and as decel basis (param1) CONFIRMED */
    f32  maxSpeed;         /* +0x28 max planar speed / max step length (SetMobyMotionParams param2,
                              clamps the arrive sqrt and the half-speed contact test) CONFIRMED */
    f32  decel;            /* +0x2C per-frame vertical/step decel subtracted from +0xA8  PARTIAL  */
    f32  arriveAngle;      /* +0x30 heading dead-band (rad): AngleShortestDiff <= this => no steer CONFIRMED */
    f32  arriveRadius;     /* +0x34 arrive distance: DistXY(pos,target) <= this => stop + raise
                              the "arrived" event flag 0x80                          CONFIRMED */
    u32  modeFlags;        /* +0x38 motion mode bits - see MOBY_MOTION_MODE_* below   CONFIRMED */
    s32  splineConstraintId;/* +0x3C active spline/rail constraint index (-1 = none); indexes the
                              global spline table at 0x1F1680 (same table the ship-takeoff
                              flight uses) and is projected onto via FUN_002CA138. Used by
                              ResolveMobyEdgeConstraint + CheckMobyPathBlocked to keep the moby
                              riding a rail/spline                                      CONFIRMED */
    u8   gap40[8];         /* +0x40 un-analyzed                                       UNCONFIRMED */
    void *leanTargetA;     /* +0x48 attached sub-moby banked by UpdateMobyLeanFromTurn PARTIAL  */
    void *leanTargetB;     /* +0x4C second attached sub-moby to bank                  PARTIAL  */
    f32  groundNormal[4];  /* +0x50 ground-surface unit normal (ProbeMobyGroundLine writes;
                              +0x58 = normal.z, used in slope atan2 tests)            CONFIRMED */
    f32  wallContact[4];   /* +0x60 last wall-contact point (ResolveMobySphereCollision writes
                              to +0x60/+0x64/+0x68/+0x6C; zeroed each frame)          CONFIRMED */
    f32  posDelta[4];      /* +0x70 net position change this frame (pos - prevPos), written at
                              the tail of ResolveMobyMotionCollision                  PARTIAL  */
    s32  wallContactCount; /* +0x80 number of wall-contact resolution hits this frame; >0 means
                              "pinned against geometry" (consumed by FUN_002832F8/...)  CONFIRMED */
    void *wallHitMoby;     /* +0x84 moby hit by the wall sphere test (g_pCollHitMoby)  PARTIAL  */
    void *groundHitMoby;   /* +0x88 moby under the ground probe (g_pCollHitMoby); drives the
                              moving-platform check in CheckMobyGroundMover            CONFIRMED */
    void *groundHitPoly;   /* +0x8C ground hit poly-info (g_collHitPolyInfo)           PARTIAL  */
    f32  groundHeight;     /* +0x90 world Z of the ground under the moby (DAT_001baf28 hit z);
                              snap target in ApplyMobyGroundAndEvents                  CONFIRMED */
    u32  eventFlags;       /* +0x94 per-step OUTPUT event bitfield - see MOBY_MOTION_EVENT_*;
                              StepMobyMotion returns this                              CONFIRMED */
    u32  eventFlagsPrev;   /* +0x98 previous frame's eventFlags (copied at frame start) PARTIAL  */
    u32  stateFlags;       /* +0x9C internal state bits: bit1 steer-blocked, bit0 path-active  PARTIAL */
    f32  velAccum[2];      /* +0xA0 integrated planar (XY) velocity accumulator (Vec4Add'd each
                              frame; reset on a gap; added into pos by the collision pass).
                              NOTE the full +0xA0 store is a vec4: +0xA8 is its Z = verticalVel
                              below, +0xAC its W.                                      CONFIRMED */
    f32  verticalVel;      /* +0xA8 vertical velocity / step-up amount (Z of the +0xA0 vec);
                              gravity-like decel by +0x2C, clamped vs ground in
                              ApplyMobyGroundAndEvents                                 CONFIRMED */
    f32  velAccumW;        /* +0xAC W lane of the +0xA0 accumulator vec4               UNCONFIRMED */
    f32  speed;            /* +0xB0 current planar speed (length of +0xA0); also anim-speed input CONFIRMED */
    f32  turnDelta;        /* +0xB4 this-frame yaw turn delta (rad); sign picks turn-anim, and
                              magnitude*30 gates the big-turn event flag 2            CONFIRMED */
    u8   gapB8[4];         /* +0xB8 un-analyzed                                       UNCONFIRMED */
    s32  lastUpdateTime;   /* +0xBC last g_gameTime this controller stepped; the once-per-frame
                              guard, and a gap here forces a ground re-probe + reset  CONFIRMED */
    s16  groundedFrames;   /* +0xC0 consecutive frames on the ground                  CONFIRMED */
    s16  airborneFrames;   /* +0xC2 consecutive frames off the ground                 CONFIRMED */
    short *waypointPath;   /* +0xC4 waypoint path base: [0]=count(short), then vec4 nodes stride
                              0x10 starting at +0x10 (SetMobyWaypointPath)            CONFIRMED */
    s16  waypointCursor;   /* +0xC8 current waypoint index being advanced toward       CONFIRMED */
    s16  waypointEnd;      /* +0xCA destination waypoint index (path end / direction)  CONFIRMED */
    f32  distToTarget;     /* +0xCC cached DistXY(pos,target) from the last step        CONFIRMED */
    f32  animSpeedRef;     /* +0xD0 reference speed for the anim-rate map (speed/this -> moby+0x48);
                              0 falls back to maxSpeed (+0x28)                          PARTIAL  */
    f32  animSpeedClamp;   /* +0xD4 clamp ratio for the anim-rate map (moby+0x48 in [1/this,this]) PARTIAL */
    s32  groundMaterial;   /* +0xD8 ground hit material id (GetCollHitMaterial); 0xFFFFFFFF on miss PARTIAL */
    f32  wanderAmplitude;  /* +0xDC wander/zig-zag amplitude (sign-flipped on timer expiry)  PARTIAL */
    s16  wanderTimer;      /* +0xE0 frames until the next wander direction flip         PARTIAL  */
    s16  wanderIntervalMin;/* +0xE2 RandRange min for the wander timer reseed           PARTIAL  */
    s16  wanderIntervalMax;/* +0xE4 RandRange max for the wander timer reseed           PARTIAL  */
    u8   animSeqMoveA;     /* +0xE6 move/turn anim-seq id A (e.g. turn-left)            PARTIAL  */
    u8   animSeqMoveB;     /* +0xE7 move/turn anim-seq id B (e.g. turn-right)           PARTIAL  */
    u8   animSeqIdleA;     /* +0xE8 idle/stop anim-seq id A (matched against moby+0x43)  PARTIAL  */
    u8   animSeqIdleB;     /* +0xE9 idle/stop anim-seq id B                             PARTIAL  */
    u8   animSeqActive;    /* +0xEA currently-driven move anim-seq id (0xFF = none)      PARTIAL  */
} MobyMotionController;

/* === +0x38 motion mode flag bits (behaviour selectors set by the owning class) === */
#define MOBY_MOTION_MODE_FREE_YAW    0x0010 /* skip heading-drive + lean; integrate raw velocity
                                               (shared bit with moby +0x34 NO_VELOCITY)        */
#define MOBY_MOTION_MODE_NO_VERTICAL 0x0004 /* skip the vertical-step / gravity block          */
#define MOBY_MOTION_MODE_NO_GROUNDMOVER 0x0100 /* set => skip CheckMobyGroundMover (no moving-
                                               platform inheritance) (PARTIAL)               */
#define MOBY_MOTION_MODE_GROUND_ANIM 0x0040 /* drive footstep/material anim-seq events         */
#define MOBY_MOTION_MODE_TURN_ANIM   0x0080 /* drive turn anim-seq + anim-rate from speed      */
#define MOBY_MOTION_MODE_WATER_FX    0x0200 /* spawn a water-surface splash when over water    */

/* === +0x94 per-step event OUTPUT flag bits (StepMobyMotion return value) === */
#define MOBY_MOTION_EVENT_AIRBORNE   0x0001 /* moby rose above ground+clearance this step      */
#define MOBY_MOTION_EVENT_BLOCKED    0x0002 /* below-ground / big-turn / steer-blocked          */
#define MOBY_MOTION_EVENT_SLOPE      0x0004 /* ground slope exceeded maxSlopeAngle (+0x10)      */
#define MOBY_MOTION_EVENT_EDGE       0x0010 /* clamped onto the edge/rail constraint (+0x3C)    */
#define MOBY_MOTION_EVENT_FELL_STUCK 0x0020 /* fell while groundedFrames==0 (stepped off)       */
#define MOBY_MOTION_EVENT_ARRIVED    0x0080 /* within arriveRadius (+0x34) of the target        */
#define MOBY_MOTION_EVENT_PATH_END   0x0100 /* path/edge constraint blocked ahead               */

/* === Module globals (see symbol_addrs/usa for the authoritative notes) ====== */
/* g_mobyMaxTurnRate        0x1AA108  per-step yaw clamp (radians)                 */
/* g_mobyMotionProfileTicks 0x1AA10C  accumulated RCNT0 ticks across motion steps  */
/* g_mobyMotionProfileCount 0x1AA114  count of measured motion steps               */

/* === Compile-time layout verification (ILP32 targets only) =================
 * Active only under a 4-byte-pointer compile (PS2 EE + native -m32). ee-gcc 2.9
 * predates __SIZEOF_POINTER__ so this is inert in the matching build; the host-64
 * linter (ptr==8) is skipped too. The native -m32 build is the verification
 * target. Only the load-bearing offsets are asserted (the ones a reader/writer
 * pins exactly); gap arrays are not. */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(__builtin_offsetof(MobyMotionController, groundClearUp)   == 0x08, "groundClearUp");
_Static_assert(__builtin_offsetof(MobyMotionController, groundClearDown) == 0x0C, "groundClearDown");
_Static_assert(__builtin_offsetof(MobyMotionController, velX)            == 0x14, "velX");
_Static_assert(__builtin_offsetof(MobyMotionController, accel)           == 0x20, "accel");
_Static_assert(__builtin_offsetof(MobyMotionController, maxSpeed)        == 0x28, "maxSpeed");
_Static_assert(__builtin_offsetof(MobyMotionController, arriveAngle)     == 0x30, "arriveAngle");
_Static_assert(__builtin_offsetof(MobyMotionController, arriveRadius)    == 0x34, "arriveRadius");
_Static_assert(__builtin_offsetof(MobyMotionController, modeFlags)       == 0x38, "modeFlags");
_Static_assert(__builtin_offsetof(MobyMotionController, leanTargetA)     == 0x48, "leanTargetA");
_Static_assert(__builtin_offsetof(MobyMotionController, groundNormal)    == 0x50, "groundNormal");
_Static_assert(__builtin_offsetof(MobyMotionController, wallContact)     == 0x60, "wallContact");
_Static_assert(__builtin_offsetof(MobyMotionController, posDelta)        == 0x70, "posDelta");
_Static_assert(__builtin_offsetof(MobyMotionController, wallContactCount)== 0x80, "wallContactCount");
_Static_assert(__builtin_offsetof(MobyMotionController, groundHitMoby)   == 0x88, "groundHitMoby");
_Static_assert(__builtin_offsetof(MobyMotionController, groundHeight)    == 0x90, "groundHeight");
_Static_assert(__builtin_offsetof(MobyMotionController, eventFlags)      == 0x94, "eventFlags");
_Static_assert(__builtin_offsetof(MobyMotionController, velAccum)        == 0xA0, "velAccum");
_Static_assert(__builtin_offsetof(MobyMotionController, verticalVel)     == 0xA8, "verticalVel");
_Static_assert(__builtin_offsetof(MobyMotionController, speed)           == 0xB0, "speed");
_Static_assert(__builtin_offsetof(MobyMotionController, turnDelta)       == 0xB4, "turnDelta");
_Static_assert(__builtin_offsetof(MobyMotionController, lastUpdateTime)  == 0xBC, "lastUpdateTime");
_Static_assert(__builtin_offsetof(MobyMotionController, groundedFrames)  == 0xC0, "groundedFrames");
_Static_assert(__builtin_offsetof(MobyMotionController, waypointPath)    == 0xC4, "waypointPath");
_Static_assert(__builtin_offsetof(MobyMotionController, distToTarget)    == 0xCC, "distToTarget");
#endif

#endif /* MOBY_MOTION_H */

#ifndef MOBY_H
#define MOBY_H

#include "common.h"

/* Moby (MOBile object / game actor) core system - render layer 0x10.
 *
 * "Moby" is Insomniac's term for every dynamic game object: Ratchet, Clank,
 * enemies, pickups, props, projectiles, the HUD elements, even the cinematic
 * cast. They all share this one 0x100-byte record and the one pool walk.
 *
 * LAYOUT IS ILP32 (4-byte pointers): sizeof == 0x100 only under a 4-byte-pointer
 * compile - the PS2 EE target and the native -m32 build (see common.h). On a
 * 64-bit host the pointer fields widen the record; that is expected, not a bug.
 * The guarded _Static_assert block at the bottom verifies every key offset under
 * ILP32 and is inert elsewhere.
 *
 * Recovered from the USA v2.00 (SCUS_972.68) moby cluster. The layout below is
 * cross-verified against multiple independent readers/writers, primarily:
 *   InitMobyFromClass   0x29FF00 (zeroes 0x100 then stamps every default)
 *   SpawnMoby           0x29FE08 (pool free-scan + aux block attach)
 *   FreeMoby            0x2A0170 (state + grid removal)
 *   BuildActiveMobyChain 0x2A16D8 / UpdateActiveMobys 0x2B7578 (per-frame walk)
 *   UpdateMobyBSphereAndGrid 0x2A1D80 (bsphere +0x00, matrix +0xC0..)
 *   BindMobyToParent    0x2B4700 (pos +0x10, facing +0xF0, color +0x38, parent +0xB8)
 *   StepMobyMotion / UpdateMobyMotionVelocity (pos +0x10, heading +0xF8)
 *   UpdateMobyAnimation 0x2A13E0 (anim block +0x40..)
 *
 * Field semantics marked CONFIRMED are corroborated at >=2 sites; PARTIAL ones
 * are single-site or inferred; UNCONFIRMED are guesses left for the next pass.
 * Offsets/sizes themselves are all CONFIRMED (driven from the 0x100 memset and
 * the exact store offsets). Gaps below are genuinely un-analyzed bytes, NOT
 * asserted padding.
 *
 * The per-class behaviour (the pUpdate at +0x64) lives in the level overlay
 * (0x01800000 segment), NOT in this ELF - this struct is the contract between
 * the engine pool walk and that overlay code.
 */

typedef struct Moby {                       /* === MOBY ENTITY RECORD, sizeof == 0x100 === */
    f32 bsphere[4];      /* +0x00 bounding sphere center.xyz + radius (world*1024 fixed)  CONFIRMED
                            (written every frame by UpdateMobyBSphereAndGrid)             */
    f32 bsphereScratch[4];/* +0x10 - NOTE this 16 bytes is ALSO the world position (see below).
                            The bsphere builder reads pos here as its center source.      */
    /* +0x10 is the WORLD POSITION vec4 (x=+0x10,y=+0x14,z=+0x18,w=+0x1C).
       Confirmed: g_pHeroMoby+0x10 == g_heroPos; BindMobyToParent copies +0x10..+0x1C
       from parent; StepMobyMotion reads +0x18 as the vertical (z) coord. */
    /* --- core entity state ------------------------------------------------- */
    u8  state;           /* +0x20 liveness/state byte. >=0xFE free-and-respawnable,
                            0xFD dead static slot, <0x80 alive-and-updated, >=0x80
                            alive-but-skip (sign bit = "skip update this frame")  CONFIRMED */
    u8  groupByte;       /* +0x21 render/cull group tag; >=0 (top bit set) defers the moby
                            to whole-group pull-in in BuildActiveMobyChain          CONFIRMED */
    u8  classSlot;       /* +0x22 loaded class SLOT (index into g_mobyClass* tables),
                            = g_mobyClassSlotRemap[classId]                         CONFIRMED */
    u8  alpha;           /* +0x23 draw alpha, default 0x80 (=1.0)                   CONFIRMED */
    void *pClass;        /* +0x24 loaded class header ptr = g_mobyClassHeaders[slot]  CONFIRMED */
    void *pChainNext;    /* +0x28 chain "next" ptr - links the per-frame active chain and the
                            scratchpad per-class lists built by BuildActiveMobyChain CONFIRMED */
    f32 scale;           /* +0x2C uniform scale (default = class header+0x24)        CONFIRMED */
    u8  drawDist;        /* +0x30 LOD/draw-distance byte; 0xFF = never distance-culled CONFIRMED */
    u8  drawDistAlways;  /* +0x31 nonzero = bypass distance cull; also blob-shadow enable CONFIRMED */
    u16 bindFlags;       /* +0x32 0xFFFF on parent-bind (BindMobyToParent)            PARTIAL  */
    u16 modeFlags;       /* +0x34 MODE / behaviour bitfield - see MOBY_MODE_* below   CONFIRMED */
    u16 sndEventFlags;   /* +0x36 default 0x7F80; anim-keyed sound event state        PARTIAL  */
    u32 color;           /* +0x38 packed color (default qword 0x0040404000000000 at +0x38) CONFIRMED */
    u32 colorHi;         /* +0x3C high word of the +0x38 color qword                  PARTIAL  */
    /* --- animation block (+0x40.., driven by UpdateMobyAnimation) ----------- */
    u8  animFrame;       /* +0x40 current anim frame index                           CONFIRMED */
    u8  animFrameNext;   /* +0x41 target/next frame index                            PARTIAL  */
    u8  animSeq;         /* +0x42 current anim sequence index (0xFF = procedural)     CONFIRMED */
    u8  animSeqNext;     /* +0x43 target/next sequence index                         CONFIRMED */
    f32 animTime;        /* +0x44 sub-frame anim time accumulator                     CONFIRMED */
    f32 animRate;        /* +0x48 anim playback rate, default 1.0                     CONFIRMED */
    f32 animRate2;       /* +0x4C second anim rate / blend, default 1.0               PARTIAL  */
    f32 motionVec[2];    /* +0x50 motion/velocity scratch read by UpdateMobyMotionVelocity
                            (+0x58 within the broader block used in the Atan2 pitch term) PARTIAL */
    void *animFramePtr;  /* +0x58 resolved current-frame data ptr (ResolveMobyAnimFramePtrs) PARTIAL */
    void *animFramePtr2; /* +0x5C resolved blend/next-frame data ptr                  PARTIAL  */
    u8  animEventByte;   /* +0x60 anim event/flags byte updated each frame            PARTIAL  */
    u8  unk61;           /* +0x61 init 0xFF                                          UNCONFIRMED */
    u8  unk62;           /* +0x62 init 0xFF, then = class hdr+0xE                     PARTIAL  */
    u8  collClassByte;   /* +0x63 set 0x18 when class hdr+6 nonzero                   PARTIAL  */
    void *pUpdate;       /* +0x64 PER-CLASS UPDATE CALLBACK fn(Moby*) - copied from
                            g_mobyClassUpdateFuncs[slot] at init, invoked by
                            UpdateActiveMobys every frame. Body is overlay-resident.  CONFIRMED */
    void *pVars;         /* +0x68 per-moby 0x80-byte aux/vars block = g_mobyAuxBlockBase +
                            index*0x80 (memset 0 on spawn). The motion controller lives at
                            *(pVars)+0x18 (GetMobyMotionController).                  CONFIRMED */
    u8  loopSoundIdx;    /* +0x6C looping-anim sound index (UpdateMobyAnimLoopSound)   CONFIRMED */
    u8  loopSoundSlot;   /* +0x6D emitter slot of the looping sound, init 0xFF         CONFIRMED */
    u8  unk6E;           /* +0x6E init 0                                             UNCONFIRMED */
    u8  shadowByte6F;    /* +0x6F set 0x18 when class hdr+0xF nonzero (shadow/glow)    PARTIAL  */
    f32 shadowZDelta;    /* +0x70 ground-shadow z delta (ProbeMobyGroundBelow writes)  PARTIAL  */
    f32 shadowScale;     /* +0x74 ground-shadow scale (ProbeMobyGroundBelow writes)    PARTIAL  */
    void *pCollMesh;     /* +0x78 collision-mesh ptr = class hdr+0x40 (sets mode 0x10) CONFIRMED */
    f32 unk7C;           /* +0x7C bit 0x8000 stops the looping sound                  PARTIAL  */
    f32 boundsCache[4];  /* +0x80 cached anim-frame bounds vec (UpdateMobyBSphereAndGrid) PARTIAL */
    u8  unk90[8];        /* +0x90 un-analyzed                                        UNCONFIRMED */
    void *animSeqArray;  /* +0x98 anim-seq related ptr = class hdr+0x10 (0 => headerless) PARTIAL */
    u8  unk9C[4];        /* +0x9C un-analyzed                                        UNCONFIRMED */
    s32 releaseTime;     /* +0xA0 dual-use: FreeMoby sets g_gameTime+2 (2-frame reuse
                            cooldown); collision queries stamp it with g_collQueryStamp
                            so a multi-cell moby is tested once per query            CONFIRMED */
    u8  gridCellCache[4];/* +0xA4 cached spatial-grid cell range (sentinel 7F 7F 80 80),
                            compared in UpdateMobyBSphereAndGrid to detect movement   CONFIRMED */
    u8  hitEventIdx;     /* +0xA8 index of this moby's latest entry in g_collHitEventRing,
                            init 0xFF; dedupes PostMobyHitEvent                       CONFIRMED */
    u8  unkA9;           /* +0xA9 un-analyzed                                        UNCONFIRMED */
    s16 classId;         /* +0xAA original moby CLASS ID (oClass arg to SpawnMoby)    CONFIRMED */
    u16 dirtyCounter;    /* +0xAC per-frame bsphere dirty/version counter, bumped by
                            UpdateMobyBSphereAndGrid. Occupies the LOW halfword that the
                            uid write below leaves zero.                              CONFIRMED */
    u16 slotIndex;       /* +0xAE table slot index (== uid high half). InitMobyFromClass
                            writes (slotIndex << 16) into the u32 at +0xAC, i.e. this
                            halfword, leaving +0xAC=dirtyCounter as zero. So the full u32
                            uid is (slotIndex<<16 | dirtyCounter).                    CONFIRMED */
    u8  unkB0[8];        /* +0xB0 un-analyzed                                        UNCONFIRMED */
    void *pParent;       /* +0xB8 parent moby ptr (BindMobyToParent); child inherits the
                            parent pos/facing/color                                  CONFIRMED */
    u8  unkBC;           /* +0xBC un-analyzed                                       UNCONFIRMED */
    u8  unkBD;           /* +0xBD cleared with the shadow/glow setup at +0x6F         PARTIAL  */
    u8  flagBE;          /* +0xBE one-shot latch byte: bit0/bit1 set by
                            SetMobyFlagBit0/1 (return "newly set")                    CONFIRMED */
    u8  unkBF;           /* +0xBF un-analyzed                                       UNCONFIRMED */
    /* --- orientation matrix (3x4, present when MOBY_MODE_HAS_MATRIX 0x100 set) -- */
    f32 mtxRow0[4];      /* +0xC0 orientation matrix row 0 (UpdateMobyBSphereAndGrid)  CONFIRMED */
    f32 mtxRow1[4];      /* +0xD0 orientation matrix row 1                            CONFIRMED */
    f32 mtxRow2[4];      /* +0xE0 orientation matrix row 2                            CONFIRMED */
    /* --- facing / heading (+0xF0..) ---------------------------------------- */
    f32 facing[4];       /* +0xF0 facing/forward vec4; copied whole on parent-bind.
                            +0xF8 within it is the HEADING YAW ANGLE (radians) read by
                            StepMobyMotion / DriveMobyAlongFacing.                    CONFIRMED */
} Moby; /* sizeof == 0x100 */

/* --- moby +0x34 MODE / behaviour flag bits (from InitMobyFromClass + the walks) --- */
#define MOBY_MODE_NO_UPDATE_FN  0x0002 /* pUpdate(+0x64) is null                       */
#define MOBY_MODE_SKIP_GRID     0x0004 /* UpdateActiveMobys skips UpdateMobyBSphereAndGrid */
#define MOBY_MODE_HEADERLESS    0x0005 /* class registered with null header (|=5)      */
#define MOBY_MODE_HAS_COLLMESH  0x0010 /* +0x78 collision mesh present (hdr+0x40 != 0)  */
#define MOBY_MODE_NO_VELOCITY   0x0010 /* motion: skip velocity integrate (same bit, motion ctx) */
#define MOBY_MODE_ANIM_FLAG40   0x0040 /* skip UpdateMobyAnimation this frame           */
#define MOBY_MODE_HAS_MATRIX    0x0100 /* +0xC0 orientation matrix is authoritative      */
#define MOBY_MODE_HIDDEN        0x0080 /* HideAllMobysAndPushState sets, Unhide clears (PARTIAL) */
#define MOBY_MODE_FLAGGED1000   0x1000 /* collected to g_mobyFlagged1000List each frame  */
#define MOBY_MODE_SHADOW_400    0x0400 /* shadow/glow setup latch (hdr+0xF nonzero)      */

/* === Pool / table globals (see symbol_addrs/usa for full notes) ============ */
/* g_mobyTableBase     0x1B1ADC  Moby* array base, stride 0x100, slot 0 reserved        */
/* g_mobySpawnStart    0x1B1AE0  first DYNAMIC slot; SpawnMoby free-scan start           */
/* g_mobyTableEnd      0x1B1AE4  end of the table; bound of every walk                   */
/* g_mobyAuxBlockBase  0x1B1AEC  parallel 0x80-byte aux/vars blocks (moby+0x68)          */
/* g_activeMobyChainHead 0x1B1AE8 head of the per-frame active chain (linked via +0x28)  */
/* g_gameTime          0x1B1608  global frame counter; gates the +0xA0 reuse cooldown    */
/* g_mobySpawnCredit   0x1B1A00  per-frame spawn budget, decremented by SpawnMoby (UNCONF)*/
/* g_mobyClassCount    0x1B1AC0  loaded header-class count = next free slot               */

/* === Class table (classId/slot -> behaviour & assets) ====================== */
/* g_mobyClassSlotRemap   0x1CE460  u8[0x2000] classId -> slot (0xFF = not loaded)        */
/* g_mobyClassSlotToId    0x1CE280  u16[] slot -> classId (reverse map)                   */
/* g_mobyClassHeaders     0x1CDB00  loaded class header ptr per slot                      */
/* g_mobyClassUpdateFuncs 0x1CDEC0  per-slot pUpdate (header classes) -> moby+0x64        */
/* g_mobyClassUpdateFuncsNoHeader 0x1D0460  per-slot pUpdate (headerless classes)         */
/* g_mobyClassBounds      0x1D1500  vec4 bounds per class slot                            */
/* g_mobyClassDataSizes   0x1D0D80  u32 per slot, hdr+0x2D << 10                           */
/* g_builtinMobyUpdateBindings 0x26E880  classId/fn binding table (empty in ELF, overlay-filled)*/

/* === Spatial grid (broad-phase hash) ======================================= */
/* g_mobyGridCells     0x1D2460  u32/cell = u16 blockIdx | u8 count | u8 cap              */
/* g_mobyGridBlocks    0x1D6460  0x20-byte blocks of u16 moby indices                     */
/* g_mobyGridBlockBitmap 0x1EE460  bit-per-block allocator                                */

/* === Compile-time layout verification (ILP32 targets only) =================
 * Active only under a 4-byte-pointer compile (PS2 EE + native -m32), where the
 * record is 0x100 bytes. ee-gcc 2.9 predates __SIZEOF_POINTER__ so this is
 * skipped in the matching build; the host-64 linter (ptr==8) is skipped too. The
 * native -m32 build runs them - that is the verification target. */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(Moby) == 0x100, "Moby must be 0x100 under ILP32");
_Static_assert(__builtin_offsetof(Moby, bsphereScratch) == 0x10, "world position");
_Static_assert(__builtin_offsetof(Moby, state)      == 0x20, "state");
_Static_assert(__builtin_offsetof(Moby, classSlot)  == 0x22, "classSlot");
_Static_assert(__builtin_offsetof(Moby, alpha)      == 0x23, "alpha");
_Static_assert(__builtin_offsetof(Moby, pClass)     == 0x24, "pClass");
_Static_assert(__builtin_offsetof(Moby, pChainNext) == 0x28, "pChainNext");
_Static_assert(__builtin_offsetof(Moby, scale)      == 0x2C, "scale");
_Static_assert(__builtin_offsetof(Moby, modeFlags)  == 0x34, "modeFlags");
_Static_assert(__builtin_offsetof(Moby, color)      == 0x38, "color");
_Static_assert(__builtin_offsetof(Moby, animFrame)  == 0x40, "animFrame");
_Static_assert(__builtin_offsetof(Moby, animTime)   == 0x44, "animTime");
_Static_assert(__builtin_offsetof(Moby, animFramePtr) == 0x58, "animFramePtr");
_Static_assert(__builtin_offsetof(Moby, pUpdate)    == 0x64, "pUpdate");
_Static_assert(__builtin_offsetof(Moby, pVars)      == 0x68, "pVars");
_Static_assert(__builtin_offsetof(Moby, pCollMesh)  == 0x78, "pCollMesh");
_Static_assert(__builtin_offsetof(Moby, boundsCache) == 0x80, "boundsCache");
_Static_assert(__builtin_offsetof(Moby, releaseTime) == 0xA0, "releaseTime");
_Static_assert(__builtin_offsetof(Moby, hitEventIdx) == 0xA8, "hitEventIdx");
_Static_assert(__builtin_offsetof(Moby, classId)    == 0xAA, "classId");
_Static_assert(__builtin_offsetof(Moby, dirtyCounter) == 0xAC, "dirtyCounter");
_Static_assert(__builtin_offsetof(Moby, slotIndex)  == 0xAE, "slotIndex");
_Static_assert(__builtin_offsetof(Moby, pParent)    == 0xB8, "pParent");
_Static_assert(__builtin_offsetof(Moby, mtxRow0)    == 0xC0, "mtxRow0");
_Static_assert(__builtin_offsetof(Moby, mtxRow1)    == 0xD0, "mtxRow1");
_Static_assert(__builtin_offsetof(Moby, mtxRow2)    == 0xE0, "mtxRow2");
_Static_assert(__builtin_offsetof(Moby, facing)     == 0xF0, "facing");
#endif

#endif /* MOBY_H */

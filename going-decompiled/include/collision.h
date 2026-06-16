#ifndef COLLISION_H
#define COLLISION_H

#include "common.h"

/* Collision / physics-query subsystem (USA v2.00 SCUS_972.68).
 *
 * This is the raycast + spatial-query core every gameplay system uses (camera,
 * weapons, mobys, player ground-probe). Recovered from the collision cluster at
 * 0x276168-0x278EB7 plus the moby-side probes/event posters in 0x2A9810-0x2AA2xx.
 *
 * Public query entry points (all leave their result in the flat hit-result
 * global block below, see g_collHit*):
 *   CollLine             0x276208  world+moby line-segment raycast
 *                                  (from, to, flags, ignoreMoby, hitEventTmpl)
 *   CollSphere           0x277268  world+moby sphere query; flag 0x40 gathers
 *                                  triangles into g_collTriBuffer and returns count
 *   QueryMobysInSphere   0x277F58  moby-only sphere gather + hit-event broadcast
 *                                  (Ghidra/engine alias CollMobysSphere) - fills
 *                                  g_collMobyHitList and posts CollHitEvent records
 *   TestLineVsGatheredTris 0x278CF0 cheap segment test vs a pre-gathered
 *                                  g_collTriBuffer (gather-once / test-many)
 *   TestSphereVsExtraMesh  0x278880 sphere vs the optional g_pCollExtraMesh
 *                                  (1/64 fixed-point overlay mesh)
 *   GetCollHitMaterial   0x278E88  low 5 bits of g_collHitPolyInfo (0 = water)
 *   LookupCollSector     0x276168  internal 3-level jagged world-sector lookup
 *
 * Moby-side helpers:
 *   ProbeGroundHeight     0x2A9810  downward CollLine, returns hit z
 *   ProbeMobyGroundBelow  0x2AA218  ground probe writing a moby's shadow z/scale
 *   ProbeMobyGroundLine   0x2B6AE0  vertical ground probe for the motion controller
 *   PostMobyHitEvent      0x2A9DD8  posts ONE CollHitEvent into g_collHitEventRing
 *   SkinMobyCollisionMesh 0x2A3F60  poses a moby collision mesh into the cache
 *
 * The broad-phase is the moby spatial grid (g_mobyGridCells 0x1D2460 etc., see
 * moby.h): the sphere queries walk the cell range that the query AABB overlaps,
 * then test each moby's collision primitive set (per-moby primitive types
 * 1/2/3 + skinned mesh).
 */

/* === Hit-result global block (flat, NOT a passed-around struct) =============
 * The collision queries do NOT take a result-struct pointer; they write their
 * outcome to this fixed block of individually-addressed globals at 0x1BAF00.
 * Listed here for reference - each is its own symbol in symbol_addrs:
 *
 *   0x1BAF00  void* g_pCollWorldData          world sector structure (overlay-set)
 *   0x1BAF04  void* g_collMobyMeshCache        base of 8 x 0x800 skinned-mesh cache
 *   0x1BAF08  int   g_collMobyMeshCacheKeys[8] cache keys
 *   0x1BAF0C  int   g_collMobyMeshCacheCursor  round-robin cache cursor
 *   0x1BAF10  int   g_collQueryStamp           per-query counter (sign bit = disabled)
 *   0x1BAF14  int   g_collHitEventCursor        ring write cursor 0..63
 *   0x1BAF18  Moby* g_pCollHitMoby             moby hit by last query (0 = world)
 *   0x1BAF1C  int   g_collHitPolyInfo          packed poly info; low5 = material id,
 *                                              negative = no hit; 0x3F set by sphere
 *   0x1BAF20  f32   g_collHitPoint[4]          last hit point (world units)
 *   0x1BAF30  f32   g_collHitPointNudged[4]    hit point nudged toward query origin
 *   0x1BAF40  f32   g_collHitNormal[4]         face normal of last hit
 *   0x1BAF50  f32   g_collHitTriVerts[4]       hit triangle vertex 0 (v0)
 *   0x1BAF60  f32   g_collHitTriVert1[4]       hit triangle vertex 1 (v1)
 *   0x1BAF70  f32   g_collHitTriVert2[4]       hit triangle vertex 2 (v2)
 *
 * All three tri verts are written by CollLine / CollSphere /
 * TestLineVsGatheredTris / TestSphereVsExtraMesh (the latter scales by 1/64).
 * The ELF side is write-only for the tri verts; readers live in level overlays.
 */

/* g_collHitPolyInfo material id is the low 5 bits; 0x1F means "no material". */
#define COLL_MATERIAL_MASK   0x1F
#define COLL_MATERIAL_NONE   0x1F
#define COLL_MATERIAL_WATER  0x00   /* CheckCameraUnderwater treats material 0 as water */

/* === CollHitEvent — moby hit/damage event record ===========================
 * The ONE genuine base+offset struct in this subsystem. g_collHitEventRing
 * (0x1BF180) is a ring of 64 of these (0x40 bytes each), written round-robin
 * via g_collHitEventCursor (& 0x3F). QueryMobysInSphere posts one per hit moby
 * during an area sweep; PostMobyHitEvent / PostMobyDamagePacket post one for a
 * single target. Each moby caches the index of its latest entry at moby+0xA8
 * (hitEventIdx, init 0xFF) so repeat hits in the same window dedupe: a fresh
 * hit with damage < the live entry's just ORs its flag bits into +0x24.
 * Consumers are the per-class moby update handlers in the level overlays, which
 * read the ring and subtract damage from the target's combat HP.
 *
 * Layout CONFIRMED from PostMobyHitEvent (0x2A9DD8), which stamps every field,
 * cross-checked against the QueryMobysInSphere broadcast writer. */
typedef struct CollHitEvent {
    f32  hitPos[4];      /* +0x00 hit point (vec4)                                    */
    f32  hitDir[4];      /* +0x10 hit/impulse direction (vec4)                        */
    s32  sourceId;       /* +0x20 attacker / source id                               */
    u32  flags;          /* +0x24 damage/effect flag bits (OR-accumulated on repeat) */
    f32  _pad28;         /* +0x28                                                     */
    f32  damage2C;       /* +0x2C damage (also mirrored at +0x34)                     */
    s32  hasDirection;   /* +0x30 1 when |hitDir| > 0.0001, else 0                    */
    f32  damage34;       /* +0x34 damage / compare key for dedupe                     */
    void *target;        /* +0x38 target moby (Moby*)                                 */
    s32  consumed;       /* +0x3C 0 = fresh (set by writer; cleared per post)         */
} CollHitEvent;          /* sizeof = 0x40                                             */

/* Layout is ILP32 (4-byte pointers): the +0x38 target pointer makes sizeof 0x40
 * only under a 4-byte-pointer compile (PS2 EE + native -m32). These asserts are
 * skipped on a 64-bit host (where the pointer widens the record) and under ee-gcc
 * 2.9 (which predates __SIZEOF_POINTER__); the native -m32 build runs them. */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(CollHitEvent) == 0x40, "CollHitEvent size");
_Static_assert(__builtin_offsetof(CollHitEvent, hitDir)       == 0x10, "hitDir");
_Static_assert(__builtin_offsetof(CollHitEvent, sourceId)     == 0x20, "sourceId");
_Static_assert(__builtin_offsetof(CollHitEvent, flags)        == 0x24, "flags");
_Static_assert(__builtin_offsetof(CollHitEvent, damage2C)     == 0x2C, "damage2C");
_Static_assert(__builtin_offsetof(CollHitEvent, hasDirection) == 0x30, "hasDirection");
_Static_assert(__builtin_offsetof(CollHitEvent, damage34)     == 0x34, "damage34");
_Static_assert(__builtin_offsetof(CollHitEvent, target)       == 0x38, "target");
_Static_assert(__builtin_offsetof(CollHitEvent, consumed)     == 0x3C, "consumed");
#endif

/* Ring geometry. */
#define COLL_HIT_EVENT_RING_COUNT  64    /* g_collHitEventRing entries           */
#define COLL_HIT_EVENT_RING_MASK   0x3F  /* cursor wrap mask                     */
#define COLL_HIT_EVENT_NONE        0xFF  /* moby->hitEventIdx "no live entry"    */

/* Hit-event eligibility: QueryMobysInSphere only posts a CollHitEvent for a
 * moby whose flag word at moby+0x34 has this bit set (area-damageable). */
#define MOBY_COLL_AREA_DAMAGEABLE  0x4000

/* g_collMobyHitList (0x1BEF80): moby* list filled by QueryMobysInSphere, cap 128. */
#define COLL_MOBY_HIT_LIST_MAX     128

/* g_collTriBuffer (0x1C0180): CollSphere gather records, 4 vec4 each (v0,v1,v2,
 * normal), cap 63; consumed by TestLineVsGatheredTris. */
#define COLL_TRI_BUFFER_MAX        63

#endif /* COLLISION_H */

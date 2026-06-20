#include "common.h"
#include "sound_emitter.h"   /* SoundEmitterSlot (0x70 emitter slot) */
#include "moby.h"            /* Moby (0x100 entity record) */
#include "vec.h"             /* Vec4 (16-byte xyzw) */

/* ------------------------------------------------------------------------- *
 *  Sound-definition record (the per-sound tuning blob a play call references).
 *
 *  Recovered from the USA v2.00 (SCUS_972.68) emitter cluster.  Every play
 *  entry point strides its sound-def pool by 0x20 bytes (PlayGlobalSound and
 *  PlaySoundFromClassBank both index `pool + idx*0x20`), so the record is
 *  exactly 0x20 bytes.  Field offsets are CONFIRMED from the asm that reads
 *  them:
 *    - StartSoundEmitter (0x2E68A0): reads +0x18 (enable gate, byte),
 *      +0x1A (sampleId, u16), +0x10/+0x14 (pitch min/max, s32).
 *    - ComputeVolumeFalloff (0x2E5398): reads +0x19 bit0 (squared-falloff flag),
 *      +0x08 (far volume, s32), +0x0C (near volume, s32).
 *    - ComputeEmitterVolume (0x2E5490): reads +0x00 / +0x04 as the curve's
 *      near/far radii (float) via the slot's def pointer.
 *  +0x1C is the bank index per sound_emitter.h (PROBABLE, not touched here).
 *  Gaps are padding/unverified.
 *
 *  Bodies below address the record with raw byte casts (matching the asm), so
 *  this stays a sized record with named CONFIRMED fields; the size is what the
 *  retype guarantees. */
typedef struct SoundDef {
    /* 0x00 */ f32 nearRadius;   /* CONFIRMED inner falloff radius             */
    /* 0x04 */ f32 farRadius;    /* CONFIRMED outer falloff radius             */
    /* 0x08 */ s32 farVolume;    /* CONFIRMED volume at/beyond farRadius       */
    /* 0x0C */ s32 nearVolume;   /* CONFIRMED volume at/within nearRadius      */
    /* 0x10 */ s32 pitchMin;     /* CONFIRMED pitch range low                  */
    /* 0x14 */ s32 pitchMax;     /* CONFIRMED pitch range high (== min: fixed) */
    /* 0x18 */ u8  enableGate;   /* CONFIRMED start gate vs flags&4            */
    /* 0x19 */ u8  curveFlags;   /* CONFIRMED bit0 = squared falloff           */
    /* 0x1A */ u16 sampleId;     /* CONFIRMED 989snd sample id                 */
    /* 0x1C */ s32 bankIndex;    /* PROBABLE  bank handle index (per header)   */
} SoundDef;                      /* sizeof == 0x20 */
/* Guard skips the assert on the ee-gcc 2.9 (C89) matching toolchain, which
 * predates __SIZEOF_POINTER__ and _Static_assert - only the native build checks it. */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(SoundDef) == 0x20, "SoundDef must be 0x20 (pool stride)");
#endif

/* ------------------------------------------------------------------------- *
 *  3D sound-emitter system (moby-glow / shrub / sky / sound-emitter unit)
 *
 *  The emitter slot pool is g_soundEmitterTable: 52 entries of 0x70 bytes.
 *  The original translation unit addresses the pool through the preceding
 *  symbol g_listenerPosHistory (0x188660); g_soundEmitterTable sits one stride
 *  (0x70) above it, so slot fields are reached as g_listenerPosHistory + 0x70 +
 *  idx*0x70 + fieldOffset.  Keeping g_listenerPosHistory as the relocation base
 *  (matching the original) and folding the +0x70 into the displacement is what
 *  makes these byte-exact.
 * ------------------------------------------------------------------------- */

#ifdef TARGET_NATIVE
/* gcc -m32 cannot emulate the 128-bit integer (mode(TI)); these units only
 * COPY/zero quadwords (no 128-bit arithmetic), so a 16-byte aligned struct is
 * an exact portable stand-in. Inert to the matching build (keeps mode(TI)). */
typedef struct { unsigned long long _q[2]; } __attribute__((aligned(16))) u_long128;
#else
typedef unsigned long u_long128 __attribute__((mode(TI)));
#endif

extern u8 g_listenerPosHistory[]; /* 0x188660 - ring of 4 listener vec4 */

/* GS/VIF frame-build + sky/shrub render globals (large absolute addresses, so
 * sized non-small to force %hi/%lo; g_texUploadCount stays gp_rel small). */
__asm__(".extern g_frameDmaCursor, 16");
extern u8 *g_frameDmaCursor;       /* 0x1B2228 - frame VIF1 chain write cursor */
__asm__(".extern g_pSkyData, 16");
extern u8 *g_pSkyData;             /* 0x1B2040 - relocated sky chunk ptr */
__asm__(".extern g_pSkySegmentOpenTag, 16");
extern u8 *g_pSkySegmentOpenTag;   /* 0x1B2060 - sky segment head tag */
__asm__(".extern g_pShrubSegmentOpenTag, 16");
extern u8 *g_pShrubSegmentOpenTag; /* 0x1B2034 - shrub segment head tag */
__asm__(".extern g_vramDynamicBase, 16");
extern s32 g_vramDynamicBase;      /* 0x1A72D4 - VRAM dynamic region base */
__asm__(".extern g_vramAllocCursor, 16");
extern s32 g_vramAllocCursor;      /* 0x1A72D0 - byte-addressed VRAM bump cursor */
extern s32 g_texUploadCount;       /* 0x1B157C - pending texture uploads (gp_rel) */

/* One 3D sound-emitter slot (stride 0x70). */
typedef struct SoundEmitter {
    /* 0x00 */ s32 voiceHandle;   /* 989snd voice handle / id */
    /* 0x04 */ u8  state;         /* 0 free, 1 keyed-on, 2 playing, 4/6/7 stop/pending */
    /* 0x05 */ u8  flags;
    /* 0x06 */ u8  pad06[0x2];
    /* 0x08 */ void *def;         /* sound definition pointer */
    /* 0x0C */ u8  pad0C[0x4];
    /* 0x10 */ s16 volumeScale;
    /* 0x12 */ s16 occlusionCursor;
    /* 0x14 */ s32 pitch;
    /* 0x18 */ s32 owner;         /* owning moby */
    /* 0x1C */ s32 f1C;
    /* 0x20 */ u8  rest[0x50];
} SoundEmitter;

/* The emitter pool, addressed through g_listenerPosHistory (+0x70). */
#define g_soundEmitterTable ((SoundEmitter *)(g_listenerPosHistory + 0x70))

/* Index-addressed view of the same pool.  The original code reaches slot
 * `idx` as g_listenerPosHistory + idx*0x70 and accesses emitter fields at the
 * absolute offsets 0x70.. (the leading 0x70 is the listener-history prefix),
 * i.e. the +0x70 lands in the load displacement, not in the symbol address.
 * Reproducing that requires the explicit idx*0x70 multiply plus field offsets
 * that already include the 0x70 prefix. */
typedef struct EmitterView {
    /* 0x00 */ u8  prefix[0x70]; /* listener-history slot occupying the stride */
    /* 0x70 */ s32 voiceHandle;
    /* 0x74 */ u8  state;
    /* 0x75 */ u8  flags;
    /* 0x76 */ u8  pad76[0xE];
    /* 0x84 */ s32 pitch;
    /* 0x88 */ s32 owner;
    /* 0x8C */ s32 f8C;
    /* 0x90 */ s32 f90;
    /* 0x94 */ u8  pad94[0xC];
    /* 0xA0 */ u_long128 quadA0;  /* 16-byte spatial/transform field */
} EmitterView;

#define EMITTER_VIEW(idx) \
    ((EmitterView *)(g_listenerPosHistory + (idx) * 0x70))

/* 989snd deferred key-on callback: store the freshly allocated voice handle in
 * the emitter slot and advance state 1 -> 2.  A zero handle means the voice
 * failed to start, so free the slot (state 0, clear the bookkeeping words). The
 * slot arrives as a 64-bit value whose low 32 bits hold its address. */
void OnEmitterVoiceKeyedOn(s32 handle, long slotAddr) {
    SoundEmitter *slot = (SoundEmitter *)slotAddr;
    if (slot == NULL) {
        return;
    }
    slot->voiceHandle = handle;
    if (handle != 0) {
        if (slot->state == 1) {
            slot->state = 2;
        }
    } else {
        slot->owner = 0;
        slot->f1C = 0;
        slot->state = 0;
    }
}

/* 989snd voice-ended callback: record the (zero) handle and free the emitter
 * slot - clear the state byte and the owner/link bookkeeping words. */
void OnEmitterVoiceEnded(s32 handle, long slotAddr) {
    SoundEmitter *slot = (SoundEmitter *)slotAddr;
    if (slot == NULL) {
        return;
    }
    slot->voiceHandle = handle;
    if (handle != 0) {
        return;
    }
    slot->owner = 0;
    slot->f1C = 0;
    slot->state = 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0000);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0010);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", HideAllMobysAndPushState);

__asm__(".extern g_sceneActorMobys, 16");
extern u8 g_sceneActorMobys[]; /* 0x1B894C - current scene cast moby pointers */
__asm__(".extern g_mobyTableBase, 16");
extern u8 *g_mobyTableBase;    /* 0x1B1ADC - moby entity array base (stride 0x100) */
__asm__(".extern g_mobyTableEnd, 16");
extern u8 *g_mobyTableEnd;     /* 0x1B1AE4 - end of the moby table */
extern void PopGameState(s32 a, s32 b);
extern void func_002857C8(s32 a, s32 b, s32 c);

/* Pop the game-state stack and un-hide every moby: clear the "hidden" flag bit
 * (0x80 at moby+0x34) across the whole moby table.  Also re-runs the scene-cast
 * helper func_002857C8 with the three words at g_sceneActorMobys+0x8B0..
 * NEAR-MISS (~85%, structurally identical): cc1 lowers the table-walk to a plain
 * `bnez` where the original uses a branch-likely (`bnel`) that hoists the moby
 * flag load into the delay slot (the branch-likely lowering wall).  The C is
 * faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", UnhideAllMobysAndPopState);
#else
void UnhideAllMobysAndPopState(void) {
    s32 *castArgs;
    u8 *moby;

    PopGameState(0, 0);
    castArgs = (s32 *)(g_sceneActorMobys + 0x674);
    func_002857C8(castArgs[0x23C / 4], castArgs[0x240 / 4], castArgs[0x244 / 4]);

    for (moby = g_mobyTableBase; moby < g_mobyTableEnd; moby += 0x100) {
        *(u16 *)(moby + 0x34) &= 0xFF7F;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0210);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0448);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0458);

extern void func_002E1220(void *rec);
extern void func_002E1370(void *rec);

/* Dispatch one draw-list record by its type tag (s16 at offset 0): tag 0 ->
 * func_002E1220, advance 0x20; tag 1 -> func_002E1370, advance 0x30; any other
 * tag -> no-op.  Returns the pointer to the next record.
 * NEAR-MISS (~81%): cc1 picks a 0x20 frame + different save layout where the
 * original uses a 0x10 frame and pre-stages the tag-compare constant in the
 * branch delay slot (frame/reg-alloc wall). The C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0568);
#else
void *func_002E0568(void *rec) {
    u8 *p = (u8 *)rec;
    s16 tag = *(s16 *)p;
    if (tag == 0) {
        func_002E1220(p);
        return p + 0x20;
    }
    if (tag == 1) {
        func_002E1370(p);
        return p + 0x30;
    }
    return p;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E05C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0650);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E07F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", EmitMobyGlowPackets);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", BuildMobyGlowRecords);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0EA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E1220);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E1370);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E19C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E1A58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", CloseShrubDrawSegment);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", PatchShrubPacketTex0);

extern u8 g_shrubRelightList[];    /* 0x2140D0 - shrub relight list */
extern void func_0011AEA0(s32 arg);
extern void CullAndEmitShrubs(void);
extern void func_00283558(void *list, s32 a, s32 b);
extern void CloseShrubDrawSegment(void);

/* Build the shrub draw segment: open the segment (record DMA cursor head tag,
 * advance the cursor, snapshot VRAM cursor), flush pending work, cull+emit the
 * shrub instances, relight the shrub list, then close the segment.
 * NEAR-MISS (~34%): the empty-asm guard suppresses the final sibling call, but
 * the original schedules the g_frameDmaCursor store as a 1-insn %gp_rel write
 * into the func_0011AEA0 jal delay slot (the delay-slot-driven gp_rel reload
 * artifact) which cc1 won't reproduce here.  The C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", BuildShrubDrawSegment);
#else
void BuildShrubDrawSegment(void) {
    g_pShrubSegmentOpenTag = g_frameDmaCursor;
    g_vramAllocCursor = g_vramDynamicBase;
    g_frameDmaCursor = g_frameDmaCursor + 0x10;
    func_0011AEA0(0);
    CullAndEmitShrubs();
    func_00283558(g_shrubRelightList, 0x3200, 0x40);
    CloseShrubDrawSegment();
    __asm__ __volatile__("");
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", CullAndEmitShrubs);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", UploadShrubTextures);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", PatchShrubVertexLighting);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E4000);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E4178);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E4280);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", UpdateSkyShellRotation);

extern u8 g_skyShellMatrix[]; /* 0x1B2270 - shared sky-shell transform matrix */
extern void MatrixIdentityVu0(void *m);
extern void UpdateSkyShellRotation(s32 shellIdx);
extern void DrawSkyShell(s32 shellIdx);

/* Draw every sky shell with its fixed per-shell spin: shells 0..9 spin via
 * UpdateSkyShellRotation, shells 10+ fall back to an identity matrix (no spin
 * data), then each is drawn.  The shell count (g_pSkyData+0x6) is re-read every
 * iteration.
 * TODO(match): functional equivalent - not byte-exact; this cc1's loop-invariant
 * code motion hoists the g_skyShellMatrix %hi address out of the loop into an
 * extra callee save (s1), growing the frame 0x10 -> 0x30, where the original
 * re-materializes the address inline in the (rarely taken) matrix block and
 * needs no s1 (the LICM-hoist wall). Body control flow + branch order match. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", DrawSkyShellsFixedSpin);
#else
void DrawSkyShellsFixedSpin(void) {
    s32 i;
    for (i = 0; i < *(s16 *)(g_pSkyData + 0x6); i++) {
        if (i >= 0xA) {
            MatrixIdentityVu0(g_skyShellMatrix);
        } else {
            UpdateSkyShellRotation(i);
        }
        DrawSkyShell(i);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E43E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E43F8);

/* Open a sky draw segment: remember the current DMA cursor as the segment head
 * tag, advance the cursor by one qword, snapshot the dynamic VRAM cursor, reset
 * the texture-upload queue, then zero the 8-byte header of every sky shell piece
 * (stride 0x10) in the piece list at g_pSkyData+0x10.
 * NEAR-MISS (~37%): the prologue matches but cc1 strength-reduces the clear loop
 * to a pointer-walk where the original keeps the index*0x10 + reloaded-base form
 * (re-reading the piece-list pointer each iteration).  The C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", BeginSkyDrawSegment);
#else
void BeginSkyDrawSegment(void) {
    u8 *cursor = g_frameDmaCursor;
    u8 *sky = g_pSkyData;
    s32 i;

    g_pSkySegmentOpenTag = cursor;
    g_frameDmaCursor = cursor + 0x10;
    g_vramAllocCursor = g_vramDynamicBase;
    g_texUploadCount = 0;

    for (i = 0; i < *(s16 *)(sky + 0xC); i++) {
        *(s64 *)(*(u8 **)(sky + 0x10) + i * 0x10) = 0;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", CloseSkyDrawSegment);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E4750);

extern void DrawSkyPiecesFormatA(void *piece);
extern void DrawSkyPiecesFormatB(void *piece);

/* Draw sky shell `shellIdx`: look up its piece list (g_pSkyData+0x20 pointer
 * array) and dispatch to format B if the piece carries a +0x4 sub-list,
 * otherwise format A.  Shell indices past the count (+0x6) are ignored.
 * NEAR-MISS (~97%): the empty-asm guard correctly suppresses cc1's sibling
 * call, but cc1 shares one `ld $ra` epilogue where the original duplicates it
 * (one in the post-call branch delay, one on the early-return path) - a fixed
 * branch-scheduling difference.  The C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", DrawSkyShell);
#else
void DrawSkyShell(s32 shellIdx) {
    u8 *sky = g_pSkyData;
    void *piece;

    if (shellIdx < *(s16 *)(sky + 0x6)) {
        piece = *(void **)(sky + 0x20 + shellIdx * 4);
        if (*(s32 *)((u8 *)piece + 0x4) != 0) {
            DrawSkyPiecesFormatB(piece);
        } else {
            DrawSkyPiecesFormatA(piece);
        }
    }
    __asm__ __volatile__("");
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", DrawSkyPiecesFormatA);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", DrawSkyPiecesFormatB);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E4C68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", TransformSkyPieceVerts);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E4DB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", EmitSkyTrianglePackets);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E5074);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", CullSkyPieceVisibility);

extern u8 g_cameraPos[];      /* 0x1B52C0 - listener / camera world position */
extern u8 g_collHitPoint[];   /* 0x1C4F20 - last line-trace hit point */
extern u8 g_sndChannelVolumes[]; /* 0x188F40 - sound-channel mix state block */
extern void Vec4SubVu0(void *dst, void *a, void *b);
extern void Vec4ScaleVu0(void *dst, void *src, float s);
extern void Vec4AddVu0(void *dst, void *a, void *b);
extern void func_00283968(void *dst, void *src, float s);
extern s32 func_002A87F0(float a, float b);
extern s32 CollLine(void *a, void *b, s32 mask, s32 owner, s32 flags);

/* Build a listener occlusion probe in vec4 `out`: trace a collision line from the
 * camera toward `out`'s world position; if it hits, pull `out` back to 0.75 of
 * the camera->hit distance (camera + 0.75*(hit-camera)), nudging the listener
 * probe to just in front of the occluder.
 * TODO(match): functional equivalent - not byte-exact; body order is exact but
 * two walls remain - this cc1 packs the three callee saves (s0,s1,ra) at a
 * 16-byte stride (0x20 frame) where the original uses an 8-byte stride (0x10
 * frame), and it lowers the trailing Vec4AddVu0 to a sibling/tail j that the
 * original keeps as a jal + restore. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", ComputeListenerOcclusionProbe);
#else
void ComputeListenerOcclusionProbe(Vec4 *out) {
    func_002A87F0(0.5f, 6.0f);
    Vec4AddVu0(out, out, g_cameraPos);
    if (CollLine(g_cameraPos, out, 0x82,
                 *(s32 *)(g_sndChannelVolumes + 0x24), 0) != 0) {
        Vec4SubVu0(out, g_collHitPoint, g_cameraPos);
        Vec4ScaleVu0(out, out, 0.75f);
        Vec4AddVu0(out, out, g_cameraPos);
    }
}
#endif

/* Cast an occlusion ray from emitter `emitter` toward listener `outHit`: build a
 * probe point a short distance (0.75 * +64) along the emitter->camera direction,
 * offset from the camera, then collision-trace a line from there into the world
 * (mask 0x82, owner *(emitter+0x18)).  Used to test whether a sound source is
 * occluded from the listener.
 * TODO(match): functional equivalent - not byte-exact; every body instruction
 * matches; the only delta is frame layout - this cc1 packs the callee saves
 * (s0,s1,s2,ra) at a 16-byte stride (0x50 frame) where the original uses an
 * 8-byte stride (0x30 frame) (the 8-byte-packed save wall, same as
 * func_002E6D98). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", CastEmitterOcclusionRay);
#else
void CastEmitterOcclusionRay(SoundEmitterSlot *emitter, void *outHit) {
    f32 probe[4];
    Vec4SubVu0(probe, (u8 *)emitter + 0x20, g_cameraPos);
    Vec4ScaleVu0(probe, probe, 0.75f);
    func_00283968(probe, probe, 64.0f);
    Vec4AddVu0(probe, probe, g_cameraPos);
    CollLine(outHit, probe, 0x82, *(s32 *)((u8 *)emitter + 0x18), 0);
}
#endif

extern float func_002837F8(void *a, void *b);
extern s32 ComputeVolumeFalloff(SoundDef *def, float dist, float lo, float hi);
extern u8 g_cameraPos[]; /* 0x1B52C0 - listener / camera world position */

/* Map a listener distance to a volume level along the emitter's distance
 * falloff curve. `def` points at the sound definition; `dist` is the listener
 * distance; `near`/`far` are the curve's inner/outer radii (def+0x0 / def+0x4).
 * The curve interpolates between two integer volume levels: the far volume at
 * def+0x8 (returned when dist >= far) and the near volume def+0xC (returned when
 * dist <= near). In between, the level is def+0x8 plus the fraction of the
 * (def+0xC - def+0x8) span given by the position of `dist` in [near, far],
 * measured from the far end: linear in (far-dist)/(far-near) by default, or in
 * its square ((far-dist)^2/(far-near)^2) when the curve's squared-falloff bit
 * (def+0x19 & 1) is set. The interpolation uses int->float conversion of the
 * volume delta and truncates the result back to int (IntToFloat/FloatToInt in
 * the asm). NATIVE SHIM (no byte target; matching build uses asm). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", ComputeVolumeFalloff);
#else
s32 ComputeVolumeFalloff(SoundDef *def, float dist, float near, float far) {
    u8 *d = (u8 *)def;
    s32 nearVol = *(s32 *)(d + 0xC);
    s32 farVol  = *(s32 *)(d + 0x8);
    float num;   /* (far-dist) or (far-dist)^2 */
    float den;   /* (far-near) or (far-near)^2 */

    if (*(u8 *)(d + 0x19) & 1) {
        /* squared falloff */
        if (dist <= near) {
            return nearVol;
        }
        if (far <= dist) {
            return farVol;
        }
        num = (far - dist) * (far - dist);
        den = (far - near) * (far - near);
    } else {
        /* linear falloff */
        if (dist <= near) {
            return nearVol;
        }
        if (far <= dist) {
            return farVol;
        }
        num = far - dist;
        den = far - near;
    }
    /* asm order: num * (float)(nearVol - farVol) / den, then truncate */
    return farVol + (s32)(num * (float)(nearVol - farVol) / den);
}
#endif

/* Compute the falloff volume for emitter slot `slot` relative to listener `pos`:
 * measure the distance from `pos` to the camera, then evaluate the slot's
 * distance-falloff curve (curve params live at *(slot+0x8): near at +0x0, far at
 * +0x4). Returns the resulting volume level.
 * TODO(match): functional equivalent - not byte-exact; structurally exact (only
 * delta is the prologue frame size) - the original packs the two 8-byte saves
 * (s0,ra) into a 0x10 frame with ra at +0x8, while this cc1 rounds to a 0x20
 * frame with ra at +0x10 (the frame-rounding / outgoing-arg-reserve wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", ComputeEmitterVolume);
#else
s32 ComputeEmitterVolume(SoundEmitterSlot *slot, Vec4 *pos) {
    float dist = func_002837F8(pos, g_cameraPos);
    float *curve = *(float **)((u8 *)slot + 0x8);
    return ComputeVolumeFalloff(curve, dist, curve[0], curve[1]);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", ComputeEmitterPan);

__asm__(".extern D_1A7BA8, 16");
extern s32 D_1A7BA8; /* tuning input A (screen/scale base) */
__asm__(".extern D_1A7BA4, 16");
extern s32 D_1A7BA4; /* tuning input B */

/* Recompute the sky/sound layout header at g_listenerPosHistory+0x48..0x5C from
 * the two tuning inputs: scaled fractions of D_1A7BA8 plus a fixed 0x266.
 * NEAR-MISS (~72%): the original schedules the three products across the EE's
 * two integer multipliers (mult / mult1) in a pattern this cc1 won't reproduce
 * (it picks a different pipe assignment).  The C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E5698);
#else
void func_002E5698(void) {
    s32 a = D_1A7BA8;
    s32 *hdr = (s32 *)(g_listenerPosHistory + 0x48);
    hdr[0] = (a * 7) / 10;       /* 0x48 */
    hdr[1] = D_1A7BA4;           /* 0x4C */
    hdr[2] = a;                  /* 0x50 */
    hdr[3] = (a * 6) / 10;       /* 0x54 */
    hdr[4] = 0x266;              /* 0x58 */
    hdr[5] = (a * 0x19) / 32;    /* 0x5C */
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", InitSoundEmitterSystem);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", StartLevelMusicStream);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", UpdateSoundEmitters);

/* True if emitter slot `slotIndex` is currently owned by `owner` and in a
 * keyed-on/playing state (state 1 or 2).  A negative index means "no slot".
 * NEAR-MISS (~68%): functionally exact but cc1 stages the owner move + the
 * zero return into different branch delay slots and lowers the final boolean
 * via explicit branches (register-coloring / branch-lowering wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", IsMobySoundActive);
#else
s32 IsMobySoundActive(s32 owner, s32 slotIndex) {
    EmitterView *slot;
    if (slotIndex >= 0) {
        slot = EMITTER_VIEW(slotIndex);
        if (slot->owner == owner && (u8)(slot->state - 1) < 2) {
            return 1;
        }
    }
    return 0;
}
#endif

/* Request that emitter slot `slotIndex` stop.  If it is already pending-free
 * (state 7) it is freed immediately (state 0, owner/link cleared); states 0
 * (free) and 6 (already stopping) are left alone; any other active state is
 * moved to 4 (stop requested).  Negative index is a no-op. */
void StopSoundEmitter(s32 slotIndex) {
    EmitterView *slot;
    s32 state;
    if (slotIndex < 0) {
        return;
    }
    slot = EMITTER_VIEW(slotIndex);
    state = slot->state;
    if (state == 7) {
        slot->owner = 0;
        slot->f8C = 0;
        slot->state = 0;
        return;
    }
    if (state == 0 || state == 6) {
        return;
    }
    slot->state = 4;
}

extern u8 g_soundBankHandles[]; /* 0x189E00 - loaded 989snd bank handle array */

/* Find a free emitter slot (state byte == 0) for the moby `owner`.  Mobys that
 * match one of the two priority pointers (g_soundBankHandles+0x22B0 / +0x1240)
 * may use the full slot range (0x34); everything else is limited to 0x2A.
 * Returns the first free slot index, or 0x34 if none is free.
 * NEAR-MISS (~55%, functionally exact): the original folds the +0x20 base
 * offset into the %lo reloc addend and stages the limit constant in the beqz
 * delay slot (reloc-addend-fold + delay-slot scheduling walls).  The C is
 * faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", AllocVoiceHandleSlot);
#else
s32 AllocVoiceHandleSlot(Moby *owner) {
    u8 *bankBase = g_soundBankHandles + 0x20;
    s32 limit;
    s32 idx;

    limit = 0x34;
    if (owner == NULL ||
        (*(void **)(bankBase + 0x2290) != owner &&
         *(void **)(bankBase + 0x1220) != owner)) {
        limit = 0x2A;
    }

    idx = 0;
    if (limit != 0 && g_soundEmitterTable[0].state != 0) {
        for (idx = 1; idx < limit; idx++) {
            if (g_soundEmitterTable[idx].state == 0) {
                break;
            }
        }
    }
    return (idx != limit) ? idx : 0x34;
}
#endif

/* Allocate and arm a 3D sound emitter for sound definition `pSoundDef`.
 *
 * `flags` selects the emitter mode (bit 0x4 = "no-loop"/one-shot gate, bit 0x10
 * = skip the distance-volume cull), `ownerMoby` is the moby the emitter follows
 * (0 = world-anchored), `pPos` is an explicit world position used when there is
 * no owner, and `volScale` is the requested volume scale.
 *
 * Returns the allocated slot index, or -1 if the sound def fails its enable
 * gate, no voice slot is free, or the emitter is culled for being too quiet.
 *
 * Gate (asm-authoritative): when (flags & 4) == 0 the def's enable byte
 * (def+0x18) must be 0; when (flags & 4) != 0 it must be non-zero; otherwise -1.
 *
 * The emitter pool is addressed through g_listenerPosHistory (the table base
 * g_soundEmitterTable sits +0x70 past it), so the slot record begins at
 * e = g_listenerPosHistory + slot*0x70 and the per-slot fields use the same
 * absolute displacements the asm emits (def at +0x78, last-pan -1.0f at +0xB0,
 * sample id at +0x7C, voice cursor 0xFFFF at +0x7E, volScale at +0x80, owner
 * and link words 0 at +0x88/+0x8C, the 16-byte occlusion-ring scratch zeroed at
 * +0xA0, the position vec4 at +0x90, voice handle -1 at +0x70, flags at +0x75,
 * state 7 at +0x74, occlusion cursor 0 at +0x82, pitch at +0x84).
 *
 * Position: from ownerMoby+0x10 (with +1.0 added to the w lane at +0x98) when an
 * owner is given; else from pPos; else a zero vec4 with flags |= 0x11 (mark as
 * unpositioned + one-shot). Unless flags & 0x10, the emitter is culled when its
 * distance-attenuated volume is below 0x20. Pitch is def+0x14, or a random value
 * in [def+0x10, def+0x14) when the two differ. NATIVE SHIM (no byte target). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", StartSoundEmitter);
#else
extern s32 GetRandomInt(s32 n);
extern void func_00283638(void *dst); /* zero a 16-byte quadword */

s32 StartSoundEmitter(SoundDef *pSoundDef, s32 flags, Moby *ownerMoby,
                      Vec4 *pPos, s32 volScale) {
    u8 *def = (u8 *)pSoundDef;
    s32 slot;
    u8 *e;       /* g_listenerPosHistory + slot*0x70 */
    s32 pitch;

    /* enable gate */
    if (flags & 0x4) {
        if (def[0x18] == 0) {
            return -1;
        }
    } else {
        if (def[0x18] != 0) {
            return -1;
        }
    }

    slot = AllocVoiceHandleSlot(ownerMoby);
    if (slot >= 0x34) {
        return -1;
    }

    e = g_listenerPosHistory + slot * 0x70;

    *(void **)(e + 0x78) = pSoundDef;
    *(float *)(e + 0xB0) = -1.0f;
    *(s16 *)(e + 0x7C) = *(u16 *)(def + 0x1A);
    *(s16 *)(e + 0x7E) = (s16)0xFFFF;
    *(s16 *)(e + 0x80) = (s16)volScale;
    *(s32 *)(e + 0x88) = 0;
    *(s32 *)(e + 0x8C) = 0;
    func_00283638(e + 0xA0); /* zero the 16-byte scratch quad */

    /* position vec4 at e+0x90 */
    if (ownerMoby != NULL) {
        *(u_long128 *)(e + 0x90) = *(u_long128 *)((u8 *)ownerMoby + 0x10);
        *(float *)(e + 0x98) += 1.0f;
    } else if (pPos != NULL) {
        *(u_long128 *)(e + 0x90) = *(u_long128 *)pPos;
    } else {
        func_00283638(e + 0x90);
        flags |= 0x11;
    }

    /* distance-volume cull (unless flagged off) */
    if (!(flags & 0x10)) {
        if (ComputeEmitterVolume(e + 0x70, e + 0x90) < 0x20) {
            return -1;
        }
    } else {
        if (volScale < 0x20) {
            return -1;
        }
    }

    *(u8 *)(e + 0x75) = (u8)flags;
    *(u8 *)(e + 0x74) = 7;
    *(s16 *)(e + 0x82) = 0;

    /* pitch: fixed def+0x14, or random in [def+0x10, def+0x14) */
    pitch = *(s32 *)(def + 0x14);
    if (*(s32 *)(def + 0x14) != *(s32 *)(def + 0x10)) {
        pitch = GetRandomInt(*(s32 *)(def + 0x14) - *(s32 *)(def + 0x10)) +
                *(s32 *)(def + 0x10);
    }

    *(s32 *)(e + 0x70) = -1;
    *(s32 *)(e + 0x84) = pitch;

    return slot;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", PlayMobySound);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", PlaySoundFromClassBank);

/* Play one of the global/common (UI/menu/system) sounds by index.
 *
 * `soundIdx` selects an entry in the global sound-def pool g_globalSoundDefsPtr
 * (a 0x20-byte stride array bounded by g_nGlobalSoundDefs); `posOverride` is an
 * optional explicit position and `owner` the owning moby. Returns the emitter
 * slot index, or -1 if the pool is unset, the index is out of range, or no
 * emitter could be started.
 *
 * Delegates to StartSoundEmitter with flags 0, pPos = 0, volScale = 0x400; on
 * success it records the source sound index (s16 at slot+0x7E) and owner
 * (s32 at slot+0x88) into the slot record (addressed off g_listenerPosHistory,
 * +0x70 ahead of g_soundEmitterTable). NATIVE SHIM (no byte target). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", PlayGlobalSound);
#else
extern void *g_globalSoundDefsPtr; /* 0x1B162C - global sound-def pool */
extern s32 g_nGlobalSoundDefs;     /* 0x1A8BBC - count of global sound defs */

s32 PlayGlobalSound(s32 soundIdx, s32 posOverride, s32 owner) {
    u8 *pool = (u8 *)g_globalSoundDefsPtr;
    s32 slot;
    u8 *e;

    if (pool == NULL) {
        return -1;
    }
    if (soundIdx >= g_nGlobalSoundDefs) {
        return -1;
    }

    slot = StartSoundEmitter(pool + soundIdx * 0x20, posOverride, (void *)owner,
                             NULL, 0x400);
    if (slot >= 0) {
        e = g_listenerPosHistory + slot * 0x70;
        *(s16 *)(e + 0x7E) = (s16)soundIdx;
        *(s32 *)(e + 0x88) = owner;
    }
    return slot;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E6D28);

/* Flag emitter slot `slotIndex` as positioned (set flags bit 0x40) and copy the
 * 16-byte spatial quad from `src` into the slot's 0xA0 field; returns 1.
 * NEAR-MISS (~80%): the original materializes the quad destination base
 * (g_listenerPosHistory+0xA0) into its own register and uses displacement 0 for
 * the sq, while cc1 keeps one base and folds +0xA0 into the store displacement
 * (addressing-distribution wall). The C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E6D38);
#else
s32 func_002E6D38(s32 slotIndex, u_long128 *src) {
    EmitterView *slot = EMITTER_VIEW(slotIndex);
    slot->flags |= 0x40;
    slot->quadA0 = *src;
    return 1;
}
#endif

/* Set the pitch field of emitter slot `slotIndex`; returns 1.
 * NEAR-MISS (~80%): the original carries a second entry point (alabel
 * func_002E6D78) with a leading dead `li v0,1; nop` pair that single-function
 * C cannot reproduce. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E6D70);
#else
s32 func_002E6D70(s32 slotIndex, s32 pitch) {
    EmitterView *slot = EMITTER_VIEW(slotIndex);
    slot->pitch = pitch;
    return 1;
}
#endif

/* Hook table anchored at g_listenerPosHistory+0x1730: a count followed by a
 * pointer to `count` entries of 0x90 bytes, each with a callback at +0x4. */
typedef struct EmitterHook {
    /* 0x00 */ u8  pad00[0x4];
    /* 0x04 */ void (*callback)(void);
    /* 0x08 */ u8  pad08[0x88];
} EmitterHook;

/* Run every registered emitter hook callback in order, skipping null slots.
 * NEAR-MISS (~82%, structurally identical): the original (later SN cc1) packs
 * its 4 callee saves 8-byte while this cc1 packs them 16-byte (the 8-byte-packed
 * save wall) -> only the save offsets + frame size differ.  The C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E6D98);
#else
void func_002E6D98(void) {
    u8 *base = g_listenerPosHistory;
    s32 i;
    s32 offset;
    for (i = 0, offset = 0; i < *(s32 *)(base + 0x1730); i++, offset += 0x90) {
        EmitterHook *hook =
            (EmitterHook *)(*(u8 **)(base + 0x1734) + offset);
        if (hook->callback != NULL) {
            hook->callback();
        }
    }
}
#endif

/* 989snd service calls used by the teardown flush. */
extern void func_00133230(void);
extern void func_00132AC8(void);
extern s32  snd_Pump(void);

/* StopAllSoundEmitters: level-teardown audio flush. Drain the 989snd ring
 * (func_00133230 + func_00132AC8 + snd_Pump until idle), then zero the listener
 * position ring (4 vec4 + the count word at +0x40) and reset all 52 voice slots
 * (stride 0x70) by clearing each slot's state word (+0x70) and flag byte (+0x74),
 * all based at g_listenerPosHistory. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", StopAllSoundEmitters);
#else
void StopAllSoundEmitters(void) {
    u8 *p;
    s32 i;
    func_00133230();
    snd_Pump();
    func_00132AC8();
    while (snd_Pump() != 0) {
    }
    for (i = 0; i < 0x40; i += 4) {
        *(s32 *)(g_listenerPosHistory + i) = 0;
    }
    *(s32 *)(g_listenerPosHistory + 0x40) = 0;
    for (p = g_listenerPosHistory; p < g_listenerPosHistory + 0x16C0; p += 0x70) {
        *(s32 *)(p + 0x70) = 0;
        *(u8 *)(p + 0x74) = 0;
    }
}
#endif

/* Store `handle` into the voice-handle word of the emitter slot, if non-NULL.
 * The slot arrives as a 32-bit value sign-extended into a 64-bit register. */
void func_002E6EC0(s32 handle, long slotAddr) {
    SoundEmitter *slot = (SoundEmitter *)slotAddr;
    if (slot != NULL) {
        slot->voiceHandle = handle;
    }
}

/* The unit tail (0x2E6F50..0x2F003F) is the spimdisasm c-mode tail-fusion blob:
 * functions reached only by j / data-ref (no jal) that spimdisasm cannot promote
 * to their own symbols, so they fuse into one INCLUDE_ASM. It is islanded as the
 * asm segment text/1E6ED0 to keep the objdiff count honest. */

#include "common.h"

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

typedef unsigned long u_long128 __attribute__((mode(TI)));

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", DrawSkyShellsFixedSpin);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", ComputeListenerOcclusionProbe);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", CastEmitterOcclusionRay);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", ComputeVolumeFalloff);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", ComputeEmitterVolume);

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
s32 AllocVoiceHandleSlot(void *owner) {
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", StartSoundEmitter);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", PlayMobySound);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", PlaySoundFromClassBank);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", PlayGlobalSound);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", StopAllSoundEmitters);

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

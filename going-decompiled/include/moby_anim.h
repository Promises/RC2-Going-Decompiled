#ifndef MOBY_ANIM_H
#define MOBY_ANIM_H

#include "common.h"

/* Moby skeletal/frame animation - the class anim-sequence table.
 *
 * Every loaded moby class header carries an array of animation SEQUENCES (walk,
 * idle, attack, ...). The header holds:
 *   hdr +0x0C  u8        sequence count          (FixupMobyClassHeader uses it)
 *   hdr +0x48  AnimSeq*  ptr array [seqCount]    (rebased by FixupMobyClassHeader)
 * A moby plays one sequence at a time via the anim block at moby+0x40 (animFrame
 * +0x40, animFrameNext +0x41, animSeq +0x42, animSeqNext +0x43, animTime +0x44,
 * animRate +0x48, animRate2/frameDur cache +0x4C, animFramePtr +0x58/+0x5C).
 *
 * animSeq == 0xFF is the special "procedural" sequence: the current pose is baked
 * into g_proceduralAnimFrames by BakeMobyProceduralFrame (used for cross-fade
 * blends between sequences - see SetMobyAnimSequenceBlended).
 *
 * Cross-verified against:
 *   UpdateMobyAnimation      0x2A13E0 (advance time, step frames, fire sound events)
 *   ResolveMobyAnimFramePtrs 0x2A01C8 (frame ptr + loop-sound resolve)
 *   FixupMobyClassHeader     0x293DA8 (offset->ptr rebase of the seq array)
 *   SetMobyAnimSequence      0x2A8200 (seq/frame set + clamp)
 *
 * NOTE - ILP32: AnimSequence contains pointer fields (pBlock14 @ +0x14, the
 * frame-ptr array @ +0x1C, and the class-header +0x48 ptr array), so the struct
 * lays out at these offsets only under a 4-byte-pointer compile (the PS2 EE
 * target and the native -m32 build). Every offset assert that sits at or past the
 * first pointer is therefore guarded for ILP32; on a 64-bit host the pointers
 * widen the record and the asserts are inert. That is expected, not a bug.
 */

/* One animation sequence in a class header's +0x48 ptr array.
 * Fixed header is 0x1C bytes; the variable part follows:
 *   frame ptr array : void*[frameCount]                          @ +0x1C
 *   sound events    : AnimSoundEvent[soundEventCount]            @ +0x1C + frameCount*4
 */
typedef struct AnimSequence {
    u8   _pad00[0x10];      /* +0x00 unverified header (counts/section sizes used by
                               the VU pose baker BakeMobyProceduralFrame:
                               +0x06/+0x0A/+0x0E shorts = per-section element counts) */
    u8   frameCount;        /* +0x10 number of frames in this sequence            CONFIRMED */
    u8   loopSoundIdx;      /* +0x11 looping-anim sound index -> moby+0x6C
                               (ResolveMobyAnimFramePtrs / SetMobyAnimSequence)    CONFIRMED */
    u8   soundEventCount;   /* +0x12 number of trailing AnimSoundEvent entries
                               -> moby+0x6E; UpdateMobyAnimation event loop count  CONFIRMED */
    u8   holdByte;          /* +0x13 hold/replay flag; ==0xFF lets a procedural
                               (seq 0xFF) anim re-fire in UpdateProceduralAnimSlots
                               and gates the loop in UpdateMobyAnimation           CONFIRMED */
    void *pBlock14;         /* +0x14 rebased block ptr (FixupMobyClassHeader);
                               role not traced                                     UNCONFIRMED */
    f32  frameDuration;     /* +0x18 default per-frame duration; when 0 the per-
                               frame override (frame data word 0) is used instead  CONFIRMED */
    /* +0x1C: void *framePtr[frameCount]  - each rebased; frame data starts with a
       f32 duration override. Followed by AnimSoundEvent[soundEventCount].         CONFIRMED */
    void *framePtrs[1];     /* +0x1C flexible frame-ptr array                      CONFIRMED */
} AnimSequence;

/* A keyed sound trigger inside a sequence, packed 4 bytes each, in the array that
 * follows the frame-ptr array (base = seq + 0x1C + frameCount*4). UpdateMobyAnimation
 * fires PlayMobySound(soundIdx,0,moby) when the played frame window crosses the
 * event's frame position. Positions are in 1/16-frame units.                     CONFIRMED */
typedef struct AnimSoundEvent {
    u16  soundIdx;          /* +0x00 sound index passed to PlayMobySound           CONFIRMED */
    u16  framePos16ths;     /* +0x02 trigger position in 1/16-frame units          CONFIRMED */
} AnimSoundEvent;

/* Pointer-free prefix offsets - hold on any host. */
_Static_assert(__builtin_offsetof(AnimSequence, frameCount)      == 0x10, "frameCount");
_Static_assert(__builtin_offsetof(AnimSequence, loopSoundIdx)    == 0x11, "loopSoundIdx");
_Static_assert(__builtin_offsetof(AnimSequence, soundEventCount) == 0x12, "soundEventCount");
_Static_assert(__builtin_offsetof(AnimSequence, holdByte)        == 0x13, "holdByte");
_Static_assert(__builtin_offsetof(AnimSoundEvent, soundIdx)      == 0x00, "soundIdx");
_Static_assert(__builtin_offsetof(AnimSoundEvent, framePos16ths) == 0x02, "framePos16ths");

/* ILP32-only: these offsets sit at/after the +0x14 pointer, so they are correct
 * only when pointers are 4 bytes (EE target / -m32). */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(__builtin_offsetof(AnimSequence, frameDuration)   == 0x18, "frameDuration");
_Static_assert(__builtin_offsetof(AnimSequence, framePtrs)       == 0x1C, "framePtrs");
#endif

#endif /* MOBY_ANIM_H */

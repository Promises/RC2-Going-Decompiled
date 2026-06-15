#ifndef SOUND_EMITTER_H
#define SOUND_EMITTER_H

#include "common.h"

/* 3D positional-sound (sound-emitter) subsystem.
 *
 * Recovered from the USA v2.00 (SCUS_972.68) emitter cluster: the public play
 * entry points (StartSoundEmitter 0x2E6908, PlayMobySound 0x2E6B30,
 * PlaySoundFromClassBank 0x2E6BD8, PlayGlobalSound 0x2E6C90), the per-frame
 * driver UpdateSoundEmitters (0x2E5900), the math helpers (ComputeEmitterVolume
 * 0x2E54D8, ComputeEmitterPan 0x2E5520, CastEmitterOcclusionRay 0x2E5340) and
 * the two 989snd voice callbacks (OnEmitterVoiceKeyedOn 0x2E6ED8,
 * OnEmitterVoiceEnded 0x2E6F20).
 *
 * This layer sits ABOVE the 989snd low-level driver: it owns a fixed pool of
 * emitter slots, follows each emitter's owner moby per frame, computes distance
 * attenuation + stereo pan + occlusion relative to the listener (camera at
 * g_flCameraPos 0x1B52C0 / camera matrix 0x1B54F0), and issues 989snd voice
 * commands (deferred key-on, snd_SetVoiceVolumeRamp, key-off).
 *
 * One slot is exactly 0x70 bytes; the pool is g_soundEmitterTable (0x1886D0)
 * with 52 (0x34) slots. Two slots are reserved for special emitters (dialog),
 * leaving 0x2A generic. Offsets are CONFIRMED from StartSoundEmitter (which
 * stamps every field on alloc) and UpdateSoundEmitters; field *semantics* are
 * CONFIRMED except where marked. */

/* Slot state machine (+0x04). Driven by StartSoundEmitter / UpdateSoundEmitters
 * / StopSoundEmitter and the 989snd callbacks. */
enum SoundEmitterState {
    SE_STATE_FREE        = 0, /* slot unused */
    SE_STATE_PLAYING     = 1, /* keyed on, voice handle pending the callback   */
    SE_STATE_ACTIVE      = 2, /* OnEmitterVoiceKeyedOn confirmed - voice live  */
    SE_STATE_STOP_NOW    = 4, /* StopSoundEmitter requested immediate stop     */
    SE_STATE_KEYING_OFF  = 6, /* key-off issued, awaiting OnEmitterVoiceEnded  */
    SE_STATE_PENDING_KEY = 7  /* StartSoundEmitter set; deferred key-on next frame */
};

/* Flag bits (+0x05), passed in by the play entry points. */
#define SE_FLAG_NON3D       0x10 /* skip 3D distance attenuation (fixed volume)   */
#define SE_FLAG_OFFSET_MODE 0x40 /* +0x30 holds a local offset transformed by the
                                    owner moby matrix (+0xC0) instead of tracking
                                    the moby origin directly */
/* 0x08 seen as a per-slot disable/skip bit in the driver follow path.            */

typedef struct SoundEmitterSlot {
    s32 voiceHandle;     /* +0x00 989snd voice handle; -1 unkeyed, 0 = ended      */
    u8  state;           /* +0x04 enum SoundEmitterState                          */
    u8  flags;           /* +0x05 SE_FLAG_* bits                                  */
    u8  pad06[2];        /* +0x06                                                 */
    void *pDef;          /* +0x08 sound-def ptr (def+0x0/+0x4 falloff radii,
                                   +0x10..+0x14 pitch range, +0x18 enabled gate,
                                   +0x19 flags, +0x1A sampleId, +0x1C bankIndex)  */
    u16 sampleId;        /* +0x0C copy of def+0x1A (989snd sample id)             */
    u16 soundIdx;        /* +0x0E source sound index in its bank; 0xFFFF if none  */
    s16 volScale;        /* +0x10 caller volume scale (0x400 = unity, >>10 applied)*/
    u16 occlCursor;      /* +0x12 write cursor into occlRing, advanced % 0x24     */
    s32 pitch;           /* +0x14 randomized pitch (def+0x10..+0x14 range)        */
    void *pOwnerMoby;    /* +0x18 owner moby; emitter follows moby origin (+0x10) */
    u32 link;            /* +0x1C aux/link word, cleared with owner on free       */
    f32 pos[4];          /* +0x20 world position vec4 (z at +0x28 used for the
                                   underwater depth volume cut)                   */
    f32 localOffset[4];  /* +0x30 SE_FLAG_OFFSET_MODE local offset (moby space)   */
    f32 lastPanAngle;    /* +0x40 last computed pan angle; -1.0 sentinel on start */
    u8  occlRing[36];    /* +0x44 per-slot occlusion-ray history ring (36 = 0x24
                                   samples); occluded fraction halves the volume  */
    u8  pad68[8];        /* +0x68 padding to 0x70                                 */
} SoundEmitterSlot; /* sizeof == 0x70 */

/* --- Pool / listener / bank state (see symbol_addrs/usa) ----------------------
 * g_soundEmitterTable       0x1886D0  SoundEmitterSlot[52] - emitter pool base
 * g_listenerPosHistory      0x188660  vec4[4] ring of listener positions/frame
 * g_listenerPosHistoryIdx   0x1886A0  write index 0..3 into the ring
 * g_listenerOcclusionProbes 0x189DA0  vec4[6] jittered probe pts around listener
 * g_soundBankHandles        0x189E00  s32[] loaded 989snd bank handles (def+0x1C)
 * g_globalSoundDefsPtr      0x1B162C  -> global sound-def pool (PlayGlobalSound)
 * g_nGlobalSoundDefs        0x1A8BBC  count bound for PlayGlobalSound (0x14)
 * g_flCameraPos             0x1B52C0  listener position (camera) - attenuation
 * camera matrix             0x1B54F0  listener orientation - pan azimuth
 * ------------------------------------------------------------------------------ */

#endif /* SOUND_EMITTER_H */

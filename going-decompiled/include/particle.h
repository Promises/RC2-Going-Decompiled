#ifndef PARTICLE_H
#define PARTICLE_H

#include "common.h"

/* Particle / visual-FX subsystem (render layer 0x20).
 *
 * Recovered from the USA v2.00 (SCUS_972.68) particle pool: AllocParticle
 * (0x2B9F50), the ~33 SpawnXxxParticle helpers (0x2BB7C0..0x2C8D48), and the
 * 111-slot per-type update-handler table dispatched by UpdateParticles
 * (0x2BA2B0). Layout below is cross-verified against AllocParticle (which
 * zeroes the whole record then stamps bTypeId at +0), SpawnWaterRippleParticle
 * (type 0x2d), and SpawnRainSplashParticle (type 1). All field semantics are
 * CONFIRMED except where noted; offsets/sizes are CONFIRMED.
 *
 * One record is exactly 0x40 bytes; the pool is 2048 slots (g_pParticlePool),
 * managed by a flat bit-allocation bitmap (g_particleAllocBitmap). */

typedef struct ParticleRecord {
    u8  bTypeId;       /* +0x00 effect type 0..0x6E - indexes g_particleUpdateHandlers */
    u8  bFlags;        /* +0x01 state bits; 0x80 = dead (set by FreeParticle), draw submode in low bits */
    u8  bTexIdx;       /* +0x02 texture index into g_particleTexTable (from the effect def) */
    u8  bBlendCode;    /* +0x03 GS blend/alpha code (e.g. 0x44 splash, 0x48 ripple) */
    u32 dwColorRGBA;   /* +0x04 packed colour: alpha<<24 | RGB */
    u8  bAngle;        /* +0x08 sprite rotation (256 = full turn, via g_particleSinTable) */
    u8  bFadeNibbles;  /* +0x09 packed fade-in/fade-out timing nibbles */
    u16 wLife;         /* +0x0A remaining life in frames */
    f32 flSize;        /* +0x0C sprite size (4096-unit fixed world scale) */
    f32 pPos[4];       /* +0x10 world position xyz (w padding) */
    f32 pPosBOrVel[4]; /* +0x20 velocity, or a second position - per-type meaning */
    f32 pTypeParams[4];/* +0x30 per-type scratch; +0x3C can hold a stored update callback
                          (DispatchParticleStoredHandler jr's to it for types 0x55/0x5d) */
} ParticleRecord; /* sizeof == 0x40 */

/* --- Pool / allocator state (see symbol_addrs/usa) ----------------------- */
/* g_pParticlePool         0x1B1D10  ParticleRecord* - 2048-slot pool base    */
/* g_particleFreeCursor    0x1B1D14  next free-slot scan cursor               */
/* g_particleHighWaterIdx  0x1B1D18  highest in-use slot (pmaxw)              */
/* g_particleLiveCount     0x1B1D1C  live particle count                      */
/* g_particleAllocBitmap   0x1F1CC0  2048-bit allocation bitmap               */
/* g_particleUpdateHandlers 0x1F22C0 111-slot per-type update fn table        */
/* g_particleEffectDefs    0x1F24C0  128 effect-def ptrs (per-level blob)     */
/* g_particleTexTable      0x1F1EC0  8 bytes/tex: VRAM word addr + CLUT addr  */

#endif /* PARTICLE_H */

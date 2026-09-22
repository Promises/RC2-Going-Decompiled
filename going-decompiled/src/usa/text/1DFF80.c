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
__asm__(".extern g_pSkySegmentCloseTag, 16");
extern u8 *g_pSkySegmentCloseTag;  /* 0x1B2064 - sky segment tail/close tag */
extern void FlushPendingTexUploads(void);     /* 0x29DF18 - drain queued GS tex uploads */
extern void AppendTexFlushDefaultTex0(void);  /* 0x2FD5D8 - TEXFLUSH + default TEX0_1 packet */
__asm__(".extern g_pShrubSegmentOpenTag, 16");
extern u8 *g_pShrubSegmentOpenTag; /* 0x1B2034 - shrub segment head tag */
extern s32 g_shrubVramPeak;        /* 0x1B203C - shrub texture VRAM high-water mark */
extern s32 UploadShrubTextures(s32 vramAllocCursor); /* 0x2E37A0 - returns VRAM used */
extern char D_1ABE10[];            /* VRAM-overflow warning fmt string */
extern void DebugPrintStub(const char *fmt, ...);    /* 0x26FEC8 - retail no-op */
extern s32  g_shrubVisibleClassList[]; /* 0x213FD0 - visible shrub class list, -1 term */
extern u8  *g_shrubClassTable[];       /* 0x2125D0 - shrub class record ptr array */
extern s16  g_shrubTexVramTable[];     /* 0x213AD0 - texId -> {tbp_lo,tbp_hi}, stride 4 */
extern u8  *g_pShrubInstanceArray;     /* 0x1B2018 - shrub instance array base, stride 0x20 */
extern u8   g_gsScreenContext[];       /* 0x1A6480 - GS screen context; dims at +0x150/+0x152 */
/* 0x2FD3F0 - append a GIF A+D reg-write. The DATA is a 64-bit GS register value.
 * The only in-unit caller is #else-arm (func_002E0650 / EmitMobyGlowPackets), so
 * the matching-build decl is byte-irrelevant; keep it s32 (untouched) and widen
 * the value to u64 for the native/#else build only, so EmitMobyGlowPackets can
 * pack bits >=32 (0x8000<<22/<<24) without truncation. */
#ifdef TARGET_NATIVE
extern void AppendGsRegPacket(s32 regId, u64 value);
#else
extern void AppendGsRegPacket(s32 regId, s32 value);
#endif
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

/* NOTE: OnEmitterVoiceKeyedOn / OnEmitterVoiceEnded (real addrs 0x2E6ED8 /
 * 0x2E6F20) live at the END of this unit in the original; they are defined in
 * address order down by func_002E6EC0, NOT here. cc1 emits functions in source
 * order, so defining them at the top would link them at the unit start and shove
 * func_002E0000..end +0x78 (the 41K C-only pointer residual). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0000);

/* Format a level-select entry's display name into `dst`. A jump table
 * (jtbl_0026CEB0_text, 25 cases on id-2 in 0..0x18) selects the format: the
 * "normal" ids {2,5,0xA,0x17,0x18,0x19,0x1A} use the name-only format D_1ABA38;
 * every other id (including out-of-range) uses the "special" format D_1AB9B8
 * with a GetLocalizedString(0xB44) prefix. The level name is
 * GetLocalizedString(g_levelSelectEntries[id]) (stride 8); both go through the
 * SDK sprintf func_00115DA8. Returns dst.
 * The 25-case jtbl->flag mapping was extracted from jtbl_0026CEB0_text (0x2E0054
 * => normal, 0x2E005C => special) — the faithful #else the prior park-note asked
 * for. Match-walled by the jtbl reloc (engine-2.96); matching arm stays INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0010);
#else
extern u8    g_levelSelectEntries[];
extern char  D_1AB9B8[];   /* "special" format: prefix + name */
extern char  D_1ABA38[];   /* "normal" format: name only */
extern char *GetLocalizedString(s32 id);
extern void  func_00115DA8(char *dst, const char *fmt, ...);

char *func_002E0010(char *dst, s32 id) {
    s32 special = 1;
    if ((u32)(id - 2) < 0x19) {
        switch (id) {
        case 0x2: case 0x5: case 0xA:
        case 0x17: case 0x18: case 0x19: case 0x1A:
            special = 0;
            break;
        default:
            special = 1;
            break;
        }
    }
    if (special) {
        char *prefix = GetLocalizedString(0xB44);
        char *name   = GetLocalizedString(*(s32 *)(g_levelSelectEntries + id * 8));
        func_00115DA8(dst, D_1AB9B8, prefix, name);
    } else {
        char *name = GetLocalizedString(*(s32 *)(g_levelSelectEntries + id * 8));
        func_00115DA8(dst, D_1ABA38, name);
    }
    return dst;
}
#endif

/* Request a transition into game state 7 (cinematic-hide), stashing the two
 * caller args for the deferred state-enter action, clearing the pre-particle
 * hook count, re-arming the scene cast helper, then marking every moby in the
 * table hidden (set bit 0x80 in the u16 mode word at moby+0x34).  `argA`/`argB`
 * are the state-change arguments cached at g_pendingStateArgA/B (read back when
 * state 7 is committed).  Counterpart of UnhideAllMobysAndPopState.
 * NEAR-MISS (~85%, structurally identical): cc1 lowers the table-walk to a plain
 * `bnez` where the original uses a branch-likely (`bnel`) that hoists the moby
 * mode-word load into the delay slot (the branch-likely lowering wall).  Also a
 * 2-callee-save (s0,s1) 8-byte-packed prologue this cc1 rounds to 16-byte.  The
 * C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", HideAllMobysAndPushState);
#else
extern u8 *g_mobyTableBase;
extern u8 *g_mobyTableEnd;
extern void func_002857C8(s32 a, s32 b, s32 c);
extern void RequestGameStateChange(s32 stateId, s32 push, s32 c, s32 d);
extern s32 g_fxHooksPreCount;          /* 0x1B1588 - pre-particle hook count   */
extern s32 g_pendingStateArgA;         /* 0x1ABE00 - stashed state-change argA  */
extern s32 g_pendingStateArgB;         /* 0x1ABE04 - stashed state-change argB  */

void HideAllMobysAndPushState(s32 argA, s32 argB) {
    u8 *moby;

    RequestGameStateChange(7, 1, 0, 0);
    g_pendingStateArgA = argA;
    g_pendingStateArgB = argB;
    g_fxHooksPreCount = 0;
    func_002857C8(0, 0, 0);

    for (moby = g_mobyTableBase; moby < g_mobyTableEnd; moby += 0x100) {
        *(u16 *)(moby + 0x34) |= 0x80;
    }
}
#endif

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

/* func_002E0210: build a save-state snapshot into g_pLevelSelectListEntries[0xF68..]
 * (early-out when g_soundBankHandlesBlk[0x22B4]==4). PARKED #70 (#else not confident):
 * gated behind an intricate 0x7FF-iteration nibble-PACKING loop (per-entry andi/srl/
 * sra bit-field writes into g_platinumBoltFlags-indexed nibbles, with a value==7 skip)
 * followed by a ~30-field snapshot copy from named globals — the field copy is
 * mechanical but the nibble packer is high silent-bug risk (exact bit/index mapping).
 * (Prior note mislabeled it as vector math — it's a save-snapshot builder.) */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0210);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0448);

/* Build a scaled orientation + a perpendicular unit axis from a direction `src`
 * (used by the glow poser): copy src to D_001B1E90[0x140], store src*scale at
 * [0x160], then form a reference axis from the abs components (permuted so the
 * smallest-magnitude component is placed to keep it least-parallel to src),
 * cross it with the direction, and renormalize to unit length at [0x150].
 * Traced fully (func_00283628 = vabs.xyzw; the bc1fl min-abs permutation mapped
 * op-for-op) -> faithful #else; engine-2.96 so the matching arm stays INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0458);
#else
extern u8   D_001B1E90[];
extern void func_00283628(void *dst, void *src);           /* vabs.xyzw (component-wise abs) */
extern void Vec4ScaleVu0(void *dst, f32 scale, void *src);
extern void Vec3CrossVu0(void *dst, void *a, void *b);
extern void Vec3RescaleToLenVu0(void *dst, f32 len, void *src);

void func_002E0458(void *src, f32 scale) {
    f32   absv[4];
    u8   *dir  = D_001B1E90 + 0x140;
    f32  *axis = (f32 *)(D_001B1E90 + 0x150);
    f32   ax, ay, az;

    ((u64 *)dir)[0] = ((u64 *)src)[0];    /* lq/sq: D[0x140] = *src (128-bit copy) */
    ((u64 *)dir)[1] = ((u64 *)src)[1];
    func_00283628(absv, src);             /* absv = |src| per component */
    Vec4ScaleVu0(D_001B1E90 + 0x160, scale, dir);   /* D[0x160] = dir * scale */

    ax = absv[0]; ay = absv[1]; az = absv[2];
    if (ax < ay) {
        if (ax < az) { axis[0] = ax; axis[1] = az; axis[2] = ay; }
        else         { axis[0] = ay; axis[1] = ax; axis[2] = az; }
    } else {
        if (ay < az) { axis[0] = az; axis[1] = ay; axis[2] = ax; }
        else         { axis[0] = ay; axis[1] = ax; axis[2] = az; }
    }

    Vec3CrossVu0(axis, axis, dir);            /* axis = axis x dir */
    Vec3RescaleToLenVu0(axis, 1.0f, axis);    /* normalize to unit length */
}
#endif

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

/* func_002E05C0: PARKED #70 (#else not confident) — emits a GIF/DMA packet into
 * g_frameDmaCursor (GIFtag 0x30000003 / 0x50000003 / 0x13000000; indexes D_261CF0 by
 * arg2*0x30 and the gp-rel D_1ABE08 by arg1) then tail-calls func_002E1A58 — a large
 * handwritten DMA/VU helper whose arity is ambiguous, so the call args can't be cleanly
 * resolved. Won't guess. Needs func_002E1A58's signature + the D_261CF0/D_1ABE08 table
 * types traced before a faithful #else. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E05C0);

/* Sibling of func_002E07F8: builds the same 32-scanline-strip framebuffer-fill
 * GIF packet, but first appends a standalone GS register write (AppendGsRegPacket),
 * uses a fixed fill colour (0x7F808080) instead of a caller value, and leaves the
 * chain open (advances the cursor without emitting a closing DMAtag). Screen dims
 * from g_gsScreenContext; nRows = h/32. GS AD register-data kept as documented hex.
 * Handwritten-style packet build; portable-only #else. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0650);
#else
void func_002E0650(void) {
    u8  *ctx     = g_gsScreenContext;
    s32  screenH = *(s16 *)(ctx + 0x150);
    s32  screenV = *(s16 *)(ctx + 0x152);
    s32  nRows   = screenH / 32;
    u32 *dmatag;
    u64 *ad;
    s32  i;

    AppendGsRegPacket(0x42, 0x64);

    /* opening DMAtag: qwc = 5 reg-writes + nRows quads */
    dmatag = (u32 *)g_frameDmaCursor;
    dmatag[0] = (nRows + 5) | 0x10000000;
    dmatag[1] = 0;
    dmatag[2] = 0;
    dmatag[3] = (nRows + 5) | 0x50000000;
    g_frameDmaCursor = (u8 *)dmatag + 0x10;

    /* GIFtag(A+D) + AD register-write list; each AD entry = {data @+0, GS reg @+8} */
    ad = (u64 *)g_frameDmaCursor;
    ad[0] = ((u64)0x8000 << 45) | 1;                        /* GIFtag: NLOOP=1, NREG=1 */
    ad[1] = 0xE;                                            /*   descriptor: A+D */
    ad[2] = 0x31001;                 ad[3] = 0x47;         /* AD: data -> GS reg */
    ad[4] = ((u64)0x9000 << 46) | 0x8001; ad[5] = 0x10;   /* AD: data -> GS reg */
    ad[6] = 0x146;                   ad[7] = 0x7F808080;   /* AD: data (fill colour) -> GS reg */
    ad[8] = (u64)(nRows | 0x8000) | ((u64)0x9000 << 46);
    ad[9] = 0x44;                                           /* AD: data -> GS reg */

    /* one screen-spanning sprite quad per strip; XY packed 16.16, Y band += 0x200 */
    if (nRows > 0) {
        s32  vEdge    = screenV << 3;
        s32  hEdge    = screenH << 3;
        u64  xLeftHi  = (u64)(u32)(0x8000 - vEdge)          << 16;
        u64  xRightHi = (u64)(u32)((screenV << 3) + 0x7FF0) << 16;
        s32  yTop     = 0x8000 - hEdge;
        s32  yBot     = 0x8200 - hEdge;
        u64 *quad     = (u64 *)((u8 *)dmatag + 0x60);

        for (i = 0; i < nRows; i++) {
            quad[0] = (u32)yTop | xLeftHi;
            quad[1] = (u32)yBot | xRightHi;
            quad += 2;
            yTop += 0x200;
            yBot += 0x200;
        }
    }

    /* leave the chain open: just advance past the reg-writes + quad rows */
    g_frameDmaCursor = (u8 *)g_frameDmaCursor + (nRows << 4) + 0x50;
}
#endif

/* Build the GIF/DMA packet that fills the framebuffer in horizontal strips (one
 * screen-wide sprite quad per 32 scanlines) — the screen-clear path. Emits an
 * opening DMAtag (qwc = 5 register-writes + nRows quad rows), a GIFtag(A+D) with
 * its GS register setup list, then one sprite quad per strip (XY packed 16.16,
 * origin 0x8000, Y band stepped 0x200 per row), and a closing DMAtag. Screen
 * dims come from g_gsScreenContext (+0x150 h, +0x152 v). GS AD register-data
 * words are kept as documented hex (not guessed register names). Handwritten-
 * style packet build; portable-only #else. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E07F8);
#else
void func_002E07F8(u64 arg0) {
    u8  *ctx     = g_gsScreenContext;
    s32  screenH = *(s16 *)(ctx + 0x150);
    s32  screenV = *(s16 *)(ctx + 0x152);
    s32  nRows   = screenH / 32;           /* one filled sprite strip per 32 scanlines */
    u32 *dmatag  = (u32 *)g_frameDmaCursor;
    u64 *ad;
    s32  i;

    /* opening DMAtag: id=CNT (0x10000000), qwc = 5 reg-writes + nRows quads */
    dmatag[0] = (nRows + 5) | 0x10000000;
    dmatag[1] = 0;
    dmatag[2] = 0;
    dmatag[3] = (nRows + 5) | 0x50000000;
    g_frameDmaCursor = (u8 *)dmatag + 0x10;

    /* GIFtag(A+D) + AD register-write list; each AD entry = {data @+0, GS reg @+8} */
    ad = (u64 *)g_frameDmaCursor;
    ad[0] = ((u64)0x8000 << 45) | 1;                        /* GIFtag: NLOOP=1, NREG=1 */
    ad[1] = 0xE;                                            /*   descriptor: A+D */
    ad[2] = 0x3D801;              ad[3] = 0x47;             /* AD: data -> GS reg */
    ad[4] = ((u64)0x9000 << 46) | 1; ad[5] = 0x10;         /* AD: data -> GS reg */
    ad[6] = 0x146;               ad[7] = arg0;             /* AD: data -> GS reg (caller) */
    ad[8] = (u64)(nRows | 0x8000) | ((u64)0x9000 << 46);
    ad[9] = 0x44;                                           /* AD: data -> GS reg */

    /* one screen-spanning sprite quad per strip; XY packed 16.16, Y band += 0x200 */
    if (nRows > 0) {
        s32  vEdge    = screenV << 3;
        s32  hEdge    = screenH << 3;
        u64  xLeftHi  = (u64)(u32)(0x8000 - vEdge)          << 16;
        u64  xRightHi = (u64)(u32)((screenV << 3) + 0x7FF0) << 16;
        s32  yTop     = 0x8000 - hEdge;
        s32  yBot     = 0x8200 - hEdge;
        u64 *quad     = (u64 *)((u8 *)dmatag + 0x60);

        for (i = 0; i < nRows; i++) {
            quad[0] = (u32)yTop | xLeftHi;
            quad[1] = (u32)yBot | xRightHi;
            quad += 2;
            yTop += 0x200;
            yBot += 0x200;
        }
    }

    /* closing DMAtag */
    dmatag = (u32 *)((u8 *)g_frameDmaCursor + (nRows << 4) + 0x50);
    g_frameDmaCursor = (u8 *)dmatag;
    dmatag[0] = 0x10000000;
    dmatag[1] = 0;
    dmatag[2] = 0x13000000;
    dmatag[3] = 0;
    g_frameDmaCursor = (u8 *)dmatag + 0x10;
}
#endif

/* Build the moby-glow GIF/DMA packets. Emits a DMA/GIF header (0x30000007 CNT tag
 * + D_1390B0 + 0x13000000 VIF + 0x50000007) into g_frameDmaCursor, then walks the
 * glow-record list: for each record it orients the glow quad (func_002E0458, scale
 * 1000.0) and fills a scratch with the record's corner floats (+0x10/14/18
 * duplicated, and the two derived edges -(+0x8)-(+0xC) / (+0xC)-(+0x8)); an inner
 * loop emits each of the record's `count` (+0x0) sub-items (advance via
 * func_002E0568, then func_002E19C0 / func_002E0EA0 / func_002E05C0 x2). Closes
 * with the frame-buffer GS setup regs (0x4C/0x42, 0x47/0x42) + func_002E07F8. The
 * closing GS values are 64-bit (0x8000<<22/<<24) — passed via the native u64
 * AppendGsRegPacket decl above (matching arm untouched). Engine-2.96 -> #else. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", EmitMobyGlowPackets);
#else
extern u8  *g_frameDmaCursor;
extern u8   D_1390B0[];
extern s32  g_vramFrameBufB;
extern void func_002E05C0(s32 a, s32 b, s32 c);
extern void func_002E0EA0(s32 a, s32 b, s32 c);
extern void func_002E19C0(void *a, void *b);

void EmitMobyGlowPackets(void *list) {
    u8  *rec  = (u8 *)list;
    u8  *work = (u8 *)list;
    u8  *cur;
    f32  sp[8];
    s32  i;

    func_002E0650();
    cur = g_frameDmaCursor;
    *(u32 *)(cur + 0x0) = 0x30000007;
    *(u32 *)(cur + 0x4) = (u32)(u8 *)D_1390B0;
    *(u32 *)(cur + 0x8) = 0x13000000;
    *(u32 *)(cur + 0xC) = 0x50000007;
    g_frameDmaCursor = cur + 0x10;

    while (*(s32 *)(rec + 0x0) != 0) {
        work += 0x30;
        func_002E0458(rec + 0x20, 1000.0f);
        sp[0] = sp[4] = *(f32 *)(rec + 0x10);
        sp[1] = sp[5] = *(f32 *)(rec + 0x14);
        sp[2] = sp[6] = *(f32 *)(rec + 0x18);
        sp[3] = -*(f32 *)(rec + 0x8) - *(f32 *)(rec + 0xC);
        sp[7] =  *(f32 *)(rec + 0xC) - *(f32 *)(rec + 0x8);
        if (*(s32 *)(rec + 0x0) > 0) {
            i = 0;
            do {
                work = func_002E0568(work);
                i++;
                func_002E19C0(sp, &sp[4]);
                func_002E0EA0(0x70000000, *(s32 *)(D_001B1E90 + 0x170), *(s32 *)(rec + 0x4));
                func_002E05C0(2, 1, 2);
                func_002E05C0(1, 0, 2);
            } while (i < *(s32 *)(rec + 0x0));
        }
        rec = work;
    }

    AppendGsRegPacket(0x4C, (u64)((g_vramFrameBufB >> 13) | 0x80000));
    AppendGsRegPacket(0x42, ((u64)0x8000 << 22) | 0x64);
    func_002E07F8(0);
    AppendGsRegPacket(0x47, 0x5360B);
    AppendGsRegPacket(0x42, ((u64)0x8000 << 24) | 0x44);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", BuildMobyGlowRecords);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E0EA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E1220);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E1370);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E19C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E1A58);

/* Close the shrub draw segment opened by BuildShrubDrawSegment: the shrub twin
 * of CloseSkyDrawSegment. Splices the shrub texture uploads ahead of the shrub
 * packets via the same DMAtag NEXT-tag detour, then bookkeeps VRAM usage:
 *   1. reserve a qword at the current cursor as the close tag, advance +0x10;
 *   2. finalise the open tag (DMAcnt, next = advanced cursor);
 *   3. upload the shrub textures (returns VRAM bytes used) + append tex flush;
 *   4. if usage exceeds 0x400000, print the overflow warning (retail no-op);
 *      track the high-water mark in g_shrubVramPeak;
 *   5. emit a return tag back to openTag+0x10, then fill the reserved close tag.
 * The close tag here is tracked in a local (no close-tag global, unlike sky). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", CloseShrubDrawSegment);
#else
void CloseShrubDrawSegment(void) {
    u32 *closeTag = (u32 *)g_frameDmaCursor;
    u32 *openTag;
    u32 *tag;
    s32 vramUsed;

    g_frameDmaCursor = (u8 *)closeTag + 0x10;

    openTag = (u32 *)g_pShrubSegmentOpenTag;
    openTag[0] = 0x20000000;
    openTag[1] = (u32)g_frameDmaCursor;
    openTag[2] = 0;
    openTag[3] = 0;

    vramUsed = UploadShrubTextures(g_vramAllocCursor);
    AppendTexFlushDefaultTex0();

    if (vramUsed > 0x400000) {
        DebugPrintStub(D_1ABE10);
    }
    if (vramUsed > g_shrubVramPeak) {
        g_shrubVramPeak = vramUsed;
    }

    tag = (u32 *)g_frameDmaCursor;
    tag[0] = 0x20000000;
    tag[1] = (u32)(g_pShrubSegmentOpenTag + 0x10);
    tag[2] = 0;
    tag[3] = 0;
    g_frameDmaCursor = (u8 *)g_frameDmaCursor + 0x10;

    closeTag[0] = 0x20000000;
    closeTag[1] = (u32)g_frameDmaCursor;
    closeTag[2] = 0;
    closeTag[3] = 0;
}
#endif

/* Patch the GS TEX0 texture-base-pointer (TBP) fields in every visible shrub's
 * draw packets with the VRAM addresses resolved by UploadShrubTextures. Walks
 * the -1-terminated g_shrubVisibleClassList; for each class record, over its
 * nItems (+0x28) sub-packets (ptr array at +0x40, stride 8); for each packet,
 * over its nTags giftags (base = packet+0x20 + (packet[0x14]<<4), stride 0x40);
 * looks up the giftag's texture id (byte +0x13) in g_shrubTexVramTable (stride
 * 4: {tbp_lo, tbp_hi}) and, when non-zero, splices the 14-bit TBP into the TEX0
 * AD-data words at +0x30 (TEX0_1) and +0x20 (TEX0_2), preserving the upper bits
 * (mask 0xFFFFC000). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", PatchShrubPacketTex0);
#else
void PatchShrubPacketTex0(void) {
    s32 *listPtr = g_shrubVisibleClassList;
    s32 classIdx = listPtr[0];

    if (classIdx < 0) {
        return;
    }
    do {
        u8 *classRec = g_shrubClassTable[classIdx];
        s32 nItems = *(s16 *)(classRec + 0x28);
        s32 i;

        listPtr++;
        for (i = 0; i < nItems; i++) {
            u8 *packet = *(u8 **)(classRec + 0x40 + i * 8);
            s32 nTags = *(s32 *)(packet + 0x10);
            u8 *giftag = packet + 0x20 + (*(s32 *)(packet + 0x14) << 4);
            s32 k;

            for (k = 0; k < nTags; k++) {
                s16 *vram = &g_shrubTexVramTable[giftag[0x13] * 2];
                s16 lo = vram[0];
                s16 hi;

                if (lo != 0) {
                    *(u32 *)(giftag + 0x30) = (*(u32 *)(giftag + 0x30) & 0xFFFFC000) | (u32)lo;
                }
                hi = vram[1];
                if (hi != 0) {
                    *(u32 *)(giftag + 0x20) = (*(u32 *)(giftag + 0x20) & 0xFFFFC000) | (u32)hi;
                }
                giftag += 0x40;
            }
        }
        classIdx = *listPtr;
    } while (classIdx >= 0);
}
#endif

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

/* Remove LOD level `mode` from a run of shrub instances. For each shrub id in
 * [idStart, idEnd), examine the 4-nibble active-LOD field at instance+0x1E: find
 * the nibble equal to `mode`, drop it (shift the higher nibbles down one, fill
 * the top nibble with 0xF), and write the field back. When the field saturates
 * to 0xFFFF (all levels exhausted) also set the "collapsed" byte at +0x1B.
 * (Handwritten asm in the ROM — uses `j`/`add`; not byte-matchable from C, so
 * this is a portable-only #else. Instances stride 0x20 in g_pShrubInstanceArray.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E4178);
#else
void func_002E4178(u16 *idStart, u16 *idEnd, s32 mode) {
    u8 *instArray = g_pShrubInstanceArray;
    u16 *p;

    for (p = idStart; p != idEnd; p++) {
        u8 *inst = instArray + (*p) * 0x20;
        u16 flags = *(u16 *)(inst + 0x1E);
        u16 v;

        if ((flags & 0x000F) == (mode << 0)) {
            v = (u16)((flags >> 4) | 0xF000);
        } else if ((flags & 0x00F0) == (mode << 4)) {
            v = (u16)(((flags >> 4) & 0x0FF0) | (flags & 0x000F) | 0xF000);
        } else if ((flags & 0x0F00) == (mode << 8)) {
            v = (u16)(((flags >> 4) & 0x0F00) | (flags & 0x00FF) | 0xF000);
        } else if ((flags & 0xF000) == (mode << 12)) {
            v = (u16)(flags | 0xF000);
        } else {
            continue;
        }

        *(u16 *)(inst + 0x1E) = v;
        if (v == 0xFFFF) {
            *(u8 *)(inst + 0x1B) = 1;
        }
    }
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E4280);

__asm__(".extern g_pSkyShellSpinRates, 16");
extern f32 *g_pSkyShellSpinRates; /* 0x1B1910 - per-shell {x,y,z} spin rate table (stride 0xC) */
extern f32 g_skyShellAngles[];    /* 0x261DB0 - per-shell {x,y,z} accumulated angles (stride 0xC) */
extern u8 g_skyShellMatrix[];     /* 0x1B2070 - shared sky-shell transform matrix */
extern float WrapAnglePiSum(float a, float b);
extern void func_00283DE0(void *matrix, f32 *eulerXYZ);

/* Advance sky shell `shellIdx`'s three Euler angles by its per-shell spin rate
 * (wrapping each into [-pi,pi] via WrapAnglePiSum) and rebuild the shared sky-
 * shell rotation matrix from the updated {x,y,z} angles.  The angle table and
 * spin-rate table are both strided 0xC (three floats per shell).
 * NEAR-MISS (~70%, structurally faithful): cc1 -O2 -G8 -fno-gcse derives the
 * three angle-slot pointers eagerly and needs a 5th callee-save, growing the
 * frame 0x40 -> 0x60/0x70, where the original interleaves each pointer's
 * derivation with the call stream and keeps only 4 saves (the lazy-derive /
 * regalloc-schedule wall).  The angle math, call order, and the {x,y,z} stack
 * vec handed to the matrix builder all match.  Not cmp-oracle'd: the tail calls
 * the VU0 matrix builder func_00283DE0, which has no native leaf yet. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", UpdateSkyShellRotation);
#else
void UpdateSkyShellRotation(s32 shellIdx) {
    s32 stride = shellIdx * 0xC;
    f32 *angleX = (f32 *)((u8 *)g_skyShellAngles + stride);
    f32 *angleY = (f32 *)((u8 *)g_skyShellAngles + 0x4 + stride);
    f32 *angleZ = (f32 *)((u8 *)g_skyShellAngles + 0x8 + stride);
    f32 euler[3];

    *angleX = WrapAnglePiSum(*angleX,
                             *(f32 *)((u8 *)g_pSkyShellSpinRates + stride));
    *angleY = WrapAnglePiSum(*angleY,
                             *(f32 *)((u8 *)g_pSkyShellSpinRates + 0x4 + stride));
    *angleZ = WrapAnglePiSum(*angleZ,
                             *(f32 *)((u8 *)g_pSkyShellSpinRates + 0x8 + stride));

    euler[0] = *angleX;
    euler[1] = *angleY;
    euler[2] = *angleZ;
    func_00283DE0(g_skyShellMatrix, euler);
}
#endif

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E43E8);

/* func_002E43F8: PARKED #70 (#else not confident) — sky-shell render driver (BeginSkyDrawSegment,
 * DrawSkyShell, CloseSkyDrawSegment, AppendGsRegPacket + FP). GS draw-segment + FP class;
 * needs the sky-draw wiring traced before a faithful #else. */
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

/* Close the sky draw segment opened by BeginSkyDrawSegment: it splices the
 * queued texture uploads AHEAD of the sky geometry via a DMAtag NEXT-tag detour.
 *   1. reserve a qword at the current cursor as the close tag, advance +0x10;
 *   2. finalise the open tag (DMAcnt 0x20000000, next = advanced cursor);
 *   3. drain the pending tex uploads (both callees advance g_frameDmaCursor);
 *   4. emit a return tag pointing back to openTag+0x10 (past the open tag);
 *   5. fill the reserved close tag (DMAcnt, next = cursor);
 *   6. reset the VRAM bump cursor to the dynamic base.
 * Each tag is 4 words: [id/qwc, next-addr, 0, 0]. g_frameDmaCursor is re-read
 * after the flush calls (they mutate it), so the reads are not cached. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", CloseSkyDrawSegment);
#else
void CloseSkyDrawSegment(void) {
    u32 *closeTag = (u32 *)g_frameDmaCursor;
    u32 *openTag;
    u32 *tag;

    g_pSkySegmentCloseTag = (u8 *)closeTag;
    g_frameDmaCursor = (u8 *)closeTag + 0x10;

    openTag = (u32 *)g_pSkySegmentOpenTag;
    openTag[0] = 0x20000000;
    openTag[1] = (u32)g_frameDmaCursor;
    openTag[2] = 0;
    openTag[3] = 0;

    FlushPendingTexUploads();
    AppendTexFlushDefaultTex0();

    tag = (u32 *)g_frameDmaCursor;
    tag[0] = 0x20000000;
    tag[1] = (u32)(g_pSkySegmentOpenTag + 0x10);
    tag[2] = 0;
    tag[3] = 0;
    g_frameDmaCursor = (u8 *)g_frameDmaCursor + 0x10;

    closeTag[0] = 0x20000000;
    closeTag[1] = (u32)g_frameDmaCursor;
    closeTag[2] = 0;
    closeTag[3] = 0;

    g_vramAllocCursor = g_vramDynamicBase;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E4750);

extern void DrawSkyPiecesFormatA(void *piece);
extern void DrawSkyPiecesFormatB(void *piece);

/* Draw sky shell `shellIdx`: look up its piece list (g_pSkyData+0x20 pointer
 * array) and dispatch to format B if the piece carries a +0x4 sub-list,
 * otherwise format A.  Shell indices past the count (+0x6) are ignored.
 * TODO(match): not byte-exact; ONE instruction short on the better arm
 * (measured task #563, unit objdiff on the committed body).
 *   engine96 97.05% via MATCH_DrawSkyShell, 22/22 insns, 1 differs - DSLOT-FILL.
 *            The ROM fills the post-FormatB `b` delay slot with a DUPLICATED
 *            `ld ra,0(sp)` and branches straight to `jr ra`; cc1 emits `nop`
 *            there and shares the single `ld ra` epilogue.  Everything else,
 *            including the `addu v0,v1,v0` operand order, is exact.
 *   sdk29    96.59%, 22/22 insns, 2 differ - the same DSLOT-FILL plus an
 *            `addu v0,v0,v1` operand-order flip the 2.96 arm does not have
 *            (REGNUM class); rewriting the index as `((void **)(sky+0x20))[i]`
 *            was RUN and is inert (96.59% unchanged, byte-identical output).
 * The trailing `__asm__ __volatile__("")` is load-bearing but is ALSO what
 * blocks the delay-slot hoist - all three positions were measured on engine96:
 * at function end 97.05% (this one), removed 70.45% (2.96 sibcalls both calls),
 * duplicated inside each branch 92.73%.  No position gets both. */
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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E4C68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", TransformSkyPieceVerts);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E4DB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", EmitSkyTrianglePackets);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E5074);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", CullSkyPieceVisibility);

extern u8 g_cameraPos[];      /* 0x1B52C0 - listener / camera world position */
extern u8 g_collHitPoint[];   /* 0x1C4F20 - last line-trace hit point */
extern u8 g_sndChannelVolumes[]; /* 0x188F40 - sound-channel mix state block */
extern void Vec4SubVu0(void *dst, void *a, void *b);
extern void Vec4ScaleVu0(void *dst, float s, void *src);   /* sig: scale BEFORE src (def 183558.c:200) */
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
        Vec4ScaleVu0(out, 0.75f, out);   /* (dst, scale, src) - was arg-swapped */
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
    Vec4ScaleVu0(probe, 0.75f, probe);   /* (dst, scale, src) - was arg-swapped */
    func_00283968(probe, probe, 64.0f);
    Vec4AddVu0(probe, probe, g_cameraPos);
    CollLine(outHit, probe, 0x82, *(s32 *)((u8 *)emitter + 0x18), 0);
}
#endif

extern float Vec3DistVu0(void *a, void *b);
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
 * TODO(match): not byte-exact on either arm; each arm gets a different half of
 * the ROM right (measured task #563, unit objdiff on the committed body).
 *   sdk29    99.12%, 17/17 insns, 5 differ - PACKED-SAVE: the ROM packs the two
 *            8-byte saves (s0,ra) into a 0x10 frame with ra at +0x8, cc1 2.9
 *            rounds to 0x20 with ra at +0x10 and swaps the epilogue restore
 *            order.  This is the 2.9 16-byte callee-save stride; no flag or C
 *            phrasing reproduces it (same class as FACT #7470's 9 FRAME rows).
 *   engine96 88.24% via MATCH_ComputeEmitterVolume, 17/17 insns, 2 differ -
 *            the 2.96 arm gets the ROM's 0x10 frame and slot offsets EXACTLY.
 *            Its residual is SCHED-PROEPI: the ROM orders `sd ra,8(sp)` before
 *            `daddu a0,a1,zero`, cc1 2.96 emits the two transposed.
 * The `s32 vol = ...; __asm__ __volatile__(""); return vol;` shape below is the
 * FACT #7343 value-returning tail-call barrier and is LOAD-BEARING on the
 * engine96 arm: without it 2.96 sibcalls (`j ComputeVolumeFalloff` with the
 * teardown in the delay slot) and the arm reads 67.06%.  The bare temp without
 * the barrier is inert (byte-identical output) - the asm is what blocks it. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", ComputeEmitterVolume);
#else
s32 ComputeEmitterVolume(SoundEmitterSlot *slot, Vec4 *pos) {
    float dist = Vec3DistVu0(pos, g_cameraPos);
    float *curve = *(float **)((u8 *)slot + 0x8);
    s32 vol = ComputeVolumeFalloff((SoundDef *)curve, dist, curve[0], curve[1]);
    __asm__ __volatile__("");
    return vol;
}
#endif

/* Compute the stereo pan angle (in degrees) for emitter slot `slot`, the
 * direction of the sound source in camera space.
 *
 * The emitter->listener vector (slot - g_cameraPos) is rotated into camera space
 * by the camera matrix (g_cameraPos+0x230 == 0x1B54F0), the azimuth is taken as
 * atan2(x, z) of that camera-space direction, normalised into [0, 2pi), and the
 * planar distance is measured.  For close sources (planar distance < 2.0) the
 * raw azimuth is smoothed against the slot's previous angle (stored at slot+0x40)
 * with a +/-0.2 rad hysteresis dead-band so a near, fast-moving source does not
 * pan-jitter; for distant sources (>= 2.0) the last-angle field (slot+0x40) is
 * reset to -1.0 (so the hysteresis restarts next time), but the RETURN is still
 * the current frame's azimuth in degrees (the asm keeps it in callee-saved $f20).
 * The radian angle is converted to degrees (*180/pi) and truncated to int.
 *
 * TODO(match): functional equivalent - not byte-exact.  WALLED: three 8-byte
 * callee saves (s0,s1,s2) + two fp saves (f20,f21) packed 8-byte where this cc1
 * rounds to 16-byte, and the hysteresis tests lower to branch-likely (`bc1fl`)
 * stores that cc1 emits as plain `bc1f` + store.  Control flow + call order are
 * faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", ComputeEmitterPan);
#else
extern void func_00284098(void *dst, void *mtx, void *src); /* rotate vec by mtx */
extern void func_00283A48(void *dst, void *a, void *b);     /* vec helper        */
extern float Atan2fPoly(float x, float z);               /* atan2(x, z)       */
extern float Vec2LengthXyVu0(void *v);                        /* planar magnitude  */
extern float WrapAnglePiSum(float a, float b);              /* wrap a+b to [-pi,pi]*/
extern float WrapAnglePiDiff(float a, float b);             /* wrap a-b to [-pi,pi]*/
extern s32   FloatToInt(float v);

s32 ComputeEmitterPan(SoundEmitterSlot *slot, Vec4 *pos) {
    f32 dir[4];        /* sp+0x00: emitter->listener, then camera-space dir */
    f32 rot[4];        /* sp+0x10: rotated scratch                          */
    f32 azimuth;       /* f20: working radian angle                         */
    f32 planar;        /* f0:  planar distance                              */
    f32 lastAngle;     /* slot+0x40: previous frame's angle                 */

    Vec4SubVu0(dir, pos, g_cameraPos);
    /* asm passes ($4=rot, $5=cameraMatrix, $6=cameraPos) - reproduced verbatim */
    func_00284098(rot, g_cameraPos + 0x230, g_cameraPos);
    func_00283A48(dir, dir, rot);

    azimuth = -Atan2fPoly(dir[0], dir[1]);
    if (azimuth < 0.0f) {
        azimuth += 6.2831855f;             /* +2pi -> [0, 2pi)              */
    }

    planar = Vec2LengthXyVu0(dir);
    if (planar < 2.0f) {
        lastAngle = *(f32 *)((u8 *)slot + 0x40);
        if (lastAngle < 0.0f) {
            /* no valid previous angle: accept the raw azimuth */
            *(f32 *)((u8 *)slot + 0x40) = azimuth;
        } else {
            f32 delta = WrapAnglePiDiff(azimuth, lastAngle);
            if (delta > 0.2f) {
                /* moving away CCW faster than the dead-band: clamp the step */
                azimuth = WrapAnglePiSum(*(f32 *)((u8 *)slot + 0x40), 0.2f);
            } else if (delta < -0.2f) {
                /* moving away CW faster than the dead-band: clamp the step */
                azimuth = WrapAnglePiDiff(*(f32 *)((u8 *)slot + 0x40), 0.2f);
            }
            /* within +/-0.2: keep the raw azimuth */
            *(f32 *)((u8 *)slot + 0x40) = azimuth;
        }
    } else {
        /* distant source: the asm only STORES -1.0 into the last-angle field
         * (slot+0x40); $f20 (azimuth) is callee-saved and still holds the
         * normalized azimuth, so the RETURN uses that, NOT -1.0. */
        *(f32 *)((u8 *)slot + 0x40) = -1.0f;
    }

    return FloatToInt(azimuth * 57.295776f);   /* radians -> degrees, trunc    */
}
#endif

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

/* InitSoundEmitterSystem: bring up the sound-emitter system — clear the listener ring
 * (g_listenerPosHistory first 0x40 bytes) and every emitter's +0x40/+0x44 and +0x70/+0x74
 * fields (0x70-stride up to +0x16C0), init the SDK sound core (snd_Init, mono per
 * g_audioStereoMode), configure 5 channels (func_001329B0 ch 1/2/4/5/6), and register the
 * 7 listener slots (func_00132888) + the reverb/params block (func_001328C0). Matching arm
 * stays INCLUDE_ASM (save-slot wall); #else is the faithful portable body. */
#ifdef TARGET_NATIVE
extern s32  g_audioStereoMode;
extern void snd_Init(s32 mode);
extern void func_00132938(s32 flag);
extern void func_00132978(s32 a, s32 b);
extern void func_001329B0(s32 channel, s32 b, s32 c);
extern void func_00132888(s32 slot, s32 value);
extern void func_001328C0(s32 a, s32 *params, s32 c, s32 d);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", InitSoundEmitterSystem);
#else
void InitSoundEmitterSystem(void) {
    u8 *base = g_listenerPosHistory;
    u8 *p;
    s32 i;
    s32 params[5];

    for (i = 0; i < 16; i++) {          /* clear the 0x40-byte listener ring */
        ((s32 *)base)[i] = 0;
    }
    *(s32 *)(base + 0x40) = 0;
    *(s32 *)(base + 0x44) = 0;
    for (p = base; p < base + 0x16C0; p += 0x70) {   /* clear each emitter's +0x70/+0x74 */
        *(s32 *)(p + 0x70) = 0;
        *(u8 *)(p + 0x74) = 0;
    }

    snd_Init(2);
    func_00132938(g_audioStereoMode < 1);
    func_00132978(0, 1);
    func_001329B0(1, 0x18, 0x2F);
    func_001329B0(2, 0x18, 0x2F);
    func_001329B0(4, 0x18, 0x2F);
    func_001329B0(5, 0x18, 0x2F);
    func_001329B0(6, 0x18, 0x2F);
    func_002E5698();
    func_00132888(0, *(s32 *)(base + 0x48));
    func_00132888(1, *(s32 *)(base + 0x4C));
    func_00132888(2, *(s32 *)(base + 0x50));
    func_00132888(3, *(s32 *)(base + 0x54));
    func_00132888(4, *(s32 *)(base + 0x58));
    func_00132888(5, *(s32 *)(base + 0x5C));
    func_00132888(6, *(s32 *)(base + 0x50));   /* quirk: 7th re-reads +0x50, not +0x60 */
    params[0] = 6;
    params[1] = 0x3B;
    params[2] = 0x6666;
    params[3] = 0x8000;
    params[4] = 0x8000;
    func_001328C0(0, params, 0x6666, 0x8000);
}
#endif

/* Point the IOP streamed-audio engine at the new level's music stream, then
 * log post-reverb free SRAM.  Issues 989snd ring command 0x51 sub-op 2 (start /
 * point stream) via func_00133750(2, levelStreamSector) where the stream sector
 * is g_levelTocHeader+0xC (== 0x1507E4), then queries free SRAM post-reverb with
 * the 0x4A/0x4B snd commands (func_001337F0 / func_00133820) and feeds the result
 * into the retail-noop DebugPrintStub ("*AFTER REVERB* level %d - free sram %d").
 * Sole caller is the per-level audio bring-up (UpdateLevelStagingMachine state 2).
 * NEAR-MISS: WALLED - 2 callee saves (s0,s1) packed 8-byte where this cc1 rounds
 * the frame.  NATIVE SHIM (no byte target). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", StartLevelMusicStream);
#else
extern u8  g_levelTocHeader[];   /* 0x1507D8 - current-level TOC header        */
extern s32 g_playerProgress;     /* 0x1A79F8 - player progress / level number  */
extern const char D_1ABE90[];    /* "*AFTER REVERB* level %d - free sram %d"   */
extern void func_00133750(s32 cmd, s32 streamSector);  /* snd cmd 0x51 sub-op 2 */
extern s32  func_001337F0(void);                       /* snd cmd 0x4A          */
extern s32  func_00133820(void);                       /* snd cmd 0x4B          */
extern void DebugPrintStub(const char *fmt, ...);      /* retail no-op          */

void StartLevelMusicStream(void) {
    s32 sramA;
    s32 sramB;

    func_00133750(2, *(s32 *)(g_levelTocHeader + 0xC));
    sramA = func_001337F0();
    sramB = func_00133820();
    DebugPrintStub(D_1ABE90, g_playerProgress, sramA, sramB);
}
#endif

/* UpdateSoundEmitters: PARKED #70 (#else not confident) — the large (~0xEE4, ~50-call)
 * per-frame sound-emitter update: per-emitter occlusion raycast (CastEmitterOcclusionRay,
 * ComputeListenerOcclusionProbe), pan/volume (ComputeEmitterPan, ComputeEmitterVolume) and
 * SDK sound updates, over the g_listenerPosHistory emitter table. Far too large + FP/branch-
 * dense to model confidently; a multi-pass Ghidra decomposition job. */
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
        if (ComputeEmitterVolume((SoundEmitterSlot *)(e + 0x70), (Vec4 *)(e + 0x90)) < 0x20) {
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

/* Play sound `soundIdx` from owner moby `owner`'s class sound bank.
 *
 * The moby's loaded class header (moby+0x24) carries a sound-def count byte at
 * +0xD and a 0x20-byte-stride sound-def array pointer at +0x28; the def for
 * `soundIdx` is array[soundIdx].  Delegates to StartSoundEmitter with the given
 * `flags`, the owner moby, no explicit position, and volume scale 0x400; on
 * success it stamps the source sound index (s16 at slot+0x7E) and owner (s32 at
 * slot+0x88) into the slot record (off g_listenerPosHistory, +0x70 ahead of
 * g_soundEmitterTable).  Returns the slot index, or -1 if the owner is null, its
 * class is unloaded, the class has no def array, soundIdx is out of range, or no
 * emitter could be started.
 * NEAR-MISS: WALLED - 2 callee saves (s0,s1) 8-byte-packed + the null/range
 * guards lower to branch-likely (`beql`/`bnel`) the cc1 emits as plain branches.
 * NATIVE SHIM (no byte target). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", PlayMobySound);
#else
s32 PlayMobySound(s32 soundIdx, s32 flags, Moby *owner) {
    u8 *classHdr;
    u8 *defArray;
    s32 slot;
    u8 *e;

    if (owner == NULL) {
        return -1;
    }
    classHdr = *(u8 **)((u8 *)owner + 0x24);
    if (classHdr == NULL) {
        return -1;
    }
    defArray = *(u8 **)(classHdr + 0x28);
    if (defArray == NULL) {
        return -1;
    }
    if (soundIdx >= *(u8 *)(classHdr + 0xD)) {
        return -1;
    }

    slot = StartSoundEmitter((SoundDef *)(defArray + soundIdx * 0x20), flags,
                             owner, NULL, 0x400);
    if (slot >= 0) {
        e = g_listenerPosHistory + slot * 0x70;
        *(s16 *)(e + 0x7E) = (s16)soundIdx;
        *(s32 *)(e + 0x88) = (s32)owner;
    }
    return slot;
}
#endif

extern u8 g_mobyClassSlotRemap[]; /* 0x1CE460 - class id -> loaded slot (0xFF=unloaded) */
extern u8 *g_mobyClassHeaders[];  /* 0x1CDB00 - loaded class header ptr per slot       */

/* Like PlayMobySound, but resolves the class sound bank from a class id rather
 * than from a live moby: `classId` indexes the remap table g_mobyClassSlotRemap
 * to a loaded slot, which indexes g_mobyClassHeaders to the class header (count
 * byte +0xD, 0x20-stride def array +0x28).  Plays def `soundIdx` with `flags`,
 * owner `owner`, no explicit position, volume scale 0x400, stamping soundIdx and
 * owner into the slot.  Returns the slot index, or -1 if the class is unloaded,
 * has no def array, soundIdx is out of range, or no emitter could be started.
 * NEAR-MISS: WALLED - same 2-save/branch-likely walls as PlayMobySound, plus a
 * `mult`-based slot stride.  NATIVE SHIM (no byte target). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", PlaySoundFromClassBank);
#else
s32 PlaySoundFromClassBank(s32 soundIdx, s32 flags, Moby *owner, s32 classId) {
    u8 *classHdr;
    u8 *defArray;
    s32 slot;
    u8 *e;

    classHdr = g_mobyClassHeaders[g_mobyClassSlotRemap[classId]];
    if (classHdr == NULL) {
        return -1;
    }
    defArray = *(u8 **)(classHdr + 0x28);
    if (defArray == NULL) {
        return -1;
    }
    if (soundIdx >= *(u8 *)(classHdr + 0xD)) {
        return -1;
    }

    slot = StartSoundEmitter((SoundDef *)(defArray + soundIdx * 0x20), flags,
                             owner, NULL, 0x400);
    if (slot >= 0) {
        e = g_listenerPosHistory + slot * 0x70;
        *(s16 *)(e + 0x7E) = (s16)soundIdx;
        *(s32 *)(e + 0x88) = (s32)owner;
    }
    return slot;
}
#endif

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

    slot = StartSoundEmitter((SoundDef *)(pool + soundIdx * 0x20), posOverride, (void *)owner,
                             NULL, 0x400);
    if (slot >= 0) {
        e = g_listenerPosHistory + slot * 0x70;
        *(s16 *)(e + 0x7E) = (s16)soundIdx;
        *(s32 *)(e + 0x88) = owner;
    }
    return slot;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E6D28);

/* Flag emitter slot `slotIndex` as positioned (set flags bit 0x40) and copy the
 * 16-byte spatial quad from `src` into the slot's 0xA0 field; returns 1.
 * TODO(match): not byte-exact - ADDR-DISTRIB (measured task #563, unit objdiff
 * on the committed body).
 *   sdk29    79.57%, ROM 14 insns vs 12 - the ROM materializes the quad
 *            destination base (g_listenerPosHistory+0xA0) into its own register
 *            and stores at displacement 0 (`addiu v0,a2,160; addu a0,a0,v0;
 *            sq v0,0(a0)`); cc1 2.9 keeps one base and folds +0xA0 into the
 *            store displacement (`sq a2,160(a0)`), which is 2 insns shorter.
 *   engine96 49.21% - worse, and additionally strength-reduces the 0x70 stride
 *            to `sll;subu;sll` where the ROM has `li 112; mult` (FACT #7379),
 *            so the 2.9 arm is the right one here.
 * LEVER RUN AND FAILED: writing the two bases explicitly in the source
 * (`u_long128 *dst = (u_long128 *)((g_listenerPosHistory + 0xA0) + idx*0x70)`)
 * does NOT survive - cc1 2.9 reassociates them back to one base and still emits
 * `sq a0,160(a2)`, scoring 74.86%, BELOW the single-slot form kept here.  The
 * fold is the compiler's, not the source's. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1DFF80", SetSoundEmitterOffset);
#else
s32 SetSoundEmitterOffset(s32 slotIndex, u_long128 *src) {
    EmitterView *slot = EMITTER_VIEW(slotIndex);
    slot->flags |= 0x40;
    slot->quadA0 = *src;
    return 1;
}
#endif

/* func_002E6D70: 8 bytes of dead debris (li v0,1; nop) carved off the real
 * entry func_002E6D78 in task #472 (the pre-carve NEAR-MISS ~80% of the fused
 * tile was this pair, which single-function C cannot reproduce). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E6D70);

/* Set the pitch field of sound-emitter slot `slotIndex` (the slot's +0x84 word,
 * reached as g_listenerPosHistory + idx*0x70 + 0x84 -- see the pool note at the
 * top of this file).  Returns 1 unconditionally; no slot-index bound check, so
 * callers must pass a live slot.
 * MATCHED 100.00% on the sdk29 arm (plain C, unit objdiff via objdiff_build.sh +
 * unit_report.sh; verify_match_unit.sh BYTE IDENTICAL, 8/8 words, 2 relocs
 * resolved; task #563).  The slot stride is spelled `idx * 0x70` so cc1 2.9
 * emits the ROM's `li 112; mult` rather than the 2.96 arm's `sll;subu;sll`
 * strength-reduction (FACT #7379). */
s32 func_002E6D78(s32 slotIndex, s32 pitch) {
    EmitterView *slot = EMITTER_VIEW(slotIndex);
    slot->pitch = pitch;
    return 1;
}

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

/* 989snd deferred key-on callback (0x2E6ED8): store the freshly allocated voice
 * handle in the emitter slot and advance state 1 -> 2.  A zero handle means the
 * voice failed to start, so free the slot (state 0, clear the bookkeeping words).
 * The slot arrives as a 64-bit value whose low 32 bits hold its address.
 * Defined HERE (not at file top) to match the original address order. */
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

/* 989snd voice-ended callback (0x2E6F20): record the (zero) handle and free the
 * emitter slot - clear the state byte and the owner/link bookkeeping words. */
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

/* func_002E6F4C: 4-byte trailing-alignment nop after the unit's last function
 * (OnEmitterVoiceEnded ends at 0x2E6F4C; the next unit starts at 0x2E6F50). The
 * compiler does not emit it, so recover it as a raw-word filler to keep the unit
 * size==span byte-exact (06333c5 precedent). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1DFF80", func_002E6F4C);

/* The unit tail (0x2E6F50..0x2F003F) is the spimdisasm c-mode tail-fusion blob:
 * functions reached only by j / data-ref (no jal) that spimdisasm cannot promote
 * to their own symbols, so they fuse into one INCLUDE_ASM. It is islanded as the
 * asm segment text/1E6ED0 to keep the objdiff count honest. */

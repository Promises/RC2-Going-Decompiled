#include "common.h"

/*
 * text/16E980 — head of the big .text segment (carve-pipeline pick #6,
 * 2026-06-13; vaddr 0x26EA00..0x274127, 90 fns): splash/attract boot helpers,
 * the camera system core (slot switching/transitions/shake/fades) and the
 * screen-space sprite FX queue (lens flare + 2D sprite draw).
 *
 * The matcher builds THIS unit at -O2 -G8 (per-unit GFLAG override in
 * tools/ee/objdiff_build.sh / diff.sh / build.sh). Plain -G8, NOT -fno-gcse:
 * for the CURRENTLY-MATCHED set the gcse flag is inert (both settings yield
 * the same 35), so we keep the simpler plain -G8. (An unmatched INCLUDE_ASM
 * here — AddScreenSpriteFx — does hold the g_screenSpriteFxQueue %hi in a
 * register across the function via load-PRE CSE, a hint that the original TU
 * had gcse on; left plain -G8 unless a future match needs the flag.) The
 * 8-byte-packed callee-save wall still applies (e.g. func_0026F850).
 *
 * -G8 extern sizing follows the documented rules (see text/198FA0 /
 * text/1A8180 headers): complete externs <= 8 bytes = true small data
 * (%gp_rel); `.extern sym,16` = cc1-small / assembler-absolute one-insn
 * macro; large/struct externs = ordinary two-insn absolute %hi/%lo.
 *
 * SAVE-LAYOUT WALL: every function below that saves two or more GPRs
 * (incl. $ra) at 8-byte slot spacing is blocked on the 8-byte-packed-save
 * wall and stays INCLUDE_ASM (the pinned cc1 reserves 16 bytes per save).
 *
 * STUB TABLE at 0x26F718: a run of 32 eight-byte stubs (empty `return;` /
 * `return 0;` bodies — debug/profiling hooks compiled out of the retail
 * build). The 0x30-byte blob func_0026F820 and the lone fill words
 * func_0026FC80/func_002701B8/func_00273320/func_002735A8 plus the
 * no-return fragments func_002702C8/func_002725D8 are NOT reachable
 * compiler output (orphaned fill / handwritten table words) and keep their
 * INCLUDE_ASM permanently.
 */

/* Cc1-small / assembler-absolute symbols (one-insn symbolic macro). */
__asm__(".extern g_screenFadeBlack, 16");

/* True small data (complete <=8-byte externs, %gp_rel). */
extern s32 D_1A8630;             /* screen-sprite-FX alpha cap (set by the HUD fade) */

/* Large / ordinary-absolute globals. */
extern s32 g_nGameState[];       /* incomplete-array decl: keeps the 4-byte int
                                  * out of cc1 small data (two-insn absolute
                                  * access like the original) */
extern f32 g_screenFadeBlack;    /* black screen fade level 0..1 */
extern f32 g_screenFadeWhite;    /* white screen flash level 0..1 */

/* One 0xA0-byte camera slot (fields per the Track-B pass; only the ones this
 * unit touches are declared). */
typedef struct Camera {
    /* 0x00 */ u8 pad0[0x74];
    /* 0x74 */ s32 takeKind;      /* takeover rule selector (TestCameraTakeover switch) */
    /* 0x78 */ u8 pad78[4];
    /* 0x7C */ u8 priority;       /* takeover priority; 0 = slot not eligible */
    /* 0x7D */ u8 unk7D;          /* cleared on activation; case-1/2 takeover gate */
    /* 0x7E */ s16 unk7E;         /* set to 1 on activation */
    /* 0x80 */ u8 pad80[4];
    /* 0x84 */ s16 configIndex;
    /* 0x86 */ s16 type;          /* slot search key (func_00270290) */
    /* 0x88 */ u8 pad88[4];
    /* 0x8C */ s16 modeId;        /* index into g_cameraModeVtbl */
    /* 0x8E */ s16 snapFlag;
    /* 0x90 */ u8 pad90[0x10];
} Camera;

/* Camera mode vtable entry (lvl.camvtbl, 0x14-byte stride; runtime-filled by
 * level overlays). Handlers are declared value-returning so cc1 does not
 * sibling-call-optimise the conditional forwarding calls. */
typedef struct CameraModeVtblEntry {
    /* 0x00 */ s32 id;
    /* 0x04 */ s32 (*takeover)();
    /* 0x08 */ s32 (*enter)();
    /* 0x0C */ s32 (*update)();
    /* 0x10 */ s32 (*poll)();
} CameraModeVtblEntry;

extern Camera g_cameraSlots[48];
extern CameraModeVtblEntry g_cameraModeVtbl[];

/* Camera transition state @0x1B5410 (+0x02 kind, Vec4 src/cur pos pairs).
 * The 16-byte Vec4 struct assignments compile to the original block-move
 * shape (address regs + offset-0 lq/sq). */
#ifdef TARGET_NATIVE
/* gcc -m32 cannot emulate mode(TI); copy-only here, so a 16-byte aligned struct
 * is an exact portable stand-in. Inert to the matching build. */
typedef struct { unsigned long long _q[2]; } __attribute__((aligned(16))) u_long128;
#else
typedef unsigned long u_long128 __attribute__((mode(TI)));
#endif
typedef struct Vec4 { f32 x, y, z, w; } __attribute__((aligned(16))) Vec4;

/* Camera-system file-scope state block @0x1B5180 (pinned in symbol_addrs so
 * the base+offset accesses symbolize; +0x190 aliases the separately-named
 * g_activeCamera). Only fields this unit touches are declared. */
typedef struct CameraSysState {
    /* 0x000 */ u8 pad0[0x140];
    /* 0x140 */ Vec4 camPos;         /* live camera position (z at +0x148) */
    /* 0x150 */ u8 pad150[0x40];
    /* 0x190 */ volatile Camera *volatile activeCamera; /* fully volatile: the
                                                * original re-reads the pointer
                                                * between dereferences and keeps
                                                * the store order interleaved */
    /* 0x194 */ Camera *prevCamera;
    /* 0x198 */ u8 pad198[0xD0];
    /* 0x268 */ f32 fadeBlackRate;    /* per-tick fade step = 1/duration */
    /* 0x26C */ f32 fadeBlackTarget;  /* fade-to level */
    /* 0x270 */ u8 fadeBlackTimer[4]; /* func_002832F8 re-arm timer */
    /* 0x274 */ f32 fadeWhiteRate;    /* white-flash per-tick step */
    /* 0x278 */ f32 fadeWhiteOutRate; /* alternate step clamp used while fadeWhiteTarget==0 (fade-out) */
    /* 0x27C */ f32 fadeWhiteTarget;  /* white flash-to level */
    /* 0x280 */ u8 fadeWhiteTimer[4]; /* func_002832F8 re-arm timer */
    /* 0x284 */ u8 pad284[0x14C];
    /* 0x3D0 */ f32 fovSpringVel;     /* spring-ease state (func_002703C0) */
    /* 0x3D4 */ f32 fovTarget;
    /* 0x3D8 */ f32 fovLive;          /* current FOV fed to the projection */
    /* 0x3DC */ f32 fovStiffness;     /* spring params (roles per func_002703C0) */
    /* 0x3E0 */ f32 fovDamping;
    /* 0x3E4 */ f32 fovMaxSpeed;
    /* 0x3E8 */ u8 fovMode;           /* 0 snap, 1 linear, 2 smooth, 3 spring */
    /* 0x3E9 */ u8 fovEnabled;        /* 1 = interpolation active */
    /* 0x3EA */ s16 fovTimer;         /* countdown (func_00283328) */
    /* 0x3EC */ f32 fovRate;          /* timer -> 0..1 scale */
    /* 0x3F0 */ f32 fovFrom;          /* ease start FOV */
    /* 0x3F4 */ u8 pad3F4[0xC];
    /* 0x400 */ s32 underwater;       /* set by CheckCameraUnderwater */
} CameraSysState;
extern CameraSysState g_cameraState;

typedef struct CameraTransitionState {
    /* 0x00 */ s16 state;             /* 1 = armed, 3 = blending; cleared on completion */
    /* 0x02 */ u8 kind;               /* g_bCameraTransitionKind (latched from pendingKind) */
    /* 0x03 */ u8 pendingKind;        /* requested blend kind for the next transition */
    /* 0x04 */ u8 pad4[0xC];
    /* 0x10 */ f32 blendA[6];         /* instant/blend-start state (func_00271140):
                                       * [0] progress, [1]<-[2], [3]=0, [4]<-[5] on kick */
    /* 0x28 */ u8 pad28[8];
    /* 0x30 */ Vec4 vec30;            /* seeded from cur1 on a kind-0 kick */
    /* 0x40 */ Vec4 vec40;            /* seeded from cur0 on a kind-0 kick */
    /* 0x50 */ Vec4 cur0;
    /* 0x60 */ Vec4 cur1;
    /* 0x70 */ f32 sphYaw;            /* running-blend state (func_002712E8) */
    /* 0x74 */ f32 sphPitch;
    /* 0x78 */ f32 sphDist;
    /* 0x7C */ s32 blendTarget;
    /* 0x80 */ f32 blendRate;         /* 1 / blendCount */
    /* 0x84 */ s32 blendCount;
    /* 0x88 */ u8 pad88[8];
    /* 0x90 */ Vec4 basisFwd;         /* normalised cine-key basis (func_00270D60) */
    /* 0xA0 */ Vec4 basisUp;
    /* 0xB0 */ Vec4 oriB;
    /* 0xC0 */ Vec4 src0;
    /* 0xD0 */ Vec4 src1;
} CameraTransitionState;
extern CameraTransitionState g_cameraTransitionState;

/* One queued screen-space sprite effect (0x30-byte stride). */
typedef struct ScreenSpriteFx {
    /* 0x00 */ volatile u_long128 worldPos; /* world-space anchor (when hasWorldPos) */
    /* 0x10 */ f32 x;                 /* direct screen x (when no world pos) */
    /* 0x14 */ u32 color;             /* RGBA, alpha capped by D_1A8630 */
    /* 0x18 */ s32 texId;             /* UI texture id */
    /* 0x1C */ f32 y;                 /* direct screen y */
    /* 0x20 */ void *owner;           /* owner moby (draw skipped when dead) */
    /* 0x24 */ s16 hasWorldPos;
    /* 0x26 */ s16 drawFlags;         /* 4 = caller-supplied mode/angle */
    /* 0x28 */ f32 angle;
    /* 0x2C */ s32 mode;              /* 0 burst / 1 mirrored quads / 2 single */
} ScreenSpriteFx;

typedef struct ScreenSpriteFxQueue {
    /* 0x000 */ ScreenSpriteFx entries[6];
    /* 0x120 */ s32 count;
} ScreenSpriteFxQueue;
extern ScreenSpriteFxQueue g_screenSpriteFxQueue;

/* Size pin (byte-neutral; asserts compile to nothing). ScreenSpriteFx is a
 * 0x30-byte queue element: confirmed by g_screenSpriteFxQueue.count sitting at
 * 0x120 (== 6 * 0x30) and by DrawScreenSpriteFxEntry's highest field read at
 * +0x2C (s32 mode). volatile u_long128 at +0 forces 16-byte alignment, and
 * 0x30 is already a multiple of 16. ee-gcc 2.9 predates __SIZEOF_POINTER__,
 * so the guard skips it on the matching (non-native) toolchain. */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(ScreenSpriteFx) == 0x30, "ScreenSpriteFx stride 0x30");
_Static_assert(__builtin_offsetof(ScreenSpriteFxQueue, count) == 0x120, "count");
#endif

extern void DecompressWad(s32 wadId, void *header);
extern void func_0011AEA0(s32 a);
extern void ResetFrameArenas(void);
extern void InstallVif1DmacHandlers(void);
extern void RemoveVif1DmacHandlers(void);
extern void func_002857C8(s32 a, s32 b, s32 c);
extern void AppendDrawEnvContext1(void);
extern void AppendDrawEnvContext2(void);
extern void AppendScreenClearPacket(s32 a);
extern u64 AppendTextureUploadBuildTex0(void *src, s32 vramDst, s32 w, s32 h);
extern void AppendFrameInitGsState(void);
extern void func_00285CE8(void);
extern void KickFrameDmaChain(void);
extern void WaitFrameDmaFence(s32 a);
extern void WaitGsPathsIdle(s32 a, s32 b);
extern void WaitVblankGetField(s32 a);
extern void DrawFullScreenTint(s32 a, s32 b, s32 c, s32 d);
extern s32 g_vramFrameBufB;

/* g_memoryArenaTable[0x14] points at the decompressed image chunk header used by
 * the splash/loading frame paths: header[0] = width, header[1] = height, the
 * pixel data starts at header + 0x10. */
typedef struct ImageChunkHeader {
    /* 0x00 */ s32 width;
    /* 0x04 */ s32 height;
    /* 0x08 */ u8 pad8[8];
    /* 0x10 */ u8 pixels[0];
} ImageChunkHeader;
extern void *g_memoryArenaTable[];   /* +0x14 (index 5) = image chunk header */

/* ShowSplashImage: decompress a still-image chunk and fade it in over 65 frames.
 * Uploads the image texture each frame, draws a full-screen tint quad whose
 * brightness ramps from 0x80 down past 0 (stepping by 2), and pumps the frame
 * DMA / vblank pipeline. Used for boot/menu stills. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", ShowSplashImage);
#else
/* TODO(match): functional equivalent - not byte-exact; three callee-saves at
   8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void ShowSplashImage(s32 wadId) {
    ImageChunkHeader *img = (ImageChunkHeader *)g_memoryArenaTable[5];
    s32 brightness = 0x80;

    DecompressWad(wadId, img);
    func_0011AEA0(0);

    do {
        ResetFrameArenas();
        img = (ImageChunkHeader *)g_memoryArenaTable[5];
        AppendTextureUploadBuildTex0((char *)img + 0x10, g_vramFrameBufB,
                                     img->width, img->height);
        AppendFrameInitGsState();
        DrawFullScreenTint(0, 0, 0, brightness);
        AppendDrawEnvContext2();
        brightness -= 2;
        func_00285CE8();
        func_0011AEA0(0);
        KickFrameDmaChain();
        WaitFrameDmaFence(1);
        WaitGsPathsIdle(0, 0);
        WaitVblankGetField(0);
    } while (brightness >= 0);
}
#endif

/* func_0026EAC8: render one fully pipelined frame of the current loading image.
 * Decompresses the image chunk, resets the frame arenas, installs the VIF1 DMA
 * handlers, builds and kicks a frame that uploads the image texture and clears
 * the screen, then waits for the frame to retire before tearing the handlers
 * back down. The single-frame sibling of ShowSplashImage's fade loop. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026EAC8);
#else
/* TODO(match): functional equivalent - not byte-exact; two callee-saves at
   8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void func_0026EAC8(s32 wadId) {
    ImageChunkHeader *img = (ImageChunkHeader *)g_memoryArenaTable[5];

    DecompressWad(wadId, img);
    func_0011AEA0(0);
    ResetFrameArenas();
    InstallVif1DmacHandlers();
    func_002857C8(0, 0, 0);
    AppendDrawEnvContext1();
    AppendScreenClearPacket(0);

    img = (ImageChunkHeader *)g_memoryArenaTable[5];
    AppendTextureUploadBuildTex0((char *)img + 0x10, g_vramFrameBufB,
                                 img->width, img->height);
    AppendFrameInitGsState();
    AppendDrawEnvContext2();
    func_00285CE8();
    func_0011AEA0(0);
    KickFrameDmaChain();
    WaitFrameDmaFence(1);
    WaitGsPathsIdle(0, 0);
    WaitVblankGetField(0);
    RemoveVif1DmacHandlers();
}
#endif

/* func_0026EB98: pick a randomized ordering of the three attract-reel slots.
 * A 3-way random roll seeds (a, b, c) with a base permutation of {0,1,2}, then a
 * second coin-flip optionally swaps b and c. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026EB98);
#else
extern s32 GetRandomInt(s32 range);
/* TODO(match): functional equivalent - not byte-exact; four callee-saves at
   8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void func_0026EB98(s32 *a, s32 *b, s32 *c) {
    s32 roll = GetRandomInt(3);

    if (roll == 1) {
        *a = 1;
        *b = 0;
        *c = 2;
    } else if (roll == 0) {
        *a = 0;
        *b = 1;
        *c = 2;
    } else if (roll == 2) {
        *a = 2;
        *b = 1;
        *c = 0;
    }

    if (GetRandomInt(2) != 0) {
        s32 t = *c;
        *c = *b;
        *b = t;
    }
}
#endif

/* BuildAttractReelPlaylist: seed the RNG from the RTC (sceCdReadClock) and build
 * a randomized attract-reel playlist. WALL: six callee-saves at 8-byte slot
 * spacing (the 0x20-vs-0x10 packed-save frame) plus the multi-field RNG-seed
 * mult chain; left INCLUDE_ASM. The TARGET_NATIVE #else is faithful coverage:
 * it seeds srand from (sec+1)(hour+1)(min+1)(day+1)(month+1) of the RTC clock,
 * then fills the reel-playlist struct with three randomized slot orderings
 * (func_0026EB98) offset by 0xB9 / and a coin-flipped 0xB3<->0xB6 pair. */
#ifdef TARGET_NATIVE
extern s32  sceCdReadClock(void *clock);
extern void srand(u32 seed);
void BuildAttractReelPlaylist(void *reelState) {
    u8 *out = (u8 *)reelState;
    u8  clk[8];
    s32 vals[3];
    u16 r0, r1, r2;
    s32 base2, base3;

    sceCdReadClock(clk);
    srand((clk[1] + 1) * (clk[3] + 1) * (clk[2] + 1) * (clk[5] + 1) * (clk[6] + 1));

    /* group 1: base 0xB9 (each value written to two slots) */
    func_0026EB98(&vals[0], &vals[1], &vals[2]);
    r0 = (u16)vals[0] + 0xB9;
    r1 = (u16)vals[1] + 0xB9;
    r2 = (u16)vals[2] + 0xB9;
    *(u16 *)(out + 0x20) = r0;
    *(u16 *)(out + 0x28) = r1;
    *(u16 *)(out + 0x30) = r2;
    *(u16 *)(out + 0x08) = r0;
    *(u16 *)(out + 0x10) = r1;
    *(u16 *)(out + 0x18) = r2;

    if (GetRandomInt(2) == 0) {
        base2 = 0xB6;
        base3 = 0xB3;
    } else {
        base2 = 0xB3;
        base3 = 0xB6;
    }

    /* group 2: base base2 (each value written to three slots) */
    func_0026EB98(&vals[0], &vals[1], &vals[2]);
    r0 = (u16)vals[0] + base2;
    r1 = (u16)vals[1] + base2;
    r2 = (u16)vals[2] + base2;
    *(u16 *)(out + 0x22) = r0;
    *(u16 *)(out + 0x26) = r1;
    *(u16 *)(out + 0x2C) = r2;
    *(u16 *)(out + 0x02) = r0;
    *(u16 *)(out + 0x06) = r1;
    *(u16 *)(out + 0x0C) = r2;
    *(u16 *)(out + 0x12) = r0;
    *(u16 *)(out + 0x16) = r1;
    *(u16 *)(out + 0x1C) = r2;

    /* group 3: base base3 */
    func_0026EB98(&vals[0], &vals[1], &vals[2]);
    r0 = (u16)vals[0] + base3;
    r1 = (u16)vals[1] + base3;
    r2 = (u16)vals[2] + base3;
    *(u16 *)(out + 0x24) = r0;
    *(u16 *)(out + 0x2A) = r1;
    *(u16 *)(out + 0x2E) = r2;
    *(u16 *)(out + 0x04) = r0;
    *(u16 *)(out + 0x0A) = r1;
    *(u16 *)(out + 0x0E) = r2;
    *(u16 *)(out + 0x14) = r0;
    *(u16 *)(out + 0x1A) = r1;
    *(u16 *)(out + 0x1E) = r2;
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", BuildAttractReelPlaylist);
#endif

/* LoadLevelAndInitHealth: large level-load + health/stat init driver. WALL:
 * jump-table switch + many callee-saves at 8-byte slot spacing (packed-save
 * wall); the body spans 660+ instructions with opaque sub-systems. Left
 * INCLUDE_ASM (not honestly decompilable without extensive cross-tracing). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", LoadLevelAndInitHealth);

/* func_0026F718: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F718(void) {
    return 0;
}

/* func_0026F720: stub-table entry — compiled-out hook, no-op. */
void func_0026F720(void) {
}

/* func_0026F728: stub-table entry — compiled-out hook, no-op. */
void func_0026F728(void) {
}

/* func_0026F730: stub-table entry — compiled-out hook, no-op. */
void func_0026F730(void) {
}

/* func_0026F738: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F738(void) {
    return 0;
}

/* func_0026F740: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F740(void) {
    return 0;
}

/* func_0026F748: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F748(void) {
    return 0;
}

/* func_0026F750: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F750(void) {
    return 0;
}

/* func_0026F758: stub-table entry — compiled-out hook, no-op. */
void func_0026F758(void) {
}

/* func_0026F760: stub-table entry — compiled-out hook, no-op. */
void func_0026F760(void) {
}

/* func_0026F768: stub-table entry — compiled-out hook, no-op. */
void func_0026F768(void) {
}

/* func_0026F770: stub-table entry — compiled-out hook, no-op. */
void func_0026F770(void) {
}

/* func_0026F778: stub-table entry — compiled-out hook, no-op. */
void func_0026F778(void) {
}

/* func_0026F780: stub-table entry — compiled-out hook, no-op. */
void func_0026F780(void) {
}

/* func_0026F788: stub-table entry — compiled-out hook, no-op. */
void func_0026F788(void) {
}

/* func_0026F790: stub-table entry — compiled-out hook, no-op. */
void func_0026F790(void) {
}

/* func_0026F798: stub-table entry — compiled-out hook, no-op. */
void func_0026F798(void) {
}

/* func_0026F7A0: stub-table entry — compiled-out hook, no-op. */
void func_0026F7A0(void) {
}

/* func_0026F7A8: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7A8(void) {
    return 0;
}

/* func_0026F7B0: 8-byte zero pad between stub-table entries. splat/spimdisasm
 * drops all-zero inter-function regions (no .s, no symbol), so the unit would be
 * 0x8 SHORT here and every downstream function would shift -0x8 — corrupting the
 * baked .word <func> pointer tables. Recover the pad as a raw-word INCLUDE_ASM
 * filler (06333c5 precedent) to keep the unit size==span byte-exact. NO re-split. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026F7B0);

/* func_0026F7B8: stub-table entry — compiled-out hook, no-op. */
void func_0026F7B8(void) {
}

/* func_0026F7C0: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7C0(void) {
    return 0;
}

/* func_0026F7C8: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7C8(void) {
    return 0;
}

/* func_0026F7D0: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7D0(void) {
    return 0;
}

/* func_0026F7D8: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7D8(void) {
    return 0;
}

/* func_0026F7E0: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7E0(void) {
    return 0;
}

/* func_0026F7E8: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7E8(void) {
    return 0;
}

/* func_0026F7F0: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7F0(void) {
    return 0;
}

/* func_0026F7F8: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F7F8(void) {
    return 0;
}

/* func_0026F800: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F800(void) {
    return 0;
}

/* func_0026F808: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F808(void) {
    return 0;
}

/* func_0026F810: stub-table entry — compiled-out hook, always returns 0. */
s32 func_0026F810(void) {
    return 0;
}

/* func_0026F818: stub-table entry — compiled-out hook, no-op. */
void func_0026F818(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026F820);

/* func_0026F850: large boot/menu still-image build helper. WALL: jump-table
 * switch + eight callee-saves at 8-byte slot spacing (packed-save wall). Left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026F850);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FC80);

/* func_0026FC88: upload a palettised texture (16x16 CLUT + image) to GS VRAM
 * and emit a 3-qword TEX0 packet into descOut. The source blob is a header:
 * +0x8 width, +0xC height, +0x14 CLUT pixel mode (0 = CT32 -> 0x400-byte CLUT,
 * else CT16 -> 0x200), CLUT data at +0x20, image data right after the CLUT.
 * Uploads the CLUT (16x16, clutVram>>8) then the image (PSM 0x1B, imageVram>>8,
 * buffer width max(1,w/64)), each via func_126288 descriptor + GIF-path image
 * kick + path idle; then packs TEX0: TBP/TBW/PSM 0x1B/TW/TH/TCC/CBP/CPSM +
 * bit 63, and descOut = { TEX0, 1, 0 }. */
#ifdef TARGET_NATIVE
extern void FillMemory32(void *dst, s32 val, s32 nbytes);
extern s32 Log2Floor(s32 x);
extern void func_126288(void *ctx, s32 bp, s32 bw, s32 psm, s32 x, s32 y,
                        s32 w, s32 h);
extern void KickGifImageUpload(void *ctx, void *data);
typedef struct GsTexUploadBlk {
    /* 0x00 */ void *clutData;
    /* 0x04 */ void *imageData;
    /* 0x08 */ u8 pad08[0xC];
    /* 0x14 */ s32 clutBytes;   /* 0x200 (CT16) or 0x400 (CT32) */
    /* 0x18 */ s32 pixelCount;
    /* 0x1C */ u8 pad1C[0x10];
    /* 0x2C */ s32 imageBp;     /* GS block pointer of the image */
    /* 0x30 */ u8 pad30[0xC];
    /* 0x3C */ s32 bufWidth;    /* TBW: max(1, width/64) */
    /* 0x40 */ u8 pad40[0xC];
    /* 0x4C */ s32 log2W;
    /* 0x50 */ s32 log2H;
} GsTexUploadBlk; /* 0x54 */
typedef struct GsTexBlobHeader {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ s32 width;
    /* 0x0C */ s32 height;
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ s32 clutPsm;     /* 0 = CT32, else CT16 */
    /* 0x18 */ u8 pad18[8];
    /* 0x20 */ u8 data[1];      /* CLUT then image (inline, size varies) */
} GsTexBlobHeader;
void func_0026FC88(void *src, void *descOut, s32 imageVram, s32 clutVram) {
    GsTexUploadBlk blk;
    u8 ctx[0x60];
    GsTexBlobHeader *tex = src;
    u64 *out = descOut;
    s32 clutBp = clutVram >> 8;
    u64 tex0;

    FillMemory32(&blk, 0, 0x54);
    blk.clutData = tex->data;
    blk.clutBytes = tex->clutPsm ? 0x200 : 0x400;
    blk.log2W = Log2Floor(tex->width);
    blk.log2H = Log2Floor(tex->height);
    blk.imageData = (u8 *)tex + blk.clutBytes + 0x20;
    blk.pixelCount = tex->width * tex->height;
    func_126288(ctx, (s16)clutBp, 1, (s16)tex->clutPsm, 0, 0, 0x10, 0x10);
    func_0011AEA0(0);
    KickGifImageUpload(ctx, blk.clutData);
    WaitGsPathsIdle(0, 0);
    blk.bufWidth = tex->width >> 6;
    if (blk.bufWidth <= 0) {
        blk.bufWidth = 1;
    }
    blk.imageBp = imageVram >> 8;
    func_126288(ctx, (s16)blk.imageBp, (s16)blk.bufWidth, 0x1B, 0, 0,
                (s16)tex->width, (s16)tex->height);
    func_0011AEA0(0);
    KickGifImageUpload(ctx, blk.imageData);
    WaitGsPathsIdle(0, 0);
    tex0 = (u64)(u32)blk.imageBp
         | ((u64)(u32)blk.bufWidth << 14)
         | ((u64)0x1B << 20)
         | ((u64)(u32)blk.log2W << 26)
         | ((u64)(u32)blk.log2H << 30)
         | ((u64)1 << 34)                  /* TCC = RGBA */
         | ((u64)(u32)clutBp << 37)        /* CBP */
         | ((u64)(u32)tex->clutPsm << 51)  /* CPSM */
         | ((u64)1 << 63);
    out[0] = tex0;
    out[1] = 1;
    out[2] = 0;
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FC88);
#endif

/* func_0026FE58: kick the boot dialog-voice file load then build the splash
 * image's texture-upload descriptor. Streams the voice chunk described by the
 * disc TOC WAD fields (+0x32C/+0x330/+0x334) into g_proceduralAnimFrames, then
 * calls func_0026FC88 to assemble a texture descriptor (VRAM base
 * g_vramTextureBase[+0x28], 0x3FFC00 bytes) and latches its handle word into the
 * raw-read state block (+0x34). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FE58);
#else
extern void StartFileLoadPumpingVoice(void *dst, s32 lbn, s32 size);
extern void func_0026FC88(void *src, void *descOut, s32 vramBase, s32 size);
extern u8 *g_proceduralAnimFrames;
extern s32 g_discToc[];                 /* +0x32C/+0x330/+0x334 WAD fields */
extern s32 g_vramTextureBase_28;        /* g_vramTextureBase + 0x28 */
extern u64 g_bRawReadFellBack_34;       /* g_bRawReadFellBack + 0x34 */
/* TODO(match): functional equivalent - not byte-exact; two callee-saves at
   8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void func_0026FE58(void) {
    /* IN-LEVEL native frame-path: out-of-scope driven-frame TRAP (tester
     * inlevel_trap_list.md). Its #else is a blocking voice/file load
     * (StartFileLoadPumpingVoice -> IOP RPC, deadlocks headless) plus a GS
     * texture upload - not seed-reclaimable. No-op (blocking-load contract, like
     * the other in-level load/render traps). Reconstruction in git (commit
     * 1fd00c0). NOTE: this is a file/texture-load trap, NOT an activeCamera-deref
     * despite the trap-list grouping. */
}
#endif

/* DebugPrintStub: varargs debug-print hook, compiled to a no-op in retail —
 * only the EABI varargs register spill remains, and crucially the ORIGINAL
 * also spills the FP argument registers $f12/$f14/$f16/$f18 (a printf-style
 * float-varargs prologue). The pinned cc1 emits the GPR varargs spill but
 * never the FP spill for a `(char*, ...)` body, so this cannot match from C.
 * WALL: float-varargs register-spill prologue. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", DebugPrintStub);
#else
/* Retail no-op: the body only spills its varargs registers and returns; no
 * memory writes outside its own frame, no output, return value ignored.
 * (Analysis-confirmed leaf no-op @0x26FEC8.) */
s32 DebugPrintStub(const char *fmt, ...) {
    (void)fmt;
    return 0;
}
#endif

/* func_0026FF00: MIS-SPLIT fragment. The .s opens with six words
 * (sw $2,0x38($4) / nop / addiu $sp,0x10 / nop / addiu $sp,0x20 / nop) that are
 * the TAIL of the preceding function, then `alabel func_0026FF18` — the real
 * entry. The matching build keeps the whole tile INCLUDE_ASM (dropping the
 * head words would shift the layout); the native build gets the REAL function
 * below under its interior name. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FF00);
#else
/* func_0026FF18: start a camera-FOV ease — the setter feeding
 * StepCameraFovInterp. mode 0 = snap to `fov` next tick; modes 1/2 = timed
 * linear/smooth ease over `frames` (0 frames degrades to a snap): arms the
 * countdown, latches fovFrom=fovLive and rate=1/frames; mode 3 = critically-
 * damped spring with the given stiffness/damping/maxSpeed. Other modes: no-op.
 * All arming paths set fovEnabled=1. */
extern f32 IntToFloat(s32 x);
void func_0026FF18(s32 frames, s32 mode, f32 fov, f32 stiffness, f32 damping,
                   f32 maxSpeed) {
    switch (mode) {
    case 0:
        g_cameraState.fovEnabled = 1;
        g_cameraState.fovTarget = fov;
        g_cameraState.fovMode = 0;
        break;
    case 1:
    case 2:
        if (frames == 0) {
            g_cameraState.fovTarget = fov;
            g_cameraState.fovMode = 0;
        } else {
            g_cameraState.fovTimer = (s16)frames;
            g_cameraState.fovFrom = g_cameraState.fovLive;
            g_cameraState.fovMode = (u8)mode;
            g_cameraState.fovRate = 1.0f / IntToFloat(frames);
        }
        g_cameraState.fovEnabled = 1;
        break;
    case 3:
        g_cameraState.fovMaxSpeed = maxSpeed;
        g_cameraState.fovMode = (u8)mode;
        g_cameraState.fovTarget = fov;
        g_cameraState.fovEnabled = 1;
        g_cameraState.fovStiffness = stiffness;
        g_cameraState.fovDamping = damping;
        break;
    default:
        break;
    }
}
#endif

/* StepCameraFovInterp: camera FOV interpolation state machine (mode at
 * g_cameraState +0x3E8, enable flag +0x3E9). Eases the live FOV (+0x3D8) toward
 * its target by the active easing mode (spring func_002703C0, linear, smooth
 * func_002A8A68), then rebuilds the projection (BuildCameraProjection,
 * g_cameraProjScale = tan(fov*0.5) = sin/cos of the half-angle). WALL: four fp/gpr callee-saves at
 * 8-byte slot spacing (packed-save wall) + branch-likely-driven fp control flow.
 * Left INCLUDE_ASM. */
#ifdef TARGET_NATIVE
extern s32 func_00283328(s16 *timer);
extern f32 IntToFloat(s32 x);
extern f32 func_002A8A68(f32 target, f32 from, f32 s);
extern f32 func_002703C0(f32 cur, f32 target, f32 stiffness, f32 damping,
                         f32 maxSpeed, f32 *vel); /* def later in this unit */
extern f32 GetFloatAbs(f32 x);
extern f32 func_00283B48(f32 x); /* sin */
extern f32 func_00283B30(f32 x); /* cos */
extern void BuildCameraProjection(void);
extern f32 g_cameraProjScale;
void StepCameraFovInterp(void) {
    if (g_cameraState.fovEnabled != 1) {
        return;
    }
    switch (g_cameraState.fovMode) {
    case 0: /* snap */
        g_cameraState.fovEnabled = 0;
        g_cameraState.fovLive = g_cameraState.fovTarget;
        break;
    case 1: { /* linear from fovFrom toward fovTarget over the timer */
        f32 s;
        if (func_00283328(&g_cameraState.fovTimer) != 0) {
            g_cameraState.fovEnabled = 0;
        }
        s = IntToFloat(g_cameraState.fovTimer) * g_cameraState.fovRate;
        g_cameraState.fovLive = g_cameraState.fovTarget +
            (g_cameraState.fovFrom - g_cameraState.fovTarget) * s;
        break;
    }
    case 2: /* smooth-step ease */
        if (func_00283328(&g_cameraState.fovTimer) != 0) {
            g_cameraState.fovEnabled = 0;
        }
        g_cameraState.fovLive = func_002A8A68(
            g_cameraState.fovTarget, g_cameraState.fovFrom,
            IntToFloat(g_cameraState.fovTimer) * g_cameraState.fovRate);
        break;
    case 3: /* critically-damped spring; settle when the velocity dies */
        g_cameraState.fovLive = func_002703C0(g_cameraState.fovLive,
            g_cameraState.fovTarget, g_cameraState.fovStiffness,
            g_cameraState.fovDamping, g_cameraState.fovMaxSpeed,
            &g_cameraState.fovSpringVel);
        if (GetFloatAbs(g_cameraState.fovSpringVel) < 1e-4f) {
            g_cameraState.fovEnabled = 0;
        }
        break;
    default:
        break;
    }
    {
        f32 half = g_cameraState.fovLive * 0.5f;
        g_cameraProjScale = func_00283B48(half) / func_00283B30(half); /* tan(fov/2) */
        BuildCameraProjection();
    }
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", StepCameraFovInterp);
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002701B8);

/* func_002701C0: snapshot the active camera. Copies the live 0xA0-byte active
 * camera slot and a following 0x280-byte block into save buffers, then points
 * the save header at the copied block. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002701C0);
#else
extern void CopyQwords(void *dst, const void *src, s32 nbytes);
extern u8 g_cameraSnapshot[0xA0];        /* g_cameraSlots + 0x1E00 (0x1B74D0) */
extern u8 g_cameraHistory[0x280];        /* live camera history (0x1B76B0) */
extern u8 g_cameraHistorySnapshot[0x280]; /* saved copy (0x1B7BB0) */
extern void *g_cameraSnapshotPtr;        /* snapshot header + 0x70 (0x1B7540) */
/* TODO(match): functional equivalent - not byte-exact; three callee-saves at
   8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void func_002701C0(void) {
    CopyQwords(g_cameraSnapshot, (const void *)g_cameraState.activeCamera, 0xA0);
    CopyQwords(g_cameraHistorySnapshot, g_cameraHistory, 0x280);
    g_cameraSnapshotPtr = g_cameraHistorySnapshot;
}
#endif

/* func_00270220: run every queued one-shot camera callback. D_001B1300+0x180
 * holds the queued-count; D_001B1300+0x140 is the function-pointer array. Calls
 * each pointer in turn (re-reading the live count each iteration so a callback
 * may shorten the queue), then clears the count back to 0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270220);
#else
extern s32 g_cameraCallbackCount;          /* D_001B1300 + 0x180 */
extern void (*g_cameraCallbacks[])(void);  /* D_001B1300 + 0x140 */
/* TODO(match): functional equivalent - not byte-exact; three callee-saves at
   8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void func_00270220(void) {
    /* IN-LEVEL native frame-path: out-of-scope driven-frame TRAP (tester
     * inlevel_trap_list.md). It calls every g_cameraCallbacks[] slot, which
     * in-level hold OVERLAY function pointers (registered by overlay code), and
     * its loop bound g_cameraCallbackCount (0x1B1480) is in the relocated band so
     * reads garbage at the static address - faults/runaway on real EE. No-op:
     * the camera-callback dispatch is overlay-driven in-level. Reconstruction in
     * git (commit 1fd00c0). */
}
#endif

/* func_00270290: find the first camera slot whose type matches `type`
 * (48-slot linear scan); returns NULL when no slot matches. */
Camera *func_00270290(s32 type) {
    Camera *cam = g_cameraSlots;

    do {
        if (cam->type == type) {
            return cam;
        }
        cam++;
    } while ((s32)cam < (s32)&g_cameraSlots[48]); /* signed compare pins the
                                                   * original slt (sltu with a
                                                   * plain pointer compare) */
    return NULL;
}

/* func_002702C8: no-return handwritten table-word fragment (see header
 * STUB-TABLE note) - not reachable compiler output. Kept permanently
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002702C8);

/* func_002702D8: critically-damped spring step toward a target. Advances `cur`
 * (the velocity at *vel) by stiffness*delta - damping*vel, clamps the velocity
 * magnitude to maxSpeed (when nonzero) and then to |delta|, and returns the new
 * position cur + clampedVel. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002702D8);
#else
extern f32 GetFloatAbs(f32 x);
/* TODO(match): functional equivalent - not byte-exact; the original reloads
   GetFloatAbs(delta) at each branch and threads the fp pipeline differently. */
f32 func_002702D8(f32 cur, f32 target, f32 stiffness, f32 damping, f32 maxSpeed,
                  f32 *vel) {
    f32 delta = target - cur;
    f32 v = *vel + (stiffness * delta - damping * *vel);
    *vel = v;
    if (maxSpeed != 0.0f) {
        if (v > maxSpeed) {
            *vel = maxSpeed;
        } else if (v < -maxSpeed) {
            *vel = -maxSpeed;
        }
    }
    if (GetFloatAbs(delta) < *vel) {
        *vel = GetFloatAbs(delta);
    } else if (-GetFloatAbs(delta) > *vel) {
        *vel = -GetFloatAbs(delta);
    }
    return cur + *vel;
}
#endif

/* func_002703C0: angular twin of func_002702D8 - critically-damped spring on a
 * wrapped angle. delta is the shortest signed angular difference target-cur
 * (WrapAnglePiDiff); the velocity is integrated, clamped to maxSpeed and to
 * |delta|, and the result is cur + vel re-wrapped into (-pi,pi] (WrapAnglePiSum). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002703C0);
#else
extern f32 WrapAnglePiDiff(f32 a, f32 b);
extern f32 WrapAnglePiSum(f32 a, f32 b);
/* TODO(match): functional equivalent - not byte-exact; five fp callee-saves
   ($f20-$f24) + the GetFloatAbs reload pattern (same wall as func_002702D8). */
f32 func_002703C0(f32 cur, f32 target, f32 stiffness, f32 damping, f32 maxSpeed,
                  f32 *vel) {
    f32 delta = WrapAnglePiDiff(target, cur);
    f32 v = *vel + (stiffness * delta - damping * *vel);
    *vel = v;
    if (maxSpeed != 0.0f) {
        if (v > maxSpeed) {
            *vel = maxSpeed;
        } else if (v < -maxSpeed) {
            *vel = -maxSpeed;
        }
    }
    if (GetFloatAbs(delta) < *vel) {
        *vel = GetFloatAbs(delta);
    } else if (-GetFloatAbs(delta) > *vel) {
        *vel = -GetFloatAbs(delta);
    }
    return WrapAnglePiSum(cur, *vel);
}
#endif

/* func_002704E0: flag the active camera slot as freshly (re)activated — sets
 * the activation halfword (+0x7E = 1) and clears the settle byte (+0x7D = 0)
 * on g_cameraState.activeCamera. The original RE-READS the active-camera
 * pointer for the second store (lw v1,400; sh; lw a0,400; sb in the jr delay
 * slot). Cc1 either CSEs the reload (plain field) or, made volatile to force
 * the reload, pins the stores in noreorder brackets so the second store can't
 * fill the jr delay slot — neither reproduces the original interleave.
 * Best 80%. WALL: volatile-reload vs delay-slot scheduling. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002704E0);
#else
/* TODO(match): functional equivalent - not byte-exact; volatile-reload vs
   delay-slot scheduling (the original re-reads activeCamera for the second
   store and fills the jr delay slot with it). */
void func_002704E0(void) {
    g_cameraState.activeCamera->unk7E = 1;
    g_cameraState.activeCamera->unk7D = 0;
}
#endif

/* func_00270500: maintain a helper moby tied to a camera slot. When the slot's
 * type field (+0x86) is zero, lazily spawn the helper moby (func_00303818 with
 * the camera base) into g_cameraHelperMoby; when nonzero, free it if present.
 * Note: func_00303818 takes the camera base (cam - 0x60 from the +0x60 field
 * pointer the caller passes). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270500);
#else
extern void *func_00303818(void *cameraBase);
extern void *g_cameraHelperMoby;
/* TODO(match): functional equivalent - not byte-exact; two callee-saves at
   8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void func_00270500(Camera *cam) {
    if (cam->type == 0) {
        if (g_cameraHelperMoby == NULL) {
            g_cameraHelperMoby = func_00303818((char *)cam - 0x60);
        }
    } else if (g_cameraHelperMoby != NULL) {
        FreeMoby(g_cameraHelperMoby);
        g_cameraHelperMoby = NULL;
    }
}
#endif

/* CallCameraEnterHandler: invoke the camera-mode vtbl `enter` handler
 * (slot +0x8) for the camera's mode id, if the level overlay installed one. */
void CallCameraEnterHandler(Camera *cam) {
    s32 (*handler)() = g_cameraModeVtbl[cam->modeId].enter;

    if (handler != NULL) {
        handler(cam);
    }
}

/* SwitchActiveCamera: activate a new camera slot - pick transition kind/duration
 * (default 0.018), save the old camera into g_prevCamera with a history copy,
 * reset the frames-since-cut counter, run the enter handler and snap g_cameraPos
 * to the new slot pos +0x30. WALL: deep nested branching with multiple
 * callee-saves at 8-byte slot spacing (packed-save wall); 0x2A0 bytes. Left
 * INCLUDE_ASM (too large to honestly decompile without subtle ordering risk). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", SwitchActiveCamera);

/* Camera takeover trigger-table row (pointed to by the word at
 * g_cameraCallbackCount+4, 0x1B1484; 32-byte rows indexed by
 * Camera.configIndex). Row +0x1C -> trigger object: +0xC = volume pointer
 * passed to func_002ADA30 against g_heroPos, +0x24 = link index into the
 * D_001B1300 pointer-table rows (<0 = none). Semantics UNCONFIRMED beyond
 * offsets (deferred Track-B naming: the 0x1B1484 slot needs a symbol pin). */
typedef struct CamTrigObj {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ void *volume;
    /* 0x10 */ u8 pad10[0x14];
    /* 0x24 */ s32 linkIdx;
} CamTrigObj;
typedef struct CamTakeRow {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ CamTrigObj *obj;
} CamTakeRow; /* 0x20 stride */

/* TestCameraTakeover: decide whether a candidate camera slot should take over
 * the active camera (mode-vtbl takeover handler + a per-type priority switch).
 * WALL: jump-table switch (jtbl_0026C4F0_text) + three callee-saves at 8-byte
 * slot spacing (packed-save wall). Left INCLUDE_ASM. */
#ifdef TARGET_NATIVE
extern s32 g_cameraCallbackCount;
extern char g_soundBankHandlesBlk[]; /* g_soundBankHandles + 0x20 */
extern s32 func_002ADA30(void *pos, void *volume);
extern void *D_001B1300; /* base pointer of the 32-byte-row link table */
extern Vec4 g_heroPos;   /* 0x189EA0 hero world position */
s32 TestCameraTakeover(Camera *cand, Camera *cur) {
    /* the trigger-row table lives in the word AFTER g_cameraCallbackCount
     * (0x1B1484, unnamed - deferred Track-B pin) */
    CamTakeRow *rows;
    s32 (*handler)();
    s32 r;

    if (cand->priority == 0) {
        return 0;
    }
    handler = g_cameraModeVtbl[cand->modeId].takeover;
    if (handler != 0) {
        r = handler(cand, cur);
        if (r == -1) {
            return 0;
        }
        if (r == 1) {
            return 1;
        }
    }
    switch (cand->takeKind) {
    case 1:
    case 2:
        if (cand->unk7D == 0) {
            return 0;
        }
        /* fall through */
    case 0:
        if (cur == 0) {
            return 1;
        }
        if (cur->unk7E != 0) {
            return 1;
        }
        return cur->priority < cand->priority;
    case 4: {
        CamTrigObj *obj;
        rows = ((CamTakeRow **)&g_cameraCallbackCount)[1];
        obj = rows[cand->configIndex].obj;
        if (cur != 0 && cur->unk7E == 0) {
            if (!(cur->priority < cand->priority)) {
                return 0;
            }
        }
        return func_002ADA30(&g_heroPos, obj->volume) != 0;
    }
    case 7: {
        CamTrigObj *obj;
        if (cand->type != *(s32 *)(g_soundBankHandlesBlk + 0x24A0)) {
            return 0;
        }
        /* NOTE: the original dereferences cur with NO null check here */
        if (cur->unk7E == 0) {
            if (!(cur->priority < cand->priority)) {
                return 0;
            }
        }
        if (cand->type != 3) {
            return 1;
        }
        rows = ((CamTakeRow **)&g_cameraCallbackCount)[1];
        obj = rows[cand->configIndex].obj;
        if (obj->linkIdx < 0) {
            return 1;
        }
        if (*(s32 *)(g_soundBankHandlesBlk + 0x630) !=
            *(s32 *)((char *)D_001B1300 + (obj->linkIdx << 5) + 0x10)) {
            return 0;
        }
        return *(s32 *)(g_soundBankHandlesBlk + 0x640) != 0;
    }
    default: /* kinds 3, 5, 6 */
        return 0;
    }
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", TestCameraTakeover);
#endif

/* CallCameraPollHandler: invoke the camera-mode vtbl `poll` handler
 * (slot +0x10) for the camera's mode id, if the level overlay installed one. */
void CallCameraPollHandler(Camera *cam) {
    s32 (*handler)() = g_cameraModeVtbl[cam->modeId].poll;

    if (handler != NULL) {
        handler(cam);
    }
}

/* DispatchCameraMode: per-frame camera arbitration + mode update. Polls the
 * active camera, scans all 48 slots for a higher-priority active camera
 * (TestCameraTakeover), switches to it on a win, then runs that camera's mode
 * `update` handler and records its post-update position into the prev-pos
 * fields (+0x64/+0x68/+0x6c). Always returns -1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", DispatchCameraMode);
#else
extern s32 g_cameraSlotActive[48];
extern void func_00270500(Camera *cam);
extern void func_00270220(void);
extern s32 TestCameraTakeover(Camera *candidate, Camera *current);
extern void SwitchActiveCamera(Camera *cam);
/* TODO(match): functional equivalent - not byte-exact; many callee-saves at
   8-byte slot spacing + the unnamed-vtbl base access (packed-save wall). */
s32 DispatchCameraMode(void) {
    Camera *chosen = (Camera *)g_cameraState.activeCamera;
    s32 changed = 0;
    s32 i;
    s32 (*update)();
    f32 *pos;

    CallCameraPollHandler((Camera *)g_cameraState.activeCamera);

    for (i = 0; i < 48; i++) {
        Camera *slot = &g_cameraSlots[i];
        if (g_cameraSlotActive[i] != 0 && slot != chosen &&
            TestCameraTakeover(slot, chosen) != 0) {
            changed = 1;
            chosen = slot;
        }
    }

    if (changed) {
        SwitchActiveCamera(chosen);
    }

    update = g_cameraModeVtbl[chosen->modeId].update;
    func_00270500(chosen);

    pos = (f32 *)((char *)chosen + 0x30);
    if (update != NULL) {
        update(chosen);
    }
    /* prev-pos snapshot at +0x64/+0x68/+0x6c */
    *(f32 *)((char *)chosen + 0x64) = pos[0];
    *(f32 *)((char *)chosen + 0x68) = pos[1];
    *(f32 *)((char *)chosen + 0x6c) = pos[2];

    func_00270220();
    return -1;
}
#endif

/* func_00270B68: camera-transition curve builder (consumed by func_00270D60).
 * Builds the blend basis/control points from the three normalised key vectors.
 * WALL: thirteen callee-saves at 8-byte slot spacing (packed-save wall) +
 * heavy interleaved fp/qword math. Left INCLUDE_ASM. */
#ifdef TARGET_NATIVE
extern void Vec4SubVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern f32 Vec3DotVu0(const Vec4 *a, const Vec4 *b);
extern f32 Vec3LengthVu0(const Vec4 *v);
extern void Vec3RescaleToLenVu0(Vec4 *dst, f32 len, const Vec4 *src);
extern f32 func_00283B60(f32 x); /* acos-family (PI/2 - result below) */
extern void func_002ADD28(void *dst, const Vec4 *v, const Vec4 *axis, f32 ang);
/* Decompose the offset posA-posB into camera-relative spherical coords in the
 * (fwd,right,up) basis: out->x = signed azimuth around `up` (sign from
 * `right`), out->y = signed elevation (sign from `up` against the offset
 * direction), out->z = offset length. Zero-length guards clamp the divisor to
 * 1e-4 like the original. */
void func_00270B68(void *out_, void *posA, void *posB, Vec4 *fwd, Vec4 *right,
                   Vec4 *up) {
    Vec4 d;      /* posA - posB */
    Vec4 axial;  /* component of d along up */
    Vec4 perp;   /* d with the axial part removed */
    Vec4 unit;   /* scratch unit vector */
    Vec4 rot;    /* fwd rotated by the azimuth */
    f32 *out = out_;
    f32 dot, len, ang;

    Vec4SubVu0(&d, posA, posB);
    dot = Vec3DotVu0(&d, up);
    Vec3RescaleToLenVu0(&axial, dot, up);
    Vec4SubVu0(&perp, &d, &axial);
    dot = Vec3DotVu0(fwd, &perp);
    len = Vec3LengthVu0(&perp);
    if (len == 0.0f) {
        len = 1e-4f;
    }
    ang = 1.5707964f - func_00283B60(dot / len);
    Vec3RescaleToLenVu0(&unit, 1.0f, &perp);
    if (Vec3DotVu0(right, &unit) < 0.0f) {
        ang = -ang;
    }
    out[0] = ang;
    func_002ADD28(&rot, fwd, up, ang);
    dot = Vec3DotVu0(&rot, &d);
    len = Vec3LengthVu0(&d);
    if (len == 0.0f) {
        len = 1e-4f;
    }
    ang = 1.5707964f - func_00283B60(dot / len);
    Vec3RescaleToLenVu0(&unit, 1.0f, &d);
    /* SIGN IDIOM (audit catch): this arm is `bc1f` + ALWAYS-EXECUTED delay
     * neg + fallthrough mov -> negate when dot >= 0. (out[0] above is the
     * OPPOSITE shape: `bc1tl` + annulled delay -> negate when dot < 0.
     * Branch-likely-nullified vs always-execute-delay sign-selects look
     * identical in C but mean opposite things.) */
    if (Vec3DotVu0(up, &unit) >= 0.0f) {
        ang = -ang;
    }
    out[1] = ang;
    out[2] = Vec3LengthVu0(&d);
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270B68);
#endif

/* func_00270D60: re-seed the spherical running-blend state from the cine-key
 * basis: normalise the three key rows (keys+0xC0/D0/E0), latch fwd/up into
 * basisFwd/basisUp, decompose the src0-vs-(g_soundBankHandlesBlk+0x80) offset
 * via func_00270B68 into sph*, and mirror src1 into oriB. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270D60);
#else
extern void func_00270B68(void *a, void *b, void *c, Vec4 *d, Vec4 *e, Vec4 *f);
/* arg order is (dst, len, src) - matches the native definition in text/183558
 * (Vec4ScaleVu0/Vec3RescaleToLenVu0 take the f32 scalar BEFORE the src ptr).
 * The f32 lands in $f12 regardless of position, so the EE/EABI build is
 * unaffected; native ILP32 is positional, so the scalar must come second. */
extern void Vec3RescaleToLenVu0(Vec4 *dst, f32 len, const Vec4 *src);
extern char g_soundBankHandlesBlk[]; /* g_soundBankHandles + 0x20 */
/* TODO(match): functional equivalent - not byte-exact; five callee-saves
   (incl. $f20) at 8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void func_00270D60(void) {
    CameraTransitionState *t = &g_cameraTransitionState;
    /* +0x2290 holds the live cinematic camera-key block pointer (reloaded each
     * use); +0x80 is a separate control field forwarded to the curve builder. */
    char *keys = *(char **)(g_soundBankHandlesBlk + 0x2290);
    Vec4 v0, v1, v2;

    Vec3RescaleToLenVu0(&v0, 1.0f, (Vec4 *)(keys + 0xC0));
    keys = *(char **)(g_soundBankHandlesBlk + 0x2290);
    Vec3RescaleToLenVu0(&v1, 1.0f, (Vec4 *)(keys + 0xD0));
    keys = *(char **)(g_soundBankHandlesBlk + 0x2290);
    Vec3RescaleToLenVu0(&v2, 1.0f, (Vec4 *)(keys + 0xE0));

    t->basisFwd = v0;
    t->basisUp = v2;

    func_00270B68(&t->sphYaw, &t->src0,
                  g_soundBankHandlesBlk + 0x80, &v0, &v1, &v2);

    t->oriB = t->src1;
}
#endif

/* func_00270E40: when no transition is pending (kind == 0), seed the saved
 * source pair from the current pair: src0 = cur0, src1 = cur1. If the
 * transition mode byte (+0x3) is 2, src0 is first offset by the hero-relative
 * delta (g_heroPos + 0xD0) before being saved. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270E40);
#else
extern void Vec4AddVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern Vec4 g_heroPos_D0;   /* g_heroPos + 0xD0 */
/* TODO(match): functional equivalent - not byte-exact; two callee-saves at
   8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void func_00270E40(void) {
    CameraTransitionState *t = &g_cameraTransitionState;

    if (t->kind == 0) {
        t->src0 = t->cur0;
        if (t->pendingKind == 2) {
            Vec4AddVu0(&t->src0, &g_heroPos_D0, &t->src0);  /* asm arg order: a=delta (its .w is kept), b=src0 */
        }
        t->src1 = t->cur1;
    }
}
#endif

/* func_00270EB8: while a camera transition is pending (kind != 0), snap the
 * transition's current qword pos/target pair (+0x50/+0x60) from the saved
 * source pair (+0xC0/+0xD0). The original loads each 16-byte block through a
 * SEPARATE address register (addiu a0,+0x50; addiu v1,+0xC0; lq 0(v1);
 * sq 0(a0); …) and interleaves the two copies (store of the first before the
 * load of the second). The pinned cc1 always emits offset-form lq/sq off the
 * base register (lq 192(a2); sq 80(a2); …) and orders the copies
 * sequentially — the offset-vs-register addressing is a fixed cc1 choice no
 * source shape overrides. Best 59%. WALL: qword block-copy addressing form. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270EB8);
#else
/* TODO(match): functional equivalent - not byte-exact; the original uses
   per-block address registers + interleaved lq/sq, cc1 emits offset-form
   lq/sq off the base register (qword block-copy addressing form). */
void func_00270EB8(void) {
    if (g_cameraTransitionState.kind != 0) {
        g_cameraTransitionState.cur0 = g_cameraTransitionState.src0;
        g_cameraTransitionState.cur1 = g_cameraTransitionState.src1;
    }
}
#endif

/* BeginCameraTransition: kick a camera-to-camera blend into `active`. With the
 * transition armed (state==1) it captures the blend sources by pendingKind:
 * kind 0 = cur pair from the camera (pos row +0x30 + func_002AC468
 * orientation), kind 2 = src pair + func_00270D60, others = spherical path
 * (normalise the three cine-key basis rows at keys+0xC0/D0/E0, decompose
 * cam-vs-hook offset via func_00270B68 into sph*, capture oriB and mirror it
 * into src1). Un-armed: kind 2 = func_00270E40+func_00270D60, kind 1 =
 * func_00270E40 + oriB<-src1, kind 0 = func_00270EB8. Then state=3, kind is
 * latched, and the interpolator state is seeded: kind 0 primes blendA and the
 * vec30/vec40 pair from cur0/cur1; other kinds bump blendCount and set
 * blendTarget/blendRate = 1/count. The second parameter is unused (kept for
 * the established caller signature). */
#ifdef TARGET_NATIVE
extern void func_002AC468(void *dstOri, Camera *cam);
extern void func_00270D60(void);
extern void func_00270E40(void);
extern void func_00270EB8(void);
void BeginCameraTransition(Camera *active, Camera *prev) {
    CameraTransitionState *ts = &g_cameraTransitionState;
    Vec4 fwd, right, up;

    (void)prev;
    if (ts->state == 1) {
        if (ts->pendingKind == 0) {
            ts->cur0 = *(Vec4 *)((u8 *)active + 0x30);
            func_002AC468(&ts->cur1, active);
        } else if (ts->pendingKind == 2) {
            ts->src0 = *(Vec4 *)((u8 *)active + 0x30);
            func_002AC468(&ts->src1, active);
            func_00270D60();
        } else {
            char *keys = *(char **)(g_soundBankHandlesBlk + 0x2290);
            /* camera hook object: pointer stored 0x100 BEFORE the transition
             * state block (deferred Track-B pin) */
            u8 *hook = *(u8 **)((u8 *)&g_cameraTransitionState - 0x100);

            Vec3RescaleToLenVu0(&fwd, 1.0f, (Vec4 *)(keys + 0xC0));
            Vec3RescaleToLenVu0(&right, 1.0f, (Vec4 *)(keys + 0xD0));
            Vec3RescaleToLenVu0(&up, 1.0f, (Vec4 *)(keys + 0xE0));
            func_00270B68(&ts->sphYaw, (u8 *)active + 0x30, hook + 0x30,
                          &fwd, &right, &up);
            func_002AC468(&ts->oriB, active);
            ts->src1 = ts->oriB;
        }
    } else {
        if (ts->pendingKind == 2) {
            func_00270E40();
            func_00270D60();
        } else if (ts->pendingKind == 1) {
            func_00270E40();
            ts->oriB = ts->src1;
        } else if (ts->pendingKind == 0) {
            func_00270EB8();
        }
    }
    ts->state = 3;
    ts->kind = ts->pendingKind;
    if (ts->pendingKind == 0) {
        ts->blendA[3] = 0.0f;
        ts->blendA[4] = ts->blendA[5];
        ts->vec40 = ts->cur0;
        ts->blendA[0] = 0.0f;
        ts->blendA[1] = ts->blendA[2];
        ts->vec30 = ts->cur1;
    } else {
        ts->blendCount += 1;
        ts->blendTarget = ts->blendCount;
        ts->blendRate = 1.0f / IntToFloat(ts->blendCount);
    }
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", BeginCameraTransition);
#endif

/* func_00271140: instant-settle / blend-start branch of the transition pipeline
 * (forwarded from ApplyCameraTransition when no blend is pending). WALL: nine
 * fp/gpr callee-saves at 8-byte slot spacing (packed-save wall) + interleaved
 * Vec4 math and func_002A8A68/func_002AC468 calls. Left INCLUDE_ASM. */
#ifdef TARGET_NATIVE
/* View of CameraTransitionState+0x10: two smooth-stepped blend tracks
 * (orientation at +0x0, position at +0xC) with per-track step + latched step,
 * the from-pair at +0x20/+0x30 and the out-pair at +0x40/+0x50 (aliasing
 * cur0/cur1 in the parent struct). */
typedef struct CamBlendState {
    /* 0x00 */ f32 oriProgress;
    /* 0x04 */ f32 oriStep;
    /* 0x08 */ f32 oriStepLatch;
    /* 0x0C */ f32 posProgress;
    /* 0x10 */ f32 posStep;
    /* 0x14 */ f32 posStepLatch;
    /* 0x18 */ u8 pad18[8];
    /* 0x20 */ Vec4 oriFrom;
    /* 0x30 */ Vec4 posFrom;
    /* 0x40 */ Vec4 posOut;
    /* 0x50 */ Vec4 oriOut;
} CamBlendState;
extern Vec4 g_cameraPos;      /* 0x1B52C0; camera matrix at +0x230 */
extern Vec4 g_heroPos_D0;     /* g_heroPos + 0xD0 per-frame hero delta */
extern void func_002841C0(void *dst, void *from, void *to, f32 t); /* ori lerp */
extern void func_00284380(void *ori, void *mtxOut);
extern void func_00284028(void *mtxDst, void *mtxSrc);
/* Instant-settle / blend-start interpolator (kind-0 path of
 * ApplyCameraTransition). Returns 1 once BOTH tracks have reached 1.0;
 * otherwise drifts the position anchor by the hero frame delta
 * (g_heroPos_D0), smooth-step lerps position (into posOut and g_cameraPos)
 * and orientation (from oriFrom toward the live camera orientation, into
 * oriOut then the g_cameraMatrix at g_cameraPos+0x230), advances both
 * progress tracks clamped to 1.0, and returns 0. */
s32 func_00271140(Vec4 *out, void *state) {
    CamBlendState *st = state;
    u8 *cam = (u8 *)out;
    Vec4 oriNow;
    u8 mtx[0x40];
    f32 t;

    if (st->posProgress == 1.0f && st->oriProgress == 1.0f) {
        return 1;
    }
    t = func_002A8A68(0.0f, 1.0f, st->posProgress);
    Vec4AddVu0(&st->posFrom, &g_heroPos_D0, &st->posFrom);
    st->posOut.x = st->posFrom.x + (*(f32 *)(cam + 0x30) - st->posFrom.x) * t;
    st->posOut.y = st->posFrom.y + (*(f32 *)(cam + 0x34) - st->posFrom.y) * t;
    st->posOut.z = st->posFrom.z + (*(f32 *)(cam + 0x38) - st->posFrom.z) * t;
    g_cameraPos = st->posOut;
    func_002AC468(&oriNow, (Camera *)cam);
    t = func_002A8A68(0.0f, 1.0f, st->oriProgress);
    func_002841C0(&st->oriOut, &st->oriFrom, &oriNow, t);
    func_00284380(&st->oriOut, mtx);
    func_00284028((u8 *)&g_cameraPos + 0x230, mtx);
    st->posProgress += st->posStep;
    if (st->posProgress > 1.0f) {
        st->posProgress = 1.0f;
    }
    st->oriProgress += st->oriStep;
    if (st->oriProgress > 1.0f) {
        st->oriProgress = 1.0f;
    }
    return 0;
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00271140);
#endif

/* func_002712E8: running-blend interpolation branch of the transition pipeline
 * (forwarded from ApplyCameraTransition while a blend is in flight).
 * DECODE IN PROGRESS (fable 2026-07-02, ~85%, body deferred one session
 * rather than risk a mis-routed operand in the uncertain band — the same
 * risk class as the audited B68 sign bug):
 *  - st = ts+0x70 view (sph* / blendTarget / rate / count; +0x20/+0x30 =
 *    basisFwd/basisUp, +0x40 oriB, +0x50/+0x60 = src0/src1).
 *  - early-out: blendTarget <= 0 -> return 1. Ease factor f24 =
 *    1 / func_002A8A68(1.0, (f32)count, (f32)target * rate).
 *  - kind==2: anchor = g_heroPos, basis from st(+0x20/+0x30), right = cross,
 *    live sph via func_00270B68(cam->pos vs anchor). kind!=2: anchor =
 *    out+0x30, basis normalized from cineKeys+0xC0/+0xE0, sph = {PI,0,0}.
 *  - eases sphYaw/sphPitch by WrapAnglePiSum(old, WrapAnglePiDiff(new,old)
 *    * f24), dist linearly; rebuilds pos = anchor + rotate(rotate(fwd*dist,
 *    up, yaw), right, pitch); pos -> st+0x50 AND g_cameraPos.
 *  - orientation band (UNCERTAIN OPERANDS): oriB -> matrix (func_00284308
 *    at sp+0x80), roll delta = (PI/2 - acos-form) signed by row1 dot,
 *    >PI/2 wrap-correction with 2PI(0x40C90FDC) adjust, <1e-5(0x3727C5AC)
 *    fast-path copies rows, else func_002AC4D0 quat + func_002ADC50
 *    applications; final rows renormalized (row1 = cross with
 *    g_heroFacingDir, len -1.0!), func_002AC468 captures into src1 AND oriB,
 *    func_002832F8 ticks blendTarget; return 0. */
/* WALL:
 * fourteen callee-saves at 8-byte slot spacing (packed-save wall); 0x410 bytes
 * of interleaved fp/qword math. Left INCLUDE_ASM. */
#ifdef TARGET_NATIVE
extern void Vec3CrossVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern void Vec4ScaleVu0(Vec4 *dst, f32 s, const Vec4 *src);
extern void func_002AC4D0(void *dstQuat, const Vec4 *axis, f32 ang);
extern void func_002ADC50(void *dst, const Vec4 *v, const void *quat);
extern void func_00284308(void *rot, void *mtx); /* rotation -> 3-row matrix */
extern void func_002AC468(void *dstOri, Camera *cam);
extern s32 func_002832F8(void *timer);
extern f32 IntToFloat(s32 x);
extern f32 GetFloatAbs(f32 x);
extern f32 WrapAnglePiDiff(f32 a, f32 b);
extern f32 WrapAnglePiSum(f32 a, f32 b);
extern f32 func_002A8A68(f32 target, f32 from, f32 s);
extern Vec4 g_heroPos;
extern Vec4 g_heroFacingDir;
extern Vec4 g_cameraMatrix;   /* 3-row rotation matrix at 0x1B54F0 */
extern Vec4 g_cameraPos;      /* 0x1B52C0 */
/* one-ULP-precise constants the original uses (bit-encoded: the yaw-band
 * threshold and the 2PI adjust are one ULP ABOVE the rounded value, so float
 * literals would mis-round them) */
static const union { u32 u; f32 f; } kPiO2      = { 0x3FC90FDB };
static const union { u32 u; f32 f; } kPiO2Ulp   = { 0x3FC90FDC };
static const union { u32 u; f32 f; } kTwoPiUlp  = { 0x40C90FDC };
static const union { u32 u; f32 f; } kPi        = { 0x40490FDB };
/* View of CameraTransitionState+0x70 (the running-blend state). */
typedef struct SphBlendState {
    /* 0x00 */ f32 yaw;
    /* 0x04 */ f32 pitch;
    /* 0x08 */ f32 dist;
    /* 0x0C */ s32 target;   /* countdown; <=0 = blend complete */
    /* 0x10 */ f32 rate;
    /* 0x14 */ s32 count;
    /* 0x18 */ u8 pad18[8];
    /* 0x20 */ Vec4 basisFwd;
    /* 0x30 */ Vec4 basisUp;
    /* 0x40 */ Vec4 oriB;
    /* 0x50 */ Vec4 posOut;
    /* 0x60 */ Vec4 oriOut;
} SphBlendState;
s32 func_002712E8(Vec4 *out, void *state) {
    SphBlendState *st = state;
    Vec4 sph;    /* eased-toward spherical targets {yaw,pitch,dist} */
    Vec4 fwd, right, up, anchor, arm, flat, proj;
    Vec4 mtxRows[3];
    Vec4 b0, b1, probe, quat2;
    f32 ease, yawDelta, roll, rollStep, elev;

    if (st->target <= 0) {
        return 1;
    }
    ease = 1.0f / func_002A8A68(1.0f, IntToFloat(st->count),
                                IntToFloat(st->target) * st->rate);
    if (g_cameraTransitionState.kind == 2) {
        anchor = g_heroPos;
        fwd = st->basisFwd;
        up = st->basisUp;
        Vec3CrossVu0(&right, &fwd, &up);
        func_00270B68(&sph, (u8 *)out + 0x30, &anchor, &fwd, &right, &up);
    } else {
        char *keys = *(char **)(g_soundBankHandlesBlk + 0x2290);
        anchor = *(Vec4 *)((u8 *)out + 0x30);
        Vec3RescaleToLenVu0(&fwd, 1.0f, (Vec4 *)(keys + 0xC0));
        Vec3RescaleToLenVu0(&up, 1.0f, (Vec4 *)(keys + 0xE0));
        sph.x = kPi.f;
        sph.y = 0.0f;
        sph.z = 0.0f;
    }
    /* ease the spherical coords toward the targets (angles wrap-aware) */
    yawDelta = WrapAnglePiDiff(sph.x, st->yaw);
    st->yaw = WrapAnglePiSum(st->yaw, yawDelta * ease);
    st->pitch = WrapAnglePiSum(st->pitch, WrapAnglePiDiff(sph.y, st->pitch) * ease);
    st->dist += (sph.z - st->dist) * ease;
    /* rebuild the camera position: fwd scaled to dist, yawed around up,
     * pitched around the derived right axis, off the anchor */
    Vec3RescaleToLenVu0(&arm, st->dist, &fwd);
    func_002ADD28(&arm, &arm, &up, st->yaw);
    Vec3CrossVu0(&right, &arm, &up);
    Vec3RescaleToLenVu0(&right, 1.0f, &right);
    func_002ADD28(&arm, &arm, &right, st->pitch);
    Vec4AddVu0(&st->posOut, &anchor, &arm);
    g_cameraPos = st->posOut;
    /* orientation: decompose the target basis (out rows) against the current
     * blend orientation (oriB as a 3-row matrix) */
    func_00284308(&st->oriB, mtxRows);
    Vec4ScaleVu0(&proj, Vec3DotVu0(&mtxRows[2], out), &mtxRows[2]);
    Vec4SubVu0(&flat, out, &proj);
    roll = kPiO2.f - func_00283B60(Vec3DotVu0(&mtxRows[0], &flat) /
                                   Vec3LengthVu0(&flat));
    /* SIGN IDIOM: bc1tl + annulled delay -> keep positive when dot >= 0 */
    if (!(Vec3DotVu0(&flat, &mtxRows[1]) >= 0.0f)) {
        roll = -roll;
    }
    /* yaw crossed more than a quarter turn this step: the roll decomposition
     * flips branch - wrap it by one ULP-above-2PI when the yaw delta and the
     * roll sign disagree */
    if (GetFloatAbs(yawDelta) > kPiO2Ulp.f) {
        f32 sign = (Vec3DotVu0(&flat, &mtxRows[1]) >= 0.0f) ? 1.0f : -1.0f;
        if ((0.0f <= yawDelta && sign < 0.0f) ||
            (yawDelta < 0.0f && 0.0f <= sign)) {
            if (roll < 0.0f) {
                roll += kTwoPiUlp.f;
            } else {
                roll -= kTwoPiUlp.f;
            }
        }
    }
    rollStep = roll * ease;
    /* roll-step the current basis rows */
    if (GetFloatAbs(rollStep) < 1e-5f) {
        b0 = mtxRows[0];
        b1 = mtxRows[1];
    } else {
        func_002AC4D0(&probe, &mtxRows[2], rollStep);
        func_002ADC50(&b0, &mtxRows[0], &probe);
        func_002ADC50(&b1, &mtxRows[1], &probe);
    }
    /* full-roll probe row (NOTE: the small-roll arm copies ROW 0, not row 2) */
    if (GetFloatAbs(roll) < 1e-5f) {
        probe = mtxRows[0];
    } else {
        func_002AC4D0(&quat2, &mtxRows[2], roll);
        func_002ADC50(&probe, &mtxRows[0], &quat2);
    }
    /* elevation step toward the target row0, signed by the target row2 */
    elev = kPiO2.f - func_00283B60(Vec3DotVu0(&probe, out));
    /* SIGN IDIOM: bc1fl + annulled delay -> negate when the dot is < 0 */
    if (!(Vec3DotVu0(&probe, (Vec4 *)((u8 *)out + 0x20)) >= 0.0f)) {
        elev = -elev;
    }
    func_002ADD28(&b0, &b0, &b1, elev * ease);
    /* write the camera matrix: row0 = |b0|, row1 = -|row0 x heroFacing|,
     * row2 = row1 x row0; then capture the new orientation into BOTH
     * oriOut (src1) and oriB, and tick the countdown */
    Vec3RescaleToLenVu0(&g_cameraMatrix, 1.0f, &b0);
    Vec3CrossVu0(&(&g_cameraMatrix)[1], &g_cameraMatrix, &g_heroFacingDir);
    Vec3RescaleToLenVu0(&(&g_cameraMatrix)[1], -1.0f, &(&g_cameraMatrix)[1]);
    Vec3CrossVu0(&(&g_cameraMatrix)[2], &(&g_cameraMatrix)[1], &g_cameraMatrix);
    func_002AC468(&st->oriOut, (Camera *)&g_cameraMatrix);
    func_002AC468(&st->oriB, (Camera *)&g_cameraMatrix);
    func_002832F8(&st->target);
    return 0;
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002712E8);
#endif

/* ApplyCameraTransition: advance the active camera blend into `out`. With no
 * transition pending (kind == 0) it forwards to func_00271140 (start/instant
 * settle); while one is running it forwards to func_002712E8 (interpolate). On
 * the running path, once the blend reports complete it copies the resulting
 * 4-row matrix into g_cameraMatrix and clears the transition (kind = 0, +0x0
 * halfword = 0). */
#ifdef TARGET_NATIVE
extern s32 func_00271140(Vec4 *out, void *state);
extern s32 func_002712E8(Vec4 *out, void *state);
extern Vec4 g_cameraMatrix;   /* 3-row rotation matrix at 0x1B54F0 */
extern Vec4 g_cameraPos;      /* 0x1B52C0 = g_cameraMatrix - 0x230 */
void ApplyCameraTransition(Vec4 *out) {
    s32 done;

    /* The matrix copy gates on the INTERPOLATOR'S RETURN value (not kind),
     * and the 4th row goes to g_cameraPos, NOT g_cameraMatrix[3]. */
    if (g_cameraTransitionState.kind == 0) {
        done = func_00271140(out, (u8 *)&g_cameraTransitionState + 0x10);
    } else {
        done = func_002712E8(out, (u8 *)&g_cameraTransitionState + 0x70);
    }
    if (done != 0) {
        (&g_cameraMatrix)[0] = out[0];
        (&g_cameraMatrix)[1] = out[1];
        (&g_cameraMatrix)[2] = out[2];
        g_cameraPos = out[3];
        g_cameraTransitionState.kind = 0;
        g_cameraTransitionState.state = 0;
    }
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", ApplyCameraTransition);
#endif

/* One camera-shake channel (three instances driven per frame). +0x0 base
 * amplitude, +0x4 this frame's applied value, +0x8 countdown timer,
 * +0xC peak (timer high-water mark used to normalise the decay). */
typedef struct CamShakeChannel {
    /* 0x00 */ f32 amplitude;
    /* 0x04 */ f32 current;
    /* 0x08 */ s32 timer;
    /* 0x0C */ s32 peak;
} CamShakeChannel;

/* ApplyCameraShakeAxis: apply one shake channel to the camera. Modes 0/1
 * offset g_cameraPos along a g_cameraMatrix row (0 = row 2, 1 = row 0) by
 * amplitude * cos(wrap(2*timer)) * t^2 where t = timer/peak; mode 2 rolls
 * g_cameraMatrix by amplitude * sin(wrap(elapsed/60*step - step*10)) * t^3 *
 * min(1, elapsed/10) (step = D_1A85B8 * pi/180). Camera type 6 clears the
 * channel; timer==0 clears the peak. */
#ifdef TARGET_NATIVE
extern Camera *g_activeCamera;      /* aliases g_cameraState+0x190 */
extern f32 D_1A85B8;                /* shake roll speed (degrees/tick) */
extern s32 func_002832F8(void *timer);
extern f32 func_002845D8(f32 x);    /* wrap angle */
extern void func_002ADCE0(void *dst, f32 ang);   /* build roll rotation */
extern void func_00284308(void *rot, void *mtx); /* rotation -> 3-row matrix */
extern void func_002840E8(void *dst, void *a, void *b); /* mtx multiply */
void ApplyCameraShakeAxis(void *channel, s32 axis) {
    CamShakeChannel *sh = channel;
    Camera *cam = g_activeCamera;
    Vec4 ofs;
    u8 mtx[0x30];
    f32 t;

    if (cam != 0 && cam->type == 6) {
        sh->peak = 0;
        sh->timer = 0;
        return;
    }
    if (sh->timer == 0) {
        sh->peak = 0;
        return;
    }
    if (sh->peak < sh->timer) {
        sh->peak = sh->timer;
    }
    func_002832F8(&sh->timer);
    t = IntToFloat(sh->timer) / IntToFloat(sh->peak);
    switch (axis) {
    case 0:
    case 1: {
        f32 c = func_00283B30(func_002845D8(IntToFloat(sh->timer) * 2.0f));
        Vec4 *row = (axis == 0) ? &(&g_cameraMatrix)[2] : &g_cameraMatrix;
        f32 amp = sh->amplitude * c * t * t;

        sh->current = amp;
        Vec3RescaleToLenVu0(&ofs, amp, row);
        Vec4AddVu0(&g_cameraPos, &g_cameraPos, &ofs);
        break;
    }
    case 2: {
        f32 t3 = t * t * t;
        f32 step = D_1A85B8 * 0.017453292f;              /* deg -> rad */
        f32 e60 = IntToFloat(sh->peak - sh->timer) * 0.016666668f;
        f32 ramp = e60 * 6.0f;
        f32 ang;

        if (ramp > 1.0f) {
            ramp = 1.0f;
        }
        ang = sh->amplitude *
              func_00283B48(func_002845D8(e60 * step - step * 10.0f)) *
              t3 * ramp;
        sh->current = ang;
        func_002ADCE0(&ofs, ang);
        func_00284308(&ofs, mtx);
        func_002840E8(&g_cameraMatrix, mtx, &g_cameraMatrix);
        break;
    }
    default:
        break;
    }
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", ApplyCameraShakeAxis);
#endif

/* Per-frame hero-motion tracking block, a sub-region of g_cameraState at
 * +0x1A0 (vaddr 0x1B5320; the original addresses it as g_prevCamera+0xC). The
 * camera mode handlers read these smoothed/derived hero signals. */
typedef struct HeroCamMotion {
    /* 0x00 */ f32 smoothX;         /* hero x (snapped to g_heroPos.x) */
    /* 0x04 */ f32 smoothY;         /* hero y (snapped to g_heroPos.y) */
    /* 0x08 */ f32 smoothHeight;    /* spring-smoothed hero z (height) */
    /* 0x0C */ f32 rawZ;            /* last frame's hero z */
    /* 0x10 */ f32 heightVel;       /* height-spring velocity state */
    /* 0x14 */ u8  pad14[0x0C];
    /* 0x20 */ Vec4 facingDir;      /* smoothed camera-facing unit dir */
    /* 0x30 */ Vec4 facingRingCur;  /* newest negated-facing sample */
    /* 0x40 */ Vec4 facingRingPrev; /* previous negated-facing sample */
    /* 0x50 */ Vec4 facingVel;      /* facing-spring velocity (per xyz) */
    /* 0x60 */ Vec4 prevPos;        /* hero position last frame */
    /* 0x70 */ Vec4 velocity;       /* hero velocity this frame */
    /* 0x80 */ Vec4 lateralDir;     /* normalized sideways direction */
    /* 0x90 */ Vec4 forwardProj;    /* velocity projected onto facing */
    /* 0xA0 */ f32 speed;           /* |velocity| */
    /* 0xA4 */ f32 lateralSpeed;    /* |lateralDir| before normalizing */
    /* 0xA8 */ f32 forwardSpeed;    /* velocity . facing */
    /* 0xAC */ f32 orientZRing[5];  /* ring of g_heroOrientVec[2] history */
} HeroCamMotion;

/* TrackHeroMotionForCamera: smooth the hero facing / velocity / speed / lateral
 * direction from g_heroPos deltas for the camera logic to consume. WALL: twelve
 * callee-saves at 8-byte slot spacing (packed-save wall) + heavy interleaved fp
 * math. Left INCLUDE_ASM for the matching build. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", TrackHeroMotionForCamera);
#else
extern f32 Vec3DotVu0(const Vec4 *a, const Vec4 *b);
extern f32 Vec3LengthVu0(const Vec4 *v);
extern void Vec4SubVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern void Vec4ScaleVu0(Vec4 *dst, f32 s, const Vec4 *src);
extern void Vec3RescaleToLenVu0(Vec4 *dst, f32 len, const Vec4 *src); /* (dst,len,src) */
extern Vec4 g_heroPos;          /* 0x189EA0 hero world position */
extern Vec4 g_heroFacingDir;    /* 0x18A0E0 hero forward unit dir (negated here) */
extern f32 g_heroOrientVec[4];  /* 0x189EB0 hero orientation quad */
extern s32 g_heroState;         /* 0x18C0B4 hero action-state code */
extern s32 g_cameraZoneType;    /* 0x18C2C0 current camera zone/type id */
/* TODO(match): functional equivalent - not byte-exact; twelve callee-saves at
   8-byte slot spacing (packed-save wall) + the interleaved fp pipeline. Float
   constants are exact: 0.015f=0x3c75c28f, 0.2f=0x3e4ccccd, -0.98f=0xbf7ae148,
   0.0075f=0x3bf5c28f, 0.175f=0x3e333333. */
void TrackHeroMotionForCamera(void) {
    HeroCamMotion *m = (HeroCamMotion *)((char *)&g_cameraState + 0x1A0);
    Vec4 facing;    /* newest camera-facing sample = -normalize(hero facing) */
    Vec4 fwdProj;   /* facing scaled by the forward speed */
    f32 dot;

    Vec3RescaleToLenVu0(&facing, -1.0f, &g_heroFacingDir);

    /* shift the 2-slot facing history, push the new sample. NB: the ring stores
       the PRE-nudge sample (the antipode +0.2 fix-up below only feeds the spring,
       not the history) - faithful to the asm, which stores +0x30 before the nudge. */
    m->facingRingPrev = m->facingRingCur;
    m->facingRingCur = facing;

    /* if the new sample nearly reverses the smoothed facing, nudge it so the
       spring does not lock up at the antipode */
    dot = Vec3DotVu0(&m->facingDir, &facing);
    if (dot < -0.98f) {
        facing.x += 0.2f;
        facing.y += 0.2f;
        facing.z += 0.2f;
    }

    /* critically-damped spring of each facing axis toward the new sample */
    m->facingDir.x = func_002702D8(m->facingDir.x, facing.x, 0.015f, 0.2f, 0.0f, &m->facingVel.x);
    m->facingDir.y = func_002702D8(m->facingDir.y, facing.y, 0.015f, 0.2f, 0.0f, &m->facingVel.y);
    m->facingDir.z = func_002702D8(m->facingDir.z, facing.z, 0.015f, 0.2f, 0.0f, &m->facingVel.z);
    Vec3RescaleToLenVu0(&m->facingDir, 1.0f, &m->facingDir);

    /* hero velocity + speed this frame (prevPos still holds last frame's pos) */
    Vec4SubVu0(&m->velocity, &g_heroPos, &m->prevPos);
    m->speed = Vec3LengthVu0(&m->velocity);

    /* split the velocity into forward (along facing) and lateral components */
    m->forwardSpeed = Vec3DotVu0(&m->velocity, &facing);
    Vec3RescaleToLenVu0(&fwdProj, m->forwardSpeed, &facing);
    m->forwardProj = fwdProj;
    Vec4SubVu0(&m->lateralDir, &m->velocity, &fwdProj);
    m->lateralSpeed = Vec3LengthVu0(&m->lateralDir);
    Vec4ScaleVu0(&m->lateralDir, 1.0f / m->lateralSpeed, &m->lateralDir);

    /* remember this frame's hero position for next frame's delta */
    m->prevPos = g_heroPos;

    if (g_cameraZoneType == 0x50 && g_heroState != 0x11) {
        m->smoothY = g_heroPos.y;
        m->smoothX = g_heroPos.x;
    } else {
        m->smoothX = g_heroPos.x;
        m->smoothY = g_heroPos.y;
        m->smoothHeight = func_002702D8(m->smoothHeight, g_heroPos.z, 0.0075f, 0.175f,
                                        0.0f, &m->heightVel);
        m->rawZ = g_heroPos.z;
    }

    /* push the hero orient-Z history ring (shift down by one, append newest) */
    {
        s32 i;
        for (i = 0; i < 4; i++) {
            m->orientZRing[i] = m->orientZRing[i + 1];
        }
        m->orientZRing[i] = g_heroOrientVec[2];
    }
}
#endif

/* CheckCameraUnderwater: walk a short downward collision ray (up to 6 CollLine
 * steps) from the camera position to decide whether the camera is submerged,
 * comparing against GetWaterSurfaceHeight; records the result into the camera
 * state (+0x400). Skipped for camera type 6 and outside g_nGameState==0.
 * WALL (re-graded on the engine pipeline, fable 2026-07-02): (a) the
 * volatile activeCamera deref emits lhu+sll+sra vs the original lh (castable),
 * (b) 2.96 splits the Vec4 copies into ld/sd pairs vs the original lq/sq,
 * (c) decisive: the original fills the beq delay slot with a 1-insn %gp_rel
 * g_nGameState load (extern class 9..15), which conflicts with this unit's
 * established incomplete-array/absolute model for that symbol (changing it
 * ripples through every matched fn's assembly). Portable #else below;
 * cmp-oracle queued (mocked CollLine/GetCollHitMaterial/water-height). */
#if defined(MATCH_CheckCameraUnderwater) || defined(TARGET_NATIVE)
extern s32 CollLine(Vec4 *from, Vec4 *to, s32 mask, void *a, void *b);
extern s32 GetCollHitMaterial(void);
extern f32 GetWaterSurfaceHeight(Vec4 *at, s32 mode);
extern Vec4 g_collHitPoint;
void CheckCameraUnderwater(void) {
    Vec4 top;    /* probe segment start (camera z + 0.75) */
    Vec4 bottom; /* probe segment end (camera z - 0.75) */
    s32 i;

    if (g_cameraState.activeCamera->type == 6 || g_nGameState[0] != 0) {
        g_cameraState.underwater = 0;
        return;
    }
    top = g_cameraState.camPos;
    bottom = g_cameraState.camPos;
    i = 0;
    top.z += 0.75f;
    bottom.z -= 0.75f;
    while (i < 6) {
        if (CollLine(&top, &bottom, 0x12, 0, 0) == 0) {
            break; /* nothing below: leave the flag as-is */
        }
        if (GetCollHitMaterial() == 0) { /* water surface */
            if (g_cameraState.camPos.z < GetWaterSurfaceHeight(&g_collHitPoint, 0) + 0.04f) {
                g_cameraState.underwater = 1;
            } else {
                g_cameraState.underwater = 0;
            }
            return;
        }
        /* solid: restart the probe just below the hit point */
        top = g_collHitPoint;
        top.z -= 0.01f;
        i++;
    }
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", CheckCameraUnderwater);
#endif

/* func_00271FE8: per-frame camera-id / cinematic-state arbiter. Reads the active
 * cinematic key block (g_soundBankHandlesBlk) and player progress to pick the
 * live camera mode id (g_cameraCallbackCount +0x10) and a derived sub-id
 * (+0xC), gated by a camera-Z distance threshold and a hardcoded hero-position
 * rectangle override (only at story progress 6), then dispatches a queued
 * camera id from whichever trigger flag byte is set.
 *
 * The matching build stays INCLUDE_ASM: BYTE-MATCH wall only (gp/absolute mix -
 * g_cameraCallbackCount+0x10 is reached both %gp_rel and %hi/%lo in the one
 * function, which no single C small-data classification reproduces). That is
 * NOT a portability blocker, so the TARGET_NATIVE #else below is a faithful
 * op-for-op coverage body. */
/* g_soundBankHandlesBlk (+0x80 = g_heroPos), g_cameraCallbackCount and
 * g_cameraPos are already declared above; only these two are new here: */
extern s32  g_playerProgress;        /* 0x1A79F8 persistent save block; word 0 = story progress */
extern s32  g_cameraTriggerLatch;    /* 0x1B53E0 queued camera-trigger id latch (UNCONFIRMED) */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00271FE8);
#else
void func_00271FE8(void) {
    char *sbh   = g_soundBankHandlesBlk;
    s32 *pMode  = (s32 *)((u8 *)&g_cameraCallbackCount + 0x10);
    s32 *pSub   = (s32 *)((u8 *)&g_cameraCallbackCount + 0x14);
    s32 *pOut   = (s32 *)((u8 *)&g_cameraCallbackCount + 0xC);
    s32 mode;

    /* Base mode from the cinematic-key control words. */
    *pMode = 0x14;
    if ((u32)(*(s32 *)(sbh + 0x229C) - 0x11) < 2 ||
        *(s32 *)(sbh + 0x2294) == 0x70) {
        *pMode = 0x34;
    }
    if (*(s32 *)(sbh + 0x229C) != 0x11) {
        if (*(f32 *)(sbh + 0x330) < g_cameraPos.z) {
            *pMode = 0x14;
        }
    }
    /* Hardcoded map-location override: only when the hero stands in a specific
     * x/y rectangle at story progress 6. */
    if (g_playerProgress == 6) {
        f32 hx = *(f32 *)(sbh + 0x80);   /* g_heroPos.x */
        if (485.7f < hx && hx < 570.7f) {
            f32 hy = *(f32 *)(sbh + 0x84);   /* g_heroPos.y */
            if (250.0f < hy && hy < 334.5f) {
                *pMode = 0x34;
            }
        }
    }

    mode = *pMode;
    *pSub  = mode;
    *pMode = mode | 0x80;

    /* Dispatch the queued camera id from the active trigger flag byte. */
    if (sbh[0x1495]) {
        g_cameraTriggerLatch = 0x100;
        *pOut = 0x1B4;
    } else if (sbh[0x149B]) {
        g_cameraTriggerLatch = 0xB00;
        *pOut = 0xBB4;
    } else if (sbh[0x1496]) {
        g_cameraTriggerLatch = 0x300;
        *pOut = 0x3B4;
    } else if (sbh[0x149C]) {
        g_cameraTriggerLatch = 0xD00;
        *pOut = 0xDB4;
    } else if (sbh[0x1494]) {
        g_cameraTriggerLatch = 0;
        *pOut = 0xB4;
    } else {
        *pOut = g_cameraTriggerLatch | 0xB4;
    }
}
#endif

/* func_002721A8: start a fade-from-black — fade level forced to 1.0, target 0,
 * rate = 1/duration (g_cameraState +0x268/+0x26C).
 * ENGINE-2.96 MATCH (candidate, fable 2026-07-02): BYTE+RELOC IDENTICAL via
 * the engine pipeline — 2.96 cc1 + split-addresses patch
 * (tools/ee/patch_cc296_splitaddr.py), -O2 -G8 -fno-builtin
 * -fno-strict-aliasing, scheduling ON, mtc1_fixup (the two "SN-as div.s
 * latency" nops are the mtc1->div.s hazard pads the fixup restores — the old
 * wall note was a misdiagnosis). Guard: MATCH_func_002721A8 promotes the real
 * C for the per-function engine build; the regular 2.9 unit build keeps
 * INCLUDE_ASM (two-compiler build). Pending tester's authoritative verify. */
#if !defined(TARGET_NATIVE) && !defined(MATCH_func_002721A8)
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002721A8);
#else
void func_002721A8(f32 duration) {
    g_screenFadeBlack = 1.0f;
    g_cameraState.fadeBlackTarget = 0.0f;
    g_cameraState.fadeBlackRate = 1.0f / duration;
}
#endif

/* UpdateScreenFadeBlack: per-frame driver that eases g_screenFadeBlack toward
 * fadeBlackTarget at fadeBlackRate (no-op while the rate is 0). When the level
 * reaches the target a follow-up timer (func_002832F8 on the +0x270 field) may
 * re-arm the target to 0; once both target and current level have settled at
 * (or below) 0 the rate is cleared so the driver idles. */
#if !defined(TARGET_NATIVE) && !defined(MATCH_UpdateScreenFadeBlack)
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", UpdateScreenFadeBlack);
#else
/* Canonical signature per the callee def (1A8180 line ~1255): f32 return,
 * (target/$f12, rate/$f13, level-ptr/$4) — the rate arg IS read (step clamp).
 * The old 2-arg decl here DROPPED it (caught 2026-07-02, engine lane);
 * cmp-oracle is STALE for this body until re-run with the 3-arg call. */
extern f32 func_002AB150(f32 target, f32 rate, f32 *level);
extern s32 func_002832F8(void *timer);
/* TODO(match): functional equivalent - not byte-exact. Engine-2.96 pipeline
   (fable 2026-07-02) reproduces the whole body shape (direct g_cameraState
   accesses give the %hi-in-s1 + per-arm lo_sum re-derivation), but walls on
   the FP-SAVE-SLOT ABI: the original allocates $f20 an 8-byte save slot at a
   16-aligned offset (f20@+0x20, frame 0x30 = FP_INC=2 with swc1); 001003
   packs a 4-byte slot (f20@+0x18, frame 0x20) and no flag reaches the
   original model (-mfp64 inert, -mdouble-float emits s.d). Wall class gates
   every $f2x-saving fn in this TU. */
void UpdateScreenFadeBlack(void) {
    if (g_cameraState.fadeBlackRate == 0.0f) {
        return;
    }
    func_002AB150(g_cameraState.fadeBlackTarget, g_cameraState.fadeBlackRate,
                  &g_screenFadeBlack);
    if (g_screenFadeBlack == g_cameraState.fadeBlackTarget) {
        if (func_002832F8((char *)&g_cameraState + 0x270) == 1) {
            g_cameraState.fadeBlackTarget = 0.0f;
        }
    }
    if (g_cameraState.fadeBlackTarget <= 0.0f && g_screenFadeBlack <= 0.0f) {
        g_cameraState.fadeBlackRate = 0.0f;
    }
}
#endif

/* UpdateScreenFadeWhite: white-flash twin of UpdateScreenFadeBlack. Eases
 * g_screenFadeWhite toward fadeWhiteTarget (+0x27C) at fadeWhiteRate (+0x274);
 * idles while the rate is 0. Eases toward the target via func_002AB150, then
 * the settle/re-arm (func_002832F8 on the +0x280 timer) and rate-clear mirror
 * the black-fade path. */
/* Engine-2.96 candidate (fable 2026-07-02): the target==0 branch passes a
 * THIRD-arg rate slot from +0x278 (the fade-out step clamp) — settled as a
 * genuine 3-arg prototyped call (callee def in 1A8180 reads $f13; K&R mixed
 * arity is impossible: this cc1 promotes unprototyped floats via fptodp).
 * The g_screenFadeWhite loads are
 * the size-class-9..15 mix (absolute straight-line, %gp_rel in delay slots),
 * so this must be assembled through asm_unit.sh's -G8 SN-parity path with
 * `.extern g_screenFadeWhite, 12`. */
#if defined(MATCH_UpdateScreenFadeWhite) || defined(TARGET_NATIVE)
#ifndef TARGET_NATIVE
__asm__(".extern g_screenFadeWhite, 12");
#endif
/* Canonical signature per the callee def (1A8180): the rate/$f13 arg is a
 * real step clamp, CSE-satisfied at the call sites by the preceding compare
 * loads (which is why the asm LOOKS 2-arg). */
extern f32 func_002AB150(f32 target, f32 rate, f32 *level);
extern s32 func_002832F8();
void UpdateScreenFadeWhite(void) {
    if (g_cameraState.fadeWhiteRate == 0.0f) {
        return;
    }
    if (g_cameraState.fadeWhiteTarget == 0.0f) {
        func_002AB150(g_cameraState.fadeWhiteTarget,
                      g_cameraState.fadeWhiteOutRate, &g_screenFadeWhite);
    } else {
        func_002AB150(g_cameraState.fadeWhiteTarget,
                      g_cameraState.fadeWhiteRate, &g_screenFadeWhite);
    }
    if (g_screenFadeWhite == g_cameraState.fadeWhiteTarget) {
        if (func_002832F8(g_cameraState.fadeWhiteTimer) == 1) {
            g_cameraState.fadeWhiteTarget = 0.0f;
        }
    }
    if (g_cameraState.fadeWhiteTarget <= 0.0f && g_screenFadeWhite <= 0.0f) {
        g_cameraState.fadeWhiteRate = 0.0f;
    }
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", UpdateScreenFadeWhite);
#endif

/* UpdateCamera: top-level per-frame camera tick. Advances the fade overlays and
 * frame counter, runs the hero-motion tracker and the mode-arbitration
 * dispatcher, then resolves the active transition: kicks a fresh blend when the
 * transition state just became 1/2, interpolates while it is 3, otherwise snaps
 * the live camera fields from the active slot. A forced-override hook
 * (func_0026F7C0) can rebuild the camera basis directly from a cinematic block.
 * Finally it derives g_cameraRot from g_cameraMatrix, applies the three shake
 * channels, runs the underwater + fog-zone sampling and the FOV interpolation. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", UpdateCamera);
#else
extern void TrackHeroMotionForCamera(void);
extern void func_00271FE8(void);
extern void BeginCameraTransition(Camera *active, Camera *prev);
extern s32 func_0026F7C0(void);
extern void ApplyCameraTransition(Vec4 *out);
extern void MatrixToEulerAngles(const Vec4 *m, Vec4 *outRot);
extern void ApplyCameraShakeAxis(void *channel, s32 axis);
extern void CheckCameraUnderwater(void);
extern void SampleCameraFogZone(Vec4 *pos);
extern void Vec3CrossVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern void Vec3RescaleToLenVu0(Vec4 *dst, f32 len, const Vec4 *src); /* (dst,len,src) - see func_00270D60 note */
extern void StepCameraFovInterp(void);
extern Vec4 g_cameraPos;
extern Vec4 g_cameraRot;
extern Vec4 g_cameraMatrix;   /* 3-row rotation matrix at 0x1B54F0 */
extern Vec4 g_cameraShakeUp;
extern Vec4 g_cameraShakeRight;
extern Vec4 g_cameraShakeRoll;
extern Vec4 g_cinematicCameraBlock[]; /* g_nNanotechBonusHealTimer + 0x12F4 */
extern u8 D_1A7A4C;
/* TODO(match): functional equivalent - not byte-exact; four callee-saves
   (incl. $f20) at 8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void UpdateCamera(void) {
    /* IN-LEVEL native frame-path: UpdateCamera is an out-of-scope driven-frame
     * TRAP (tester state/inlevel_trap_list.md). It dereferences cs->activeCamera
     * (g_cameraState+0x190) - the OVERLAY-resident pointer (the unmapped
     * 0x089C1000) in-level, see tools/native/camera_map_re.md - and reads other
     * relocated-band camera globals, faulting on real EE. No-op the tick: the
     * camera transform is externally supplied (seeded from the in-level dump),
     * and TrackHeroMotionForCamera is driven + validated separately. The full
     * frontend reconstruction is preserved in git (commit 1fd00c0); restore and
     * guard it if the in-level seed is ever un-relocated. */
}
#endif

/* func_00272550: MIS-SPLIT fragment. The .s opens with four words
 * (daddu $2,$5,$0 / nop / addiu $sp,0x10 / nop) that are the TAIL of the
 * preceding function (its return path), followed by `alabel func_00272560` —
 * the real entry. A clean C #else body would only emit the real function and
 * drop the leading tail words, shifting the segment layout. Kept permanently
 * INCLUDE_ASM so the orphaned tail words stay in place. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00272550);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002725D8);

/* DrawLensFlare: project the sun/light source to screen space and draw the
 * lens-flare sprite chain. WALL: seventeen callee-saves at 8-byte slot spacing
 * (packed-save wall) + interleaved projection/draw fp math; 0x10C bytes. Left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", DrawLensFlare);

/* DrawScreenSpriteFxEntry(x, y, fx): render one queued screen-sprite effect at
 * screen position (x,y), 40px-per-unit scale. mode 0 = radial burst (repeat
 * fx->drawFlags-count times stepping the angle by fx->angle step); mode 1 = four
 * mirrored quads offset along the angle; mode 2 = one centered sprite. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", DrawScreenSpriteFxEntry);
#else
extern u64 GetUiTextureTex0(s32 texId);
extern f32 CosfVu0(f32 a);
extern f32 SinfVu0(f32 a);
extern void DrawRotatedSprite2d(f32 x, f32 y, f32 w, f32 h, f32 angle,
                                f32 u0, f32 v0, s32 u1, s32 v1, u64 tex0,
                                u32 color, u32 rgba, s32 mirrorX, s32 mirrorY);
/* TODO(match): functional equivalent - not byte-exact; nine callee-saves +
   the fp-pipeline scheduling of the repeated Cos/Sin reloads (packed-save wall). */
void DrawScreenSpriteFxEntry(f32 x, f32 y, ScreenSpriteFx *fx) {
    u64 tex0 = GetUiTextureTex0(fx->texId);
    s32 mode = fx->mode;
    f32 angle = fx->y; /* +0x1C: base draw angle for this entry */
    f32 scale = fx->x * 40.0f;

    if (mode == 1) {
        f32 dx = CosfVu0(angle) * 40.0f * fx->x;
        f32 dy = SinfVu0(angle) * 40.0f * fx->x;
        f32 ex = SinfVu0(angle) * 40.0f * fx->x;
        f32 ey = CosfVu0(angle) * -40.0f * fx->x;
        DrawRotatedSprite2d(x, y, scale, scale, angle, 0, 0, 0x3f, 0x3f, tex0,
                            0xfffff3, fx->color, 0, 0);
        DrawRotatedSprite2d(x + ex, y + ey, scale, scale, angle, 0, 0, 0x3f, 0x3f,
                            tex0, 0xfffff3, fx->color, 1, 0);
        DrawRotatedSprite2d(x - dx, y - dy, scale, scale, angle, 0, 0, 0x3f, 0x3f,
                            tex0, 0xfffff3, fx->color, 0, 1);
        DrawRotatedSprite2d((x + ex) - dx, (y + ey) - dy, scale, scale, angle, 0,
                            0, 0x3f, 0x3f, tex0, 0xfffff3, fx->color, 1, 1);
    } else if (mode == 0) {
        s32 i;
        for (i = 0; i < fx->drawFlags; i++) {
            DrawRotatedSprite2d(x, y, scale, scale, angle, 0, 0, 0x3f, 0x3f, tex0,
                                0xfffff3, fx->color, 0, 0);
            angle = WrapAnglePiSum(angle, fx->angle);
        }
    } else if (mode == 2) {
        DrawRotatedSprite2d(x, y, scale, scale, angle, 0.5f, 0.5f, 0x3f, 0x3f,
                            tex0, 0xfffff3, fx->color, 0, 0);
    }
}
#endif

/* func_00272CC0: screen-sprite-FX pre-pass (run by DrawScreenSpriteFxQueue
 * before the per-entry draw). WALL: sixteen callee-saves at 8-byte slot spacing
 * (packed-save wall); 0x1B8 bytes of interleaved projection/draw math. Left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00272CC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273320);

/* AddScreenSpriteFx(owner, color, worldPos, texId, mode, x, y, angle): queue a
 * screen-space sprite effect (cap 6 per frame; drawn by
 * DrawScreenSpriteFxQueue), skipped in cutscene state (g_nGameState==2).
 * Anchors at *worldPos when given (hasWorldPos=1) else at direct screen x/y;
 * the color alpha is capped by D_1A8630. mode -1 = derive: UI texture ids 0xD
 * and 0x1F..0x27 default to single-quad mode 2 at a fixed pi/2 angle; any
 * other explicit mode stores the caller's mode/angle (drawFlags=4).
 *
 * Best 83%, three independent structural deltas: (1) the original uses the
 * branch-LIKELY form (beqzl) on the worldPos==NULL test, which cc1 only emits
 * for the inverted block layout we can't drive from C; (2) it keeps a CSE
 * copy of the queue base in a second temp (move t4,v0) where cc1 rematerialises
 * the %lo; (3) the pi/2 immediate is the SDK PR_PI/2 macro value 0x3FC90FDC,
 * which the nearest float literal (0x3FC90FDB) misses by 1 ULP. WALL:
 * branch-likely layout + CSE-temp + macro-constant ULP. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", AddScreenSpriteFx);
#else
/* TODO(match): functional equivalent - not byte-exact; branch-likely block
   layout + a CSE copy of the queue base + the PR_PI/2 macro 0x3FC90FDC differing
   from the nearest float literal by 1 ULP (see header note above). */
void AddScreenSpriteFx(f32 x, f32 y, f32 angle, void *owner, u32 color,
                       const u_long128 *worldPos, s32 texId, s32 mode) {
    s32 alphaCap = D_1A8630;

    if (g_nGameState[0] == 2 || g_screenSpriteFxQueue.count >= 6) {
        return;
    }

    {
        ScreenSpriteFx *fx = &g_screenSpriteFxQueue.entries[g_screenSpriteFxQueue.count];

        fx->owner = owner;
        fx->x = x;
        fx->y = y;
        fx->color = color;
        fx->texId = texId;
        if ((s32)(color >> 24) > alphaCap) {
            fx->color = (color & 0xFFFFFF) | (alphaCap << 24);
        }

        if (worldPos == NULL) {
            fx->hasWorldPos = 0;
        } else {
            fx->worldPos = *worldPos;
            fx->hasWorldPos = 1;
        }

        if (mode == -1) {
            if (texId == 0xD || (texId > 0xC && texId < 0x28 && texId > 0x1E)) {
                fx->mode = 2;
                /* PR_PI/2 (0x3FC90FDC) */
                *(u32 *)&fx->angle = 0x3FC90FDC;
                fx->drawFlags = 0;
            }
        } else {
            fx->mode = mode;
            fx->drawFlags = 4;
            fx->angle = angle;
        }

        g_screenSpriteFxQueue.count++;
    }
}
#endif

/* DrawScreenSpriteFxQueue: final fx-layer pass. For each queued screen-sprite
 * effect, project world-anchored entries to screen space (skipping ones whose
 * owner moby is dead, state 0xFE/0xFD) and draw via DrawScreenSpriteFxEntry;
 * direct entries draw at the default screen centre. Clears the queue afterwards;
 * suppressed entirely while the hero is in state 0x6F. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", DrawScreenSpriteFxQueue);
#else
extern s32 g_heroState;
extern f32 IntToFloat(s32 x);
extern void func_00272cc0(void);
extern void ProjectWorldToScreen(f32 *out, ScreenSpriteFx *fx); /* FUN_0027a138 */
extern s32 g_screenCenterDefaultX; /* DAT_001a7348 */
extern s32 g_screenCenterDefaultY; /* DAT_001a734c */
extern s32 g_nGsPixelOffsetX;
extern s32 g_nGsPixelOffsetY;
/* TODO(match): functional equivalent - not byte-exact; eight callee-saves at
   8-byte slot spacing (packed-save wall). */
void DrawScreenSpriteFxQueue(void) {
    s32 i;
    f32 cx, cy;

    if (g_heroState == 0x6F) {
        g_screenSpriteFxQueue.count = 0;
        return;
    }

    func_00272cc0();
    if (g_screenSpriteFxQueue.count == 0) {
        return;
    }

    cx = IntToFloat(g_screenCenterDefaultX);
    cy = IntToFloat(g_screenCenterDefaultY);

    for (i = 0; i < g_screenSpriteFxQueue.count; i++) {
        ScreenSpriteFx *fx = &g_screenSpriteFxQueue.entries[i];
        f32 sx = cx;
        f32 sy = cy;

        if (fx->owner != NULL) {
            s8 ownerState = *(s8 *)((char *)fx->owner + 0x20);
            if (ownerState == -2 || ownerState == -3) {
                continue;
            }
            if (fx->hasWorldPos != 0) {
                f32 proj[2];
                ProjectWorldToScreen(proj, fx);
                sx = (proj[0] - IntToFloat(g_nGsPixelOffsetX)) * 0.0625f;
                sy = (proj[1] - IntToFloat(g_nGsPixelOffsetY)) * 0.0625f;
            }
            DrawScreenSpriteFxEntry(sx, sy, fx);
        }
    }

    g_screenSpriteFxQueue.count = 0;
}
#endif

/* func_002735A8: orphaned fill word (see header STUB-TABLE note) - not reachable
 * compiler output. Kept permanently INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002735A8);

/* Fog zone record: 0x80-stride entries at g_collTriBuffer+0x1540, indexed by
 * the zone id func_002A7490 returns. Two packed-RGB endpoint colours (+0x54 A,
 * +0x58 B) and four f32 param pairs lerped by the blend factor t: near/far
 * fog intensities and two 1024-scaled distances. Flag bit 2 (+0x50) enables
 * the zone. Field semantics UNCONFIRMED beyond widths/offsets. */
typedef struct FogZone {
    /* 0x00 */ u8 pad0[0x50];
    /* 0x50 */ s32 flags;         /* bit 2 = fog active */
    /* 0x54 */ u32 colorA;        /* packed 00RRGGBB endpoint at t=0 */
    /* 0x58 */ u32 colorB;        /* packed 00RRGGBB endpoint at t=1 */
    /* 0x5C */ f32 distA0;        /* 1024-scaled after lerp -> fog+0x8 */
    /* 0x60 */ f32 intensA0;      /* 255-complement after lerp -> fog+0x10 */
    /* 0x64 */ f32 distB0;        /* 1024-scaled after lerp -> fog+0xC */
    /* 0x68 */ f32 intensB0;      /* 255-complement after lerp -> fog+0x14 */
    /* 0x6C */ f32 distA1;
    /* 0x70 */ f32 intensA1;
    /* 0x74 */ f32 distB1;
    /* 0x78 */ f32 intensB1;
} FogZone;

/* SampleCameraFogZone: sample the fog volume the camera is inside (func_002A7490
 * locate, then lerp the per-zone fog colour/density into the fog-param block at
 * g_blobShadowCount+0x4.. -- bytes b/g/r at +4/5/6, distances*1024 at +8/+0xC,
 * 255-complemented intensities at +0x10/+0x14; deferred Track-B pin for the
 * block symbol. Old packed-save wall note superseded: engine unit, #else lane. */
#ifdef TARGET_NATIVE
extern s32 func_002A7490(Vec4 *pos, f32 *tOut, s32 *zoneOut);
extern s32 FloatToInt(f32 x);
extern u8 g_collTriBuffer[];
extern s32 g_blobShadowCount;
void SampleCameraFogZone(Vec4 *pos) {
    f32 t;
    s32 zone;
    u8 *fog = (u8 *)&g_blobShadowCount; /* param block lives at +0x4.. */
    FogZone *z;
    s32 ti, inv;
    f32 k;

    if (func_002A7490(pos, &t, &zone) == 0) {
        return;
    }
    z = (FogZone *)(g_collTriBuffer + 0x1540 + (zone << 7));
    if (!(z->flags & 2)) {
        return;
    }
    ti = FloatToInt(t * 255.0f);
    inv = 255 - ti;
    k = 1.0f - t;
    fog[4] = (u8)(((z->colorB & 0xFF) * ti + (z->colorA & 0xFF) * inv) >> 8);
    fog[5] = (u8)((((z->colorB >> 8) & 0xFF) * ti + ((z->colorA >> 8) & 0xFF) * inv) >> 8);
    fog[6] = (u8)((((z->colorB >> 16) & 0xFF) * ti + ((z->colorA >> 16) & 0xFF) * inv) >> 8);
    *(f32 *)(fog + 0x08) = (z->distA1 * t + z->distA0 * k) * 1024.0f;
    *(f32 *)(fog + 0x0C) = (z->distB1 * t + z->distB0 * k) * 1024.0f;
    *(f32 *)(fog + 0x10) = 255.0f - (z->intensA1 * t + z->intensA0 * k) * 255.0f;
    *(f32 *)(fog + 0x14) = 255.0f - (z->intensB1 * t + z->intensB0 * k) * 255.0f;
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", SampleCameraFogZone);
#endif

/* func_00273740: collision/projectile helper. WALL: ten callee-saves at 8-byte
 * slot spacing (packed-save wall) + interleaved fp/qword math. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273740);

/* func_00273988: collision/projectile helper (Vec4Scale/Sub + func_002B0C40
 * pushout). WALL: many callee-saves at 8-byte slot spacing (packed-save wall) +
 * interleaved fp/qword math. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273988);

/* func_00273B80: per-state moby/projectile dispatcher (state byte +0x5D, 6-way).
 * WALL: jump-table switch (jtbl_0026C510_text) + three callee-saves at 8-byte
 * slot spacing (packed-save wall). Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273B80);

/* func_00273D20: projectile collision-response handler (Vec3 cross/rescale,
 * pushout func_002B0E40, PlayMobySound on impact material). WALL: seven
 * callee-saves at 8-byte slot spacing (packed-save wall) + interleaved fp/qword
 * math. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273D20);

/* func_00273EA8: projectile/effect state helper (sibling of func_00273D20).
 * WALL: eight callee-saves at 8-byte slot spacing (packed-save wall) +
 * interleaved fp/qword math. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273EA8);

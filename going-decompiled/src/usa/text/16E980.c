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
    /* 0x00 */ u8 pad0[0x7D];
    /* 0x7D */ u8 unk7D;          /* cleared on activation */
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

/* Camera-system file-scope state block @0x1B5180 (pinned in symbol_addrs so
 * the base+offset accesses symbolize; +0x190 aliases the separately-named
 * g_activeCamera). Only fields this unit touches are declared. */
typedef struct CameraSysState {
    /* 0x000 */ u8 pad0[0x190];
    /* 0x190 */ volatile Camera *volatile activeCamera; /* fully volatile: the
                                                * original re-reads the pointer
                                                * between dereferences and keeps
                                                * the store order interleaved */
    /* 0x194 */ Camera *prevCamera;
    /* 0x198 */ u8 pad198[0xD0];
    /* 0x268 */ f32 fadeBlackRate;    /* per-tick fade step = 1/duration */
    /* 0x26C */ f32 fadeBlackTarget;  /* fade-to level */
} CameraSysState;
extern CameraSysState g_cameraState;

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
typedef struct CameraTransitionState {
    /* 0x00 */ u8 pad0[2];
    /* 0x02 */ u8 kind;               /* g_bCameraTransitionKind */
    /* 0x03 */ u8 pad3[0x4D];
    /* 0x50 */ Vec4 cur0;
    /* 0x60 */ Vec4 cur1;
    /* 0x70 */ u8 pad70[0x50];
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
 * mult chain; left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", BuildAttractReelPlaylist);

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

/* func_0026FC88: assemble a texture-upload descriptor for a 32-bit image
 * (FillMemory32 the 0x54-byte descriptor, then fill GS register words from the
 * image dims via Log2Floor). WALL: five callee-saves at 8-byte slot spacing
 * (packed-save wall) + the descriptor-building register packing. Left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FC88);

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
    u64 desc[6];   /* 0x30-byte texture-upload descriptor scratch */

    StartFileLoadPumpingVoice(g_proceduralAnimFrames,
                              g_discToc[0x330 / 4] + g_discToc[0x32C / 4],
                              g_discToc[0x334 / 4]);
    func_0026FC88(g_proceduralAnimFrames, desc, g_vramTextureBase_28, 0x3FFC00);
    g_bRawReadFellBack_34 = desc[0];
}
#endif

/* DebugPrintStub: varargs debug-print hook, compiled to a no-op in retail —
 * only the EABI varargs register spill remains, and crucially the ORIGINAL
 * also spills the FP argument registers $f12/$f14/$f16/$f18 (a printf-style
 * float-varargs prologue). The pinned cc1 emits the GPR varargs spill but
 * never the FP spill for a `(char*, ...)` body, so this cannot match from C.
 * WALL: float-varargs register-spill prologue. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", DebugPrintStub);

/* func_0026FF00: MIS-SPLIT fragment. The .s opens with six words
 * (sw $2,0x38($4) / nop / addiu $sp,0x10 / nop / addiu $sp,0x20 / nop) that are
 * the TAIL of the preceding function, then `alabel func_0026FF18` — the real
 * entry (a camera fov / fade-mode setter writing g_cameraState +0x3D4.. by
 * arg-0 mode). A clean C #else body would drop the leading tail words and shift
 * the segment layout, so this stays permanently INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FF00);

/* StepCameraFovInterp: camera FOV interpolation state machine (mode at
 * g_cameraState +0x3E8, enable flag +0x3E9). Eases the live FOV (+0x3D8) toward
 * its target by the active easing mode (spring func_002703C0, linear, smooth
 * func_002A8A68), then rebuilds the projection (BuildCameraProjection,
 * g_cameraProjScale = 0.5/tan(fov*0.5)). WALL: four fp/gpr callee-saves at
 * 8-byte slot spacing (packed-save wall) + branch-likely-driven fp control flow.
 * Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", StepCameraFovInterp);

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
    s32 i = 0;

    if (g_cameraCallbackCount > 0) {
        void (**slot)(void) = &g_cameraCallbacks[0];
        do {
            i++;
            (*slot)();
            slot++;
        } while (i < g_cameraCallbackCount);
    }
    g_cameraCallbackCount = 0;
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

/* TestCameraTakeover: decide whether a candidate camera slot should take over
 * the active camera (mode-vtbl takeover handler + a per-type priority switch).
 * WALL: jump-table switch (jtbl_0026C4F0_text) + three callee-saves at 8-byte
 * slot spacing (packed-save wall). Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", TestCameraTakeover);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270B68);

/* func_00270D60: build the camera-transition control vectors from the active
 * cinematic camera-key block (g_soundBankHandles +0x22B0). Normalises the three
 * key vectors (+0xC0/+0xD0/+0xE0) to unit length, stashes two of them at
 * g_cameraTransitionState +0x90/+0xA0, then forwards the normalised set to
 * func_00270B68 (the transition-curve builder) and finally copies +0xD0->+0xB0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00270D60);
#else
extern void func_00270B68(void *a, void *b, void *c, Vec4 *d, Vec4 *e, Vec4 *f);
extern void Vec3RescaleToLenVu0(Vec4 *dst, const Vec4 *src, f32 len);
extern char g_soundBankHandlesBlk[]; /* g_soundBankHandles + 0x20 */
/* TODO(match): functional equivalent - not byte-exact; five callee-saves
   (incl. $f20) at 8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void func_00270D60(void) {
    CameraTransitionState *t = &g_cameraTransitionState;
    /* +0x2290 holds the live cinematic camera-key block pointer (reloaded each
     * use); +0x80 is a separate control field forwarded to the curve builder. */
    char *keys = *(char **)(g_soundBankHandlesBlk + 0x2290);
    Vec4 v0, v1, v2;

    Vec3RescaleToLenVu0(&v0, (Vec4 *)(keys + 0xC0), 1.0f);
    keys = *(char **)(g_soundBankHandlesBlk + 0x2290);
    Vec3RescaleToLenVu0(&v1, (Vec4 *)(keys + 0xD0), 1.0f);
    keys = *(char **)(g_soundBankHandlesBlk + 0x2290);
    Vec3RescaleToLenVu0(&v2, (Vec4 *)(keys + 0xE0), 1.0f);

    *(Vec4 *)((char *)t + 0x90) = v0;
    *(Vec4 *)((char *)t + 0xA0) = v2;

    func_00270B68((char *)t + 0x70, (char *)t + 0xC0,
                  g_soundBankHandlesBlk + 0x80, &v0, &v1, &v2);

    *(Vec4 *)((char *)t + 0xB0) = *(Vec4 *)((char *)t + 0xD0);
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
        if (t->pad3[0] == 2) {
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

/* BeginCameraTransition: kick a camera-to-camera blend from prev to active.
 * WALL: seven callee-saves at 8-byte slot spacing (packed-save wall) + qword
 * block copies of the transition source pair. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", BeginCameraTransition);

/* func_00271140: instant-settle / blend-start branch of the transition pipeline
 * (forwarded from ApplyCameraTransition when no blend is pending). WALL: nine
 * fp/gpr callee-saves at 8-byte slot spacing (packed-save wall) + interleaved
 * Vec4 math and func_002A8A68/func_002AC468 calls. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00271140);

/* func_002712E8: running-blend interpolation branch of the transition pipeline
 * (forwarded from ApplyCameraTransition while a blend is in flight). WALL:
 * fourteen callee-saves at 8-byte slot spacing (packed-save wall); 0x410 bytes
 * of interleaved fp/qword math. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002712E8);

/* ApplyCameraTransition: advance the active camera blend into `out`. With no
 * transition pending (kind == 0) it forwards to func_00271140 (start/instant
 * settle); while one is running it forwards to func_002712E8 (interpolate). On
 * the running path, once the blend reports complete it copies the resulting
 * 4-row matrix into g_cameraMatrix and clears the transition (kind = 0, +0x0
 * halfword = 0). */
/* WALL: three callee-saves at 8-byte slot spacing (0x20-vs-0x10 packed-save).
   Left INCLUDE_ASM (no #else): the matrix copy gates on the INTERPOLATOR's RETURN
   value (not kind), and the 4th row goes to g_cameraPos (NOT g_cameraMatrix[3]) - a
   functional-equiv body is error-prone here; revisit with care. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", ApplyCameraTransition);

/* ApplyCameraShakeAxis: apply one shake channel to the camera - axis 0/1 offset
 * g_cameraPos along the matrix up/right rows, axis 2 rolls g_cameraMatrix; each
 * is sin/cos-windowed decay, skipped for camera type 6. WALL: five callee-saves
 * at 8-byte slot spacing (packed-save wall) + interleaved fp windowing math.
 * Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", ApplyCameraShakeAxis);

/* TrackHeroMotionForCamera: smooth the hero facing / velocity / speed / lateral
 * direction from g_heroPos deltas for the camera logic to consume. WALL: twelve
 * callee-saves at 8-byte slot spacing (packed-save wall) + heavy interleaved fp
 * math. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", TrackHeroMotionForCamera);

/* CheckCameraUnderwater: walk a short downward collision ray (up to 6 CollLine
 * steps) from the camera position to decide whether the camera is submerged,
 * comparing against GetWaterSurfaceHeight; records the result into the camera
 * state (+0x400). Skipped for camera type 6 and outside g_nGameState==0. WALL:
 * four callee-saves at 8-byte slot spacing (packed-save wall) + the
 * branch-likely-driven ray loop and qword block copies. Left INCLUDE_ASM
 * (result-offset/loop ordering is too easy to get subtly wrong). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", CheckCameraUnderwater);

/* func_00271FE8: per-frame camera-id / cinematic-state arbiter. Reads the active
 * cinematic key block (g_soundBankHandles +0x22B0) and player progress to pick
 * the live camera mode id (D_001B1300 +0x190) and a derived sub-id (+0x18C),
 * gated by distance thresholds and per-flag overrides. WALL: gp/absolute mix -
 * the same symbol (D_001B1300 +0x190) is accessed both via %gp_rel and via
 * %hi/%lo in the one function, which no single C small-data classification
 * reproduces. Frameless leaf otherwise. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00271FE8);

/* func_002721A8: start a fade-from-black — fade level forced to 1.0, target 0,
 * rate = 1/duration (g_cameraState +0x268/+0x26C). Instructions all match, but
 * the original schedules the g_cameraState lui/addiu address early and pads
 * the div.s latency with two nops before the divide (the SN-as
 * float-pipeline-latency padding). The pinned cc1 + GNU as emit no such pad.
 * Best 83%. WALL: SN-as div.s latency nop padding. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002721A8);
#else
/* TODO(match): functional equivalent - not byte-exact; SN-as div.s latency nop
   padding (two nops pad the divide that GNU as does not emit). */
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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", UpdateScreenFadeBlack);
#else
extern void func_002AB150(f32 *level, f32 target);
extern s32 func_002832F8(void *timer);
/* TODO(match): functional equivalent - not byte-exact; four callee-saves
   (incl. $f20) at 8-byte slot spacing (the 0x20-vs-0x10 packed-save wall). */
void UpdateScreenFadeBlack(void) {
    CameraSysState *cs = &g_cameraState;

    if (cs->fadeBlackRate == 0.0f) {
        return;
    }
    func_002AB150(&g_screenFadeBlack, cs->fadeBlackTarget);
    if (g_screenFadeBlack == cs->fadeBlackTarget) {
        if (func_002832F8((char *)cs + 0x270) == 1) {
            cs->fadeBlackTarget = 0.0f;
        }
    }
    if (cs->fadeBlackTarget <= 0.0f && g_screenFadeBlack <= 0.0f) {
        cs->fadeBlackRate = 0.0f;
    }
}
#endif

/* UpdateScreenFadeWhite: white-flash twin of UpdateScreenFadeBlack. Eases
 * g_screenFadeWhite toward fadeWhiteTarget (+0x27C) at fadeWhiteRate (+0x274);
 * idles while the rate is 0. Eases toward the target via func_002AB150, then
 * the settle/re-arm (func_002832F8 on the +0x280 timer) and rate-clear mirror
 * the black-fade path. */
/* WALL: three callee-saves at 8-byte slot spacing (0x20-vs-0x10 packed-save).
   Left INCLUDE_ASM (no #else): the ease step has a target==0 branch that eases
   toward the +0x278 field (not +0x27C) - easy to get subtly wrong; revisit with care. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", UpdateScreenFadeWhite);

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
extern void Vec3RescaleToLenVu0(Vec4 *dst, const Vec4 *src, f32 len);
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
    CameraSysState *cs = &g_cameraState;
    s32 transition;
    Camera *active;

    *(s32 *)((char *)cs + 0x404) += 1;
    UpdateScreenFadeBlack();
    UpdateScreenFadeWhite();
    func_00271FE8();
    TrackHeroMotionForCamera();
    DispatchCameraMode();

    active = (Camera *)cs->activeCamera;
    transition = *(u16 *)((char *)cs + 0x290);
    if ((u32)(transition - 1) < 2u) {
        BeginCameraTransition(active, cs->prevCamera);
    }

    if (*(s16 *)((char *)cs + 0x290) == 3) {
        ApplyCameraTransition((Vec4 *)active); /* a0 = cs->activeCamera, not cs+0x140 */
    } else {
        /* snap the live camera fields straight from the active slot */
        *(Vec4 *)((char *)cs + 0x140) = *(Vec4 *)((char *)active + 0x30);
        *(Vec4 *)((char *)cs + 0x370) = *(Vec4 *)((char *)active + 0x00);
        *(Vec4 *)((char *)cs + 0x380) = *(Vec4 *)((char *)active + 0x10);
        *(Vec4 *)((char *)cs + 0x390) = *(Vec4 *)((char *)active + 0x20);
    }

    if (func_0026F7C0() != 0) {
        /* cinematic override: copy pos + look vector, rebuild the basis */
        g_cameraPos = g_cinematicCameraBlock[0];
        *(Vec4 *)((char *)&g_cameraPos + 0x230) = g_cinematicCameraBlock[1];
        Vec3CrossVu0((Vec4 *)((char *)&g_cameraPos + 0x240),
                     (Vec4 *)((char *)&g_cameraPos + 0x230),
                     &g_cinematicCameraBlock[0]);
        Vec3RescaleToLenVu0((Vec4 *)((char *)&g_cameraPos + 0x240),
                            (Vec4 *)((char *)&g_cameraPos + 0x240), 1.0f);
        Vec3CrossVu0((Vec4 *)((char *)&g_cameraPos + 0x250),
                     (Vec4 *)((char *)&g_cameraPos + 0x240),
                     (Vec4 *)((char *)&g_cameraPos + 0x230));
        Vec3RescaleToLenVu0((Vec4 *)((char *)&g_cameraPos + 0x250),
                            (Vec4 *)((char *)&g_cameraPos + 0x250), 1.0f);
    }

    MatrixToEulerAngles((const Vec4 *)&g_cameraMatrix, &g_cameraRot);
    ApplyCameraShakeAxis(&g_cameraShakeUp, 0);
    ApplyCameraShakeAxis(&g_cameraShakeRight, 1);
    ApplyCameraShakeAxis(&g_cameraShakeRoll, 2);
    CheckCameraUnderwater();
    SampleCameraFogZone(&g_cameraPos);

    if (D_1A7A4C != 0) {
        Vec3CrossVu0((Vec4 *)((char *)&g_cameraMatrix + 0x10),
                     (Vec4 *)((char *)&g_cameraMatrix + 0x20),
                     (Vec4 *)&g_cameraMatrix);
    }
    StepCameraFovInterp();
    *(Vec4 *)((char *)&g_cameraMatrix + 0x50) = (Vec4){0};
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

/* SampleCameraFogZone: sample the fog volume the camera is inside (func_002A7490
 * locate, then lerp the per-zone fog colour/density into g_blobShadowCount+0x4..).
 * WALL: two callee-saves (incl. $f20) at 8-byte slot spacing (packed-save wall)
 * + byte-packed RGBA channel lerp with fixed-point mult chains. Left
 * INCLUDE_ASM (high ordering risk - left bare rather than risk an incorrect body). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", SampleCameraFogZone);

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

#include "common.h"

/* ===== EU build (SCES_516.07) - region twin of the USA text/16E980 unit
 * (src/usa/text/16E980.c): splash/attract boot helpers, the camera system
 * core and the screen-space sprite-FX queue. Built at -O2 -G8 (per-unit
 * GFLAG in tools/ee/objdiff_build.sh), the same as the USA sibling.
 *
 * The C bodies are REGION-AGNOSTIC - objdiff masks the gp/reloc deltas - so the
 * matched bodies are spliced verbatim from the USA unit; only the referenced
 * global symbol NAMES are retargeted to the EU addresses:
 *   USA g_cameraSlots    0x1B56D0  -> EU 0x1B5750 (spimdisasm names this
 *                                     `g_nVendorBuyQuantity + 0x3508`)
 *   USA g_cameraModeVtbl 0x26E900  -> EU D_0026E680
 * The 15 compiled-out hook stubs (return 0 / no-op) carry no relocs and match
 * for free. Functions on the USA-side instruction-divergence walls (qword
 * block-copy form, branch-likely layout, SN-as nop padding, 8-byte-packed
 * saves, etc.) stay INCLUDE_ASM here as well; see the USA unit for the
 * per-function wall notes. ===== */

/* USA g_cameraModeVtbl (0x14-byte stride, runtime-filled by level overlays).
 * In the EU asm tree this is the unnamed symbol D_0026E680. */
typedef struct CameraModeVtblEntry {
    /* 0x00 */ s32 id;
    /* 0x04 */ s32 (*takeover)();
    /* 0x08 */ s32 (*enter)();
    /* 0x0C */ s32 (*update)();
    /* 0x10 */ s32 (*poll)();
} CameraModeVtblEntry;
extern CameraModeVtblEntry D_0026E680[];

/* USA g_cameraSlots (48 records of 0xA0 bytes). The EU symbol is unnamed; the
 * asm reaches it as `g_nVendorBuyQuantity + 0x3508`, so the body indexes that
 * named base to reproduce the same relocation. */
typedef struct Camera {
    /* 0x00 */ u8 pad0[0x86];
    /* 0x86 */ s16 type;          /* slot search key */
    /* 0x88 */ u8 pad88[4];
    /* 0x8C */ s16 modeId;        /* index into the camera-mode vtable */
    /* 0x8E */ u8 pad8E[0x12];
} Camera;
extern u8 g_nVendorBuyQuantity[];

/* func_0026E838 (EU twin of USA ShowSplashImage): decompress a still-image
 * chunk and fade it in, uploading the texture and drawing a darkening
 * full-screen tint (0x80 -> below 0, step 2) each frame, pumping the frame
 * DMA/vblank pipeline.
 *
 * REGION DELTA (PAL vs USA ShowSplashImage): the EU per-frame loop ALSO
 * installs the draw-env-context-1 packet + a full-screen clear each iteration
 * (func_002856D8 == USA func_002857C8 twin, AppendDrawEnvContext1,
 * AppendScreenClearPacket) and inserts an extra func_0026E780() after
 * func_00285CE8 — none of which the USA NTSC ShowSplashImage.s emits (USA = 13
 * jals, EU = 17). Modeled faithfully to the EU asm and flagged.
 *
 * Matching arm stays INCLUDE_ASM (packed-save wall); #else is the structure
 * model (functional, not byte-exact). Word-verified vs USA ShowSplashImage +
 * EU .s: DecompressWad=func_0029DE68, ResetFrameArenas=func_002FD230,
 * func_002856D8, AppendDrawEnvContext1=func_00285768, AppendScreenClearPacket=
 * func_00285800, AppendTextureUploadBuildTex0=func_002FDB58, AppendFrameInitGsState=
 * func_0027BD40, DrawFullScreenTint=func_0027E2A8, AppendDrawEnvContext2=func_00285880,
 * func_00285BF8, func_0026E780(EU-only), KickFrameDmaChain=func_002FD2D8,
 * WaitFrameDmaFence=func_002FD418, WaitGsPathsIdle=func_001244B8,
 * WaitVblankGetField=func_001261F0; arena table g_nVendorBuyQuantity+0x8C78 [5],
 * g_vramFrameBufB = D_001A7308+0x54. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026E838);
#else
extern void DecompressWad(s32 wadId, void *header);
extern void func_0011AEA0(s32 a);
extern void ResetFrameArenas(void);
extern void func_002856D8(s32 a, s32 b, s32 c);   /* USA func_002857C8 twin */
extern void AppendDrawEnvContext1(void);
extern void AppendScreenClearPacket(s32 a);
extern u64  AppendTextureUploadBuildTex0(void *src, s32 vramDst, s32 w, s32 h);
extern void AppendFrameInitGsState(void);
extern void DrawFullScreenTint(s32 a, s32 b, s32 c, s32 d);
extern void AppendDrawEnvContext2(void);
extern void func_00285BF8(void);                  /* USA func_00285CE8 twin */
extern void func_0026E780(void);                  /* EU-only per-frame call */
extern void KickFrameDmaChain(void);
extern void WaitFrameDmaFence(s32 a);
extern void WaitGsPathsIdle(s32 a, s32 b);
extern void WaitVblankGetField(s32 a);
extern u8 D_001A7308[];                            /* +0x54 = g_vramFrameBufB */
typedef struct ImageChunkHeader {
    /* 0x00 */ s32 width;
    /* 0x04 */ s32 height;
    /* 0x08 */ u8 pad8[8];
    /* 0x10 */ u8 pixels[1];
} ImageChunkHeader;
/* Functional #else model (not byte-exact; packed-save wall on the matching arm). */
void func_0026E838(s32 wadId) {
    void **arenaTable = (void **)(g_nVendorBuyQuantity + 0x8C78);
    ImageChunkHeader *img = (ImageChunkHeader *)arenaTable[5];
    s32 brightness = 0x80;

    DecompressWad(wadId, img);
    func_0011AEA0(0);

    do {
        ResetFrameArenas();
        func_002856D8(0, 0, 0);
        AppendDrawEnvContext1();
        AppendScreenClearPacket(0);
        img = (ImageChunkHeader *)arenaTable[5];
        AppendTextureUploadBuildTex0((char *)img + 0x10,
                                     *(s32 *)(D_001A7308 + 0x54),
                                     img->width, img->height);
        AppendFrameInitGsState();
        DrawFullScreenTint(0, 0, 0, brightness);
        AppendDrawEnvContext2();
        brightness -= 2;
        func_00285BF8();
        func_0026E780();
        func_0011AEA0(0);
        KickFrameDmaChain();
        WaitFrameDmaFence(1);
        WaitGsPathsIdle(0, 0);
        WaitVblankGetField(0);
    } while (brightness >= 0);
}
#endif

/* func_0026E928 (EU twin of USA func_0026EAC8): render one fully pipelined
 * frame of the current loading image (single-frame sibling of func_0026E838's
 * fade loop) - decompress, reset arenas, install VIF1 DMA handlers, build+kick
 * a texture-upload + screen-clear frame, wait for retire, tear handlers down.
 *
 * REGION DELTA (PAL vs USA func_0026EAC8): EU inserts one extra func_0026E780()
 * after func_00285CE8, absent from the USA .s (USA = 17 jals, EU = 18).
 *
 * Matching arm stays INCLUDE_ASM (packed-save wall); #else is the structure
 * model (functional, not byte-exact). Word-verified vs USA func_0026EAC8 + EU .s:
 * DecompressWad=func_0029DE68, ResetFrameArenas=func_002FD230,
 * InstallVif1DmacHandlers=func_002FDD70, func_002856D8, AppendDrawEnvContext1=
 * func_00285768, AppendScreenClearPacket=func_00285800, AppendTextureUploadBuildTex0=
 * func_002FDB58, AppendFrameInitGsState=func_0027BD40, AppendDrawEnvContext2=
 * func_00285880, func_00285BF8, func_0026E780(EU-only), KickFrameDmaChain=
 * func_002FD2D8, WaitFrameDmaFence=func_002FD418, WaitGsPathsIdle=func_001244B8,
 * WaitVblankGetField=func_001261F0, RemoveVif1DmacHandlers=func_002FDE00;
 * g_vramFrameBufB = D_001A7308+0x54. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026E928);
#else
extern void DecompressWad(s32 wadId, void *header);
extern void func_0011AEA0(s32 a);
extern void ResetFrameArenas(void);
extern void InstallVif1DmacHandlers(void);
extern void func_002856D8(s32 a, s32 b, s32 c);   /* USA func_002857C8 twin */
extern void AppendDrawEnvContext1(void);
extern void AppendScreenClearPacket(s32 a);
extern u64  AppendTextureUploadBuildTex0(void *src, s32 vramDst, s32 w, s32 h);
extern void AppendFrameInitGsState(void);
extern void AppendDrawEnvContext2(void);
extern void func_00285BF8(void);                  /* USA func_00285CE8 twin */
extern void func_0026E780(void);                  /* EU-only extra call */
extern void KickFrameDmaChain(void);
extern void WaitFrameDmaFence(s32 a);
extern void WaitGsPathsIdle(s32 a, s32 b);
extern void WaitVblankGetField(s32 a);
extern void RemoveVif1DmacHandlers(void);
extern u8 D_001A7308[];                            /* +0x54 = g_vramFrameBufB */
/* ImageChunkHeader typedef defined above in func_0026E838's #else (file-scope). */
/* Functional #else model (not byte-exact; packed-save wall on the matching arm). */
void func_0026E928(s32 wadId) {
    void **arenaTable = (void **)(g_nVendorBuyQuantity + 0x8C78);
    ImageChunkHeader *img = (ImageChunkHeader *)arenaTable[5];

    DecompressWad(wadId, img);
    func_0011AEA0(0);
    ResetFrameArenas();
    InstallVif1DmacHandlers();
    func_002856D8(0, 0, 0);
    AppendDrawEnvContext1();
    AppendScreenClearPacket(0);

    img = (ImageChunkHeader *)arenaTable[5];
    AppendTextureUploadBuildTex0((char *)img + 0x10, *(s32 *)(D_001A7308 + 0x54),
                                 img->width, img->height);
    AppendFrameInitGsState();
    AppendDrawEnvContext2();
    func_00285BF8();
    func_0026E780();
    func_0011AEA0(0);
    KickFrameDmaChain();
    WaitFrameDmaFence(1);
    WaitGsPathsIdle(0, 0);
    WaitVblankGetField(0);
    RemoveVif1DmacHandlers();
}
#endif

/* func_0026EA00 (EU twin of USA func_0026EB98): pick a randomized ordering of
 * the three attract-reel slots - a 3-way random roll seeds (*a,*b,*c) with a
 * base permutation of {0,1,2}, then a coin-flip optionally swaps *b and *c.
 * Matching arm stays INCLUDE_ASM (packed-save wall); #else is the structure
 * model. Word-verified vs USA func_0026EB98 + EU .s: GetRandomInt = func_002A81F8
 * (called with 3 then 2 - exactly USA GetRandomInt(3)/GetRandomInt(2)); a=$4,
 * b=$5, c=$6. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026EA00);
#else
extern s32 GetRandomInt(s32 range);
void func_0026EA00(s32 *a, s32 *b, s32 *c) {
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026EAC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", LoadLevelAndInitHealth);

/* func_0026F580: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F580(void) {
    return 0;
}

void func_0026F588(void) {
}

void func_0026F590(void) {
}

void func_0026F598(void) {
}

/* func_0026F5A0: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F5A0(void) {
    return 0;
}

/* func_0026F5A8: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F5A8(void) {
    return 0;
}

/* func_0026F5B0: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F5B0(void) {
    return 0;
}

/* func_0026F5B8: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F5B8(void) {
    return 0;
}

void func_0026F5C0(void) {
}

void func_0026F5C8(void) {
}

void func_0026F5D0(void) {
}

void func_0026F5D8(void) {
}

void func_0026F5E0(void) {
}

void func_0026F5E8(void) {
}

void func_0026F5F0(void) {
}

void func_0026F5F8(void) {
}

void func_0026F600(void) {
}

void func_0026F608(void) {
}

/* func_0026F610: return-0 stub but the EU split folds 8 bytes of trailing
 * inter-function 0x0 padding into the body (jr/daddu + 2 nops, size 0x10). A
 * `return 0` compiles to only the 8-byte body, so this cannot match here; the
 * USA twin lands its stub without the trailing pad. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026F610);

void func_0026F620(void) {
}

/* func_0026F628: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F628(void) {
    return 0;
}

/* func_0026F630: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F630(void) {
    return 0;
}

/* func_0026F638: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F638(void) {
    return 0;
}

/* func_0026F640: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F640(void) {
    return 0;
}

/* func_0026F648: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F648(void) {
    return 0;
}

/* func_0026F650: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F650(void) {
    return 0;
}

/* func_0026F658: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F658(void) {
    return 0;
}

/* func_0026F660: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F660(void) {
    return 0;
}

/* func_0026F668: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F668(void) {
    return 0;
}

/* func_0026F670: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F670(void) {
    return 0;
}

void func_0026F678(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026F680);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FAE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FAE8);

/* func_0026FCB8 (EU twin of USA func_0026FE58): kick the boot dialog-voice file
 * load then build the splash image's texture-upload descriptor. Same in-level
 * driven-frame TRAP as the USA twin: its real body is a blocking voice/file
 * load (StartFileLoadPumpingVoice=func_002B8838 -> IOP RPC, deadlocks headless)
 * plus a GS texture upload (func_0026FC88=func_0026FAE8), latching the handle
 * into g_nSaveLoadStatusCode+0x48. Modeled as the same no-op #else as USA
 * func_0026FE58 (blocking-load contract). NOT a region delta. Matching arm stays
 * INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FCB8);
#else
void func_0026FCB8(void) {
}
#endif

/* func_0026FD28 (EU twin of USA DebugPrintStub): varargs debug-print hook,
 * compiled to a no-op in retail - only the EABI GPR varargs spill plus the FP
 * argument spill remain. The pinned cc1 never emits the FP spill for a
 * (char*, ...) body, so this cannot match from C (float-varargs register-spill
 * prologue wall). Matching arm stays INCLUDE_ASM; #else is the retail no-op
 * model (mirrors USA DebugPrintStub). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FD28);
#else
s32 func_0026FD28(const char *fmt, ...) {
    (void)fmt;
    return 0;
}
#endif

/* func_0026FD60 (EU twin of USA func_0026FF00): MIS-SPLIT tile. The .s opens
 * with six words that are the TAIL of the preceding function, then the real
 * entry glabel func_0026FD78. The matching build keeps the whole tile
 * INCLUDE_ASM (dropping the head words would shift the layout); the native build
 * gets the real function below under its interior name (mirrors USA
 * func_0026FF00 -> func_0026FF18). Word-verified vs USA func_0026FF18 + EU .s:
 * g_cameraState = g_nVendorBuyQuantity + 0x2FB8 (== USA 0x1B5180 + 0x80),
 * IntToFloat = func_002845A0; FOV field offsets identical to USA. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FD60);
#else
extern f32 IntToFloat(s32 x);
/* Camera-FOV interpolation sub-block of the camera-system state (EU base
 * g_nVendorBuyQuantity + 0x2FB8). */
typedef struct CamFovState {
    /* 0x000 */ u8  pad0[0x3D4];
    /* 0x3D4 */ f32 fovTarget;
    /* 0x3D8 */ f32 fovLive;
    /* 0x3DC */ f32 fovStiffness;
    /* 0x3E0 */ f32 fovDamping;
    /* 0x3E4 */ f32 fovMaxSpeed;
    /* 0x3E8 */ u8  fovMode;
    /* 0x3E9 */ u8  fovEnabled;
    /* 0x3EA */ s16 fovTimer;
    /* 0x3EC */ f32 fovRate;
    /* 0x3F0 */ f32 fovFrom;
} CamFovState;
/* func_0026FD78: start a camera-FOV ease. mode 0 = snap next tick; modes 1/2 =
 * timed linear/smooth ease over `frames` (0 frames degrades to a snap): latches
 * fovFrom=fovLive and rate=1/frames; mode 3 = critically-damped spring with the
 * given stiffness/damping/maxSpeed. Other modes: no-op. All arming paths set
 * fovEnabled=1. */
void func_0026FD78(s32 frames, s32 mode, f32 fov, f32 stiffness, f32 damping,
                   f32 maxSpeed) {
    CamFovState *cs = (CamFovState *)(g_nVendorBuyQuantity + 0x2FB8);

    switch (mode) {
    case 0:
        cs->fovEnabled = 1;
        cs->fovTarget = fov;
        cs->fovMode = 0;
        break;
    case 1:
    case 2:
        if (frames == 0) {
            cs->fovTarget = fov;
            cs->fovMode = 0;
        } else {
            cs->fovTimer = (s16)frames;
            cs->fovFrom = cs->fovLive;
            cs->fovMode = (u8)mode;
            cs->fovRate = 1.0f / IntToFloat(frames);
        }
        cs->fovEnabled = 1;
        break;
    case 3:
        cs->fovMaxSpeed = maxSpeed;
        cs->fovMode = (u8)mode;
        cs->fovTarget = fov;
        cs->fovEnabled = 1;
        cs->fovStiffness = stiffness;
        cs->fovDamping = damping;
        break;
    default:
        break;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FE88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270018);

/* func_00270020 (EU twin of USA func_002701C0): snapshot the active camera.
 * Copies the live 0xA0-byte active-camera slot and a following 0x280-byte block
 * into save buffers, then points the save header at the copied block. Matching
 * arm stays INCLUDE_ASM (packed-save wall); #else is the structure model.
 * Word-verified vs USA func_002701C0 + EU .s: CopyQwords = func_00283410;
 * g_activeCamera = g_nVendorBuyQuantity+0x3148, g_cameraSnapshot = +0x5308,
 * g_cameraHistorySnapshot = +0x59E8 (all USA base + 0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270020);
#else
extern void CopyQwords(void *dst, const void *src, s32 nbytes);
void func_00270020(void) {
    u8 *snapshot = g_nVendorBuyQuantity + 0x5308;
    u8 *historySnap = g_nVendorBuyQuantity + 0x59E8;

    CopyQwords(snapshot, *(const void **)(g_nVendorBuyQuantity + 0x3148), 0xA0);
    CopyQwords(historySnap, historySnap - 0x500, 0x280);
    *(u8 **)(snapshot + 0x70) = historySnap;
}
#endif

/* func_00270080 (EU twin of USA func_00270220): run every queued one-shot
 * camera callback (loop over D_001B1380+0x140 fn-ptr array, count at +0x180,
 * jalr each, then clear the count). Same in-level driven-frame TRAP as the USA
 * twin: in-level the callback slots hold OVERLAY function pointers and the loop
 * bound lives in the relocated band, so reads garbage / faults on real EE at the
 * static address. Modeled as the same no-op #else as USA func_00270220
 * (overlay-driven in-level). NOT a region delta. Matching arm stays INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270080);
#else
void func_00270080(void) {
}
#endif

/* func_002700F0 (USA func_00270290): find the first camera slot whose type
 * matches `type` (48-slot linear scan); returns NULL when no slot matches.
 * g_cameraSlots = g_nVendorBuyQuantity + 0x3508 in the EU asm. */
Camera *func_002700F0(s32 type) {
    Camera *cam = (Camera *)(g_nVendorBuyQuantity + 0x3508);

    do {
        if (cam->type == type) {
            return cam;
        }
        cam++;
    } while ((s32)cam < (s32)((Camera *)(g_nVendorBuyQuantity + 0x3508) + 48));
    return NULL;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270128);

/* func_00270138 (EU twin of USA func_002702D8): critically-damped spring step
 * toward a target. Advances the velocity at *vel by stiffness*delta -
 * damping*vel, clamps its magnitude to maxSpeed (when nonzero) then to |delta|,
 * and returns cur + clampedVel. Matching arm stays INCLUDE_ASM; #else is the
 * structure model (not byte-exact; fp-pipeline/reload wall). Word-verified vs USA
 * func_002702D8 + EU .s: GetFloatAbs = func_00283508 (sole callee); vel* = $4. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270138);
#else
extern f32 func_00283508(f32 x);   /* GetFloatAbs */
f32 func_00270138(f32 cur, f32 target, f32 stiffness, f32 damping, f32 maxSpeed,
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
    if (func_00283508(delta) < *vel) {
        *vel = func_00283508(delta);
    } else if (-func_00283508(delta) > *vel) {
        *vel = -func_00283508(delta);
    }
    return cur + *vel;
}
#endif

/* func_00270220 (EU twin of USA func_002703C0): angular sibling of func_00270138
 * - a critically-damped spring on a wrapped angle. delta is the shortest signed
 * angular difference target-cur (WrapAnglePiDiff); the velocity is integrated,
 * clamped to maxSpeed and to |delta|, and the result is cur + vel re-wrapped
 * into (-pi,pi] (WrapAnglePiSum). Matching arm stays INCLUDE_ASM; #else is the
 * structure model (not byte-exact). Word-verified vs USA func_002703C0 + EU .s:
 * WrapAnglePiDiff(target,cur) = func_002844A0 (first jal), GetFloatAbs =
 * func_00283508, WrapAnglePiSum(cur,*vel) = func_00284458 (last jal); vel* = $4. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270220);
#else
extern f32 func_002844A0(f32 a, f32 b);   /* WrapAnglePiDiff */
extern f32 func_00284458(f32 a, f32 b);   /* WrapAnglePiSum */
extern f32 func_00283508(f32 x);          /* GetFloatAbs */
f32 func_00270220(f32 cur, f32 target, f32 stiffness, f32 damping, f32 maxSpeed,
                  f32 *vel) {
    f32 delta = func_002844A0(target, cur);
    f32 v = *vel + (stiffness * delta - damping * *vel);
    *vel = v;
    if (maxSpeed != 0.0f) {
        if (v > maxSpeed) {
            *vel = maxSpeed;
        } else if (v < -maxSpeed) {
            *vel = -maxSpeed;
        }
    }
    if (func_00283508(delta) < *vel) {
        *vel = func_00283508(delta);
    } else if (-func_00283508(delta) > *vel) {
        *vel = -func_00283508(delta);
    }
    return func_00284458(cur, *vel);
}
#endif

/* func_00270340 (EU twin of USA func_002704E0): flag the active camera slot as
 * freshly (re)activated - sets the activation halfword (+0x7E = 1) and clears the
 * settle byte (+0x7D = 0) on the active camera (the original re-reads the
 * active-camera pointer for the second store). Matching arm stays INCLUDE_ASM;
 * #else is the structure model (not byte-exact). Word-verified vs USA func_002704E0
 * + EU .s: camera-state base = g_nVendorBuyQuantity + 0x2FB8 (== USA g_cameraState
 * 0x1B5180 + 0x80), activeCamera ptr at +0x190; fields +0x7E/+0x7D as USA. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270340);
#else
void func_00270340(void) {
    u8 *base = g_nVendorBuyQuantity + 0x2FB8;
    u8 *cam = *(u8 **)(base + 0x190);

    *(s16 *)(cam + 0x7E) = 1;
    cam = *(u8 **)(base + 0x190);   /* original re-reads the pointer */
    *(cam + 0x7D) = 0;
}
#endif

/* func_00270360 (EU twin of USA func_00270500): maintain a helper moby tied to a
 * camera slot. When the slot's type field (+0x86) is zero, lazily spawn the
 * helper moby into the helper slot; when nonzero, free it if present. Matching
 * arm stays INCLUDE_ASM; #else is the structure model (not byte-exact).
 * Word-verified vs USA func_00270500 + EU .s: helper-block base =
 * g_nVendorBuyQuantity + 0x3158 (== USA g_prevCamera+0xC = 0x1B5320), helper moby
 * ptr at +0xC4; spawn = func_00303B40 (USA func_00303818), FreeMoby = func_0029FCF8.
 * NOTE: the spawn arg is (helper-block base - 0x60), matching BOTH USA and EU .s
 * (`addiu $4,$16,-0x60`, $16 = g_prevCamera+0xC); the USA #else models it loosely
 * as (cam - 0x60), the EU #else follows the asm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270360);
#else
extern void *func_00303B40(void *cameraBase);   /* spawn helper moby */
extern void func_0029FCF8(void *moby);           /* FreeMoby */
void func_00270360(Camera *cam) {
    u8 *base = g_nVendorBuyQuantity + 0x3158;
    void **helper = (void **)(base + 0xC4);

    if (cam->type == 0) {
        if (*helper == NULL) {
            *helper = func_00303B40(base - 0x60);
        }
    } else if (*helper != NULL) {
        func_0029FCF8(*helper);
        *helper = NULL;
    }
}
#endif

/* func_002703C0 (USA CallCameraEnterHandler): invoke the camera-mode vtbl
 * `enter` handler (slot +0x8) for the camera's mode id, if installed. */
void func_002703C0(Camera *cam) {
    s32 (*handler)() = D_0026E680[cam->modeId].enter;

    if (handler != NULL) {
        handler(cam);
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270408);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002706A8);

/* func_00270870 (USA CallCameraPollHandler): invoke the camera-mode vtbl
 * `poll` handler (slot +0x10) for the camera's mode id, if installed. */
void func_00270870(Camera *cam) {
    s32 (*handler)() = D_0026E680[cam->modeId].poll;

    if (handler != NULL) {
        handler(cam);
    }
}

/* func_002708B8 (EU twin of USA DispatchCameraMode): per-frame camera arbitration
 * + mode update. Polls the active camera, scans all 48 slots for a higher-priority
 * active camera (TestCameraTakeover), switches to it on a win, runs that camera's
 * mode `update` handler, records its post-update position into the prev-pos fields
 * (+0x64/+0x68/+0x6c), and dispatches the queued one-shot callbacks. Always
 * returns -1. Matching arm stays INCLUDE_ASM; #else is the structure model (not
 * byte-exact). Word-verified vs USA DispatchCameraMode + EU .s: activeCamera ptr =
 * g_nVendorBuyQuantity+0x3148, g_cameraSlots = +0x3508, g_cameraSlotActive =
 * +0x5C68, vtbl = D_0026E680 (update @+0xC); CallCameraPollHandler = func_00270870,
 * TestCameraTakeover = func_002706A8, SwitchActiveCamera = func_00270408,
 * helper-moby = func_00270360, callback-dispatch = func_00270080. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002708B8);
#else
extern s32 func_002706A8(Camera *cand, Camera *cur);   /* TestCameraTakeover */
extern void func_00270408(Camera *cam);                 /* SwitchActiveCamera */
s32 func_002708B8(void) {
    Camera *chosen = *(Camera **)(g_nVendorBuyQuantity + 0x3148);
    Camera *slots = (Camera *)(g_nVendorBuyQuantity + 0x3508);
    s32 *active = (s32 *)(g_nVendorBuyQuantity + 0x5C68);
    s32 changed = 0;
    s32 i;
    s32 (*update)();
    f32 *pos;

    func_00270870(chosen);   /* CallCameraPollHandler */

    for (i = 0; i < 48; i++) {
        Camera *slot = &slots[i];
        if (active[i] != 0 && slot != chosen &&
            func_002706A8(slot, chosen) != 0) {
            changed = 1;
            chosen = slot;
        }
    }

    if (changed) {
        func_00270408(chosen);   /* SwitchActiveCamera */
    }

    update = D_0026E680[chosen->modeId].update;
    func_00270360(chosen);       /* helper-moby maintainer */

    pos = (f32 *)((char *)chosen + 0x30);
    if (update != NULL) {
        update(chosen);
    }
    /* prev-pos snapshot at +0x64/+0x68/+0x6c */
    *(f32 *)((char *)chosen + 0x64) = pos[0];
    *(f32 *)((char *)chosen + 0x68) = pos[1];
    *(f32 *)((char *)chosen + 0x6c) = pos[2];

    func_00270080();             /* run queued camera callbacks */
    return -1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002709C8);

/* func_00270BC0 (EU twin of USA func_00270D60): re-seed the spherical
 * running-blend state from the cine-key basis - normalise the three key rows
 * (keys+0xC0/D0/E0), latch fwd/up into basisFwd/basisUp, decompose the
 * src0-vs-(soundBankHandlesBlk+0x80) offset via the curve builder into sph*, and
 * mirror src1 into oriB. Matching arm stays INCLUDE_ASM; #else is the structure
 * model (not byte-exact). Word-verified vs USA func_00270D60 + EU .s:
 * transition-state base = g_nVendorBuyQuantity+0x3248, g_soundBankHandlesBlk =
 * g_sndChannelVolumes+0x1778 (cine-key ptr at +0x2290, control field at +0x80),
 * Vec3RescaleToLenVu0 = func_002837E0, curve builder (USA func_00270B68) =
 * func_002709C8; field offsets sphYaw 0x70 / basisFwd 0x90 / basisUp 0xA0 /
 * oriB 0xB0 / src0 0xC0 / src1 0xD0 identical to USA.
 * Vec4 + CameraTransitionState typedefs defined HERE (earliest transition-state
 * body); reused by func_00270CA0 / func_00270D18. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270BC0);
#else
typedef struct Vec4 { f32 x, y, z, w; } __attribute__((aligned(16))) Vec4;
/* Camera transition state block (EU base g_nVendorBuyQuantity + 0x3248 ==
 * USA g_cameraTransitionState). Only fields this batch touches are declared. */
typedef struct CameraTransitionState {
    /* 0x00 */ u8   pad0[0x02];
    /* 0x02 */ u8   kind;         /* latched blend kind; 0 = none pending */
    /* 0x03 */ u8   pendingKind;  /* requested blend kind for the next transition */
    /* 0x04 */ u8   pad4[0x4C];
    /* 0x50 */ Vec4 cur0;
    /* 0x60 */ Vec4 cur1;
    /* 0x70 */ f32  sphYaw;       /* running-blend state head */
    /* 0x74 */ u8   pad74[0x1C];
    /* 0x90 */ Vec4 basisFwd;     /* normalised cine-key basis */
    /* 0xA0 */ Vec4 basisUp;
    /* 0xB0 */ Vec4 oriB;
    /* 0xC0 */ Vec4 src0;
    /* 0xD0 */ Vec4 src1;
} CameraTransitionState;
extern void func_002837E0(Vec4 *dst, f32 len, const Vec4 *src);   /* Vec3RescaleToLenVu0 */
extern void func_002709C8(void *a, void *b, void *c, Vec4 *d, Vec4 *e, Vec4 *f); /* curve builder */
extern u8 g_sndChannelVolumes[];   /* +0x1778 = g_soundBankHandlesBlk */
void func_00270BC0(void) {
    CameraTransitionState *t =
        (CameraTransitionState *)(g_nVendorBuyQuantity + 0x3248);
    char *snd = (char *)(g_sndChannelVolumes + 0x1778);
    char *keys = *(char **)(snd + 0x2290);
    Vec4 v0, v1, v2;

    func_002837E0(&v0, 1.0f, (Vec4 *)(keys + 0xC0));
    keys = *(char **)(snd + 0x2290);
    func_002837E0(&v1, 1.0f, (Vec4 *)(keys + 0xD0));
    keys = *(char **)(snd + 0x2290);
    func_002837E0(&v2, 1.0f, (Vec4 *)(keys + 0xE0));

    t->basisFwd = v0;
    t->basisUp = v2;

    func_002709C8(&t->sphYaw, &t->src0, snd + 0x80, &v0, &v1, &v2);

    t->oriB = t->src1;
}
#endif

/* func_00270CA0 (EU twin of USA func_00270E40): when no transition is pending
 * (kind == 0), seed the saved source pair from the current pair: src0 = cur0,
 * src1 = cur1. If the requested blend kind (+0x3) is 2, src0 is first offset by
 * the hero-relative delta (g_heroPos + 0xD0) before being saved. Matching arm
 * stays INCLUDE_ASM; #else is the structure model (not byte-exact). Word-verified
 * vs USA func_00270E40 + EU .s: transition-state base = g_nVendorBuyQuantity+0x3248,
 * g_heroPos_D0 = g_sndChannelVolumes+0x18C8, Vec4AddVu0 = func_00283580.
 * (Vec4 / CameraTransitionState typedefs reused from func_00270BC0's #else.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270CA0);
#else
extern void func_00283580(Vec4 *dst, const Vec4 *a, const Vec4 *b);   /* Vec4AddVu0 */
extern u8 g_sndChannelVolumes[];   /* +0x18C8 = g_heroPos + 0xD0 */
void func_00270CA0(void) {
    CameraTransitionState *t =
        (CameraTransitionState *)(g_nVendorBuyQuantity + 0x3248);

    if (t->kind == 0) {
        t->src0 = t->cur0;
        if (t->pendingKind == 2) {
            /* asm arg order: a = hero-relative delta, b = src0 */
            func_00283580(&t->src0, (Vec4 *)(g_sndChannelVolumes + 0x18C8),
                          &t->src0);
        }
        t->src1 = t->cur1;
    }
}
#endif

/* func_00270D18 (EU twin of USA func_00270EB8): while a camera transition is
 * pending (kind != 0), snap the transition's current qword pos/target pair
 * (+0x50/+0x60) from the saved source pair (+0xC0/+0xD0). Matching arm stays
 * INCLUDE_ASM; #else is the structure model (not byte-exact; qword block-copy
 * addressing wall). Word-verified vs USA func_00270EB8 + EU .s: transition-state
 * base = g_nVendorBuyQuantity+0x3248; kind @+0x02, cur0 @+0x50, cur1 @+0x60,
 * src0 @+0xC0, src1 @+0xD0 identical to USA.
 * (Vec4 / CameraTransitionState typedefs reused from func_00270BC0's #else.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270D18);
#else
void func_00270D18(void) {
    CameraTransitionState *t =
        (CameraTransitionState *)(g_nVendorBuyQuantity + 0x3248);

    if (t->kind != 0) {
        t->cur0 = t->src0;
        t->cur1 = t->src1;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270D50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270FC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00271178);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00271758);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002717F8);

/* func_00271A30 (EU twin of USA TrackHeroMotionForCamera): smooth the hero
 * facing / velocity / speed / lateral direction from g_heroPos deltas for the
 * camera logic to consume. Matching arm stays INCLUDE_ASM (packed-save +
 * fp-pipeline wall); #else is the structure model (not byte-exact). Word-verified
 * vs USA TrackHeroMotionForCamera + EU .s: HeroCamMotion base =
 * g_nVendorBuyQuantity+0x3158 (== USA g_prevCamera+0xC), g_heroFacingDir =
 * g_nNanotechBonusHealTimer+0xE4, g_heroPos = that -0x240, g_soundBankHandlesBlk
 * = g_sndChannelVolumes+0x1778 (g_cameraZoneType @+0x24A0, g_heroState @+0x2294,
 * g_heroOrientVec[2] @+0x98); spring func_002702D8 = func_00270138,
 * Vec3RescaleToLenVu0 = func_002837E0, Vec3DotVu0 = func_00283670, Vec4SubVu0 =
 * func_002835B0, Vec3LengthVu0 = func_002836B0, Vec4ScaleVu0 = func_002835F0.
 * (Vec4 typedef reused from func_00270BC0's #else.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00271A30);
#else
extern f32  func_00270138(f32 cur, f32 target, f32 stiffness, f32 damping,
                          f32 maxSpeed, f32 *vel);          /* spring (func_002702D8) */
extern f32  func_00283670(const Vec4 *a, const Vec4 *b);    /* Vec3DotVu0 */
extern f32  func_002836B0(const Vec4 *v);                   /* Vec3LengthVu0 */
extern void func_002835B0(Vec4 *dst, const Vec4 *a, const Vec4 *b); /* Vec4SubVu0 */
extern void func_002835F0(Vec4 *dst, f32 s, const Vec4 *src);       /* Vec4ScaleVu0 */
extern void func_002837E0(Vec4 *dst, f32 len, const Vec4 *src);     /* Vec3RescaleToLenVu0 */
extern u8 g_nNanotechBonusHealTimer[];   /* +0xE4 = g_heroFacingDir */
extern u8 g_sndChannelVolumes[];         /* +0x1778 = g_soundBankHandlesBlk */
typedef struct HeroCamMotion {
    /* 0x00 */ f32 smoothX;
    /* 0x04 */ f32 smoothY;
    /* 0x08 */ f32 smoothHeight;
    /* 0x0C */ f32 rawZ;
    /* 0x10 */ f32 heightVel;
    /* 0x14 */ u8  pad14[0x0C];
    /* 0x20 */ Vec4 facingDir;
    /* 0x30 */ Vec4 facingRingCur;
    /* 0x40 */ Vec4 facingRingPrev;
    /* 0x50 */ Vec4 facingVel;
    /* 0x60 */ Vec4 prevPos;
    /* 0x70 */ Vec4 velocity;
    /* 0x80 */ Vec4 lateralDir;
    /* 0x90 */ Vec4 forwardProj;
    /* 0xA0 */ f32 speed;
    /* 0xA4 */ f32 lateralSpeed;
    /* 0xA8 */ f32 forwardSpeed;
    /* 0xAC */ f32 orientZRing[5];
} HeroCamMotion;
void func_00271A30(void) {
    HeroCamMotion *m = (HeroCamMotion *)(g_nVendorBuyQuantity + 0x3158);
    u8 *facingBase = g_nNanotechBonusHealTimer + 0xE4;   /* g_heroFacingDir */
    Vec4 *heroPos = (Vec4 *)(facingBase - 0x240);        /* g_heroPos */
    u8 *sbh = g_sndChannelVolumes + 0x1778;              /* g_soundBankHandlesBlk */
    Vec4 facing;    /* newest camera-facing sample = -normalize(hero facing) */
    Vec4 fwdProj;   /* facing scaled by the forward speed */
    f32 dot;
    s32 i;

    func_002837E0(&facing, -1.0f, (Vec4 *)facingBase);

    /* shift the 2-slot facing history, push the new (pre-nudge) sample */
    m->facingRingPrev = m->facingRingCur;
    m->facingRingCur = facing;

    /* if the new sample nearly reverses the smoothed facing, nudge it so the
       spring does not lock up at the antipode */
    dot = func_00283670(&m->facingDir, &facing);
    if (dot < -0.98f) {
        facing.x += 0.2f;
        facing.y += 0.2f;
        facing.z += 0.2f;
    }

    /* critically-damped spring of each facing axis toward the new sample */
    m->facingDir.x = func_00270138(m->facingDir.x, facing.x, 0.015f, 0.2f, 0.0f, &m->facingVel.x);
    m->facingDir.y = func_00270138(m->facingDir.y, facing.y, 0.015f, 0.2f, 0.0f, &m->facingVel.y);
    m->facingDir.z = func_00270138(m->facingDir.z, facing.z, 0.015f, 0.2f, 0.0f, &m->facingVel.z);
    func_002837E0(&m->facingDir, 1.0f, &m->facingDir);

    /* hero velocity + speed this frame (prevPos still holds last frame's pos) */
    func_002835B0(&m->velocity, heroPos, &m->prevPos);
    m->speed = func_002836B0(&m->velocity);

    /* split the velocity into forward (along facing) and lateral components */
    m->forwardSpeed = func_00283670(&m->velocity, &facing);
    func_002837E0(&fwdProj, m->forwardSpeed, &facing);
    m->forwardProj = fwdProj;
    func_002835B0(&m->lateralDir, &m->velocity, &fwdProj);
    m->lateralSpeed = func_002836B0(&m->lateralDir);
    func_002835F0(&m->lateralDir, 1.0f / m->lateralSpeed, &m->lateralDir);

    /* remember this frame's hero position for next frame's delta */
    m->prevPos = *heroPos;

    if (*(s32 *)(sbh + 0x24A0) == 0x50 && *(s32 *)(sbh + 0x2294) != 0x11) {
        m->smoothY = heroPos->y;
        m->smoothX = heroPos->x;
    } else {
        m->smoothX = heroPos->x;
        m->smoothY = heroPos->y;
        m->smoothHeight = func_00270138(m->smoothHeight, heroPos->z, 0.0075f, 0.175f,
                                        0.0f, &m->heightVel);
        m->rawZ = heroPos->z;
    }

    /* push the hero orient-Z history ring (shift down by one, append newest) */
    for (i = 0; i < 4; i++) {
        m->orientZRing[i] = m->orientZRing[i + 1];
    }
    m->orientZRing[i] = *(f32 *)(sbh + 0x98);   /* g_heroOrientVec[2] */
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00271D28);

/* func_00271E78 (EU twin of USA func_00271FE8): per-frame camera-id / cinematic-
 * state arbiter. Reads the active cinematic key block (g_soundBankHandlesBlk) and
 * player progress to pick the live camera mode id and a derived sub-id, gated by a
 * camera-Z distance threshold and a hardcoded hero-position rectangle override
 * (only at story progress 6), then dispatches a queued camera id from whichever
 * trigger flag byte is set. Matching arm stays INCLUDE_ASM (gp/absolute-mix wall);
 * #else is the structure model. Word-verified vs USA func_00271FE8 + EU .s:
 * g_soundBankHandlesBlk = g_sndChannelVolumes+0x1778, g_cameraCallbackCount =
 * D_001B1380+0x180 (pOut @+0x18C, pMode @+0x190, pSub @+0x194), g_cameraPos.z =
 * g_nVendorBuyQuantity+0x3100, g_cameraTriggerLatch = g_nVendorBuyQuantity+0x3158
 * +0xC0 (== USA 0x1B53E0), g_playerProgress kept by name. Rectangle floats
 * 485.7/570.7/250.0/334.5 identical to USA. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00271E78);
#else
extern s32 g_playerProgress;             /* story progress (word 0 of save block) */
extern u8  g_sndChannelVolumes[];        /* +0x1778 = g_soundBankHandlesBlk */
extern u8  D_001B1380[];                 /* +0x180 = g_cameraCallbackCount */
void func_00271E78(void) {
    u8 *sbh   = g_sndChannelVolumes + 0x1778;         /* g_soundBankHandlesBlk */
    s32 *pOut  = (s32 *)(D_001B1380 + 0x18C);          /* g_cameraCallbackCount +0xC */
    s32 *pMode = (s32 *)(D_001B1380 + 0x190);          /* +0x10 */
    s32 *pSub  = (s32 *)(D_001B1380 + 0x194);          /* +0x14 */
    u8 *cbBase = g_nVendorBuyQuantity + 0x3158;
    s32 *pLatch = (s32 *)(cbBase + 0xC0);              /* g_cameraTriggerLatch */
    s32 mode;

    /* Base mode from the cinematic-key control words. */
    *pMode = 0x14;
    if ((u32)(*(s32 *)(sbh + 0x229C) - 0x11) < 2 ||
        *(s32 *)(sbh + 0x2294) == 0x70) {
        *pMode = 0x34;
    }
    if (*(s32 *)(sbh + 0x229C) != 0x11) {
        if (*(f32 *)(sbh + 0x330) < *(f32 *)(g_nVendorBuyQuantity + 0x3100)) {
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
        *pLatch = 0x100;
        *pOut = 0x1B4;
    } else if (sbh[0x149B]) {
        *pLatch = 0xB00;
        *pOut = 0xBB4;
    } else if (sbh[0x1496]) {
        *pLatch = 0x300;
        *pOut = 0x3B4;
    } else if (sbh[0x149C]) {
        *pLatch = 0xD00;
        *pOut = 0xDB4;
    } else if (sbh[0x1494]) {
        *pLatch = 0;
        *pOut = 0xB4;
    } else {
        *pOut = *pLatch | 0xB4;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272038);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272068);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272120);

/* func_00272208 (EU twin of USA UpdateCamera): top-level per-frame camera tick.
 * Same in-level driven-frame TRAP as the USA twin: it dereferences the OVERLAY-
 * resident activeCamera pointer (g_cameraState+0x190) and other relocated-band
 * camera globals, faulting on real EE headless. Modeled as the same no-op #else
 * as USA UpdateCamera (the camera transform is externally supplied from the
 * in-level seed; func_00271A30/TrackHeroMotionForCamera is driven+validated
 * separately). NOT a region delta. Matching arm stays INCLUDE_ASM. Confirmed the
 * UpdateCamera twin by its EU .s call set (func_00271A30, func_00271E78,
 * func_002708B8, the transition kick/interp helpers, and the FOV interp). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272208);
#else
void func_00272208(void) {
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002723E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272468);

/* func_00272840 (EU twin of USA DrawScreenSpriteFxEntry): render one queued
 * screen-sprite effect at screen position (x,y), 40px-per-unit scale. mode 0 =
 * radial burst (repeat fx->drawFlags times, stepping the angle by fx->angle);
 * mode 1 = four mirrored quads offset along the angle; mode 2 = one centered
 * sprite. Matching arm stays INCLUDE_ASM (packed-save + fp-pipeline wall); #else
 * is the structure model. Word-verified vs USA DrawScreenSpriteFxEntry + EU .s:
 * GetUiTextureTex0 = func_0027CD60, DrawRotatedSprite2d = func_0027E990, CosfVu0 =
 * func_00283A58, SinfVu0 = func_00283A40, WrapAnglePiSum = func_00284458;
 * 40.0/0.5, color 0xFFFFF3; fx fields x@0x10 color@0x14 texId@0x18 y@0x1C
 * drawFlags@0x26 angle@0x28 mode@0x2C. ScreenSpriteFx typedef defined HERE
 * (earliest sprite-FX body); reused by func_002732B8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272840);
#else
extern u64  func_0027CD60(s32 texId);                /* GetUiTextureTex0 */
extern f32  func_00283A58(f32 a);                    /* CosfVu0 */
extern f32  func_00283A40(f32 a);                    /* SinfVu0 */
extern f32  func_00284458(f32 a, f32 b);             /* WrapAnglePiSum */
extern void func_0027E990(f32 x, f32 y, f32 w, f32 h, f32 angle, f32 u0, f32 v0,
                          s32 u1, s32 v1, u64 tex0, u32 color, u32 rgba,
                          s32 mirrorX, s32 mirrorY); /* DrawRotatedSprite2d */
typedef struct ScreenSpriteFx {
    /* 0x00 */ u8   worldPos[0x10];  /* world-space anchor (when hasWorldPos) */
    /* 0x10 */ f32  x;               /* direct screen x / scale base */
    /* 0x14 */ u32  color;           /* RGBA */
    /* 0x18 */ s32  texId;           /* UI texture id */
    /* 0x1C */ f32  y;               /* base draw angle for this entry */
    /* 0x20 */ void *owner;          /* owner moby (draw skipped when dead) */
    /* 0x24 */ s16  hasWorldPos;
    /* 0x26 */ s16  drawFlags;       /* mode-0 repeat count */
    /* 0x28 */ f32  angle;           /* per-step angle */
    /* 0x2C */ s32  mode;            /* 0 burst / 1 mirrored quads / 2 single */
} ScreenSpriteFx;
void func_00272840(f32 x, f32 y, ScreenSpriteFx *fx) {
    u64 tex0 = func_0027CD60(fx->texId);
    s32 mode = fx->mode;
    f32 angle = fx->y;              /* +0x1C: base draw angle for this entry */
    f32 scale = fx->x * 40.0f;

    if (mode == 1) {
        f32 dx = func_00283A58(angle) * 40.0f * fx->x;
        f32 dy = func_00283A40(angle) * 40.0f * fx->x;
        f32 ex = func_00283A40(angle) * 40.0f * fx->x;
        f32 ey = func_00283A58(angle) * -40.0f * fx->x;
        func_0027E990(x, y, scale, scale, angle, 0, 0, 0x3f, 0x3f, tex0,
                      0xfffff3, fx->color, 0, 0);
        func_0027E990(x + ex, y + ey, scale, scale, angle, 0, 0, 0x3f, 0x3f,
                      tex0, 0xfffff3, fx->color, 1, 0);
        func_0027E990(x - dx, y - dy, scale, scale, angle, 0, 0, 0x3f, 0x3f,
                      tex0, 0xfffff3, fx->color, 0, 1);
        func_0027E990((x + ex) - dx, (y + ey) - dy, scale, scale, angle, 0,
                      0, 0x3f, 0x3f, tex0, 0xfffff3, fx->color, 1, 1);
    } else if (mode == 0) {
        s32 i;
        for (i = 0; i < fx->drawFlags; i++) {
            func_0027E990(x, y, scale, scale, angle, 0, 0, 0x3f, 0x3f, tex0,
                          0xfffff3, fx->color, 0, 0);
            angle = func_00284458(angle, fx->angle);
        }
    } else if (mode == 2) {
        func_0027E990(x, y, scale, scale, angle, 0.5f, 0.5f, 0x3f, 0x3f,
                      tex0, 0xfffff3, fx->color, 0, 0);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272B50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002731B0);

/* func_002732B8 (EU twin of USA DrawScreenSpriteFxQueue): final fx-layer pass.
 * Runs the pre-pass, then for each queued screen-sprite effect projects world-
 * anchored entries to screen space (skipping ones whose owner moby is dead, state
 * 0xFE/0xFD) and draws via func_00272840; direct entries draw at the default
 * screen centre. Clears the queue afterwards; suppressed entirely while the hero
 * is in state 0x6F. Matching arm stays INCLUDE_ASM (packed-save wall); #else is
 * the structure model. Word-verified vs USA DrawScreenSpriteFxQueue + EU .s:
 * g_heroState = g_giantClankHealth+0xA20, g_screenSpriteFxQueue =
 * g_nVendorBuyQuantity+0x53B8 (count @+0x120, entries 0x30 stride), pre-pass
 * func_00272CC0 = func_00272B50, IntToFloat = func_002845A0, ProjectWorldToScreen
 * = func_00279FC0, DrawScreenSpriteFxEntry = func_00272840; g_screenCenterDefaultX/Y
 * = D_001A7308+0xC0/+0xC4, g_nGsPixelOffsetX/Y = D_001A7308+0xC8/+0xCC.
 * (ScreenSpriteFx typedef reused from func_00272840's #else.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002732B8);
#else
extern void func_00272B50(void);                        /* screen-sprite-FX pre-pass */
extern f32  func_002845A0(s32 x);                        /* IntToFloat */
extern void func_00279FC0(f32 *out, ScreenSpriteFx *fx); /* ProjectWorldToScreen */
extern u8   g_giantClankHealth[];                        /* +0xA20 = g_heroState */
extern u8   D_001A7308[];                                /* +0xC0/C4 centre, +0xC8/CC gs offset */
void func_002732B8(void) {
    u8 *queue = g_nVendorBuyQuantity + 0x53B8;           /* g_screenSpriteFxQueue */
    ScreenSpriteFx *entries = (ScreenSpriteFx *)queue;
    s32 *count = (s32 *)(queue + 0x120);
    s32 i;
    f32 cx, cy;

    if (*(s32 *)(g_giantClankHealth + 0xA20) == 0x6F) {  /* g_heroState */
        *count = 0;
        return;
    }

    func_00272B50();
    if (*count == 0) {
        return;
    }

    cx = func_002845A0(*(s32 *)(D_001A7308 + 0xC0));      /* g_screenCenterDefaultX */
    cy = func_002845A0(*(s32 *)(D_001A7308 + 0xC4));      /* g_screenCenterDefaultY */

    for (i = 0; i < *count; i++) {
        ScreenSpriteFx *fx = &entries[i];
        f32 sx = cx;
        f32 sy = cy;

        if (fx->owner != NULL) {
            s8 ownerState = *(s8 *)((char *)fx->owner + 0x20);
            if (ownerState == -2 || ownerState == -3) {
                continue;
            }
            if (fx->hasWorldPos != 0) {
                f32 proj[2];
                func_00279FC0(proj, fx);
                sx = (proj[0] - func_002845A0(*(s32 *)(D_001A7308 + 0xC8))) * 0.0625f;
                sy = (proj[1] - func_002845A0(*(s32 *)(D_001A7308 + 0xCC))) * 0.0625f;
            }
            func_00272840(sx, sy, fx);
        }
    }

    *count = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273438);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273440);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002735D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273818);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273A10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273BB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273D38);

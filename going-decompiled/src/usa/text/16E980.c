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
 * SAVE-LAYOUT WALL (PACKED-SAVE), MEASURED (task #512, 2026-09-20, master
 * f5fad751): of the 37 unguarded #else arms, 32 save >= 2 GPRs at 8-byte
 * stride in the ROM (tools/ee/.t512/18_gpr_saves.txt, from the frozen .s), and
 * cc1 2.9 reserves 16 bytes per GPR save, so those 32 cannot reach 100 on the
 * sdk29 arm whatever the C says. The other 5 (DebugPrintStub func_002704E0
 * func_00270EB8 func_00271FE8 AddScreenSpriteFx) are the only 2.9-eligible arms.
 * The engine96 arm (cc1 2.96-001003-1, -fno-schedule-insns -fno-strict-aliasing)
 * has 8-byte slots but differs in prologue/epilogue emission order
 * (SCHED-PROEPI), same-cycle emission order (SCHED-TIEBREAK: the ROM sets up
 * constant call args a1,a2,... and a0 LAST, filling the jal delay slot with a0;
 * both held compilers set a0 first) and sibling-calls every void tail call.
 *
 * WHOLE-UNIT BOTH-ARMS SCREEN (task #512): every #else arm promoted at once on
 * each arm, unit objdiff report over objdiff_build.sh (fuzzy %; tools/ee/.t512/
 * 05_all29_report.txt, 07_all96_report.txt, classes 08/09_classify*.txt). None
 * reached 100 on either arm; sdk29 >= engine96 on 31 of 40, but 32 of those are
 * PACKED-SAVE-walled there. The wall is a property of the 2.9 arm only: the
 * s136os arm (SN 1.36 -fopt-stack) has the ROM's 8-byte slots, and seventeen of
 * these arms are matched there (FACTs #8830, #9658, #9707, #9856, #10000; the
 * s136os re-screen of the remaining arms is NOTE #9857, with func_00270D60 and
 * func_00273D20 taken further in NOTEs #9994 and #9998). Table figures below are the
 * t512 measurements on the two held arms, kept as recorded.
 *   arm                        sdk29%  e96%   saves   residual class (sdk29 arm)
 *   ShowSplashImage             88.84  86.76  3g/0f   PACKED-SAVE,GPREL
 *   func_0026EAC8               91.96  88.10  2g/0f   PACKED-SAVE,SIBCALL,GPREL
 *   func_0026EB98               67.51  56.13  4g/0f   PACKED-SAVE,LIKELY-
 *   BuildAttractReelPlaylist    70.34  68.39  7g/0f   PACKED-SAVE,MULT
 *   func_0026FC88               64.78  29.62  6g/0f   PACKED-SAVE,MULT
 *   func_0026FE58                5.19   5.19  2g/0f   PACKED-SAVE,-
 *   DebugPrintStub              64.29  62.29  0g/0f   -
 *   StepCameraFovInterp         28.68  19.80  3g/2f   PACKED-SAVE,SIBCALL,GPREL
 *   func_002701C0               57.65  54.96  3g/0f   PACKED-SAVE,GPREL
 *   func_00270220                5.19   5.19  3g/0f   PACKED-SAVE,LIKELY-
 *   func_002702D8               94.07  49.89  2g/2f   PACKED-SAVE,-
 *   func_002703C0               99.65  68.85  2g/5f   PACKED-SAVE,-
 *   func_00270500               53.92  73.92  2g/0f   PACKED-SAVE,GPREL
 *   TestCameraTakeover          81.03  59.97  4g/0f   PACKED-SAVE,GPREL
 *   DispatchCameraMode          79.93  54.15  6g/0f   PACKED-SAVE,-
 *   func_00270B68               88.16  76.58  8g/4f   PACKED-SAVE,LIKELY+
 *   func_00270D60               31.69  56.55  5g/1f   PACKED-SAVE,-
 *   func_00270E40               62.14  50.45  2g/0f   PACKED-SAVE,LIKELY-
 *   func_00270EB8               58.57  57.86  0g/0f   -
 *   BeginCameraTransition       54.37  51.32  7g/1f   PACKED-SAVE,LIKELY-
 *   func_00271140               85.11  69.03  5g/3f   PACKED-SAVE,-
 *   func_002712E8               59.27  53.15  10g/6f  PACKED-SAVE,GPREL
 *   ApplyCameraTransition       52.30  54.60  3g/0f   PACKED-SAVE,-
 *   ApplyCameraShakeAxis        77.87  60.29  3g/3f   PACKED-SAVE,GPREL
 *   TrackHeroMotionForCamera    55.33  42.81  9g/4f   PACKED-SAVE,LIKELY-,GPREL
 *   func_00271FE8               75.86  60.38  0g/0f   GPREL
 *   UpdateCamera                 1.19   1.19  4g/1f   PACKED-SAVE,-
 *   DrawScreenSpriteFxEntry      4.74   7.33  4g/5f   PACKED-SAVE,SIBCALL,LIKELY-
 *   AddScreenSpriteFx           57.48  48.89  0g/0f   MULT,GPREL
 *   DrawScreenSpriteFxQueue     46.64  44.15  8g/1f   PACKED-SAVE,GPREL
 *   SampleCameraFogZone         75.16  14.12  2g/1f   PACKED-SAVE,MULT,LIKELY-,GPREL
 *   func_00273740               65.50  70.46  10g/2f  PACKED-SAVE,-
 *   func_00273988               15.17  16.16  4g/1f   PACKED-SAVE,LIKELY-
 *   func_00273B80               72.83  72.35  4g/0f   PACKED-SAVE,GPREL
 *   func_00273D20               87.95  84.24  8g/0f   PACKED-SAVE,LIKELY+
 *   func_00273EA8               80.64  74.47  6g/3f   PACKED-SAVE,GPREL
 *   CheckCameraUnderwater       39.33  39.33  -g/-f   -
 *   func_002721A8               65.83  65.83  -g/-f   -
 *   UpdateScreenFadeBlack       86.52  86.52  -g/-f   -
 *   UpdateScreenFadeWhite       77.96  77.96  -g/-f   -
 * Per-arm levers RUN and their result are in each arm's TODO(match) comment
 * where the number changed; unlisted arms carry the screen number above.
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
__asm__(".extern g_vramTextureBase_28, 16");  /* func_0026FE58 */
__asm__(".extern g_bRawReadFellBack_34, 16"); /* func_0026FE58 */
__asm__(".extern g_cameraCallbackCount, 16"); /* func_00270220 */
__asm__(".extern g_vramFrameBufB, 16");       /* ShowSplashImage, func_0026EAC8 */
__asm__(".extern g_nGameState, 16");          /* AddScreenSpriteFx */

/* True small data (complete <=8-byte externs, %gp_rel). */
extern s32 D_1A8630;             /* screen-sprite-FX alpha cap (set by the HUD fade) */

/* Large / ordinary-absolute globals. */
extern s32 g_nGameState;         /* cc1-small, assembler-absolute: the
                                  * `.extern g_nGameState, 16` above */
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
    /* 0x190 */ union { Camera *p; } activeCamera; /* read as a UNION MEMBER, not
                                                * volatile: the original (built
                                                * without strict aliasing) re-reads
                                                * this pointer after every store
                                                * through it. cc1 2.9 -O2 applies
                                                * type-based aliasing, so a plain
                                                * `Camera *` field is CSE'd across
                                                * the `sh`; a union member access
                                                * has alias set 0 and forces the
                                                * same reload WITHOUT volatile's
                                                * side effect of pinning the store
                                                * out of the jr delay slot (t512:
                                                * func_002704E0 80% volatile ->
                                                * 100% union, sdk29 arm). */
    /* 0x194 */ Camera *prevCamera;
    /* 0x198 */ u8 pad198[0xD0];
    /* 0x268 */ f32 fadeBlackRate;    /* per-tick fade step = 1/duration */
    /* 0x26C */ f32 fadeBlackTarget;  /* fade-to level */
    /* 0x270 */ u8 fadeBlackTimer[4]; /* TickCountdownTimer re-arm timer */
    /* 0x274 */ f32 fadeWhiteRate;    /* white-flash per-tick step */
    /* 0x278 */ f32 fadeWhiteOutRate; /* alternate step clamp used while fadeWhiteTarget==0 (fade-out) */
    /* 0x27C */ f32 fadeWhiteTarget;  /* white flash-to level */
    /* 0x280 */ u8 fadeWhiteTimer[4]; /* TickCountdownTimer re-arm timer */
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
/* WaitGsPathsIdle is sceGsSyncPath (vendored libgraph.h: int sceGsSyncPath(int
 * mode, u_short timeout)); the ROM returns 0 in $v0. Declared value-returning
 * (RULING #9118): as void, cc1 treats $v0 as free across the call and
 * func_0026FC88's s136os body allocates differently (4 words). */
extern s32 WaitGsPathsIdle(s32 a, s32 b);
extern s32 WaitVblankGetField(s32 a);
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
 * DMA / vblank pipeline. Used for boot/menu stills.
 *   wadId: the WAD entry holding the image chunk (decompressed into the
 *          arena-table slot 5 buffer).
 * MATCHED on the s136os arm (task #1901; the 2.9 arm's packed-save wall does
 * not apply there). Devices and phrasing, each measured in a solo s136 compile:
 * - g_vramFrameBufB is read as `lui $5 / lw $5,%lo($5)`, gas's expansion of a
 *   cc1-small symbol the assembler knows is not small data: the unit-level
 *   `.extern g_vramFrameBufB, 16` above (RULING #8620 family). Without it the
 *   load is %gp_rel and the body one word short (33 positional differences).
 * - the brightness counter is set after the setup calls (set at its
 *   declaration, `li $16,0x80` issues after the `sd $31`: 3 words) and
 *   stepped after the last call of the body (the old arm stepped it after
 *   AppendDrawEnvContext2, which issues the tint call's a1 before a2: 2
 *   words). A do/while with the same two placements is byte-identical.
 * GUARD: on EE this C is compiled alone by SN 2.95.3 v1.36 -fopt-stack
 * (tools/ee/s136os_functions.txt) and spliced over the S136OS_SLOT line by
 * tools/ee/s136os_splice.sh; on native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_ShowSplashImage)
S136OS_SLOT(ShowSplashImage);
#else
void ShowSplashImage(s32 wadId) {
    ImageChunkHeader *img = (ImageChunkHeader *)g_memoryArenaTable[5];
    s32 brightness;

    DecompressWad(wadId, img);
    func_0011AEA0(0);

    for (brightness = 0x80; brightness >= 0; brightness -= 2) {
        ResetFrameArenas();
        img = (ImageChunkHeader *)g_memoryArenaTable[5];
        AppendTextureUploadBuildTex0((char *)img + 0x10, g_vramFrameBufB,
                                     img->width, img->height);
        AppendFrameInitGsState();
        DrawFullScreenTint(0, 0, 0, brightness);
        AppendDrawEnvContext2();
        func_00285CE8();
        func_0011AEA0(0);
        KickFrameDmaChain();
        WaitFrameDmaFence(1);
        WaitGsPathsIdle(0, 0);
        WaitVblankGetField(0);
    }
}
#endif

/* func_0026EAC8: render one fully pipelined frame of the current loading image.
 * Decompresses the image chunk, resets the frame arenas, installs the VIF1 DMA
 * handlers, builds and kicks a frame that uploads the image texture and clears
 * the screen, then waits for the frame to retire before tearing the handlers
 * back down. The single-frame sibling of ShowSplashImage's fade loop.
 *   wadId: the WAD entry holding the image chunk.
 * MATCHED on the s136os arm (task #1901). The C is unchanged from the old
 * #else arm; the one device is the unit-level `.extern g_vramFrameBufB, 16`
 * shared with ShowSplashImage (without it the g_vramFrameBufB load is %gp_rel
 * and the function one word short, 28 positional differences). The image pointer is re-read from the
 * arena table after the setup calls, as the ROM does (`lw $2,0x14($16)`).
 * GUARD: compiled alone by SN 2.95.3 v1.36 -fopt-stack and spliced over the
 * S136OS_SLOT line (tools/ee/s136os_splice.sh); plain C on native. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0026EAC8)
S136OS_SLOT(func_0026EAC8);
#else
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
 * second coin-flip optionally swaps b and c.
 *   a, b, c: out slots, each receiving 0, 1 or 2.
 * MATCHED on the s136os arm (task #1901). No device. The roll is a `switch`
 * with the cases in 0, 1, 2 order: the ROM's dispatch is cc1's balanced
 * decision tree (`beq 1`, then `slti <2`), which the old if/else-if chain
 * does not produce (an if/else-if chain in the same order: 4 words short,
 * 38 positional differences). The swap reads *b first (`t = *c` first: 8).
 * GUARD: compiled alone by SN 2.95.3 v1.36 -fopt-stack and spliced over the
 * S136OS_SLOT line (tools/ee/s136os_splice.sh); plain C on native. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0026EB98)
S136OS_SLOT(func_0026EB98);
#else
extern s32 GetRandomInt(s32 range);
void func_0026EB98(s32 *a, s32 *b, s32 *c) {
    switch (GetRandomInt(3)) {
    case 0:
        *a = 0;
        *b = 1;
        *c = 2;
        break;
    case 1:
        *a = 1;
        *b = 0;
        *c = 2;
        break;
    case 2:
        *a = 2;
        *b = 1;
        *c = 0;
        break;
    }

    if (GetRandomInt(2) != 0) {
        s32 t = *b;
        *b = *c;
        *c = t;
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026F7B0);

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026F820);

/* func_0026F850: large boot/menu still-image build helper. WALL: jump-table
 * switch + eight callee-saves at 8-byte slot spacing (packed-save wall). Left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026F850);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FC80);

/* func_0026FC88: upload a palettised texture (16x16 CLUT + image) to GS VRAM
 * and emit a 3-qword TEX0 packet into descOut. The source blob is a header:
 * +0x8 width, +0xC height, +0x14 CLUT pixel mode (0 = CT32 -> 0x400-byte CLUT,
 * else CT16 -> 0x200), CLUT data at +0x20, image data right after the CLUT.
 * Uploads the CLUT (16x16, clutVram>>8) then the image (PSM 0x1B, imageVram>>8,
 * buffer width max(1,w/64)), each via func_00126288 descriptor + GIF-path image
 * kick + path idle; then packs TEX0: TBP/TBW/PSM 0x1B/TW/TH/TCC/CBP/CPSM +
 * bit 63, and descOut = { TEX0, 1, 0 }.
 *   src:       the texture blob (GsTexBlobHeader).
 *   descOut:   receives the three doublewords { TEX0, 1, 0 }.
 *   imageVram: image VRAM byte address (>> 8 = TBP).
 *   clutVram:  CLUT VRAM byte address (>> 8 = CBP).
 * MATCHED on the s136os arm (task #1970). No asm devices. What closed it,
 * each priced by removing it alone in a solo s136 compile (positional word
 * differences of 116, relocations masked, before the dli row below, which the
 * solo screen does not apply and which accounts for 2 of each figure):
 *   - clutBp (clutVram >> 8) formed after the CLUT-size test, not at its
 *     declaration: the ROM keeps clutVram in s1 and shifts it in the first
 *     Log2Floor delay slot (removing: 115 words, 97);
 *   - the CLUT size as an if/else testing clutPsm == 0 (a ternary compiles to
 *     movn: 115 words, 105; testing != 0 inverts the branch: +3);
 *   - TEX0 packed from sign-extended (u64) fields, as the ROM's lw + dsll
 *     (a (u32) cast per field adds zero-extension: +5);
 *   - imageData = tex + (clutBytes + 0x20), the ROM's association (+2);
 *   - WaitGsPathsIdle declared s32 at unit level (see there; void: +4).
 * The bit-63 constant (`dli $3,0x8000000000000000`) is a RULING #8549 site:
 * the ROM carries SN Ps2EeAs's `addiu $3,$0,-1; dsll32 $3,$3,31`, GNU as
 * expands it `ori $3,$0,0x8000; dsll32 $3,$3,16`; the row is in
 * tools/ee/ps2eeas_dli_sites.txt.
 * GUARD: on EE this C is compiled alone by SN 2.95.3 v1.36 -fopt-stack
 * (tools/ee/s136os_functions.txt) and spliced over the S136OS_SLOT line by
 * tools/ee/s136os_splice.sh; on native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0026FC88)
S136OS_SLOT(func_0026FC88);
#else
extern void FillMemory32(void *dst, s32 val, s32 nbytes);
extern s32 Log2Floor(s32 x);
extern void func_00126288(void *ctx, s32 bp, s32 bw, s32 psm, s32 x, s32 y,
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
    s32 clutBp;
    u64 tex0;

    FillMemory32(&blk, 0, 0x54);
    blk.clutData = tex->data;
    if (tex->clutPsm == 0) {
        blk.clutBytes = 0x400;
    } else {
        blk.clutBytes = 0x200;
    }
    clutBp = clutVram >> 8;
    blk.log2W = Log2Floor(tex->width);
    blk.log2H = Log2Floor(tex->height);
    blk.imageData = (u8 *)tex + (blk.clutBytes + 0x20);
    blk.pixelCount = tex->width * tex->height;
    func_00126288(ctx, (s16)clutBp, 1, (s16)tex->clutPsm, 0, 0, 0x10, 0x10);
    func_0011AEA0(0);
    KickGifImageUpload(ctx, blk.clutData);
    WaitGsPathsIdle(0, 0);
    blk.bufWidth = tex->width >> 6;
    if (blk.bufWidth <= 0) {
        blk.bufWidth = 1;
    }
    blk.imageBp = imageVram >> 8;
    func_00126288(ctx, (s16)blk.imageBp, (s16)blk.bufWidth, 0x1B, 0, 0,
                (s16)tex->width, (s16)tex->height);
    func_0011AEA0(0);
    KickGifImageUpload(ctx, blk.imageData);
    WaitGsPathsIdle(0, 0);
    tex0 = (u64)blk.imageBp
         | ((u64)blk.bufWidth << 14)
         | ((u64)0x1B << 20)
         | ((u64)blk.log2W << 26)
         | ((u64)blk.log2H << 30)
         | ((u64)1 << 34)                  /* TCC = RGBA */
         | ((u64)clutBp << 37)             /* CBP */
         | ((u64)tex->clutPsm << 51)       /* CPSM */
         | ((u64)1 << 63);
    out[0] = tex0;
    out[1] = 1;
    out[2] = 0;
}
#endif

/* func_0026FE58: kick the boot dialog-voice file load then build the splash
 * image's texture-upload descriptor. Streams the voice chunk described by the
 * disc TOC WAD fields (sector = +0x330 + +0x32C, count = +0x334) into
 * g_proceduralAnimFrames, then calls func_0026FC88 to assemble a texture
 * descriptor for that buffer (VRAM base g_vramTextureBase[+0x28], 0x3FFC00)
 * into a stack block and latches its first doubleword into the raw-read state
 * block (+0x34). No params, no return.
 * MATCHED on the s136os arm (task #1800). Both latched globals are reached
 * through the one-insn symbolic macro (`lui; lw %lo` into the destination,
 * and the `$at` form of the `sd`), so they are declared assembler-absolute
 * with `.extern sym, 16` at unit level (directive only, emits nothing; see
 * the device block at the top): a plain -G8 extern of <= 8 bytes would be
 * %gp_rel, and an in-arm device is refused by the splice (cc1's own `, 4`
 * line would then follow it).
 * GUARD: as DebugPrintStub's — the s136os splice supplies the EE body.
 * Native keeps its no-op: this is an IN-LEVEL native frame-path TRAP (tester
 * inlevel_trap_list.md) — a blocking voice/file load (IOP RPC, deadlocks
 * headless) plus a GS texture upload, not seed-reclaimable. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0026FE58)
S136OS_SLOT(func_0026FE58);
#else
extern void StartFileLoadPumpingVoice(void *dst, s32 lbn, s32 size);
extern void func_0026FC88(void *src, void *descOut, s32 imageVram, s32 clutVram);
extern u8 g_proceduralAnimFrames[];
extern s32 g_discToc[];                 /* +0x32C/+0x330/+0x334 WAD fields */
extern s32 g_vramTextureBase_28;        /* g_vramTextureBase + 0x28 */
extern u64 g_bRawReadFellBack_34;       /* g_bRawReadFellBack + 0x34 */
void func_0026FE58(void) {
#ifndef TARGET_NATIVE
    u64 desc[3];
    s32 *toc = g_discToc;

    StartFileLoadPumpingVoice(g_proceduralAnimFrames,
                              toc[0x330 / 4] + toc[0x32C / 4], toc[0x334 / 4]);
    func_0026FC88(g_proceduralAnimFrames, desc, g_vramTextureBase_28, 0x3FFC00);
    g_bRawReadFellBack_34 = desc[0];
#endif
}
#endif

/* DebugPrintStub: varargs debug-print hook, compiled to a no-op in retail.
 * Takes a printf-style format and its arguments and ignores all of them. Only
 * the EABI varargs prologue survives: the seven GPR argument registers
 * $5..$11 and the four FP argument registers $f12/$f14/$f16/$f18 are spilled
 * to the 0x80-byte frame.
 * Return: declared s32 because callers declare it so and two (198FA0,
 * 250080) forward the result; the ROM body never writes $v0, so on EE the C
 * has no return statement (a `return 0` adds a word). Native keeps the
 * defined 0 it always returned.
 * MATCHED on the s136os arm (task #1767): SN 2.95.3 v1.36 cc1 emits the FP
 * varargs spill that cc1 2.9 never does (the former "float-varargs prologue"
 * wall was a compiler-revision wall, not a C one).
 * GUARD: on EE this C is the image's body, compiled alone by SN 2.95.3 v1.36
 * -fopt-stack (tools/ee/s136os_functions.txt) and spliced over the
 * S136OS_SLOT line by tools/ee/s136os_splice.sh; the 2.9 compile sees only the
 * slot. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_DebugPrintStub)
S136OS_SLOT(DebugPrintStub);
#else
s32 DebugPrintStub(const char *fmt, ...) {
#ifdef TARGET_NATIVE
    return 0;
#endif
}
#endif

/* func_0026FF00: 0x18 bytes of dead inter-function debris (sw $2,0x38($4) /
 * nop / addiu $sp,0x10 / nop / addiu $sp,0x20 / nop) that splat used to fuse
 * onto the real function func_0026FF18 below; carved apart in task #472. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FF00);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_0026FF18);
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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002701B8);

/* func_002701C0: snapshot the active camera (alias SnapshotCameraState).
 * Copies the live 0xA0-byte active camera slot into g_cameraSnapshot and the
 * 0x280-byte camera history block into g_cameraHistorySnapshot, then points
 * the snapshot's +0x70 word at the saved history copy.
 * The history source (g_cameraHistory, 0x1B76B0) is spelled as
 * g_cameraHistorySnapshot - 0x500, and the +0x70 pointer as a field of the
 * snapshot: the ROM derives both from the destination registers
 * (`addiu a1,s0,-0x500`, `sw s0,0x70(s1)`), which cc1 does only when the C
 * names them relative to the same symbol.
 * MATCHED on the s136os arm (task #1767). GUARD: as DebugPrintStub's — the
 * s136os splice supplies the EE body; native compiles this C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002701C0)
S136OS_SLOT(func_002701C0);
#else
extern void CopyQwords(void *dst, const void *src, s32 nbytes);
typedef struct CameraSnapshot {
    /* 0x00 */ u8 pad0[0x70];
    /* 0x70 */ u8 *history;           /* -> g_cameraHistorySnapshot */
    /* 0x74 */ u8 pad74[0x2C];
} CameraSnapshot;                     /* 0x1B74D0; a copy of one 0xA0-byte Camera */
extern CameraSnapshot g_cameraSnapshot;
extern u8 g_cameraHistorySnapshot[0x280]; /* 0x1B7BB0; g_cameraHistory is 0x500 below */
void func_002701C0(void) {
    CopyQwords(&g_cameraSnapshot, g_cameraState.activeCamera.p, 0xA0);
    CopyQwords(g_cameraHistorySnapshot, g_cameraHistorySnapshot - 0x500, 0x280);
    g_cameraSnapshot.history = g_cameraHistorySnapshot;
}
#endif

/* func_00270220: run every queued one-shot camera callback (alias
 * RunCameraCallbacks). g_cameraCallbackCount (D_001B1300+0x180) holds the
 * queued count and g_cameraCallbacks (D_001B1300+0x140) the function-pointer
 * array. Calls each pointer in turn, re-reading the live count each iteration
 * (a callback may change the queue), then clears the count. No params, no
 * return.
 * MATCHED on the s136os arm (task #1800). The plain indexed loop is what
 * gives the ROM's registers (i in $16, the strength-reduced slot pointer in
 * $17); the pointer-walking do/while spellings swap them.
 * The count is reached through the
 * one-insn symbolic macro (`lui; lw %lo` and the `$at` form of the clearing
 * `sw`), so it is declared assembler-absolute with `.extern sym, 16` at
 * unit level (directive only, emits nothing; device block at the top); a
 * plain -G8 extern would be %gp_rel.
 * GUARD: as DebugPrintStub's — the s136os splice supplies the EE body.
 * Native keeps its no-op: this is an IN-LEVEL native frame-path TRAP (tester
 * inlevel_trap_list.md) — in-level the slots hold OVERLAY function pointers
 * and the count (0x1B1480) sits in the relocated band. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00270220)
S136OS_SLOT(func_00270220);
#else
extern s32 g_cameraCallbackCount;          /* D_001B1300 + 0x180 */
extern void (*g_cameraCallbacks[])(void);  /* D_001B1300 + 0x140 */
void func_00270220(void) {
#ifndef TARGET_NATIVE
    s32 i;

    for (i = 0; i < g_cameraCallbackCount; i++) {
        g_cameraCallbacks[i]();
    }
    g_cameraCallbackCount = 0;
#endif
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002702C8);

/* func_002702D8: critically-damped spring step toward a target. Advances `cur`
 * (the velocity at *vel) by stiffness*delta - damping*vel, clamps the velocity
 * magnitude to maxSpeed (when nonzero) and then to |delta|, and returns the new
 * position cur + clampedVel. GetFloatAbs(delta) is re-evaluated at each
 * branch, as the original does.
 * MATCHED on the s136os arm (FACT #8830; cc1 2.9 never closed it).
 * GUARD (task #1269): on EE this C is the image's body, compiled alone by SN
 * 2.95.3 v1.36 -fopt-stack (tools/ee/s136os_functions.txt) and spliced over the
 * S136OS_SLOT line by tools/ee/s136os_splice.sh; the 2.9 compile sees only the
 * slot, so a build that skips the splice loses the function. On native it is
 * plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002702D8)
S136OS_SLOT(func_002702D8);
#else
extern f32 GetFloatAbs(f32 x);
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
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002703C0)
S136OS_SLOT(func_002703C0);
#else
extern f32 WrapAnglePiDiff(f32 a, f32 b);
extern f32 WrapAnglePiSum(f32 a, f32 b);
extern f32 GetFloatAbs(f32 x);
/* MATCHED on the s136os arm (task #1901): SN 1.36 -fopt-stack gives the ROM's
 * 8-byte save slots, which were the whole 2.9-arm residual (t512: sdk29 99.65%,
 * frame only). No device. The GetFloatAbs prototype above is required: it is
 * otherwise declared only inside func_002702D8's guarded arm, which this
 * function's solo s136 TU does not see, and the implicit-int call costs 17
 * words (int return, cvt.s.w, 88 words built against the ROM's 71).
 *   cur/target: angles in radians; stiffness/damping: spring gains;
 *   maxSpeed: velocity cap (0 = none); vel: in/out angular velocity.
 *   Returns the new wrapped angle. */
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

/* func_002704E0 (0x002704E0): flag the active camera slot as freshly
 * (re)activated — sets the activation halfword (+0x7E = 1) and clears the
 * settle byte (+0x7D = 0) on g_cameraState.activeCamera. No params, no return.
 * The pointer is re-read for the second store (lw; sh; lw; jr; sb in the delay
 * slot): that is the union-member (alias-set-0) read of `activeCamera`, see the
 * CameraSysState comment — a volatile field forced the reload but kept the sb
 * out of the delay slot (80%), a plain field CSE'd it away.
 * MATCHED 100.00% on the sdk29 arm (unit objdiff report, objdiff_build.sh, clean;
 * verify_match_unit.sh BYTE IDENTICAL, task #512). */
void func_002704E0(void) {
    g_cameraState.activeCamera.p->unk7E = 1;
    g_cameraState.activeCamera.p->unk7D = 0;
}

/* func_00270500: maintain the camera helper moby (alias
 * UpdateCameraHelperMobySlot). When the slot's type field (+0x86) is zero,
 * lazily spawn the helper moby at the camera position (func_00303818) into
 * g_cameraHelperMoby; when nonzero, free it if present and clear the pointer.
 * Params: cam - the camera slot whose type decides spawn vs free. No return.
 * The ROM anchors one callee-saved base at g_cameraState + 0x1A0 (0x1B5320,
 * g_heroCamMotion) and reaches both the helper pointer (+0xC4 = 0x1B53E4,
 * g_cameraHelperMoby) and the spawn position (-0x60 = 0x1B52C0, the camera
 * position) from it: the spawn argument is that FIXED address, not derived
 * from `cam` (FACT #5824 — the former `cam - 0x60` arm was wrong). The
 * TrackHeroMotionForCamera arm uses the same `&g_cameraState + 0x1A0` idiom.
 * MATCHED on the s136os arm (task #1800; body from NOTE #9661, minus its
 * empty fence after FreeMoby: removing it leaves the assembled function
 * word-identical — cc1 emits the jal in reorder mode and gas pads the delay
 * slot with the ROM's nop either way). No devices.
 * GUARD: as DebugPrintStub's — the s136os splice supplies the EE body;
 * native compiles this C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00270500)
S136OS_SLOT(func_00270500);
#else
extern void *func_00303818(void *pos);
typedef struct CameraHelperView {   /* g_cameraState + 0x1A0 (0x1B5320) */
    /* 0x00 */ u8 pad0[0xC4];
    /* 0xC4 */ void *helperMoby;      /* g_cameraHelperMoby (0x1B53E4) */
} CameraHelperView;
void func_00270500(Camera *cam) {
    extern void FreeMoby(void *moby);
    CameraHelperView *m = (CameraHelperView *)((char *)&g_cameraState + 0x1A0);
    if (cam->type == 0) {
        if (m->helperMoby == NULL) {
            m->helperMoby = func_00303818((char *)m - 0x60);
        }
    } else if (m->helperMoby != NULL) {
        FreeMoby(m->helperMoby);
        m->helperMoby = NULL;
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
 * fields (+0x64/+0x68/+0x6c). Always returns -1.
 * MATCHED on the s136os arm (task #1901). No device; phrasing only, each
 * measured in a solo s136 compile against the old arm's spelling:
 * - the active-flag test is its own `if` around the slot test, so cc1 forms
 *   %hi(g_cameraSlotActive) before %hi(g_cameraSlots) as the ROM does (the
 *   single && chain swaps the two lui and their registers: 2 words);
 * - `chosen` is stored before `changed` (the ROM's `daddu $16,$17` first;
 *   the other order: 2 words);
 * - the +0x30 -> +0x64 snapshot pointers are formed after the update call
 *   (forming `pos` before it keeps it in a callee-saved register across the
 *   call and reorders the tail: 25 words).
 * GUARD: compiled alone by SN 2.95.3 v1.36 -fopt-stack and spliced over the
 * S136OS_SLOT line (tools/ee/s136os_splice.sh); plain C on native. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_DispatchCameraMode)
S136OS_SLOT(DispatchCameraMode);
#else
extern s32 g_cameraSlotActive[48];
extern void func_00270500(Camera *cam);
extern void func_00270220(void);
extern s32 TestCameraTakeover(Camera *candidate, Camera *current);
extern void SwitchActiveCamera(Camera *cam);
s32 DispatchCameraMode(void) {
    Camera *chosen = g_cameraState.activeCamera.p;
    s32 changed = 0;
    s32 i;
    s32 (*update)();

    CallCameraPollHandler(g_cameraState.activeCamera.p);

    for (i = 0; i < 48; i++) {
        if (g_cameraSlotActive[i] != 0) {
            Camera *slot = &g_cameraSlots[i];
            if (slot != chosen && TestCameraTakeover(slot, chosen) != 0) {
                chosen = slot;
                changed = 1;
            }
        }
    }

    if (changed) {
        SwitchActiveCamera(chosen);
    }

    update = g_cameraModeVtbl[chosen->modeId].update;
    func_00270500(chosen);

    if (update != NULL) {
        update(chosen);
    }
    {
        /* prev-pos snapshot: +0x30 position -> +0x64/+0x68/+0x6c */
        f32 *src = (f32 *)((char *)chosen + 0x30);
        f32 *dst = (f32 *)((char *)chosen + 0x64);
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
    }

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

/* EE_REG: pin a live local to a register on the EE arm only (RULING #8598,
 * REGISTER-PIN DEVICE); empty on native. Used by func_00270E40 and
 * func_00270EB8 below. */
#ifndef TARGET_NATIVE
#define EE_REG(r) __asm__(r)
#else
#define EE_REG(r)
#endif

/* func_00270E40: when no transition is pending (kind == 0), seed the saved
 * source pair from the current pair: src0 = cur0, src1 = cur1. If the
 * transition mode byte (+0x3) is 2, src0 is first offset by the hero-relative
 * delta (g_heroPos + 0xD0) before being saved. No params, no return.
 * MATCHED on the s136os arm (task #1970). The ROM copies each qword through
 * address registers (`addiu a1,s0,0xc0; addiu v1,s0,0x50; lq v0,0(v1);
 * sq v0,0(a1)`), the 16-byte block-move shape func_00270EB8 documents; SN 1.36
 * cc1 folds the addresses into offset-form lq/sq instead. Devices, each
 * priced by removing it ALONE in a solo s136 compile (positional word
 * differences of 29, relocations masked):
 *   - EMPTY asm fences (RULING #8483; emit nothing) on dst/src (4 words) and
 *     on src1/dst1 (removing it: 27 words, 11 differences) keep each address
 *     in its own register;
 *   - the untied volatile barrier taking `src` as an input (5 words) holds the
 *     pendingKind test below the first sq: without it cc1 schedules the
 *     `li $2,2` into the lq->sq slot and every later register shifts;
 *   - `src1` pinned to $4 (RULING #8598; 5 words), the ROM's register for the
 *     second pair's source (a $3 pin on dst1 instead is equivalent; one pin
 *     suffices);
 *   - dst1 is formed before src1 in the source (3 words).
 * GUARD: on EE this C is compiled alone by SN 2.95.3 v1.36 -fopt-stack
 * (tools/ee/s136os_functions.txt) and spliced over the S136OS_SLOT line by
 * tools/ee/s136os_splice.sh; native compiles it, EE_REG empty and the fences
 * emitting nothing, as the plain two-qword copy. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00270E40)
S136OS_SLOT(func_00270E40);
#else
extern void Vec4AddVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern Vec4 g_heroPos_D0;   /* g_heroPos + 0xD0 */
void func_00270E40(void) {
    CameraTransitionState *t = &g_cameraTransitionState;

    if (t->kind == 0) {
        Vec4 *dst = &t->src0;
        Vec4 *src = &t->cur0;
        register Vec4 *src1 EE_REG("$4");
        Vec4 *dst1;
        __asm__("" : "+r"(dst), "+r"(src));
        *dst = *src;
        __asm__ __volatile__("" : : "r"(src));
        if (t->pendingKind == 2) {
            /* asm arg order: a = delta (its .w is kept), b = src0 */
            Vec4AddVu0(dst, &g_heroPos_D0, dst);
        }
        dst1 = &t->src1;
        src1 = &t->cur1;
        __asm__("" : "+r"(src1), "+r"(dst1));
        *dst1 = *src1;
    }
}
#endif

/* func_00270EB8: while a camera transition is pending (kind != 0), snap the
 * transition's current qword pos/target pair (+0x50/+0x60) from the saved
 * source pair (+0xC0/+0xD0). No params, no return.
 * MATCHED on the s136os arm (task #1767). The ROM copies each qword through
 * address registers (`addiu a0,a2,0x50; addiu v1,a2,0xc0; lq v0,0(v1);
 * sq v0,0(a0)`, then the +0xD0/+0x60 pair) — the shape of a 16-byte block
 * move, whose expander forces both addresses into registers and clobbers its
 * own scratch. SN 1.36 cc1 compiles every 16-byte struct copy here as an
 * offset-form TImode move instead (Vec4, u_long128, byte-array, volatile,
 * 12-in-16 and memcpy spellings all measured: `lq v0,0xc0(a2)`), so:
 *   - the three EMPTY asm fences (RULING #8483; they emit nothing) make each
 *     address opaque so cc1 cannot fold it into the lq/sq offset; the middle
 *     one is `volatile` and re-defines `t`, which holds the second pair's
 *     address setup below the first `sq` as in the ROM (removing it, or only
 *     its `volatile`, reorders that setup);
 *   - EE_REG pins `src` to $3 and `src1` to $5 (RULING #8598, REGISTER-PIN
 *     DEVICE): without them cc1 shares registers the block move's clobbers
 *     kept apart. Each pin and each fence was removed alone and the order or
 *     registers moved; pins on `dst` ($4) and `dst1` ($3) were dead weight
 *     and are not here.
 * GUARD: as DebugPrintStub's. Native: EE_REG is empty, the fences emit
 * nothing, and the body is the plain two-qword copy. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00270EB8)
S136OS_SLOT(func_00270EB8);
#else
void func_00270EB8(void) {
    CameraTransitionState *t = &g_cameraTransitionState;
    if (t->kind != 0) {
        Vec4 *dst = &t->cur0;
        register Vec4 *src EE_REG("$3") = &t->src0;
        register Vec4 *src1 EE_REG("$5");
        Vec4 *dst1;
        __asm__("" : "+r"(dst), "+r"(src));
        *dst = *src;
        __asm__ __volatile__("" : "+r"(t));
        dst1 = &t->cur1;
        src1 = &t->src1;
        __asm__("" : "+r"(src1), "+r"(dst1));
        *dst1 = *src1;
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
 *  - orientation band (UNCERTAIN OPERANDS): oriB -> matrix (QuatToMatrix3
 *    at sp+0x80), roll delta = (PI/2 - acos-form) signed by row1 dot,
 *    >PI/2 wrap-correction with 2PI(0x40C90FDC) adjust, <1e-5(0x3727C5AC)
 *    fast-path copies rows, else func_002AC4D0 quat + func_002ADC50
 *    applications; final rows renormalized (row1 = cross with
 *    g_heroFacingDir, len -1.0!), func_002AC468 captures into src1 AND oriB,
 *    TickCountdownTimer ticks blendTarget; return 0. */
/* WALL:
 * fourteen callee-saves at 8-byte slot spacing (packed-save wall); 0x410 bytes
 * of interleaved fp/qword math. Left INCLUDE_ASM. */
#ifdef TARGET_NATIVE
extern void Vec3CrossVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern void Vec4ScaleVu0(Vec4 *dst, f32 s, const Vec4 *src);
extern void func_002AC4D0(void *dstQuat, const Vec4 *axis, f32 ang);
extern void func_002ADC50(void *dst, const Vec4 *v, const void *quat);
extern void QuatToMatrix3(void *rot, void *mtx); /* rotation -> 3-row matrix */
extern void func_002AC468(void *dstOri, Camera *cam);
extern s32 TickCountdownTimer(void *timer);
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
    QuatToMatrix3(&st->oriB, mtxRows);
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
    TickCountdownTimer(&st->target);
    return 0;
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002712E8);
#endif

/* ApplyCameraTransition: advance the active camera blend into `out`. With no
 * transition pending (kind == 0) it forwards to func_00271140 (start/instant
 * settle); while one is running it forwards to func_002712E8 (interpolate).
 * Once the call reports complete it copies the resulting 4-row matrix out:
 * rows 0-2 into g_cameraMatrix and row 3 into g_cameraPos (0x230 below the
 * matrix, addressed from the same base register as the ROM does), then clears
 * the transition (+0x0 halfword = 0, kind = 0).
 *   out: the 4-row (Vec4) camera matrix the blend writes.
 * MATCHED on the s136os arm (task #1970). The row copies are the 16-byte
 * block-move shape (each address in its own register, offset-0 lq/sq; see
 * func_00270EB8), which SN 1.36 cc1 otherwise folds into offset-form lq/sq.
 * Devices, each priced by removing it ALONE in a solo s136 compile
 * (positional word differences of 40, relocations masked):
 *   - EMPTY tied asm fences (RULING #8483; emit nothing) on each row copy's
 *     dst/src pair: rows 1, 2 and 3 cost 19 (39 words), 15 (39) and 12 (38);
 *   - volatile empty fences, all emitting nothing: on `m` before the row-0
 *     copy (2; keeps its lq below the %lo add), untied after the row-0 copy
 *     (3), and taking `src` as an input after rows 1 and 2 (4 each) — each
 *     holds the next row's address setup below the previous sq;
 *   - row 1's and row 2's destinations pinned to $5 and $6 (RULING #8598;
 *     14 each): the ROM gives each row's destination its own register, and
 *     no pin-free spelling measured (shared or distinct locals, operand and
 *     statement order) reproduced that without moving `m` off $3. A $3 pin
 *     on `m` measured dead weight and is not here;
 *   - the +0x0 halfword is cleared before kind in the source (2): cc1 issues
 *     the last of the two stores first.
 * GUARD: on EE this C is compiled alone by SN 2.95.3 v1.36 -fopt-stack
 * (tools/ee/s136os_functions.txt) and spliced over the S136OS_SLOT line by
 * tools/ee/s136os_splice.sh; native compiles it, the fences emitting nothing
 * and EE_REG empty. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_ApplyCameraTransition)
S136OS_SLOT(ApplyCameraTransition);
#else
extern s32 func_00271140(Vec4 *out, void *state);
extern s32 func_002712E8(Vec4 *out, void *state);
extern Vec4 g_cameraMatrix;   /* 3-row rotation matrix at 0x1B54F0 */
void ApplyCameraTransition(Vec4 *out) {
    CameraTransitionState *t = &g_cameraTransitionState;
    s32 done;

    /* The matrix copy gates on the INTERPOLATOR'S RETURN value (not kind),
     * and the 4th row goes to g_cameraPos, NOT g_cameraMatrix[3]. */
    if (t->kind == 0) {
        done = func_00271140(out, t->blendA);
    } else {
        done = func_002712E8(out, &t->sphYaw);
    }
    if (done != 0) {
        Vec4 *m = &g_cameraMatrix;
        Vec4 *src, *pos;

        __asm__ __volatile__("" : "+r"(m));
        m[0] = out[0];
        __asm__ __volatile__("");
        {
            register Vec4 *row1 EE_REG("$5") = m + 1;
            src = out + 1;
            __asm__("" : "+r"(row1), "+r"(src));
            *row1 = *src;
        }
        __asm__ __volatile__("" : : "r"(src));
        {
            register Vec4 *row2 EE_REG("$6") = m + 2;
            src = out + 2;
            __asm__("" : "+r"(row2), "+r"(src));
            *row2 = *src;
        }
        __asm__ __volatile__("" : : "r"(src));
        pos = m - 0x23;      /* g_cameraPos, 0x230 below the matrix */
        src = out + 3;
        __asm__("" : "+r"(pos), "+r"(src));
        *pos = *src;
        t->state = 0;
        t->kind = 0;
    }
}
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
extern s32 TickCountdownTimer(void *timer);
extern f32 func_002845D8(f32 x);    /* wrap angle */
extern void func_002ADCE0(void *dst, f32 ang, void *src); /* build roll rotation about src */
extern void QuatToMatrix3(void *rot, void *mtx); /* rotation -> 3-row matrix */
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
    TickCountdownTimer(&sh->timer);
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
        /* 0.017453294f = 0x3C8EFA36, read from this body's own .s.
         * ⚠️ DO NOT "CORRECT" THIS TO A DERIVED pi/180. Deriving the constant
         * ACCURATELY reproduces the defect: pi/180 rounded to f32, at any
         * precision you care to write it, encodes 0x3C8EFA35 -- one ULP BELOW
         * what the ROM holds. The value below looks wrong to anyone who knows
         * deg->rad, and is the only correct spelling here. (The defective
         * spelling is named by its BITS above and deliberately not written as
         * a literal: a screen that does not strip comments would read it as
         * live code -- see f8b02259.) */
        f32 step = D_1A85B8 * 0.017453294f;              /* deg -> rad */
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
        func_002ADCE0(&ofs, ang, &g_cameraMatrix);
        QuatToMatrix3(&ofs, mtx);
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
 * +0x1A0 (vaddr 0x1B5320, the symbol g_heroCamMotion; before task #1474 the
 * split misattributed it as g_prevCamera+0xC). The
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
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern f32 func_002702D8(f32 cur, f32 target, f32 stiffness, f32 damping, f32 maxSpeed, f32 *vel);
/* (end of this body's declarations) */
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

    if (g_cameraState.activeCamera.p->type == 6 || g_nGameState != 0) {
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
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern char g_soundBankHandlesBlk[];
extern s32 g_cameraCallbackCount;
extern Vec4 g_cameraPos;
/* (end of this body's declarations) */
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

/* func_002721A8: start a fade-from-black. Forces the black fade level to
 * 1.0, sets the target to 0 and the per-tick rate to 1/duration
 * (g_cameraState +0x26C/+0x268), which UpdateScreenFadeBlack then eases.
 * duration: fade length in ticks (must be nonzero).
 * MATCHED on the s136os arm (task #1767), replacing its former MATCH_ guard
 * (an engine-2.96 candidate that read 65.83% under the tree's engine flags).
 * GUARD: as DebugPrintStub's.
 *
 * R5900_MTC1_PAD1 / R5900_MTC1_PAD1_PIN: SCHEDULING DEVICE (RULING #8435,
 * FACT #7918 / #8434), not a statement about the machine. The ROM has two
 * nops between `li.s $f0,1.0` (lui $at; mtc1 $at,$f0) + the g_cameraState
 * address and the `div.s` that reads $f0. Neither SN 1.36 cc1 nor GNU as
 * emits them (cc1 already gives every other instruction in the ROM's order);
 * which tool produced the ROM's pad is not settled (#7918 reads it as a
 * compiler-revision rule; ~160 of 335 ROM div.s sites carry it). Each macro
 * is one noreorder `nop` tied by operands: both
 * take `one` in-out, so they sit after the li.s and before the div.s; the
 * second also takes `st` in-out, so the address forms before the pad and the
 * stores through it issue after the div.s, as in the ROM. Two one-nop pads
 * rather than one two-nop pad: with a single pad tied to both values, cc1
 * issues the lui before the li.s (measured). EE arm only; nothing on native. */
#ifndef TARGET_NATIVE
#define R5900_MTC1_PAD1(f) \
    __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+f"(f))
#define R5900_MTC1_PAD1_PIN(f, p) \
    __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+f"(f), "+r"(p))
#else
#define R5900_MTC1_PAD1(f) ((void)0)
#define R5900_MTC1_PAD1_PIN(f, p) ((void)0)
#endif
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002721A8)
S136OS_SLOT(func_002721A8);
#else
void func_002721A8(f32 duration) {
    f32 one = 1.0f;
    CameraSysState *st = &g_cameraState;
    R5900_MTC1_PAD1(one);
    R5900_MTC1_PAD1_PIN(one, st);
    g_screenFadeBlack = one;
    st->fadeBlackTarget = 0.0f;
    st->fadeBlackRate = one / duration;
}
#endif

/* UpdateScreenFadeBlack: per-frame driver that eases g_screenFadeBlack toward
 * fadeBlackTarget at fadeBlackRate (no-op while the rate is 0). When the level
 * reaches the target a follow-up timer (TickCountdownTimer on the +0x270 field) may
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
extern s32 TickCountdownTimer(void *timer);
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
        if (TickCountdownTimer((char *)&g_cameraState + 0x270) == 1) {
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
 * the settle/re-arm (TickCountdownTimer on the +0x280 timer) and rate-clear mirror
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
extern s32 TickCountdownTimer();
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
        if (TickCountdownTimer(g_cameraState.fadeWhiteTimer) == 1) {
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

/* func_00272550: 0x10 bytes of dead inter-function debris (daddu $2,$5,$0 /
 * nop / addiu $sp,0x10 / nop) carved off the real entry func_00272560 in
 * task #472. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00272550);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00272560);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002725D8);

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273320);

/* AddScreenSpriteFx(size, angle, angleStep, owner, color, worldPos, texId,
 * mode): queue one screen-space sprite effect for DrawScreenSpriteFxQueue
 * (cap 6 per frame); nothing is queued in cutscene state (g_nGameState == 2).
 *   size, angle, angleStep: stored at +0x10, +0x1C and +0x28. The drawer reads
 *       them as sprite size, rotation and per-copy angle step (NOTE #5502), so
 *       the struct's `x`/`y` field names are historical, not screen x/y.
 *   owner:    the owner moby (+0x20); the drawer skips the entry once it dies.
 *   color:    RGBA; its alpha is capped at D_1A8630.
 *   worldPos: world anchor, copied with hasWorldPos = 1; NULL leaves the entry
 *             unanchored (hasWorldPos = 0), drawn at the screen centre.
 *   texId:    UI texture id.
 *   mode:     draw mode stored as given with drawFlags = 4 and angleStep; -1
 *             derives it: texIds 0xD and 0x1F..0x27 get mode 2 with a pi/2
 *             step (0x3FC90FDC, one ulp above the nearest float to pi/2) and
 *             drawFlags 0; any other texId leaves mode/angleStep/drawFlags as
 *             the slot's previous occupant wrote them (the ROM does too).
 * No return value.
 * MATCHED on the s136os arm (task #1986). Every item below was priced by
 * undoing it ALONE in a solo s136 compile (positional word differences
 * against the ROM's 64, relocations masked, li.s expanded as asm_unit.sh
 * does; the full body is 0):
 *   - the derive test is a `switch` with a `case 0x1F ... 0x27` range: cc1
 *     emits the ROM's ==0xD / <0xD / <0x28 / <0x1F decision tree for it,
 *     where the && chain folds into one subtract-and-compare (60 words, 29);
 *   - D_1A8630 is read after the early return, as the ROM loads it after the
 *     count test (read at its declaration: 15);
 *   - the explicit-mode branch stores mode, angleStep, drawFlags in that
 *     order (mode, drawFlags, angleStep: 3);
 *   - g_nGameState is a complete `s32` with the unit-level `.extern
 *     g_nGameState, 16` (the 1907F0/1CA080 model), so it loads as gas's
 *     `lui $3 / lw $3,%lo($3)` (the old incomplete array: 2);
 *   - DEVICE: the EMPTY volatile barrier between the anchor copy and the
 *     flag store (RULING #8483; emits nothing) keeps `li 1 / sh` below the
 *     `lq / sq`, so the NULL test fills its slot from the NULL arm as the
 *     ROM's `beqzl` + `sh $0` instead of hoisting the `li` (66 words, 38);
 *   - DEVICE: `one` pinned to $3 (RULING #8598): behind the barrier the
 *     constant otherwise reuses the copy's $2 where the ROM has $3 (2).
 * GUARD: on EE this C is compiled alone by SN 2.95.3 v1.36 -fopt-stack
 * (tools/ee/s136os_functions.txt) and spliced over the S136OS_SLOT line by
 * tools/ee/s136os_splice.sh; native compiles it with EE_REG empty and the
 * barrier emitting nothing. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_AddScreenSpriteFx)
S136OS_SLOT(AddScreenSpriteFx);
#else
void AddScreenSpriteFx(f32 size, f32 angle, f32 angleStep, void *owner,
                       u32 color, const u_long128 *worldPos, s32 texId,
                       s32 mode) {
    ScreenSpriteFx *fx;
    s32 alphaCap;

    if (g_nGameState == 2 || g_screenSpriteFxQueue.count >= 6) {
        return;
    }

    alphaCap = D_1A8630;
    fx = &g_screenSpriteFxQueue.entries[g_screenSpriteFxQueue.count];
    fx->owner = owner;
    fx->x = size;
    fx->y = angle;
    fx->color = color;
    fx->texId = texId;
    if ((s32)(color >> 24) > alphaCap) {
        fx->color = (color & 0xFFFFFF) | (alphaCap << 24);
    }

    if (worldPos != NULL) {
        fx->worldPos = *worldPos;
        __asm__ __volatile__("");
        {
            register s32 one EE_REG("$3") = 1;
            fx->hasWorldPos = one;
        }
    } else {
        fx->hasWorldPos = 0;
    }

    if (mode == -1) {
        switch (texId) {
        case 0xD:
        case 0x1F ... 0x27:
            fx->mode = 2;
            fx->angle = 1.57079649f;   /* 0x3FC90FDC */
            fx->drawFlags = 0;
            break;
        }
    } else {
        fx->mode = mode;
        fx->angle = angleStep;
        fx->drawFlags = 4;
    }

    g_screenSpriteFxQueue.count++;
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/16E980", func_002735A8);

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

/**
 * func_00273740 — SpawnChildMoby: generic child / debris / pickup spawner.
 *
 * Spawns a moby of `classId`, binds it under `templateMoby`, and copies the
 * template's lighting id, world position (`pos`), orientation (`rot`), relative
 * scale and parent linkage across; flags it active (+0x34). Picks a random launch
 * direction inside a cone whose half-angle is cos(launchAngle * k) and, for mode 1,
 * copies+resolves the template's animation state. Hands the part index + launch
 * velocity to func_00273988 (motion init), then installs the debris-physics update
 * fp (func_00311A80). Returns the spawned child (0 on spawn failure).
 *
 * Engine #else (functional; matching arm hits a packed-save wall).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273740);
#else
extern long SpawnMoby(u64 classId);
extern long func_002AC088(long moby);
extern void BindMobyToParent(long child, long parent);
extern void func_002AFA80(long moby, u32 color);
/* VU0 macro-op building a 0x30-byte (3-quadword) matrix at `dst` from the quad at
 * `src`: asm 0x283DA0 does `lqc2 vf1,0($5)` (SOURCE) then `sqc2 vf20/21/22` to
 * 0/0x10/0x20($4) (DEST). NOT a vector transform - the old "TransformVectorMicroVu0"
 * label is wrong and is not carried forward here. */
extern void func_00283DA0(void *dst, const void *src);
extern void func_002A1F20(long moby);
extern void GetRandomVectorInSphere(Vec4 *out, f32 a, f32 b);
extern f32  func_002835C0(f32 x);                     /* SqrtfVu0 */
extern void ResolveMobyAnimFramePtrs(long moby);
extern void UpdateMobyAnimation(long moby);
extern void func_00311A80(void);                      /* debris-physics update fp */
extern void func_00273988(f32 rate, void *moby, s32 mode, s32 index,
                          void *srcPos, void *dstPos, u8 flags);

long func_00273740(f32 launchAngle, f32 rate, long templateMoby, long mode,
                   u64 classId, s32 partIdx, void *pos, void *rot, void *vel,
                   u8 alpha) {
    long child = SpawnMoby(classId);
    long lightRec = func_002AC088(templateMoby);
    u32  lightColor = 0;

    if (lightRec != 0) {
        u8 *lr = (u8 *)lightRec;
        lightColor = ((u32)lr[6] << 16) | ((u32)lr[5] << 8) | 0x80000000 | (u32)lr[4];
    }
    if (child != 0) {
        u8  *c = (u8 *)child;
        u8  *t = (u8 *)templateMoby;
        u16  flags34;
        f32  radiusLo;
        Vec4 launchVec;

        BindMobyToParent(child, templateMoby);
        if (lightColor != 0) {
            func_002AFA80(child, lightColor);
        }
        *(Vec4 *)(c + 0x10) = *(Vec4 *)pos;   /* world position (16-byte lq/sq) */
        *(Vec4 *)(c + 0xf0) = *(Vec4 *)rot;   /* orientation (16-byte lq/sq) */
        func_00283DA0(c + 0xc0, c + 0xf0);   /* asm: $5 = c+0xF0, live from 0x273808 to the jal */
        if (templateMoby != 0) {
            *(f32 *)(c + 0x2c) =
                *(f32 *)(*(int *)(c + 0x24) + 0x24) *
                (*(f32 *)(t + 0x2c) / *(f32 *)(*(int *)(t + 0x24) + 0x24));
        }
        func_002A1F20(child);

        flags34 = (u16)(*(u16 *)(c + 0x34) & 0xffdf);
        *(u16 *)(c + 0x34) = flags34 | 0x100;
        if (templateMoby != 0 && (*(u16 *)(t + 0x34) & 0x800) != 0) {
            *(u16 *)(c + 0x34) = flags34 | 0x900;
        }
        *(s32 *)(c + 0x98) = 0;
        *(int *)(*(int *)(c + 0x68) + 0x54) = (int)templateMoby;

        {
            const union { u32 u; f32 f; } k = { 0x3918825C };
            radiusLo = func_00283B48(launchAngle * k.f);
        }
        GetRandomVectorInSphere(&launchVec, radiusLo, radiusLo);
        launchVec.w = func_002835C0(1.0f - radiusLo * radiusLo);

        if (mode == 1) {
            if (templateMoby != 0) {
                c[0x42] = t[0x43];
                c[0x43] = t[0x43];
            }
            ResolveMobyAnimFramePtrs(child);
            *(s32 *)(c + 0x48) = 0;
            *(s32 *)(c + 0x44) = 0;
            UpdateMobyAnimation(child);
        }

        func_00273988(rate, (void *)child, (s32)mode, partIdx, vel, &launchVec, alpha);

        if ((*(u16 *)(c + 0x7c) & 0x8000) != 0 && (*(u16 *)(c + 0x34) & 0x10) != 0) {
            *(u16 *)(c + 0x34) &= 0xffef;
        }
        *(void **)(c + 0x64) = (void *)func_00311A80;
    }
    return child;
}
#endif

/**
 * func_00273988 — initialise a projectile's motion state block.
 *
 * Marks the moby active (flags+0x34 |= 0x100), copies the source and target
 * positions into the state block (+0x00 / +0x10), stamps the fixed motion
 * parameters (drag/gravity/etc. at +0x38..+0x4c, with the +0x48 term scaled from
 * `rate`/3600), and records the mode + flags. Then, per `mode`:
 *   - 1: homing — resolve the target point for light `index` (func_002A04D8),
 *        aim the heading (sub-block +0x20) at it, project via func_002B0C40.
 *   - 2: replay — seed the trail from func_002A0678(index), then aim + project.
 *   - 0/3: ballistic — scale the moby's velocity into the heading (x1/1024) and
 *        stash the +0x2c speed term, then aim + project.
 *   - other: leave the block seeded but unaimed.
 *
 * Engine #else (functional; matching arm hits a packed-save wall).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273988);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void Vec4SubVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern void Vec4ScaleVu0(Vec4 *dst, f32 s, const Vec4 *src);
/* (end of this body's declarations) */
extern void func_002A04D8(void *moby, s32 idx, void *out);
extern s32  func_002A0678(void *moby, void *a, void *b, s32 arg);
extern void func_002B0C40(s32 ctx, void *out, void *a, void *b);

void func_00273988(f32 rate, void *moby, s32 mode, s32 index, void *srcPos,
                   void *dstPos, u8 flags) {
    u8 *m = (u8 *)moby;
    u8 *block = *(u8 **)(m + 0x68);
    u8 *sub = block + 0x20;
    u8  scratch[16];

    *(s32 *)(m + 0x98) = 0;
    *(u16 *)(m + 0x34) |= 0x100;

    *(Vec4 *)block          = *(Vec4 *)srcPos;   /* +0x00 source position (16-byte lq/sq) */
    *(Vec4 *)(block + 0x10) = *(Vec4 *)dstPos;   /* +0x10 target position (16-byte lq/sq) */

    *(f32 *)(block + 0x38) = 0.45f;                 /* 0x3EE66666 */
    *(f32 *)(block + 0x3c) = 0.033333335f;          /* 0x3D088889 */
    *(f32 *)(block + 0x40) = 0.5f;                  /* 0x3F000000 */
    *(u32 *)(block + 0x44) = 0x3C567752;
    *(f32 *)(block + 0x48) = rate * 0.00027777778f; /* 0x3991A2B4 = 1/3600 */
    *(u32 *)(block + 0x4c) = 0x3AE4C38A;
    *(u16 *)(block + 0x34) = 0xb4;
    block[0x5e] = flags;
    block[0x5d] = (u8)mode;
    block[0x58] = 0;
    block[0x5c] = 0;
    *(s32 *)(block + 0x30) = 0;

    if (mode == 1) {
        *(s16 *)(block + 0x36) = (s16)index;
        *(u16 *)(m + 0x7c) = (u16)((1 << (index & 0x1f)) | 0x8000);
        func_002A04D8(moby, index, scratch);
        Vec4SubVu0((Vec4 *)sub, (const Vec4 *)scratch, (const Vec4 *)(m + 0x10));
        func_002B0C40((s32)moby, sub, sub, (void *)0);
        return;
    }
    if (mode == 2) {
        *(s16 *)(block + 0x36) = (s16)index;
        *(u16 *)(m + 0x7c) = (u16)((1 << (index & 0x1f)) | 0x8000);
        block[0x58] = (u8)func_002A0678(moby, sub, *(void **)(block + 0x30), index);
    } else if (mode == 0 || mode == 3) {
        Vec4ScaleVu0((Vec4 *)sub, 0.0009765625f, (const Vec4 *)moby);   /* 0x3A800000 */
        *(f32 *)(block + 0x2c) = *(f32 *)(m + 0xc) * 0.0009765625f;
    } else {
        return;
    }

    Vec4SubVu0((Vec4 *)sub, (const Vec4 *)sub, (const Vec4 *)(m + 0x10));
    func_002B0C40((s32)moby, sub, sub, (void *)0);
}
#endif

/**
 * func_00273B80 — per-state projectile dispatcher + world-bounds despawn.
 *
 * Dispatches on the projectile state byte (block+0x5d, 6-way):
 *  - state 0/2: seed the trail buffer (func_00274CA0 for state 0, else replay via
 *    func_002A0678), point block+0x30 at it, run the collision handler
 *    (func_00273D20), then — every 4th frame, or once the lifetime timer
 *    (func_00283328 @block+0x34) elapses — force the result to 1; clear block+0x30.
 *  - state 1/3: run func_00273EA8, force result to 1 when the timer elapses.
 *  - state 4: func_00274128. state 5: func_00274130. state >= 6: no-op (result 0).
 *
 * Finally, despawn (return 1) if the projectile has left the [2, 1021] world box;
 * otherwise return the per-state result.
 *
 * Engine #else (functional; matching arm hits a jump-table + packed-save wall).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273B80);
#else
extern s32  func_00273D20(void *moby);   /* defined below (forward for the dispatch) */
extern s32  func_00273EA8(void *moby);
extern s32  func_00274128(void *moby);
extern s32  func_00274130(void *moby);
extern s32  func_00274CA0(void *moby, void *buf, s32 n);
extern s32  func_002A0678(void *moby, void *a, void *b, s32 arg);
extern s32  g_gameTime;

s32 func_00273B80(void *moby) {
    u8 *block = *(u8 **)((u8 *)moby + 0x68);
    s32 result = 0;
    u8  trailBuf[512];
    u8  scratch[16];

    switch (*(u8 *)(block + 0x5d)) {
    case 0:
    case 2:
        if (*(u8 *)(block + 0x5d) == 0) {
            *(u8 *)(block + 0x58) = (u8)func_00274CA0(moby, trailBuf, 0x20);
        } else {
            func_002A0678(moby, scratch, trailBuf, *(s16 *)(block + 0x36));
        }
        *(void **)(block + 0x30) = trailBuf;
        result = func_00273D20(moby);
        if (*(u8 *)(block + 0x5c) == 0 && (g_gameTime & 3) != 0) {
            *(s32 *)(block + 0x30) = 0;
        } else {
            if (func_00283328((s16 *)(block + 0x34)) != 0) {
                result = 1;
            }
            *(s32 *)(block + 0x30) = 0;
        }
        break;
    case 1:
    case 3:
        result = func_00273EA8(moby);
        if (func_00283328((s16 *)(block + 0x34)) != 0) {
            result = 1;
        }
        break;
    case 4:
        result = func_00274128(moby);
        break;
    case 5:
        result = func_00274130(moby);
        break;
    }

    {
        f32 x = *(f32 *)((u8 *)moby + 0x10);
        f32 y = *(f32 *)((u8 *)moby + 0x14);
        f32 z = *(f32 *)((u8 *)moby + 0x18);

        /* out-of-[2,1021]-box despawn — written as the exact out-of-bounds compares
         * (not >=/<=) to preserve the ROM's NaN behaviour (a NaN coord passes). */
        if (x < 2.0f || 1021.0f < x || y < 2.0f || 1021.0f < y ||
            z < 2.0f || 1021.0f < z) {
            return 1;
        }
    }
    return result;
}
#endif

/**
 * func_00273D20 — projectile collision-response handler.
 *
 * For a live projectile state (block+0x5c < 3; else no-op returning 1), advances
 * the sub-state (func_00274138 for state 0, else func_002742F8), then queries the
 * surface hit (func_00274510). On a hit it normalises the collision normal
 * (g_collHitNormal), builds a tangent frame (cross with the moby's forward via
 * func_00274A98 + a pushout basis via func_002B0E40), and runs the response
 * (func_00274BB8). If the response did not block (0) it applies the ricochet
 * (func_00274778) and, for state 0, plays the impact sound; if it blocked, it just
 * plays the sound. Any spawned sound emitter is anchored at block+0x20. Returns 0
 * once processed, 1 when the state gate rejects.
 *
 * Engine #else (functional; the matching arm hits a packed-save wall). The
 * matching build uses the INCLUDE_ASM arm above.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273D20);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void Vec3RescaleToLenVu0(Vec4 *dst, f32 len, const Vec4 *src);
extern void Vec3CrossVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
/* (end of this body's declarations) */
extern void func_00274138(void *moby);
extern void func_002742F8(void *moby);
extern s32  func_00274510(void *moby);
extern void func_00274A98(void *moby, void *localOut, Vec4 *tangent);
extern void func_00274778(void *moby, s32 slot, Vec4 *basis);
extern s32  func_00274BB8(f32 a, f32 b, void *block, Vec4 *normal, Vec4 *basis);
extern void func_002B0E40(void *pos, Vec4 *outBasis, s32 flag);
extern s32  PlayMobySound(s32 soundId, s32 flags, void *moby);
extern void SetSoundEmitterOffset(s32 handle, void *offset);
extern Vec4 g_collHitNormal;

s32 func_00273D20(void *moby) {
    u8 *block = *(u8 **)((u8 *)moby + 0x68);

    if (*(u8 *)(block + 0x5c) >= 3) {
        return 1;
    }
    if (*(u8 *)(block + 0x5c) == 0) {
        func_00274138(moby);
    } else {
        func_002742F8(moby);
    }

    {
        s32 slot = func_00274510(moby);

        if (slot != -1) {
            Vec4 tangent;
            Vec4 basis;
            s32  soundHandle = -1;
            s32  blocked;
            u8   soundId;

            Vec3RescaleToLenVu0(&g_collHitNormal, 1.0f, &g_collHitNormal);
            Vec3CrossVu0(&tangent, &g_collHitNormal, (const Vec4 *)block);
            func_00274A98(moby, block + 0x10, &tangent);
            func_002B0E40((u8 *)moby + 0x10, &basis, 1);
            blocked = func_00274BB8(*(f32 *)(block + 0x38), *(f32 *)(block + 0x3c),
                                    block, &g_collHitNormal, &basis);
            soundId = *(u8 *)(block + 0x5e);
            if (blocked == 0) {
                if (soundId != 0xff && *(u8 *)(block + 0x5c) == 0) {
                    soundHandle = PlayMobySound(soundId, 0, moby);
                }
                func_00274778(moby, slot, &basis);
            } else if (soundId != 0xff) {
                soundHandle = PlayMobySound(soundId, 0, moby);
            }
            if (soundHandle != -1) {
                SetSoundEmitterOffset(soundHandle, block + 0x20);
            }
        }
    }
    return 0;
}
#endif

/* func_00273EA8 — projectile bounce / reflection state handler (sibling of
 * func_00273D20).
 *
 * Advances the projectile toward the surface (func_00274218), then queries the
 * swept collision (func_002746D8). On no hit, returns 0 with no effect.
 *
 * On a hit it normalises the surface normal (g_collHitNormal to unit length),
 * builds a local tangent frame from the normal x the moby velocity
 * (func_00274A98) plus a push-out basis at moby+0x10 (func_002B0E40), and dots
 * that basis against the normal.
 *
 *  - basis.n > 0.7 (near head-on): REFLECT. Recompute v.n against the unit
 *    normal, form a blend length = v.n + v.n*0.5, but when |v.n*0.5| < 1/30
 *    (grazing) drop the extra half-term (length = v.n, blend half = 0). Scale
 *    the normal to that length and subtract it from the velocity (Vec4Sub) to
 *    reflect. When the half-blend collapsed to 0 (the grazing / near-normal
 *    case) additionally apply a random SPIN: rescale velocity by a damped
 *    length (func_002AB150 with rate 0x3AFEDCBB toward target 1/30), cross it
 *    with the normal to get a spin axis, and build a small rotation of angle
 *    -0.5*len(axis)/moby[0x2c] (func_002ADCE0) that is slerped 10% into the
 *    moby orientation at +0x10 (func_002841C0 / QuatSlerpNormalizedVu0).
 *    Finally, when |v.n| >= 1/30, play the bounce sound: PlayGlobalSound(8) if
 *    the per-projectile sound id (block+0x5e) is 0xff, else PlayMobySound(id);
 *    anchor the emitter at block+0x20 (SetSoundEmitterOffset).
 *  - basis.n <= 0.7 (glancing): SLIDE — project the velocity onto the normal
 *    (func_002839D8 / Vec3ProjectOntoNormalVu0).
 *
 * Always returns 0. `block` = *(moby+0x68) is the projectile physics block.
 *
 * Engine #else (functional coverage; the matching arm hits the packed-save
 * wall — eight callee-saves at 8-byte spacing + interleaved fp/qword math).
 * The matching build uses the INCLUDE_ASM arm above.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/16E980", func_00273EA8);
#else
extern void func_00274218(void *moby);
extern s32  func_002746D8(void *moby);
extern void func_00274A98(void *moby, void *localOut, Vec4 *tangent);
extern void func_002B0E40(void *pos, Vec4 *outBasis, s32 flag);
extern f32  func_002AB150(f32 target, f32 rate, f32 *level);
extern void func_002839D8(Vec4 *dst, const Vec4 *v, const Vec4 *normal); /* project onto normal */
extern void Vec3RescaleToLenVu0(Vec4 *dst, f32 len, const Vec4 *src);
extern void Vec3CrossVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern void Vec4SubVu0(Vec4 *dst, const Vec4 *a, const Vec4 *b);
extern f32  Vec3DotVu0(const Vec4 *a, const Vec4 *b);
extern f32  Vec3LengthVu0(const Vec4 *v);
extern f32  GetFloatAbs(f32 x);
extern s32  PlayMobySound(s32 soundId, s32 flags, void *moby);
extern s32  PlayGlobalSound(s32 soundId, s32 flags, void *moby);
extern void SetSoundEmitterOffset(s32 handle, void *offset);
extern Vec4 g_collHitNormal;

/* bit-exact FP constants (from the .s lui/ori immediates) */
static const union { u32 u; f32 f; } kOneOver30    = { 0x3D088889 }; /* 1/30      */
static const union { u32 u; f32 f; } kDotThreshold  = { 0x3F333333 }; /* 0.7       */
static const union { u32 u; f32 f; } kSpinRate       = { 0x3AFEDCBB }; /* ~0.00194  */
static const union { u32 u; f32 f; } kSpinSlerp       = { 0x3DCCCCCD }; /* 0.1       */

s32 func_00273EA8(void *moby) {
    u8 *block = *(u8 **)((u8 *)moby + 0x68);

    func_00274218(moby);
    if (func_002746D8(moby) != 0) {
        Vec4 tangent;    /* sp+0x00: normal x velocity                     */
        Vec4 pushBasis;  /* sp+0x10: push-out basis from func_002B0E40     */
        Vec4 reflScaled; /* sp+0x20: normal scaled to the reflection blend */
        Vec4 spinAxis;   /* sp+0x30: velocity x normal (spin rotation axis)*/
        f32  spinLevel;  /* sp+0x40: damped velocity length for the spin   */
        f32  basisDot;
        f32  vn;
        f32  half;
        f32  blendLen;

        Vec3RescaleToLenVu0(&g_collHitNormal, 1.0f, &g_collHitNormal);
        Vec3CrossVu0(&tangent, &g_collHitNormal, (const Vec4 *)block);
        func_00274A98(moby, block + 0x10, &tangent);
        func_002B0E40((u8 *)moby + 0x10, &pushBasis, 1);

        basisDot = Vec3DotVu0(&pushBasis, &g_collHitNormal);
        if (kDotThreshold.f < basisDot) {
            vn   = Vec3DotVu0((const Vec4 *)block, &g_collHitNormal);
            half = vn * 0.5f;
            if (GetFloatAbs(half) < kOneOver30.f) {
                half     = 0.0f;
                blendLen = vn + 0.0f;
            } else {
                blendLen = vn + half;
            }
            Vec3RescaleToLenVu0(&reflScaled, blendLen, &g_collHitNormal);
            Vec4SubVu0((Vec4 *)block, (const Vec4 *)block, &reflScaled);

            if (half == 0.0f) {
                spinLevel = Vec3LengthVu0((const Vec4 *)block);
                func_002AB150(kOneOver30.f, kSpinRate.f, &spinLevel);
                Vec3RescaleToLenVu0((Vec4 *)block, spinLevel, (const Vec4 *)block);
                Vec3CrossVu0(&spinAxis, (const Vec4 *)block, &g_collHitNormal);
                func_002ADCE0(&spinAxis,
                              (Vec3LengthVu0(&spinAxis) * -0.5f) /
                                  *(f32 *)(block + 0x2c),
                              &spinAxis);
                func_002841C0(block + 0x10, block + 0x10, &spinAxis, kSpinSlerp.f);
            }

            if (kOneOver30.f <= GetFloatAbs(vn)) {
                s32 handle;
                u8  soundId = *(u8 *)(block + 0x5e);

                if (soundId == 0xff) {
                    handle = PlayGlobalSound(8, 0, moby);
                } else {
                    handle = PlayMobySound(soundId, 0, moby);
                }
                if (handle != -1) {
                    SetSoundEmitterOffset(handle, block + 0x20);
                }
            }
        } else {
            func_002839D8((Vec4 *)block, (const Vec4 *)block, &g_collHitNormal);
        }
    }
    return 0;
}
#endif

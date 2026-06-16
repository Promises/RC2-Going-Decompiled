#include "common.h"

/* ------------------------------------------------------------------------- *
 *  TILE B: tfrag/tie geometry draw + Gadgetron vendor shop UI + the low-level
 *  GS/VIF/DMA packet builders that feed the renderer.
 *
 *  This unit was built by the later SN cc1 (gameplay/UI TU model: -O2 -G8
 *  -fno-gcse, sized externs so non-small globals use absolute %hi/%lo).
 * ------------------------------------------------------------------------- */

/* g_nGameStatePending / g_nGameState sit in the gp-addressable range, but the
 * original TU reaches them with absolute %hi/%lo (size-class >= 16 forces cc1
 * away from %gp_rel at -G8). */
__asm__(".extern g_nGameStatePending, 16");
__asm__(".extern g_nGameState, 16");
extern s32 g_nGameStatePending; /* 0x1A8BB8 pending top-level state (-2 = none) */
extern s32 g_nGameState;        /* 0x1A8BB0 current top-level game/screen state */

__asm__(".extern g_vramAllocCursor, 16");
extern u8 *g_vramAllocCursor;   /* 0x1A72D0 byte-addressed VRAM bump cursor */

void UploadTieTextures(u8 *vramCursor);
void AppendTexFlushDefaultTex0(void);
void RequestLevelExit(s32 destination, s32 commitSave);
void AppendGsRegPacket(s32 reg, u64 value);
void func_0011AEA0(s32 mode);
void CullAndBinTieInstances(void);
void AllocateTieTextureVram(void);
void EmitTieDrawPackets(void);
void EmitTieLodMorphPackets(void);
void CopyQwords(void *dst, const void *src, s32 qwordCount);

__asm__(".extern g_vramDynamicBase, 16");
__asm__(".extern g_frameDmaCursor, 16");
extern u8 *g_vramDynamicBase; /* 0x1A72D4 VRAM dynamic region base */
extern u8 *g_frameDmaCursor;  /* 0x1B2228 frame VIF1 chain write cursor */
extern u8 g_tieDrawTemplate[];/* 0x1ACAE0 32-qword tie draw-segment template */

/* One persistent tie-texture VRAM slot (stride 0x20). */
typedef struct VramSlot {
    u8  _pad00[0x14];
    s16 lruNext;  /* 0x14 LRU link, 0xFFFF = none */
    s16 refCount; /* 0x16 cleared on reset */
    u8  _pad18[0x7];
    u8  occupied; /* 0x1F 0xFF = free/sentinel */
} VramSlot;

__asm__(".extern g_vramSlotTableStart, 16");
__asm__(".extern g_vramSlotTableEnd, 16");
extern VramSlot *g_vramSlotTableStart; /* 0x1B2134 */
extern VramSlot *g_vramSlotTableEnd;   /* 0x1B2138 */
__asm__(".extern g_splashImageBuffer, 16");
__asm__(".extern g_tieVramLruHead, 16");
__asm__(".extern g_tieVramLruTail, 16");
extern void *g_pTieMatrixArray[];      /* 0x1B213C */
extern void *g_splashImageBuffer;      /* 0x1BAE54 */
extern s32  g_tieVramLruHead;          /* 0x1B2160 */
extern s32  g_tieVramLruTail;          /* 0x1B2164 */
extern s32  g_tieVramLruSize;          /* 0x1B2168 gp_rel */

/* One area-record entry (stride 0xA0) in the area table at 0x1393E0. */
typedef struct AreaRecord {
    u8 _pad[0x18];
    s16 statusFlag; /* 0x18 - negative when the area is not selectable */
    u8 _pad1a[0xA0 - 0x1A];
} AreaRecord;

/* The area table: a contiguous array of AreaRecord entries; the active index is
 * stored at byte 0x148 of the table base, and the "exit requested" flag at byte
 * 0x17C. */
extern AreaRecord g_areaTable[]; /* 0x1393E0 */

/* A two-word callback pair the cinematic-exit path installs. The first word is
 * reached with absolute %hi/%lo (size-class >= 16); the second sits in the gp
 * range and is reached with %gp_rel. */
__asm__(".extern g_exitCinematicCallback, 16");
extern void *g_exitCinematicCallback;     /* 0x1A8C80 */
extern void *g_exitCinematicCallbackArg;  /* 0x1A8C84 */
extern f32 g_exitFadeRate;                /* 0x1A8C7C gp_rel fade ramp rate */

__asm__(".extern g_screenFadeBlack, 16");
extern f32 g_screenFadeBlack; /* 0x1B1520 black screen-fade level 0..1 */
void FadeOutToBlackBlocking(s32 mode);

/* TODO(hle): needs PS2 graphics/IO HLE backend - closes a tfrag DMA draw segment (writes the 0x20000000 GIF tag into the chain). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", CloseTfragDrawSegment);

/* TODO(hle): needs PS2 graphics/IO HLE backend - patches the TEX0 GS register inside a prebuilt tfrag GIF packet. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", PatchTfragPacketTex0);

/* TODO(hle): needs PS2 graphics/IO HLE backend - builds the tfrag VIF1/GIF draw segment into the frame DMA chain. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", BuildTfragDrawSegment);

/* TODO(hle): needs PS2 graphics/IO HLE backend - VU0 frustum-culls tfrags and emits their draw packets. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", CullAndEmitTfrags);

/* TODO(hle): needs PS2 graphics/IO HLE backend - VU0 point-light cull/bin pass for tfrag lighting. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", CullTfragPointLights);

/* TODO(hle): needs PS2 graphics/IO HLE backend - builds the tfrag texture GIF upload packets. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", UploadTfragTextures);

/* TODO(hle): needs PS2 graphics/IO HLE backend - patches per-vertex light colours inside a tfrag GIF packet. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", PatchTfragVertexLighting);

/* TODO(hle): needs PS2 graphics/IO HLE backend - VU0 macro-mode (lqc2/vmul/vsub) tfrag bound/visibility test. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F19D0);

/* TODO(hle): needs PS2 graphics/IO HLE backend - tfrag LOD/morph selector over the VU0-culled tfrag array. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F1B58);

/* Flush the queued tie texture uploads: build the upload packets for whatever
 * slots the LRU assigned (current VRAM cursor), then append the TEXFLUSH +
 * default-TEX0 packet so the GS picks up the new textures. */
void FlushTieTextureUploads(void) {
    UploadTieTextures(g_vramAllocCursor);
    AppendTexFlushDefaultTex0();
    /* suppress cc1's sibling-call optimisation so the original jal+frame
     * teardown is reproduced rather than a tail j. */
    __asm__ __volatile__("");
}

/* TODO(hle): needs PS2 graphics/IO HLE backend - patches the TEX0 GS register inside a prebuilt tie GIF packet. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", PatchTiePacketTex0);

/* Build a full tie (instanced static geometry) draw segment into the frame's
 * VIF1 chain: emit the GS scissor/setup reg packet, reset the dynamic VRAM
 * cursor, cull+bin the visible instances, flush their texture uploads, allocate
 * persistent VRAM slots, emit the per-class draw + LOD-morph packets, then
 * splice the 32-qword draw-segment template and advance the chain cursor.
 *
 * NOT byte-matched: the original TU reaches g_vramAllocCursor with %gp_rel HERE
 * but with absolute %hi/%lo in FlushTieTextureUploads (the same symbol, two
 * different addressings within one TU - the reload artifact). We size it for
 * FlushTieTextureUploads (which matches), so this store comes out absolute
 * instead of gp_rel. Body is otherwise instruction-identical; kept as the
 * portable #else impl (see docs/PORTING.md). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", BuildTieDrawSegment);
#else
void BuildTieDrawSegment(void) {
    AppendGsRegPacket(0x47, 0x5180B);
    g_vramAllocCursor = g_vramDynamicBase;
    func_0011AEA0(0);
    CullAndBinTieInstances();
    FlushTieTextureUploads();
    func_0011AEA0(0);
    AllocateTieTextureVram();
    EmitTieDrawPackets();
    EmitTieLodMorphPackets();
    CopyQwords(g_frameDmaCursor, g_tieDrawTemplate, 0x20);
    g_frameDmaCursor += 0x20;
}
#endif

/* TODO(hle): needs PS2 graphics/IO HLE backend - tie draw-pipeline frame-stack sliver (spimdisasm fragment). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F1DE8);

/* Reset the persistent tie-texture VRAM slot table: clear the LRU bookkeeping
 * and mark every slot in [start, end) free (occupied = 0xFF, lruNext = none,
 * refCount = 0).
 *
 * NOT byte-matched: the original loop is software-pipelined with a `bnel`
 * branch-likely (the next slot's occupied byte is written in the loop's delay
 * slot); this cc1 lowers the do/while to a plain `bne`. Functionally identical;
 * kept as the portable #else impl (see docs/PORTING.md). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", ResetVramSlotTable);
#else
void ResetVramSlotTable(void) {
    VramSlot *slot;
    g_pTieMatrixArray[2] = g_splashImageBuffer;
    g_tieVramLruHead = 0;
    g_tieVramLruTail = 0;
    g_tieVramLruSize = 0;
    slot = g_vramSlotTableStart;
    if (slot != g_vramSlotTableEnd) {
        do {
            slot->occupied = 0xFF;
            slot->refCount = 0;
            slot->lruNext = -1;
            slot++;
        } while (slot != g_vramSlotTableEnd);
    }
}
#endif

/* TODO(hle): needs PS2 graphics/IO HLE backend - VU0 frustum-cull + per-class bin of tie instances. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", CullAndBinTieInstances);

/* TODO(hle): needs PS2 graphics/IO HLE backend - GS/VIF status busy-wait poll (hand-written, dead lui $31 in jr delay slot). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F2CB8);

/* TODO(hle): needs PS2 graphics/IO HLE backend - emits the per-class tie instance draw GIF packets. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", EmitTieDrawPackets);

/* TODO(hle): needs PS2 graphics/IO HLE backend - tie draw-packet builder helper (VIF/GIF). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F3468);

/* TODO(hle): needs PS2 graphics/IO HLE backend - tie draw-packet builder helper (VIF/GIF). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F35B0);

/* TODO(hle): needs PS2 graphics/IO HLE backend - GS/VIF status busy-wait poll (hand-written, dead lui $31 in jr delay slot). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F383C);

/* TODO(hle): needs PS2 graphics/IO HLE backend - GS/VIF status busy-wait poll (hand-written, dead lui $31 in jr delay slot). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F3864);

/* TODO(hle): needs PS2 graphics/IO HLE backend - emits the tie LOD cross-fade/morph GIF packets. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", EmitTieLodMorphPackets);

/* TODO(hle): needs PS2 graphics/IO HLE backend - GS/VIF status busy-wait poll (hand-written, dead lui $31 in jr delay slot). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F4104);

/* TODO(hle): needs PS2 graphics/IO HLE backend - GS/VIF status busy-wait poll (hand-written, dead lui $31 in jr delay slot). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F412C);

/* TODO(hle): needs PS2 graphics/IO HLE backend - builds the tie texture GIF upload packets at the VRAM cursor. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", UploadTieTextures);

/* TODO(hle): needs PS2 graphics/IO HLE backend - assigns persistent VRAM slots for the bound tie textures. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", AllocateTieTextureVram);

/* TODO(hle): needs PS2 graphics/IO HLE backend - GS/VIF status busy-wait poll (hand-written). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F4B50);

/* TODO(hle): needs PS2 graphics/IO HLE backend - VU0 macro-mode (pextlw/pminw/pmaxw) tie AABB reduction. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F4B78);

/* TODO(hle): needs PS2 graphics/IO HLE backend - VU0 macro-mode (pminw) tie vertex min-component reduction. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F4C98);

/* TODO(hle): needs PS2 graphics/IO HLE backend - patches per-vertex light colours inside a tie GIF packet. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", PatchTieVertexLighting);

/* TODO(hle): needs PS2 graphics/IO HLE backend - GS/VIF status busy-wait poll (hand-written, dead lui $31 in jr delay slot). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F5DD0);

/* TODO(hle): needs PS2 graphics/IO HLE backend - tie draw-pipeline VIF/GIF helper. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F5DF8);

/* TODO(hle): needs PS2 graphics/IO HLE backend - hand-written tie VRAM-slot LRU state-machine update (sh/lbu bit packing). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F5F70);

/* TODO(match): functional equivalent pending - multi-absolute-%hi global stores + gp-mix; clears the per-frame FX/draw-hook queues. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", ResetFxDrawQueues);

/* TODO(match): functional equivalent pending - bc1fl float-equal chain + gp/absolute mix; per-frame map/timer bookkeeping. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6110);

/* TODO(hle): needs PS2 graphics/IO HLE backend - tie draw-pipeline frame-stack sliver (spimdisasm fragment). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6218);

/* TODO(match): functional equivalent pending - cinematic-trigger dispatch; multi callee-save frame. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", PlayLevelCinematic);

/* TODO(match): functional equivalent pending - level (re)spawn/restore: camera proj + moby free loop + fade; many callee-saves + lq/sq. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6600);

/* TODO(hle): needs PS2 graphics/IO HLE backend - tie draw-pipeline frame-stack sliver (spimdisasm fragment). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6940);

/* TODO(match): functional equivalent pending - per-frame cinematic-camera matrix build: lq/sq + VU0 vec helpers. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6950);

/* Install the deferred exit-cinematic callback + its argument. */
void func_002F6B10(void *callback, void *arg) {
    g_exitCinematicCallback = callback;
    g_exitCinematicCallbackArg = arg;
}

void func_002F72D8(void);
void EnqueueCinematic(void *queue);
void StartCinematicFromQueue(void *queue);

extern u8 g_cinematicQueue[];      /* 0x1BACC0 pending-cinematic reel queue */
extern s32 g_cinematicExitPending; /* 0x1A7478 gp_rel exit-cinematic flag */
extern s16 g_exitSceneMode[];      /* 0x152C20 level-exit scene state words */
extern s32 g_exitSceneArmed;       /* 0x13955C cleared when the exit scene starts */

/* Arm the exit fade: set the fade ramp rate, and if the screen is not already
 * fully faded to black, run a synchronous fade-out. */
void func_002F6B20(void) {
    g_exitFadeRate = 1.2f;
    if (g_screenFadeBlack < 1.0f) {
        FadeOutToBlackBlocking(6);
        __asm__ __volatile__("");
    }
}

/* True while a transition into (or current presence in) game-state 2 is in
 * flight - i.e. the pending OR the committed top-level state is 2. */
s32 func_002F6B68(void) {
    return g_nGameStatePending == 2 || g_nGameState == 2;
}

/* TODO(match): functional equivalent pending - marks a cinematic id unlocked then enqueues+starts it; multi callee-save + gp/absolute mix. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6B98);

/* TODO(match): functional equivalent pending - stores the 5 cinematic-scene start params + tears down audio/fade; 5 callee-saves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6C78);

/* TODO(match): functional equivalent pending - secondary-voice cinematic audio start; multi callee-save + gp/absolute mix. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6D50);

/* TODO(match): functional equivalent pending - level-exit cinematic driver loop: streams reels, GIF uploads, state pops; huge frame + lq/sq. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6E10);

/* Mark the currently-selected area's exit flag (if the area exists and is
 * selectable), then request the level exit. */
void func_002F72D8(void) {
    AreaRecord *base = g_areaTable;
    s32 idx = *(s32 *)((u8 *)base + 0x148);
    if (idx != -1) {
        if (base[idx].statusFlag >= 0) {
            *(s32 *)((u8 *)base + 0x17C) = 1;
        }
    }
    RequestLevelExit(0, 0);
    __asm__ __volatile__("");
}

/* Queue + start the level-exit cinematic, then arm the deferred exit: install
 * func_002F72D8 as the post-cinematic callback and flag the exit scene.
 *
 * NOT byte-matched: 3 GPR saves (s0/s1/ra) hit the 8-byte-packed-save wall -
 * the original packs the save slots 8-byte (frame 0x20) while this cc1 emits
 * 16-byte spacing (frame 0x30). Body is otherwise instruction-identical; kept
 * as the portable #else impl (see docs/PORTING.md). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F7328);
#else
void func_002F7328(void) {
    g_cinematicExitPending = 1;
    EnqueueCinematic(g_cinematicQueue);
    StartCinematicFromQueue(g_cinematicQueue);
    func_002F6B10(func_002F72D8, 0);
    g_exitSceneMode[4] = 1;
    g_exitSceneArmed = 0;
}
#endif

/* TODO(match): functional equivalent pending - vendor item buy/upgrade price lookup; jump-table (jtbl_0026D1C0) switch. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", GetVendorItemPrice);

/* TODO(match): functional equivalent pending - rebuilds the buyable vendor item list from inventory; many callee-saves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", BuildVendorItemList);

/* TODO(match): functional equivalent pending - vendor menu sub-state reset; many callee-saves + gp/absolute mix. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F8038);

/* A small lookup table of 4-word records (key + 3 value words), terminated by a
 * zero key. func_002F81A0 finds the record whose key == 'key', then reports
 * whether record value-word 'col' (0..2) equals 'expected'. Used by the vendor
 * availability filter to test per-item flag columns.
 *
 * NOT byte-matched: the original is a hand-shaped frameless leaf whose search
 * loop is built entirely from `beql`/`bnel` branch-likely instructions (the
 * cmp result is consumed in the branch delay slot); this cc1 lowers the loop to
 * plain `beq`/`bne`, so the encodings differ. Functionally identical; kept as
 * the portable #else impl. */
extern u32 D_264E40[][4]; /* 0x264E40 record table (stride 0x10, <=0x38 rows) */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F81A0);
#else
s32 func_002F81A0(s32 expected, s32 key, s32 col) {
    s32 i = 0;
    if (D_264E40[0][0] == 0) {
        return 0;
    }
    if (key != (s32)D_264E40[0][0]) {
        u32 *row = &D_264E40[1][0];
        for (;;) {
            if (*row == 0) {
                return 0;
            }
            i++;
            if (i >= 0x38) {
                break;
            }
            if (key == (s32)*row) {
                break;
            }
            row += 4;
        }
    }
    if (D_264E40[i][0] == 0) {
        return 0;
    }
    return expected == (s32)D_264E40[i][col + 1];
}
#endif

/* TODO(match): functional equivalent pending - vendor item-availability scan over g_weaponTable/g_itemEquippedSlot; many callee-saves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F8228);

/* TODO(match): functional equivalent pending - vendor menu render/build (0xA3C); huge stack frame + many callee-saves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F85B8);

/* TODO(match): functional equivalent pending - opens the vendor shop UI; many callee-saves + lq/sq + gp/absolute mix. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", EnterVendorMenu);

/* TODO(match): functional equivalent pending - vendor preview-actor setup; lq/sq + VU0 vec helpers + many callee-saves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F95E8);

/* The vendor-shop UI state block (lives inside the big 0x21E000 UI arena at
 * +0x11C0). +0x3C is the active text-widget handle SetVendorCaption hands to the
 * formatter; +0x64 is a per-frame caption-dirty / scroll counter cleared here. */
typedef struct VendorUiState {
    u8  _pad00[0x3C];
    s32 captionWidget; /* 0x3C text widget the caption string is bound to */
    u8  _pad40[0x64 - 0x40];
    s32 captionCounter;/* 0x64 cleared whenever the caption is (re)set */
} VendorUiState;

extern VendorUiState g_vendorUi;     /* 0x21F1C0 */
extern u8 g_vendorCaptionFmt[];      /* 0x1AD338 caption format/template string */
void func_00115DA8(s32 widget, void *fmt, s32 captionId);

/* Bind a caption string to the vendor UI's caption text widget (formatting it
 * through the shared text formatter) and reset the caption refresh counter.
 *
 * NOT byte-matched: 2 GPR saves (s0/ra) hit the 8-byte-packed-save wall - the
 * original packs both slots 8-byte (frame 0x10: sd s0,0x0 / sd ra,0x8) while
 * this cc1 emits 16-byte spacing (frame 0x20). Body is otherwise
 * instruction-identical; kept as the portable #else impl. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", SetVendorCaption);
#else
void SetVendorCaption(s32 captionId) {
    func_00115DA8(g_vendorUi.captionWidget, g_vendorCaptionFmt, captionId);
    g_vendorUi.captionCounter = 0;
}
#endif

/* TODO(match): functional equivalent pending - vendor buy/confirm/upgrade state machine; large frame + jtbl. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", VendorPurchaseStateMachine);

/* TODO(match): functional equivalent pending - closes the vendor UI and restores camera/HUD/armor; many callee-saves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", ExitVendorMenu);

/* TODO(hle): needs PS2 graphics/IO HLE backend - vendor-input frame-stack sliver (spimdisasm fragment). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002FA238);

/* TODO(match): functional equivalent pending - vendor input handler (affordability + cursor nav); large frame + jtbl. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", UpdateVendorMenuInput);

void UpdateVendorMenuInput(void);

/* Thin per-frame trampoline to the vendor input handler (the vendor screen's
 * registered update callback). */
void func_002FA608(void) {
    UpdateVendorMenuInput();
    /* suppress cc1's sibling-call optimisation so the original jal + frame
     * teardown is reproduced rather than a tail j. */
    __asm__ __volatile__("");
}

/* The unit tail (0x2FA628..0x2FFC1F) is the spimdisasm c-mode tail-fusion blob:
 * functions reached only by j / data-ref (no jal) that spimdisasm cannot promote
 * to their own symbols, so they fuse into one INCLUDE_ASM. It is islanded as the
 * asm segment text/1FA5A8 to keep the objdiff count honest. */

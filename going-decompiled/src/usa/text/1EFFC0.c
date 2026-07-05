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

/* Tfrag texture-patch registry: a 0-terminated array of {packet array, count}
 * entries; each packet is 0x50 bytes with a texture index at +0x23. */
typedef struct TfragTexPatch {
    u32 *packets; /* +0x00 */
    s32  count;   /* +0x04 */
} TfragTexPatch;
extern TfragTexPatch g_tfragTexPatchList[];
/* Per-texture VRAM block table: two u16 TBP values per texture index. */
extern u16 g_tfragTexVramTable[];

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", PatchTfragPacketTex0);
#else
/**
 * Relocate the texture pointers in every registered tfrag GIF packet.
 *
 * Walks g_tfragTexPatchList (until a null packet array); for each of the entry's
 * `count` packets (0x50-byte stride) it looks up the packet's texture index
 * (byte +0x23) in g_tfragTexVramTable and, when non-zero, rewrites the low 14
 * bits (the GS TBP field) of the packet words at +0x00 and +0x30 with the two
 * VRAM block values.
 */
void PatchTfragPacketTex0(void) {
    TfragTexPatch *entry;

    for (entry = g_tfragTexPatchList; entry->packets != 0; entry++) {
        s32 count = entry->count;
        u8 *packet = (u8 *)entry->packets;
        s32 i;

        for (i = 0; i < count; i++, packet += 0x50) {
            s32 texIndex = packet[0x23];
            u16 tbp0 = g_tfragTexVramTable[texIndex * 2];
            u16 tbp1 = g_tfragTexVramTable[texIndex * 2 + 1];

            if (tbp0 != 0) {
                *(u32 *)(packet + 0x00) = (*(u32 *)(packet + 0x00) & 0xFFFFC000) | tbp0;
            }
            if (tbp1 != 0) {
                *(u32 *)(packet + 0x30) = (*(u32 *)(packet + 0x30) & 0xFFFFC000) | tbp1;
            }
        }
    }
}
#endif

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

extern u8 *g_pTfragArray;   /* 0x1B20F0 -> base of the 0x40-byte tfrag headers */

/** func_002F1B58 — advance the tfrag LOD/morph selector over a list of tfrag
 *  indices. `list`..`listEnd` is a u16 index array into g_pTfragArray (stride
 *  0x40). Each tfrag's +0x36 word packs four 4-bit LOD levels; for the first
 *  nibble position whose value equals the selector nibble (selector, <<4, <<8,
 *  <<12), the levels below it shift down one and the top nibble is forced to 0xF
 *  (the "retired" level). When every nibble has retired (word == 0xFFFF) the
 *  tfrag's +0x35 byte is set to 1 (fully culled). Tfrags matching no position are
 *  left untouched. Pure integer bookkeeping over the already-VU0-culled array. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F1B58);
#else
void func_002F1B58(u16 *list, u16 *listEnd, s32 selector) {
    u8 *base = g_pTfragArray;
    s32 sel0 = selector;
    s32 sel1 = selector << 4;
    s32 sel2 = selector << 8;
    s32 sel3 = selector << 12;

    while (list != listEnd) {
        u16 idx;
        u8 *tf;
        s32 flags, nv;

        idx = *list++;
        tf = base + (idx << 6);
        flags = *(u16 *)(tf + 0x36);

        if ((flags & 0xF) == sel0) {
            nv = (flags >> 4) | 0xF000;
        } else if ((flags & 0xF0) == sel1) {
            nv = (flags & 0xF) | ((flags >> 4) & 0xFF0) | 0xF000;
        } else if ((flags & 0xF00) == sel2) {
            nv = (flags & 0xFF) | ((flags >> 4) & 0xF00) | 0xF000;
        } else if ((flags & 0xF000) == sel3) {
            nv = flags | 0xF000;
        } else {
            continue;
        }
        *(u16 *)(tf + 0x36) = (u16)nv;
        if (nv == 0xFFFF) {
            *(u8 *)(tf + 0x35) = 1;
        }
    }
}
#endif

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

extern s32 g_tieVisibleClassList[]; /* negative-terminated list of visible TIE class indices */
extern void *g_tieClassQueue[];     /* per-class record pointers (record: +0xF count, +0x1C packets) */
extern u16 g_tieTexVramTable[];     /* two u16 VRAM block values per texture index */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", PatchTiePacketTex0);
#else
/**
 * Relocate the texture pointers in every visible TIE class's GIF packets (the
 * TIE analogue of PatchTfragPacketTex0).
 *
 * For each class index in g_tieVisibleClassList (until a negative terminator),
 * takes its record from g_tieClassQueue and, for each of the record's `count`
 * (+0xF) packets (+0x1C, 0x50-byte stride), looks up the packet's texture index
 * (byte +0x33) in g_tieTexVramTable and, when non-zero, rewrites the low 14 bits
 * (GS TBP field) of the packet words at +0x00 and +0x20 with the two VRAM blocks.
 */
void PatchTiePacketTex0(void) {
    s32 *visible;

    for (visible = g_tieVisibleClassList; *visible >= 0; visible++) {
        u8 *record = (u8 *)g_tieClassQueue[*visible];
        s32 count = *(u8 *)(record + 0xF);
        u8 *packet = *(u8 **)(record + 0x1C);
        s32 i;

        for (i = 0; i < count; i++, packet += 0x50) {
            s32 texIndex = packet[0x33];
            u16 tbp0 = g_tieTexVramTable[texIndex * 2];
            u16 tbp1 = g_tieTexVramTable[texIndex * 2 + 1];

            if (tbp0 != 0) {
                *(u32 *)(packet + 0x00) = (*(u32 *)(packet + 0x00) & 0xFFFFC000) | tbp0;
            }
            if (tbp1 != 0) {
                *(u32 *)(packet + 0x20) = (*(u32 *)(packet + 0x20) & 0xFFFFC000) | tbp1;
            }
        }
    }
}
#endif

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

/** func_002F5F70 — VRAM-slot variant of the tfrag LOD selector (func_002F1B58).
 *  Walks a u16 index list into g_vramSlotTableStart (stride 0x20); each slot's
 *  +0x1A word packs four 4-bit LOD levels. For the first nibble position matching
 *  the selector nibble (selector, <<4, <<8, <<12), the lower levels shift down and
 *  the top nibble is forced to 0xF; when the word retires to 0xFFFF, the slot's
 *  +0x1E byte gets bit 0 set (OR 1, preserving the other status bits — unlike
 *  func_002F1B58 which overwrites its mark byte). Slots matching no position are
 *  left untouched. Pure integer bit-packing over the VRAM slot table. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F5F70);
#else
void func_002F5F70(u16 *list, u16 *listEnd, s32 selector) {
    u8 *base = (u8 *)g_vramSlotTableStart;
    s32 sel0 = selector;
    s32 sel1 = selector << 4;
    s32 sel2 = selector << 8;
    s32 sel3 = selector << 12;

    while (list != listEnd) {
        u16 idx;
        u8 *slot;
        s32 flags, nv;

        idx = *list++;
        slot = base + (idx << 5);
        flags = *(u16 *)(slot + 0x1A);

        if ((flags & 0xF) == sel0) {
            nv = (flags >> 4) | 0xF000;
        } else if ((flags & 0xF0) == sel1) {
            nv = (flags & 0xF) | ((flags >> 4) & 0xFF0) | 0xF000;
        } else if ((flags & 0xF00) == sel2) {
            nv = (flags & 0xFF) | ((flags >> 4) & 0xF00) | 0xF000;
        } else if ((flags & 0xF000) == sel3) {
            nv = flags | 0xF000;
        } else {
            continue;
        }
        *(u16 *)(slot + 0x1A) = (u16)nv;
        if (nv == 0xFFFF) {
            *(u8 *)(slot + 0x1E) |= 1;
        }
    }
}
#endif

/* Per-frame reset of the FX / draw-hook queue counters. Zeroes all five hook
 * counts (pre/post/late particle + after-ties/after-shrubs draw) and the blob
 * shadow count. Then, only while the player is in Clank-solo mode
 * (g_bPlayerMode == 1) and not in a pad-suppressed / menu-overlay state
 * (D_138320 bit 0x10 clear, and neither the committed nor pending top-level
 * state is the in-game pause overlay 4), clears HUD CLUT slot halfwords
 * [+4,+6,+8,+0xA,+0xC].
 *
 * NOT byte-matched: the original TU reaches g_nGameState / g_nGameStatePending
 * with %gp_rel HERE, but func_002F6B68 (already matched) reaches the SAME two
 * symbols with absolute %hi/%lo - one symbol, two addressings in one TU (the
 * reload artifact). Sizing them to match func_002F6B68 forces this read
 * absolute, so the encodings here diverge. Body is otherwise
 * instruction-identical; kept as the portable #else impl. */
extern s32 g_fxHooksPreCount;            /* 0x1B1588 */
extern s32 g_fxHooksPostCount;           /* 0x1B158C */
extern s32 g_fxHooksLateCount;           /* 0x1B15B8 */
extern s32 g_drawHooksAfterTiesCount;    /* 0x1B1590 */
extern s32 g_drawHooksAfterShrubsCount;  /* 0x1B1594 */
extern s32 g_blobShadowCount;            /* 0x1B15BC gp_rel */
extern u8  g_bPlayerMode;                /* 0x18C0D4 0=Ratchet 1=Clank-solo 2=Giant */
extern u32 D_138320;                     /* 0x138320 pad/input state word */
extern s16 g_hudClutSlots[];             /* 0x1B1818 HUD CLUT slot table */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", ResetFxDrawQueues);
#else
void ResetFxDrawQueues(void) {
    g_fxHooksPreCount = 0;
    g_drawHooksAfterTiesCount = 0;
    g_drawHooksAfterShrubsCount = 0;
    g_fxHooksPostCount = 0;
    g_fxHooksLateCount = 0;
    g_blobShadowCount = 0;
    if (g_bPlayerMode == 1 && !(D_138320 & 0x10) &&
        g_nGameState != 4 && g_nGameStatePending != 4) {
        g_hudClutSlots[2] = 0;  /* +0x4 */
        g_hudClutSlots[3] = 0;  /* +0x6 */
        g_hudClutSlots[4] = 0;  /* +0x8 */
        g_hudClutSlots[5] = 0;  /* +0xA */
        g_hudClutSlots[6] = 0;  /* +0xC */
    }
}
#endif

/* func_002F6110 globals (declared for the TARGET_NATIVE #else only; all resolve
 * to splat data symbols in the matching build). */
extern u8  D_138180[];          /* object state block: int flag @+0x1A0, pos/vel floats @+0x100/+0x104/+0x108/+0x10C */
extern s32 D_1AD1A0;            /* consecutive-idle-frame counter */
extern s32 g_gameTime;          /* global frame/time tick */
extern u8  g_gsPixelOffsetY[];  /* block; per-frame counter field @+0x3C */
extern s32 D_1A8C70;            /* gate: only bump the per-player idle stat when set */
extern s32 g_playerProgress;    /* current player/save index */
extern u8  g_health[];          /* block; per-player idle-stat table @+0xEAC, stride 4 */
extern u8  g_deferredSegment2Tag[]; /* block; previous-frame counter mirror @+0xC0 */
extern s32 D_1A9E70;            /* set to -1 when the frame counter desyncs */

/** Per-frame idle/bookkeeping tick. Counts consecutive frames in which the
 *  tracked object is fully at rest — its int flag (D_138180+0x1A0) and all four
 *  position/velocity floats (+0x100/+0x104/+0x108/+0x10C) are zero — in
 *  D_1AD1A0, resetting to 0 the moment any is non-zero. Then advances g_gameTime
 *  and the +0x3C frame counter; while gated (D_1A8C70 set) and still within the
 *  first 0x384 idle frames, bumps the current player's idle stat; and flags
 *  D_1A9E70 = -1 if the deferred-segment counter mirror has fallen out of step
 *  with the previous frame's value. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6110);
#else
void func_002F6110(void) {
    if (*(s32 *)(D_138180 + 0x1A0) == 0 &&
        *(f32 *)(D_138180 + 0x108) == 0.0f &&
        *(f32 *)(D_138180 + 0x10C) == 0.0f &&
        *(f32 *)(D_138180 + 0x100) == 0.0f &&
        *(f32 *)(D_138180 + 0x104) == 0.0f) {
        D_1AD1A0 += 1;
    } else {
        D_1AD1A0 = 0;
    }

    g_gameTime += 1;
    *(s32 *)(g_gsPixelOffsetY + 0x3C) += 1;

    if (D_1A8C70 != 0 && D_1AD1A0 < 0x384) {
        ((s32 *)(g_health + 0xEAC))[g_playerProgress] += 1;
    }

    if (*(s32 *)(g_deferredSegment2Tag + 0xC0) != *(s32 *)(g_gsPixelOffsetY + 0x3C) - 1) {
        D_1A9E70 = -1;
    }
}
#endif

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
void EnqueueCinematic(void *queue, s32 reelId);
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
/* Unlock + start a cinematic by id. Negative ids are a no-op (just clears the
 * scene param slot). Otherwise: map the id to its asset-table index, mark it
 * watched in g_cinematicUnlockedFlags (only for ids < 0xB1), reset the
 * cinematic queue (func_00289560), then dispatch the scene via func_002F6C78
 * with parameters pulled from this cinematic's g_discToc directory entry. The
 * 5th param (flag) is 1 only for the special id 0xBF; the language param is
 * forced to 0 while a queued cinematic is already busy.
 *
 * NOT byte-matched: 4 GPR saves (s0-s2 + ra) hit the 8-byte-packed-save wall,
 * and g_cinematicSceneParams (g_tieVramLruSize + 0x24) is reached %gp_rel here
 * vs absolute elsewhere - the same-symbol reload artifact. Body is otherwise
 * instruction-identical; kept as the portable #else impl.
 *
 * ORACLE STATUS (this cinematic cluster - func_002F6B98 / func_002F6C78 /
 * func_002F6D50): OUT-OF-SCOPE, tester-classified. They drive a live-cinematic
 * scene-params block (g_cinematicSceneParams) that is not effect-diffable in the
 * tester's harness, and depend on disc-asset / queue state not seedable headless.
 * NOT a coverage gap - an explicit OOS class (live-cinematic-state dependency),
 * distinct from the standalone-cmp-oracle and tester-EE-effect-diff classes. */
s32 MapCinematicIdToIndex(s32 cinId);
void func_00289560(void *queue);
void func_002F6C78(s32 p0, s32 p1, s32 p2, s32 p3, s32 p4);
extern u32 g_cinematicUnlockedFlags[]; /* 0x139768 watched-cinematics bitfield */
extern u8  g_discToc[];                /* 0x14B540 master disc asset directory */
extern u8  g_currentLanguage;          /* 0x1A7BBC language index */
/* Cinematic-scene start param block at g_tieVramLruSize + 0x20 (0x1B2188). */
extern s32 g_cinematicSceneParams[];   /* [0]=+0x20 .. [4]=+0x30, [5]=+0x34 */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6B98);
#else
void func_002F6B98(s32 cinId) {
    s32 idx;
    s32 flag;
    s32 lang;
    u8 *entry;

    if (cinId < 0) {
        g_cinematicSceneParams[1] = 0; /* +0x24 */
        return;
    }
    idx = MapCinematicIdToIndex(cinId);
    flag = (cinId == 0xBF) ? 1 : 0;
    if ((u32)cinId < 0xB1) {
        /* asm: srl 2 / sll 2 -> word index cinId>>2 (NOT >>5; non-standard
         * overlapping bitfield), bit cinId & 0x1F (sllv). */
        g_cinematicUnlockedFlags[cinId >> 2] |= 1u << (cinId & 0x1F);
    }
    func_00289560(g_cinematicQueue);
    entry = g_discToc + idx * 0x10;
    lang = (*(s32 *)g_cinematicQueue != 0) ? 0 : g_currentLanguage;
    func_002F6C78(*(s32 *)(entry + 0x10) + *(s32 *)(g_discToc + 4),
                  *(s32 *)(entry + 0x14),
                  (s32)(entry + 8),
                  lang,
                  flag);
}
#endif

/* Begin a cinematic scene: set the listener flag bit, run the scene-prep
 * callbacks (func_00133710 / func_0011AEA0), tear down all sound emitters and
 * the dialog voice channels, latch the five caller-supplied scene-start params
 * (plus the saved CD read-mode field) into the cinematic-scene param block,
 * fade to black, and clear the active subtitle.
 *
 * NOT byte-matched: 5 GPR saves (s0-s4 + ra) hit the 8-byte-packed-save wall,
 * and the param block is reached with mixed absolute %hi/%lo and %gp_rel
 * (g_tieVramLruSize + 0x30) - the same-symbol reload artifact. Body is
 * otherwise instruction-identical; kept as the portable #else impl. */
void func_00133710(s32 a, s32 b, s32 c, s32 d, s32 e);
void StopAllSoundEmitters(void);
void ResetDialogVoiceChannels(void);
void func_002898E0(void);
extern s16 g_cdReadMode;              /* 0x1A63E8 sceCdRMode (+0x8 = datapattern hw) */
extern u8 g_listenerPosHistory[];    /* 0x188660 listener pos ring + flags */
/* Cinematic-scene start param block at g_tieVramLruSize + 0x20 (0x1B2188). */
extern s32 g_cinematicSceneParams[];  /* [0]=+0x20 .. [4]=+0x30, [5]=+0x34 */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6C78);
#else
void func_002F6C78(s32 p0, s32 p1, s32 p2, s32 p3, s32 p4) {
    g_listenerPosHistory[0x6B] |= 0x8;
    func_00133710(2, 0, 0, 0, 0);
    func_0011AEA0(0);
    StopAllSoundEmitters();
    ResetDialogVoiceChannels();
    g_cinematicSceneParams[0] = p0;                              /* +0x20 */
    g_cinematicSceneParams[5] = *(s16 *)((u8 *)&g_cdReadMode + 8); /* +0x34 */
    g_cinematicSceneParams[1] = p1;                              /* +0x24 */
    g_cinematicSceneParams[2] = p2;                              /* +0x28 */
    g_cinematicSceneParams[3] = p3;                              /* +0x2C */
    g_cinematicSceneParams[4] = p4;                              /* +0x30 */
    FadeOutToBlackBlocking(4);
    func_002898E0();
}
#endif

/* Cinematic-exit audio/fade teardown. Clears the exit-pending flag if it was 2,
 * fades the screen to black (blocking), restarts the secondary voice channel
 * with the saved sample id, near-mutes the dialog voices, sets the listener
 * flag bit, and resets the per-frame arenas. Finally, if a deferred
 * exit-cinematic callback is installed AND the cinematic queue is idle (count
 * == 0) AND no top-level transition into state 1/2 is pending, fires the
 * callback once and clears it.
 *
 * NOT byte-matched: 2 GPR saves (s0/ra) packed 8-byte (sd s0,0x0 / sd ra,0x8,
 * frame 0x10) hit the 8-byte-packed-save wall (this cc1 emits 16-byte spacing),
 * and g_exitCinematicCallbackArg is reached %gp_rel on the read but absolute
 * %hi/%lo on the clear (the same-symbol reload artifact). Body is otherwise
 * instruction-identical; kept as the portable #else impl. */
void StartSecondaryVoice(s32 sampleId, s32 chan, s32 volume);
void SetDialogVoiceVolumesMute(void);
void ResetFrameArenas(void);
extern s32 g_cinematicSceneParams[];     /* 0x1B2188 block; [5]=+0x34 voice id */
extern u8 g_listenerPosHistory[];        /* 0x188660 listener pos ring + flags */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6D50);
#else
void func_002F6D50(void) {
    void (*cb)(void *);
    if (g_cinematicExitPending == 2) {
        g_cinematicExitPending = 0;
    }
    FadeOutToBlackBlocking(4);
    StartSecondaryVoice(g_cinematicSceneParams[5], 1, 0x400);
    SetDialogVoiceVolumesMute();
    g_listenerPosHistory[0x6B] |= 0x10;
    ResetFrameArenas();
    cb = (void (*)(void *))g_exitCinematicCallback;
    if (cb != 0 && *(s32 *)(g_cinematicQueue + 0x38) == 0) {
        if (g_nGameStatePending != 2 && g_nGameStatePending != 1) {
            cb(g_exitCinematicCallbackArg);
            g_exitCinematicCallback = 0;
            g_exitCinematicCallbackArg = 0;
        }
    }
}
#endif

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
    EnqueueCinematic(g_cinematicQueue, 0x15);
    StartCinematicFromQueue(g_cinematicQueue);
    func_002F6B10(func_002F72D8, 0);
    g_exitSceneMode[4] = 1;
    g_exitSceneArmed = 0;
}
#endif

/* The vendor-shop UI state block (lives inside the big 0x21E000 UI arena at
 * +0x11C0). +0x3C is the active text-widget handle SetVendorCaption hands to the
 * formatter; +0x64 is a per-frame caption-dirty / scroll counter. func_002F8038
 * also builds parallel entry arrays at +0x140/+0x144/+0x148 (stride 0xC) with the
 * live entry count at +0x740. The currently-selected slot index lives at +0x80;
 * per-selected-slot fields sit at base +0x140 with a 0x18 stride. */
typedef struct VendorUiState {
    u8  _pad00[0x3C];
    s32 captionWidget; /* 0x3C text widget the caption string is bound to */
    u8  _pad40[0x64 - 0x40];
    s32 captionCounter;/* 0x64 cleared whenever the caption is (re)set */
} VendorUiState;
extern VendorUiState g_vendorUi;     /* 0x21F1C0 */

/* GetVendorItemPrice globals/callee (declared for the TARGET_NATIVE #else only). */
extern u8  g_itemEquippedSlot[0x38]; /* itemId -> active g_weaponTable variant slot (0x139568) */
extern u8  g_weaponTable[];          /* per-variant def/state table, stride 0xE0 (0x239B20); +0x80 = bolt price */
extern s32 GetWeaponStatsAtLevel(void *outStatBlock, s32 itemId, s32 level);

/** GetVendorItemPrice — return the bolt price of the currently-selected vendor
 *  slot. The selected index (g_vendorUi+0x80) picks a slot (base +0x140, stride
 *  0x18); the slot's +0x154 flag chooses the pricing path:
 *   - +0x154 != 0 (a weapon/upgrade slot): classify the slot's item id (+0x140)
 *     via the (id - 0xC) index into a 42-entry table — only classes {0,2,5,6,41}
 *     price at the current level (mode 1); every other in-range id and every
 *     out-of-range id prices at the next level (mode 2). Fetch that level's stat
 *     block with GetWeaponStatsAtLevel and return its +0x80 price if the lookup
 *     succeeds (else 0).
 *   - +0x154 == 0 && +0x150 == 0 (a plain item): return g_weaponTable's +0x80
 *     price for the item's currently-equipped variant slot.
 *  The +0x154 == 0 && +0x150 != 0 case (and any failed lookup) returns 0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", GetVendorItemPrice);
#else
s32 GetVendorItemPrice(void) {
    u8  statBlock[0xE0];
    s32 selIdx = *(s32 *)((char *)&g_vendorUi + 0x80);
    char *slot = (char *)&g_vendorUi + selIdx * 0x18;
    s32 price = 0;

    if (*(s32 *)(slot + 0x154) != 0) {
        s32 itemId = *(s32 *)(slot + 0x140);
        s32 index = itemId - 0xC;
        s32 level = 2; /* default: next-level price */
        if ((u32)index < 0x2A) {
            switch (index) {
            case 0: case 2: case 5: case 6: case 41:
                level = 1; /* these classes price at the current level */
                break;
            }
        }
        if (GetWeaponStatsAtLevel(statBlock, itemId, level) != 0) {
            price = *(s32 *)(statBlock + 0x80);
        }
    } else if (*(s32 *)(slot + 0x150) == 0) {
        s32 itemId = *(s32 *)(slot + 0x140);
        u8  equippedSlot = g_itemEquippedSlot[itemId];
        price = *(s32 *)(g_weaponTable + equippedSlot * 0xE0 + 0x80);
    }
    return price;
}
#endif

/* TODO(match): functional equivalent pending - rebuilds the buyable vendor item list from inventory; many callee-saves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", BuildVendorItemList);

/* func_002F8038 globals (declared for the TARGET_NATIVE #else only). */
extern s32 D_1AD240;   /* start-index seed: (D_1AD240 != 0) ? +1 : 1 */
extern s32 D_1AD238;   /* companion latch, cleared to 0 on reset */
extern s32 D_1AD2E8[]; /* 5 source value words copied into the built entries */
extern u8  D_1395B8[]; /* story-flag block; per-slot upgrade bitmask byte @+0x8D */
extern s32 IsVendorUpgradesUnlocked(void);

/** func_002F8038 — rebuild the vendor upgrade-slot entry list. Starting from
 *  slot (D_1AD240 ? D_1AD240+1 : 1), walk slots [start,5) and append an entry for
 *  each slot that is EITHER flagged in the per-slot upgrade bitmask
 *  (D_1395B8[0x8D] bit i, only for i<8) OR — when not flagged — permitted by
 *  IsVendorUpgradesUnlocked(). Each appended entry (stride 0xC at g_vendorUi+0x140)
 *  stores (i + 0xEA92) at +0x0, the slot's source word D_1AD2E8[i] at +0x4, and the
 *  slot index i at +0x8; the running count lives at +0x740. Resets the count and
 *  the D_1AD238 latch first. Note the flagged path SKIPS the unlock call. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F8038);
#else
void func_002F8038(void) {
    s32 srcVals[5];
    s32 i, k, count, emit;

    i = (D_1AD240 != 0) ? D_1AD240 + 1 : 1;
    *(s32 *)((char *)&g_vendorUi + 0x740) = 0;
    D_1AD238 = 0;

    for (k = 0; k < 5; k++) {
        srcVals[k] = D_1AD2E8[k];
    }

    for (; i < 5; i++) {
        emit = ((u32)i < 8 && ((D_1395B8[0x8D] >> i) & 1)) ? 1 : 0;
        if (!emit && IsVendorUpgradesUnlocked() == 0) {
            continue;
        }
        count = *(s32 *)((char *)&g_vendorUi + 0x740);
        *(s32 *)((char *)&g_vendorUi + 0x148 + count * 0xC) = i;
        *(s32 *)((char *)&g_vendorUi + 0x144 + count * 0xC) = srcVals[i];
        *(s32 *)((char *)&g_vendorUi + 0x140 + count * 0xC) = i + 0xEA92;
        *(s32 *)((char *)&g_vendorUi + 0x740) = count + 1;
    }
}
#endif

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

/* VendorUiState + g_vendorUi are declared above (moved up for func_002F8038). */
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

/* func_002FA624: 4-byte trailing-alignment nop after the unit's last function.
 * The compiler does not emit it (and splat drops the standalone pad word), so the
 * unit would be 0x4 SHORT — recover it as a raw-word filler to keep size==span
 * byte-exact (06333c5 precedent; NO re-split). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002FA624);

/* The unit tail (0x2FA628..0x2FFC1F) is the spimdisasm c-mode tail-fusion blob:
 * functions reached only by j / data-ref (no jal) that spimdisasm cannot promote
 * to their own symbols, so they fuse into one INCLUDE_ASM. It is islanded as the
 * asm segment text/1FA5A8 to keep the objdiff count honest. */

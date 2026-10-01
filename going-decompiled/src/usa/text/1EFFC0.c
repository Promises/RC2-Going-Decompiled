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

/* g_vramAllocCursor is reached TWO ways inside this one TU: %gp_rel in
 * CloseTfragDrawSegment and BuildTieDrawSegment, absolute %hi/%lo in
 * BuildTfragDrawSegment and FlushTieTextureUploads -- all four at 0x1A72D0.
 * One `.extern` size cannot serve both, so the size is chosen PER ARM, which
 * the MATCH_ guard already partitions: the sdk29 arm (which owns and matches
 * FlushTieTextureUploads) keeps the absolute size-16 class, the engine96 arm
 * (which owns BuildTieDrawSegment) takes the small-data class that yields
 * gp_rel. build.sh defines no MATCH_, so the IMAGE always takes the 16. */
#if defined(MATCH_BuildTieDrawSegment)
__asm__(".extern g_vramAllocCursor, 4");
#else
__asm__(".extern g_vramAllocCursor, 16");
#endif
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
/* NOTE: the third argument is a BYTE count despite the name (FACT #5479). */
void CopyQwords(void *dst, const void *src, s32 byteCount);

__asm__(".extern g_vramDynamicBase, 16");
__asm__(".extern g_frameDmaCursor, 16");
extern u8 *g_vramDynamicBase; /* 0x1A72D4 VRAM dynamic region base */
extern u8 *g_frameDmaCursor;  /* 0x1B2228 frame VIF1 chain write cursor */
extern u8 g_tieDrawTemplate[];/* 0x1ACAE0 tie draw-segment template, 0x20 bytes = 2 qwords (FACT #5479) */

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

/* One tfrag GIF packet (0x50 bytes). Two TEX0 low words carry a GS TBP field
 * in bits 0..13. */
typedef struct TfragPacket {
    u32 tex0;        /* +0x00 first TEX0 low word */
    u8  _pad04[0x1F];
    u8  texIndex;    /* +0x23 index into g_tfragTexVramTable */
    u8  _pad24[0xC];
    u32 tex0Second;  /* +0x30 second TEX0 low word */
    u8  _pad34[0x1C];
} TfragPacket;

/* Tfrag texture-patch registry: a 0-terminated array of {packet array, count}
 * entries. */
typedef struct TfragTexPatch {
    TfragPacket *packets; /* +0x00 */
    s32          count;   /* +0x04 */
} TfragTexPatch;
extern TfragTexPatch g_tfragTexPatchList[];
/* Per-texture VRAM block table: two TBP values per texture index. The ROM
 * reads them with `lh`, so they are declared signed. */
extern s16 g_tfragTexVramTable[];

/* GS TEX0 low word: everything above the 14-bit TBP field. */
#define TEX0_TBP_KEEP_MASK 0xFFFFC000

/**
 * PatchTfragPacketTex0 — point every registered tfrag packet at its texture's
 * current VRAM block.
 *
 * Walks g_tfragTexPatchList until an entry with a null packet array. For each
 * of the entry's `count` packets it looks up the packet's texture index in
 * g_tfragTexVramTable and, for each of the two TBP values that is non-zero,
 * replaces the TBP field (bits 0..13) of the matching TEX0 word.
 *
 * Byte-exact on the sdk29 arm (task #675). Two things are non-obvious:
 *  - The TBP pair is read through an `s16` pointer. The ROM loads each value
 *    with `lh` and re-reads the second one after the first store; u16 values
 *    in locals give `lhu`, both loads hoisted.
 *  - `next` is taken at the top of the outer loop. The ROM computes the next
 *    entry's address before the packet loop reuses the entry register.
 */
void PatchTfragPacketTex0(void) {
    TfragTexPatch *entry;
    TfragTexPatch *next;

    for (entry = g_tfragTexPatchList; entry->packets != 0; entry = next) {
        s32 count = entry->count;
        TfragPacket *packet;
        s32 i;

        next = entry + 1;
        packet = entry->packets;
        for (i = 0; i < count; i++, packet++) {
            s16 *tbp = &g_tfragTexVramTable[packet->texIndex * 2];

            if (tbp[0] != 0) {
                packet->tex0 = (packet->tex0 & TEX0_TBP_KEEP_MASK) | tbp[0];
            }
            if (tbp[1] != 0) {
                packet->tex0Second = (packet->tex0Second & TEX0_TBP_KEEP_MASK) | tbp[1];
            }
        }
    }
}

/* Opens/builds the tfrag draw segment into the frame's VIF1 chain. Snapshots
 * the frame DMA cursor as the segment head tag, advances the cursor by one
 * qword, resets the dynamic VRAM bump cursor, then builds a view matrix on the
 * stack (identity, translation row = -1024 * cameraPos with w=1) composed with
 * the camera view matrix (cameraPos-0x100), uploads it via two VIF unpacks,
 * culls+emits the tfrags, closes the segment, and clears the 0x3000-byte
 * tfrag relight list.
 *
 * NOT byte-matched (engine-2.96 TU): (1) the original packs the two saved regs
 * ($16/$31) into 8-byte stack slots (frame 0x50) whereas canonical ee-gcc 2.9
 * uses 16-byte save slots (frame 0x60) — the save-slot delta; (2) g_frameDmaCursor
 * is written %gp_rel here but read absolute (the same gp/abs-split reload artifact
 * documented on BuildTieDrawSegment). Body is otherwise instruction-equivalent;
 * kept as the portable #else impl. */
/* RESIDUAL CLASS (task #576): SPLIT-WITHIN-FUNCTION -- g_frameDmaCursor
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 73.33%, engine96 73.60%
 *   (better arm: engine96). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   the named symbol is reached BOTH ways inside this one function -- absolute
 *   %hi/%lo AND %gp_rel, same address. The split is POSITIONAL, not a size
 *   class (FACT #8058): every %gp_rel ref sits in a branch delay slot and every
 *   absolute ref outside one, 0 exceptions over all 122 compiled USA functions
 *   that split a symbol this way. No `.extern` size can express that; an
 *   assembler-side delay-slot rule could, and tools/ee has none yet.
 *   Unreachable today, but not proven a wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", BuildTfragDrawSegment);
#else
extern u8  *g_pTfragSegmentOpenTag;   /* 0x1B211C tfrag draw-segment head tag */
extern u16  g_tfragRelightList[];     /* 0x215E00 u16 tfrag ids, 0xFFFF-terminated */
extern u8   g_cameraPos[];            /* 0x1B52C0 camera world position vec4 */

void MatrixIdentityVu0(void *m);
void Vec4ScaleVu0(void *dst, f32 scale, void *src);
void MatrixMultiplyVu0(void *dst, void *a, void *b);
void AppendVifUnpackPacket(s32 vuAddr, void *data, s32 qwordCount);
void CullAndEmitTfrags(void);
void CloseTfragDrawSegment(void);
void func_00283558(void *dst, s32 fill, s32 len);

void BuildTfragDrawSegment(void) {
    f32 mtx[16]; /* 0x40-byte stack 4x4 matrix */

    g_pTfragSegmentOpenTag = g_frameDmaCursor;
    g_frameDmaCursor += 0x10;
    g_vramAllocCursor = g_vramDynamicBase;

    MatrixIdentityVu0(mtx);
    Vec4ScaleVu0(&mtx[12], -1024.0f, g_cameraPos);
    mtx[15] = 1.0f;
    MatrixMultiplyVu0(mtx, g_cameraPos - 0x100, mtx);
    AppendVifUnpackPacket(5, mtx, 4);
    AppendVifUnpackPacket(0x14D, mtx, 4);
    func_0011AEA0(0);
    CullAndEmitTfrags();
    CloseTfragDrawSegment();
    func_00283558(g_tfragRelightList, 0x3000, 0x40);
}
#endif

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
/* RESIDUAL CLASS (task #576): UNDIAGNOSED
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 22.00%, engine96 32.14%
 *   (better arm: engine96). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   SCREENED ONLY. Both arms were measured; the residual was not diagnosed to a
 *   mechanism. This is an open arm, not a wall -- do not read it as one. */
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

/* One TIE GIF packet (0x50 bytes). Two TEX0 low words carry a GS TBP field
 * in bits 0..13. */
typedef struct TiePacket {
    u32 tex0;        /* +0x00 first TEX0 low word */
    u8  _pad04[0x1C];
    u32 tex0Second;  /* +0x20 second TEX0 low word */
    u8  _pad24[0xF];
    u8  texIndex;    /* +0x33 index into g_tieTexVramTable */
    u8  _pad34[0x1C];
} TiePacket;

/* Per-class TIE draw record, reached through g_tieClassQueue. */
typedef struct TieClassRecord {
    u8         _pad00[0xF];
    u8         packetCount; /* +0x0F */
    u8         _pad10[0xC];
    TiePacket *packets;     /* +0x1C */
} TieClassRecord;

extern s32 g_tieVisibleClassList[];        /* negative-terminated list of visible TIE class indices */
extern TieClassRecord *g_tieClassQueue[];  /* per-class record pointers */
extern s16 g_tieTexVramTable[];            /* two TBP values per texture index, read with `lh` */

/**
 * PatchTiePacketTex0 — point every visible TIE class's packets at their
 * textures' current VRAM blocks. The TIE counterpart of PatchTfragPacketTex0.
 *
 * For each class index in g_tieVisibleClassList, up to a negative terminator,
 * takes the class record from g_tieClassQueue and walks its packets. Each
 * packet's texture index selects a TBP pair in g_tieTexVramTable; each
 * non-zero value replaces the TBP field (bits 0..13) of the matching TEX0 word.
 *
 * Byte-exact on the sdk29 arm (task #675). Non-obvious:
 *  - The loop bound is the record's packetCount re-read on every iteration,
 *    not cached. The packet stores may alias the record as far as the
 *    compiler knows, and the ROM reloads it (`lbu 0xF`) each time round.
 *  - The TBP pair is read through an `s16` pointer (`lh`, second value
 *    re-read after the first store), as in PatchTfragPacketTex0.
 *  - `next` is taken at the top of the outer loop, before the record lookup.
 */
void PatchTiePacketTex0(void) {
    s32 *visible;
    s32 *next;

    for (visible = g_tieVisibleClassList; *visible >= 0; visible = next) {
        TieClassRecord *record = g_tieClassQueue[*visible];
        TiePacket *packet;
        s32 i;

        next = visible + 1;
        packet = record->packets;
        for (i = 0; i < record->packetCount; i++, packet++) {
            s16 *tbp = &g_tieTexVramTable[packet->texIndex * 2];

            if (tbp[0] != 0) {
                packet->tex0 = (packet->tex0 & TEX0_TBP_KEEP_MASK) | tbp[0];
            }
            if (tbp[1] != 0) {
                packet->tex0Second = (packet->tex0Second & TEX0_TBP_KEEP_MASK) | tbp[1];
            }
        }
    }
}

/* Build a full tie (instanced static geometry) draw segment into the frame's
 * VIF1 chain: emit the GS scissor/setup reg packet, reset the dynamic VRAM
 * cursor, cull+bin the visible instances, flush their texture uploads, allocate
 * persistent VRAM slots, emit the per-class draw + LOD-morph packets, then
 * splice the 32-qword draw-segment template and advance the chain cursor.
 *
 * Takes and returns nothing; operates entirely on the frame DMA chain globals.
 * The template splice copies 0x20 BYTES (2 qwords, FACT #5479 -- CopyQwords'
 * third argument is a byte count, corroborated here by g_frameDmaCursor, a u8*,
 * advancing by the same 0x20), NOT 0x20 qwords.
 *
 * BYTE-EXACT on the engine96 arm (cc1 2.96-ee-001003 via MATCH_BuildTieDrawSegment,
 * task #576): unit objdiff 100.00% (objdiff_build.sh + unit_report.sh, clean
 * tree) and verify_match_unit.sh BYTE IDENTICAL TO ROM, 38/38 words, 20 relocs
 * resolved.
 *
 * The lever is the per-arm `.extern g_vramAllocCursor` size at the top of this
 * file, and it is the WHOLE residual -- the previous comment named the mechanism
 * correctly. That store is the original's only %gp_rel reference here; under the
 * absolute size-16 class it expands to a lui/sw PAIR, which cannot sit in the
 * jal delay slot and shifts the tail. Isolated A/B, one line apart, same arm:
 *   .extern g_vramAllocCursor, 16  -> 92.97%, verify_match_unit rc=1 DIFFERS
 *   .extern g_vramAllocCursor, 4   -> 100.00%, rc=0 BYTE IDENTICAL
 * Sizing it small for the WHOLE TU is not available: FlushTieTextureUploads
 * reaches the same address absolutely and is already byte-exact on sdk29, so a
 * TU-wide flip would trade one match for another. Splitting the size by arm
 * costs nothing because the guard already routes each function to one arm, and
 * build.sh defines no MATCH_, so the shipped image is untouched either way. */
#if defined(MATCH_BuildTieDrawSegment) || defined(TARGET_NATIVE)
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
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", BuildTieDrawSegment);
#endif

/* TODO(hle): needs PS2 graphics/IO HLE backend - tie draw-pipeline frame-stack sliver (spimdisasm fragment). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F1DE8);

/* Reset the persistent tie-texture VRAM slot table: clear the LRU bookkeeping
 * and mark every slot in [start, end) free (occupied = 0xFF, lruNext = none,
 * refCount = 0).
 *
 * NOT byte-matched: the original loop is software-pipelined with a `bnel`
 * branch-likely (the next slot's occupied byte is written in the loop's delay
 * slot); this cc1 lowers the do/while to a plain `bne`. Functionally identical;
 * kept as the portable #else impl (see docs/PORTING.md). */
/* RESIDUAL CLASS (task #576): UNDIAGNOSED
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 68.70%, engine96 55.70%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   SCREENED ONLY. Both arms were measured; the residual was not diagnosed to a
 *   mechanism. This is an open arm, not a wall -- do not read it as one. */
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
/* RESIDUAL CLASS (task #576): GPREL-MISMATCH-UNTESTED -- g_vramSlotTableStart
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 25.62%, engine96 32.00%
 *   (better arm: engine96). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   this arm reads the named symbol %gp_rel while the TU sizes it 16 (absolute).
 *   The per-arm sizing lever that closed BuildTieDrawSegment is therefore a
 *   CANDIDATE here and has NOT been tried -- the gap is far too wide for
 *   addressing to be the whole residual, but it was not measured either way. */
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

/* ROM-split externs (the 1CA080.c construct): the ROM reaches g_bPlayerMode and
 * D_138320 through a compiler-split lui/%lo pair with the high half in its own
 * register, which cc1 only emits for an object outside its -G8 small-data
 * class. Declarations only. Empty on TARGET_NATIVE: Mach-O rejects a section
 * name without a segment. */
#ifndef TARGET_NATIVE
#define ROM_SPLIT __attribute__((section(".data")))
#else
#define ROM_SPLIT
#endif
extern s32 g_fxHooksPreCount;            /* 0x1B1588 */
extern s32 g_fxHooksPostCount;           /* 0x1B158C */
extern s32 g_fxHooksLateCount;           /* 0x1B15B8 */
extern s32 g_drawHooksAfterTiesCount;    /* 0x1B1590 */
extern s32 g_drawHooksAfterShrubsCount;  /* 0x1B1594 */
extern s32 g_blobShadowCount;            /* 0x1B15BC gp_rel */
extern u8  g_bPlayerMode ROM_SPLIT;      /* 0x18C0D4 0=Ratchet 1=Clank-solo 2=Giant */
extern u32 D_138320 ROM_SPLIT;           /* 0x138320 pad/input state word */
extern s16 g_hudClutSlots[];             /* 0x1B1818 HUD CLUT slot table */

/* Addressing, per symbol, as the ROM has it (FACT #7697):
 *  - the five hook counters are absolute (lui $at + store). ResetFxDrawQueues
 *    is their only reader in this TU, so they are sized 16 file-wide.
 *  - g_nGameState / g_nGameStatePending are %gp_rel here, but the file-scope
 *    `.extern ..., 16` above keeps func_002F6B68's accesses absolute. These two
 *    reads therefore go through ASSEMBLER aliases sized 4, equated BEFORE use
 *    (the g_mobyTableBaseGp construct in 188858.c): gas resolves the equate at
 *    the use and addresses it off $gp. The relocation names the real symbol.
 *  - the five g_hudClutSlots halfwords each get their own `lui $at` + `sh`, so
 *    the original named five objects rather than indexing an array (an array
 *    hoists one base register). They go through five aliases sized 16 whose
 *    equates are emitted AFTER the function: gas cannot resolve an equate that
 *    is not yet defined, so it takes the alias's own 16 and expands absolute.
 *    Equated before use instead, gas resolves g_hudClutSlots+N and picks
 *    %gp_rel for N < 8 (+4/+6/+8 measured gp_rel, +0xA/+0xC absolute).
 *    The C type is s16 so cc1 keeps the one-instruction store macro. The
 *    relocations name g_hudClutSlots+N, as the ROM's do, so no symbol_addrs
 *    entry and no re-split of 0x1B181C..0x1B1825 is needed. */
#ifndef TARGET_NATIVE
__asm__(".extern g_fxHooksPreCount, 16");
__asm__(".extern g_fxHooksPostCount, 16");
__asm__(".extern g_fxHooksLateCount, 16");
__asm__(".extern g_drawHooksAfterTiesCount, 16");
__asm__(".extern g_drawHooksAfterShrubsCount, 16");
__asm__(".extern g_nGameStateGp, 4\n\tg_nGameStateGp = g_nGameState");
__asm__(".extern g_nGameStatePendingGp, 4\n\tg_nGameStatePendingGp = g_nGameStatePending");
__asm__(".extern g_hudClutSlot4Abs, 16");
__asm__(".extern g_hudClutSlot6Abs, 16");
__asm__(".extern g_hudClutSlot8Abs, 16");
__asm__(".extern g_hudClutSlotAAbs, 16");
__asm__(".extern g_hudClutSlotCAbs, 16");
extern s32 g_nGameStateGp;
extern s32 g_nGameStatePendingGp;
extern s16 g_hudClutSlot4Abs;
extern s16 g_hudClutSlot6Abs;
extern s16 g_hudClutSlot8Abs;
extern s16 g_hudClutSlotAAbs;
extern s16 g_hudClutSlotCAbs;
#else
#define g_nGameStateGp g_nGameState
#define g_nGameStatePendingGp g_nGameStatePending
#define g_hudClutSlot4Abs (g_hudClutSlots[2])
#define g_hudClutSlot6Abs (g_hudClutSlots[3])
#define g_hudClutSlot8Abs (g_hudClutSlots[4])
#define g_hudClutSlotAAbs (g_hudClutSlots[5])
#define g_hudClutSlotCAbs (g_hudClutSlots[6])
#endif

/*
 * ResetFxDrawQueues - per-frame reset of the FX / draw-hook queue counters.
 *
 * Zeroes all five hook counts (pre/post/late particle + after-ties/after-shrubs
 * draw) and the blob shadow count. Then, only while the player is in Clank-solo
 * mode (g_bPlayerMode == 1) and not in a pad-suppressed / menu-overlay state
 * (D_138320 bit 0x10 clear, and neither the committed nor the pending top-level
 * state is the in-game pause overlay 4), clears the HUD CLUT slot halfwords at
 * g_hudClutSlots +0x4, +0x6, +0x8, +0xA and +0xC.
 *
 * Takes and returns nothing.
 *
 * BYTE-EXACT on the sdk29 arm (cc1 2.9-ee-991111, -O2 -G8 -fno-gcse), task #865.
 * Codegen notes: g_bPlayerMode is read into a local BEFORE the counter stores,
 * which gives the ROM's first block (lui, store, lbu $4, li $3,1). cc1 2.96
 * (engine96 arm) issues the li in the first cycle instead and does not match.
 * The addressing is carried by the declarations above.
 */
void ResetFxDrawQueues(void) {
    u8 mode = g_bPlayerMode;

    g_fxHooksPreCount = 0;
    g_drawHooksAfterTiesCount = 0;
    g_drawHooksAfterShrubsCount = 0;
    g_fxHooksPostCount = 0;
    g_fxHooksLateCount = 0;
    g_blobShadowCount = 0;
    if (mode == 1 && !(D_138320 & 0x10) &&
        g_nGameStateGp != 4 && g_nGameStatePendingGp != 4) {
        g_hudClutSlot4Abs = 0;
        g_hudClutSlot6Abs = 0;
        g_hudClutSlot8Abs = 0;
        g_hudClutSlotAAbs = 0;
        g_hudClutSlotCAbs = 0;
    }
}

#ifndef TARGET_NATIVE
__asm__("g_hudClutSlot4Abs = g_hudClutSlots + 0x4");
__asm__("g_hudClutSlot6Abs = g_hudClutSlots + 0x6");
__asm__("g_hudClutSlot8Abs = g_hudClutSlots + 0x8");
__asm__("g_hudClutSlotAAbs = g_hudClutSlots + 0xA");
__asm__("g_hudClutSlotCAbs = g_hudClutSlots + 0xC");
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
/* RESIDUAL CLASS (task #576): SPLIT-WITHIN-FUNCTION -- D_1A7390
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 81.32%, engine96 72.23%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   the named symbol is reached BOTH ways inside this one function -- absolute
 *   %hi/%lo AND %gp_rel, same address. The split is POSITIONAL, not a size
 *   class (FACT #8058): every %gp_rel ref sits in a branch delay slot and every
 *   absolute ref outside one, 0 exceptions over all 122 compiled USA functions
 *   that split a symbol this way. No `.extern` size can express that; an
 *   assembler-side delay-slot rule could, and tools/ee has none yet.
 *   Unreachable today, but not proven a wall. */
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6218);

/* TODO(match): functional equivalent pending - cinematic-trigger dispatch; multi callee-save frame. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", PlayLevelCinematic);

/* TODO(match): functional equivalent pending - level (re)spawn/restore: camera proj + moby free loop + fade; many callee-saves + lq/sq. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", ExitCinematicSceneTeardown);

/* TODO(hle): needs PS2 graphics/IO HLE backend - tie draw-pipeline frame-stack sliver (spimdisasm fragment). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6940);

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
 * and g_cinematicFmvSize (0x1B218C) is reached %gp_rel here, in a delay slot,
 * vs absolute elsewhere - the delay-slot rule of FACT #8058. Body is otherwise
 * instruction-identical; kept as the portable #else impl.
 *
 * ORACLE STATUS (this cinematic cluster - func_002F6B98 / func_002F6C78 /
 * EnterCinematicBeginPlayback): OUT-OF-SCOPE, tester-classified. They drive a live-cinematic
 * scene-param scalars (g_cinematicSceneParams..g_cinematicResumeVoiceId) that are not effect-diffable in the
 * tester's harness, and depend on disc-asset / queue state not seedable headless.
 * NOT a coverage gap - an explicit OOS class (live-cinematic-state dependency),
 * distinct from the standalone-cmp-oracle and tester-EE-effect-diff classes. */
s32 MapCinematicIdToIndex(s32 cinId);
void func_00289560(void *queue);
void func_002F6C78(s32 p0, s32 p1, s32 p2, s32 p3, s32 p4);
extern u32 g_cinematicUnlockedFlags[]; /* 0x139768 watched-cinematics bitfield */
extern u8  g_discToc[];                /* 0x14B540 master disc asset directory */
extern u8  g_currentLanguage;          /* 0x1A7BBC language index */
/* The cinematic scene-start parameters: six SEPARATE scalar globals at
 * 0x1B2188..0x1B219C (g_tieVramLruSize + 0x20..0x34), not one array.
 * func_002F6C78 stores each through its own `lui $1` and one through %gp_rel,
 * which cc1 only emits for distinct small-data symbols - one array symbol
 * gets a shared CSE'd base (task #749 probe, task #790 re-split).
 * The ROM reaches every one of them both ways, and every %gp_rel ref sits in a
 * branch delay slot (FACT #8058). Our gas decides by size, one size per
 * symbol per TU, so all six are declared large (absolute) here. The one
 * delay-slot %gp_rel store in func_002F6C78 needs g_cinematicFmvFlag small
 * (size 4) on that function's engine96 arm only - see its comment. */
__asm__(".extern g_cinematicSceneParams, 16");
__asm__(".extern g_cinematicFmvSize, 16");
__asm__(".extern g_cinematicReelEntry, 16");
__asm__(".extern g_cinematicLanguage, 16");
__asm__(".extern g_cinematicFmvFlag, 16");
__asm__(".extern g_cinematicResumeVoiceId, 16");
extern s32 g_cinematicSceneParams;   /* 0x1B2188 FMV disc offset (PlayFmvMovie arg 1) */
extern s32 g_cinematicFmvSize;       /* 0x1B218C FMV size, 0 = nothing armed */
extern s32 g_cinematicReelEntry;     /* 0x1B2190 -> TOC entry +0x8, reel descriptor */
extern s32 g_cinematicLanguage;      /* 0x1B2194 language index */
extern s32 g_cinematicFmvFlag;       /* 0x1B2198 1 only for cinematic id 0xBF */
extern s32 g_cinematicResumeVoiceId; /* 0x1B219C secondary-voice sample id to restart */

/* RESIDUAL CLASS (task #576): UNDIAGNOSED
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 38.21%, engine96 19.39%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   SCREENED ONLY. Both arms were measured; the residual was not diagnosed to a
 *   mechanism. This is an open arm, not a wall -- do not read it as one. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6B98);
#else
void func_002F6B98(s32 cinId) {
    s32 idx;
    s32 flag;
    s32 lang;
    u8 *entry;

    if (cinId < 0) {
        g_cinematicFmvSize = 0;
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
 * the dialog voice channels, then latch the five caller-supplied scene-start
 * params plus the secondary voice's current sample id into the six cinematic
 * scalars, fade to black, and clear the active subtitle (func_002898E0).
 *
 * fmvOffset  FMV disc offset (TOC entry +0x10 plus the TOC base)
 * fmvSize    FMV size (TOC entry +0x14); 0 means nothing is armed
 * reelEntry  pointer to the cinematic's TOC entry +0x8 (reel descriptor)
 * language   language index
 * flag       1 only for cinematic id 0xBF
 *
 * Its one direct caller is func_002F6B98 (the only reference to this symbol
 * anywhere in asm/usa; an indirect jalr would not show there). */
void func_00133710(s32 a, s32 b, s32 c, s32 d, s32 e);
void StopAllSoundEmitters(void);
void ResetDialogVoiceChannels(void);
void func_002898E0(void);
/* 0x1A63F0: the secondary voice channel's current sample id, +0x4 into the
 * StartSecondaryVoice state block at 0x1A63EC (unnamed in symbol_addrs). The ROM
 * splits its `lh` into lui/%lo, which cc1 emits only for a non-small (array)
 * declaration. */
extern s16 D_1A63F0[];
extern u8 g_listenerPosHistory[];    /* 0x188660 listener pos ring + flags */

/* RESIDUAL CLASS (task #790): ONE PROLOGUE SCHEDULING SLOT, engine96 arm.
 *   97.04% on diff96.sh (per-function fuzzy, engine96 arm) for the body below,
 *   with this function under an engine96 per-function guard and
 *   g_cinematicFmvFlag declared `.extern ..., 4` on that arm only. Not the unit
 *   gate, not 100.00%, so this stays INCLUDE_ASM with no guard.
 *   Everything but the prologue is instruction-identical to the ROM: the six
 *   per-store lui/%lo and the delay-slot %gp_rel store come from the task
 *   #790 re-split (six real scalars), the jal+epilogue from the trailing
 *   barrier, the `li $4,4` above the `lh` from the barrier after
 *   ResetDialogVoiceChannels.
 *   Residual: the ROM issues `addiu $3,%lo(g_listenerPosHistory)` right after
 *   `sd $16`, and 2.96-001003's sched2 issues it after the five parameter moves
 *   (5 words shifted). Measured and not moved by: a local pointer (93.33 before
 *   the barriers), a "+r" pin on it (86.22), a volatile access (91.26), a
 *   barrier at the start of the body (93.33), a sized array (97.04), sched1 ON
 *   (96.30, diagnostic only). sched2 OFF gives 57.04, so sched2 is what
 *   places it.
 *   ARGUED WALL under 2.96-001003 sched2 (tasks #807/#810/#817, read from the
 *   -fsched-verbose=6 -dR dump at diff96.sh's flags): sched2 issues two insns
 *   per cycle (memory + alu), and the anti dependence of `move sN,aN` on
 *   `sd sN` costs 0 (the move goes `into ready`, not `into queue with cost`).
 *   Each move is issued in the same cycle as its sd, ahead of the addiu that
 *   is already on the ready list with priority 15 against the move's 10. So
 *   insertion order decides here, not priority, and the other ready alu insns
 *   do not help. That the freed insn is put at the top of the list without a
 *   re-sort is read from this behaviour, not from gcc source. The ROM's order
 *   needs each move to wait one cycle.
 *   2.9-991111 sched2, at this unit's flags (-O2 -G8 -fno-gcse, sched1 on),
 *   is NOT different in issue width or in cost: it also issues two insns per
 *   cycle and also frees each move `into ready` at cost 0. What differs is
 *   the pick. 2.9 never issues the move in the cycle that frees it. The
 *   second slot goes to an insn already on the list at the start of the
 *   cycle: the addiu (priority 23 against 20) in the first save cycle, then the
 *   move freed one cycle earlier. That is the ROM's lagged pattern
 *   (`sd; addiu; sd; move`). 2.9 still misses the ROM in the layout: a
 *   96-byte frame with 16-byte save slots, and saves in reverse order
 *   (s4 first). Its param->sN mapping is the ROM's (s0<-a0 .. s4<-a4) at
 *   these flags. Only at -G0 with sched1 on does the mapping differ too.
 *   Under 2.96, the six C spellings tried all give the same prologue:
 *   unsigned params, a u8 temp, local copies of the params and a local
 *   pointer (#807), and register-asm pins plus a u8 * read-modify-write
 *   (#810). "No C lever" is argued from the dependence graph. Nobody has
 *   enumerated the spellings. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F6C78);
#else
void func_002F6C78(s32 fmvOffset, s32 fmvSize, s32 reelEntry, s32 language,
                   s32 flag) {
    s32 voiceId;

    g_listenerPosHistory[0x6B] |= 0x8;
    func_00133710(2, 0, 0, 0, 0);
    func_0011AEA0(0);
    StopAllSoundEmitters();
    ResetDialogVoiceChannels();
    /* barrier: keeps reorg from pulling the first param store into this
     * call's delay slot (the ROM leaves a nop there), and lets sched2 hoist
     * the `li $4,4` above the `lh` as the ROM does */
    __asm__ __volatile__("");
    voiceId = D_1A63F0[0];
    g_cinematicSceneParams = fmvOffset;
    g_cinematicResumeVoiceId = voiceId;
    g_cinematicFmvSize = fmvSize;
    g_cinematicReelEntry = reelEntry;
    g_cinematicLanguage = language;
    g_cinematicFmvFlag = flag;
    FadeOutToBlackBlocking(4);
    func_002898E0();
    /* the ROM calls func_002898E0 with a jal and a full epilogue, not a
     * sibcall; the barrier keeps cc1 from turning it into a tail jump */
    __asm__ __volatile__("");
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
extern u8 g_listenerPosHistory[];        /* 0x188660 listener pos ring + flags */

/* RESIDUAL CLASS (task #576): SPLIT-WITHIN-FUNCTION -- g_exitCinematicCallbackArg
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 75.15%, engine96 70.45%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   the named symbol is reached BOTH ways inside this one function -- absolute
 *   %hi/%lo AND %gp_rel, same address. The split is POSITIONAL, not a size
 *   class (FACT #8058): every %gp_rel ref sits in a branch delay slot and every
 *   absolute ref outside one, 0 exceptions over all 122 compiled USA functions
 *   that split a symbol this way. No `.extern` size can express that; an
 *   assembler-side delay-slot rule could, and tools/ee has none yet.
 *   Unreachable today, but not proven a wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", EnterCinematicBeginPlayback);
#else
void EnterCinematicBeginPlayback(void) {
    void (*cb)(void *);
    if (g_cinematicExitPending == 2) {
        g_cinematicExitPending = 0;
    }
    FadeOutToBlackBlocking(4);
    StartSecondaryVoice(g_cinematicResumeVoiceId, 1, 0x400);
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

/* Cinematic / FMV playback driver for game-state 1 (= RunCinematicPlaybackFrame,
 * symbol_addrs 0x2F6E10 - kept as RunCinematicPlaybackFrame to match the INCLUDE_ASM symbol).
 * If no scene is armed (g_cinematicFmvSize == 0) it tears down the queue and
 * pops back to the previous state. Otherwise it:
 *   - blits the letterbox/backdrop UI texture (g_uiTextureCache record #2) to the
 *     GS: two GIF image-upload packets (func_00126288 = BuildGsImageUploadPacket, then
 *     KickGifImageUpload) plus a packed TEX0 register latched into the record,
 *   - stops all sound / dialog voices and pumps the save-load state machine until
 *     the active area's load settles,
 *   - loops the reel playlist / cinematic queue: for each entry it optionally streams
 *     the reel file (StartFileLoadPumpingVoice) and plays it to completion
 *     (PlayFmvMovie, which blocks and returns a skip flag), advancing either the
 *     attract playlist (g_cinematicQueue[0]/[4]) or the dequeued cinematic
 *     (DequeueCinematic) and refreshing the scene params from the disc TOC entry,
 *   - finishes on an empty queue / stop signal by kicking a final upload, resetting
 *     the VRAM slot table and frame arenas, and chaining to state 2
 *     (RequestGameStateChange(2,2,id,..)) when a cinematic was dequeued, else
 *     PopGameState.
 *
 * COVERAGE #else only (engine region, ee-gcc 2.96 - not byte-matchable): this body
 * is GS image-upload hardware plus live-FMV/cinematic state, so it is tester-OOS
 * (same class as the func_002F6B98/C78/D50 cluster above - not effect-diffable
 * headless). Faithful op-for-op transcription; the scene-state fields are referenced
 * via their real named globals (g_sceneFrame @0x1B87F4, g_sceneFmvStreamBase
 * @0x1B880C) rather than the g_cameraSlotActive+0x990 nearest-symbol base the asm
 * reaches them through (that base is a linker artifact, unsound in native ILP32). */
extern u8  g_memoryArenaTable[];       /* 0x1B21A0 memory-region base table */
extern u8  g_uiTextureCache[];         /* 0x1B96C0 UI texture records, 0x10 stride */
extern s32 g_uiTextureDataBase;        /* 0x1B1584 base of UI texture pixel data */
extern s32 g_sceneFrame;               /* 0x1B87F4 scene playback frame counter */
extern s32 g_sceneFmvStreamBase;       /* 0x1B880C FMV stream target base (UNCONFIRMED) */
extern s32 g_pendingDialogVoiceId;     /* 0x1A63CC queued dialog voice id (-1 = none) */
extern u8  D_1AD1F8[];                  /* 0x1AD1F8 caption/string blob */
void func_0029DAD0(void *str, s32 arg);
void func_002895E0(void *queue);       /* cinematic-queue helper */
void func_00126288(void *dest, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g); /* BuildGsImageUploadPacket */
void func_00126DC0(void (*isr)(void), s32 arg);
void OnVblankInterrupt(void);
void PopGameState(s32 a, s32 b);
void WaitFrameDmaFence(s32 mask);
void KickGifImageUpload(void *packet, s32 dataAddr);
void WaitGsPathsIdle(s32 a, s32 b);
s32  snd_CheckLoadInProgress(s32 a);
void SaveLoadStateMachine(void);
void StartFileLoadPumpingVoice(s32 dest, s32 src, s32 size);
s32  PlayFmvMovie(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
s32  DequeueCinematic(void *queue, s32 *outId, s32 *outType);
s32  RequestGameStateChange(s32 a, s32 b, s32 c, s32 d, s32 e);

/* RESIDUAL CLASS (task #576): UNDIAGNOSED
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 70.59%, engine96 58.87%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   SCREENED ONLY. Both arms were measured; the residual was not diagnosed to a
 *   mechanism. This is an open arm, not a wall -- do not read it as one. */
/* DLI lever MEASURED (task #1220; unit objdiff report, objdiff_build.sh, this #else body
 * promoted SOLO, sdk29 arm, colima-ee-x86; every other row in the unit unchanged). cc1 emits
 * `dli $7,0x7ed8400000000`; the ROM holds SN Ps2EeAs's expansion at 0x2F6F7C. A RULING #8549
 * allowlist row for that site moves this body 70.49% -> 51.90%: DOWN. The row was applied (its
 * words are in the object) and the score still fell. The dli is NOT the only residual, so no
 * row was landed and this stays INCLUDE_ASM. Residual class: non-dli codegen (undiagnosed). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", RunCinematicPlaybackFrame);
#else
void RunCinematicPlaybackFrame(void) {
    u8  gifPacket[0x60];    /* GS GIF image-upload packet scratch (frame [0,0x60)) */
    s32 arenaC, arena14, fmvUploadBase;
    s32 dequeuedCinId, dequeueResult;
    s32 skip, done;
    s32 idx, p2;
    u8 *tc;

    if (g_cinematicFmvSize == 0) {   /* nothing armed */
        func_0029DAD0(D_1AD1F8, -1);
        func_002895E0(g_cinematicQueue);
        PopGameState(0, 0);
        return;
    }

    arena14 = *(s32 *)(g_memoryArenaTable + 0x14);
    arenaC  = *(s32 *)(g_memoryArenaTable + 0xC);
    fmvUploadBase = arena14 + 0xE0000;

    WaitFrameDmaFence(1);

    /* --- Blit the letterbox/backdrop UI texture (record #2) to the GS. --- */
    tc = (u8 *)g_uiTextureCache + 0x20;
    {
        s32 w = 1 << tc[0xC];   /* 1 << log2Width  */
        s32 h = 1 << tc[0xD];   /* 1 << log2Height */
        func_00126288(gifPacket, 0x3F70, (s32)(w << 10) >> 16,
                    *(s16 *)(tc + 0xE), 0, 0, (s16)w, (s16)h);
        func_0011AEA0(0);
        KickGifImageUpload(gifPacket,
                           g_uiTextureDataBase + ((u16)*(u16 *)(tc + 8) << 4));
        WaitGsPathsIdle(0, 0);

        func_00126288(gifPacket, 0x3F6C, 1, 0, 0, 0, 0x10, 0x10);
        func_0011AEA0(0);
        KickGifImageUpload(gifPacket,
                           g_uiTextureDataBase + ((u16)*(u16 *)(tc + 0xA) << 4));
        WaitGsPathsIdle(0, 0);

        /* Pack the GS TEX0 register for this record and latch it into the record. */
        {
            u32 logW = tc[0xC];
            u32 logH = tc[0xD];
            s16 texE = *(s16 *)(tc + 0xE);
            u64 tex0 = (u64)((s32)(1 << tc[0xC]) >> 6) << 14;
            tex0 |= 0x3F70;
            tex0 |= (u64)(s64)texE << 20;
            tex0 |= (u64)logW << 26;
            tex0 |= (u64)logH << 30;
            tex0 |= (u64)0xFDB08000u << 19;
            tex0 |= (u64)0x8000u << 47;
            *(u64 *)tc = tex0;
            func_0011AEA0(0);
        }
    }

    StopAllSoundEmitters();
    ResetDialogVoiceChannels();
    snd_CheckLoadInProgress(0);

    /* --- Pump the save-load state machine until the active area settles. --- */
    g_pendingDialogVoiceId = -1;
    while (*(s32 *)((u8 *)g_areaTable + 0x15C) >= 3 ||
           *(s32 *)((u8 *)g_areaTable + 0x164) >= 0) {
        SaveLoadStateMachine();
    }

    /* --- Play the reel playlist / cinematic queue to completion. --- */
    dequeuedCinId = 0;
    dequeueResult = -1;
    done = 0;
    do {
        p2 = g_cinematicReelEntry;   /* current reel descriptor */
        if (p2 == 0 || *(s32 *)p2 == 0) {
            g_sceneFmvStreamBase = 0;
            g_sceneFrame = 0;
        } else {
            s32 sz = *(s32 *)(p2 + 4);
            /* round the byte size up to a 2 KiB unit (signed-shift bias) */
            s32 rounded = (-1 < sz + 0x7FF) ? (sz + 0x7FF) : (sz + 0xFFE);
            StartFileLoadPumpingVoice(fmvUploadBase,
                                      *(s32 *)p2 + *(s32 *)((u8 *)g_discToc + 4),
                                      rounded >> 11);
            g_sceneFrame = 0;
            g_sceneFmvStreamBase = fmvUploadBase;
        }

        skip = PlayFmvMovie(g_cinematicSceneParams, g_cinematicFmvSize,
                            (arenaC + 0x3F) & ~0x3F, (arena14 + 0x3F) & ~0x3F,
                            g_cinematicLanguage,
                            (g_cinematicFmvFlag != 0) ? 1 : 0);

        if (skip != 0) {
            *(s32 *)((u8 *)g_cinematicQueue + 0x3C) = 1;
            func_002895E0(g_cinematicQueue);
        }

        if (*(s32 *)g_cinematicQueue != 0) {
            /* attract playlist: wrap the reel index 1..0x18 and read the next id */
            s32 next = *(s32 *)g_cinematicQueue + 1;
            s32 base, nextId;
            *(s32 *)g_cinematicQueue = (next < 0x19) ? next : 1;
            base = *(s32 *)((u8 *)g_cinematicQueue + 4);
            nextId = *(s16 *)(base + *(s32 *)g_cinematicQueue * 2);
            idx = MapCinematicIdToIndex(nextId);
            g_cinematicSceneParams = *(s32 *)((u8 *)g_discToc + idx * 0x10 + 0x10)
                                        + *(s32 *)((u8 *)g_discToc + 4);
            g_cinematicFmvSize = *(s32 *)((u8 *)g_discToc + idx * 0x10 + 0x14);
        } else if (*(s32 *)((u8 *)g_cinematicQueue + 0x38) == 0) {
            done = 1;
        } else {
            DequeueCinematic(g_cinematicQueue, &dequeuedCinId, &dequeueResult);
            if (dequeueResult != 0) {
                if (dequeueResult == 1) {   /* movz: only a "stop" (==1) ends the loop */
                    done = 1;
                }
            } else {
                idx = MapCinematicIdToIndex(dequeuedCinId);
                g_cinematicSceneParams = *(s32 *)((u8 *)g_discToc + idx * 0x10 + 0x10)
                                            + *(s32 *)((u8 *)g_discToc + 4);
                g_cinematicFmvSize = *(s32 *)((u8 *)g_discToc + idx * 0x10 + 0x14);
            }
        }
    } while (done == 0);

    /* --- Teardown: final upload, reset VRAM/arenas, chain to the next state. --- */
    snd_CheckLoadInProgress(0);
    func_00126DC0(OnVblankInterrupt, 0);
    WaitGsPathsIdle(0, 0);
    KickGifImageUpload(gifPacket,
                       g_uiTextureDataBase + ((u16)*(u16 *)(tc + 0xA) << 4));
    WaitGsPathsIdle(0, 0);
    ResetVramSlotTable();
    *(u64 *)tc = 0;
    ResetFrameArenas();

    if (skip == 0 && dequeueResult == 1) {
        RequestGameStateChange(2, 2, dequeuedCinId, 0, 0);
    } else {
        PopGameState(0, 0);
    }
}
#endif

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
/* RESIDUAL CLASS (task #576): UNDIAGNOSED
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 77.27%, engine96 74.96%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   SCREENED ONLY. Both arms were measured; the residual was not diagnosed to a
 *   mechanism. This is an open arm, not a wall -- do not read it as one. */
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
/* RESIDUAL CLASS (task #576): UNDIAGNOSED
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 46.50%, engine96 34.16%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   SCREENED ONLY. Both arms were measured; the residual was not diagnosed to a
 *   mechanism. This is an open arm, not a wall -- do not read it as one. */
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

/* BuildVendorItemList — (re)build the Gadgetron vendor's buyable-item list from
 * current inventory + per-weapon availability. The list is the record array at
 * g_vendorItemList (== g_vendorUi + 0x140, stride 0x18), with the live entry
 * count at g_vendorUi + 0x740 (== g_nVendorItemCount). Each record holds six
 * words: +0x140 item id, +0x144 a kind/owned flag, +0x148 an icon/def word,
 * +0x14C a caption id, +0x150 spare, +0x154 a "finalized" flag.
 *
 * There are two modes, chosen by g_vendorUi + 0x744:
 *   - 0  (normal shop): first (when upgrades are unlocked) seed g_inventoryOrder
 *        with every upgrade-capable owned item, then walk that order building
 *        records, then a second sweep over all 0x38 items adds upgrade/owned
 *        entries the order walk missed.
 *   - !=0 (special ship-customization vendor): only the six fixed item ids
 *        {0xC,0xE,0x11,0x12,0x2C,0x35} are listed, then func_002FBC38 marks the
 *        owned slots.
 * Both modes finish with a compaction pass that drops records duplicating an
 * earlier one in fields +0x140/+0x144/+0x154.
 *
 * The "purchasable at this tier" test is inlined four times in the original as a
 * jump table on (itemId - 0xC); all four tables (jtbl_0026D270/320/3D0/480)
 * encode the same class set {0,2,5,6,41} — identical to GetVendorItemPrice — so
 * it is factored here into vendorTierPurchasable(). The original also copies the
 * eight driving tables to the stack first; nothing writes them in between, so
 * those reads are equivalent to indexing the globals directly.
 *
 * arg (always 0 from the callers): when nonzero, suppresses the not-yet-owned
 * "catalog" entries in the order walk.
 *
 * Engine-region (ee-gcc 2.96) — matching-walled; portable #else body. Field
 * offsets kept literal (partial VendorUiState). */
/* RESIDUAL CLASS (task #576): UNDIAGNOSED
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 30.48%, engine96 28.42%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   SCREENED ONLY. Both arms were measured; the residual was not diagnosed to a
 *   mechanism. This is an open arm, not a wall -- do not read it as one. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", BuildVendorItemList);
#else
extern u8   g_inventoryOrder[];   /* display order: low6 = itemId, bit0x40 = owned, 0xFF term */
extern u8   g_inventoryOwned[];   /* per-item have-flag */
extern s32  g_vendorItemList;     /* buyable-item record array (== g_vendorUi + 0x140) */
extern u8   D_262E00[];           /* per-item record table, stride 0x14; word0 = caption id */
extern s32  IsVendorUpgradesUnlocked(void);
extern void AddItemToInventoryOrder(s32 itemId);
extern s32  GetWeaponUpgradeLevel(s32 itemId);
extern void func_002FBC38(void);  /* marks owned special-vendor slots */

/* Is itemId purchasable given its current upgrade level? Mirrors the four inline
 * jump tables (and GetVendorItemPrice's classifier): fully-upgraded (level >= 2)
 * always yes; not-yet-started (level 0) always no; at level 1 only the item
 * classes {0,2,5,6,41} (indexed by itemId - 0xC) qualify. */
static s32 vendorTierPurchasable(s32 level, s32 itemId) {
    s32 idx;
    if (level >= 2) {
        return 1;
    }
    if (level != 1) {
        return 0;
    }
    idx = itemId - 0xC;
    if ((u32)idx < 0x2A) {
        switch (idx) {
        case 0: case 2: case 5: case 6: case 41:
            return 1;
        }
    }
    return 0;
}

void BuildVendorItemList(s32 arg) {
    char *ui   = (char *)&g_vendorUi;
    char *list = (char *)&g_vendorItemList;
    char *rec;
    s32   numOrder;
    s32   i, count;

    (void)list; /* records are addressed ui-relative; list kept for clarity */

    /* Seed g_inventoryOrder with upgrade-capable owned items (item 0xA skipped). */
    if (IsVendorUpgradesUnlocked() != 0) {
        for (i = 0; i < 0x38; i++) {
            if (i == 0xA) {
                continue;
            }
            if (*(s32 *)(g_weaponTable + g_itemEquippedSlot[i] * 0xE0) == 0) {
                continue;
            }
            AddItemToInventoryOrder(i);
        }
    }

    /* Count the leading valid order entries (stop at the 0xFF terminator, cap 0x20). */
    numOrder = 0;
    if (g_inventoryOrder[0] != 0xFF) {
        numOrder = 1;
        while (numOrder < 0x20 && g_inventoryOrder[numOrder] != 0xFF) {
            numOrder++;
        }
    }

    *(s32 *)(ui + 0x740) = 0; /* reset the live entry count */

    if (*(s32 *)(ui + 0x744) != 0) {
        /* -------- special ship-customization vendor -------- */
        for (i = 0; i < 0x38; i++) {
            u8 *def;
            u8  owned;
            if (i != 0xC && i != 0xE && i != 0x11 &&
                i != 0x12 && i != 0x2C && i != 0x35) {
                continue;
            }
            owned = g_inventoryOwned[i];
            def = g_weaponTable + g_itemEquippedSlot[i] * 0xE0;
            if (owned != 0 && *(u16 *)(def + 0x88) == 0) {
                continue;
            }
            count = *(s32 *)(ui + 0x740);
            rec = ui + count * 0x18;
            *(s32 *)(rec + 0x140) = i;
            *(s32 *)(rec + 0x144) = (owned != 0) ? 1 : 0;
            *(s32 *)(rec + 0x148) = *(s32 *)(def + 0x14);
            *(s32 *)(rec + 0x14C) = *(s32 *)(D_262E00 + i * 0x14);
            *(s32 *)(rec + 0x150) = 0;
            *(s32 *)(rec + 0x154) = 0;
            *(s32 *)(ui + 0x740) = count + 1;
        }
        func_002FBC38();
    } else {
        /* -------- normal shop: order walk -------- */
        s32 j;
        for (j = 0; j < numOrder; j++) {
            u8  order = g_inventoryOrder[j];
            u8 *def;
            if (order == 0xFF) {
                continue;
            }
            i = order & 0x3F;

            /* Upgrade-buyable weapon slot: owned, purchasable at both the live
             * level and the base-def level, not maxed (+0x6C <= 0), unlocked. */
            if (g_inventoryOwned[i] != 0) {
                def = g_weaponTable + g_itemEquippedSlot[i] * 0xE0;
                if (*(s32 *)def != 0 &&
                    vendorTierPurchasable(GetWeaponUpgradeLevel(i), i) != 0 &&
                    i != 0x10 &&
                    vendorTierPurchasable(def[0x4], i) == 0 &&
                    *(s32 *)(def + 0x6C) <= 0 &&
                    IsVendorUpgradesUnlocked() != 0) {
                    count = *(s32 *)(ui + 0x740);
                    rec = ui + count * 0x18;
                    *(s32 *)(rec + 0x140) = i;
                    *(s32 *)(rec + 0x144) = 0;
                    *(s32 *)(rec + 0x148) = *(s32 *)(def + 0x14);
                    *(s32 *)(rec + 0x14C) = 0xCDB;
                    *(s32 *)(rec + 0x150) = 0;
                    *(s32 *)(rec + 0x154) = 1;
                    *(s32 *)(ui + 0x740) = count + 1;
                }
            }

            if (order & 0x40) {
                /* Owned item with a live catalog def (+0x88): list it. */
                def = g_weaponTable + g_itemEquippedSlot[i] * 0xE0;
                if (*(u16 *)(def + 0x88) != 0) {
                    count = *(s32 *)(ui + 0x740);
                    rec = ui + count * 0x18;
                    *(s32 *)(rec + 0x140) = i;
                    *(s32 *)(rec + 0x144) = 1;
                    *(s32 *)(rec + 0x148) = *(s32 *)(def + 0x14);
                    *(s32 *)(rec + 0x14C) = 0xCDB;
                    *(s32 *)(rec + 0x150) = 0;
                    *(s32 *)(rec + 0x154) = 0;
                    *(s32 *)(ui + 0x740) = count + 1;
                }
            } else if (arg == 0 && g_inventoryOwned[i] == 0) {
                /* Not-yet-owned catalog entry. */
                count = *(s32 *)(ui + 0x740);
                rec = ui + count * 0x18;
                *(s32 *)(rec + 0x140) = i;
                *(s32 *)(rec + 0x144) = 0;
                *(s32 *)(rec + 0x14C) = 0xCDB;
                *(s32 *)(rec + 0x150) = 0;
                if (i == 9 || i == 0x49) {
                    *(s32 *)(rec + 0x148) = 0x259;
                } else {
                    def = g_weaponTable + g_itemEquippedSlot[i] * 0xE0;
                    *(s32 *)(rec + 0x148) = *(s32 *)(def + 0x14);
                }
                *(s32 *)(rec + 0x154) = 0;
                *(s32 *)(ui + 0x740) = count + 1;
            }
        }

        /* -------- normal shop: full sweep over all items -------- */
        for (i = 0; i < 0x38; i++) {
            u8 *def = g_weaponTable + g_itemEquippedSlot[i] * 0xE0;
            if (*(s32 *)def == 0 || g_inventoryOwned[i] == 0) {
                continue;
            }

            /* Upgrade-buyable (same test as the order walk). */
            if (vendorTierPurchasable(GetWeaponUpgradeLevel(i), i) != 0 && i != 0x10 &&
                vendorTierPurchasable(def[0x4], i) == 0 &&
                *(s32 *)(def + 0x6C) <= 0 &&
                IsVendorUpgradesUnlocked() != 0) {
                s32 k, skip = 0;
                count = *(s32 *)(ui + 0x740);
                if (count > 0) {
                    for (k = 0; k < count; k++) {
                        if (*(s32 *)(ui + k * 0x18 + 0x140) == i) {
                            break;
                        }
                    }
                    /* skip if already listed with the finalized flag set */
                    if (k < count && *(s32 *)(ui + k * 0x18 + 0x154) != 0) {
                        skip = 1;
                    }
                }
                if (skip) {
                    continue;
                }
                count = *(s32 *)(ui + 0x740);
                rec = ui + count * 0x18;
                *(s32 *)(rec + 0x140) = i;
                *(s32 *)(rec + 0x144) = 0;
                *(s32 *)(rec + 0x148) = *(s32 *)(def + 0x14);
                *(s32 *)(rec + 0x14C) = 0xCDB;
                *(s32 *)(rec + 0x150) = 0;
                *(s32 *)(rec + 0x154) = 1;
                *(s32 *)(ui + 0x740) = count + 1;
            }

            /* Owned catalog item (+0x88): list it unless already listed WITHOUT
             * the finalized flag (inverse of the dedup test above). */
            if (*(u16 *)(def + 0x88) != 0) {
                s32 k, skip = 0;
                count = *(s32 *)(ui + 0x740);
                if (count > 0) {
                    for (k = 0; k < count; k++) {
                        if (*(s32 *)(ui + k * 0x18 + 0x140) == i) {
                            break;
                        }
                    }
                    if (k < count && *(s32 *)(ui + k * 0x18 + 0x154) == 0) {
                        skip = 1;
                    }
                }
                if (!skip) {
                    count = *(s32 *)(ui + 0x740);
                    rec = ui + count * 0x18;
                    *(s32 *)(rec + 0x140) = i;
                    *(s32 *)(rec + 0x144) = 1;
                    *(s32 *)(rec + 0x148) = *(s32 *)(def + 0x14);
                    *(s32 *)(rec + 0x14C) = *(s32 *)(D_262E00 + i * 0x14);
                    *(s32 *)(rec + 0x150) = 0;
                    *(s32 *)(rec + 0x154) = 0;
                    *(s32 *)(ui + 0x740) = count + 1;
                }
            }
        }
    }

    /* -------- compaction: drop records duplicating an earlier one in fields
     * +0x140/+0x144/+0x154, shifting the tail down; repeat until a pass makes no
     * change. Translated operationally from the original: the "dirty" latch
     * persists across the outer index within a pass, exactly as the asm's $14. */
    for (;;) {
        s32 dirty = 0;
        s32 a = 0;
        count = *(s32 *)(ui + 0x740);
        if (count - 1 <= 0) {
            break;
        }
        for (;;) {
            s32 a140 = *(s32 *)(ui + a * 0x18 + 0x140);
            s32 a144 = *(s32 *)(ui + a * 0x18 + 0x144);
            s32 a154 = *(s32 *)(ui + a * 0x18 + 0x154);
            count = *(s32 *)(ui + 0x740);
            if (a + 1 < count) {
                s32 b = a + 1;
                for (;;) {
                    if (*(s32 *)(ui + b * 0x18 + 0x140) == a140 &&
                        *(s32 *)(ui + b * 0x18 + 0x144) == a144 &&
                        *(s32 *)(ui + b * 0x18 + 0x154) == a154) {
                        count = *(s32 *)(ui + 0x740);
                        dirty = 1;
                        if (b < count - 1) {
                            s32 c = b;
                            for (;;) {
                                char *p = ui + c * 0x18 + 0x140;
                                *(s32 *)(p + 0x00) = *(s32 *)(p + 0x18 + 0x00);
                                *(s32 *)(p + 0x04) = *(s32 *)(p + 0x18 + 0x04);
                                *(s32 *)(p + 0x08) = *(s32 *)(p + 0x18 + 0x08);
                                *(s32 *)(p + 0x0C) = *(s32 *)(p + 0x18 + 0x0C);
                                *(s32 *)(p + 0x10) = *(s32 *)(p + 0x18 + 0x10);
                                *(s32 *)(p + 0x14) = *(s32 *)(p + 0x18 + 0x14);
                                c++;
                                count = *(s32 *)(ui + 0x740);
                                if (!(c < count - 1)) {
                                    break;
                                }
                            }
                        }
                    }
                    if (!dirty) {
                        count = *(s32 *)(ui + 0x740);
                        b++;
                        if (b < count) {
                            continue;
                        }
                        break;
                    } else {
                        count = *(s32 *)(ui + 0x740);
                        *(s32 *)(ui + 0x740) = count - 1;
                        break;
                    }
                }
            }
            count = *(s32 *)(ui + 0x740);
            a++;
            if (a < count - 1) {
                continue;
            }
            break;
        }
        if (!dirty) {
            break;
        }
    }
}
#endif

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
/* RESIDUAL CLASS (task #576): UNDIAGNOSED
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 42.54%, engine96 48.78%
 *   (better arm: engine96). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   SCREENED ONLY. Both arms were measured; the residual was not diagnosed to a
 *   mechanism. This is an open arm, not a wall -- do not read it as one. */
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

/* RESIDUAL CLASS (task #576): UNDIAGNOSED
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 31.48%, engine96 59.70%
 *   (better arm: engine96). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   SCREENED ONLY. Both arms were measured; the residual was not diagnosed to a
 *   mechanism. This is an open arm, not a wall -- do not read it as one. */
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

/* func_002F8228 [SEEDABLE] — rebuild the Gadgetron vendor item list from current
 * player state. Two passes over the 0x38 item ids:
 *   Pass 1: for each item with a live weapon-table entry, record which of its up
 *           to three variant tiers (j=0..2) are purchasable — either unlocked by
 *           story progress (func_002F81A0(g_playerProgress, item, tier)) or
 *           available as a paid upgrade (weapon-table tier word +0x98/+0x9C/+0xA0
 *           nonzero AND IsVendorUpgradesUnlocked) — as a bitmask in D_1A7AC0[item].
 *   Pass 2: for each owned item (g_inventoryOwned), append a list entry (base
 *           g_vendorUi+0x140, stride 0x10, count g_vendorUi+0x740) for every
 *           purchasable tier not already flagged listed in g_itemStateFlags; each
 *           entry stores {tier, item id, tier word, weapon field +0x14}. Item 0 is
 *           skipped. (The compiler expanded the tier-word offset into a 3-way
 *           branch; it is just +0x98 + tier*4.) Engine-region (ee-gcc 2.96) —
 *           matching-walled; portable #else body. Field offsets kept literal. */
/* RESIDUAL CLASS (task #576): SPLIT-WITHIN-FUNCTION -- g_playerProgress
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 33.56%, engine96 34.91%
 *   (better arm: engine96). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   the named symbol is reached BOTH ways inside this one function -- absolute
 *   %hi/%lo AND %gp_rel, same address. The split is POSITIONAL, not a size
 *   class (FACT #8058): every %gp_rel ref sits in a branch delay slot and every
 *   absolute ref outside one, 0 exceptions over all 122 compiled USA functions
 *   that split a symbol this way. No `.extern` size can express that; an
 *   assembler-side delay-slot rule could, and tools/ee has none yet.
 *   Unreachable today, but not proven a wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F8228);
#else
extern s32 g_nVendorItemCount; /* reset each rebuild (list count lives in g_vendorUi+0x740) */
extern u8  D_1A7AC0[];         /* 0x1A7AC0 per-item purchasable-tier bitmask (pass 1 -> pass 2) */
extern u8  g_inventoryOwned[]; /* per-item "owned" flag */
extern u8  g_itemStateFlags[]; /* per-item already-listed tier flags (bit 2*tier) */

void func_002F8228(void) {
    char *ui = (char *)&g_vendorUi;
    s32 i, j, k, m;

    g_nVendorItemCount = 0;

    /* Pass 1: mark each item's purchasable variant tiers. */
    for (i = 0; i < 0x38; i++) {
        u8 *def = g_weaponTable + g_itemEquippedSlot[i] * 0xE0;
        if (*(s32 *)def == 0) {
            continue;
        }
        for (j = 0; j < 3; j++) {
            if (func_002F81A0(g_playerProgress, i, j) != 0) {
                D_1A7AC0[i] |= (1 << j);
            } else if (*(s32 *)(def + 0x98 + j * 4) != 0 && IsVendorUpgradesUnlocked() != 0) {
                D_1A7AC0[i] |= (1 << j);
            }
        }
    }

    /* Pass 2: append a list entry per owned item's not-yet-listed tiers. */
    for (k = 0; k < 0x38; k++) {
        u8 *def;
        s32 mask;
        if (k == 0) {
            continue; /* item 0 is never listed */
        }
        def = g_weaponTable + g_itemEquippedSlot[k] * 0xE0;
        if (*(s32 *)def == 0 || g_inventoryOwned[k] == 0) {
            continue;
        }
        mask = D_1A7AC0[k];
        for (m = 0; m < 3; m++) {
            s32 tierWord;
            s32 n;
            char *entry;
            if (!(mask & (1 << m))) {
                continue;
            }
            tierWord = *(s32 *)(def + 0x98 + m * 4);
            if (tierWord == 0) {
                continue;
            }
            if (g_itemStateFlags[k] & (1 << (m * 2))) {
                continue; /* this tier is already in the list */
            }
            n = *(s32 *)(ui + 0x740);
            entry = ui + 0x140 + n * 0x10;
            *(s32 *)(entry + 0x00) = m;                    /* tier index    */
            *(s32 *)(entry + 0x08) = tierWord;             /* tier price/def */
            *(s32 *)(entry + 0x04) = k;                    /* item id        */
            *(s32 *)(entry + 0x0C) = *(s32 *)(def + 0x14); /* weapon +0x14   */
            *(s32 *)(ui + 0x740) = n + 1;
        }
    }
}
#endif

/* func_002F85B8 — rebuild the Gadgetron vendor's ship-customization + special-item
 * list from current player state.
 *
 * Eight parallel 36-row tables drive the build, one column per record field:
 *   D_26D528 = name string id (-> GetLocalizedString), D_26D5B8 = aux word,
 *   D_26D648 = the g_shipCustomization bit-field mask, D_26D6D8 = expected/aux word,
 *   D_26D768 = item id, D_26D7F8 = required skill-point count, D_26D890 = price/def
 *   word, D_1AD300 = extra word (ship-customization records only). The original
 *   copies each table into a stack scratch buffer first; since nothing writes them
 *   in between, those reads are equivalent to indexing the globals directly (done
 *   here — the copies are elided).
 *
 * Two groups of entries are appended (running count at g_vendorUi+0x740, records
 * stride 0x1C):
 *   1. Ship-customization upgrades (rows 0..12): for each category, the field value
 *      v = (g_shipCustomization & D_26D648[row]) >> shift selects the next buyable
 *      variant. 1-bit fields append their row when v == 0; 2-bit fields append the
 *      level-1 row at v == 0 and the level-2 row at v == 1, nothing once maxed
 *      (v >= 2). One g_vendorUi record is emitted per collected row (+0x140..+0x158).
 *   2. Special items (rows 13..35): emitted when (g_shipCustomization &
 *      D_26D648[row]) != D_26D6D8[row] AND CountSkillPointsCompleted() >=
 *      D_26D7F8[row]; each fills a g_vendorItemList record (+0x0,+0x4) and a
 *      g_vendorUi record (+0x148,+0x14C,+0x154,+0x158).
 *
 * Engine-region (ee-gcc 2.96) — matching-walled; portable #else body. Field offsets
 * kept literal (partial VendorUiState). */
/* RESIDUAL CLASS (task #576): SPLIT-WITHIN-FUNCTION -- g_shipCustomization
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 20.46%, engine96 18.09%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   the named symbol is reached BOTH ways inside this one function -- absolute
 *   %hi/%lo AND %gp_rel, same address. The split is POSITIONAL, not a size
 *   class (FACT #8058): every %gp_rel ref sits in a branch delay slot and every
 *   absolute ref outside one, 0 exceptions over all 122 compiled USA functions
 *   that split a symbol this way. No `.extern` size can express that; an
 *   assembler-side delay-slot rule could, and tools/ee has none yet.
 *   Unreachable today, but not proven a wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002F85B8);
#else
extern s32   g_shipCustomization;     /* ship-customization unlock bit-field */
extern s32   g_vendorItemList;        /* 0x1C-stride record array, parallel to g_vendorUi */
extern s32   D_26D528[]; /* row -> name string id      */
extern s32   D_26D5B8[]; /* row -> aux word (ship-cust) */
extern s32   D_26D648[]; /* row -> g_shipCustomization mask */
extern s32   D_26D6D8[]; /* row -> expected/aux word    */
extern s32   D_26D768[]; /* row -> item id              */
extern s32   D_26D7F8[]; /* row -> required skill points */
extern s32   D_26D890[]; /* row -> price/def word       */
extern s32   D_1AD300[]; /* row -> extra word (ship-cust) */
extern s32   CountSkillPointsCompleted(void);
extern void *GetLocalizedString(s32 stringId);

void func_002F85B8(void) {
    char *ui    = (char *)&g_vendorUi;
    char *list  = (char *)&g_vendorItemList;
    s32   flags = g_shipCustomization;
    s32   skillPoints = CountSkillPointsCompleted();
    s32   codes[16];
    s32   n = 0;
    s32   i, j, v, count;

    /* Group 1a: collect the next-buyable row for each not-yet-maxed ship-
     * customization category (see header for the 1-bit / 2-bit field rules). */
    if (((flags & D_26D648[0])  >> 4)  == 0) codes[n++] = 0;
    if (((flags & D_26D648[2])  >> 6)  == 0) codes[n++] = 2;
    v = (flags & D_26D648[3]) >> 7;
    if (v == 0) codes[n++] = 3; else if (v == 1) codes[n++] = 4;
    if (((flags & D_26D648[5])  >> 9)  == 0) codes[n++] = 6;
    v = (flags & D_26D648[7]) >> 10;
    if (v == 0) codes[n++] = 7; else if (v == 1) codes[n++] = 8;
    v = (flags & D_26D648[9]) >> 14;
    if (v == 0) codes[n++] = 9; else if (v == 1) codes[n++] = 0xA;
    if (((flags & D_26D648[11]) >> 12) == 0) codes[n++] = 0xB;
    if (((flags & D_26D648[12]) >> 13) == 0) codes[n++] = 0xC;

    /* Group 1b: one g_vendorUi record per collected code. */
    *(s32 *)(ui + 0x740) = 0;
    for (j = 0; j < n; j++) {
        s32   code = codes[j];
        char *rec;
        count = *(s32 *)(ui + 0x740);
        rec = ui + count * 0x1C;
        *(s32 *)(rec + 0x144) = D_26D890[code];
        *(s32 *)(rec + 0x140) = D_26D768[code];
        *(s32 *)(rec + 0x154) = (s32)GetLocalizedString(D_26D528[code]);
        *(s32 *)(rec + 0x158) = D_26D5B8[code];
        *(s32 *)(rec + 0x148) = D_26D648[code];
        *(s32 *)(rec + 0x14C) = D_26D6D8[code];
        *(s32 *)(rec + 0x150) = D_1AD300[code];
        *(s32 *)(ui + 0x740) = count + 1;
    }

    /* Group 2: special items (rows 13..35), gated by story flags + skill points. */
    for (i = 0; i <= 0x16; i++) {
        s32   t = 13 + i;
        char *rec;
        if ((flags & D_26D648[t]) == D_26D6D8[t]) {
            continue;
        }
        if (skillPoints < D_26D7F8[t]) {
            continue;
        }
        count = *(s32 *)(ui + 0x740);
        *(s32 *)(list + count * 0x1C + 0x4) = D_26D890[t];
        *(s32 *)(list + count * 0x1C + 0x0) = D_26D768[t];
        rec = ui + count * 0x1C;
        *(s32 *)(rec + 0x154) = (s32)GetLocalizedString(D_26D528[t]);
        *(s32 *)(rec + 0x158) = D_26D528[t];
        *(s32 *)(rec + 0x148) = D_26D648[t];
        *(s32 *)(rec + 0x14C) = D_26D6D8[t];
        *(s32 *)(ui + 0x740) = count + 1;
    }
}
#endif

/* EnterVendorMenu [SEEDABLE] — open the Gadgetron vendor shop UI.
 *
 * `arg` is overloaded (moby pointer / small sentinel): when (u32)arg < 4 it is a
 * spawn-mode selecting which fresh preview the shop opens with; otherwise it is an
 * existing preview moby pointer to reuse.
 *
 * Zeroes the whole g_vendorUi block (FillMemory32), sets the panel vertical-scale
 * (g_nVendorBuyQuantity+0x38) to 1.0 (NTSC) or 0x3F89D89D (PAL), and latches the
 * current armor (D_1AD240 = g_equippedArmor). Then:
 *   - spawn mode (arg < 4): SpawnMoby(0xB) a preview moby, seed its transform
 *     (+0x10) from the D_1AD260/D_1AD270 template plus g_cameraPos (z negated),
 *     zero its scratch quad (+0xF0), face it (+0xF8 = pi), register it
 *     (UpdateMobyBSphereAndGrid), then pick the initial tab (+0x50) from the mode
 *     (0/1/2/3), tab 2 also seeding the pitch (+0xBC = -pi/2) and clearing +0xA4/
 *     +0xB4/+0xB8.
 *   - reuse mode (arg is a moby p): pick the tab from the moby class (+0xAA) and,
 *     for class 0xB, its controller sub-state (*(p+0x68)+0x94); classes 0x1309/
 *     0xDDC store the moby into +0x28.
 * A forced tab override (D_1AD244, -1 = none) wins if set. Then initialise the rest
 * of the shop: language-derived +0x44, camera/zoom scratch (g_nVendorBuyQuantity
 * +0x3C..+0x50, D_1AD2B0/B4/B8), retune music (func_00132AF8) and max the dialog
 * voices; build the item list for the chosen tab (BuildVendorItemList / func_002F8038
 * / func_002F8228 / func_002F85B8); centre the selection (+0x80/+0x6C); refresh the
 * caption sound (func_0029C488/func_0029C4C0); wire the render buffers from
 * g_memoryArenaTable + screen dims; play the open sound (PlayMobySound 3); enter game
 * state 5 (func_0026F750) and — when no camera override is parked (+0x60 == 0) —
 * position the preview camera (func_00283A48/Vec4AddVu0/WrapAnglePiSum/func_00336230/
 * func_00336168/func_003361C0/UpdateCamera). Finally seed the directional-light
 * preview matrices (g_dirLightMatrices+0x380..+0x3B0).
 *
 * Engine-region (ee-gcc 2.96): every lq/sq is a plain 16-byte block copy and all vec/
 * matrix/spawn math is done by callees, so the body ports faithfully as portable C.
 * The `g_nVendorPurchaseState+0x4C` reference in the original is a linker
 * nearest-symbol artifact for g_vendorUi+0xD0 (delta 0x84) — kept ui-relative here.
 * Matching-walled — kept as the portable #else impl. Field offsets kept literal. */
/* RESIDUAL CLASS (task #576): UNDIAGNOSED
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 52.85%, engine96 51.02%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   SCREENED ONLY. Both arms were measured; the residual was not diagnosed to a
 *   mechanism. This is an open arm, not a wall -- do not read it as one. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", EnterVendorMenu);
#else
extern s32   g_bPalMode;              /* PAL flag (0 = NTSC) */
extern u8    g_nVendorBuyQuantity[];  /* vendor buy-quantity + camera/param scratch */
extern s16   g_equippedArmor;         /* current player armor tier */
extern u8    g_currentLanguage;       /* language index */
extern u8    D_1A7BB9;                /* widescreen preview-template selector */
extern u8    D_1AD260[16];            /* preview transform template A */
extern u8    D_1AD270[16];            /* preview transform template B (widescreen) */
extern u8    D_1AD290[16];            /* light-matrix transform template */
extern u8    D_1AD2A0[16];            /* light-matrix fill template */
extern s32   D_1AD244;                /* forced initial tab (-1 = none) */
extern s32   D_1AD2B0;                /* zoom/lerp scratch (cleared) */
extern f32   D_1AD2B4;                /* zoom rate (0.8) */
extern f32   D_1AD2B8;                /* zoom rate (1.0) */
extern u8    g_cameraPos[];           /* camera world position vec4 */
extern u8    g_dirLightMatrices[];    /* directional light matrix block */
extern u8    g_soundBankHandlesBlk[]; /* sound-bank handle block base */
extern u8    g_memoryArenaTable[];    /* memory-region table */
extern s32   g_screenWidth;
extern s32   g_screenHeight;
extern f32   g_screenFadeWhite;       /* white screen-flash level */
extern s32   g_shipCustomization;     /* selected ship-customization id */
extern u8    g_respawnPlayerYaw[];    /* saved respawn block; +0x5C read here */

extern void  FillMemory32(void *dst, s32 value, s32 nbytes);
extern void *SpawnMoby(s32 oClass);
extern void  UpdateMobyBSphereAndGrid(void *moby);
extern void  func_00283638(void *dst);
extern void  Vec4AddVu0(void *dst, void *a, void *b);
extern f32   WrapAnglePiSum(f32 angle, f32 modulus);
extern void  func_00283A48(void *outXform, void *tmpl, void *origin);
extern void  func_00336230(void *a, void *b, s32 c, s32 d, s32 e);
extern void  func_00336168(void *v);
extern void  func_003361C0(void *v);
extern void  UpdateCamera(void);
extern void  func_0028EC70(void);
extern void  func_0029C488(s32 arg);
extern void  func_0029C4C0(s32 a, s32 b);
extern void  func_002FA608(void);
extern void  func_002F85B8(void);
extern void  func_002898E0(void);
extern void  func_0026F750(s32, s32);
extern void  func_0026F728(void);
/* Takes ONE arg: the sound-command word. asm 0x132AF8 does `sw $4,0x0($29)` and
 * hands that stack word to snd_QueueCommandToRing(0x16, 4, &word, 0, 0), so $4
 * is consumed. Declared correctly at usa/text/1CA080.c:450. */
extern void  func_00132AF8(s32 arg);
extern void  SetDialogVoiceVolumesMax(s32 arg);
extern void  snd_Pump(void);
extern s32   PlayMobySound(s32 soundIdx, s32 flags, void *owner);
extern void  BuildVendorItemList(s32 arg);

void EnterVendorMenu(s32 arg) {
    char *ui = (char *)&g_vendorUi;
    char *moby;

    FillMemory32(&g_vendorUi, 0, 0x760);

    if (g_bPalMode == 0) {
        *(f32 *)(g_nVendorBuyQuantity + 0x38) = 1.0f;
    } else {
        *(u32 *)(g_nVendorBuyQuantity + 0x38) = 0x3F89D89D; /* PAL vertical scale */
    }

    *(s32 *)(ui + 0x60) = 0;
    D_1AD240 = g_equippedArmor;

    if ((u32)arg < 4) {
        s32 mode = arg;

        *(s32 *)(ui + 0x60) = 1;
        moby = (char *)SpawnMoby(0xB);
        *(s16 *)(moby + 0x32) = 0x40;

        {
            f32 tmpl[4];
            u8 *src = D_1A7BB9 ? (u8 *)D_1AD270 : (u8 *)D_1AD260;
            memcpy(tmpl, src, 0x10);
            tmpl[2] = -tmpl[2];
            Vec4AddVu0(moby + 0x10, g_cameraPos, tmpl);
        }
        func_00283638(moby + 0xF0);
        *(f32 *)(moby + 0xF8) = PR_PI;
        UpdateMobyBSphereAndGrid(moby);

        *(s32 *)(ui + 0x60) = 1;
        *(s32 *)(ui + 0x744) = 0;
        *(u8 *)(moby + 0x20) = 3;

        if (mode == 0) {
            *(s32 *)(ui + 0x50) = 0;
            *(s32 *)(ui + 0x744) = 0;
        } else if (mode == 1) {
            *(s32 *)(ui + 0x50) = 1;
        } else if (mode == 2) {
            *(s32 *)(ui + 0x50) = 2;
            *(u32 *)(ui + 0xBC) = 0xBFC90FDB; /* -pi/2 */
            *(s32 *)(ui + 0xA4) = 0;
            *(s32 *)(ui + 0xB4) = 0;
            *(s32 *)(ui + 0xB8) = 0;
        } else { /* mode == 3 */
            *(s32 *)(ui + 0x50) = 3;
        }
    } else {
        char *p = (char *)arg;
        s32   kind = *(s16 *)(p + 0xAA);

        moby = p;
        if (kind == 0xB) {
            s32 sub = *(s32 *)(*(char **)(p + 0x68) + 0x94);
            *(s32 *)(ui + 0x744) = 0;
            if (sub == 0) {
                *(s32 *)(ui + 0x50) = 0;
            } else if (sub == 1) {
                *(s32 *)(ui + 0x50) = 1;
                func_002FA608();
            } else if (sub == 4) {
                *(s32 *)(ui + 0x50) = 2;
            } else if (sub == 2) {
                *(s32 *)(ui + 0x50) = 3;
            } else if (sub == 3) {
                *(s32 *)(ui + 0x744) = 1;
                *(s32 *)(ui + 0x50) = 0;
            }
        } else if (kind == 0x1309) {
            *(char **)(ui + 0x28) = p;
            *(s32 *)(ui + 0x50) = 2;
        } else if (kind == 0xDDC) {
            *(char **)(ui + 0x28) = p;
            *(s32 *)(ui + 0x50) = 1;
            func_002FA608();
        }
    }

    /* forced-tab override wins if latched */
    {
        s32 forced = D_1AD244;
        if (forced != -1) {
            *(s32 *)(ui + 0x50) = forced;
        }
    }

    /* language-derived caption index (+0x44) */
    {
        s32 lang = g_currentLanguage;
        *(s32 *)(ui + 0x88) = 0;
        *(s32 *)(ui + 0x8C) = 0;
        *(s32 *)(ui + 0x38) = 0;
        *(s32 *)(ui + 0x44) = lang - 1;
        if (lang - 1 < 0) {
            *(s32 *)(ui + 0x44) = 0;
        }
    }

    /* camera / zoom / buy-quantity scratch init */
    *(s32 *)(g_nVendorBuyQuantity + 0x4C) = 0x60;
    D_1AD2B4 = 0.8f;
    D_1AD2B8 = 1.0f;
    *(s32 *)(g_nVendorBuyQuantity + 0x50) = 0x55;
    *(s32 *)(ui + 0x68) = -1;
    *(s32 *)(ui + 0x78) = -1;
    *(s32 *)(ui + 0x7C) = -1;
    *(s32 *)(ui + 0x64) = 0;
    *(s32 *)(ui + 0x80) = 0;
    *(s32 *)((char *)&g_tieVramLruSize + 0x5C) = 0;
    *(s32 *)(ui + 0x84) = 0;
    *(s32 *)(g_nVendorBuyQuantity + 0x48) = 0x60;
    *(s32 *)g_nVendorBuyQuantity = 0;
    *(s32 *)((char *)&g_tieVramLruSize + 0x50) = 0;
    D_1AD2B0 = 0;
    *(s32 *)(g_nVendorBuyQuantity + 0x3C) = 0;
    *(s32 *)(g_nVendorBuyQuantity + 0x40) = 0;
    *(s32 *)(g_nVendorBuyQuantity + 0x44) = 0;
    /* asm 002F9274 `addiu $4,$0,0x5D` is the last write to $4 before the jal at
     * 002F92D8 -- the ROM passes the constant 0x5D. The sibling call site
     * usa/text/1CA080.c:473 passes the same 0x5D. Dropped, $4 arrived holding
     * %hi(D_1AD2B8), the address base left by the store two statements up. */
    func_00132AF8(0x5D);
    SetDialogVoiceVolumesMax(0);
    snd_Pump();

    /* build the item list for the chosen tab (+0x50) */
    switch (*(s32 *)(ui + 0x50)) {
    case 0: BuildVendorItemList(0); break;
    case 3: func_002F8038();        break;
    case 1: func_002F8228();        break;
    case 2: func_002F85B8();        break;
    default:                        break;
    }

    /* centre the selection on the middle row */
    {
        s32 cnt  = *(s32 *)(ui + 0x740);
        s32 half = cnt / 2;
        s32 sel  = (2 < cnt) ? half : 1;
        *(s32 *)(ui + 0x80) = half;
        *(s32 *)(ui + 0x6C) = (sel - half) * 0x28;
    }
    func_0028EC70();

    /* refresh the caption/help sound for the active tab */
    {
        s32 tab = *(s32 *)(ui + 0x50);
        if (tab != 2) {
            func_0029C488(0x112A880);
            tab = *(s32 *)(ui + 0x50);
        }
        if (tab == 0) {
            s32   half = *(s32 *)(ui + 0x80);
            char *slot = ui + half * 0x18;
            if (*(s32 *)(slot + 0x144) == 1 &&
                *(s32 *)(g_soundBankHandlesBlk + 0x1248) != 0xA) {
                func_0029C4C0(*(s32 *)(slot + 0x140), 0x112A880);
            }
        }
    }

    /* wire the render/scratch buffers from the arena table + screen dims */
    {
        s32 w        = g_screenWidth;
        s32 h        = g_screenHeight;
        s32 base0xC  = *(s32 *)(g_memoryArenaTable + 0xC)  + 0x60000;
        s32 base0x10 = *(s32 *)(g_memoryArenaTable + 0x10) + 0x60000;
        *(s32 *)(ui + 0x14) = base0xC;
        *(s32 *)(ui + 0x1C) = base0x10 + ((w * h) << 2);
        *(s32 *)(ui + 0x00) = 0;
        *(s32 *)(ui + 0x10) = base0x10;
        *(s32 *)(ui + 0x08) = 0;
        *(s32 *)(ui + 0x0C) = 1;
        *(char **)(ui + 0x28) = moby;
        PlayMobySound(3, 0x11, moby);
    }

    /* enter the shop game state */
    *(s32 *)(ui + 0x4C) = g_shipCustomization;
    *(s32 *)(ui + 0xA0) = *(s16 *)(g_respawnPlayerYaw + 0x5C);
    func_002898E0();
    g_nGameState = 5;
    g_screenFadeBlack = 0.0f;
    g_screenFadeWhite = 0.0f;
    func_0026F750(0x61, 1);
    g_soundBankHandlesBlk[0x22B5] = 1;
    g_soundBankHandlesBlk[0x22BC] = 1;
    func_0026F728();

    /* position the preview camera when no override is parked */
    if (*(s32 *)(ui + 0x60) == 0) {
        char *pmoby = *(char **)(ui + 0x28);
        f32   local[4];

        if (D_1A7BB9) {
            func_00283A48(ui + 0xD0, D_1AD270, pmoby + 0xC0);
        } else {
            func_00283A48(ui + 0xD0, D_1AD260, pmoby + 0xC0);
        }
        Vec4AddVu0(ui + 0xD0, ui + 0xD0, pmoby + 0x10);
        *(f32 *)(ui + 0xE8) = WrapAnglePiSum(*(f32 *)(pmoby + 0xF8), PR_PI);
        func_00336230(ui + 0xD0, ui + 0xE0, 1, 0, 0);

        memcpy(local, ui + 0xD0, 0x10);
        if (*(s32 *)(ui + 0x50) == 2) {
            local[1] += 5.0f;
        }
        func_00336168((void *)local);
        func_003361C0(ui + 0xE0);
        UpdateCamera();
    }

    /* seed the directional-light preview matrices */
    {
        char *lm    = (char *)&g_dirLightMatrices + 0x380;
        char *pmoby = *(char **)(ui + 0x28);
        func_00283A48(lm + 0x10, D_1AD290, pmoby + 0xC0);
        memcpy(lm, D_1AD2A0, 0x10);
        memset(lm + 0x30, 0, 0x10);
        memset(lm + 0x20, 0, 0x10);
    }
}
#endif

/* TeardownVendorSceneRestorePlayer [SEEDABLE] — tear down the vendor UI's preview widgets and hand
 * control back to the world, optionally spawning the departing preview actor.
 *
 * Releases the two live preview handles (+0x7C, +0x78) through func_0028C108 and
 * blanks them, quiesces the vendor sub-scene (func_0028ECA8), returns to game
 * state 0 (func_0026F750) and clears the global-scene-active flag (func_0026F730).
 * Then, only when no preview actor is already parked (+0x60 == 0): allocate one
 * (func_00299980), seed its transform from the D_1AD280 template plus the current
 * preview moby's (+0x28) origin (+0xC0) and offset (+0x10) via func_00283A48 /
 * Vec4AddVu0, wrap its facing angle (+0xF8) into [-pi,pi] (WrapAnglePiSum) and
 * place it (func_0026F780), then kick the fx queue (func_003363A0(2)). Always:
 * mark the preview moby active (+0x20 = 1), nudge the save-image-area sub-state
 * halfword (+0x1072) out of the {6,7} band to 5, run the +0x8C teardown hook
 * (func_002FBFD8) and free the preview moby (+0x28) when one is parked (+0x60 != 0),
 * then retune music (func_00132B28), mute dialog voices and pump the sound stack.
 *
 * Engine-region (ee-gcc 2.96): the lq/sq is a plain 16-byte template copy and the
 * VU0/angle math is done by callees, so the body ports faithfully as portable C.
 * Matching-walled — kept as the portable #else impl. Field offsets kept literal
 * (partial VendorUiState). */
/* RESIDUAL CLASS (task #576): GPREL-MISMATCH-INSUFFICIENT -- g_nGameState; per-arm size 4 measured 65.58 -> 68.62 on engine96, still DIFFERS and still below its own sdk29 arm
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 75.63%, engine96 65.58%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   addressing is A residual but NOT the only one -- measured, not assumed: */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", TeardownVendorSceneRestorePlayer);
#else
extern u8   g_bGlobalSceneActive;    /* global sub-scene active flag (byte) */
extern u8   g_saveImageArea[];       /* +0x1072 = pause/save sub-state halfword */
extern u8   D_1AD280[16];            /* preview transform template (vec4) */
extern void func_0028C108(s32 handle, s32 arg); /* release a menu sound/anim handle */
extern void func_0028ECA8(void);     /* quiesce the vendor sub-scene */
extern void func_0026F750(s32, s32); /* restore top-level game/screen mode */
extern void func_0026F730(void);     /* clear global-scene bookkeeping */
extern void *func_00299980(void);    /* allocate a preview actor, returns handle */
extern void func_00283A48(void *outXform, void *tmpl, void *origin);
extern void Vec4AddVu0(void *dst, void *a, void *b);
extern f32  WrapAnglePiSum(f32 angle, f32 modulus);
extern void func_0026F780(void *xform, void *rot, s32 flags, void *actor);
extern void func_003363A0(s32);
extern void func_002FBFD8(void);
extern void FreeMoby(void *moby);
extern void func_00132B28(s32);
extern void snd_Pump(void);

void TeardownVendorSceneRestorePlayer(void) {
    char *ui = (char *)&g_vendorUi;
    char *pv;
    u16  *subState;

    /* release + blank the two live preview handles */
    if (*(s32 *)(ui + 0x7C) != -1) {
        func_0028C108(*(s32 *)(ui + 0x7C), 0);
    }
    if (*(s32 *)(ui + 0x78) != -1) {
        func_0028C108(*(s32 *)(ui + 0x78), 0);
    }
    *(s32 *)(ui + 0x78) = -1;
    *(s32 *)(ui + 0x7C) = -1;
    func_0028ECA8();

    /* return to the world: game state 0, drop the global-scene flag */
    g_nGameState = 0;
    func_0026F750(0, 1);
    g_bGlobalSceneActive = 0;
    func_0026F730();

    /* no preview parked yet -> spawn the departing preview actor */
    if (*(s32 *)(ui + 0x60) == 0) {
        void *actor = func_00299980();
        char *moby  = *(char **)(ui + 0x28);
        f32   xform[4];
        f32   rot[4];
        f32   tmpl[4];

        memcpy(tmpl, D_1AD280, sizeof(tmpl));
        func_00283A48(xform, tmpl, moby + 0xC0);
        Vec4AddVu0(xform, xform, moby + 0x10);

        rot[0] = 0.0f;
        rot[1] = 0.0f;
        rot[2] = WrapAnglePiSum(*(f32 *)(moby + 0xF8), PR_PI);
        func_0026F780(xform, rot, 0, actor);
        func_003363A0(2);
    }

    pv = *(char **)(ui + 0x28);
    *(u8 *)(pv + 0x20) = 1;

    /* bump the save-image-area sub-state out of the {6,7} band */
    subState = (u16 *)(g_saveImageArea + 0x1072);
    if ((u32)(*subState - 6) >= 2) {
        *subState = 5;
    }

    if (*(s32 *)(ui + 0x8C) != 0) {
        func_002FBFD8();
    }
    if (*(s32 *)(ui + 0x60) != 0) {
        FreeMoby(*(char **)(ui + 0x28));
    }

    func_00132B28(0x5D);
    SetDialogVoiceVolumesMute();
    snd_Pump();
}
#endif

/* VendorUiState + g_vendorUi are declared above (moved up for func_002F8038). */
extern u8 g_vendorCaptionFmt[];      /* 0x1AD338 caption format/template string */
/* 0x00115DA8 IS sprintf (symbol_addrs: `sprintf = 0x00115DA8`), and the ROM's jal
 * at 0x2F9788 spells it that way. It is nevertheless declared here under its
 * legacy auto-name: introducing `sprintf` as a second live C name for 0x115DA8
 * GROWS the ORPHAN_LATENT never-grow set (106 -> 107, NEW func_00115DA8) and
 * landing_gate.sh usa --strict FAILS on it -- func_00115DA8 is still the spelling
 * used by text/188858.c and text/1CA080.c, so the two names would coexist. Under
 * RULING #7317 a baseline is never raised to admit a landing, so the rename is a
 * separate tree-wide cleanup, not this landing. Measured here: the spelling is
 * NOT a matching lever -- SetVendorCaption scores 99.00% either way. */
void func_00115DA8(s32 widget, void *fmt, s32 captionId);

/* Bind a caption string to the vendor UI's caption text widget (formatting it
 * through sprintf at 0x115DA8) and reset the caption refresh counter.
 *
 * RESIDUAL CLASS: compiler-revision codegen, on BOTH arms and for a DIFFERENT
 * reason on each -- measured task #576 (unit objdiff, objdiff_build.sh +
 * unit_report.sh, clean tree, each arm built alone):
 *   sdk29    99.00%  callee-save STRIDE only. Scheduling is already exact; the
 *                    original packs the two slots 8-byte (frame 0x10,
 *                    sd s0,0x0 / sd ra,0x8), this cc1 emits 16-byte spacing
 *                    (frame 0x20, ra at +0x10).
 *   engine96 77.20%  the packing is CORRECT here (frame 0x10, ra at +0x8) but
 *                    the schedule is not: 2.96 hoists lui/addiu %hi(g_vendorUi)
 *                    ahead of %hi(g_vendorCaptionFmt) and fills the jal delay
 *                    slot with `addiu $5` instead of the original `lw $4,0x3C`.
 * The former comment called the 8-byte packing a wall; the MECHANISM is right
 * and is kept. What it did not say is that the packing is an ARM CHOICE (#565,
 * func_002A0480) which here does NOT pay: the engine arm buys the stride and
 * loses the schedule, so neither arm closes.
 * The restore ORDER (ROM: ld s0,0x0 then ld ra,0x8) IS reachable on engine96:
 * a trailing __asm__ __volatile__("") after the last store gives it (task #749,
 * diff96.sh per-function probe 76.20% -> 77.00%, re-derived by task #762).
 * What remains is argument-setup order: 2.96-001003 materialises %hi(g_vendorUi)
 * into s0 before %hi(g_vendorCaptionFmt). Task #762 got the IDENTICAL listing
 * (diff96.sh 77.00%) from five variants: sprintf's variadic prototype, fmt and
 * dst hoisted into locals, a VendorUiState * alias, the assignment inside the
 * argument, and sched1 ON. So this is not a source-order lever. Left
 * INCLUDE_ASM as the portable #else impl. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", SetVendorCaption);
#else
void SetVendorCaption(s32 captionId) {
    func_00115DA8(g_vendorUi.captionWidget, g_vendorCaptionFmt, captionId);
    g_vendorUi.captionCounter = 0;
}
#endif

/* VendorPurchaseStateMachine [SEEDABLE] — drive one step of the Gadgetron
 * buy/confirm/apply transaction for the currently-selected shop slot. Called
 * each frame with two persistent caller flags: *pConfirm (the confirm/cancel
 * result) and *pAdjustable (set when the pending purchase is a quantity-
 * adjustable ammo refill). The active phase lives at g_vendorUi+0x84:
 *   phase 1 (validate): price the selection, seed g_nVendorBuyQuantity from the
 *       bolt wallet (clamped to the weapon's remaining ammo room), reject an
 *       already-full ammo gauge or an unaffordable pick (deny sound), mark
 *       *pAdjustable for an affordable ammo purchase, then advance to phase 2.
 *   phase 2 (quantity/confirm): while the purchase is quantity-adjustable, the
 *       up/down buttons (fresh press or the held auto-repeat cadence) raise/
 *       lower the buy quantity within [1, min(affordable, ammo room)]; a fresh
 *       confirm (0x40) or cancel (0x10) press latches *pConfirm and advances to
 *       phase 4.
 *   phase 4 (apply): on cancel just play the deny sound; on confirm, deduct the
 *       bolts and either refill ammo (func_0026F748) or grant the item/upgrade
 *       (GiveInventoryItem + optional stat/ammo refill + UpgradeWeaponToMax),
 *       rebuild the vendor list, refresh the caption, and play the success sound.
 *       Every terminal path resets the phase to 0 and calls RefreshVendorSelection.
 *
 * The item id -> stat-level classification for the upgrade ammo refill is a
 * 42-entry jump table (jtbl_0026D920, index = id - 0xC) that selects the current
 * tier (level 1) only for id classes {0,2,5,6,41} and the next tier (level 2)
 * otherwise — the same set GetVendorItemPrice uses (a distinct table with the
 * same mapping). Engine-region (ee-gcc 2.96) — matching-walled; portable #else
 * body. Field offsets kept literal (partial VendorUiState). */
/* RESIDUAL CLASS (task #576): SPLIT-WITHIN-FUNCTION -- g_boltCount, g_nVendorBuyQuantity
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 45.67%, engine96 36.16%
 *   (better arm: sdk29). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   the named symbol is reached BOTH ways inside this one function -- absolute
 *   %hi/%lo AND %gp_rel, same address. The split is POSITIONAL, not a size
 *   class (FACT #8058): every %gp_rel ref sits in a branch delay slot and every
 *   absolute ref outside one, 0 exceptions over all 122 compiled USA functions
 *   that split a symbol this way. No `.extern` size can express that; an
 *   assembler-side delay-slot rule could, and tools/ee has none yet.
 *   Unreachable today, but not proven a wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", VendorPurchaseStateMachine);
#else
extern s32   g_boltCount;          /* 0x1A7A00 player bolt wallet */
extern s32   g_weaponAmmo[];       /* per-item current ammo, indexed by item id */
extern u8    g_nVendorBuyQuantity[]; /* +0x0 = the pending buy quantity (scalar) */
extern s32   PlayMobySound(s32 soundIdx, s32 flags, void *owner);
extern void  BuildVendorItemList(s32 arg);
extern void  GiveInventoryItem(s32 itemId);
extern void  func_002AE6C8(s32 itemId);        /* post-grant item hook (skipped for 0x1F/0x3D) */
extern void  UpgradeWeaponToMax(s32 itemId);   /* apply a bought weapon upgrade */
extern void  func_0028AB70(s32 itemId);        /* refresh the granted item's UI state */
extern void  func_0028C108(s32 handle, s32 arg); /* release a menu sound/anim handle */
extern void  func_0029C4C0(s32 itemId, s32 flags);
extern void  RefreshVendorSelection(void);
extern s32   func_0026F748(s32 itemId, s32 qty); /* add ammo; returns the amount NOT delivered */
extern void *GetLocalizedString(s32 stringId);

/* Auto-repeat gate for holding the quantity up/down button: no fire below 0x10,
   then every 8th tick, and continuously once past 0x30. */
static s32 VendorQtyRepeatFires(s32 holdTimer) {
    if (holdTimer < 0x10) {
        return 0;
    }
    if ((holdTimer & 7) == 0) {
        return 1;
    }
    return holdTimer >= 0x30;
}

/* The nav click plays on the leading edge (timer < 0x10) or on the coarse
   (timer % 4)==0 beat. */
static s32 VendorQtyRepeatClicks(s32 holdTimer) {
    return holdTimer < 0x10 || (holdTimer & 3) == 0;
}

/* Terminal tail shared by every phase-4 outcome: play the result sound on the
   preview moby, drop the transaction back to the idle phase, and rebuild. */
static void VendorPurchaseFinish(char *ui, s32 soundId) {
    PlayMobySound(soundId, 0x11, *(void **)(ui + 0x28));
    *(s32 *)(ui + 0x84) = 0;
    RefreshVendorSelection();
}

void VendorPurchaseStateMachine(s32 *pConfirm, s32 *pAdjustable) {
    char *ui   = (char *)&g_vendorUi;
    void *moby = *(void **)(ui + 0x28);
    s32   phase = *(s32 *)(ui + 0x84);

    switch (phase) {
    case 1: {
        /* ---- validate the pending purchase and seed the buy quantity ---- */
        s32   sel = *(s32 *)(ui + 0x80);
        char *slot = ui + sel * 0x18;
        s32   itemId = *(s32 *)(slot + 0x140);
        u8   *def = g_weaponTable + g_itemEquippedSlot[itemId] * 0xE0;
        u16   unitCost = *(u16 *)(def + 0x88);
        s32   itemType, ammoRoom;

        if (unitCost == 0) {
            *(s32 *)g_nVendorBuyQuantity = 0;
        } else {
            *(s32 *)g_nVendorBuyQuantity = g_boltCount / unitCost;
        }
        ammoRoom = *(u16 *)(def + 0x8E) - g_weaponAmmo[itemId];
        if (ammoRoom < *(s32 *)g_nVendorBuyQuantity) {
            *(s32 *)g_nVendorBuyQuantity = ammoRoom;
        }

        *pConfirm = 0;
        *pAdjustable = 0;
        itemType = *(s32 *)(slot + 0x144);

        if (itemType == 1 && g_weaponAmmo[itemId] >= *(u16 *)(def + 0x8E)) {
            PlayMobySound(2, 0x11, moby);            /* ammo gauge already full */
        } else if (itemType == 1) {
            if (g_boltCount < *(u16 *)(def + 0x88)) {
                PlayMobySound(2, 0x11, moby);        /* can't afford one unit */
            } else {
                *pAdjustable = 1;                    /* quantity-adjustable */
            }
        } else if (g_boltCount < GetVendorItemPrice()) {
            PlayMobySound(2, 0, moby);               /* can't afford the item */
        }
        *(s32 *)(ui + 0x84) = 2;
        break;
    }

    case 2: {
        /* ---- adjust quantity (ammo purchases), then confirm/cancel ---- */
        s32 btn = *(s32 *)(D_138180 + 0x1C4);

        if (*pAdjustable == 1) {
            s32   sel = *(s32 *)(ui + 0x80);
            char *slot = ui + sel * 0x18;
            s32   downTimer = *(s32 *)((char *)&g_tieVramLruSize + 0x54);
            s32   upTimer   = *(s32 *)((char *)&g_tieVramLruSize + 0x58);

            /* DOWN: decrement, never below 1 */
            if ((btn & 0x8000) || VendorQtyRepeatFires(downTimer)) {
                s32 q = *(s32 *)g_nVendorBuyQuantity;
                if (q >= 2) {
                    *(s32 *)g_nVendorBuyQuantity = q - 1;
                    if (VendorQtyRepeatClicks(downTimer)) {
                        PlayMobySound(1, 0x11, moby);
                    }
                }
            }

            /* UP: increment, clamped to whichever runs out first — bolts or ammo room */
            if ((btn & 0x2000) || VendorQtyRepeatFires(upTimer)) {
                s32 itemId = *(s32 *)(slot + 0x140);
                u8 *def = g_weaponTable + g_itemEquippedSlot[itemId] * 0xE0;
                u16 unitCost = *(u16 *)(def + 0x88);
                if (unitCost != 0) {
                    s32 maxAffordable = g_boltCount / unitCost;
                    s32 ammoRoom = *(u16 *)(def + 0x8E) - g_weaponAmmo[itemId];
                    s32 limit = (ammoRoom < maxAffordable) ? ammoRoom : maxAffordable;
                    s32 q = *(s32 *)g_nVendorBuyQuantity;
                    if (q < limit) {
                        *(s32 *)g_nVendorBuyQuantity = q + 1;
                        if (VendorQtyRepeatClicks(upTimer)) {
                            PlayMobySound(1, 0x11, moby);
                        }
                    }
                }
            }
        }

        /* CANCEL (0x10) or CONFIRM (0x40), each on a fresh press, latch the
           result and advance to the apply phase. */
        {
            s32 held = *(s32 *)((char *)&g_tieVramLruSize + 0x5C);
            if ((btn & 0x10) && !(held & 0x10)) {
                *pConfirm = 0;
                *(s32 *)(ui + 0x84) = 4;
            } else if ((btn & 0x40) && !(held & 0x40)) {
                *pConfirm = 1;
                *(s32 *)(ui + 0x84) = 4;
            }
        }
        break;
    }

    case 4: {
        /* ---- apply the latched transaction ---- */
        s32   sel, itemType, cost;
        char *slot;

        if (*pConfirm != 1) {
            VendorPurchaseFinish(ui, 0);             /* cancelled */
            break;
        }

        sel = *(s32 *)(ui + 0x80);
        slot = ui + sel * 0x18;
        itemType = *(s32 *)(slot + 0x144);

        if (itemType == 1) {
            u8 *def = g_weaponTable + g_itemEquippedSlot[*(s32 *)(slot + 0x140)] * 0xE0;
            cost = *(u16 *)(def + 0x88) * *(s32 *)g_nVendorBuyQuantity;
        } else {
            cost = GetVendorItemPrice();
        }

        /* Can't afford: a full ammo gauge denies outright (no caption); a
           non-full gauge falls through to the typed handler, which re-checks
           and denies with a caption. */
        if (g_boltCount < cost) {
            s32 curId = *(s32 *)(slot + 0x140);
            u8 *curDef = g_weaponTable + g_itemEquippedSlot[curId] * 0xE0;
            if (g_weaponAmmo[curId] >= *(u16 *)(curDef + 0x8E)) {
                VendorPurchaseFinish(ui, 0);
                break;
            }
        }

        if (itemType == 1) {
            /* ---- ammo refill purchase ---- */
            s32 itemId = *(s32 *)(slot + 0x140);
            u8 *def = g_weaponTable + g_itemEquippedSlot[itemId] * 0xE0;
            s32 maxAmmo = *(u16 *)(def + 0x8E);
            s32 buyQty, given;
            u16 unitCost;

            if (g_weaponAmmo[itemId] >= maxAmmo) {   /* already full */
                VendorPurchaseFinish(ui, 0);
                break;
            }
            buyQty = *(s32 *)g_nVendorBuyQuantity;
            unitCost = *(u16 *)(def + 0x88);
            if (g_boltCount < unitCost * buyQty) {   /* can't afford the batch */
                SetVendorCaption((s32)GetLocalizedString(0x2C62));
                VendorPurchaseFinish(ui, 0);
                break;
            }
            if (maxAmmo < g_weaponAmmo[itemId]) {    /* clamp an over-full gauge */
                g_weaponAmmo[itemId] = maxAmmo;
            }
            buyQty = *(s32 *)g_nVendorBuyQuantity;
            given = func_0026F748(itemId, buyQty);   /* returns the undelivered remainder */
            g_boltCount -= *(u16 *)(def + 0x88) * (buyQty - given);
            BuildVendorItemList(0);
            SetVendorCaption((s32)GetLocalizedString(0x2C61));
            VendorPurchaseFinish(ui, 7);
            break;
        } else {
            /* ---- one-shot item / weapon-upgrade purchase ---- */
            s32 price = GetVendorItemPrice();
            s32 itemId;

            if (g_boltCount < price) {
                SetVendorCaption((s32)GetLocalizedString(0x2C62));
                VendorPurchaseFinish(ui, 0);
                break;
            }

            itemId = *(s32 *)(slot + 0x140);
            GiveInventoryItem(itemId);

            if (*(s32 *)(slot + 0x154) != 0) {
                /* refill the bought upgrade's ammo to its tier default */
                u8  statBlock[0xE0];
                s32 index = itemId - 0xC;
                s32 level = 2;
                if ((u32)index < 0x2A) {
                    switch (index) {
                    case 0: case 2: case 5: case 6: case 41:
                        level = 1;
                        break;
                    }
                }
                if (GetWeaponStatsAtLevel(statBlock, itemId, level) != 0 &&
                    *(u16 *)(statBlock + 0x88) != 0) {
                    g_weaponAmmo[itemId] = *(u16 *)(statBlock + 0x8E);
                }
            }

            if (itemId != 0x1F && itemId != 0x3D) {
                func_002AE6C8(itemId);
            }
            *(s32 *)(ui + 0x68) = itemId;
            if (*(s32 *)(slot + 0x154) != 0) {
                UpgradeWeaponToMax(itemId);
            }
            func_0028AB70(itemId);
            g_boltCount -= price;
            BuildVendorItemList(0);

            if (*(s32 *)(ui + 0x78) != -1) {
                func_0028C108(*(s32 *)(ui + 0x78), 0);
            }
            *(s32 *)(ui + 0x78) = -1;

            if (*(s32 *)(slot + 0x144) == 1) {
                func_0029C4C0(*(s32 *)(slot + 0x140), 0x112A880);
            } else {
                s32 lastIdx = *(s32 *)(ui + 0x740) - 1;
                if (lastIdx < sel) {
                    *(s32 *)(ui + 0x80) = lastIdx;
                }
            }
            SetVendorCaption((s32)GetLocalizedString(0x2C61));
            VendorPurchaseFinish(ui, 7);
            break;
        }
    }

    default:
        break;
    }
}
#endif

/* ExitVendorMenu [SEEDABLE] — close the Gadgetron vendor shop and restore the
 * world/camera/player display.
 *
 * Quiesces the shop (func_002FCFC8), plays the close sound on the preview moby
 * (+0x28) and — when a camera override is active (+0x60 != 0) — restores the saved
 * camera transform (+0xE0 -> g_cameraRot) and its parameter block
 * (g_nVendorBuyQuantity+0x8 -> g_cameraRot+0x220). Flags the preview moby's +0x34
 * status word (|= 0x100), sets the UI phase (+0x0 = 2), and resets the vendor
 * buy-quantity/zoom scratch (g_nVendorBuyQuantity +0x3C/+0x40/+0x44 and D_1AD2B0/B4/B8).
 * Then, per the active tab (+0x50):
 *   - tab 3 (armor): pick the armor id (D_1AD238 ? D_1AD23C : D_1AD240), equip and
 *     reload the player model (LoadPlayerDisplayModel); if a selection was latched
 *     (D_1AD238 != 0) refresh the armor's caption (GetLocalizedString/func_0029DAD0
 *     over the D_1AD388 string-id table).
 *   - tab 2 (ship): if the ship-customization id changed, reload the ship model
 *     (LoadShipDisplayModel) and fix up its display record; reload the ship texture
 *     when the palette id (D_1A7AFA & 0x1F) changed; then func_002E6FB0(0).
 *
 * Engine-region (ee-gcc 2.96): the lq/sq and ldl/ldr are plain block copies and
 * every model/texture load is a callee, so the body ports faithfully as portable
 * C. Matching-walled — kept as the portable #else impl. Field offsets kept literal. */
/* RESIDUAL CLASS (task #576): UNDIAGNOSED
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 41.61%, engine96 45.37%
 *   (better arm: engine96). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   SCREENED ONLY. Both arms were measured; the residual was not diagnosed to a
 *   mechanism. This is an open arm, not a wall -- do not read it as one. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", ExitVendorMenu);
#else
extern u8   g_cameraRot[];           /* saved world-camera transform block */
extern u8   g_nVendorBuyQuantity[];  /* vendor buy-quantity + camera param scratch */
extern s32  D_1AD2B0;                /* zoom/lerp scratch (cleared) */
extern f32  D_1AD2B4;                /* zoom rate (1.0) */
extern f32  D_1AD2B8;                /* zoom rate (1.1) */
extern u16  D_1AD23C;                /* armor id when a selection is latched */
extern s32  D_1AD388[5];             /* per-armor caption string-id table */
extern s16  g_equippedArmor;         /* current player armor tier */
extern u8   g_levelDialogToc[];      /* +0x13B0 = ship display record */
extern s32  g_shipCustomization;     /* selected ship-customization id */
extern u16  D_1A7AFA;                /* ship palette / texture selector */
extern s32  PlayMobySound(s32 soundIdx, s32 flags, void *owner);
extern void func_002FCFC8(void);     /* tear down the vendor shop scene */
extern void LoadPlayerDisplayModel(s32 armorTier);
extern void *GetLocalizedString(s32 stringId);
extern void func_0029DAD0(void *str, s32 arg);
extern void LoadShipDisplayModel(s32 shipVariant);
extern void LoadShipDisplayTexture(s32 paletteId);
extern void func_002E6FB0(s32 arg);

void ExitVendorMenu(void) {
    char *ui = (char *)&g_vendorUi;
    char *moby;

    func_002FCFC8();
    PlayMobySound(5, 0x11, *(void **)(ui + 0x28));

    if (*(s32 *)(ui + 0x60) != 0) {
        memcpy(g_cameraRot, ui + 0xE0, 0x10);
        memcpy(g_cameraRot + 0x220, g_nVendorBuyQuantity + 0x8, 0x30);
    }

    moby = *(char **)(ui + 0x28);
    *(u16 *)(moby + 0x34) |= 0x100;

    *(s32 *)(ui + 0x0) = 2;
    D_1AD2B4 = 1.0f;
    D_1AD2B8 = 1.1f;
    *(s32 *)(g_nVendorBuyQuantity + 0x40) = 0x60;
    *(s32 *)(g_nVendorBuyQuantity + 0x44) = 0x55;
    D_1AD2B0 = 0;
    *(s32 *)(g_nVendorBuyQuantity + 0x48) = 0;
    *(s32 *)(g_nVendorBuyQuantity + 0x4C) = 0;
    *(s32 *)(g_nVendorBuyQuantity + 0x50) = 0;
    *(s32 *)(g_nVendorBuyQuantity + 0x3C) = 0x60;

    if (*(s32 *)(ui + 0x50) == 3) {
        s32 armorSel = (D_1AD238 != 0) ? (s32)D_1AD23C : (s32)(u16)D_1AD240;
        s32 nameTable[5];

        g_equippedArmor = (s16)armorSel;
        LoadPlayerDisplayModel(g_equippedArmor);
        memcpy(nameTable, D_1AD388, sizeof(nameTable));
        if (D_1AD238 != 0) {
            void *str = GetLocalizedString(nameTable[g_equippedArmor]);
            func_0029DAD0(str, -1);
        }
    }

    if (*(s32 *)(ui + 0x50) == 2) {
        char *base = (char *)(g_levelDialogToc + 0x13B0);
        s32   shipCust = g_shipCustomization;
        s32   paletteId;

        if (*(s32 *)(base + 0x18) != shipCust) {
            char *entry;
            LoadShipDisplayModel(shipCust & 3);
            entry = *(char **)(base + 0x0);
            if (entry != NULL && *(s32 *)(entry + 0x98) != 0) {
                char *sub = *(char **)(entry + 0x24);
                *(s32 *)(entry + 0x98) = *(s32 *)(sub + 0x10);
            }
        }
        paletteId = D_1A7AFA & 0x1F;
        if (*(s32 *)(base + 0x30) != paletteId) {
            LoadShipDisplayTexture(paletteId);
        }
        func_002E6FB0(0);
    }
}
#endif

/* TODO(hle): needs PS2 graphics/IO HLE backend - vendor-input frame-stack sliver (spimdisasm fragment). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002FA238);

/* UpdateVendorMenuInput [SEEDABLE] — per-frame D-pad/button handler for the
 * Gadgetron vendor shop list. It reads the new-press button word (D_138180+0x1C4)
 * plus the held-button word and per-direction auto-repeat hold timers, and:
 *   - UP   (0x2000): move the selection up one row (index +0x80, live count +0x740),
 *                    advancing the smooth-scroll offset +0x6C by +0x38 (gated <0x39);
 *   - DOWN (0x8000): move down one row (offset stays >= -0x38, index stays > 0);
 *   - CONFIRM (0x40): try to buy the selected slot (see VendorTryPurchase) — on
 *                     success latch +0x84 and play the buy sound, else the deny sound;
 *   - EXIT (0x10)   : leave the shop (ExitVendorMenu).
 * A direction fires on a fresh press, or (while held) whenever its hold timer has
 * passed 0x10 and lands on (timer % 10) == 1. Early-outs if a purchase is already
 * latched this frame (+0x84). Engine-region (ee-gcc 2.96) — matching-walled;
 * portable #else body. Field offsets kept literal (partial VendorUiState). */
/* RESIDUAL CLASS (task #576): SPLIT-WITHIN-FUNCTION -- g_boltCount
 *   Both arms measured at unit objdiff (objdiff_build.sh + unit_report.sh,
 *   whole-unit blanket screen, clean tree): sdk29 25.74%, engine96 28.07%
 *   (better arm: engine96). Neither reaches 100.00%, so this stays INCLUDE_ASM
 *   and the #else below remains the portable impl.
 *   the named symbol is reached BOTH ways inside this one function -- absolute
 *   %hi/%lo AND %gp_rel, same address. The split is POSITIONAL, not a size
 *   class (FACT #8058): every %gp_rel ref sits in a branch delay slot and every
 *   absolute ref outside one, 0 exceptions over all 122 compiled USA functions
 *   that split a symbol this way. No `.extern` size can express that; an
 *   assembler-side delay-slot rule could, and tools/ee has none yet.
 *   Unreachable today, but not proven a wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", UpdateVendorMenuInput);
#else
extern s32  g_boltCount;      /* 0x1A7A00 player bolt wallet */
extern s32  g_weaponAmmo[];   /* per-item current ammo, indexed by item id */
extern s32  D_001A81C0;       /* UP   auto-repeat hold timer (pad state +0x58) */
extern s32  D_001A81BC;       /* DOWN auto-repeat hold timer (pad state +0x54) */
extern s32  D_001A81C4;       /* currently-held button mask (pad state +0x5C) */
extern s32  PlayMobySound(s32 soundIdx, s32 flags, void *owner);
extern void func_0028C108(s32 handle, s32 arg); /* release a menu sound/anim handle */
extern void ExitVendorMenu(void);

/* A direction acts on a fresh press, or on the auto-repeat cadence while held. */
static s32 VendorDirFires(s32 pressed, s32 holdTimer) {
    if (pressed) {
        return 1;
    }
    if (holdTimer < 0x10) {
        return 0;
    }
    return (holdTimer % 10) == 1;
}

/* Shared tail after a successful up/down move: refresh the preview highlight,
 * step the scroll offset, play the nav sound, and drop the old preview handle. */
static void VendorNavCommit(char *ui, s32 offsetDelta) {
    void *moby = *(void **)(ui + 0x28);
    if (*(s32 *)(ui + 0x88) != 0) {
        *(s32 *)(ui + 0x90) = (*(s32 *)(ui + 0x50) != 2) ? 1 : 3;
    }
    *(s32 *)(ui + 0x6C) += offsetDelta;
    PlayMobySound(1, 0x11, moby);
    if (*(s32 *)(ui + 0x78) != -1 && *(s32 *)(ui + 0x50) == 0) {
        func_0028C108(*(s32 *)(ui + 0x78), 0);
    }
    *(s32 *)(ui + 0x78) = -1;
}

/* Outcome of a CONFIRM press on the selected slot:
 *   1 = purchase succeeds, 0 = denied (play deny sound), -1 = ignore (no sound). */
static s32 VendorTryPurchase(char *ui) {
    s32 sel = *(s32 *)(ui + 0x80);
    s32 tab = *(s32 *)(ui + 0x50);
    s32 kind;

    if (tab == 3) {
        /* upgrade tab: only act when the selected 0xC-stride entry is live */
        if (*(s32 *)(ui + sel * 0xC + 0x144) == 0) {
            return -1;
        }
    }
    if (tab != 0) {
        return 1; /* non-shop tabs commit unconditionally */
    }

    kind = *(s32 *)(ui + 0x144 + sel * 0x18);
    if (kind == 0) {
        /* weapon/upgrade slot: affordable at the computed price? */
        if (g_boltCount >= GetVendorItemPrice()) {
            return 1;
        }
        kind = *(s32 *)(ui + 0x144 + sel * 0x18); /* reloaded (still 0) -> deny below */
    }
    if (kind != 1) {
        return 0; /* not a refillable ammo slot -> deny */
    }
    {
        /* ammo slot: must be below max and the refill must be affordable */
        s32 itemId  = *(s32 *)(ui + sel * 0x18 + 0x140);
        s32 variant = g_itemEquippedSlot[itemId];
        u8 *def     = g_weaponTable + variant * 0xE0;
        if (g_weaponAmmo[itemId] >= *(u16 *)(def + 0x8E)) {
            return 0; /* already full */
        }
        if (g_boltCount < *(u16 *)(def + 0x88)) {
            return 0; /* can't afford a refill */
        }
        return 1;
    }
}

void UpdateVendorMenuInput(void) {
    char *ui = (char *)&g_vendorUi;

    if (*(s32 *)(ui + 0x84) != 0) {
        return; /* a purchase is already latched this frame */
    }

    /* ---- UP: select previous row (higher index) ---- */
    if (VendorDirFires(*(s32 *)(D_138180 + 0x1C4) & 0x2000, D_001A81C0)) {
        if (*(s32 *)(ui + 0x6C) < 0x39) {
            s32 sel = *(s32 *)(ui + 0x80);
            if (sel < *(s32 *)(ui + 0x740) - 1) {
                *(s32 *)(ui + 0x80) = sel + 1;
                VendorNavCommit(ui, 0x38);
            }
        }
    }

    /* ---- DOWN: select next row (lower index) ---- */
    if (VendorDirFires(*(s32 *)(D_138180 + 0x1C4) & 0x8000, D_001A81BC)) {
        if (*(s32 *)(ui + 0x6C) >= -0x38) {
            s32 sel = *(s32 *)(ui + 0x80);
            if (sel > 0) {
                *(s32 *)(ui + 0x80) = sel - 1;
                VendorNavCommit(ui, -0x38);
            }
        }
    }

    /* ---- CONFIRM: buy the selected slot (ignored while the button is held) ---- */
    if ((*(s32 *)(D_138180 + 0x1C4) & 0x40) && !(D_001A81C4 & 0x40)) {
        void *moby = *(void **)(ui + 0x28);
        s32 outcome = VendorTryPurchase(ui);
        if (outcome == 1) {
            if (*(s32 *)(ui + 0x88) != 0) {
                *(s32 *)(ui + 0x90) = 1;
            }
            *(s32 *)(ui + 0x84) = 1; /* latch: block further input until consumed */
            PlayMobySound(0, 0x11, moby);
        } else if (outcome == 0) {
            PlayMobySound(2, 0x11, moby);
        }
    }

    /* ---- EXIT ---- */
    if (*(s32 *)(D_138180 + 0x1C4) & 0x10) {
        ExitVendorMenu();
    }
}
#endif

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1EFFC0", func_002FA624);

/* The unit tail (0x2FA628..0x2FFC1F) is the spimdisasm c-mode tail-fusion blob:
 * functions reached only by j / data-ref (no jal) that spimdisasm cannot promote
 * to their own symbols, so they fuse into one INCLUDE_ASM. It is islanded as the
 * asm segment text/1FA5A8 to keep the objdiff count honest. */

#include "common.h"

/*
 * text/250080 — FMV (movie) playback engine, the very tail of .text
 * (carve-pipeline pick #2, 2026-06-12; vaddr 0x350100..0x352D07). IPU
 * bitstream decode thread + GIF/vblank frame display + the small ring/queue
 * helpers they share. InitFmvPlaybackEngine / OnFmvVblankFlip /
 * OnFmvGifDmaInterrupt were Track-B-named earlier; everything operates on
 * the FMV memory arena at g_pFmvArenaBase (pts ring +0xD9100, DMA-add queue
 * +0xD9090, frame queue +0xD9168, stream object +0xD9048).
 *
 * The matcher builds THIS unit at -O2 -G8 -fno-gcse (per-unit GFLAG/CC1EXTRA
 * override in tools/ee/objdiff_build.sh / diff.sh / build.sh) — same
 * later-SN-cc1 TU model as the other gameplay-text units.
 *
 * -G8 extern-sizing rules (same as text/1907F0 / text/198FA0):
 *   - a complete extern object of size <= 8 bytes is placed in small data
 *     (gp-relative access);
 *   - an object the original reads with the ADJACENT lui/%lo "assembler
 *     macro" shape (dest-reg loads, $at stores) is declared as a small
 *     scalar/pointer PLUS a file-scope `.extern sym,16` override: cc1 emits
 *     the one-insn symbolic macro and GNU as expands it absolutely.
 *
 * SAVE-LAYOUT WALL (same as text/1907F0): this TU was built by the later SN
 * cc1 that packs callee-save slots 8-byte (sd s0@sp+0x0, ra@sp+0x8); the
 * pinned cc1 reserves 16 bytes per save. Every function below that saves two
 * or more GPRs (incl. $ra) is byte-identical except those slot offsets and
 * stays INCLUDE_ASM, marked "8-byte-packed saves". $ra-only functions are
 * unaffected.
 */

/* cc1-small / assembler-absolute symbols (see header). */
__asm__(".extern g_pFmvArenaBase, 16");
__asm__(".extern D_1AE788, 16");
__asm__(".extern D_1AE78C, 16");
__asm__(".extern D_1AE790, 16");

/* The FMV arena base pointer (0x1B234C). Declared as a small pointer so cc1
 * schedules the load as one insn; expanded absolutely by the assembler. */
extern u8 *g_pFmvArenaBase;
extern s32 D_1AE788; /* vblank field-flip "frame displayed" latch */
extern s32 D_1AE78C; /* cleared with D_1AE788 by the vblank waiter */
extern s32 D_1AE790; /* GIF-DMA "frame consumed" pending flag */

/* Debug printf-stub format strings (rodata). */
extern char D_1AE7D8[]; /* "[ Error ] %s\n" */
extern char D_1AE860[]; /* frame-drop diagnostic */

/* Fixed offsets of the FMV engine blocks inside the arena (the bitstream
 * ring itself lives at arena+0, header after its 0x50000-byte payload). */
#define FMV_STREAM_OFS 0xD9048 /* bitstream/stream-state object */
#define FMV_DMAQ_OFS 0xD9090   /* IPU DMA-add queue */
#define FMV_PTS_OFS 0xD9100    /* pts queue control block */
#define FMV_FRAMEQ_OFS 0xD9168 /* decoded-frame display queue */

/*
 * The pts ring: 0x50000 bytes of payload followed by its header — so the
 * header fields sit at +0x50000/+0x50004/+0x50008 from the ring base, which
 * is exactly how every helper addresses them (base reg + 0x50000).
 */
typedef struct FmvPtsRing {
    /* 0x00000 */ u8 data[0x50000];
    /* 0x50000 */ s32 readOfs; /* consume position inside data[] */
    /* 0x50004 */ s32 fill;    /* bytes currently queued */
    /* 0x50008 */ s32 size;    /* capacity (0x50000) */
} FmvPtsRing;

/*
 * The decoded-frame display queue: two parallel arrays — GS frame buffers
 * (0xD0000 apart) and decoded-frame slots (0x138C0 apart, header {state,
 * index} at the front of each slot).
 */
typedef struct FmvFrameQueue {
    /* 0x00 */ u8 *gsFrames;  /* GS-side frame area (0xD0000 stride) */
    /* 0x04 */ u8 *frames;    /* decoded-frame slots (0x138C0 stride) */
    /* 0x08 */ s32 writeIdx;  /* next slot to fill */
    /* 0x0C */ s32 count;     /* frames queued for display */
    /* 0x10 */ s32 capacity;  /* number of slots */
} FmvFrameQueue;

/* The pts queue control block (arena+0xD9100): a started flag and the
 * elementary-stream cursor state used by the header/payload walker. */
typedef struct FmvPtsQueue {
    /* 0x00 */ s32 started;   /* header consumed / stream live */
    /* 0x04 */ s32 mode;      /* stream type (4 = headerless) */
    /* 0x08 */ u8 hdr[0x28];  /* buffered packet header bytes */
    /* 0x30 */ s32 hdrFill;   /* header bytes buffered so far */
    /* 0x34 */ u8 *dataPtr;   /* payload ring base */
    /* 0x38 */ s32 ringOfs;   /* payload write offset (wraps) */
    /* 0x3C */ s32 consumed;  /* payload bytes consumed */
    /* 0x40 */ s32 ringSize;  /* payload ring size (1KB granular) */
    /* 0x44 */ s32 received;  /* payload bytes received in total */
    /* 0x48 */ u8 pad48[0x8];
    /* 0x50 */ s32 queued;    /* bytes queued for the IPU */
} FmvPtsQueue;

/* Generic FMV object handle (state word at +0xA8 = the playback FSM). */
typedef struct FmvStream {
    /* 0x00 */ u8 pad[0xA8];
    /* 0xA8 */ s32 state;
} FmvStream;

extern s32 func_0011AAD0(s32 count);           /* RotateThreadReadyQueue */
extern s32 func_00351910(void *dmaq);
extern s32 func_00350830(FmvPtsQueue *q);
extern s32 func_00350840(FmvPtsQueue *q);
/* func_00350910 reads $a0 as the stream-state ptr and returns nothing (void).
   The matching build keeps the original `(void)`-shaped forward decl so the
   byte-exact forwarder func_00350840 below (`return func_00350910()`) compiles
   identically; the native build uses the real `void(s32*)` signature. */
#ifndef TARGET_NATIVE
extern s32 func_00350910(void);
#else
void func_00350910(s32 *st);
#endif
extern s32 func_001338C8(void);
extern s32 func_0012EE28(void);
#ifndef TARGET_NATIVE
extern s32 func_003517C0(void *stream);
extern s32 func_003518B8(void *stream);
#endif
extern s32 func_00351FB0(void *stream);
extern s32 func_00351B10(void *dmaq);
extern s32 func_00351C20(void *dmaq);
extern void func_003521B0(void *dmaq, void *cmd);
extern s32 DebugPrintStub(char *fmt, ...);

/* Helpers referenced by the native (#else) FMV ring orchestration bodies. */
void func_003506A8(FmvPtsQueue *q, u8 **pPtr0, s32 *pLen0, u8 **pPtr1, s32 *pLen1);
s32 func_003511A8(u8 *dst0, s32 len0d, u8 *dst1, s32 cap, u8 *src0, s32 len1d,
                  u8 *src1, s32 tail);
s32 func_00350778(FmvPtsQueue *q, s32 n);
#ifdef TARGET_NATIVE
/* Globals + helpers referenced only by the native (#else) bodies below. */
extern u8 *D_1B2354;  /* IPU decode output scratch buffer (0x1B2354) */
extern char D_1AE7E8[];   /* DMA-add-queue-full error string */
extern char D_1AE820[];   /* decode-thread stop diagnostic */
extern char D_1AE838[];   /* host frame-read error string */
extern s32 func_00350868(u8 *stream, u8 *src, s32 len, s32 dstOfs);
/* func_003517C0/func_003518B8: deferred-native FMV stream funcs whose #else bodies
   model them with inconsistent arg counts across call sites (true signatures need the
   asm; FMV native backend is deferred). Declared with unspecified args so the corpus
   compiles; resolve when the FMV native path is built. */
extern s32 func_003517C0();
extern s32 func_003518B8();
extern s32 func_00352638(u8 *obj, u64 a, u64 b, s32 pos, s32 n);
extern void ZeroQwords(void *p, s32 n);
extern s64 func_00133850(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern void *func_0012F738(void);
extern s32 func_0012FA70(u8 *obj, s32 slot, void *cb, s32 arg);
extern void func_003525D0(FmvStream *s);
extern s32 func_003515E8(u8 *obj, u64 a, u64 b, u64 c, u64 d, u64 e);
extern s32 func_0012F9A8(u8 *obj);
extern void func_0012F9C8(u8 *obj);
extern s32 func_0012F950(u8 *obj, s32 ptr, s32 len);
extern s32 func_00352C30(u8 *fq);
extern void func_00352BB8(u8 *fq);
extern void func_00350B60(void *gif, u8 *frame, u32 a, u32 b, s32 idx);
extern void WaitSema(s32 sema);
extern void SignalSema(s32 sema);
extern s32 func_00133960(void);
/* Stream-event callbacks registered by func_00352468 (defined later). */
s32 func_00352A20(s32 unused, s32 *frame);
s32 func_00352A48(void);
s32 func_00352A80(void);
s32 func_00352AB0(void);
extern s32 func_00352AE0(u8 *obj);
#endif

/**
 * Yield the FMV thread for one scheduler rotation (RotateThreadReadyQueue
 * on priority 1, the FMV decode thread's priority). The empty asm guard
 * keeps cc1 from sibling-call optimising the void tail call (the original
 * calls and returns).
 */
void func_00350100(void) {
    func_0011AAD0(1);
    __asm__ __volatile__("");
}

/**
 * Poll whether the pts ring holds at least one decodable unit (forwards to
 * the >=0x1000-bytes-queued check on the arena's pts ring).
 */
s32 func_00350120(void) {
    return func_00350830((FmvPtsQueue *)(g_pFmvArenaBase + FMV_PTS_OFS));
}

/* TODO(hle): needs PS2 graphics/IO HLE backend — FMV engine bring-up programs
   the DMAC directly (REG_DMAC_CTRL/STAT), spawns the IPU decode thread, and
   installs the vblank-start + DMAC-channel-2 interrupt handlers. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", InitFmvPlaybackEngine);

/* func_003503D8: FMV engine shutdown (kills the IPU thread, restores DMAC).
 * Blocked: the original mixes %gp_rel and absolute %hi/%lo accesses to
 * g_pFmvArenaBase/g_fmvThreadId inside one function (the delay-slot gp_rel
 * reload artifact) — only one form is expressible per declaration. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003503D8);

/**
 * Print an FMV error message through the debug stub ("[ Error ] %s\n").
 */
s32 func_003504C8(char *msg) {
    return DebugPrintStub(D_1AE7D8, msg);
}

/**
 * Pump the pts-ring stall check on the arena's pts ring (wrapper used as a
 * callback).
 */
s32 func_003504F0(void) {
    return func_00350840((FmvPtsQueue *)(g_pFmvArenaBase + FMV_PTS_OFS));
}

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350520);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves. Revisit with the gameplay-TU compiler.

   Construct the pts-queue object: zero the 0x20-quadword header, stash the
   payload ring base (param2) and IPU scratch base (param3), seed the stream
   type (3) and the 0x400-byte ring granularity, record the decode output
   buffer in D_1B2354, allocate the IPU sema (func_00133850), and report
   whether the allocation succeeded. */
s32 func_00350520(s32 *obj, s32 ringBase, s32 scratchBase, u8 *decodeBuf) {
    s64 sema;

    ZeroQwords(obj + 2, 0x20);
    obj[0xD] = ringBase;
    obj[0x10] = scratchBase;
    obj[1] = 3;
    obj[0] = 0;
    obj[0xC] = 0;
    obj[0xE] = 0;
    obj[0xF] = 0;
    obj[0x11] = 0;
    obj[0x14] = 0;
    obj[0x16] = 0;
    obj[0x17] = 0;
    obj[0x18] = 0;
    D_1B2354 = decodeBuf;
    obj[0x13] = 0x400;
    sema = func_00133850(0x400, 0x1000, 0x400, 0, 5, 3);
    obj[0x12] = (s32)sema;
    return sema >= 0;
}
#endif

/**
 * Stream-callback: kick the CD/WAD reader pump and report ready.
 */
s32 func_003505E0(void) {
    func_001338C8();
    return 1;
}

/* func_00350600: 8 bytes of inter-function padding (addiu $sp,+0x10 / nop),
 * not compiler output — no C can produce it. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350600);

/* func_00350608: read-chunk dispatch on the stream object. Blocked:
 * 8-byte-packed saves (s0@0x0, ra@0x8; see header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350608);

/* func_00350660: stream-state reset. Blocked: 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350660);

/**
 * Compute the two writable spans of the pts payload ring (the producer's
 * scatter region). Returns (ptr,len) of the leading span through *pPtr0/*pLen0
 * and of the wrapped tail span through *pPtr1/*pLen1:
 *   - stream not started: either the buffered-header window (mode 4 = whole
 *     ring from dataPtr) or the 0x28-byte packet-header staging area at
 *     (q + hdrFill + 8);
 *   - stream live: the free region after ringOfs, split where it wraps.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003506A8);
#else
/* TODO(match): functional equivalent - not byte-exact; the original schedules
   the started/mode tests as branch-likely (bnel/beql) pairs the pinned cc1
   won't emit (40% best). Revisit with the gameplay-TU compiler. */
void func_003506A8(FmvPtsQueue *q, u8 **pPtr0, s32 *pLen0, u8 **pPtr1, s32 *pLen1) {
    if (q->started == 0) {
        if (q->mode == 4) {
            *pPtr0 = q->dataPtr;
            *pLen0 = q->ringSize;
            *pPtr1 = 0;
            *pLen1 = 0;
        } else {
            *pPtr0 = (u8 *)q + q->hdrFill + 8;
            *pLen0 = 0x28 - q->hdrFill;
            *pPtr1 = q->dataPtr;
            *pLen1 = q->ringSize;
        }
    } else {
        s32 ringSize = q->ringSize;
        s32 ringOfs = q->ringOfs;
        s32 untilEnd = ringSize - q->consumed;

        if (ringSize - ringOfs < untilEnd) {
            *pPtr0 = q->dataPtr + ringOfs;
            *pLen0 = ringSize - ringOfs;
            *pPtr1 = q->dataPtr;
            *pLen1 = untilEnd - (ringSize - ringOfs);
        } else {
            *pPtr0 = q->dataPtr + ringOfs;
            *pLen0 = untilEnd;
            *pPtr1 = 0;
            *pLen1 = 0;
        }
    }
}
#endif

/* func_00350778: account n arrived elementary-stream bytes (fill the
 * 0x28-byte packet header, then advance the payload ring cursors, ring
 * size re-snapped to 1KB). Best attempt 68%: identical structure but the
 * pinned cc1 colours the s/rem/take temporaries a3/t0/a1 where the
 * original has a2/a3/v0-with-copy - the register-coloring wall. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350778);

/**
 * True once the pts ring has at least 0x1000 bytes queued (enough for the
 * IPU to start decoding).
 */
s32 func_00350830(FmvPtsQueue *q) {
    return q->queued >= 0x1000;
}

/**
 * If the stream has been started (state word nonzero), pump the bitstream
 * feeder once. (Declared value-returning like the original: callers forward
 * its undefined result, which also keeps cc1 from sibling-call optimising
 * the forwarding tails.)
 */
#ifndef TARGET_NATIVE
s32 func_00350840(FmvPtsQueue *q) {
    if (q->started != 0) {
        return func_00350910();   /* byte-exact: jal then jr $31 forwarding $2 */
    }
}
#else
s32 func_00350840(FmvPtsQueue *q) {
    /* Native: func_00350910 is void and takes the stream-state ptr; pass q
       (the asm forwards its own $a0 unchanged). The original forwards the
       leftover $2; here that result is undefined, so return 0. */
    if (q->started != 0) {
        func_00350910((s32 *)q);
    }
    return 0;
}
#endif

/* func_00350868: SIF-DMA bounce of a decoded block to IOP memory. Blocked:
 * 8-byte-packed saves (s0/s1/s2/s3/ra). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350868);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350910);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0..s2/ra) plus several branch-likely div-by-zero guards. Revisit
   with the gameplay-TU compiler.

   Drain the elementary stream in 0x400-byte units: depending on the stream
   FSM state word (st[0]) decide how many ready bytes there are, then for each
   unit de-interleave st[6] macroblock rows of st[7] bytes out of the payload
   ring into the decode scratch (D_1B2354), inject the SCD/sequence markers at
   the 0xC00 boundary, and bounce each completed 0x400 block to IOP via
   func_00350868. Procedure (void): the asm reads $a0 as the stream-state ptr
   (daddu $16,$4,$0; lw 0x0($16)); its caller func_00350840 forwards its own
   $a0 unchanged and returns the leftover $2 (the "undefined result"). */
void func_00350910(s32 *st) {
    u32 avail = 0;
    s32 state = st[0];
    s32 rdy;

    if (state == 2) {
        avail = (func_00133960() - st[0x18]) & 0xFFF;
        rdy = (s32)avail < 0x400;
    } else if (state > 2) {
        if (state == 3) {
            return;
        }
        rdy = 0;
    } else if (state == 1) {
        if (st[0xF] < 0x1000) {
            return;
        }
        avail = 0x1000 - st[0x14];
        rdy = (s32)avail < 0x400;
    } else {
        rdy = 0;
    }

    if (!rdy && st[6] << 10 <= st[0xF]) {
        s32 mbCols = st[6];

        do {
            avail -= 0x400;
            if (mbCols > 0) {
                s32 col = 0;
                s32 srcBase = st[0xF];

                while (1) {
                    s32 ringSize = st[0x10];
                    s32 emitted = 0;
                    s32 rowBytes = st[7];
                    u8 *src = (u8 *)(st[0xD] + (st[0xE] - srcBase + ringSize) % ringSize +
                                     col * rowBytes);
                    u8 *dst = D_1B2354;

                    do {
                        s32 i;
                        for (i = 0; i < st[7]; i++) {
                            *dst++ = *src++;
                            emitted++;
                        }
                        rowBytes = st[7];
                        src += rowBytes * (st[6] - 1);
                    } while (emitted < 0x400);

                    if (st[0x18] == 0xC00) {
                        D_1B2354[0x3F1] = 3;
                    }
                    if (st[0x18] == 0) {
                        D_1B2354[0x11] = 2;
                        D_1B2354[1] = 6;
                    }
                    func_00350868((u8 *)st, D_1B2354, 0x400, col * 0x1000 + st[0x18]);
                    if (st[6] <= col + 1) {
                        break;
                    }
                    srcBase = st[0xF];
                    col++;
                }
            }
            {
                s32 step = st[0x18] + 0x400;
                s32 r = (step >= 0) ? step : step + 0x13FF;
                s32 left = st[0xF] - mbCols * 0x400;
                st[0x18] = step - (r >> 12) * 0x1000;
                st[0x14] += 0x400;
                st[0xF] = left;
                mbCols = st[6];
            }
        } while ((s32)avail >= 0x400 && mbCols * 0x400 <= st[0xF]);
    }
}
#endif

/* TODO(hle): needs PS2 graphics/IO HLE backend — builds the per-frame GIF
   packet chain (GIFtags + UNPACK/TRXPOS/TRXREG/TRXDIR) that uploads a decoded
   FMV frame to GS texture memory, then BeginFrameDrawList. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350B60);

/* TODO(hle): needs PS2 graphics/IO HLE backend — vblank-start handler: reads
   the GS CSR FIELD bit, kicks the prebuilt GIF display-transfer DMA chain on
   channel 2, and ends with the handwritten `sync.l; ei` pair. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", OnFmvVblankFlip);

/* OnFmvGifDmaInterrupt: DMAC ch2 handler — contains the handwritten `sync;
 * ei` interrupt-reenable pair, which no C source can produce. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", OnFmvGifDmaInterrupt);

/* func_00350F28: vblank field-sync waiter. Blocked: 8-byte-packed saves
 * (s0@0x0, ra@0x8) plus a delay-slot %gp_rel read of g_bProgressiveScan. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350F28);

/**
 * Clear the "frame displayed this vblank" latch.
 */
void func_00350F78(void) {
    D_1AE788 = 0;
}

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350F88);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0..s4/ra). Revisit with the gameplay-TU compiler.

   Copy a window of arrived bytes into the IPU payload ring's two writable
   spans: clamps the request to the descriptor's remaining length, queries the
   spans (func_003517C0 on the DMA-queue stream at arena+0xD9090, the two span
   pointers masked to the 0x20000000 scratch-pad address window), scatters into
   them (func_003511A8), records the resulting DMA-add command (func_00352638)
   reporting an error if it could not be queued, advances the read cursor
   (func_003525B0), and reports whether any bytes were stored. */
s32 func_00350F88(u8 *unused, u8 *desc, u8 *ringBase) {
    s32 span0Ptr;
    s32 span0Len;
    s32 span1Ptr;
    s32 span1Len;
    u8 *streamObj = g_pFmvArenaBase + FMV_STREAM_OFS;
    s32 start = *(s32 *)(desc + 8);
    s32 remain = *(s32 *)(desc + 0xC);
    s32 take = ((s32)(ringBase + *(s32 *)(ringBase + 0x50008)) - start);
    s32 stored;

    if (remain < take) {
        take = remain;
    }
    func_003517C0((s32 *)(streamObj + 0x48), &span0Ptr, &span0Len, &span1Ptr,
                  &span1Len);
    stored = func_003511A8((u8 *)((span0Ptr & 0xFFFFFFF) | 0x20000000), span0Len,
                           (u8 *)((span1Ptr & 0xFFFFFFF) | 0x20000000), span1Len,
                           (u8 *)start, take, ringBase, remain - take);
    if (stored > 0 &&
        func_00352638(streamObj, *(u64 *)(desc + 0x10), *(u64 *)(desc + 0x18),
                      span0Ptr, stored) == 0) {
        func_003504C8(D_1AE7E8);
    }
    func_003518B8(streamObj + 0x48, stored);
    return stored > 0;
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003510C0);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0..s4/ra). Revisit with the gameplay-TU compiler.

   Copy a window of arrived elementary-stream bytes into the pts ring: clamps
   the request to the ring's free header window and the descriptor's remaining
   length, queries the ring's two writable spans (func_003506A8), scatters into
   them (func_003511A8), advances the ring (func_00350778), and reports whether
   any bytes were stored. */
s32 func_003510C0(u8 *unused, u8 *desc, u8 *ringBase) {
    u8 *span0Ptr;
    s32 span0Len;
    u8 *span1Ptr;
    s32 span1Len;
    s32 ringEnd = *(s32 *)(ringBase + 0x50008);
    u32 wantEnd = *(s32 *)(desc + 8) + 4;
    u32 ringTop = (u32)(ringBase + ringEnd);
    s32 first;
    s32 remain;
    s32 stored;

    if (ringTop <= wantEnd) {
        wantEnd -= ringEnd;
    }
    remain = *(s32 *)(desc + 0xC) - 4;
    first = ringTop - wantEnd;
    if (remain < first) {
        first = remain;
    }

    func_003506A8((FmvPtsQueue *)(g_pFmvArenaBase + FMV_PTS_OFS), &span0Ptr,
                  &span0Len, &span1Ptr, &span1Len);
    stored = func_003511A8(span0Ptr, span0Len, span1Ptr, span1Len, (u8 *)wantEnd,
                           first, ringBase, remain - first);
    func_00350778((FmvPtsQueue *)(g_pFmvArenaBase + FMV_PTS_OFS), stored);
    return stored > 0;
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003511A8);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0..s7/ra). Revisit with the gameplay-TU compiler.

   Scatter two source spans (src0/len0, src1/len1) into a two-segment ring
   destination (dst0 with capacity len0d, wrapping into dst1) given the leading
   write offset; returns the total bytes written (0 if it would overflow). */
s32 func_003511A8(u8 *dst0, s32 len0d, u8 *dst1, s32 cap, u8 *src0, s32 len1d,
                  u8 *src1, s32 tail) {
    if (len1d + tail <= len0d + cap) {
        if (len1d < len0d) {
            s32 gap = len0d - len1d;
            if (tail < gap) {
                memcpy(dst0, src0, len1d);
                memcpy(dst0 + len1d, src1, tail);
            } else {
                memcpy(dst0, src0, len1d);
                memcpy(dst0 + len1d, src1, gap);
                memcpy(dst1, src1 + (len0d - len1d), tail - gap);
            }
        } else {
            memcpy(dst0, src0, len0d);
            memcpy(dst1, src0 + len0d, len1d - len0d);
            memcpy(dst1 + (len1d - len0d), src1, tail);
        }
        return len1d + tail;
    }
    return 0;
}
#endif

/**
 * Initialise the pts ring header: empty, read position at the start,
 * capacity = the 0x50000-byte payload area.
 */
void func_003512D8(FmvPtsRing *ring) {
    ring->size = 0x50000;
    ring->readOfs = 0;
    ring->fill = 0;
}

/**
 * FMV idle hook (registered where a flush callback is optional) — does
 * nothing.
 */
void func_003512F0(void) {
}

/**
 * Free space in the pts ring: returns the writable byte count and (if any)
 * the current write pointer through *out.
 */
s32 func_003512F8(u8 *base, u8 **out) {
    FmvPtsRing *ring = (FmvPtsRing *)base;
    s32 space = ring->size - ring->fill;

    if (space != 0) {
        *out = base + ring->readOfs;
    }
    return space;
}

/* func_00351328: commit up to n bytes into the pts ring (fill += take,
 * readOfs = (readOfs+take) %% size). Best attempt 74%: the original
 * schedules the div-by-zero beqzl/break check directly after the loads,
 * six insns BEFORE the div, while the pinned cc1 keeps the check after the
 * div (with the fill store between) and swaps the size/fill registers.
 * Same div-expansion-scheduling family as GetRandomInt. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351328);

/**
 * Peek the queued region of the pts ring: returns the queued byte count and
 * (if nonzero) the read pointer through *out.
 */
s32 func_00351370(u8 *base, u8 **out) {
    FmvPtsRing *ring = (FmvPtsRing *)base;

    if (ring->fill != 0) {
        *out = base + (ring->readOfs - ring->fill + ring->size) % ring->size;
    }
    return ring->fill;
}

/* func_003513B8: consume up to n bytes from the pts ring (fill -= min(n,
 * fill)). Best attempt 95.6%: instruction-identical (incl. the fill-copy /
 * movn clamp shape) but the copy/condition temporaries colour v1/a2 where
 * the original has a2/v1 - the register-coloring wall. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003513B8);

/**
 * Arm a stream request: store destination + length and report accepted.
 */
s32 func_003513E0(u32 *req, u32 len, u32 dest) {
    req[1] = len;
    req[0] = dest;
    return 1;
}

/**
 * Stream-callback stub: always "ready".
 */
s32 func_003513F0(void) {
    return 1;
}

/* func_003513F8: CD sector read kickoff. Blocked: 8-byte-packed saves
 * (s0..s4/ra). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003513F8);

/**
 * Translate a DMA tag address into a sector delta from the stream start:
 * returns 0 when the tag sits at the stream's own end-marker position,
 * otherwise the >>11 (2KB-sector) offset from the base LBA.
 */
u32 func_00351498(u32 *s, u32 addr) {
    if (addr != ((s[2] * 0x10 + s[1] + 0x10) & 0xFFFFFFF)) {
        return (addr - s[0]) >> 11;
    }
    return 0;
}

/* func_003514E0 / func_00351550: DMAC ch3/ch4 CHCR writes under the
 * ENABLEW suspend protocol. Blocked: 8-byte-packed saves (s0@0x0, ra@0x8). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003514E0);

/* TODO(hle): needs PS2 graphics/IO HLE backend — writes REG_DMAC_4_IPU_TO_CHCR
   under the REG_DMAC_ENABLER/ENABLEW channel-suspend protocol (the companion of
   func_003514E0). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351550);

/**
 * Build a 64-bit IPU_TO DMA tag word: madr in the high word, the quadword
 * count field at bit 28, the tag id bits in the low word.
 */
void func_003515C0(u64 *tag, u64 madr, u64 qwc, u64 id) {
    *tag = madr << 32 | (qwc << 32) >> 4 | (id << 32) >> 32;
}

/* func_003515E8: stream object constructor (CreateSema + ring reset).
 * Blocked: 8-byte-packed saves (s0@0x20, ra@0x28). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003515E8);

/* TODO(hle): needs PS2 graphics/IO HLE backend — builds the IPU_TO DMA source
   tag chain (one 0x80-qwc IPU_TO tag per macroblock plus the terminator) and
   kicks channel 4 by writing REG_DMAC_4_IPU_TO_QWC/MADR/TADR. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351660);

/* WALL: deferred-native body had a signature inconsistency with its
   forwarder/caller (caught by the TARGET_NATIVE compile sweep). Left bare
   INCLUDE_ASM (no #else); revisit with the asm when the FMV native backend
   is built. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003517C0);

/* func_003518B8: sema-guarded read-cursor advance. Blocked: 8-byte-packed
 * saves (s0/s1/ra). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003518B8);

/* TODO(hle): needs PS2 graphics/IO HLE backend — advances the IPU_TO DMA tag
   ring from the channel's REG_DMAC_4_IPU_TO_CHCR/MADR progress, rebuilds the
   consumed tags, and re-kicks the channel. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351910);

/* func_00351B10: IPU/DMA state capture + reset. Blocked: 8-byte-packed
 * saves (s0@0x0, ra@0x8) plus lui/ori materialisation of 0x10002010 where
 * the pinned cc1 folds the low half into the load offset. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351B10);

/* TODO(hle): needs PS2 graphics/IO HLE backend — the IPU restart path: reads
   REG_IPU_CTRL/IPU_BP, drains REG_DMAC_3_IPU_FROM, re-issues the IPU command
   (REG_IPU_CMD) and re-arms REG_DMAC_4_IPU_TO_MADR/TADR/QWC. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351C20);

/* func_00351F58: IPU_TO channel teardown + DeleteSema. Blocked:
 * 8-byte-packed saves (s0@0x0, ra@0x8). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351F58);

/* func_00351FB0: sema-guarded total-bytes-queued read. Blocked:
 * 8-byte-packed saves (s0/s1/ra). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351FB0);

/* func_00352000: sema-guarded 2KB round-up of the byte cursor. Blocked:
 * 8-byte-packed saves (s0@0x0, ra@0x8). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352000);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352058);
#else
/* TODO(match): functional equivalent - not byte-exact; multiple div ops each
   guarded by a branch-likely beql/break div-by-zero check (the div-expansion-
   scheduling + branch-likely wall, same family as func_00351328). Revisit with
   the gameplay-TU compiler.

   ACK consumed DMA-tag ranges against the IPU frame-slot ring: walks the slot
   array (slots are 0x18 bytes, base at obj+0x50, ring length obj+0x54, read
   cursor obj+0x58, write cursor obj+0x5C, sector size obj+0x08*0x800) from the
   current tail, clamping each slot's remaining length (+0x14) by how much of
   the request descriptor (req+0x10 base, req+0x14 length) overlaps it modulo
   the sector size, retiring fully-consumed slots and advancing the tail. */
s32 func_00352058(u8 *obj, u8 *req) {
    s32 ringLen = *(s32 *)(obj + 0x54);
    s32 idx = (*(s32 *)(obj + 0x5C) - *(s32 *)(obj + 0x58) + ringLen) % ringLen;
    s32 sectorSize = *(s32 *)(obj + 8) * 0x800;
    s32 ok = 1;

    if (*(s32 *)(obj + 0x58) > 0) {
        u8 *slot = *(u8 **)(obj + 0x50) + idx * 0x18;

        if (*(s32 *)(slot + 0x14) != 0) {
            s32 reqLen = *(s32 *)(req + 0x14);

            while (reqLen != 0) {
                s32 slotPos = *(s32 *)(slot + 0x10);

                if ((slotPos + sectorSize - *(s32 *)(req + 0x10)) % sectorSize < reqLen) {
                    s32 remain = *(s32 *)(slot + 0x14);
                    s32 take = *(s32 *)(req + 0x10) + reqLen - slotPos;

                    if (remain < take) {
                        take = remain;
                    }
                    *(s32 *)(slot + 0x14) = remain - take;
                    *(s32 *)(slot + 0x10) = (slotPos + take) % sectorSize;

                    if (remain - take == 0) {
                        if (*(s64 *)slot >= 0) {
                            *(s32 *)(slot + 0x14) = 0;
                            *(s64 *)slot = -1;
                            *(s64 *)(slot + 8) = -1;
                            *(s32 *)(slot + 0x10) = 0;
                        }
                        s32 tail = *(s32 *)(obj + 0x58) - 1;
                        if (tail < 0) {
                            tail = 0;
                        }
                        *(s32 *)(obj + 0x58) = tail;
                    }
                } else {
                    ok = 0;
                }

                idx = (idx + 1) % *(s32 *)(obj + 0x54);
                if (!ok) {
                    return 0;
                }
                slot = *(u8 **)(obj + 0x50) + idx * 0x18;
                if (*(s32 *)(slot + 0x14) == 0) {
                    return 0;
                }
                reqLen = *(s32 *)(req + 0x14);
            }
        }
    }
    return 0;
}
#endif

/* WALL: deferred-native body had a signature inconsistency with its
   forwarder/caller (caught by the TARGET_NATIVE compile sweep). Left bare
   INCLUDE_ASM (no #else); revisit with the asm when the FMV native backend
   is built. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003521B0);

/* TODO(hle): needs PS2 graphics/IO HLE backend — scans the IPU frame-slot ring
   for the range covering the channel's current REG_DMAC_4_IPU_TO_MADR /
   REG_IPU_BP position and pops it; depends on live IPU/DMAC hardware state. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003522C0);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352468);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves. Revisit with the gameplay-TU compiler.

   Construct the FMV stream object: reset its message dispatch table
   (func_0012F738), register the five stream-event callbacks (frame-drop
   report, two DMA-add-queue pumps, retry, and the cursor snapshot), clear the
   playback FSM (func_003525D0), then build the embedded bitstream object at
   +0x48 (func_003515E8). Always reports success. */
s32 func_00352468(u8 *obj, u64 p2, u64 p3, u64 p4, u64 p5, u64 p6, u64 p7,
                  u64 p8) {
    func_0012F738();
    func_0012FA70(obj, 0, (void *)func_00352A20, 0);
    func_0012FA70(obj, 1, (void *)func_00352A48, 0);
    func_0012FA70(obj, 2, (void *)func_00352A80, 0);
    func_0012FA70(obj, 3, (void *)func_00352AB0, 0);
    func_0012FA70(obj, 5, (void *)func_00352AE0, 0);
    func_003525D0((FmvStream *)obj);
    func_003515E8(obj + 0x48, p4, p5, p6, p7, p8);
    return 1;
}
#endif

/* func_00352568: 8 bytes of inter-function padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352568);

/**
 * Stream-callback: kick the WAD streaming pump and report handled.
 */
s32 func_00352570(void) {
    func_0012EE28();
    return 1;
}

/**
 * Forward to the bitstream feeder of the stream object embedded at +0x48.
 */
s32 func_00352590(u8 *obj) {
    return func_003517C0(obj + 0x48);
}

/**
 * Forward to the read-cursor advance of the embedded stream object.
 */
s32 func_003525B0(u8 *obj) {
    return func_003518B8(obj + 0x48);
}

/**
 * Reset the playback FSM to idle.
 */
void func_003525D0(FmvStream *s) {
    s->state = 0;
}

/* func_003525D8: stop + detach the embedded stream. Blocked: 8-byte-packed
 * saves (s0@0x0, ra@0x8). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003525D8);

/* func_00352610: `s->state = 1; return 1;`. Blocked: the original (later
 * SN cc1) reuses ONE `li v0,1` for both the store and the return value; the
 * pinned cc1 always materialises two (same wall as text/1907F0
 * func_00290EE8, re-measured here at 63%). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352610);

/**
 * Read the playback FSM state.
 */
s32 func_00352620(FmvStream *s) {
    return s->state;
}

/**
 * Exchange the playback FSM state, returning the previous value.
 */
s32 func_00352628(FmvStream *s, s32 state) {
    s32 old = s->state;

    s->state = state;
    return old;
}

/* func_00352638: queue an IPU DMA-add command {addr, size, pos -
 * obj->streamStart, tag} for a bitstream span. Best attempt 54%: the
 * pinned cc1 schedules the obj->0x48 load + subu early (or, with volatile
 * pinning, pushes the arena load late and pads the jal delay slot) where
 * the original loads obj->0x48 late and puts the rel store in the jal
 * delay slot - prologue-scheduling shapes not reachable from this source. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352638);

/**
 * Forward to the total-bytes-queued read of the embedded stream object.
 */
s32 func_00352680(u8 *obj) {
    return func_00351FB0(obj + 0x48);
}

/* func_003526A0: 8 bytes of inter-function padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003526A0);

/* func_003526A8: end-of-stream flush (pads the bitstream to a 4-byte
 * boundary). Blocked: 8-byte-packed saves (s0@0x20, ra@0x28). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003526A8);

/* func_00352780: poll stream-done then host-side done. Blocked:
 * 8-byte-packed saves (s0/s1/ra). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352780);

/* func_003527C8: the FMV decode-thread main loop. Blocked: 8-byte-packed
 * saves (s0/s1/s2/ra) plus a delay-slot %gp_rel read of g_pFmvArenaBase
 * mixed with its absolute form (the reload-artifact wall). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003527C8);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352868);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0..s2/ra) plus a delay-slot %gp_rel read of the arena base mixed
   with its absolute form. Revisit with the gameplay-TU compiler.

   The FMV frame-display worker loop: pulls the next decoded picture from the
   host (func_0012F9A8/func_0012F950), waits (yielding) for a free display
   slot (func_00352C30), builds the per-tile GIF display chain for each frame
   tile (func_00350B60), commits the slot (func_00352BB8), and yields. Exits
   when the host signals end-of-stream or the FSM reports stop. */
s32 func_00352868(u8 *host) {
    s32 result = 1;

    for (;;) {
        u8 *slot;
        s32 *streamObj = (s32 *)host;

        if (func_0012F9A8(host) != 0) {
            break;
        }
        if (func_00352620((FmvStream *)host) == 1) {
            result = -1;
            DebugPrintStub(D_1AE820);
            break;
        }
        while ((slot = (u8 *)func_00352C30(g_pFmvArenaBase + 0xD9168)) == 0) {
            func_00350100();
        }
        if (func_0012F950(host, (s32)slot, 0x340) < 0) {
            func_003504C8(D_1AE838);
        }
        if (streamObj[2] == 0) {
            u32 madr = streamObj[0];
            u32 tadr = streamObj[1];
            s32 tile = 0;
            s32 gifOfs = 0;
            s32 frameOfs = 0;

            while (tile < *(s32 *)(g_pFmvArenaBase + 0xD9178)) {
                func_00350B60(*(u8 **)(g_pFmvArenaBase + 0xD916C) + gifOfs + 0x40,
                              *(u8 **)(g_pFmvArenaBase + 0xD9168) + frameOfs, madr, tadr,
                              tile);
                frameOfs += 0xD0000;
                gifOfs += 0x138C0;
                tile++;
            }
        }
        func_00352BB8(g_pFmvArenaBase + 0xD9168);
        func_00350100();
    }
    func_0012F9C8(host);
    return result;
}
#endif

/**
 * Report a dropped/late frame through the debug stub (prints the frame's
 * sequence number).
 */
s32 func_00352A20(s32 unused, s32 *frame) {
    DebugPrintStub(D_1AE860, frame[1]);
    return 1;
}

/**
 * Stream-callback: yield once to the scheduler, then pump the IPU DMA-add
 * queue and report handled.
 */
s32 func_00352A48(void) {
    func_00350100();
    func_00351910(*(u8 *volatile *)&g_pFmvArenaBase + FMV_DMAQ_OFS);
    return 1;
}

/**
 * Stream-callback: pump the IPU DMA-add queue once and report handled.
 */
s32 func_00352A80(void) {
    func_00351B10(g_pFmvArenaBase + FMV_DMAQ_OFS);
    return 1;
}

/**
 * Stream-callback: pump the IPU DMA-add retry path once and report handled.
 */
s32 func_00352AB0(void) {
    func_00351C20(g_pFmvArenaBase + FMV_DMAQ_OFS);
    return 1;
}

/* func_00352AE0: snapshot the DMA queue cursor pair into the stream object.
 * Blocked: 8-byte-packed saves (s0@0x20, ra@0x28). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352AE0);

/* func_00352B30: initialise the decoded-frame display queue (fields, then
 * clear each slot's {state,index} header). Best attempt 88%: byte-identical
 * structure (incl. the strength-reduced offset induction var initialised
 * inside the loop preheader, recovered with a volatile q + i*0x138C0
 * indexing) but the i/offset/stride registers colour t0/a1/a2 where the
 * original has a1/a2/t0 - the known register-coloring wall. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352B30);

/**
 * FMV idle hook (frame-queue variant) — does nothing.
 */
void func_00352B88(void) {
}

/**
 * Clear the frame queue's display state (no frames queued, write cursor
 * reset).
 */
void func_00352B90(FmvFrameQueue *q) {
    volatile FmvFrameQueue *v = q;

    v->count = 0;
    v->writeIdx = 0;
}

/**
 * True when every slot of the frame queue is filled (display is saturated).
 */
s32 func_00352BA0(FmvFrameQueue *q) {
    return q->count == q->capacity;
}

/* func_00352BB8: commit the just-decoded slot (mark state 2, advance the
 * write cursor modulo capacity) under DI/EI. Blocked: 8-byte-packed saves
 * (s0@0x0, ra@0x8). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352BB8);

/* func_00352C30: writable GS frame pointer (writeIdx * 0xD0000). Blocked:
 * 8-byte-packed saves (s0@0x0, ra@0x8). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352C30);

/**
 * True when the frame queue holds no displayable frame.
 */
s32 func_00352C70(FmvFrameQueue *q) {
    return q->count == 0;
}

/* func_00352C80: oldest queued decoded-frame pointer ((writeIdx - count +
 * cap) % cap slot). Blocked: 8-byte-packed saves (s0@0x0, ra@0x8). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352C80);

/**
 * Release the oldest displayed frame from the queue (if any).
 */
void func_00352CE8(FmvFrameQueue *q) {
    volatile s32 *count = &q->count;

    if (*count > 0) {
        *count = *count - 1;
    }
}

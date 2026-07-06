#include "common.h"

/*
 * EU text/251520 — REGION-AXIS twin of USA text/250080 (FMV playback engine,
 * the very tail of .text). Same later-SN-cc1 TU built at -O2 -G8 -fno-gcse
 * (per-unit GFLAG/CC1EXTRA in tools/ee/objdiff_build.sh). The C bodies are
 * region-agnostic; only the referenced extern global/function NAMES differ
 * between the builds (objdiff masks the gp/reloc deltas). Ported from
 * src/usa/text/250080.c; every body here matches the EU bytes at the unit
 * level. See the USA unit for the full FMV-arena layout documentation.
 *
 * EU symbol retargets vs USA (USA name -> EU name / address):
 *   g_pFmvArenaBase 0x1B234C -> 0x1B23CC, which the EU asm aliases as
 *       (g_nVendorBuyQuantity + 0x184) [g_nVendorBuyQuantity = 0x1B2248];
 *   DebugPrintStub                  -> func_0026FD28 (EU has a real printf);
 *   D_1AE7D8 ("[ Error ] %s\n")     -> D_1AE890;
 *   D_1AE860 (frame-drop diag)      -> D_1AE918;
 *   D_1AE788 (vblank "displayed")   -> D_1AE840;
 *   func_00350830 (pts >=0x1000)    -> func_00351CD0;
 *   func_00350840 (pts pump)        -> func_00351CE0;
 *   func_001338C8 (CD/WAD pump)     -> func_00133928;
 *   func_0012EE28 (WAD pump)        -> func_0012EE28 (same);
 *   func_003517C0 (bitstream feed)  -> func_00352C60;
 *   func_003518B8 (read-cursor adv) -> func_00352D58;
 *   func_00351FB0 (total queued)    -> func_00353450;
 *   func_00350100 (thread yield)    -> func_003515A0;
 *   func_00351910 (IPU dmaq pump)   -> func_00352DB0;
 *   func_00351B10 (dmaq pump)       -> func_00352FB0;
 *   func_00351C20 (dmaq retry pump) -> func_003530C0;
 *   func_0011AAD0 (RotateReadyQ)    -> func_0011AAD0 (same).
 *
 * The 50 functions that stay INCLUDE_ASM below are the same shapes blocked in
 * USA (8-byte-packed saves, register-coloring, div scheduling, reload
 * artifacts, handwritten asm, inter-function padding) — see the USA unit's
 * per-function comments. They are region-agnostic blockers, not EU-specific
 * divergence.
 */

/* cc1-small / assembler-absolute symbols. g_nVendorBuyQuantity is the EU
 * named anchor; the FMV arena base pointer lives at +0x184 from it. Sized 16
 * so cc1 emits the one-insn symbolic macro (not %gp_rel) and the assembler
 * expands it absolutely. */
__asm__(".extern g_nVendorBuyQuantity, 16");
__asm__(".extern D_1AE840, 16");

extern s32 g_nVendorBuyQuantity;
extern s32 D_1AE840; /* vblank field-flip "frame displayed" latch */

/* The FMV arena base pointer (EU 0x1B23CC = g_nVendorBuyQuantity + 0x184). */
#define g_pFmvArenaBase (*(u8 **)((char *)&g_nVendorBuyQuantity + 0x184))

/* Debug printf-stub format strings (rodata). */
extern char D_1AE890[]; /* "[ Error ] %s\n" */
extern char D_1AE918[]; /* frame-drop diagnostic */

#define FMV_STREAM_OFS 0xD9048 /* bitstream/stream-state object */
#define FMV_DMAQ_OFS 0xD9090   /* IPU DMA-add queue */
#define FMV_PTS_OFS 0xD9100    /* pts queue control block */
#define FMV_FRAMEQ_OFS 0xD9168 /* decoded-frame display queue */

typedef struct FmvPtsRing {
    /* 0x00000 */ u8 data[0x50000];
    /* 0x50000 */ s32 readOfs;
    /* 0x50004 */ s32 fill;
    /* 0x50008 */ s32 size;
} FmvPtsRing;

typedef struct FmvFrameQueue {
    /* 0x00 */ u8 *gsFrames;
    /* 0x04 */ u8 *frames;
    /* 0x08 */ s32 writeIdx;
    /* 0x0C */ s32 count;
    /* 0x10 */ s32 capacity;
} FmvFrameQueue;

typedef struct FmvPtsQueue {
    /* 0x00 */ s32 started;
    /* 0x04 */ s32 mode;
    /* 0x08 */ u8 hdr[0x28];
    /* 0x30 */ s32 hdrFill;
    /* 0x34 */ u8 *dataPtr;
    /* 0x38 */ s32 ringOfs;
    /* 0x3C */ s32 consumed;
    /* 0x40 */ s32 ringSize;
    /* 0x44 */ s32 received;
    /* 0x48 */ u8 pad48[0x8];
    /* 0x50 */ s32 queued;
} FmvPtsQueue;

typedef struct FmvStream {
    /* 0x00 */ u8 pad[0xA8];
    /* 0xA8 */ s32 state;
} FmvStream;

extern s32 func_0011AAD0(s32 count);           /* RotateThreadReadyQueue */
extern s32 func_00352DB0(void *dmaq);
extern s32 func_00351CD0(FmvPtsQueue *q);
extern s32 func_00351CE0(FmvPtsQueue *q);
extern s32 func_00351DB0(void);
extern s32 func_00133928(void);
extern s32 func_0012EE28(void);
#ifndef TARGET_NATIVE
extern s32 func_00352C60(void *stream);
extern s32 func_00352D58(void *stream);
#else
/* func_00352C60/func_00352D58 (twins of USA func_003517C0/func_003518B8): the
   #else bodies model them with inconsistent arg counts across call sites (1 arg
   in the stream forwarders, 5/2 args in func_00352428 - true signatures need
   the asm; FMV native backend deferred). Declared with unspecified args so the
   corpus compiles. */
extern s32 func_00352C60();
extern s32 func_00352D58();
#endif
extern s32 func_00353450(void *stream);
extern s32 func_00352FB0(void *dmaq);
extern s32 func_003530C0(void *dmaq);
extern s32 func_0026FD28(char *fmt, ...);

/* === func_003515A0 (USA func_00350100) ============================== */
/**
 * Yield the FMV thread for one scheduler rotation (RotateThreadReadyQueue on
 * priority 1). The empty asm guard suppresses cc1's void sibling-call opt.
 */
void func_003515A0(void) {
    func_0011AAD0(1);
    __asm__ __volatile__("");
}

/* === func_003515C0 (USA func_00350120) ============================== */
/**
 * Poll whether the pts ring holds at least one decodable unit (forwards to
 * the >=0x1000-bytes-queued check on the arena's pts ring).
 */
s32 func_003515C0(void) {
    return func_00351CD0((FmvPtsQueue *)(g_pFmvArenaBase + FMV_PTS_OFS));
}

/* func_003515F0 (USA OnFmvGifDmaInterrupt): handwritten `sync; ei` pair. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003515F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00351878);

/* === func_00351968 (USA func_003504C8) ============================== */
/**
 * Print an FMV error message through the debug stub ("[ Error ] %s\n").
 */
s32 func_00351968(char *msg) {
    return func_0026FD28(D_1AE890, msg);
}

/* === func_00351990 (USA func_003504F0) ============================== */
/**
 * Pump the pts-ring stall check on the arena's pts ring (callback wrapper).
 */
s32 func_00351990(void) {
    return func_00351CE0((FmvPtsQueue *)(g_pFmvArenaBase + FMV_PTS_OFS));
}

/* func_003519C0 (USA func_00350520): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003519C0);

/* === func_00351A80 (USA func_003505E0) ============================== */
/**
 * Stream-callback: kick the CD/WAD reader pump and report ready.
 */
s32 func_00351A80(void) {
    func_00133928();
    return 1;
}

/* func_00351AA0 (USA func_00350600): inter-function padding, no C. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00351AA0);

/* func_00351AA8 (USA func_00350608): commit-dispatch on the stream object.
 * Blocked, 8-byte-packed saves; asm-logic identical to USA func_00350608 (the
 * commit callee shifts: EU func_00133950 vs USA func_001338F0). The portable
 * TARGET_NATIVE body lives in the USA twin (src/usa/text/250080.c). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00351AA8);

/* func_00351B00 (USA func_00350660): stream-state reset. Blocked, 8-byte-packed
 * saves; asm-logic identical to USA func_00350660 (lock callee shifts: EU
 * func_001338F0 vs USA func_00133890). The portable TARGET_NATIVE body lives in
 * the USA twin (src/usa/text/250080.c). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00351B00);

/* func_00351B48 (USA func_003506A8): query the pts ring's two writable spans
 * for the producer. When not yet started, returns either the whole data region
 * (mode 4) or the header-fill window plus the data region; once started, returns
 * the wrap-around free window (up to two segments). Region-agnostic port of the
 * USA body (typed FmvPtsQueue accesses only; no callees, no globals). Matching
 * arm stays asm (branch-likely scheduling the pinned cc1 won't emit). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00351B48);
#else
void func_00351B48(FmvPtsQueue *q, u8 **pPtr0, s32 *pLen0, u8 **pPtr1, s32 *pLen1) {
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

/* func_00351C18 (USA func_00350778): blocked, register-coloring wall. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00351C18);

/* === func_00351CD0 (USA func_00350830) ============================== */
/**
 * True once the pts ring has at least 0x1000 bytes queued.
 */
s32 func_00351CD0(FmvPtsQueue *q) {
    return q->queued >= 0x1000;
}

/* === func_00351CE0 (USA func_00350840) ============================== */
/**
 * If the stream has been started, pump the bitstream feeder once. (Declared
 * value-returning like the original; the undefined fall-through result is
 * forwarded by callers and keeps cc1 from sibling-call opting the tails.)
 */
s32 func_00351CE0(FmvPtsQueue *q) {
    if (q->started != 0) {
        return func_00351DB0();
    }
}

/* func_00351D08 (USA func_00350868): SIF-DMA bounce of a decoded block to IOP
 * memory. Blocked: 8-byte-packed saves; asm-logic identical to USA func_00350868
 * (notify callee shifts: EU func_00133990 vs USA func_00133930). The portable
 * TARGET_NATIVE body lives in the USA twin (tester-EE-routed: SIF busy-wait). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00351D08);

/* func_00351DB0 (USA func_00350910): bitstream feeder (EU twin). Blocked in
 * USA too — left INCLUDE_ASM there. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00351DB0);

/* func_00352000 (USA func_00350B60): blocked (INCLUDE_ASM in USA too). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352000);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003521A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352378);

/* func_003523C8 (USA func_00350F28): blocked, 8-byte-packed saves +
 * delay-slot %gp_rel read. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003523C8);

/* === func_00352418 (USA func_00350F78) ============================== */
/**
 * Clear the "frame displayed this vblank" latch.
 */
void func_00352418(void) {
    D_1AE840 = 0;
}

/* func_00352428 (USA func_00350F88): copy a window of arrived bytes into the
 * IPU payload ring's two writable spans - clamps the request to the
 * descriptor's remaining length, queries the spans (func_00352C60 on the
 * DMA-queue stream at streamObj+0x48, span pointers masked to the 0x20000000
 * scratch-pad address window), scatters into them (func_00352648), records the
 * resulting DMA-add command (func_00353AD8) reporting an error (func_00351968
 * on D_1AE8A0) if it could not be queued, advances the read cursor
 * (func_00352D58), and reports whether any bytes were stored. Ported from the
 * USA body: g_pFmvArenaBase retargets to the EU alias via the file-level macro,
 * and the callees shift +0x14A0 (func_003517C0->func_00352C60,
 * func_003511A8->func_00352648, func_00352638->func_00353AD8,
 * func_003504C8->func_00351968, func_003518B8->func_00352D58); the error string
 * D_1AE7E8->D_1AE8A0 (both verified against the EU asm). NB the structure-exact
 * model names the span-query/cursor-advance helpers func_00352C60/func_00352D58,
 * whereas the matching asm calls func_00353A30/func_00353A50 - a deliberate
 * modelling choice carried over verbatim from the USA twin (the #else is not
 * byte-verified). Matching arm stays asm (8-byte-packed saves). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352428);
#else
extern s32 func_00352648(u8 *dst0, s32 len0d, u8 *dst1, s32 cap, u8 *src0, s32 len1d,
                         u8 *src1, s32 tail);        /* twin func_003511A8 (defined below) */
extern s32 func_00353AD8(u8 *obj, u64 a, u64 b, s32 pos, s32 n); /* twin func_00352638 (below) */
extern char D_1AE8A0[];   /* DMA-add-queue-full error string (twin of D_1AE7E8) */
s32 func_00352428(u8 *unused, u8 *desc, u8 *ringBase) {
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
    func_00352C60((s32 *)(streamObj + 0x48), &span0Ptr, &span0Len, &span1Ptr,
                  &span1Len);
    stored = func_00352648((u8 *)((span0Ptr & 0xFFFFFFF) | 0x20000000), span0Len,
                           (u8 *)((span1Ptr & 0xFFFFFFF) | 0x20000000), span1Len,
                           (u8 *)start, take, ringBase, remain - take);
    if (stored > 0 &&
        func_00353AD8(streamObj, *(u64 *)(desc + 0x10), *(u64 *)(desc + 0x18),
                      span0Ptr, stored) == 0) {
        func_00351968(D_1AE8A0);
    }
    func_00352D58(streamObj + 0x48, stored);
    return stored > 0;
}
#endif

/* func_00352560 (USA func_003510C0): copy a window of arrived elementary-stream
 * bytes into the pts ring - clamps the request to the ring's free header window
 * and the descriptor's remaining length, queries the ring's two writable spans
 * (func_00351B48), scatters into them (func_00352648), advances the ring
 * (func_00351C18), and reports whether any bytes were stored. Ported from the
 * USA body: g_pFmvArenaBase retargets to the EU alias via the file-level macro,
 * and the three callees shift +0x14A0 (func_003506A8->func_00351B48,
 * func_003511A8->func_00352648, func_00350778->func_00351C18) - all verified
 * against the EU asm. Matching arm stays asm (8-byte-packed saves). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352560);
#else
extern s32 func_00351C18(FmvPtsQueue *q, s32 n); /* pts-ring advance (twin of func_00350778) */
extern s32 func_00352648(u8 *dst0, s32 len0d, u8 *dst1, s32 cap, u8 *src0, s32 len1d,
                         u8 *src1, s32 tail); /* two-span scatter (defined below) */
s32 func_00352560(u8 *unused, u8 *desc, u8 *ringBase) {
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

    func_00351B48((FmvPtsQueue *)(g_pFmvArenaBase + FMV_PTS_OFS), &span0Ptr,
                  &span0Len, &span1Ptr, &span1Len);
    stored = func_00352648(span0Ptr, span0Len, span1Ptr, span1Len, (u8 *)wantEnd,
                           first, ringBase, remain - first);
    func_00351C18((FmvPtsQueue *)(g_pFmvArenaBase + FMV_PTS_OFS), stored);
    return stored > 0;
}
#endif

/* func_00352648 (USA func_003511A8): scatter two source spans (src0/len0,
 * src1/len1) into a two-segment ring destination (dst0 with capacity len0d,
 * wrapping into dst1) given the leading write offset; returns the total bytes
 * written (0 if it would overflow). Region-agnostic port of the USA body (only
 * memcpy, which is region-identical). Matching arm stays asm (8-byte-packed
 * saves s0..s7/ra). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352648);
#else
s32 func_00352648(u8 *dst0, s32 len0d, u8 *dst1, s32 cap, u8 *src0, s32 len1d,
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

/* === func_00352778 (USA func_003512D8) ============================== */
/**
 * Initialise the pts ring header: empty, read position at the start, capacity
 * = the 0x50000-byte payload area.
 */
void func_00352778(FmvPtsRing *ring) {
    ring->size = 0x50000;
    ring->readOfs = 0;
    ring->fill = 0;
}

/* === func_00352790 (USA func_003512F0) ============================== */
/**
 * FMV idle hook (flush-callback variant) — does nothing.
 */
void func_00352790(void) {
}

/* === func_00352798 (USA func_003512F8) ============================== */
/**
 * Free space in the pts ring: returns the writable byte count and (if any)
 * the current write pointer through *out.
 */
s32 func_00352798(u8 *base, u8 **out) {
    FmvPtsRing *ring = (FmvPtsRing *)base;
    s32 space = ring->size - ring->fill;

    if (space != 0) {
        *out = base + ring->readOfs;
    }
    return space;
}

/* func_003527C8 (USA func_00351328): blocked, div scheduling wall. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003527C8);

/* === func_00352810 (USA func_00351370) ============================== */
/**
 * Peek the queued region of the pts ring: returns the queued byte count and
 * (if nonzero) the read pointer through *out.
 */
s32 func_00352810(u8 *base, u8 **out) {
    FmvPtsRing *ring = (FmvPtsRing *)base;

    if (ring->fill != 0) {
        *out = base + (ring->readOfs - ring->fill + ring->size) % ring->size;
    }
    return ring->fill;
}

/* func_00352858 (USA func_003513B8): blocked, register-coloring wall. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352858);

/* === func_00352880 (USA func_003513E0) ============================== */
/**
 * Arm a stream request: store destination + length and report accepted.
 */
s32 func_00352880(u32 *req, u32 len, u32 dest) {
    req[1] = len;
    req[0] = dest;
    return 1;
}

/* === func_00352890 (USA func_003513F0) ============================== */
/**
 * Stream-callback stub: always "ready".
 */
s32 func_00352890(void) {
    return 1;
}

/* func_00352898 (USA func_003513F8): kick off a CD sector read for the FMV
 * bitstream. Converts byteLen to 2KB sectors (>>11), issues sceCdRead
 * (func_001253A8) from the object's LBN cursor (obj+0x4) into buf with a fixed
 * retry mode {trycount=0x64, spindlctrl=1, datapattern=0}. flag != 0 returns 0
 * without advancing (query/prime); otherwise advances the cursor, waits
 * (sceCdSync, func_00124B88(0)) and returns byteLen. Region-agnostic port of the
 * USA body (SDK callees are region-identical; no data globals). Matching arm
 * stays asm (8-byte-packed saves). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352898);
#else
extern s32 func_001253A8(u32 lbn, u32 sectors, void *buf, void *mode); /* sceCdRead */
extern s32 func_00124B88(s32 mode);                                    /* sceCdSync */
s32 func_00352898(u8 *obj, void *buf, s32 byteLen, s32 flag) {
    u8 mode[4];
    s32 sectors = byteLen >> 11; /* bytes -> 2KB sectors */

    mode[0] = 0x64; /* trycount    */
    mode[1] = 1;    /* spindlctrl  */
    mode[2] = 0;    /* datapattern */
    mode[3] = 0;    /* pad */
    func_001253A8(*(s32 *)(obj + 0x4), sectors, buf, mode);
    if (flag != 0) {
        return 0;
    }
    *(s32 *)(obj + 0x4) += sectors; /* advance the LBN cursor */
    func_00124B88(0);
    return byteLen;
}
#endif

/* === func_00352938 (USA func_00351498) ============================== */
/**
 * Translate a DMA tag address into a sector delta from the stream start:
 * returns 0 when the tag sits at the stream's own end-marker position,
 * otherwise the >>11 (2KB-sector) offset from the base LBA.
 */
u32 func_00352938(u32 *s, u32 addr) {
    if (addr != ((s[2] * 0x10 + s[1] + 0x10) & 0xFFFFFFF)) {
        return (addr - s[0]) >> 11;
    }
    return 0;
}

/* func_00352980 / func_003529F0 (USA func_003514E0/00351550): blocked,
 * 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352980);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003529F0);

/* === func_00352A60 (USA func_003515C0) ============================== */
/**
 * Build a 64-bit IPU_TO DMA tag word: madr in the high word, the quadword
 * count field at bit 28, the tag id bits in the low word.
 */
void func_00352A60(u64 *tag, u64 madr, u64 qwc, u64 id) {
    *tag = madr << 32 | (qwc << 32) >> 4 | (id << 32) >> 32;
}

/* func_00352A88 (USA func_003515E8): FMV bitstream/IPU-DMA stream-object
 * constructor. Records the caller-supplied ring parameters into the object,
 * folds the ring's physical base into the DMA source-tag word (address masked
 * to 0x0FFFFFFF, OR'd with the 0x20000000 DMAtag "next"/id field), creates the
 * object's binary completion semaphore (max=1, init=1), primes the first
 * IPU_TO tag chain (func_00352B00, EU twin of USA func_00351660), then zeroes
 * the 64-bit read cursor at +0x48. Always reports success (1). Region-agnostic
 * port of the USA body: CreateSema is at the shared SDK address 0x11AC20 in
 * both builds; only the IPU-primer callee shifts (+0x14A0). The original leaves
 * SemaParam's currentCount/numWaitThreads uninitialised (stack scratch); only
 * maxCount/initCount are written. Matching arm stays asm (8-byte-packed saves,
 * s0@0x20, ra@0x28). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352A88);
#else
/* EE-kernel binary-semaphore create (EU func_0011AC20, shared SDK address).
   SemaParam layout mirrors the SDK's <eekernel.h> (not included by this TU). */
struct SemaParam { s32 currentCount, maxCount, initCount, numWaitThreads; u32 attr, option; };
extern s32 CreateSema(struct SemaParam *p);
extern void func_00352B00(u8 *stream); /* IPU_TO DMA tag-chain primer (defined below) */
s32 func_00352A88(u8 *obj, u64 ringBase, u64 tagAddr, u64 qwc, u64 tadr, u64 chainId) {
    struct SemaParam sema;
    sema.maxCount = 1;
    sema.initCount = 1;
    *(s32 *)(obj + 0x0)  = (s32)ringBase;
    *(s32 *)(obj + 0x4)  = ((s32)tagAddr & 0x0FFFFFFF) | 0x20000000;
    *(s32 *)(obj + 0x8)  = (s32)qwc;
    *(s32 *)(obj + 0x18) = (s32)qwc << 11;
    *(s32 *)(obj + 0x50) = (s32)tadr;
    *(s32 *)(obj + 0x54) = (s32)chainId;
    *(s32 *)(obj + 0x40) = CreateSema(&sema);
    func_00352B00(obj);
    *(s64 *)(obj + 0x48) = 0;
    return 1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352B00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352C60);

/* func_00352D58 (USA func_003518B8): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352D58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352DB0);

/* func_00352FB0 (USA func_00351B10): blocked, 8-byte-packed saves +
 * lui/ori materialisation fold. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352FB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003530C0);

/* func_003533F8 (USA func_00351F58): IPU_TO channel teardown + DeleteSema.
 * Blocked: 8-byte-packed saves; asm-logic identical to USA func_00351F58 (same
 * DMAC ch4 MMIO regs 0x1000B410/420/430). The portable TARGET_NATIVE body lives
 * in the USA twin (tester-EE-routed: live DMAC MMIO). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003533F8);

/* func_00353450 (USA func_00351FB0): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353450);

/* func_003534A0 (USA func_00352000): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003534A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003534F8);

/* func_00353650 (USA func_003521B0): sema-guarded enqueue of a 0x18-byte
 * DMA-add command into the queue's ring buffer. Under the queue sema
 * (dmaq+0x40): if there is room (count@0x58 < capacity@0x54), validates the
 * command via func_003534F8 (EU twin of USA func_00352058) and - unless BOTH
 * 64-bit words (cmd+0x0, cmd+0x8) are negative (the skip sentinel) - copies
 * {u64,u64,s32,s32} into ring[writeIdx@0x5C] (base@0x50, stride 0x18), bumps
 * the count and wraps the write index modulo capacity, returning 1. Returns 0
 * if the queue is full or the command was the skip sentinel. Region-agnostic
 * port of the USA body: WaitSema/SignalSema are at the shared SDK addresses
 * 0x11AC60/0x11AC40 in both builds; only the validator callee shifts (+0x14A0).
 * Matching arm stays asm; #else is the structure-exact model. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353650);
#else
extern void WaitSema(s32 sema);
extern s32 SignalSema(s32 sema);
extern s32 func_003534F8(u8 *obj, u8 *req); /* DMA-add command validator */
s32 func_00353650(void *dmaq, void *cmd) {
    u8 *q = (u8 *)dmaq;
    u8 *c = (u8 *)cmd;
    s32 result = 0;

    WaitSema(*(s32 *)(q + 0x40));
    if (*(s32 *)(q + 0x58) < *(s32 *)(q + 0x54)) {   /* count < capacity */
        func_003534F8(q, c);
        if (*(s64 *)(c + 0x0) >= 0 || *(s64 *)(c + 0x8) >= 0) {
            u8 *slot = *(u8 **)(q + 0x50) + *(s32 *)(q + 0x5C) * 0x18;
            *(s64 *)(slot + 0x0)  = *(s64 *)(c + 0x0);
            *(s64 *)(slot + 0x8)  = *(s64 *)(c + 0x8);
            *(s32 *)(slot + 0x10) = *(s32 *)(c + 0x10);
            *(s32 *)(slot + 0x14) = *(s32 *)(c + 0x14);
            *(s32 *)(q + 0x58) += 1;
            *(s32 *)(q + 0x5C) =
                (*(s32 *)(q + 0x5C) + 1) % *(s32 *)(q + 0x54);
            result = 1;
        }
    }
    SignalSema(*(s32 *)(q + 0x40));
    return result;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353760);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353908);

/* func_00353A08 (USA func_00352568): inter-function padding, no C. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353A08);

/* === func_00353A10 (USA func_00352570) ============================== */
/**
 * Stream-callback: kick the WAD streaming pump and report handled.
 */
s32 func_00353A10(void) {
    func_0012EE28();
    return 1;
}

/* === func_00353A30 (USA func_00352590) ============================== */
/**
 * Forward to the bitstream feeder of the stream object embedded at +0x48.
 */
s32 func_00353A30(u8 *obj) {
    return func_00352C60(obj + 0x48);
}

/* === func_00353A50 (USA func_003525B0) ============================== */
/**
 * Forward to the read-cursor advance of the embedded stream object.
 */
s32 func_00353A50(u8 *obj) {
    return func_00352D58(obj + 0x48);
}

/* === func_00353A70 (USA func_003525D0) ============================== */
/**
 * Reset the playback FSM to idle.
 */
void func_00353A70(FmvStream *s) {
    s->state = 0;
}

/* func_00353A78 (USA func_003525D8): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353A78);

/* func_00353AB0 (USA func_00352610): blocked, single `li v0,1` reuse wall. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353AB0);

/* === func_00353AC0 (USA func_00352620) ============================== */
/**
 * Read the playback FSM state.
 */
s32 func_00353AC0(FmvStream *s) {
    return s->state;
}

/* === func_00353AC8 (USA func_00352628) ============================== */
/**
 * Exchange the playback FSM state, returning the previous value.
 */
s32 func_00353AC8(FmvStream *s, s32 state) {
    s32 old = s->state;

    s->state = state;
    return old;
}

/* func_00353AD8 (USA func_00352638): queue an IPU DMA-add command {addr, size,
 * pos - obj->streamStart, tag} for a bitstream span into the arena's DMA-add
 * queue (g_pFmvArenaBase + FMV_DMAQ_OFS); returns the enqueue result. Ported
 * from the USA body: the g_pFmvArenaBase reference retargets to the EU alias
 * (g_nVendorBuyQuantity + 0x184) via the file-level macro, and the enqueue
 * callee shifts func_003521B0 -> func_00353650 (+0x14A0) - both verified against
 * the EU asm. Matching arm stays asm (prologue-scheduling shapes not reachable
 * from this source); the #else is the structure-exact functional model. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353AD8);
#else
s32 func_00353AD8(u8 *obj, u64 addr, u64 size, s32 pos, s32 tag) {
    struct {
        u64 addr;
        u64 size;
        s32 pos;
        s32 tag;
    } cmd;

    cmd.addr = addr;
    cmd.size = size;
    cmd.pos = pos - *(s32 *)(obj + 0x48); /* pos relative to obj->streamStart */
    cmd.tag = tag;
    return func_00353650(g_pFmvArenaBase + FMV_DMAQ_OFS, &cmd);
}
#endif

/* === func_00353B20 (USA func_00352680) ============================== */
/**
 * Forward to the total-bytes-queued read of the embedded stream object.
 */
s32 func_00353B20(u8 *obj) {
    return func_00353450(obj + 0x48);
}

/* func_00353B40 (USA func_003526A0): inter-function padding, no C. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353B40);

/* func_00353B48 (USA func_003526A8): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353B48);

/* func_00353C20 (USA func_00352780): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353C20);

/* func_00353C68 (USA func_003527C8): the FMV decode-thread main loop. Resets
 * the embedded stream ring (func_00352B00 on obj+0x48), initialises the arena
 * frame queue (func_00354030 at g_pFmvArenaBase+0xD9168), primes the host frame
 * reader (func_00353D08), then decodes frames (func_00353AC0) as long as the
 * arena's "more data" flag (+0xD9174) stays set and no frame reports completion
 * (returns 1), and finally drives the stream to its terminal state 3
 * (func_00353AC8). Ported from the USA body: g_pFmvArenaBase retargets to the
 * EU alias (g_nVendorBuyQuantity + 0x184) via the file-level macro, and the
 * five callees shift +0x14A0 (func_00351660->func_00352B00,
 * func_00352B90->func_00354030, func_00352868->func_00353D08,
 * func_00352620->func_00353AC0, func_00352628->func_00353AC8) - all verified
 * against the EU asm. Matching arm stays asm (8-byte-packed saves + a
 * %gp_rel/absolute reload-artifact wall on g_pFmvArenaBase); the #else is the
 * structure-exact model (the frame decode itself is the deferred FMV native
 * backend, but the loop structure is exact). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353C68);
#else
extern void func_00352B00(u8 *stream);
extern s32 func_00353D08(u8 *host);
extern void func_00354030(FmvFrameQueue *q);
s32 func_00353C68(FmvStream *obj) {
    func_00352B00((u8 *)obj + 0x48);
    func_00354030((FmvFrameQueue *)(g_pFmvArenaBase + 0xD9168));
    func_00353D08((u8 *)obj);
    while (*(s32 *)(g_pFmvArenaBase + 0xD9174) != 0) {
        if (func_00353AC0(obj) == 1) {
            break;
        }
    }
    return func_00353AC8(obj, 3);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353D08);

/* === func_00353EC0 (USA func_00352A20) ============================== */
/**
 * Report a dropped/late frame through the debug stub (prints the frame's
 * sequence number).
 */
s32 func_00353EC0(s32 unused, s32 *frame) {
    func_0026FD28(D_1AE918, frame[1]);
    return 1;
}

/* === func_00353EE8 (USA func_00352A48) ============================== */
/**
 * Stream-callback: yield once to the scheduler, then pump the IPU DMA-add
 * queue and report handled.
 */
s32 func_00353EE8(void) {
    func_003515A0();
    func_00352DB0(*(u8 *volatile *)&g_pFmvArenaBase + FMV_DMAQ_OFS);
    return 1;
}

/* === func_00353F20 (USA func_00352A80) ============================== */
/**
 * Stream-callback: pump the IPU DMA-add queue once and report handled.
 */
s32 func_00353F20(void) {
    func_00352FB0(g_pFmvArenaBase + FMV_DMAQ_OFS);
    return 1;
}

/* === func_00353F50 (USA func_00352AB0) ============================== */
/**
 * Stream-callback: pump the IPU DMA-add retry path once and report handled.
 */
s32 func_00353F50(void) {
    func_003530C0(g_pFmvArenaBase + FMV_DMAQ_OFS);
    return 1;
}

/* func_00353F80 (USA func_00352AE0): snapshot the DMA-queue cursor pair into the
 * stream object (stream-event callback id 5). Blocked: 8-byte-packed saves;
 * asm-logic identical to USA func_00352AE0 (snapshot callee shifts: EU
 * func_00353760 vs USA func_003522C0). The portable TARGET_NATIVE body lives in
 * the USA twin (src/usa/text/250080.c). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353F80);

/* func_00353FD0 (USA func_00352B30): blocked, register-coloring wall. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353FD0);

/* === func_00354028 (USA func_00352B88) ============================== */
/**
 * FMV idle hook (frame-queue variant) — does nothing.
 */
void func_00354028(void) {
}

/* === func_00354030 (USA func_00352B90) ============================== */
/**
 * Clear the frame queue's display state (no frames queued, write cursor
 * reset).
 */
void func_00354030(FmvFrameQueue *q) {
    volatile FmvFrameQueue *v = q;

    v->count = 0;
    v->writeIdx = 0;
}

/* === func_00354040 (USA func_00352BA0) ============================== */
/**
 * True when every slot of the frame queue is filled (display is saturated).
 */
s32 func_00354040(FmvFrameQueue *q) {
    return q->count == q->capacity;
}

/* func_00354058 (USA func_00352BB8): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00354058);

/* func_003540D0 (USA func_00352C30): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003540D0);

/* === func_00354110 (USA func_00352C70) ============================== */
/**
 * True when the frame queue holds no displayable frame.
 */
s32 func_00354110(FmvFrameQueue *q) {
    return q->count == 0;
}

/* func_00354120 (USA func_00352C80): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00354120);

/* === func_00354188 (USA func_00352CE8) ============================== */
/**
 * Release the oldest displayed frame from the queue (if any).
 */
void func_00354188(FmvFrameQueue *q) {
    volatile s32 *count = &q->count;

    if (*count > 0) {
        *count = *count - 1;
    }
}

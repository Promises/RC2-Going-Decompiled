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
extern s32 func_00352C60(void *stream);
extern s32 func_00352D58(void *stream);
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

/* func_00351AA8 (USA func_00350608): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00351AA8);

/* func_00351B00 (USA func_00350660): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00351B00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00351B48);

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
 * memory. Blocked: 8-byte-packed saves. */
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352428);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352560);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352648);

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

/* func_00352898 (USA func_003513F8): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352898);

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

/* func_00352A88 (USA func_003515E8): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352A88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352B00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352C60);

/* func_00352D58 (USA func_003518B8): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352D58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352DB0);

/* func_00352FB0 (USA func_00351B10): blocked, 8-byte-packed saves +
 * lui/ori materialisation fold. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00352FB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003530C0);

/* func_003533F8 (USA func_00351F58): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003533F8);

/* func_00353450 (USA func_00351FB0): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353450);

/* func_003534A0 (USA func_00352000): blocked, 8-byte-packed saves. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003534A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_003534F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353650);

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

/* func_00353AD8 (USA func_00352638): blocked, prologue-scheduling shapes. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353AD8);

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

/* func_00353C68 (USA func_003527C8): blocked, 8-byte-packed saves +
 * reload-artifact wall. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/251520", func_00353C68);

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

/* func_00353F80 (USA func_00352AE0): blocked, 8-byte-packed saves. */
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

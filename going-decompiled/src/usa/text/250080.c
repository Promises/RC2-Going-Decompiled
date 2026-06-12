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
extern s32 func_00350910(void);
extern s32 func_001338C8(void);
extern s32 func_0012EE28(void);
extern s32 func_003517C0(void *stream);
extern s32 func_003518B8(void *stream);
extern s32 func_00351FB0(void *stream);
extern s32 func_00351B10(void *dmaq);
extern s32 func_00351C20(void *dmaq);
extern void func_003521B0(void *dmaq, void *cmd);
extern s32 DebugPrintStub(char *fmt, ...);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350520);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003506A8);

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
s32 func_00350840(FmvPtsQueue *q) {
    if (q->started != 0) {
        return func_00350910();
    }
}

/* func_00350868: SIF-DMA bounce of a decoded block to IOP memory. Blocked:
 * 8-byte-packed saves (s0/s1/s2/s3/ra). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350868);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350910);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350B60);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350F88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003510C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003511A8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351660);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003517C0);

/* func_003518B8: sema-guarded read-cursor advance. Blocked: 8-byte-packed
 * saves (s0/s1/ra). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003518B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351910);

/* func_00351B10: IPU/DMA state capture + reset. Blocked: 8-byte-packed
 * saves (s0@0x0, ra@0x8) plus lui/ori materialisation of 0x10002010 where
 * the pinned cc1 folds the low half into the load offset. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351B10);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352058);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003521B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003522C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352468);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352868);

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

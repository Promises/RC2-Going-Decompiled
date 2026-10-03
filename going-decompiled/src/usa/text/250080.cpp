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

/* R5900_SHORT_LOOP_PAD1(v, next): one `nop` emitted under noreorder, tied to the
 * loop's live value by its operands. A SCHEDULING DEVICE (RULING #8435; the pad
 * is the R5900 short-loop pad, FACT #7918 / #7937 / #8434): the ROM pads short
 * busy-wait loops with nops before the backward branch, which neither cc1 nor
 * our assembler inserts. EE arm only; on native it is nothing. */
#ifndef TARGET_NATIVE
#define R5900_SHORT_LOOP_PAD1(v, next) \
    __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+r"(v) : "r"(next))
#else
#define R5900_SHORT_LOOP_PAD1(v, next) ((void)0)
#endif

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

/* The pts queue control block (arena+0xD9100, 0x68 bytes — the gap to the next
 * arena block FMV_FRAMEQ_OFS at +0xD9168): a started flag and the
 * elementary-stream cursor state used by the header/payload walker.
 *
 * Word-index map confirmed from the accessors func_003506A8 (CONFIRMED, span
 * helper, word idx [0xc..0x10]), func_00350608 (commit, reads [5],[6],[0x12],
 * [0x13],[0x17]), func_00350660 (reset, clears [0xc],[0xe],[0xf],[0x11],[0x14],
 * [0x16],[0x17]) and func_00350830 (queued gate, [0x14]). Fields past +0x50
 * recovered from those accessors; ones with no inferable meaning are explicit
 * padding to the true block size (NOT invented). */
typedef struct FmvPtsQueue {
    /* 0x00 */ s32 started;   /* header consumed / stream live — CONFIRMED */
    /* 0x04 */ s32 mode;      /* stream type (4 = headerless) — CONFIRMED */
    /* 0x08 */ u8 hdr[0x28];  /* buffered packet header bytes — CONFIRMED */
    /* 0x30 */ s32 hdrFill;   /* header bytes buffered so far — CONFIRMED */
    /* 0x34 */ u8 *dataPtr;   /* payload ring base — CONFIRMED */
    /* 0x38 */ s32 ringOfs;   /* payload write offset (wraps) — CONFIRMED */
    /* 0x3C */ s32 consumed;  /* payload bytes consumed — CONFIRMED */
    /* 0x40 */ s32 ringSize;  /* payload ring size (1KB granular) — CONFIRMED */
    /* 0x44 */ s32 received;  /* payload bytes received in total — CONFIRMED */
    /* 0x48 */ s32 commitBase;/* idx[0x12]: base passed to func_001338f0 (commit) — PROBABLE */
    /* 0x4C */ s32 commitLen; /* idx[0x13]: length snapped to 1KB at commit — PROBABLE */
    /* 0x50 */ s32 queued;    /* idx[0x14]: bytes queued for the IPU — CONFIRMED */
    /* 0x54 */ s32 _pad54;    /* idx[0x15]: untouched in traced accessors — UNCONFIRMED */
    /* 0x58 */ s32 field58;   /* idx[0x16]: cleared on reset (func_00350660) — UNCONFIRMED */
    /* 0x5C */ s32 commitArg; /* idx[0x17]: arg5 to func_001338f0, cleared on reset — PROBABLE */
    /* 0x60 */ u8  pad60[0x8];/* to block extent 0x68 (FRAMEQ_OFS - PTS_OFS) — padding */
} FmvPtsQueue;

/* FMV stream object (arena+0xD9048, 0xB8 bytes — the gap to the next arena
 * block FMV_PTS_OFS at +0xD9100). Constructed by FmvStreamInit: a 5-slot
 * stream-event callback table at +0x00 (func_0012FA70 registers event ids
 * 0/1/2/3/5), an embedded bitstream/IPU-DMA sub-object at +0x48
 * (FmvBitstreamObjInit(obj+0x48,...) — this is FMV_DMAQ_OFS = STREAM+0x48),
 * and the playback FSM state word at +0xA8 (func_003525D0 reset=0, func_00352620
 * read, func_00352628 exchange, FmvRequestStop set=1). The 0x00..0xA8 span is
 * opaque here (callback table + bitstream object internals); only the FSM word
 * is named. Trailing bytes padded to the true block size. */
typedef struct FmvStream {
    /* 0x00 */ u8  pad[0xA8]; /* callback table + embedded bitstream obj (+0x48) — opaque */
    /* 0xA8 */ s32 state;     /* playback FSM (0=idle,1=stop,2=running,3=done) — CONFIRMED */
    /* 0xAC */ u8  padAC[0xC];/* to block extent 0xB8 (PTS_OFS - STREAM_OFS) — padding */
} FmvStream;

/* Full, bindable layouts — verified under ILP32 (the tester's host model and the
 * R5900 ABI both 4-byte pointer). ee-gcc 2.9 predates __SIZEOF_POINTER__ so the
 * checks are guarded; they're a no-op on the matching cross-compile but bind the
 * sizes the functional-equivalence tester allocates against. */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(FmvPtsRing)   == 0x5000C, "FmvPtsRing must be 0x5000C under ILP32");
_Static_assert(__builtin_offsetof(FmvPtsRing, readOfs) == 0x50000, "FmvPtsRing.readOfs");
_Static_assert(__builtin_offsetof(FmvPtsRing, size)    == 0x50008, "FmvPtsRing.size");

_Static_assert(sizeof(FmvFrameQueue) == 0x14, "FmvFrameQueue must be 0x14 under ILP32");
_Static_assert(__builtin_offsetof(FmvFrameQueue, writeIdx) == 0x08, "FmvFrameQueue.writeIdx");
_Static_assert(__builtin_offsetof(FmvFrameQueue, count)    == 0x0C, "FmvFrameQueue.count");
_Static_assert(__builtin_offsetof(FmvFrameQueue, capacity) == 0x10, "FmvFrameQueue.capacity");

_Static_assert(sizeof(FmvPtsQueue) == 0x68, "FmvPtsQueue must be 0x68 (PTS arena block) under ILP32");
_Static_assert(__builtin_offsetof(FmvPtsQueue, hdrFill)  == 0x30, "FmvPtsQueue.hdrFill");
_Static_assert(__builtin_offsetof(FmvPtsQueue, dataPtr)  == 0x34, "FmvPtsQueue.dataPtr");
_Static_assert(__builtin_offsetof(FmvPtsQueue, ringSize) == 0x40, "FmvPtsQueue.ringSize");
_Static_assert(__builtin_offsetof(FmvPtsQueue, queued)   == 0x50, "FmvPtsQueue.queued");
_Static_assert(__builtin_offsetof(FmvPtsQueue, commitArg)== 0x5C, "FmvPtsQueue.commitArg");

_Static_assert(sizeof(FmvStream) == 0xB8, "FmvStream must be 0xB8 (STREAM arena block) under ILP32");
_Static_assert(__builtin_offsetof(FmvStream, state) == 0xA8, "FmvStream.state");
#endif

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
extern s32 func_003521B0(void *dmaq, void *cmd);  /* returns 1 on enqueue, 0 if full (consumed by func_00352638) */
extern s32 DebugPrintStub(char *fmt, ...);
/* Callees of the s136os-arm bodies (func_003514E0, func_00351550, func_00351F58,
   func_003525D8), which the image compiles from C: visible to every arm, because
   C++ has no implicit declarations. */
extern void func_0011F5E0(void);   /* disable interrupts (DI) */
extern void func_0011F628(void);   /* enable interrupts (EI) */
extern void func_00351550(u32 chcrCmd); /* DMAC ch4 (IPU_TO) CHCR suspend write */
extern void func_0011AC30(s32 sema);   /* DeleteSema */
extern s32 func_00351F58(u8 *obj);
extern void func_0012F940(u8 *obj);

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
extern void func_00350868(u8 *stream, u8 *src, s32 len, s32 dstOfs);
/* func_003517C0/func_003518B8: deferred-native FMV stream funcs whose #else bodies
   model them with inconsistent arg counts across call sites (true signatures need the
   asm; FMV native backend is deferred). Declared with the stream pointer typed and the
   rest variadic (C++ reads an empty `()` as `(void)`), so the corpus compiles; resolve
   when the FMV native path is built. */
extern s32 func_003517C0(void *stream, ...);
extern s32 func_003518B8(void *stream, ...);
extern s32 func_00352638(u8 *obj, u64 a, u64 b, s32 pos, s32 n);
extern s32 func_003522C0(void *dmaq, ...);  /* FMV DMA-add-queue enqueue (deferred native; ret ignored) */
extern void ZeroQwords(void *p, s32 n);
extern s32 func_00133850(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern void *func_0012F738(void);
extern s32 func_0012FA70(u8 *obj, s32 slot, void *cb, s32 arg);
extern void func_003525D0(FmvStream *s);
extern s32 FmvBitstreamObjInit(u8 *obj, u64 a, u64 b, u64 c, u64 d, u64 e);
extern s32 func_0012F9A8(u8 *obj);
extern void func_0012F9C8(u8 *obj);
extern s32 func_0012F950(u8 *obj, s32 ptr, s32 len);
extern s32 FmvFrameQueueGetWriteSlot(u8 *fq);
extern void FmvFrameQueuePush(u8 *fq);
extern void func_00350B60(void *gif, u8 *frame, u32 a, u32 b, s32 idx);
extern void WaitSema(s32 sema);
extern s32 SignalSema(s32 sema);   /* SDK SignalSema returns int (eekernel.h); func_00352000 returns it */
extern s32 func_00133960(void);
/* Stream-event callbacks registered by FmvStreamInit (defined later). */
s32 func_00352A20(s32 unused, s32 *frame);
s32 func_00352A48(void);
s32 func_00352A80(void);
s32 func_00352AB0(void);
/* func_00352AE0 is a stream-event callback (id 5): the dispatcher passes the
   event id in arg0 (unused) and the stream object in arg1. */
extern s32 func_00352AE0(s32 unused, u8 *obj);
/* Stream/host helper referenced only by the #else bodies. */
extern s32 func_0012F9B8(u8 *host);
s32 FmvFrameQueueGetDisplaySlot(FmvFrameQueue *q);
/* Stream-commit + reset callees referenced only by the #else bodies. */
extern s32 func_001338F0(s32 commitBase, s32 len, s32 commitArg, s32 queued, s32 arg5);
extern void func_00133890(s32 *obj);   /* commit-path stream lock */
/* SIF-DMA bounce primitives (func_00350868 #else): queue a SIF DMA, busy-wait
   for the slot, poll for completion, then signal. */
extern s32 func_0011AEA0(s32 mode);    /* sceSifSetDChain / SIF DMA arm */
extern s32 func_0011AFE0(void *desc, s32 count);  /* sceSifSetDma (returns id) */
extern s32 func_0011AFC0(s32 id);      /* sceSifDmaStat (busy while >= 0) */
extern void func_00133930(s32 len, s32 dstOfs);  /* post-transfer notify */
/* Semaphore callees (CreateSema/WaitSema/SignalSema) referenced only by the #else bodies. */
extern s32 func_0011AC20(void *param); /* CreateSema (returns sema id) */
extern s32 func_0011AC60(s32 sema);    /* WaitSema (acquire) */
extern s32 func_0011AC40(s32 sema);    /* SignalSema (release) */
void FmvStreamStartDma(u8 *stream);        /* ring init, defined below (fwd for FmvBitstreamObjInit) */
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

/* func_003503D8: FMV engine shutdown — flat teardown: quiesce (func_00124B88),
 * stop the IPU/decode helpers (func_003512F0/func_00352B88), kill+delete the FMV
 * thread (func_0011AA70/func_0011AA30 over g_fmvThreadId), disable DMAC ch2, close
 * the vblank handler (func_0011A950 + func_00126DC0(OnVblankInterrupt)), tear down the
 * DMA queues (func_003525D8/func_003505E0/func_003513F0 over the arena sub-objects),
 * quiesce again, and clear bit 1 of the INTC-mask reg 0x1000E000.
 * Blocked (match): the original mixes %gp_rel and absolute %hi/%lo accesses to
 * g_pFmvArenaBase/g_fmvThreadId in one function — only one form per declaration.
 * NEEDS-TESTER-ORACLE: the arena-relative args the asm passes to the void(void)
 * callees (func_003512F0/func_00352B88/func_003505E0/func_003513F0 — func_00352B88 is
 * a confirmed void(void) 2.9 match) are dead-passed in the original and dropped here;
 * the oracle should confirm faithfulness. */
#ifdef TARGET_NATIVE
extern void func_0011AA70(s32 threadId);   /* kill thread */
extern void func_0011AA30(s32 threadId);   /* delete thread */
extern void DisableDmac(s32 channel);
extern void func_0011A950(s32 a, s32 b);
extern void func_00126DC0(void *handler);    /* remove vblank handler */
extern s32  g_fmvThreadId;
extern void OnVblankInterrupt(void);
/* forward decls — these are declared/defined later in this file */
extern s32  func_00124B88(s32 mode);
extern void func_003512F0(void);
extern void func_00352B88(void);
extern s32  func_003525D8(FmvStream *obj);
extern s32  func_003505E0(void);
extern s32  func_003513F0(void);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003503D8);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_00124B88(s32 mode);
extern void func_003512F0(void);
extern void func_00352B88(void);
extern void func_0011AA70(s32 threadId);
extern void func_0011AA30(s32 threadId);
extern void DisableDmac(s32 channel);
extern void func_0011A950(s32 a, s32 b);
extern void func_00126DC0(void *handler);
extern s32 func_003525D8(FmvStream *obj);
extern s32 func_003505E0(void);
extern s32 func_003513F0(void);
extern s32 g_fmvThreadId;
extern void OnVblankInterrupt(void);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 75.25% / engine96 61.41%. Residual: GPREL-DELAY-SLOT: with the dead-passed arena args restored and func_126DC0 declared s32 the sdk29 arm reads 95.59% and the ONLY residual is the `lw a0,%gp_rel(g_pFmvArenaBase)($gp)` in the jal func_003512F0 delay slot, which GNU as expands as lui/lw+nop; rewriting that one line of base.s to the %gp_rel form assembles to 100.00% (tools/ee/.t513/probe/base_gprel.s) — a toolchain post-pass question, not a C one. */
void func_003503D8(void) {
    func_00124B88(0);
    func_003512F0();
    func_00352B88();
    func_0011AA70(g_fmvThreadId);
    func_0011AA30(g_fmvThreadId);
    DisableDmac(2);
    func_0011A950(2, *(s32 *)(g_pFmvArenaBase + 0xD90F8));
    func_00126DC0((void *)OnVblankInterrupt);
    func_003525D8((FmvStream *)(g_pFmvArenaBase + 0xD9048));
    func_003505E0();
    func_003513F0();
    func_00124B88(0);
    *(volatile u32 *)0x1000E000 &= 0xFFFFFFFD;
}
#endif

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", FmvPtsQueueInit);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void ZeroQwords(void *p, s32 n);
extern s32 func_00133850(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern u8 *D_1B2354;
/* (end of this body's declarations) */
/* The EE arm names the qword filler and the decode-buffer pointer by their ROM
 * symbols (func_00283438, D_001B2354; the native runtime knows them as
 * ZeroQwords and D_1B2354). The pointer store is the ROM's absolute `lui $1;
 * sw` pair: an ADDRESSING-MODEL DEVICE, `.extern ,16` makes the assembler
 * expand cc1's one-insn `sw` macro that way. */
#ifndef TARGET_NATIVE
__asm__(".extern D_001B2354, 16");
extern u8 *D_001B2354;
extern void func_00283438(void *p, s32 n);
#define FMV_PTS_DECODE_BUF D_001B2354
#define FMV_ZERO_QWORDS func_00283438
#else
#define FMV_PTS_DECODE_BUF D_1B2354
#define FMV_ZERO_QWORDS ZeroQwords
#endif
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 71.55% / engine96 69.85%. Residual: PACKED-SAVE (5 callee saves) + 29 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves. Revisit with the gameplay-TU compiler.

   Construct the pts-queue object: zero the 0x20-quadword header, stash the
   payload ring base (param2) and IPU scratch base (param3), seed the stream
   type (3) and the 0x400-byte ring granularity, record the decode output
   buffer in D_1B2354, allocate the IPU sema (func_00133850), and report
   whether the allocation succeeded.

   SCREEN (task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
   masked; a candidate, NOT match evidence): EXACT 47/47, relocations equal,
   with three levers, each measured undone: the `.extern ,16` store above (else
   14/47, first diff @33 `sw $19,%gp_rel`); func_00133850 returning s32, which is
   what the wrapper passes back from snd_SendCommandSync (s64 costs a
   dsll32/dsra32 pair: 12/47); and the result written as an early `return 0`
   (`sema >= 0` folds to nor/srl: 2/47, first diff @38). */
s32 FmvPtsQueueInit(s32 *obj, s32 ringBase, s32 scratchBase, u8 *decodeBuf) {
    s32 sema;

    FMV_ZERO_QWORDS(obj + 2, 0x20);
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
    FMV_PTS_DECODE_BUF = decodeBuf;
    obj[0x13] = 0x400;
    sema = func_00133850(0x400, 0x1000, 0x400, 0, 5, 3);
    obj[0x12] = sema;
    if (sema < 0) {
        return 0;
    }
    return 1;
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350600);

#ifndef TARGET_NATIVE
/* func_00350608: read-chunk dispatch on the stream object. Blocked:
 * 8-byte-packed saves (s0@0x0, ra@0x8; see header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350608);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_001338F0(s32 commitBase, s32 len, s32 commitArg, s32 queued, s32 arg5);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 52.05% / engine96 33.50%. Residual: PACKED-SAVE (2 callee saves) + 16 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0@0x0, ra@0x8). Revisit with the gameplay-TU compiler.

   Commit the buffered packet to the IPU bitstream feeder: snap the staged
   commit length (commitLen) down to a 1KB (0x400) boundary - biasing a negative
   value by +0x3FF first so the arithmetic >>10<<10 floors toward zero the same
   way the compiler's sra/sll pair does - then hand {commitBase, snappedLen,
   commitArg, hdr-word@0x14, hdr-word@0x18} to func_001338F0 and flip the
   stream's `started` word to 2 (header consumed). Returns 2 (the started value
   is reused as the return - the func_001338F0 result is discarded).

   NOTE(offset): args 4 and 5 come from word idx [5] (q+0x14) and [6] (q+0x18),
   both inside the packet-header staging area hdr[0x28] (NOT q->queued@0x50 - the
   cmp oracle caught an earlier version that read queued). Raw offsets kept. */
s32 func_00350608(FmvPtsQueue *q) {
    s32 len = q->commitLen;
    s32 snapped = ((len >= 0 ? len : len + 0x3FF) >> 10) << 10;

    func_001338F0(q->commitBase, snapped, q->commitArg,
                  *(s32 *)((u8 *)q + 0x14), *(s32 *)((u8 *)q + 0x18));
    q->started = 2;
    return 2;
}
#endif

/* func_00350660: stream-state reset. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00350660)
S136OS_SLOT(func_00350660);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_00133890(s32 *obj);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 98.65% / engine96 87.88%. Residual: PACKED-SAVE (2 callee saves) + 8 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/** func_00350660 - reset the pts-queue stream cursor to "no packet buffered":
 *  acquire the stream lock (func_00133890), then clear the started flag, the
 *  header-staging fill, the payload ring offset/consumed/received cursors, the
 *  queued count and the two commit scratch words (field58 / commitArg). The
 *  payload base, ring size and mode are left intact.
 *  @param q  the pts queue control block
 * MATCHED on the s136os arm (task #1370): the 8-byte save slots are SN 2.95.3
 * v1.36 -fopt-stack's (FACT #8810). The ROM clears commitArg (+0x5C) FIRST and
 * this compiler emits the last store of a group that ends at the epilogue
 * first, so commitArg is written LAST here (last-store-first, FACT #8947). In
 * the ROM's own order the s136os output is 6 of 17 words off.
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
void func_00350660(FmvPtsQueue *q) {
    func_00133890((s32 *)q);
    q->started = 0;
    q->hdrFill = 0;
    q->ringOfs = 0;
    q->consumed = 0;
    q->received = 0;
    q->queued = 0;
    q->field58 = 0;
    q->commitArg = 0;
}
#endif

/**
 * Compute the two writable spans of the pts payload ring (the producer's
 * scatter region). Returns (ptr,len) of the leading span through pPtr0/pLen0
 * and of the wrapped tail span through pPtr1/pLen1:
 *   - stream not started: either the buffered-header window (mode 4 = whole
 *     ring from dataPtr) or the 0x28-byte packet-header staging area at
 *     (q + hdrFill + 8);
 *   - stream live: the free region after ringOfs, split where it wraps.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003506A8);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 40.29% / engine96 5.87%. Residual: REGALLOC on sdk29 (a5-relative loads coloured v0/a0, beql/bnel inversion); not iterated (72 insns). */
/* TODO(match): functional equivalent - not byte-exact (sdk29 40.29%, unit
   objdiff report, task #513 line above); the original schedules the
   started/mode tests as branch-likely (bnel/beql) pairs. The pinned cc1's
   reorg DOES emit bnel/beql/bnezl (NOTE #8503, FACT #8614, func_001231C8's
   matched `bnel` at 022FA8.c:146), so branch-likely is not shown to be this
   function's wall; no lever run on it is recorded (task #1230 / NOTE #8761).
   Revisit with those levers or the gameplay-TU compiler. */
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

#ifndef TARGET_NATIVE
/* func_00350868: SIF-DMA bounce of a decoded block to IOP memory. Blocked:
 * 8-byte-packed saves (s0/s1/s2/s3/ra). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350868);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0011AEA0(s32 mode);
extern s32 func_0011AFE0(void *desc, s32 count);
extern s32 func_0011AFC0(s32 id);
extern void func_00133930(s32 len, s32 dstOfs);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 81.88% / engine96 66.19%. Residual: PACKED-SAVE (5 callee saves) + 12 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0/s1/s2/s3/ra).

   ROUTE: tester-EE-pending (NOT isolated-cmp-oracleable) - the two do/while
   loops busy-wait on real SIF-DMA hardware (func_0011AFE0 returns 0 until a DMA
   slot frees; func_0011AFC0 returns >= 0 while the transfer is in flight), so an
   isolated EE with mocked SIF stubs would hang or pass vacuously. Verify under
   the live tester-EE FMV-playback scenario instead.

   Bounce a decoded 0x400 block from EE (src) to the IOP at the stream's IOP
   buffer base (obj[0x48]) + dstOfs: build a 4-word SIF-DMA descriptor
   {src, iopBase, len, 0}, arm the chain (func_0011AEA0), retry the enqueue until
   a slot is granted, wait for completion, then notify (func_00133930).

   SCREEN (task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
   masked; a candidate, NOT match evidence): EXACT 42/42, relocations equal. The
   two wait loops carry the ROM's R5900 short-loop pad nops (2 and 3) before
   their backward branches (R5900_SHORT_LOOP_PAD1, a scheduling device, RULING
   #8435), and an EMPTY fence after the chain-arm call (RULING #8483, emits
   nothing) makes cc1 load the IOP base into $2 as the ROM does. Fence removed:
   2/42, first diff @12 (`lw $3` vs `lw $2`); pads removed: 21/42; both: 23/42. */
void func_00350868(u8 *obj, u8 *src, s32 len, s32 dstOfs) {
    s32 desc[4];
    s32 id;
    s32 stat;

    func_0011AEA0(0);
    __asm__ __volatile__("");
    desc[0] = (s32)src;
    desc[1] = *(s32 *)(obj + 0x48);
    desc[2] = len;
    desc[3] = 0;
    do {
        id = func_0011AFE0(desc, 1);
        R5900_SHORT_LOOP_PAD1(id, id);
        R5900_SHORT_LOOP_PAD1(id, id);
    } while (id == 0);
    do {
        stat = func_0011AFC0(id);
        R5900_SHORT_LOOP_PAD1(stat, stat);
        R5900_SHORT_LOOP_PAD1(stat, stat);
        R5900_SHORT_LOOP_PAD1(stat, stat);
    } while (stat >= 0);
    func_00133930(len, dstOfs);
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350910);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_00133960(void);
extern void func_00350868(u8 *stream, u8 *src, s32 len, s32 dstOfs);
extern u8 *D_1B2354;
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 30.39% / engine96 34.69%. Residual: PACKED-SAVE (4 callee saves) + 152 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
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
        rdy = 1;   /* avail stays 0 here, so (s32)avail < 0x400 == 1 -> skips the drain */
    } else if (state == 1) {
        if (st[0xF] < 0x1000) {
            return;
        }
        avail = 0x1000 - st[0x14];
        rdy = (s32)avail < 0x400;
    } else {
        rdy = 1;   /* avail stays 0 here, so (s32)avail < 0x400 == 1 -> skips the drain */
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

/* func_00350F28: vblank field-sync waiter — spin while WaitVblankGetField(0) still
 * reports `targetField` and progressive-scan is off; once the field changes (or
 * progressive is on) latch D_1AE788=1 / D_1AE78C=0 and return 1.
 * Blocked: 8-byte-packed saves (s0@0x0, ra@0x8) — under 2.9 -G8 -fno-gcse this C
 * compiles a 16-byte-slot 0x20 frame; the original packs 8-byte in a 0x10 frame
 * (the genuine save-slot wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350F28);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 84.05% / engine96 76.10%. Residual: PACKED-SAVE (2 callee saves) + 9 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee saves. */
extern s32 WaitVblankGetField(s32 mode);
extern s32 g_bProgressiveScan;
s32 func_00350F28(s32 targetField) {
    while (WaitVblankGetField(0) == targetField && g_bProgressiveScan == 0) {
    }
    D_1AE788 = 1;
    D_1AE78C = 0;
    return 1;
}
#endif

/**
 * Clear the "frame displayed this vblank" latch.
 */
void func_00350F78(void) {
    D_1AE788 = 0;
}

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350F88);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_00352638(u8 *obj, u64 a, u64 b, s32 pos, s32 n);
extern char D_1AE7E8[];
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 72.19% / engine96 53.51%. Residual: PACKED-SAVE (7 callee saves) + 35 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
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
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 79.61% / engine96 54.65%. Residual: PACKED-SAVE (6 callee saves) + 24 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
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
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 66.32% / engine96 51.71%. Residual: PACKED-SAVE (9 callee saves) + 50 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
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
/**
 * Commit up to @n bytes into the pts ring: clamps the request to the free
 * space (size - fill), advances both the fill count and the wrapped read
 * offset by that amount, and returns the number of bytes actually committed.
 *
 * @param base ring base (header at base+0x50000)
 * @param n    bytes the caller wishes to commit
 * @return     bytes committed (= min(n, size - fill))
 *
 * readOfs = (readOfs + take) % size (the div whose quotient is discarded and
 * remainder mfhi'd back); fill += take is stored before the readOfs store;
 * the return is `take` from the movn.
 *
 * Byte-exact on the sdk29 arm (cc1 2.9-ee-991111 -O2 -G8 -fno-gcse; task
 * #513): unit objdiff 100.00% (objdiff_build.sh + unit_report.sh). Two
 * spellings matter: the clamp is `if (n < space) space = n` on the SAME
 * variable (gives the ROM's `slt; movn` with space as the default operand,
 * see func_003513B8), and `size` is loaded before `fill` so the register
 * allocator colours size=a3 / fill=a4 and the div-by-zero `beqzl a3; break`
 * is scheduled right after the loads, as in the ROM. The ternary spelling
 * with fill loaded first scored 74.12% (sdk29) / 20.59% (engine96).
 */
s32 func_00351328(u8 *base, s32 n) {
    FmvPtsRing *ring = (FmvPtsRing *)base;
    s32 size = ring->size;
    s32 fill = ring->fill;
    s32 readOfs = ring->readOfs;
    s32 space = size - fill;

    if (n < space) {
        space = n;
    }
    readOfs += space;
    fill += space;
    ring->readOfs = readOfs % size;
    ring->fill = fill;
    return space;
}

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
/**
 * Consume up to @n bytes from the pts ring: drops min(n, fill) bytes off the
 * queued count, leaving the read offset untouched (the caller wraps it), and
 * returns the number of bytes actually dropped.
 *
 * @param base ring base (header at base+0x50000)
 * @param n    bytes the caller wishes to drop
 * @return     bytes dropped (= min(n, fill))
 *
 * Only the fill field (base+0x50004) is read. The STORED-back value is
 * `fill - take` but the RETURN is `take` itself — the function reports the
 * consumed count, not the remainder. (The cmp oracle caught an earlier
 * version that returned the remainder.)
 *
 * Byte-exact on the sdk29 arm (cc1 2.9-ee-991111 -O2 -G8 -fno-gcse; task
 * #513): unit objdiff 100.00% (objdiff_build.sh + unit_report.sh). The
 * clamp is written as `if (n < fill) fill = n` on the SAME variable so cc1
 * emits the ROM's `slt; movn` with fill as the default operand — the
 * `take = (n < fill) ? n : fill` spelling gives `movz` with n as default
 * (66.11% on both arms).
 */
s32 func_003513B8(u8 *base, s32 n) {
    FmvPtsRing *ring = (FmvPtsRing *)base;
    s32 fill = ring->fill;
    s32 rem = fill;

    if (n < fill) {
        fill = n;
    }
    ring->fill = rem - fill;
    return fill;
}

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

/* func_003513F8(obj, buf, byteLen, flag): kick off a CD sector read for the FMV
 * bitstream. Converts byteLen to 2KB sectors (>>11), issues sceCdRead
 * (func_001253A8) from the object's current LBN cursor (obj+0x4) into buf with a
 * fixed retry mode {trycount=0x64, spindlctrl=1, datapattern=0}. When flag != 0
 * it returns 0 without advancing (query/prime mode); otherwise it advances the
 * LBN cursor by the sector count, waits (sceCdSync, func_00124B88(0)) and returns
 * byteLen. Matching arm stays asm (8-byte-packed saves s0..s4/ra); the #else is
 * the structure-exact model (the CD I/O itself is the deferred FMV native
 * backend, but the call structure + cursor advance are exact). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003513F8);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 64.74% / engine96 77.79%. Residual: PACKED-SAVE (6 callee saves) + 19 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
extern s32 func_001253A8(u32 lbn, u32 sectors, void *buf, void *mode); /* sceCdRead */
extern s32 func_00124B88(s32 mode);                                    /* sceCdSync */
s32 func_003513F8(u8 *obj, void *buf, s32 byteLen, s32 flag) {
    u8 mode[4];
    s32 sectors = byteLen >> 11; /* bytes -> 2KB sectors */

    mode[0] = 0x64; /* trycount   */
    mode[1] = 1;    /* spindlctrl */
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

/* func_003514E0: write a DMAC channel CHCR under the ENABLEW suspend protocol —
 * suspend (ENABLEW = ENABLER | 0x10000), write the CHCR command, resume
 * (ENABLEW = ENABLER & ~0x10000), bracketed by func_0011F5E0/func_0011F628.
 * cc1 2.9 -G8 -fno-gcse emits a 16-byte-slot 0x20 frame vs the ROM's 8-byte
 * 0x10 (s0@0x0, ra@0x8); MATCHED on the s136os arm, which emits the packed frame
 * (FACT #8830). The volatile accesses are the DMAC/ENABLEW MMIO registers. */
/* GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003514E0)
S136OS_SLOT(func_003514E0);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 75.44% / engine96 56.11%. Residual: PACKED-SAVE (2 callee saves) + 7 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
void func_003514E0(u32 chcrCmd) {
    func_0011F5E0();   /* DI */
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 | 0x10000;
    *(volatile u32 *)0x1000B000 = chcrCmd;
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 & 0xFFFEFFFF;
    func_0011F628();   /* EI */
}
#endif

/* func_00351550: write DMAC ch4 (IPU_TO) CHCR under the ENABLEW channel-suspend
 * protocol — suspend (ENABLEW = ENABLER | 0x10000), write the CHCR command,
 * resume (ENABLEW = ENABLER & ~0x10000), all bracketed by DI/EI. The ch4
 * companion of func_003514E0 (which does the same for ch3/IPU_FROM at
 * 0x1000B000). cc1 2.9 cannot pack its saves (s0@0x0, ra@0x8); MATCHED on the
 * s136os arm (FACT #8830). */
/* GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00351550)
S136OS_SLOT(func_00351550);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 75.44% / engine96 56.11%. Residual: PACKED-SAVE (2 callee saves) + 7 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
void func_00351550(u32 chcrCmd) {
    func_0011F5E0();   /* DI */
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 | 0x10000;
    *(volatile u32 *)0x1000B400 = chcrCmd;
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 & 0xFFFEFFFF;
    func_0011F628();   /* EI */
}
#endif

/**
 * Build a 64-bit IPU_TO DMA tag word: madr in the high word, the quadword
 * count field at bit 28, the tag id bits in the low word.
 */
void func_003515C0(u64 *tag, u64 madr, u64 qwc, u64 id) {
    *tag = madr << 32 | (qwc << 32) >> 4 | (id << 32) >> 32;
}

/* FmvBitstreamObjInit: construct the IPU_TO bitstream sub-object. Stores the source
 * buffer/tag pointers and the two ring counts, builds the +0x4 DMA-tag word
 * ((b & 0x0FFFFFFF) | 0x20000000 = a "next" tag pointing at physical b), creates
 * the decode semaphore (init/max = 1) into +0x40, initialises the ring via
 * FmvStreamStartDma, and clears the +0x48 pts accumulator. Always returns 1. The
 * u64 params match the call site (FmvStreamInit); the body uses their low 32
 * bits (the ROM stores them with sw). Byte-match blocked: 8-byte-packed saves. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", FmvBitstreamObjInit);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0011AC20(void *param);
extern void FmvStreamStartDma(u8 *stream);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 38.57% / engine96 0.00%. Residual: PACKED-SAVE (2 callee saves) + 25 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
s32 FmvBitstreamObjInit(u8 *obj, u64 a, u64 b, u64 c, u64 d, u64 e) {
    s32 semaParam[8];

    *(u32 *)(obj + 0x0) = (u32)a;
    *(u32 *)(obj + 0x50) = (u32)d;
    *(u32 *)(obj + 0x54) = (u32)e;
    semaParam[1] = 1;   /* init count */
    semaParam[2] = 1;   /* max count  */
    *(u32 *)(obj + 0x4) = ((u32)b & 0x0FFFFFFF) | 0x20000000;
    *(u32 *)(obj + 0x18) = (u32)c << 11;
    *(u32 *)(obj + 0x8) = (u32)c;
    *(u32 *)(obj + 0x40) = func_0011AC20(semaParam);
    FmvStreamStartDma(obj);
    *(s64 *)(obj + 0x48) = 0;
    return 1;
}
#endif

/* FmvStreamStartDma: initialise the IPU_TO bitstream sub-object's ring state and
 * build its DMA source-tag chain. Clears the playback counters, resets each
 * f50-array ring slot (stride 0x18: two -1 handles + two zeroed counters),
 * emits one 0x80-qwc IPU_TO source tag per macroblock via func_003515C0
 * (id 0x80) followed by the qwc=2 terminator, then programs channel-4
 * MADR/QWC/TADR and suspends its CHCR via func_00351550(5). Operates on the
 * sub-object embedded at FmvStream+0x48 (type not yet recovered -> raw
 * offsets). Byte-match blocked: 8-byte-packed saves. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", FmvStreamStartDma);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 67.90% / engine96 63.94%. Residual: PACKED-SAVE (4 callee saves) + 61 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
void FmvStreamStartDma(u8 *stream) {
    s32 i;

    *(s32 *)(stream + 0x44) = 1;
    *(s32 *)(stream + 0xC) = 0;
    *(s32 *)(stream + 0x10) = 0;
    *(s32 *)(stream + 0x14) = 0;
    *(s32 *)(stream + 0x58) = 0;
    *(s32 *)(stream + 0x5C) = 0;

    if (*(s32 *)(stream + 0x54) > 0) {
        for (i = 0; i < *(s32 *)(stream + 0x54); i++) {
            u8 *slot = *(u8 **)(stream + 0x50) + i * 0x18;
            *(s64 *)(slot + 0x0) = -1;
            *(s64 *)(slot + 0x8) = -1;
            *(s32 *)(slot + 0x10) = 0;
            *(s32 *)(slot + 0x14) = 0;
        }
    }

    i = 0;
    if (*(s32 *)(stream + 0x8) > 0) {
        for (i = 0; i < *(s32 *)(stream + 0x8); i++) {
            func_003515C0((u64 *)(*(u32 *)(stream + 0x4) + i * 0x10),
                          (*(u32 *)(stream + 0x0) + (i << 11)) & 0x0FFFFFFF,
                          3, 0x80);
        }
    }

    func_003515C0((u64 *)(*(u32 *)(stream + 0x4) + i * 0x10),
                  *(u32 *)(stream + 0x4) & 0x0FFFFFFF, 2, 0);

    *(volatile u32 *)0x1000B420 = 0;                                    /* ch4 QWC  */
    *(volatile u32 *)0x1000B410 = *(u32 *)(stream + 0x0) & 0x0FFFFFFF;  /* ch4 MADR */
    *(volatile u32 *)0x1000B430 = *(u32 *)(stream + 0x4) & 0x0FFFFFFF;  /* ch4 TADR */
    func_00351550(5);                                                   /* ch4 CHCR suspend */
}
#endif

/* WALL: deferred-native body had a signature inconsistency with its
   forwarder/caller (caught by the TARGET_NATIVE compile sweep). Left bare
   INCLUDE_ASM (no #else); revisit with the asm when the FMV native backend
   is built. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003517C0);

/* func_003518B8: sema-guarded read-cursor advance. Blocked: 8-byte-packed
 * saves (s0/s1/ra). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003518B8);

/* func_00351910: advance the IPU_TO (DMAC ch4) DMA source-tag RING by the
 * macroblocks the channel already consumed, re-emit the freshly-freed tags, and
 * re-kick the channel. Acquires the stream sema (+0x40) — if the "armed" flag
 * (+0x44) is clear the transfer was torn down, so it reports the FMV error
 * (D_1AE800) and returns 0. Otherwise it suspends ch4 (func_00351550(5)),
 * snapshots the channel CHCR/MADR, and translates MADR into a ring position via
 * func_00351498. The object is the IPU_TO bitstream sub-object (raw offsets, as
 * in the siblings):
 *   +0x0  srcBase       physical base address of the macroblock buffer
 *   +0x4  tagBase       physical base of the source-tag ring (0x10-byte tags)
 *   +0x8  ringSize (N)  number of macroblock tags in the ring
 *   +0xC  head          ring index of the oldest still-outstanding tag
 *   +0x10 outstanding   count of tags the DMAC still owns
 *   +0x14 ptsAccum      pts accumulator (macroblocks-worth, granularity 0x800)
 *   +0x40 sema          decode semaphore id
 *   +0x44 armed         non-zero while the channel is live
 * The consumed count is derived from how far MADR advanced: sectorDelta =
 * func_00351498(obj, madr); the number of whole macroblocks consumed rolls
 * `head` forward and `outstanding` down (mod N ring math). ptsAccum/0x800 gives
 * how many new tags to (re)emit; each is rebuilt via func_003515C0 with qwc 3
 * (0x80-id source tag), the final tag of the batch terminated with qwc 0. If any
 * tag was emitted, ch4's CHCR is restarted (patched to 0x30000000 | 0x100 =
 * chained transfer, dir=to-memory-off, start). Returns 1. Op-for-op faithful to
 * the frozen .s (all divides signed `div`; the beql break-0,7 guards are the
 * compiler's div-by-zero traps for a possibly-zero divisor -> plain % here). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351910);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0011AC60(s32 sema);
extern s32 func_0011AC40(s32 sema);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 78.06% / engine96 70.31%. Residual: PACKED-SAVE (9 callee saves) + 72 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
s32 func_00351910(void *dmaq) {
    extern char D_1AE800[];   /* FMV "IPU_TO ring not armed" error string */
    s32 *obj = (s32 *)dmaq;
    s32 ringSize;         /* $7  = obj[2] (N) */
    s32 sectorDelta;      /* $2  = func_00351498(obj, madr) */
    s32 head;             /* running ring head (obj[3]) */
    s32 outstanding;      /* running outstanding count (obj[4]) */
    s32 emitIdx;          /* $16 = ring index to (re)emit at */
    s32 nTags;            /* $19 = ptsAccum / 0x800, tags to emit */
    s32 ptsAccum;         /* $1  = obj[5] before wrap */
    s32 ptsQuot;          /* $9  = rounded-toward-zero ptsAccum / 0x800 */
    u32 chcr;             /* $21 = ch4 CHCR snapshot */
    u32 madr;             /* $5  = ch4 MADR snapshot */
    s32 emitted;          /* $23 = flag: at least one tag emitted */
    s32 i;                /* $17 = emit-loop counter */

    func_0011AC60(obj[0x10]);                 /* WaitSema (acquire) */
    if (obj[0x11] == 0) {                      /* +0x44 armed clear -> torn down */
        func_003504C8(D_1AE800);
        return 0;
    }

    emitted = 0;
    func_00351550(5);                          /* suspend ch4 (IPU_TO) */
    chcr = *(volatile u32 *)0x1000B400;        /* ch4 CHCR */
    madr = *(volatile u32 *)0x1000B410;        /* ch4 MADR */

    sectorDelta = func_00351498((u32 *)dmaq, madr);
    ringSize = obj[2];                         /* N */

    /* consumed macroblocks -> roll head forward, outstanding down */
    sectorDelta = ((sectorDelta + ringSize) - obj[3]) % ringSize;
    outstanding = obj[4] - sectorDelta;
    head = (obj[3] + sectorDelta) % ringSize;
    obj[4] = outstanding;
    outstanding = head + outstanding;          /* head + outstanding */
    obj[3] = head;
    emitIdx = outstanding % ringSize;

    /* ptsAccum / 0x800, rounded toward zero (bias +0x7FF when negative) */
    ptsAccum = obj[5];
    ptsQuot = (ptsAccum >= 0 ? ptsAccum : ptsAccum + 0x7FF) >> 11;
    nTags = ptsQuot;
    obj[5] = ptsAccum - ptsQuot * 0x800;       /* keep the remainder */

    if (nTags > 0) {
        /* prime tag: the slot just behind the batch (outstanding + N - 1) */
        s32 idx = (outstanding + ringSize - 1) % ringSize;
        emitted = 1;
        func_003515C0((u64 *)(obj[1] + idx * 0x10),
                      (u64)(u32)(obj[0] + idx * 0x800), 3, 0x80);
    }

    if (nTags > 0) {
        for (i = 0; i < nTags; i++) {
            /* last tag of the batch terminates the chain (qwc 0) */
            u64 qwc = (i != nTags - 1) ? 3 : 0;
            func_003515C0((u64 *)(obj[1] + emitIdx * 0x10),
                          (u64)(u32)(obj[0] + emitIdx * 0x800), qwc, 0x80);
            emitIdx = (emitIdx + 1) % obj[2];
        }
    }

    outstanding = obj[4];
    obj[4] = outstanding + nTags;
    if (outstanding + nTags != 0) {
        if (emitted) {
            chcr = (chcr & 0x0FFFFFFF) | 0x30000000;
        }
        func_00351550(chcr | 0x100);           /* restart ch4 */
    }

    func_0011AC40(obj[0x10]);                   /* SignalSema (release) */
    return 1;
}
#endif

/* func_00351B10: snapshot + halt the IPU DMA channels (save-state capture).
 * Acquires the stream sema, suspends IPU_TO (ch4, func_00351550(5)) and records
 * its MADR/TADR/QWC/CHCR into dmaq+0x1C..0x28, spin-waits for the IPU to drain
 * (IPU_CTRL & 0xF0), suspends IPU_FROM (ch3, func_003514E0(0)) and records its
 * MADR/QWC/CHCR + IPU_BP/IPU_CTRL into dmaq+0x2C..0x3C, then releases the sema.
 * Returns 1. Raw offsets (dmaq sub-object type not recovered).
 * SCREEN (task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
 * masked; a candidate, NOT match evidence): EXACT 68/68, relocations equal. The
 * drain loop carries the ROM's three R5900 short-loop pad nops before its bnez
 * (R5900_SHORT_LOOP_PAD1, a scheduling device); without them the screen is
 * 34/68, built 65 words, first diff @29. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351B10);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0011AC60(s32 sema);
extern void func_003514E0(u32 chcrCmd);
extern s32 func_0011AC40(s32 sema);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 91.84% / engine96 60.10%. Residual: PACKED-SAVE (2 callee saves) + 18 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
s32 func_00351B10(void *dmaq) {
    u8 *obj = (u8 *)dmaq;

    func_0011AC60(*(s32 *)(obj + 0x40));                    /* acquire */
    *(u32 *)(obj + 0x44) = 0;
    func_00351550(5);                                       /* suspend IPU_TO (ch4) */
    *(u32 *)(obj + 0x1C) = *(volatile u32 *)0x1000B410;    /* ch4 MADR */
    *(u32 *)(obj + 0x20) = *(volatile u32 *)0x1000B430;    /* ch4 TADR */
    *(u32 *)(obj + 0x24) = *(volatile u32 *)0x1000B420;    /* ch4 QWC  */
    *(u32 *)(obj + 0x28) = *(volatile u32 *)0x1000B400;    /* ch4 CHCR */

    if (*(volatile u32 *)0x10002010 & 0xF0) {               /* wait for IPU to drain */
        u32 busy;
        do {
            busy = *(volatile u32 *)0x10002010 & 0xF0;
            R5900_SHORT_LOOP_PAD1(busy, busy);
            R5900_SHORT_LOOP_PAD1(busy, busy);
            R5900_SHORT_LOOP_PAD1(busy, busy);
        } while (busy);
    }

    func_003514E0(0);                                       /* suspend IPU_FROM (ch3) */
    *(u32 *)(obj + 0x2C) = *(volatile u32 *)0x1000B010;    /* ch3 MADR */
    *(u32 *)(obj + 0x30) = *(volatile u32 *)0x1000B020;    /* ch3 QWC  */
    *(u32 *)(obj + 0x34) = *(volatile u32 *)0x1000B000;    /* ch3 CHCR */
    *(u32 *)(obj + 0x38) = *(volatile u32 *)0x10002020;    /* IPU_BP   */
    *(u32 *)(obj + 0x3C) = *(volatile u32 *)0x10002010;    /* IPU_CTRL */

    func_0011AC40(*(s32 *)(obj + 0x40));                    /* release */
    return 1;
}
#endif

/* TODO(hle): needs PS2 graphics/IO HLE backend — the IPU restart path: reads
   REG_IPU_CTRL/IPU_BP, drains REG_DMAC_3_IPU_FROM, re-issues the IPU command
   (REG_IPU_CMD) and re-arms REG_DMAC_4_IPU_TO_MADR/TADR/QWC. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00351C20);

/* GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00351F58)
/* func_00351F58: IPU_TO channel teardown + DeleteSema. MATCHED on the s136os
 * arm (packed saves s0@0x0, ra@0x8; FACT #8830). */
S136OS_SLOT(func_00351F58);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 99.32% / engine96 62.64%. Residual: PACKED-SAVE only on sdk29 (2 callee saves at 16-byte stride, every non-save word equal); SCHED on engine96 (8-byte slots right, instruction set identical, order differs under -fno-schedule-insns; -fno-gcse+scheduling probed on the engine arm, no better). */
/* Byte-exact on the s136os arm (task #1309); cc1 2.9 cannot pack the callee
   saves (s0@0x0, ra@0x8).

   ROUTE: tester-EE-pending (NOT isolated-cmp-oracleable) - clears the live DMAC
   channel-4 (IPU_TO) QWC/MADR/TADR MMIO registers directly, which has no
   portable-C semantics on a non-EE host and touches hardware on the real EE
   outside a running DMA. Verify under the live tester-EE FMV-playback teardown.

   Tear down the IPU_TO DMA channel: suspend it (func_00351550(5)), zero its
   transfer registers (QWC 0x1000B410, MADR 0x1000B420, TADR 0x1000B430), then
   delete the object's completion sema (obj[0x40]). Always reports success (1). */
s32 func_00351F58(u8 *obj) {
    func_00351550(5);
    *(volatile u32 *)0x1000B420 = 0;   /* MADR */
    *(volatile u32 *)0x1000B410 = 0;   /* QWC  */
    *(volatile u32 *)0x1000B430 = 0;   /* TADR */
    func_0011AC30(*(s32 *)(obj + 0x40));
    return 1;
}
#endif

/* GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00351FB0)
/* func_00351FB0: sema-guarded total-bytes-queued read. MATCHED on the s136os
 * arm (packed saves s0/s1/ra; FACT #8830). */
S136OS_SLOT(func_00351FB0);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 76.58% / engine96 89.47%. Residual: PACKED-SAVE (3 callee saves) + 9 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* Byte-exact on the s136os arm (task #1309; SN 2.95.3 v1.36 -fopt-stack packs
   the s0/s1/ra saves cc1 2.9 cannot).

   NOTE(type): `obj` is the embedded bitstream/IPU-DMA sub-object (FmvStream+0x48,
   built by FmvBitstreamObjInit), the same sibling type not yet recovered for
   func_00352058/FmvPtsQueueInit — sema +0x40, retired-block count +0x10
   (in 2KB units), residual byte cursor +0x14; left u8* with raw offsets.

   Total elementary-stream bytes queued for the IPU, read under the object's
   sema: (retired 2KB blocks << 11) + residual byte cursor. */
/* The sema calls name the ROM's EE-kernel syscall stubs by address
   (func_0011AC60 = WaitSema, func_0011AC40 = SignalSema, as func_00351F58 names
   func_0011AC30 = DeleteSema): the SDK names have no definition in this link. */
extern s32 func_0011AC60(s32 sema);   /* WaitSema */
extern s32 func_0011AC40(s32 sema);   /* SignalSema */
s32 func_00351FB0(void *stream) {
    u8 *obj = (u8 *)stream;
    s32 total;

    func_0011AC60(*(s32 *)(obj + 0x40));
    total = (*(s32 *)(obj + 0x10) << 11) + *(s32 *)(obj + 0x14);
    func_0011AC40(*(s32 *)(obj + 0x40));
    return total;
}
#endif

#ifndef TARGET_NATIVE
/* func_00352000: sema-guarded 2KB round-up of the byte cursor. Blocked:
 * 8-byte-packed saves (s0@0x0, ra@0x8). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352000);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void WaitSema(s32 sema);
extern s32 SignalSema(s32 sema);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 75.67% / engine96 58.62%. Residual: PACKED-SAVE (2 callee saves) + 12 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0/ra). Revisit with the gameplay-TU compiler.

   NOTE(type): `obj` is the embedded bitstream sub-object (FmvStream+0x48); sema
   +0x40, residual byte cursor +0x14 (raw offsets, sibling type not recovered).

   Snap the residual byte cursor up to the next 2KB (0x800) boundary, under the
   object's sema. The asm biases by 0x7FF then arithmetic-right-shifts by 11
   (with the +0xFFE negative-input correction the compiler inserts); the cursor
   is a non-negative byte count, so this is the usual round-up-to-2KB.
   RETURN: the asm stores the rounded cursor in the SignalSema jal delay slot, so
   $2 at `jr` is SignalSema's return value (NOT the rounded cursor) - return that.
   (Fixes else_divergences #15: the return diverged on real R5900.) */
s32 func_00352000(u8 *obj) {
    s32 v;

    WaitSema(*(s32 *)(obj + 0x40));
    v = *(s32 *)(obj + 0x14);
    v = (((v + 0x7FF >= 0) ? v + 0x7FF : v + 0xFFE) >> 11) << 11;
    *(s32 *)(obj + 0x14) = v;
    return SignalSema(*(s32 *)(obj + 0x40));
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352058);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 83.40% / engine96 59.19%. Residual: REGALLOC+BRANCH-SHAPE on sdk29 (a2/a3 colouring, `beqz t6`/`bnez` inversion, div-guard placement); not iterated (99 insns). */
/* TODO(match): functional equivalent - not byte-exact; multiple div ops each
   guarded by a branch-likely beql/break div-by-zero check (the div-expansion-
   scheduling + branch-likely wall, same family as func_00351328). Revisit with
   the gameplay-TU compiler.

   NOTE(type): `obj` here is NOT a FmvStream — it is the *embedded bitstream/
   IPU-DMA sub-object* (the one a FmvStream holds at +0x48, constructed by
   FmvBitstreamObjInit and reached by the func_00352590/B0/680 forwarders via
   obj+0x48). Its own field layout (slot-ring base +0x50, ring length +0x54,
   read/write cursors +0x58/+0x5C, sector size +0x08) starts at offset 0, so it
   does not fit FmvStream (0xB8) — left u8* until that sibling type is recovered.

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
                        s32 tail;   /* C89: declare at block top (ee-gcc) */
                        if (*(s64 *)slot >= 0) {
                            *(s32 *)(slot + 0x14) = 0;
                            *(s64 *)slot = -1;
                            *(s64 *)(slot + 8) = -1;
                            *(s32 *)(slot + 0x10) = 0;
                        }
                        tail = *(s32 *)(obj + 0x58) - 1;
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

/* func_003521B0(dmaq, cmd): sema-guarded enqueue of a 0x18-byte DMA-add command
 * into the queue's ring buffer. Under the queue sema (dmaq+0x40): if there is
 * room (count@0x58 < capacity@0x54), validates the command via func_00352058 and
 * - unless BOTH 64-bit words (cmd+0x0, cmd+0x8) are negative (the skip sentinel) -
 * copies {u64,u64,s32,s32} into ring[writeIdx@0x5C] (base@0x50, stride 0x18),
 * bumps the count and wraps the write index modulo capacity, returning 1. Returns
 * 0 if the queue is full or the command was the skip sentinel. (The prior
 * signature inconsistency was the void-vs-s32 extern, now fixed - the caller
 * func_00352638 checks the result == 0.) Matching arm stays asm; #else is the
 * structure-exact model. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003521B0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void WaitSema(s32 sema);
extern s32 func_00352058(u8 *obj, u8 *req);
extern s32 SignalSema(s32 sema);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 71.46% / engine96 67.53%. Residual: PACKED-SAVE (4 callee saves) + 40 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
s32 func_003521B0(void *dmaq, void *cmd) {
    u8 *q = (u8 *)dmaq;
    u8 *c = (u8 *)cmd;
    s32 result = 0;

    WaitSema(*(s32 *)(q + 0x40));
    if (*(s32 *)(q + 0x58) < *(s32 *)(q + 0x54)) {   /* count < capacity */
        func_00352058(q, c);
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

/* TODO(hle): needs PS2 graphics/IO HLE backend — scans the IPU frame-slot ring
   for the range covering the channel's current REG_DMAC_4_IPU_TO_MADR /
   REG_IPU_BP position and pops it; depends on live IPU/DMAC hardware state. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003522C0);

#if !defined(TARGET_NATIVE) && !defined(S136OS_FmvStreamInit)
S136OS_SLOT(FmvStreamInit);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void *func_0012F738(void);
extern s32 func_0012FA70(u8 *obj, s32 slot, void *cb, s32 arg);
extern void func_003525D0(FmvStream *s);
extern s32 FmvBitstreamObjInit(u8 *obj, u64 a, u64 b, u64 c, u64 d, u64 e);
extern s32 func_00352A20(s32 unused, s32 *frame);
extern s32 func_00352A48(void);
extern s32 func_00352A80(void);
extern s32 func_00352AB0(void);
extern s32 func_00352AE0(s32 unused, u8 *obj);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 94.22% / engine96 85.55%. Residual: PACKED-SAVE (7 callee saves) + 11 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/** FmvStreamInit - construct the FMV stream object: reset its message dispatch
 *  table (func_0012F738), register the five stream-event callbacks (frame-drop
 *  report, two DMA-add-queue pumps, retry, and the cursor snapshot), clear the
 *  playback FSM (func_003525D0), then build the embedded bitstream object at
 *  +0x48 (FmvBitstreamObjInit).
 *  @param obj     the stream object
 *  @param p2, p3  unused by this body
 *  @param p4..p8  forwarded unchanged to FmvBitstreamObjInit
 *  @return 1 (always reports success)
 * MATCHED on the s136os arm (task #1370): the 8-byte-packed callee saves the
 * MEASURED line above records are SN 2.95.3 v1.36 -fopt-stack's (FACT #8810).
 * The body was already exact; it only needed the callee declarations above to
 * compile alone.
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
s32 FmvStreamInit(FmvStream *obj, u64 p2, u64 p3, u64 p4, u64 p5, u64 p6, u64 p7,
                  u64 p8) {
    func_0012F738();
    func_0012FA70((u8 *)obj, 0, (void *)func_00352A20, 0);
    func_0012FA70((u8 *)obj, 1, (void *)func_00352A48, 0);
    func_0012FA70((u8 *)obj, 2, (void *)func_00352A80, 0);
    func_0012FA70((u8 *)obj, 3, (void *)func_00352AB0, 0);
    func_0012FA70((u8 *)obj, 5, (void *)func_00352AE0, 0);
    func_003525D0(obj);
    FmvBitstreamObjInit((u8 *)obj + 0x48, p4, p5, p6, p7, p8);
    return 1;
}
#endif

/* func_00352568: 8 bytes of inter-function padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352568);

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
s32 func_00352590(FmvStream *obj) {
    return func_003517C0((u8 *)obj + 0x48);
}

/**
 * Forward to the read-cursor advance of the embedded stream object.
 */
s32 func_003525B0(FmvStream *obj) {
    return func_003518B8((u8 *)obj + 0x48);
}

/**
 * Reset the playback FSM to idle.
 */
void func_003525D0(FmvStream *s) {
    s->state = 0;
}

/* func_003525D8: stop + detach the embedded stream — channel teardown + DeleteSema
 * on the object at +0x48, detach the message dispatch table (func_0012F940), report
 * success. On cc1 2.9: 8-byte-packed saves (s0@0x0, ra@0x8) — 2.9 -G8 -fno-gcse
 * compiles a 16-byte-slot 0x20 frame; the original packs 8-byte in 0x10.
 * MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
 * byte-exact (FACT #8810; census FACT #8830).
 * GUARD (task #1272): on EE this C is the image's body, compiled alone by the
 * s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
 * S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
 * that skips the splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003525D8)
S136OS_SLOT(func_003525D8);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 98.85% / engine96 84.62%. Residual: PACKED-SAVE only on sdk29 (2 callee saves at 16-byte stride, every non-save word equal); SCHED on engine96 (8-byte slots right, instruction set identical, order differs under -fno-schedule-insns; -fno-gcse+scheduling probed on the engine arm, no better). */
s32 func_003525D8(FmvStream *obj) {
    func_00351F58((u8 *)obj + 0x48);
    func_0012F940((u8 *)obj);
    return 1;
}
#endif

#if defined(MATCH_FmvRequestStop) || defined(TARGET_NATIVE)
/**
 * Request a stop: mark the playback FSM "stop" (state 1) and report accepted.
 *
 * Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_FmvRequestStop;
 * task #513): unit objdiff 100.00% (objdiff_build.sh + unit_report.sh) — the
 * 2.96 cc1 reuses the one `li v0,1` for both the store and the return value,
 * where the 2.9 arm materialises two (63.33% there, #6521's ARTIFACT class).
 * The INCLUDE_ASM below still feeds the 2.9 link in build.sh, which defines
 * no MATCH_.
 */
s32 FmvRequestStop(FmvStream *s) {
    s->state = 1;
    return 1;
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", FmvRequestStop);
#endif

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
 * obj->streamStart, tag} for a bitstream span into the arena's DMA-add queue
 * (g_pFmvArenaBase+0xD9090); returns the enqueue result. Best attempt 54%
 * (matching arm): the pinned cc1 schedules the obj->0x48 load + subu early (or,
 * with volatile pinning, pushes the arena load late and pads the jal delay slot)
 * where the original loads obj->0x48 late and puts the rel store in the jal delay
 * slot - prologue-scheduling shapes not reachable from cc1 2.9 with this source.
 * MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
 * byte-exact (FACT #8810; census FACT #8830).
 * GUARD (task #1272): on EE this C is the image's body, compiled alone by the
 * s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
 * S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
 * that skips the splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00352638)
S136OS_SLOT(func_00352638);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 54.39% / engine96 27.50%. Residual: SCHED on sdk29 (the obj+0x48 load and the tag store are placed early; 3 statement orders probed, none moves them). */
s32 func_00352638(u8 *obj, u64 addr, u64 size, s32 pos, s32 tag) {
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
    return func_003521B0(g_pFmvArenaBase + 0xD9090, &cmd);
}
#endif

/**
 * Forward to the total-bytes-queued read of the embedded stream object.
 */
s32 func_00352680(FmvStream *obj) {
    return func_00351FB0((u8 *)obj + 0x48);
}

/* func_003526A0: 8 bytes of inter-function padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/250080", func_003526A0);

/* func_003526A8: end-of-stream flush (pads the bitstream to a 4-byte boundary).
 * Blocked (match): 8-byte-packed saves (s0@0x20, ra@0x28).
 * PARKED #70 (#else not confident): the asm sets up 4 stack out-param pointers
 * (sp+0x10/+0x14/+0x18/+0x1C) before `jal func_00352590`, then reads sp+0x14/+0x1C
 * (sum<4 -> early-out) and sp+0x10/+0x18 (masked &0xFFFFFFF | 0x20000000 into DMATAGs
 * for func_003511A8), also copying D_1AE818 (unaligned lwl/lwr) to sp+0x0 — but Ghidra
 * decompiles func_00352590 as a 1-param forwarder `FUN_003517c0(p+0x48)`, contradicting
 * the 4-out-param wiring. Resolve func_00352590.s + func_003517c0 (does it write the 4
 * slots?) + func_003511A8's arg arity before writing a faithful #else. Not forcing a
 * low-confidence body. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003526A8);

#ifndef TARGET_NATIVE
/* func_00352780: poll stream-done then host-side done. Blocked:
 * 8-byte-packed saves (s0/s1/ra). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00352780);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0012F9B8(u8 *host);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 75.39% / engine96 75.61%. Residual: PACKED-SAVE (3 callee saves) + 8 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0/s1/ra). Revisit with the gameplay-TU compiler.

   End-of-playback poll: false while the embedded stream still has bytes queued
   (func_00352680 != 0); once the stream has drained, report whether the host
   side has also signalled done (func_0012F9B8 != 0). */
s32 func_00352780(FmvStream *obj) {
    if (func_00352680(obj) != 0) {
        return 0;
    }
    return func_0012F9B8((u8 *)obj) != 0;
}
#endif

/* FmvDecodeThreadEntry: the FMV decode-thread main loop. Resets the embedded stream
 * ring (FmvStreamStartDma on obj+0x48), initialises the arena frame queue
 * (func_00352B90 at +0xD9168), primes the host frame reader (FmvDisplayWorkerLoop), then
 * decodes frames (func_00352620) as long as the arena's "more data" flag
 * (+0xD9174) stays set and no frame reports completion (returns 1), and finally
 * drives the stream to its terminal state 3 (func_00352628). Matching arm stays
 * asm (8-byte-packed saves + a %gp_rel/absolute reload-artifact wall on
 * g_pFmvArenaBase); the #else is the structure-exact model (the frame decode
 * itself is the deferred FMV native backend, but the loop structure is exact). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", FmvDecodeThreadEntry);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 92.38% / engine96 64.59%. Residual: PACKED-SAVE (4 callee saves) + 5 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
extern void FmvStreamStartDma(u8 *stream);
extern s32 FmvDisplayWorkerLoop(u8 *host);
extern void func_00352B90(FmvFrameQueue *q);
s32 FmvDecodeThreadEntry(FmvStream *obj) {
    FmvStreamStartDma((u8 *)obj + 0x48);
    func_00352B90((FmvFrameQueue *)(g_pFmvArenaBase + 0xD9168));
    FmvDisplayWorkerLoop((u8 *)obj);
    while (*(s32 *)(g_pFmvArenaBase + 0xD9174) != 0) {
        if (func_00352620(obj) == 1) {
            break;
        }
    }
    return func_00352628(obj, 3);
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", FmvDisplayWorkerLoop);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0012F9A8(u8 *obj);
extern s32 FmvFrameQueueGetWriteSlot(u8 *fq);
extern s32 func_0012F950(u8 *obj, s32 ptr, s32 len);
extern void func_00350B60(void *gif, u8 *frame, u32 a, u32 b, s32 idx);
extern void FmvFrameQueuePush(u8 *fq);
extern void func_0012F9C8(u8 *obj);
extern char D_1AE820[];
extern char D_1AE838[];
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 85.53% / engine96 55.33%. Residual: PACKED-SAVE (10 callee saves) + 43 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0..s2/ra) plus a delay-slot %gp_rel read of the arena base mixed
   with its absolute form. Revisit with the gameplay-TU compiler.

   The FMV frame-display worker loop: pulls the next decoded picture from the
   host (func_0012F9A8/func_0012F950), waits (yielding) for a free display
   slot (FmvFrameQueueGetWriteSlot), builds the per-tile GIF display chain for each frame
   tile (func_00350B60), commits the slot (FmvFrameQueuePush), and yields. Exits
   when the host signals end-of-stream or the FSM reports stop. */
s32 FmvDisplayWorkerLoop(u8 *host) {
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
        while ((slot = (u8 *)FmvFrameQueueGetWriteSlot(g_pFmvArenaBase + 0xD9168)) == 0) {
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
        FmvFrameQueuePush(g_pFmvArenaBase + 0xD9168);
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
 * MATCHED on the s136os arm: the 8-byte-packed saves (s0@0x20, ra@0x28) are
 * SN 2.95.3 v1.36 -fopt-stack's (FACT #8810); the 0x30 frame needs the
 * three-element local below.
 * GUARD (task #1313): on EE this C is the image's body, compiled alone by the
 * s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
 * S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
 * that skips the splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00352AE0)
S136OS_SLOT(func_00352AE0);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 99.35% / engine96 54.25%. Residual: PACKED-SAVE only on sdk29 (2 callee saves at 16-byte stride, every non-save word equal); SCHED on engine96 (8-byte slots right, instruction set identical, order differs under -fno-schedule-insns; -fno-gcse+scheduling probed on the engine arm, no better). */
/* Stream-event callback (id 5): read the two 64-bit cursor words of the IPU
   DMA-add queue (at arena+0xD9090) via func_003522C0 into a stack pair, then
   stash them into the stream object at +0x8 / +0x10 (the snapshot the retry
   path replays from). The event id arrives in arg0 (unused); the stream object
   in arg1. Always reports handled (1). */
/* Repeated from the TARGET_NATIVE block above so the s136os arm (compiled with
 * TARGET_NATIVE undefined) sees a prototype: since 250080 became .cpp (task
 * #1312) the SN 1.36 cc1plus rejects the implicit declaration that cc1 allowed.
 * Identical to the native declaration, so the redeclaration is legal in both. */
extern s32 func_003522C0(void *dmaq, ...);
s32 func_00352AE0(s32 unused, u8 *obj) {
    /* func_003522C0 writes cursor[0] and cursor[1] only. The third element
     * is never touched: it is what gives the ROM's 0x30 frame (locals at
     * sp+0..0x1F, saves at 0x20/0x28) under SN 1.36; with two elements the
     * frame is 0x20 (task #1313). */
    u64 cursor[3];

    func_003522C0(g_pFmvArenaBase + FMV_DMAQ_OFS, cursor);
    *(u64 *)(obj + 0x8) = cursor[0];
    *(u64 *)(obj + 0x10) = cursor[1];
    return 1;
}
#endif

/**
 * FmvFrameQueueInit - initialise the decoded-frame display queue.
 *
 * @param rec   queue header: +0x0 arg1, +0x4 slot base, +0x8/+0xC cleared,
 *              +0x10 slot count
 * @param arg1  stored at +0x0
 * @param base  address of slot 0 (slots are 0x138C0 bytes apart)
 * @param count number of slots
 *
 * Zeroes each slot's state word (+0x0) and stamps its index at +0x4. The slot
 * base is re-read from the header for every store, as in the ROM.
 *
 * Byte-exact on the sdk29 arm (task #895). #513 measured 84.82 and named the
 * header-store order as a scheduling wall; it is reachable:
 *  - a memory barrier after the +0xC store, with `i = 0` between them, keeps
 *    the ROM's `sw zero,12; move t0,zero` first;
 *  - a second barrier after the last header store keeps `sw zero,8` out of
 *    the blez delay slot, which then takes the `lui` of the stride;
 *  - the loop is an `if (count > 0) do {} while` with `off = 0` inside the
 *    guard, so its `move a1,zero` issues after the branch as in the ROM;
 *  - i is bound to $8 (t0), the ROM's counter register (empty on native).
 */
#ifndef TARGET_NATIVE
#define FQ_INDEX_IN_T0 __asm__("$8")
#else
#define FQ_INDEX_IN_T0
#endif
void FmvFrameQueueInit(s32 *rec, s32 arg1, s32 base, s32 count) {
    register s32 i FQ_INDEX_IN_T0;
    s32 off;

    rec[3] = 0;
    i = 0;
    __asm__ __volatile__("" : : : "memory");
    rec[0] = arg1;
    rec[1] = base;
    rec[4] = count;
    rec[2] = 0;
    __asm__ __volatile__("" : : : "memory");
    if (count > 0) {
        off = 0;
        do {
            *(s32 *)(off + rec[1]) = 0;
            *(s32 *)(off + rec[1] + 4) = i;
            i++;
            off += 0x138C0;
        } while (i < count);
    }
}

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

#ifndef TARGET_NATIVE
/* FmvFrameQueuePush: commit the just-decoded slot (mark state 2, advance the
 * write cursor modulo capacity) under DI/EI. Blocked: 8-byte-packed saves
 * (s0@0x0, ra@0x8). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", FmvFrameQueuePush);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 66.50% / engine96 68.10%. Residual: PACKED-SAVE (2 callee saves) + 21 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0/ra). Revisit with the gameplay-TU compiler.

   Commit the just-decoded frame slot for display, with interrupts disabled:
   mark the current write slot's state word = 2 (ready), bump the queued count,
   and advance the write cursor modulo the slot capacity. */
void FmvFrameQueuePush(u8 *fq) {
    FmvFrameQueue *q = (FmvFrameQueue *)fq;

    func_0011F5E0();   /* DI */
    *(s32 *)(q->frames + q->writeIdx * 0x138C0) = 2;
    q->count++;
    q->writeIdx = (q->writeIdx + 1) % q->capacity;
    func_0011F628();   /* EI */
}
#endif

/* FmvFrameQueueGetWriteSlot: writable GS frame pointer (writeIdx * 0xD0000). On
 * cc1 2.9: 8-byte-packed saves (s0@0x0, ra@0x8).
 * MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
 * byte-exact (FACT #8810; census FACT #8830).
 * GUARD (task #1272): on EE this C is the image's body, compiled alone by the
 * s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
 * S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
 * that skips the splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_FmvFrameQueueGetWriteSlot)
S136OS_SLOT(FmvFrameQueueGetWriteSlot);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 95.31% / engine96 69.06%. Residual: PACKED-SAVE (2 callee saves) + 1 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* Address of the GS frame buffer the producer may write next: 0 if the display
   queue is full, else gsFrames + writeIdx * 0xD0000 (the GS-side stride). */
s32 FmvFrameQueueGetWriteSlot(u8 *fq) {
    FmvFrameQueue *q = (FmvFrameQueue *)fq;

    if (func_00352BA0(q) != 0) {
        return 0;
    }
    return (s32)(q->gsFrames + q->writeIdx * 0xD0000);
}
#endif

/**
 * True when the frame queue holds no displayable frame.
 */
s32 func_00352C70(FmvFrameQueue *q) {
    return q->count == 0;
}

#ifndef TARGET_NATIVE
/* FmvFrameQueueGetDisplaySlot: oldest queued decoded-frame pointer ((writeIdx - count +
 * cap) % cap slot). Blocked: 8-byte-packed saves (s0@0x0, ra@0x8). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", FmvFrameQueueGetDisplaySlot);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 96.52% / engine96 78.12%. Residual: PACKED-SAVE (2 callee saves) + 4 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee
   saves (s0/ra). Revisit with the gameplay-TU compiler.

   Address of the oldest decoded frame still queued for display: 0 if the queue
   is empty, else the frame slot at ((writeIdx - count + capacity) % capacity)
   in the 0x138C0-stride slot array. */
s32 FmvFrameQueueGetDisplaySlot(FmvFrameQueue *q) {
    s32 idx;

    if (func_00352C70(q) != 0) {
        return 0;
    }
    idx = (q->writeIdx - q->count + q->capacity) % q->capacity;
    return (s32)(q->frames + idx * 0x138C0);
}
#endif

/**
 * Release the oldest displayed frame from the queue (if any).
 */
void func_00352CE8(FmvFrameQueue *q) {
    volatile s32 *count = &q->count;

    if (*count > 0) {
        *count = *count - 1;
    }
}

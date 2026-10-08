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

/* FMV_ARENA_BASE_GP: ADDRESSING-MODEL DEVICE (RULING #9073 under RULING #8620; emits
 * no code). An offset-0 assembler equate of g_pFmvArenaBase with no `.extern` size,
 * so an access spelled through it assembles gp-relative while every access
 * spelled g_pFmvArenaBase keeps the `.extern g_pFmvArenaBase, 16` absolute form
 * above. It reproduces two of the ROM's delay-slot reads,
 * `lw $2,%gp_rel(g_pFmvArenaBase)($28)`: FmvDecodeThreadEntry's at 0x0035283C
 * (task #1434) and FmvDisplayWorkerLoop's at 0x003528F8 (task #1888); the
 * relocation names the real symbol. At FILE SCOPE (task #1434), not in a member's
 * arm, so the s136os splice admits it (FACT #9057). EE only: on native it is
 * g_pFmvArenaBase. */
#ifndef TARGET_NATIVE
__asm__("g_pFmvArenaBaseGp = g_pFmvArenaBase");
extern u8 *g_pFmvArenaBaseGp;
#define FMV_ARENA_BASE_GP g_pFmvArenaBaseGp
#else
#define FMV_ARENA_BASE_GP g_pFmvArenaBase
#endif

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
extern s32 func_0012EE28(void); /* DECL-LEVER(#2011): defined (obj, index, value, a3, a4) in cod/022FA8; func_00352570 below forwards its own incoming argument registers through a bare `jal` (the ROM sets none), which this argument-less declaration reproduces */
#ifndef TARGET_NATIVE
extern s32 func_003517C0(void *stream);
#endif
extern s32 func_003518B8(void *stream, s32 n);
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
extern s32 func_0011AC30(s32 sema);    /* DeleteSema */
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
/* func_003517C0: deferred-native FMV stream func whose #else bodies model it
   with inconsistent arg counts across call sites (true signature needs the asm;
   FMV native backend is deferred). Declared with the stream pointer typed and the
   rest variadic (C++ reads an empty `()` as `(void)`), so the corpus compiles;
   resolve when the FMV native path is built. (func_003518B8 was the other one;
   it has its real signature since task #1801, declared above for both arms.) */
extern s32 func_003517C0(void *stream, ...);
extern s32 func_00352638(u8 *obj, u64 a, u64 b, s32 pos, s32 n);
extern s32 func_003522C0(void *dmaq, ...);  /* FMV DMA-add-queue enqueue (deferred native; ret ignored) */
extern void ZeroQwords(void *p, s32 n);
extern s32 func_00133850(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern void *IpuInitDecoder(void); /* DECL-LEVER(#2025): defined (obj, buf, size) in cod/022FA8; FmvStreamInit forwards its own incoming $a0..$a2 through a bare `jal` (the ROM sets none of them before the call), which this argument-less declaration reproduces, as #2011 did for func_0012EE28 */
extern s32 func_0012FA70(u8 *obj, s32 slot, void *cb, s32 arg);
extern void func_003525D0(FmvStream *s);
extern s32 FmvBitstreamObjInit(u8 *obj, s32 a, s32 b, s32 c, s32 d, s32 e);
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
extern void func_001338F0(s32 commitBase, s32 len, s32 commitArg, s32 queued, s32 arg5);
extern void func_00133890(s32 *obj);   /* commit-path stream lock */
/* SIF-DMA bounce primitives (func_00350868 #else): queue a SIF DMA, busy-wait
   for the slot, poll for completion, then signal. */
extern void func_0011AEA0(s32 mode);   /* FlushCache (EE syscall 0x64); returns nothing */
extern s32 func_0011AFE0(void *desc, s32 count);  /* sceSifSetDma (returns id) */
extern s32 func_0011AFC0(s32 id);      /* sceSifDmaStat (busy while >= 0) */
extern void func_00133930(s32 len, s32 dstOfs);  /* post-transfer notify */
/* Semaphore callees (CreateSema/WaitSema/SignalSema) referenced only by the #else bodies. */
extern s32 func_0011AC20(void *param); /* CreateSema (returns sema id) */
extern s32 func_0011AC60(s32 sema);    /* WaitSema (acquire) */
extern s32 func_0011AC40(s32 sema);    /* SignalSema (release) */
s32 FmvStreamStartDma(u8 *stream);         /* ring init, defined below (fwd for FmvBitstreamObjInit) */
#endif

/* The IPU_TO bitstream sub-object embedded at FmvStream + 0x48: the fields the
 * matched bodies (FmvBitstreamObjInit, func_003518B8, func_00351910) touch.
 * The ring is ringSize blocks of 0x800 bytes at srcBase, one DMA source tag per
 * block in the tag ring at tagWord's address. */
typedef struct FmvBitstreamObj {
    /* 0x00 */ u32 srcBase;      /* physical base of the macroblock buffer */
    /* 0x04 */ u32 tagWord;      /* "next" DMA tag to the source-tag ring */
    /* 0x08 */ s32 ringSize;     /* ring blocks (0x800 bytes each) */
    /* 0x0C */ s32 head;         /* oldest block the channel has not consumed */
    /* 0x10 */ s32 queued;       /* blocks tagged for the channel and not yet consumed */
    /* 0x14 */ s32 pendingBytes; /* bytes fed and not yet tagged (/0x800 at func_00351910) */
    /* 0x18 */ u32 ringBytes;    /* ringSize << 11 */
    /* 0x1C */ u8 pad1C[0x24];
    /* 0x40 */ s32 sema;         /* decode semaphore id */
    /* 0x44 */ s32 armed;        /* 1 while the ring is live (FmvStreamStartDma) */
    /* 0x48 */ s64 totalBytes;   /* bytes fed since init */
    /* 0x50 */ s32 slotBase;
    /* 0x54 */ s32 slotCount;
} FmvBitstreamObj;

/* The SDK's CreateSema parameter block (eekernel.h struct SemaParam). */
typedef struct FmvSemaParam {
    s32 currentCount;
    s32 maxCount;
    s32 initCount;
    s32 numWaitThreads;
    u32 attr;
    u32 option;
} FmvSemaParam;

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
 * thread (func_0011AA70/func_0011AA30 over g_fmvThreadId), disable DMAC ch2, remove
 * its handler (func_0011A950), reinstall the game's vblank-start handler
 * (func_00126DC0(OnVblankInterrupt), FACT #5907), tear down the
 * DMA queues (func_003525D8/func_003505E0/func_003513F0 over the arena sub-objects),
 * quiesce again, and clear bit 1 of the INTC-mask reg 0x1000E000.
 * The ROM's one %gp_rel read of g_pFmvArenaBase (0x003503EC, the jal
 * func_003512F0 delay slot) is that callee's ARGUMENT, so it is spelled through
 * FMV_ARENA_BASE_GP (the file-scope device above); every other arena read stays
 * absolute. The three changes are jointly needed (s136 screen, task #1434, re-run
 * by task #1437): with no arguments 51/59; arguments without the device 52/59
 * (built 61); arguments + device with func_00126DC0 declared void 3/59 (cc1 then
 * reuses $2 at 0x0035044C where the ROM has $3); all three EXACT 59/59.
 * The arena-relative args the asm passes to func_003512F0/func_00352B88/
 * func_003505E0/func_003513F0 are dead (each callee ignores it) but are passed here
 * as the ROM passes them, and the four signatures carry them (RULING #9118, task
 * #1437). func_00126DC0 is declared s32: its ROM body returns the previous handler
 * in $2 (0x00126E44 `daddu $2,$18,$0`), though no ROM caller reads it. */
#ifdef TARGET_NATIVE
extern void func_0011AA70(s32 threadId);   /* kill thread */
extern void func_0011AA30(s32 threadId);   /* delete thread */
extern void DisableDmac(s32 channel);
extern void func_0011A950(s32 a, s32 b);
extern s32  func_00126DC0(void *handler);    /* install INTC-2 vblank-start handler; returns the previous one */
extern s32  g_fmvThreadId;
extern void OnVblankInterrupt(void);
/* forward decls — these are declared/defined later in this file */
extern s32  func_00124B88(s32 mode);
extern void func_003512F0(u8 *arena);
extern void func_00352B88(u8 *frameQueue);
extern s32  func_003525D8(FmvStream *obj);
extern s32  func_003505E0(u8 *ptsQueue);
extern s32  func_003513F0(u8 *block);
#endif

/* GUARD (task #1437): on EE this C is the image's func_003503D8, compiled alone by
 * the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003503D8)
S136OS_SLOT(func_003503D8);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_00124B88(s32 mode);
extern void func_003512F0(u8 *arena);
extern void func_00352B88(u8 *frameQueue);
extern void func_0011AA70(s32 threadId);
extern void func_0011AA30(s32 threadId);
extern void DisableDmac(s32 channel);
extern void func_0011A950(s32 a, s32 b);
extern s32 func_00126DC0(void *handler);
extern s32 func_003525D8(FmvStream *obj);
extern s32 func_003505E0(u8 *ptsQueue);
extern s32 func_003513F0(u8 *block);
extern s32 g_fmvThreadId;
extern void OnVblankInterrupt(void);
/* (end of this body's declarations) */
/* History: task #513 measured sdk29 75.25% / engine96 61.41% (unit objdiff report,
 * no arguments) and 95.59% on sdk29 with the arguments restored, the residual being
 * the gp-relative delay-slot read that FMV_ARENA_BASE_GP now spells. */
void func_003503D8(void) {
    func_00124B88(0);
    func_003512F0(FMV_ARENA_BASE_GP);
    func_00352B88(g_pFmvArenaBase + FMV_FRAMEQ_OFS);
    func_0011AA70(g_fmvThreadId);
    func_0011AA30(g_fmvThreadId);
    DisableDmac(2);
    func_0011A950(2, *(s32 *)(g_pFmvArenaBase + 0xD90F8));
    func_00126DC0((void *)OnVblankInterrupt);
    func_003525D8((FmvStream *)(g_pFmvArenaBase + 0xD9048));
    func_003505E0(g_pFmvArenaBase + FMV_PTS_OFS);
    func_003513F0(g_pFmvArenaBase + 0xD9040);
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
/* ADDRESSING-MODEL DEVICE for FmvPtsQueueInit's absolute lui form of D_001B2354 (emits no
 * code). At FILE SCOPE, not in the member's #else arm, so the unit's own 2.9 TU
 * declares the symbol too: tools/ee/s136os_splice.sh never carries an `.extern`
 * for a symbol the unit declares, so cc1's end-of-file small-size line from the
 * s136os TU is not carried in front of the block, where it would make the read
 * gp-relative (task #1387: the splice REFUSED the in-arm placement as ADDRESSING).
 * No 2.9 code in this unit names the symbol bare, so nothing else changes. */
__asm__(".extern D_001B2354, 16");
#endif
/* GUARD (task #1387): on EE the #else body below is the image's FmvPtsQueueInit, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_FmvPtsQueueInit)
S136OS_SLOT(FmvPtsQueueInit);
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

   MATCHED on the s136os arm (task #1387; screened by task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
   masked): EXACT 47/47, relocations equal,
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
 *
 * @param ptsQueue  the pts queue control block (arena+FMV_PTS_OFS); unused. The
 *                  ROM caller passes it (func_003503D8, 0x00350478 delay slot), so
 *                  the signature carries it (RULING #9118, task #1437).
 * @return          1 (ready).
 */
s32 func_003505E0(u8 *ptsQueue) {
    func_001338C8();
    return 1;
}

/* func_00350600: 8 bytes of inter-function padding (addiu $sp,+0x10 / nop),
 * not compiler output — no C can produce it. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350600);

/* func_00350608: read-chunk dispatch on the stream object. */
/* GUARD (task #1394): on EE the #else body below is the image's func_00350608, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00350608)
S136OS_SLOT(func_00350608);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_001338F0(s32 commitBase, s32 len, s32 commitArg, s32 queued, s32 arg5);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 52.05% / engine96 33.50%. Residual: PACKED-SAVE (2 callee saves) + 16 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* The 8-byte-packed callee saves (s0@0x0, ra@0x8) are SN 2.95.3 v1.36
   -fopt-stack's (FACT #8810), which the s136os arm compiles this body with.

   Commit the buffered packet to the IPU bitstream feeder: truncate the staged
   commit length (commitLen) to a whole number of 1KB (0x400) units (a signed
   division, rounding toward zero), hand {commitBase, snappedLen, commitArg,
   hdr-word@0x14, hdr-word@0x18} to func_001338F0 and flip the stream's
   `started` word to 2 (header consumed). Returns nothing: $2 holds the 2 only
   because it is the stored value, and the one caller (FmvStreamFeedLoop)
   overwrites $2 straight after the call.

   NOTE(offset): args 4 and 5 come from word idx [5] (q+0x14) and [6] (q+0x18),
   both inside the packet-header staging area hdr[0x28] (NOT q->queued@0x50 - the
   cmp oracle caught an earlier version that read queued). Raw offsets kept.

   MATCHED on the s136os arm (task #1394: vmu with the base seeded, image
   cmp 0; screened by task #1389, SN 2.95.3 v1.36 -fopt-stack, masked words
   and relocations).
   Levers: the snap written as the division commitLen / 0x400 * 0x400 (cc1
   expands it to the ROM's slt/movn; the hand-expanded ?: compiled to a
   bltzl), and two declaration fixes - func_001338F0 is void (its matched
   definition, cod/0321A0.c, is void; both of this unit's declarations said
   s32) and this function is void. */
void func_00350608(FmvPtsQueue *q) {
    func_001338F0(q->commitBase, q->commitLen / 0x400 * 0x400, q->commitArg,
                  *(s32 *)((u8 *)q + 0x14), *(s32 *)((u8 *)q + 0x18));
    q->started = 2;
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
 * @param q      the pts queue
 * @param pPtr0  out: start of the leading span
 * @param pLen0  out: its length in bytes
 * @param pPtr1  out: start of the wrapped span (0 if none)
 * @param pLen1  out: its length in bytes (0 if none)
 *
 * Every field is re-read after each store through the out-pointers (they may
 * alias the queue), only untilEnd is held; the mode and wrap tests are written
 * the way round the ROM lays its blocks out (mode != 4 and the no-wrap case
 * fall through).
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in tools/ee/s136os_functions.txt)
 * and spliced over S136OS_SLOT by tools/ee/s136os_splice.sh (task #1834). There
 * is no asm fallback: a build that skips the splice drops the function. On
 * native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003506A8)
S136OS_SLOT(func_003506A8);
#else
void func_003506A8(FmvPtsQueue *q, u8 **pPtr0, s32 *pLen0, u8 **pPtr1, s32 *pLen1) {
    if (q->started == 0) {
        if (q->mode != 4) {
            *pPtr0 = (u8 *)q + (q->hdrFill + 8);
            *pLen0 = 0x28 - q->hdrFill;
            *pPtr1 = q->dataPtr;
            *pLen1 = q->ringSize;
        } else {
            *pPtr0 = q->dataPtr;
            *pLen0 = q->ringSize;
            *pPtr1 = 0;
            *pLen1 = 0;
        }
    } else {
        s32 untilEnd = q->ringSize - q->consumed;

        if (q->ringSize - q->ringOfs >= untilEnd) {
            *pPtr0 = q->dataPtr + q->ringOfs;
            *pLen0 = untilEnd;
            *pPtr1 = 0;
            *pLen1 = 0;
        } else {
            *pPtr0 = q->dataPtr + q->ringOfs;
            *pLen0 = q->ringSize - q->ringOfs;
            *pPtr1 = q->dataPtr;
            *pLen1 = untilEnd - (q->ringSize - q->ringOfs);
        }
    }
}
#endif

/**
 * Account `n` freshly stored elementary-stream bytes to the pts queue.
 * Until the stream has started, the first bytes fill the 0x28-byte packet
 * header (hdrFill); the stream starts once the header is full, or at once for
 * a headerless (mode 4) stream. What is left of `n` advances the payload ring:
 * the ring size is snapped down to a 1KB multiple and written back, the write
 * offset wraps modulo it, and the consumed/received totals grow by `n`.
 * @param q  the arena's pts queue (arena + FMV_PTS_OFS)
 * @param n  bytes just copied into the ring (func_003510C0's `stored`)
 * @return   nothing meaningful: the ROM falls off the end with the received
 *           total left in $v0, and its only caller ignores it. The `s32`
 *           stays because the file-scope declaration every caller sees says
 *           so; func_003510C0 is matched against that declaration.
 *
 * Byte-exact on the s136os arm (task #2037), no devices. The cc1 2.9 arm's
 * old note ("68%, the register-coloring wall": a3/t0/a1 for what the ROM
 * colours a2/a3/v0-with-copy) does not apply to SN 1.36. Two phrasings decide
 * the bytes: the clamp is a ternary on a separate `room` local (the ROM's
 * `movz` on `room < n` and the `move $3,$2` copy into `take`; an in-place
 * `if (n < take) take = n` gives `slt n,take; movn` in other registers), and
 * the header branch is the then-arm with mode 4 the else-arm (the ROM's
 * fall-through order; the other order inverts the `beq`).
 *
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00350778)
S136OS_SLOT(func_00350778);
#else
s32 func_00350778(FmvPtsQueue *q, s32 n) {
    s32 size;
    if (q->started == 0) {
        if (q->mode != 4) {
            s32 room = 0x28 - q->hdrFill;
            s32 take = (room < n) ? room : n;
            q->hdrFill += take;
            if (q->hdrFill >= 0x28) {
                q->started = 1;
            }
            n -= take;
        } else {
            q->started = 1;
        }
    }
    size = (q->ringSize / 0x400) * 0x400;
    q->ringSize = size;
    q->ringOfs = (q->ringOfs + n) % size;
    q->consumed += n;
    q->received += n;
}
#endif

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

/* func_00350868: SIF-DMA bounce of a decoded block to IOP memory. */
/* GUARD (task #1387): on EE the #else body below is the image's func_00350868, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00350868)
S136OS_SLOT(func_00350868);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_0011AEA0(s32 mode);
extern s32 func_0011AFE0(void *desc, s32 count);
extern s32 func_0011AFC0(s32 id);
extern void func_00133930(s32 len, s32 dstOfs);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 81.88% / engine96 66.19%. Residual: PACKED-SAVE (5 callee saves) + 12 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* The 8-byte-packed callee saves (s0/s1/s2/s3/ra) are SN 2.95.3 v1.36
   -fopt-stack's (FACT #8810), which the s136os arm compiles this body with.

   FUNCTIONAL TEST ROUTE: tester-EE-pending (NOT isolated-cmp-oracleable) - the two do/while
   loops busy-wait on real SIF-DMA hardware (func_0011AFE0 returns 0 until a DMA
   slot frees; func_0011AFC0 returns >= 0 while the transfer is in flight), so an
   isolated EE with mocked SIF stubs would hang or pass vacuously. Verify under
   the live tester-EE FMV-playback scenario instead.

   Bounce a decoded 0x400 block from EE (src) to the IOP at the stream's IOP
   buffer base (obj[0x48]) + dstOfs: build a 4-word SIF-DMA descriptor
   {src, iopBase, len, 0}, flush the data cache (func_0011AEA0: EE syscall 100,
   FlushCache by the SDK's numbering; it returns nothing), retry the enqueue until
   a slot is granted, wait for completion, then notify (func_00133930).

   MATCHED on the s136os arm (task #1387; screened by task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
   masked): EXACT 42/42, relocations equal. The
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
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 30.39% / engine96 34.69%. Residual: PACKED-SAVE (4 callee saves) + 152 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs).
 * Save stride NOT re-measured for this member on the s136os arm: its C arm does not compile
 * solo there (NOTE #9871). Of the 111 labeled members that were, 0 reproduce the 16-byte save
 * stride there (FACT #9873), so the stride is not evidence that this member is walled.
 * Residual: UNMEASURED. */
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
 * Blocked on cc1 2.9: 8-byte-packed saves (s0@0x0, ra@0x8) — under 2.9 -G8 -fno-gcse
 * this C compiles a 16-byte-slot 0x20 frame; the original packs 8-byte in a 0x10 frame
 * (the genuine save-slot wall of that arm).
 * Save stride re-measured on the s136os arm (SN 1.36 -fopt-stack): this member's prologue - the
 * ROM's save set at its 8-byte stride, and its frame - is reproduced exactly (NOTE #9871), so
 * the save-slot wall named above is a cc1 2.9 property and was measured false as the reason
 * this member stays unmatched (FACT #9873). Residual: SCHED (NOTE #9710; 4/20 words). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_00350F28);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 84.05% / engine96 76.10%. Residual: PACKED-SAVE (2 callee saves) + 9 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee saves.
 * NEAR-MISS SCREEN (task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated
 * fields masked): 4/20 words differ, from 12/20, with the ROM's one R5900
 * short-loop pad nop before the loop's backward beqz (R5900_SHORT_LOOP_PAD1,
 * scheduling device, RULING #8435). Residual (SCHED): the ROM stores
 * D_1AE788 = 1 between the two epilogue restores (ld $16; lui/sw; ld $31); cc1
 * stores both flags after them. Both store orders give 4/20; an empty fence
 * between the stores moves it before ld $16 (3/20, still not the ROM's place). */
extern s32 WaitVblankGetField(s32 mode);
extern s32 g_bProgressiveScan;
s32 func_00350F28(s32 targetField) {
    while (WaitVblankGetField(0) == targetField) {
        s32 progressive = g_bProgressiveScan;
        R5900_SHORT_LOOP_PAD1(progressive, progressive);
        if (progressive != 0) {
            break;
        }
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
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 72.19% / engine96 53.51%. Residual: PACKED-SAVE (7 callee saves) + 35 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs).
 * Save stride NOT re-measured for this member on the s136os arm: its C arm does not compile
 * solo there (NOTE #9871). Of the 111 labeled members that were, 0 reproduce the 16-byte save
 * stride there (FACT #9873), so the stride is not evidence that this member is walled.
 * Residual: UNMEASURED. */
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

/**
 * Copy a window of arrived elementary-stream bytes into the pts ring.
 * Clamps the request to the ring's free header window and the descriptor's
 * remaining length, queries the ring's two writable spans (func_003506A8),
 * scatters into them (func_003511A8) and advances the ring (func_00350778).
 * @param unused   not read
 * @param desc     stream descriptor: +0x8 window start, +0xC bytes remaining
 *                 (both carry a 4-byte header)
 * @param ringBase bitstream ring; +0x50008 is its payload length
 * @return 1 if any bytes were stored, else 0
 *
 * The descriptor length is its own variable (`len`), and the second span
 * length is a new one (`remain = len - first`). `len` then dies before the
 * call to func_003506A8, so cc1 keeps the subtraction ahead of that jal and
 * reorg puts it in the delay slot, as the ROM does. Written `remain -= first`
 * on one variable, sched1 moves the subu past the call and the clamp's movn
 * lands in the delay slot instead: 2/57 words (measured by restoring that
 * spelling alone, task #1888). No devices.
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in tools/ee/s136os_functions.txt)
 * and spliced over S136OS_SLOT by tools/ee/s136os_splice.sh (task #1888). There
 * is no asm fallback: a build that skips the splice drops the function. On
 * native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003510C0)
S136OS_SLOT(func_003510C0);
#else
s32 func_003510C0(u8 *unused, u8 *desc, u8 *ringBase) {
    u8 *span0Ptr;
    s32 span0Len;
    u8 *span1Ptr;
    s32 span1Len;
    s32 ringEnd = *(s32 *)(ringBase + 0x50008);
    u32 wantEnd = *(s32 *)(desc + 8) + 4;
    u32 ringTop = (u32)(ringBase + ringEnd);
    s32 first;
    s32 len;
    s32 remain;
    s32 stored;

    if (ringTop <= wantEnd) {
        wantEnd -= ringEnd;
    }
    len = *(s32 *)(desc + 0xC) - 4;
    first = ringTop - wantEnd;
    if (len < first) {
        first = len;
    }
    remain = len - first;

    func_003506A8((FmvPtsQueue *)(g_pFmvArenaBase + FMV_PTS_OFS), &span0Ptr,
                  &span0Len, &span1Ptr, &span1Len);
    stored = func_003511A8(span0Ptr, span0Len, span1Ptr, span1Len, (u8 *)wantEnd,
                           first, ringBase, remain);
    func_00350778((FmvPtsQueue *)(g_pFmvArenaBase + FMV_PTS_OFS), stored);
    return stored > 0;
}
#endif

/**
 * Scatter two source spans into a two-segment ring destination.
 * The destination is dst0 (len0d bytes) followed by its wrap segment dst1
 * (cap bytes); the sources are src0 (len1d bytes) then src1 (tail bytes).
 * Copies them in order, splitting at the dst0/dst1 boundary.
 * @return the bytes written (len1d + tail), or 0 if they do not fit
 *
 * The overflow test is an early return, and the len1d >= len0d and
 * tail >= gap cases are written first: that is the ROM's block order. The
 * split offsets are spelled `p + a - b` (the ROM adds, then subtracts).
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in tools/ee/s136os_functions.txt)
 * and spliced over S136OS_SLOT by tools/ee/s136os_splice.sh (task #1834). There
 * is no asm fallback: a build that skips the splice drops the function. On
 * native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003511A8)
S136OS_SLOT(func_003511A8);
#else
s32 func_003511A8(u8 *dst0, s32 len0d, u8 *dst1, s32 cap, u8 *src0, s32 len1d,
                  u8 *src1, s32 tail) {
    if (len1d + tail > len0d + cap) {
        return 0;
    }
    if (len1d >= len0d) {
        memcpy(dst0, src0, len0d);
        memcpy(dst1, src0 + len0d, len1d - len0d);
        memcpy(dst1 + len1d - len0d, src1, tail);
    } else {
        s32 gap = len0d - len1d;
        if (tail >= gap) {
            memcpy(dst0, src0, len1d);
            memcpy(dst0 + len1d, src1, gap);
            memcpy(dst1, src1 + len0d - len1d, tail - gap);
        } else {
            memcpy(dst0, src0, len1d);
            memcpy(dst0 + len1d, src1, tail);
        }
    }
    return len1d + tail;
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
 *
 * @param arena  the FMV arena base (g_pFmvArenaBase); unused. The ROM caller
 *               passes it (func_003503D8, 0x003503EC delay slot), so the
 *               signature carries it (RULING #9118, task #1437).
 */
void func_003512F0(u8 *arena) {
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
 *
 * @param block  the arena block at arena+0xD9040; unused. The ROM caller passes
 *               it (func_003503D8, 0x00350490 delay slot), so the signature
 *               carries it (RULING #9118, task #1437).
 * @return       1 (ready).
 */
s32 func_003513F0(u8 *block) {
    return 1;
}

/* func_003513F8(obj, buf, byteLen, flag): kick off a CD sector read for the FMV
 * bitstream. Converts byteLen to 2KB sectors (>>11), issues sceCdRead
 * (sceCdRead) from the object's current LBN cursor (obj+0x4) into buf with a
 * fixed retry mode {trycount=0x64, spindlctrl=1, datapattern=0}. When flag != 0
 * it returns 0 without advancing (query/prime mode); otherwise it advances the
 * LBN cursor by the sector count, waits (sceCdSync, func_00124B88(0)) and returns
 * byteLen. Matching arm stays asm (8-byte-packed saves s0..s4/ra); the #else is
 * the structure-exact model (the CD I/O itself is the deferred FMV native
 * backend, but the call structure + cursor advance are exact). */
/* GUARD (task #1387): on EE the #else body below is the image's func_003513F8, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003513F8)
S136OS_SLOT(func_003513F8);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 64.74% / engine96 77.79%. Residual: PACKED-SAVE (6 callee saves) + 19 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
extern s32 sceCdRead(u32 lbn, u32 sectors, void *buf, void *mode); /* sceCdRead */
extern s32 func_00124B88(s32 mode);                                    /* sceCdSync */
/* MATCHED on the s136os arm (task #1387; screened by task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
 * masked): EXACT 39/39, relocations equal. The
 * ROM returns through a result held in $20 (0 unless the read is synchronous)
 * and never writes mode[3]. With an early `return 0`: 28/39, first diff @10 (no
 * $20 save); with mode[3] cleared: 20/39 (an extra sb). */
s32 func_003513F8(u8 *obj, void *buf, s32 byteLen, s32 flag) {
    u8 mode[4];
    s32 sectors = byteLen >> 11; /* bytes -> 2KB sectors */
    s32 result = 0;

    mode[0] = 0x64; /* trycount   */
    mode[1] = 1;    /* spindlctrl */
    mode[2] = 0;    /* datapattern; mode[3] is padding the ROM never writes */
    sceCdRead(*(s32 *)(obj + 0x4), sectors, buf, mode);
    if (flag == 0) {
        *(s32 *)(obj + 0x4) += sectors; /* advance the LBN cursor */
        func_00124B88(0);
        result = byteLen;
    }
    return result;
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

/**
 * FmvBitstreamObjInit: construct the IPU_TO bitstream sub-object. Stores the
 * source buffer and slot-array words, builds the +0x4 DMA-tag word
 * ((tagBase & 0x0FFFFFFF) | 0x20000000 = a "next" tag pointing at physical
 * tagBase), records the ring size and its byte length, creates the decode
 * semaphore (init/max = 1) into +0x40, initialises the ring via
 * FmvStreamStartDma, and clears the +0x48 fed-byte total.
 * @param obj        the sub-object (FmvStream + 0x48)
 * @param srcBase    physical base of the macroblock buffer          (+0x0)
 * @param tagBase    physical base of the source-tag ring            (+0x4)
 * @param ringSize   number of 0x800-byte ring blocks (+0x8; << 11 at +0x18)
 * @param slotBase   ring-slot array base                            (+0x50)
 * @param slotCount  ring-slot count                                 (+0x54)
 * @return 1, always
 *
 * MATCHED on the s136os arm (task #1801; SN 2.95.3 v1.36 -fopt-stack, FACT
 * #8810). Three things close it, each measured by undoing it alone on a solo
 * s136 compile (relocated fields masked):
 *  - the parameters are 32-bit: the ROM stores $5..$9 straight with sw. As u64
 *    every one is sign-extended with dsll32/dsra32 first (40 words vs 30).
 *    The caller FmvStreamInit forwards its own p4..p8, so they are s32 there
 *    too; its s136 block is text-identical either way.
 *  - FmvStreamStartDma is declared s32 (the ROM returns 1 from it). Declared
 *    void, cc1 orders the +0x40 store before the argument move and reorg fills
 *    the jal delay slot with the move: 2/30 words.
 *  - the statement order below: semaphore initCount before maxCount, ringSize
 *    before the tag word, ring bytes last. The old body's order is 12/30;
 *    reverting only the two semaphore stores is 2/30, only the ring stores
 *    12/30. Typing the stores (struct fields vs `*(u32 *)(obj + N)` casts)
 *    changes nothing here.
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_FmvBitstreamObjInit)
S136OS_SLOT(FmvBitstreamObjInit);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0011AC20(void *param);   /* CreateSema (returns sema id) */
extern s32 FmvStreamStartDma(u8 *stream);
/* (end of this body's declarations) */

s32 FmvBitstreamObjInit(u8 *obj, s32 srcBase, s32 tagBase, s32 ringSize,
                        s32 slotBase, s32 slotCount) {
    FmvBitstreamObj *o = (FmvBitstreamObj *)obj;
    FmvSemaParam sema;

    o->srcBase = srcBase;
    o->slotBase = slotBase;
    o->slotCount = slotCount;
    sema.initCount = 1;
    sema.maxCount = 1;
    o->ringSize = ringSize;
    o->tagWord = ((u32)tagBase & 0x0FFFFFFF) | 0x20000000;
    o->ringBytes = (u32)ringSize << 11;
    o->sema = func_0011AC20(&sema);
    FmvStreamStartDma(obj);
    o->totalBytes = 0;
    return 1;
}
#endif

/**
 * FmvStreamStartDma: initialise the IPU_TO bitstream sub-object's ring state and
 * build its DMA source-tag chain. Sets +0x44 to 1, clears the playback
 * counters, resets every DMA-add ring slot (+0x50 array, +0x54 slots: two -1
 * tags and two zeroed counters), emits one tag per 0x800-byte ring block via
 * func_003515C0 (tag id 3, qwc 0x80, address = the block) followed by a
 * zero-qwc id-2 tag addressed at the tag ring itself, then programs channel-4
 * QWC/MADR/TADR and suspends its CHCR via func_00351550(5).
 * @param stream the sub-object embedded at FmvStream+0x48 (type not yet
 *               recovered as a whole -> raw offsets)
 * @return 1, always (task #1801 found the old `void` declaration was the
 *         FmvBitstreamObjInit residual)
 *
 * MATCHED on the s136os arm (task #1888). Three things close it, each measured
 * by undoing it alone on a solo s136 compile (relocated fields masked):
 *  - each slot store re-reads the +0x50 array base, as the ROM does (four
 *    `lw 0x50` per iteration): one struct-array access per store. With the
 *    slot pointer formed once per iteration it is 80/88 words (82 built).
 *  - both loops are plain `for` loops with no enclosing `if (n > 0)`: cc1
 *    then emits the ROM's single blez guard. With the guards it is 90/88
 *    (92 built: a second, redundant entry test per loop).
 *  - the block's source address is written `(i << 11) + base`: the other
 *    operand order swaps the addu operands, 3/88.
 * No devices.
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in tools/ee/s136os_functions.txt)
 * and spliced over S136OS_SLOT by tools/ee/s136os_splice.sh (task #1888). There
 * is no asm fallback: a build that skips the splice drops the function. On
 * native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_FmvStreamStartDma)
S136OS_SLOT(FmvStreamStartDma);
#else
/* One DMA-add ring slot (stride 0x18; FmvDmaAddCmd's layout at func_003521B0). */
typedef struct FmvDmaAddSlot {
    /* 0x00 */ s64 tag0; /* -1 = empty */
    /* 0x08 */ s64 tag1;
    /* 0x10 */ s32 arg0; /* position within the sector (func_00352058) */
    /* 0x14 */ s32 arg1; /* bytes still unconsumed (func_00352058) */
} FmvDmaAddSlot;

s32 FmvStreamStartDma(u8 *stream) {
    s32 i;

    *(s32 *)(stream + 0x44) = 1;
    *(s32 *)(stream + 0xC) = 0;
    *(s32 *)(stream + 0x10) = 0;
    *(s32 *)(stream + 0x14) = 0;
    *(s32 *)(stream + 0x58) = 0;
    *(s32 *)(stream + 0x5C) = 0;

    for (i = 0; i < *(s32 *)(stream + 0x54); i++) {
        (*(FmvDmaAddSlot **)(stream + 0x50))[i].tag0 = -1;
        (*(FmvDmaAddSlot **)(stream + 0x50))[i].tag1 = -1;
        (*(FmvDmaAddSlot **)(stream + 0x50))[i].arg0 = 0;
        (*(FmvDmaAddSlot **)(stream + 0x50))[i].arg1 = 0;
    }

    for (i = 0; i < *(s32 *)(stream + 0x8); i++) {
        func_003515C0((u64 *)(*(u32 *)(stream + 0x4) + i * 0x10),
                      ((i << 11) + *(u32 *)(stream + 0x0)) & 0x0FFFFFFF, 3, 0x80);
    }

    func_003515C0((u64 *)(*(u32 *)(stream + 0x4) + i * 0x10),
                  *(u32 *)(stream + 0x4) & 0x0FFFFFFF, 2, 0);

    *(volatile u32 *)0x1000B420 = 0;                                    /* ch4 QWC  */
    *(volatile u32 *)0x1000B410 = *(u32 *)(stream + 0x0) & 0x0FFFFFFF;  /* ch4 MADR */
    *(volatile u32 *)0x1000B430 = *(u32 *)(stream + 0x4) & 0x0FFFFFFF;  /* ch4 TADR */
    func_00351550(5);                                                   /* ch4 CHCR suspend */
    return 1;
}
#endif

/* WALL: deferred-native body had a signature inconsistency with its
   forwarder/caller (caught by the TARGET_NATIVE compile sweep). Left bare
   INCLUDE_ASM (no #else); revisit with the asm when the FMV native backend
   is built. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003517C0);

/**
 * func_003518B8: account `n` freshly fed bytes to the IPU_TO bitstream object
 * under its semaphore — add them to the pending count at +0x14 (which
 * func_00351910 turns into new DMA source tags, 0x800 bytes per tag) and to
 * the 64-bit running total at +0x48.
 * @param stream  the bitstream sub-object (FmvStream + 0x48)
 * @param n       bytes just fed (func_00350F88's `stored`; func_003525B0
 *                passes its own a1 straight through)
 * @return SignalSema's result
 *
 * MATCHED on the s136os arm (task #1801; SN 2.95.3 v1.36 -fopt-stack, FACT
 * #8810) as first written, no devices. The tree had no C for it before: the
 * old note blamed 8-byte-packed saves, which SN 1.36 -fopt-stack emits.
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003518B8)
S136OS_SLOT(func_003518B8);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0011AC60(s32 sema);    /* WaitSema (acquire) */
extern s32 func_0011AC40(s32 sema);    /* SignalSema (release) */
/* (end of this body's declarations) */
s32 func_003518B8(void *stream, s32 n) {
    FmvBitstreamObj *o = (FmvBitstreamObj *)stream;

    func_0011AC60(o->sema);
    o->pendingBytes += n;
    o->totalBytes += n;
    return func_0011AC40(o->sema);
}
#endif

/**
 * func_00351910: top up the IPU_TO (DMAC ch4) source-tag ring with the bytes fed
 * since the last call, and restart the channel.
 * Under the stream semaphore: if the ring is not armed (+0x44, set by
 * FmvStreamStartDma) it reports D_1AE800 through func_003504C8 and returns 0.
 * Otherwise it suspends ch4 (func_00351550(5)), snapshots CHCR and MADR, and
 * converts MADR to a ring block with func_00351498. Every block between the old
 * head and that one has been consumed: head moves forward and queued drops by
 * that count. pendingBytes / 0x800 whole blocks (nTags) are then tagged, from
 * tail = head + queued onward. Before them, the last block already queued
 * (tail - 1) is re-tagged as REF (id 3), so the old chain end now links on into
 * the new tags. The new tags are REF except the last, which is REFE (id 0) and
 * ends the chain; each moves 0x80 qwords (one block). func_003515C0's third
 * argument is the tag ID and its fourth the QWC (the ID lands at bit 28). queued
 * grows by nTags. If anything is queued, ch4 is restarted with CHCR | 0x100
 * (STR). When tags were added, CHCR's top nibble (the TAG field of the last tag
 * read) is first set to 3, so the restart sees a REF tag, not the old REFE.
 * Ring arithmetic is signed `%`. The ROM's beql/break 0,7 pairs are cc1's
 * divide-by-zero traps, not code.
 * @param dmaq  the IPU_TO bitstream sub-object (FmvStream + 0x48)
 * @return 1, or 0 when the ring is not armed
 *
 * MATCHED on the s136os arm (task #1950; SN 2.95.3 v1.36 -fopt-stack, FACT
 * #8810), no devices. The old body screened 88/127 (FACT #9920). The cause was
 * not a register wall: five phrasings set cc1's allocation and schedule. Each
 * was measured by undoing it alone on a solo s136 compile (relocated fields
 * masked):
 *  - the loop walks its own cursor `block`, copied from `start`. With the start
 *    index computed straight into the loop variable, global-alloc ranks it
 *    below i and dmaq and colours them $18/$16/$17, against the ROM's
 *    $16/$17/$18 (89/127, 129 built). The copy gives the loop cursor a short
 *    live range, so it ranks first, and `start` takes its register by
 *    preference (the ROM's `mfhi $16`).
 *  - the loop is a plain `for` with no enclosing `if (nTags > 0)`. With the
 *    guard, cc1 threads the prime block's test past the loop and adds a second
 *    entry test (53/127, 129 built).
 *  - the new head is computed before queued: 16/127 the other way round
 *    (local-alloc swaps their registers, and tail moves out of $8).
 *  - the prime index adds `last = ringSize - 1` as a local: inline, cc1
 *    reassociates it as (tail + ringSize) - 1 (4/127).
 *  - `emitted = 1` follows the prime call: before it, `li $23,1` issues ahead
 *    of the argument constants (3/127).
 *  - the ring and source bases are read as s32: the struct's u32 fields would
 *    zero-extend each u64 address argument with dsll32/dsrl32 (64/127, 131
 *    built).
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00351910)
S136OS_SLOT(func_00351910);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0011AC60(s32 sema);    /* WaitSema (acquire) */
extern s32 func_0011AC40(s32 sema);    /* SignalSema (release) */
/* (end of this body's declarations) */
s32 func_00351910(void *dmaq) {
    extern char D_1AE800[];   /* FMV "IPU_TO ring not armed" error string */
    FmvBitstreamObj *o = (FmvBitstreamObj *)dmaq;
    s32 ringSize;
    s32 consumed;     /* blocks the channel finished since the last call */
    s32 queued;
    s32 head;
    s32 tail;         /* first block not yet tagged: head + queued */
    s32 start;        /* tail, wrapped into the ring */
    s32 block;        /* loop cursor: ring block being tagged */
    s32 nTags;        /* whole blocks of pendingBytes to tag */
    s32 pending;
    u32 chcr;         /* ch4 CHCR snapshot */
    u32 madr;         /* ch4 MADR snapshot */
    s32 emitted = 0;
    s32 i;

    func_0011AC60(o->sema);                    /* WaitSema */
    if (o->armed == 0) {
        func_003504C8(D_1AE800);
        return 0;
    }

    func_00351550(5);                          /* suspend ch4 */
    chcr = *(volatile u32 *)0x1000B400;        /* D4_CHCR */
    madr = *(volatile u32 *)0x1000B410;        /* D4_MADR */

    consumed = func_00351498((u32 *)dmaq, madr);
    ringSize = o->ringSize;
    consumed = (consumed + ringSize - o->head) % ringSize;
    head = (o->head + consumed) % ringSize;
    queued = o->queued - consumed;
    o->queued = queued;
    o->head = head;
    tail = head + queued;
    start = tail % ringSize;

    pending = o->pendingBytes;
    nTags = pending / 0x800;
    o->pendingBytes = pending - nTags * 0x800; /* keep the partial block */

    if (nTags > 0) {
        /* re-tag the old chain end as REF so it links on into the new tags */
        s32 last = ringSize - 1;
        s32 idx = (tail + last) % ringSize;
        func_003515C0((u64 *)((s32)o->tagWord + idx * 0x10),
                      (u64)((s32)o->srcBase + idx * 0x800), 3, 0x80);
        emitted = 1;
    }

    block = start;
    for (i = 0; i < nTags; i++) {
        u64 id = (i != nTags - 1) ? 3 : 0;     /* REF, the last one REFE */
        func_003515C0((u64 *)((s32)o->tagWord + block * 0x10),
                      (u64)((s32)o->srcBase + block * 0x800), id, 0x80);
        block = (block + 1) % o->ringSize;
    }

    o->queued = o->queued + nTags;
    if (o->queued != 0) {
        if (emitted) {
            chcr = (chcr & 0x0FFFFFFF) | 0x30000000;   /* TAG.ID = REF */
        }
        func_00351550(chcr | 0x100);           /* STR: restart ch4 */
    }

    func_0011AC40(o->sema);                    /* SignalSema */
    return 1;
}
#endif

/* func_00351B10: snapshot + halt the IPU DMA channels (save-state capture).
 * Acquires the stream sema, suspends IPU_TO (ch4, func_00351550(5)) and records
 * its MADR/TADR/QWC/CHCR into dmaq+0x1C..0x28, spin-waits for the IPU to drain
 * (IPU_CTRL & 0xF0), suspends IPU_FROM (ch3, func_003514E0(0)) and records its
 * MADR/QWC/CHCR + IPU_BP/IPU_CTRL into dmaq+0x2C..0x3C, then releases the sema.
 * Returns 1. Raw offsets (dmaq sub-object type not recovered).
 * MATCHED on the s136os arm (task #1387; screened by task #1382, s136 arm = SN 1.36 -fopt-stack, solo, relocated fields
 * masked): EXACT 68/68, relocations equal. The
 * drain loop carries the ROM's three R5900 short-loop pad nops before its bnez
 * (R5900_SHORT_LOOP_PAD1, a scheduling device); without them the screen is
 * 34/68, built 65 words, first diff @29. */
/* GUARD (task #1387): on EE the #else body below is the image's func_00351B10, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00351B10)
S136OS_SLOT(func_00351B10);
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

/* func_00352000: sema-guarded 2KB round-up of the byte cursor. */
/* GUARD (task #1394): on EE the #else body below is the image's func_00352000, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00352000)
S136OS_SLOT(func_00352000);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0011AC60(s32 sema);    /* WaitSema (acquire) */
extern s32 func_0011AC40(s32 sema);    /* SignalSema (release) */
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 75.67% / engine96 58.62%. Residual: PACKED-SAVE (2 callee saves) + 12 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* The 8-byte-packed callee saves (s0/ra) are SN 2.95.3 v1.36
   -fopt-stack's (FACT #8810), which the s136os arm compiles this body with.

   NOTE(type): `obj` is the embedded bitstream sub-object (FmvStream+0x48); sema
   +0x40, residual byte cursor +0x14 (raw offsets, sibling type not recovered).

   Snap the residual byte cursor up to the next 2KB (0x800) boundary, under the
   object's sema. The asm biases by 0x7FF then arithmetic-right-shifts by 11
   (with the +0xFFE negative-input correction the compiler inserts); the cursor
   is a non-negative byte count, so this is the usual round-up-to-2KB.
   RETURN: the asm stores the rounded cursor in the SignalSema jal delay slot, so
   $2 at `jr` is SignalSema's return value (NOT the rounded cursor) - return that.
   (Fixes else_divergences #15: the return diverged on real R5900.)

   MATCHED on the s136os arm (task #1394: vmu with the base seeded, image
   cmp 0; screened by task #1389, SN 2.95.3 v1.36 -fopt-stack, masked words
   and relocations).
   Levers: the round-up written as the signed division it is,
   (v + 0x7FF) / 0x800 * 0x800 - cc1 expands it to the ROM's slt/movn
   (the +0xFFE is the division's own negative-input bias), where the
   hand-expanded ?: compiled to a bltzl; and the semaphore calls by their ROM
   symbols func_0011AC60/func_0011AC40, declared returning s32 as the kernel
   calls do (WaitSema/SignalSema have no EE definition; with the old `void
   WaitSema` declaration cc1 colours the cursor $2 and the -1 constant $3). */
s32 func_00352000(u8 *obj) {
    s32 v;

    func_0011AC60(*(s32 *)(obj + 0x40));
    v = *(s32 *)(obj + 0x14);
    *(s32 *)(obj + 0x14) = (v + 0x7FF) / 0x800 * 0x800;
    return func_0011AC40(*(s32 *)(obj + 0x40));
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
 * copies {s64,s64,s32,s32} into ring[writeIdx@0x5C] (base@0x50, stride 0x18),
 * bumps the count and wraps the write index modulo capacity.
 *   dmaq: the FMV DMA-add queue; cmd: the command to copy.
 *   returns 1 if the queue had room (the sentinel is accepted and dropped, as
 *   the ROM sets the result in the sentinel branch's delay slot too), 0 if full.
 * The ring slot is re-addressed for every field store (the stores may alias the
 * queue), as the ROM does. The sema calls use their ROM symbols: WaitSema and
 * SignalSema have no EE definition (as in func_00352000).
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in tools/ee/s136os_functions.txt)
 * and spliced over S136OS_SLOT by tools/ee/s136os_splice.sh (task #1834). There
 * is no asm fallback: a build that skips the splice drops the function. On
 * native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003521B0)
S136OS_SLOT(func_003521B0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0011AC60(s32 sema);
extern s32 func_00352058(u8 *obj, u8 *req);
extern s32 func_0011AC40(s32 sema);
/* (end of this body's declarations) */
typedef struct FmvDmaAddCmd {
    /* 0x00 */ s64 tag0;
    /* 0x08 */ s64 tag1;
    /* 0x10 */ s32 arg0;
    /* 0x14 */ s32 arg1;
} FmvDmaAddCmd;
typedef struct FmvDmaAddQueue {
    /* 0x00 */ u8 pad00[0x40];
    /* 0x40 */ s32 sema;
    /* 0x44 */ u8 pad44[0xC];
    /* 0x50 */ FmvDmaAddCmd *ring;
    /* 0x54 */ s32 capacity;
    /* 0x58 */ s32 count;
    /* 0x5C */ s32 writeIdx;
} FmvDmaAddQueue;
s32 func_003521B0(void *dmaq, void *cmd) {
    FmvDmaAddQueue *q = (FmvDmaAddQueue *)dmaq;
    FmvDmaAddCmd *c = (FmvDmaAddCmd *)cmd;
    s32 result = 0;

    func_0011AC60(q->sema);
    if (q->count < q->capacity) {
        func_00352058((u8 *)q, (u8 *)c);
        if (c->tag0 >= 0 || c->tag1 >= 0) {
            q->ring[q->writeIdx].tag0 = c->tag0;
            q->ring[q->writeIdx].tag1 = c->tag1;
            q->ring[q->writeIdx].arg0 = c->arg0;
            q->ring[q->writeIdx].arg1 = c->arg1;
            q->count += 1;
            q->writeIdx = (q->writeIdx + 1) % q->capacity;
        }
        result = 1;
    }
    func_0011AC40(q->sema);
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
extern void *IpuInitDecoder(void); /* DECL-LEVER(#2025): defined (obj, buf, size) in cod/022FA8; FmvStreamInit forwards its own incoming $a0..$a2 through a bare `jal` (the ROM sets none of them before the call), which this argument-less declaration reproduces, as #2011 did for func_0012EE28 */
extern s32 func_0012FA70(u8 *obj, s32 slot, void *cb, s32 arg);
extern void func_003525D0(FmvStream *s);
extern s32 FmvBitstreamObjInit(u8 *obj, s32 a, s32 b, s32 c, s32 d, s32 e);
extern s32 func_00352A20(s32 unused, s32 *frame);
extern s32 func_00352A48(void);
extern s32 func_00352A80(void);
extern s32 func_00352AB0(void);
extern s32 func_00352AE0(s32 unused, u8 *obj);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 94.22% / engine96 85.55%. Residual: PACKED-SAVE (7 callee saves) + 11 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/** FmvStreamInit - construct the FMV stream object: reset its message dispatch
 *  table (IpuInitDecoder), register the five stream-event callbacks (frame-drop
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
s32 FmvStreamInit(FmvStream *obj, u64 p2, u64 p3, s32 p4, s32 p5, s32 p6, s32 p7,
                  s32 p8) {
    IpuInitDecoder();
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
 * Forward to the fed-byte accounting (func_003518B8) of the embedded bitstream
 * object; `n` passes through in $a1 untouched (the ROM only rewrites $a0).
 */
s32 func_003525B0(FmvStream *obj, s32 n) {
    return func_003518B8((u8 *)obj + 0x48, n);
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

/**
 * Request a stop: mark the playback FSM "stop" (state 1).
 * @param s  the FMV stream
 *
 * Returns nothing. The ROM leaves $v0 = 1 only because $v0 is the register
 * its store of the constant goes through; the one caller (FmvStreamFeedLoop,
 * jal at 0x34FEE4) overwrites $v0 with its next call, func_003512F8, on both
 * paths to 0x34FEEC without reading it. (Earlier C returned 1, which made both
 * cc1 2.9 and SN 1.36 materialise `li 1` twice, #6521's ARTIFACT class.)
 *
 * MATCHED on the s136os arm (task #1769; SN 2.95.3 v1.36 -fopt-stack, FACT
 * #8810) as `void`: `li $2,1; jr $31; sw $2,0xA8($4)`, the ROM's three words.
 * Its MATCH_FmvRequestStop engine96 guard (task #513) is retired with this
 * promotion: the function is image-resident, no longer arm-scored (RULING
 * #8118), and it was the unit's only MATCH_ member.
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (row in tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_FmvRequestStop)
S136OS_SLOT(FmvRequestStop);
#else
void FmvRequestStop(FmvStream *s) {
    s->state = 1;
}
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
 * Blocked (match, cc1 2.9 arm): 8-byte-packed saves (s0@0x20, ra@0x28).
 * PARKED #70 (#else not confident): the asm sets up 4 stack out-param pointers
 * (sp+0x10/+0x14/+0x18/+0x1C) before `jal func_00352590`, then reads sp+0x14/+0x1C
 * (sum<4 -> early-out) and sp+0x10/+0x18 (masked &0xFFFFFFF | 0x20000000 into DMATAGs
 * for func_003511A8), also copying D_1AE818 (unaligned lwl/lwr) to sp+0x0 — but Ghidra
 * decompiles func_00352590 as a 1-param forwarder `FUN_003517c0(p+0x48)`, contradicting
 * the 4-out-param wiring. Resolve func_00352590.s + func_003517c0 (does it write the 4
 * slots?) + func_003511A8's arg arity before writing a faithful #else. Not forcing a
 * low-confidence body.
 * Save stride NOT re-measured for this member on the s136os arm: it has no C arm to compile
 * (NOTE #9871). Of the 111 labeled members that were, 0 reproduce the 16-byte save stride there
 * (FACT #9873), so the stride is not evidence that this member is walled. Residual: UNMEASURED. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/250080", func_003526A8);

/* func_00352780: poll stream-done then host-side done. */
/* GUARD (task #1394): on EE the #else body below is the image's func_00352780, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00352780)
S136OS_SLOT(func_00352780);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 func_0012F9B8(u8 *host);
/* (end of this body's declarations) */
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 75.39% / engine96 75.61%. Residual: PACKED-SAVE (3 callee saves) + 8 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* The 8-byte-packed callee saves (s0/s1/ra) are SN 2.95.3 v1.36
   -fopt-stack's (FACT #8810), which the s136os arm compiles this body with.

   End-of-playback poll: false while the embedded stream still has bytes queued
   (func_00352680 != 0); once the stream has drained, report whether the host
   side has also signalled done (func_0012F9B8 != 0).

   MATCHED on the s136os arm (task #1394: vmu with the base seeded, image
   cmp 0; screened by task #1389, SN 2.95.3 v1.36 -fopt-stack, masked words
   and relocations).
   Lever: the result held in a variable initialised 0 (the ROM's $17) instead
   of an early `return 0` (17/18, a 0x10 frame with no $17), as func_003513F8. */
s32 func_00352780(FmvStream *obj) {
    s32 done = 0;

    if (func_00352680(obj) == 0) {
        done = func_0012F9B8((u8 *)obj) != 0;
    }
    return done;
}
#endif

/* FmvDecodeThreadEntry: the FMV decode-thread main loop. Resets the embedded stream
 * ring (FmvStreamStartDma on obj+0x48), initialises the arena frame queue
 * (func_00352B90 at +0xD9168), primes the host frame reader (FmvDisplayWorkerLoop), then
 * decodes frames (func_00352620) as long as the arena's "more data" flag
 * (+0xD9174) stays set and no frame reports completion (returns 1), and finally
 * drives the stream to its terminal state 3 (func_00352628).
 *
 * @param obj  the FMV stream object (arena+0xD9048).
 * @return     func_00352628's result for the terminal state.
 *
 * The loop re-reads the arena base each pass. The ROM reads it absolute
 * (lui/lw, 0x00352814) before the loop and gp-relative in the back-edge
 * branch's delay slot (0x0035283C); cc1 rotates the loop condition into exactly
 * those two copies. The two reads are written as two C expressions so that only
 * the back-edge copy goes through FMV_ARENA_BASE_GP (the device above): one
 * expression through the device makes BOTH copies gp-relative (19/39, task
 * #1434), and without the device the back-edge copy is lui/lw (13/39).
 *
 * GUARD (task #1434): on EE this C is the image's FmvDecodeThreadEntry, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C.
 * History: task #513 measured sdk29 92.38% / engine96 64.59% (unit objdiff report,
 * packed saves); task #1389's s136 screen read 13/39 for the single-expression
 * loop. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_FmvDecodeThreadEntry)
S136OS_SLOT(FmvDecodeThreadEntry);
#else
extern s32 FmvStreamStartDma(u8 *stream);
extern s32 FmvDisplayWorkerLoop(u8 *host);
extern void func_00352B90(FmvFrameQueue *q);
s32 FmvDecodeThreadEntry(FmvStream *obj) {
    u8 *arena;

    FmvStreamStartDma((u8 *)obj + 0x48);
    func_00352B90((FmvFrameQueue *)(g_pFmvArenaBase + 0xD9168));
    FmvDisplayWorkerLoop((u8 *)obj);
    arena = g_pFmvArenaBase;
    while (*(s32 *)(arena + 0xD9174) != 0) {
        if (func_00352620(obj) == 1) {
            break;
        }
        arena = FMV_ARENA_BASE_GP;
    }
    return func_00352628(obj, 3);
}
#endif

/**
 * FmvDisplayWorkerLoop: the FMV frame-display worker loop. Pulls the next
 * decoded picture from the host (func_0012F9A8/func_0012F950), waits (yielding)
 * for a free display slot (FmvFrameQueueGetWriteSlot), builds the per-tile GIF
 * display chain for each frame tile (func_00350B60) unless host word +0x8 is
 * set, commits the slot (FmvFrameQueuePush), and yields. Exits when the host
 * signals end-of-stream or the FSM reports stop.
 * @param host the host frame-reader object (+0x0/+0x4 the two DMA addresses
 *             handed to func_00350B60, +0x8 a skip-tiles flag)
 * @return 1 on end-of-stream, -1 when func_00352620 reports stop
 *
 * MATCHED on the s136os arm (task #1888). Three things close it, each measured
 * by undoing it alone on a solo s136 compile (relocated fields masked):
 *  - the host words are read through `host` itself. Through a separate
 *    `s32 *` copy declared in the loop, cc1 spills the copy to the stack and
 *    reloads it: 93/110 words (112 built).
 *  - the queue push takes the arena base from a join: the skip path reads it
 *    through FMV_ARENA_BASE_GP, the tile path through g_pFmvArenaBase. The ROM
 *    reads it gp-relative in the `bnez` delay slot (0x003528F8
 *    `lw $2,%gp_rel(g_pFmvArenaBase)($28)`) and absolute after the tile loop
 *    (0x0035298C). FMV_ARENA_BASE_GP is an ADDRESSING-MODEL DEVICE (RULING
 *    #9073 under RULING #8620; the file-scope offset-0 equate defined at the
 *    top of this file, emits no code, its relocation names the real symbol).
 *    Without it, either spelling of the push: 75/110 (112 built).
 *  - the tile offsets are written `tile * 0x138C0` and `tile * 0xD0000`:
 *    loop.c strength-reduces them and zeroes the two new induction registers
 *    in the preheader, after the blez, as the ROM does. Written as running
 *    `+=` accumulators they are zeroed before the guard in swapped registers:
 *    11/110.
 * GUARD: on EE this C is the image's body, compiled alone by the s136os arm
 * (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in tools/ee/s136os_functions.txt)
 * and spliced over S136OS_SLOT by tools/ee/s136os_splice.sh (task #1888). There
 * is no asm fallback: a build that skips the splice drops the function. On
 * native it is plain C, and FMV_ARENA_BASE_GP is g_pFmvArenaBase.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_FmvDisplayWorkerLoop)
S136OS_SLOT(FmvDisplayWorkerLoop);
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
s32 FmvDisplayWorkerLoop(u8 *host) {
    s32 result = 1;

    for (;;) {
        u8 *slot;
        u8 *arena;

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
        if (((s32 *)host)[2] != 0) {
            arena = FMV_ARENA_BASE_GP;
        } else {
            u32 madr = ((s32 *)host)[0];
            u32 tadr = ((s32 *)host)[1];
            s32 tile;

            for (tile = 0; tile < *(s32 *)(g_pFmvArenaBase + 0xD9178); tile++) {
                func_00350B60(*(u8 **)(g_pFmvArenaBase + 0xD916C) + tile * 0x138C0 + 0x40,
                              *(u8 **)(g_pFmvArenaBase + 0xD9168) + tile * 0xD0000, madr, tadr,
                              tile);
            }
            arena = g_pFmvArenaBase;
        }
        FmvFrameQueuePush(arena + 0xD9168);
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
 *
 * @param frameQueue  the decoded-frame display queue (arena+FMV_FRAMEQ_OFS);
 *                    unused. The ROM caller passes it (func_003503D8, 0x00350404
 *                    delay slot), so the signature carries it (RULING #9118,
 *                    task #1437).
 */
void func_00352B88(u8 *frameQueue) {
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

/* FmvFrameQueuePush: commit the just-decoded slot (mark state 2, advance the
 * write cursor modulo capacity) under DI/EI. */
/* GUARD (task #1394): on EE the #else body below is the image's FmvFrameQueuePush, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_FmvFrameQueuePush)
S136OS_SLOT(FmvFrameQueuePush);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 66.50% / engine96 68.10%. Residual: PACKED-SAVE (2 callee saves) + 21 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* The 8-byte-packed callee saves (s0/ra) are SN 2.95.3 v1.36
   -fopt-stack's (FACT #8810), which the s136os arm compiles this body with.

   Commit the just-decoded frame slot for display, with interrupts disabled:
   mark the current write slot's state word = 2 (ready), bump the queued count,
   and advance the write cursor modulo the slot capacity.

   MATCHED on the s136os arm (task #1394: vmu with the base seeded, image
   cmp 0; screened by task #1389, SN 2.95.3 v1.36 -fopt-stack, masked words
   and relocations).
   Lever: writeIdx and count accessed through volatile views - a CODEGEN DEVICE
   (RULING #8404 class), as func_00352B90 and func_00352CE8 already view these
   fields. Volatile accesses keep source order, so the count read-modify-write
   completes before writeIdx is re-read, as in the ROM; with either view plain,
   cc1 hoists the writeIdx reload and the div above the count store (first diff
   @5, lw 0x8 into $4 vs $3). Writer census: see FmvFrameQueueGetDisplaySlot. */
void FmvFrameQueuePush(u8 *fq) {
    FmvFrameQueue *q = (FmvFrameQueue *)fq;

    func_0011F5E0();   /* DI */
    {
        volatile s32 *writeIdx = &q->writeIdx;
        volatile s32 *count = &q->count;

        *(s32 *)(q->frames + *writeIdx * 0x138C0) = 2;
        *count = *count + 1;
        *writeIdx = (*writeIdx + 1) % q->capacity;
    }
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

/* FmvFrameQueueGetDisplaySlot: oldest queued decoded-frame pointer ((writeIdx - count +
 * cap) % cap slot). */
/* GUARD (task #1394): on EE the #else body below is the image's FmvFrameQueueGetDisplaySlot, compiled
 * alone by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_FmvFrameQueueGetDisplaySlot)
S136OS_SLOT(FmvFrameQueueGetDisplaySlot);
#else
/* MEASURED (task #513, 2026-09-20, whole-unit both-arms screen at origin/master 96f30718, objdiff_build.sh + unit_report.sh; sdk29 = this body alone on cc1 2.9 -O2 -G8 -fno-gcse, engine96 = all 39 arms MATCH_-guarded together on cc1 2.96-001003-1): sdk29 96.52% / engine96 78.12%. Residual: PACKED-SAVE (2 callee saves) + 4 non-save residual words (REGALLOC/SCHED) on sdk29; SCHED on engine96 (instruction set identical, order differs). */
/* Address of the oldest decoded frame still queued for display: 0 if the queue
   is empty, else the frame slot at ((writeIdx - count + capacity) % capacity)
   in the 0x138C0-stride slot array. Read by OnFmvVblankFlip each field.

   MATCHED on the s136os arm (task #1394: vmu with the base seeded, image
   cmp 0; screened by task #1389, SN 2.95.3 v1.36 -fopt-stack, masked words
   and relocations).
   The writeIdx and count reads are volatile: a CODEGEN DEVICE (RULING #8404
   class), not a claim about this read. Two volatile reads keep source order, so
   cc1 loads writeIdx before count as the ROM does; with either one plain, the
   scheduler loads count first (first diff @7, lw 0xC vs lw 0x8). Writer census
   of both fields: func_00352B90 (reset, already a volatile view),
   FmvFrameQueuePush (decode thread, under DI/EI), func_00352CE8 (release,
   already a volatile count) - the queue is shared with the vblank handler. */
s32 FmvFrameQueueGetDisplaySlot(FmvFrameQueue *q) {
    s32 writeIdx;
    s32 count;
    s32 idx;

    if (func_00352C70(q) != 0) {
        return 0;
    }
    writeIdx = *(volatile s32 *)&q->writeIdx;
    count = *(volatile s32 *)&q->count;
    idx = (writeIdx - count + q->capacity) % q->capacity;
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

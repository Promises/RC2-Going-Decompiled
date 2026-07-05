#include "common.h"

/* ===== EU build (SCES_516.07). Shared declarations + region-agnostic
 * function bodies ported from the USA unit (src/usa/cod/015180.c).
 * objdiff masks the gp/reloc deltas (EU gp 0x1AF070 vs USA 0x1AEFF0),
 * so the C bodies are identical; only the boot/crt0 PAL-vs-NTSC funcs
 * and a couple of padding-pin splits differ and stay INCLUDE_ASM. ===== */

/* EE-kernel parameter blocks (see include/rtl/ee/eekernel.h); declared locally
 * so this unit stays free of the full SDK header (whose eetypes.h would clash
 * with common.h's base typedefs). Layout is byte-identical to the SDK. */
struct ThreadParam {
    s32  status;
    void *entry;
    void *stack;
    s32  stackSize;
    void *gpReg;
    s32  initPriority;
    s32  currentPriority;
    u32  attr;
    u32  option;
    s32  waitType;
    s32  waitId;
    s32  wakeupCount;
};

struct SemaParam {
    s32 currentCount;
    s32 maxCount;
    s32 initCount;
    s32 numWaitThreads;
    u32 attr;
    u32 option;
};

extern s32 D_00133EF4;
extern s32 D_0013A388;

/* 0xCDCDCDCD inter-function-fill class (func_001158F4, func_0011F364,
 * func_0011FB8C, func_00124414, func_00125D94, func_00126104, func_00126284,
 * func_0012646C, func_0012672C, func_00126DBC, func_00130A8C). Each of these
 * symbols begins with one or more leading 0xCDCDCDCD debug-fill words emitted
 * between functions; spimdisasm folds the fill into the symbol's body.
 *
 * UNRECOVERABLE with the current splat/spimdisasm (1.41.0): unlike the
 * `addiu sp,+0xN; nop` padding mis-splits (which were fixed via symbol_addrs
 * boundary pins), these do NOT cleanly split:
 *   - The fill is most often a SINGLE word (0x4). spimdisasm's function-end
 *     detection only looks one symbol-pair (currentVram+8) ahead and its
 *     userDeclaredSize end-check requires instructionOffset+8 == start+size,
 *     which is unsatisfiable for a 4-byte function — so a lone CD word can
 *     never terminate as its own function (pinning size:0x4 balloons the file).
 *   - The real bodies are reached only by `j`/data-reference (e.g. the
 *     func_00130A8C j-thunk to func_00130B80), never by `jal`, so `_findCalls`
 *     never promotes them to function starts.
 *   - Several bodies are handwritten VU0/GS code (func_0011F364 sq-context save,
 *     func_00124414 cfc2/ctc2 GS sync) flagged as unimplemented instructions,
 *     which spimdisasm emits as a symbol, not a function.
 * Declaring the real-start symbol truncates the pad correctly but DROPS the body
 * from the nonmatchings tree (a coverage hole), so the boundaries are left at
 * the spimdisasm default (fill attached to the body). Re-evaluate if splat/
 * spimdisasm gains lone-word-pad splitting. */

/* func_00115C90: clears D_00133E78, calls func_0011B270(arg1); on failure
 * (-1) writes the resulting D_00133E78 error code back through arg0. Logic
 * matches but NOT byte-exact: the original saves $16/$17/$31 with 128-bit `sq`
 * while this cc1 emits `sd` for callee-saves. Left as INCLUDE_ASM. */

/* func_00115E68: tail-calls func_001175F0(arg0, 0, 0xA) and returns its result
 * sign-extended from 32 to 64 bits. Not matched: the original saves $31 with a
 * 128-bit `sq` (not the `sd` ee-gcc emits here at -O2 -G0) and carries an extra
 * dsll32/dsra32 sign-extend that cc1 elides for an s32-returning callee. Both
 * are codegen/ABI forms this compiler won't reproduce. Left as INCLUDE_ASM. */

extern s32 D_00134708;

extern void func_0011B050(s32 count, s32 *value);

/* func_0011B978(arg0, arg1, arg2): pack a four-word command record (low 16 bits
 * of arg0, arg1, arg2, and the uncached-mirror address of D_0013CA10 ORed with
 * 0x20000000) and push it through func_0011B050 with count 1. Instructions are
 * essentially identical but NOT byte-exact: the original schedules the prologue
 * `sd $31` and `move a1,sp` after the record stores, an ordering ee-gcc won't
 * reproduce from source. Left as INCLUDE_ASM. */

/* func_0011BA20 / func_0011BA58: pack arg0, arg1 and the low 16 bits of arg2
 * into a stack record and push it through func_0011B050 (count -5 / -6). Not
 * matched: the original moves arg1 out of $5 into a temp before reusing $5 for
 * the record address, so it stores arg1 via the temp. ee-gcc instead stores
 * arg1 directly from $5 before clobbering it — a register-allocation/scheduling
 * order this cc1 won't reproduce from C. Left as INCLUDE_ASM. */

/* Global list head initialised by func_0011BAC8: value word, entry count,
 * head/tail links and the inline first slot they initially point at. */
typedef struct ListHead0013CA40 {
    s32 value;        /* 0x0 */
    s32 count;        /* 0x4 */
    void *head;       /* 0x8 */
    void *tail;       /* 0xC */
    s32 firstSlot[4]; /* 0x10 */
} ListHead0013CA40;
extern ListHead0013CA40 D_0013CAC0;

extern s32 D_0013D100[];

/* func_0011CB58: subsystem reset — DisableDmac(5); func_0011A950(5,
 * D_0013CF54); D_0013469C = 0. Body is structurally identical at 98.46%, but the
 * original allocates $3 (v1) for every %hi address temporary while ee-gcc picks
 * $2 (v0); a one-register global allocation offset this cc1 won't reproduce from
 * source. Left as INCLUDE_ASM. */

extern char *D_0013CFE4; /* 8-byte-stride (key,value) table for negative indices */
extern char *D_0013CFEC; /* 8-byte-stride (key,value) table for indices >= 0 */

extern s32 func_0011CBE8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                         s32 arg5, s32 arg6);

extern void func_0011CB58(void);
extern s32 D_00134720;

/* func_0011D208: allocate the next slot of a circular pool described by arg0
 * (arg0[5]=slot base, arg0[6]=slot count, arg0[9]=counter). index = counter %
 * count; stores counter+1 back; returns &slot[index] (0x40-byte slots). The C
 * body `arg0[5] + ((arg0[9] % arg0[6]) << 6)` reproduces every instruction, but
 * the div-by-zero trap is `break 0,7` in the original and GNU as encodes
 * ee-gcc's `break 7` in the upper code field instead — an assembler-encoding
 * mismatch (2 words), not a source issue. Left as INCLUDE_ASM. */

/* func_0011D590(arg0): link the element arg0->field_0x34 into its manager's
 * (elem->field_0x40) active list — patching the tail's back-link (+0x3C) or the
 * head (+0xC) — then copy a block of transform/state fields from arg0 into the
 * element, and kick processing via func_0011B8D8() when the manager's count
 * (+0x0) is non-negative and busy flag (+0x4) is clear. ~86%: behaviour fully
 * recovered, but ee-gcc won't reproduce the original's branch-likely with an
 * annulled speculative load on the head/tail test. Left as INCLUDE_ASM. */

/* sceSifCheckStatRpc: validity predicate for the handle in arg0 — returns 1 iff
 * arg0[0] points to a live object, arg0[1] matches obj[6] (the +0x18 id/gen),
 * and obj[4] (+0x10) bit 0 is set; else 0. ~65% — ee-gcc collapses the final
 * if/else into `andi v0,v0,1` and inverts the id-check branch (beql) instead of
 * the original two-exit branch shape. Left as INCLUDE_ASM. */

extern s32 func_0011AC20(s32 *desc);
extern s32 D_001347B8;
extern s32 D_001347BC;

/* func_0011D950: look up slot `idx` in the fixed 0x20-entry table D_0013FE80
 * (0x10-byte stride). After the lazy-init (func_0011D868) and acquiring the
 * table lock (func_0011AC60(D_001347B8)), release the lock (func_0011AC40) and
 * return the slot address when idx (unsigned) is in range, else 0. Body
 * `func_0011D868(); func_0011AC60(D_001347B8); if (idx >= 0x20) {
 * func_0011AC40(D_001347B8); return 0; } slot = &D_0013FE80[idx*0x10];
 * func_0011AC40(D_001347B8); return slot;` reaches 99.6% — every instruction
 * lines up except the sltiu range-check lands in $2 here while the original
 * allocates it to $3 (keeping $2 for the table base). A one-register
 * allocation choice this cc1 won't reproduce. Left as INCLUDE_ASM. */

extern s32 func_0011AC60(s32 handle);
extern s32 D_001347B4;

/* func_0011DDC8: tail-call forward of the global handle D_001347B4 to
 * func_0011AC40 (the original is a frameless `j func_0011AC40`). ee-gcc 2.9 does
 * not apply sibling-call optimisation for this shape — it emits a full jal with
 * a stack frame — so it can't match from C. Left as INCLUDE_ASM. */

extern s32 D_001347AC;
extern u8 D_00140128[4];

/* func_0011E740: 0x60 bytes of inter-function padding (`addiu sp,+0xN; nop`
 * filler words) split off by symbol_addrs size:0x60; the real function begins at
 * sceSifInitIopHeap. Pure padding, no C. */

/* sceSifInitIopHeap: real function recovered from the splat mis-split above (init/
 * retry loop around sceSifBindRpc, writes D_001347C4). Boundary now correct;
 * body not yet decompiled. Left as INCLUDE_ASM. */

extern s32 func_0011D620(void *a0, s32 a1, s32 a2, void *a3, s32 a4,
                         void *a5, s32 a6, s32 a7, s32 a8);
extern s32 D_001347C4;
extern s32 D_001401C0;
extern s32 D_00140240;
extern s32 D_00140200;

extern s32 D_001347C8;
extern u8 D_001405A8[4];

extern void func_0011EB00(s32 arg0, s32 arg1, s32 arg2, void *outbuf);

extern s32 func_0011B030(s32 arg0);

/* func_0011F130: dispatch on func_0011B080() (EE syscall 0x7F, current context):
 * if it equals 0x02000000 call func_0011F170, else call func_0011B090. The
 * original keeps one frame and uses jal for both arms (converging at a shared
 * epilogue), but ee-gcc sibling-call-optimises the func_0011F170 arm into a
 * tail `j` and hoists the $ra restore into the bne delay slot — a codegen-shape
 * mismatch not expressible in source. Left as INCLUDE_ASM. */

extern s32 D_00134E38;
extern s32 D_00134E3C;

extern s32 D_0014186C;
extern void func_0011FB98(void);

extern void (*D_00135DB4)(void);

extern s32 func_00115544(const char *a, const char *b);

extern s32 func_00115F28(s32 size);

extern s32 (*D_00135DB8)(void);

extern s32 func_00120498(void);

extern s32 D_00141880;
extern u8 D_00141870[16];
extern u8 D_00141888;

/* Decomposed IEEE-754 double produced by func_00122760: a class tag, sign,
 * unbiased exponent and the explicit mantissa. */
typedef struct {
    s32 fpClass;   /* 0x00: classification tag (3 = normal; zero/subnormal/inf-nan tags not all traced) */
    s32 sign;      /* 0x04: sign bit */
    s32 exponent;  /* 0x08: unbiased exponent */
    s32 pad;       /* 0x0C */
    s64 mantissa;  /* 0x10: explicit mantissa */
} FpParts;

extern void func_00122760(s64 *value, FpParts *out);
extern FpParts *func_00122800(FpParts *a, FpParts *b, FpParts *out);
extern s64 func_00122630(FpParts *parts);

extern s32 func_00122F10(FpParts *a, FpParts *b);

extern void func_001234C0(s32 fpClass, s32 sign, s32 exponent, s32 mantissa);

/* func_00123400: decompose the IEEE-754 single-precision float at src[0] into a
 * classification record at out (out[1]=sign, out[0]=class {0=sNaN,1=qNaN,
 * 2=zero/subnormal,3=normal,4=inf}, out[3]=mantissa, out[2]=unbiased exponent),
 * returning the class. Behaviour fully understood (~68%) but pervasive register
 * allocation differs from the original; left as INCLUDE_ASM. */

extern void func_001232F0(void *args);

extern void func_0011A9A0(s32 id, void *handler, s32 obj);
extern void func_0011AC30(s32 obj);
extern void func_00124540(void);

extern s32 func_00124B88(s32 arg0);
extern s32 func_0011F5E0(void);
extern s32 func_0011F628(void);
extern s32 D_001418C0;

/* func_001248B0: if the callback D_00141844 is installed and the suppression
 * flag D_001363A4 is clear, invoke the callback with the parameter D_00141848.
 * The body compiles byte-exact, but this is a splat mis-split: the per-function
 * .s (and the address-ordered unit listing) start the symbol 8 bytes early on a
 * trailing `addiu $29,$29,0x40; nop` epilogue fragment of the previous function
 * (func_001248F8 even references `func_001248B0 + 0x8` as the real entry). Can't
 * be matched at the unit level without a re-split. Left as INCLUDE_ASM. */

extern s32 D_00137E80;

/**
 * Normalise a handle/id: if its top nibble (bits 31..28) equals 7, clear the
 * upper nibble and set bit 31 instead (i.e. remap tag 0x7 to 0x8). Otherwise
 * return arg0 unchanged.
 */

extern s32 D_00137EB0[];

/**
 * Bounds-checked lookup into the 10-entry table D_00137EB0. Returns
 * D_00137EB0[arg0] for arg0 in [0,9], or 0 if arg0 is out of range.
 */

/* EU note: the USA padding-pin split for func_00127220 has no clean EU mirror —
 * the EU split keeps the inter-function padding fused (asm symbol func_00127218),
 * so the USA func_00127220 body is NOT ported here (left as the EU split emits it).
 * No EU symbol_addrs pin is added (the EU boundary genuinely differs). */

extern u32 func_00126ED8(u32 arg0);
extern void sceDmaSyncChan(void *obj);

struct Obj127220 {
    /* 0x0 */  s32 chcr;
    u8 pad4[0x1C];
    /* 0x20 */ s32 qwc;
    u8 pad24[0xC];
    /* 0x30 */ u32 tadr;
};

extern s32 func_00127508(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern s32 D_00137E68;

extern s32 *D_00141BA8;
extern s32 *D_00141BAC;
extern s32 *D_00141BB0;

extern s32 D_00143188;
/* Array-typed +0x80 EU twin (USA D_00143180): func_00128440 stores arg0 at
 * D_00143200[1] and func_00128250 shares this decl. The array phrasing is
 * required for the func_00128440 match — see the USA unit's match notes. */
extern s32 D_00143200[];
extern char D_0013B8E8[];
extern void func_00128898(const char *fmt, ...);

extern s32 D_00137F00;
/* Sub-object of a resource-table entry (dual instance at +0x0/+0x80 of the
 * object; func_00128DB0 selects between the two by their +0x7C word). */
typedef struct ResSubObj {
    u8 pad[0x7C];
    s32 unk7C;     /* 0x07C — selection key (larger wins) */
} ResSubObj;
/* 16-entry resource table (+0x80 EU twin of USA D_00143640), stride 0x330:
 * +0x4 active flag, +0x8 handle, +0xC object pointer. Struct-typed so
 * func_00128D58's per-field array indexing compiles to the dual-base store. */
typedef struct ResTableEntry {
    s32 unk0;         /* 0x000 */
    s32 active;       /* 0x004 */
    s32 handle;       /* 0x008 */
    ResSubObj *obj;   /* 0x00C */
    u8 pad[0x330 - 16];
} ResTableEntry;
extern ResTableEntry D_001436C0[];

/* func_00128A48: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real function begins at func_00128A50. Pure padding, no C. */

/* func_00128A50: real function recovered from the splat mis-split above (indexes
 * the 0x330-stride table D_001436C0, dispatches to func_00128D58/DB0/E98 + a
 * memcpy). Boundary now correct; body not yet decompiled. Left as INCLUDE_ASM. */

/* func_00128D58 / func_00128DB0 / func_00128E18 now matched below (EU +0x80
 * twins; see the function-adjacent doc comments). */

extern s32 D_00137F90[];

extern s32 func_0012C788(s32 *arg0, s32 arg1);
extern s32 func_0012C878(s32 *arg0, s32 arg1);
extern s32 func_0012CFA0(s32 *arg0);

/* func_0012E890(arg0, arg1, arg2, arg3): initialise the record at arg0 (limit
 * arg1 at field_0x8/0xC, end arg2+arg3 at field_0x24, span arg3 at field_0x28,
 * start arg2 at field_0x20; zero field_0x0..0x4, 0x10, 0x18..0x1C) then
 * tail-call func_0012E8E8(arg0, 0, arg2, arg3). ~75% — ee-gcc schedules the
 * field stores differently around the sibling call (the original interleaves
 * the start-store into the tail-call delay slot), a codegen shape not
 * expressible in source. Left as INCLUDE_ASM. */

extern void func_0012E8E8(u64 *arg0, s32 arg1);

/* func_0012EB28: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real function begins at func_0012EB30. Pure padding, no C. */

/* func_0012EB30: real function recovered from the splat mis-split above (large
 * 0x150-frame routine iterating a 0x18-stride record list and dispatching via an
 * indirect call). Boundary now correct; body not yet decompiled. Left as
 * INCLUDE_ASM. */

extern void func_00130E88(void);

/* func_0012F948: 8 bytes of inter-function padding (a dead `sll $6,$6,4; nop`)
 * between func_0012F940 and the real func_0012F950 — a splat mis-split, pinned
 * to size 0x8 in symbol_addrs so func_0012F950 gets a clean .s. */

/* func_0012F950(obj, arg1, arg2): seed the display/DMA sub-object obj->field_0x40
 * (set fB0=1, fD8=(arg1 & 0x0FFFFFFF) | 0x20000000, fE4=arg2, fDC=fE0=0) then run
 * func_0012FBF0(obj). The body reproduces every field write, but ee-gcc sibling-
 * call-optimises the trailing void call to `j func_0012FBF0` where the original
 * keeps a stack frame (`sd $31`/`jal`) — the inverse-sibling-call form this cc1
 * won't reproduce. Left as INCLUDE_ASM. */

extern void func_00130178(s32 *arg0);
extern void func_00130088(s32 *arg0);

/* func_0012FA70(arg0, index, arg2, arg3): in the entry table at arg0->field_0x40
 * (8-byte stride records), write arg3 into record[index]+0x10, return the old
 * value of record[index]+0xC and overwrite it with arg2. ~74%; the original
 * keeps the table base live and computes both member addresses before storing,
 * a scheduling shape ee-gcc won't reproduce here. Re-probed 2026-06-12 with
 * two separately-formed record pointers (base+idx*8 and (base+0xC)+idx*8) -
 * still 74.44%, the address-formation schedule does not budge. Left as
 * INCLUDE_ASM. */

/* func_0012FA98(arg0, arg1): if arg0 and its table arg0->field_0x40 are non-null,
 * fetch the destructor at table[*arg1*2 + 3] and, if set, call
 * dtor(arg0, arg1, table[*arg1*2 + 4]); return its result or 0. The natural body
 * reaches 92% — but the original keeps `ret` in $7 (a3) where ee-gcc allocates
 * a2, and emits a plain `beqz` on the callback test where ee-gcc picks the
 * branch-likely `beqzl` (annulling its delay slot). Both are scheduling/reg-
 * alloc forms this cc1 won't reproduce. Left as INCLUDE_ASM. */

extern s32 func_0012FA98(s32 *obj, s32 *req);

/* func_0012FE78: dispatch on the state word (offset 0x174) of arg0's sub-object
 * (arg0->field_0x40) — when it equals 3 hand off to func_0012FD60, otherwise to
 * func_0012FEC0; returns the chosen handler's result. Body
 * `if (((s32*)arg0[0x10])[0x5D] != 3) return func_0012FEC0(arg0); return
 * func_0012FD60(arg0);` reaches 97% — every instruction matches but the original
 * parks arg0 in $7 (a3) and the state in $2, while ee-gcc allocates a1/a0; a
 * register-allocation form this cc1 won't reproduce. Left as INCLUDE_ASM. */

extern void func_00130098(s32 *obj);

/* func_00130098(obj): advance/finalise a pending transfer and clear the
 * in-progress flag (field_0x120). If a request is queued (field_0x120 != 0)
 * dispatch via func_00130288(obj, &D_0013BDC8); else by mode field_0x174 finish
 * via func_0012DC50(obj, field_0x1BC, field_0x118 - 1) (mode 3) or
 * func_0012DD60(obj, field_0x1CC, field_0x1DC). ~85% — the original tests the
 * mode with a plain `bne` and hoists `count-1` into its delay slot, but ee-gcc
 * picks the branch-likely `bnel` and fills the slot with the next load. A
 * branch-form/scheduling shape this cc1 won't reproduce. Left as INCLUDE_ASM. */

/* func_00130118: initialise subsystem 1 (func_0012B198(1)), then program the
 * four hardware DMA/GIF register pointers into arg0
 * (field_0x590=0x70000000, 0x594=0x70001800, 0x6D0=0x70001B00, 0x6D4=0x70003300)
 * and clear the busy flag at field_0x810. Body matches 90% — but the original
 * parks the use-once 0x70000000 in the callee-saved $17 (and so reserves a 0x30
 * frame saving $16/$17), whereas ee-gcc at -O2 keeps it in a caller-saved temp
 * and only saves $16 (0x20 frame). A register-allocation form this cc1 won't
 * reproduce. Left as INCLUDE_ASM. */

extern s32 func_00130DB8(s32 mode, s32 arg1);

/* func_00130240(arg0): dispatch arg0 through Kprintf against the global
 * table D_0013BDE8 — the original is a frameless tail call (`j Kprintf`).
 * ee-gcc 2.9 does not sibling-call-optimise this, so it emits jal + a stack
 * frame and cannot match from C. Left as INCLUDE_ASM. */

extern void func_00115DA8(void *buf);
extern void func_00130288(s32 arg0, void *buf);

extern s32 func_0012FA98(s32 *obj, s32 *req);
extern void func_00130240(void *buf);

/* func_001307B0(obj, cmd, madr): restart the GIF/PATH3 DMA pipeline — tear down
 * sub-object 2 (func_0012FA98), flush (func_0012C3B0) and reset the GIF mode
 * register (0x10002000=0); then with interrupts disabled program channel
 * 0x1000B400 (MADR 0x1000B410 = madr & 0x0FFFFFFF, QWC 0x1000B420 = 4, CHCR
 * 0x1000B400 = 0x101), restoring interrupts if on; finally issue IPU command
 * `cmd` (func_0012C380), flush again and tear down sub-object 3. 99.82% — every
 * instruction matches except the frame size: the original reserves a 0x60 frame
 * (saves parked at +0x20..+0x50) where ee-gcc only needs 0x50. A frame-size-only
 * constant mismatch this cc1 won't reproduce. Left as INCLUDE_ASM. */

/* func_00130A50/A60/A70/A80(arg0): frameless tail-call thunks forwarding arg0 to
 * func_00130288 with table D_0013BE58 / D_0013BE88 / D_0013BEA0 / D_0013BED8
 * respectively (original `j func_00130288`). ee-gcc 2.9 won't sibling-call them
 * (emits jal + frame), so they can't match from C. Left as INCLUDE_ASM. */

/* func_00130AA0(arg0): frameless tail call to func_00130C68 with the sub-object
 * at arg0->field_0x40 + 0x4C (original `j func_00130C68`). ee-gcc 2.9 won't
 * sibling-call it (emits jal + frame). Left as INCLUDE_ASM. */

extern void func_00131540(void);
extern s8 D_001381D8[];

/* EU note: the USA padding-pin split for func_00131628 has no clean EU mirror —
 * the EU split keeps the inter-function padding fused (asm symbol func_00131620),
 * so the USA func_00131628 body is NOT ported here. No EU symbol_addrs pin added. */

extern u8 D_00138152;

/* func_001316C8: controller/port status getter — on territory 'T'
 * (func_001315E0()) return the cached byte D_00138156; else sample the pad
 * (func_0011ACD0), return 0 if the 3-bit port field (bits 13..15) is zero, else
 * read extended status (func_0011AF30) and return bit 4 of its low byte. Every
 * instruction matches at 98.85% except the %hi temp register for D_00138156: the
 * original reuses $2 (`lui $2; lbu $2,%lo($2)`) while ee-gcc splits the lui into
 * $3. A reg-alloc form this cc1 won't reproduce. Left as INCLUDE_ASM. */

/* func_00131730: binary byte (0..99) -> packed BCD, n + (n/10)*6 masked to a
 * byte (inverse of func_00131760), e.g. 59 -> 0x59. 99.55% — the ONLY
 * difference is the div-by-zero trap: GNU as encodes ee-gcc's check as
 * `break 7` but the original is `break 0, 7` (different code field). This is an
 * assembler-encoding mismatch (like the move->daddu one), not a source issue.
 * Left as INCLUDE_ASM. */

/* func_00131760: packed-BCD byte -> binary, n - (n>>4)*6 masked to a byte
 * (e.g. 0x59 -> 59). Decompiles to ~87%; the only diff is the multiply form:
 * the original emits 2-operand `mult $0,rs,rt` + `mflo`, but ee-gcc lowers `*`
 * to the 3-operand R5900 `mult rd,rs,rt`. That is a compiler-flag/codegen
 * choice, not expressible in source, so it stays INCLUDE_ASM. */

extern u8 func_00131760(u8 packed);

extern u8 func_00131730(u8 binary);

extern void func_00131850(u8 *arg0);

extern void func_00131908(u8 *arg0);


/**
 * Accessor: return the global pointer/handle D_00133EF4 (the base of the
 * subsystem context block this unit operates on).
 */
s32 func_00115200(void) {
    return D_00133EF4;
}

/**
 * Accessor: return the address of the global D_0013A388.
 */
s32 *func_00115210(void) {
    return &D_0013A388;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115220);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115228);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115250);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", memcmp);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", memcpy);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", memset);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115544);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115690);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001157AC);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001158F4);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115AC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115C90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115CF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115D38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", snd_PrintError);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115DA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115E28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", AssertFail);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115E68);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115E90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115F28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115F78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00115FC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00116300);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00116360);

/**
 * Seed the random-number generator: store arg0 as the RNG state word at
 * D_00133EF4 + 0x58 (the seed consumed by func_001163B0).
 */
void func_001163A0(s32 arg0) {
    *(s32 *)(D_00133EF4 + 0x58) = arg0;
}

/**
 * Linear-congruential RNG. Advances the 32-bit state at D_00133EF4 + 0x58 with
 * the classic glibc constants (state = state*0x41C64E6D + 0x3039) and returns
 * the new state masked to 31 bits (non-negative).
 */
s32 func_001163B0(void) {
    s32 *p = (s32 *)(D_00133EF4 + 0x58);
    s32 v = *p * 0x41C64E6D + 0x3039;
    *p = v;
    return v & 0x7FFFFFFF;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001163E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00116460);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001166C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00116E10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00117108);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00117278);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001175F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00117650);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00117848);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00117CA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001182B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00118460);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001184D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00118548);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001185E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00118BC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00118CC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00118D98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00119AC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00119BC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00119BF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A7F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A810);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A820);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A830);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A840);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A850);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A860);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A870);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A880);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A890);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A8A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A8B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A8C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A8D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A8E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A8F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A900);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A910);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A920);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A930);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A940);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A950);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A960);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A970);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A980);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A990);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A9A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A9B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A9C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A9D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A9E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011A9F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AA00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AA10);

/**
 * func_0011AA20 = EE kernel syscall 0x20 (CreateThread).
 * SCE library stub: load the syscall number into $v1 and trap into the EE
 * kernel; the kernel returns its result in $v0 (no register move emitted, so
 * the C body is the bare inline-asm trap). Kept under the splat func_ name so
 * objdiff pairs it by symbol against the frozen asm. Takes a ThreadParam* and
 * returns the new thread id; the syscall result is left in $v0 by the trap.
 */
s32 func_0011AA20(struct ThreadParam *param) {
    __asm__ volatile("addiu $3, $0, 0x20\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AA30);

/**
 * func_0011AA40 = EE kernel syscall 0x22 (StartThread).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel's result is returned in $v0. Takes the thread id
 * and its start argument.
 */
s32 func_0011AA40(s32 thid, void *arg) {
    __asm__ volatile("addiu $3, $0, 0x22\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AA50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AA60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AA70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AA80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AA90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AAA0);

/**
 * func_0011AAB0 = EE kernel syscall 0x29 (RotateThreadReadyQueue).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel's result is returned in $v0. Called with two
 * arguments by func_0011B800 (thread id + a constant 1).
 */
void func_0011AAB0(s32 thid, s32 arg) {
    __asm__ volatile("addiu $3, $0, 0x29\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AAC0);

/**
 * func_0011AAD0 = EE kernel syscall 0x2B (ReleaseWaitThread).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel releases the given thread from its wait state and
 * returns its result in $v0. Called by func_0011B728 with a thread id.
 */
s32 func_0011AAD0(s32 thid) {
    __asm__ volatile("addiu $3, $0, 0x2B\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AAE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AAF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AB00);

/**
 * func_0011AB10 = EE kernel syscall 0x2F (GetThreadId).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel returns the current thread id in $v0.
 */
s32 func_0011AB10(void) {
    __asm__ volatile("addiu $3, $0, 0x2F\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AB20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AB30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AB40);

/**
 * func_0011AB50 = EE kernel syscall 0x33 (WakeupThread).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel wakes the sleeping thread and returns its result in
 * $v0. Called by func_0011B728 with a thread id.
 */
s32 func_0011AB50(s32 thid) {
    __asm__ volatile("addiu $3, $0, 0x33\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AB60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AB70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AB80);

/**
 * func_0011AB90 = EE kernel syscall 0x37 (SuspendThread).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel suspends the given thread and returns its result in
 * $v0. Called by func_0011B728 with a thread id.
 */
s32 func_0011AB90(s32 thid) {
    __asm__ volatile("addiu $3, $0, 0x37\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ABA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ABB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ABC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ABD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ABE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ABF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AC00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AC10);

/**
 * func_0011AC20 = EE kernel syscall 0x40 (CreateSema).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel creates a semaphore from the descriptor in $a0 and
 * returns the new semaphore id in $v0.
 */
s32 func_0011AC20(s32 *desc) {
    __asm__ volatile("addiu $3, $0, 0x40\n\tsyscall 0" ::: "$3", "memory");
}

/**
 * func_0011AC30 = EE kernel syscall 0x41 (DeleteSema).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel deletes the semaphore identified by $a0.
 */
void func_0011AC30(s32 obj) {
    __asm__ volatile("addiu $3, $0, 0x41\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AC40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AC50);

/**
 * func_0011AC60 = EE kernel syscall 0x44 (WaitSema).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel blocks the caller until the semaphore can be taken
 * and returns the semaphore id in $v0. Used as the table-lock acquire (see
 * func_0011D868 / func_0011AC40 release) and by the worker loop func_0011B728.
 */
s32 func_0011AC60(s32 sema) {
    __asm__ volatile("addiu $3, $0, 0x44\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AC70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AC80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AC90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ACA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ACB0);

/**
 * func_0011ACC0 = EE kernel syscall 0x4A. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Called as a
 * register-write primitive: the $a0 pointer holds the value the kernel writes
 * to a hardware register (see func_0011F8D0). Exact SDK name UNCONFIRMED.
 */
void func_0011ACC0(s32 *in) {
    __asm__ volatile("addiu $3, $0, 0x4A\n\tsyscall 0" ::: "$3", "memory");
}

/**
 * func_0011ACD0 = EE kernel syscall 0x4B. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Called as a
 * register-read primitive: the kernel stores the register value through the
 * $a0 pointer (see func_0011F8D0). Exact SDK name UNCONFIRMED.
 */
void func_0011ACD0(s32 *out) {
    __asm__ volatile("addiu $3, $0, 0x4B\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ACE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ACF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AD00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AD10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AD20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AD30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AD40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AD50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AD60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AD70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AD80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AD90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ADA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ADB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ADC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ADD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ADE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ADF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AE00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AE10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AE20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AE30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AE40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AE50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AE60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AE70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AE80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AE90);

/**
 * func_0011AEA0 = EE kernel syscall 0x64. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap; the kernel's
 * result is returned in $v0. Takes one argument in $a0 (a mode/channel
 * selector — see the DMA channel setup paths func_0011F938/func_0011FAB8).
 * Exact SDK name UNCONFIRMED.
 */
void func_0011AEA0(s32 mode) {
    __asm__ volatile("addiu $3, $0, 0x64\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AEB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AEC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AED0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AEE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AEF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AF00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AF10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AF20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AF30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AF40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AF50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AF60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AF70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AF80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AF90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AFA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AFB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AFC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AFD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AFE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011AFF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B000);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B010);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B020);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B030);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B040);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B050);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B060);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B070);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B080);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B090);

/**
 * Reset the global counter/flag D_00134708 to 0.
 */
void func_0011B0A0(void) {
    D_00134708 = 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B0B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B140);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B1E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B268);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B270);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B320);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B328);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B3D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B450);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B458);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B500);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B580);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B588);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B5F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B658);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B6C0);

extern s32  D_0013C680;  /* semaphore id created for the worker subsystem */
extern char D_0013A9A0[]; /* error format string for an unknown command op */
extern s32  func_0011C7E8(void *fmt, ...); /* printf-style formatter (defined below) */

/* The worker thread's command ring: a `head` write-cursor (masked to 0x1FF on
 * read, advanced modulo 0x200) followed by 512 interleaved {op, arg} byte pairs
 * starting at offset 8. */
struct WorkerQueue {
    s32 head;
    s32 pad;
    u8  cmds[1024];
};

/**
 * func_0011B728 = the unit's background worker-thread main loop (EU twin, same
 * address as the USA build). Blocks forever on the subsystem semaphore
 * D_0013C680 (func_0011AC60/WaitSema); each time it is signalled it pops the next
 * command record from the 512-entry ring buffer at *queue (the `head` cursor
 * masked to 0x1FF, then the 2-byte {op,arg} record at cmds[idx*2]) and dispatches
 * on the op byte: 0 -> func_0011AB50/WakeupThread, 1 -> func_0011AAD0/
 * ReleaseWaitThread, 2 -> func_0011AB90/SuspendThread (each on the record's
 * thread-id arg byte); any other op prints the error string D_0013A9A0 via
 * func_0011C7E8. Never returns.
 *
 * The op[] and arg[] views are hoisted as two loop-invariant base pointers
 * (queue+8 / queue+9) indexed by the record offset idx*2, matching ee-gcc's
 * register allocation; the switch cases are ordered 0,1,2 to reproduce the
 * original case-block layout.
 */
void func_0011B728(struct WorkerQueue *queue) {
    u8 *op = (u8 *)queue + 8;
    u8 *arg = (u8 *)queue + 9;
    for (;;) {
        s32 idx, slot;
        func_0011AC60(D_0013C680);
        idx = queue->head & 0x1FF;
        queue->head = idx + 1;
        slot = idx * 2;
        switch (op[slot]) {
        case 0:
            func_0011AB50(arg[slot]);
            break;
        case 1:
            func_0011AAD0(arg[slot]);
            break;
        case 2:
            func_0011AB90(arg[slot]);
            break;
        default:
            func_0011C7E8(D_0013A9A0);
            break;
        }
    }
}

extern s32 D_00134710;   /* worker-thread id / init guard (<=0 until created) */
extern s32 D_0013C688[2];/* StartThread argument block (two words, zeroed) */
extern u8  D_0013C280[]; /* the worker thread's stack buffer */
extern s32 D_001AF070;   /* the gp base value handed to the worker thread */

/**
 * Bring up the unit's background worker thread once. Guards on D_00134710 (>0
 * means already up): creates a semaphore (func_0011AC20 = CreateSema) with
 * maxCount 0xFF, then a thread (func_0011AA20 = CreateThread) entered at
 * func_0011B728 with a 0x400-byte stack and the engine gp. On success it caches
 * the thread id in D_00134710, starts it (func_0011AA40 = StartThread) with a
 * zeroed two-word argument block, and yields the ready queue
 * (func_0011AAB0 = RotateThreadReadyQueue) for the current thread
 * (func_0011AB10 = GetThreadId). On any failure it returns -1, tearing the
 * semaphore back down (func_0011AC30 = DeleteSema) if the thread could not be
 * created. Returns the worker thread id (D_00134710) on success.
 */
s32 func_0011B800(void) {
    struct ThreadParam thread;
    struct SemaParam sema;
    s32 thid;

    if (D_00134710 > 0) {
        return -1;
    }
    sema.maxCount = 0xFF;
    sema.initCount = 0;
    D_0013C680 = func_0011AC20((s32 *)&sema);
    if (D_0013C680 < 0) {
        return -1;
    }
    thread.entry = (void *)func_0011B728;
    thread.stack = D_0013C280;
    thread.stackSize = 0x400;
    thread.gpReg = &D_001AF070;
    thread.initPriority = 0;
    thid = func_0011AA20(&thread);
    D_00134710 = thid;
    if (thid < 0) {
        func_0011AC30(D_0013C680);
        return -1;
    }
    D_0013C688[0] = 0;
    D_0013C688[1] = 0;
    func_0011AA40(thid, D_0013C688);
    func_0011AAB0(func_0011AB10(), 1);
    return D_00134710;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B8D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B970);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B978);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011B9C0);

/**
 * Pack arg0 and the signed-byte form of arg1 into a stack record and push it
 * through func_0011B050 with count 3.
 */
void func_0011B9C8(s32 arg0, s32 arg1) {
    s32 args[2];
    args[0] = arg0;
    args[1] = (s8)arg1;
    func_0011B050(3, args);
}

/**
 * Push the single 32-bit value arg0 through func_0011B050 with count 4
 * (the value is passed by address in a local).
 */
void func_0011B9F8(s32 arg0) {
    s32 value = arg0;
    func_0011B050(4, &value);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011BA20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011BA58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011BA90);

/**
 * Push the single 32-bit value arg0 through func_0011B050 with count 0x10
 * (the value is passed by address in a local).
 */
void func_0011BAA0(s32 arg0) {
    s32 value = arg0;
    func_0011B050(0x10, &value);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011BAC4);

/**
 * Initialise the global list head D_0013CAC0: store `value`, clear the entry
 * count, and point both head and tail links at the inline first slot (+0x10);
 * returns &D_0013CAC0. (The volatile stores pin the original head/count/tail
 * store order, which the scheduler would otherwise batch — this is the
 * volatile-pinning technique that cracked the old ~98.5% wall.)
 */
s32 *func_0011BAC8(s32 value) {
    s32 *base = (s32 *)&D_0013CAC0;
    D_0013CAC0.value = value;
    *(volatile s32 *)(base + 2) = (s32)(base + 4);
    *(volatile s32 *)(base + 1) = 0;
    *(volatile s32 *)(base + 3) = (s32)(base + 4);
    return base;
}

/**
 * Advance the write cursor of the ring buffer at arg0. Bumps the entry count
 * (+0x4) and the cursor (+0xC) by one; when the cursor reaches the end of the
 * inline storage (base + capacity(+0x0) + 0x10) it wraps back to the start of
 * that storage (base + 0x10).
 */
void func_0011BAF0(s32 *arg0) {
    s32 cursor;
    s32 end;
    arg0[1] = arg0[1] + 1;
    cursor = arg0[3] + 1;
    end = arg0[0] + 0x10;
    arg0[3] = cursor;
    if (cursor == (s32)arg0 + end) {
        arg0[3] = (s32)arg0 + 0x10;
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011BB38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011BCD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011BE20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011BEDC);

/**
 * Spin until the busy bit (0x8000) of the status register at 0x1000F130 clears,
 * then write the low byte of `value` to the command register at 0x1000F180.
 * Returns `value`.
 */
s32 func_0011BEE0(s32 value) {
    while (*(volatile u32 *)0x1000F130 & 0x8000) {
    }
    *(volatile u8 *)0x1000F180 = value;
    return value;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011BF18);

/**
 * Send byte `ch` to the output port via func_0011BEE0, translating a bare LF
 * ('\n', 0x0A) into a CR ('\r', 0x0D) followed by the LF.
 */
void func_0011BFC8(s32 ch) {
    if (ch == '\n') {
        func_0011BEE0('\r');
        func_0011BEE0('\n');
    } else {
        func_0011BEE0(ch);
    }
}

/**
 * func_0011C000 = convert the IEEE-754 double whose raw bits are `bits` into a
 * clamped integer in [0, 9999]. Extracts the 11-bit exponent field and rebiases
 * it to exp = field - 0x433 (the power-of-two that scales the 53-bit
 * significand, including the implicit leading 1). Values below 2^-53 round to 0;
 * values needing >= 2^13 saturate to 9999 (0x270F). Otherwise the significand is
 * shifted left by exp (exp >= 0) or right by (-exp - 2) with a round-up when the
 * two dropped low bits are both set, and the low 32 bits are returned.
 *
 * NEAR-MISS WALL (95.56% via objdiff, not byte-exact) - see the USA twin's
 * comment in src/usa/cod/015180.c for the functionally-correct C and the two
 * residual ee-gcc codegen diffs (exp<0 register threading + the original's
 * branch-LIKELY `bnel` rounding compare that ee-gcc emits as a plain `bne`).
 * Seedable leaf: shipped as a cmp-oracle'd portable #else below (asm-vs-C proven
 * bit-identical on real R5900 by run_cmp_015180_iso.sh).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011C000);
#else
s32 func_0011C000(s64 bits) {
    s64 x = bits;
    s64 exp = (s64)(((u64)(x << 1)) >> 53) - 0x433;
    if (exp < -0x35) {
        return 0;
    }
    if (exp >= 0xD) {
        return 0x270F;
    }
    x = (s64)((((u64)x) << 12) >> 12 | 0x10000000000000ULL);
    if (exp < 0) {
        u64 v = ((u64)x) >> (s32)(-exp - 2);
        x = (s64)((v & 3) == 3 ? (v >> 2) + 1 : v >> 2);
    } else {
        x = x << (s32)exp;
    }
    return (s32)x;
}
#endif

/**
 * func_0011C090 = print the double whose bits are `value` in scientific notation
 * via func_0011C7E8 (EU twin of the USA function, same address). Emits a leading
 * '-' (through the char hook D_00134718) for negatives, normalises the magnitude
 * into [0.1, 1.0) tracking a decimal exponent (scale up by 10 / func_00122B00 when
 * < 0.1, down by 10 / func_00122DA8 when >= 1.0), scales the mantissa by 1e6
 * (D_0013AA68), truncates (func_001212C8) and clamps (func_0011C000), then prints
 * "0.dddd" (D_0013AA40) followed by "e+NN" (D_0013AA48) or "e-NN" (D_0013AA50).
 *
 * MATCHING WALL (FP-constant-pool / li.d): see the USA twin's note. The matching
 * arm stays INCLUDE_ASM (byte-exact); the faithful body references the recovered
 * pool-double bit patterns (0.1 = 0x3FB999999999999A, 1e6 = 0x412E848000000000)
 * and the cheap inline 1.0/10.0. EU data symbols are the +0x80-shifted twins of
 * USA's. (A magnitude of exactly 0.0 would spin the scale-up loop forever, so the
 * caller never passes 0.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011C090);
#else
extern s32 func_00123028(s64 a, s64 b);
extern s64 func_00122A98(s64 a, s64 b);
extern s64 func_00122B00(s64 a, s64 b);
extern s64 func_00122DA8(s64 a, s64 b);
extern s64 func_001212C8(s64 x);
extern s32 func_0011C000(s64 bits);
extern s32 func_0011C7E8(void *fmt, ...);
extern void (*D_00134718)(s32 ch);         /* single-character output hook */
extern char D_0013AA40[];                  /* "0.%d" */
extern char D_0013AA48[];                  /* "e+%d" */
extern char D_0013AA50[];                  /* "e%d"  */

#define DBL_0_1 0x3FB999999999999ALL       /* 0.1  (D_0013AA58 / D_0013AA60) */
#define DBL_1E6 0x412E848000000000LL       /* 1e6  (D_0013AA68)              */
#define DBL_10  0x4024000000000000LL       /* 10.0 */
#define DBL_1   0x3FF0000000000000LL       /* 1.0  */

s32 func_0011C090(s64 value) {
    s32 exp = 0;
    s64 scaled;
    s32 digits;

    if (func_00123028(value, 0) < 0) {
        value = func_00122A98(0, value);
        D_00134718('-');
    }
    if (func_00123028(value, DBL_0_1) < 0) {
        do {
            value = func_00122B00(value, DBL_10);
            exp--;
        } while (func_00123028(value, DBL_0_1) < 0);
    } else {
        while (func_00123028(value, DBL_1) >= 0) {
            value = func_00122DA8(value, DBL_10);
            exp++;
        }
    }
    scaled = func_00122B00(value, DBL_1E6);
    digits = func_0011C000(func_001212C8(scaled));
    func_0011C7E8(D_0013AA40, digits);
    if (exp < 0) {
        return func_0011C7E8(D_0013AA50, exp);
    }
    return func_0011C7E8(D_0013AA48, exp);
}
#endif

/**
 * func_0011C1F8 = the Kprintf/vfprintf CORE (formatted-output engine). EU twin of
 * the USA function at the same address (cod/015180 is region-co-located, delta +0);
 * see the USA unit for the full recovered structure. Walks `fmt`, emits each byte
 * via (*D_00134698)(int), and on '%' parses a zero-pad width + length modifier
 * ('l'/'h') then dispatches o/x/d/u/e/f/s/c via jtbl_0013A9F0; integers render
 * right-to-left into a 32-byte stack buffer (NUL at sp+0x1F). MATCHING WALL: a
 * faithful reconstruction rebuilds to ~85.7% byte-identical (same frame/saves/jump
 * table); the residual is an unsteerable ee-gcc 2.9 char-load-scheduling + temp-
 * allocation tie-break (lbu-reload vs move, a2 vs a3), not behaviour. The MATCHING
 * arm stays INCLUDE_ASM in both regions; the portable #else (below) is the correct
 * printf, cmp-oracle'd asm-vs-C bit-identical on the real R5900 in the USA region
 * (region-co-located code; the only EU delta is the +0x80-shifted char hook
 * D_00134718). The %e/%f path composes onto func_0011C090's own #else.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011C1F8);
#else
extern s32 func_0011F5E0(void);
extern s32 func_0011F628(void);
extern s64 func_001234F0(float f);
extern s64 __moddi3(s64 a, s64 b);   /* __moddi3  signed mod   */
extern s64 __divdi3(s64 a, s64 b);   /* __divdi3  signed div   */
extern u64 __umoddi3(u64 u, u64 v);   /* __umoddi3 unsigned mod */
extern u64 __udivdi3(u64 u, u64 v);   /* __udivdi3 unsigned div */

s32 func_0011C1F8(char *fmt, s64 *ap) {
    char buf[32];
    s64 n;
    char *s = fmt;
    char *p;
    char *q;
    char *pad;
    s32 lenmod;
    s32 c;
    s32 saved = func_0011F5E0();

    if (*s == 0) goto Lend;

    for (;;) {
        pad = 0;
        lenmod = 0;
        c = (s8)*s;
        if (c != '%') goto Lputc;
        p = s + 1;
    Lscan:
        s = p;
        {
            s32 idx = (s8)((u8)*s - 0x30);
            if ((u32)idx >= 0x49) { p = s + 1; goto Lskip; }
            switch (idx) {
            case '0' - '0': { /* zero-pad field width */
                s32 w;
                s32 d1 = *(p + 1) - 0x30;
                if ((u8)d1 < 0xA) {
                    s32 d2 = *(p + 2) - 0x30;
                    if ((u32)d2 < 0xA) {
                        w = d1 * 10 + d2;
                        if (w >= 0x20) w = 0x1F;
                        s = p + 2;
                    } else {
                        w = d1;
                        s = p + 1;
                    }
                    if (w > 0) {
                        pad = &buf[0x1F] - w;
                        p = s + 1;
                        do {
                            buf[0x1F - w] = '0';
                            w--;
                        } while (w > 0);
                        s = p;
                        goto Lscan;
                    }
                    p = s + 1;
                    goto Lscan;
                }
                p++;
                goto Lscan;
            }
            case 'l' - '0':
                lenmod = 0x6C;
                p++;
                goto Lscan;
            case 'h' - '0':
                lenmod = 0x68;
                p++;
                goto Lscan;
            case 'o' - '0':
                if (lenmod == 0x6C) { ap++; n = *(ap - 1); }
                else if (lenmod == 0x68) { ap++; n = *(u16 *)(ap - 1); }
                else { ap++; n = *(u32 *)(ap - 1); }
                q = &buf[0x1F];
                buf[0x1F] = 0;
                if (n == 0) {
                    q = &buf[0x1E];
                    buf[0x1E] = '0';
                    p++;
                } else {
                    p++;
                    do {
                        *--q = (char)((n & 7) + 0x30);
                        n = (u64)n >> 3;
                    } while (n != 0);
                }
                if (pad != 0 && pad < q) q = pad;
                if (*q == 0) goto Lskip;
                do { D_00134718(*q); q++; } while (*q != 0);
                s = p;
                goto Lcont;
            case 'x' - '0':
                if (lenmod == 0x6C) { ap++; n = *(ap - 1); }
                else if (lenmod == 0x68) { ap++; n = *(u16 *)(ap - 1); }
                else { ap++; n = *(u32 *)(ap - 1); }
                q = &buf[0x1F];
                buf[0x1F] = 0;
                if (n == 0) {
                    q = &buf[0x1E];
                    buf[0x1E] = '0';
                    p++;
                } else {
                    p++;
                    do {
                        u64 nib = n & 0xF;
                        *--q = (char)(nib < 0xA ? nib + 0x30 : nib + 0x57);
                        n = (u64)n >> 4;
                    } while (n != 0);
                }
                if (pad != 0 && pad < q) q = pad;
                if (*q == 0) goto Lskip;
                do { D_00134718(*q); q++; } while (*q != 0);
                s = p;
                goto Lcont;
            case 'd' - '0':
            {
                s64 dn;
                if (lenmod == 0x6C) { ap++; dn = *(ap - 1); }
                else if (lenmod == 0x68) { ap++; dn = *(s16 *)(ap - 1); }
                else { ap++; dn = *(s32 *)(ap - 1); }
                q = &buf[0x1F];
                buf[0x1F] = 0;
                if (dn == 0) {
                    q = &buf[0x1E];
                    buf[0x1E] = '0';
                    p++;
                } else {
                    if (dn < 0) {
                        D_00134718('-');
                        dn = -dn;
                    }
                    p++;
                    while (dn != 0) {
                        *--q = (char)(__moddi3(dn, 10) + 0x30);
                        dn = __divdi3(dn, 10);
                    }
                }
                if (pad != 0 && pad < q) q = pad;
                if (*q == 0) goto Lskip;
                do { D_00134718(*q); q++; } while (*q != 0);
                s = p;
                goto Lcont;
            }
            case 'u' - '0':
                if (lenmod == 0x6C) { ap++; n = *(ap - 1); }
                else if (lenmod == 0x68) { ap++; n = *(u16 *)(ap - 1); }
                else { ap++; n = *(u32 *)(ap - 1); }
                q = &buf[0x1F];
                buf[0x1F] = 0;
                if (n == 0) {
                    q = &buf[0x1E];
                    buf[0x1E] = '0';
                    p++;
                } else {
                    p++;
                    do {
                        *--q = (char)(__umoddi3(n, 10) + 0x30);
                        n = __udivdi3(n, 10);
                    } while (n != 0);
                }
                if (pad != 0 && pad < q) q = pad;
                if (*q == 0) goto Lskip;
                do { D_00134718(*q); q++; } while (*q != 0);
                s = p;
                goto Lcont;
            case 'e' - '0':
            case 'f' - '0': {
                float f;
                ap++;
                f = *(float *)(ap - 1);
                if (f == 0.0f) {
                    D_00134718('0');
                    p++;
                } else {
                    p++;
                    func_0011C090(func_001234F0(f));
                }
                s = p;
                goto Lcont;
            }
            case 's' - '0': {
                char *str;
                ap++;
                str = (char *)*(s32 *)(ap - 1);
                if (*str == 0) {
                    D_00134718('(');
                    p++;
                    D_00134718('n');
                    D_00134718('u');
                    D_00134718('l');
                    D_00134718('l');
                    D_00134718(')');
                    s = p;
                    goto Lcont;
                }
                q = str;
                p++;
                do {
                    D_00134718(*q);
                    q++;
                } while (*q != 0);
                s = p;
                goto Lcont;
            }
            case 'c' - '0':
            {
                s64 cn;
                ap++;
                cn = *(char *)(ap - 1);
                D_00134718((s32)cn);
                p++;
                s = p;
                goto Lcont;
            }
            default:
                p++;
                goto Lskip;
            }
        }

    Lputc:
        D_00134718(c);
        p = s + 1;
        s = p;
        goto Lcont;

    Lskip:
        s = p;
    Lcont:
        if (*s == 0) break;
    }

Lend:
    if (saved != 0) {
        return func_0011F628();
    }
    return 0;
}
#endif

/**
 * func_0011C7E8 = printf-style wrapper around the core formatter func_0011C1F8.
 * Spills its variadic register arguments ($a1..$a7) to the stack home area and
 * forwards (dest, va_list) to func_0011C1F8, returning its result. `dest` is the
 * sink passed straight through; the va_list points at the first variadic arg.
 */
extern s32 func_0011C1F8(char *fmt, s64 *ap);

s32 func_0011C7E8(void *dest, ...) {
    /* EABI single-float va_start (va_list == char*): point past the named arg
     * into the spilled variadic register-save area. */
    char *ap = (char *)__builtin_next_arg(dest)
               - (__builtin_args_info(2) >= 8
                      ? 0
                      : (8 - __builtin_args_info(2)) * 8);
    return func_0011C1F8((char *)dest, (s64 *)ap);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011C820);

/**
 * Callback that writes a record's value (arg0[5]) into the array at arg1[7]
 * (arg1->field_0x1C), indexed by the record's key/index arg0[4]:
 * ((s32*)arg1[7])[arg0[4]] = arg0[5].
 */
void func_0011C880(s32 *arg0, s32 *arg1) {
    s32 *base = (s32 *)arg1[7];
    base[arg0[4]] = arg0[5];
}

/**
 * Callback that copies the index/key field arg0[4] into arg1[2]
 * (arg1->field_0x8).
 */
void func_0011C8A0(s32 *arg0, s32 *arg1) {
    arg1[2] = arg0[4];
}

/**
 * Lookup into the global table D_0013D100: return D_0013D100[arg0]
 * (no bounds checking).
 */
s32 func_0011C8B0(s32 arg0) {
    return D_0013D100[arg0];
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011C8C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011C8D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011CB58);

/**
 * Store a (key,value) pair into the sign-selected table pair: entry `index`
 * of D_0013CFEC for index >= 0, of D_0013CFE4 for index < 0 (the negative
 * index reaches backwards from that table's base); key at slot+0x0, value at
 * slot+0x4. (Reusing `index` for the loaded table pointer keeps it in $a0
 * like the original, the pre-computed `addr` rides the bgez delay slot, and
 * the volatile stores pin the original value-then-key order.)
 */
void func_0011CB90(s32 index, s32 key, s32 value) {
    s32 addr = index * 8;
    if (index < 0) {
        index = (s32)D_0013CFE4;
    } else {
        index = (s32)D_0013CFEC;
    }
    addr += index;
    ((volatile s32 *)addr)[1] = value;
    ((volatile s32 *)addr)[0] = key;
}

/**
 * Clear the key word (slot+0x0) of entry `index` in the sign-selected table
 * pair (same addressing and register-reuse shape as func_0011CB90).
 */
void func_0011CBC0(s32 index) {
    s32 addr = index * 8;
    if (index < 0) {
        index = (s32)D_0013CFE4;
    } else {
        index = (s32)D_0013CFEC;
    }
    addr += index;
    *(s32 *)addr = 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011CBE8);

/**
 * Thin wrapper around func_0011CBE8 that forces its second argument (the mode
 * flag) to 0 and shifts the caller's arg1..arg5 into arg2..arg6.
 */
s32 func_0011CD20(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    return func_0011CBE8(arg0, 0, arg1, arg2, arg3, arg4, arg5);
}

/**
 * Thin wrapper around func_0011CBE8 that forces its second argument (the mode
 * flag) to 1 and shifts the caller's arg1..arg5 into arg2..arg6.
 */
s32 func_0011CD60(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    return func_0011CBE8(arg0, 1, arg1, arg2, arg3, arg4, arg5);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011CDA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011CEC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011CF74);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011CF78);

/**
 * Reset helper: run the subsystem reset routine func_0011CB58(), then clear the
 * global state word D_00134720 to 0.
 */
void func_0011D118(void) {
    func_0011CB58();
    D_00134720 = 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011D140);

/**
 * Reset object arg0: clear its field_0x18 (arg0[6]) and clear bit 0 of the flag
 * word field_0x10 (arg0[4]) — i.e. mark it inactive/idle.
 */
void func_0011D1E8(s32 *arg0) {
    arg0[6] = 0;
    arg0[4] &= 0xFFFFFFFE;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011D208);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011D238);

extern s32 *func_0011D208(s32 idx);

/**
 * Enqueue a command against the ring slot for `idx`: allocate the slot via
 * func_0011D208, copy the descriptor fields obj[5]/obj[7] into it (explicit
 * temps), stamp the command word 0x8000000C at slot[8], then void-tail-call
 * func_0011CD60(0x80000008, slot, 0x40, obj[8], obj[9], obj[10]) — six plain
 * EABI register args, sibcall-optimised to the original's `j` (lever 7).
 */
void func_0011D2F0(s32 *obj, s32 idx) {
    s32 *slot = func_0011D208(idx);
    s32 a = obj[5];
    s32 b = obj[7];

    slot[5] = a;
    slot[7] = b;
    slot[8] = 0x8000000C;
    func_0011CD60(0x80000008, (s32)slot, 0x40, obj[8], obj[9], obj[10]);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011D350);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", rename);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011D450);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011D590);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011D620);

/**
 * Validity predicate for the RPC handle in arg0 (EU twin of USA
 * sceSifCheckStatRpc; splat hasn't named it here): returns 1 iff arg0[0]
 * points to a live object, arg0[1] matches obj[6] (the +0x18 id/gen), and
 * obj[4] (+0x10) bit 0 is set; else 0. The explicit gotos keep the original's
 * two-exit shape (lever 10).
 */
s32 func_0011D810(s32 *arg0) {
    s32 *obj = (s32 *)*arg0;
    if (obj == 0) goto ret0;
    if (arg0[1] != obj[6]) goto ret0;
    if ((obj[4] & 1) != 0) goto ret1;
ret0:
    return 0;
ret1:
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011D850);

/**
 * Lazily create the two paired handles D_001347B8 / D_001347BC (sentinel -1 =
 * uninitialised on the first): build a small descriptor on the stack (fields 1
 * and 2 set, field 5 cleared), then create both handles from it via
 * func_0011AC20. A no-op once D_001347B8 exists.
 */
void func_0011D868(void) {
    if (D_001347B8 == -1) {
        s32 desc[8];
        desc[5] = 0;
        desc[2] = 1;
        desc[1] = 1;
        D_001347B8 = func_0011AC20(desc);
        D_001347BC = func_0011AC20(desc);
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011D8C8);

extern void func_0011AC40(s32 sema);
extern s32 D_001347B8;
extern u8 D_0013FF00[];

/**
 * Look up slot `idx` in the fixed 0x20-entry table D_0013FF00 (0x10-byte
 * stride; EU +0x80 twin of USA D_0013FE80). After the lazy-init
 * (func_0011D868) and acquiring the table lock (func_0011AC60(D_001347B8)),
 * release the lock (func_0011AC40) and return the slot address when idx
 * (unsigned) is in range, else 0. The explicit `goto` keeps the in-range
 * block as the branch TARGET (lever 10) — the one-register choice for match.
 */
void *func_0011D950(u32 idx) {
    void *slot;
    func_0011D868();
    func_0011AC60(D_001347B8);
    if (idx < 0x20) goto hit;
    func_0011AC40(D_001347B8);
    return 0;
hit:
    slot = &D_0013FF00[idx * 0x10];
    func_0011AC40(D_001347B8);
    return slot;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011D9C0);

/**
 * Lazily create the singleton handle D_001347B4 (sentinel -1 = uninitialised):
 * build a small descriptor on the stack (fields 1 and 2 set, field 5 cleared),
 * hand it to func_0011AC20 and cache the resulting handle. A no-op once created.
 */
void func_0011DD48(void) {
    if (D_001347B4 == -1) {
        s32 desc[8];
        desc[5] = 0;
        desc[2] = 1;
        desc[1] = 1;
        D_001347B4 = func_0011AC20(desc);
    }
}

/**
 * Run the func_0011DD48 teardown step, then forward the global handle
 * D_001347B4 to func_0011AC60. Always returns 0.
 */
s32 func_0011DD98(void) {
    func_0011DD48();
    func_0011AC60(D_001347B4);
    return 0;
}

extern void func_0011AC40(s32 sema);
extern s32 D_001347B4;

/**
 * Release the singleton table lock: forward the global semaphore handle
 * D_001347B4 (EU +0x80 twin of USA D_00134734) to func_0011AC40 (SignalSema).
 * A void tail call, sibling-call-optimised to the original's frameless
 * `j func_0011AC40` (the handle load rides the jump's delay slot).
 */
void func_0011DDC8(void) {
    func_0011AC40(D_001347B4);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011DDD8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011DE08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011E010);

/**
 * Reset the subsystem state guarded by D_001347AC: clear the flag word to 0 and
 * zero the 4-byte descriptor at D_00140128. Always returns 0.
 */
s32 func_0011E0A0(void) {
    D_001347AC = 0;
    memset(D_00140128, 0, 4);
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011E0D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011E360);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011E4E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011E740);

/**
 * Register a request with the D_00140200 service: bail out returning 0 if the
 * service slot D_001347C4 is inactive (negative); otherwise stash the request
 * parameters (arg1, arg0, arg2) into the D_00140240 descriptor and submit it via
 * func_0011D620. Returns the resulting handle D_00140200 on success, 0 on
 * failure.
 */
s32 func_0011E828(s32 arg0, s32 arg1, s32 arg2) {
    if (D_001347C4 < 0) {
        return 0;
    }
    (&D_00140240)[0] = arg1;
    (&D_00140240)[1] = arg0;
    (&D_00140240)[2] = arg2;
    if (func_0011D620(&D_001401C0, 4, 0, &D_00140240, 0xC,
                      &D_00140200, 4, 0, 0) >= 0) {
        return D_00140200;
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011E8A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011E920);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011E938);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011EA38);

/**
 * Reset the subsystem state guarded by D_001347C8: set the flag word to -1
 * (uninitialised sentinel) and zero the 4-byte descriptor at D_001405A8.
 * Always returns 0.
 */
s32 func_0011EAC8(void) {
    D_001347C8 = -1;
    memset(D_001405A8, 0, 4);
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011EB00);

/**
 * Forward (arg0, arg1, arg2) to func_0011EB00, supplying a 16-byte scratch
 * buffer on the stack as its fourth (output) argument.
 */
void func_0011ED08(s32 arg0, s32 arg1, s32 arg2) {
    u8 buf[16];
    func_0011EB00(arg0, arg1, arg2, buf);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ED28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011ED60);

/**
 * Query controller/pad state bit 0x40000 (via func_0011B030(4)); if set, run the
 * func_0011B0A0 handler and return 1, otherwise return 0.
 */
s32 func_0011EEA0(void) {
    if (func_0011B030(4) & 0x40000) {
        func_0011B0A0();
        return 1;
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011EED8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011EFE8);

/**
 * func_0011EFF0 = EE kernel syscall 0x5A. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Installs a DMA/INTC
 * handler over a buffer; called from func_0011F058 with a 3-word argument
 * (handler addr, buffer, length). Same primitive as func_0011F878. Exact SDK
 * name UNCONFIRMED.
 */
s32 func_0011EFF0(s32 a, s32 b, s32 c) {
    __asm__ volatile("addiu $3, $0, 0x5A\n\tsyscall 0" ::: "$3", "memory");
}

/**
 * Copy nbytes>>2 words (32-bit) from src to dst and return 0. nbytes is rounded
 * down to a whole number of words; a zero word-count copies nothing.
 */
s32 func_0011F000(s32 *dst, s32 *src, u32 nbytes) {
    u32 words = nbytes >> 2;
    u32 i;
    for (i = 0; i < words; i++) {
        *dst = *src;
        src++;
        dst++;
    }
    return 0;
}

/**
 * func_0011F038 = EE kernel syscall 0x5B. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap; result returned in
 * $v0. Takes one argument in $a0 (a channel id — see func_0011F058). Same
 * primitive as func_0011F8C0. Exact SDK name UNCONFIRMED.
 */
s32 func_0011F038(s32 a) {
    __asm__ volatile("addiu $3, $0, 0x5B\n\tsyscall 0" ::: "$3", "memory");
}

/**
 * func_0011F048 = EE kernel syscall 0x74. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Used during DMA
 * channel setup (see func_0011F058) with a 2-word argument; callers ignore the
 * result, so this is modelled as void. Same primitive as func_0011F868. Exact
 * SDK name UNCONFIRMED.
 */
void func_0011F048(s32 a, s32 b) {
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
}

/* A single DMA channel descriptor in the static init table D_00134B50: a
 * (channel-id, mode) word pair consumed by the syscall stubs. Same layout as the
 * D_001355E8 / D_00135D70 tables used by func_0011F938 / func_0011FAB8. */
typedef struct DmaChannelInit {
    s32 channel;
    s32 mode;
} DmaChannelInit;

extern DmaChannelInit D_00134B50[8];
extern u8 D_001347D0;
extern s32 D_00134B48;

/**
 * func_0011F058: bring up the third DMA-channel group (the one _InitSys finishes
 * with its tail call). Arms channel entry[0] (func_0011F048 = syscall 0x74),
 * installs the 0x80075000 handler over D_001347D0 spanning 0x330 bytes
 * (func_0011EFF0 = syscall 0x5A), toggles the interrupt-enable syscall
 * (func_0011AEA0 = 0x64) off then on, arms entries[1] and [2] directly, then for
 * the remaining entries (3..7) queries each channel (func_0011F038 = syscall
 * 0x5B) and re-arms it with the returned value. Finally stores func_0011F038(3)
 * into D_00134B48. Unlike func_0011F938 / func_0011FAB8 this group has no
 * hardware gate. Exact SDK name UNCONFIRMED.
 */
void func_0011F058(void) {
    u32 i;
    func_0011F048(D_00134B50[0].channel, D_00134B50[0].mode);
    func_0011EFF0(0x80075000, (s32)&D_001347D0, 0x330);
    func_0011AEA0(0);
    func_0011AEA0(2);
    func_0011F048(D_00134B50[1].channel, D_00134B50[1].mode);
    func_0011F048(D_00134B50[2].channel, D_00134B50[2].mode);
    for (i = 3; i < 8; i++) {
        func_0011F048(D_00134B50[i].channel, func_0011F038(D_00134B50[i].channel));
    }
    D_00134B48 = func_0011F038(3);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011F120);

extern s32 func_0011B080(void);
extern s32 func_0011B090(void);
extern s32 func_0011F170(void);

/**
 * Dispatch on func_0011B080() (EE syscall 0x7F, current context): if it equals
 * 0x02000000 run func_0011F170, else func_0011B090; return the arm's result.
 * RETURNING the call result keeps both arms as framed `jal`s converging at the
 * shared epilogue (lever 7).
 */
s32 func_0011F130(void) {
    if (func_0011B080() == 0x02000000) {
        return func_0011F170();
    }
    return func_0011B090();
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011F170);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011F364);

/**
 * func_0011F5E0 = disable EE interrupts, reporting the prior enable state.
 * Reads COP0 Status, isolates the EIE bit (0x10000); if interrupts were off it
 * returns 0 immediately. Otherwise it executes the handwritten `di` (disable)
 * then `sync.p`, re-reading Status until the EIE bit clears (the EE pipeline can
 * leave it set for a cycle), and returns non-zero. The di/sync.p ARE the
 * operation, so they are matched with inline asm. Pairs with func_0011F628.
 */
s32 func_0011F5E0(void) {
    s32 status, cur;
    __asm__ volatile("mfc0 %0, $12" : "=r"(status));
    status &= 0x10000;
    if (status != 0) {
        do {
            __asm__ volatile("di");
            __asm__ volatile("sync.p");
            __asm__ volatile("mfc0 %0, $12" : "=r"(cur));
            cur &= 0x10000;
        } while (cur != 0);
    }
    return status != 0;
}

/**
 * func_0011F628 = re-enable EE interrupts, reporting the prior enable state.
 * Reads COP0 Status, isolates the EIE bit (0x10000), executes the handwritten
 * `ei` instruction to enable interrupts, and returns non-zero iff interrupts
 * were already enabled. The `ei` is the operation itself, so it is matched with
 * inline asm rather than modelled. Pairs with func_0011F5E0 (suspend).
 */
s32 func_0011F628(void) {
    s32 status;
    __asm__ volatile(
        "mfc0 %0, $12\n\t"
        "lui  $3, 0x1\n\t"
        "and  %0, %0, $3\n\t"
        "ei"
        : "=r"(status) :: "$3", "memory");
    return status != 0;
}

/**
 * Create the paired handles D_00134E38 / D_00134E3C from two identical
 * descriptors (each with fields 1 and 2 set to 1) via func_0011AC20.
 */
void func_0011F640(void) {
    s32 desc1[8];
    s32 desc2[8];
    desc1[1] = 1;
    desc1[2] = 1;
    desc2[1] = 1;
    desc2[2] = 1;
    D_00134E38 = func_0011AC20(desc1);
    D_00134E3C = func_0011AC20(desc2);
}

/**
 * Copy nbytes>>2 words (32-bit) from src to dst and return 0. Identical body to
 * func_0011F000 (a duplicated word-copy helper).
 */
s32 func_0011F688(s32 *dst, s32 *src, u32 nbytes) {
    u32 words = nbytes >> 2;
    u32 i;
    for (i = 0; i < words; i++) {
        *dst = *src;
        src++;
        dst++;
    }
    return 0;
}

/**
 * Linear find: return the first pointer in [p,last) whose word equals value,
 * else 0. The single combined `&&` loop condition makes ee-gcc peel the first
 * iteration and emit the deref-then-range branch-likely pair + movz tail-merge
 * (levers 5+6).
 */
s32 *func_0011F6C0(s32 *p, s32 *last, s32 value) {
    while (*p != value && p < last) {
        p++;
    }
    return (p < last) ? p : 0;
}

/**
 * func_0011F700 = EE kernel syscall 0x83. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Called from the
 * device/handler init path (func_0011F718) with a 3-word argument. Exact SDK
 * name UNCONFIRMED.
 */
s32 func_0011F700(s32 a, s32 b, s32 c) {
    __asm__ volatile("addiu $3, $0, 0x83\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011F710);

extern s32 D_00134E28[4]; /* handler table: two {arg0,arg1} pairs */
extern s32 D_00134E20;    /* cached end-of-walk pointer (func_0011F6C0 side) */
extern s32 *func_0011F6C0(s32 *first, s32 *last, s32 value);

/**
 * Install the unit's exception/interrupt handlers and walk the two parallel
 * handler regions to their common end.
 *
 * First registers two handlers via func_0011F818 (syscall 0x74) from the table
 * D_00134E28 (a pair of {arg0,arg1} entries). Then it seeds two cursors with
 * func_0011F700 (syscall 0x83) over the 0x80000000..0x80080000 range, one
 * keyed on func_0011F6C0 and one on func_0011F688, and advances whichever
 * cursor (offset back by its handler's fixed bias, 0x20C / 0x168) is behind
 * until the two biased cursors meet. The meeting point is cached in D_00134E20.
 */
void func_0011F718(void) {
    s32 p;
    s32 q;
    s32 a;
    s32 b;

    func_0011F818(D_00134E28[0], D_00134E28[1]);
    func_0011F818(D_00134E28[2], D_00134E28[3]);
    p = func_0011F700(0x80000000, 0x80080000, (s32)func_0011F6C0);
    q = func_0011F700(0x80000000, 0x80080000, (s32)func_0011F688);
    a = p - 0x20C;
    b = q - 0x168;
    while (a != b) {
        if ((u32)a < (u32)b) {
            p = func_0011F700(p + 4, 0x80080000, (s32)func_0011F6C0);
            a = p - 0x20C;
        } else {
            q = func_0011F700(q + 4, 0x80080000, (s32)func_0011F688);
            b = q - 0x168;
        }
    }
    D_00134E20 = a;
}

/**
 * func_0011F818 = EE kernel syscall 0x74 (same primitive as func_0011F868).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap. Called from the device/handler init path (func_0011F718) with
 * a 2-word argument. Exact SDK name UNCONFIRMED.
 */
s32 func_0011F818(s32 a, s32 b) {
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
}

extern void func_0011F058(void);
extern void func_0011F938(void);
extern void func_0011FAB8(void);

/**
 * func_0011F828 = _InitSys (USA 0x0011F828): EE crt0 runtime bring-up, called
 * once from _start before main. Runs the unit's init sequence in fixed order —
 * thread/exception scaffolding (func_0011F640), device/handler init
 * (func_0011F718), the GS/DMA reset path (func_0011FAB8), the background worker
 * thread (func_0011B800) and the first DMA-channel group (func_0011F938) — then
 * tail-calls func_0011F058 to finish.
 */
void func_0011F828(void) {
    func_0011F640();
    func_0011F718();
    func_0011FAB8();
    func_0011B800();
    func_0011F938();
    func_0011F058();
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011F864);

/**
 * func_0011F868 = EE kernel syscall 0x74. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Used during DMA
 * channel setup (see func_0011F938) with a 2-word argument; callers ignore the
 * result, so this is modelled as void. Exact SDK name UNCONFIRMED.
 */
void func_0011F868(s32 a, s32 b) {
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
}

/**
 * func_0011F878 = EE kernel syscall 0x5A. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Used during DMA
 * channel setup (see func_0011F938) with a 3-word argument. Exact SDK name
 * UNCONFIRMED.
 */
s32 func_0011F878(s32 a, s32 b, s32 c) {
    __asm__ volatile("addiu $3, $0, 0x5A\n\tsyscall 0" ::: "$3", "memory");
}

/**
 * Copy nbytes>>2 words (32-bit) from src to dst and return 0. Identical body to
 * func_0011F000 (a duplicated word-copy helper).
 */
s32 func_0011F888(s32 *dst, s32 *src, u32 nbytes) {
    u32 words = nbytes >> 2;
    u32 i;
    for (i = 0; i < words; i++) {
        *dst = *src;
        src++;
        dst++;
    }
    return 0;
}

/**
 * func_0011F8C0 = EE kernel syscall 0x5B. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap; result returned
 * in $v0. Takes one argument in $a0 (a channel id — see func_0011F938).
 * Exact SDK name UNCONFIRMED.
 */
s32 func_0011F8C0(s32 a) {
    __asm__ volatile("addiu $3, $0, 0x5B\n\tsyscall 0" ::: "$3", "memory");
}

/**
 * Read a hardware register pair, force its mode field to 0x2000 (clearing the
 * 0x...E000 bits), write it back, then re-read it; returns 1 if the resulting
 * 3-bit field at bits 13..15 is zero, else 0.
 */
s32 func_0011F8D0(void) {
    s32 regs[2];
    func_0011ACD0(&regs[0]);
    regs[1] = (regs[0] & 0xFFFF1FFF) | 0x2000;
    func_0011ACC0(&regs[1]);
    func_0011ACD0(&regs[1]);
    func_0011ACC0(&regs[0]);
    return (((u32)regs[1] >> 13) & 0x7) < 1;
}

/* D_001355E8 / D_00135D70 are the (channel-id, mode) init tables for the first
 * two DMA-channel groups; see DmaChannelInit above (defined for func_0011F058). */
extern DmaChannelInit D_001355E8[3];
extern u8 D_00134E40;

/**
 * func_0011F938: bring up the first DMA-channel group. Gated on func_0011F8D0
 * (only runs when the hardware mode field reads back clean). Arms channel
 * entry[0] (func_0011F868 = syscall 0x74), installs the 0x80074000 handler over
 * D_00134E40 spanning 0x7A8 bytes (func_0011F878 = syscall 0x5A), toggles the
 * interrupt-enable syscall (func_0011AEA0 = 0x64) off then on, arms entry[1],
 * then for the remaining entries (index 2) queries each channel
 * (func_0011F8C0 = syscall 0x5B) and re-arms it with the returned value.
 * Exact SDK name UNCONFIRMED.
 */
void func_0011F938(void) {
    u32 i;
    if (func_0011F8D0()) {
        func_0011F868(D_001355E8[0].channel, D_001355E8[0].mode);
        func_0011F878(0x80074000, (s32)&D_00134E40, 0x7A8);
        func_0011AEA0(0);
        func_0011AEA0(2);
        func_0011F868(D_001355E8[1].channel, D_001355E8[1].mode);
        for (i = 2; i < 3; i++) {
            func_0011F868(D_001355E8[i].channel,
                          func_0011F8C0(D_001355E8[i].channel));
        }
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011F9E4);

/**
 * Shutdown hook thunk: void tail call to the func_0011F130 dispatcher,
 * sibling-call-optimised to the original's `j func_0011F130`.
 */
void func_0011FA18(void) {
    func_0011F130();
}

extern void func_0011A840(s32 code);

/**
 * libc exit() (EU twin of USA `exit`; splat hasn't named it here): run the
 * func_0011FA18 shutdown hook, then hand `code` to the terminator
 * func_0011A840 — a void tail call, sibcall-optimised to the original's
 * `j func_0011A840` (lever 7, framed form; `code` rides callee-saved $16).
 */
void func_0011FA20(s32 code) {
    func_0011FA18();
    func_0011A840(code);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011FA48);

/**
 * func_0011FA50 = EE kernel syscall 0x74 (same primitive as func_0011F868).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap. Called from the GS/DMA reset path (func_0011FAB8) with a
 * 2-word argument; callers ignore the result, so this is modelled as void.
 * Exact SDK name UNCONFIRMED.
 */
void func_0011FA50(s32 a, s32 b) {
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
}

/**
 * func_0011FA60 = EE kernel syscall 0x5A (same primitive as func_0011F878).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap. Called from the GS/DMA reset path (func_0011FAB8) with a
 * 3-word argument. Exact SDK name UNCONFIRMED.
 */
s32 func_0011FA60(s32 a, s32 b, s32 c) {
    __asm__ volatile("addiu $3, $0, 0x5A\n\tsyscall 0" ::: "$3", "memory");
}

/**
 * Copy nbytes>>2 words (32-bit) from src to dst and return 0. Identical body to
 * func_0011F000 (a duplicated word-copy helper).
 */
s32 func_0011FA70(s32 *dst, s32 *src, u32 nbytes) {
    u32 words = nbytes >> 2;
    u32 i;
    for (i = 0; i < words; i++) {
        *dst = *src;
        src++;
        dst++;
    }
    return 0;
}

/**
 * func_0011FAA8 = EE kernel syscall 0x5B (same primitive as func_0011F8C0).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; result returned in $v0. Takes one argument in $a0 (a channel
 * id). Called in a loop from the GS/DMA reset path (func_0011FAB8). Exact SDK
 * name UNCONFIRMED.
 */
s32 func_0011FAA8(s32 a) {
    __asm__ volatile("addiu $3, $0, 0x5B\n\tsyscall 0" ::: "$3", "memory");
}

extern DmaChannelInit D_00135D70[8];
extern u8 D_00135608;
extern u8 D_00135D48;

/**
 * func_0011FAB8: bring up the second (8-entry) DMA-channel group, used by the
 * GS/DMA reset path. Skips entirely when timer/DMAC register 0x10001810 has
 * bit 0x100 set (work already in progress). Otherwise arms entry[0]
 * (func_0011FA50 = syscall 0x74), installs two handlers via func_0011FA60
 * (syscall 0x5A) — 0x80076000 over D_00135608 (0x740 bytes) and 0x82000 over
 * D_00135D48 (0x28 bytes) — toggles the interrupt-enable syscall
 * (func_0011AEA0 = 0x64) off then on, arms entry[1], then for entries 2..7
 * queries each channel (func_0011FAA8 = syscall 0x5B) and re-arms it with the
 * returned value. Exact SDK name UNCONFIRMED.
 */
void func_0011FAB8(void) {
    u32 i;
    if ((*(volatile s32 *)0x10001810 & 0x100) == 0) {
        func_0011FA50(D_00135D70[0].channel, D_00135D70[0].mode);
        func_0011FA60(0x80076000, (s32)&D_00135608, 0x740);
        func_0011FA60(0x82000, (s32)&D_00135D48, 0x28);
        func_0011AEA0(0);
        func_0011AEA0(2);
        func_0011FA50(D_00135D70[1].channel, D_00135D70[1].mode);
        for (i = 2; i < 8; i++) {
            func_0011FA50(D_00135D70[i].channel,
                          func_0011FAA8(D_00135D70[i].channel));
        }
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0011FB8C);

/**
 * One-shot initialiser: the first time it is called (guard word D_0014186C is
 * still zero) it sets the guard and tail-calls func_0011FB98 to do the real
 * setup; subsequent calls do nothing.
 */
void func_0011FC48(void) {
    if (D_0014186C == 0) {
        D_0014186C = 1;
        func_0011FB98();
    }
}

/**
 * __divdi3 = libgcc `__divdi3` (signed 64-bit division, a / b); EU twin of
 * the USA function at the same address (region-co-located, delta +0). ee-gcc
 * inlines libgcc2.c's signed wrapper around `__udivmoddi4` (sign-strip via
 * bgez/negu, unsigned long-division core driven by `__clz_tab` D_0013AC58 and
 * 16-bit-digit `udiv_qrnnd` with `divu`/`break 0,7`, then re-sign the quotient).
 * Compiler runtime, NOT game code; not reconstructable as clean hand C that
 * matches byte-exact, so the MATCHING arm stays INCLUDE_ASM (links verbatim). The
 * portable #else is the faithful behaviour (`a / b`), cmp-oracle'd bit-identical
 * on the real R5900. Excluded domains (`break 0,7` / overflow): b == 0 and
 * INT64_MIN / -1. See the USA unit for the full analysis. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", __divdi3);
#else
s64 __divdi3(s64 a, s64 b) {
    return a / b;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00120354);

/**
 * Invoke the installed callback held in the global function pointer D_00135DB4.
 */
void func_00120368(void) {
    D_00135DB4();
}

/**
 * Compare the two strings arg0 and arg1 with func_00115544 (strcmp); return
 * arg2 when they are equal, otherwise 0.
 */
s32 func_00120390(const char *arg0, const char *arg1, s32 arg2) {
    s32 result = arg2;
    if (func_00115544(arg0, arg1) != 0) {
        result = 0;
    }
    return result;
}

/* No-op stub (empty body; present as a registered/overridable hook). */
void func_001203C0(void) {
}

/**
 * Allocate and zero-init a 0x18-byte record via func_00115F28 (OOM hook
 * func_00120368 on failure); field [1] is set to point at the record's own
 * tail (p+0x10), forming an empty self-referential list head. Returns the record.
 */
s32 *func_001203C8(void) {
    s32 *p = (s32 *)func_00115F28(0x18);
    if (p == 0) {
        func_00120368();
    }
    memset(p, 0, 0x18);
    p[1] = (s32)(p + 4);
    return p;
}

/**
 * Return the value produced by the installed callback D_00135DB8 (a base
 * value/pointer queried by the +4 / +8 variants below).
 */
s32 func_00120420(void) {
    return D_00135DB8();
}

/**
 * Return D_00135DB8() + 8 (the base value from the callback, offset by 8 bytes).
 */
s32 func_00120448(void) {
    return D_00135DB8() + 8;
}

/**
 * Install func_00120498 as the active callback D_00135DB8 and invoke it once
 * (priming its lazily-initialised state).
 */
void func_00120470(void) {
    D_00135DB8 = func_00120498;
    D_00135DB8();
}

/**
 * Lazily initialise and return the 16-byte singleton at D_00141870. On first
 * call (guarded by the flag D_00141880) the block is zeroed and its field at
 * offset 4 is pointed at D_00141888. Always returns the block's address.
 */
s32 func_00120498(void) {
    if (!D_00141880) {
        D_00141880 = 1;
        memset(D_00141870, 0, 0x10);
        *(u8 **)(D_00141870 + 4) = &D_00141888;
    }
    return (s32)D_00141870;
}

/**
 * Return D_00135DB8() + 4 (the base value from the callback, offset by 4 bytes).
 */
s32 func_00120500(void) {
    return D_00135DB8() + 4;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00120528);

/**
 * Accessor: return the s16 at arg0 + 0x6 (arg0[3]).
 */
s16 func_00120800(s16 *arg0) {
    return arg0[3];
}

/**
 * Accessor: return the s16 at arg0 + 0x4 (arg0[2]).
 */
s16 func_00120808(s16 *arg0) {
    return arg0[2];
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00120810);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001208E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00120A30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00120AB8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00120B38);

/* No-op stub (empty body; present as a registered/overridable hook). */
void func_00120BC8(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00120BD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00120F00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001210E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001212C4);

/**
 * func_001212C8 = truncate the non-negative double whose bits are `x` to a
 * 64-bit integer, i.e. soft-float __fixunsdfdi (negative x -> 0). Splits into a
 * high 32-bit limb hi = trunc(x * 2^-32) and a low residual: builds hi<<32 back
 * into a double (func_001213B8 of the limb, or of the halved limb then doubled
 * via func_00122A40 when the top bit is set), subtracts it from x
 * (func_00122A98), truncates the residual to a 32-bit limb (func_001231C8) and
 * adds it onto hi<<32 (subtracting when the residual went slightly negative).
 *
 * NEAR-MATCH WALL (region-co-located with USA). Functionally exact but loses
 * byte-equality the same way as its soft-float siblings (callee-saved 0.0 const,
 * inlined unsigned-64->double branch layout). Seedable: now that its multiply
 * dependency func_00122B00 ships a #else, it ships a faithful portable
 * TARGET_NATIVE #else, cmp-oracle'd bit-identical on the real R5900. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001212C8);
#else
extern s32 func_00123028(s64 a, s64 b);
extern u32 func_001231C8(s64 a);
extern s64 func_001213B8(s64 value);
extern s64 func_00122A40(s64 a, s64 b);
extern s64 func_00122A98(s64 a, s64 b);
extern s64 func_00122B00(s64 a, s64 b);

s64 func_001212C8(s64 x) {
    s32 hi, lo;
    s64 hiShift, hiD, lowD, result;

    if (func_00123028(x, 0) < 0) {       /* x < 0.0 -> 0 */
        return 0;
    }
    /* high limb hi = trunc(x * 2^-32); 0x3DF0000000000000 == 2^-32 */
    hi = func_001231C8(func_00122B00(x, 0x3DF0000000000000LL));
    hiShift = (s64)hi << 32;
    if (hiShift < 0) {                   /* hi bit31 set: halve then double to dodge */
        hiD = func_001213B8((s64)((u64)hiShift >> 1));   /* the floatdidf sign overflow */
        hiD = func_00122A40(hiD, hiD);
    } else {
        hiD = func_001213B8(hiShift);    /* hiD = (double)(hi << 32) */
    }
    lowD = func_00122A98(x, hiD);        /* residual = x - hi*2^32 */
    if (func_00123028(lowD, 0) >= 0) {
        lo = func_001231C8(lowD);
        result = hiShift + (s64)(u64)(u32)lo;
    } else {                             /* residual went slightly negative */
        lo = func_001231C8(func_00122A98(0, lowD));
        result = hiShift - (s64)(u64)(u32)lo;
    }
    return result;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001213B4);

extern s64 func_00123078(s32 x);
extern s64 func_00122B00(s64 a, s64 b);
extern s64 func_00122A40(s64 a, s64 b);

/**
 * func_001213B8 = convert a 64-bit signed integer to a double (packed bits),
 * i.e. soft-float __floatdidf. The high 32 bits are converted as a signed int
 * and scaled by 2^32 (two 65536.0 multiplies); the low 32 bits are converted as
 * a signed int and, when negative, biased by +2^32 so they contribute as an
 * unsigned 32-bit limb. result = (double)hi * 2^32 + (unsigned)lo.
 *
 * NEAR-MISS WALL (85.92% via objdiff, region-co-located with USA). Functionally
 * faithful; the only divergence is ee-gcc -O2 -G0 rematerialising the 65536.0
 * multiplier constant before each func_00122B00 call rather than keeping it in a
 * callee-saved register across both (a reload policy choice). Shipped as a
 * portable TARGET_NATIVE #else; verification routed to tester-EE (calls sibling
 * soft-float #else bodies).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001213B8);
#else
s64 func_001213B8(s64 value) {
    s64 scale = 0x40F0000000000000LL;
    s32 hi = (s32)(value >> 32);
    s64 hiDouble = func_00122B00(func_00122B00(func_00123078(hi), scale), scale);
    s32 lo = (s32)(value & 0xFFFFFFFFLL);
    s64 loDouble = func_00123078(lo);
    if (lo < 0) {
        loDouble = func_00122A40(loDouble, 0x41F0000000000000LL);
    }
    return func_00122A40(loDouble, hiDouble);
}
#endif

/**
 * __moddi3 = libgcc `__moddi3` (signed 64-bit modulo, a % b); EU twin of the
 * USA function at the same address (region-co-located, delta +0). ee-gcc inlines
 * libgcc2.c's signed wrapper around `__udivmoddi4` (sign-strip, unsigned core via
 * `__clz_tab` D_0013AD58 + `udiv_qrnnd`/`break 0,7`, remainder takes the dividend's
 * sign). Compiler runtime, NOT game code; the MATCHING arm stays INCLUDE_ASM
 * (links verbatim). The portable #else is the faithful behaviour (`a % b`),
 * cmp-oracle'd bit-identical on the real R5900. Excluded domains: b == 0 and
 * INT64_MIN / -1. See the USA unit for the full analysis. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", __moddi3);
#else
s64 __moddi3(s64 a, s64 b) {
    return a % b;
}
#endif

/**
 * __muldi3 = 64-bit integer multiply (low 64 bits), a*b (libgcc __muldi3).
 * NEAR-MISS WALL (72.29% via objdiff, region-co-located with USA): correct
 * instruction set, but ee-gcc -O2 -G0 differs in half-product register
 * allocation and materialises the 0xFFFFFFFF mask via `dli` vs the original
 * `lui;dsrl32`. Standalone-seedable (pure a*b) -> HARD-GATE cmp-oracle candidate,
 * routed to tester-EE for the real-R5900 run.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", __muldi3);
#else
s64 __muldi3(s64 a, s64 b) {
    union { struct { s32 low; s32 high; } s; s64 ll; } w, uu, vv;
    uu.ll = a;
    vv.ll = b;
    w.ll = (s64)((u64)(u32)uu.s.low * (u32)vv.s.low);
    w.s.high += uu.s.low * vv.s.high + uu.s.high * vv.s.low;
    return w.ll;
}
#endif

/**
 * Frameless tail-call thunk: forward to func_00120368 (which dispatches the
 * installed handler D_00135DB4). Takes and returns nothing. The original is a
 * bare `j func_00120368`; ee-gcc 2.9 reproduces the sibling call because both
 * the thunk and target are void(void) leaves with no argument/return shuffle.
 */
void func_00121B18(void) {
    func_00120368();
}

/**
 * __udivdi3 = libgcc `__udivdi3` (unsigned 64-bit division, u / v); EU twin of
 * the USA function at the same address (region-co-located, delta +0). Inlines
 * libgcc2.c's `__udivmoddi4` long division directly (no sign handling — straight
 * to the sltu/divu unsigned core via `__clz_tab` D_0013AE58 + `udiv_qrnnd`/
 * `break 0,7`); the quotient is returned, the remainder discarded. Compiler
 * runtime, NOT game code; the MATCHING arm stays INCLUDE_ASM (links verbatim). The
 * portable #else is the faithful behaviour (`u / v`), cmp-oracle'd bit-identical on
 * the real R5900. Excluded domain: v == 0. Paired: __umoddi3 = `__umoddi3`. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", __udivdi3);
#else
u64 __udivdi3(u64 u, u64 v) {
    return u / v;
}
#endif

/**
 * __umoddi3 = libgcc `__umoddi3` (unsigned 64-bit modulo, u % v); EU twin of
 * the USA function at the same address (region-co-located, delta +0). Inlines
 * libgcc2.c's `__udivmoddi4` long division (count_leading_zeros via the 256-byte
 * `__clz_tab` D_0013AF58, then 16-bit-digit `udiv_qrnnd` with `divu`/`break 0,7`).
 * Compiler runtime, NOT game code and NOT a format sub-engine - not reconstructable
 * as clean hand C that matches byte-exact; links verbatim. Paired: __udivdi3 =
 * `__udivdi3`. See the USA unit for the full analysis. The MATCHING arm stays
 * INCLUDE_ASM in both regions; the portable #else is the faithful behaviour
 * (`u % v`), cmp-oracle'd bit-identical on the real R5900. Excluded domain: v == 0.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", __umoddi3);
#else
u64 __umoddi3(u64 u, u64 v) {
    return u % v;
}
#endif

/**
 * func_00122630 = recompose an FpParts descriptor into a packed IEEE-754 double
 * (NaN->quiet 0x7FF, inf/zero clamps, normal: rebias +0x3FF, round-to-even on
 * the low 8 mantissa bits, pack sign|exp|mantissa). NEAR-MISS WALL (59.12%) -
 * see the USA twin for the uninitialized-scratch masking quirk. Shipped as a
 * cmp-oracle'd portable #else (run_cmp_015180_iso.sh).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00122630);
#else
s64 func_00122630(FpParts *p) {
    s32 cls = p->fpClass;
    s32 sign = p->sign;
    s64 mant = p->mantissa;
    s32 expOut = 0;
    if (cls < 2) {
        mant |= 0x0008000000000000ULL;
        expOut = 0x7FF;
    } else if (cls == 4) {
        expOut = 0x7FF;
        mant = 0;
    } else if (cls == 2) {
        mant = 0;
    } else if (mant != 0) {
        s32 exp = p->exponent;
        if (exp < -0x3FE) {
            s32 sh = -0x3FE - exp;
            if (sh < 0x39) {
                mant = (s64)((u64)mant >> sh);
            } else {
                mant = 0;
            }
            mant = (s64)((u64)mant >> 8);
        } else if (exp >= 0x400) {
            expOut = 0x7FF;
            mant = 0;
        } else {
            expOut = exp + 0x3FF;
            if ((mant & 0xFF) == 0x80) {
                if (mant & 0x100) {
                    mant += 0x80;
                }
            } else {
                mant += 0x7F;
            }
            if ((u64)mant > 0x1FFFFFFFFFFFFFFFULL) {
                mant = (s64)((u64)mant >> 1);
                expOut++;
            }
            mant = (s64)((u64)mant >> 8);
        }
    }
    return ((s64)sign << 63) | ((s64)(expOut & 0x7FF) << 52) |
           (mant & 0x000FFFFFFFFFFFFFLL);
}
#endif

/**
 * func_00122760 = decompose the IEEE-754 double *value into FpParts at out
 * (sign, 11-bit exponent field, 52-bit fraction; class 2 zero/subnormal, 4 inf,
 * 1/0 quiet/signalling NaN, 3 normal with mantissa = (frac<<8)|(1<<60) and
 * exponent rebiased by -0x3FF). NEAR-MISS WALL (93.54%) - see the USA twin for
 * the sign/exp register-swap + constant-preload scheduling diffs ee-gcc won't
 * reproduce. Shipped as a cmp-oracle'd portable #else (run_cmp_015180_iso.sh).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00122760);
#else
void func_00122760(s64 *value, FpParts *out) {
    u64 v = *(u64 *)value;
    s32 sign = (s32)(v >> 63);
    s64 frac = v & 0x000FFFFFFFFFFFFFULL;
    s32 exp = (s32)(v >> 52) & 0x7FF;
    out->sign = sign;
    if (exp == 0) {
        out->fpClass = 2;
        return;
    }
    if (exp == 0x7FF) {
        if (frac == 0) {
            out->fpClass = 4;
            return;
        }
        if (frac & 0x0008000000000000ULL) {
            out->fpClass = 1;
        } else {
            out->fpClass = 0;
        }
        out->mantissa = frac;
        return;
    }
    out->mantissa = (frac << 8) | 0x1000000000000000ULL;
    out->exponent = exp - 0x3FF;
    out->fpClass = 3;
}
#endif

/* The soft-float canonical-NaN descriptor (EU; = USA D_00141810, +0x80-shifted).
 * Recovered from the ROM image at vaddr 0x00141890: a 0x28-byte block that is
 * ALL ZEROES, i.e. a zero-filled FpParts {class 0, sign 0, exp 0, mantissa 0}.
 * The add/multiply/divide cores return &D_00141890 on their invalid-operation
 * result paths; func_00122630 recomposes it to the canonical quiet NaN
 * 0x7FF8000000000000. Declared here so the portable #else cores can take its
 * address (defined in the unit's data). */
extern FpParts D_00141890;

/* func_00122800 = software double-precision ADD of two decomposed operands
 * (FpParts a + b -> out). NaN propagates; inf+inf opposite sign -> NaN constant
 * &D_00141890; zero/inf shortcuts copy the surviving operand; otherwise align +
 * add (like signs) or subtract (unlike signs), renormalise, write class 3.
 *
 * NEAR-MISS WALL (region-co-located with USA; ~144 instrs, two sticky-shift
 * loops + branch-likely fillers). Seedable: ships a faithful portable
 * TARGET_NATIVE #else, cmp-oracle'd bit-identical on the real R5900 - see the
 * USA twin for the full behavioural description. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00122800);
#else
FpParts *func_00122800(FpParts *a, FpParts *b, FpParts *out) {
    s32 clsA, clsB;
    s32 expA, expB, expR;
    s32 signA, signB;
    s64 mantA, mantB;
    s32 d, ad;

    clsA = a->fpClass;
    if (clsA < 2) {                      /* a is NaN -> propagate a */
        return a;
    }
    clsB = b->fpClass;
    if (clsB < 2) {                      /* b is NaN -> propagate b */
        return b;
    }
    if (clsA == 4) {                     /* a is inf */
        if (clsB != 4) {
            return a;                    /* inf + finite = inf */
        }
        if (a->sign == b->sign) {
            return a;                    /* inf + inf (same sign) = inf */
        }
        return &D_00141890;              /* inf + (-inf) = NaN */
    }
    if (clsB == 4) {                     /* finite + inf = inf */
        return b;
    }
    if (clsB == 2) {                     /* b is zero */
        if (clsA != 2) {
            return a;                    /* normal + 0 = a */
        }
        *out = *a;                       /* 0 + 0 = zero, sign = signA & signB */
        out->sign = a->sign & b->sign;
        return out;
    }
    if (clsA == 2) {                     /* 0 + normal = b */
        return b;
    }

    /* both normal: align the smaller mantissa, then add or subtract. */
    expA = a->exponent;
    expB = b->exponent;
    mantA = a->mantissa;
    mantB = b->mantissa;
    signA = a->sign;
    signB = b->sign;
    d = expA - expB;
    ad = (d >= 0) ? d : -d;
    if (ad < 64) {
        if (expA > expB) {
            while (expB < expA) {
                mantB = (s64)(((u64)mantB >> 1) | ((u64)mantB & 1));
                expB++;
            }
            expR = expA;
        } else if (expA < expB) {
            s32 cnt = expB - expA;
            while (cnt != 0) {
                mantA = (s64)(((u64)mantA >> 1) | ((u64)mantA & 1));
                cnt--;
            }
            expR = expB;
        } else {
            expR = expA;
        }
    } else {                             /* exponent gap >= 64: drop the smaller */
        if (expA > expB) {
            mantB = 0;
            expR = expA;
        } else {
            mantA = 0;
            expR = expB;
        }
    }

    if (signA == signB) {                /* like signs: add */
        out->sign = signA;
        out->exponent = expR;
        out->mantissa = mantA + mantB;
    } else {                             /* unlike signs: subtract smaller */
        s64 diff = (signA != 0) ? (mantB - mantA) : (mantA - mantB);
        out->exponent = expR;
        if (diff < 0) {
            out->mantissa = -diff;
            out->sign = 1;
        } else {
            out->mantissa = diff;
            out->sign = 0;
        }
        {                                /* renormalise up on cancellation */
            s64 m = out->mantissa;
            s32 e = out->exponent;
            while ((u64)(m - 1) < 0x0FFFFFFFFFFFFFFFULL) {
                m <<= 1;
                e--;
                out->mantissa = m;
                out->exponent = e;
            }
        }
    }

    out->fpClass = 3;
    {                                    /* one-step renormalise down on carry */
        s64 m = out->mantissa;
        if ((u64)m > 0x1FFFFFFFFFFFFFFFULL) {
            out->mantissa = (s64)(((u64)m >> 1) | ((u64)m & 1));
            out->exponent += 1;
        }
    }
    return out;
}
#endif

/**
 * Software double-precision binary op: decompose both operands into their
 * IEEE-754 parts (func_00122760), combine them with func_00122800 into a result
 * descriptor, then recompose that into a packed double via func_00122630.
 */
s64 func_00122A40(s64 a, s64 b) {
    s64 va = a;
    s64 vb = b;
    FpParts pa;
    FpParts pb;
    FpParts result;
    func_00122760(&va, &pa);
    func_00122760(&vb, &pb);
    return func_00122630(func_00122800(&pa, &pb, &result));
}

/**
 * Software double-precision subtraction: decompose both operands, flip the sign
 * of the second, then add (func_00122800) and recompose (func_00122630), i.e.
 * compute a + (-b).
 */
s64 func_00122A98(s64 a, s64 b) {
    s64 va = a;
    s64 vb = b;
    FpParts pa;
    FpParts pb;
    FpParts result;
    func_00122760(&va, &pa);
    func_00122760(&vb, &pb);
    pb.sign ^= 1;
    return func_00122630(func_00122800(&pa, &pb, &result));
}

/* func_00122B00 = software double-precision MULTIPLY of two packed doubles
 * (a * b -> packed double). NaN propagates with the product sign; 0*inf -> NaN
 * constant &D_00141890; otherwise (both normal) the 122-bit product of the two
 * 61-bit mantissas is built from four 32x32 partial products (__muldi3),
 * normalised and round-to-nearest-even.
 *
 * NEAR-MISS WALL (region-co-located with USA). Seedable: ships a faithful
 * portable TARGET_NATIVE #else, cmp-oracle'd bit-identical on the real R5900
 * (incl. the 0*inf NaN path reading D_00141890). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00122B00);
#else
extern s64 __muldi3(s64 a, s64 b);

s64 func_00122B00(s64 a, s64 b) {
    FpParts pa, pb, result;
    s64 va = a, vb = b;
    s32 clsA, clsB, comb;
    u64 aLo, aHi, bLo, bHi, p0, p1, p2, p3, mid, midCarry, low64, lowCarry, high64;
    s32 exp;

    func_00122760(&va, &pa);
    func_00122760(&vb, &pb);
    comb = (pa.sign ^ pb.sign) != 0;

    clsA = pa.fpClass;
    if (clsA < 2) {                      /* a is NaN -> propagate a, product sign */
        pa.sign = comb;
        return func_00122630(&pa);
    }
    clsB = pb.fpClass;
    if (clsB < 2) {                      /* b is NaN -> propagate b, product sign */
        pb.sign = comb;
        return func_00122630(&pb);
    }
    if (clsA == 4) {                     /* a is inf */
        if (clsB == 2) {
            return func_00122630(&D_00141890);   /* inf * 0 = NaN */
        }
        pa.sign = comb;                  /* inf * (inf|normal) = inf */
        return func_00122630(&pa);
    }
    if (clsB == 4) {                     /* a is not inf, b is inf */
        if (clsA == 2) {
            return func_00122630(&D_00141890);   /* 0 * inf = NaN */
        }
        pb.sign = comb;                  /* normal * inf = inf */
        return func_00122630(&pb);
    }
    if (clsA == 2) {                     /* 0 * finite = zero */
        pa.sign = comb;
        return func_00122630(&pa);
    }
    if (clsB == 2) {                     /* normal * 0 = zero */
        pb.sign = comb;
        return func_00122630(&pb);
    }

    /* both normal: 122-bit product of the two 61-bit mantissas (high limb kept
     * with a sticky OR of the dropped low bits), then normalise + round. */
    aLo = (u64)pa.mantissa & 0xFFFFFFFFULL;
    aHi = (u64)pa.mantissa >> 32;
    bLo = (u64)pb.mantissa & 0xFFFFFFFFULL;
    bHi = (u64)pb.mantissa >> 32;
    p0 = (u64)__muldi3((s64)bLo, (s64)aLo);
    p1 = (u64)__muldi3((s64)bHi, (s64)aLo);
    p2 = (u64)__muldi3((s64)bLo, (s64)aHi);
    p3 = (u64)__muldi3((s64)bHi, (s64)aHi);
    mid = p1 + p2;
    midCarry = (mid < p1) ? 1 : 0;
    low64 = p0 + (mid << 32);
    lowCarry = (low64 < p0) ? 1 : 0;
    high64 = ((midCarry << 32) | lowCarry) + (((mid >> 32) & 0xFFFFFFFFULL) + p3);
    exp = pa.exponent + pb.exponent + 4;

    result.sign = comb;
    result.exponent = exp;

    while (high64 > 0x1FFFFFFFFFFFFFFFULL) {     /* renormalise down (carry) */
        s32 bit = (s32)(high64 & 1);
        result.exponent = ++exp;
        if (bit != 0) {
            low64 = (low64 >> 1) | 0x8000000000000000ULL;
        }
        high64 >>= 1;
    }
    while (high64 < 0x1000000000000000ULL) {     /* renormalise up */
        u64 topbit = low64 & 0x8000000000000000ULL;
        high64 <<= 1;
        if (topbit != 0) {
            high64 |= 1;
        }
        low64 <<= 1;
        result.exponent = --exp;
    }

    if ((s32)(high64 & 0xFF) == 0x80) {          /* round-to-nearest-even, low byte */
        if (high64 & 0x100) {                    /* odd -> round up */
            high64 += 0x80;
        } else if (low64 != 0) {                 /* even, inexact -> round up */
            high64 += 0x80;
        }
    }
    result.mantissa = (s64)high64;
    result.fpClass = 3;
    return func_00122630(&result);
}
#endif

/* func_00122DA8 = software double-precision DIVIDE (FpParts a / b -> packed
 * double via func_00122630): decompose both with func_00122760; NaN propagates;
 * result sign = signA^signB; inf/inf and 0/0 -> NaN constant &D_00141890;
 * inf/finite -> inf, 0/finite -> 0, finite/inf -> 0, finite/0 -> inf; both
 * normal -> exponent-align + restoring bitwise long-division of the 61-bit
 * mantissas with round-to-nearest-even.
 *
 * NEAR-MISS WALL (region-co-located with USA; same class as the add-core
 * func_00122800). Seedable: ships a faithful portable TARGET_NATIVE #else,
 * cmp-oracle'd bit-identical on the real R5900 (incl. the inf/inf and 0/0 NaN
 * paths reading D_00141890). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00122DA8);
#else
s64 func_00122DA8(s64 a, s64 b) {
    FpParts pa, pb;
    s64 va = a, vb = b;
    s32 clsA, clsB, comb, expR;
    u64 rem, divisor, bitmask, quotient;

    func_00122760(&va, &pa);
    func_00122760(&vb, &pb);

    clsA = pa.fpClass;
    if (clsA < 2) {                      /* a is NaN -> propagate a */
        return func_00122630(&pa);
    }
    clsB = pb.fpClass;
    if (clsB < 2) {                      /* b is NaN -> propagate b */
        return func_00122630(&pb);
    }
    comb = (pa.sign ^ pb.sign) != 0;
    pa.sign = comb;                      /* quotient sign = signA ^ signB */

    if (clsA == 4) {                     /* a is inf */
        if (clsB != 4) {
            return func_00122630(&pa);   /* inf / finite = inf */
        }
        return func_00122630(&D_00141890);   /* inf / inf = NaN */
    }
    if (clsA == 2) {                     /* a is zero */
        if (clsB != 2) {
            return func_00122630(&pa);   /* 0 / finite or 0 / inf = 0 */
        }
        return func_00122630(&D_00141890);   /* 0 / 0 = NaN */
    }
    /* a is normal */
    if (clsB == 4) {                     /* normal / inf = 0 */
        pa.mantissa = 0;
        pa.exponent = 0;
        return func_00122630(&pa);
    }
    if (clsB == 2) {                     /* normal / 0 = inf */
        pa.fpClass = 4;
        return func_00122630(&pa);
    }

    /* both normal: restoring bitwise long division of the 61-bit mantissas. */
    expR = pa.exponent - pb.exponent;
    rem = (u64)pa.mantissa;
    divisor = (u64)pb.mantissa;
    if (rem < divisor) {                 /* pre-shift so the leading bit lands at 2^60 */
        expR--;
        rem <<= 1;
    }
    pa.exponent = expR;
    bitmask = 0x1000000000000000ULL;     /* quotient bit 60, descending to bit 0 */
    quotient = 0;
    do {
        if (rem >= divisor) {
            quotient |= bitmask;
            rem -= divisor;
        }
        bitmask >>= 1;
        rem <<= 1;
    } while (bitmask != 0);

    if ((s32)(quotient & 0xFF) == 0x80) {        /* round-to-nearest-even, low byte */
        if (quotient & 0x100) {                  /* odd -> round up */
            quotient += 0x80;
        } else if (rem != 0) {                   /* even, inexact remainder -> round up */
            quotient += 0x80;
        }
    }
    pa.mantissa = (s64)quotient;
    return func_00122630(&pa);
}
#endif

/**
 * func_00122F10 = ordered comparison of two decomposed doubles (FpParts):
 * NaN->1, else order by class then (for equal-sign normals) exponent then
 * unsigned mantissa. MATCHED (byte-exact) via the near-miss idiom levers, same as
 * the USA twin (delta 0): (cX^K)==0 xori class tests, c?1:-1 (movz) vs (c==0)?-1:1
 * (movn) sign-selects, and a-first magnitude compares (a>b) to recover the
 * annulling bnel + register threading.
 */
s32 func_00122F10(FpParts *a, FpParts *b) {
    s32 ca = a->fpClass;
    s32 cb;
    if ((u32)ca < 2) {
        return 1;
    }
    cb = b->fpClass;
    if ((u32)cb < 2) {
        return 1;
    }
    if ((ca ^ 4) == 0) {
        if ((cb ^ 4) == 0) {
            return b->sign - a->sign;
        }
        return a->sign ? -1 : 1;
    }
    if ((cb ^ 4) == 0) {
        return b->sign ? 1 : -1;
    }
    if ((ca ^ 2) == 0) {
        if ((cb ^ 2) == 0) {
            return 0;
        }
        return (b->sign == 0) ? -1 : 1;
    }
    if ((cb ^ 2) == 0) {
        return a->sign ? -1 : 1;
    }
    if (a->sign != b->sign) {
        return a->sign ? -1 : 1;
    }
    if (a->exponent > b->exponent) {
        return a->sign ? -1 : 1;
    }
    if (b->exponent > a->exponent) {
        return a->sign ? 1 : -1;
    }
    if ((u64)a->mantissa > (u64)b->mantissa) {
        return a->sign ? -1 : 1;
    }
    if ((u64)b->mantissa > (u64)a->mantissa) {
        return a->sign ? 1 : -1;
    }
    return 0;
}

/**
 * Compare two doubles by IEEE-754 class: decompose each operand with
 * func_00122760, then combine the two classifications via func_00122F10 and
 * return its result.
 */
s32 func_00123028(s64 a, s64 b) {
    s64 va = a;
    s64 vb = b;
    FpParts pa;
    FpParts pb;
    func_00122760(&va, &pa);
    func_00122760(&vb, &pb);
    return func_00122F10(&pa, &pb);
}

/**
 * func_00123078 = convert a 32-bit signed integer to a double (soft-float
 * __floatsidf). zero -> class 2; else class 3 with magnitude mantissa, exponent
 * seeded 60, normalising left-shift loop to put the leading bit at position 60;
 * INT_MIN returns the constant double -2^31 (0xC1E0000000000000). Recomposed by
 * func_00122630.
 *
 * NEAR-MISS WALL (87.07% via objdiff, region-co-located with USA). Control flow,
 * constants and the `(u64)-1 >> 4` limit all match; the residual is ee-gcc -O2
 * -G0 delay-slot/register allocation in the normalise loop/tail. Shipped as a
 * portable TARGET_NATIVE #else; verification routed to tester-EE (calls sibling
 * soft-float func_00122630 #else).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00123078);
#else
s64 func_00123078(s32 x) {
    FpParts parts;
    s64 mant;
    parts.fpClass = 3;
    parts.sign = (u32)x >> 31;
    if (x == 0) {
        parts.fpClass = 2;
        return func_00122630(&parts);
    }
    parts.exponent = 60;
    if (parts.sign != 0) {
        if (x == (s32)0x80000000) {
            return (s64)0xC1E0000000000000ULL;
        }
        parts.mantissa = -x;
    } else {
        parts.mantissa = x;
    }
    mant = parts.mantissa;
    if ((u64)mant <= (u64)-1 >> 4) {
        s32 exp = parts.exponent;
        do {
            mant <<= 1;
            exp--;
        } while ((u64)mant <= (u64)-1 >> 4);
        parts.exponent = exp;
        parts.mantissa = mant;
    }
    return func_00122630(&parts);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00123130);

/**
 * func_001231C8 = convert a non-negative double to a 32-bit integer (truncate
 * toward zero; soft-float __fixunsdfsi-style). NaN/zero/subnormal/negative -> 0;
 * infinity and exponent>=32 -> 0xFFFFFFFF; otherwise shift the class-3 mantissa
 * (leading bit at 60) into place: right by (60-exp), left by (exp-60) above.
 *
 * MATCHED (byte-exact) via the near-miss idiom levers — same as the USA twin
 * (region-co-located, delta 0): return type u32 makes 0xFFFFFFFF build as
 * `lui;ori` (not s32 `li -1`) and frees the annulling `bnel`; `(cls^K)==0` forces
 * the `xori;beqz` class tests; and `if (exp>=0x3D) return <<; return >>;` lays the
 * `<<` block first to match the original's order. Calls sibling func_00122760.
 */
u32 func_001231C8(s64 a) {
    s64 va = a;
    FpParts parts;
    s32 cls;
    s32 exp;
    func_00122760(&va, &parts);
    cls = parts.fpClass;
    if ((cls ^ 2) == 0) {
        return 0;
    }
    if ((u32)cls < 2) {
        return 0;
    }
    if (parts.sign != 0) {
        return 0;
    }
    if ((cls ^ 4) == 0) {
        return 0xFFFFFFFFu;
    }
    exp = parts.exponent;
    if (exp < 0) {
        return 0;
    }
    if (exp >= 0x20) {
        return 0xFFFFFFFFu;
    }
    if (exp >= 0x3D) {
        return (u32)((u64)parts.mantissa << (exp - 0x3C));
    }
    return (u32)((u64)parts.mantissa >> (0x3C - exp));
}

/**
 * Build an FpParts descriptor from explicit class/sign/exponent and a 64-bit
 * mantissa (8-byte aligned at offset 0x10) and recompose it into a packed double
 * via func_00122630, RETURNING that double (the .s tail-passes func_00122630's
 * v0/v1 through unchanged — no reload before jr ra).
 */
s64 func_00123268(s32 fpClass, s32 sign, s32 exponent, s64 mantissa) {
    FpParts parts;
    parts.fpClass = fpClass;
    parts.sign = sign;
    parts.exponent = exponent;
    parts.mantissa = mantissa;
    return func_00122630(&parts);
}

/**
 * Round a double towards a 30-bit significand: decompose the operand, take the
 * top 30 bits of its 64-bit mantissa, OR in a sticky bit if any of the low 30
 * bits are set, and forward the class/sign/exponent plus that rounded mantissa
 * to func_001234C0.
 */
void func_00123298(s64 a) {
    s64 va = a;
    FpParts parts;
    s32 high;
    s32 rounded;
    func_00122760(&va, &parts);
    high = (s32)(parts.mantissa >> 30);
    rounded = high | 1;
    if ((parts.mantissa & 0x3FFFFFFF) == 0) {
        rounded = high;
    }
    func_001234C0(parts.fpClass, parts.sign, parts.exponent, rounded);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001232EC);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001232F0);

/* Decomposed IEEE-754 single produced by func_00123400 (32-bit fields). */
typedef struct {
    s32 fpClass;   /* 0x00: 0=sNaN,1=qNaN,2=zero/subnormal,3=normal,4=inf */
    s32 sign;      /* 0x04 */
    s32 exponent;  /* 0x08: unbiased (bias 0x7F) */
    s32 mantissa;  /* 0x0C: normal = (frac<<7)|(1<<30) */
} SpParts;

/* func_00123400: decompose the IEEE-754 single at src[0] into SpParts, returning
 * the class. NEAR-MISS WALL (~68%): register allocation differs AND the asm
 * returns 1 for both NaN kinds while storing class 0 for a signalling NaN.
 * Shipped as a cmp-oracle'd portable #else (run_cmp_015180_iso.sh). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00123400);
#else
s32 func_00123400(u32 *src, SpParts *out) {
    u32 bits = src[0];
    s32 frac = (s32)(bits & 0x7FFFFF);
    s32 exp = (s32)((bits >> 23) & 0xFF);
    out->sign = (s32)(bits >> 31);
    if (exp == 0) {
        return out->fpClass = 2;
    }
    if (exp == 0xFF) {
        if (frac == 0) {
            return out->fpClass = 4;
        }
        out->fpClass = (frac & 0x100000) ? 1 : 0;
        out->mantissa = frac;
        return 1;   /* asm leaves 1 in the return reg for both NaN kinds */
    }
    out->mantissa = (frac << 7) | 0x40000000;
    out->exponent = exp - 0x7F;
    return out->fpClass = 3;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00123490);

/**
 * Pack four 32-bit arguments into a stack record and hand it to func_001232F0.
 */
void func_001234C0(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 args[4];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    func_001232F0(args);
}

extern s32 func_00123400(u32 *src, SpParts *out);

/**
 * func_001234F0 = convert a single-precision float to a double and recompose it
 * (soft-float __extendsfdf2 helper): decompose into SpParts (func_00123400),
 * widen the 31-bit single mantissa to the 61-bit double form (<<30), hand to
 * func_00123268, RETURNING the recomposed double (the soft-float __extendsfdf2
 * widen result — the .s tail-passes func_00123268's v0/v1 through, no reload
 * before jr ra).
 *
 * NEAR-MISS WALL (75.00% via objdiff, region-co-located with USA): post-call body
 * is byte-identical; the only divergence is ee-gcc -O2 -G0 scheduling the three
 * prologue insns {save $31, &f, swc1 spill} in a different order. Shipped as a
 * portable TARGET_NATIVE #else; cmp-oracle'd (extendsfdf2, the double-bits return).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001234F0);
#else
s64 func_001234F0(float f) {
    SpParts sp;
    func_00123400((u32 *)&f, &sp);
    return func_00123268(sp.fpClass, sp.sign, sp.exponent,
                         (s64)((u64)(u32)sp.mantissa << 30));
}
#endif

/**
 * Decode a little-endian base-128 varint from src into *out, 7 bits per byte
 * with bit 7 as the continuation flag. Returns the pointer just past the last
 * byte consumed.
 */
u8 *func_00123530(u8 *src, s32 *out) {
    s32 shift = 0;
    s32 value;
    u8 b;
    b = *src;
    src++;
    value = b & 0x7F;
    while (b & 0x80) {
        b = *src;
        src++;
        shift += 7;
        value |= (b & 0x7F) << shift;
    }
    *out = value;
    return src;
}

/**
 * Decode a signed (sign-extended) little-endian base-128 varint from src into
 * *out: 7 bits per byte, bit 7 continues. After the last byte, if fewer than 32
 * bits were consumed and the value's sign bit (0x40 of the final byte) is set,
 * the high bits are filled with ones. Returns the pointer past the last byte.
 */
u8 *func_00123578(u8 *src, s32 *out) {
    u32 shift = 0;
    s32 value = 0;
    u8 b;
    do {
        b = *src;
        src++;
        value |= (b & 0x7F) << shift;
        shift += 7;
    } while (b & 0x80);
    if (shift < 0x20 && (b & 0x40)) {
        value |= -1 << shift;
    }
    *out = value;
    return src;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001235C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001236C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00123930);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00123978);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00123A00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00123B40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00123C28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00123D30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001240C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001242A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124414);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001244B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124540);

/**
 * Register interrupt handler `id` (low 16 bits): build a small descriptor on the
 * stack (mode=1), create the handler object via func_0011AC20, bind the
 * func_00124540 trampoline to it with func_0011A9A0, then enable
 * (func_0011AC60) and commit (func_0011AC30) it.
 */
void func_00124568(s32 id) {
    s32 desc[8];
    s32 obj;
    s32 channel = id & 0xFFFF;
    desc[1] = 1;
    desc[2] = 0;
    desc[5] = 0;
    obj = func_0011AC20(desc);
    func_0011A9A0(channel, func_00124540, obj);
    func_0011AC60(obj);
    func_0011AC30(obj);
}

/**
 * Install `handler` as the active interrupt handler in the global D_001418C0.
 * Aborts (returning 0) if func_00124B88(1) reports the slot is busy. Otherwise,
 * with interrupts disabled (func_0011F5E0), swaps in the new handler, restores
 * the prior interrupt-enable state (func_0011F628 when they were on) and returns
 * the handler it replaced.
 */
s32 func_001245D0(s32 handler) {
    s32 old;
    s32 wasEnabled;
    if (func_00124B88(1) != 0) {
        return 0;
    }
    wasEnabled = func_0011F5E0();
    old = D_001418C0;
    D_001418C0 = handler;
    if (wasEnabled != 0) {
        func_0011F628();
    }
    return old;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124630);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001246D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124780);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124818);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001248B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001248F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124970);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124980);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124AF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124B88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124C28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124C98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00124E08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001250E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001252E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001253A4);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001253A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00125588);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00125620);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001256D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001257D0);

/**
 * Accessor: return the address of the global D_00137E80.
 */
s32 *func_00125960(void) {
    return &D_00137E80;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012596C);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00125970);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00125A10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00125A20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00125D94);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00125E54);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00125E58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00125F20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00126104);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001261F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00126284);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012646C);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001265B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012672C);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00126DBC);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00126E60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00126ED0);

/* func_00126ED8 is DmaSprAddrToMadr - SPR pointer to MADR conversion (kept func_ name - matched). */
u32 func_00126ED8(u32 arg0) {
    if ((arg0 >> 28) == 7) {
        arg0 &= 0x0FFFFFFF;
        arg0 |= 0x80000000;
    }
    return arg0;
}

/**
 * Zero `count` bytes starting at `dst` (a simple byte-wise memset to 0).
 */
void func_00126F00(u8 *dst, s32 count) {
    s32 i;
    for (i = count - 1; i != -1; i--) {
        *dst = 0;
        dst++;
    }
}

/* func_00126F38 is sceDmaGetChan - libdma channel-struct lookup (kept func_ name - matched). */
s32 func_00126F38(u32 arg0) {
    if (arg0 < 0xA) {
        return D_00137EB0[arg0];
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00126F60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00127040);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00127218);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00127288);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001272A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00127340);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McInit);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00127500);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McOpen);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McBeginCreateFile);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McClose);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McSeek);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001277F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McRead);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McWrite);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00127B18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McDelayMillis);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McSync);

/**
 * Read fields from the structure at physical address arg0 (accessed through the
 * uncached mirror, arg0 | 0x20000000) and publish them through three optional
 * global out-pointers: p[0] -> *D_00141BA8, p[1] -> *D_00141BAC, and the word at
 * p+0x90 -> *D_00141BB0. Each store is skipped if its out-pointer is null.
 */
void func_00127C68(u32 arg0) {
    s32 *p = (s32 *)(arg0 | 0x20000000);
    if (D_00141BA8) *D_00141BA8 = p[0];
    if (D_00141BAC) *D_00141BAC = p[1];
    if (D_00141BB0) *D_00141BB0 = *(s32 *)((char *)p + 0x90);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McGetInfo);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00127E40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McGetDir);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00127F90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McChdir);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McMkDir);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", McGetEntSpace);

/**
 * Initialise the D_00143200 subsystem by calling func_0011D620 with the config
 * block at &D_00143188, mode 0x80000963, two 0x400-sized buffers both pointing
 * at D_00143200, and zeroed trailing arguments; returns the resulting handle
 * stored in D_00143200[0]. (EU +0x80 twin; array phrasing shared with
 * func_00128440 — preserves the D_00143200 reloc, byte-neutral vs the scalar.)
 */
s32 func_00128250(void) {
    func_0011D620(&D_00143188, 0x80000963, 0, D_00143200, 0x400,
                  D_00143200, 0x400, 0, 0);
    return D_00143200[0];
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001282A8);

/**
 * func_00128440(arg0): open the D_00143200 subsystem in mode 0x80000904 with
 * arg0 stored at D_00143200[1]; on failure log D_0013B8E8 (func_00128898) and
 * return 0, else return the handle D_00143200[0]. EU +0x80 twin; see the USA
 * unit for the match notes (array extern, not the scalar (&D)[1] idiom).
 */
s32 func_00128440(s32 arg0) {
    D_00143200[1] = arg0;
    if (func_0011D620(&D_00143188, 0x80000904, 0, D_00143200, 0x400,
                      D_00143200, 0x400, 0, 0) < 0) {
        func_00128898(D_0013B8E8);
        return 0;
    }
    return D_00143200[0];
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001284B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00128578);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001286C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001286C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001287A8);

/**
 * Compiled-out VARARGS debug print stub (libmc area): the body is empty but
 * the `...` still makes ee-gcc home the unnamed arg registers $5-$11 to the
 * 0x80-byte stack area (the sd run at +0x48..+0x78) — that varargs prologue
 * IS the whole function. First arg is the (ignored) format string.
 */
void func_00128898(const char *fmt, ...) {
}

/**
 * Reset the 16-entry table at D_001436C0 (each entry is 0x330 bytes): zero the
 * first three words of every entry across the 0x3300-byte span, set the
 * initialised flag D_00137F00 to 1, and return 1.
 */
s32 func_001288C0(void) {
    s32 *entry;
    s32 *end;
    D_00137F00 = 1;
    entry = (s32 *)D_001436C0;
    end = (s32 *)((u8 *)D_001436C0 + 0x3300);
    do {
        entry[0] = 0;
        entry[1] = 0;
        entry[2] = 0;
        entry = (s32 *)((u8 *)entry + 0x330);
    } while ((s32)entry < (s32)end);
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00128900);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00128A48);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00128B28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00128C18);

extern s32 func_00128578(s32 index);

/**
 * func_00128D58(index): acquire a resource via func_00128578(index); on
 * success record the handle at entry+0x8 and set the active flag at entry+0x4
 * in the 0x330-stride D_001436C0 table; returns the handle (negative =
 * failure). EU +0x80 twin (table at D_001436C0); see the USA unit for notes.
 */
s32 func_00128D58(s32 index) {
    s32 h = func_00128578(index);
    if (h < 0) {
        return h;
    }
    D_001436C0[index].handle = h;
    D_001436C0[index].active = 1;
    return h;
}

extern void func_0011B3D0(void *arg0, void *arg1);

/**
 * func_00128DB0(index): run func_0011B3D0 over table entry `index`'s object
 * (args: obj, obj+0x100), then return whichever of the object's two
 * sub-instances (obj / obj+0x80) has the larger +0x7C word (ties -> obj).
 * EU +0x80 twin (table at D_001436C0); see the USA unit for the match notes.
 */
ResSubObj *func_00128DB0(s32 index) {
    ResSubObj *pair[2];
    ResSubObj *obj = D_001436C0[index].obj;
    pair[0] = obj;
    pair[1] = (ResSubObj *)((u8 *)obj + 0x80);
    func_0011B3D0(obj, (u8 *)obj + 0x100);
    return pair[pair[0]->unk7C < pair[1]->unk7C];
}

extern s32 D_00137F08[];

/**
 * func_00128E18(index): lazily refresh the two-word state cache D_00137F08
 * from table entry `index`'s object; returns 0 when obj->unk7C is 0 or the
 * cache is current, else updates from the pair and returns 1. EU +0x80 twin
 * (cache D_00137F08, table D_001436C0); see the USA unit for the match notes.
 */
s32 func_00128E18(s32 index) {
    ResSubObj *pair[2];
    ResSubObj *obj = D_001436C0[index].obj;
    ResSubObj *next = (ResSubObj *)((u8 *)obj + 0x80);
    s32 k = obj->unk7C;
    pair[0] = obj;
    pair[1] = next;
    if (k == 0) {
        return 0;
    }
    if (D_00137F08[0] == k && D_00137F08[1] == next->unk7C) {
        return 0;
    }
    D_00137F08[0] = pair[0]->unk7C;
    D_00137F08[1] = pair[1]->unk7C;
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00128E98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00128F48);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00128FD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001290BC);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00129120);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00129160);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001291A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00129218);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001292C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00129368);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00129410);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00129450);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001296A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00129DA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012A1C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012A3E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012A460);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012A4F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012A5B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012A680);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012A730);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012A7E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012A8E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012A9E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012AA80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012AB30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012AC10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012ACF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012ADD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012AEA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012AFC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012B0D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012B138);

/**
 * Set bit 23 of the hardware register at 0x10002010 (IPU_CTRL) to the low bit of
 * arg0, preserving all other bits (read-modify-write with mask 0xFF7FFFFF).
 */
void func_0012B198(s32 arg0) {
    u32 *reg = (u32 *)0x10002010;
    *reg = (*reg & 0xFF7FFFFF) | (arg0 << 23);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012B1C0);

/* func_0012B3C0(arg0): thin wrapper — forwards to func_0012C508(arg0, 3) and
 * returns its result. (The prior "sibling-call wall" note was wrong: ee-gcc 2.9
 * has NO sibling-call optimization for a value-returning call, so this compiles
 * to the original's jal + real frame — byte-exact. EU matches USA at delta 0.) */
extern s32 func_0012C508(s32 arg0, s32 arg1);
s32 func_0012B3C0(s32 arg0) {
    return func_0012C508(arg0, 3);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012B3E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012B568);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012B678);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012B780);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012B8B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012BAA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012BB60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012C008);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012C090);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012C230);

/**
 * Issue an IPU command: write `cmd` to the IPU_CMD hardware register
 * (0x10002000), then look up D_00137F90[cmd >> 28] (indexed by the command's
 * top nibble = the IPU opcode) and cache it in arg0->field_0x818.
 */
void func_0012C380(s32 *arg0, u32 cmd) {
    *(volatile u32 *)0x10002000 = cmd;
    *(s32 *)((u8 *)arg0 + 0x818) = D_00137F90[cmd >> 28];
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012C3B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012C458);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012C508);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012C680);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012C788);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012C878);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012C9C8);

/**
 * Run channel 5's transfer on arg0, recording its handle at arg0->field_0x1B4.
 * If channel 1 is ready (func_0012C878(arg0, 1) is non-zero) kick it off again,
 * fire channel 7 via func_0012C788 and flush through func_0012CFA0. Returns 0.
 */
s32 func_0012CA48(s32 *arg0) {
    arg0[0x6D] = func_0012C878(arg0, 5);
    if (func_0012C878(arg0, 1) != 0) {
        func_0012C878(arg0, 1);
        func_0012C788(arg0, 7);
        func_0012CFA0(arg0);
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012CAB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012CBC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012CC88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012CDB0);

/**
 * Drain object arg0: while channel 1 still reports work
 * (func_0012C878(arg0, 1) is non-zero), keep servicing channel 8 via
 * func_0012C788(arg0, 8). The trailing channel-1 poll (0 on exit) is left in
 * the return register; callers ignore it.
 */
s32 func_0012CFA0(s32 *arg0) {
    while (func_0012C878(arg0, 1) != 0) {
        func_0012C788(arg0, 8);
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012CFE8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012D060);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012D100);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012D1C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012D2C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012D350);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012D420);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012D4B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012D768);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012D808);

/**
 * State transition on object arg0: if its state field_0x8 (arg0[2]) is not
 * already 2, copy field_0x118 (arg0[0x46]) into field_0xAC (arg0[0x2B]) and set
 * the state to 2. Always sets the dirty/request flag field_0x820 (arg0[0x208])
 * to 1.
 */
void func_0012DA98(s32 *arg0) {
    if (arg0[2] != 2) {
        arg0[0x2B] = arg0[0x46];
        arg0[2] = 2;
    }
    arg0[0x208] = 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012DAC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012DC50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012DD60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012DF18);

/**
 * Program and start the DMA channel at 0x1000B000 for a chain/normal transfer:
 * with interrupts disabled, set MADR (0x1000B010) to the 28-bit address `madr`
 * tagged with bit31, QWC (0x1000B020) to `size >> 4` quadwords, then CHCR
 * (0x1000B000) to 0x100 to kick it. Restore interrupts only if they had been on.
 */
void func_0012E088(u32 madr, s32 size) {
    s32 wasEnabled = func_0011F5E0();
    *(volatile u32 *)0x1000B010 = (madr & 0x0FFFFFFF) | 0x80000000;
    *(volatile u32 *)0x1000B020 = size >> 4;
    *(volatile u32 *)0x1000B000 = 0x100;
    if (wasEnabled) {
        func_0011F628();
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012E10C);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012E110);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012E238);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012E378);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012E538);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012E608);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012E890);

/**
 * Extract the top arg1 bits of the 64-bit value at *arg0: returns
 * (s32)(*arg0 >> (64 - arg1)) — i.e. the most-significant arg1 bits, right
 * aligned. (Bitstream/MSB-first reader helper.)
 */
s32 func_0012E8C8(u64 *arg0, s32 arg1) {
    u64 val = *arg0;
    return (s32)(val >> (0x40 - arg1));
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012E8E8);

/**
 * Read arg1 bits from the bitstream at arg0 (func_0012E8C8(arg0, arg1)) and
 * then advance the stream by arg1 bits (func_0012E8E8(arg0, arg1)), returning
 * the value that was read.
 */
s32 func_0012E980(u64 *arg0, s32 arg1) {
    s32 value = func_0012E8C8(arg0, arg1);
    func_0012E8E8(arg0, arg1);
    return value;
}

/**
 * Read a single bit from the bitstream at arg0 (func_0012E8C8(arg0, 1)) and
 * then advance the stream by one bit (func_0012E8E8(arg0, 1)), returning the
 * bit that was read.
 */
s32 func_0012E9D0(u64 *arg0) {
    s32 bit = func_0012E8C8(arg0, 1);
    func_0012E8E8(arg0, 1);
    return bit;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012EA18);

/**
 * Advance a ring-buffer read/write cursor. arg0 is a buffer descriptor:
 *   arg0[2]  = current offset, arg0[9] = end offset, arg0[10] = span.
 * Adds (arg1 >> 3) entries to the current offset and wraps it back by the span
 * if it reaches/passes the end. Returns the new offset.
 */
s32 func_0012EA70(s32 *arg0, s32 arg1) {
    u32 pos = arg0[2] + (arg1 >> 3);
    if (pos >= (u32)arg0[9]) {
        pos -= arg0[10];
    }
    return pos;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012EA9C);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012EAA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012EB28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012EE28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012EF20);

/**
 * Skip one tagged record in the bitstream arg0: consume the 0x38-bit and 0x28-
 * bit header fields, then keep consuming 0x18-bit entries while the following
 * marker bit (func_0012E8C8(arg0, 1)) reads 1. Always returns 1.
 */
s32 func_0012F070(u64 *arg0) {
    func_0012E980(arg0, 0x38);
    func_0012E980(arg0, 0x28);
    while (func_0012E8C8(arg0, 1) == 1) {
        func_0012E980(arg0, 0x18);
    }
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012F0E0);

/**
 * Abort both DMA channels (0x1000B000 and 0x1000B400): with interrupts disabled,
 * set then clear the DMA enable bit while clearing each channel's CHCR.STR
 * (0x100) bit, restore interrupts if they had been on, zero the channels' QWC
 * (0x1000B020 / 0x1000B420), then re-init the GIF path via func_00130E88.
 */
void func_0012F690(void) {
    s32 wasEnabled = func_0011F5E0();
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 | 0x10000;
    *(volatile u32 *)0x1000B000 &= 0xFFFFFEFF;
    *(volatile u32 *)0x1000B400 &= 0xFFFFFEFF;
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 & 0xFFFEFFFF;
    if (wasEnabled) {
        func_0011F628();
    }
    *(volatile u32 *)0x1000B020 = 0;
    *(volatile u32 *)0x1000B420 = 0;
    func_00130E88();
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012F738);

/**
 * Stub predicate that always returns 1 (a registered callback whose default
 * answer is "true"/success).
 */
s32 func_0012F940(void) {
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012F948);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012F998);

/**
 * Predicate: follow arg0->field_0x40 (arg0[0x10]) to a sub-object and return 1
 * if that object's field_0x4 (base[1]) is zero, else 0.
 */
s32 func_0012F9B8(s32 *arg0) {
    s32 *base = (s32 *)arg0[0x10];
    return base[1] == 0;
}

/**
 * Reset the sub-object held at arg0->field_0x40: clear its leading three words
 * and arg0->field_0x8, clear field_0xAC, mark field_0x80 invalid (-1), run the
 * teardown helper func_00130178 on it, clear field_0x118, then hand off to
 * func_00130088 to finish (re)initialising it.
 */
void func_0012F9C8(s32 *arg0) {
    s32 *base = (s32 *)arg0[0x10];
    base[0] = 0;
    base[1] = 0;
    base[2] = 0;
    arg0[2] = 0;
    base[0x2B] = 0;
    base[0x20] = -1;
    func_00130178(base);
    base[0x46] = 0;
    func_00130088(base);
}

/**
 * Follow arg0->field_0x40 (arg0[0x10]) to a sub-object, then for each of six
 * child pointers stored at base offsets 0x1B8,0x1C8,0x1D8 and 0x1BC,0x1CC,0x1DC
 * (base[0x6E,0x72,0x76,0x6F,0x73,0x77]), clear that child's field_0x28
 * (child[0xA]) to 0 when the pointer is non-null. Returns 1.
 */
s32 func_0012FA18(s32 *arg0) {
    s32 *base = (s32 *)arg0[0x10];
    s32 *p;
    p = (s32 *)base[0x6E]; if (p) p[0xA] = 0;
    p = (s32 *)base[0x72]; if (p) p[0xA] = 0;
    p = (s32 *)base[0x76]; if (p) p[0xA] = 0;
    p = (s32 *)base[0x6F]; if (p) p[0xA] = 0;
    p = (s32 *)base[0x73]; if (p) p[0xA] = 0;
    p = (s32 *)base[0x77]; if (p) p[0xA] = 0;
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012FA70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012FA98);

/**
 * Run entry #1's destructor on `obj`: build a request whose index word is 1 and
 * dispatch it via func_0012FA98(obj, req). The request occupies a 0x20-byte
 * stack buffer (only its first word, the entry index, is used here).
 */
s32 func_0012FAE8(s32 *obj) {
    s32 req[8];
    req[0] = 1;
    return func_0012FA98(obj, req);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012FB10);

/**
 * Initialise a cursor/range descriptor arg0: store the start (arg1) and limit
 * (arg2) at field_0x0/field_0x4, and seed both the current (field_0x8) and
 * saved (field_0xC) positions to the start.
 */
void func_0012FB48(s32 *arg0, s32 arg1, s32 arg2) {
    arg0[0] = arg1;
    arg0[1] = arg2;
    arg0[2] = arg1;
    arg0[3] = arg1;
}

/**
 * Save the current position: copy field_0x8 (arg0[2]) into the saved slot
 * field_0xC (arg0[3]).
 */
void func_0012FB60(s32 *arg0) {
    arg0[3] = arg0[2];
}

/**
 * Restore the saved position: copy field_0xC (arg0[3]) back into the current
 * slot field_0x8 (arg0[2]).
 */
void func_0012FB70(s32 *arg0) {
    arg0[2] = arg0[3];
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012FB80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012FBF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012FD60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012FE78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0012FEC0);

/**
 * Commit the pending range on arg0's sub-object (arg0->field_0x40): if both
 * obj->field_0x4 and obj->field_0x8 are set, flush it via func_00130098,
 * record the produced length (obj->field_0x118 - obj->field_0xAC) in
 * arg0->field_0x8, clear obj->field_0x4 and return 1; otherwise return 0.
 * (The explicit `end` temporary forces the original field_0x118-before-
 * field_0xAC load order, which `a-b` alone evaluates the other way.)
 */
s32 func_00130020(s32 *arg0) {
    s32 *obj = (s32 *)arg0[0x10];
    s32 ret = 0;
    if (obj[1] && obj[2]) {
        s32 end;
        func_00130098(obj);
        end = obj[0x46];
        arg0[2] = end - obj[0x2B];
        ret = 1;
        obj[1] = 0;
    }
    return ret;
}

/**
 * Clear arg0->field_0x848 and (re)initialise subsystem 1 via func_0012B198(1).
 * The call is a tail call.
 */
void func_00130088(s32 *arg0) {
    *(s32 *)((u8 *)arg0 + 0x848) = 0;
    func_0012B198(1);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00130098);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00130118);

/**
 * Hard-reset the DMA/GIF path attached to `obj`: flag the context busy
 * (field_0x818 = 1, field_0x1B0 = 0), then with interrupts disabled stop both
 * DMA channels (0x1000B000/0x1000B400) and their VIF (0x1000D400), zero each
 * channel's QWC (0x1000B020/0x1000B420/0x1000D420), reset the GIF mode register
 * (0x10002010 = 0x40000000) and finish by waiting on GIF idle via
 * func_00130DB8(0). Interrupts are restored only if they had been on.
 */
void func_00130178(s32 *obj) {
    s32 wasEnabled;
    *(s32 *)((u8 *)obj + 0x818) = 1;
    *(s32 *)((u8 *)obj + 0x1B0) = 0;
    wasEnabled = func_0011F5E0();
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 | 0x10000;
    *(volatile u32 *)0x1000B000 = 0;
    *(volatile u32 *)0x1000B400 = 0;
    *(volatile u32 *)0x1000D400 = 0;
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 & 0xFFFEFFFF;
    if (wasEnabled) {
        func_0011F628();
    }
    *(volatile u32 *)0x1000B020 = 0;
    *(volatile u32 *)0x1000B420 = 0;
    *(volatile u32 *)0x1000D420 = 0;
    *(volatile u32 *)0x10002010 = 0x40000000;
    func_00130DB8(0, 0);
}

extern u8 D_0013BE68[];
extern void *func_0011C820(const char *format, ...);

/**
 * Default message handler: print the message `buf` through func_0011C820
 * (the EU Kprintf twin) with the fixed format string D_0013BE68 (EU +0x80
 * twin of USA D_0013BDE8). Void tail call → sibling-call-optimised to the
 * original's frameless `j func_0011C820`.
 */
void func_00130240(void *buf) {
    func_0011C820((const char *)D_0013BE68, buf);
}

/**
 * Build a temporary 256-byte descriptor on the stack via func_00115DA8, then
 * dispatch it for arg0 through func_00130288(arg0, buf).
 */
void func_00130250(s32 arg0) {
    u8 buf[256];
    func_00115DA8(buf);
    func_00130288(arg0, buf);
}

/**
 * Route the message `buf` for object `arg0`: when arg0 is live and has both a
 * registered sub-object (field_0x858) and a non-null field_0xC, deliver it to
 * that sub-object via func_0012FA98 (request = {0, buf}); otherwise fall back to
 * the default handler func_00130240.
 */
void func_00130288(s32 arg0, void *buf) {
    s32 *self = (s32 *)arg0;
    s32 *obj;
    s32 req[2];
    obj = (s32 *)self[0x216];
    if (obj != 0 && self != 0 && self[3] != 0) {
        req[1] = (s32)buf;
        req[0] = 0;
        func_0012FA98(obj, req);
    } else {
        func_00130240(buf);
    }
}

/**
 * Store a width/height (or x/y) pair into descriptor arg0: arg1 -> field_0x4,
 * arg2 -> field_0x8, plus their >>4 (divided-by-16, e.g. pixels->blocks)
 * counterparts into field_0xC and field_0x10. Returns 1.
 */
s32 func_001302E0(s32 *arg0, s32 arg1, s32 arg2) {
    arg0[1] = arg1;
    arg0[2] = arg2;
    arg0[3] = arg1 >> 4;
    arg0[4] = arg2 >> 4;
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00130300);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00130428);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001306D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001307B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00130890);

/**
 * Query a batch of channel/register states via func_0012C878(obj, selector):
 * prime selector 3, and only if selector 1 is set, sample selector 8 three
 * times (caching the last into obj+0x144). Then cache selector 0xE into
 * obj+0x148, pulse selector 1, and cache selector 0xE again into obj+0x14C.
 */
void func_001309C0(s32 *obj) {
    func_0012C878(obj, 3);
    if (func_0012C878(obj, 1) != 0) {
        func_0012C878(obj, 8);
        func_0012C878(obj, 8);
        *(s32 *)((u8 *)obj + 0x144) = func_0012C878(obj, 8);
    }
    *(s32 *)((u8 *)obj + 0x148) = func_0012C878(obj, 0xE);
    func_0012C878(obj, 1);
    *(s32 *)((u8 *)obj + 0x14C) = func_0012C878(obj, 0xE);
}

extern u8 D_0013BED8[];
extern u8 D_0013BF08[];
extern u8 D_0013BF20[];
extern u8 D_0013BF58[];
extern void func_00130C68(u8 *arg0);

/**
 * Frameless tail-call thunk: dispatch arg0 through func_00130288 with the
 * fixed message table D_0013BED8 (EU +0x80 twin of USA D_0013BE58). Void tail
 * call → sibling-call-optimised into the original's `j func_00130288`.
 * Sibling thunks A60/A70/A80 differ only by table.
 */
void func_00130A50(s32 arg0) {
    func_00130288(arg0, D_0013BED8);
}

/** Sibling of func_00130A50 with message table D_0013BF08 (USA D_0013BE88). */
void func_00130A60(s32 arg0) {
    func_00130288(arg0, D_0013BF08);
}

/** Sibling of func_00130A50 with message table D_0013BF20 (USA D_0013BEA0). */
void func_00130A70(s32 arg0) {
    func_00130288(arg0, D_0013BF20);
}

/** Sibling of func_00130A50 with message table D_0013BF58 (USA D_0013BED8). */
void func_00130A80(s32 arg0) {
    func_00130288(arg0, D_0013BF58);
}

/* func_00130A8C: starts with a 0xCDCDCDCD fill word (uninitialised-memory
 * pattern) before the real entry at 0x130A90 — not producible from C. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00130A8C);

/**
 * Frameless tail-call thunk: forward the sub-object at arg0->field_0x40 + 0x4C
 * to func_00130C68. Void tail call → sibling-call-optimised to the original's
 * `j func_00130C68` with the +0x4C adjust in the delay slot.
 */
void func_00130AA0(void *arg0) {
    func_00130C68(*(u8 **)((u8 *)arg0 + 0x40) + 0x4C);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00130AAC);

/**
 * Kick a DMA transfer on the channel whose control word lives at 0x1000B000:
 * with interrupts disabled, set the channel's enable bit (0x10000) in the DMA
 * enable register (read 0x1000F520, write 0x1000F590), write `chcr` to the
 * channel, then clear the enable bit again; restore interrupts on the way out.
 */
void func_00130AB0(s32 chcr) {
    func_0011F5E0();
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 | 0x10000;
    *(volatile u32 *)0x1000B000 = chcr;
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 & 0xFFFEFFFF;
    func_0011F628();
}

/**
 * Identical to func_00130AB0 but kicks the DMA channel whose control word lives
 * at 0x1000B400 (channel +1): toggles the enable bit in the DMA enable register
 * around the channel `chcr` write, with interrupts disabled.
 */
void func_00130B18(s32 chcr) {
    func_0011F5E0();
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 | 0x10000;
    *(volatile u32 *)0x1000B400 = chcr;
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 & 0xFFFEFFFF;
    func_0011F628();
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00130B80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00130C68);

/**
 * Poll the GIF/PATH status word at 0x10002010 by mode: mode 0 spins until the
 * sign bit (transfer-active) clears and returns 0; mode 1 returns just that
 * sign bit (1 if active); any other mode returns 0. (`arg1` is unused — present
 * in the original signature so callers pass a second zeroed argument.)
 */
s32 func_00130DB8(s32 mode, s32 arg1) {
    s32 result = 0;
    switch (mode) {
    case 0:
        while (*(volatile s32 *)0x10002010 < 0) {
        }
        result = 0;
        break;
    case 1:
        result = (u32)*(volatile u32 *)0x10002010 >> 31;
        break;
    }
    return result;
}

/**
 * Kick the DMA channel at 0x1000B400 (same sequence as func_00130B18):
 * toggle the channel-enable bit in the DMA enable register around the `chcr`
 * write, with interrupts disabled.
 */
void func_00130E20(s32 chcr) {
    func_0011F5E0();
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 | 0x10000;
    *(volatile u32 *)0x1000B400 = chcr;
    *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 & 0xFFFEFFFF;
    func_0011F628();
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00130E88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001310C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001313C4);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", s_isnan);

/**
 * Pass the 64-bit value at arg0 + 0x8 as both arguments to func_00123028,
 * discard its result, and return 0.
 */
s32 func_00131400(s64 *arg0) {
    s64 v = arg0[1];
    func_00123028(v, v);
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131424);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_0013153C);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131540);

/**
 * Lazily initialise the global block D_001381D8 (calling func_00131540() the
 * first time, detected by its leading byte being 0), then return 1 if byte 4 of
 * the block equals 0x54 ('T'), else 0 — a region/territory check.
 */
s32 func_001315E0(void) {
    if (D_001381D8[0] == 0) {
        func_00131540();
    }
    return D_001381D8[4] == 0x54;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131620);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131680);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001316C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001316D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131720);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131728);

/**
 * Binary byte (0..99) → packed BCD (EU +0x60 twin of USA func_00131730,
 * RTC/BCD clock family): BCD(n) = n + 6*(n/10), e.g. 59 → 0x59. The u8
 * param/return produce the callee-side andi masks; divide-by-10 emits the
 * divu + beql/break zero-guard (harness break-0,7 fixup) and the native
 * 3-op `mult`.
 */
u8 func_00131790(u8 binary) {
    return binary / 10 * 6 + binary;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001317C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001317E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131848);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_001318B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131968);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131A10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131A40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131A68);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131AF8);

/* func_00131B48 = _start (USA 0x00131AE8): the EE ELF entry point — hand-written
 * crt0, NOT compilable from C and not given a portable #else (it IS the machine
 * bootstrap: a C-only boot still enters through this exact assembly before any C
 * can run). It clears all 32 GPRs (padduw), all 32 FPRs (mtc1), HI/LO/HI1/LO1/SA
 * and the FCR, zeroes the .bss span (EU D_0013C100..D_001A74F0) with 128-bit
 * `sq` stores, sets up $gp (EU D_001AF070) and $sp via the two SetMemoryMode
 * syscalls (0x3C/0x3D), then calls _InitSys, func_0011AEA0(0), enables
 * interrupts (ei) and calls main before tail-jumping to exit. Left as
 * INCLUDE_ASM by nature. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131B48);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131D08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131D10);

/* A 16-byte section header in a loaded overlay/WAD segment. The section's
 * payload immediately follows the header inline (at +0x10). */
typedef struct SectionHeader {
    void *dest;   /* 0x0 destination VA the payload is relocated to */
    s32   size;   /* 0x4 payload size in bytes */
    s32   pad8;   /* 0x8 (unused) */
    s32   key;    /* 0xC group id shared by a contiguous run of sections */
} SectionHeader;  /* 0x10 */

extern u8 *g_pLoadedSegment;

/* func_00131D18 = InstallLoadedOverlay (USA 0x00131CB8): relocate/install the
 * freshly loaded overlay segment pointed to by g_pLoadedSegment (EU data symbol
 * D_001A7308). The segment's first word is the byte offset to the first section
 * header; from there it walks consecutive 16-byte section headers, copying each
 * section's payload (which immediately follows its header) to the header's dest
 * VA — 64 bits at a time when dest, src and size are all 8-byte aligned, else
 * 32 bits at a time. It installs the run of sections that share the first
 * section's group id (key) and returns that id, stopping at the first section
 * whose id differs.
 *
 * NEAR-MISS, kept as INCLUDE_ASM for the matching build (#ifndef TARGET_NATIVE):
 * ee-gcc's delay-slot filler emits the three payload-alignment tests as ordinary
 * `bne` whereas the original uses annulling `bnel` branches that recompute
 * `dest + size` only on the taken (4-byte) path — a filler decision not
 * expressible from C source. The portable #else below is the functionally-
 * faithful rendering (cmp-oracle'd against the .s on real R5900). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131D18);
#else
s32 func_00131D18(void) {
    u8 *base = g_pLoadedSegment;
    SectionHeader *hdr = (SectionHeader *)(base + *(s32 *)base);
    s32 key = 0;

    for (;;) {
        u8 *src = (u8 *)hdr + 0x10;
        u8 *dst = (u8 *)hdr->dest;
        s32 size;

        if (key != 0) {
            if (hdr->key != key) {
                return key;
            }
        } else {
            key = hdr->key;
        }
        size = hdr->size;

        if (((size & 7) == 0) && (((u32)src & 7) == 0) && (((u32)dst & 7) == 0)) {
            /* dest, src and size all 8-byte aligned: copy 64 bits at a time */
            u64 *d = (u64 *)dst;
            u64 *s = (u64 *)src;
            u64 *end = (u64 *)(dst + size);
            while (d != end) {
                *d = *s;
                d++;
                s++;
            }
        } else {
            /* otherwise copy 32 bits at a time */
            s32 *d = (s32 *)dst;
            s32 *s = (s32 *)src;
            s32 *end = (s32 *)(dst + size);
            while (d != end) {
                *d = *s;
                d++;
                s++;
            }
        }
        hdr = (SectionHeader *)(src + size);
    }
}
#endif

extern void LoadLevelAndInitHealth(void);
extern s32 func_00131D18(void);

/* func_00131DF8 = main (USA 0x00131D98): the game's top-level loop. Runs the
 * one-shot init func_0011FC48 once, then loops forever: call the current stage
 * routine (initially the level loader LoadLevelAndInitHealth), install the
 * overlay segment it loaded (func_00131D18 = InstallLoadedOverlay) and adopt
 * that call's returned id as the next stage routine to run, then pump the frame
 * twice via func_0011AEA0 (modes 0 and 2). Never returns.
 *
 * NEAR-MISS, kept as INCLUDE_ASM for the matching build (#ifndef TARGET_NATIVE):
 * the body is otherwise byte-exact, but the original fills the first
 * func_0011AEA0(0) call's delay slot with the func_00131D18-return capture
 * (`move s0,v0`) and emits the `a0=0` arg setup standalone, whereas ee-gcc fills
 * that delay slot with the closest arg setup (`a0=0`). That is a delay-slot
 * filler tie-break no C statement ordering can change. The portable #else below
 * is the C entry. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00131DF8);
#else
/* The C entry routine compiles to the symbol `main` via an asm label rather than
 * being literally named `main` so cc1 does not inject the `jal __main` ctor hook
 * (the original is built freestanding and has no such call). */
void GameMain(void) __asm__("main");
void GameMain(void) {
    void (*stage)(void);

    func_0011FC48();
    stage = LoadLevelAndInitHealth;
    for (;;) {
        stage();
        stage = (void (*)(void))func_00131D18();
        func_0011AEA0(0);
        func_0011AEA0(2);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", snd_Init);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", snd_Pump);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/015180", func_00132270);


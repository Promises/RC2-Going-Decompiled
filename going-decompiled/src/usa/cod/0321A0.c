#include "common.h"

/*
 * cod/0321A0 — the 989snd EE-side command/bank TU (carved out of cod/015180
 * on 2026-06-10). This unit is an original separate translation unit built at
 * nonzero -G: its small-data globals (sdata cluster 0x1A7480..0x1A74F8 plus
 * 0x1A7180/0x1A7210) are accessed uniformly via %gp_rel($gp), which -G0 cannot
 * express. The matcher builds THIS unit at -O2 -G8 (see the per-unit override
 * in tools/ee/objdiff_build.sh / diff.sh); every other unit stays -O2 -G0.
 *
 * -G8 rule for externs in this file: a complete extern object of size <= 8
 * bytes is placed in small data (gp-relative access); anything that the
 * original accesses with absolute %hi/%lo pairs must be declared with an
 * incomplete array type (extern T sym[];) so cc1 cannot prove it small.
 */

extern s32 snd_QueueCommandToRing(s32 sel, s32 count, void *data, s32 arg3, s32 arg4);
extern s32 snd_SendCommandSync(s32 arg0, s32 arg1, void *arg2);
/* sceSifCallRpc-shaped SIF RPC primitive: client, fn, mode, send buf/size,
 * recv buf/size, end callback/param. Shared by the 989snd RPC senders below. */
extern s32 func_0011D620(void *rpc, s32 fno, s32 mode, void *sbuf, s32 ssize,
                         void *rbuf, s32 rsize, void *endfn, s32 endpar);
extern s32 snd_Pump(void);
extern s32 func_00125588(void);
extern s32 func_00125620(void);
extern void *func_001245D0(void *arg0);

/* 989snd small-data globals (gp-relative in the original; complete <=8-byte
 * declarations so -G8 places them in small data). */
extern s32 D_001A74C4;   /* command-ring service-pending flag */
extern s32 *D_001A74A0[2]; /* double-buffered ring entry-count pointers */
extern s32 D_001A74C0;   /* active ring buffer index (0/1) */
extern s32 g_sndIopReady; /* nonzero when the IOP sound/loader driver is up */
extern void *D_001A7490; /* snd_Pump tick callback (set by SetSndPumpCallback) */

extern void func_0011AEA0(s32 arg);            /* pre-RPC flush/sync */
extern s32  sceSifCheckStatRpc(void *rpc);     /* nonzero while the RPC is busy */
extern void snd_PrintError(const char *msg, ...); /* printf-style; some call sites pass %d args */
/* D_001A7040 is g_sndRpcClientCmd, the command-channel sceSif RPC client. The
 * original compiler treated it as small data while the assembler placed it
 * absolutely, so its address is loaded with an unsplit `la` macro: lui/addiu
 * ahead of the jal, and a nop left in the delay slot. The complete <=8-byte
 * declaration makes cc1 emit that `la`; the `.extern ,16` override (first
 * directive wins) makes the assembler expand it absolutely. Same model as
 * text/198FA0's g_guiInstance. */
__asm__(".extern D_001A7040, 16");
extern u8   D_001A7040[8];
extern s32  D_001A74F8;    /* suppresses the completion-mismatch error print */
extern char D_0013BFA0[];  /* "RPC completion mismatch" error string */
extern u8  *D_001A7480;    /* active DMA-transfer buffer */
extern s32  D_001A7484;    /* active DMA-transfer entry count */

/**
 * snd_ServiceRpcCompletion - poll the 989snd command channel's in-flight DMA
 * transfer.
 *
 * Returns 1 when no transfer is in flight, or when the reply is complete: the
 * IOP has written the 0xFFFFFFFF terminator both to the buffer's first word and
 * to the word after the D_001A7484 entries, in which case the in-flight pointer
 * is cleared. Returns 0 while the RPC is still busy, or when it has finished
 * without both terminators; the latter prints the mismatch diagnostic unless
 * D_001A74F8 suppresses it.
 */
s32 snd_ServiceRpcCompletion(void) {
    u32 *buf;

    func_0011AEA0(0);
    if (D_001A7480 == 0) {
        return 1; /* no transfer in flight */
    }
    if (sceSifCheckStatRpc(D_001A7040) != 0) {
        return 0; /* RPC still running */
    }
    buf = (u32 *)D_001A7480;
    if (buf[0] == 0xFFFFFFFF && buf[D_001A7484 + 1] == 0xFFFFFFFF) {
        D_001A7480 = 0; /* both terminators consumed -> transfer done */
        return 1;
    }
    if (D_001A74F8 == 0) {
        snd_PrintError(D_0013BFA0);
    }
    return 0;
}

/**
 * snd_SetupDmaTransfer - make `buffer` the in-flight 989snd RPC transfer.
 * Records buffer and its entry count in D_001A7480/D_001A7484, zeroes the
 * header word buffer[0] and the terminator word after the `count` entries,
 * and writes both words back to memory with func_0011B3D0 so the IOP sees
 * them. snd_ServiceRpcCompletion later waits for the IOP to set both words
 * to 0xFFFFFFFF.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, selected in
 * tools/ee/s136os_functions.txt), which packs the s0/s1/ra saves 8 bytes
 * apart as the ROM does; cc1 2.9 gives each save a 16-byte slot. The
 * terminator address is spelled twice on purpose: as a store through
 * `buffer[count + 1]` (the ROM folds the +4 into the `sw` offset) and as
 * `buffer += count * 4 + 4` for the second flush (the ROM adds 4 to count*4
 * first, then the base). One shared expression makes cc1 compute the address
 * once and drops the s1 save.
 */
extern void func_0011B3D0(void *start, void *end); /* writeback/flush a small range */
#if !defined(TARGET_NATIVE) && !defined(S136OS_snd_SetupDmaTransfer)
S136OS_SLOT(snd_SetupDmaTransfer);
#else
void snd_SetupDmaTransfer(u8 *buffer, s32 count) {
    D_001A7484 = count;
    D_001A7480 = buffer;
    ((s32 *)buffer)[count + 1] = 0; /* terminator slot after the list */
    *(s32 *)buffer = 0;             /* header slot */
    func_0011B3D0(buffer, buffer + 3);
    buffer += count * 4 + 4;
    func_0011B3D0(buffer, buffer + 3);
}
#endif

extern s32  D_001A74C8;   /* nonzero while the command ring still has pending work */
extern s32  D_001A7488;   /* last bank-load status/error code */
extern s32  D_001A71C0;   /* load-request word 0 (base of the 8-byte send buffer) */
extern s32  D_001A71C4;   /* load-request word 1 */
/* IOP load-result slot (4-byte receive buffer), preset to the 0xFFFFFFFF
 * sentinel. Unsigned because the ROM builds that preset as a 32-bit unsigned
 * constant (`lui $2,0xffff; ori $2,$2,0xffff` at 0x132520), which cc1 emits
 * only for an unsigned value; s32 gives `li $2,-1` (4/82 words in
 * snd_BankLoadAsync, 67/96 in snd_BankLoadByLoc). */
extern u32  D_001A7180;
/* g_sndRpcClientBank, the load-channel sceSif RPC client — the twin of
 * D_001A7040. Declared as a complete 8-byte object so cc1 treats it as small
 * data and prints an unsplit `la`; the `.extern ,16` below makes GNU as expand
 * that absolutely (lui/addiu at every use, as in the ROM) instead of cc1
 * splitting %hi/%lo and hoisting the %hi into a callee-saved register. */
extern u8   D_001A7140[8];
/* ADDRESSING-MODEL DEVICES (RULING #8620 term 4 / #9574, offset 0 each) for the
 * snd_BankLoad* family, ahead of every compiled use in the unit. The ROM stores
 * D_001A71C4 and D_001A74D8 through absolute `lui $1` macros and loads
 * D_001A7140's address with an absolute lui/addiu, while cc1 at -G8 sees them
 * as small data: `.extern ,16` makes GNU as expand them absolutely. The request
 * word D_001A71C0 and the result word D_001A7180 are gp-relative only inside
 * delay slots (D_001A71C0's `bnel` slots at 0x132364 and 0x1324FC;
 * D_001A7180's loads in the slots at 0x132420 and 0x132458) and absolute
 * everywhere else: size 12 is asm_unit.sh's marker for exactly that split
 * (gp-addressable but assembler-absolute). Cost of removing each alone (s136os solo TU, task
 * #1914), words of snd_BankLoadAsync /82 and snd_BankLoadByLoc /96:
 * D_001A71C4 47 / 64, D_001A7180 45 / 62 (and 16 in place of 12: 0 / 35),
 * D_001A74D8 42 / -, D_001A71C0 19 / 41, D_001A7140 32 / 53 (and its [8]
 * size 35 / 54). */
__asm__(".extern D_001A71C4, 16");
__asm__(".extern D_001A7180, 12");
__asm__(".extern D_001A74D8, 16");
__asm__(".extern D_001A71C0, 12");
__asm__(".extern D_001A7140, 16");
extern char D_001A7578[]; /* command-ring / RPC stall diagnostic string */
extern char D_001A7600[]; /* "sound system not ready" diagnostic */
extern char D_001A7630[]; /* "load already in progress" diagnostic */
extern char D_001A7650[]; /* "load RPC failed" diagnostic */
extern s32  snd_CheckLoadInProgress(s32 noWait);

/* SCHEDULING DEVICE (RULING #8435): one `noreorder` nop, ordered after the load
 * of `next` (input) and before the next use of `v` (output). It reproduces the
 * load-delay nop the ROM's assembler emitted after a polled load that cc1
 * marks `#nop` and GNU as (r5900, interlocked) drops. Same form as
 * cod/0314C0.c's macro of this name. No-op on the native arm. */
#ifndef TARGET_NATIVE
#define R5900_SHORT_LOOP_PAD1(v, next) \
    __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+r"(v) : "r"(next))
#else
#define R5900_SHORT_LOOP_PAD1(v, next) ((void)0)
#endif

/**
 * snd_BankLoadByLoc - load a sound bank over the IOP's SIF RPC load channel and
 * wait for the result.
 *
 * @arg0, @arg1: the two words of the load request (D_001A71C0/D_001A71C4, the
 *               RPC send block).
 * Returns the IOP's load result (the word it writes over the 0xFFFFFFFF
 * sentinel in D_001A7180), or 0 when the command ring still has pending work,
 * a load is already in flight, or the RPC call fails (status 0x106 in
 * D_001A7488). Each early-out prints a diagnostic unless D_001A74F8
 * suppresses it.
 *
 * Like snd_BankLoadAsync the ROM stores D_001A71C0 = arg0 in the delay slot of
 * a branch-likely (`bnel` at 0x132360), so only the path that issues the load
 * writes the request word.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, selected in
 * tools/ee/s136os_functions.txt). Devices: the addressing declarations above
 * and R5900_SHORT_LOOP_PAD1 in the poll loop. The ROM pads `lw $2,
 * D_001A7180; nop; beq $2,$16` at 0x132474..0x13247C; without the pad the body
 * is one word short (13/96, s136os solo TU, task #1914). The pad's output is
 * the sentinel register, not the polled value, so cc1 still knows $2 holds
 * D_001A7180 at the loop exit and returns it without a reload, as the ROM does.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_snd_BankLoadByLoc)
S136OS_SLOT(snd_BankLoadByLoc);
#else
s32 snd_BankLoadByLoc(s32 arg0, s32 arg1) {
    D_001A7488 = 0;

    if (D_001A74C8 != 0) {
        /* command ring still busy: can't start a load */
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7600);
        }
        return 0;
    }

    if (snd_CheckLoadInProgress(1) == 1) {
        /* a load is already in flight */
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7630);
        }
        return 0;
    }

    /* stage the request and mark the result slot pending */
    D_001A71C0 = arg0;
    D_001A71C4 = arg1;
    D_001A7180 = 0xFFFFFFFF;

    /* wait for the load RPC channel to go idle */
    while (sceSifCheckStatRpc(D_001A7140) != 0) {
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7578);
        }
        snd_Pump();
        func_0011AEA0(0);
    }

    /* issue the bank-load RPC (function 3); a negative return means it failed */
    if (func_0011D620(D_001A7140, 3, 1, &D_001A71C0, 8, &D_001A7180, 4, 0, 0) < 0) {
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7650);
        }
        D_001A7488 = 0x106;
        return 0;
    }

    /* block until the IOP overwrites the sentinel with the load result */
    if (D_001A7180 == 0xFFFFFFFF) {
        u32 pending = 0xFFFFFFFF;
        u32 result;
        do {
            func_0011AEA0(0);
            result = D_001A7180;
            R5900_SHORT_LOOP_PAD1(pending, result);
        } while (result == pending);
    }
    return D_001A7180;
}
#endif

extern s32 D_001A74D0;  /* async completion context word (arg2) */
extern s64 D_001A74D8;  /* async completion context qword (arg3, 64-bit) */

/**
 * snd_BankLoadAsync - fire-and-forget variant of snd_BankLoadByLoc.
 *
 * @arg0, @arg1: the two words of the load request (D_001A71C0/D_001A71C4,
 *               the RPC send block).
 * @arg2, @arg3: the async completion context, kept in D_001A74D0 and (sign-
 *               extended to 64 bits) D_001A74D8 for the bank-channel callback.
 * Returns nothing.
 *
 * Clears the last bank-load status, then gives up (printing a diagnostic unless
 * D_001A74F8 suppresses it) when the command ring still has pending work or a
 * load is already in flight. Otherwise stages the request, presets the result
 * word to the 0xFFFFFFFF "pending" sentinel, waits (pumping the sound engine)
 * until the load RPC channel is idle, marks a load pending (D_001A74C8 = 1) and
 * issues load RPC function 3 without blocking on the result.
 *
 * The ROM stores D_001A71C0 = arg0 in the delay slot of a branch-likely
 * (`bnel` at 0x1324F8), so the store happens only on the path that issues the
 * load; cc1 makes that choice itself from this shape.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, selected in
 * tools/ee/s136os_functions.txt). Devices: the addressing declarations above
 * snd_BankLoadByLoc, and the empty volatile fence (RULING #8483) after the last
 * staging store. The fence keeps the loop's hoisted `lui $16,%hi(D_001A7578)`
 * below the stores, so it is the instruction cc1 moves into the `b` delay slot
 * at 0x132544 as the ROM has it; without the fence sched1 lifts the lui above
 * the D_001A74D0 store, the `sd` to D_001A74D8 takes the slot, and asm_unit.sh
 * hoists its absolute expansion back out leaving a nop (5/82 words, s136os solo
 * TU, task #1914). Spelling the sentinel 0xFFFFFFFF or -1 makes no difference;
 * D_001A7180's u32 type does.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_snd_BankLoadAsync)
S136OS_SLOT(snd_BankLoadAsync);
#else
void snd_BankLoadAsync(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    D_001A7488 = 0;

    if (D_001A74C8 != 0) {
        /* command ring still busy: can't start a load */
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7600);
        }
        return;
    }

    if (snd_CheckLoadInProgress(1) == 1) {
        /* a load is already in flight */
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7630);
        }
        return;
    }

    /* stage the request, the result sentinel and the async completion context */
    D_001A71C0 = arg0;
    D_001A71C4 = arg1;
    D_001A7180 = 0xFFFFFFFF;
    D_001A74D0 = arg2;
    D_001A74D8 = arg3;
    __asm__ __volatile__(""); /* scheduling device, see the doc comment */

    /* wait for the load RPC channel to go idle */
    while (sceSifCheckStatRpc(D_001A7140) != 0) {
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7578);
        }
        snd_Pump();
        func_0011AEA0(0);
    }

    /* mark a load pending and issue the RPC without blocking on the result */
    D_001A74C8 = 1;
    func_0011D620(D_001A7140, 3, 1, &D_001A71C0, 8, &D_001A7180, 4, 0, 0);
}
#endif

/* func_001325E0: 4 bytes of inter-function fill (`addiu sp,0x20`) before the
 * unrecoverable snd_BankLoadFromEE_CB body at 0x1325E8 (reached only by
 * fallthrough/data-ref, so spimdisasm emits no .s for it — see STATUS). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001325E0);

/* func_001325E8 = snd_BankLoadFromEE_CB (0x1325E8): recovered splat-dropped
 * function (spimdisasm emitted no .s — reached by fallthrough/data-ref).
 * Byte-exact; recovers the 0xF0 that shifted cod rodata/jtbls -0xF0. A raw .word
 * dump while it was func_001325E8; decoded instructions since the rename (task #1255). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", snd_BankLoadFromEE_CB);

extern char D_001A7730[]; /* "sound system not ready" diagnostic */
extern char D_001A7760[]; /* "IOP load RPC failed" diagnostic */

/**
 * snd_BankLoadFromIOP - load a sound bank that is already resident on the IOP
 * (load RPC function 0x59, one 4-byte argument) and wait for the result.
 *
 * @arg0: the request word, sent from D_001A71C0.
 * Returns the IOP's load result (the word it writes over the 0xFFFFFFFF
 * sentinel in D_001A7180), or 0 when the command ring still has pending work or
 * the RPC call fails (status 0x106 in D_001A7488); each early-out prints a
 * diagnostic unless D_001A74F8 suppresses it. The simplest of the load family:
 * no snd_CheckLoadInProgress gate and no branch-likely.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, selected in
 * tools/ee/s136os_functions.txt), with snd_BankLoadByLoc's devices: the
 * addressing declarations above snd_BankLoadByLoc and R5900_SHORT_LOOP_PAD1 in
 * the poll loop, which reproduces the ROM's `lw $2,D_001A7180; nop; beq` at
 * 0x1327F4..0x1327FC (without it 11/79 words, s136os solo TU, task #1914).
 * The shared declarations, each removed alone (words /79): D_001A7180's
 * `.extern` 37 (16 in place of 12: 62), D_001A71C0's 63, D_001A7140's 51 (its
 * [8] size 38), D_001A7180 as s32 4; D_001A71C4's and D_001A74D8's are unused
 * here and read 0.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_snd_BankLoadFromIOP)
S136OS_SLOT(snd_BankLoadFromIOP);
#else
s32 snd_BankLoadFromIOP(s32 arg0) {
    D_001A7488 = 0;

    if (D_001A74C8 != 0) {
        /* command ring still busy: can't start a load */
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7730);
        }
        return 0;
    }

    /* stage the single-word request and mark the result slot pending */
    D_001A71C0 = arg0;
    D_001A7180 = 0xFFFFFFFF;

    /* wait for the load RPC channel to go idle */
    while (sceSifCheckStatRpc(D_001A7140) != 0) {
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7578);
        }
        snd_Pump();
        func_0011AEA0(0);
    }

    /* issue the IOP-side bank-load RPC (function 0x59); negative means failure */
    if (func_0011D620(D_001A7140, 0x59, 1, &D_001A71C0, 4, &D_001A7180, 4, 0, 0) < 0) {
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7760);
        }
        D_001A7488 = 0x106;
        return 0;
    }

    /* block until the IOP overwrites the sentinel with the load result */
    if (D_001A7180 == 0xFFFFFFFF) {
        u32 pending = 0xFFFFFFFF;
        u32 result;
        do {
            func_0011AEA0(0);
            result = D_001A7180;
            R5900_SHORT_LOOP_PAD1(pending, result);
        } while (result == pending);
    }
    return D_001A7180;
}
#endif

/* func_00132818: 0x10 bytes of inter-function padding split off by symbol_addrs
 * size:0x10; the real wrapper begins at func_00132828. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132818);

/**
 * Invoke snd_QueueCommandToRing with selector 8 and no payload, forwarding its
 * return value. (Returning the callee's value - rather than a void call - is
 * what keeps ee-gcc from sibling-call-optimising this into `j ...`; the
 * original keeps the frame and uses jal+jr, proving the original wrappers are
 * value-returning.)
 */
s32 func_00132828(void) {
    return snd_QueueCommandToRing(8, 0, 0, 0, 0);
}

/**
 * Invoke snd_QueueCommandToRing with selector 6, count 4, arg0 passed by
 * address in a stack local, and zero for the two trailing arguments.
 */
void func_00132858(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(6, 4, &value, 0, 0);
}

/**
 * Invoke snd_QueueCommandToRing with selector 9, count 8, and a stack record
 * holding arg0 and arg1; the two trailing arguments are zero.
 */
void func_00132888(s32 arg0, s32 arg1) {
    s32 args[2];
    args[0] = arg0;
    args[1] = arg1;
    snd_QueueCommandToRing(9, 8, args, 0, 0);
}

/**
 * Queue 989snd command 0x60 with a 0x1C-byte record: word 0 is cmd, followed
 * by either a copy of the 0x18-byte *payload or, when payload is NULL, a single
 * -1 sentinel word (the rest of the record is left uninitialised).
 *
 * payload: 0x18 bytes, no alignment assumed. Returns snd_QueueCommandToRing's
 * result.
 *
 * The payload lands at record offset 4, which is not doubleword-aligned, so
 * the ROM copies it with three unaligned ldl/ldr + sdl/sdr pairs. ee-gcc 2.9
 * emits exactly that from the constant-size memcpy below: this body is the
 * shipped one, byte-exact at -O2 -G8 (the unit's flags).
 */
s32 func_001328C0(s32 cmd, const void *payload) {
    s32 record[8];
    record[0] = cmd;
    if (payload != 0) {
        memcpy((u8 *)record + 4, payload, 0x18);
    } else {
        record[1] = -1;
    }
    return snd_QueueCommandToRing(0x60, 0x1C, record, 0, 0);
}

/**
 * Invoke snd_QueueCommandToRing with selector 0xB, count 4, arg0 passed by
 * address in a stack local, and zero for the two trailing arguments.
 */
void func_00132938(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0xB, 4, &value, 0, 0);
}

/* func_00132968: 0x10 bytes of inter-function padding (`addiu sp,0x10; nop`
 * pairs) that splat's auto-detection grouped as a standalone symbol. The real
 * wrapper body begins at func_00132978; an explicit size:0x10 in symbol_addrs
 * keeps this padding split off so the wrapper can match. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132968);

/**
 * Invoke snd_QueueCommandToRing with selector 0xD, count 8, and a stack record
 * holding arg0 and arg1; the two trailing arguments are zero. (Real start of
 * the symbol splat previously mis-split as func_00132968 — see that note.)
 */
void func_00132978(s32 arg0, s32 arg1) {
    s32 args[2];
    args[0] = arg0;
    args[1] = arg1;
    snd_QueueCommandToRing(0xD, 8, args, 0, 0);
}

/**
 * Invoke snd_QueueCommandToRing with selector 0x4E, count 0xC, and a stack
 * record holding arg0, arg1 and arg2; the two trailing arguments are zero.
 */
void func_001329B0(s32 arg0, s32 arg1, s32 arg2) {
    s32 args[3];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    snd_QueueCommandToRing(0x4E, 0xC, args, 0, 0);
}

/* func_001329F0: 0x20 bytes of inter-function padding split off by symbol_addrs
 * size:0x20; the real wrapper begins at func_00132A10. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001329F0);

/**
 * Invoke snd_QueueCommandToRing with selector 0x11, count 0x18, and a 6-word
 * stack record holding arg0..arg5; arg6 and arg7 are forwarded as the two
 * trailing arguments.
 */
void func_00132A10(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
                   s32 arg6, s32 arg7) {
    s32 args[6];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    args[4] = arg4;
    args[5] = arg5;
    snd_QueueCommandToRing(0x11, 0x18, args, arg6, arg7);
}

/* func_00132A58: 0x18 bytes of inter-function padding split off by symbol_addrs
 * size:0x18; the real wrapper begins at func_00132A70. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132A58);

/**
 * Invoke snd_QueueCommandToRing with selector 0x15, count 4, arg0 passed by
 * address in a stack local, and zero for the two trailing arguments.
 */
void func_00132A70(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x15, 4, &value, 0, 0);
}

/* func_00132AA0: 0x28 bytes of inter-function padding split off by symbol_addrs
 * size:0x28; the real wrapper begins at func_00132AC8. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132AA0);

/**
 * Invoke snd_QueueCommandToRing with selector 0x18 and no payload, forwarding
 * its return value (the value-return keeps the frame, see func_00132828).
 */
s32 func_00132AC8(void) {
    return snd_QueueCommandToRing(0x18, 0, 0, 0, 0);
}

/**
 * Invoke snd_QueueCommandToRing with selector 0x16, count 4, arg0 passed by
 * address in a stack local, and zero for the two trailing arguments.
 */
void func_00132AF8(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x16, 4, &value, 0, 0);
}

/**
 * Invoke snd_QueueCommandToRing with selector 0x17, count 4, arg0 passed by
 * address in a stack local, and zero for the two trailing arguments.
 */
void func_00132B28(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x17, 4, &value, 0, 0);
}

/**
 * Invoke snd_QueueCommandToRing with selector 0x19, count 4, arg0 passed by
 * address in a stack local, and arg1/arg2 forwarded as the two trailing
 * arguments.
 */
void func_00132B58(s32 arg0, s32 arg1, s32 arg2) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x19, 4, &value, arg1, arg2);
}

/* func_00132B88: 0x38 bytes of inter-function padding split off by symbol_addrs
 * size:0x38; the real wrapper begins at func_00132BC0. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132B88);

/**
 * func_00132BC0 is snd_SetVoiceVolumeRamp: the 989snd EE command-ring wrapper
 * for cmd opcode 0x21 (set voice param / volume ramp). (Kept as func_ here
 * because renaming a matched function would desync its frozen nonmatchings .s
 * glabel.) Queues the command via snd_QueueCommandToRing with count 0x18 and a
 * 6-word stack record holding arg0..arg5; arg6/arg7 are the two trailing args.
 */
void func_00132BC0(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
                   s32 arg6, s32 arg7) {
    s32 args[6];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    args[4] = arg4;
    args[5] = arg5;
    snd_QueueCommandToRing(0x21, 0x18, args, arg6, arg7);
}

/* func_00132C08: 0x40 bytes of inter-function fill (addiu $sp,+N / nop pairs)
 * before snd_SendCommandSync at 0x132C48. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132C08);

/* g_sndCmdRpcSend / g_sndCmdRpcRecv (+ its reply word at +4), the command
 * channel's RPC send and receive blocks. The ROM forms each address with an
 * absolute lui/addiu at every use and loads the reply word through the
 * unsplit `lui $16; lw $16` macro, i.e. cc1-small / assembler-absolute like
 * D_001A7040: complete small declarations so cc1 prints unsplit macros, and
 * `.extern ,16` (RULING #8620 term 4, offset 0) so GNU as expands them
 * absolutely. With incomplete arrays cc1 splits %hi/%lo and hoists the %hi
 * into a callee-saved register. Cost in snd_SendCommandSync of removing each
 * alone (words /105, s136os solo TU, task #1914): D_001A70C0's .extern 91 and
 * [8] 22, D_001A7080's .extern 72 and [8] 71, D_001A7084's .extern 19 and
 * [1] 9. */
__asm__(".extern D_001A70C0, 16");
__asm__(".extern D_001A7080, 16");
__asm__(".extern D_001A7084, 16");
extern u8  D_001A70C0[8];  /* RPC command parameter byte buffer */
extern u8  D_001A7080[8];  /* DMA send / RPC receive block */
extern s32 D_001A7084[1];  /* RPC reply status word (receive block + 4) */
extern void snd_FlushCommandRing(void);

/* SCHEDULING DEVICE (RULING #8435): three `noreorder` nops, ordered after the
 * call that produces `v` and before its test. Two spin loops in the ROM read
 * `jal <poll>; nop; nop; nop; nop; bnez/beqz $2` — the delay slot plus three
 * nops cc1 never emits: snd_SendCommandSync's at 0x132D80..0x132D94 and
 * func_00133250's at 0x133298..0x1332AC. This reproduces those three. No-op on
 * the native arm. */
#ifndef TARGET_NATIVE
#define R5900_SPIN_PAD3(v) \
    __asm__(".set noreorder\n\tnop\n\tnop\n\tnop\n\t.set reorder" : "+r"(v))
#else
#define R5900_SPIN_PAD3(v) ((void)0)
#endif

/**
 * snd_SendCommandSync - send one 989snd command over the SIF RPC command
 * channel and wait for its reply.
 *
 * @fno:      the RPC function number (the command).
 * @count:    number of parameter bytes; 0 sends no buffer.
 * @cmdBytes: the parameter bytes, copied into g_sndCmdRpcSend.
 * Returns the reply status word the IOP writes at g_sndCmdRpcRecv + 4.
 *
 * Drains any in-flight command-ring transfer, kicks the receive block's DMA
 * set-up, waits (pumping, with a diagnostic unless D_001A74F8 suppresses it)
 * until the channel is idle, issues the RPC and spins until
 * snd_ServiceRpcCompletion confirms the reply. Before returning it flushes the
 * active command ring when that ring holds entries and no service is pending.
 * func_0011D620 is the sceSifCallRpc-shaped primitive (client, fno, mode, send
 * buf/size, recv buf/size, end callback/param).
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, selected in
 * tools/ee/s136os_functions.txt). Devices: the declarations above,
 * R5900_SPIN_PAD3 in the completion spin (24/105 words without it, s136os solo
 * TU, task #1914), and an empty tied fence on the reply word, which keeps its
 * load ahead of the ring test as the ROM issues it (0x132DA0) rather than
 * sunk into the `beqz` delay slot (20/105 without it).
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_snd_SendCommandSync)
S136OS_SLOT(snd_SendCommandSync);
#else
s32 snd_SendCommandSync(s32 fno, s32 count, void *cmdBytes) {
    s32 i;

    /* copy the command's parameter bytes into the shared RPC send buffer */
    for (i = 0; i < count; i++) {
        D_001A70C0[i] = ((u8 *)cmdBytes)[i];
    }

    /* drain any in-flight command-ring DMA before reusing the channel */
    while (D_001A7480 != 0) {
        snd_Pump();
        func_0011AEA0(0);
    }

    /* kick the DMA transfer, then wait for the SIF RPC channel to go idle */
    snd_SetupDmaTransfer(D_001A7080, 1);
    while (sceSifCheckStatRpc(D_001A7040) != 0) {
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7578);
        }
        snd_Pump();
        func_0011AEA0(0);
    }

    /* issue the synchronous RPC (mode 1); a zero-length command sends no buffer */
    if (count != 0) {
        func_0011D620(D_001A7040, fno, 1, D_001A70C0, count, D_001A7080, 0xC, 0, 0);
    } else {
        func_0011D620(D_001A7040, fno, 1, 0, 0, D_001A7080, 0xC, 0, 0);
    }

    /* spin until the completion service confirms the reply landed */
    {
        s32 done;

        do {
            done = snd_ServiceRpcCompletion();
            R5900_SPIN_PAD3(done);
        } while (done == 0);
    }

    {
        s32 result = D_001A7084[0]; /* the reply status word */

        __asm__("" : "+r"(result)); /* scheduling device, see the doc comment */
        /* if the active ring buffer still holds queued entries and no service
         * is pending, flush it now */
        if (*D_001A74A0[D_001A74C0] != 0 && D_001A74C4 == 0) {
            snd_FlushCommandRing();
        }
        return result;
    }
}
#endif

/* snd_QueueCommandToRing: append one command to the active double-buffered
 * 989snd ring (or, when idle with an empty command, issue a bare sync RPC). Not
 * matched — on cc1 2.9 the multi-callee-save frame hits the same 16-byte save-slot
 * layout wall as snd_SetupDmaTransfer, itself now matched on the s136os arm (near-miss).
 * Portable #else body (no branch-likely
 * delay slots; all control flow is plain bnez/beqz/bne/beq/blez/b).
 *
 * Ring model (buffer i = D_001A74C0): D_001A74A0[i] -> a 0x1000-byte command
 * buffer whose leading word is the entry count and whose remaining bytes hold
 * packed (u16 sel, u16 count, payload...) records; D_001A74A8[i] is the bytes of
 * free space left (write cursor = base + 0x1000 - free); D_001A74B0[i] -> a
 * parallel array of 16-byte descriptors (word0 = arg3, qword@8 = arg4). A buffer
 * caps at 0x100 entries. arg4 is a 32-bit parameter here (the shared decl) but
 * lands in the descriptor as a sign-extended 64-bit qword, matching the original.
 * D_001A74C4 is a "service pending" flag briefly borrowed (cleared then restored)
 * while spinning for space.
 * Save stride re-measured on the s136os arm (SN 1.36 -fopt-stack): saves land at the ROM's
 * 8-byte stride, but the saved register set/count differs from the ROM's (NOTE #9871) - an
 * allocation/frame difference, not the stride. So the save-slot wall named above is a cc1 2.9
 * property and was measured false as the reason this member stays unmatched (FACT #9873).
 * Residual: the saved-register set; the rest UNMEASURED. */
extern s32   snd_CommitRingEntry(void);
extern s32   D_001A74A8[2]; /* per-buffer free space in bytes */
extern u8   *D_001A74B0[2]; /* per-buffer 16-byte ring-entry descriptor arrays */
extern char  D_0013BFF8[];  /* ring-wrap diagnostic format string */
extern char  D_001A78D0[];  /* ring-stall diagnostic format string */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", snd_QueueCommandToRing);
#else
s32 snd_QueueCommandToRing(s32 sel, s32 count, void *data, s32 arg3, s32 arg4) {
    s32 idx;
    s32 alignedSize;
    s32 raisedFlag = 0; /* set when we transiently cleared the service flag */
    s32 spins = 0;      /* ring-full retry counter (drives the stall diagnostics) */
    s32 i;
    u8 *cursor;
    u8 *entry;

    /* idle with an empty command: nothing to queue, so just issue a bare
     * synchronous RPC (selector as the RPC function number) */
    if (D_001A74C4 == 0 && D_001A7480 == 0 && count == 0 && arg3 == 0) {
        snd_SetupDmaTransfer(D_001A7080, 1);
        while (sceSifCheckStatRpc(D_001A7040) != 0) {
            if (D_001A74F8 == 0) {
                snd_PrintError(D_001A7578);
            }
            snd_Pump();
            func_0011AEA0(0);
        }
        return func_0011D620(D_001A7040, sel, 1, 0, 0, D_001A7080, 0xC, 0, 0);
    }

    /* bytes this command occupies: payload rounded up to a multiple of 4, plus
     * the 4-byte (sel, count) header */
    alignedSize = ((count + 3) & ~3) + 4;

    /* wait until the active buffer has room and isn't at its 0x100-entry cap,
     * pumping the sound system while we spin */
    idx = D_001A74C0;
    if (*D_001A74A0[idx] == 0x100 || D_001A74A8[idx] < alignedSize) {
        for (;;) {
            if (D_001A74C4 != 0) {
                D_001A74C4 = 0;
                raisedFlag = 1;
            }
            snd_Pump();
            if (spins == 1 && D_001A74F8 == 0) {
                idx = D_001A74C0;
                snd_PrintError(D_0013BFF8, idx, *D_001A74A0[idx]);
            }
            idx = D_001A74C0;
            if (*D_001A74A0[idx] == 0x100) {
                spins++;
                continue;
            }
            if (D_001A74A8[idx] < alignedSize) {
                continue;
            }
            break;
        }
    }
    if (spins != 0 && D_001A74F8 == 0) {
        snd_PrintError(D_001A78D0, spins);
    }
    if (raisedFlag != 0) {
        D_001A74C4 = 1; /* restore the service flag we borrowed */
    }

    /* append the command bytes at the buffer's write cursor: 2-byte selector,
     * 2-byte length, then the payload */
    idx = D_001A74C0;
    cursor = (u8 *)D_001A74A0[idx] + (0x1000 - D_001A74A8[idx]);
    *(u16 *)cursor = sel;
    cursor += 2;
    *(u16 *)cursor = count;
    cursor += 2;
    for (i = 0; i < count; i++) {
        cursor[i] = ((u8 *)data)[i];
    }

    /* consume the space, write the parallel 16-byte descriptor at the next free
     * slot (arg3 word + arg4 qword), and commit */
    D_001A74A8[idx] -= alignedSize;
    entry = D_001A74B0[idx] + *D_001A74A0[idx] * 16;
    *(s32 *)entry = arg3;
    *(s64 *)(entry + 8) = arg4;
    return snd_CommitRingEntry();
}
#endif

/**
 * snd_CommitRingEntry - commit one queued command to the active 989snd ring
 * buffer: bump the entry count of the ring selected by D_001A74C0 (through the
 * per-buffer count pointers in D_001A74A0) and run snd_Pump to service it,
 * forwarding snd_Pump's return value.
 */
s32 snd_CommitRingEntry(void) {
    s32 *entryCount = D_001A74A0[D_001A74C0];
    *entryCount += 1;
    return snd_Pump();
}

/**
 * snd_FlushCommandRing - DMA-kick and SIF-RPC flush of the active 989snd
 * command ring buffer, then flip to the other buffer and reset it.
 *
 * The ring is double-buffered by D_001A74C0 (0/1): D_001A74A0[i] points at a
 * buffer whose leading word is its entry count, D_001A74B8[i] is that buffer's
 * DMA/receive area, D_001A74A8[i] tracks its remaining free space (reset to
 * 0xFFC). Waits for the command channel D_001A7040 to go idle (reporting
 * D_001A7578 unless D_001A74F8 silences it), then fires RPC function 0x4D in
 * mode 1 (NOWAIT): send the used part of the buffer, receive into the DMA
 * area, (count * 4) + 8 bytes, no end callback. func_0011D620 is the
 * sceSifCallRpc-shaped primitive (client, fno, mode, send buf/size, recv
 * buf/size, end callback/param). No parameters, no return value.
 *
 * Compiled by the s136os arm (tools/ee/s136os_functions.txt), which packs the
 * callee saves 8 bytes apart as the ROM does. The flip is spelled through a
 * fresh local `next` (`D_001A74C0 != 1`) because the earlier
 * `(D_001A74C0 ^ 1) != 0 ? 1 : 0` allocates the tail in $2 where the ROM has
 * $3 (9 of 69 words, NOTE #9892). The ROM's empty slot after the loop-entry
 * `b` (`addiu $16,$gp,%gp_rel(D_001A74A8); b; nop`) is not C: GNU as would
 * swap cc1's 1-word `la` into that slot, and asm_unit.sh's small-`la` slot pin
 * (RULING #9966) keeps it out, as the SN ee-as did. Without that rule the
 * member is 67 words, not 69. No device in the body.
 */
extern u8  *D_001A74B8[2]; /* per-buffer DMA/receive buffers */
/* D_001A74A8, D_001A7578 and func_0011D620 are declared above. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_snd_FlushCommandRing)
S136OS_SLOT(snd_FlushCommandRing);
#else
void snd_FlushCommandRing(void) {
    s32 idx = D_001A74C0;

    /* kick the DMA transfer for the buffer currently being filled */
    snd_SetupDmaTransfer(D_001A74B8[idx], *D_001A74A0[idx]);

    /* wait for the SIF RPC command channel to go idle */
    while (sceSifCheckStatRpc(D_001A7040) != 0) {
        if (D_001A74F8 == 0) {
            snd_PrintError(D_001A7578);
        }
        func_0011AEA0(0);
    }

    /* fire the ring-flush RPC (function 0x4D) for the active buffer */
    idx = D_001A74C0;
    func_0011D620(D_001A7040, 0x4D, 1,
                  D_001A74A0[idx],              /* send buffer    */
                  0x1000 - D_001A74A8[idx],     /* send size      */
                  D_001A74B8[idx],              /* receive buffer */
                  (*D_001A74A0[idx] << 2) + 8,  /* receive size   */
                  0, 0);                        /* no completion callback */

    /* flip to the other buffer and reset it for refilling */
    {
        s32 next = D_001A74C0 != 1;
        D_001A74C0 = next;
        *D_001A74A0[next] = 0;
        D_001A74A8[next] = 0xFFC;
    }
}
#endif

/*
 * func_00133220: raise the command-ring service-pending flag D_001A74C4 and
 * return 1.
 *
 * The ROM stores the return register itself (`li $2,1; jr $31; sw $2,...`).
 * Written plainly, cc1 2.9 materialises the constant twice (`li $3,1; li $2,1`)
 * and scored 63.33 (#855; engine96 read 100.00 but would need a MATCH_ guard).
 * Holding the value in $2 behind a tied empty asm makes it a single non-constant
 * value used by both the store and the return: byte-exact on sdk29, so the
 * function is in the shipped image (task #948).
 */
s32 func_00133220(void) {
#ifndef TARGET_NATIVE
    register s32 raised __asm__("$2") = 1;
    __asm__("" : "+r"(raised));
#else
    s32 raised = 1;
#endif
    D_001A74C4 = raised;
    return raised;
}

/**
 * Clear the command-ring service-pending flag D_001A74C4 (set by
 * func_00133220) and run one snd_Pump service pass, forwarding its return
 * value.
 */
s32 func_00133230(void) {
    D_001A74C4 = 0;
    return snd_Pump();
}

extern s32 snd_SendCommandSync(s32 sel, s32 count, void *data);
extern s32 snd_CheckLoadInProgress(s32 noWait);
extern s32 D_001A74C8; /* nonzero while the command ring still has pending work */

/**
 * func_00133250 - bring up the IOP sound/loader driver.
 *
 * @arg0..@arg3: the four words of the 0x2A (bring-up) command record.
 * Returns 0 when the driver is already up (g_sndIopReady == 1); otherwise
 * drains any pending command-ring work (pumping until snd_Pump reports idle),
 * waits out any in-flight load, sends command 0x2A synchronously and stores its
 * result as the new g_sndIopReady, which it also returns.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, selected in
 * tools/ee/s136os_functions.txt), which packs the s0-s3/ra saves 8 bytes apart
 * as the ROM does. Its one device is R5900_SPIN_PAD3 in the drain loop: without
 * it the body is three words short (24/43 words, s136os solo TU, task #1914).
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00133250)
S136OS_SLOT(func_00133250);
#else
s32 func_00133250(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 cmd[4];

    if (g_sndIopReady == 1) {
        return 0; /* IOP driver already up */
    }
    if (D_001A74C8 != 0) {
        s32 busy;

        do { /* drain the pending command ring */
            busy = snd_Pump();
            R5900_SPIN_PAD3(busy);
        } while (busy != 0);
    }
    snd_CheckLoadInProgress(0); /* block until any in-flight load finishes */
    cmd[0] = arg0;
    cmd[1] = arg1;
    cmd[2] = arg2;
    cmd[3] = arg3;
    g_sndIopReady = snd_SendCommandSync(0x2A, 0x10, cmd); /* 0x2A = bring-up */
    return g_sndIopReady;
}
#endif

/* func_00133300: 0x10 bytes of inter-function padding split off by symbol_addrs
 * size:0x10; the real wrapper begins at func_00133310. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133300);

/**
 * Invoke snd_QueueCommandToRing with selector 0x34 and no payload, forwarding
 * its return value (the value-return keeps the frame, see func_00132828).
 */
s32 func_00133310(void) {
    return snd_QueueCommandToRing(0x34, 0, 0, 0, 0);
}

/* func_00133340: 0x10 bytes of inter-function padding split off by symbol_addrs
 * size:0x10; the real start begins at snd_PlaySample. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133340);

/* snd_QueueCommandToRing as the ROM really takes it: arg4 is a 64-bit qword
 * (the ring stores it with `sd`). The shared declaration above keeps arg4 at
 * s32 because the other callers in this unit pass 32-bit values with no
 * widening, and an s64 prototype would sign-extend them (it un-matches
 * func_00132B58, func_00133430 and func_00133460). */
typedef s32 (*SndQueueCommand64Fn)(s32 sel, s32 count, void *data, s32 arg3, s64 arg4);

/**
 * snd_PlaySample - 989snd EE command-ring wrapper for opcode 0x2C (start voice
 * / play sample). Queues a 0x20-byte record through snd_QueueCommandToRing.
 *
 * Twelve arguments, eight in a0..a7 and four on the caller's stack (FACT #7455):
 * a0/a1 are stored as-is; a2/a3 are 16-bit low halves packed with the 16-bit
 * high halves a4/a5 into two words; a6..a9 fill the record tail. a10 and the
 * 64-bit a11 are forwarded as the ring entry's two descriptor fields. a11 is
 * passed through unnarrowed, via SndQueueCommand64Fn. Returns the ring's
 * result.
 *
 * rec[3]'s halves are computed into locals, low half first: that source order
 * reproduces the ROM's `andi a3` ahead of `sll t1`. rec[2] is computed inline.
 */
s32 snd_PlaySample(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5,
                   s32 a6, s32 a7, s32 a8, s32 a9, s32 a10, s64 a11) {
    s32 rec[8]; /* the 0x20-byte command record handed to the ring */
    s32 low3;
    s32 high3;

    rec[0] = a0;
    rec[1] = a1;
    rec[2] = (a4 << 16) | (a2 & 0xFFFF); /* two 16-bit fields packed hi | lo */
    low3 = a3 & 0xFFFF;
    high3 = a5 << 16;
    rec[3] = high3 | low3;
    rec[4] = a6;
    rec[5] = a7;
    rec[6] = a8;
    rec[7] = a9;
    return ((SndQueueCommand64Fn)snd_QueueCommandToRing)(0x2C, 0x20, rec, a10, a11);
}

/* func_001333C0: 0x10 bytes of inter-function padding split off by symbol_addrs
 * size:0x10; the real wrapper begins at func_001333D0. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001333C0);

/**
 * Invoke snd_QueueCommandToRing with selector 0x2D, count 4, arg0 passed by
 * address in a stack local, and zero for the two trailing arguments.
 */
void func_001333D0(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x2D, 4, &value, 0, 0);
}

/**
 * func_00133400 is snd_StopVoice: the 989snd EE command-ring wrapper for cmd
 * opcode 0x2E (stop voice). (Kept as func_ here - renaming a matched function
 * desyncs its frozen nonmatchings .s glabel.) Queues via snd_QueueCommandToRing
 * with count 4 and arg0 (the voice handle) passed by address in a stack local.
 */
void func_00133400(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x2E, 4, &value, 0, 0);
}

/**
 * Invoke snd_QueueCommandToRing with selector 0x32, count 4, arg0 passed by
 * address in a stack local, and arg1/arg2 forwarded as the two trailing
 * arguments.
 */
void func_00133430(s32 arg0, s32 arg1, s32 arg2) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x32, 4, &value, arg1, arg2);
}

/**
 * Invoke snd_QueueCommandToRing with selector 0x4F, count 4, arg0 passed by
 * address in a stack local, and arg1/arg2 forwarded as the two trailing
 * arguments.
 */
void func_00133460(s32 arg0, s32 arg1, s32 arg2) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x4F, 4, &value, arg1, arg2);
}

/**
 * Invoke snd_SendCommandSync with selector 0x36 and count 4, passing arg0 by
 * address in a stack local as the third (data) argument.
 */
void func_00133490(s32 arg0) {
    s32 value = arg0;
    snd_SendCommandSync(0x36, 4, &value);
}

/* sceCdRead(lbn, sectors, buf, mode) — the direct libcdvd read-start fallback.
 * `mode` is DEREFERENCED (3 bytes: trycount/spindlctrl/datapattern), so the 4th
 * argument must be live in $7 at the call. */
extern s32 sceCdRead(s32 lbn, s32 sectors, s32 buf, void *mode);
extern s32 snd_CheckLoadInProgress(s32 noWait);
extern s32 D_001A7494;                     /* pending-read marker */
/* D_001A7100 is the 16-byte load-status block the IOP and the EE share by DMA:
 * CdStartRead sets it to 1 and writes the 0x1A7100..0x1A713F range back with
 * func_0011B3D0 (SyncDCache) for the IOP to read; snd_CheckLoadInProgress
 * invalidates the same range (func_0011B500, InvalidDCache) and re-reads it
 * after the IOP has cleared it; snd_Init (cod/0314C0) zeroes it. Its other
 * writer is the IOP, hence volatile. The qualifier is also a CODEGEN DEVICE for
 * CdStartRead (RULING #8404's terms; writer census above): it orders the store
 * ahead of the two argument `la`s, as the ROM issues it — non-volatile, cc1
 * sinks the store below them (6/49 words). */
extern volatile s32 D_001A7100;
extern u8  D_001A713F;                     /* last byte of the 64-byte sync range */
extern s32 D_001A7498;                     /* cached "load complete" flag */
extern volatile s32 g_sndIopLoadStatus;    /* EE-side load status (0 = done); see CdGetLoadStatus */
/* ADDRESSING-MODEL DEVICE (RULING #8620 term 4 / #9574, offset 0 each): the ROM
 * reaches these three with absolute %hi/%lo pairs — `lui $1; sw` for the two
 * stores and `lui/addiu` for the two `la` arguments at 0x133508..0x133524 —
 * while cc1 at -G8 treats the complete <=8-byte declarations as small data and
 * prints bare-symbol macros. `.extern <sym>, 16` ahead of the first use makes
 * GNU as expand them absolutely (first directive wins), the same model as
 * D_001A7040 above. Each is load-bearing alone on the s136os arm (solo TU,
 * fcmp, task #1914): without D_001A7100's 31/49 words, without D_001A713F's
 * 27/49, without g_sndIopLoadStatus's 25/49. The later g_sndIopLoadStatus line
 * before CdGetLoadStatus is kept; it no longer decides anything. */
__asm__(".extern D_001A7100, 16");
__asm__(".extern D_001A713F, 16");
__asm__(".extern g_sndIopLoadStatus, 16");

/**
 * CdStartRead - start an asynchronous CD read through the IOP sound/loader
 * driver.
 *
 * @lbn, @sectors, @buf: forwarded as the three words of ring command 0x38.
 * @rmode: a sceCdRMode*, used only by the libcdvd fallback.
 * Returns 1 when the read was queued, 0 when a load is already in flight, or
 * sceCdRead's result when the IOP driver is down (g_sndIopReady == 0); that
 * path passes all four arguments straight through in $4-$7.
 *
 * Queueing marks D_001A7100 busy and clears g_sndIopLoadStatus, syncs the
 * status block out to memory for the IOP, then sets D_001A7494 (the pending
 * marker snd_Pump checks) and clears the cached completion flag D_001A7498.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, selected in
 * tools/ee/s136os_functions.txt), which packs the s0-s3/ra saves 8 bytes apart
 * as the ROM does. The two devices it needs are the `.extern ,16` lines and the
 * volatile D_001A7100 above.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_CdStartRead)
S136OS_SLOT(CdStartRead);
#else
s32 CdStartRead(s32 lbn, s32 sectors, s32 buf, void *rmode) {
    s32 cmd[3]; /* the three command words for the 0x38 read request */

    if (g_sndIopReady == 0) {
        /* IOP driver down -> direct libcdvd. Pass all four through, as the ROM does. */
        return sceCdRead(lbn, sectors, buf, rmode);
    }
    if (snd_CheckLoadInProgress(1) == 1) {
        return 0; /* a load is already in flight */
    }
    D_001A7100 = 1; /* mark a load in progress */
    g_sndIopLoadStatus = 0;
    func_0011B3D0((void *)&D_001A7100, &D_001A713F);
    cmd[0] = lbn;
    cmd[1] = sectors;
    cmd[2] = buf;
    D_001A7494 = 1;
    D_001A7498 = 0;
    snd_QueueCommandToRing(0x38, 0xC, cmd, 0, 0);
    return 1;
}
#endif

/* sceCdSync(mode), defined in cod/022FA8.c. The ROM passes this function's own
 * noWait straight through ($4 is untouched before the jal at 0x13359C). */
extern s32  func_00124B88(s32 mode);          /* direct-RPC load-status fallback */
extern void func_0011B500(void *dst, void *src); /* InvalidDCache: re-read the IOP's copy */
extern volatile s32 D_001A7100; /* IOP-shared load status block (0 = done); see CdStartRead */
extern u8   D_001A713F;  /* last byte of the 64-byte sync range */
extern s32  D_001A7498;  /* cached "load complete" flag */

/**
 * snd_CheckLoadInProgress - test, or wait for, the IOP CD/bank load that
 * CdStartRead started.
 *
 * @noWait: 1 = only test; anything else = block until the load finishes.
 * Returns 1 while a load is still in flight and noWait is 1, otherwise 0 (also
 * once a blocking wait has finished). With the IOP driver down (g_sndIopReady
 * == 0) it forwards noWait to sceCdSync and returns that result.
 *
 * Each test invalidates the cache range 0x1A7100..0x1A713F so the status block
 * is re-read from memory the IOP writes, then caches "done" (status == 0) in
 * D_001A7498. The blocking wait pumps the sound engine between tests.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, selected in
 * tools/ee/s136os_functions.txt). No device of its own; it relies on
 * CdStartRead's `.extern ,16` lines for D_001A7100/D_001A713F (each removed
 * alone: 34/47 and 36/47 words, s136os solo TU, task #1914). Three phrasings
 * are load-bearing, each priced by reverting it alone: `scratch` held in a local
 * (the ROM keeps &D_001A713F in $s1 across both calls and re-forms
 * &D_001A7100 each time; 37/47), `done == 1` (the ROM compares against the 1
 * it also tests noWait with; 28/47), and a top-tested `while` (25/47 as a
 * do-while). D_001A7100's volatile qualifier makes no difference here.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_snd_CheckLoadInProgress)
S136OS_SLOT(snd_CheckLoadInProgress);
#else
s32 snd_CheckLoadInProgress(s32 noWait) {
    s32 done;

    if (g_sndIopReady == 0) {
        return func_00124B88(noWait); /* IOP driver down -> direct RPC status */
    }
    {
        u8 *scratch = &D_001A713F;

        func_0011B500((void *)&D_001A7100, scratch);
        D_001A7498 = done = (D_001A7100 == 0);
        if (done == 1) {
            return 0; /* load already complete */
        }
        if (noWait == 1) {
            return 1; /* still loading, caller asked not to block */
        }
        while (!done) { /* block: pump the sound engine until the load finishes */
            snd_Pump();
            func_0011B500((void *)&D_001A7100, scratch);
            D_001A7498 = done = (D_001A7100 == 0);
        }
    }
    return 0;
}
#endif

/**
 * CdStopRead - stop the CD streaming read. When the IOP sound/loader driver is
 * up, queue ring command 0x37 (stop read) and report success (1); otherwise
 * fall back to the direct libcdvd path func_00125620 and forward its result.
 */
s32 CdStopRead(void) {
    if (g_sndIopReady) {
        snd_QueueCommandToRing(0x37, 0, 0, 0, 0);
        return 1;
    }
    return func_00125620();
}

/* g_sndIopLoadStatus (0x1A7110) is written asynchronously on the IOP's behalf,
 * so it is volatile. The original compiler treated it as small data while the
 * assembler placed it absolutely, so its load is an unsplit `lw` macro that
 * expands to lui/lw. The `.extern ,16` override reproduces that (first
 * directive wins). `volatile` also keeps cc1 from scheduling the load into the
 * `b` delay slot, which leaves that slot free for the epilogue's `ld ra`, as in
 * the ROM. */
__asm__(".extern g_sndIopLoadStatus, 16");
extern volatile s32 g_sndIopLoadStatus;
extern s32 QueryCdStatusOverRpc(void);/* 0x125588 libcdvd status via RPC fallback */

/**
 * CdGetLoadStatus - the current CD/bank load status (0 = done).
 *
 * With the IOP sound/loader driver up (g_sndIopReady), returns the status word
 * the IOP maintains in g_sndIopLoadStatus. Otherwise asks libcdvd directly over
 * RPC (QueryCdStatusOverRpc). The ready path is tested first because the ROM
 * lays it out as the fall-through block.
 */
s32 CdGetLoadStatus(void) {
    if (g_sndIopReady != 0) {
        return g_sndIopLoadStatus;
    }
    return QueryCdStatusOverRpc();
}

/* func_001336C0: 0x10 bytes of inter-function padding split off by symbol_addrs
 * size:0x10; the real function begins at SetSndPumpCallback. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001336C0);

/**
 * SetSndPumpCallback - install a new snd_Pump tick callback. When the IOP
 * sound/loader driver is up, swap the gp-relative callback slot D_001A7490 to
 * `callback` and return the previous one; otherwise defer to the pre-init path
 * func_001245D0 and forward its result.
 */
void *SetSndPumpCallback(void *callback) {
    if (g_sndIopReady) {
        void *previous = D_001A7490;
        D_001A7490 = callback;
        return previous;
    }
    return func_001245D0(callback);
}

/* func_00133700: 0x10 bytes of inter-function padding split off by symbol_addrs
 * size:0x10; the real wrapper begins at func_00133710. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133700);

/**
 * Invoke snd_QueueCommandToRing with selector 0x50, count 0x14, and a 5-word
 * stack record holding arg0..arg4; the two trailing arguments are zero.
 */
void func_00133710(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 args[5];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    args[4] = arg4;
    snd_QueueCommandToRing(0x50, 0x14, args, 0, 0);
}

/**
 * Invoke snd_QueueCommandToRing with selector 0x51, count 8, and a stack record
 * holding arg0 and arg1; the two trailing arguments are zero.
 */
void func_00133750(s32 arg0, s32 arg1) {
    s32 args[2];
    args[0] = arg0;
    args[1] = arg1;
    snd_QueueCommandToRing(0x51, 8, args, 0, 0);
}

/**
 * Invoke snd_QueueCommandToRing with selector 0x10, count 0x10 (16 bytes), and
 * a stack record of four words (arg0..arg3); the two trailing arguments are
 * zero.
 */
void func_00133788(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 args[4];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    snd_QueueCommandToRing(0x10, 0x10, args, 0, 0);
}

/* func_001337C8: 0x28 bytes of inter-function padding split off by symbol_addrs
 * size:0x28; the real wrapper begins at func_001337F0. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001337C8);

/**
 * Invoke snd_SendCommandSync with selector 0x4A and no payload, forwarding its
 * return value (the value-return keeps the frame, see func_00132828).
 */
s32 func_001337F0(void) {
    return snd_SendCommandSync(0x4A, 0, 0);
}

/* func_00133818: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real wrapper begins at func_00133820. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133818);

/**
 * Invoke snd_SendCommandSync with selector 0x4B and no payload, forwarding its
 * return value (the value-return keeps the frame, see func_00132828).
 */
s32 func_00133820(void) {
    return snd_SendCommandSync(0x4B, 0, 0);
}

/* func_00133848: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real wrapper begins at func_00133850. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133848);

/**
 * Invoke snd_SendCommandSync with selector 0x3B and count 0x18 (24 bytes),
 * passing a 6-word stack record (arg0..arg5) as the data argument, and
 * forwarding its return value: nothing after the jal writes $v0, and the one
 * caller, FmvPtsQueueInit, stores and tests it (task #1428).
 */
s32 func_00133850(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    s32 args[6];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    args[4] = arg4;
    args[5] = arg5;
    return snd_SendCommandSync(0x3B, 0x18, args);
}

/**
 * Invoke snd_SendCommandSync with selector 0x3D and no payload, forwarding its
 * return value (the value-return keeps the frame, see func_00132828).
 */
s32 func_00133890(void) {
    return snd_SendCommandSync(0x3D, 0, 0);
}

/* func_001338B8: 0x10 bytes of inter-function padding split off by symbol_addrs
 * size:0x10; the real wrapper begins at func_001338C8. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001338B8);

/**
 * Invoke snd_SendCommandSync with selector 0x3C and no payload, forwarding its
 * return value (the value-return keeps the frame, see func_00132828).
 */
s32 func_001338C8(void) {
    return snd_SendCommandSync(0x3C, 0, 0);
}

/**
 * Invoke snd_SendCommandSync with selector 0x3E and count 0x14 (20 bytes),
 * passing a stack record of five words (arg0..arg4) as the data argument.
 */
void func_001338F0(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 args[5];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    args[4] = arg4;
    snd_SendCommandSync(0x3E, 0x14, args);
}

/* func_00133928: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real wrapper begins at func_00133930. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133928);

/**
 * Invoke snd_SendCommandSync with selector 0x5A and count 8, passing a 2-word
 * stack record (arg0, arg1) as the data argument.
 */
void func_00133930(s32 arg0, s32 arg1) {
    s32 args[2];
    args[0] = arg0;
    args[1] = arg1;
    snd_SendCommandSync(0x5A, 8, args);
}

/**
 * Invoke snd_SendCommandSync with selector 0x5B and no payload, forwarding its
 * return value (the value-return keeps the frame, see func_00132828).
 */
s32 func_00133960(void) {
    return snd_SendCommandSync(0x5B, 0, 0);
}

/**
 * Scale arg0 by 1524/741, i.e. (arg0 * 0x5F4) / 0x2E5, with signed division.
 * The divide-by-zero trap (`beqzl` + `break 7`) is emitted by cc1 for the
 * constant divisor anyway.
 *
 * The `break 7` word is not a gate difference, even though it looks like one
 * in a scratch build. Bare mips-linux-gnu-as encodes cc1's `break 7` as
 * 0x0007000D, with the code in the high field. tools/ee/move_fixup.sed
 * rewrites it to `break 0,7` before assembly, so the gate emits 0x000001CD,
 * which is the ROM's word (FACT #6462, re-measured in task #866 as
 * FACT #8203). The rule's own comment in move_fixup.sed records that every
 * compiler-emitted break in both regions is 0x000001CD. Do not "fix" a
 * 0x0007000D seen outside the asm_unit.sh pipeline. cod/0314C0.c's
 * func_00131730 relies on the same rewrite.
 *
 * This C body was already correct before promotion: cc1 2.9's output for it
 * matches all 9 ROM words under SN as.exe (FACT #6219). It had scored 82.22%
 * only because mips-linux-gnu-as, under `.set reorder`, moved `mflo` into
 * the `jr` delay slot and dropped the trailing nop. That was assembler
 * reorder-slot noise, not a C or ee-gcc scheduling wall. It was fixed by the
 * mflo/mfhi return-slot rule in tools/ee/asm_unit.sh (A4, task #919,
 * FACT #8339), with no change to the C. Promoted in task #942.
 */
s32 func_00133988(s32 x) {
    return x * 0x5F4 / 0x2E5;
}

/* OnVblankInterrupt: bumps the 64-bit tick counter D_001A7208 and snapshots
 * D_001A7210 = D_001A7200 + T1_COUNT (0x10000800). Best attempt 75% (15/16
 * insns, volatile u64 scalars + `.extern ,16` overrides reproduce the
 * $at-macro store) — the one residue is the timer-address `ori`, which this
 * cc1 schedules after the D_001A7200 load while the original keeps it before.
 * A scheduling choice not expressible in source. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", OnVblankInterrupt);

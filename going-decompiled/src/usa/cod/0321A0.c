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

/* snd_ServiceRpcCompletion: polls the cmd-channel RPC result buffer
 * (D_001A7480/D_001A7484) after func_0011AEA0/sceSifCheckStatRpc, prints
 * D_0013BFA0 via snd_PrintError on stall. Best attempt 78% — ee-gcc lays the
 * early-return-1 branch out as `b`+`li` (or a branch-likely `bnel`) instead of
 * the original's `beqz` straight to the epilogue with the return value in the
 * delay slot. A branch-layout shape this cc1 won't reproduce (near-miss).
 * Portable #else body. */
extern void func_0011AEA0(s32 arg);            /* pre-RPC flush/sync */
extern s32  sceSifCheckStatRpc(void *rpc);     /* nonzero while the RPC is busy */
extern void snd_PrintError(const char *msg, ...); /* printf-style; some call sites pass %d args */
extern u8   D_001A7040[];  /* sceSif RPC data block */
extern s32  D_001A74F8;    /* suppresses the completion-mismatch error print */
extern char D_0013BFA0[];  /* "RPC completion mismatch" error string */
extern u8  *D_001A7480;    /* active DMA-transfer buffer */
extern s32  D_001A7484;    /* active DMA-transfer entry count */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", snd_ServiceRpcCompletion);
#else
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
    if (buf[0] == 0xFFFFFFFF &&
        *(u32 *)((u8 *)buf + D_001A7484 * 4 + 4) == 0xFFFFFFFF) {
        D_001A7480 = 0; /* both terminators consumed -> transfer done */
        return 1;
    }
    if (D_001A74F8 == 0) {
        snd_PrintError(D_0013BFA0);
    }
    return 0;
}
#endif

/* snd_SetupDmaTransfer: body reproduces 1:1 (89%), but the function saves
 * s0+s1+ra and this cc1 reserves a 16-byte stack slot per callee save (frame
 * 0x30, sd at 0/16/32) while the original packs them 8-byte (frame 0x20, sd at
 * 0/8/16). No flag found that packs the slots (2.95.2 even emits sq); this
 * callee-save-layout wall blocks every multi-save function in this unit
 * (near-miss). Portable #else body. */
extern void func_0011B3D0(void *start, void *end); /* writeback/flush a small range */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", snd_SetupDmaTransfer);
#else
void snd_SetupDmaTransfer(u8 *buffer, s32 count) {
    D_001A7484 = count;
    D_001A7480 = buffer;
    *(s32 *)(buffer + count * 4 + 4) = 0; /* terminator slot after the list */
    *(s32 *)(buffer + 0) = 0;             /* header slot */
    func_0011B3D0(buffer, buffer + 3);
    func_0011B3D0(buffer + count * 4 + 4, buffer + count * 4 + 4 + 3);
}
#endif

/* snd_BankLoadByLoc: request a sound-bank load from the IOP over the SIF RPC
 * load channel and block until the IOP posts the result. Returns the IOP's load
 * handle/result, or 0 on any early-out (system down, a load already running, or
 * the RPC call failing). Not matched — the multi-callee-save frame hits the same
 * 16-byte save-slot layout wall as snd_SetupDmaTransfer (near-miss). Portable
 * #else body.
 *
 * NOTE the branch-likely at 0x132360: `bnel` writing D_001A71C0 = arg0 in its
 * delay slot fires ONLY when snd_CheckLoadInProgress(1) != 1 (the proceed path);
 * when a load is already in progress (== 1) the write is nullified — so the
 * request word is only stored on the path that actually issues the load. */
extern s32  D_001A74C8;   /* nonzero while the command ring still has pending work */
extern s32  D_001A7488;   /* last bank-load status/error code */
extern s32  D_001A71C0;   /* load-request word 0 (base of the 8-byte send buffer) */
extern s32  D_001A71C4;   /* load-request word 1 */
extern s32  D_001A7180;   /* IOP load-result slot (4-byte receive buffer) */
extern u8   D_001A7140[]; /* SIF RPC client data block for the load channel */
extern char D_001A7578[]; /* command-ring / RPC stall diagnostic string */
extern char D_001A7600[]; /* "sound system not ready" diagnostic */
extern char D_001A7630[]; /* "load already in progress" diagnostic */
extern char D_001A7650[]; /* "load RPC failed" diagnostic */
extern s32  snd_CheckLoadInProgress(s32 noWait);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", snd_BankLoadByLoc);
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
    D_001A7180 = -1;

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

    /* block until the IOP overwrites the -1 sentinel with the load result */
    if (D_001A7180 == -1) {
        do {
            func_0011AEA0(0);
        } while (D_001A7180 == -1);
    }
    return D_001A7180;
}
#endif

/* snd_BankLoadAsync: fire-and-forget variant of snd_BankLoadByLoc — stage the
 * load request (plus an async completion context arg2/arg3), mark a load pending
 * (D_001A74C8 = 1), issue the load RPC (function 3), and return WITHOUT waiting
 * for the result. Returns nothing. Not matched — same 16-byte save-slot layout
 * wall as snd_SetupDmaTransfer (near-miss). Portable #else body.
 *
 * Same branch-likely as snd_BankLoadByLoc: the `bnel` at 0x1324F8 stores
 * D_001A71C0 = arg0 only when snd_CheckLoadInProgress(1) != 1 (the proceed path).
 * arg3 lands in D_001A74D8 as a sign-extended 64-bit qword in the original. */
extern s32 D_001A74D0;  /* async completion context word (arg2) */
extern s64 D_001A74D8;  /* async completion context qword (arg3, 64-bit) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", snd_BankLoadAsync);
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

    /* stage the request, the async completion context, and the pending sentinel */
    D_001A71C0 = arg0;
    D_001A71C4 = arg1;
    D_001A7180 = -1;
    D_001A74D0 = arg2;
    D_001A74D8 = arg3;

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001325E0);

/* func_001325E8 = snd_BankLoadFromEE_CB (0x1325E8): recovered splat-dropped
 * function (spimdisasm emitted no .s — reached by fallthrough/data-ref).
 * Raw words, byte-exact; recovers the 0xF0 that shifted cod rodata/jtbls -0xF0. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001325E8);

/* snd_BankLoadFromIOP: request a sound-bank load already resident on the IOP
 * (RPC function 0x59, single 4-byte argument) and block until the IOP posts the
 * result. Returns the load handle/result, or 0 on an early-out / RPC failure.
 * The simplest of the load family — no snd_CheckLoadInProgress gate and no
 * branch-likely. Not matched — same 16-byte save-slot layout wall as
 * snd_SetupDmaTransfer (near-miss). Portable #else body. */
extern char D_001A7730[]; /* "sound system not ready" diagnostic */
extern char D_001A7760[]; /* "IOP load RPC failed" diagnostic */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", snd_BankLoadFromIOP);
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
    D_001A7180 = -1;

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

    /* block until the IOP overwrites the -1 sentinel with the load result */
    if (D_001A7180 == -1) {
        do {
            func_0011AEA0(0);
        } while (D_001A7180 == -1);
    }
    return D_001A7180;
}
#endif

/* func_00132818: 0x10 bytes of inter-function padding split off by symbol_addrs
 * size:0x10; the real wrapper begins at func_00132828. Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132818);

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

/* func_001328C0: builds a 0x1C-byte record for snd_QueueCommandToRing (selector
 * 0x60) — word 0 = arg0, then either a 24-byte copy of *arg1 or a -1 sentinel.
 * Not matched: the 24-byte payload sits at a *misaligned* offset 4 and the
 * source is itself unaligned, so the original copies it with unaligned
 * ldl/ldr/sdl/sdr. Reproducing that requires a packed (alignment-1) struct copy
 * that ee-gcc 2.9 won't emit from natural C — an aligned struct lands the
 * payload at offset 8 with aligned ld/sd instead (near-miss). Portable #else. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001328C0);
#else
s32 func_001328C0(s32 cmd, const void *payload) {
    s32 buf[8];
    buf[0] = cmd;
    if (payload != 0) {
        memcpy((u8 *)buf + 4, payload, 0x18); /* copy the 0x18-byte payload */
    } else {
        *(s32 *)((u8 *)buf + 4) = -1;         /* no payload -> -1 sentinel */
    }
    return snd_QueueCommandToRing(0x60, 0x1C, buf, 0, 0);
}
#endif

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132968);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001329F0);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132A58);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132AA0);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132B88);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00132C08);

/* snd_SendCommandSync: assemble a 989snd command of `count` bytes and issue it
 * synchronously over the SIF RPC channel, blocking until the reply lands. arg0
 * is the RPC function number, arg1 the parameter-byte count, arg2 the parameter
 * bytes. Returns the RPC reply status word (D_001A7084). Not matched — the
 * multi-callee-save frame hits the same 16-byte save-slot layout wall as
 * snd_SetupDmaTransfer (near-miss). Portable #else body (no branch-likely delay
 * slots; all control flow is plain blez/bnez/beqz/b).
 *
 * func_0011D620 is the sceSifCallRpc-shaped primitive (client, fno, mode, send
 * buf/size, recv buf/size, end callback/param); a zero-length command sends no
 * buffer. */
extern u8  D_001A70C0[];   /* RPC command parameter byte buffer (absolute) */
extern u8  D_001A7080[];   /* DMA send / RPC receive scratch buffer (absolute) */
extern s32 D_001A7084[];   /* RPC reply status word (absolute) */
extern void snd_FlushCommandRing(void);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", snd_SendCommandSync);
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
    while (snd_ServiceRpcCompletion() == 0) {
    }

    /* if the active ring buffer still holds queued entries and no service is
     * pending, flush it now */
    if (*D_001A74A0[D_001A74C0] != 0 && D_001A74C4 == 0) {
        snd_FlushCommandRing();
    }
    return D_001A7084[0];
}
#endif

/* snd_QueueCommandToRing: append one command to the active double-buffered
 * 989snd ring (or, when idle with an empty command, issue a bare sync RPC). Not
 * matched — the multi-callee-save frame hits the same 16-byte save-slot layout
 * wall as snd_SetupDmaTransfer (near-miss). Portable #else body (no branch-likely
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
 * while spinning for space. */
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

/* snd_FlushCommandRing: DMA-kick + SIF-RPC flush of the active 989snd command
 * ring buffer, then flip to the other buffer. Not matched — the multi-callee-
 * save frame hits the same 16-byte save-slot layout wall as snd_SetupDmaTransfer
 * (near-miss). Portable #else body (no branch-likely delay slots, so a faithful
 * transcription; all control flow is plain bnez/b).
 *
 * The ring is double-buffered by D_001A74C0 (0/1): D_001A74A0[i] points at a
 * buffer whose leading word is its entry count, D_001A74B8[i] is that buffer's
 * DMA/receive area, D_001A74A8[i] tracks remaining free space (reset to 0xFFC).
 * func_0011D620 is the sceSifCallRpc-shaped primitive (client, fno, mode, send
 * buf/size, recv buf/size, end callback/param). */
extern u8  *D_001A74B8[2]; /* per-buffer DMA/receive buffers */
/* D_001A74A8, D_001A7578 and func_0011D620 are declared above. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", snd_FlushCommandRing);
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
                  D_001A74A0[idx],              /* send buffer   */
                  0x1000 - D_001A74A8[idx],     /* send size     */
                  D_001A74B8[idx],              /* receive buffer*/
                  (*D_001A74A0[idx] << 2) + 8,  /* receive size  */
                  0, 0);                        /* no completion callback */

    /* flip to the other buffer and reset it for refilling */
    idx = (D_001A74C0 ^ 1) != 0 ? 1 : 0;
    D_001A74C0 = idx;
    *D_001A74A0[idx] = 0;
    D_001A74A8[idx] = 0xFFC;
}
#endif

/* func_00133220: sets the gp-relative flag D_001A74C4 to 1 and returns 1
 * (reusing the same register for the store and the return). Not matched even
 * at -G8: ee-gcc materialises the constant twice (`li $3,1; li $2,1; sw $3`)
 * for every store-constant-and-return-it formulation tried, while the original
 * stores the return register itself. A register-allocation shape this cc1
 * won't reproduce from natural C (near-miss). Portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133220);
#else
s32 func_00133220(void) {
    return D_001A74C4 = 1; /* raise the command-ring service-pending flag */
}
#endif

/**
 * Clear the command-ring service-pending flag D_001A74C4 (set by
 * func_00133220) and run one snd_Pump service pass, forwarding its return
 * value.
 */
s32 func_00133230(void) {
    D_001A74C4 = 0;
    return snd_Pump();
}

/* func_00133250: waits for IOP readiness then sends sync command 0x2A with a
 * 4-word record, storing the result back into g_sndIopReady. Blocked twice
 * over: a spin loop whose body is `jal snd_Pump` followed by THREE literal
 * nops (cc1 never pads like that), plus the multi-callee-save 16-byte
 * save-slot layout wall (see snd_SetupDmaTransfer) (near-miss). Portable #else. */
extern s32 snd_SendCommandSync(s32 sel, s32 count, void *data);
extern s32 snd_CheckLoadInProgress(s32 noWait);
extern s32 D_001A74C8; /* nonzero while the command ring still has pending work */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133250);
#else
s32 func_00133250(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 cmd[4];

    if (g_sndIopReady == 1) {
        return 0; /* IOP driver already up */
    }
    if (D_001A74C8 != 0) {
        while (snd_Pump() != 0) { } /* drain the pending command ring */
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133300);

/**
 * Invoke snd_QueueCommandToRing with selector 0x34 and no payload, forwarding
 * its return value (the value-return keeps the frame, see func_00132828).
 */
s32 func_00133310(void) {
    return snd_QueueCommandToRing(0x34, 0, 0, 0, 0);
}

/* func_00133340: 0x10 bytes of inter-function padding split off by symbol_addrs
 * size:0x10; the real start begins at snd_PlaySample. Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133340);

/* snd_PlaySample: 989snd EE command-ring wrapper for cmd opcode 0x2C (start
 * voice / play sample). Builds a record from a mix of register and stack
 * arguments (16-bit fields packed via andi/sll/or, plus four words pulled from
 * the caller's stack) for snd_QueueCommandToRing count 0x20. The stack-argument
 * loads (lw 0x30/0x38/0x40($sp), ld 0x48($sp)) come from an 8+ argument calling
 * convention this cc1 won't reproduce from natural C (near-miss). Portable #else
 * body.
 *
 * Args a0..a11: a0/a1 stored as-is; a2/a3 are 16-bit low halves paired with the
 * 16-bit high halves a4/a5 into two packed words; a6..a9 fill the record tail.
 * a10/a11 are forwarded to snd_QueueCommandToRing as its ring-slot fields (a11
 * is a 64-bit qword in the original; the shared ring decl narrows it to 32-bit,
 * which is inconsequential to the portable build). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", snd_PlaySample);
#else
s32 snd_PlaySample(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5,
                   s32 a6, s32 a7, s32 a8, s32 a9, s32 a10, s64 a11) {
    s32 rec[8]; /* the 0x20-byte command record handed to the ring */
    rec[0] = a0;
    rec[1] = a1;
    rec[2] = (a4 << 16) | (a2 & 0xFFFF); /* two 16-bit fields packed hi | lo */
    rec[3] = (a5 << 16) | (a3 & 0xFFFF);
    rec[4] = a6;
    rec[5] = a7;
    rec[6] = a8;
    rec[7] = a9;
    return snd_QueueCommandToRing(0x2C, 0x20, rec, a10, a11);
}
#endif

/* func_001333C0: 0x10 bytes of inter-function padding split off by symbol_addrs
 * size:0x10; the real wrapper begins at func_001333D0. Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001333C0);

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

/* CdStartRead: queue ring command 0x38 (start read) with a 3-word record, or
 * fall back to func_001253A8 when the IOP driver is down. Best attempt 79% —
 * blocked by the multi-callee-save 16-byte save-slot layout wall (see
 * snd_SetupDmaTransfer) plus the D_001A7100/g_sndIopLoadStatus $at-macro
 * stores (cc1-small / assembler-absolute disagreement, see CdGetLoadStatus)
 * (near-miss). Portable #else body. */
extern s32 func_001253A8(void);            /* direct libcdvd read-start fallback */
extern s32 snd_CheckLoadInProgress(s32 noWait);
extern s32 D_001A7494;                     /* pending-read marker */
extern s32 D_001A7100;                     /* IOP-polled load status word */
extern u8  D_001A713F;                     /* poll-request scratch byte */
extern s32 D_001A7498;                     /* cached "load complete" flag */
extern s32 g_sndIopLoadStatus;             /* EE-side load status (0 = done) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", CdStartRead);
#else
s32 CdStartRead(s32 arg0, s32 arg1, s32 arg2) {
    s32 cmd[3]; /* the three command words for the 0x38 read request */

    if (g_sndIopReady == 0) {
        return func_001253A8(); /* IOP driver down -> direct libcdvd */
    }
    if (snd_CheckLoadInProgress(1) == 1) {
        return 0; /* a load is already in flight */
    }
    D_001A7100 = 1;             /* mark a load in progress */
    g_sndIopLoadStatus = 0;
    func_0011B3D0(&D_001A7100, &D_001A713F);
    cmd[0] = arg0;
    cmd[1] = arg1;
    cmd[2] = arg2;
    D_001A7494 = 1;
    D_001A7498 = 0;
    snd_QueueCommandToRing(0x38, 0xC, cmd, 0, 0);
    return 1;
}
#endif

/* snd_CheckLoadInProgress: tests/waits on the bank-load-in-progress flag
 * D_001A7100 (via func_0011B500 + snd_Pump). Best attempt 69% — the original
 * re-materialises the D_001A7100/D_001A713F addresses from fresh lui macros
 * every loop iteration (cc1-small symbolic refs the SN assembler expanded
 * absolutely), while this cc1+GAS either CSEs explicit %hi pairs in extra
 * callee-saved regs or gp-relativises the small declarations. Also hits the
 * multi-callee-save save-slot wall (see snd_SetupDmaTransfer) (near-miss).
 * Portable #else body. */
extern s32  func_00124B88(void);              /* direct-RPC load-status fallback */
extern void func_0011B500(void *dst, void *src); /* poll IOP load status into dst */
extern s32  D_001A7100;  /* IOP-polled load status word (0 = done) */
extern u8   D_001A713F;  /* poll-request scratch byte */
extern s32  D_001A7498;  /* cached "load complete" flag */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", snd_CheckLoadInProgress);
#else
s32 snd_CheckLoadInProgress(s32 noWait) {
    s32 done;

    if (g_sndIopReady == 0) {
        return func_00124B88(); /* IOP driver down -> direct RPC status */
    }
    func_0011B500(&D_001A7100, &D_001A713F);
    D_001A7498 = done = (D_001A7100 == 0);
    if (done) {
        return 0; /* load already complete */
    }
    if (noWait == 1) {
        return 1; /* still loading, caller asked not to block */
    }
    do { /* block: pump the sound engine until the load finishes */
        snd_Pump();
        func_0011B500(&D_001A7100, &D_001A713F);
        D_001A7498 = done = (D_001A7100 == 0);
    } while (!done);
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

/* CdGetLoadStatus: returns the cached g_sndIopLoadStatus when g_sndIopReady,
 * else falls back to func_00125588. Best attempt 77% — the original reads
 * g_sndIopLoadStatus through a symbolic `lw` macro (cc1 considered it small,
 * the SN assembler expanded it absolutely as lui/lw); reproducing that with a
 * file-scope `.extern sym,16` override makes GNU cc1 schedule the 2-insn
 * macro into a branch delay slot, corrupting the expansion (near-miss).
 * Portable #else body. */
extern s32 g_sndIopLoadStatus;        /* 0x1A7110 EE-side load status (0 = done) */
extern s32 QueryCdStatusOverRpc(void);/* 0x125588 libcdvd status via RPC fallback */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", CdGetLoadStatus);
#else
s32 CdGetLoadStatus(void) {
    if (g_sndIopReady == 0) {
        return QueryCdStatusOverRpc();
    }
    return g_sndIopLoadStatus;
}
#endif

/* func_001336C0: 0x10 bytes of inter-function padding split off by symbol_addrs
 * size:0x10; the real function begins at SetSndPumpCallback. Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001336C0);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133700);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001337C8);

/**
 * Invoke snd_SendCommandSync with selector 0x4A and no payload, forwarding its
 * return value (the value-return keeps the frame, see func_00132828).
 */
s32 func_001337F0(void) {
    return snd_SendCommandSync(0x4A, 0, 0);
}

/* func_00133818: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real wrapper begins at func_00133820. Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133818);

/**
 * Invoke snd_SendCommandSync with selector 0x4B and no payload, forwarding its
 * return value (the value-return keeps the frame, see func_00132828).
 */
s32 func_00133820(void) {
    return snd_SendCommandSync(0x4B, 0, 0);
}

/* func_00133848: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real wrapper begins at func_00133850. Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133848);

/**
 * Invoke snd_SendCommandSync with selector 0x3B and count 0x18 (24 bytes),
 * passing a 6-word stack record (arg0..arg5) as the data argument.
 */
void func_00133850(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    s32 args[6];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    args[4] = arg4;
    args[5] = arg5;
    snd_SendCommandSync(0x3B, 0x18, args);
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_001338B8);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133928);

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

/* func_00133988: scale arg0 by 1524/741, i.e. (arg0 * 0x5F4) / 0x2E5. ~82%; the
 * only diff is that ee-gcc fills the `jr ra` delay slot with the `mflo`, while
 * the original keeps `mflo` before the return and leaves a nop in the slot — a
 * scheduling choice not expressible in source (near-miss). Portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", func_00133988);
#else
s32 func_00133988(s32 x) {
    return x * 0x5F4 / 0x2E5;
}
#endif

/* OnVblankInterrupt: bumps the 64-bit tick counter D_001A7208 and snapshots
 * D_001A7210 = D_001A7200 + T1_COUNT (0x10000800). Best attempt 75% (15/16
 * insns, volatile u64 scalars + `.extern ,16` overrides reproduce the
 * $at-macro store) — the one residue is the timer-address `ori`, which this
 * cc1 schedules after the D_001A7200 load while the original keeps it before.
 * A scheduling choice not expressible in source. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0321A0", OnVblankInterrupt);

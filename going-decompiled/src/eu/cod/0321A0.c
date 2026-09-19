#include "common.h"

/*
 * cod/0321A0 (EU mirror) — the 989snd EE-side command/bank TU, the EU port of
 * USA cod/0321A0 (carved 2026-06-13 as the EU REGION-axis sibling). The C
 * bodies are region-agnostic and identical to the USA unit; only the extern
 * global names differ (the EU 989snd small-data cluster sits +0x80 from USA,
 * and the snd-block text is shifted +0x60). objdiff masks the gp/reloc deltas,
 * so the same source matches both regions.
 *
 * Like USA, this is an original separate translation unit built at nonzero -G:
 * its small-data globals are accessed uniformly via %gp_rel($gp), which -G0
 * cannot express. The matcher builds THIS unit at -O2 -G8 (per-unit override in
 * tools/ee/objdiff_build.sh / diff.sh / build.sh).
 *
 * EU boundaries SIGNATURE-matched (NOT a fixed +0x80): the unit begins at
 * snd_ServiceRpcCompletion (EU 0x132280 / USA 0x132220) and ends just before
 * func_001339F0-equiv (EU tail unit cod/033970 at 0x133A50 / USA 0x1339F0).
 * Within the block the USA->EU text delta is +0x60 throughout, so each
 * func_<EUaddr> = USA func_<EUaddr-0x60>.
 *
 * -G8 rule for externs: a complete extern object of size <= 8 bytes is placed
 * in small data (gp-relative access); anything the original accesses with
 * absolute %hi/%lo pairs must be declared with an incomplete array type so cc1
 * cannot prove it small.
 */

extern s32 snd_QueueCommandToRing(s32 sel, s32 count, void *data, s32 arg3, s32 arg4);
extern s32 snd_SendCommandSync(s32 arg0, s32 arg1, void *arg2);
extern s32 snd_Pump(void);
extern s32 QueryCdStatusOverRpc(void);
extern s32 func_00125620(void);
extern void *func_001245D0(void *arg0);

/* 989snd small-data globals (gp-relative in the original; complete <=8-byte
 * declarations so -G8 places them in small data). EU addresses (= USA +0x80). */
extern s32 D_001A7544;     /* command-ring service-pending flag (USA D_001A74C4) */
extern s32 *D_001A7520[2]; /* double-buffered ring entry-count pointers (USA D_001A74A0) */
extern s32 D_001A7540;     /* active ring buffer index (0/1) (USA D_001A74C0) */
extern s32 D_001A750C;     /* nonzero when the IOP sound/loader driver is up (USA g_sndIopReady) */
extern void *D_001A7510;   /* snd_Pump tick callback (USA D_001A7490) */

/* Additional 989snd globals + callees used by the EU-lockstep #else bodies below
 * (ring core + loaders). EU data symbols are uniformly USA + 0x80 (sdata cluster
 * AND the cod rodata diagnostic strings alike). Callee sceSifCheckStatRpc appears
 * as func_0011D810 in EU's stripped symtab; we use the meaningful name here. */
extern s32   D_001A7528[2]; /* per-buffer ring free space in bytes (USA D_001A74A8) */
extern u8   *D_001A7530[2]; /* per-buffer 16-byte descriptor arrays (USA D_001A74B0) */
extern u8   *D_001A7538[2]; /* per-buffer DMA/receive buffers (USA D_001A74B8) */
extern u8   *D_001A7500;    /* active DMA-transfer buffer (USA D_001A7480) */
extern s32   D_001A7578;    /* suppresses the completion-mismatch error print (USA D_001A74F8) */
extern u8    D_001A70C0[];  /* SIF RPC data block, command channel (USA D_001A7040) */
extern u8    D_001A7100[];  /* DMA send / RPC receive scratch buffer (USA D_001A7080) */
extern s32   D_001A7104[];  /* RPC reply status word (USA D_001A7084) */
extern u8    D_001A7140[];  /* RPC command parameter byte buffer (USA D_001A70C0) */
extern char  D_001A75F8[];  /* command-ring / RPC stall diagnostic (USA D_001A7578) */
extern char  D_001A7950[];  /* ring-stall diagnostic format string (USA D_001A78D0) */
extern char  D_0013C078[];  /* ring-wrap diagnostic format string (USA D_0013BFF8) */
extern void  func_0011AEA0(s32 arg);
extern s32   sceSifCheckStatRpc(void *rpc);
extern void  snd_PrintError(const char *msg, ...);
extern void  snd_SetupDmaTransfer(u8 *buffer, s32 count);
extern s32   snd_ServiceRpcCompletion(void);
extern s32   snd_CommitRingEntry(void);
extern void  snd_FlushCommandRing(void);
extern s32   func_0011D620(void *rpc, s32 fno, s32 mode, void *sbuf, s32 ssize,
                           void *rbuf, s32 rsize, void *endfn, s32 endpar);
/* bank-loader globals (USA + 0x80) + snd_CheckLoadInProgress */
extern s32   D_001A7508;  /* last bank-load status/error code (USA D_001A7488) */
extern s32   D_001A7548;  /* ring-busy flag (USA D_001A74C8) */
extern s32   D_001A7240;  /* load-request word 0 (USA D_001A71C0) */
extern s32   D_001A7244;  /* load-request word 1 (USA D_001A71C4) */
extern s32   D_001A7200;  /* IOP load-result slot (USA D_001A7180) */
extern s32   D_001A7550;  /* async completion context word (USA D_001A74D0) */
extern s64   D_001A7558;  /* async completion context qword (USA D_001A74D8) */
extern u8    D_001A71C0[];/* SIF RPC client data block, load channel (USA D_001A7140) */
extern char  D_001A7680[];/* "sound system not ready" diagnostic (USA D_001A7600) */
extern char  D_001A76B0[];/* "load already in progress" diagnostic (USA D_001A7630) */
extern char  D_001A76D0[];/* "load RPC failed" diagnostic (USA D_001A7650) */
extern char  D_001A77B0[];/* "sound system not ready" diagnostic (USA D_001A7730) */
extern char  D_001A77E0[];/* "IOP load RPC failed" diagnostic (USA D_001A7760) */
extern s32   snd_CheckLoadInProgress(s32 noWait);
extern s32   D_001A7504;  /* active DMA-transfer entry count (USA D_001A7484) */
extern char  D_0013C020[];/* RPC completion-mismatch diagnostic (USA D_0013BFA0) */
extern void  func_0011B3D0(void *start, void *end); /* writeback/flush a small range */

/* CD-read / IOP-readiness globals + callees used by the EU-lockstep #else bodies
 * below (func_00133280..CdGetLoadStatus + snd_CheckLoadInProgress). EU data = USA
 * + 0x80. NB: EU D_001A7180 here (= USA D_001A7100, the IOP-polled load status)
 * is a DISTINCT symbol from D_001A7200 above (= USA D_001A7180). */
extern s32   D_001A7180;  /* IOP-polled load status word (USA D_001A7100, 0 = done) */
extern u8    D_001A71BF;  /* poll-request scratch byte (USA D_001A713F) */
extern s32   D_001A7190;  /* EE-side load status (USA g_sndIopLoadStatus 0x1A7110, 0 = done) */
extern s32   D_001A7514;  /* pending-read marker (USA D_001A7494) */
extern s32   D_001A7518;  /* cached "load complete" flag (USA D_001A7498) */
/* sceCdRead(lbn, sectors, buf, mode) — the direct libcdvd read-start fallback.
 * `mode` is DEREFERENCED (3 bytes: trycount/spindlctrl/datapattern), so the 4th
 * argument must be live in $7 at the call. */
extern s32   func_001253A8(s32 lbn, s32 sectors, s32 buf, void *mode);
extern s32   func_00124B88(void);            /* direct-RPC load-status fallback */
extern void  func_0011B500(void *dst, void *src); /* poll IOP load status into dst */

/* snd_ServiceRpcCompletion: poll the cmd-channel RPC result buffer after
 * func_0011AEA0/sceSifCheckStatRpc; returns 1 when the transfer is complete (both
 * terminators consumed), 0 while it is still running. Not matched — branch-layout
 * shape this cc1 won't reproduce (same wall as USA). Portable #else body (EU
 * lockstep with USA snd_ServiceRpcCompletion; data globals +0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_ServiceRpcCompletion);
#else
s32 snd_ServiceRpcCompletion(void) {
    u32 *buf;

    func_0011AEA0(0);
    if (D_001A7500 == 0) {
        return 1; /* no transfer in flight */
    }
    if (sceSifCheckStatRpc(D_001A70C0) != 0) {
        return 0; /* RPC still running */
    }
    buf = (u32 *)D_001A7500;
    if (buf[0] == 0xFFFFFFFF &&
        *(u32 *)((u8 *)buf + D_001A7504 * 4 + 4) == 0xFFFFFFFF) {
        D_001A7500 = 0; /* both terminators consumed -> transfer done */
        return 1;
    }
    if (D_001A7578 == 0) {
        snd_PrintError(D_0013C020);
    }
    return 0;
}
#endif

/* snd_SetupDmaTransfer: record the DMA buffer/count and write the two -0-header/
 * terminator slots, flushing both to memory. Not matched — multi-callee-save
 * 16-byte save-slot layout wall (same as USA). Portable #else body (EU lockstep
 * with USA snd_SetupDmaTransfer; data globals +0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_SetupDmaTransfer);
#else
void snd_SetupDmaTransfer(u8 *buffer, s32 count) {
    D_001A7504 = count;
    D_001A7500 = buffer;
    *(s32 *)(buffer + count * 4 + 4) = 0; /* terminator slot after the list */
    *(s32 *)(buffer + 0) = 0;             /* header slot */
    func_0011B3D0(buffer, buffer + 3);
    func_0011B3D0(buffer + count * 4 + 4, buffer + count * 4 + 4 + 3);
}
#endif

/* snd_BankLoadByLoc: request a sound-bank load from the IOP over the SIF RPC load
 * channel and block until the IOP posts the result. Returns the load handle, or 0
 * on early-out / RPC failure. Not matched — multi-callee-save frame wall (same as
 * USA). Portable #else body (EU lockstep with USA snd_BankLoadByLoc; globals +0x80).
 * The `bnel` stores D_001A7240 = arg0 only on the CheckLoadInProgress != 1 path. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_BankLoadByLoc);
#else
s32 snd_BankLoadByLoc(s32 arg0, s32 arg1) {
    D_001A7508 = 0;

    if (D_001A7548 != 0) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A7680);
        }
        return 0;
    }

    if (snd_CheckLoadInProgress(1) == 1) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A76B0);
        }
        return 0;
    }

    D_001A7240 = arg0;
    D_001A7244 = arg1;
    D_001A7200 = -1;

    while (sceSifCheckStatRpc(D_001A71C0) != 0) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A75F8);
        }
        snd_Pump();
        func_0011AEA0(0);
    }

    if (func_0011D620(D_001A71C0, 3, 1, &D_001A7240, 8, &D_001A7200, 4, 0, 0) < 0) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A76D0);
        }
        D_001A7508 = 0x106;
        return 0;
    }

    if (D_001A7200 == -1) {
        do {
            func_0011AEA0(0);
        } while (D_001A7200 == -1);
    }
    return D_001A7200;
}
#endif

/* snd_BankLoadAsync: fire-and-forget variant of snd_BankLoadByLoc — stage the
 * request (plus async context arg2/arg3), mark a load pending (D_001A7548 = 1),
 * issue the load RPC (fn 3), and return without waiting. Returns nothing. Not
 * matched — same frame wall as USA. Portable #else body (EU lockstep with USA
 * snd_BankLoadAsync; globals +0x80). Same `bnel` (D_001A7240 = arg0 only on the
 * proceed path); arg3 lands in D_001A7558 as a sign-extended 64-bit qword. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_BankLoadAsync);
#else
void snd_BankLoadAsync(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    D_001A7508 = 0;

    if (D_001A7548 != 0) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A7680);
        }
        return;
    }

    if (snd_CheckLoadInProgress(1) == 1) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A76B0);
        }
        return;
    }

    D_001A7240 = arg0;
    D_001A7244 = arg1;
    D_001A7200 = -1;
    D_001A7550 = arg2;
    D_001A7558 = arg3;

    while (sceSifCheckStatRpc(D_001A71C0) != 0) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A75F8);
        }
        snd_Pump();
        func_0011AEA0(0);
    }

    D_001A7548 = 1;
    func_0011D620(D_001A71C0, 3, 1, &D_001A7240, 8, &D_001A7200, 4, 0, 0);
}
#endif

/* func_00132640 (= USA func_001325E0): 4 bytes of inter-function fill before the
 * unrecoverable snd_BankLoadFromEE_CB body (reached only by fallthrough/data-ref,
 * so no .s for it). Pure fill, no C. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00132640);

/* snd_BankLoadFromEE_CB: unrecoverable body (see func_00132640). INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_BankLoadFromEE_CB);

/* snd_BankLoadFromIOP: request a bank already resident on the IOP (RPC fn 0x59,
 * single 4-byte arg) and block until the IOP posts the result. Returns the load
 * handle, or 0 on early-out / RPC failure. Simplest of the family (no
 * CheckLoadInProgress gate, no branch-likely). Not matched — same frame wall as
 * USA. Portable #else body (EU lockstep with USA snd_BankLoadFromIOP; +0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_BankLoadFromIOP);
#else
s32 snd_BankLoadFromIOP(s32 arg0) {
    D_001A7508 = 0;

    if (D_001A7548 != 0) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A77B0);
        }
        return 0;
    }

    D_001A7240 = arg0;
    D_001A7200 = -1;

    while (sceSifCheckStatRpc(D_001A71C0) != 0) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A75F8);
        }
        snd_Pump();
        func_0011AEA0(0);
    }

    if (func_0011D620(D_001A71C0, 0x59, 1, &D_001A7240, 4, &D_001A7200, 4, 0, 0) < 0) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A77E0);
        }
        D_001A7508 = 0x106;
        return 0;
    }

    if (D_001A7200 == -1) {
        do {
            func_0011AEA0(0);
        } while (D_001A7200 == -1);
    }
    return D_001A7200;
}
#endif

/* func_00132878 (= USA func_00132818): 0x10 bytes of inter-function padding
 * pinned by symbol_addrs size:0x10; the real wrapper begins at func_00132888.
 * Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00132878);

/**
 * func_00132888 (= USA func_00132828): invoke snd_QueueCommandToRing with
 * selector 8 and no payload, forwarding its return value (the value-return keeps
 * the frame and forces jal+jr rather than a sibling-call optimisation).
 */
s32 func_00132888(void) {
    return snd_QueueCommandToRing(8, 0, 0, 0, 0);
}

/**
 * func_001328B8 (= USA func_00132858): invoke snd_QueueCommandToRing with
 * selector 6, count 4, arg0 passed by address in a stack local.
 */
void func_001328B8(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(6, 4, &value, 0, 0);
}

/**
 * func_001328E8 (= USA func_00132888): invoke snd_QueueCommandToRing with
 * selector 9, count 8, and a stack record holding arg0 and arg1.
 */
void func_001328E8(s32 arg0, s32 arg1) {
    s32 args[2];
    args[0] = arg0;
    args[1] = arg1;
    snd_QueueCommandToRing(9, 8, args, 0, 0);
}

/* func_00132920 (= USA func_001328C0): builds a 0x1C-byte record (selector
 * 0x60) with an unaligned 24-byte copy the compiler won't emit from natural C
 * (near-miss). Portable #else body (EU lockstep with USA func_001328C0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00132920);
#else
s32 func_00132920(s32 cmd, const void *payload) {
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
 * func_00132998 (= USA func_00132938): invoke snd_QueueCommandToRing with
 * selector 0xB, count 4, arg0 passed by address in a stack local.
 */
void func_00132998(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0xB, 4, &value, 0, 0);
}

/* func_001329C8 (= USA func_00132968): 0x10 bytes of inter-function padding
 * pinned size:0x10; the real wrapper begins at func_001329D8. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_001329C8);

/**
 * func_001329D8 (= USA func_00132978): invoke snd_QueueCommandToRing with
 * selector 0xD, count 8, and a stack record holding arg0 and arg1.
 */
void func_001329D8(s32 arg0, s32 arg1) {
    s32 args[2];
    args[0] = arg0;
    args[1] = arg1;
    snd_QueueCommandToRing(0xD, 8, args, 0, 0);
}

/**
 * func_00132A10 (= USA func_001329B0): invoke snd_QueueCommandToRing with
 * selector 0x4E, count 0xC, and a stack record holding arg0, arg1 and arg2.
 */
void func_00132A10(s32 arg0, s32 arg1, s32 arg2) {
    s32 args[3];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    snd_QueueCommandToRing(0x4E, 0xC, args, 0, 0);
}

/* func_00132A50 (= USA func_001329F0): 0x20 bytes of inter-function padding
 * pinned size:0x20; the real wrapper begins at func_00132A70. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00132A50);

/**
 * func_00132A70 (= USA func_00132A10): invoke snd_QueueCommandToRing with
 * selector 0x11, count 0x18, and a 6-word stack record holding arg0..arg5;
 * arg6 and arg7 are forwarded as the two trailing arguments.
 */
void func_00132A70(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
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

/* func_00132AB8 (= USA func_00132A58): 0x18 bytes of inter-function padding
 * pinned size:0x18; the real wrapper begins at func_00132AD0. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00132AB8);

/**
 * func_00132AD0 (= USA func_00132A70): invoke snd_QueueCommandToRing with
 * selector 0x15, count 4, arg0 passed by address in a stack local.
 */
void func_00132AD0(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x15, 4, &value, 0, 0);
}

/* func_00132B00 (= USA func_00132AA0): 0x28 bytes of inter-function padding
 * pinned size:0x28; the real wrapper begins at func_00132B28. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00132B00);

/**
 * func_00132B28 (= USA func_00132AC8): invoke snd_QueueCommandToRing with
 * selector 0x18 and no payload, forwarding its return value.
 */
s32 func_00132B28(void) {
    return snd_QueueCommandToRing(0x18, 0, 0, 0, 0);
}

/**
 * func_00132B58 (= USA func_00132AF8): invoke snd_QueueCommandToRing with
 * selector 0x16, count 4, arg0 passed by address in a stack local.
 */
void func_00132B58(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x16, 4, &value, 0, 0);
}

/**
 * func_00132B88 (= USA func_00132B28): invoke snd_QueueCommandToRing with
 * selector 0x17, count 4, arg0 passed by address in a stack local.
 */
void func_00132B88(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x17, 4, &value, 0, 0);
}

/**
 * func_00132BB8 (= USA func_00132B58): invoke snd_QueueCommandToRing with
 * selector 0x19, count 4, arg0 passed by address in a stack local, and
 * arg1/arg2 forwarded as the two trailing arguments.
 */
void func_00132BB8(s32 arg0, s32 arg1, s32 arg2) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x19, 4, &value, arg1, arg2);
}

/* func_00132BE8 (= USA func_00132B88): 0x38 bytes of inter-function padding
 * pinned size:0x38; the real wrapper begins at func_00132C20. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00132BE8);

/**
 * func_00132C20 (= USA func_00132BC0 = snd_SetVoiceVolumeRamp): the 989snd EE
 * command-ring wrapper for cmd opcode 0x21 (set voice param / volume ramp).
 * Queues via snd_QueueCommandToRing with count 0x18 and a 6-word stack record
 * holding arg0..arg5; arg6/arg7 are the two trailing args.
 */
void func_00132C20(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
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

/* func_00132C68 (= USA func_00132C08): 0x40 bytes of inter-function fill before
 * snd_SendCommandSync. Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00132C68);

/* snd_SendCommandSync: assemble a 989snd command of `count` bytes and issue it
 * synchronously over the SIF RPC channel, blocking until the reply lands. arg0 is
 * the RPC function number, arg1 the parameter-byte count, arg2 the parameter
 * bytes. Returns the RPC reply status word (D_001A7104). Not matched — multi-
 * callee-save frame wall (same as USA). Portable #else body (EU lockstep with
 * USA snd_SendCommandSync; data globals +0x80). No branch-likely delay slots. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_SendCommandSync);
#else
s32 snd_SendCommandSync(s32 fno, s32 count, void *cmdBytes) {
    s32 i;

    /* copy the command's parameter bytes into the shared RPC send buffer */
    for (i = 0; i < count; i++) {
        D_001A7140[i] = ((u8 *)cmdBytes)[i];
    }

    /* drain any in-flight command-ring DMA before reusing the channel */
    while (D_001A7500 != 0) {
        snd_Pump();
        func_0011AEA0(0);
    }

    /* kick the DMA transfer, then wait for the SIF RPC channel to go idle */
    snd_SetupDmaTransfer(D_001A7100, 1);
    while (sceSifCheckStatRpc(D_001A70C0) != 0) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A75F8);
        }
        snd_Pump();
        func_0011AEA0(0);
    }

    /* issue the synchronous RPC (mode 1); a zero-length command sends no buffer */
    if (count != 0) {
        func_0011D620(D_001A70C0, fno, 1, D_001A7140, count, D_001A7100, 0xC, 0, 0);
    } else {
        func_0011D620(D_001A70C0, fno, 1, 0, 0, D_001A7100, 0xC, 0, 0);
    }

    /* spin until the completion service confirms the reply landed */
    while (snd_ServiceRpcCompletion() == 0) {
    }

    /* if the active ring buffer still holds queued entries and no service is
     * pending, flush it now */
    if (*D_001A7520[D_001A7540] != 0 && D_001A7544 == 0) {
        snd_FlushCommandRing();
    }
    return D_001A7104[0];
}
#endif

/* snd_QueueCommandToRing: append one command to the active double-buffered 989snd
 * ring (or, when idle with an empty command, issue a bare sync RPC). Not matched
 * — multi-callee-save frame wall (same as USA). Portable #else body (EU lockstep
 * with USA snd_QueueCommandToRing; data globals +0x80, incl. the rodata strings).
 * No branch-likely delay slots.
 *
 * Ring model (buffer i = D_001A7540): D_001A7520[i] -> a 0x1000-byte command
 * buffer (leading word = entry count, rest packed (u16 sel, u16 count, payload));
 * D_001A7528[i] = free bytes (write cursor = base + 0x1000 - free); D_001A7530[i]
 * -> 16-byte descriptors (word0 = arg3, qword@8 = arg4). Cap 0x100 entries. arg4
 * is 32-bit here but lands as a sign-extended 64-bit qword. D_001A7544 is a
 * service-pending flag borrowed (cleared then restored) while spinning. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_QueueCommandToRing);
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
    if (D_001A7544 == 0 && D_001A7500 == 0 && count == 0 && arg3 == 0) {
        snd_SetupDmaTransfer(D_001A7100, 1);
        while (sceSifCheckStatRpc(D_001A70C0) != 0) {
            if (D_001A7578 == 0) {
                snd_PrintError(D_001A75F8);
            }
            snd_Pump();
            func_0011AEA0(0);
        }
        return func_0011D620(D_001A70C0, sel, 1, 0, 0, D_001A7100, 0xC, 0, 0);
    }

    /* bytes this command occupies: payload rounded up to a multiple of 4, plus
     * the 4-byte (sel, count) header */
    alignedSize = ((count + 3) & ~3) + 4;

    /* wait until the active buffer has room and isn't at its 0x100-entry cap,
     * pumping the sound system while we spin */
    idx = D_001A7540;
    if (*D_001A7520[idx] == 0x100 || D_001A7528[idx] < alignedSize) {
        for (;;) {
            if (D_001A7544 != 0) {
                D_001A7544 = 0;
                raisedFlag = 1;
            }
            snd_Pump();
            if (spins == 1 && D_001A7578 == 0) {
                idx = D_001A7540;
                snd_PrintError(D_0013C078, idx, *D_001A7520[idx]);
            }
            idx = D_001A7540;
            if (*D_001A7520[idx] == 0x100) {
                spins++;
                continue;
            }
            if (D_001A7528[idx] < alignedSize) {
                continue;
            }
            break;
        }
    }
    if (spins != 0 && D_001A7578 == 0) {
        snd_PrintError(D_001A7950, spins);
    }
    if (raisedFlag != 0) {
        D_001A7544 = 1; /* restore the service flag we borrowed */
    }

    /* append the command bytes at the buffer's write cursor: 2-byte selector,
     * 2-byte length, then the payload */
    idx = D_001A7540;
    cursor = (u8 *)D_001A7520[idx] + (0x1000 - D_001A7528[idx]);
    *(u16 *)cursor = sel;
    cursor += 2;
    *(u16 *)cursor = count;
    cursor += 2;
    for (i = 0; i < count; i++) {
        cursor[i] = ((u8 *)data)[i];
    }

    /* consume the space, write the parallel 16-byte descriptor at the next free
     * slot (arg3 word + arg4 qword), and commit */
    D_001A7528[idx] -= alignedSize;
    entry = D_001A7530[idx] + *D_001A7520[idx] * 16;
    *(s32 *)entry = arg3;
    *(s64 *)(entry + 8) = arg4;
    return snd_CommitRingEntry();
}
#endif

/**
 * snd_CommitRingEntry - commit one queued command to the active 989snd ring
 * buffer: bump the entry count of the ring selected by D_001A7540 (through the
 * per-buffer count pointers in D_001A7520) and run snd_Pump to service it,
 * forwarding snd_Pump's return value.
 */
s32 snd_CommitRingEntry(void) {
    s32 *entryCount = D_001A7520[D_001A7540];
    *entryCount += 1;
    return snd_Pump();
}

/* snd_FlushCommandRing: DMA-kick + SIF-RPC flush of the active 989snd command
 * ring buffer, then flip to the other buffer. Not matched — multi-callee-save
 * frame wall (same as USA). Portable #else body (EU lockstep with USA
 * snd_FlushCommandRing; data globals +0x80). No branch-likely delay slots. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_FlushCommandRing);
#else
void snd_FlushCommandRing(void) {
    s32 idx = D_001A7540;

    /* kick the DMA transfer for the buffer currently being filled */
    snd_SetupDmaTransfer(D_001A7538[idx], *D_001A7520[idx]);

    /* wait for the SIF RPC command channel to go idle */
    while (sceSifCheckStatRpc(D_001A70C0) != 0) {
        if (D_001A7578 == 0) {
            snd_PrintError(D_001A75F8);
        }
        func_0011AEA0(0);
    }

    /* fire the ring-flush RPC (function 0x4D) for the active buffer */
    idx = D_001A7540;
    func_0011D620(D_001A70C0, 0x4D, 1,
                  D_001A7520[idx],              /* send buffer   */
                  0x1000 - D_001A7528[idx],     /* send size     */
                  D_001A7538[idx],              /* receive buffer*/
                  (*D_001A7520[idx] << 2) + 8,  /* receive size  */
                  0, 0);                        /* no completion callback */

    /* flip to the other buffer and reset it for refilling */
    idx = (D_001A7540 ^ 1) != 0 ? 1 : 0;
    D_001A7540 = idx;
    *D_001A7520[idx] = 0;
    D_001A7528[idx] = 0xFFC;
}
#endif

/* func_00133280 (= USA func_00133220): raise the command-ring service-pending
 * flag D_001A7544 and return 1, reusing one register for the store and the
 * return. Not matched even at -G8 (cc1 materialises the constant twice rather
 * than storing the return register itself) — a store-constant-and-return-it
 * register-allocation near-miss. Portable #else body (EU lockstep). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133280);
#else
s32 func_00133280(void) {
    return D_001A7544 = 1; /* raise the command-ring service-pending flag */
}
#endif

/**
 * func_00133290 (= USA func_00133230): clear the command-ring service-pending
 * flag D_001A7544 (set by func_00133280) and run one snd_Pump service pass,
 * forwarding its return value.
 */
s32 func_00133290(void) {
    D_001A7544 = 0;
    return snd_Pump();
}

/* func_001332B0 (= USA func_00133250): wait for IOP readiness, then send sync
 * command 0x2A (driver bring-up) with a 4-word record and cache the result in
 * the readiness flag D_001A750C. Blocked by a jal snd_Pump spin loop padded with
 * literal nops plus the multi-callee-save save-slot wall (near-miss). Portable
 * #else body (EU lockstep with USA func_00133250; data globals +0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_001332B0);
#else
s32 func_001332B0(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 cmd[4];

    if (D_001A750C == 1) {
        return 0; /* IOP driver already up */
    }
    if (D_001A7548 != 0) {
        while (snd_Pump() != 0) { } /* drain the pending command ring */
    }
    snd_CheckLoadInProgress(0); /* block until any in-flight load finishes */
    cmd[0] = arg0;
    cmd[1] = arg1;
    cmd[2] = arg2;
    cmd[3] = arg3;
    D_001A750C = snd_SendCommandSync(0x2A, 0x10, cmd); /* 0x2A = bring-up */
    return D_001A750C;
}
#endif

/* func_00133360 (= USA func_00133300): 0x10 bytes of inter-function padding
 * pinned size:0x10; the real wrapper begins at func_00133370. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133360);

/**
 * func_00133370 (= USA func_00133310): invoke snd_QueueCommandToRing with
 * selector 0x34 and no payload, forwarding its return value.
 */
s32 func_00133370(void) {
    return snd_QueueCommandToRing(0x34, 0, 0, 0, 0);
}

/* func_001333A0 (= USA func_00133340): 0x10 bytes of inter-function padding
 * pinned size:0x10; the real start begins at func_001333B0 (snd_PlaySample).
 * Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_001333A0);

/* func_001333B0 (= USA snd_PlaySample): 989snd cmd opcode 0x2C (start voice /
 * play sample). Builds a 0x20-byte record from a mix of register and stack args
 * (16-bit fields packed hi|lo, plus four words from the caller's stack) for
 * snd_QueueCommandToRing count 0x20. The 8+ argument calling convention this cc1
 * won't reproduce from natural C (near-miss). Portable #else body (EU lockstep
 * with USA snd_PlaySample; zero data globals, so identical C). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_001333B0);
#else
s32 func_001333B0(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5,
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

/* func_00133420 (= USA func_001333C0): 0x10 bytes of inter-function padding
 * pinned size:0x10; the real wrapper begins at func_00133430. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133420);

/**
 * func_00133430 (= USA func_001333D0): invoke snd_QueueCommandToRing with
 * selector 0x2D, count 4, arg0 passed by address in a stack local.
 */
void func_00133430(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x2D, 4, &value, 0, 0);
}

/**
 * func_00133460 (= USA func_00133400 = snd_StopVoice): the 989snd EE
 * command-ring wrapper for cmd opcode 0x2E (stop voice). Queues via
 * snd_QueueCommandToRing with count 4 and arg0 (the voice handle) passed by
 * address in a stack local.
 */
void func_00133460(s32 arg0) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x2E, 4, &value, 0, 0);
}

/**
 * func_00133490 (= USA func_00133430): invoke snd_QueueCommandToRing with
 * selector 0x32, count 4, arg0 passed by address in a stack local, and
 * arg1/arg2 forwarded as the two trailing arguments.
 */
void func_00133490(s32 arg0, s32 arg1, s32 arg2) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x32, 4, &value, arg1, arg2);
}

/**
 * func_001334C0 (= USA func_00133460): invoke snd_QueueCommandToRing with
 * selector 0x4F, count 4, arg0 passed by address in a stack local, and
 * arg1/arg2 forwarded as the two trailing arguments.
 */
void func_001334C0(s32 arg0, s32 arg1, s32 arg2) {
    s32 value = arg0;
    snd_QueueCommandToRing(0x4F, 4, &value, arg1, arg2);
}

/**
 * func_001334F0 (= USA func_00133490): invoke snd_SendCommandSync with selector
 * 0x36 and count 4, passing arg0 by address in a stack local as the data
 * argument.
 */
void func_001334F0(s32 arg0) {
    s32 value = arg0;
    snd_SendCommandSync(0x36, 4, &value);
}

/* CdStartRead (= USA CdStartRead): queue ring command 0x38 (start read) with a
 * 3-word record when the IOP driver is up, else fall back to func_001253A8.
 * Blocked by the multi-callee-save save-slot wall plus the D_001A7180/D_001A7190
 * $at-macro absolute stores (cc1-small / assembler-absolute disagreement)
 * (near-miss). Portable #else body (EU lockstep with USA CdStartRead; data
 * globals +0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", CdStartRead);
#else
/* rmode is not used by the ring-command path — it exists only to be forwarded to
 * sceCdRead. Naming it is what makes that forwarding a CONTRACT: the ROM passes
 * $4-$7 straight through, and every caller (EU 0x002B871C / 0x002B8810 / the
 * asm-only CdReadSync at 0x00133AA8) supplies a real sceCdRMode* in $7. */
s32 CdStartRead(s32 arg0, s32 arg1, s32 arg2, void *rmode) {
    s32 cmd[3]; /* the three command words for the 0x38 read request */

    if (D_001A750C == 0) {
        /* IOP driver down -> direct libcdvd. Pass all four through, as the ROM does. */
        return func_001253A8(arg0, arg1, arg2, rmode);
    }
    if (snd_CheckLoadInProgress(1) == 1) {
        return 0; /* a load is already in flight */
    }
    D_001A7180 = 1;             /* mark a load in progress */
    D_001A7190 = 0;             /* clear the EE-side load status */
    func_0011B3D0(&D_001A7180, &D_001A71BF);
    cmd[0] = arg0;
    cmd[1] = arg1;
    cmd[2] = arg2;
    D_001A7514 = 1;
    D_001A7518 = 0;
    snd_QueueCommandToRing(0x38, 0xC, cmd, 0, 0);
    return 1;
}
#endif

/* snd_CheckLoadInProgress: test/wait on the bank-load-in-progress flag D_001A7180
 * (via func_0011B500 + snd_Pump), caching the result in D_001A7518. When noWait
 * is set, report progress without blocking; otherwise pump the sound engine until
 * the load finishes. Blocked by the re-materialised lui-macro loop plus the
 * multi-callee-save wall (same as USA) (near-miss). Portable #else body (EU
 * lockstep with USA snd_CheckLoadInProgress; data globals +0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_CheckLoadInProgress);
#else
s32 snd_CheckLoadInProgress(s32 noWait) {
    s32 done;

    if (D_001A750C == 0) {
        return func_00124B88(); /* IOP driver down -> direct RPC status */
    }
    func_0011B500(&D_001A7180, &D_001A71BF);
    D_001A7518 = done = (D_001A7180 == 0);
    if (done) {
        return 0; /* load already complete */
    }
    if (noWait == 1) {
        return 1; /* still loading, caller asked not to block */
    }
    do { /* block: pump the sound engine until the load finishes */
        snd_Pump();
        func_0011B500(&D_001A7180, &D_001A71BF);
        D_001A7518 = done = (D_001A7180 == 0);
    } while (!done);
    return 0;
}
#endif

/**
 * func_001336A0 (= USA CdStopRead): stop the CD streaming read. When the IOP
 * sound/loader driver is up (D_001A750C), queue ring command 0x37 (stop read)
 * and report success (1); otherwise fall back to the direct libcdvd path
 * func_00125620 and forward its result.
 */
s32 func_001336A0(void) {
    if (D_001A750C) {
        snd_QueueCommandToRing(0x37, 0, 0, 0, 0);
        return 1;
    }
    return func_00125620();
}

/* CdGetLoadStatus (= USA CdGetLoadStatus): return the cached EE-side load status
 * D_001A7190 when the IOP driver is up, else fall back to QueryCdStatusOverRpc. Blocked
 * by a symbolic-lw-macro absolute expansion the GNU cc1 won't schedule correctly
 * (near-miss). Portable #else body (EU lockstep with USA CdGetLoadStatus). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", CdGetLoadStatus);
#else
s32 CdGetLoadStatus(void) {
    if (D_001A750C == 0) {
        return QueryCdStatusOverRpc(); /* IOP driver down -> libcdvd status via RPC */
    }
    return D_001A7190;
}
#endif

/* func_00133720 (= USA func_001336C0): 0x10 bytes of inter-function padding
 * pinned size:0x10; the real function begins at func_00133730
 * (SetSndPumpCallback). Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133720);

/**
 * func_00133730 (= USA SetSndPumpCallback): install a new snd_Pump tick
 * callback. When the IOP sound/loader driver is up (D_001A750C), swap the
 * gp-relative callback slot D_001A7510 to `callback` and return the previous
 * one; otherwise defer to the pre-init path func_001245D0 and forward its
 * result.
 */
void *func_00133730(void *callback) {
    if (D_001A750C) {
        void *previous = D_001A7510;
        D_001A7510 = callback;
        return previous;
    }
    return func_001245D0(callback);
}

/* func_00133760 (= USA func_00133700): 0x10 bytes of inter-function padding
 * pinned size:0x10; the real wrapper begins at func_00133770. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133760);

/**
 * func_00133770 (= USA func_00133710): invoke snd_QueueCommandToRing with
 * selector 0x50, count 0x14, and a 5-word stack record holding arg0..arg4.
 */
void func_00133770(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 args[5];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    args[4] = arg4;
    snd_QueueCommandToRing(0x50, 0x14, args, 0, 0);
}

/**
 * func_001337B0 (= USA func_00133750): invoke snd_QueueCommandToRing with
 * selector 0x51, count 8, and a stack record holding arg0 and arg1.
 */
void func_001337B0(s32 arg0, s32 arg1) {
    s32 args[2];
    args[0] = arg0;
    args[1] = arg1;
    snd_QueueCommandToRing(0x51, 8, args, 0, 0);
}

/**
 * func_001337E8 (= USA func_00133788): invoke snd_QueueCommandToRing with
 * selector 0x10, count 0x10 (16 bytes), and a stack record of four words
 * (arg0..arg3).
 */
void func_001337E8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 args[4];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    snd_QueueCommandToRing(0x10, 0x10, args, 0, 0);
}

/* func_00133828 (= USA func_001337C8): 0x28 bytes of inter-function padding
 * pinned size:0x28; the real wrapper begins at func_00133850. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133828);

/**
 * func_00133850 (= USA func_001337F0): invoke snd_SendCommandSync with selector
 * 0x4A and no payload, forwarding its return value.
 */
s32 func_00133850(void) {
    return snd_SendCommandSync(0x4A, 0, 0);
}

/* func_00133878 (= USA func_00133818): 0x8 bytes of inter-function padding
 * pinned size:0x8; the real wrapper begins at func_00133880. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133878);

/**
 * func_00133880 (= USA func_00133820): invoke snd_SendCommandSync with selector
 * 0x4B and no payload, forwarding its return value.
 */
s32 func_00133880(void) {
    return snd_SendCommandSync(0x4B, 0, 0);
}

/* func_001338A8 (= USA func_00133848): 0x8 bytes of inter-function padding
 * pinned size:0x8; the real wrapper begins at func_001338B0. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_001338A8);

/**
 * func_001338B0 (= USA func_00133850): invoke snd_SendCommandSync with selector
 * 0x3B and count 0x18 (24 bytes), passing a 6-word stack record (arg0..arg5) as
 * the data argument.
 */
void func_001338B0(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
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
 * func_001338F0 (= USA func_00133890): invoke snd_SendCommandSync with selector
 * 0x3D and no payload, forwarding its return value.
 */
s32 func_001338F0(void) {
    return snd_SendCommandSync(0x3D, 0, 0);
}

/* func_00133918 (= USA func_001338B8): 0x10 bytes of inter-function padding
 * pinned size:0x10; the real wrapper begins at func_00133928. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133918);

/**
 * func_00133928 (= USA func_001338C8): invoke snd_SendCommandSync with selector
 * 0x3C and no payload, forwarding its return value.
 */
s32 func_00133928(void) {
    return snd_SendCommandSync(0x3C, 0, 0);
}

/**
 * func_00133950 (= USA func_001338F0): invoke snd_SendCommandSync with selector
 * 0x3E and count 0x14 (20 bytes), passing a stack record of five words
 * (arg0..arg4) as the data argument.
 */
void func_00133950(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 args[5];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    args[4] = arg4;
    snd_SendCommandSync(0x3E, 0x14, args);
}

/* func_00133988 (= USA func_00133928): 0x8 bytes of inter-function padding
 * pinned size:0x8; the real wrapper begins at func_00133990. Pure padding. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133988);

/**
 * func_00133990 (= USA func_00133930): invoke snd_SendCommandSync with selector
 * 0x5A and count 8, passing a 2-word stack record (arg0, arg1) as the data
 * argument.
 */
void func_00133990(s32 arg0, s32 arg1) {
    s32 args[2];
    args[0] = arg0;
    args[1] = arg1;
    snd_SendCommandSync(0x5A, 8, args);
}

/**
 * func_001339C0 (= USA func_00133960): invoke snd_SendCommandSync with selector
 * 0x5B and no payload, forwarding its return value.
 */
s32 func_001339C0(void) {
    return snd_SendCommandSync(0x5B, 0, 0);
}

/* func_001339E8 (= USA func_00133988): scale arg0 by 1524/741; the only diff is
 * a mflo delay-slot scheduling choice not expressible in source (near-miss).
 * Portable #else body (EU lockstep with USA func_00133988). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_001339E8);
#else
s32 func_001339E8(s32 x) {
    return x * 0x5F4 / 0x2E5;
}
#endif

/* OnVblankInterrupt (= USA OnVblankInterrupt): bumps the 64-bit tick counter and
 * snapshots the T1_COUNT timer; a timer-address `ori` scheduling residue this
 * cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", OnVblankInterrupt);

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
extern s32 func_00125588(void);
extern s32 func_00125620(void);
extern void *func_001245D0(void *arg0);

/* 989snd small-data globals (gp-relative in the original; complete <=8-byte
 * declarations so -G8 places them in small data). EU addresses (= USA +0x80). */
extern s32 D_001A7544;     /* command-ring service-pending flag (USA D_001A74C4) */
extern s32 *D_001A7520[2]; /* double-buffered ring entry-count pointers (USA D_001A74A0) */
extern s32 D_001A7540;     /* active ring buffer index (0/1) (USA D_001A74C0) */
extern s32 D_001A750C;     /* nonzero when the IOP sound/loader driver is up (USA g_sndIopReady) */
extern void *D_001A7510;   /* snd_Pump tick callback (USA D_001A7490) */

/* snd_ServiceRpcCompletion: branch-layout shape this cc1 won't reproduce (same
 * wall as USA). Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_ServiceRpcCompletion);

/* snd_SetupDmaTransfer: multi-callee-save 16-byte save-slot layout wall (same
 * as USA snd_SetupDmaTransfer). Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_SetupDmaTransfer);

/* snd_BankLoadByLoc: multi-callee-save frame wall. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_BankLoadByLoc);

/* snd_BankLoadAsync: multi-callee-save frame wall. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_BankLoadAsync);

/* func_00132640 (= USA func_001325E0): 4 bytes of inter-function fill before the
 * unrecoverable snd_BankLoadFromEE_CB body (reached only by fallthrough/data-ref,
 * so no .s for it). Pure fill, no C. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00132640);

/* snd_BankLoadFromEE_CB: unrecoverable body (see func_00132640). INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_BankLoadFromEE_CB);

/* snd_BankLoadFromIOP: multi-callee-save frame wall. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_BankLoadFromIOP);

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
 * 0x60) with an unaligned 24-byte copy the compiler won't emit from natural C.
 * Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00132920);

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

/* snd_SendCommandSync: multi-callee-save frame wall. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_SendCommandSync);

/* snd_QueueCommandToRing: multi-callee-save frame wall. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_QueueCommandToRing);

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

/* snd_FlushCommandRing: multi-callee-save frame wall. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_FlushCommandRing);

/* func_00133280 (= USA func_00133220): store-constant-and-return-it
 * register-allocation shape this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133280);

/**
 * func_00133290 (= USA func_00133230): clear the command-ring service-pending
 * flag D_001A7544 (set by func_00133280) and run one snd_Pump service pass,
 * forwarding its return value.
 */
s32 func_00133290(void) {
    D_001A7544 = 0;
    return snd_Pump();
}

/* func_001332B0 (= USA func_00133250): IOP-readiness spin loop padded with
 * literal nops plus the multi-callee-save wall. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_001332B0);

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
 * play sample), uses an 8+ argument calling convention this cc1 won't reproduce
 * from natural C. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_001333B0);

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

/* func_00133518 (= USA CdStartRead): multi-callee-save wall plus the
 * cc1-small/assembler-absolute $at-macro store wall. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133518);

/* snd_CheckLoadInProgress: re-materialised lui-macro loop + multi-callee-save
 * wall (same as USA). Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", snd_CheckLoadInProgress);

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

/* func_001336E8 (= USA CdGetLoadStatus): symbolic-lw-macro absolute expansion
 * the GNU cc1 won't schedule correctly. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_001336E8);

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
 * a mflo delay-slot scheduling choice not expressible in source. INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_001339E8);

/* func_00133A10 (= USA OnVblankInterrupt): bumps the 64-bit tick counter and
 * snapshots the T1_COUNT timer; a timer-address `ori` scheduling residue this
 * cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/0321A0", func_00133A10);

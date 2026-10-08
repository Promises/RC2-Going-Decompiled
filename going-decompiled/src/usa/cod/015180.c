#include "common.h"

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

extern s32 D_00133E74;

/* memcmp, memcpy, memset, strcmp and strncpy, which sat between this unit's
 * head (now cod/015180h) and func_00115C90, are linked from newlib's verbatim
 * hand-written R5900 .S (going-decompiled/libc/, task #1884, RULING #9817),
 * as are strcpy and strlen (task #1925); func_001158F8 between strlen and
 * strncpy is cod/015878. This unit keeps the name
 * cod/015180 for the rest (0x115C90..0x11FC67). */

/* func_00115C90: clears D_00133E78, calls func_0011B270(arg1); on failure
 * (-1) writes the resulting D_00133E78 error code back through arg0. Logic
 * matches but NOT byte-exact: the original saves $16/$17/$31 with 128-bit `sq`
 * while this cc1 emits `sd` for callee-saves. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115C90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115CF0);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115D38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", snd_PrintError);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sprintf);
INCLUDE_ASM_ALIAS(func_00115DA8, sprintf);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", abort);
INCLUDE_ASM_ALIAS(func_00115E28, abort);

/**
 * AssertFail (EU names it) — the SDK assert handler. NEVER RETURNS.
 *
 * Prints the standard C assertion diagnostic and aborts:
 *     assertion "%s" failed: file "%s", line %d
 * read verbatim from the ROM at 0x13A370.
 *
 * IT HAS NO EPILOGUE AND NO `jr $ra`, AND THAT IS CORRECT. An assert handler does not
 * return; the tail call to func_00115E28 (the abort) is the end of it. Do not "repair"
 * the missing return — I previously mistook this shape for a mid-function split and was
 * wrong. EU's disassembled copy has zero return instructions either.
 *
 * PARAMETER ORDER IS UNUSUAL AND IS DETERMINED BY THE REGISTER SHUFFLE, not guessed:
 *     $4 (file) -> $7  = the format's SECOND conversion, %s "file"
 *     $5 (line) -> $8  = the format's THIRD  conversion, %d "line"
 *     $6 (expr) stays  = the format's FIRST  conversion, %s the assertion text
 * so the third parameter is passed straight through into the vararg area untouched.
 * That is why $6 appears "never written" when reading only this function: it is an
 * INPUT, not a gap. func_00115CF0's prologue confirms the varargs shape — it spills
 * $6..$11 to contiguous slots, re-points $6 at that area as a va_list, spills
 * $f12/$f14/$f16/$f18 for float varargs, and tail-calls its vfprintf worker.
 */
#ifndef TARGET_NATIVE
// recovered splat-dropped code (epilogue-stump mis-split): raw words, byte-exact
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", AssertFail);
#else
/* 0x13A370 — "assertion \"%s\" failed: file \"%s\", line %d\n" */
extern const char D_0013A370[];
/* printf-like: (stream, fmt, ...) — varargs, forwards a va_list to its vfprintf. */
extern void func_00115CF0(void *stream, const char *fmt, ...);
/* the abort tail — never returns. */
extern void func_00115E28(void);

void AssertFail(const char *file, s32 line, const char *expr) {
    /* stream = D_00133E74[+0xC] — the same global the accessor at the top of this
     * unit returns; +0xC is its stderr-equivalent handle. */
    func_00115CF0(*(void **)(D_00133E74 + 0xC), D_0013A370, expr, file, line);
    func_00115E28();
}
#endif

/* func_00115E68: tail-calls func_001175F0(arg0, 0, 0xA) and returns its result
 * sign-extended from 32 to 64 bits. Not matched: the original saves $31 with a
 * 128-bit `sq` (not the `sd` ee-gcc emits here at -O2 -G0) and carries an extra
 * dsll32/dsra32 sign-extend that cc1 elides for an s32-returning callee. Both
 * are codegen/ABI forms this compiler won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115E68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", exit_runAtexitHandlers);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", malloc);
INCLUDE_ASM_ALIAS(func_00115F28, malloc);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115F78);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115FC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00116300);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00116360);

/**
 * Seed the random-number generator: store arg0 as the RNG state word at
 * D_00133E74 + 0x58 (the seed consumed by func_001163B0).
 */
void func_001163A0(s32 arg0) {
    *(s32 *)(D_00133E74 + 0x58) = arg0;
}

/**
 * Linear-congruential RNG. Advances the 32-bit state at D_00133E74 + 0x58 with
 * the classic glibc constants (state = state*0x41C64E6D + 0x3039) and returns
 * the new state masked to 31 bits (non-negative).
 */
s32 func_001163B0(void) {
    s32 *p = (s32 *)(D_00133E74 + 0x58);
    s32 v = *p * 0x41C64E6D + 0x3039;
    *p = v;
    return v & 0x7FFFFFFF;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001163E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00116460);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001166C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00116E10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00117108);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00117278);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001175F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00117650);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00117848);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00117CA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001182B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00118460);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001184D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00118548);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001185E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00118BC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", VfprintfDispatch);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", VfprintfFloat);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00119AC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00119BC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", VfprintfInteger);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A7F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A810);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A820);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A830);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A840);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A850);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A860);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A870);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A880);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A890);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A8A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A8B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A8C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A8D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A8E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A8F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A900);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A910);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A920);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A930);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A940);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A950);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A960);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A970);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A980);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A990);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A9A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A9B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A9C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A9D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A9E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011A9F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA10);

/**
 * func_0011AA20 = EE kernel syscall 0x20 (CreateThread).
 * SCE library stub: load the syscall number into $v1 and trap into the EE
 * kernel; the kernel returns its result in $v0 (no register move emitted, so
 * the C body is the bare inline-asm trap). Kept under the splat func_ name so
 * objdiff pairs it by symbol against the frozen asm. Takes a ThreadParam* and
 * returns the new thread id; the syscall result is left in $v0 by the trap.
 */
s32 func_0011AA20(struct ThreadParam *param) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x20\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA30);

/**
 * func_0011AA40 = EE kernel syscall 0x22 (StartThread).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel's result is returned in $v0. Takes the thread id
 * and its start argument.
 */
s32 func_0011AA40(s32 thid, void *arg) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x22\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAA0);

/**
 * func_0011AAB0 = EE kernel syscall 0x29 (ChangeThreadPriority).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel sets thread `thid` to priority `priority` and
 * returns the previous priority (or a negative error) in $v0. The ROM body is
 * `addiu $v1,$0,0x29; syscall`. 0x29 is ChangeThreadPriority in the EE kernel
 * numbering (external SDK knowledge; DeleteSema 0x41 / SignalSema 0x42 /
 * WaitSema 0x44 / FlushCache 0x64 sit in the same table), and both callers use
 * the SDK idiom ChangeThreadPriority(GetThreadId(), 1): func_0011B800 here and
 * PlayFmvMovie (text/24D728), each passing func_0011AB10 = syscall 0x2F
 * (GetThreadId). This stub was formerly labelled RotateThreadReadyQueue, which
 * is 0x2B and takes one argument. Value-returning as the SDK declares it
 * (`int ChangeThreadPriority(int, int)`, include/rtl/ee/eekernel.h); both
 * callers discard the result (task #1848).
 */
s32 func_0011AAB0(s32 thid, s32 priority) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x29\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAC0);

/**
 * func_0011AAD0 = EE kernel syscall 0x2B (RotateThreadReadyQueue).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel rotates the ready queue at priority `priority` and
 * returns its result in $v0. The ROM body is `addiu $v1,$0,0x2B; syscall`;
 * 0x2B is RotateThreadReadyQueue in the EE kernel numbering (external SDK
 * knowledge; ReleaseWaitThread, which this stub was formerly labelled, is
 * 0x2D). Called by the deferred-request worker func_0011B728 (op 1) and as
 * func_0011AAD0(1) by text/250080 (task #1848).
 */
s32 func_0011AAD0(s32 priority) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x2B\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB00);

/**
 * func_0011AB10 = EE kernel syscall 0x2F (GetThreadId).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel returns the current thread id in $v0.
 */
s32 func_0011AB10(void) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x2F\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB40);

/**
 * func_0011AB50 = EE kernel syscall 0x33 (WakeupThread).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel wakes the sleeping thread and returns its result in
 * $v0. Called by func_0011B728 with a thread id.
 */
s32 func_0011AB50(s32 thid) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x33\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB80);

/**
 * func_0011AB90 = EE kernel syscall 0x37 (SuspendThread).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel suspends the given thread and returns its result in
 * $v0. Called by func_0011B728 with a thread id.
 */
s32 func_0011AB90(s32 thid) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x37\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ABA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ABB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ABC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ABD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ABE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ABF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC10);

/**
 * func_0011AC20 = EE kernel syscall 0x40 (CreateSema).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel creates a semaphore from the descriptor in $a0 and
 * returns the new semaphore id in $v0.
 */
s32 func_0011AC20(s32 *desc) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x40\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

/**
 * func_0011AC30 = EE kernel syscall 0x41 (DeleteSema).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel deletes the semaphore identified by $a0 and returns
 * its id (or a negative error) in $v0. Value-returning as the SDK declares it
 * (`int DeleteSema(int)`, include/rtl/ee/eekernel.h); a caller that sees it
 * declared `void` frees $v0 and schedules differently (func_00124818, #1840).
 */
s32 func_0011AC30(s32 obj) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x41\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC50);

/**
 * func_0011AC60 = EE kernel syscall 0x44 (WaitSema).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel blocks the caller until the semaphore can be taken
 * and returns the semaphore id in $v0. Used as the table-lock acquire (see
 * func_0011D868 / func_0011AC40 release) and by the worker loop func_0011B728.
 */
s32 func_0011AC60(s32 sema) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x44\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ACA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ACB0);

/**
 * func_0011ACC0 = EE kernel syscall 0x4A (SetOsdConfigParam). SCE library
 * syscall stub (see func_0011AA20): load the syscall number into $v1 and
 * trap; the kernel takes the OSD configuration word from the $a0 pointer (see
 * func_0011F8D0). The ROM body is `addiu $v1,$0,0x4A; syscall`. 0x4A is
 * SetOsdConfigParam in the EE kernel numbering (external SDK knowledge; its
 * 0x4B partner is func_0011ACD0, NOTE #9692). The vendored eekernel.h has no
 * prototype; void matches ps2sdk's `void SetOsdConfigParam(ConfigParam *)`
 * (task #1848).
 */
void func_0011ACC0(s32 *config) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x4A\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

/**
 * func_0011ACD0 = EE kernel syscall 0x4B (GetOsdConfigParam). SCE library
 * syscall stub (see func_0011AA20): load the syscall number into $v1 and
 * trap; the kernel stores the OSD configuration word through the $a0 pointer
 * (see func_0011F8D0, and the screenType / timezone readers in cod/022FA8).
 * The ROM body is `addiu $v1,$0,0x4B; syscall`. 0x4B is GetOsdConfigParam in
 * the EE kernel numbering (external SDK knowledge, NOTE #9692 section 4). The
 * vendored eekernel.h has no prototype; void matches ps2sdk's
 * `void GetOsdConfigParam(ConfigParam *)` (task #1848).
 */
void func_0011ACD0(s32 *config) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x4B\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ACE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ACF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AD00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AD10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AD20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AD30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AD40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AD50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AD60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AD70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AD80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AD90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ADA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ADB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ADC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ADD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ADE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ADF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AE00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AE10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AE20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AE30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AE40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AE50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AE60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AE70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AE80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AE90);

/**
 * func_0011AEA0 = FlushCache: EE kernel syscall 0x64 (100), the name confirmed
 * from the ROM's syscall immediate (FACT #3909). SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Takes one
 * argument in $a0, the cache-flush mode.
 * void: the ROM body (0x11AEA0 addiu $v1,$0,0x64; syscall; jr $ra; nop)
 * writes only $v1, so $v0 at jr $ra is whatever the kernel left, not a value
 * this function computes, and none of its 100 ROM call sites reads $v0
 * (task #1462, RULING #9122).
 */
void func_0011AEA0(s32 mode) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x64\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AEB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AEC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AED0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AEE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AEF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AF00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AF10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AF20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AF30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AF40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AF50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AF60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AF70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AF80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AF90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AFA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AFB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AFC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AFD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AFE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AFF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B000);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B010);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B020);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B030);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B040);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B050);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B060);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B070);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B080);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B090);

extern s32 D_00134688;

/**
 * Reset the global counter/flag D_00134688 to 0.
 */
void func_0011B0A0(void) {
    D_00134688 = 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", WaitVblankStartIntc);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B140);

extern s32 func_0011BE20(void);
extern s32 func_0011BCD0(const char *buf, s32 len);

/**
 * TTY write hook (called twice each by func_00118BC0 and func_00119AC0): for
 * fd 1 or 2, open the DECI2 TTY channel on first use (func_0011BE20; the flag
 * D_00134688 latches a successful open) and send len bytes of buf through
 * func_0011BCD0.
 *
 * @return func_0011BCD0's result, or -1 for any other fd or a failed open
 *
 * func_0011BE20 takes no argument. The earlier best spelling (NOTE #9721, 9 of
 * 31 words) passed it one, which made cc1 load $a0 in that jal's delay slot,
 * where the ROM's is a nop, and kept the extra value live in a saved register.
 * The ARITY fix alone closes it.
 */
s32 func_0011B1E8(s32 fd, const char *buf, s32 len) {
    if (fd == 1 || fd == 2) {
        if (D_00134688 == 0) {
            if (func_0011BE20() == 0) {
                return -1;
            }
            D_00134688 = 1;
        }
        return func_0011BCD0(buf, len);
    }
    return -1;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B268);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B270);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B320);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B328);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B3D0);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B450);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B458);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B500);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B580);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B588);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B5F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", DisableDmac);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", EnableDmac);

extern s32  D_0013C600;  /* semaphore id created for the worker subsystem */
extern char D_0013A920[]; /* error format string for an unknown command op */
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
 * func_0011B728 = the unit's background worker-thread main loop (entered from
 * func_0011AA20/StartThread, see func_0011B800). It blocks forever on the
 * subsystem semaphore D_0013C600 (func_0011AC60/WaitSema); each time it is
 * signalled it pops the next command record from the 512-entry ring buffer at
 * *queue (the `head` cursor masked to 0x1FF, then the 2-byte {op,arg} record at
 * cmds[idx*2]) and dispatches on the op byte: 0 -> func_0011AB50/WakeupThread,
 * 1 -> func_0011AAD0/ReleaseWaitThread, 2 -> func_0011AB90/SuspendThread (each
 * on the record's thread-id arg byte); any other op prints the error string
 * D_0013A920 via func_0011C7E8. Never returns.
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
        func_0011AC60(D_0013C600);
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
            func_0011C7E8(D_0013A920);
            break;
        }
    }
}

extern s32 D_00134690;   /* worker-thread id / init guard (<=0 until created) */
extern s32 D_0013C608[2];/* StartThread argument block (two words, zeroed) */
extern u8  D_0013C200[]; /* the worker thread's stack buffer */
extern s32 D_001AEFF0;   /* the gp base value handed to the worker thread */

/**
 * Bring up the unit's background worker thread once. Guards on D_00134690 (>0
 * means already up): creates a semaphore (func_0011AC20 = CreateSema) with
 * maxCount 0xFF, then a thread (func_0011AA20 = CreateThread) entered at
 * func_0011B728 with a 0x400-byte stack and the engine gp. On success it caches
 * the thread id in D_00134690, starts it (func_0011AA40 = StartThread) with a
 * zeroed two-word argument block, and yields the ready queue
 * (func_0011AAB0 = RotateThreadReadyQueue) for the current thread
 * (func_0011AB10 = GetThreadId). On any failure it returns -1, tearing the
 * semaphore back down (func_0011AC30 = DeleteSema) if the thread could not be
 * created. Returns the worker thread id (D_00134690) on success.
 */
s32 func_0011B800(void) {
    struct ThreadParam thread;
    struct SemaParam sema;
    s32 thid;

    if (D_00134690 > 0) {
        return -1;
    }
    sema.maxCount = 0xFF;
    sema.initCount = 0;
    D_0013C600 = func_0011AC20((s32 *)&sema);
    if (D_0013C600 < 0) {
        return -1;
    }
    thread.entry = (void *)func_0011B728;
    thread.stack = D_0013C200;
    thread.stackSize = 0x400;
    thread.gpReg = &D_001AEFF0;
    thread.initPriority = 0;
    thid = func_0011AA20(&thread);
    D_00134690 = thid;
    if (thid < 0) {
        func_0011AC30(D_0013C600);
        return -1;
    }
    D_0013C608[0] = 0;
    D_0013C608[1] = 0;
    func_0011AA40(thid, D_0013C608);
    func_0011AAB0(func_0011AB10(), 1);
    return D_00134690;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B8D8);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B970);

extern s32 func_0011B050(s32 count, s32 *value);

extern u8 D_0013CA10[];

/**
 * Pack a four-word command record (low 16 bits of `a` via the u16 param's
 * callee-side andi, `b`, `c`, and the uncached-mirror address of D_0013CA10
 * ORed with 0x20000000) on the stack and push it through func_0011B050 with
 * count 1, RETURNING its status ($2 passthrough). The old park blamed an
 * "ordering ee-gcc won't reproduce" (`sd $31` / `move a1,sp` after the record
 * stores) — the u16-param + local-array + VALUE-RETURN phrasing reproduces
 * exactly that schedule (the void statement-call form schedules the andi and
 * stores differently; the live return value is part of the shape).
 *
 * MATCHED: byte-exact at -O2 -G0 (raw-byte + symbol-size verified).
 */
s32 func_0011B978(u16 a, s32 b, s32 c) {
    s32 msg[4];

    msg[0] = a;
    msg[1] = b;
    msg[2] = c;
    msg[3] = (s32)((u32)D_0013CA10 | 0x20000000);
    return func_0011B050(1, msg);
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B9C0);

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

/**
 * Pack a three-word record (a, b, low 16 bits of c via the u16 param's
 * callee-side andi) on the stack and push it through func_0011B050 with
 * count -5, returning its status. The old park blamed the arg1-via-temp
 * store order on the allocator — with the VALUE-RETURN phrasing (see
 * func_0011B978) cc1 emits exactly the original's early `b`-copy schedule.
 * Sibling func_0011BA58 differs only by count.
 *
 * MATCHED: byte-exact at -O2 -G0 (raw-byte + symbol-size verified).
 */
s32 func_0011BA20(s32 a, s32 b, u16 c) {
    s32 msg[3];

    msg[0] = a;
    msg[1] = b;
    msg[2] = c;
    return func_0011B050(-5, msg);
}

/** Sibling of func_0011BA20 with count -6.
 *  MATCHED: byte-exact at -O2 -G0 (raw-byte + symbol-size verified). */
s32 func_0011BA58(s32 a, s32 b, u16 c) {
    s32 msg[3];

    msg[0] = a;
    msg[1] = b;
    msg[2] = c;
    return func_0011B050(-6, msg);
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BA90);

/**
 * Push the single 32-bit value arg0 through func_0011B050 with count 0x10
 * (the value is passed by address in a local).
 */
void func_0011BAA0(s32 arg0) {
    s32 value = arg0;
    func_0011B050(0x10, &value);
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BAC4);

/* Global list head initialised by func_0011BAC8: value word, entry count,
 * head/tail links and the inline first slot they initially point at. */
typedef struct ListHead0013CA40 {
    s32 value;        /* 0x0 */
    s32 count;        /* 0x4 */
    void *head;       /* 0x8 */
    void *tail;       /* 0xC */
    s32 firstSlot[4]; /* 0x10 */
} ListHead0013CA40;
extern ListHead0013CA40 D_0013CA40;

/**
 * Initialise the global list head D_0013CA40: store `value`, clear the entry
 * count, and point both head and tail links at the inline first slot (+0x10);
 * returns &D_0013CA40. (The volatile stores pin the original head/count/tail
 * store order, which the scheduler would otherwise batch — this is the
 * volatile-pinning technique that cracked the old ~98.5% wall.)
 */
s32 *func_0011BAC8(s32 value) {
    s32 *base = (s32 *)&D_0013CA40;
    D_0013CA40.value = value;
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

// recovered splat-dropped code (epilogue-stump mis-split): raw words, byte-exact
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BB30);

/* The DECI2 TTY channel (protocol 0x210) the SDK's kernel printf writes
 * through: func_0011BE20 opens it, func_0011BB38 is its event handler and
 * func_0011BCD0 sends a buffer and waits. The four counters are volatile
 * because the handler updates them from DECI2 event context while
 * func_0011BCD0 spins on `busy`. */
typedef struct Deci2Header {
    u16 len;         /* whole packet length, header included */
    u16 reserved;
    u16 protocol;
    s8 source;       /* 'E' (EE) */
    s8 destination;  /* 'H' (host) */
} Deci2Header;

typedef struct TtyRing {
    s32 capacity;
    s32 count;
    u8 *head;
    u8 *tail;        /* write cursor, advanced by func_0011BAF0 */
} TtyRing;

typedef struct TtyChannel {
    volatile s32 socket;   /* DECI2 socket, negative if the open failed */
    volatile s32 sendLen;  /* bytes of the send packet still to go */
    volatile s32 recvLen;  /* bytes of the receive packet read so far */
    volatile s32 busy;     /* nonzero while a send is in flight */
    u8 *sendBuf;           /* uncached view of D_0013CB80 */
    u8 *recvBuf;           /* uncached view of D_0013CCC0 */
    TtyRing *input;        /* received characters, from func_0011BAC8 */
} TtyChannel;

extern TtyChannel D_0013CB50;
extern u8 D_0013CB80[];
extern u8 D_0013CCC0[];
extern char D_0013A948[]; /* "TTY: packet size larger than expect\n" */
extern char D_0013A970[]; /* "TTY: receive error" */
extern char D_0013A988[]; /* "TTY: send err %d\n" */
extern char D_0013A9A0[]; /* "TTY: err ti->wlen=%08x\n" */

/**
 * DECI2 event handler of the TTY channel (registered by func_0011BE20).
 *
 * @param event  DECI2 event: 1/2 read, 3 write, 4 write done
 * @param param  for a read, the byte count to fetch (0: the packet is
 *               complete); unused otherwise
 * @param tty    the channel (D_0013CB50, passed back as the open's opt)
 *
 * A read appends `param` bytes to the receive packet (func_0011BA20); a
 * read with 0 copies the packet's payload (after its 12-byte header) into
 * the input ring. A write pushes the rest of the send packet
 * (func_0011BA58); write-done clears `busy`. Errors are reported through
 * the kernel printf and otherwise ignored.
 *
 * `param` doubles as the read count and the payload index, and the write
 * count is its own variable: that is what gives the ROM's allocation
 * (`tty` in $17, the write count in $a1, where the error printf takes it).
 */
void func_0011BB38(s32 event, s32 param, TtyChannel *tty) {
    Deci2Header *packet;

    switch (event) {
    case 1:
    case 2:
        if (param != 0) {
            if ((u32)(tty->recvLen + param) > 0x140) {
                func_0011C7E8(D_0013A948);
            }
            param = func_0011BA20(tty->socket, (s32)(tty->recvBuf + tty->recvLen), param);
            if (param < 0) {
                func_0011C7E8(D_0013A970);
            }
            tty->recvLen += param;
        } else {
            packet = (Deci2Header *)tty->recvBuf;
            for (param = 12; param < packet->len; param++) {
                *tty->input->tail = tty->recvBuf[param];
                func_0011BAF0((s32 *)tty->input);
            }
            tty->recvLen = 0;
        }
        break;
    case 3: {
        s32 sent;

        sent = func_0011BA58(tty->socket, (s32)tty->sendBuf, tty->sendLen);
        if (sent < 0) {
            func_0011C7E8(D_0013A988, sent);
            tty->busy = 0;
        } else {
            tty->sendBuf += sent;
            tty->sendLen -= sent;
        }
        break;
    }
    case 4:
        if (tty->sendLen != 0) {
            func_0011C7E8(D_0013A9A0, tty->sendLen);
        }
        tty->busy = 0;
        break;
    }
}

/* func_0011BCD0 (send a buffer over the channel, '\n' -> "\r\n", then wait
 * for write-done) stays asm: cc1 reproduces it except the final wait loop,
 * where its volatile `socket` load sits before `jal func_0011B9F8`. The SN
 * assembler left that delay slot empty; GNU as 2.40 moves the load into it,
 * and asm_unit.sh's volatile-marker pin that undoes this runs only at -G8
 * (this unit is -G0). 21 of 84 words, all in that loop and the epilogue it
 * shifts. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BCD0);

extern void func_0011AEA0(s32 mode);

/**
 * Open the TTY channel: flush the data cache (func_0011AEA0 = FlushCache),
 * open DECI2 protocol 0x210 with func_0011BB38 as the handler, and if that
 * succeeds, clear the counters, point the send/receive buffers at the
 * uncached mirrors of D_0013CB80/D_0013CCC0, pre-fill the send packet's
 * DECI2 header (protocol 0x210, 'E' -> 'H') and create the 0x100-byte input
 * ring.
 *
 * @return 1 if the channel is open, 0 if the DECI2 open failed
 */
s32 func_0011BE20(void) {
    u8 *packet;

    func_0011AEA0(0);
    D_0013CB50.socket = func_0011B978(0x210, (s32)&D_0013CB50, (s32)func_0011BB38);
    if (D_0013CB50.socket < 0) {
        return 0;
    }
    D_0013CB50.busy = 0;
    D_0013CB50.sendLen = 0;
    D_0013CB50.recvLen = 0;
    D_0013CB50.recvBuf = (u8 *)((u32)D_0013CCC0 | 0x20000000);
    D_0013CB50.sendBuf = packet = (u8 *)((u32)D_0013CB80 | 0x20000000);
    ((Deci2Header *)packet)->protocol = 0x210;
    ((Deci2Header *)packet)->source = 'E';
    ((Deci2Header *)packet)->destination = 'H';
    ((Deci2Header *)packet)->reserved = 0;
    *(s32 *)(packet + 8) = 0;
    D_0013CB50.input = (TtyRing *)func_0011BAC8(0x100);
    return 1;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BEDC);

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

extern s32 D_00134694;
extern u8 D_0013CE00[];

/* func_0011BF18(c): append byte c to the debug line buffer D_0013CE00 (write
 * position D_00134694). If the buffer is near full (pos >= 0x7E) reset and flush
 * it first via func_0011BAA0. A newline (0xA) terminates the line (NUL after it)
 * and flushes via a void tail call to func_0011BAA0 (sibling-call-optimised to
 * `j func_0011BAA0` since this fn is void; cf. lever #7); any other byte is
 * appended and the position advanced. */
void func_0011BF18(s32 c) {
    s32 cnt = D_00134694;
    if (cnt >= 0x7E) {
        D_00134694 = 0;
        D_0013CE00[0x7F] = 0;
        func_0011BAA0((s32)D_0013CE00);
        cnt = D_00134694;
    }
    if (c == 0xA) {
        D_00134694 = 0;
        D_0013CE00[cnt] = (u8)c;
        D_0013CE00[cnt + 1] = 0;
        func_0011BAA0((s32)D_0013CE00);
    } else {
        D_00134694 = cnt + 1;
        D_0013CE00[cnt] = (u8)c;
    }
}

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
 * func_0011C000 = convert the IEEE-754 double whose raw bits are `bits` into an
 * integer. The sign bit is ignored. The 11-bit exponent field is rebiased to
 * exp = field - 0x433, the power of two that scales the 53-bit significand
 * (implicit leading 1 restored). exp < -0x35, i.e. |x| < 0.5, gives 0; exp >= 13,
 * i.e. |x| >= 2^65, saturates to 9999 (0x270F). That is the only clamp: in
 * between, the result is NOT bounded to [0, 9999] (100000.0 gives 100000, and
 * above 2^31 the low word can read negative). Otherwise the significand is
 * shifted left by exp (exp >= 0), or right by (-exp - 2) and then by 2 more,
 * rounding up only when those last two dropped bits are both set (a fraction of
 * .75 or more; 2.5 gives 2). Returns the low 32 bits.
 *
 * Non-obvious: the ROM updates the exponent and significand IN PLACE
 * (`exp -= 0x433`, `exp = -exp`, `x = (x << 12) >> 12`). Folding them into one
 * expression per value (the earlier C, the #else arm at edb1aa7b: 81.11% solo
 * on the sdk29 arm by the unit objdiff report, objdiff_build.sh, measured at
 * 6235db84 and 1a06cc63, FACT #7958/#8021, and again on master fe247108 plus
 * this unit by task #753) makes cc1 thread them through $2 and extra temporaries
 * instead of keeping exp in $6 and the significand in $5. The rounding test must also be written with
 * `== 3` as the then-branch, which gives the ROM's `bnel` with the plain
 * shift in the likely slot.
 */
s32 func_0011C000(s64 bits) {
    u64 x = bits;
    s64 exp;

    exp = (x << 1) >> 53;
    exp -= 0x433;
    if (exp < -0x35) {
        return 0;
    }
    if (exp >= 0xD) {
        return 0x270F;
    }
    x = (x << 12) >> 12;
    x |= 0x10000000000000ULL;
    if (exp < 0) {
        exp = -exp;
        x >>= exp - 2;
        if ((x & 3) == 3) {
            x = (x >> 2) + 1;
        } else {
            x >>= 2;
        }
    } else {
        x <<= exp;
    }
    return x;
}

/**
 * func_0011C090 = print the double whose bits are `value` in scientific notation
 * via the formatter func_0011C7E8. Emits a leading '-' (through the char hook
 * D_00134698) for negatives and works on the magnitude, normalising it into
 * [0.1, 1.0) while tracking a decimal exponent: when >= 0.1 (D_0013A9D8) it
 * divides by 10 (func_00122DA8) until < 1.0, counting the exponent up; when
 * < 0.1 it multiplies by 10 (func_00122B00) until >= 0.1 (D_0013A9E0), counting
 * down. The normalised mantissa is scaled by 1e6 (D_0013A9E8) and truncated to an
 * integer in [100000, 999999] (func_001212C8), and that INTEGER is passed to
 * func_0011C000, which reads its argument as the raw bits of a double. Its
 * exponent field is 0 for any integer below 2^52, so func_0011C000 always returns
 * 0 here (no clamping takes place) and the mantissa always prints as "0.0" through
 * "0.%d" (D_0013A9C0) — the digits are lost in the ROM itself. Finally appends the
 * exponent as "e+NN" (D_0013A9C8) or "e%d"/"e-NN" (D_0013A9D0).
 *
 * MATCHING WALL (FP-constant-pool / li.d toolchain ceiling). A byte-exact rebuild
 * reaches only ~50% because the original loads its three non-trivial double
 * constants - 0.1 (D_0013A9D8 and a second copy D_0013A9E0) and 1e6 (D_0013A9E8)
 * - from the rodata constant pool via the SN assembler's `li.d`/`ld $5,
 * %lo(D_..)($at)` macro (recomputed inline, $at, NOT hoisted). From C, ee-gcc
 * -O2 -G0 EITHER (a) emits the same `li.d $5, 0.1` pseudo for FP literals, which
 * our GNU `mips-linux-gnu-as -march=r5900` REJECTS ("opcode not supported"; the
 * original SN asm expanded it into the D_0013A9xx pool entries), OR (b) when the
 * constants are referenced as named externs, splits the address and HOISTS the
 * %hi out of the scale loops into extra callee-saved registers (5 saved vs the
 * original 3), diverging structurally. Neither is steerable from C with this
 * assembler, so the MATCHING arm stays INCLUDE_ASM (byte-exact). The li.d
 * rejection is purely a matching-toolchain problem, NOT a behaviour problem: the
 * faithful body below references the pool doubles by their recovered bit patterns
 * (0.1 = 0x3FB999999999999A, 1e6 = 0x412E848000000000; the cheap 1.0/10.0 the
 * original builds inline with ori+dsll32) and is cmp-oracle'd asm-vs-C
 * bit-identical (every formatted byte) on the real R5900 by
 * tools/ee/eetest/cmp/isolated/run_cmp_015180_iso.sh. Note a magnitude of exactly
 * 0.0 would spin the scale-up loop forever (0*10 stays < 0.1), so the caller never
 * passes 0; the oracle excludes it for the same reason. */
/* DLI lever MEASURED (task #1220; unit objdiff report, objdiff_build.sh, this #else body promoted
 * SOLO, sdk29 arm, colima-ee-x86; every other row in the unit unchanged). cc1 synthesises
 * 0x3fb999999999999a and 0x412e848000000000 with `dli`. The ROM holds neither assembler's
 * expansion: it loads both from .rodata and synthesises only 0x4024.../0x3ff0... inline. The pool:
 * 0x3fb999999999999a sits in TWO distinct slots, D_0013A9D8 and D_0013A9E0, and 0x412e848000000000
 * in D_0013A9E8, each loaded by its own `ld %lo(...)` (FACT #8772, ROM words), so the two-slot
 * layout is itself a constraint. No allowlist row applies. Solo score 54.76%. Residual class:
 * LITERAL-POOL constants, not dli. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C090);
#else
extern s32 func_00123028(s64 a, s64 b);   /* soft-float ordered compare (a<=>b) */
extern s64 func_00122A98(s64 a, s64 b);   /* soft-float subtract a - b          */
extern s64 func_00122B00(s64 a, s64 b);   /* soft-float multiply a * b          */
extern s64 func_00122DA8(s64 a, s64 b);   /* soft-float divide a / b            */
extern s64 func_001212C8(s64 x);          /* truncate non-negative double -> u64 */
extern s32 func_0011C000(s64 bits);       /* double bits -> int; 0 for the int passed here */
extern s32 func_0011C7E8(void *fmt, ...);  /* the printf-style formatter sink     */
extern void (*D_00134698)(s32 ch);        /* single-character output hook        */
extern char D_0013A9C0[];                  /* "0.%d" */
extern char D_0013A9C8[];                  /* "e+%d" */
extern char D_0013A9D0[];                  /* "e%d"  */

/* recovered bit patterns of the rodata/inline double constants */
#define DBL_0_1 0x3FB999999999999ALL      /* 0.1  (D_0013A9D8 / D_0013A9E0) */
#define DBL_1E6 0x412E848000000000LL      /* 1e6  (D_0013A9E8)              */
#define DBL_10  0x4024000000000000LL      /* 10.0 (built ori 0x8048,dsll32) */
#define DBL_1   0x3FF0000000000000LL      /* 1.0  (built ori 0xFFC0,dsll32) */

s32 func_0011C090(s64 value) {
    s32 exp = 0;
    s64 scaled;
    s32 digits;

    if (func_00123028(value, 0) < 0) {           /* negative: emit '-', use |value| */
        value = func_00122A98(0, value);
        D_00134698('-');
    }
    if (func_00123028(value, DBL_0_1) < 0) {      /* |value| < 0.1: scale up by 10 */
        do {
            value = func_00122B00(value, DBL_10);
            exp--;
        } while (func_00123028(value, DBL_0_1) < 0);
    } else {                                      /* |value| >= 1.0: scale down by 10 */
        while (func_00123028(value, DBL_1) >= 0) {
            value = func_00122DA8(value, DBL_10);
            exp++;
        }
    }
    /* mantissa now in [0.1, 1.0): scale to six digits and truncate; the integer
     * is then read as double bits by func_0011C000, so digits is always 0 */
    scaled = func_00122B00(value, DBL_1E6);
    digits = func_0011C000(func_001212C8(scaled));
    func_0011C7E8(D_0013A9C0, digits);            /* "0.%d", always "0.0" */
    if (exp < 0) {
        return func_0011C7E8(D_0013A9D0, exp);    /* "e-NN" (sign carried by %d) */
    }
    return func_0011C7E8(D_0013A9C8, exp);        /* "e+NN" */
}
#endif

/**
 * func_0011C1F8 = the Kprintf/vfprintf CORE - a hand-rolled formatted-output
 * engine. Signature `s32 func_0011C1F8(char *fmt, s64 *ap)`: walks `fmt`, emitting
 * every literal byte through the global char hook (*D_00134698)(int), and on '%'
 * parses an optional zero-pad field width ("%0NN", 1-2 digits, clamped to 31) and
 * an optional length modifier ('l' -> 64-bit, 'h' -> 16-bit, else 32-bit), then
 * dispatches a conversion via the jump table jtbl_0013A9F0 (indexed by spec-'0'):
 *   o  octal      (digits built with n&7, n>>=3)
 *   x  hex        (n&0xF, n>>=4; 'a'-'f' lowercase, +0x57)
 *   d  signed dec (emits '-', uses __moddi3 %10 / __divdi3 /10)
 *   u  unsigned   (__umoddi3 %10 / __udivdi3 /10)
 *   e,f float     (lwc1 arg; ==0 -> '0', else func_001234F0 then func_0011C090)
 *   s  string     ("(null)" when the target is empty, else byte-copy)
 *   c  char        (sign-extended single byte)
 * Integers are formatted right-to-left into a 32-byte stack buffer ending at a NUL
 * at sp+0x1F; the field width pre-fills '0's from sp+(0x1F-w) and the digit start
 * pointer is clamped down to it. Wraps the whole run between func_0011F5E0 (disable
 * interrupts) and func_0011F628 (restore); the return value is the interrupt-state
 * cookie (0 / func_0011F628()). Variadic args are pulled from `ap` in 8-byte slots.
 *
 * MATCHING WALL (cc1 register-allocation + char-load-scheduling ceiling). A clean,
 * behaviourally-faithful reconstruction of the full control flow (the goto-state-
 * machine, the jtbl switch in memory order, the per-branch `ap++; *(T*)(ap-1)`
 * arg fetch, the s0=cursor / s1=value / s2=p / s3=ap / s4=pad / s5=hook / s6=cookie
 * register model) rebuilds to ~85.7% byte-identical (380/380 instrs, same frame,
 * same prologue/saves, same jump table). The residual is NOT behaviour - it is two
 * unsteerable ee-gcc 2.9 choices that diverge per call site:
 *   (1) the char output loops: the original re-loads each emitted byte as a fresh
 *       `lbu` (separate from the `lb` zero-test), whereas gcc CSEs our `*q` to one
 *       load + `move` (this also cascades the hook-pointer to v1 vs v0);
 *   (2) the length-modifier temp lands in a2 vs the original's a3, and %c's byte
 *       in a temp vs s1.
 * Both are scheduler/allocator tie-breaks invariant under every source phrasing
 * tried (do-while vs while, (s8)(u8) casts, per-case vs shared value locals,
 * declaration order). The MATCHING arm stays INCLUDE_ASM (byte-exact). The
 * recovered faithful C ships as the portable TARGET_NATIVE #else: it is a correct
 * printf, cmp-oracle'd asm-vs-C bit-identical (the D_00134698 hook-captured byte
 * stream over every conversion specifier, width and length modifier) on the real
 * R5900 by run_cmp_015180_iso.sh. The %e/%f path composes onto func_0011C090's
 * own #else (itself cmp-oracle'd).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C1F8);
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
                do { D_00134698(*q); q++; } while (*q != 0);
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
                do { D_00134698(*q); q++; } while (*q != 0);
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
                        D_00134698('-');
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
                do { D_00134698(*q); q++; } while (*q != 0);
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
                do { D_00134698(*q); q++; } while (*q != 0);
                s = p;
                goto Lcont;
            case 'e' - '0':
            case 'f' - '0': {
                float f;
                ap++;
                f = *(float *)(ap - 1);
                if (f == 0.0f) {
                    D_00134698('0');
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
                    D_00134698('(');
                    p++;
                    D_00134698('n');
                    D_00134698('u');
                    D_00134698('l');
                    D_00134698('l');
                    D_00134698(')');
                    s = p;
                    goto Lcont;
                }
                q = str;
                p++;
                do {
                    D_00134698(*q);
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
                D_00134698((s32)cn);
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
        D_00134698(c);
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

extern void (*D_00134698)(s32 ch);  /* single-character output hook */

/**
 * Kprintf: the kernel-console printf. Temporarily points the character hook
 * D_00134698 at func_0011BF18 (the debug line buffer), runs the core formatter
 * func_0011C1F8 over the variadic arguments, then restores the previous hook.
 *
 * @param fmt  printf-style format string
 * @param ...  its arguments, spilled to the stack home area as for
 *             func_0011C7E8 (EABI single-float va_start)
 * @return     the formatter's result
 */
s32 Kprintf(const char *fmt, ...) {
    void (*saved)(s32) = D_00134698;
    char *ap;
    s32 ret;

    D_00134698 = func_0011BF18;
    ap = (char *)__builtin_next_arg(fmt)
         - (__builtin_args_info(2) >= 8 ? 0 : (8 - __builtin_args_info(2)) * 8);
    ret = func_0011C1F8((char *)fmt, (s64 *)ap);
    D_00134698 = saved;
    return ret;
}

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

extern s32 D_0013D080[];

/**
 * Lookup into the global table D_0013D080: return D_0013D080[arg0]
 * (no bounds checking).
 */
s32 func_0011C8B0(s32 arg0) {
    return D_0013D080[arg0];
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C8C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C8D8);

extern s32 DisableDmac(s32 channel);
extern s32 func_0011A950(s32 channel, s32 handlerId);
extern s32 D_0013CF54;
extern s32 D_0013469C;

/**
 * Shut down the SIF command layer: disable DMA channel 5 (SIF0), remove the
 * channel-5 handler whose id is held in D_0013CF54 (func_0011A950 is the
 * RemoveDmacHandler syscall stub, syscall 0x13), and clear the "initialised"
 * flag D_0013469C. Identity by body: libkernel's sceSifExitCmd.
 *
 * Both callees are declared VALUE-RETURNING, as they are (eekernel.h has
 * `int DisableDmac(int)`): each call's result occupies $2, so the %hi
 * temporary after it lands in $3 as in the ROM. Declaring either one void
 * frees $2 for the following lui/lw (or lui/sw) pair — 2 words each, measured.
 */
void func_0011CB58(void) {
    DisableDmac(5);
    func_0011A950(5, D_0013CF54);
    D_0013469C = 0;
}

extern char *D_0013CF64; /* 8-byte-stride (key,value) table for negative indices */
extern char *D_0013CF6C; /* 8-byte-stride (key,value) table for indices >= 0 */

/**
 * Store a (key,value) pair into the sign-selected table pair: entry `index`
 * of D_0013CF6C for index >= 0, of D_0013CF64 for index < 0 (the negative
 * index reaches backwards from that table's base); key at slot+0x0, value at
 * slot+0x4. (Reusing `index` for the loaded table pointer keeps it in $a0
 * like the original, the pre-computed `addr` rides the bgez delay slot, and
 * the volatile stores pin the original value-then-key order.)
 */
void func_0011CB90(s32 index, s32 key, s32 value) {
    s32 addr = index * 8;
    if (index < 0) {
        index = (s32)D_0013CF64;
    } else {
        index = (s32)D_0013CF6C;
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
        index = (s32)D_0013CF64;
    } else {
        index = (s32)D_0013CF6C;
    }
    addr += index;
    *(s32 *)addr = 0;
}

/* SIF command packet header (the SDK's sceSifCmdHdr) and DMA descriptor
 * (sceSifDmaData). */
typedef struct SifCmdHeader {
    u32 packetSize : 8;
    u32 dataSize : 24;
    s32 dataDest;   /* IOP address the extra data goes to, 0 if none */
    s32 command;    /* command id, e.g. 0x80000009 = RPC bind */
    u32 option;
} SifCmdHeader;

typedef struct SifDmaDesc {
    s32 src;
    s32 dest;
    s32 size;
    s32 attr;
} SifDmaDesc;

extern s32 D_0013CF60; /* IOP-side command buffer address */
extern void sceSifWriteBackDCache(void *ptr, s32 size);
extern s32 func_0011AFE0(SifDmaDesc *desc, s32 count); /* sceSifSetDma */
extern s32 func_0011AFF0(SifDmaDesc *desc, s32 count); /* isceSifSetDma */

/**
 * Send a SIF command to the IOP (the SDK's _sceSifSendCmd): fill in the
 * packet's header and DMA it, after an optional extra data block, to the
 * IOP command buffer D_0013CF60.
 *
 * @param command     command id, written to the header
 * @param mode        bit 0: called from an interrupt handler (use the
 *                    i-prefixed SetDma); bit 2: write the data block back
 *                    from the data cache first
 * @param header      the packet, which starts with its SifCmdHeader
 * @param packetSize  packet length, 16..112 bytes
 * @param src, dest, size  optional data block (EE address, IOP address,
 *                    length); none if size <= 0
 * @return the DMA id from SetDma, 0 if packetSize is out of range or the
 *         DMA could not be queued
 *
 * The pointer-typed parameters are load-bearing: declared all-s32 (as the
 * old extern had them) cc1 schedules the stores differently, 20 words off.
 * So is the data block's statement order, and the 0x44 attr must be stored
 * after packetSize; the textbook order (header first, then the descriptor)
 * moves the stores and swaps two registers.
 */
s32 func_0011CBE8(s32 command, s32 mode, SifCmdHeader *header, s32 packetSize,
                  void *src, s32 dest, s32 size) {
    SifDmaDesc dma[2];
    s32 count;

    if ((u32)(packetSize - 16) > 96) {
        return 0;
    }
    count = 0;
    if (size > 0) {
        header->dataSize = size;
        dma[0].src = (s32)src;
        dma[0].dest = dest;
        header->dataDest = dest;
        dma[0].size = size;
        dma[0].attr = 0;
        count = 1;
        if (mode & 4) {
            sceSifWriteBackDCache(src, size);
        }
    } else {
        header->dataSize = 0;
        header->dataDest = 0;
    }
    dma[count].src = (s32)header;
    dma[count].dest = D_0013CF60;
    dma[count].size = packetSize;
    header->command = command;
    header->packetSize = packetSize;
    dma[count].attr = 0x44;
    count++;
    sceSifWriteBackDCache(header, packetSize);
    if (mode & 1) {
        return func_0011AFF0(dma, count);
    }
    return func_0011AFE0(dma, count);
}

/**
 * Thin wrapper around func_0011CBE8 that forces its second argument (the mode
 * flag) to 0 and shifts the caller's arg1..arg5 into arg2..arg6.
 */
s32 func_0011CD20(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    return func_0011CBE8(arg0, 0, (SifCmdHeader *)arg1, arg2, (void *)arg3, arg4,
                         arg5);
}

/**
 * Thin wrapper around func_0011CBE8 that forces its second argument (the mode
 * flag) to 1 and shifts the caller's arg1..arg5 into arg2..arg6.
 */
s32 func_0011CD60(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    return func_0011CBE8(arg0, 1, (SifCmdHeader *)arg1, arg2, (void *)arg3, arg4,
                         arg5);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CDA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceSifWriteBackDCache);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CF74);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceSifInitRpc);

extern void func_0011CB58(void);
extern s32 D_001346A0;

/**
 * Reset helper: run the subsystem reset routine func_0011CB58(), then clear the
 * global state word D_001346A0 to 0.
 */
void func_0011D118(void) {
    func_0011CB58();
    D_001346A0 = 0;
}

/* A 0x40-byte SIF RPC packet as the packet allocator sees it. */
typedef struct RpcPacket {
    s32 header[4];  /* SifCmdHeader */
    s32 recId;      /* bit 0 set while allocated; slot index in the high half */
    s32 pktAddr;    /* the packet's own address */
    s32 rpcId;
    s32 rest[9];
} RpcPacket;

/* The SDK's RPC packet table: an id counter and a table of 0x40-byte
 * packets. */
typedef struct RpcPacketTable {
    s32 lastId;
    RpcPacket *packets;
    s32 count;
} RpcPacketTable;

/**
 * Allocate an RPC packet (the SDK's _rpc_get_packet): with interrupts off
 * (func_0011F5E0 / func_0011F628), take the first packet whose allocated
 * bit is clear, and stamp it with (slot << 16) | 5, its own address and a
 * fresh id from the table's counter. When the incremented counter reads 1
 * that id is used and the counter is bumped once more, so 2 is never handed
 * out on that pass.
 *
 * @param table  the packet table (D_0013E900)
 * @return the packet, or 0 if all are in use
 *
 * Reading the id back through the else arm is what makes cc1 rematerialise
 * the constant 1 on the wrap path, as the ROM does.
 */
RpcPacket *func_0011D140(RpcPacketTable *table) {
    RpcPacket *packet;
    s32 slot, count, id;

    func_0011F5E0();
    count = table->count;
    packet = table->packets;
    for (slot = 0; slot < count; slot++, packet++) {
        if (!(packet->recId & 1)) {
            packet->recId = (slot << 16) | 5;
            if (++table->lastId == 1) {
                id = table->lastId++;
            } else {
                id = table->lastId;
            }
            packet->pktAddr = (s32)packet;
            packet->rpcId = id;
            func_0011F628();
            return packet;
        }
    }
    func_0011F628();
    return 0;
}

/**
 * Reset object arg0: clear its field_0x18 (arg0[6]) and clear bit 0 of the flag
 * word field_0x10 (arg0[4]) — i.e. mark it inactive/idle.
 */
void func_0011D1E8(s32 *arg0) {
    arg0[6] = 0;
    arg0[4] &= 0xFFFFFFFE;
}

/* A circular pool of 0x40-byte slots (only the fields func_0011D208 reads). */
typedef struct RingPool {
    char _p0[0x14];
    u8 *slots;      /* 0x14: base of the slot array */
    s32 count;      /* 0x18: number of slots */
    char _p1c[0x24 - 0x1C];
    s32 next;       /* 0x24: index of the next slot to hand out */
} RingPool;

/**
 * Hand out the next slot of a circular pool, round-robin.
 *
 * @param pool  the pool; pool->next is advanced past the returned slot
 * @return      &slots[next % count] (0x40-byte slots)
 *
 * The cursor is stored back as (next % count) + 1, i.e. already wrapped, so it
 * never grows past count. A zero count traps (the compiler's divide-by-zero
 * `break 0,7`). Forming the slot address BEFORE the cursor store is what gives
 * the ROM's order (mfhi into $2, store, then the addu in the jr delay slot);
 * returning the sum after the store swaps $2/$3 and sinks the sw into the slot.
 */
s32 *func_0011D208(RingPool *pool) {
    s32 index = pool->next % pool->count;
    s32 *slot = (s32 *)(pool->slots + (index << 6));

    pool->next = index + 1;
    return slot;
}

struct D238Node {
    s32 data;             /* 0x0:  resource handle freed via func_0011D1E8, then cleared */
    char _p4[0x8 - 0x4];
    s32 status;           /* 0x8:  fed to func_0011AC50 when >= 0 (role inferred) */
    char _pc[0x14 - 0xc];
    s32 stateA, stateB;   /* 0x14, 0x18: state written by the 0x80000009 command */
    void (*handler)(s32); /* 0x1C: callback invoked by the 0x8000000A command */
    s32 handlerArg;       /* 0x20: argument passed to handler */
    s32 stateC;           /* 0x24: state written by the 0x80000009 command */
};
struct D238Obj {
    char _p0[0x1C];
    struct D238Node *node;                /* 0x1C: node this request operates on */
    u32 command;                          /* 0x20 */
    s32 newStateC, newStateA, newStateB;  /* 0x24, 0x28, 0x2C: new state for the copy command */
};
extern void func_0011AC50(s32 arg);

/* func_0011D238(arg0): dispatch on the command word arg0->f20 over the node
 * arg0->f1C. 0x8000000A invokes the node's callback (n->f1C)(n->f20) if set;
 * 0x80000009 copies three fields from arg0 into the node; any other value just
 * uses the node as-is. Then, for every command, if n->f8 >= 0 run
 * func_0011AC50(n->f8), free n->f0 via func_0011D1E8, and clear n->f0. The
 * `goto`s are load-bearing: they make the 0x8000000A case and the callback-set
 * case the branch TARGETS, which is what reproduces the original's dispatch
 * branch-likely forms (beql/bnel with annulled field loads) and keeps the
 * callback pointer in $2 — the plain if/else-chain mis-orders the branches and
 * mis-colours that register. */
void func_0011D238(struct D238Obj *arg0) {
    struct D238Node *n;
    u32 cmd = arg0->command;
    if (cmd == 0x8000000A) {
        goto caseInvoke;
    }
    if (cmd > 0x8000000A) {
        n = arg0->node;
        goto common;
    }
    if (cmd == 0x80000009) {
        goto caseCopy;
    }
    n = arg0->node;
    goto common;
caseInvoke:
    n = arg0->node;
    if (n->handler == 0) {
        goto common;
    }
    n->handler(n->handlerArg);
    n = arg0->node;
    goto common;
caseCopy:
    n = arg0->node;
    n->stateC = arg0->newStateC;
    n->stateA = arg0->newStateA;
    n->stateB = arg0->newStateB;
common:
    if (n->status >= 0) {
        func_0011AC50(n->status);
    }
    func_0011D1E8((s32 *)n->data);
    n->data = 0;
}

/**
 * Enqueue a command against the next slot of `pool`: allocate the slot via
 * func_0011D208, copy the descriptor fields obj[5]/obj[7] into it (explicit
 * temps — the original loads both fields up-front before any argument
 * builds), stamp the command word 0x8000000C at slot[8], then void-tail-call
 * func_0011CD60(0x80000008, slot, 0x40, obj[8], obj[9], obj[10]) — six plain
 * EABI register args, sibcall-optimised to the original's `j` with the
 * sp-restore in the delay slot (lever 7).
 */
void func_0011D2F0(s32 *obj, RingPool *pool) {
    s32 *slot = func_0011D208(pool);
    s32 a = obj[5];
    s32 b = obj[7];

    slot[5] = a;
    slot[7] = b;
    slot[8] = 0x8000000C;
    func_0011CD60(0x80000008, (s32)slot, 0x40, obj[8], obj[9], obj[10]);
}

/* Chained-bucket lookup. Walk the bucket list anchored at table->0x28; within
 * each bucket walk the node chain at bucket->0x8 (nodes linked by ->0x38) and
 * return the first node whose key word (->0x0) equals `key`. If a bucket's chain
 * is exhausted, advance to the next bucket via ->0x14. Returns 0 if nothing
 * matches. Frameless leaf; the entry guard and both inner walks are emitted as
 * branch-likely (beql/bnel) loads, which ee-gcc reproduces from this plain
 * while/while phrasing. */
struct D350Node {
    u32 key;                       /* 0x0  */
    u8  _pad4[0x38 - 0x4];
    struct D350Node *next;         /* 0x38: next node in this bucket's chain */
};

struct D350Bucket {
    u8  _pad0[0x8];
    struct D350Node *chain;        /* 0x8:  head of the node chain */
    u8  _padc[0x14 - 0xc];
    struct D350Bucket *nextBucket; /* 0x14 */
};

struct D350Table {
    u8  _pad0[0x28];
    struct D350Bucket *buckets;    /* 0x28: head of the bucket list */
};

struct D350Node *func_0011D350(u32 key, struct D350Table *table) {
    struct D350Bucket *b = table->buckets;
    while (b != 0) {
        struct D350Node *n = b->chain;
        while (n != 0) {
            if (n->key == key) {
                return n;
            }
            n = n->next;
        }
        b = b->nextBucket;
    }
    return 0;
}

/* An RPC bind request packet (the SDK's SifRpcBindPkt). */
typedef struct SifRpcBindPacket {
    s32 header[4];
    s32 recId;
    s32 pktAddr;
    s32 rpcId;
    s32 client;
    s32 server;     /* the server id asked for */
} SifRpcBindPacket;

/* The reply to a bind request (the SDK's SifRpcRendPkt), sent as command
 * 0x80000008. */
typedef struct SifRpcBindReply {
    s32 header[4];
    s32 recId;
    s32 pktAddr;    /* 0x14: the requester's packet, echoed back */
    s32 rpcId;
    s32 client;     /* 0x1C: the requester's client, echoed back */
    u32 cid;        /* 0x20: the command being answered (0x80000009) */
    s32 server;     /* 0x24: the bound server, 0 if none is registered */
    s32 buff;       /* 0x28: that server's receive buffer */
    s32 cbuff;      /* 0x2C: that server's callback buffer */
} SifRpcBindReply;

/* The fields of a registered RPC server (the SDK's sceSifServeData) that a
 * bind reply reports. */
typedef struct SifRpcServer {
    s32 rpcNumber;  /* 0x00 */
    s32 func;       /* 0x04 */
    s32 buff;       /* 0x08 */
    s32 size;       /* 0x0C */
    s32 cfunc;      /* 0x10 */
    s32 cbuff;      /* 0x14 */
} SifRpcServer;

/**
 * Answer an IOP request to bind to an EE RPC server (SIF command 0x80000009;
 * sceSifInitRpc registers this as its handler, with the RPC state D_0013E900
 * as `pool`). Takes a reply slot from the pool (func_0011D208), echoes the
 * request's packet and client back, looks the asked-for server up
 * (func_0011D350) and reports it with its two buffers - all three 0 when no
 * such server is registered - then sends the reply as command 0x80000008
 * through func_0011CD60, tail-called.
 *
 * Splat labelled this `rename`; it is not libc rename (see symbol_addrs).
 * Loading both echoed fields into locals before either store gives the ROM's
 * load/load/store/store order; copying them field by field interleaves them.
 */
void HandleRpcBindRequest(SifRpcBindPacket *req, RingPool *pool) {
    SifRpcBindReply *reply = (SifRpcBindReply *)func_0011D208(pool);
    SifRpcServer *server;
    s32 pktAddr = req->pktAddr;
    s32 client = req->client;

    reply->client = client;
    reply->pktAddr = pktAddr;
    reply->cid = 0x80000009;
    server = (SifRpcServer *)func_0011D350(req->server, (struct D350Table *)pool);
    if (server == 0) {
        reply->server = 0;
        reply->buff = 0;
        reply->cbuff = 0;
    } else {
        reply->server = (s32)server;
        reply->buff = server->buff;
        reply->cbuff = server->cbuff;
    }
    func_0011CD60(0x80000008, (s32)reply, 0x40, 0, 0, 0);
}

/* An SIF RPC client (the SDK's sceSifClientData). */
typedef struct SifRpcClient {
    s32 pktAddr;    /* the packet of the request in flight */
    s32 rpcId;
    s32 semaId;     /* semaphore a waiting call blocks on, -1 for none */
    s32 mode;
    s32 command;
    s32 buff;
    s32 cbuff;
    s32 endFunc;
    s32 endParam;
    void *serve;    /* set by the IOP's reply once the bind reached a server */
} SifRpcClient;

extern RpcPacketTable D_0013E900;

/**
 * sceSifBindRpc: bind `client` to the IOP RPC server `rpcNumber`. Takes an
 * RPC packet (func_0011D140), records it and its id in the client, fills in
 * the bind request and sends it (command 0x80000009). Unless `mode` bit 0
 * (no-wait) is set, blocks on a fresh semaphore until the reply has been
 * handled, then deletes the semaphore.
 *
 * @return 0 if sent (and, when waiting, answered); -1 if no packet was
 *         free, -2 if the send failed, -3 if the semaphore could not be
 *         created
 *
 * Reading the packet's id into the client before storing the packet
 * pointer gives the ROM's schedule (the load issues first, ahead of the
 * annulled branch slot that otherwise inverts the null test).
 */
s32 sceSifBindRpc(SifRpcClient *client, u32 rpcNumber, u32 mode) {
    struct SemaParam sema;
    SifRpcBindPacket *bind;

    client->command = 0;
    client->serve = 0;
    bind = (SifRpcBindPacket *)func_0011D140(&D_0013E900);
    if (bind == 0) {
        return -1;
    }
    client->rpcId = bind->rpcId;
    client->pktAddr = (s32)bind;
    bind->server = rpcNumber;
    bind->pktAddr = (s32)bind;
    bind->client = (s32)client;
    if (!(mode & 1)) {
        sema.maxCount = 1;
        sema.initCount = 0;
        client->semaId = func_0011AC20((s32 *)&sema);
        if (client->semaId < 0) {
            func_0011D1E8((s32 *)bind);
            return -3;
        }
        if (func_0011CD20(0x80000009, (s32)bind, 0x40, 0, 0, 0) == 0) {
            func_0011D1E8((s32 *)bind);
            func_0011AC30(client->semaId);
            return -2;
        }
        func_0011AC60(client->semaId);
        func_0011AC30(client->semaId);
        return 0;
    }
    client->semaId = -1;
    if (func_0011CD20(0x80000009, (s32)bind, 0x40, 0, 0, 0) == 0) {
        func_0011D1E8((s32 *)bind);
        return -2;
    }
    return 0;
}

/* SIF RPC server-side records, as func_0011D590 sees them (the layouts of the
 * SDK's sceSifQueueData, sceSifServeData and the call message). Every field
 * is a plain s32, pointers included: with pointer-typed fields cc1 reorders the
 * copy's stores (every one of the 35 ROM words differed in a solo compile),
 * with uniform s32 it keeps them. */
typedef struct RpcQueue {
    s32 key;       /* server thread id, negative while there is none */
    s32 active;    /* nonzero while the server thread is running a request */
    s32 link;
    s32 start;     /* RpcServe *: first pending request */
    s32 end;       /* RpcServe *: last pending request */
} RpcQueue;

typedef struct RpcServe {
    s32 command, func, buff, size, cfunc, cbuff, csize, client;
    s32 paddr, fno, receive, rsize, rmode, rid, link;
    s32 next;      /* RpcServe *: next pending request in the queue */
    RpcQueue *base;
} RpcServe;

typedef struct RpcCallMsg {
    s32 header[4];
    s32 recId, paddr, unused18, client, rpcNumber, sendSize, receive, recvSize, rmode;
    RpcServe *serve;
} RpcCallMsg;

extern s32 func_0011B8D8(s32 tid);

/**
 * SIF RPC "call" command handler (the SDK's _request_call): append the target
 * serve record to its queue's pending list, copy the call parameters from the
 * message into it, and wake the server thread (func_0011B8D8) when the queue
 * has one and it is idle. The wake is a `j` tail call.
 *
 * @param msg  the incoming call message; msg->serve names the serve record
 *
 * Reading paddr and client into locals before their two stores reproduces the
 * ROM's lw/lw/sw/sw opening of the copy; written as two plain assignments,
 * cc1 interleaves them. (A comment here used to call this ~86% and walled on a
 * branch-likely; cc1 emits that bnel itself.)
 */
void func_0011D590(RpcCallMsg *msg) {
    RpcServe *sd = msg->serve;
    RpcQueue *q = sd->base;
    s32 paddr, client;

    if (q->start == 0) {
        q->start = (s32)sd;
    } else {
        ((RpcServe *)q->end)->next = (s32)sd;
    }
    q->end = (s32)sd;

    paddr = msg->paddr;
    client = msg->client;
    sd->paddr = paddr;
    sd->client = client;
    sd->fno = msg->rpcNumber;
    sd->size = msg->sendSize;
    sd->receive = msg->receive;
    sd->rsize = msg->recvSize;
    sd->rmode = msg->rmode;
    sd->rid = msg->recId;

    if (q->key >= 0 && q->active == 0) {
        func_0011B8D8(q->key);
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D620);

/**
 * Validity predicate for the RPC handle in arg0: returns 1 iff arg0[0] points
 * to a live object, arg0[1] matches obj[6] (the +0x18 id/gen), and obj[4]
 * (+0x10) bit 0 is set; else 0. The explicit gotos keep the original's
 * two-exit shape (separate return-0 / return-1 `jr` blocks, lever 10) —
 * returning `obj[4] & 1` directly is what collapsed the tail into `andi v0`
 * and flipped the id-check to a beql in the earlier ~65% attempt.
 */
s32 sceSifCheckStatRpc(s32 *arg0) {
    s32 *obj = (s32 *)*arg0;
    if (obj == 0) goto ret0;
    if (arg0[1] != obj[6]) goto ret0;
    if ((obj[4] & 1) != 0) goto ret1;
ret0:
    return 0;
ret1:
    return 1;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D850);

extern s32 func_0011AC20(s32 *desc);
extern s32 D_00134738;
extern s32 D_0013473C;

/**
 * Lazily create the two paired handles D_00134738 / D_0013473C (sentinel -1 =
 * uninitialised on the first): build a small descriptor on the stack (fields 1
 * and 2 set, field 5 cleared), then create both handles from it via
 * func_0011AC20. A no-op once D_00134738 exists.
 */
void func_0011D868(void) {
    if (D_00134738 == -1) {
        s32 desc[8];
        desc[5] = 0;
        desc[2] = 1;
        desc[1] = 1;
        D_00134738 = func_0011AC20(desc);
        D_0013473C = func_0011AC20(desc);
    }
}

extern s32 func_0011AC60(s32 handle);
extern s32 func_0011AC40(s32 sema);
extern u8 D_0013FE80[];

/* func_0011D8C8: allocate a slot from the 0x200-byte pool D_0013FE80 (32 slots
 * of 0x10). Under lock (func_0011D868 + func_0011AC60/func_0011AC40 on the lock
 * handle D_00134738), scan for the first free slot (word at +0x4 is zero), claim
 * it by storing 0x10000000 there, and return it (or 0 if the pool is full). The
 * claim store is emitted BEFORE the unlock call so the 0x10000000 constant stays
 * in a caller-saved temp (it isn't live across the call) — matching the original
 * frame; storing after the unlock would force it into a callee-saved register. */
void *func_0011D8C8(void) {
    u8 *e;
    func_0011D868();
    func_0011AC60(D_00134738);
    e = D_0013FE80;
    while (e < D_0013FE80 + 0x200) {
        if (*(s32 *)(e + 4) == 0) {
            *(s32 *)(e + 4) = 0x10000000;
            func_0011AC40(D_00134738);
            return e;
        }
        e += 0x10;
    }
    func_0011AC40(D_00134738);
    return 0;
}

extern s32 func_0011AC40(s32 sema);
extern u8 D_0013FE80[];

/**
 * Look up slot `idx` in the fixed 0x20-entry table D_0013FE80 (0x10-byte
 * stride). After the lazy-init (func_0011D868) and acquiring the table lock
 * (func_0011AC60(D_00134738)), release the lock (func_0011AC40) and return the
 * slot address when idx (unsigned) is in range, else 0. The explicit `goto`
 * makes the in-range block the branch TARGET (lever 10), which is what lets
 * the allocator keep the sltiu range-check in $3 and the table base in $2 —
 * the if/else phrasing of the same body walled at 99.6% on exactly that
 * one-register choice.
 */
void *func_0011D950(u32 idx) {
    void *slot;
    func_0011D868();
    func_0011AC60(D_00134738);
    if (idx < 0x20) goto hit;
    func_0011AC40(D_00134738);
    return 0;
hit:
    slot = &D_0013FE80[idx * 0x10];
    func_0011AC40(D_00134738);
    return slot;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D9C0);

extern s32 func_0011AC60(s32 handle);
extern s32 D_00134734;

/**
 * Lazily create the singleton handle D_00134734 (sentinel -1 = uninitialised):
 * build a small descriptor on the stack (fields 1 and 2 set, field 5 cleared),
 * hand it to func_0011AC20 and cache the resulting handle. A no-op once created.
 */
void func_0011DD48(void) {
    if (D_00134734 == -1) {
        s32 desc[8];
        desc[5] = 0;
        desc[2] = 1;
        desc[1] = 1;
        D_00134734 = func_0011AC20(desc);
    }
}

/**
 * Take the file-I/O lock: create the lock semaphore D_00134734 on first use
 * (func_0011DD48), then wait on it (func_0011AC60, WaitSema). Always returns 0.
 * func_0011DDC8 releases it.
 *
 * @param mode  the caller's command number (0 open, 1 close, 2 read); every
 *              ROM caller passes it, the body never reads it
 */
s32 func_0011DD98(s32 mode) {
    func_0011DD48();
    func_0011AC60(D_00134734);
    return 0;
}

extern s32 func_0011AC40(s32 sema);

/**
 * Release the singleton table lock: forward the global semaphore handle
 * D_00134734 to func_0011AC40 (SignalSema). A tail call whose result is
 * discarded, so ee-gcc sibling-call-optimises it into the original's frameless
 * `j func_0011AC40` (the handle load rides the jump's delay slot). That holds
 * with func_0011AC40 declared value-returning (`int SignalSema(int)`, as the
 * SDK has it): only `return func_0011AC40(...)` keeps $ra and becomes
 * `jal` + a frame (task #1848, measured both ways).
 */
void func_0011DDC8(void) {
    func_0011AC40(D_00134734);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011DDD8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011DE08);

extern s32 D_0013472C;
extern u8 D_001400A8[4];
extern u8 D_00134684[4];  /* "2550": this library's 4-byte release tag (no NUL) */
extern u8 *D_00134740;    /* -> "....": the wildcard tag */

/**
 * Report whether the 4-byte tag at D_001400A8 is incompatible with this
 * library: it differs from the release tag "2550" (D_00134684) AND from the
 * tag D_00134740 points at ("...."), and those two differ from each other.
 * func_0011E0A0 below clears the tag. memcmp's result is used only for != 0.
 *
 * @return 1 if incompatible, 0 if the tag matches either one
 */
s32 func_0011E010(void) {
    u8 *release = D_00134684;
    u8 *tag = D_001400A8;
    s32 mismatch = 0;

    if (memcmp(tag, release, 4) && memcmp(tag, D_00134740, 4)
        && memcmp(release, D_00134740, 4)) {
        mismatch = 1;
    }
    return mismatch;
}

/**
 * Reset the subsystem state guarded by D_0013472C: clear the flag word to 0 and
 * zero the 4-byte descriptor at D_001400A8. Always returns 0.
 */
s32 func_0011E0A0(void) {
    D_0013472C = 0;
    memset(D_001400A8, 0, 4);
    return 0;
}

extern s32 func_0011D620(void *a0, s32 a1, s32 a2, void *a3, s32 a4,
                         void *a5, s32 a6, s32 a7, s32 a8);
extern void sceSifWriteBackDCache(void *ptr, s32 size);

/* The file-I/O RPC client and its two transfer buffers. */
extern u8 D_00140080[];
extern u8 D_0013F5C0[];

/* The request block sent to the IOP file server: a completion semaphore, where
 * the 4-byte result is to be written back, then the per-command arguments.
 * Every command ends with the caller's file-table index. */
typedef struct FioRequest {
    s32 sema;
    s32 *result;
    s32 resultSize;
    union {
        struct {
            s32 flags;
            s32 mode;
            char path[0x400];
            s32 index;
        } open;
        struct {
            s32 handle;
            s32 index;
        } close;
        struct {
            s32 handle;
            void *buf;
            s32 size;
            s32 _pad;
            s32 index;
        } read;
    } u;
} FioRequest;
extern FioRequest D_0013E980;

/* A file-table slot (D_0013FE80, 0x10 bytes each): the server-side handle and
 * the open mode (0 = free). */
typedef struct FioSlot {
    s32 handle;
    s32 mode;
} FioSlot;

extern s32 func_0011DE08(void);
extern s32 D_00134738;

/**
 * Open `path` with `flags` (and, as the variadic argument, the creation mode):
 * initialise the library on first use, refuse when the IOP file server's
 * version tag is incompatible, claim a file-table slot, then send command 0
 * (open) carrying the flags (bits 0x10000000/0x80000000 masked off), the mode,
 * the path (truncated to 0x3FF characters) and the slot's index. On success the
 * slot records the server handle and the flags.
 *
 * @return the new descriptor (the file-table index), the server's negative
 *         status, -0x10004 for an incompatible server, -19 when the table is
 *         full, -11 when the RPC fails
 *
 * `ret` carries both the server's accepted word and the returned descriptor:
 * the ROM keeps them in one register ($17), and two variables put the
 * descriptor in its own callee-saved register instead.
 */
s32 func_0011E0D8(const char *path, s32 flags, ...) {
    FioRequest *req = &D_0013E980;
    struct SemaParam sp;
    FioSlot *slot;
    char *ap;
    s32 result;
    s32 index;
    s32 mode;
    s32 sema;
    s32 ret;
    s32 i;

    func_0011DD98(0);
    if (D_0013472C == 0) {
        func_0011DE08();
    }
    if (func_0011E010() != 0) {
        func_0011DDC8();
        return -0x10004;
    }
    slot = func_0011D8C8();
    if (slot == 0) {
        func_0011DDC8();
        return -19;
    }
    ap = (char *)__builtin_next_arg(flags)
         - (__builtin_args_info(2) >= 8 ? 0 : (8 - __builtin_args_info(2)) * 8);
    mode = *(s32 *)ap;
    for (i = 0; i < 0x400; i++) {
        if ((req->u.open.path[i] = path[i]) == 0) {
            break;
        }
    }
    if (i == 0x400) {
        req->u.open.path[0x3FF] = 0;
    }
    index = ((u8 *)slot - D_0013FE80) >> 4;
    req->u.open.flags = flags & 0x6FFFFFFF;
    req->u.open.mode = mode;
    req->u.open.index = index;
    sp.maxCount = 1;
    sp.initCount = 0;
    sp.option = 0;
    sema = func_0011AC20((s32 *)&sp);
    req->result = &result;
    req->sema = sema;
    req->resultSize = 4;
    if (func_0011D620(D_00140080, 0, 0, &D_0013E980, 0x418, D_0013F5C0, 4, 0, 0) < 0) {
        func_0011AC30(sema);
        func_0011DDC8();
        return -11;
    }
    ret = *(s32 *)((u32)D_0013F5C0 | 0x20000000);
    func_0011DDC8();
    if (ret == 0) {
        func_0011AC30(sema);
        return -11;
    }
    func_0011AC60(sema);
    func_0011AC30(sema);
    if (result < 0) {
        func_0011AC60(D_00134738);
        slot->mode = 0;
        func_0011AC40(D_00134738);
        return result;
    }
    ret = index;
    func_0011AC60(D_00134738);
    slot->handle = result;
    slot->mode |= flags;
    func_0011AC40(D_00134738);
    return ret;
}

/**
 * Close file `fd`: under the file-I/O lock, send command 1 (close) with the
 * slot's server handle and table index, free the slot, and wait for the IOP's
 * completion on a fresh semaphore.
 *
 * @return 0 (or the server's negative status), -1 when the library is not
 *         initialised, -9 for a bad descriptor, -11 when the RPC fails
 */
s32 func_0011E360(s32 fd) {
    FioSlot *slot = func_0011D950(fd);
    FioRequest *req = &D_0013E980;
    struct SemaParam sp;
    s32 result;
    s32 sema;
    s32 ok;

    func_0011DD98(1);
    if (D_0013472C == 0) {
        func_0011DDC8();
        return -1;
    }
    if (slot == 0 || slot->mode == 0) {
        func_0011DDC8();
        return -9;
    }
    req->u.close.handle = slot->handle;
    req->u.close.index = ((u8 *)slot - D_0013FE80) >> 4;
    sp.maxCount = 1;
    sp.initCount = 0;
    sp.option = 0;
    sema = func_0011AC20((s32 *)&sp);
    D_0013E980.sema = sema;
    req->result = &result;
    req->resultSize = 4;
    if (func_0011D620(D_00140080, 1, 0, req, 0x14, D_0013F5C0, 4, 0, 0) < 0) {
        func_0011AC30(sema);
        func_0011DDC8();
        return -11;
    }
    slot->mode = 0;
    ok = *(s32 *)((u32)D_0013F5C0 | 0x20000000);
    func_0011DDC8();
    if (ok == 0) {
        func_0011AC30(sema);
        return -11;
    }
    func_0011AC60(sema);
    func_0011AC30(sema);
    if (result < 0) {
        return result;
    }
    return 0;
}

/* The no-wait semaphore table: one entry per outstanding no-wait read, holding
 * the completion semaphore the IOP will signal; -1 marks a free entry. */
extern s32 D_001346A8[32];

/**
 * Read up to `size` bytes from file `fd` into `buf`: send command 2 (read) with
 * the slot's server handle, the buffer, the size and the slot's table index,
 * and wait for the IOP's completion on a fresh semaphore. A slot opened with
 * mode bit 0x8000 (no-wait) instead files that semaphore in the first free
 * entry of D_001346A8 under the file-I/O lock, negates the request's semaphore
 * to tell the server so, and returns 0 once the RPC is accepted. The caller's
 * buffer is written back from the D-cache first unless mode bit 0x20000000 is
 * set; the request block always is.
 *
 * @return the byte count (or the server's negative status), 0 for an accepted
 *         no-wait read, -1 when the library is not initialised, -9 for a bad
 *         descriptor, -11 when the RPC fails
 *
 * Each spelling below is priced by undoing it alone (solo unit compile scored
 * against the ROM words with relocated fields masked, differing words after
 * alignment; task #1872):
 *  - the second no-wait test is written `(s16)mode & 0x8000`. Written like the
 *    first, gcse PRE merges the two tests into one `andi` held in a
 *    callee-saved register across six calls, where the ROM recomputes
 *    `andi $2,$19,0x8000` in the `bnez` delay slot; the cast makes it a
 *    different expression to gcse and combine still folds it to the same
 *    `andi`. Spelled plainly: 26 words (149 vs 152).
 *  - `req->result` is stored before `req->resultSize`; the other order: 4.
 *  - the RPC is passed `&D_0013E980` rather than `req`: 21.
 *
 * SCHEDULING DEVICE (RULING #8483: empty template, emits nothing): the fence
 * after `i = 0` names the table's first word as a memory operand, so the
 * table's `%hi` and `i = 0` are both issued before it, and it stops reorg's
 * backward delay-slot search at the pre-loop `bne`. That slot is then filled
 * from both successor threads with the one `lui %hi(D_0013F5C0)` that gcse
 * hoisted onto them, as in the ROM (0x11E5E8). Without the fence `i = 0` takes
 * the slot and that `lui` stays duplicated at the head of both threads: 5
 * words, 153 vs 152. Untied (`__asm__ __volatile__("")`): 5, the table's
 * `%hi` then issues after it. `i = 0` in the for-init (after the fence): 5.
 * The non-volatile form also reads 0.
 */
s32 func_0011E4E0(s32 fd, void *buf, s32 size) {
    FioSlot *slot = func_0011D950(fd);
    FioRequest *req = &D_0013E980;
    struct SemaParam sp;
    s32 result;
    s32 mode;
    s32 sema;
    s32 ok;
    s32 i;

    func_0011DD98(2);
    if (D_0013472C == 0) {
        func_0011DDC8();
        return -1;
    }
    if (slot == 0 || (mode = slot->mode) == 0) {
        func_0011DDC8();
        return -9;
    }
    req->u.read.handle = slot->handle;
    sp.maxCount = 1;
    req->u.read.index = ((u8 *)slot - D_0013FE80) >> 4;
    req->u.read.buf = buf;
    req->u.read.size = size;
    sp.initCount = 0;
    sp.option = 0;
    sema = func_0011AC20((s32 *)&sp);
    req->result = &result;
    req->resultSize = 4;
    D_0013E980.sema = sema;
    if (mode & 0x8000) {
        func_0011AC60(D_0013473C);
        i = 0;
        __asm__ __volatile__("" : : "m"(D_001346A8[0]));
        for (; i < 32; i++) {
            if (D_001346A8[i] == -1) {
                D_001346A8[i] = req->sema;
                req->sema = -req->sema;
                break;
            }
        }
        func_0011AC40(D_0013473C);
    }
    if (!(mode & 0x20000000)) {
        sceSifWriteBackDCache(buf, size);
    }
    sceSifWriteBackDCache(req, 0x20);
    if (func_0011D620(D_00140080, 2, 0, &D_0013E980, 0x20, D_0013F5C0, 4, 0, 0) < 0) {
        func_0011AC30(sema);
        func_0011DDC8();
        return -11;
    }
    ok = *(s32 *)((u32)D_0013F5C0 | 0x20000000);
    func_0011DDC8();
    if (ok == 0) {
        func_0011AC30(sema);
        return -11;
    }
    if ((s16)mode & 0x8000) {
        func_0011AC30(sema);
        return 0;
    }
    func_0011AC60(sema);
    func_0011AC30(sema);
    return result;
}

/* func_0011E740: 0x60 bytes of inter-function padding (`addiu sp,+0xN; nop`
 * filler words) split off by symbol_addrs size:0x60; the real function begins at
 * sceSifInitIopHeap. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E740);

extern s32 func_0011D620(void *a0, s32 a1, s32 a2, void *a3, s32 a4,
                         void *a5, s32 a6, s32 a7, s32 a8);
extern s32 D_00134744;

/* The IOP-heap RPC client: only `serve` (+0x24), set once the bind has
 * reached the server, is read here. */
typedef SifRpcClient IopHeapClient;
extern IopHeapClient D_00140140;

extern s32 sceSifBindRpc(IopHeapClient *client, u32 rpcNumber, u32 mode);

/**
 * sceSifInitIopHeap (recovered from the splat mis-split above): bind the
 * IOP-heap RPC client D_00140140 to server 0x80000003, retrying after a
 * ~1M-iteration busy wait until the server answers, then mark the service
 * active (D_00134744 = 0; it is negative while unbound).
 *
 * @return 0 once bound; -1 if sceSifBindRpc fails
 */
s32 sceSifInitIopHeap(void) {
    s32 spin;

    while (1) {
        if (sceSifBindRpc(&D_00140140, 0x80000003, 0) < 0) {
            return -1;
        }
        if (D_00140140.serve != 0) {
            /* clearing it inside the exit branch, not after the loop, is
             * what puts %hi(D_00134744) in $2 ahead of the return value */
            D_00134744 = 0;
            break;
        }
        for (spin = 0x100000; spin != -1; spin--) {
        }
    }
    return 0;
}
extern s32 D_001401C0;
extern s32 D_00140180;

/**
 * Register a request with the D_00140180 service: bail out returning 0 if the
 * service slot D_00134744 is inactive (negative); otherwise stash the request
 * parameters (arg1, arg0, arg2) into the D_001401C0 descriptor and submit it via
 * func_0011D620. Returns the resulting handle D_00140180 on success, 0 on
 * failure.
 */
s32 func_0011E828(s32 arg0, s32 arg1, s32 arg2) {
    if (D_00134744 < 0) {
        return 0;
    }
    (&D_001401C0)[0] = arg1;
    (&D_001401C0)[1] = arg0;
    (&D_001401C0)[2] = arg2;
    if (func_0011D620(&D_00140140, 4, 0, &D_001401C0, 0xC,
                      &D_00140180, 4, 0, 0) >= 0) {
        return D_00140180;
    }
    return 0;
}

/**
 * Free a block of IOP memory obtained from func_0011E828 (sceSifAllocSysMemory):
 * the same D_00140140 RPC client, function 2 instead of 4, sending one word
 * (the block address) and receiving the server's one-word status.
 *
 * @param addr  the IOP address to free
 * @return      0 if the client was never bound (D_00134744 < 0); the server's
 *              reply D_00140180 on a successful call; -1 if the call failed
 */
s32 sceSifFreeSysMemory(void *addr) {
    if (D_00134744 < 0) {
        return 0;
    }
    (&D_001401C0)[0] = (s32)addr;
    if (func_0011D620(&D_00140140, 2, 0, &D_001401C0, 4,
                      &D_00140180, 4, 0, 0) >= 0) {
        return D_00140180;
    }
    return -1;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E920);

extern s32 D_00134748;
extern u8 D_00140528[4];

/* A 4-byte release tag, copied as a unit (it is not word aligned in the
 * copy, hence lwl/lwr + swl/swr). */
typedef struct ReleaseTag {
    u8 bytes[4];
} ReleaseTag;

/* The module loader's RPC buffer, sent and received in place. The reply to
 * function 0xFF is the 4-byte release tag at +0; a module call sends the id
 * at +0 and the argument block at +0x104 and gets status/result back at
 * +0 / +4. */
typedef struct LoadFileBuffer {
    s32 word0;
    s32 argLen;
    u8 path[0xFC];
    u8 args[0xFC];
} LoadFileBuffer;

extern IopHeapClient D_00140500; /* client bound to server 0x80000006 */
extern LoadFileBuffer D_00140300;

/**
 * Bind the RPC client D_00140500 to IOP server 0x80000006 (the module
 * loader) unless already done (D_00134748 >= 0), retrying after a
 * ~1M-iteration busy wait until the server answers; then mark it bound
 * (D_00134748 = 0), call its function 0xFF to fetch the IOP's 4-byte
 * release tag into the RPC buffer D_00140300 and keep a copy in D_00140528.
 *
 * @return 0 on success or if already bound; -1 if sceSifBindRpc fails;
 *         0xFFFEFFFF if the tag query fails
 *
 * The bound/already-bound arm wraps the whole body (rather than an early
 * `return 0`) because the ROM's `return 0` for it is the function's last
 * block.
 */
s32 func_0011E938(void) {
    s32 spin;

    if (D_00134748 < 0) {
        while (1) {
            if (sceSifBindRpc(&D_00140500, 0x80000006, 0) < 0) {
                return -1;
            }
            if (D_00140500.serve != 0) {
                D_00134748 = 0;
                if (func_0011D620(&D_00140500, 0xFF, 0, 0, 0, &D_00140300, 4,
                                  0, 0) < 0) {
                    return -0x10001;
                }
                *(ReleaseTag *)D_00140528 = *(ReleaseTag *)&D_00140300;
                return 0;
            }
            for (spin = 0x100000; spin != -1; spin--) {
            }
        }
    }
    return 0;
}
extern u8 *D_0013474C;    /* -> "....": the wildcard tag */

/**
 * The same incompatibility test as func_0011E010 for the tag at D_00140528,
 * against "2550" (D_00134684) and the tag D_0013474C points at ("....").
 * func_0011EAC8 below clears the tag.
 *
 * @return 1 if incompatible, 0 if the tag matches either one
 */
s32 func_0011EA38(void) {
    u8 *release = D_00134684;
    u8 *tag = D_00140528;
    s32 mismatch = 0;

    if (memcmp(tag, release, 4) && memcmp(tag, D_0013474C, 4)
        && memcmp(release, D_0013474C, 4)) {
        mismatch = 1;
    }
    return mismatch;
}

/**
 * Reset the subsystem state guarded by D_00134748: set the flag word to -1
 * (uninitialised sentinel) and zero the 4-byte descriptor at D_00140528.
 * Always returns 0.
 */
s32 func_0011EAC8(void) {
    D_00134748 = -1;
    memset(D_00140528, 0, 4);
    return 0;
}

/* A module argument block, copied as a unit when it fills the buffer
 * (cc1's inline block move, with its run-time alignment test). */
typedef struct ModuleArgs {
    u8 bytes[0xFC];
} ModuleArgs;

/**
 * Call module-loader function 6 (load or start a module, with arguments)
 * over the D_00140500 RPC client: make sure the client is bound
 * (func_0011E938) and the IOP release is compatible (func_0011EA38), put
 * `id` and up to 0xFC bytes of `args` into the RPC buffer D_00140300, and
 * run the call in place.
 *
 * @param id       module id / request word, sent at +0
 * @param argLen   argument bytes; clamped to 0xFC
 * @param args     argument block, or 0 for none
 * @param result   receives the reply's second word
 * @return the reply's first word; -0x10000 if the bind failed, -0x10004 on
 *         a release mismatch, -0x10001 if the RPC failed
 *
 * The reply's status is read into a local before `*result` is stored (the
 * store may alias the buffer, and the ROM loads the status first).
 */
s32 func_0011EB00(s32 id, s32 argLen, const void *args, s32 *result) {
    if (func_0011E938() < 0) {
        return -0x10000;
    }
    if (func_0011EA38() != 0) {
        return -0x10004;
    }
    D_00140300.word0 = id;
    if (args != 0) {
        if (argLen > 0xFC) {
            *(ModuleArgs *)D_00140300.args = *(const ModuleArgs *)args;
            D_00140300.argLen = 0xFC;
        } else {
            memcpy(D_00140300.args, args, argLen);
            D_00140300.argLen = argLen;
        }
    } else {
        D_00140300.argLen = 0;
    }
    if (func_0011D620(&D_00140500, 6, 0, &D_00140300, 0x200, &D_00140300, 8,
                      0, 0) < 0) {
        return -0x10001;
    }
    {
        s32 status = D_00140300.word0;

        *result = D_00140300.argLen;
        return status;
    }
}

/**
 * Forward (arg0, arg1, arg2) to func_0011EB00, supplying a 16-byte scratch
 * buffer on the stack as its fourth (output) argument.
 */
void func_0011ED08(s32 arg0, s32 arg1, s32 arg2) {
    u8 buf[16];
    func_0011EB00(arg0, arg1, (const void *)arg2, (s32 *)buf);
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ED28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceSifResetIop);

extern s32 func_0011B030(s32 arg0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceSifRebootIop);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011EFE8);

/**
 * func_0011EFF0 = EE kernel syscall 0x5A. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. func_0011F058
 * calls it with (dst, src, nbytes) right after its SetSyscall has made 0x5A the
 * word-copy routine func_0011F000. The call therefore copies a kernel patch,
 * and is not a DMA/INTC setup (see SyscallPatchEntry). Same primitive as
 * func_0011F878. Exact SDK name UNCONFIRMED.
 */
s32 func_0011EFF0(s32 a, s32 b, s32 c) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x5A\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
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
 * $v0. Takes one argument in $a0, a syscall number from D_00134AD0. By the time
 * func_0011F058 calls it, 0x5B has been replaced by the kernel patch at
 * 0x80075000, and the result is installed as that number's handler. Same
 * primitive as func_0011F8C0. Exact SDK name UNCONFIRMED.
 */
s32 func_0011F038(s32 a) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x5B\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

/**
 * func_0011F048 = EE kernel syscall 0x74. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Called by
 * func_0011F058.
 * SetSyscall: install `handler` as the kernel's entry for syscall number
 * `syscall`. Identity from the ROM, not only from SDK numbering: each caller
 * walks a {syscall number, handler} table (D_00134AD0 / D_00135568 /
 * D_00135CF0, first rows {0x5A, a local word-copy routine} and
 * {0x5B, 0x8007x000}), and the very next syscall 0x5A call copies a kernel
 * patch to 0x8007x000 with exactly the (dst, src, nbytes) arguments the
 * routine just installed as 0x5A takes (func_0011F000 for D_00134AD0). The
 * SCE kernel numbering calls 0x74 RFU116 (ps2sdk: SetSyscall, external SDK
 * knowledge). The vendored eekernel.h has no prototype for it; void matches
 * ps2sdk's and no caller reads $v0 (task #1848). One of four file-local
 * copies (0x11F048, 0x11F818, 0x11F868, 0x11FA50; the libkernl table entry
 * is 0x11AFA0), so the splat name is kept rather than a duplicate name.
 */
void func_0011F048(s32 syscall, s32 handler) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

/* One row of a kernel syscall-patch table: a syscall number and the handler
 * address to install for it with SetSyscall (0x74). Used by D_00134AD0 (here),
 * D_00135568 (func_0011F938) and D_00135CF0 (func_0011FAB8). ROM rows:
 *   D_00134AD0 = {0x5A,0x11F000} {0x5B,0x80075000} {0x54,0x11F4C0} {0x55..0x59,0}
 *   D_00135568 = {0x5A,0x11F888} {0x5B,0x80074000} {0xFFFFC402,0}
 *   D_00135CF0 = {0x5A,0x11FA70} {0x5B,0x80076000} {0xFC,0} {0xFE,0} {0xFD,0}
 *                {0xFF,0} {0x12C,0} {0x8,0}
 * Row 0's handler is always one of this unit's (dst, src, nbytes) word-copy
 * routines, and the caller's next call is syscall 0x5A with exactly those
 * arguments, copying a kernel patch to row 1's 0x8007x000. Row 1 then installs
 * that patch as syscall 0x5B. Each caller installs a fixed prefix of rows
 * directly from the table (rows 0..2 here, rows 0..1 in the other two tables).
 * For every later row it installs whatever syscall 0x5B returns for that
 * row's number, so those rows' zero handlers are placeholders. Earlier text called this a
 * "DmaChannelInit" (channel, mode) pair. That was wrong: nothing here touches
 * the DMAC (FACT #9773, task #1853). Layout unchanged: two s32 words. */
typedef struct SyscallPatchEntry {
    s32 syscallNum;
    s32 handler;
} SyscallPatchEntry;

extern SyscallPatchEntry D_00134AD0[8];
extern u8 D_00134750;
extern s32 D_00134AC8;

/**
 * func_0011F058: apply the third kernel patch (_InitSys finishes with this as its
 * tail call), driven by the syscall-patch table D_00134AD0.
 * 1. Install row 0 with SetSyscall (func_0011F048 = 0x74). That makes syscall
 *    0x5A the word-copy routine func_0011F000.
 * 2. Copy the 0x330-byte kernel patch at D_00134750 to 0x80075000 through it
 *    (func_0011EFF0 = syscall 0x5A).
 * 3. Call FlushCache (func_0011AEA0 = 0x64) with 0 and then 2, so the copied code
 *    is visible to instruction fetch. The meaning of 0 and 2 is ps2sdk's, which
 *    is external knowledge.
 * 4. Install rows 1 and 2 directly. Row 1 makes the patch at 0x80075000 syscall
 *    0x5B.
 * 5. For rows 3..7, install the handler that the new syscall 0x5B
 *    (func_0011F038) returns for that row's number.
 * 6. Store the 0x5B result for 3 in D_00134AC8.
 * Unlike func_0011F938 / func_0011FAB8 this patch has no gate.
 * Exact SDK name UNCONFIRMED.
 */
void func_0011F058(void) {
    u32 i;
    func_0011F048(D_00134AD0[0].syscallNum, D_00134AD0[0].handler);
    func_0011EFF0(0x80075000, (s32)&D_00134750, 0x330);
    func_0011AEA0(0);
    func_0011AEA0(2);
    func_0011F048(D_00134AD0[1].syscallNum, D_00134AD0[1].handler);
    func_0011F048(D_00134AD0[2].syscallNum, D_00134AD0[2].handler);
    for (i = 3; i < 8; i++) {
        func_0011F048(D_00134AD0[i].syscallNum,
                      func_0011F038(D_00134AD0[i].syscallNum));
    }
    D_00134AC8 = func_0011F038(3);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F120);

extern s32 func_0011B080(void);
extern s32 func_0011B090(void);
extern s32 func_0011F170(void);

/**
 * Dispatch on func_0011B080() (EE syscall 0x7F, current context): if it equals
 * 0x02000000 run func_0011F170, else func_0011B090; return the arm's result
 * (a free $2 passthrough). RETURNING the call result is what keeps both arms
 * as framed `jal`s converging at the shared epilogue (lever 7) — the earlier
 * void-call phrasing made ee-gcc sibling-call the func_0011F170 arm to a `j`.
 */
s32 func_0011F130(void) {
    if (func_0011B080() == 0x02000000) {
        return func_0011F170();
    }
    return func_0011B090();
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F170);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F364);

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
#ifndef TARGET_NATIVE
    __asm__ volatile("mfc0 %0, $12" : "=r"(status));
#else
    status = 0;   /* EE COP0 Status read - not host-executable */
#endif
    status &= 0x10000;
    if (status != 0) {
        do {
#ifndef TARGET_NATIVE
            __asm__ volatile("di");
            __asm__ volatile("sync.p");
            __asm__ volatile("mfc0 %0, $12" : "=r"(cur));
#else
            cur = 0;
#endif
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
#ifndef TARGET_NATIVE
    __asm__ volatile(
        "mfc0 %0, $12\n\t"
        "lui  $3, 0x1\n\t"
        "and  %0, %0, $3\n\t"
        "ei"
        : "=r"(status) :: "$3", "memory");
#else
    status = 0;   /* EE COP0 Status read + ei - not host-executable */
#endif
    return status != 0;
}

extern s32 D_00134DB8;
extern s32 D_00134DBC;

/**
 * Create the paired handles D_00134DB8 / D_00134DBC from two identical
 * descriptors (each with fields 1 and 2 set to 1) via func_0011AC20.
 */
void func_0011F640(void) {
    s32 desc1[8];
    s32 desc2[8];
    desc1[1] = 1;
    desc1[2] = 1;
    desc2[1] = 1;
    desc2[2] = 1;
    D_00134DB8 = func_0011AC20(desc1);
    D_00134DBC = func_0011AC20(desc2);
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
 * else 0 (the range check also applies at the found exit — a match at or past
 * `last` still returns 0, exactly the original's movz on the sltu). The single
 * combined `&&` loop condition is what makes ee-gcc peel the first iteration
 * and emit the deref-then-range branch-likely pair + movz tail-merge (levers
 * 5+6) — the earlier nested-if phrasing produced an ordinary beq/bne loop
 * (12.5%) and was wrongly parked as "can't match from C".
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
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x83\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F710);

extern s32 D_00134DA8[4]; /* two {syscall number, handler} pairs (ROM:
                           * {0x83, func_0011F6C0} {0x5A, func_0011F688}),
                           * the same shape as SyscallPatchEntry */
extern s32 D_00134DA0;    /* cached kernel syscall-table base (set by func_0011F718) */
extern s32 *func_0011F6C0(s32 *first, s32 *last, s32 value);
extern void func_0011F818(s32 syscall, s32 handler); /* defined later in-unit */

/**
 * Locate the kernel's syscall table and cache its base in D_00134DA0.
 *
 * Steps:
 * 1. SetSyscall (func_0011F818 = 0x74) installs the two D_00134DA8 rows. The
 *    linear find func_0011F6C0 becomes syscall 0x83, and the word copy
 *    func_0011F688 becomes syscall 0x5A.
 * 2. Syscall 0x83 (func_0011F700) runs the find in kernel mode over
 *    0x80000000..0x80080000. It searches for the address of each installed
 *    handler, which gives the table slot holding it.
 * 3. Each hit minus its slot offset is a candidate table base. The offsets are
 *    0x20C = 0x83 * 4 and 0x168 = 0x5A * 4.
 * 4. Whichever candidate is lower is advanced to the next hit until the two
 *    agree.
 *
 * The offsets are in-ROM evidence that 0x74(n, h) writes h into word n of
 * that table. ps2sdk's name for it, SetSyscall, is external knowledge.
 */
void func_0011F718(void) {
    s32 p;
    s32 q;
    s32 a;
    s32 b;

    func_0011F818(D_00134DA8[0], D_00134DA8[1]);
    func_0011F818(D_00134DA8[2], D_00134DA8[3]);
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
    D_00134DA0 = a;
}

/**
 * func_0011F818 = EE kernel syscall 0x74. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Called twice by
 * func_0011F718.
 * SetSyscall: install `handler` as the kernel's entry for syscall number
 * `syscall`. func_0011F718 shows this in-ROM: it finds its two handlers in the
 * kernel table at 0x83 * 4 and 0x5A * 4. This is the fourth copy, alongside
 * 0x11F048, 0x11F868 and 0x11FA50, so the splat name is kept.
 * Return type narrowed s32 -> void under RULING #9779 (task #1853):
 *  - ROM: the stub is `addiu $v1,$0,0x74; syscall; jr $ra; nop` and writes no
 *    $v0.
 *  - SDK: ps2sdk's prototype is `void SetSyscall(int, void *)` (external
 *    knowledge). The vendored eekernel.h has no prototype for 0x74.
 *  - Callers: the ROM has exactly 2 `jal` sites, both in func_0011F718, and
 *    neither reads $v0. In the boot ELF image the address is taken nowhere:
 *    there is no data word 0x0011F818, no %lo, and it is not in any
 *    syscall-patch table. Overlays loaded at runtime were not scanned.
 *  - The other three copies are already void.
 * The EE call in func_0011F718 was implicit int before this change, and the
 * unit object is byte-identical either way.
 */
void func_0011F818(s32 syscall, s32 handler) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

extern void func_0011F058(void);
extern void func_0011F938(void);
extern void func_0011FAB8(void);

/**
 * _InitSys: EE crt0 runtime bring-up, called once from _start before main.
 * Runs the unit's init sequence in fixed order — thread/exception scaffolding
 * (func_0011F640), locating the kernel syscall table (func_0011F718), the
 * timer-gated kernel patch (func_0011FAB8), the background worker thread
 * (func_0011B800) and the OSD-config-gated kernel patch (func_0011F938) —
 * then tail-calls func_0011F058, the ungated third patch. The three patches are
 * syscall-patch tables (see SyscallPatchEntry), not DMA-channel setup.
 */
void _InitSys(void) {
    func_0011F640();
    func_0011F718();
    func_0011FAB8();
    func_0011B800();
    func_0011F938();
    func_0011F058();
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F864);

/**
 * func_0011F868 = EE kernel syscall 0x74. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Called by
 * func_0011F938.
 * SetSyscall: install `handler` as the kernel's entry for syscall number
 * `syscall`. Identity from the ROM, not only from SDK numbering: each caller
 * walks a {syscall number, handler} table (D_00134AD0 / D_00135568 /
 * D_00135CF0, first rows {0x5A, a local word-copy routine} and
 * {0x5B, 0x8007x000}), and the very next syscall 0x5A call copies a kernel
 * patch to 0x8007x000 with exactly the (dst, src, nbytes) arguments the
 * routine just installed as 0x5A takes (func_0011F000 for D_00134AD0). The
 * SCE kernel numbering calls 0x74 RFU116 (ps2sdk: SetSyscall, external SDK
 * knowledge). The vendored eekernel.h has no prototype for it; void matches
 * ps2sdk's and no caller reads $v0 (task #1848). One of four file-local
 * copies (0x11F048, 0x11F818, 0x11F868, 0x11FA50; the libkernl table entry
 * is 0x11AFA0), so the splat name is kept rather than a duplicate name.
 */
void func_0011F868(s32 syscall, s32 handler) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

/**
 * func_0011F878 = EE kernel syscall 0x5A. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. func_0011F938
 * calls it with (dst, src, nbytes) right after its SetSyscall has made 0x5A the
 * word-copy routine func_0011F888. The call therefore copies a kernel patch
 * (see SyscallPatchEntry). Exact SDK name UNCONFIRMED.
 */
s32 func_0011F878(s32 a, s32 b, s32 c) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x5A\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
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
 * in $v0. Takes one argument in $a0, a syscall number from D_00135568. By the
 * time func_0011F938 calls it, 0x5B has been replaced by the kernel patch at
 * 0x80074000, and the result is installed as that number's handler. Exact SDK
 * name UNCONFIRMED.
 */
s32 func_0011F8C0(s32 a) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x5B\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

/**
 * Probe whether the kernel keeps bits 13..15 of the OSD config word. Steps:
 * 1. Read the word (func_0011ACD0 = 0x4B, GetOsdConfigParam).
 * 2. Write it back with that field set to 1 (func_0011ACC0 = 0x4A,
 *    SetOsdConfigParam).
 * 3. Read it again, then restore the original word.
 * Returns 1 if the field still reads 0, else 0. func_0011F938 applies its
 * kernel patch only when this returns 1. The 0x4A/0x4B names are external SDK
 * numbering (task #1848).
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

/* D_00135568 / D_00135CF0 are the syscall-patch tables for func_0011F938 and
 * func_0011FAB8; see SyscallPatchEntry above (defined for func_0011F058). */
extern SyscallPatchEntry D_00135568[3];
extern u8 D_00134DC0;

/**
 * func_0011F938: apply a kernel patch driven by the syscall-patch table
 * D_00135568. It does nothing unless func_0011F8D0's OSD-config probe returns 1.
 * 1. Install row 0 with SetSyscall (func_0011F868 = 0x74). That makes syscall
 *    0x5A the word-copy routine func_0011F888.
 * 2. Copy the 0x7A8-byte patch at D_00134DC0 to 0x80074000 through it
 *    (func_0011F878 = 0x5A).
 * 3. Call FlushCache (func_0011AEA0 = 0x64) with 0 and then 2.
 * 4. Install row 1, which makes the patch at 0x80074000 syscall 0x5B.
 * 5. For row 2, install the handler that the new syscall 0x5B
 *    (func_0011F8C0) returns for that row's number. The ROM number there is
 *    0xFFFFC402, which is not a positive syscall number. It is passed through
 *    unchanged.
 * Exact SDK name UNCONFIRMED.
 */
void func_0011F938(void) {
    u32 i;
    if (func_0011F8D0()) {
        func_0011F868(D_00135568[0].syscallNum, D_00135568[0].handler);
        func_0011F878(0x80074000, (s32)&D_00134DC0, 0x7A8);
        func_0011AEA0(0);
        func_0011AEA0(2);
        func_0011F868(D_00135568[1].syscallNum, D_00135568[1].handler);
        for (i = 2; i < 3; i++) {
            func_0011F868(D_00135568[i].syscallNum,
                          func_0011F8C0(D_00135568[i].syscallNum));
        }
    }
}

/* 0x11F9E4 is one 0xCDCDCDCD fill word that spimdisasm fused onto the head of
 * func_0011F9E8; the start-only `type:func` pin on 0x11F9E8 (FACT #7390's
 * method) carves it off as this 0x4 fragment. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F9E4);

/**
 * Copy nbytes bytes from src to dst and return 0 — the byte-wise twin of the
 * word-copy routine func_0011F888. Reached only through data: it is the handler
 * of D_00135568's fourth row {0x5A, func_0011F9E8}, a syscall-0x5A copy routine
 * like row 0's; func_0011F938's install loop stops at row 2, and no ROM
 * instruction references the row or the routine directly.
 */
s32 func_0011F9E8(u8 *dst, u8 *src, u32 nbytes) {
    u32 i;
    for (i = 0; i < nbytes; i++) {
        *dst++ = *src++;
    }
    return 0;
}

/**
 * Shutdown hook thunk: void tail call to the func_0011F130 dispatcher. ee-gcc
 * sibling-call-optimises it into the original's `j func_0011F130` — the callee
 * being a framed non-leaf is irrelevant to the caller-local sibcall decision.
 */
void func_0011FA18(void) {
    func_0011F130();
}

extern void func_0011A840(s32 code);

/**
 * libc exit(): run the func_0011FA18 shutdown hook (the func_0011F130
 * dispatcher thunk), then hand `code` to the terminator func_0011A840 —
 * a void tail call, sibcall-optimised to the original's `j func_0011A840`
 * with the frame restore folded into the delay slot (lever 7, framed form;
 * `code` rides callee-saved $16 across the hook call).
 */
void exit(s32 code) {
    func_0011FA18();
    func_0011A840(code);
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FA48);

/**
 * func_0011FA50 = EE kernel syscall 0x74 (same primitive as func_0011F868).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap. Called by func_0011FAB8.
 * SetSyscall: install `handler` as the kernel's entry for syscall number
 * `syscall`. Identity from the ROM, not only from SDK numbering: each caller
 * walks a {syscall number, handler} table (D_00134AD0 / D_00135568 /
 * D_00135CF0, first rows {0x5A, a local word-copy routine} and
 * {0x5B, 0x8007x000}), and the very next syscall 0x5A call copies a kernel
 * patch to 0x8007x000 with exactly the (dst, src, nbytes) arguments the
 * routine just installed as 0x5A takes (func_0011F000 for D_00134AD0). The
 * SCE kernel numbering calls 0x74 RFU116 (ps2sdk: SetSyscall, external SDK
 * knowledge). The vendored eekernel.h has no prototype for it; void matches
 * ps2sdk's and no caller reads $v0 (task #1848). One of four file-local
 * copies (0x11F048, 0x11F818, 0x11F868, 0x11FA50; the libkernl table entry
 * is 0x11AFA0), so the splat name is kept rather than a duplicate name.
 */
void func_0011FA50(s32 syscall, s32 handler) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

/**
 * func_0011FA60 = EE kernel syscall 0x5A (same primitive as func_0011F878).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap. func_0011FAB8 calls it twice with (dst, src, nbytes) right
 * after its SetSyscall has made 0x5A the word-copy routine func_0011FA70. The
 * calls therefore copy kernel patch data (see SyscallPatchEntry). Exact SDK
 * name UNCONFIRMED.
 */
s32 func_0011FA60(s32 a, s32 b, s32 c) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x5A\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
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
 * $v1 and trap; result returned in $v0. Takes one argument in $a0, a syscall
 * number from D_00135CF0. By the time func_0011FAB8's loop calls it, 0x5B has
 * been replaced by the kernel patch at 0x80076000, and the result is installed
 * as that number's handler. Exact SDK name UNCONFIRMED.
 */
s32 func_0011FAA8(s32 a) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x5B\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

extern SyscallPatchEntry D_00135CF0[8];
extern u8 D_00135588;
extern u8 D_00135CC8;

/**
 * func_0011FAB8: apply a kernel patch driven by the 8-row syscall-patch table
 * D_00135CF0. It does nothing if bit 0x100 of the hardware register 0x10001810
 * is set.
 * 1. Install row 0 with SetSyscall (func_0011FA50 = 0x74). That makes syscall
 *    0x5A the word-copy routine func_0011FA70.
 * 2. Make two copies through it (func_0011FA60 = 0x5A):
 *    - the 0x740-byte patch at D_00135588 to 0x80076000;
 *    - the 0x28 bytes at D_00135CC8 to 0x82000.
 * 3. Call FlushCache (func_0011AEA0 = 0x64) with 0 and then 2.
 * 4. Install row 1, which makes the patch at 0x80076000 syscall 0x5B.
 * 5. For rows 2..7, install the handler that the new syscall 0x5B
 *    (func_0011FAA8) returns for that row's number.
 * Exact SDK name UNCONFIRMED.
 */
void func_0011FAB8(void) {
    u32 i;
    if ((*(volatile s32 *)0x10001810 & 0x100) == 0) {
        func_0011FA50(D_00135CF0[0].syscallNum, D_00135CF0[0].handler);
        func_0011FA60(0x80076000, (s32)&D_00135588, 0x740);
        func_0011FA60(0x82000, (s32)&D_00135CC8, 0x28);
        func_0011AEA0(0);
        func_0011AEA0(2);
        func_0011FA50(D_00135CF0[1].syscallNum, D_00135CF0[1].handler);
        for (i = 2; i < 8; i++) {
            func_0011FA50(D_00135CF0[i].syscallNum,
                          func_0011FAA8(D_00135CF0[i].syscallNum));
        }
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FB8C);

extern s32 D_001417EC;
extern void func_0011FB98(void);

/**
 * One-shot initialiser: the first time it is called (guard word D_001417EC is
 * still zero) it sets the guard and tail-calls func_0011FB98 to do the real
 * setup; subsequent calls do nothing.
 */
void func_0011FC48(void) {
    if (D_001417EC == 0) {
        D_001417EC = 1;
        func_0011FB98();
    }
}

/*
 * __divdi3 (libgcc `__divdi3`, 0x11FC68) is not in this unit: the EE build links
 * it from GCC's own libgcc2.c as the libgcc.a member _divdi3.o
 * (going-decompiled/libgcc/, RULING #8206), placed between this unit and
 * cod/0202D8. The portable build has no libgcc2 of its own, so it keeps the
 * behavioural equivalent (signed 64-bit division, a / b). Excluded domains, as
 * on the EE (`break 0,7` there): b == 0 and INT64_MIN / -1.
 */
#ifdef TARGET_NATIVE
s64 __divdi3(s64 a, s64 b) {
    return a / b;
}
#endif

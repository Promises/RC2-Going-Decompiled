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
extern s32 D_0013A308;

/**
 * Accessor: return the global pointer/handle D_00133E74 (the base of the
 * subsystem context block this unit operates on).
 */
s32 func_00115200(void) {
    return D_00133E74;
}

/**
 * Accessor: return the address of the global D_0013A308.
 */
s32 *func_00115210(void) {
    return &D_0013A308;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115220);

/* func_00115228: thin wrapper -- calls func_00115210 and returns its result.
 * ROM arm kept: the original builds a 0x10 frame and saves $31 with sq/lq
 * (quadword), which this cc1 does not emit for a leaf-ish wrapper. A
 * byte-MATCHING wall, not a bar to a faithful portable arm. NOT a byte-match
 * claim. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115228);
#else
/**
 * Wrapper around func_00115210: returns &D_0013A308.
 *
 * ⚠️ THE ROM PASSES AN ARGUMENT THAT THE CALLEE IGNORES, and this arm
 * deliberately does not reproduce it. Verified rather than assumed:
 *
 *   func_00115228.s   lw   $4, %lo(D_00133E74)($2)   <- loads a value into arg1
 *                     jal  func_00115210             <- 0x0C045484 -> 0x00115210
 *   func_00115210.s   lui  $2, %hi(D_0013A308)
 *                     jr   $31
 *                     addiu $2, $2, %lo(D_0013A308)  <- never reads $4
 *
 * So the load is dead at the callee, and func_00115210 is already decompiled
 * here as `(void)`. Reproducing the dead argument would mean declaring a
 * parameter the callee does not have, purely to imitate a register write with
 * no observable effect -- D_00133E74 is ordinary bss, not MMIO, so the read
 * itself is unobservable too.
 *
 * Most likely the original called this through a declaration carrying a
 * context parameter that the implementation dropped -- common in this SDK
 * unit's accessor family. That is a guess about the ORIGINAL SOURCE and is
 * marked as one; the .s facts above are not.
 */
s32 *func_00115228(void) {
    return func_00115210();
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115250);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", memcmp);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", memcpy);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", memset);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", strcmp);
INCLUDE_ASM_ALIAS(func_00115544, strcmp);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115690);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", strlen);
INCLUDE_ASM_ALIAS(func_001157AC, strlen);

/* 0xCDCDCDCD inter-function-fill class. Each of these symbols is one or more
 * leading 0xCDCDCDCD debug-fill words (sometimes with dead stores/nops) emitted
 * between functions; spimdisasm folds the fill into the FOLLOWING symbol and
 * demotes the real prologue to an interior `alabel`.
 *
 * Task #472 (FACT #7359): where the real start is a `jal` target in the ROM
 * (func_001158F8, func_00124418, func_00125D98, func_00126108, func_00126288,
 * func_00126470, func_00126730, func_00126DC0, func_00128F50, func_001290E0)
 * a `type:func` pin on the real start alone makes splat carve the fill off as
 * its own fragment (a lone CD word terminates fine as a 0x4 symbol — see
 * func_001253A4) and the body gets its own .s AS LONG AS an INCLUDE_ASM names
 * it: splat writes nonmatchings/<unit>/<fn>.s only for names the .c references
 * (segtypes/common/c.py global_asm_funcs), which is the "coverage hole" an
 * earlier attempt hit. The pad fragments below are INCLUDE_ASM_FRAGMENT.
 *
 * Still fused: func_0011F364, func_0011FB8C, func_00130A8C — their real bodies
 * are reached only by `j`/data reference, never by `jal`, so they are the
 * j-target class of task #471, not a carve defect of this class. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001158F4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001158F8);

/* func_00115AC0 = strncpy(char *dst, const char *src, size_t n).
 * Returns $2 = the ORIGINAL dst. Identity CONFIRMED two independent ways:
 *   1. Algorithm read of the asm - classic SIMD zero-byte detect
 *      (~w) & (w - 0x0101..01) & 0x8080..80, done 128-bit via
 *      pnor/psubb/pand/pcpyud on lq/sq pairs (16B/iter) when src|dst is
 *      16-byte aligned, 64-bit via nor/dsubu/and on ld/sd (8B/iter) when only
 *      8-byte aligned, else a plain lbu/sb byte loop.
 *   2. tools/ee/sdk-lib/obj/strncpy.o (extracted from the vendored SCE
 *      libc.a) is the SAME ROUTINE: its .symtab names the source file
 *      src/newlib/libc/machine/r5900/strncpy.S, and its 444 code bytes are
 *      instruction-for-instruction identical to this function apart from the
 *      constant-materialization idiom noted below.
 *
 * NON-OBVIOUS, CALLER-VISIBLE BEHAVIOUR: this does NOT guarantee NUL
 * termination. The beqz $6 at 0x115C38 jumps straight to jr $31 when n is
 * exhausted before a NUL is seen, storing no terminator. The zero-fill tail at
 * .L00115C68 only runs when the SOURCE ended early (it pads the remainder of n
 * with 0). So on truncation the caller gets an unterminated buffer and must
 * write its own terminator - that is why callers such as the one below pass
 * 0x3FF into a 0x400-byte field.
 *
 * PARKED as INCLUDE_ASM - not matchable from C, and NOT for lack of trying.
 * This is hand-written R5900 assembly in the SDK (a .S file, per the symtab
 * above), so there is no C source to recover. MEASURED: compiling the 64-bit
 * zero-detect idiom above with our ee-gcc 2.9 -O2 -G0 emits ONLY scalar
 * ld/sd/nor/daddu/and - 0 occurrences of lq/sq/pnor/psubb/pand/pcpyld/pcpyud.
 * ee-gcc 2.9 has no autovectorizer and no intrinsics for the 128-bit ops, so
 * the lq/sq path is unreachable from portable C (the same wall already
 * recorded for the 128-bit lq/sq bodies in text/1CA080.c and elsewhere).
 *
 * The FREE-MATCH shortcut (link the SDK object verbatim) also does NOT apply:
 * our vendored libc.a is a DIFFERENT SDK revision than the one the game
 * linked. Both build the same masks to the same values (verified numerically:
 * 0x0101010101010101 and 0x8080808080808080) but by different li expansions -
 * the ROM does ori $7,$0,0x8080 / dsll 16 / ori / dsll 16 / ori / dsll 9 /
 * ori 0x101 (7 insns, deriving 0x0101.. from the 0x8080.. chain via dsll 9),
 * while the vendored object does lui $7,0x101 / ori / dsll 16 / ori / dsll 16
 * / ori (6 insns). That shifts every later branch displacement, so 97 of 111
 * words differ. No other libc.a revision is present in the tree (searched),
 * and no vendored object contains the ROM's dsll $7,$7,9 idiom.
 * USA and EU are identical here (same vaddr, same instructions). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", strncpy);
INCLUDE_ASM_ALIAS(func_00115AC0, strncpy);

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
 * func_0011AAB0 = EE kernel syscall 0x29 (RotateThreadReadyQueue).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel's result is returned in $v0. Called with two
 * arguments by func_0011B800 (thread id + a constant 1).
 */
void func_0011AAB0(s32 thid, s32 arg) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x29\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAC0);

/**
 * func_0011AAD0 = EE kernel syscall 0x2B (ReleaseWaitThread).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel releases the given thread from its wait state and
 * returns its result in $v0. Called by func_0011B728 with a thread id.
 */
s32 func_0011AAD0(s32 thid) {
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
 * $v1 and trap; the kernel deletes the semaphore identified by $a0.
 */
void func_0011AC30(s32 obj) {
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
 * func_0011ACC0 = EE kernel syscall 0x4A. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Called as a
 * register-write primitive: the $a0 pointer holds the value the kernel writes
 * to a hardware register (see func_0011F8D0). Exact SDK name UNCONFIRMED.
 */
void func_0011ACC0(s32 *in) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x4A\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

/**
 * func_0011ACD0 = EE kernel syscall 0x4B. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Called as a
 * register-read primitive: the kernel stores the register value through the
 * $a0 pointer (see func_0011F8D0). Exact SDK name UNCONFIRMED.
 */
void func_0011ACD0(s32 *out) {
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
 * func_0011AEA0 = EE kernel syscall 0x64. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap; the kernel's
 * result is returned in $v0. Takes one argument in $a0 (a mode/channel
 * selector — see the DMA channel setup paths func_0011F938/func_0011FAB8).
 * Exact SDK name UNCONFIRMED.
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B1E8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BB38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BCD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BE20);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", Kprintf);

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

/* func_0011CB58: subsystem reset — DisableDmac(5); func_0011A950(5,
 * D_0013CF54); D_0013469C = 0. Body is structurally identical at 98.46%, but the
 * original allocates $3 (v1) for every %hi address temporary while ee-gcc picks
 * $2 (v0); a one-register global allocation offset this cc1 won't reproduce from
 * source. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CB58);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CBE8);

extern s32 func_0011CBE8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                         s32 arg5, s32 arg6);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D140);

/**
 * Reset object arg0: clear its field_0x18 (arg0[6]) and clear bit 0 of the flag
 * word field_0x10 (arg0[4]) — i.e. mark it inactive/idle.
 */
void func_0011D1E8(s32 *arg0) {
    arg0[6] = 0;
    arg0[4] &= 0xFFFFFFFE;
}

/* func_0011D208: allocate the next slot of a circular pool described by arg0
 * (arg0[5]=slot base, arg0[6]=slot count, arg0[9]=counter). index = counter %
 * count; stores counter+1 back; returns &slot[index] (0x40-byte slots). The C
 * body `arg0[5] + ((arg0[9] % arg0[6]) << 6)` reproduces every instruction, but
 * the div-by-zero trap is `break 0,7` in the original and GNU as encodes
 * ee-gcc's `break 7` in the upper code field instead — an assembler-encoding
 * mismatch (2 words), not a source issue. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D208);

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

extern s32 *func_0011D208(s32 idx);

/**
 * Enqueue a command against the ring slot for `idx`: allocate the slot via
 * func_0011D208, copy the descriptor fields obj[5]/obj[7] into it (explicit
 * temps — the original loads both fields up-front before any argument
 * builds), stamp the command word 0x8000000C at slot[8], then void-tail-call
 * func_0011CD60(0x80000008, slot, 0x40, obj[8], obj[9], obj[10]) — six plain
 * EABI register args, sibcall-optimised to the original's `j` with the
 * sp-restore in the delay slot (lever 7).
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", rename);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceSifBindRpc);

/* func_0011D590(arg0): link the element arg0->field_0x34 into its manager's
 * (elem->field_0x40) active list — patching the tail's back-link (+0x3C) or the
 * head (+0xC) — then copy a block of transform/state fields from arg0 into the
 * element, and kick processing via func_0011B8D8() when the manager's count
 * (+0x0) is non-negative and busy flag (+0x4) is clear. ~86%: behaviour fully
 * recovered, but ee-gcc won't reproduce the original's branch-likely with an
 * annulled speculative load on the head/tail test. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D590);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D850);

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
extern void func_0011AC40(s32 arg);
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

extern void func_0011AC40(s32 sema);
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
 * Run the func_0011DD48 teardown step, then forward the global handle
 * D_00134734 to func_0011AC60. Always returns 0.
 */
s32 func_0011DD98(void) {
    func_0011DD48();
    func_0011AC60(D_00134734);
    return 0;
}

extern void func_0011AC40(s32 sema);

/**
 * Release the singleton table lock: forward the global semaphore handle
 * D_00134734 to func_0011AC40 (SignalSema). A void tail call, so ee-gcc
 * sibling-call-optimises it into the original's frameless `j func_0011AC40`
 * (the handle load rides the jump's delay slot).
 */
void func_0011DDC8(void) {
    func_0011AC40(D_00134734);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011DDD8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011DE08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E010);

extern s32 D_0013472C;
extern u8 D_001400A8[4];

/**
 * Reset the subsystem state guarded by D_0013472C: clear the flag word to 0 and
 * zero the 4-byte descriptor at D_001400A8. Always returns 0.
 */
s32 func_0011E0A0(void) {
    D_0013472C = 0;
    memset(D_001400A8, 0, 4);
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E0D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E360);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E4E0);

/* func_0011E740: 0x60 bytes of inter-function padding (`addiu sp,+0xN; nop`
 * filler words) split off by symbol_addrs size:0x60; the real function begins at
 * sceSifInitIopHeap. Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E740);

/* sceSifInitIopHeap: real function recovered from the splat mis-split above (init/
 * retry loop around sceSifBindRpc, writes D_00134744). Boundary now correct;
 * body not yet decompiled. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceSifInitIopHeap);

extern s32 func_0011D620(void *a0, s32 a1, s32 a2, void *a3, s32 a4,
                         void *a5, s32 a6, s32 a7, s32 a8);
extern s32 D_00134744;
extern s32 D_00140140;
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceSifFreeSysMemory);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E920);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E938);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011EA38);

extern s32 D_00134748;
extern u8 D_00140528[4];

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011EB00);

extern void func_0011EB00(s32 arg0, s32 arg1, s32 arg2, void *outbuf);

/**
 * Forward (arg0, arg1, arg2) to func_0011EB00, supplying a 16-byte scratch
 * buffer on the stack as its fourth (output) argument.
 */
void func_0011ED08(s32 arg0, s32 arg1, s32 arg2) {
    u8 buf[16];
    func_0011EB00(arg0, arg1, arg2, buf);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ED28);

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
 * func_0011AA20): load the syscall number into $v1 and trap. Installs a DMA/INTC
 * handler over a buffer; called from func_0011F058 with a 3-word argument
 * (handler addr, buffer, length). Same primitive as func_0011F878. Exact SDK
 * name UNCONFIRMED.
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
 * $v0. Takes one argument in $a0 (a channel id — see func_0011F058). Same
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
 * func_0011AA20): load the syscall number into $v1 and trap. Used during DMA
 * channel setup (see func_0011F058) with a 2-word argument; callers ignore the
 * result, so this is modelled as void. Same primitive as func_0011F868. Exact
 * SDK name UNCONFIRMED.
 */
void func_0011F048(s32 a, s32 b) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

/* A single DMA channel descriptor in the static init table D_00134AD0: a
 * (channel-id, mode) word pair consumed by the syscall stubs. Same layout as the
 * D_00135568 / D_00135CF0 tables used by func_0011F938 / func_0011FAB8. */
typedef struct DmaChannelInit {
    s32 channel;
    s32 mode;
} DmaChannelInit;

extern DmaChannelInit D_00134AD0[8];
extern u8 D_00134750;
extern s32 D_00134AC8;

/**
 * func_0011F058: bring up the third DMA-channel group (the one _InitSys finishes
 * with its tail call). Arms channel entry[0] (func_0011F048 = syscall 0x74),
 * installs the 0x80075000 handler over D_00134750 spanning 0x330 bytes
 * (func_0011EFF0 = syscall 0x5A), toggles the interrupt-enable syscall
 * (func_0011AEA0 = 0x64) off then on, arms entries[1] and [2] directly, then for
 * the remaining entries (3..7) queries each channel (func_0011F038 = syscall
 * 0x5B) and re-arms it with the returned value. Finally stores func_0011F038(3)
 * into D_00134AC8. Unlike func_0011F938 / func_0011FAB8 this group has no
 * hardware gate. Exact SDK name UNCONFIRMED.
 */
void func_0011F058(void) {
    u32 i;
    func_0011F048(D_00134AD0[0].channel, D_00134AD0[0].mode);
    func_0011EFF0(0x80075000, (s32)&D_00134750, 0x330);
    func_0011AEA0(0);
    func_0011AEA0(2);
    func_0011F048(D_00134AD0[1].channel, D_00134AD0[1].mode);
    func_0011F048(D_00134AD0[2].channel, D_00134AD0[2].mode);
    for (i = 3; i < 8; i++) {
        func_0011F048(D_00134AD0[i].channel, func_0011F038(D_00134AD0[i].channel));
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

extern s32 D_00134DA8[4]; /* handler table: two {arg0,arg1} pairs */
extern s32 D_00134DA0;    /* cached end-of-walk pointer (func_0011F6C0 side) */
extern s32 *func_0011F6C0(s32 *first, s32 *last, s32 value);

/**
 * Install the unit's exception/interrupt handlers and walk the two parallel
 * handler regions to their common end.
 *
 * First registers two handlers via func_0011F818 (syscall 0x74) from the table
 * D_00134DA8 (a pair of {arg0,arg1} entries). Then it seeds two cursors with
 * func_0011F700 (syscall 0x83) over the 0x80000000..0x80080000 range, one
 * keyed on func_0011F6C0 and one on func_0011F688, and advances whichever
 * cursor (offset back by its handler's fixed bias, 0x20C / 0x168) is behind
 * until the two biased cursors meet. The meeting point is cached in D_00134DA0.
 */
void func_0011F718(void) {
#ifdef TARGET_NATIVE
    extern s32 func_0011F818(s32 a, s32 b);  /* defined later in-unit */
#endif
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
 * func_0011F818 = EE kernel syscall 0x74 (same primitive as func_0011F868).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap. Called from the device/handler init path (func_0011F718) with
 * a 2-word argument. Exact SDK name UNCONFIRMED.
 */
s32 func_0011F818(s32 a, s32 b) {
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
 * (func_0011F640), device/handler init (func_0011F718), the GS/DMA reset path
 * (func_0011FAB8), the background worker thread (func_0011B800) and the first
 * DMA-channel group (func_0011F938) — then tail-calls func_0011F058 to finish.
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
 * func_0011AA20): load the syscall number into $v1 and trap. Used during DMA
 * channel setup (see func_0011F938) with a 2-word argument; callers ignore the
 * result, so this is modelled as void. Exact SDK name UNCONFIRMED.
 */
void func_0011F868(s32 a, s32 b) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

/**
 * func_0011F878 = EE kernel syscall 0x5A. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Used during DMA
 * channel setup (see func_0011F938) with a 3-word argument. Exact SDK name
 * UNCONFIRMED.
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
 * in $v0. Takes one argument in $a0 (a channel id — see func_0011F938).
 * Exact SDK name UNCONFIRMED.
 */
s32 func_0011F8C0(s32 a) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x5B\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
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

/* D_00135568 / D_00135CF0 are the (channel-id, mode) init tables for the first
 * two DMA-channel groups; see DmaChannelInit above (defined for func_0011F058). */
extern DmaChannelInit D_00135568[3];
extern u8 D_00134DC0;

/**
 * func_0011F938: bring up the first DMA-channel group. Gated on func_0011F8D0
 * (only runs when the hardware mode field reads back clean). Arms channel
 * entry[0] (func_0011F868 = syscall 0x74), installs the 0x80074000 handler over
 * D_00134DC0 spanning 0x7A8 bytes (func_0011F878 = syscall 0x5A), toggles the
 * interrupt-enable syscall (func_0011AEA0 = 0x64) off then on, arms entry[1],
 * then for the remaining entries (index 2) queries each channel
 * (func_0011F8C0 = syscall 0x5B) and re-arms it with the returned value.
 * Exact SDK name UNCONFIRMED.
 */
void func_0011F938(void) {
    u32 i;
    if (func_0011F8D0()) {
        func_0011F868(D_00135568[0].channel, D_00135568[0].mode);
        func_0011F878(0x80074000, (s32)&D_00134DC0, 0x7A8);
        func_0011AEA0(0);
        func_0011AEA0(2);
        func_0011F868(D_00135568[1].channel, D_00135568[1].mode);
        for (i = 2; i < 3; i++) {
            func_0011F868(D_00135568[i].channel,
                          func_0011F8C0(D_00135568[i].channel));
        }
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F9E4);

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
 * $v1 and trap. Called from the GS/DMA reset path (func_0011FAB8) with a
 * 2-word argument; callers ignore the result, so this is modelled as void.
 * Exact SDK name UNCONFIRMED.
 */
void func_0011FA50(s32 a, s32 b) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

/**
 * func_0011FA60 = EE kernel syscall 0x5A (same primitive as func_0011F878).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap. Called from the GS/DMA reset path (func_0011FAB8) with a
 * 3-word argument. Exact SDK name UNCONFIRMED.
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
 * $v1 and trap; result returned in $v0. Takes one argument in $a0 (a channel
 * id). Called in a loop from the GS/DMA reset path (func_0011FAB8). Exact SDK
 * name UNCONFIRMED.
 */
s32 func_0011FAA8(s32 a) {
#ifndef TARGET_NATIVE
    __asm__ volatile("addiu $3, $0, 0x5B\n\tsyscall 0" ::: "$3", "memory");
#else  /* EE kernel syscall - not executable on the native host */
#endif
}

extern DmaChannelInit D_00135CF0[8];
extern u8 D_00135588;
extern u8 D_00135CC8;

/**
 * func_0011FAB8: bring up the second (8-entry) DMA-channel group, used by the
 * GS/DMA reset path. Skips entirely when timer/DMAC register 0x10001810 has
 * bit 0x100 set (work already in progress). Otherwise arms entry[0]
 * (func_0011FA50 = syscall 0x74), installs two handlers via func_0011FA60
 * (syscall 0x5A) — 0x80076000 over D_00135588 (0x740 bytes) and 0x82000 over
 * D_00135CC8 (0x28 bytes) — toggles the interrupt-enable syscall
 * (func_0011AEA0 = 0x64) off then on, arms entry[1], then for entries 2..7
 * queries each channel (func_0011FAA8 = syscall 0x5B) and re-arms it with the
 * returned value. Exact SDK name UNCONFIRMED.
 */
void func_0011FAB8(void) {
    u32 i;
    if ((*(volatile s32 *)0x10001810 & 0x100) == 0) {
        func_0011FA50(D_00135CF0[0].channel, D_00135CF0[0].mode);
        func_0011FA60(0x80076000, (s32)&D_00135588, 0x740);
        func_0011FA60(0x82000, (s32)&D_00135CC8, 0x28);
        func_0011AEA0(0);
        func_0011AEA0(2);
        func_0011FA50(D_00135CF0[1].channel, D_00135CF0[1].mode);
        for (i = 2; i < 8; i++) {
            func_0011FA50(D_00135CF0[i].channel,
                          func_0011FAA8(D_00135CF0[i].channel));
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

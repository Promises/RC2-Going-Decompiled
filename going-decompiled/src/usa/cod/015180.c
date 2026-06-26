#include "common.h"

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115220);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115228);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115250);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", memcmp);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", memcpy);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", memset);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115544);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115690);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001157AC);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001158F4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115AC0);

/* func_00115C90: clears D_00133E78, calls func_0011B270(arg1); on failure
 * (-1) writes the resulting D_00133E78 error code back through arg0. Logic
 * matches but NOT byte-exact: the original saves $16/$17/$31 with 128-bit `sq`
 * while this cc1 emits `sd` for callee-saves. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115C90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115CF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115D38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115D48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115DA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115E28);

/* func_00115E68: tail-calls func_001175F0(arg0, 0, 0xA) and returns its result
 * sign-extended from 32 to 64 bits. Not matched: the original saves $31 with a
 * 128-bit `sq` (not the `sd` ee-gcc emits here at -O2 -G0) and carries an extra
 * dsll32/dsra32 sign-extend that cc1 elides for an s32-returning callee. Both
 * are codegen/ABI forms this compiler won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115E68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115E90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115F28);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00118CC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00118D98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00119AC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00119BC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00119BF8);

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
 * objdiff pairs it by symbol against the frozen asm.
 */
void func_0011AA20(void) {
    __asm__ volatile("addiu $3, $0, 0x20\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA30);

/**
 * func_0011AA40 = EE kernel syscall 0x22 (StartThread).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel's result is returned in $v0.
 */
void func_0011AA40(void) {
    __asm__ volatile("addiu $3, $0, 0x22\n\tsyscall 0" ::: "$3", "memory");
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
 * $v1 and trap; the kernel's result is returned in $v0.
 */
void func_0011AAB0(void) {
    __asm__ volatile("addiu $3, $0, 0x29\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB00);

/**
 * func_0011AB10 = EE kernel syscall 0x2F (GetThreadId).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap; the kernel returns the current thread id in $v0.
 */
void func_0011AB10(void) {
    __asm__ volatile("addiu $3, $0, 0x2F\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB90);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC60);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AEA0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B268);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B270);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B320);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B328);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B3D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B450);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B458);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B500);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B580);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B588);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B5F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", DisableDmac);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", EnableDmac);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B728);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B800);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B8D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B970);

extern void func_0011B050(s32 count, s32 *value);

/* func_0011B978(arg0, arg1, arg2): pack a four-word command record (low 16 bits
 * of arg0, arg1, arg2, and the uncached-mirror address of D_0013CA10 ORed with
 * 0x20000000) and push it through func_0011B050 with count 1. Instructions are
 * essentially identical but NOT byte-exact: the original schedules the prologue
 * `sd $31` and `move a1,sp` after the record stores, an ordering ee-gcc won't
 * reproduce from source. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B978);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B9C0);

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

/* func_0011BA20 / func_0011BA58: pack arg0, arg1 and the low 16 bits of arg2
 * into a stack record and push it through func_0011B050 (count -5 / -6). Not
 * matched: the original moves arg1 out of $5 into a temp before reusing $5 for
 * the record address, so it stores arg1 via the temp. ee-gcc instead stores
 * arg1 directly from $5 before clobbering it — a register-allocation/scheduling
 * order this cc1 won't reproduce from C. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BA20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BA58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BA90);

/**
 * Push the single 32-bit value arg0 through func_0011B050 with count 0x10
 * (the value is passed by address in a local).
 */
void func_0011BAA0(s32 arg0) {
    s32 value = arg0;
    func_0011B050(0x10, &value);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BAC4);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BB38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BCD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BE20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BEDC);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BF18);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C000);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C090);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C1F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C7E8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C8C8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CF74);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D238);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D2F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D350);

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

/* sceSifCheckStatRpc: validity predicate for the handle in arg0 — returns 1 iff
 * arg0[0] points to a live object, arg0[1] matches obj[6] (the +0x18 id/gen),
 * and obj[4] (+0x10) bit 0 is set; else 0. ~65% — ee-gcc collapses the final
 * if/else into `andi v0,v0,1` and inverts the id-check branch (beql) instead of
 * the original two-exit branch shape. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceSifCheckStatRpc);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D8C8);

/* func_0011D950: look up slot `idx` in the fixed 0x20-entry table D_0013FE80
 * (0x10-byte stride). After the lazy-init (func_0011D868) and acquiring the
 * table lock (func_0011AC60(D_00134738)), release the lock (func_0011AC40) and
 * return the slot address when idx (unsigned) is in range, else 0. Body
 * `func_0011D868(); func_0011AC60(D_00134738); if (idx >= 0x20) {
 * func_0011AC40(D_00134738); return 0; } slot = &D_0013FE80[idx*0x10];
 * func_0011AC40(D_00134738); return slot;` reaches 99.6% — every instruction
 * lines up except the sltiu range-check lands in $2 here while the original
 * allocates it to $3 (keeping $2 for the table base). A one-register
 * allocation choice this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D950);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D9C0);

extern void func_0011AC60(s32 handle);
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

/* func_0011DDC8: tail-call forward of the global handle D_00134734 to
 * func_0011AC40 (the original is a frameless `j func_0011AC40`). ee-gcc 2.9 does
 * not apply sibling-call optimisation for this shape — it emits a full jal with
 * a stack frame — so it can't match from C. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011DDC8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011EFE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011EFF0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F038);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F048);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F058);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F120);

/* func_0011F130: dispatch on func_0011B080() (EE syscall 0x7F, current context):
 * if it equals 0x02000000 call func_0011F170, else call func_0011B090. The
 * original keeps one frame and uses jal for both arms (converging at a shared
 * epilogue), but ee-gcc sibling-call-optimises the func_0011F170 arm into a
 * tail `j` and hoists the $ra restore into the bne delay slot — a codegen-shape
 * mismatch not expressible in source. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F130);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F170);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F364);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F5E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F628);

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

/* func_0011F6C0(first, last, value): linear find — return the first pointer in
 * [first,last) whose word equals value, else 0. The original is a frameless leaf
 * built from branch-likely (beql/bnel) tests and movz tail-merges at the found/
 * not-found joins; ee-gcc 2.9 at -O2 emits an ordinary beq/bne loop (12.5%), not
 * this conditional-move form, so it can't match from C. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F6C0);

/**
 * func_0011F700 = EE kernel syscall 0x83. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Called from the
 * device/handler init path (func_0011F718) with a 3-word argument. Exact SDK
 * name UNCONFIRMED.
 */
s32 func_0011F700(s32 a, s32 b, s32 c) {
    __asm__ volatile("addiu $3, $0, 0x83\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F710);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F718);

/**
 * func_0011F818 = EE kernel syscall 0x74 (same primitive as func_0011F868).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap. Called from the device/handler init path (func_0011F718) with
 * a 2-word argument. Exact SDK name UNCONFIRMED.
 */
s32 func_0011F818(s32 a, s32 b) {
    __asm__ volatile("addiu $3, $0, 0x74\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", _InitSys);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F864);

/**
 * func_0011F868 = EE kernel syscall 0x74. SCE library syscall stub (see
 * func_0011AA20): load the syscall number into $v1 and trap. Used during DMA
 * channel setup (see func_0011F938) with a 2-word argument. Exact SDK name
 * UNCONFIRMED.
 */
s32 func_0011F868(s32 a, s32 b) {
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
 * func_0011AA20): load the syscall number into $v1 and trap. Used during DMA
 * channel setup (see func_0011F938) with a 2-word argument. Exact SDK name
 * UNCONFIRMED.
 */
s32 func_0011F8C0(s32 a, s32 b) {
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F938);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F9E4);

/* func_0011FA18: frameless tail call `j func_0011F130` in the original. Because
 * func_0011F130 is itself a framed non-leaf, ee-gcc 2.9 declines the sibling-call
 * optimisation here and emits a full jal + stack frame, so this shape can't match
 * from C. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FA18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", exit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FA48);

/**
 * func_0011FA50 = EE kernel syscall 0x74 (same primitive as func_0011F868).
 * SCE library syscall stub (see func_0011AA20): load the syscall number into
 * $v1 and trap. Called from the GS/DMA reset path (func_0011FAB8) with a
 * 2-word argument. Exact SDK name UNCONFIRMED.
 */
s32 func_0011FA50(s32 a, s32 b) {
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
 * $v1 and trap. Called in a loop from the GS/DMA reset path (func_0011FAB8).
 * Exact SDK name UNCONFIRMED.
 */
s32 func_0011FAA8(s32 a, s32 b) {
    __asm__ volatile("addiu $3, $0, 0x5B\n\tsyscall 0" ::: "$3", "memory");
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FAB8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FC68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00120354);

extern void (*D_00135D34)(void);

/**
 * Invoke the installed callback held in the global function pointer D_00135D34.
 */
void func_00120368(void) {
    D_00135D34();
}

extern s32 func_00115544(const char *a, const char *b);

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

/**
 * No-op stub (empty body; present as a registered/overridable hook).
 */
void func_001203C0(void) {
}

extern s32 func_00115F28(s32 size);

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

extern s32 (*D_00135D38)(void);

/**
 * Return the value produced by the installed callback D_00135D38 (a base
 * value/pointer queried by the +4 / +8 variants below).
 */
s32 func_00120420(void) {
    return D_00135D38();
}

/**
 * Return D_00135D38() + 8 (the base value from the callback, offset by 8 bytes).
 */
s32 func_00120448(void) {
    return D_00135D38() + 8;
}

extern s32 func_00120498(void);

/**
 * Install func_00120498 as the active callback D_00135D38 and invoke it once
 * (priming its lazily-initialised state).
 */
void func_00120470(void) {
    D_00135D38 = func_00120498;
    D_00135D38();
}

extern s32 D_00141800;
extern u8 D_001417F0[16];
extern u8 D_00141808;

/**
 * Lazily initialise and return the 16-byte singleton at D_001417F0. On first
 * call (guarded by the flag D_00141800) the block is zeroed and its field at
 * offset 4 is pointed at D_00141808. Always returns the block's address.
 */
s32 func_00120498(void) {
    if (!D_00141800) {
        D_00141800 = 1;
        memset(D_001417F0, 0, 0x10);
        *(u8 **)(D_001417F0 + 4) = &D_00141808;
    }
    return (s32)D_001417F0;
}

/**
 * Return D_00135D38() + 4 (the base value from the callback, offset by 4 bytes).
 */
s32 func_00120500(void) {
    return D_00135D38() + 4;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00120528);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00120810);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001208E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00120A30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00120AB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00120B38);

/**
 * No-op stub (empty body; present as a registered/overridable hook).
 */
void func_00120BC8(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00120BD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00120F00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001210E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001212C4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001212C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001213B4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001213B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00121450);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00121AB8);

/**
 * Frameless tail-call thunk: forward to func_00120368 (which dispatches the
 * installed handler D_00135D34). Takes and returns nothing. The original is a
 * bare `j func_00120368`; ee-gcc 2.9 reproduces the sibling call because both
 * the thunk and target are void(void) leaves with no argument/return shuffle.
 */
void func_00121B18(void) {
    func_00120368();
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00121B20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001220F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122630);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122760);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122800);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122B00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122DA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122F10);

extern s32 func_00122F10(FpParts *a, FpParts *b);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123078);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123130);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001231C8);

/**
 * Build an FpParts descriptor from explicit class/sign/exponent and a 64-bit
 * mantissa (8-byte aligned at offset 0x10) and recompose it into a double via
 * func_00122630, discarding the result.
 */
void func_00123268(s32 fpClass, s32 sign, s32 exponent, s64 mantissa) {
    FpParts parts;
    parts.fpClass = fpClass;
    parts.sign = sign;
    parts.exponent = exponent;
    parts.mantissa = mantissa;
    func_00122630(&parts);
}

extern void func_001234C0(s32 fpClass, s32 sign, s32 exponent, s32 mantissa);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001232EC);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001232F0);

/* func_00123400: decompose the IEEE-754 single-precision float at src[0] into a
 * classification record at out (out[1]=sign, out[0]=class {0=sNaN,1=qNaN,
 * 2=zero/subnormal,3=normal,4=inf}, out[3]=mantissa, out[2]=unbiased exponent),
 * returning the class. Behaviour fully understood (~68%) but pervasive register
 * allocation differs from the original; left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123400);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123490);

extern void func_001232F0(void *args);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001234F0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001235C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001236C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123930);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123978);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123A00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123B40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123C28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123D30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001240C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001242A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124414);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", WaitGsPathsIdle);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124540);

extern void func_0011A9A0(s32 id, void *handler, s32 obj);
extern void func_0011AC30(s32 obj);
extern void func_00124540(void);

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

extern s32 func_00124B88(s32 arg0);
extern s32 func_0011F5E0(void);
extern void func_0011F628(void);
extern s32 D_00141840;

/**
 * Install `handler` as the active interrupt handler in the global D_00141840.
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
    old = D_00141840;
    D_00141840 = handler;
    if (wasEnabled != 0) {
        func_0011F628();
    }
    return old;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124630);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001246D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124780);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124818);

/* func_001248B0: if the callback D_00141844 is installed and the suppression
 * flag D_001363A4 is clear, invoke the callback with the parameter D_00141848.
 * The body compiles byte-exact, but this is a splat mis-split: the per-function
 * .s (and the address-ordered unit listing) start the symbol 8 bytes early on a
 * trailing `addiu $29,$29,0x40; nop` epilogue fragment of the previous function
 * (func_001248F8 even references `func_001248B0 + 0x8` as the real entry). Can't
 * be matched at the unit level without a re-split. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001248B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001248F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124970);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124980);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124AF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124B88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124C28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124C98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceCdInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceCdDiskReady);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceCdMmode);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001253A4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001253A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", QueryCdStatusOverRpc);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125620);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceCdReadClock);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceGsResetGraph);

extern s32 D_00137E00;

/**
 * Accessor: return the address of the global D_00137E00.
 */
s32 *func_00125960(void) {
    return &D_00137E00;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012596C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125970);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125A10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", BuildGsDispEnv);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125D94);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125E54);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125E58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", BuildGsDrawEnvPacket);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00126104);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", WaitVblankGetField);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00126284);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012646C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", KickGifImageUpload);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012672C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00126DBC);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceDmaSyncChan);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00126ED0);

/**
 * Normalise a handle/id: if its top nibble (bits 31..28) equals 7, clear the
 * upper nibble and set bit 31 instead (i.e. remap tag 0x7 to 0x8). Otherwise
 * return arg0 unchanged.
 */
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

extern s32 D_00137E30[];

/**
 * Bounds-checked lookup into the 10-entry table D_00137E30. Returns
 * D_00137E30[arg0] for arg0 in [0,9], or 0 if arg0 is out of range.
 */
/* func_00126F38 is sceDmaGetChan - libdma channel-struct lookup (kept func_ name - matched). */
s32 func_00126F38(u32 arg0) {
    if (arg0 < 0xA) {
        return D_00137E30[arg0];
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", ResetDmacChannels);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127040);

/* func_00127218: 8 bytes of inter-function padding (a dead `sw $4,0($3); nop`)
 * between func_00127040 and the real func_00127220 — a splat mis-split, pinned
 * to size 0x8 in symbol_addrs so func_00127220 gets a clean .s. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127218);

extern u32 func_00126ED8(u32 arg0);
extern void sceDmaSyncChan(void *obj);

struct Obj127220 {
    /* 0x0 */  s32 chcr;
    u8 pad4[0x1C];
    /* 0x20 */ s32 qwc;
    u8 pad24[0xC];
    /* 0x30 */ u32 tadr;
};

/**
 * func_00127220 is sceDmaSend (chain mode) - kept func_ name, matched. obj is
 * the sceDmaChan register block (chcr at +0, qwc at +0x20, tadr at +0x30).
 * Converts the chain pointer via func_00126ED8 (DmaSprAddrToMadr), syncs the
 * channel, stores TADR (unless the channel reports 0xFFFFFFFF), zeroes QWC and
 * kicks with CHCR = (chcr & ~0xC) | 0x105 (chain mode, TTE, STR).
 */
void func_00127220(struct Obj127220 *obj, u32 arg1) {
    u32 handle = func_00126ED8(arg1);
    sceDmaSyncChan(obj);
    if (obj->tadr != 0xFFFFFFFF) {
        obj->tadr = handle;
    }
    obj->qwc = 0;
    obj->chcr = (obj->chcr & ~0xC) | 0x105;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127288);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001272A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127340);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127500);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127508);

extern s32 func_00127508(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern s32 D_00137E68;

/**
 * Allocate/acquire via func_00127508(arg0, arg1, arg2, 0x40). On failure (NULL
 * result) record error code 0xB in D_00137E68. Returns the func_00127508 result.
 */
s32 func_00127630(s32 arg0, s32 arg1, s32 arg2) {
    s32 result = func_00127508(arg0, arg1, arg2, 0x40);
    if (result == 0) {
        D_00137E68 = 0xB;
    }
    return result;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127668);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127720);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001277F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127888);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001279A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127B18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127B40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127B88);

extern s32 *D_00141B28;
extern s32 *D_00141B2C;
extern s32 *D_00141B30;

/**
 * Read fields from the structure at physical address arg0 (accessed through the
 * uncached mirror, arg0 | 0x20000000) and publish them through three optional
 * global out-pointers: p[0] -> *D_00141B28, p[1] -> *D_00141B2C, and the word at
 * p+0x90 -> *D_00141B30. Each store is skipped if its out-pointer is null.
 */
void func_00127C68(u32 arg0) {
    s32 *p = (s32 *)(arg0 | 0x20000000);
    if (D_00141B28) *D_00141B28 = p[0];
    if (D_00141B2C) *D_00141B2C = p[1];
    if (D_00141B30) *D_00141B30 = *(s32 *)((char *)p + 0x90);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127CC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127E40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127F90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128068);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128180);

extern s32 D_00143108;
extern s32 D_00143180;

/**
 * Initialise the D_00143180 subsystem by calling func_0011D620 with the config
 * block at &D_00143108, mode 0x80000963, two 0x400-sized buffers both pointing
 * at &D_00143180, and zeroed trailing arguments; returns the resulting handle
 * stored in D_00143180.
 */
s32 func_00128250(void) {
    func_0011D620(&D_00143108, 0x80000963, 0, &D_00143180, 0x400,
                  &D_00143180, 0x400, 0, 0);
    return D_00143180;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceDbcInit);

/* func_00128440(arg0): open the D_00143180 subsystem in mode 0x80000904 with
 * arg0 stored at (&D_00143180)[1], via func_0011D620; on failure log D_0013B868
 * (func_00128898) and return 0, else return the handle D_00143180. ~70% — the
 * original parks %hi(D_00143180) in callee-saved $16 and reuses it for the final
 * `lw $2,%lo(D_00143180)($16)`, and selects a `bgez` over the `bgezl` ee-gcc
 * emits for the early-return shape. A reg-alloc + branch-form mismatch this cc1
 * won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128440);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001284B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128578);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001286C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001286C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001287A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128898);

extern s32 D_00137E80;
extern u8 D_00143640[];

/**
 * Reset the 16-entry table at D_00143640 (each entry is 0x330 bytes): zero the
 * first three words of every entry across the 0x3300-byte span, set the
 * initialised flag D_00137E80 to 1, and return 1.
 */
s32 func_001288C0(void) {
    s32 *entry;
    s32 *end;
    D_00137E80 = 1;
    entry = (s32 *)D_00143640;
    end = (s32 *)(D_00143640 + 0x3300);
    do {
        entry[0] = 0;
        entry[1] = 0;
        entry[2] = 0;
        entry = (s32 *)((u8 *)entry + 0x330);
    } while ((s32)entry < (s32)end);
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", sceDbcPortOpen);

/* func_00128A48: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real function begins at func_00128A50. Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128A48);

/* func_00128A50: real function recovered from the splat mis-split above (indexes
 * the 0x330-stride table D_00143640, dispatches to func_00128D58/DB0/E98 + a
 * memcpy). Boundary now correct; body not yet decompiled. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128A50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128B28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128C18);

/* func_00128D58(index): acquire a resource via func_00128578(index); on success
 * (non-negative handle) record it at entry+0x8 and set the active flag at
 * entry+0x4 in the 0x330-stride D_00143640 table. ~87% — the original keeps two
 * separate base registers for the same entry pointer ($5 and a copied $3) and
 * stores result-then-flag; ee-gcc uses one base and reschedules the pair. A
 * scheduling/reg-alloc shape this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128D58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128DB0);

/* func_00128E18(index): lazily refresh the two-word state cache D_00137E88 from
 * table entry `index` (stride 0x330 in D_00143640; object pointer at +0xC).
 * Returns 0 when obj[0x7C] is 0 or the cache already holds the (obj[0x7C],
 * (obj+0x80)[0x7C]) pair; otherwise updates the cache and returns 1. Behaviour
 * recovered, but the original spills obj/next to a stack frame and ee-gcc keeps
 * them in registers here, giving a different instruction shape. Left as
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128E18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128E98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128F48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128FD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001290BC);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00129120);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00129160);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001291A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00129218);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001292C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00129368);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00129410);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00129450);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001296A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00129DA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012A1C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012A3E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012A460);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012A4F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012A5B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012A680);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012A730);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012A7E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012A8E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012A9E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012AA80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012AB30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012AC10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012ACF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012ADD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012AEA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012AFC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012B0D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012B138);

/**
 * Set bit 23 of the hardware register at 0x10002010 (IPU_CTRL) to the low bit of
 * arg0, preserving all other bits (read-modify-write with mask 0xFF7FFFFF).
 */
void func_0012B198(s32 arg0) {
    u32 *reg = (u32 *)0x10002010;
    *reg = (*reg & 0xFF7FFFFF) | (arg0 << 23);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012B1C0);

/* func_0012B3C0(arg0): thin wrapper that calls func_0012C508(arg0, 3) and
 * returns. The original keeps a real frame + jal (no sibling-call), but ee-gcc
 * sibling-call-optimizes the tail call to `j func_0012C508`; that codegen-shape
 * mismatch isn't expressible in clean source. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012B3C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012B3E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012B568);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012B678);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012B780);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012B8B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012BAA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012BB60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012C008);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012C090);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012C230);

extern s32 D_00137F10[];

/**
 * Issue an IPU command: write `cmd` to the IPU_CMD hardware register
 * (0x10002000), then look up D_00137F10[cmd >> 28] (indexed by the command's
 * top nibble = the IPU opcode) and cache it in arg0->field_0x818.
 */
void func_0012C380(s32 *arg0, u32 cmd) {
    *(volatile u32 *)0x10002000 = cmd;
    *(s32 *)((u8 *)arg0 + 0x818) = D_00137F10[cmd >> 28];
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012C3B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012C458);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012C508);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012C680);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012C788);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012C878);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012C9C8);

extern s32 func_0012C788(s32 *arg0, s32 arg1);
extern s32 func_0012C878(s32 *arg0, s32 arg1);
extern s32 func_0012CFA0(s32 *arg0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012CAB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012CBC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012CC88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012CDB0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012CFE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012D060);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012D100);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012D1C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012D2C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012D350);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012D420);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012D4B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012D768);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012D808);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012DAC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012DC50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012DD60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012DF18);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012E10C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012E110);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012E238);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012E378);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012E538);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012E608);

/* func_0012E890(arg0, arg1, arg2, arg3): initialise the record at arg0 (limit
 * arg1 at field_0x8/0xC, end arg2+arg3 at field_0x24, span arg3 at field_0x28,
 * start arg2 at field_0x20; zero field_0x0..0x4, 0x10, 0x18..0x1C) then
 * tail-call func_0012E8E8(arg0, 0, arg2, arg3). ~75% — ee-gcc schedules the
 * field stores differently around the sibling call (the original interleaves
 * the start-store into the tail-call delay slot), a codegen shape not
 * expressible in source. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012E890);

/**
 * Extract the top arg1 bits of the 64-bit value at *arg0: returns
 * (s32)(*arg0 >> (64 - arg1)) — i.e. the most-significant arg1 bits, right
 * aligned. (Bitstream/MSB-first reader helper.)
 */
s32 func_0012E8C8(u64 *arg0, s32 arg1) {
    u64 val = *arg0;
    return (s32)(val >> (0x40 - arg1));
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012E8E8);

extern void func_0012E8E8(u64 *arg0, s32 arg1);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012EA18);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012EA9C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012EAA0);

/* func_0012EB28: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real function begins at func_0012EB30. Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012EB28);

/* func_0012EB30: real function recovered from the splat mis-split above (large
 * 0x150-frame routine iterating a 0x18-stride record list and dispatching via an
 * indirect call). Boundary now correct; body not yet decompiled. Left as
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012EB30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012EE28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012EF20);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F0E0);

extern void func_00130E88(void);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F738);

/**
 * Stub predicate that always returns 1 (a registered callback whose default
 * answer is "true"/success).
 */
s32 func_0012F940(void) {
    return 1;
}

/* func_0012F948: 8 bytes of inter-function padding (a dead `sll $6,$6,4; nop`)
 * between func_0012F940 and the real func_0012F950 — a splat mis-split, pinned
 * to size 0x8 in symbol_addrs so func_0012F950 gets a clean .s. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F948);

/* func_0012F950(obj, arg1, arg2): seed the display/DMA sub-object obj->field_0x40
 * (set fB0=1, fD8=(arg1 & 0x0FFFFFFF) | 0x20000000, fE4=arg2, fDC=fE0=0) then run
 * func_0012FBF0(obj). The body reproduces every field write, but ee-gcc sibling-
 * call-optimises the trailing void call to `j func_0012FBF0` where the original
 * keeps a stack frame (`sd $31`/`jal`) — the inverse-sibling-call form this cc1
 * won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F950);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F998);

/**
 * Predicate: follow arg0->field_0x40 (arg0[0x10]) to a sub-object and return 1
 * if that object's field_0x4 (base[1]) is zero, else 0.
 */
s32 func_0012F9B8(s32 *arg0) {
    s32 *base = (s32 *)arg0[0x10];
    return base[1] == 0;
}

extern void func_00130178(s32 *arg0);
extern void func_00130088(s32 *arg0);

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

/* func_0012FA70(arg0, index, arg2, arg3): in the entry table at arg0->field_0x40
 * (8-byte stride records), write arg3 into record[index]+0x10, return the old
 * value of record[index]+0xC and overwrite it with arg2. ~74%; the original
 * keeps the table base live and computes both member addresses before storing,
 * a scheduling shape ee-gcc won't reproduce here. Re-probed 2026-06-12 with
 * two separately-formed record pointers (base+idx*8 and (base+0xC)+idx*8) -
 * still 74.44%, the address-formation schedule does not budge. Left as
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FA70);

/* func_0012FA98(arg0, arg1): if arg0 and its table arg0->field_0x40 are non-null,
 * fetch the destructor at table[*arg1*2 + 3] and, if set, call
 * dtor(arg0, arg1, table[*arg1*2 + 4]); return its result or 0. The natural body
 * reaches 92% — but the original keeps `ret` in $7 (a3) where ee-gcc allocates
 * a2, and emits a plain `beqz` on the callback test where ee-gcc picks the
 * branch-likely `beqzl` (annulling its delay slot). Both are scheduling/reg-
 * alloc forms this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FA98);

extern s32 func_0012FA98(s32 *obj, s32 *req);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FB10);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FB80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FBF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FD60);

/* func_0012FE78: dispatch on the state word (offset 0x174) of arg0's sub-object
 * (arg0->field_0x40) — when it equals 3 hand off to func_0012FD60, otherwise to
 * func_0012FEC0; returns the chosen handler's result. Body
 * `if (((s32*)arg0[0x10])[0x5D] != 3) return func_0012FEC0(arg0); return
 * func_0012FD60(arg0);` reaches 97% — every instruction matches but the original
 * parks arg0 in $7 (a3) and the state in $2, while ee-gcc allocates a1/a0; a
 * register-allocation form this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FE78);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FEC0);

extern void func_00130098(s32 *obj);

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

/* func_00130098(obj): advance/finalise a pending transfer and clear the
 * in-progress flag (field_0x120). If a request is queued (field_0x120 != 0)
 * dispatch via func_00130288(obj, &D_0013BDC8); else by mode field_0x174 finish
 * via func_0012DC50(obj, field_0x1BC, field_0x118 - 1) (mode 3) or
 * func_0012DD60(obj, field_0x1CC, field_0x1DC). ~85% — the original tests the
 * mode with a plain `bne` and hoists `count-1` into its delay slot, but ee-gcc
 * picks the branch-likely `bnel` and fills the slot with the next load. A
 * branch-form/scheduling shape this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130098);

/* func_00130118: initialise subsystem 1 (func_0012B198(1)), then program the
 * four hardware DMA/GIF register pointers into arg0
 * (field_0x590=0x70000000, 0x594=0x70001800, 0x6D0=0x70001B00, 0x6D4=0x70003300)
 * and clear the busy flag at field_0x810. Body matches 90% — but the original
 * parks the use-once 0x70000000 in the callee-saved $17 (and so reserves a 0x30
 * frame saving $16/$17), whereas ee-gcc at -O2 keeps it in a caller-saved temp
 * and only saves $16 (0x20 frame). A register-allocation form this cc1 won't
 * reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130118);

extern s32 func_00130DB8(s32 mode, s32 arg1);

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

/* func_00130240(arg0): dispatch arg0 through Kprintf against the global
 * table D_0013BDE8 — the original is a frameless tail call (`j Kprintf`).
 * ee-gcc 2.9 does not sibling-call-optimise this, so it emits jal + a stack
 * frame and cannot match from C. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130240);

extern void func_00115DA8(void *buf);
extern void func_00130288(s32 arg0, void *buf);

/**
 * Build a temporary 256-byte descriptor on the stack via func_00115DA8, then
 * dispatch it for arg0 through func_00130288(arg0, buf).
 */
void func_00130250(s32 arg0) {
    u8 buf[256];
    func_00115DA8(buf);
    func_00130288(arg0, buf);
}

extern s32 func_0012FA98(s32 *obj, s32 *req);
extern void func_00130240(void *buf);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130300);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130428);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001306D0);

/* func_001307B0(obj, cmd, madr): restart the GIF/PATH3 DMA pipeline — tear down
 * sub-object 2 (func_0012FA98), flush (func_0012C3B0) and reset the GIF mode
 * register (0x10002000=0); then with interrupts disabled program channel
 * 0x1000B400 (MADR 0x1000B410 = madr & 0x0FFFFFFF, QWC 0x1000B420 = 4, CHCR
 * 0x1000B400 = 0x101), restoring interrupts if on; finally issue IPU command
 * `cmd` (func_0012C380), flush again and tear down sub-object 3. 99.82% — every
 * instruction matches except the frame size: the original reserves a 0x60 frame
 * (saves parked at +0x20..+0x50) where ee-gcc only needs 0x50. A frame-size-only
 * constant mismatch this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001307B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130890);

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

/* func_00130A50/A60/A70/A80(arg0): frameless tail-call thunks forwarding arg0 to
 * func_00130288 with table D_0013BE58 / D_0013BE88 / D_0013BEA0 / D_0013BED8
 * respectively (original `j func_00130288`). ee-gcc 2.9 won't sibling-call them
 * (emits jal + frame), so they can't match from C. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130A50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130A60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130A70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130A80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130A8C);

/* func_00130AA0(arg0): frameless tail call to func_00130C68 with the sub-object
 * at arg0->field_0x40 + 0x4C (original `j func_00130C68`). ee-gcc 2.9 won't
 * sibling-call it (emits jal + frame). Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130AA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130AAC);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130B80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130C68);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130E88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001310C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001313C4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", s_isnan);

/**
 * Pass the 64-bit value at arg0 + 0x8 as both arguments to func_00123028,
 * discard its result, and return 0.
 */
s32 func_00131400(s64 *arg0) {
    s64 v = arg0[1];
    func_00123028(v, v);
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131424);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0013153C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131540);

extern void func_00131540(void);
extern s8 D_00138158[];

/**
 * Lazily initialise the global block D_00138158 (calling func_00131540() the
 * first time, detected by its leading byte being 0), then return 1 if byte 4 of
 * the block equals 0x54 ('T'), else 0 — a region/territory check.
 */
s32 func_001315E0(void) {
    if (D_00138158[0] == 0) {
        func_00131540();
    }
    return D_00138158[4] == 0x54;
}

/* func_00131620: 8 bytes of inter-function padding (a dead `sdr $3,0($7); nop`)
 * between func_001315E0 and the real func_00131628 — a splat mis-split, pinned
 * to size 0x8 in symbol_addrs so func_00131628 gets a clean .s. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131620);

extern u8 D_00138152;

/**
 * Return a 2-bit status code. For the 'T' (0x54) territory variant
 * (func_001315E0() true) this is the cached byte D_00138152; otherwise sample
 * the pad/controller status word (func_0011ACD0) and return bits 1..2 of it.
 */
s32 func_00131628(void) {
    u32 status;
    if (func_001315E0()) {
        return D_00138152;
    }
    func_0011ACD0((s32 *)&status);
    return (status >> 1) & 3;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131668);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131670);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001316C0);

/* func_001316C8: controller/port status getter — on territory 'T'
 * (func_001315E0()) return the cached byte D_00138156; else sample the pad
 * (func_0011ACD0), return 0 if the 3-bit port field (bits 13..15) is zero, else
 * read extended status (func_0011AF30) and return bit 4 of its low byte. Every
 * instruction matches at 98.85% except the %hi temp register for D_00138156: the
 * original reuses $2 (`lui $2; lbu $2,%lo($2)`) while ee-gcc splits the lui into
 * $3. A reg-alloc form this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001316C8);

/* func_00131730: binary byte (0..99) -> packed BCD, n + (n/10)*6 masked to a
 * byte (inverse of func_00131760), e.g. 59 -> 0x59. 99.55% — the ONLY
 * difference is the div-by-zero trap: GNU as encodes ee-gcc's check as
 * `break 7` but the original is `break 0, 7` (different code field). This is an
 * assembler-encoding mismatch (like the move->daddu one), not a source issue.
 * Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131730);

/* func_00131760: packed-BCD byte -> binary, n - (n>>4)*6 masked to a byte
 * (e.g. 0x59 -> 59). Decompiles to ~87%; the only diff is the multiply form:
 * the original emits 2-operand `mult $0,rs,rt` + `mflo`, but ee-gcc lowers `*`
 * to the 3-operand R5900 `mult rd,rs,rt`. That is a compiler-flag/codegen
 * choice, not expressible in source, so it stays INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131760);

extern u8 func_00131760(u8 packed);

/**
 * Convert the packed-BCD time fields of the record at arg0 to binary in place:
 * apply func_00131760 to the bytes at offsets 7,6,5,3,2,1 (skipping offset 4),
 * each replaced by its decoded value.
 */
void func_00131780(u8 *arg0) {
    arg0[7] = func_00131760(arg0[7]);
    arg0[6] = func_00131760(arg0[6]);
    arg0[5] = func_00131760(arg0[5]);
    arg0[3] = func_00131760(arg0[3]);
    arg0[2] = func_00131760(arg0[2]);
    arg0[1] = func_00131760(arg0[1]);
}

extern u8 func_00131730(u8 binary);

/**
 * Convert the binary time fields of the record at arg0 to packed BCD in place
 * (the inverse of func_00131780): apply func_00131730 to the bytes at offsets
 * 7,6,5,3,2,1 (skipping offset 4), each replaced by its encoded value.
 */
void func_001317E8(u8 *arg0) {
    arg0[7] = func_00131730(arg0[7]);
    arg0[6] = func_00131730(arg0[6]);
    arg0[5] = func_00131730(arg0[5]);
    arg0[3] = func_00131730(arg0[3]);
    arg0[2] = func_00131730(arg0[2]);
    arg0[1] = func_00131730(arg0[1]);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131850);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131908);

extern void func_00131850(u8 *arg0);

/**
 * Advance the counter byte at arg0+0x3: increment it, and when it wraps to
 * 0x18 reset it to 0 and run func_00131850(arg0) to roll over to the next unit.
 */
void func_001319B0(u8 *arg0) {
    s32 next = arg0[3] + 1;
    arg0[3] = next;
    if ((next & 0xFF) == 0x18) {
        arg0[3] = 0;
        func_00131850(arg0);
    }
}

extern void func_00131908(u8 *arg0);

/**
 * Tick down the cooldown byte at arg0+0x3: if non-zero, just decrement it;
 * otherwise reload it to 0x17 and run func_00131908(arg0) to advance state.
 */
void func_001319E0(u8 *arg0) {
    u8 timer = arg0[3];
    if (timer != 0) {
        arg0[3] = timer - 1;
    } else {
        arg0[3] = 0x17;
        func_00131908(arg0);
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131A08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131A98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", _start);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131CA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131CB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", InstallLoadedOverlay);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", main);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", snd_Pump);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132210);

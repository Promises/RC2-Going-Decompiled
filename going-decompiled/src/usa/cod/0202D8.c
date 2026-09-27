/*
 * cod/0202D8 (0x120358..0x121AB7): the part of the old cod/015180 unit between
 * the libgcc.a members _divdi3.o and _muldi3.o, which the build links from GCC's
 * own source (going-decompiled/libgcc/, RULING #8206; carve: task #879). It
 * holds libgcc's _eh.o, _fixunsdfdi.o, _floatdidf.o and _moddi3.o as the ROM's
 * asm plus this project's own C, which is decompilation, not GCC source text.
 */
#include "common.h"

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_00120358);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_00120528);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_00120810);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_001208E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_00120A30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_00120AB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_00120B38);

/**
 * No-op stub (empty body; present as a registered/overridable hook).
 */
void func_00120BC8(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_00120BD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_00120F00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_001210E0);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_001212C4);

/**
 * func_001212C8 = truncate the non-negative double whose bits are `x` to a
 * 64-bit integer, i.e. soft-float __fixunsdfdi for x >= 0 (negative x returns 0).
 * Splits the value into a high 32-bit limb hi = (u32)(x * 2^-32, func_00122B00
 * then func_001231C8) and a low part: it forms hi<<32 back into a double through
 * the inlined unsigned-64 -> double conversion (func_001213B8 of the limb, or of
 * the halved limb then doubled via func_00122A40 when the top bit is set),
 * subtracts that from x (func_00122A98) to get the residual double, truncates the
 * residual to an unsigned 32-bit limb (func_001231C8) and adds it back onto
 * hi<<32 (subtracting when the residual went slightly negative). Returns
 * hi*2^32 +/- low. The 2^-32 scale (0x3DF0000000000000) is cheap so it builds
 * inline (ori+dsll32), avoiding the constant-pool/li.d wall of func_0011C090.
 *
 * NEAR-MATCH WALL (78.59% via objdiff). The faithful C below is functionally
 * exact but loses byte-equality the same way as its soft-float siblings
 * (func_001213B8 85%, func_001231C8 68%): ee-gcc -O2 -G0 (1) keeps the double
 * constant 0.0 used by the residual compare AND the residual negate in a
 * callee-saved register ($18) across the intervening func_00123028 call (3 saved
 * regs vs the 2 a literal 0 / $0 yields), and (2) lays out the inlined
 * unsigned-64->double `if (limb<0)` block (func_001213B8 + func_00122A40 self-
 * add) with different branch placement / register threading than clean C emits.
 * Neither is steerable from C. Seedable (double bits -> u64): now that its
 * multiply dependency func_00122B00 ships a #else, it ships a faithful portable
 * TARGET_NATIVE #else, cmp-oracle'd asm-vs-C bit-identical on the real R5900. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_001212C8);
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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_001213B4);

extern s64 func_00123078(s32 x);
extern s64 func_00122B00(s64 a, s64 b);
extern s64 func_00122A40(s64 a, s64 b);

/**
 * func_001213B8 = convert a 64-bit signed integer to a double (packed bits),
 * i.e. soft-float __floatdidf. The high 32 bits are converted as a signed int
 * and scaled by 2^32 (two 65536.0 multiplies); the low 32 bits are converted as
 * a signed int and, when negative, biased by +2^32 so they contribute as an
 * unsigned 32-bit limb. The partial results are summed:
 * result = (double)hi * 2^32 + (unsigned)lo.
 *
 * NEAR-MISS WALL (85.92% via objdiff). The body is functionally faithful and
 * compiles to a byte-identical instruction stream EXCEPT for one ee-gcc -O2 -G0
 * codegen quirk: the original keeps the 65536.0 multiplier constant
 * (0x40F0000000000000) live in the callee-saved register $17 across both
 * func_00122B00 calls (then reuses $17 for the high partial), whereas ee-gcc
 * here rematerialises the 2-instruction constant before each multiply rather
 * than allocating a callee-saved register for it (a reload/rematerialisation
 * policy decision not controllable from C). Shipped as a portable TARGET_NATIVE
 * #else; verification is routed to tester-EE (the #else calls sibling soft-float
 * #else bodies, so it is not standalone cmp-oracle'able here).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", func_001213B8);
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
 * __moddi3 = libgcc `__moddi3` (signed 64-bit modulo, a % b). ee-gcc inlines
 * libgcc2.c's signed wrapper around `__udivmoddi4`: it takes the magnitudes of
 * both operands (the bgez/negu sign-strip sequences at entry), runs the unsigned
 * long-division core (count_leading_zeros via the 256-byte `__clz_tab` D_0013AD58,
 * then 16-bit-digit `udiv_qrnnd` with `divu`/`break 0,7`), captures the remainder
 * through the inlined `&w` slot (sp+0), and gives it the sign of the dividend a.
 *
 * NOT GAME CODE: compiler runtime emitted by ee-gcc itself; not reconstructable
 * as clean hand C that matches byte-exact, so the MATCHING arm stays INCLUDE_ASM
 * (links verbatim). The portable #else is the faithful behaviour (`a % b`),
 * cmp-oracle'd asm-vs-C bit-identical on the real R5900. Excluded domains (the
 * asm traps `break 0,7` / overflow): b == 0 and INT64_MIN / -1. Paired siblings:
 * __divdi3 = `__divdi3`, __udivdi3 = `__udivdi3`, __umoddi3 =
 * `__umoddi3`. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", __moddi3);
#else
s64 __moddi3(s64 a, s64 b) {
    return a % b;
}
#endif

/*
 * __muldi3 (libgcc `__muldi3`, 0x121AB8) is not in this unit: the EE build links
 * it from GCC's own libgcc2.c as the libgcc.a member _muldi3.o
 * (going-decompiled/libgcc/, RULING #8206), placed between this unit and
 * cod/021A98. The portable build keeps the behavioural equivalent: the low 64
 * bits of the product, a * b.
 */
#ifdef TARGET_NATIVE
s64 __muldi3(s64 a, s64 b) {
    return a * b;
}
#endif

/*
 * cod/0202D8 (0x120358..0x1212C7): the part of the old cod/015180 unit between
 * the libgcc.a members _divdi3.o and _fixunsdfdi.o, which the build links from
 * GCC's own source (going-decompiled/libgcc/, RULING #8206; carve: tasks #879,
 * #893). It holds libgcc's _eh.o as the ROM's asm plus this project's own C,
 * which is decompilation, not GCC source text. The TARGET_NATIVE build also
 * keeps here the portable C for _fixunsdfdi.o and _floatdidf.o, which the EE
 * build links from the library.
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

/**
 * Compare the two strings arg0 and arg1 with strcmp (0x115544, linked from
 * newlib's r5900 strcmp.S since task #1884; the ROM's jal target is the same
 * address under its old name func_00115544); return arg2 when they are equal,
 * otherwise 0.
 */
s32 func_00120390(const char *arg0, const char *arg1, s32 arg2) {
    s32 result = arg2;
    if (strcmp(arg0, arg1) != 0) {
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", __sjthrow);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", old_find_exception_handler);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", find_exception_handler);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", get_reg_addr);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", copy_reg);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", next_stack_level);

/**
 * No-op stub (empty body; present as a registered/overridable hook).
 */
void func_00120BC8(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", throw_helper);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", __throw);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0202D8", __rethrow);

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
 * inline (ori+dsll32).
 *
 * The EE build does not compile this: 0x1212C8 is libgcc's __fixunsdfdi, and the
 * build links it from GCC's own libgcc2.c as the libgcc.a member _fixunsdfdi.o
 * (going-decompiled/libgcc/, RULING #8206, task #893), byte-exact. This is the
 * project's own portable C for the TARGET_NATIVE build, cmp-oracle'd asm-vs-C
 * bit-identical on the real R5900. */
#ifdef TARGET_NATIVE
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

extern s64 func_00123078(s32 x);

/**
 * func_001213B8 = convert a 64-bit signed integer to a double (packed bits),
 * i.e. soft-float __floatdidf. The high 32 bits are converted as a signed int
 * and scaled by 2^32 (two 65536.0 multiplies); the low 32 bits are converted as
 * a signed int and, when negative, biased by +2^32 so they contribute as an
 * unsigned 32-bit limb. The partial results are summed:
 * result = (double)hi * 2^32 + (unsigned)lo.
 *
 * The EE build does not compile this: 0x1213B8 is libgcc's __floatdidf, and the
 * build links it from GCC's own libgcc2.c as the libgcc.a member _floatdidf.o
 * (going-decompiled/libgcc/, RULING #8206, task #893), byte-exact. This is the
 * project's own portable C for the TARGET_NATIVE build; its verification is
 * routed to tester-EE (it calls sibling soft-float #else bodies, so it is not
 * standalone cmp-oracle'able here).
 */
#ifdef TARGET_NATIVE
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

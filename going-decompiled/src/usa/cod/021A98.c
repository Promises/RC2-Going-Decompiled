/*
 * cod/021A98 (0x121B18..0x13221F): the part of the old cod/015180 unit after the
 * libgcc.a member _muldi3.o, which the build links from GCC's own source
 * (going-decompiled/libgcc/, RULING #8206; carve: task #879). It starts with
 * libgcc's _pure.o, _udivdi3.o, _umoddi3.o, dp-bit.o, fp-bit.o and frame.o as
 * the ROM's asm plus this project's own C, then the libsn/SDK code that follows.
 */
#include "common.h"

/**
 * Frameless tail-call thunk: forward to func_00120368 (which dispatches the
 * installed handler D_00135D34). Takes and returns nothing. The original is a
 * bare `j func_00120368`; ee-gcc 2.9 reproduces the sibling call because both
 * the thunk and target are void(void) leaves with no argument/return shuffle.
 */
void func_00121B18(void) {
    func_00120368();
}

/**
 * __udivdi3 = libgcc `__udivdi3` (unsigned 64-bit division, u / v). It inlines
 * libgcc2.c's `__udivmoddi4` long division directly (no sign handling at entry —
 * straight to the sltu/divu unsigned core): count_leading_zeros normalisation via
 * the 256-byte `__clz_tab` D_0013AE58, then schoolbook 16-bit-digit division using
 * the host `divu` via the longlong.h `udiv_qrnnd` macro (hence the `break 0,7`
 * divide-by-zero traps). The quotient is returned; the remainder is discarded.
 *
 * NOT GAME CODE: compiler runtime emitted by ee-gcc itself; not reconstructable
 * as clean hand C that matches byte-exact, so the MATCHING arm stays INCLUDE_ASM
 * (links verbatim). The portable #else is the faithful behaviour (`u / v`),
 * cmp-oracle'd asm-vs-C bit-identical on the real R5900. Excluded domain (the asm
 * traps `break 0,7`): v == 0 — the caller (func_0011C1F8's %u, always v == 10)
 * never hits it. Paired sibling: __umoddi3 = `__umoddi3` (u % v). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", __udivdi3);
#else
u64 __udivdi3(u64 u, u64 v) {
    return u / v;
}
#endif

/**
 * __umoddi3 = libgcc `__umoddi3` (unsigned 64-bit modulo, u % v). It inlines
 * libgcc2.c's `__udivmoddi4` long-division: a `count_leading_zeros` normalisation
 * driven by the 256-byte `__clz_tab` (D_0013AF58 - the classic 0,1,2,2,3,3,3,3,...
 * leading-bit table), then schoolbook 16-bit-digit division using the host `divu`
 * via the longlong.h `udiv_qrnnd` macro (hence the `break 0,7` divide-by-zero
 * traps). The remainder slot pointer is the inlined `&w` (sp+0); the quotient is
 * computed but discarded, and the remainder is returned. Paired helpers in this
 * unit: __udivdi3 = `__udivdi3` (u / v), __divdi3 = `__divdi3` (signed
 * a / b), __moddi3 = `__moddi3` (signed a % b). The format core
 * func_0011C1F8 calls these to render %u/%d.
 *
 * NOT GAME CODE / NOT a format sub-engine: this is compiler runtime emitted by
 * ee-gcc itself (it is the body the EE longlong.h macros + libgcc2.c expand to).
 * It is not reconstructable as clean hand-written C and matched byte-exact - the
 * `udiv_qrnnd`/`count_leading_zeros` macros are arch-specific compiler internals
 * (the `divu`+`break` digit step, the dsll32/dsra32 64->32 splits). The MATCHING
 * arm stays INCLUDE_ASM; it links verbatim like the other SDK/runtime routines in
 * this unit. The portable #else is the faithful behaviour (`u % v`), cmp-oracle'd
 * asm-vs-C bit-identical on the real R5900. Excluded domain (the asm traps
 * `break 0,7`): v == 0 — the %u caller always passes v == 10.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", __umoddi3);
#else
u64 __umoddi3(u64 u, u64 v) {
    return u % v;
}
#endif

/* Decomposed IEEE-754 double produced by func_00122760: a class tag, sign,
 * unbiased exponent and the explicit mantissa. */
typedef struct {
    s32 fpClass;   /* 0x00: classification tag (0=sNaN,1=qNaN,2=zero/subnormal,3=normal,4=inf) */
    s32 sign;      /* 0x04: sign bit */
    s32 exponent;  /* 0x08: unbiased exponent (bias 0x3FF) */
    s32 pad;       /* 0x0C */
    s64 mantissa;  /* 0x10: explicit mantissa (normal: frac<<8 | 1<<60) */
} FpParts;

/**
 * func_00122630 = recompose an FpParts descriptor into a packed IEEE-754 double.
 * NaN -> exp 0x7FF with the quiet bit forced; inf/zero -> exp 0x7FF / 0 with
 * mantissa 0; normal -> rebias exp (+0x3FF), round-to-nearest-even on the low 8
 * mantissa bits (with underflow denormal-shift and overflow-to-inf clamps),
 * then pack sign|exp|mantissa. Returns the 64-bit double bit pattern.
 *
 * NEAR-MISS WALL (59.12% via objdiff). The original threads the result through an
 * UNINITIALIZED scratch register that is fully masked away (a quirk ee-gcc -O2
 * -G0 will not reproduce from clean C), plus branch-likely round/normalise
 * fillers. Seedable (FpParts->bits): shipped as a cmp-oracle'd portable #else
 * (asm-vs-C proven bit-identical on real R5900 by run_cmp_015180_iso.sh).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00122630);
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
 * func_00122760 = decompose the IEEE-754 double *value into FpParts at out.
 * Extracts sign, the 11-bit exponent field and the 52-bit fraction. Classifies:
 * exp==0 -> class 2 (zero/subnormal); exp==0x7FF -> class 4 (inf, frac==0) or
 * NaN (frac!=0, class 1 quiet / 0 signalling) storing the raw 52-bit fraction;
 * otherwise class 3 (normal) with mantissa = (frac<<8) | (1<<60) and the
 * exponent rebiased by -0x3FF.
 *
 * NEAR-MISS WALL (93.54% via objdiff). The C below is functionally faithful but
 * ee-gcc -O2 -G0 makes a different register/scheduling choice in the prologue:
 * the original computes the sign into the freed value-ptr reg ($4) before
 * overwriting v with v>>52 (so exp lands in $4, v>>52 in $2 and the 0x7FF
 * comparison constant is preloaded into the exp==0 branch delay slot), whereas
 * ee-gcc schedules v>>52 first (sign->$2, exp->$4) and reloads 0x7FF later.
 * Neither the sign/exp register swap nor the constant-preload is controllable
 * from C. Seedable (bits->FpParts): shipped as a cmp-oracle'd portable #else
 * (asm-vs-C proven bit-identical on real R5900 by run_cmp_015180_iso.sh).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00122760);
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

/* The soft-float canonical-NaN descriptor. Recovered from the ROM image at
 * vaddr 0x00141810: a 0x28-byte block that is ALL ZEROES, i.e. a zero-filled
 * FpParts { fpClass=0, sign=0, exponent=0, mantissa=0 }. The add/multiply/divide
 * cores return &D_00141810 on their invalid-operation result paths (inf-inf,
 * 0*inf, inf/inf, 0/0); func_00122630 then recomposes it: fpClass 0 (<2) forces
 * the quiet bit and exponent 0x7FF, so &D_00141810 packs to the canonical quiet
 * NaN 0x7FF8000000000000. Defined in the unit's data (here only declared so the
 * portable #else cores can take its address). */
extern FpParts D_00141810;

/* func_00122800 = software double-precision ADD of two decomposed operands
 * (FpParts a + b -> out), the core of func_00122A40/func_00122A98. NaN
 * propagates (returns a or b); inf+inf of opposite sign returns the global NaN
 * descriptor &D_00141810; zero/inf shortcuts copy the surviving operand;
 * otherwise it aligns the smaller mantissa (right-shift by the exponent delta
 * with a sticky bit), adds (same sign) or subtracts (differing sign), then
 * renormalises the 61-bit result (shift-up loop on cancellation, one-step
 * shift-down on carry) and writes class 3 / exponent / sign / mantissa to out.
 *
 * NEAR-MISS WALL (~144 instrs with two sticky-shift loops + branch-likely
 * fillers; no byte-match from C). Seedable (FpParts pair -> FpParts): ships a
 * faithful portable TARGET_NATIVE #else, cmp-oracle'd asm-vs-C bit-identical on
 * the real R5900 (run_cmp_015180_iso.sh, incl. the inf-inf NaN path that reads
 * D_00141810). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00122800);
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
        return &D_00141810;              /* inf + (-inf) = NaN */
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

/* func_00122B00 = software double-precision MULTIPLY of two packed doubles
 * (a * b -> packed double), the multiply sibling of the add-core func_00122800
 * and divide-core func_00122DA8. Decomposes both operands with func_00122760,
 * then: NaN propagates (recomposes the NaN operand with the product sign); the
 * result sign is signA^signB; 0*inf returns the global NaN descriptor
 * &D_00141810; zero/inf shortcuts copy the appropriate zero/inf result;
 * otherwise (both normal) it forms the 122-bit product of the two 61-bit
 * mantissas from four 64x64 partial products (__muldi3/__muldi3), keeps the
 * high limb with a sticky OR of the dropped bits, sums the exponents,
 * normalises (one-step shift-down on the leading carry) and round-to-nearest-
 * even on the low byte, writing class 3 / sign / exponent / mantissa to a result
 * descriptor recomposed by func_00122630.
 *
 * NEAR-MISS WALL (sticky-shift normalise loop, annulling bnel delay-slot
 * fillers, a movn round-select). Seedable (packed double pair -> packed double):
 * ships a faithful portable TARGET_NATIVE #else, cmp-oracle'd asm-vs-C
 * bit-identical on the real R5900 (incl. the 0*inf NaN path reading
 * D_00141810). The product mantissa is built from four 32x32 partial products
 * via __muldi3 exactly as the original does. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00122B00);
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
            return func_00122630(&D_00141810);   /* inf * 0 = NaN */
        }
        pa.sign = comb;                  /* inf * (inf|normal) = inf */
        return func_00122630(&pa);
    }
    if (clsB == 4) {                     /* a is not inf, b is inf */
        if (clsA == 2) {
            return func_00122630(&D_00141810);   /* 0 * inf = NaN */
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

/* func_00122DA8 = software double-precision DIVIDE of two decomposed operands
 * (FpParts a / b -> packed double via func_00122630). Decomposes both operands
 * with func_00122760, then: NaN propagates (returns a or b); the result sign is
 * signA^signB; inf/inf and 0/0 return the global NaN descriptor &D_00141810;
 * inf/finite -> inf, 0/finite -> 0, finite/inf -> 0, finite/0 -> inf; otherwise
 * (both normal) it aligns by exponent difference, runs a restoring bitwise
 * long-division of the 61-bit mantissas (shift-and-subtract producing one
 * quotient bit per step from bit 60 down) and round-to-nearest-even on the low
 * byte, writing class 3 / sign / exponent / quotient to the result descriptor.
 *
 * NEAR-MISS WALL (same class as its sibling add-core func_00122800): the
 * restoring-division loop uses annulling `bnel` delay-slot fillers and a `movn`
 * round-select that ee-gcc -O2 -G0 will not reproduce from clean C. Seedable
 * (packed double pair -> packed double): ships a faithful portable
 * TARGET_NATIVE #else, cmp-oracle'd asm-vs-C bit-identical on the real R5900
 * (incl. the inf/inf and 0/0 NaN paths reading D_00141810). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00122DA8);
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
        return func_00122630(&D_00141810);   /* inf / inf = NaN */
    }
    if (clsA == 2) {                     /* a is zero */
        if (clsB != 2) {
            return func_00122630(&pa);   /* 0 / finite or 0 / inf = 0 */
        }
        return func_00122630(&D_00141810);   /* 0 / 0 = NaN */
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
 * func_00122F10 = ordered comparison of two decomposed doubles (FpParts).
 * Returns 1 if either operand is NaN (class < 2). Otherwise orders by class
 * (inf/zero/normal) and, for two normals of equal sign, by exponent then by
 * unsigned mantissa, yielding a negative/zero/positive result whose sign tracks
 * a<b / a==b / a>b. (Two infinities or two zeros compare by sign difference.)
 *
 * MATCHED (byte-exact) via the near-miss idiom levers — the "83.70% wall" was
 * C-controllable: (1) `(cX^K)==0` forces the original's `xori;beqz` class tests;
 * (2) the sign-selects use `c?1:-1` (movz) vs `(c==0)?-1:1` (movn) to reproduce
 * the original's distinct movz/movn fillers + tail-merging; (3) writing the
 * exponent/mantissa magnitude compares a-first (`a>b`, not `b<a`) makes a's field
 * load first and recovers the annulling `bnel` branch-likely + register threading.
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

/**
 * func_00123078 = convert a 32-bit signed integer to a double (packed bits),
 * i.e. soft-float __floatsidf. Builds an FpParts descriptor: zero -> class 2;
 * otherwise class 3 with the magnitude as mantissa and exponent seeded at 60,
 * then a normalising left-shift loop brings the leading bit to position 60
 * (decrementing the exponent). INT_MIN is returned directly as the constant
 * double -2^31 (0xC1E0000000000000) to avoid negating 0x80000000. The
 * descriptor is recomposed by func_00122630.
 *
 * NEAR-MISS WALL (87.07% via objdiff). The control flow, constants and the
 * `(u64)-1 >> 4` normalise limit all compile to the original's instructions; the
 * residual divergence is ee-gcc -O2 -G0 delay-slot/register allocation in the
 * normalise loop and tail: the original fills the guard-branch delay slot with
 * the exponent load (keeping the mantissa in $5, exponent in $4 and computing
 * &parts in the final jal's own delay slot), whereas ee-gcc hoists &parts into
 * the guard delay slot (mantissa $3, jal delay nop) — a dbr/allocation choice
 * not controllable from C. Shipped as a portable TARGET_NATIVE #else; the #else
 * calls sibling soft-float func_00122630 (#else), so verification is routed to
 * tester-EE rather than a vacuous standalone oracle.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00123078);
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

/**
 * func_00123130 = convert a double to a SIGNED 32-bit integer (truncate toward
 * zero), i.e. soft-float __fixdfsi — the signed sibling of func_001231C8. NaN,
 * zero and negative-exponent inputs return 0; infinity and any value too large
 * for s32 (unbiased exponent >= 31) saturate to INT_MAX / INT_MIN by sign;
 * otherwise the class-3 mantissa (leading bit at 60) is right-shifted by
 * (60 - exp) and negated when the sign bit is set.
 *
 * Same idiom set as func_001231C8: class equality via `(cls ^ K) == 0` (xori);
 * the inf and overflow saturations share one block (both `sign != 0 ? MIN : MAX`,
 * which lowers to `movn`; the `!= 0` operand order also fixes the saturation
 * constant build schedule); the sign-apply is `sign == 0 ? r : -r` (negu + movn).
 *
 * MATCHED: byte-exact at -O2 -G0 (raw-byte + symbol-size verified).
 */
s32 func_00123130(s64 a) {
    s64 va = a;
    FpParts parts;
    s32 cls, exp, result;
    func_00122760(&va, &parts);
    cls = parts.fpClass;
    if ((cls ^ 2) == 0) {
        return 0;
    }
    if ((u32)cls < 2) {
        return 0;
    }
    if ((cls ^ 4) == 0) {
        return parts.sign != 0 ? (s32)0x80000000 : 0x7FFFFFFF;
    }
    exp = parts.exponent;
    if (exp < 0) {
        return 0;
    }
    if (exp >= 31) {
        return parts.sign != 0 ? (s32)0x80000000 : 0x7FFFFFFF;
    }
    result = (s32)((u64)parts.mantissa >> (60 - exp));
    return parts.sign == 0 ? result : -result;
}

/**
 * func_001231C8 = convert a non-negative double to a 32-bit integer (truncating
 * toward zero), i.e. soft-float __fixunsdfsi-style. NaN, zero, subnormal and
 * negative inputs return 0; infinity and any value too large for 32 bits
 * (unbiased exponent >= 32) return 0xFFFFFFFF. Otherwise the class-3 mantissa
 * (leading bit at position 60) is shifted to place the integer value in the low
 * bits: right by (60-exp) for exp<=60, left by (exp-60) above.
 *
 * MATCHED (byte-exact) via the near-miss idiom levers — the former "68.20% wall"
 * was C-controllable after all: (1) return type u32 makes the overflow constant
 * 0xFFFFFFFF build as `lui;ori` (not the s32 `li -1`) and frees the annulling
 * `bnel` on the exponent range check; (2) `(cls^K)==0` forces the original's
 * `xori;beqz` class tests where `cls==K` would emit `li;beq`. Calls sibling
 * soft-float func_00122760.
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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001232EC);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001232F0);

/* Decomposed IEEE-754 single produced by func_00123400 (32-bit fields). */
typedef struct {
    s32 fpClass;   /* 0x00: 0=sNaN,1=qNaN,2=zero/subnormal,3=normal,4=inf */
    s32 sign;      /* 0x04 */
    s32 exponent;  /* 0x08: unbiased (bias 0x7F) */
    s32 mantissa;  /* 0x0C: normal = (frac<<7)|(1<<30) */
} SpParts;

/* func_00123400: decompose the IEEE-754 single-precision float at src[0] into an
 * SpParts classification record, returning the class. Behaviour fully understood
 * but a NEAR-MISS WALL (~68% via objdiff): pervasive register allocation differs
 * from the original AND the asm returns 1 for BOTH NaN kinds while storing class
 * 0 for a signalling NaN (a leftover-register quirk ee-gcc won't reproduce).
 * Seedable (float bits -> SpParts): shipped as a cmp-oracle'd portable #else
 * (asm-vs-C proven bit-identical on real R5900 by run_cmp_015180_iso.sh). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00123400);
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00123490);

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

extern s32 func_00123400(u32 *src, SpParts *out);

/**
 * func_001234F0 = convert a single-precision float to a double and recompose it
 * (soft-float __extendsfdf2 helper): decompose the float into SpParts
 * (func_00123400), widen its 31-bit single mantissa to the 61-bit double form
 * (<<30) and hand class/sign/exponent/mantissa to func_00123268, RETURNING the
 * recomposed double (the soft-float __extendsfdf2 widen result). The .s does not
 * reload v0/v1 before jr ra, so func_001234F0 returns func_00123268's double
 * verbatim. (exponent and mantissa carry over; the single and double unbiased
 * exponents coincide here since func_00123268 re-applies the bias.)
 *
 * MATCHED: byte-exact at -O2 -G0 (raw-byte + symbol-size verified). Also the
 * cmp-oracle'd extendsfdf2 behaviour (the double-bits return).
 */
s64 func_001234F0(float f) {
    SpParts sp;
    func_00123400((u32 *)&f, &sp);
    return func_00123268(sp.fpClass, sp.sign, sp.exponent,
                         (s64)((u64)(u32)sp.mantissa << 30));
}

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001235C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001236C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00123930);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00123978);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00123A00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00123B40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00123C28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00123D30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001240C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001242A0);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124414);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124418);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", WaitGsPathsIdle);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124540);

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
extern s32 func_0011F628(void);
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124630);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001246D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124780);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124818);

/* func_001248B0: if the callback D_00141844 is installed and the suppression
 * flag D_001363A4 is clear, invoke the callback with the parameter D_00141848.
 * The body compiles byte-exact, but this is a splat mis-split: the per-function
 * .s (and the address-ordered unit listing) start the symbol 8 bytes early on a
 * trailing `addiu $29,$29,0x40; nop` epilogue fragment of the previous function
 * (func_001248F8 even references `func_001248B0 + 0x8` as the real entry). Can't
 * be matched at the unit level without a re-split. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001248B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001248F8);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124970);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124980);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124AF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124B88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124C28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00124C98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", sceCdInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", sceCdDiskReady);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", sceCdMmode);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001253A4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001253A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", QueryCdStatusOverRpc);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00125620);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", sceCdReadClock);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", sceGsResetGraph);

extern s32 D_00137E00;

/**
 * Accessor: return the address of the global D_00137E00.
 */
s32 *func_00125960(void) {
    return &D_00137E00;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012596C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", GsDefDispEnvNeedsOffsetFix);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", GetGsDisplayOffsets);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", BuildGsDispEnv);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00125D94);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00125D98);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00125E54);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", CalcGsZbufferBasePtr);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", BuildGsDrawEnvPacket);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00126104);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00126108);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", WaitVblankGetField);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00126284);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00126288);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012646C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00126470);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", KickGifImageUpload);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012672C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00126730);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00126DBC);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00126DC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", sceDmaSyncChan);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00126ED0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", ResetDmacChannels);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00127040);

/* func_00127218: 8 bytes of inter-function padding (a dead `sw $4,0($3); nop`)
 * between func_00127040 and the real func_00127220 — a splat mis-split, pinned
 * to size 0x8 in symbol_addrs so func_00127220 gets a clean .s. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00127218);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00127288);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001272A8);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00127340);

/**
 * McInit (libmc): bring up the EE-side memory-card RPC client.
 * Takes no arguments. Returns 0 on success (the IOP-side status word), or a
 * negative error: SignalSema's result minus 100 if releasing the mutex failed,
 * -0x78 / -120 if mcserv.irx is older than 0x20A, -0x79 / -121 if mcman.irx is
 * older than 0x20E.
 *
 * Sequence: create the libmc mutex g_mcMutexSema once (CreateSema with
 * maxCount/initCount 1) if it has never been made (handle < 0); drain any
 * in-flight call via McSync(0,0,0); take the mutex (WaitSema); sceSifInitRpc(0);
 * then bind the libmc server (id 0x80000400) onto the client block g_mcRpcClient.
 *
 * Non-obvious behaviour:
 *  - The bind is a RETRY loop, not a single call. sceSifBindRpc only *starts* the
 *    bind, so the code then polls the client's ready word (g_mcRpcClient + 0x24)
 *    and, while it is still 0, burns a calibrated ~0x100000-iteration delay loop
 *    (padded with nops so the timing does not depend on the pipeline) before
 *    binding again. It only proceeds once the IOP reports the client ready.
 *  - A bind that returns negative is NOT retried: it prints "bind error libmc"
 *    via Kprintf and then HANGS FOREVER in a deliberate infinite loop. Losing the
 *    memory-card server is treated as unrecoverable.
 *  - The version handshake is the point of RPC command 0xFE: the 0xC-byte reply
 *    is [status, mcserv version, mcman version]. Each version failure clears the
 *    client-ready word (+0x24) so later Mc* wrappers refuse to run, which is the
 *    same "de-initialise" store used on the mutex-release failure path.
 *  - Unlike the per-call wrappers, McInit RELEASES the mutex itself (SignalSema)
 *    rather than leaving it for McSync, because the 0xFE handshake is awaited
 *    synchronously here and no completion callback will arrive for it.
 *
 * MATCH STATUS: NOT byte-exact - parked at 89.26% (objdiff, USA cod/015180).
 * The C below is semantically correct and compiles to instruction-for-instruction
 * equivalent code: both delay loops are byte-identical (lui 0x10 / nop / addiu -1
 * / 4x nop / bnez / nop, and the 6-nop hang loop), the SemaParam stores land in
 * the original's order and slots (20(sp), then 24(sp) in the jal delay slot), and
 * every branch form and constant matches (bgezl, slti 522/526, -100/-120/-121).
 * The ONLY residual is register colouring: the original allocates SEVEN
 * callee-saved registers (s0-s6, 176-byte frame) because it keeps a redundant
 * third %hi(0x14) base live alongside both derived pointers, whereas ee-gcc 2.9
 * folds that base away and needs only SIX (s0-s4, 144-byte frame). That is a
 * 6-instruction delta consisting purely of the extra sd/ld pair plus frame
 * padding. No C phrasing controls the allocator here - measured levers, all
 * worse or neutral: hoisting the request pointer above the loop 87.18%, hoisting
 * the result pointer 85.22%, array-form client store in the first error arm
 * 87.59%, in the third arm 87.44%. This is the known register-colouring wall.
 */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", McInit);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00127500);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", McOpen);

extern s32 McOpen(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern s32 D_00137E68;

/**
 * Allocate/acquire via McOpen(arg0, arg1, arg2, 0x40). On failure (NULL
 * result) record error code 0xB in D_00137E68. Returns the McOpen result.
 */
s32 func_00127630(s32 arg0, s32 arg1, s32 arg2) {
    s32 result = McOpen(arg0, arg1, arg2, 0x40);
    if (result == 0) {
        D_00137E68 = 0xB;
    }
    return result;
}

extern u8   g_mcRpcClient[];   /* libmc RPC client block (init flag @+0x24) */
extern s32  g_mcMutexSema;     /* libmc mutex/semaphore handle */
extern s32  D_00141B80;        /* libmc RPC send-buffer (fd marshalled @+0) */
extern u32  g_mcRpcResult;     /* libmc RPC receive-buffer (result code) */
extern s32  func_0011AC70(s32 sema);
extern void func_0011AC40(s32 sema);

/**
 * func_00127668 = McClose (libmc): close the memory-card file descriptor `fd`.
 * Guards on the RPC client being initialised (returns -0x64 / -100 if not), takes
 * the libmc mutex (returns -0xC8 / -200 if that fails), marshals fd into the send
 * buffer and issues RPC #3 (func_0011D620, sceSifCallRpc-style). On RPC success
 * records status 3 in D_00137E68; on RPC failure releases the mutex. Returns the
 * RPC result.
 */
s32 func_00127668(s32 fd) {
    u8 *client = g_mcRpcClient;
    s32 r;
    if (*(s32 *)(client + 0x24) == 0) {
        return -0x64;
    }
    if (func_0011AC70(g_mcMutexSema) < 0) {
        return -0xC8;
    }
    D_00141B80 = fd;
    r = func_0011D620(client, 3, 1, &D_00141B80, 0x30, &g_mcRpcResult, 4, 0, 0);
    if (r == 0) {
        D_00137E68 = 3;
    } else {
        func_0011AC40(g_mcMutexSema);
    }
    return r;
}

/** func_00127720 = McSeek (libmc): seek fd to offset by whence. Same RPC-wrapper
 *  pattern as McClose (func_00127668) — init guard, mutex, RPC #4 — with the send
 *  buffer carrying fd @+0, offset @+0x10, whence @+0x14. */
s32 func_00127720(s32 fd, s32 offset, s32 whence) {
    u8 *client = g_mcRpcClient;
    s32 r;
    if (*(s32 *)(client + 0x24) == 0) {
        return -0x64;
    }
    if (func_0011AC70(g_mcMutexSema) < 0) {
        return -0xC8;
    }
    D_00141B80 = fd;
    *(s32 *)((char *)&D_00141B80 + 0x10) = offset;
    *(s32 *)((char *)&D_00141B80 + 0x14) = whence;
    r = func_0011D620(client, 4, 1, &D_00141B80, 0x30, &g_mcRpcResult, 4, 0, 0);
    if (r == 0) {
        D_00137E68 = 4;
    } else {
        func_0011AC40(g_mcMutexSema);
    }
    return r;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001277F8);

extern void sceSifWriteBackDCache(void *buf, s32 size);  /* cache writeback/invalidate */
extern u8   D_00142000[];                         /* libmc DMA staging buffer */
extern void func_001277F8(void);                  /* McRead RPC end-callback */

/** func_00127888 = McRead (libmc): read `size` bytes from fd into `buf`. RPC #5
 *  send buffer carries fd@+0, size@+0xC, buf@+0x18, DMA-staging @+0x1C; flushes
 *  the user buffer and the DMA buffer before the RPC, which runs with the
 *  func_001277F8 end-callback (endArg = the DMA buffer). */
s32 func_00127888(s32 fd, void *buf, s32 size) {
    u8 *client = g_mcRpcClient;
    s32 r;
    if (*(s32 *)(client + 0x24) == 0) {
        return -0x64;
    }
    if (func_0011AC70(g_mcMutexSema) < 0) {
        return -0xC8;
    }
    D_00141B80 = fd;
    *(u8 **)((char *)&D_00141B80 + 0x1C) = D_00142000;
    *(void **)((char *)&D_00141B80 + 0x18) = buf;
    *(s32 *)((char *)&D_00141B80 + 0xC) = size;
    sceSifWriteBackDCache(buf, size);
    sceSifWriteBackDCache(D_00142000, 0xC0);
    r = func_0011D620(client, 5, 1, &D_00141B80, 0x30, &g_mcRpcResult, 4,
                      (s32)func_001277F8, (s32)D_00142000);
    if (r == 0) {
        D_00137E68 = 5;
    } else {
        func_0011AC40(g_mcMutexSema);
    }
    return r;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", McWrite);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00127B18);

extern void func_00127B18(void);   /* libmc timer callback */
extern s32  func_0011AB40(void);   /* start/arm the timer (void tail call) */

/** McDelayMillis (libmc): arm a `millis`-ms timer whose expiry
 *  runs func_00127B18. Registers the handler (func_0011A9A0) with a fresh timer
 *  object (func_0011AB10) then tail-calls func_0011AB40 to start it. */
void McDelayMillis(s32 millis) {
    s32 id = millis & 0xFFFF;
    void (*cb)(void) = func_00127B18;
    s32 obj = func_0011AB10();
    func_0011A9A0(id, cb, obj);
    func_0011AB40();
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", McSync);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", McGetInfo);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00127E40);

extern u8    D_00141BB0[];   /* libmc GetDir send-buffer (1044 bytes) */
extern char *func_00115AC0(char *dst, const char *src, s32 n);  /* strncpy */

/**
 * func_00127E48 = McGetDir (libmc): request a memory-card directory listing for
 * `name` (a path/glob) on (port, slot) into the caller's `table`, an array of up
 * to `maxent` 64-byte directory entries; `mode` selects the listing variant.
 *
 * Same RPC-wrapper pattern as the rest of the family (McClose/McSeek): returns
 * -0x64 / -100 if the RPC client is not initialised and -0xC8 / -200 if the
 * libmc mutex is already held, before touching anything. Additionally rejects a
 * null or empty `name` with -0xD2 / -210 — and that path, unlike the two guards
 * above, happens AFTER the mutex was taken, so it must release it first.
 *
 * The entry table is only cache-flushed when `maxent >= 0`; a negative maxent
 * (a "count only, don't fill" request) skips the writeback entirely, so `table`
 * is allowed to be garbage in that case. maxent << 6 is the byte size at 64
 * bytes per entry.
 *
 * Release-or-hold: on RPC success the mutex is deliberately NOT released — the
 * call is in flight and McSync's completion path releases it, with the pending
 * command number (13) recorded in D_00137E68. Only an RPC submission failure
 * releases the mutex here, since no completion will ever arrive.
 *
 * Note the send buffer is D_00141BB0, NOT the D_00141B80 used by the smaller
 * calls in this family: the 1024-byte name field needs a 0x414-byte request.
 * The name is copied with a bounded strncpy(dst, name, 1023) that does NOT
 * guarantee termination, hence the explicit terminator store at +0x413.
 * Returns the sceSifCallRpc result (0 = successfully submitted).
 */
s32 func_00127E48(s32 port, s32 slot, const char *name, s32 mode,
                  s32 maxent, void *table) {
    u8 *client = g_mcRpcClient;
    u8 *req;
    s32 r;

    if (*(s32 *)(client + 0x24) == 0) {
        return -0x64;
    }
    if (func_0011AC70(g_mcMutexSema) < 0) {
        return -0xC8;
    }
    if (name == NULL || *name == '\0') {
        func_0011AC40(g_mcMutexSema);
        return -0xD2;
    }

    req = D_00141BB0;
    *(s32 *)(req + 0x00) = port;
    *(s32 *)(req + 0x04) = slot;
    *(s32 *)(req + 0x08) = mode;
    *(s32 *)(req + 0x0C) = maxent;
    *(void **)(req + 0x10) = table;
    func_00115AC0((char *)(req + 0x14), name, 0x3FF);
    req[0x413] = 0;

    if (maxent >= 0) {
        sceSifWriteBackDCache(table, maxent << 6);
    }

    r = func_0011D620(client, 13, 1, req, 0x414, &g_mcRpcResult, 4, 0, 0);
    if (r == 0) {
        D_00137E68 = 13;
    } else {
        func_0011AC40(g_mcMutexSema);
    }
    return r;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00127F90);

/**
 * McChdir — issue one libmc (memory-card) request over SIF RPC, function 16.
 *
 * Fills the shared 48-byte request block with the caller's two arguments and fires an
 * asynchronous RPC at the IOP-side libmc server. Returns 0 once the call is ACCEPTED —
 * not once it completes; the IOP posts its result into g_mcRpcResult later and McSync
 * is what waits for it.
 *
 * Returns  0     the request was accepted and is in flight
 *         -100   the RPC client is not bound yet (McInit has not run / bind failed)
 *         -200   another libmc call already holds the serialising semaphore
 *         other  the RPC layer's own failure code, passed through unchanged
 *
 * THE SEMAPHORE IS DELIBERATELY NOT RELEASED ON SUCCESS. It is taken here and stays
 * held for the lifetime of the in-flight call; the completion path releases it. Only
 * the FAILURE path signals it back, because on failure there is no completion coming.
 * Reading this as a leak is the natural misreading, and it is wrong.
 *
 * It polls rather than waits: the guard is PollSema (syscall 0x45), NOT WaitSema
 * (0x44), so a busy card returns -200 immediately instead of blocking the EE. A caller
 * that treats -200 as fatal rather than "retry later" will drop requests.
 *
 * Naming basis: g_mcRpcClient / g_mcRpcResult / g_mcMutexSema are CONFIRMED in
 * symbol_addrs; func_0011AC70 and func_0011AC40 are pinned by their syscall numbers
 * (0x45 PollSema, 0x42 SignalSema), calibrated against this unit's own stubs
 * (0x40 CreateSema, 0x41 DeleteSema, 0x44 WaitSema at func_0011AC60).
 *
 * The ROM arm is raw .word: this body is a RECOVERED splat-dropped function (see the
 * marker below), so the portable arm is the only readable form of it in the tree.
 */
#ifndef TARGET_NATIVE
// recovered splat-dropped code (epilogue-stump mis-split): raw words, byte-exact
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", McChdir);
#else
/* 0x141B00 — SIF RPC client handle for libmc; +0x24 is nonzero once bound. */
extern u8  g_mcRpcClient[];
/* 0x1430C0 — where the IOP DMAs the result code for the last call. */
extern u32 g_mcRpcResult;
/* 0x137E6C — semaphore serialising libmc RPC calls, created in McInit. */
extern s32 g_mcMutexSema;
/* 0x141B80 — the shared 48-byte request block. +4 and +8 are this call's arguments. */
extern u8  g_mcRpcRequest[];
/* 0x137E68 — the RPC function number of the call currently in flight. */
extern s32 g_mcPendingCmd;

extern s32 func_0011AC70(s32 sema);   /* syscall 0x45 PollSema   */
extern void func_0011AC40(s32 sema);  /* syscall 0x42 SignalSema */
/* func_0011D620 (sceSifCallRpc-shaped) is already declared at file scope earlier in
 * this unit; NOT redeclared here. My first draft did redeclare it with `void *` for the
 * trailing end-function/end-param pair and the native gate rejected it as a conflicting
 * type — the existing declaration spells those two as s32. Reusing the unit's own
 * declaration is both correct and the reason the conflict cannot recur. */

s32 McChdir(s32 arg0, s32 arg1) {
    s32 rc;

    if (*(s32 *)(g_mcRpcClient + 0x24) == 0) {
        return -100;                       /* client never bound */
    }
    if (func_0011AC70(g_mcMutexSema) < 0) {
        return -200;                       /* busy — poll, not wait */
    }

    *(s32 *)(g_mcRpcRequest + 4) = arg0;
    *(s32 *)(g_mcRpcRequest + 8) = arg1;

    rc = func_0011D620(g_mcRpcClient, 16, 1,
                       g_mcRpcRequest, 48, &g_mcRpcResult, 4, 0, 0);
    if (rc != 0) {
        func_0011AC40(g_mcMutexSema);      /* no completion is coming — release */
    } else {
        g_mcPendingCmd = 16;               /* held; the completion path releases */
    }
    return rc;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", McMkDir);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", McGetEntSpace);

extern s32 D_00143108;
/* Array-typed: func_00128440 stores arg0 at D_00143180[1] and the ARRAY
 * extern is required for its match — the scalar (&D)[1] idiom anchors the
 * %hi at D+4 and flips the branch to bgezl. func_00128250 shares this decl
 * and stays byte-identical under the array phrasing (tester-verified USA). */
extern s32 D_00143180[];

/**
 * Initialise the D_00143180 subsystem by calling func_0011D620 with the config
 * block at &D_00143108, mode 0x80000963, two 0x400-sized buffers both pointing
 * at D_00143180, and zeroed trailing arguments; returns the resulting handle
 * stored in D_00143180[0].
 */
s32 func_00128250(void) {
    func_0011D620(&D_00143108, 0x80000963, 0, D_00143180, 0x400,
                  D_00143180, 0x400, 0, 0);
    return D_00143180[0];
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", sceDbcInit);

extern char D_0013B868[];
extern void func_00128898(const char *fmt, ...);

/**
 * func_00128440(arg0): open the D_00143180 subsystem in mode 0x80000904 with
 * arg0 stored at D_00143180[1], via func_0011D620; on failure log D_0013B868
 * (func_00128898) and return 0, else return the handle D_00143180[0].
 *
 * Match note: the ARRAY extern (not the scalar (&D)[1] idiom) is what lets the
 * %hi(D_00143180) CSE into callee-saved $16 across the call and lands the
 * result load in the bgez delay slot — byte-exact as plain faithful C.
 */
s32 func_00128440(s32 arg0) {
    D_00143180[1] = arg0;
    if (func_0011D620(&D_00143108, 0x80000904, 0, D_00143180, 0x400,
                      D_00143180, 0x400, 0, 0) < 0) {
        func_00128898(D_0013B868);
        return 0;
    }
    return D_00143180[0];
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001284B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00128578);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001286C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001286C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001287A8);

/**
 * Compiled-out VARARGS debug print stub (libmc area): the body is empty but
 * the `...` still makes ee-gcc home the unnamed arg registers $5-$11 to the
 * 0x80-byte stack area (the sd run at +0x48..+0x78) — that varargs prologue
 * IS the whole function. First arg is the (ignored) format string.
 */
void func_00128898(const char *fmt, ...) {
}

extern s32 D_00137E80;
/* Sub-object of a resource-table entry (dual instance at +0x0/+0x80 of the
 * object; func_00128DB0 selects between the two by their +0x7C word). */
typedef struct ResSubObj {
    u8 pad[0x7C];
    s32 unk7C;     /* 0x07C — selection key (larger wins) */
} ResSubObj;

/* 16-entry resource table, stride 0x330: +0x4 active flag, +0x8 handle,
 * +0xC object pointer. Struct-typed so func_00128D58's per-field array
 * indexing compiles to the original dual-base store pair. */
typedef struct ResTableEntry {
    s32 unk0;         /* 0x000 */
    s32 active;       /* 0x004 */
    s32 handle;       /* 0x008 */
    ResSubObj *obj;   /* 0x00C */
    u8 pad[0x330 - 16];
} ResTableEntry;
extern ResTableEntry D_00143640[];

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
    end = (s32 *)((u8 *)D_00143640 + 0x3300);
    do {
        entry[0] = 0;
        entry[1] = 0;
        entry[2] = 0;
        entry = (s32 *)((u8 *)entry + 0x330);
    } while ((s32)entry < (s32)end);
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", sceDbcPortOpen);

/* func_00128A48: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real function begins at func_00128A50. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00128A48);

/* func_00128A50: real function recovered from the splat mis-split above (indexes
 * the 0x330-stride table D_00143640, dispatches to func_00128D58/DB0/E98 + a
 * memcpy). Boundary now correct; body not yet decompiled. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00128A50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00128B28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00128C18);

extern s32 func_00128578(s32 index);

/**
 * func_00128D58(index): acquire a resource via func_00128578(index); on
 * success (non-negative handle) record it at entry+0x8 and set the active
 * flag at entry+0x4 in the 0x330-stride D_00143640 table; returns the handle
 * (negative = failure, table untouched).
 *
 * Match note: the original's dual base registers come from indexing
 * D_00143640[index] SEPARATELY per field (no pointer temp — a local
 * ResTableEntry* changes the frame), and the early-exit `if (h < 0) return h;`
 * yields the unified epilogue + in-block return copy. Byte-exact.
 */
s32 func_00128D58(s32 index) {
    s32 h = func_00128578(index);
    if (h < 0) {
        return h;
    }
    D_00143640[index].handle = h;
    D_00143640[index].active = 1;
    return h;
}

extern void func_0011B3D0(void *arg0, void *arg1);

/**
 * func_00128DB0(index): run func_0011B3D0 over table entry `index`'s object
 * (args: obj, obj+0x100), then return whichever of the object's two
 * sub-instances (obj / obj+0x80) has the larger +0x7C word (ties -> obj).
 *
 * Match note: the original's "spills" around the call are a LOCAL ARRAY
 * (`pair[2]` on the stack) — the tail computes `pair[p0->unk7C < p1->unk7C]`
 * as an indexed stack load. Writing the pair as separate locals can't match.
 */
ResSubObj *func_00128DB0(s32 index) {
    ResSubObj *pair[2];
    ResSubObj *obj = D_00143640[index].obj;
    pair[0] = obj;
    pair[1] = (ResSubObj *)((u8 *)obj + 0x80);
    func_0011B3D0(obj, (u8 *)obj + 0x100);
    return pair[pair[0]->unk7C < pair[1]->unk7C];
}

extern s32 D_00137E88[];

/**
 * func_00128E18(index): lazily refresh the two-word state cache D_00137E88
 * from table entry `index`'s object. Returns 0 when obj->unk7C is 0 or the
 * cache already holds the (obj->unk7C, next->unk7C) pair; otherwise updates
 * the cache from the pair and returns 1.
 *
 * Match note: the "spills" are the LOCAL ARRAY idiom again — pair[2] on the
 * stack (cc1 2.9 never register-promotes array elements), while the compare
 * path reads k/next from registers. Same class as func_00128DB0; byte-exact.
 */
s32 func_00128E18(s32 index) {
    ResSubObj *pair[2];
    ResSubObj *obj = D_00143640[index].obj;
    ResSubObj *next = (ResSubObj *)((u8 *)obj + 0x80);
    s32 k = obj->unk7C;
    pair[0] = obj;
    pair[1] = next;
    if (k == 0) {
        return 0;
    }
    if (D_00137E88[0] == k && D_00137E88[1] == next->unk7C) {
        return 0;
    }
    D_00137E88[0] = pair[0]->unk7C;
    D_00137E88[1] = pair[1]->unk7C;
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00128E98);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00128F48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00128F50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00128FD0);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001290BC);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001290E0);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00129120);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00129148);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00129160);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00129178);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001291A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00129218);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001292C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00129368);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00129410);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00129450);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001296A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00129DA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012A1C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012A3E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012A460);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012A4F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012A5B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012A680);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012A730);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012A7E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012A8E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012A9E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012AA80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012AB30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012AC10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012ACF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012ADD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012AEA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012AFC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012B0D8);

/**
 * func_0012B138 — saturating halfword -> byte pack (VU0 macro mode).
 *
 * Converts 384 signed halfwords to 384 unsigned bytes, clamping each to [0, 255]:
 * a clamp-and-pack blit, the classic "fixed-point accumulator -> 8-bit pixel/PCM"
 * conversion. `dst` receives 384 bytes; `src` supplies 384 halfwords (768 bytes).
 *
 * The ROM does it 16 halfwords at a time with the EE's parallel ops: two `lq` loads,
 * `pminh`/`pmaxh` against 0x00FF and zero for the saturation, `ppacb` to take the low
 * byte of each halfword, one `sq` out. 24 iterations x 16 = 384.
 *
 * NOTE the clamp constant is a 128-bit value EMBEDDED IN THE INSTRUCTION STREAM at
 * 0x12B180 (four `.word 0x00FF00FF` between the loop and the `jr ra`), loaded with
 * `lq t3,0(t2)` where t2 is an address inside this very function. That is unusual and
 * it is why the C arm cannot be byte-exact here: the constant is part of the code.
 *
 * `pmaxh` against $zero clamps the LOW end, so negative inputs become 0 rather than
 * wrapping — the saturation is genuinely two-sided, not a mask.
 *
 * Present identically in USA and EU (same first word, same 24 instructions).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012B138);
#else
void func_0012B138(u8 *dst, const s16 *src) {
    s32 i;

    /* The ROM packs 16 at a time; the visible effect is elementwise and in order,
     * because `ppacb t2, t1, t0` places t0's eight low bytes below t1's eight, and
     * the store lands at the pre-increment `dst`. So a linear loop is faithful. */
    for (i = 0; i < 384; i++) {
        s16 v = src[i];

        if (v > 255) v = 255;   /* pminh against 0x00FF */
        if (v < 0)   v = 0;     /* pmaxh against $zero  */
        dst[i] = (u8)v;         /* ppacb: low byte of each halfword */
    }
}
#endif

/**
 * Set bit 23 of the hardware register at 0x10002010 (IPU_CTRL) to the low bit of
 * arg0, preserving all other bits (read-modify-write with mask 0xFF7FFFFF).
 */
void func_0012B198(s32 arg0) {
    u32 *reg = (u32 *)0x10002010;
    *reg = (*reg & 0xFF7FFFFF) | (arg0 << 23);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012B1C0);

/* func_0012B3C0(arg0): thin wrapper — forwards to func_0012C508(arg0, 3) and
 * returns its result. (The prior "sibling-call wall" note was wrong: ee-gcc 2.9
 * has NO sibling-call optimization, so this compiles to the original's jal +
 * real frame — byte-exact.) */
extern s32 func_0012C508(s32 arg0, s32 arg1);
s32 func_0012B3C0(s32 arg0) {
    return func_0012C508(arg0, 3);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012B3E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012B568);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012B678);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012B780);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012B8B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012BAA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012BB60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012C008);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012C090);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012C230);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", IpuWaitReady);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", IpuWaitCmdResult);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012C508);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012C680);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", IpuSkipBits);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", IpuGetBits);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012C9C8);

extern s32 IpuSkipBits(s32 *arg0, s32 arg1);
extern s32 IpuGetBits(s32 *arg0, s32 arg1);
extern s32 func_0012CFA0(s32 *arg0);

/**
 * Run channel 5's transfer on arg0, recording its handle at arg0->field_0x1B4.
 * If channel 1 is ready (IpuGetBits(arg0, 1) is non-zero) kick it off again,
 * fire channel 7 via IpuSkipBits and flush through func_0012CFA0. Returns 0.
 */
s32 func_0012CA48(s32 *arg0) {
    arg0[0x6D] = IpuGetBits(arg0, 5);
    if (IpuGetBits(arg0, 1) != 0) {
        IpuGetBits(arg0, 1);
        IpuSkipBits(arg0, 7);
        func_0012CFA0(arg0);
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", IpuParseVideoStartCodes);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012CBC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012CC88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012CDB0);

/**
 * Drain object arg0: while channel 1 still reports work
 * (IpuGetBits(arg0, 1) is non-zero), keep servicing channel 8 via
 * IpuSkipBits(arg0, 8). The trailing channel-1 poll (0 on exit) is left in
 * the return register; callers ignore it.
 */
s32 func_0012CFA0(s32 *arg0) {
    while (IpuGetBits(arg0, 1) != 0) {
        IpuSkipBits(arg0, 8);
    }
}

struct ScrollObj {
    char _0[0x150];
    s32 mode;          /* 0x150: ==3 short-circuits the delta application */
    char _a[0x1AC - 0x154];
    s32 target;        /* 0x1AC: computed scroll target */
    char _b[0x84C - 0x1B0];
    s32 base;          /* 0x84C: base position */
    s32 extent;        /* 0x850: clamped up to >= target */
    s32 wrapFlag;      /* 0x854: cleared on any non-zero delta */
};

/**
 * func_0012CFE8(obj, delta): advance the scroll position of obj by delta.
 * Unless obj is in mode 3 or delta is 0, clears wrapFlag (0x854) and notes
 * whether it had been zero on a negative delta. The target (0x1AC) becomes
 * base+delta, bumped by 0x400 when the just-cleared wrapFlag was zero and the
 * applied delta did not decrease. Finally extent (0x850) is clamped up to the
 * target.
 *
 * Matched 2026-06-30 (was an INCLUDE_ASM give-up class: beql/bgezl/bnel
 * branch-likely + movn). The branch-likely forms fall out naturally from the
 * correct control flow - the keystone was that wrapFlag is zeroed on EVERY
 * non-zero delta (the bgezl-taken delay slot), NOT only on a negative delta;
 * placing `obj->wrapFlag = 0` outside the `delta < 0` test is what produces the
 * annulled-delay bgezl. Solo-clean (no data global) so EU C is identical.
 */
void func_0012CFE8(struct ScrollObj *obj, s32 delta) {
    s32 wasZero = 0;
    s32 applied = 0;
    s32 base, target, extent;
    if (obj->mode != 3 && delta != 0) {
        if (delta < 0) {
            if (obj->wrapFlag == 0) wasZero = 1;
        }
        obj->wrapFlag = 0;
        applied = delta;
    }
    base = obj->base;
    target = base + delta;
    obj->target = target;
    if (wasZero != 0) {
        if (applied >= delta) obj->target = target + 0x400;
    }
    extent = obj->extent;
    if (extent < obj->target) extent = obj->target;
    obj->extent = extent;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", IpuParseGopHeader);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012D100);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012D1C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012D2C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012D350);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012D420);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012D4B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012D768);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012D808);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012DAC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012DC50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012DD60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012DF18);

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012E10C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012E110);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012E238);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012E378);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012E538);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012E608);

extern void func_0012E8E8(u64 *arg0, s32 arg1);

struct E890Rec {
    u64 cleared0;      /* 0x0:  zeroed as one doubleword (sd $0) */
    s32 limit;         /* 0x8:  = limit arg */
    s32 limit2;        /* 0xC:  = limit arg (second slot) */
    s32 cleared10;     /* 0x10: zeroed */
    s32 _pad14;        /* 0x14: (layout) */
    u64 cleared18;     /* 0x18: zeroed as one doubleword (sd $0) */
    s32 start;         /* 0x20: = start arg */
    s32 end;           /* 0x24: = start + span */
    s32 span;          /* 0x28: = span arg */
};

/**
 * func_0012E890: initialise the record at arg0 - store the limit (arg1) at
 * field_0x8 and field_0xC, the end (start+span) at field_0x24, the span (arg3)
 * at field_0x28, the start (arg2) at field_0x20; zero field_0x0..0x4, 0x10 and
 * field_0x18..0x1C - then tail-call func_0012E8E8(arg0, 0).
 *
 * Matched 2026-06-30 (disproves the ~75% give-up that called this "a codegen
 * shape not expressible in source"): the store SCHEDULE is the whole game.
 * Writing the fields in ascending-offset order, but with field_0x8 emitted just
 * before the field_0x0 doubleword-zero, reproduces ee-gcc's exact schedule -
 * including the start-store (field_0x20 = arg2) landing in the sibling-call
 * delay slot. The two 8-byte zero stores are doublewords (sd $0), obtained by
 * typing field_0x0/0x18 as u64.
 */
void func_0012E890(struct E890Rec *rec, s32 limit, s32 start, s32 span) {
    rec->limit    = limit;
    rec->cleared0 = 0;
    rec->limit2   = limit;
    rec->cleared10 = 0;
    rec->cleared18 = 0;
    rec->start = start;
    rec->end   = start + span;
    rec->span  = span;
    func_0012E8E8((u64 *)rec, 0);
}

/**
 * Extract the top arg1 bits of the 64-bit value at *arg0: returns
 * (s32)(*arg0 >> (64 - arg1)) — i.e. the most-significant arg1 bits, right
 * aligned. (Bitstream/MSB-first reader helper.)
 */
s32 func_0012E8C8(u64 *arg0, s32 arg1) {
    u64 val = *arg0;
    return (s32)(val >> (0x40 - arg1));
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012E8E8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012EA18);

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012EA9C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012EAA0);

/* func_0012EB28: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real function begins at func_0012EB30. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012EB28);

/* func_0012EB30: real function recovered from the splat mis-split above (large
 * 0x150-frame routine iterating a 0x18-stride record list and dispatching via an
 * indirect call). Boundary now correct; body not yet decompiled. Left as
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012EB30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012EE28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012EF20);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012F0E0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012F738);

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012F948);

/**
 * func_0012F950(arg0, arg1, arg2): seed the display/DMA sub-object at arg0->0x40
 * (fB0=1, fD8=(arg1 & 0x0FFFFFFF) | 0x20000000, fE4=arg2, fDC=fE0=0) then run
 * func_0012FBF0(arg0) and return its result.
 *
 * MATCHED: byte-exact at -O2 -G0 (raw-byte + symbol-size verified). The
 * original's stack frame + `jal` (not a `j func_0012FBF0` sibcall) is what
 * ee-gcc 2.9 -O2 -G0 emits for a VALUE-returning tail call — `return
 * func_0012FBF0(arg0)` keeps $ra to pass the callee's value back. (The prior
 * "ee-gcc sibcalls the void call, won't reproduce" note predated that finding; a
 * void `func_0012FBF0(arg0);` WOULD sibcall to `j`.)
 */
extern s32 func_0012FBF0(void *arg0);

s32 func_0012F950(u8 *arg0, u32 arg1, s32 arg2) {
    u8 *obj = *(u8 **)(arg0 + 0x40);
    *(s32 *)(obj + 0xB0) = 1;
    *(s32 *)(obj + 0xD8) = (arg1 & 0x0FFFFFFF) | 0x20000000;
    *(s32 *)(obj + 0xE4) = arg2;
    *(s32 *)(obj + 0xE0) = 0;
    *(s32 *)(obj + 0xDC) = 0;
    return func_0012FBF0(arg0);
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012F998);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012F9A8);

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

/**
 * func_0012FA70: in the record table at arg0->field_0x40 (8-byte stride),
 * write arg3 into record[index]+0x10, then swap arg2 into record[index]+0xC,
 * returning that field's previous value.
 *
 * Matched 2026-06-30 (disproves the earlier 74.44% give-up): the original forms
 * the base+0xC member pointer FIRST, then advances both it and the base pointer
 * by index*8 in place (two live induction pointers). Reproduced exactly by
 * `member = base + 3; base += index*2; member += index*2;` - forming the +0xC
 * pointer before the in-place increments is what fixes the address-formation
 * schedule the prior attempt couldn't budge.
 */
s32 func_0012FA70(s32 *arg0, s32 index, s32 arg2, s32 arg3) {
    s32 *base = (s32 *)arg0[0x10];   /* arg0->field_0x40 */
    s32 *member = base + 3;          /* &record[].field_0xC */
    s32 old;
    base += index * 2;
    member += index * 2;
    base[4] = arg3;                  /* record[index] + 0x10 */
    old = *member;                   /* record[index] + 0xC */
    *member = arg2;
    return old;
}

/* func_0012FA98(arg0, arg1): if arg0 and its table arg0->field_0x40 are non-null,
 * fetch the destructor at table[*arg1*2 + 3] and, if set, call
 * dtor(arg0, arg1, table[*arg1*2 + 4]); return its result or 0. The natural body
 * reaches 92% — but the original keeps `ret` in $7 (a3) where ee-gcc allocates
 * a2, and emits a plain `beqz` on the callback test where ee-gcc picks the
 * branch-likely `beqzl` (annulling its delay slot). Both are scheduling/reg-
 * alloc forms this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012FA98);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012FB10);

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

extern void func_00130288(s32 arg0, void *buf);
extern char D_0013BD48[];

struct AllocRegion { u32 base; u32 span; u32 cursor; };

/**
 * func_0012FB80(arg0, region, size, align): bump-allocate within region
 * {base@0x0, span@0x4, cursor@0x8}. Round cursor UP to a multiple of align,
 * add size -> end. If base+span >= end, commit cursor=end and return the
 * aligned start; else the request overflows the region -> dispatch
 * func_00130288(arg0, &D_0013BD48) and return 0.
 *
 * Matched 2026-06-30: round-up = ((cursor+align-1)/align)*align via unsigned
 * divu+mult (the divu auto-emits the break-7 divide-by-zero guard). Lever #9
 * (arm order): writing the FIT case first (`if (base+span >= end)`) makes
 * ee-gcc emit the original `bnel`-to-overflow (overflow = the branch-likely
 * target). EU twin: same body, data global D_0013BD48 -> +0x80 D_0013BDC8.
 */
s32 func_0012FB80(s32 arg0, struct AllocRegion *region, s32 size, u32 align) {
    u32 rounded = ((region->cursor + align - 1) / align) * align;
    u32 end = rounded + size;
    if (region->base + region->span >= end) {
        region->cursor = end;
        return rounded;
    }
    func_00130288(arg0, D_0013BD48);
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012FBF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012FD60);

/* func_0012FE78: dispatch on the state word (offset 0x174) of arg0's sub-object
 * (arg0->field_0x40) — when it equals 3 hand off to func_0012FD60, otherwise to
 * func_0012FEC0; returns the chosen handler's result. Body
 * `if (((s32*)arg0[0x10])[0x5D] != 3) return func_0012FEC0(arg0); return
 * func_0012FD60(arg0);` reaches 97% — every instruction matches but the original
 * parks arg0 in $7 (a3) and the state in $2, while ee-gcc allocates a1/a0; a
 * register-allocation form this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012FE78);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0012FEC0);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00130098);

/**
 * func_00130118: initialise subsystem 1 (func_0012B198(1)), then program the
 * four hardware DMA/GIF register pointers into arg0 (field_0x590=0x70000000,
 * 0x594=0x70001800, 0x6D0=0x70001B00, 0x6D4=0x70003300) and clear the busy flag
 * at field_0x810. MATCHED. The original parks the use-once 0x70000000 in the
 * callee-saved $17 (0x30 frame, saving $16/$17). The LEVER (disproving the
 * earlier "cc1 won't reproduce" note): compute that constant into a LOCAL
 * *before* the call — that makes it live across func_0012B198, so ee-gcc spills
 * it to a callee-saved register exactly as the original. */
void func_00130118(void *arg0) {
    u32 gif = 0x70000000;
    func_0012B198(1);
    *(u32 *)((char *)arg0 + 0x590) = gif;
    *(u32 *)((char *)arg0 + 0x594) = 0x70001800;
    *(u32 *)((char *)arg0 + 0x6D0) = 0x70001B00;
    *(u32 *)((char *)arg0 + 0x6D4) = 0x70003300;
    *(u32 *)((char *)arg0 + 0x810) = 0;
}

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

extern u8 D_0013BDE8[];
extern void *Kprintf(const char *format, ...);

/**
 * Default message handler: print the message `buf` through Kprintf with the
 * fixed format string D_0013BDE8. Void tail call → sibling-call-optimised to
 * the original's frameless `j Kprintf` (buf shuffled to $a1, the format
 * address built across the delay slot).
 */
void func_00130240(void *buf) {
    Kprintf((const char *)D_0013BDE8, buf);
}

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00130300);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00130428);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001306D0);

/* func_001307B0(obj, cmd, madr): restart the GIF/PATH3 DMA pipeline — tear down
 * sub-object 2 (func_0012FA98), flush (IpuWaitReady) and reset the GIF mode
 * register (0x10002000=0); then with interrupts disabled program channel
 * 0x1000B400 (MADR 0x1000B410 = madr & 0x0FFFFFFF, QWC 0x1000B420 = 4, CHCR
 * 0x1000B400 = 0x101), restoring interrupts if on; finally issue IPU command
 * `cmd` (func_0012C380), flush again and tear down sub-object 3. 99.82% — every
 * instruction matches except the frame size: the original reserves a 0x60 frame
 * (saves parked at +0x20..+0x50) where ee-gcc only needs 0x50. A frame-size-only
 * constant mismatch this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001307B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00130890);

/**
 * Query a batch of channel/register states via IpuGetBits(obj, selector):
 * prime selector 3, and only if selector 1 is set, sample selector 8 three
 * times (caching the last into obj+0x144). Then cache selector 0xE into
 * obj+0x148, pulse selector 1, and cache selector 0xE again into obj+0x14C.
 */
void func_001309C0(s32 *obj) {
    IpuGetBits(obj, 3);
    if (IpuGetBits(obj, 1) != 0) {
        IpuGetBits(obj, 8);
        IpuGetBits(obj, 8);
        *(s32 *)((u8 *)obj + 0x144) = IpuGetBits(obj, 8);
    }
    *(s32 *)((u8 *)obj + 0x148) = IpuGetBits(obj, 0xE);
    IpuGetBits(obj, 1);
    *(s32 *)((u8 *)obj + 0x14C) = IpuGetBits(obj, 0xE);
}

extern u8 D_0013BE58[];
extern u8 D_0013BE88[];
extern u8 D_0013BEA0[];
extern u8 D_0013BED8[];
extern void func_00130C68(u8 *arg0);

/**
 * Frameless tail-call thunk: dispatch arg0 through func_00130288 with the
 * fixed message table D_0013BE58. Void tail call, so ee-gcc sibling-call-
 * optimises it into the original's `j func_00130288`. Sibling thunks
 * A60/A70/A80 differ only by table.
 */
void func_00130A50(s32 arg0) {
    func_00130288(arg0, D_0013BE58);
}

/** Sibling of func_00130A50 with message table D_0013BE88. */
void func_00130A60(s32 arg0) {
    func_00130288(arg0, D_0013BE88);
}

/** Sibling of func_00130A50 with message table D_0013BEA0. */
void func_00130A70(s32 arg0) {
    func_00130288(arg0, D_0013BEA0);
}

/** Sibling of func_00130A50 with message table D_0013BED8. */
void func_00130A80(s32 arg0) {
    func_00130288(arg0, D_0013BED8);
}

/* func_00130A8C: starts with a 0xCDCDCDCD fill word (uninitialised-memory
 * pattern) before the real entry at 0x130A90 — not producible from C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00130A8C);

/**
 * Frameless tail-call thunk: forward the sub-object at arg0->field_0x40 + 0x4C
 * to func_00130C68. Void tail call → sibling-call-optimised to the original's
 * `j func_00130C68` with the +0x4C adjust in the delay slot.
 */
void func_00130AA0(void *arg0) {
    func_00130C68(*(u8 **)((u8 *)arg0 + 0x40) + 0x4C);
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00130AAC);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00130B80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00130C68);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00130E88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001310C0);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001313C4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", s_isnan);

/**
 * Pass the 64-bit value at arg0 + 0x8 as both arguments to func_00123028,
 * discard its result, and return 0.
 */
s32 func_00131400(s64 *arg0) {
    s64 v = arg0[1];
    func_00123028(v, v);
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00131424);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_0013153C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00131540);

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00131620);

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00131668);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00131670);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001316C0);

/* func_001316C8: controller/port status getter — on territory 'T'
 * (func_001315E0()) return the cached byte D_00138156; else sample the pad
 * (func_0011ACD0), return 0 if the 3-bit port field (bits 13..15) is zero, else
 * read extended status (func_0011AF30) and return bit 4 of its low byte. Every
 * instruction matches at 98.85% except the %hi temp register for D_00138156: the
 * original reuses $2 (`lui $2; lbu $2,%lo($2)`) while ee-gcc splits the lui into
 * $3. A reg-alloc form this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_001316C8);

/**
 * Binary byte (0..99) → packed BCD (RTC/BCD clock family, inverse of the
 * neighbouring BCD→binary func_00131760): BCD(n) = n + 6*(n/10), e.g.
 * 59 → 0x59. The u8 param/return produce the callee-side andi masks; the
 * divide-by-10 emits the divu + beql/break zero-guard (harness break-0,7
 * fixup in move_fixup.sed) and the 3-op `mult` this cc1 produces natively.
 */
u8 func_00131730(u8 binary) {
    return binary / 10 * 6 + binary;
}

/**
 * Packed BCD byte → binary (RTC/BCD clock family, inverse of the neighbouring
 * binary→BCD func_00131730): n - 6*(n>>4), e.g. 0x59 → 59.
 *
 *   packed  a packed-BCD byte (high nibble tens, low nibble units)
 *   ->      the binary value, truncated to a byte
 *
 * Written to mirror the ROM rather than as the more obvious
 * `(n >> 4) * 10 + (n & 0xF)`: the original computes the CORRECTION term
 * (each BCD nibble wastes 6 of its 16 codes), which is why the .s holds a
 * single `mult ... 6` and a `subu` and never masks the low nibble.
 *
 * The two forms are EXACTLY equivalent, not merely equivalent on valid BCD:
 *   n = 16h + l  ->  n - 6h = 16h + l - 6h = 10h + l
 * Verified over all 256 byte values, 0 disagreements -- including malformed
 * BCD (a nibble > 9), where the low nibble simply carries into the result.
 *
 * The u8 param/return produce the callee-side `andi 0xFF` masks -- the entry
 * `andi $2,$4,0xFF` and the one in the jr delay slot.
 *
 * Non-obvious: the correction term is a GNU C local register variable bound
 * to LO. The ROM multiplies with the 2-operand `mult $3,$4` (rd = $0, product
 * left in LO) followed by `mflo $3`; a plain `(n >> 4) * 6` makes this cc1
 * pick the 3-operand R5900 `mult $3,$3,$4` alternative of its mulsi3 pattern
 * instead. Binding the product to LO selects the pattern's LO-output
 * alternative, and the read back out of LO is the `mflo`. The host build has
 * no LO register, so it gets an ordinary local.
 */
u8 func_00131760(u8 packed) {
#ifndef TARGET_NATIVE
    register u32 correction asm("lo");
#else
    u32 correction;
#endif

    correction = (packed >> 4) * 6;
    return packed - correction;
}

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00131850);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00131908);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00131A08);

extern s32 func_00131670(void);
extern s32 func_001316C8(void);
extern void func_00131A08(void *arg0, s32 value);

/**
 * func_00131A98(arg0): combine the two clock/territory getters and dispatch.
 * value = func_00131670() + (func_001316C8() * 0x3C - 0x21C); then tail-call
 * func_00131A08(arg0, value). Both getters are parameterless (they read their
 * own globals), so arg0 only survives across the calls (in $16) for the final
 * dispatch. Void sibling call (lever #7).
 *
 * Matched 2026-06-30: keys were (1) the two getters take NO args (declaring
 * them void stops ee-gcc reloading a0 before each call), (2) grouping the
 * constant fold as a temp `r2*0x3C - 0x21C` so -0x21C lands on the product not
 * on r1, (3) the void tail call compiles to `j func_00131A08`.
 */
void func_00131A98(void *arg0) {
    s32 r1 = func_00131670();
    s32 t = func_001316C8() * 0x3C - 0x21C;
    func_00131A08(arg0, r1 + t);
}

/* Inter-function padding at 0x131AE0..0x131AE7 — the two zero words retail
 * places between func_00131A98 and _start. They exist ONLY in the trailing
 * context of asm/usa/nonmatchings/cod/015180/func_00131A98.s, after
 * `endlabel func_00131A98` (whose stub header declares size 0x48, ending the
 * body at 0x131AE0). That file is not included at all once this unit supplies a
 * C body for the function, so without this directive nothing emits them and
 * everything from _start to the next 16-byte-aligned boundary lands 8 bytes low
 * — putting the retail ELF entry 0x00131AE8 two instructions inside _start.
 *
 * Guarded because the C-alt arm is placed in the 0xC00000 overlay, which is not
 * address-pinned to retail: there the padding buys nothing and only shifts
 * relocations.
 *
 * MEASURED here (t15866, USA, ref fd032de7 + this change, flat .rom from
 * tools/ee/build.sh's LMA link vs extracted/usa/SCUS_972.68.rom):
 *   without it — _start 0x00131AE0 (retail e_entry 0x00131AE8), snd_Pump
 *     0x00132020 (retail 0x00132028), 1353 differing bytes in the .cod band;
 *   with it    — _start 0x00131AE8, snd_Pump 0x00132028, 0 differing bytes in
 *     the .cod band.
 * Prior art: af962ded (USA, never gated) and 4a16558b (EU twin). */
#ifndef TARGET_NATIVE
__asm__(".word 0\n\t.word 0");
#endif

/* _start (0x131AE8): the EE ELF entry point — hand-written crt0, NOT compilable
 * from C and not given a portable #else (it IS the machine bootstrap: a C-only
 * boot still enters through this exact assembly before any C can run). It clears
 * all 32 GPRs (padduw), all 32 FPRs (mtc1), HI/LO/HI1/LO1/SA and the FCR, zeroes
 * the .bss span D_0013C080..D_001A7470 with 128-bit `sq` stores, sets up $gp
 * (D_001AEFF0) and $sp via the two SetMemoryMode/stack syscalls (0x3C/0x3D),
 * then calls _InitSys, func_0011AEA0(0), enables interrupts (ei) and calls
 * main(argc,argv) before tail-jumping to exit. Left as INCLUDE_ASM by nature. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", _start);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", _exitThunk);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00131CB0);


/* A 16-byte section header in a loaded overlay/WAD segment. The section's
 * payload immediately follows the header inline (at +0x10). */
typedef struct SectionHeader {
    void *dest;   /* 0x0 destination VA the payload is relocated to */
    s32   size;   /* 0x4 payload size in bytes */
    s32   pad8;   /* 0x8 (unused) */
    s32   key;    /* 0xC group id shared by a contiguous run of sections */
} SectionHeader;  /* 0x10 */

extern u8 *g_pLoadedSegment;

/* InstallLoadedOverlay (0x131CB8): relocate/install the freshly loaded overlay
 * segment pointed to by g_pLoadedSegment. The segment's first word is the byte
 * offset to the first section header; from there it walks consecutive 16-byte
 * section headers, copying each section's payload (which immediately follows its
 * header) to the header's dest VA — 64 bits at a time when dest, src and size
 * are all 8-byte aligned, else 32 bits at a time. It installs the run of
 * sections that share the first section's group id (key) and returns that id,
 * stopping at the first section whose id differs.
 *
 * NEAR-MISS, kept as INCLUDE_ASM for the matching build (#ifndef TARGET_NATIVE):
 * this is pure memory-relocation C (no hardware), and a faithful rotated
 * single-running-pointer rendering reproduces the original instruction-for-
 * instruction EXCEPT that ee-gcc's delay-slot filler emits the three payload-
 * alignment tests as ordinary `bne` (filling the delay from before the branch)
 * whereas the original uses annulling `bnel` branches that lazily recompute
 * `dest + size` only on the taken (4-byte) path. That is a filler decision, not
 * expressible from C source; -fno-gcse only makes it worse (frame spill) and the
 * unit is fixed at -O2 -G0, so the branch form cannot be coerced. ~50%
 * byte-match; honest effort exhausted. The portable #else below is the
 * functionally-faithful rendering (cmp-oracle'd against the .s on real R5900). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", InstallLoadedOverlay);
#else
s32 InstallLoadedOverlay(void) {
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
extern s32 InstallLoadedOverlay(void);

/* main (0x131D98): the game's top-level loop. Runs the one-shot init
 * func_0011FC48 once, then loops forever: call the current stage routine
 * (initially the level loader LoadLevelAndInitHealth), install the overlay
 * segment it loaded (InstallLoadedOverlay) and adopt that call's returned id as
 * the next stage routine to run, then pump the frame twice via func_0011AEA0
 * (modes 0 and 2). Never returns.
 *
 * NEAR-MISS, kept as INCLUDE_ASM for the matching build (#ifndef TARGET_NATIVE).
 * The body is otherwise byte-exact (the asm-label trick below suppresses the
 * `jal __main` ctor hook ee-gcc injects into any function literally named
 * `main`), but the original fills the first func_0011AEA0(0) call's delay slot
 * with the InstallLoadedOverlay-return capture (`move s0,v0`) and emits the
 * `a0=0` arg setup standalone, whereas ee-gcc unconditionally fills that delay
 * slot with the closest arg setup (`a0=0`) and emits the capture standalone.
 * That is a delay-slot filler tie-break — `move a0,0` is always RTL-emitted last
 * (it is part of the following call), so no C statement ordering can place the
 * capture after it. 98.8% byte-match; the portable #else below is the C entry. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", main);
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
        stage = (void (*)(void))InstallLoadedOverlay();
        func_0011AEA0(0);
        func_0011AEA0(2);
    }
}
#endif

// recovered splat-dropped code (epilogue-stump mis-split): raw words, byte-exact
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00131DE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", snd_Pump);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/021A98", func_00132210);

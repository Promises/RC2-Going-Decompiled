/*
 * cod/021A98 (0x121B18..0x122F0F): the part of the old cod/015180 unit after the
 * libgcc.a member _muldi3.o, which the build links from GCC's own source
 * (going-decompiled/libgcc/, RULING #8206; carve: task #879). It holds libgcc's
 * _pure.o, _udivdi3.o, _umoddi3.o and the head of dp-bit.o (__pack_d .. dpdiv)
 * as the ROM's asm plus this project's own C. It ends where the linked member
 * _fpcmp_parts_df.o (__fpcmp_parts_d, 0x122F10) begins; the rest of dp-bit.o
 * and everything after it is cod/022FA8 (carve: task #918).
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", __pack_d);
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", __unpack_d);
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", _fpadd_parts);
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", dpmul);
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/021A98", dpdiv);
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

/*
 * func_00122F10 = __fpcmp_parts_d (libgcc fp-bit.c `__fpcmp_parts`, dp-bit.o):
 * ordered comparison of two decomposed doubles (FpParts). On the EE there is no
 * game C for it: the address holds the libgcc.a member _fpcmp_parts_df.o, linked
 * from GCC's verbatim fp-bit.c (going-decompiled/libgcc/MEMBERS, RULING #8206,
 * task #918). The portable build links no libgcc member, so it gets this
 * behavioural stand-in instead: repack both operands with func_00122630 and
 * compare them as host doubles.
 *
 * Returns 1 if either operand is a NaN; otherwise -1, 0 or 1 for a < b, a == b,
 * a > b, with +0 and -0 equal. These are the libgcc routine's results for every
 * pair func_00122760 can produce (its only caller is func_00123028).
 */
#ifdef TARGET_NATIVE
s32 func_00122F10(FpParts *a, FpParts *b) {
    union { s64 bits; double value; } x, y;
    x.bits = func_00122630(a);
    y.bits = func_00122630(b);
    if (x.value != x.value || y.value != y.value) {
        return 1;
    }
    return (x.value > y.value) - (x.value < y.value);
}
#endif

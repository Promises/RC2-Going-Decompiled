/*
 * cod/022FA8 (0x123028..0x1313C7): the part of cod/021A98 after the libgcc.a
 * member _fpcmp_parts_df.o (__fpcmp_parts_d, 0x122F10..0x123023), which the
 * build links from GCC's own fp-bit.c (going-decompiled/libgcc/, RULING #8206)
 * followed by the ROM's zero pad word at 0x123024 (a `zeropad` subsegment;
 * carve: task #918). It starts with the rest of libgcc's dp-bit.o (dpcmp ..
 * dptofp), then fp-bit.o and frame.o as the ROM's asm plus this project's own
 * C, then the libsn/SDK code that follows, up to libm.a's e_sqrt.o and its CD
 * pad. libm.a's s_isnan.o (0x1313C8) and w_sqrt.o (0x131430) are linked from
 * newlib's own source (going-decompiled/libm/, task #1856); the code around
 * them is cod/031380 and cod/0314C0.
 */
#include "common.h"

/* Decomposed IEEE-754 double produced by func_00122760 (same layout as in
 * cod/021A98, where func_00122760 and func_00122630 live). */
typedef struct {
    s32 fpClass;   /* 0x00: classification tag (0=sNaN,1=qNaN,2=zero/subnormal,3=normal,4=inf) */
    s32 sign;      /* 0x04: sign bit */
    s32 exponent;  /* 0x08: unbiased exponent (bias 0x3FF) */
    s32 pad;       /* 0x0C */
    s64 mantissa;  /* 0x10: explicit mantissa (normal: frac<<8 | 1<<60) */
} FpParts;

extern void func_00122760(s64 *value, FpParts *out);
extern s64 func_00122630(FpParts *parts);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", litodp);
INCLUDE_ASM_ALIAS(func_00123078, litodp);
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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001232EC);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", __pack_f);
INCLUDE_ASM_ALIAS(func_001232F0, __pack_f);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", __unpack_f);
INCLUDE_ASM_ALIAS(func_00123400, __unpack_f);
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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00123490);

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

/* symbol_addrs names these two decode_uleb128 / decode_sleb128 (libgcc frame.o
 * statics), and the regenerated extract_cie_info / execute_cfa_insn /
 * __frame_state_for leaves call them by those names (task #1255). */
INCLUDE_ASM_ALIAS(decode_uleb128, func_00123530);
INCLUDE_ASM_ALIAS(decode_sleb128, func_00123578);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", fde_merge);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", end_fde_sort);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", count_fdes);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", add_fdes);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", frame_init);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", find_fde);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", extract_cie_info);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", execute_cfa_insn);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", __frame_state_for);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", fde_split);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00124414);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00124418);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", WaitGsPathsIdle);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00124540);

extern void func_0011A9A0(s32 id, void *handler, s32 obj);
extern s32 func_0011AC30(s32 obj);  /* syscall 0x41 DeleteSema */
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00124630);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001246D0);

/* libcdvd's semaphores and busy flag. Written from the RPC end-callback
 * func_00124630 (interrupt context) as well as from sceCdInit, func_001253A8,
 * func_00125620, func_00124818 and func_00124780 (ROM writer census), which
 * makes `volatile` a plausible original declaration. Its placement here is a
 * measured codegen device (RULING #8404), not derived from that census:
 * dropping `volatile` from D_001363A0, D_001363A8 or D_001363B0 alone leaves
 * func_00124780 3, 2 and 3 words off, while D_001363AC, which func_00124780
 * also stores, matches either way and is left plain. */
extern volatile s32 D_001363A0;
extern volatile s32 D_001363A8;
extern s32 D_001363AC;
extern volatile s32 D_001363B0;   /* CD command in flight */
extern s32 func_0011AC20(s32 *param);

/**
 * Create libcdvd's semaphores unless both D_001363A8 and D_001363AC already
 * hold one (-1 = none): D_001363A8 and D_001363AC as binary semaphores that
 * start signalled (max 1, initial 1) and D_001363A0 as one that starts
 * unsignalled (initial 0), all from one SemaParam on the stack
 * (func_0011AC20 = CreateSema); then clear the busy flag D_001363B0.
 *
 * The initial count (param[2]) is written before the maximum (param[1]):
 * the other way round cc1 issues the two stores swapped against the ROM.
 */
void func_00124780(void) {
    s32 param[6];
    if (D_001363A8 == -1 || D_001363AC == -1) {
        param[5] = 0;
        param[2] = 1;
        param[1] = 1;
        D_001363A8 = func_0011AC20(param);
        D_001363AC = func_0011AC20(param);
        param[2] = 0;
        D_001363A0 = func_0011AC20(param);
        D_001363B0 = 0;
    }
}

extern s32 D_00136394;  /* libcdvd initialised (non-zero once set up) */
/* libcdvd's pending end-callback state (-1 = none). Stored by func_00124630
 * (the RPC end-callback, interrupt context, which re-reads it right after its
 * own store), func_001253A8, func_00125620 and func_00124818 (ROM writer census
 * over asm/usa). `volatile` here is a measured codegen device (RULING #8404):
 * without it cc1 sinks the -1 store into the func_0011AC40 call's delay slot,
 * where the ROM stores it before loading the call's argument. */
extern volatile s32 D_001363D4;
extern s32 func_0011AC40(s32 sema);  /* syscall 0x42 SignalSema */
extern void func_0011CBC0(s32 index);

/**
 * libcdvd semaphore teardown. If the library was set up (D_00136394), cancel
 * any pending end-callback state (D_001363D4 = -1) and signal the completion
 * semaphore D_001363A0 so a waiter is released; then delete libcdvd's three
 * semaphores (D_001363A8, D_001363AC, D_001363A0; func_0011AC30 = DeleteSema)
 * and, with interrupts disabled (func_0011F5E0), clear the key word of entry
 * 0x80000012 (func_0011CBC0; a negative index selects the D_0013CF64 table),
 * restoring interrupts (func_0011F628) only if they were on.
 *
 * Needs func_0011AC30 declared value-returning, as the SDK declares
 * DeleteSema: with it `void`, cc1 frees $v0 and takes `lui $2` where the ROM
 * has `lui $3` for %hi(D_001363AC) (2/37 words, NOTE #9755).
 */
void func_00124818(void) {
    s32 wasEnabled;
    if (D_00136394 != 0) {
        D_001363D4 = -1;
        func_0011AC40(D_001363A0);
    }
    func_0011AC30(D_001363A8);
    func_0011AC30(D_001363AC);
    func_0011AC30(D_001363A0);
    wasEnabled = func_0011F5E0();
    func_0011CBC0((s32)0x80000012);
    if (wasEnabled != 0) {
        func_0011F628();
    }
}

/* func_001248B0: if the callback D_00141844 is installed and the suppression
 * flag D_001363A4 is clear, invoke the callback with the parameter D_00141848.
 * The body compiles byte-exact, but this is a splat mis-split: the per-function
 * .s (and the address-ordered unit listing) start the symbol 8 bytes early on a
 * trailing `addiu $29,$29,0x40; nop` epilogue fragment of the previous function
 * (func_001248F8 even references `func_001248B0 + 0x8` as the real entry). Can't
 * be matched at the unit level without a re-split. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001248B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001248F8);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00124970);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00124980);

extern s32 func_00124980(s32 arg);
extern s32 func_0011AC40(s32 sema);  /* syscall 0x42 SignalSema */
extern s32 D_00137550[];  /* libcdvd N-command RPC client */
extern u32 D_00136400[];  /* RPC receive buffer */

/**
 * libcdvd N-command query: func_00124980(2) first (0 means the call cannot
 * proceed: return 0), then issue RPC #0xE on the client D_00137550 with no send
 * data and a 4-byte reply into D_00136400 (func_0011D620 = sceSifCallRpc),
 * release the semaphore D_001363A8 and return the reply word, read through the
 * uncached 0x20000000 mirror. A failed RPC (negative) releases and returns 0.
 */
u32 func_00124AF0(void) {
    u32 value;
    if (func_00124980(2) == 0) {
        return 0;
    }
    if (func_0011D620(D_00137550, 0xE, 0, 0, 0, D_00136400, 4, 0, 0) < 0) {
        func_0011AC40(D_001363A8);
        return 0;
    }
    value = *(u32 *)((u32)D_00136400 | 0x20000000);
    func_0011AC40(D_001363A8);
    return value;
}

extern void *Kprintf(const char *format, ...);
extern s32 sceSifCheckStatRpc(s32 *rpc);
extern s32 D_00136390;     /* libcdvd debug level */
extern char D_0013B208[];  /* "N cmd wait\n" */

/**
 * sceCdSync: libcdvd N-command sync. With mode 0, print "N cmd wait" when the
 * debug level D_00136390 is positive, then wait until both the CD busy flag
 * D_001363B0 is clear and the N-command RPC client D_00137550 is idle
 * (sceSifCheckStatRpc), calling func_00124568(0x3C) between polls; return 0.
 * With any other mode, return 1 if either is still busy, else 0, without
 * waiting. The busy flag is re-read on every poll (D_001363B0 is volatile).
 */
s32 func_00124B88(s32 mode) {
    if (mode == 0) {
        if (D_00136390 > 0) {
            Kprintf(D_0013B208);
        }
        while (D_001363B0 != 0 || sceSifCheckStatRpc(D_00137550) != 0) {
            func_00124568(0x3C);
        }
        return 0;
    }
    if (D_001363B0 != 0 || sceSifCheckStatRpc(D_00137550) != 0) {
        return 1;
    }
    return 0;
}
extern char D_0013B218[];  /* "S cmd wait\n" */
extern s32 D_00137DC8[];   /* RPC client data of the S-command channel */

/**
 * libcdvd S-command sync (sceCdSyncS-shaped): with mode 0, print "S cmd wait"
 * when the debug level D_00136390 is positive, then poll the S-command RPC
 * client D_00137DC8 with sceSifCheckStatRpc, calling func_00124568(0x3C)
 * between polls until it is idle, and return 0. With any other mode, return
 * sceSifCheckStatRpc's busy status once without waiting.
 * The ROM's zero word after it (0x124C94) is alignment padding that the next
 * function's 8-byte alignment reproduces.
 */
s32 func_00124C28(s32 mode) {
    if (mode == 0) {
        if (D_00136390 > 0) {
            Kprintf(D_0013B218);
        }
        while (sceSifCheckStatRpc(D_00137DC8)) {
            func_00124568(0x3C);
        }
        return 0;
    }
    return sceSifCheckStatRpc(D_00137DC8);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00124C98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", sceCdInit);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", sceCdDiskReady);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", sceCdMmode);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001253A4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001253A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", QueryCdStatusOverRpc);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00125620);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", sceCdReadClock);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", sceGsResetGraph);

extern s32 D_00137E00;

/**
 * Accessor: return the address of the global D_00137E00.
 */
s32 *func_00125960(void) {
    return &D_00137E00;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012596C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", GsDefDispEnvNeedsOffsetFix);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", GetGsDisplayOffsets);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", BuildGsDispEnv);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00125D94);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00125D98);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00125E54);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", CalcGsZbufferBasePtr);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", BuildGsDrawEnvPacket);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126104);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126108);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", WaitVblankGetField);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126284);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126288);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012646C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126470);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", KickGifImageUpload);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012672C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126730);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126DBC);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126DC0);

extern u32 func_001272A8(volatile u32 *chcr);  /* defined below */
extern char D_0013B6F0[];  /* "libdma: sync timeout\n" */

/**
 * libdma channel sync: spin while the STR bit (0x100) of the channel's CHCR
 * register (the first word of the register block `chan`) is set. After 0x1000000
 * polls the countdown goes negative and every further poll prints "libdma: sync
 * timeout" and suspends the channel with func_001272A8; it never gives up.
 *
 * Spelled as a pre-decrement from 0x1000000: cc1 then loads the constant with
 * one `lui` and decrements it in the loop preheader, which is the ROM's
 * `lui 0x100` + `addiu -1` pair (splat reads the pair as a %hi/%lo of a symbol
 * "D_FFFFFF"). Starting from 0xFFFFFF instead gives `lui`+`ori` at the top.
 */
void sceDmaSyncChan(void *chan) {
    volatile u32 *chcr = chan;
    s32 count = 0x1000000;
    while (*chcr & 0x100) {
        if (--count < 0) {
            Kprintf(D_0013B6F0);
            func_001272A8(chan);
        }
    }
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126ED0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", ResetDmacChannels);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00127040);

/* func_00127218: 8 bytes of inter-function padding (a dead `sw $4,0($3); nop`)
 * between func_00127040 and the real func_00127220 — a splat mis-split, pinned
 * to size 0x8 in symbol_addrs so func_00127220 gets a clean .s. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00127218);

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00127288);

/**
 * Suspend a DMA channel (libdma, sceDmaPause-shaped; sceDmaSyncChan calls it
 * on every poll after its timeout). With interrupts disabled (func_0011F5E0),
 * hold the DMAC (set bit 16 of D_ENABLER 0x1000F520 through D_ENABLEW
 * 0x1000F590 unless already set), read D_CTRL 0x1000E000 (value unused),
 * clear the STR bit (0x100) of the channel's CHCR `chcr`, then write the
 * original D_ENABLER value back and restore interrupts (func_0011F628) if they
 * were on.
 * Returns the CHCR value read before STR was cleared.
 */
u32 func_001272A8(volatile u32 *chcr) {
    s32 wasEnabled = func_0011F5E0();
    u32 enabler = *(volatile u32 *)0x1000F520;
    u32 old;
    if (!(enabler & 0x10000)) {
        *(volatile u32 *)0x1000F590 = enabler | 0x10000;
    }
    *(volatile u32 *)0x1000E000;
    old = *chcr;
    *chcr = old & 0xFFFFFEFF;
    *(volatile u32 *)0x1000F590 = enabler;
    if (wasEnabled) {
        func_0011F628();
    }
    return old;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00127340);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", McInit);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00127500);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", McOpen);

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
extern s32 func_0011AC40(s32 sema);

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

/* McRead's DMA staging block D_00142000 as func_001277F8 reads it: two byte
 * runs the IOP returns outside the DMA'd body, each with its length and the
 * address it belongs at in the caller's buffer. */
typedef struct {
    s32 headSize;      /* 0x00: bytes held in head[] */
    s32 tailSize;      /* 0x04: bytes held in tail[] */
    u8 *headDst;       /* 0x08: where head[] is copied to */
    u8 *tailDst;       /* 0x0C: where tail[] is copied to */
    u8 head[0x40];     /* 0x10 */
    u8 tail[0x40];     /* 0x50 */
} McReadFragments;

/**
 * McRead RPC end-callback (libmc; registered by func_00127888 with the staging
 * buffer D_00142000 as its argument): read the staging block through the
 * uncached 0x20000000 mirror and copy its head and tail byte runs, one byte at
 * a time, to their destinations in the caller's buffer. A zero length skips
 * that run. The loop bound is re-read from the block on every iteration.
 */
void func_001277F8(void *staging) {
    McReadFragments *frag = (McReadFragments *)((u32)staging | 0x20000000);
    s32 i;
    if (frag->headSize != 0) {
        u8 *dst = frag->headDst;
        for (i = 0; i < frag->headSize; i++) {
            *dst++ = frag->head[i];
        }
    }
    if (frag->tailSize != 0) {
        u8 *dst = frag->tailDst;
        for (i = 0; i < frag->tailSize; i++) {
            *dst++ = frag->tail[i];
        }
    }
}

extern void sceSifWriteBackDCache(void *buf, s32 size);  /* cache writeback/invalidate */
extern u8   D_00142000[];                         /* libmc DMA staging buffer */

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", McWrite);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00127B18);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", McSync);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", McGetInfo);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00127E40);

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00127F90);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", McChdir);
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
extern s32 func_0011AC40(s32 sema);  /* syscall 0x42 SignalSema */
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", McMkDir);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", McGetEntSpace);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", sceDbcInit);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001284B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00128578);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001286C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001286C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001287A8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", sceDbcPortOpen);

/* func_00128A48: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real function begins at func_00128A50. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00128A48);

/* func_00128A50: real function recovered from the splat mis-split above (indexes
 * the 0x330-stride table D_00143640, dispatches to func_00128D58/DB0/E98 + a
 * memcpy). Boundary now correct; body not yet decompiled. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00128A50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00128B28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00128C18);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00128E98);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00128F48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00128F50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00128FD0);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001290BC);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001290E0);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00129120);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00129148);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00129160);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00129178);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001291A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00129218);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001292C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00129368);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00129410);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00129450);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001296A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00129DA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012A1C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012A3E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012A460);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012A4F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012A5B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012A680);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012A730);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012A7E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012A8E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012A9E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012AA80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012AB30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012AC10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012ACF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012ADD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012AEA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012AFC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012B0D8);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012B138);
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012B1C0);

/* func_0012B3C0(arg0): thin wrapper — forwards to func_0012C508(arg0, 3) and
 * returns its result. (The prior "sibling-call wall" note was wrong: ee-gcc 2.9
 * has NO sibling-call optimization, so this compiles to the original's jal +
 * real frame — byte-exact.) */
extern s32 func_0012C508(s32 arg0, s32 arg1);
s32 func_0012B3C0(s32 arg0) {
    return func_0012C508(arg0, 3);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012B3E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012B568);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012B678);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012B780);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012B8B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012BAA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012BB60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012C008);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012C090);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012C230);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", IpuWaitReady);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", IpuWaitCmdResult);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012C508);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012C680);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", IpuSkipBits);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", IpuGetBits);

extern void IpuWaitReady(s32 *ipu);
extern s32 IpuSkipBits(s32 *arg0, s32 arg1);
extern s32 func_0012C680(s32 *arg0, s32 arg1);

/**
 * Re-synchronise the IPU bitstream on a start code: wait for the IPU to go idle
 * (IpuWaitReady), skip the bits up to the next byte boundary (the low 3 bits of
 * IPU_BP 0x10002020 give the offset; skipped only when non-zero), then step
 * through the stream a byte at a time (IpuSkipBits(ipu, 8)) until the 24-bit
 * peek func_0012C680(ipu, 0x18) reads the start-code prefix 0x000001.
 */
void func_0012C9C8(s32 *ipu) {
    s32 pad;
    IpuWaitReady(ipu);
    pad = -(*(volatile u32 *)0x10002020 & 7) & 7;
    if (pad != 0) {
        IpuSkipBits(ipu, pad);
    }
    while (func_0012C680(ipu, 0x18) != 1) {
        IpuSkipBits(ipu, 8);
    }
}

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", IpuParseVideoStartCodes);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012CBC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012CC88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012CDB0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", IpuParseGopHeader);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012D100);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012D1C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012D2C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012D350);

/* The IPU decoder context shared by func_0012D420 and func_00130098 (fields
 * named only as far as those two show them). frames[] is the 3x4 table of
 * buffer pointers func_0012FA18 walks: rows at 0x1B8/0x1C8/0x1D8. */
typedef struct {
    u8  _pad0[0xF8];
    s32 state;            /* 0x0F8: 1 is advanced to 2 by func_0012D420 */
    u8  _padFC[0x118 - 0xFC];
    s32 count;            /* 0x118 */
    s32 _pad11C;
    s32 pending;          /* 0x120: a queued request for func_00130288 */
    u8  _pad124[0x150 - 0x124];
    s32 field150;         /* 0x150: 3 selects frame slot 3 instead of slot 0 */
    u8  _pad154[0x174 - 0x154];
    s32 mode;             /* 0x174: 3 = single-row path (func_0012DC50) */
    u8  _pad178[0x1B8 - 0x178];
    s32 frames[3][4];     /* 0x1B8 */
} IpuDecoder;

extern void func_0012DC50(IpuDecoder *dec, s32 frame, s32 last);
extern void func_0012DD60(IpuDecoder *dec, s32 frameA, s32 frameB, s32 last);

/**
 * When `enable` is set, finish a run of `count` items on decoder `dec`: in
 * mode 3 through func_0012DC50 with row 0's frame, otherwise through
 * func_0012DD60 with rows 1 and 2; slot 3 of each row is used when field150
 * is 3, slot 0 otherwise. Either way the last index (count - 1) is passed.
 * Then advance state 1 to 2.
 *
 * func_0012DD60 takes FOUR arguments here (it ignores the fourth); with three,
 * cc1 has no use for $a3 and the ROM's `addiu $7,$7,-1` disappears. The slot
 * is chosen into locals before ONE call per mode: two calls per mode give a
 * longer body with the calls duplicated.
 */
void func_0012D420(IpuDecoder *dec, s32 count, s32 enable) {
    if (enable != 0) {
        s32 a, b;
        if (dec->mode == 3) {
            if (dec->field150 == 3) {
                a = dec->frames[0][3];
            } else {
                a = dec->frames[0][0];
            }
            func_0012DC50(dec, a, count - 1);
        } else {
            if (dec->field150 == 3) {
                a = dec->frames[1][3];
                b = dec->frames[2][3];
            } else {
                a = dec->frames[1][0];
                b = dec->frames[2][0];
            }
            func_0012DD60(dec, a, b, count - 1);
        }
    }
    if (dec->state == 1) {
        dec->state = 2;
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012D4B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012D768);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012D808);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012DAC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012DC50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012DD60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012DF18);

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012E10C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012E110);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012E238);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012E378);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012E538);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012E608);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012E8E8);

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

/**
 * Skip `bytes` bytes of the bitstream reader `rec` (the record func_0012E890
 * initialises): clear the bit buffer (field 0x0) and its fill count (0x10),
 * add bytes*8 to the 64-bit consumed-bit total at 0x18, re-derive the byte
 * cursor (0xC) as the buffer base (0x8) plus total/8 — wrapping it back by the
 * span (0x28) once it reaches the end (0x24) — then refill via
 * func_0012E8E8(rec, 0).
 *
 * The two clears must come first in the source: written after the total is
 * updated, cc1 issues the total's `sd` ahead of them, where the ROM has it
 * after.
 */
void func_0012EA18(struct E890Rec *rec, s32 bytes) {
    rec->cleared0 = 0;
    rec->cleared10 = 0;
    rec->cleared18 += bytes * 8;
    rec->limit2 = rec->limit + (s32)(rec->cleared18 >> 3);
    if ((u32)rec->limit2 >= (u32)rec->end) {
        rec->limit2 -= rec->span;
    }
    func_0012E8E8((u64 *)rec, 0);
}

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012EA9C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012EAA0);

/* func_0012EB28: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real function begins at func_0012EB30. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012EB28);

/* func_0012EB30: real function recovered from the splat mis-split above (large
 * 0x150-frame routine iterating a 0x18-stride record list and dispatching via an
 * indirect call). Boundary now correct; body not yet decompiled. Left as
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012EB30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012EE28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012EF20);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012F0E0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012F738);

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012F948);

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012F998);

/**
 * Accessor: follow arg0->field_0x40 (arg0[0x10]) to its sub-object and return
 * that object's first word (base[0]); the sibling of func_0012F9B8, which tests
 * base[1]. The FMV display worker (FmvDisplayWorkerLoop, text/250080) stops on
 * a non-zero result, i.e. it reads this word as the host's end-of-stream flag.
 * The ROM's zero word after it (0x12F9B4) is alignment padding that the
 * next function's 8-byte alignment reproduces.
 */
s32 func_0012F9A8(s32 *arg0) {
    s32 *base = (s32 *)arg0[0x10];
    return base[0];
}

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012FA98);

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

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012FB10);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012FBF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012FD60);

extern s32 func_0012FEC0(s32 *obj, s32 arg1, s32 arg2);
extern s32 func_0012FD60(s32 *obj, s32 arg1, s32 arg2);

/**
 * Dispatch on the state word (offset 0x174) of obj's sub-object
 * (obj->field_0x40): when it equals 3 hand off to func_0012FD60, otherwise to
 * func_0012FEC0, forwarding arg1/arg2 unchanged; returns the chosen handler's
 * result.
 *
 * Both handlers read $a1/$a2 (they compare arg1 < arg2 and arg2 against -1), so
 * the dispatcher takes and forwards three arguments. With those two live, obj
 * has to be parked in $a3, which is the ROM's allocation; the one-argument
 * spelling frees $a1 for it and cannot match.
 */
s32 func_0012FE78(s32 *obj, s32 arg1, s32 arg2) {
    if (((s32 *)obj[0x10])[0x5D] != 3) {
        return func_0012FEC0(obj, arg1, arg2);
    }
    return func_0012FD60(obj, arg1, arg2);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012FEC0);

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

extern char D_0013BDC8[];

/**
 * Finalise the decoder's current transfer and clear its pending flag
 * (0x120). If a request is pending, report it through func_00130288(obj,
 * D_0013BDC8); otherwise finish the run of `count` (0x118) items with slot 1
 * of the frame table — in mode 3 via func_0012DC50 (row 0), else via
 * func_0012DD60 (rows 1 and 2) — passing the last index count - 1.
 *
 * Two things the in-tree "won't reproduce" note this replaces had missed:
 * func_0012DD60 is called with FOUR arguments (count - 1 is the fourth, in
 * $a3 — the `addiu $7,$6,-1` the ROM hoists into the mode test's delay slot,
 * which is why that test is a plain `bne`), and `count` is read once, before
 * the pending test (the ROM loads it in that branch's delay slot).
 */
void func_00130098(s32 *obj) {
    IpuDecoder *dec = (IpuDecoder *)obj;
    s32 count = dec->count;
    if (dec->pending != 0) {
        func_00130288((s32)obj, D_0013BDC8);
    } else if (dec->mode == 3) {
        func_0012DC50(dec, dec->frames[0][1], count - 1);
    } else {
        func_0012DD60(dec, dec->frames[1][1], dec->frames[2][1], count - 1);
    }
    dec->pending = 0;
}

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00130300);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00130428);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001306D0);

/**
 * func_001307B0(obj, cmd, madr): restart the IPU input DMA — run entry 2 of
 * the sub-object's table (func_0012FA98 on obj field 0x858, request index 2),
 * wait for the IPU (IpuWaitReady), reset IPU_CMD 0x10002000 to 0 and wait
 * again; then with interrupts disabled program channel 0x1000B400 (MADR
 * 0x1000B410 = madr & 0x0FFFFFFF, QWC 0x1000B420 = 4, CHCR 0x1000B400 =
 * 0x101), restoring interrupts if they were on; finally issue IPU command
 * `cmd` (func_0012C380), wait once more and run table entry 3.
 *
 * The request is a 0x20-byte buffer (as in func_0012FAE8): that is what
 * puts the saves at +0x20..+0x50 in the ROM's 0x60 frame. A 0x10-byte request
 * gives a 0x50 frame and is the "frame size only" residual the in-tree note
 * this replaces recorded.
 */
void func_001307B0(s32 *obj, u32 cmd, u32 madr) {
    s32 req[8];
    s32 wasEnabled;
    req[0] = 2;
    func_0012FA98((s32 *)obj[0x216], req);
    IpuWaitReady(obj);
    *(volatile u32 *)0x10002000 = 0;
    IpuWaitReady(obj);
    wasEnabled = func_0011F5E0();
    *(volatile u32 *)0x1000B410 = madr & 0x0FFFFFFF;
    *(volatile u32 *)0x1000B420 = 4;
    *(volatile u32 *)0x1000B400 = 0x101;
    if (wasEnabled) {
        func_0011F628();
    }
    func_0012C380(obj, cmd);
    IpuWaitReady(obj);
    req[0] = 3;
    func_0012FA98((s32 *)obj[0x216], req);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00130890);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00130A8C);

/**
 * Frameless tail-call thunk: forward the sub-object at arg0->field_0x40 + 0x4C
 * to func_00130C68. Void tail call → sibling-call-optimised to the original's
 * `j func_00130C68` with the +0x4C adjust in the delay slot.
 */
void func_00130AA0(void *arg0) {
    func_00130C68(*(u8 **)((u8 *)arg0 + 0x40) + 0x4C);
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00130AAC);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00130B80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00130C68);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00130E88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", e_sqrt);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001313C4);

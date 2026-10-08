/*
 * cod/022FA8 (0x123530..0x1313C7): the part of the old cod/021A98 after the
 * libgcc.a members dp-bit.o (0x122630..0x1232EB, task #1837) and fp-bit.o's
 * four residents (0x1232F0..0x12352F, task #1854, as two members around the
 * dead-strip debris in cod/023410), which the build links from the Cygnus EE
 * fp-bit.c (going-decompiled/libgcc/, RULING #8206 / #9753). Before them this
 * unit started at 0x123028, after _fpcmp_parts_df.o and its zeropad (task
 * #918). It holds frame.o as the ROM's asm plus this project's own C, then the
 * libsn/SDK code that follows, up to libm.a's e_sqrt.o and its CD pad.
 * libm.a's s_isnan.o (0x1313C8) and w_sqrt.o (0x131430) are linked from
 * newlib's own source (going-decompiled/libm/, task #1856); the code around
 * them is cod/031380 and cod/0314C0. The unit keeps its name, so no .s path
 * below 0x1313C8 moves. The dp-bit/fp-bit C below is only the TARGET_NATIVE
 * build's stand-ins.
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

/* dpcmp .. dptofp (0x123028..0x1232EB) and the CDCD pad after them: on the EE
 * this range is the linked libgcc.a member dp-bit.o (task #1837), so the C
 * below is only the TARGET_NATIVE build's stand-ins. */
#ifdef TARGET_NATIVE
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
#endif

/* __pack_f .. fptodp (0x1232F0..0x12352F): on the EE this range is the linked
 * libgcc.a members fp-bit-pack and fp-bit-make (task #1854) with the fp-bit.o
 * dead-strip debris between them (cod/023410), so the C below is only the
 * TARGET_NATIVE build's stand-ins. */
#ifdef TARGET_NATIVE
/* __make_dp (dp-bit.o, 0x123268): func_001234F0 below calls it by this name. */
extern s64 func_00123268(s32 fpClass, s32 sign, s32 exponent, s64 mantissa);

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
#endif

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
 * func_00124630 (interrupt context) as well as from sceCdInit, sceCdRead,
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
 * own store), sceCdRead, func_00125620 and func_00124818 (ROM writer census
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

extern s32 D_001363A4;  /* suppresses func_001248B0's callback while non-zero */
extern s32 D_001363BC;  /* set once the 0x80000012 handler is installed */
extern void func_001248B0(void);
extern void func_0011CB90(s32 index, s32 key, s32 value);

/**
 * Install libcdvd's SIF command handler for command 0x80000012: with the
 * callback suppressed (D_001363A4 = 1) and interrupts disabled
 * (func_0011F5E0), store the handler func_001248B0 and a zero argument into
 * that command's slot (func_0011CB90), restore interrupts only if they were
 * on, lift the suppression and mark the handler installed (D_001363BC).
 * Returns 1.
 *
 * The handler address is `func_001248B0 + 8`, exactly the ROM's relocation:
 * splat starts func_001248B0 eight bytes early on the previous function's
 * epilogue tail (see func_001248B0's comment), so +8 is its real entry.
 */
s32 func_001248F8(void) {
    s32 wasEnabled;
    D_001363A4 = 1;
    wasEnabled = func_0011F5E0();
    func_0011CB90((s32)0x80000012, (s32)((u8 *)func_001248B0 + 8), 0);
    if (wasEnabled != 0) {
        func_0011F628();
    }
    D_001363A4 = 0;
    D_001363BC = 1;
    return 1;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00124970);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00124980);

extern s32 func_00124980(s32 arg);
extern s32 func_0011AC40(s32 sema);  /* syscall 0x42 SignalSema */
extern s32 D_00137550[];  /* libcdvd N-command RPC client */
extern u32 D_00136400[];  /* RPC receive buffer */

/**
 * libcdvd N-command query: func_00124980(2) first (0 means the call cannot
 * proceed: return 0), then issue RPC #0xE on the client D_00137550 with no send
 * data and a 4-byte reply into D_00136400 (sceSifCallRpc),
 * release the semaphore D_001363A8 and return the reply word, read through the
 * uncached 0x20000000 mirror. A failed RPC (negative) releases and returns 0.
 */
u32 func_00124AF0(void) {
    u32 value;
    if (func_00124980(2) == 0) {
        return 0;
    }
    if (sceSifCallRpc(D_00137550, 0xE, 0, 0, 0, D_00136400, 4, 0, 0) < 0) {
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

extern s32 func_00124C98(s32 cmd);  /* S-command lock: 0 = busy */
extern u32 D_00137580[];  /* S-command RPC receive buffer */
extern void sceSifWriteBackDCache(void *buf, s32 size);  /* cache writeback/invalidate */
extern u32 D_001379C0[];  /* S-command RPC send buffer */

/**
 * sceCdMmode: set the libcdvd media mode. Take the S-command lock with
 * func_00124C98(0x22) (0 = busy: return 0), store `media` in the send buffer
 * D_001379C0, write it back from the data cache and call RPC #0x22 on the
 * S-command client D_00137DC8 with that 4-byte request and a 4-byte reply into
 * D_00137580. Release the semaphore D_001363AC and return the reply word (read
 * uncached); a failed RPC releases and returns 0.
 *
 * The buffer is held in a local taken before the lock call: the ROM forms its
 * address (and keeps the %hi in $s1 for the store) ahead of the first call.
 * Spelled with D_001379C0 at every use, cc1 forms it after the call and the
 * whole body shifts (48 of 49 words).
 */
s32 sceCdMmode(s32 media) {
    u32 *buf = D_001379C0;
    s32 value;
    if (func_00124C98(0x22) == 0) {
        return 0;
    }
    *buf = media;
    sceSifWriteBackDCache(buf, 4);
    if (sceSifCallRpc(D_00137DC8, 0x22, 0, buf, 4, D_00137580, 4, 0, 0) < 0) {
        func_0011AC40(D_001363AC);
        return 0;
    }
    value = *(s32 *)((u32)D_00137580 | 0x20000000);
    func_0011AC40(D_001363AC);
    return value;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001253A4);

/* sceCdRMode: the read mode sceCdRead takes. */
typedef struct {
    u8 tryCount;
    u8 spindleCtrl;
    u8 dataPattern;   /* 0 = 2048-byte sectors, 1 = 2328, 2 = 2340 */
    u8 _pad3;
} CdReadMode;

/* The sceCdRead request sent to the IOP (RPC #1 on D_00137550). */
typedef struct {
    u32 lbn;
    u32 sectors;
    void *buf;
    u8 tryCount;
    u8 spindleCtrl;
    u8 dataPattern;
    u8 _padF;
    void *readInfo;   /* D_00137480 */
    s32 *status;      /* D_00137540 */
} CdReadRequest;

/* libcdvd flags: bit 0 skips the ready check, bit 1 the buffer writeback.
 * `volatile` here is a CODEGEN DEVICE (RULING #8404), not a claim that the
 * flags change asynchronously: its only writer in the ROM is sceCdInit's
 * clear (`sw $0, %lo(D_001363B4)`, census of asm/usa). A volatile load is
 * not copied into a branch delay slot by reorg, which leaves the ROM's
 * `b; nop` before the second test in sceCdRead; plain, cc1 fills that
 * slot with the load (2 of 120 words differ). */
extern volatile s32 D_001363B4;
extern CdReadRequest D_00136480;
extern u8 D_00137480[0x90];
extern s32 D_00137540;
extern char D_0013B2E0[];      /* "call cdread cmd\n" */
extern char D_0013B2F8[];      /* "cdread end\n" */
extern void func_001246D0(void);

/**
 * libcdvd's sceCdRead (its debug messages, D_0013B2E0/D_0013B2F8, read
 * "call cdread cmd" and "cdread end"): read `sectors` sectors from `lbn`
 * into `buf` with read mode `mode`. Unless flag bit 0 is set, give up (0)
 * when the drive reports not-ready (func_00124AF0 returns 6); take the
 * N-command lock (func_00124980(4), 0 = busy: return 0); fill the request
 * D_00136480; size the buffer by the data pattern (2328- or 2340-byte
 * sectors, else 2048) and write it back from the cache unless flag bit 1
 * is set, along with the request, the read-info block and the status word;
 * mark the command in flight (D_001363D4, D_001363B0) and issue RPC #1
 * asynchronously with func_001246D0's real entry (+0x10) as the end
 * callback. A failed RPC clears the in-flight marks, releases the lock and
 * returns 0. Returns 1 once the read is queued.
 *
 * The data-pattern switch carries an explicit `case 0` on the default body:
 * without it cc1 drops the ROM's `slti` split of the decision tree (76 of
 * 120 words differ).
 */
s32 sceCdRead(u32 lbn, u32 sectors, void *buf, CdReadMode *mode) {
    CdReadRequest *req = &D_00136480;
    s32 size;
    if (!(D_001363B4 & 1) && func_00124AF0() == 6) {
        return 0;
    }
    if (func_00124980(4) == 0) {
        return 0;
    }
    req->lbn = lbn;
    req->sectors = sectors;
    req->buf = buf;
    req->tryCount = mode->tryCount;
    req->spindleCtrl = mode->spindleCtrl;
    req->dataPattern = mode->dataPattern;
    req->readInfo = D_00137480;
    req->status = &D_00137540;
    switch (mode->dataPattern) {
    case 1:
        size = sectors * 0x918;
        break;
    case 2:
        size = sectors * 0x924;
        break;
    case 0:
    default:
        size = sectors << 11;
        break;
    }
    D_00137540 = 0;
    if (!(D_001363B4 & 2)) {
        sceSifWriteBackDCache(buf, size);
    }
    sceSifWriteBackDCache(D_00137480, 0x90);
    sceSifWriteBackDCache(req, 0x18);
    sceSifWriteBackDCache(&D_00137540, 4);
    if (D_00136390 > 0) {
        Kprintf(D_0013B2E0);
    }
    D_001363D4 = 1;
    D_001363B0 = 1;
    if (func_0011D620(D_00137550, 1, 1, req, 0x18, 0, 0,
                      (s32)((u8 *)func_001246D0 + 0x10), D_00137480) < 0) {
        D_001363D4 = 0;
        D_001363B0 = 0;
        func_0011AC40(D_001363A8);
        return 0;
    }
    if (D_00136390 > 0) {
        Kprintf(D_0013B2F8);
    }
    return 1;
}

/**
 * libcdvd S-command 4 with no send data: take the S-command lock with
 * func_00124C98(3) (0 = busy: return -1), call RPC #4 on the S-command client
 * D_00137DC8 with a 4-byte reply into D_00137580 (sceSifCallRpc =
 * sceSifCallRpc), release the semaphore D_001363AC and return the reply word,
 * read through the uncached 0x20000000 mirror. A failed RPC releases and
 * returns -1.
 */
s32 QueryCdStatusOverRpc(void) {
    s32 value;
    if (func_00124C98(3) == 0) {
        return -1;
    }
    if (sceSifCallRpc(D_00137DC8, 4, 0, 0, 0, D_00137580, 4, 0, 0) < 0) {
        func_0011AC40(D_001363AC);
        return -1;
    }
    value = *(s32 *)((u32)D_00137580 | 0x20000000);
    func_0011AC40(D_001363AC);
    return value;
}

/**
 * libcdvd S-command with no send data: take the S-command lock with
 * func_00124C98(0x1E) (0 = busy: return 0), mark the pending end-callback state
 * D_001363D4 = 8 for the duration of RPC #0x16 on the S-command client
 * D_00137DC8 (4-byte reply into D_00137580), clear it again, release the
 * semaphore D_001363AC and return the reply word (read uncached). A failed RPC
 * releases, clears the state and returns 0.
 *
 * Both reads of D_001363AC go through a volatile lvalue. This is a CODEGEN
 * DEVICE under RULING #8404, not a claim that the semaphore id changes
 * asynchronously: its only writers in the ROM are func_00124780 (CreateSema)
 * and sceCdInit, neither in interrupt context. It keeps cc1 from moving the
 * argument load: plain on both reads, the failure path's load drops into the
 * SignalSema call's delay slot (the ROM leaves a nop there) and the body
 * shifts, 20 of 46 words; volatile on the failure path only, the success
 * path issues the load ahead of the `or` that forms the uncached address,
 * 2 of 46. Sibling S-commands (QueryCdStatusOverRpc, sceCdMmode) match with
 * plain reads.
 */
s32 func_00125620(void) {
    s32 value;
    if (func_00124C98(0x1E) == 0) {
        return 0;
    }
    D_001363D4 = 8;
    if (sceSifCallRpc(D_00137DC8, 0x16, 0, 0, 0, D_00137580, 4, 0, 0) < 0) {
        func_0011AC40(*(volatile s32 *)&D_001363AC);
        D_001363D4 = 0;
        return 0;
    }
    D_001363D4 = 0;
    value = *(s32 *)((u32)D_00137580 | 0x20000000);
    func_0011AC40(*(volatile s32 *)&D_001363AC);
    return value;
}

extern char D_0013B308[];  /* "Libcdvd call Clock read 1\n" */
extern char D_0013B328[];  /* "Libcdvd call Clock read 2\n" */

/* sceCdCLOCK: 8 bytes of BCD time (status, second, minute, hour, pad, day,
 * month, year), byte-aligned, so a copy is the unaligned ldl/ldr/sdl/sdr pair. */
typedef struct CdClock {
    u8 bytes[8];
} CdClock;

/**
 * sceCdReadClock: read the real-time clock into `clock`. Take the S-command
 * lock with func_00124C98(0xF) (0 = busy: return 0), print "Libcdvd call Clock
 * read 1" when the debug level D_00136390 is positive, call RPC #1 on the
 * S-command client D_00137DC8 with a 0x10-byte reply into D_00137580, copy the
 * 8 clock bytes at reply+4 (read uncached) to `clock`, print "... read 2",
 * release the semaphore D_001363AC and return the reply's first word. A failed
 * RPC releases and returns 0.
 */
s32 sceCdReadClock(CdClock *clock) {
    s32 value;
    if (func_00124C98(0xF) == 0) {
        return 0;
    }
    if (D_00136390 > 0) {
        Kprintf(D_0013B308);
    }
    if (sceSifCallRpc(D_00137DC8, 1, 0, 0, 0, D_00137580, 0x10, 0, 0) < 0) {
        func_0011AC40(D_001363AC);
        return 0;
    }
    *clock = *(CdClock *)((u32)&D_00137580[1] | 0x20000000);
    if (D_00136390 > 0) {
        Kprintf(D_0013B328);
    }
    value = *(s32 *)((u32)D_00137580 | 0x20000000);
    func_0011AC40(D_001363AC);
    return value;
}

/* libgraph's global parameter block (sceGsGParam), the object func_00125960
 * returns the address of. */
typedef struct GsGParam {
    s16 interMode;       /* 1 = interlaced */
    s16 outMode;
    s16 ffMode;          /* field/frame mode */
    s16 version;         /* GS revision; 1 selects the single-circuit display */
    void *vsyncFunc;     /* installed V-sync callback (0 = none) */
    s32 vsyncHandlerId;  /* its INTC handler id */
} GsGParam;

s32 *func_00125960(void);
extern s32 func_0011B588(s32 cause);                           /* DisableIntc */
extern s32 func_0011A920(s32 cause, s32 handlerId);            /* RemoveIntcHandler */
extern u64 func_0011AF60(u64 imr);                             /* GsPutIMR */
extern void func_0011A820(s16 interlace, s16 omode, s16 ffmode); /* SetGsCrt */

/**
 * libgraph's sceGsResetGraph: mode 0 resets the GS (CSR = 0x200), records
 * the interlace, output and field/frame modes and the GS revision (CSR bits
 * 16..23) in the GsGParam block, masks all GS interrupts (GsPutIMR 0xFF00),
 * removes any installed V-sync callback (the callback cleared before its
 * handler id; the other order swaps the two stores, 2 of 100 words), and
 * programs the CRTC (SetGsCrt). Mode 1 only flushes the GS (CSR = 0x100).
 * Mode 5 records the modes and revision without a reset and programs the
 * CRTC. Other modes do nothing. The ffMode field is stored as a flag.
 */
void sceGsResetGraph(s16 mode, s16 inter, s16 omode, s16 ffmode) {
    GsGParam *gp;
    switch (mode) {
    case 0:
        gp = (GsGParam *)func_00125960();
        *(volatile u64 *)0x12001000 = 0x200;
        gp->interMode = inter;
        gp->outMode = omode;
        gp->version = (*(volatile u64 *)0x12001000 >> 16) & 0xFF;
        func_0011AF60(0xFF00);
        gp->ffMode = ffmode != 0;
        if (gp->vsyncFunc != 0) {
            func_0011B588(2);
            func_0011A920(2, gp->vsyncHandlerId);
            gp->vsyncFunc = 0;
            gp->vsyncHandlerId = 0;
        }
        func_0011A820(inter & 1, omode & 0xFF, ffmode & 1);
        break;
    case 1:
        *(volatile u64 *)0x12001000 = 0x100;
        break;
    case 5:
        gp = (GsGParam *)func_00125960();
        gp->interMode = inter;
        gp->outMode = omode;
        gp->ffMode = ffmode != 0;
        gp->version = (*(volatile u64 *)0x12001000 >> 16) & 0xFF;
        func_0011A820(inter & 1, omode & 0xFF, ffmode & 1);
        break;
    }
}

extern s32 D_00137E00;

/**
 * Accessor: return the address of the global D_00137E00.
 */
s32 *func_00125960(void) {
    return &D_00137E00;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012596C);

extern s32 func_0011E0D8(const char *path, s32 flags, ...);  /* open  */
extern s32 func_0011E4E0(s32 fd, void *buf, s32 size);       /* read  */
extern s32 func_0011E360(s32 fd);                            /* close */
extern s32 func_00115E68(const char *s);                     /* atoi  */
extern char D_0013B348[];  /* "rom0:ROMVER" */

/**
 * Report whether this console's boot ROM is newer than 2001-06-08: open
 * "rom0:ROMVER" (returning -1 if that fails), read it one byte at a time into
 * a 256-byte buffer until a NUL or 256 bytes, close it, and return whether
 * the 8-digit date that ends 9 bytes before the stop point parses (atoi) to
 * more than 20010608.
 *
 * The counter and the cursor are initialised in one for-clause, counter
 * first: the ROM zeroes the counter in the open test's delay slot and points
 * the cursor at the buffer in the loop-entry branch's. Initialising the
 * cursor in a separate statement before the loop swaps the two (2 of 40).
 */
s32 GsDefDispEnvNeedsOffsetFix(void) {
    char buf[0x100];
    char *p;
    u32 len;
    s32 fd;
    fd = func_0011E0D8(D_0013B348, 1);
    if (fd < 0) {
        return -1;
    }
    for (len = 0, p = buf; len < 0x100; len++) {
        func_0011E4E0(fd, p, 1);
        if (*p++ == 0) {
            break;
        }
    }
    func_0011E360(fd);
    return func_00115E68(&buf[len - 9]) > 20010608;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", GetGsDisplayOffsets);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", BuildGsDispEnv);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00125D94);

/**
 * Put a display environment (sceGsPutDispEnv-shaped): write the five 64-bit GS
 * privileged-register images in `env` to the hardware. PMODE (0x12000000)
 * always; then, if GsGParam.version is 1, DISPFB1/DISPLAY1 (0x12000070/80) from
 * env[2]/env[3] and env[4] to 0x120000C0 (EXTDATA in the GS register map);
 * otherwise SMODE2 (0x12000020) from env[1], DISPFB2/DISPLAY2 (0x12000090/A0)
 * from env[2]/env[3] and BGCOLOR (0x120000E0) from env[4]. env[1] is unused on
 * the version-1 path.
 */
void func_00125D98(u64 *env) {
    GsGParam *gp = (GsGParam *)func_00125960();
    if (gp->version == 1) {
        *(volatile u64 *)0x12000000 = env[0];
        *(volatile u64 *)0x12000070 = env[2];
        *(volatile u64 *)0x12000080 = env[3];
        *(volatile u64 *)0x120000C0 = env[4];
    } else {
        *(volatile u64 *)0x12000000 = env[0];
        *(volatile u64 *)0x12000020 = env[1];
        *(volatile u64 *)0x12000090 = env[2];
        *(volatile u64 *)0x120000A0 = env[3];
        *(volatile u64 *)0x120000E0 = env[4];
    }
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00125E54);

/**
 * Size of a frame buffer in GS pages (sceGszbufaddr-shaped; the symbol name
 * reads it as the Z buffer's base): width in 64-pixel pages, height in
 * 64-line pages when bit 1 of `psm` is set or 32-line pages otherwise, both
 * rounded up. The page count is returned as is when the GsGParam's first
 * and third halfwords (interMode, ffMode) read exactly 1 and 0, i.e.
 * interlaced field mode, and doubled otherwise.
 *
 * That test is one 64-bit load masked with 0x0000FFFF0000FFFF, as in the ROM;
 * two halfword compares would not produce it.
 */
s16 CalcGsZbufferBasePtr(s16 psm, s16 width, s16 height) {
    GsGParam *gp = (GsGParam *)func_00125960();
    s16 fbw = (width + 63) / 64;
    s16 fbh;
    if (psm & 2) {
        fbh = (height + 63) / 64;
    } else {
        fbh = (height + 31) / 32;
    }
    if ((*(u64 *)gp & 0x0000FFFF0000FFFF) == 1) {
        return fbw * fbh;
    }
    return fbw * fbh * 2;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", BuildGsDrawEnvPacket);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126104);

extern char D_0013B390[];  /* "sceGsPutDrawEnv: DMA Ch.2 does not terminate\r\n" */

/**
 * sceGsPutDrawEnv: send a drawing-environment GIF packet down DMA channel 2
 * (GIF). Wait for the channel's STR bit (0x100 of D2_CHCR, 0x1000A000) to
 * clear; after 0x1000000 polls print the timeout message and return -1. Then
 * set D2_QWC (0x1000A020) to the packet's NLOOP (bits 0..14 of its first
 * doubleword) + 1, D2_MADR (0x1000A010) to the packet's physical address
 * (bit 31 set for a scratchpad address, 0x7xxxxxxx), and start the channel in
 * normal mode (CHCR = 0x101). Returns 0.
 *
 * The poll count is tested before it is incremented (`count++ > 0x1000000`):
 * the ROM copies the count, then bumps it in the timeout branch's delay slot.
 */
s32 func_00126108(u64 *packet) {
    u32 count = 0;
    while (*(volatile u32 *)0x1000A000 & 0x100) {
        if (count++ > 0x1000000) {
            Kprintf(D_0013B390);
            return -1;
        }
    }
    *(volatile u32 *)0x1000A020 = (s32)(*packet & 0x7FFF) + 1;
    if (((u32)packet & 0x70000000) == 0x70000000) {
        *(volatile u32 *)0x1000A010 = ((u32)packet & 0x0FFFFFFF) | 0x80000000;
    } else {
        *(volatile u32 *)0x1000A010 = (u32)packet & 0x0FFFFFFF;
    }
    *(volatile u32 *)0x1000A000 = 0x101;
    return 0;
}

extern void WaitVblankStartIntc(void);
extern s64 func_0011B140(void);  /* returns the GS CSR value */

/**
 * Wait for the next V-blank and return the field being displayed
 * (sceGsSyncV-shaped). With no V-sync callback installed, wait with
 * WaitVblankStartIntc and read the FIELD bit (13) of the GS CSR (0x12001000)
 * directly; with one installed, func_0011B140 supplies the CSR value instead.
 * In non-interlaced mode the field is always 1. `mode` (sceGsSyncV's
 * argument, which every caller passes as 0) is not read.
 */
s32 WaitVblankGetField(s32 mode) {
    GsGParam *gp = (GsGParam *)func_00125960();
    s64 field;
    if (gp->vsyncFunc == 0) {
        WaitVblankStartIntc();
        if (gp->interMode == 1) {
            return (*(volatile u64 *)0x12001000 >> 13) & 1;
        }
    } else {
        field = (func_0011B140() >> 13) & 1;
        if (gp->interMode == 1) {
            return field;
        }
    }
    return 1;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126284);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126288);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012646C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126470);

extern char D_0013B4E0[];

/**
 * libgraph's sceGsExecLoadImage (its timeout message, D_0013B4E0, reads
 * "sceGsExecLoadImage: DMA Ch.2 does not terminate"): send a 6-quadword
 * load-image setup packet (the GIF tags and BITBLTBUF/TRXPOS/TRXREG/TRXDIR
 * that func_00126288 builds) over GIF DMA channel 2 (0x1000A000), then the
 * image data itself, whose quadword count is the NLOOP field of the image
 * GIF tag at packet[10]. Addresses in scratchpad (0x70000000) are sent with
 * the SPR bit (0x80000000), others masked to 28 bits.
 *
 * Returns 0, or -1 after printing the message when channel 2 is still busy
 * after 0x1000000 polls (one count across both waits). Each wait carries its
 * own `Kprintf; return -1` as in func_00126108; one shared timeout label puts
 * the merged block at the end, where the ROM has it between the two arms of
 * the second address test (22/95 words differ).
 */
s32 KickGifImageUpload(u64 *packet, void *image) {
    u32 count = 0;
    while (*(volatile u32 *)0x1000A000 & 0x100) {
        if (count++ > 0x1000000) {
            Kprintf(D_0013B4E0);
            return -1;
        }
    }
    *(volatile u32 *)0x1000A020 = 6;
    if (((u32)packet & 0x70000000) == 0x70000000) {
        *(volatile u32 *)0x1000A010 = ((u32)packet & 0x0FFFFFFF) | 0x80000000;
    } else {
        *(volatile u32 *)0x1000A010 = (u32)packet & 0x0FFFFFFF;
    }
    *(volatile u32 *)0x1000A000 = 0x101;
    while (*(volatile u32 *)0x1000A000 & 0x100) {
        if (count++ > 0x1000000) {
            Kprintf(D_0013B4E0);
            return -1;
        }
    }
    *(volatile u32 *)0x1000A020 = (s32)(packet[10] & 0x7FFF);
    if (((u32)image & 0x70000000) == 0x70000000) {
        *(volatile u32 *)0x1000A010 = ((u32)image & 0x0FFFFFFF) | 0x80000000;
    } else {
        *(volatile u32 *)0x1000A010 = (u32)image & 0x0FFFFFFF;
    }
    *(volatile u32 *)0x1000A000 = 0x101;
    return 0;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012672C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126730);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00126DBC);

extern s32 func_0011B588(s32 cause);                           /* DisableIntc */
extern s32 func_0011A920(s32 cause, s32 handlerId);            /* RemoveIntcHandler */
extern s32 func_0011A900(s32 cause, void *handler, s32 next);  /* AddIntcHandler */
extern s32 func_0011B5F0(s32 cause);                           /* EnableIntc */

/**
 * Install `handler` as the V-sync (INTC cause 2) callback, or remove the
 * current one when `handler` is 0 (sceGsSyncVCallback-shaped); returns the
 * previous callback. Removing disables the interrupt, removes the handler and
 * clears both GsGParam fields; installing first removes any previous handler,
 * then adds `handler` and enables the interrupt.
 *
 * On removal the handler id is cleared before the callback: the other order
 * swaps the two stores (2 of 40).
 */
s32 func_00126DC0(void *handler) {
    GsGParam *gp = (GsGParam *)func_00125960();
    void *old = gp->vsyncFunc;
    if (handler == 0) {
        func_0011B588(2);
        func_0011A920(2, gp->vsyncHandlerId);
        gp->vsyncHandlerId = 0;
        gp->vsyncFunc = 0;
    } else {
        if (old != 0) {
            func_0011B588(2);
            func_0011A920(2, gp->vsyncHandlerId);
        }
        gp->vsyncFunc = handler;
        gp->vsyncHandlerId = func_0011A900(2, handler, -1);
        func_0011B5F0(2);
    }
    return (s32)old;
}

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

extern s32 D_0013B708[];   /* per-channel "present" flags, parallel to D_00137E30 */
extern s32 func_00127040(u8 *buf);  /* returns 0 (ROM: `jr $31; daddu $2,$0,$0`) */

/* One EE DMA channel's register block (only the words ResetDmacChannels clears). */
typedef struct {
    u32 chcr;                  /* 0x00 */
    u32 _pad04[3];
    u32 madr;                  /* 0x10 */
    u32 _pad14[7];
    u32 tadr;                  /* 0x30 */
    u32 _pad34[3];
    u32 asr0;                  /* 0x40 */
    u32 _pad44[3];
    u32 asr1;                  /* 0x50 */
    u32 _pad54[11];
    u32 sadr;                  /* 0x80 */
} DmaChannelRegs;

/**
 * ResetDmacChannels (libdma sceDmaReset): note whether the DMAC is enabled
 * (D_CTRL 0x1000E000 bit 0), clear SADR, CHCR, TADR, MADR, ASR1 and ASR0 of
 * every channel whose D_0013B708 flag is set (register blocks from the
 * D_00137E30 table), clear the D_STAT interrupt status bits and keep only its
 * mask half, then rebuild the DMAC control state from a zeroed 20-byte
 * descriptor (func_00126F00 = clear, func_00127040 = apply). With mode 1 the
 * DMAC enable bit is set again. Returns the enable bit as it was on entry.
 *
 * func_00127040 must be declared value-returning: as `void` cc1 puts the
 * `mode == 1` constant in $v0 instead of the ROM's $v1 (2/56 words).
 */
s32 ResetDmacChannels(s32 mode) {
    u8 desc[0x20];
    s32 wasEnabled;
    s32 i;
    wasEnabled = *(volatile u32 *)0x1000E000 & 1;
    for (i = 0; i < 10; i++) {
        if (D_0013B708[i] != 0) {
            volatile DmaChannelRegs *ch = (volatile DmaChannelRegs *)D_00137E30[i];
            ch->sadr = 0;
            ch->chcr = 0;
            ch->tadr = 0;
            ch->madr = 0;
            ch->asr1 = 0;
            ch->asr0 = 0;
        }
    }
    *(volatile u32 *)0x1000E010 = 0xFF1F;
    *(volatile u32 *)0x1000E010 = *(volatile u32 *)0x1000E010 & 0xFF1F0000;
    func_00126F00(desc, 0x14);
    func_00127040(desc);
    if (mode == 1) {
        *(volatile u32 *)0x1000E000 = *(volatile u32 *)0x1000E000 | 1;
    }
    return wasEnabled;
}

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

extern u8   g_mcRpcClient[];   /* libmc RPC client block (init flag @+0x24) */
extern s32  g_mcMutexSema;     /* libmc mutex/semaphore handle */
extern s32  D_00141B80;        /* libmc RPC send-buffer (fd marshalled @+0) */
extern u32  g_mcRpcResult;     /* libmc RPC receive-buffer (result code) */
extern s32  func_0011AC70(s32 sema);
extern s32 func_0011AC40(s32 sema);

/* libmc's name-carrying RPC request (McOpen, McMkDir = Delete, func_00127E48 = GetDir):
 * 0x414 bytes, the largest send buffer in the family. */
typedef struct {
    s32  port;                 /* 0x000 */
    s32  slot;                 /* 0x004 */
    s32  mode;                 /* 0x008: open flags / GetDir mode; 0 for Delete */
    s32  maxent;               /* 0x00C: GetDir only */
    void *table;               /* 0x010: GetDir only */
    char name[0x400];          /* 0x014: strncpy'd, 1023 chars + forced NUL */
} McNameRequest;
extern McNameRequest D_00141BB0;   /* libmc name-request send buffer */
extern s32 g_mcPendingCmd;    /* 0x137E68: RPC function number of the call in flight */

/**
 * McOpen (libmc sceMcOpen): open `name` on memory card `port`/`slot` with the
 * open flags `mode`, as libmc RPC #2. Returns -100 if the RPC client is not
 * bound, -200 if the libmc mutex cannot be taken (PollSema), -210 for a null
 * or empty name; else the sceSifCallRpc result (0 = submitted). On success
 * the mutex stays held and the pending command (2) is recorded for McSync's
 * completion path; a failed submission releases it here.
 *
 * The request is written through the struct global itself, name first: cc1
 * then addresses every field relative to the name's address (`name - 0x14`),
 * which is the ROM's form. Through a pointer to the buffer it forms the base
 * first instead (39/73 words).
 */
s32 McOpen(s32 port, s32 slot, const char *name, s32 mode) {
    u8 *client = g_mcRpcClient;
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
    strncpy(D_00141BB0.name, name, 0x3FF);
    D_00141BB0.port = port;
    D_00141BB0.mode = mode;
    D_00141BB0.slot = slot;
    D_00141BB0.name[0x3FF] = 0;
    r = sceSifCallRpc(client, 2, 1, &D_00141BB0, 0x414, &g_mcRpcResult, 4, 0, 0);
    if (r == 0) {
        g_mcPendingCmd = 2;
    } else {
        func_0011AC40(g_mcMutexSema);
    }
    return r;
}
extern s32 D_00137E68;

/**
 * Allocate/acquire via McOpen(arg0, arg1, arg2, 0x40). On failure (NULL
 * result) record error code 0xB in D_00137E68. Returns the McOpen result.
 */
s32 func_00127630(s32 arg0, s32 arg1, s32 arg2) {
    s32 result = McOpen(arg0, arg1, (const char *)arg2, 0x40);
    if (result == 0) {
        D_00137E68 = 0xB;
    }
    return result;
}


/**
 * func_00127668 = McClose (libmc): close the memory-card file descriptor `fd`.
 * Guards on the RPC client being initialised (returns -0x64 / -100 if not), takes
 * the libmc mutex (returns -0xC8 / -200 if that fails), marshals fd into the send
 * buffer and issues RPC #3 (sceSifCallRpc). On RPC success
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
    r = sceSifCallRpc(client, 3, 1, &D_00141B80, 0x30, &g_mcRpcResult, 4, 0, 0);
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
    r = sceSifCallRpc(client, 4, 1, &D_00141B80, 0x30, &g_mcRpcResult, 4, 0, 0);
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
    r = sceSifCallRpc(client, 5, 1, &D_00141B80, 0x30, &g_mcRpcResult, 4,
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

extern s32 sceSifCheckStatRpc(s32 *rpc);

/**
 * McSync (libmc): poll (mode != 0) or wait for (mode 0) the libmc RPC call in
 * flight. Returns -1 if no call is pending, 0 if it is still running, else 1:
 * then the pending command is cleared, the call's result (g_mcRpcResult) is
 * stored through `result` when non-null, and the libmc mutex is released
 * (func_0011AC40 = SignalSema). `cmd`, when non-null, receives the pending
 * command number in both the running and the finished case. Waiting re-checks
 * the RPC status every 60 ms (McDelayMillis).
 *
 * The finished flag reuses the status variable (`busy = busy == 0`); a
 * separate `done` local and an early `return` give a different body (55/56).
 */
s32 McSync(s32 mode, s32 *cmd, s32 *result) {
    s32 busy;
    if (g_mcPendingCmd == 0) {
        return -1;
    }
    busy = sceSifCheckStatRpc((s32 *)g_mcRpcClient);
    if (mode == 0 && busy != 0) {
        while (sceSifCheckStatRpc((s32 *)g_mcRpcClient) != 0) {
            McDelayMillis(60);
        }
        busy = 0;
    }
    busy = busy == 0;
    if (cmd != 0) {
        *cmd = g_mcPendingCmd;
    }
    if (busy) {
        g_mcPendingCmd = 0;
        if (result != 0) {
            *result = g_mcRpcResult;
        }
        func_0011AC40(g_mcMutexSema);
    }
    return busy;
}

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

    req = (u8 *)&D_00141BB0;
    *(s32 *)(req + 0x00) = port;
    *(s32 *)(req + 0x04) = slot;
    *(s32 *)(req + 0x08) = mode;
    *(s32 *)(req + 0x0C) = maxent;
    *(void **)(req + 0x10) = table;
    strncpy((char *)(req + 0x14), name, 0x3FF); /* 0x115AC0, linked from newlib's r5900 strncpy.S (task #1884) */
    req[0x413] = 0;

    if (maxent >= 0) {
        sceSifWriteBackDCache(table, maxent << 6);
    }

    r = sceSifCallRpc(client, 13, 1, req, 0x414, &g_mcRpcResult, 4, 0, 0);
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
/* sceSifCallRpc is already declared at file scope earlier in
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

    rc = sceSifCallRpc(g_mcRpcClient, 16, 1,
                       g_mcRpcRequest, 48, &g_mcRpcResult, 4, 0, 0);
    if (rc != 0) {
        func_0011AC40(g_mcMutexSema);      /* no completion is coming — release */
    } else {
        g_mcPendingCmd = 16;               /* held; the completion path releases */
    }
    return rc;
}
#endif

/**
 * McMkDir is libmc's sceMcDelete, not mkdir (FACT #5885: RPC #15 is
 * sceMcFuncNoDelete in the vendored libmc.h table; the real mkdir is
 * func_00127630, McOpen with mode 0x40). The splat-era name is kept because
 * the function is matched. Deletes `name` on memory card `port`/`slot`
 * through the same 0x414-byte name request as McOpen (mode word 0). Same
 * guards and return codes as McOpen (-100 / -200 / -210, else the
 * sceSifCallRpc result); on success the mutex stays held with the pending
 * command 15 recorded for McSync.
 */
s32 McMkDir(s32 port, s32 slot, const char *name) {
    u8 *client = g_mcRpcClient;
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
    strncpy(D_00141BB0.name, name, 0x3FF);
    D_00141BB0.port = port;
    D_00141BB0.slot = slot;
    D_00141BB0.name[0x3FF] = 0;
    D_00141BB0.mode = 0;
    r = sceSifCallRpc(client, 0xF, 1, &D_00141BB0, 0x414, &g_mcRpcResult, 4, 0, 0);
    if (r == 0) {
        g_mcPendingCmd = 0xF;
    } else {
        func_0011AC40(g_mcMutexSema);
    }
    return r;
}

extern u8  g_mcRpcRequest[];  /* 0x141B80: the shared 48-byte libmc request block */
extern s32 g_mcPendingCmd;    /* 0x137E68: RPC function number of the call in flight */

/* The libmc request block's argument words as McGetEntSpace fills them. */
typedef struct McPortSlotRequest {
    s32 unk0;
    s32 port;  /* +0x4 */
    s32 slot;  /* +0x8 */
} McPortSlotRequest;

/**
 * libmc sceMcUnformat (McGetEntSpace is the build label, kept because matched
 * names are frozen): unformat the card in `port`/`slot`. RPC #0x11 is
 * sceMcFuncNoUnformat in the vendored libmc.h (FACT #5885). Same RPC-wrapper
 * shape as McClose (func_00127668): returns -0x64 (-100, sceMcErrUnbind) if
 * the RPC client is not bound and -0xC8 (-200, sceMcErrSemapho) if the libmc
 * mutex cannot be taken, otherwise stores port and slot in the shared request
 * block and issues RPC #0x11. On RPC success records 0x11 as the pending
 * command (the completion path releases the mutex); on failure releases the
 * mutex itself. Returns the RPC result.
 *
 * Two parameters, as libmc.h declares sceMcUnformat(int port, int slot); the
 * ROM's only caller (SaveLoadStateMachine) passes two. Its call site is gated
 * on the card's +0x14 word being 0, which FACT #5885 records but does not
 * explain. The request is written through a typed pointer: spelled as byte
 * offsets from g_mcRpcRequest, cc1 rebases the two stores off a `+4` address
 * and the body is 7 of 52 words off.
 */
s32 McGetEntSpace(s32 port, s32 slot) {
    u8 *client = g_mcRpcClient;
    McPortSlotRequest *req;
    s32 r;
    if (*(s32 *)(client + 0x24) == 0) {
        return -0x64;
    }
    if (func_0011AC70(g_mcMutexSema) < 0) {
        return -0xC8;
    }
    req = (McPortSlotRequest *)g_mcRpcRequest;
    req->port = port;
    req->slot = slot;
    r = sceSifCallRpc(client, 0x11, 1, req, 0x30, &g_mcRpcResult, 4, 0, 0);
    if (r == 0) {
        g_mcPendingCmd = 0x11;
    } else {
        func_0011AC40(g_mcMutexSema);
    }
    return r;
}

extern s32 D_00143108;
/* Array-typed: func_00128440 stores arg0 at D_00143180[1] and the ARRAY
 * extern is required for its match — the scalar (&D)[1] idiom anchors the
 * %hi at D+4 and flips the branch to bgezl. func_00128250 shares this decl
 * and stays byte-identical under the array phrasing (tester-verified USA). */
extern s32 D_00143180[];

/**
 * Initialise the D_00143180 subsystem by calling sceSifCallRpc with the config
 * block at &D_00143108, mode 0x80000963, two 0x400-sized buffers both pointing
 * at D_00143180, and zeroed trailing arguments; returns the resulting handle
 * stored in D_00143180[0].
 */
s32 func_00128250(void) {
    sceSifCallRpc(&D_00143108, 0x80000963, 0, D_00143180, 0x400,
                  D_00143180, 0x400, 0, 0);
    return D_00143180[0];
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", sceDbcInit);

extern char D_0013B868[];
extern void func_00128898(const char *fmt, ...);

/**
 * func_00128440(arg0): open the D_00143180 subsystem in mode 0x80000904 with
 * arg0 stored at D_00143180[1], via sceSifCallRpc; on failure log D_0013B868
 * (func_00128898) and return 0, else return the handle D_00143180[0].
 *
 * Match note: the ARRAY extern (not the scalar (&D)[1] idiom) is what lets the
 * %hi(D_00143180) CSE into callee-saved $16 across the call and lands the
 * result load in the bgez delay slot — byte-exact as plain faithful C.
 */
s32 func_00128440(s32 arg0) {
    D_00143180[1] = arg0;
    if (sceSifCallRpc(&D_00143108, 0x80000904, 0, D_00143180, 0x400,
                      D_00143180, 0x400, 0, 0) < 0) {
        func_00128898(D_0013B868);
        return 0;
    }
    return D_00143180[0];
}

struct ResSubObj;  /* the DMA'd device-frame pair, defined below */
extern char D_0013B888[];  /* "sceDbcCreateSocket: rpc error\n" */

/* A libdbc socket's 16-byte device name, wrapped so it copies as one block. */
typedef struct DbcName {
    u8 b[16];
} DbcName;

/* What sceDbcCreateSocket sends: the caller's socket parameters with the
 * library's own `type` word inserted after `option` (sceDbcPortOpen sets it). */
typedef struct DbcSocketParam {
    u32 option;
    s32 type;
    s32 port;
    s32 slot;
    s32 number;
    DbcName name;
} DbcSocketParam;

/* The socket-creation view of the shared RPC buffer D_00143180: the parameter
 * block, the server's result (the socket index) at +0x24, then the two
 * receive buffers the IOP DMAs device frames into. */
typedef struct DbcSocketRequest {
    u32 option;
    s32 type;
    s32 port;
    s32 slot;
    s32 number;
    DbcName name;
    s32 result;
    struct ResSubObj *buf0;
    struct ResSubObj *buf1;
} DbcSocketRequest;

/**
 * libdbc sceDbcCreateSocket (named by its ROM error string, D_0013B888): copy
 * `param` into the RPC buffer together with the two DMA receive buffers and
 * ask the IOP server for a socket (RPC 0x80000901, blocking, client
 * D_00143108). Returns the socket index from the reply, or 0 after printing
 * the error on an RPC failure. The name is copied a byte at a time, as the ROM
 * does.
 *
 * The buffer arguments are pointers: typed `s32` they change the copy loop's
 * register allocation (9 of 50 words); field names and declaration order are
 * inert.
 */
s32 func_001284B0(DbcSocketParam *param, struct ResSubObj *buf0,
                  struct ResSubObj *buf1) {
    DbcSocketRequest *req = (DbcSocketRequest *)D_00143180;
    s32 i;
    req->buf0 = buf0;
    req->buf1 = buf1;
    req->option = param->option;
    req->type = param->type;
    req->port = param->port;
    req->slot = param->slot;
    req->number = param->number;
    for (i = 0; i < 16; i++) {
        req->name.b[i] = param->name.b[i];
    }
    if (sceSifCallRpc(&D_00143108, 0x80000901, 0, D_00143180, 0x400,
                      D_00143180, 0x400, 0, 0) < 0) {
        func_00128898(D_0013B888);
        return 0;
    }
    return req->result;
}

extern void func_0011B3D0(void *arg0, void *arg1);
extern char D_0013B8C8[];  /* "sceDbcGetDepNumber: rpc error\n" */

/* The 0x40-byte port-state block the IOP-side libdbc server DMAs into
 * D_00143580 (func_0011B3D0 is handed its 0x80-byte span). Declared as bytes:
 * the ROM copies it with unaligned ldl/ldr+sdl/sdr pairs, i.e. a struct copy
 * whose alignment cc1 could not assume to be 8. */
typedef struct DbcPortStates {
    u8 bytes[0x40];
} DbcPortStates;
extern DbcPortStates D_00143580;
extern s32 D_00143600[];  /* 16 per-port connection states (copy of the above) */

/**
 * libdbc sceDbcGetDepNumber (named by the ROM's own error string, D_0013B8C8):
 * refresh the cached port-state table, then ask the IOP server (RPC 0x80000903
 * on client D_00143108) for the device's dependency number on `port`.
 *
 * The DMA'd state block D_00143580 is first handed to func_0011B3D0 (start,
 * start + 0x80), then copied to D_00143600 with interrupts disabled
 * (func_0011F5E0 / func_0011F628; the re-enable is unconditional here).
 * Returns -12 if port `port` is not in state 1 (connected), 0 after printing
 * the error on an RPC failure, else the reply word D_00143180[1].
 */
s32 func_00128578(s32 port) {
    func_0011B3D0(&D_00143580, (u8 *)&D_00143580 + 0x80);
    func_0011F5E0();
    *(DbcPortStates *)D_00143600 = D_00143580;
    func_0011F628();
    if (D_00143600[port] != 1) {
        return -12;
    }
    D_00143180[0] = port;
    if (sceSifCallRpc(&D_00143108, 0x80000903, 0, D_00143180, 0x400,
                      D_00143180, 0x400, 0, 0) < 0) {
        func_00128898(D_0013B8C8);
        return 0;
    }
    return D_00143180[1];
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_001286C0);

extern s32 D_00143130;      /* second libdbc RPC client (asynchronous sends) */
extern char D_0013B990[];   /* "sceDbcSendData2: rpc error\n" */
extern char D_0013B9B0[];   /* "sceDbcReceiveData: rpc error\n" */

/* The data-transfer view of the shared RPC buffer D_00143180 used by
 * sceDbcSendData2 / sceDbcReceiveData: port, command word, payload size, the
 * payload itself, and the server's result word at +0x8C. */
typedef struct DbcDataRequest {
    s32 port;
    s32 cmd;
    s32 size;
    u8 data[0x80];
    s32 result;
} DbcDataRequest;

/**
 * libdbc sceDbcSendData2 (named by its error string, D_0013B990): queue
 * `*size` bytes of `data` with command word `cmd` for `port` on the second
 * RPC client D_00143130 (RPC 0x8000091B, mode 1 = no-wait). Returns 0 without
 * sending if that client's previous call is still in flight
 * (sceSifCheckStatRpc == 1), 0 after printing the error on an RPC failure,
 * else 1. The size is re-read from `*size` on every loop test, as the ROM does.
 *
 * Written as an OLD-STYLE (K&R) definition on purpose: its callers in this
 * library compiled WITHOUT a prototype in scope. func_00128FD0 passes the
 * 64-bit command header in one register with no narrowing, which a prototyped
 * `s32 cmd` would have forced (dsll32/dsra32); see func_001287A8.
 */
s32 func_001286C8(port, cmd, size, data)
    s32 port;
    s32 cmd;
    s32 *size;
    u8 *data;
{
    DbcDataRequest *req = (DbcDataRequest *)D_00143180;
    s32 i;
    req->port = port;
    req->cmd = cmd;
    req->size = *size;
    for (i = 0; i < *size; i++) {
        req->data[i] = data[i];
    }
    if (sceSifCheckStatRpc(&D_00143130) == 1) {
        return 0;
    }
    if (sceSifCallRpc(&D_00143130, 0x8000091B, 1, D_00143180, 0x400,
                      D_00143180, 0x400, 0, 0) < 0) {
        func_00128898(D_0013B990);
        return 0;
    }
    return 1;
}

/**
 * libdbc sceDbcReceiveData (named by its error string, D_0013B9B0): send
 * command word `cmd` with the requested length `*size` for `port` (RPC
 * 0x8000091A, blocking, client D_00143108). Returns 0 after printing the error
 * on an RPC failure; otherwise, if the server's result is non-negative, writes
 * the reply length back to `*size` and copies that many reply bytes to `data`.
 * Returns the server's result word (re-read after the copy).
 *
 * OLD-STYLE (K&R) definition, like func_001286C8: the ROM's callers saw no
 * prototype. func_00128F50 passes the whole 64-bit header (Pad2Cmd.all),
 * while func_00128B28 / func_00128C18 pass its low word as an int, which the
 * ROM sign-extends at the call (dsll32/dsra32). One prototype cannot give
 * both: a `u64 cmd` prototype makes this function narrow its argument itself
 * (59 of 60 words) and a union parameter fails the same way, against 0 for
 * this definition. A native compiler converts the `u64` argument to `s32`
 * through the visible old-style definition (clang accepts it; a struct
 * argument would be a hard error there).
 */
s32 func_001287A8(port, cmd, size, data)
    s32 port;
    s32 cmd;
    s32 *size;
    u8 *data;
{
    DbcDataRequest *req = (DbcDataRequest *)D_00143180;
    s32 i;
    req->port = port;
    req->cmd = cmd;
    req->size = *size;
    if (sceSifCallRpc(&D_00143108, 0x8000091A, 0, req, 0x400,
                      req, 0x400, 0, 0) < 0) {
        func_00128898(D_0013B9B0);
        return 0;
    }
    if (req->result >= 0) {
        *size = req->size;
        for (i = 0; i < req->size; i++) {
            data[i] = req->data[i];
        }
    }
    return req->result;
}

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
    u8 state;      /* 0x000: device state byte (func_00128C18's result) */
    u8 pad1;
    u8 length;     /* 0x002: bytes of `data` that are valid */
    u8 pad3;
    s32 valid;     /* 0x004: non-zero once the instance holds a frame */
    u8 pad8[0x14];
    u8 data[0x60]; /* 0x01C: payload; the button bitmap follows the first `length` bytes */
    s32 unk7C;     /* 0x07C — selection key (larger wins) */
} ResSubObj;

/* Where one button's value sits in the controller's report (func_00128E98
 * fills 40 of these from the device's button profile bitmap). */
typedef struct Pad2ButtonInfo {
    s32 present;   /* 1 if the profile says the device has this button */
    s32 width;     /* bits of report data: 8 (analog, indices 16..31 and 35..38) or 1 */
    s32 byteIndex; /* report byte that holds it */
    s32 bitIndex;  /* bit within that byte for a 1-bit button, else 0 */
} Pad2ButtonInfo;

/* 16-entry resource table, stride 0x330: +0x4 active flag, +0x8 handle,
 * +0xC object pointer. Struct-typed so func_00128D58's per-field array
 * indexing compiles to the original dual-base store pair. */
typedef struct ResTableEntry {
    s32 unk0;         /* 0x000 */
    s32 active;       /* 0x004 */
    s32 handle;       /* 0x008 */
    ResSubObj *obj;   /* 0x00C */
    Pad2ButtonInfo buttons[40]; /* 0x010: decoded button profile (func_00128E98) */
    u8 pad[0x330 - 0x290];
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

/* The caller's socket parameters: the layout of the vendored libpad2.h
 * scePad2SocketParam {option, port, slot, number, name[16]} (FACT #5893). */
typedef struct Pad2SocketParam {
    u32 option;
    s32 port;
    s32 slot;
    s32 number;
    DbcName name;
} Pad2SocketParam;

/**
 * Open a controller socket (the shape of libpad2's scePad2CreateSocket,
 * FACT #5893): `buf` must be 64-byte aligned (else -1) and holds the two
 * 0x80-byte frame instances the IOP DMAs into. Copies `param` (or zeroes when
 * it is NULL) into a libdbc socket request with `type` 1 and option bit 0 set,
 * creates the socket with func_001284B0 (a negative result is returned as
 * is), records `buf` in the socket's table slot and marks the slot used, then
 * clears both instances (header bytes, `valid`, the selection key, and the
 * first 0x20 payload bytes to 0xFF). Returns the socket index.
 *
 * The slot's `unk0` is written before `obj` in source; the other order swaps
 * the ROM's two stores (6 of 82 words).
 */
s32 sceDbcPortOpen(Pad2SocketParam *param, ResSubObj *buf) {
    DbcSocketParam sp;
    ResSubObj *obj;
    s32 h;
    s32 i;
    if ((u32)buf & 0x3F) {
        return -1;
    }
    if (param != 0) {
        sp.option = param->option;
        sp.port = param->port;
        sp.slot = param->slot;
        sp.number = param->number;
        sp.name = param->name;
    } else {
        sp.option = 0;
        sp.port = 0;
        sp.slot = 0;
        sp.number = 0;
        sp.name.b[0] = 0;
    }
    sp.type = 1;
    sp.option |= 1;
    h = func_001284B0(&sp, buf, buf + 1);
    if (h < 0) {
        return h;
    }
    D_00143640[h].unk0 = 1;
    D_00143640[h].obj = buf;
    obj = buf;
    for (i = 0; i < 2; i++) {
        obj->state = 0;
        obj->unk7C = 0;
        obj->pad1 = 0;
        obj->pad3 = 0;
        obj->length = 0;
        obj->valid = 0;
        memset(obj->data, 0xFF, 0x20);
        obj++;
    }
    return h;
}

/* func_00128A48: 0x8 bytes of inter-function padding split off by symbol_addrs
 * size:0x8; the real function begins at func_00128A50. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00128A48);

extern s32 func_00128D58(s32 index);
extern ResSubObj *func_00128DB0(s32 index);
extern s32 func_00128E18(s32 index);
extern s32 func_00128E98(u8 *profile, Pad2ButtonInfo *info);

/**
 * Read the latest frame of controller `index` (shape of libpad2's
 * scePad2Read(int, unsigned char *)): returns -1 if the table slot is unused
 * (+0x0 == 0) or its socket cannot be (re)opened via func_00128D58. Otherwise
 * picks the newer of the two DMA'd instances (func_00128DB0) and, if it holds
 * `length` bytes, copies them to `out` and decodes the button profile that
 * follows them into the slot's button table (func_00128E98). Returns -1 if the
 * instance holds no valid frame, else the frame length.
 *
 * Recovered from a splat mis-split: the 8 bytes before it are padding
 * (func_00128A48). The `src != 0` test on an array member's address is the
 * ROM's own `beqz` on obj+0x1C.
 */
s32 func_00128A50(s32 index, u8 *out) {
    ResSubObj *obj;
    u8 *src;
    if (D_00143640[index].unk0 == 0) {
        return -1;
    }
    if (D_00143640[index].active == 0) {
        if (func_00128D58(index) < 0) {
            return -1;
        }
    }
    obj = func_00128DB0(index);
    if (obj->length != 0) {
        src = obj->data;
        if (src != 0) {
            memcpy(out, src, obj->length);
            func_00128E98(obj->data + obj->length, D_00143640[index].buttons);
        }
    }
    if (obj->valid == 0) {
        return -1;
    }
    return obj->length;
}

/* The 64-bit command header libpad2 sends through sceDbcSendData2 /
 * sceDbcReceiveData. A u64 container is what the ROM builds: the top field's
 * clear mask is the 64-bit constant 0xFFFFFFFF00FFFFFF (li/dsll/ori/dsll/ori),
 * which a 32-bit container would load as lui/ori. */
typedef struct Pad2CmdHeader {
    u64 cmd : 14;
    u64 mode : 2;
    u64 sub : 8;
    u64 version : 8;
} Pad2CmdHeader;

/* The header as the value handed to func_001286C8 / func_001287A8: callers
 * func_00128B28 / func_00128C18 pass the 32-bit `word`, which the ROM
 * sign-extends at the call; func_00128F50 / func_00128FD0 pass the whole
 * doubleword `all`, unnarrowed (see func_001287A8 for why both exist). */
typedef union Pad2Cmd {
    Pad2CmdHeader bits;
    s32 word;
    u64 all;
} Pad2Cmd;

/**
 * Fetch controller `index`'s button profile (shape of libpad2's
 * scePad2GetButtonProfile(int, unsigned char *)): opens the socket on demand
 * (func_00128D58; -1 if that fails), receives command 2/3/2 into `profile`
 * (func_001287A8, whose negative result is returned as is), decodes it into
 * the slot's button table (func_00128E98) and returns the profile length.
 *
 * The header is uninitialised before its four fields are set, as in the ROM
 * (whose `and` masks read the incoming register): the struct lives in $16.
 */
s32 func_00128B28(s32 index, u8 *profile) {
    Pad2Cmd cmd;
    s32 size;
    s32 ret;
    if (D_00143640[index].active == 0) {
        if (func_00128D58(index) < 0) {
            return -1;
        }
    }
    cmd.bits.cmd = 2;
    cmd.bits.mode = 3;
    cmd.bits.sub = 2;
    cmd.bits.version = 1;
    ret = func_001287A8(index, cmd.word, &size, profile);
    if (ret < 0) {
        return ret;
    }
    func_00128E98(profile, D_00143640[index].buttons);
    return size;
}

/**
 * Return controller `index`'s state byte (shape of libpad2's
 * scePad2GetState(int)); 0 if the socket cannot be opened. When
 * func_00128E18 reports the DMA'd pair changed, the byte comes from the newer
 * instance (func_00128DB0) — and if that instance holds no frame and the
 * socket cannot be reopened, the slot is marked inactive and 0 is returned.
 * Otherwise it is fetched with command 0xC/2/1 through func_001287A8 (0 on a
 * negative result).
 *
 * `value` is a single addressable byte at sp+0 with `size` at sp+4, as in the
 * ROM; a `u8 value[4]` array instead measured a 0x60 frame against 0x50.
 */
s32 func_00128C18(s32 index) {
    Pad2Cmd cmd;
    u8 value;
    s32 size;
    ResSubObj *obj;
    if (D_00143640[index].active == 0) {
        if (func_00128D58(index) < 0) {
            return 0;
        }
    }
    if (func_00128E18(index) != 0) {
        obj = func_00128DB0(index);
        if (obj->valid == 0) {
            if (func_00128D58(index) < 0) {
                D_00143640[index].active = 0;
                return 0;
            }
        }
        value = obj->state;
    } else {
        cmd.bits.cmd = 0xC;
        cmd.bits.mode = 2;
        cmd.bits.sub = 1;
        cmd.bits.version = 1;
        if (func_001287A8(index, cmd.word, &size, &value) < 0) {
            return 0;
        }
    }
    return value;
}

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

/**
 * Decode a device's 40-bit button profile (5 bytes at `profile`, LSB first)
 * into the report layout table `info[40]`. Every button the profile marks
 * present gets the next report position: buttons 16..31 and 35..38 (the
 * analog ones) take a whole byte (width 8); the others pack eight to a byte
 * (width 1, bitIndex 0..7). Absent buttons get all zeroes. Returns 1.
 *
 * `i` is declared first: declaring it after the other counters swaps its
 * register with `bitIndex`'s ($6/$7), 10 of 44 words.
 */
s32 func_00128E98(u8 *profile, Pad2ButtonInfo *info) {
    s32 i;
    s32 bit = 0;
    s32 byteIndex = 0;
    s32 bitIndex = 0;
    for (i = 0; i < 40; i++) {
        if ((*profile >> bit) & 1) {
            info->present = 1;
            info->byteIndex = byteIndex;
            info->bitIndex = bitIndex;
            if ((i >= 16 && i < 32) || (i >= 35 && i < 39)) {
                info->width = 8;
                byteIndex++;
            } else {
                bitIndex++;
                info->width = 1;
                if ((bitIndex & 7) == 0) {
                    byteIndex++;
                    bitIndex = 0;
                }
            }
        } else {
            info->present = 0;
            info->byteIndex = 0;
            info->bitIndex = 0;
        }
        bit++;
        if ((bit & 7) == 0) {
            profile++;
            bit = 0;
        }
        info++;
    }
    return 1;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_00128F48);

/**
 * Receive command 2/2/3 for controller `port` into `data` through
 * func_001287A8 and return the received length, or func_001287A8's negative
 * result. Passes the whole header (no narrowing; see func_001287A8). `size`
 * is not initialised before the call: func_001287A8 forwards whatever it
 * holds as the requested length, as the ROM does.
 */
s32 func_00128F50(s32 port, u8 *data) {
    Pad2Cmd cmd;
    s32 size;
    s32 ret;
    cmd.bits.cmd = 2;
    cmd.bits.mode = 2;
    cmd.bits.sub = 3;
    cmd.bits.version = 1;
    ret = func_001287A8(port, cmd.all, &size, data);
    if (ret < 0) {
        return ret;
    }
    return size;
}

/* The 0x30-byte payload func_00128FD0 sends: the two lengths, then the second
 * block followed by the first. */
typedef struct Pad2SendBuffer {
    s32 len0;
    s32 len1;
    u8 data[0x28];
} Pad2SendBuffer;

/**
 * Send two data blocks to controller `port` in one command 0xB/1/3
 * (func_001286C8, asynchronous): the payload is {len0, len1, src1[len1],
 * src0[len0]} and its size is always passed as 0x28. Returns func_001286C8's
 * result (1 = queued, 0 = busy or RPC error).
 *
 * Statement order is the ROM's: `size` before the header fields (else the
 * `li 40` / `lui 0x100` pair swaps, 20 of 59 words), and `len0` stored before
 * the second memcpy (it sits in that call's delay slot).
 */
s32 func_00128FD0(s32 port, s32 len0, u8 *src0, s32 len1, u8 *src1) {
    Pad2Cmd cmd;
    Pad2SendBuffer buf;
    s32 size;
    size = 0x28;
    cmd.bits.cmd = 0xB;
    cmd.bits.mode = 1;
    cmd.bits.sub = 3;
    cmd.bits.version = 1;
    buf.len1 = len1;
    memcpy(buf.data, src1, len1);
    buf.len0 = len0;
    memcpy(buf.data + len1, src0, len0);
    return func_001286C8(port, cmd.all, &size, (u8 *)&buf);
}

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

/* The MPEG-2 decoder context of the IPU video player. The header parsers in
 * this unit follow the MSSG mpeg2decode reference syntax field for field, which
 * is where the names below come from; fields no function here shows stay
 * padding. frames[] is the 3x4 table of buffer pointers func_0012FA18 walks:
 * rows at 0x1B8/0x1C8/0x1D8. */
typedef struct {
    u8  _pad0[0xC];
    s32 field0C;               /* 0x00C..0x048: cleared or seeded by IpuInitDecoder */
    s32 _pad10;
    s32 field14;
    s32 _pad18;
    void *field1C;             /* 0x01C: D_00130A90, the real entry of func_00130A8C */
    s32 _pad20;
    void (*field24)(void *);   /* 0x024: func_00130AA0 */
    s32 _pad28;
    s32 field2C;
    s32 _pad30;
    s32 field34;
    s32 _pad38;
    s32 field3C;
    s32 _pad40;
    s32 field44;               /* 0x044: a 0x600-byte block from func_0012FB80 */
    s32 field48;
    u8  _pad4C[0x70 - 0x4C];
    s32 field70;
    s32 _pad74;
    s64 field78;
    s32 field80;               /* 0x080: owner field 0x10, after func_0012DAC0 */
    s32 _pad84;
    u64 field88;               /* 0x088: D_00137F38[bits 5..8 of owner field 0x20] */
    s32 field90;
    s32 field94;               /* 0x094..0x09C: -1 after IpuInitDecoder */
    s32 field98;
    s32 field9C;
    u8  _padA0[0xAC - 0xA0];
    s32 fieldAC;
    s32 fieldB0;               /* 0x0B0: non-zero picks func_0012E608 over func_0012D808 */
    s32 pictureParams[6];      /* 0x0B4: copied from IpuPictureSize.params */
    s32 fieldCC;               /* 0x0CC: IpuPictureSize.field5C */
    s32 fieldD0;               /* 0x0D0: IpuPictureSize.field60 */
    s32 firstPictureStructure; /* 0x0D4: the first picture_structure since the
                                  last sequence header */
    s32 _padD8;
    s32 maxWidth;              /* 0x0DC: widest picture the buffers allow */
    s32 maxHeight;             /* 0x0E0: tallest picture; 0 = check bufferSize */
    s32 bufferSize;            /* 0x0E4 */
    s32 fieldE8;               /* 0x0E8: cleared by each GOP header */
    s32 _padEC;
    s64 fieldF0;               /* 0x0F0: -1 after IpuInitDecoder */
    s32 state;                 /* 0x0F8: 1 is advanced to 2 by func_0012D420 */
    s32 fieldFC;
    s32 field100;
    s32 field104;
    u8  _pad108[0x118 - 0x108];
    s32 count;                 /* 0x118 */
    s32 lastDecodeZero;        /* 0x11C: the last VDEC result was 0 */
    s32 pending;               /* 0x120: an unpaired field picture */
    s32 horizontalSize;        /* 0x124 */
    s32 verticalSize;          /* 0x128 */
    s32 mbWidth;               /* 0x12C: picture width in macroblocks */
    s32 mbHeight;              /* 0x130 */
    s32 bitRateValue;          /* 0x134 */
    s32 vbvBufferSize;         /* 0x138 */
    s32 progressiveSequence;   /* 0x13C */
    s32 chromaFormat;          /* 0x140: 1 = 4:2:0, the only one supported */
    u8  _pad144[0x150 - 0x144];
    s32 pictureCodingType;     /* 0x150: 1 I, 2 P, 3 B */
    s32 fullPelForward;        /* 0x154 */
    s32 forwardFCode;          /* 0x158 */
    s32 fullPelBackward;       /* 0x15C */
    s32 backwardFCode;         /* 0x160 */
    s32 fCode[2][2];           /* 0x164: f_code[forward/backward][h/v] */
    s32 pictureStructure;      /* 0x174: 1 top field, 2 bottom field, 3 frame */
    s32 topFieldFirst;         /* 0x178 */
    s32 framePredFrameDct;     /* 0x17C */
    s32 concealmentMotionVectors; /* 0x180 */
    s32 repeatFirstField;      /* 0x184 */
    s32 progressiveFrame;      /* 0x188 */
    s32 frameCentreHOffset[3]; /* 0x18C */
    s32 frameCentreVOffset[3]; /* 0x198 */
    s32 closedGop;             /* 0x1A4 */
    s32 brokenLink;            /* 0x1A8 */
    s32 _pad1AC;
    s32 field1B0;              /* 0x1B0: set by a skipped macroblock */
    s32 _pad1B4;
    s32 frames[3][4];          /* 0x1B8 */
    u8  _pad1E8[0x810 - 0x1E8];
    s32 mbSlot;                /* 0x810: selects a 0x140-byte record (see func_0012BAA0) */
    s32 _pad814;
    s32 lastCmdFlag;           /* 0x818: D_00137F10[opcode] of the last IPU command */
    void *field81C;            /* 0x81C: scratchpad 0x70003600 after IpuInitDecoder */
    u8  _pad820[0x828 - 0x820];
    s64 pictureInfo[2];        /* 0x828: the two values callback request 5
                                  returns for each picture */
    s32 bitBuffer;             /* 0x838: the 32 stream bits FDEC last returned */
    s32 bitsValid;             /* 0x83C: 32 after each FDEC */
    s32 loadIntraQuant;        /* 0x840 */
    s32 loadNonIntraQuant;     /* 0x844 */
    s32 mpeg2;                 /* 0x848: 1 once a sequence_extension is seen */
    s32 gopBasePicture;        /* 0x84C: lastPicture + 1 at each GOP header */
    s32 lastPicture;           /* 0x850 */
    s32 newGop;                /* 0x854: set by each GOP header */
    s32 *callbacks;            /* 0x858: handed to func_0012FAE8 while waiting */
} IpuDecoder;

extern void IpuWaitReady(s32 *ipu);
extern s32 func_0012FA98(s32 *obj, s32 *req);
extern void func_00130288(s32 arg0, void *buf);
extern char D_0013BA98[];

/**
 * Wait for the IPU output DMA (IPU_FROM) to drain and resynchronise the bit position
 * (called before each macroblock by IpuDecodeSlice). After IpuWaitReady,
 * while the IPU_FROM DMA (channel 3, QWC 0x1000B020) still has quadwords
 * and IPU_CTRL (0x10002010) shows no error (bit 14), ask callback entry 1
 * for more input whenever the IPU_TO channel (4) is idle (QWC 0, CHCR STR
 * clear). Then reload bitBuffer from IPU_TOP and bitsValid from IPU_BP (32,
 * or what is left of the current word when TOP is valid).
 *
 * On an IPU error: report it (D_0013BA98), run callback entries 2 and 3
 * around a reset command (IPU_CTRL = 0x40000000), stop channel 3 with the
 * DMAC held (D_ENABLEW bit 16, interrupts disabled), clear its QWC and
 * return 0. Returns 1 otherwise.
 *
 * IPU_BP is read through a volatile pointer BEFORE the plain IPU_TOP read,
 * as in func_0012C508; TOP first leaves 13 of the 127 words different.
 */
s32 IpuWaitBdec(IpuDecoder *dec) {
    s32 ok = 1;
    s32 kick[8];
    s32 req[8];
    s64 top;
    u32 bp;
    s32 wasEnabled;

    IpuWaitReady((s32 *)dec);
    while (*(volatile u32 *)0x1000B020 != 0 && !(*(volatile u32 *)0x10002010 & 0x4000)) {
        if (*(volatile u32 *)0x1000B420 == 0 && !(*(volatile u32 *)0x1000B400 & 0x100)) {
            kick[0] = 1;
            func_0012FA98(dec->callbacks, kick);
        }
    }
    bp = *(volatile u32 *)0x10002020;
    top = *(s64 *)0x10002030;
    dec->bitBuffer = top;
    if (top < 0) {
        dec->bitsValid = (bp & 0x1F) ? 32 - (bp & 0x1F) : 0;
    } else {
        dec->bitsValid = 32;
    }
    if (*(volatile u32 *)0x10002010 & 0x4000) {
        func_00130288((s32)dec, D_0013BA98);
        req[0] = 2;
        func_0012FA98(dec->callbacks, req);
        *(u32 *)0x10002010 = 0x40000000;
        req[0] = 3;
        func_0012FA98(dec->callbacks, req);
        wasEnabled = func_0011F5E0();
        *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 | 0x10000;
        *(volatile u32 *)0x1000B000 = 0;
        *(volatile u32 *)0x1000F590 = *(volatile u32 *)0x1000F520 & ~0x10000;
        if (wasEnabled) {
            func_0011F628();
        }
        ok = 0;
        *(volatile u32 *)0x1000B020 = 0;
    }
    return ok;
}

/* func_0012B3C0(arg0): thin wrapper — forwards to func_0012C508(arg0, 3) and
 * returns its result. (The prior "sibling-call wall" note was wrong: ee-gcc 2.9
 * has NO sibling-call optimization, so this compiles to the original's jal +
 * real frame — byte-exact.) */
extern s32 func_0012C508(s32 arg0, s32 arg1);
s32 func_0012B3C0(s32 arg0) {
    return func_0012C508(arg0, 3);
}

/**
 * Derive the dual-prime motion vectors (mpeg2decode's Dual_Prime_Arithmetic)
 * from the vector (mvx, mvy) and the decoded dmvector. In a frame picture
 * dmvOut[0] predicts one field from the other and dmvOut[1] the reverse,
 * with the 1/2 and 3/2 scalings swapped when the bottom field comes first;
 * in a field picture only dmvOut[0] is formed, its vertical part shifted by
 * one line toward the opposite parity.
 */
void func_0012B3E0(IpuDecoder *dec, s32 dmvOut[2][2], s32 *dmvector, s32 mvx, s32 mvy) {
    if (dec->pictureStructure == 3) {
        if (dec->topFieldFirst) {
            dmvOut[0][0] = ((mvx + (mvx > 0)) >> 1) + dmvector[0];
            dmvOut[0][1] = ((mvy + (mvy > 0)) >> 1) + dmvector[1] - 1;
            dmvOut[1][0] = ((3 * mvx + (mvx > 0)) >> 1) + dmvector[0];
            dmvOut[1][1] = ((3 * mvy + (mvy > 0)) >> 1) + dmvector[1] + 1;
        } else {
            dmvOut[0][0] = ((3 * mvx + (mvx > 0)) >> 1) + dmvector[0];
            dmvOut[0][1] = ((3 * mvy + (mvy > 0)) >> 1) + dmvector[1] - 1;
            dmvOut[1][0] = ((mvx + (mvx > 0)) >> 1) + dmvector[0];
            dmvOut[1][1] = ((mvy + (mvy > 0)) >> 1) + dmvector[1] + 1;
        }
    } else {
        dmvOut[0][0] = ((mvx + (mvx > 0)) >> 1) + dmvector[0];
        dmvOut[0][1] = ((mvy + (mvy > 0)) >> 1) + dmvector[1];
        if (dec->pictureStructure == 1) {
            dmvOut[0][1]--;
        } else {
            dmvOut[0][1]++;
        }
    }
}

/* func_00130250 formats a report; its definition below takes only the object,
 * so this caller sees no prototype and passes the format and its value. */
extern void func_00130250();
extern char D_0013BAB8[];
extern s32 func_0012C680(s32 *arg0, s32 arg1);
extern s32 IpuSkipBits(s32 *arg0, s32 arg1);

/**
 * Decode a macroblock_address_increment (mpeg2decode's
 * Get_macroblock_address_increment) with VDEC table 0 (func_0012C508):
 * macroblock_stuffing (0x22) is skipped, each macroblock_escape (0x23) adds
 * 33, and any other value ends the increment and is added. A 0 is a bad
 * code: in an MPEG-2 stream an 11-bit 0xF that follows is skipped (and
 * decoding goes on); otherwise the code is reported (D_0013BAB8 through
 * func_00130250), lastDecodeZero set, and 1 returned.
 *
 * Returns the address increment. The source case order (stuffing, escape,
 * 0, default) and `more = 1` ahead of the escape's += 33 give the ROM's
 * layout; the other orders leave 3 to 56 words different.
 */
s32 func_0012B568(s32 *ipu) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    s32 increment = 0;
    u32 code;
    s32 peek;
    s32 more;
    do {
        code = func_0012C508((s32)ipu, 0);
        switch (code) {
        case 0x22:
            more = 1;
            break;
        case 0x23:
            more = 1;
            increment += 0x21;
            break;
        case 0:
            peek = func_0012C680(ipu, 11);
            if (dec->mpeg2 != 0 && peek == 0xF) {
                IpuSkipBits(ipu, 11);
                more = 1;
                break;
            }
            func_00130250(dec, D_0013BAB8, code);
            dec->lastDecodeZero = 1;
            return 1;
        default:
            increment += code;
            more = 0;
            break;
        }
    } while (more);
    return increment;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012B678);

extern void func_0012C9C8(s32 *ipu);
extern s32 func_0012CA48(s32 *arg0);
extern void func_00130288(s32 arg0, void *buf);
extern char D_0013BB10[];
extern char D_0013BB38[];

/**
 * Start a slice (mpeg2decode's start_of_slice): clear lastDecodeZero, align
 * to the next start code and peek it. Anything but a slice start code
 * (0x101..0x1AF) is reported (D_0013BB10 through func_00130250) and ends the
 * picture (returns 2). Otherwise skip the code, read the slice header
 * (func_0012CA48, which returns slice_vertical_position_extension) and the
 * first macroblock_address_increment (func_0012B568); a bad increment is
 * reported (D_0013BB38) and returns 1. Then set the macroblock address
 * *mba from the slice's vertical position and the increment, reset *mbaInc
 * to 1, set field1B0 and clear the motion-vector predictors pmv[][][]
 * (mbaMax is unused). Returns 0 to decode the slice's macroblocks.
 *
 * The predictor clears are the reference's two chained assignments, whose
 * store order is the ROM's; as eight statements in index order 8 of the 75
 * words differ.
 */
s32 func_0012B780(IpuDecoder *dec, s32 mbaMax, s32 *mba, s32 *mbaInc, s32 pmv[2][2][2]) {
    u32 code;
    s32 vertPosExt;
    dec->lastDecodeZero = 0;
    func_0012C9C8((s32 *)dec);
    code = func_0012C680((s32 *)dec, 32);
    if (code < 0x101 || code > 0x1AF) {
        func_00130250(dec, D_0013BB10, code);
        return 2;
    }
    IpuSkipBits((s32 *)dec, 32);
    vertPosExt = func_0012CA48((s32 *)dec);
    *mbaInc = func_0012B568((s32 *)dec);
    if (dec->lastDecodeZero) {
        func_00130288((s32)dec, D_0013BB38);
        return 1;
    }
    *mba = ((vertPosExt << 7) + (code & 255) - 1) * dec->mbWidth + *mbaInc - 1;
    *mbaInc = 1;
    dec->field1B0 = 1;
    pmv[0][0][0] = pmv[0][0][1] = pmv[1][0][0] = pmv[1][0][1] = 0;
    pmv[0][1][0] = pmv[0][1][1] = pmv[1][1][0] = pmv[1][1][1] = 0;
    return 0;
}

extern s32 IpuWaitBdec(IpuDecoder *dec);
extern s32 func_0012BB60(IpuDecoder *dec, s32 *macroblockType, s32 *motionType,
                         s32 *dctType, s32 pmv[2][2][2], s32 mvFieldSel[2][2],
                         s32 dmvector[2]);
extern s32 func_00129450(IpuDecoder *dec, s32 mba, s32 mbaInc, s32 macroblockType,
                         s32 motionType, s32 pmv[2][2][2], s32 mvFieldSel[2][2],
                         s32 dmvector[2]);
extern void func_0012A1C8(IpuDecoder *dec, s32 slot);
extern char D_0013BB58[];
s32 func_0012BAA0(IpuDecoder *dec, s32 pmv[2][2][2], s32 *motionType,
                  s32 mvFieldSel[2], s32 *macroblockType);

/**
 * Decode one slice (mpeg2decode's slice): start it (func_0012B780, whose
 * nonzero result is returned as is), then for each macroblock up to mbaMax
 * clear the current 0x140-byte record's word 0x6CC and resynchronise the
 * IPU (IpuWaitBdec, 0 ends the picture with 2). With no increment pending,
 * a zero 23-bit peek or a bad code ends the slice (3); otherwise decode the
 * next increment (func_0012B568). An address past mbaMax is reported
 * (D_0013BB58) and returns 2. A coded macroblock (increment 1) is decoded by
 * func_0012BB60 (failure: 1), a skipped one by func_0012BAA0 (failure: 2);
 * both are then motion-compensated by func_00129450 (failure: 2). From the
 * second macroblock on, the previous record (mbSlot ^ 1) is flushed through
 * func_0012A1C8. Every failure clears lastDecodeZero.
 *
 * Returns 0 once mbaMax macroblocks are done.
 */
s32 IpuDecodeSlice(IpuDecoder *dec, s32 mbaMax) {
    s32 pmv[2][2][2];
    s32 mvFieldSel[2][2];
    s32 dmvector[2];
    s32 mba, mbaInc;
    s32 macroblockType, motionType, dctType;
    s32 ret;

    mba = 0;
    mbaInc = 0;
    ret = func_0012B780(dec, mbaMax, &mba, &mbaInc, pmv);
    if (ret != 0) {
        return ret;
    }
    dec->lastDecodeZero = 0;
    for (;;) {
        if (mba >= mbaMax) {
            return 0;
        }
        *(s32 *)((u8 *)dec + dec->mbSlot * 0x140 + 0x6CC) = 0;
        if (IpuWaitBdec(dec) == 0) {
            return 2;
        }
        if (mbaInc == 0) {
            if (func_0012C680((s32 *)dec, 23) == 0 || dec->lastDecodeZero) {
                dec->lastDecodeZero = 0;
                return 3;
            }
            mbaInc = func_0012B568((s32 *)dec);
            if (dec->lastDecodeZero) {
                goto resync;
            }
        }
        if (mba >= mbaMax) {
            func_00130288((s32)dec, D_0013BB58);
            return 2;
        }
        if (mbaInc == 1) {
            if (func_0012BB60(dec, &macroblockType, &motionType, &dctType, pmv,
                              mvFieldSel, dmvector) == 0) {
            resync:
                dec->lastDecodeZero = 0;
                return 1;
            }
        } else if (func_0012BAA0(dec, pmv, &motionType, mvFieldSel[0], &macroblockType) == 0) {
            goto fail;
        }
        if (func_00129450(dec, mba, mbaInc, macroblockType, motionType, pmv,
                          mvFieldSel, dmvector) == 0) {
        fail:
            dec->lastDecodeZero = 0;
            return 2;
        }
        if (mba != 0) {
            func_0012A1C8(dec, dec->mbSlot ^ 1);
        }
        mba++;
        dec->mbSlot ^= 1;
        mbaInc--;
    }
}

/* A decoded-picture buffer, as far as func_0012D350 shows it. */
typedef struct {
    u8  _pad0[0x28];
    s32 decoded;               /* 0x28: set once func_0012B678 succeeds */
} IpuFrameBuf;

extern void func_00130288(s32 arg0, void *buf);
extern char D_0013BB78[];

/**
 * Skipped-macroblock bookkeeping (mpeg2decode's skipped_macroblock): mark the
 * current 0x140-byte record (word 0x6CC + 0x140 * mbSlot) and field1B0, reset
 * the forward motion-vector predictors of a P picture, derive the motion type
 * (frame motion for a frame picture, else field motion selecting the bottom
 * field when the picture is one), report a skipped macroblock in an I picture
 * as an error, and clear the macroblock's intra bit.
 *
 * Returns 0 for the I-picture error, else 1. The predictor reset is the
 * reference's chained assignment, which stores PMV[1][0][1] first: written as
 * four statements in index order, cc1 issues them in a different order.
 */
s32 func_0012BAA0(IpuDecoder *dec, s32 pmv[2][2][2], s32 *motionType,
                  s32 mvFieldSel[2], s32 *macroblockType) {
    s32 ok = 1;
    *(s32 *)((u8 *)dec + dec->mbSlot * 0x140 + 0x6CC) = 1;
    dec->field1B0 = 1;
    if (dec->pictureCodingType == 2) {
        pmv[0][0][0] = pmv[0][0][1] = pmv[1][0][0] = pmv[1][0][1] = 0;
    }
    if (dec->pictureStructure == 3) {
        *motionType = 2;
    } else {
        *motionType = 1;
        mvFieldSel[0] = mvFieldSel[1] = dec->pictureStructure == 2;
    }
    if (dec->pictureCodingType == 1) {
        func_00130288((s32)dec, D_0013BB78);
        ok = 0;
    }
    *macroblockType &= ~1;
    return ok;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/022FA8", func_0012BB60);

/**
 * Decode one motion-vector component in place (mpeg2decode's
 * decode_motion_vector): `pred` holds the prediction, halved first for a
 * full-pel vector; a positive or negative motion_code moves it by
 * ((|code| - 1) << rSize) + residual + 1, wrapped into [-lim, lim) with
 * lim = 16 << rSize; the result is stored back, doubled again for full-pel.
 */
void func_0012C008(s32 *pred, s32 rSize, s32 code, s32 residual, s32 fullPel) {
    s32 lim = 16 << rSize;
    s32 vec = *pred;
    if (fullPel) {
        vec = vec >> 1;
    }
    if (code > 0) {
        vec += ((code - 1) << rSize) + residual + 1;
        if (vec >= lim) {
            vec -= lim * 2;
        }
    } else if (code < 0) {
        vec -= ((-code - 1) << rSize) + residual + 1;
        if (vec < -lim) {
            vec += lim * 2;
        }
    }
    *pred = fullPel ? vec << 1 : vec;
}

extern void func_0012C230(s32 *ipu, s32 *pmv, s32 *dmvector, s32 hRSize, s32 vRSize,
                          s32 dmv, s32 mvScale, s32 fullPel);

/**
 * Decode a macroblock's motion vectors (mpeg2decode's motion_vectors) for
 * direction `s` (0 forward, 1 backward). With one vector, a field vector
 * that is not dual-prime first reads one motion_vertical_field_select for
 * both fields; the vector is decoded into pmv[0][s] (func_0012C230) and
 * copied to pmv[1][s]. With two, each field reads its select bit and its
 * own vector into pmv[0][s] and pmv[1][s].
 *
 * func_0012C230 is defined below, so it is declared here: with an implicit
 * declaration the pmv copy's two loads swap registers.
 */
void func_0012C090(s32 *ipu, s32 pmv[2][2][2], s32 *dmvector, s32 mvFieldSel[2][2],
                   s32 s, s32 motionVectorCount, s32 mvFormat, s32 hRSize,
                   s32 vRSize, s32 dmv, s32 mvScale) {
    if (motionVectorCount == 1) {
        if (mvFormat == 0 && !dmv) {
            mvFieldSel[1][s] = mvFieldSel[0][s] = IpuGetBits(ipu, 1);
        }
        func_0012C230(ipu, pmv[0][s], dmvector, hRSize, vRSize, dmv, mvScale, 0);
        pmv[1][s][0] = pmv[0][s][0];
        pmv[1][s][1] = pmv[0][s][1];
    } else {
        mvFieldSel[0][s] = IpuGetBits(ipu, 1);
        func_0012C230(ipu, pmv[0][s], dmvector, hRSize, vRSize, dmv, mvScale, 0);
        mvFieldSel[1][s] = IpuGetBits(ipu, 1);
        func_0012C230(ipu, pmv[1][s], dmvector, hRSize, vRSize, dmv, mvScale, 0);
    }
}

extern s32 IpuGetBits(s32 *arg0, s32 arg1);

/**
 * Decode one motion vector (mpeg2decode's motion_vector): for the horizontal
 * and then the vertical component, read motion_code with VDEC table 2
 * (func_0012C508) and, when both the code and the component's r_size are
 * non-zero, an r_size-bit motion_residual (IpuGetBits), and update the
 * predictor through func_0012C008 (decode_motion_vector). A field vector in a
 * frame picture (mvScale) has its vertical predictor halved around the
 * update. With dual-prime (dmv) each component also reads its dmvector
 * (func_0012B3C0).
 */
void func_0012C230(s32 *ipu, s32 *pmv, s32 *dmvector, s32 hRSize, s32 vRSize,
                   s32 dmv, s32 mvScale, s32 fullPel) {
    s32 motionCode;
    s32 motionResidual;
    motionCode = func_0012C508((s32)ipu, 2);
    motionResidual = (hRSize != 0 && motionCode != 0) ? IpuGetBits(ipu, hRSize) : 0;
    func_0012C008(&pmv[0], hRSize, motionCode, motionResidual, fullPel);
    if (dmv) {
        dmvector[0] = func_0012B3C0((s32)ipu);
    }
    motionCode = func_0012C508((s32)ipu, 2);
    motionResidual = (vRSize != 0 && motionCode != 0) ? IpuGetBits(ipu, vRSize) : 0;
    if (mvScale) {
        pmv[1] >>= 1;
    }
    func_0012C008(&pmv[1], vRSize, motionCode, motionResidual, fullPel);
    if (mvScale) {
        pmv[1] <<= 1;
    }
    if (dmv) {
        dmvector[1] = func_0012B3C0((s32)ipu);
    }
}

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

extern s32 func_0012FAE8(s32 *obj);

/* The library's inline helpers: the ROM's IpuSkipBits and func_0012C680 carry
 * their own copies of IpuWaitReady's loop and of func_0012C380's two stores
 * rather than calling them (cc1 2.9 inlines only `inline` functions at -O2). */

/* Spin while IPU_CTRL (0x10002010) reads busy (bit 31) without an error
 * (bit 14). Every 5001 polls the decoder's callbacks get entry #1 run
 * (func_0012FAE8), and the count restarts. */
static inline void IpuWaitIdle(IpuDecoder *dec) {
    s32 count = 0;
    while ((*(volatile u32 *)0x10002010 & 0x80004000) == 0x80000000) {
        if (count++ > 5000) {
            func_0012FAE8(dec->callbacks);
            count = 0;
        }
    }
}

/* func_0012C380, inline: write IPU_CMD and note the opcode's table entry. */
static inline void IpuIssueCommand(IpuDecoder *dec, u32 cmd) {
    *(volatile u32 *)0x10002000 = cmd;
    dec->lastCmdFlag = D_00137F10[cmd >> 28];
}

/**
 * Wait for the IPU to go idle (IpuWaitIdle, out of line).
 */
void IpuWaitReady(s32 *ipu) {
    IpuWaitIdle((IpuDecoder *)ipu);
}

/**
 * Wait for an IPU command result: poll the 64-bit IPU_CMD (0x10002000) until
 * its busy bit 63 clears or IPU_CTRL (0x10002010) reports an error (bit 14),
 * running the decoder's callback entry #1 every 5001 polls as IpuWaitReady
 * does. Returns the last IPU_CMD value read.
 */
s64 IpuWaitCmdResult(s32 *ipu) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    s32 count = 0;
    s64 cmd;
    while ((cmd = *(volatile s64 *)0x10002000) < 0
           && !(*(volatile u32 *)0x10002010 & 0x4000)) {
        if (count++ > 5000) {
            func_0012FAE8(dec->callbacks);
            count = 0;
        }
    }
    return cmd;
}

/**
 * Decode one VLC symbol with the IPU (VDEC, mpeg2decode's table lookups):
 * wait for the IPU, issue VDEC 0x30000000 with table `tbl` in bits 26-27,
 * and spin on IPU_CMD's busy bit (running callback entry #1 every 5001
 * polls). Then reload the look-ahead from IPU_TOP (0x10002030) — when its
 * busy bit is set only the bits up to the next byte of IPU_BP (0x10002020)
 * count as valid — and note whether the result was 0 (lastDecodeZero).
 * Returns the decoded value (the result's low 16 bits, sign-extended).
 *
 * IPU_BP is read through a volatile pointer and IPU_TOP through a plain one,
 * in that order: the ROM forms IPU_BP's address in a register and loads
 * IPU_TOP through the assembler's absolute-address macro, which is what cc1
 * gives each kind of access. Both volatile, or both plain, leave 16 to 31
 * words different; so does the valid-bit count as a conditional expression.
 */
s32 func_0012C508(s32 arg0, s32 tbl) {
    IpuDecoder *dec = (IpuDecoder *)arg0;
    s32 count = 0;
    s32 cmd;
    s64 result;
    s64 top;
    u32 bp;
    IpuWaitIdle(dec);
    cmd = 0x30000000 | (tbl << 26);
    *(volatile u32 *)0x10002000 = cmd;
    dec->lastCmdFlag = D_00137F10[cmd >> 28];
    while ((result = *(volatile s64 *)0x10002000) < 0) {
        if (count++ > 5000) {
            func_0012FAE8(dec->callbacks);
            count = 0;
        }
    }
    bp = *(volatile u32 *)0x10002020;
    top = *(s64 *)0x10002030;
    dec->bitBuffer = top;
    if (top < 0) {
        dec->bitsValid = -(bp & 0x1F) & 0x1F;
    } else {
        dec->bitsValid = 32;
    }
    dec->lastDecodeZero = (s32)result == 0;
    return (s16)result;
}

/**
 * Peek at the next `n` bits of the IPU bitstream without consuming them
 * (mpeg2decode's Show_Bits): when the last command issued was not one that
 * leaves the look-ahead valid (lastCmdFlag) or fewer than `n` bits remain,
 * wait for the IPU, issue FDEC 0 and reload the 32-bit look-ahead from its
 * result. Returns the top `n` bits of the look-ahead.
 *
 * The wait and the command issue are IpuWaitIdle / IpuIssueCommand inlined,
 * as in IpuSkipBits.
 */
s32 func_0012C680(s32 *ipu, s32 n) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    if (dec->lastCmdFlag != 0 || dec->bitsValid < n) {
        IpuWaitIdle(dec);
        IpuIssueCommand(dec, 0x40000000);
        dec->bitBuffer = IpuWaitCmdResult(ipu);
        dec->bitsValid = 32;
    }
    return (u32)dec->bitBuffer >> (32 - n);
}

/**
 * Skip `n` bits of the IPU bitstream: wait for the IPU, issue FDEC with the
 * skip count (0x40000000 | n), and refill the 32-bit look-ahead from its
 * result (bitBuffer, bitsValid = 32).
 *
 * No return statement, though the declarations say s32: the ROM leaves FDEC's
 * result in $v0 only as a side effect, none of the nine ROM call sites reads
 * it, and an explicit `return` of it moves the sign-extension into $v1 and
 * lengthens the body by one or two words (func_0012CFA0 above has the same
 * shape).
 */
s32 IpuSkipBits(s32 *ipu, s32 n) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    IpuWaitIdle(dec);
    IpuIssueCommand(dec, 0x40000000 | n);
    dec->bitBuffer = IpuWaitCmdResult(ipu);
    dec->bitsValid = 32;
}

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

extern void func_00130300(s32 *ipu);
extern void IpuParseGopHeader(s32 *ipu);
extern void func_0012CBC0(s32 *ipu);
extern s32 func_0012FA98(s32 *obj, s32 *req);

/* A request for the decoder's callback dispatcher (func_0012FA98): the entry
 * index, then two 64-bit values the entry may overwrite. 0x20 bytes, as the
 * other requests in this unit. */
typedef struct {
    s32 index;
    s32 _pad4;
    s64 arg[2];
    s64 _pad18;
} IpuCallbackRequest;

/**
 * Read MPEG-2 headers up to the next picture (mpeg2decode's Get_Hdr): align
 * to each start code (func_0012C9C8) and dispatch on it — a sequence header
 * (0x1B3, func_00130300), a GOP header (0x1B8, IpuParseGopHeader), a picture
 * header (0x100, func_0012CBC0) or the sequence end code (0x1B7). Any other
 * code is skipped. After a picture header, ask callback entry 5 for the
 * picture's two 64-bit values (both seeded -1) into pictureInfo[].
 *
 * Returns the picture's picture_coding_type (1 I, 2 P, 3 B), or 0 at the end
 * of the sequence.
 */
s32 IpuParseVideoStartCodes(s32 *ipu) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    IpuCallbackRequest req;
    for (;;) {
        func_0012C9C8(ipu);
        switch ((u32)IpuGetBits(ipu, 32)) {
        case 0x1B3:
            func_00130300(ipu);
            break;
        case 0x1B8:
            IpuParseGopHeader(ipu);
            break;
        case 0x100:
            func_0012CBC0(ipu);
            req.index = 5;
            req.arg[0] = -1;
            req.arg[1] = -1;
            func_0012FA98(dec->callbacks, (s32 *)&req);
            dec->pictureInfo[0] = req.arg[0];
            dec->pictureInfo[1] = req.arg[1];
            return dec->pictureCodingType;
        case 0x1B7:
            return 0;
        }
    }
}

extern void func_0012CC88(s32 *ipu);
struct ScrollObj;
extern void func_0012CFE8(struct ScrollObj *obj, s32 delta);

/**
 * Parse an MPEG-2 picture header (mpeg2decode's picture_header):
 * temporal_reference (10 bits), picture_coding_type (3), vbv_delay (16, unused),
 * then the forward full-pel flag and f_code for P and B pictures and the
 * backward pair for B pictures. Then finish the header (func_0012CFA0, the
 * extra-information loop, and func_0012CC88) and advance the picture
 * numbering by the temporal reference (func_0012CFE8).
 */
void func_0012CBC0(s32 *ipu) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    s32 temporalRef = IpuGetBits(ipu, 10);
    dec->pictureCodingType = IpuGetBits(ipu, 3);
    IpuGetBits(ipu, 16);
    if (dec->pictureCodingType == 2 || dec->pictureCodingType == 3) {
        dec->fullPelForward = IpuGetBits(ipu, 1);
        dec->forwardFCode = IpuGetBits(ipu, 3);
    }
    if (dec->pictureCodingType == 3) {
        dec->fullPelBackward = IpuGetBits(ipu, 1);
        dec->backwardFCode = IpuGetBits(ipu, 3);
    }
    func_0012CFA0(ipu);
    func_0012CC88(ipu);
    func_0012CFE8((struct ScrollObj *)ipu, temporalRef);
}

/* The extension_and_user_data dispatch table: one handler per 4-bit
 * extension_start_code_identifier 0..10 (D_0013BBC8, copied to the stack by
 * each call). */
typedef struct {
    void (*handler[11])(s32 *ipu);
} IpuExtHandlerTable;
extern IpuExtHandlerTable D_0013BBC8;

/**
 * Consume the extension and user data that follow a header (mpeg2decode's
 * extension_and_user_data): align to the next start code, and while it peeks
 * as an extension (0x1B5) or user data (0x1B2) start code, skip the code.
 * For an extension, read its 4-bit identifier and run that entry of the
 * handler table (an identifier above 10 runs entry 0); user data is skipped
 * to the next start code.
 *
 * The clamp is a conditional expression: as an `if` (or `>= 11`), cc1 tests
 * `id < 11` with an immediate, where the ROM compares against 10 kept in a
 * register.
 */
void func_0012CC88(s32 *ipu) {
    IpuExtHandlerTable table = D_0013BBC8;
    s32 code;
    u32 id;
    func_0012C9C8(ipu);
    while ((code = func_0012C680(ipu, 32)) == 0x1B5 || code == 0x1B2) {
        if (code == 0x1B5) {
            IpuSkipBits(ipu, 32);
            id = IpuGetBits(ipu, 4);
            id = id > 10 ? 0 : id;
            table.handler[id](ipu);
            func_0012C9C8(ipu);
        } else {
            IpuSkipBits(ipu, 32);
            func_0012C9C8(ipu);
        }
    }
}

/**
 * Parse an MPEG-2 picture coding extension (mpeg2decode's
 * picture_coding_extension): the four f_codes, then the fields the IPU
 * decodes with go straight into IPU_CTRL (0x10002010) — intra_dc_precision
 * (bits 16-17), q_scale_type (bit 22), intra_vlc_format (bit 21) and
 * alternate_scan (bit 20) — and the rest into the decoder. The first
 * picture_structure after a sequence header is also kept in
 * firstPictureStructure. chroma_420_type and the composite display fields
 * are read and dropped.
 */
void func_0012CDB0(s32 *ipu) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    dec->fCode[0][0] = IpuGetBits(ipu, 4);
    dec->fCode[0][1] = IpuGetBits(ipu, 4);
    dec->fCode[1][0] = IpuGetBits(ipu, 4);
    dec->fCode[1][1] = IpuGetBits(ipu, 4);
    *(volatile u32 *)0x10002010 =
        (*(volatile u32 *)0x10002010 & ~0x30000) | (IpuGetBits(ipu, 2) << 16);
    dec->pictureStructure = IpuGetBits(ipu, 2);
    if (dec->firstPictureStructure == 0) {
        dec->firstPictureStructure = dec->pictureStructure;
    }
    dec->topFieldFirst = IpuGetBits(ipu, 1);
    dec->framePredFrameDct = IpuGetBits(ipu, 1);
    dec->concealmentMotionVectors = IpuGetBits(ipu, 1);
    *(volatile u32 *)0x10002010 =
        (*(volatile u32 *)0x10002010 & ~0x400000) | (IpuGetBits(ipu, 1) << 22);
    *(volatile u32 *)0x10002010 =
        (*(volatile u32 *)0x10002010 & ~0x200000) | (IpuGetBits(ipu, 1) << 21);
    *(volatile u32 *)0x10002010 =
        (*(volatile u32 *)0x10002010 & ~0x100000) | (IpuGetBits(ipu, 1) << 20);
    dec->repeatFirstField = IpuGetBits(ipu, 1);
    IpuGetBits(ipu, 1);
    dec->progressiveFrame = IpuGetBits(ipu, 1);
    if (IpuGetBits(ipu, 1)) {
        IpuGetBits(ipu, 1);
        IpuGetBits(ipu, 3);
        IpuGetBits(ipu, 1);
        IpuGetBits(ipu, 7);
        IpuGetBits(ipu, 8);
    }
}

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

extern s32 func_0012B678(s32 *ipu);
extern char D_0013BBF8[];
extern char D_0013BC20[];
extern char D_0013BC50[];
extern char D_0013BC70[];

/**
 * Parse an MPEG-2 group-of-pictures header (mpeg2decode's group_of_pictures_
 * header): clear fieldE8, start the new GOP's picture numbering after the
 * last picture seen, read and drop the time code (drop flag, hours, minutes,
 * marker, seconds, pictures), keep closed_gop and broken_link, then finish
 * the header through func_0012CC88.
 */
void IpuParseGopHeader(s32 *ipu) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    dec->fieldE8 = 0;
    dec->newGop = 1;
    dec->gopBasePicture = dec->lastPicture + 1;
    IpuGetBits(ipu, 1);
    IpuGetBits(ipu, 5);
    IpuGetBits(ipu, 6);
    IpuGetBits(ipu, 1);
    IpuGetBits(ipu, 6);
    IpuGetBits(ipu, 6);
    dec->closedGop = IpuGetBits(ipu, 1);
    dec->brokenLink = IpuGetBits(ipu, 1);
    func_0012CC88(ipu);
}

/**
 * Parse a quant-matrix extension (mpeg2decode's quant_matrix_extension) onto
 * the IPU: for each of load_intra / load_non_intra_quantiser_matrix that is
 * set, wait for the IPU and issue SETIQ (0x50000000 intra, 0x58000000
 * non-intra), which takes the 64 matrix bytes straight from the bitstream.
 * The chroma matrices of 4:2:2/4:4:4 streams are not supported and only
 * reported (func_00130288).
 */
void func_0012D100(s32 *ipu) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    s32 load;
    load = IpuGetBits(ipu, 1);
    dec->loadIntraQuant = load;
    if (load != 0) {
        IpuWaitReady(ipu);
        func_0012C380(ipu, 0x50000000);
        IpuWaitReady(ipu);
    }
    load = IpuGetBits(ipu, 1);
    dec->loadNonIntraQuant = load;
    if (load != 0) {
        IpuWaitReady(ipu);
        func_0012C380(ipu, 0x58000000);
        IpuWaitReady(ipu);
    }
    if (IpuGetBits(ipu, 1) != 0) {
        func_00130288((s32)ipu, D_0013BBF8);
    }
    if (IpuGetBits(ipu, 1) != 0) {
        func_00130288((s32)ipu, D_0013BC20);
    }
}

/**
 * Parse a picture-display extension (mpeg2decode's picture_display_extension):
 * the number of frame-centre offsets follows from progressive_sequence,
 * repeat_first_field, top_field_first and the picture structure (1 to 3), and
 * each offset is a 16-bit horizontal and a 16-bit vertical value, both
 * followed by a marker bit.
 *
 * The `!= frame -> 1` test comes first in the interlaced branch, as in the
 * reference: tested as `== frame` first, cc1 lays the two `n = 1` paths out
 * separately where the ROM shares one.
 */
void func_0012D1C8(s32 *ipu) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    s32 n, i;
    if (dec->progressiveSequence != 0) {
        if (dec->repeatFirstField != 0) {
            n = dec->topFieldFirst ? 3 : 2;
        } else {
            n = 1;
        }
    } else {
        if (dec->pictureStructure != 3) {
            n = 1;
        } else {
            n = dec->repeatFirstField ? 3 : 2;
        }
    }
    for (i = 0; i < n; i++) {
        dec->frameCentreHOffset[i] = IpuGetBits(ipu, 16);
        IpuGetBits(ipu, 1);
        dec->frameCentreVOffset[i] = IpuGetBits(ipu, 16);
        IpuGetBits(ipu, 1);
    }
}

/**
 * Skip a copyright extension (mpeg2decode's copyright_extension): copyright
 * flag, identifier, original_or_copy, reserved bits and the three copyright
 * number parts with their marker bits are read and dropped.
 */
void func_0012D2C0(s32 *ipu) {
    IpuGetBits(ipu, 1);
    IpuGetBits(ipu, 8);
    IpuGetBits(ipu, 1);
    IpuGetBits(ipu, 7);
    IpuGetBits(ipu, 1);
    IpuGetBits(ipu, 0x14);
    IpuGetBits(ipu, 1);
    IpuGetBits(ipu, 0x16);
    IpuGetBits(ipu, 1);
    IpuGetBits(ipu, 0x16);
}

/**
 * Decode the picture data: a frame picture arriving while a field picture is
 * still unpaired is reported ("odd number of field pictures") and the pair
 * abandoned; the destination buffer is chosen by picture structure (frames
 * row 0, 1 or 2, slot 2), an unknown structure being reported and decoded into
 * row 0's. Runs func_0012B678 and, if it succeeds, marks the buffer decoded.
 * Returns func_0012B678's result.
 *
 * The case order (frame, top, bottom, default) is the one that gives the
 * ROM's block layout; the other 23 orders of the four labels do not.
 */
s32 func_0012D350(s32 *ipu) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    IpuFrameBuf *frame;
    s32 r;
    if (dec->pictureStructure == 3 && dec->pending != 0) {
        func_00130288((s32)ipu, D_0013BC50);
        dec->pending = 0;
    }
    switch (dec->pictureStructure) {
    case 3:
        frame = (IpuFrameBuf *)dec->frames[0][2];
        break;
    case 1:
        frame = (IpuFrameBuf *)dec->frames[1][2];
        break;
    case 2:
        frame = (IpuFrameBuf *)dec->frames[2][2];
        break;
    default:
        frame = (IpuFrameBuf *)dec->frames[0][2];
        func_00130288((s32)ipu, D_0013BC70);
        break;
    }
    r = func_0012B678(ipu);
    if (r != 0) {
        frame->decoded = 1;
    }
    return r;
}

extern void func_0012DC50(IpuDecoder *dec, s32 frame, s32 last);
extern void func_0012DD60(IpuDecoder *dec, s32 frameA, s32 frameB, s32 last);

/**
 * When `enable` is set, finish a run of `count` items on decoder `dec`: for a
 * frame picture (structure 3) through func_0012DC50 with row 0's frame,
 * otherwise through func_0012DD60 with rows 1 and 2; slot 3 of each row is
 * used for a B picture (coding type 3), slot 0 otherwise. Either way the last index (count - 1) is passed.
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
        if (dec->pictureStructure == 3) {
            if (dec->pictureCodingType == 3) {
                a = dec->frames[0][3];
            } else {
                a = dec->frames[0][0];
            }
            func_0012DC50(dec, a, count - 1);
        } else {
            if (dec->pictureCodingType == 3) {
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

/* The picture dimensions func_0012D768 checks. */
typedef struct {
    s32 _pad0;
    s32 width;                 /* 0x04 */
    s32 height;                /* 0x08 */
    s32 blocks;                /* 0x0C: blocks * blockSize is the bytes needed */
    s32 blockSize;             /* 0x10 */
    u8  _pad14[0x28 - 0x14];
    s32 field28;               /* 0x28: 1 = decode it (func_0012DC50) */
    u8  _pad2C[0x44 - 0x2C];
    s32 params[6];             /* 0x44: copied to IpuDecoder.pictureParams */
    s32 field5C;               /* 0x5C */
    s32 field60;               /* 0x60 */
} IpuPictureSize;

extern s32 sprintf(char *dst, const char *fmt, ...);
extern char D_0013BC90[];

/**
 * Check that the decoder's buffers can hold a picture of `size`: against
 * maxWidth x maxHeight when maxHeight is set, else against bufferSize bytes.
 * A picture that does not fit is reported ("Too small buffer size for %dx%d
 * picture"). Returns 1 if it fits, else 0.
 */
s32 func_0012D768(IpuDecoder *dec, IpuPictureSize *size) {
    char msg[0x100];
    s32 ok;
    s32 maxHeight = dec->maxHeight;
    if (maxHeight != 0) {
        ok = dec->maxWidth >= size->width && maxHeight >= size->height;
    } else {
        ok = dec->bufferSize >= size->blocks * size->blockSize;
    }
    if (!ok) {
        sprintf(msg, D_0013BC90, size->width, size->height);
        func_00130288((s32)dec, msg);
    }
    return ok;
}

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

/* The words of the decoder's owner (IpuDecoder.callbacks) that func_0012DAC0
 * fills in for func_0012DC50. */
typedef struct {
    u8  _pad0[0x10];
    s32 field10;
    s32 _pad14;
    u64 field18;
    u64 field20;
} IpuOwnerFields;

extern u32 D_00137F38[];
extern void func_0012DAC0(IpuDecoder *dec, IpuPictureSize *size, s32 *out10,
                          u64 *out18, u64 *out20);
extern void func_0012E608(IpuDecoder *dec, IpuPictureSize *size);
extern void func_0012D808(IpuDecoder *dec, IpuPictureSize *size);

/**
 * Take a new picture description (`frame`, an IpuPictureSize pointer held in
 * the decoder's frame table; `last` is passed by both callers but never read):
 * let func_0012DAC0 fill three words
 * of the decoder's owner, copy the owner's word 0x10 and a D_00137F38 entry
 * (indexed by bits 5..8 of the owner's word 0x20) into the decoder, along with
 * the picture's parameter words 0x44..0x60. If the decoder's buffers can hold
 * the picture (func_0012D768) and it is flagged for decoding (field28 == 1),
 * decode it by func_0012E608 or func_0012D808 (by fieldB0) and run the state
 * transition func_0012DA98.
 *
 * The owner pointer is re-read from the decoder for each use and the two
 * checks share one `&&`: holding the owner in a local across the call, or two
 * early returns, gives a different body (26/68 words).
 */
void func_0012DC50(IpuDecoder *dec, s32 frame, s32 last) {
    IpuPictureSize *size = (IpuPictureSize *)frame;
    IpuOwnerFields *owner = (IpuOwnerFields *)dec->callbacks;
    func_0012DAC0(dec, size, &owner->field10, &owner->field18, &owner->field20);
    dec->field80 = ((IpuOwnerFields *)dec->callbacks)->field10;
    dec->field88 = D_00137F38[(s32)(((IpuOwnerFields *)dec->callbacks)->field20 >> 5) & 0xF];
    dec->fieldCC = size->field5C;
    dec->fieldD0 = size->field60;
    dec->pictureParams[0] = size->params[0];
    dec->pictureParams[1] = size->params[1];
    dec->pictureParams[2] = size->params[2];
    dec->pictureParams[3] = size->params[3];
    dec->pictureParams[4] = size->params[4];
    dec->pictureParams[5] = size->params[5];
    if (func_0012D768(dec, size) && size->field28 == 1) {
        if (dec->fieldB0 != 0) {
            func_0012E608(dec, size);
        } else {
            func_0012D808(dec, size);
        }
        func_0012DA98((s32 *)dec);
    }
}

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

/* One register-shadow record: the packed register value, its field mask and
 * two caller words. 0x18 bytes. */
typedef struct {
    u64 key;                   /* 0x00: func_0012EAA0(index, value) */
    u64 mask;                  /* 0x08: D_00137F78[index].mask */
    s32 value;                 /* 0x10 */
    s32 extra;                 /* 0x14 */
} RegShadowEntry;

typedef struct {
    u8 _pad0[0x44];
    RegShadowEntry *entries;   /* 0x44: up to 64 records */
    s32 count;                 /* 0x48 */
} RegShadowTable;

/* The 10 packed-register field descriptors func_0012EAA0 indexes. */
typedef struct {
    u64 base;
    u64 mask;
} RegFieldDesc;
extern RegFieldDesc D_00137F78[];
extern u64 func_0012EAA0(u32 index, u64 value);

/**
 * Look up the packed register value func_0012EAA0(index, value) in the
 * shadow table at obj->field_0x40 and return the matching record's `value`
 * word (0 if none). The table is then written at the index the search stopped
 * on (the match, or one past the end) when that is below 64: key, a4, a3 and
 * the field's mask go in, and the record count is incremented — on a hit as
 * well as on an append, exactly as the ROM does.
 *
 * The comparison is spelled `key == entries[i].key`: the other way round cc1
 * swaps the two `bnel` operands (2/61 words).
 */
s32 func_0012EE28(s32 *obj, u32 index, u64 value, s32 a3, s32 a4) {
    RegShadowTable *tbl = (RegShadowTable *)obj[0x10];
    RegShadowEntry *entries = tbl->entries;
    s32 found = 0;
    u64 key = func_0012EAA0(index, value);
    s32 i;
    for (i = 0; i < tbl->count; i++) {
        if (key == entries[i].key) {
            found = entries[i].value;
            break;
        }
    }
    if (i < 64) {
        tbl->count++;
        entries[i].key = key;
        entries[i].extra = a4;
        entries[i].value = a3;
        entries[i].mask = D_00137F78[index].mask;
    }
    return found;
}

/* An MPEG-2 pack header as func_0012EF20 records it. */
typedef struct {
    u32 scrExtension;          /* 0x0: system_clock_reference_extension (9 bits) */
    u32 scrBaseLow;            /* 0x4: SCR base bits 31..0 */
    u32 scrBaseHigh;           /* 0x8: SCR base bit 32 */
    s32 hasSystemHeader;       /* 0xC: a system header (0x000001BB) follows */
} MpegPackHeader;

/* Declared without a prototype: the ROM passes the header in $a1 as well,
 * which func_0012F070's one-parameter definition below does not read. */
extern s32 func_0012F070();

/**
 * Parse an MPEG-2 pack header from the bitstream `bs` (func_0012E980 =
 * get bits, func_0012E9D0 = marker bit): skip the pack start code and the
 * '01' prefix, read the 33-bit SCR base in its 3/15/15-bit pieces and the
 * 9-bit SCR extension, skip the mux rate and reserved bits, and consume the
 * stuffing bytes. If a system header start code (0x000001BB) follows, it is
 * flagged and skipped by func_0012F070. Always returns 1.
 */
s32 func_0012EF20(u64 *bs, MpegPackHeader *hdr) {
    u32 i = 0;
    u32 scr32to30, scr29to15, scr14to0, stuffing;
    func_0012E980(bs, 0x22);                    /* pack start code + '01' */
    scr32to30 = func_0012E980(bs, 3);
    func_0012E9D0(bs);
    scr29to15 = func_0012E980(bs, 15);
    func_0012E9D0(bs);
    scr14to0 = func_0012E980(bs, 15);
    func_0012E9D0(bs);
    hdr->scrExtension = func_0012E980(bs, 9);
    func_0012E980(bs, 0x1E);                    /* marker, mux rate, markers, reserved */
    stuffing = func_0012E980(bs, 3);
    hdr->scrBaseHigh = (scr32to30 >> 2) & 1;
    hdr->scrBaseLow = (scr32to30 << 30) | (scr29to15 << 15) | scr14to0;
    for (i = 0; i < stuffing; i++) {
        func_0012E980(bs, 8);
    }
    if (func_0012E8C8(bs, 32) == 0x1BB) {
        hdr->hasSystemHeader = 1;
        func_0012F070(bs, hdr);
    } else {
        hdr->hasSystemHeader = 0;
    }
    return 1;
}

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

struct AllocRegion;
extern void func_0012FB48(s32 *arg0, s32 arg1, s32 arg2);
extern s32 func_0012FB80(s32 arg0, struct AllocRegion *region, s32 size, u32 align);
extern void func_0012FB60(s32 *arg0);
extern void func_00130118(void *arg0);
extern void func_0012F9C8(s32 *arg0);
extern s32 func_0012FA18(s32 *arg0);
extern void func_00130AA0(void *arg0);
extern char D_0013BD20[];
extern u8 D_00130A90[];

/**
 * Set up the IPU MPEG decoder for the FMV stream object `obj` in the
 * caller's work area `buf`/`size`: clear the whole area, place the decoder
 * context at its first word-aligned byte (obj[0x10] points at it) and, when
 * fewer than 0x10C0 bytes remain, report it (D_0013BD20) and return 0.
 * Otherwise give the rest above +0x10C0 to the bump allocator at +0x108
 * (func_0012FB48), reset the stream object's counters and its two pairs of
 * 64-bit timestamps (-1 = none), clear or seed the decoder's state words,
 * hook the stream callbacks D_00130A90 / func_00130AA0, take a 0x600-byte
 * block (func_0012FB80), initialise the IPU (func_00130118) and the stream
 * object's tables (func_0012F9C8, func_0012FA18), point the 3x3 used slots
 * of frames[][] at their 0x68-byte buffers, reset the allocator
 * (func_0012FB60) and point field81C at scratchpad 0x70003600.
 *
 * The success path has no `return`: the ROM ends with the scratchpad
 * address still in $v0 from the field81C store, which is what falling off
 * the end gives (an explicit `return` of that value makes cc1 form the
 * constant twice, 12 of 130 words). The only caller, FmvStreamInit,
 * ignores the result. The two closing zero stores are written 0x84C before
 * 0x854: in the ROM order cc1 issues them swapped (2 of 130).
 */
void *IpuInitDecoder(s32 *obj, u8 *buf, u32 size) {
    IpuDecoder *dec;
    u32 left;
    memset(buf, 0, size);
    dec = (IpuDecoder *)(((u32)buf + 3) >> 2 << 2);
    left = size - ((u8 *)dec - buf);
    if (left < 0x10C0) {
        func_00130288((s32)dec, D_0013BD20);
        return 0;
    }
    obj[0x10] = (s32)dec;
    func_0012FB48((s32 *)((u8 *)dec + 0x108), (s32)((u8 *)dec + 0x10C0), left - 0x10C0);
    obj[0] = 0;
    obj[1] = 0;
    obj[2] = 0;
    *(s64 *)&obj[4] = -1;
    *(s64 *)&obj[6] = -1;
    *(s64 *)&obj[8] = 0;
    *(s64 *)&obj[10] = -1;
    *(s64 *)&obj[12] = -1;
    *(s64 *)&obj[14] = 0;
    dec->pictureParams[0] = 0;
    dec->pictureParams[1] = 0;
    dec->pictureParams[2] = 0;
    dec->pictureParams[3] = 0;
    dec->pictureParams[4] = 0;
    dec->pictureParams[5] = 0;
    dec->fieldCC = 0;
    dec->fieldD0 = 0;
    dec->firstPictureStructure = 0;
    dec->_padD8 = 0;
    dec->maxWidth = 0;
    dec->maxHeight = 0;
    dec->bufferSize = 0;
    dec->fieldE8 = 0;
    dec->state = 0;
    dec->field0C = 0;
    dec->field14 = 0;
    dec->field2C = 0;
    dec->field34 = 0;
    dec->field3C = 0;
    dec->fieldF0 = -1;
    dec->field1C = D_00130A90;
    dec->field24 = func_00130AA0;
    dec->field44 = func_0012FB80((s32)dec, (struct AllocRegion *)((u8 *)dec + 0x108), 0x600, 8);
    dec->field48 = 0;
    dec->fieldFC = 0;
    dec->field100 = 0;
    dec->field104 = 0;
    dec->field70 = 0;
    dec->field78 = 0;
    dec->field80 = -1;
    dec->field88 = 0;
    dec->field90 = 0;
    dec->fieldAC = 0;
    dec->field94 = -1;
    dec->field98 = -1;
    dec->field9C = -1;
    dec->callbacks = obj;
    dec->fieldB0 = 1;
    func_00130118(dec);
    func_0012F9C8(obj);
    func_0012FA18(obj);
    dec->frames[0][0] = (s32)((u8 *)dec + 0x1E8);
    dec->frames[0][1] = (s32)((u8 *)dec + 0x250);
    dec->frames[0][3] = (s32)((u8 *)dec + 0x2B8);
    dec->frames[1][0] = (s32)((u8 *)dec + 0x320);
    dec->frames[1][1] = (s32)((u8 *)dec + 0x388);
    dec->frames[1][3] = (s32)((u8 *)dec + 0x3F0);
    dec->frames[2][0] = (s32)((u8 *)dec + 0x458);
    dec->frames[2][1] = (s32)((u8 *)dec + 0x4C0);
    dec->frames[2][3] = (s32)((u8 *)dec + 0x528);
    func_0012FB60((s32 *)((u8 *)dec + 0x108));
    dec->lastPicture = -1;
    dec->gopBasePicture = 0;
    dec->field81C = (void *)0x70003600;
    dec->newGop = 0;
}

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

/* One handler slot of the table at obj->field_0x40 (installed by
 * func_0012FA70): the callback and the word passed back to it. */
typedef struct {
    s32 (*fn)(s32 *obj, s32 *req, s32 arg);
    s32 arg;
} HandlerSlot;

typedef struct {
    u8 _pad0[0xC];
    HandlerSlot slot[1];       /* 0x0C: indexed by the request's first word */
} HandlerTable;

/**
 * func_0012FA98(obj, req): if obj and its handler table obj->field_0x40 are
 * non-null and the slot named by req[0] has a callback, call
 * callback(obj, req, slot.arg) and return its result; otherwise return 0.
 *
 * The earlier note here ("ret kept in $7 and a plain beqz are forms this cc1
 * won't reproduce", 92%) described a spelling, not a wall: indexing a typed
 * slot array (`tbl->slot[*req].fn`, `.arg`) instead of `tbl[*req * 2 + 3]`
 * gives the ROM's allocation and branch exactly.
 */
s32 func_0012FA98(s32 *obj, s32 *req) {
    s32 ret = 0;
    if (obj != 0) {
        HandlerTable *tbl = (HandlerTable *)obj[0x10];
        if (tbl != 0 && tbl->slot[*req].fn != 0) {
            ret = tbl->slot[*req].fn(obj, req, tbl->slot[*req].arg);
        }
    }
    return ret;
}

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
 * of the frame table — for a frame picture via func_0012DC50 (row 0), else via
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
    } else if (dec->pictureStructure == 3) {
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

extern char D_0013BDF8[];
extern u8 D_00138040[];
extern u8 D_00138080[];
extern void func_001307B0(s32 *obj, u32 cmd, u32 madr);
extern void func_00130428(s32 *callbacks);

/**
 * Parse an MPEG-2 sequence header (mpeg2decode's sequence_header) onto the
 * IPU: reset firstPictureStructure, take horizontal_size and vertical_size
 * (a height above 2800 is reported through func_00130288; aspect ratio and
 * frame rate are dropped), bit_rate_value and vbv_buffer_size. For each of
 * the intra and non-intra quantiser matrices, issue SETIQ (0x50000000 /
 * 0x58000000) to load it from the bitstream when the header carries one,
 * else feed the default matrix (D_00138040 / D_00138080) through
 * func_001307B0. Then finish with the extensions (func_0012CC88) and pass
 * the callbacks to func_00130428.
 */
void func_00130300(s32 *ipu) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    u32 bits;
    dec->firstPictureStructure = 0;
    bits = IpuGetBits(ipu, 32);
    dec->horizontalSize = bits >> 20;
    dec->verticalSize = (bits >> 8) & 0xFFF;
    if (dec->verticalSize > 0xAF0) {
        func_00130288((s32)dec, D_0013BDF8);
    }
    bits = IpuGetBits(ipu, 30);
    dec->bitRateValue = bits >> 12;
    dec->vbvBufferSize = (bits >> 1) & 0x3FF;
    if ((dec->loadIntraQuant = IpuGetBits(ipu, 1)) != 0) {
        IpuWaitReady(ipu);
        func_0012C380(ipu, 0x50000000);
        IpuWaitReady(ipu);
    } else {
        func_001307B0(ipu, 0x50000000, (u32)D_00138040);
    }
    if ((dec->loadNonIntraQuant = IpuGetBits(ipu, 1)) != 0) {
        IpuWaitReady(ipu);
        func_0012C380(ipu, 0x58000000);
        IpuWaitReady(ipu);
    } else {
        func_001307B0(ipu, 0x58000000, (u32)D_00138080);
    }
    func_0012CC88(ipu);
    func_00130428(dec->callbacks);
}

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

extern char D_0013BE10[];
extern char D_0013BE38[];

/**
 * Parse an MPEG-2 sequence extension (mpeg2decode's sequence_extension):
 * mark the stream MPEG-2 (mpeg2 = 1, IPU_CTRL's MPEG-1 bit cleared through
 * func_0012B198), then take progressive_sequence and chroma_format (anything
 * but 4:2:0 is reported) and widen the sequence header's sizes, bit rate and
 * VBV buffer size by their extension bits. A profile_and_level_indication
 * other than 0x48 (MP@ML), 0x58 (SP@ML) or 0x44 (MP@HL) is reported too.
 *
 * The three extension fields are extracted bit rate, vertical, horizontal:
 * that order gives the ROM's register assignment ($21, $20, $19); the other
 * five orders leave 4 to 7 words different.
 */
void func_00130890(s32 *ipu) {
    IpuDecoder *dec = (IpuDecoder *)ipu;
    u32 bits;
    s32 profileAndLevel;
    s32 horizontalExt;
    s32 verticalExt;
    s32 bitRateExt;
    s32 vbvBufferExt;
    dec->mpeg2 = 1;
    func_0012B198(0);
    bits = IpuGetBits(ipu, 28);
    dec->chromaFormat = (bits >> 17) & 3;
    bitRateExt = (bits >> 1) & 0xFFF;
    verticalExt = (bits >> 13) & 3;
    horizontalExt = (bits >> 15) & 3;
    if (dec->chromaFormat != 1) {
        func_00130288((s32)dec, D_0013BE10);
    }
    dec->progressiveSequence = (bits >> 19) & 1;
    profileAndLevel = bits >> 20;
    vbvBufferExt = (u32)IpuGetBits(ipu, 16) >> 8;
    if (profileAndLevel != 0x48 && profileAndLevel != 0x58 && profileAndLevel != 0x44) {
        func_00130288((s32)dec, D_0013BE38);
    }
    dec->horizontalSize = (horizontalExt << 12) | (dec->horizontalSize & 0xFFF);
    dec->verticalSize = (verticalExt << 12) | (dec->verticalSize & 0xFFF);
    dec->bitRateValue += bitRateExt << 18;
    dec->vbvBufferSize += vbvBufferExt << 10;
}

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

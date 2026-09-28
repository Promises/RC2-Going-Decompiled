/*
 * cod/0213D0 (0x121450..0x121AB7): libgcc's _moddi3.o, between the libgcc.a
 * member _floatdidf.o and _muldi3.o, which the build links from GCC's own
 * source (going-decompiled/libgcc/, RULING #8206; carve: task #893). It holds
 * the ROM's asm for __moddi3 plus this project's portable C, which is a
 * behavioural spec, not GCC source text.
 */
#include "common.h"

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0213D0", __moddi3);
#else
s64 __moddi3(s64 a, s64 b) {
    return a % b;
}
#endif

/*
 * __muldi3 (libgcc `__muldi3`, 0x121AB8) is not in this unit: the EE build links
 * it from GCC's own libgcc2.c as the libgcc.a member _muldi3.o
 * (going-decompiled/libgcc/, RULING #8206), placed right after this unit,
 * before cod/021A98. The portable build keeps the behavioural equivalent: the low 64
 * bits of the product, a * b.
 */
#ifdef TARGET_NATIVE
s64 __muldi3(s64 a, s64 b) {
    return a * b;
}
#endif

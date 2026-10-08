/*
 * cod/031380 (0x131400..0x13142F): the code between the two libm.a members the
 * build links from newlib's own source (going-decompiled/libm/, RULING #9783):
 * s_isnan.o (isnan, 0x1313C8) before it and w_sqrt.o (sqrt, 0x131430) after
 * it. Split out of cod/022FA8 when those two became `lib` subsegments
 * (task #1856).
 */
#include "common.h"

/* 0x123028 is libgcc dp-bit.o's dpcmp. On the EE the address is the linked
 * libgcc.a member dp-bit.o (task #1837), so the call names the member's own
 * symbol, dpcmp (symbol_addrs names 0x123028 so too); the portable build calls
 * its TARGET_NATIVE stand-in, func_00123028 (cod/022FA8). */
#ifdef TARGET_NATIVE
extern s32 func_00123028(s64 a, s64 b);
#define DPCMP func_00123028
#else
extern s32 dpcmp(s64 a, s64 b);
#define DPCMP dpcmp
#endif

/**
 * libm.a s_matherr.o's matherr(struct exception *x): the default error hook
 * the fdlibm wrappers call, which declines every error (returns 0). w_sqrt.o's
 * `matherr` relocation (.text+0xac) is the ROM's jal to this address, and
 * the link binds that name here (going-decompiled/libgcc/LINK_ALIASES).
 *
 * arg0 is the struct exception; arg0[1] is its arg1 field (offset 0x8).
 * newlib's libm/common/s_matherr.c tests `x->arg1 != x->arg1` (a NaN check)
 * and returns 0 on both paths, so the soft-float compare dpcmp (libgcc
 * dp-bit.o, 0x123028) is called and its result discarded (DPCMP above picks
 * the per-arm name).
 */
s32 func_00131400(s64 *arg0) {
    s64 v = arg0[1];
    DPCMP(v, v);
    return 0;
}

/* 0x131424..0x13142F: CD fill, a stray `addiu $sp,$sp,0x90` word, CD fill —
 * link debris between s_matherr.o and w_sqrt.o, not code (FACT #7418). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/031380", func_00131424);

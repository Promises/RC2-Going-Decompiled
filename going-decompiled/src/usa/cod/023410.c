/*
 * cod/023410 (0x123490..0x1234BF): 0x30 bytes the SN linker left behind when
 * it dead-stripped 11 of the SDK libgcc.a fp-bit.o's 15 functions. They sit
 * between the linked members fp-bit-pack (__pack_f __unpack_f) and fp-bit-make
 * (__make_fp fptodp), task #1854. Six (word, 0) pairs; each first word equals
 * the final delay-slot word of some stripped function, but there are six pairs
 * for eleven functions, so how they got here is not known (NOTE #9777). No
 * code reaches them. They stay the ROM's words, never source.
 */
#include "common.h"

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/023410", func_00123490);

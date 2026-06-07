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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001158F4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00115AC0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AA90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AAF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AB10);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011AC90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ACA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ACB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ACC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ACD0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B0B0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B658);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B6C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B728);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B800);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B8D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B970);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B978);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011B9C0);

extern void func_0011B050(s32 count, s32 *value);

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

/* func_0011BAC8(arg0): initialise the global list head D_0013CA40 — store arg0
 * at +0x0, clear the count at +0x4, point both head (+0x8) and tail (+0xC) links
 * at the inline first slot (+0x10); return &D_0013CA40. ~98.5% — ee-gcc's
 * scheduler always orders the three stores 0x8,0xc,0x4 regardless of source
 * order, but the original is 0x8,0x4,0xc. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BAC8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011BFC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C000);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C090);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C1F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C7E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011C820);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CB58);

/* func_0011CB90(index, key, value): store a (key,value) pair into one of two
 * parallel 8-byte-stride tables selected by the sign of index (D_0013CF6C for
 * index >= 0, D_0013CF64 for index < 0); key at slot+0x0, value at slot+0x4.
 * ~77% — the original hoists index*8 into the bgez delay slot and orders the
 * two stores value-then-key, a scheduling shape ee-gcc won't reproduce here.
 * Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CB90);

/* func_0011CBC0(index): clear the key word (slot+0x0) of the entry at index in
 * the sign-selected table pair (same addressing as func_0011CB90). ~77% — same
 * branch-delay scheduling mismatch as func_0011CB90. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CBC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CBE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CD20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CD60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CDA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CEC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CF74);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011CF78);

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
 * count; stores counter+1 back; returns &slot[index] (0x40-byte slots). ~59% —
 * blocked by div register allocation and the `break 0,7` div-check trap that
 * GNU as encodes differently from the original (`break 7`). Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D208);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D238);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D2F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D350);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", rename);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D450);

/* func_0011D590(arg0): link the element arg0->field_0x34 into its manager's
 * (elem->field_0x40) active list — patching the tail's back-link (+0x3C) or the
 * head (+0xC) — then copy a block of transform/state fields from arg0 into the
 * element, and kick processing via func_0011B8D8() when the manager's count
 * (+0x0) is non-negative and busy flag (+0x4) is clear. ~86%: behaviour fully
 * recovered, but ee-gcc won't reproduce the original's branch-likely with an
 * annulled speculative load on the head/tail test. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D590);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D620);

/* func_0011D810: validity predicate for the handle in arg0 — returns 1 iff
 * arg0[0] points to a live object, arg0[1] matches obj[6] (the +0x18 id/gen),
 * and obj[4] (+0x10) bit 0 is set; else 0. ~65% — ee-gcc collapses the final
 * if/else into `andi v0,v0,1` and inverts the id-check branch (beql) instead of
 * the original two-exit branch shape. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D810);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D850);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D868);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D8C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D950);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011D9C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011DD48);

extern void func_0011DD48(void);
extern void func_0011AC60(s32 handle);
extern s32 D_00134734;

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E740);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E828);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011E8A8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011ED60);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011EED8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F130);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F170);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F364);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F5E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F628);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F640);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F6C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F700);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F710);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F718);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F818);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F828);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F864);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F868);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F878);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F8C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F8D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F938);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011F9E4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FA18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FA20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FA48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FA50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FA60);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FAA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FAB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FB8C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FC48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0011FC68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00120354);

extern void (*D_00135D34)(void);

/**
 * Invoke the installed callback held in the global function pointer D_00135D34.
 */
void func_00120368(void) {
    D_00135D34();
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00120390);

/**
 * No-op stub (empty body; present as a registered/overridable hook).
 */
void func_001203C0(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001203C8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00121B18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00121B20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001220F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122630);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122760);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122800);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122A40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122A98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122B00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122DA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00122F10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123028);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123078);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123130);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001231C8);

extern void func_00122630(void *args);

/**
 * Pack three 32-bit arguments and one 64-bit argument into a stack record
 * (the 64-bit field is 8-byte aligned at offset 0x10) and pass it to
 * func_00122630.
 */
void func_00123268(s32 arg0, s32 arg1, s32 arg2, s64 arg3) {
    struct {
        s32 a;
        s32 b;
        s32 c;
        s32 pad;
        s64 d;
    } args;
    args.a = arg0;
    args.b = arg1;
    args.c = arg2;
    args.d = arg3;
    func_00122630(&args);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00123298);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001244B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124540);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124568);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001245D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124630);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001246D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124780);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124818);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001248B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001248F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124970);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124980);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124AF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124B88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124C28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124C98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00124E08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001250E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001252E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001253A4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001253A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125588);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125620);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001256D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001257D0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125A20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125D94);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125E54);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125E58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00125F20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00126104);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001261F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00126284);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012646C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001265B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012672C);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00126DBC);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00126E60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00126ED0);

/**
 * Normalise a handle/id: if its top nibble (bits 31..28) equals 7, clear the
 * upper nibble and set bit 31 instead (i.e. remap tag 0x7 to 0x8). Otherwise
 * return arg0 unchanged.
 */
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
s32 func_00126F38(u32 arg0) {
    if (arg0 < 0xA) {
        return D_00137E30[arg0];
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00126F60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127040);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00127218);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128250);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001282A8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128900);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128A48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128B28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00128C18);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012CA48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012CAB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012CBC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012CC88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012CDB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012CFA0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012E088);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012E980);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012E9D0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012EB28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012EE28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012EF20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F070);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F0E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F690);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F738);

/**
 * Stub predicate that always returns 1 (a registered callback whose default
 * answer is "true"/success).
 */
s32 func_0012F940(void) {
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F948);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F998);

/**
 * Predicate: follow arg0->field_0x40 (arg0[0x10]) to a sub-object and return 1
 * if that object's field_0x4 (base[1]) is zero, else 0.
 */
s32 func_0012F9B8(s32 *arg0) {
    s32 *base = (s32 *)arg0[0x10];
    return base[1] == 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012F9C8);

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
 * a scheduling shape ee-gcc won't reproduce here. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FA70);

/* func_0012FA98(arg0, arg1): if arg0 and its table arg0->field_0x40 are non-null,
 * fetch the destructor at table[*arg1*2 + 3] and, if set, call
 * dtor(arg0, arg1, table[*arg1*2 + 4]); return its result or 0. ~92% — only
 * register allocation (result in a3 vs a2) and a beqz/beqzl delay-slot choice
 * differ. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FA98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FAE8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FE78);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_0012FEC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130020);

/**
 * Clear arg0->field_0x848 and (re)initialise subsystem 1 via func_0012B198(1).
 * The call is a tail call.
 */
void func_00130088(s32 *arg0) {
    *(s32 *)((u8 *)arg0 + 0x848) = 0;
    func_0012B198(1);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130098);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130118);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130178);

/* func_00130240(arg0): dispatch arg0 through func_0011C820 against the global
 * table D_0013BDE8 — the original is a frameless tail call (`j func_0011C820`).
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130288);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001307B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130890);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001309C0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130AB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130B18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130B80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130C68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130DB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130E20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00130E88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001310C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001313C4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", s_isnan);

extern s32 func_00123028(s64 a, s64 b);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131620);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131668);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131670);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001316C0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001317E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131850);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131908);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001319B0);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131AE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131CA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131CB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131CB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00131D98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132028);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132210);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132220);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001322B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132318);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132498);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001325E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001326D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132818);

extern void func_00132DF0(s32 sel, s32 count, void *data, s32 arg3, s32 arg4);

/**
 * Invoke func_00132DF0 with selector 6, count 4, arg0 passed by address in a
 * stack local, and zero for the two trailing arguments.
 */
void func_00132858(s32 arg0) {
    s32 value = arg0;
    func_00132DF0(6, 4, &value, 0, 0);
}

/**
 * Invoke func_00132DF0 with selector 9, count 8, and a stack record holding
 * arg0 and arg1; the two trailing arguments are zero.
 */
void func_00132888(s32 arg0, s32 arg1) {
    s32 args[2];
    args[0] = arg0;
    args[1] = arg1;
    func_00132DF0(9, 8, args, 0, 0);
}

/* func_001328C0: builds a 0x1C-byte record for func_00132DF0 (selector 0x60) —
 * word 0 = arg0, then either a 24-byte copy of *arg1 or a -1 sentinel. Not
 * matched: the 24-byte payload sits at a *misaligned* offset 4 and the source
 * is itself unaligned, so the original copies it with unaligned
 * ldl/ldr/sdl/sdr. Reproducing that requires a packed (alignment-1) struct copy
 * that ee-gcc 2.9 won't emit from natural C — an aligned struct lands the
 * payload at offset 8 with aligned ld/sd instead. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001328C0);

/**
 * Invoke func_00132DF0 with selector 0xB, count 4, arg0 passed by address in a
 * stack local, and zero for the two trailing arguments.
 */
void func_00132938(s32 arg0) {
    s32 value = arg0;
    func_00132DF0(0xB, 4, &value, 0, 0);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132968);

/**
 * Invoke func_00132DF0 with selector 0x4E, count 0xC, and a stack record holding
 * arg0, arg1 and arg2; the two trailing arguments are zero.
 */
void func_001329B0(s32 arg0, s32 arg1, s32 arg2) {
    s32 args[3];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    func_00132DF0(0x4E, 0xC, args, 0, 0);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001329F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132A58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132AA0);

/**
 * Invoke func_00132DF0 with selector 0x16, count 4, arg0 passed by address in a
 * stack local, and zero for the two trailing arguments.
 */
void func_00132AF8(s32 arg0) {
    s32 value = arg0;
    func_00132DF0(0x16, 4, &value, 0, 0);
}

/**
 * Invoke func_00132DF0 with selector 0x17, count 4, arg0 passed by address in a
 * stack local, and zero for the two trailing arguments.
 */
void func_00132B28(s32 arg0) {
    s32 value = arg0;
    func_00132DF0(0x17, 4, &value, 0, 0);
}

/**
 * Invoke func_00132DF0 with selector 0x19, count 4, arg0 passed by address in a
 * stack local, and arg1/arg2 forwarded as the two trailing arguments.
 */
void func_00132B58(s32 arg0, s32 arg1, s32 arg2) {
    s32 value = arg0;
    func_00132DF0(0x19, 4, &value, arg1, arg2);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132B88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132C08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132C48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00132DF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001330D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133108);

/* func_00133220: sets the global flag D_001A74C4 to 1 and returns 1. The store
 * is gp-relative (%gp_rel(D_001A74C4)($28)) in the original because the symbol
 * lives in small-data; without small-data/$gp symbol setup ee-gcc emits a
 * lui/%hi+%lo pair instead, so it can't match yet. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133220);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133230);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133250);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133300);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133340);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001333C0);

/**
 * Invoke func_00132DF0 with selector 0x2E, count 4, arg0 passed by address in a
 * stack local, and zero for the two trailing arguments.
 */
void func_00133400(s32 arg0) {
    s32 value = arg0;
    func_00132DF0(0x2E, 4, &value, 0, 0);
}

/**
 * Invoke func_00132DF0 with selector 0x32, count 4, arg0 passed by address in a
 * stack local, and arg1/arg2 forwarded as the two trailing arguments.
 */
void func_00133430(s32 arg0, s32 arg1, s32 arg2) {
    s32 value = arg0;
    func_00132DF0(0x32, 4, &value, arg1, arg2);
}

/**
 * Invoke func_00132DF0 with selector 0x4F, count 4, arg0 passed by address in a
 * stack local, and arg1/arg2 forwarded as the two trailing arguments.
 */
void func_00133460(s32 arg0, s32 arg1, s32 arg2) {
    s32 value = arg0;
    func_00132DF0(0x4F, 4, &value, arg1, arg2);
}

extern void func_00132C48(s32 arg0, s32 arg1, void *arg2);

/**
 * Invoke func_00132C48 with selector 0x36 and count 4, passing arg0 by address
 * in a stack local as the third (data) argument.
 */
void func_00133490(s32 arg0) {
    s32 value = arg0;
    func_00132C48(0x36, 4, &value);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001334B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133580);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133640);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133688);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001336C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133700);

/**
 * Invoke func_00132DF0 with selector 0x51, count 8, and a stack record holding
 * arg0 and arg1; the two trailing arguments are zero.
 */
void func_00133750(s32 arg0, s32 arg1) {
    s32 args[2];
    args[0] = arg0;
    args[1] = arg1;
    func_00132DF0(0x51, 8, args, 0, 0);
}

/**
 * Invoke func_00132DF0 with selector 0x10, count 0x10 (16 bytes), and a stack
 * record of four words (arg0..arg3); the two trailing arguments are zero.
 */
void func_00133788(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 args[4];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    func_00132DF0(0x10, 0x10, args, 0, 0);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001337C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133818);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133848);

/* func_00133890: calls func_00132C48(0x3D, 0, 0) and returns. Not matched: the
 * original keeps a stack frame and uses jal+jr, but ee-gcc sibling-call-
 * optimises this single-call void wrapper into `j func_00132C48`. This cc1 has
 * no flag to suppress the tail-call here. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133890);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001338B8);

/**
 * Invoke func_00132C48 with selector 0x3E and count 0x14 (20 bytes), passing a
 * stack record of five words (arg0..arg4) as the data argument.
 */
void func_001338F0(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 args[5];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    args[4] = arg4;
    func_00132C48(0x3E, 0x14, args);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133928);

/* func_00133960: calls func_00132C48(0x5B, 0, 0) and returns. Not matched: same
 * sibling-call-optimisation blocker as func_00133890 — ee-gcc emits
 * `j func_00132C48` instead of the original's framed jal+jr. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133960);

/* func_00133988: scale arg0 by 1524/741, i.e. (arg0 * 0x5F4) / 0x2E5. ~82%; the
 * only diff is that ee-gcc fills the `jr ra` delay slot with the `mflo`, while
 * the original keeps `mflo` before the return and leaves a nop in the slot — a
 * scheduling choice not expressible in source. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133988);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001339B0);

/* func_001339F0: copy `count` bytes from src to dst via indexed access
 * (dst[i]=src[i]); non-positive count copies nothing. ~88% — the original fills
 * the loop's `bnez` delay slot with the `sb` store while ee-gcc emits the store
 * before the branch and nops the slot (a scheduling choice). Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_001339F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133A28);

/* func_00133A78: calls func_00133A28(0x3E9, 0xB, &D_0014B540) and returns. Not
 * matched: same sibling-call-optimisation blocker as func_00133890 — ee-gcc
 * tail-calls into func_00133A28 rather than keeping the original's stack frame.
 * Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133A78);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180", func_00133AA0);

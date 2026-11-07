/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x00438090, in place
 * of its split asm; the row is config/L11_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

extern int func_L11_0033DAC8(char *, int, float, float, int, int);
extern int func_002B3BA8(char *, int, int, int, float, float);

/**
 * func_L11_00438090 - re-arm the measured stage record's timed effect.
 *
 * Modes below 4 clear +0x270 and the guard +0x282. With the guard set
 * (+0x282 == 1) the timed-effect handle from +0x27C is stored at +0x280 and
 * +0x200 is copied to +0x270; otherwise the class-instance query
 * func_L11_004430B8 (+0xB8, +0xAA) must first answer non-zero, the guard is
 * then set and the same two stores run.
 *
 * @arg0: the moby object; reads +0x20 (mode), +0x68 (state), +0xAA, +0xB8.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00432628.c, function func_L11_00436850 (v1.01 address), author commit 5964c620 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_00436850 -> func_L11_00438090 (this function's v2.00 split name);
 *   - func_002B3BA8_6850 -> func_L11_0033DAC8 (their _6850 suffix disambiguates a
 *   - second prototype of the same callee; this standalone TU spells the plain
 *   - level split name with their 6-argument prototype);
 *   - func_L11_00441868 -> func_L11_004430B8 (v1.01-address callee respelled to the v2.00 level split name).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_00438090(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x20) < 4) {
        *(int *)(state + 0x270) = 0;
        goto done;
    }

    if (*(short *)(state + 0x282) == 1) {
        *(short *)(state + 0x280) =
            func_L11_0033DAC8(arg0, *(int *)(state + 0x27C), 100.0f, 100.0f, 0, 0);
        *(int *)(state + 0x270) = *(int *)(state + 0x200);
    } else {
        if (func_L11_004430B8(*(int *)(arg0 + 0xB8), *(short *)(arg0 + 0xAA)) == 0)
            goto done;
        *(short *)(state + 0x282) = 1;
        *(short *)(state + 0x280) =
            func_L11_0033DAC8(arg0, *(int *)(state + 0x27C), 100.0f, 100.0f, 0, 0);
        *(int *)(state + 0x270) = *(int *)(state + 0x200);
    }
    return;

done:
    *(short *)(state + 0x282) = 0;
}

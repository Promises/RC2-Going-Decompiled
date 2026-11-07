/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x00438150, in place
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

extern void func_L11_00340930(char *, float, float, float, float);
extern void func_L11_0032D918(void *, int, int, int);
extern void func_L11_0032DBF8(void *, int, int);
extern void func_L11_00325748(char *);
extern void func_L11_00325678(char *, int, int, int);
extern void func_L11_00398770(char *, int, float, float);

/**
 * func_L11_00438150 - arm the record for a (mode, active) pair.
 *
 * Clears the guards (+0x282, +0x88), seeds the timer from the halfword at
 * +0x24, stores `active` at +0x286 and `mode` at +0x284, sets the animation
 * bounds through func_L11_00340930, takes flags 0x1000 (dropping 0x46), and
 * by `active` either retires with 7 + class setup 1 (and re-fires the pending
 * camera at +0x50 through the +0x54..+0x56 bytes) or retires with 3 and marks
 * +0x31.
 *
 * @object: the moby object; reads +0x24 (definition), +0x34, +0x68 (state).
 * @mode: stored at state +0x284. @active: non-zero takes the armed path.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00432628.c, function func_L11_00436910 (v1.01 address), author commit 5acf9869 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_00436910 -> func_L11_00438150 (this function's v2.00 split name);
 *   - func_002A0488 -> func_L11_00325678, func_002A05A8 -> func_L11_00325748,
 *   - func_002A7470 -> func_L11_0032D918, func_002A7750 -> func_L11_0032DBF8,
 *   - func_002B60B0 -> func_L11_00340930, func_L11_003973F8 -> func_L11_00398770 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_00438150(char *object, int mode, int active) {
    char *state = *(char **)(object + 0x68);
    unsigned short flags;
    char *definition;
    int cache;

    *(short *)(state + 0x282) = 0;
    *(short *)(state + 0x88) = 0;
    *(float *)(state + 0x20) = *(short *)(state + 0x24);
    *(short *)(state + 0x286) = active;
    func_L11_00340930(object, 0.004444444552063942f, 0.13333334028720856f,
                  0.006981317885220051f, 0.20943954586982727f);
    flags = *(unsigned short *)(object + 0x34);
    definition = *(char **)(object + 0x24);
    cache = *(int *)(definition + 0x10);
    *(unsigned short *)(object + 0x34) = (flags | 0x1000) & 0xFFB9;
    *(int *)(object + 0x98) = cache;
    *(unsigned char *)(object + 0x7F) = 0;
    if (*(short *)(state + 0x286) != 0) {
        func_L11_0032DBF8(object, 7, 3);
        *(unsigned short *)(object + 0x34) |= 0x100;
        func_L11_0032D918(object, 1, 0, 0);
        func_L11_00325748(object);
        func_L11_00398770(object, 0, 2.0f, 2.0f);
    } else {
        func_L11_0032DBF8(object, 3, 0);
        *(unsigned char *)(object + 0x31) = 1;
        *(unsigned short *)(object + 0x34) &= 0xFEFE;
    }
    *(unsigned char *)(state + 0x2E) = 1;
    *(short *)(state + 0x284) = mode;
    *(unsigned char *)(state + 0x6A) = 0;
    if (*(short *)(state + 0x50) != 0) {
        *(short *)(state + 0x50) = 0;
        func_L11_00325678(object, *(unsigned char *)(state + 0x54),
                      *(unsigned char *)(state + 0x55), *(unsigned char *)(state + 0x56));
        *(short *)(state + 0x52) = 1;
    }
}

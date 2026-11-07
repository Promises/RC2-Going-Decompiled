/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x0044B0C8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SN 2.95.3 v1.36 cc1 with -fopt-stack
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
extern void func_L11_00340930(char *, float, float, float, float);
extern void func_L11_0032D918(char *, int, int, int);
extern void func_L11_0032DBF8(char *, int, int);
extern void func_L11_00325748(char *);
extern void func_L11_00325678(char *, int, int, int);
extern int func_L11_0033C680(char *, char *, char *);
extern void func_L11_00398770(char *, int, float, float);
extern void func_L11_003256A8(char *, int *, int *, int *);
extern void func_L11_0044ACC8(char *a0);

/**
 * func_L11_0044B0C8 - initialise a paired-moby (class 0xD98) instance.
 *
 * Clears the state's +0x88 and +0x366 words, primes the scale timer at +0x20
 * from the halfword at +0x24 and records @active at +0x36E, then arms the
 * moby's rotation envelope (func_L11_00340930) and clears +0x6A. The object's
 * flags (+0x34) gain 0x1000 with bits 2/3/6 dropped, and +0x98 takes the
 * group table's +0x10 value. When @active is set the moby is shown (group
 * 4/2, flag 0x100) and its spawn effect run (func_L11_00398770); otherwise it
 * is hidden (group 3/1, +0x31 set, bit 0 of +0x34 dropped, its +0x43-based
 * cleanup run). The sub-moby is then spawned (func_L11_0044ACC8), @mode is
 * recorded at +0x36C, the +0x1F0 collision block handed over
 * (func_L11_0033C680) and, when the state's +0x50 halfword is set, the saved
 * animation (+0x54..+0x56) restored via func_L11_00325678 with +0x52 latched.
 * A live sub-moby also has its tint copied from the parent's colours
 * (func_L11_003256A8 -> func_L11_00325678).
 *
 * @object: the moby.
 * @mode: stored at state+0x36C.
 * @active: nonzero = shown and spawned now.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004465F8.c, function func_L11_004496A0 (v1.01
 * address), author commit 5acf9869 (Flavius). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_004496A0 -> func_L11_0044B0C8 (our split name at the v2.00
 *     place);
 *   - func_002A0488 -> func_L11_00325678;
 *   - func_002A04B8 -> func_L11_003256A8;
 *   - func_002A05A8 -> func_L11_00325748;
 *   - func_002A7470 -> func_L11_0032D918;
 *   - func_002A7750 -> func_L11_0032DBF8;
 *   - func_002B3380 -> func_L11_0033C680;
 *   - func_002B60B0 -> func_L11_00340930;
 *   - func_L11_003973F8 -> func_L11_00398770;
 *   - func_L11_004492A0 -> func_L11_0044ACC8;
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - the sibling func_L11_004492A0, defined in the same upstream TU, is
 *     declared here as an extern under our name (func_L11_0044ACC8); upstream
 *     compiled both in one TU, this row compiles one TU per function.
 *
 */
void func_L11_0044B0C8(char *object, int mode, int active) {
    char *state = *(char **)(object + 0x68);
    int colors[3];
    int green, blue;

    *(short *)(state + 0x88) = 0;
    *(short *)(state + 0x366) = 0;
    *(float *)(state + 0x20) = *(short *)(state + 0x24);
    *(short *)(state + 0x36E) = active;
    func_L11_00340930(object, 0.0055555556900799274f, 0.10333333909511566f,
                  0.0034906589426100254f, 0.41887909173965454f);
    *(u8 *)(state + 0x6A) = 0;
    *(u8 *)(state + 0x2E) = 1;
    *(u8 *)(object + 0x7F) = 0;
    *(u16 *)(object + 0x34) = (*(u16 *)(object + 0x34) | 0x1000) & 0xFFB9;
    *(int *)(object + 0x98) = *(int *)(*(char **)(object + 0x24) + 0x10);
    if (*(short *)(state + 0x36E) != 0) {
        func_L11_0032DBF8(object, 4, 2);
        *(u16 *)(object + 0x34) |= 0x100;
        func_L11_0032D918(object, 0, 0, 0);
        func_L11_00325748(object);
        func_L11_00398770(object, 0, 2.0f, 2.0f);
    } else {
        if (*(u8 *)(object + 0x43) != 0)
            func_L11_0032D918(object, 0, 0, 10);
        func_L11_0032DBF8(object, 3, 1);
        *(u8 *)(object + 0x31) = 1;
        *(u16 *)(object + 0x34) &= 0xFEFE;
    }
    func_L11_0044ACC8(object);
    *(short *)(state + 0x36C) = mode;
    *(int *)(state + 0x200) = 0;
    func_L11_0033C680(object, state + 0x1F0, state + 0x20);
    if (*(short *)(state + 0x50) != 0) {
        *(short *)(state + 0x50) = 0;
        func_L11_00325678(object, *(u8 *)(state + 0x54), *(u8 *)(state + 0x55),
                      *(u8 *)(state + 0x56));
        *(short *)(state + 0x52) = 1;
    }
    if (*(char **)(state + 0x354) != 0) {
        func_L11_003256A8(object, &colors[0], &colors[1], &colors[2]);
        green = colors[1];
        blue = colors[2];
        func_L11_00325678(*(char **)(state + 0x354), colors[0], green, blue);
    }
}

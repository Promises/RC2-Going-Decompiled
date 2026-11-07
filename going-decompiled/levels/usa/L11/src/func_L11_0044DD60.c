/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x0044DD60, in place of its split asm; the row is
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

/**
 * func_L11_0044DD60 - initialise a gate-class (0xDCF) moby.
 *
 * The gate-class sibling of func_L11_0044B0C8: clears the +0x4CA gate and the
 * +0x88 word, primes the +0x20 timer from +0x24, records @active at +0x4D2,
 * arms the rotation envelope (func_L11_00340930) and re-flags the object the
 * same way. @active selects the shown (group 4/2, flag 0x100,
 * func_L11_00398770) or hidden (group 3/0) setup. Both sub-moby slots of the
 * +0x4A0 pair are re-shown (bit 0 of +0x34 set, +0x31 cleared, +0x98 reloaded
 * from their group tables), @mode lands at +0x4D0, the +0x1F0 collision block
 * is handed over (func_L11_0033C680) and a saved animation (+0x50/+0x54..56)
 * is restored via func_L11_00325678 as there.
 *
 * @object: the moby.
 * @mode: stored at state+0x4D0.
 * @active: nonzero = shown now.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004465F8.c, function func_L11_0044C328 (v1.01
 * address), author commit 5acf9869 (Flavius). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_0044C328 -> func_L11_0044DD60 (our split name at the v2.00
 *     place);
 *   - func_002A0488 -> func_L11_00325678;
 *   - func_002A05A8 -> func_L11_00325748;
 *   - func_002A7470 -> func_L11_0032D918;
 *   - func_002A7750 -> func_L11_0032DBF8;
 *   - func_002B3380 -> func_L11_0033C680;
 *   - func_002B60B0 -> func_L11_00340930;
 *   - func_L11_003973F8 -> func_L11_00398770;
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here.
 *
 */
void func_L11_0044DD60(char *object, int mode, int active) {
    char *state = *(char **)(object + 0x68);
    char **slot;
    char *collision, *health;
    int i;

    *(short *)(state + 0x4CA) = 0;
    *(short *)(state + 0x88) = 0;
    *(u8 *)(state + 0x2E) = 1;
    *(float *)(state + 0x20) = *(short *)(state + 0x24);
    *(short *)(state + 0x4D2) = active;
    func_L11_00340930(object, 0.004444444552063942f, 0.13333334028720856f,
                  0.0011635529808700085f, 0.10471977293491364f);
    *(u8 *)(object + 0x7F) = 0;
    *(u16 *)(object + 0x34) = (*(u16 *)(object + 0x34) | 0x1000) & 0xFFB9;
    *(int *)(object + 0x98) = *(int *)(*(char **)(object + 0x24) + 0x10);
    if (*(short *)(state + 0x4D2) != 0) {
        func_L11_0032DBF8(object, 4, 2);
        *(u16 *)(object + 0x34) |= 0x100;
        func_L11_0032D918(object, 2, 0, 0);
        func_L11_00325748(object);
        func_L11_00398770(object, 0, 2.0f, 2.0f);
    } else {
        func_L11_0032DBF8(object, 3, 0);
        *(u8 *)(object + 0x31) = 1;
        *(u16 *)(object + 0x34) &= 0xFEFE;
    }
    *(u8 *)(state + 0x6A) = 0;
    collision = state + 0x1F0;
    health = state + 0x20;
    for (i = 0; i < 2; i++) {
        slot = (char **)(state + 0x4A0 + i * 4);
        if (*slot != 0) {
            *(u16 *)(*slot + 0x34) &= 0xFFFE;
            *(u8 *)(*slot + 0x31) = 1;
            *(int *)(*slot + 0x98) = *(int *)(*(char **)(*slot + 0x24) + 0x10);
        }
    }
    *(short *)(state + 0x4D0) = mode;
    *(int *)(state + 0x200) = 0;
    func_L11_0033C680(object, collision, health);
    if (*(short *)(state + 0x50) != 0) {
        *(short *)(state + 0x50) = 0;
        func_L11_00325678(object, *(u8 *)(state + 0x54), *(u8 *)(state + 0x55),
                      *(u8 *)(state + 0x56));
        *(short *)(state + 0x52) = 1;
    }
}

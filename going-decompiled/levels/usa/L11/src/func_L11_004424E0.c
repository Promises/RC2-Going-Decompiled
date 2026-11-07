/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x004424E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

/* The level's copies of the group helpers at these v2.00 places. */
extern s32 func_L11_00418968(u8 *);
extern s32 func_L11_004189A8(u8 *);
extern void func_L11_00418A18(u8 *, u8 *);

/* The weight word (+0x20) of OTHER's state. */
#define WEIGHT (*(f32 *)(*(u8 **)(other + 0x68) + 0x20))

/**
 * func_L11_004424E0 - add another object to the group in the state's current
 * slot, counting it by class.
 *
 * The current slot is state+0x120 indexed by the word at state+0x18C. When
 * that group exists and func_L11_00418968 accepts it, @other's class (+0xAA)
 * is counted: 0xD98 at +0x100, 0xDCF +0x104, 0xDF4 +0x108, 0xDF7 +0x10C,
 * 0xDF9 +0x110 (unless func_L11_004189A8 says the group refuses it: return 0),
 * 0xD37 +0x114; for those classes the weight word of @other's state (+0x20)
 * is set to 1.0. Then @other is added to the group (func_L11_00418A18).
 *
 * @obj: the group's owner; its state is at +0x68.
 * @other: the object to add.
 * Return: 1 when added, else 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_0043E1A0.c, function func_L11_00440C90 (v1.01
 * address), author commit c7d9e30a (bigbeno37). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_00440C90 -> func_L11_004424E0 (our split name at the v2.00 place);
 *   - func_L02_003D77B8 -> func_L11_00418968, func_L02_003D77F8 -> func_L11_004189A8,
 *     func_L02_003D7868 -> func_L11_00418A18 (upstream names the L02 copies;
 *     L11 calls its own);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the
 *     u8/s16/s32/f32 typedefs common.h supplied are written here.
 */
s32 func_L11_004424E0(u8 *obj, u8 *other)
{
    u8 *state = *(u8 **)(obj + 0x68);
    u8 *slots = state + 0x120;

    if (*(u8 **)(slots + (*(s32 *)(state + 0x18C) << 2)) != 0 &&
        func_L11_00418968(*(u8 **)(slots + *(s32 *)(state + 0x18C) * 4)) != 0) {
        switch (*(s16 *)(other + 0xAA)) {
        case 0xD98:
            *(s32 *)(state + 0x100) += 1;
            WEIGHT = 1.0f;
            break;
        case 0xDCF:
            *(s32 *)(state + 0x104) += 1;
            WEIGHT = 1.0f;
            break;
        case 0xDF4:
            *(s32 *)(state + 0x108) += 1;
            WEIGHT = 1.0f;
            break;
        case 0xDF7:
            *(s32 *)(state + 0x10C) += 1;
            WEIGHT = 1.0f;
            break;
        case 0xDF9:
            if (func_L11_004189A8(*(u8 **)(slots + (*(s32 *)(state + 0x18C) << 2))) != 0)
                return 0;
            *(s32 *)(state + 0x110) += 1;
            WEIGHT = 1.0f;
            break;
        case 0xD37:
            *(s32 *)(state + 0x114) += 1;
            WEIGHT = 1.0f;
            break;
        }
        func_L11_00418A18(*(u8 **)(state + (*(s32 *)(state + 0x18C) << 2) + 0x120), other);
        return 1;
    }
    return 0;
}

#undef WEIGHT

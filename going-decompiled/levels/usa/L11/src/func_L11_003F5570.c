/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x003F5570, in place
 * of its split asm; the row is config/L11_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

extern void func_L11_003011A8(void *a0);
extern void func_L11_003014F8(f32 *dst, f32 *a, f32 len);
extern f32 func_L11_00301808(f32 a0);
extern f32 func_L11_00301820(f32 a0);
extern f32 func_L11_003022A8(f32 a0, f32 a1);
extern f32 D_L11_001B058C __attribute__((sda));

/**
 * func_L11_003F5570 - place the state's two 0xA0-stride records against the
 * object's direction frame.
 *
 * For each of the two records: builds the base vector from the object's +0xE0
 * direction scaled by the .lit length D_L11_001B058C (unless the record's
 * twin flag halfword already reads 1), then walks the record's two 0x10-stride
 * sub-entries blending their angles into position fields +0x20..+0x28 from
 * +0x40..+0x48 and +0x60..+0x68.
 *
 * @arg0: the moby object; reads +0x68 (state), +0xE0.
 *
 * Non-obvious: the loop counter is recycled through `rec`/`p` (the integer
 * round-trip `i = (s32)rec; p = (u8 *)i;`) - kept verbatim; it is how the
 * retail keeps the record pointer in a fixed register.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_003F4060-g8.c, function func_L11_003F4060 (v1.01 address), author commit b4b0fe9b (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - `__attribute__((sda))` kept verbatim (this toolchain ignores it, warning
 *     only; measured inert at this arm and -G flag);
 *   - func_L11_003F4060 -> func_L11_003F5570 (this function's v2.00 split name);
 *   - D_L11_001B05DC -> D_L11_001B058C (exact v2.00 split dlabel);
 *   - func_00282C88 -> func_L11_003011A8, func_00282F20 -> func_L11_003014F8,
 *   - func_00283180 -> func_L11_00301808, func_00283198 -> func_L11_00301820,
 *   - func_00283B98 -> func_L11_003022A8 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_003F5570(u8 *arg0) {
    u8 *s4 = arg0;
    u8 *s3 = *(u8 **)(s4 + 0x68);
    f32 vec[4];
    u8 *rec;
    u8 *p;
    s32 i, next;

    for (i = 0; i < 2; i = next) {
        s32 j;

        rec = s3 + 0xA0 + i * 0xA0;
        func_L11_003011A8((TI128 *)vec);
        {
            u8 *flagRecord = s3 + i * 2;
            if (*(s16 *)(flagRecord + 0x278) != 1) {
                func_L11_003014F8(vec, (f32 *)(s4 + 0xE0), D_L11_001B058C);
            }
        }
        next = i + 1;
        i = (s32)rec;
        p = (u8 *)i;
        for (j = 1; j >= 0; j--) {
            *(f32 *)(p + 0x40) = func_L11_003022A8(*(f32 *)(p + 0x40), *(f32 *)(p + 0x60));
            *(f32 *)(p + 0x44) = func_L11_003022A8(*(f32 *)(p + 0x44), *(f32 *)(p + 0x64));
            *(f32 *)(p + 0x48) = func_L11_003022A8(*(f32 *)(p + 0x48), *(f32 *)(p + 0x68));
            *(f32 *)(p + 0x20) = vec[0] + func_L11_00301808(*(f32 *)(p + 0x40)) * *(f32 *)(p + 0x80);
            *(f32 *)(p + 0x24) = vec[1] + func_L11_00301820(*(f32 *)(p + 0x44)) * *(f32 *)(p + 0x84);
            *(f32 *)(p + 0x28) = vec[2] - func_L11_00301820(*(f32 *)(p + 0x48)) * *(f32 *)(p + 0x88);
            p += 0x10;
        }
    }
}

/* Level 09 (Dobbo), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2580, RULING #13291 row L09-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x003FFF78, in place of its
 * split asm; the row is going-decompiled/levels/usa/L09/config/L09_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column; the plain-`as` ablation is in the row's FACT, task #2580).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;

/* Declarations in upstream's order. The level's tuning words (our labels at
 * 0x001B1168..0x001B1180 and 0x001B1230) are $gp-relative at -G8 by their
 * size; upstream's `sda` attribute text is kept, and this cc1 ignores it (a
 * warning), so it emits nothing. Every callee is L09's copy of boot code. */
extern void func_L09_002EB3D8(float, void *, void *);
extern float D_L09_001B1170 __attribute__((sda));
extern float D_L09_001B1180 __attribute__((sda));
extern s32 D_L09_001B1230 __attribute__((sda));
extern s32 D_L09_001B1168 __attribute__((sda));
extern u8 D_L09_001B1178 __attribute__((sda));
extern s32 D_L09_001B1174 __attribute__((sda));
extern u16 D_L09_001B117C __attribute__((sda));
extern float func_L09_002EB2F0(char *a0, char *a1);
extern void func_L09_0030E9A0(char *a0, s32 a1, void *a2);
extern void func_L09_002EB128(float *a0, char *a1, float *a2);
extern char *func_L09_003AFC38(float *a0, char *a1, float *a2, s32 a3, s32 t0, float f12, float f13, float f14, float f15);
extern s32 func_L09_0035D9A8(s32 a0, s32 a1, char *a2);
extern s32 func_L09_002EADC8(char *a0);

/**
 * func_L09_003FFF78 - fire an object's alternating projectile volley at its
 * target while the target is close.
 *
 * When the arming byte +0x42B of the object's state (+0x68) is set and the
 * target (+0x390) is within 3.0 of the position at +0x150's +0x10: holds the
 * timer +0x412 at 600 and, when the cooldown +0x426 and the delay +0x428 are
 * both 0, takes the matrix of muzzle (+0x429) + 4 through
 * func_L09_0030E9A0, builds the aim vector between the matrix translation and
 * the point at +0x140 (if +0x441 is set and that point is within 3.0 of the
 * target) or the target itself (func_L09_002EB128), scales it by
 * D_L09_001B1170 / 60 and spawns the shot through func_L09_003AFC38; every
 * D_L09_001B1230-th shot also plays sound D_L09_001B1168. Then it swaps the
 * muzzle, counts the shot (+0x42A), re-arms the delay from D_L09_001B1178 and,
 * after D_L09_001B1174 shots, resets the count and sets the cooldown from
 * D_L09_001B117C. Out of range with no delay pending, it re-arms the delay
 * once func_L09_002EADC8 on the count returns 0.
 *
 * @a0: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEAE8-g8.c, function func_L09_003FEAE8
 * (v1.01 address), author commit 1794e699 (Louis-Philippe Le Sieur). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (BYTE-IDENTICAL), each name to the target of the
 * v2.00 disc's own relocation at this place (task #2580 relverify):
 *   - func_L09_003FEAE8 -> func_L09_003FFF78 (our split name);
 *   - D_L09_001B11B8/11C0/11C4/11C8/11CC/11D0/1280 -> our labels
 *     D_L09_001B1168/1170/1174/1178/117C/1180/1230 (`sda` text kept);
 *   - callees func_00282F20 -> func_L09_002EB3D8, func_00282E80 ->
 *     func_L09_002EB2F0, func_0029FC20 -> func_L09_0030E9A0, func_00282CF0 ->
 *     func_L09_002EB128, func_L09_003AE8C8 -> func_L09_003AFC38, func_002E5CF8
 *     -> func_L09_0035D9A8, func_002829A8 -> func_L09_002EADC8;
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here; upstream's one comment folded into this one.
 */
void func_L09_003FFF78(char *a0) {
    char *s2 = *(char **)(a0 + 0x68);
    float matrix[16];
    float va[4];

    {
        if (*(u8 *)(s2 + 0x42B) != 0 && func_L09_002EB2F0(s2 + 0x390, *(char **)(s2 + 0x150) + 0x10) < 3.0f) {
            *(s16 *)(s2 + 0x412) = 600;
            if (*(s16 *)(s2 + 0x426) == 0 && *(u8 *)(s2 + 0x428) == 0) {
                func_L09_0030E9A0(a0, *(u8 *)(s2 + 0x429) + 4, matrix);
                if (*(u8 *)(s2 + 0x441) && func_L09_002EB2F0(s2 + 0x140, s2 + 0x390) < 3.0f) {
                    func_L09_002EB128(va, s2 + 0x140, matrix + 12);
                } else {
                    func_L09_002EB128(va, s2 + 0x390, matrix + 12);
                }
                func_L09_002EB3D8(D_L09_001B1170 * 0.016666668f, va, va);
                func_L09_003AFC38(matrix + 12, a0, va, 1, 1, (float)*(u8 *)(s2 + 0x34), D_L09_001B1180, 1.0f, 0.5f);
                if (*(u8 *)(s2 + 0x42A) % D_L09_001B1230 == 0)
                    func_L09_0035D9A8(D_L09_001B1168, 0, a0);
                *(u8 *)(s2 + 0x429) ^= 1;
                *(u8 *)(s2 + 0x42A) += 1;
                *(u8 *)(s2 + 0x428) = D_L09_001B1178;
                if (D_L09_001B1174 < *(u8 *)(s2 + 0x42A)) {
                    *(u8 *)(s2 + 0x42A) = 0;
                    *(u16 *)(s2 + 0x426) = D_L09_001B117C;
                }
            }
        } else if (*(u8 *)(s2 + 0x428) == 0) {
            if (func_L09_002EADC8(s2 + 0x42A) == 0)
                *(u8 *)(s2 + 0x428) = D_L09_001B1178;
        }
    }
}

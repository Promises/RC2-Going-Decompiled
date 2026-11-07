/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x0044B2A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SN 2.95.3 v1.36 cc1 with -fopt-stack
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));
extern void func_L11_0032DBF8();
extern void func_L11_0032D918();
extern void func_L11_00325748(char *a0);
extern void func_L11_004431C8(s32 a0, int a1);
extern void func_L11_00324870(char *a0, char *a1);
extern void func_L11_00300F68(char *a0, int a1, int a2);
extern void func_L11_0044AD58(char *self, TI128 *pos);

/**
 * func_L11_0044B2A0 - return a paired-moby (class 0xD98) to its reset state.
 *
 * Switches to group 1/0, runs the group/animation reset
 * (func_L11_0032DBF8, func_L11_0032D918, func_L11_00325748), takes the object
 * out of the shown set (+0x34 keeps 0x47 and loses 0x1000; +0x31 and +0x98
 * cleared) and, when its phase (+0x366) is 1, hands the class counter back
 * (func_L11_004431C8 with 0xD98). The partner pointer (+0x350) and the phase
 * are cleared, a live sub-moby's position is flushed through
 * func_L11_0044AD58, and when the state's +0x2B1 byte is set the 0x2B0 rig is
 * cleared back to unit scale (func_L11_00324870 and func_L11_00300F68 over
 * 0x40 bytes).
 *
 * @a0: the moby; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004465F8.c, function func_L11_00449878 (v1.01
 * address), author commit fa8d6672 (llesieur99); two callee
 * spellings were later changed in 1d296136 (Louis-Philippe Le Sieur,
 * the same account). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_00449878 -> func_L11_0044B2A0 (our split name at the v2.00
 *     place);
 *   - func_00282A60 -> func_L11_00300F68;
 *   - func_323688 -> func_L11_00324870;
 *   - func_324560 -> func_L11_00325748;
 *   - func_32C700 -> func_L11_0032D918;
 *   - func_32C9E0 -> func_L11_0032DBF8;
 *   - func_L11_00441978 -> func_L11_004431C8;
 *   - func_L11_00449330 -> func_L11_0044AD58;
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - the opening brace moved to the signature line (whitespace only);
 *   - the 128-bit typedef `TI128` upstream defines at file scope is written
 *     here (the body copies a 16-byte position through it).
 *
 */
void func_L11_0044B2A0(char *a0) {
    char *s2 = a0;
    char *s1;
    TI128 tmp;

    func_L11_0032DBF8(a0, 1, 0);
    func_L11_0032D918(a0, 0, 0, 3);
    func_L11_00325748(a0);
    *(u16 *)(s2 + 0x34) = (*(u16 *)(s2 + 0x34) & 0xEFFF) | 0x47;
    *(u8 *)(s2 + 0x31) = 0;
    *(s32 *)(s2 + 0x98) = 0;
    s1 = *(char **)(s2 + 0x68);
    if (*(s16 *)(s1 + 0x366) == 1)
        func_L11_004431C8(*(s32 *)(s2 + 0xB8), 0xD98);
    *(s16 *)(s1 + 0x366) = 0;
    *(s32 *)(s1 + 0x350) = 0;
    if (*(s32 *)(s1 + 0x354) != 0) {
        tmp = *(TI128 *)((char *)(long)*(s32 *)(s1 + 0x354) + 0x10);
        func_L11_0044AD58(s2, &tmp);
    }
    if (*(u8 *)(s1 + 0x2B1) != 0) {
        f32 one = 1.0f;

        *(u8 *)(s1 + 0x2B2) = 0;
        *(f32 *)(s1 + 0x2D8) = one;
        *(f32 *)(s1 + 0x2D4) = one;
        *(f32 *)(s1 + 0x2D0) = one;
        func_L11_00324870(s2, s1 + 0x2B0);
        func_L11_00300F68(s1 + 0x2B0, 0, 0x40);
    }
}

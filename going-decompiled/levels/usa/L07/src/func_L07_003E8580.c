/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x003E8580, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* The 16-byte vector the object keeps. */
typedef int TI128 __attribute__((mode(TI)));

extern void func_L07_003EB7D8(char *, char *);

extern void func_L07_00310FD0(char *, int, int);

extern void func_L07_002E53D8(TI128 *, TI128 *, char *);

extern void func_L07_002E4F18(TI128 *, TI128 *, char *);

extern void func_L07_003E3A50(TI128 *, int, int, int, int, int, int, int, float, float, int, int, int,
                          int);

extern void func_L07_00356570(int, int, char *);

extern void func_L07_002E51F8(TI128 *, TI128 *, float);

extern void func_L07_0031DD30(char *, int, TI128 *, float, float, float, float, int);

/**
 * func_L07_003E8580 - arm the object's mode, then set up its position and
 * hand it to the launcher.
 *
 * The object's u16 flags at +0x34 get 0x41 set and 0x1000 cleared, its +0x98
 * word is zeroed, func_L07_003EB7D8 runs on the state block's (+0x68) +0x330
 * block and func_L07_00310FD0(@arg, 0, 3) sets the mode. A point 0.75 along
 * the object's local z is moved into world space by the +0xC0 matrix
 * (func_L07_002E53D8) and offset by the +0x10 position (func_L07_002E4F18),
 * then passed to the effect spawner func_L07_003E3A50 with the state's +0x338
 * word last. func_L07_00356570(6, 0, @arg) follows, and the state's +0x1C0
 * vector scaled by 0.05 (func_L07_002E51F8) is the launch vector given to
 * func_L07_0031DD30 with 1016, the four speeds 3/6/3/6 and the mask 255.
 *
 * @arg: the object.
 * Return: none.
 *
 * Non-obvious (upstream's note): func_L07_0031DD30's integer mask is its last
 * parameter, and this compiler sets the arguments up in parameter order: the
 * mask's store then comes after the four float copies, in the call's delay
 * slot, as the retail has it.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/07_siberius/L07_00389EB8.c, function func_L07_003E7240
 * (v1.01 address), author commit b32a5088 (Gianni), by blame -M -C of the
 * definition (all 20 lines; no rename-copy found).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2656 relverify by
 * name):
 *   - func_L07_003E7240 -> func_L07_003E8580 (our split name);
 *   - func_00325D30 -> func_L07_003EB7D8, func_002A6C58 -> func_L07_00310FD0,
 *     func_00283098 -> func_L07_002E53D8, func_00282CC0 -> func_L07_002E4F18,
 *     func_0032C700 -> func_L07_003E3A50, func_002E5CF8 -> func_L07_00356570,
 *     func_00282F20 -> func_L07_002E51F8, func_002B0A68 -> func_L07_0031DD30
 *     (L07's own copies at the v2.00 places);
 *   - `#include "common.h"`/`#include "include_asm.h"` and the file's other
 *     declarations dropped: this function uses no common.h typedef; and
 *     upstream's comment folded into this one.
 */
void func_L07_003E8580(char *arg)
{
    char *o;
    TI128 v[1];
    TI128 w;

    o = *(char **)(arg + 0x68);
    *(unsigned short *)(arg + 0x34) = (*(unsigned short *)(arg + 0x34) | 0x41) & 0xEFFF;
    *(int *)(arg + 0x98) = 0;
    func_L07_003EB7D8(arg, o + 0x330);
    func_L07_00310FD0(arg, 0, 3);
    v[0] = 0;
    ((float *)v)[2] = 0.75f;
    func_L07_002E53D8(v, v, arg + 0xC0);
    func_L07_002E4F18(v, v, arg + 0x10);
    func_L07_003E3A50(v, 0, 25, 20, 20, 20, 45, 1, 0.6f, 20.0f, 1, 0, 0, *(int *)(o + 0x338));
    func_L07_00356570(6, 0, arg);
    func_L07_002E51F8(&w, o + 0x1C0, 0.05000000447034836f);
    func_L07_0031DD30(arg, 1016, &w, 3.0f, 6.0f, 3.0f, 6.0f, 255);
}

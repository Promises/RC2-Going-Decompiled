/* Level 09 (Dobbo), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2580, RULING #13291 row L09-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0040C5B8, in place of its
 * split asm; the row is going-decompiled/levels/usa/L09/config/L09_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column; the plain-`as` ablation is in the row's FACT, task #2580).
 */

typedef unsigned char u8;
typedef unsigned short u16;

/* Upstream's MACRO_ADDR, from its include/common.h (a section attribute on an
 * extern declaration, RULING #8620, upstream's own text). */
#define MACRO_ADDR __attribute__((section(".sdata")))

/* Declarations in upstream's order. Every callee is L09's copy of boot code. */
extern int func_L09_00317BC8(int a);
extern int func_L09_00317C08(int a, int b);
extern void func_L09_00320670(char *a0, int a1);
extern float func_L09_002EC100(float a, float b);
extern float func_L09_002EB6E8(float a);
extern float func_L09_002EB700(float a);
extern int func_L09_002EC258(float a);
extern void func_L09_002EB3D8(float f12, void *a0, void *a1);
/* The level's tuning words (our labels at 0x001B1570..0x001B1584), each
 * $gp-relative at -G8 by its size. Upstream's `sda` attribute text is kept;
 * this cc1 ignores it (a warning), so it emits nothing. */
extern int D_L09_001B1578 __attribute__((sda));
/* A SECOND NAME for the same address 0x001B1578, as upstream has it
 * (upstream's D_L09_001B15B8_4): an incomplete array, so the assembler
 * addresses it absolutely (lui/lw, R_MIPS_HI16/LO16 with addend 4), where the
 * ROM reads [1] that way and the scalar above $gp-relative. It must be a
 * different symbol from D_L09_001B1578: as an asm-label view of
 * D_L09_001B1578 (RULING #13168 form) the assembler takes it as small too,
 * the read turns $gp-relative and .text is 0x13C bytes, not 0x140 (measured,
 * task #2580). So it is an address-named extern WITHOUT the level prefix,
 * since D_L09_001B1578 is taken: tools/levels/level_elf.py undef values
 * D_001B1578 at the address in its name (its "unprefixed" bucket). A name,
 * not a device: it emits nothing and changes no code. */
extern int D_001B1578[] MACRO_ADDR;
extern float D_L09_001B1584 __attribute__((sda));
extern float D_L09_001B1580 __attribute__((sda));
extern int D_L09_001B1574 __attribute__((sda));
extern u8 D_L09_001B1570 __attribute__((sda));

/**
 * func_L09_0040C5B8 - per-frame setup of a Dobbo object's circular motion.
 *
 * Calls func_L09_00320670(@a0, sel) with sel = D_L09_001B1578, or
 * D_001B1578[1] (the next word) when func_L09_00317BC8(7) is 0. Advances the
 * angle at +0x10 of the object's state (+0x68) by D_L09_001B1584 *
 * 0.00029088825 (= pi/180/60) through func_L09_002EC100, and writes
 * D_L09_001B1580 times the two trigonometric results of func_L09_002EB6E8 /
 * func_L09_002EB700 on it to +0xF0/+0xF4. Sets byte +0x23 to D_L09_001B1570 plus
 * D_L09_001B1574 times the mean of the state's first two floats (rounded by
 * func_L09_002EC258) plus a random -9..9 (func_L09_00317C08), sets bit 0x800
 * of +0x34, then applies func_L09_002EB958 and two func_L09_002EB3D8 scalings
 * to the vectors at +0xC0 and +0xD0.
 *
 * @a0: the object.
 *
 * Non-obvious: func_L09_002EB958 has no declaration, as upstream writes it
 * (an implicit `int` declaration in C89); both arguments are pointers passed
 * in registers, so the call is the same.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48-g8.c, function func_L09_0040B0C8
 * (v1.01 address), author commit 27b929ce (Lynder063). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2580 relverify):
 *   - func_L09_0040B0C8 -> func_L09_0040C5B8 (our split name);
 *   - D_L09_001B15B0/15B4/15B8/15C0/15C4 -> our labels
 *     D_L09_001B1570/1574/1578/1580/1584 (`sda` text kept);
 *   - D_L09_001B15B8_4 -> D_001B1578: the second name for 0x001B1578, spelled
 *     without the level prefix because D_L09_001B1578 is the scalar's (see
 *     its declaration);
 *   - callees func_316A28 -> func_L09_00317BC8, func_316A68 ->
 *     func_L09_00317C08, func_31F4D0 -> func_L09_00320670, func_2EB478 ->
 *     func_L09_002EC100, func_2EAA60 -> func_L09_002EB6E8, func_2EAA78 ->
 *     func_L09_002EB700, func_2EB5D0 -> func_L09_002EC258, func_2EA750 ->
 *     func_L09_002EB3D8, func_2EACD0 -> func_L09_002EB958 (still undeclared);
 *   - `#include "common.h"` dropped: the u8/u16 typedefs and the MACRO_ADDR
 *     definition written here, only this function's declarations kept.
 */
void func_L09_0040C5B8(char *a0) {
    char *s3 = *(char **)(a0 + 0x68);
    char *s1;
    float v[3];
    int n;
    float count;
    int sel;
    if (func_L09_00317BC8(7)) {
        sel = D_L09_001B1578;
    } else {
        sel = D_001B1578[1];
    }
    func_L09_00320670(a0, sel);
    s1 = a0 + 0xC0;
    *(float *)(s3 + 0x10) = func_L09_002EC100(*(float *)(s3 + 0x10), D_L09_001B1584 * 0.00029088825f);
    v[0] = func_L09_002EB6E8(*(float *)(s3 + 0x10)) * D_L09_001B1580;
    v[1] = func_L09_002EB700(*(float *)(s3 + 0x10)) * D_L09_001B1580;
    v[2] = 0;
    count = (float)D_L09_001B1574;
    *(float *)(a0 + 0xF0) = v[0];
    *(float *)(a0 + 0xF4) = v[1];
    n = func_L09_002EC258(count * (*(float *)s3 + *(float *)(s3 + 4)) * 0.5f);
    *(u8 *)(a0 + 0x23) = D_L09_001B1570 + n + func_L09_00317C08(-9, 9);
    *(u16 *)(a0 + 0x34) |= 0x800;
    func_L09_002EB958(s1, a0 + 0xF0);
    func_L09_002EB3D8(*(float *)s3, s1, s1);
    func_L09_002EB3D8(*(float *)(s3 + 4), a0 + 0xD0, a0 + 0xD0);
}

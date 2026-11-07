/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0045A6F0, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 19 of 48
 * .text words differ from the disc, relocated fields masked, and .text is 0xC0
 * bytes, not the place's 0xC8, and it emits a .lit4 pool of 4 bytes, which
 * level_elf.py cobj refuses (only .text is allowed).
 */

extern float D_L11_001B2038 __attribute__((section(".sdata")));
extern float D_L11_001B203C __attribute__((section(".sdata")));
extern int D_L11_001B202C __attribute__((section(".sdata")));
extern int D_L11_001B2030 __attribute__((section(".sdata")));
extern float func_L11_0032DD60(float, float);
extern int func_L11_0032DD08(int, int);
extern float func_L11_0032DE28(void);
extern void func_L11_0045A8A8(void *, int, float, int, float, int, int);

/**
 * func_L11_0045A6F0 - spawn the two layers of the object's burst effect.
 *
 * @object: the object (moby); +0x10 its position.
 * Return: none.
 *
 * Non-obvious: The first layer, type D_L11_001B202C, has size D_L11_001B2038
 * plus a random 0..(D_L11_001B2038 * D_L11_001B203C) (func_L11_0032DD60), 80
 * to 96 particles (func_L11_0032DD08), a random phase (func_L11_0032DE28) and
 * arguments 49, 2. The second, type D_L11_001B2030, is 1.2 times that size,
 * with 8 to 22 particles and arguments 11, 2. Both go through
 * func_L11_0045A8A8.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00458C38-g8.c (blob 17a4f34b), function
 * func_L11_00458C38 (v1.01 address), author commit 5f3b0386 (Louis-Philippe Le
 * Sieur). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names (no device). Respellings, each to
 * the target of the v2.00 disc's own relocation at this place (relverify by
 * name on this object, task #2578):
 *   - func_L11_00458C38 -> func_L11_0045A6F0 (our split name at the v2.00
 *     place);
 *   - D_L11_001B1FBC -> D_L11_001B202C (our label at that address);
 *   - D_L11_001B1FC0 -> D_L11_001B2030 (our label at that address);
 *   - D_L11_001B1FC8 -> D_L11_001B2038 (our label at that address);
 *   - D_L11_001B1FCC -> D_L11_001B203C (our label at that address);
 *   - func_002A7820 -> func_L11_0032DD08 (the level's own copy at its L11
 *     place);
 *   - func_002A7878 -> func_L11_0032DD60 (the level's own copy at its L11
 *     place);
 *   - func_002A7940 -> func_L11_0032DE28 (the level's own copy at its L11
 *     place);
 *   - func_L11_00458DF0 -> func_L11_0045A8A8 (its v2.00 split name);
 *   - upstream's SDATA(X) macro (its include/common.h) written out as its
 *     `__attribute__((section(".sdata")))` part; its `__asm__("X__gp")` label,
 *     a second name for X, goes with the respelling;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_0045A6F0(unsigned char *object)
{
    float size, random_size;
    random_size = func_L11_0032DD60(0.0f, D_L11_001B2038 * D_L11_001B203C);
    size = D_L11_001B2038;
    size += random_size;
    func_L11_0045A8A8(object + 0x10, D_L11_001B202C, size, func_L11_0032DD08(80, 96),
                      func_L11_0032DE28(), 49, 2);
    size *= 1.2f;
    func_L11_0045A8A8(object + 0x10, D_L11_001B2030, size, func_L11_0032DD08(8, 22),
                      func_L11_0032DE28(), 11, 2);
}

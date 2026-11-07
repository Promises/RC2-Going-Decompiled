/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0044AD58, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 30 of 44
 * .text words differ from the disc, relocated fields masked, and .text is 0xB0
 * bytes, not the place's 0xC8, and it emits a .lit4 pool of 12 bytes, which
 * level_elf.py cobj refuses (only .text is allowed).
 */

/* RULING #8620 addressing device, EE arm only (level code has no native arm): a
 * section attribute on an extern DECLARATION moves no data and emits nothing.
 * Ablation (task #2578): with upstream's `sda`/`nosda` text instead, .text
 * 0xCC bytes, not 0xC8 (cobj refuses). */
typedef int TI128 __attribute__((mode(TI)));
extern void func_L11_0032DFB0(char *, float, float);
extern void func_L11_003011E0(void *, void *, void *);
extern void func_L11_003398F0(char *, int, char *, char *, int, int, float *, void *, void *, float);
extern void func_L11_00323A00(char *);
extern char D_L11_001A8F20[] __attribute__((section(".sdata")));

/**
 * func_L11_0044AD58 - release the actor's effect record, spawning a final
 * burst from it while the actor is visible.
 *
 * @self: the actor (moby); +0x31 visible, +0x68 its state block (+0x354 the
 *        effect record).
 * @pos:  a 16-byte value, copied into the frame and otherwise unused.
 * Return: none.
 *
 * Non-obvious: The burst is class 0x9D8 (func_L11_003398F0) between the
 * record's +0x10 and +0xF0. It gets a random spread vector (func_L11_0032DFB0,
 * 0.1..1/6) added to D_L11_001A8F20. The record is then freed
 * (func_L11_00323A00) and the pointer cleared.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00449330.c (blob 13087dbf), function
 * func_L11_00449330 (v1.01 address), author commit 72a6596f (Flavius). The
 * file was also touched by 7c6f71af (Mateusz Kłysz), which only renamed this
 * body's data name D_L02_001A8F20 to D_L11_001A8F20 (kept here: it is our
 * label at that address); the audit's licence scan pairs that name with
 * Lombyte, but no statement of the body is from that commit, so it is not
 * Lombyte-origin and no notice is carried. Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED: built at -G8
 * where upstream builds the file at -G0 (a tsv gflag setting, not a text edit;
 * measured at -G0: same length, level .text 2 differing bytes, first at
 * 0x0044ADAF), and the section respelling below. Respellings, each to the
 * target of the v2.00 disc's own relocation at this place (relverify by name
 * on this object, task #2578):
 *   - func_L11_00449330 -> func_L11_0044AD58 (our split name at the v2.00
 *     place);
 *   - func_002F1F70 -> func_L11_003011E0 (the level's own copy at its L11
 *     place);
 *   - func_003144D8 -> func_L11_00323A00 (the level's own copy at its L11
 *     place);
 *   - func_0032B468 -> func_L11_003398F0 (the level's own copy at its L11
 *     place);
 *   - func_31EA58 -> func_L11_0032DFB0 (the level's own copy at its L11
 *     place);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on 1
 *     extern declaration(s) (RULING #8620: upstream's patched cc1 reads sda,
 *     ours does not);
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_0044AD58(char *self, TI128 *pos) {
    char *state = *(char **)(self + 0x68);
    float copy[4];
    float vec[4];

    *(TI128 *)copy = *pos;
    if (*(unsigned char *)(self + 0x31) != 0) {
        func_L11_0032DFB0((char *)vec, 0.10000000894069672f, 0.16666667163372040f);
        func_L11_003011E0(vec, vec, D_L11_001A8F20);
        func_L11_003398F0(self, 0x9D8, *(char **)(state + 0x354) + 0x10, *(char **)(state + 0x354) + 0xF0,
                    0, 0, vec, D_L11_001A8F20, D_L11_001A8F20, 0.0027777778450399637f);
    }
    func_L11_00323A00(*(char **)(state + 0x354));
    *(char **)(state + 0x354) = 0;
}

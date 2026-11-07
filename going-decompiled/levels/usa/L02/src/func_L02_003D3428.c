/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * through the boot's assembler model (task #2579, RULING #13291 row L02-ASM).
 * Compiled ALONE as its own TU and linked at its one place, 0x003D3428, in
 * place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by tools/ee/asm_unit.sh (the row's `asm_unit` column; the plain-`as`
 * ablation is in the row's FACT, task #2579).
 */

typedef int TI128 __attribute__((mode(TI)));
extern void func_L02_003D3428(char *, TI128 *);
extern void func_L02_0031FD50(char *, float, float);
extern void func_L02_002F2D58(void *, void *, void *);
extern void func_L02_0032C760(char *, int, char *, char *, int, int, float *, void *, void *, float);
extern void func_L02_003157A0(char *);
extern char D_L02_001A8F20[] __attribute__((section(".sdata")));

/**
 * func_L02_003D3428 - let go of the held object, with a burst effect if the
 * actor is active.
 *
 * @self: the actor (moby); +0x68 its state block, whose +0x2E4 is the held
 *        object.
 * @pos:  a 128-bit position; it is copied to a local and not used further.
 * Return: none.
 *
 * Non-obvious: when +0x31 is set and an object is held, it builds a vector
 * with func_L02_0031FD50(vec, 0.1, 1/6), adds D_L02_001A8F20 to it
 * (func_L02_002F2D58) and spawns effect 0x9D8 (func_L02_0032C760) at the held
 * object's +0x10 and +0xF0. In every case it then releases the held object
 * (func_L02_003157A0) and clears +0x2E4.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/02_maktar_nebula/L02_002ED7B0.c, function func_L02_003D1ED0
 * (v1.01 address), author commit ac1ee013 (bigmak94). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * body's statements unchanged. Respellings, each to the target of the v2.00
 * disc's own relocation at this place (relverify by name, task #2579):
 *   - func_L02_003D1ED0 -> func_L02_003D3428 (our split name at the v2.00
 *     place);
 *   - func_31EA58 -> func_L02_0031FD50 (the level's own copy at its L02
 *     place);
 *   - func_002F1F70 -> func_L02_002F2D58 (the level's own copy at its L02
 *     place);
 *   - func_0032B468 -> func_L02_0032C760 (the level's own copy at its L02
 *     place);
 *   - func_003144D8 -> func_L02_003157A0 (the level's own copy at its L02
 *     place);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     extern declaration of D_L02_001A8F20 (RULING #8620 addressing-model
 *     device, the audit's `resp` variant; ablation in the row's FACT);
 *   - built at -G8 against upstream's -G0 (the audit's code G, NOTE #13281; a
 *     tsv setting);
 *   - `#include "common.h"` dropped (this body uses nothing from it); upstream
 *     file-scope declarations it does not use are not carried; the rest keep
 *     upstream's order and text.
 */
void func_L02_003D3428(char *self, TI128 *pos) {
    char *state = *(char **)(self + 0x68);
    float copy[4];
    float vec[4];

    *(TI128 *)copy = *pos;
    if (*(unsigned char *)(self + 0x31) != 0 && *(char **)(state + 0x2E4) != 0) {
        func_L02_0031FD50((char *)vec, 0.10000000894069672f, 0.16666667163372040f);
        func_L02_002F2D58(vec, vec, D_L02_001A8F20);
        func_L02_0032C760(self, 0x9D8, *(char **)(state + 0x2E4) + 0x10, *(char **)(state + 0x2E4) + 0xF0,
                    0, 0, vec, D_L02_001A8F20, D_L02_001A8F20, 0.0027777778450399637f);
    }
    func_L02_003157A0(*(char **)(state + 0x2E4));
    *(char **)(state + 0x2E4) = 0;
}

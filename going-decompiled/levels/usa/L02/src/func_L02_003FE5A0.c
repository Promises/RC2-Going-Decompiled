/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * through the boot's assembler model (task #2579, RULING #13291 row L02-ASM).
 * Compiled ALONE as its own TU and linked at its one place, 0x003FE5A0, in
 * place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by tools/ee/asm_unit.sh (the row's `asm_unit` column; the plain-`as`
 * ablation is in the row's FACT, task #2579).
 */

typedef unsigned char u8;
typedef int s32;
extern void func_L02_003168E0(char *, s32, char *);
extern float func_L02_002F2F50(char *, char *);
extern char g_heroPos[];

/**
 * func_L02_003FE5A0 - is the actor's joint 0 far from the hero?
 *
 * @a0: the actor (moby).
 * Return: 1 when +0x31 is set and joint 0's position (func_L02_003168E0) is
 *         more than 25 units from g_heroPos (func_L02_002F2F50), else 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/02_maktar_nebula/L02_003F5430.c, function func_L02_003FD000
 * (v1.01 address), author commit 773eafc2 (llesieur99); the file is modified
 * upstream since b2a35115 ({ΔM}), and this body is as at the pin. Adopted with
 * the authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED;
 * the body's statements unchanged. Respellings, each to the target of the
 * v2.00 disc's own relocation at this place (relverify by name, task #2579):
 *   - func_L02_003FD000 -> func_L02_003FE5A0 (our split name at the v2.00
 *     place);
 *   - D_00189EA0 -> g_heroPos (the boot's name, same address);
 *   - func_0029FCC0 -> func_L02_003168E0 (the level's own copy at its L02
 *     place);
 *   - func_00282E80 -> func_L02_002F2F50 (the level's own copy at its L02
 *     place);
 *   - built at -G8 against upstream's -G0 (the audit's code G, NOTE #13281; a
 *     tsv setting);
 *   - `#include "common.h"` dropped (the file defines the typedefs it uses
 *     itself); upstream file-scope declarations it does not use are not
 *     carried; the rest keep upstream's order and text.
 */
s32 func_L02_003FE5A0(char *a0) {
    char buf[16];
    s32 r = 0;
    func_L02_003168E0(a0, 0, buf);
    if (*(u8 *)(a0 + 0x31) != 0) {
        if (func_L02_002F2F50(buf, g_heroPos) > 25.0f)
            r = 1;
    }
    return r;
}

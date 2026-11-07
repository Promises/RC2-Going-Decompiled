/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2581, RULING #13291 row L14-ASM). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E5F58, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled by
 * tools/ee/asm_unit.sh (the row's `asm_unit` column). Measured with plain GNU
 * as instead (task #2581, same TU): .text is 0x144 bytes, not the place's
 * 0x158, and it emits a .lit4 pool of 8 bytes; level_elf.py cobj refuses it.
 */

extern void func_L14_00323250();
extern char g_listenerPosHistory[];
extern int func_L14_003336B8(char *self);
extern int func_L14_0036ADA0(int a0, int a1, char *a2);
extern int func_L14_0036AAA0(int a0);
extern float func_L14_002F7AB0(float a0, float a1);

/**
 * func_L14_003E5F58 - a group-steering object: leave the group, take it over
 * when it is free, or turn while holding a sound entry.
 *
 * @self: the object (moby); its stage byte is +0x20, its state block +0x68
 *        (whose +0x00 is the sound entry it holds), its angle +0xF0.
 * Return: nothing.
 *
 * Non-obvious: Upstream's comment, kept in substance: in stage 2 it takes an
 * entry from the 0x70-byte table at g_listenerPosHistory (the boot's name at
 * that address), a new one (func_L14_0036ADA0(0, 4, self)) when the one it
 * holds is not its own or is free, and turns half a degree (0.0087266471 rad,
 * func_L14_002F7AB0) while the angle is above -pi/2. At the end of stage 2 it
 * releases the entry (func_L14_0036AAA0) when it is its own and in use, and
 * forgets it either way. Stage 0 sets stage 1 and clears the entry; stage 1
 * waits for func_L14_003336B8, then sets stage 2.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own, `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c (blob
 * d4a45444), function func_L14_003E4A40 (v1.01 address), author commit 913bd1bc
 * (Gianni). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED: built at -G8 where upstream builds the file at -G0
 * (its stem has no -g8; a tsv gflag setting, not a text edit; measured at -G0,
 * task #2581: .text 0x154 bytes, not 0x158, cobj refuses); the text is verbatim
 * but for names. Edits, each name to the target of the v2.00 disc's own
 * relocation at this place (relverify by name on this object, task #2581):
 *   - func_L14_003E4A40 -> func_L14_003E5F58 (our split name at the v2.00
 *     place);
 *   - D_00188660 -> g_listenerPosHistory (the boot's name at that address);
 *   - func_2F6CC8 -> func_L14_002F7AB0 (the level's own copy at its L14 place);
 *   - func_321F58 -> func_L14_00323250 (the level's own copy at its L14 place);
 *   - func_332310 -> func_L14_003336B8 (the level's own copy at its L14 place);
 *   - func_3697D0 -> func_L14_0036AAA0 (the level's own copy at its L14 place);
 *   - func_369AD0 -> func_L14_0036ADA0 (the level's own copy at its L14 place);
 *   - a repeated file-scope declaration of g_listenerPosHistory (upstream
 *     D_00188660) carried once;
 *   - `#include "common.h"` dropped (the body uses none of its typedefs);
 *     upstream file-scope declarations the body does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L14_003E5F58(char *self) {
    char *state = *(char **)(self + 0x68);
    char *e1;
    char *e2;
    int idx;

    switch (*(unsigned char *)(self + 0x20)) {
    case 0:
        func_L14_00323250(self, 1, -1);
        *(int *)state = -1;
        break;
    case 1:
        if (func_L14_003336B8(self) != 0) {
            func_L14_00323250(self, 2, -1);
        }
        break;
    case 2:
        if (*(float *)(self + 0xF0) > -1.5707964f) {
            e1 = g_listenerPosHistory + *(int *)state * 0x70;
            if (*(int *)(e1 + 0x88) != (int)self || *(unsigned char *)(e1 + 0x74) == 0) {
                *(int *)state = func_L14_0036ADA0(0, 4, self);
            }
            *(float *)(self + 0xF0) = func_L14_002F7AB0(*(float *)(self + 0xF0), 0.0087266471f);
        }
        idx = *(int *)state;
        if (idx != -1) {
            e2 = g_listenerPosHistory + idx * 0x70;
            if (*(int *)(e2 + 0x88) == (int)self && *(unsigned char *)(e2 + 0x74) != 0) {
                func_L14_0036AAA0(idx);
            }
        }
        *(int *)state = -1;
        break;
    }
}

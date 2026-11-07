/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2581, RULING #13291 row L14-ASM). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E0BB8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled by
 * tools/ee/asm_unit.sh (the row's `asm_unit` column). Measured with plain GNU
 * as instead (task #2581, same TU): .text is 0x13C bytes, not the place's
 * 0x158, and it emits a .lit4 pool of 12 bytes; level_elf.py cobj refuses it.
 */

typedef int TI128 __attribute__((mode(TI)));
typedef union { TI128 q; float f[4]; } Vector;
extern char *D_L14_001B260C __attribute__((sda));
extern char g_listenerPosHistory[];
extern void func_L14_003267B0(void *, void *, float, float, float, float);
extern void func_L14_0032A0A0(void *, void *, void *, void *);
extern float func_L14_002F6960(float);
extern int func_L14_0036ADA0(int, int, void *);
extern void func_L14_0036AAA0(int);

/**
 * func_L14_003E0BB8 - turn an object toward its target angle and keep its
 * turning sound alive while it moves.
 *
 * @self: the object (moby); +0x68 its state block, +0x18 its rotation, +0xF0 its
 *        angle.
 * Return: nothing.
 *
 * Non-obvious: The target is the angle at +0x38 of the 0x80-byte record that
 * state +0x60 indexes in the table D_L14_001B260C when state +0x7C is set, else
 * state +0x70. func_L14_003267B0 steps the rotation toward it (rates 1/900 and
 * 1/450, limit 1.0), keeping the turn speed in state +0x74, and
 * func_L14_0032A0A0 applies a Z rotation of that speed. While |speed|
 * (func_L14_002F6960) exceeds 0.01 the object holds a sound entry in the
 * 0x70-byte table at g_listenerPosHistory (the boot's name at that address): it
 * takes a new one (func_L14_0036ADA0(0, 4, self)) when the one in state +0x78
 * is not its own or not in use. Otherwise it releases its own entry
 * (func_L14_0036AAA0) and sets state +0x78 to -1.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own, `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003DF6A0-g8.c (blob
 * cc73151c), function func_L14_003DF6A0 (v1.01 address), author commit bf60a47f
 * (Louis-Philippe Le Sieur). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names (no
 * device). Edits, each name to the target of the v2.00 disc's own relocation at
 * this place (relverify by name on this object, task #2581):
 *   - func_L14_003DF6A0 -> func_L14_003E0BB8 (our split name at the v2.00
 *     place);
 *   - D_00188660 -> g_listenerPosHistory (the boot's name at that address);
 *   - D_L14_001B268C -> D_L14_001B260C (address-named, inside
 *     D_L14_001B1D00+0x90C; RULING #13291 section 2);
 *   - func_00282C48 -> func_L14_002F6960 (the level's own copy at its L14
 *     place);
 *   - func_002AA548 -> func_L14_003267B0 (the level's own copy at its L14
 *     place);
 *   - func_002AD6F0 -> func_L14_0032A0A0 (the level's own copy at its L14
 *     place);
 *   - func_002E59F8 -> func_L14_0036AAA0 (the level's own copy at its L14
 *     place);
 *   - func_002E5CF8 -> func_L14_0036ADA0 (the level's own copy at its L14
 *     place);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning) and the member closes with it as written;
 *   - `#include "common.h"` dropped (the body uses none of its typedefs);
 *     upstream file-scope declarations the body does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L14_003E0BB8(char *self) {
    struct { Vector argument, rotation; } work;
    char *state = *(char **)(self + 0x68);
    float target;
    int index;
    if (*(int *)(state + 0x7C))
        target = *(float *)(((char (*)[0x80])D_L14_001B260C)[*(int *)(state + 0x60)] + 0x38);
    else
        target = *(float *)(state + 0x70);
    func_L14_003267B0(self + 0x18, state + 0x74, target,
                 0.0011111111f, 0.0022222222f, 1.0f);
    work.rotation.q = 0;
    work.rotation.f[2] = *(float *)(state + 0x74);
    work.argument.q = work.rotation.q;
    func_L14_0032A0A0(state + 0x20, &work.argument, self + 0xF0, self + 0xF0);
    if (0.01f < func_L14_002F6960(*(float *)(state + 0x74))) {
        char *entry = g_listenerPosHistory + *(int *)(state + 0x78) * 0x70;
        if (*(char **)(entry + 0x88) != self || *(unsigned char *)(entry + 0x74) == 0)
            *(int *)(state + 0x78) = func_L14_0036ADA0(0, 4, self);
    } else {
        index = *(int *)(state + 0x78);
        if (index != -1) {
            char *entry = g_listenerPosHistory + index * 0x70;
            if (*(char **)(entry + 0x88) == self && *(unsigned char *)(entry + 0x74) != 0)
                func_L14_0036AAA0(index);
        }
        *(int *)(state + 0x78) = -1;
    }
}

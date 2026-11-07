/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0043C218, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 38 of 82
 * .text words differ from the disc, relocated fields masked, and .text is
 * 0x148 bytes, not the place's 0x150, and it emits a .lit4 pool of 4 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

extern float D_L11_001B1318 __attribute__((sda));
extern void func_L11_0032D918(void *, int, int, int);
extern float func_L11_0032DD60(float, float);
extern int func_L11_0032DCC8(int);
extern void func_L11_00300F68(unsigned int *, unsigned int, int);
extern void func_L11_003247F8(unsigned char *, unsigned int, unsigned char *);
extern int func_L11_0032DCC8(int n);

/**
 * func_L11_0043C218 - re-aim the object: reset its measured state, draw a
 * random turn rate and clear its two rotation slots.
 *
 * @obj: the object (moby); +0x68 its state block.
 * Return: none.
 *
 * Non-obvious: Stage byte +0x2E becomes 3 and the hit points are copied to
 * +0x20 as a float. The flags are reset (visible, +0x34 bit 0 cleared, 0x1000
 * set, 0x46 cleared), and animation 1 is set unless it is current. The turn
 * rate (+0x2D4) is 10 to 30 degrees with a random sign, and +0x2DC is
 * +/-D_L11_001B1318 by a second coin flip. The two 0x40-byte rotation slots at
 * +0x220 and +0x260 are cleared (func_L11_00300F68) and handed to
 * func_L11_003247F8 as slots 2 and 3. The file declares func_L11_0032DCC8
 * twice, as upstream does.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00432628-g8.c (blob debf25a2), function
 * func_L11_0043A9D8 (v1.01 address), author commit 47360109 (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device). Respellings, each to the target of the
 * v2.00 disc's own relocation at this place (relverify by name on this object,
 * task #2578):
 *   - func_L11_0043A9D8 -> func_L11_0043C218 (our split name at the v2.00
 *     place);
 *   - D_L11_001B1338 -> D_L11_001B1318 (our label at that address);
 *   - func_00282A60 -> func_L11_00300F68 (the level's own copy at its L11
 *     place);
 *   - func_0029F978 -> func_L11_003247F8 (the level's own copy at its L11
 *     place);
 *   - func_002A7470 -> func_L11_0032D918 (the level's own copy at its L11
 *     place);
 *   - func_002A77E0 -> func_L11_0032DCC8 (the level's own copy at its L11
 *     place);
 *   - func_002A7878 -> func_L11_0032DD60 (the level's own copy at its L11
 *     place);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_0043C218(char *obj)
{
    char *state = *(char **)(obj + 0x68);
    float f20;
    float f0;

    *(unsigned char *)(state + 0x2E) = 3;
    *(float *)(state + 0x20) = (float)*(short *)(state + 0x24);
    *(short *)(state + 0x1A8) = 0;
    *(unsigned char *)(state + 0x18A) = 0;
    *(unsigned char *)(obj + 0x31) = 1;
    *(unsigned short *)(obj + 0x34) &= 0xFFFE;
    *(int *)(obj + 0x98) = *(int *)(*(int *)(obj + 0x24) + 0x10);
    *(unsigned short *)(obj + 0x34) = (*(unsigned short *)(obj + 0x34) | 0x1000) & 0xFFB9;
    *(unsigned char *)(obj + 0x7F) = 0;
    if (*(unsigned char *)(obj + 0x43) != 1)
        func_L11_0032D918(obj, 1, 0, 0);
    f20 = func_L11_0032DD60(10.0f, 30.0f) * 0.017453294f;
    if (func_L11_0032DCC8(2) != 0)
        f20 = -f20;
    *(float *)(state + 0x2D4) = f20;
    if (func_L11_0032DCC8(2) != 0)
        f0 = -D_L11_001B1318;
    else
        f0 = D_L11_001B1318;
    *(short *)(state + 0x2EE) = 0;
    *(float *)(state + 0x2DC) = f0;
    func_L11_00300F68((unsigned int *)(state + 0x220), 0, 0x40);
    func_L11_003247F8((unsigned char *)obj, 2, (unsigned char *)(state + 0x220));
    func_L11_00300F68((unsigned int *)(state + 0x260), 0, 0x40);
    func_L11_003247F8((unsigned char *)obj, 3, (unsigned char *)(state + 0x260));
}

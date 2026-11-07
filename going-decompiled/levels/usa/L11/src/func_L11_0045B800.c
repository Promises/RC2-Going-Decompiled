/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0045B800, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 15 of 61
 * .text words differ from the disc, relocated fields masked, and .text is 0xF4
 * bytes, not the place's 0xFC, and it emits a .lit4 pool of 4 bytes, which
 * level_elf.py cobj refuses (only .text is allowed).
 */

extern void func_L11_003014F8(char *a0, void *a1, float f12);
extern void func_L11_00332468(void *a0, void *a1, float f12);
extern void func_L11_00301280(char *a0, void *a1, float f12);
extern void func_L11_00334290(void *a0, void *a1, void *a2);
extern void func_L11_003011E0(void *a0, void *a1, void *a2);
extern float func_L11_003022F0(float a, float b);
extern void func_L11_002F9490(void *a0, void *a1);
extern void func_L11_0045B900(char *p);
extern float D_L11_001B206C __attribute__((sda));

/**
 * func_L11_0045B800 - rebuild the state's offset block from its rotation and
 * install the next handler.
 *
 * @p: the object (moby); +0x10 its position, +0x68 its state block.
 * Return: none.
 *
 * Non-obvious: A copy of the state's vector +0x40 (func_L11_003014F8, scale
 * 1.0) goes through func_L11_00332468 with +0x30. +0x00 scaled by +0x60 goes
 * into +0x10 (func_L11_00301280) and is combined with that vector
 * (func_L11_00334290). +0x20 is its negation; both are then offset by the
 * object's position. The rotation advances by D_L11_001B206C * 1/60 degree
 * (func_L11_003022F0), and func_L11_0045B900 is installed as the next handler
 * (func_L11_002F9490).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00453358-g8.c (blob 02c436b7), function
 * func_L11_00459D48 (v1.01 address), author commit 431ac503 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). The audit closed it only with
 * its file-level `long long` -> `long` respelling (RULING #13197), but this
 * body's own text has no `long long`, so alone in its TU it needs none (FACT
 * #13321). Respellings, each to the target of the v2.00 disc's own relocation
 * at this place (relverify by name on this object, task #2578):
 *   - func_L11_00459D48 -> func_L11_0045B800 (our split name at the v2.00
 *     place);
 *   - D_L11_001B1FFC -> D_L11_001B206C (our label at that address);
 *   - func_00282CC0 -> func_L11_003011E0 (the level's own copy at its L11
 *     place);
 *   - func_00282D30 -> func_L11_00301280 (the level's own copy at its L11
 *     place);
 *   - func_00282F20 -> func_L11_003014F8 (the level's own copy at its L11
 *     place);
 *   - func_00283BE0 -> func_L11_003022F0 (the level's own copy at its L11
 *     place);
 *   - func_002AB668 -> func_L11_00332468 (the level's own copy at its L11
 *     place);
 *   - func_002ACDE8 -> func_L11_00334290 (the level's own copy at its L11
 *     place);
 *   - func_L00_002E6CF0 -> func_L11_002F9490 (the level's own copy at its L11
 *     place);
 *   - func_L11_00459E48 -> func_L11_0045B900 (its v2.00 split name);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_0045B800(char *p)
{
    char *q = *(char **)(p + 0x68);
    float v[4];

    func_L11_003014F8((char *)v, q + 0x40, 1.0f);
    func_L11_00332468(v, v, *(float *)(q + 0x30));
    func_L11_00301280(q + 0x10, q, *(float *)(q + 0x60));
    func_L11_00334290(q + 0x10, q + 0x10, v);
    *(float *)(q + 0x20) = -*(float *)(q + 0x10);
    *(float *)(q + 0x24) = -*(float *)(q + 0x14);
    *(float *)(q + 0x28) = -*(float *)(q + 0x18);
    func_L11_003011E0(q + 0x10, p + 0x10, q + 0x10);
    func_L11_003011E0(q + 0x20, p + 0x10, q + 0x20);
    *(float *)(q + 0x30) = func_L11_003022F0(*(float *)(q + 0x30),
                                         D_L11_001B206C * 0.000290888245f);
    func_L11_002F9490(func_L11_0045B900, p);
}

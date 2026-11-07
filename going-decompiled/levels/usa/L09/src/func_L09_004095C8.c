/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x004095C8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef signed short s16;
typedef int s32;

extern void func_L09_004094E8(char *a0, s32 a1, s32 a2, s32 a3);

/**
 * func_L09_004095C8 - set an entity's colour key, or hand an off-kind entity
 * to the shared body.
 *
 * Nothing happens for a null @a1 or an entity whose state byte at +0x20 is
 * 0xFE or 0xFD. By the kind word at +0xAA: kind 0xD0F keeps the work local --
 * the colour key @a2 goes to the entity's state byte at +0x31C, the entity's
 * byte at +0x30 is copied to the state's +0x31E unless it is 0xFF, and a
 * nonzero @a2 latches the entity's +0x30 to 0xFF. Kinds 0xD09 and 0xCEA go to
 * func_L09_004094E8(@a1, 0, @a3, @a4) with the key suppressed; kind 0xDE0
 * copies @a0's byte at +0x95 to the entity's +0x95; every other kind goes to
 * func_L09_004094E8(@a1, @a2, @a3, @a4) unchanged.
 *
 * @a0: the asking object.
 * @a1: the entity record to recolour.
 * @a2: the colour key (an s32; only its low byte is stored).
 * @a3, @a4: extra requests passed through to func_L09_004094E8.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48.c, function func_L09_00408138 (v1.01
 * address), author commit ee3fee45 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_00408138 -> func_L09_004095C8 (our split name);
 *   - func_L09_00408058 -> func_L09_004094E8 (the shared body's L09 copy,
 *     itself adopted in this row);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
void func_L09_004095C8(char *a0, char *a1, s32 a2, s32 a3, s32 a4)
{
    u8 c;
    s16 kind;
    char *p;

    if (a1 == 0) {
        return;
    }
    c = *(u8 *)(a1 + 0x20);
    if (c == 0xFE) {
        return;
    }
    if (c == 0xFD) {
        return;
    }
    kind = *(s16 *)(a1 + 0xAA);
    if (kind == 0xD0F) {
        p = *(char **)(a1 + 0x68);
        *(u8 *)(p + 0x31C) = a2;
        c = *(u8 *)(a1 + 0x30);
        if (c != 0xFF) {
            *(u8 *)(p + 0x31E) = c;
        }
        if (a2 != 0) {
            *(u8 *)(a1 + 0x30) = 0xFF;
        }
        return;
    }
    if (kind == 0xD09 || kind == 0xCEA) {
        func_L09_004094E8(a1, 0, a3, a4);
        return;
    }
    if (kind == 0xDE0) {
        *(u8 *)(a1 + 0x95) = *(u8 *)(a0 + 0x95);
        return;
    }
    func_L09_004094E8(a1, a2, a3, a4);
}

/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x00444A50, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern int func_L20_0044B010(char *);
extern int func_L20_0033FB28(char *, int);
extern int func_L20_003BB7F0(char *);
extern int func_L20_0044B020(char *);

/**
 * func_L20_00444A50 - decide whether another object is a valid target for this
 * one.
 *
 * @arg0: this object; +0x68 its state, whose word at +0x220 is passed to
 * func_L20_0033FB28.
 * @arg1: the candidate; +0x10 its position, +0xAA its signed 16-bit id.
 * Return: 1 if the candidate qualifies, else 0.
 *
 * Non-obvious: func_L20_0033FB28(arg1 + 0x10, state word +0x220) must return
 * nonzero. Ids below 0x1FF never qualify. Ids 0x1FF and 0x200 qualify when
 * func_L20_003BB7F0(arg1) returns 0. Id 0x12D3 qualifies unless
 * func_L20_0044B010(arg1) or func_L20_0044B020(arg1) returns nonzero. No other
 * id qualifies.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_0043ECB0.c (blob c24c6906), function
 * func_L20_00443508 (v1.01 address), author commit 765097bb (ReaperOfVeriod).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2566):
 *   - func_L20_00443508 -> func_L20_00444A50 (our split name at the v2.00
 *     place);
 *   - func_0033E808 -> func_L20_0033FB28 (the level's own copy at its L20
 *     place);
 *   - func_003BA3F8 -> func_L20_003BB7F0 (the level's own copy at its L20
 *     place);
 *   - func_L20_00449AC8 -> func_L20_0044B010 (its v2.00 split name);
 *   - func_L20_00449AD8 -> func_L20_0044B020 (its v2.00 split name);
 *   - `#include "common.h"` dropped; the typedefs this body uses are written
 *     here (or are the file's own); upstream file-scope declarations it does
 *     not use are not carried, the rest keep upstream's order and text,
 *     duplicates included.
 */
int func_L20_00444A50(char *arg0, char *arg1) {
    char *state = *(char **)(arg0 + 0x68);
    short v;

    if (func_L20_0033FB28(arg1 + 0x10, *(int *)(state + 0x220)) == 0) {
        goto ret0;
    }
    v = *(short *)(arg1 + 0xAA);
    if (v < 0x1FF) {
        goto ret0;
    }
    if (v < 0x201) {
        goto other;
    }
    if (v != 0x12D3) {
        goto ret0;
    }
    if (func_L20_0044B010(arg1) != 0) {
        goto ret0;
    }
    if (func_L20_0044B020(arg1) != 0) {
        goto ret0;
    }
    return 1;
other:
    return func_L20_003BB7F0(arg1) == 0;
ret0:
    return 0;
}

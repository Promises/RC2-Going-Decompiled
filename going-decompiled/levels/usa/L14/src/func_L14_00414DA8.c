/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x00414DA8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/**
 * func_L14_00414DA8 - read a word through the object's state block.
 *
 * @param_1: the object; its state block pointer is at +0x68.
 * Return: the word at state +0x18.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98.c, function
 * func_L14_004137F8 (v1.01 address), author commit 7fccf735 (Flavius; the
 * commit that added the file, i.e. this body; `git blame` marks it as the
 * boundary commit of the file's history). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL:
 * VERBATIM but for names; edits:
 *   - name func_L14_004137F8 -> func_L14_00414DA8 (our split name at the
 *     v2.00 place);
 *   - the file's `#include` lines, other functions and their externs not
 *     carried (nothing here uses them).
 */
unsigned int func_L14_00414DA8(unsigned int param_1)
{
    unsigned int inner_ptr;
    unsigned int result;

    inner_ptr = *(unsigned int *)(param_1 + 0x68);
    result = *(unsigned int *)(inner_ptr + 0x18);
    return result;
}

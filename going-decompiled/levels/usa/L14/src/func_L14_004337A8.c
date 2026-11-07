/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x004337A8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/**
 * func_L14_004337A8 - no-op entry point (the moby-table entry of the pair).
 *
 * @object: the moby, unused.
 * Return: nothing.
 *
 * Non-obvious: an intentionally empty body; its twin func_L14_004337A0 (the
 * previous word) is the directly-called entry of the same pair. With no
 * calls and no globals the body has no relocation; its evidence is the
 * unmasked .text compare (diff 0 of 2 words, relocs 0).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98.c, function
 * func_L14_004321C8 (v1.01 address), author commit 42e0babe (Louis-Philippe
 * Le Sieur; the same account as llesieur99, FACT #37797; the definition line
 * blames 42e0babe too). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names;
 * edits:
 *   - name func_L14_004321C8 -> func_L14_004337A8 (our split name at the
 *     v2.00 place);
 *   - the file's `#include` lines, other functions and their externs not
 *     carried (nothing here uses them).
 */
void func_L14_004337A8(void *object)
{
}

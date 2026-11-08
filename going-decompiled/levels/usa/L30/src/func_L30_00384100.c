/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00384100, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm the SDK 2.9 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, plain as.
 */

/**
 * func_L30_00384100 - linear interpolation between two floats.
 * @a0: the value at @a2 = 0.
 * @a1: the value at @a2 = 1.
 * @a2: the blend factor.
 *
 * Computed as a0 * (1 - a2) + a1 * a2, not a0 + (a1 - a0) * a2.
 * Return: the blended value.
 *
 * Arm: every arm tried (sdk29 and s136, at -G0 and -G8) gives 0 differing
 * words here; sdk29 -G0 is NOTE #13281's record (its tie-break order), not a
 * measured distinction.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0032AAF0.c, function
 * func_L30_00382EF0 (v1.01 address), author commit 91e04732 (llesieur99;
 * `git blame -M -C` gives all 3 definition lines to it, `git log -S`
 * agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names:
 *   - func_L30_00382EF0 -> func_L30_00384100 (our split name); the body
 *     references no other symbol (0 relocations);
 *   - the file's `#include` lines and other declarations dropped.
 */
float func_L30_00384100(float a0, float a1, float a2) {
    return a0 * (1.0f - a2) + a1 * a2;
}

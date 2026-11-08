/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00437E50, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/**
 * func_L15_00437E50 - put an object into state 2 with a 0x1E0 timer.
 *
 * @a0: the object; its state block is at +0x68.
 * Return: nothing.
 *
 * Sets the state's +0x270 to 1, +0x274 to 0x1E0 and +0x180 to 450.0, the
 * object's bytes +0x23 to 0x20 and +0x20 to 2, bit 8 of its flags halfword
 * +0x34, and clears its word +0x98.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_004328E8.c, function func_L15_00436090 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_00436090 -> func_L15_00437E50 (our split name at the v2.00 place);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above it (a note that func_L15_00435E80 lives in
 *   another file) is about another function and is dropped.
 */
void func_L15_00437E50(char *a0) {
    char *v1 = *(char **)(a0 + 0x68);

    *(int *)(v1 + 0x270) = 1;
    *(int *)(v1 + 0x274) = 0x1E0;
    *(char *)(a0 + 0x23) = 0x20;
    *(char *)(a0 + 0x20) = 2;
    *(unsigned short *)(a0 + 0x34) |= 8;
    *(float *)(v1 + 0x180) = 450.0f;
    *(int *)(a0 + 0x98) = 0;
}

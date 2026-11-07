/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x00457A90,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm
 * tools/ee/ee_cc1.sh sdk29 at -O2 -G0.
 *
 * func_L02_00457A90 - stub the level's callers pass an argument to and which
 * ignores it; returns -1. (An unused parameter costs no codegen.)
 *
 * @arg0: ignored.
 * Return: always -1.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_00456258 (v1.01 address; upstream
 * credits rac1-decomp src/core/00118A40.c func_00119100), author commit
 * 76651091 (Flavius). Adopted with the authors' permission (RULINGs #10405,
 * #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - name respelling func_L02_00456258 -> func_L02_00457A90 (our split name
 *     at the v2.00 place).
 * No relocations: the 2 words are compared unmasked.
 */
/* rac1-decomp src/core/00118A40.c: func_00119100 */
/* Takes an argument its callers pass (e.g. r1_func_00112468) and ignores
   it; an unused parameter costs no codegen. */
int func_L02_00457A90(int arg0) {
    return -1;
}

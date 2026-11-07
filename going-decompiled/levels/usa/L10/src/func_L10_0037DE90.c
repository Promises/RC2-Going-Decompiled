/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x0037DE90, in place of its split asm; the row is
 * going-decompiled/levels/usa/L10/config/L10_c.tsv, the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, plain as.
 */

/* Upstream's own macro (its common.h), kept as written: a section attribute
   on an extern declaration places nothing; at -G0 it leaves the load as the
   split %hi/%lo pair the ROM has. */
#define NOT_SDA __attribute__((section(".data")))

/* A level pointer word at 0x00221D08. No label starts there in our split (it
   lies inside a larger data label), so this extern is ADDRESS-NAMED and the
   level ELF values it from its name (RULING #13291 §2). */
extern void *D_L10_00221D08 NOT_SDA;

/**
 * func_L10_0037DE90 - return the pointer word at 0x00221D08.
 *
 * Return: the word, as an int.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/10_hrugis_cloud/L10_003170D0.c, function
 * func_L10_0037CC20 (v1.01 address), author commit 76651091 (Flavius), whose
 * file comment calls it carried from rac1-decomp's func_001138A8; the body
 * is one `return` of a global, a trivial line that carries no further credit
 * (RULING #13541 rev 2). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names;
 * the respellings, each to the target of the v2.00 disc's own relocation at
 * this place (task #2657 relverify):
 *   - func_L10_0037CC20 -> func_L10_0037DE90 (our split name at the v2.00 place);
 *   - r1_D_0012F86C_0037CC20 -> D_L10_00221D08, an address-named extern for
 *     a target inside a larger split data label (RULING #13291 §2);
 *   - `#include "common.h"` dropped: only the NOT_SDA macro this function
 *     uses is written here.
 */
int func_L10_0037DE90(void) {
    return (int)D_L10_00221D08;
}

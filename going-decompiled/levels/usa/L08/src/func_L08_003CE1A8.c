/* Level 08 (Tabora), USA v2.00: a level-only function built from C (task
 * #2627, RULING #13291 row L08-G1). Compiled ALONE as its own TU and linked at
 * its one place, 0x003CE1A8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L08/config/L08_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

/* The level's object-handle table: a pointer global holding the base of
 * 0x100-byte entries. Lives inside the split data label D_L08_001B2200
 * (+0x7DC): address-named extern valued from the name by level_elf (RULING
 * #13291 section 2). Upstream's `__attribute__((sda))` is kept verbatim; our
 * stock cc1 warns `attribute directive ignored`, byte-inert here (masked
 * compare diff 0). */
extern char *D_L08_001B29DC __attribute__((sda));

/**
 * func_L08_003CE1A8 - a handle's entry float at +0x18.
 *
 * @index: the entity handle; -1 returns 0.0f.
 * Return: the float at entry + 0x18 of the 0x100-byte entry.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/08_tabora/L08_002D90C8-g8.c, function
 * func_L08_003CCD58 (v1.01 address), author commit b31a91e7 (MantraAU).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L08_003CCD58 -> func_L08_003CE1A8 (our split name at the
 *     v2.00 place);
 *   - D_L08_001B29DC kept: upstream's name already states the v2.00 target
 *     address 0x001B29DC (inside the split data label D_L08_001B2200,
 *     +0x7DC); address-named extern valued from the name by level_elf
 *     (RULING #13291 section 2);
 *   - the file's `#include "common.h"` line, the TI128 typedef, the other
 *     functions and their externs not carried (nothing else here uses them).
 */
float func_L08_003CE1A8(int index) {
    if (index != -1)
        return *(float *)(D_L08_001B29DC + ((unsigned int)index << 8) + 0x18);
    return 0.0f;
}

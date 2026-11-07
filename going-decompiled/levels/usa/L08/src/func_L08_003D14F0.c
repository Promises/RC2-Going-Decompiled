/* Level 08 (Tabora), USA v2.00: a level-only function built from C (task
 * #2627, RULING #13291 row L08-G1). Compiled ALONE as its own TU and linked at
 * its one place, 0x003D14F0, in place of its split asm; the row is
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
 * func_L08_003D14F0 - is a handle's entry class byte set? (second copy.)
 *
 * @index: the entity handle; -1 is invalid.
 * Return: 1 when the entry's byte at +0x20 (read through the pointer the
 * entry keeps at +0x68, then +0x14) is nonzero, else 0.
 *
 * Upstream carries this body twice (func_L08_003CCD28 and func_L08_003D0098);
 * the ROM holds both copies, so both are adopted, each at its own place.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/08_tabora/L08_002D90C8-g8.c, function
 * func_L08_003D0098 (v1.01 address), author commit 886754b0 (Louis-Philippe
 * Le Sieur). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L08_003D0098 -> func_L08_003D14F0 (our split name at the
 *     v2.00 place);
 *   - D_L08_001B29DC kept: upstream's name already states the v2.00 target
 *     address 0x001B29DC (inside the split data label D_L08_001B2200,
 *     +0x7DC); address-named extern valued from the name by level_elf
 *     (RULING #13291 section 2);
 *   - the file's `#include "common.h"` line, the TI128 typedef, the other
 *     functions and their externs not carried (nothing else here uses them).
 */
int func_L08_003D14F0(int index) {
    if (index != -1) {
        if (*(unsigned char *)(*(unsigned char **)(D_L08_001B29DC
                                                  + ((unsigned int)index << 8) + 0x68)
                              + 0x14) != 0)
            return 1;
    }
    return 0;
}

/* Level 08 (Tabora), USA v2.00: a level-only function built from C (task
 * #2627, RULING #13291 row L08-G1). Compiled ALONE as its own TU and linked at
 * its one place, 0x00330050, in place of its split asm; the row is
 * going-decompiled/levels/usa/L08/config/L08_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned int ti_word __attribute__((mode(TI)));

/* The level's per-handle 0x10-stride shape lists, indexed by handle. Lives
 * inside the split data label D_L08_001F0000 (+0xE7C0): address-named extern
 * valued from the name by level_elf (RULING #13291 section 2). */
extern int *D_L08_001FE7C0[];
/* L08's copy of the shared point-vs-shape distance, at its v2.00 place. */
extern float func_L08_002FAC20(void *, void *);

/**
 * func_L08_00330050 - the largest shape distance over one handle's list.
 *
 * @object: the query point block, passed through to the distance helper.
 * @index: the entity handle; -1 returns `minimum` unchanged.
 * @minimum: the running floor.
 *
 * list = D_L08_001FE7C0[index]; its first word is a count, and entries are
 * 0x10 bytes from list+4. Each entry's distance against `object` is taken
 * with func_L08_002FAC20 and the running maximum is kept in `minimum`.
 * Return: the updated minimum.
 *
 * Non-obvious: upstream names the callee func_00282E80 (a v1.01 shared-code
 * name), but the ROM's relocation here targets 0x002FAC20, which is L08's
 * func_L08_002FAC20; the name is respelled to the disc-implied target.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/08_tabora/L08_002D90C8.c, function
 * func_L08_0032EED8 (v1.01 address), author commit b31a91e7 (MantraAU).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L08_0032EED8 -> func_L08_00330050 (our split name at the
 *     v2.00 place);
 *   - D_L08_001FE7C0 kept: upstream's name already states the v2.00 target
 *     address 0x001FE7C0 (inside the split data label D_L08_001F0000,
 *     +0xE7C0); address-named extern valued from the name by level_elf
 *     (RULING #13291 section 2);
 *   - func_00282E80 -> func_L08_002FAC20 (the disc-implied callee at
 *     0x002FAC20; upstream's v1.01 name no longer states the target);
 *   - the file's `#include` line, other functions and their externs not
 *     carried (nothing else here uses them); the ti_word typedef kept.
 */
float func_L08_00330050(ti_word *object, int index, float minimum) {
    int *list;
    int i;
    if (index == -1) return minimum;
    list = D_L08_001FE7C0[index];

    for (i = 0; i < *list; i++) {
        float value = func_L08_002FAC20((char *)(list + 4) + i * 0x10, object);
        if (minimum < value) minimum = value;
    }
    return minimum;
}

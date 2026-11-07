/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x003F7F48, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

typedef float f32;

/* The level's per-frame step coefficient at 0x001B0E98 (our split label).
   `sda` is upstream's own text, ignored by this cc1 ("attribute directive
   ignored"): at -G8 the 4-byte float is $gp-relative by its size alone. */
extern f32 D_L09_001B0E98 __attribute__((sda));

/* The level's copies of boot vector helpers, named by their v2.00 places. */
extern void func_L09_002EB128(f32 *, const f32 *, const f32 *);
extern void func_L09_002EB180(char *, char *, f32);
extern void func_L09_002EB0F8(char *, char *, char *);

/**
 * func_L09_003F7F48 - rescale the object's vector by the level's step
 * coefficient and move the sub-object's second point and the object along it.
 *
 * The sub-object's pair of points (p and p+0x10) is differenced into a
 * 16-byte frame local (func_L09_002EB128), the difference is scaled by
 * D_L09_001B0E98 (func_L09_002EB180), and the scaled vector is added to the
 * second point and then that point to the object's position at +0x10
 * (func_L09_002EB0F8, twice). The sub-object's first point is left alone.
 *
 * @a0: the object; its sub-object is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_002F8BA0-g8.c, function func_L09_003F6AB8 (v1.01
 * address), author commit 431ac503 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL:
 * VERBATIM but for names (no device); the respellings, each to the target of
 * the v2.00 disc's own relocation at this place (task #2561 relverify):
 *   - func_L09_003F6AB8 -> func_L09_003F7F48 (our split name at the v2.00 place);
 *   - D_L09_001B0EE8 -> D_L09_001B0E98 (our split label at the v2.00 target);
 *   - func_00282CC0 -> func_L09_002EB0F8, func_00282CF0 -> func_L09_002EB128,
 *     func_00282D30 -> func_L09_002EB180;
 *   - `#include "common.h"` dropped: its f32 typedef is written here, only the
 *     declarations this function uses are kept, and upstream's comments are
 *     folded into this one.
 */
void func_L09_003F7F48(unsigned char *a0) {
    unsigned char *s0 = a0;
    unsigned char *p = *(unsigned char **)(s0 + 0x68);
    unsigned char *s1 = p + 0x10;
    char buf[16];
    func_L09_002EB128((f32 *)buf, (const f32 *)p, (const f32 *)s1);
    func_L09_002EB180(buf, buf, D_L09_001B0E98);
    func_L09_002EB0F8((char *)s1, (char *)s1, buf);
    func_L09_002EB0F8((char *)(s0 + 0x10), (char *)(s0 + 0x10), (char *)s1);
}

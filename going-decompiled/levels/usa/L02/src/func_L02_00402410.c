/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x00402410, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm the SDK 2.9 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;
typedef short s16;
typedef int s32;

/* A word inside L02's larger .bss label D_L02_001B3930, so address-named
 * (RULING #13291 §2). */
extern int D_L02_001CAA80;

/**
 * func_L02_00402410 - test an object for class 0xB59 in its armed state.
 *
 * @a0: the object.
 *
 * True when its class (+0xAA) is 0xB59, the level word D_L02_001CAA80 is 1
 * and the object's byte +0x20 is exactly 8.
 * Return: D_L02_001CAA80 (that is, 1) when all hold, else 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003F5430.c,
 * function func_L02_00400CC8 (v1.01 address), author commit 597e1ea6 (ReaperOfVeriod).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_00400CC8 -> func_L02_00402410 (our split name at
 *     the v2.00 place);
 *   - D_0017A80 -> D_L02_001CAA80 (upstream's name states no address;
 *     the v2.00 relocation targets 0x001CAA80);
 *   - u8/s16/s32 typedefs carried from the file's own;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
s32 func_L02_00402410(char *a0) {
    s32 r = 0;
    if (*(s16 *)(a0 + 0xAA) == 0xB59 && D_L02_001CAA80 == 1)
        r = (*(u8 *)(a0 + 0x20) ^ 8) ? 0 : D_L02_001CAA80;
    return r;
}

/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003FD5C0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned int u32;
typedef short s16;
typedef int s32;

extern s32 func_L02_00401D98(char *, s32);
/* L02's boot-name map name at 0x001BA3C0 (symbol_addrs/L02_boot_names.txt);
 * the boot program's own split names that address differently (NOTE #13409,
 * class A3), which does not hold for this level. */
extern char g_cameraState[];

/**
 * func_L02_003FD5C0 - make an object the actor's current one and start its
 * message.
 *
 * @a0: the actor; its state block is at +0x68.
 * @a1: the object.
 *
 * The level record g_cameraState keeps a current object at +0x284: if it is
 * the actor's previous one (state +0xD0) it is dropped, and an empty slot takes
 * a1 with +0x288 cleared. The actor's +0xD0 becomes a1; class 0x9F8 starts
 * message 0xF and class 0xA44 message 0x15 (func_L02_00401D98).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003F5430.c,
 * function func_L02_003FC020 (v1.01 address), author commit 773eafc2 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003FC020 -> func_L02_003FD5C0 (our split name at
 *     the v2.00 place);
 *   - func_L02_00400650 -> func_L02_00401D98 (v2.00 place);
 *   - D_L02_001BA3C0 -> g_cameraState (task #2646): upstream's spelling
 *     (the same address in v2.00) respelled to the name
 *     symbol_addrs/L02_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2
 *     class, NOTE #13409);
 *   - u32/s16/s32 typedefs carried from the file's own;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L02_003FD5C0(char *a0, char *a1) {
    char *a3 = *(char **)(a0 + 0x68);
    char *g = g_cameraState;
    s16 t;
    if (*(u32 *)(g + 0x284) == *(u32 *)(a3 + 0xD0))
        *(u32 *)(g + 0x284) = 0;
    if (*(u32 *)(g + 0x284) == 0) {
        *(u32 *)(g + 0x284) = (u32)a1;
        *(u32 *)(g + 0x288) = 0;
    }
    *(u32 *)(a3 + 0xD0) = (u32)a1;
    t = *(s16 *)(a1 + 0xAA);
    if (t == 0x9F8)
        func_L02_00401D98(a0, 0xF);
    else if (t == 0xA44)
        func_L02_00401D98(a0, 0x15);
}

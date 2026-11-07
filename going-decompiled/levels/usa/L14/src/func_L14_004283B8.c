/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x004283B8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned long u64;

/* The boot's GS screen context (resident core, below 0x1A7C80), read as
 * halfwords as upstream declares it. */
extern short g_gsScreenContext[];

/* L14's copies of boot helpers, at their v2.00 places. */
extern void func_L14_00387180(unsigned int reg, u64 value);

extern void func_L14_002F0190(int, int, int, int, int);

/**
 * func_L14_004283B8 - queue the level jingle at the volume the object asks for.
 *
 * @a0: the object; its state block is at +0x68.
 *
 * The volume is (int)((1.0 - state float +0x68) * 128.0) in the top byte.
 * Calls func_L14_00387180(0x42, 9), then
 * func_L14_002F0190(0, g_gsScreenContext[0xA9], 0, g_gsScreenContext[0xA8],
 * volume).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98.c,
 * function func_L14_00426DE8 (v1.01 address), author commit 431ac503 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; edits:
 *   - name func_L14_00426DE8 -> func_L14_004283B8 (our split name at
 *     the v2.00 place);
 *   - D_001A6480 -> g_gsScreenContext (our name at 0x001A6480);
 *     upstream's `short []` type kept;
 *   - func_385CB8 -> func_L14_00387180 and func_2EF6D8 ->
 *     func_L14_002F0190 (L14's copies);
 *   - func_385CB8's `unsigned long` parameter written `u64` (RULING
 *     #13197; `u64` is `unsigned long`, 64-bit on the EE, typedef
 *     written here as the boot's common.h has it);
 *   - the audit's `long long`->`long` respelling (RULING #13197) is
 *     needed only to compile the upstream FILE whole on s136; no line
 *     carried here contains `long long`;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_004283B8(char *a0)
{
    char *v0;
    int s0;

    v0 = *(char **)(a0 + 0x68);
    s0 = ((int)((1.0f - *(float *)(v0 + 0x68)) * 128.0f)) << 24;
    func_L14_00387180(0x42, 9);
    func_L14_002F0190(0, g_gsScreenContext[0xA9], 0, g_gsScreenContext[0xA8], s0);
}

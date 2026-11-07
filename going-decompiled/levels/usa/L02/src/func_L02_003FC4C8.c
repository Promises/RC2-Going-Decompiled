/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003FC4C8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned short u16;

/* L02's copies of boot / level-00 helpers, at their v2.00 places. */
extern int func_L02_002F2998(void *);
extern void func_L02_002EAE30(char *, char *);
extern void func_L02_00312210(void);
/* A function of this level, passed by address (upstream declares it as data). */
extern char func_L02_00401C20[];

/**
 * func_L02_003FC4C8 - per-frame update of a controller actor.
 *
 * @a0: the actor; its state block is at +0x68.
 *
 * Bumps the halfword frame count at +0x96 (held at 0xFFFF), ticks the
 * countdown at state +0xE4 (func_L02_002F2998), hands func_L02_00401C20 and
 * the actor to func_L02_002EAE30 when its byte +0x20 is above 1, and runs
 * func_L02_00312210.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003F5430.c,
 * function func_L02_003FAF28 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003FAF28 -> func_L02_003FC4C8 (our split name at
 *     the v2.00 place);
 *   - D_L02_004004D8 -> func_L02_00401C20 (the v2.00 relocation target
 *     is that function's glabel; upstream's `char []` type kept);
 *   - func_00282948 -> func_L02_002F2998, func_L00_002E6EF8 ->
 *     func_L02_002EAE30, func_L00_0030CF60 -> func_L02_00312210 (L02's
 *     copies);
 *   - u8/u16 typedefs carried from the file's own;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L02_003FC4C8(char *a0) {
    char *s0 = a0;
    char *obj = *(char **)(s0 + 0x68);
    u16 timer = *(u16 *)(s0 + 0x96);

    if (timer <= 0xFFFE)
        *(u16 *)(s0 + 0x96) = timer + 1;
    func_L02_002F2998(obj + 0xE4);
    if (*(u8 *)(s0 + 0x20) > 1)
        func_L02_002EAE30(func_L02_00401C20, s0);
    func_L02_00312210();
}

/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x00444870, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

/* RULING #8620 addressing device (level code has no native arm): upstream's
 * `__attribute__((sda))` is written `section(".sdata")` on these extern
 * DECLARATIONS. It moves no data and emits nothing: cc1 then prints the
 * one-instruction `lw/sw $r,SYM` macro, which GNU as expands to an adjacent
 * lui + lw/sw pair (absolute %hi/%lo, not $gp-relative at -G0), as retail
 * has it. Ablation (task #2565): with upstream's `sda` (ignored by this cc1)
 * the function differs from the disc in 11 of 34 words (masked compare). */
extern unsigned int g_mobyTableBase __attribute__((section(".sdata")));
extern int func_L20_0044B010(char *);
extern void func_L20_0039D1B8(char *, char *, int, int);

/**
 * func_L20_00444870 - bind a moby's group entity to it and orient it.
 *
 * p = the entity record of id state +0x238 (0x100-byte records at
 * g_mobyTableBase). When func_L20_0044B010(p) accepts it, calls
 * func_L20_0044AF30(p, moby, other) and func_L20_0039D1B8(p, p + 0x10, 0x168,
 * 0).
 *
 * @arg0: the moby; its +0x68 points at the state. @arg1: passed through.
 * Return: p, or 0 when refused.
 *
 * Non-obvious: func_L20_0044AF30 has no prototype in upstream's file (an
 * implicit int declaration); kept, it does not change the call.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0043ECB0.c, function func_L20_00443328 (v1.01
 * address), author commit 765097bb (ReaperOfVeriod). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_00443328 -> func_L20_00444870; D_L20_001B2F9C ->
 *     g_mobyTableBase; func_0039BDF0 -> func_L20_0039D1B8; func_L20_004499E8 ->
 *     func_L20_0044AF30; func_L20_00449AC8 -> func_L20_0044B010
 *   - D_L20_001B2F1C -> g_mobyTableBase (task #2646): the name
 *     symbol_addrs/L20_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `__attribute__((sda))` written `__attribute__((section(".sdata")))` on
 *     the extern declarations (RULING #8620 addressing device, comment above
 *     them)
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 *   - the duplicate declaration of g_mobyTableBase written once
 */
char *func_L20_00444870(char *arg0, char *arg1) {
    char *state = *(char **)(arg0 + 0x68);
    unsigned int p = g_mobyTableBase + (*(int *)(state + 0x238) << 8);

    if (func_L20_0044B010((char *)p) != 0) {
        func_L20_0044AF30((char *)p, arg0, arg1);
        func_L20_0039D1B8((char *)p, (char *)(p + 0x10), 0x168, 0);
        return (char *)p;
    }
    return 0;
}

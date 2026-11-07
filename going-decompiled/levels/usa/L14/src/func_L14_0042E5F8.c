/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x0042E5F8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef long s64;

/* The boot's fp-bit / libm routines (resident core), passing doubles as
 * 64-bit integers as upstream declares them. 0x001234F0 has no live name in
 * symbol_addrs (the boot's C calls it func_001234F0), so it is address-named.
 * cc1 warns "conflicting types for built-in function `sqrt'" at the sqrt
 * declaration: it takes and returns the double's bit pattern as s64, as
 * upstream declares it. The level gate measures the bytes unchanged by it. */
extern void *func_001234F0(float scale);

extern s64 sqrt(s64 value);

extern s64 dpmul(s64 a, s64 b);

extern void *dpdiv(void *a, s64 b);

extern float dptofp(s64 a);

/* L14's copy of the boot's state-change helper. */
extern void func_L14_00323250(char *a0, int a1, int a2);

/**
 * func_L14_0042E5F8 - store the object's scale once, while it is being set up.
 *
 * @a0: the object; its state block is at +0x68.
 *
 * Only while its byte +0x20 is 0: converts the float at +0xC to a double
 * (func_001234F0), divides it by sqrt(2.0) * 1024.0 (sqrt, dpmul, dpdiv on
 * the doubles' bit patterns), stores the result as a float (dptofp) at state
 * +0x10, and moves to state 1 (func_L14_00323250(a0, 1, -1)).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98.c,
 * function func_L14_0042D018 (v1.01 address), author commit 431ac503 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; edits:
 *   - name func_L14_0042D018 -> func_L14_0042E5F8 (our split name at
 *     the v2.00 place);
 *   - func_801234F0 -> func_001234F0 (address-named: no live
 *     symbol_addrs name at 0x001234F0);
 *   - func_80131430 -> sqrt, func_80122B00 -> dpmul, func_80122DA8 ->
 *     dpdiv, func_80123298 -> dptofp (our boot names at those
 *     addresses; upstream's 0x80000000-based spellings encode the
 *     same jal);
 *   - func_80321F58 -> func_L14_00323250 (L14's copy);
 *   - `long long` written `s64` (RULING #13197: the audit's `long
 *     long`->`long` respelling; `s64` is `long`, 64-bit on the EE,
 *     typedef written here as the boot's common.h has it); the `LL`
 *     suffixes of the two constants kept;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_0042E5F8(char *a0)
{
    char *p;
    void *s0;
    s64 v;

    p = *(char **)(a0 + 0x68);
    if (*(unsigned char *)(a0 + 0x20) != 0)
        return;
    s0 = func_001234F0(*(float *)(a0 + 0xC));
    v = sqrt(0x4000000000000000LL);
    v = dpmul(v, 0x4090000000000000LL);
    v = dpdiv(s0, v);
    *(float *)(p + 0x10) = dptofp(v);
    func_L14_00323250(a0, 1, -1);
}

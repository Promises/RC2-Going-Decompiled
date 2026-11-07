/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F23E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/**
 * func_L14_003F23E0 - reset a shadow/axis record: the four axis pairs, the
 * flags and the packed word.
 *
 * @a0: the record.
 * Return: nothing.
 *
 * Non-obvious: the two 8-byte stores are upstream's own `long long` stores
 * (0, and the packed constant 280993940374112 = 8 bytes of flags). The arm is
 * sdk29, where EE `long long` is 64 bits and this compiles as written; on the
 * s136 arm the same text fails ("unsupported wide integer operation", the
 * FACT #13321 class), so the s64 respelling is NOT applied here.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c, function
 * func_L14_003F0E98 (v1.01 address), author commit 7495b38a (Louis-Philippe
 * Le Sieur; the same account as llesieur99, FACT #37797; the definition line
 * blames 7495b38a too). Adopted with the authors' permission (RULINGs #10405,
 * #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_003F0E98 -> func_L14_003F23E0 (our split name at the
 *     v2.00 place);
 *   - the file's `#include` lines, other functions and their externs not
 *     carried (nothing here uses them; the body's `long long` stores are
 *     upstream's own text, kept).
 */
void func_L14_003F23E0(char *a0)
{
    *(long long *)(a0 + 0x70) = 0LL;
    *(int *)(a0 + 0x50) = 0;
    *(int *)(a0 + 0x54) = 0;
    *(float *)(a0 + 0x58) = 1.0f;
    *(int *)(a0 + 0x5C) = 0;
    *(int *)(a0 + 0x60) = 0;
    *(float *)(a0 + 0x64) = 1.0f;
    *(float *)(a0 + 0x68) = 1.0f;
    *(float *)(a0 + 0x6C) = 1.0f;
    *(float *)(a0 + 0x8) = -4.0f;
    *(float *)(a0 + 0x18) = 4.0f;
    *(float *)(a0 + 0x4) = -2.0f;
    *(float *)(a0 + 0x24) = 2.0f;
    *(int *)(a0 + 0x0) = 0;
    *(int *)(a0 + 0x10) = 0;
    *(int *)(a0 + 0x20) = 0;
    *(int *)(a0 + 0x30) = 0;
    *(float *)(a0 + 0xC) = 1.0f;
    *(float *)(a0 + 0x1C) = 1.0f;
    *(float *)(a0 + 0x2C) = 1.0f;
    *(long long *)(a0 + 0x80) = 280993940374112LL;
    *(float *)(a0 + 0x28) = -4.0f;
    *(float *)(a0 + 0x38) = 4.0f;
    *(float *)(a0 + 0x14) = -2.0f;
    *(float *)(a0 + 0x34) = 2.0f;
    *(float *)(a0 + 0x3C) = 1.0f;
}

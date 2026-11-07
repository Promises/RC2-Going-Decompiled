/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003FE020, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

typedef unsigned char u8;
typedef int s32;

extern void func_L02_0031F998(char *, s32, s32);
extern char g_heroPos[];
extern s32 func_L02_00401D98(char *, s32);
extern void func_L02_003FD130(char *a0);
extern void func_L02_003FD808(char *a0, s32 a1);
extern void func_L02_003FDAB8(char *a0, s32 a1);
extern int func_L02_0031FA68(int);
extern void func_L02_002DAA58(char *);
extern void func_L02_003082E0(int, int);
extern void func_L02_002F2D20(void *);
extern int func_L02_003FB110(int);
extern void func_L02_002BD368(char *, float *, int, int);
extern void func_L02_00360B18(void *, void *);
extern void func_L02_00335928(int, int);
/* Inside the larger split label D_L02_001B3930: address-named (RULING #13291 §2). */
extern int D_L02_001CAA80;
/* The boot's D_1395B8 (core, below 0x1A7C80): the name the boot's code uses
 * for this address, an operand name inside the boot label D_139599 rather
 * than a label of its own, and not inside g_quickSelectMap; spelled as the
 * boot and the L02 split asm spell it (task #2601, FACT #13369; validator
 * #2602). */
extern u8 D_1395B8[];
/* Two tables of 128-byte records, inside the larger split label
 * D_L02_001B2D80: address-named. ADDRESSING-MODEL DEVICE (RULING #8620, EE
 * arm only) on these three: `.sdata` on the extern declaration makes cc1
 * print the one-instruction `lw`/`sw` macro, which GNU as expands absolute at
 * -G0, as retail has it. It moves no data and emits nothing. */
extern char *D_L02_001B368C __attribute__((section(".sdata")));
extern char *D_L02_001B369C __attribute__((section(".sdata")));
extern int D_L02_001AA1B0 __attribute__((section(".sdata")));

/**
 * func_L02_003FE020 - start the actor.
 * @a0: the actor; its state block is the pointer at +0x68.
 *
 * Calls func_L02_00401D98 with 0x25 plus a random pick below 2
 * (func_L02_0031FA68) and func_L02_0031F998(@a0, 1, -1). Releases the object
 * the state holds at +0xC0 (func_L02_002DAA58) and clears the slot. When
 * D_L02_001CAA80 is set, calls func_L02_003082E0(0, state +0x224). Then
 * zeroes a vector (func_L02_002F2D20) and, by the flag
 * D_1395B8[func_L02_003FB110(0)], sets its third float to -1.59 and hands
 * it with the record state +0x74 of D_L02_001B369C, or to 2.16 with the
 * record state +0x78 of D_L02_001B368C (each at +0x30), to
 * func_L02_002BD368. Then func_L02_00360B18 on g_heroPos and g_heroPos+0x10,
 * the actor's group handlers (func_L02_003FD130), func_L02_003FD808 and
 * func_L02_003FDAB8 with 0, func_L02_00335928 with state +0x224 and +0x22C,
 * and finally D_L02_001AA1B0 = -1.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_003F5430.c, function func_L02_003FCA80
 * (v1.01 address), author commit 24d977dd (bigmak94). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_003FCA80 -> func_L02_003FE020 (our split name at the
 *     v2.00 place);
 *   - D_00189EA0 -> g_heroPos (the boot's name, same address);
 *   - func_L02_00400650 -> func_L02_00401D98, func_002A77E0 ->
 *     func_L02_0031FA68, func_002A7750 -> func_L02_0031F998,
 *     func_L00_002D7140 -> func_L02_002DAA58, func_L00_00303530 ->
 *     func_L02_003082E0, func_00282C88 -> func_L02_002F2D20,
 *     func_L02_003F9B70 -> func_L02_003FB110, func_L00_002B9D60 ->
 *     func_L02_002BD368, func_002DF3F8 -> func_L02_00360B18,
 *     func_L02_003FBB90 -> func_L02_003FD130, func_L02_003FC268 ->
 *     func_L02_003FD808, func_L02_003FC518 -> func_L02_003FDAB8,
 *     func_L01_00323310 -> func_L02_00335928: our split names at the v2.00
 *     addresses the ROM's own relocations imply (relverify, task #2557);
 *   - D_001395B8 -> D_1395B8: the same address, respelled to the boot's and
 *     the L02 split asm's existing name for it (task #2601, FACT #13369);
 *   - D_L02_001CAA80, D_L02_001B368C, D_L02_001B369C and D_L02_001AA1B0
 *     keep their upstream names, which already state their v2.00 addresses
 *     (all but the last address-named, RULING #13291 §2; D_L02_001AA1B0 is
 *     our split label);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     D_L02_001B368C, D_L02_001B369C and D_L02_001AA1B0 declarations (RULING
 *     #8620; upstream's `sda` is a marker for its own patched compiler,
 *     which our cc1 ignores);
 *   - `#include "include_asm.h"` dropped (nothing here used it); the file's
 *     own typedefs kept; the siblings defined in the same upstream file
 *     (func_L02_003FBB90, func_L02_003FC268, func_L02_003FC518) are declared
 *     from their definitions' prototypes; every other declaration dropped
 *     and the upstream comment folded into this one.
 */
void func_L02_003FE020(char *a0) {
    char *s1 = *(char **)(a0 + 0x68);
    float v[4];

    func_L02_00401D98(a0, func_L02_0031FA68(2) + 0x25);
    func_L02_0031F998(a0, 1, -1);
    if (*(char **)(s1 + 0xC0) != 0) {
        func_L02_002DAA58(*(char **)(s1 + 0xC0));
        *(s32 *)(s1 + 0xC0) = 0;
    }
    if (D_L02_001CAA80 != 0)
        func_L02_003082E0(0, *(s32 *)(s1 + 0x224));
    func_L02_002F2D20(v);
    if (D_1395B8[func_L02_003FB110(0)] != 0) {
        v[2] = -1.59f;
        func_L02_002BD368(D_L02_001B369C + *(s32 *)(s1 + 0x74) * 128 + 0x30, v, 0, 1);
    } else {
        v[2] = 2.16f;
        func_L02_002BD368(D_L02_001B368C + *(s32 *)(s1 + 0x78) * 128 + 0x30, v, 0, 1);
    }
    func_L02_00360B18(g_heroPos, g_heroPos + 0x10);
    func_L02_003FD130(a0);
    func_L02_003FD808(a0, 0);
    func_L02_003FDAB8(a0, 0);
    func_L02_00335928(*(s32 *)(s1 + 0x224), *(s32 *)(s1 + 0x22C));
    D_L02_001AA1B0 = -1;
}

/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003D3700, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* Upstream include/common.h's own definitions of the typedefs the body uses. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed int s32;

/* L03's copies of boot helpers, at their v2.00 places. */
extern void func_L03_002B8718(s32 a0);
extern void func_L03_003146C0(s32 a0);
extern void func_L03_002BF080(char *a0, char *a1, s32 a2, s32 a3);
extern void func_L03_00312AA0(void *a0);
extern void func_L03_00330570(s32 a0, s32 a1);
extern void func_L03_0035B2F8(char *a0, char *a1);
extern void func_L03_0030EA10(s32 a0, s32 a1);
/* The boot's block at 0x00189E20 (core). */
extern char g_soundBankHandlesBlk[];
/* A boot byte at 0x001395C0: no name exists at that exact address (L03's
 * split has D_1395B8 eight bytes below; the boot splits and symbol_addrs.txt
 * name nothing there), so it is address-named (RULING #13291 section 2). */
extern u8   D_001395C0;
/* Upstream's typed view of the level's entity table base, L03's boot-name
 * map name g_mobyTableBase at 0x001B2A1C (an asm-label view, upstream's own
 * text, RULING #13168). Upstream marks it `sda`; here it is the
 * ADDRESSING-MODEL DEVICE of RULING #8620: a section(".sdata") attribute on
 * the extern DECLARATION (no data moved, nothing emitted) that only changes
 * how cc1 addresses it. Ablation (this TU, s136 -G0, plain as, masked
 * compare at the place): without it 2 of 63 words differ from the disc,
 * with it 0. */
extern u8 *D_L03_001B2A1C_tbl[] __asm__("g_mobyTableBase") __attribute__((section(".sdata")));

/**
 * func_L03_003D3700 - end the level's scripted sequence and hand control
 * back.
 *
 * @a0: the sequence object; its block pointer is at +0x68.
 * Return: nothing.
 *
 * Calls func_L03_002B8718(0), clears g_soundBankHandlesBlk's halfword
 * +0x24FC, calls func_L03_003146C0 with the object's byte +0xB0, sets the
 * boot byte D_001395C0 and six more resident bytes (0x001A7B02..04 and
 * 0x001A7B3A..3C) to 1, and calls func_L03_002BF080(a0 + 0x10, a0 + 0xF0, 0,
 * 1). Frees the moby at block+0x2C when there is one, and the table entry
 * block[0] of g_mobyTableBase (func_L03_00312AA0). Stores 0x80468CDC at
 * +0x78, then calls func_L03_00330570(block+0x24, -1),
 * func_L03_0035B2F8(g_soundBankHandlesBlk + 0x80, g_soundBankHandlesBlk +
 * 0x90) and func_L03_0030EA10(0, -1).
 *
 * Non-obvious: upstream's note on its file says this is USA v2.00's own
 * function: the six resident bytes func_L03_003D20F8 (their v1.01 code) sets
 * sit 8 bytes further on here, written at their constant addresses (the
 * retail's `lui $at,0x1A; sb` form, no relocation).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/us2/levels/03_endako/L03_003D3700.c, function
 * func_US2_L03_003D3700 (v2.00 address), author commit 1d1f5ade (Flavius;
 * `git blame -M -C` names it for all 29 definition lines at fe6adf9b, and
 * `git log --reverse -S'func_US2_L03_003D3700('` agrees). Its new text is
 * the name and the six constant-address stores; every other non-blank line
 * is an image of earlier text in func_L03_003D20F8
 * (src/levels/03_endako/L03_00399230.c), which RULING #13427 revs 2-4 and
 * #13541 rev 2 credit to its earliest writers by `git blame -M -C` there:
 * Gianni (b32a5088, 2026-10-22, from the draft
 * nonmatching/levels/func_L03_003D20F8.c: the first five calls/stores, the
 * 0x80468CDC store and the last three calls), Jacob (4e18867a,
 * 2026-10-28T15:20Z: the table view, the two locals s2/p, the
 * func_2BEE78 call and the two frees) and llesieur99 (431ac503,
 * 2026-10-12: the s0/s1 locals); 1d1f5ade is 2026-10-28T17:14Z, after all
 * three. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED; edits:
 *   - name func_US2_L03_003D3700 -> func_L03_003D3700 (our split name, the
 *     same v2.00 place);
 *   - `__attribute__((sda))` written `__attribute__((section(".sdata")))`
 *     (RULING #8620 addressing-model device on an extern declaration,
 *     ablation above);
 *   - the asm label "D_L03_001B2A1C" -> "g_mobyTableBase", the name L03's
 *     boot-name map gives at that address (the NAMES rule: no second
 *     spelling); the C identifier D_L03_001B2A1C_tbl kept;
 *   - D_00189E20_11 -> g_soundBankHandlesBlk (the boot's name at
 *     0x00189E20); D_001395C0 kept (address-named, see above);
 *   - seven callees as L03's copies, the v2.00 ROM's call targets:
 *     func_2B8510 -> func_L03_002B8718, func_313378 -> func_L03_003146C0,
 *     func_2BEE78 -> func_L03_002BF080, func_311758 -> func_L03_00312AA0,
 *     func_32F140 -> func_L03_00330570, func_359FD0 -> func_L03_0035B2F8,
 *     func_30D710 -> func_L03_0030EA10;
 *   - `#include "common.h"` dropped, its three typedefs written out;
 *     upstream's file comment folded into this one.
 */
void func_L03_003D3700(char *a0)
{
    char *s0 = a0;
    char *s1 = *(char **)(s0 + 0x68);
    char *s2;
    void *p;

    func_L03_002B8718(0);
    s2 = g_soundBankHandlesBlk;
    *(u16 *)(s2 + 0x24FC) = 0;
    func_L03_003146C0(*(u8 *)(s0 + 0xB0));
    D_001395C0 = 1;
    *(u8 *)0x001A7B3A = 1;
    *(u8 *)0x001A7B03 = 1;
    *(u8 *)0x001A7B3B = 1;
    *(u8 *)0x001A7B04 = 1;
    *(u8 *)0x001A7B3C = 1;
    *(u8 *)0x001A7B02 = 1;
    func_L03_002BF080(s0 + 0x10, s0 + 0xF0, 0, 1);
    p = *(void **)(s1 + 0x2C);
    if (p != 0) {
        func_L03_00312AA0(p);
    }
    func_L03_00312AA0(D_L03_001B2A1C_tbl[0] + (*(s32 *)s1 << 8));
    *(s32 *)(s0 + 0x78) = 0x80468CDC;
    func_L03_00330570(*(s32 *)(s1 + 0x24), -1);
    func_L03_0035B2F8(s2 + 0x80, s2 + 0x90);
    func_L03_0030EA10(0, -1);
}

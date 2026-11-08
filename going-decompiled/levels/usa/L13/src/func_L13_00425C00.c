/* Level 13 (Boldan), USA v2.00: a level-only function built from C (task
 * #2672, RULING #13291 row L13-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00425C00, in place of its split asm; the row is
 * going-decompiled/levels/usa/L13/config/L13_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;
extern char g_heroPos[];
extern u32 g_mobyTableBase[] __attribute__((section(".sdata")));
extern s32 func_L13_0032B7C0(u8 *, s32);
extern u8 D_1395B8[];

/**
 * func_L13_00425C00 - latch five progress bytes of D_1395B8 from the
 * hero's position and one moby's state.
 *
 * @a0: the object; its state block is at +0x68 (ids 0..3, moby index 4).
 * Return: nothing.
 *
 * For ids 2, 3, 0 and 1 in that order, when func_L13_0032B7C0(g_heroPos, id)
 * reports non-zero, the bytes 0x25, 0x24, 0x27 and 0x28 of D_1395B8 are set
 * to 1 if still 0. The byte 0x26 is set the same way when the moby at
 * g_mobyTableBase + index * 0x100 has its state byte +0x20 equal to 2.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/13_boldan/L13_00306F38.c, function func_L13_00424728 (v1.01 address), author commit daf0e4a1 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_L13_00424728 -> func_L13_00425C00 (our split name at the v2.00 place);
 *   - D_001395B8 -> D_1395B8 (the boot name L13_boot_names.txt gives at that address,
 *   core);
 *   - D_00189EA0 -> g_heroPos (our boot name at that address);
 *   - D_L13_001B291C -> g_mobyTableBase: the name symbol_addrs/L13_boot_names.txt gives at
 *   exactly 0x001B289C, where the split has no label of its own (NAMES rule,
 *   task #2646 class A2);
 *   - func_002ACBC8 -> func_L13_0032B7C0 (the v2.00 relocation target, our name there);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on g_mobyTableBase (RULING #8620
 *   ADDRESSING-MODEL DEVICE, EE arm only; this TU has no other arm): our cc1
 *   does not know `sda`/`nosda` (it warns "directive ignored"); with `.sdata` it prints
 *   the one-instruction access macro, which GNU as expands at -G0 to the
 *   adjacent absolute lui + %lo pair retail has. It moves no
 *   data and emits nothing;
 *   - ablation (task #2672): with upstream's `sda`/`nosda` spelling and nothing
 *   else changed, 10 of 72 masked words differ;
 *   - upstream's func_32A4D0 and func_002ACBC8 are one function (both relocate
 *   to 0x0032B7C0), so after respelling they are one declaration; the second is
 *   dropped (the body names only func_002ACBC8);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L13_00425C00(u8 *a0) {
    s32 *ptr = *(s32 **)(a0 + 0x68);
    if (func_L13_0032B7C0((u8 *)g_heroPos, ptr[2]) != 0) {
        if (D_1395B8[0x25] == 0) {
            D_1395B8[0x25] = 1;
        }
    }
    if (func_L13_0032B7C0((u8 *)g_heroPos, ptr[3]) != 0) {
        if (D_1395B8[0x24] == 0) {
            D_1395B8[0x24] = 1;
        }
    }
    if (func_L13_0032B7C0((u8 *)g_heroPos, ptr[0]) != 0) {
        if (D_1395B8[0x27] == 0) {
            D_1395B8[0x27] = 1;
        }
    }
    if (func_L13_0032B7C0((u8 *)g_heroPos, ptr[1]) != 0) {
        if (D_1395B8[0x28] == 0) {
            D_1395B8[0x28] = 1;
        }
    }
    if (*((u8 *)g_mobyTableBase[0] + (ptr[4] << 8) + 0x20) == 2) {
        if (D_1395B8[0x26] == 0) {
            D_1395B8[0x26] = 1;
        }
    }
}

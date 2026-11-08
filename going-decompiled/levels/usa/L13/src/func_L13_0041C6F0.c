/* Level 13 (Boldan), USA v2.00: a level-only function built from C (task
 * #2672, RULING #13291 row L13-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x0041C6F0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L13/config/L13_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef float f32;
typedef unsigned char u8;
typedef int s32;
extern s32 func_L13_0032B7C0(u8 *, s32);
extern char g_heroPos[];
extern f32 D_L13_001B24E8 __attribute__((section(".sdata")));

/**
 * func_L13_0041C6F0 - set a level value from which of two volumes the
 * hero is in.
 *
 * @a0: the object; its state block is at +0x68 and holds two ids.
 * Return: nothing.
 *
 * D_L13_001B24E8 becomes 30.0 when func_L13_0032B7C0(g_heroPos, id 0)
 * reports non-zero, then 95.0 when it does for id 1 (the second test wins).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/13_boldan/L13_00306F38.c, function func_L13_0041B260 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_L13_0041B260 -> func_L13_0041C6F0 (our split name at the v2.00 place);
 *   - D_00189EA0 -> g_heroPos (our boot name at that address);
 *   - D_L13_001B2568 -> D_L13_001B24E8: an address-named extern (RULING #13291 section 2) at
 *   0x001B24E8, inside the split label D_L13_001B2080 (+0x468); no name exists at that exact address in
 *   the boot splits (labels or operands), symbol_addrs.txt, L13_boot_names.txt
 *   or the level's split labels (grep with firing controls, task #2672);
 *   - func_32A4D0 -> func_L13_0032B7C0 (the v2.00 relocation target, our name there);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on D_L13_001B24E8 (RULING #8620
 *   ADDRESSING-MODEL DEVICE, EE arm only; this TU has no other arm): our cc1
 *   does not know `sda`/`nosda` (it warns "directive ignored"); with `.sdata` it prints
 *   the one-instruction access macro, which GNU as expands at -G0 to the
 *   adjacent absolute lui + %lo pair retail has. It moves no
 *   data and emits nothing;
 *   - ablation (task #2672): with upstream's `sda`/`nosda` spelling and nothing
 *   else changed, 16 of 28 masked words differ (0x70 bytes, not 0x74);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L13_0041C6F0(u8 *a0) {
    s32 *ptr = *(s32 **)(a0 + 0x68);
    u8 *base = g_heroPos;
    if (func_L13_0032B7C0(base, ptr[0]) != 0) D_L13_001B24E8 = 30.0f;
    if (func_L13_0032B7C0(base, ptr[1]) != 0) D_L13_001B24E8 = 95.0f;
}

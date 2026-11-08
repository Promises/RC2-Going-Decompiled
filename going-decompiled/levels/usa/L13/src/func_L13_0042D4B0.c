/* Level 13 (Boldan), USA v2.00: a level-only function built from C (task
 * #2672, RULING #13291 row L13-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x0042D4B0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L13/config/L13_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef float f32;
typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
extern s32 func_L13_002AF8F0(u8 *);
extern void func_L13_00328FF0(f32 *, f32 *, f32, f32, f32, f32);
extern u32 g_mobyTableBase[] __attribute__((section(".sdata")));
extern u8 D_L13_001B1E10[];

/**
 * func_L13_0042D4B0 - lower an object when a watched moby reaches state 2
 * (with a func_L13_002AF8F0 call).
 *
 * @a0: the object (height float +0x18); its state block is at +0x68.
 * Return: nothing.
 *
 * The same three states as func_L13_0042D3B8, except that on the change to
 * state 2 it sets the state first and then calls
 * func_L13_002AF8F0(D_L13_001B1E10).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/13_boldan/L13_00306F38.c, function func_L13_0042BFD8 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_L13_0042BFD8 -> func_L13_0042D4B0 (our split name at the v2.00 place);
 *   - D_L13_001B1E60 -> D_L13_001B1E10 (our split label at the v2.00 address);
 *   - D_L13_001B291C -> g_mobyTableBase: the name symbol_addrs/L13_boot_names.txt gives at
 *   exactly 0x001B289C, where the split has no label of its own (NAMES rule,
 *   task #2646 class A2);
 *   - func_2AF870 -> func_L13_002AF8F0 (the v2.00 relocation target, our name there);
 *   - func_327D00 -> func_L13_00328FF0 (the v2.00 relocation target, our name there);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on g_mobyTableBase (RULING #8620
 *   ADDRESSING-MODEL DEVICE, EE arm only; this TU has no other arm): our cc1
 *   does not know `sda`/`nosda` (it warns "directive ignored"); with `.sdata` it prints
 *   the one-instruction access macro, which GNU as expands at -G0 to the
 *   adjacent absolute lui + %lo pair retail has. It moves no
 *   data and emits nothing;
 *   - ablation (task #2672): with upstream's `sda`/`nosda` spelling and nothing
 *   else changed, 5 of 57 masked words differ;
 *   - upstream declares func_2AF870, func_327D00 and D_L13_001B1E60 twice (at the
 *   file head and again above the function); the repeat is dropped;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L13_0042D4B0(u8 *a0) {
    u8 *ptr = *(u8 **)(a0 + 0x68);
    if (a0[0x20] == 0) {
        *(u16 *)(a0 + 0x34) |= 0x800;
        *(f32 *)(ptr + 0x8) = *(f32 *)(a0 + 0x18);
        *(f32 *)(ptr + 0x4) = *(f32 *)(a0 + 0x18) - 6.0f;
        a0[0x20] = 1;
    }
    if (a0[0x20] == 1) {
        u8 v = *((u8 *)g_mobyTableBase[0] + (*(u32 *)ptr << 8) + 0x20);
        if (v == 2) {
            a0[0x20] = v;
            func_L13_002AF8F0(D_L13_001B1E10);
        }
    }
    if (a0[0x20] == 2) {
        func_L13_00328FF0((f32 *)(a0 + 0x18), (f32 *)(ptr + 0xC), *(f32 *)(ptr + 0x4),
                    0.0016666667f, 0.0016666667f, 0.200000018f);
    }
}

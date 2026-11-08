/* Level 13 (Boldan), USA v2.00: a level-only function built from C (task
 * #2672, RULING #13291 row L13-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x0042D3B8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L13/config/L13_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef float f32;
typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
extern u32 g_mobyTableBase[] __attribute__((section(".sdata")));
extern void func_L13_0036A110(s32, s32, u8 *);
extern void func_L13_00328FF0(f32 *, f32 *, f32, f32, f32, f32);

/**
 * func_L13_0042D3B8 - lower an object when a watched moby reaches state 2
 * (with a func_L13_0036A110 call).
 *
 * @a0: the object (height float +0x18); its state block is at +0x68.
 * Return: nothing.
 *
 * State 0 sets bit 0x800 of the flags halfword +0x34, stores the height in
 * the state's +0x8 and the height minus 6.0 in its +0x4, and moves to state
 * 1. In state 1, when the moby whose index is the state's first word has
 * state byte 2, it calls func_L13_0036A110(0, 0, a0) and moves to state 2.
 * In state 2 it calls func_L13_00328FF0(height, state + 0xC, the lowered
 * height, 0.0016666667, 0.0016666667, 0.2) every frame.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/13_boldan/L13_00306F38.c, function func_L13_0042BEE0 (v1.01 address), author commit daf0e4a1 (Flavius); its
 * lines from `u8 *ptr = ...` to `if (v == 2) {` (10 of 20) are llesieur99's
 * text from func_L13_0042BFD8 in the same file (author commit 91e04732,
 * earlier; git blame -M -C), so both are credited (RULING #13427 rev 2,
 * #13541 rev 2).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_L13_0042BEE0 -> func_L13_0042D3B8 (our split name at the v2.00 place);
 *   - D_L13_001B291C -> g_mobyTableBase: the name symbol_addrs/L13_boot_names.txt gives at
 *   exactly 0x001B289C, where the split has no label of its own (NAMES rule,
 *   task #2646 class A2);
 *   - func_002AA548 -> func_L13_00328FF0 (the v2.00 relocation target, our name there);
 *   - func_002E5CF8 -> func_L13_0036A110 (the v2.00 relocation target, our name there);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on g_mobyTableBase (RULING #8620
 *   ADDRESSING-MODEL DEVICE, EE arm only; this TU has no other arm): our cc1
 *   does not know `sda`/`nosda` (it warns "directive ignored"); with `.sdata` it prints
 *   the one-instruction access macro, which GNU as expands at -G0 to the
 *   adjacent absolute lui + %lo pair retail has. It moves no
 *   data and emits nothing;
 *   - ablation (task #2672): with upstream's `sda`/`nosda` spelling and nothing
 *   else changed, 5 of 60 masked words differ;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L13_0042D3B8(u8 *a0) {
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
            func_L13_0036A110(0, 0, a0);
            a0[0x20] = v;
        }
    }
    if (a0[0x20] == 2) {
        func_L13_00328FF0((f32 *)(a0 + 0x18), (f32 *)(ptr + 0xC), *(f32 *)(ptr + 0x4),
                      0.0016666667f, 0.0016666667f, 0.200000018f);
    }
}

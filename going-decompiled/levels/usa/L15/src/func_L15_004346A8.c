/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x004346A8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* EE `long` is 64-bit; RULING #13197 spells it s64 in adopted bodies. */
typedef long s64;
extern void func_L15_003984B0(int, int);
extern void func_L15_003015B0(int, int, int, int, s64);
extern int func_L15_00307608(void *);
extern void func_L15_00434820(void *, int, int, float, float, float, int, int, int);
extern short g_gsScreenContext[];
extern char D_L15_00269F60[];

/**
 * func_L15_004346A8 - set up the screen context and, while the object is
 * idle, the level's four effect blocks.
 *
 * @a0: the object; its state block is at +0x68.
 * Return: nothing.
 *
 * Calls func_L15_003984B0(0x42, 0x48) and func_L15_003015B0 with the
 * screen context's halfwords 0xA9 and 0xA8 and colour 0x40404040. When the
 * timer at state +0x50 has not expired, the four 0xA0-byte blocks at
 * D_L15_00269F60 are each set up by func_L15_00434820 with their own size
 * (0x46 or 0x20), scale and colour.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_004328E8.c, function func_L15_004328E8 (v1.01 address), author commit daf0e4a1 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED (no device; the prototype's `long` written `s64` per RULING
 * #13197 is a type respelling, byte-inert, validator #2682); edits:
 *   - name func_L15_004328E8 -> func_L15_004346A8 (our split name at the v2.00 place);
 *   - D_001A6480 -> g_gsScreenContext (our boot name at that address);
 *   - D_L15_00269FE0 -> D_L15_00269F60: an address-named extern (RULING #13291 section 2) at
 *   0x00269F60, inside the split label D_L15_00240000 (+0x29F60); no name exists at that exact address in
 *   the boot splits (labels or operands), symbol_addrs.txt, L15_boot_names.txt
 *   or the level's split labels (grep with firing controls, task #2677);
 *   - func_0027DE50 -> func_L15_003015B0 (the v2.00 relocation target, our name there);
 *   - func_00282948 -> func_L15_00307608 (the v2.00 relocation target, our name there);
 *   - func_002FC560 -> func_L15_003984B0 (the v2.00 relocation target, our name there);
 *   - func_L15_00432A60 -> func_L15_00434820 (the v2.00 relocation target, our name there);
 *   - the `long` fifth parameter of func_L15_003015B0's prototype is written
 *   `s64`, with `typedef long s64` in this TU (RULING #13197; EE `long` is
 *   64-bit, so the object is unchanged: .text, .rel.text and .symtab are
 *   identical to the `long` spelling, measured task #2677);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L15_004346A8(void *a0) {
    char *s0 = *(char **)(a0 + 0x68);

    func_L15_003984B0(0x42, 0x48);
    func_L15_003015B0(0, g_gsScreenContext[0xA9], 0, g_gsScreenContext[0xA8], 0x40404040);
    if (func_L15_00307608(s0 + 0x50) == 0) {
        char *q = D_L15_00269F60;

        func_L15_00434820(q, 0x46, 0x46, 3.2f, 6.35f, 0.2f, 0x0A, 0x007F7F7F, 0x00FF1010);
        func_L15_00434820(q + 0xA0, 0x20, 0x20, 3.1f, 6.19f, 0.2f, 0x0A, 0x007F7F7F, 0x00FF1040);
        func_L15_00434820(q + 0x140, 0x20, 0x20, 3.1f, 6.19f, 0.2f, 0x0A, 0x007F7F7F, 0x00FF1040);
        func_L15_00434820(q + 0x1E0, 0x20, 0x20, 3.1f, 6.19f, 0.2f, 0x0A, 0x007F7F7F, 0x00FF1040);
    }
}

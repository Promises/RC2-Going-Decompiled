/* Level 13 (Boldan), USA v2.00: a level-only function built from C (task
 * #2672, RULING #13291 row L13-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x0041D270, in place of its split asm; the row is
 * going-decompiled/levels/usa/L13/config/L13_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

typedef struct { u8 pad[0x80]; u8 data[0x270]; u8 more[0xC]; f32 value; } HeroBDA8;
extern HeroBDA8 g_soundBankHandlesBlk;
extern u8 *D_L13_001B298C __attribute__((section(".sdata")));
extern s32 func_L13_0032B7C0(u8 *, s32);
extern void func_L13_002F9800(u8 *, u8 *, f32);

/**
 * func_L13_0041D270 - apply a level effect to the hero for each of eight
 * volume ids it is in.
 *
 * @a0: the object; its state block is at +0x68 and holds eight ids.
 * Return: nothing.
 *
 * State 0 sets the byte +0x30 to 0xFF and moves to state 1. Then for each
 * id that is not -1 and for which func_L13_0032B7C0(g_soundBankHandlesBlk +
 * 0x80, id) reports non-zero, it calls func_L13_002F9800(+0x2F0 of the
 * block, D_L13_001B298C + id * 0x80, -1.0) and stores 10.0 at +0x2FC.
 *
 * Non-obvious: upstream views 0x00189E20 (our g_soundBankHandlesBlk) as a
 * struct whose +0x80 is g_heroPos; the struct type is kept as written.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/13_boldan/L13_0041BDA8.c, function func_L13_0041BDA8 (v1.01 address), author commit 202aee5a (bigbeno37), keeping
 * five non-brace lines of Louis-Philippe Le Sieur's earlier nonmatching
 * draft 34b3d626 (the signature, `s32 i;` and the state-0 opener), credited
 * as surviving text (RULING #13427 rev 2, #13541 rev 4, validator #2678).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_L13_0041BDA8 -> func_L13_0041D270 (our split name at the v2.00 place);
 *   - D_00189E20 -> g_soundBankHandlesBlk (our boot name at that address);
 *   - D_L13_001B2A0C -> D_L13_001B298C: an address-named extern (RULING #13291 section 2) at
 *   0x001B298C, inside the split label D_L13_001B2080 (+0x90C); no name exists at that exact address in
 *   the boot splits (labels or operands), symbol_addrs.txt, L13_boot_names.txt
 *   or the level's split labels (grep with firing controls, task #2672);
 *   - func_00282F20 -> func_L13_002F9800 (the v2.00 relocation target, our name there);
 *   - func_002ACBC8 -> func_L13_0032B7C0 (the v2.00 relocation target, our name there);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on D_L13_001B298C (RULING #8620
 *   ADDRESSING-MODEL DEVICE, EE arm only; this TU has no other arm): our cc1
 *   does not know `sda`/`nosda` (it warns "directive ignored"); with `.sdata` it prints
 *   the one-instruction access macro, which GNU as expands at -G0 to the
 *   adjacent absolute lui + %lo pair retail has. It moves no
 *   data and emits nothing;
 *   - ablation (task #2672): with upstream's `sda`/`nosda` spelling and nothing
 *   else changed, 50 of 57 masked words differ (0xE4 bytes, not 0xDC);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L13_0041D270(u8 *a0) {
    s32 *ids = *(s32 **)(a0 + 0x68);
    s32 i;

    if (a0[0x20] == 0) {
        a0[0x30] = 0xFF;
        a0[0x20] = 1;
    }
    for (i = 0; i < 8; i++) {
        if (ids[i] != -1 && func_L13_0032B7C0(g_soundBankHandlesBlk.data, ids[i]) != 0) {
            func_L13_002F9800(g_soundBankHandlesBlk.data + 0x270, D_L13_001B298C + (ids[i] << 7), -1.0f);
            g_soundBankHandlesBlk.value = 10.0f;
        }
    }
}

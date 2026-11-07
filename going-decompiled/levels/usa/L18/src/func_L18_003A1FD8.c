/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003A1FD8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;

extern s32 func_L18_003092E0(s16 *timer);
extern s32 func_L18_0030FA60(s32 key, s32 selection);
extern s32 func_L18_0033C668(void *point, s32 index);
extern s32 func_L18_0037CA38(s32 index, s32 value, void *object);
extern void func_L18_00341F68(s32 first, s32 second);
extern u16 D_0018CF40;
extern u8 g_mapDataSet[] __attribute__((section(".sdata")));
extern u8 g_skillPointFlags[] __attribute__((section(".sdata")));
extern u8 g_heroPos[];
extern u8 g_soundBankHandlesBlk[];
extern u32 D_L18_001A903C[] __attribute__((section(".sdata")));

/**
 * func_L18_003A1FD8 - Damosel trigger object: hint timer and the skill-point
 * two-zone check.
 *
 * @entity: the object; its state block is at +0x68 (zone indices at +0 and
 * +4, sub-state at +8, a timer at +0xE).
 *
 * On the first call it sets byte +0x30 to 0xFF, the timer to 40, +0xC to 0
 * and the state byte to 1. While D_0018CF40 is clear and g_mapDataSet[0] is
 * set, each expiry of the timer (func_L18_003092E0) calls
 * func_L18_0030FA60(0xA27, 0x7E). In state 1: sub-state 0 waits until the
 * hero block (g_heroPos) is in zone +0 (func_L18_0033C668) and its word
 * +0x221C is 15, then moves to sub-state 1; sub-state 1 drops back to 0 when
 * the word +0x2294 of the g_soundBankHandlesBlk record is 0x3E, otherwise,
 * once that record +0x80 is in zone +4 and g_skillPointFlags[7] is clear, it
 * sets the flag, calls func_L18_0037CA38(1, 0, 0) and
 * func_L18_00341F68(0x1233, -1), and sets D_L18_001A903C[0].
 * Return: nothing.
 *
 * Non-obvious: the values stored into the flag and D_L18_001A903C are the
 * sub-state (1), and the zone check's success in sub-state 0 writes the
 * mode byte (1) as the new sub-state; upstream keeps both as such.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/us2/levels/18_damosel/L18_003A1FD8.c,
 * function func_US2_L18_003A1FD8 (v2.00 address), author commit c4928ea3 (Flavius),
 * which copied the text, token-identical but for names, from the v1.01
 * function func_L18_003A0C28 (src/levels/18_damosel/L18_002E68B0.c), written
 * by bigbeno37 in e4fd2fe8, moving Gianni's draft b32a5088, whose first-run
 * initialisation block survives verbatim (RULING #13427 revs 2-4).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_US2_L18_003A1FD8 -> func_L18_003A1FD8 (our split name at the v2.00 place);
 *   - D_00189E20 -> g_soundBankHandlesBlk (our boot name at that address);
 *   - D_00189EA0 -> g_heroPos (our boot name at that address);
 *   - D_001A7A60 -> g_skillPointFlags (our boot name at 0x001A7A68, the address the v2.00
 *   relocation implies; upstream's spelling states an address 8 lower);
 *   - D_001A7AFD -> g_mapDataSet (our boot name at 0x001A7B05, the address the v2.00
 *   relocation implies; upstream's spelling states an address 8 lower);
 *   - func_00282978 -> func_L18_003092E0 (the v2.00 relocation target, our name there);
 *   - func_002ACBC8 -> func_L18_0033C668 (the v2.00 relocation target, our name there);
 *   - func_002E5E58 -> func_L18_0037CA38 (the v2.00 relocation target, our name there);
 *   - func_US2_00289840 -> func_L18_0030FA60 (the v2.00 relocation target, our name there);
 *   - func_US2_002B1880 -> func_L18_00341F68 (the v2.00 relocation target, our name there);
 *   - D_0018CF40 kept: upstream's name already states the v2.00 address
 *   (relverify 0x0018CF40; the nearest lower name in our labels is g_health,
 *   0xC54 below); no name exists at
 *   that exact address (same grep); address-named (RULING #13291 section 2);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *   two extern declarations that carried it, D_001A7AFD and D_L18_001A903C
 *   (RULING #8620 ADDRESSING-MODEL
 *   DEVICE, EE arm only; this TU has no other arm): our cc1 then prints the
 *   one-instruction load macro that GNU as expands absolute at -G0, as retail
 *   has it. Ablation: with upstream's `sda` spelling on those two, 4 of 84
 *   masked words differ, the first at +0x54 (validator #2666). It moves no
 *   data and emits nothing. The third declaration, D_001A7A60, carries
 *   upstream's MACRO_ADDR, which include/common.h defines as
 *   `__attribute__((section(".sdata")))`: written out as its expansion, not
 *   the device;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them).
 */
void func_L18_003A1FD8(u8 *entity) {
    u8 *state;
    u32 substate;
    u8 mode;

    state = *(u8 **)(entity + 0x68);
    if (entity[0x20] == 0) {
        entity[0x30] = 0xFF;
        *(u16 *)(state + 0xE) = 0x28;
        *(u16 *)(state + 0xC) = 0;
        entity[0x20] = 1;
    }
    if (D_0018CF40 != 0)
        goto after;
    if (g_mapDataSet[0] != 0) {
        if (func_L18_003092E0((s16 *)(state + 0xE)) != 0)
            func_L18_0030FA60(0xA27, 0x7E);
    }
after:
    mode = entity[0x20];
    if (mode != 1)
        goto end;
    substate = *(u32 *)(state + 0x8);
    switch (substate) {
    case 0: {
        u8 *block = g_heroPos;
        if (func_L18_0033C668(block, *(u32 *)(state + 0)) != 0) {
            if (*(u32 *)(block + 0x221C) == 15)
                *(u32 *)(state + 8) = mode;
        }
        break;
    }
    case 1: {
        u8 *block = g_soundBankHandlesBlk;
        if (*(u32 *)(block + 0x2294) == 0x3E) {
            *(u32 *)(state + 8) = 0;
        } else if (func_L18_0033C668(block + 0x80, *(u32 *)(state + 4)) != 0) {
            if (g_skillPointFlags[7] == 0) {
                g_skillPointFlags[7] = substate;
                func_L18_0037CA38(1, 0, 0);
                func_L18_00341F68(0x1233, -1);
                D_L18_001A903C[0] = substate;
            }
        }
        break;
    }
    }
end:
    return;
}

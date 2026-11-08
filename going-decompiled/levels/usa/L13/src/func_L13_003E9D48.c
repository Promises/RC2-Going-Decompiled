/* Level 13 (Boldan), USA v2.00: a level-only function built from C (task
 * #2672, RULING #13291 row L13-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003E9D48, in place of its split asm; the row is
 * going-decompiled/levels/usa/L13/config/L13_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;

extern u8 g_soundBankHandlesBlk[];
extern u8 g_heroPos[];
extern u8 D_18C750[];
extern u8 D_1395B8[];
extern u32 D_1A7390[] __attribute__((section(".sdata")));
extern u8 g_skillPointFlags[] __attribute__((section(".sdata")));
extern s32 D_L13_001A903C[] __attribute__((section(".sdata")));
extern s32 D_0018CC18;
extern s32 func_L13_0032B7C0(void *a0, s32 a1);
extern void func_L13_0036A270(s32 a0, s32 a1, s32 a2);
extern void func_L13_00330630(s32 a0, s32 a1);
extern void func_L13_002FF938(s32 a0, s32 a1);

/**
 * func_L13_003E9D48 - drive a two-stage volume trigger that awards a skill
 * point and plays a limited sound cue.
 *
 * @self: the object; its state block is at +0x68.
 * Return: nothing.
 *
 * State 0 sets +0x30 to 0xFF, clears the state's halfword +0xC and moves to
 * state 1. In state 1 the stage halfword (+0xE) is 0 until the hero is in
 * volume +4 (func_L13_0032B7C0) with word +0x221C of g_heroPos equal to 0xF;
 * in stage 1 a hero word +0x2294 of 0x3E resets the stage, otherwise volume
 * +8 sets g_skillPointFlags[4] once, calls func_L13_0036A270(1, 0, 0) and
 * func_L13_00330630(0x1233, -1) and sets D_L13_001A903C[0]. Then, if the
 * hero is in volume +0 and the D_18C750 timer test (+0x1BC/+0x1C0 against
 * D_1A7390, a 0x69780 window) passes, it plays func_L13_002FF938(0x9E3, 0x3A)
 * when D_0018CC18 is not negative and counts D_1395B8[0x7E] up, at most
 * twice, once per object (the state's +0xC).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/us2/levels/13_boldan/L13_003E9D48.c, function func_US2_L13_003E9D48 (v2.00 address), author commit c4928ea3 (Flavius), a rename-copy of
 * func_L13_003E8850 in src/levels/13_boldan/L13_003E8850.c, the same function at
 * its v1.01 address: 37 of its lines are Lynder063's nonmatching draft
 * (author commits 42b4c94e and 8058f251), carried into src by bigbeno37 with
 * 9 lines of their own (author commit 202aee5a); all three are earlier than
 * c4928ea3 and credited (RULING #13427 rev 2 and rev 4, #13541 rev 2).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_US2_L13_003E9D48 -> func_L13_003E9D48 (our split name at the v2.00 place);
 *   - D_001395B8 -> D_1395B8 (the boot name L13_boot_names.txt gives at that address,
 *   core);
 *   - D_00189E20 -> g_soundBankHandlesBlk (our boot name at that address);
 *   - D_00189EA0 -> g_heroPos (our boot name at that address);
 *   - D_0018C750 -> D_18C750 (the boot name L13_boot_names.txt gives at that address,
 *   core);
 *   - D_001A7390 -> D_1A7390 (the boot name L13_boot_names.txt gives at that address,
 *   core);
 *   - D_001A7A60 -> g_skillPointFlags (our boot name at that address);
 *   - D_001A903C -> D_L13_001A903C (our split label at the v2.00 address; the boot-name map's
 *   D_1A8CEC at the same address is not used, an exact split label wins);
 *   - func_002ACBC8 -> func_L13_0032B7C0 (the v2.00 relocation target, our name there);
 *   - func_002E5E58 -> func_L13_0036A270 (the v2.00 relocation target, our name there);
 *   - func_US2_00289840 -> func_L13_002FF938 (the v2.00 relocation target, our name there);
 *   - func_US2_002B1880 -> func_L13_00330630 (the v2.00 relocation target, our name there);
 *   - D_0018CC18 kept: upstream's name already states the v2.00 address
 *   (0x0018CC18, inside g_health (+0x92C); no name exists at that exact
 *   address (same grep); address-named (RULING #13291 section 2);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on D_1A7390, g_skillPointFlags and D_L13_001A903C (RULING #8620
 *   ADDRESSING-MODEL DEVICE, EE arm only; this TU has no other arm): our cc1
 *   does not know `sda`/`nosda` (it warns "directive ignored"); with `.sdata` it prints
 *   the one-instruction access macro, which GNU as expands at -G0 to the
 *   adjacent absolute lui + %lo pair retail has. It moves no
 *   data and emits nothing;
 *   - ablation (task #2672): with upstream's `sda`/`nosda` spelling and nothing
 *   else changed, 65 of 109 masked words differ (0x1B4 bytes, not 0x1B8);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L13_003E9D48(u8 *self)
{
    u8 *state = *(u8 **)(self + 0x68);
    s16 h;
    u8 *hero;
    u8 *timers;
    u8 *counters;

    if (self[0x20] == 0) {
        self[0x30] = 0xFF;
        *(s16 *)(state + 0xC) = 0;
        self[0x20] = 1;
    }
    if (self[0x20] != 1)
        return;
    h = *(s16 *)(state + 0xE);
    switch (h) {
    case 0:
        if (func_L13_0032B7C0(g_heroPos, *(s32 *)(state + 4)) != 0 && *(s32 *)(g_heroPos + 0x221C) == 0xF)
            *(s16 *)(state + 0xE) = 1;
        break;
    case 1:
        hero = g_soundBankHandlesBlk;
        if (*(s32 *)(hero + 0x2294) == 0x3E) {
            *(s16 *)(state + 0xE) = 0;
        } else if (func_L13_0032B7C0(hero + 0x80, *(s32 *)(state + 8)) != 0 && g_skillPointFlags[4] == 0) {
            g_skillPointFlags[4] = 1;
            func_L13_0036A270(1, 0, 0);
            func_L13_00330630(0x1233, -1);
            D_L13_001A903C[0] = 1;
        }
        break;
    }
    if (func_L13_0032B7C0(g_heroPos, *(s32 *)state) == 0)
        return;
    timers = D_18C750;
    if (*(u16 *)(timers + 0x1BC) != 0 && !(D_1A7390[0] - *(u32 *)(timers + 0x1C0) > 0x69780))
        return;
    counters = D_1395B8;
    if (counters[0x7E] < 2 && *(s16 *)(state + 0xC) == 0) {
        if (D_0018CC18 >= 0)
            func_L13_002FF938(0x9E3, 0x3A);
        counters[0x7E] = counters[0x7E] + 1;
        *(s16 *)(state + 0xC) = 1;
    }
}

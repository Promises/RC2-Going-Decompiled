/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x00425068, in place of its split asm; the row is
 * going-decompiled/levels/usa/L10/config/L10_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

/* The mission state, as far as this function resets it. */
typedef struct {
    u8 pad00[0x10];
    s32 f10;                    /* 0x10 */
    s32 rounds;                 /* 0x14 */
    u8 pad18[0x28 - 0x18];
    s32 f28;                    /* 0x28 */
    s32 waiting;                /* 0x2C */
    f32 f30;                    /* 0x30 */
} MissionState;

/* A 40-byte reward slot: available while value has not reached limit. */
typedef struct {
    u8 pad[16];
    s32 limit;                  /* 0x10 */
    s32 value;                  /* 0x14 */
    u8 tail[16];
} RewardSlot;

/* Level words at our split labels. `sda` is upstream's own text, ignored by
   this cc1: at -G8 a 4-byte word is $gp-relative by its size alone. */
extern s32 D_L10_001B1340 __attribute__((sda));  /* the selected kind, -2 for none */
extern s32 D_L10_001B1344 __attribute__((sda));
extern s32 D_L10_001B13D8 __attribute__((sda));  /* set while a kind is being chosen */
/* The mission state at 0x00221C80. No label starts there in our split (it
   lies inside a larger data label), so this extern is ADDRESS-NAMED and the
   level ELF values it from its name (RULING #13291 §2). */
extern MissionState D_L10_00221C80;
extern RewardSlot D_L10_002A3C50[];

/* The level's copies of boot code, named by their v2.00 places. */
extern s32 func_L10_0037DB20(s32 kind);
extern void func_L10_0037E248(void);
extern void func_L10_0037CF80(s32 value);
extern void func_L10_00380700(void);
extern void func_L10_0037CFC0(void);

/**
 * func_L10_00425068 - reset the mission state and refill the reward slot of
 * kind 6.
 *
 * Clears the selected kind to -2 and raises the choosing flag, marks the
 * mission waiting with f28 = 100, zeroes the rounds, f10 and the word at
 * 0x001B1344, sets f30 to 8.0, refills the slot func_L10_0037DB20(6) returns
 * (value = limit), and calls the four level helpers in upstream's order.
 *
 * @object: unused.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/10_hrugis_cloud/L10_003170D0-g8.c,
 * function func_L10_00423500 (v1.01 address), author commit ba383701
 * (bigbeno37); its last three call lines are Louis-Philippe Le Sieur's,
 * from the earlier nonmatching draft nonmatching/levels/func_L10_00423698.c
 * (c34a2b3e; `git blame -M -C -C -C` at the pin, RULINGs #13427 rev 3 and
 * #13541 rev 2). Adopted with the authors' permission (RULINGs #10405,
 * #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names; the
 * respellings, each to the target of the v2.00 disc's own relocation at this
 * place (task #2657 relverify):
 *   - func_L10_00423500 -> func_L10_00425068 (our split name at the v2.00 place);
 *   - D_L10_001B13A0 -> D_L10_001B1340, D_L10_001B13A4 -> D_L10_001B1344,
 *     D_L10_001B1438 -> D_L10_001B13D8, D_L10_002A3C70 -> D_L10_002A3C50
 *     (our split labels at the v2.00 targets);
 *   - D_L10_00221CD0 -> D_L10_00221C80, an address-named extern for a target
 *     inside a larger split data label (RULING #13291 §2);
 *   - func_002ED8A0 -> func_L10_0037DB20, func_L05_0037B210 ->
 *     func_L10_0037E248, func_L05_00379F28 -> func_L10_0037CF80,
 *     func_L05_0037D6E8 -> func_L10_00380700, func_L05_00379F68 ->
 *     func_L10_0037CFC0 (our split names at the v2.00 targets);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the
 *     typedefs used are written here, only this function's declarations
 *     kept, and upstream's comments folded into these.
 */
void func_L10_00425068(void *object) {
    s32 slot;

    D_L10_001B1340 = -2;
    D_L10_001B13D8 = 1;
    D_L10_00221C80.waiting = 1;
    D_L10_00221C80.f28 = 100;
    D_L10_001B1344 = 0;
    D_L10_00221C80.rounds = 0;
    slot = func_L10_0037DB20(6);
    D_L10_002A3C50[slot].value = D_L10_002A3C50[slot].limit;
    func_L10_0037E248();
    D_L10_00221C80.f30 = 8.0f;
    D_L10_00221C80.f10 = 0;
    func_L10_0037CF80(0);
    func_L10_00380700();
    func_L10_0037CFC0();
}

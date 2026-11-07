/* Level 24 (Ship Shack), USA v2.00: a level-only function built from C through
 * the boot's assembler model (task #2552, RULING #13291 section 1). Compiled
 * ALONE as its own TU and linked at its one place, 0x0037AE10, in place of its
 * split asm; the row is going-decompiled/levels/usa/L24/config/L24_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2552): it puts the four
 * `li.s` float constants in a gp-relative .lit4 pool (12 bytes) rather than
 * expanding them inline as SN's assembler does, so .text is 0x1E8 bytes, not
 * the place's 0x20C, and level_elf.py cobj refuses the object.
 */

typedef struct {
    float x, y, z, w;
} Point;

/* The hero view the boot keeps at g_soundBankHandlesBlk (0x189E20): +0x80 is
 * the hero position (g_heroPos), +0x2294/+0x229C the movement mode and the
 * activity state (upstream's field names). */
typedef struct {
    unsigned char pad0[0x80];
    Point position;
    unsigned char pad90[8];
    float heading;
    unsigned char pad9c[0x2294 - 0x9c];
    int mode;
    unsigned char pad2298[4];
    int activity;
} PlayerView;

extern unsigned char g_soundBankHandlesBlk[];
/* RULING #8620 addressing devices, EE arm only (level code has no native arm):
 * a section attribute on an extern DECLARATION moves no data and emits nothing.
 * ".sdata" makes cc1 print the one-instruction `lw $r,SYM` (gp-relative at
 * -G8); ".data" keeps D_138324 absolute (%hi/%lo). Ablation (task #2552):
 * with all three attributes removed, .text is 0x208 bytes, not 0x20C, and
 * level_elf.py cobj refuses the object. */
extern int D_L24_001AEB70 __attribute__((section(".sdata")));
extern int D_L24_001AEB74 __attribute__((section(".sdata")));
extern int D_L24_001A8F00;
/* 0x138324, spelled as the boot already spells it: the %hi/%lo operand of
 * UpdateSubtitleStateMachine (asm/usa/nonmatchings/text/188858/
 * UpdateSubtitleStateMachine.s:219), declared by boot C
 * (src/usa/text/188858.c:2045) and listed in L24_boot_names.txt:119 as a core
 * name. Address-named; tools/levels/level_elf.py values it from its name
 * (RULING #13291 section 2). */
extern unsigned int D_138324 __attribute__((section(".data")));
extern void func_L24_00319B50(unsigned char *, unsigned char, int);
extern void func_L24_003231F0(char *, float *, int, int, float);
extern float func_L24_002ED790(void *, void *);
extern float func_L24_002ED520(float);
extern float func_L24_002EDC50(float, float);
extern float func_L24_002EE688(float, float);
extern int func_L24_00323528(int, int, int);
extern int func_L24_00325408(int, int, int, int, unsigned char *);

#define PLAYER ((PlayerView *)g_soundBankHandlesBlk)

/**
 * func_L24_0037AE10 - update one Ship Shack interaction object (a three-state
 * machine in its byte +0x20).
 *
 * State 0: func_L24_00319B50(object, 1, -1), the level's copy of the boot's
 * func_002A85B8, then nothing else.
 * State 1: func_L24_003231F0(object, its +0x68 pointer, D_L24_001AEB70,
 * D_L24_001AEB74, 0.043633237 = 2.5 degrees in radians). The hero may use it
 * only when: his activity is 0, or the mode is 3, or the
 * activity is 1; he is within 3.0 units (DistXYVu0's copy) and within 0.7 in
 * height (GetFloatAbs's copy); the angle between his heading and the direction
 * to the object is at most pi/2 (Atan2fPoly's and AngleAbsDiffPi's copies);
 * and his mode is neither 0x1C nor 0x31. If permitted, `selected` is
 * func_L24_00323528(1, 0x12EB, 1) != 0 (the level's copy of func_002B1A90).
 * When bit 0x10 of D_138324 is set and `selected`, it calls
 * func_L24_00325408(5, 1, object, 0, 0) (RequestGameStateChange's copy) and
 * moves to state 2.
 * State 2: back to state 1 once D_L24_001A8F00 (g_nGameState) is 0.
 *
 * @object: the object (moby); +0x10/+0x14/+0x18 position, +0x20 state,
 *          +0x68 animation data.
 *
 * Non-obvious: every call target is the LEVEL's own copy of a boot function,
 * at its L24 place, not the boot's address (the level program carries them).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/us2/levels/24_ship_shack/L24_0037AE10-g8.c (blob 84e6dd46), function
 * func_US2_L24_0037AE10, author commit c4928ea3 (Flavius). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED, the
 * body's statements unchanged; respellings:
 *   - name func_US2_L24_0037AE10 -> func_L24_0037AE10 (our split name);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` and
 *     `__attribute__((nosda))` -> `__attribute__((section(".data")))`
 *     (RULING #8620; upstream's patched compiler reads sda/nosda, ours does not);
 *   - D_00189E20 -> g_soundBankHandlesBlk (the boot's name, same address);
 *   - D_00138324 -> D_138324: the same address, respelled to the boot's
 *     existing name for it (task #2621, census NOTE #13409);
 *   - D_L24_001AEBB8 -> D_L24_001AEB70, D_L24_001AEBBC -> D_L24_001AEB74:
 *     upstream's names state v1.01 addresses, ours are the targets of the
 *     v2.00 disc's own gp-relative loads;
 *   - calls to boot addresses -> the L24 copies the disc calls:
 *     func_002A7750 -> func_L24_00319B50, func_L00_00326048 ->
 *     func_L24_003231F0, func_00282E80 -> func_L24_002ED790, func_00282C48 ->
 *     func_L24_002ED520, func_00283248 -> func_L24_002EDC50, func_00283C80 ->
 *     func_L24_002EE688, func_002B0C28 -> func_L24_00323528, func_002B49E0 ->
 *     func_L24_00325408;
 *   - the repeated `((PlayerView *)D_00189E20)` cast spelled as the PLAYER
 *     macro; `#include "include_asm.h"` dropped (nothing used it); comments
 *     added.
 */
void func_L24_0037AE10(unsigned char *object)
{
    unsigned int permitted;
    int selected;
    float *animation = *(float **)(object + 0x68);
    switch (object[0x20]) {
    case 0:
        func_L24_00319B50(object, 1, -1);
        break;
    case 1:
        permitted = 1;
        selected = 0;
        func_L24_003231F0((char *)object, animation,
                          D_L24_001AEB70, D_L24_001AEB74, 0.043633237f);
        if (PLAYER->activity && PLAYER->mode != 3) {
            if (PLAYER->activity != 1) permitted = 0;
        }
        if (func_L24_002ED790(object + 0x10, &PLAYER->position) > 3.0f ||
            func_L24_002ED520(PLAYER->position.z - *(float *)(object + 0x18)) > 0.7f)
            permitted = 0;
        if (func_L24_002EE688(PLAYER->heading,
              func_L24_002EDC50(*(float *)(object + 0x10) - PLAYER->position.x,
                                *(float *)(object + 0x14) - PLAYER->position.y)) > 1.5707965f)
            permitted = 0;
        if (PLAYER->mode == 0x1c || PLAYER->mode == 0x31) permitted = 0;
        if (permitted) selected = func_L24_00323528(1, 0x12EB, 1) != 0;
        if ((D_138324 & 0x10) && selected) {
            func_L24_00325408(5, 1, (int)object, 0, 0);
            object[0x20] = 2;
        }
        break;
    case 2:
        if (!D_L24_001A8F00) object[0x20] = 1;
        break;
    }
}

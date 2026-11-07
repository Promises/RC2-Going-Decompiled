/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * through the boot's assembler model (task #2579, RULING #13291 row L02-ASM).
 * Compiled ALONE as its own TU and linked at its one place, 0x003D5870, in
 * place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by tools/ee/asm_unit.sh (the row's `asm_unit` column; the plain-`as`
 * ablation is in the row's FACT, task #2579).
 */

extern int D_L02_001B01A0 __attribute__((sda));
extern int D_L02_001FF6B8[];
extern float D_L02_001BA500[4];
extern int func_L02_002F3F40(float);
extern float func_L02_002F2F18(void *, void *);
extern void func_L02_00321DB0(char *);
extern void *func_L02_00322040(char *, int, int);
extern int func_L02_0032CF98(char *, void *, int);
extern void func_L02_0031F6B8(char *, int, int, int);
extern void func_L02_003DCDE8(char *);
extern void func_L02_003FD5C0(void *, void *);
extern void func_L02_003D5E68(char *);
extern int func_L02_003309B0(char *, int, int, int, float, float);

/**
 * func_L02_003D5870 - per-frame update of an L02 enemy: set-up, trigger and
 * damage bookkeeping.
 *
 * @self: the enemy (moby); +0x20 its state, +0x68 its state block.
 * Return: none.
 *
 * Non-obvious: in state 0 it sets D_L02_001B01A0 to 1, fills the block's
 * movement and timing parameters, and enters its next state
 * (func_L02_003D5E68). While +0x31 is set and it is within 60 units of
 * D_L02_001BA500, it calls func_L02_00321DB0 and sets +0x6F to 0x38. While
 * +0x330 is 0 it reads a hit (func_L02_00322040). The hit's damage at +0x2C is
 * zeroed in states below 5, or in state 10 with +0x324 below 5. It is scaled by
 * 0.3 from classes 0xC89/0xBFE. Then damage * 32 is added to
 * D_L02_001FF6B8[block +0x37]. A reaction (func_L02_0032CF98) records the
 * attacker and clears D_L02_001B01A0 unless the source class is 0x47 or 0. It
 * plays animation 16 (on +0x95 == 4) or 4 (on +0x95 == 3 with +0x20 <= 0). In
 * any non-zero state with +0x2E == 2, it hands the source at +0x310 to
 * func_L02_003DCDE8/func_L02_003FD5C0 when that source's state is at least 3
 * and not 0xFE/0xFD, then re-enters func_L02_003D5E68. Last, +0x322 gets
 * func_L02_003309B0's result and +0x30C a copy of +0x210.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/02_maktar_nebula/L02_003D4308-g8.c, function
 * func_L02_003D4308 (v1.01 address), author commit 13442ffb (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: verbatim but for names (no device). Respellings,
 * each to the target of the v2.00 disc's own relocation at this place
 * (relverify by name, task #2579):
 *   - func_L02_003D4308 -> func_L02_003D5870 (our split name at the v2.00
 *     place);
 *   - D_L02_001B01E0 -> D_L02_001B01A0 (our label at that address);
 *   - func_00283CF0 -> func_L02_002F3F40 (the level's own copy at its L02
 *     place);
 *   - func_00282E48 -> func_L02_002F2F18 (the level's own copy at its L02
 *     place);
 *   - func_002A93B0 -> func_L02_00321DB0 (the level's own copy at its L02
 *     place);
 *   - func_002A9640 -> func_L02_00322040 (the level's own copy at its L02
 *     place);
 *   - func_002B0ED8 -> func_L02_0032CF98 (the level's own copy at its L02
 *     place);
 *   - func_002A7470 -> func_L02_0031F6B8 (the level's own copy at its L02
 *     place);
 *   - func_L02_003DB880 -> func_L02_003DCDE8 (its v2.00 split name);
 *   - func_L02_003FC020 -> func_L02_003FD5C0 (its v2.00 split name);
 *   - func_L02_003D4900 -> func_L02_003D5E68 (its v2.00 split name);
 *   - func_002B3BA8 -> func_L02_003309B0 (the level's own copy at its L02
 *     place);
 *   - upstream's `__attribute__((sda))` on D_L02_001B01A0 kept as its text
 *     (this cc1 ignores it with a warning; the member closes with it as
 *     written);
 *   - D_L02_001FF6B8 and D_L02_001BA500 kept: address-named externs inside our
 *     split labels D_L02_001F0000 and D_L02_001B3930 (RULING #13291 section
 *     2), valued from their names by level_elf;
 *   - `#include "common.h"` dropped (this body uses nothing from it); upstream
 *     file-scope declarations it does not use are not carried; the rest keep
 *     upstream's order and text.
 */
void func_L02_003D5870(char *self) {
    char *state = *(char **)(self + 0x68);
    char *damage;

    if (*(unsigned char *)(self + 0x20) == 0) {
        D_L02_001B01A0 = 1;
        *(unsigned char *)(state + 0x48) = func_L02_002F3F40(15.0f);
        *(unsigned char *)(state + 0x4A) = func_L02_002F3F40(24.0f);
        *(unsigned char *)(state + 0xD7) = 1;
        *(unsigned char *)(state + 0x29) = 1;
        *(unsigned char *)(state + 0x1F9) = 1;
        *(short *)(state + 0x32E) = 0;
        *(int *)(state + 0x310) = 0;
        *(short *)(state + 0x318) = 0;
        *(short *)(state + 0x31A) = 0;
        *(int *)(state + 0x300) = 0;
        *(float *)(state + 0x8C) = 0.004166667f;
        *(float *)(state + 0x94) = 5.0f;
        *(float *)(state + 0x9C) = 0.15f;
        *(float *)(state + 0xBC) = 0.25f;
        *(float *)(state + 0xB8) = 0.1f;
        *(unsigned char *)(state + 0x28) = 3;
        *(float *)(state + 0x110) = *(float *)(state + 0x90) = 3.0f;
        *(float *)(state + 0x114) = 2.0f;
        *(float *)(state + 0x11C) = *(float *)(state + 0x118) = *(float *)(state + 0x1E4) = 0.5f;
        *(float *)(state + 0x120) = 1.1344641f;
        *(float *)(state + 0x140) = 0.52359885f;
        *(float *)(state + 0x144) = 0.01f;
        *(float *)(state + 0x13C) = 0.0055555557f;
        func_L02_003D5E68(self);
    }
    if (*(unsigned char *)(self + 0x31)) {
        if (func_L02_002F2F18(self + 0x10, D_L02_001BA500) < 60.0f) {
            func_L02_00321DB0(self);
            *(unsigned char *)(self + 0x6F) = 0x38;
        }
    }
    if (*(short *)(state + 0x330) == 0) {
        damage = func_L02_00322040(self, 0x10000, 0);
        if (damage) {
            if (*(unsigned char *)(self + 0x20) < 5 ||
                (*(unsigned char *)(self + 0x20) == 10 && *(short *)(state + 0x324) < 5))
                *(int *)(damage + 0x2C) = 0;
            {
                char *source = *(char **)(damage + 0x20);
                if (source) {
                    if (*(short *)(source + 0xAA) == 0xC89 || *(short *)(source + 0xAA) == 0xBFE)
                        *(float *)(damage + 0x2C) *= 0.3f;
                }
            }
            D_L02_001FF6B8[*(unsigned char *)(state + 0x37)] += (int)(*(float *)(damage + 0x2C) * 32.0f);
        }
        if (func_L02_0032CF98(self, state + 0x60, 0)) {
            if (*(int *)(self + 0x98) == 0)
                *(int *)(self + 0x98) = *(int *)(*(char **)(self + 0x24) + 0x10);
            {
                char *source = *(char **)(state + 0x38);
                if (source && *(short *)(source + 0xAA) != 0x47 && *(short *)(source + 0xAA) != 0)
                    D_L02_001B01A0 = 0;
            }
            if (*(unsigned char *)(self + 0x95) == 4) {
                if (*(unsigned char *)(self + 0x43) != 16)
                    func_L02_0031F6B8(self, 16, 0, 3);
            }
            if (*(unsigned char *)(self + 0x95) == 3 && *(float *)(state + 0x20) <= 0.0f) {
                if (*(unsigned char *)(self + 0x43) != 4)
                    func_L02_0031F6B8(self, 4, 0, 3);
            }
        }
        if (*(unsigned char *)(self + 0x20) != 0 && *(unsigned char *)(state + 0x2E) == 2) {
            char *source = *(char **)(state + 0x310);
            if (source && *(unsigned char *)(source + 0x20) != 0xFE &&
                *(unsigned char *)(source + 0x20) != 0xFD && *(unsigned char *)(source + 0x20) >= 3) {
                func_L02_003DCDE8(source);
                func_L02_003FD5C0(*(void **)(self + 0xB8), *(void **)(state + 0x310));
            }
            func_L02_003D5E68(self);
        }
    }
    *(short *)(state + 0x322) = func_L02_003309B0(self, *(int *)(state + 0x314), 0, 0, 100.0f, 100.0f);
    *(int *)(state + 0x30C) = *(int *)(state + 0x210);
}

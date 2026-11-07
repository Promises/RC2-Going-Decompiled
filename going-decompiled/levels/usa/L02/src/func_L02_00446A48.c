/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * through the boot's assembler model (task #2579, RULING #13291 row L02-ASM).
 * Compiled ALONE as its own TU and linked at its one place, 0x00446A48, in
 * place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by tools/ee/asm_unit.sh (the row's `asm_unit` column; the plain-`as`
 * ablation is in the row's FACT, task #2579).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
extern void func_L02_0031F998();
extern void func_L02_0032C360(void *, int, void *, float, float, float, float, int);
extern void *func_L02_00322040(void *, int, int);
extern void func_L02_003157A0(u8 *);
typedef struct { u8 pad[0x464]; u8 spent[1]; } L02_CellFlags;
extern L02_CellFlags D_L02_0021F520;
extern u8 D_19B4A8[];
extern int g_playerProgress __attribute__((section(".sdata")));
extern char D_L02_001A8F20[] __attribute__((section(".sdata")));
extern void *func_L02_00315438(int);
extern void func_L02_003303C0(void *, u8 *);
extern int func_L02_00367DF8(int, int, void *);

/**
 * func_L02_00446A48 - a collectible with a 3-bit counter in the per-planet
 * table: show it while uncollected, count it when hit by its collector.
 *
 * @a0: the object (moby); +0x20 its state, +0xB2 its id.
 * Return: none.
 *
 * Non-obvious: the counter of id is a nibble of D_19B4A8 at byte
 * (id >> 1) + (g_playerProgress << 10): the high nibble for odd ids, the low
 * one for even ids, 3 bits used. In state 0, unless the id is marked spent in
 * D_L02_0021F520's flags, a zero counter sends it to (1, -1)
 * (func_L02_0031F998); otherwise it is deleted (func_L02_003157A0). In state 1
 * a hit (func_L02_00322040) from a class 0x9C8 source spawns moby 0xE43
 * (func_L02_00315438) bound to it (func_L02_003303C0), starts an effect
 * (func_L02_0032C360) and sound 0 (func_L02_00367DF8), and raises the counter,
 * saturating at 7. Then it deletes itself. Without such a hit it sets +0xA8 to
 * 0xFF.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/02_maktar_nebula/L02_0040A4A8.c, function func_L02_00445200
 * (v1.01 address), author commit ce9897f7 (bigmak94). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * body's statements unchanged. Respellings, each to the target of the v2.00
 * disc's own relocation at this place (relverify by name, task #2579):
 *   - func_L02_00445200 -> func_L02_00446A48 (our split name at the v2.00
 *     place);
 *   - D_L02_0021F4F0 -> D_L02_0021F520 (address-named, inside our split label
 *     D_L02_0020FFF0; RULING #13291 section 2);
 *   - D_0019B4A8 -> D_19B4A8 (the L02 split asm's existing name for that
 *     address, FACT #13369);
 *   - D_001A79F0 -> g_playerProgress (the boot's name at the disc-implied
 *     target 0x001A79F8);
 *   - func_002A7750 -> func_L02_0031F998 (the level's own copy at its L02
 *     place);
 *   - func_002B0A68 -> func_L02_0032C360 (the level's own copy at its L02
 *     place);
 *   - func_002A9640 -> func_L02_00322040 (the level's own copy at its L02
 *     place);
 *   - func_0029F338 -> func_L02_003157A0 (the level's own copy at its L02
 *     place);
 *   - func_0029EFD0 -> func_L02_00315438 (the level's own copy at its L02
 *     place);
 *   - func_002B37E8 -> func_L02_003303C0 (the level's own copy at its L02
 *     place);
 *   - func_002E5CF8 -> func_L02_00367DF8 (the level's own copy at its L02
 *     place);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     extern declarations of g_playerProgress (upstream D_001A79F0) and
 *     D_L02_001A8F20 (RULING #8620 addressing-model device, the audit's `resp`
 *     variant; ablation in the row's FACT);
 *   - of upstream's two identical-typed declarations of D_001A79F0 (`s32` at
 *     line 403, `int` at line 802) only the second is carried;
 *   - built at -G8 against upstream's -G0 (the audit's code G, NOTE #13281; a
 *     tsv setting);
 *   - `#include "common.h"` dropped (the file defines the typedefs it uses
 *     itself); upstream file-scope declarations it does not use are not
 *     carried; the rest keep upstream's order and text.
 */
void func_L02_00446A48(u8 *a0) {
    u8 *p;
    u8 *q;
    void *fx;
    int id;
    int id2;    /* state 1 reads the number again into a register of its own, as retail does */
    int v;

    switch (a0[0x20]) {
    case 0:
        id = *(u16 *)(a0 + 0xB2);
        if (D_L02_0021F520.spent[(s16)id] == 0) {
            v = D_19B4A8[((s16)id >> 1) + (g_playerProgress << 10)];
            if (id & 1)
                v = (v >> 4) & 7;
            else
                v = v & 7;
            if (v == 0) {
                func_L02_0031F998(a0, 1, -1);
                break;
            }
        }
        func_L02_003157A0(a0);
        break;
    case 1:
        p = func_L02_00322040(a0, 0x10000, 0);
        if (p != 0 && (q = *(u8 **)(p + 0x20)) != 0 && *(s16 *)(q + 0xAA) == 0x9C8) {
            fx = func_L02_00315438(0xE43);
            if (fx != 0)
                func_L02_003303C0(fx, a0);
            func_L02_0032C360(a0, 0x3FFF, D_L02_001A8F20, 3.0f, 6.0f, 3.0f, 6.0f, 0xFF);
            func_L02_00367DF8(0, 0, a0);
            id2 = *(u16 *)(a0 + 0xB2);
            if (id2 & 1) {
                if (((D_19B4A8[((s16)id2 >> 1) + (g_playerProgress << 10)] >> 4) & 7) != 7)
                    D_19B4A8[((s16)id2 >> 1) + (g_playerProgress << 10)] += 0x10;
            } else {
                if ((D_19B4A8[((s16)id2 >> 1) + (g_playerProgress << 10)] & 7) != 7)
                    D_19B4A8[((s16)id2 >> 1) + (g_playerProgress << 10)] += 1;
            }
            func_L02_003157A0(a0);
        } else {
            a0[0xA8] = 0xFF;
        }
        break;
    }
}

/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00442668, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 41 of 156
 * .text words differ from the disc, relocated fields masked, and .text is
 * 0x270 bytes, not the place's 0x274.
 */

/* RULING #8620 addressing device, EE arm only (level code has no native arm): a
 * section attribute on an extern DECLARATION moves no data and emits nothing.
 * Ablation (task #2578): with upstream's `sda`/`nosda` text instead, same
 * length, level .text 4 differing bytes, first at 0x0044275E. */
typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
extern u8 *D_L11_001FC4A0[];
typedef struct {
    s16 half[6];
    u8 pad0[8];
    u8 flag14;
    u8 flag15;
    u8 pad1[4];
} JobaClassRecordView;
typedef struct {
    s32 count;
    JobaClassRecordView *records;
} JobaClassTableView;
extern u32 D_L11_001B14D8 __attribute__((section(".sdata")));
extern s32 D_L11_001B3638 __attribute__((section(".sdata")));
extern u8 *D_L11_001B361C[3] __attribute__((section(".sdata")));
extern char g_heroPos[];
extern s32 func_L11_004443B0(u8 *);
extern s32 func_L11_00418968(u8 *);
extern float func_L11_003013F0(void *, void *);
#define GROUP(off) (*(s32 *)(state + (off)))

/**
 * func_L11_00442668 - fill the object's current target slot with an entity
 * from its group's id list.
 *
 * @obj: the object (moby); +0x68 its state block: +0xC4 the group, +0x120 the
 *       slot table, +0x18C the current slot, +0x1F0/+0x1F2 its class record.
 * Return: none.
 *
 * Non-obvious: An empty slot is taken as it is. A filled one is replaced only
 * when func_L11_00418968 refuses its entity and the slot index is still below
 * the class record's limit (byte +0x16) - 1; the index then advances. The slot
 * gets the first entity of the group's id list (D_L11_001FC4A0, entries
 * 256-byte records of D_L11_001B361C) that both func_L11_004443B0 and
 * func_L11_00418968 accept. If it is still empty after that walk, it takes the
 * first entity that func_L11_00418968 accepts and that lies more than 30.0
 * from g_heroPos (func_L11_003013F0). Upstream: the table address is formed at
 * each use, as retail does, which lets GCSE hoist it ahead of the first walk.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_0043E1A0-g8.c (blob 53f934ac), function
 * func_L11_00440E18 (v1.01 address), author commit d932dae8 (bigmak94).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED: the section respelling below; otherwise verbatim but for
 * names. Respellings, each to the target of the v2.00 disc's own relocation at
 * this place (relverify by name on this object, task #2578):
 *   - func_L11_00440E18 -> func_L11_00442668 (our split name at the v2.00
 *     place);
 *   - D_00189EA0 -> g_heroPos (the boot's name, same address);
 *   - D_L11_001B14F8 -> D_L11_001B14D8 (our label at that address);
 *   - D_L11_001B359C_abs -> D_L11_001B361C (address-named, inside
 *     D_L11_001B2E00; RULING #13291 section 2);
 *   - D_L11_001B35B8 -> D_L11_001B3638 (address-named, inside D_L11_001B2E00;
 *     RULING #13291 section 2);
 *   - D_L11_001FC420 -> D_L11_001FC4A0 (address-named, inside D_L11_001F0000;
 *     RULING #13291 section 2);
 *   - func_00282E80 -> func_L11_003013F0 (the level's own copy at its L11
 *     place);
 *   - func_L02_003D77B8 -> func_L11_00418968 (the level's own copy at its L11
 *     place);
 *   - func_L11_00442B60 -> func_L11_004443B0 (its v2.00 split name);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on 3
 *     extern declaration(s) (RULING #8620: upstream's patched cc1 reads sda,
 *     ours does not);
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_00442668(u8 *obj) {
    u8 *state = *(u8 **)(obj + 0x68);
    JobaClassTableView *entry = ((JobaClassTableView **)&D_L11_001B14D8)[*(s16 *)(state + 0x1F0)];
    JobaClassRecordView *rec = &entry->records[*(s16 *)(state + 0x1F2)];
    u8 *slot = *(u8 **)(state + (*(s32 *)(state + 0x18C) << 2) + 0x120);
    s32 ok = 0;
    u16 *q;
    u8 **slots;
    u16 *p;
    u8 *e;

    if (slot != 0) {
        if (func_L11_00418968(slot) == 0 && *(s32 *)(state + 0x18C) < *((u8 *)rec + 0x16) - 1) {
            ok = 1;
            *(s32 *)(state + 0x18C) = *(s32 *)(state + 0x18C) + 1;
        }
    } else {
        ok = 1;
    }
    if (ok) {
        if (GROUP(0xC4) == 0xFF)
            return;
        q = (u16 *)D_L11_001FC4A0[GROUP(0xC4)];
        if (q == 0)
            return;
        for (p = q; (GROUP(0xC4) & 0xFF) != 0xFF && GROUP(0xC4) <= D_L11_001B3638 && p != 0 &&
                    ((p != 0 && (s16)p[-1] >= 0) || p == (u16 *)D_L11_001FC4A0[GROUP(0xC4)]);
             p++) {
            e = D_L11_001B361C[0] + ((*p & 0x7FFF) << 8);
            if (e == 0)
                break;
            if (func_L11_004443B0(e)) {
                if (func_L11_00418968(e)) {
                    slots = (u8 **)(state + 0x120);
                    *(u8 **)((u8 *)slots + (GROUP(0x18C) << 2)) = e;
                    break;
                }
            }
        }
        slots = (u8 **)(state + 0x120);
        if (*(u8 **)((u8 *)slots + (GROUP(0x18C) << 2)) == 0) {
            for (p = (u16 *)D_L11_001FC4A0[GROUP(0xC4)];
                 (GROUP(0xC4) & 0xFF) != 0xFF && GROUP(0xC4) <= D_L11_001B3638 && p != 0 &&
                 ((p != 0 && (s16)p[-1] >= 0) || p == (u16 *)D_L11_001FC4A0[GROUP(0xC4)]);
                 p++) {
                e = D_L11_001B361C[0] + ((*p & 0x7FFF) << 8);
                if (e == 0)
                    break;
                if (func_L11_00418968(e)) {
                    if (func_L11_003013F0(e + 0x10, g_heroPos) > 30.0f) {
                        slots = (u8 **)(state + 0x120);
                        *(u8 **)((u8 *)slots + (GROUP(0x18C) << 2)) = e;
                        return;
                    }
                }
            }
        }
    }
}

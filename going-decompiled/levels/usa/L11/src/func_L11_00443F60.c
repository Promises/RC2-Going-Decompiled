/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00443F60, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 1 of 41
 * .text words differ from the disc, relocated fields masked.
 */

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef short s16;
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
extern u32 D_L11_001B14D8 __attribute__((sda));

/**
 * func_L11_00443F60 - test the state's six counters against the selected class
 * record.
 *
 * @obj: the object (moby); +0x68 its state block: +0x100 six counters, +0x118 and
 *       +0x11C two extra conditions, +0x140 a lock, +0x1F0/+0x1F2 the class.
 * Return: 1 when every counter equals its record halfword, each extra
 * condition the record flags (+0x14, +0x15) holds, and the lock is clear; else
 * 0.
 *
 * Non-obvious: The class table is the pointer array at D_L11_001B14D8, indexed
 * by +0x1F0; each entry is (count, records) and +0x1F2 picks the record.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_0043E1A0-g8.c (blob 53f934ac), function
 * func_L11_00442710 (v1.01 address), author commit daf0e4a1 (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device). Respellings, each to the target of the
 * v2.00 disc's own relocation at this place (relverify by name on this object,
 * task #2578):
 *   - func_L11_00442710 -> func_L11_00443F60 (our split name at the v2.00
 *     place);
 *   - D_L11_001B14F8 -> D_L11_001B14D8 (our label at that address);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
s32 func_L11_00443F60(u8 *obj) {
    u8 *state = *(u8 **)(obj + 0x68);
    s32 index = *(s16 *)(state + 0x1F0);
    s32 cls = *(s16 *)(state + 0x1F2);
    JobaClassTableView *entry = ((JobaClassTableView **)&D_L11_001B14D8)[index];
    JobaClassRecordView *rec = &entry->records[cls];
    u32 *counters = (u32 *)(state + 0x100);
    s32 same = 1;
    s32 i;
    s32 result;
    for (i = 0; i < 6; i++) {
        same &= (counters[i] == (u32)rec->half[i]);
    }
    if (rec->flag14 != 0) {
        same &= *(u32 *)(state + 0x118);
    }
    if (rec->flag15 != 0) {
        same &= *(u32 *)(state + 0x11C);
    }
    result = 0;
    if (*(u32 *)(state + 0x140) == 0) {
        if (same != 0) {
            result = 1;
        }
    }
    return result;
}

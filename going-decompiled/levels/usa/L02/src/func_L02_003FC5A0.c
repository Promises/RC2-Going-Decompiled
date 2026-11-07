/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x003FC5A0,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G8 -fopt-stack.
 *
 * func_L02_003FC5A0 - accumulate the active group entries and reset their
 * current report state: walks the 16-bit stream the object's table entry
 * points at, adds 1 and the record's +0x20 float to the state counters for
 * each record whose type word names one of the six counted kinds, then seeds
 * the state's float from the hero position and clears/stores two state
 * halfwords.
 *
 * @object: the entity object; its state is *(object + 0x68).
 * Return: nothing.
 *
 * Non-obvious: the stream word is consumed with `*cursor++ >= 0` (a negative
 * halfword ends the walk); the table base arrives through $gp (upstream spells
 * it with the sda attribute, which our stock cc1 warns "attribute directive
 * ignored" about — kept verbatim, measured byte-inert); the final
 * `do { ... } while (0)` writes D_L02_001B11D4 to state+0x11E.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_003FB000-g8.c, function func_L02_003FB000 (v1.01 address), author
 * commit b887d976 (Louis-Philippe Le Sieur). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082). BYTE-IDENTICAL: verbatim but
 * for names, plus:
 *   - `#include "common.h"` dropped; the u16/u32/u8 typedef lines are written
 *     out at the top instead;
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_003FB000 -> func_L02_003FC5A0,
 *     D_00189E20 -> g_soundBankHandlesBlk (the boot's name, same address),
 *     D_L02_001B1214 -> D_L02_001B11D4;
 *   - kept address-named (RULING #13291 §2; each target lies inside a larger
 *     split label, ours=D_L02_001B2D80+0x81C and D_L02_001F0000+0xC060):
 *     D_L02_001B359C and D_L02_001FC060 keep their spellings.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

extern char *D_L02_001B359C __attribute__((sda));
extern u32 D_L02_001FC060[];
extern u16 D_L02_001B11D4 __attribute__((sda));
extern char g_soundBankHandlesBlk[];

/* Accumulate the active group entries and reset their current report state. */
void func_L02_003FC5A0(char *object)
{
    char *state = *(char **)(object + 0x68);
    u32 index = *(u32 *)(state + 0x40);
    u32 entry;
    short *cursor;
    char *table;
    char *record;
    char *hero;

    *(int *)(state + 0x60) = 0;
    *(float *)(state + 0x64) = 0;
    if (index == 0xFF)
        return;
    entry = D_L02_001FC060[index];
    if (entry == 0)
        return;
    cursor = (short *)entry;
    table = D_L02_001B359C;
    do {
        short type;
        record = table + ((*(u16 *)cursor & 0x7FFF) << 8);
        if (record == 0)
            continue;
        if ((u8)record[0x20] == 0xFE)
            continue;
        if ((u8)record[0x20] == 0xFD)
            continue;
        if ((u8)record[0x20] == 1)
            continue;
        type = *(short *)(record + 0xAA);
        if (type == 0x9CA || type == 0xB22 || type == 0xC3B || type == 0xC3E || type == 0x9F8 || type == 0xA44) {
            *(int *)(state + 0x60) += 1;
            *(float *)(state + 0x64) += *(float *)(*(char **)(record + 0x68) + 0x20);
        }
    } while (*cursor++ >= 0);
    hero = g_soundBankHandlesBlk;
    *(float *)(state + 0x68) = (float)*(int *)(hero + 0x24CC);
    *(short *)(state + 0x11C) = 0;
    do {
        *(u16 *)(state + 0x11E) = D_L02_001B11D4;
    } while (0);
}

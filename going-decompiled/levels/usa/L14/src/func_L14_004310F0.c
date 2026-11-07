/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2581, RULING #13291 row L14-ASM). Compiled ALONE as its own TU and
 * linked at its one place, 0x004310F0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled by
 * tools/ee/asm_unit.sh (the row's `asm_unit` column). Measured with plain GNU
 * as instead (task #2581, same TU): .text is 0x288 bytes, not the place's
 * 0x294, and it emits a .lit4 pool of 4 bytes; level_elf.py cobj refuses it.
 */

/* RULING #8620 addressing device, EE arm only (level code has no native arm): a
 * section attribute on an extern DECLARATION moves no data and emits nothing.
 * Ablation (task #2581): with upstream's `sda` text on the two .sdata
 * declarations instead, same length, level .text 12 differing bytes, first at
 * 0x00431308.
 *
 * 0x001B251C is g_mobyTableBase, the name L14's boot-name map
 * (symbol_addrs/L14_boot_names.txt) gives at exactly that address. Upstream
 * declares it twice, as a pointer and, as D_L14_001B259C_1, as an array of
 * pointers (its comment: "the second read of the same pointer uses absolute
 * addressing"). The first read is g_mobyTableBase. The second keeps the
 * address-named D_001B251C, a second name for the same address that is
 * LOAD-BEARING: the assembler chooses $gp-relative or absolute addressing per
 * SYMBOL (from its `.extern` size), and retail reads this address both ways
 * in one function, so it needs two symbols. Measured (task #2646): binding
 * the array to g_mobyTableBase by asm label (RULING #13168's typed view)
 * leaves cc1 one `.extern g_mobyTableBase, 4`, the second read turns
 * $gp-relative, and .text is 0x290 bytes, not the place's 0x294 (cobj
 * refuses). It is the allowlisted site L14 func_L14_004310F0 D_001B251C in
 * tools/levels/second_names_allow.tsv (class A2, awaiting watcher-2).
 */
extern float func_L14_002F7A68(float, float);
extern float func_L14_002F7028(float);
extern float func_L14_002F6C18(void *, void *);
extern int func_L14_002F7BC0(float);
extern int func_L14_0036ADA0(int, int, void *);
extern void func_L14_00323250(void *, int, int);
extern void func_L14_0031A198(void *, int, void *);
extern void func_L14_0036AAA0(int);
extern void func_L14_00333698(void *, int);
extern void func_L14_0031AC78(int);
typedef struct {
    char pad[0x80];
    float pos[4];
    char gap[0x2AC];
    void *owner;
    int active;
} PrisonHero;
extern PrisonHero g_soundBankHandlesBlk;
extern unsigned char g_listenerPosHistory[];
extern unsigned char *g_mobyTableBase __attribute__((section(".sdata")));
extern unsigned char *D_001B251C[] __attribute__((section(".sdata")));
/* 0x1395E3: the boot's own dlabel D_1395E3, its own label in the boot split
 * (asm/usa/data/cod/033B00.data.s:7605, also the operand at
 * asm/usa/text/1E6ED0.s:7257); address-named (RULING #13291 section 2). */
extern unsigned char D_1395E3 __attribute__((section(".data")));

/**
 * func_L14_004310F0 - a pulsing pickup pad: glow, and when the hero stands on
 * it, claim it and switch its effects.
 *
 * @object: the object (moby); +0x68 its state block, +0x20 its stage byte, +0x78
 *          its colour.
 * Return: nothing; returns at once when the state block is null.
 *
 * Non-obvious: The phase (state +0x08) advances by 0.10471977
 * (func_L14_002F7A68); the glow value (func_L14_002F7BC0 of
 * (func_L14_002F7028(phase) * 4 - 3) * 128) is clamped to [32, 128]. Stage 0
 * sets flag 16 in +0x34, sets stage 1 and clears the sound entry (state +0x14).
 * Stage 1 keeps a sound entry in the 0x70-byte table at g_listenerPosHistory
 * (the boot's name at that address), and, when the record at
 * g_soundBankHandlesBlk (the boot's name at 0x00189E20; upstream types it as
 * the hero, PrisonHero) has this object as owner, is active and is within 0.5
 * (func_L14_002F6C18): sets stage 2, releases its entry, plays two sounds, sets
 * the byte D_1395E3 (the boot's label at 0x1395E3), calls func_L14_00333698 on
 * the two moby records of g_mobyTableBase named by state +0x00 and +0x04 when
 * +0x00 is not -1, and calls func_L14_0031AC78(state +0x10). Its colour word is
 * 0x80000000 | the glow in byte 0 (stage 1), or in bytes 1 and 2 (stage 2).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own, `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_0042FB10-effect-g8.c
 * (blob 86393936), function func_L14_0042FB10 (v1.01 address), author commit
 * 4912fc03 (bigbeno37). Adopted with the authors' permission (RULINGs #10405,
 * #12108, #13082, #13291). ADAPTED: the section respelling and the second
 * address spelling below; otherwise verbatim but for names. Edits, each name to
 * the target of the v2.00 disc's own relocation at this place (relverify by
 * name on this object, task #2581):
 *   - func_L14_0042FB10 -> func_L14_004310F0 (our split name at the v2.00
 *     place);
 *   - D_00188660 -> g_listenerPosHistory (the boot's name at that address);
 *   - D_00189E20 -> g_soundBankHandlesBlk (the boot's name at that address);
 *   - D_001395E3 -> D_1395E3: the same address, respelled to the boot's
 *     existing dlabel there (task #2621, census NOTE #13409, FACT #13403);
 *   - D_L14_001B259C -> g_mobyTableBase, the name symbol_addrs/
 *     L14_boot_names.txt gives at the v2.00 relocation target 0x001B251C;
 *   - D_L14_001B259C_1 -> D_001B251C (address-named, the same v2.00
 *     target; kept as a second symbol for the absolute read, see above);
 *   - D_L14_001B251C -> g_mobyTableBase (task #2646): the address-named
 *     spelling was a second name for the same address (A2 class, NOTE
 *     #13409);
 *   - func_00282E80 -> func_L14_002F6C18 (the level's own copy at its L14
 *     place);
 *   - func_00283198 -> func_L14_002F7028 (the level's own copy at its L14
 *     place);
 *   - func_00283B98 -> func_L14_002F7A68 (the level's own copy at its L14
 *     place);
 *   - func_00283CF0 -> func_L14_002F7BC0 (the level's own copy at its L14
 *     place);
 *   - func_0029FCC0 -> func_L14_0031A198 (the level's own copy at its L14
 *     place);
 *   - func_002A0430 -> func_L14_0031AC78 (the level's own copy at its L14
 *     place);
 *   - func_002A7750 -> func_L14_00323250 (the level's own copy at its L14
 *     place);
 *   - func_002E59F8 -> func_L14_0036AAA0 (the level's own copy at its L14
 *     place);
 *   - func_002E5CF8 -> func_L14_0036ADA0 (the level's own copy at its L14
 *     place);
 *   - func_L00_003294F0 -> func_L14_00333698 (the level's own copy at its L14
 *     place);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on 2
 *     extern declarations (RULING #8620: upstream's patched cc1 reads sda, ours
 *     ignores it);
 *   - upstream's `NOT_SDA` macro (its include/common.h) written out as its
 *     expansion `__attribute__((section(".data")))`;
 *   - `#include "common.h"` dropped (the body uses none of its typedefs);
 *     upstream file-scope declarations the body does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L14_004310F0(unsigned char *object)
{
    float pos[4];
    unsigned char *state = *(unsigned char **)(object + 0x68), *entry, *first;
    int value, id;
    float distance;

    if (!state)
        return;
    *(float *)(state + 8) = func_L14_002F7A68(*(float *)(state + 8), 0.10471977293491364f);
    value = func_L14_002F7BC0((func_L14_002F7028(*(float *)(state + 8)) * 4.0f - 3.0f) * 128.0f);
    if (value > 128)
        value = 128;
    else if (value < 32)
        value = 32;

    if (object[0x20] == 0) {
        *(unsigned short *)(object + 0x34) |= 16;
        func_L14_00323250(object, 1, -1);
        *(int *)(state + 0x14) = -1;
    } else if (object[0x20] == 1) {
        first = g_listenerPosHistory + *(int *)(state + 0x14) * 112;
        if (*(void **)(first + 0x88) != object || !first[0x74])
            *(int *)(state + 0x14) = func_L14_0036ADA0(1, 4, object);
        func_L14_0031A198(object, 0, pos);
        distance = func_L14_002F6C18(g_soundBankHandlesBlk.pos, pos);
        *(int *)(state + 0xC) = 0;
        if (g_soundBankHandlesBlk.owner == object && g_soundBankHandlesBlk.active && distance < 0.5f) {
            func_L14_00323250(object, 2, -1);
            id = *(int *)(state + 0x14);
            if (id != -1) {
                entry = g_listenerPosHistory + id * 112;
                if (*(void **)(entry + 0x88) == object && entry[0x74])
                    func_L14_0036AAA0(id);
            }
            *(int *)(state + 0x14) = -1;
            func_L14_0036ADA0(0, 0, object);
            func_L14_0036ADA0(2, 0, object);
            D_1395E3 = 1;
            if (*(int *)state != -1) {
                func_L14_00333698(g_mobyTableBase + (*(int *)state << 8), 1);
                {
                    int child = *(int *)(state + 4);
                    unsigned char *base = D_001B251C[0];
                    func_L14_00333698(base + (child << 8), 1);
                }
            }
            func_L14_0031AC78(*(int *)(state + 0x10));
        }
        *(unsigned int *)(object + 0x78) = value | (int)0x80000000;
    } else if (object[0x20] == 2) {
        if (!*(int *)(state + 0xC))
            *(int *)(state + 0xC) = 1;
        {
            unsigned int green = (value << 8) | 0x80000000;
            *(unsigned int *)(object + 0x78) = (value << 16) | green;
        }
    }
}

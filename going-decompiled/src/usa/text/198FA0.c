#include "common.h"

/* One entry of a save-section descriptor table. The serialized layout each
 * entry contributes is an 8-byte header followed by `len` payload bytes,
 * padded up to a 4-byte boundary. The table is terminated by an entry whose
 * srcPtr is NULL. (Stride 0x10; tag/_pad carry per-section metadata used by
 * the (de)serializers, not by the size calculation.) */
typedef struct SaveSection {
    void *srcPtr;
    s32   len;
    s32   tag;
    s32   _pad;
} SaveSection;

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299020);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299040);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299128);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299150);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299178);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002991E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299238);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002992B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002992E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299348);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299398);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002993D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299478);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002994B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299528);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299568);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002995E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299698);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002996C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002996E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299708);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299730);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299758);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002997C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002998D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299918);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299960);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299968);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299980);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", BuildSaveGamePaths);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299B00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299B18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299BF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SaveLoadStateMachine);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", BuildSaveImage);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", InitMemCardLib);

/* CalcSaveSectionsSize(table): return the number of bytes the section table
 * `table` serializes to. Layout is a leading 8-byte block, then for every
 * non-terminator entry an 8-byte header plus its (4-byte-aligned) payload, then
 * an 8-byte trailing terminator. Used to size the memory-card read/write
 * buffers for the two global save-section tables. */
s32 CalcSaveSectionsSize(SaveSection *table) {
    s32 size = 8;
    if (table->srcPtr != 0) {
        do {
            size += 8;
            size += table->len;
            table++;
            size = (size + 3) & -4;
        } while (table->srcPtr != 0);
    }
    return size + 8;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BCA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BD48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SerializeSaveSections);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BEA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", DeserializeSaveSections);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", CommitProgressCheckpoint);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C418);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C448);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C488);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C4C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C500);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", PostGuiScreenEvent);

/* func_0029C548(a,b): if g_guiInstance present, stores b then a into the pair
 * at g_guiInstance+0x379F4/+0x379F0. Logic recovered (frameless, 97.8%), but the
 * two same-base stores come out in the wrong order: ee-gcc's scheduler sorts the
 * store pair by ascending offset while the original emits them descending. Not
 * source-controllable -> store-scheduling wall, left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C548);

/* func_0029C570(): returns 1 if either of the pair at g_guiInstance+0x379F0/+4
 * is non-zero (else 0). Logic recovered (99.4%), but the original short-circuit
 * `a || b` was compiled with a branch-likely (beqzl, second load in the delay
 * slot) which this cc1 build does not emit here -> branch-likely wall, left asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C570);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C5B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C600);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C638);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C648);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C678);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C700);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C818);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C8F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CA88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CA98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CC48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CCB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CD18);

/* func_0029CF08: empty jr-ra stub - installed by GuiManagerCreate as a default no-op GUI callback. */
void func_0029CF08(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CF10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CF40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CF70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CFA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CFE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D010);

/* func_0029D040(a): `g_guiInstance ? func_00347B88(g_guiInstance+0x39620, a) : 0`.
 * Representative of the large g_guiInstance forwarding-wrapper family in this unit
 * (func_0029CF10..func_0029DB58). Logic recovered, but every member differs only
 * in prologue scheduling: the original emits `addiu $sp` first then the %hi load,
 * whereas this cc1 front-loads the %hi and slots the arg-shuffle/$ra-save into the
 * branch delay differently. Systematic, not source-controllable -> left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D040);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D080);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D0C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D108);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D138);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D178);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D1A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D1D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D218);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D248);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D288);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D2B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D2F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D328);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D368);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D398);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D3D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D408);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D448);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D478);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D4B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D4E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D528);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D568);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D598);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D5D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D608);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D648);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D678);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D6B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D6E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D728);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D758);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D798);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D7C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D808);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D838);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D878);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D8A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D8E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D918);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D958);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D988);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", TickBoltCounterHud);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D9B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", FlushHudDisplayValue);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DA18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DA58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DA88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DAD0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DB10);

/* func_0029DB50: empty jr-ra stub - installed by GuiManagerCreate as a default no-op GUI callback. */
void func_0029DB50(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DB58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", GuiManagerCreate);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DC70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DCB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DDB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DDE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", EnableDmacChannels);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DF18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E090);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E1D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", DecompressWad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E5D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E5F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", UpdateLevelObjectiveStates);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", EvaluateProgressCondition);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", GatherActiveObjectives);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EA90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EAC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EBF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EC70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029ECE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029FDF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SpawnMoby);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", InitMobyFromClass);

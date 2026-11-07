/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * through the boot's assembler model (task #2579, RULING #13291 row L02-ASM).
 * Compiled ALONE as its own TU and linked at its one place, 0x003FEE58, in
 * place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by tools/ee/asm_unit.sh (the row's `asm_unit` column; the plain-`as`
 * ablation is in the row's FACT, task #2579).
 */

typedef unsigned char      u8;
extern int func_L02_00332360(void);
extern int func_L02_002F9128(void);
extern int func_L02_003FB110(unsigned int);
extern int func_L02_002C7A60(int, int);
extern int func_L02_00367F58(int, int, void *);
extern int func_L02_002F29C8(short *);
extern void func_L02_0031F998(u8 *, int, int);
extern void func_L02_004544E8(int);
extern void func_L02_002CC330(int);
extern int func_L02_00332158(int, int, int, int, u8 *);
extern void func_L02_003FE020(u8 *);
extern void func_L02_00336D40(void);
extern int func_L02_00336D58(unsigned int, int, int *);
extern void func_L02_002EAC28(void (*)(void *), void *);
extern void func_L02_003FF808(void *);
extern int g_heroState[3], D_L02_002004A4[3];

typedef struct {
    u8 pad[0x1a0];
    int field1a0, field1a4;
    u8 gap[24];
    int field1c0, field1c4;
} InputView;
extern unsigned char D_138180[];
extern unsigned char g_itemEquippedSlot[], D_1395B8[];
typedef struct {
    u8 pad[4];
    u8 enabled;
    u8 rest[0xdb];
} MenuRecord;
extern unsigned char D_L02_00264A70[];
#define MENU ((MenuRecord *)D_L02_00264A70)
/* The class pointer table is addressed through gp. */
extern int D_L02_001B1150 __attribute__((sda));
#define TABLE ((int **)&D_L02_001B1150)
#define H(o) (*(short *)(state+(o)))

/**
 * func_L02_003FEE58 - per-frame input handler of the level's selection menu:
 * move the highlighted row, confirm, cancel or idle out.
 *
 * @object: the menu object (moby); +0x68 its state block: +0x118 the row,
 *          +0x11A a repeat delay, +0x120/+0x122 the chosen entry, +0x126/+0x128
 *          the arrow flashes.
 * Return: none.
 *
 * Non-obvious: it does nothing in game state 5 (func_L02_00332360). With no
 * repeat delay pending, pad bit 0x2000 moves to the next row and 0x8000 to the
 * previous one. Only rows whose unlock flag is set count; the flag is D_1395B8
 * at func_L02_003FB110 of the row's id. Rows 12 and 13 also need
 * func_L02_002F9128. Row 7 needs the menu record (D_L02_00264A70, 0xE0 bytes
 * each) of equipped slot 0x1E enabled, and row 8 that of slot 0x2A. Bit 0x40
 * confirms the row: it stores the choice and D_1395B8[0x3C], sets animation 8
 * and starts the transition. Bit 0x80 requests state 5 (func_L02_00332158).
 * Bit 0x10 goes back (func_L02_003FE020). An idle timeout on all four checks
 * of func_L02_00336D58 sets animation 5. Otherwise the next handler
 * func_L02_003FF808 is installed (func_L02_002EAC28). It is the L02 sibling of
 * L11's func_L11_00444CE8 (same upstream shape, different rows and offsets).
 * D_138180 is the pad block (InputView), named by address because no boot
 * symbol starts there. H() is upstream's state-halfword macro.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/02_maktar_nebula/L02_003FD8B8-g8.c, function
 * func_L02_003FD8B8 (v1.01 address), author commit 9bbea36d (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: verbatim but for names (no device). Respellings, each to the
 * target of the v2.00 disc's own relocation at this place (relverify by name,
 * task #2579):
 *   - func_L02_003FD8B8 -> func_L02_003FEE58 (our split name at the v2.00
 *     place);
 *   - D_00138180 -> D_138180 (the L02 split asm's existing name for that
 *     address, FACT #13369);
 *   - D_00139568 -> g_itemEquippedSlot (the boot's name, same address);
 *   - D_001395B8 -> D_1395B8 (the L02 split asm's existing name for that
 *     address, FACT #13369);
 *   - D_0018C0B4 -> g_heroState (the boot's name, same address);
 *   - D_L02_001B1190 -> D_L02_001B1150 (our label at that address);
 *   - func_00282978 -> func_L02_002F29C8 (the level's own copy at its L02
 *     place);
 *   - func_00288D58 -> func_L02_002F9128 (the level's own copy at its L02
 *     place);
 *   - func_002A7750 -> func_L02_0031F998 (the level's own copy at its L02
 *     place);
 *   - func_002B49E0 -> func_L02_00332158 (the level's own copy at its L02
 *     place);
 *   - func_002B4BE8 -> func_L02_00332360 (the level's own copy at its L02
 *     place);
 *   - func_002B8518 -> func_L02_00336D40 (the level's own copy at its L02
 *     place);
 *   - func_002E5E58 -> func_L02_00367F58 (the level's own copy at its L02
 *     place);
 *   - func_003355A8 -> func_L02_004544E8 (the level's own copy at its L02
 *     place);
 *   - func_L00_002C42E0 -> func_L02_002C7A60 (the level's own copy at its L02
 *     place);
 *   - func_L00_002C8BA8 -> func_L02_002CC330 (the level's own copy at its L02
 *     place);
 *   - func_L00_002E6CF0 -> func_L02_002EAC28 (the level's own copy at its L02
 *     place);
 *   - func_L00_0032EF10 -> func_L02_00336D58 (the level's own copy at its L02
 *     place);
 *   - func_L02_003F9B70 -> func_L02_003FB110 (its v2.00 split name);
 *   - func_L02_003FCA80 -> func_L02_003FE020 (its v2.00 split name);
 *   - func_L02_003FE250 -> func_L02_003FF808 (its v2.00 split name);
 *   - D_L02_002004A4 kept: an address-named extern inside our split label
 *     D_L02_00200010 (RULING #13291 section 2), valued from its name by
 *     level_elf;
 *   - upstream's `__attribute__((sda))` on D_L02_001B1150 kept as its text
 *     (this cc1 ignores it with a warning; the member closes with it as
 *     written);
 *   - `#include "common.h"` dropped: the u8 typedef this body uses is copied
 *     from it; upstream file-scope declarations are all carried in upstream's
 *     order and text.
 */
void func_L02_003FEE58(u8 *object)
{
    u8 *state = *(u8 **)(object + 0x68);
    int i, found;
    InputView *input;
    if (func_L02_00332360()==5) return;
    if (g_heroState[0]!=0x6f) func_L02_002C7A60(0x6f,1);
    if (H(0x11a)==0) {
        if ((((InputView *)D_138180)->field1c4&0x2000) && H(0x118)<13) {
            for (i=H(0x118)+1;i<14;i++) {
                if (!D_1395B8[func_L02_003FB110(TABLE[i][2])]) continue;
                if (i>=12 && !func_L02_002F9128()) continue;
                if (i==7) {
                    MenuRecord *choice = (MenuRecord *)(D_L02_00264A70 + g_itemEquippedSlot[0x1e] * 0xe0);
                    if (!choice->enabled) continue;
                }
                if (i==8) {
                    MenuRecord *choice = (MenuRecord *)(D_L02_00264A70 + g_itemEquippedSlot[0x2a] * 0xe0);
                    if (!choice->enabled) continue;
                }
                H(0x118)=i;
                H(0x11a)=15;
                H(0x128)=15;
                func_L02_00367F58(3,0,0);
                break;
            }
        } else if ((((InputView *)D_138180)->field1c4&0x8000) && H(0x118)>0) {
            found=0;
            if (H(0x118)==1) {
                H(0x118)=0;
                found=1;
                func_L02_00367F58(3,0,0);
            } else for (i=H(0x118)-1;i>=0;i--) {
                if (!D_1395B8[func_L02_003FB110(TABLE[i][2])]) continue;
                if (i>=12 && !func_L02_002F9128()) continue;
                if (i==7) {
                    MenuRecord *choice = (MenuRecord *)(D_L02_00264A70 + g_itemEquippedSlot[0x1e] * 0xe0);
                    if (!choice->enabled) continue;
                }
                if (i==8) {
                    MenuRecord *choice = (MenuRecord *)(D_L02_00264A70 + g_itemEquippedSlot[0x2a] * 0xe0);
                    if (!choice->enabled) continue;
                }
                H(0x118)=i;
                found=1;
                break;
            }
            if (!found) H(0x118)=0;
            H(0x11a)=15;
            H(0x126)=15;
            func_L02_00367F58(3,0,0);
        }
    } else func_L02_002F29C8((short *)(state+0x11a));
    input = (InputView *)D_138180;
    if (input->field1c4&0x40) {
        if (H(0x118)==0) {
            H(0x120)=*(unsigned short *)(state+0x118);
            H(0x122)=0;
            D_1395B8[0x3c]=state[0x118];
            func_L02_0031F998(object,8,-1);
            func_L02_004544E8(2);
            func_L02_002CC330(0);
            func_L02_00367F58(4,0,0);
            input->field1c4=0;
            goto clear_buttons;
        } else {
            if (!D_1395B8[func_L02_003FB110(TABLE[H(0x118)][2])]) return;
            if (H(0x118)>=12 && !func_L02_002F9128()) return;
            if (H(0x118)==7 && !MENU[g_itemEquippedSlot[0x1e]].enabled) return;
            if (H(0x118)==8 && !MENU[g_itemEquippedSlot[0x2a]].enabled) return;
        }
        H(0x120)=*(unsigned short *)(state+0x118);
        H(0x122)=0;
        D_1395B8[0x3c]=state[0x118];
        func_L02_0031F998(object,8,-1);
        func_L02_004544E8(2);
        func_L02_002CC330(0);
        func_L02_00367F58(4,0,0);
        input->field1c4=0;
        goto clear_buttons;
    } else if (input->field1c4&0x80) {
        func_L02_00332158(5,1,0,0,0);
        return;
    } else if (input->field1c4&0x10) {
        func_L02_003FE020(object);
        input->field1c4=0;
clear_buttons:
        input->field1a0=0;
        input->field1c0=0;
        input->field1a4=0;
        func_L02_00336D40();
        return;
    } else if (D_L02_002004A4[0] &&
               func_L02_00336D58(1,60,0) && func_L02_00336D58(2,60,0) &&
               func_L02_00336D58(4,60,0) && func_L02_00336D58(8,60,0)) {
        H(0x120)=*(unsigned short *)(state+0x118);
        func_L02_0031F998(object,5,-1);
        return;
    }
    func_L02_002EAC28(func_L02_003FF808,object);
}

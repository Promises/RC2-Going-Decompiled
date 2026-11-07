/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00444CE8, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 3 of 297
 * .text words differ from the disc, relocated fields masked.
 */

typedef unsigned char u8;

extern int func_L11_0033F288(void);
extern int func_L11_003075E8(void);
extern int func_L11_0043F948(unsigned int);
extern int func_L11_002D9208(int, int);
extern int func_L11_00375010(int, int, void *);
extern int func_L11_00300E80(short *);
extern void func_L11_0032DBF8(u8 *, int, int);
extern void func_L11_00489F28(int);
extern void func_L11_002DCCC8(int);
extern int func_L11_0033F080(int, int, int, int, u8 *);
extern void func_L11_00443E18(u8 *);
extern void func_L11_00344020(void);
extern int func_L11_00344038(unsigned int, int, int *);
extern void func_L11_004455E0(void *);
extern int g_heroState[3], D_L11_002008E4[3];
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
extern unsigned char D_L11_00279C20[];
#define MENU ((MenuRecord *)D_L11_00279C20)
extern int D_L11_001B14D8 __attribute__((sda));
#define TABLE ((int **)&D_L11_001B14D8)
#define H(o) (*(short *)(state+(o)))

/**
 * func_L11_00444CE8 - per-frame input handler of the level's selection menu:
 * move the highlighted row, confirm, cancel or idle out.
 *
 * @object: the menu object (moby); +0x68 its state block: +0x1E8 the row, +0x1EA
 *          a repeat delay, +0x1F0/+0x1F2 the chosen class, +0x1F6/+0x1F8 the
 *          arrow flashes.
 * Return: none.
 *
 * Non-obvious: It does nothing in game state 5 (func_L11_0033F288). With no
 * repeat delay pending, pad bit 0x2000 moves to the next row and 0x8000 to the
 * previous one. Only rows whose unlock flag (D_1395B8 by func_L11_0043F948
 * of the row's id) is set count. Rows 11 and 12 also need func_L11_003075E8,
 * and row 8 the equipped item's menu record (D_L11_00279C20, 0xE0 bytes each)
 * enabled. Bit 0x40 confirms the row: it stores the choice, sets animation 8
 * and starts the transition. Bit 0x80 requests state 5 (func_L11_0033F080).
 * Bit 0x10 goes back (func_L11_00443E18). An idle timeout on all four checks
 * of func_L11_00344038 sets animation 5. Otherwise the next handler
 * func_L11_004455E0 is installed (func_L11_002F9490). D_138180 is the pad
 * block (InputView), address-named with the spelling the boot asm already
 * links for 0x138180 (a %hi/%lo operand in 48 boot asm files, e.g.
 * asm/usa/nonmatchings/text/188858/func_0028C840.s:80, and in the L11 split
 * asm/text/002BCF00.s; boot C declares it too, src/usa/text/188858.c:5254).
 * H() is upstream's state-halfword macro.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00443498-g8.c (blob 17e07e59), function
 * func_L11_00443498 (v1.01 address), author commit 3cd8af2d (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2578):
 *   - func_L11_00443498 -> func_L11_00444CE8 (our split name at the v2.00
 *     place);
 *   - D_00139568 -> g_itemEquippedSlot (the boot's name, same address);
 *   - D_001395B8 -> D_1395B8: the same address, respelled to the boot's and
 *     the L11 split asm's existing name for it (task #2603, FACT #13369);
 *   - D_00138180 -> D_138180: the same address, respelled to the boot's and
 *     the L11 split asm's existing operand name for it (task #2621, census
 *     NOTE #13409);
 *   - D_0018C0B4 -> g_heroState (the boot's name, same address);
 *   - D_L11_001B14F8 -> D_L11_001B14D8 (our label at that address);
 *   - D_L11_00200864 -> D_L11_002008E4 (address-named, inside D_L11_00200010;
 *     RULING #13291 section 2);
 *   - D_L11_00279BA0 -> D_L11_00279C20 (our label at that address);
 *   - func_00282978 -> func_L11_00300E80 (the level's own copy at its L11
 *     place);
 *   - func_00288D58 -> func_L11_003075E8 (the level's own copy at its L11
 *     place);
 *   - func_002A7750 -> func_L11_0032DBF8 (the level's own copy at its L11
 *     place);
 *   - func_002B49E0 -> func_L11_0033F080 (the level's own copy at its L11
 *     place);
 *   - func_002B4BE8 -> func_L11_0033F288 (the level's own copy at its L11
 *     place);
 *   - func_002B8518 -> func_L11_00344020 (the level's own copy at its L11
 *     place);
 *   - func_002E5E58 -> func_L11_00375010 (the level's own copy at its L11
 *     place);
 *   - func_003355A8 -> func_L11_00489F28 (the level's own copy at its L11
 *     place);
 *   - func_L00_002C8BA8 -> func_L11_002DCCC8 (the level's own copy at its L11
 *     place);
 *   - func_L00_002E6CF0 -> func_L11_002F9490 (the level's own copy at its L11
 *     place);
 *   - func_L00_0032EF10 -> func_L11_00344038 (the level's own copy at its L11
 *     place);
 *   - func_L11_002D8EF8 -> func_L11_002D9208 (its v2.00 split name);
 *   - func_L11_0043E108 -> func_L11_0043F948 (its v2.00 split name);
 *   - func_L11_004425C8 -> func_L11_00443E18 (its v2.00 split name);
 *   - func_L11_00443D78 -> func_L11_004455E0 (its v2.00 split name);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_00444CE8(u8 *object)
{
    u8 *state = *(u8 **)(object + 0x68);
    int i, found;
    InputView *input;
    if (func_L11_0033F288()==5) return;
    if (g_heroState[0]!=0x6f) func_L11_002D9208(0x6f,1);
    if (H(0x1ea)==0) {
        if ((((InputView *)D_138180)->field1c4&0x2000) && H(0x1e8)<12) {
            for (i=H(0x1e8)+1;i<13;i++) {
                if (!D_1395B8[func_L11_0043F948(TABLE[i][2])]) continue;
                if (i>=11 && !func_L11_003075E8()) continue;
                if (i==8) {
                    MenuRecord *choice = (MenuRecord *)(D_L11_00279C20 + g_itemEquippedSlot[0x1e] * 0xe0);
                    if (!choice->enabled) continue;
                }
                H(0x1e8)=i;
                H(0x1ea)=15;
                H(0x1f8)=15;
                func_L11_00375010(3,0,0);
                break;
            }
        } else if ((((InputView *)D_138180)->field1c4&0x8000) && H(0x1e8)>0) {
            found=0;
            if (H(0x1e8)==1) {
                H(0x1e8)=0;
                found=1;
            } else for (i=H(0x1e8)-1;i>=0;i--) {
                if (!D_1395B8[func_L11_0043F948(TABLE[i][2])]) continue;
                if (i>=11 && !func_L11_003075E8()) continue;
                if (i==8) {
                    MenuRecord *choice = (MenuRecord *)(D_L11_00279C20 + g_itemEquippedSlot[0x1e] * 0xe0);
                    if (!choice->enabled) continue;
                }
                H(0x1e8)=i;
                found=1;
                break;
            }
            if (!found) H(0x1e8)=0;
            H(0x1ea)=15;
            H(0x1f6)=15;
            func_L11_00375010(3,0,0);
        }
    } else func_L11_00300E80((short *)(state+0x1ea));
    input = (InputView *)D_138180;
    if (input->field1c4&0x40) {
        if (H(0x1e8)==0) {
            H(0x1f0)=*(unsigned short *)(state+0x1e8);
            H(0x1f2)=0;
            D_1395B8[0x49]=state[0x1e8];
            func_L11_0032DBF8(object,8,-1);
            func_L11_00489F28(2);
            func_L11_002DCCC8(0);
            func_L11_00375010(4,0,0);
            input->field1c4=0;
            goto clear_buttons;
        } else {
            if (!D_1395B8[func_L11_0043F948(TABLE[H(0x1e8)][2])]) return;
            if (H(0x1e8)>=11 && !func_L11_003075E8()) return;
            if (H(0x1e8)==8 && !MENU[g_itemEquippedSlot[0x1e]].enabled) return;
        }
        H(0x1f0)=*(unsigned short *)(state+0x1e8);
        H(0x1f2)=0;
        D_1395B8[0x49]=state[0x1e8];
        func_L11_0032DBF8(object,8,-1);
        func_L11_00489F28(2);
        func_L11_002DCCC8(0);
        func_L11_00375010(4,0,0);
        input->field1c4=0;
        goto clear_buttons;
    } else if (input->field1c4&0x80) {
        func_L11_0033F080(5,1,0,0,0);
        return;
    } else if (input->field1c4&0x10) {
        func_L11_00443E18(object);
        input->field1c4=0;
clear_buttons:
        input->field1a0=0;
        input->field1c0=0;
        input->field1a4=0;
        func_L11_00344020();
        return;
    } else if (D_L11_002008E4[0] &&
               func_L11_00344038(1,60,0) && func_L11_00344038(2,60,0) &&
               func_L11_00344038(4,60,0) && func_L11_00344038(8,60,0)) {
        H(0x1f0)=*(unsigned short *)(state+0x1e8);
        func_L11_0032DBF8(object,5,-1);
        return;
    }
    func_L11_002F9490(func_L11_004455E0,object);
}

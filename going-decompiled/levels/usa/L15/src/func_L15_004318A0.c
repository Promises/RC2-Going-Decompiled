/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x004318A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack (upstream builds its file at -G8), plain as.
 */

typedef struct StageSetup {
    int initial;
    char pad04[0x10];
    int count;
    char pad18[0x10];
    int limit;
    int timer;
    float scale;
} StageSetup;
typedef struct StageRecord {
    int words[10];
} StageRecord;
extern int D_L15_001B14E0 __attribute__((sda));
extern int D_L15_001B14E4 __attribute__((sda));
extern int D_L15_001B1574 __attribute__((sda));
extern StageSetup D_L15_00221DC0;
extern StageRecord D_L15_0029FD60[];
extern int func_L15_00382588(int);
extern void func_L15_003819E8(int);
extern void func_L15_00385230(void);
extern void func_L15_00381A28(void);

/**
 * func_L15_004318A0 - reset the level's stage counters and restore the
 * selected record's value.
 *
 * Return: nothing.
 *
 * Sets D_L15_001B14E0 to -2, D_L15_001B1574 to 1 and D_L15_001B14E4 to 0;
 * in the stage block D_L15_00221DC0 the timer (+0x2C) becomes 1, the limit
 * (+0x28) 100, the count (+0x14) 0 and the scale (+0x30) 8.0. The record
 * func_L15_00382588(6) selects in D_L15_0029FD60 (0x28 bytes each) gets its
 * word 5 set from its word 4. It then calls func_L15_003819E8(0),
 * func_L15_00385230() and func_L15_00381A28().
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_0042FD40-g8.c, function func_L15_0042FD40 (v1.01 address), author commit b7357856 (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_0042FD40 -> func_L15_004318A0 (our split name at the v2.00 place);
 *   - D_L15_001B1530 -> D_L15_001B14E0 (our split label at the v2.00 address);
 *   - D_L15_001B1534 -> D_L15_001B14E4 (our split label at the v2.00 address);
 *   - D_L15_001B15C4 -> D_L15_001B1574 (our split label at the v2.00 address);
 *   - D_L15_00221E10 -> D_L15_00221DC0: an address-named extern (RULING #13291 section 2) at
 *   0x00221DC0, inside the split label D_L15_00220000 (+0x1DC0); no name exists at that exact address in
 *   the boot splits (labels or operands), symbol_addrs.txt, L15_boot_names.txt
 *   or the level's split labels (grep with firing controls, task #2677);
 *   - D_L15_0029FD80 -> D_L15_0029FD60 (our split label at the v2.00 address; the boot-name map's
 *   D_262920 at the same address is not used, an exact split label wins);
 *   - func_002ED8A0 -> func_L15_00382588 (the v2.00 relocation target, our name there);
 *   - func_L05_00379F28 -> func_L15_003819E8 (the v2.00 relocation target, our name there);
 *   - func_L05_00379F68 -> func_L15_00381A28 (the v2.00 relocation target, our name there);
 *   - func_L05_0037D6E8 -> func_L15_00385230 (the v2.00 relocation target, our name there);
 *   - upstream's `__attribute__((sda))` on D_L15_001B14E0, D_L15_001B14E4 and D_L15_001B1574 kept verbatim:
 *   our cc1 ignores it with a warning and, at -G8, places these 4-byte
 *   objects $gp-relative as retail has them, so it is byte-inert here;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L15_004318A0(void)
{
    StageSetup *stage = &D_L15_00221DC0;
    int n;

    D_L15_001B14E0 = -2;
    D_L15_001B1574 = 1;
    stage->timer = 1;
    stage->limit = 100;
    D_L15_001B14E4 = 0;
    stage->count = 0;
    n = func_L15_00382588(6);
    stage->scale = 8.0f;
    D_L15_0029FD60[n].words[5] = D_L15_0029FD60[n].words[4];
    func_L15_003819E8(0);
    func_L15_00385230();
    func_L15_00381A28();
}

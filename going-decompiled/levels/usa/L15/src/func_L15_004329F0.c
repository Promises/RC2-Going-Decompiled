/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x004329F0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack (upstream builds its file at -G8), plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

typedef struct {
    u8 pad00[0xAA];
    s16 uid;
} Moby430C80;
extern s32 D_L15_001B14E4 __attribute__((section(".sdata")));
extern s32 D_L15_00221DC0[];
extern u8 *D_L15_001B2694[] __attribute__((section(".sdata")));
extern s32 g_gameTime[] __attribute__((section(".sdata")));
extern void func_L15_00347748(void);
extern s32 func_L15_00381988(void);
extern Moby430C80 **func_L15_00381978(void);
extern int func_L15_00381990(int arg);
extern void func_L15_003819D8(int);
extern void func_L15_00382CA8(void);
extern void func_L15_00382D18(void *object, int handler);
extern void *func_L15_00383038(int, int);
extern void func_L15_00384A28(void *, void *, f32);
extern void func_L15_00385150(void *, void *, f32);
extern int func_L15_00382F68(int);
extern void func_L15_00432238(void *, int);
extern void func_L15_00437C40(void *);
extern void func_L15_00437E50(void *);
extern void func_L15_0032A2A0(void *);
typedef struct {
    u8 pad00[0x34];
    u16 flags;
} Marker;
extern Marker *D_L15_001B1534 __attribute__((section(".sdata")));
extern Marker *D_L15_001B1538 __attribute__((section(".sdata")));

/**
 * func_L15_004329F0 - the level's one-time start-up step, then the check
 * that ends the stage when no group 0x1211 member is left.
 *
 * @self: the calling object.
 * Return: nothing.
 *
 * On the first call (D_L15_001B14E4 == 0) it sets that flag, clears the
 * stage timer (word +0x2C of D_L15_00221DC0), runs three set-up calls, and for every
 * listed moby of class 0x957 sets handler 1 and points it at the stage's
 * current path entry (D_L15_001B2694[0] + stage * 128 + 0x30, radius 64.0).
 * When func_L15_00383038(0x1211, 1) returns an object it is started with
 * func_L15_00437C40 and func_L15_00437E50; the markers D_L15_001B1534 and
 * D_L15_001B1538, when set and with a non-zero flags halfword +0x34, are
 * passed to func_L15_0032A2A0 and cleared. Every call, when func_L15_00382F68
 * counts no live member in the group func_L15_00381990(0x1211) names and
 * g_gameTime[0] is at least 6, it calls func_L15_00432238(self, 2).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/us2/levels/15_gorn/L15_004329F0-g8.c, function func_US2_L15_004329F0 (v2.00 address), author commit 5e9cfe7c (bigbeno37). Its
 * body, but for the two Marker blocks, is Flavius's func_L15_00430C80 in
 * src/levels/shared/L15_00430C80-g8.c (author commit a0ffc4d1, earlier;
 * bigbeno37 carried it to PAL in 4d81cc78 and then here), so Flavius is
 * credited for that text and bigbeno37 for the Marker type and the two
 * D_L15_001B1534 / D_L15_001B1538 blocks (RULING #13427 rev 2, #13541 rev 2).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_US2_L15_004329F0 -> func_L15_004329F0 (our split name at the v2.00 place);
 *   - D_L15_001B1534 -> D_L15_001B14E4 (our split label at the v2.00 address);
 *   - D_L15_001B1584 -> D_L15_001B1534 (our split label at the v2.00 address);
 *   - D_L15_001B1588 -> D_L15_001B1538 (our split label at the v2.00 address);
 *   - D_L15_001B2148 -> g_gameTime: the name symbol_addrs/L15_boot_names.txt gives at
 *   exactly 0x001B20C8, where the split has no label of its own (NAMES rule,
 *   task #2646 class A2);
 *   - D_L15_001B2714 -> D_L15_001B2694: an address-named extern (RULING #13291 section 2) at
 *   0x001B2694, inside the split label D_L15_001B1D80 (+0x914); no name exists at that exact address in
 *   the boot splits (labels or operands), symbol_addrs.txt, L15_boot_names.txt
 *   or the level's split labels (grep with firing controls, task #2677);
 *   - D_L15_00221E10 -> D_L15_00221DC0: an address-named extern (RULING #13291 section 2) at
 *   0x00221DC0, inside the split label D_L15_00220000 (+0x1DC0); no name exists at that exact address in
 *   the boot splits (labels or operands), symbol_addrs.txt, L15_boot_names.txt
 *   or the level's split labels (grep with firing controls, task #2677);
 *   - func_0029F338 -> func_L15_0032A2A0 (the v2.00 relocation target, our name there);
 *   - func_002B7320 -> func_L15_00347748 (the v2.00 relocation target, our name there);
 *   - func_L00_004272A0 -> func_L15_00381988 (the v2.00 relocation target, our name there);
 *   - func_L05_00379EB8 -> func_L15_00381978 (the v2.00 relocation target, our name there);
 *   - func_L05_00379ED0 -> func_L15_00381990 (the v2.00 relocation target, our name there);
 *   - func_L05_00379F18 -> func_L15_003819D8 (the v2.00 relocation target, our name there);
 *   - func_L05_0037B210 -> func_L15_00382CA8 (the v2.00 relocation target, our name there);
 *   - func_L05_0037B280 -> func_L15_00382D18 (the v2.00 relocation target, our name there);
 *   - func_L05_0037B440 -> func_L15_00383038 (the v2.00 relocation target, our name there);
 *   - func_L05_0037CEE0 -> func_L15_00384A28 (the v2.00 relocation target, our name there);
 *   - func_L05_0037D608 -> func_L15_00385150 (the v2.00 relocation target, our name there);
 *   - func_L15_00381CF0 -> func_L15_00382F68 (the v2.00 relocation target, our name there);
 *   - func_L15_00435E80 -> func_L15_00437C40 (the v2.00 relocation target, our name there);
 *   - func_L15_00436090 -> func_L15_00437E50 (the v2.00 relocation target, our name there);
 *   - func_US2_L15_00432238 -> func_L15_00432238 (the v2.00 relocation target, our name there);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on D_L15_001B14E4, D_L15_001B2694, g_gameTime, D_L15_001B1534
 *   and D_L15_001B1538 (RULING #8620
 *   ADDRESSING-MODEL DEVICE, EE arm only; this TU has no other arm): our cc1
 *   does not know `sda`/`nosda` (it warns "directive ignored"); with `.sdata` at -G8 the
 *   access is $gp-relative (R_MIPS_GPREL16), as retail's is. It moves no
 *   data and emits nothing;
 *   - ablation (task #2677): with upstream's `sda` spelling and nothing
 *   else changed, 54 of 111 masked words differ (0x1BC bytes, not 0x1B8);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L15_004329F0(void *self) {
    Moby430C80 **list;
    void *obj;
    s32 n;
    s32 i;

    if (D_L15_001B14E4 == 0) {
        D_L15_001B14E4 = 1;
        D_L15_00221DC0[0x2C / 4] = 0;
        func_L15_00347748();
        func_L15_00382CA8();
        func_L15_003819D8(0);
        list = func_L15_00381978();
        n = func_L15_00381988();
        for (i = 0; i < n; i++) {
            if (list[i]->uid == 0x957) {
                func_L15_00382D18(list[i], 1);
                func_L15_00384A28(list[i], D_L15_001B2694[0] + D_L15_00221DC0[0] * 128 + 0x30, 64.0f);
                func_L15_00385150(list[i], D_L15_001B2694[0] + D_L15_00221DC0[0] * 128 + 0x30, 64.0f);
            }
        }
        obj = func_L15_00383038(0x1211, 1);
        if (obj != 0) {
            func_L15_00437C40(obj);
            func_L15_00437E50(obj);
        }
        if (D_L15_001B1534 != 0 && D_L15_001B1534->flags != 0) {
            func_L15_0032A2A0(D_L15_001B1534);
            D_L15_001B1534 = 0;
        }
        if (D_L15_001B1538 != 0 && D_L15_001B1538->flags != 0) {
            func_L15_0032A2A0(D_L15_001B1538);
            D_L15_001B1538 = 0;
        }
    }
    if (func_L15_00382F68(func_L15_00381990(0x1211)) == 0 && g_gameTime[0] >= 6) {
        func_L15_00432238(self, 2);
    }
}

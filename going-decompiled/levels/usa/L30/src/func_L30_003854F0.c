/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x003854F0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* A byte inside the boot's g_bGlobalSceneActive block (+0x7), no name at its
 * exact address, so address-named (RULING #13291 section 2). */
extern char D_0018C0DC[];
/* The camera position in L30's boot-name map (exactly 0x001B9C40). */
extern char g_cameraPos[];
/* L30's functions at their v2.00 places. */
extern void func_L30_00317E40(char *);
extern int func_L30_0032D238(char *, int);
extern void func_L30_00322088(char *, int, int);

/**
 * func_L30_003854F0 - three-state trigger: raise a scene flag while the
 * camera is inside the actor's volume.
 * @a0: the actor; its state block is at +0x68 (its first word names the
 *      volume, -1 for none).
 *
 * State 0 (byte +0x20): with no volume the actor is retired
 * (func_L30_00317E40), else it moves to state 1 (func_L30_00322088). State 1:
 * once func_L30_0032D238(g_cameraPos, volume) reports the camera inside,
 * the byte D_0018C0DC is set to 1 and the actor moves to state 2. State 2:
 * the byte stays 1 while the camera is inside; when it leaves, the byte is
 * cleared and the actor goes back to state 1.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0032AAF0.c, function
 * func_L30_003842E0 (v1.01 address), author commit 431ac503 (llesieur99;
 * `git blame -M -C` gives all 27 definition lines to it, `git log -S`
 * agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_003842E0 -> func_L30_003854F0 (our split name);
 *   - D_001B9DC0 -> g_cameraPos (the name L30's boot-name map gives exactly
 *     0x001B9C40, the v2.00 target);
 *   - D_0018C0DC kept (same address in v2.00, inside g_bGlobalSceneActive,
 *     no boot label, operand or symbol_addrs name at it);
 *   - func_316DB8 -> func_L30_00317E40, func_32C0D0 -> func_L30_0032D238,
 *     func_320FD0 -> func_L30_00322088 (L30's functions at their v2.00
 *     places);
 *   - the file's `#include` lines and other declarations dropped.
 */
void func_L30_003854F0(char *a0) {
    char *st = *(char **)(a0 + 0x68);

    switch (*(unsigned char *)(a0 + 0x20)) {
    case 0:
        if (*(int *)st == -1) {
            func_L30_00317E40(a0);
        } else {
            func_L30_00322088(a0, 1, -1);
        }
        break;
    case 1:
        if (func_L30_0032D238(g_cameraPos, *(int *)st) != 0) {
            *(unsigned char *)D_0018C0DC = 1;
            func_L30_00322088(a0, 2, -1);
        }
        break;
    case 2:
        if (func_L30_0032D238(g_cameraPos, *(int *)st) != 0) {
            *(unsigned char *)D_0018C0DC = 1;
        } else {
            *(unsigned char *)D_0018C0DC = 0;
            func_L30_00322088(a0, 1, -1);
        }
        break;
    }
}

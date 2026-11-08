/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x003ECDF8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* The 16-byte vector the object keeps. */
typedef int TI128 __attribute__((mode(TI)));

extern void func_L07_00310FD0(char *, int, int);

extern void func_L07_002E53D8(TI128 *, TI128 *, char *);

extern void func_L07_002E4F18(TI128 *, TI128 *, char *);

extern void func_L07_003E3A50(TI128 *, int, int, int, int, int, int, int, float, float, int, int, int,
                          int);

extern void func_L07_00356570(int, int, char *);

extern void func_L07_002E51F8(TI128 *, TI128 *, float);

extern void func_L07_0031DD30(char *, int, TI128 *, float, float, float, float, int);

/**
 * func_L07_003ECDF8 - arm the actor's mode, then set up its position and
 * hand it to the launcher.
 *
 * The actor's +0x98 word is zeroed, its u16 flags at +0x34 get 0x41 set and
 * 0x1000 cleared, and func_L07_00310FD0(@actor, 0, 3) sets the mode. A point
 * 0.75 along the actor's local z is moved into world space by the +0xC0
 * matrix (func_L07_002E53D8) and offset by the +0x10 position
 * (func_L07_002E4F18), then passed to the effect spawner func_L07_003E3A50
 * with the state block's (+0x68) +0x314 word last. func_L07_00356570(9, 0,
 * @actor) follows, and the state's +0x1C0 vector scaled by 0.05
 * (func_L07_002E51F8) is the launch vector given to func_L07_0031DD30 with
 * 0x7F8, the four speeds 3/6/3/6 and the mask 0xFF.
 *
 * @actor: the object.
 * Return: none.
 *
 * Non-obvious: a sibling of func_L07_003E8580 with its own constants (state
 * +0x314, event 9, launch id 0x7F8) and its own statement order; the two
 * bodies are not token-identical.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/07_siberius/L07_00389EB8.c, function func_L07_003EBAB8
 * (v1.01 address), author commits 753ae275 (bigbeno37) and 3ba5ed46
 * (Louis-Philippe Le Sieur), by blame -M -C of the definition (10 and 8 of
 * its 18 lines; no rename-copy found): Louis-Philippe Le Sieur's draft
 * nonmatching/levels/func_L07_003EBAB8.c (3ba5ed46) supplies the signature,
 * the locals and the first four statements, which survive here, and
 * bigbeno37's 753ae275 moved it into the file and added the rest of the body
 * (RULING #13427 rev 2). blame -M -C -C -C traces the brace and the two
 * aligned float declarations further back, to the same author's earlier
 * draft nonmatching/levels/func_L11_0046BD18.c (520d89e6), so the credited
 * writer is unchanged (RULING #13541 rev 2).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2656 relverify by
 * name):
 *   - func_L07_003EBAB8 -> func_L07_003ECDF8 (our split name);
 *   - func_002A6C58 -> func_L07_00310FD0, func_00283098 -> func_L07_002E53D8,
 *     func_00282CC0 -> func_L07_002E4F18, func_0032C700 -> func_L07_003E3A50,
 *     func_002E5CF8 -> func_L07_00356570, func_00282F20 -> func_L07_002E51F8,
 *     func_002B0A68 -> func_L07_0031DD30 (L07's own copies at the v2.00
 *     places);
 *   - `#include "common.h"`/`#include "include_asm.h"` and the file's other
 *     declarations dropped: this function uses no common.h typedef.
 */
void func_L07_003ECDF8(char *actor)
{
    float position[4] __attribute__((aligned(16)));
    float direction[4] __attribute__((aligned(16)));
    char *state = *(char **)(actor + 0x68);
    *(int *)(actor + 0x98) = 0;
    *(unsigned short *)(actor + 0x34) = (*(unsigned short *)(actor + 0x34) | 0x41) & 0xefff;
    func_L07_00310FD0(actor, 0, 3);
    *(TI128 *)position = 0;
    position[2] = 0.75f;
    func_L07_002E53D8((TI128 *)position, (TI128 *)position, actor + 0xc0);
    func_L07_002E4F18((TI128 *)position, (TI128 *)position, actor + 0x10);
    func_L07_003E3A50((TI128 *)position, 0, 25, 20, 20, 20, 45, 1, 0.6f, 20.0f,
                 1, 0, 0, *(int *)(state + 0x314));
    func_L07_00356570(9, 0, actor);
    func_L07_002E51F8((TI128 *)direction, (TI128 *)(state + 0x1c0), 0.0500000045f);
    func_L07_0031DD30(actor, 0x7f8, (TI128 *)direction, 3.0f, 6.0f, 3.0f, 6.0f, 0xff);
}

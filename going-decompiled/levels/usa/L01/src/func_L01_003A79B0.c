/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003A79B0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

extern float D_L01_001AF83C __attribute__((sda));

extern char D_L01_001BF930[];

extern void func_L01_002E4440(char *, char *, float);

extern void func_L01_002E4178(char *, char *, char *);

extern int func_L01_002D5048(char *, int, char *, float);

extern void func_L01_002E41A8(char *, char *, char *);

/**
 * func_L01_003A79B0 - rebuild the object's display string at +0x10 from the
 * text at +0xE0 and a scale constant, appending a suffix when a predicate
 * over the rebuilt string fires.
 *
 * The buffer is rendered by func_L01_002E4440 (text at +0xE0 scaled by the
 * .lit float D_L01_001AF83C), concatenated with the current +0x10 string by
 * func_L01_002E4178; func_L01_002D5048 then tests the buffer against the
 * object, and only when it returns nonzero is the constant suffix
 * D_L01_001BF930 appended (func_L01_002E41A8) and the result stored back to
 * +0x10.
 *
 * @arg0: the object record (its +0x10 string is updated in place).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003835D8-g8.c, function func_L01_003A65B0
 * (v1.01 address), author commit 91e04732 (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2626 relverify by name):
 *   - func_L01_003A65B0 -> func_L01_003A79B0 (our split name);
 *   - D_L01_001AF87C -> D_L01_001AF83C (our committed .lit label at the
 *     v2.00 target);
 *   - D_L01_001BF930 kept address-named (RULING #13291 section 2: the v2.00
 *     target 0x001BF930 lies inside g_collHitEventRing+0x7B0, no split label
 *     starts there);
 *   - func_00282F20 -> func_L01_002E4440, func_00282CC0 -> func_L01_002E4178,
 *     func_00277258 -> func_L01_002D5048 and func_00282CF0 ->
 *     func_L01_002E41A8 (L01's own copies at the v2.00 places);
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped: this function references no other symbol.
 */
void func_L01_003A79B0(char *arg0) {
    char buf[16];

    func_L01_002E4440(buf, arg0 + 0xE0, D_L01_001AF83C);
    func_L01_002E4178(buf, buf, arg0 + 0x10);
    if (func_L01_002D5048(buf, 0, arg0, D_L01_001AF83C) != 0) {
        func_L01_002E41A8(buf, D_L01_001BF930, buf);
        func_L01_002E4178(arg0 + 0x10, arg0 + 0x10, buf);
    }
}

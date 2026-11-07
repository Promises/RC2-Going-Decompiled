/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x00401D98, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* The boot's voice-load state (core, below 0x1A7C80). */
extern char g_fileLoadVoiceState[];

/**
 * func_L02_00401D98 - start one of the actor's thirty-nine voice messages.
 *
 * @a0: the actor; its state block is at +0x68.
 * @a1: the message, 0..0x26.
 *
 * The message's word comes from state +0x130 (0..0x13) or +0x180 (0x14..).
 * It is refused when the voice-load state is busy (+0x68 non-zero or +0x24
 * not -1); otherwise +0x28 is cleared, +0x24 set to (word & 0x3FFF) + 0x7D0
 * and the message's halfword flag at state +0x1D0 set.
 * Return: 1 when started, 0 when refused or out of range.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003F5430.c,
 * function func_L02_00400650 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_00400650 -> func_L02_00401D98 (our split name at
 *     the v2.00 place);
 *   - D_001A63A8 -> g_fileLoadVoiceState (our name at 0x001A63A8);
 *   - u16/u32/s32 typedefs carried from the file's own;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
s32 func_L02_00401D98(char *a0, s32 a1) {
    char *p = *(char **)(a0 + 0x68);
    s32 v;

    if ((u32)a1 >= 0x27)
        return 0;
    if (a1 < 0x14) {
        s32 i = a1;
        char *q = p + 0x130;
        v = *(s32 *)(q + i * 4);
    } else {
        s32 i = a1 - 0x14;
        char *q = p + 0x180;
        v = *(s32 *)(q + i * 4);
    }
    {
        char *g = g_fileLoadVoiceState;

        if (*(s32 *)(g + 0x68) != 0)
            goto fail;
        if (*(s32 *)(g + 0x24) != -1)
            goto fail;
        *(s32 *)(g + 0x28) = 0;
        *(s32 *)(g + 0x24) = (v & 0x3FFF) + 0x7D0;
    }
    {
        char *q = p + 0x1D0;
        *(u16 *)(q + a1 * 2) = 1;
    }
    return 1;
fail:
    return 0;
}

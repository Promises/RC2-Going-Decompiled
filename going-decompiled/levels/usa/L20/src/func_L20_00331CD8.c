/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x00331CD8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern unsigned char D_L20_001DB9E0[];
extern void *D_L20_001DB080[];
extern void func_L20_00326640(void *);
extern void func_L20_00328D10(void *);

/**
 * func_L20_00331CD8 - set a moby's class and load its class record.
 *
 * Stores the class id (as a short) at +0xAA, looks up the class's slot byte
 * in D_L20_001DB9E0 (stored at +0x22) and the class record pointer in
 * D_L20_001DB080[slot] (stored at +0x24), then runs func_L20_00326640 on the
 * moby. From the record it copies byte +0xE to +0x62, the float +0x24 to
 * +0x2C and the word +0x10 to +0x98, and sets +0x61 to 0xFF. A non-zero
 * record word +0x40 sets flag 0x10 in +0x34 and is copied to +0x78; a
 * non-zero record byte +0xF sets +0x6F to 0x18, flag 0x400 in +0x34 and
 * clears +0x70, +0x74 and +0xBD; a non-zero record byte +6 sets +0x63 to
 * 0x18. Ends with func_L20_00328D10(moby).
 *
 * @arg0: the moby. @arg1: its class id.
 * Return: nothing.
 *
 * Non-obvious: the record pointer is re-read from +0x24 twice, the second
 * time through a `volatile` cast (upstream's), which keeps the third load the
 * ROM does.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_002DEBE0.c, function func_L20_00330A68 (v1.01
 * address), author commit 71f1dc28 (ReaperOfVeriod). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_00330A68 -> func_L20_00331CD8; D_L20_001DB100 ->
 *     D_L20_001DB080; D_L20_001DBA60 -> D_L20_001DB9E0; func_0029F390 ->
 *     func_L20_00326640; func_002A0F48 -> func_L20_00328D10
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 */
void func_L20_00331CD8(void *arg0, int arg1)
{
    unsigned char *p;
    unsigned char *r;
    unsigned char *s;
    int q;

    *(short *)(arg0 + 0xAA) = (short)arg1;
    *(unsigned char *)(arg0 + 0x22) = D_L20_001DB9E0[(short)arg1];
    *(void **)(arg0 + 0x24) = D_L20_001DB080[*(unsigned char *)(arg0 + 0x22)];
    func_L20_00326640(arg0);
    p = *(unsigned char **)(arg0 + 0x24);
    *(unsigned char *)(arg0 + 0x62) = p[0xE];
    *(float *)(arg0 + 0x2C) = *(float *)(p + 0x24);
    *(int *)(arg0 + 0x98) = *(int *)(p + 0x10);
    *(unsigned char *)(arg0 + 0x61) = 0xFF;
    if (p) {
        q = *(int *)(p + 0x40);
        if (q) {
            *(unsigned short *)(arg0 + 0x34) |= 0x10;
            *(int *)(arg0 + 0x78) = *(int *)(p + 0x40);
        }
        r = *(unsigned char **)(arg0 + 0x24);
        if (r[0xF]) {
            *(unsigned char *)(arg0 + 0x6F) = 0x18;
            *(unsigned short *)(arg0 + 0x34) |= 0x400;
            *(int *)(arg0 + 0x70) = 0;
            *(int *)(arg0 + 0x74) = 0;
            *(unsigned char *)(arg0 + 0xBD) = 0;
        }
        s = *(unsigned char * volatile *)(arg0 + 0x24);
        if (s[6]) {
            *(unsigned char *)(arg0 + 0x63) = 0x18;
        }
    }
    func_L20_00328D10(arg0);
}

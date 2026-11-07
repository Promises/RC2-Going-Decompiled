/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x00447A48, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

/* The state's per-line "played" flags at +0x2A0 (see func_L11_00447750). */
typedef struct { char pad[0x2A0]; short counters[17]; } JobaCounters46020;
/* The boot block at 0x00189E20; word 0x933 is read as a timer below. */
extern int g_soundBankHandlesBlk[];
/* A level float at 0x001B18C8 (our split label); `sda` is upstream's own text,
   ignored by this cc1: at -G8 the 4-byte float is $gp-relative by size. */
extern float D_L11_001B18C8 __attribute__((sda));
/* The level's copy of the random-integer helper, and our C line claimer. */
extern int func_L11_0032DCC8(int);
extern int func_L11_00447750(char *, int);

/**
 * func_L11_00447A48 - pick a voice line for the object's current target and
 * claim the voice slot for it.
 *
 * With no target (state+0x180) nothing happens. For a 0xB3A target: when the
 * boot timer word (g_soundBankHandlesBlk[0x933]) has run out, a random line
 * 6..7 that has not been played; line 8 if unplayed (state +0x2B0) and the
 * timer is below D_L11_001B18C8; line 9 if unplayed (+0x2B2) and the target's
 * weight (its state +0x20) is not positive. For a 0xD4A target the same with
 * lines 12..14, 15 (+0x2BE) and 16 (+0x2C0, which also needs the state's
 * count at +0x140 to be 0). Later choices override earlier ones; any other
 * target class picks nothing. A chosen line goes to func_L11_00447750.
 *
 * @self: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_00446020-g8.c, function func_L11_00446020 (v1.01
 * address), author commit 3cd8af2d (bigbeno37). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_00446020 -> func_L11_00447A48 (our split name at the v2.00 place);
 *   - D_00189E20 -> g_soundBankHandlesBlk (the boot's name, same address);
 *   - D_L11_001B1870 -> D_L11_001B18C8 (our split label at the v2.00 target);
 *   - func_002A77E0 -> func_L11_0032DCC8, func_L11_00445D28 -> func_L11_00447750;
 *   - `#include "common.h"` dropped (nothing in this function used it).
 */
void func_L11_00447A48(char *self) {
    int action;
    char *state = *(char **)(self + 0x68);
    char *target = *(char **)(state + 0x180);

    if (target != 0) {
        action = -1;
        if (*(short *)(target + 0xAA) != 0xB3A) {
            if (*(short *)(target + 0xAA) == 0xD4A) {
                char *targetState = *(char **)(target + 0x68);
                if ((float)g_soundBankHandlesBlk[0x933] <= 0.0f) {
                    action = func_L11_0032DCC8(3) + 0xC;
                    if (((JobaCounters46020 *)state)->counters[action] != 0) {
                        action = -1;
                    }
                }
                if (*(short *)(state + 0x2BE) == 0) {
                    if ((float)g_soundBankHandlesBlk[0x933] < D_L11_001B18C8) {
                        action = 0xF;
                    }
                }
                if (*(short *)(state + 0x2C0) == 0) {
                    if (*(float *)(targetState + 0x20) <= 0.0f) {
                        if (*(int *)(state + 0x140) == 0) {
                            action = 0x10;
                        }
                    }
                }
            }
        } else {
            char *targetState = *(char **)(target + 0x68);
            if ((float)g_soundBankHandlesBlk[0x933] <= 0.0f) {
                action = func_L11_0032DCC8(2) + 6;
                if (((JobaCounters46020 *)state)->counters[action] != 0) {
                    action = -1;
                }
            }
            if (*(short *)(state + 0x2B0) == 0) {
                if ((float)g_soundBankHandlesBlk[0x933] < D_L11_001B18C8) {
                    action = 8;
                }
            }
            if (*(short *)(state + 0x2B2) == 0) {
                if (*(float *)(targetState + 0x20) <= 0.0f) {
                    action = 9;
                }
            }
        }
        if (action > 0) {
            func_L11_00447750(self, action);
        }
    }
}

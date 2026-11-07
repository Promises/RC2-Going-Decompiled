
/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D5FC8 - the held-target test: 1 only when the chain word
(state+0x310) names a record whose byte at +0x20 is >= 3 and not 0xFE/0xFD and
the float at state+0x20 still lies below (3 - step)/3 of the count at
state+0x24; 0 otherwise.

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D4A60 (v1.01 address), author commit 773eafc2
(llesieur99). BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names.
Respelling: func_L02_003D4A60 -> func_L02_003D5FC8 (our v2.00 place). No
relocations.

*/

int func_L02_003D5FC8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *holder = *(char **)(state + 0x310);
    unsigned char mode;
    int hit = 0;

    if (holder != 0) {
        mode = *(unsigned char *)(holder + 0x20);
        if (mode != 0xFE) {
            if (mode != 0xFD) {
                if (mode >= 3) {
                    if (*(float *)(state + 0x20) <
                        (float)*(short *)(state + 0x24) *
                            (3.0f - (float)*(short *)(state + 0x31A)) * 0.33333334f) {
                        hit = 1;
                    }
                }
            }
        }
    }
    return hit;
}

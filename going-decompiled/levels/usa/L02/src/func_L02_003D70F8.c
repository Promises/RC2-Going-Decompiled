extern void func_L02_003D7360(char *);

/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D70F8 - release the actor's held-target chain and clear the
carried slots' armed bits. If the chain word (state+0x300) is set, runs
func_L02_003D7360 on it and zeroes the word. Then clears state+0x222, sets the
six floats at +0x240/+0x244/+0x248 and +0x280/+0x284/+0x288 to 1.0f, clears
state+0x262, and walks the five slots at state+0x2A0 backwards setting each
live one's flags bit 0 (+0x34) and clearing its +0x31 byte.

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D5B90 (v1.01 address), author commit 773eafc2
(llesieur99). BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names.
Respellings: func_L02_003D5B90 -> func_L02_003D70F8 (our v2.00 place);
func_L02_003D5DF8 -> func_L02_003D7360 (relocation-derived). Extern decl
written out; #include dropped (unused).

*/

void func_L02_003D70F8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *holder = *(char **)(state + 0x300);
    char **slots;
    char *node;
    int i;

    if (holder != 0) {
        func_L02_003D7360(holder);
        *(int *)(state + 0x300) = 0;
    }
    *(unsigned char *)(state + 0x222) = 0;
    *(float *)(state + 0x248) = 1.0f;
    *(float *)(state + 0x244) = 1.0f;
    *(float *)(state + 0x240) = 1.0f;
    *(float *)(state + 0x288) = 1.0f;
    *(float *)(state + 0x284) = 1.0f;
    *(float *)(state + 0x280) = 1.0f;
    *(unsigned char *)(state + 0x262) = 0;
    slots = (char **)(state + 0x2A0);
    for (i = 4; i >= 0; i--) {
        node = slots[0];
        if (node != 0) {
            *(unsigned short *)(node + 0x34) = *(unsigned short *)(node + 0x34) | 1;
            *(unsigned char *)(slots[0] + 0x31) = 0;
        }
        slots++;
    }
}

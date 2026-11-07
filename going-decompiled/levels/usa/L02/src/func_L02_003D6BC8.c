extern void func_L02_003297F8(char *, char *, int, int);

/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D6BC8 - set the carried sub-object chain's mode. arg1 == 1 arms the
+0x262 variant (state+0x288/+0x284/+0x280 floats to 0.0001f); any other value
arms the +0x222 variant (+0x248/+0x244/+0x240 to 0.0001f). Then zeroes the
halfword at state+0x320 and, walking the five slots at state+0x2A0 backwards,
clears bit 0 of each live slot's flags halfword (+0x34), sets its +0x31 byte to
1, and hands it to func_L02_003297F8 with arg1. Finally zeroes state+0x31E.

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D5660 (v1.01 address), author commit 431ac503
(llesieur99). BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names.
Respellings: func_L02_003D5660 -> func_L02_003D6BC8 (our v2.00 place);
func_002AF1D0 -> func_L02_003297F8 (relocation-derived). Extern decl written
out; #include dropped (unused).

*/

void func_L02_003D6BC8(char *arg0, int arg1) {
    char *state = *(char **)(arg0 + 0x68);
    char **slots;
    int i;

    if (arg1 == 1) {
        *(unsigned char *)(state + 0x262) = arg1;
        *(float *)(state + 0x288) = 0.0001f;
        *(float *)(state + 0x284) = 0.0001f;
        *(float *)(state + 0x280) = 0.0001f;
    } else {
        *(unsigned char *)(state + 0x222) = 1;
        *(float *)(state + 0x248) = 0.0001f;
        *(float *)(state + 0x244) = 0.0001f;
        *(float *)(state + 0x240) = 0.0001f;
    }
    *(short *)(state + 0x320) = 0;
    {
        unsigned char one = 1;

        slots = (char **)(state + 0x2A0);
        for (i = 4; i >= 0; i--) {
            if (slots[0] != 0) {
                *(unsigned short *)(slots[0] + 0x34) =
                    *(unsigned short *)(slots[0] + 0x34) & 0xFFFE;
                *(unsigned char *)(slots[0] + 0x31) = one;
                func_L02_003297F8(arg0, slots[0], arg1, 0);
            }
            slots++;
        }
    }
    *(short *)(state + 0x31E) = 0;
}

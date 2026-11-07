
/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D5FB0 - clear the actor's one-shot done word (state+0x318) when it
is set.

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D4A48 (v1.01 address), author commit 773eafc2
(llesieur99). BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names.
Respelling: func_L02_003D4A48 -> func_L02_003D5FB0 (our v2.00 place). No
relocations.

*/

void func_L02_003D5FB0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(short *)(state + 0x318) != 0) {
        *(short *)(state + 0x318) = 0;
    }
}

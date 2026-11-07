
/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003DCDD8 - store the actor's chain word: writes arg1 to
state+0x264 (state is the pointer at obj+0x68).

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003DB310.c,
function func_L02_003DB870 (v1.01 address), author commit adcbd51e
(llesieur99). BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names.
Respelling: func_L02_003DB870 -> func_L02_003DCDD8 (our v2.00 place). No
relocations.

*/

void func_L02_003DCDD8(char *arg0, int arg1) {
    *(int *)(*(char **)(arg0 + 0x68) + 0x264) = arg1;
}

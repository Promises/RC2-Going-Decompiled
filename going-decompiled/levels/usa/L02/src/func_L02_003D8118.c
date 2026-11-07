
/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D8118 - whether the actor's variant byte (+0x20) is one of the
later variants (>= 3).

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D6BB0 (v1.01 address), author commit 773eafc2
(llesieur99). BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names.
Respelling: func_L02_003D6BB0 -> func_L02_003D8118 (our v2.00 place). No
relocations.

*/

int func_L02_003D8118(char *arg0) {
    return *(unsigned char *)(arg0 + 0x20) >= 3;
}

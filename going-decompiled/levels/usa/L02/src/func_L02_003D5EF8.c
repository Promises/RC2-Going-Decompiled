
/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D5EF8 - advance the actor's step counter (+0x31A) until the float
at state+0x20 no longer lies below (3 - step)/3 of the count at state+0x24,
then latch the done word at state+0x318 to 1. One-shot: returns at once when
the done word is set.

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D4990 (v1.01 address), author commit ac1ee013 (bigmak94).
BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names. Respelling:
func_L02_003D4990 -> func_L02_003D5EF8 (our v2.00 place). No relocations.

*/

void func_L02_003D5EF8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(short *)(state + 0x318) != 0)
        return;
    (*(short *)(state + 0x31A))++;
    while (*(float *)(state + 0x20) <
           (float)*(short *)(state + 0x24) * (3.0f - (float)*(short *)(state + 0x31A)) * 0.33333334f)
        (*(short *)(state + 0x31A))++;
    *(short *)(state + 0x318) = 1;
}

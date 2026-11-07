typedef int TI128 __attribute__((mode(TI)));
extern void func_L02_003D3428(char *, TI128 *);
extern void func_L02_003FD750(char *, int);
extern void func_L02_00316610(char *, char *);
extern void func_L02_002F2AB0(char *, int, int);
extern void func_L02_003174E8(char *);
extern void func_L02_0031F6B8(char *, int, int, int);
extern void func_L02_0031F998(char *, int, int);

/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D38E8 - tear the actor's shared effect record down after its
sub-object is gone. Loads the state (obj+0x68); while its effect pointer
(+0x2E4) is set, copies the effect's position vector (+0x10) and hands both to
the shared reset func_L02_003D3428; then, if state+0x2F6 says sound id 0x9CA
was playing, stops it (func_L02_003FD750 on obj+0xB8); zeroes the sound word,
the effect pointer (+0x2E0) and, when the variant byte at +0x291 is set, the
three colour scales at +0x2B0/+0x2B4/+0x2B8 plus the +0x290 record (through
func_L02_00316610 and the 0x40-byte clear func_L02_002F2AB0). Finally re-arms
the actor (func_L02_0031F6B8 / func_L02_003174E8 / func_L02_0031F998), clears
obj+0x31 and obj+0x98, and rewrites the flags halfword at obj+0x34 as
(flags & 0xEFFF) | 0x47.

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D2390.c,
function func_L02_003D2390 (v1.01 address), author commit 291c840e (Flavius).
BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names. Respellings:
func_L02_003D2390 -> func_L02_003D38E8 (our v2.00 place); callees by the
v2.00 ROM's own relocations at this place (draft relverify, task #2558):
func_2F1CC8 -> func_L02_002F2AB0, func_315348 -> func_L02_00316610,
func_316220 -> func_L02_003174E8, func_31E3C0 -> func_L02_0031F6B8,
func_31E6A0 -> func_L02_0031F998, func_3FC1B0 -> func_L02_003FD750,
func_L02_003D1ED0 -> func_L02_003D3428. The typedef and extern decls are
written out (upstream took them from common.h and the file's head);
#include dropped (unused); upstream's file-head comment folded into this one.

*/

void func_L02_003D38E8(char *self) {
    char *state = *(char **)(self + 0x68);

    if (*(char **)(state + 0x2E4) != 0) {
        TI128 pos = *(TI128 *)(*(char **)(state + 0x2E4) + 0x10);

        func_L02_003D3428(self, &pos);
    }
    if (*(short *)(state + 0x2F6) == 1)
        func_L02_003FD750(*(char **)(self + 0xB8), 0x9CA);
    *(short *)(state + 0x2F6) = 0;
    *(int *)(state + 0x2E0) = 0;
    if (*(unsigned char *)(state + 0x291) != 0) {
        *(unsigned char *)(state + 0x292) = 0;
        *(float *)(state + 0x2B0) = *(float *)(state + 0x2B4) = *(float *)(state + 0x2B8) = 1.0f;
        func_L02_00316610(self, state + 0x290);
        func_L02_002F2AB0(state + 0x290, 0, 0x40);
    }
    func_L02_0031F6B8(self, 0, 0, 3);
    func_L02_003174E8(self);
    func_L02_0031F998(self, 1, 0);
    *(unsigned char *)(self + 0x31) = 0;
    *(int *)(self + 0x98) = 0;
    *(unsigned short *)(self + 0x34) = (*(unsigned short *)(self + 0x34) & 0xEFFF) | 0x47;
}

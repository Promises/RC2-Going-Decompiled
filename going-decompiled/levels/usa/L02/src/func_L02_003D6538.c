typedef int TI128 __attribute__((mode(TI)));
extern void func_L02_00323170(char *, char *, float, float, float, float);
extern void func_L02_00323628(char *, char *, float, float, float, float, int);
extern void func_L02_00328918(char *, char *, char *, float, float, float);

/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D6538 - copy the 16-byte vector at arg1 into the effect slots the
actor's state keeps: three easing copies of the vector's xyz into state+0x1B0/
+0x1B4/+0x1B8 (func_L02_00323170, 0.025f steps, 1.5000001f clamp), two eased
angle copies (func_L02_00323628) from arg2/arg3 into state+0x1C4/+0x2F0 (the
latter's rate taken live from state+0x124/+0x128/+0x12C), and one eased vector
(func_L02_00328918) from state+0x2F4/+0x304 (0.5f, 0.087266475f, -1.0f).

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D4FD0 (v1.01 address), author commit ee3fee45
(llesieur99). BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names.
Respellings: func_L02_003D4FD0 -> func_L02_003D6538 (our v2.00 place);
func_321E78 -> func_L02_00323170, func_322330 -> func_L02_00323628,
func_327620 -> func_L02_00328918 (relocation-derived). The 128-bit typedef is
written out (upstream took it from common.h); extern decls written out;
#include dropped (unused).

*/

void func_L02_003D6538(char *arg0, TI128 *arg1, float arg2, float arg3) {
    char *state = *(char **)(arg0 + 0x68);
    TI128 vec = *arg1;
    float *v = (float *)&vec;

    func_L02_00323170(arg0 + 0x10, state + 0x1B0, v[0], 0.025f, 0.025f, 1.5000001f);
    func_L02_00323170(arg0 + 0x14, state + 0x1B4, v[1], 0.025f, 0.025f, 1.5000001f);
    func_L02_00323170(arg0 + 0x18, state + 0x1B8, v[2], 0.025f, 0.025f, 1.5000001f);
    func_L02_00323628(arg0 + 0xF8, state + 0x1C4, arg2, 0.003490659f, 0.003490659f,
                0.22107507f, 0);
    func_L02_00323628(arg0 + 0xF4, state + 0x2F0, arg3, *(float *)(state + 0x124),
                *(float *)(state + 0x128), *(float *)(state + 0x12C), 0);
    func_L02_00328918(arg0, state + 0x2F4, state + 0x304, 0.5f, 0.087266475f, -1.0f);
}

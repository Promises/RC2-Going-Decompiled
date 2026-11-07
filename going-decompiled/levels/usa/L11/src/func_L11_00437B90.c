/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00437B90, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 199 of
 * 239 .text words differ from the disc, relocated fields masked, and .text is
 * 0x3BC bytes, not the place's 0x414, and it emits a .lit4 pool of 40 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

extern int D_L11_001A8FF4 __attribute__((sda));
extern char g_cameraPos[];
extern int func_L11_00302400(float);
extern float func_L11_0032DD60(float,float);
extern int func_L11_0032DCC8(int);
extern void func_L11_004382B8(char *);
extern char *func_L11_00330218(char *,int,int);
extern char *func_L11_00443038(int);
extern void func_L11_00301210(float *,const float *,const float *);
extern void func_L11_003014F8(char *,char *,float);
extern float func_L11_00301300(void *,void *);
extern float func_L11_00301340(void *);
extern int func_L11_0033A1D8(char *,char *,int);
extern void func_L11_00438090(char *);
extern void func_L11_00301D20(void *,void *);
extern void func_L11_00338628(void *,void *,int);
extern int func_L11_00447BE8(int,void *);
extern void func_L11_0032DBF8(void *,int,int);
extern float func_L11_00301398(void *);
extern void func_L11_003011E0(char *,char *,char *);
extern float func_L11_003013B8(void *,void *);
extern void func_L11_0032FF88(char *);
#define F(p,t,o) (*(t *)((char *)(p)+(o)))

/**
 * func_L11_00437B90 - per-frame update of a Joba enemy: set it up on its first
 * frame, then handle hits and keep it moving near its target.
 *
 * @p: the enemy (moby); +0x20 its stage, +0x68 its state block.
 * Return: none.
 *
 * Non-obvious: On stage 0 it seeds its state block: sight ranges 15 and 4, a
 * random-sign turn rate of 15 to 40 degrees (func_L11_0032DD60,
 * func_L11_0032DCC8), a random mirror flag in +0x34, and its motion constants.
 * Then it starts its setup func_L11_004382B8. F(p, type, offset) is upstream's
 * field-access macro.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00436350-g8.c (blob 79f3e83a), function
 * func_L11_00436350 (v1.01 address), author commit 644c5b4d (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2578):
 *   - func_L11_00436350 -> func_L11_00437B90 (our split name at the v2.00
 *     place);
 *   - D_L11_001BA8C0 -> g_cameraPos (L11's boot-map name at
 *     the v2.00 target 0x001BA940);
 *   - func_00282CC0 -> func_L11_003011E0 (the level's own copy at its L11
 *     place);
 *   - func_00282CF0 -> func_L11_00301210 (the level's own copy at its L11
 *     place);
 *   - func_00282DB0 -> func_L11_00301300 (the level's own copy at its L11
 *     place);
 *   - func_00282DF0 -> func_L11_00301340 (the level's own copy at its L11
 *     place);
 *   - func_00282E48 -> func_L11_003013B8 (the level's own copy at its L11
 *     place);
 *   - func_00282F20 -> func_L11_003014F8 (the level's own copy at its L11
 *     place);
 *   - func_00283698 -> func_L11_00301D20 (the level's own copy at its L11
 *     place);
 *   - func_00283CF0 -> func_L11_00302400 (the level's own copy at its L11
 *     place);
 *   - func_002A7750 -> func_L11_0032DBF8 (the level's own copy at its L11
 *     place);
 *   - func_002A77E0 -> func_L11_0032DCC8 (the level's own copy at its L11
 *     place);
 *   - func_002A7878 -> func_L11_0032DD60 (the level's own copy at its L11
 *     place);
 *   - func_002A93B0 -> func_L11_0032FF88 (the level's own copy at its L11
 *     place);
 *   - func_002A9640 -> func_L11_00330218 (the level's own copy at its L11
 *     place);
 *   - func_002AFFD8 -> func_L11_00338628 (the level's own copy at its L11
 *     place);
 *   - func_002B0ED8 -> func_L11_0033A1D8 (the level's own copy at its L11
 *     place);
 *   - func_L00_002EE290 -> func_L11_00301398 (the level's own copy at its L11
 *     place);
 *   - func_L11_00436850 -> func_L11_00438090 (its v2.00 split name);
 *   - func_L11_00436A78 -> func_L11_004382B8 (its v2.00 split name);
 *   - func_L11_004417E8 -> func_L11_00443038 (its v2.00 split name);
 *   - func_L11_004461C0 -> func_L11_00447BE8 (its v2.00 split name);
 *   - D_L11_001BA940 -> g_cameraPos (task #2646): the name
 *     symbol_addrs/L11_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_00437B90(char *p) {
    char *s=F(p,char *,0x68), *hit;
    float v[4], angle, z;
    if (F(p,unsigned char,0x20)==0) {
        F(s,unsigned char,0x48)=func_L11_00302400(15.0f);
        F(s,unsigned char,0x4A)=func_L11_00302400(4.0f);
        F(s,unsigned char,0x29)=1;
        F(s,char *,0x18)=s+0x100;
        angle=func_L11_0032DD60(15.0f,40.0f)*0.017453294f;
        if(func_L11_0032DCC8(2)) angle=-angle;
        F(s,float,0x274)=angle;
        if(func_L11_0032DCC8(2)==1) F(p,unsigned short,0x34)|=0x8000;
        F(s,short,0x282)=0;
        F(s,unsigned char,0x60)=10;
        F(s,int,0x98)=4;
        F(s,float,0x84)=0.65f;
        F(s,float,0x80)=0.6f;
        F(s,float,0x100)=0.6f;
        F(s,float,0x104)=0.05f;
        F(s,float,0x8C)=0.4f;
        F(s,float,0x108)=0.4f;
        F(s,float,0x10C)=0.4f;
        F(s,float,0x134)=0.01f;
        F(s,float,0x110)=1.5707964897155762f;
        F(s,float,0x130)=0.523598849773407f;
        F(s,float,0x12C)=0.0027777778450399637f;
        F(s,unsigned char,0x28)=3;
        func_L11_004382B8(p);
    }
    if(F(p,unsigned char,0x95)>=2) {
        hit=func_L11_00330218(p,0x10000,1);
        if(hit) {
            char *other=F(hit,char *,0x20);
            if(other && !F(p,unsigned char,0x31) &&
            (F(other,short,0xAA)==0xA03 || F(other,short,0xAA)==0xB89)) F(hit,int,0x2C)=0;
            if(F(s,short,0x286)) {
                char *velocity=hit+0x10;
                func_L11_00301210(v,(float *)func_L11_00443038(F(p,int,0xB8)),(float *)(p+0x10));
                v[2]=0;
                func_L11_003014F8((char *)v,(char *)v,1.0f);
                func_L11_003014F8((char *)v,(char *)v,func_L11_00301300(velocity,v));
                angle=func_L11_00301340(velocity);
                func_L11_00301210((float *)velocity,(float *)velocity,v);
                func_L11_003014F8(velocity,velocity,angle);
            }
        }
        if(func_L11_0033A1D8(p,s+0x50,0)==5) {
            F(s,int,0xE0)=0;
            F(s,float,0xE4)=0.8377581834793091f;
        }
        func_L11_00438090(p);
        if(F(s,short,0x286)) {
            func_L11_00301D20(s+0x210,p+0xC0);
            func_L11_00338628(p+0x10,s+0x240,0);
            if(!func_L11_00447BE8(F(p,int,0xB8),p+0x10)) {
                F(s,int,0x20)=0;
                func_L11_0032DBF8(p,11,4);
            }
            if(F(p,unsigned char,0x20)!=11) {
                z=F(p,float,0x18);
                func_L11_00301210(v,(float *)(p+0x10),(float *)func_L11_00443038(F(p,int,0xB8)));
                v[2]=0;
                if(func_L11_00301398(v)>225.0f) {
                    func_L11_003014F8((char *)v,(char *)v,15.0f);
                    func_L11_003011E0(p+0x10,(char *)v,func_L11_00443038(F(p,int,0xB8)));
                    F(p,float,0x18)=z;
                }
            }
        }
    }
    if(!F(s,short,0x286) && F(p,unsigned char,0x20)!=11 && !D_L11_001A8FF4) {
        F(p,unsigned short,0x34)|=0x400;
        if(F(p,unsigned char,0x31) && func_L11_003013B8(p+0x10,g_cameraPos)<24.0f) {
            func_L11_0032FF88(p);
            F(p,unsigned char,0x6F)=22;
        }
    } else F(p,unsigned short,0x34)&=0xFBFF;
    if(F(p,unsigned char,0x20)>=2 && F(s,unsigned char,0x2E)==2) func_L11_004382B8(p);
}

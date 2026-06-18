/* ee_utils.c - M1 SDK/EE utility shims (TARGET_NATIVE only).
 *
 * Portable bodies for leaf EE utility routines that live ONLY in raw-split asm
 * (no per-function INCLUDE_ASM home in a .c), so they would otherwise stay
 * trap-stubs in the native build. Inert to the matching build (which assembles
 * the original asm). Behavior derived from the R5900 disasm + verified by a
 * ghidra-annotator pass (addresses below); see docs/HLE.md (M1).
 *
 * Both fill/copy take a BYTE count in arg3 (NOT element/qword count) and, like
 * the asm, write at least one element even when the count is <= 0 (the original
 * does an unconditional first store before the loop-guard). No observed caller
 * passes <= 0; the edge is reproduced for faithfulness.
 */
#ifdef TARGET_NATIVE

#include "common.h"

/* FillMemory32 @0x00283410: store the 32-bit `word` across `nbytes` bytes
 * (multiple of 4). asm: sw a1,0(a0); a2-=4; while(a2>0){a0+=4; sw a1,0(a0); a2-=4;} */
void FillMemory32(void *dst, u32 word, s32 nbytes)
{
    u32 *p = (u32 *)dst;
    s32 n = nbytes;
    do {
        *p++ = word;
        n -= 4;
    } while (n > 0);
}

/* CopyQwords @0x00283500: copy 128-bit quadwords dst<-src across `nbytes` bytes
 * (multiple of 16). asm: do{ lq v1,0(a1); a1+=16; a0+=16; a2-=16; sq v1,-16(a0); }while(a2>0) */
void CopyQwords(void *dst, const void *src, s32 nbytes)
{
    u32 *d = (u32 *)dst;
    const u32 *s = (const u32 *)src;
    s32 n = nbytes;
    do {
        d[0] = s[0];
        d[1] = s[1];
        d[2] = s[2];
        d[3] = s[3];
        d += 4;
        s += 4;
        n -= 16;
    } while (n > 0);
}

#endif /* TARGET_NATIVE */

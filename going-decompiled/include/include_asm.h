#ifndef INCLUDE_ASM_H
#define INCLUDE_ASM_H

#if !defined(M2CTX) && !defined(PERMUTER) && !defined(TARGET_NATIVE)

#ifndef INCLUDE_ASM
#define INCLUDE_ASM(FOLDER, NAME) \
    __asm__( \
        ".section .text\n" \
        "    .set noat\n" \
        "    .set noreorder\n" \
        "    .include \"" FOLDER "/" #NAME ".s\"\n" \
        "    .set reorder\n" \
        "    .set at\n" \
    )
#endif
#ifndef INCLUDE_RODATA
#define INCLUDE_RODATA(FOLDER, NAME) \
    __asm__( \
        ".section .rodata\n" \
        "    .include \"" FOLDER "/" #NAME ".s\"\n" \
        ".section .text" \
    )
#endif

#if INCLUDE_ASM_USE_MACRO_INC
__asm__(".include \"include/macro.inc\"\n");
#else
__asm__(".include \"include/labels.inc\"\n");
#endif

#else

#ifndef INCLUDE_ASM
#define INCLUDE_ASM(FOLDER, NAME)
#endif
#ifndef INCLUDE_RODATA
#define INCLUDE_RODATA(FOLDER, NAME)
#endif

#endif /* !defined(M2CTX) && !defined(PERMUTER) */

/* A splat leaf that is NOT a function: an orphaned epilogue tail, an
 * inter-function fill word, or a dead remnant after a `jr $ra`. The image
 * needs its bytes, so it is still pulled in exactly like INCLUDE_ASM (the two
 * expand identically), but it is not a match target and must not be counted
 * as one. tools/ee/fragment_census.py --lint checks that every site carrying
 * this marker classifies as debris from its .s content, and that no small
 * plain INCLUDE_ASM leaf does. */
#define INCLUDE_ASM_FRAGMENT(FOLDER, NAME) INCLUDE_ASM(FOLDER, NAME)

#endif /* INCLUDE_ASM_H */

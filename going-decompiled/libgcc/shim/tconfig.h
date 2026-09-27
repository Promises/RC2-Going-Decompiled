/* PROJECT-WRITTEN build shim, not GCC source.
 *
 * libgcc2.c includes "tconfig.h" from GCC's build tree (where it pulls in the
 * target's tm.h). There is no GCC build tree here, so this supplies the few
 * target facts libgcc2.c reads for mips/R5900, little-endian (see PROVENANCE). */
#define BITS_PER_UNIT 8
#define WORDS_BIG_ENDIAN 0
#define LONG_DOUBLE_TYPE_SIZE 64
#define MIN_UNITS_PER_WORD 4
#define inhibit_libc

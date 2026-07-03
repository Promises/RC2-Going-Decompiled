#ifndef COMMON_H
#define COMMON_H

#include "include_asm.h"

/* Two build targets share this tree (see docs/PORTING.md):
 *   - default (no macro): the byte-exact PS2 build (ee-gcc EABI) - the matching
 *     oracle. Unmatchable functions use INCLUDE_ASM (the original MIPS .s).
 *   - TARGET_NATIVE: a portable x86/ARM build. Matched C is reused as-is;
 *     INCLUDE_ASM functions provide a functionally-equivalent C body in their
 *     #else branch. Build this target ILP32 so pointers stay 4 bytes like the
 *     PS2/EE (keeps struct layouts identical). Hardware (GS/VIF/DMA/VU) code is
 *     not yet ported - that needs the platform backend (deferred). */
#ifdef TARGET_NATIVE
#include <stdint.h>
#include <string.h>         /* memset/memcpy/memcmp used by #else portable bodies */
#define _USE_MATH_DEFINES   /* M_PI on MSVC; harmless elsewhere */
#include <math.h>           /* PR_PI below uses M_PI */
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
#else
typedef char s8;
typedef short s16;
typedef int s32;
typedef long s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef float f32;
#endif

#define UNK_TYPE s32
#define UNK_PTR void*
#define UNK_RET void
#define UNK_FUN_ARG void(*)(void)
#define UNK_FUN_PTR(name) void(*name)(void)
#define UNK_ARGS

#ifndef NULL
#define NULL  0
#endif
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))

#define GS_X_COORD(x) ((2048 - (640 / 2) + x) << 4)
#define GS_Y_COORD(y) ((2048 - (224 / 2) + y) << 4)

#define PR_EXTERN extern "C"

#define PR_SIZEOF(x) (int)(sizeof(x))
#define PR_ARRAYSIZEU(arr) (sizeof(arr) / sizeof(arr[0]))
#define PR_ARRAYSIZE(arr) (s32)(sizeof(arr) / sizeof(arr[0]))
#define PR_CONCAT(x, y) ((x << 16) | (y))
#define PR_BIT(x) (1 << x)

#define PR_SCOPE() {
#define PR_SCOPEEND() }

#define PR_ALIGNU(size, align) ((u_int)(size + (align - 1)) & ~(align - 1))
#define PR_ALIGN(size, align) ((size + (align - 1)) & ~(align - 1))

#define PR_ALIGNED(x) __attribute__((aligned(x)))

#define PR_UC_ADDR (0x20000000)
#define PR_UCA_ADDR (0x30000000)

#define PR_UNCACHED(addr)      ((u_int)(addr) | PR_UC_ADDR)
#define PR_UNCACHEDACCEL(addr) ((u_int)(addr) | PR_UCA_ADDR)

#define PR_PADDING(name, x) char name[x]

#define PR_BREAK() asm("break")

/* literal so PR_PI is valid in BOTH builds (the matching build pulls in no
 * <math.h>); same float bits as (float)M_PI. */
#define PR_PI 3.14159265358979323846f

#endif /* COMMON_H */

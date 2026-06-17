/*
 * native_prelude.h — libc headers the TARGET_NATIVE #else bodies use. Force-
 * included (clang/gcc -include) by the native build so units that call
 * memset/memcpy etc. compile without editing the 19 source files. Only active
 * under TARGET_NATIVE.
 *
 * NOTE: we deliberately do NOT declare the still-INCLUDE_ASM (MIPS) game
 * callees here — the units already declare the ones they use with correct,
 * specific signatures, and a blanket K&R declaration would *conflict* with
 * those. Undeclared callees (the genuine linkage boundary) are left to surface
 * as warnings under the gate's -Wno-error=implicit-function-declaration; the
 * runnable harness (Tier 2) supplies real stub DEFINITIONS for linking.
 */
#ifndef NATIVE_PRELUDE_H
#define NATIVE_PRELUDE_H

#ifdef TARGET_NATIVE
#include <string.h>   /* memset, memcpy, memcmp */
#include <stdlib.h>
#endif

#endif /* NATIVE_PRELUDE_H */

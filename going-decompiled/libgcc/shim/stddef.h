/* PROJECT-WRITTEN build shim, not GCC source: the one stddef.h type libgcc2.c
 * uses (size_t), for the EE's 32-bit ABI. */
#ifndef _LIBGCC_SHIM_STDDEF_H
#define _LIBGCC_SHIM_STDDEF_H
typedef unsigned int size_t;
#endif

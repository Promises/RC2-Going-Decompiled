/* PROJECT-WRITTEN build shim, not newlib source. It stands in for the
 * <errno.h> that newlib/libm/math/w_sqrt.c includes: errno is reached through
 * the reentrant accessor __errno(), and EDOM is 33, the constant the SDK's
 * own w_sqrt.o stores (li v1,33 at .text+0xc4). */
#ifndef LIBM_SHIM_ERRNO_H
#define LIBM_SHIM_ERRNO_H

extern int *__errno(void);
#define errno (*__errno())

#define EDOM 33

#endif

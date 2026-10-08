/* PROJECT-WRITTEN build shim, not newlib source. It stands in for the
 * <machine/ieeefp.h> that newlib/libm/common/fdlibm.h includes: the EE is
 * little-endian with a 64-bit IEEE double, and fdlibm.h's word macros need
 * 32-bit integer types. */
#ifndef LIBM_SHIM_MACHINE_IEEEFP_H
#define LIBM_SHIM_MACHINE_IEEEFP_H

#define __IEEE_LITTLE_ENDIAN

typedef int __int32_t;
typedef unsigned int __uint32_t;

#endif

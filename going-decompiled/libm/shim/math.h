/* PROJECT-WRITTEN build shim, not newlib source. It stands in for the
 * <math.h> that newlib/libm/common/fdlibm.h includes, and declares only what
 * s_isnan.c and w_sqrt.c use, for the EE's ABI. The interface it states
 * (the SVID `struct exception` layout, the DOMAIN code, the fdlibm error-mode
 * values and the `__fdlib_version` symbol) is fixed by the SDK libm.a objects
 * these two files reproduce byte-exact; see PROVENANCE. */
#ifndef LIBM_SHIM_MATH_H
#define LIBM_SHIM_MATH_H

#include <machine/ieeefp.h>

/* Argument record handed to matherr(); w_sqrt.c fills it for x < 0. */
struct exception {
	int type;
	char *name;
	double arg1;
	double arg2;
	double retval;
	int err;
};

#define DOMAIN 1	/* exception.type: argument outside the domain */

extern int isnan(double);
extern int matherr(struct exception *);

/* fdlibm's error-handling mode, a read-only global the wrappers consult. */
enum libm_shim_error_mode {
	libm_shim_ieee = -1,
	libm_shim_svid,
	libm_shim_xopen,
	libm_shim_posix
};
extern const enum libm_shim_error_mode __fdlib_version;

#define _LIB_VERSION __fdlib_version
#define _IEEE_  libm_shim_ieee
#define _SVID_  libm_shim_svid
#define _XOPEN_ libm_shim_xopen
#define _POSIX_ libm_shim_posix

#endif

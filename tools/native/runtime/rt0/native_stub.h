/* native_stub.h — M4 stub-trap runtime (TARGET_NATIVE).
 *
 * The native build weak-stubs every still-INCLUDE_ASM game function so a partial
 * binary LINKS (see runtime/rt0/gen_stubs.py + docs/HLE.md M4). A stub is NOT a
 * silent no-op: when a host run reaches unimplemented code it must be FLAGGED as
 * "hit a stub", never mistaken for a wrong result. So every stub records a
 * sentinel and traps with a distinct exit code.
 *
 * A harness can: (1) check exit == NATIVE_STUB_EXIT to distinguish a stub-hit
 * from an assertion failure, and (2) read g_nativeStubName for which one.
 */
#ifndef NATIVE_STUB_H
#define NATIVE_STUB_H

#ifdef TARGET_NATIVE

#define NATIVE_STUB_EXIT 99  /* distinct from assert-fail / normal exit codes */

extern int         g_nativeStubHit;   /* set to 1 the moment any stub is hit  */
extern const char *g_nativeStubName;  /* name of the stub that was hit         */

/* Record the sentinel + trap. noreturn: the process exits NATIVE_STUB_EXIT. */
void native_stub_hit(const char *name);

#endif /* TARGET_NATIVE */
#endif /* NATIVE_STUB_H */

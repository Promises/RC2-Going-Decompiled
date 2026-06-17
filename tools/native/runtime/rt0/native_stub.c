/* native_stub.c — M4 stub-trap runtime (TARGET_NATIVE). See native_stub.h. */
#ifdef TARGET_NATIVE

#include "native_stub.h"
#include <stdio.h>
#include <stdlib.h>

int         g_nativeStubHit  = 0;
const char *g_nativeStubName  = 0;

void native_stub_hit(const char *name)
{
    g_nativeStubHit = 1;
    g_nativeStubName = name;
    fprintf(stderr, "NATIVE STUB HIT: %s (unimplemented INCLUDE_ASM function)\n", name);
    /* Trap with a sentinel exit so a host run that reaches unimplemented code is
     * reported as "hit a stub", not mistaken for a wrong-result test failure. */
    fflush(stderr);
    exit(NATIVE_STUB_EXIT);
}

#endif /* TARGET_NATIVE */

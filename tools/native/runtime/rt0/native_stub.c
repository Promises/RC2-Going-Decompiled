/* native_stub.c — M4 stub-trap runtime (TARGET_NATIVE). See native_stub.h. */
#ifdef TARGET_NATIVE

#include "native_stub.h"
#include <stdio.h>
#include <unistd.h>

int         g_nativeStubHit  = 0;
const char *g_nativeStubName  = 0;

void native_stub_hit(const char *name)
{
    g_nativeStubHit = 1;
    g_nativeStubName = name;
    fprintf(stderr, "NATIVE STUB HIT: %s (unimplemented INCLUDE_ASM function)\n", name);
    /* Trap with a sentinel exit so a host run that reaches unimplemented code is
     * reported as "hit a stub", not mistaken for a wrong-result test failure.
     * _exit, not exit (task #1489): cod/015180.c defines the game's own libc
     * exit() as matched C, and in a link that includes it (run_state_batch.sh
     * since its objects stopped colliding) it interposes over the host's.
     * exit() would then run the game's shutdown path into another trap and
     * recurse instead of ending the process with the sentinel. The flushes do
     * what exit()'s stdio teardown did. */
    fflush(stdout);
    fflush(stderr);
    _exit(NATIVE_STUB_EXIT);
}

#endif /* TARGET_NATIVE */

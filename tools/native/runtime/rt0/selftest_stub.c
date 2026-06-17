/* selftest_stub.c - M4 trap self-test (TARGET_NATIVE).
 *
 * Proves the weak stub table traps (not silent) when a host run reaches an
 * unimplemented INCLUDE_ASM function: link this + stubs.c + native_stub.c and
 * run. Expected: prints "NATIVE STUB HIT: <name>" and exits NATIVE_STUB_EXIT
 * (99). A harness reads the exit code / g_nativeStubName to flag "hit a stub"
 * rather than mistaking it for a wrong result. See docs/HLE.md M4.
 *
 * Build (in the colima native-build image):
 *   gcc -m32 -DTARGET_NATIVE -I../../  -I. selftest_stub.c stubs.c native_stub.c \
 *       -Wl,--gc-sections -ffunction-sections -o /tmp/selftest && /tmp/selftest
 */
#ifdef TARGET_NATIVE
#include "native_stub.h"
#include <stdio.h>

/* Stand-in for any still-asm game function (resolved by the weak stub table). */
extern void AppendGsRegPacket(void);

int main(void)
{
    printf("M4 self-test: calling a stubbed function; expect trap (exit %d)\n",
           NATIVE_STUB_EXIT);
    AppendGsRegPacket();   /* weak stub -> native_stub_hit -> exit, never returns */
    printf("FAIL: stub returned without trapping\n");
    return 1;
}
#endif

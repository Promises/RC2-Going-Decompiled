/* snd_null.c - 989snd IOP/SDK-boundary HLE no-op backend (TARGET_NATIVE).
 *
 * Headless has no IOP, so the 989snd RPC / command-ring service calls are no-ops.
 * These are the SDK boundary - NOT game logic - exactly like backend_null is for
 * GS/VIF/DMA, so they live in the runtime as strong defs that override the weak
 * M4 trap-stubs (rt0/stubs.c). They are deliberately NOT TARGET_NATIVE #else
 * bodies in the matching source: there is nothing to validate against the EE
 * (the EE side does real IOP RPC), and the EE-side game sound STATE is owned by
 * the matched / #else game C (e.g. StopAllSoundEmitters clears its own slots).
 *
 * Reached snd-RPC primitives, no-op'd as they get reached (one-frame-headless):
 *   snd_Pump          - the main service pump. Returns 0 so the drain loops
 *                       (`while (snd_Pump()) {}`) exit immediately - no IOP work
 *                       is ever pending headless.
 *   func_00133230     - removed (task #1460): cod/0321A0.c's plain-C body
 *                       clears D_001A74C4 and returns snd_Pump(), which is 0
 *                       here, so it already behaves as this no-op did.
 *   func_00132AC8,
 *   snd_PlaySample    - removed (task #1489, RULING #9179): cod/0321A0.c defines
 *                       both as matched plain C, and a stub here shadowed that
 *                       C. Their real bodies now run natively and queue into
 *                       snd_QueueCommandToRing (tree C). On the EE that ring
 *                       is drained by the real snd_Pump; the no-op above never
 *                       drains it, so a ring that fills (0x100 entries or
 *                       0xFFC bytes) would spin snd_QueueCommandToRing's wait.
 *                       No measured run reaches that. The IOP-side primitives
 *                       below the ring live in iop_null.c.
 */
#ifdef TARGET_NATIVE
#include "common.h"

s32 snd_Pump(void)        { return 0; }

#endif /* TARGET_NATIVE */

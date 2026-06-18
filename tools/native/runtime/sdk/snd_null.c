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
 *   func_00133230     - ring-service flush wrapper (sets the snd-internal
 *                       bookkeeping word D_001A74C4 - never read outside the
 *                       989snd unit - then pumps). Drop both; return 0.
 *   func_00132AC8     - snd_QueueCommandToRing(0x18) wrapper; nothing reaches the
 *                       (absent) IOP, so dropping the queue is a no-op.
 */
#ifdef TARGET_NATIVE
#include "common.h"

s32 snd_Pump(void)        { return 0; }
s32 func_00133230(void)   { return 0; }
s32 func_00132AC8(void)   { return 0; }

#endif /* TARGET_NATIVE */

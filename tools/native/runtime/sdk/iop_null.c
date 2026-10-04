/* iop_null.c - EE-kernel cache / libcdvd HLE for the 989snd layer (TARGET_NATIVE).
 *
 * RULING #9179 moved the native HLE boundary down from eight game/989snd
 * functions (FadeOutToBlackBlocking, func_00132AC8, func_00133250,
 * SetSndPumpCallback, snd_BankLoadAsync, snd_PlaySample,
 * StartFileLoadPumpingVoice, StopDialogVoice) to the primitives their real C
 * actually reaches below the 989snd EE layer. Task #1489 derived this set by
 * executing those eight bodies natively and recording which undefined symbol
 * each one trapped on. These three are the trapping primitives. All are
 * raw INCLUDE_ASM with no C anywhere in the tree, and none is game logic:
 *
 *   func_0011B3D0  SyncDCache(start, end): handwritten. Masks interrupts via
 *                  COP0 Status, aligns the range to 64 bytes and runs
 *                  func_0011B328's `cache 0x14` (D-cache writeback+invalidate)
 *                  loop. Reached by snd_SetupDmaTransfer and CdStartRead
 *                  staging SIF DMA buffers. A host is cache-coherent: no-op.
 *   func_0011B500  InvalidDCache(start, end): the same wrapper around
 *                  func_0011B458's `cache 0x16` (invalidate only) loop. Reached
 *                  by snd_CheckLoadInProgress before it re-reads the IOP-DMA'd
 *                  status word D_001A7100. Host-coherent: no-op. (0321A0.c's
 *                  comment calls it "poll IOP load status into dst"; it moves
 *                  no data itself, the IOP's DMA does.)
 *   func_00124B88  sceCdSync(mode), libcdvd: mode != 0 returns 1 while the CD
 *                  busy flag D_001363B0 is set or the CD RPC client is busy,
 *                  else 0; mode 0 blocks until both clear. Headless has no CD
 *                  command in flight, so it returns 0 (idle). That is also
 *                  what the ROM body returns on a zero arena. Reached by
 *                  snd_CheckLoadInProgress and func_001245D0 on their
 *                  g_sndIopReady == 0 fallback paths.
 *
 * NOT stubbed, deliberately:
 *   - snd_QueueCommandToRing, snd_SendCommandSync, sceSifCheckStatRpc and
 *     CdStartRead have C in the tree, so stubbing them would re-create one
 *     layer down the shadowing #9179 forbids.
 *   - func_001253A8 (sceCdRead) is reachable only statically, through
 *     CdStartRead's IOP-down fallback. No execution reached it, and a null
 *     read would report a load that delivered no data. It stays a trap.
 * The 989snd service pump snd_Pump (snd_null.c) and sceSifCallRpc
 * func_0011D620 (hw/backend_null.c) were already at this layer and are
 * unchanged.
 */
#ifdef TARGET_NATIVE
#include "common.h"

void func_0011B3D0(void) {}                /* SyncDCache - host is coherent */
void func_0011B500(void) {}                /* InvalidDCache - host is coherent */
s32  func_00124B88(void) { return 0; }     /* sceCdSync - 0 = no CD command in flight */

#endif /* TARGET_NATIVE */

/*
 * state_seed_test.c — host-side state-seeding cross-validation (native twin of
 * the EE run_state_suite). Links against the M2 globals arena, memcpy's a PINE
 * snapshot (state/*.bin, keyed by ROM address) into g_dataArena at the mapped
 * offsets, and reads globals back through their arena symbols — proving the
 * decomper's arena placement and the snapshot<->arena mapping on the host.
 *
 * Arena: g_dataArena == __gamedata_start; symbol X lives at
 * g_dataArena + (rom_addr(X) - ARENA_BASE). Snapshot region at ROM R seeds to
 * g_dataArena + (R - ARENA_BASE). ARENA_BASE is the generated arena's ROM base
 * (arena_map.txt header). It moves when gen_arena.py places a lower global, so
 * it is derived here from a ROM-named anchor, never hardcoded (task #1419).
 */
#include <stdio.h>

extern unsigned char g_dataArena[];   /* == __gamedata_start (arena.ld) */
extern int g_nGameState;
extern int g_boltCount;
extern unsigned char D_138180[];      /* ROM 0x138180 by its name: the anchor */

#define ARENA_BASE (0x138180u - (unsigned)(D_138180 - g_dataArena))

static int seed(const char *path, unsigned rom, unsigned len)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) { printf("  FAIL: cannot open %s\n", path); return -1; }
    unsigned n = (unsigned)fread(g_dataArena + (rom - ARENA_BASE), 1, len, fp);
    fclose(fp);
    printf("  seeded %-34s rom=0x%06X -> arena+0x%05X (%u bytes)\n",
           path, rom, rom - ARENA_BASE, n);
    return 0;
}

int main(void)
{
    printf("== seeding arena from New Game snapshot ==\n");
    seed("tools/ee/eetest/state/globals.bin", 0x1A7000, 0xC000);
    seed("tools/ee/eetest/state/input.bin",   0x138300, 0x400);
    printf("== read back through arena symbols ==\n");
    printf("  g_nGameState = %d  (expect 4)\n", g_nGameState);
    printf("  g_boltCount  = %d  (expect 0)\n", g_boltCount);
    int ok = (g_nGameState == 4 && g_boltCount == 0);
    printf("RESULT: %s\n", ok ? "PASS - host reads real seeded state via arena"
                              : "FAIL - arena/snapshot mapping mismatch");
    return ok ? 0 : 1;
}

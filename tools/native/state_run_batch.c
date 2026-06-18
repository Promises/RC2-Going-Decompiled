/*
 * state_run_batch.c — host-side batch runner / trap-stub fuzzer. Seeds the M2
 * arena from a PINE snapshot, then runs each target in its OWN forked child (a
 * trap-stub does exit(99), so one hit can't kill the batch). Per function:
 *   [clean]  ran to completion        [STUB ] hit a trap-stub (name on stderr)
 *   [CRASH]  signal (bad deref)        [exitN] other
 * STUB/CRASH/link-undefined are the decomper's next-priority surface.
 *
 * This batch is biased toward functions that CALL into the engine/libc surface
 * (draws, ctors, screen/dialog init) — those trip exit 99 when a shim is
 * missing, which is the real fuzzing signal. Leaf getters of placed globals are
 * already proven clean.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

extern unsigned char g_dataArena[];
#define ARENA_BASE 0x138180u

/* void(void) — best stub-trippers (no arg deref before the engine call) */
extern void func_00336648(void);
extern void func_00337C48(void);
extern void func_003396E0(void);
extern void func_00339A80(void);
extern void func_00339F90(void);
extern void func_003426D8(void);
extern void func_00343330(void);
/* void(void*) — pass a zeroed buffer; may trip a stub or crash on a null field */
extern void func_0033A7A8(void *p);
extern void func_00342670(void *p);
extern void func_00343558(void *p);

static unsigned char zbuf[0x800] __attribute__((aligned(16)));

static void seed(const char *p, unsigned rom, unsigned n) {
    FILE *f = fopen(p, "rb");
    if (f) { (void)!fread(g_dataArena + (rom - ARENA_BASE), 1, n, f); fclose(f); }
}

static void report(int st, const char *name) {
    if (WIFEXITED(st)) {
        int e = WEXITSTATUS(st);
        if (e == 99) printf("  [STUB ] %-14s -> trap-stub (name on stderr) — DECOMPER PRIORITY\n", name);
        else if (e != 0) printf("  [exit%d] %-14s\n", e, name);
    } else if (WIFSIGNALED(st)) {
        printf("  [CRASH] %-14s signal %d (null/heap deref — needs real object / EE-only)\n", name, WTERMSIG(st));
    }
}

/* fflush before fork so children don't inherit (and re-flush) buffered output */
#define RUN0(fn) do { fflush(stdout); pid_t _p = fork(); \
    if (_p == 0) { fn(); printf("  [clean] %-14s\n", #fn); fflush(stdout); _exit(0); } \
    int _s; waitpid(_p, &_s, 0); report(_s, #fn); fflush(stdout); } while (0)
#define RUNP(fn) do { fflush(stdout); pid_t _p = fork(); \
    if (_p == 0) { fn(zbuf); printf("  [clean] %-14s\n", #fn); fflush(stdout); _exit(0); } \
    int _s; waitpid(_p, &_s, 0); report(_s, #fn); fflush(stdout); } while (0)

int main(void) {
    seed("tools/ee/eetest/state/globals.bin", 0x1A7000, 0xC000);
    seed("tools/ee/eetest/state/input.bin",   0x138300, 0x400);
    printf("== native stub-surface fuzz batch (seeded New Game) ==\n");
    fflush(stdout);
    RUN0(func_00336648); RUN0(func_00337C48); RUN0(func_003396E0);
    RUN0(func_00339A80); RUN0(func_00339F90); RUN0(func_003426D8);
    RUN0(func_00343330);
    RUNP(func_0033A7A8); RUNP(func_00342670); RUNP(func_00343558);
    printf("== batch done ==\n");
    return 0;
}

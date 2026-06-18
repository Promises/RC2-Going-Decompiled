/*
 * state_run_batch.c — host-side batch runner / trap-stub fuzzer. Seeds the M2
 * arena from a PINE snapshot, then runs each target function in its OWN forked
 * child (a trap-stub does exit(99), so one stub-hit can't kill the batch).
 * Per function reports: [clean] ret=N | [STUB] hit a trap-stub (name on stderr)
 * | [CRASH] signal | [unplaced] (link-time). The stub/crash cases are exactly
 * the decomper's next-priority surface.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

extern unsigned char g_dataArena[];
#define ARENA_BASE 0x138180u

/* targets: matched/functional-equiv readers of arena-placed globals */
extern int GetPrevGameState(void);
extern int GetGameStateStackTop(void);
extern int GetSavePromptPending(void);

static void seed(const char *p, unsigned rom, unsigned n) {
    FILE *f = fopen(p, "rb");
    if (f) { (void)!fread(g_dataArena + (rom - ARENA_BASE), 1, n, f); fclose(f); }
}

#define RUN(fn) do {                                                          \
    pid_t _pid = fork();                                                      \
    if (_pid == 0) { int _r = fn(); printf("  [clean] %-24s ret=%d\n", #fn, _r);\
                     fflush(stdout); _exit(0); }                              \
    int _st; waitpid(_pid, &_st, 0);                                          \
    if (WIFEXITED(_st)) { int _e = WEXITSTATUS(_st);                          \
        if (_e == 99) printf("  [STUB ] %-24s -> trap-stub (name on stderr) — decomper priority\n", #fn); \
        else if (_e != 0) printf("  [exit%d] %-24s\n", _e, #fn); }            \
    else if (WIFSIGNALED(_st)) printf("  [CRASH] %-24s signal %d (likely heap-pointer deref — EE-only)\n", #fn, WTERMSIG(_st)); \
  } while (0)

int main(void) {
    seed("tools/ee/eetest/state/globals.bin", 0x1A7000, 0xC000);
    seed("tools/ee/eetest/state/input.bin",   0x138300, 0x400);
    printf("== native state batch (seeded New Game snapshot) ==\n");
    fflush(stdout);   /* empty the buffer before forking, or children re-flush it */
    RUN(GetPrevGameState);
    RUN(GetGameStateStackTop);
    RUN(GetSavePromptPending);
    printf("== batch done ==\n");
    return 0;
}

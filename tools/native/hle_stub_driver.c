/* hle_stub_driver.c — execution driver for check_hle_stubs.sh (task #1556).
 *
 * Calls each body whose HLE stubs live in runtime/sdk/iop_null.c (RULING #9179:
 * the eight functions that used to be stubbed themselves and now run their real
 * tree C) in a forked child, and prints one machine-readable ROW per body:
 *
 *   ROW <arena> <body> <outcome> <detail> <reached>
 *
 *   outcome  CLEAN | STUB | CRASH | TIMEOUT | EXIT
 *   detail   STUB: the trapped symbol; CRASH: "sig<N>@<symbol+off>";
 *            EXIT: the exit code; otherwise "-"
 *   reached  comma list of the declared HLE stubs the body called before it
 *            ended (or "-"), in first-call order
 *
 * "reached" comes from check_hle_stubs.sh's generated -Wl,--wrap=<stub>
 * wrappers, which call hle_note_reach() before the stub itself. A STUB outcome
 * comes from native_stub_hit()'s "NATIVE STUB HIT: <name>" line and exit 99.
 * A child that faults is caught by an SA_SIGINFO handler that names the
 * faulting pc with dladdr (the executable is linked -rdynamic) and re-raises,
 * so the parent still sees the signal. The fork is what keeps one faulting
 * body (FadeOutToBlackBlocking faults on a zero arena) from taking down the
 * run. alarm(3) is the hang guard.
 *
 * The arena is zero unless seed regions are given on the command line:
 *   hle_stub_driver <arena-label> [<file> <rom-addr> <length>]...
 * A region file of the wrong size, or a region outside the arena block, is a
 * hard error (exit 3), never a silent partial seed. The driver leaves with
 * _exit, never exit(): cod/015180.c defines the game's own libc exit(), which
 * interposes over the host's once that unit links, and would run game code
 * (native_stub.c, task #1489, has the same fix). (gen_batch.py's seed()
 * ignores a missing file. Here, check_hle_stubs.sh decides whether the seeded
 * arena exists before it calls this.)
 *
 * ARENA_BASE / ARENA_SPAN come from arena_map.txt's header via -D, as
 * gen_batch.py reads them, so they are never hardcoded here.
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ucontext.h>
#include <unistd.h>

#ifndef ARENA_BASE
#error "ARENA_BASE must be passed with -D (from arena_map.txt's header)"
#endif
#ifndef ARENA_SPAN
#error "ARENA_SPAN must be passed with -D (from arena_map.txt's header)"
#endif

extern unsigned char g_dataArena[];

/* The bodies, declared without a prototype's argument list: each is called
 * through Body with eight word arguments. Under the i386 cdecl convention
 * the caller pops the arguments, so a callee taking fewer reads only the
 * ones it declares (snd_PlaySample takes the most, 6+). */
typedef long (*Body)(long, long, long, long, long, long, long, long);
extern void FadeOutToBlackBlocking(void);
extern void func_00132AC8(void);
extern void func_00133250(void);
extern void SetSndPumpCallback(void);
extern void snd_BankLoadAsync(void);
extern void snd_PlaySample(void);
extern void StartFileLoadPumpingVoice(void);
extern void StopDialogVoice(void);

typedef struct {
    const char *name;
    void (*fn)(void);
    long args[8];
} Subject;

/* Argument vectors are #1535's: all zero except FadeOutToBlackBlocking(2)
 * and StartFileLoadPumpingVoice(0, 0, 1). One vector per body, so a
 * verdict covers the paths these inputs take, not every path. */
static const Subject subjects[] = {
    { "FadeOutToBlackBlocking",    FadeOutToBlackBlocking,    { 2 } },
    { "func_00132AC8",             func_00132AC8,             { 0 } },
    { "func_00133250",             func_00133250,             { 0 } },
    { "SetSndPumpCallback",        SetSndPumpCallback,        { 0 } },
    { "snd_BankLoadAsync",         snd_BankLoadAsync,         { 0 } },
    { "snd_PlaySample",            snd_PlaySample,            { 0 } },
    { "StartFileLoadPumpingVoice", StartFileLoadPumpingVoice, { 0, 0, 1 } },
    { "StopDialogVoice",           StopDialogVoice,           { 0 } },
};
#define NSUBJECTS (int)(sizeof(subjects) / sizeof(subjects[0]))

/* Called by the generated __wrap_<stub> before it runs the declared stub. */
void hle_note_reach(const char *name)
{
    char line[160];
    int n = snprintf(line, sizeof(line), "HLE-REACH %s\n", name);
    (void)!write(2, line, (size_t)n);
}

/* Leaves with code, flushing stdio first; see the header on why not exit(). */
static void die(int code)
{
    fflush(stdout);
    fflush(stderr);
    _exit(code);
}

static void on_fault(int sig, siginfo_t *si, void *ctx)
{
    ucontext_t *uc = (ucontext_t *)ctx;
    void *pc = (void *)uc->uc_mcontext.gregs[REG_EIP];
    Dl_info info;
    char line[256];
    int n;

    if (dladdr(pc, &info) && info.dli_sname)
        n = snprintf(line, sizeof(line), "HLE-CRASH sig%d@%s+0x%lx addr=%p\n", sig,
                     info.dli_sname, (unsigned long)((char *)pc - (char *)info.dli_saddr),
                     si->si_addr);
    else
        n = snprintf(line, sizeof(line), "HLE-CRASH sig%d@%p addr=%p\n", sig, pc, si->si_addr);
    (void)!write(2, line, (size_t)n);
    signal(sig, SIG_DFL);
    raise(sig);
}

static void seed_region(const char *path, unsigned long rom, unsigned long len)
{
    struct stat st;
    FILE *f;

    if (rom < ARENA_BASE || rom + len > (unsigned long)ARENA_BASE + ARENA_SPAN) {
        fprintf(stderr, "hle_stub_driver: region %s @0x%lX+0x%lX lies outside the arena block\n",
                path, rom, len);
        die(3);
    }
    if (stat(path, &st) != 0 || (unsigned long)st.st_size != len) {
        fprintf(stderr, "hle_stub_driver: %s is missing or not 0x%lX bytes\n", path, len);
        die(3);
    }
    f = fopen(path, "rb");
    if (!f || fread(g_dataArena + (rom - ARENA_BASE), 1, len, f) != len) {
        fprintf(stderr, "hle_stub_driver: short read from %s\n", path);
        die(3);
    }
    fclose(f);
}

/* Appends name to the comma list in out unless it is already there. */
static void add_unique(char *out, size_t cap, const char *name)
{
    size_t nl = strlen(name);
    const char *p = out;

    while ((p = strstr(p, name)) != NULL) {
        if ((p == out || p[-1] == ',') && (p[nl] == ',' || p[nl] == '\0'))
            return;
        p += nl;
    }
    if (strlen(out) + nl + 2 > cap)
        return;
    if (*out)
        strcat(out, ",");
    strcat(out, name);
}

static void run_one(const char *arena, const Subject *s)
{
    static char buf[1 << 16];
    char reached[512] = "", detail[256] = "-", name[160];
    const char *outcome = "?";
    int pp[2], st, len = 0;
    ssize_t got;
    pid_t pid;
    char *line, *save;

    if (pipe(pp) != 0) {
        perror("pipe");
        die(3);
    }
    pid = fork();
    if (pid < 0) {
        perror("fork");
        die(3);
    }
    if (pid == 0) {
        struct sigaction sa;
        const long *a = s->args;

        dup2(pp[1], 1);
        dup2(pp[1], 2);
        close(pp[0]);
        close(pp[1]);
        memset(&sa, 0, sizeof(sa));
        sa.sa_sigaction = on_fault;
        sa.sa_flags = SA_SIGINFO;
        sigaction(SIGSEGV, &sa, NULL);
        sigaction(SIGBUS, &sa, NULL);
        sigaction(SIGILL, &sa, NULL);
        sigaction(SIGFPE, &sa, NULL);
        alarm(3);
        ((Body)s->fn)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
        fflush(stdout);
        _exit(0);
    }
    close(pp[1]);
    while (len < (int)sizeof(buf) - 1 && (got = read(pp[0], buf + len, sizeof(buf) - 1 - len)) > 0)
        len += (int)got;
    buf[len] = '\0';
    close(pp[0]);
    waitpid(pid, &st, 0);

    for (line = strtok_r(buf, "\n", &save); line; line = strtok_r(NULL, "\n", &save)) {
        if (sscanf(line, "HLE-REACH %159s", name) == 1)
            add_unique(reached, sizeof(reached), name);
        else if (sscanf(line, "NATIVE STUB HIT: %159s", name) == 1)
            snprintf(detail, sizeof(detail), "%s", name);
        else if (sscanf(line, "HLE-CRASH %159s", name) == 1)
            snprintf(detail, sizeof(detail), "%s", name);
    }

    if (WIFEXITED(st)) {
        int e = WEXITSTATUS(st);
        if (e == 0) {
            outcome = "CLEAN";
            strcpy(detail, "-");
        } else if (e == 99) {
            outcome = "STUB";
        } else {
            outcome = "EXIT";
            snprintf(detail, sizeof(detail), "%d", e);
        }
    } else if (WIFSIGNALED(st)) {
        int sig = WTERMSIG(st);
        if (sig == SIGALRM) {
            outcome = "TIMEOUT";
            strcpy(detail, "-");
        } else {
            outcome = "CRASH";
            if (strncmp(detail, "sig", 3) != 0)
                snprintf(detail, sizeof(detail), "sig%d", sig);
        }
    }
    printf("ROW %s %s %s %s %s\n", arena, s->name, outcome, detail, *reached ? reached : "-");
}

int main(int argc, char **argv)
{
    int i;

    if (argc < 2 || (argc - 2) % 3 != 0) {
        fprintf(stderr, "usage: %s <arena-label> [<file> <rom-addr> <length>]...\n", argv[0]);
        die(3);
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    for (i = 2; i < argc; i += 3)
        seed_region(argv[i], strtoul(argv[i + 1], NULL, 0), strtoul(argv[i + 2], NULL, 0));
    for (i = 0; i < NSUBJECTS; i++)
        run_one(argv[1], &subjects[i]);
    die(0);
    return 0;
}

/*
 * native_test.h — tiny assert/mock framework for TARGET_NATIVE functional tests.
 *
 * A test file is a standalone ILP32 program: it #includes this header, defines
 * any mocks for the MIPS callees its function-under-test reaches (see the
 * recording helpers below), and provides main() that calls the function and
 * CHECK()s the observable results. The runner (run_test.sh) compiles the unit
 * under test + the test file together with -ffunction-sections/--gc-sections,
 * so only the function-under-test and its real callees are linked — you only
 * mock what that function actually calls.
 *
 * Exit status is the number of failed checks (0 = pass), so the runner can gate
 * on it directly.
 */
#ifndef NATIVE_TEST_H
#define NATIVE_TEST_H

#include <stdio.h>
#include <string.h>

static int g_checks = 0;
static int g_fails  = 0;

#define CHECK(cond) do {                                                       \
    g_checks++;                                                                \
    if (!(cond)) { g_fails++;                                                  \
        printf("  FAIL %s:%d  CHECK(%s)\n", __FILE__, __LINE__, #cond); }      \
  } while (0)

#define CHECK_EQ(got, want) do {                                               \
    g_checks++; long _g = (long)(got), _w = (long)(want);                      \
    if (_g != _w) { g_fails++;                                                 \
        printf("  FAIL %s:%d  %s == %s  (got %ld, want %ld)\n",                \
               __FILE__, __LINE__, #got, #want, _g, _w); }                     \
  } while (0)

/* float compare with tolerance */
#define CHECK_FEQ(got, want, eps) do {                                         \
    g_checks++; double _g = (double)(got), _w = (double)(want);               \
    double _d = _g - _w; if (_d < 0) _d = -_d;                                 \
    if (_d > (eps)) { g_fails++;                                               \
        printf("  FAIL %s:%d  %s ~= %s  (got %g, want %g)\n",                  \
               __FILE__, __LINE__, #got, #want, _g, _w); }                     \
  } while (0)

/* Report + return: use as `return TEST_SUMMARY();` at the end of main(). */
static int TEST_SUMMARY(void) {
    printf("  %d checks, %d failed\n", g_checks, g_fails);
    return g_fails;
}

/* --- mock call recorder ---------------------------------------------------
 * For verifying a function forwards to the right callee with the right args,
 * a mock can log into this ring and the test can assert against it. */
typedef struct MockCall {
    const char *fn;     /* callee name */
    long a0, a1, a2, a3;/* up to 4 recorded args (cast to long) */
} MockCall;

static MockCall g_calls[64];
static int      g_ncalls = 0;

static void mock_record(const char *fn, long a0, long a1, long a2, long a3) {
    if (g_ncalls < (int)(sizeof(g_calls)/sizeof(g_calls[0]))) {
        MockCall *c = &g_calls[g_ncalls];
        c->fn = fn; c->a0 = a0; c->a1 = a1; c->a2 = a2; c->a3 = a3;
    }
    g_ncalls++;
}
#define MOCK_RECORD0(fn)             mock_record(fn,0,0,0,0)
#define MOCK_RECORD1(fn,a)           mock_record(fn,(long)(a),0,0,0)
#define MOCK_RECORD2(fn,a,b)         mock_record(fn,(long)(a),(long)(b),0,0)
#define MOCK_RECORD3(fn,a,b,c)       mock_record(fn,(long)(a),(long)(b),(long)(c),0)
#define MOCK_RECORD4(fn,a,b,c,d)     mock_record(fn,(long)(a),(long)(b),(long)(c),(long)(d))

#endif /* NATIVE_TEST_H */

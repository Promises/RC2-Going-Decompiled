# Native functional-test harness (`TARGET_NATIVE`)

Runs the `#else` bodies as ILP32 and asserts behaviour. The matching PS2 build
cannot catch defects in them — it compiles `INCLUDE_ASM`, so the C is inert.

    tools/native/check.sh [file.c]                       # Tier 1: compile-check
    tools/native/build_image.sh                          # Tier 2: one-time image
    tools/native/run_test.sh <unit.c> <tests/test_x.c>   # Tier 2: compile+run
    tools/native/check_hle_stubs.sh [--seed-drop=S]      # HLE stubs vs execution

Rationale, tier semantics and gotchas: PROCEDURE `native-target-two-tier-harness`.
Capturing ground truth from PCSX2: PROCEDURE `pcsx2-golden-trace-capture`
(`ORACLE.md` was migrated there). The globals arena: RULING
`native-arena-mechanic-b-linker-script`.

`check_hle_stubs.sh` checks the declared HLE stubs in `runtime/sdk/iop_null.c`
against what RULING #9179's eight bodies reach when EXECUTED, in both
directions, on a zero arena and on the seeded arena.
`runtime/link_native.sh` cannot answer that question: it links with
`--unresolved-symbols=ignore-all`. Exit codes are 0 PASS, 1 FAIL and
2 INCOMPLETE. 2 is what a tree without the gitignored seed files gets, and it
is not a pass. The script's header has the verdicts and the seeds.

# Native functional-test harness (`TARGET_NATIVE`)

Runs the `#else` bodies as ILP32 and asserts behaviour. The matching PS2 build
cannot catch defects in them — it compiles `INCLUDE_ASM`, so the C is inert.

    tools/native/check.sh [file.c]                       # Tier 1: compile-check
    tools/native/build_image.sh                          # Tier 2: one-time image
    tools/native/run_test.sh <unit.c> <tests/test_x.c>   # Tier 2: compile+run

Rationale, tier semantics and gotchas: PROCEDURE `native-target-two-tier-harness`.
Capturing ground truth from PCSX2: PROCEDURE `pcsx2-golden-trace-capture`
(`ORACLE.md` was migrated there). The globals arena: RULING
`native-arena-mechanic-b-linker-script`.

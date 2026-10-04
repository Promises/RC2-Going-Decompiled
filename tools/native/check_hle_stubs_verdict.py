#!/usr/bin/env python3
"""check_hle_stubs_verdict.py — the verdict half of check_hle_stubs.sh (task #1556).

Reads the driver's ROW lines and prints one verdict per finding. Every verdict
names the arena that produced it. check_hle_stubs.sh's header defines the
vocabulary; this file implements it. Exit codes: 0 PASS · 1 FAIL · 2 INCOMPLETE.

--selftest runs every verdict against recorded rows, so each one this script
can print is shown firing, and none of them can only ever pass. It also runs
seed_windows() on scratch seed dirs, so the snapshot read, the labelled
fallback and both refusals are each shown firing.

--seed-windows DIR prints the address and length of each seed file, read from
DIR/snapshot.json (task #1573).
"""
import argparse
import json
import os
import sys
import tempfile

SUBJECTS = 8           # hle_stub_driver.c's subject table
CANONICAL = ("zero", "seeded")

# The seed files the driver loads, by snapshot.json region name (gen_batch.py
# names each file <region>.bin).
SEED_REGIONS = ("globals", "input")
# gen_batch.py's windows for a seed dir that has no snapshot.json. They are
# right for the in-level and menu seeds (globals 0x15000) and WRONG for the
# canonical New Game seed (globals 0xC000), which is why they are only a
# fallback and are announced when taken.
FALLBACK_WINDOWS = {"globals": (0x1A7000, 0x15000), "input": (0x138300, 0x400)}


class SeedRefused(Exception):
    """The seed dir cannot be loaded as described; the message says why."""


def seed_windows(seed_dir):
    """Returns ([(file, addr, len)], source, note) for SEED_REGIONS in seed_dir.

    The address and length come from seed_dir/snapshot.json, the record of what
    was captured, exactly as gen_batch.py reads it. Without a snapshot.json the
    FALLBACK_WINDOWS are used and note says so. A seed file whose size is not
    its window's length raises SeedRefused, as does a snapshot.json that lacks a
    region or cannot be parsed. A missing seed file is not checked here: the
    caller reports the arena as absent.
    """
    snap = os.path.join(seed_dir, "snapshot.json")
    present = [r for r in SEED_REGIONS if os.path.exists(os.path.join(seed_dir, r + ".bin"))]
    note = None
    if os.path.exists(snap):
        try:
            regions = {r["name"]: (int(r["addr"]), int(r["len"]))
                       for r in json.load(open(snap)).get("regions", [])}
        except (ValueError, KeyError, TypeError, AttributeError) as e:
            raise SeedRefused("%s is not a readable snapshot (%s: %s)" % (snap, type(e).__name__, e))
        absent = [r for r in SEED_REGIONS if r not in regions]
        if absent:
            raise SeedRefused("%s has no region named %s" % (snap, ", ".join(absent)))
        source = "snapshot.json"
    else:
        regions = FALLBACK_WINDOWS
        source = "fallback"
        if present:
            note = ("FALLBACK: %s does not exist, so the seed windows are gen_batch.py's no-snapshot "
                    "fallback (%s); they are right for in-level seeds and wrong for the New Game seed"
                    % (snap, ", ".join("%s 0x%X+0x%X" % (r, a, n) for r, (a, n) in sorted(regions.items()))))
    out = []
    for r in SEED_REGIONS:
        addr, length = regions[r]
        path = os.path.join(seed_dir, r + ".bin")
        if r in present and os.path.getsize(path) != length:
            raise SeedRefused("%s is %d (0x%X) bytes but its window from %s is 0x%X+0x%X"
                              % (path, os.path.getsize(path), os.path.getsize(path), source, addr, length))
        out.append((r + ".bin", addr, length))
    return out, source, note


def parse_rows(lines):
    rows = []
    for ln in lines:
        f = ln.split()
        if len(f) == 6 and f[0] == "ROW":
            reached = [] if f[5] == "-" else f[5].split(",")
            rows.append({"arena": f[1], "body": f[2], "outcome": f[3],
                         "detail": f[4], "reached": reached})
    return rows


def parse_baseline(lines):
    """`<arena> <body> <trap>  # reason` -> {(arena, body, trap): reason}."""
    out = {}
    for ln in lines:
        text, _, reason = ln.partition("#")
        f = text.split()
        if not f:
            continue
        if len(f) != 3:
            raise SystemExit("hle_stub_traps.txt: malformed row: %r" % ln.rstrip())
        out[tuple(f)] = reason.strip()
    return out


def verdict(rows, declared, dropped, baseline, seeded, seed_paths, say):
    """Prints the verdicts through say(); returns the exit code."""
    fail = incomplete = False
    effective = [s for s in declared if s not in dropped]
    seeding = bool(dropped) or any(s for s in declared if s.startswith("+"))
    arenas = []
    for r in rows:
        if r["arena"] not in arenas:
            arenas.append(r["arena"])

    for a in arenas:
        bodies = [r["body"] for r in rows if r["arena"] == a]
        if len(bodies) != SUBJECTS:
            say("INCOMPLETE [%s] %d of %d bodies reported: %s" % (a, len(bodies), SUBJECTS, " ".join(bodies)))
            incomplete = True
    if "zero" not in arenas:
        say("INCOMPLETE [zero] the zero arena produced no rows")
        incomplete = True
    if seeded == "absent":
        say("SKIPPED (seeded arena unavailable:%s) -- unreachable here: MISSING/KNOWN/STALE on the "
            "seeded arena, and SUPERFLUOUS for any stub the zero arena does not reach (printed UNDETERMINED)"
            % (seed_paths or " ?"))
        incomplete = True
    elif seeded != "canonical":
        say("NONCANONICAL seeded arena (%s): its rows are labelled seeded-noncanonical and discharge "
            "neither the seeded baseline nor UNDETERMINED" % seeded)
        incomplete = True

    # Traps: the missing-stub direction.
    observed = set()
    for r in rows:
        if r["outcome"] != "STUB":
            continue
        key = (r["arena"], r["body"], r["detail"])
        observed.add(key)
        if key in baseline:
            say("KNOWN [%s] %s -> %s  (%s)" % (r["arena"], r["body"], r["detail"], baseline[key]))
        else:
            tag = " (a --seed-drop stub)" if r["detail"] in dropped else ""
            say("MISSING [%s] %s traps on %s, which is not a declared HLE stub%s"
                % (r["arena"], r["body"], r["detail"], tag))
            fail = True
    for key, reason in sorted(baseline.items()):
        if key in observed:
            continue
        if key[0] not in arenas:
            say("UNCHECKED [%s] baseline %s -> %s: that arena did not run" % key)
        elif seeding:
            say("NOT-JUDGED [%s] baseline %s -> %s did not fire: a seed is active" % key)
        else:
            say("STALE [%s] baseline %s -> %s did not fire; remove the row (RULING #7317)" % key)
            fail = True

    # Reach: the superfluous direction.
    for s in effective:
        name = s.lstrip("+")
        by = {}
        for r in rows:
            if name in r["reached"]:
                by.setdefault(r["arena"], []).append(r["body"])
        if by:
            say("NEEDED %s  %s" % (name, "; ".join("[%s] %s" % (a, ",".join(b)) for a, b in by.items())))
        elif seeded == "canonical" and "seeded" in arenas and "zero" in arenas:
            say("SUPERFLUOUS %s  reached by no body on [zero] or [seeded]" % name)
            fail = True
        else:
            say("UNDETERMINED %s  not reached on [%s]; the canonical seeded arena did not run, so it "
                "cannot be ruled superfluous" % (name, ",".join(arenas) or "-"))
            incomplete = True
    for s in dropped:
        hit = [r for r in rows if r["outcome"] == "STUB" and r["detail"] == s]
        if hit:
            say("DROPPED %s  (--seed-drop) trapped %d body run(s), listed as MISSING above" % (s, len(hit)))
        else:
            say("DROPPED %s  (--seed-drop) trapped NO body on [%s]: the drop is invisible on these arenas"
                % (s, ",".join(arenas) or "-"))

    # Bodies whose reach was cut short.
    for r in rows:
        if r["outcome"] != "CLEAN":
            say("UNFINISHED [%s] %s %s %s, reach so far: %s; anything after this point is unobserved"
                % (r["arena"], r["body"], r["outcome"], r["detail"], ",".join(r["reached"]) or "-"))

    if fail:
        say("#### check_hle_stubs: FAIL")
        return 1
    if incomplete:
        say("#### check_hle_stubs: INCOMPLETE (not a pass)")
        return 2
    say("#### check_hle_stubs: PASS")
    return 0


# --selftest -------------------------------------------------------------------
# The zero rows are this script's own output at master 5638f87f (task #1556, XPS).
# The seeded rows are illustrative, shaped after #1535's seeded column
# (snd_BankLoadAsync reaches InvalidDCache). They are fixtures, not
# measurements.
ZERO = """\
ROW zero FadeOutToBlackBlocking CRASH sig11@FadeOutToBlackBlocking+0xf8 -
ROW zero func_00132AC8 CLEAN - func_0011B3D0
ROW zero func_00133250 STUB snd_PrintError func_00124B88,func_0011B3D0
ROW zero SetSndPumpCallback CLEAN - func_00124B88
ROW zero snd_BankLoadAsync CLEAN - func_00124B88
ROW zero snd_PlaySample CRASH sig11@snd_QueueCommandToRing+0x103 -
ROW zero StartFileLoadPumpingVoice CRASH sig11@snd_QueueCommandToRing+0x103 -
ROW zero StopDialogVoice CLEAN - -
"""
SEEDED = ZERO.replace("ROW zero", "ROW seeded").replace(
    "ROW seeded snd_BankLoadAsync CLEAN - func_00124B88",
    "ROW seeded snd_BankLoadAsync CLEAN - func_00124B88,func_0011B500")
DECL = ["func_0011B3D0", "func_0011B500", "func_00124B88"]
BASE = {("zero", "func_00133250", "snd_PrintError"): "fixture",
        ("seeded", "func_00133250", "snd_PrintError"): "fixture"}
DROP_B3D0 = ZERO.replace("ROW zero func_00132AC8 CLEAN - func_0011B3D0",
                         "ROW zero func_00132AC8 STUB func_0011B3D0 func_0011B3D0").replace(
    "ROW zero func_00133250 STUB snd_PrintError func_00124B88,func_0011B3D0",
    "ROW zero func_00133250 STUB func_0011B3D0 func_0011B3D0")

CASES = [
    # name, rows, declared, dropped, baseline, seeded, expected rc, lines that must appear
    ("pass: both arenas, baseline matched", ZERO + SEEDED, DECL, [], BASE, "canonical", 0,
     ["NEEDED func_0011B500  [seeded] snd_BankLoadAsync", "KNOWN [zero] func_00133250 -> snd_PrintError",
      "#### check_hle_stubs: PASS"]),
    ("drop invisible without the seed: InvalidDCache", ZERO, DECL, ["func_0011B500"],
     {k: v for k, v in BASE.items() if k[0] == "zero"}, "absent", 2,
     ["DROPPED func_0011B500  (--seed-drop) trapped NO body on [zero]", "#### check_hle_stubs: INCOMPLETE"]),
    ("skipped: zero arena only", ZERO, DECL, [], {k: v for k, v in BASE.items() if k[0] == "zero"},
     "absent", 2,
     ["SKIPPED (seeded arena unavailable: tools/ee/eetest/state/globals.bin",
      "UNDETERMINED func_0011B500", "#### check_hle_stubs: INCOMPLETE"]),
    ("missing: drop SyncDCache", DROP_B3D0, DECL, ["func_0011B3D0"], BASE, "absent", 1,
     ["MISSING [zero] func_00132AC8 traps on func_0011B3D0",
      "MISSING [zero] func_00133250 traps on func_0011B3D0",
      "DROPPED func_0011B3D0  (--seed-drop) trapped 2 body run(s)", "#### check_hle_stubs: FAIL"]),
    ("missing: an unlisted trap", ZERO, DECL, [], {}, "absent", 1,
     ["MISSING [zero] func_00133250 traps on snd_PrintError"]),
    ("superfluous: an extra stub nobody reaches", ZERO + SEEDED, DECL + ["+func_001253A8"], [], BASE,
     "canonical", 1, ["SUPERFLUOUS func_001253A8"]),
    ("no false superfluous without the seed", ZERO, DECL + ["+func_001253A8"], [], BASE, "absent", 2,
     ["UNDETERMINED func_001253A8"]),
    ("stale: a baseline row that does not fire", ZERO + SEEDED, DECL, [],
     dict(list(BASE.items()) + [(("zero", "StopDialogVoice", "func_00999999"), "fixture")]), "canonical", 1,
     ["STALE [zero] baseline StopDialogVoice -> func_00999999"]),
    ("noncanonical seed is not a pass", ZERO + SEEDED.replace("ROW seeded", "ROW seeded-noncanonical"),
     DECL, [], BASE, "noncanonical:globals.bin=00000000", 1,
     ["NONCANONICAL seeded arena", "MISSING [seeded-noncanonical] func_00133250 traps on snd_PrintError",
      "UNCHECKED [seeded] baseline"]),
    ("short run is incomplete", "\n".join(ZERO.splitlines()[:5]), DECL, [],
     {k: v for k, v in BASE.items() if k[0] == "zero"}, "absent", 2,
     ["INCOMPLETE [zero] 5 of 8 bodies reported"]),
]


# Seed-window cases: (name, snapshot globals len or None for no snapshot.json,
# globals.bin size, whether input is in the snapshot, expected (globals len,
# source) or the text of the refusal). Each runs in a scratch seed dir.
WINDOW_CASES = [
    ("window: New Game seed reads 0xC000 from snapshot", 0xC000, 0xC000, True, (0xC000, "snapshot.json")),
    ("window: in-level seed reads 0x15000 from snapshot", 0x15000, 0x15000, True, (0x15000, "snapshot.json")),
    ("window: snapshot len disagrees with the file", 0x15000, 0xC000, True,
     "is 49152 (0xC000) bytes but its window from snapshot.json is 0x1A7000+0x15000"),
    ("window: no snapshot falls back, and says so", None, 0x15000, True, (0x15000, "fallback")),
    ("window: no snapshot, fallback len is wrong", None, 0xC000, True,
     "is 49152 (0xC000) bytes but its window from fallback is 0x1A7000+0x15000"),
    ("window: snapshot without an input region", 0xC000, 0xC000, False, "has no region named input"),
]


def window_case(snap_len, size, with_input, want):
    """Builds a scratch seed dir, reads its windows; returns (ok, what was seen)."""
    with tempfile.TemporaryDirectory() as d:
        with open(os.path.join(d, "globals.bin"), "wb") as f:
            f.write(bytes(size))
        with open(os.path.join(d, "input.bin"), "wb") as f:
            f.write(bytes(0x400))
        if snap_len is not None:
            regions = [{"name": "globals", "addr": 0x1A7000, "len": snap_len}]
            if with_input:
                regions.append({"name": "input", "addr": 0x138300, "len": 0x400})
            with open(os.path.join(d, "snapshot.json"), "w") as f:
                json.dump({"regions": regions}, f)
        try:
            wins, source, note = seed_windows(d)
        except SeedRefused as e:
            return isinstance(want, str) and want in str(e), "refused: %s" % e
        seen = "globals 0x%X from %s%s" % (wins[0][2], source, "; " + note if note else "")
        if isinstance(want, str):
            return False, seen
        # A fallback must announce itself; a snapshot read must not.
        return (wins[0][2], source) == want and (note is not None) == (source == "fallback"), seen


def selftest():
    bad = 0
    for name, snap_len, size, with_input, want in WINDOW_CASES:
        ok, seen = window_case(snap_len, size, with_input, want)
        bad += not ok
        print("%s  %-45s %s" % ("ok  " if ok else "FAIL", name, seen))
    for name, rows, decl, drop, base, seeded, want_rc, want_lines in CASES:
        out = []
        rc = verdict(parse_rows(rows.splitlines()), decl, drop, base, seeded,
                     " tools/ee/eetest/state/globals.bin tools/ee/eetest/state/input.bin", out.append)
        missing = [w for w in want_lines if not any(w in o for o in out)]
        ok = rc == want_rc and not missing
        bad += not ok
        print("%s  %-45s rc %d (want %d)%s" % ("ok  " if ok else "FAIL", name, rc, want_rc,
                                              "" if not missing else "  missing: %r" % missing))
        if not ok:
            print("\n".join("      " + o for o in out))
    total = len(WINDOW_CASES) + len(CASES)
    print("selftest: %d of %d cases pass -- %s" % (total - bad, total, "PASS" if not bad else "FAIL"))
    return 1 if bad else 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--seed-windows", metavar="DIR")
    ap.add_argument("--rows")
    ap.add_argument("--declared", default="")
    ap.add_argument("--added", default="")
    ap.add_argument("--dropped", default="")
    ap.add_argument("--baseline")
    ap.add_argument("--seeded", default="absent")
    ap.add_argument("--seed-paths", default="")
    a = ap.parse_args()
    if a.selftest:
        return selftest()
    if a.seed_windows is not None:
        # For check_hle_stubs.sh: one "<file> <addr> <len> <source>" line per
        # seed file, the fallback note (if any) on stderr; rc 2 on a refusal.
        try:
            wins, source, note = seed_windows(a.seed_windows)
        except SeedRefused as e:
            sys.stderr.write("check_hle_stubs: seed refused: %s\n" % e)
            return 2
        if note:
            sys.stderr.write(note + "\n")
        for f, addr, length in wins:
            print("%s 0x%X 0x%X %s" % (f, addr, length, source))
        return 0
    rows = parse_rows(open(a.rows).read().splitlines())
    declared = a.declared.split() + ["+" + s for s in a.added.split()]
    baseline = parse_baseline(open(a.baseline).read().splitlines())
    return verdict(rows, declared, a.dropped.split(), baseline, a.seeded, a.seed_paths, print)


if __name__ == "__main__":
    sys.exit(main())

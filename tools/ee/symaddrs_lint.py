#!/usr/bin/env python3
"""symaddrs_lint.py - will splat's REAL parser accept these symbol_addrs files?

WHY THIS EXISTS. splat aborts the whole split on a malformed symbol_addrs COMMENT,
and the failure is silent unless someone runs splat. Three such comments went
unnoticed for three weeks, during which no USA re-split was possible and every
newly-added symbol was inert. A batch that adds hundreds of entries can re-brick
it with one stray `;` or one `word:` token in prose.

WHAT IT DOES *NOT* DO: reimplement splat's grammar. It calls
`splat.util.symbols.handle_sym_addrs` itself. A second implementation of the
rules is a second predicate that drifts from the first, and drift between two
predicates that were meant to agree is the failure mode this whole area has been
made of. The lint's job is isolation, banding and a readable message - not
re-deriving what splat already decides.

🔴 ONE PROCESS PER FILE, ALWAYS. splat's symbol table is module-global: parsing
two files in one interpreter leaks state and produces a bogus "Duplicate symbol"
failure that looks exactly like a real defect. This runs each file in a fresh
subprocess. The selftest keeps a deliberately-contaminated invocation as a
fixture, so the isolation cannot silently regress.

🔴 THE CHILD MUST SET SPLAT UP THE WAY split.py DOES, OR IT GOES BLIND. For any
`type:X` attribute outside splat's own four (func/jtbl/jtbl_label/label) and not
capitalised, splat's check_valid_type asks the disassembler instance for its
known C types. split.py creates that instance from the loaded config before it
reads symbol_addrs; a child that skips this raises "Disassembler instance not
initialized" on the first `type:u32` and the file bands `??`. Measured on the
USA map (task #699, FACT #7992): lines 1-708 were checked and nothing after them
was, so a double-`;` at line 100 FAILed and the same line at EOF read `??`. The
child therefore loads the real splat config (--config, default USA) and creates
the instance with splat's own call. A failure to do that is could-not-look, never
a verdict about the file - and it is never skipped, because skipping it is the
blindness this paragraph describes.

exit 0 = every file parses / 1 = at least one rejected / 2 = could not look
         3 = usage
"""
import os
import subprocess
import sys

# The config whose disassembler setup the child reproduces. check_valid_type only
# consults the instance's known C types, which splat selects by `platform:`; both
# regions' configs say `platform: ps2`, so the USA default serves the EU map too.
DEFAULT_CONFIG = os.path.normpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..", "..", "going-decompiled", "config", "usa", "SCUS_972.68.yaml"))

# Run in a child so splat's global symbol table starts empty every time.
CHILD = r"""
import sys
from pathlib import Path
from splat.util import symbols
p = Path(sys.argv[1])
try:
    lines = open(p).readlines()
except OSError as e:
    print("CANNOTLOOK %s" % e, file=sys.stderr)
    raise SystemExit(64)
try:
    # Same order as splat.scripts.split.main: config, then disassembler instance.
    from splat import __version__
    from splat.util import conf
    from splat.disassembler import disassembler_instance
    conf.load([Path(sys.argv[2])])
    disassembler_instance.create_disassembler_instance(False, __version__)
except BaseException as e:
    # log.error raises SystemExit; a bad config path raises OSError. Either way
    # the file was not examined, so this is could-not-look - banding it 65 would
    # report a harness fault as a defect in the map.
    print("CANNOTLOOK splat setup from %s failed: %s: %s"
          % (sys.argv[2], type(e).__name__, e), file=sys.stderr)
    raise SystemExit(64)
try:
    symbols.handle_sym_addrs(p, lines, [])
except SystemExit as e:
    # splat's log.error path: message already on stderr.
    raise SystemExit(65 if e.code else 0)
except AssertionError as e:
    # splat's OTHER hard-exit path: `assert line.count(";") == 1`. It is a
    # CONTENT rejection, not a harness fault, and it does not raise SystemExit -
    # treating it as a crash bands a real defect as could-not-look. Caught by the
    # selftest before this shipped.
    print("AssertionError: %s" % e, file=sys.stderr)
    raise SystemExit(65)
print("OK %d" % len(lines))
"""

OK, REJECTED, CANNOTLOOK = 0, 1, 2

# splat writes a tqdm progress bar to stderr alongside its diagnostic, and the
# bar is usually LAST. Taking the final stderr line therefore reports a progress
# bar as the reason a file was rejected - a finding with no content. Prefer a
# line that actually names the fault.
_MARKERS = ("error reading", "Missing attribute", "AssertionError",
            "Duplicate symbol", "invalid", "Traceback")


def diagnostic(stderr: str, stdout: str = "") -> str:
    """The line(s) naming the fault, never the progress bar.

    🔴 splat SPLITS its diagnostic ACROSS STREAMS: the location goes to stderr
    ("error reading <file>, line 5") and the reason to stdout ("Missing
    attribute value in '(mis-record:'"). Reading only one gives a rejection with
    no stated cause - which is a finding nobody can act on. Measured: searching
    stderr alone yielded "no diagnostic emitted" on a file whose reason was
    sitting in stdout the whole time.
    """
    lines = [l.strip() for l in ((stderr or "") + "\n" + (stdout or "")).splitlines()
             if l.strip()]
    # 🔴 The location is written onto the SAME physical line as the progress bar
    # ("…?, ?it/s]error reading <file>, line 2000:"), so filtering bar lines before
    # looking for it drops the line number every time. Cut it out of any line first.
    where = next((l[l.lower().index("error reading"):] for l in lines
                  if "error reading" in l.lower()), "")
    real = [l for l in lines if "it/s" not in l]
    why = next((l for l in reversed(real)
                if any(m.lower() in l.lower() for m in _MARKERS)
                and not l.lower().startswith("error reading")), "")
    if where and why:
        return ("%s %s" % (where, why))[:200]
    return (where or why or (real[-1] if real else "rejected, no diagnostic emitted"))[:200]


def check(path: str, python: str, config: str):
    """Parse one file in a FRESH interpreter. Returns (band, detail)."""
    proc = subprocess.run(
        [python, "-c", CHILD, path, config],
        capture_output=True, text=True,
    )
    if proc.returncode == 0:
        # splat's setup prints its version banner first; the verdict is last.
        out = [l for l in proc.stdout.splitlines() if l.strip()]
        return OK, out[-1].strip() if out else ""
    if proc.returncode == 64:
        return CANNOTLOOK, proc.stderr.strip().splitlines()[-1] if proc.stderr else "unreadable"
    if proc.returncode == 65:
        return REJECTED, diagnostic(proc.stderr, proc.stdout)
    # Anything else is the harness failing, not a verdict about the file.
    tail = [l for l in (proc.stderr or proc.stdout).splitlines() if l.strip()]
    return CANNOTLOOK, "harness rc=%d %s" % (proc.returncode, tail[-1][:120] if tail else "")


def main(argv):
    if not argv:
        print(__doc__.strip().splitlines()[0], file=sys.stderr)
        print("usage: symaddrs_lint.py <symbol_addrs.txt> [more...] [--python PATH]"
              " [--config SPLAT_YAML]",
              file=sys.stderr)
        return 3
    python = sys.executable
    config = DEFAULT_CONFIG
    files = []
    it = iter(argv)
    for a in it:
        if a == "--python":
            python = next(it, None)
            if not python:
                print("--python needs a value", file=sys.stderr)
                return 3
        elif a == "--config":
            config = next(it, None)
            if not config:
                print("--config needs a value", file=sys.stderr)
                return 3
        else:
            files.append(a)
    if not files:
        print("no files given", file=sys.stderr)
        return 3

    # 🔴 LABEL EACH ROW WITH THE PATH AS GIVEN, NEVER Path(f).name. Every region's
    # map is called `symbol_addrs.txt`, and every extraction of one is usually
    # called `<region>_symbol_addrs.txt` - so a run comparing the same file at two
    # refs printed TWO rows reading `usa_symbol_addrs.txt`, one FAIL and one ok,
    # with nothing to say which ref each verdict belonged to. The predicate was
    # right and the SUBJECT was unstated. Measured 2026-07-26 while cross-checking
    # a staged batch against the map that bricked the split.
    width = min(max((len(f) for f in files), default=0), 60)
    rejected = cannot = 0
    for f in files:
        band, detail = check(f, python, config)
        if band == OK:
            print("  ok   %-*s %s" % (width, f, detail))
        elif band == REJECTED:
            print("  FAIL %-*s %s" % (width, f, detail))
            rejected += 1
        else:
            print("  ??   %-*s could not look: %s" % (width, f, detail))
            cannot += 1

    print("%d ok / %d rejected / %d unexaminable  (of %d)"
          % (len(files) - rejected - cannot, rejected, cannot, len(files)))
    if cannot:
        return CANNOTLOOK
    return REJECTED if rejected else OK


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

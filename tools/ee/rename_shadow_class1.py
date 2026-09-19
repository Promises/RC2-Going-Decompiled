#!/usr/bin/env python3
"""rename_shadow_class1.py — retire class-1 shadow rows (task #452, NOTE #7245 §3).

A class-1 row is `INCLUDE_ASM(dir, func_ADDR)` in src/<region> while
symbol_addrs/<region> names ADDR something else. splat emits nothing for that
function (its current name is not an INCLUDE_ASM argument), the tracked
func_ADDR.s is a frozen leftover, and the two spellings are held together only
by build.sh's PROVIDE line. The fix per row is the #442 P6(b) edit: rename the
macro argument AND every C reference to the old identifier in src/<region>,
re-split so <Name>.s is emitted, then delete the func_ADDR.s twin.

Two arms, because one carries a hazard the other does not (task #452):
  A  the new name is a project name — a rename is byte-neutral by construction
     (asm-only function; the identifier lives in relocations and PROVIDE).
  B  the new name is a libc / libgcc / SDK identifier — once a COMPILED caller
     spells `strlen(p)`, cc1 2.9 may recognise a builtin and change the caller's
     codegen. B rows are measured per caller in a separate task; this script
     only lists them.

Partition rule (each B row prints the rule that caught it):
  R1  the name is a word in include/rtl/** (the vendored SDK headers)
  R2  the name is a C-standard library identifier (C89/C99 header names below)
  R3  the name is reserved for the implementation: it begins with `_`
      (C99 7.1.3 — file-scope identifiers beginning with an underscore)
  R4  the name is in an SDK namespace prefix: sce* (Sony), snd_* (989snd)
  R5  the name is a libgcc / libm internal (frame.c, fp-bit.c, libgcc2.c,
      e_sqrt.c — the explicit list below)

Usage:
  rename_shadow_class1.py <region> --partition           print A/B rows + counts
  rename_shadow_class1.py <region> --apply A|B [--dry] [--exclude func_X,...]
                                                         substitute, print counts
Run from the repo root (or set ROOT=).  Substitution is word-bounded on
[^0-9A-Za-z_] both sides and confined to src/<region>/**; references to a
USA-address identifier inside src/eu (the #442 cross-region refs) are never
touched because the row set is derived per region.

Failure observables: a row with 0 INCLUDE_ASM substitutions (the row is not
where the scan said) prints `ZERO` and the script exits 1; a name caught by
two rules prints both.  Positive controls: --partition must place
SplitFrameCountToMinSecHundredths in A (control name, not a row) and strcmp in B.
"""
import os
import re
import subprocess
import sys

C_STANDARD = set("""
abort abs acos asctime asin atan atan2 atexit atof atoi atol atoll bsearch
calloc ceil clearerr clock cos cosh ctime difftime div exit exp fabs fclose
feof ferror fflush fgetc fgetpos fgets floor fmod fopen fprintf fputc fputs
fread free freopen frexp fscanf fseek fsetpos ftell fwrite getc getchar getenv
gets gmtime isalnum isalpha iscntrl isdigit isgraph islower isprint ispunct
isspace isupper isxdigit labs ldexp ldiv localeconv localtime log log10 longjmp
main malloc mblen mbstowcs mbtowc memchr memcmp memcpy memmove memset mktime
modf perror pow printf putc putchar puts qsort raise rand realloc remove rename
rewind scanf setbuf setjmp setlocale setvbuf signal sin sinh sprintf sqrt srand
sscanf strcat strchr strcmp strcoll strcpy strcspn strerror strftime strlen
strncat strncmp strncpy strpbrk strrchr strspn strstr strtod strtok strtol
strtoul strxfrm system tan tanh time tmpfile tmpnam tolower toupper ungetc
vfprintf vprintf vsprintf wcstombs wctomb
snprintf vsnprintf strtoll strtoull llabs lldiv fabsf sqrtf sinf cosf tanf
atan2f floorf ceilf fmodf powf expf logf
""".split())

# libgcc / libm internals that are not builtins but are library names
# (gcc/frame.c, gcc/config/fp-bit.c, gcc/libgcc2.c, libm/e_sqrt.c)
LIBGCC_INTERNAL = set("""
add_fdes count_fdes copy_reg dpadd dpsub dpcmp dptofp dpmul dpdiv litodp e_sqrt
end_fde_sort execute_cfa_insn extract_cie_info fde_merge fde_split
find_exception_handler old_find_exception_handler find_fde frame_init
get_reg_addr next_stack_level throw_helper new_eh_context eh_context_static
decode_uleb128 decode_sleb128
""".split())

SDK_PREFIX = ("sce", "snd_")

ROOT = os.environ.get("ROOT", ".")


def load_symbol_addrs(region):
    """ADDR (8 hex, upper) -> first name in symbol_addrs (mirrors splat)."""
    path = os.path.join(ROOT, "going-decompiled/symbol_addrs", region, "symbol_addrs.txt")
    pat = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;")
    names = {}
    with open(path) as f:
        for line in f:
            m = pat.match(line)
            if not m:
                continue
            addr = "%08X" % int(m.group(2), 16)
            names.setdefault(addr, m.group(1))
    return names


def c_files(region):
    src = os.path.join(ROOT, "going-decompiled/src", region)
    out = []
    for d, _, fs in os.walk(src):
        for f in fs:
            if f.endswith((".c", ".h", ".cpp")):
                out.append(os.path.join(d, f))
    return sorted(out)


INC = re.compile(r'INCLUDE_ASM(?:_FRAGMENT)?\("([^"]+)",\s*(func_[0-9A-Fa-f]{8})\s*\)')


def class1_rows(region):
    """[(file, line, old, new, dir)] for every INCLUDE_ASM(dir, func_ADDR) whose
    ADDR symbol_addrs names differently (the shadow_scan2.sh CLASS1 set)."""
    names = load_symbol_addrs(region)
    rows = []
    for path in c_files(region):
        with open(path) as f:
            for ln, line in enumerate(f, 1):
                m = INC.search(line)
                if not m:
                    continue
                old = m.group(2)
                new = names.get(old[5:].upper())
                if new and new != old:
                    rows.append((os.path.relpath(path, ROOT), ln, old, new, m.group(1)))
    return rows


_rtl_cache = {}


def rtl_hit(name):
    if name not in _rtl_cache:
        r = subprocess.run(
            ["git", "grep", "-lwE", name, "--", "going-decompiled/include/rtl"],
            cwd=ROOT, capture_output=True, text=True)
        _rtl_cache[name] = [l for l in r.stdout.splitlines() if l]
    return _rtl_cache[name]


def classify(name):
    """Return the list of B rules the name trips; empty list means arm A."""
    rules = []
    if rtl_hit(name):
        rules.append("R1:" + rtl_hit(name)[0])
    if name in C_STANDARD:
        rules.append("R2")
    if name.startswith("_"):
        rules.append("R3")
    if name.startswith(SDK_PREFIX):
        rules.append("R4")
    if name in LIBGCC_INTERNAL:
        rules.append("R5")
    return rules


def partition(region):
    rows = class1_rows(region)
    a, b = [], []
    for row in rows:
        rules = classify(row[3])
        (b if rules else a).append((row, rules))
    return a, b


def do_partition(region):
    a, b = partition(region)
    for row, rules in a:
        print("A %s:%d %s -> %s" % (row[0], row[1], row[2], row[3]))
    for row, rules in b:
        print("B %s:%d %s -> %s  [%s]" % (row[0], row[1], row[2], row[3], " ".join(rules)))
    print("A=%d B=%d total=%d" % (len(a), len(b), len(a) + len(b)))
    for ctl, want in (("SplitFrameCountToMinSecHundredths", "A"), ("strcmp", "B")):
        got = "B" if classify(ctl) else "A"
        print("CONTROL %s -> %s (expected %s) %s" % (ctl, got, want, "OK" if got == want else "FAIL"))
        if got != want:
            sys.exit(1)


def do_apply(region, arm, dry, exclude):
    a, b = partition(region)
    rows = [r for r, _ in (a if arm == "A" else b)]
    for r in rows:
        if r[2] in exclude:
            print("EXCLUDED %s:%d %s -> %s" % (r[0], r[1], r[2], r[3]))
    rows = [r for r in rows if r[2] not in exclude]
    files = c_files(region)
    text = {p: open(p).read() for p in files}
    bad = 0
    total = 0
    for path, ln, old, new, _ in rows:
        pat = re.compile(r"(?<![0-9A-Za-z_])" + re.escape(old) + r"(?![0-9A-Za-z_])")
        inc = 0
        refs = 0
        touched = []
        for p in files:
            n = len(pat.findall(text[p]))
            if not n:
                continue
            # count INCLUDE_ASM-line hits separately
            for line in text[p].splitlines():
                if pat.search(line):
                    if INC.search(line):
                        inc += 1
                    else:
                        refs += 1
            touched.append("%s:%d" % (os.path.relpath(p, ROOT), n))
            if not dry:
                text[p] = pat.sub(new, text[p])
        total += inc + refs
        flag = "" if inc else "  ZERO"
        if not inc:
            bad += 1
        print("%s %s -> %s inc=%d refs=%d files=%s%s" % (
            path, old, new, inc, refs, ",".join(touched), flag))
    if not dry:
        for p in files:
            with open(p, "w") as f:
                f.write(text[p])
    print("rows=%d substitutions=%d zero_rows=%d%s" % (len(rows), total, bad, " (dry)" if dry else ""))
    if bad:
        sys.exit(1)


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(2)
    region = sys.argv[1]
    if sys.argv[2] == "--partition":
        do_partition(region)
    elif sys.argv[2] == "--apply":
        arm = sys.argv[3]
        if arm not in ("A", "B"):
            sys.exit("arm must be A or B")
        exclude = set()
        if "--exclude" in sys.argv:
            exclude = set(sys.argv[sys.argv.index("--exclude") + 1].split(","))
        do_apply(region, arm, "--dry" in sys.argv, exclude)
    else:
        sys.exit("unknown mode")


if __name__ == "__main__":
    main()

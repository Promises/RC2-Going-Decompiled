#!/usr/bin/env python3
"""libgcc_transcription_scan.py — no GCC runtime source text in game C.

RULING #8206: libgcc lives as GCC's own files under going-decompiled/libgcc/,
linked as a library, never transcribed into a game .c. This is the executed
form of that rule for landing_gate.sh (task #893).

    libgcc_transcription_scan.py <src-dir>    e.g. going-decompiled/src/usa

Scans every .c under <src-dir> with comments and string/char literals removed
first, because doc comments legitimately DESCRIBE GCC's algorithm (at 56105532
every GCC-internal identifier in game C sat in a comment, none in code).
Describing is not transcribing; only code is judged.

Two rules, each printed per member as `<RULE> <file>:<line> <name>`. The
allowed spec one-liners are printed too, as KEEP-SPEC, so the exempt set is
visible by member and a change to it shows in the output; they do not fail.

  DEF    a DEFINITION (a body) of a libgcc.a entry point, by its libgcc name.
         `extern` declarations at call sites are not definitions and are
         allowed. ONE exception, stated here rather than left to a regex: a
         body that is exactly `return <param> <op> <param>;` with <op> one of
         * / % is the behavioural SPEC the portable build needs (`a * b` is not
         GCC's text — no GCC expression is three tokens) and is allowed, but
         ONLY inside a TARGET_NATIVE arm (`#ifdef TARGET_NATIVE`, or the #else
         of `#ifndef TARGET_NATIVE`). In the EE arm the member comes from the
         ROM's asm or from the library, never from a game body, so any EE-arm
         definition fails, one-liner or not.
  IDENT  any occurrence, in code (preprocessor lines included), of an
         identifier that exists only in GCC's libgcc2.c / longlong.h /
         fp-bit.c and cannot be recovered from the ROM (NOTE #8209 signal T2).

What this does NOT see (say it, so nobody reads a pass as more than it is):
  - a transcription placed under a splat name (func_<addr>) that avoids every
    listed identifier, e.g. fp-bit's __fpcmp_parts written as ternaries: the
    name is not a libgcc name and the tokens are ordinary C. NOTE #8209's T1
    verbatim-run screen is the instrument for that class, and it needs the GCC
    corpus beside it;
  - files other than .c (headers under include/ are not scanned).

Exit 0 clean / 1 a member fired / 2 could not run.
"""
import os
import re
import sys

# libgcc.a entry points that could be defined by their libgcc name: libgcc2.c's
# L_* modules plus the fp-bit/_eh/frame/__main globals in the ROM (NOTE #8217).
ENTRY = {
    "__muldi3", "__divdi3", "__moddi3", "__udivdi3", "__umoddi3",
    "__udivmoddi4", "__negdi2", "__lshrdi3", "__ashldi3", "__ashrdi3",
    "__ffsdi2", "__cmpdi2", "__ucmpdi2", "__fixunsdfdi", "__fixdfdi",
    "__fixunssfdi", "__fixsfdi", "__floatdidf", "__floatdisf",
    "__fixunsdfsi", "__fixunssfsi", "__main", "__do_global_ctors",
    "__do_global_dtors", "__pure_virtual", "__default_terminate",
    "__terminate", "__throw", "__rethrow", "__sjthrow", "__sjpopnthrow",
    "__throw_type_match", "__get_eh_context", "__get_eh_info",
    "__get_dynamic_handler_chain", "__eh_rtime_match", "__unwinding_cleanup",
    "__pack_d", "__unpack_d", "__pack_f", "__unpack_f", "__make_dp",
    "__make_fp", "_fpadd_parts", "_fpmul_parts", "_fpdiv_parts",
    "__fpcmp_parts_d", "__fpcmp_parts_f", "__frame_state_for",
    "__register_frame_info", "__deregister_frame_info",
}

# Identifiers that exist only in GCC's source (NOTE #8209 T2 list, extended
# with the fp-bit.c structure/field names and longlong.h macros).
GCC_ONLY = {
    "DIunion", "umul_ppmm", "udiv_qrnnd", "__udiv_qrnnd_c", "sub_ddmmss",
    "add_ssaaaa", "count_leading_zeros", "count_trailing_zeros",
    "__umulsidi3", "__ll_lowpart", "__ll_highpart", "__ll_B", "__BITS4",
    "UDIV_NEEDS_NORMALIZATION", "SI_TYPE_SIZE", "W_TYPE_SIZE",
    "UQItype", "SItype", "USItype", "DItype", "UDItype", "DFtype", "SFtype",
    "word_type", "fp_number_type", "FLO_union_type", "FLO_type",
    "fractype", "halffractype", "intfrac", "tfraction", "a_normal_exp",
    "b_normal_exp", "a_fraction", "b_fraction", "NO_DENORMALS", "FRACBITS",
    "NGARDS", "GARDMASK", "GARDMSB", "GARDROUND", "IMPLICIT_1",
    "IMPLICIT_2", "LSHIFT", "FP_LSHIFT", "EXPBIAS", "EXPMAX", "QUIET_NAN",
    "CLASS_SNAN", "CLASS_QNAN", "CLASS_ZERO", "CLASS_NUMBER",
    "CLASS_INFINITY",
}

TOKEN = re.compile(r"[A-Za-z_]\w*|\d\w*|->|\+\+|--|<<|>>|[^\s\w]")


def strip(text):
    """Blank comments and string/char literals, keeping every newline so line
    numbers survive."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(re.sub(r"[^\n]", " ", text[i:j])); i = j
        elif text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i)); i = j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            out.append(c + re.sub(r"[^\n]", " ", text[i + 1:j - 1]) + c); i = j
        else:
            out.append(c); i += 1
    return "".join(out)


def native_arm(stack):
    """True when the current line is compiled only by the TARGET_NATIVE build."""
    return any(kind == "native" and live for kind, live in stack)


def scan_file(path, rel, hits, keep):
    with open(path, encoding="utf-8", errors="replace") as f:
        lines = strip(f.read()).split("\n")
    # Pass 1: preprocessor arms and IDENT hits on every line; code tokens with
    # their line number and arm collected for pass 2.
    stack, toks = [], []  # stack rows: (kind, live) kind "native"|"other"
    for ln, line in enumerate(lines, 1):
        for name in re.findall(r"[A-Za-z_]\w*", line):
            if name in GCC_ONLY:
                hits.append(("IDENT", rel, ln, name))
        m = re.match(r"\s*#\s*(\w+)\s*(.*)", line)
        if m:
            d, arg = m.group(1), m.group(2).strip()
            if d in ("if", "ifdef", "ifndef"):
                if re.fullmatch(r"(defined\s*\(?\s*)?TARGET_NATIVE\s*\)?", arg) and d != "ifndef":
                    stack.append(("native", True))
                elif d == "ifndef" and arg == "TARGET_NATIVE":
                    stack.append(("native", False))
                elif d == "if" and re.fullmatch(r"!\s*defined\s*\(?\s*TARGET_NATIVE\s*\)?", arg):
                    stack.append(("native", False))
                else:
                    stack.append(("other", True))
            elif d in ("else", "elif") and stack:
                kind, live = stack[-1]
                stack[-1] = (kind, not live) if kind == "native" else (kind, True)
            elif d == "endif" and stack:
                stack.pop()
            continue
        nat = native_arm(stack)
        for t in TOKEN.findall(line):
            toks.append((t, ln, nat))
    # Pass 2: top-level `NAME ( ... ) {` is a definition; collect its body.
    depth, i, n = 0, 0, len(toks)
    while i < n:
        t = toks[i][0]
        if t == "{":
            depth += 1
        elif t == "}":
            depth -= 1
        elif depth == 0 and t in ENTRY and i + 1 < n and toks[i + 1][0] == "(":
            j, par = i + 1, 0
            while j < n:
                par += toks[j][0] == "("
                par -= toks[j][0] == ")"
                if par == 0:
                    break
                j += 1
            params = [x[0] for x in toks[i + 2:j]]
            if j + 1 < n and toks[j + 1][0] == "{":
                k, b = j + 1, 0
                while k < n:
                    b += toks[k][0] == "{"
                    b -= toks[k][0] == "}"
                    if b == 0:
                        break
                    k += 1
                body = [x[0] for x in toks[j + 2:k]]
                names = set(p for p in params if re.fullmatch(r"[A-Za-z_]\w*", p))
                spec = (len(body) == 5 and body[0] == "return" and body[4] == ";"
                        and body[2] in ("*", "/", "%")
                        and body[1] in names and body[3] in names)
                if spec and toks[i][2]:
                    keep.append(("KEEP-SPEC", rel, toks[i][1], t))
                else:
                    why = "EE-arm" if not toks[i][2] else "not-a-spec-one-liner"
                    hits.append(("DEF", rel, toks[i][1], f"{t} ({why})"))
                i = k + 1
                continue
        i += 1


def main():
    if len(sys.argv) != 2 or not os.path.isdir(sys.argv[1]):
        print(__doc__.split("\n\n")[1], file=sys.stderr)
        return 2
    root = sys.argv[1]
    hits, keep, files = [], [], 0
    for d, _, fs in sorted(os.walk(root)):
        for fn in sorted(fs):
            if fn.endswith(".c"):
                files += 1
                p = os.path.join(d, fn)
                scan_file(p, os.path.relpath(p, root), hits, keep)
    if files == 0:
        print(f"libgcc_transcription_scan: no .c under {root}", file=sys.stderr)
        return 2
    for rule, rel, ln, name in keep + hits:
        print(f"{rule} {rel}:{ln} {name}")
    print(f"# scanned {files} .c file(s) under {root}: {len(hits)} member(s) fired,"
          f" {len(keep)} allowed TARGET_NATIVE spec one-liner(s) listed as KEEP-SPEC")
    return 1 if hits else 0


if __name__ == "__main__":
    sys.exit(main())

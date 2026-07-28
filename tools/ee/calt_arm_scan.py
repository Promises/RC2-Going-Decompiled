#!/usr/bin/env python3
"""Structurally enumerate every portable `#else` arm under `#ifndef TARGET_NATIVE`
in the decomp sources, and emit the arm body text for classification.

PREDICATE (what an "arm" is here):
  A contiguous region of a .c file that begins at the `#else` (or `#elif`)
  matching an `#ifndef TARGET_NATIVE` / `#if !defined(TARGET_NATIVE)` /
  `#if defined(MATCH_x) || defined(TARGET_NATIVE)` preprocessor conditional, and
  ends at that conditional's `#endif`.  This is the code that COMPILES AND RUNS
  in the C-only (calt) overlay build.  Regions guarded the other way
  (`#ifdef TARGET_NATIVE` ... `#else`) invert: the arm that runs on EE is the
  `#else` there too only when the leading test is TRUE for native; we record the
  polarity per region so the caller can tell.

Read-only.  Emits TSV + JSON to stdout/paths given.
"""
import json
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else "going-decompiled/src"

IF_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)$")
# A function definition line: something ending in ') {' or ')' at col 0-ish.
FUNC_RE = re.compile(
    r"^(?:static\s+|inline\s+|extern\s+)*[A-Za-z_][A-Za-z0-9_ \*\(\),]*?"
    r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*\)\s*\{?\s*$"
)


def native_polarity(kind, expr):
    """Return True if this conditional's TRUE-branch is the TARGET_NATIVE (host)
    branch, False if the TRUE-branch is the EE/asm branch, None if unrelated."""
    if "TARGET_NATIVE" not in expr:
        return None
    if kind == "ifndef":
        return False  # true-branch = NOT native = the asm/INCLUDE_ASM side
    if kind == "ifdef":
        return True
    if kind in ("if", "elif"):
        # `#if !defined(TARGET_NATIVE)` -> true branch is asm side
        if re.search(r"!\s*defined\s*\(\s*TARGET_NATIVE", expr):
            return False
        return True
    return None


def enclosing_func(lines, idx):
    """Walk backwards from idx to find the most recent plausible function
    definition header at brace depth 0-ish.  Best effort; reported as-is."""
    for i in range(idx, max(-1, idx - 400), -1):
        ln = lines[i].rstrip()
        if not ln or ln.startswith("#") or ln.startswith("//") or ln.startswith("*"):
            continue
        m = FUNC_RE.match(ln)
        if m and m.group(1) not in ("if", "for", "while", "switch", "return", "sizeof"):
            return m.group(1)
    return "?"


def scan_file(path):
    with open(path, "r", errors="replace") as f:
        lines = f.readlines()
    stack = []  # (kind_polarity, start_line, expr)
    arms = []
    for i, raw in enumerate(lines):
        m = IF_RE.match(raw)
        if not m:
            continue
        kind, expr = m.group(1), m.group(2)
        if kind in ("if", "ifdef", "ifndef"):
            stack.append({"pol": native_polarity(kind, expr), "if_line": i,
                          "expr": expr.strip(), "else_line": None})
        elif kind in ("else", "elif"):
            if stack:
                stack[-1]["else_line"] = i
        elif kind == "endif":
            if not stack:
                continue
            top = stack.pop()
            if top["pol"] is None or top["else_line"] is None:
                continue
            # The arm that EXECUTES in the C-only build:
            #  pol False (#ifndef TARGET_NATIVE): true-branch = asm; ELSE = C arm
            #  pol True  (#ifdef  TARGET_NATIVE): true-branch = host C; ELSE = asm
            if top["pol"] is False:
                a0, a1 = top["else_line"] + 1, i
                which = "ELSE"
            else:
                a0, a1 = top["if_line"] + 1, top["else_line"]
                which = "IFDEF-TRUE"
            body = "".join(lines[a0:a1])
            arms.append({
                "file": os.path.relpath(path, ROOT),
                "if_line": top["if_line"] + 1,
                "arm_start": a0 + 1,
                "arm_end": a1,
                "which": which,
                "expr": top["expr"],
                "func": enclosing_func(lines, top["if_line"]),
                "nlines": a1 - a0,
                "body": body,
            })
    return arms


def main():
    all_arms = []
    for dirpath, _dirs, files in os.walk(ROOT):
        for fn in sorted(files):
            if fn.endswith(".c"):
                all_arms.extend(scan_file(os.path.join(dirpath, fn)))
    json.dump(all_arms, sys.stdout)


if __name__ == "__main__":
    main()

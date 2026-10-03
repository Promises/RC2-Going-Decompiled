#!/usr/bin/env python3
"""decl_def_lint.py — list every function DECLARATION that disagrees with its
DEFINITION in another translation unit, on the calling-convention level.

Why (task #1379): EABI hides these on the EE. f32 arguments travel in $f12..,
integers and pointers in $a0.., so a declaration that swaps an f32 and a
pointer lands in the same registers as the definition and the EE bytes still
match. The i386 native arm passes everything on the stack, so there the callee
reads the wrong slots; a `void` definition behind a value-returning declaration
leaves the caller reading an unset return. tools/native/check.sh compiles each
unit ALONE, so a self-consistent wrong prototype compiles clean there (#1362
measured it: decl + call reverted together = pass). Four such instances were
found by hand by four seats (#1338, #1340, #1344, #1368) and a fifth by #1387.

HOW. The units, flags and C++ wrapping are check.sh's own: this script runs
check.sh with CC pointing back at itself (`--cc-shim`), so each unit is parsed
by the same clang invocation the native gate compiles it with, plus
`-fsyntax-only -Xclang -ast-dump=json`. Only FunctionDecl nodes are read, so
function-pointer locals, casts and calls through pointers never produce a row
(the false IMPLICIT rows #1338 excluded by hand cannot arise). Every
declaration is compared with the C definition of the same name in the same
REGION (usa decls against usa defs, eu against eu).

Each side is reduced to an ABI CLASS per parameter and for the return:
`ptr` (pointers, arrays, function pointers), `f32`, `f64`, `i8`/`i16`/`i32`
(integers and enums by width), `i64`, `i128`, `agg:<tag>` (struct/union by
value), `void`. A row is reported when the classes differ:
  RETURN-UNSET  declared value-returning, defined void: the caller reads
              a return the definition never writes
  RETURN-WIDTH  declared a wider int than the definition returns
  RETURN      both value-returning, in different locations (s32 vs s64/f32)
  ORDER       same classes, different order (the f32/pointer permutation)
  PARAM       a parameter's class differs at a position
  ARITY-MISSING  the declaration passes fewer arguments than the
              definition reads (it reads unset registers/stack slots)
  ARITY       counts differ AND the shared prefix disagrees
  VARIADIC    one side is variadic and the other is not
  NOPROTO     a C `f()` / implicit declaration of a definition that takes an
              f32 or a sub-int parameter (the call promotes them: f32 -> f64)
              or returns something the implicit `int` cannot hold
Pointee-only differences (`Vec4 *` vs `f32 *`) and the other ABI-identical
classes listed under INFO below are not errors.

STATUS of a row:
  ANNOTATED    the declaration carries `DECL-LEVER(#<task>): <reason>` in a
               comment on its own line or the line directly above it. That is
               the ONLY suppression, so a sanctioned lever (S32DECL, #1344's
               s32-declared void callees) is DECLARED deliberate and anything
               else is an error.
  FIXABLE      the definition is UNMATCHED (its unit still carries an
               INCLUDE_ASM line for it and no MATCH_ guard), so its C is not
               compiled on EE: correcting the DEFINITION cannot move an EE
               byte. Correcting the declaration instead can, when a matched
               caller compiles against it: gate that (image cmp 0).
  DESIGN-CALL  the definition is MATCHED code (image-resident, s136os-spliced
               or MATCH_-guarded). MATCHED CODE IS NOT CHANGED ON A LINT'S
               SAY-SO: the definition is never the fix. Correct the
               declaration when that is byte-neutral (image cmp 0), annotate
               it when a ruling or a measured lever makes it deliberate, or
               escalate.
  STALE-LEVER  a DECL-LEVER annotation on a declaration that no longer
               disagrees with its definition: remove the annotation.
Exit: 0 clean (only ANNOTATED rows), 1 any other row, 2 the parse could not
run (fails CLOSED: a unit that does not compile, a missing dump, a clang
error). Rows are printed one per declaration SITE, never one per symbol.

BLIND SPOTS (a clean run is not a proof of absence):
  - Declarations with no C definition in the region (the callee is still
    INCLUDE_ASM-only, or lives in the SDK/libc) are compared with nothing.
  - Only the native arm is parsed (-DTARGET_NATIVE): a declaration that exists
    only inside an `#ifndef TARGET_NATIVE` arm, or behind any other guard the
    native build does not enter, is invisible.
  - Units that check.sh does not compile (no TARGET_NATIVE marker) are not
    parsed; a definition that lives only there is unseen.
  - Pointee types are not compared (see above), nor are typedef'd pointer
    types beyond what clang's desugared spelling shows, nor qualifiers.
  - Integer signedness is not compared (s32 vs u32 is one class).
  - Calls through function pointers are not declarations and are not checked.
  - Matched/unmatched is read from the source text (INCLUDE_ASM / MATCH_
    directive lines anchored to line start), not from the built image.
  - A row is one declaration SITE compared with the FIRST definition of
    the name (by path); DUPDEF lists names defined twice in a region.

INFO rows (shown with --info, never an error): RETURN-DISCARDED (declared
void, defined value-returning: the caller never reads it), RETURN-WORD /
PARAM-WORD (int <-> pointer, or a narrower int: same location on both arms),
ARITY-EXTRA (trailing arguments the definition never reads), POINTEE, and a
NOPROTO declaration whose call cannot be mis-promoted.

COST (XPS, clang 23.1.1): ~30 s for the 44 units; --base ~50 s (two trees);
--selftest ~4 s.

Usage:
  tools/native/decl_def_lint.py [--info] [unit ...]   lint this tree
  tools/native/decl_def_lint.py --base <rev> [--info] error sites at this
                                  tree that <rev>'s tree does not have (a
                                  multiset keyed without line numbers)
  tools/native/decl_def_lint.py --selftest            fixture arms
"""
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
LEVER_RE = re.compile(r"DECL-LEVER\(#\d+(?:,\s*#\d+)*\):\s*\S")

# ---------------------------------------------------------------- cc shim ---


def real_cc():
    """The compiler check.sh would have used had CC been unset."""
    cc = os.environ.get("DDL_REAL_CC")
    if cc:
        return cc.split()
    if os.uname().sysname == "Darwin":
        sdk = subprocess.check_output(["xcrun", "--show-sdk-path"], text=True).strip()
        return ["clang", "--target=i386-pc-linux-gnu", "-isystem", sdk + "/usr/include"]
    return ["clang"]


def ast_nodes(root):
    """Yield (node, file, line) for every dict node with a 'kind', in dump
    order, tracking the current presumed file/line. clang's JSON dumper omits
    'file' and 'line' when unchanged from the previously PRINTED location, so
    every location must be visited in print order (key order, depth first)."""
    cur = {"file": None, "line": None}
    stack = [root]
    while stack:
        x = stack.pop()
        if isinstance(x, list):
            stack.extend(reversed(x))
            continue
        if not isinstance(x, dict):
            continue
        if "offset" in x and "kind" not in x:      # a bare source location
            if "file" in x:
                cur["file"] = x["file"]
            if "line" in x:
                cur["line"] = x["line"]
            continue
        if "kind" in x:
            # loc is printed first; resolve it now so the node reports it
            loc = x.get("loc")
            if isinstance(loc, dict):
                for part in ([loc.get("spellingLoc"), loc.get("expansionLoc")]
                             if "expansionLoc" in loc else [loc]):
                    if isinstance(part, dict):
                        if "file" in part:
                            cur["file"] = part["file"]
                        if "line" in part:
                            cur["line"] = part["line"]
            if "loc" in x:
                yield x, cur["file"], cur["line"]
            # A reference to a decl (DeclRefExpr's referencedDecl, ...) is a
            # dict with a kind and a type but NO location: it is a USE, not a
            # declaration (it made every call site read as a 0-parameter
            # declaration). Statements and expressions also lack a loc but
            # carry ranges and nested decls, so only the reference keys are
            # dropped, not loc-less nodes.
            rest = [v for k, v in x.items() if k not in ("loc", "referencedDecl",
                                                          "foundReferencedDecl")]
            stack.extend(reversed(rest))
            continue
        stack.extend(reversed(list(x.values())))


def split_fn_type(t):
    """'ret (a, b)' -> ('ret', 'a, b'); handles nested parens in params."""
    t = t.strip()
    # strip trailing qualifiers like __attribute__ text clang may append
    if not t.endswith(")"):
        return None
    depth = 0
    for i in range(len(t) - 1, -1, -1):
        c = t[i]
        if c == ")":
            depth += 1
        elif c == "(":
            depth -= 1
            if depth == 0:
                return t[:i].strip(), t[i + 1:-1].strip()
    return None


INT_WIDTH = [("long long", "i64"), ("__int128", "i128"), ("char", "i8"),
             ("short", "i16"), ("int", "i32"), ("long", "i32"), ("unsigned", "i32"),
             ("signed", "i32"), ("_Bool", "i8"), ("bool", "i8"), ("wchar_t", "i32")]


def classify(t, typedefs, depth=0):
    s = re.sub(r"\b(const|volatile|restrict|__restrict|__restrict__)\b", " ", t)
    s = " ".join(s.split())
    if not s:
        return "unk:" + t
    if "*" in s or "[" in s or "(" in s or "&" in s:
        return "ptr"
    m = re.match(r"^(struct|union|class)\s+(\S+)$", s)
    if m:
        return "agg:" + m.group(2)
    if s.startswith("enum "):
        return "i32"
    if s == "void":
        return "void"
    if s == "float":
        return "f32"
    if s in ("double", "long double"):
        return "f64"
    for k, cls in INT_WIDTH:
        if re.search(r"\b" + re.escape(k) + r"\b", s):
            return cls
    if s in typedefs and depth < 16:
        return classify(typedefs[s], typedefs, depth + 1)
    return "unk:" + s


def extract(dump, unit, lang):
    """Reduce one TU's AST dump to its function declarations/definitions."""
    typedefs, tags, fns = {}, {}, []
    for node, f, line in ast_nodes(dump):
        k = node.get("kind")
        if k in ("TypedefDecl", "TypeAliasDecl"):
            ty = node.get("type", {})
            typedefs[node.get("name")] = ty.get("desugaredQualType") or ty.get("qualType", "")
        elif k in ("RecordDecl", "CXXRecordDecl", "EnumDecl"):
            pass
        elif k == "FunctionDecl":
            ty = node.get("type", {})
            parms = [c for c in node.get("inner", []) if c.get("kind") == "ParmVarDecl"]
            body = any(c.get("kind") == "CompoundStmt" for c in node.get("inner", []))
            st = split_fn_type(ty.get("qualType", ""))
            if st is None:
                continue
            ret_s, plist = st
            variadic = plist.endswith("...")
            noproto = (lang == "c" and plist == "")
            fns.append({
                "name": node.get("name"),
                "file": f, "line": line,
                "def": body,
                "implicit": bool(node.get("isImplicit")),
                "ret": ret_s,
                "params": [p.get("type", {}).get("qualType", "") for p in parms],
                "pdesugar": [p.get("type", {}).get("desugaredQualType")
                             or p.get("type", {}).get("qualType", "") for p in parms],
                "variadic": variadic, "noproto": noproto,
                "static": node.get("storageClass") == "static",
            })
    for fn in fns:
        fn["ret_cls"] = classify(fn["ret"], typedefs)
        fn["param_cls"] = [classify(p, typedefs) for p in fn["pdesugar"]]
    return {"unit": unit, "lang": lang, "fns": fns}


def cc_shim(argv):
    """Invoked by check.sh as $CC. Compile-check exactly as asked, dump the
    AST, write the reduced record, exit with clang's status."""
    out_dir = os.environ["DDL_OUT"]
    args = list(argv)
    obj = None
    if "-o" in args:
        i = args.index("-o")
        obj = args[i + 1]
        del args[i:i + 2]
    args = [a for a in args if a != "-c"]
    src = [a for a in args if a.endswith((".c", ".cpp")) and not a.startswith("-")][-1]
    lang = "c++" if "c++" in args else "c"
    unit = src
    if src.endswith(".wrap.cpp"):
        incs = re.findall(r'#include "([^"]+)"', open(src).read())
        unit = incs[-1]
    cmd = real_cc() + args + ["-fsyntax-only", "-Xclang", "-ast-dump=json"]
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    sys.stderr.write(p.stderr.decode(errors="replace"))
    if p.returncode != 0:
        return p.returncode
    rec = extract(json.loads(p.stdout), os.path.abspath(unit), lang)
    name = os.path.basename(obj or src) + ".ddl.json"
    with open(os.path.join(out_dir, name), "w") as fh:
        json.dump(rec, fh)
    return 0

# ------------------------------------------------------------ the lint ---


def region_of(path):
    m = re.search(r"going-decompiled/src/([^/]+)/", path)
    return m.group(1) if m else None


def rel(path):
    return os.path.relpath(path, ROOT) if path and os.path.isabs(path) else path


_src_cache = {}


def src_lines(path):
    if path not in _src_cache:
        try:
            with open(path, errors="replace") as fh:
                _src_cache[path] = fh.read().split("\n")
        except OSError:
            _src_cache[path] = []
    return _src_cache[path]


def lever_at(path, line):
    """The DECL-LEVER annotation on the declaration's own line, or on the line
    directly above when that line is a comment and nothing else. Without that
    restriction the trailing annotation of one declaration also suppressed
    the declaration on the next line (seen on 250080.cpp, task #1379)."""
    lines = src_lines(path)
    if 1 <= line <= len(lines) and LEVER_RE.search(lines[line - 1]):
        return lines[line - 1].strip()
    if 2 <= line <= len(lines) + 1:
        above = lines[line - 2]
        if LEVER_RE.search(above) and re.match(r"^\s*(/\*|//|\*)", above):
            return above.strip()
    return None


def unmatched_set(region):
    """Functions whose image body is still asm: an INCLUDE_ASM* line names
    them (anchored at line start, so prose cannot), minus MATCH_-guarded ones,
    which are arm-scored matched code. Returned with the MATCH_ set."""
    inc = re.compile(r"^\s*INCLUDE_ASM\w*\(\s*\"[^\"]*\"\s*,\s*(\w+)\s*\)")
    guard = re.compile(r"^\s*#\s*(?:if|ifdef|ifndef|elif)\b.*\bMATCH_(\w+)")
    asm, guarded = set(), set()
    base = os.path.join(ROOT, "going-decompiled", "src", region)
    for d, _, files in os.walk(base):
        for f in files:
            if not f.endswith((".c", ".cpp")):
                continue
            for line in src_lines(os.path.join(d, f)):
                m = inc.match(line)
                if m:
                    asm.add(m.group(1))
                    continue
                m = guard.match(line)
                if m:
                    guarded.add(m.group(1))
    return asm - guarded


# Where a value travels. Pointers and every integer up to 32 bits share one
# location on BOTH arms (an EE $a/$v register, one 4-byte i386 stack slot or
# %eax), so int <-> pointer and s16 <-> s32 are ABI-identical. f32 ($f12../$f0
# on EE), f64, by-value aggregates and 64/128-bit integers travel somewhere
# else on at least one arm: an s64 shares s32's single 64-bit GPR on EE (the
# S32DECL lever's domain) but takes two i386 stack slots / %edx:%eax.
WORD = {"ptr": 32, "i32": 32, "i16": 16, "i8": 8}


def loc_of(cls):
    return "word" if cls in WORD else cls


def compare(decl, dfn):
    """(errors, infos) between a declaration and the definition.

    An ERROR is a disagreement under which a caller built from the declaration
    passes or reads a value somewhere the definition does not put it, on
    either arm. An INFO is a type disagreement that is ABI-identical."""
    err, info = [], []
    dr, fr = decl["ret_cls"], dfn["ret_cls"]
    if decl["implicit"] or decl["noproto"]:
        why = [c for c in dfn["param_cls"] if c in ("f32", "i8", "i16")]
        bad_ret = fr == "void" or loc_of(fr) != "word"
        what = "%s `%s ()` vs %s (%s)" % (
            "implicit" if decl["implicit"] else "unprototyped",
            decl["ret"], dfn["ret"], ", ".join(dfn["params"]))
        if why:
            err.append("NOPROTO %s: promoted %s" % (what, ",".join(why)))
        elif bad_ret and fr != "void":
            err.append("NOPROTO %s: return not in the int location" % what)
        else:
            info.append("NOPROTO %s" % what)
        return err, info
    if dr != fr:
        pair = "%s vs %s" % (decl["ret"], dfn["ret"])
        if dr == "void":
            info.append("RETURN-DISCARDED " + pair)        # caller never reads it
        elif fr == "void":
            err.append("RETURN-UNSET " + pair)             # caller reads garbage
        elif loc_of(dr) != loc_of(fr):
            err.append("RETURN " + pair)
        elif WORD[dr] > WORD[fr]:
            err.append("RETURN-WIDTH " + pair)             # upper bits unwritten
        else:
            info.append("RETURN-WORD " + pair)
    if decl["variadic"] != dfn["variadic"]:
        err.append("VARIADIC decl %s, def %s" % (decl["variadic"], dfn["variadic"]))
    a, b = decl["param_cls"], dfn["param_cls"]
    if len(a) != len(b):
        pair = "%d vs %d: (%s) vs (%s)" % (len(a), len(b), ", ".join(decl["params"]),
                                          ", ".join(dfn["params"]))
        n = min(len(a), len(b))
        same_prefix = [loc_of(c) for c in a[:n]] == [loc_of(c) for c in b[:n]]
        if len(a) > len(b) and same_prefix:
            # the caller passes trailing arguments the callee never reads;
            # caller-cleanup on i386, dead registers on EE: ABI-harmless
            info.append("ARITY-EXTRA " + pair)
        elif len(a) < len(b):
            err.append("ARITY-MISSING " + pair)            # callee reads unset args
        else:
            err.append("ARITY " + pair)
        return err, info
    la, lb = [loc_of(c) for c in a], [loc_of(c) for c in b]
    if la != lb:
        if sorted(la) == sorted(lb):
            err.append("ORDER (%s) vs (%s)" % (", ".join(decl["params"]),
                                               ", ".join(dfn["params"])))
        else:
            for i, (x, y) in enumerate(zip(la, lb)):
                if x != y:
                    err.append("PARAM %d: %s vs %s" % (i + 1, decl["params"][i],
                                                       dfn["params"][i]))
    for i, (x, y) in enumerate(zip(a, b)):
        if x != y and loc_of(x) == loc_of(y):
            info.append("PARAM-WORD %d: %s vs %s" % (i + 1, decl["params"][i],
                                                     dfn["params"][i]))
        elif x == y == "ptr" and \
                decl["pdesugar"][i].replace("const ", "") != dfn["pdesugar"][i].replace("const ", ""):
            info.append("POINTEE %d: %s vs %s" % (i + 1, decl["params"][i],
                                                  dfn["params"][i]))
    return err, info


def run_check(units):
    """check.sh OF THE TREE BEING LINTED (its flags, its unit list), with CC
    pointed at this script's shim. The temp dir is the caller's to remove."""
    out = tempfile.mkdtemp(prefix="ddl.")
    env = dict(os.environ)
    env["DDL_OUT"] = out
    env["CC"] = "%s %s --cc-shim" % (sys.executable, os.path.abspath(__file__))
    p = subprocess.run(["bash", os.path.join(ROOT, "tools", "native", "check.sh")] + units,
                       env=env, cwd=ROOT, stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT, text=True)
    return out, p


def under(path, *dirs):
    return bool(path) and any(os.path.abspath(path).startswith(os.path.join(ROOT, d) + os.sep)
                              for d in dirs)


def collect(units):
    """Run check.sh through the shim; (units, records) or an UNRUNNABLE text.
    Fails closed: a unit that does not compile, a summary line that is
    missing, or a unit count that differs from the record count."""
    out, p = run_check(units)
    try:
        m = re.search(r"--- native compile-check: pass=(\d+) fail=(\d+) ---", p.stdout)
        if not m or p.returncode != 0 or int(m.group(2)) != 0:
            return None, p.stdout + "check.sh did not report a clean compile"
        npass = int(m.group(1))
        names = sorted(os.listdir(out))
        if npass == 0 or len(names) != npass:
            return None, "%d units compiled, %d AST records" % (npass, len(names))
        return (npass, [json.load(open(os.path.join(out, f))) for f in names]), None
    finally:
        shutil.rmtree(out, ignore_errors=True)


def lint_rows(units):
    """(npass, rows) for the tree at ROOT, or (None, why)."""
    got, why = collect(units)
    if got is None:
        return None, why
    npass, recs = got

    # Cross-TU identity is an EXTERNAL name: static functions are per-unit and
    # never match another unit's symbol. Definitions count only from the
    # game's own source (not libc/libstdc++ inline bodies); declarations only
    # from the tree (going-decompiled/, tools/native/), not system headers.
    defs, decls = {}, []
    for r in recs:
        reg = region_of(r["unit"])
        for fn in r["fns"]:
            if fn["static"]:
                continue
            fn["region"] = reg
            if fn["def"]:
                if under(fn["file"], "going-decompiled/src"):
                    defs.setdefault((reg, fn["name"]), {})[fn["file"], fn["line"]] = fn
            elif under(fn["file"], "going-decompiled", "tools/native"):
                decls.append(fn)

    rows, seen = [], set()
    unmatched = {reg: unmatched_set(reg) for reg in {k[0] for k in defs}}
    for (reg, name), ds in sorted(defs.items()):
        if len(ds) > 1:
            rows.append(("DUPDEF", reg, name, ", ".join(
                "%s:%s" % (rel(f), l) for f, l in sorted(ds)), "-", "defined in more than one place"))
    for d in decls:
        reg, name = d["region"], d["name"]
        if (reg, name) not in defs:
            continue
        site = (reg, name, d["file"], d["line"])
        if site in seen:
            continue
        seen.add(site)
        dfn = sorted(defs[(reg, name)].values(), key=lambda f: (f["file"], f["line"]))[0]
        err, info = compare(d, dfn)
        lev = None if d["implicit"] else lever_at(d["file"], d["line"])
        where = "%s:%s" % (rel(d["file"]), d["line"])
        defwhere = "%s:%s" % (rel(dfn["file"]), dfn["line"])
        if err:
            if lev:
                st = "ANNOTATED"
            elif name in unmatched.get(reg, set()):
                st = "FIXABLE"
            else:
                st = "DESIGN-CALL"
            rows.append((st, reg, name, where, defwhere, "; ".join(err)))
        elif lev:
            rows.append(("STALE-LEVER", reg, name, where, defwhere,
                         "annotated, but agrees with the definition at the ABI level"))
        for i in info:
            rows.append(("INFO", reg, name, where, defwhere, i))

    rows.sort(key=lambda r: (ORDER[r[0]], r[1], r[2], r[3]))
    return npass, rows


ORDER = {"DESIGN-CALL": 0, "FIXABLE": 1, "STALE-LEVER": 2, "DUPDEF": 3,
         "ANNOTATED": 4, "INFO": 5}
ERROR_STATES = ("DESIGN-CALL", "FIXABLE", "STALE-LEVER", "DUPDEF")


def fmt(r):
    return "%-11s %-3s %s  decl %s  def %s  %s" % r


def site_key(r):
    """A row's identity across two trees: no line numbers (an unrelated edit
    above a declaration moves it), no status (a definition promoted between
    the trees turns FIXABLE into DESIGN-CALL without the row changing)."""
    return (r[1], r[2], r[3].rsplit(":", 1)[0], r[5])


def lint(units, show_info=False):
    npass, rows = lint_rows(units)
    if npass is None:
        print(rows)
        print("#### decl-def-lint: UNRUNNABLE (fails closed)")
        return 2
    counts = {}
    for r in rows:
        counts[r[0]] = counts.get(r[0], 0) + 1
        if r[0] != "INFO" or show_info:
            print(fmt(r))
    bad = sum(counts.get(k, 0) for k in ERROR_STATES)
    print("--- decl-def-lint: units=%d %s ---" % (
        npass, " ".join("%s=%d" % (k, counts.get(k, 0)) for k in ORDER)))
    print("#### decl-def-lint: %s" % ("FAIL" if bad else "PASS"))
    return 1 if bad else 0


def lint_diff(base, units):
    """Error rows at the tip (this tree) that the base tree does not have,
    compared as a MULTISET of site keys. The base tree is `git archive`d from
    <base> so its own check.sh, flags and unit list are used."""
    global ROOT
    tip_root = ROOT
    tmp = tempfile.mkdtemp(prefix="ddl-base.")
    try:
        arch = subprocess.run(["git", "-C", tip_root, "archive", "--format=tar", base, "--",
                               "going-decompiled/src", "going-decompiled/include",
                               "tools/native"], stdout=subprocess.PIPE)
        if arch.returncode != 0:
            print("#### decl-def-lint: UNRUNNABLE (cannot archive base %s; fails closed)" % base)
            return 2
        subprocess.run(["tar", "-x", "-C", tmp], input=arch.stdout, check=True)
        n_tip, tip = lint_rows(units)
        ROOT = tmp
        n_base, base_rows = lint_rows([u.replace(tip_root, tmp, 1) for u in units])
    finally:
        ROOT = tip_root
        shutil.rmtree(tmp, ignore_errors=True)
    if n_tip is None or n_base is None:
        print(tip if n_tip is None else base_rows)
        print("#### decl-def-lint: UNRUNNABLE (fails closed)")
        return 2
    pool = {}
    for r in base_rows:
        if r[0] in ERROR_STATES:
            pool[site_key(r)] = pool.get(site_key(r), 0) + 1
    new, gone = [], dict(pool)
    for r in tip:
        if r[0] not in ERROR_STATES:
            continue
        k = site_key(r)
        if gone.get(k, 0) > 0:
            gone[k] -= 1
        else:
            new.append(r)
    for r in new:
        print("NEW  " + fmt(r))
    ngone = sum(gone.values())
    for k, v in sorted(gone.items()):
        for _ in range(v):
            print("GONE %-3s %s  decl %s  %s" % k)
    nb = sum(pool.values())
    print("--- decl-def-lint diff: base %s units=%d errors=%d -> tip units=%d errors=%d; "
          "new=%d gone=%d ---" % (base, n_base, nb, n_tip,
                                  sum(1 for r in tip if r[0] in ERROR_STATES), len(new), ngone))
    print("#### decl-def-lint diff: %s" % ("FAIL" if new else "PASS"))
    return 1 if new else 0


# ------------------------------------------------------------ selftest ---
# Each arm builds a throwaway tree holding the REAL tools/native/check.sh and
# two units, lints it, and asserts the verdict AND the member rows. Every arm
# that asserts a FAIL is the seeded control for the arm that asserts a PASS on
# the same shape with one thing changed.

DEF_UNIT = """#ifndef TARGET_NATIVE
INCLUDE_ASM("asm/usa/text/def", Unmatched);
#else
void Unmatched(float *dst, float len, const float *src) { dst[0] = src[0] * len; }
#endif
void Matched(float *dst, float len, const float *src) { dst[0] = src[0] * len; }
void Sink(int x) { (void)x; }
int  Value(void) { return 1; }
"""


def _arm(name, use_unit, expect_rc, expect_rows, absent=(), def_unit=DEF_UNIT):
    global ROOT
    tip_root = ROOT
    tmp = tempfile.mkdtemp(prefix="ddl-st.")
    try:
        os.makedirs(os.path.join(tmp, "tools", "native"))
        for f in ("check.sh", "mips_callees.h"):
            shutil.copy(os.path.join(tip_root, "tools", "native", f),
                        os.path.join(tmp, "tools", "native", f))
        src = os.path.join(tmp, "going-decompiled", "src", "usa", "text")
        os.makedirs(src)
        os.makedirs(os.path.join(tmp, "going-decompiled", "include"))
        with open(os.path.join(src, "def.c"), "w") as fh:
            fh.write("/* TARGET_NATIVE unit */\n" + def_unit)
        with open(os.path.join(src, "use.c"), "w") as fh:
            fh.write("/* TARGET_NATIVE unit */\n" + use_unit)
        ROOT = tmp
        import io
        import contextlib
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = lint([])
        out = buf.getvalue()
    finally:
        ROOT = tip_root
        shutil.rmtree(tmp, ignore_errors=True)
    ok = rc == expect_rc and all(re.search(r, out, re.M) for r in expect_rows) \
        and not any(re.search(r, out, re.M) for r in absent)
    print("%s %s: rc %d (want %d)" % ("OK  " if ok else "SELFTEST-FAIL", name, rc, expect_rc))
    if not ok:
        print("    " + out.replace("\n", "\n    "))
    return ok


def selftest():
    arms = [
        ("s1 permuted f32/pointer vs a MATCHED def -> DESIGN-CALL, FAIL",
         "void Matched(float *dst, const float *src, float len);\n"
         "void f(float *a) { Matched(a, a, 2.0f); }\n", 1,
         [r"^DESIGN-CALL usa Matched  decl going-decompiled/src/usa/text/use.c:2 .* ORDER"]),
        ("s2 the same declaration ANNOTATED -> PASS",
         "void Matched(float *dst, const float *src, float len); /* DECL-LEVER(#1379): test */\n"
         "void f(float *a) { Matched(a, a, 2.0f); }\n", 0,
         [r"^ANNOTATED   usa Matched .* ORDER", r"^#### decl-def-lint: PASS"]),
        ("s3 annotation on the line ABOVE also counts -> PASS",
         "/* DECL-LEVER(#1379): test */\n"
         "void Matched(float *dst, const float *src, float len);\n"
         "void f(float *a) { Matched(a, a, 2.0f); }\n", 0, [r"^ANNOTATED   usa Matched"]),
        ("s3b a TRAILING annotation does not reach the next line's declaration -> FAIL",
         "void Sink(void *p); /* DECL-LEVER(#1379): test */\n"
         "void Matched(float *dst, const float *src, float len);\n"
         "void f(float *a) { Matched(a, a, 2.0f); }\n", 1,
         [r"^DESIGN-CALL usa Matched  decl going-decompiled/src/usa/text/use.c:3 .* ORDER",
          r"^STALE-LEVER usa Sink"], [r"^ANNOTATED"]),
        ("s4 an annotation with no task reference does NOT suppress -> FAIL",
         "void Matched(float *dst, const float *src, float len); /* DECL-LEVER: test */\n"
         "void f(float *a) { Matched(a, a, 2.0f); }\n", 1, [r"^DESIGN-CALL usa Matched"]),
        ("s5 permuted vs an UNMATCHED (INCLUDE_ASM) def -> FIXABLE, FAIL",
         "void Unmatched(float *dst, const float *src, float len);\n"
         "void f(float *a) { Unmatched(a, a, 2.0f); }\n", 1, [r"^FIXABLE     usa Unmatched .* ORDER"]),
        ("s6 an annotation on an AGREEING declaration -> STALE-LEVER, FAIL",
         "void Matched(float *dst, float len, const float *src); /* DECL-LEVER(#1379): test */\n"
         "void f(float *a) { Matched(a, 2.0f, a); }\n", 1, [r"^STALE-LEVER usa Matched"]),
        ("s7 value-declared, void-defined -> RETURN-UNSET, FAIL",
         "int Sink(int x);\nint f(void) { return Sink(1); }\n", 1,
         [r"^DESIGN-CALL usa Sink .* RETURN-UNSET int vs void"]),
        ("s8 void-declared, value-defined + int/pointer param: ABI-identical -> PASS (INFO only)",
         "void Value(void);\nvoid Sink(void *p);\nvoid f(void) { Value(); Sink(0); }\n", 0,
         [r"^#### decl-def-lint: PASS"], [r"^DESIGN-CALL", r"^FIXABLE"]),
        ("s9 calls, function-pointer locals and casts are not declarations -> PASS",
         "void Matched(float *dst, float len, const float *src);\n"
         "void f(float *a) { void (*fp)(int) = (void (*)(int))Matched; fp(1); Matched(a, 1.0f, a); }\n",
         0, [r"^#### decl-def-lint: PASS"], [r"^DESIGN-CALL", r"^FIXABLE"]),
        ("s10 a block-scope declaration is seen -> FAIL",
         "void f(float *a) { extern void Matched(float *, const float *, float); Matched(a, a, 1.0f); }\n",
         1, [r"^DESIGN-CALL usa Matched  decl going-decompiled/src/usa/text/use.c:2 "]),
        ("s11 fewer arguments than the definition reads -> ARITY-MISSING, FAIL",
         "void Sink(void);\nvoid f(void) { Sink(); }\n", 1, [r"ARITY-MISSING 0 vs 1"]),
        ("s12 a unit that does not compile -> UNRUNNABLE rc 2 (fails closed)",
         "void f(void) { this is not C; }\n", 2, [r"^#### decl-def-lint: UNRUNNABLE"]),
    ]
    ok = all([_arm(*a) for a in arms])
    print("#### decl-def-lint selftest: %s (%d arms)" % ("PASS" if ok else "FAIL", len(arms)))
    return 0 if ok else 1


def main(argv):
    if argv and argv[0] == "--cc-shim":
        return cc_shim(argv[1:])
    if argv == ["--selftest"]:
        return selftest()
    units, base, info = [], None, False
    it = iter(argv)
    for a in it:
        if a == "--info":
            info = True
        elif a == "--base":
            base = next(it)
        elif a.startswith("-"):
            print(__doc__)
            return 2
        else:
            units.append(os.path.abspath(a))
    if base:
        return lint_diff(base, units)
    return lint(units, show_info=info)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

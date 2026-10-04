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

DIRECTION RULE for FIXABLE rows (watcher-2, decision 2 on task #1414's Q2;
task #1425), verbatim: "DIRECTION RULE for FIXABLE rows (the definition is
still `INCLUDE_ASM`): fix toward the DEFINITION's own C (its `#else` body)
when one exists; otherwise toward the MAJORITY of declaration sites. Ties are
listed for a ruling, not auto-fixed." Each FIXABLE row is followed by a
`direction:` line giving the rule's choice AND its evidence: whether the
definition has C of its own (a body with at least one statement; an empty
`{}` arm has none), and the tally of that symbol's declaration sites agreeing
and disagreeing with it at the ABI level. With no C to fix toward, sites vote
by ABI signature (where each value travels, not its spelling); a tie prints
`TIE — needs a ruling` and is never resolved. The rule is OUTPUT: this lint
never rewrites source, and matched names/code are never changed on its say-so.
WHY not "fix the definition" (#1379's first wording): #1414 measured
Vec3RescaleToLenVu0 with 9 declaration sites agreeing with its definition and
5 disagreeing; editing the definition would have broken the 9.
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
--selftest ~10 s (28 arms, tasks #1425/#1461).

UNITS (task #1425). Unit arguments SELECT rows; they never narrow the parse.
The whole TARGET_NATIVE population is always parsed, and a row is kept when
its declaring unit, its declaration's file or its definition's file is a named
unit. Before #1425 the named units were the only ones parsed, so a declaration
whose definition lived in an unnamed unit was compared with nothing: #1424
measured `units=2`, 0 rows, PASS on a tree with a known positive (re-derived
here at cf68dc69: 250080.cpp alone printed `units=1`, 0 rows, PASS where the
whole tree lists func_00133850 at :218 and :385). A named unit that is not in
the population is UNRUNNABLE (rc 2), never an empty PASS. Naming units saves
no time; the summary says `units=<parsed> requested=<named>`.

--base IDENTITY (task #1425, watcher-2 decision 1 on #1414's Q1). A row is
keyed on (region, symbol, declaration file, disagreement KIND), never on the
type text, so RE-SPELLING an existing mismatch (#1389's s64 -> s32 vs a void
definition) is not NEW. The KIND is the row's error tags (RETURN-UNSET, ORDER,
ARITY-MISSING, ...; PARAM keeps its position). The printed row still carries
the types. Rows sharing a key are COUNTED (a multiset): a further same-kind
site in the same file is NEW, but a same-kind SWAP within one file (one fixed,
one added) is not; see site_key. `--key text` restores the old identity for
--selftest's negative control only.

--base REPORT (task #1461). A key whose count GREW prints one `NEW +<delta>`
header with its `count base B -> tip T`, then EVERY tip site of that key
(`  site <row>`, tip line numbers). Sites sharing a key are indistinguishable
to it, so the report names all of them rather than guess: before #1461 base
rows absorbed tip rows in tip order and the key's LAST site was always the one
named NEW (#1450: a third func_0011AEA0 declaration added at 250080.cpp:265
was reported at a pre-existing site, FACT #9147). A key whose count FELL
prints `GONE -<delta>` and every BASE site (`  was  <row>`, base line
numbers). The summary's new=/gone= are the summed deltas. A pure same-kind
SWAP still prints nothing.

Usage:
  tools/native/decl_def_lint.py [--info] [unit ...]   lint this tree (units
                                  select rows, see UNITS)
  tools/native/decl_def_lint.py --base <rev> [--repo <dir>] [--info] [unit ...]
                                  error sites at this tree that <rev>'s tree
                                  does not have; <rev> is resolved in <dir>'s
                                  git repository (default: this tree's)
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
            stmts = [c for c in node.get("inner", []) if c.get("kind") == "CompoundStmt"]
            body = bool(stmts)
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
                "body_stmts": len(stmts[0].get("inner", [])) if stmts else 0,
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


def collect():
    """Run check.sh over EVERY unit through the shim; (units, records) or an
    UNRUNNABLE text. Always the whole population, whatever units the caller
    asked about: a row needs BOTH its declaration's unit and its definition's
    unit, so parsing only the asked units compared their declarations with
    nothing (task #1425; #1424 measured `units=2`, 0 rows, PASS on a tree with
    a known positive). Fails closed: a unit that does not compile, a summary
    line that is missing, or a unit count that differs from the record count."""
    out, p = run_check([])
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
    """(npass, rows, directions) for the tree at ROOT, or (None, why, None).

    `units` (absolute paths, may be empty) SELECTS rows, it never narrows the
    parse: the whole population is parsed and a row is kept when the unit that
    declared it, its declaration's file or its definition's file is one of the
    units. A unit that is not in the population (no TARGET_NATIVE marker, a
    typo, another tree) fails closed: selecting from nothing would be a PASS."""
    got, why = collect()
    if got is None:
        return None, why, None
    npass, recs = got
    want = set(units)
    missing = sorted(want - {r["unit"] for r in recs})
    if missing:
        return None, ("unit(s) not in check.sh's TARGET_NATIVE population, so no row "
                      "could be selected for them: %s" % ", ".join(rel(u) for u in missing)), None

    # Cross-TU identity is an EXTERNAL name: static functions are per-unit and
    # never match another unit's symbol. Definitions count only from the
    # game's own source (not libc/libstdc++ inline bodies); declarations only
    # from the tree (going-decompiled/, tools/native/), not system headers.
    defs, decls, decl_units = {}, [], {}
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
                decl_units.setdefault((reg, fn["name"], fn["file"], fn["line"]), set()).add(r["unit"])

    def selected(files, site=None):
        if not want:
            return True
        hit = {os.path.abspath(f) for f in files if f} | decl_units.get(site, set())
        return bool(hit & want)

    rows, seen, sites = [], set(), {}
    unmatched = {reg: unmatched_set(reg) for reg in {k[0] for k in defs}}
    for (reg, name), ds in sorted(defs.items()):
        if len(ds) > 1 and selected([f for f, _ in ds]):
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
        sites.setdefault((reg, name), []).append((d, bool(err)))
        if not selected([d["file"], dfn["file"]], site):
            continue
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
    directions = {}
    for r in rows:
        if r[0] == "FIXABLE" and (r[1], r[2]) not in directions:
            dfn = sorted(defs[(r[1], r[2])].values(), key=lambda f: (f["file"], f["line"]))[0]
            directions[r[1], r[2]] = direction(dfn, sites[r[1], r[2]])
    return npass, rows, directions


# ------------------------------------------------- FIXABLE direction rule ---
# watcher-2, decision 2 on task #1414's Q2 (task #1425), VERBATIM:
DIRECTION_RULE = (
    "DIRECTION RULE for FIXABLE rows (the definition is still `INCLUDE_ASM`): "
    "fix toward the DEFINITION's own C (its `#else` body) when one exists; "
    "otherwise toward the MAJORITY of declaration sites. Ties are listed for a "
    "ruling, not auto-fixed.")
# WHY not "fix the definition": #1414 measured Vec3RescaleToLenVu0 with 9
# declaration sites AGREEING with its definition and 5 disagreeing; editing the
# definition to the 5 would have broken the 9. This lint REPORTS the direction
# and its evidence; it never rewrites a declaration or a definition, and
# MATCHED NAMES/CODE are never changed on its say-so.


def abi_sig(fn):
    """A declaration's identity for the majority vote: where each value
    travels (loc_of), not its spelling, so ABI-identical spellings vote
    together."""
    return (loc_of(fn["ret_cls"]), tuple(loc_of(c) for c in fn["param_cls"]),
            fn["variadic"], fn["noproto"] or fn["implicit"])


def sig_text(fn):
    return "%s (%s%s)" % (fn["ret"], ", ".join(fn["params"]),
                          ", ..." if fn["variadic"] else "")


def direction(dfn, site_list):
    """The rule's verdict for one FIXABLE symbol, with its evidence: whether
    the definition has its own C (a body with at least one statement — an
    empty `{}` arm is not C to fix toward) and the declaration-site tally."""
    agree = sum(1 for _, bad in site_list if not bad)
    tally = "declaration sites: %d agree, %d disagree with the definition" % (
        agree, len(site_list) - agree)
    defwhere = "%s:%s" % (rel(dfn["file"]), dfn["line"])
    if dfn.get("body_stmts", 0) > 0:
        return ("toward the DEFINITION — it has its own C (%s, %d statement(s)); %s"
                % (defwhere, dfn["body_stmts"], tally))
    votes = {}
    for d, _ in site_list:
        votes.setdefault(abi_sig(d), []).append(d)
    ranked = sorted(votes.values(), key=lambda v: -len(v))
    head = "the definition at %s has no C of its own (empty body), so the MAJORITY of " \
           "declaration sites decides; %s" % (defwhere, tally)
    if len(ranked) > 1 and len(ranked[0]) == len(ranked[1]):
        tied = [v for v in ranked if len(v) == len(ranked[0])]
        return "TIE — needs a ruling: %s; tied at %d site(s) each: %s" % (
            head, len(ranked[0]), " | ".join(sig_text(v[0]) for v in tied))
    return "toward the MAJORITY: %s; %d of %d site(s) declare %s" % (
        head, len(ranked[0]), len(site_list), sig_text(ranked[0][0]))


ORDER = {"DESIGN-CALL": 0, "FIXABLE": 1, "STALE-LEVER": 2, "DUPDEF": 3,
         "ANNOTATED": 4, "INFO": 5}
ERROR_STATES = ("DESIGN-CALL", "FIXABLE", "STALE-LEVER", "DUPDEF")


def fmt(r):
    return "%-11s %-3s %s  decl %s  def %s  %s" % r


def row_kinds(r):
    """The disagreement KINDS of a row, without their type text: `RETURN-UNSET
    s64 vs void` and `RETURN-UNSET s32 vs void` are one kind. A PARAM error
    keeps its POSITION (`PARAM 2`), which names WHICH parameter disagrees and
    carries no spelling. Sorted, so error order is not identity."""
    if r[0] in ("DUPDEF", "STALE-LEVER"):
        return (r[0],)
    out = []
    for e in r[5].split("; "):
        m = re.match(r"(PARAM \d+|[A-Z][A-Z-]*)", e)
        out.append(m.group(1) if m else e)
    return tuple(sorted(out))


def row_file(r):
    """The declaration's file, line numbers dropped; for DUPDEF every
    definition file (an unrelated edit above a declaration moves its line)."""
    return ", ".join(re.sub(r":\d+$", "", s) for s in r[3].split(", "))


def site_key(r):
    """A row's identity across two trees: (region, symbol, file, KIND) — task
    #1425, watcher-2's decision 1 on #1414's Q1. Not the type text: #1414
    DEMONSTRATED that keying on it reads a RE-SPELLING of a pre-existing
    mismatch as NEW (#1389's s64 -> s32 on func_00133850: `NEW RETURN-UNSET
    s32 vs void`, `GONE s64 vs void` x2, diff FAIL, on a landing that
    introduced no mismatch). The type text stays in the PRINTED row; it only
    no longer takes part in identity. No line number (an unrelated edit above
    a declaration moves it), no status (a promotion between the trees turns
    FIXABLE into DESIGN-CALL without the disagreement changing).

    COLLISION (chosen, task #1425): two sites in ONE file disagreeing with the
    same symbol's definition by the same KIND share a key. They are COUNTED,
    not collapsed (a multiset; see diff_rows), so a THIRD such site in that
    file reads as NEW. What the key cannot see is a SWAP inside one file: one
    such site fixed and another of the same kind added in the same landing
    nets to zero and is not NEW. Keying on the line instead would make every
    unrelated edit above a declaration a false NEW, which is the failure this
    key exists to remove; the swap is the price, and the full run (no --base)
    still lists every site."""
    return (r[1], r[2], row_file(r), row_kinds(r))


def site_key_text(r):
    """The PRE-#1425 key, (region, symbol, file, full disagreement TEXT). Kept
    ONLY as `--key text`, so --selftest and the landing gate's selftest can
    show their re-spelling fixture is one the old key FAILS — without that
    control, the new key passing it would prove nothing. Never a gate key."""
    return (r[1], r[2], r[3].rsplit(":", 1)[0], r[5])


KEYS = {"kind": site_key, "text": site_key_text}


def diff_rows(base_rows, tip_rows, key=site_key):
    """(grown, shrunk, nbase): the error rows of both trees grouped by key and
    compared as a MULTISET of keys. `grown` lists every key whose tip count
    exceeds its base count as (key, base count, tip count, EVERY tip site);
    `shrunk` lists every key whose count fell as (key, base count, tip count,
    EVERY base site). nbase is the base's error-row total.

    Every site of a changed key is returned, never a chosen one (task #1461).
    Sites sharing a key are indistinguishable to the key by construction, so
    "which is the added one" is not decidable here; the old form let base rows
    absorb tip rows in tip order and so always blamed the key's LAST site —
    #1450 seeded a third func_0011AEA0 declaration at 250080.cpp:265 and the
    gate named the pre-existing :678 (FACT #9147, at master dbf91765); #1461
    re-measured it at 83ff96b3: added :265, named :664 (base :663)."""
    sites = ({}, {})
    for rows, into in zip((base_rows, tip_rows), sites):
        for r in rows:
            if r[0] in ERROR_STATES:
                into.setdefault(key(r), []).append(r)
    base, tip = sites
    grown = [(k, len(base.get(k, ())), len(v), v) for k, v in tip.items()
             if len(v) > len(base.get(k, ()))]
    shrunk = [(k, len(v), len(tip.get(k, ())), v) for k, v in base.items()
              if len(v) > len(tip.get(k, ()))]
    return grown, shrunk, sum(len(v) for v in base.values())


def fmt_key(k, key="kind"):
    return "%s %s  decl %s  %s" % (k[0], k[1], k[2], "; ".join(k[3]) if key == "kind" else k[3])


def diff_report(grown, shrunk, key="kind"):
    """The --base report: one NEW header per grown key, then EVERY tip site of
    that key (`  site `, tip line numbers); one GONE header per shrunk key,
    then EVERY base site of it (`  was  `, BASE line numbers). The header
    carries the key's base -> tip count, so a third same-kind site reads
    `+1 ... count base 2 -> tip 3` over all three sites, the added one among
    them. A same-file same-kind SWAP leaves the count unchanged and prints
    nothing (the blind spot site_key documents); when it rides with a further
    add, the swapped-in site is listed under the NEW header too."""
    out = []
    for k, nb, nt, rows in grown:
        out.append("NEW  +%d  %s  [count base %d -> tip %d: %s]" % (
            nt - nb, fmt_key(k, key), nb, nt, "every site is new" if nb == 0 else
            "the key cannot tell which %d of these %d same-kind sites %s the added one%s; "
            "all are listed" % (nt - nb, nt, "is" if nt - nb == 1 else "are",
                                "" if nt - nb == 1 else "s")))
        out += ["  site " + fmt(r) for r in rows]
    for k, nb, nt, rows in shrunk:
        out.append("GONE -%d  %s  [count base %d -> tip %d: %s; base line numbers]" % (
            nb - nt, fmt_key(k, key), nb, nt, "every site is gone" if nt == 0 else
            "%d of these %d base sites, not decidable which; all are listed" % (nb - nt, nb)))
        out += ["  was  " + fmt(r) for r in rows]
    return out


def lint(units, show_info=False):
    npass, rows, dirs = lint_rows(units)
    if npass is None:
        print(rows)
        print("#### decl-def-lint: UNRUNNABLE (fails closed)")
        return 2
    counts = {}
    for r in rows:
        counts[r[0]] = counts.get(r[0], 0) + 1
        if r[0] != "INFO" or show_info:
            print(fmt(r))
        if r[0] == "FIXABLE":
            print("            direction: " + dirs[r[1], r[2]])
    if counts.get("FIXABLE"):
        print("--- " + DIRECTION_RULE)
    bad = sum(counts.get(k, 0) for k in ERROR_STATES)
    print("--- decl-def-lint: units=%d%s %s ---" % (
        npass, " requested=%d" % len(units) if units else "",
        " ".join("%s=%d" % (k, counts.get(k, 0)) for k in ORDER)))
    print("#### decl-def-lint: %s" % ("FAIL" if bad else "PASS"))
    return 1 if bad else 0


def lint_diff(base, units, repo=None, key="kind"):
    """Error rows at the tip (this tree) that the base tree does not have,
    compared as a MULTISET of site keys (diff_rows). The base tree is `git
    archive`d from <base> in REPO (default: this tree) so its own check.sh,
    flags and unit list are used; both arms are linted by THIS script."""
    global ROOT
    tip_root = ROOT
    tmp = tempfile.mkdtemp(prefix="ddl-base.")
    try:
        arch = subprocess.run(["git", "-C", repo or tip_root, "archive", "--format=tar", base, "--",
                               "going-decompiled/src", "going-decompiled/include",
                               "tools/native"], stdout=subprocess.PIPE)
        if arch.returncode != 0:
            print("#### decl-def-lint: UNRUNNABLE (cannot archive base %s; fails closed)" % base)
            return 2
        subprocess.run(["tar", "-x", "-C", tmp], input=arch.stdout, check=True)
        n_tip, tip, _ = lint_rows(units)
        ROOT = tmp
        n_base, base_rows, _ = lint_rows([u.replace(tip_root, tmp, 1) for u in units])
    finally:
        ROOT = tip_root
        shutil.rmtree(tmp, ignore_errors=True)
    if n_tip is None or n_base is None:
        print(tip if n_tip is None else base_rows)
        print("#### decl-def-lint: UNRUNNABLE (fails closed)")
        return 2
    grown, shrunk, nb = diff_rows(base_rows, tip, KEYS[key])
    for line in diff_report(grown, shrunk, key):
        print(line)
    print("--- decl-def-lint diff: base %s units=%d errors=%d -> tip units=%d errors=%d; "
          "new=%d gone=%d; key=%s ---" % (base, n_base, nb, n_tip,
                                          sum(1 for r in tip if r[0] in ERROR_STATES),
                                          sum(nt - b for _, b, nt, _ in grown),
                                          sum(b - nt for _, b, nt, _ in shrunk), key))
    print("#### decl-def-lint diff: %s" % ("FAIL" if grown else "PASS"))
    return 1 if grown else 0


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


def _tree(tmp, use_unit, def_unit, extra=None):
    """A throwaway tree: the REAL check.sh and two units, def.c and use.c,
    plus any `extra` {basename: text} units."""
    os.makedirs(os.path.join(tmp, "tools", "native"))
    for f in ("check.sh", "mips_callees.h"):
        shutil.copy(os.path.join(ROOT, "tools", "native", f),
                    os.path.join(tmp, "tools", "native", f))
    src = os.path.join(tmp, "going-decompiled", "src", "usa", "text")
    os.makedirs(src)
    os.makedirs(os.path.join(tmp, "going-decompiled", "include"))
    with open(os.path.join(src, "def.c"), "w") as fh:
        fh.write("/* TARGET_NATIVE unit */\n" + def_unit)
    for f, text in [("use.c", use_unit)] + sorted((extra or {}).items()):
        with open(os.path.join(src, f), "w") as fh:
            fh.write("/* TARGET_NATIVE unit */\n" + text)
    return src


def _quiet(fn, *a, **k):
    import io
    import contextlib
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rv = fn(*a, **k)
    return rv, buf.getvalue()


def _verdict(name, rc, expect_rc, out, expect_rows, absent):
    ok = rc == expect_rc and all(re.search(r, out, re.M) for r in expect_rows) \
        and not any(re.search(r, out, re.M) for r in absent)
    print("%s %s: rc %d (want %d)" % ("OK  " if ok else "SELFTEST-FAIL", name, rc, expect_rc))
    if not ok:
        print("    " + out.replace("\n", "\n    "))
    return ok


def _arm(name, use_unit, expect_rc, expect_rows, absent=(), def_unit=DEF_UNIT, units=(),
         extra=None):
    """`units` are basenames under the fixture's src/usa/text (or any path,
    taken as given) passed as the explicit unit arguments."""
    global ROOT
    tip_root = ROOT
    tmp = tempfile.mkdtemp(prefix="ddl-st.")
    try:
        src = _tree(tmp, use_unit, def_unit, extra)
        ROOT = tmp
        rc, out = _quiet(lint, [os.path.join(src, u) for u in units])
    finally:
        ROOT = tip_root
        shutil.rmtree(tmp, ignore_errors=True)
    return _verdict(name, rc, expect_rc, out, expect_rows, absent)


def _diff_arm(name, base_use, tip_use, key, expect_new, expect_rows=(), absent=()):
    """The --base differential's comparison (diff_rows) on two fixture trees
    that differ only in use.c. rc is 1 when a row is NEW, as lint_diff's."""
    global ROOT
    tip_root = ROOT
    trees = []
    try:
        got = []
        for use in (base_use, tip_use):
            tmp = tempfile.mkdtemp(prefix="ddl-st.")
            trees.append(tmp)
            _tree(tmp, use, DEF_UNIT)
            ROOT = tmp
            got.append(lint_rows([]))
            ROOT = tip_root
    finally:
        ROOT = tip_root
        for t in trees:
            shutil.rmtree(t, ignore_errors=True)
    if got[0][0] is None or got[1][0] is None:
        return _verdict(name, 2, 1 if expect_new else 0, str(got), (), ())
    grown, shrunk, _ = diff_rows(got[0][1], got[1][1], KEYS[key])
    out = "".join(line + "\n" for line in diff_report(grown, shrunk, key))
    return _verdict(name, 1 if grown else 0, 1 if expect_new else 0, out, expect_rows, absent)


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
    permuted = ("void Matched(float *dst, const float *src, float len);\n"
                "void f(float *a) { Matched(a, a, 2.0f); }\n")
    # task #1425, PER-UNIT MODE. Before it, explicit units narrowed the PARSE,
    # so a declaration whose definition sat in an unnamed unit was compared
    # with nothing: s13 printed `units=1`, 0 rows, PASS (#1424's fail-open).
    arms += [
        ("s13 per-unit: ONLY the declaring unit named -> the row is still found, FAIL",
         permuted, 1, [r"^DESIGN-CALL usa Matched  decl going-decompiled/src/usa/text/use.c:2 .* ORDER",
                       r"units=2 requested=1 "], (), DEF_UNIT, ("use.c",)),
        ("s14 per-unit: ONLY the defining unit named -> the row is found from that side, FAIL",
         permuted, 1, [r"^DESIGN-CALL usa Matched  decl going-decompiled/src/usa/text/use.c:2 "],
         (), DEF_UNIT, ("def.c",)),
        ("s15 per-unit control: a unit with no disagreement named -> its rows only, PASS",
         permuted, 0, [r"^#### decl-def-lint: PASS", r"units=3 requested=1 "], [r"^DESIGN-CALL"],
         DEF_UNIT, ("use2.c",), {"use2.c": "void g(void) { Sink(1); }\n"}),
        ("s16 per-unit: a unit outside the population -> UNRUNNABLE rc 2, never an empty PASS",
         permuted, 2, [r"not in check.sh's TARGET_NATIVE population.*nope\.c",
                       r"^#### decl-def-lint: UNRUNNABLE"], (), DEF_UNIT, ("nope.c",)),
    ]
    # task #1425, FIXABLE DIRECTION RULE (watcher-2 decision 2): reported, with
    # its evidence, never applied. EMPTY: an #else arm with no C of its own.
    empty = DEF_UNIT.replace("void Unmatched(float *dst, float len, const float *src) "
                             "{ dst[0] = src[0] * len; }",
                             "void Unmatched(float *dst, float len, const float *src) {}")
    agree = ("void Unmatched(float *dst, float len, const float *src);\n"
             "void g1(float *a) { Unmatched(a, 1.0f, a); }\n")
    swap = ("void Unmatched(float *dst, const float *src, float len);\n"
            "void g2(float *a) { Unmatched(a, a, 1.0f); }\n")
    arms += [
        ("s17 direction: the definition has its own C -> toward the DEFINITION, with the tally",
         swap, 1, [r"^FIXABLE     usa Unmatched ", r"^ +direction: toward the DEFINITION — it has its own "
                   r"C \(going-decompiled/src/usa/text/def\.c:\d+, 1 statement\(s\)\); declaration "
                   r"sites: 1 agree, 1 disagree", r"^--- DIRECTION RULE for FIXABLE rows"],
         (), DEF_UNIT, (), {"use2.c": agree}),
        ("s18 direction: empty definition, 1 site vs 1 site -> TIE, needs a ruling (never resolved)",
         swap, 1, [r"^ +direction: TIE — needs a ruling: the definition at .* has no C of its own"],
         [r"direction: toward"], empty, (), {"use2.c": agree}),
        ("s19 direction: empty definition, 2 sites vs 1 -> toward the MAJORITY, even against the "
         "definition's spelling",
         swap, 1, [r"^ +direction: toward the MAJORITY: .* 2 of 3 site\(s\) declare void "
                   r"\(float \*, const float \*, float\)"],
         [r"TIE"], empty, (), {"use2.c": agree, "use3.c": swap.replace("g2", "g3")}),
    ]
    ok = all([_arm(*a) for a in arms])
    # task #1425, THE --base KEY (watcher-2 decision 1): both directions, and
    # the old key on the re-spelling as the control that shows the fixture
    # discriminates (a new key passing it proves nothing if the old one did).
    s64 = "long long Sink(int x);\nlong long f(void) { return Sink(1); }\n"
    s32 = "int Sink(int x);\nint f(void) { return Sink(1); }\n"
    diffs = [
        ("d1 a pure RE-SPELLING of a mismatch (s64 -> s32 vs a void def, #1389's shape) -> "
         "no NEW, PASS", s64, s32, "kind", False, (), [r"^NEW"]),
        ("d2 control: the same re-spelling under the OLD text key -> NEW, FAIL (#1414 Q1)",
         s64, s32, "text", True, [r"^NEW  \+1  usa Sink .* \[count base 0 -> tip 1: every site is new\]$",
                                  r"^  site DESIGN-CALL usa Sink .* RETURN-UNSET int vs void$"]),
        ("d3 a genuinely NEW mismatch (an agreeing declaration made value-returning) -> NEW, FAIL",
         "void Sink(int x);\nvoid f(void) { Sink(1); }\n", s32, "kind", True,
         [r"^NEW  \+1  usa Sink  decl going-decompiled/src/usa/text/use\.c  RETURN-UNSET  "
          r"\[count base 0 -> tip 1: every site is new\]$",
          r"^  site DESIGN-CALL usa Sink  decl going-decompiled/src/usa/text/use\.c:2 .* RETURN-UNSET"]),
        ("d4 a SECOND site of the same kind in the same file -> counted, NEW, FAIL",
         s32, s32 + "int g(void) { extern int Sink(int); return Sink(2); }\n", "kind", True,
         [r"^NEW  \+1  usa Sink  decl going-decompiled/src/usa/text/use\.c  RETURN-UNSET  \[count base 1 -> tip 2: ",
          r"^  site DESIGN-CALL usa Sink  decl going-decompiled/src/usa/text/use\.c:2 ",
          r"^  site DESIGN-CALL usa Sink  decl going-decompiled/src/usa/text/use\.c:4 "]),
        ("d5 control: an unrelated line inserted above (the site moves) -> no NEW, PASS",
         s32, "/* moved */\n" + s32, "kind", False, (), [r"^NEW"]),
        # task #1461, ATTRIBUTION. d4 adds its site LAST, which is where the old
        # tip-order absorption happened to blame, so d4 passed with the defect
        # live. d6 adds the site FIRST (#1450's 250080.cpp:265 shape): the old
        # report named only the pre-existing :3. The assertion is on the ROW
        # TEXT — the added :2 must appear, with the counts.
        ("d6 a same-kind site added ABOVE an existing one -> NEW lists the ADDED site, with "
         "base -> tip counts, FAIL",
         s32, "int g(void) { extern int Sink(int); return Sink(2); }\n" + s32, "kind", True,
         [r"^NEW  \+1  usa Sink  decl going-decompiled/src/usa/text/use\.c  RETURN-UNSET  "
          r"\[count base 1 -> tip 2: the key cannot tell which 1 of these 2 same-kind sites is the added "
          r"one; all are listed\]$",
          r"^  site DESIGN-CALL usa Sink  decl going-decompiled/src/usa/text/use\.c:2 .* RETURN-UNSET int vs void$",
          r"^  site DESIGN-CALL usa Sink  decl going-decompiled/src/usa/text/use\.c:3 .* RETURN-UNSET int vs void$"]),
        ("d7 GONE, symmetric: one of two same-kind sites removed -> no NEW, PASS; GONE lists every "
         "BASE site with counts",
         s32 + "int g(void) { extern int Sink(int); return Sink(2); }\n", s32, "kind", False,
         [r"^GONE -1  usa Sink  decl going-decompiled/src/usa/text/use\.c  RETURN-UNSET  "
          r"\[count base 2 -> tip 1: 1 of these 2 base sites, not decidable which; all are listed; "
          r"base line numbers\]$",
          r"^  was  DESIGN-CALL usa Sink  decl going-decompiled/src/usa/text/use\.c:2 ",
          r"^  was  DESIGN-CALL usa Sink  decl going-decompiled/src/usa/text/use\.c:4 "], [r"^NEW"]),
        ("d8 KNOWN BLIND SPOT (site_key, kept): a same-file same-kind SWAP (one site removed, "
         "another added) nets to zero -> no NEW, no GONE, PASS",
         s32 + "int g(void) { extern int Sink(int); return Sink(2); }\n",
         "int g(void) { extern int Sink(int); return Sink(2); }\n"
         "int h(void) { extern int Sink(int); return Sink(3); }\n", "kind", False, (),
         [r"^NEW", r"^GONE"]),
    ]
    ok = all([_diff_arm(*d) for d in diffs]) and ok
    n = len(arms) + len(diffs)
    print("#### decl-def-lint selftest: %s (%d arms)" % ("PASS" if ok else "FAIL", n))
    return 0 if ok else 1


def main(argv):
    if argv and argv[0] == "--cc-shim":
        return cc_shim(argv[1:])
    if argv == ["--selftest"]:
        return selftest()
    units, base, info, repo, key = [], None, False, None, "kind"
    it = iter(argv)
    for a in it:
        if a == "--info":
            info = True
        elif a == "--base":
            base = next(it)
        elif a == "--repo":
            repo = next(it)
        elif a == "--key":
            key = next(it)
            if key not in KEYS:
                print(__doc__)
                return 2
        elif a.startswith("-"):
            print(__doc__)
            return 2
        else:
            units.append(os.path.abspath(a))
    if base:
        return lint_diff(base, units, repo=repo, key=key)
    if repo or key != "kind":
        print("--repo and --key apply only to --base")
        return 2
    return lint(units, show_info=info)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

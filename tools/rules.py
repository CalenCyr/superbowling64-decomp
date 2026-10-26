#!/usr/bin/env python3
"""Rule checker for the decomp-code-rules catalogue (the `rules` verb in AGENTS.md).

Usage:
    python3 tools/rules.py [paths...] [--json OUT] [--report report.json] [--no-advisory]

With no paths it checks every `src/**/*.c`. Headers under `include/` are always read, for the
cross-file rules (R1, R2) and for findings in types and globals that a function uses.

Output: one line per finding, `file:line: Rnn [match|advisory] message`, then a summary. A function
with any [match] finding is MATCH-PENDING once its bytes are exact; `--report` (an objdiff report)
restricts the MATCH-PENDING list to exact functions, otherwise every failing C function is listed.
The exit status is 1 when any [match] finding exists. `--json` writes the findings and the per-function
verdicts for `tools/progress.py --rules`.

`INCLUDE_ASM` stubs are the parking mechanism: they are unmatched work, never findings.

What it checks, per function body (findings in a type, global or macro are charged to every function
that names it):

  R1  [match]  a struct/union/enum/typedef name defined in two files; a struct/union keyword mismatch
               between declarations; a layout view (a struct whose members copy a prefix of another
               struct's members) used in a cast.
  R2  [match]  an `extern` object declaration outside a header (file scope or block scope); one global
               declared with different types; a global defined in a header, defined in two units, or
               defined without an `extern` in a header (file-scope `static` is unit-private and fine).
  R4  [match]  a byte-pointer cast plus a constant offset under a pointer cast (`*(T *)((u8 *)p + 0x10)`)
               or directly dereferenced/indexed; an offset macro (`#define` holding that pattern).
  R13 [match]  hook only: C has no vtables. In C++ sources a virtual named pad/dummy would fire.
  R17 [match]  `volatile` without a `// volatile: <evidence>` note on the line or the line above; locals
               never used, or only ever assigned (unless the declaration carries an `R36` note); empty
               if/else/plain blocks; bare `;` statements (empty loop bodies, `case X: ;`, `label: ;` are
               allowed); `do { } while (0)` in a function body; self-assignment; `+ 0`, `- 0`, `* 1`,
               `/ 1`; padding members (`pad`, `_pad3`, `padding`, `filler`, `dummy`) without a
               `// pad: <evidence>` note naming the SDK or file format.
  R18 [match]  `asm`/`__asm`/`__asm__` outside `INCLUDE_ASM`, unless the function or macro is listed in
               config/rules.txt as `asm-helper NAME` (counted, listed) or `original-asm NAME` (listed
               apart, counted neither way).

Advisory (never fail a match): R7 placeholder and address-like names (function, parameters, locals,
globals, types and members referenced); R8 Table 1 style (K&R braces, spaces inside parentheses,
`Prefix_VerbNoun` functions, `lowerCamel` locals); R33 missing header comment; R35 braceless bodies;
R25 block-scope `extern` of a function; R37 `#if 0`.

What it cannot detect (Section F of the catalogue; these need the reader test by hand):
  - one variable reused for unrelated values (R30), or a meaningless temporary that only splits an
    expression for the compiler;
  - a dummy local that is read, but only to burn a register; statements that exist only to steer the
    compiler but look meaningful (reordered statements, comma chains, assignments hidden in arguments);
  - `register` chosen to steer allocation; casts to a variable's own type or cast chains (needs types);
  - `goto` into loops or labels used only to reorder blocks;
  - offset arithmetic through a pointer already typed `u8 *` (`*(s32 *)(buf + 8)`), which is genuine in a
    file parser and fake on a struct; `void *`/`int` used to dodge a type;
  - arrays sized to steer layout; struct copies used without a cast; wrong referents (wrong function or
    global with identical bytes);
  - anything produced by macro expansion: sources are not preprocessed, so a macro is checked once, at
    its `#define`, and charged to the functions that use it.
"""
import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CONFIG = ROOT / "config" / "rules.txt"

# ---------------------------------------------------------------- lexer

TOKEN_RE = re.compile(r"""
    (?P<ws>[ \t\r\f\v]+)
  | (?P<nl>\n)
  | (?P<comment>//[^\n]*|/\*.*?\*/)
  | (?P<string>"(?:\\.|[^"\\\n])*"|'(?:\\.|[^'\\\n])*')
  | (?P<number>(?:0[xX][0-9a-fA-F]+|\d+\.?\d*(?:[eE][+-]?\d+)?|\.\d+(?:[eE][+-]?\d+)?)[uUlLfF]*)
  | (?P<ident>[A-Za-z_]\w*)
  | (?P<op>\.\.\.|<<=|>>=|->|\+\+|--|<<|>>|<=|>=|==|!=|&&|\|\||[-+*/%&|^!=<>]=|[{}()\[\];,.?:~!%^&|*/+\-<>=#\\])
""", re.S | re.X)


class Tok:
    __slots__ = ("kind", "text", "line", "col", "space_after")

    def __init__(self, kind, text, line, col):
        self.kind, self.text, self.line, self.col = kind, text, line, col
        self.space_after = False

    def __repr__(self):
        return f"{self.text}@{self.line}"


def blank_non_matching(text):
    """Blank the `#ifdef NON_MATCHING` branch (the parked draft; the project's parking mechanism, exempt from every
    rule) up to its `#else` or `#endif`, keeping line numbers. The `#else` branch, normally `INCLUDE_ASM`, stays."""
    lines = text.split("\n")
    depth = 0           # nesting inside a blanked branch; 0 = not blanking
    for i, l in enumerate(lines):
        d = l.strip()
        if depth == 0:
            if re.match(r"#\s*ifdef\s+NON_MATCHING\b", d):
                depth = 1
            continue
        if re.match(r"#\s*if", d):
            depth += 1
        elif re.match(r"#\s*endif\b", d):
            depth -= 1
            if depth == 0:
                continue
        elif depth == 1 and re.match(r"#\s*(else|elif)\b", d):
            depth = 0
            continue
        lines[i] = ""
    return "\n".join(lines)


class Source:
    """A file split into code tokens (preprocessor lines removed), comments by line, and directives."""

    def __init__(self, path, text=None):
        self.path = path
        self.rel = rel(path)
        self.text = blank_non_matching(path.read_text(errors="replace") if text is None else text)
        self.lines = self.text.split("\n")
        self.comments = {}          # line -> [comment text]
        self.directives = []        # (line, text) with continuations joined
        self.tokens = []
        self._lex()

    def _lex(self):
        # Pull out preprocessor directives first (keeping line numbers), blanking them in the code.
        code_lines = list(self.lines)
        i = 0
        while i < len(code_lines):
            if code_lines[i].lstrip().startswith("#"):
                start, parts = i, []
                while True:
                    parts.append(code_lines[i])
                    cont = code_lines[i].rstrip().endswith("\\")
                    code_lines[i] = ""
                    if not cont or i + 1 >= len(code_lines):
                        break
                    i += 1
                self.directives.append((start + 1, "\n".join(parts)))
            i += 1
        code = "\n".join(code_lines)
        line, col, pos = 1, 0, 0
        prev = None
        while pos < len(code):
            m = TOKEN_RE.match(code, pos)
            if not m:
                m = re.compile(r".").match(code, pos)
                kind = "op"
            else:
                kind = m.lastgroup
            text = m.group()
            if kind == "comment":
                self.comments.setdefault(line, []).append(text)
            elif kind in ("ws", "nl"):
                if prev is not None:
                    prev.space_after = True
            else:
                prev = Tok(kind, text, line, col)
                self.tokens.append(prev)
            nl = text.count("\n")
            if nl:
                line += nl
                col = len(text) - text.rfind("\n") - 1
            else:
                col += len(text)
            pos = m.end()
        for dline, dtext in self.directives:
            for k, ln in enumerate(dtext.split("\n")):
                for c in re.findall(r"//[^\n]*|/\*.*?\*/", ln):
                    self.comments.setdefault(dline + k, []).append(c)

    def note(self, line, pattern):
        """A comment matching `pattern` on `line` or the line above."""
        for ln in (line, line - 1):
            for c in self.comments.get(ln, []):
                if re.search(pattern, c):
                    return True
        return False


def rel(path):
    try:
        return str(Path(path).resolve().relative_to(ROOT))
    except ValueError:
        return str(path)


def match_close(toks, i):
    """Index of the bracket closing toks[i]."""
    pairs = {"(": ")", "[": "]", "{": "}"}
    o, c = toks[i].text, pairs[toks[i].text]
    depth = 0
    for j in range(i, len(toks)):
        if toks[j].text == o:
            depth += 1
        elif toks[j].text == c:
            depth -= 1
            if depth == 0:
                return j
    return len(toks) - 1


def match_open(toks, j):
    pairs = {")": "(", "]": "[", "}": "{"}
    c, o = toks[j].text, pairs[toks[j].text]
    depth = 0
    for i in range(j, -1, -1):
        if toks[i].text == c:
            depth += 1
        elif toks[i].text == o:
            depth -= 1
            if depth == 0:
                return i
    return 0


# ---------------------------------------------------------------- C vocabulary

BASE_TYPES = {"void", "char", "short", "int", "long", "float", "double", "signed", "unsigned", "_Bool"}
QUALIFIERS = {"const", "volatile", "register", "static", "extern", "auto", "inline", "__inline__",
              "__inline", "typedef", "restrict"}
TAG_KW = {"struct", "union", "enum"}
STMT_KW = {"if", "else", "while", "for", "do", "switch", "case", "default", "return", "break", "continue",
           "goto", "sizeof"}
ASM_TOKENS = {"asm", "__asm", "__asm__"}
BYTE_TYPES = [("char",), ("u8",), ("s8",), ("unsigned", "char"), ("signed", "char"), ("BYTE",), ("uchar",)]

PAD_RE = re.compile(r"^_*(pad|padding|filler|dummy)(_?[0-9A-Fa-fx]*)$", re.I)
# R7 placeholders: the project profile's splat defaults plus the catalogue's generic list.
PLACEHOLDER_RE = re.compile(
    r"^(func_[0-9A-Fa-f]{8}|D_[0-9A-Fa-f]{8}|jtbl_[0-9A-Fa-f]{8}|L[0-9A-Fa-f]{8})(_\w+)?$"
    r"|^(FUN_|DAT_|sub_|unk_|unk[0-9A-F]|Unk|local_|param_|field_0x|uStack|iVar|m_Unk|UnkType_)"
    r"|^(var|temp|tmp|ptr|result|v)\d+$")
ADDRESS_RE = re.compile(r"\b[A-Za-z_]*?_?(?=[0-9a-fA-F]*[0-9])[0-9a-fA-F]{6,8}\b")
LOWER_CAMEL_RE = re.compile(r"^[a-z][A-Za-z0-9]*$")
FUNC_STYLE_RE = re.compile(r"^([A-Z][A-Za-z0-9]*_)?[A-Z][A-Za-z0-9]*(_f)?$")
NO_SPACE_AFTER = {"if", "while", "for", "switch", "return", "sizeof"}


def placeholder(name):
    return bool(PLACEHOLDER_RE.search(name) or ADDRESS_RE.fullmatch(name))


# ---------------------------------------------------------------- findings

class Finding:
    def __init__(self, src, line, rule, tier, msg, func=None):
        self.file, self.line, self.rule, self.tier, self.msg, self.func = src, line, rule, tier, msg, func

    def text(self):
        where = f"{self.func}: " if self.func else ""
        return f"{self.file}:{self.line}: {self.rule} [{self.tier}] {where}{self.msg}"

    def as_dict(self):
        return {"file": self.file, "line": self.line, "rule": self.rule, "tier": self.tier,
                "function": self.func, "message": self.msg}


# ---------------------------------------------------------------- top-level parse

class Decl:
    """A file-scope declaration: object, function prototype, typedef, or tag definition."""

    def __init__(self, src, toks):
        self.src, self.toks = src, toks
        self.line = toks[0].line if toks else 0


class Function:
    def __init__(self, src, name, line, head, body, open_i, close_i):
        self.src, self.name, self.line = src, name, line
        self.head, self.body = head, body       # token lists; body excludes outer braces
        self.open_i, self.close_i = open_i, close_i
        self.findings = []
        self.params = []
        self.locals = []                        # (name, tok, decl line)
        self.refs = set()


class Unit:
    def __init__(self, src, known_types):
        self.src = src
        self.functions, self.stubs, self.decls = [], [], []
        self.tag_defs = []          # (kind, name, line, body tokens, member list)
        self.typedefs = []          # (name, line, normalised)
        self.toplevel_asm = []      # line
        self.known_types = known_types
        self._parse()

    def _parse(self):
        toks = self.src.tokens
        i, start = 0, 0
        n = len(toks)
        while i < n:
            t = toks[i]
            if t.text in ("INCLUDE_ASM", "INCLUDE_RODATA") and i + 1 < n and toks[i + 1].text == "(":
                j = match_close(toks, i + 1)
                args = [x.text for x in toks[i + 2:j]]
                name = args[-1] if args else "?"
                if t.text == "INCLUDE_ASM":
                    self.stubs.append((name, t.line))
                i = j + 1
                if i < n and toks[i].text == ";":
                    i += 1
                start = i
                continue
            if t.text in ASM_TOKENS and i == start:
                self.toplevel_asm.append(t.line)
                j = match_close(toks, i + 1) if i + 1 < n and toks[i + 1].text == "(" else i
                while j < n and toks[j].text != ";":
                    j += 1
                i = j + 1
                start = i
                continue
            if t.text == "{":
                # function definition: the statement so far ends with `)` (or K&R parameter declarations)
                head = toks[start:i]
                fname = self._function_name(head)
                j = match_close(toks, i)
                if fname is not None:
                    fn = Function(self.src, fname[0], fname[1], head, toks[i + 1:j], i, j)
                    self.functions.append(fn)
                    i = j + 1
                    start = i
                    continue
                i = j + 1           # aggregate or initializer body; keep the statement going
                continue
            if t.text == ";":
                self.decls.append(Decl(self.src, toks[start:i]))
                i += 1
                start = i
                continue
            i += 1

    def _function_name(self, head):
        if not head or any(x.text == "=" for x in head):
            return None
        # find the first top-level `(` preceded by an identifier: NAME ( params )
        depth = 0
        for k, x in enumerate(head):
            if x.text == "(":
                if depth == 0 and k > 0 and head[k - 1].kind == "ident" \
                        and head[k - 1].text not in STMT_KW | TAG_KW:
                    close = match_close(head, k)
                    if close == len(head) - 1 or head[close + 1].kind == "ident":
                        return head[k - 1].text, head[k - 1].line
                    if head[close + 1].text == "(":
                        continue
                depth += 1
            elif x.text == ")":
                depth -= 1
            elif x.text in ("struct", "union", "enum") and k + 2 < len(head) and head[k + 2].text == "{":
                return None
        return None


def strip_quals(tokens):
    return [t for t in tokens if t.text not in QUALIFIERS]


def norm(tokens):
    return " ".join(t.text for t in tokens)


def split_top(tokens, sep=","):
    parts, cur, depth = [], [], 0
    for t in tokens:
        if t.text in "([{":
            depth += 1
        elif t.text in ")]}":
            depth -= 1
        if t.text == sep and depth == 0:
            parts.append(cur)
            cur = []
        else:
            cur.append(t)
    parts.append(cur)
    return parts


def type_prefix_len(tokens, known_types):
    """Length of the declaration-specifier prefix (storage, qualifiers, type) of a declaration."""
    k, seen_type = 0, False
    while k < len(tokens):
        x = tokens[k].text
        if x in QUALIFIERS or x == "__attribute__":
            if x == "__attribute__" and k + 1 < len(tokens) and tokens[k + 1].text == "(":
                k = match_close(tokens, k + 1)
            k += 1
            continue
        if x in TAG_KW:
            k += 1
            if k < len(tokens) and tokens[k].kind == "ident":
                k += 1
            if k < len(tokens) and tokens[k].text == "{":
                k = match_close(tokens, k) + 1
            seen_type = True
            continue
        if x in BASE_TYPES:
            seen_type = True
            k += 1
            continue
        if tokens[k].kind == "ident" and x in known_types and not seen_type:
            seen_type = True
            k += 1
            continue
        break
    return k if seen_type else 0


def declarator_name(tokens):
    """(name token, is_function, is_pointer_to_function) for one declarator."""
    depth = 0
    for k, t in enumerate(tokens):
        if t.text == "=" and depth == 0:
            break
        if t.text in "([":
            depth += 1
        elif t.text in ")]":
            depth -= 1
        elif t.kind == "ident" and t.text not in QUALIFIERS:
            nxt = tokens[k + 1].text if k + 1 < len(tokens) else ""
            is_func = nxt == "(" and depth == 0
            return t, is_func, depth > 0
    return None, False, False


def tag_members(body, known_types):
    """[(type text, name, tok)] for a struct body (tokens between braces)."""
    out = []
    for stmt in split_top(body, ";"):
        if not stmt:
            continue
        p = type_prefix_len(stmt, known_types)
        ty = norm(strip_quals(stmt[:p]))
        for d in split_top(stmt[p:]):
            nt, _, _ = declarator_name(d)
            if nt is not None:
                out.append((ty + " " + norm([x for x in d if x is not nt]), nt.text, nt))
    return out


# ---------------------------------------------------------------- the checker

class Checker:
    def __init__(self, targets, advisory=True):
        self.advisory = advisory
        self.allow = self._read_config()
        headers = sorted((ROOT / "include").rglob("*.h"))
        headers = [h for h in headers if h.name != "include_asm.h"]
        units = sorted((ROOT / "src").rglob("*.c"))
        self.target_paths = {Path(t).resolve() for t in targets}
        for t in targets:
            if Path(t).resolve() not in {u.resolve() for u in units}:
                units.append(Path(t).resolve())
        # known type names: a first pass over typedefs in every file
        self.known_types = {"u8", "s8", "u16", "s16", "u32", "s32", "u64", "s64", "f32", "f64"}
        sources = [Source(p) for p in headers + units]
        for s in sources:
            toks = s.tokens
            for k, t in enumerate(toks):
                if t.text == "typedef":
                    j, depth = k, 0
                    while j < len(toks):
                        if toks[j].text in "({[":
                            depth += 1
                        elif toks[j].text in ")}]":
                            depth -= 1
                        elif toks[j].text == ";" and depth == 0:
                            break
                        j += 1
                    stmt = toks[k:j]
                    p = type_prefix_len(stmt, self.known_types)
                    for d in split_top(stmt[p:]):
                        nt, _, _ = declarator_name(d)
                        if nt:
                            self.known_types.add(nt.text)
        self.units = [Unit(s, self.known_types) for s in sources]
        self.findings = []
        self.global_findings = {}   # identifier -> [Finding] charged to functions naming it

    def _read_config(self):
        allow = {"asm-helper": set(), "original-asm": set()}
        if CONFIG.exists():
            for ln in CONFIG.read_text().splitlines():
                ln = ln.split("#", 1)[0].split()
                if len(ln) == 2 and ln[0] in allow:
                    allow[ln[0]].add(ln[1])
        return allow

    # -- helpers
    def add(self, src, line, rule, tier, msg, func=None, charge=None):
        f = Finding(src.rel, line, rule, tier, msg, func.name if func else None)
        self.findings.append(f)
        if func is not None:
            func.findings.append(f)
        for name in charge or ():
            self.global_findings.setdefault(name, []).append(f)
        return f

    def is_header(self, src):
        return src.path.suffix == ".h"

    # -- cross-file rules
    def check_tags_and_typedefs(self):
        defs = {}       # name -> [(src, line, kind, normalised body)]
        kinds = {}      # tag name -> {kind: (src, line)}
        structs = {}    # name -> member list
        kinds_seen = []
        for u in self.units:
            toks = u.src.tokens
            depth = 0
            for k, t in enumerate(toks):
                if t.text in TAG_KW and k + 1 < len(toks):
                    nxt = toks[k + 1]
                    if nxt.kind == "ident":
                        kinds.setdefault(nxt.text, {}).setdefault(t.text, (len(kinds_seen), u.src, nxt.line))
                        kinds_seen.append(nxt.text)
                        if k + 2 < len(toks) and toks[k + 2].text == "{":
                            j = match_close(toks, k + 2)
                            body = toks[k + 3:j]
                            defs.setdefault(t.text + " " + nxt.text, []).append(
                                (u.src, nxt.line, norm(body)))
                            if t.text != "enum":
                                structs[nxt.text] = (u.src, nxt.line, tag_members(body, self.known_types))
                    if t.text != "enum" and (nxt.kind == "ident" or nxt.text == "{"):
                        b = k + 1 if nxt.text == "{" else k + 2
                        if b < len(toks) and toks[b].text == "{":
                            j = match_close(toks, b)
                            for ty, name, nt in tag_members(toks[b + 1:j], self.known_types):
                                if PAD_RE.match(name) and not u.src.note(nt.line, r"pad:\s*\S"):
                                    owner = self._typedef_after(toks, j) or (nxt.text if nxt.kind == "ident" else "?")
                                    self.add(u.src, nt.line, "R17", "match",
                                             f"padding member '{name}' in {owner} (name the bytes by offset, "
                                             f"or note `// pad: <SDK/format evidence>`)", charge=[owner])
                if t.text == "typedef":
                    j = k
                    while j < len(toks) and toks[j].text != ";":
                        if toks[j].text == "{":
                            j = match_close(toks, j)
                        j += 1
                    stmt = toks[k:j]
                    p = type_prefix_len(stmt, self.known_types)
                    for d in split_top(stmt[p:]):
                        nt, _, _ = declarator_name(d)
                        if nt:
                            body = norm(stmt[1:p]) + " " + norm([x for x in d if x is not nt])
                            defs.setdefault("typedef " + nt.text, []).append((u.src, nt.line, body))
                            if p >= 2 and stmt[p - 1].text == "}":
                                o = match_open(stmt, p - 1)
                                if nt.text not in structs:
                                    structs[nt.text] = (u.src, nt.line, tag_members(stmt[o + 1:p - 1], self.known_types))
        for key, places in defs.items():
            files = {s.rel for s, _, _ in places}
            if len(files) > 1:
                name = key.split()[-1]
                bodies = {b for _, _, b in places}
                what = "with different bodies" if len(bodies) > 1 else "twice"
                first = places[0]
                for s, line, _ in places[1:]:
                    self.add(s, line, "R1", "match",
                             f"{key} defined {what}: also {first[0].rel}:{first[1]}", charge=[name])
            elif len(places) > 1:
                pass  # repeated in one file: the compiler rejects it
        for name, ks in kinds.items():
            sk = {k for k in ks if k != "enum"}
            if len(sk) > 1:
                (_, s1, l1), (_, s2, l2) = sorted([ks["struct"], ks["union"]], key=lambda x: x[0])
                self.add(s2, l2, "R1", "match", f"'{name}' declared with both struct and union keywords "
                         f"(first at {s1.rel}:{l1})", charge=[name])
        self.structs = structs

    @staticmethod
    def _typedef_after(toks, close):
        if close + 1 < len(toks) and toks[close + 1].kind == "ident":
            return toks[close + 1].text
        return None

    def check_globals(self):
        decls = {}      # name -> [(src, line, extern?, type text, is header, static)]
        for u in self.units:
            for d in u.decls:
                toks = d.toks
                if not toks or toks[0].text == "typedef":
                    continue
                p = type_prefix_len(toks, self.known_types)
                if p == 0:
                    continue
                spec = [t.text for t in toks[:p]]
                ext, static = "extern" in spec, "static" in spec
                base = norm(strip_quals(toks[:p]))
                for part in split_top(toks[p:]):
                    nt, is_func, _ = declarator_name(part)
                    if nt is None or is_func:
                        continue
                    ty = base + " " + norm([x for x in part if x is not nt]).split(" = ")[0]
                    ty = re.sub(r"\[\s*[^\]]*\]", "[]", ty)
                    decls.setdefault(nt.text, []).append((u.src, nt.line, ext, ty.strip(), self.is_header(u.src), static))
        for name, places in decls.items():
            externs_h = [p for p in places if p[2] and p[4]]
            for s, line, ext, ty, hdr, static in places:
                if ext and not hdr:
                    self.add(s, line, "R2", "match", f"extern '{name}' outside a header", charge=[name])
                if not ext and hdr:
                    self.add(s, line, "R2", "match", f"global '{name}' defined in a header", charge=[name])
            defs = [p for p in places if not p[2] and not p[4] and not p[5]]
            if len(defs) > 1:
                for s, line, *_ in defs[1:]:
                    self.add(s, line, "R2", "match",
                             f"global '{name}' defined in two units (also {defs[0][0].rel}:{defs[0][1]})",
                             charge=[name])
            if defs and not externs_h:
                s, line, *_ = defs[0]
                self.add(s, line, "R2", "match", f"global '{name}' has no extern in a header (or make it static)",
                         charge=[name])
            types = {}
            for s, line, ext, ty, *_ in places:
                types.setdefault(ty, (s, line))
            if len(types) > 1:
                (t0, (s0, l0)), *rest = types.items()
                for ty, (s, line) in rest:
                    self.add(s, line, "R2", "match",
                             f"'{name}' declared as '{ty}' here and as '{t0}' at {s0.rel}:{l0}", charge=[name])

    def check_volatile_globals(self):
        """volatile in file-scope declarations and members (function bodies are checked per function)."""
        for u in self.units:
            body_ranges = [(f.open_i, f.close_i) for f in u.functions]
            toks = u.src.tokens
            for k, t in enumerate(toks):
                if t.text != "volatile" or any(a <= k <= b for a, b in body_ranges):
                    continue
                if u.src.note(t.line, r"volatile:\s*\S"):
                    continue
                # charge the declared name (or the enclosing typedef/tag) to its users
                j = k
                while j < len(toks) and toks[j].text not in (";", ",", "=", ")"):
                    j += 1
                names = [x.text for x in toks[k:j] if x.kind == "ident" and x.text not in QUALIFIERS]
                charge = names[-1:] + self._enclosing_type(toks, k)
                self.add(u.src, t.line, "R17", "match",
                         f"volatile without a `// volatile: <evidence>` note ({' '.join(charge) or '?'})",
                         charge=charge)

    def _enclosing_type(self, toks, k):
        depth = 0
        for i in range(k, -1, -1):
            if toks[i].text == "}":
                depth += 1
            elif toks[i].text == "{":
                if depth == 0:
                    if i > 0 and toks[i - 1].kind == "ident" and i > 1 and toks[i - 2].text in TAG_KW:
                        return [toks[i - 1].text]
                    j = match_close(toks, i)
                    td = self._typedef_after(toks, j)
                    return [td] if td else []
                depth -= 1
        return []

    def check_macros(self):
        """R4 offset macros, R18 asm macros, R17 do/while(0) is allowed in macros."""
        for u in self.units:
            for line, text in u.src.directives:
                m = re.match(r"\s*#\s*define\s+(\w+)", text)
                if not m:
                    continue
                name = m.group(1)
                inner = re.sub(r"^\s*#\s*define\s+\w+", "", text)
                fake = Source(u.src.path, text=inner.replace("\\\n", " \n"))
                for tk in fake.tokens:
                    tk.line += line - 1
                if self._offset_sites(fake.tokens):
                    self.add(u.src, line, "R4", "match", f"offset macro '{name}' (byte cast plus constant offset)",
                             charge=[name])
                if any(t.text in ASM_TOKENS for t in fake.tokens) and name not in self.allow["asm-helper"] \
                        and name not in self.allow["original-asm"] and name not in ("INCLUDE_ASM", "INCLUDE_RODATA"):
                    self.add(u.src, line, "R18", "match",
                             f"inline asm in macro '{name}' (list it as `asm-helper {name}` in config/rules.txt "
                             f"if the original had it)", charge=[name])
                if self.advisory and re.match(r"\s*#\s*if\s+0\b", text):
                    self.add(u.src, line, "R37", "advisory", "`#if 0` block")

    # -- per-function rules
    def _offset_sites(self, toks):
        """Token indices of byte-pointer casts followed by `+/- constant` under a pointer cast or deref."""
        sites = []
        for i, t in enumerate(toks):
            if t.text != "(":
                continue
            for bt in BYTE_TYPES:
                n = len(bt)
                seq = [x.text for x in toks[i + 1:i + 1 + n + 2]]
                quals = 0
                while i + 1 + quals < len(toks) and toks[i + 1 + quals].text in ("const", "volatile"):
                    quals += 1
                seq = [x.text for x in toks[i + 1 + quals:i + 1 + quals + n + 2]]
                if seq == list(bt) + ["*", ")"]:
                    k = i + 1 + quals + n + 2       # operand start
                    k = self._skip_operand(toks, k)
                    if k is None or k + 1 >= len(toks):
                        continue
                    if toks[k].text in ("+", "-") and toks[k + 1].kind == "number":
                        if self._under_deref(toks, i, k + 2):
                            sites.append(i)
                    break
        return sites

    @staticmethod
    def _skip_operand(toks, k):
        if k >= len(toks):
            return None
        if toks[k].text == "(":
            k = match_close(toks, k) + 1
        elif toks[k].kind == "ident":
            k += 1
        else:
            return None
        while k < len(toks):
            if toks[k].text in (".", "->") and k + 1 < len(toks):
                k += 2
            elif toks[k].text in ("(", "["):
                k = match_close(toks, k) + 1
            else:
                break
        return k

    @staticmethod
    def _under_deref(toks, cast_i, end):
        """The offset expression is wrapped by a pointer cast `(T *)(...)`, a `*`, `[`, or `->`."""
        i = cast_i - 1
        wrapped_close = None
        if i >= 0 and toks[i].text == "(":
            wrapped_close = match_close(toks, i)
            if wrapped_close != end:
                return False
            after = toks[wrapped_close + 1].text if wrapped_close + 1 < len(toks) else ""
            if after in ("[", "->"):
                return True
            i -= 1
        if i >= 0 and toks[i].text == ")":
            o = match_open(toks, i)
            inner = [x.text for x in toks[o + 1:i]]
            if inner and inner[-1] == "*":
                return True
        if i >= 0 and toks[i].text == "*" and (i == 0 or toks[i - 1].text in "(=,;{}+-*/&|!?:" or
                                               toks[i - 1].text in ("return",)):
            return True
        return False

    def check_function(self, fn):
        src, body = fn.src, fn.body
        # parameters
        depth, pstart = 0, None
        for k, x in enumerate(fn.head):
            if x.text == "(":
                if depth == 0 and k > 0 and fn.head[k - 1].text == fn.name:
                    pstart = k
                depth += 1
        if pstart is not None:
            pend = match_close(fn.head, pstart)
            for part in split_top(fn.head[pstart + 1:pend]):
                p = type_prefix_len(part, self.known_types)
                nt, _, _ = declarator_name(part[p:])
                if nt:
                    fn.params.append(nt.text)
        ids = [t for t in body if t.kind == "ident"]
        fn.refs = {t.text for t in fn.head if t.kind == "ident"} | {t.text for t in ids}
        self._check_locals(fn)
        self._check_blocks(fn)
        self._check_statements(fn)
        for t in body:
            if t.text in ASM_TOKENS and fn.name not in self.allow["asm-helper"] | self.allow["original-asm"]:
                self.add(src, t.line, "R18", "match", "inline asm in a function body", func=fn)
        for i in self._offset_sites(body):
            self.add(src, body[i].line, "R4", "match", "offset cast: byte-pointer cast plus a constant offset "
                     "(use a typed member)", func=fn)
        # R1 layout view: cast to a struct whose members are a strict prefix of another struct's
        for k, t in enumerate(body):
            if t.text == "(" and k + 2 < len(body) and body[k + 1].text in self.structs and body[k + 2].text == "*":
                view = body[k + 1].text
                vm = [(a, b) for a, b, _ in self.structs[view][2]]
                for other, (_, _, om) in self.structs.items():
                    om2 = [(a, b) for a, b, _ in om]
                    if other != view and vm and len(vm) < len(om2) and om2[:len(vm)] == vm:
                        self.add(src, t.line, "R1", "match",
                                 f"cast to layout view '{view}' (its members copy a prefix of '{other}')", func=fn)
                        break

    def _check_locals(self, fn):
        src, body = fn.src, fn.body
        # walk blocks; declarations come at the start of each block (C89)
        i = 0
        while i < len(body):
            if i == 0 or body[i - 1].text in ("{",):
                j = i
                while j < len(body):
                    end = j
                    depth = 0
                    while end < len(body):
                        if body[end].text in "([{":
                            depth += 1
                        elif body[end].text in ")]}":
                            depth -= 1
                            if depth < 0:
                                break
                        elif body[end].text == ";" and depth == 0:
                            break
                        end += 1
                    stmt = body[j:end]
                    p = type_prefix_len(stmt, self.known_types)
                    if p == 0 or end >= len(body) or body[end].text != ";":
                        break
                    spec = [x.text for x in stmt[:p]]
                    for part in split_top(stmt[p:]):
                        nt, is_func, _ = declarator_name(part)
                        if nt is None:
                            continue
                        if "extern" in spec or is_func:
                            if is_func:
                                if self.advisory:
                                    self.add(src, nt.line, "R25", "advisory",
                                             f"block-scope declaration of function '{nt.text}'", func=fn)
                            else:
                                self.add(src, nt.line, "R2", "match", f"block-scope extern '{nt.text}'", func=fn)
                            continue
                        if "volatile" in spec or any(x.text == "volatile" for x in part):
                            if not src.note(nt.line, r"volatile:\s*\S"):
                                self.add(src, nt.line, "R17", "match",
                                         f"volatile local '{nt.text}' without a `// volatile: <evidence>` note",
                                         func=fn)
                        fn.locals.append((nt.text, nt, "static" in spec))
                    j = end + 1
                i = j
            i += 1
        for name, nt, static in fn.locals:
            uses = [k for k, t in enumerate(body) if t.text == name and t is not nt
                    and not (k > 0 and body[k - 1].text in (".", "->"))]
            if src.note(nt.line, r"R36"):
                continue
            if not uses:
                self.add(src, nt.line, "R17", "match", f"unused local '{name}'", func=fn)
                continue
            reads = [k for k in uses if not self._pure_write(body, k)]
            if not reads and not static:
                self.add(src, nt.line, "R17", "match", f"local '{name}' is assigned but never read", func=fn)
        # volatile casts and other volatile tokens in the body (declarations handled above)
        decl_lines = {nt.line for _, nt, _ in fn.locals}
        for t in body:
            if t.text == "volatile" and t.line not in decl_lines and not src.note(t.line, r"volatile:\s*\S") \
                    and not self._after_asm(body, t):
                self.add(src, t.line, "R17", "match", "volatile without a `// volatile: <evidence>` note", func=fn)

    @staticmethod
    def _after_asm(body, t):
        k = body.index(t)
        return k > 0 and body[k - 1].text in ASM_TOKENS

    @staticmethod
    def _pure_write(body, k):
        nxt = body[k + 1].text if k + 1 < len(body) else ""
        prv = body[k - 1].text if k > 0 else ""
        return nxt == "=" and prv in (";", "{", "}", ",", "(", ")", "=", ":")

    def _brace_kinds(self, body):
        """Index of each `{` -> 'block' or 'aggregate'."""
        kinds = {}
        for k, t in enumerate(body):
            if t.text != "{":
                continue
            p = body[k - 1] if k > 0 else None
            pp = body[k - 2] if k > 1 else None
            if p is not None and (p.text in ("=", ",") and kinds.get(self._enclosing_open(body, k)) != "block"
                                  or p.text == "=" or p.text in TAG_KW
                                  or (pp is not None and pp.text in TAG_KW)):
                kinds[k] = "aggregate"
            elif p is not None and p.text in ("{", ",") and kinds.get(self._enclosing_open(body, k)) == "aggregate":
                kinds[k] = "aggregate"
            else:
                kinds[k] = "block"
        return kinds

    @staticmethod
    def _enclosing_open(body, k):
        depth = 0
        for i in range(k - 1, -1, -1):
            if body[i].text == "}":
                depth += 1
            elif body[i].text == "{":
                if depth == 0:
                    return i
                depth -= 1
        return None

    def _check_blocks(self, fn):
        src, body = fn.src, fn.body
        kinds = self._brace_kinds(body)
        for k, t in enumerate(body):
            # empty blocks
            if t.text == "{" and kinds.get(k) == "block" and k + 1 < len(body) and body[k + 1].text == "}":
                owner = self._block_owner(body, k)
                if owner in ("if", "else", "plain", "switch"):
                    self.add(src, t.line, "R17", "match", f"empty {owner} block", func=fn)
                elif owner == "do":
                    pass    # reported as do/while below or allowed as an empty loop
            # bare ;
            if t.text == ";" and k > 0:
                p = body[k - 1]
                if p.text in (";", "{") and not self._in_for_header(body, k):
                    self.add(src, t.line, "R17", "match", "bare `;` statement", func=fn)
                elif p.text == "}" and kinds.get(match_open(body, k - 1)) == "block" \
                        and not self._is_do_close(body, k - 1):
                    self.add(src, t.line, "R17", "match", "bare `;` statement", func=fn)
                elif p.text == "else":
                    self.add(src, t.line, "R17", "match", "bare `;` as else body", func=fn)
                elif p.text == ")":
                    o = match_open(body, k - 1)
                    kw = body[o - 1].text if o > 0 else ""
                    if kw == "if":
                        self.add(src, t.line, "R17", "match", "bare `;` as if body", func=fn)
            # do { } while (0)
            if t.text == "do" and k + 1 < len(body) and body[k + 1].text == "{":
                c = match_close(body, k + 1)
                if c + 4 < len(body) and body[c + 1].text == "while" and body[c + 2].text == "(" \
                        and body[c + 3].kind == "number" and int(re.sub(r"[uUlL]+$", "", body[c + 3].text), 0) == 0 \
                        and body[c + 4].text == ")":
                    self.add(src, t.line, "R17", "match", "`do { } while (0)` in a function body (use the macro)",
                             func=fn)

    @staticmethod
    def _in_for_header(body, k):
        depth = 0
        for i in range(k, -1, -1):
            if body[i].text == ")":
                depth += 1
            elif body[i].text == "(":
                if depth == 0:
                    return i > 0 and body[i - 1].text == "for"
                depth -= 1
            elif body[i].text in ("{", "}") and depth == 0:
                return False
        return False

    @staticmethod
    def _is_do_close(body, close):
        o = match_open(body, close)
        return o > 0 and body[o - 1].text == "do"

    @staticmethod
    def _block_owner(body, k):
        if k == 0:
            return "plain"
        p = body[k - 1]
        if p.text == "else":
            return "else"
        if p.text == "do":
            return "do"
        if p.text == ")":
            o = match_open(body, k - 1)
            kw = body[o - 1].text if o > 0 else ""
            if kw in ("if", "switch"):
                return kw
            if kw in ("while", "for"):
                return "loop"
            return "other"
        if p.text in (";", "{", "}", ":"):
            return "plain"
        return "other"

    def _check_statements(self, fn):
        src, body = fn.src, fn.body
        # self-assignment: `<lhs> = <lhs> ;` with identical tokens
        for k, t in enumerate(body):
            if t.text != "=":
                continue
            s = k - 1
            while s >= 0 and body[s].text not in (";", "{", "}", ",") and not (
                    body[s].text == "(" and s > 0 and body[s - 1].text in ("for", "if", "while")):
                if body[s].text == ")":
                    s = match_open(body, s)
                s -= 1
            e = k + 1
            while e < len(body) and body[e].text not in (";", ",", ")", "}"):
                if body[e].text in ("(", "["):
                    e = match_close(body, e)
                e += 1
            lhs, rhs = [x.text for x in body[s + 1:k]], [x.text for x in body[k + 1:e]]
            if lhs and lhs == rhs:
                e = re.sub(r" ?(->|\.|\[|\]) ?", r"\1", " ".join(lhs))
                self.add(src, t.line, "R17", "match", f"self-assignment `{e} = {e}`", func=fn)
        # identity arithmetic
        def num(x):
            try:
                return float(int(re.sub(r"[uUlL]+$", "", x.text), 0)) if not re.search(r"[.eE]", x.text) or \
                    x.text.lower().startswith("0x") else float(re.sub(r"[fFlL]+$", "", x.text))
            except ValueError:
                return None
        binary_left = lambda p: p.kind in ("ident", "number") or p.text in (")", "]")
        for k, t in enumerate(body):
            if t.text in ("+", "-", "*", "/") and 0 < k < len(body) - 1 and body[k + 1].kind == "number" \
                    and binary_left(body[k - 1]):
                v = num(body[k + 1])
                nxt = body[k + 2].text if k + 2 < len(body) else ""
                if t.text in "+-" and v == 0 and nxt not in ("*", "/", "%", "[", "("):
                    self.add(src, t.line, "R17", "match", f"identity arithmetic `{t.text} {body[k + 1].text}`", func=fn)
                elif t.text in "*/" and v == 1 and nxt not in ("[", "("):
                    self.add(src, t.line, "R17", "match", f"identity arithmetic `{t.text} {body[k + 1].text}`", func=fn)
            if t.kind == "number" and k + 1 < len(body) and body[k + 1].text in ("+", "*") and \
                    (k == 0 or body[k - 1].text in ("=", "(", ",", "return", "[")):
                v = num(t)
                if (body[k + 1].text == "+" and v == 0) or (body[k + 1].text == "*" and v == 1):
                    self.add(src, t.line, "R17", "match", f"identity arithmetic `{t.text} {body[k + 1].text}`", func=fn)

    # -- advisory
    def check_advisory(self, fn, globals_types):
        src, body = fn.src, fn.body
        libultra = "libultra" in src.rel or src.rel.startswith("lib/")
        # R7: names in the function, params, locals, referenced globals/types/members
        names = []
        for t in list(fn.head) + list(body):
            if t.kind == "ident" and placeholder(t.text) and t.text not in names:
                names.append(t.text)
        for ty in [n for n in fn.refs if n in self.structs]:
            for _, m, _ in self.structs[ty][2]:
                if placeholder(m) and m in fn.refs and m not in names:
                    names.append(m)
        if names:
            self.add(src, fn.line, "R7", "advisory", "open naming: " + ", ".join(names), func=fn)
        if libultra:
            return
        # R33 header comment
        head_line = fn.head[0].line if fn.head else fn.line
        above = src.lines[head_line - 2].strip() if head_line >= 2 else ""
        if not (src.comments.get(head_line - 1) or above.endswith("*/")):
            self.add(src, fn.line, "R33", "advisory", "no header comment", func=fn)
        elif "=====" not in "\n".join(src.lines[max(0, head_line - 12):head_line - 1]):
            self.add(src, fn.line, "R8", "advisory", "header comment is not the profile's `/* ===== Name ===== */` "
                     "banner", func=fn)
        # R8 style: function name, locals, params
        bad = []
        if not placeholder(fn.name) and not FUNC_STYLE_RE.match(fn.name):
            bad.append(f"function '{fn.name}' (Prefix_VerbNoun)")
        for n in fn.params + [x[0] for x in fn.locals]:
            if not placeholder(n) and not LOWER_CAMEL_RE.match(n):
                bad.append(f"'{n}' (lowerCamel)")
        if bad:
            self.add(src, fn.line, "R8", "advisory", "naming style: " + ", ".join(bad), func=fn)
        # R8 layout: spaces inside parentheses (Table 1: `if ( cond ) {`), K&R braces
        tight = [t for k, t in enumerate(body[:-1]) if t.text == "(" and not t.space_after
                 and body[k + 1].text != ")" and k > 0 and
                 (body[k - 1].text in NO_SPACE_AFTER or body[k - 1].kind == "ident")]
        if tight:
            self.add(src, tight[0].line, "R8", "advisory",
                     f"layout: no spaces inside parentheses at {len(tight)} site(s) (Table 1 writes `if ( x )`)",
                     func=fn)
        allman = [t for k, t in enumerate(body) if t.text == "{" and k > 0 and body[k - 1].line < t.line
                  and body[k - 1].text in (")", "else", "do")]
        if fn.head and fn.head[-1].line < fn.src.tokens[fn.open_i].line:
            allman.insert(0, fn.src.tokens[fn.open_i])
        if allman:
            self.add(src, allman[0].line, "R8", "advisory", f"layout: brace on its own line at {len(allman)} "
                     "site(s) (Table 1 is K&R)", func=fn)
        # R35 braceless bodies
        for k, t in enumerate(body):
            if t.text in ("if", "while", "for") and k + 1 < len(body) and body[k + 1].text == "(":
                c = match_close(body, k + 1)
                if c + 1 < len(body) and body[c + 1].text not in ("{", ";"):
                    if t.text == "while" and self._closes_do(body, k):
                        continue
                    self.add(src, t.line, "R35", "advisory", f"braceless {t.text} body", func=fn)
            if t.text == "else" and k + 1 < len(body) and body[k + 1].text not in ("{", "if"):
                self.add(src, t.line, "R35", "advisory", "braceless else body", func=fn)

    @staticmethod
    def _closes_do(body, k):
        return k > 0 and body[k - 1].text == "}" and Checker._is_do_close(body, k - 1)

    def check_r13(self, unit):
        """R13 (real vtables) needs the binary's vtable data and applies to C++ only; C units have no
        virtuals, so this is a hook. In a C++ unit it flags virtuals named pad/dummy."""
        if unit.src.path.suffix not in (".cpp", ".cc", ".cxx"):
            return
        toks = unit.src.tokens
        for k, t in enumerate(toks):
            if t.text == "virtual":
                j = k
                while j < len(toks) and toks[j].text != "(":
                    j += 1
                if j > k and PAD_RE.match(toks[j - 1].text):
                    self.add(unit.src, t.line, "R13", "match", f"padding virtual '{toks[j - 1].text}'",
                             charge=[toks[j - 1].text])

    # -- driver
    def run(self):
        self.check_tags_and_typedefs()
        self.check_globals()
        self.check_volatile_globals()
        self.check_macros()
        for u in self.units:
            self.check_r13(u)
            for fn in u.functions:
                self.check_function(fn)
                if self.advisory:
                    self.check_advisory(fn, None)
            for line in u.toplevel_asm:
                self.add(u.src, line, "R18", "match", "file-scope inline asm outside INCLUDE_ASM")
        # charge type/global/macro findings to the functions that name them
        for u in self.units:
            for fn in u.functions:
                for name in sorted(fn.refs & set(self.global_findings)):
                    for f in self.global_findings[name]:
                        if f not in fn.findings:
                            fn.findings.append(f)

    def results(self, targets_only):
        rows = []
        for u in self.units:
            if targets_only and u.src.path.resolve() not in self.target_paths:
                continue
            if self.is_header(u.src) and not u.functions:
                continue
            unit = str(Path(u.src.rel).with_suffix(""))
            for fn in u.functions:
                kind = "original-asm" if fn.name in self.allow["original-asm"] else "c"
                rows.append({"unit": unit, "file": u.src.rel, "function": fn.name, "line": fn.line, "kind": kind,
                             "asm_helper": fn.name in self.allow["asm-helper"],
                             "match_findings": [f.as_dict() for f in fn.findings if f.tier == "match"],
                             "advisory_findings": [f.as_dict() for f in fn.findings if f.tier == "advisory"]})
            for name, line in u.stubs:
                kind = "original-asm" if name in self.allow["original-asm"] else "parked"
                rows.append({"unit": unit, "file": u.src.rel, "function": name, "line": line, "kind": kind,
                             "asm_helper": False, "match_findings": [], "advisory_findings": []})
        return rows


def load_exact(report_path):
    exact = set()
    r = json.load(open(report_path))
    for u in r.get("units", []):
        for f in u.get("functions", []):
            if f.get("fuzzy_match_percent") == 100.0:
                exact.add((u["name"], f["name"]))
    return exact


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("paths", nargs="*", help="C units to check (default: src/**/*.c)")
    ap.add_argument("--json", metavar="OUT", help="write machine-readable results ('-' for stdout)")
    ap.add_argument("--report", metavar="REPORT", help="objdiff report.json: MATCH-PENDING = exact and failing")
    ap.add_argument("--no-advisory", action="store_true", help="skip advisory findings")
    args = ap.parse_args(argv)

    targets = [Path(p) for p in args.paths]
    for t in targets:
        if t.is_dir():
            targets.remove(t)
            targets += sorted(t.rglob("*.c"))
    targets = [t if t.suffix else t.with_suffix(".c") for t in targets]
    for t in targets:
        if not t.exists():
            print(f"rules: no such unit: {t}", file=sys.stderr)
            return 2
    default = not targets
    if default:
        targets = sorted((ROOT / "src").rglob("*.c"))
    chk = Checker(targets, advisory=not args.no_advisory)
    chk.run()
    rows = chk.results(targets_only=True)
    tfiles = {rel(t) for t in targets}
    # findings in headers charged to checked functions are shown too
    charged = {id(f) for u in chk.units for fn in u.functions
               if u.src.path.resolve() in chk.target_paths for f in fn.findings}
    shown = [f for f in chk.findings if f.file in tfiles or id(f) in charged]
    shown.sort(key=lambda f: (f.file, f.line, f.rule))
    exact = load_exact(args.report) if args.report else None

    out = sys.stdout if args.json != "-" else sys.stderr
    for f in shown:
        print(f.text(), file=out)
    failing = [r for r in rows if r["kind"] == "c" and r["match_findings"]]
    if exact is not None:
        pending = [r for r in failing if (r["unit"], r["function"]) in exact]
        not_exact = [r for r in failing if (r["unit"], r["function"]) not in exact]
    else:
        pending, not_exact = failing, []
    c_rows = [r for r in rows if r["kind"] == "c"]
    counts = {
        "c_functions": len(c_rows),
        "c_functions_passing": len(c_rows) - len(failing),
        "parked_include_asm": sum(r["kind"] == "parked" for r in rows),
        "original_asm": sum(r["kind"] == "original-asm" for r in rows),
        "match_findings": sum(f.tier == "match" for f in shown),
        "advisory_findings": sum(f.tier == "advisory" for f in shown),
        "match_pending": len(pending),
    }
    print("", file=out)
    label = "MATCH-PENDING" if exact is not None else "MATCH-PENDING (if exact; pass --report to check)"
    print(f"{label}: {len(pending)}", file=out)
    for r in pending:
        rules = sorted({f["rule"] for f in r["match_findings"]})
        print(f"  {r['unit']} {r['function']}  {' '.join(rules)}", file=out)
    if not_exact:
        print(f"failing [match] rules, not exact yet: {len(not_exact)}", file=out)
        for r in not_exact:
            print(f"  {r['unit']} {r['function']}  {' '.join(sorted({f['rule'] for f in r['match_findings']}))}",
                  file=out)
    for r in rows:
        if r["kind"] == "original-asm":
            print(f"ORIGINAL-ASM: {r['unit']} {r['function']}", file=out)
    print(f"rules: {counts['c_functions']} C functions ({counts['c_functions_passing']} pass every [match] rule), "
          f"{counts['parked_include_asm']} parked INCLUDE_ASM, {counts['match_findings']} [match] findings, "
          f"{counts['advisory_findings']} advisory findings", file=out)
    if args.json:
        data = {"version": 1, "counts": counts, "functions": rows,
                "findings": [f.as_dict() for f in shown],
                "match_pending": [{"unit": r["unit"], "function": r["function"]} for r in pending],
                "failing": [{"unit": r["unit"], "function": r["function"]} for r in failing]}
        text = json.dumps(data, indent=1) + "\n"
        if args.json == "-":
            sys.stdout.write(text)
        else:
            Path(args.json).write_text(text)
    return 1 if counts["match_findings"] else 0


if __name__ == "__main__":
    sys.exit(main())

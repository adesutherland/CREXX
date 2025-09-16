# RxLite — Minimal Regex Toolkit for REXX
_Last updated: 2025-09-05_

A compact, pure-REXX regular expression toolkit with practical helpers.

**Supports:** literals, `.`, anchors `^`/`$`, character classes `[abc]`, ranges `[a-z]`, negation `[^…]`, escapes `\d \D \w \W \s \S`, quantifiers `* + ?` (plus lazy `*? +?`), **top-level alternation `|`**, and case-insensitive flag `(?i)`.

> Positions are **1-based**. Matching is **leftmost-first** with simple backtracking.
> Internally the engine tracks `rxlite_start` and an end-cursor `rxlite_end` (cursor **after** the match). Effective length is `rxlite_len = rxlite_end - rxlite_start`. Callers should not touch these; use helpers instead.

---

## Public API (external module)
Exported functions for callers:

### Match / Scan
- `REGEXMATCH(s, p) -> 1/0`  
  True if `p` matches **anywhere** in `s`. Respects `(?i)`.
- `REGEXFIND(s, p, fromPos) -> 1/0`  
  Finds the **next** match at or after `fromPos` (use in a loop).

### Transform
- `REGEXREPLACE(s, p, repl) -> string`  
  Replace **all** matches.
- `REGEXREPLACE_LIMIT(s, p, repl, limit=0) -> string`  
  Replace at most `limit` matches (`0` = all).

**Replacement expansion (new):**
- In `repl`, the following are recognized:
    - `$0` or `$&` → the **entire match**
    - `$$` → a literal dollar sign
    - (No `$1..$9` yet; groups are not implemented.)

### Collect
- `REGEXSPLIT(s, p, parts.[, opts, limit]) -> count`  
  Split on matches of `p`; returns the **gaps** between matches. Options:
    - `T` trim pieces, `D` drop empties, `E` expand multi-char delimiters
    - `limit` maximum number of pieces (last piece absorbs remainder)
- `REGEXFINDALL(s, p, outText.[, opts, limit]) -> count`  
  Gather all matched substrings into `outText.` (text-only for simplicity). Options:
    - `O` allow **overlapping** matches
    - `limit` maximum matches

### Helpers (new)
- `RXDETAILS outStem.`  
  Finalizes internally and fills: `outStem.1 = start`, `outStem.2 = len`, `outStem.3 = end`. Returns 1 if a prior match exists, else 0.
- `RXSUBSTR(s) -> string`  
  Returns the last match’s substring from `s` (auto-finalizes length).
- `RXNEXTPOS() -> int`  
  Returns the correct next scan position: `start + max(1, len)` (safe for zero-length matches).

> Keep `regexfinalizeLen` and all `__internal` routines private; only public calls and these helpers should be used by callers.

---

## Regex Syntax Supported
- **Anchors**: `^` (start), `$` (end)
- **Dot**: `.` any single character
- **Character classes**: `[abc]`, `[a-z]`, `[A-Za-z0-9_]`, negation `[^…]`  
  Escapes inside classes: `\n \r \t`
- **Escapes**: `\d` digits `[0-9]`, `\D` non-digits, `\w` word (`A..Z a..z 0..9 _`), `\W` non-word, `\s` whitespace (space, tab, LF, CR, FF), `\S` non-whitespace
- **Quantifiers**: `*` (0+), `+` (1+), `?` (0/1); **lazy** `*?`, `+?`
- **Alternation**: **top-level** `|` only (e.g., `foo|bar`). Literal pipe: `\|`. Inside `[...]` pipe is literal.
- **Case-insensitive**: prefix the pattern with `(?i)` (e.g., `(?i).*\.txt$`).

> **Not yet:** grouping `(...)`, backreferences `\1`, lookaround, nested alternation via groups, Unicode categories.

---

## Alternation Semantics (top-level)
The pattern is split at `|` **outside** of character classes and not escaped with `\`. Each alternative is tried independently with the normal matcher. **Anchors apply per alternative.**  
Example: `^foo|bar$` means “`foo` at start **OR** `bar` at end” (not `^(foo|bar)$`).

---

## Replacement Expansion (new)
Inside `repl`:
- `$0` / `$&` → expands to the **entire match**
- `$$` → literal `$`

Examples:
```rexx
say REGEXREPLACE("abc123", "\d+", "<$0>")      /* abc<123> */
say REGEXREPLACE("X1X2",   "X.*?X", "[$&]")     /* [X1X]2   */
say REGEXREPLACE("a1b",    "\d",   "$$")       /* a$b      */
```

---

## Usage Examples
```rexx
/* Existence */
ok = REGEXMATCH("abc123", "^\w+\d+$")         /* 1 */

/* Scanning */
s = "ID=7; next=42; end."
pos = 1
do while REGEXFIND(s, "\d+", pos)
  call RXDETAILS rxlast.
  say 'num:' RXSUBSTR(s) 'at' rxlast.1 'len' rxlast.2
  pos = RXNEXTPOS()
end

/* Split */
n = REGEXSPLIT("a,,c,", ",+", parts.)           /* parts.1="a" parts.2="c" parts.3="" */
n = REGEXSPLIT("a,,,b", ",+", parts., "E")      /* parts.1="a" parts.2="" parts.3="" parts.4="b" */

/* Replace (greedy vs lazy) */
say REGEXREPLACE("X1X2X3X", "X.*X",  "#")       /* #   */
say REGEXREPLACE("X1X2X3X", "X.*?X", "#")       /* #2# */

/* Case-insensitive */
ok = REGEXMATCH("File.Txt", "(?i).+\.[a-z]+$") /* 1 */
```

---

## Design Notes & Integration
- **Indices** are 1-based.
- **Zero-length matches** (e.g., `^`, `$`) are valid; when scanning, always advance by at least 1: `start + max(1, len)`.
- `regexfinalizeLen` computes `len = end - start` when `end` is set; it remains **internal** and is called by public helpers.
- For external-module use, prefer **helpers** (`RXDETAILS`, `RXSUBSTR`, `RXNEXTPOS`) over reading globals.

---

## Regex vs REXX PARSE — different goals & mechanisms
**Different tools for different jobs.** A quick contrast without trying to turn one into the other:

- **Goal**
    - **Regex (RxLite):** describe the *shape* of text to find/verify/transform (tokens, dates, suffixes, etc.).
    - **REXX PARSE:** carve a string into *fields/variables* using positions, words, or literal delimiters.
- **Mechanism**
    - **Regex:** backtracking matcher with quantifiers, classes, anchors, top-level alternation, case-insensitive mode.
    - **PARSE:** deterministic left-to-right slicing via a template; no backtracking or quantifiers.
- **Outputs**
    - **Regex:** returns matches/gaps/replacements (`REGEXFIND`, `REGEXFINDALL`, `REGEXSPLIT`, `REGEXREPLACE`). You decide where to store them.
    - **PARSE:** assigns directly to variables in one statement (`parse var s a b c`).

---

## Smoke Test (quick run)

```rexx
/* Minimal sanity checks */
call TEST_EQ 'repl $0', REGEXREPLACE("abc123", "\d+", "<$0>"), "abc<123>"
call TEST_EQ 'lazy',     REGEXREPLACE("X1X2X3X", "X.*?X", "#"), "#2#"
call TEST_TRUE '(?i) txt', REGEXMATCH("File.Txt", "(?i).*\.[a-z]+$")
n = REGEXSPLIT("a,,c,", ",+", p.); call TEST_EQ 'split n', n, 3
m = REGEXFINDALL("ID=7; next=42","\d+", t.); call TEST_EQ 'findall', m, 2
```
_(Define `TEST_EQ/TEST_TRUE` as in your test harness.)_

---

## Limitations / Not Yet Implemented
- Grouping `(...)` and backreferences `\1` (pattern-time)
- Alternation inside groups (`^(foo|bar)$` true precedence)
- Lookaround (`(?=...)`, `(?<=...)`, etc.)
- Unicode character classes / locale-aware categories

> Current API is stable; features can be added incrementally.
# RXPP Program Documentation (internal)

## 1. Purpose and scope

RXPP is a **source-to-source pre-compiler** for CREXX *levelb* Rexx sources. It reads an input Rexx file, processes preprocessor directives and macro definitions, performs controlled compile-time transformations, and writes a transformed Rexx output file plus a linker import include.

RXPP is **not** a user-facing documentation generator. It is a compilation pipeline implemented as multiple passes over a shared mutable source buffer.

Major responsibilities include:

- Macro definition and expansion (fixed and variadic)
- Conditional compilation (##IF / ##IFN / ##ELSE / ##END)
- Structural rewrites (FOR, SELECT/CASE, OO syntax)
- Stem access normalization (getstem / putstem rewriting)
- Controlled injection of generated code

---

## 2. High-level pipeline

### Pass 0: Startup / CLI

The main program parses `command[]` arguments to determine:

- input file
- output file
- macro library path
- verbosity and flags

It initializes the environment via:

`rxppinit(infile, maclib)`

which:

- prepares all global arrays (`source[]`, `stype[]`, macro tables, etc.)
- initializes defaults (`cflags`, `printgen_flags`, module name, paths)
- resets counters and working state

---

### Pass 1: Load macros, load source, classify statements

`RXPPPassOne(infile, outfile, maclib, macsys)` performs:

1. **Macro library loading**
    - Reads user macro library (`maclib`)
    - Reads system macro library (`macsys`)
    - Extracts macro definitions via `GetPreComp(...)`
    - Drops source buffer afterwards (macro registration only)

2. **Source loading**
    - Reads the actual input Rexx file into `source[]`

3. **Early structural setup**
    - Neutralizes existing `options levelb`, `import rxfnsb`, etc.
    - Injects RXPP header lines into the source buffer

4. **In-file macro extraction**
    - Runs `GetPreComp(rexxlines)` again on the real source
    - Registers `##DEFINE` macros
    - Sorts macro names by length to prevent partial matches

---

### Pass 2: Conditional normalization + IF/ENDIF linking + OO create defs

`RXPPPassTwo()` performs:

- Normalization of `##ELSE` into:
    - `##endif`
    - `##ifn <condition>`

- Construction of `aifblock[]`, mapping each `##IF / ##IFN` to its matching `##END`

- OO prefix preparation:
    - Rewrites `lhs = OOCREATE(prefix, ...)` into `lhs = <prefix>CREATE(...)`
    - Stores `lhs_prefix = <prefix>` for later OO call translation

---

### Pass 3: Emit final output buffer

`RXPPPassThree(outfile)` walks `source[]` linearly and:

- Skips suppressed lines (`stype='X'`)
- Emits preprocessor lines as comments based on `cflags` and `printgen_flags`
- Expands macros via `expandRecursive()`
- Applies OO translation via `ooTranslate()`
- Writes final output using `writeall(outbuf, outfile)`
- Writes linker include via `linkerInfo(outfile, imported_funcs)`

---

## 3. Core data structures (global)

### source[]

Mutable source buffer. RXPP inserts new lines, shifts existing lines, and overwrites entries during transformation.

### stype[]

Parallel to `source[]`. Tags each line with a processing status, e.g.:

- `D` define
- `I` include
- `IF` / `IFN`
- `X` suppressed
- `PARSE`, `IMPORT`, `ARRAY`, `GLOBAL`, `STEM`
- `noexp` reserved inserted line (skip further inspection)
- `cmt` inside block comment

### Macro tables

- `macros_mname[]`, `macros_margs[]`, `macros_mbody[]`, `macros_mspace[]`
- `macros_varname[]`, `macros_varvalue[]` for `{var}` injection

### Conditional block table

- `aifblock[]`: maps `##IF / ##IFN` line numbers to matching `##END`

### Stem expansion state

- `stem_scount`: unique temporary counter
- `stem_tempvar[]`: names of generated temporaries (`_tmp_rxpp_n`, `_expr_n`)
- `rxpp_lrc`: last stem diagnostic message

---

## 4. Major subsystems

### Line parsing model (token-oriented, non-AST)

RXPP does **not** construct or operate on a formal syntax tree. Instead, it uses a **token-oriented, context-aware line parsing model**.

Parsing is performed *line by line* and consists of:

- Scanning a source line using quote-aware and parenthesis-aware helpers
- Breaking the line into **tokens and substrings** (words, arguments, quoted strings, parenthesized expressions)
- Applying rule-specific transformations directly to those tokens

This parsing approach is **pragmatic rather than grammatical**: RXPP only recognizes as much structure as is required to reliably identify precompiler constructs (macros, directives, stems, OO syntax, control shorthands).

The model deliberately avoids:

- Building an abstract syntax tree (AST)
- Full Rexx grammar recognition
- Semantic analysis beyond compile-time constructs

This keeps RXPP aligned with Rexx’s flexible, text-oriented nature, reduces complexity, and allows transformations to be expressed as deterministic rewrites over a mutable source buffer.

---

### A) Preprocessor directives

Detected in `GetPreComp()` and dispatched via `CMD_*` handlers:

- `##DEFINE` → `CMD_define`
- `##INCLUDE / ##USE` → `CMD_include`
- `##CFLAG` → `early_flag_pick_up`
- `##ARRAY` → `CMD_array`
- `##GLOBAL` → `CMD_global`
- `##STEM` → `CMD_stem`
- `##DATA / ##INPUT / ##SYS*` → `CMD_data`

Also rewrites convenience constructs:

- `FOR`, `SELECT`, `SWITCH`, `WHEN`, `CASE`

---

### B) Macro expansion

`expandRecursive()` repeatedly applies `expandLine()` until stable.

Macro expansion includes:

- Argument parsing (`fetchArguments`, `parseArgList`)
- Fixed argument substitution (`replaceFixArg`)
- Variadic expansion (`variadic`)
- Identifier-safe replacement (`replaceArg`)
- Low-level string splicing (`insertatc`)

---

### C) OO call translation

`ooTranslate()` rewrites:

`object.method(args)` → `<prefix>METHOD(object, args)`

where `<prefix>` is retrieved from `getvar(object'_prefix')`.

Special handling exists for stem-like objects and GET/PUT calls.

---

### D) Stem rewrite engine

Triggered by:

- Explicit `##STEM`
- Automatic detection via `CMD_isstem`

Pipeline:

- Identify LHS / RHS
- Extract RHS stems into temporary `getstem(...)` calls
- Rewrite RHS to `_tmp_rxpp_n`
- Rewrite LHS stems into `putstem(...)`
- Pre-evaluate computed tails `. (expr)` via `_expr_n`

This subsystem inserts new source lines dynamically and advances `lino` accordingly.

---

## 5. Procedure catalog (summary)

### Entry / passes

- `RXPPPassOne` – load macros and source; extract in-file macros
- `RXPPPassTwo` – normalize conditionals; build IF/ENDIF links; prep OO
- `RXPPPassThree` – expand and emit final output
- `rxppinit` – initialize all global state

### OO support

- `oocreatedefs` – preprocess OOCREATE constructs
- `oo_translate_tilde` – legacy `~` syntax support
- `ooTranslate` – main OO call translator

### Conditional support

- `early_flag_pick_up` – early `##CFLAG` processing
- `findMatchingEndif` – nesting-aware IF/ENDIF matcher

### Source manipulation

- `ReadSource`, `insert_source`, `insert_line`, `inject2Source`
- `writeline`, `linkerInfo`

### Directive handlers

- `CMD_define`, `CMD_include`, `CMD_data`, `CMD_array`, `CMD_global`
- `CMD_set`, `CMD_unset`
- `CMD_select`, `CMD_when`, `CMD_case`, `CMD_for`

### Macro engine

- `expandRecursive`, `expandLine`, `resolveMacro`
- `fetchArguments`, `parseArgList`, `replaceFixArg`, `replaceArg`
- `variadic`, `tso2func`, `injectVariable`

### PARSE rewrite

- `parsevar`, `flush`, `embed`, `preCleanTemplate`, `FirstWord`

### Comment handling

- `CommentDepth`, `CommentScan`

### Stem engine

- `CMD_stem`
- `lhsStem`, `lhsnoStem`, `lhsisStem`
- `splitRHS`, `ResolveStem`, `rewriteStemsInExpr`
- `findStem`, `stemEndBySplit`, `replaceOnce`
- `isStem`, `isStemCandidate`, `isStemNameChar`, `isPlainStemName`
- `qextractall_dotparen_flat`, `matchParen`

---

## 6. Precomp interface expectations

RXPP relies on the native **`precomp`** module (C layer) for performance-critical and quote-/nesting-aware primitives. The C layer is intentionally *mechanical*: it provides deterministic operations (array shifting, scanning, tokenization, file I/O) while **RXPP keeps all semantic decisions in REXX**.

### 6.1 Registered procedures (contract)

#### File I/O

- **`readall(expose array, expose file, arg2)` → `.int`**  
  Reads a text file into a Rexx stem array (line-oriented). RXPP assumes 1-based line indexing and that `array.0` reflects the populated length.

- **`writeall(expose array, file, arg2)` → `.int`**  
  Writes a Rexx stem array back to disk. RXPP uses this for the final output file and the linker include.

#### Array operations

- **`insert_array(expose a, from, new)` → `.int`**  
  Inserts `new` empty slots into stem `a` starting at index `from`, shifting existing elements upward. RXPP builds `insert_source()` on top of this, so *index stability* and correct `a.0` maintenance are critical.

- **`drop_array(expose a)` → `.int`**  
  Clears the stem array efficiently. Used to reset `source[]`, `stype[]`, and other work arrays between phases.

- **`copy_array(expose a, b, from, tto)` → `.int`**  
  Copies a slice between stems. Used for bulk movement / cloning of buffers (less common than `insert_array`, but useful for refactors and utilities).

- **`list_array(expose a, from, tto, hdr)` → `.void`**  
  Debug dump used under `cflags` to inspect `source[]` and other stems.

- **`search_array(expose a, needle, startrow, match)` → `.int`**  
  Searches a stem for `needle`. RXPP uses this for include deduplication and simple lookups. The `match` parameter controls comparison semantics.

#### Searching and scanning

- **`fsearch(expose array, pos, str1, str2, str3, expose item)` → `.int`**  
  Multi-pattern forward scan across `array[]`. Returns the line number of the next match after `pos` and sets `item` to indicate which pattern matched. Pass 2 depends on this for conditional normalization (##IF/##IFN/##ELSE/##END), so correctness under mutation is important.

- **`ffind(expose array, pos, str1)` → `.int`**  
  Single-pattern forward scan (lighter-weight than `fsearch`). Used for constructs like `OOCREATE(` scanning.

- **`fpos(string, substring, offset)` → `.int`**  
  Substring position search with an offset. RXPP treats this as a reliable helper for “find token at/after position” style logic.

#### Parsing helpers

- **`splitargs(string, expose tokens)` → `.int`**  
  Splits a comma-separated argument list into tokens while respecting parentheses nesting. RXPP’s macro expansion correctness depends on this *not splitting on commas inside nested calls*.

- **`find_quoted(string, expose tokens, expose types)` → `.int`**  
  Quote-aware tokenization. Used where RXPP must not treat delimiters inside strings as syntax.

#### String helpers

- **`insertatc(haystack, needle, offset, len)` → `.string`**  
  Low-level splice primitive: replace `len` characters at `offset` inside `haystack` with `needle`. RXPP uses this heavily during macro expansion (`resolveMacro`) and expects consistent indexing and stable behavior for edge offsets.

- **`safe_quote(string)` → `.string`**  
  Produces a Rexx-safe literal string for emission into generated code (used e.g. by `CMD_data`).

- **`stemquote(path)` → `.string`**  
  Converts a stem path into RXPP’s canonical quoted representation (e.g., representing dotted tails in a form suitable for `getstem()` / `putstem()` calls). The stem subsystem assumes this quoting is stable.

#### Macro helpers and misc

- **`hasmacro(line, maclist, from)` → `.int`**  
  Detects macro occurrences against a macro-name table. Used as the macro expansion entry-point scanner.

- **`sort_bylen(expose a, expose b, expose c, expose d)` / `shell_sort(expose a, offset, order)` → `.void`**  
  Sorting helpers used to order macro name tables (notably longest-first) to avoid partial/accidental matches.

- **`xlog(string)` → `.void`**  
  Low-level logging hook.

- **`templist(mode, index, string)` → `.string`**  
  Temporary list/debug utility; primarily used for tracing and experiments.

### 6.2 Behavioral assumptions RXPP makes

RXPP implicitly depends on these invariants in the C layer:

1. **Stem indexing semantics are consistent**: arrays behave as RXPP expects (1-based content, `.0` as count).
2. **`insert_array()` is order-preserving** and updates `.0` correctly.
3. **Tokenization and splitting are quote-/nest-aware** (`splitargs`, `find_quoted`) to prevent corrupt expansions.
4. **Search helpers remain stable under mutation**: `fsearch/ffind` must behave predictably even as RXPP inserts lines.
5. **String splicing uses the same coordinate system** RXPP assumes (offset and length semantics must match).

The precomp interface is therefore part of RXPP’s **compatibility contract**: changes to these primitives should be treated as runtime/ABI-level changes.

---

## 7. Design notes

- RXPP is pass-oriented and mutation-based
- REXX encodes meaning; C encodes mechanics
- Stem rewriting is the most complex and sensitive subsystem
- Comments serve as architectural contracts

---

**End of RXPP internal documentation**


## 7) Stability

RXPP is **functionally stable but architecturally fluid**.

- The overall *pipeline structure* (three passes, mutable source buffer, directive-driven rewriting) is stable and has proven workable for complex transformations.
- Core subsystems such as macro expansion, conditional handling, and OO call translation are mature and unlikely to change fundamentally.
- The **stem rewrite engine** is newer and intentionally conservative: it favors correctness and traceability over minimal code generation. Expect refinements and simplifications as real-world usage increases.
- Internal APIs rely heavily on *side effects* (global arrays, line insertion, shifting indices). This is by design but means that refactoring must be done carefully and incrementally.
- Error handling is pragmatic rather than strict: RXPP prefers to emit warnings and continue where possible instead of aborting compilation.

In short: RXPP should be considered **stable for development and experimentation**, but not yet frozen as a long-term compatibility contract. Internal procedures, especially those related to stem handling and parsing helpers, may still evolve as the design settles.

## 8) Design principles

RXPP is intentionally engineered as a **mutable, single-pass-at-a-time transformation pipeline**, rather than a classical compiler with immutable ASTs.

Key principles:

### Mutable source buffer

- `source[]` is the *single source of truth* throughout all passes.
- Transformations operate by **inserting, rewriting, and suppressing lines in-place**.
- This mirrors how Rexx itself is typically reasoned about: line-oriented, procedural, and text-centric.
- It avoids the overhead and complexity of building and maintaining a full syntax tree for a language that is intentionally flexible and dynamic.

### Side effects are first-class

- Procedures are allowed (and expected) to:
    - insert new lines
    - advance `lino`
    - modify `stype[]`
    - update global counters and tables
- This enables *local reasoning*: a handler fully performs its transformation and leaves the buffer in a consistent state for the next step.

### Pragmatism over purity

- RXPP optimizes for **understandability and debuggability**, not formal correctness proofs.
- When in doubt, RXPP prefers:
    - emitting extra lines rather than compact ones
    - temporary variables rather than clever inlining
    - comments and warnings rather than hard failures

### Deterministic, linear processing

- Each pass walks `source[]` linearly.
- Insertions always happen *ahead* of the current line or at known anchors.
- No pass relies on backtracking or speculative parsing.

## 9) Known limitations and non-goals

RXPP deliberately does **not** try to be a full Rexx compiler or parser.

Known limitations:

- No full syntax tree: complex grammatical ambiguities are handled heuristically.
- Limited error recovery: malformed constructs may produce odd expansions rather than precise diagnostics.
- Macro expansion is textual by design; it does not understand semantic scopes.
- Conditional logic (`##IF`) is evaluated against precompiler variables only, not runtime values.
- Performance scales linearly with source size but can degrade with heavy macro nesting or aggressive stem rewriting.

Explicit non-goals:

- Perfect compatibility with all Rexx dialect edge cases.
- Static type checking or semantic validation.
- Optimized code generation.
- Rewriting RXPP into an AST-based compiler framework.

## 10) Mental model for contributors

To work safely and effectively on RXPP, it helps to adopt the right mental model.

### Think in passes, not features

- Every change belongs clearly to **one pass**.
- If a feature seems to require cross-pass awareness, it likely needs refactoring or explicit staging.

### Treat `lino` as fragile state

- Any procedure that inserts lines must:
    - know exactly *where* it inserts
    - update and return `lino` correctly
- Off-by-one errors almost always manifest as mis-ordered output rather than crashes.

### Respect `stype[]`

- `stype[]` is the control plane of RXPP.
- If you insert lines, decide immediately whether they should be:
    - inspected again
    - skipped (`noexp`)
    - treated as generated (`R`)

### Prefer clarity over cleverness

- RXPP is read far more often than it is modified.
- Future maintainers (including yourself) benefit from:
    - explicit helper procedures
    - verbose intermediate variables
    - comments explaining *why*, not *what*

### Add instrumentation early

- Many subsystems already support debug logging via `cflags`.
- When adding new logic, make it observable first; optimize later.

### Assume global context

- RXPP is not re-entrant and not thread-safe.
- Changes should assume a single compilation in progress, with shared global state.

Following these principles makes it possible to extend RXPP confidently without destabilizing the pipeline.

## 11) Overview of remaining helper procedures

The following procedures support RXPP internally and are generally not entry points for new features. They are grouped by responsibility and briefly described.

### Array and list utilities (precomp interface)

- `insert_array` – insert empty slots into a Rexx stem array, shifting existing entries.
- `drop_array` – clear a Rexx stem array efficiently.
- `copy_array` – copy a slice of one array into another.
- `search_array` – linear search for a string in an array with match control.
- `list_array` – debug helper to dump array contents.
- `sort_bylen` / `shell_sort` – ordering helpers used mainly for macro name handling.

### String and token helpers

- `changestr` – replace all occurrences of a substring (used implicitly via helpers).
- `countstr` – count occurrences of a substring.
- `fpos` – case-insensitive substring search with offset.
- `qpos` – quote-aware substring search.
- `qsplit` / `qsplitsafe` – delimiter-based split, optionally parenthesis- and quote-aware.
- `qword` / `qwords` – quote-aware word extraction.
- `safe_quote` – quote a string safely for Rexx source emission.

### File I/O helpers

- `readall` – read a file into a Rexx array.
- `writeall` – write a Rexx array to a file.

### Logging and diagnostics

- `xlog` – low-level logging hook from the C layer.
- `templist` – temporary list/debug utility used during macro expansion.

### Miscellaneous helpers

- `hasmacro` – detect whether a macro call exists in a line.
- `findvar` / `getvar` / `setvar` – access and manage precompiler variables.
- `quote` – simple string quoting helper.
- `embed` – embed string literals safely in generated code.

These helpers are intentionally small and focused. Most are thin wrappers around native C functions in `precomp.c`, designed to keep RXPP logic readable while offloading repetitive low-level work.

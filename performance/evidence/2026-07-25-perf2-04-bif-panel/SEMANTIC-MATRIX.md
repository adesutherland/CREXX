# PERF2-04 semantic-invariant matrix

This matrix is derived from the current Level B sources, their RexxDoc/API
companions, current RXAS instruction reference and focused optimized/no-opt
tests. It does not substitute generic REXX behavior for the repository's
actual contract.

"Product gate" means a future production candidate must pass the whole row.
"PoC boundary" states exactly what the scratch control proves. A speed result
outside that boundary cannot select production ownership.

## Cross-family invariants

| Invariant | Current contract | Product gate |
| --- | --- | --- |
| Text unit | `.string` positions and lengths are Unicode codepoints over valid UTF-8, not byte offsets | ASCII, multibyte, combining-codepoint and embedded U+0000 cases on both VMs |
| Index origin | public positions are one-based; assembler cursors/index operands are zero-based | first, last, after-end and conversion-at-most-once cases |
| Source value | by-value source isolation and exposed/reference alias lifetime remain exact | direct, repeated/overlapping actuals, source-import and binary-import fixtures |
| Result ownership | returned string/value is owned exactly as today; identity-preserving returns remain allowed where documented | fresh/aliased input, repeated result use, copied and reference actuals |
| Evaluation | actuals and defaults retain left-to-right/current evaluation order; omitted defaults run only when needed | side-effecting actual/default and omitted-hole/status tests |
| Error | signal identity, order and payload remain exact | first-invalid argument wins; optimized/no-opt parity |
| Observation | source steps, function/argument/result TRACE events and source identities remain exact | TRACE on/off, local/source-import/binary-import forms |
| Transport | existing I6 bodies must validate by schema, declaration and body shape; any new fact is independently reconstructed | correct, missing, old, malformed and contradictory evidence tests |
| Runtime | `rxvm` and `rxbvm` agree on output, signal and state/cursor effects | both VMs, optimized and no-opt |

The scratch source rewrites are TRACE-off machine ceilings. They add post-timer
result/source guards, but they do not reproduce the original BIF function TRACE
event. That gap is explicit and remains a production blocker until a compiler
implementation preserves it or fails closed.

## LEN - `LENGTH`

Authoritative surface: `lib/rxfnsb/rexx/length.crexx` and `length.md`.

| Case | Current result/effect | Required distinction |
| --- | --- | --- |
| empty | `0` | no stale cached UTF-8 count |
| ASCII/multibyte/combining | number of codepoints | never byte length or grapheme count |
| embedded U+0000 | counts as one codepoint | scan does not terminate at NUL |
| source ownership | source unchanged | direct, computed and aliased source |
| error | valid `.string` has no Level B error branch; VM still enforces valid UTF-8 | signal/source parity |

`LEN-H1` proves only RexxCPS's positive dynamic decimal-to-string values and
direct `strlen` destination placement. A discarded scratch construction also
proved why representation state matters: initializing a reused string register
to `""` left stale cached codepoint metadata across `dcopy; dtos`, yielding a
false zero length. The accepted typed-null control matches current temporary
state and returns `3`. Production cleanup must preserve all string metadata,
not just textual bytes.

`P04-CEX1` separately certifies exact constant `LENGTH` calls through the VM's
validated value representation and codepoint count. Empty, multibyte,
combining and embedded-U+0000 cases are guarded in source/binary imports and
both VMs. Dynamic and body-drifted calls retain the body, so this certificate
does not reopen rejected dynamic LEN-H1 result placement.

## SLC - `SUBSTR`, `LEFT`, `RIGHT`

Authoritative surfaces: the matching `.crexx` and `.md` files.

| Case | Current contract | Product gate |
| --- | --- | --- |
| start | `SUBSTR` start is positive and one-based | zero/negative signal before later work; first/last/after-end |
| omitted length | returns from start to end; distinct from supplied zero | omitted-status transport and evaluation order |
| supplied length | non-negative; zero returns empty | zero, exact, shorter and beyond-end |
| padding | default blank or exactly one explicit codepoint, right-padded for `SUBSTR`/`LEFT`, left-padded for `RIGHT` | ASCII and multibyte pad; empty/multiple pad signal |
| full-width identity | may return the original string value when the selected width is exact | result ownership/alias lifetime |
| cursor | current primitives may move a source register cursor while preserving text | repeated/overlapping actual and subsequent cursor-sensitive use |
| empty/out of range | exact empty/padded behavior from the maintained body | no underflow from one-to-zero-based conversion |

`SLC-H1` covers only the two current RexxCPS sites: ASCII constants, positive
in-range starts, supplied positive lengths, no padding, and TRACE off. It is
not proof for omitted length, validation, Unicode seeking, padding, aliasing or
error order. Those cases stay on the complete Level B fallback unless a
production proof admits them separately.

`P04-CEX1` supplies that separate proof for constant `LEFT`/`RIGHT`: width is
validated before pad, pad must be exactly one codepoint even when width is
zero, truncation uses zero-based internal cursors, and padding repeats the
exact pad bytes on the correct side. Negative widths, invalid pads, dynamic
actuals and results above the 1,048,576-codepoint compiler bound retain the
complete Level B call and its signal order.

## WRD - `WORD`, `WORDS`, `WORDPOS`

Authoritative surfaces: `word.crexx`, `words.crexx`, `wordpos.crexx` and their
API companions. Word boundaries are those implemented by the VM's Unicode
blank-search primitives.

| Case | Current contract | Product gate |
| --- | --- | --- |
| numbering | positive one-based word/start arguments | zero/negative `INVALID_ARGUMENTS` before scan |
| separators | runs of VM-recognized Unicode whitespace delimit words | ASCII spaces, U+2003/U+3000 and mixed runs |
| missing word/phrase | `WORD` returns empty; `WORDPOS` returns zero | empty and blank-only source/search |
| equality | `WORDPOS` phrase and `WORD` result comparison are exact/case-sensitive | repeated words, prefix/suffix and same-length mismatch |
| source | scans use codepoint positions and preserve source text | multibyte word and separator cases |
| result | `WORD` materializes the selected word under the current ownership contract | no borrowed cursor/result lifetime |

`WRD-L1` is an exact-use predicate control for `word(key1,1) = "?"`. Its
guard compares the control with the real `WORD` result on empty, blank-only,
ASCII and multibyte whitespace/text cases. It deliberately does not establish
a new public predicate, full `WORD` replacement, `WORDS`, `WORDPOS`, function
TRACE equivalence or multi-site generality.

## SRC - `POS`, `LASTPOS` and related search

| Case | Current contract | Product gate |
| --- | --- | --- |
| start/end | positive one-based public bounds | zero/negative signal, after-end, clamped `LASTPOS` end |
| empty operands | current Level B functions return zero | empty needle/haystack combinations |
| match | whole exact codepoint sequence; overlapping starts remain observable | repeated/overlapping and multibyte cases |
| result | one-based first/last match or zero | first, last and no-match |
| algorithm | `POS` uses one existing `strpos`; `LASTPOS` iterates it | no new assist unless a general multi-site control beats composition |

Base64 uses `POS(character, alphabet)` on one-codepoint ASCII values. The
codepoint-arithmetic PoC replaces that exact fixed-alphabet lookup; it is not a
general `POS` implementation.

## CAS - `UPPER`, `LOWER`

Authoritative surfaces: `upper.crexx`/`upper.md`, `lower.crexx`/`lower.md`, and
the `strupper`/`strlower` RXAS contract.

| Case | Current contract | Product gate |
| --- | --- | --- |
| mapping | VM's deliberately limited, locale-independent simple Unicode table | ASCII, `äöüé`, unchanged characters and non-covered mappings |
| length | not full Unicode case folding; current simple mappings preserve encoded width | no expansion/contraction claim |
| empty/U+0000 | empty unchanged; U+0000 is ordinary and later text is still mapped | embedded-NUL fixture |
| source | input text is unchanged; destination cursor resets | alias source/destination and repeated actuals |
| `UPPER` binding | exposed input is a zero-copy source binding but the body writes only a separate result | symbol, literal, computed and alias actuals |
| observation | current BIF source/function trace remains visible in optimized form | no source rewrite substitution |

`CAS-L0` proves the exact current body can inline when a direct symbol actual is
available. `CAS-H1` proves direct dynamic-input `strupper` result placement.
`CAS-L2` proves the canonical values of the four exact RexxCPS constant
arguments. Neither H1 nor L2 reproduces the original UPPER function TRACE event;
production lowering/folding must use the authoritative mapping and retain that
observation or fail closed.

`P04-CEX1` extends the same canonical case evaluator to exact `LOWER` providers.
A same-summary body using `strupper` is deliberately rejected by fingerprint;
nested `LENGTH(LOWER(constant))` proves composition after ordinary flow-aware
constant reduction.

## DAT/CNV - typed classification and conversions

`DATATYPE` has its own Level B ASCII catalog, numeric parser and exact
`INVALID_ARGUMENTS` behavior. The binary/hex/character/decimal helpers have
individual typed signatures, range rules and conversion errors. No optimized
Tier A dynamic Level B call currently selects one of these bodies.

RexxCPS's measured `ITOS`, `DTOS` and `STOD` instructions are language/value
representation crossings, not calls to these BIF modules. Their numeric
context, exact integer range, decimal spelling and error paths remain PERF2-07
work. PERF2-04 therefore makes no semantic shortcut or native ownership claim
for DAT/CNV.

## B64 - fixed-valid Base64 algorithm control

The current benchmark's deciding cell uses deterministic 1,024-byte input,
1,368 encoded codepoints, valid ASCII alphabet/padding, exact 1,024-byte
round-trip result and checksum 130,560.

| Case | PoC proof | Missing before any API selection |
| --- | --- | --- |
| no padding / one / two | canonical 1,024-byte input exercises one `=` padding codepoint | separate empty and all padding widths |
| alphabet | fixed standard ASCII alphabet | malformed, whitespace, alternate alphabet and signal contract |
| output | exact bytes, encoded length and checksum | ownership/alias and API signature |
| Unicode | codepoint-safe extraction of an ASCII encoded string | explicit non-ASCII rejection semantics |
| native control | same fixed valid cell | every malformed/error/TRACE/API behavior |

`strchar` and `poschar` also update their string operand's cached byte/codepoint
cursor in the current VM implementation. Private control operands make that
safe here; a general optimizer must prove or model the cursor effect instead of
treating these instructions as observationally pure reads.

The native C control is therefore an upper bound only. CAP-03 keeps the common
benchmark unchanged and calls for a separately specified pure Level B Base64
API. That API needs its own semantic matrix before production; the current
algorithm result does not authorize benchmark-specific rewriting.

## Current correctness floor

- 72/72 focused exact-B0-R CTests pass, including optimized/no-opt seed BIFs,
  imported body fallback/shape/version tests, reference lifetime and the
  PERF2-03 receiver/accessor guards.
- 32/32 dual-VM optimized/no-opt Level B PoC cells pass for the UPPER call-shape
  and Base64 controls.
- Each retained RexxCPS ceiling must pass both ordinary Release VMs with empty
  stderr and its post-timer exact result/source guard before timing evidence is
  admitted.

These are focused panel checks, not broad production closeout. A selected
future slice adds exactly the distinguishing regression tests its proof needs.

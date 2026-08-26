# Level L Bootstrap Frontend, cREXX Lexer, And Prepared D-Form Proof

This retained experiment implements the first executable Level L lexer-maker
slice entirely outside the production compiler.

The implemented path is:

```text
authored .levell
    -> hand-written Level B scanner and deterministic parser
    -> Level B Token objects and automatic structural AST
    -> validated IDs, references, actions, and lexer invariants
    -> deterministic re2c adapter input
    -> retained re2c 4.5.1 cREXX syntax adapter
    -> generated Level B scanner
    -> rxc -> rxas -> rxlink -> rxtvm and rxbvm

retained ICU nfc.txt
    -> generated Level L scanner
    -> hand-written deterministic Level B record parser
    -> typed version/CCC/mapping AST with byte and token spans
    -> deterministic semantic dump
    -> byte comparison with the independent C++/re2c oracle
    -> versioned portable Level B table image
    -> recursive canonical and algorithmic Hangul decomposition
    -> stable canonical combining-class ordering
    -> complete Unicode 17.0.0 NFD conformance on both VMs

retained UnicodeData.txt
    -> deterministic hand-written Level B UCD parser
    -> canonical and compatibility typed decomposition ASTs
    -> generation-time recursive closure and canonical ordering
    -> one portable prepared NFD/NFKD image
    -> direct strict UTF-8 byte scan with prepared actions
    -> complete Unicode 17.0.0 NFD and NFKD conformance on both VMs
```

The authored language is Level L. re2c is used only to construct and emit the
DFA in this proof; its syntax and embedded-action surface are not exposed to a
Level L author.

## Implemented surface

`levell_bootstrap.crexx` provides Level B `levelltoken`, `levellastnode`, and
`levellresult` classes plus the bootstrap parser and adapter emitter. The
frontend implements the documented first lexer subset:

- `options levell`, `namespace`, Rexx `/* ... */` comments, and static dotted
  declaration names;
- `token`, `diagnostic`, `lexer input`, `charset`, `pattern`, and `rule`
  declarations;
- Unicode scalar literals, inclusive ranges, charset union and subtraction,
  `scalar`, grouping, concatenation, alternation, and repetition;
- `emit`, `skip`, and `error` actions plus `eof`, `malformed`, and `otherwise`
  rule subjects;
- deterministic token-ID assignment, reference/action validation, special-rule
  validation, non-empty ordinary-rule checks, source byte spans, token dumps,
  and structural AST dumps; and
- a first-error panic with no recovery or speculative parser backtracking.

`specs/tinyexpr.levell` is the small differential proof.
`specs/gennorm2.levell` is the real Unicode-language input: it describes the
lexer for ICU's retained `gennorm2` normalization file notation, including
version, hexadecimal, range, canonical-combining-class, mapping, comment, and
line-boundary tokens.

`unicode_gennorm2.crexx` consumes those generated tokens with a second
single-cursor, one-token-lookahead parser. It constructs
`unicodegennorm2record`, `unicodegennorm2ast`, and `unicodegennorm2result`
objects for the version declaration, CCC singles/ranges, and one-/two-way
mappings. Each record retains its source line, exact byte span, and inclusive
token span. Validation rejects unsupported/duplicate versions, malformed
productions, non-scalars, invalid or overlapping CCC records, duplicate
mappings, and invalid mapping arity at the first fault.

The record byte span starts at its first semantic token and ends immediately
before the line terminator, so it includes any skipped spacing/comment tail.
The token span is stored as a zero-based start plus count and includes the
terminating newline token.

`unicode_nfd.crexx` compiles the typed Unicode AST into a deterministic
1,186,472-byte proof image. The image has a 64-byte versioned header, a dense
canonical-combining-class byte table, a two-stage mapping-page index, fixed
mapping records, and a shared code-point value pool. All persistent fields use
portable little-endian `<at..u8>`, `<at..u16>`, and `<at..u32>` access. Its
Unicode 17.0.0 image SHA-256 is
`dd7e03a88172dd451bf4c353771c565d07ade91f64ec8cbab81b9e6b33362944`.

The same module provides an explicit Level B NFD normalizer over a validated
image. It implements recursive canonical decomposition, algorithmic Hangul
decomposition, stable canonical ordering, direct CCC lookup, paged mapping
lookup, and a cycle/depth panic. Table, Hangul, and binary-search arithmetic
uses the mode-independent Level B `<idiv>` and `<mod>` operators, so the result
does not depend on the source file's `OPTIONS NUMERIC_*` parser mode.

`unicode_data.crexx` parses the retained Unicode 17.0.0 `UnicodeData.txt`
directly into the existing typed normalization AST. It rejects malformed
records, non-scalar mappings, duplicate/out-of-order records, and CCC 255 at
the first fault. `unicode_d.crexx` closes every canonical and compatibility
mapping recursively at generation time, applies canonical ordering to the
complete expansion, classifies the resulting boundary shape, and emits one
deterministic 1,659,772-byte portable image. The image SHA-256 is
`224dc0355cdf2ea4bbe51640bde15c2687d6a719196b8332f3455a2ccda5500e`.

The shared NFD/NFKD engine scans UTF-8 bytes directly. ASCII identity runs are
coalesced, mapped expansions are stored pre-encoded, Hangul uses arithmetic,
and only the open trailing nonstarter run is buffered. Monotone CCC runs append
directly; an inversion switches to a stable counting-bucket pass over the 55
positive CCC ranks. A `.binary` ingress validates shortest-form UTF-8 and
rejects malformed, surrogate, truncated, and out-of-range encodings.

The `.string` and `.binary` source arguments use Level B exposed-argument
binding as a read-only borrow. Optimized RXAS therefore contains no whole-input
`scopy`, `bcopy`, or `stobin`: `sblen` obtains UTF-8 byte length, `sgetu8` reads
a string byte, and `sbmove` copies selected string spans into prepared output.
The `.string` result currently incurs one final `bcopy` plus `bintos` ownership
conversion. `normalize_binary` retains a binary result and avoids that
string-result conversion. An ownership-transfer conversion is a separate
future RXAS decision, not part of this proof.

## Reproduce

From the repository root:

```sh
experiments/unicode/level-l-bootstrap/run.sh
```

The default product build is `cmake-build-unicode-debug`. Optional environment
variables are:

- `CREXX_BUILD_DIR`: product build directory;
- `CREXX_LEVEL_L_BOOTSTRAP_BUILD_DIR`: untracked experiment output directory;
- `CREXX_BUILD_JOBS`: parallel build count, default `10`; and
- `RE2C_BIN`: alternate re2c executable, which must report version `040501`;
- `CXX`: C++ compiler used for the independent semantic oracle, default `c++`.

The run regenerates the Token dump, AST dump, re2c adapter input, and final
cREXX for both specifications. It requires byte identity with the committed
files under `generated/`, compiles and links both generated scanners and the
scalar NFD and prepared NFD/NFKD modules, and runs all focused checks on
`rxtvm` and `rxbvm`. It also
regenerates and runs the independent C++/re2c NFD oracle and requires its
summary to match the retained Phase 2 evidence.

The TinyExpr scanner agrees with maintained `rxfnsl` token kinds, names, and
byte spans. The Unicode scanner accepts all 2,500 lines of the retained ICU
78.3 `nfc.txt` and checks exact counts for the 163 ranges, 403 CCC records, 961
two-way mappings, 1,120 one-way mappings, version declaration, line endings,
and EOF. A non-ASCII comment fixture checks UTF-8 matching and byte offsets.
The record parser then constructs 2,485 typed records containing 403 CCC
records covering 968 code points, 961 two-way mappings, 1,120 one-way
mappings, and 3,127 mapping values. Its complete semantic dump is
byte-identical to the retained C++/re2c parser; its span-bearing record dump is
byte-identical across both VM families. Focused fixtures prove CRLF/UTF-8 byte
spans plus malformed-rule and unsupported-version first-fault panics.

The Level B normalizer passes all 100,170 applicable NFD relations from Unicode
17.0.0 `NormalizationTest.txt` and all 1,094,978 required identity checks for
scalars absent from Part 1. Focused cases cover empty and ASCII input, direct
and recursive Latin decomposition, canonical reordering, Hangul LV/LVT,
non-BMP identity, CCC lookup, and cyclic mapping rejection. Both VM families
produce the byte-identical retained summary in `evidence/nfd-result.txt`.

The prepared engine independently passes all 200,340 NFD and NFKD relations
from the same 20,034-row corpus and all 1,094,978 required unlisted-scalar
identity checks for each form. Focused fixtures cover compatibility and
recursive expansion, cross-source-character ordering, marks-only mappings,
stable equal-CCC ordering, a long disordered bucket run, Hangul, non-BMP
identity, exact D-form Quick_Check, binary parity, and malformed UTF-8. Both VM
families reproduce `evidence/unicode-d-result.txt` byte for byte.

## Deliberate boundaries

- This is an experimental Level B tool, not new syntax in production `rxc` and
  not a released Level L compiler.
- The bootstrap source scanner currently accepts ASCII Level L source. Unicode
  input symbols are fully expressible with `U+` literals; a non-ASCII authored
  literal panics explicitly rather than being misread.
- Generated scanners currently accept `.string`, whose Level B ingress contract
  already guarantees valid UTF-8. The authored `malformed` rule is validated
  but unreachable and omitted by this adapter. A future `.binary` input path
  must add an explicit UTF-8 decoder before claiming malformed-byte handling.
- The PoC emitter walks the validated syntax tree directly and uses re2c as its
  automaton backend. A separately reusable neutral IR and native Level B/Level L
  automaton builder remain later work.
- Tokens use a portable 12-byte proof layout with `<at..u16>`/`<at..u32>`.
  Both normalization tables are also portable proof layouts. The prepared
  image is much smaller than the earlier packed NFD experiment, so portable is
  the default research direction. None is an approved product format;
  host-native packed and load/convert-once representations remain measured
  alternatives after the PoC.
- The Unicode rule parser and NFD table compiler cover the retained `gennorm2`
  canonical data only. They are not a general UCD/property compiler.
- The scalar baseline implements NFD. The prepared byte engine implements NFD
  and NFKD, but not NFC/NFKC composition, chunked input, case folding,
  segmentation, or a public Level G API.
- Exposed source arguments are read-only by convention but Level B cannot yet
  express a const borrow. A future public API must decide whether to retain
  that convention, add a read-only borrowing surface, or accept an input copy.
- D-form state retains only the open trailing nonstarter run, but the current
  public methods accept one complete contiguous input. Arbitrary chunk splits
  and incomplete UTF-8 prefix state remain a later streaming proof.
- Level L parser-maker syntax, self-lexing/self-hosting, recovery, token
  payloads, streaming, and physical packed AST storage remain later work.

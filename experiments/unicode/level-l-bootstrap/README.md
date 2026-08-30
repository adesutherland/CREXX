# Level L Bootstrap Frontend, cREXX Lexer, And Prepared Normalization Proof

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
retained DerivedNormalizationProps.txt
    -> deterministic hand-written Level B UCD parser
    -> canonical and compatibility typed decomposition ASTs
    -> normative Full_Composition_Exclusion set
    -> generation-time recursive closure and canonical ordering
    -> exact Primary Composite directory plus algorithmic Hangul
    -> one portable prepared NFD/NFKD/NFC/NFKC image
    -> dense portable NFD virtual-alphabet table
    -> native .string codepoint scan for prepared NFD
    -> strict UTF-8 .binary ingress for the general engine
    -> fused ordered-stream canonical composition for C forms
    -> complete Unicode 17.0.0 four-form conformance on both VMs

prepared NFD scalar partition
    -> one at-u32 prepared symbol per Unicode scalar
    -> exact mapping, identity-mark, Hangul, and invalid-scalar kinds
    -> generated thin Level B wrapper
    -> shared bounded-register exact-kind SELECT executor
    -> complete Unicode 17.0.0 NFD conformance on both VMs
```

The authored language is Level L. The retained re2c adapter still processes the
generated wrapper in this proof, but the selected prepared NFD route does not
ask re2c to construct a UTF-8 byte DFA. re2c syntax and embedded actions are not
exposed to a Level L author.

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
the first fault. `unicode_normprops.crexx` independently parses the normative
`Full_Composition_Exclusion` property and audits its 1,120 Unicode 17 scalars.
`unicode_d.crexx` closes every canonical and compatibility mapping recursively
at generation time, applies canonical ordering to the complete expansion,
classifies the resulting boundary shape, and emits one deterministic
6,179,204-byte portable image. The image SHA-256 is
`4712c3ae75a8a0217cfb650bb0d97a40d7e2921e1f7981f5c40420684ec38b09`.

The version-3 image retains each prepared scalar beside its CCC and pre-encoded
UTF-8 and adds a 4,456,448-byte portable `<at..u32>` NFD virtual-alphabet
section. Identity starters classify as their scalar value. Synthetic high-bit
kinds identify nonstarters, the four prepared mapping boundary classes,
algorithmic Hangul, and invalid scalars. The image also contains 961 exact
non-Hangul Primary Composite pairs under 391 starters; the largest starter has
19 candidate pairs. Full composition exclusions are applied while generating
the relation, and Hangul L+V and LV+T composition is arithmetic rather than
table-driven.

The prepared NFD `.string` route iterates CREXX codepoints with `STRCHAR`, reads
one virtual-alphabet symbol with `BGETU32`, and dispatches its exact synthetic
kind through the compiler's `.jtable`/`JUMPI` lowering. Identity output uses
`APPENDCHAR`; prepared decomposition records retain the mathematically derived
closed/open boundary so only the still-open nonstarter run is queued and
ordered. No UTF-8 binary conversion or whole-input copy occurs on this route.

The shared general engine remains available to all four forms and to explicit
`.binary` input. It scans UTF-8 bytes directly, uses pre-encoded mapped
expansions, and retains only the open trailing nonstarter run. Monotone CCC runs
append directly; an inversion switches to a stable counting-bucket pass over
the 55 positive CCC ranks. C forms feed the same ordered stream into a fused
UAX #15 composer retaining the active starter, D117 blocking CCC, and
still-mutable ordered tail. The binary ingress validates shortest-form UTF-8
and rejects malformed, surrogate, truncated, and out-of-range encodings.

`unicode_normalization_generate.crexx` embeds the combined 11,237,664-byte
normalization and case-fold image once in the generated Level B source as the
named `.binary` constant `UNICODE_DATA`. Production `rxc` owns the lowering:
it retains one shared compiler-side payload, writes one compiler-private
module-scoped RXAS `.const`, and uses that alias for every exact table operand.
The former placeholder, `.rxasconst` sidecar, and Rexx RXAS-sealing program are
removed. This changes build ownership only; runtime table access remains
read-only and zero-copy.

The retained hand-written oracle accepts `.binary` directly and implements its
`.string` wrapper through the existing explicit string/binary conversions. The
experimental raw string-byte RXAS surface was rolled back. The selected NFD
successor instead uses the ordinary `STRCHAR` and `APPENDCHAR` string surface,
so it needs neither a raw byte view nor a whole-string binary copy.

`unicode_nfd_lexer_generate.crexx` is the retained prepared-algorithm lowering
pass. It does not add arbitrary actions to the authored Level L syntax. The
validated Unicode scalar partition is compiled into the image's virtual
alphabet, while this pass emits a deterministic thin wrapper around the shared
Level B executor. The executor's exact consecutive kinds compile to a jump
table and its optimized `normalize()` currently declares 32 locals, independent
of the number of Unicode mappings. The earlier 3,440-state UTF-8 DFA remains a
useful rejected experiment: it proved correctness but amplified frame and
per-scalar costs and is not the selected prepared route.

## Reproduce

From the repository root:

```sh
experiments/unicode/level-l-bootstrap/run.sh
```

The default product build is `cmake-build-unicode-debug`. Optional environment
variables are:

- `CREXX_BUILD_DIR`: product build directory;
- `CREXX_LEVEL_L_BOOTSTRAP_BUILD_DIR`: untracked experiment output directory;
- `CREXX_LEVEL_L_BOOTSTRAP_NOOPT_DIR`: caller-supplied clean noopt staging
  directory; otherwise the harness creates a fresh retained directory;
- `CREXX_BUILD_JOBS`: parallel build count, default `10`; and
- `RE2C_BIN`: alternate re2c executable, which must report version `040501`;
- `CXX`: C++ compiler used for the independent semantic oracle, default `c++`.

The run regenerates the Token dump, AST dump, re2c adapter input, and final
cREXX for both specifications. It requires byte identity with the committed
files under `generated/`, compiles and links both generated scanners and the
scalar NFD and prepared four-form modules, and runs all focused checks on
`rxtvm` and `rxbvm`. The normalization harness is additionally rebuilt with
both `rxc -n` and `rxas -n` and rerun on both VMs. It also
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

The prepared engine independently passes all 400,680 NFD, NFKD, NFC, and NFKC
relations from the same 20,034-row corpus and all 1,094,978 required
unlisted-scalar identity checks for each form. Focused fixtures cover
compatibility and recursive expansion, cross-source-character ordering and
composition, marks-only mappings, Full Composition Exclusions, D117 blocking,
stable equal-CCC ordering, a long disordered bucket run, chained and Hangul
composition, non-BMP identity, exact normalization predicates, binary parity,
and malformed UTF-8. Optimized and compiler/assembler-noopt images on both VM
families reproduce `evidence/unicode-normalization-result.txt` byte for byte.

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
- The scalar baseline implements NFD. The shared general engine implements all
  four normalization forms over `.string` codepoints: NFD/NFC use the dense
  prepared canonical classification, NFKD/NFKC use the prepared compatibility
  bit/page/record index, and C forms feed decomposed scalars into the shared
  primary-composition index. D-form `is_normalized` is an exact one-pass quick
  check. C-form predicates read the official Unicode 17 NFC_QC/NFKC_QC value
  for each codepoint from a packed shared-constant section, reject `No`, check
  canonical-order boundaries, and return immediately when every value is
  `Yes`; a `Maybe` value alone invokes the existing exact streamed
  decomposition/composition comparison without constructing normalized
  output. The binary surface remains a separate experimental UTF-8 path.
  Neither surface
  implements chunked input, case folding, segmentation, or a public Level G
  API.
- Exposed source arguments are read-only by convention but Level B cannot yet
  express a const borrow. A future public API must decide whether to retain
  that convention, add a read-only borrowing surface, or accept an input copy.
- D-form state retains only the open trailing nonstarter run; C forms also
  retain the active starter and its mutable ordered tail. The current public
  methods accept one complete contiguous input. Arbitrary chunk splits and
  incomplete UTF-8 prefix state remain a later streaming proof.
- Level L parser-maker syntax, self-lexing/self-hosting, recovery, token
  payloads, streaming, and physical packed AST storage remain later work.

# Level L Language Tooling And Level G Unicode Research Track

Status: Phase 1 and Phase 2 complete; the Level L lexer syntax and bounded re2c
output-adapter experiment are complete. The Level L front end and generator
core remain research work.

Branch: `unicode`, based on `origin/develop` at `7b78375bd` on 2026-08-25.
The branch is intentionally kept separate from `develop` until its contracts,
implementation, conformance evidence, and integration decision are approved.

## Purpose

This track joins two related goals without pretending that they are the same
product surface:

1. build language-engineering tools whose authored surface is Level L, starting
   with a lexer maker and leaving a parser maker as a later milestone; and
2. use those tools and CREXX's binary-memory facilities to generate the pinned
   Unicode data and algorithms needed by explicit Level G Unicode services.

The work should also expose measured gaps in the CREXX/RXAS binary surface. A
gap is evidence for a possible change, not advance approval for new syntax,
opcodes, ownership rules, or architecture. Those decisions remain explicit
approval gates.

## Boundary And Maturity

- Level L remains experimental and developer-only. `OPTIONS LEVELL` currently
  selects the Level B-derived compiler pipeline; it is not a separate mature
  language implementation.
- Level G owns normalization, full case folding, Unicode properties, collation,
  and segmentation. Level B must retain its valid-UTF-8/codepoint contract and
  must not silently gain normalization or grapheme semantics.
- This track must not change CREXX identifier spelling, identifier equality,
  keyword matching, source normalization, or Level C byte/UTF8 profiles as an
  incidental consequence of building Unicode tooling.
- Unicode operations must be explicit. "Unicode comparison" is too vague to be
  an API contract: exact comparison, canonical-equivalence comparison, full
  caseless matching, identifier matching, and locale collation have different
  rules.
- The first output is research and generated-shape evidence. It is not a claim
  that Level L, a lexer generator, a parser generator, or Level G Unicode is
  released or production-ready.

## Existing CREXX Starting Point

### Lexer And Parser Toolchain

- CREXX vendors re2c 4.5.1 and uses it with UTF-8 mode (`-8`) to generate the
  compiler and assembler scanners from `.re` sources.
- The shared `utf8/encoding.re` includes re2c's generated Unicode category
  definitions. Current compiler identifiers use a project-specific category
  expression based on `L`; that is not yet a UAX #31 identifier policy.
- CREXX uses the vendored Lemon LALR(1) parser generator for its current
  grammars. Lemon supports a caller-supplied parser driver template with `-T`,
  but the current grammar actions and template are C-oriented.
- re2c 4.5.1 supports user-defined language syntax files, but that does not make
  re2c source syntax or its backend model the intended Level L contract. Its
  first role in this track is as an executable notation and conformance oracle
  for restating the Unicode manuals' IBM-specific generator rules.

### Level L Generated-Shape Proof

`lib/rxfnsl/rexx/tinyexpr.crexx` is already a useful baseline. It proves:

- a packed binary character-class constant;
- fixed 16-byte token records containing kind, byte start, byte length, and a
  value;
- direct `<at..u8>`, `<at..u16>`, `<at..u32>`, and `<at..int>` access;
- binary buffer growth with `binresize`;
- zero-copy lexeme comparison with `<compare..binary>`; and
- a small precedence parser over the packed token stream.

It is hand-written in a generated shape. It is explicitly not a lexer or parser
generator, but it gives the first target against which generated output can be
compared.

### Binary Surface

The portable interchange/storage surface is the byte-addressed `<at..type>`
family. Fixed integer and float fields have declared widths and canonical
little-endian encoding. This is the safe baseline for generated Unicode data
that must remain byte-identical in source, RXBIN, packages, and across hosts.

The newer `<packed..int>` and `<packed..float>` forms expose faster host-native
VM representations. Their host width/endian meaning prevents a packed image
from automatically serving as the committed cross-architecture format, but
does not rule them out as the runtime layout. A portable source/model could
generate or materialize host-native packed tables during build, install, or
load. The post-PoC layout verdict must therefore measure portable `<at..type>`,
host-native packed, and load/convert-once hybrid shapes rather than presuming
portability is the only criterion.

Current strengths include:

- readable binary constants without copying them merely to inspect fields;
- mutable binary values with reusable private capacity;
- fixed-width bounds-checked loads and stores;
- in-place resize, fill, copy, move, append, update, gap, and drop helpers;
- zero-copy comparison of source/table spans; and
- optimized packed dispatch support already used elsewhere in the compiler.

Known limitations relevant to generated tooling include:

- writes do not resize a buffer, so capacity/logical length must be planned;
- variable-size binary span writes and explicit-length string-field writes are
  incomplete;
- constants are source-module-local and generated constant families require
  explicit `PROCEDURE EXPOSE` lists;
- no binary struct declaration removes repeated field-offset arithmetic;
- no general read-only binary slice/view can cross a procedure boundary;
- no automatic packed ordinary arrays exist; and
- repeated offset calculations and multiple small field reads can outweigh the
  benefit of a compact representation.

These are investigation points. The first implementation must measure actual
generated lexer and Unicode workloads before proposing changes.

### Current Unicode Contract

The repository pins claimed Unicode property data to Unicode 17.0.0.

Level B currently provides valid UTF-8 strings, codepoint-based length and
slicing, Unicode `White_Space`, and a deliberately limited locale-independent
simple case mapping. It does not normalize implicitly and it does not implement
full case folding, canonical equivalence, locale collation, or grapheme
segmentation.

The repository history reachable from current refs contains early Unicode
documentation and later UTF/codepoint work, but this review did not find the
previous re2c-based normalization/case-folding implementation or its Unicode
conformance inputs. The new branch must therefore retain source, generated
artifacts where appropriate, pinned inputs, checksums, and conformance commands
from the start.

## What "re2c-Inspired" Means

The initial proof of concept should use ordinary re2c to duplicate selected
rules specified in the Unicode manuals. Those rules are written for an
IBM-specific rule generator and notation. The PoC must record every translation
or small semantic adjustment needed to express them in re2c, then prove the
result against the applicable Unicode conformance data.

That exercise supplies three things:

- a recovered executable interpretation of the manual rules;
- a conformance oracle for the later CREXX implementation; and
- evidence about the regular-rule, action, state, table, and diagnostic
  capabilities the Level L tool actually needs.

After the PoC, the CREXX tool is better described as inspired by re2c than as a
port of re2c. It may reuse ideas such as named definitions, declarative regular
rules, longest-match/rule-priority semantics, automaton construction, and
generated table-driven scanners. It is not required to accept re2c `.re`
syntax, re2c directives, the `YY*` API, embedded C actions, or the manuals'
IBM-oriented notation.

The Level L rule language should therefore be designed from the demonstrated
Unicode and general lexer requirements, using CREXX-native concepts and packed
output. Its syntax is a later language-design decision and requires explicit
approval; neither re2c compatibility nor novelty for its own sake is a goal.

## Normative Unicode Inputs

The implementation must pin a complete Unicode version and treat the Unicode
Character Database as generated input, not as hand-maintained tables. The first
version is 17.0.0 to match the current CREXX documentation.

Primary references and inputs include:

- [UAX #15: Unicode Normalization Forms](https://www.unicode.org/reports/tr15/)
- [UAX #44: Unicode Character Database](https://www.unicode.org/reports/tr44/)
- [Unicode 17.0.0 UCD](https://www.unicode.org/Public/17.0.0/ucd/)
- `UnicodeData.txt` for general categories, combining classes, simple case
  mappings, and decomposition mappings
- `CompositionExclusions.txt` and `DerivedNormalizationProps.txt` for composed
  forms and quick-check/derived mappings
- `NormalizationTest.txt` for the required NFC, NFD, NFKC, and NFKD conformance
  invariants
- `CaseFolding.txt` for simple/full/default/Turkic case-fold mappings
- `SpecialCasing.txt` for full and contextual/locale case mappings
- `DerivedCoreProperties.txt` and `PropList.txt` for derived properties
- [UAX #29: Unicode Text Segmentation](https://www.unicode.org/reports/tr29/)
  plus its grapheme, word, and sentence break test files for later Level G work
- [UAX #31: Unicode Identifiers and Syntax](https://www.unicode.org/reports/tr31/)
  if CREXX identifier policy is opened as a separately approved language-design
  task

Every retained input must record its Unicode version, upstream URL, license
notice, and cryptographic checksum. Regeneration must be deterministic and
must fail on an unexpected format/version instead of silently producing new
tables.

## Unicode Algorithm Shape

### Normalization

The four forms are separate tested operations:

- NFD: recursive canonical decomposition followed by canonical combining-class
  ordering;
- NFC: NFD followed by canonical composition, including algorithmic Hangul and
  the composition exclusion/blocking rules;
- NFKD: recursive compatibility decomposition followed by canonical ordering;
- NFKC: NFKD followed by the same canonical composition phase used by NFC.

The previous success with NFD is a good starting point, but NFC is not "NFD in
reverse". It needs an indexed composition-pair table, starter tracking,
blocking by canonical combining class, composition exclusions, and Hangul
composition. The 17.0.0 `NormalizationTest.txt` invariants are the acceptance
oracle for all four forms, including idempotence and cross-form relationships.

### Case Operations

Case conversion and case folding must remain distinct:

- simple upper/lower mappings are one-to-one and cannot represent all Unicode
  results;
- full upper/lower/title mappings can expand and may depend on context or
  locale;
- simple case folding uses `CaseFolding.txt` statuses `C + S`;
- full default case folding uses `C + F` and can expand strings;
- Turkic folding is an explicit option using `T`, not the default; and
- case folding does not in general preserve normalization form.

`NFKC_Casefold` is a separately defined identifier-oriented mapping in
`DerivedNormalizationProps.txt`; it must not be substituted for ordinary full
case folding without an explicit API contract.

### Properties, Segmentation, And Identifiers

Property lookup is shared infrastructure, but each consumer owns its policy.
The initial property engine should support pinned value/range lookup without
claiming all UCD properties as a stable Level G API.

Grapheme, word, and sentence segmentation are later UAX #29 algorithms with
their own state/rules and conformance files. They are not consequences of
normalization.

Identifier work is also separate. If opened, it should start from `XID_Start`
and `XID_Continue`, choose an explicit normalization profile, and address UAX
#31 stability and Unicode source-security guidance. It must not be smuggled in
through the general lexer maker.

## Architectural Hypothesis To Test

The research should use a generated-data boundary:

```text
lexer specification --------> Level L lexer tool -----> Level L scanner source/tables
                                      |
Unicode 17 UCD files --------> Unicode data compiler --> versioned portable tables
                                                                    |
UTF-8 text ------------------------------------------------> Level G Unicode algorithms
```

The retained interchange representation may be one or more versioned
`.binary` constants using explicit-width little-endian fields and shared
mapping pools. The execution representation remains an empirical question and
may instead be host-native packed data generated or materialized from the same
logical source. Candidate logical data shapes include:

- disjoint range tables for Boolean/enumerated properties;
- codepoint-to-offset/length records for decomposition and case mappings;
- a shared codepoint-sequence pool for one-to-many mappings;
- canonical combining-class data;
- composition-pair indexes plus algorithmic Hangul handling; and
- normalization quick-check values where measurement proves them worthwhile.

The exact layout is deliberately not fixed here. Alternatives such as range
search, two-stage page tables, tries, perfect hashes, or hybrid ASCII/BMP/full
scalar tables should be generated from the same logical data. Each promising
shape should then be compared in portable fixed-width, host-native packed, and
where useful load/convert-once forms for size, lookup cost, materialization
cost, compilation cost, architecture scope, and both-VM behavior.

## Recommended First Vertical Slice

The first implementation experiment should recover one of the previously
successful Unicode rule translations in re2c, not yet generate Level L source.

1. Select a bounded manual-defined operation with an authoritative conformance
   corpus. NFD is the preferred first normalization target because it exercises
   mappings, ordering, and output while retaining the known historical success
   boundary; a smaller case-fold/compare slice may precede it for scaffolding.
2. Transcribe the manual's IBM-oriented rules into ordinary vendored re2c,
   retaining a rule-by-rule correspondence note and explaining every required
   change.
3. Pin the exact Unicode 17.0.0 inputs and run the complete applicable
   conformance invariants, not only illustrative examples.
4. Retain the re2c source, generated reference implementation, commands, and
   results so this proof cannot be lost again.
5. Extract a capability list from the working proof: rule composition, actions,
   state, mapping expansion, ordering, buffering, diagnostics, and generated
   data shapes.
6. Only then propose the first CREXX-native Level L rule syntax and generated
   packed scanner/transducer shape for approval.

This sequence preserves re2c as a differential oracle while deliberately
leaving the long-term Level L surface free to differ from both re2c and the
manuals' source notation.

## Numbered Work Plan And Gates

### 0. Durable Work Area — complete

- Create local branch/worktree from fresh `origin/develop`.
- Publish `origin/unicode` before implementation begins.
- Keep unrelated `develop` changes outside the worktree.

Gate: local and remote heads resolve to the same retained commit.

### 1. Contract And Input Freeze — complete

- Approve the Level L/Level G boundary and explicit non-goals.
- Pin Unicode 17.0.0 inputs, checksums, provenance, and licensing.
- Define generated-source versus generated-binary retention policy.
- Define a small rule-capability matrix: literals, classes, alternation,
  concatenation, repetition, maximal munch, rule priority, emitted actions,
  mapping expansion, EOF, and diagnostics.
- Defer start conditions, trailing context, captures/tags, streaming refill, and
  arbitrary code actions unless the first use case requires them.

Gate 1: passed on 2026-08-25. The approved defaults are recorded in
`experiments/unicode/README.md`; Unicode 17.0.0 and ICU 78.3 source fixtures,
licenses, provenance, and SHA-256 checksums are retained under
`experiments/unicode/inputs/`. No language syntax changed.

### 2. Unicode-Rule Transcription PoC In re2c — implemented

- Select the first manual-defined compare, case-fold, or NFD rule slice.
- Duplicate the IBM-oriented rules in ordinary re2c and retain a correspondence
  ledger for syntax and semantic adjustments.
- Run the applicable pinned Unicode conformance corpus.
- Retain re2c input, generated reference code, build/run commands, and results.
- Derive the actual Level L lexer/transducer requirements from the passing PoC.

Implementation result on 2026-08-25: the retained re2c source parses ICU 78.3's
Unicode 17.0.0 `gennorm2` notation, implements recursive canonical and
algorithmic Hangul decomposition plus stable canonical ordering, and passes
100,170 NFD corpus relations and 1,094,978 unlisted-scalar identity checks. A
rule correspondence ledger, demonstrated Level L capability list,
byte-reproducible generated C++, negative parser checks, command, and evidence
are retained under `experiments/unicode/poc/`.

Gate 2: accepted by Adrian on 2026-08-25; the executable interpretation remains
the retained differential oracle for subsequent Level L and Unicode work.

### 3. Level L Lexer-Maker Language And Core

- Design a CREXX-native rule syntax from the accepted capability list; do not
  assume compatibility with re2c or the IBM-oriented manual notation.
- Maintain the human-facing
  [Level L syntax](../../books/crexx_language_reference/level_l_syntax.md) and
  the [AI and implementer context](../../ai-context/CREXX_LEVEL_L_SYNTAX.md) as
  the Level L syntax references. They record the approved `symbol: kind`, Rexx
  comment, `options levell`, and static compound-symbol direction and will be
  refined by executable use cases.
- Give the accepted lexer-spec subset a versioned grammar and diagnostics.
- Separate regex/spec parsing, automaton construction, table layout, and source
  emission so each can be tested independently.
- Preserve the passing re2c Unicode PoC as a differential oracle for common
  semantics, not as the required product syntax.
- Emit deterministic source and tables with no timestamps or unstable ordering.

Completed on 2026-08-25:

- the human-facing and AI/implementer Level L syntax references define the
  approved first lexer-spec surface; and
- the bounded [re2c output-adapter experiment](../../../experiments/unicode/level-l-emitter/README.md)
  emits a byte-reproducible TinyExpr scanner as cREXX, takes it through `rxc`,
  `rxas`, and `rxlink`, and matches the hand-written `rxfnsl` scanner on both VM
  families. It required no compiler, RXAS, linker, or VM change.

Accepted implementation direction on 2026-08-25:

- Level L will provide foundational `Token` and AST class families implemented
  only in Level B so the compiler can bootstrap without depending on Level G;
- successful parsing automatically creates a production/token syntax tree;
  typed semantic ASTs, visitors, rewrites, and recovery can be layered on later;
- the first frontend is intentionally hand-crafted, naive Level B code whose
  job is only to tokenize the approved Level L subset and construct that AST;
  semantic lowering is a separate later component; and
- committed canonical specifications and deterministic generated Level B
  source will support a clean-build fixed-point self-hosting test without an
  opaque binary seed.

The next implementation slice is that hand-crafted Level B frontend. Its first
retained positive inputs are the documented TinyExpr specification, a Level L
specification for Level L's own lexer, and a Level L specification for the ICU
`gennorm2`/Unicode rule-file lexer. The latter proves that the same frontend can
support the Unicode work: the generated lexer tokenizes the rule file, while
the later parser and data compiler remain separate.

The first slice is accepted when the Level B frontend parses those three
specifications, produces deterministic token and structural AST dumps with
exact byte spans, reports the required negative diagnostics, and gives the same
results on both VM families. It does not need to construct an automaton, choose
a packed layout, or emit scanner source.

Remaining before this phase is complete: implement the Level L specification
parser, diagnostics, neutral IR, automaton construction, and deterministic
emission from authored Level L rather than re2c input.

Gate 3 remains open: the adapter proves the generated target shape, but the
Level L front end and generator core do not yet exist. Closure requires scanners
generated from authored Level L to agree with the oracle and hand-written
fixtures, with unsupported features failing explicitly.

### 4. Binary-Surface Verdict

- Profile the real generated scanner and first Unicode tables.
- Compare portable fixed-width `<at..type>`, host-native `<packed..int>` (and
  float only where a real workload needs it), and load/convert-once layouts.
- Separate committed/source portability from runtime representation: determine
  whether tables must compile unchanged across architectures or may be
  generated/materialized for the target host.
- Inspect generated RXAS for table loads, bounds checks, copies, offset
  recomputation, dispatch, and buffer growth.
- Test alternative portable layouts before changing the VM/compiler.
- If a gap remains decisive, propose the smallest change with a reproducer,
  before/after evidence, semantics, diagnostics, portability analysis, and
  focused tests.

Gate 4: Adrian approves or rejects each compiler/RXAS/VM/language change. No
architecture or syntax change is bundled implicitly into Unicode work.

### 5. Unicode Data Compiler And Property Kernel

- Parse and validate the pinned UCD files.
- Emit versioned portable tables plus a manifest of counts, ranges, mappings,
  sizes, and checksums.
- Implement scalar decoding/encoding and property/range lookup on the Level B
  foundation for later Level G use.
- Cross-check generated logical mappings against independent fixtures or a
  trusted reference implementation without making that implementation the
  product dependency.

Gate 5: deterministic regeneration and complete property/table audit pass on
both VM families.

### 6. NFD First

- Implement recursive canonical decomposition, algorithmic Hangul
  decomposition, and stable canonical combining-class ordering.
- Prove empty/ASCII/Latin/combining/non-BMP/Hangul behavior.
- Run the complete applicable NFD invariants from `NormalizationTest.txt`.

Gate 6: complete Unicode 17.0.0 NFD conformance, idempotence, both VMs, and no
implicit changes to existing Level B operations.

### 7. NFC Composition

- Add composition-pair lookup, starter/blocking state, composition exclusions,
  and algorithmic Hangul composition.
- Exercise long combining sequences and bounded-memory behavior.
- Run all NFC and cross-form invariants in `NormalizationTest.txt`.

Gate 7: complete Unicode 17.0.0 NFC conformance and an accepted performance and
memory verdict.

### 8. Compatibility Forms

- Add compatibility decomposition tags/mappings.
- Reuse the proved ordering and composition phases for NFKD and NFKC.
- Document the semantic loss inherent in compatibility normalization.

Gate 8: complete Unicode 17.0.0 NFKD/NFKC conformance.

### 9. Full Case Folding And Explicit Comparison

- Implement simple and full default folding from `CaseFolding.txt`.
- Treat Turkic folding and full locale/context case conversion as explicit
  separately approved surfaces.
- Define comparison operations only after their normalization/folding order,
  allocation behavior, and result semantics are approved.
- Verify multi-codepoint expansions and normalization interactions.

Gate 9: pinned mapping audit, accepted public API, both-VM conformance, and no
claim of locale collation.

### 10. Segmentation And Additional Properties

- Implement grapheme boundaries first, then word/sentence boundaries only if
  required by the approved Level G slice.
- Use the matching UAX #29 conformance files and keep tailored locale behavior
  outside the default algorithm unless explicitly designed.

Gate 10: complete conformance for each claimed segmentation service.

### 11. Parser-Maker Track

- Start only after the lexer/token representation and generated-table surface
  are stable.
- Retain the hand-crafted Level B frontend as the bootstrap parser and
  differential source-AST oracle.
- Define a small Level L grammar IR and implement the table-construction
  algorithms in Level B/Level L; Lemon may remain an additional differential
  oracle but is not the required product backend.
- Treat automatic production/token AST construction as foundational parser
  behaviour.
- Keep physical AST layout, typed semantic actions and trees, visitors/rewrites,
  parser recovery, ambiguity/conflict reporting, incremental parsing, and
  editor parser-mode needs as explicit later features rather than assuming one
  generated runtime solves them all.

Gate 11: approved parser model and a generated tiny grammar matching a retained
reference parser.

### 12. Integration And Publication Decision

- Rebase/merge current `develop` deliberately and resolve changed contracts.
- Run focused and broad Debug tests, both VM variants, maintained sanitizer
  gates, clean Release builds, install/package proof, and supported-platform
  qualification proportional to the final change.
- Audit docs, generated artifacts, licenses, reproducibility, and version pins.
- Publish or merge only the scopes whose contracts and conformance evidence are
  complete; retain unfinished research on `unicode` without inflating it into a
  product claim.

Gate 12: explicit integration and publication approval.

## Evidence To Retain From The First Commit Onward

- exact source and remote base commit;
- every Unicode input URL, version, checksum, and license notice;
- lexer/parser specifications and generator version;
- deterministic regeneration command and clean-tree result;
- generated-artifact manifest and size/count summaries;
- focused tests and full conformance logs;
- differential-oracle commands and version;
- both-VM, optimized/non-optimized, platform, sanitizer, and package boundaries;
- benchmark corpus, build mode, host details, and retained baseline; and
- a decision ledger for each proposed CREXX binary/language/compiler change.

## Phase 1 Decisions Accepted

On 2026-08-25 Adrian approved Phase 1 and Phase 2 using the recommended
defaults:

1. `unicode` is the durable remote research branch.
2. The first executable proof uses ordinary vendored re2c and complete
   applicable conformance data.
3. Unicode 17.0.0 is the pinned first data set.
4. Downloaded source fixtures are retained in the repository with provenance,
   licensing, and SHA-256 verification.
5. Accepted generated reference source is committed and must also pass a
   reproducibility check; transient build products are not committed.
6. The eventual Level L surface is CREXX-native and re2c-inspired, with no
   syntax compatibility promise to re2c or ICU `gennorm2`.

Research identified ICU `gennorm2` and its `nfc.txt` data-file notation as the
specific IBM-origin rule surface for the first transcription. ICU 78.3's file
is explicitly generated for Unicode 17.0.0 and represents combining classes,
two-way canonical mappings, and one-way canonical mappings compactly. The Phase
2 proof therefore targets the NFD half of those rules; NFC composition remains
outside this gate.

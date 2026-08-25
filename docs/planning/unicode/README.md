# Level L Language Tooling And Level G Unicode Research Track

Status: research charter and work plan; implementation has not started.

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
- re2c 4.5.1 supports user-defined language syntax files. A backend can be
  trialled with `--lang none --syntax FILE`, and the loop/switch code model is a
  plausible match for Level L's `DO`/`SELECT` control flow. This should be tested
  before attempting a re2c port or a new regex/DFA implementation.

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

The portable table surface is the byte-addressed `<at..type>` family. Fixed
integer and float fields have declared widths and canonical little-endian
encoding. This is the right default for generated Unicode data that must be
portable in source, RXBIN, packages, and across hosts.

The newer `<packed..int>` and `<packed..float>` forms expose host-native VM
representations. They are useful for host-local numeric kernels, but are the
wrong storage contract for portable Unicode tables: they have host width/endian
meaning and no stored item type.

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

The likely table representation is one or more versioned `.binary` constants
using explicit-width little-endian fields and shared mapping pools. Candidate
data shapes include:

- disjoint range tables for Boolean/enumerated properties;
- codepoint-to-offset/length records for decomposition and case mappings;
- a shared codepoint-sequence pool for one-to-many mappings;
- canonical combining-class data;
- composition-pair indexes plus algorithmic Hangul handling; and
- normalization quick-check values where measurement proves them worthwhile.

The exact layout is deliberately not fixed here. Alternatives such as range
search, two-stage page tables, tries, perfect hashes, or hybrid ASCII/BMP/full
scalar tables should be generated from the same logical data and compared for
size, lookup cost, compilation cost, and both-VM behavior.

## Recommended First Vertical Slice

The first implementation experiment should be a generated Level L lexer, not a
Unicode library function and not a new parser generator.

1. Create a small re2c syntax backend for Level L using `--lang none`, the
   loop/switch code model, and only the features needed by a bounded lexer.
2. Generate a scanner for the ASCII machine-readable syntax used by selected
   UCD files: hexadecimal scalar/range, semicolon-delimited fields, comments,
   whitespace, and line endings.
3. Compare its token stream with a C scanner generated by the unmodified
   vendored re2c from the same rules.
4. Compare ordinary generated arrays/selects with the existing `tinyexpr`
   packed-binary shape.
5. Record where a syntax file is sufficient and where packed constants require
   a narrow re2c renderer extension, a deterministic table-packing pass, or a
   new Level L emitter.
6. Run the generated scanner through `rxc`, `rxas`, `rxlink`, and both VM
   families, in optimized and non-optimized forms.

This slice gives immediate value to the Unicode data compiler while preserving
re2c as a differential oracle. It does not commit the project to porting all of
re2c or to implementing a regex engine from scratch.

## Numbered Work Plan And Gates

### 0. Durable Work Area — complete

- Create local branch/worktree from fresh `origin/develop`.
- Publish `origin/unicode` before implementation begins.
- Keep unrelated `develop` changes outside the worktree.

Gate: local and remote heads resolve to the same retained commit.

### 1. Contract And Input Freeze

- Approve the Level L/Level G boundary and explicit non-goals.
- Pin Unicode 17.0.0 inputs, checksums, provenance, and licensing.
- Define generated-source versus generated-binary retention policy.
- Define a small lexer-spec feature matrix: literals, classes, alternation,
  concatenation, repetition, maximal munch, rule priority, EOF, and diagnostics.
- Defer start conditions, trailing context, captures/tags, streaming refill, and
  arbitrary code actions unless the first use case requires them.

Gate 1: approved contracts and fixture manifest; no language syntax change.

### 2. re2c-to-Level-L Backend Experiment

- Build the minimal custom syntax file and generated UCD scanner.
- Retain the input rules, commands, generated output, and differential fixtures.
- Determine whether the generic backend can express safe CREXX source without a
  re2c core fork.
- Measure source size, compile time, RXBIN size, scan throughput, and peak table
  footprint for ordinary and packed shapes.

Gate 2: choose one of three paths with evidence: syntax-only backend, narrow
renderer/table-packer change, or native Level L lexer-generator implementation.

### 3. Level L Lexer-Maker Core

- Give the accepted lexer-spec subset a versioned grammar and diagnostics.
- Separate regex/spec parsing, automaton construction, table layout, and source
  emission so each can be tested independently.
- Preserve re2c as a differential oracle for common supported features.
- Emit deterministic source and tables with no timestamps or unstable ordering.

Gate 3: generated scanners agree with the oracle and hand-written fixtures;
unsupported features fail explicitly.

### 4. Binary-Surface Verdict

- Profile the real generated scanner and first Unicode tables.
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
- First test whether Lemon's grammar analysis plus a Level L `-T` parser driver
  template can emit the desired packed table/runtime shape.
- If that boundary is too C-specific, specify a small grammar IR and implement a
  Level L table generator independently.
- Keep parser recovery, semantic actions, AST allocation, ambiguity/conflict
  reporting, incremental parsing, and editor parser-mode needs as explicit
  features rather than assuming one generated runtime solves them all.

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

## Immediate Decisions Requested Before Phase 1 Implementation

1. Confirm that `unicode` is the intended durable remote branch name.
2. Confirm the recommended re2c custom-backend/UCD-scanner vertical slice.
3. Confirm Unicode 17.0.0 as the first pinned data set.
4. Decide whether downloaded UCD source files are retained in the repository or
   fetched into a checksum-verified build cache while manifests and generated
   product tables are retained.
5. Decide whether generated Level L `.crexx` is committed, regenerated in the
   build, or both (committed output plus a reproducibility check).

No implementation beyond branch establishment and this research plan should
begin until these decisions are accepted.

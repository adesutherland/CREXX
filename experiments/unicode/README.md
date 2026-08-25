# Unicode And Language-Tooling Experiment

Status: Phase 1 contract/input freeze complete; Phase 2 NFD reference proof
implemented and awaiting Gate 2 acceptance.

This directory is the retained experimental work area for the `unicode`
branch. It is deliberately outside the product libraries and is not installed
or enabled by the ordinary CREXX build.

## Frozen Boundary

- Level L is the eventual authoring surface for language-engineering tools.
  Its first proposed product is a lexer maker; a parser maker is later work.
- Level G owns explicit Unicode normalization, case folding, comparison,
  properties, collation, and segmentation services.
- Level B remains the valid-UTF-8/codepoint substrate. This experiment does not
  change identifier spelling/equality, keyword matching, source normalization,
  or the existing Level B and Level C string contracts.
- No Level L syntax, compiler/RXAS/VM operation, binary layout, or public Level
  G API is approved by this proof. Any such decision remains a later explicit
  design gate.

## Frozen Inputs

The retained source fixtures are Unicode 17.0.0 and ICU 78.3's Unicode 17.0.0
`gennorm2` normalization rules. See [inputs/SOURCES.md](inputs/SOURCES.md) for
provenance/licensing and [inputs/SHA256SUMS](inputs/SHA256SUMS) for the immutable
fixture hashes.

Retaining the source fixtures in Git is intentional. A build or test must not
quietly substitute a newer network copy. A future Unicode upgrade is a reviewed
fixture replacement with a new version directory, hashes, regenerated outputs,
and conformance evidence.

## Generated-Artifact Policy

- Authoritative upstream inputs, licenses, manifests, handwritten `.re`
  sources, correspondence ledgers, and compact conformance summaries are
  committed.
- Generated C/C++ is retained when it is part of the accepted reference oracle
  or is needed to prevent the proof being lost. It must be byte-for-byte
  reproducible from the committed source and vendored generator.
- Compiler executables, object files, temporary build trees, and full routine
  logs are not committed. Evidence records the command, tool versions, input
  hashes, result counts, and summary; a failing log is retained separately when
  it is needed for diagnosis.
- Generated portable Unicode binaries will be committed only after their format
  is versioned and their role at the Level G boundary is approved. Phase 1 and
  Phase 2 do not choose that format.

Runtime layout is deliberately still open. `<at..type>` supplies the portable
fixed-width baseline, while host-native `<packed..int>` is expected to be
faster and may be the better execution form. After the PoC, the same logical
tables must be measured as portable fixed-width data, target-generated packed
data, and a portable-on-disk/packed-in-memory hybrid. The verdict must state
whether source or binaries need to cross architectures unchanged and include
generation/materialization cost as well as steady-state lookup speed.

## Phase 1 Rule-Capability Contract

The first CREXX-native lexer/transducer design must be derived from working
proofs. The initial required capability envelope is:

| Capability | Phase 1 disposition | First evidence source |
| --- | --- | --- |
| literals and code-point ranges | required | ICU rule operators and UCD fields |
| named classes/definitions | required | Unicode properties and reusable grammar fields |
| alternation and concatenation | required | rule/input grammar |
| bounded/unbounded repetition | required | whitespace, comments, fields, sequences |
| maximal munch | required | deterministic tokenization |
| same-length rule priority | required | explicit deterministic tie-breaking |
| emitted token/action kind | required | rule operators and parsed records |
| mapping expansion | required | one-to-many normalization/case-fold mappings |
| EOF and malformed-input rules | required | complete fixture validation |
| source position and diagnostics | required | fail-closed version/format errors |
| deterministic generated output | required | retained oracle regeneration |
| arbitrary embedded host code | deferred | conflicts with a portable Level L surface |
| start conditions | deferred | not required by the first normalization proof |
| trailing context | deferred | not required by the first normalization proof |
| captures/tags | deferred | reconsider only with a concrete consumer |
| streaming refill protocol | deferred | first proof uses pinned complete files |

“Deferred” is not “unsupported forever”; it means the first design must not pay
for or expose the feature without evidence from an approved use case.

## Phase 2 Target

The selected first vertical slice is canonical decomposition (NFD). ICU's
`gennorm2` notation is the recovered IBM-origin notation remembered from the
earlier proof. ICU 78.3's `nfc.txt` supplies:

- non-zero canonical combining classes (`0300..0314:230`);
- two-way canonical mappings (`00E1=0061 0301`); and
- one-way canonical mappings (`0340>0300`).

The re2c proof parses that exact notation, applies recursive canonical and
algorithmic Hangul decomposition, performs stable canonical ordering, and
checks all applicable NFD invariants in Unicode 17.0.0
`NormalizationTest.txt`. It deliberately does not implement NFC composition.

See [poc/RULE-CORRESPONDENCE.md](poc/RULE-CORRESPONDENCE.md) for the detailed
translation ledger and `poc/README.md` for reproducible commands.

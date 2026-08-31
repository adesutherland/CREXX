# Level G Unicode Product Surface and Roadmap

Status: the approved Unicode 17.0.0 product baseline is implemented. Release
status remains governed by the repository release notes and tags.

Date: 2026-08-30

## Purpose

This document records the selected public boundary, ownership rules, execution
architecture, TUTOR alignment, rejected alternatives, and follow-on roadmap for
cREXX Unicode services. Public usage belongs in the [Unicode text services
chapter](../../books/crexx_library_reference/unicode.md); implementation detail
belongs in the [Unicode algorithm
appendix](../../books/crexx_vm_spec/unicode_algorithms.md) and
[`CREXX_UNICODE.md`](../../ai-context/CREXX_UNICODE.md).

The baseline is an explicit Level G service. It does not redefine `.string`,
comparison, assignment, identifiers, source parsing, ordinary BIF indexes, or
text I/O.

## Product boundary

- Level B `.string` remains valid UTF-8 with codepoint-based operations.
- Level B `.binary` remains arbitrary bytes.
- Level G owns explicit normalization, full default case mapping, folding,
  segmentation, and byte/text codecs.
- Ordinary equality and hashing remain exact; no operation normalizes or folds
  implicitly.
- Versioned public Unicode algorithms advance together and are reported by
  `rxunicode..version()`.
- Generated tables and private numeric selectors are not public API.
- A service may use immutable task-local working state internally, but only an
  object with a distinct user-visible capability is public.

## Approved baseline surface

The namespace query is:

```text
version() = .string
```

Normalization is eight direct procedures:

```text
toNFD(text = .string) = .string
toNFC(text = .string) = .string
toNFKD(text = .string) = .string
toNFKC(text = .string) = .string
isNFD(text = .string) = .boolean
isNFC(text = .string) = .boolean
isNFKD(text = .string) = .boolean
isNFKC(text = .string) = .boolean
```

Default casing and folding are:

```text
toUppercase(text = .string) = .string
toLowercase(text = .string) = .string
toCasefold(text = .string) = .string
toSimpleCasefold(text = .string) = .string
toTurkicCasefold(text = .string) = .string
toTurkicSimpleCasefold(text = .string) = .string
```

There is no reusable case-folder or normalizer object because neither retains
useful state between independent calls. Titlecase, simple upper/lower, and
locale-tailored case conversion are not in the baseline.

Default extended grapheme operations are:

```text
graphemeCount(text) = .int
graphemeSubstr(text, start[, length[, pad]]) = .string
graphemePos(needle, haystack[, start]) = .int
graphemeReverse(text) = .string
```

`.graphemes(text)` is an immutable indexed snapshot for repeated random access,
search, reversal, and iteration. Direct count and bounded substring remain
streaming.

The whole-value byte/text boundary is:

```text
encode(text = .string[, encoding = "UTF-8"[, replacement = .binary]]) = .binary
decode(data = .binary[, encoding = "UTF-8"[, replacement = .string]]) = .string
isDecodable(data = .binary[, encoding = "UTF-8"]) = .boolean
isEncodingSupported(encoding = .string) = .boolean
```

Supported families are UTF-8, explicit-endian UTF-16/UTF-32, US-ASCII,
ISO-8859-1, Windows-1252, IBM437, IBM850, and IBM1047. Strict conversion is the
default. An explicitly supplied, non-empty typed third argument opts into
replacement. Codecs neither add nor consume a BOM as metadata.

Level B `readbinary(path)` and `writebinary(path, data)` provide complete-file
byte I/O so applications can compose file transport with codecs immediately.
They do not imply an incremental stream codec.

## Execution architecture

Normalization, default case mapping, folding, and grapheme segmentation use
the RXVM `.string` codepoint path:

- obtain the scalar count with `STRLEN`;
- read sequential scalars with `STRCHAR`;
- classify from one compiler-owned immutable prepared constant;
- retain only the algorithm's bounded semantic state; and
- emit through `APPENDCHAR` or record codepoint boundaries.

This makes RXVM the single authority for decoding a valid `.string`. It avoids
an input `.binary` copy, a second re2c UTF-8 decoder, and a complete UTF-32
materialization.

Normalization prepares recursive decomposition, canonical combining classes,
composition data, and Quick_Check. NFD/NFKD stably merge combining marks across
source-scalar boundaries. NFC/NFKC compose the ordered stream directly,
including Hangul and blocking. Predicates use exact checks; NFC/NFKC may decide
with Quick_Check and fall back exactly on the first `Maybe`.

Transform calls do not run an unconditional predicate first. A positive
matching normalization certificate may return immediately; otherwise the
transform performs one pass. This avoids making changed large buffers pay for
two complete scans.

Default uppercase/lowercase uses full mappings; lowercase implements the
Unicode `Final_Sigma` context using prepared `Cased` and `Case_Ignorable`
properties. Case folding remains a scalar-to-sequence concatenation for its
selected default/simple/Turkic mode. Grapheme segmentation applies UAX #29
revision 47 GB3 through GB999.

UTF codecs are algorithmic. ASCII and ISO-8859-1 use range mapping. Each
table-backed single-byte encoding has a dense 256-entry decoder and a
page-bounded sorted reverse index. The exact target-byte replacement contract
avoids pretending that `?` has the same byte in ASCII and EBCDIC.

## Constant and cache ownership

Every prepared data image is a module-scoped source binary constant. `rxc`
shares one source payload and emits one named RXAS constant; `rxas` and
`rxlink` keep one constant-pool value. Runtime table reads name that value
directly. No object or call copies the table.

Normalization certificates describe `.string` content. They are neither VM
UTF-8 bookkeeping nor class-private application flags. The selected
`UNICODE-CERT-01` layout uses four independent positive facts in a protected
language-owned flag sub-band:

- known NFD;
- known NFC;
- known NFKD; and
- known NFKC.

Absence means unknown, never false. Exact whole-value copies preserve the
facts; content mutation clears them; known ASCII and empty production may set
all four. The VM participates in invalidation because it owns mutation, but
applications cannot inspect or set certificates. Algorithms remain correct
without cache state. The detailed contract is
`performance/UNICODE-CERT-01-WORKLIST.md`.

Numeric conversion caching remains separate under
`performance/VALUE-CACHE-01-WORKLIST.md`. It requires numeric-context
provenance and must not borrow Unicode certificate bits or semantics.

## Prepared data boundary

Pinned UCD inputs, sources, terms, and checksums are under
`lib/rxfnsg/unicode/data/unicode-17.0.0/`. TUTOR-compatible legacy mapping
inputs and their provenance are under `lib/rxfnsg/unicode/data/legacy/`.

The deterministic build emits these checked images:

| Family | Image size | Principal audit |
| --- | ---: | --- |
| normalization | 6,736,284 bytes | 2,081 NFD and 5,914 NFKD mappings, 55 CCC ranks |
| default case mapping | 4,590,140 bytes | 3,037 mapping records, 4,632 cased scalars |
| case folding | 4,501,380 bytes | 1,585 records, 1,707 components |
| grapheme properties | 1,114,176 bytes | complete codepoint property byte plus header |
| legacy encodings | 14,536 bytes | four encodings and 1,024 source mappings |

Generation fails closed on an unexpected version, checksum, record count,
scalar, duplicate, or layout. Private runtime initializers repeat exact magic,
version, size, count, and bounds checks.

## TUTOR alignment

TUTOR is used as a catalogue of Unicode problems and Rexx-friendly names, not
as a replacement for cREXX's type system.

Selected common vocabulary includes `toNFD`, `toNFC`, compatibility
normalization names, `is*` predicates, `toCasefold`, `encode`, `decode`, and
familiar encoding aliases. The legacy table compiler accepts TUTOR Format A
mapping inputs; cREXX then emits its own versioned immutable image.

The deliberate differences are:

- `.binary` and valid UTF-8 `.string` remain the fundamental byte/text types;
- grapheme semantics are explicit procedures and a view, not a coercible string
  kind;
- normalization is never a property silently imposed on ordinary text;
- strict conversion is default and replacement is typed and explicit;
- no Y/P/G/T/U literal family or mutable global coercion policy is added; and
- encoded streams wait for a typed incremental I/O design.

This preserves useful common source vocabulary only where Unicode version,
error policy, endian policy, and indexing unit also align.

## Selected performance findings

The retained measurements select architecture; they are not portable throughput
promises.

The generated NFD byte-DFA expanded to 3,440 states, 6,455 method locals, and
172,334 optimized RXAS lines. It measured 84.0x to 101.1x slower on short-call
rows and 8.8x to 9.0x slower on the retained 101,506-byte buffer. Returning to
VM codepoints and prepared scalar descriptors removed that order-of-magnitude
loss and restored approximate canonical parity or better large-buffer results.

Across the common normalization family, final directional ratios ranged from
0.940x to 1.103x. Quick_Check greatly improved mixed predicates but a large
known-normalized `Maybe` buffer could pay for a full preliminary scan plus exact
fallback. The selected split is therefore Quick_Check for predicates,
certificates for known transform hits, and no unconditional transform precheck.

The informal certificate screen found unchanged-large-buffer transform hits
26.1x to 133.8x faster and predicate hits 15.0x to 166.1x faster, with no
significant cold-path regression detected in that screen.

For direct grapheme count and bounded substring on the retained 49,152-byte
buffer, building a complete per-call index was 1.802% to 2.953% slower. Direct
operations therefore stream, while the explicitly requested view retains its
index.

Raw evidence and host limitations are retained under
`performance/evidence/2026-08-28-unicode-normalization-decisions/` and
`performance/evidence/2026-08-29-unicode-grapheme-first-release-verdict/`.

## Rejected baseline shapes

- Implicit normalization, normalized equality, or normalized assignment.
- `.binary` normalization overloads or implicit binary decoding.
- Public numeric form selectors or stringly `normalize(text, "NFC")` dispatch.
- A public reusable normalizer with no incremental user capability.
- Per-object or per-call copies of multi-megabyte prepared images.
- A generated re2c decoder for algorithms over already-valid `.string`.
- A complete UTF-32 copy before Unicode work begins.
- An unconditional precheck before every transform.
- Per-call full grapheme indexing for count or bounded substring.
- Hidden mutable/shared grapheme caches on ordinary strings.
- Ambiguous `UTF-16`/`UTF-32` names or implicit BOM policy.
- A string-valued replacement policy that obscures target bytes.
- Titlecase and locale casing without a concrete consumer and locale contract.

## Qualification contract

The baseline must retain:

- deterministic generation and checksum failure tests;
- complete Unicode 17 normalization conformance and cross-form relations;
- exact normalization predicate tests, including Quick_Check `Maybe`;
- every prepared default case mapping plus every unlisted scalar identity;
- every case-fold record in four modes plus unlisted identity;
- every retained `GraphemeBreakTest.txt` record and focused public behaviour;
- every codec family, alias, strict error, replacement path, and table-backed
  all-byte round trip;
- complete-file binary I/O including embedded NUL and invalid UTF-8;
- optimized and no-opt builds under `rxbvm` and `rxtvm`; and
- RXAS audits for shared constants, direct codepoint/table access, and absence
  of input/table copies.

Compiler constant lowering and optional imported `.binary` defaults have
focused permanent regressions because the Unicode surface exposed those
toolchain requirements. Library work is expected to validate `rxc`, `rxas`,
`rxlink`, and `rxvm`, not only the Rexx source.

## Follow-on roadmap

The baseline deliberately leaves the following as separate work:

1. **Incremental codecs and encoded stream adapters.** Design a state object
   only with explicit chunk remainder, finalization, strict/replacement policy,
   BOM ownership, bounded output, cancellation, task transfer, and EOF rules.
   Compose it with the future general `rxio.input`, `rxio.output`, and
   `rxio.stream` interfaces rather than overloading Classic variable-width
   positioning.
2. **Typed properties and names.** Begin with General Category, White Space,
   CCC, case properties, grapheme-break property, codepoint names, and aliases.
   Prefer typed accessors over one heterogeneous string-dispatched lookup.
3. **Caseless-normalized profiles.** Specify canonical caseless comparison,
   `NFKC_Casefold`, and matching key/hash functions explicitly. Do not imply
   them from `toCasefold` alone.
4. **Security and identifiers.** Keep UAX #31 policy and confusable handling
   separate from general text conversion.
5. **Further segmentation.** Add word or sentence boundaries only for a real
   consumer; preserve the explicit unit in every name.
6. **Locale services and collation.** Treat locale casing, tailoring, display
   width, and Unicode/CLDR collation as larger optional contracts, potentially
   with a qualified provider.
7. **Data-image convergence.** Combine prepared sections only if measurement
   justifies it and shared-constant, update, and conformance boundaries remain
   independently auditable.
8. **Optional TUTOR compatibility facade.** Consider a thin portability layer
   only for operations whose types, Unicode version, endian behavior, indexing
   unit, and failure policy are identical to the typed surface. It must
   delegate to `rxunicode`, must not reintroduce implicit normalization or
   dynamic string kinds, and must not become a second semantic authority.

Any performance edit to these paths follows `performance/AGENTS.md`: freeze
after focused correctness, run the smallest decisive profiling-off Release
comparison against retained evidence, report it, and stop for direction before
broad closeout.

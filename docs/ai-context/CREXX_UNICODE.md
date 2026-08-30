# cREXX Unicode implementation context

Use this note when changing `rxunicode`, its generated data, compiler constant
lowering, or Unicode conformance tests. The language boundary is documented in
`docs/books/crexx_language_reference/unicode.md`, the public API in
`docs/books/crexx_library_reference/unicode.md`, and the human-readable and
mathematical algorithms in `docs/books/crexx_vm_spec/unicode_algorithms.md`.
Decisions and future work remain in
`docs/planning/unicode/PRODUCT-SURFACE-AND-ROADMAP.md`.

## Non-negotiable boundary

- `.string` is valid UTF-8 text and ordinary Level B positions count
  codepoints.
- `.binary` is arbitrary bytes. Only an explicit codec assigns an encoding.
- Normalization, full case mapping, folding, and grapheme segmentation are
  explicit Level G services; none changes equality, assignment, or ordinary
  BIF semantics.
- Public Level G source contains no assembler. It delegates VM-adjacent work to
  the private Level B `_rxunicode` namespace.
- Generated tables, numeric form selectors, build parsers, and executor
  procedures are not public APIs.
- All versioned Unicode algorithms report Unicode 17.0.0 and advance together.

## Product source map

Runtime surface and templates:

- `lib/rxfnsg/rexx/unicode.crexx`: pure Level G facade, `.graphemes`, and
  `.graphemeiterator`.
- `lib/rxfnsg/unicode/tools/unicode_d.crexx`: private normalization executor
  source adapted by the product generator into the build-tree module.
- `lib/rxfnsg/rexx/unicode_case_mapping.crexx.in`: private full default
  upper/lower executor template.
- `lib/rxfnsg/rexx/unicode_casefold.crexx.in`: private case-fold executor
  template.
- `lib/rxfnsg/rexx/unicode_grapheme.crexx.in`: private grapheme executor
  template.
- `lib/rxfnsg/rexx/unicode_codec.crexx.in`: private strict/replacement codec
  template.

Build-time table compilers and source generators live in
`lib/rxfnsg/unicode/tools/`. Their paired entry points are:

- normalization: `unicode_data`, `unicode_normprops`, `unicode_gennorm2`,
  `level_l_gennorm2`, and `generate_normalization`;
- default case mapping: `case_mapping_table` and `generate_case_mapping`;
- case folding: `casefold_table` and `generate_casefold`;
- graphemes: `grapheme_table` and `generate_grapheme`; and
- legacy encodings: `encoding_table` and `generate_encoding`.

Pinned inputs are under:

- `lib/rxfnsg/unicode/data/unicode-17.0.0/` for UCD normalization, casing,
  folding, and segmentation data; and
- `lib/rxfnsg/unicode/data/legacy/` for TUTOR-compatible single-byte mappings.

Every data directory carries `SOURCES.md` and `SHA256SUMS`. Generated runtime
`.crexx` files are build-tree artifacts. Never edit or commit them.

The normalization generator uses an explicit field-and-method allowlist when
adapting `unicode_d.crexx`. The wider build-tool class is not copied wholesale:
binary/lexer entry points and their scratch state are intentionally absent from
the product runtime. Keep the allowlist narrow and extend it only when a public
codepoint-path requirement proves that another member is needed.

Product conformance is concentrated in:

- `ts_unicode_normalization.crexx`;
- `ts_unicode_case_mapping.crexx`;
- `ts_unicode_casefold.crexx`;
- `ts_unicode_grapheme.crexx`; and
- `ts_unicode_codec.crexx`.

`ts_fileio_binary.crexx` covers the Level B whole-file byte boundary. Compiler
tests `optional_binary_import_*` protect imported optional `.binary` defaults
used by the public codec signature.

## Common execution and constant ownership

Normalization, case mapping, folding, and grapheme algorithms consume a
`.string` through RXVM codepoint operations:

1. `STRLEN` obtains the scalar count.
2. Sequential `STRCHAR` reads one scalar and lets RXVM decode UTF-8 using its
   advancing string cache.
3. Direct `BGETU8`, `BGETU16`, or `BGETU32` reads classify the scalar from one
   module constant.
4. The executor updates bounded semantic state.
5. `APPENDCHAR` emits result scalars, or the grapheme engine records codepoint
   boundaries.

Do not reintroduce a re2c UTF-8 decoder, convert the complete input through
`.binary`, materialize UTF-32, or store the table on each object. Those shapes
duplicate the VM's text authority or copy large values before useful work.

Each generated image is one module-scoped binary constant. `rxc` shares the
source payload and emits one named RXAS constant; `rxas`/`rxlink` retain one
constant-pool item. Direct table reads continue to name that constant. Do not
replace it with a mutable global register or runtime table copy.

## Normalization

The public forms are `toNFD`, `toNFC`, `toNFKD`, and `toNFKC`, with matching
`is*` predicates. The one 6,736,284-byte image contains canonical combining
classes, recursive prepared NFD/NFKD mappings, composition data, and official
Quick_Check properties. Audited source counts include 2,081 NFD mappings,
5,914 NFKD mappings, and 55 positive CCC ranks.

NFD/NFKD stream prepared recursive decomposition and stably merge combining
marks across source-scalar boundaries. NFC/NFKC feed the same ordered stream
directly to canonical composition, including arithmetic Hangul and composition
blocking. They do not allocate a complete decomposed intermediate.

NFD/NFKD predicates check exact decomposition and ordering without producing a
result. NFC/NFKC use Quick_Check: `No` rejects, ordered all-`Yes` accepts, and
the first `Maybe` transfers to the exact allocation-free checker. Do not add an
unconditional predicate pass before a transform; changed large inputs then pay
for two scans.

Four VM-carried, protected language-owned positive certificates may mark a
value/register known NFD, NFC, NFKD, or NFKC. They are stored in the status
word's language band, not its VM-private low byte. Absence means unknown. Exact
whole-value copies preserve them, content mutation clears them, and empty/known
ASCII production may set all four. Certificates are optional performance
evidence; correctness must not depend on their presence. The authoritative
flag and invalidation contract is `performance/UNICODE-CERT-01-WORKLIST.md`.

## Full default case mapping

`toUppercase` and `toLowercase` use one 4,590,140-byte image with 3,037 mapping
records. Each input scalar selects identity or a prepared mapping span. Full
mapping can expand. Lowercase additionally retains the Unicode `Final_Sigma`
context: preceding/following cased characters are tested while case-ignorable
characters are skipped.

The baseline deliberately excludes titlecase and locale-tailored mappings.
Default upper/lower do not normalize, fold, or change Level B `upper`/`lower`.

## Case folding

The 4,501,380-byte case-fold image contains a dense descriptor table, 1,585
fixed records, and 1,707 mapping components. The source audit is 1,481 common,
31 simple, 104 full, and 2 Turkic records; maximum mapping length is three.

Internal modes are:

| Value | Public operation | Records selected |
| --- | --- | --- |
| `1` | simple default | `C + S` |
| `2` | full default | `C + F` |
| `3` | simple Turkic | default simple plus `T` override |
| `4` | full Turkic | default full plus `T` override |

`toCasefold` means mode 2. Folding is scalar-local concatenation and therefore
needs no cross-scalar normalization state. It does not promise canonical
equivalence or source/result index correspondence.

## Grapheme segmentation

The 1,114,176-byte image stores one packed property byte per Unicode codepoint
plus a checked header. The byte combines Grapheme_Cluster_Break,
Extended_Pictographic, and Indic_Conjunct_Break information. The scanner applies
UAX #29 revision 47 GB3 through GB999 and retains previous class, RI parity,
emoji-ZWJ suffix state, and Indic-conjunct suffix state.

Direct count and bounded substring stream without a full boundary vector.
One-off search and reverse derive only the boundaries required by the call.
`.graphemes(text)` explicitly allocates one packed codepoint-boundary index and
reuses it for random access and iteration. Do not add a hidden VM/string
grapheme cache; mutation/ownership costs would be paid by unrelated writers.

## Codecs

UTF-8, explicit-endian UTF-16, and explicit-endian UTF-32 are algorithmic.
US-ASCII and ISO-8859-1 use direct range tests. Windows-1252, IBM437, IBM850,
and IBM1047 share one 14,536-byte `CUEM` layout-version-1 image:

- 64-byte header;
- four 32-byte descriptors;
- one dense 256-entry `u32` decode table per encoding;
- one 257-entry `u16` page-boundary index per encoding; and
- sorted fixed 8-byte reverse records for page-bounded encode lookup.

The build checks exactly four encodings, 1,024 source mappings, and the exact
image length. Runtime initialization repeats the magic, format, size, count,
descriptor, and bounds audit.

Strict conversion is the two-argument authority. The public optional third
argument dispatches to a replacement executor only when it was explicitly
supplied. Decode replaces maximal malformed input subparts with caller text.
Encode replaces each unmappable scalar with exact caller bytes after validating
that the complete replacement is legal in the target encoding. An empty
replacement is invalid.

No codec adds or consumes a BOM. UTF-8 strict decode validates then uses the
runtime `.binary`/`.string` conversion path; it does not duplicate UTF-8
materialization. UTF-8 replacement decoding needs a byte scanner because its
contract is specifically to locate malformed byte subparts; this is separate
from algorithms over an already-valid `.string`.

The single-byte build accepts TUTOR Format A source mappings but emits the
cREXX-owned image above. Windows-1252 undefined positions map to their matching
C1 controls, matching the retained TUTOR mapping policy.

## Failure and signal rules

- Public argument policy errors signal `INVALID_ARGUMENTS`.
- Strict text/byte conversion failures signal `UNICODE_ERROR`.
- Invalid indexed access signals `OUT_OF_RANGE` where documented.
- Corrupt generated data and internal invariant failures signal `FAILURE`.

Private fixed conditions use the source `SIGNAL` instruction and the ordinary
compiler-exit lowering pipeline. Do not restore source-level `assembler signal`
or a forwarding `raise` wrapper.

The Level B `fileio.crexx` bootstrap is a separate dependency boundary: the
base library is built before the certified compiler-exit bundle that imports
it. Its `readbinary`/`writebinary` failures therefore call the existing private
`_rxsysb.raise` transport, as other bootstrap file functions do. Enabling the
source `SIGNAL` keyword there requires a separately designed two-stage library
bootstrap; it must not create a `library -> compiler_exit_bin -> library`
cycle.

## Build and qualification

The normal `rxfnsg` build verifies input hashes, compiles the Level B table
tools, generates source constants with `rxbvm`, compiles the private modules,
and links them with the public facade into `rxfnsg.rxbin`.

Focused product qualification starts with:

```text
cmake --build cmake-build-debug --parallel 10 --target \
  ts_unicode_normalization ts_unicode_case_mapping ts_unicode_casefold \
  ts_unicode_grapheme ts_unicode_codec ts_fileio_binary

ctest --test-dir cmake-build-debug --parallel 4 --output-on-failure \
  -R 'ts_unicode_(normalization|case_mapping|casefold|grapheme|codec)|ts_fileio_binary'
```

Every Unicode functional harness is compiled optimized and no-opt for both
`rxbvm` and `rxtvm`. Normalization consumes the complete Unicode 17
`NormalizationTest.txt`, including all normative cross-form relations and
unlisted-scalar identity. Graphemes consume every retained
`GraphemeBreakTest.txt` record and exercise every public boundary consumer.

Case mapping and folding parse the pinned `UnicodeData.txt`,
`SpecialCasing.txt`, and `CaseFolding.txt` independently of the build-time
table compilers. They compare the product to those raw mappings, exercise the
Final_Sigma context matrix, validate every prepared record, and check identity
for every unlisted scalar. This independent oracle is deliberate: a parser or
status-selection defect in a table compiler must not generate its own expected
answer.

Codec qualification uses exact byte vectors at every UTF width/surrogate
boundary, round-trips all 1,112,064 Unicode scalar values through UTF-8 and
both endian forms of UTF-16/UTF-32, rejects representative malformed classes,
and pins maximal-subpart replacement behavior. ASCII and Latin-1 ranges are
exhaustive. The four legacy mapping sources are parsed independently and all
1,024 byte/codepoint relations are checked in both directions as well as by
`isDecodable`; ordinary all-256-byte round trips remain as a second invariant.

RXAS shape tests supplement semantic tests. They must retain one named constant,
direct codepoint/table access, bounded state, and the appropriate result path;
they reject table attributes, repeated large literals, input copies, and
separate UTF-8 decoding for `.string` algorithms. Shape checks protect the
selected architecture but never replace conformance tests.

After code and focused tests are unchanged, follow repository guidance for the
full Debug, sanitizer, Release, install/package, and hosted gates. Do not repeat
an unchanged expensive suite merely because documentation or history changed.

## Performance guardrails

- Do not add an unconditional normalization precheck. Use a positive
  certificate; predicates alone own Quick_Check.
- Keep direct grapheme count and bounded substring streaming. The explicit view
  alone owns a full index.
- Do not switch from VM codepoints to a generated byte-DFA for valid `.string`
  inputs. Retained measurements found the oversized generated path much slower,
  especially for short calls.
- Do not change image packing, shared-constant ownership, or cache invalidation
  from benchmark intuition. Read the retained evidence and
  `performance/AGENTS.md`, run the required first Release verdict, and stop for
  direction after an approved performance edit.

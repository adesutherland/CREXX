# Shared B Unicode runtime

This component owns the existing Unicode 17 executors and their pinned raw
Unicode/legacy-codec inputs. `rxunicode_core` builds the independently selected
`bin/rxunicode-core.rxbin`; it does not build or import `rxfnsg`. The minimal
`library.rxbin` remains free of the large prepared Unicode images.

The component preserves existing private `_rxunicode` names, signatures,
selectors, algorithms, image layouts and errors. Its public B `unicode_api`
module owns the existing 23 `rxunicode` procedures with their exact ABI and
RexxDoc. G retains its collection/iterator classes and includes those same B
members in its aggregate. Classic adapters remain in `lib/rxfnsc`, compiling
against the standalone core's metadata without changing BIF or stream bodies.

| Owner | Maintained inputs and responsibility |
|---|---|
| `rexx/` | Public procedural API plus B case mapping, folding, codec and grapheme executor templates. |
| `data/unicode-17.0.0/` | Canonical pinned UCD inputs, including independent Level L recognition inputs, provenance and checksums. |
| `data/legacy/` | Canonical TUTOR-compatible mapping sources, provenance and checksums. |
| `tools/unicode/` | Existing table compilers, deterministic source generators, Level L grammar/adapter and the B build controller. |
| `tests/` | Independent private/public B runtime fixtures, compiled metadata and Classic provider/import checks, and RXAS expectations for one constant per family without table/input copies. |
| `lib/rxfnsg/tests_functional/` | Unchanged independent normalization, full case mapping, folding, grapheme and codec conformance fixtures. |

`tools/unicode/unicode_d.crexx` retains its existing dual role as a build-time
table compiler and the source from which `generate_normalization.crexx` copies
an explicit allowlist of runtime fields/methods. The extraction does not copy
the complete tool into the runtime or create another normalization algorithm.
Generated `.crexx`, RXAS, bytecode and prepared images remain build outputs.

## Build and use graph

```text
native B seed and native tools
  -> minimal B library
     -> independent L0/L1/L2 bootstrap and gennorm2 grammar
     -> classlib and certified compiler exits
  -> Unicode table tools and source generation using those prerequisites
  -> five private B executor modules -> public B unicode_api -> rxunicode-core.rxbin
  -> Classic adapters (rxfnsc) and G classes (rxfnsg aggregate)
```

Level L generates its small recognition index directly from canonical pinned
raw data and the minimal B library. It never imports the optional core or G
facade, so canonical source-data ownership does not create a bootstrap cycle.
L tools and compiler exits are build prerequisites of the optional component;
programs using its executors need neither compiler nor generation tools.

`UnicodeBuilder.cmake` bootstraps `tools/unicode/build_unicode.crexx`, which owns
the retained U1–U8 dependency waves and U9 for the public member. The controller
reuses the one existing
compiler-sealed worker source under `tools/build-system/levelg/`. That generic
tooling adapter's legacy path and `options levelg` do not introduce a G facade
or source-language compiler frontend into the Unicode runtime. General
controller helpers currently retain their existing bodies in both controllers;
consolidating build orchestration infrastructure is separate from Unicode
algorithm ownership.

Each runtime member and generator has a content-derived action key. The core
link preserves inline metadata, retains source in Debug and strips source in
other configurations, as before. It publishes through a temporary file and
rename and preserves the published bytes when unchanged. CMake owns one
completion stamp plus all declared outputs. A required successful invocation
refreshes their timestamps once, preventing Make secondary-output rules from
repeating the producer; an unchanged subsequent build changes no artifacts.
The G consumer likewise records successful producer completion when linking
retains identical bytes, so a recovered core does not cause repeated G builds.
Deleting a declared output re-enters the one producer. Runtime
member freshness requires both RXBIN and readable RXAS, so removing a shape
check's RXAS forces its owning member to be rebuilt. Changed raw data re-enters
the pinned configure-time checksum gate before generation.

Build only the core with:

```sh
cmake --build cmake-build-debug --parallel 10 --target rxunicode_core
```

`CREXX_UNICODE_BUILD_JOBS` controls the controller's internal parallel jobs;
CMake parallelism controls prerequisites. Exported `RXUNICODE_CORE_RXBIN`,
`RXUNICODE_CORE_MEMBERS_ROOT`, `RXUNICODE_CORE_GENERATED_ROOT`, table-tool and raw
data variables let G and conformance tests consume the same single producer.
The existing `rxunicode_*_source` target names now depend on this component.

The five existing RXAS shape tests now inspect this component's member outputs.
Full semantic qualification continues through existing G interface tests and
Classic consumer regressions; `tests/ts_unicode_core.crexx` directly consumes all five existing private
algorithm families using only the minimal B library and core. Its opt/no-opt
images link during preparation; runtime-only CTests execute both engines with
autoload disabled. These packaging checks do not make private selectors a
public API. The complete cold,
incremental, installed and cross-consumer obligations remain in the
[compiler plan](../../../research/level-l/compilers/PLAN.md).

Algorithm and error contracts remain in
[Unicode implementation context](../../../docs/ai-context/CREXX_UNICODE.md)
and the [algorithm appendix](../../../docs/books/crexx_vm_spec/unicode_algorithms.md).

## Public B interface

Reviewed against local baseline `ec602e941254a59c0779a314c4f26d803adab3a5`.
Adrian approved this contract after the independent B/L cycle audit on
19 October 2026. The [compiler plan](../../../research/level-l/compilers/PLAN.md#shared-b-unicode-interface-and-c-adapter-migration)
owns acceptance, implementation order and the current qualification boundary.

### Ownership and public names

The existing 23 procedural `rxunicode` definitions and their RexxDoc blocks
live in `lib/rxfnsb/unicode/rexx/unicode_api.crexx`, using `options levelb`.
Keep the public namespace **`rxunicode`**, source spelling `import rxunicode`
and `rxunicode..operation(...)`, and the existing selectable product
**`rxunicode-core.rxbin`**. The internal module name `unicode_api` distinguishes
it from G's retained `unicode` module. Do not introduce another public namespace,
another image, a new forwarding layer, or a second algorithm implementation.

The G module retains `.graphemes` and `.graphemeiterator`, their existing
private `_rxunicode` calls, and their documentation. Its aggregate continues to
export the complete old surface for existing consumers. C retains its existing
BIF/stream adapters and call spelling, but compiles them against exact B-owned
metadata. Neither private numeric selectors nor serialized table/boundary
layouts become public.

### Exact procedural surface

The declarations below reproduce the existing public signatures. Defaults are
metadata defaults, not permission to discard argument-presence information.
Only the eight normalization entries have `arg expose text`; every other entry
retains its existing value parameters.

| Public procedure(s) | Parameter declaration after `arg` | Return type |
|---|---|---|
| `version` | none | `.string` (`"17.0.0"`) |
| `toNFD`, `toNFC`, `toNFKD`, `toNFKC` | `expose text = .string` | `.string` |
| `isNFD`, `isNFC`, `isNFKD`, `isNFKC` | `expose text = .string` | `.boolean` |
| `toUppercase`, `toLowercase` | `text = .string` | `.string` |
| `toCasefold`, `toSimpleCasefold`, `toTurkicCasefold`, `toTurkicSimpleCasefold` | `text = .string` | `.string` |
| `encode` | `text = .string, encoding = "UTF-8", replacement = ""x as .binary` | `.binary` |
| `decode` | `data = .binary, encoding = "UTF-8", replacement = ""` | `.string` |
| `isDecodable` | `data = .binary, encoding = "UTF-8"` | `.boolean` |
| `isEncodingSupported` | `encoding = .string` | `.boolean` |
| `graphemeCount` | `text = .string` | `.int` |
| `graphemeSubstr` | `text = .string, start = .int, length = 0, pad = " "` | `.string` |
| `graphemePos` | `needle = .string, haystack = .string, start = 1` | `.int` |
| `graphemeReverse` | `text = .string` | `.string` |

### Value, presence and failure contracts

- `.string` is valid UTF-8 text; `.binary` is arbitrary exact bytes. Operations
  preserve caller text/bytes. Normalization may retain its existing protected
  positive certificates through exposed arguments. Returned values follow the
  existing value/ownership rules; no physical zero-copy or fresh-allocation
  guarantee is added. Do not turn ordinary arguments into exposed arguments
  as part of this migration.
- Unicode remains 17.0.0. Normalization is explicit; full casing retains
  expansion and `Final_Sigma`; the four named folds retain their current
  default/simple/Turkic meanings. Ordinary equality, symbol casing, word
  classification and numeric behavior are unchanged.
- `encode`/`decode` inspect `?replacement`. Omission means strict conversion;
  explicit empty replacement is invalid even for an otherwise valid input.
  Encode replacement is nonempty target-encoded `.binary`; decode replacement
  is nonempty `.string`. Decode replaces maximal malformed input subparts;
  encode replaces each unmappable scalar. No BOM is inserted or consumed.
- Codec names retain trim/uppercase and removal of `-`/`_`. Supported compact
  spellings are `UTF8`, `UTF16LE`, `UTF16BE`, `UTF32LE`, `UTF32BE`;
  `ASCII`/`USASCII`/`ISO646US`;
  `ISO88591`/`LATIN1`/`ISOLATIN1`/`IBM819`/`CP819`/`819`;
  `WINDOWS1252`/`CP1252`/`1252`; and `IBM`, `CP` or bare numeric spellings for
  `437`, `850` and `1047`. Bare `UTF16`/`UTF32` remain unsupported. Explicitly
  supplying an empty encoding does not select the default.
- `isDecodable` returns false for malformed bytes but signals
  `INVALID_ARGUMENTS` for an unsupported encoding. `isEncodingSupported`
  returns false for an unsupported encoding. Strict conversion errors signal
  `UNICODE_ERROR`; invalid options/replacement values signal
  `INVALID_ARGUMENTS`.
- Grapheme positions are one-based extended-grapheme positions, not byte or
  codepoint positions. `graphemeSubstr` with omitted length returns the suffix;
  explicit zero returns empty. An explicit length must be nonnegative and the
  pad must be exactly one grapheme. Existing code validates pad only when length
  is supplied; preserve `graphemeSubstr(text,start,,pad)` omission behavior.
  `start < 1` is invalid. `graphemePos` matches exact text with both endpoints
  on grapheme boundaries and returns zero for an empty needle/haystack or no
  match. Reverse preserves scalar order inside each grapheme.
- Preserve family-specific internal failures. Most prepared-data guards signal
  `FAILURE`; normalization also retains internal `UNICODE_ERROR` paths in
  `tools/unicode/unicode_d.crexx`. No uniform internal-error ABI is added.
  G indexed access retains `OUT_OF_RANGE`.
  Do not add catch-all wrappers or convert infrastructure failure into a normal
  result during this ownership move.

Classic adapters continue to own argument checks and presence, `.RexxValue`
conversion, the fixed U+0000–U+00FF byte ordinal bridge, authored source/error
context and stream policy. Existing Unicode BIF wrappers translate
`UNICODE_ERROR` to Classic 23.1 and argument/range errors to 40; stream methods
record their existing codec-failure/error state, and Classic stream BIF adapters
retain SYNTAX/23.1 and NOTREADY translation. A selected external codepage
never changes byte ordinals. These are adapter obligations, not B API features.

### Build, metadata and migration boundary

The public B module compiles after the five private members using their exact
metadata, then links with them into the one core image. It must not import a
previous core/G aggregate to build itself. Minimal B and independent L
bootstrap remain upstream; their runtime closure stays free of this optional
component.

G compiles its remaining classes against the core's private metadata. Its
explicit link selectors include **`unicode_api`**; class
methods do not reference the 23 public procedures, so transitive selection
would silently drop them. Core-only `rxunicode` metadata must expose all 23
procedures without G classes; G's `rxunicode` metadata must expose those 23
plus its two classes once.
The source/docs contract checker inspects both owning source files and their
complete union, preserving RexxDoc blocks and literal defaults. The compiled
metadata check compares canonical names, argument types, optional and reference
bits; owning language, module and source records deliberately change.

The exact `rxfnsc` staging root contains the core and removes stale G metadata.
Both target and file dependencies remain; every Classic member depends on the
staged Unicode image, so a changed import rebuilds its metadata consumers.
After successful copies, staging refreshes all six output timestamps once so
an unchanged-byte producer refresh does not repeat on the next Make build.
The extracted-rule regression covers cold staging, each missing output,
changed/same-byte core refresh, stale G removal and subsequent no-ops.
The consolidated `rxfnsc` image contains its own Classic members and external
Unicode dependencies. Its BIF bodies and stream call spelling are unchanged.

Native `rxc-c` lowers the 23 BIF names to the existing
`rexxclassicbifunicode` entries in `rxfnsc`; it does not need to import the G
facade itself. The provider regression inspects rebuilt wrapper autoload
identities, then compiles existing Unicode and stream fixtures with
`--no-exe-import` against exact library/classlib/rxfnsc/core metadata. It checks
standalone-core autoload, compiler-free static execution, G preloaded with core
physically absent, and duplicate-provider rejection. Inlined public calls may
leave private helper hints; both public and private hints select the core.
The new C research harness includes the core only in target runtime inputs and
preparation. Its B/L compiler-host image remains independent of target Unicode.

Each linked image selects **one provider aggregate**:

- C-only/B Unicode applications: standalone core plus the required B/Classic
  libraries, with no G image or G metadata in the isolated proof.
- Existing combined/G products: G's aggregate already embeds the B Unicode
  members. Retain that provider once; do not append the standalone core to an
  input list already containing G.

The linker rejects duplicate providers; this migration does not add deduplication
or change linker behavior. The native importer can coalesce duplicate names
and retain the first package's autoload stem, so a broad `bin/` import directory
cannot prove B ownership. Test exact roots and generated autoload metadata,
plus installed mixed-product compatibility and explicit static linking.
The VM links already-loaded exports before considering unresolved autoload
hints; prove the integrated G/embedded path with the standalone core physically
absent, rather than adding a new loader mechanism.
Existing bytecode carrying a G autoload identity is not retroactively rewritten;
rebuild affected callers against the selected provider for independence proof.
The current installer remains a combined product. Installed C independence is
a restricted-runtime test using installed artifacts with G absent, not a claim
that this slice creates a separate C-only installer or changes default routing.

Qualification reuses the five Unicode conformance families, existing
RXAS/optional-import checks, Classic Unicode/stream tests and G class fixtures.
New public B surface/presence tests and exact C-only dependency controls cover
the gaps in private-core tests. Emitted-call and helper inspection checks that
the moved facade adds neither a forwarding call nor a new table/input copy.
Direct count/substr retain their streaming shape; search/reverse may retain
the boundaries their algorithms require.

This slice introduces no new property/simple-casing, public traversal/index,
locale, incremental-codec or work-budget API. Those broader STEP03 contracts
remain open; current C property/configuration overlays and L recognition indexes
remain with their existing owners. It also does not add new compiler BIF
bindings: migration reuses the existing support libraries, while completing
the new C compiler's BIF catalogue remains STEP04/06–08. INTERPRET remains last.

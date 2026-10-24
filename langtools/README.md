# Level L language tools

`rxlang` is the Level L generator command. Its source is `rxlang.crexx` and
its callable services are delivered in `rxlanglib.rxbin`. The language remains
`options levell`; `.rxl` and `.levell` input forms are unchanged. These names
replace the old `levell` executable and `levelllib.rxbin` package.

All general Level L documentation and examples live here, alongside tool and
library tests. `lib/rxfnsl/` contains only SDK implementation and required Unicode
build data. Experimental compiler-specific material remains in research.

Start with the [Level L user guide](doc/user-guide.md) for a complete runnable
language, authoring reference and consumer APIs. The
[architecture guide](doc/architecture.md) explains the packages, dependency
direction, source/IR ownership and build/runtime pipelines. The
[diagnostic assessment](doc/diagnostics.md) records error quality,
the implemented recovery profiles and remaining work. This file focuses
on generator implementation and qualification details.

| Reference | Use it for |
|---|---|
| [Bootstrap](doc/bootstrap.md) | Native seed, L0/L1/L2, build dependencies and fixed-point publication. |
| [Generation](doc/generation-design.md) | Pure generation, optional lexical capsules and atomic cache/output publication. |
| [IR and analysis](doc/ir-design.md) | Checked IR, semantic services, facts, transformations and graph/effect proofs. |
| [RXAS target](doc/rxas-design.md) | Canonical instruction metadata, selected records and register assignment. |
| [IR execution](doc/execute-design.md) | Trusted runtime handlers, manifests, leases, frame lifetime and suspension. |

Level L is reusable language-engineering infrastructure. RexxScript 1 is its
first product consumer; the library has no dependency on RexxScript values,
compiler AST classes or compiler globals. The product compiler's third_party/re2c/Lemon frontends are maintained separately.

Build and run from the repository root:

```sh
cmake --build cmake-build-debug --target rxlang
cmake-build-debug/bin/rxlang langtools/tests/example.levell --rxas > example.rxas
cmake-build-debug/bin/rxas example
```

The command checks the entire specification before emitting executable loader
instructions. CMake uses `cmake/generate.cmake`, which writes a temporary artifact and
renames it only on success. A failed generator preserves the previous artifact.
Direct shell redirection does not offer that atomic replacement guarantee.
Without `--rxas`, the command emits Level B-compatible source for inspection and
comparison. `--graph` prints the common parser graph for independent analysis;
it does not certify DFA/LR admission.

## Driver integration and regressions

`crexx` recognizes `.crexx` sources headed by `options levell`, `.rxl` sources
with an optional OPTIONS header, and existing `.levell` files. It generates
packed RXAS, assembles providers before compiling consumers, and tracks grammar,
generator, assembler and runtime identities. A library contains the authored
modules; a program selects their shared runtime once. This keeps independently
built grammar libraries composable. CMake's Level B bootstrap remains independent
of the high-level driver.

Focused panels take an existing build and a compatibility output argument.
Their diagnostic workspaces are private and deleted automatically:

```sh
python3 langtools/tests/check-authoring-build.py BUILD OUTPUT
python3 langtools/tests/check-list-shorthand.py BUILD OUTPUT
```

The first covers source selection, L0/L1/L2 parity, combined and separate builds,
rebuilds, last-good-output preservation and native packaging. Use `--bin PREFIX/bin`
to exercise installed tools. The second compares retained recursive rules with
repetition across LL(1), LR(1), packrat and Earley, including order, spans and
failure locations. Existing generated-runtime CTests include an exact-budget
3,000-item named-tree regression. The installed package smoke uses the driver
for both source forms and checks that a failed grammar preserves the program.

The independent callable-service smoke tests compare their generated RXAS with
the CLI's output, converting only native CRLF text newlines to LF for this
comparison. The service artifacts and binary publication checks retain exact
bytes. Prepare them with `levell_services_opt` and
`levell_services_noopt`; each target builds the shared
`levell_generated_grammar` oracle as well as its service bytecode. Its original
specification/tool dependencies retain freshness and missing-output recovery,
and parallel Make consumers wait for this single producer. No earlier smoke
test or full `testlangtools` build is required.

## Authored rules and admission

`example.levell` is a complete comma-separated-number language. The larger
[`rexxscript.levell`](../rexxscript/grammar/rexxscript.levell) uses the same
`name: kind qualifiers = ...` declaration family:

```rexx
number: token = 1
scan: lexer input utf8
  digit: charset = U+0030 to U+0039
  integer: rule = digit+ then emit number
parse: parser semantics contextfree input tokens start value
  value: rule = number
```

The product command supports acyclic regular lexical definitions, canonical
LR(1), LL(1), ordinary packrat PEG, Earley shared forests and Pratt operator
components. Lexing uses longest match followed by declaration priority.
`semantics contextfree` is the default; ordered choice requires
`semantics ordered`. The default parser is LR(1) for context-free rules and
packrat for ordered rules. Select other algorithms explicitly with `backend`;
unsupported profiles fail at generation. Pika and Squirrel remain research
implementations, not silently selected fallbacks.

The required tool and runtime are Level B. The specification frontend is
self-hosted through L0/L1/L2; see [the bootstrap guide](doc/bootstrap.md).
`input utf8` uses Unicode scalar rules, while `input utf8bytes` explicitly
preserves the former PoC byte matching with UTF-8 validation. `input bytes`
selects unchecked raw bytes. Scalar ranges, subtraction and supplementary
codepoints are lowered to the existing byte DFA without Level G dependencies.
Named lexical errors and lexer-only definitions are supported. Unicode 17.0.0
property charsets are available under `input utf8`; identifier rules choose
which sets to use. Recognition preserves original spelling.

`comments line`, `comments block` and `comments nested` use the shared
source-preserving comment processor. Optional `outside` patterns customize
string protection, and `newlines keep` retains block line endings for the lexer.
`tokenstage` selects finite context and bounded lookahead; first matching
declaration wins. Consumer-specific transformations remain explicit.

Build errors name source rules and lines. Nullable tokens cannot make progress;
recursive lexical rules are unsupported by this backend even when some
particular recursive definition might describe a regular language. LR conflicts
name the token and competing completed rules. They do not by themselves prove
ambiguity. Ordered choice and predicates require an admitted ordered profile;
selecting a CFG backend cannot silently change their semantics. Construction
exhaustion reports a limit, not a
proof that the language cannot be parsed.

Run `rxlang --limits` for construction and runtime allowances, defaults and
representation ceilings. The [user guide's resource reference](doc/user-guide.md#resource-limits)
explains all eighteen settings. Negative specifications are in
`tests/grammars`.

Optional host budgets do not belong in the grammar. For example,
`rxlang file.levell --rxas --limit lr.states=12000` changes the build budget;
applications pass an optional `lllimits..policy` to their parse request. Unknown
keys fail explicitly. Recursive depth and token/table representation constraints
are distinct from configurable work allowances; increasing a budget does not
change backend capabilities.


## Packed artifacts and ownership

RXAS contains the existing portable packed DFA and sparse LR images, source
rule names/lines, grammar identity, token names and declared token stages. It
exports one compiler-importable `namespace..language()` function returning an
`lllanguage..definition`. Its loader calls public library factories; it does not
embed object attribute layouts. `rxas` and `rxlink` own binary serialization and
linking. There is no direct Level L RXBIN writer.

The loaders check image shape, targets, ordering and grammar identity, including
the selected parser root. A loaded
DFA is immutable. A loaded LR table retains the sparse image plus a 16-byte
per-state directory. Lookups borrow its binary storage through a private
read-only `arg expose` helper; they neither copy the image nor create a weak
reference on every lookup. The owner remains alive for the synchronous call.
DFA reads use encoded offsets directly. Public owned values, including
`image()` and program/tree returns, can still copy. Ordinary cREXX value copying
is not copy-on-write. These APIs do not claim universal zero-copy initialization.

Images use explicit little-endian `<at..i32>` fields. The existing
`<packed..int>`/`.packedint` surface is useful for host-local working storage,
but its native representation must not be substituted for portable emitted
constants. Tables are trusted generated artifacts: validation is not a proof
that a malicious replacement image implements its claimed grammar.

For a row with `m` entries, LR image lookup uses at most
`floor(log2(m))+1` comparisons when `m>0`, and zero for an empty row. The loader
requires strictly increasing record order before accepting the image. Runtime
storage is the image plus `16*q` directory bytes for `q` states, in addition to
production/source metadata and per-request trees/stacks. Parsing costs
`O((t+a) log(M+1))` for tokens `t`, emitted derivation work `a` and maximum row
width `M`; for a fixed admitted grammar this remains linear in `t+a`. DFA
transition lookup is constant time. These bounds do not include arbitrary
consumer actions, BIF work, full source-tree copying or error-repair search.

## Review checks

`testGenerated.crexx` compares every action/goto cell of the second language,
every DFA byte class/transition, exact images, malformed images, source ownership
and diagnostics. `rexxscript/tests_functional/build-review.sh opt|noopt` builds
isolated complete interpreter variants; its optional second argument `crexx`
selects the source-emission control. Set `CREXX_BIN_DIR` to the current product
bin directory when using `build-local.sh`. CMake's generated-consumer and modern
RexxScript CTests execute prebuilt programs, with no nested build/test matrix.

The flow, condition, structured-flow and lowering fixtures load one immutable
generated definition, then create a fresh owning request for every document.
Their semantic, budget, error and lifetime checks retain all four parser
backends, both consumer compiler/assembler modes and both concrete engines.
The backend-independent `lowerdemo` helper has one shared preparation owner.
`ts_lllowering_deep` runs the unchanged 2,000-node lowering control once with
the optimized image on the product engine; `-a semantic` selects the other
lowering checks, `-a deep` selects that control, and no argument runs both.
The existing `ts_llast_{noopt,opt}_{rxbvm,rxtvm}` controls already exercise the
same 2,000-deep walker and 1,000-wide tree. The corresponding `ts_llborrowed`
and `ts_llsource` cells retain expired/copied-owner, source-index and resource
boundaries; every small lowering mapping and cleanup case remains in its full
matrix. Consumer modes vary the harness, not the already prepared SDK.
Structured-flow serialization is required only for the maintained sanitizer
contention case; ordinary scheduling uses representative resource observations.

Ordinary flow and structured-flow cells select `-a semantic`: short repeated
sequences and nested controls retain all backend/mode/engine paths, exact graph
allowances and zero/one-short failures. `ts_llflow_resources` and
`ts_llstructuredflow_resources` reuse the LL1 optimized image on the product
engine for the original width 200/depth 40, loop depth 30 and every intermediate
graph-allocation allowance. Reported sizes, completed sweeps and zero remaining
checks guard that selection. No argument retains the complete original manual
workload and output, including Earley's original bounded sizes; `-a resources`
adds its scope observations. These are common post-parse graph/analysis controls,
not parser-budget overrides. The generated AST matrix, origin/recovery matrix,
3,000-item generated tree and AST depth/width controls retain their distinct
parser/source/owner boundaries. Instrumented flow, structured-flow and lowering
cells retain their full original workloads, including SAN-QA-019's serial route,
pending the separate native-memory scope review. The separate resource/deep
entries are ordinary-only, avoiding duplicate full workloads under instrumentation.

The retained ordinary Earley cells in these four families reserve six CTest
process slots because their dense parser/forest index has a large working set
even for the small semantic inputs. This weight admits at most five such cells
in the normal 30-slot pool; it does not start six threads or assert a speed
budget. Other backends and the canonical resource entries keep their ordinary
slot cost. Instrumented scheduling remains under its existing policy.

`ts_llearley_budget` reuses the optimized Earley origins image on the product
engine. Its private `-a earley-budget` route parses one clean flat record with
fresh owning requests and policies sharing one definition. A monotone search
bounded by the public default proves the exact clean-success allowance and
the one-short `earley.cells` resource outcome; unrelated failures are rejected.
Two recovery-API probes must preserve those outcomes without repairs or a
valid partial AST. The bound permits 23 search requests under the current
default, plus the default-success and two recovery probes. The external marker
guards the selected route and boundary checks. All no-argument origins cases
and their existing backend/mode/engine matrix remain unchanged.

Callable-service consumers read prepared inputs and publish only to their own
mode/engine-specific destination and sibling staging directory. Ordinary cells
can share the test pool; the instrumented route retains its existing serial
contention policy and all requests, oracles and hang-protection timeouts.

Lexical-reuse service probes likewise use read-only prepared images and private
request state. Their ordinary six-slot admission replaces whole-pool exclusion;
instrumented isolation remains. The CLI cache/build control retains its separate
serialization and its actual output/cache locking checks. The single service
panel keeps fresh current-admission and independent cold/warm byte oracles for
each distinct identity, malformed-capsule, budget and failure case.


## Products and public services

`stage-l-libraries` builds the core SDK and its three separately named packages.
`stage-l-tools` builds `rxlang`, `rxlanglib.rxbin` and the generic tool helpers.
Both belong to normal `stage-product`. Research compilers have their own opt-in
stage and install component; neither SDK nor Level L imports them.

Optional IR packages have separate targets and images:
`rxfnsl_analysis` supplies [graphs and proofs](doc/ir-design.md),
`rxfnsl_rxas` supplies [RXAS records and register assignment](doc/rxas-design.md),
and `rxfnsl_execute` supplies [bounded IR execution](doc/execute-design.md).
Select them through imports and linking; normal SDK installation delivers all
four images. Building the individual `rxfnsl` target prepares only the core and
its base dependency. Optional packages do not enter grammar bootstrap or static
targets merely because they ship with the SDK.

| Image / target | Public modules | Runtime image dependencies |
|---|---|---|
| `rxfnsl.rxbin` / `rxfnsl` | Core source, AST, IR and transformations | `library.rxbin` |
| `rxfnsl-analysis.rxbin` / `rxfnsl_analysis` | `llirgraph`, `llireffects` | Core and `library.rxbin` |
| `rxfnsl-rxas.rxbin` / `rxfnsl_rxas` | `llrxascatalog`, `llrxas` | `library.rxbin` |
| `rxfnsl-execute.rxbin` / `rxfnsl_execute` | `llirexecute` | Core, analysis and `library.rxbin` |

The [RXAS guide](doc/rxas-design.md#construct-one-function) supplies a complete
independent producer. Its emitted program runs without these SDK images. The
[execution guide](doc/execute-design.md) describes the admitted control profile and
continuation ownership; it does not promise a full Classic or RexxScript backend.

`llgenerate..request(source, format, filename, limits, rxl, bytes, ?lexical)` accepts exact
binary source and returns `status()`, `error()` and owned `output()`. Defaults are
`rxas`, `<source>`, a default Level L policy, `0` and 67108864. The service uses
`llir.writer`, returns no partial artifact on error and has no CLI/stdout or file
publication dependency. `llfront` and `llgenerate` are in `rxlanglib.rxbin`;
link it alongside `rxfnsl.rxbin` and the B runtime library. The optional final
binary opts into lexical construction reuse; omission keeps ordinary cold
behavior. See the [service reference](doc/user-guide.md#callable-generation-service).

The Level B helpers in `common/` are separately installed libraries:
`llrxastext` escapes exact bytes and holds source-tagged instruction records;
`llbinmeta` reads bounded RXBIN007 envelopes and raw callable metadata;
`llfileoutput` publishes owned writer output atomically. Compiler type/ABI
admission stays in the research compiler. `tests/services.crexx` is their
independent consumer, including failed-output preservation.

The [selected-callable classification API](doc/user-guide.md#selected-callable-classification)
supplies pool-local procedure/method/factory facts and an opt-in bounded owning
catalogue with monotonic usage queries. The
[Level L plan](../research/level-l/PLAN.md#callable-metadata-helper-contracts)
owns its acceptance criteria and implementation status. Existing metadata APIs remain compatible; language ABI policy stays
with the consumer. Pure `llbinmeta..validname(name)` checks the existing metadata
identifier byte alphabet without allocating a catalogue; bounded catalogue name
checks reserve work before using the same utility. The independent
`tests/metadata-classify` consumer and its RXAS producers cover ownership,
selected errors, lifetime and aggregate limits.

Run a clean B-only bootstrap with an existing native seed:

```sh
CREXX_BIN_DIR="$PWD/cmake-build-debug/bin" langtools/bootstrap/build-local.sh opt
CREXX_BIN_DIR="$PWD/cmake-build-debug/bin" langtools/bootstrap/build-local.sh noopt
```

Each invocation cleans its temporary build and logs. L1/L2 generated grammar
agreement is a build prerequisite. The Make scheduling regression also checks
missing L2 recovery and no-op/changed-input behaviour.

## Compiler-host contract projection

With `CREXX_BUILD_RESEARCH_COMPILERS`, `llcontract` builds the optional RXPA
provider `bin/providers/rxcontract.rxplugin`; `llcontract_static` supplies its
static counterpart. Both and their native-package dependencies belong only to
`ResearchCompilers`. The compiler-host binding imports/links the provider
explicitly. It is outside the default SDK, Level L bootstrap and emitted
application/runtime closure. `llcontract_projection` is the focused rxbvm
fixture, prepared by `llcontract_test`.

`rxcontract.project(name, result, args, flags, kind, task, bytes, work)` returns
an owned `.string[]`. It decodes the complete callable grammar through
binutils `rxcontract`, including recursive references/arrays and authored
defaults. It does not implement a second parser or admit a source-language ABI.
`kind` is 0 procedure, 1 method or 2 factory; `task` is 0/1. C scalar/reference
admission remains the C consumer's policy.

The one-based `rxcontract1` vector has 13 header fields followed by seven fields
per argument:

| Fields | Meaning |
| --- | --- |
| 1–7 | version, status, error field, error message, error byte offset, reserved bytes, reserved work |
| 8–13 | canonical name, kind, task, normalized full result type, raw flags, argument count |
| 14 + 7(n−1), through +6 | argument name, full type, exposed 0/1, optional 0/1, vararg 0/1, default kind, authored default literal |

Default kinds are `none`, `integer`, `float` and `string`. Status is `ok`,
`invalid`, `limit` or `allocation`. Failure contains the 13-field header and no
partial declaration. Decimal counters/flags are textual fields. Byte/work
allowances charge input, reserve repeated decoder scans before parsing, and
charge output projection. They measure logical work/payload, not elapsed time
or native allocation RSS. Embedded NUL metadata and nonrepresentable allowances
are refused.

`rxcontract.acquire(artifact, callable, bytes, work)` selects a real native
provider declaration through the optional owning loader service. `artifact` is
an explicitly selected `.rxplugin` file; no source, PATH or package-name sweep
occurs. Its owned 13-field `rxnative1` vector is: version, status, diagnostic,
used bytes, used work, actual provider id, canonical callable name, kind, owner,
raw result, raw arguments, raw flags, task. The status family additionally
includes `unavailable`. Failure retains only the version/status/diagnostic and
counters. The caller then projects the raw callable fields with `project` and
applies its language admission policy. A native manifest/provider identity is
not inferred from RXBIN prototype declarations.

The loader owns provider initialization/session lifetime and refuses a nested
same-context declaration query. Logical acquisition allowances do not bound
trusted initializer execution. Legacy prefix-only RXPA hosts safely report the
optional service unavailable. The pure B `signature.projected` consumer in the
research compiler accepts the checked vector without importing native code.

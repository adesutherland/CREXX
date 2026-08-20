# Stage 6: Provisional Component Classification

These are initial Release 1 recommendations for review, not approved product
policy. They do not change the build, install, language, or compatibility
contract.

## How to read the classification

The classification uses independent axes:

| Axis | Meaning |
|---|---|
| Contract | `B`, `G`, `C`, `L`, `B+G`, `shared tooling/runtime`, `host/integration`, or `none/example`. A C implementation language is not a Level C contract. |
| Product role | `bootstrap core`, `level core`, `standard`, `integration`, `optional`, `example`, `experimental`, `deprecated`, or `removal candidate`. |
| Delivery | `required`, `default`, `opt-in`, `developer-only`, or `source-only/not shipped`. |
| Source composition | `pure Rexx`, `pure RXAS`, `pure native`, `generated`, or a `mixed package`. |
| Runtime call path | `Rexx-only`, `hybrid`, `native`, or `not implemented`. `hybrid` includes inline/imported RXAS, native plugins, VM host operations, or external services. |
| Maturity | `supported`, `provisional`, `experimental`, `deprecated`, `recognition-only`, or `placeholder`. |

Composition is deliberately two-dimensional. For example, the Level G LLM
module is pure Rexx source but has a hybrid network/TLS/service call path. A
Level B source file containing inline `assembler` is still Rexx-authored but
has a hybrid call path. A package containing Rexx adapters and C providers is a
mixed package.

Every public leaf inherits the classification of its syntax family, source
selector, or containing package unless an exception is stated. This supplies a
classification for all 2,158 raw IDs without duplicating the leaf catalogue.

## Language syntax contracts

The current typed compiler uses one grammar for Level B, G, and L selection.
There is not yet an enforced G-only syntax delta. The least misleading Release
1 classification is therefore:

| Syntax rows | B/G classification | Product role | Delivery/maturity | Composition |
|---|---|---|---|---|
| `SYN-TYPED-LEVELB-OPTION` | B only | bootstrap core | required/supported | native compiler surface |
| `SYN-TYPED-LEVELG-OPTION` | G only | level core | required/provisional | native compiler surface |
| `SYN-TYPED-LEVELL-OPTION` | neither B nor G; L selector | level core | developer-only/experimental | native compiler surface |
| every other `SYN-TYPED-*` row | **B+G** | bootstrap core in B; inherited language foundation in G | required/supported for current typed behavior | native compiler lowering to RXAS/VM |
| every `SYN-CLASSIC-*` row | C only, with maturity taken from its observed-state row | Level C core or standard | parser/default; executable support only where proven | native Classic parser plus hybrid Level B runtime |
| every `SYN-RXPP-*` row | shared preprocessor syntax, not B/G/C/L syntax | standard tooling | default/provisional | hybrid RXPP transformation pipeline |

Level L currently accepts the typed foundation as an implementation fact, but
the L language contract is too skeletal to classify those same rows as stable
L features. No syntax row is provisionally G-only other than the selector. If
Release 1 intends G to hide low-level B constructs such as register views,
inline RXAS, or binary memory views, that requires a later language-design
decision and compiler enforcement; this catalogue does not invent that split.

The eight `BIF-TYPED-*` identities inherit B+G except `BIF-TYPED-TRACE`, which
is a reserved name rather than a callable BIF. Their precise callable/contextual
status remains as recorded in the raw file.

## Level B library closure

All 129 selectors and their public leaves are B contracts. This first cut
separates a bootstrap-core **candidate closure** from the larger default and
optional libraries. “Candidate” is important: the final B closure must be
derived from the actual transitive needs of the compiler, preprocessor,
link-time exits, driver, and core libraries, then reduced and measured. Current
membership in `library.rxbin` is not treated as proof of necessity.

| Proposed role/delivery | Exact selectors | Count |
|---|---|---:|
| bootstrap-core candidate; required | `_address`, `_rxsystem`, `loadmodule`, `raise`, `signal`, `symbol`, `trace`, `value`, `version`; `abbrev`, `center`, `centre`, `changestr`, `compare`, `copies`, `countstr`, `delstr`, `insert`, `length`, `lower`, `overlay`, `pos`, `lastpos`, `left`, `right`, `reverse`, `space`, `strip`, `substr`, `substro`, `translate`, `upper`, `verify`; `_ftrunc`, `_itrunc`, `abs`, `format`, `max`, `min`, `numeric`, `sign`, `trunc`; `b2x`, `b2d`, `binary`, `c2x`, `c2d`, `d2b`, `d2c`, `d2x`, `x2b`, `x2c`, `x2d`, `xrange`; `arrayfind`, `splice`, `arrayinsert`, `arraydelete`, `arrayappend`, `arrayprepend`, `objectarrayinsert`, `objectarraydelete`, `objectarrayappend`, `objectarrayprepend`, `objectarraydrop`, `objectarraymove`, `arrayget`, `arrayset`, `arraycontains`, `arrayindexof`, `arraycopy`, `arraydrop`, `arrayhi`, `arraymove`, `stem`; `delword`, `word`, `words`, `wordindex`, `subword`, `wordlength`, `wordpos`, `parsecompile`, `parsestring`, `parse`, `datatype`, `parseExec` | 87 |
| B standard; default | `getenv`, `linesize`; `filter`, `sequence`, `find`, `index`; `_datei`, `_dateo`, `_jdn`, `date`, `random`, `reradix`, `time`; `fnv`; `arraypop`, `arrayshift`, `arrayreverse`, `arrayjoin`, `arraysort`; `qpos`, `qsplit`, `qsplitsafe`, `qextractall`, `qextractpair`, `qstripcomment`, `qremoveall`, `qword`, `qwordlength`, `qwords`, `qwordindex`, `qwordpos`, `qsubword`; `fsayfmt` | 33 |
| B optional; opt-in linked image | `regex`, `wordrep`; `fmtmask`, `compilemask`, `runmask`; `fileio`; `rxsocket`, `rxhttp`; `rxjson` | 9 |

The 87-row candidate is intentionally a conservative starting closure, not a
claim that 87 functions are sufficiently light. Dependency tracing should be
allowed to demote unused items before the later Level B re-engineering stage.
Conversely, an actual bootstrap dependency can promote a standard item, but it
must then meet bootstrap performance and quality requirements.

Composition inheritance for Level B leaves is:

- source language is pure Rexx for the 129 selectors;
- a selector whose source contains inline `assembler`, imports RXAS, or invokes
  VM/system operations has a hybrid call path (85 selector files currently
  match that source-review rule);
- the consolidated `library.rxbin` is therefore a mixed implementation image;
- source-only active RXAS `_elapsed` and `_rxvml_address_native` are pure RXAS
  components supporting that image, not separate user BIF contracts.

## Class libraries

Public exports and members inherit from their source selector.

| Selectors | Contract | Role | Delivery | Source/call path | Maturity |
|---|---|---|---|---|---|
| `StringIterable`, `StringIterator`, `ObjectIterable`, `ObjectPrintable`, `ObjectIterator`, `ObjectComparator`, `RexxComparator`, `Rexx` | B+G object foundation | level core | default; promote only proven bootstrap dependencies to required | pure Rexx source; selected files use inline assembler, so the image is hybrid | supported |
| production array-list, linked-list, stack, hash-map/set, tree-map/set selectors and their iterators | B+G collections | standard | default | pure Rexx source; selected tree structures have hybrid inline-RXAS paths | supported/provisional performance |
| `StringOldTreeMap` | B | experimental comparison implementation | developer-only | pure Rexx source, hybrid path | deprecated for user use |
| `Qfind`, `Scanlex` | B+G | optional language/data utilities | opt-in | pure Rexx | provisional |
| `Id`, `KeyDB`, `Os` | B+G adapter contracts | integration | opt-in as separate `classlib_native` image | pure Rexx wrappers with native plugin call paths; mixed delivered package | provisional |

The “production collections” row means every remaining selector in the
33-selector pure-Rexx classlib after the explicitly named object-foundation,
`StringOldTreeMap`, `Qfind`, and `Scanlex` rows. It covers every corresponding
export and member exactly once.

## Level G

| Component/leaves | Contract | Role | Delivery | Source/call path | Maturity |
|---|---|---|---|---|---|
| `INT-rxfnsg-llm` and its members | G only | standard hosted-service abstraction | default in a G library image | pure Rexx source; interface itself is abstract | provisional |
| Anthropic, Gemini, Ollama, and OpenAI classes and members | G only | integration | opt-in provider modules rather than one mandatory core image | pure Rexx source; hybrid HTTP/socket/TLS/external-service call path | experimental-to-provisional by provider |

The current single `llm.crexx` image couples the abstraction and all providers;
the proposed rows describe the desired contract/delivery split, not a change
performed here. Level G currently has no distinct general-purpose library
beyond this hosted-service domain, so it should not yet be described as a
complete user-level standard library.

## Level C: unique contracts, never aliases of B/G

All 70 `CBIF-*` rows are unique Level C standard BIF contracts. They do not
inherit the role, signature, coercion, error behavior, or implementation status
of same-named Level B functions.

| Level C component | Contract/role | Delivery | Implementation composition | Maturity |
|---|---|---|---|---|
| Classic parser, DSLSH, and canonical lowering | C level core | default | pure native parser/lowering, calling Level B runtime support where executable | parser supported; lowering subset provisional |
| `rxfnsc` value, stem, pool, and BIF-dispatch image | C level core | required with executable Level C | pure Level B Rexx source with selected inline assembler and classlib dependencies; hybrid runtime path | provisional |
| `CBIF-LENGTH`, `CBIF-SUBSTR` | C standard BIFs | default | native compiler lowering plus Level B-implemented C runtime; hybrid pipeline | executable path proven |
| `CBIF-ABBREV`, `ABS`, `COPIES`, `DATATYPE`, `LEFT`, `LOWER`, `MAX`, `MIN`, `POS`, `RIGHT`, `SIGN`, `SPACE`, `STRIP`, `UPPER`, `VERIFY`, `WORD`, `WORDS` | C standard BIFs | default only when compiler lowering reaches them | Level B Rexx dispatcher implementation exists | implemented in dispatcher; compiled Level C path not yet proven here |
| every other `CBIF-*` row | C standard BIF | recognition may remain default for parser/tooling; runtime must fail closed | no implementation established by this review | recognition-only |

The last row is the exact complement of the 19 dispatcher names above within
`raw-levelc-bifs.md`, so all 70 contracts are classified once. Standards
compliance requires a per-BIF signature/coercion/error/conformance matrix; name
recognition alone is not compliance.

RexxScript remains a separate product: its source contracts are B, its purpose
is Rexx compatibility, and its evaluator is not a Level C implementation. It
is provisionally `standard/default`, pure Level B Rexx source with a hybrid
native-packaged CLI. Its compatibility exports and members inherit that row.

## Level L

All 12 `FN-rxfnsl-tinyexpr-*` leaves are L-only, `example`,
`developer-only`, pure Level L Rexx source with packed-binary operations, and
`experimental`. They prove the language-oriented path but do not define a
general Level L library. The `rxfnsl` image should not be in a normal user
default until a Level L contract is designed.

## Preprocessor, macros, and debugger

| Component/leaves | Contract | Role | Delivery | Source/call path | Maturity |
|---|---|---|---|---|---|
| `TOOL-rxpp` and all `SYN-RXPP-*` rows | shared B/G source tooling | standard | default | mixed package: Level B Rexx engine, native launcher/VM, library images, and `precomp` | provisional |
| `PKG-RXPA-precomp` and its 20 `RXPA-precomp.*` registrations | RXPP internal compatibility boundary | level core within RXPP | required with RXPP; not a general plugin | pure native callable implementation in a mixed RXPP package | supported internally/provisional ABI |
| `TOOL-rxpp-sh` | shared editor/parser tooling | optional | developer-only unless parser mode is a Release 1 product | pure native over compiler parser services | provisional |
| `LIB-RXPP-maclib` and all `MACRO-maclib-*` leaves | generated B/G source convenience surface | experimental | developer-only until corrected and covered | pure RXPP definitions generating Rexx; generated paths vary | not release-quality |
| `LIB-RXPP-macsys` and all `MACRO-macsys-*` leaves | generated B-oriented/system source helpers | experimental | developer-only | pure RXPP definitions, several generating inline RXAS/system calls; hybrid generated path | not release-quality |
| `LIB-RXPP-mathlib` and all six `RXPPPROC-mathlib-*` leaves | B source-inclusion helpers | example | source-only/not shipped until corrected | pure Rexx source inclusion | known semantic defects |
| `LIB-RXPP-syslib` | no contract | removal candidate | source-only/not shipped | empty Rexx placeholder | placeholder |
| `SRC-RXPP-rxppt` | no current contract | removal/archive candidate | source-only/not shipped | pure Level B Rexx workbench | residual |
| `TOOL-rxdb`, its two exports, and 18 public members | B developer tooling | experimental | developer-only | pure Level B Rexx source compiled/C-packed; hybrid VM metadata and inline-RXAS path | prototype/T1 |

The macro rows override current broad install behavior. Fixing the identified
definitions is implementation work for a later task; this catalogue only
records why they should not currently be called “standard”.

## Core toolchain, runtimes, hosts, and profiling

| Components | Contract | Role | Delivery | Composition | Maturity |
|---|---|---|---|---|---|
| `TOOL-rxc`, `TOOL-rxas`, `TOOL-rxlink` | shared B/G/C/L toolchain | level core | required | pure native, with generated parser sources and linked Rexx exits | supported typed path; C/L maturity separate |
| `TOOL-rxdas`, `TOOL-rxcpack` | shared binary/native packaging tooling | level core | required for development/native packaging | pure native | supported |
| `TOOL-crexx` | B-authored user/toolchain driver | standard entry point | default | pure Level B Rexx source, generated C package, hybrid process/system path | provisional-supported |
| `TOOL-rxvm`, `TOOL-rxbvm`, `LIB-rxvml`, `LIB-rxbvml` | shared runtime | level core | required | pure native | supported, performance baseline required |
| `TOOL-rxvme`, `TOOL-rxbvme` | B-packed convenience runtimes | standard | default | mixed native VM plus generated/packed B bytecode | supported |
| `LIB-crexxsaa`, `TOOL-crexxsaa` | host API/integration | integration | default | pure native | provisional-supported API |
| `INFRA-RXPA`, `INFRA-RXPASHIM` | native extension infrastructure | level core | required by native packaging/plugins | pure native | supported internal/public ABI boundary |
| `INFRA-VM-PROFILING`, `TOOL-rxseq` | developer performance tooling | optional | developer-only; profiling runtime-off by default | pure native | provisional |
| `TOOL-rxvm-instrumented`, `TOOL-rxbvm-instrumented` | test infrastructure | example/test harness | developer-only/not installed | generated native variants | testing only |

## Compiler exits

Every exit is implemented in pure Level B Rexx, but the linked bundle is mixed
purpose. Public leaves inherit these rows.

| Exits/infrastructure | Contract | Role | Delivery | Maturity |
|---|---|---|---|---|
| `EXIT-address`, `EXIT-parse`, `EXIT-signal`, `EXIT-trace`, `INFRA-EXIT-TOKEN` | B+G compiler language infrastructure | bootstrap/level core | required | supported |
| `EXIT-rexxscript` | B compatibility integration | integration | default only with RexxScript | provisional-supported |
| `EXIT-dump`, `EXIT-query` | compiler developer utilities | optional | developer-only | provisional |
| `EXIT-execio`, `EXIT-fsay`, `EXIT-msay` | I/O integrations/examples | optional | opt-in | provisional |
| `EXIT-add`, `EXIT-sort`, `EXIT-sortx`, `EXIT-substr`, `EXIT-dummy`, `EXIT-pprint` | mechanism demonstrations | example | developer-only/source examples; `pprint` is packaged below `examples/` | demonstration |

The default `rxcexits.rxbin` image still mixes several support levels, but the
`pprint` demonstration is now separated into an explicit example bundle. The
remaining delivery proposal requires a later bundle partition or allowlist.

## Native plugins

All `RXPA-` leaves inherit from their package. Adapter-bearing packages are
mixed packages; otherwise their callable path is pure native. These are
host/integration contracts unless explicitly tied to B tooling, not Level C.

| Package | Role | Proposed delivery | Maturity/disposition |
|---|---|---|---|
| `arrays` | deprecated duplicate | source-only/not shipped | deprecated in favor of typed arrays |
| `cipher` | optional | opt-in | provisional; MD5-only legacy scope |
| `console` | optional platform integration | opt-in | experimental/platform-limited |
| `fileio` | optional platform integration | opt-in | provisional/platform-limited |
| `fpool` | removal candidate | source-only/not shipped | incomplete |
| `getpi` | example | developer-only | demonstrator |
| `gui` | integration | opt-in | experimental/GTK-dependent |
| `hash` (`rx_hash`, public namespace `rxhash`) | B+G standard | default | supported SHA-256 surface; process-reentrant, binary-safe, dynamic/static declarative packaging |
| `id` | integration | opt-in with `classlib_native` | provisional |
| `keyaccess` | integration | opt-in with `classlib_native` | experimental pending integrity evidence |
| `llist` | optional duplicate | developer-only | experimental |
| `map` | optional native collection | opt-in | provisional/experimental |
| `matrix` | optional numeric package | opt-in | experimental/Windows-limited |
| `odbc` | integration | opt-in | experimental/feature-gated |
| `pick` | example GUI subset | source-only/not shipped | experimental/unselected |
| `pipe` | integration | opt-in | experimental/unselected by default |
| `process` | integration | opt-in | experimental/unselected by default |
| `recv390` | specialist integration | opt-in | provisional |
| `regex` | optional alternate implementation | developer-only | experimental; semantics not reconciled |
| `rxmath` | optional native numeric library | opt-in | provisional; not Level C math |
| `rxml` | integration | opt-in | experimental/platform-limited |
| `rxtcp` | deprecated/legacy network alternative | developer-only | deprecation candidate |
| `socket` | deprecated network alternative | source-only/not shipped | explicitly deprecated |
| `stack` | optional duplicate | developer-only | deprecation candidate |
| `strings` | optional native accelerator/legacy surface | developer-only pending semantics and safety review | experimental |
| `system` | first-party tool integration | default; required by tools that use it, not by the light B library contract | provisional-supported subset; broad API needs partitioning |
| `testrx` | removal candidate | source-only/not shipped | residue |
| `treemap` | optional duplicate | developer-only | experimental |

`PKG-RXVM-mc-decimal` is shared runtime `level core/required`, pure native, and
the supported normal decimal implementation. `PKG-RXVM-db-decimal` is an
`optional/developer-only` pure-native alternative with provisional maturity;
it must not imply equivalent conformance. `PKG-RXFNSB-NATIVE-DES` is a
deprecated, source-only native legacy cipher.

## Residual libraries, examples, demonstrations, and PoCs

| Raw family | Contract | Role | Delivery/composition |
|---|---|---|---|
| `LIB-veclib` and its six exports/members | B | experimental | developer-only; pure Rexx source, current internal-looking API |
| `LIB-rxmath-source` and all `SRC-RXMATH-*` | none until selected | removal/archive candidates | source-only; mixed Rexx/native/RXAS tree |
| active `_elapsed` and ADDRESS-bridge RXAS rows | B runtime implementation | level core implementation | required inside containing runtime; pure RXAS |
| every other residual `SRC-RXFNSB-RXAS-*` row | no supported contract | deprecated/removal candidate | source-only; pure RXAS |
| every `DEMO-*` and `EXAMPLE-*` row | none/example; examples may demonstrate B, G, L, RXPP, or integrations | example | source-only/developer material; Rexx except the explicitly RXAS demo |
| both `POC-*` rows | C compiler architecture evidence, not C contract | experimental PoC | developer-only; pure native/compiler source |

## Stage 6 proposed product shape

The simple headline taxonomy is therefore:

- **Core** means only language/runtime/toolchain closure required to build and
  execute the selected level; Level B has the stricter `bootstrap core` subset.
- **Standard** means supported user-facing capability delivered by default but
  not required for bootstrap.
- **Optional/integration** means an explicit install choice with its own
  dependencies and test obligations.
- **Examples/experimental** are developer material and must not enter normal
  Release 1 delivery merely because their targets write into `bin`.
- **Deprecated/removal candidates** remain catalogued so their disappearance is
  an explicit compatibility decision.

No classification in this file approves source deletion, contract removal,
package movement, or a syntax restriction. Those require the later approval
and implementation stages.

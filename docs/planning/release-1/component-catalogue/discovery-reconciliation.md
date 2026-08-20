# Stage 3: Discovery Reconciliation

This stage reconciles the Stage 2 source inventory with the configured build,
linked bytecode metadata, registered tests, documentation, and install rules.
It records what the repository currently does. Product-role and delivery
recommendations are deliberately deferred to Stage 6.

## Evidence order

When two sources disagree, this catalogue uses the following order and records
the disagreement rather than silently choosing one description:

1. source declarations define the callable or syntactic contract;
2. CMake defines whether and under what conditions it is built;
3. a fresh linked image proves what the current toolchain emits;
4. CTest registration proves the existence and shape of automated evidence,
   but not that every path is covered or currently passing;
5. install rules define current delivery;
6. user and AI-context documentation describe intent and usability.

Release status is not inferred from `develop`; this is a repository-state
catalogue, not a claim that every discovered component has shipped.

## Linked library reconciliation

The Debug build contained fresh linked images in `cmake-build-debug/bin`. Each
image was newer than its selected source roots when inspected. `rxdas` output
was redirected to temporary files outside the repository and its exposed
metadata was compared with the Stage 2 source inventory.

| Linked image | Source/build boundary | Reconciliation result |
|---|---|---|
| `library.rxbin` | active `rxfnsb` Rexx and RXAS selectors | Every namespace-exposed active source procedure, register, class, and interface was present in linked metadata. |
| `classlib.rxbin` | 33 pure-Rexx classlib selectors, including `StringOldTreeMap` | Every exposed class/interface/function and every catalogued public member/factory was represented. |
| `classlib_native.rxbin` | `Id`, `KeyDB`, and `Os` | Every selected export and public member was represented. This is a separate linked image, not part of `classlib.rxbin`. |
| `rxfnsg.rxbin` | Level G `llm.crexx` | All five exported types and their public members were represented. |
| `rxfnsc.rxbin` | Classic-value, stem, variable-pool, and BIF-dispatch support | Every namespace-exposed source contract and catalogued public member was represented. |
| `rxfnsl.rxbin` | Level L `tinyexpr.crexx` | All exported functions were represented. |
| `rexxscript.rxbin` | RexxScript evaluator/runtime and compatibility exports | Every namespace-exposed source contract and catalogued public member was represented. |
| `rxcexits.rxbin` | 15 default compiler exits plus token infrastructure | The configured compiler-exit bundle exists as one linked image. The sixteenth exit, `pprint`, is built and tested as a standalone example. |

Factory metadata requires one normalisation: an exposed factory is encoded as
`§factory` in procedure metadata and as a `factory` member marker for its
interface. After that normalisation, named and default factories reconcile.
No source-declared namespace export was absent from its linked image.

`veclib.rxbin` is also an `ALL` target, but is emitted under
`cmake-build-debug/lib/veclib`, not the shared `bin` directory. It is therefore
built by default but is not captured by the root `install(DIRECTORY .../bin/)`
rule.

## Library build and current delivery boundaries

| Component | Default build | Output | Current install consequence |
|---|---|---|---|
| `rxfnsb` library | `ALL` | `bin/library.rxbin` | installed by the complete `bin/` directory sweep |
| `classlib` | `ALL` | `bin/classlib.rxbin` | installed by the sweep |
| `classlib_native` | `ALL` | `bin/classlib_native.rxbin` | installed by the sweep despite being a separate native-adapter surface |
| `rxfnsg` | `ALL` | `bin/rxfnsg.rxbin` | installed by the sweep |
| `rxfnsc` | `ALL` | `bin/rxfnsc.rxbin` | installed by the sweep |
| `rxfnsl` | `ALL` | `bin/rxfnsl.rxbin` | installed by the sweep |
| RexxScript | `ALL` | `bin/rexxscript.rxbin` and CLI | installed by the sweep |
| compiler exits | normal build dependency | `bin/rxcexits.rxbin` | the 15-exit default bundle is installed by the sweep; standalone `pprint_example.rxbin` installs under `examples/exits/pprint` |
| vector library | `ALL` | `lib/veclib/veclib.rxbin` | not installed by the root sweep |
| old `lib/rxmath` tree | no CMake entry point | none | source-only |

The root install rule copies every matching file under the configured build's
`bin/` directory. It does not express product-level allowlists. Consequently,
default build selection, stale or auxiliary files in `bin`, and release
packaging are currently coupled. This fact is important to Stage 6; it is not a
recommendation to change the rule in this task.

## Primary toolchain, runtime, host, and profiling components

| Component family | Build/reconciliation fact | Current delivery consequence |
|---|---|---|
| `rxc`, `rxas`, `rxlink` | separate native compiler, assembler, and linker targets with extensive subsystem tests | executables are emitted under `bin` and swept into install |
| `rxdas`, `rxcpack` | native disassembler and RXBIN-to-C packager; disassembler has round-trip coverage | emitted under `bin` and swept into install |
| `crexx` | Level B driver compiled and native-packaged through the toolchain; depends on core tools, runtimes, `system`, and library images | emitted under `bin` and swept into install |
| `rxvm`, `rxbvm` | two dispatch variants over deliberately separate VM core object variants | emitted under `bin` and swept into install |
| `rxvme`, `rxbvme` | the same dispatch variants with the packed Level B library | emitted under `bin` and swept into install |
| `rxvml`, `rxbvml` | embeddable static runtime variants used by native packaging/hosts | archives are emitted under `bin` and therefore swept into install |
| `libcrexxsaa` and `crexxsaa` CLI | public shared host API/header plus cache-maintenance CLI; also have explicit install rules | library and CLI are deliberately installed; the header is installed under `include` |
| RXPA and `rxpashim` | native plugin ABI/build framework and standalone symbol provider | static artifacts in `bin` are included by the directory sweep |
| RXPP and `rxpp-sh` | preprocessor plus optional parser/source-map shell | `rxpp` is a normal tool; `rxpp-sh` is built/installed when parser mode is enabled |
| RXPP support libraries and `precomp` | 64 macros and six source-included procedures are shipped in four support files; `precomp` supplies 20 registered native primitives to the Rexx engine | all four support files are staged and installed; the static native module is linked into `rxpp`, while a dynamic form is needed to compile it |
| RXDB | Level B debugger and text UI compiled, assembled, linked, and C-packed into a native executable | the executable is emitted under `bin`; release notes explicitly describe it as experimental |
| VM profiling and `rxseq` | profiling is compile-time gated and runtime-off by default; `rxseq` is the offline sequence analyser | `rxseq` is emitted under `bin`; the profiling contract is present only in a profiling build |
| `rxvm_instrumented`, `rxbvm_instrumented` | instrumentation-contract test executables created under `BUILD_TESTING` | appear in a testing build's `bin` and are consequently swept by the broad install rule |

The primary tools are therefore now explicit catalogue components. Static
implementation libraries such as `rxclib`, `rxaslib`, and VM object libraries
remain implementation evidence of their containing product rather than public
contract leaves.

## RXPA plugin CMake and delivery matrix

`dynamic` and `static` below describe targets declared by the local CMake. A
plugin built into the shared `bin` directory is also swept into the current
install. “Matrix” means the common template or helper registers both optimized
and unoptimized Rexx smoke executions.

| Catalogue ID | Root selection | Local targets | Test evidence when selected | Current Debug/bin evidence |
|---|---|---|---|---|
| `PKG-RXPA-arrays` | default | dynamic | opt matrix; explicitly labelled deprecated/slow | dynamic present |
| `PKG-RXPA-cipher` | default | dynamic + static | opt matrix | both present |
| `PKG-RXPA-console` | commented on Linux/macOS | dynamic | single runtime test | not selected |
| `PKG-RXPA-fileio` | Windows | dynamic | single runtime test | not selected on macOS |
| `PKG-RXPA-fpool` | unreferenced | dynamic | single runtime test | not selected |
| `PKG-RXPA-getpi` | default | dynamic | opt matrix | dynamic present |
| `PKG-RXPA-gui` | `ENABLE_GTK` | dynamic + GTK3 | single runtime test | not selected |
| `PKG-RXPA-hash` | default | dynamic + static provider package | dual-VM opt matrix, concurrency, native and install/package tests | both forms and canonical provider artifacts present |
| `PKG-RXPA-id` | default | dynamic + static | template opt matrix | both present |
| `PKG-RXPA-keyaccess` | default | dynamic + static | opt matrix; noopt smoke labelled | both present |
| `PKG-RXPA-llist` | default | dynamic + static | opt matrix | both present |
| `PKG-RXPA-map` | default | dynamic; static target commented | opt matrix with pass expression | dynamic present |
| `PKG-RXPA-matrix` | Windows | dynamic | single runtime test | not selected on macOS |
| `PKG-RXPA-odbc` | `ENABLE_ODBC` | dynamic + static + ODBC | single runtime test | not selected |
| `PKG-RXPA-pick` | commented | dynamic + GTK3 | single runtime test | not selected |
| `PKG-RXPA-pipe` | commented | dynamic; static target commented | template opt matrix | not selected |
| `PKG-RXPA-process` | commented | dynamic; static target commented | template opt matrix | not selected |
| `PKG-RXPA-recv390` | default | dynamic + static | template opt matrix | both present |
| `PKG-RXPA-regex` | commented on Windows | dynamic + external `regex` | single runtime test | not selected |
| `PKG-RXPA-rxmath` | default | dynamic + static | opt matrix; noopt smoke labelled | both present |
| `PKG-RXPA-rxml` | Windows | dynamic | single runtime test | not selected on macOS |
| `PKG-RXPA-rxtcp` | default | dynamic + static | opt matrix with loopback lock | both present |
| `PKG-RXPA-socket` | `CREXX_BUILD_LEGACY_SOCKET_PLUGIN`, default off | dynamic + OpenSSL | template opt matrix with pass expression | not selected; root calls it deprecated and directs users to VM sockets/`rxsocket` |
| `PKG-RXPA-stack` | default | dynamic + static | opt matrix; noopt smoke labelled | both present |
| `PKG-RXPA-strings` | default | dynamic | opt matrix; noopt smoke labelled | dynamic present |
| `PKG-RXPA-system` | default | dynamic + static | opt matrix plus direct smoke | both present; static target is a dependency of first-party tools |
| `PKG-RXPA-testrx` | unreferenced; no CMake | none | none | source directory only |
| `PKG-RXPA-treemap` | default | dynamic + static | template opt matrix | both present |

This accounts for all 28 plugin directories. The current cross-platform default
is broad: 15 directories are selected before platform/feature conditions. It
includes several small demonstrations and legacy alternatives alongside
runtime integrations.

## VM plugin reconciliation

| Catalogue ID | Build forms | Runtime relation | Automated evidence |
|---|---|---|---|
| `PKG-RXVM-mc-decimal` | dynamic, ordinary static, and manual-link static | `mc_decimal_manual` is linked into every normal VM and `crexxsaa`; dynamic loading remains available | static, dynamic, manual-link sanity cases plus a fuller decimal suite |
| `PKG-RXVM-db-decimal` | dynamic and manual-link static | alternative decimal implementation; used explicitly by decimal/archive-link tests, not the normal VM link | unit executable, RXAS decimal selection, and archive-link regression |

Both dynamic modules and static archives are written under `bin` and therefore
fall under the same install sweep. Their product roles are assessed separately
in Stage 6.

## Test-registration reconciliation

`ctest --test-dir cmake-build-debug -N` registered 1,608 tests in the inspected
configuration. Registration was used as evidence; Stage 3 did not treat a
registered test name as proof of semantic completeness.

| Surface | Registered evidence found | Boundary/limitation |
|---|---|---|
| `rxfnsb` | large functional suite compiled/run in noopt and opt modes; a separate performance suite; compiler and VM tests also exercise BIFs | not every exposed helper has a one-to-one named test; performance cases are evidence assets, not current benchmark results |
| classlib | collection/iterator functional cases in opt/noopt and both VMs where applicable; inline-preservation smoke; RexxDoc coverage check | native adapters depend on their plugin tests; coverage is uneven by class |
| `rxfnsg` | linked-image inline smoke plus functional LLM tests | hosted-provider cases depend on environment/network and are not broad deterministic conformance tests |
| `rxfnsc` | dedicated functional suite plus RexxScript/Level C compiler paths | the Classic BIF dispatcher currently implements only a narrow subset of the recognised table |
| `rxfnsl` | linked binary-surface smoke and tiny-expression functional suite | proves the current demonstrator, not a general Level L library |
| RexxScript | runtime and direct evaluator opt/noopt suites, image build checks, CLI smoke, and `crexx` smoke | separate language/runtime product; not proof of Level C conformance |
| RXPP | seven focused tests: smoke, source map, shell mapping/lookup/tokens, diagnostics, and diagnostic catalogues | good tooling-path evidence; not a catalogue of all preprocessor scripts |
| RXPP support files | the RXPP smoke expands and compiles `SQUARE`; token tests identify a macro call | no systematic functional tests were found for the other 63 macro definitions, the six `mathlib` procedures, or empty `syslib` |
| RXDB | one usage smoke executes the packed debugger and checks its CLI help | no automated stepping, watch, module-load, trace-event, or UI-state contract test was found |
| default RXPA plugins | opt/noopt tests registered for arrays, stack, cipher, strings, getpi, keyaccess, llist, treemap, recv390, map, rxmath, rxtcp, and system in the configured tree | predominantly smoke-level; conditional/unreferenced plugins are absent from this configuration |
| compiler exits | compiler suites exercise exits and linked `rxcexits`; certified language exits have focused syntax/runtime coverage | the bundle contains demonstrator exits whose inclusion is not itself evidence of core status |
| VM decimal plugins | dedicated static/dynamic/manual/full and archive-link tests | alternatives have different runtime roles and cannot be classified as one interchangeable contract without further conformance work |

## Documentation reconciliation

- `docs/ai-context/CREXX_LIBS.md` accurately identifies the main linked
  libraries and the possibility of hybrid Rexx/native call paths, but it is not
  a complete product catalogue.
- `docs/release-1-plan.md` already establishes a useful intended boundary:
  algorithms should tend toward Rexx; tightly coupled primitives belong in the
  VM/RXAS layer; plugins are for external integration, OS/application-specific
  surfaces, and experimental work. It also calls for `Id`, `KeyDB`, and `Os` to
  remain outside the core classlib image. Stage 6 applies, but does not silently
  change, that policy.
- `docs/planning/beta-3/issue-candidates.md` explicitly calls for plugin/exit
  inventory and classlib curation, consistent with this catalogue.
- The typed BIF reference names `ARG`, `ADDRESS`, `CONDITION`, `JUSTIFY`,
  `QUEUED`, `SOURCELINE`, `STORAGE`, and `TRACE`, although those names do not
  all correspond to ordinary namespace-exposed typed-library procedures.
  `ARG` is compiler-supported and `TRACE` is documented as reserved in favour
  of the statement. They now have separate `BIF-` entries rather than being
  hidden as apparent source omissions.
- `docs/bifs` contains older/legacy pages that mix typed, Classic, retired, and
  aspirational surfaces. It cannot be used as an authoritative current export
  list without contract-level tagging.
- The compiler has 70 separately catalogued Level C BIF names in its
  recognition table. Recognition is broader than current execution support.
  The `rxfnsc` dispatcher currently implements 19 names (`ABBREV`, `ABS`,
  `COPIES`, `DATATYPE`, `LEFT`, `LENGTH`, `LOWER`, `MAX`, `MIN`, `POS`,
  `RIGHT`, `SIGN`, `SPACE`, `STRIP`, `SUBSTR`, `UPPER`, `VERIFY`, `WORD`, and
  `WORDS`), while compiled Level C lowering currently has proven BIF paths for
  `LENGTH` and `SUBSTR`.

## Discrepancy register

| ID | Reconciled discrepancy | Effect on later stages | Resolution boundary |
|---|---|---|---|
| `D03-01` | Eight typed BIF-reference names are intrinsic, reserved, contextual, or documentation-only rather than ordinary `rxfnsb` exports. | Prevents false missing-source findings; syntax/BIF purpose and role must be explicit. | Classify in Stage 6; documentation cleanup requires later approval. |
| `D03-02` | The old `lib/rxmath` source tree has no CMake entry point, while `lib/plugins/rxmath` is an active default native plugin. | Two unlike implementations share a product name. | Treat as separate source and plugin items; disposition is an approval decision. |
| `D03-03` | `lib/rxfnsb/rxas` contains many deprecated/residual implementations; only `_elapsed` and the ADDRESS bridge are selected. | Filesystem presence otherwise overstates active RXAS content. | Active and residual IDs remain separate. |
| `D03-04` | Fifteen RXPA plugin directories are selected by default, including legacy alternatives and demonstrations. | Current build presence is not a reliable core/quality signal. | Stage 6 proposes roles and delivery independently. |
| `D03-05` | Root install copies the whole build `bin/` tree. | Default build, package contents, and incidental artifacts are coupled. | Record proposed delivery allowlists in Stage 6; no packaging change now. |
| `D03-06` | Several plugin directories have complete local CMake/tests but are commented or unreferenced at the plugin root. | Their local tests are not registered in normal configurations. | Classify as opt-in, developer, source-only, or removal candidates. |
| `D03-07` | The linked library faithfully exposes internal-looking underscore symbols because the sources namespace-expose them. | Public metadata and likely intended internal status disagree. | Keep as public raw contracts; Stage 6 may propose internalisation, never silently delete them. |
| `D03-08` | Level C recognises 70 BIF names, but current runtime dispatch is much narrower. | Parser acceptance cannot be reported as Level C BIF compliance. | Preserve separate recognised/implemented maturity; standards work is later. |
| `D03-09` | G and L libraries are `ALL` targets and swept into install, although their release maturity is still directional. | Current delivery may imply more commitment than the source surface supports. | Stage 6 proposes explicit delivery and maturity. |
| `D03-10` | `classlib_native` is a separate linked image but is also built and installed by default. | Separation of code is not yet separation of product delivery. | Assess its adapters independently from core classlib. |
| `D03-11` | Legacy BIF pages do not map cleanly to current typed exports or unique Level C contracts. | Documentation counts would create duplicates and false coverage. | Use stable typed/Classic IDs; later docs work must tag or retire legacy pages. |
| `D03-12` | `veclib` builds by default outside `bin`, while the old `rxmath` tree does not build at all. | “Built by default” and “shipped by default” differ across residual libraries. | Maintain separate build and delivery fields. |
| `D03-13` | Several native plugins duplicate typed/classlib capabilities: arrays, strings, stacks, lists, maps, trees, regex, file I/O, sockets, and math. | Duplicate contracts increase testing, packaging, and user-choice ambiguity. | Stage 4 groups them by purpose; Stage 6 proposes one role for each implementation. |
| `D03-14` | Compiler exits with language-critical and demonstrator purposes are linked into one `rxcexits` image. | Bundle membership overstates equal product status. | Keep exits as 16 independent catalogue items and propose bundle partitions. |
| `D03-15` | The initial raw sweep covered library/plugin leaves but omitted explicit primary toolchain/runtime products. | A library-only catalogue would not support an Eel1 delivery decision. | Twenty-one tool/runtime/profiling rows were added and reconciled before classification. |
| `D03-16` | RXPP stages and installs 64 macro definitions and six included procedures, but focused tests exercise only a tiny sample and source review finds obvious suspect definitions. | Shipping the files can be mistaken for a supported standard macro library. | Catalogue every leaf; Stage 6 separates the preprocessor engine from convenience/example libraries. |
| `D03-17` | RXDB is built and installed but release documentation labels it experimental; it exposes a handler and UI class while testing only CLI usage. | Install presence overstates debugger maturity and support. | Retain independent tool and public-surface entries; propose an experimental/developer delivery role. |
| `D03-18` | `precomp` exposes 20 RXPA registrations but is an internal mechanical dependency of RXPP rather than a user plugin contract. | A registration-only inventory can incorrectly promote internal helpers to user BIFs. | Keep leaves for compatibility/change control and classify their package as RXPP infrastructure. |
| `D03-19` | `preprocessor/rxppt.crexx` is a large older workbench with machine-specific paths and no active CMake reference. | Its presence can be confused with the maintained RXPP engine. | Keep a separate residual-source identity pending removal/archive approval. |

## Stage 3 conclusion

The raw catalogue reconciles with current source selection and linked metadata.
The remaining issues are product-boundary discrepancies, not unexplained
inventory holes. The late tool, debugger, and macro audit was folded back into
the raw catalogue before classification. The catalogue can therefore be
reorganised by purpose without using build presence as a proxy for core
status.

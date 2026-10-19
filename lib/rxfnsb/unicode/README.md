# Shared B Unicode runtime

This component owns the existing Unicode 17 executors and their pinned raw
Unicode/legacy-codec inputs. `rxunicode_core` builds the independently selected
`bin/rxunicode-core.rxbin`; it does not build or import `rxfnsg`. The minimal
`library.rxbin` remains free of the large prepared Unicode images.

The extraction preserves the existing private `_rxunicode` names, signatures,
selectors, algorithms, image layouts and errors. It introduces no public B
namespace. The G facade in `lib/rxfnsg/rexx/unicode.crexx` keeps its public
procedures, collections, iterators and documentation tags. Classic adapters
remain in `lib/rxfnsc`; migrating them from the G facade to an approved public
B procedural interface is a later contract gate in CLC-STEP03, not completion
of this packaging slice.

| Owner | Maintained inputs and responsibility |
|---|---|
| `rexx/` | Existing B case mapping, folding, codec and grapheme executor templates. |
| `data/unicode-17.0.0/` | Canonical pinned UCD inputs, including independent Level L recognition inputs, provenance and checksums. |
| `data/legacy/` | Canonical TUTOR-compatible mapping sources, provenance and checksums. |
| `tools/unicode/` | Existing table compilers, deterministic source generators, Level L grammar/adapter and the B build controller. |
| `tests/` | Existing RXAS checks for one constant per family, direct codepoint/table access and absence of table/input copies. |
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
  -> five private B executor modules -> rxunicode-core.rxbin
  -> unchanged G facade -> rxfnsg.rxbin
```

Level L generates its small recognition index directly from canonical pinned
raw data and the minimal B library. It never imports the optional core or G
facade, so canonical source-data ownership does not create a bootstrap cycle.
L tools and compiler exits are build prerequisites of the optional component;
programs using its executors need neither compiler nor generation tools.

`UnicodeBuilder.cmake` bootstraps `tools/unicode/build_unicode.crexx`, which owns
the retained U1–U8 dependency waves. The controller reuses the one existing
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
Full semantic qualification continues through the unchanged G facade tests and
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

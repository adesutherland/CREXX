# C RXPA native objects — normal local evidence

Implemented and checked on macOS ARM64 on 2026-09-14. This is uncommitted work
over HEAD `c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8`, retaining the existing
native-inference STEP-03/04 changes. [identity.json](identity.json) records
the relevant source hashes, host and unchanged legacy initializer layout.
The numbered outcomes and acceptance disposition are in the
[completion plan](../../planning/rxpa-native-objects.md).

## Result and changes

1. A sized `object_set_type` host-service tail and `SETOBJECTTYPE` publish a
   loaded concrete class on a C-built value. Native module graphs are created
   lazily and owned by the VM module. Invalid calls leave the value unchanged.
   The legacy `rxpa_initctx` text/layout is identical to HEAD; the installed
   SDK header matches the working tree. Prefix-only/partial-tail hosts,
   unknown versions and null services are covered by the host-service test.
2. SDK binding macros cover default/named factories, matches, concrete methods
   and default interface methods. `METHODPROCEDURE` supplies the initialized
   receiver guard. C factories use only declared arguments; methods receive
   their receiver in `ARG0`. A complete C-only fixture owns its resource through
   the existing native payload copy/finalize hooks.
3. The fixture exposed and repaired concrete integration defects: native class
   factories incorrectly acquired interface-selection graph buckets; the
   existing `final method` metadata spelling was not accepted by graph/runtime
   readers; imported named factories/matches were split into fake namespaces
   when their scopes/parameters were cloned; and native matches/nested methods
   attempted to enter bytecode frames. Existing optimized shapes are retained.
4. Native UTF-8 validation now preserves untouched object initialization flags.
   Nested callback signals have a separate outcome and defer terminal panic
   reporting to the outer caller. Ordinary integer callback results—including
   values wider than C `int`—remain results. C method signals remain signals
   even when their numeric code equals the return value. Outermost uncaught
   panic address/source regressions still pass.

## Before controls

| Evidence | Reproduced failure or positive control |
| --- | --- |
| [before.c](before.c), [before.crexx](before.crexx), [before.log](before.log) | Existing C factory/concrete methods worked, but all three construction routes reported `.object`. |
| [graph-before.log](graph-before.log) | Adding a concrete native factory broke the expected graph bucket count and graph deserialization. |
| [default-before.log](default-before.log), [debug detail](default-before-debug.log) | Existing `final method` spelling was rejected as an unknown member kind. |
| [compiler-before.log](compiler-before.log), [compiler-scope.log](compiler-scope.log) | A minimal imported native named-factory/callback consumer crashed; the scope trace identifies a fake `§factory` namespace. The permanent reduction is `tests/rxpa/rxpa_objects_callback_compile.crexx`. |
| [native-match-before.log](native-match-before.log) | Native factory matching attempted to activate a bytecode frame with no bytecode space. |
| [receiver-before.log](receiver-before.log) | Missing native method guard produced the wrong signal on an uninitialized receiver. |
| [validator-before.log](validator-before.log), [probe source](validator-probe.crexx) | Native post-call text validation cleared an untouched uninitialized receiver/child marker. The expanded host-service unit failed before the repair. |
| [callback-signal-before.log](callback-signal-before.log) | A caught nested Rexx signal still printed a terminal panic. |
| [large-callback-before.log](large-callback-before.log) | A valid 5,000,000,000 callback return was misclassified as failure through the narrower process status. |
| [no-provider-control.log](no-provider-control.log) | Ordinary rxc with the static lane's restricted imports fails `CLASS_NOT_FOUND`; the DECL_ONLY compiler succeeds with those same roots. |

These logs represent their respective implementation stages, not one single
historical clean commit. No sanitizer was used to obtain them.

## Passing checks

| Check | Evidence and boundary |
| --- | --- |
| 44 focused Debug CTests | [debug-focused.log](debug-focused.log), [selection/prep targets](focused-tests.json). Graph, sized host/old-manifest compatibility, class metadata, named/interface factories, callbacks, payload ownership, native unwind, receiver cleanup and outer panic diagnostics. |
| 24 individual native-object Debug CTests | [debug-registered.log](debug-registered.log). Each launches one VM; compilation/assembly/linking occur in the normal QA prep target. Both VMs, optimization modes and delivery forms; worker cases reserve three CTest processors. All passed in 4.73 seconds at parallel 6. |
| Explicit Debug four-tool matrix | [dynamic](debug-dynamic.log), [static rxbvm](debug-static-b.log), [static rxtvm](debug-static-t.log). Three consumers × two optimization modes × two VMs × two delivery forms: 24 executions. Also compiles the minimal scope reduction. |
| Explicit Release four-tool matrix | [dynamic](release-dynamic.log), [static rxbvm](release-static-b.log), [static rxtvm](release-static-t.log): the same 24 executions. Static lanes use a DECL_ONLY rxc, a bin-only import root and an empty provider search directory. |
| Release host/graph units | [release-unit.log](release-unit.log), 2/2 pass. |
| External installed SDK | [configure](installed/02-sdk-configure.log), [build](installed/03-sdk-build.log). C dynamic/static/DECL_ONLY provider and C++ normal/DECL_ONLY macro controls compile against the scratch-installed SDK. |
| Installed and relocated consumers | [installed logs](installed), [executable hashes](installed/packages.json). All three consumers pass both installed VMs and standalone native executables relocated into fresh Unicode/space-containing paths. Only the executable is copied; provider/runtime search environment is cleared. |

The three consumers cover full native objects/callbacks/ownership, class-only
provider retention, and two attached workers. Workers create/use/free their own
objects, have distinct sessions, preserve controller state and destroy both
worker sessions on pool closure. They do not transfer native value handles.

These counts overlap where the explicit matrix and registered CTests exercise
the same contract; they are not independent performance samples. No inference
workload, model benchmark or broad full-suite rerun was performed.

## Reproduce and remaining qualification

Normal Debug regression preparation and execution:

```sh
cmake --build cmake-build-debug --target rxpa_objects_artifacts objects_rxbvm objects_rxtvm --parallel 6
ctest --test-dir cmake-build-debug -L native-objects --parallel 6 --output-on-failure
```

The explicit `tests/rxpa/check_native_objects.cmake` runner takes `BUILD`,
`SOURCE`, `OUTPUT` and optional `RXC`, `VM`, `IMPORT_PATH`, `PROVIDER_PATH`.
Use the retained logs for exact lane commands. The packaging script is
`python3 tests/rxpa/check_native_objects_package.py BUILD SOURCE FRESH_WORK`;
prepare `stage-c1-toolchain`, `stage-product` and `stage-optional` first. Its
fresh directory owns the entire scratch install and SDK/native builds.

Windows/Linux, maintained sanitizer runs and full-product qualification remain
native-inference STEP-06. SAN-009 stays open and release-blocking, owned by Codex
under Adrian's direction. The explicit nested matrix/package runners remain
unregistered aggregates until isolated Debug/sanitizer scheduling evidence is
available. The individual VM regressions above are registered now. This record
does not claim sanitizer-clean, cross-platform complete, released, or completion
of the typed llama API.

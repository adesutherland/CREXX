# CLion clean rebuild repair — 10 October 2026

Local repair of CP-C-FIND-04 is complete on `develop`, unpublished. The
[authoritative plan](../../ROADMAP.md#proposed-c-first-extraction-phase--2026-10-10)
records its three verified criteria. This follows the local C split commit
`cabab2553`; the compiler/metadata implementation and benchmark sources are
unchanged. [clion-clean-rebuild.json](clion-clean-rebuild.json) records the one
build-input delta, commands, log hashes, build-profile and binary identities.

CLion cleaned both existing profiles and its Debug `all` build at 32 jobs failed
four RexxCPS compilations at `TRACE VALUE`, with `CLASS_NOT_FOUND`. The TRACE
exit emits a helper that uses RexxValue from `rxfnsc.rxbin`. At failure,
`library.rxbin`, `classlib.rxbin` and `rxcexits.rxbin` existed while that runtime
publication was absent. Both RexxCPS registrations depended only on the ordinary
library and compiler-exit bundle. The omission predates the C split; previously
present artifacts masked it during incremental qualification.

The minimal unchanged Level B source reproduces the failure:

```rexx
options levelb
tracevar = "off"
trace value tracevar
say "TRACE-VALUE-OK"
```

Its no-opt compilation returns 2 before runtime publication and 0 after building
only `rxfnsc`. The [original diagnostic](clion-trace-value-original.log) is
retained. This is the expected missing-class error when a required provider is
absent, not evidence of a metadata-format or language-interface change.

The two owning benchmark registrations now declare both `rxfnsc` and its
published output as prerequisites. The compiler-exit bootstrap dependency is
unchanged. Verification removes only the just-built runtime publication, then
builds all four fixture targets at 32 jobs. The generated graph has both edges;
the runtime publication finishes before every fixture compilation, and all four
commands pass.

| Check | Result |
| --- | --- |
| Missing-publication producer ordering | Pass, runtime publication precedes all four consumers |
| Existing CLion Debug `all`, 32 jobs | Pass, resumed from the user's clean-build failure |
| Existing CLion Release `all`, 8 jobs | Pass, rebuilt from the user's clean Release tree |
| RexxCPS ordinary/opaque opt/no-opt and decimal-register controls | **5/5 pass**, 14.41 seconds |

The controls use existing runtime and register assertions. No benchmark workload
or production performance implementation changed; no performance verdict is
claimed. The earlier compiler correctness, package and ownership evidence is
reused because its source inputs are unchanged. This repair changes only
`tests/benchmarks/CMakeLists.txt` among the previous 6,976 qualified code/test/build
inputs. It restores CP-C-AC-05's clean-product build proof.

SAN-011, SAN-012 and SAN-QA-018 retain their explicitly assigned **Codex / next
Release 1 release-QA gate** full closure. They remain open and release-blocking;
this build repair adds no sanitizer-clean or release-ready claim. Metadata
formats, serialized identities, callable ABI and language semantics are unchanged.

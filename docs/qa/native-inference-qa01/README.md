# NI-S4-QA01 repair and native factory-shim cleanup — 15 September 2026

Status: NI-S4-QA01 and the separately identified NI-S4-QA02 build defect are
repaired and locally qualified. All 2,347 ordinary Debug tests have passing
evidence from the full run plus affected rechecks. No sanitizer or inference
performance run is part of this follow-up. The full outcomes and acceptance
criteria remain in [STEP-04](../../planning/native-inference-step-04.md).

## Compiler regression

The committed baseline is `6a3306655079` on `develop`. Its existing HTTP
`request`/`get` consumers fail with `RETURNS_VOID` and `RETVAL_MISSING` during
QA preparation. [The original baseline](../native-inference-baseline/README.md)
retains that failed attempt; it is not rewritten as a successful run.

The ten-line [minimal HTTP consumer](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/minimal_http.crexx) reproduces the failure
without executing a network request. The older compiler from the retained
S3-D01 native package compiles both old and current `rxfnsg` libraries. The
current compiler rejects both. [Cross-input identities](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/cross-inputs.json) and
the four `old-*` / `new-*` logs exclude stale library generation as the cause.

Controlled compiler variants replace one archive member at a time, retaining
all other current objects, headers, libraries and link flags. Restoring the
pre-Step-04 `rxcp_ast_core.c` makes the reduction compile; restoring only
`rxcpsymb.c` reproduces the earlier invalid-scope internal error. See
[the variant script](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/compiler_variants.py) and [results](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/variants-corrected.log).
The successful original named-factory repair must therefore be preserved.

The new factory reconstruction changes the order in which imported declarations
become available. Task lowering sees a callable definition while the call's
return type is still `TP_UNKNOWN`. The temporary
[type trace](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/task-lowering-type-trace.log) identifies this exact rejection;
the [AST excerpt](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/partial-block-excerpt.log) shows a partial block with scope
setup and captured arguments but no final `LEAVE WITH`. The large original
diagnostic remains identified by [SHA256](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/trace-identity.json); it contains
invalid UTF-8 diagnostic bytes, replaced only in the readable excerpt.

`rxcp_task_lower.c` now waits for task and ordinary call types to resolve
before changing an implicit expression or explicit parallel block. Existing
factory reconstruction, task transfer rules and optimizations remain intact.
The permanent imported-task fixture gains an implicit object-return call through
a named-factory receiver, alongside its existing local/process parallel blocks.
All four existing tests failed before the fix and passed afterward, exercising
compiler, assembler, linker and both VMs in both optimization modes. See
[before](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/regression-before.log) and [after](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/regression-after.log).

## Removing the construction workaround

Adrian additionally requested removal of Rexx factory wrappers introduced
because native object construction was unavailable. `rxstats.linearfit` is the
concrete native-result carrier found by the review. Its factory and accessors
now belong to `rxstats.c`, using the checked C object-publication service.
`statsvalue.crexx` and its CMake and Level B controller entries are removed.
Factory spelling, private two-double representation and numeric calculations
are preserved. The constructor/accessor RexxDoc descriptions and parameter/
return tags move with the implementation.

Native regression results now carry the same concrete class identity as factory
results. The permanent stats tests check identity through `.object`, casts,
independent copies and uninitialized access. All four positive runtime variants
failed the new identity check before migration; the existing packed-int negative
control passed. See [before](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/stats-before.log) and the
[initial focused pass](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/stats-after-initial.log). Final qualification uses the
rebuilt `rxfnsg` without the former carrier. The installed SDK consumer also
checks native/factory identity and accessors through dynamic and native delivery.

Scalar statistical operations and immutable accessors remain process-reentrant.
Factory/regression publication obtains the owning VM's host through the existing
session-aware RXPA path. No new resource registry or statistical algorithm is
introduced. The accumulation kernels were compared byte-for-byte with HEAD.

`llama` already publishes its entire typed surface from C; no duplicate Rexx
facade existed there. Other reviewed Rexx adapters retain distinct behavior:
Id/Os convenience contracts, KeyDB lifecycle policy, HTTP/task contracts,
SQLite ADDRESS handling, and packed primitive ownership. Their use of native
calls alone is not a reason to remove them. Current human and agent guides now
show the C bindings, and the old native-surface review is labelled historical.

## QA preparation and test corrections

The first full run completed in **829.61 seconds: 2,337 passed, nine failed,
one timed out**. [Its complete log](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/full-debug-ctest.log) is retained unchanged.
The failures were resolved as follows:

| Finding | Correction and evidence |
| --- | --- |
| Two ADDRESS parser goldens | Retained final executable ASTs are byte-identical to the old goldens; refreshed symbol tables now use literal named-factory symbols instead of an invalid namespace. One needless convergence iteration disappears. Both pass in [`affected-ctest.log`](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/affected-ctest.log). |
| Stats concurrency policy | The harness now checks six reentrant procedures and two session-affine publication procedures against the V2 manifest, then retains the real overlapping scalar calls and context destruction. |
| Six llama lifecycle tests | The llama directory omitted QA-tier finalization, leaving its old lifecycle binary outside `qa-prep`. Registering the already declared prerequisites rebuilds the harness against the current bridge layout. The missing explicit `<cstring>` include is added. All eight llama checks pass, including the toolchain consumer. |
| Signature aggregate timeout | The unchanged test passes in isolation in **67.53 seconds**. Set `RUN_SERIAL` and keep its 300-second timeout. The test body/assertions are unchanged; Step 6 retains sanitizer scheduling proof. |
| Two session harness checks exposed after relinking | Look up the probe by plugin identity rather than assuming it is the only/first session. The added stats session is legitimate. Distinct probe IDs, overlap, create/destroy balance and malformed-manifest rejection remain checked; the latter also asserts legacy dispatch. All 30 shared-harness tests pass. |

The 40-test affected replay first passed 38 and exposed the two session-list
assumptions; that failed replay is retained, followed by the successful
[30-test replay](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/session-harness-final.log). No expected-failure annotation,
weakened product assertion, disabled test or sanitizer suppression was added.

## NI-S4-QA02: static archive changes must relink consumers

This distinct build defect explains why the old static harness initially passed
with the previous stats implementation. Unix whole-archive flags embedded the
archive path, but supplied no file dependency to trigger relinking when that
archive changed. The final QA preparation rebuilt `rxstats_static.a` without
relinking the harness. Editing its test source later forced that link.

The installed SDK regression first builds declaration, static and relative
helper consumers returning 41, then changes only the archive source to return
42. All three old executables retain 41 after rebuilding; see
[before values](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/incremental-before-values.json),
[failed test](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/incremental-before-valid.log), and
[commands](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/incremental-before-commands.log). The permanent check lives in
`tests/rxpa/external_sdk/sdk_incremental_link.cmake` and the existing SDK test;
this adds a small C-only incremental control, not a new scenario aggregate.

All three helpers now declare target ordering and an archive `LINK_DEPENDS`.
Existing linker flags and MSVC init-symbol behavior are unchanged. The
[generated dependency graph](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/static-link-dependency-graph.log) includes
`rxstats_static.a` as a relink input as well as an ordering dependency.
The [final QA rebuild](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/incremental-qa-prep.log) required no further executable
or object rebuilds: the affected static harness had already been relinked and
checked. The full installed SDK check passes in **30.52 seconds**, including
all three incremental controls, C/C++ providers, dynamic/native stats identity
and both VM/optimization modes. See [result](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/incremental-sdk-final.log) and
[complete commands](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/incremental-final-commands.log). Windows/Linux verification
remains in Step 6.

## Final qualification

The full command was:

```sh
cmake --build cmake-build-debug --target qa-prep --parallel 6
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure -LE performance-measurement
```

The [coverage ledger](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/coverage-ledger.json) assigns each of **2,347 selected
tests** its final passing evidence: **2,305 unaffected full-run passes plus
42 distinct tests covered by subsequent rechecks**. This is combined evidence,
not a claim that the original full invocation was green. The selection remains
identical, with zero disabled tests and the existing 186 performance
measurements excluded. All original full-run failures and subsequent failures
are retained, classified and superseded by passing affected checks.

Host: Apple M5, 10 logical CPUs, Darwin 25.6.0 arm64, CMake 4.3.2. The
[starting identity](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/source-identity.json) and [`source-inputs.json.gz`](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/source-inputs.json.gz) record
7,399 tracked non-Markdown inputs outside documentation and retained performance
evidence. [Verification](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/broad-input-verification.json) confirms they were
unchanged throughout the broad run. The [completed identity](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/completed-source-identity.json)
and [`completed-source-inputs.json.gz`](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/completed-source-inputs.json.gz) capture the final 7,400 inputs, including
the new SDK fixture, and enumerate every post-broad input change. Compiler and
runtime C/C++ sources are unchanged since the broad run; later changes are test
expectations/harnesses and QA/link dependency configuration. Those dependencies
and all affected tests were rechecked, so unchanged broad passes are reused.
[`SHA256SUMS`](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-qa01/SHA256SUMS) identifies the retained artifacts.

QA01-AC-01–05, QA01-01–05, QA02-AC-01–03 and S4-B-02 are ticked with this
ordinary local evidence. STEP-06 still owns sanitizer and platform qualification,
including open, release-blocking SAN-009 under Codex/Adrian. This cannot close
Windows/Linux/CUDA/Vulkan, exact-head hosted publication or release readiness.
Generation remains STEP-05. No push, user-prefix install or model benchmarking
was performed; the SDK check installs only into its isolated QA prefix.


## Local reviewable commits

- `6054bad3888a85934b1e662e0bdd55ff88385c47`: QA01 compiler/task-lowering repair
  and permanent regression/goldens.
- `c328c6d37ca98d23b883993dcc72e3f010cf079c`: QA02 archive relink dependencies
  and the permanent installed SDK incremental control.
- The enclosing factory-cleanup/evidence commit contains the C stats carrier,
  obsolete source/build removal, QA preparation/harness corrections and current
  human/agent documentation. These commits are local; nothing is pushed.

The completed source manifest identifies the tested final code independently
of commit grouping. Only history/documentation changed after that capture;
unchanged ordinary test evidence is reused under the repository rule.

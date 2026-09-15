# S3-D01 Release decision and local qualification — 14 September 2026

The [approved repair and acceptance record](../../planning/native-inference-worker-transition-proposal.md)
owns scope and status. This is an uncommitted implementation on baseline
`c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8`. Adrian accepted the first Release
result after clarification that its cost is per legacy callback, and authorized
the remaining native/package and broader Debug qualification. Core implementation
and measured Release inputs remain unchanged. The STEP-03 provider work remains
separate uncommitted work. **No sanitizer build/test was run after Adrian's hold;
that hold and SAN-009's open status remain in force.**

- `debug-before-repair-*.log`: both permanent startup controls time out on the
  original implementation, before the coordinator/executor repair.
- `debug-build.log`, `debug-tests.log`: 14/14 focused controls pass, including
  attached startup, direct/locked callback rejection, nested execution, failed
  worker startup/recovery, cold transition, serialization and session policies.
- `native-positive-*.log`, `native-positive-dependencies.txt`: the original
  native cREXX worker transport program builds and passes, with no llama/ggml
  runtime dependency. Its earlier timeout remains in the STEP-03 evidence.
- `release-baseline-identity.json`, `candidate-identity.json`: exact source,
  artifact and configuration hashes. The baseline VM was rebuilt before core
  edits and retained separately; baseline and candidate use identical current
  compiled workload, plugin and library bytes. `implementation.patch` separates
  the tracked interpreter changes; the new startup C test is retained in source.
- `manifest.txt`, `timing/`: serial, pairwise-balanced ordinary Release capture
  with one warmup and 12 recorded samples per cell. All 78 executions pass.
  Product `rxvm` selects `rxbvm` on this compiler. Every output/sample is retained;
  no outlier was removed. These three RXPA paths are one mechanism experiment,
  not a representative portfolio or RexxCPS/inference performance claim.
- `paired-summary.csv`, `summarize_paired.crexx`: paired changes and t intervals.
  The Level B reducer adapts the retained E3b-P2 reducer to control/candidate
  labels and a 3% point-estimate guard. Its inherited loop-binding warning is
  retained in `release-summary.log`; all 12-pair/count/output checks pass.
- `host-before.txt`, `host-after.txt`: Apple M5, 24 GiB, 10 logical CPUs, macOS
  arm64, AC power, low-power mode off, no reported thermal/performance warning.

The capture tool's version string comes from the older executable hosting that
tool; it is not the identity of the measured VM. The measured binaries and
source/configuration identities are recorded independently in the JSON files.

| Workload | Mean paired elapsed-time change | 95% interval |
| --- | --- | --- |
| Legacy calls | +2.846764% | +1.454974% to +4.238554% |
| Reentrant control | -0.529833% | -1.942412% to +0.882747% |
| Session-affine control | -0.400618% | -2.403657% to +1.602422% |

Legacy overhead is measurable. Its point estimate is below 3%, but the interval
crosses that guard; Adrian explicitly accepted this cost. The other paths have no
statistically clear change. The legacy difference of medians is 35.3535 ms over
20 million calls (about 1.77 ns/call in this workload).

Focused Debug command (after building the named C test targets and fixture):

```sh
ctest --test-dir cmake-build-debug --parallel 1 --output-on-failure \
  -R '^rxpa_(worker_startup_.*|static_catalogue_replay|static_session_lifecycle|session_factory_failure_rollback|session_manifest_fail_closed|float_static_alias_binding|legacy_call_serialization|process_reentrant_overlap|legacy_recursive_reentry|branch_free_load_binding|bound_legacy_serialization|legacy_transition_quiescence)$'
```

The original native control uses `crexx --program SCRATCH/positive
tests/native-inference/worker_transport_positive.crexx --jobs 1 --native`, then
runs that executable from the scratch directory. It needs the matching Debug
driver, native runtime and standard library. No model is required.

The Release capture uses the existing Level B
`performance/tools/run_cross_runtime_matrix.crexx` with `--manifest manifest.txt
--output-dir timing --measurement timing --warmups 1 --runs 12`; the exact paths
are in the manifest. Run the retained Level B reducer with the paired-summary
output path and `timing/samples.csv`. The source/test inputs remain versioned;
baseline binary paths describe this session's scratch artifacts.

## Qualification after acceptance

The expanded `rxllama_package_measure` target passed in **114.687 seconds** in
normal Debug, using a scratch install with spaces and Unicode in its paths.
`native-package/` retains its install/manifest/consumer logs, individual control
logs, source/binary identities and runtime manifests. In addition to the existing
relocation, dependency/cache, missing-backend and dynamic-worker controls, it
now builds and runs native transport, native foreign-handle rejection and four
native cREXX workers sharing each BGE/Smol model with private preparation on CPU
and Metal. Native consumers execute in a clean loader environment outside the
checkout. These are lifecycle/package controls, not embedding/generation request
or throughput qualification.

The package matrix remains an explicit build target. Its expanded workload needs
a new isolated maintained-sanitizer measurement before CTest registration; the
old 175.305-second sanitizer measurement covers only the earlier matrix and VM.

Broader normal Debug qualification passed **2,314/2,314 in 900.23 seconds** after
`qa-prep`, with no failures or timeouts. `debug-qa-prep.log` and
`debug-full-ctest.log` retain preparation and the complete CTest output;
`post-approval-identity.json` records current source/build identities and commands.
The broad command excludes separately governed performance measurements:

```sh
cmake --build cmake-build-debug --target qa-prep --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure -LE performance-measurement
```

The accepted Release result is reused; all 30 source/artifact/control hashes in
`candidate-identity.json` still match. No measurement rerun or tuning was needed. The repair
is not phase/release closure: new sanitizer and remaining platform qualification
are open. Adrian subsequently approved STEP-03 closure and assigned SAN-009,
S3-D01 and the expanded package matrix to STEP-06 native-inference release QA,
owned by Codex under his direction. This closes the bounded implementation phase,
not the outstanding sanitizer criteria. Checksums cover retained files except
SHA256SUMS.

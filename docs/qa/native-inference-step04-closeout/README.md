# STEP-04 functional closeout evidence — 14 September 2026

Adrian accepted the indicative Release overhead/variation and continuation of
STEP-04. NI-S4-P01 is an accepted observation with unresolved cause. This bundle
adds normal local functional evidence without changing production inference
code, vector representation, upstream settings or the accepted timing samples.

## Passed controls

1. **Request boundaries, CPU and Metal:** the expanded `embedding_bridge.cpp`
   oracle passes exact aggregate row/token/byte limits and just-over-limit
   rejection, including query-prefix byte accounting and query inputs at exactly
   512 and 513 tokens. It also checks wrong resource kind, zero work budget,
   mutation after submit, repeated processing without another decode, late and
   repeated cancellation, failed-batch nonpublication, immediate session reuse,
   recovery and reservation cleanup. Existing complete-string, reference-vector,
   Unicode/empty/NUL, independent-session and layout checks remain passing.
2. **CPU/Metal parity:** six query/document cells at layouts one/four/eight pass
   the unchanged maximum-coordinate `0.002` and minimum-cosine `0.9999`
   integration tripwires. The required-GPU model reports actual offload.
3. **Four real cREXX workers:** each runs 20 eight-row batches with distinct
   input text, one shared model allocation and private sessions/results. Every
   row matches that worker's isolated baseline at absolute difference `0.00001`
   and cosine `0.999999`. Output passes existing rxvector f32le encode/decode
   and cosine consumption. All requests close, each worker counts exactly 20
   completions, the model loads once, and worker ownership returns to the sole
   parent before final reservation cleanup.
4. **Four-tool matrix:** persistent embedding acceptance and the worker test
   both pass on CPU/Metal, compiled with normal optimization and `rxc -n`, through
   rxc/rxas/rxlink and both rxbvm/rxtvm. Eight cells, two VMs each: 16 consumer
   runs. Each ordinary embedding acceptance includes 100 singles and 20 batches.
5. **Scratch installation/native consumers:** a new disposable Debug installation
   compiles copied source outside the checkout using installed import roots.
   Both programs pass on CPU/Metal with both installed VMs (eight runs), and
   their native executables pass after relocation with only declared packaged
   runtime dependencies (four runs). Runtime environment overrides are removed.
   Model bytes remain separate local read-only data; no model download or server
   is used. Unicode/space-containing package paths are included.

The three C++ oracle invocations and 28 interpreted/native consumer invocations
are normal Debug checks on Apple M5 / Darwin arm64, not a platform or sanitizer
qualification. No new aggregate CTest is registered; these runners remain
explicit until STEP-06 establishes sanitizer scheduling properties.

## Inputs and reproduction

Baseline HEAD is `c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8` on `develop`, with
the existing uncommitted implementation. `identity.json` records source,
production and test artifact identities, and the moved generated test images.
The accepted Release production binaries remain unchanged. The model is the
same pinned BGE small EN v1.5 F16 artifact and the engine is the same pinned
llama.cpp revision recorded in the STEP-01 plan. `install-workspace.txt` names
the retained disposable install and consumer directory; `packages.json` records
relocated dependency hashes. Installed command logs are retained in `installed/`.

Build `rxllama_embedding_bridge` in normal Debug, then invoke it as
`rxllama_embedding_bridge MODE MODEL SHA256`, where MODE is `cpu`,
`required-gpu`, or `cross-device`. See `debug-build.log` and `debug-*.log`.

The explicit four-tool runner is `tests/native-inference/step04_toolchain.cmake`.
Supply BUILD, SOURCE, fresh OUTPUT, MODE, MODEL and HASH; optional PROGRAM selects
`embedding_acceptance` or `shared_worker_embeddings`, MARKER specifies the
expected success marker, and NO_OPT=ON selects `rxc -n`. Each retained
`toolchain.log` records exact compiler/assembler/linker/VM commands and results.
The earlier successful CPU optimized worker cell is in `worker-cpu-opt/`; the
other seven are indexed by `debug-four-tool-matrix.log`.

Run `tests/native-inference/step04_package_consumer.py INSTALL SOURCE MODELS
FRESH_WORK cpu,required-gpu` against a scratch installation. The final argument
is an explicit hardware selection, not an automatic assumption about a host's
GPU availability. This script is a QA harness only; Python is not an inference
product dependency.

Initial new-harness errors are retained separately: an omitted rxfloat import,
then omitted Level-G/class library link inputs caused unresolved factory
autoload at worker startup. The harness now explicitly links `library`,
`classlib` and `rxfnsg`, following the installed driver's library inputs and the
documented distinction between compiler import roots and runtime module roots.
These were test-construction errors; no compiler, VM or provider repair was
needed, and no failing product control was weakened.

## Remaining scope

S4-05a and the existing-native-surface portion of S4-05b now have local evidence.
The typed public facade and persistent public examples remain S4-05c, with the
concrete [S4-D03 proposal](../../planning/native-inference-typed-interface-proposal.md)
ready for the public naming/return-contract review reserved in STEP-01. Its own
ordinary preimplementation controls, wrapper implementation and installed/native
consumer validation remain open. STEP-04 is not yet declared complete.

SAN-009 remains open/release-blocking; its assigned STEP-06 owner is Codex under
Adrian's direction. S3-D01, S4-D01 and the expanded request/worker/native matrix
retain their STEP-06 sanitizer/platform obligations. No sanitizer, full Debug
suite, performance rerun, commit, push or STEP-05 implementation occurred.


## Subsequent typed completion

The remaining-scope section above describes this bundle's capture point.
[S4-D03's later completion evidence](../native-inference-typed/README.md) now
records the typed API, installed persistent examples and their own Debug/Release
native delivery checks. Use the live STEP-04 checklist for current disposition;
this historical native evidence has not been rewritten as later qualification.

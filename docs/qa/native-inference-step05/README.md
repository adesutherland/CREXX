# STEP-05 generation qualification — 15 September 2026

Status: STEP-05 normal local closeout complete after Adrian accepted the first
Release verdict. Authority: [STEP-05](../../planning/native-inference-step-05.md) and the
[parent plan](../../planning/native-inference-backlog.md). These phase controls do
not close the initial platform matrix or authorize publication.

## Delivered behavior

The optional llama.rexx provider now prepares private SmolLM2 generation contexts
once, accepts repeated single/batched prompts, performs bounded prefill/decode,
and returns owned incremental UTF-8 chunks, token deltas and finish reasons.
Typed C RXPA and low-level calls use the same shared-model/private-session state.
No Rexx factory or conversion wrapper was added. S5-D01's optional sized host
service copies counted UTF-8, including U+0000, validates and counts codepoints,
and preserves its destination on invalid input. Callers provide byte lengths;
no caller padding or global five-NUL storage change is required.

The original embedding-session RexxDoc block remains attached to its public C
method; new generation factories/methods and both examples have source contracts.
The first-verdict production logic is unchanged during closeout. Only that
comment attachment and the provider README differ within the frozen production
file set; test/example and human/agent documentation additions complete this phase.

## Coverage ledger

| Criterion | Evidence |
| --- | --- |
| S5-AC-01 | CPU/Metal independent direct-library parity: exact sampled token IDs, full text, ordered rows and finish reasons; single/four-row requests, repeated prepared use. Minimum evidence in the accepted Release bundle, plus final minimum controls here. |
| S5-AC-02 | Exact input byte/row/aggregate-token limits; 480 prompt + 32 output fits a 512-token sequence and 481 + 32 rejects before compute. Invalid UTF-8/state/row/capability/configuration, empty batch and output-token cap controls. Generated-byte overflow preserves a valid prefix, reports error and recovers. Split/invalid output scalar controls are model-independent. Generic host-service controls preserve embedded NUL, ownership, aliasing and old-table safety. |
| S5-AC-03 | Debug and ordinary Release CPU/Metal: 100 singles + 20 four-row batches; 1/2/4 concurrent VM owners, each 20 batches, same allocation identity and private ordered outputs; co-resident BGE/Smol processing; zero retained RSS growth. Separate direct/bridge/cREXX one/four-context memory checks. Preparation close boundaries, building/prefill/active-decode cancellation and recovery. Existing model-load drain controls remain in broad CTest. |
| S5-AC-04 | [First Release verdict](../../../performance/evidence/2026-09-15-ni-s5-first-release/README.md) accepted by Adrian: 104 correct processes. CPU mean paired differences -0.00%/+2.66%; Metal -16.82%/-2.52% for one/four rows. No material positive Metal recurrence; no model tuning, upstream repair or timing replay. |
| S5-AC-05 | All 24 additional Debug, 32 installed Release VM and 16 relocated native CPU/Metal executions pass, across both optimization modes. Eight native programs built. Raw commands and package hashes retained. |
| S5-AC-06 | All 2,347 selected ordinary Debug CTests pass in one fresh broad run (911.92 seconds), after final QA preparation. Parent disposition and numbered criteria reconciled. Sanitizer and remaining platform/publication gates stay with STEP-06. |

The full CTest result is retained in `full-debug.log`, detailed output in
`LastTest.log.gz`, and selected test names/scheduling in `full-debug-selection.json`.
No test was disabled. The accepted minimum sixteen public executions remain in
the first Release bundle. `public-closeout-remainder/results.json` accounts for
all 24 additional cases through nine retained passes, one unchanged isolated
replay and fourteen remaining executions. `installed/counts.json` accounts for
all 32 VM / 16 native executions and eight native builds.

`identity.json` records the baseline SHA, final working sources, host/build
configuration, tool/provider/image hashes and absence of product artifact drift.
It also proves the only frozen production-code-file change since the accepted
verdict is relocation of an unchanged RexxDoc block; the other changed file in
that set is the README. Added native assertions were independently checked after
the sustained run; unchanged sustained coverage was reused. `SHA256SUMS` protects
the complete retained bundle.

## Memory and responsiveness

Normal M5 process RSS was sampled after warm-up and through the unchanged
100-single/20-batch workload. All four Debug/Release CPU/Metal runs stayed within
the unchanged 32 MiB growth allowance, with maximum measured growth zero.
Release co-resident BGE plus Smol controls reserved 1,677,721,600 bytes and peaked
at 2,714,615,808 bytes on CPU / 1,938,653,184 bytes on Metal, below 4 GiB.
These parity processes intentionally load separate direct/provider Smol weights;
the following isolated processes compare one shared allocation fairly.

| Mode / private contexts | Direct peak RSS | Bridge peak RSS | cREXX VM peak RSS | Allowed excess over direct |
| --- | ---: | ---: | ---: | ---: |
| CPU / 1 | 1,041,186,816 | 1,040,728,064 | 1,040,171,008 | 83,886,080 |
| CPU / 4 | 1,575,944,192 | 1,575,649,280 | 1,574,371,328 | 134,217,728 |
| Metal / 1 | 656,998,400 | 656,998,400 | 656,326,656 | 83,886,080 |
| Metal / 4 | 1,179,975,680 | 1,178,550,272 | 1,178,435,584 | 134,217,728 |

All 12 memory processes passed fixed text/token/finish assertions; direct/bridge
sampled-token signatures also matched. Peaks are individual normal-process
observations, not statistical claims or discrete-GPU VRAM qualification. The
allowance remains 64 MiB per process plus 16 MiB per active session.

The first new Debug qualifier incorrectly enforced the 250 ms Release target;
it stopped at 299.512 ms. STEP-02 gates only ordinary Release, snapshots isolated
work before concurrent jobs, and reports concurrency separately. The harness
now follows that established policy, without lowering token/row/context/thread
limits or changing production logic. The failed log remains retained.

Release isolated maximum process calls were 133.932 ms CPU / 87.1309 ms Metal,
passing 250 ms. Concurrent maxima for 1/2/4 owners were CPU
137.219 / 197.395 / 2,640.34 ms and Metal 83.2419 / 179.148 / 461.575 ms.
A token work budget is not a wall-clock scheduling or GPU preemption deadline.
Applications choose owner counts and native thread counts together. This is
concurrent latency evidence, not a matched generation-glue slowdown or a reason
to reopen the accepted performance panel.

## Fixture correction

The expanded low-level consumer initially failed its post-loop cancellation and
completion assertions. Its first assignment to `state` was inside the loop,
so the later read referred to a different binding. The compiler already emitted
`#NOT_IN_SAME_SCOPE`. Initializing `state` before the loop repairs the fixture;
both assertions and all repeated workloads remain unchanged. The original
compiler warning and failed semantic output are retained.

## Functional matrix scheduling

The first overlapping Debug/package run reached the shared example's 120-second
task-scope deadline in the optimized CPU `rxtvm` case. The same linked image and
arguments then passed in isolation in 63.093 seconds, including every one of the
four workers' twenty batches and all private-output/shared-allocation assertions.
No product code, workload, assertion or deadline changed. Remaining matrices run
sequentially; the partial passes and isolated replay are reused instead of
restarting the whole matrix. This is a QA scheduling correction, with the failed
output retained, not a new unexplained generation-glue performance acceptance.

## Reproduction

Use the pinned source/model identities from
[STEP-01](../../planning/native-inference-step-01.md). On this M5, hardware modes
are `cpu` and `required-gpu` (Metal). A GPU result must have actual offloaded layers.

```sh
cmake --build cmake-build-debug --target qa-prep --parallel 6
cmake --build BUILD --target rxllama_generation_bridge \
  llama_provider_runtime_package crexx-provider-package --parallel 4
BUILD/lib/plugins/llama/tests/rxllama_generation_bridge MODE SMOL SMOL_HASH BGE BGE_HASH
BUILD/lib/plugins/llama/tests/rxllama_generation_bridge MODE SMOL SMOL_HASH --output-boundary
BUILD/lib/plugins/llama/tests/rxllama_generation_bridge MODE SMOL SMOL_HASH
python3 tests/native-inference/generation_toolchain.py \
  BUILD SOURCE MODELS FRESH_WORK cpu,required-gpu --closeout
cmake --build RELEASE_BUILD --target stage-c1-toolchain stage-product stage-optional \
  llama_provider_runtime_package crexx-provider-package --parallel 4
cmake --install RELEASE_BUILD --prefix SCRATCH_PREFIX
python3 tests/native-inference/generation_package_consumer.py \
  SCRATCH_PREFIX SOURCE MODELS FRESH_WORK cpu,required-gpu
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure -LE performance-measurement
```

The default `generation_toolchain.py` matrix is the already retained minimum
16 executions; `--closeout` runs the 24 new/changed consumers instead of repeating
that unchanged evidence. The package consumer separately proves installed
imports and relocated native dependencies. It copies source outside the checkout,
copies only executables and declared runtime files to paths with spaces/Unicode,
and removes provider/backend/library overrides for execution. Python is QA
orchestration, never an inference runtime dependency or performance reducer.

For separate memory controls use ordinary Release
`rxllama_generation_bridge --memory-direct|--memory-provider MODE SMOL HASH 1|4`.
Compile/assemble/link `generation_memory.crexx` with the same Release tools and
run it through the selected `rxbvm` with `MODE SMOL HASH 1|4`; the retained Mac
commands use `/usr/bin/time -l` for VM peak RSS. Match text/token/finish identities
and compare actual peaks with the unchanged allowance. Keep these processes
serial and separate from compilation, broad QA and concurrent functional matrices.

The package runner gained Windows `.exe` path handling during this macOS run;
its macOS command arguments are unchanged. The captured driver identity and final
source identity are distinguished in the ledger. The shared C++ fixture also
sets `NOMINMAX` before Windows headers. Neither is Windows qualification.

## Remaining qualification

No sanitizer ran. SAN-009 remains open and release blocking, assigned to Codex
under Adrian at STEP-06 along with S3-D01/S4-D01/S5-D01, generic C RXPA object and
provider lifecycle proof. Windows/Linux/CUDA/Vulkan, supported-platform leak
checks, low-memory/driver failure cells, complete provisioning provenance and
exact-head hosted publication gates remain open. Before using the new aggregate
in STEP-06, apply the existing S2-QA01 live-allocator accounting to instrumented
memory checks, keep sanitizer timing diagnostic, and establish normal/sanitizer
scheduling. Normal RSS limits must remain unchanged. STEP-07 may overlap unavailable
hardware proof. This local phase is not a sanitizer-clean or release-ready claim.

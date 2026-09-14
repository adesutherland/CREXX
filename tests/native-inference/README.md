# Native inference acceptance controls

These are STEP-02 controls for the approved
[llama.rexx plan](../../docs/planning/native-inference-backlog.md).
The [STEP-02 record](../../docs/planning/native-inference-step-02.md) defines
thresholds and current gaps. Nothing here installs or implements `rxllama`.

`direct_control.cpp` is a native test workload calling pinned upstream APIs.
It uses one model with private contexts, checks real text/token outputs,
repetition, batching, concurrent contexts, token boundaries, bounded retained
memory, active cancellation/recovery and cancelled or malformed loads. Its
`--co-resident` mode measures both models together, with one/two/four private
contexts per model and close/reload cycles. It records API-boundary wall time
including GPU completion, separately from upstream diagnostic compute counters
and complete-process time. Performance orchestration uses the existing Level B
`performance/tools/run_cross_runtime.crexx` runner and the Level B
`performance/tools/summarize_native_inference.crexx` reducer.

## Reproduce the local control build

Obtain the source/model bytes in the
[pin record](../../docs/planning/native-inference-step-01-lock.json), verifying
their identities before use. The upstream archive used for the local build is
`https://codeload.github.com/ggml-org/llama.cpp/tar.gz/5266f24da75dc449bd56cbed7addb9c8e4a6a73e`;
its SHA-256 is `2de0d87eda4696e9f6bbd771d4c623267f4e95856cce6f99793f91522f993e43`.
Use separate build directories; keep raw build output in a retained log.

```sh
cmake -S "$CREXX_LLAMA_SOURCE" -B cmake-build-llama-controls -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DGGML_BACKEND_DL=ON \
  -DGGML_NATIVE=OFF -DGGML_METAL=ON -DGGML_METAL_EMBED_LIBRARY=ON \
  -DGGML_OPENMP=OFF -DLLAMA_OPENSSL=OFF -DLLAMA_BUILD_COMMON=ON \
  -DLLAMA_BUILD_TESTS=ON -DLLAMA_BUILD_EXAMPLES=ON -DLLAMA_BUILD_TOOLS=ON \
  -DLLAMA_BUILD_SERVER=OFF -DLLAMA_BUILD_APP=OFF
cmake --build cmake-build-llama-controls --parallel 6 \
  --target llama-bench llama-embedding llama-simple test-llama-archs
cmake -S tests/native-inference -B cmake-build-llama-workload -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DLLAMA_SOURCE_DIR="$CREXX_LLAMA_SOURCE" \
  -DLLAMA_LIBRARY_DIR="$CREXX_LLAMA_BACKEND_DIR" \
  -DCONTROL_MODEL_DIR="$CREXX_LLAMA_MODEL_DIR" -DCONTROL_TEST_METAL=ON
cmake --build cmake-build-llama-workload --parallel 2
```

`CREXX_LLAMA_BACKEND_DIR` is the absolute path to the upstream build's `bin`
directory. These are the measured macOS configuration choices; Linux/Windows
GPU build flags and runtime dependencies still need their own qualification.
The direct control explicitly loads backend files and never uses environment
backend lookup. CPU runs load CPU only; they do not silently use Metal or the
separately built BLAS module. Any CPU/BLAS comparison must be labelled separately.

## Run controls

```sh
cmake-build-llama-workload/native_inference_control \
  "$CREXX_BGE_MODEL" "$CREXX_LLAMA_BACKEND_DIR" cpu bge bge-cpu.json 100 20 4 2
cmake-build-llama-workload/native_inference_control \
  "$CREXX_BGE_MODEL" "$CREXX_LLAMA_BACKEND_DIR" metal bge bge-metal.json 100 20 4 2 bge-cpu.json
cmake-build-llama-workload/native_inference_control \
  "$CREXX_SMOL_MODEL" "$CREXX_LLAMA_BACKEND_DIR" metal smol smol-metal.json 100 20 4 2
cmake-build-llama-workload/native_inference_control \
  "$CREXX_STORIES_MODEL" "$CREXX_LLAMA_BACKEND_DIR" cpu scratch stories.json 100 20 4 2
```

Arguments after the JSON output are repeated single requests, batches, private
contexts and CPU threads per context. `bge` uses 8-row batches including 32,
128 and 384-token rows; `smol` and `scratch` use 4 rows and at most 32 generated
tokens per row. Generation uses a 512-token limit per sequence, 512 logical
batch tokens, 128 physical batch tokens and 8 maximum outputs. Causal generation
prefill returns between 128-token native calls, retaining all logical rows and
each row's first sample before logits are replaced. BGE uses 4096
logical/physical batch tokens so a complete noncausal sequence is never split.
Both use 8 sequence slots and 4096 total context tokens. The BGE profile tests
document and prefixed-query 512/513-token boundaries. Warm-up precedes counters
and memory samples; concurrent-context checks occur after the serial workload.

Each process writes JSON and prints `CONTROL_JSON` plus a final `PASS`/`FAIL`.
The process returns nonzero for failed assertions. Numeric failures are retained
in JSON and remain failures even when independent checks can continue. Do not
use the existence of output, or completion of a workload, as its pass condition.
The optional last argument compares BGE against a passing version 2 CPU
reference's tokenizer IDs and vectors at each of the single/four/eight-row
layouts. Other cross-platform identity checks belong in the evidence
manifest. The caller must verify the model hash; the direct workload does not
implement the future provider's hash validator.

`CREXX_LLAMA_CONTROL_FLASH=off` selects a labelled diagnostic only. Neither that
diagnostic nor the locally widened F32 BGE file changes the pinned F16 reference
or the active pass limits. The unmodified upstream embedding executable
also reproduces the single-versus-batch numeric difference; retain its exact
commands, raw output and comparison with the direct control evidence.

`S2-D01-v2`, approved on 2026-09-14, checks both maximum absolute coordinate
drift and cosine direction: same-layout repeats/private contexts use
`0.00001 / 0.999999`; different batch layouts use `0.001 / 0.99998`; CPU/GPU
comparisons use `0.002 / 0.9999`. The last number is a minimum cosine, not a
retrieval accuracy percentage. JSON retains each comparison's limits/count and
worst measured values, plus single/four/eight-row vectors as frozen references.
Retain their model/source/backend identities and checksums; do not regenerate a
reference to conceal a regression. No version 1 failure becomes a pass.

The cheap `native_inference_embedding_tripwire` CTest (`--test-tripwire`) checks
accepted variation and deliberate coordinate/direction corruption, including
invalid and nonfinite vectors. It needs the linked runtime libraries but loads
no model or compute backend. Real-model tests are opt-in within this standalone
QA project; none enters the product's broad CTest pool.

```sh
cmake-build-llama-workload/native_inference_control --co-resident \
  "$CREXX_BGE_MODEL" "$CREXX_SMOL_MODEL" "$CREXX_LLAMA_BACKEND_DIR" metal co-resident.json
cmake-build-llama-workload/native_inference_template_control \
  "$CREXX_SMOL_MODEL" "$CREXX_LLAMA_BACKEND_DIR/libggml-cpu.so" template.json
cmake-build-llama-workload/native_inference_fixture_generator \
  "$CREXX_LLAMA_BACKEND_DIR/libggml-cpu.so" --arch llama --seed 1234 --out generated-fixtures
```

`template_control.cpp` compares upstream Jinja rendering with the approved
explicit SmolLM2 rendering for default/explicit system messages, empty user text
and Unicode/newlines, retaining token IDs. `fixture_loader.cpp` loads the CPU
module before calling the unchanged upstream `test-llama-archs.cpp` entry point;
this repairs the QA launch setup without modifying upstream source. Generated
dense/MoE models contain random weights, not useful language knowledge. Test
the dense file using the `scratch` profile, or set `CONTROL_GENERATED_MODEL` to
its absolute path before CTest. Record its hash, seed and toolchain: upstream's
seed derivation includes `std::hash`, so bytes are not promised across toolchains.
Backend suffixes above are those actually produced on this Mac; use the exact
module filename produced by the target platform's CMake build.

## Debug and sanitizer controls

Configure separate `cmake-build-llama-debug` and `cmake-build-llama-debugasan`
trees from `tests/native-inference`, with `CONTROL_BUILD_UPSTREAM=ON`, the same
explicit source/model/GPU options, `CMAKE_BUILD_TYPE=Debug`, and
`CONTROL_SANITIZE=ON` only for the latter. This builds upstream CPU code with
the selected instrumentation, rather than linking an uninstrumented Release
engine into an ASan test. `CONTROL_DEBUG_GGML_OPT=ON` applies `-O1` to the
ggml tensor libraries only; llama.cpp and the QA adapter retain Debug settings,
and assertions and ASan instrumentation remain enabled throughout. The initial
unoptimized-kernel runs were stopped and are not passing evidence.

```sh
tools/asan-run.sh --build-dir cmake-build-llama-debugasan --phase build \
  --build-target native_inference_control --build-target native_inference_template_control \
  --build-target ggml-cpu --build-target ggml-metal --build-jobs 6 --no-live-tail
tools/asan-run.sh --build-dir cmake-build-llama-debugasan --phase ctest \
  --regex '^native_inference_' --test-jobs 1 --no-live-tail
```

Run the same focused commands in the normal Debug tree first. Each real-model
test is serial, with 600 seconds for real-model/co-resident cases and 120 seconds
for small fixtures/template; the completion evidence retains measured durations.
The fixed 100-request/20-batch workload is identical across lanes. Timing budgets
are enforced only in ordinary Release. ASan's reserved heap/quarantine inflates
RSS: S2-QA01 retains RSS but applies the unchanged 32 MiB growth allowance to
live allocator bytes in ASan, including close/reload cycles. Normal builds keep
the process-RSS guard. This does not replace Linux LeakSanitizer or GPU memory
qualification; Apple LeakSanitizer is unavailable and GPU kernels are not ASan
instrumented. No quarantine setting or sanitizer suppression is added.

## cREXX pre-implementation failure and positive control

```sh
cmake-build-release/bin/crexx tests/native-inference/provider_acceptance.crexx \
  --program cmake-build-llama-workload/provider_acceptance --noexec
cmake-build-release/bin/crexx lib/plugins/vector/rxvector_test.crexx \
  --program cmake-build-llama-workload/vector-positive --noexec
cmake-build-release/bin/rxvm cmake-build-llama-workload/vector-positive.rxbin
```

The provider test currently fails with missing `rxllama` procedures. It tests
normal success, rejected configuration, lifecycle, repeated/batch packed output,
generation and simultaneous model residency, and stale/parent handles once
implemented. It includes incremental output, active cancellation, wrong
capability/empty/oversized input, bad hashes and missing models. Its runtime
arguments are BGE path/hash and SmolLM2 path/hash.
The existing vector control must
pass through compile/assemble/link/execution. Neither test uses `WILL_FAIL` or
a skip to turn an absent feature into a green test.

`provider_worker_acceptance.crexx` is a separate ordinary red test for copied
native model-handle rejection in two foreign worker VMs. Its arguments are BGE
path/hash. `worker_transport_positive.crexx` independently proves the binary
worker transport shape through compiler, assembler, linker and VM execution.
Run its linked image with `cmake-build-release/bin/rxvme`, or run plain `rxvm`
from `cmake-build-release/bin` so worker VMs can resolve the standard library.
The parent VM's `-l` argument alone did not provide the worker's library root in
the observed raw-tree invocation. Installed worker provisioning remains an
explicit STEP-03/06 check. Required-GPU absence/fallback, package discovery,
static/dynamic installs and actual shared provider allocation identity need
their production-slice controls; these red tests do not claim that coverage.

The standalone CMake project registers twelve tests when both local devices
and the generated fixture are explicitly provisioned, including a separate
four-thread Smol CPU configuration. Intentionally red
provider tests remain explicit compilations. No nested product aggregate or
broad-pool registration is introduced.

## Measurement boundary

Compile the existing capture runner to a linked image, then invoke `rxvm IMAGE
-a --workload ... --warmups 2 --runs 10 --expect 'PASS native inference direct
control' --output-dir NEW_DIRECTORY -- CONTROL ...`. The runner retains stdout,
stderr, exact command and all serial samples. Use a new directory: it replaces
its own `raw` subdirectory. Capture power/load/thermal state before and after.

This measures a complete native control process plus separate backend setup,
model load, each context creation, first request, QA preparation, 100 warm
single requests and 20 warm batches. A first BGE request is one row; the first
Smol request is four rows with up to 32 tokens each. QA preparation includes
reference construction and two in-process warmups, so is not a minimal product
prepare operation. Fresh process does not mean cold OS page or shader caches.
The complete process includes model/context setup, serial and
concurrent verification, output serialization, cancellation and teardown. It is
not a pure startup measurement or a cREXX provider latency verdict. The Level B
reducer consumes all ten recorded `CONTROL_JSON` rows, checks correctness and
fixed counts, and reports mean/range and approximate Student-t 95% mean
intervals without removing outliers. A known 1..10 series verifies its output.
Later wrapper comparisons need matched controls and twelve paired rounds;
absolute samples here do not establish auto-selection or optimal CPU settings.

The completion baseline uses two CPU threads per context for BGE and Metal
controls, and four for Smol CPU. Whole-prefill Smol captures with both two and
four CPU threads hit the conservative 250 ms whole-call guard, despite passing
individual controls. Those failed captures remain retained. S2-C02 changes only
causal prefill call segmentation to the already configured 128-token physical
batch size, matching the approved bounded-processing design. It preserves
100/20/four-context work, all logical prompts, context/output limits and timing
thresholds. CPU and Metal generated IDs/finish reasons exactly match the retained
whole-prefill references. Use the explicitly named bounded128 capture cells for
the final generation control; BGE is unchanged. Timing is for one active context
during the warm serial phase; concurrent worker admission/latency remains later
provider qualification.

# STEP-04 typed llama completion — 14 September 2026

This is normal local implementation/delivery evidence for
[S4-D03 F-AC-01–04](../../planning/native-inference-typed-interface-proposal.md)
and [STEP-04 S4-AC-01–06](../../planning/native-inference-step-04.md).
The candidate retains the presented public contract and remains uncommitted
for Adrian's public-contract/phase-closure review. This bundle supplements the
earlier native embedding and C RXPA evidence; it does not replace their scope.

## Delivered behavior

`import llama` publishes C-implemented configuration, runtime, model, embedding
session, request, result and diagnostic interfaces. Owners retain the existing
checked native payload; no second registry or inference implementation exists.
Creation failures return initialized values with captured status. Diagnostics
are durable snapshots, complete input bytes reach the bridge, and a partial
`add_all` failure cancels the request while preserving the original row error.
Results own packed doubles and survive request closure; `values()` returns an
independent packedfloat consumed by existing rxvector operations.

The installed public examples demonstrate startup/load/preparation followed by
twenty batches, and four real workers with private contexts sharing one model
allocation. Worker results match each worker's isolated eight-row reference;
each worker completes twenty batches, model load count stays one and teardown
returns reservations to zero. Owners never cross worker boundaries.

The [provider reference](../../../lib/plugins/llama/README.md), C RexxDoc in
`lib/plugins/llama/typed.h`, and installed examples document every public
operation, failure/status behavior, complete text, bounds, ownership and shutdown.
Source documentation was added; no existing public API documentation was removed.

## Ordinary failures and repairs

1. `typed_configuration.log` and `typed_embeddings.log` retain missing-class
   failures before the typed adapter existed. The permanent consumers now check
   failure snapshots/recovery, full NUL-bearing embedding text, selector rejection,
   failed construction, parent/child and copied-owner close, partial convenience
   cancellation, preparation/state/admission, owned output and repeated batches.
2. `interface-first-before.log` retains the independent C-only failure when
   interface factories were the first/only provider references. The compiler
   now emits existing callable/provider metadata for known native implementation
   candidates. Default/named interface factories require no preceding concrete
   call. It introduces no discovery mechanism or bytecode ABI change.
3. `text-worker-before.log` and `text-callback-before.log` retain model-free
   failures when a native string had a nonzero physical integer field. Work
   completed successfully but the field became a false worker failure or
   C-to-Rexx callback signal. The internal typed external-call boundary now
   distinguishes execution failure from returned values, captures actual
   signals, and preserves the process-facing `run()` contract. The permanent
   fixture covers direct native text, C→Rexx→C text, worker string return and a
   legitimate nonzero integer task result. Existing actual unhandled-worker
   signal controls also pass.

Initial consumer authoring corrections follow existing contracts: device indexes
are one-based, preparation can need repeated calls, request configuration must
match session hardware policy, aggregate token limits must be reduced before
row limits, and NUL is valid in embedding text but invalid in selectors. Native
array metadata uses canonical `[*]`. These are not relaxed engine assertions.

## Normal local verification

Host: Apple M5, Darwin arm64; CPU and required-GPU Metal. No sanitizer execution.

| Check | Result and retained evidence |
| --- | --- |
| Typed Debug four-tool consumers | 28 runs: configuration plus embedding acceptance and both public examples, optimized/unoptimized, both VMs, CPU/Metal where a model is used. `debug-typed/` contains exact commands and `results.json`. |
| Typed installed Release/native consumers | 28 installed VM runs plus 14 relocated native runs; eight programs built from copied source, using installed imports and examples. `installed-typed/` contains commands and dependency hashes in `packages.json`. |
| Generic native-object regressions | All 32 individual Debug CTests pass, covering four fixtures, optimized/unoptimized and dynamic/static providers on both VMs. `debug-native-objects.log`. |
| Declaration-only compiler | Eight four-tool executions, four fixtures × two optimization modes, with the static compiler and static provider VM; the dynamic fixture import path is excluded. `debug-static-toolchain.log`. |
| Focused compatibility | All 55 Debug tests pass: factory selection/imports, RXPA services/callbacks, executor values/signals/startup, HTTP/concurrency and related controls. Selection and build targets: `focused-tests.json`; output: `debug-focused-tests.log`. |
| Existing low-level generation preparation | Four installed Release lifecycle runs, CPU/Metal on both VMs, confirm the original `rxllama` API still loads/shares Smol, prepares/reuses generation sessions and closes safely. `release-lowlevel-cpu.log` and `release-lowlevel-metal.log`. No generation request API or performance test is introduced. |
| External installed SDK | C/C++ normal and DECL_ONLY compilation; four generic consumers run on both installed VMs and as four relocated native executables. `installed-sdk/` records the eight VM/four native runs and package hashes. |

The typed acceptance verifies finite 384-dimensional results and existing
rxvector f32le conversion/cosine consumption. Worker same-layout tripwires remain
maximum coordinate difference `1e-5`, minimum cosine `0.999999`, unit-norm error
`2e-5`. The earlier [native closeout](../native-inference-step04-closeout/README.md)
retains layouts 1/4/8, CPU/Metal numeric comparisons, 100 singles, exact 512/513
query-token and aggregate bounds. The inference bridge, ABI header and package
implementation are hash-identical to that evidence; typed delivery and the two
generic repairs have fresh checks here. This is not a repeat of full STEP-03 QA.

## Reproduction and identity

`identity.json` records baseline HEAD, working source hashes, Debug/Release
artifacts and unchanged underlying bridge/package inputs. Models remain separate
read-only data. The pinned BGE F16 digest is
`f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999`;
llama.cpp is `5266f24da75dc449bd56cbed7addb9c8e4a6a73e`.

For an already configured ENABLE_LLAMA CPU/Metal build, prepare the normal
toolchain/product **and the explicit optional provider targets**:

```sh
cmake --build BUILD --target stage-c1-toolchain stage-product stage-optional \
  llama_provider_runtime_package crexx-provider-package --parallel 6
python3 tests/native-inference/typed_toolchain.py BUILD SOURCE MODELS FRESH_WORK cpu,required-gpu
cmake --install BUILD --prefix SCRATCH_PREFIX
python3 tests/native-inference/typed_package_consumer.py SCRATCH_PREFIX SOURCE MODELS FRESH_WORK cpu,required-gpu
```

Use distinct fresh work directories. Model modes must match available hardware.
The installed checks copy only the executable and declared runtime files to
paths containing spaces/Unicode and remove CREXX_HOME/provider/library overrides
for execution. No sibling checkout import, model server, inference CLI or Python
runtime is required by those executables. Python only orchestrates QA.

The first Release install used a stale provider because the staged build command
omitted `llama_provider_runtime_package`; `stage-optional` selects examples/demos,
not optional providers. `release-installed.log` retains that missing-class failure.
The provider target correctly depends on dynamic/static adapters and runtime
dependencies. Building it and reinstalling resolved the failure without changing
CMake wiring. `release-provider-build.log` and final install/consumer logs retain
the correction. The first static-compiler command also used an incorrect VM path;
`debug-static-compiler.log` retains the command error, and the final command uses
`BUILD/tests/objects_rxbvm`. Neither is a product defect.

## Acceptance boundary and next gate

The accepted STEP-04 performance verdict and unresolved NI-S4-P01 Metal variation
remain unchanged. No new performance study or claim for typed-facade overhead is
made. Existing reference numeric and preparation contracts remain unchanged;
generation request methods are STEP-05.

Windows/Linux/CUDA/Vulkan, broader final-product/hosted QA and all maintained
sanitizer execution remain STEP-06. **SAN-009 is still open and release-blocking**,
owned by Codex under Adrian's direction; this includes the assigned S3-D01/S4-D01
and native-object/compiler/executor/typed consumer proof. Individual generic VM
tests use prepared artifacts; new nested typed/package aggregates remain explicit
until isolated Debug/sanitizer scheduling evidence is obtained in STEP-06.
No commit, push, user-prefix installation or STEP-05 implementation occurred.

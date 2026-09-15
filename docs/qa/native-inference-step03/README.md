# STEP-03 local evidence — 14 September 2026

Baseline HEAD: `c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8`. The provider
implementation is uncommitted. [STEP-03](../../planning/native-inference-step-03.md)
owns the acceptance reconciliation. Adrian subsequently approved STEP-03 closure
with remaining SAN-009/S3-D01 sanitizer proof assigned to STEP-06 native-inference
release QA, owned by Codex under his direction. SAN-009 remains open.

This bundle records the provider candidate **before S3-D01**. Its native-worker
deadlock has since been repaired with Adrian's approval; see the separate
[S3-D01 evidence](../native-inference-s3d01/README.md) for the accepted Release
cost and expanded Debug native-worker/package results. All sanitizer results
here predate that VM repair and do not qualify it.

Adrian subsequently requested SAN-009 diagnosis/repair with **no sanitizer
rerun yet**. The follow-up audit confirmed that the existing repair and all
20 recorded source/test/build-input hashes are unchanged, both recorded build
configurations still match, and all 99 evidence checksums were valid before
this documentation update. No build or test was launched during that audit;
results below predate the hold. Further sanitizer builds/tests await Adrian's
direction. The worklist retains the diagnosis and qualification boundary.

| Evidence | Result and boundary |
| --- | --- |
| `final-debug-provider-tests.log`, `final-asan-provider-tests.log` | Final pre-S3-D01 provider candidate: 8/8 provider/four-tool tests in each build; 56.90 / 158.65 seconds. Corresponding detailed CTest logs and build logs are retained. |
| `debug-focused-ctest.log`, `asan-focused-ctest.log` | Earlier 9/9 selections include the unchanged native-project build/cache contract. Its Debug/ASan results are reused after the provider-only option validation change. |
| `debug-package/`, `asan-package/` | Complete scratch install; declarative manifest negatives; native cache invalidation; native BGE/Smol CPU/Metal relocation; missing-GPU fallback/failure despite an ambient path; missing runtime manifest; dynamic foreign-handle rejection and four actual cREXX workers sharing weights/private contexts. |
| `source-before-device-validation.json`, `source-and-configuration.json` | Exact source/test/configuration hashes and host details. Only `bridge.cpp` and its C++ lifecycle test changed after the broader package checks, adding CPU/device and session/device conflict rejection. Driver, RXPA adapter ABI and packaging code are unchanged. |
| `cmake-build-*-manifest*.json` | Hash inventories of the measured runtime and native bundles. Binary libraries and model weights are not committed here. `*-final.json` records the final validation candidate. |
| `lifecycle-before-provider.log`, `fingerprint-before-helper.log` | Ordinary pre-implementation failures; no expected-pass wrappers. |
| `device-conflict-before-repair.log` | The ordinary new option-conflict control fails when an incompatible device selector is accepted. |
| `asan-odr-original.log`, `asan-probe-cycle-red.log` | Original SAN-009 trigger and permanent test failing with the original Unix probe close restored. No ODR suppression was applied; repaired source was restored afterward. |
| `asan-original-trigger-repaired.log` | The original four-tool aggregate passes after the probe-lifetime repair. |
| `native-worker-startup-sample.txt`, `native-worker-positive-red.log`, `native-worker-positive-dependencies.txt` | Original S3-D01 native startup deadlock, including a provider-free control whose dependencies contain no llama/ggml runtime. Repair and current qualification are recorded in the separate S3-D01 bundle. |

The package matrix measured 82.100 seconds in normal Debug and 175.305 seconds
in maintained Apple ASan, in isolation after building install prerequisites.
The expanded native-worker matrix now passes in Debug (114.687 seconds, separate
S3-D01 bundle). It remains an explicit target until the new sanitizer scheduling
measurement is authorized and complete, before broad CTest registration.
The existing four-tool CTest is serial with a 180-second timeout,
based on its separately retained Debug/ASan measurements.

The final focused command is:

```sh
ctest --test-dir cmake-build-debug --parallel 1 --output-on-failure \
  -R '^rxllama_(backend_probe_cycle|bridge_.*|toolchain_lifecycle)$'
tools/asan-run.sh --build-dir cmake-build-debugasan --phase ctest \
  --regex '^rxllama_(backend_probe_cycle|bridge_.*|toolchain_lifecycle)$' \
  --test-jobs 1 --leaks off
```

Build `llama_provider_runtime_package` and `rxllama_bridge_lifecycle` first,
using `tools/asan-run.sh --phase build` for the maintained sanitizer tree.
The scratch-install matrix is `rxllama_package_measure`; its scripts, target
prerequisites and configuration are source controlled. The harness owns the
temporary install/work directories named in `workspace.txt`. The model paths
refer to the separately provisioned STEP-01 files; no test downloads weights.

Apple's Clang ASan lacks LeakSanitizer; `--leaks off` here is that documented
capability limit. Linux ASan/LSan, broad full-platform gates, Windows, CUDA and
Vulkan are not qualified by this evidence. These are lifecycle/admission and
package controls, not STEP-04/05 request semantics or Release performance
verdicts. Numeric tolerances and upstream control decisions remain the approved
STEP-02 evidence.

`SHA256SUMS` covers every retained evidence file except itself. The external
scratch paths can be reproduced from source; their presence on this machine is
not required to read the retained command/output evidence here.

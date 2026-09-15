# Native inference candidate lanes

The authoritative numbered work package is
[`docs/planning/native-inference-ci.md`](../../docs/planning/native-inference-ci.md).
`release-matrix.json` defines the six complete binary packages; candidate jobs
use the same staging/smoke path as eventual publication. No trained GGUF is
downloaded. The 4.8 MB generated fixture lives under `tests/native-inference` and
is excluded from user archives, as is the engine test executable.

Push `temp/llama-release-*` to run the package matrix. Manual candidate runs may
select `base`, `cuda`, or one named lane. Selection cannot narrow a develop/tag
publication matrix. The separate Deep Build and Sanitizer workflows are
dispatched against the same candidate branch after package triage:

```sh
gh workflow run build.yml --ref temp/llama-release-qa -f lane=linux
gh workflow run deep-build.yml --ref temp/llama-release-qa
gh workflow run sanitizers.yml --ref temp/llama-release-qa
```

Inspect the returned run's branch and `headSha` before cancelling or rerunning
it. `gh run cancel RUN_ID` cancels that run; `gh run rerun RUN_ID --failed`
retries its failed jobs at the same SHA. New runs supersede only the same
workflow on the candidate branch. Publication remains limited to existing
develop/tag events, never candidate pushes or manual candidate runs.

Nested package smoke is serial with a 3,600-second outer hang guard and
1,800-second child guards. It measured 44.5 seconds in Debug and 48.3 seconds
under Apple ASan before CTest registration. It has the `qualification` tier:
ordinary package jobs run it once against their final staged payload, while
Deep Build qualification and full sanitizer QA include it through CTest.
The broad hosted jobs use 240-minute outer backstops. Actual GPU presence and
fixture computation are logged; an OS label is not device evidence.

CUDA build inputs are NVIDIA 12.9.1 component archives pinned by size/SHA256 in
`cuda-12.9.1.json`. `scripts/ci-cuda-toolkit.py` assembles and caches the SDK and
retains component licenses in the provider notice and user guides. Windows CUDA
uses the installed MSVC v142/14.29 toolset supported by this CUDA line; the base
Windows package retains MinGW/Vulkan. The engine keeps its upstream portable
CUDA architecture defaults (`GGML_NATIVE=OFF`); CI does not tune or narrow its
GPU target list. OS graphics drivers are not redistributed.

After signing a trusted staged payload, verify its signatures before running
`scripts/refresh-provider-manifests.py`. That script updates only the declared
file hashes (including the runtime-manifest dependency) and is not installed.
Final-payload smoke must pass before archiving. Never use it to repair a user's
damaged package: the runtime must reject altered files.

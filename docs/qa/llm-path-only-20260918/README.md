# Path-only native model setup — 18 September 2026

Local implementation of the PATH criteria in
[the LLM interface plan](../../planning/llm-provider-interface.md#path-only-local-model-setup-18-september-2026).
Adrian subsequently authorized publication and local installation on 18 September.
The delivery extension is tracked in that plan. Base revision and exact
source/test/build-definition hashes are retained in [inputs.json](inputs.json).
Build configuration and provider binary hashes are in
[build-inputs.json](build-inputs.json).

The common client now accepts a filename without an expected hash:

```rexx
config = .llmconfig("llama", "/path/to/model.gguf")
client = .llm.open(config)
```

Omitted/empty SHA-256 is calculated once during opening, before the existing
content-addressed sharing lookup. This is a synchronous file read; tensor
loading continues asynchronously. An explicit hash retains background
verification. Compatible inferred/pinned owners share weights and retain the
same embedding-space identity. Reference presets still require their exact
artifacts. Models remain immutable application-provisioned files while in use.

## Evidence

- [Before-change regression](regression-before.log): the new path-only common
  consumer compiled, assembled and linked, then failed at preparation with
  `expected lowercase SHA-256` on the unchanged provider.
- [Normal Debug common-driver suite](debug-common.log): passed in 94.25 seconds.
  This covers both VMs and optimization modes, automatic generation/embedding
  identity, repeated/batched generation, supplied-hash output parity, bad-hash
  diagnostics, explicit child closure, owned results, optional-provider errors
  and deterministic HTTP fixtures. [Command manifest](debug-commands.json).
- [Bridge rejection controls](debug-negative.log): wrong/malformed hash,
  missing file, incorrect reference-preset artifact, template, tokenizer,
  geometry and budget errors remain explicit. The generation/embedding bridge
  controls also compare the inferred SHA with the known fixture hash and verify
  shared allocation identity for inferred and explicitly pinned owners.
- [Normal Debug lifecycle](debug-lifecycle.log): the existing two-model CPU
  ownership/sharing/lifecycle check passed in 13.20 seconds with the new bridge.
- [Runnable example](example.log): the updated common generation example
  passed through rxc, rxas, rxlink and rxvm with only driver, model filename and
  prompt. It used the existing local SmolLM2 Q8_0 artifact, with no hash or
  hardware/template override, and returned a complete sentence.
- [Focused maintained Apple ASan verification](asan-checks.log): both tests
  passed, common drivers in 320.32 seconds and the two-model CPU lifecycle in
  34.36 seconds. [Command manifest](asan-commands.json) and
  [rejection controls](asan-negative.log) retain the detailed results.
- Relative Markdown file links and `git diff --check` passed. Existing RexxDoc
  tag coverage was preserved: typed API 30 `@param` and 40 `@return` tags;
  common example one of each. Source/test/build input hashes were unchanged
  through final verification.

Normal build command: `cmake --build cmake-build-debug --target
rxllama_common_prerequisites --parallel 6`. The two focused CTest commands used
`-R '^rxllama_common_drivers$'` and `-R '^rxllama_bridge_both_cpu$'` respectively.
The common aggregate retains serialized scheduling and its 1800-second hang
backstop. No core/VM/compiler implementation changed; broad core correctness
and hosted overnight matrices were not repeated for this provider-only change.

Sanitizer preparation used `tools/asan-run.sh --phase build --build-target
rxllama_common_prerequisites --build-target rxllama_bridge_lifecycle
--build-leaks off --no-live-tail`. Verification used the same runner with
`--phase ctest --regex '^(rxllama_common_drivers|rxllama_bridge_both_cpu)$'
--leaks off --test-jobs 1 --no-live-tail`. Complete build logs remain under
`cmake-build-debugasan/asan-logs/20260918-140819-build`; the test-run directory is
`cmake-build-debugasan/asan-logs/20260918-141910-ctest`.

PATH-AC-01–04 and PATH-STEP-01–04 are complete locally. Authorized delivery adds
PATH-AC-05–07 and PATH-STEP-05/06. Their live local receipt is
`cmake-build-release/qualification/path-only-delivery.json`; normal Build CREXX
and CodeQL records attached to the published commit are the hosted publication
authority. Parent native-inference platform/release acceptance remains unchanged.

These are local macOS checks. Apple LeakSanitizer is unsupported; no new Linux,
Windows, GPU, release or model-quality qualification is claimed.

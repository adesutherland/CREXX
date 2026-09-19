# Validated per-layer GGUF geometry — approved 19 September 2026

## Vision and outcome

The generic native provider should accept supported GGUF scalar or per-layer
attention geometry without a model allowlist or relaxed resource admission.
The installed Gemma 4 12B artifact currently fails on the valid 48-element
`gemma4.attention.head_count_kv` array before engine loading. The user explicitly
approved the CREXX fix alongside the RAG local-search follow-up.

## Acceptance criteria

1. **AC-01:** Existing scalar geometry and reservation results remain unchanged.
2. **AC-02:** Per-layer geometry uses every layer, with a conservative reservation;
   wrong length/type, nonpositive/out-of-range values and integer overflow fail
   before engine allocation. Unsupported layouts stay unsupported.
3. **AC-03:** The exact local 12B artifact passes metadata admission; report actual
   bounded native preparation/generation separately from parsing success and
   from answer quality. Record any remaining engine/resource limitation honestly.
4. **AC-04:** Regression reproduction, positive controls, focused native-provider
   correctness and build evidence are retained, with public documentation updated.

## Steps

1. **STEP-01 — complete:** Add metadata-only regression and scalar/malformed
   controls against the unchanged provider (AC-01–02).
2. **STEP-02 — complete; depends on STEP-01:** Implement validated per-layer geometry
   and document conservative admission (AC-01–02).
3. **STEP-03 — complete; depends on STEP-02:** Build coherent provider package and
   run focused provider/toolchain tests and bounded local-model probes (AC-03–04).
4. **STEP-04 — complete; depends on STEP-03:** Record results/limitations in this
   delivery record and the native-provider documentation/roadmap (AC-04).

Baseline: clean hotfix `bc1ab4f886f55722e4b895b320030742353a3561`.
Evidence source: ScottishHistory `reports/local-generation-20260919`.
Do not modify the unrelated dirty develop checkout. Do not weaken memory/VRAM
limits or dispatch unrelated overnight matrices for this metadata repair.

## Local result and limits

All four acceptance criteria have local evidence. The new `rxllama_model_geometry`
test reproduced rejection of valid arrays while its scalar exact-reservation
positive control and malformed controls passed. Final implementation accepts
KV-head and feed-forward arrays only, matching the pinned engine's
`get_key_or_arr` use; scalar key/value widths retain their existing restrictions.
Each element is validated; unsigned overflow, zero/negative/oversized values,
wrong element types and wrong layer counts fail before weight allocation.

The Release provider package and its common-driver/toolchain prerequisites build.
Final `rxllama_model_geometry` and `rxllama_common_drivers` pass in **25.14 s**.
The latter exercises compiler, assembler, linker, both VMs/native consumers,
generic model fixtures, HTTP drivers and lifecycle controls. Package path tests
for C++17/C++20 and backend probe cycling pass in **1.26 s**; the Windows-path
logic test also passes. Those three binaries initially were not built; their
not-run results were corrected by building and running them, not counted as passes.

Exact model: `/Users/adrian/Models/gemma4/gemma-4-12B-it-Q4_0.gguf`, SHA-256
`3712b9bd32cae83a22f67ee7a4466d8d7a4f21646ac8a07d19bf9418e8767a70`.
Final-package native `.llm` run: required Metal, one row, context 512, output 96,
20 GiB RAM/VRAM budgets, processing work 128. Preparation takes **27.71 s**;
three short controls complete at EOS in **1.37 / 1.32 / 1.51 s**, with correct
facts/abstention. This is an execution control, not a Scottish quality benchmark.
Raw Gemma control markers remain visible. At 4096 context tokens admission still
refuses the unchanged budget in 0.59 s. E4B scalar preparation and three requests
also passed; timings during compilation are explicitly not comparative evidence.

Evidence and artifact hashes:
`/Users/adrian/Documents/ScottishHistory/reports/local-generation-followup-20260919/`
(`native-manifest.json`, native lane databases/logs and `qa/`). Work is local and
uncommitted; installed CREXX and published develop are unchanged. No hosted or
cross-platform closure, memory-reservation optimization or model-quality release
claim is made by this repair.

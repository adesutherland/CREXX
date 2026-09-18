# JSON accessor borrowing — approved bounded repair

Adrian approved repairing the existing Scottish retrieval implementation on
18 September, following the source-grounded JSON and RAG profiling review.
This is the generic CREXX part of that repair; preserve unrelated dirty work.

## Outcome and design

Make small indexed JSON reads avoid copying the whole immutable source/index.
Keep the streaming parser, compact structural index, on-demand value decoding
and public API. No compiler/runtime/ABI change, cache, native provider or new
JSON representation is selected. The downstream owning plan and retained
reproducer are `crexx-rag/docs/retrieval-profiling-20260918.md` and the Scottish
`reports/bge-migration-20260918/profiling-20260918/` directory.

Alternatives considered: unchanged pass-by-value helpers preserve behavior but
reproduce seconds of copying; exposed read-only binary arguments remove those
copies using existing language semantics; caching or a new index format would
add lifecycle and invalidation concerns without addressing a necessary gap.
The five-helper borrowing prototype is selected from the retained comparison.

## Acceptance and implementation steps

1. **AC-01 / STEP-01:** Before editing production, run existing JSON helper,
   immutable-document, numeric-projection and noisy-input controls. Add checks
   for independent document copies, caller source mutation and mutation of
   returned scalar/child arrays. Record baseline results.
2. **AC-02 / STEP-02:** Change only read-only binary inputs in `_json_node_u8`,
   `_json_node_u32`, `_json_node_get`, `_json_member_node` and
   `_json_element_node` to `arg expose`. Inspect every affected helper for
   mutation and preserve callers and public signatures. Run focused controls
   on optimized/unoptimized images and both concrete VMs.
3. **AC-03 / STEP-03:** Freeze implementation after focused correctness and
   run the smallest ordinary profiling-off Release downstream comparison.
   Use an isolated copy of the installed Release package, replacing only its
   rebuilt Level-B library; do not overwrite the normal install. Retain exact
   tool/library/source hashes. Report the end-to-end verdict to Adrian and
   stop for direction under `performance/AGENTS.md` before broad closeout.
4. **AC-04 / STEP-04:** After that verdict is accepted, build the current
   hotfix core, run the focused JSON cases followed by the remaining Level-B
   functional and core smoke/essential cases once, document immutable-access
   behavior and retain evidence. Reuse the already measured downstream result;
   no new benchmark or overnight deep/sanitizer matrix is required for this
   ordinary library publication.
5. **AC-05 / STEP-05:** Adrian approved staging through CREXX hotfix and
   publishing to `origin/develop`. Transfer only this repair, its tests and
   supporting documentation from the dirty develop checkout; commit the
   qualified change on hotfix, push hotfix then the identical SHA to develop,
   and check the automatic publication workflows. Global installation and RAG
   publication remain outside this request.

Current status: AC-01 and AC-02 passed. The four focused cases (`ts_rxjson`,
`ts_rxjson_document`, `ts_rxjson_numeric_projection`, `ts_rxjson_noisy_contract`)
pass both before and after the five-helper repair, with optimization on/off
and on `rxvme`/`rxbvm` (16 baseline and 16 candidate executions). Added ownership
checks cover caller-source changes, independent document copies and mutated
returned scalars/child arrays, including Unicode and embedded NUL values.
The candidate library is assembled and linked using the frozen installed
`15c8a3ba4200` Release tools in a private package copy. Normal installation is
unchanged. AC-03 was accepted by Adrian on 18 September: the downstream
RAG repair takes 3.41/1.64/1.63 seconds versus a matching 10.47-second baseline,
with identical ordered passages, scores, claims and scan counts. These combined
results include RAG bulk traversal/dictionary changes, not JSON borrowing
alone. Focused ANN acceptance passes; native window recovery had one model-load
timeout followed by an unchanged passing replay. AC-04 passed in the clean
hotfix checkout, fast-forwarded to `e99136a1d5725d0c44128f64f505e1b46c64a51f`
before transferring the repair: 451/451 unique normal correctness cases
(287 Level-B functional and 169 smoke/essential, with overlap). The 14 JSON
cases ran first and were excluded from the remaining selection. One missing
terminal-test fixture was built and only its initially unrun case was run.
The [qualification record](../qa/rxjson-accessor-20260918/README.md) retains
all results, the initial not-run result, hashes and publication boundary.
AC-05 is authorized; remote identity and automatic hosted checks are the final
publication steps.
Evidence: Scottish `reports/bge-migration-20260918/retrieval-repair-20260918/`.
This is a single-client, non-representative performance repair; no portfolio,
RexxCPS, sanitizer, Windows/Linux or release-readiness claim is implied.

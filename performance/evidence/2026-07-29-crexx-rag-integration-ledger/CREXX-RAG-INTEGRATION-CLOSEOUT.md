# CREXX-side crexx-rag integration ledger closeout

Status: **closed; all CRI items have accepted dispositions**

Date: 2026-07-30

Home source: `develop` at
`d78c6fcfa81ef03fdbea65ff9cc39ed99e8716bf` plus the unstaged programme
changes described here. Nothing was staged, committed, pushed or installed to
the normal user prefix.

## One-to-one disposition ledger

| Item | Disposition | Accepted result/evidence |
| --- | --- | --- |
| CRI-01 | **fixed** | Imported Level B record identity remains nominally consistent through Level G return/assignment/invocation; focused and full proof in the [evidence index](README.md#cri-01--imported-level-b-record-returned-through-level-g). |
| CRI-02 | **fixed** | Read-only exact non-escaping binary inline formals avoid defensive payload copies without changing value semantics; [governed worklist](../../CRI02-BINARY-BYVALUE-WORKLIST.md). |
| CRI-03 | **no-CREXX-change** | Deterministic timeout/malformed/connection coverage passes; the hosted symptom was repeated downstream reparsing and an ineffective dimension field, not an `rxhttp` defect; [evidence index](README.md#cri-03--historical-hosted-rxhttp-timeout-classification). |
| CRI-04 | **fixed** | Definite-return analysis distinguishes terminal non-fall-through loops from reachable leaves/fall-through; [evidence index](README.md#cri-04--terminal-do-forever-and-missing-return). |
| CRI-05 | **fixed** | Certified Level G PARSE lowering retains a legal internal Level B fragment context while authored Level G assembler remains rejected; [evidence index](README.md#cri-05--level-g-parse-and-certified-level-b-lowering). |
| CRI-06 | **fixed** | Malformed RXPA signatures now receive specific structured import diagnostics rather than an internal compiler error; [evidence index](README.md#cri-06--malformed-rxpa-signature-diagnostics). |
| CRI-07 | **documented/package-closed** | Installed headers, version match, CMake targets/helper, explicit paths, docs and external plugin consumer are complete; [evidence index](README.md#cri-07--installed-rxpa-sdk-and-external-consumer). |
| CRI-08 | **fixed** | String setter/signal callbacks accept immutable source strings without ABI layout change; C/C++ `-Werror` consumers pass; [evidence index](README.md#cri-08--rxpa-const-correct-status-strings). |
| CRI-09 | **fixed** | Approved parse-once `rxjson.jsondocument`, typed nodes, noisy-container scanning and validation are production library APIs; [closeout](CRI09-CLOSEOUT.md). |
| CRI-10 | **documented/package-closed** | Existing scalar/array ADDRESS redirection covers multiline stdout/stderr/status/empty/Unicode/delimiters; [closeout](CRI10-CLOSEOUT.md). |
| CRI-11 | **documented/package-closed** | Existing `CREXX run :argv[]` preserves argv boundaries without a shell; [closeout](CRI11-CLOSEOUT.md). |
| CRI-12 | **documented/package-closed** | Redirect arrays append; `arraydrop` gives replacement semantics; [closeout](CRI12-CLOSEOUT.md). |
| CRI-13 | **fixed** | Explicit `node_f32_array`/`node_i64_array` projections return owning headerless canonical-little-endian binary; [closeout](CRI13-CLOSEOUT.md). |
| CRI-14 | **fixed** | Approved durable `crexx.operation-contract/1` artifact, CLI and installed CMake helper; [closeout](CRI14-CLOSEOUT.md). |

## Change inventory

Production compiler work is in `compiler/rxcp_*` and `compiler/rxcpfunc.c`,
with maintained regressions under `compiler/tests/`. It covers nominal imported
record identity, loop reachability, certified PARSE lowering, RXPA diagnostic
routing and the read-only binary-formal optimization.

Runtime/package work is in `interpreter/rxpa*.c`, `interpreter/rxvmload.c`,
`rxpa/`, the root CMake install, `cmake/CREXXConfig.cmake.in` and the installed
helpers. External SDK, const-correctness and failure-path tests are under
`tests/rxpa/`.

Library/runtime documentation work is in `lib/rxfnsb/rexx/rxjson.crexx`, its
library references, `docs/books/crexx_language_reference/statements.md` and the
functional tests under `lib/rxfnsb/tests_functional/`. Maintained benchmark
and analysis sources are under `tests/performance/`; all are cREXX Level B.

The operation-contract implementation, CLI and tests are under `contract/`.
Its installed helper is `cmake/CrexxOperationContract.cmake`; public guidance
is `docs/books/crexx_programming_guide/operation_contracts.md`. Private graph
support changes are in `binutils/rxsignature.c`, `binutils/rxgraph.c` and their
unit tests.

Performance programme registrations, decisions, first-Release stops, raw
sample locations and follow-on queues are retained in `performance/ROADMAP.md`,
`performance/capability-gaps.md`, the CRI-02 governed worklist and the dated
decision/closeout documents in this directory.

## Baseline, candidate and validation

- pre-change source: `develop` at `d78c6fcfa81e`, initially no tracked/staged
  diff and five preserved user-owned untracked lifecycle artifacts;
- installed baseline: `crexx-1.0.0-beta.3+local.g057592681c0c`, built
  20260728, distinct from source;
- clean pre-edit Debug baseline: 1,925/1,925, zero failures/skips, 205.06
  seconds;
- final scratch candidate: `crexx-1.0.0-beta.3+local.gd78c6fcfa81e.dirty`,
  built 20260730 with CMake 4.3.2 and AppleClang 21.0.0.21000101;
- final clean Debug build: passed;
- final clean Debug CTest: 1,965/1,965, zero failures/skips, 324.77 seconds at
  `--parallel 30`;
- final focused package/contract/RXPA consumer: 3/3;
- final scratch install: 141 files, no private/source-tree CMake paths;
- read-only concrete reproducers: optimized/non-optimized compilation passes;
  runtime-relevant cases pass on `rxvm` and `rxbvm`;
- read-only binary probe: all four mode/VM cells pass exact checksum and the
  optimizer inversion is absent; and
- out-of-tree read-only downstream: configure/build pass with both fallback
  switches off; 23/26 local tests pass and the three exact downstream removal
  seams are recorded in [CRI14-CLOSEOUT.md](CRI14-CLOSEOUT.md#read-only-downstream-replay).

CRI-02 and CRI-09/13 Release verdicts, formal samples, checksums, RSS and
profile/instruction evidence remain the controlling performance proof. Later
contract/package work does not modify those production paths.

## Compatibility and ABI statement

The accepted compiler fixes preserve nominal type strictness, `.binary` value
semantics, existing ADDRESS semantics and existing selector spellings. No
public C ABI layout, language syntax, runtime reflection behavior, RXAS
encoding or RXBIN encoding changed. RXPA const-correctness changes source types
at setter/signal callback boundaries whose callees already copied and never
mutated the input; the installed C/C++ consumer proves source compatibility and
the function-pointer/binary calling shape is unchanged.

The new Level B JSON class/methods, installed SDK targets/helpers and
`crexx-contract` CLI are additive APIs. The explicitly approved
`crexx.operation-contract/1` document is the only new public serialized
format. It is deliberately independent of its current private RXBIN-007
metadata adapter.

## Rejected alternatives and unresolved risks

- Weakening record type checking or comparing only short class names was
  rejected; nominal resolved identity is retained.
- Exposing/globalizing the binary under test, deleting its by-value boundary or
  changing value semantics was rejected; exact read-only alias proof is used.
- A second JSON parser, provider/RAG-specific parsing API and silent packed
  numeric inference were rejected. The approved production class is generic
  and packing is explicit.
- Header/envelope packed formats and public wrapper classes were rejected for
  Release 1 after the measured class surface was materially slower than raw B.
- Global selector canonicalization was rejected after it broke an existing
  ADDRESS bridge; CRI-14 canonicalization is adapter-local.
- Public RXBIN graph headers/ABI, runtime reflection and new annotation syntax
  were rejected for CRI-14. The stable artifact boundary makes those possible
  future metadata mechanisms rather than present dependencies.

Remaining non-blocking risks are cross-platform CI before publication, the
intentionally narrow operation-contract format-1 type slice, and three queued
performance investigations (`PERF2-07-B02`, `PERF2-07-C01`,
`PERF2-03-F06`). They do not reopen an accepted CRI disposition. The downstream
project must remove its old JSON/vector and socket-module accommodations before
its own current suite becomes 26/26.

## Preservation proof

The final CREXX audit is retained at
`/private/tmp/crexx-gate1a-final-logs.v1A6Ld/crexx-final-audit.txt`. CREXX
remains on `develop` at `d78c6fcfa81ef03fdbea65ff9cc39ed99e8716bf`, aligned
with `origin/develop`, with no staged diff. Its tracked programme diff summary
is 49 files, 3,033 insertions and 541 deletions; the separately listed
untracked set contains the programme sources/evidence plus the same five
pre-existing user-owned lifecycle artifacts. No pre-existing file was reset,
stashed, cleaned or overwritten.

The final read-only-checkout audit is retained at
`/private/tmp/crexx-gate1a-final-logs.v1A6Ld/crexx-rag-final-audit.txt`. It
proves that `crexx-rag` remains on `main` at
`97cd87e91344d6ac1773a054bd38df23eb128ed2` and that all three frozen state
hashes exactly match the pre-work audit:

```text
tracked diff  97443028cfa8e86624fc5fda9ea5fb33d02f4d7413e5a8a848d92ec365288ef9
index diff    30cea35503c6dc073f3007218b9458f2bc0c28b2c7661327b9144036d5a7c61d
porcelain-v2  02a6e4216ff47be3ce7252be2ccb39157bc45fda316c254f7198648109df14f1
```

The tracked and staged diff summaries also remain exactly 27 files with 574
insertions/55 deletions and 17 rename-only files respectively. No file in that
checkout was edited, built in, reconfigured, installed into, staged, committed,
stashed, cleaned or removed.

## Downstream unblock matrix

| Downstream accommodation | CREXX evidence | Downstream action |
| --- | --- | --- |
| Level B fallback around Level G `parseplan` | Exact Level G repro compiles/runs in both modes/VMs; CRI-05 fixed. | Restore intended Level G profiles/controllers one at a time and rerun their focused smoke tests. |
| Unreachable return after terminal `do forever` | Exact retained source compiles in both modes; CRI-04 flow matrix passes. | Remove only dummy unreachable returns and rerun parser/data tests. |
| String-only Level G surface around typed records | Exact two-module Level G record return now compiles/assembles in both modes; CRI-01 cross-module matrix passes. | Return the Level B record directly and compare CLI/ADDRESS/MCP semantics before deleting serialization-only glue. |
| `.binary` exposure/direct-access workaround | Exact downstream probe passes all cells; formal Release proves the optimized by-value inversion removed. | Retest genuine read-only by-value helpers; retain direct access where it is naturally the simplest hot loop, not as a compiler workaround. |
| Vendored RXPA header/version/helper fallback | Downstream config/build succeeds with both fallbacks off; installed external plugin and path/version negatives pass. | Select the scratch/approved installed CREXX package, turn both fallbacks off, then remove vendored files and source-root logic only after the project suite confirms. |
| Malformed RXPA diagnostic-only expectation | Structured location-bearing negative matrix passes and never reports an internal compiler error. | Update expected diagnostics; keep valid named signature syntax. |
| Legacy `rx_socket` runtime module argument | Scratch replay without only that argument passes both VMs and loopback. | Remove `rx_socket` from downstream VM module lists; sockets are provided by the current VM core/runtime library route. |
| Incubator JSON class/envelope/vector methods | Production JSON benchmark and numeric projections pass; current downstream failures name `as_f32_vector`/`as_i64_vector`. | Retire the competing incubator `jsondocument`, `f32vector`, `i64vector` and envelope; migrate callers to production `jsondocument` plus explicit raw binary type/count ownership. |

## Suggested closure text for `docs/crexx-integration-issues.md`

The following text is for the downstream agent; the read-only file was not
edited here.

1. **Level G imported record:** “Resolved by the approved CREXX candidate.
   Cross-module Level B record identity now survives Level G return,
   assignment and invocation without weakening nominal checks. Remove the
   string-only facade after its semantic comparison passes.”
2. **Optimized binary helper:** “Resolved. Exact read-only, non-escaping
   by-value binary formals no longer materialize a payload copy when inlined.
   The retained probe has matching checksums on both VMs and no optimized
   inversion. Exposure is no longer required as a compiler workaround.”
3. **Hosted timeout:** “Closed as no CREXX transport defect. Deterministic
   timeout, malformed-response and connection failures remain structured; the
   historical delay was downstream repeated reparsing plus the wrong dimension
   field. No timeout semantics changed.”
4. **Terminal `do forever`:** “Resolved. Definite-return analysis recognizes
   terminal non-fall-through loops while preserving reachable/conditional
   leave and true fall-through diagnostics. Remove dummy returns.”
5. **Level G PARSE:** “Resolved. Certified PARSE lowering follows its legal
   Level B internal path; authored Level G assembler remains rejected. Retest
   and restore intended Level G sources.”
6. **Malformed RXPA signature:** “Resolved. Invalid separators, components,
   types and returns produce specific user diagnostics, not
   `#INTERNAL_ERROR_PARSING_IMPORT_AST`.”
7. **Observed installed SDK/package gap:** “Package-closed. The candidate
   installs `rxpa/crexxpa.h`, the matched version header, CMake package/imported
   targets, `RXPluginFunction.cmake`, tool/runtime paths and documentation. The
   no-fallback external plugin test passes both VMs and rejects mismatches and
   missing paths.”
8. **RXPA const strings:** “Resolved without an ABI layout change. Setter and
   signal paths accept immutable diagnostic/status strings and C/C++ `-Werror`
   consumers pass. Legacy registration metadata remains a separate mutable
   API.”
9. **Fuzzy model/tool output request:** “Resolved by generic JSON facilities,
   not provider vocabulary: boundary scanning of noisy text, parse-once typed
   traversal, null/missing distinctions, Unicode, malformed/truncated failure
   and explicit validation statuses.”
10. **Multiline ADDRESS capture:** “Package-closed by documentation and tests.
    Existing scalar/array redirects capture stdout, stderr, status, empty and
    Unicode/multiline output.”
11. **ADDRESS argv:** “Package-closed. `CREXX run :argv[]` is the supported
    shell-free argv-preserving route and is tested with whitespace, empty
    arguments, quotes, Unicode and metacharacters.”
12. **Redirect array lifecycle:** “Package-closed. Redirect arrays append;
    call `arraydrop` on each reused output/error array when replacement rather
    than accumulation is required.”
13. **Large JSON/packed numeric request:** “Resolved. Use one immutable
    `rxjson.jsondocument`, retain it while traversing node IDs, and request
    explicit `node_f32_array`/`node_i64_array` owning raw little-endian binary.
    JSON itself does not infer width, byte order, dimensions or vector meaning.”
14. **Schema/contract request:** “Resolved by the approved durable build-time
    `crexx.operation-contract/1` artifact, `crexx-contract` and installed CMake
    helper. Existing Level B interfaces describe typed input/result/error;
    metadata acquisition may evolve without changing consumers.”

## Paste-ready downstream resumption prompt

```text
Work in /Users/adrian/CLionProjects/crexx-rag and reread its AGENTS.md and
current Gate-1A documents completely. Use the approved CREXX candidate from a
fresh scratch or explicitly selected install; do not silently use the old
normal-prefix toolchain. First audit branch/HEAD/status/diffs and preserve all
pre-existing work. Configure a clean out-of-tree build with
CPRAG_ALLOW_VENDORED_CREXXPA=OFF and
CPRAG_ALLOW_CREXX_SOURCE_FALLBACK=OFF, and prove every CREXX executable/header
comes from the selected prefix.

Replay the Gate-1A concrete reproducers and evidence. Remove only workarounds
whose CREXX-side proof is linked from
/Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-29-crexx-rag-integration-ledger/CREXX-RAG-INTEGRATION-CLOSEOUT.md:
restore intended Level G PARSE sources, remove dummy unreachable returns,
restore typed Level G record returns, retest genuine read-only by-value binary
helpers, remove the vendored RXPA SDK/helper fallback, and update malformed
signature diagnostics. Remove the retired rx_socket runtime-module argument.
Retire the competing incubator jsondocument/f32vector/i64vector envelope and
migrate callers to the production rxjson.jsondocument plus explicit
node_f32_array/node_i64_array raw binary with schema-owned type and count.

Run focused tests after each removal, then the complete deterministic/loopback
configure/build/CTest. Make no hosted call and use no credentials. Refresh
docs/crexx-integration-issues.md with the supplied closure text and retain exact
commands/results/hashes. Stop again at Gate 1A before Phase 1B and present the
updated unblock matrix, every remaining failure, and the smallest next
decision. Do not begin Phase 1B without Adrian's separate approval.
```

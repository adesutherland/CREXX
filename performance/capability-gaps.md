# Performance capability observations

Status: PERF2-08 Apple dispositions approved 2026-07-27; CAP-01 Option B/A2/B1
implemented and closed 2026-07-30 under CAP-01-J01

This register keeps capability findings separate from benchmark results. A
workaround is not automatically a language-design request, and a slower pilot
is not proof that a missing surface should be added. Promote an item into
implementation work only after a focused correctness contract and before/after
performance case are agreed.

## Confirmed gaps

| ID | Missing surface | Evidence and benchmark consequence | Approved disposition / future owner |
| --- | --- | --- | --- |
| CAP-01 | Parse-once JSON object model or indexed document handle | `rxjson` was string/path oriented and repeated parsing; the retained benchmark and noisy-input regression established the cost and correctness boundary. | **Closed by `CAP-01-J01` on 2026-07-30.** Legacy selectors are preserved; pure-Level-B immutable `.jsondocument`, document-local traversal, strict typed getters, and `jsonscancontainer` are implemented. Accepted B1 uses one allocation-free boundary-validation pass plus one indexing pass for a successful slice. Schema/repair policy and packed numeric serialization remain separate decisions. |
| CAP-02 | Heterogeneous nested array/reference containers suitable for an array-of-arrays tree | Level B references are present, including references to arrays, but the language reference explicitly excludes nested reference containers. Arrays cannot currently be stored as ordinary `.object` values. `awfy_storage.crexx` therefore represents each logical upstream array with a `StorageNode` plus an `.object[]`, adding an allocation and object dispatch per logical node; its cREXX time is not a common Storage score. | **Diagnostic + defer to post-Release 1 Level G.** No Level B ownership/type change was approved. A future decision must preserve weak-reference lifetime rules and typed-array fast paths. |
| CAP-03 | Standard RFC 4648 Base64 library API | Repository search found no Level B Base64 service, so every runtime port contains the full codec loop. This is a library gap, not a byte-language gap: cREXX's pre-sized `.binary` implementation passes the exact 1,024-byte round trip. | **Benchmark qualified; API deferred.** The existing equal-work benchmark remains common. A pure Level B API is a separate product track; native/SIMD is considered only after the portable form is selected and measured. |
| CAP-04 | Public load-only lifecycle timing boundary | The public cREXX, ooRexx and NetRexx command paths expose compile/translate and execution, but not one consistent loaded-without-executing phase. `run_lifecycle.crexx` therefore reports `load_first_result`, not pure loader time. | **Pure-load comparison excluded.** Retain the honestly named phase. A new CLI mode, VM API or counter is a separate public-interface decision. |

## Accuracy checks and non-gaps

- **References are not missing.** Bounce passes the PRNG by
  `reference .AwfyRandom` and aliases typed-array ball slots before mutation.
  List stores `reference .ListElement` links, returns references, checks
  `refvalid`, and dereferences explicitly. Its arena is needed because cREXX
  references are intentionally weak; the extra ownership structure is the
  disclosed adaptation.
- **Binary-safe storage is not missing.** `.binary`, `binresize`, direct
  `<at..u8>` access and binary comparison all work in the Base64 port. The
  remaining CAP-03 question is reusable library coverage and possible
  acceleration.
- **Classes and application control flow are not missing.** Bounce, List and
  Storage use Level B classes. Richards uses the same explicit task-kind state
  machine in all three ports so the comparison is reviewable; it does not prove
  a cREXX closure or object-dispatch gap.
- **Compound Boolean expressions are an audit candidate, not a confirmed
  missing capability.** Richards uses simple early-return predicates. A more
  compact compound form was avoided during authoring, but no minimal failing
  reproducer is retained. Do not open a language/performance item until a
  focused opt/no-opt source proves a current compiler or runtime defect.

## Future evaluation order

These are recorded owners, not authorization or a queued PERF2-09 follow-on:

1. CAP-01 is the highest-value library/runtime experiment because it affects
   parser reuse, allocations and real application code, and the current JSON
   comparison cannot be aggregated.
2. CAP-02 is the clearest language/container question. Start with the narrowest
   ownership-safe container prototype and use Storage plus reference lifetime
   tests as gates.
3. CAP-03 is bounded and low-risk as a pure Level B library addition; use the
   current benchmark to decide whether native acceleration is justified.
4. CAP-04 improves measurement resolution rather than workload speed, so it
   belongs with evidence automation unless loader profiling reveals a product
   bottleneck.

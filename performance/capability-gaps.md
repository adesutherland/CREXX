# Performance capability observations

Status: reviewed against the 2026-07-15 expanded portfolio, current Level B
documentation and retained qualification evidence

This register keeps capability findings separate from benchmark results. A
workaround is not automatically a language-design request, and a slower pilot
is not proof that a missing surface should be added. Promote an item into
implementation work only after a focused correctness contract and before/after
performance case are agreed.

## Confirmed gaps

| ID | Missing surface | Evidence and benchmark consequence | Candidate closure to evaluate |
| --- | --- | --- | --- |
| CAP-01 | Parse-once JSON object model or indexed document handle | `rxjson` is deliberately string/path oriented and its own documentation says it is not a full object model. `json_parser.crexx` can validate/count the common payload, while ooRexx builds its supplied DOM and NetRexx builds Java collections. The timings are therefore diagnostic and not a common JSON score. | Prototype either a Level B JSON value hierarchy or an opaque indexed document handle with typed object/array/scalar access. Compare parse-once lookup, allocation and repeated path access before selecting an API. |
| CAP-02 | Heterogeneous nested array/reference containers suitable for an array-of-arrays tree | Level B references are present, including references to arrays, but the language reference explicitly excludes nested reference containers. Arrays cannot currently be stored as ordinary `.object` values. `awfy_storage.crexx` therefore represents each logical upstream array with a `StorageNode` plus an `.object[]`, adding an allocation and object dispatch per logical node; its cREXX time is not a common Storage score. | First test the smaller surface: allow a safely owned array value in a heterogeneous object/container slot. Compare that with a purpose-built nested-array or owned-reference container. Preserve weak-reference lifetime rules and typed-array fast paths. |
| CAP-03 | Standard RFC 4648 Base64 library API | Repository search found no Level B Base64 service, so every runtime port contains the full codec loop. This is a library gap, not a byte-language gap: cREXX's pre-sized `.binary` implementation passes the exact 1,024-byte round trip and materially improves on the initial concatenating draft. | Add a pure Level B reference API such as `base64encode(.binary) -> .string` and `base64decode(.string) -> .binary`; retain the benchmark as its contract. Consider a native/SIMD provider only after the portable version is measured. |
| CAP-04 | Public load-only lifecycle timing boundary | The public cREXX, ooRexx and NetRexx command paths expose compile/translate and execution, but not one consistent loaded-without-executing phase. `run_lifecycle.crexx` therefore reports `load_first_result`, not pure loader time. | For cREXX, evaluate an `rxvm` validation/load-only mode or loader counters exposed by the existing VM API. Keep CLI startup, module decode/link and first instructions separately attributable without changing normal execution. |

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

## Closure order

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

# StringTreeMap AVL Review

Status: PoC promoted and hardened on 2026-07-02.

## Outcome

The shelved `StringAvlTreeMap` PoC was promoted into the production
`StringTreeMap` class. The previous array-backed implementation was retained as
`StringOldTreeMap` for performance comparisons only.

The production map now uses:

- integer node handles stored in parallel arrays
- AVL rotations on insert and remove
- physical deletion with a free-list for slot reuse
- iterative ordered traversal for `firstKey`, `lastKey`, `keyArray`,
  `valueArray`, `iterator`, and `toString`
- inline `SETATTRS` only for clearing internal arrays in `free()`

## Verification Added

`testStringTreeMap` now covers:

- empty map behavior
- insert, update, and same-value return code
- strict empty-key versus blank-key behavior
- ordered key/value arrays
- key and value set views
- iterator snapshots
- all four insertion rotation shapes
- leaf, one-child, and two-child removal
- remove/reinsert slot reuse
- free/reuse behavior
- churn with delete and later insert

`stringtreemap_insert_compare` now reports old/new insert, lookup, and ordered
key-array timings while retaining the same smoke-test pass marker.

Focused Debug and Release checks passed:

```sh
ctest --test-dir cmake-build-debug \
  -R 'testStringTreeMap|stringtreemap_insert_compare|classlib_rxdas_inline_preserve_smoke|classlib_rexxdoc_coverage' \
  --output-on-failure

ctest --test-dir cmake-build-release \
  -R 'testStringTreeMap|stringtreemap_insert_compare|classlib_rxdas_inline_preserve_smoke|classlib_rexxdoc_coverage' \
  --output-on-failure
```

## Performance Snapshot

Release build on the local macOS arm64 development machine after direct-loop
lookup tuning:

| Entries | Old insert | New insert | Old get | New get | Old keyArray | New keyArray |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 2,500 | 81,395 us | 6,734 us | 119,909 us | 2,290 us | 206,960 us | 254 us |
| 5,000 | 312,562 us | 14,820 us | 477,744 us | 4,890 us | 730,247 us | 498 us |
| 10,000 | 1,218,911 us | 30,774 us | 1,912,327 us | 11,514 us | 2,727,733 us | 922 us |

These are observational timings, not release thresholds.

## RXAS / rxdas Findings

The original helper-shaped lookup was functionally correct, and the prior
mutating-method scalar-return inliner bug is covered by
`inline_test_mutating_method_scalar_attr_return`. However, generated RXAS still
showed block-expression scaffolding around the inlined `_findNode()` helper.
Rewriting `get()` and `containsKey()` as direct loops reduced Release `get()`
time for 2,500 entries from about 200 ms to about 2.3 ms.

Final Release artifact review:

- source RXAS: 5,648 lines, 594 `linkattr1`, 287 `minattrs`, 5 `call`
- rxdas output: 5,454 lines, 548 `linkattr1`, 276 `minattrs`, 5 `call`
- no `_findNode` procedure remains
- remaining calls are for `StringTreeSet` and `StringTreeMapIterator`
  construction, not the hot `put`, `get`, or `remove` paths

The broader recommendations are consolidated in
`docs/planning/beta-3/notes/levelb-language-improvement-backlog.md`.
That backlog records the remediation split: fix helper-shaped scalar return
lowering primarily in the inliner, use the RXAS keyhole optimizer for local
cleanup only, and add RXAS/VM assists for the remaining indexed attribute-array
cost model.

## Binary Metadata V2 Trial

Status: temporary benchmark-only trial completed on 2026-07-09; experimental
classes were removed after measurement because they did not improve on the
production `StringTreeMap` overall.

Temporary `StringTreeMapV2` kept the same AVL algorithm and public string-to-string map
contract as `StringTreeMap`, but stores node links, node heights, and free-list
entries in packed `.binary` memory instead of parallel `.int[]` arrays. Keys and
values remain `.string[]` arrays so the comparison isolates the binary-memory
metadata surface rather than changing string ownership or tree semantics.

The V2 layout uses fixed Release 1 `.int` binary fields:

- node left child at byte offset 0
- node right child at byte offset 8
- node height at byte offset 16
- node size 24 bytes
- free-list entry size 8 bytes

The generated RXAS confirms that metadata access lowers to direct `bgeti64` and
`bseti64` instructions. It does not reduce call count compared with
`StringTreeMap`; both current generated artifacts have 5 calls, while the V2
artifact is larger in the local Debug build:

| Artifact | RXAS lines | Binary get/set instructions |
| --- | ---: | ---: |
| `StringTreeMap` | 5,648 | 0 |
| `StringTreeMapV2` | 6,233 | 218 |

Local Debug build, compiler opt mode, macOS arm64 observational timings:

| Entries | Old insert | Current insert | V2 insert | Old get | Current get | V2 get | Old keyArray | Current keyArray | V2 keyArray |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 2,500 | 167,013 us | 15,129 us | 18,468 us | 246,072 us | 4,237 us | 4,601 us | 386,026 us | 462 us | 547 us |
| 20,000 | 9,955,560 us | 141,204 us | 172,484 us | 15,142,044 us | 40,586 us | 44,505 us | 24,607,689 us | 3,355 us | 4,049 us |

These numbers are not release thresholds, but they are a useful early signal.
The binary surface is coherent enough to express the structure, yet direct
binary field access currently loses modestly to the already optimized `.int[]`
metadata representation for this AVL workload. That suggests the next review
should compare algorithmic improvements that benefit both implementations
against binary-only opportunities such as better packed-struct helpers,
capacity growth/reserve support, or hot-path intrinsics for repeated
field-offset access.

### Source-Shape and Packed-Field Playoff

Follow-up temporary variants were added to separate source-shape effects from
binary field representation:

- `StringTreeMapV2A` keeps the V2 `.int` binary layout but caches the current
  node key and node byte offset in hot descent/traversal paths.
- `StringTreeMapV3` applies the same source-shape caching to the array/register
  implementation.
- `StringTreeMapV4` keeps the V2A source shape but stores left/right/free-list
  node handles as `.u32` and height as `.u8`, reducing each binary node from 24
  bytes to 9 bytes.

Local Debug build, compiler opt mode, macOS arm64 observational timings:

| Entries | Current insert | V2 insert | V2A insert | V3 insert | V4 insert | Current get | V2 get | V2A get | V3 get | V4 get | Current keyArray | V2 keyArray | V2A keyArray | V3 keyArray | V4 keyArray |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 2,500 | 14,212 us | 17,071 us | 16,996 us | 13,814 us | 15,233 us | 4,303 us | 4,485 us | 4,249 us | 3,868 us | 4,175 us | 433 us | 544 us | 638 us | 440 us | 530 us |
| 20,000 | 144,480 us | 171,789 us | 172,936 us | 152,215 us | 157,822 us | 41,193 us | 47,365 us | 46,679 us | 41,809 us | 43,036 us | 3,611 us | 4,363 us | 4,291 us | 3,619 us | 4,264 us |

RXAS source artifact counts from the same Debug build:

| Artifact | Lines | `bgetu32` | `bsetu32` | `bgetu8` | `bsetu8` | `bgeti64` | `bseti64` | `linkattr1` | `minattrs` | Calls |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `StringTreeMap` | 5,648 | 0 | 0 | 0 | 0 | 0 | 0 | 594 | 287 | 5 |
| `StringTreeMapV3` | 5,558 | 0 | 0 | 0 | 0 | 0 | 0 | 582 | 279 | 5 |
| `StringTreeMapV2` | 6,233 | 0 | 0 | 0 | 0 | 140 | 78 | 385 | 69 | 5 |
| `StringTreeMapV2A` | 6,223 | 0 | 0 | 0 | 0 | 140 | 78 | 373 | 61 | 5 |
| `StringTreeMapV4` | 6,223 | 96 | 39 | 44 | 39 | 0 | 0 | 373 | 61 | 5 |

The immediate lesson is that the register/array representation is already very
strong for scalar AVL metadata. Source-shape caching helps both families.
Packed binary fields help V4 versus the first binary variants, especially on
insert, but this workload still does not show binary metadata as a clear win.
For binary-memory work, the stronger expected use case remains dense byte
scanning and zero-copy compare/extract paths such as JSON lexing/parsing, where
the representation avoids repeated string slicing rather than replacing small
integer arrays.

The experimental classes and cloned tests were removed after this measurement.
The production benchmark remains `stringtreemap_insert_compare`, comparing the
legacy `StringOldTreeMap` with the production `StringTreeMap`.

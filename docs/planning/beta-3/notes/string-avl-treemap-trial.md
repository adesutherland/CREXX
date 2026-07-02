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

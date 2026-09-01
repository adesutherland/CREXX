# Level B Language Improvement Backlog

Status: working backlog, updated 2026-07-02 from the `StringTreeMap` AVL review.

## Evidence Source

The current production `StringTreeMap` rewrite promoted the shelved
`StringAvlTreeMap` PoC into the public `StringTreeMap` class and retained the
previous array-backed implementation as `StringOldTreeMap` for comparative
benchmarks.

Release benchmark on the local macOS arm64 development machine, 2,500 entries:

| Operation | StringOldTreeMap | StringTreeMap |
| --- | ---: | ---: |
| insert | 81,395 us | 6,734 us |
| get | 119,909 us | 2,290 us |
| keyArray | 206,960 us | 254 us |

Manual larger-size spot checks after the final direct-loop tuning:

| Entries | Old insert | New insert | Old get | New get | Old keyArray | New keyArray |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 5,000 | 312,562 us | 14,820 us | 477,744 us | 4,890 us | 730,247 us | 498 us |
| 10,000 | 1,218,911 us | 30,774 us | 1,912,327 us | 11,514 us | 2,727,733 us | 922 us |

Release RXAS/rxdas review of `StringTreeMap` after tuning:

- source RXAS: 5,648 lines, 594 `linkattr1`, 287 `minattrs`, 5 `call`
- disassembled RXBIN: 5,454 lines, 548 `linkattr1`, 276 `minattrs`, 5 `call`
- the remaining calls are set/iterator construction, not `put`, `get`, or
  `remove`

## Helper-Shaped Lookup Remediation Roadmap

The `_findNode()` finding should be handled as a layered improvement, not as a
single optimizer catch-all.

1. Inliner: primary owner.

   The inliner should learn a cheaper lowering for simple value-return helpers
   at simple assignment/test call sites. The target shape is direct caller
   control flow: map callee `return value` to the caller's target register plus
   a branch to one continuation, without wrapping the inlined body in
   block-expression scaffolding when the parent context does not require it.
   This is the layer that still has source-level knowledge that `_findNode()`
   is a private read-only helper returning a scalar node id.

2. RXAS keyhole optimizer: secondary cleanup.

   The keyhole optimizer can trim local waste after lowering: redundant typed
   copies, branch-to-branch shapes, duplicate linked reads, and adjacent
   attribute-read cleanup where no barrier or mapped-register use intervenes.
   It should not be expected to reconstruct a direct source loop from a
   heavyweight block-expression expansion.

3. RXAS/VM assists: remaining cost-model work.

   Even after the direct-loop rewrite, array-backed collections still pay the
   current indexed attribute-array sequence cost: `linkattr1`, often
   `minattrs`, typed copy, and `unlink`. Typed one-based indexed get/set
   assists, plus strict read forms that do not grow/check capacity like writes,
   would help all Rexx-native collections.

4. Current Level B guideline.

   Helpers remain appropriate for clarity in ordinary code. For measured
   classlib hot paths, use the direct source shape until RXAS/rxdas inspection
   proves that helper inlining produces equivalent bytecode.

## Backlog Items

1. Add a typed array-clear source surface.

   Current classlib code still uses `assembler SETATTRS array,0` to clear
   `.int[]` and mixed internal arrays. `arraydrop` and `objectarraydrop` cover
   common public helper cases, but data-structure internals need a documented
   typed/generic clear operation so ordinary Level B code does not need inline
   assembler for this.

2. Add faster indexed attribute-array read/write lowering.

   Hot data-structure code currently lowers each indexed attribute-array access
   through `linkattr1`, often `minattrs`, a typed copy, and `unlink`. A
   compiler or VM assist for typed one-based attribute-array get/set would
   materially reduce the cost of Rexx-native collections without forcing them
   into native plugins.

3. Distinguish read-only indexed access from growth-capable writes.

   RXAS for reads still shows `minattrs` in many indexed array reads. A source
   or compiler distinction between strict read and grow-on-write would let hot
   lookups avoid capacity checks that are only meaningful for assignment.

4. Keep helper-inlining performance visible.

   The `_findNode()` helper was semantically inlined but left enough
   block-expression scaffolding to make `get()` roughly two orders of magnitude
   slower than the direct-loop form at 2,500 entries. The inliner is now
   functionally correct for the earlier mutating-method return bug, but hot
   helper forms still need RXAS-level performance review.

5. Audit Rexx BIF inline metadata and hot BIF bodies as a separate pass.

   `library.rxbin` preserves `META_INLINE`, and `lib/rxfnsb/rexx` builds with
   `rxc -x --import-rxas`, but there are about 140 Rexx BIF source modules.
   This tree-map change did not require BIF tuning because the final map hot
   paths call no BIFs. A dedicated BIF pass should verify inline availability,
   generated RXAS size, and hot functions such as `substr`, `left`, `right`,
   `word`, `words`, and `pos`.

6. Clarify external access to indexed array attributes.

   A benchmark diagnostic could read scalar attributes such as `root_`, but
   expressions like `tree_map.height_[tree_root]` were rejected. If indexed
   array attributes are intended to be externally readable, the source grammar
   and docs should say how; if not, the class/private-attribute direction should
   make that explicit.

7. Consider a white-box test/debug hook pattern for core collections.

   Balance and node-pool invariants are useful to test, but exposing production
   methods solely for instrumentation would expand the public surface. A
   standard test-only or doc-private hook pattern would let classlib tests check
   internals without committing them as stable user APIs.

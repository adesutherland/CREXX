# Develop integration and RS-F01 closure — 11 October 2026

`levell` now includes develop **54f736804de08684852c63eb7673435157ef1e30**,
fast-forwarded from `91313bcc5a447e9403f95b1f0bfc4dd9503d0618`. The local Level L
and RexxScript implementation remains uncommitted and unpublished. No changes
were merged back into develop, and the primary checkout was not edited.

The integration completed without conflicts. Tracked local edits were stashed
and restored; untracked work stayed in place. The only overlapping source was
`tools/build-system/build_stage_g.crexx`, whose restored diff preserves the
Level L Unicode dependencies alongside upstream changes. The pre-merge manifest
and tracked patch preserve the prior state. Immediately after integration,
979 preserved inputs were byte-identical; only the merged driver and the new
plan record differed. The unrelated performance roadmap edit is unchanged.

## Verification

| Check | Result |
|---|---|
| Fresh native core tools | Rebuilt rxc, rxas, rxlink, both VMs, rxdas and the numeric-test helper from the integrated sources. |
| Upstream numeric contracts | **2/2 pass:** `float_constant_folding_contract` and `numeric_constant_contract`, totaling **632 tool commands / 264 VM executions**. Both compiler/assembler modes, B/G, imports and linked images are covered by those maintained contracts. |
| Original RS-F01 reproducer | **4/4 cells return 0:** optimized/unoptimized compilation, both VMs, through assembler and linker. |
| Scalar compiler/interpreter overlap | **28/28 agree** across optimized/unoptimized Level G and both VMs, and with the rebuilt native RexxScript runner. **Zero optimizer disagreements.** The maintained harness now rejects the old discrepancy instead of allowing it as a known exception. |
| Affected consumers | **8/8 CTests pass:** generated language on both VMs, RexxScript's 60-case panel on both VMs, four Unicode normalization cells. |
| Native CLI | **6/6 checks pass:** successful example, I/O, UTF-8, parse, undefined-variable and capability failures. |
| Bootstrap agreement | Rebuilt L1/L2 generated RXAS match exactly and retain the pre-merge artifact hash. |
| Unicode generation | Regenerated logical normalization source is byte-identical to the retained pre-merge source. |
| Build diagnostics | Native core and consumer compilations are warning-free. |

**RS-F01 is closed locally. RS-AC04 is verified for the declared scalar
NUMERIC_COMMON profile.** This is not complete Level G compatibility or closure
of the remaining language-tooling research programme.

The upstream repair keeps typed numeric payloads separate from display text.
It does not change binary floating-point addition into decimal arithmetic:
`0.1+0.2=0.3` remains false, while the decimal control remains true. The upstream
[repair receipt](../../../../docs/qa/float-constant-folding-2026-10-11/README.md)
and [follow-up fixes](../../../../docs/qa/float-constant-folding-2026-10-11/related-repair/README.md)
explain the implementation and broader edge cases.

## Retained evidence

- [merge.json](merge.json), [pre-merge.json](pre-merge.json) and
  [tracked-before.patch](tracked-before.patch): integration and preservation.
- [manifest.json](manifest.json): exact current sources, tools, build inputs and
  result hashes. The reproducer comment and documentation were updated after
  the run; its executable statement is unchanged. The run receipt retains the
  original qualified source hash.
- [numeric-ctest.xml](numeric-ctest.xml), [numeric counts](numeric-counts.json),
  [numeric test log](logs/numeric-tests.log) and full command logs under `logs/`.
- [reproducer/receipt.json](reproducer/receipt.json),
  [scalar/receipt.json](scalar/receipt.json), [cli/receipt.json](cli/receipt.json).
- [consumers-ctest.xml](consumers-ctest.xml),
  [consumer build](logs/consumers-build.log), [consumer tests](logs/consumers-tests.log).
- [stages.json](stages.json), [unicode-regeneration.json](unicode-regeneration.json).

The first multi-target build regenerated the makefiles and built the requested
core binaries, but its original make invocation did not see the newly added
`mutate_rxbin_float` target. A fresh invocation built that helper successfully;
the two numeric contracts then passed. The initial failure is retained in
`logs/core-build-initial.log`; it was build invocation state, not a product or
test failure.

Qualification is focused native macOS ARM64 Debug integration. The prior
research/bootstrapping receipts remain historical evidence with their original
source/tool identities. No fresh full no-opt Level L build, broad CTest,
sanitizer, release/install or cross-platform sweep is claimed. Upstream's
separately retained broad qualification was not repeated. All original open
research criteria and the remaining diagnostic/recovery work remain open.

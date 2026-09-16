# Issue #699 repair and qualification

## Outcome and scope

The repair keeps exact private callable/class dependencies from opening a
namespace to unrelated source extensions. Source discovery now requires the
current source namespace or an actual `IMPORT` in the current program. Parsed
header imports and compiler-generated imports both count; materialized imported
declaration stubs do not. The scan reads live declaration/header nodes only and
does not cache visibility or traverse executable bodies.

This corrects the proved #699 trigger using the existing importer. It preserves
inline metadata, exact dependency lookup, normal namespace extensions and all
language/API surfaces. It does not introduce general support for arbitrary
cyclic source contracts or claim to repair every possible incomplete-contract
cycle. The [initial assessment](README.md) remains historical evidence.

Adrian authorized repair/publication after the assessment, then explicitly
requested full regression testing. The authoritative [plan](../../planning/issue-699-source-import-diagnosis.md)
retains both decisions. The main CREXX/llama and downstream RAG checkouts remain
untouched; no model workload, release/tag or installation is part of this work.

## Ownership and regression contracts

The only new allocation is normalization of an `IMPORT` name. It is local to
the predicate and freed on every path. AST/scope pointers are borrowed for the
call only. There is no new persistent state, scope mutation, import-cache
invalidation rule or lifetime extension. Source candidates retain the existing
header-normalization and dependency-snapshot machinery.

Two permanent smoke-tier tests independently cover the convergence and provider
failures. Each checks opt/noopt source compilation, `lineout` control, unchanged
dependency snapshots after an unused extension body edit, RXBIN import controls,
explicit extension use/argument validation, generated ADDRESS imports and own
namespace discovery. The provider test additionally rejects a class whose
`implements` clause has actually been removed. Tests compile but never execute
the cleanup fixture, which refers to stdout.

The unmodified installed baseline (`17e844441`) fails the new tests with the
original convergence and provider mismatch diagnostics. Each test's CMake return
code is 1 because the harness correctly rejects the compiler failure; retained
output gives the underlying compiler codes 255 and 2. These are ordinary
compile-success regressions, unlike the historical `diagnose.py` script that
intentionally asserts the defective baseline.

The matrices were measured separately: normal Debug 10.35 s / 13.98 s and Apple
ASan 35.66 s / 50.57 s (convergence/provider). Both use `RUN_SERIAL TRUE` and a
300-second CTest backstop. There are no timing assertions, retries or new nested
builds. Each declares `rxc`, `rxas`, `library` and `classlib` preparation targets.

## Qualification

- Baseline failure: `repair-evidence/baseline-regressions.log.gz` and `baseline-regressions.json`.
- Core product: `cmake --build cmake-build-debug --target stage-product
  qa-prep-smoke --parallel 8`, pass; `repair-evidence/core-build.log.gz`.
- Original downstream trigger: direct rebuilt Debug `rxc` against the retained
  reconstructed source graph and installed unchanged binary imports. Both
  `ragcommand` and `ragprocess` pass opt and noopt with `closefile` restored.
  Exact commands, logs and assembly hashes: `repair-evidence/original-modules/`.
- Maintained sanitizer: `tools/asan-run.sh --phase build --build-target rxc
  --build-target rxas --build-jobs 8 --build-leaks off --no-live-tail`, then
  `tools/asan-run.sh --phase ctest --regex
  '^(source_import_private_dependency_.*|binary_forward_dependencies|binary_storage_selector_dependencies|source_import_contract_stub_.*|global_import_types)$'
  --test-jobs 1 --leaks off --no-live-tail`: **7/7 pass**, 195.20 s.
  Apple LeakSanitizer is unavailable; this is focused Apple ASan evidence, not
  Linux leak or full cross-platform sanitizer qualification. No finding arose.
- Full normal correctness: `cmake --build cmake-build-debug --target
  qa-comprehensive --parallel 8` — **2,294/2,294 pass**, 792.65 s CTest time;
  `repair-evidence/full-correctness.log.gz`.
- Hotfix publication and develop promotion: local qualification complete; publication next.
- Automatic publication workflows: pending promotion; no manual deep or full
  sanitizer matrix dispatched. Normal workflows run on develop, not hotfix.

`repair-evidence/qualified-inputs.json` freezes the production/test inputs so
documentation bookkeeping and history-only promotion can reuse valid evidence.
Platform cells without completed hosted evidence remain unverified.

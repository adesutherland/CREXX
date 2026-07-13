# Stage Reviews

This file records the required completeness and task-compliance review performed after each approved execution stage.

## Stage 1: taxonomy and boundaries

State: approved by Adrian before this catalogue was created.

The approved scope, independent classification axes, Level C split rule, public-leaf granularity, and non-re-engineering boundary are preserved in `README.md`.

## Stage 2: raw catalogue

State: complete; reviewed before Stage 3 began.

### Completeness checks

- The first raw sweep contained 2,028 stable IDs. Reconciliation added eight typed intrinsic/reserved/documentation identities, 21 missing primary tool/runtime entries, and then the explicit debugger/macro audit added 96 entries. The maintained raw catalogue now has **2,153 stable, unique IDs**: 137 syntax/preprocessor capabilities, 386 namespace exports, 917 public members, 422 unique native RXPA registrations, 70 Classic BIF names, eight typed intrinsic/reserved/documentation-only BIFs, 70 RXPP macro/included-procedure leaves, and 143 package/tool/source/demo/example/PoC entries.
- All 131 active `lib/rxfnsb/rexx/CMakeLists.txt` selectors have a source file and at least one raw catalogue row.
- All 36 `classlib` and `classlib_native` CMake selectors have a source file and a raw catalogue row.
- All 27 `lib/plugins` directories, both VM decimal-plugin directories, and all 16 compiler-exit directories are represented.
- All 423 in-scope `ADDPROC` registration sites are represented as 422 public leaves. The difference is the identical `console.setcolor` registration appearing in two platform branches; it is one contract and the catalogue records both sites.
- The first validation pass found omitted nonstandard roots. The inventory was extended to cover `lib/rxfnsb/native/des`, active and residual `lib/rxfnsb/rxas` files, the unbuilt `lib/rxmath/rexx` source candidates, and compiler-exit token infrastructure.
- Public class/interface members are included only when the owner is namespace-exposed. Ten methods on non-exported `rxfnsc` implementation classes were correctly left as implementation evidence rather than public leaves.
- Demonstrations, examples, and both compiler PoC directories are represented individually.
- RXDB is represented as a tool, two namespace exports, and 18 public UI members. Private `main`/helper procedures are intentionally not product leaves.
- RXPP is represented as syntax, tool/optional shell, its internal `precomp` package and 20 native registrations, all four staged support files, all 64 shipped macro definitions, all six `mathlib` procedures, and the unselected `rxppt.crexx` workbench candidate.

### Compliance checks

- Raw files record source/build facts only. They do not assign product role, delivery status, quality, or proposed level.
- Level C BIF names have independent `CBIF-` identities and are not merged with identically named typed-library functions.
- Private helpers and internal compiler/VM/RXAS implementation details are not promoted to product entries.
- No implementation, CMake, packaging, API, or language files were changed. Existing profiling work remains untouched.

### Known boundary carried into reconciliation

- Namespace exposure is treated as the source declaration of public visibility, even for internal-looking underscore symbols. Stage 3 must compare that with linked metadata and documentation before Stage 6 proposes whether the exposure is intentional.
- CMake presence is only a discovery fact. Active build, conditional build, linked-image inclusion, packaging, and source-only status are reconciled in Stage 3.
- The syntax catalogue records capability-sized items rather than every token or grammar production; the dedicated RXAS instruction reference remains outside this duplicated inventory.

## Stage 3: reconcile completeness

State: complete; reviewed before Stage 4 began.

### Completeness checks

- All eight configured linked library/exit images were inspected from a fresh Debug build. Every active source namespace export and every catalogued public member/factory reconciled with linked metadata after normalising factory encoding.
- The source, CMake selection, declared target forms, registered-test shape, and current Debug artifact evidence were recorded for all 27 RXPA plugin directories.
- Both VM decimal plugins were reconciled, including the distinction between the `mc_decimal` implementation linked into normal VMs and the alternative `db_decimal` implementation used by explicit tests.
- Default build and install effects were checked for every main linked library, RexxScript, compiler exits, and `veclib`.
- The 1,608-test configured CTest registry was sampled by product surface and traced back to subsystem CMake. Test registration is explicitly not treated as proof of complete semantic coverage.
- BIF documentation, the 70-name Level C recognition table, active runtime dispatch, Release 1 planning policy, and the legacy BIF pages were reconciled. Eight non-ordinary typed BIF identities discovered here were added to the raw catalogue.
- Nineteen residual or product-boundary discrepancies are retained in a register with an explicit later-stage or later-approval disposition.

### Validation checks

- The Stage 3 library-plugin matrix contains 27 rows, matching both the `lib/plugins` filesystem count and those 27 raw package IDs; RXPP's separate internal `precomp` package is reconciled in the tooling section.
- The linked-image table contains all eight inspected images; the library delivery table additionally accounts for `veclib` and the unbuilt old `rxmath` tree.
- All currently present canonical raw/reconciliation files named in `README.md` exist. The four later-stage files are intentionally created by Stages 4-7.
- The maintained raw files contain no duplicate stable ID.
- The debugger/macro correction was audited mechanically: 64 `##define` lines, six included procedures, two exposed RXDB symbols, and 18 public RXDB UI members match their raw rows.
- `git diff --check` reports no whitespace errors in the catalogue tree.

### Compliance checks

- Reconciliation reports current facts and discrepancies; it does not infer “core” from build presence or settle provisional classifications early.
- Level C recognition and implementation are kept separate from typed-library contracts.
- No implementation, build, packaging, API, or language source was changed. Linked-image disassembly was written only to temporary files outside the repository.

## Stage 4: organise by purpose

State: complete; reviewed before Stage 5 began.

### Completeness checks

- Fifteen purpose domains cover toolchain/language, runtime control, text, numeric, binary, collections, local system, network, parsing/data, object model, external integration, Level C/RexxScript, Level G, Level L, and example/PoC purposes.
- Deterministic inheritance rules assign every raw export, public member, and native registration through its source selector or containing package without duplicating the leaf catalogue.
- All typed, Classic, and RXPP syntax rows have either an explicit purpose exception or a stated default rule. All 70 Classic BIFs and all eight typed intrinsic/reserved/doc-only BIFs are assigned.
- All 131 active `rxfnsb` selectors, 36 classlib/classlib-native selectors, 27 RXPA directories, two VM plugins, 16 compiler exits, tools, residual sources, demonstrations, examples, and PoCs have a primary purpose.
- The RXPP macro/included-procedure leaves inherit a defined purpose by support file and function family; RXDB public leaves inherit the debugger/tooling purpose.
- A duplication map records the major same-purpose implementations without claiming that they have equivalent contracts.

### Validation checks

- The `rxfnsb` purpose table was mechanically compared with the CMake selector list: 131 versus 131, no missing selectors, no extras, and no duplicate assignment.
- The native-package matrix contains the 27 distinct `lib/plugins` package IDs plus the separately assigned internal RXPP `precomp` package, matching Stage 2 and Stage 3 boundaries.
- The exit/tool matrix contains 16 distinct exit IDs.
- The maintained raw inventory remains 2,153 IDs and all 2,153 are unique.
- `git diff --check` reports no whitespace errors in the catalogue tree.

### Compliance checks

- Purpose is kept independent of product role: examples are grouped as examples, but build presence, maturity, and core/optional recommendations are not assigned here.
- Level C and RexxScript share a compatibility purpose domain but remain separate components and contracts.
- Primary purpose is single-valued; secondary example tags and duplication relationships do not create duplicate entries.

## Stage 5: initial quality assessment

State: complete; reviewed before Stage 6 began.

### Completeness checks

- The assessment separates test evidence, documentation, performance risk, confidence, source composition, and runtime call path; it assigns no composite score.
- Language/compiler surfaces, all 131 Level B selectors, all 36 classlib selectors, G/C/L libraries, RexxScript, `veclib`, all 27 RXPA directories, both VM decimal plugins, native DES, all 16 exits, residual sources, demos/examples, and PoCs are covered either directly or by a documented source/package inheritance rule.
- Dependency/blast-radius relationships are explicit, including the Level B-to-classlib-to-Level C-to-RexxScript chain and RXPP's four linked runtime inputs.
- Performance findings distinguish source-review risk from measured speed. Hot-path, repeated-scan/copy, manual-memory/registry, stateful I/O, and external-service concerns are recorded without inventing benchmark numbers.
- Use of cREXX features is reviewed separately for B, G, C-runtime implementation, L, classlib, RexxScript, and RXPA packages.
- RXDB, the hybrid RXPP engine, `precomp`, each shipped RXPP support library, and the residual RXPP workbench have explicit composition, evidence, documentation, performance-risk, and source-review findings.

### Validation checks

- A focused Debug run selected 115 representative tests across all major library levels, classlib, RexxScript, RXPP, certified exits, decimal plugins, and selected native plugins: 115 passed and zero failed. A non-overlapping 24-test tool/runtime selection also passed, and the RXDB usage smoke passed separately: 140 selected tests passed and zero failed in total.
- The native assessment table has 27 rows and a mechanical name check found no missing plugin directory.
- The exit assessment names all 16 exit directories.
- Top-level RexxDoc counts were rechecked: 24 blocks across 134 Level B library source files and 329 blocks across 37 top-level classlib `.crexx` files; the classlib coverage regression test passed.
- The maintained raw inventory remains 2,153 unique IDs, and `git diff --check` reports no catalogue whitespace errors.

### Compliance checks

- The focused run is described as test evidence, not coverage, conformance, cross-platform certification, or a benchmark.
- Level C parser recognition, runtime implementation, and standards compliance remain three distinct facts.
- Pure Rexx source is not used to conceal transitive VM/native dependencies; source composition and call path are both reported.
- No re-engineering or source/build/package change was made. The assessment ends with later questions, not fixes performed during this task.

## Stage 6: provisional classifications

State: complete; reviewed before Stage 7 began. Recommendations remain
unapproved.

### Completeness checks

- Classification inheritance covers every one of the 2,153 raw IDs by syntax family, source selector, or containing package; explicit exceptions are named.
- Every syntax capability has a B/G/C/L or shared-tooling disposition. The 80 non-selector typed capabilities are B+G; the B, G, and L selectors remain level-specific; all 36 Classic rows are unique C capabilities; all 18 RXPP rows are shared tooling rather than language-level syntax.
- All 131 Level B selectors are present exactly once in three explicit groups: 87 bootstrap-core candidates, 35 standard/default selectors, and nine optional selectors.
- All 33 main classlib selectors, three native-backed classlib selectors, five G exports, 70 unique C BIFs, 12 L exports, RexxScript, `veclib`, RXDB, RXPP, 64 macros, six included procedures, and residual sources have a stated disposition.
- All 27 native plugin directories, both decimal plugins, native DES, 16 compiler exits, primary tools/runtimes/host APIs, profiling components, examples, demos, and PoCs are covered.
- Source composition and runtime call path are separate, so pure Rexx source with inline RXAS/native/external dependencies is not mislabeled as an entirely Rexx-only implementation.

### Validation checks

- The three Level B classification rows were mechanically compared with `RXXFNSB_REXX_SELECTORS`: 131 versus 131, with no missing, extra, or duplicate selector.
- The native-plugin disposition table was mechanically compared with the 27 `lib/plugins` directories: 27 versus 27, with no missing or extra package.
- All 16 `EXIT-*` IDs occur in the exit classification and all 2,153 raw IDs remain unique.
- Level C uses an explicit 19-name implemented set plus its exact complement within the 70-row raw list; only `LENGTH` and `SUBSTR` are claimed as proven executable lowering paths.
- `git diff --check` reports no catalogue whitespace error.

### Compliance checks

- Classifications are labelled provisional and perform no language, API, build, install, packaging, or source change.
- Level C identities remain separate from same-named B/G functions.
- The B core list is a conservative candidate pending dependency tracing, not a premature re-engineering scope.
- Experimental/debugger/macro findings are not hidden by current build/install presence.

## Stage 7: cross-cutting conclusions

State: complete; reviewed. Decisions are ready for the later approval stage.

### Completeness checks

- Conclusions cover Level B closure/performance, Level G scope, Level C compliance, Level L maturity, RXPP/macros, RXDB, plugins, compiler exits, packaging, quality gates, profiling, and catalogue maintenance.
- Eleven concrete approval decisions are listed. Each is traceable to raw discovery, reconciliation, purpose, quality, or classification evidence.
- The proposed follow-on order starts with approval and dependency/measurement work, not indiscriminate optimisation of the present library image.

### Validation and compliance checks

- Numerical claims reconcile: 2,153 unique raw IDs; 131 Level B selectors; 70 Level C BIF identities; 64 macros plus six included procedures; 27 plugin directories; 16 exits; and 140 selected passing tests.
- Conclusions distinguish current repository fact, provisional recommendation, and future implementation work.
- The requested later Level B re-engineering is explicitly out of scope and was not performed.
- No source, build, install, packaging, or public contract file outside this documentation catalogue was changed by this task; unrelated profiling work remains untouched.

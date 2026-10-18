# Level L language tooling research — 10 October 2026

## Repository architecture and cleanup

**Status: reorganisation and final naming/library-only refinement verified locally; authorised for publication to develop.**
This section owns the repository/layout refactor. The compiler subplan moves to
`research/level-l/compilers/PLAN.md` with its implementation. Work uses the main
CLion `develop` checkout. B and G share the typed compiler core; Classic is
independent. C-first delivery, Level B implementation, deferred INTERPRET and
paused CI monitoring remain unchanged. Approval covers the directory, service,
build/install and cleanup changes below. Adrian authorised committing and pushing
the completed work on 17 October 2026, followed by a whole-repository review.
That wider review must present its proposal for approval before implementation.

The repository-wide documentation/output rule in `AGENTS.md` is already
instructed by Adrian and takes effect now: only enduring documentation belongs
in the tree. Delete logs, raw runs, receipts and obsolete notes after extracting
useful conclusions; do not keep them in evidence directories or external
archives. This rule applies to all initiatives, not just this refactor.

### Vision and intended outcome

Make Level L a clearly owned, reusable toolchain and SDK, independent of the
experimental source languages. Keep the general Level L libraries under
`lib/rxfnsl`; give the Level L command and its callable tool implementation a
normal tool directory. Put development of the new compilers under research.
Classic has an independent frontend and compiler service. B and G share a typed
compiler core, with explicit language profiles and their own commands/services.
Reusable backend and file/metadata utilities are shared through libraries, not
through a common application main or a dispatcher that owns every language.

The completed tree should contain maintained source, tests/fixtures/examples,
current architecture/contracts/guides/plans and lasting decisions. It should
not accumulate run output or dated narrative. Keep current correctness and RXAS
quality while changing ownership; this is not new language coverage, a default
compiler replacement, a new runtime or an INTERPRET design.

### Findings from the source and build review

| ID | Current finding | Consequence / proposed correction |
|---|---|---|
| RA-F01 | `lib/rxfnsl/CMakeLists.txt` adds `rexx`, `tools`, `compilers` and tests. `tools/CMakeLists.txt` owns bootstrap, the final `rxfnsl` library, the executable and a Unicode consumer grammar. | A library directory owns tools and experimental compilers. Separate their source and CMake owners; keep one producer for every bootstrap output. |
| RA-F02 | `tools/levell.crexx` combines CLI parsing/file I/O, admission, table construction and both source/RXAS generation. Its emission routines print with SAY. | Extract a callable Level L generation service using Level L output/writer services, then make the command handle options, I/O and reporting. Library clients must not call main or capture stdout. |
| RA-F03 | `driver/llrxc-c.crexx` and `llrxc-g.crexx` call `llcompilercli..run(level,argv)`, which calls `llcompiler..request(level,...)`. `llcparse` imports and initialises both grammars. `llfrontend` interleaves C/G binding, type, expression, call and statement decisions. | Separate Classic and typed frontends/services. Sharing an argument parser or output publisher is fine; the common language-selecting application is removed. |
| RA-F04 | `common/llcrexxir` registers both semantic families. `rxas/llrxas` combines generic target mechanics with C entry/context/error code and typed operations, controlled by `_level`. | Separate language family registration and selectors from reusable target mechanics. Keep Classic policy with C and typed policy in the B/G core. |
| RA-F05 | Both command link lists contain both grammars/frontends; `llcompilers.rxbin` combines their reusable services. There is no implemented new B frontend. | Command names currently overstate isolation. Split linked service products and verify their actual transitive closures. Prepare a B/G common core without claiming B implementation/self-hosting. |
| RA-F06 | `stage-l-libraries` requires the experimental services, target adapter and C/G commands. Root install copies the build `bin/` directory broadly. | Separate library/tool/research stages and prevent old or disabled research outputs leaking into product installs. |
| RA-F07 | Generic Level L IR/AST/passes/writer modules are already independent of C/G. Classic `llctarget` is separately built against the target runtime and is not linked into the compiler host. | Preserve these good boundaries. Do not move C/G semantic policy into Level L or merge target runtime with compiler-host libraries. |
| RA-F08 | Research contains 14 source symlinks to maintained L library modules; these are not independent implementations. The L library's `examples/ir/demo.crexx` imports compiler-owned `llcrexxir`. | Point maintained research runners at canonical modules; remove redundant source aliases once callers migrate. Move the compiler-specific demo into compiler research; keep genuinely generic examples in L. |
| RA-F09 | The tracked `research/level-l/evidence` tree has 16,198 files, about 129.6 MiB, including 12,154 logs and 671 RXAS dumps. Compiler evidence adds 199 tracked files, about 3.2 MiB; current untracked qualification output also exists. | Delete disposable output and obsolete narrative. Keep lasting conclusions in current docs; do not relocate this bulk into another evidence/archive tree. |
| RA-F10 | Some things called evidence are live inputs: `check-gennorm2-compat.py` reads `bootstrap/reference-gennorm2.crexx`; research cases read oracle/corpus text from evidence. Several guides describe stale phases/layouts. | Move real inputs into owned fixtures with provenance, update callers, then delete the old evidence trees. Consolidate current guides/plans rather than keeping parallel histories. |

Static inspection verifies these ownership findings, not the proposed new
implementation. The working checkout and remote develop both named
`e4ee47528fe60e76a81f6d18a7849aa8aa663037` at review. STEP07A/SAY publication
was subsequently authorised. No product tests were rerun for the architecture review.

### Approved directory and product ownership

Use **`langtools/` as Level L itself**, as Adrian clarified. It includes lexer
generation, parsers, source and tree services, IR and emission; `parser` would
understate the scope. Do not add a redundant `langtools/level-l/` subtree.

```text
langtools/                  Level L itself; no second level-l subtree
  common/                  B-only tool support libraries; no language frontend
  rxlang.crexx             rxlang command options, I/O and diagnostics
  cmake/                   generation/build adapter
  rexx/                    llfront and callable grammar-generation/emission service
  grammar/                 the Level L language's own grammar
  bootstrap/               seed reader, L0/L1/L2 construction and verification
  tests/                   tool/bootstrap/delivery and independent library regressions
  examples/                all maintained general Level L examples
  doc/                     SDK/tool/service/bootstrap reference and user guides
lib/rxfnsl/
  rexx/                    generic lexer/parser engines, AST, IR, passes, writer
  unicode/                 library-owned pinned property data and its generation
research/level-l/
  PLAN.md                  this programme and repository refactor
  compilers/
    PLAN.md                moved compiler plan and C instruction ledger links
    common/                shared cREXX compiler/ABI facilities only
    typed/                 substantial shared B/G grammar/frontend/types/lowering
    levelb/                B profile, service and command when STEP07B is implemented
    levelg/                G profile/extensions, service and command
    levelc/                Classic grammar, token policy, semantics and service
      runtime/             llctarget adapter, separate from compiler host
    tests/ examples/ doc/   compiler-owned maintained assets
```

All general Level L guides, examples and independent library tests live in
`langtools`. Only library implementation and required build data remain under
`lib/rxfnsl`; independent library qualification is preserved in its new location.
Research engines still serving open research questions remain in research;
this review does not promote unfinished Pika/Squirrel/recovery experiments.

**Move/split map:**

- `lib/rxfnsl/tools` and `bootstrap` move to the Level L tool owner. Split the
  current generator command into its real CLI and callable implementation.
  Move `llfront.crexx` and `grammar/levell.levell` with that implementation.
- Build the generic SDK as `rxfnsl.rxbin`, without the Level L specification
  compiler. Build that compiler's library separately as **`rxlanglib.rxbin`**,
  retaining the `llfront` namespace and providing the extracted generation
  service. Consumers of `llfront` must explicitly use the tool library. This is
  an intentional package boundary change, not an unexplained missing import.
- `grammar/gennorm2.levell` belongs to its Unicode consumer under
  `lib/rxfnsg/unicode`; `tinyexpr.levell` belongs with its library example.
  Keep generated syntax/table products in the build tree, not in source.
- Move the compiler subtree, plan, instruction ledger and maintained examples
  to `research/level-l/compilers`, then split the mixed implementation as above.
  Preserve the STEP07A/SAY fixes and regression fixtures in the move.
- Split general RXBIN envelope/metadata reading, RXAS literal escaping and file
  loading/publication utilities into `langtools/common` only where they have a
  demonstrated shared use. The semantic interpretation of C/B/G metadata and
  source rules stays with the owning compiler. Do not promote the entire
  current `llcompilercontracts`, `llcrexxir` or `llrxas` module as generic L.
- Keep AST/IR/source/origin/writer infrastructure and generic transformations in
  `lib/rxfnsl`. Shared tool helpers may use these; they may not import research.
  Extract the smallest concrete utilities, not a speculative universal backend
  framework or a new IR/file-format redesign.

### Frontend and service boundaries

**B/G share most implementation.** `typed/` owns their common lexical/grammar
machinery, tree construction, scopes, binding, types, defaults/presence,
imports, calls, semantic checks and lowering as those capabilities are added.
G composes that core with G-specific additions; B selects its defined profile
without importing G extensions. Use one maintained representation of common
rules, not copied B/G grammars that drift. A profile does not promise that every
G feature is valid B. Exact source contracts and their negative tests decide.
STEP07B still owns implementing and qualifying B; the refactor does not invent
B coverage or implement self-hosting.

**Classic C is independent.** It owns contextual token rules, AST schema,
variable pools, numeric state, calls, condition/TRACE semantics and target
lowering. It shares general L machinery and proven target/IO utilities with the
typed core, but does not import a typed frontend or use a common C/G parser.

Each command owns the orchestration for its language and calls its own reusable
service: proposed `llccompiler..request(source,...)` and
`llgcompiler..request(source,...)`, with `llbcompiler` later. Retain existing
source-length, logical-name, budget, request/result lifetime, no-output-on-error
and optimisation controls. These services do not print, exit, publish files or
call an application main. Shared helpers parse common options and publish owned
output; they do not choose the source language or discover/load another compiler.

Replace the experimental combined `llcompilers.rxbin` with **`llccompiler.rxbin`**
and **`llgcompiler.rxbin`** and explicit shared dependencies. Migrate repository
service clients and docs together; retire `request(level,...)` rather than
keeping a compatibility multiplexer that restores the coupling. Preserve the
CLI names `llrxc-c` and `llrxc-g` and their existing options. The future B entry
composes the typed core, not the G command. Generated C VM launchers remain
packaging glue; all compiler algorithms and extracted helpers remain Level B.

The shared backend receives verified IR and explicit target/ABI contracts.
Compiler-specific family verifiers and selectors are supplied by that compiler;
the target engine does not select C/G semantics by command name. Preserve
existing operation identities and metadata meaning during extraction. Any new
public extension protocol beyond the already reviewed L services needs its
own concrete API review; this is not authority for a new language design.

### Build, install and consumer boundaries

The intended dependency order is:

```text
existing native B seed + B runtime
  -> generic rxfnsl
  -> Level L L0 -> L1 -> L2 -> rxlanglib + rxlang
  -> generated language modules and ordinary consumers
  -> explicitly selected experimental C / typed-G compiler products
```

No SDK or Level L tool dependency points back into compiler research. The seed
remains a build prerequisite; new source requests never invoke it. The existing
Classic target-runtime package's G dependency is separately documented and is
not removed or hidden by this host-compiler refactor.

Propose `stage-l-libraries` for the SDK, `stage-l-tools` for `rxlanglib`/`rxlang`,
and `stage-research-compilers` for experimental services/commands/adapters.
The normal `stage-product` includes the first two. Research builds/install are
explicitly selected with a proposed `CREXX_BUILD_RESEARCH_COMPILERS` option,
**default OFF**, and a `ResearchCompilers` install component. A language selection
allows C-only or G-only preparation; B is unavailable until implemented. Normal
CI must explicitly enable and exercise maintained C/G tests so opt-in packaging
does not silently remove coverage. This delivery-policy change was approved with the reorganisation. Experimental
commands remain available through the explicit research component.

Research outputs must use their own build/output directory; root's broad `bin/`
install must not scoop up disabled or stale experimental files. Explicitly
exclude retired related filenames from incremental product installs, update
product inventory tests, wrapper generator paths, package preparation and QA
producer registration. Do not delete unrelated files from existing install
prefixes. Prove fresh and incremental installs separately.

Move generator-dependent tests to the tool owner; pure SDK tests must not
require `rxlanglib` or research compilers. Preserve existing Make producer-order
and Ninja freshness fixes. Test missing-output recovery and no-op rebuilds;
relocation must not reintroduce duplicate L2 production or absent install files.
Generated modules, the `crexx` build grammar, Unicode consumer and installed
Level L examples are integration consumers, not evidence to delete.

### Cleanup policy and concrete disposition

No archive tree, dated replacement reports or committed cleanup manifest is
proposed. Apply the root standing rule to current and future work:

| Material | Disposition |
|---|---|
| Generic library source, maintained research engines, compiler source, regression cases, licenses and pinned upstream data | Keep with the correct owner; move only for the approved split. |
| Live oracle/corpus inputs hidden in `research/level-l/evidence` | Move into `cases/fixtures` or the owning tool/library test fixtures; update every reader and preserve required attribution. Includes config/consumer/recovery/lexrecovery/script oracles, migration fixtures and the old gennorm2 control. |
| Logs, stdout/stderr captures, timing files, generated run dumps, raw samples, run JSON/receipts and per-run hash bundles | Delete after extracting any still-useful result into the current plan/contract; do not track or archive them. Do not indiscriminately delete authored RXAS tests or binary-format fixtures. |
| Historical STEP review notes, dated evidence READMEs, completed handoffs and repeated status narration | Consolidate lasting decisions/contracts, active findings and current validation facts into maintained docs; delete the obsolete originals and update links. |
| STEP07A/SAY qualification output | The 109 untracked run files were deleted after consolidation into the compiler plan during the local checkout transfer. Keep the maintained source/tests and concise qualification boundary. |
| The 14 research source aliases and old build wrappers | Migrate active readers to canonical source; remove aliases/obsolete wrappers once their tests and documented workflows use the owner. |
| Unrelated root `build/`, mainframe driver locks or another task's running outputs | Outside this cleanup's ownership; preserve them. |

Keep one compiler plan/ledger, current architecture/API guides and runnable
examples. Condense both existing plans' historical narration while preserving
open acceptance IDs, accepted language decisions, deferred work and current
qualification limitations. Reused tests remain valid for unchanged inputs;
rename-only results and changed build/package proof are distinguished. Where a
raw result cannot support a current claim, narrow the claim rather than retain
junk or claim an unverified pass.

Update scripts that currently write results into tracked evidence paths to use
per-invocation temporary/ignored build directories and delete disposable output
when finished. Add a scoped tracked-file hygiene check and import/build-boundary
checks with explicit maintained-fixture exceptions. Do not blanket-ignore source
extensions or hide tracked junk with ignore rules.

### Numbered acceptance criteria

**RA-AC01–10 were verified for the first local reorganisation.** The naming
and library-only refinement below reopens affected path/build checks. These
supplement, and do not close or weaken, the compiler and wider research criteria.

1. **RA-AC01 — ownership:** paths, CMake declarations and install owners match
   the approved split; `lib/rxfnsl` contains only library implementation and required build data.
   Verify a source/target inventory and no imports from L into compiler research.
2. **RA-AC02 — tool service:** Level L generation has a callable B-only service
   returning owned output/diagnostics through L writer/sinks, with no main/stdout
   dependency. Check library and CLI success/error/budget/source/RXAS parity.
3. **RA-AC03 — compiler separation:** C-only builds and runs with typed/G grammar
   and frontend inputs absent; G-only builds and runs without C frontend/adapter.
   Inspect transitive imports and linked namespaces, not just executable names.
4. **RA-AC04 — typed sharing:** one shared B/G typed core and explicit profile
   boundaries; G regressions remain valid. B is visibly unimplemented/deferred,
   with no G-only dependencies admitted into its intended bootstrap closure.
5. **RA-AC05 — reusable infrastructure:** generic L tests and an independent
   non-C/G helper consumer prove the extracted utility boundaries; C/typed rules
   stay in their owners. No new native compiler logic or hidden fallback.
6. **RA-AC06 — bootstrap/build:** required L0/L1/L2 convergence, B-only bootstrap
   modes, Make single-producer and Ninja freshness/no-op/missing-output checks
   pass for changed inputs. Retained identical semantic results are reused.
7. **RA-AC07 — delivery:** default product, opt-in research and C/G-selected
   products have correct fresh/incremental install inventories and isolated
   service/CLI examples, including a prefix with spaces and stale old outputs.
   CI preparation still selects every affected producer and regression.
8. **RA-AC08 — semantics and RXAS:** STEP07A/SAY plus affected C/G/L regressions,
   source/error/atomic-output tests and representative IR/RXAS/helper inspections
   pass. No unexplained executable expansion, metadata drift or scope change.
9. **RA-AC09 — clean tree:** logs/raw runs/receipts/obsolete notes are removed;
   real fixtures have owners and consumers. Scripts/checks prevent new tracked
   junk. No external archive or history-preservation commit substitutes for cleanup.
10. **RA-AC10 — current documentation:** one maintained account of boundaries,
    builds, APIs and qualification; all affected links work. Preserve every open
    language/platform/INTERPRET criterion and distinguish local from hosted proof.

### Numbered implementation sequence and approval boundary

| Step | Work | Criteria / gate |
|---|---|---|
| **RA-STEP01 — approved** | Review actual code/build owners; select paths, service split, B/G sharing, package changes and cleanup policy. | AC01–10; Adrian approved implementation on 17 October 2026. The global no-junk instruction is already authorised. |
| **RA-STEP02 — complete locally** | Move canonical owners and update path consumers; extract the Level L service and tool support, preserving bootstrap dependencies and namespace meaning. | AC01/02/05/06; dependency: STEP01 approval. Check each coherent change before proceeding. |
| **RA-STEP03 — complete locally** | Split C from the typed core; replace common language-dispatch main/service; split family registration/target policy and service products. | AC03–05/08; preserve B deferral, G semantics and all current C fixes. |
| **RA-STEP04 — complete locally** | Separate product/research stages, outputs and install components; update wrapper, Unicode, inventory, CI and test preparation owners. | AC06/07; no silent loss of tests or reliance on stale outputs. |
| **RA-STEP05 — complete locally** | Promote genuine fixtures, consolidate durable docs, remove obsolete files/run output and migrate output-writing scripts. | AC09/10; check references before deletion; no archive requirement. |
| **RA-STEP06 — complete locally** | Run the smallest decisive affected build/tests/install/closure checks; reuse unchanged proof and inspect RXAS. Update current docs and delete temporary results. | AC01–10; new/expanded nested test aggregates need isolated Debug/sanitizer scheduling checks. Broader QA only for an identified gap; publication is a separate instruction. |

**Approved scope:** the `langtools` / SDK / research ownership split; an
independent C compiler alongside a mostly shared B/G typed core; separate callable
tool/compiler products and migration of the combined experimental API; and the
explicit opt-in research delivery policy. No new source-language syntax, B
implementation, runtime redesign, default replacement or INTERPRET work is
included. The standing no-junk rule is not conditional on this approval.


### Final naming and library-only refinement

**Approved direction, 17 October 2026:** use `rxlang` for the Level L generator
command, target and `langtools/rxlang.crexx`. Remove the generic `cli/` directory;
put its CMake generation adapter under `langtools/cmake/`. Name the callable tool
package `rxlanglib.rxbin`; preserve `llfront`/`llgenerate` APIs and the authored
`options levell` language. Update build, driver, install and documentation users
together. Do not retain an installed duplicate of the old `levell` command.

**Outcome:** `langtools/doc` and `langtools/examples` own all general Level L
learning and reference material. Independent library regressions move to
`langtools/tests/library`. `lib/rxfnsl` contains only library implementation and
required Unicode build inputs. The TinyExpr demonstration implementation moves
out of the SDK package into its example and is compiled separately for its test.
Compiler research retains its language-specific plans, examples and regression
fixtures. This is the last Level L layout refinement before a separately scoped
whole-repository review. Publication is now authorised as recorded above; the
wider review is separate from this completed implementation.

1. **RA-NAME-AC01 (verified locally):** `rxlang`, its source and target, and `rxlanglib` are
   canonical; no `cli/` source owner or stale installed `levell` products. Build,
   command/service parity, driver generation and fresh/incremental install prove
   the naming change; `options levell` and grammar suffixes remain compatible.
2. **RA-NAME-AC02 (verified locally):** no examples, standalone guides or tests remain under
   `lib/rxfnsl`; canonical paths, links, independent tests and runnable examples
   work under `langtools`. Inline API documentation remains with source.
3. **RA-NAME-AC03 (verified locally):** research C/typed/G boundaries remain intact; relevant
   regression and generated RXAS results agree. Reuse the previous refactor's
   unchanged semantic qualification. Keep main develop, unrelated work, paused
   CI monitoring and the no-junk rule intact.

| Step | Work and gate |
|---|---|
| RA-STEP07 — complete locally | Rename the command/package/adapter and move documentation, examples and tests; AC01/02. Uses Adrian's requested refinement. |
| RA-STEP08 — complete locally | Verify changed producers, wrappers, selected regressions, links and install names; update the existing guides/plan and delete temporary outputs; AC01–03. |

### Final naming and library-only verification

Verified on the same main `develop` base recorded below, with the final working
source/build/test inputs. These unchanged results qualify the authorised
publication; they do not establish hosted workflow success.

| Criteria | Local verification |
|---|---|
| RA-NAME-AC01 | `stage-product`, `stage-research-compilers`, `testlangtools` and the affected TinyExpr comparison/C/G PoC targets build. The renamed bootstrap passes its Make producer-order regression and a real Ninja no-op. The callable `rxlanglib` bytes equal the previous tool-library bytes; command usage and driver lookup now name `rxlang`. |
| RA-NAME-AC01/02 | Fresh and repeated installs into a prefix with spaces contain the new command/package and exclude stale `levell`, `levelllib`, TinyExpr and experimental compiler products. Old tool products deliberately present in the build bin do not leak into either install. The installed `check-authoring-build.py` panel passes 56 commands, including source forms, L0/L1/L2 parity, moved settings/calculator examples, combined/separate libraries, native packaging, incremental rebuilds and last-good-output preservation. |
| RA-NAME-AC02/03 | Forty-four focused CTests pass: relocated independent SDK tests, generated/tool service consumers, TinyExpr functional and binary-code-shape tests, its existing dispatch workload correctness checks, downstream Unicode, repository/product inventory, bootstrap ordering and the 54-command C/G PoC. This is correctness evidence, not a formal performance comparison. |
| RA-NAME-AC02/03 | `rxfnsl` no longer contains the TinyExpr teaching implementation; its separate `tinyexpr` namespace/module lives in `langtools/examples`. The C/G compiler service and Classic target package bytes are unchanged. Earlier independent frontend-isolation and broader semantic qualification remain valid. Current ownership, Python syntax, affected Markdown links and obsolete-path checks pass. Temporary logs, inventories and diagnostic workspaces were deleted after extracting these results. |

This completes the Level L ownership/naming refinement. Whole-repository
cleanup, full compiler replacement and hosted/platform qualification are separate
work; existing open criteria elsewhere in these plans are unchanged.

### Product-inventory script compatibility

The Linux smoke job for `c087788cb7f1291a8fa5027033e7a1c2caa97625`
exposed a gap in RA-AC07: the standalone inventory script did not establish
CMake's policy baseline, so CMake 3.x rejected its research-selection `IN_LIST`
condition. Local CMake 4.x had masked the omission. All other 204 smoke tests
passed, including the repaired rxbuild build-order test. The
[failing job](https://github.com/adesutherland/CREXX/actions/runs/39055903770/job/117468173602)
is the hosted reproduction; no compiler or runtime failure was observed.

The intended result is the same default and selected C/G inventory contract on
the repository's supported CMake versions. RA-AC07 remains the acceptance
criterion: the existing default and research inventory regressions must pass,
and a missing Classic adapter must still fail. RA-STEP08's compatibility repair
establishes the root project's CMake 3.24 policy baseline in the script; no
compiler, product-stage or packaging behaviour changes.

Local verification on the same base with this two-line script change reproduced
the original failure using CMake 3.24.3. After the fix, both 3.24.3 and local
4.3.2 pass `cmake -DBUILD=<build> -DRESEARCH=ON '-DLANGUAGES=C;G'
-P langtools/tests/check-products.cmake`, plus default, C-only, G-only and
lowercase selections. Both reject an isolated inventory missing `llctarget.rxbin`.
`ctest --test-dir cmake-build-debug -R '^(levell|llcompiler)_product_inventory$'
passes both existing regressions. `cmake --build cmake-build-debug --target
stage-product --parallel 8` also passes without warnings; unchanged functional
qualification is reused. These checks close the
local compatibility gap in RA-AC07/RA-STEP08; hosted verification remains the
normal push-triggered workflows. Temporary tools and diagnostics are deleted
after verification.

### Standalone and service-test preparation repair

**Approved continuation, 18 October 2026:** Adrian authorised updating local
develop, preserving active work, implementing both repairs, focused tests and
publication, then resuming lightweight CI monitoring on the new head. The
checkout fast-forwarded to `3eb8a76d160e56ec93a19ac2bf40839267454dea`
(rxbuild-only RXPP handling and report-writer sample), preserving all 115
pre-existing uncommitted files including this plan. Acceptance below remains
bounded to these repairs; the local checks below qualify their publication.

The [Linux Debug Optimizer Parity job](https://github.com/adesutherland/CREXX/actions/runs/39070474392/job/117515399533)
at `db2fabdd6cbd036874a27fcb6d14832a696e647e` passed 1004/1009 tests.
The bootstrap-order fixture failed at configuration because the shared tool
declarations called product installation helpers in standalone mode. Four
opt/noopt service cases failed because their preparation targets did not build
the generated `levell_example.rxas` CLI oracle. These are separate preparation
gaps following the layout change, with no optimizer mismatch or sanitizer
diagnostic in the failing output.

Both mechanisms reproduced locally before any implementation edit:
`ctest --test-dir cmake-build-debug -R '^rxlang_bootstrap_build_order$'
--output-on-failure` fails with the same unknown install helper. With only the
generated CLI oracle temporarily removed, building `levell_services_opt` and
`levell_services_noopt` leaves it absent; `levell_services_opt_rxtvm` then fails
with the same READBINARY error. The original generated file and its timestamp
were restored. Temporary diagnostic files were deleted.

**Outcome:** keep standalone bootstrap independent of native CLI installation,
and make each service preparation target supply its actual runtime inputs in
a fresh or partially prepared tree. Preserve normal product installation,
single-producer bootstrap ordering, freshness and the independent service/CLI
comparison. No compiler, language or runtime semantics change.

1. **RA-PREP-AC01 — verified locally (RA-AC06/07):** the real standalone project configures
   without product install helpers; the existing parallel-Make regression passes
   clean, missing-L2, changed-input and no-op checks. Normal product installation
   still registers `rxlang` and `rxlanglib`.
2. **RA-PREP-AC02 — verified locally (RA-AC05/07):** preparing either service target
   recreates a missing generated CLI oracle, keeps no-op builds unchanged and
   supplies all four existing opt/noopt and VM service regressions.
3. **RA-PREP-AC03 — local checks verified; publication prepared (RA-AC09/10):** focused local checks and the required
   product build pass; current guides explain these boundaries. Preserve active
   work, delete temporary diagnostics, publish only the repairs, and follow the
   new normal workflows without claiming hosted success from local proof.

| Step | Work and gate |
|---|---|
| RA-STEP09 — complete locally | Reproduce the standalone configuration failure; guard product installation with the existing standalone boundary; verify the actual standalone configuration, Make fixture and normal product inventory. Serves PREP-AC01/03. |
| RA-STEP10 — complete locally | Reproduce the missing-oracle failure; give the oracle one shared producer with its file freshness dependencies and order both service targets after it; check missing-output recovery and no-op behavior, then run the four service cases and product build. Serves PREP-AC02/03. |

The 18 October continuation covers these narrow repairs in the existing main
develop checkout. Keep the distinct causes reviewable and publish
them together after focused qualification; reuse unchanged semantic evidence.

**Local qualification, 18 October 2026:** base
`3eb8a76d160e56ec93a19ac2bf40839267454dea` plus the two CMake repairs above.
The actual standalone project configures with Ninja and Unix Makefiles.
`python3 -B langtools/tests/check-bootstrap-build.py --source <checkout>
--work unused --cmake <cmake> --make /usr/bin/make` passes the existing
real-declaration fixture's clean, missing-L2, changed-input and no-op checks.
This is the same command registered by `rxlang_bootstrap_build_order`, reused
without a redundant rerun. A component install into a disposable prefix with
spaces installs `rxlang` and `rxlanglib.rxbin` byte-identical to the products.

`cmake --build cmake-build-debug --target stage-product levell_services_opt
levell_services_noopt testlevell_generated --parallel 8` passes without warnings.
After refreshing the incoming product prerequisites, removing only the generated
CLI oracle and preparing each service target separately restores its exact
bytes while leaving both service images' bytes and timestamps unchanged. A
subsequent preparation is a no-op. The original specification/tool file
dependencies remain intact; the generated-module consumer also waits for the
shared producer.

`ctest --test-dir cmake-build-debug --output-on-failure --parallel 4 -R
'^(levell_services_.*|levell_generated_.*|levell_product_inventory|rxbuild_demo_build_order|rxbuild_directory_missing|rxbuild_directory_blocked)$'`
passes all ten tests: four service mode/VM cases, two generated-module cases,
product inventory and three rxbuild regressions after the incoming update.
The existing bootstrap fixture and service cases reproduce the original
failures; normal fresh hosted preparation will exercise the repaired
dependencies. No duplicate test or expanded aggregate was added.

These results qualify the combined local preparation repair, not a fresh
sanitizer, full optimizer-parity, cross-platform or release pass. Compiler,
runtime and test-oracle semantics are unchanged. The two repairs are prepared
as separate commits for one develop publication; normal push-triggered Build
CREXX and CodeQL runs will be monitored at that exact head. All 114 unrelated
active files remain uncommitted and byte-identical; temporary diagnostics are
deleted after extracting this summary. Hosted results belong in the workflow
links and conversation, without a status-only follow-up commit.

### Reorganisation verification and remaining scope

Qualified in the main `develop` checkout against base
`06a548321662e6dc4e91899363eb12831b4d952c`, with the source/build/test changes in
this working tree. The following results cover this refactor, not full compiler
replacement, release or hosted platform qualification. Generated diagnostic
workspaces and logs were deleted after extracting these conclusions.

| Criteria | Decisive local checks and observations |
|---|---|
| RA-AC01/04/05/09 | `python3 langtools/tests/check-layout.py`: ownership/import/Level B and no-disposable-output rules pass. C has no typed/G imports; typed/G has no C imports. SDK/tool source has no research dependency. Independent service tests cover literal escaping, instruction origins, RXBIN metadata validation and atomic file publication. B remains explicitly deferred. |
| RA-AC02/05/08 | `stage-product` and selected research products build successfully. Eighty focused SDK/tool CTests passed, including all four host-mode/VM independent service tests. CLI and service bytes agree; syntax, malformed metadata, budgets and last-good-output failures are covered. Source-emission oracle harness passes 160 cases in four mode/VM cells. |
| RA-AC03/07 | `check-compiler-isolation.py --build cmake-build-debug` builds and executes C with typed/G sources absent, then G with C sources/adapter absent. Both print 5. Separate installed command and B service consumers pass. |
| RA-AC06 | Optimized L0/L1/L2 converges in the product graph; a clean temporary standalone no-opt bootstrap also converges. Ninja no-op and missing-L2 recovery preserve output bytes. The real-declaration Make fixture passes clean/missing-image/changed-input/no-op cases in Debug and through the maintained ASan runner, about 16 seconds isolated; it is a scheduling proof, not native sanitizer qualification. |
| RA-AC07 | Default research-OFF product graph builds without a research stage. Fresh and repeated installs to paths with spaces exclude deliberately stale old/new research names in global bin. Explicit C-only and G-only install components contain exactly the selected products even when opposite products remain in the build. Both-language installed pipelines/services pass; normal CI explicitly opts in and registers producers. |
| RA-AC08 | Optimized C/G backend, provenance, optimisation, PoC, SAY and STEP07A suites passed. No-opt frontend groups, SAY and STEP07A passed. Final pruned-owner build passed eight IR demo/contract cells, both product inventories and the 99-command STEP07A suite; direct PoC execution passes all 54 commands with a zero process exit. Counts overlap earlier checks and are not summed as distinct tests. |
| RA-AC08 | Four final walkthrough RXAS instruction/source streams exactly match their prior examples: C frames 6/25 before and 6/16 after, with 17/11 calls; G frames 2/14 and 2/5, one wrapper call. Classic parameter metadata was refreshed to the existing STEP07A names. Calls, cleanup windows, condition relay and logical filenames remain unchanged. This is no expansion and no new performance claim; full Classic TRACE/allocation quality remains open in the compiler plan. |
| RA-AC09/10 | Over 16,000 disposable files deleted; real oracle inputs moved to owned fixtures. Research/test runners use temporary workspaces and clean them. The narrow toolkit runner passes both VMs; its two stale diagnostic expectations now match the current resource/name contract. The 84-case configuration and 121-case LR-repair independent oracles still match their maintained fixtures. Owned Python/shell syntax and documentation links pass. |

Architecture, bootstrap, service, user and example guides now describe the
three owners and explicit research install. Historical acceptance IDs below
remain for continuity; obsolete run-path references do not define current
storage or publication policy. Existing research experiments beyond the changed
runners were not broadly requalified. Native compiler behavior, complete C/G/B,
INTERPRET, cross-target bootstrap and hosted packaging remain their own gates.
The first substantive publication includes this qualification summary and the
current guides. Normal automatic workflows are expected to pass; their actual
results remain separate hosted evidence. CI monitoring remains stopped.



### Authoring and build refinements — approved 13 October 2026

**Vision:** make a Level L language module a normal part of a cREXX project.
Programmers can use `.crexx` with `options levell`, or a short dedicated suffix,
and let `crexx` generate, assemble and link the module. Teach ordinary lists
using the existing repetition notation and useful named fields. Preserve the
language accepted by each example, original source diagnostics, the Level B
bootstrap and existing consumers. This increment supplements LANG-AC02/04/11–13;
the wider research and language criteria remain unchanged.

**Status:** Adrian approved the source forms, wrapper integration, list cleanup,
regression tests and guide update. A combined program build includes its grammar
modules directly; creating a separate library is optional. **UX-AC01–05 are
locally verified** by [current plan](PLAN.md).
The existing `develop` checkout was fast-forwarded to `54b610b95` before editing;
incoming Fortran/Forth work was preserved. Before publication, develop packaging
fixes through `1031ef530` were integrated and their combined installed workflow
was checked. Adrian authorized commit/push to `develop` and light monitoring
on 13 October. The enclosing commit is the publication payload; normal automatic
CI is expected to pass, with its actual results reported in the conversation.
No new release tag or overall programme completion is claimed.

**Approved decisions:**

- Use `.crexx` with `options levell` as the ordinary source form. Offer `.rxl`
  as the short Level L suffix; retain `.levell` for existing sources. For
  `.rxl`, Level L is the default when OPTIONS is absent. An explicit different
  level is an error. Namespace declarations remain part of the source.
- Route through the installed `crexx` driver and existing `--library` and
  `--program` commands. Generate packed RXAS, assemble with `rxas`, and link
  generated modules and their Level L runtime dependencies. A standalone
  grammar builds a reusable module; an application supplies execution.
- The compiler source-import scan identifies Level L headers and leaves their
  namespaces to generated modules; it does not feed grammar declarations into
  the typed source parser. Its dependency snapshot still tracks changes in
  source kind. This is source selection, not a parser/scanner replacement.
- Build generated grammar providers before compiling their consumers. Extend
  existing caching, dependency and atomic-publication rules; generator/runtime
  changes invalidate affected products. Required bootstrap construction still
  works without Level G or the high-level project driver.
- Reuse existing `*`, `+`, `?`, groups and `as` captures. Convert ordinary
  hand-authored list/tail scaffolding where it simplifies the source. Preserve
  deliberate recursive backend demonstrations. Keep Pratt operator declarations
  as the existing concise expression form.

**List-construction finding:** the first simplified RexxScript run exposed
quadratic intermediate frontier copies in the named-tree view: the existing
3,000-statement regression exhausted `tree.entries`. The view now counts
transparent frontiers and materializes visible links for named parents only.
UX-AC04 includes a direct 3,000-item test at an exact 6,001-entry budget, its
one-entry-too-small refusal, order/span checks and transparent-root selection.
This repairs the observed regression without increasing user budgets.

**Acceptance criteria — locally verified on macOS ARM64 Debug:**

1. **UX-AC01 — Source selection.** `.crexx` with `options levell`, `.rxl` with
   its agreed default, and existing `.levell` sources generate equivalent
   modules. Tests cover leading comments, case, conflicting headers, paths
   containing spaces and unchanged B/G/C selection. Locations refer to original
   source even when the level is supplied by a suffix.
2. **UX-AC02 — Integrated builds.** Installed `crexx` builds a standalone
   grammar library and a mixed grammar/application project, links the required
   runtime, and runs the example through bytecode and native packaging. Verify
   dependency ordering rather than relying on input-file order or old outputs.
3. **UX-AC03 — Build lifecycle.** Focused tests prove clean/no-op builds,
   grammar/tool/runtime invalidation, output collision checks and preservation
   of the last good artifact on errors. Grammar failures stop dependent work
   and retain the authored filename, rule and location.
4. **UX-AC04 — List clarity and equivalence.** Simplified settings and
   RexxScript lists preserve acceptance, item order, captures and source spans,
   with readers adapted to the intended flatter named tree. Test zero/one/many
   items, blank lines, repeated separators, trailing separators, missing
   separators, malformed items, diagnostics and recovery. Exercise LL/LR/PEG/
   Earley where the selected grammar is admitted; preserve their distinct
   semantics and report capability failures against authored rules.
5. **UX-AC05 — Bootstrap and teaching continuity.** L0/L1/L2 regeneration still
   agrees with no Level G bootstrap dependency. Update the guide, diagrams and
   progressive settings/calculator instructions to use tested wrapper commands
   and repetition-first teaching. Existing recursive backend lessons remain
   clearly identified. Retain focused toolchain/both-VM regression evidence.

**Implementation steps — approved:**

1. **UX-STEP01 (AC01–05):** agree suffix/header behavior and build/list scope;
   reconcile the current guide and consumers against those decisions. **Agreed.**
2. **UX-STEP02 (AC01–03/05):** implement source selection and driver generation,
   dependency ordering, linking and lifecycle handling. Depends on STEP01;
   retain the independent Level B bootstrap path. **Verified.**
3. **UX-STEP03 (AC04/05):** simplify ordinary lists and adapt tree consumers;
   compare with retained original grammars and inspect diagnostics/recovery.
   Depends on STEP01; new semantics require a separate decision. **Verified:**
   384 old/new observations plus capture, recovery and interpreter regressions.
4. **UX-STEP04 (AC01–05):** run focused installed/build/backend regressions,
   update the user guide and record evidence and remaining criteria. Depends
   on STEP02/03; follow the repository's ordinary publication policy. **Verified:**
   30 existing CTests, 56 installed authoring commands, 22 installed package
   commands, 13 packaging unit tests and 80 valid guide links. Wider research
   and host-platform qualification criteria below remain unchanged.

### AST infrastructure — implementation approved 13 October 2026

#### Progressive AST services — approved 15 October 2026

**Vision:** build on the checked structured-flow and owned-AST services so an
application can supply justified condition facts, lower richer syntax into a
validated simpler tree, and retain reliable source origins through generation
and recovery. Ordinary grammars keep their current surface. These are language
engineering services implemented in Level B, with examples showing application
semantics separately from library guarantees. Adrian requested sequential
implementation, commit and publication of steps 1–3 after each step passes.

**Acceptance criteria — open until individually verified:**

1. **AST7-AC01 — Condition facts.** Optional true/false facts are bound to one
   immutable tree, carry a reason, reject contradictions/foreign or stale owners
   and obey a counted budget. Absent facts mean unknown. Flow consumes a sealed
   fact set, excludes only impossible outcome edges, and preserves expression
   evaluation, writes, short-circuiting and loop behavior. Independent tests
   cover constant and unknown cases, witnesses and lifecycle failures. Document
   the conditional soundness argument; supplying a reason is not a theorem proof.
2. **AST7-AC02 — Checked lowering.** A children-first pass can construct a
   different declared target schema, combine completed children and publish only
   a complete validated result. Missing translations, invalid fields/counts,
   sharing, stale/foreign handles and resource exhaustion fail atomically.
   A compiler-shaped example lowers a higher-level control construct and tests
   its observable flow against an independent original-language control.
3. **AST7-AC03 — Origins and recovery.** Exact, composite, derived, synthetic
   and recovered origins have validated contracts and useful diagnostic anchors.
   Recovery exposes an explicitly marked owned AST with original-byte damage
   records, while ordinary strict parsing and script execution remain guarded.
   Copies and lowering preserve recovery information; generated nodes cannot
   silently turn recovered input into a clean tree. Verify Unicode/CRLF/empty
   spans, original diagnostics, lifetime checks and applicable parser backends.
4. **AST7-AC04 — Progressive delivery.** Each step includes runnable examples,
   manual/reference updates, mathematical scope/bounds and permanent focused
   regressions. Product build, relevant correctness, both VMs/optimization modes,
   Level B bootstrap and installed use pass for affected inputs. Retain exact
   qualification identities; commit only this work directly on synchronized
   develop and verify its pushed SHA. Automatic hosted CI remains a separate
   result; inspect it lightly and fix actionable failures without broad dispatch.

**Implementation steps:**

1. **AST7-STEP01 (AC01/04):** implement optional condition facts and their graph
   integration; qualify and publish. Keep default graph behavior unchanged.
2. **AST7-STEP02 (AC02/04; after STEP01):** implement target-schema lowering and
   a worked control-flow translation with explicit structural and semantic
   checks; qualify and publish separately.
3. **AST7-STEP03 (AC03/04; after STEP02):** add synthetic/recovered source-origin
   contracts and integrate the existing bounded recovery service; qualify and
   publish separately. Keep broader emitter and production migration work open.

**Delivery gate:** “green” before each publication means the successful product
build and focused functional/installed checks required by AGENTS.md. Retain
unchanged valid evidence rather than rerunning broad suites. Normal Build CREXX
and CodeQL run automatically after each push; actual hosted results are reported
separately. No release tag, overnight dispatch or arbitrary proof solver is
included. The unrelated build directory and mainframe lock files are preserved.

**Status:** AST7-STEP01 and AST7-AC01 are complete and published. Evidence:
[current plan](PLAN.md): product
build, 36 unchanged controls, 16 new matrix tests, both Level B bootstrap modes,
34 no-opt delivery commands and 12 fresh installed checks. Step 1 was published as `26339d2f01fb379fdd4f51bb4be9255d4d562494`.
AST7-STEP02 and AST7-AC02 are locally complete, with evidence in
[current plan](PLAN.md): product build,
20 existing controls, 16 lowering tests, bootstrap parity and 54 delivery
commands. Step 2 was published as `8448fc03d6131606c14cf71c60855f163f30a759`.
AST7-STEP03 and AST7-AC03 are locally complete and delivered by this third
separate publication. Evidence:
[current plan](PLAN.md): successful product
build, 112 distinct focused CTests, 160 recovery scenarios, both bootstrap modes,
34 no-opt delivery commands and 12 installed checks. AST7-AC04 is complete for
this three-increment local qualification and publication scope; hosted results
remain separately observable on GitHub. No wider production migration is claimed. All three requested increments have
satisfied their local delivery gates. Hosted workflow outcomes are reported in
the conversation, without a status-only commit.

#### Structured loops and exits — approved 14 October 2026

**Vision:** describe ordinary language control flow with a small set of mappings
from named AST fields, and explain which reads may occur before assignment.
Loops, early exits and short-circuit expressions must compose correctly, with
source-linked paths and a checked reachability query. The same Level B services
serve every admitted parser backend. This increment preserves the existing
grammar → AST → flow separation and the wider AST programme below.

**Approved surface:** `loop while condition do body`, `loop do body until
condition`, `loop do body`, `break`, `continue`, `return value` (a required or
optional child), bare `return`, `shortcircuit left and right`, and `shortcircuit
left or right`. Break/continue target the nearest enclosing loop; return exits
the analysed root. `result.before(node).reachable()` reports reachability in
the constructed graph. Conditions remain source-linked ASTs. Consuming constant
or proof facts to remove branches is a later increment. Adrian approved local
implementation on 14 October and commit/push to develop on 15 October 2026.

**Numbered acceptance criteria — locally verified:**

1. **AST6-AC01 — Surface and checks.** All three readers accept the approved
   forms. Located diagnostics reject unknown fields, wrong child counts,
   incomplete mappings and invalid qualifiers. Existing grammars still work.
2. **AST6-AC02 — Composed control.** Pre-test, post-test and unconditional loops,
   nested break/continue, optional return values and short-circuit outcomes
   produce the intended graph. Abrupt exits cannot reach following statements.
   Invalid jump placement fails with the source kind and location.
3. **AST6-AC03 — Analysis.** Definite assignment and reachability agree with
   independent path controls, including zero iterations, returning branches,
   skipped operands and cycles. Diagnostics identify the original read and an
   actual path without assignment. Stale views and exhausted budgets expose no
   partial answers. Record termination and storage/work bounds for cyclic graphs.
4. **AST6-AC04 — Delivery and teaching.** Add a worked example and prebuilt
   runtime regressions across LL(1), LR(1), PEG and Earley, both VMs and consumer
   optimization modes. Check source/RXAS providers, Level B bootstrap in both
   modes and installed wrapper use. Update the manual's grammar/AST/flow
   walkthrough, flow reference and API, and retain qualification inputs/results.

**Numbered implementation steps:**

1. **AST6-STEP01 (AC01):** extend the shared reader, self grammar and schema
   checks; preserve generated source and packed-table service contracts.
2. **AST6-STEP02 (AC02/03; after STEP01):** build loop targets and distinct
   short-circuit outcomes iteratively; add checked reachability and diagnostics.
3. **AST6-STEP03 (AC01–03; after STEP02):** add independent path controls,
   placement/lifecycle/budget regressions and the cyclic analysis argument.
4. **AST6-STEP04 (AC04; after STEP03):** qualify the toolchain and installed
   example, finish the guide and record evidence. Leave wider AST obligations
   and constant-aware analysis explicitly open.

**Status: AST6-AC01–04 and AST6-STEP01–04 locally verified, 14 October 2026.**
Adrian authorized direct commit/push to `develop` on 15 October 2026. The
publication base is `9c790a5cb4aa25b04166d30d1c9479d24a9722b0`, which already
contains the separate mainframe/VM input-failure work. Level L source/test hashes
remain unchanged. A current product rebuild, 18 focused CTests and 12 refreshed
installed-wrapper checks pass; the unchanged wider qualification is retained.
The enclosing commit is the publication payload. Normal automatic CI is expected
to pass; its actual result is reported in the conversation without a status-only
commit. Evidence and exact input identities are retained in
[current plan](PLAN.md).

| Criterion | Verification |
|---|---|
| AST6-AC01 | 112 completed surface commands include valid source/RXAS stage parity and 39 exactly located semantic refusals; 12 small malformed probes pass at L0/L1/L2. A missing seed field now reports EOF instead of line 0. |
| AST6-AC02 | All loop forms, nearest nested jumps, returns and distinct short-circuit outcomes pass on LL(1), LR(1), PEG and Earley, in both consumer modes and VMs. |
| AST6-AC03 | Independent 16-case cyclic path controls, zero-iteration/return/short-circuit cases, exact witness text, lifecycle and exhaustive insufficient graph allowances pass. AST-CONTRACT.md records construction invariants, finite convergence and resource bounds. |
| AST6-AC04 | All 64 selected CTests pass; focused reader repair tests pass afterward with unchanged flow bytecode/providers. Both Level B modes have identical L1/L2 syntax output; 34 unoptimized-service checks, nine focused bootstrap checks and 12 final installed checks pass. Recovery passes 24 candidate and 12 preservation cases. The manual, example, reference and links are checked. |

The self grammar factors common name alternatives and explicitly selects the
existing LL(1) backend, preserving its terminal syntax without increasing user
resource defaults. Full malformed-example fixtures were reduced after the
completed surface panel; broad recovery already had independent coverage.
The final reader-only repair reuses unchanged flow/runtime evidence and has its
own focused probes and installed readback. Wider AST acceptance criteria remain
open as recorded below; constant/proof-aware pruning was not part of this step.

#### Sparse properties and simple flow analysis — approved 14 October 2026

**Vision:** attach typed, sparse application information to an immutable AST,
with declared scalar defaults and familiar named accessors; describe execution
order separately from tree shape and obtain a checked definite-assignment
result. A small rules-language example teaches the complete path from grammar
to diagnostics. These are general language-engineering services built in Level
B, independent of the selected parser and of RexxScript execution policy.

**Approved surface:** `notes: properties` followed by `message: property .string
default ''` and `priority: property .int default 0`; generated `getmessage()`,
`hasmessage()` and `setmessage()` (Adrian's naming correction). Groups borrow
their tree; `finish()` publishes a checked frozen result. `execution: flow`
contains kind-labelled `sequence`, `assign`, `branch ... then ... else ...`,
`evaluate`, `read` and `value` mappings. Indentation remains formatting: a
properties group owns successive property declarations; a flow group owns
successive operation declarations. A different declaration ends the group.
Required AST children keep their structural checks. Collection defaults and
new language control constructs are outside this increment.

**Numbered acceptance criteria:**

1. **AST5-AC01 — Authoring.** Located checks reject duplicate/unknown properties,
   invalid literal defaults, unknown kinds/fields, wrong cardinalities and
   incomplete flow mappings. L0/L1/L2 accept the same valid surface and reject
   invalid semantic fixtures. Existing basic grammars remain unchanged.
2. **AST5-AC02 — Sparse properties.** Unset getters return defaults without
   inserting entries; `has...()` distinguishes explicit default values. Typed
   writes, freeze, exact/exhausted budgets, stale/foreign/copied/rewritten trees
   and expired group views are checked. Failed groups publish no usable data.
3. **AST5-AC03 — Flow.** Validated mappings build a finite source-linked graph
   with evaluation before assignment, ordered sequence, two branch successors
   and correct joins including an absent ELSE. Unsupported node effects cannot
   silently become pure. Resource and ownership failures are atomic.
4. **AST5-AC04 — Definite assignment.** Entry variables, reads, assignments and
   joins agree with independent path controls. Results distinguish an analysis
   failure from a completed result containing unsafe reads. Explain diagnostics
   with an actual unassigned path. Record the finite monotone domain, transfer
   equations, termination and resource bounds; do not claim arbitrary program
   equivalence or richer loop-language support.
5. **AST5-AC05 — Delivery.** Source and packed RXAS generation, applicable parser
   backends, Level B bootstrap, both VMs, optimized/unoptimized consumers and
   installed wrapper use are checked. Add a worked example, runtime regressions,
   precise negative diagnostics, user-guide/API documentation and proof notes.
   Preserve all wider AST-AC01–06 obligations.

**Numbered implementation steps:**

1. **AST5-STEP01 (AC01/02):** add property contracts, checked sparse storage and
   typed generated accessors, extending the shared reader and bootstrap syntax.
2. **AST5-STEP02 (AC01/03):** check flow mappings against inferred AST schemas;
   construct the standard graph iteratively with source-node references.
3. **AST5-STEP03 (AC04):** implement bounded definite assignment and explanatory
   diagnostics; validate independently enumerated branch paths and failure states.
4. **AST5-STEP04 (AC01–05):** qualify both generation formats/backends/toolchain,
   add the complete example, and update the manual and structural contract.

**Status: AST5-AC01–05 and AST5-STEP01–04 locally verified, 14 October 2026.**
The [current plan](PLAN.md) retains
exact input identities, commands, results and the two repaired boundary
findings. Adrian authorized commit/push to `develop` and light CI monitoring on
14 October. Before publication the checkout was synchronized through
`0dc462b6d70f2bdca69827c811dedb5e2ae18ea8`. All 13 qualified implementation/test
inputs and eight recorded tool/library images still matched their hashes.
The incoming change only adjusts wrapper directory reuse; the product rebuild
and 12 fresh installed-wrapper checks passed after integration. The enclosing
commit is the publication payload. Final whitespace cleanup affected one Python
test line and two retained log copies; Python AST equality and both hashes are
recorded, so the existing test evidence remains valid. Normal automatic CI is expected to pass;
actual results are reported in the conversation without a status-only commit.

| Criterion | Verification |
|---|---|
| AST5-AC01 | 16 invalid grammar fixtures agree at L0/L1/L2 with exact line, byte and message; two generated-API compile refusals; existing basic-parser tests pass. |
| AST5-AC02 | Defaults, explicit default values, typed updates, freeze, entry limits and all owner/view lifecycle cases pass in source and packed providers. A rejected stale view leaves its replacement group usable. |
| AST5-AC03 | All six execution roles, optional ELSE, joins and ordered Pratt evaluation pass. Every smaller graph allowance is refused cleanly; the exact allowance succeeds. |
| AST5-AC04 | Independent four-path controls over 16 write configurations agree; exact/exhausted state/work budgets, diagnostic locations and path witnesses pass. The finite-domain proof and resource bounds are recorded in AST-CONTRACT.md. |
| AST5-AC05 | Product and standalone unoptimized Level B builds pass; L1/L2 output agrees across both modes. All 16 new CTests pass, with 34 unchanged relevant passes retained. The surface panel passes 200 commands and the delivery panel 46; installed combined/separate builds and the worked manual example pass. |

The earlier acyclic STEP04 delivery remains verified. This increment adds
sparse annotations and definite assignment for the reviewed structured flow
profile. Wider cyclic-language mappings, grouping/lowering, recovered origins
and production compiler migration remain open below.

#### AST-STEP04 first increment — locally qualified

**Vision:** exercise the common AST in a real script engine, remove positional
tree plumbing from expressions, assignment, SAY and IF, and make reusable
analysis and checked constant folding available without adding grammar syntax.
The enclosing sequence/block/loop wiring uses named fields where required
to carry this slice; loop algorithms, language rules, BIF effects and Level G
numeric behavior remain application policy. This is the approved acyclic
increment of STEP04, not closure of the later cyclic-analysis or recovered-origin
work. Adrian authorized publication of this increment on 14 October 2026.

**Acceptance criteria for this increment:**

1. **AST4-AC01 — Migration.** RexxScript executes the named AST for expressions,
   assignment, SAY and IF. Existing language acceptance, values, branch and
   short-circuit behavior, BIF order/effects and source diagnostics agree with
   the retained pre-migration implementation. Existing loops, return and
   control validation continue to work around the migrated slice.
2. **AST4-AC02 — Analysis.** A Level B children-first service records one
   application-defined result per node, reads only completed dependencies and
   publishes only a complete result. Wrong-tree, expired-owner, missing/future
   dependency, incomplete pass and resource failures have explicit tests.
   Rewritten/copied trees cannot consume another tree's facts accidentally.
3. **AST4-AC03 — Meaning-preserving fold.** RexxScript analysis reports known
   constants and conservative variable reads/effects. Folding calls the same
   scalar operations as execution, preserves types and exact representable
   values, never invokes BIFs, and leaves failing/unsupported computations for
   execution. Folded and ordinary execution agree on results, output, state,
   effects and reachable-error locations; input ASTs remain usable.
4. **AST4-AC04 — Structure and limits.** Checked one-pass reconstruction is
   finite, source ranges survive replacements, and failed analysis/rewrite
   publishes no partial result. Cover deep/wide/empty trees, fresh runs,
   cancellation and bounded interpreter work; record the structural proof and
   separate it from application-equivalence evidence.
5. **AST4-AC05 — Integration and teaching.** Level B bootstrap, normal product,
   both VMs, opt/noopt focused tests and installed RexxScript/Level L examples
   pass. The guide explains the analysis-to-rewrite pipeline and a complete
   worked example. Preserve wider AST-AC01–06 obligations above/below.

**Implementation steps:**

1. **AST4-STEP01 (AC01/03):** retain the current interpreter as a differential
   control; map existing grammar rules with captures and `then tree`, preserving
   recognition and keeping the ordinary source surface unchanged. **Complete.**
2. **AST4-STEP02 (AC02/04):** implement tree-owned result tables and checked
   children-first analysis, with lifecycle, dependency and budget regressions.
   **Complete.**
3. **AST4-STEP03 (AC01/03/04):** migrate named-field execution and add RexxScript
   facts and checked folding, sharing numeric/literal operations. Depends on
   STEP01/02; unsupported or non-round-trippable constants remain unchanged.
   **Complete.**
4. **AST4-STEP04 (AC01–05):** compare against retained controls and unfolded
   execution, qualify affected builds/tests and installed use, and update the
   guide, contract and evidence. New tests should be prebuilt direct VM tests,
   not nested aggregate builds. Keep subsequent analysis profiles visibly open.
   **Complete.**

**Increment status:** AST4-AC01–05 and AST4-STEP01–04 are verified locally on
macOS ARM64 Debug, based on develop `50926edb8`. Adrian authorized commit/push
to `develop` and light CI monitoring on 14 October. Before publication the
checkout was synchronized through `60f58c0a9`; all 20 recorded inputs still
matched their qualification hashes. Incoming changes concern separate test
fixtures, their evidence and Apple packaging retries. The enclosing commit is
the publication payload; only publication documentation changed after those
checks. [current plan](PLAN.md)
retain the results and publication reconciliation. Normal automatic CI is
expected to pass; actual results are reported in the conversation, without a
status-only commit. The unchanged Level B bootstrap contract is preserved.

| Criterion | Verification |
|---|---|
| AST4-AC01 | Existing 60-case RexxScript suite on both VMs; 28 exact command-line comparisons with the retained pre-migration interpreter; named-field execution review. |
| AST4-AC02 | Dedicated analysis tests cover dependencies, exact/exhausted budgets, early/double finish, missing/duplicate facts, expired/copied/rewritten/reassigned owners and current-cursor access. |
| AST4-AC03 | 40 new cases compare folded and ordinary output, state, scalar types/values, float bits, short-circuit effects and exact error strings; exact-float and integer replacement shapes checked. |
| AST4-AC04 | Existing empty, 1,000-item and 2,000-wrapper AST controls; atomic rewrite/analysis failures; source anchors; fresh executions, cancellation and fuel controls; updated structural contract. |
| AST4-AC05 | Warning-free product; four equal opt/noopt L1/L2 self-descriptions; both VMs and compiler modes; 267 AST surface observations plus 51 unoptimised capture/diagnostic observations; eight installed commands; updated guide and example. |

**Previous delivery:** the reviewed first AST surface is implemented and locally qualified
on macOS ARM64, based on synchronized develop `ec481f8e6`. It includes attached
mappings, inferred field contracts, owned trees, reference handles, iterative
walks, checked one-pass reconstruction, calculator/folding and settings examples.
[current plan](PLAN.md) retain the results and
identities. Adrian authorized commit/push to `develop` and light CI monitoring
on 13 October. The enclosing commit is the publication payload; its qualified
code and test inputs are unchanged. Normal automatic CI is expected to pass,
with actual results reported in the conversation rather than a status-only
commit. This advances LANG-AC04/09/10/12/13 and research AC-26 without closing
their wider obligations.

**Cumulative criterion status:**

| Criterion | Verified here | Still open in the wider criterion |
|---|---|---|
| AST-AC01 | Simple attached mappings; named text/child fields; required, optional and list counts; located refusals; settings/calculator | None for the reviewed inferred-schema surface. |
| AST-AC02 | Independent owned results; borrowed handles; request release; exact/composite/derived/synthetic/recovered source ranges; owned damage records; Unicode; stale/foreign-handle rejection; sparse typed properties/defaults; counted budgets | Broader migration measurements. |
| AST-AC03 | Iterative walks; field/list access; application folds; checked reconstruction; empty/deep/wide/malformed/shared inputs; atomic failure; opt/noopt and both VMs; explicit traversal bounds | General search/fold convenience services and the wider builder/grouping profiles. |
| AST-AC04 | Schema-checked one-pass rewrites; acyclic application facts; owner/snapshot invalidation; checked RexxScript constant folding with exact numeric round trips and diagnostic anchors; checked structured flow, loops/exits/short-circuit outcomes, finite definite assignment and reachability with path witnesses | Broader application-specific analysis and production migration. Condition facts and the checked Unless-to-If lowering are verified by AST7-STEP01/02. |
| AST-AC05 | Equivalent admitted AST fixtures across general backends; Pratt/LR/Earley expressions; composite settings; explicit ambiguous/failed refusal; named RexxScript slice, effects/control/error comparisons and retained pre-migration control; recovered-origin integration including LR/Pratt regions | Broader language and production migration corpus. |
| AST-AC06 | Level B bootstrap in both optimization modes; identical L1/L2 output; installed combined/library builds; progressive manual | Production migration work; the compiler-shaped Unless lowering example is verified by AST7-STEP02. |

AST-STEP02/03 and the first-delivery portion of AST-STEP05 are complete within
the scope above. The acyclic RexxScript increment of AST-STEP04 is implemented;
its local integration gates are verified above. Restricted cyclic definite
assignment and reachability are verified by AST6-AC03. Optional condition facts,
checked target-schema lowering and recovered origins are verified by AST7-AC01–04.
General convenience algorithms and production migration measurements stay open.
The complete AST infrastructure programme is not being marked complete.

**Approved first surface:**

```rexx
program: rule = (statement as item)*
  then tree (items = item*)
sum: infix arithmetic plus precedence 10 left
  then tree Add(left = left, right = right)
```

- The enclosing label supplies the default AST kind; an explicit kind renames
  it. Indentation is formatting, not scope. Named captures supply ordinary
  fields; operator declarations supply left/right/inner operand roles.
- Text fields retain spelling; application code chooses numeric conversions.
  `*`, `+` and `?` describe ordered lists and optional fields. Construction is
  separate from recognition and occurs only for a successful selected parse.
- Walks offer parents-first and children-first order. A children-first rewrite
  exposes rewritten children to their parent, visits each original node once,
  and does not recursively revisit replacement nodes. It preserves its input
  and publishes only a checked complete result, with origins retained.
- The worked calculator demonstrates this surface, constant folding in ordinary
  Level B, and original-source diagnostics. Basic grammars need no AST schema.
  The first schema is inferred from mappings; richer explicit schema and
  multiple-projection syntax remain design extensions to review, not silently
  accepted additions to the source language.
- Regional component wrapper mappings are rejected with `LL_AST_REGION`, because
  that wrapper is absent from the grafted result. Token/infix/prefix and enclosing
  parser-rule mappings remain available; the checker never silently drops a
  requested wrapper mapping. This admission boundary is documented and tested.

**Implementation sequence for this delivery:**

1. AST-STEP02: mapping grammar/lowering, inferred field contracts and generated
   definitions; retain L0/L1/L2 bootstrap parity (AST-AC01/02/06).
2. AST-STEP03: owned result storage, small reference handles, iterative walks,
   checked builders and one-pass reconstruction (AST-AC02/03/04).
3. AST-STEP05: calculator and settings examples, positive/negative backend,
   ownership, origin and rewrite regressions, installed build and guide evidence
   (AST-AC01–06). **Verified for the first-delivery scope above.** The wider analysis/grouping/RexxScript criteria below stay
   individually visible; completing this delivery is not their automatic proof.

**Vision and intended outcome:** let a language author turn a successful parse
into useful application nodes with named fields and lists, then inspect, check
and transform them through a common Level B library. The same application AST
should work with every admitted parser backend. Source ownership and diagnostic
origins survive each stage. Settings, calculators, script engines and later
compiler frontends use the same services; cREXX-specific symbols, types, control
flow and runtime behavior remain consumer policies.

**Why this next:** the product already exposes DFA, LL(1), canonical LR(1),
packrat PEG, Pratt and Earley. Its named view supports arbitrary child counts
and captures, but application-tree projection still has a binary 0/1/2-child
schema. Builders, grouping and restricted attribute scheduling have research
controls, not one settled product API. A stronger AST layer therefore improves
all current algorithms and tests the intended compiler-migration direction.
A new parser should follow a concrete grammar, diagnosis or resource need that
the current portfolio cannot meet. Pika and Squirrel remain research work;
there is no proposed change to their support status here.

**Infrastructure contracts:**

- Keep the ordinary named tree and captures useful without a schema. An optional
  AST declaration defines named node kinds, required/optional fields and ordered
  lists. Variable child counts do not require hand-written binary list nodes.
  Review concrete settings/calculator before-and-after surface examples first.
- Keep original source, parser derivations/forests and application ASTs distinct.
  An ambiguous forest cannot become an AST by silently choosing its first path.
  Existing explicit result/status contracts remain visible.
- Prefer owned node storage and tree-scoped node IDs, with snapshots frozen after
  construction. A checked builder or rewrite creates a new result; source maps
  relate new nodes to old nodes and original text. Derived analysis records
  identify their owning tree/revision. Exact, composite, synthetic and recovered
  origins remain distinguishable. Measure copying and lifecycle behavior before
  promising a particular physical representation or zero-copy API.
- Supply iterative walks in source order and child-before-parent order, named
  field/list access, searches and folds (combine child results into a parent
  result). Add pure checked rewrites for wrapper removal, list construction and
  one declared grouping/lowering example. Failed passes publish no partial AST.
- Start analysis scheduling with acyclic dependencies; then promote the existing
  restricted monotone finite-domain worklist control. Such analyses only add
  facts until stable. Every stronger operation needs its own admitted contract.
  A work budget bounds execution; it does not prove equivalence or convergence
  of arbitrary rewrite callbacks.
- Keep syntax mechanics separate from execution: branch choice, short-circuiting,
  BIF effects and language-specific arithmetic stay in the application evaluator.
  Preserve the Level B bootstrap; production rxc/rxas frontend migration remains
  a separately approved programme.

The implemented count analysis, traversal bounds, ownership rules and rewrite
transaction are described in [AST-CONTRACT.md](AST-CONTRACT.md). In particular,
finite structural reconstruction does not prove arbitrary application semantics.

**Numbered acceptance criteria — open until verified:**

1. **AST-AC01 — Shape and simple authoring.** A basic grammar still works without
   AST declarations. Optional schemas express named required/optional fields and
   zero/one/many lists. Invalid field names, kinds and cardinalities produce
   located grammar/schema diagnostics. Verify runnable settings/calculator
   examples and negative build fixtures against the reviewed surface.
2. **AST-AC02 — Ownership and origins.** Construct and retain two independent
   trees, interleave traversals, release their requests and preserve valid owned
   results. Check original bytes, Unicode spans, trivia/source access, synthetic
   and recovered origins, and rejection of foreign or stale node handles.
   Retain measured storage/copy behavior and named resource-limit failures.
3. **AST-AC03 — Common algorithms.** Iterative walks, field/list lookup, search,
   folds and checked reconstruction preserve order and origins on empty, deep,
   wide and malformed trees. A failed pass leaves its input usable and publishes
   no partial result. Test exact budgets and equivalent opt/noopt execution on
   both VMs; derive bounds for the admitted traversal profiles.
4. **AST-AC04 — Analysis and rewrite contracts.** Validate before/after schemas
   and invalidate derived facts after a relevant change. Execute a structural
   rewrite with stated pre/postconditions, an acyclic attribute calculation and
   a restricted monotone cyclic analysis against independent controls. Cycles,
   unsupported operations and resource exhaustion have distinct diagnostics;
   only proved profiles carry termination/equivalence claims.
5. **AST-AC05 — Backend and application independence.** Equivalent admitted
   unambiguous fixtures produce the same application AST across the current
   applicable backends. Forests need explicit resolution; recovered input
   remains marked and cannot execute as a clean RexxScript program. Migrate
   settings, calculator and one bounded RexxScript slice, preserving outputs,
   errors and control-flow behavior while removing positional tree plumbing.
6. **AST-AC06 — Product continuity and migration evidence.** The supported AST
   library builds through Level B, L0/L1/L2 still agree, and combined/installed
   program builds pass. Update the progressive manual and one compiler-shaped
   builder/validator example. Record the remaining cREXX AST/proof migration
   work without claiming a production compiler migration.

**Numbered implementation steps:**

1. **AST-STEP01 (AC01–06):** inventory the product and research tree contracts;
   present small before/after surface examples, ownership rules and diagnostics.
   **Verified:** Adrian approved the calculator walkthrough and attached
   `then tree` form before requesting implementation. See decisions above.
2. **AST-STEP02 (AC01–03/06):** implement the reviewed schema, owned storage,
   builders, fields/lists and provenance, retaining existing parse views.
   Depends on STEP01; qualify settings and calculator first.
3. **AST-STEP03 (AC02–04):** add iterative traversal/folds, validation and pure
   reconstruction; retain independent controls and explicit bounds. Depends on
   STEP02; no unrestricted rewrite engine is implied.
4. **AST-STEP04 (AC04/05):** promote the admitted analysis schedules and one
   grouping/lowering pass; adapt a bounded RexxScript consumer. Depends on
   STEP03; compare with the existing consumer before removing duplicate code.
5. **AST-STEP05 (AC01–06):** qualify backend equivalence, failure/lifecycle,
   bootstrap and installed examples; update the manual and migration map.
   Depends on STEP02–04; report criteria separately from wider research closure.

**Design references:** the [LLVM AST tutorial](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl02.html)
illustrates an application-oriented syntax tree. [MLIR's rewrite contracts](https://mlir.llvm.org/docs/PatternRewriter/)
separate pattern matching, mutation and traversal/scheduling, and distinguish
bounded recursion from general repeated rewriting. These are reference designs,
not dependencies or proofs for Level L. Reuse and requalify the local `llgroup`
and `llschedule` research controls where their restricted contracts fit.


## Vision and intended outcomes

### Overall programme: common rules, several implementations

Give a cREXX programmer one understandable Rexx-like Level L authoring surface
for lexers, parsers and declared structural transformations, with as much
sharing as their semantics permit. A rule's meaning must survive a change of
algorithm. Backends must explain unsupported constructs at generation time;
they must not silently select a different language, precedence or tree.

**Adrian's scope correction:** this is a general language-engineering toolkit.
The cREXX B/G/C frontends are one demanding example, not the product's purpose
or privileged architecture. Serve lexer-only tools, parsers consuming existing
tokens, compilers/interpreters/transpilers, DSLs, configuration/data formats,
embedded languages and source inspection/transformation. The reusable model
must not depend on cREXX compiler AST classes, globals or runtime semantics.
cREXX is the implementation and authoring environment; languages being
processed need not resemble Rexx.

**Adrian's long-term direction:** cREXX internals, including its AST and the
proofs/invariants around it, should eventually migrate onto the new Level L
infrastructure. Build general source, token/tree, visitor/transformation and
contract/certificate services that can support that migration. cREXX becomes
one consumer, with its dialect/type/runtime policies above the reusable layer.
This research must test migration feasibility and describe the path; the full
production migration is a subsequent programme, not silently included in or
claimed complete by parser PoCs.

Determine, through mathematics and executable cREXX implementations, which
algorithms deserve supported code, which need restricted grammar profiles, and
which should remain experiments. Investigate whether compiler-style staged
parsing and hand-written fixups can be generated or scheduled from declarative
rules, with justified termination, equivalence and cost claims. Include
grammar-author diagnostics as well as errors in the text being parsed.

Adrian clarified that the compiler question concerns **the B/G/C frontends
that create the initial AST**, not a general parsing API for Level C callers.
The end product of this research is an evidence-backed language-tooling
strategy, working comparative implementations and a concrete support recommendation.
The research must cover useful language examples, not just arithmetic speed
tests. Slower parsing may be a good result when it buys useful grammar support,
clearer rules or better recovery. Actual replacement of the production
frontends and final adoption of new language syntax require a later decision.

### Accepted first-product direction — RexxScript, 10 October

Adrian selects RexxScript as the first product consumer. It is an embedded
external-rules/configuration language serving Level G applications: a reduced,
modernized Level G surface, initially without script-defined classes. RexxValue
remains the value substrate. Included expressions should follow Level G rules;
the current RexxScript evaluator is historical evidence, not the new language
specification. Simple classes may be considered later. Level L remains general
language-engineering infrastructure, independent of this consumer.

Adrian subsequently accepts the recommended initial direction with these
explicit qualifications: declarations are unavailable; SIGNAL, GOTO and
INTERPRET are unavailable; host integration is to be confirmed later; use
the Level C BIFs through RexxScript's existing rxfnsc linkage. Adrian explicitly
corrected the earlier Level B reference to Level C and confirmed the existing
connection. This is reuse of the current shared BIF service, not selection or
implementation of a new function library. His item7 refers to
the user-facing host-integration decision, not RS-D07's internal numbering.
All other recommended starting points are accepted as the initial baseline.
No variable/type declaration syntax is introduced: assignment establishes a
script-local name, and reading an undefined name is an error. Initially there
are no script-defined classes or collection-language extensions.

This fixes the first-consumer direction and backend selection while leaving
the explicit host boundary and detailed conformance manifest to be completed.
It neither removes the approved research obligations nor retroactively changes
the frozen old-RexxScript oracle. AC-21's historical gate remains verified for
its original observations. A new product implementation/conformance plan must
follow selection of the decisions below. No production code changes in this
decision pass; STEP-14/15 and their open criteria remain open.

**Historical first-consumer minimum (unchanged for the existing RexxScript
consumer; superseded as the toolkit delivery scope by LANG-D01):** one DFA lexical backend and one
deterministic CFG backend, initially canonical LR(1). Expressions can remain
inside the same LR grammar. Pratt is an optional generated specialization,
admitted only with a proved boundary/tree mapping; it is not required to launch.
Finite-context token passes, a nested-comment counter where required, and pure
source-tree passes supply the composite infrastructure. They do not require a
second general parser or arbitrary fixed-point evaluation. Keep third_party/re2c/Lemon as
established comparison/transition tools; the native PoCs still need product
qualification. LALR compaction is optional and must reject merge-only conflicts.
No native IELR project is required for this first consumer.

Ordinary normalized non-left-recursive PEG is a credible later toolkit backend,
not equivalent to the experimental Pika/Squirrel recursive/recovery profiles.
It need not be shipped for this minimum RexxScript implementation. Pika,
Squirrel/c14 and generalized parsing remain research/reference options and are
not first-product dependencies. Do not promise LR admission of the full new
language until its actual grammar is constructed and checked.

### First-product decision ledger — accepted baseline and remaining detail

The baseline recommendations below are accepted, subject to the explicit
qualifications above. Exact grammar examples, BIF signatures/adapters and
product qualification are still implementation/design work; this acceptance
does not claim they already exist. The host registration, exposure, transport
and write-back contract remains expressly undecided. Exact new Level L
declaration spelling still needs the concrete example review described below.

| ID | Decision | Recommended starting point / evidence needed |
|---|---|---|
| RS-D01 | RexxScript consumer backend set | DFA + canonical LR; retain Pratt as optional for this consumer. Confirm grammar admission, named conflicts and source-tree agreement. The broader initial toolkit set is now LANG-D01, including PEG, LL(1), Pratt and Earley. |
| RS-D02 | Exact Level G expression profile | Pin one documented Level G profile, initially NUMERIC_COMMON, including precedence, associativity, comparisons, concatenation and evaluation order. Share a declarative operator contract and differential tests; do not copy current RexxScript's left-to-right evaluator. Test `1+2*3`, unary/power combinations, `/`, `%`, numeric strings and side-effecting operands. Other options require explicit capability selection. |
| RS-D03 | Values and variable rules | Accepted: RexxValue values throughout evaluation and a Level G-compatible operator adapter; fresh script-local state, implicit names established by assignment, undefined-name errors. Variable/type declarations are unavailable. Scalar-only initially; classes/collections are later work. Pin numeric precision, conversions and truth conditions against the selected Level G profile. Host bindings remain pending under RS-D06. |
| RS-D04 | Initial statement/function subset | Accepted baseline: assignments, grouped expressions, IF/THEN/ELSE, selected structured DO forms, LEAVE/ITERATE, calls and RETURN. SIGNAL, GOTO and INTERPRET are explicitly unavailable. Imports/exits and class declarations are outside the initial subset. Exact output/COLLECT behavior and any script-defined function syntax must be specified in the concrete grammar; no implicit addition of declarations or host effects. |
| RS-D05 | Lexical/context profile | Define source encoding, identifier/case policy, comment forms/nesting, line continuation, statement endings and whitespace/abuttal. Recommend the chosen Level G rules with owned UTF-8 source and byte-span diagnostics; native ASCII-only PoCs are insufficient. Context stages must be finite/declared and source-preserving. |
| RS-D06 | Host and library boundary | Host integration expressly deferred: registration, exposed inputs/outputs, retained facade, transport and write-back are not selected. BIF choice settled: Level C BIFs using the existing RexxScript/rxfnsc linkage, shared RexxValue call context, argument-presence flags and error contracts. Retain that connection and adapt new parsed expressions to it; do not introduce a Level B BIF adapter or a duplicate library. Level G expression semantics and Level C function contracts are distinct. Existing function dispatch/state requirements are integration details to verify, not a renewed library-selection decision. |
| RS-D07 | Effects and execution limits | Host-selected capabilities for existing file/time operations; no ambient shell/import dispatch. Fuel/loop/recursion/output/allocation limits and cancellation have distinct results. These are application policies above pure parsing, not evidence that arbitrary called host code is bounded. |
| RS-D08 | Errors and write-back | Accepted: located stable diagnostic codes, source/rule context and a useful expected set; diagnostic recovery never permits execution after syntax errors. Success-only exposed-output commit remains a recommendation within the deferred host contract, not an accepted host API. Function/I/O effects require their own policy. |
| RS-D09 | Execution and reuse | Parse/validate once into an immutable source-mapped tree or small checked instruction plan; execute with fresh per-run state. Choose between tree interpretation and small private instructions from measured lifecycle needs. Avoid making native-code compilation/JIT/full compiler semantics first-release requirements. Cache keys include language/profile, host signature and grammar/runtime versions. |
| RS-D10 | Compatibility and versioning | Modern syntax gets an explicit language/API version; never guess by trying multiple dialects. Preserve the old implementation as a separately selected legacy path only if actual consumers require it. Existing tests must be classified as retained behavior, intentional change, or unsupported feature. |
| RS-D11 | Level L authoring/diagnostic surface | Preserve the accepted Rexx-like declaration family; approve exact parser/pass/operator declaration spelling using a complete RexxScript example. Use explicit semantics and stage contracts; common spelling does not imply interchangeable CFG/PEG choice. Source diagnostics must hide raw Lemon/table internals by default. |
| RS-D12 | Product acceptance and packaging | Select supported hosts, standalone/embedded entry points and first useful scripts. Require rxc/rxas/rxlink/both-VM correctness, Level G overlap conformance, host/error/lifecycle tests, retained limits and corrected measurements. Resolve/reproduce frozen-tool findings as applicable to the selected product toolchain. Research ports alone are not production qualification. |

The accepted baseline is sufficient to develop the grammar, admission proof,
expression conformance fixtures and isolated parser/evaluator PoC without
waiting for host integration. Prepare the exact feature/semantic/BIF matrix and
concrete language examples next. Keep host-dependent work behind the deferred
contract and present any actual semantic gap rather than asking again about
accepted choices. A numbered product implementation/acceptance plan must be
recorded before production edits. This is a decision ledger
within the existing authority, not a competing implementation plan or a claim
that the new RexxScript product has been built.

### Authorized product implementation — Level L and RexxScript 1

Adrian's subsequent instruction to implement the tool, grammar and connected
interpreter authorizes this concrete implementation for review. The accepted
RS-D baseline above applies; no further approval is needed to implement it.
The exact surface is now delivered as executable examples for careful review,
not declared a released language. All work stays local on `rxlang`.

**Vision:** a reusable Level L command compiles one Rexx-style specification
into checked native DFA and canonical LR tables and source-owned trees. A
RexxScript specification is its first product consumer. A versioned modern
interpreter executes the selected tree with typed RexxValue values, Level G
NUMERIC_COMMON operators, structured control flow and the existing Level C
BIF services. Grammar errors fail the build; script errors are located and
never execute a repaired/damaged tree. Parsing and execution have separate
owned lifetimes. Existing legacy consumers remain explicitly selectable while
the modern implementation is reviewed. Host exposure/registration/write-back
remains the expressly deferred decision; it does not block a standalone runner
or an isolated reusable parsed-program/run API.

Numbered acceptance criteria (current evidence/status follows the steps):

1. **RS-AC01 — Tool and authored source.** A native cREXX Level L command
   accepts the established declaration family, admits regular lexical rules
   and context-free parser rules, and emits deterministic reusable artifacts.
   Test another small language as well as RexxScript; reject unsupported
   constructs, invalid specifications, conflicts and construction exhaustion.
2. **RS-AC02 — Lexer and source.** Native byte DFA handles valid UTF-8 source,
   byte spans, longest match and declared priority; malformed UTF-8 fails with
   an offset. Nested comments and contextual classification have explicit
   stages. Test non-ASCII strings, comments, separators, continuation,
   whitespace/abuttal and damaged lexemes; no ASCII-only success claim.
3. **RS-AC03 — Actual grammar.** The checked-in RexxScript grammar is accepted
   by canonical LR(1), drives runtime parsing and preserves named source tree
   nodes. Explicit modern version and feature manifest cover assignments,
   IF/THEN/ELSE, structured DO, LEAVE/ITERATE, calls, output and RETURN;
   declarations, classes, SIGNAL/GOTO/INTERPRET are rejected.
4. **RS-AC04 — Values and expressions.** Preserve typed RexxValue results,
   implement the documented scalar NUMERIC_COMMON precedence, associativity,
   conversions and left-to-right evaluation, and compare overlap cases with
   the existing Level G compiler. Undefined reads fail; assignment creates
   local names. Non-overlap/unsupported constructs are explicit.
5. **RS-AC05 — Shared BIF integration.** Use existing rxfnsc functions,
   RexxBifCallContext, argument-presence flags and script-local pool/config.
   Exercise nested calls, omitted arguments, typed results and BIF errors.
   Host-dependent services report capability errors until their contract is
   selected; do not silently create a new host API or duplicate the library.
6. **RS-AC06 — Execution and diagnostics.** Parse once, reuse an owned program,
   execute in fresh state; test two independent/interleaved runs, control flow,
   return, missing variables, syntax/BIF errors, fuel/depth/output limits and
   cancellation. Stable codes and source locations accompany failures.
7. **RS-AC07 — Build and review.** Integrate generation/library/runner into
   CMake, run focused rxc/rxas/rxlink checks in optimized/no-opt modes and both
   concrete VMs, retain deterministic generation and negative-exit evidence,
   and document executable examples, compatibility and remaining limitations.
   Existing applicable legacy tests remain valid. No publication is implied.

Implementation steps:

1. **RS-STEP01 (RS-AC01–02):** promote the shared library core to its owning
   rxfnsl directory, keep one maintained source, extend the frontend with the
   accepted lexer declaration family and checked DFA generation. Add reusable
   table serialization/loading and a native generator command.
2. **RS-STEP02 (RS-AC02–04):** author the complete first RexxScript grammar,
   specify lexical/context stages, construct canonical LR tables, resolve
   conflicts through explicit grammar structure and retain rejection examples.
   Depends on RS-STEP01; semantic changes outside RS-D return for review.
3. **RS-STEP03 (RS-AC04–06):** implement typed tree evaluation, structured
   execution, shared Level C BIF dispatch, source diagnostics and execution
   budgets. Keep the host decision isolated. Depends on RS-STEP02.
4. **RS-STEP04 (RS-AC01–07):** connect product generation, library and runner;
   execute conformance/lifecycle/failure fixtures and appropriate existing
   tests with pinned tool provenance. Retain actual failures and limitations.
5. **RS-STEP05 (RS-AC07):** update user/developer guidance, examples and the
   acceptance ledger, and deliver the local implementation for review.

These product criteria supplement AC-01–26; they do not close or remove the
seven open research criteria, including the corrected lifecycle measurements.

### Product acceptance audit — local implementation for review

This table summarizes the original local product qualification. Current ownership
and reorganisation checks are above. Historical passes do not qualify later
changed inputs or establish release, sanitizer or cross-platform status.

| Criterion | Current status and retained evidence |
|---|---|
| RS-AC01 | **Verified locally.** Native tool emits deterministic RXAS for RexxScript and a second language. Nine generator cases cover named admission failures, unsupported semantics, construction exhaustion, UTF-8/I/O and atomic preservation of an old output after failure. Packed source/RXAS images match byte-for-byte. |
| RS-AC02 | **Verified for the documented profile.** UTF-8 source/strings, nested comments, spans, layout, continuations and damage are exercised in the 60-case interpreter matrix and CLI negatives. ASCII identifier restrictions and unavailable wider Level G lexical forms remain explicit. |
| RS-AC03 | **Verified.** Actual grammar: 149 DFA states, 1,612 native canonical LR states; independent Bison admits the identical common graph without conflicts (1,613 states including its final EOF state). Structured statements and exclusions execute in the functional matrix. |
| RS-AC04 | **Verified locally for the declared scalar NUMERIC_COMMON profile.** After integration of develop `54f736804`, all 28 compiler/interpreter scalar comparisons agree in optimized/unoptimized modes on both VMs. The original reproducer returns 0 in all four cells; permanent upstream numeric contracts pass. RS-F01 was closed for that integrated input without claiming complete Level G compatibility. |
| RS-AC05 | **Verified for the accepted initial boundary.** Shared rxfnsc bodies, call context, presence flags and fresh pool/config exercised by nested/omitted calls, typed values, VALUE effects, Unicode and BIF failures. Native Unicode/stream/provider dependencies linked. Host-dependent calls fail explicitly; the deferred host contract remains undecided. |
| RS-AC06 | **Verified for the documented synchronous API.** Sixty cases per complete opt/no-opt module build on each VM include owned program reuse/fresh runs, control and early return, parse-before-execute, failure codes/locations, source/depth/fuel/output/value limits and pre-run cancellation. No asynchronous cancellation or universal BIF allocation bound is asserted. |
| RS-AC07 | **Verified local review delivery; RS-F01 subsequently repaired and integrated.** CMake product targets and four current product CTests pass. Six standalone CLI cases pass. Second-language/image/ownership tests pass 4,725 assertions in each of four cells. Source-emitted grammar also runs all 60 interpreter cases on both VMs. Unchanged legacy correctness evidence is retained and reused; product docs/examples describe compatibility and limits. Publication history is separate from this bounded qualification. |

**RS-F01 — compiler scalar folding disagreement, closed after local integration.**
The pre-integration `rexxscript/tests_functional/float-comparison-reproducer.rexx`
printed `0` with `rxc -n` and `1` with optimized rxc, on both VMs. The broader scalar test first
exposed an interpreter adapter error: using Classic RexxValue comparisons for
typed Level G operands rounded a float through decimal text. The adapter is
corrected to typed operations and its regression passes. That original product increment did not change compiler/VM sources. The
subsequent upstream repair is now integrated at `54f736804`; fresh local tools
return `0` in all four reproducer cells, and both permanent upstream numeric
contracts pass. Historical disagreement outputs remain retained. The integrated 28-case scalar
panel and focused consumer checks also pass under SYNC-AC02–03, closing RS-AC04
for the declared profile. The scalar check now rejects every optimizer
disagreement, including recurrence of the original case.

**RS-F02 — native runner incremental dependency, repaired.**
The runner's original CMake dependency ordered library generation but did not
invalidate its embedded bytecode when the library file changed. The runner and
test link commands now depend on actual runtime library files; the rebuild log
and final native behavior retain the repair evidence. Initial missing Unicode,
stream and filesystem provider linkage is also repaired in the native runner.

**Ownership review:** loaded tables retain portable packed images. Private
lookup calls borrow storage through synchronous read-only EXPOSE arguments;
retained optimized assembly shows attribute links/swaps without binary copies
or per-lookup weak-reference construction. The final DFA image is35,012 bytes,
LR image294,972 bytes and LR directory25,792 bytes. Request/program ownership
still copies at public value boundaries, including language-to-request and
table-to-parser initialization. This is bounded per-request initialization
cost, not a claim that tables are shared without copies across all requests.
Further borrowing/shared-definition work needs a clear owner-lifetime contract;
the current API does not let a borrowed object escape its owner.


## Initial-pilot acceptance criteria

1. **AC-01 — Practical source audit.** Inspect the eight repositories linked
   in Mike's 10 October email; distinguish author claims from observed code,
   record pinned revisions, lexer/parser approaches, diagnostics and reuse
   opportunities. Include Prolog if its source can be located and current
   RexxScript owning sources. Never imply these projects were qualified here.
2. **AC-02 — Native lexer generation.** A generator implemented in Level L
   compiles reusable lexical specifications into executable cREXX scanner
   data/code. Demonstrate longest match, rule priority, progress, spans,
   strings/comments and malformed input. State the supported regex subset.
3. **AC-03 — Native parser generation.** A generator implemented in Level L
   compiles grammar specifications into executable cREXX parser data/code.
   Demonstrate useful expression and statement/format grammars, direct and
   indirect recursion, precedence, full-input checking and rejection cases.
   Precisely bound the supported PEG subset and do not call a reduced engine
   a complete port of the Pika reference implementation.
4. **AC-04 — re2c comparison.** Execute a re2c-backed scanner with the same
   lexical contract; document Level L wrapping versus native generation and
   custom re2c syntax backends, dependencies and replacement limits.
5. **AC-05 — Actual Pika reference.** Pin, inspect, compile and execute the
   linked reference implementation; exercise left recursion, ordered choice,
   malformed input and recovery. If unavailable, retain the concrete failure
   and keep this criterion open.
6. **AC-06 — Useful diagnostics.** Native PoCs report source location,
   unexpected input, contextual expectation and at least one concrete repair
   suggestion; multiple-error fixtures retain valid subsequent statements.
   Differentiate coverage gaps, diagnostic explanation and edit repair.
7. **AC-07 — Mathematics and counterexamples.** Derive lex-first cost and
   memory bounds, break-even and token-boundary equivalence conditions.
   Execute maximal-munch/ordered-choice and contextual-token counterexamples.
   State complexity assumptions and adversarial limitations explicitly.
8. **AC-08 — Measured experiments.** Correctness-gated, reproducible serial
   character/token comparisons record operation counts and timing summaries,
   including lexing cost, short/long identifiers, strings/comments and
   malformed inputs. Inspect then delete raw samples. Label these single-workload research observations,
   not a representative product performance verdict.
9. **AC-09 — Toolchain and reproducibility.** Exercise rxc, rxas, rxlink and
   rxvm; compare optimized/no-opt and both concrete VMs on relevant native
   cases. Retain source/build provenance, commands, outputs and limitations.
10. **AC-10 — Reviewable delivery.** Keep a concise research report, runnable
    commands, acceptance status and a ranked next-experiment list in this
    checkout; show Adrian concrete results and decisions still needing his
    approval. All above criteria must be verified for overall completion.
11. **AC-11 — Existing Rexx-like surface.** Reuse the pinned existing bootstrap
    parser unchanged; execute an authored `.levell` fixture through that AST
    into the native DFA generator and compare token kinds/spans/EOF with the
    established TinyExpr control. Retain original syntax provenance and state
    backend limitations. Document the same surface family for future parser
    tooling without inventing an approved parser syntax.

## Initial-pilot implementation steps

1. **STEP-01 (AC-01, AC-09).** Synchronize selected checkout, record plan and
   source/build provenance; audit owning docs and public repositories.
2. **STEP-02 (AC-02, AC-04, AC-09).** Implement native regex/scanner generator
   and a pinned re2c control; verify common token/span contracts.
3. **STEP-03 (AC-03, AC-05, AC-09).** Implement native grammar generator and
   bounded reverse-DP parser; run the actual Java reference as an independent
   control. Depends on specification contracts from STEP-02.
4. **STEP-04 (AC-06, AC-07).** Add diagnostic/recovery fixtures and formal
   equivalence/cost analysis; test grammar and lexing counterexamples.
5. **STEP-05 (AC-08, AC-09).** Freeze passing experiment sources and run
   bounded serial measurements with raw evidence and exact inputs.
6. **STEP-06 (AC-10).** Reconcile every criterion against evidence, write
   findings and prioritized follow-ons. Production adoption, new cREXX syntax,
   ABI/compiler changes and publication are separate maintainer decisions.
7. **STEP-07 (AC-11, AC-02, AC-04, AC-09).** Add a backend-only adapter from
   the existing validated Level L AST; reproduce the authored TinyExpr proof
   through native DFA and retained direct third_party/re2c/cREXX emission. Depends on the
   established syntax and existing backend proofs; do not edit the Unicode
   checkout or reinterpret Unicode input contracts as ASCII contracts.


## Expanded programme proposed for approval

### Direction and decision boundaries

1. Keep the established `symbol: kind ...` authoring style. Share names,
   terminals, sequences, repetition, source spans, trees and diagnostics.
   Preserve distinct meanings for regular-expression alternation, PEG ordered
   choice, CFG alternatives and longest lexical match. A common spelling must
   not conceal these distinctions. Propose explicit semantic profiles or
   choice declarations; exact new spelling remains provisional in research.
2. Separate **what the rules mean**, **how they are evaluated**, and **how
   damaged input is explained/recovered**. A grammar is not itself an algorithm.
   A PEG authoring profile does not require Pika as its implementation.
3. Treat algorithm selection as a checked lowering. A requested backend either
   implements the declared semantics or reports a located capability error.
   An optional automatic selection mode may choose among equivalent admitted
   implementations and must explain its choice; it cannot silently approximate.
4. Build an executable common IR and cREXX research engines, not merely wrappers
   around foreign parsers. Use pinned upstream implementations as independent
   controls, retaining licenses and attribution. Separate faithful ports from
   modified algorithms so a repair cannot erase an upstream counterexample.
5. Compare all B/G/C initial-AST strategies behind an algorithm-independent
   frontend contract. Include source-owned structure, trivia/abuttal, dialect,
   diagnostics, ownership and subsequent normalization/lowering obligations.
   Keep compilation/import/type-resolution services distinguishable from
   recognition. Do not infer those services from successful syntax recognition.
   Apply the same consumer contract to other languages. Grammar modules,
   embedding boundaries, token-only ingress, lexer-only egress, source
   preservation and reusable tree visitors/transforms are toolkit concerns;
   adapters to a compiler's particular AST are consumer concerns.
   The eventual direction is to replace those legacy adapters with cREXX use
   of the shared Level L infrastructure, including applicable AST invariants
   and proof obligations. Do not bake today's compiler layout into the shared
   model or leave migration feasibility as an untested aspiration.
6. Stay on local `rxlang` in the existing checkout. Approval of this programme
   authorizes research implementations, provisional surface examples and local
   comparisons; it does not adopt a production grammar, replace Lemon/re2c,
   change B/G/C syntax or publish the work. A negative result is useful evidence,
   but does not excuse an unimplemented required comparison.


### Hypotheses and decisive experiments

| ID | Hypothesis | Evidence that could support or refute it |
|---|---|---|
| H-01 | One authoring surface can serve several engines without confusing semantic differences. | The same admitted specification produces equivalent token/tree results on at least two engines; ordered/ unordered choice and unsupported features produce explicit counterexamples or build errors. |
| H-02 | PEG-style rules are easier for cREXX authors on many practical tasks. | Side-by-side Classic keyword, expression, nested structure and damaged-input tasks; compare ability to predict acceptance/tree and repair a rule. Ordered-choice traps and grammar-extension regressions count against PEG. Author preference is not a theorem. |
| H-03 | Much existing parser glue and structural fixup work can be generated. | Replace selected *research copies* of token normalization, contextual classification and tree grouping with declarative rules; compare tokens, source AST and diagnostics. Identify the remaining semantic knowledge explicitly. |
| H-04 | Lexing first can reduce chart/memo work while preserving language and source meaning. | Derive sufficient boundary conditions, check them where possible, then compare scannerless/tokenized recognition and ASTs; include counterexamples involving priorities, keywords, abuttal and predicates crossing boundaries. |
| H-05 | Pika can be useful despite its observed limits. | Faithful native port, independent reference conformance, tested semantic restrictions, sparse scheduling and shared tree experiments; measure useful recovery against complete end-to-end cost and adversarial growth. |
| H-06 | Squirrel is a viable PEG/LR engine with a tractable supported profile. | Run packaged core and c14 separately, port both, compare recursive semantics and tree/recovery results, and audit memo invalidation, expansion overlap and retained objects. A modern manuscript claim is still a claim to test. |
| H-07 | A generated composite can approach deterministic-parser speed without imposing its restrictions on the whole grammar. | Extract proven regular/predictive/operator regions; use an admitted general engine for the rest; prove interface conditions and compare against unspecialized evaluation, including failed optimization attempts. |
| H-08 | Good diagnostics are achievable across algorithms. | Compare located grammar witnesses and source repairs using the same mutation corpus, user-facing rubric and bounded repair oracle. Include LR recovery so Pika/PEG do not receive an artificial advantage. |
| H-09 | Useful automatic complexity certificates are feasible. | A checker derives bounds for declared fragments, rejects false certificates and reports unknown cases honestly; adversarial families stay within every issued bound. |
| H-10 | The shared model serves language engineering beyond compiler parsing. | The same library/IR supports standalone lexing, existing-token parsing, a configuration/data format, an embedded-language example and a tree-based source transformation without importing cREXX compiler internals. |
| H-11 | General Level L infrastructure can eventually own cREXX's AST and its contracts. | A representative builder, traversal and structural rewrite run on shared trees with explicit invariant checks and source provenance, compared through an adapter with the existing compiler representation; document remaining semantic/lifecycle gaps and a staged migration. |

### Proposed implementation portfolio

All required native entries below must execute through the cREXX toolchain.
Generated tables plus a cREXX runtime count as implementation; a foreign
process wrapper alone does not. Each has a documented semantic subset and
negative capability tests. Initial implementations favor clarity over tuning.

| ID | Required native experiment | Role and admission conditions |
|---|---|---|
| ALG-01 | Thompson NFA control, native DFA generator/runtime, and a memoized maximal-munch variant | Extend the existing lexical proof. Regular rules, rule priority, Unicode/binary profiles and captures have explicit contracts; nested comments need a declared stack/counter stage, not a false claim that a DFA handles them. Retain direct re2c emission as an independent backend/control. |
| ALG-02 | Predictive LL(1) plus a Pratt/precedence expression specialization | Small, explainable fast controls and candidate internal specializations. FIRST/FOLLOW conflicts reject LL(1); Pratt is admitted only for an explicit operator grammar, preserving precedence/associativity and tree mapping. |
| ALG-03 | Canonical LR(1) generator/runtime, with LALR state-merging comparison | Deterministic CFG baseline, including a grammar that is LR(1) but not LALR(1). Preserve Lemon as the current compiler control. Use Bison IELR as a reference if available; a native IELR implementation is not needed to test this decision. |
| ALG-04 | Ordinary packrat PEG interpreter/generator path | Semantic control for well-formed non-left-recursive PEG, including predicates and ordered choice. Normalize/memoize costly subexpressions so a rule-only cache does not earn an unjustified linear certificate. |
| ALG-05 | Faithful Pika port plus separately named optimized variants | Cover the reference clause kinds, recursion scheduling and tree selection; exercise documented reference limitations. Compare character, token and selective/sparse evaluation; reject unsupported semantics in the public experimental backend. The existing dense positive-subset PoC is only a control. |
| ALG-06 | Squirrel core port and separate c14 recovery port | Pin algorithm and result semantics, run the Dart references, compare to ordinary PEG on the shared domain, and preserve known differences in recursive cases. Do not port hundreds of archived recovery experiments or equate c14 with the packaged library. |
| ALG-07 | Earley with a shared packed forest; small weighted repair oracle | General CFG and ambiguity baseline with an established conservative bound. Preserve alternatives rather than enumerate exponentially many trees. Handle nullable cycles explicitly, including cyclic/infinite derivations. Small exact recovery is an oracle, not a promised fast production feature. |
| ALG-08 | Composite planner/runtime plus declarative token/tree passes | Use dependency analysis to select admitted regions, execute generated token normalization and structural passes, and retain an unspecialized semantic control. Do not cross an unresolved choice, recursion or context boundary merely because two engines have matching APIs. |

GLL, a correctly bounded GLR variant, adaptive LL(*), regex derivatives and
scannerless generalized parsing remain explicit literature/reference
comparators in the final support matrix. Their capabilities, costs and unique
use cases must be assessed. They are not all mandatory additional native ports:
Earley supplies the first general CFG control and canonical LR supplies the
deterministic one. Add a further native engine only if a concrete unresolved
question requires it, recording the reason and acceptance impact here.

**Initial support hypothesis, not the verdict:** a native DFA backend, an
LR(1)-capable backend and a PEG backend cover the likely core; LL(1)/Pratt can
be internal specializations, and a general CFG engine can supply an explicit
fallback/ambiguity service. Composite planning coordinates these. Pika and
Squirrel must earn their place through the same correctness, usability and
cost criteria. An engine's restricted but reliable profile can be worth
supporting even when a universal claim fails.

### Mathematics to establish

**M-01 — Meanings before transformations.** Define recognition, full-input
acceptance, tree/capture selection and source ownership for regular rules,
ordinary PEG, the selected recursive-PEG extension and CFG rules. Distinguish
language equivalence from tree equivalence and diagnostic behavior. Preserve
named production identity through normalization. Decide which differences
are allowed observations before measuring them. Side effects during speculative
recognition are excluded from the proved core; semantic construction is pure
or delayed until selection.

**M-02 — Conservative static analysis.** Compute nullability, possible first
tokens, progress, reachability, recursive dependency components and relevant
lookahead/context use with sound approximations. Exact LL(1)/LR(1) table
checks and regular automata analysis can settle useful cases. General PEG
equivalence/nonemptiness and general CFG equivalence are undecidable; arbitrary
host callbacks also defeat universal cost/termination checking. Therefore use
`supported with certificate`, `supported with stated conditional bound`,
`unsupported`, and `not proved` as distinct results. A strict requested profile
rejects missing proof at build time; that does not assert that the language is
inherently unparsable. Construction-budget exhaustion is another distinct error.

**M-03 — Automatic grammar manipulation.** Investigate EBNF expansion,
left factoring, guarded left-recursion/operator lowering, automaton
minimization and regular-region extraction. Every rewrite has preconditions,
a local proof argument and an AST/source reconstruction map. A CFG rewrite
cannot be assumed correct for PEG priority. An ordinary conflict is not itself
proof of language ambiguity. Synthesize a witness where possible and otherwise
retain the unresolved obligation. Use exhaustive short-input and generated
counterexample searches as tests of proofs, not substitutes for them.

**M-04 — Composite parsing and fixups.** Classify the existing stages:

- Finite-state token transductions for suitable continuation/terminator rules;
  stack-aware rules for nesting; explicit finite contextual modes where valid.
- Syntax-directed tree construction, grouping and provenance propagation.
- Acyclic attribute dependencies with a generated evaluation order.
- Monotone equations over a stated finite-height lattice, evaluated by a
  dependency worklist; derive the height/edge cost rather than merely cap loops.
- Rewrites with a decreasing measure and deterministic order, or a justified
  confluence argument when order independence is promised.
- Name/type resolution, arbitrary exits and other semantic effects that need
  declared host services and separate bounds.

Prove selected passes preserve intended source meaning and compose. The engine
can derive a schedule from declared rules; it cannot infer arbitrary intended
fixups from an incomplete grammar. For hybrid calls, specify entry/exit,
consumption, context, result multiplicity and error ownership. CFG callees may
need multiple end positions; replacing those with a PEG's first success is
not a valid interface conversion. Memo keys must include every relevant mode
or context identity. Changing context cannot reuse stale `(rule, position)`
results.

**M-05 — Lex-first equivalence.** Extend the pilot's boundary theorem to the
chosen tree and trivia contract. Preserve lexical priority, contextual words,
case policy, byte/codepoint mapping, comments, whitespace/abuttal and raw
malformed-input behavior. Test recognizer predicates that inspect inside or
across a proposed token. When a unique tokenization cannot be certified,
retain scannerless parsing, parser-controlled lexing or an explicit token
lattice; account for that alternative's cost. Do not call ASCII-only results
Unicode qualification or normalize source implicitly.

**M-06 — Cost certificates.** Use input bytes/scalars `n`, token count `t`,
normalized grammar size `g`, lexer-state count `q`, recursion expansion work
`e`, produced tree/forest size `a` and repair budget `d`. Separate generation
cost, parsing work, peak/live retained memory, and repeated-request retention.

`T_total = T_decode + T_lex + T_parse + T_tree + T_passes + T_recovery`.

For a suitable fixed deterministic grammar, seek `O(t + a)` parsing. For
normalized pure non-LR packrat, justify `O(g*t + a)` with the actual
memoization granularity and terminal costs. For Earley retain a conservative
polynomial bound with grammar dependence explicit; forest construction and
enumerating all trees are different operations. For LR extensions account
for every expansion/re-evaluation and all retained match nodes: `O(g*t)` memo
keys alone proves neither linear work nor linear live memory. A length bound
per start position can sum to a quadratic total.

For lexers, analyze bounded backup or implement a linear maximal-munch
construction following Reps; include its grammar-dependent storage/time.
DFA construction can be exponential in grammar size even when scanning is
linear in input. For recovery, report the dependence on `d`, ambiguity and
candidate widths. A runtime fuel limit guarantees a bounded run, not successful
recognition, and must yield `resource exhausted`, never `syntax invalid`.

Certificates must name the backend/version, grammar hash, semantic profile,
input-unit assumptions, output costs and proof obligations. Seek decidable
sufficient conditions, including candidate disjoint-expansion conditions, not
a complete algorithm for classifying every possible grammar's complexity.

**M-07 — Audit recursive PEG and repair claims.** Specify the semantic target
before judging Pika/Squirrel correct. Ordered choice and predicates need not
be monotone under ordering results solely by matched length. A grow-only
iteration is not automatically Kleene iteration of a monotone function. As a
simple sanity check, for `A <- A 'b' / 'a'` on `ab`, the rule body evaluated
with a one-character seed grows to two, but evaluated with the two-character
seed falls back to one. Keeping the longer seed is an additional operational
policy whose meaning needs stating. Compare formal LR-PEG treatments and test
mixed, indirect, nullable, predicate and equal-length/different-tree cases.

For recovery, define what is minimized: token edits, character edits, annotated
missing constructs, structural loss or a lexicographic ranking. Prove that
discarding one candidate at an equal end position is safe for every later
composition before treating it as dynamic programming. Keep multiple candidates
when needed. Audit the updated Squirrel manuscript's hardness and optimality
arguments with grammar size and edit budget explicit. Small exact search can
refute a claim; passing it is not a universal proof. No metric can establish
the programmer's intended repair uniquely in every case.

**M-08 — Trees, transformations and proof ownership.** Define the generic
services needed for eventual AST migration: node/production identity, ordered
children, optional sharing, source/token links, trivia, error nodes, traversal,
construction, transformation and owned lifetime. Keep lossless source trees,
syntactic ASTs and later semantic IR distinguishable rather than forcing them
into one physical record or one set of invariants. Describe how richer typed
nodes and consumer attributes attach without dependence on compiler globals.

For representative transformations, state preconditions and postconditions
for structural integrity, source provenance and meaning preservation. Track
which facts/certificates survive a mutation and which must be invalidated or
rechecked, using stable identities and explicit revisions where needed. Keep
runtime invariant checks, bounded exhaustive validation and mathematical
proofs distinct. Prototype a compiler-like builder, visitor and grouping or
lowering rewrite on the reusable structures. The migration outline must cover
later symbol/type/control/ownership facts and their consumers; passing this
prototype does not prove the existing compiler's entire semantic pipeline.

### Corpus and usability protocol

Use three complementary groups, selected and frozen before comparison:

1. **Useful consumers.** Existing authored TinyExpr and Unicode rule-file
   lexer examples, RexxScript expressions/statements, Mike's Pascal/PLM plus
   contrasting BASIC, Lisp and Prolog constructs, and representative B/G/C
   frontend fragments from owning tests. Inventory all eight audited frontends
   and record why selected fragments cover or miss their requirements. At
   least one complete small language/format must reach source-to-tree output;
   the programme is not restricted to fragment recognition.
   Include a complete configuration/data format, a small embedded-language
   boundary example, a standalone lexer and a parser fed by an independently
   supplied token stream. Include one tree visitor/transform that produces
   useful output, preserving or deliberately mapping original source/trivia.
   These are coequal use cases, not demonstrations built on a cREXX-specific
   frontend abstraction. Assess streaming, incremental/editor reuse, grammar
   modules and captures as support dimensions; qualify only the modes actually
   implemented in this research and give explicit errors for unavailable ones.
2. **Rexx-specific semantics.** Soft keywords and assignment/label precedence,
   abuttal versus blank concatenation, nested comments, comma continuations,
   clause boundaries, Classic IF/THEN/ELSE and PARSE templates, B/G procedure
   grouping/declarations, source spans and malformed/incomplete text. Separate
   authored syntax trees from later semantic canonicalization. Do not claim
   full B/G/C conformance from this representative prototype corpus.
3. **Adversaries and damage.** Existing Pika/munch counterexamples, overlapping
   LR expansion, nullable cycles, predicates, prefix choices, ambiguity,
   deep nesting, long lexical atoms and repeated calls. Generate deletion,
   insertion, substitution and truncation cases; retain a held-out set to
   reduce tuning to known examples. Include damage in lexing as well as parsing.

Differentially compare only engines that implement the same declared semantics.
When two profiles intentionally differ, retain the distinguishing input and
explanation rather than count the difference as a bug. Compare trees/spans as
well as acceptance. Use pinned independent oracles and canonical tree adapters,
so engines sharing a mistaken lowering are not their own only validation.

Two different diagnostic audiences need evidence:

- **Grammar author:** source rule names/locations, readable witness input,
  conflicting alternatives or recursion path, explanation of the backend
  limitation, and a semantics-preserving suggestion when established. Mark a
  behavior-changing suggestion explicitly. Show before/after trees for a
  proposed rewrite. A LALR merge conflict should be distinguished from genuine
  ambiguity and from a rule-order choice that hides a valid alternative.
- **Program author:** unexpected input, a short contextual expectation,
  source line/caret, related opening delimiter, stable code, and explicit
  suggested edits or missing/skipped nodes. Recovery must preserve later
  correct structure and must not present a repaired parse as clean acceptance.

Compare ordinary farthest-failure reporting, PEG labeled failures, Pika
islands/explanation, Squirrel/c14 and LR expected-token plus CPCT+-style local
repair. Separate diagnostic quality from repair speed. Retain local versus
global optimality and chosen cost model. A repair search can time out honestly.
Grammar-build diagnostics must not require reading automaton state dumps;
those remain optional detail. Prepare concrete Rexx-style examples for Adrian
to judge. Without a user study, report authoring assessments as assessments,
not a proved universal claim that PEG is more natural.

### Expanded acceptance criteria — current status in the audit below

12. **AC-12 — Shared semantics and build admission.** A documented common
    model preserves the existing lexer surface and distinguishes parser
    semantics explicitly. At least two backend pairs consume the same authored
    rules on their shared domain. Tests show located build-time rejection of
    unsupported constructs, unresolved proof requirements and budget exhaustion.
13. **AC-13 — Current frontend and logical-grammar inventory.** Map relevant
    B/G/C lexer/glue/parser/tree/semantic responsibilities to current sources
    and tests. Reconcile the historical logical grammar and new surface;
    record production integration obligations and uncovered cases explicitly.
14. **AC-14 — Required native portfolio.** ALG-01 through ALG-08 execute with
    documented inputs, trees or forests, supported subsets and failure paths.
    Pika and Squirrel are identifiable ports plus separately named experiments;
    reference wrappers or the pilot subset cannot satisfy those native ports.
15. **AC-15 — Independent semantic evidence.** Retain pinned reference runs,
    exhaustive bounded/differential cases and minimized disagreements for
    recognition, precedence, recursive choices, trees, spans and errors.
    Distinguish definition differences from implementation faults.
16. **AC-16 — Generated stages and hybrid proof.** Execute at least one token
    normalization, contextual-word classification and structural AST pass from
    declarations, plus one parser-region specialization. Compare with explicit
    reference passes. Retain preconditions, provenance maps, termination/cost
    arguments and a rejected unsafe composition.
17. **AC-17 — Lexer and lex-first obligations.** Retain scannerless/tokenized
    parity on a stated useful domain, a checked sufficient boundary condition,
    counterexamples outside it and the maximal-munch improvement/bound. UTF-8,
    byte spans, malformed raw input and binary/NUL capability are either
    exercised or rejected explicitly per backend. Existing wider contracts
    cannot silently become an ASCII contract.
18. **AC-18 — Mathematics and executable certificates.** Deliver M-01 through
    M-08 with theorem/assumption/counterexample/open-conjecture labels. A
    cREXX checker emits certificates for at least regular lexing, deterministic
    parsing and normalized non-LR PEG, plus a conservative general-CFG bound;
    the recursive-PEG/composite analyses may soundly return unknown. Include
    certificate-validation failures and adversarial scaling checks.
19. **AC-19 — Grammar-author explanation.** Executable examples explain an
    unreachable lexical rule, empty/nonprogressing rule, recursion cycle,
    LL conflict, LALR-only conflict, genuine CFG ambiguity and PEG priority
    surprise in source terms. A bounded witness search reports failure to find
    a witness honestly; it cannot prove ambiguity by exhibiting unrelated
    continuations. Retain a rule-repair walkthrough understandable without a
    Lemon state report.
20. **AC-20 — Input diagnostics and recovery.** Compare the named recovery
    families on frozen and held-out damage corpora with a stated quality rubric,
    explicit repair costs, a small exact oracle and useful subsequent structure.
    Retain cases of bad recovery, not only successes. Recovered results and
    resource failures remain distinct from valid or syntactically invalid input.
21. **AC-21 — Useful frontend prototypes.** Deliver a complete small-language
    source-to-tree example and representative B/G/C initial-AST comparisons,
    including the Rexx-specific cases above. Report evidence for choosing PEG,
    CFG or a composite for each dialect without claiming full replacement.
22. **AC-22 — End-to-end cost and lifecycle.** Freeze inputs and compare
    generation/initialization, decoding/lexing, recognition, tree creation,
    transformations and recovery separately and together. Summarize serial
    measurements and counters for work, retained objects and peak/storage
    behavior, including repeated small-after-large and interleaved owned
    contexts. State stack/resource limits and actual platform scope; delete raw
    samples after checking the summary.
23. **AC-23 — Toolchain and reproducibility.** Native modules use rxc, rxas,
    rxlink and both concrete VMs; focused optimized/no-opt parity covers new
    semantics. Check deterministic generation and negative exits. Preserve
    exact source/build/input hashes and runnable scripts; apply performance
    governance before new measurements and reuse unchanged pilot evidence.
24. **AC-24 — Decision report and reviewable syntax.** Publish locally a support
    matrix for every candidate: semantics, proven/conditional/measured bounds,
    grammar restrictions, diagnostics, code maturity and maintenance cost.
    Recommend supported core, specialist/experimental and rejected options,
    with concrete Level L declarations and B/G/C frontend diagrams/examples
    for final approval. Explain what remains unknown and the cost of resolving
    it. Every required experiment above must be dispositioned against its own
    criterion; merely choosing a favorite engine does not complete the plan.
25. **AC-25 — General language-engineering applicability.** Run the standalone
    lexer, existing-token parser, complete format, embedded-language boundary
    and source-tree transformation examples described above. They share the
    common model and reusable cREXX libraries without depending on compiler
    ASTs or B/G/C semantics. The final matrix assesses generation/library use,
    grammar reuse, captures/trivia, binary/Unicode, streaming, incremental
    operation and embedding; distinguish executed capability from recommended
    later product work. cREXX frontend success alone cannot satisfy this gate.
26. **AC-26 — AST/invariant migration feasibility.** Execute a representative
    builder, traversal and rewrite using the general Level L tree/contract
    infrastructure, with positive and deliberately violated invariant cases,
    provenance preservation and certificate invalidation/revalidation. Compare
    the observable result to a current cREXX case through a documented adapter.
    Deliver a staged migration map for compiler AST and proof/invariant
    consumers, including lifecycle and semantic gaps. Keep full migration open
    for the later production programme; generic parsing alone is insufficient.


### Expanded implementation steps and gates

8. **STEP-08 (AC-12, AC-13, AC-24, AC-25, AC-26) — Approve strategy; freeze questions.**
   Present this plan now. After approval, inventory source semantics and
   consumers, pin reference versions, propose the common IR and provisional
   declarations, and freeze the hypothesis/corpus manifest. Approval permits
   these research surfaces, not a final product-language change. No new parser
   implementation precedes approval of this programme.
9. **STEP-09 (AC-12, AC-15, AC-18, AC-23) — Build semantic controls and analysis.**
   Implement the shared source/tree model, capability checks, progress/dependency
   analysis and canonical comparison tools. Use a plain PEG control and
   independent CFG/reference results. Begin the proof/counterexample ledger.
   Depends on STEP-08; avoid timing engines with incompatible semantics.
10. **STEP-10 (AC-14, AC-15, AC-17, AC-18, AC-23) — Implement traditional
    controls and lexers.** Extend native lexical backends; implement LL/Pratt,
    LR(1)/LALR and Earley/forest engines. Establish end-to-end tree correctness,
    UTF-8/profile boundaries and certificates before optimization. Reuse
    unchanged third_party/re2c/native evidence, extending only for new obligations.
11. **STEP-11 (AC-14, AC-15, AC-18, AC-20, AC-23) — Audit and port Pika and
    Squirrel.** Execute pinned upstream references, including Squirrel c14;
    reproduce positive and adversarial cases. Port algorithms with explicit
    semantic correspondence, then investigate sparse/token scheduling, shared
    trees and safe grammar transformations as separate variants. Report the
    first substantive feasibility findings before polishing implementations.
12. **STEP-12 (AC-13, AC-16, AC-17, AC-21, AC-25, AC-26) — Generate composite language tools.**
    Build the selected transducer/attribute/rewrite mechanisms and at least one
    safe regional specialization. Compare staged and more direct grammars on
    useful language and B/G/C AST cases. Depends on semantic and engine controls;
    later type/import/exit behavior remains an explicit service obligation.
    Execute the independent lexer/token/format/embedding/transformation consumers
    so the common architecture is tested outside compiler construction.
    Prove the representative tree/contract migration slice and record the
    remaining compiler-internal migration requirements.
13. **STEP-13 (AC-19, AC-20, AC-21) — Explain rules and damaged programs.**
    Implement grammar witnesses and input recovery controls, exercise the frozen
    and held-out corpora, and produce authoring walkthroughs. Include lexer
    failures and context recovery. Use exact short-input repair to challenge
    optimality and pruning claims; preserve valid-input semantics.
14. **STEP-14 (AC-18, AC-22, AC-23) — Freeze, measure and challenge bounds.**
    Correctness-gate each candidate, freeze its source/build/input identity,
    then run serial research measurements and increasing adversarial sizes.
    Measure both cold service cost and reuse where relevant. Validate owned
    context teardown and memory retention. No production performance edit is
    authorized; if proposed later, its separate first-Release-verdict gate
    applies before broad closeout work.
15. **STEP-15 (AC-12 through AC-26) — Reconcile and recommend.** Write the
    general toolkit support matrix, proofs/limits and executable examples;
    include B/G/C as one consumer recommendation and a staged AST/invariant
    migration route onto the generic infrastructure.
    Keep unmet criteria open. Ask Adrian to decide final syntax, supported
    algorithms and any production frontend migration using those concrete
    results. Local research completion and production adoption are different
    milestones.

The steps are dependency order, not a requirement to delay all feedback until
the end. Report meaningful counterexamples and capability decisions as they
appear. A failed universal claim narrows a proposed supported profile only
with an explicit record; it must not erase the requested investigation.

### Primary research routes

These sources guide the investigation; cited claims still need matching
implementation assumptions and local evidence. No new runtime experiments
were performed merely to prepare this approval plan.

- [Ford: PEG semantics and decidability](https://bford.info/pub/lang/peg.pdf).
- [Medeiros et al.: formal left-recursive PEG extension](https://arxiv.org/abs/1207.0443)
  and [Tratt: direct left-recursive PEG analysis](https://tratt.net/laurie/research/pubs/html/tratt__direct_left_recursive_parsing_expression_grammars/).
- [Pika paper](https://arxiv.org/html/2005.06444) and
  [pinned reference](https://github.com/lukehutch/pikaparser/tree/8050ceaf70c79a04416fdbdc374e92f9478de7b0).
- [Squirrel current repository manuscript](https://github.com/lukehutch/squirrelparser/blob/bb1ae84701060462636c07c814286ac2875e0568/paper/squirrel_parser.tex),
  [packaged parser](https://github.com/lukehutch/squirrelparser/blob/bb1ae84701060462636c07c814286ac2875e0568/dart/lib/src/parser/parser.dart)
  and [recovery-engine status](https://github.com/lukehutch/squirrelparser/blob/bb1ae84701060462636c07c814286ac2875e0568/dart/experiments/recovery/README.md).
  Keep the [older arXiv presentation](https://arxiv.org/html/2601.05012)
  distinct from the newer source/manuscript.
- [Reps: linear maximal-munch tokenization](https://research.cs.wisc.edu/wpis/abstracts/toplas98b.abs.html).
- [Bison: LR table families](https://www.gnu.org/software/bison/manual/html_node/LR-Table-Construction.html)
  and [grammar counterexamples](https://www.gnu.org/software/bison/manual/html_node/Counterexamples.html).
- [Scott/Johnstone: GLL](https://www.cs.rhul.ac.uk/research/languages/csle/GLLparsers.html)
  and [generalized LR research](https://ir.cwi.nl/pub/14233).
- [Parr/Harwell/Fisher: adaptive LL(*)](https://www.antlr.org/papers/allstar-techreport.pdf).
- [JastAdd: attributes, rewrites and finite-height fixed points](https://jastadd.cs.lth.se/web/documentation/concept-overview.php).
- [CPCT+: LR error recovery](https://soft-dev.org/pubs/html/diekmann_tratt__dont_panic/),
  [PEG labeled-failure recovery](https://arxiv.org/abs/1806.11150)
  and [automatic PEG recovery annotations](https://arxiv.org/abs/1905.02145).



## Retained acceptance contracts

These accepted criteria and step IDs remain binding. Completed build/documentation
increments are historical; unverified language, platform and replacement gates
remain open. The reorganisation does not reopen unchanged semantic results or
claim a new platform pass. Current ownership is described above.

1. **CI-AC01 — Portable checkout.** Remove Windows device names from tracked
   evidence and generated comment fixtures, preserving their bytes. A permanent
   path guard runs in the existing publication safeguards; Windows checkout
   succeeds in the resulting hosted run.
2. **CI-AC02 — Native linkage.** The modern RexxScript runner links its static
   providers before their RXPA implementation, as the existing `crexx` runner
   does. Local product and runner checks pass; Linux Release and CodeQL builds
   confirm GNU linker closure.
3. **CI-AC03 — Green publication.** Commit the repair and evidence together,
   push to `develop`, and inspect the resulting Build CREXX and CodeQL runs.
   Diagnose remaining failures; retry service failures when the logs establish
   that cause. Report actual green results in the conversation, with no
   status-only follow-up commit.

1. **CI-STEP01 (CI-AC01/02):** retain failed-job evidence and identify causes.
   **Verified:** `bb973735f` Build run `38298866712` fails Windows checkout on
   `nul.txt` and Linux linkage on RXPA symbols. CodeQL `38298866314` hits the
   same linkage failure. macOS ARM product/QA/package checks pass; Apple accepts
   the installer but ticket retrieval loses its network connection (Error 68).
2. **CI-STEP02 (CI-AC01/02):** repair fixture paths and runner link order, add
   the path regression, and run focused local checks. **Locally verified:**
   product build, four CTests, native runner checks, 586 comment-panel
   observations, Windows/path and package guards pass. All renamed input bytes
   and 3,169 retained panel hashes match. See [current validation](#current-validation).
3. **CI-STEP03 (CI-AC03):** publish the qualified repair and follow normal
   hosted CI to completion. **The enclosing commit is the approved repair
   payload.** Hosted Windows/GNU link and final green-workflow evidence follows
   publication and is reported in the conversation. Depends on STEP02.

1. **PUB-AC01 — Product closure.** `stage-product` builds `rxlang`,
   `rexxscript1` and their libraries from a fresh build directory. Inspect the
   generated dependency graph and record the command, input identity and
   successful result here; delete the temporary build log after inspection.
2. **PUB-AC02 — QA closure.** Standard QA preparation builds the generated
   Level L and modern RexxScript test images and concrete VM prerequisites.
   Existing tests execute in the smoke/correctness tiers on the combined tree.
3. **PUB-AC03 — Integration correctness.** Commit the reviewed manual/examples,
   integrate current `develop`, and pass product smoke plus the affected Level L,
   legacy/modern RexxScript, compiler-exit, RXPP and Unicode correctness tests.
   Reuse unchanged deeper backend evidence with explicit input boundaries.
4. **PUB-AC04 — Installed tools.** In an isolated install prefix, generate a
   grammar, compile/assemble/link its consumer and run it on available concrete
   VMs. Run modern RexxScript with Unicode, expressions and BIFs; check syntax
   failure prevents execution. Retain permanent package smoke coverage.
5. **PUB-AC05 — Reviewable publication.** Include receipts and current plan in
   the substantive publication, push the qualified descendant to `develop`
   without force, and verify the remote commit. Normal automatic publication
   CI follows; no new release tag or full-programme completion is claimed.

1. **PUB-STEP01 (PUB-AC01/02/04):** repair stage and QA dependencies; add
   installed-consumer checks. **Verified.** No language-design change.
2. **PUB-STEP02 (PUB-AC03):** commit the manual and merge current `develop`
   into the authorized `rxlang` checkout. **Verified:** manual `fb4972050`, clean
   merge `2121969ce` includes `a415090dc`. Depends on STEP01.
3. **PUB-STEP03 (PUB-AC01–04):** fresh product/QA build, relevant correctness
   runs and installed checks; fix reproduced integration defects. **Verified**
   by [current validation](#current-validation).
4. **PUB-STEP04 (PUB-AC05):** record exact inputs/results, commit and publish
   only after STEP03 passes; verify remote state. **Local publication payload
   complete; approved delivery is the fast-forward push of the enclosing commit
   to `develop`.** Remote identity and actual CI state are reported in the
   conversation, without a status-only follow-up commit.

1. **DOC-AC01 — Learning sequence.** Explain lexer, token, grammar, parser and
   tree before relying on those terms; put the first runnable example before
   advanced backend/composition material. Verify by reading the whole guide.
2. **DOC-AC02 — Accurate reference.** Check declarations, defaults, tree/API
   contracts, all request outcomes and resource settings against owning source.
   Retain practical restrictions with the relevant operation, and explain what
   the reader should do. Check links and execute changed tutorial commands.
3. **DOC-AC03 — Enduring prose.** Remove dated delivery/QA/roadmap statements,
   repetitive qualification provisos and unexplained specialist vocabulary.
   Preserve needed usage constraints without making claims beyond the code.
4. **DOC-AC04 — Reviewable closeout.** Record corrected documentation errors,
   validation and remaining product scope here. Production code and retained
   implementation qualification remain unchanged unless a separately justified
   repair becomes necessary.

1. **DOC-STEP01 (DOC-AC01–03):** audit the current guide and owning code, then
   arrange introduction, tutorial, progressive examples and reference. **Verified.**
2. **DOC-STEP02 (DOC-AC01–03):** rewrite the manual and fill reference gaps. **Verified.**
   Adrian additionally asks for practical first-use cases for each backend.
   Include a runnable PEG configuration-value example, using existing syntax,
   with priority, reversed-priority and CFG controls in the focused teaching
   panel. This changes examples/tests, not the Level L language or runtime.
3. **DOC-STEP03 (DOC-AC02–04):** check internal links, current source contracts
   and executable instructions; record the results and present the revised guide.
   **Verified** by [current validation](#current-validation).

1. **CONT-AC01 (LANG-AC05/10/11/13):** every remaining construction or recovery
   safeguard is a connected optional host budget or a named, checked format/
   stack ceiling. Tiny budgets produce resource outcomes, raised successful
   budgets preserve artifacts, loaders accept valid artifacts independently of
   build defaults, and overflow fails before index/allocation arithmetic.
2. **CONT-AC02 (LANG-AC03/11/12):** compile pinned Unicode 17 property sets using
   Level B inputs without Level G; exercise identifiers and scalar boundaries
   against independently read raw data, retaining exact source-byte spans.
   Comparison-key and display policies retain their full LANG-AC03 obligations.
3. **CONT-AC03 (LANG-AC06/07/08/14):** regress diagnostic category, expected
   tokens, authored context, byte/line coordinates and later valid structure
   across admitted backends; keep resource failure distinct from syntax,
   recovery and ambiguity, and preserve no-execution/last-good-output contracts.
4. **CONT-AC04 (LANG-AC12/13):** retain exact test inputs/tool identities and
   update runnable examples, guide and this plan with measured scope and gaps.

1. **CONT-STEP01 (CONT-AC01/04):** inventory fixed limits and connect remaining
   construction/recovery settings; preserve safe stack/format ceilings, add
   loader/overflow/budget regressions and qualify the Level B bootstrap.
2. **CONT-STEP02 (CONT-AC02/04):** after STEP01, generate pinned property ranges,
   add optional charset property references, extend the same settings example
   and qualify scalar/data/build provenance without a Level G cycle.
3. **CONT-STEP03 (CONT-AC03/04):** after the relevant STEP01/02 services, improve
   accurate contextual/Unicode diagnostics and grammar/target resync; add
   backend-specific regressions and retain failures honestly.
4. **CONT-STEP04 (CONT-AC01–04):** reconcile acceptance evidence and update the
   guide and status. New language semantics beyond approved LANG-D03–10 still
   require maintainer review; routine lowering and test details are authorized.

| Continuation criterion | Current evidence and boundary |
|---|---|
| CONT-AC01 | Eighteen connected host settings, discoverable defaults/ceilings, checked dynamic LR item membership, fixed stack/format messages and independent loaders. `resource-cleanup/limits-fixed` and `limits-noopt`: 35 commands each plus 24 boundary/loader checks each; 114 request cases + seven build controls in each complete library mode. Defaults remain engineering safeguards; global memory/time and workload calibration are not established. |
| CONT-AC02 | Level B compiles 45 pinned Unicode 17 sets from three checksummed raw files. Independent raw-data comparison passes four native cells per library mode. XID lexer: 12,523 inputs in eight cells, including every interval edge/neighbor, random scalars, NUL and byte spans; wrong-profile/unknown-property refusals. The seventh settings lesson executes source/RXAS on both VMs. No normalization/folding key or terminal-cell service is claimed. |
| CONT-AC03 | 78 located/message/speculation cases per complete library mode across all six parser routes. 160 record-recovery cases per mode, including malformed UTF-8 and source ordering. Eight origin checks retain original bytes and associate sorted diagnostics with error nodes. L1/L2 declaration candidates: 24 cases + twelve last-good-output checks per library mode across quote/comment/group damage. Strict parsing and execution remain strict; nested/token-stage recovery and unclosed target comments retain their stated limits. |
| CONT-AC04 | User guide, diagnostic assessment, mathematics and progressive example updated. current Level B-only cold/no-op property bootstrap runs three cases with zero re2c calls and no Level G source tree. Exact L1/L2 and opt/no-opt self-description agree. Current consumer checks, six native CLI controls and unoptimized 60-case interpreter cells pass. |

1. **EX-AC01 (LANG-AC02/03/09/13):** inventory implemented comment behavior and
   record a concrete generalized-comment proposal with boundaries, admission,
   diagnostics and tests. New spellings/semantics require Adrian's decision;
   proposal preparation is not implementation or acceptance of that feature.
2. **EX-AC02 (LANG-AC01/05/07/12/13/14):** runnable small grammars demonstrate
   predictive admission, left recursion, common-prefix/ordered choice and
   genuinely ambiguous arithmetic. Verify admitted/refused engines, complete
   input, forest multiplicity and honest error messages on both VMs and
   optimized/unoptimized consumer builds. Explain that a failed parse path
   is not itself a user error, and a conflict is not proof of ambiguity.
3. **EX-AC03 (LANG-AC04/07/12/13):** a complete calculator includes grammar,
   generated module, Level B tree evaluator, input, build instructions and
   expected results. Test precedence, associativity, grouping, decimals,
   alternative comment delimiters, malformed input and located division by
   zero. Parsing must succeed before evaluation; invalid input emits no result.
4. **EX-AC04 (LANG-AC09/12/13):** reconcile modern and legacy RexxScript guides
   against their sources; inventory syntax mechanics that can migrate and
   semantics that remain handwritten. No legacy removal or forced Pratt
   conversion without consumer/semantic proof and maintainer direction.

1. **EX-STEP01 (EX-AC01/04):** inspect source and prepare the comment/migration
   review. Record remaining decisions before changing language architecture.
2. **EX-STEP02 (EX-AC02):** add backend selection examples/probe and guide
   walkthrough using existing syntax. No automatic fallback or new resolver.
3. **EX-STEP03 (EX-AC03):** implement the calculator using existing operator,
   lexer/comment and source-tree APIs; qualify its complete toolchain.
4. **EX-STEP04 (EX-AC02–04):** qualify focused teaching regressions, retain
   exact inputs, correct guide drift and commit the concrete increment.
5. **EX-STEP05 (EX-AC01):** after approval of the concrete generalized-comment
   contract, implement it in the common model/runtime/emitters/bootstrap and
   qualify quoting, prefixes, nesting, newline and recovery behavior across
   admitted backends. Pending design approval; no implication that teaching
   example completion satisfies this step.

1. **COM-AC01 (EX-AC01, LANG-AC02/05/13):** the three forms, distinct custom
   delimiters, optional `outside PATTERN` and `newlines keep` execute. Multiple
   forms choose longest opener and reject duplicate openers with related source
   locations. Unknown/empty/invalid/recursive/nullable shielding and unsupported
   qualifiers fail admission; no ignored declaration.
2. **COM-AC02 (LANG-AC03/04/07/10):** a shared coordinator keeps exact original
   bytes/spans, preserves line-comment CR/LF and declared block newlines, honors
   quote/regular-pattern opacity and same-form nesting/close priority. Named
   unterminated diagnostics point to the original opener. UTF-8, source/work
   limits and contract errors remain distinct; scanning progresses or exhausts.
3. **COM-AC03 (LANG-AC08/09/14):** strict/recovery routes use the same comment
   policy; delimiters/separators inside opaque regions cannot trigger spurious
   comments or record recovery. Unclosed blocks never become clean/executable.
   Recovery and later valid structure remain checked within admitted profiles.
4. **COM-AC04 (LANG-AC11/12):** source/RXAS, both VMs and complete opt/no-opt
   Level L builds pass focused independent comment panels. L0/L1/L2 agree,
   current settings/calculator/RexxScript consumers pass, and the required
   Level B-only bootstrap remains independent of Level G/re2c. Reuse unchanged
   broader proof; no new nested aggregate CTest.
5. **COM-AC05 (LANG-AC12/13):** grow the existing worked example, document simple
   and advanced contracts/limits, retain exact test/tool identities and reconcile
   COM/EX progress without closing unrelated LANG/research obligations.

1. **COM-STEP01 (AC01/02):** implement checked common descriptors/coordinator and
   preserve the legacy prepared/definition constructors. Add focused native
   preparation tests before broad integration.
2. **COM-STEP02 (AC01/04):** extend shared lowering/self-grammar, compile shielding
   patterns through the existing regular builder, emit descriptors/packed images
   identically in source/RXAS, and rebuild/verify all bootstrap stages.
3. **COM-STEP03 (AC02/03):** connect strict/recovery/source-author routes and test
   original spans, errors, newline boundaries, speculative quote patterns and
   lowered budgets. Depends on STEP01/02.
4. **COM-STEP04 (AC01–05):** run backend/consumer/optimization panels and focused
   build lifecycle checks, update guide/plan/receipts and preserve remaining
   programme scope. Depends on the relevant preceding implementation steps.

| Original step | Current state |
|---|---|
| STEP-01–07 | Initial pilot complete; AC-01–11 verified. |
| STEP-08 | Approved strategy, semantic/source inventory and corpus established. |
| STEP-09 | Shared analysis, capability contracts and independent controls implemented/tested; final certificate audit open. |
| STEP-10 | Required traditional parser/lexer implementations execute; final portfolio/evidence reconciliation open. |
| STEP-11 | Faithful native Pika/Squirrel/c14 ports and distinct refinements execute; final bounds/cost audit open. |
| STEP-12 | Representative stages, tree migration, useful-language and composite consumers execute; applicability matrix drafted. |
| STEP-13 | Grammar-author and recovery representative gates verified, including held-out lexical damage. |
| STEP-14 | Lifecycle correctness passes in four cells. First timing block is inconclusive; corrected release boundary passes, uncontended serial measurements remain. COST.md and tranches17/18. |
| STEP-15 | REPORT.md/AUDIT.md drafted; final acceptance reconciliation and cost interpretation open. |
| RS-STEP01–02 | Product tool, packed RXAS generation, UTF-8 source services and actual RexxScript grammar implemented and locally verified. |
| RS-STEP03 | Interpreter implemented; historical four-cell evidence retained, and the 60-case panel passes both VMs after develop integration. RS-F01 is closed and RS-AC04 verified for the declared scalar profile. |
| RS-STEP04 | CMake/library/native runner, dual-format artifacts, second language, negative exits and local toolchain checks verified within the documented scope. |
| RS-STEP05 | Local implementation and review guides delivered; qualification limitation and original research obligations remain explicit. |
| GUIDE-STEP01–03 | Comprehensive Level L guide, runnable settings language and diagnostic audit delivered; GUIDE-AC01–03 verified locally. This row records the earlier guide checkpoint; the current recovery increment is below. LL-D01–06 remain open for their full obligations. |
| BOOT-STEP01–05 | Checkpoint 1 implemented and verified locally: Level B foundation, L0/L1/L2 bootstrap, native Unicode lexing and retained consumer/lifecycle evidence. BOOT-AC01–08 verified within the stated native-host scope; unmerged for review. |
| LANG-STEP01 | Portfolio and proposed common-surface direction approved; one settings language is the progressive worked example. |
| LANG-STEP02–08 | In progress: all six components have authored product routes. Shared messages, declaration resync, flat-record recovery, Pratt/LR regions and tree projection have executable increments and bounded evidence below. Captures, eighteen host budgets, pinned property classes, escaped coordinates and lexical/UTF8 recovery have bounded functional evidence under CONT-STEP01–04. Full Unicode comparison/display policies, wider composition/recovery contracts, default calibration and final qualification remain open. |

1. **LANG-AC01 — Six integrated components.** Generate and execute useful
   specifications through DFA, LL(1), LR(1), packrat PEG, Pratt and Earley using
   the product tool/library, not research-only entry points. Record admitted
   profiles, expected refusals, explicit selection and artifact identities.
2. **LANG-AC02 — Consistent common surface.** Implement the reviewed rule,
   operator, token, tree and diagnostic declarations with named source ranges.
   Every accepted declaration affects the artifact or checked metadata; no
   ignored annotation or literal/token-ID reinterpretation. Exercise shared CFG
   examples across LL/LR/Earley and operator examples across applicable engines;
   preserve ordered semantics separately. Covers remaining LL-D01/LL-D03 gaps.
3. **LANG-AC03 — Unicode/source policy.** Preserve scalar and byte contracts,
   malformed-input locations, NUL-as-data and original spans. Implement the
   reviewed property/identifier/comparison/display profile with pinned data
   and no dependency on generated Level G services. Independently test scalar
   boundaries, combining sequences, supplementary characters, case expansions,
   normalization differences, tabs and line endings under each admitted policy.
   Unsupported policies reject explicitly; no silent fallback to ASCII.
4. **LANG-AC04 — Trees, forests and ownership.** Preserve named captures,
   list structure, trivia/source references and agreed application-tree shape
   through lowering and backend changes. Earley exposes ambiguity/forest
   results until an explicit selection is made. Recovered nodes and edits
   retain origins. Verify independent/interleaved requests, release behavior
   and shared packed tables without per-request table copies.
5. **LANG-AC05 — Grammar-author explanation.** Report undefined/duplicate
   declarations, empty tokens, recursion/progress failures, LL and LR conflicts,
   unsupported profile features and construction exhaustion at original rules.
   Include related ranges, contextual wording and bounded witnesses where
   available; an exhausted witness search does not prove unambiguity.
6. **LANG-AC06 — Specification resynchronization.** The proper generated
   frontend collects two independent errors and retains a valid later
   declaration across the agreed quote/comment/group recovery boundaries.
   Suppress consequences of already-invalid declarations. Any error returns
   failure and preserves the last good generated artifact. L0 remains minimal.
7. **LANG-AC07 — Target diagnostics.** Present unexpected input, meaningful
   expectations, construct context and related openers consistently across
   backends. Verify LR expectations against the stack and distinguish exact
   facts from bounded hypotheses. Unify filenames, byte ranges and agreed
   display coordinates. Covers LL-D02, LL-D04 and LL-D06.
8. **LANG-AC08 — Target resynchronization.** All initial components integrate
   the common recovery contract within their admitted profiles, including
   lexical damage before parsing, nested boundaries, EOF and repeated errors.
   Test clean-input invariance, two independent errors plus later structure,
   inserted/skipped origins, bounded progress, exhaustion and no execution of
   recovered RexxScript. Do not require identical repairs across algorithms or
   claim global optimality. Covers LL-D05 together with LANG-AC06.
9. **LANG-AC09 — Declarative composition.** Implement the reviewed surface for
   token-stage pipelines, parser regions and tree passes. Exercise a layout or
   contextual-word stage, Pratt inside an outer parser, an embedded-language
   boundary and a structural tree pass. Verify source/AST projection, delimiter
   ownership, recovery containment and rejected unsafe composition. Forests
   cannot flow into a single-tree consumer without explicit resolution.
10. **LANG-AC10 — Admission and bound honesty.** Shared analysis supplies facts;
    providers own algorithm checks and implementation-specific cost claims.
    Composition additionally checks input units, contexts, progress, result
    multiplicity and boundaries. Retain negative certificate/proof cases and
    explicit unknown/resource outcomes. Keep host resource budgets outside
    grammar meaning, reconcile constructors and loaders with representation
    checks, and retain overflow-safe preflight and measured default rationale.
    Finite fuel is a work limit, not proof
    of correct recovery, successful parsing or a grammar-specific speed bound.
11. **LANG-AC11 — Bootstrap and build continuity.** Retain Level B-only required
    closure, minimal L0, L1/L2 deterministic regeneration, packed RXAS emission,
    clean/no-op/incremental/failure build behavior and Level L's re2c-free
    boundary. New property tables must be generated from pinned raw data using
    the bootstrap foundation. Existing rxc/rxas and their re2c uses remain out
    of scope. Cross-host/target limits remain explicit rather than implied.
12. **LANG-AC12 — Consumer and guide qualification.** Extend the settings,
    RexxScript and Unicode consumers plus independent backend fixtures. Run
    affected rxc/rxas/rxlink and both-VM checks, with opt/no-opt coverage of new
    semantics. Retain exact inputs and limits, reuse unchanged evidence and
    update runnable guides/capability tables. Report this checkpoint separately
    from open research criteria, full frontend migration and publication.
13. **LANG-AC13 — Simple surface and progressive learning.** Deliver one small
    complete grammar with only token/lexer/parser concepts, useful default
    tree output and located errors. Grow that same example through captures,
    expressions, recovery, Unicode policies and composition without a rewrite
    into a different notation. Exact defaults are documented and versioned;
    omitted options never trigger an undocumented semantic change. Review the
    introductory sequence with Adrian before calling the surface settled.
    Runnable before/after examples and targeted grammar-writer feedback are
    evidence; an arbitrary line-count target is not a usability proof.
14. **LANG-AC14 — Easy contextual messages.** The beginner example adds a
    bespoke rule message with a single small declaration and no host callback,
    custom tree or recovery configuration. Retain rendered filename/location,
    unexpected lexeme/EOF and context examples, plus an optional captured-name
    or related-opener example. Reject unknown template fields at generation;
    never invent an unavailable or ambiguous context value. Exercise nested
    rules, PEG alternatives/predicates, Earley alternatives and recovery to
    show speculative failures do not produce misleading final messages.
    Enabling a message changes presentation, not clean acceptance or tree shape.

1. **LANG-STEP01 (AC01–14):** record the approved portfolio, reconcile existing
   product/research contracts, and present concrete surface/Unicode/composition
   choices around a progressively extended beginner example. Portfolio recorded;
   proposal prepared; new design decisions pending
   the requested discussion. No existing acceptance criterion is closed here.
2. **LANG-STEP02 (AC02–05, AC10–11, AC13–14):** after surface review, establish the
   common versioned declaration model, source/result/diagnostic contracts and
   admission checks; repair ignored annotations and token-literal traps.
3. **LANG-STEP03 (AC01–04, AC10–11):** integrate all six components behind the
   common model, including product LL(1)/Earley and operator lowering. Qualify
   deterministic/shared CFG and ordered examples; expose forest multiplicity.
   Depends on STEP02. Delivery is not complete after any partial backend subset.
4. **LANG-STEP04 (AC03–05, AC07, AC11):** implement reviewed Unicode property,
   source-map and display behavior using Level B/bootstrap-safe inputs. Can
   proceed beside STEP03 after STEP02; no implicit normalization or G cycle.
5. **LANG-STEP05 (AC05–08, AC14):** connect easy contextual authored messages,
   structured explanations and implement
   specification recovery, then whole-pipeline target recovery for every
   admitted backend profile. Depends on STEP02 and the relevant STEP03/04
   services; preserve clean recognition and strict execution/build boundaries.
6. **LANG-STEP06 (AC02, AC04, AC08–10):** implement reviewed pipeline/region/tree
   declarations and backend-specific composition checks. Test equivalent
   unstaged cases and unsafe boundaries, including recovery ownership.
7. **LANG-STEP07 (AC01–14):** qualify backend/Unicode/recovery/composite cases,
   build lifecycle and affected consumers. Measure new aggregate scheduling
   before registering or expanding aggregate CTests, per AGENTS.md; do not add
   redundant broad or performance runs to a documentation/design checkpoint.
8. **LANG-STEP08 (AC12–14):** reconcile every criterion, update user/developer
   guides and retain review evidence. Keep incomplete criteria open and report
   exact native-host limits. Work remains on `rxlang`, with remote safety backup
   authorized above; integration and release follow the previously agreed
   end-of-work review boundary.

1. **SYNC-AC01 — Preserve and integrate.** Record the exact upstream revision,
   preserve tracked and untracked local inputs, merge it into `rxlang`, and
   inspect any overlap. Verify the unrelated roadmap edit and local source
   inventory remain present. Evidence: merge log and before/after hashes.
2. **SYNC-AC02 — Numeric repair.** Rebuild the native tools from the integrated
   sources. Run the permanent numeric/float contract regressions, the original
   RS-F01 reproducer and the 28-case compiler/RexxScript scalar comparison with
   optimized/unoptimized compilation on both VMs. No remaining optimizer
   disagreement is admitted as a pass.
3. **SYNC-AC03 — Consumer continuity.** Rebuild the Level L three-stage tool,
   verify deterministic stage agreement, and run focused generated-language,
   RexxScript and Unicode normalization consumer checks. Preserve scope limits
   and record actual source/tool identities. Update RS-F01 status only from
   these observations; do not close unrelated research criteria.

1. **SYNC-STEP01 (AC01):** fetch/pin `origin/develop`, retain a local-work
   manifest and tracked patch, integrate upstream and restore/verify changes.
2. **SYNC-STEP02 (AC02):** rebuild tools and consumer prerequisites, then run
   the focused numeric controls and scalar overlap. Depends on STEP01.
3. **SYNC-STEP03 (AC03):** execute affected consumer/stage checks, reconcile
   evidence and update the guide/plan. Depends on STEP02; report unexpected
   regressions before expanding the scope.

| Criterion | Retained result |
|---|---|
| SYNC-AC01 | The integration retained local changes against the selected upstream revision without conflicts. This historical sync result does not describe the current checkout; the reorganisation uses main develop. |
| SYNC-AC02 | Fresh native tools; both numeric contracts pass (632 tool commands, 264 VM executions). Original reproducer returns 0 in four cells. All 28 scalar cases match RexxScript and optimized/unoptimized Level G on both VMs, with zero accepted disagreements. RS-F01 closed; RS-AC04 verified for the declared profile. |
| SYNC-AC03 | Warning-free consumer build; rebuilt L1/L2 RXAS identical to each other and the prior artifact. Eight consumer CTests pass: generated language on both VMs, RexxScript's 60-case panel on both VMs, four normalization cells. Six native CLI checks pass. Logical generated normalization source unchanged. |

1. **GUIDE-AC01 — Usable guide.** Explain installation/build assumptions,
   complete language authoring, generation/linking, lexer-only and supplied-token
   use, trees/source ownership, finite token stages, limits and troubleshooting.
   Verify its worked example through rxc, rxas, rxlink and both VMs.
2. **GUIDE-AC02 — Diagnostic evidence.** Exercise malformed specifications and
   target inputs, including two independent errors and a valid later statement.
   Record actual messages, exit status and whether processing resumes; inspect
   claimed diagnostic customization against the generated artifact/runtime.
3. **GUIDE-AC03 — Honest recovery assessment.** Reconcile retained research
   measurements with their exact denominators, document current product gaps,
   and recommend a staged improvement path without implying it is implemented.

1. **GUIDE-STEP01 (AC01–02):** inspect the current frontend/runtime; add a
   small independent tutorial language and consumer, plus bounded diagnostic
   probes. Reuse valid earlier product qualification.
2. **GUIDE-STEP02 (AC01–03):** write the owning guide under `langtools/doc/`,
   link existing entry points, and retain a diagnostic/recovery assessment with
   exact evidence. Depends on STEP01; new language-design choices remain for
   review.
3. **GUIDE-STEP03 (AC01–03):** execute the tutorial and diagnostic probes,
   check documentation links/examples, and record criterion outcomes here.

| Criterion | Observable result |
|---|---|
| GUIDE-AC01 | Complete settings specification/reader/input under `langtools/examples`; rxc/rxas/rxlink and both VM runs produce the documented output. Guide covers the authoring/API/ownership/build/limit topics. |
| GUIDE-AC02 | 46 bounded command checks, including ten damaged settings inputs on both VMs, grammar failures, two independent errors, inactive custom diagnostic/tree declarations and the actual RexxScript no-execution boundary. No new recovery code was implemented. |
| GUIDE-AC03 | RECOVERY.md separates damaged-only retention from clean controls using consistent denominators. The completed one-off reconciliation script and run output were removed. The diagnostic assessment recommends staged improvements with explicit design gates and closure evidence. |

1. **GUIDE-STEP04 (GUIDE-AC01):** trace representative source through tokens,
   named tree and application handling; retain bounded inspection output.
2. **GUIDE-STEP05 (GUIDE-AC01):** add an overview before the command tutorial,
   using one running example, pipeline diagrams, the IF derivation, and a
   component-selection checklist; verify it against STEP04.

1. **BOOT-AC01 — Level B foundation.** Clean-build the required Level L core,
   seed, generator and CLI with no transitive dependency on `rxfnsg` or generated
   Level G services. Retain the actual dependency graph and build trace.
2. **BOOT-AC02 — Three-stage bootstrap.** Build L0, L1 and L2 from the
   authoritative specification, with stage-isolated outputs and checked
   dependency order. The seed has safe bounded failure and enough syntax for G.
3. **BOOT-AC03 — Regeneration agreement.** L1/L2 canonical generated
   artifacts agree, and both tools agree on a positive/negative grammar corpus,
   token/rule/tree contracts and downstream generated languages. State the
   independent correctness controls and their coverage separately.
4. **BOOT-AC04 — Unicode lexer compatibility.** Regenerate and execute the
   historical TinyExpr/gennorm2 workloads natively, preserving observable tokens,
   spans, EOF, basic error identities and semantic records. Test scalar/range
   boundaries, subtraction, multibyte characters, supplementary scalars,
   malformed UTF-8 and embedded NUL with the explicitly agreed source policy.
   Document rather than conceal any necessary compatibility adapter.
5. **BOOT-AC05 — Unicode consumer qualification.** Inventory current Unicode
   generation consumers and replace the affected retained scanner/generation
   paths. Preserve pinned input hashes, generated logical table contents and
   runtime contracts; run the owning conformance gates for changed data/tool
   inputs. Unchanged valid evidence may be reused. No new G-to-L cycle is added.
6. **BOOT-AC06 — re2c retirement boundary.** No re2c invocation, template,
   category-file or other hidden generator dependency remains in Level L or the
   migrated workload generation steps. After building the unchanged prerequisite
   C toolchain, exercise a fresh Level L generation/build with re2c unavailable
   to that phase and retain its command/input trace. Existing C-toolchain uses
   remain allowed and are explicitly outside this criterion.
7. **BOOT-AC07 — Build lifecycle and consumers.** Cold build, no-op rebuild,
   grammar edit and runtime edit produce the intended regeneration/relinking;
   failure preserves the last valid artifact and returns nonzero. Run the
   generated settings and RexxScript consumers on both VMs. Account for host
   generator versus target artifacts in supported build configurations.
8. **BOOT-AC08 — Reviewable delivery.** Update the guide/build documentation,
   record stage identities, Unicode/data provenance and retained checks, and
   report this checkpoint for review. Basic rejection and named lexical actions
   work now; rich explanation, repair and multi-error recovery remain the next
   checkpoint. Original research and any unresolved conformance obligations remain explicit. RS-F01 was subsequently closed by the develop integration recorded above.

1. **BOOT-STEP01 (AC01, AC04–06):** freeze the dependency/Unicode workload
   inventory under the confirmed retirement boundary; present the scalar/byte
   compatibility details and Level B seed contracts before implementation.
2. **BOOT-STEP02 (AC01–02):** factor the Level B-only foundation and minimal
   seed, with isolated import roots and stage outputs. Depends on STEP01.
3. **BOOT-STEP03 (AC02–04):** author Level L's lexer/parser specification and
   implement its tree-to-common-IR lowering; generate L1, then L2, and check
   agreement. Add the Unicode lexical capability required by the frozen
   workloads without Level G services. Depends on STEP02.
4. **BOOT-STEP04 (AC04–06):** migrate affected Unicode generation workloads,
   replacing Level L's re2c-generated artifacts and generator-data dependencies
   with the native tool. Preserve independent references and qualification;
   leave existing C scanners and their toolchain dependencies unchanged.
5. **BOOT-STEP05 (AC03, AC05–08):** verify the clean acyclic build, stage
   agreement, incremental/failure behavior and affected consumer conformance;
   update documentation and deliver checkpoint 1 for review.

| Criterion | Implementation / retained local evidence |
|---|---|
| BOOT-AC01 | Standalone cold build from a source snapshot containing neither Level G nor re2c; the maintained `langtools/tests/check-bootstrap-lifecycle.py` owns reproduction. Native CLI dependency inspection and product build exclude Level G. |
| BOOT-AC02 | Stage-isolated L0/L1/L2 graph; seed expression/set nesting limits and generated frontend; the frontend harness covers 17 positive/negative cases through all three stages. |
| BOOT-AC03 | L1/L2 generated RXAS are byte-identical; optimized/fully unoptimized bootstrap agreement. Five independent lexical panels also agree across all three generators, with decoded-scalar/regex oracles and both VMs. |
| BOOT-AC04 | Independent lexical oracle coverage: 2,700 cases per core configuration, four probe/VM cells; scalar boundaries/subtraction, bounded Unicode expressions, TinyExpr and gennorm2. The gennorm2 compatibility harness covers 17 owning token/typed-record/error cases in eight old/new × optimizer × VM cells. NUL correction below is explicit. |
| BOOT-AC05 | Level G normalization generation consumes the new image through the adapter; regeneration leaves the logical normalization source unchanged. Four owning Unicode-normalization conformance cells pass. Pinned Unicode 17.0.0 data/manifests remain maintained build inputs. |
| BOOT-AC06 | Seven isolated lifecycle phases with a failing re2c executable first in PATH; zero invocations, no Level G/re2c source present. Current C-toolchain dependencies are outside this proof. |
| BOOT-AC07 | Cold/no-op/grammar-edit/runtime-edit/failure/restoration phases pass; failure preserves prior generated artifacts and downstream library. Settings guide/diagnostic checks run both VMs; generated-language and 60-case RexxScript panels pass both VMs. Native host build qualified locally; cross builds reject explicitly and host/target packaging remains unqualified. |
| BOOT-AC08 | Bootstrap guide, authoring guide and diagnostic assessment updated. The current source-emission check passes 160 inputs in four cells. Historical bootstrap work is integrated; the reorganisation qualification above is local and unpublished. Rich recovery/backend selection, RS-F01 and the original open research criteria remain separate. |

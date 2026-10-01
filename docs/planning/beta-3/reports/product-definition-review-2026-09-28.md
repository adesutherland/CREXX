# Beta 3 and Release 1 product-definition review

Review date: 28 September 2026. Status: **proposal for Adrian's agreement**.
This report does not adopt new language decisions, close issues, or replace
`docs/ROADMAP.md`, `docs/release-1-plan.md`, or existing acceptance worklists.
Adrian's subsequent direction explicitly includes a **mainframe PoC/alpha
release in beta 3**. That inclusion is selected scope; the integration and
qualification recommendations below remain for agreement.
The subsequent [core-baseline proposal](../core-baseline-2026-09-28.md) records
Adrian's selected next step: architectural reconciliation and cREXX integration,
a metadata-only SOURCELINE feasibility/implementation attempt and the sanitizer
workflow repair, before product definition/catalogue/archive decisions. Its
architecture was approved by Adrian on 28 September. Adrian permits unretained comments to be
blank and explicitly leaves #602 open after beta 3 if reconstruction is infeasible
or not sensible; this supersedes the earlier automatic transfer proposal below.

## Intended outcome

Make beta 3 the publication of an explicitly defined product, close the
outstanding product-definition questions, and establish a complete Release 1
baseline that can subsequently change through recorded decisions. Cover **all
ten open GitHub issues**, including the three outside the beta 3 milestone.
An issue closed because its definition is accepted or its work is transferred
must remain distinguishable from an implementation completed and tested.

Recommendation: complete the definition, documentation, mainframe integration,
package and assurance work below before beta 3. Keep substantial new language implementation on the
already selected later milestones. Retain every unfinished Release 1 outcome
in the authoritative plan with acceptance criteria and ownership.

## Evidence baseline

- Remote `develop`: `143921e11e4d573909fcc4def28da5dceadba9d9`.
- Last product-changing revision: `02d1fc3812628706741e5e084eec0a3bed6615d7`.
  The intervening change is only `docs/planning/issues-704-707-20260927.md`.
- Local checkout: `47168a1f16365d6c2aaa54de770889c0e8dce6a4`, 39 commits behind,
  with pre-existing source, documentation and planning edits. None was merged,
  reset or edited by this review.
- Beta 2 remains the latest versioned beta tag/release. Beta 3 has neither a
  tag nor versioned release assets. The live milestone is due 30 September.
- GitHub has **10 open issues**, **7 in beta 3**, and **2 open pull requests**.
  All ten issue bodies and their comments were reviewed. The milestone has
  fourteen closed issues; the original candidate lists are historical input,
  not an additional uncompleted feature list.
- The local roadmap/release plan contain Adrian's recorded 18 September
  core-complete / functionally-complete / Performance Beta sequence. Published
  `develop` still describes the older, narrower sequence. Reconciliation must
  preserve the later local decisions and the intervening published work.
- Sister project `mainframe-lab`: clean published `main` at
  `1742740df60c0858421f196993e55da3edcb681b`. Its 28 September
  `docs/BETA3-MAINFRAME-PLAN.md` supersedes older delivery priorities for this
  cut. Lab source publication and earlier package tests do not establish a
  final cREXX beta 3 release. The mainframe findings below were checked against
  the current lab plans, patch exports and published cREXX source.

## Proposed definition to lock for beta 3

Beta 3 ships the accumulated Level B toolchain and libraries, RXPP, the current
standalone/embedded RexxScript product, the bounded compiled Level C proofs,
the implemented initial G concurrency/HTTP and explicit Unicode services,
accepted performance changes, and deliberately classified integrations.
It includes the recent driver locking, RXPP, vector, JSON and SDK work.
It also includes the selected **CMS31, TSO31 and TSO64 PoC/alpha packages**,
with their own supported-capability manifest and native qualification.

The definition should explicitly state:

1. **RXPP is a separate, visible toolchain stage.** The driver invokes it for
   `.rxpp`; ordinary `rxc` compilation does not secretly expand macros.
   Generated source can be inspected. Source maps, include origins and macro
   argument spans have documented ownership. Script-generated lines retain
   invocation-level provenance; token-level script provenance is not promised.
2. **RexxScript is a separate interpreted product.** Ship its standalone runner,
   embedded command and evaluator API, isolated variable pools, captured output,
   explicit host-variable exposure, documented control flow and intrinsic set.
   Existing file/stream intrinsics are part of the actual surface: “sandbox”
   must not imply filesystem denial or complete Classic Rexx semantics.
   No general external calls, ADDRESS/shell execution, or nested INTERPRET is
   added for beta 3. Record current diagnostic limitations explicitly.
3. **Level B libraries use typed, explicit contracts.** Document ownership,
   mutation, lifetime, errors and interface use. Keep Classic value/pool/BIF
   compatibility in its appropriate runtime layer. Preserve native provider
   factories instead of introducing duplicate Rexx wrappers. No broad rename,
   removal or source-directory reorganisation is necessary for this cut.
4. **Component support and delivery are explicit.** Adopt the categories below,
   refresh the existing catalogue, and match actual build/install/package
   selection to the resulting beta 3 manifest.
5. **Level C is bounded in beta 3.** Publish the executable proofs with their
   unsupported boundaries. Full compatibility remains the November outcome;
   it is not represented as complete by the beta 3 demos.
6. **Native inference is an optional, separately qualified integration.** Core
   users do not acquire a mandatory model or llama dependency. Package smoke
   success does not establish all real-model/device, memory and drain criteria.
   Beta 3 states precisely which cells have evidence; the parent requirements
   remain open where unmet.
7. **Mainframe delivery is explicitly PoC/alpha.** Ship the three selected
   profiles using the actual beta candidate and matching libraries. Publish
   their supported tools, services, memory requirements and omissions. Preserve
   normal namespace/import behavior, 64-bit Rexx integers and RXBIN portability.
   The alpha label does not replace native installation and execution checks.

### Proposed component policy

| Category | Product promise | Build/package and evidence rule |
| --- | --- | --- |
| Core | Required language/runtime/toolchain capability | Required in its supported profile; documented and tested on that profile's supported platforms. |
| Standard | Supported ordinary library/tooling capability | Included by default; not automatically part of the minimal bootstrap closure. |
| Optional integration | Useful capability with extra dependencies | Explicit install/build selection; dependency, lifecycle, licence and platform status documented; its absence does not break core startup. |
| Experimental/example | Research or demonstration | Clearly labelled, separately selected or shipped as example material; no implied stable API. |
| Deprecated | Retained transition surface | Replacement and compatibility policy recorded; no new dependencies on it. |
| Remove | No retained product purpose | Remove only with an explicit item-level decision and any needed migration note. |

Implementation language is independent of category. A native provider can be
standard; a Rexx-authored library can be an optional integration. Preserve the
accepted RCC provider split, declarative dependency loading and native-package
archives. The existing component catalogue is a substantial starting point,
but its provisional classifications and old counts cannot simply be approved
as if they describe September's product. Refresh at least Unicode, HTTP,
concurrency, SQLite/vector, llama, RXPP and current SDK/package contents.

## All open issues: proposed closure disposition

Owners below are the current GitHub assignees, not new assignments.

| Issue | Current evidence and remaining work | Proposed closure condition |
| --- | --- | --- |
| [#610 — preprocessor role](https://github.com/adesutherland/CREXX/issues/610), Adrian | RXPP is already a root component, visible driver stage, installed tool and source-map producer. Current guide and direct/editor/source-map tests exist. The original design note still says working/unapproved. | Accept the existing architectural role and documented mapping limits; reconcile the design note and attach current evidence. No new macro language is required. |
| [#612 — RexxScript strategy](https://github.com/adesutherland/CREXX/issues/612), Adrian | Standalone/embedded implementation and master user/developer guides exist. The issue still asks for a first slice. | Approve the bounded product above, exact shared-BIF and source-diagnostic expectations, and one standalone plus one embedded checked example. Close the definition issue; retain any broader Release 1 work by named acceptance row. |
| [#616 — class-library principles](https://github.com/adesutherland/CREXX/issues/616), Adrian | Catalogue, typed class/interface contract and provider refactors supply evidence, but do not by themselves constitute approval of the requested policy. | Approve principles plus a current keep/deprecate/remove/rename/move list. Give every remaining implementation item a named disposition; retain compatibility unless a specific change is accepted. |
| [#617 — plugin policy](https://github.com/adesutherland/CREXX/issues/617), Peter | Existing catalogue has role/delivery/composition axes; much remains expressly provisional. | Approve category definitions and their build, package, documentation and test obligations, including standard/default versus bootstrap core. |
| [#622 — plugin/exit inventory](https://github.com/adesutherland/CREXX/issues/622), Peter | A detailed component catalogue already exists. Some rows are outdated; RXPA and library work has substantially changed the tree. | Refresh that catalogue, map every shipped integration/exit to the approved policy, and record dependencies/options/package/tests and migration notes. Reuse the catalogue instead of starting a second inventory. |
| [#624 — package validation](https://github.com/adesutherland/CREXX/issues/624), Rene | Four core platform outputs, matching optional llama packages and three exact-revision SDKs are in the current snapshot. Final beta 3 assets do not yet exist. | Verify the selected beta 3 assets, identities/checksums, signatures, clean install/use/reinstall/removal and fallback status. Close only against the actual release assets and matching docs. |
| [#625 — demos/tutorials](https://github.com/adesutherland/CREXX/issues/625), Rene | Numerous implementation fixtures and examples exist; the issue still lacks the final curated user-facing result. | Select a small named set, retain commands/expected output/known limits, and verify it against the candidate packages. Include B, RXPP, standalone/embedded RexxScript, initial G, bounded C and L generated-output examples; a provider demo remains optional. |
| [#663 — RXPP macros](https://github.com/adesutherland/CREXX/issues/663), unassigned | The issue itself reports implementation; named-section examples, macro/runtime docs and focused regressions exist. | Verify the documented `.gen`, `.tail`, `.section` and `##emit` scenarios and installed invocation; reconcile the promised guide and close as delivered if its acceptance passes. Do not treat it as a new feature project. |
| [#665 — data queues](https://github.com/adesutherland/CREXX/issues/665), Peter | Comments and current provider documentation describe execution-local named queues, including RXQUEUE management. The language-reference chapter still incorrectly says named queues are unsupported. | Reconcile implementation/tests and docs, then close the delivered execution-local queue scope. External cross-process queue services are a separate, explicitly excluded product capability. This does not prove complete compiled Level C lowering. |
| [#602 — SOURCELINE](https://github.com/adesutherland/CREXX/issues/602), unassigned | This remains a real stateful BIF gap. The Classic BIF reference requires retained visible source lines and execution-context plumbing; `.srcstep` alone is insufficient. | Recommend including it explicitly in Beta 4's complete Level C acceptance matrix. For the requested zero-issue checkpoint, close as an approved transfer only after that durable row, owner and tests are recorded. It remains unimplemented work. If every closure must instead mean implemented, this feature must be built and qualified before the checkpoint. |

The proposed zero-open-issue outcome therefore requires nine evidence/definition
closures and one explicit decision about #602. No issue was closed during this
review. Future defects and execution tasks can still be opened; the zero count
is a reconciled baseline, not a prohibition on reporting problems.
The mainframe integration work below is additional work outside that ten-issue
list. It needs named ownership and acceptance links in the release plan before
the issue count can be used as a useful completeness check. Link its package
and example evidence to #624/#625 as well as the existing lab criteria.

## Mainframe PoC/alpha: selected beta 3 deliverable

The authoritative lab scope is
[BETA3-MAINFRAME-PLAN.md](/Users/adrian/CLionProjects/mainframe-lab/docs/BETA3-MAINFRAME-PLAN.md),
supported by its
[delivery plan](/Users/adrian/CLionProjects/mainframe-lab/docs/MAINFRAME-DELIVERY-PLAN.md)
and
[manual qualification procedure](/Users/adrian/CLionProjects/mainframe-lab/docs/operator/BETA3-MANUAL-QUALIFICATION.md).
Retain its B3-01–06 IDs instead of creating a second mainframe acceptance plan.

| Package | Beta 3 commitment | Evidence still needed |
| --- | --- | --- |
| CMS31 | AMODE31, relocatable RMODE ANY; native CMS 20 on z/VM 4.4, with declared service limits. | Current candidate and matching library/package hashes; affected native application checks and manual installation/execution. Earlier CMS31 package `0017` passed the bounded full-library/compiler-exit matrix. |
| TSO31 | AMODE31, RMODE31/ANY; main image above 16 MiB on z/OS 1.5. | Final candidate/package qualification. The lab has accepted Linux-built `0017` above-line relocation and RXC/RXAS/RXVM, full-library PARSE/exits, import, I/O/error/repeat evidence; it is a bounded local result, not a new hosted beta release. |
| TSO64 | AMODE64 C with 64-bit pointers, RMODE31/ANY code below 2 GiB, high data and explicit low service buffers on z/OS 1.5. | Complete the current Linux delivery package and qualify the actual cREXX applications. The new shared C/newlib reference passes native AMODE64/problem-state and identical XMIT-restored execution; earlier cREXX LP64 tests used a different RMODE24 package. Neither closes the final beta package gate. |

Software floating point and 64-bit Rexx integers remain part of the selected
contract. CMS64, code above 2 GiB/RMODE64, the broader large-memory/high-code
MD-04–06 programme and general production mainframe support are outside this
beta cut. Their existing worklist outcomes remain open. CMS24 and MVS24 are
not among the three selected beta packages.

The alpha manifest must enumerate native RXC/RXAS/RXVM, matching bytecode
libraries and the supported compiler exits, source/binary imports, text/binary
I/O, errors and repeat use. Other tools such as RXLINK, the driver and RXPP
need an explicit delivered/host-assisted/unavailable status; desktop packaging
must not imply that every tool or library is available natively. General native
host exits, OS ADDRESS routing, workers/channels, sockets, dynamic extensions
and a mainframe decimal-maths plugin remain separately scoped capabilities,
not promises introduced by this release. Retain the measured heap/region and
file-service limits for each profile.

### cREXX changes already made

The earlier CMS/platform foundation is already in published cREXX `develop`.
Ancestry checks confirmed the explicit CMS platform and C99 adaptations,
sequential stream reading, environment/process separation, opt-in single-thread
profile, smaller/configurable pools, embedded VM/compiler-exit support,
directory snapshots, CMS20 and native text hooks, source-close repair, and
the later embedded-directive handler repair. Representative upstream revisions
are `e05066b01`, `5bbf84049`, `bd96ddd56`, `e9f003cfd`, `b7afdf985`,
`20e3de226`, `e68681cd1`, `edc4f3378` and `04ea56d1c`.

Older lab patch README statements that all of this is unpublished are stale.
Do not replay the complete historical patch series onto current cREXX or lose
the later handler repair by rebuilding an old base.

### cREXX changes still requiring integration or disposition

The active Linux pipeline applies five visible patches in
[linux-topics.series](/Users/adrian/CLionProjects/mainframe-lab/patches/crexx-cms-release-poc/linux-topics.series).
Their changes remain absent from the inspected published cREXX revision.
The older TSO register exposes additional candidate areas; these overlap with
newer platform work and require reconciliation, not automatic replay.

| Candidate | Purpose and current status | Recommended beta 3 treatment |
| --- | --- | --- |
| Linux topic `0013`: compressor workspace | Moves the large automatic RXBIN compression hash table to checked per-call heap storage. Current cREXX still has the automatic table. The lab retains bounded byte-equivalence, round-trip and failure-injection evidence. | Integrate after focused review, retaining output-format/algorithm behavior and permanent allocation/cleanup regressions. This is constrained-stack correctness work. |
| Linux topic `0014`: platform discovery | Supplies native directory/member enumeration through the platform iterator. The compiler retains shared provider/header matching. Current cREXX lacks `CREXX_PORT_DIRECTORY`. | Integrate the current platform route and verify differently named providers, mixed extensions and all matching members. Never reinstate the rejected filename-equals-namespace shortcut. |
| Linux topic `0015`: RXBIN import diagnostics | Reports binary-import read failures instead of losing the reader's diagnostic. The added compiler diagnostic path is absent upstream. | Integrate with invalid/truncated/incompatible import failure tests and a successful following invocation. |
| Linux topic `0016`: unresolved-procedure sentinel | Handles the portable no-address value when loading RXBIN on a 32-bit host. Current reader still rejects the large sentinel as an overflowing address. | Reconcile with older TSO topic `0011`, which also covers canonical writing, historical ILP32 imports, docs and a focused format regression. Produce one complete repair, preserving real-address overflow rejection and 64-bit Rexx values. |
| Linux topic `0017`: namespace/root precedence | Keeps the first source root for a namespace while allowing multiple providers within that root. Current cREXX lacks the new resolver check and its regression. | Review against the shared import contract, integrate and retain the supplied root-order regression on native and desktop profiles. |
| Older TSO topic `0010`: explicit platform services | Explicit TSO selection, file/member service hooks and allocation-safe integer-only OOM diagnostics; the TSO-specific surface is absent from current cREXX. | Reconcile the required pieces with the newer common discovery/platform route and current runtime. Document every required platform delta in the final source manifest. Do not retain accidental Linux service assumptions. |
| Older TSO topic `0012`: compiler allocation checks | Guards `sym_fn` symbol/name allocation after a reproduced bounded-heap failure. Current cREXX still dereferences these unchecked allocations. | Carry the bounded repair and permanent failure regression into the integration batch; preserve the existing panic contract and prove controlled status plus subsequent usable invocation. |
| Older TSO topic `0013`: VM value factory allocation checks | Guards worker-owned and standalone factories before value initialization. Current cREXX still calls initialization after unchecked allocation. The lab records a failing base and passing fault-injected candidate. | Reconcile and integrate both paths with focused regressions. This is separate from the compressor patch bearing the same topic number in the other directory. |

The two patch registers are
[Linux/CMS continuation](/Users/adrian/CLionProjects/mainframe-lab/patches/crexx-cms-release-poc/README.md)
and [earlier TSO candidates](/Users/adrian/CLionProjects/mainframe-lab/patches/crexx/README.md).
This review establishes the integration gap; it does not approve the exported
patches unchanged or claim that their old bases qualify current cREXX.
The source must be rebased/reconciled, reviewed under cREXX's rules and covered
by focused regressions plus normal product correctness checks. Retain useful
lab receipts, with their source and artifact identities intact.

The lab also records a compiler-exit load failure that can produce a success
status in the capacity-limited CMS24 configuration. Check its current shared
compiler behavior and give it an explicit disposition; excluding CMS24 does
not by itself establish that the error path is irrelevant to the selected
profiles. Broad naming/text-conversion consolidation and new host capabilities
can remain later work, while concrete errors affecting the selected alpha
contract must be accounted for before delivery.

Native entry, relocation, low-buffer/service bridges, newlib runtime packaging,
compiler/bootstrap assets and PDOS changes belong to the sister project's
delivery work. They must appear in the package provenance but are not all
cREXX source patches. The reviewed Linux pipeline increment did not require a
new generic-newlib core change.

### Qualification and release ownership

1. **B3-01:** freeze the actual beta cREXX revision, library bytes, runtime/native
   sources and any visible deltas; hash each package and executable. Name the
   cREXX integration owner and lab package/native-test owner in the release
   plan. Current issue assignees alone do not cover this cross-project work.
2. **B3-02/03:** execute the changed CMS31, TSO31 and TSO64 application packages
   on their named native guests. Cover compiler/assembler/VM use, full selected
   compiler exits, imports with independent filenames and ordered roots,
   text/binary round trips, negative status, cleanup and repeat invocation.
   For TSO64 retain actual entry mode, problem state, LP64/high-data and
   low-service-buffer evidence. A small C/newlib pass is insufficient.
3. **B3-04:** retain the accepted shared 31/64-bit console/file/memory reference
   and identical XMIT-restored results. Reuse them while their inputs are
   unchanged; do not relabel them as cREXX application results.
4. **B3-06:** complete the lab's manual installation and execution gate for the
   exact distributed binaries on z/OS and z/VM, with operator, guest, commands,
   outputs/status and hashes. Blank templates, package-only builds and automated
   smoke cannot satisfy this existing gate. Provide installation instructions,
   notices, capability limits and a checked alpha example beside the artifacts.
5. **B3-05:** report PDOS binary compatibility independently. The selected goal
   is execution of identical TSO binaries in problem state, without a PDOS
   relink or translated payload. Current work remains incomplete; even a small
   reference pass would not qualify the full RXC/RXAS/RXVM release binaries.
   The lab plan explicitly keeps this from becoming a new gate for otherwise
   native-qualified beta packages.

This adds a real integration and package dependency to beta 3. The 30 September
target has not been demonstrated achievable by this review. The decision at
the cut must use completed native package evidence or an explicit scope/date
revision; older PoCs cannot be renamed beta 3 to satisfy the date.

## Proposed Release 1 definition

Retain the locally recorded dates: **Beta 4 core complete 30 November 2026;
Beta 5 platform functionally complete 31 January 2027; Beta 6 Performance Beta
31 March; RC1 15 April; Release 1 1 May 2027.** The dates are planning targets,
not an estimate demonstrated by this review.

The following closes the major product-definition gaps as a concrete proposal.
New selections are marked as such and need Adrian's agreement before adoption.
Detailed syntax and implementation designs still follow normal repository
review; product definition does not choose an unreviewed mechanism.

| Product area | Proposed Release 1 commitment | Boundary / acceptance |
| --- | --- | --- |
| Level B and toolchain | Stable documented typed language, core libraries, compiler/assembler/linker/VMs, driver, RXPP, native packaging and SDK. | Existing data, Unicode/binary, ownership, signal, import, linking and provider contracts are the starting point. Complete positive/negative and installed-consumer coverage; no incidental compatibility cleanup. |
| Compiled Level C | Preserve the already selected complete Classic compatibility target, including control flow, PARSE, procedures, stems/pools, numeric context, conditions/signals, BIFs, source/TRACE and configured host services. | November. Construct the exhaustive matrix from the compliance and BIF references. Every unsupported required row stays open. Only individually approved “won't implement” exceptions count as exclusions. SOURCELINE is explicitly required in this proposal. |
| Polymorphism — new scope proposal | Complete interface-based substitutability across assignment, arguments, return values, collections, factories, casts and module/late-load boundaries; multiple interface implementation and existing default/final methods. | November. Current docs say interface-to-interface assignment should not be relied on: this proposal requires a precise usable contract and its implementation. Propose excluding inheritance, overloads, generics, singleton declarations and new destructor syntax from R1; class/interface constants retain their R2 disposition. These exclusions require agreement and do not claim the current implementation completes polymorphism. |
| RexxScript | Supported small standalone/embedded interpreted product using its documented string-first surface and shared Classic helpers, with explicit variable isolation, stream capabilities, errors and source provenance. | November stabilization. Positive/negative, evaluator-isolation and CLI/embedded tests. No promise of full Classic equivalence, general external CALL, ADDRESS or nested INTERPRET. |
| Level G standard platform — proposed explicit list | Structured tasks/processes and typed transfer, HTTP/TLS/streaming, Unicode 17 normalization/casing/folding/graphemes/codecs, JSON, hashing, math/statistics and ordinary collection/library services. Database/vector/LLM integrations have explicit optional delivery roles. | January. Supported capability/version/platform matrix, ownership/cancellation/error tests, examples and clean packages. Preserve exact ordinary string equality and explicit rich-Unicode operations. No shared writable VM state, detached tasks, actor/event framework or cross-host provider ABI is added by this definition. |
| G ownership/containers — new scope proposal | Usable owned nested and heterogeneous lists/maps, with explicit construction, transfer, mutation, iteration and destruction; preserve typed-array fast paths and weak-reference lifetime rules. | January. A nested-data/graph consumer must be expressible without accidental dangling references or undocumented ownership. Prefer an explicit owner-based library contract; no generic syntax or garbage-collection redesign is presumed. |
| Level L — new scope proposal | A reusable lexical-scanning and parsing toolkit with one supported grammar-to-cREXX generation route and runtime, source-position diagnostics and installed usage. | January. Generate, compile, link and execute at least an expression language and a structured configuration parser. TinyExpr alone is the existing generated-output proof, not the whole deliverable. Choice of generator technology needs its own design decision; broad L syntax sugar is excluded. |
| Pipes — new inclusion proposal | Include the bounded synchronous stage interface/reference executor, immutable PipePlan and named ports as the initial contribution surface. | January if this inclusion is agreed. Deterministic pipeline results, errors, resource teardown and an independently authored stage. Asynchronous/distributed Pipes is excluded. Until agreement this remains a candidate under the existing roadmap. |
| Common LLM and native inference — proposed release allocation | Common provider API and optional llama.rexx integration belong in the R1 product, with the full existing parent acceptance criteria preserved. | Allocate required platform functionality/qualification to January. Retain ARM Mac CPU/Metal, Intel Mac CPU-only delivery, and Windows/Linux CPU, NVIDIA CUDA and representative AMD/Intel Vulkan requirements. Real-device/model, sharing, memory, failure/drain and provenance evidence remain required. Package smoke does not close them; unavailable hardware leaves an open row or needs an explicit scope revision. |
| Portability — selected beta 3 alpha; proposed R1 support boundary | Supported desktop binary product: Linux x64, Windows x64, macOS arm64 and x86_64. Beta 3 includes the selected CMS31/TSO31/TSO64 PoC/alpha packages and their bounded native contract above. | Propose retaining an explicit experimental mainframe support tier through R1 unless separately promoted on evidence. This does not promise desktop capability parity, CMS64, RMODE64 or complete PDOS compatibility. Preserve B3/MD and PORT-C89 worklist outcomes; qualify the actual published alpha packages. |
| Performance | Qualify the complete platform and accept measured improvements during the selected Performance Beta. | March; no mandatory memoization, quickening, fusion or JIT mechanism. Preserve first Release verdicts and accepted/rejected evidence. Freeze optimization implementation for RC1. |
| Applications, tools and demonstrations | A named curated asset set built against the platform, with optional dependencies explicit; distinguish bundled assets from independent projects. | Through RC1. Each selected asset has an owner, useful scenario, setup and expected result. A possible application never silently expands core release requirements. |
| Distribution | Versioned core packages and exact-source SDKs; separately versioned/matched optional integrations; checksums, licences, support matrix, install/uninstall and migration documentation. | RC1/release. ZIP fallback on every supported platform; signed Windows and signed/notarized Mac installers only where verified. Linux deb must pass its explicit lifecycle/dependency checks. |

The current R1-AC-01 through R1-AC-06 remain the authoritative acceptance IDs.
On agreement, add the product rows and their detailed inventories under those
criteria in `docs/release-1-plan.md`; do not create a competing roadmap. The
complete Level C inventory and selected polymorphism/G/L capability lists are
definition work to complete now, even though their implementations land later.
This review has not estimated the full November/January implementation load.

## Remaining work before the beta 3 cut

1. **Approve and publish the definition baseline.** Reconcile the newer local
   release sequence with current remote product evidence. Refresh release notes,
   support categories and issue acceptance links. Preserve local work while
   integrating; the dirty, older checkout is not a release candidate.
2. **Finish the nine evidence/definition closures and decide #602.** Refresh the
   existing component catalogue and only fix the demonstrated documentation,
   packaging or behavior discrepancies it exposes. Curate and check examples.
3. **Integrate and deliver the mainframe PoC/alpha.** Reconcile the five active
   Linux-pipeline cREXX patches and three older TSO candidate areas above.
   Preserve normal compiler/VM semantics and add the permanent focused
   regressions. Complete current CMS31/TSO31/TSO64 packages, supported-capability
   manifests and the lab's B3-01/02/03/06 native/manual gates. Link this work to
   the release plan and #624/#625; it cannot disappear when the issue count is
   reduced to zero. Keep PDOS B3-05 as a separately reported workstream.
4. **Make scheduled sanitizer assurance able to finish.** The 28 September run
   [36375692027](https://github.com/adesutherland/CREXX/actions/runs/36375692027)
   checked out `143921e11`, but its scheduled workflow comes from `master` and
   still has a 120-minute job backstop. `develop` already carries 240 minutes.
   Retained Linux/macOS logs contain 2,344/2,342 completed passing tests out of
   2,365, respectively, and no failed tests or ASan/LSan diagnostics before
   cancellation while `crexx_project_build_contract` was active. Reconcile the
   default-branch workflow and obtain terminal candidate coverage; investigate
   any subsequently reproduced failure narrowly. This is currently incomplete
   assurance with concrete workflow drift, not a newly established memory bug.
5. **Dispose of the historical Linux performance QA-C obligation explicitly.**
   `performance/PERFORMANCE-CLOSEOUT-PLAN.md` still requires the frozen
   `81f15918676d92a7d3e88954d94779ed759e9db8` counterpart. Recommend retaining
   that as historical incomplete evidence and making the exact beta 3 candidate
   the release acceptance target, if Adrian approves that change. Otherwise
   complete the existing obligation. A newer run cannot silently satisfy it.
   This is distinct from concurrency's Linux closeout, which already delegates
   final-head authority to the hosted Build/Sanitizer workflows.
6. **Complete the selected asset/signing matrix and tag delivery.** The current
   moving snapshot has unsigned Windows setup assets and no separately named
   signed Windows derivatives. CI-F21's current-snapshot signing/publication
   criteria remain open in the inspected plan. Establish the actual signatures
   and matching binaries for the chosen release payload; retained signed older
   artifacts are not proof for a new payload. Preserve the approved ZIP fallback.

The selected mainframe alpha is beta 3 implementation and delivery work.
No additional broad optimization programme, Level C completion, generator
programme or ownership redesign is recommended for this cut. Their product
definitions should nevertheless be decided before this baseline is closed.

## Evidence already available; do not repeat without cause

- Product revision `02d1fc381` passed
  [Build CREXX 36339434928](https://github.com/adesutherland/CREXX/actions/runs/36339434928)
  and [CodeQL 36339434773](https://github.com/adesutherland/CREXX/actions/runs/36339434773).
  The newer `143921e11` delta is documentation only.
- [Deep Build QA 36371260666](https://github.com/adesutherland/CREXX/actions/runs/36371260666)
  passed on 28 September. Its gate log resolves the actual candidate to
  `143921e11`, despite the workflow metadata showing the `master` dispatch SHA.
- [Downloaded SDK RAG QA 36342388981](https://github.com/adesutherland/CREXX/actions/runs/36342388981)
  passed Windows x64/macOS arm64/macOS x86_64 using published CREXX product
  `02d1fc381`. QA workflow revision `29c5c1f46` is not the product revision.
- Issues #701 and #704–#707 are closed. The latter batch's plan records driver
  guard lifetime, clean-up, SDK and RXPP regressions and platform receipts.
- The sanitizer register has no currently open SAN finding. SAN-009 is closed;
  native-inference device qualification is a separate open obligation.
- The last inspected complete scheduled sanitizer pass was
  [36092018540](https://github.com/adesutherland/CREXX/actions/runs/36092018540),
  targeting `88b64b150` on 25 September. It is historical coverage, not an
  exact-current-candidate pass.
- KeyAccess decisions were accepted on 18 September. RXVECTOR-02 and the JSON
  accessor repair have published evidence; old local “unpublished” or
  “acceptance pending” paragraphs need reconciliation, not renewed development.

## Backlog and pull-request housekeeping

The July Level B improvement backlog, beta 3 candidate inventory, performance
idea queue and component-catalogue snapshots need a status map into the selected
R1 product rows. Helper inlining, indexed access, BIF tuning and similar older
ideas must be checked against completed performance work before reopening.
They are not automatically beta 3 requirements because their files live under
`planning/beta-3`.

[PR #703](https://github.com/adesutherland/CREXX/pull/703) proposes a cast for
the already repaired #702 shell-name issue. Current `develop` preserves const
through lookup instead. Recommend reconciling the PR as superseded rather than
merging the old implementation. [PR #662](https://github.com/adesutherland/CREXX/pull/662)
is the still-applicable `tiem` to `time` comment correction in `S370/cms.h`;
it can be handled as a separate trivial contribution. Neither was changed here.

## Recommended agreement

Approve beta 3 as the defined current product above; adopt a revision-controlled
R1 baseline with the explicit polymorphism, G/L, Pipes, native-inference and
platform decisions; close the delivered/definition issues against evidence;
transfer #602 only with its required Beta 4 acceptance preserved; and complete
the selected mainframe alpha integration/native gates and actual beta 3
assurance/package gates. Record later changes against the
affected capability, milestone and acceptance criterion.

Primary records reviewed include `docs/ROADMAP.md`, `docs/release-1-plan.md`,
beta 3 release notes and issue candidates, the Level B backlog, component
catalogue and RCC overlays, performance roadmap/closeout, concurrency closeout,
RXPP and RexxScript guides, the class/interface and queue references, Unicode
product baseline, native-inference parent/pipeline plans, sanitizer register,
current tags/assets, issues/comments, PR diffs, workflow jobs and retained logs.
The extension also reviewed the sister project's current beta/delivery plans,
coordinator handover, native qualification reports, manual release procedure,
active Linux source/patch pipeline and older TSO patch register, checking their
upstream claims against published cREXX source and commit ancestry.
This was an evidence review; no build/test workflow was dispatched and no
source behavior was changed.

# CREXX sanitizer assurance worklist

This is the canonical live register for first-party findings from maintained
sanitizer configurations.  Repository policy and closure requirements are in
[`AGENTS.md`](../AGENTS.md) and
[`CREXX_ASAN_TESTING.md`](ai-context/CREXX_ASAN_TESTING.md).

## Sanitizer scope review and proposal — 20 October 2026

**Status: Adrian approved implementation and qualification on 20 October 2026.**
The approved order is unchanged isolated Linux timeout diagnosis first, then the
exact ten-cell daily reduction, separated fixture/native preparation and shared
Classic preparation, qualified before activation. The existing scheduled policy
remains effective until qualification and the separate master-workflow decision.
See [measured results](#bounded-measurement-results) and the
[exact membership proposal](#exact-membership-approved-for-implementation).
Implementation uses the existing synchronized hotfix checkout. Focused hosted
qualification is authorized; no master activation or release promotion is
authorized. The local Linux ARM64 VM has no configured usable SSH identity, so a
dispatch-only Linux x64 replay route preserves the existing default/full route
and checks out the original failed source commit for the first replay.

**20 October follow-up:** Adrian requested a wider test-strategy review for
the direction in which tools/libraries are implemented in cREXX, native C
supplies the bootstrap, and the VM/plugins remain native. The revised proposal
below replaces the initial emphasis on selecting a smaller daily subset of
the same whole-toolchain job. The initial daily/weekly suggestion was not
approved. Architecture, coverage and test purpose must determine the new
profiles before cadence or test-count reductions are implemented.

### Vision and intended outcomes

Qualify the native execution foundation deeply and test cREXX tools for their
own semantic contracts without making every language case a repeated native
memory-safety campaign. A healthy result means correct compiler/tool output,
correct and memory-safe VM/plugin execution for the tested boundaries, bounded
long-lived resource use, and trustworthy platform/packaging integration.
These are separate observable obligations.

Preserve independent language oracles, emitted-code quality and full ordinary
correctness testing. Concentrate sanitizer effort on both concrete VMs, loader
and native support, first-party plugins, bootstrap/native tools while they
remain in use, and selected cREXX workloads exercising those components.
Preserve all applicable permanent SAN regressions and existing broad closure
gates pending explicit approval of any replacement. No native tool is retired
by this review. Resolve the current failure rather than excluding it.

### SAN-QA-020 — Linux Unicode normalization exhausts its CTest deadline

Status: **isolated Linux diagnosis verified; scheduling repair and broad proof open**.
Owner: Codex under Adrian's approved implementation/qualification task.
This is a QA finding; no new memory-safety finding was observed.

- Reviewed [Sanitizer QA 39366632770](https://github.com/adesutherland/CREXX/actions/runs/39366632770).
  Both jobs checked out `e5c066b87945dc697df6c771fc5a56a20962c772`, matching
  local and remote hotfix. The workflow metadata names master
  `77ba820c35e40450311823a3e7f171e37a7982f8`; that is the scheduled workflow
  definition, not the tested source. Remote develop had subsequently advanced
  to `875a84b75d9a2b91e4a459d5379e57df4f92b15f` during review.
- Linux failed `ts_unicode_normalization_rxtvm_noopt` at **1,200.03 seconds**
  and `ts_unicode_normalization_rxtvm_opt` at **1,200.02 seconds**.
  The corresponding `rxbvm` cases passed at 904.10 and 908.25 seconds.
  CTest completed 2,661 of 3,289 selected tests: 2,659 passed, two timed out,
  and **628 did not run**. Build and preparation passed.
- macOS passed **3,294/3,294**, including all four normalization cases
  (378.08–391.85 seconds) and all 16 structured-flow cases. Linux reached
  none of the serial structured-flow cases before stopping: SAN-QA-019's
  Linux confirmation remains pending; this run does not close it.
- Inspected the complete build, preparation and CTest logs for both platforms.
  There was no ASan/LSan diagnostic, reported memory error, or other CTest
  failure. This establishes the observed failure type, not a Linux clean pass.
- Original invocation: `tools/asan-run.sh --build-dir <fresh-tree>
  --phase full --build-jobs 4 --test-jobs 8
  --exclude-label '^performance-measurement$' --no-live-tail`, with
  `--build-leaks on --leaks on` on Linux and both off on Apple.
- The failed revision had `TIMEOUT 1200` without `RUN_SERIAL`.
  Its unchanged fixture checks 20,034 official rows, 400,680 transformation
  relations, 400,680 predicates and 1,094,978 unlisted scalar identities per
  form, in each of four optimizer/VM combinations. Contention is a credible
  explanation, supported by the other long-running Unicode cases, but a
  product slowdown or hang has not been ruled out.
- Approved diagnostic action, now completed below: prepare the exact failing inputs, replay the two
  unchanged cases once under isolated normal Linux Debug and maintained Linux
  ASan/LSan scheduling through the runner, and inspect any diagnostic before
  deciding the repair. Proposed selection:
  `--phase ctest --regex '^ts_unicode_normalization_rxtvm_(noopt|opt)$'
  --test-jobs 1 --leaks on --no-live-tail`. Use generous diagnostic hang
  protection if the existing limit prevents observing completion; record it.
  Normal Debug uses its own build tree and the same selection.
- Closure: establish and repair the mechanism, retain full corpus assertions,
  pass focused normal/sanitized checks and the original four-cell corpus
  under the declared scheduling, then complete the missing Linux assurance.
  If an actual sanitizer report appears, create its own SAN-nnn entry and
  follow all existing memory-finding closure rules. Moving the corpus to
  weekly execution alone cannot close SAN-QA-020.

### SAN-QA-020 isolated Linux diagnosis and repair

[Focused run 39407904028](https://github.com/adesutherland/CREXX/actions/runs/39407904028)
passes on the original `e5c066b87945dc697df6c771fc5a56a20962c772`; checkout
logs confirm the actual commit. The selected regex is exactly
`^ts_unicode_normalization_rxtvm_(noopt|opt)$`, with `--test-jobs 1`,
`--leaks on`, and build leaks on in both trees. Ordinary Debug passes **2/2**
in **24.75/24.37 seconds**, **49.34 seconds** total. Maintained Linux x64
ASan/LSan passes **2/2** in **265.42/261.81 seconds**, **527.34 seconds** total.
Complete build and test logs contain no sanitizer diagnostic or compiler warning.
The ordinary configure/build/test step takes 7m13s; the sanitizer step 30m21s
(including approximately 21m25s target build and 8m47s tests). These are bounded
hosted diagnostic observations, not product performance measurements.

The unchanged complete corpus finishes with over 4.5x margin beneath its
1200-second limit when isolated, establishing the shared-pool scheduling defect.
All four normalization cells now declare `RUN_SERIAL TRUE`; retain TIMEOUT 1200,
the full Unicode corpus and every transformation/predicate/identity oracle.
Repaired normal Debug four-cell qualification passes locally on Apple arm64:
`tools/asan-run.sh --build-dir cmake-build-debug --phase ctest --regex
'^ts_unicode_normalization_' --test-jobs 8 --leaks off --no-live-tail` passes
**4/4**, serially, in **70.63 seconds** (16.16/18.66 seconds threaded,
18.86/16.89 seconds switch). It reuses unchanged product/fixture inputs after
CMake regenerates the declared scheduling; all four retain TIMEOUT 1200.
Repaired Linux four-cell and broad assurance remain open, as does SAN-QA-019's
Linux confirmation. The combined final Linux qualification owns these checks;
no second unchanged hosted build is launched just to recheck this property. No memory-safety finding was observed.
The replay's dependency configure reports fetching DSLSH develop; the current
remote identity is `383e5daab0ffcf6ec83e02db7a01a27709dabb2c` (July 28), with no
apparent recent drift. Its exact fetched commit is not printed by these logs;
new donor seals must bind actual dependency contents instead of branch names.
Downloaded temporary logs were inspected and deleted after this summary.

### Where the time goes

Times below come from hosted job/step timestamps and CTest's final wall time.
They are diagnostic observations on hosted runners, not Release performance
measurements or estimates of future savings.

| Platform | Instrumented build | Additional qa-prep | CTest wall time | Whole job |
| --- | ---: | ---: | ---: | ---: |
| Linux x64 ASan/LSan, stopped early | 70m 14s | 1m 09s | 81m 55s | 2h 33m 43s |
| macOS arm64 ASan, complete | 52m 42s | 49s | 183m 57s | 3h 58m 06s |

The workflow took 3h 58m 25s elapsed; the two main jobs consumed approximately
6h 32m combined runner time. These are different quantities.

Joining the macOS completion records to the unchanged hotfix CTest inventory
matched all 3,294 names. Its 220 `RUN_SERIAL` tests account for **99m 54s**
of elapsed test execution, including about 75 minutes of compiler-labelled
tests. They cannot overlap other tests within that CTest invocation. The four
`classic_expression_contract_*` cells alone total 15m 03s; the four
`levelc_bif_reference_audit_*` cells total 10m 28s. The 16 structured-flow
cells total 10m 52s. Blanket removal of serial protection is not justified:
many tests have nested toolchains or shared workspaces.

The four normalization cells total 25m 39s of individual macOS test time,
but run concurrently: that sum is **not** removable wall time. Likewise,
overlapping label totals must not be added as a savings forecast. The
`qualification` and `stress` tiers total only 70 and 29 seconds respectively
on this run; simply excluding those labels will not solve the runtime problem.

The build currently invokes the default ALL target, followed by broad
`qa-prep`, independently of test selection. Filtering CTest alone leaves
53–70 minutes of build work. Existing `CREXX_PREP_TARGETS` and stage targets
provide an owning dependency model to reuse for a narrower daily preparation
target. The available logs do not attribute the build time to individual
actions, so no claim is made about how much of that time is avoidable.

This is not literally every project test: timing-measurement tests are already
excluded, llama is disabled, and the experimental compiler option defaults
off. The core Level L libraries, tools and their tests are included.

### Architecture review: present implementation and intended direction

The current developer checkout's `research/level-l/compilers/PLAN.md` and
`doc/architecture.md` explicitly put the new Classic compiler in B/Level L,
with native B as its build seed and no native source compiler/backend in new
compilation requests. RXAS, RXLINK, VM and native transport remain legitimate
dependencies. Default replacement, native-C retirement and B self-hosting
remain separate decisions. The hotfix snapshot predates some of that plan's
19–20 October implementation; no code or scope was copied between checkouts.

Consequences:

- Language/compiler correctness remains essential. A compiler written in
  cREXX can produce wrong code without causing an ASan report. Its full
  semantic, diagnostic, source/TRACE, emitted-code, atomic-publication and
  resource-limit contracts belong in ordinary QA with independent expectations.
- Its native memory risk primarily lives in the VM, native libraries/plugins
  and host calls that execute it. A large compiler run is useful VM input,
  but thousands of language cases may revisit the same native paths.
- Long-lived tool/service use raises the importance of repeated requests,
  failed-request recovery, memory retention, aliasing and teardown. Hundreds
  of separate successful CLI processes do not establish those properties.
- Native bootstrap, assembler, linker, readers and shipped native compiler
  services still require native correctness and sanitizer tests. Their
  current presence cannot be discounted on the basis of eventual replacement.
- `.rxbin` does not require ASan instrumentation: the native engine and
  first-party native code executing it do. Therefore preparing fixtures with
  ordinary tools and executing them on an instrumented VM is a valid distinct
  runtime test scope. It does not qualify the fixture producers' memory safety.

### What the source and test inventory already provide

CTest discovery in hotfix reports 3,480 registrations, of which 1,760 originate
in `compiler/tests`, 226 in `interpreter`, and 91 in `tests/rxpa`. These are
registration counts, not native coverage measurements: compiler tests also
exercise the VM, while some runtime tests are registered elsewhere.
Only 17 tests carry the literal `sanitizer` label. Selecting that label would
miss substantial native coverage; current labels are not a strategy.

Useful existing foundations, inspected rather than rerun in this review:

- `interpreter/tests/test_rxvmmemory.c`: allocator classes, slab/extent
  boundaries, resize, overflow, wrong-thread ownership, live-allocation counts
  and teardown. `ts_regvalue_tester.c` adds value/reference/binary operations
  and controlled allocation-failure paths.
- Handwritten RXAS suites such as `tests_binary_memory.rxas`,
  `tests_references.rxas`, `tests_signal_call_unwind.rxas`,
  `tests_instruction_gaps.rxas` and `tests_nr09_superinstructions.rxas`.
  These can exercise operations without relying on the new source compiler
  to emit them. `rxops.h` and `rxopeffects.h` supply an opcode/effects inventory,
  but an inventory entry is not proof that the instruction was tested.
- Initializer, sealed-program-generation and persistent-worker C harnesses;
  `test_rxvmplugin_context.c`; RXPA concurrency/host/UTF tests; and
  `tests/rxpa/native_payload_funcs.crexx` with copy/finalizer observations.
- The runtime has real distinctions between switch and computed-goto engines,
  standalone and embedded use, static/dynamic providers, and provider
  concurrency policies. `rxvm` itself is the selected concrete-engine
  symlink/copy, not a third dispatch implementation.
- The custom allocator explicitly poisons freed slots for ASan. Its own
  teardown test also deliberately leaves a logical allocation live, verifies
  that teardown reports it, then releases the arena. This demonstrates why
  allocator/resource assertions must accompany process-exit leak detection.

A targeted search of maintained workflow, CMake, tools, scripts, test, VM,
RXPA and binutils paths found no per-workload native branch-coverage selection
report or maintained `LLVMFuzzerTestOneInput` harness. This is an identified
measurement/harness gap, not proof that any particular test is redundant.
The recent hosted timings remain the baseline above; no native coverage was
measured during this review.

### Proposed test structure

| Layer | Purpose and retained evidence | Sanitizer treatment |
| --- | --- | --- |
| Language/tool correctness | Complete B/C/G/L and library contracts; independent expected results/errors; IR/RXAS structure; CLI/service parity; deterministic failure/publication; ordinary resource limits | Full ordinary Debug/Release QA. Selected cases also enter runtime sanitizer profiles when they add a native scenario |
| Native VM primitives | Direct value, allocator, reference, opcode, loader, numeric and text/binary boundary contracts, including negative paths | Core ASan/LSan on Linux and ASan on Apple; both concrete dispatch engines where available |
| Native plugins and embedding | API/ABI, lifetime, callbacks, concurrency policy, resource ownership, loading and partial-failure cleanup | Instrument first-party provider/bridge code and embedding harnesses as well as VM code; explicit platform/provider inventory |
| Runtime state sequences | Reuse, aliasing, repeated requests, load/link/initialize/run/unwind/destroy, workers and native resource lifetimes | Short deterministic sequences daily; longer bounded soak/generated sequences separately |
| Real cREXX workloads | Complex interactions from actual Level L, new C compiler, libraries and applications, with independent output assertions | Small fixed representative panel plus affected/rotating workloads; selected tools themselves execute on the instrumented VM |
| Native bootstrap/toolchain | Memory safety and correctness of the C seed and remaining C assembler/linker/readers/editor services | Separate native-tool profile; focused permanent regressions and affected-change checks, with broader bootstrap qualification retained |
| Installation/platform/performance | Installed resolution, ABI/package layout, cross-platform behavior and ordinary-product performance | Dedicated existing QA. No ASan timing ratios; retain native loading/cleanup cases in the runtime profile |

#### Directly test the VM and plugin contracts

Build a source-to-test coverage map around these obligations, reusing existing
tests before adding new ones:

1. **Values and allocation:** zero/one and allocation-class boundaries;
   grow/shrink/reuse; strings, binaries, decimals, attributes and references;
   source/destination aliases; local/argument/global returns; allocation
   failure preserving valid state; cross-owner rejection; logical leaks.
2. **Instruction and call state:** admitted operand forms and edge values,
   frame/register reuse, copied/moved/borrowed values, callbacks, exceptions,
   cancellation and unwind. Every supported instruction family has a mapped
   normal and applicable error/boundary test. Check effects and resource state,
   not just a successful process exit.
3. **Loading and embedding:** valid and malformed RXBIN/metadata boundaries,
   partial failure, initialization cycles, late loading/relinking, repeated
   contexts and calls, failed initialization without corrupting an already
   published module set, and complete teardown.
4. **Providers:** a per-provider table of registration/load, argument/return
   boundaries, create/copy/move/overwrite/finalize, failure halfway through,
   callbacks/reentrancy and declared concurrency. Distinguish the shared RXPA
   machinery from each provider's own native resource/algorithm code.
   Include real provider implementations as well as controlled mock failures.
5. **Worker/process/resource state:** deterministic handshakes, ownership
   transfer, cancellation during useful lifecycle phases, repeated startup/
   teardown, native handle/descriptor balance, and recovery after errors.
   Retain genuine concurrency within these tests even when CTest isolates them.

Use independent expected values/state for direct RXAS tests. Agreement between
the two VM engines is useful additional evidence, but they share substantial
handler code and can agree on the same defect. A compiler-generated workload
also cannot establish coverage of instructions that compiler never emits.

For long-lived compiler/services, assert logical live objects/allocations and
native handles at explicit quiescent points, plus bounded retained capacity
after warm-up. Respect documented slab reserves; do not demand zero RSS after
every request. Check failure followed by another successful request and final
context destruction. ASan/LSan complements those assertions; it does not
establish language ownership semantics or bounded reachable caches.

#### Use real workloads for interactions, not as the only VM test mechanism

Start the representative panel with:

- Level L recognition/generation and IR transformation/emission, retaining
  distinct backend allocation patterns;
- the new C compiler in repeated service and CLI use: successful compilation,
  invalid input, output refusal, reuse and teardown, then execution of emitted
  code against independent expectations;
- Unicode/text/binary transformations at boundary and expansion sizes;
- a native-object/provider workload and a worker/stream workload.

Include dynamic compilation/INTERPRET lifetime cases when that capability is
implemented and approved; do not claim current coverage of future features.
The current sanitizer workflow defaults the research compilers off, so
representative new-C execution requires an explicit prepared workload.
Do not add the whole research compiler matrix to the native suite by default.

Proposed extension after the initial map: bounded, reproducible generated
operation sequences for values/containers and controlled malformed-container
inputs. Use a model/expected-state oracle and retain minimized regression
sources for failures. Start any coverage-guided fuzzing with small resettable
native reader/value boundaries. Stateful worker or DSO load/unload scenarios
need process isolation and explicit cleanup; they are not automatically valid
in-process libFuzzer targets. Provider fixtures must avoid unintended external
network, command or filesystem effects. An error report remains a blocker,
not an acceptable fuzz outcome to suppress.

#### Decide duplication from evidence

Before moving a costly family out of routine sanitizer execution:

1. Record what it promises: native code paths, value/size boundaries, lifetime
   sequence, failure mode, provider, engine, platform and independent oracle.
2. Measure **native** function/region/branch coverage for a bounded comparison
   of the direct panel, representative workloads and costly candidate family.
   Include first-party plugins and embedding code, and retain per-family
   attribution. Record which source/ABI/compiler configuration was measured.
3. Identify the candidate's additional native branches and distinct scenarios.
   Equal coverage is necessary evidence for some reductions, not a proof of
   equivalence: the same branches can behave differently at a size boundary,
   after a different lifetime sequence, or under concurrency.
4. Retain the smallest understandable set covering required scenarios, every
   permanent SAN trigger and all unique high-risk paths. Replace uncovered
   failure paths with focused tests before removing broad coverage.
5. Put each moved family in the ordinary suite and, initially, the weekly
   broad sanitizer safety net. Record its retained runtime counterpart,
   residual risk, owner and change trigger. Unknown cases stay included.

Do not conflate native C optimization, B seed optimization, Level L IR passes,
RXAS optimization, dispatch engine and linked/static/dynamic packaging.
These are separate axes. Full Cartesian multiplication is not automatically
necessary; neither is blanket deletion of all unoptimized or second-engine
cases acceptable. Preserve combinations that change ownership, allocation,
loader, ABI or error behavior. Keep the product-entry smoke even where `rxvm`
aliases a concrete engine; audit build/link boundaries before deduplicating
apparently similar embedding harnesses. For example,
`cmake/CrexxTestModes.cmake` routes `_opt` tests through its linked-runtime
helper: a name that appears to vary only optimization may also vary linking.

Do not set a universal coverage-percentage target. Require no lost measured
high-risk coverage and explicit disposition of uncovered applicable branches,
with the scenario map as the acceptance contract. Retain compact summaries
and maintained test-selection sources; discard raw profiles/logs afterwards.

### Build separation, cadence and residual risk

**Proposed build separation:** prepare deterministic RXAS/RXBIN fixtures once
with the qualified ordinary toolchain; build the sanitizer VM/native dependency
closure and first-party providers/harnesses; run those fixtures on explicit
sanitized engines. Bind artifacts to source, generator, flags, ABI/format and
dependencies. Test discovery must establish that the executing VM/plugins are
the instrumented products, including subprocesses or embedded service hosts.
Use the existing CMake preparation ownership rather than a parallel build graph.
This removes a requirement to sanitize every fixture producer on every runtime
run; it does not remove instrumented build-time execution from the native-tool
profile or the maintained full gate.

The new compiler is both a tested tool and a runtime workload. Normal preparation
of its bytecode is distinct from executing its compilation service under ASan.
A small end-to-end test must exercise both that service and its generated
application on the intended engine. Do not let a wrapper silently substitute an
ordinary installed VM or a native source compiler.

After the coverage review, the recommended operating policy is:

- **Routine native foundation:** VM/plugin primitive, lifecycle and fixed
  representative-workload panels on Linux ASan/LSan and Apple ASan; all
  applicable historical SAN regressions run in their owning profiles.
  Maintain ordinary Windows/other supported-platform correctness; the two
  sanitizer hosts do not prove those platform paths.
- **Affected changes:** expand the relevant profile for VM primitives,
  allocator/reference/lifetime changes, native providers, seed/assembler/linker
  changes or changed generated-code shapes. Compare against the appropriate
  last qualified inputs, not merely the previous commit. Uncertain shared
  impact selects broader coverage.
- **Source-only cREXX changes:** full relevant ordinary functional and
  code-quality tests plus appropriate runtime workload updates. A new native
  usage pattern still merits targeted sanitizer checks; unchanged native inputs
  do not require a fresh whole-toolchain campaign for each source increment.
- **Weekly during migration:** existing broad non-measurement sanitizer
  coverage, with full ordinary conformance preserved. This is an initial
  safety net while measuring the focused strategy, not a permanent assumption
  that every functional test must always run under ASan.
- **Release and SAN closure:** existing full gates stay required until an
  explicit later decision accepts a coverage-backed replacement. Current
  SAN-QA-019/020 obligations are unchanged.

Main residual risks are missed size/history interactions, rare concurrency
interleavings, and optional/platform code absent from the tested configuration.
Mitigate with boundary/state oracles, bounded generated/stress sequences,
weekly broad checks and an explicit provider/platform inventory. A disabled
provider has no pass. Instrument first-party optional adapters in their owning
lane while preserving the approved upstream llama.cpp/CUDA boundary.

ASan is not a race detector. Consider a separate, initially exploratory
ThreadSanitizer pilot for worker/provider concurrency and maintain the existing
exploratory UBSan boundary. Do not multiply the whole suite by every sanitizer
or introduce a new mandatory matrix before feasibility and value are reviewed.
Any actual first-party finding still follows the worklist rules.

The initial two-hour daily target remains a **provisional engineering target**,
pending separate measurement of fixture preparation, instrumented native build,
runtime panel and native-tool profile. It is neither a forecast nor a kill
timer. Native `-O1 -g` ASan with assertions retained is a possible separate
configuration pilot; LLVM recommends optimization for reasonable ASan speed,
and this repository already uses an ASan `-O1` override for `rxvmintp.c` in
Release/RelWithDebInfo. Do not silently replace the maintained Debug sanitizer
configuration or drop its diagnostic/closure coverage.

Retain locks, measured isolation and generous hang protection. Report cold and
reused build costs separately, plus total runner consumption. Keep platform/
profile/configuration-qualified success records; daily success cannot mark
broad coverage complete, and a successful unchanged Apple scope need not be
repeated solely because Linux failed.

Scheduled workflows still load from **master**, whose 240-minute guard is
nearly exhausted. Any later rollout before a release needs separate approval
for the narrowly scoped workflow-only master update previously proposed.
This review grants no exception to release-branch discipline.

### Approved bounded measurement scope

Adrian authorised the coverage map and bounded measurements in hotfix, with
no change to test selection. STEP03 is complete for the bounded scope below. Use a disposable Clang native
Debug source-coverage build, assertions enabled, on this Apple arm64 host;
this is a diagnostic coverage/timing exercise, not ASan qualification or a
product performance verdict. Existing normal fixtures are prepared with the
same source inputs. No CMake registrations, workflow or runner policy changes.

The panel covers direct allocator/value/ownership and provider contracts,
handwritten RXAS, and selected Unicode, Level L structured-flow and Classic
compiler aggregates. Measure each retained oracle serially, once per cell,
with the existing generous hang limit. Stop expansion at 30 minutes of actual
panel execution; report any unmeasured family rather than extrapolating a
coverage claim. Build time is recorded separately. Keep branch/function
attribution for VM and first-party providers; producer-tool instrumentation
is a distinct scope. Use marginal coverage plus scenario obligations, never
branch equality alone, to propose membership. Delete temporary raw profiles,
logs and harnesses after recording concise results here.

### Coverage map: native risks and existing ownership

This is a **scenario map**, not a claim that every branch or scenario has been
executed in this review. CTest discovery at `e5c066b87945` gives 3,480 Apple
registrations; the existing `performance-measurement` exclusion removes 186,
leaving exactly **3,294**, matching the hosted Apple selection. The Linux
selection is 3,289 because platform registrations differ. Counts are scoped
to the configured options; the research compiler and optional live providers
are not silently included.

In this map, `*` means all existing names in that family, not a new CTest
selection expression. Unmeasured families remain included. Source ownership,
commands, preparation targets and assertions were inspected through CMake and
CTest JSON; actual measured cells are identified separately below.

| Native owner / risk | Existing test counterpart | Decision / remaining gap |
| --- | --- | --- |
| `rxvmmemory.c`, `rxvalue.h`, `rxvmref.c`: allocator class/extent edges, overflow, grow/shrink, aliases, OOM, wrong owner, logical live allocations | `rxvmmemory_allocator`, `value_factory_allocation_failure`, `ts_regvalue_tester`, `rxvmstem_storage`, `rxvmstem_allocator_family` | Keep all. Direct measured panel. Process-exit LSan cannot replace the logical-live-count assertions. |
| `rxvmintp.c`, `rxvmhandlers_*.inc`, `rxops.h`: register/value forms, numeric/text/binary operations, source state and dispatch | `rxas*` runtime instruction tests, especially `rxasinstructiongaptests`, `rxasnr09superinstructiontests`, register/UTF flags, references, string, binary-memory, decimal, conversion, object/interface suites | Keep all. Twelve source-independent RXAS fixtures measured on both concrete engines. There is still no complete opcode × operand × failure-path matrix; do not call the panel ISA-complete. |
| `rxvmload.c`, `rxvmlinkindex.c`, `rxvmprogram.c`, `rxbin` readers: malformed image, bounds, autoload, link/initialization failure, publication | `rxasjtablecorrupt*`, `rxasjumptable*`, `rxbin_autoload_contract`, `module_initializers-{rxvml,rxbvml,rxtvml}`, `program_generation_control-{rxvml,rxbvml,rxtvml}` | Keep every variant. Source-inspected, not measured in the bounded panel. General malformed-container fuzzing remains a gap; jump-table corruption is not complete loader validation. |
| Calls, signals, frame/return ownership and unwinding | `rxassignal*`, `rxasreference*`, object/interface tests; eight `san002_*` engine/mode cells | Keep all; retain independent expected output, signal and alias semantics. Signal-call unwind and reference paths included in the measured RXAS panel. |
| `rxvmworker.c`, `rxvmexecutor.c`, `rxvmactive.c`: reuse, ownership, wakeups, cancellation and teardown | `rxvmworker_lifecycle`, `rxvmactive_isolation`, all non-measurement `persistent_worker_executor-*`, worker-startup and native-return tests | Keep all. Deliberate concurrency and stress states are not duplicates of isolated CLI runs. No claim of race-freedom from ASan; TSan remains a separate proposal. |
| `rxvmchannel*.c`, byte endpoints and process protocol | `rxvmchannel_provider_registry`, `rxvmbyteendpoint_substrate`, `rxvmchannel_byte_provider`, `rxvmchannel_request_lifetime`, `rxvmchannel_process_crash_replacement`, `gate_f_channel_*` | Keep all; source-inspected, not measured here. Retain crash recovery and descriptor/request lifetime assertions. |
| `rxspawn.c`, file/socket/TLS/platform code | `rxspawn_resource_inheritance`, POSIX termination/launch-diagnostic tests, `rxvmfile_private_open`, `rxassockettests`, `ts_rxsocket_*`, `ts_socket_tls_live_*`, `ts_http_streaming_*`; platform's 17 tests on this host | Keep all available variants. Linux/OpenSSL, Apple/Network and Windows/Schannel are distinct; this Apple measurement cannot qualify the other implementations. |
| `rxvmplugin`, RXPA manifests/bindings and host ABI | `rxvmplugin_context_ownership`, `rxpa_host_text_services`, `rxpa_utf_validation`, static catalogue/session, factory rollback, manifest failure, legacy serialization/reentry, quiescence/startup, dynamic ownership tests | Keep all. Thirty-five native C cells measured, including eleven bundled-provider concurrency cells. Shared host infrastructure coverage does not establish all provider algorithms. |
| Native payload/object ABI, finalize/copy and callbacks | all 91 `tests/rxpa` registrations and two multplugin registrations; specifically `rxpa_native_payload_funcs_{dynamic,static}_{noopt,opt}` and native object/callback/unwind tests | Keep every variant. These distinct ABI/lifetime paths are not substituted by the concurrency panel. Payload fixtures remain essential even where source coverage overlaps. |
| Decimal provider/context and numeric edge behavior | `rxasdecimaltests`, `rxasdecimaltests-mc`, `rxasdecimaltests-db`; seven mc-decimal and one db-decimal registrations; `rxvmplugin_context_ownership` | Keep all. Measured default-decimal RXAS plus independent context teardown; no claim that the alternate provider was covered. |
| Native C seed/frontends, AST/import ownership, metadata and emitters | compiler's native unit/fault-injection tests and permanent SAN tests; source/diagnostic/import/TRACE/emission matrices | Keep unmeasured families. Measure costly Classic aggregates with producer instrumentation as well as runtime instrumentation. Migration to cREXX does not retire the shipped C tools. |
| Native RXAS/RXLINK/readers, SSA and metadata | assembler parsing, linker, binutils/contract, RXAS optimizer/flow-graph tests; compiler contract builder | Keep all. SAN-001 SSA-growth, SAN-011/012 contract ownership and native-format error paths remain pinned. Ordinary fixture generation alone does not qualify these producers. |
| cREXX library/tool algorithms running on the C VM | complete ordinary B/C/G/library suites; Unicode, four Level L parsing backends, RexxScript and compiler workloads | Keep ordinary conformance complete. The bounded candidate panel measures native marginal coverage; any proposed cadence reduction names exact cells below. |
| New research Level C compiler | owning `research/level-l/compilers/PLAN.md`, independent semantics/diagnostics, code-shape and service/resource tests | Absent from the current overnight default. Add a deliberately bounded representative success/error/recovery/teardown workload when prepared and approved; this hotfix snapshot cannot qualify newer develop research work. Do not claim current coverage of future INTERPRET. |
| Install, driver, packaging, SAA and native launch boundaries | driver, `crexxsaa_*`, project-build/import/installed-provider and native packaging contracts | Keep all unmeasured tests. Do not drop `crexx_project_build_contract` or `binary_global_import_types` just because they are expensive; the latter is the permanent SAN-008 trigger. |

#### Provider inventory and explicit limits

All currently enabled first-party provider tests stay in the proposed daily
scope unless a later explicit item says otherwise. On this Apple configuration:

- Eleven **measured** concurrency providers: cipher, hash, stack, strings,
  getpi, id, float, stats, vector, fs and platform. Static provider registration
  and dynamic loading are both exercised by the owning concurrency harness;
  each cell's executed profile includes its native DSO where applicable.
- **Retain functional/API tests too:** `arrays_test_*`, `cipher_test_*`,
  `getpi_test_*`, `stack_test_*`, `strings_test_*`, `rxfloat_*`, `rxstats_*`,
  `rxvector_*`, `rx_hash_*`, `rxid_*`, `rxfs_*`, `rxplatform_*`. Concurrency
  probes are small behavioural probes, not replacements for these surfaces.
- Other enabled providers retain `keyaccess_test_*` and
  `keyaccess_compact_failure`, `llist_test_*` (SAN-005), `treemap_test_*`,
  `map_test_*`, `recv390_test_*`, `tcp_test_*`, all thirteen `rxsqlite_*`
  registrations, and the four `rxconsole_*` registrations. Their algorithm,
  resource and error-path coverage was not measured in this panel.
- `rxcstream` is loaded by the Classic runtime; the compiler-aggregate panel
  instruments it. Keep `classic_stream_lifecycle` for stream ownership and
  reuse. It is not replaced by arithmetic-expression execution.
- ODBC is disabled for live use, but both mock-driver VM tests and both host/
  prepared-session qualification tests remain. They do not qualify a live
  driver/database. Mainframe ADDRESS harnesses are opt-in: retain their
  separate native-boundary/allocator policy and SAN-013 obligations.
- llama, GTK, the deprecated socket plugin and Windows-only rxml/fileio/matrix
  have no coverage from this configuration. They need their existing owning
  platform/provider lane when enabled. The llama first-party adapter boundary
  does not extend sanitizer responsibility to the approved excluded upstream
  engine/CUDA closure. A disabled provider is recorded as **not tested**.

Preserve focused reproducer obligations for **SAN-001 through SAN-013**,
including those without a `sanitizer` label: RXAS flow-graph growth,
`compiler_contract_builder`, `frame_control_ast`,
`binary_global_import_types`, `llist_test_*`, the `san00[23467]*` cases,
SAN-009's optional first-party adapter probe and SAN-013's opt-in mainframe
boundary cases. Existing source-library generation triggers also remain in
the broad native-tool/release closure. SAN-QA-019/020 remain open for Linux
proof; a smaller proposed profile must not bypass that outstanding work.

### Bounded measurement results

**Outcome:** 87/87 cells passed their original value/error/output assertions:
35 native C cells, twelve handwritten RXAS cases on each engine (24), and
28 complete aggregate cells. Successful executions consumed **694.23 seconds
(11m34s)**; setup failures added about 110 seconds, still within the approved
30-minute execution bound. These are **Debug source-coverage runs, not ASan/
LSan qualification**, and do not close SAN-QA-019/020 or establish Linux passes.

Inputs were hotfix **`e5c066b87945dc697df6c771fc5a56a20962c772`**, with only
this documentation changed and unrelated `scripts/__pycache__/` preserved.
Host: Darwin 25.6.0 arm64, 10 logical CPUs; Apple Clang 21.0.0
(`clang-2100.1.1.101`), CMake 4.3.2, Ninja, Debug assertions retained,
`-fprofile-instr-generate -fcoverage-mapping` for C/C++. Native optimisation,
VM handler policy, bytecode optimisations and data/oracles were unchanged.
`ENABLE_LLAMA=OFF`, live GTK/ODBC off, Network TLS, research compilers off.

| Measured group | Cells | Local elapsed total | Native functions hit | Native source regions hit | Native branch outcomes hit |
| --- | ---: | ---: | ---: | ---: | ---: |
| Direct C plus handwritten RXAS | 59 | 7.70s | 1,424 | 22,909 | 6,881 |
| Classic expression contract | 4 | 264.80s | 3,033 | 58,017 | 38,571 |
| Classic BIF reference audit | 4 | 232.60s | 2,594 | 45,003 | 28,507 |
| Level L structured flow | 16 | 115.44s | 593 | 16,253 | 4,253 |
| Full Unicode normalization | 4 | 73.70s | 496 | 14,697 | 3,691 |

Counts are unions **within each row**, not additive coverage percentages.
They cover mappable first-party native source/headers and generated native
compiler code, excluding third-party sources and test C files. Branch and
region identities preserve the function and macro-expansion file ID; function
branch records supply the actual source filename. This avoids confusing VM
handler macro expansion with one shared source location. Both concrete engine
results are retained separately for engine-specific comparisons.

Important observations:

1. The direct panel is useful but insufficient on its own. Structured flow
   adds **626** native branch outcomes beyond it, and normalization adds
   **530**. These include VM handlers, loader/image reading and value paths.
   The two figures overlap; neither is an independent additional percentage.
   Large real cREXX workloads still have a place in VM assurance.
2. For **each** four-cell Classic family, keeping direct `noopt` and
   `linked_opt` retains **every measured function, region and branch outcome
   of that family's four cells**, even without borrowing coverage from other
   tests. Native compiler, assembler and linker coverage was included. Both
   source-compiler modes and both direct/linked runtime paths remain present.
   The assembler stays at its existing default optimisation in all four;
   the `noopt` flag here controls `rxc`, not native C compilation or RXAS.
3. Dropping all unoptimised cells is unjustified. For example, the Classic
   expression optimized-only pair loses 530 observed branch outcomes compared
   with its complete family. Structured-flow optimized-only execution still
   misses two outcomes even after the direct panel: an `INC_REG` handler
   outcome and the string-concatenation destination-alias path. Keep LL1 in
   both modes on **each** engine; the other backends' optimized cells then
   retain the full measured structured-flow coverage separately on each engine.
4. All four normalization cells passed here, with the complete Unicode input
   and assertions. This does **not** resolve the hosted Linux timeout. A
   two-cell cross-combination adds no missing coverage in the measured panel,
   but changing this family's frequency now would obscure the still-open
   failure investigation. Keep all four in the approval proposal below.
5. Preparation has substantial, separable duplication. Each Classic expression
   cell launches 52 compilers, 44 assemblers and 44 VMs; linked cells also
   launch 44 linkers. Across four cells that is **648 processes: 208 rxc,
   176 rxas, 88 rxlink and 176 rxvm**. The eight invalid-source compiles per
   cell use identical options even under `--noopt`. A later shared preparation
   fixture could keep all 176 VM executions with 96 rxc calls (88 valid mode/
   case combinations plus eight diagnostics), 88 rxas and 88 rxlink calls.
   Prove artifact identity and every assertion before taking that reduction.
   The BIF family can similarly prepare once per compiler mode while retaining
   four runtime cells. Its optimized cells took 108.97s/107.59s versus
   7.72s/8.31s unoptimized; this is a diagnostic observation, not a compiler
   performance verdict or an ASan speed ratio.

**Method and reproducibility.** Discover the same inventory with
`ctest --test-dir cmake-build-debug --show-only=json-v1`. The eight direct
native cells are the five allocator/value/stem names in the map plus
`rxvmplugin_context_ownership`, `rxpa_utf_validation` and
`rxpa_host_text_services`. Add all 27 registrations whose executable is
`interpreter/test_rxpa_concurrency`. The twelve RXAS names are
`rxasutfflagtests`, `rxasreferencetests`, `rxassignalcallunwindtests`,
`rxasnr09superinstructiontests`, `rxasstringtests`, `rxasbinaryslicetests`,
`rxasbinarymemorytests`, `rxasdecimaltests`, `rxasconversiontests`,
`rxasobjecttests`, `rxasinterfacetests`, `rxasinstructiongaptests`; execute
unchanged fixtures with both instrumented concrete VMs. Add all four named
Classic expression cells, all four BIF audit cells, all sixteen structured-flow
cells and all four normalization cells.

Configure a disposable build with:

```sh
cmake -S <hotfix> -B <temporary-build> -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  '-DCMAKE_C_FLAGS=-fprofile-instr-generate -fcoverage-mapping' \
  '-DCMAKE_CXX_FLAGS=-fprofile-instr-generate -fcoverage-mapping' \
  -DENABLE_LLAMA=OFF -DCREXX_ALLOW_NETWORK_DOWNLOADS=OFF
```

Build with `--parallel 4`, initially targeting `rxbvm`, `rxtvm`,
`test_rxvmmemory`, `test_value_factory_allocation`, `ts_regvalue_tester`,
`test_rxvmstem`, `test_rxvmstem_allocator_family`, `test_rxvmplugin_context`,
`test_rxpa_utf_validation`, `test_rxpa_host_services`, `test_rxpa_concurrency`.
Add `_rxpa_dynlink`, `_rxpa_invalid_manifest` and `cstream` for the measured
closure. The first native build reported 2.72s after 6.98s configuration;
adding the dynamic fixtures pulled in native tools and library generation and
cost 186.17s, with cstream another 0.36s. Ordinary selected-fixture preparation
in the existing build took 134.09s. These are different scopes with local
cache/filesystem conditions, **not forecasts of hosted ASan build times**.

Ordinary preparation used the selected tests' `CREXX_PREP_TARGETS`: library,
classlib, rxfnsc; rxc/rxas/rxlink/rxvm/rxbvm/rxtvm; the shared
`run_tests_{basic,decimal,interfaces,references,register_flags,signals,string,utf_flags}`
closure and `test_compact_format`, `test_db_decimal_archive_link_bin`,
`test_reentrancy`; all eight `ts_llstructuredflow_{ll1,lr1,packrat,earley}_{noopt,opt}`
artifact targets and all four normalization `*_artifact` targets. No `ALL`
or full sanitizer build was launched for this measurement.

A disposable CTest harness preserved each original command's working directory,
environment, timeout, expected exit handling and required/forbidden output
expressions. Runtime fixtures used ordinary bytecode and instrumented VMs.
Classic aggregates additionally used instrumented rxc/rxas/rxlink. An executable
view combined the instrumented binaries/providers with the ordinary fixture
libraries, including autoloaded dependencies; bytecode was not instrumented.
Use `LLVM_PROFILE_FILE=<cell>/%m-%p.profraw`, merge per cell with
`llvm-profdata merge -sparse`, then `llvm-cov export` over the executed native
closure. Export runtime/providers and producer objects separately with
`--skip-expansions --num-threads=1`; function-level region/branch data still
retains the expansion identities needed for comparison.

Each cell ran once successfully, serially; the first native cells overlapped
ordinary fixture preparation, so their tiny elapsed totals are not isolated
benchmarks. The costly workload cells had no concurrent task build or reporting
job. No warm-up/sampling campaign or native performance claim is implied.
Missing disposable-closure dependencies caused three setup failures: the
manifest fixture DSO, rxcstream, and autoloaded rxfnsg. The respective affected
cells were replayed after preparation was corrected; passing cells were reused.
These are preparation findings, not product or sanitizer findings.

The initial all-object expanded LLVM report exceeded local resources; splitting
reports and omitting the redundant expanded tree fixed extraction without
rerunning tests. LLVM also reports an absent profile for the **unexecuted
rxlink `main`** when its object is included for direct compiler cells, and a
constant/compatibility variant of the test-header helper
`rxpa_host_service_fixture.c:rxpa_host_has_string_view`. These have no covered
outcomes in those cells; the linker is covered in retained linked cells and
all host-compatibility tests remain retained. The comparison concerns observed,
mappable coverage, not proof that every compiled function was covered. Raw
profiles, exported data, scratch harnesses, logs and diagnostic build are
discarded after this summary; permanent fixtures and original assertions remain.

### Exact membership approved for implementation

**Recommended first tranche: move ten existing cells from daily sanitizer
execution to weekly broad/affected-family execution. Delete no test or oracle.**
Let **U** be the current platform's CTest inventory after its existing
`^performance-measurement$` exclusion. The proposed daily set is exactly
**U minus the ten names below**; every other test, including future unclassified
tests, stays included. On the measured Apple configuration this is **3,284 of
3,294**. Weekly, release and SAN closure retain **U**. Ordinary correctness
selection and performance governance stay unchanged. This is intentionally a
small, demonstrated change; it is not yet the final compact VM profile.

| Family / owner | Keep daily | Move to weekly and affected-family checks | Risk and guard |
| --- | --- | --- | --- |
| Classic expression / compiler tests | `classic_expression_contract_noopt`, `classic_expression_contract_linked_opt` | `classic_expression_contract_opt`, `classic_expression_contract_linked_noopt` | Medium: mode × linking interaction can differ despite equal coverage. Full ordinary four-cell oracle remains; native frontend/optimizer/emitter/RXAS/linker/loader or contract changes restore all four in affected assurance. |
| BIF reference audit / Classic compiler/library tests | `levelc_bif_reference_audit_noopt`, `levelc_bif_reference_audit_linked_opt` | `levelc_bif_reference_audit_opt`, `levelc_bif_reference_audit_linked_noopt` | Medium: same interaction risk, especially numeric size/context. Full expected-value audit remains in both daily cells; complete ordinary/weekly matrix retained. Restore all four for relevant native, ABI or generated-code contract changes. |
| Structured flow / Level L tests plus VM | All four LL1 cells; `ts_llstructuredflow_{lr1,packrat,earley}_opt_{rxbvm,rxtvm}` (six cells) | `ts_llstructuredflow_lr1_noopt_rxbvm`, `ts_llstructuredflow_lr1_noopt_rxtvm`, `ts_llstructuredflow_packrat_noopt_rxbvm`, `ts_llstructuredflow_packrat_noopt_rxtvm`, `ts_llstructuredflow_earley_noopt_rxbvm`, `ts_llstructuredflow_earley_noopt_rxtvm` | Medium: backend-specific unoptimized lifetime/history interactions remain possible. All four parser backends and both engines stay daily; LL1 retains both modes on each engine. Ordinary/weekly keep all sixteen; relevant VM/value/loader/bytecode or backend-contract changes restore all sixteen in consolidated affected assurance. |

All ten names exist exactly once in the measured U. Their removal from the
87-cell measurement leaves **77 cells with zero lost measured native functions,
regions or branch outcomes**. Structured-flow preservation also holds separately
on each engine, without borrowing coverage from the other engine. The selected
77 cells consumed **402.30s** versus 694.23s for 87, a counterfactual subtraction
of this single diagnostic run (**291.93s less, 42% of this panel's time**), not
a rerun or a whole-job/ASan savings prediction.

**Keep without reduction:** all four `ts_unicode_normalization_*` cells, all
native/direct VM, provider, concurrency, lifecycle, malformed-image and SAN
regressions in the map, and every unmeasured family. In particular, keep
`binary_global_import_types`, `crexx_project_build_contract`, RexxScript,
native compiler unit/import/editor/TRACE cases and the other library/Unicode
matrices. Cost or a `compiler` label alone does not establish redundancy.
Do not drop the second engine, static providers, noopt globally, live resource
or error-path tests. Do not shorten the normalization corpus or its assertions.

**Approved implementation order:**

1. Finish the unchanged isolated Linux timeout investigation, repair its
   scheduling/deadline cause if confirmed, and complete the outstanding
   SAN-QA-019/020 Linux proof. Do not activate these cadence changes as a way
   to turn the incomplete failing run green.
2. Implement the ten-name daily/weekly distinction with explicit per-profile
   results and a default-keep policy. Check the same named families on Linux;
   qualify the proposed profile with maintained sanitizer configurations before
   rollout. Linux leaks remain on; Apple leak detection remains unsupported.
   A daily pass never stands for a broad pass. Changes with uncertain shared
   native impact select the broader affected scope.
3. Separate ordinary fixture preparation from the sanitizer runtime/provider
   closure and instrument native producers in their own owning profile.
   Preserve instrumented original-trigger library generation in broad native
   tool/SAN/release qualification. Prove every plugin, autoload dependency and
   child executable; the failed disposable setups show why this is a real
   acceptance requirement. Reuse results by exact relevant inputs and platform/
   configuration/profile, without introducing a whole-suite per-slice gate for
   B/Level L source-only increments.
4. Then factor the repeated Classic preparation described above, retaining
   all normal/weekly assertions and proving isolated `ctest -R` preparation,
   failure propagation, concurrent access safety and artifact identity. Keep
   generous hang limits and existing serial/lock obligations. Measure the
   actual saved build/prep/test time in the final proposed profile.

The three moved families together currently occupy **36m23s of serial Apple
ASan test time** in the reviewed hosted run; only part of that time is removed
by this proposal. The existing Apple job is almost four hours, including a
52m42s build and 183m57s test phase. Ten moves alone do **not** establish the
provisional two-hour target. The larger opportunities are preparation reuse
and a separately qualified native runtime/tool split. Moving the remaining
compiler/library bulk to weekly requires further family evidence; it is not
silently authorised here.

The weekly/full safety net, affected-family expansion and independent ordinary
oracles are what make these frequency reductions a **medium, controlled risk**,
not zero risk. Coverage does not prove size/history/concurrency equivalence.
The new research compiler still needs its bounded real workload and service
lifecycle acceptance; the current overnight default does not include it.
Scheduled deployment still requires the separate narrow master-workflow decision.
At the end of bounded measurement no selection, workflow, runner, product
source or permanent test had changed. The subsequent approved implementation
and qualification status is recorded below.

Review verification: all ten proposed names were checked against the generated
U inventory (exactly one match each); no SAN/normalization name is moved; all
87 retained execution results are successful; the proposed coverage-set
difference is empty for functions, regions and branch outcomes. `git diff
--check` passes. Only this worklist is changed. Measurement builds, profiles,
exports, logs, helper scripts and five expression scratch directories have
been removed; the unrelated untracked cache remains untouched.

### Approved implementation design and current qualification

The dispatch-only replay route is published as `2fa4dad72a3b9db27251ddbcbcf94f4b51eaa029`.
[Linux replay 39407904028](https://github.com/adesutherland/CREXX/actions/runs/39407904028)
checks out the unchanged failing `e5c066b87945dc697df6c771fc5a56a20962c772`
and runs the two threaded normalization cells in normal Debug, then maintained
ASan/LSan, both with one CTest worker and Linux leaks enabled. This is diagnosis;
it cannot record broad assurance. YAML parsing, actionlint and `git diff --check`
pass. The unchanged replay has passed as recorded above; normalization scheduling is
repaired and locally verified before profile/preparation implementation. No selection change
has been activated.

The preparation implementation will use the existing CMake custom-command
owners. An explicit external ordinary fixture option will replace only audited
bytecode-generation commands with checked copies of their output/byproduct sets;
ordinary defaults and unknown generators retain their original commands. The
ordinary donor is sealed after its build/preparation, binding maintained source
contents, platform, relevant generator/configuration/tool identity and output
hashes. Reject missing, partial or stale donors; validate all source artifacts
before publishing output and remove stale destinations on failure. Retain native
prerequisites and native output paths. Build every native executable, library,
provider and harness in the sanitizer tree, including embedded generated-C
compilation. The installed `crexx` driver already records effective native C and
link flags for child builds; the first-party external SDK fixture needs explicit
parent sanitizer-flag forwarding. This is a preparation gap, not a sanitizer
finding.

Full/weekly/native-tool profiles must use original instrumented generation and
must reject trees used for external fixtures, including a tree subsequently
reconfigured with the donor option cleared. Daily selection remains exactly
U minus the ten approved names. Unknown tests stay; no command/label ownership
classification removes further tests. Affected native/shared/unknown changes or
missing comparison history select broad U. Manual/tag defaults stay full. A
tested weekly/daily policy can be implemented with activation disabled until
platform qualification and the separate master decision. Success identities
include platform, effective configuration, profile, preparation mode and selected
inputs; an empty selection fails and a daily/focused pass cannot write broad
assurance.

Shared Classic preparation is limited to the expression contract and BIF audit.
Keep every runtime oracle, expected exit (including 28), diagnostic identity and
CRLF normalization. Use CTest setup fixtures so isolated `-R` runs prepare their
inputs and setup failures prevent consumers. Serialize/lock preparation, bind it
to producer/source/library hashes, publish atomically and clean scratch output.
The generic BIF linked helper's other consumers retain their current behavior.

Normal Debug shared Classic qualification passes **11/11** (eight runtime
cells, two setup fixtures and integrity) in **268.20 seconds**, using
`tools/asan-run.sh --build-dir cmake-build-debug --phase ctest --regex
'^(classic_expression_contract|levelc_bif_reference_audit)_' --test-jobs 1
--leaks off --no-live-tail`. BIF preparation is 97.22 seconds and its runtime
cells 0.25–1.31 seconds; expression preparation is 83.40 seconds, direct runtime
7.05–7.10 and linked consumers 34.62–35.01 seconds including their first link.
These are bounded normal Debug observations, not an ASan or speedup claim.
Independent recording-stub comparison against the original harness verifies
all 44 runtime sources, language modes, expected values/exits and diagnostic
fragments, plus all eight source-error bodies/codes, unchanged. Generated
CTest discovery confirms isolated `-R` includes integrity, owning preparation
and exactly the requested runtime cell. Shared preparation compiles/assembles
both modes; linking occurs only for consumers that need it, under the same lock.
A source/product identity change invalidates preparation. A linked image must
have a matching published owner hash before execution; unowned interrupted
outputs are rebuilt. Disposable focused logs were inspected then deleted.

Further source-audit constraints: validate source/configuration/dependency identity
once per mandatory build validation, not once per copied file, then check each
owner's artifact hashes. Include actual configured external dependency contents
and identities (DSLSH can be fetched from a moving branch), and reject changed
inputs at final qualification. Local source mutation during a run invalidates
proof. Keep mixed native-adapter generation original: its BYPRODUCTS include
provider DSOs, which cannot be taken from an ordinary donor. The audited fixture
helper accepts only explicit source/RXAS/RXBIN/generated-C outputs; native binaries
remain owned by instrumented targets. Copy the full Stage G generated source,
RXAS and RXBIN set; retain Level L's original L1/L2 compare-files verification.
A disposable CMake argument-boundary experiment passed for literal and escaped
semicolons; the permanent independent fixture must also cover spaces, generator
expressions, multiple commands/byproducts, ordering and failure blocking consumers.

### Local implementation qualification and candidate gate

The implementation retains the original ordinary/full test behavior, adds four
preparation/integrity CTests and implements daily as **U minus exactly ten**.
Current Apple discovery is **U=3,298**, daily **3,288**; the original coverage
measurement used U=3,294 before these infrastructure additions. All four
normalization cells retain RUN_SERIAL and TIMEOUT 1200; native/SAN and unknown
new names remain included. No product C, Rexx library or research compiler
source changed in this tranche.

- Core/SDK/stage preparation passes through `tools/asan-run.sh --build-dir
  cmake-build-debug --phase build --build-target cri07_rxpa_sdk_consumer_prereqs
  --build-target stage-optional --build-jobs 6 --build-leaks off --no-live-tail`,
  with no compiler warnings or sanitizer diagnostics.
- Focused normal Debug passes **12/12 in 131.51 seconds** using runner phase
  ctest, one test job, leaks off and regex
  `^(sanitizer_strategy_contract|rxpa_external_sdk_consumer|crexx_project_build_contract|crexx_native_.*)$`.
  SDK takes 27.19 seconds, project/incremental/provider contract 88.61 seconds,
  and eight native driver/packaging cases 0.97–5.18 seconds. This validates
  ordinary routing/package correctness; candidate ASan execution remains open.
- Ten independent behavioral contracts pass in **2.90 seconds**. They execute
  actual CMake custom commands to compare ordinary forwarding, semicolons,
  spaces, generator expressions and multiple commands/byproducts; verify
  multi-config ordinary generation; reject stale/missing owner members and
  changed source/configuration/actual dependency content; block consumers after
  failure; exercise interprocess locking and interrupted/unowned linked-image
  publication; audit provider-own compile flags and the product VM entrypoint;
  check cadence/default-keep/platform/profile reuse and false-broad cases.
  The actual runner also executes a controlled CTest then fails its post-test
  native audit, proving no success completion or result is published.
- Fresh real CREXX ordinary/Ninja and ASan/Ninja configurations agree on 95
  relevant settings and actual DSLSH commit/content identity
  `383e5daab0ffcf6ec83e02db7a01a27709dabb2c`. Temporary configure trees/logs
  were removed. Canonical reuse is tested across relocation/history metadata,
  with semantic/configuration/tool/dependency/host/preparation differences
  still separated. Equivalent broad profile labels share one broad cache scope.
- Native closure verifies effective flags for built source objects, nested SDK
  provider sources, product VM routing, driver child flag files and linked
  native products/copies throughout the build tree. CMake private compiler
  probes and relocatable objects are distinguished from executable/provider
  outputs; objects retain their separate flag audit. Newly built CTest targets
  are audited after execution and included in final observed identity; changing
  an already-present native product invalidates the run.

Existing logical-state oracles are retained: allocator bounded reserve then
zero trim/live allocations, exact wrong-owner/overflow counts; stem ownership
and zero teardown counts; worker missing/signal failure followed by successful
request, zero running requests and exact failure/cancellation/destruction
counts. Linux full and both daily candidate jobs execute these existing tests.
The future new-C compiler-service lifecycle/resource criterion remains an
explicit programme gap, owned by its research plan; this snapshot does not
qualify the newer compiler or claim retirement/self-hosting. Independent
Level L/new-C source/oracles and the default research option OFF are unchanged.

**Remaining qualification before activation:** publish the timeout and compatible
preparation/policy commits together; dispatch one exact-head candidate with
three isolated fresh jobs: Linux original full U (including all four
normalization and all 16 structured-flow cases), Linux separated daily and
Apple separated daily. Linux's ordinary donor also runs full ordinary
correctness. Measure ordinary preparation, native build and CTest separately
against the provisional two-hour target, without turning that target into a
kill timer. Reuse the unchanged prior full Apple product evidence; neither a
daily pass nor this local qualification is a new broad/platform pass. Expected
publication status is hotfix implementation with candidate qualification
pending, scheduled activation disabled, and no master/develop promotion. Hosted
run links own later observed outcomes; do not add a CI-status-only commit.

### Numbered acceptance criteria

1. **SCOPE-AC01 — investigation verified, repair open:** account for the
   latest run's tested SHA, failures, incomplete selection and phase costs as
   above. Close SAN-QA-020 by isolated Linux evidence and a justified repair;
   preserve SAN-QA-019's outstanding Linux verification.
2. **SCOPE-AC02 — mapping and exact membership locally verified; platform execution pending:** map native modules,
   instruction families, value/lifetime/failure scenarios, providers and
   platforms to existing tests and explicit gaps. Measure the bounded
   per-family native-coverage comparison before deciding reductions. Every
   moved family has its owner, retained counterpart, residual risk and trigger;
   preserve SAN variants and distinct dispatch/ABI/ownership paths.
3. **SCOPE-AC03 — local ownership/compatibility verified; clean platform proof pending:** from clean inputs,
   ordinary fixture preparation and the instrumented VM/provider closure
   produce compatible, traceable artifacts without stale/missing dependencies.
   Prove which native binaries execute in all selected host/subprocess paths.
   The native-tool profile retains instrumented producer tests, Linux leaks
   stay on, and full-profile selection retains the existing broad set.
4. **SCOPE-AC04 — behavioral policy and failure fixtures verified; hosted integration pending:** policy fixtures prove daily/weekly/affected-change/
   manual/tag decisions, missing-history fallback, platform/profile-qualified
   reuse, failure propagation and no false broad-success marker. No workflow
   is `continue-on-error`; incomplete execution is reported as incomplete.
5. **SCOPE-AC05 — open:** after profile approval, measure one clean execution
   on Linux and Apple, separating ordinary preparation, sanitizer build and
   runtime/native-tool scope, against the provisional two-hour target. Reuse
   unchanged valid full Apple evidence; finish outstanding Linux broad proof
   after its timeout repair. The approved qualification includes these clean candidate executions and the
   missing Linux broad proof; it does not authorize another coverage or benchmark campaign.
6. **SCOPE-AC06 — current implementation docs reconciled; final qualification report pending:** update `AGENTS.md`, the sanitizer guide, this worklist
   (including its stale CI-enforcement footer) and affected owning test docs
   coherently. Record exact qualified inputs, commands, results and limitations;
   delete task-created diagnostics. Keep existing SAN closure requirements.
7. **SCOPE-AC07 — implementation approved; activation gate open:** Adrian approved
   the coverage map, bounded measurements and exact resulting profiles on
   20 October. Qualify before activation; Adrian separately decides the master exception.
   Publication follows the authority in the implementation request. Verify
   the effective scheduled definition before claiming rollout complete.
8. **SCOPE-AC08 — existing state oracles retained/inspected; candidate execution pending:** retained direct and real-workload tests check expected
   values, errors, aliasing and resource state, including repeated requests,
   failure/recovery and teardown. Compiler/engine parity is not the sole oracle.
   Verify logical allocation/handle balance and bounded documented reserves;
   ASan/LSan alone does not satisfy the lifecycle requirement.
9. **SCOPE-AC09 — ordinary/research ownership preserved; candidate ordinary full proof pending:** full ordinary language/tool acceptance, independent
   Level L/new-C coverage, emitted-code quality and existing platform/packaging
   requirements remain intact. Record current native dependencies and optional
   untested configurations; do not infer retirement or self-hosting completion.

### Numbered implementation steps

1. **SCOPE-STEP01 — done, review only (AC01):** inspect hosted metadata and one
   temporary artifact copy, source fixture/registration, runner, existing
   policy and generated local CTest inventory. Record findings here. No
   qualification runs were launched for this review.
2. **SCOPE-STEP02 — diagnosis verified; repair qualification in progress (AC01/07):** prepare and replay the
   unchanged Linux normalization failures in isolation; diagnose before
   editing scheduling or product code. Present a separate implementation plan
   and retain applicable decision gates if product changes prove necessary.
3. **SCOPE-STEP03 — done, bounded review/measurement (AC02/03/08/09):**
   extend this source review into the native module/provider/scenario crosswalk,
   measure marginal coverage of a bounded representative/candidate panel,
   identify genuine gaps and propose exact profile membership and dependencies.
   The map, 87-cell results and exact ten-name proposal above are complete.
   Unmeasured families stay included; no full-profile or Linux qualification
   is claimed. Adrian approved the exact result on 20 October; qualification precedes activation.
4. **SCOPE-STEP04 — implemented and locally verified; platform proof in STEP05 (AC02/03/04/08):**
   add justified focused boundary/state tests, implement separate fixture and
   native-instrumentation preparation, then explicit runner profiles and
   cadence/qualification records. Check selection, dependency and policy
   fixtures. Fuzzing, TSan and optimization pilots are separately bounded
   proposals, not an automatic expansion of this delivery.
5. **SCOPE-STEP05 — normal focused proof verified; hosted candidate pending (AC01/05/08/09):** qualify affected
   normal Debug tests and independent oracles, focused maintained sanitizers,
   one measured candidate profile per platform and necessary Linux broad proof.
   Compare coverage/scenario obligations with the baseline and report gaps;
   reuse valid unchanged results.
6. **SCOPE-STEP06 — depends on STEP05 and approval (AC06/07):** reconcile
   documentation and current remote changes; prepare separate reviewable
   timeout-repair and scope-policy commits in hotfix for normal promotion to
   develop if publication is authorised. Apply the master workflow-only
   change only if explicitly approved. Report observed hosted results without
   status-only commits.

Review method: `gh run view 39366632770 --json jobs`, relevant per-job step
logs, `gh run download ... --pattern 'sanitizer-logs-*'` into a disposable
temporary directory, and `ctest --test-dir cmake-build-debug
--show-only=json-v1` (discovery only). Runtime assertions and source claims were
checked against the actual fixture, CMake declarations and Unicode guide.
Raw downloaded diagnostics are disposable and are removed after this summary;
the hosted run is the durable evidence link. Existing unrelated hotfix
`scripts/__pycache__/` and develop work remain untouched.

Follow-up review sources: the current compiler programme plan/architecture in
the develop checkout (read only), `docs/ai-context/RXVM_INTERPRETER.md`,
`CREXX_LIBS.md`, `interpreter/CMakeLists.txt`, VM tests/allocator sources,
`tests/rxpa`, plugin registration and generated CTest discovery in hotfix.
The initial source review did not execute tests. The subsequently approved
bounded measurement phase and its limits are recorded above; full profile
implementation and platform qualification remain open.

Tooling references for the proposal:
[LLVM AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
describes native instrumentation and its supported memory-error checks;
[Clang source coverage](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html)
provides function/region/branch measurements;
[libFuzzer](https://llvm.org/docs/LibFuzzer.html) describes coverage-guided
input testing and limitations for global state, persistent threads and
`dlclose` targets; and
[ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) targets
data races in a separate instrumentation configuration. These tool capabilities
inform the proposed methods; they do not establish cREXX test coverage.

## Historical assurance summaries

Status at 2026-08-23: every finding in this register is repaired and the
supported Linux ASan/LSan closure gate passes 2,363/2,363 on exact commit
`e3de72939df0dacf22c0793371233ed439227437` at
`cmake-build-debugasan/asan-logs/20260823-074630-full`. GitHub Sanitizer QA run
[32623796998](https://github.com/adesutherland/CREXX/actions/runs/32623796998)
also passes both Linux x64 ASan/LSan and macOS arm64 ASan on that commit.
SAN-001 through SAN-005 and SAN-QA-001 through SAN-QA-007 are therefore closed
as sanitizer findings.

Status at 2026-08-26: SAN-QA-008 is closed after the synchronized hotfix passed
the original broad macOS arm64 trigger locally and GitHub Sanitizer QA run
[32958593912](https://github.com/adesutherland/CREXX/actions/runs/32958593912)
passed both Linux x64 ASan/LSan and macOS arm64 ASan on final published code
revision `a744b4d2551d795cf9bbf09c46c5a3fd71e53d46`. The stronger diagnostic
remains as regression evidence; no sanitizer or product failure reproduced.

Status at 2026-08-31: SAN-QA-008 is reopened after the same typed provider
failure recurred in the macOS arm64 lane of GitHub Sanitizer QA run
[33421800264](https://github.com/adesutherland/CREXX/actions/runs/33421800264)
at `a697c4939310ec5df172a1e37608ef891bd01127`. The retained artifact contains
no ASan memory diagnostic. A concurrent Level B builder workload then exposed
a Darwin parent/child `setpgid()` race. The repair candidate gives the child
sole process-group setup ownership and retains a parallel cREXX spawn
regression. GitHub run
[33433382690](https://github.com/adesutherland/CREXX/actions/runs/33433382690)
then passed Linux ASan/LSan but reproduced the provider failure on macOS ASan;
the retained completion was again `state=2`, `errorCode=17` and contained no
memory-safety report. The provider remains parallel and now prints its bounded
failure message so another recurrence identifies the failing launch stage.

Status at 2026-09-03: SAN-QA-012 records a macOS arm64 ASan test-orchestration
failure in the scheduled Sanitizer QA gate. The failed test completed in 0.29
seconds and emitted no AddressSanitizer diagnostic. Its native-return fixture
used a fixed 50 ms scheduling window as an ordering mechanism. First hosted
qualification of that repair then exposed the same class of invalid scheduling
assumption in the CRI-17 attached-provider fixture; both deterministic handshake
repairs are under qualification on `hotfix`. The adjacent audit has now removed
every fixed 50 ms correctness observation from maintained tests.

A later production process-channel repair in `c87809d2b` is not a sanitizer
finding. Its exact three-test process panel passes ordinary Debug at
`cmake-build-debug/asan-logs/20260823-100116-ctest` and leak-enabled Linux
ASan/LSan at `cmake-build-debugasan/asan-logs/20260823-100525-ctest`; the full
ordinary Debug gate then passes 2,363/2,363 at
`cmake-build-debug/asan-logs/20260823-100548-full`.

Status at 2026-08-27: SAN-006 and SAN-007 are final-head closure candidates.
Their bounded repairs and required local macOS gates pass. They remain open and
release-blocking while GitHub runs, and become closed only if GitHub Sanitizer
QA and Build CREXX both pass for the exact published code revision. A failure
in either workflow leaves the affected item open; do not claim the current
hotfix or Release 1 line is sanitizer-clean before that condition is satisfied.

Status at 2026-09-12: all remaining registered findings are closed on exact
code revision `1dde1dd14fbc288cc0a3041adab4244d533dc528`. GitHub
[Build CREXX 34718340724](https://github.com/adesutherland/CREXX/actions/runs/34718340724),
[CodeQL 34718340684](https://github.com/adesutherland/CREXX/actions/runs/34718340684),
and [Deep Build QA 34718575156](https://github.com/adesutherland/CREXX/actions/runs/34718575156)
pass. [Sanitizer QA 34718578056](https://github.com/adesutherland/CREXX/actions/runs/34718578056)
passes 2,303/2,303 tests on both Linux x64 ASan/LSan and macOS arm64 ASan.
SAN-006 through SAN-008, SAN-QA-008 through SAN-QA-012, SAN-QA-014 and
SAN-QA-015 therefore satisfy their recorded closure conditions. SAN-QA-013
was already closed by its focused evidence.

The three nested aggregates that previously produced contention-driven timeout
failures all pass under the retained serial scheduling. On macOS, binary
imports, project build and process runtime take 467.03, 365.50 and 219.55
seconds; on Linux they take 561.59, 462.71 and 187.81 seconds. The complete
CTest phases take 3,708.62 seconds on macOS and 4,385.57 seconds on Linux. No
AddressSanitizer or LeakSanitizer diagnostic appears in either retained
artifact.

## Overnight structured-flow timeout repair — 19 October 2026

### SAN-QA-019 — structured-flow matrix exceeds its shared-pool deadline

Status: **repair locally qualified for hotfix/develop publication; overnight
Linux confirmation pending**. Owner: Codex, under Adrian's request to diagnose,
repair and publish from the existing hotfix checkout. Affected revision:
`016e6a65466c65a5c722b96452d95025acb463b2`.

**Vision and intended outcome:** restore dependable overnight sanitizer
qualification while retaining every structured-flow backend, optimizer and VM
cell and its semantic/resource-bound assertions. Correctness deadlines are hang
backstops; the test pool must not turn host contention into a product failure.
No language or runtime change is proposed.

- Trigger: [Sanitizer QA 39196944021](https://github.com/adesutherland/CREXX/actions/runs/39196944021),
  Linux x64 ASan/LSan. The full instrumented build and preparation succeed;
  `ts_llstructuredflow_ll1_noopt_rxtvm` and `ts_llstructuredflow_ll1_opt_rxtvm`
  each time out at 300.12 seconds in the eight-slot CTest pool. The other
  structured-flow cells pass, including LR1 threaded cells at 274.36 and
  227.67 seconds. CTest stops after 2,667 of 3,289 selected tests; unexecuted
  tests are not passes. The full build, preparation and CTest logs contain no
  ASan/LSan memory diagnostic or non-timeout test failure. The macOS arm64
  ASan lane passes. This is a QA finding, not a memory-safety finding.
- Original command: `tools/asan-run.sh --phase full --build-jobs 4
  --test-jobs 8 --exclude-label '^performance-measurement$' --no-live-tail`,
  Linux build/test leak detection enabled.
- Isolated unchanged controls: the exact two LL1 threaded cells pass Debug
  in 4.63/4.48 seconds and maintained Apple ASan in 18.89/18.66 seconds,
  using `--phase ctest --regex
  '^ts_llstructuredflow_ll1_(noopt|opt)_rxtvm$' --test-jobs 1 --leaks off
  --no-live-tail` after building their two prep targets and `rxtvm` through the
  runner. Initial local CTest found missing generated fixtures; preparation
  resolved that setup issue before measurement. Apple leak detection is
  unsupported; this is not Linux LSan proof.
- Cause and repair: all 16 structured-flow cells inherited the generic
  300-second parallel policy despite their CPU-intensive cyclic-analysis,
  path-control and exact resource-boundary matrix. They now use `RUN_SERIAL`
  and a 900-second hang backstop across all four backends, both optimizer
  modes and both VMs. Hosted passing macOS Earley cells already took
  257.76–267.47 seconds under contention. Condition tests retain their existing
  scheduling; no source, oracle, sanitizer option or coverage is weakened.
- The same hosted macOS job took 239.5 minutes against a 240-minute outer
  limit (CTest alone: 11,026.81 seconds). The workflow hang backstop becomes
  360 minutes to retain practical headroom when the matrix runs in isolation;
  build/test scope and platform leak policy remain unchanged.
  Scheduled jobs load their workflow definition from default `master` and
  check out the selected `develop` source: the CTest repair applies on the next
  overnight run, while the outer-limit change takes effect when the workflow
  definition reaches `master` through the normal release promotion. This
  hotfix does not update the release branch or claim that limit is already live
  for scheduled jobs.

**Numbered acceptance criteria**

1. **SQA19-AC01:** replay the two unchanged failing cells once in isolation
   after preparing their artifacts, in normal Debug and maintained Apple ASan;
   record elapsed times, semantic results and sanitizer capability limits.
2. **SQA19-AC02:** set explicit scheduling and a generous hang backstop for the
   affected matrix and outer workflow from isolated measurements and hosted
   failure evidence.
   Inspect generated CTest properties and retain every existing test/oracle.
3. **SQA19-AC03:** pass the affected matrix in Debug and maintained ASan after
   the scheduling repair, with unchanged product/test-source inputs. A working
   core product build must also pass. No broad suite rerun is required for a
   scheduling-only repair without a sanitizer finding.
4. **SQA19-AC04:** summarize exact inputs, results and limitations here, publish
   the repair to hotfix and promote it to develop while preserving unrelated
   local changes. Hosted success remains an observed workflow result.

**Numbered implementation steps**

1. **SQA19-STEP01 (AC01):** inspect original logs, prepare the two LL1 fixtures
   and run the unchanged isolated Debug control followed by Apple ASan.
2. **SQA19-STEP02 (AC02):** use the measured costs to repair CTest scheduling
   and timeout declarations in `langtools/tests/CMakeLists.txt`.
3. **SQA19-STEP03 (AC03):** prepare/run the complete structured-flow matrix in
   Debug then Apple ASan; inspect registration and core-build results.
4. **SQA19-STEP04 (AC04):** update this disposition, remove temporary logs,
   commit, push hotfix and fast-forward develop. Let normal overnight Linux
   ASan/LSan assurance establish hosted confirmation without claiming it early.

**Qualification and publication disposition, 19 October 2026**

- **AC01 / STEP01 verified:** unchanged isolated controls pass as recorded
  above. Local timings are diagnostic observations on macOS arm64 with ten
  logical CPUs, not formal performance results.
- **AC02 / STEP02 verified:** generated CTest JSON in both trees retains all
  16 structured-flow cells with `RUN_SERIAL=true`, `TIMEOUT=900` and the
  original `failures=0` oracle; all 16 condition cells retain their original
  300-second parallel registration. `actionlint .github/workflows/sanitizers.yml`
  passes. CMake input SHA-256:
  `4026821d20de6703e300caa613b2da5eac75f4438071c4185914da59021ca5de`.
- **AC03 / STEP03 verified:** runner build of all eight
  `ts_llstructuredflow_<backend>_<mode>` prep targets passes in Debug and
  maintained Apple ASan. Debug core targets `rxc`, `rxc-c`, `rxas`, `rxlink`
  and `rxvm` pass without warnings. The exact final CTest shape is
  `tools/asan-run.sh --build-dir <tree> --phase ctest --regex
  '^ts_llstructuredflow_' --test-jobs 8 --leaks off --no-live-tail`:
  Debug passes 16/16 in 90.40 seconds; Apple ASan passes 16/16 in 364.75
  seconds, with no address-sanitizer diagnostic. ASan build leak detection is
  also off because Apple LSan is unsupported; its rebuild emits existing
  Apple `sprintf` deprecation warnings. No supported Linux leak policy changes.
- Qualified product/test-source identity is the affected revision above;
  only CTest scheduling, the outer workflow guard and these owning documents
  change. Documentation and history-only promotion do not invalidate the
  retained results. Temporary diagnostic logs are deleted after this summary.
- **AC04 / STEP04 publication disposition:** publish the commit containing
  this repair on `hotfix` and promote the same commit to `develop`, preserving
  unrelated local work. Normal automatic build/smoke, optimizer-parity and
  CodeQL workflows are expected to pass; only their hosted results establish
  that outcome. No full hosted sanitizer repeat is dispatched for this
  scheduling-only repair. The next scheduled Linux ASan/LSan run owns hosted
  confirmation; Codex/Adrian retain SAN-QA-019 for diagnosis if it recurs.

## Mainframe ADDRESS qualification — 11 October 2026

SAN-013 is closed on 12 October after focused repair and the full supported
platform gates. Its trigger, repair and exact evidence are retained under
[Closed campaign findings](#closed-campaign-findings). Native-provider and
PDOS guest qualification remains separately recorded.

## Current overnight regression — 10 October 2026

### SAN-012 — detached source anchors retain semantic annotation allocations

Status: **closed, 2026-10-11**; focused, original-trigger and both full platform
gates pass as recorded in the [closure disposition below](#11-october-closure-disposition).
Owner: **Codex**. Affected inputs: C-first split/metadata-reader
candidate based on `91313bcc5a447e9403f95b1f0bfc4dd9503d0618`.
On 2026-10-10 Adrian initially assigned SAN-011 and SAN-012 closure to the next
Release 1 release-QA gate, then explicitly requested full sanitizer and Deep
QA immediately. The completed proof below fulfills that renewed request.

- Original trigger: Linux `tools/asan-run.sh --build-dir build-asan-clang
  --phase full --build-jobs 4 --test-jobs 8 --exclude-label
  '^performance-measurement$' --no-live-tail`, leaks enabled in build and tests.
  Run `20261010-155050-full` stops during class-library generation with
  2,628 leaked bytes in 32 allocations rooted in `source_semantic_f`, copied
  type/dimension strings and `source_tree_sync_semantics`.
- Diagnostic mechanism and scope: [historical san-012-original.log](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/classic-frontend-split-2026-10-10/san-012-original.log).
  Full retained Linux log: `/tmp/crexx-san012-linux-original-build.log`.
- Cause: semantic cleanup walks the visible SourceNode tree. Owned detached
  metadata anchors are on the Context's allocation list and can receive
  annotations without becoming children of that visible tree.
- Repair: `source_tree_clear_semantics` releases annotations for every
  Context-owned SourceNode. Permanent `compiler_contract_builder` assertions
  create an anchor detached from the visible tree, annotate it, clear/resync
  semantics and destroy the receiving Context. This allocation cleanup repair
  is distinct from SAN-011's borrowed-anchor use-after-free.
- Focused proof: final normal Debug passes at
  `/tmp/crexx-san012-debug-regression.DpMWbE`; maintained Apple ASan
  `20261010-171147-ctest` and Linux ASan/LSan `20261010-161146-ctest` each pass
  `compiler_contract_builder` 1/1. Linux keeps leaks enabled. Apple LSan is
  unsupported. The [historical local receipt](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/classic-frontend-split-2026-10-10/README.md)
  retains hashes, final code/test/build identity and the final 84-test controls.
- Full Apple `20261010-171343-full` passed build/preparation and was stopped at
  CTest; Linux `20261010-161345-full` was stopped during preparation after
  Adrian's later-gate assignment. Neither is completed full-gate evidence.
- Closure: the permanent focused regression and its unchanged repair inputs
  match the retained Debug/Apple/Linux evidence. Fresh hosted full builds
  execute the original class-library generators with Linux build leaks on;
  both complete platform gates pass. No suppression or exclusion is introduced.

### SAN-011 — imported metadata declaration source anchors outlive their Context

Status: **closed, 2026-10-11**; focused, original-trigger and both full platform
gates pass as recorded in the [closure disposition below](#11-october-closure-disposition).
Owner: **Codex**. Affected inputs: C-first split/metadata-reader
candidate based on `91313bcc5a447e9403f95b1f0bfc4dd9503d0618`.
The initial later-gate assignment above was followed by Adrian's explicit
request for immediate full sanitizer and Deep QA. That proof is now complete.

- Original trigger: `tools/asan-run.sh --phase build --build-target
  qa-measure-classic-frontend --build-jobs 4 --build-leaks off --leaks off
  --no-live-tail`, maintained Apple ASan run `20261010-160024-build`.
  Runtime `_rxsystem` compilation reports heap-use-after-free at
  `source_tree_clear_context_links`, `rxcp_source_tree.c:39`, during teardown.
  Apple LSan is unsupported; leaks-off is that capability limit.
- Retained full log: `cmake-build-debugasan/asan-logs/20261010-160024-build/build.log`.
  Diagnostic mechanism and scope: [historical san-011-original.log](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/classic-frontend-split-2026-10-10/san-011-original.log).
- Cause: source contract anchors are allocated in the declaration Context;
  AST duplication borrows that SourceNode into other contexts. Class-registry
  teardown can free its owner before another Context clears those links.
- Repair: AST duplication and generated-node anchor transfer copy metadata
  anchors into each receiving Context, preserving real source identity and
  leaving unsupported coordinates unknown. Permanent `compiler_contract_builder`
  assertions free the declaration owner first, then consumer/generated contexts.
- Focused proof: normal Debug and maintained Apple ASan
  `20261010-161340-ctest` pass the permanent regression. Original runtime-library
  build and the 98-command parity/ownership target pass at
  `20261010-161341-build`; the expanded 109-command target passes at
  `20261010-164844-build`. The final regression, extended for SAN-012, passes
  Apple ASan `20261010-171147-ctest` and Linux ASan/LSan
  `20261010-161146-ctest`. The [historical receipt](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/classic-frontend-split-2026-10-10/README.md)
  records final-input coverage and the intentionally stopped full runs.
- Closure: the unchanged owner-first teardown regression passes in the retained
  focused Debug/Apple/Linux commands and both hosted full gates. Fresh hosted
  instrumented builds regenerate `_rxsystem`, the original broader trigger.
  No suppression or exclusion is introduced. This closes the finding; it does
  not designate a new release candidate or release.

### SAN-QA-018 — Classic whole-number BIF operands round through binary float

Status: **closed, 2026-10-11**; focused, original-trigger and both full platform
gates pass as recorded in the [closure disposition below](#11-october-closure-disposition).
Owner: Codex, GitHub pipeline
diagnosis/repair requested by Adrian on 2026-10-10. Affected revision:
`10e8184e39ad28e62178055473249925f9db82a7`.
On 2026-10-10 Adrian initially authorized publication with later Release 1
release-QA proof, then explicitly requested immediate full sanitizer and Deep
QA. The completed proof below fulfills that renewed request.

- Scheduled [Sanitizer QA 38022249457](https://github.com/adesutherland/CREXX/actions/runs/38022249457),
  Linux x64 ASan/LSan, fails all four `levelc_bif_integer_limits` modes at
  `ARG(9223372036854775807)`. The maintained VM reports `CONVERSION_ERROR`
  in `RexxClassicBifArg.crexx:58`; no ASan/LSan memory diagnostic appears in
  the full retained build, prep or CTest logs.
- Original trigger: `tools/asan-run.sh --phase full --build-jobs 4 --test-jobs 8
  --exclude-label '^performance-measurement$' --no-live-tail`, with Linux
  build/test leaks on. The gate stops after the four functional failures;
  remaining unexecuted tests are not passes.
- Retained log: `/tmp/crexx-san-artifacts.0TnyOD/20261010-035546-full/ctest.log`,
  lines 3172–3228. Permanent evidence is owned by the
  [pipeline repair plan](planning/release-1/ci-core-stream-packaging-2026-10-10.md).
- Root cause: a validated canonical whole-number string is consumed through
  `+ 0`, emitting `stof`, `fadd`, `ftoi`. Binary64 rounds INT64_MAX to 2^63.
  Linux's checked float-to-integer range test rejects it. The direct integer
  conversion must preserve validated whole-number operands exactly.
- Permanent reproducer: `compiler/tests/rexx_src/levelc_bif_integer_limits.rexx`,
  in optimized/no-opt direct/linked modes. Preserve existing output and authored
  Classic range-error checks.
- Repair: 54 validated whole-number consumers in 24 Classic sources use direct
  checked `as .int`, including stream position conversion before subtracting
  one. `classic_unicode_contract.py` additionally checks a zero-length read
  and omitted-text output positioned at INT64_MAX without allocating padding.
  All complete RexxDoc blocks remain unchanged.
- Normal Debug BIF correctness passes 169/169 (294.94 seconds). Generated Debug
  and Release ARG procedures use `stoi` rather than the lossy float sequence.
  The [historical development receipt](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/ci-core-stream-packaging-2026-10-10/README.md)
  owns exact final-input focused normal/maintained-ASan evidence.
- Closure: the retained matching six-test Debug/Apple-ASan panel and all four
  permanent integer-limit modes pass. Both fresh full sanitizer gates build
  the repaired runtime and execute every mode plus the exact INT64_MAX stream
  positions in `classic_unicode_contract_opt`. Linux keeps build/test leaks on.
  No suppression or exclusion is introduced.

### 11 October closure disposition

SAN-011, SAN-012 and SAN-QA-018 satisfy all four closure requirements in
[`CREXX_ASAN_TESTING.md`](ai-context/CREXX_ASAN_TESTING.md). Original failing
diagnostics and permanent regressions remain above; the prior local receipts
retain their historical qualification boundary. Their focused logs were
reread and their recorded hashes verified, without repeating valid tests.
`rxcp_ast_core.c`, `rxcp_source_tree.c`, `test_contract_builder.c` and every
repaired Classic integer source retain the focused qualification hashes.
Later build-stage changes are exercised by the fresh full gates.

[Sanitizer QA 38086283792](https://github.com/adesutherland/CREXX/actions/runs/38086283792)
passes on `a69745746117ab01df47ae24d47793f397962488`:

| Platform | Fresh full build and preparation | Complete CTest | Leak capability |
| --- | --- | --- | --- |
| Linux x64 ASan/LSan | Pass, including `_rxsystem` and class-library generators | 3,122/3,122; 9,653.00 s | Build and test leaks **on** |
| macOS arm64 ASan | Pass, including `_rxsystem` and class-library generators | 3,126/3,126; 9,410.31 s | Apple LSan unavailable; covered by Linux |

Both logs explicitly pass `compiler_contract_builder`, all four
`levelc_bif_integer_limits` modes and `classic_unicode_contract_opt`. Neither
platform's complete build, QA preparation or CTest logs contains an ASan/LSan
finding or failed/timed-out test. Only the established `performance-measurement`
label is excluded; the core gate uses `ENABLE_LLAMA=OFF` and does not claim
upstream inference-engine coverage.

Full downloaded logs are retained under
`output/ci-regression-20261010.79vob9/`, with inspected line references and hashes
in `linux-full-sanitizer-pass-evidence.json` and
`macos-full-sanitizer-pass-evidence.json`. Artifact digests are
`175eab694d4c0eea55e9c40b6e3a42a6eded825e3822ab2485c156b6e6d8c650`
(Linux) and `90d847e0a26cf0ea6a43a8ad183af57601aa00c1c840af6e779411911a5c655a`
(macOS). Linux build/preparation/CTest log hashes are
`2f8d4cc3c5a5586f67073a7d653a52c751223f276bbd8363464847c29983c574`,
`bdf51afc62724883d2fa12042536e559b371db3301db7db6d72e9d3e3aba73f5`,
`e14b1d578f10b9e5f969d05330d15ab10f2bd4ba5eb10e9493c588effc163c32`;
macOS hashes are
`a4c8d55496ce6bf32820d278ec33e111dab4ee1fab7bc12a467961a0faf87715`,
`9cbce0ad83c92db5a584412ff546c53eefe4036906c233a5fcb3e4194f6caac0`,
`f40017cdb27b69d8f166e3c74f7253fb03dd3eddfdd28d4730b93816158699a6`.

Final product revision `5e8275768355fa3dc84c24e815de91d6d63a6ca3` changes only
Windows execution of the broker and its process manifest, plus the separately
qualified portable quoting helper. The
[historical Windows host-text receipt](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/ci-windows-hosttext-2026-10-10/README.md) proves
byte-identical Unix broker preprocessing and unchanged existing QA selections;
the helper passes nine vectors in Debug and maintained Apple ASan. This
retains the full Unix proof for the final product inputs. Both Windows full
comprehensive/qualification lanes also pass, and normal Build/CodeQL are green.

This is the required finding disposition, not a new product change. Its
documentation-only publication reuses these exact qualified inputs; no broad
gate is repeated to close the register. The unchanged non-documentation Git
tree inventory has SHA-256
`bd7d85c27b3770f0a3877303970d5525cdb86a15796df3685f434bb38b9426be`.
The pipeline plan owns final delivery acceptance. No release is cut here.

## Previous overnight regression — 9 October 2026

### SAN-010 — frame-control AST unit retains compiler-owned allocations

Status: closed 2026-10-09. Owner: Codex, overnight QA repair/publication task requested by
Adrian on 2026-10-09. Affected candidate: develop `0aafdb155b07d2a345193a870cc47922e1e837c0`;
the local Level C closeout `b1b8bed2d` still needs the same cleanup repair.

- Observed first-party Linux LeakSanitizer finding: 6,831 bytes in 52
  allocations, all rooted in `compiler/tests/test_frame_control_ast.c` AST,
  scopes, symbol metadata and emitted output fragments. The fixture creates a
  stack Context and never releases its compiler-owned objects.
- Original trigger: scheduled [Sanitizer QA 37881640063](https://github.com/adesutherland/CREXX/actions/runs/37881640063),
  Linux x64 ASan/LSan job 113662450534, maintained runner `tools/asan-run.sh
  --phase full --build-jobs 4 --test-jobs 8 --exclude-label '^performance-measurement$'`
  with build/test `detect_leaks=1`. The run stops at `frame_control_ast`;
  unexecuted cases are not passes. The macOS ASan lane passes but cannot supply
  LSan proof.
- The original diagnostic is summarized above; its temporary capture has been deleted.
- Permanent reproducer/closure test: existing CTest `frame_control_ast`. Repair
  must release output/AST/symbol/scope/flow ownership through canonical Context
  cleanup, preserving all frame-node emission assertions and source bounds.
- Repair `2e8c820c6` uses `cntx_f`, an owned source buffer and `fre_cntx`.
  All original frame emission and source assertions remain. Normal Debug and
  Apple ASan focused frame checks pass; Apple LSan is unsupported.
- Matching supported Linux focused ASan/LSan passes with leaks on in
  `build-asan-clang/asan-logs/20261009-090042-ctest`. Final full supported
  Ubuntu ARM64/Clang 18 ASan/LSan passes **3,116/3,116** (5,010.29 seconds)
  at `build-asan-clang/asan-logs/20261009-101427-full`, with build/test leaks on
  and only the maintained performance-measurement exclusion. Build/prep/test
  logs contain no sanitizer report. The first attempt stopped on an ordinary
  DB-decimal rounding defect, repaired in `1ac5995c6`; the complete gate was
  rerun after that production input change without using partial passes as an
  exclusion list. No suppression or unsupported leak-off claim was used.
- Full normal Debug passes 3,320/3,320 with affected DB consumers requalified
  after their repair. [historical Qualification receipt](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/overnight-2026-10-09/qualification.json)
  and [historical full supported summary](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/overnight-2026-10-09/linux-full/summary.log)
  identify final input fingerprints, commands, platform and retained full logs.
  This closes SAN-010, not wider Release 1 platform or language compatibility.

## Native inference integration finding

Status at 2026-09-16: SAN-009 is closed within the approved first-party scope.
The platform and packaging acceptance criteria remain separate release gates.

### SAN-009 — CPU backend probe unload/reload re-registers Apple-ASan globals

Status: closed 2026-09-16. Discovered
2026-09-14 during STEP-03. Current owner: Codex, STEP-06 qualification. Adrian's
initial direction was to diagnose/fix SAN-009 while he reviewed S3-D01, but
**do not rerun the sanitizer yet**. That historical hold lasted until the
STEP-06 approval below. Required platform proof remains open. Adrian approved
STEP-03 closure and the named STEP-06
native-inference release-QA handoff, owned by Codex under his direction.
The historical pending statements below are superseded by this closure record;
no sanitizer suppression was used.

- **Closure, 2026-09-16:** the permanent original/reversion reproducer, matching
  Debug/Apple-ASan regression, original four-tool lifecycle trigger and full
  local 2,349-test Apple-ASan gate are retained below. The separate Linux
  first-party probe passes Debug and ASan/LSan at `9152850e8` with ordinary,
  unchanged engine libraries and leak detection enabled. Broad core Sanitizer
  QA [35026470003](https://github.com/adesutherland/CREXX/actions/runs/35026470003)
  at `21666b6bcf0c5a14986652ef0b0a76bddc43611b` now passes both platforms:
  Linux 2,340/2,340 tests (4,493.34 s, build/test leaks on), Apple 2,340/2,340
  (4,506.68 s, unsupported Apple LSan off). Build, preparation and CTest logs
  contain no sanitizer diagnostic. Retained logs/settings/job identities are
  under [historical capture](https://github.com/adesutherland/CREXX/tree/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/native-inference-ci).
  The bridge/CPU residency repair is unchanged; intervening Windows packaging
  and helper-variable fixes do not alter it. This closes SAN-009, not overall
  native-inference delivery or real-GPU qualification.

- **Supported first-party probe, 2026-09-15:** isolated Linux run
  `35027428540` at `9152850e8` passes the permanent probe in normal Debug and
  ASan/LSan with `detect_leaks=1`. It rebuilds only the current first-party
  bridge, SHA256 support and probe harness. The ordinary engine/backend files
  come from the verified `76df02be3` release-layout archive; symbol inspection
  and unchanged file hashes confirm they were neither rebuilt nor instrumented.
  Build graphs, identities and maintained-runner logs are retained under
  [`docs/qa/native-inference-ci/remote/9152850e8/first-party-probe/`](https://github.com/adesutherland/CREXX/tree/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-ci/remote/9152850e8/first-party-probe).
  The equivalent local ARM control passes with Apple's documented leak limit.
  The subsequent CI-F11 change only renames the helper's private co-residency
  variable; it does not change the probe entry point or bridge implementation.
  Full core Linux ASan/LSan `35026470003` is still pending, so the item stays open.

- **First-party scope clarification, 2026-09-15:** Adrian explicitly limits
  the maintained sanitizer programme to cREXX code, excluding upstream
  llama.cpp/CUDA qualification. Hosted core jobs now build without llama;
  adapter ownership checks are a separate gate and may link an ordinary
  uninstrumented engine. Reuse the retained instrumented Apple regression
  evidence above/below rather than repeating upstream builds. Supported Linux
  first-party ASan/LSan closure evidence is still required; a core-only pass
  does not close this provider finding. Resume it after the approved four-core
  gate. This is no suppression or waiver of the first-party defect.

- **STEP-06 commencement, 2026-09-15:** Adrian approved the completed STEP-07
  documentation/examples and the planned transition to full QA. The earlier
  sanitizer hold is lifted for the named STEP-06 qualification gate, owned by
  Codex under Adrian. Follow matching normal controls, the maintained runner
  and every closure condition below; this approval does not close SAN-009.
- **Focused STEP-06 proof, 2026-09-15:** current `rxllama_backend_probe_cycle`
  and `rxpa_host_text_services` pass together in normal Debug (2/2, 0.74 s)
  and Apple ASan (2/2, 1.80 s). Sustained generation CPU/Metal controls also
  pass under ASan. [historical Current evidence](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/native-inference-step06/README.md)
  retains commands/logs. The expanded package/native matrix now passes Apple
  ASan (385.991 s, `20260915-150153-build`), including native foreign-handle
  rejection and four-worker BGE/Smol CPU/Metal ownership after S3-D01. Installed
  typed embedding and generation matrices also pass. The original broader
  lifecycle workload passes as `rxllama_toolchain_lifecycle` (7.25 s) in
  `20260915-150827-full`; this CTest invokes the same `step03_toolchain.cmake`
  CPU/BGE compile/assemble/link/both-VM workload as the original measurement
  target, in a separate output directory. The full local build/preparation and
  2,349/2,349 Apple-ASan CTests pass (CTest 2,278.82 s), with no sanitizer
  diagnostic in retained build, preparation or test logs. See
  [full local result](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-step06/full-local/summary.json).
  Supported Linux ASan/LSan remains outstanding.
  The item stays open.

- **Hosted cold-start guard, 2026-09-15:** the probe also performs full backend
  initialization, including Metal library creation. Widen its serial CTest hang
  backstop from 30 to 1,800 seconds after observing a roughly 70-second cold
  Intel consumer in the package campaign. The unchanged permanent probe passes
  normal Debug (0.66 s) and Apple ASan (1.02 s); see
  [historical pipeline guard evidence](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/native-inference-ci/README.md#cold-backend-initialization-guard).
  This is not a suppression or an explanation of the separate original Intel
  engine stall. SAN-009 still requires supported Linux ASan/LSan closure.

- Affected revision: baseline `c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8`
  plus the uncommitted STEP-03 provider implementation.
- Original trigger: `tools/asan-run.sh --build-dir cmake-build-debugasan
  --phase build --build-target rxllama_toolchain_measure --build-leaks off`,
  with `ENABLE_LLAMA=ON`, the pinned archive and explicit test models. The first
  rxbvm lifecycle reports an ODR violation for `ggml_arm_arch_features` while
  the bridge probes the packaged CPU backend.
- Retained [original report](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-step03/asan-odr-original.log).
  The scanner also copied the compiler's sanitizer runtime into the package.
  Excluding/removing that copy did **not** repair the reproducer. That initial
  hypothesis is not the cause established by the controlled experiment.
- Reproduced cause: first-party `cpu_score()` calls `dlopen`, probes capabilities,
  then `dlclose`; the subsequent load re-registers instrumented Mach-O globals.
  The permanent `rxllama_backend_probe_cycle` control fails with the original
  close behavior restored and passes when probe references remain resident.
  The controlled reversion changed only the Unix probe close operation, then
  restored the repaired source. See
  [permanent regression before repair](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-step03/asan-probe-cycle-red.log).
- Repair: retain probe DSOs for process lifetime, skip an unnecessary feature
  probe when only one CPU variant is packaged, and follow pinned upstream's
  process-lifetime backend-DSO policy. No engine inference mathematics changed.
  Upstream `ggml/src/ggml-backend-reg.cpp` explicitly retains its loaded DSOs
  because backend background threads can outlive registry destruction.
  Instrumentation runtimes are independently excluded from product bundles.
- Before the S3-D01 VM edit, matching Debug/Apple-ASan probe, six CPU/Metal bridge controls and
  four-tool lifecycle all pass (8/8 in each build). The full scratch-install,
  native relocation and dynamic-worker matrices also pass in both builds.
  The existing native-project contract passes in both builds. See
  [historical retained evidence index](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/native-inference-step03/README.md).
  These observations do not close broader platform gates.
- Follow-up review, 2026-09-14, after Adrian's sanitizer hold: inspected
  `cpu_score()`, engine cleanup, the permanent probe-cycle control, the retained
  original/reversion failures and pinned upstream registry destruction. The
  repair was already present in the working tree; no further source correction
  is needed for the reproduced failure. All 20 source/test/build-input hashes
  and both recorded build configurations match `source-and-configuration.json`;
  all 99 retained evidence checksums passed before this documentation update.
  Existing test results above predate the hold and are reused, not new runs.
  No build or test was launched, no interpreter source changed, and S3-D01
  remains under review. Backend-library residency is process-wide; model and
  private-context allocations still follow their ordinary release paths.
- Adrian subsequently approved S3-D01 and accepted its measured per-legacy-call
  cost. The expanded native/dynamic model-worker matrix and all 2,314
  non-measurement CTests pass in normal Debug;
  the earlier sanitizer evidence predates that VM repair and matrix expansion.
  Adrian subsequently approved STEP-03 closure and assigned the remaining
  proof to STEP-06 native-inference release QA, owned by Codex under his
  direction. Include S3-D01, the expanded native matrix and its isolated sanitizer
  scheduling measurement alongside the SAN-009 regression and broad platform
  gates. The sanitizer hold remains until STEP-06; SAN-009 is still open.
- Closure requires the permanent regression, matching normal/maintained sanitizer
  checks, the original trigger and applicable broad platform gates in
  [the sanitizer guide](ai-context/CREXX_ASAN_TESTING.md). Apple LSan is unsupported;
  supported Linux ASan/LSan remains required. STEP-06 is the explicitly approved named gate.  No release-ready or sanitizer-clean claim.

## Findings closed on 2026-09-12

### SAN-008 — array size shorthand reads beyond an imported array's bounds table

Status: closed on exact code revision
`1dde1dd14fbc288cc0a3041adab4244d533dc528`; GitHub Build CREXX and both
Sanitizer QA platforms pass, including the permanent #694 regression.

- Affected revision: `fa887be158213403b9b1de458cfdecc7e4c86122` plus the local
  #694 import repair. The unchecked access in `set_node_types_walker()` also
  exists in that base revision; the new permanent import matrix exposes it.
- Reproducer: import the `binary_global_import_types` provider with
  `grid = .int[-2 to 2, 1 to 3]`, then compile a consumer that reads
  `grid[-2, 1, 1]`. The normal Debug compiler returns
  `ARRAY_DIMS_MISMATCH`; maintained Apple ASan first reports a four-byte
  heap-buffer-overflow just beyond the two-element `dim_base` allocation.
- Original maintained command: `tools/asan-run.sh --build-dir
  cmake-build-debugasan --phase ctest --regex
  '^(binary_global_import_types|global_import_types|inline_return_conversions|binary_forward_dependencies|binary_storage_selector_dependencies|crexx_process_runtime)$'
  --test-jobs 2 --leaks off --no-live-tail`. Leak detection is unavailable
  on Apple ASan; no supported sanitizer has been disabled.
- Retained log:
  `cmake-build-debugasan/asan-logs/20260912-150240-ctest/ctest.log`.
  The report names `rxcp_val_type.c:1788` in `set_node_types_walker()` and
  the allocation in `node_to_type()` via `sym_imva()`.
- Cause: the earlier type-setting pass checks the last subscript's base for
  the literal-zero array-size shorthand before validating that the subscript
  ordinal exists. Guarding the later type-safety and optimization passes
  alone does not cover this earlier read.
- Repair: retain the signed `ast_chdi()` result, require it to be non-negative,
  cast only after that check, and require it to be below the `size_t`
  dimension count before reading `dim_base`. The permanent matrix includes
  both extra final-one and final-zero cases, so the size shorthand cannot hide
  an excess dimension. Existing valid one-based final-zero queries retain
  their rewrite.
- Local proof: the final normal-Debug focused panel passed 14/14 at
  `cmake-build-debug/asan-logs/20260912-154901-ctest`; the identical maintained
  Apple-ASan trigger passed 6/6 at
  `cmake-build-debugasan/asan-logs/20260912-155610-ctest`; Release passed the
  binary import matrix at
  `cmake-build-release/asan-logs/20260912-160205-ctest`; and normal Debug
  `qa-prep` plus all 2,302 non-performance tests passed at
  `cmake-build-debug/asan-logs/20260912-160326-build` and
  `cmake-build-debug/asan-logs/20260912-160637-ctest`. Apple ASan provides no
  leak-closure authority. At candidate time both exact-commit hosted sanitizer
  lanes remained required; run 34718578056 now supplies them.
- Permanent closure test: `binary_global_import_types`, including excess
  final subscripts of both one and zero, source/RXAS/RXBIN/shared-pool imports,
  and both compiler modes. Required closure is the identical normal Debug
  and maintained ASan focused command passing, then the full supported
  platform sanitizer gates and ordinary Debug correctness QA. No suppression,
  test exclusion, or waiver is authorized.

### SAN-QA-015 — PTY restoration observation races Darwin pending-input state

Status: closed on exact code revision
`1dde1dd14fbc288cc0a3041adab4244d533dc528`; Build CREXX, Deep Build QA and
both Sanitizer QA platforms pass. This was a shared maintained test-harness
failure, not an ASan memory diagnostic.

- Affected revision: `d9982d0004fc712849366419eab10a26debe8cd8`.
  The full Debug correctness run passed 2,285/2,286, failing only
  `ui_ansi_host_edges_opt` with `terminal modes not restored`. Retained log:
  `cmake-build-debug/asan-logs/20260909-065711-ctest/ctest.log`.
- Reproducer: eight independent processes each repeat the actual optimized
  ANSI host-edge image against the existing `Terminal` harness 100 times.
  Four processes reproduced the failure; all before/after fields were identical
  except `c_lflag ^ before = 0x20000000`, Darwin's `PENDIN` state. The retained
  diagnostic is `/tmp/crexx-ci-20260909.wjmUik/ansi-edges-parallel.log`.
  A 200-run serial control passed, as did a 200-run shell-only control.
- Cause: Darwin sets `PENDIN` when raw input returns to canonical mode and
  clears it on a terminal read. The harness observes the child's exit marker
  before its holding shell is guaranteed to have performed that read.
- Repair: complete a newline/read/acknowledgement handshake with the holding
  shell before comparing terminal attributes. The complete equality check,
  alternate-screen assertion and cursor assertion remain unchanged.
- Permanent regression surface: `ui_ansi_host_edges_{noopt,opt}`,
  `rxconsole_pty` and `text_inspector_ansi_pty` retain their real terminal
  conversations. Hotfix QA owns focused Debug and Apple-ASan validation,
  concurrent replay and hosted qualification before closure. The unaffected
  2,282 tests from the completed broad run need no repetition for this
  Python-harness-only edit.
- Local qualification: all four affected tests pass in normal Debug
  (`/tmp/crexx-ci-20260909.wjmUik/pty-fixed-debug.log`) and maintained Apple
  ASan (`cmake-build-debugasan/asan-logs/20260909-071031-ctest`). The same
  eight-process panel passes all 800 repetitions after repair at
  `/tmp/crexx-ci-20260909.wjmUik/ansi-edges-parallel-fixed.log`. A negative
  control deliberately leaving ECHO disabled still fails the exact attribute
  assertion (`pty-negative-control.log`). The current exact-head hosted gates
  supply the final proof.

### SAN-QA-014 — RXPP loses macros outside the input-source directory

Status: closed on exact code revision
`1dde1dd14fbc288cc0a3041adab4244d533dc528`; Build CREXX, Deep Build QA and
both Sanitizer QA platforms pass.

- Affected revision: `82730b495c07a6fb1bc34ffe4e9ad8b33aba50d3`.
  `CMD_loadMacro` changed the relative `##LOADMACRO` root from the selected
  macro-library directory to the input-source directory. Text Inspector stages
  its UI macros separately, so its generated application loses the macro bodies.
- Evidence: Sanitizer QA [34308566325](https://github.com/adesutherland/CREXX/actions/runs/34308566325),
  Build CREXX [34211281347](https://github.com/adesutherland/CREXX/actions/runs/34211281347),
  and Deep Build QA [34303936426](https://github.com/adesutherland/CREXX/actions/runs/34303936426).
  Full downloaded logs are retained at `/tmp/crexx-ci-20260909.wjmUik/`.
- Permanent reproducer: `rxpp_loadmacro` now separates the input and selected
  macro-library roots, with a conflicting source-relative package. The earlier
  fixture placed both roots in the same directory and could not detect this.
- Original broad trigger: maintained full Linux ASan/LSan and macOS ASan;
  Text Inspector generation, core, TUI and ANSI PTY coverage. The macOS PTY
  also recorded exit 139 without an ASan stack; retain this as unqualified
  until the complete repaired application passes the maintained lane.
- Closure evidence: hotfix QA restored the documented macro-library lookup and
  passed focused Debug/ASan plus the current full hosted Build, Deep Build and
  Sanitizer QA gates.
- Repair: the documented macro-library lookup is restored. The strengthened
  fixture fails before repair and passes after it; all six Text Inspector
  tests pass in normal Debug. The same code now passes focused Apple-ASan and
  the complete hosted gates above. Investigation and evidence are in
  [the hotfix report](planning/beta-3/reports/ci-hotfix-2026-09-09.md).

### SAN-007 — imported inline-payload AST freed during recursive function replacement

Status: closed on exact code revision
`1dde1dd14fbc288cc0a3041adab4244d533dc528`; Build CREXX and both Sanitizer
QA platforms pass. The ownership repair and permanent regression also retain
their focused normal Debug and Apple-ASan evidence. No suppression, exclusion
or waiver was used.

- Scope: compiler imported-function replacement and inline-payload attachment
  during recursive source-import validation.
- Failure: `symbol_has_initializer_definition()` reads a freed imported inline
  metadata AST node at `rxcp_val_sym.c:123`. `add_func()` frees the former
  imported function context through `freimpfc()` while the active validation
  tree still retains an initializer node allocated by
  `inline_meta_import_node()` and attached by
  `rxcp_inline_attach_imported_symbol()`.
- Original trigger: `tools/asan-run.sh --phase full --test-jobs 8
  --build-leaks off --leaks off --no-live-tail --tail-lines 50` on macOS arm64.
  The instrumented build fails while generating
  `lib/rxfnsg/rexx/httpcore.rxbin` from `httpcore.crexx`.
- Focused reproducer: `tools/asan-run.sh --phase build --build-leaks off
  --build-target lib/rxfnsg/rexx/httpcore.rxbin --build-jobs 1`. The
  corresponding normal-Debug file target is the required non-sanitized
  control. The focused runner reproduces the same allocation, free and invalid
  read stacks deterministically at
  `cmake-build-debugasan/asan-logs/20260827-155705-build/build.log`.
- Affected revision: the repair is committed at
  `759411507bea289dfe448926357ed84a5f8c91e1` and included in published code
  revision `c0ac864d59428a807102a7b266933967f3b2e294`.
- Retained log:
  `cmake-build-debugasan/asan-logs/20260827-154123-full/build.log`.
- Permanent regression: `san007_imported_inline_payload_lifetime` compiles the
  maintained `httpcore.crexx` import chain in an isolated work directory. Its
  pre-repair Apple-ASan failure is retained at
  `cmake-build-debugasan/asan-logs/20260827-160157-ctest/ctest.log`.
- Repair: context teardown now disconnects every AST-to-symbol connector while
  both sides are live, and symbol teardown clears any surviving AST
  back-pointer before freeing its connector. This repairs both destruction
  orders without retaining a dead imported context or guarding the later
  initializer query. The deterministic `httpcore` target passed through
  `tools/asan-run.sh` at
  `cmake-build-debugasan/asan-logs/20260827-160329-build/build.log`; SAN-006 and
  SAN-007 then passed together at
  `cmake-build-debugasan/asan-logs/20260827-161107-ctest/ctest.log`.
- Broad Apple-ASan evidence: `tools/asan-run.sh --phase full --test-jobs 8
  --build-leaks off --leaks off --no-live-tail --tail-lines 50` passed the
  complete instrumented build and 2,398 of 2,398 CTests in 1,028.38 seconds.
  Logs are retained under
  `cmake-build-debugasan/asan-logs/20260827-161118-full/`. Apple leak detection
  is unsupported and was disabled as required; this is address-safety evidence,
  not Linux LSan closure.
- Normal-product evidence: the full Debug suite passed 2,397/2,397 before the
  teardown-only repair; SAN-006 and the new SAN-007 regression then passed 2/2
  in normal Debug after the repair. The earlier ordinary profiling-off Release,
  focused Release, both-VM, install and package gates remain applicable because
  the repair changes compiler teardown ownership, not emitted RXAS, RXBIN,
  parser or package surfaces. The complete post-repair Apple-ASan build and
  2,398/2,398 CTest run supplies broad final-tree execution. No local gate was
  repeated after the source-identical develop merge.
- Closure evidence: the later exact code revision
  `1dde1dd14fbc288cc0a3041adab4244d533dc528`, which retains this repair and
  regression, passes hosted Build CREXX and both Sanitizer QA platforms.
- Closure checks: the permanent focused regression must pass in normal Debug
  and Apple ASan; the focused `httpcore` target must pass through the runner;
  cumulative normal-product evidence and the full local Apple-ASan build/CTest
  must cover the final code tree; and the exact published code SHA must pass
  GitHub Sanitizer QA's Linux x64 ASan/LSan and macOS arm64 ASan lanes plus
  Build CREXX. Apple leak detection remains unsupported and supplies no Linux
  LSan closure authority.

### SAN-006 — superseded imported class context freed during recursive validation

Status: closed on exact code revision
`1dde1dd14fbc288cc0a3041adab4244d533dc528`; Build CREXX and both Sanitizer
QA platforms pass. The imported-context ownership repair also retains its
focused and broad macOS qualification evidence.

- Scope: compiler symbol construction in `sym_fn()` while a Level G class or
  interface import recursively loads further class metadata.
- Failure: Apple AddressSanitizer reports a one-byte heap-use-after-free read in
  `sym_fn+0x180`. The freed allocation is reused during recursive
  `load_another_file()` / `ensure_class_imported()` processing; ten parallel
  class-library member compiles reproduce the same address-safety failure.
- Original trigger:
  `tools/asan-run.sh --phase build --build-leaks off --build-target
  cri17_attached_provider_control-rxbvml --build-jobs 10`.
- Permanent focused reproducer: CTest `san006_import_context_lifetime` stages
  `classlib_native.rxbin`, the individual `KeyDB.rxbin`, `KeyDB.crexx`, and a
  five-line Level B main source in an isolated directory, then invokes `rxc -x
  --no-exe-import` against that directory. The aggregate and individual image
  first establish a duplicate imported `KeyDB`; recursive validation then
  replaces it with the richer source contract. Removing either duplicate image
  eliminates the pre-repair failure. Ordinary Debug passed before the repair at
  `cmake-build-debug/asan-logs/20260826-203459-ctest`, while Apple-ASan failed
  with the exact `sym_fn()` use-after-free at
  `cmake-build-debugasan/asan-logs/20260826-203511-ctest`.
- Affected revision: clean code base `2aa4dd371121c5e452d7e1e6768c1f090521291f`
  plus the in-progress CRI-17 test/runtime worktree. The compiler source itself
  was not edited by CRI-17, so discovery attribution does not reduce priority.
- Original retained log:
  `cmake-build-debugasan/asan-logs/20260826-174436-build/build.log`.
- Root cause: the richer-contract path in `add_class()` preserved the stable
  imported-class registry record but immediately freed its former parsed
  `Context` and shared file-name allocation. Active recursive import/validation
  frames could still reach that context's AST `node_string` and file-name
  pointers when `sym_fn()` resumed.
- Repair: each stable imported-class record now owns a chain of superseded
  `{Context *, file_name}` pairs. Richer replacement moves the former pair to
  that chain, transfers the new `context`, `file_name`, `contract_node`,
  `contract_type`, and merged implements metadata, and frees every retained
  pair only when the registry record is destroyed. Equal or poorer duplicates
  still free only their newly parsed inactive context. The adjacent audit also
  verified stable class identity storage, implements-copy ownership, and the
  imported-function duplicate paths; no recursive import behavior is disabled.
- Local repair evidence:
  - focused ordinary Debug pass:
    `cmake-build-debug/asan-logs/20260826-203608-ctest`;
  - focused Apple-ASan pass with leak detection off:
    `cmake-build-debugasan/asan-logs/20260826-203621-ctest`;
  - original CRI-17 sanitizer build trigger pass:
    `cmake-build-debugasan/asan-logs/20260826-203628-build`;
  - related CRI-17 ordinary Debug 56/56 and Apple-ASan 4/4 passes:
    `cmake-build-debug/asan-logs/20260826-204004-ctest` and
    `cmake-build-debugasan/asan-logs/20260826-204533-ctest`;
  - complete ordinary Debug 2,391/2,391 pass:
    `cmake-build-debug/asan-logs/20260826-205944-ctest`;
  - complete Apple-ASan build and 2,391/2,391 CTest pass with build/test leak
    detection off:
    `cmake-build-debugasan/asan-logs/20260826-210616-full`.
- Closure evidence: exact code revision
  `1dde1dd14fbc288cc0a3041adab4244d533dc528` passes GitHub Build CREXX and
  Sanitizer QA. The hosted run supplies both Linux x64 ASan/LSan and macOS
  arm64 ASan proof. Apple LeakSanitizer remains unsupported locally; the hosted
  Linux lane supplies leak-closure authority.

## Platform coverage at task close

- Apple ASan on the maintained macOS host does not provide supported leak
  detection. This is not a sanitizer-closure gap: the GitHub Linux x64
  ASan/LSan lane is the leak-closure authority, while the macOS arm64 lane
  supplies Apple-platform address-safety coverage.
- GitHub Build CREXX supplies the final-head MinSizeRel build, CTest and package
  coverage across Linux, macOS and Windows. GitHub Sanitizer QA supplies the
  final-head Linux x64 ASan/LSan and macOS arm64 ASan gates.
- Exact revision `1dde1dd14fbc288cc0a3041adab4244d533dc528` passes Build
  CREXX and both Sanitizer QA platforms, closing every candidate listed here.

## Qualification infrastructure repairs

### SAN-QA-017 — vector concurrency assertion assumes obsolete plugin-wide policy

Status: closed, 2026-09-21; owner Codex under Adrian. QA assertion failure; no
ASan/LSan memory diagnostic in retained failing artifacts.

- Affected revision: `4468c7d75925cc158a568392f4abe97a59156df6`; introduced
  with the RXVECTOR-02 owner change `5949ef27e`.
- Original triggers: Deep `35555049283` and Sanitizer `35559006744` on
  21 September; same failures in `35484487476` / `35487589362` on 20 September.
  All platforms fail `rxpa_bundled_vector_concurrency` with
  `rc=0 capabilities=0 expected=1 handles=1 before=0`. Handle accounting is valid.
- Reproducer: `ctest --test-dir cmake-build-debug -R
  '^rxpa_bundled_vector_concurrency$' --output-on-failure`.
- Cause: the provider now has four process-reentrant procedures and session-affine
  immutable-owner APIs; the test still requires blanket plugin reentrancy. Deep
  also exposes the related missing codec-test preparation dependency.
- Plan/closure: [RXVECTOR-02 AC-09–12](planning/rxvector-binary-owner-20260919.md#overnight-qa-repair-21-september-2026).
  Retain permanent mixed-policy coverage, focused Debug/Apple ASan, normal QA
  preparation and exact-head hosted Deep plus Linux ASan/LSan and Apple ASan.
- Local repair evidence: Debug 21/21 (3.86 s), maintained Apple ASan 21/21
  (6.84 s), four invalid-provider controls rejected, clean QA preparation and
  normal correctness 2344/2344 (817.05 s). No product source change or sanitizer
  diagnostic. Apple LeakSanitizer is unavailable.
- Published revision: `45571319f49f2fd4ad09ad4ebf3f337b20482a00`. Build
  `35574178258`, CodeQL `35574177834` and Deep `35574185432` all pass.
  All five Deep comprehensive platforms pass both repaired tests.
- Closure: [Sanitizer QA 35574187408](https://github.com/adesutherland/CREXX/actions/runs/35574187408)
  is terminal success on that exact revision. Linux ASan/LSan passes 2358/2358
  (3265.95 s); build, QA preparation and CTest all retain `detect_leaks=1`.
  macOS arm64 ASan passes 2358/2358 (4122.91 s). Both repaired tests pass on
  both platforms; no sanitizer diagnostic appears in the complete retained
  build/preparation/test logs. Apple LeakSanitizer is unavailable and Linux
  supplies leak qualification. No suppression, exclusion or timeout change.
- Evidence: [historical capture](https://github.com/adesutherland/CREXX/tree/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/overnight-2026-09-21), including full logs, terminal
  workflow metadata and `hosted/verdict.json`. No remaining action for this item.


### SAN-QA-016 — filesystem concurrency test assumes obsolete plugin-wide policy

Status: closed, 2026-09-17; owner Codex under Adrian. QA assertion failure, with
no observed AddressSanitizer/LeakSanitizer memory diagnostic. The approved
repair and delivery criteria are CI-F20 / F20-AC-01–04 in
[`native-inference-ci.md`](planning/native-inference-ci.md).

- Affected revision: develop `59fc02eb905ea2a0878e4055b7114921d5e6a294`.
  Sanitizer run `35179649593` fails on Linux x64 and macOS arm64; Deep run
  `35175162069` fails on Linux, Windows and both Mac architectures. All six
  failures are `rxpa_bundled_fs_concurrency` with
  `RXPA manifest query failed: rc=0 capabilities=0 handles=1/0`.
- Reproducer: `ctest --test-dir cmake-build-debug -R
  '^rxpa_bundled_fs_concurrency$' --output-on-failure`. The unchanged local
  harness reproduces the identical assertion in 0.02 seconds.
- Cause: `03bf7855b` added the VM-session-owned `rxfs.fileguard` and converted
  rxfs to its documented mixed V2 procedure policy. The test still expects the
  old blanket process-reentrant capability. The library handle count is valid.
- Retained logs and run metadata: [historical capture](https://github.com/adesutherland/CREXX/tree/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/overnight-2026-09-17).
- Focused qualification: Debug 16/16 (0.91 s) and maintained Apple ASan 16/16
  (2.58 s) pass, including all bundled plugin concurrency checks, session-load
  rollback and all four filesystem VM/optimization cells. Three deliberately
  invalid policy/lifecycle variants are rejected by the repaired test.
- Closure: published repair `d8f59732d4d4abeb18ec20509821fd30ecf8962e` passes
  [Sanitizer QA 35189897956](https://github.com/adesutherland/CREXX/actions/runs/35189897956)
  with 2,345/2,345 on Linux ASan/LSan and macOS arm64 ASan. Linux build,
  QA preparation and CTest logs all retain `detect_leaks=1`. No ASan/LSan
  diagnostic appears in the complete retained artifacts. Apple LeakSanitizer
  is unavailable. Deep `35189896087`, Build `35189886874` and CodeQL
  `35189886774` are also terminal success on that exact revision. Deep passes
  five comprehensive platforms and the original filesystem regression on each.
  No exclusions, suppression, scheduling or timeout changes were introduced.

### SAN-QA-013 — expanded project-build matrix exceeds its aggregate timeout

Status: closed for the observed macOS harness timeout. The expanded permanent
contract passes normal Debug and Apple-ASan, accounting with the unchanged broad
passes for all 2,271 applicable macOS sanitizer checks. Owner: RXC-PROJECT-01
(Adrian/Codex). This is not Linux or cross-platform sanitizer closure.

- Affected revision: e0e67ad3e plus the local RXC-PROJECT-01 compiler/wrapper
  and expanded `driver/tests/crexx_project_build.cmake` matrix, 2026-09-06.
- Reproducer: `tools/asan-run.sh --build-dir ../build-debugasan --phase full
  --build-jobs 4 --test-jobs 8 --build-leaks off --leaks off
  --exclude-label performance-measurement --no-live-tail`, from the isolated
  `crexx-scaling-20260906.1lT795/source` worktree on macOS ARM64.
- Retained failure:
  `../build-debugasan/asan-logs/20260906-110736-full/ctest.log`:
  `crexx_project_build_contract` exceeded 180 seconds; the other 2,270 checks
  passed. There is no ASan memory diagnostic. The expanded contract adds many
  complete project checks/builds, including environment, tool-content and
  installed-candidate matrices, but retained the earlier aggregate deadline.
- Repair: retain every assertion and subprocess, expose the current scenario
  label in the test log, and allow the expanded composite a bounded 600 seconds.
  This does not change compiler performance, child options or test coverage.
- Focused proof: normal Debug passes in 77.01 seconds at
  `../build-debug/asan-logs/20260906-120138-ctest/ctest.log`; Apple-ASan passes
  in 193.84 seconds at
  `../build-debugasan/asan-logs/20260906-120320-ctest/ctest.log`. The latter
  exceeds the previous 180-second aggregate bound and completes below the new
  bound, with every assertion retained. These logs and the original broad log
  are copied into the RXC-PROJECT-01 qualification evidence bundle.
- Closure: the same permanent contract passes normal Debug and maintained
  Apple-ASan; retain those logs with the 2,270 unchanged broad passes to account
  for all 2,271 applicable macOS checks. Linux is not the failing platform and
  no Linux or leak-clean claim follows from this timeout repair.

### SAN-QA-012 — concurrency fixtures assume scheduler timing or assignment

Status: closed on exact code revision
`1dde1dd14fbc288cc0a3041adab4244d533dc528`; Build CREXX, Deep Build QA and
both Sanitizer QA platforms pass. Focused final-tree normal-Debug and Apple-ASan
controls also pass. This was a maintained sanitizer test failure, not an ASan
memory diagnostic or a product timeout.

- Scope: the E5 persistent-worker native-return cancellation test, adjacent
  RXPA concurrency fixtures, and the CRI-17 attached-provider worker-session
  test found by applying the same audit criterion to the hosted repair.
- Failure: scheduled Sanitizer QA run
  [33717508663](https://github.com/adesutherland/CREXX/actions/runs/33717508663)
  failed only `persistent_worker_executor-rxvml-native-return` in its macOS
  arm64 ASan lane. The test reported all three aggregate states as failed after
  `PANIC: (SIGNAL KILL)`; the Linux x64 ASan/LSan lane passed the complete
  suite. The retained hosted artifact contains no sanitizer report.
- Tested revision: although GitHub attaches a scheduled workflow run to the
  default branch, the workflow's gate resolves and checks out current
  `develop`. The failing gate log records
  `e9c8f753a84f192d80cbb2b6df2f9b264fd69a57`, not the displayed `master`
  revision `fae35f645c43e825a06682b7aef6ce456a6536a6`.
- Timeout classification: the test completed in 0.29 seconds against a
  30-second CTest timeout, and the job failed after about 65 minutes against a
  120-minute workflow timeout. `SIGNAL KILL` is the VM signal deliberately
  published by the coalesced CANCEL/KILL case; it is also present in passing
  direct runs and is not evidence that GitHub or the OS killed the process.
- Root cause: `e5native.pause` published its entry and then slept for 50 ms.
  The controller had to observe entry and publish mailbox events before that
  sleep expired. Under the eight-way full Apple-ASan load, the native worker
  could return while the controller was descheduled, so the test sometimes
  observed ordinary completion instead of cancellation. The delay was test
  orchestration only; no production worker ordering contract requires it.
- Why earlier gates did not catch it: the fixture entered `develop` on
  2026-08-13. Its original Apple-ASan closeout ran 20 focused tests serially,
  and the full Debug suite passed. From the first hosted sanitizer run that
  contained the fixture on 2026-08-22 through the run before this failure, 31
  completed Sanitizer QA runs passed; the failed-step logs from all 23 other
  completed failures contain no native-return failure, while four runs were
  cancelled. The previous scheduled gate qualified `develop` revision
  `516d953bdf7b1ae455aa28ffe83e0fe0ac24bb5d`; the failing revision landed
  afterward and was checked by the next nightly run. None of the intervening
  changes edited the failing fixture. This was the first retained occurrence,
  not an ignored earlier failure; the rare scheduler window and serial focused
  qualification allowed the latent test race to survive.
- Deterministic pre-repair reproducer: delaying the controller for 100 ms
  immediately after observing native entry makes the former 50 ms fixture fail
  in ordinary Debug with the same aggregate E5 result. The temporary
  counterfactual was removed; its build and test logs are retained under
  `cmake-build-debug/asan-logs/20260903-sanqa012-pre-fix/`.
- Repair: the native fixture now waits for an explicit atomic release from the
  controller. The controller publishes CANCEL, or CANCEL followed by KILL,
  before release; the existing 30-second CTest timeout bounds a broken
  handshake. Per-stage diagnostics separately report mailbox publication,
  coalesced priority, worker reuse and teardown instead of copying one boolean
  into every field.
- Adjacent audit: the RXPA and mock-ODBC concurrency fixtures also used 50 ms
  sleeps to infer positive overlap or legacy-call contention. Positive overlap
  now uses condition-variable rendezvous, ODBC enables its rendezvous only for
  the overlap test, and legacy serialization waits until both callers have
  explicitly attempted the call. Legacy-transition quiescence no longer samples
  a 50 ms period in which completion must remain absent: the internal
  coordinator broadcasts when its transition is genuinely waiting, and the
  test waits positively for that state before checking quiescence. These
  rendezvous tests have explicit 30-second CTest timeouts so a missing event or
  genuine regression is reported as a timeout rather than hanging the gate.
- First hosted qualification: exact-SHA Build CREXX run
  [33736096849](https://github.com/adesutherland/CREXX/actions/runs/33736096849)
  passed its other jobs but the Windows x64 comprehensive job failed only
  `cri17_attached_provider_noopt`: both fast task calls legitimately ran on
  provider session 2. The optimized sibling and both direct C controls passed.
  `.taskpool.local(2, ...)` permits two calls to overlap but does not promise
  worker affinity or any particular schedule, so comparing two unsynchronised
  calls could not prove two worker sessions. The CRI-17 provider now holds the
  two calls at a condition-variable rendezvous before returning their captured
  session identities. A single-worker or session-isolation regression therefore
  reaches the existing 60-second CTest timeout instead of passing by schedule.
- Strategic follow-through: a repository-wide review of local task pools,
  session/worker identity checks, maximum-overlap checks and fixed 50 ms sleeps
  found no further scheduler-assignment assumptions. Persistent-executor tests
  address fixed workers explicitly; positive RXPA and ODBC overlap tests now
  rendezvous; HTTP/task tests depend on protocol backpressure rather than task
  identity. A search covering common 50 ms sleep, microsecond and nanosecond
  spellings now finds no fixed 50 ms delay in maintained test code.
- Local repair evidence:
  - the nine-cell E5/RXPA/ODBC panel passed in normal Debug, followed by 500
    consecutive passes of each of its eight rendezvous-based cells; logs are
    retained under
    `cmake-build-debug/asan-logs/20260903-sanqa012-repair/`;
  - dependency-prepared comprehensive Debug QA passed 2,236/2,236 in 288.87
    seconds, retained as `qa-comprehensive.log` in that directory;
  - the same nine-cell panel passed Apple ASan with leak detection disabled at
    `cmake-build-debugasan/asan-logs/20260903-091130-ctest/`;
  - the maintained Apple-ASan command matching the hosted macOS lane completed
    its full instrumented build and QA preparation, then passed 2,250/2,250
    non-measurement CTests at eight-way concurrency in 1,068.52 seconds. The
    exact failing `rxvml` cell and both concrete-VM siblings passed in 0.14,
    0.13 and 0.14 seconds. Logs are retained under
    `cmake-build-debugasan/asan-logs/20260903-091948-full/`; none contains an
    AddressSanitizer diagnostic.
  - after the Windows finding, the repaired four-cell CRI-17 normal-Debug panel
    passed, 375 completed repeated cells passed before the deliberately stopped
    local repetition, and the same four-cell panel passed focused Apple ASan at
    `cmake-build-debugasan/asan-logs/20260903-105559-ctest/` with no sanitizer
    diagnostic. Broad final-tree qualification is assigned to the parallel
    exact-SHA hosted workflows rather than repeating the unchanged low-risk
    local suites.
  - after removing the final RXPA observation window, its six-cell coordinator
    panel passed in normal Debug and focused Apple ASan at
    `cmake-build-debugasan/asan-logs/20260903-110318-ctest/`, with no sanitizer
    diagnostic.
- Closure evidence: exact code revision
  `1dde1dd14fbc288cc0a3041adab4244d533dc528` passes Build CREXX, Sanitizer QA
  and Deep Build QA, including Windows x64 Build and Linux x64 ASan/LSan.

### SAN-QA-011 — private bootstrap rules assume their working directories exist

Status: closed on exact code revision
`1dde1dd14fbc288cc0a3041adab4244d533dc528`; Build CREXX and both Sanitizer
QA platforms pass. Focused normal-Debug and Apple-ASan controls plus the
maintained broad Apple-ASan gate also pass. This was a maintained sanitizer
build failure, not an ASan memory diagnostic.

- Scope: private CMake bootstrap rules for the installed `crexx --library` /
  `--tool` Level B worker and the `rxpp` member image.
- Failure: the full Apple-ASan runner reached 98 percent of its build and then
  Make failed before executing the worker custom command because its declared
  working directory, `crexx_driver/crexx-build-worker`, did not exist. After
  that repair passed focused Debug and ASan controls, the broad rerun exposed
  the same defect in `preprocessor/members/rxpp`.
- Original trigger: `tools/asan-run.sh --phase full --build-jobs 5
  --test-jobs 5 --build-leaks off --leaks off --exclude-label
  '^performance-measurement$' --no-live-tail` on macOS arm64 at local Phase 4
  revision `bee5f690a20386a7459a173de5e4870f71199e58` plus the uncommitted P4.4
  qualification corrections.
- Retained log:
  `cmake-build-debugasan/asan-logs/20260901-112213-full/build.log`; the second
  family trigger is retained at
  `cmake-build-debugasan/asan-logs/20260901-114104-full/build.log`.
- Root cause: the custom command attempted to create its private directories
  as its first command, but CMake's Makefile rule changes to `WORKING_DIRECTORY`
  before any command is run. A clean tree therefore failed at the shell-level
  `cd`; previously populated Ninja trees masked the missing producer step.
- Repair: keep each custom command as the sole producer of its private tree.
  Run each rule itself from its existing component binary directory, create
  the private source/import directory, and use `cmake -E chdir` only around
  `rxc` and `rxas` after that directory exists. This avoids configure-time
  shadow producers and remains safe when the generated private tree starts
  absent. An audit of every current `add_custom_command` that combines
  `make_directory` with `WORKING_DIRECTORY` found no other unowned working
  directory; classlib, Level C and RexxScript member roots are deliberately
  established at configure time and the remaining rules use existing build
  directories.
- Focused reproducers: remove only the generated
  `crexx_driver/crexx-build-worker` tree and build target
  `crexx_build_worker`; remove only `preprocessor/members/rxpp` and build target
  `rxpp`. Both targets must pass in ordinary Debug and through
  `tools/asan-run.sh --phase build --build-target TARGET` with the
  platform-appropriate leak setting.
- Local repair evidence: the absent worker tree passed normal Debug at
  `cmake-build-debug/asan-logs/20260901-113600-build` and Apple ASan at
  `cmake-build-debugasan/asan-logs/20260901-114051-build`. The absent `rxpp`
  member tree passed normal Debug at
  `cmake-build-debug/asan-logs/20260901-114216-build` and Apple ASan at
  `cmake-build-debugasan/asan-logs/20260901-114225-build`.
- Broad Apple-ASan evidence: the original full trigger then passed its complete
  instrumented build, explicit `qa-prep` barrier and all 2,238 non-performance
  CTests. No AddressSanitizer diagnostic was present; logs are retained under
  `cmake-build-debugasan/asan-logs/20260901-114245-full`.
- Closure evidence: exact code revision
  `1dde1dd14fbc288cc0a3041adab4244d533dc528` passes GitHub Build and
  Sanitizer QA. Linux ASan/LSan supplies the leak-closure authority.

### SAN-QA-010 — parallel spawn stress exceeds its scope under contended ASan

Status: closed on exact code revision
`1dde1dd14fbc288cc0a3041adab4244d533dc528`; Build CREXX and both Sanitizer
QA platforms pass after the focused and designed-slice Apple-ASan proof. No
sanitizer suppression, product assertion or launcher cancellation behavior was
weakened.

- Scope: CTest scheduling for the new
  `test_address_parallel_spawn_{noopt,opt}` launcher-race regressions.
- Failure: the designed Phase 3 Apple-ASan slice ran both 4,096-spawn variants
  concurrently with four heavy Unicode tests. At final revision candidate
  `aecd79da5551de81cc97ecc84210f01c184c9169`, both task scopes reached their
  60-second correctness deadline after 65–66 seconds and returned typed
  deadline completions. The retained log is
  `cmake-build-debugasan/asan-logs/20260831-203532-ctest/ctest.log`. It contains
  no AddressSanitizer memory diagnostic or process-group launch failure.
- Original trigger: `tools/asan-run.sh --phase ctest --test-jobs 5 --leaks off`
  with the designed Phase 3 regex recorded in the Phase 3 progress note.
- Smallest permanent reproducer: each stress variant launches 4,096 controlled
  children through a local Level B task pool; the fixture asserts POSIX child
  process-group ownership. The ordinary Debug pair passed together in about
  10.5 seconds on the same tree.
- Repair candidate: retain the assertions, internal deadline and outer timeout,
  but mark the two stress variants `RUN_SERIAL`. Ordinary Apple Debug retains
  4,096 launches per variant; sanitizer builds use 256 launches per variant
  through the same eight-worker ownership path because instrumentation makes
  the full stress exceed the correctness watchdog even in isolation. This
  keeps maximum race pressure in the stress lane and memory-safety coverage in
  the sanitizer lane without turning either deadline into a performance test.
- Repair evidence: the focused Apple-ASan pair passed 2/2 serially in
  `cmake-build-debugasan/asan-logs/20260831-204902-ctest/ctest.log`. The complete
  designed Phase 3 slice then passed 33/33 with five CTest workers in
  `cmake-build-debugasan/asan-logs/20260831-205106-ctest/ctest.log`; the spawn
  variants waited for the concurrent Unicode work and then ran one at a time.
  Neither retained run contains an AddressSanitizer diagnostic. Exact-SHA run
  [33433382690](https://github.com/adesutherland/CREXX/actions/runs/33433382690)
  proved that 1,024 instrumented launches still lacked enough hosted headroom:
  both variants reached the unchanged 60-second internal deadline after about
  64 seconds. The 256-launch focused Apple-ASan pair subsequently passed in
  12.88 and 15.15 seconds, retaining the eight-worker race shape with ample
  correctness-watchdog margin.
- Closure evidence: exact code revision
  `1dde1dd14fbc288cc0a3041adab4244d533dc528` passes final GitHub Build and
  both Sanitizer QA lanes. Linux ASan/LSan supplies the leak-closure authority.

### SAN-QA-009 — interactive stdin watchdogs expire under contended Apple ASan

Status: closed on exact code revision
`1dde1dd14fbc288cc0a3041adab4244d533dc528`; both Sanitizer QA platforms pass
after the focused normal-Debug and Apple-ASan proof. No sanitizer suppression,
test exclusion or product-behaviour waiver was used.

- Scope: the three interactive `linein(stdin)` correctness tests which launch
  a VM or the `crexx` driver through pipe/PTY harnesses.
- Failure: GitHub Sanitizer QA run
  [33400864223](https://github.com/adesutherland/CREXX/actions/runs/33400864223)
  failed only `ts_linein_stdin_prompt` and `ts_linein_stdin_crexx_tty` in its
  [macOS arm64 ASan job](https://github.com/adesutherland/CREXX/actions/runs/33400864223/job/99516538587)
  at revision `5befda0f86c827fd02cd8f7597833a039dc2e2ef`.
  The direct prompt did not become visible within its 3-second harness
  watchdog; the PTY driver exceeded its 15-second watchdog while another
  instrumented `crexx` driver case was running. The sibling driver case passed
  in 15.32 seconds. The uploaded sanitizer artifact contains no ASan memory
  diagnostic; this was a first-party maintained-lane failure until closure.
- Original trigger: the workflow's maintained full Apple-ASan build and CTest
  command, `tools/asan-run.sh --phase full --build-jobs 4 --test-jobs 8
  --build-leaks off --leaks off`, as retained by the run above.
- Permanent focused regressions: `ts_linein_stdin_prompt`,
  `ts_linein_stdin_crexx_driver` and `ts_linein_stdin_crexx_tty`. They verify
  the complete prompt/input/completion conversation, including real PTY
  foreground ownership; their behavioral assertions are unchanged.
- Repair: serialize only this interactive family with the
  `linein_stdin_conversation` CTest resource lock. Use a 30-second internal
  correctness watchdog so instrumented compiler/driver startup is not treated
  as a performance assertion. CTest retains an outer 70-second limit for the
  two-phase pipe harness and 40 seconds for the single-deadline PTY harness.
  These tests remain eligible to run alongside unrelated QA; performance tests
  and timing claims remain outside this correctness lane.
- Local evidence: the focused fixture target passed ordinary Debug at
  `cmake-build-debug/asan-logs/20260831-170647-build`, followed by all three
  tests at `cmake-build-debug/asan-logs/20260831-170829-ctest`. The same target
  passed Apple ASan at
  `cmake-build-debugasan/asan-logs/20260831-171457-build`, followed by all three
  tests at `cmake-build-debugasan/asan-logs/20260831-171637-ctest`. The ASan
  tree also exposed and qualified clean-Make working-directory ownership for
  the private classlib, Level C and RexxScript member families.
- Closure evidence: the clean full exact-SHA GitHub Sanitizer QA run passes on
  Linux x64 ASan/LSan and macOS arm64 ASan. Apple ASan provides no local
  leak-closure authority; the hosted Linux lane supplies it.
- Acceptance: retain all three permanent conversations, prove their focused
  build/test prerequisites from a clean-enough tree, pass the focused normal
  Debug and maintained Apple-ASan shapes, rerun the original broad trigger,
  and pass both GitHub sanitizer lanes on the final published revision.

### SAN-QA-008 — saturated child redirect loses typed timeout completion

Status: closed on exact code revision
`1dde1dd14fbc288cc0a3041adab4244d533dc528`; both Sanitizer QA platforms pass,
including `rxspawn_posix_termination` and `rxvmchannel_byte_provider`. The
provider remains parallel and the strengthened diagnostic remains active.

- 2026-09-12 recurrence: Sanitizer QA
  [34702281757](https://github.com/adesutherland/CREXX/actions/runs/34702281757),
  macOS arm64 job [103576080611](https://github.com/adesutherland/CREXX/actions/runs/34702281757/job/103576080611),
  checked out `99d500eb99ecc947ce819643eccc14b430760e8a` and failed only
  `rxvmchannel_byte_provider` with the same saturated-redirect assertion,
  `state=2`, `errorCode=17`, and `Failure spawn terminate. Details: RC=1
  Text=Operation not permitted`. The retained artifact is
  `/tmp/crexx-693-macos-artifact.lceTQL/20260912-152819-full/ctest.log`
  (SHA-256 `0c817d5fbffbe095e0193f07426f312323e75aaadae84b11637b2b4c05e42f8a`).
  No ASan/LSan memory diagnostic appears. The new #693 regression was
  selected later and passed; the earlier termination repair has therefore
  not covered every occurrence. Owner: resumed hotfix maintenance, tracked in
  [#697](https://github.com/adesutherland/CREXX/issues/697).
- 2026-09-12 repair candidate: a pipe-synchronized native probe verifies a
  signalable live group, releases its owned child to exit, and observes group
  `EPERM` before `waitid` exposes terminal status. All 1,000 iterations failed
  with the old helper; all 1,000 pass after retrying the signal against the
  still-owned, unreaped direct child. A denied direct signal still requires
  confirmed terminal status, and post-reap cleanup remains group-only. The
  strengthened `rxspawn_posix_termination` regression fails before repair and
  passes afterward; real permission errors and reap ownership remain covered.
  The identical termination/launch/byte-provider panel passes 3/3 in normal
  Debug at `cmake-build-debug/asan-logs/20260912-180805-ctest` and maintained
  Apple ASan at `cmake-build-debugasan/asan-logs/20260912-180909-ctest`.
  The maintained Apple-ASan full run at
  `cmake-build-debugasan/asan-logs/20260912-185113-full` passes 2,299/2,302;
  `rxspawn_posix_termination`, `rxspawn_posix_launch_diagnostic`, and
  `rxvmchannel_byte_provider` all pass, with no sanitizer diagnostic. Three
  aggregate/build-driver tests time out under the eight-way run and then pass
  unchanged with one test job at
  `cmake-build-debugasan/asan-logs/20260912-193456-ctest` in 305.57, 266.14,
  and 3.01 seconds. All 33,795 frozen non-documentation inputs match their
  pre-run SHA-256 manifest. Exact-commit run 34718578056 now supplies the
  required hosted Linux ASan/LSan and macOS ASan qualification; see the
  [repair report](planning/beta-3/reports/mike-fix-697-2026-09-12.md).
- 2026-09-09 recurrence: Sanitizer QA
  [34308566325](https://github.com/adesutherland/CREXX/actions/runs/34308566325)
  actually checked out `82730b495c07a6fb1bc34ffe4e9ad8b33aba50d3`.
  The retained macOS artifact at
  `/tmp/crexx-ci-20260909.wjmUik/sanitizer/sanitizer-logs-macos/20260909-034939-full/ctest.log`
  identifies `Failure spawn terminate. Details: RC=1 Text=Operation not permitted`,
  `state=2`, `errorCode=17`. This establishes termination, not launch setup,
  as the failing stage. Hotfix QA owns a deterministic exited-child reproducer
  and normal Debug/maintained sanitizer qualification; no assertion is relaxed.
- Repair candidate, 2026-09-09: the new `rxspawn_posix_termination` regression
  deterministically reproduces Darwin's group `EPERM` for an exited but
  unreaped child. The private signalling helper now confirms terminal child
  state using nonblocking `waitid` with `WNOWAIT` before accepting that result.
  Reap ownership and post-reap group-only cleanup remain intact; live-child and
  unrelated failures remain errors. The regression, byte-provider test and
  launch diagnostic pass together in normal Debug and Apple ASan (3/3 at
  `cmake-build-debugasan/asan-logs/20260909-065238-ctest`). At that point closure
  remained conditional on the original broad trigger and both exact-code
  hosted sanitizer lanes passing; see the
  [hotfix report](planning/beta-3/reports/ci-hotfix-2026-09-09.md).

- Scope: the byte-channel child-process provider's deadline and saturated
  output-redirect completion path, exercised by `rxvmchannel_byte_provider`.
- Failure: macOS arm64 Sanitizer QA run
  [32879812936](https://github.com/adesutherland/CREXX/actions/runs/32879812936)
  failed only `rxvmchannel_byte_provider` at revision
  `6fd40945d8665c5184b1044269697412310b538f`. The retained `ctest.log`
  reports `FAIL: saturated redirect preserves typed timeout completion`.
  There is no AddressSanitizer memory diagnostic in the retained log; this is
  nevertheless a first-party failure in the maintained sanitizer lane and is
  release-blocking until qualified.
- Original trigger: `tools/asan-run.sh --phase full --build-jobs 4
  --test-jobs 8 --build-leaks off --leaks off`, as recorded in the uploaded
  `sanitizer-logs-macos` artifact for the run above.
- Smallest permanent reproducer: the saturated bounded-output deadline case in
  `interpreter/tests/test_rxvmchannel_byte.c`, run with
  `tools/asan-run.sh --phase ctest --regex '^rxvmchannel_byte_provider$'
  --leaks off --test-jobs 1` after building the focused target through the
  runner. The case now reports both the returned completion state and error
  code and the provider's bounded message if it fails, so a future lane failure
  identifies the failing launch or redirect stage rather than another one-bit
  assertion.
- Local evidence: 200 serial ordinary Debug repetitions, 200 serial macOS
  arm64 ASan repetitions and 400 macOS arm64 ASan repetitions across eight
  concurrent runner-managed CTest processes all passed on the synchronized
  hotfix source. The original broad macOS arm64 trigger then passed its build
  and all 2,378/2,378 CTests on code commit `71aa1fd15` at
  `cmake-build-debugasan/asan-logs/20260826-104630-full`; the formerly failing
  `rxvmchannel_byte_provider` passed as test 1151 under eight-way load. No
  memory-safety report or typed-state failure was reproduced.
- Closure: GitHub Sanitizer QA run
  [32958593912](https://github.com/adesutherland/CREXX/actions/runs/32958593912)
  passed both macOS arm64 ASan and Linux x64 ASan/LSan on final published code
  revision `a744b4d2551d795cf9bbf09c46c5a3fd71e53d46`, after the complete local
  macOS arm64 ASan pass above. If the test recurs, its retained state and error
  code distinguish deadline publication, child launch and redirect shutdown;
  do not weaken the assertion or timeout.
- Recurrence: GitHub Sanitizer QA run
  [33421800264](https://github.com/adesutherland/CREXX/actions/runs/33421800264)
  failed this same assertion on macOS arm64 at
  `a697c4939310ec5df172a1e37608ef891bd01127`. The retained completion is
  `state=2`, `errorCode=17` (`RXVM_CHANNEL_PROVIDER_FAILURE`), while Linux
  passed this test and neither artifact contains an ASan/LSan memory report.
  A concurrent Level B builder workload independently exposed `Failure
  creating controlled child process group ... Operation not permitted`.
  A native stress probe confirmed that Darwin can return `EPERM` to one of
  concurrent parent/child `setpgid()` calls even when the other establishes
  the requested group. The repair candidate makes the child the sole group
  creator and uses the close-on-exec launch-status pipe as the parent's proof
  of setup. The permanent cREXX regression is
  `test_address_parallel_spawn_{noopt,opt}`.
- Later recurrence: exact-SHA run
  [33433382690](https://github.com/adesutherland/CREXX/actions/runs/33433382690)
  passed Linux ASan/LSan, but macOS ASan again returned `state=2`,
  `errorCode=17` in 0.96 seconds. It ran much earlier than the serial spawn
  stress tests and overlapped only short interpreter/RXPA tests, so it is not
  classified as the stress watchdog issue in SAN-QA-010. The old diagnostic
  did not retain the provider message, so the exact failing stage cannot be
  recovered from that artifact. The test remains parallel and its assertion
  and 100-millisecond semantic deadline are unchanged. An adjacent lifecycle
  audit also removed post-`waitpid` fallback from process-group signalling to
  a potentially reused positive PID; that verified safety defect is not
  claimed as the cause of this recurrence.
- Acceptance: retain a focused regression that fails for the original reason,
  pass the same focused shape in ordinary Debug and maintained sanitizer
  builds, rerun the original broad trigger cleanly, and pass both GitHub
  sanitizer lanes on the final published revision.

### SAN-QA-007 — RXQUEUE export variants share a temporary file

Status: closed; the exact macOS arm64 ASan failure is retained, focused
normal-Debug and Linux ASan/LSan proof is green, and the complete Linux plus
GitHub sanitizer gates pass on `e3de72939`.

- Scope: CTest scheduling for `ts_rxqueue_noopt` and `ts_rxqueue_opt` only;
  queue implementation, export/import behavior, test operations, and assertions
  are unchanged.
- Failure: macOS arm64 Sanitizer QA run
  [32618328641](https://github.com/adesutherland/CREXX/actions/runs/32618328641)
  started both variants together. They then reported missing imported or
  replaced `REPORT` entries because each uses `ts_rxqueue_export.txt` in their
  shared working directory.
- Diagnosis: the two variants are individually process-local except for that
  fixed export file. Their overlapping `EXPORT`, `IMPORT`, replacement, and
  `erasefile` operations can consume or remove the other variant's data.
- Repair: assign both tests the narrow `rxqueue_export_file` CTest resource
  lock so the variants cannot overlap while unrelated tests remain parallel.
- Focused proof: with eight CTest jobs requested, both variants pass ordinary
  Debug at `cmake-build-debug/asan-logs/20260823-065551-ctest` and leak-enabled
  Linux ASan/LSan at
  `cmake-build-debugasan/asan-logs/20260823-074553-ctest`. CTest starts the
  second variant only after the first finishes, proving the generated resource
  lock is active rather than relying on incidental timing.
- Acceptance: both variants must pass together while requesting parallel CTest
  execution in ordinary Debug and Linux ASan/LSan, followed by complete Linux
  and GitHub sanitizer gates on the committed repair.

### SAN-QA-006 — RXPA signature matrix exceeds short ASan timeout

Status: closed; focused normal-Debug and Linux ASan/LSan replays and the
complete supported Linux sanitizer gate pass on `e3de72939`.

- Scope: the CTest timeout for `rxpa_signature_diagnostics` only; compiler,
  RXPA, optimizer, diagnostic assertions, and runtime behavior are unchanged.
- Failure: the full leak-enabled Linux ASan/LSan run at
  `cmake-build-debugasan/asan-logs/20260823-043515-full` timed out the test at
  120 seconds while three long performance tests were active. It emitted no
  sanitizer diagnostic or partial assertion failure.
- Diagnosis: this is a composite contract containing two valid compiles, two
  assembles, four VM executions, and twelve expected-invalid compiler runs.
  The exact test passes alone in ordinary Debug in 16.61 seconds at
  `cmake-build-debug/asan-logs/20260823-052452-ctest` and leak-enabled Linux
  ASan/LSan in 45.02 seconds at
  `cmake-build-debugasan/asan-logs/20260823-052515-ctest`.
- Repair: use the repository's established 300-second allowance for composite
  compiler/runtime contracts. The 20 subprocesses and all output/diagnostic
  assertions remain unchanged.
- Focused proof: generated CTest metadata reports the repaired 300-second
  property in both build trees. The repaired test passes ordinary Debug in
  16.64 seconds at `cmake-build-debug/asan-logs/20260823-053553-ctest` and
  leak-enabled Linux ASan/LSan in 45.44 seconds at
  `cmake-build-debugasan/asan-logs/20260823-053616-ctest`.
- Acceptance: the focused test must pass in normal Debug and leak-enabled
  Linux ASan/LSan, followed by the complete supported Linux sanitizer gate.

### SAN-QA-005 — parser snapshot tests require full-suite isolation

Status: closed; focused normal-Debug and Linux ASan/LSan replays and the
complete supported Linux sanitizer gate pass on `e3de72939`.

- Scope: CTest scheduling for the three preprocessor syntax-highlighting tests;
  parser, preprocessor, and compiler behavior is unchanged.
- Failure: the full leak-enabled Linux ASan/LSan run at
  `cmake-build-debugasan/asan-logs/20260823-011709-full` captured only the
  asynchronous emergency parse tree for `rxpp_sh_srcmap` while CPU-heavy
  compiler and E6 measurement tests were active. There was no sanitizer
  diagnostic, crash, or nonzero `parser_tester` result.
- Repair: apply the existing `syntax_highlighting_parser` resource lock and
  `RUN_SERIAL` isolation used by compiler parser-snapshot tests to all three
  preprocessor `parser_tester` contracts.
- Focused proof: `rxpp_sh_srcmap` passes ordinary Debug in 1.86 seconds at
  `cmake-build-debug/asan-logs/20260823-024503-ctest` and leak-enabled Linux
  ASan/LSan in 4.57 seconds at
  `cmake-build-debugasan/asan-logs/20260823-024513-ctest`. After the repair,
  the complete three-test parser panel passes while requesting eight CTest
  jobs in both ordinary Debug and leak-enabled Linux ASan/LSan as part of the
  nine-test panels at `20260823-025705-ctest` and `20260823-025810-ctest`.
- Acceptance: the parser-mode tests must pass in the complete supported Linux
  sanitizer gate with their serial property present.

### SAN-QA-004 — E6 measurements contend under parallel ASan

Status: closed; the exact six-cell Linux ASan/LSan replay and the complete
supported Linux sanitizer gate pass on `e3de72939`.

- Scope: CTest scheduling for E6 performance-measurement cells only; benchmark
  workloads, VM behavior, iteration counts, and 120-second limits are
  unchanged.
- Failure: in the full leak-enabled Linux run at
  `cmake-build-debugasan/asan-logs/20260823-011709-full`, three concurrent
  one-worker E6 cells timed out at 120 seconds. A fourth completed in 103.57
  seconds, showing severe shared-host distortion.
- Diagnosis: the exact six one-worker spin/churn cells pass serially under
  leak-enabled Linux ASan/LSan in 12.68 to 40.11 seconds each, with all six
  completing in 172.31 seconds at
  `cmake-build-debugasan/asan-logs/20260823-024140-ctest`.
- Repair: mark every E6 performance-measurement cell `RUN_SERIAL`. These tests
  are intended to measure scale; overlap with another benchmark or a
  compiler-heavy test invalidates both their recorded wall time and their
  timeout contract.
- Focused proof: a combined six-cell E6 plus three-test parser panel passes
  9/9 while requesting eight CTest jobs in ordinary Debug at
  `cmake-build-debug/asan-logs/20260823-025705-ctest` and leak-enabled Linux
  ASan/LSan at `cmake-build-debugasan/asan-logs/20260823-025810-ctest`. CTest's
  generated metadata reports `RUN_SERIAL=true` for all 24 E6 cells; the ASan
  panel completed in 180.41 seconds with no overlap and no timeout change.
- Acceptance: the complete supported Linux sanitizer gate must pass without
  increasing the individual test timeout.

### SAN-QA-003 — ASan fake stack defeats POSIX doorbell slot lookup

Status: closed; focused normal-Debug and Linux ASan/LSan qualification and the
complete supported Linux sanitizer gate pass on `e3de72939`.

- Scope: the POSIX native-doorbell signal handler under address
  instrumentation; ordinary production dispatch and executor semantics are
  unchanged.
- Failure: Linux Sanitizer QA run
  [32601922923](https://github.com/adesutherland/CREXX/actions/runs/32601922923)
  timed out the base and doorbell-stress persistent-worker panels for all three
  VM libraries. A serialized local replay also remained in `loop_forever` past
  300 seconds, so scheduling and timeout changes were rejected as the fix.
- Root cause: the handler identifies its target through the address of a local
  stack marker and compares it with registered pthread stack ranges. GCC ASan
  instrumented that handler and could place the marker on its fake stack,
  outside the real pthread range; `pthread_kill()` succeeded but the handler
  silently found no slot and never published `CANCEL`.
- Repair: exclude only the bounded async-signal handler from address
  instrumentation, alongside its existing stack-protector exclusion. This
  keeps the stack probe on the real target stack and also prevents sanitizer
  runtime traversal from entering the async-signal-safe handler. The ordinary
  120-second tests and their parallel scheduling remain unchanged.
- Focused proof: all six base/doorbell-stress variants pass concurrently in
  ordinary Debug at `cmake-build-debug/asan-logs/20260823-011534-ctest` and in
  leak-enabled Linux ASan/LSan at
  `cmake-build-debugasan/asan-logs/20260823-011247-ctest`; the latter completes
  in 7.77 seconds total. Disassembly of the rebuilt ASan
  `rxbvm_core_objects` and `rxtvm_core_objects` handlers contains no ASan
  runtime reference.
- Acceptance: all six cells must pass together through leak-enabled Linux
  ASan/LSan with eight requested CTest jobs, followed by the complete supported
  Linux sanitizer gate.

### SAN-QA-002 — composite control-flow test timeout under parallel ASan

Status: closed; focused normal-Debug and Linux ASan/LSan replays and the
complete supported Linux sanitizer gate pass on `e3de72939`.

- Scope: test infrastructure only; no production compiler or runtime code.
- Failure: the leak-enabled Linux full CTest run timed out
  `do_forever_return_contract` at its fixed 120-second test limit while eight
  instrumented compiler tests were active. The same exact test passed alone
  with leak detection in 33.22 seconds and emitted no sanitizer diagnostic.
  The full-run failure and isolated replay are retained in
  `20260822-231527-ctest` and `20260822-234647-ctest` respectively.
- Repair: use the repository's established 300-second long-test allowance for
  this composite compile/assemble/two-VM/negative-diagnostic contract.
- Focused proof: the repaired test passes normal Debug in 12.24 seconds at
  `cmake-build-debug/asan-logs/20260822-234817-ctest` and leak-enabled Linux
  ASan/LSan in 33.87 seconds at
  `cmake-build-debugasan/asan-logs/20260822-234906-ctest`.
- Acceptance: the focused test must pass in normal Debug and leak-enabled
  Linux ASan/LSan, followed by the complete supported Linux sanitizer gate.

### SAN-QA-001 — ASan static control kernels require PIC

Status: closed; the exact normal-Debug and Linux ASan/LSan fixture and the
complete supported Linux sanitizer gate pass on `e3de72939`.

- Surface: the test-only `rcc5f_stats_kernel` and `rxvector01_kernel` static
  controls in `tests/performance/CMakeLists.txt`.
- Failure: the Linux sanitizer fixture could not link either static kernel
  into its direct-control shared plugin because ASan instrumentation introduced
  non-PIC `R_X86_64_PC32` relocations.  GitHub Sanitizer QA run
  [32596937877](https://github.com/adesutherland/CREXX/actions/runs/32596937877)
  and local run `cmake-build-debugasan/asan-logs/20260822-213156-full` retain
  the exact `recompile with -fPIC` diagnostics.
- Repair: declare `POSITION_INDEPENDENT_CODE ON` on both static kernel targets.
  These controls are already linked into module libraries; the property makes
  that existing target relationship valid under instrumented ELF builds and
  does not change production providers or benchmark work.
- Focused proof: the complete linked-runtime fixture passes normally in
  `cmake-build-debug/asan-logs/20260822-221819-ctest`.  The first repaired ASan
  replay rebuilt 924/1,251 invalidated artifacts before reaching the fixture's
  1,800-second CTest timeout without a link or sanitizer error; its retained
  incremental replay passes in
  `cmake-build-debugasan/asan-logs/20260822-230331-ctest`.
- Closure proof: the supported Linux ASan/LSan build plus CTest gate and
  GitHub Sanitizer QA pass on `e3de72939`.

## Closed campaign findings

### SAN-013 — native command test mock reads past short commands

Status: **closed, 12 October 2026**. Owner: Codex. Affected inputs: the new
opt-in mainframe ADDRESS test harness on `develop`, based on
`54f736804de08684852c63eb7673435157ef1e30`.

- Original trigger: maintained Apple ASan
  `tools/asan-run.sh --phase ctest --regex '^mainframe_address_' --leaks off
  --test-jobs 1 --no-live-tail`, run `20261011-204239-ctest`. Five of eight
  tests report heap-buffer-overflow in `tests/native_boundary.c`.
  Full diagnostic: `cmake-build-debugasan/asan-logs/20261011-204239-ctest/ctest.log`.
  Apple LSan is unsupported; this is an ASan memory finding.
- Cause: the CMS and TSO native boundary mocks compare seven bytes against
  `BAD@CMD` without checking the supplied native command length. The ordinary
  four-byte `TIME` command exposes the read past its allocation. The mock is
  first-party test code; production command adapters are not implicated by
  this diagnostic.
- Repair: check the native command extent/length before the exact comparison.
  The existing policy and real-script fixtures retain the short-command
  original trigger; no sanitizer suppression or test exclusion is introduced.
- Required proof: focused Debug and maintained ASan original-trigger replay,
  followed by supported Linux ASan/LSan and the applicable full platform gates
  on the final reviewed inputs. Local focused success alone does not close
  this finding or establish release qualification.
- Focused repair evidence: eight of eight Debug tests pass at
  `/tmp/crexx-mainframe-short-command-tests.log`; the identical original-trigger
  panel passes eight of eight in maintained Apple ASan run
  `20261011-204858-ctest`. The repaired build is `20261011-204821-build`.
  The strengthened final panel passes eight of eight in fresh Linux ASan/LSan
  run `20261011-205056-ctest`, after regenerating stale bytecode whose preserved
  source timestamp had prevented a rebuild. That stale fixture returned its
  file-content mismatch code without a sanitizer diagnostic. The full normal
  macOS gate passes 3,151 of 3,151 tests in
  `/tmp/crexx-mainframe-normal-full-tests.log` (3,539.72 seconds). Full macOS
  ASan `20261011-212216-full` completes 3,137/3,139; two B fixtures return 19
  without a memory diagnostic because their bytecode predates the strengthened
  source. Missing directory QA tier finalization prevented qa-prep from
  rebuilding those prerequisites. That wiring is repaired, and the completed
  fresh preparation followed by `20261011-234830-ctest` passes all 11 current
  ADDRESS/allocation checks. The fixtures retain all four guest libraries.
  Earlier full Linux ASan/LSan `20261011-205105-full` passes 3,133/3,133.
  After the RXPA exhaustion and fixture preparation delta, focused Linux
  ASan/LSan `20261011-232037-ctest` passes all 11 allocation/ADDRESS checks.
  Full final normal macOS passes all 3,338 checks in
  `/tmp/crexx-mainframe-normal-final-full-tests.log` (3,786.04 seconds).
  Final macOS ASan `20261012-002213-full` passes 3,140/3,140 in
  6,593.15 seconds; the maintained driver completes at RC 0. Apple build/test
  leak detection is explicitly off. Final Linux ASan/LSan passes 3,134/3,134
  in 6,563.10 seconds; its maintained driver completes at RC 0.
  The first macOS final build tried unsupported build-phase leak detection;
  its replay explicitly uses both `--build-leaks off` and `--leaks off`.
  Linux `20261011-232133-full` stopped after 51 executed checks (50 pass)
  because macOS AppleDouble `._*.c` metadata entered the diagnostic-catalogue
  source glob. Its UNICODE_ERROR is not an ASan finding. Eight individually
  verified transfer sidecars were removed; all 10,484 intended source hashes
  still match. Reconfigured catalogue replay passes with leak checks in
  `20261012-001728-ctest`. Full Linux replay `20261012-002225-full` completes
  with all selected checks passing. After completion, all 10,484 source files
  still match the allocation-final snapshot, SHA-256
  `4dd9c46480685dbbedc6ae88e91c248db9a67a81de2a94276e9c42ed0872de3f`.
  The original short-command/allocator inputs are unchanged by the separately
  reviewed native-provider amendment. Native and PDOS guest qualification
  remains a separate boundary; closure here is the host mock memory finding.
- Final sanitizer commands: Apple uses `tools/asan-run.sh --phase full
  --build-dir cmake-build-debugasan --leaks off --build-leaks off --build-jobs 4
  --test-jobs 8 --exclude-label performance-measurement --no-live-tail`. Linux
  uses the same full phase with `--build-dir build-asan-clang --leaks on
  --build-leaks on --build-jobs 4 --test-jobs 8 --keep-going
  --exclude-label performance-measurement --no-live-tail`.
  Logs and run environments are retained under the named run directories;
  Linux copies are in `output/mainframe-address-linux/final-full-20261012`.
  No sanitizer suppression or new product-test exclusion closes this item.

### SAN-005 — linked-list test exits with owned nodes

Status: closed using the plugin's documented teardown; focused Linux Debug and
ASan/LSan qualification and the complete supported Linux gate pass on
`e3de72939`.

- Surface: the optimized and unoptimized linked-runtime variants of
  `lib/plugins/llist/llist_test.crexx`; plugin allocation and API semantics are
  unchanged.
- Failure: the exact-commit full Linux ASan/LSan run at
  `cmake-build-debugasan/asan-logs/20260823-030158-full` reports 1,230 leaked
  bytes in ten allocations for both `llist_test_noopt` and `llist_test_opt`.
- Root cause: the test creates 20 nodes, removes ten, then exits while still
  owning the other ten. The two five-object LSan groups correspond to the
  remaining prepend and append chains.
- Permanent reproducer: the two existing linked-list test variants pass
  functionally in ordinary Debug at
  `cmake-build-debug/asan-logs/20260823-042146-ctest` but reproduce the exact
  leak in isolation at
  `cmake-build-debugasan/asan-logs/20260823-043336-ctest`.
- Repair: call the documented `llist.freellist(0)` operation after the active
  queue/pull checks and before program exit. This preserves the intended ten
  live-node intermediate behavior while satisfying the caller-owned teardown
  contract.
- Focused proof: both variants pass ordinary Debug at
  `cmake-build-debug/asan-logs/20260823-043406-ctest` and leak-enabled Linux
  ASan/LSan at `cmake-build-debugasan/asan-logs/20260823-043420-ctest`.
- Acceptance: both linked-runtime variants and the complete supported Linux
  ASan/LSan build plus CTest gate must pass with leak detection enabled.

### SAN-004 — statement-form parallel pending-result class ownership

Status: closed with a permanent minimal regression; focused Linux Debug and
ASan/LSan qualification, the original pooled-HTTP trigger, and the complete
supported Linux gate pass on `e3de72939`.

- Surface: `compiler/rxcp_task_lower.c`, teardown of pending typed-object task
  results created by statement-form `DO PARALLEL` lowering.
- Failure: four direct `strdup()` leaks totalling 80 bytes while the Linux x64
  sanitizer build compiles the optimized `ts_http_pooled` images.
- Root cause: `task_pending_add()` owns copies of both `handle_name` and
  `result_class`.  Parallel block-expression teardown released both, but the
  statement-form success and failure paths released only `handle_name` before
  freeing the pending array.
- Trigger: Linux x64 Sanitizer QA run
  [32593095785](https://github.com/adesutherland/CREXX/actions/runs/32593095785)
  fails while generating the optimized `ts_http_pooled` images with
  `SUMMARY: AddressSanitizer: 80 byte(s) leaked in 4 allocation(s)`.
- Affected revision: `653c1293f1b2f84faa75e86bddd50cce6979723a`.
- Permanent reproducer:
  `compiler/tests/rexx_src/san004_task_pending_object_result.crexx` assigns two
  typed-object task results inside statement-form `DO PARALLEL`; its direct
  optimized compiler test is labelled `san-004` and `sanitizer`.
- Repair: centralize pending-result teardown in `task_pending_clear()`, release
  both owned strings and the array on every statement/expression success or
  failure path, and reset the plan's pending storage state.
- Focused proof: the unfixed normal-Debug control passes in
  `cmake-build-debug/asan-logs/20260822-210428-ctest`, while the same unfixed
  leak-enabled cell reports two direct allocations totalling 20 bytes in
  `cmake-build-debugasan/asan-logs/20260822-210434-ctest`.  The repaired cells
  pass in `20260822-210534-ctest` and `20260822-210540-ctest`.
- Original-trigger proof: the repaired leak-enabled `ts_http_pooled` target
  regenerates all four pooled-HTTP images without a sanitizer finding in
  `cmake-build-debugasan/asan-logs/20260822-210605-build`; the immediately
  repeated incremental proof passes in `20260822-213117-build`.  The related
  normal-Debug panel passes 14/14 in `20260822-211330-ctest`, covering the
  minimal compiler cell, all four pooled-HTTP runtime modes, Level-G task
  lowering, and imported typed-object task methods.
- Output proof: the optimized reproducer RXAS is byte-identical before and
  after the repair, with SHA-256
  `5d34d6c7878e073bcd1d34bb635ad2da872499079ea904a4433025db379ca2a1`.
- Closure proof: the original optimized `ts_http_pooled` trigger and the
  complete supported Linux ASan/LSan build plus CTest gate pass on
  `e3de72939`.
- Closure: retained CI and minimal unfixed LSan evidence, repaired focused
  Debug/ASan test, byte-identical optimized RXAS, the original pooled-HTTP
  build trigger, applicable task/parallel tests, and a complete supported Linux
  ASan/LSan build plus CTest gate.

### SAN-003 — inline capture symbol shape replaced without releasing ownership

Status: closed with permanent regressions; focused Linux Debug and ASan/LSan
qualification and the complete supported Linux gate pass on `e3de72939`.

- Surface: `compiler/rxcp_remap_build.c`, value-shape copying into an existing
  inline capture symbol.
- Failure: eight direct four-byte leaks from `rxcp_remap_copy_dims()` while the
  optimized compiler builds `lib/rxfnsb/rexx/trace.crexx`.
- Root cause: fixed-point inlining can reuse a deterministic temporary symbol.
  The value-shape copy helpers replace its owned dimension metadata with fresh
  allocations without first releasing the previous shape.
- Trigger: Linux x64 Sanitizer QA run
  [32585975224](https://github.com/adesutherland/CREXX/actions/runs/32585975224)
  fails during the instrumented build of `lib/rxfnsb/rexx/trace.rxbin` with
  `SUMMARY: AddressSanitizer: 32 byte(s) leaked in 8 allocation(s)`.
- Affected revision: `62c96aa4d2435f8479924393cee7922a68775985`.
- Permanent reproducer:
  `compiler/tests/rexx_src/san003_inline_scoped_array_actual.crexx`, registered
  in optimized and unoptimized runtime modes.  Optimized compilation repeatedly
  attempts a method call that passes a class-attribute `.string[]` by value,
  creates the deterministic scoped-argument capture symbol, then rejects the
  candidate at the profitability gate.  The runtime cells retain a semantic
  result check.
- Repair: allocate the complete replacement value shape first, release the
  existing symbol-owned dimensions and class only after allocation succeeds,
  then install the new shape.  This preserves the supported optimized inline
  attempt and also leaves the old shape intact if allocation fails.
- Focused proof: the unfixed optimized compile reports the same 32-byte/eight-
  allocation leak in
  `cmake-build-debugasan/asan-logs/20260822-191323-ctest/ctest.log` while the
  normal-Debug semantic controls pass in `20260822-191337-ctest`.  The repaired
  direct compile and original `trace.crexx` build trigger pass leak-enabled in
  `20260822-192120-ctest` and `20260822-192126-build`; the combined permanent
  SAN-003 and four-cell `rxfs` panel passes 7/7 in `20260822-192836-ctest`, and
  29 applicable inliner tests pass in both normal Debug and Linux ASan/LSan in
  `20260822-192909-ctest` and `20260822-193021-ctest`.  The complete ordinary
  Linux Debug build and 2,362/2,362 CTest gate pass in
  `cmake-build-debug/asan-logs/20260822-193230-full`.
- Output proof: the optimized reproducer RXAS is byte-identical before and
  after the repair, with SHA-256
  `5fda0341f2af6b01e9236235b48eff831511221a3a8b65eaf23c0284e087c096`.
- Closure proof: the complete supported Linux ASan/LSan build plus CTest gate
  passes on `e3de72939`.
- Closure: retained unfixed LSan evidence, repaired focused Debug/ASan test,
  byte-identical optimized RXAS across the repair, the original `trace.crexx`
  build trigger, applicable inliner tests, and a complete supported Linux
  ASan/LSan build plus CTest gate.

### SAN-001 — RXAS SSA value pointer retained across growth

Status: closed for sanitizer assurance; the repair remains accepted for RCC-5
publication and the complete supported Linux gate passes on `e3de72939`.

- Surface: `assembler/rxas_flow_ssa.c`, `rxas_flow_value_node()`.
- Failure: heap-use-after-free at the write to `version->source_value_id` after
  `flow_ssa_resolve_value()` can append to and reallocate `value_versions`.
- Trigger: full Apple-ASan build while generating
  `benchmark_awfy_towers_opt.rxbin`.
- Affected revision: `298f412dc0e40ef12b4957df4b5f8b57a8a14d9f` plus the
  current approved RCC-5 decimal worktree.
- Retained logs:
  `cmake-build-debugasan/asan-logs/20260820-151050-ctest/ctest.log` and
  `cmake-build-debugasan/asan-logs/20260820-205831-full/build.log`.
- Permanent reproducer: `test_ssa_value_node_growth()` in
  `tests/test_rxas_flow_graph.c` constructs exactly 64 lazy integer-copy values
  and resolves the first source across the backing-array growth boundary.  The
  unfixed normal-Debug control fails at the intended assertion in
  `cmake-build-debug/asan-logs/20260821-095107-ctest/ctest.log`, and the unfixed
  Apple-ASan control reports the original heap-use-after-free in
  `cmake-build-debugasan/asan-logs/20260821-101842-ctest/ctest.log`.
- Repair: retain the stable source value ID and source attributes, perform the
  growth-capable resolution, and reacquire `value_versions[value_id]` before
  writing or reading the record.  An audit of adjacent `FlowValueVersion *`
  accessors found no other pointer retained across an append-capable call.
- Focused proof: repaired normal Debug and Apple-ASan pass in
  `20260821-101929-ctest` and `20260821-101939-ctest`; optimized Towers,
  bytecode/semantic equivalence, and 70 applicable RXAS flow/proof contract
  tests pass in normal Debug and Apple-ASan in `20260821-102446-ctest` and
  `20260821-102451-ctest`.
- Performance: Adrian accepted the profiling-off Release verdict on
  2026-08-21.  The retained 36-pair assembly result is neutral around zero,
  clears the 3% guard, preserves exact output and bytecode, and is recorded in
  [`performance/evidence/2026-08-21-san-001-ssa-release-verdict/`](https://github.com/adesutherland/CREXX/tree/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/performance/evidence/2026-08-21-san-001-ssa-release-verdict).
- Platform proof: the earlier complete Apple-ASan build and 2,310/2,310 CTest
  gate pass in `cmake-build-debugasan/asan-logs/20260821-112920-full`. The
  consolidated current RCC-5 build and 2,356/2,356 CTest gate also pass with no
  sanitizer report in `20260822-104056-full`; its instrumented build includes
  the original optimized Towers trigger.
- Closure proof: the supported Linux ASan/LSan gate passes on `e3de72939`.
- Closure: focused Debug/ASan regression, optimized Towers artifact, applicable
  RXAS contract tests, ordinary Release performance verdict, and complete
  Apple-ASan plus supported Linux ASan/LSan gates.

### SAN-002 — RXVM string-buffer ownership across return and reuse

Status: closed for sanitizer assurance; the `f95f906de` repair remains
accepted for RCC-5 publication, its permanent regressions remain active, and
the complete supported Linux gate passes on `e3de72939`.

- Surface: decimal-to-string/extract capacity calculation for a value produced
  under a wider numeric context and returned to a narrower caller context.
- Root cause: the former `getRequiredStringSize()` used only the active
  context's digits.  A returned 32-digit value in an 18-digit caller therefore
  received a 32-byte standard slot even though DEXTR writes a 33-byte
  coefficient including the terminator.  The current value-aware contract
  sizes from the greater of the active context and stored `decNumber` digits.
- Manifestation A: the DEXTR overrun entered an adjacent live destination slot;
  preparing that destination for `string_concat_var_const()` poisoned the slot,
  and the subsequent 33-byte source copy reported a use-after-poison read.
- Manifestation B: the same DEXTR overrun entered an adjacent slot already
  poisoned during ordinary return/value reuse and reported a use-after-poison
  write in `mc_decimal.decimalExtract()`.
- Trigger: the temporary `rcc5_allocator_diag` test used while qualifying
  RCC-5B.  Because that diagnostic was not retained, absence from later focused
  tests is not closure evidence.
- Affected revision: the context-only sizing used in the work leading to
  `f95f906de`; the value-aware repair is present in that revision and current
  HEAD.
- Retained logs:
  `cmake-build-debugasan/asan-logs/20260820-113550-ctest/ctest.log`,
  `cmake-build-debugasan/asan-logs/20260820-113825-ctest/ctest.log`, and
  `cmake-build-debugasan/asan-logs/20260820-114917-ctest/ctest.log`.
- Permanent regressions:
  `tests/sanitizers/san002_concat_after_wide_decimal.crexx` and
  `tests/sanitizers/san002_wide_decimal_extract.crexx`, registered optimized
  and unoptimized under both `rxbvm` and `rxtvm`.
- Counterfactual proof: temporarily restoring only the former context-only
  sizing makes all eight Apple-ASan cells fail.  The concat case reports the
  retained `string_concat_var_const()` use-after-poison read and the extract
  case reports the retained `decimalExtract()` use-after-poison write in
  `cmake-build-debugasan/asan-logs/20260821-110014-ctest/ctest.log`.  The
  production file was then restored byte-for-byte.
- Focused proof: final repaired normal Debug passes 8/8 in
  `cmake-build-debug/asan-logs/20260821-110131-ctest/ctest.log`; identical
  Apple-ASan passes 8/8 in
  `cmake-build-debugasan/asan-logs/20260821-110400-ctest/ctest.log`.
  The retained `ts_math_numeric` family also passes optimized/unoptimized under
  both VMs in normal Debug and Apple-ASan (`20260821-110453-ctest` and
  `20260821-110620-ctest`).
- Classification: both retained manifestations are consequences of this one
  output-capacity defect, so no separate SAN-003 is required.  No production
  edit was needed during reconstruction, and therefore no new first-Release
  performance verdict was triggered beyond the already accepted RCC-5B/C
  verdicts.
- Platform proof: the earlier complete Apple-ASan build and 2,310/2,310 CTest
  gate pass in `cmake-build-debugasan/asan-logs/20260821-112920-full`. The
  consolidated current RCC-5 build and 2,356/2,356 CTest gate also pass with no
  sanitizer report in `20260822-104056-full`, including all eight permanent
  SAN-002 cells.
- Closure proof: the supported Linux ASan/LSan gate passes on `e3de72939`.
- Closure: permanent optimized/no-opt tests under both concrete VMs, focused
  Debug/ASan checks, any required ordinary Release performance verdict, and the
  complete platform sanitizer gates.

## Closed historical findings

Historical failures remain evidence rather than live debt only when their
repair and broad closure are retained:

- the RXAS sparse proof-snapshot use-after-free was fixed in `6cde2c509` and
  followed by complete Debug/ASan 1,999/1,999;
- the Apple-ASan provider unload/reload global-buffer-overflow and allocator
  extent poison handling were fixed in `f95f906de` and their focused triggers
  passed;
- the `rxtcp.tcpreceive` 10,000-byte leak was fixed in `d78c6fcfa` and followed
  by complete Linux ASan/LSan 1,925/1,925; and
- the `keyaccess.readkey()` four-byte leak and sibling parser-tool resource
  leaks have retained leak-enabled closure documented in
  `docs/ai-context/CREXX_ASAN_TESTING.md`.

Closed entries must not be deleted when this worklist is updated.  Move each
resolved live item into this historical section with its permanent regression,
repair commit or worktree identity, exact final commands, and broad result.

## CI enforcement

`.github/workflows/sanitizers.yml` currently runs full Linux x64 ASan/LSan
and macOS arm64 ASan for version tags and default manual dispatch. Scheduled
invocations resolve current `develop` and skip an exact SHA already qualified by
both full platform jobs. Ordinary pushes and pull requests do not invoke this
workflow; full sanitizer jobs are not universal develop publication gates.
The explicit `normalization-linux` dispatch is an isolated diagnosis route and
cannot write a broad-assurance marker. Linux build/test leaks stay enabled;
Apple LeakSanitizer is unsupported.

The effective scheduled definition is still on `master`, with its existing
240-minute job guard. Hotfix has the 360-minute guard and focused dispatch route;
this is not scheduled activation. The approved daily/preparation changes remain
subject to clean platform qualification and the separate narrow master-workflow
decision. Hosted run links establish observed results; local policy or a stable
job name alone does not establish successful execution or branch protection.

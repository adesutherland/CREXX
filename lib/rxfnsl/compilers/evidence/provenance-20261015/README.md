# Essential metadata and condition-provenance correction

15 October 2026, macOS ARM64. Baseline develop:
`bf74c72111fbd466d1f93c421e6d5dadbbedcc2c`. This receipt covers the approved
CP-AC01–06 / CP-STEP01–03 increment in [PLAN.md](../../PLAN.md). Implementation
and focused local qualification are complete. Adrian subsequently authorised
develop publication and CI-monitor handover on 15 October 2026. This does not
claim hosted CI, full TRACE, full condition semantics or replacement
readiness. [inputs.json](inputs.json) identifies the qualified source/test/build
bytes, executable images, target libraries and exact CRLF fixture blobs.

The publication check matched all 150 retained source/test/build and executable
hashes, plus both CRLF Git blob identities. The product build, focused CTests
and Apple ASan evidence below therefore remain valid without repeating them.
This first substantive push includes the standing emission-quality guidance,
review, implementation, tests, examples and qualification documentation. The
normal automatic Build and CodeQL runs are monitored separately; the previously
reported missing `bin/llcompilers.rxbin` package dependency remains open and
is not repaired or waived by this publication.

## Result and acceptance

All compiler and adapter changes are Level B, and RXAS still goes through the
Level L writer. There are no C/VM changes, new language forms, condition objects,
stack mechanisms or IR/source-map formats. The existing Classic function-level
handler and its necessary authored-context register remain.

| Criterion | Verified evidence |
|---|---|
| CP-AC01: logical source names | Service tests cover Unix, Windows and UNC absolute paths, relative logical names, CLI basename policy, memory input and deliberate disambiguation. Two physical roots produce identical output in both the service and CLI. Syntax diagnostics use the logical name too. |
| CP-AC02: honest source records | B service assertions cover full single-line spans, Unicode/CRLF, first-line multiline composite projection, derived/synthetic flags, empty entry anchors and immutable full IR ranges. Real generated setup/implicit returns now use derived origins. Every emitted clause ID in the focused corpus is zero; real clause grouping remains open. |
| CP-AC03: fault and authored context | Recursive/deep helper C/G failures, calls and changing anchors through loops, generated-entry injection, and simple paired errors retain the authored context and original fault. Classic fault identity is `RexxClassicBifs`, not the formatting helper; source lines are checked when retained in that runtime package. |
| CP-AC04: admitted error contract | Four injected VM-bound string faults verify CLASSIC_SYNTAX / DIVISION_BY_ZERO / CONVERSION_ERROR / INVALID_ARGUMENTS, message, original source anchor, nonzero statuses 28/5/6/7 and no handler recursion. Real division preserves `RXC-LC-42.3` / C status 28 and G status 5. The existing numeric-policy cases pass; ignored-signal code is unchanged. The [payload audit](../../doc/CONDITION-PROVENANCE-REVIEW.md) explicitly excludes general object payloads. |
| CP-AC05: diagnostic state | Permanent checks reject repeated unchanged source records and location loads within a straight path, resetting at labels/functions. Entry fallback names the logical file/routine. [Shape comparison](shape-comparison.json) separates source records, context loads, executable instructions, calls and frames. |
| CP-AC06: proof and docs | Product build; both B-host modes, both assembler modes and both concrete VMs; focused Apple ASan; stripped-source and skipped-error cases; independent requests; reusable package consumer; refreshed source/IR/RXAS examples and user/architecture guides. Broader gates remain explicit in the plan. |

## Executed checks

- [Product build](product-build.log): `stage-product`, separate `llrxc-c`/`llrxc-g`,
  `llcompilers`, `llctarget`, and affected frontend/backend consumers. No warnings
  in this normal incremental build.
- [31 CTests](focused-ctests.log): 24 frontend syntax/grammar/mapping/call/contract/
  lifecycle tests, four existing backend aggregates, three new provenance
  aggregates. All passed. Each backend aggregate retains its 31-program,
  232-command corpus, both assembler modes and service/literal/error checks;
  complete command receipts are `llcompiler_backend_{mode}_{vm}.json`.
- The fourth provenance profile, no-opt host / `rxbvm`, already passed as the
  isolated [Debug measurement](debug-measure.json), [log](debug-measure.log),
  **90 commands in 28.205 seconds**. Its exact test/source bytes match the
  registered test, so it was reused instead of repeated. The other three
  `llcompiler_provenance_*.json` receipts each retain 90 commands, taking
  29.5–29.8 seconds. These are scheduling observations, not compiler benchmarks.
- [Focused Apple ASan](asan-measure.json), [result](asan-result.log): the same
  90-command no-opt / `rxbvm` profile passed in **89.676 seconds** using
  `tools/asan-run.sh --phase build --build-target llcompiler_provenance_measure
  --build-leaks off --no-live-tail --tail-lines 20`. No sanitizer diagnostic was
  reported in this bounded run. Apple LeakSanitizer is unavailable; no broad or
  cross-platform sanitizer claim is made.
- [CTest registration](ctest-registration.json): four provenance lanes, each
  `RUN_SERIAL`, shared `llcompiler_backend` resource lock, 900-second whole-test
  timeout and 180-second nested-tool hang guards. Producer targets are registered
  for normal test preparation. Registration followed the isolated measurements.
- [Reusable compiler package](service.json), [result](service.log): compile a B
  consumer against only `llcompilers`, `rxfnsl` and `library`, assemble/link it,
  then pass all 50 backend/service assertions. This checks that the changed
  internal helper is present in the reusable package, not only modular tests.
- [Examples](examples.json): 28 commands regenerate the four existing literal/
  pipeline IR and RXAS walkthroughs, execute their expected output, and compile/
  execute the two new simple error examples through the built command wrappers.
  Literal C/G still print `01` / `1`; both pipelines print `10`, `0`, `3`.
  Errors print `1` then stop with the respective C/G failure status.

The four provenance profiles share eight source fixtures and four injected
fatal names; repeated host/assembler/VM executions are not distinct programs.
The B assertions additionally exercise direct-IR provenance, logical names and
output ownership. Both `unicode-*.crexx` fixtures retain their exact four CRLF
line endings under a scoped `.gitattributes`; clean-filter Git blob identities
match the tested bytes. No extra run was needed for that byte-preserving rule.

## Readable output quality

Counts describe emitted RXAS before assembly. Instruction lines exclude labels,
imports and directives; calls/frames include the complete module. Context-load
counts include generated/routine-entry strings as well as numeric source sites.
The baseline is the versioned STEP04 output at `bf74c7211`; current examples are
actual regenerated output. [Full counts](shape-comparison.json):

| Example | Instructions before → after | Source records | Context loads | Call sites |
|---|---:|---:|---:|---:|
| Literal C | 78 → 90 | 11 → 8 | 12 → 6 | 10 → 10 |
| Literal G | 12 → 12 | 6 → 6 | 0 → 0 | 1 → 1 |
| Pipeline C | 551 → 537 | 99 → 59 | 101 → 51 | 55 → 55 |
| Pipeline G | 90 → 90 | 69 → 51 | 0 → 0 | 3 → 3 |

Frames are unchanged. The origin-preserving fatal handler adds 18 cold
instructions per Classic routine for literal-name dispatch and re-raising;
straight-path context loads fall. Thus the tiny Classic literal module grows
by 12 instructions even though repeated context updates halve. The larger
Classic pipeline shrinks by 14 instructions overall. This accounts for the
tradeoff rather than claiming every output gets shorter or faster. Helper and
numeric-context work is unchanged; fewer caller lines are not a total-cost
or runtime-speed measurement. The new empty source anchors also account for
why the literal G source-record count does not fall.

The prior [native B/G/C comparison](../emission-review-20261015/README.md) remains
valid as a historical control: the native compiler, assembler, linker, VM and
its two runtime package hashes are unchanged. Its inefficiencies are not an
acceptance target. Unused default construction, frame windows, entry jumps,
clause/TRACE fidelity and shared optimisations remain open in the plan.

## Error observations and limits

Captured [C error](error-c.stderr): authored context `error-c.crexx:3:5`, code
`RXC-LC-42.3`, original fault module `RexxClassicBifs`, address 3989 in this image,
and its source at `RexxClassicBifs.crexx:505:15`. The B formatting helper no
longer replaces the fault. [G error](error-g.stderr): DIVISION_BY_ZERO at
`error-g.crexx:3:5`. These numeric module/address values belong to these images,
not a stable API identity across builds.

An explicit no-source entry may print `source: :0:0:` in the existing VM's
one-caller display. This is recorded honestly instead of assigning the previous
routine's source to generated code. The assembler's binary-module label can
still include its physical RXAS path; logical compiler source metadata is a
separate field. With linker stripping, fault module/address remains and source
text is absent. Classic's authored-context message can still be present.

Full clause grouping, TRACE events, structured condition payloads, complete
stack traces and INTERPRET code/source lifetime remain later work. Compiler
commands and `llctarget.rxbin` must be rebuilt together because the internal
adapter now exposes `failuremessage` and the emitted handler performs the raise.

## Findings during qualification

The first no-opt frontend rebuild found an old shared-directory contract image
shadowing the newly built mode-specific image. Import lookup now searches the
declared mode directory first. The stale unrelated file was preserved. Focused
frontend tests and both host builds verify the correction.

The maintained sanitizer tree needed CMake regeneration before its newly added
measurement target existed. Its first measured test then exposed a harness
assumption: Asan configuration strips runtime source records (`rxfnsc` retains
them only for Debug). Original fault module/address was correct. The harness
now verifies fault identity regardless of retained source text, and checks the
source line when available. The unchanged product then passed the focused
rerun above. No product sanitizer finding arose from that assertion failure.
The initial prerequisite refresh also emitted existing native `sprintf`
deprecation warnings; no C code was changed or warning suppression added.

The already reported STEP04 hosted package dependency failure is outside this
correction. No CI repair, rerun, broad matrix or branch switch was performed.
The later publication approval covers this qualified correction and monitor
handover. The ordinary source build and focused results above do not claim
that the known hosted delivery gate is closed.

# RXC-PROJECT-01 qualification record

2026-09-06. Both Release verdicts were accepted by Adrian. This record covers
the final compiler, wrapper, regression tests and installed offline checks.
The implementation is based on `e0e67ad3e89e19f98ab09676ffa2bbc99fad7010`,
developed in isolation on `temp/rxc-project-scaling` and integrated locally
into `develop`.
No remote publication or global installation is authorized or performed.

## Result and mechanism

The frozen 48-member application takes **509.45 -> 70.98 seconds** in the
accepted, uncontended Release comparison: 86.1% less elapsed time, 7.18x.
Normal optimization and source/TRACE/callable metadata remain enabled.
Maximum process RSS is essentially unchanged (586,940,416 -> 590,643,200
bytes); this is not aggregate memory across concurrent processes.

| Member | Baseline elapsed | Accepted candidate elapsed | Baseline / candidate maximum RSS |
| --- | ---: | ---: | ---: |
| crexxrag_cli | 172.91 s | 17.65 s | 415,907,840 / 423,362,560 bytes |
| ragproduct | 173.38 s | 19.14 s | 586,940,416 / 590,643,200 bytes |
| ragmcp | 133.95 s | 14.92 s | 411,451,392 / 412,975,104 bytes |

These are individual workload observations, not a statistical portfolio claim.
The [first verdict](../2026-09-06-rxc-project-scaling-first-verdict/README.md)
and [second verdict](../2026-09-06-rxc-project-scaling-forward-verdict/README.md)
retain all member elapsed/user/system/RSS data, compiler argv, artifact hashes,
host/compiler configuration and comparison boundaries. The final environment
keys and RXAS post-wave checks were correctness refinements after the verdict;
they do not change either compiler performance repair. The final application
qualification ran beside ASan QA, so its elapsed figures are retained but are
excluded from this comparison.

There were three distinct causes:

1. Inline eligibility repeatedly asked whether a local contract existed. That
   query descended into executable AST bodies which grew during optimization.
   It now inspects live file-level declarations, where normalized source permits
   classes/interfaces. There is no eligibility cache, new cutoff, disabled
   optimization or validation relaxation.
2. Binary function signatures could refer to classes whose metadata existed
   but whose stubs were not registered yet. Resolving `_rxsysb.addressrequest`
   then loaded RAG's same-namespace ADDRESS source and nearly all its imports.
   A stack scoped to the current binary import recognizes that module's forward
   declarations. Metadata, eager inline payload attachment and normal consumer
   validation remain intact. In `provider_contract`, 46 unnecessarily loaded
   source candidates become namespace-only candidates.
3. The old wrapper derived every member key from all project source contents.
   Any edit therefore selected all members. `rxc` now writes and checks private
   dependency snapshots; the wrapper owns tool/action keys and final linking.
   Imported implementations remain full-content dependencies because they can
   be inlined. Unloaded source candidates retain namespace-header dependencies.
   Binary candidates are conservatively fingerprinted. Users maintain no list
   of implicit dependencies.

The tiny permanent `binary_forward_dependencies` regression fails on the
predecessor because an unused `_rxsysb` extension body invalidates the snapshot.
With the repair, unused body edits remain current; actually used extensions
still invalidate and execute correctly, and excess arguments are rejected in
optimized and unoptimized modes. `project_dependencies` and
`crexx_project_build_contract` cover source/header/root changes, source/binary
shadowing, RXAS mtime precedence, corrupt snapshots, options/tool identities,
implicit installed candidates, environment presence/value changes and a
deterministic mid-wave mutation which must preserve the published artifact.

## Separate ADDRESS library comparison

After all QA and application matrix work ended, the exact separate ADDRESS
wrapper route from the frozen CMake file ran serially against the baseline
and final matched packages. Only source/output/install paths were adapted.
Normal optimization, all metadata and `--jobs 1` were retained; each run had
its own 900-second process-tree bound and completed successfully.

| Measurement | Clean e0e67 baseline | Final qualified tools |
| --- | ---: | ---: |
| Whole library wave elapsed | 156.18 s | 11.53 s |
| Whole wave user / system | 155.47 / 0.49 s | 11.35 / 0.11 s |
| Member rxc elapsed | 155.84 s | 11.15 s |
| Member rxc user / system | 155.19 / 0.43 s | 11.03 / 0.08 s |
| Maximum process RSS | 402,817,024 bytes | 402,046,976 bytes |

The whole-wave reduction is 92.6% (13.55x), with essentially unchanged RSS.
`address-comparison.json` and the adjacent baseline/qualified directories
retain actual member argv, PIDs, native time/RSS and return codes. This is a
single paired observation on macOS ARM64, not a statistical generalization.

## Final application invalidation matrix

The final compiler/wrapper completed the following checks on the full frozen
application copied to scratch. Each cell retains its source hash, selected
member names, return code and timing in `incremental-matrix.json`; exact child
argv files are indexed by `member-capture-index.json`. ASan QA ran concurrently,
so these durations are qualification evidence only.

| Change | Members compiled | Repeat with identical inputs |
| --- | ---: | ---: |
| Unchanged clean output | 0 | 0 |
| Private `ragimprove._boundtext` body edit | 14 | — |
| Public method added to exported `ragimproveplan` | 14 | — |
| Original source restored | 14 | — |
| `--diagnostic-locale en_US` | 48 | 0 |
| Compiler launcher content identity | 48 | 0 |

All cells succeeded. The private edit changes a local initialization inside
an existing procedure; the contract edit adds a method using the established
class syntax. Neither experiment invokes the application. Source and launcher
bytes are restored in a `finally` block and verified against their originals.
The permanent small project test separately proves a one-member independent
edit, observable imported-body changes, contract changes and stale-candidate
rejection. The accepted uncontended private-edit result was 48 -> 14 members,
93.92 -> 48.70 seconds, with a 0.27-second zero-compiler unchanged repeat.

## Provenance and history

The read-only RAG donor is `main` at
`87fdfd8df9f21653d55aacd4365e15d4530cf8f2`, including its 18 prospective-config
corrections and defect document. The 131-file frozen manifest is retained in
the second verdict; its SHA-256 is
`bf8322b4f5c588729d17f1d5070950ee76be590ff6f606b0673511a53a82e921`.
No partially refreshed generated project members served as source baseline.
Temporary edit tests use `donor-qualification`; `donor-frozen` stays immutable.

The original installed compiler and wrapper byte-match the occupied checkout's
Debug binaries, but not a reconstructable clean source revision. `rxc -v`
reports `e908adeae093.dirty`; installed BUILDINFO reports `32efa89a2daa.dirty`.
This is a mixed-age package, not proof that either SHA produced every artifact.
The exact installed compiler/wrapper hashes and the attributable clean e0e67
Release baseline are retained in the first-verdict provenance JSON.

Historical source introduction/escape points are documented in the
[worklist](../../RXC-PROJECT-SCALING-WORKLIST.md): declaration scan
`9c1ed04bf3b27d109e8a1164704c9b383a2d3091` (April 21), imported-source
optimization `182a3f552b208bd60f56508be1def3e7a127aac2` (April 29), reference
eligibility `6d77cbf4bf761af91b410c39a5017e6b7d1f6e4b` (June 9), metadata class
aggregation `42dfa9f511f0b7e4f99a91a0d4da90c16ab0db1e` (April 24), downstream
ADDRESS source `5ba2bd0135aea984e7b921f93ccd57546761e9d6` (August 24), and global
member keys `ad2f98e4413fcd68d18d8268ca29598ef389060e` (September 1). The last
change even added a test expecting a full wave after a comment edit. These are
exact source-history findings, not a complete executable timing bisect or a
single-SHA attribution of the unreconstructable dirty installed compiler.

Final scratch Release artifacts:

| Tool | SHA-256 |
| --- | --- |
| rxc | `67ddcc9e1eed14735a557dac4f3c56359baddf9d884a66f347e1811217a8ff1d` |
| crexx | `44acfb76208a8734286da2b8e7e74440fbdcf261883c312dccaa56b7dee777b5` |
| rxas | `aaa6436bffe5c22d039386c8ca90bad0858e63a2809a54c921c6ee16260dca8b` |
| rxlink | `2165291d224003cce5781a07d740ce8fad29b18bf15524596c56fd1910c84ca5` |

`implementation-sha256.json` identifies every changed production/test input;
`qualified-install-sha256.json` identifies the whole scratch package. These
binaries were built before the local commit and report the base dirty version.
Commit metadata and documentation edits do not change their qualified inputs.
`qualified-matched-package-differences.json` confirms that the timing package
differs from the baseline only in the actual compiler and wrapper.

## Correctness and installed qualification

* Release: 2,189 broad non-measurement checks passed, plus all four repaired
  format checks and the final wrapper contract replay. Parser mode is off.
* Debug: all 2,457 checks passed across the initial 2,397 passes and the 60/60
  replay. Parser mode is on. `qa-prep` intentionally omits measurement inputs;
  building `qa-prep-measurement` supplied the 56 missing generated fixtures.
  Those Debug measurement-test passes are correctness/harness evidence, not
  Release performance results.
* Four older RXBIN fixtures incorrectly called bit `0x40` unsupported after
  `72d376e295783d0f610ed3af87b848af5eae43e5` assigned it to autoload hints.
  They now test unassigned `0x80000000`, retaining precise rejection. Their
  non-timed assertions are labelled codegen so Release/ASan correctness gates
  include them. The loader is unchanged. Original failures and replay logs
  are retained; these were fixture defects, not sanitizer memory findings.
* Apple-ASan: all 2,271 applicable checks pass as a composite of 2,270 broad
  passes and the focused project-contract replay. The expanded contract hit
  its old 180-second aggregate timeout in the initial sweep (no memory
  diagnostic). SAN-QA-013 retains that failure, changes only the harness bound
  to 600 seconds and adds scenario labels. The exact contract then passes
  normal Debug in 77.01 seconds and Apple-ASan in 193.84 seconds. No assertions,
  subprocesses or sanitizer instrumentation are removed. The unchanged broad
  checks are retained instead of rerunning them after this one test's bound
  and logging change. Linux ASan/LSan is not qualified in this task.

The fresh Release install uses a scratch prefix and
`CMAKE_SKIP_INSTALL_ALL_DEPENDENCY=ON`. Product preparation included
`stage-c1-toolchain`, `stage-b1-substrate`, `stage-product`, `stage-optional`,
`vector_static` and `sqlite_static`. Existing stage-target omissions are
recorded rather than silently assuming `stage-product` alone installs the
complete optional-provider package. Broad QA includes the installed external
RXPA SDK consumer and program/library/native wrapper contracts.

The frozen downstream CMake `crexxrag_product` and `crexxrag_address` targets
build with the scratch installed tools. The linked application hash is
`d041bca66a758d767a588fe17dbadbbd7c05355c0d2c20817219ea1e7510d16d`.
Offline results:

* Linked application init/status on new scratch libraries passes on both VMs.
* Supported `crexx -native -nocompile -noexec` packaging and native library
  init/status pass, with statically linked installed providers.
* Configuration format/identity/bounds/glossary/profile tests pass with normal
  optimization and with optimization off, on both VMs; linked doctor,
  configuration explanation and data-profile inspection also pass.
* Separate ADDRESS library OPEN/STATUS/CLOSE passes on both VMs against a new
  scratch library.

The donor's full native target runs worker checks, and its full configuration
and ADDRESS tests run ingestion/maintenance operations. They were not invoked.
The retained scratch harness selects only the offline portions. The original
configuration harness also names retired `rx_system` in a manual VM module
list; only the scratch harness uses current `crexx --program` linking and
provider metadata instead. No dependency or metadata is removed from the
product to make the checks pass. No corpus, credentials, hosted provider,
ingestion, maintenance, workers or donor automation were accessed or run.

## Remaining limits

This is macOS ARM64 proof. Linux and Windows qualification and any hosted
exact-SHA publication gates remain separate. Apple ASan supplies no LSan.
Existing sanitizer-worklist items are not closed by this task's local results.
The downstream runtime checks cover bounded offline surfaces, not operational
ingestion correctness or the separate SQL/ingestion failure.

Dependency reuse is conservative and does not promise a minimal semantic
graph. Arbitrary external reads by custom compiler exits cannot be inferred;
callers must keep those inputs stable and use `--rebuild` when they change.
Repeated unused-import warning AST walks were separately observed and left
unchanged; they remain a possible later compiler optimization.

Raw logs, NUL-delimited argv, process samples, measured outputs and frozen
sources remain under
`/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795`.
`logs.json` supplies retained paths and hashes. The two earlier verdicts are
dated snapshots of their narrower qualification state; this record supplies
the final follow-through.

Raw compiler/build logs retain their original whitespace; code and documentation
pass the whitespace diff check. The CSV is a derived LF-normalized table;
native timing files and source/hash manifests remain unchanged.

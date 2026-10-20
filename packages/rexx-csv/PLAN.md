# rexx-csv — shared package plan

Status: **cREXX, Classic and NetRexx implementation/macOS qualification
complete, 20 October 2026.** All four actual runtimes pass their shared-oracle,
boundary and installed-consumer checks at the versions recorded below.
Linux/Windows results belong to the focused hosted gate described below.
Untested JVMs and live rexxpository integration/publication remain open. Adrian approved D-05 before NetRexx implementation.
This is the authoritative scope and acceptance record. `SPEC.md` defines the
approved shared behaviour; `tests/fixtures/` owns the common, hand-authored test data.

## Vision and intended outcomes

Provide an independently installable CSV source library with the same data
semantics on cREXX, Regina, ooRexx and NetRexx. Users should be able to read a
CSV document into string fields, change those fields and write an unambiguous
CSV document without acquiring a native plugin or a third-party parser.
Interfaces should fit each language while sharing one specification and one
set of expected results. cREXX is the first implementation, not the definition
of correctness for the other implementations.

The approved first-increment scope is an in-memory parser and writer, documentation,
small examples, shared conformance fixtures and rexxpository source packaging.
File opening, stream ownership and encoding conversion belong to the caller;
examples will show those boundaries. Streaming, dialect detection, alternative
delimiters, header mapping, schema/type conversion and spreadsheet-specific
transformations are later possibilities, outside the first version.
No compiler, VM, ISA, native ABI or core-library changes are planned.

The eventual portability target is macOS, Linux and Windows for every runtime
family. This is a proposed qualification scope, not a claim of existing support.
A cREXX-only increment can finish independently, with the other runtime and
platform criteria remaining open. Publication and any narrower supported matrix
require an explicit decision; an unavailable test machine does not count as a
pass or silently reduce the objective.

## Repository home

Adrian approved the lowercase `packages/rexx-csv/` home on 20 October 2026:

```text
packages/rexx-csv/
  PLAN.md                  # this plan moves with the package
  SPEC.md                  # common behaviour and binding decisions
  README.md, LICENSE
  crexx/                   # Level B source and runtime-specific tests
  classic/                 # one Classic source, tested on Regina and ooRexx
  netrexx/                 # NetRexx source and runtime-specific tests
  examples/                # short examples for each binding
  tests/fixtures/           # one common oracle
  tests/                   # orchestration and fixture adapters
  packaging/               # source artifact assembly and package metadata
```

This is a reusable library distributed as a package. `examples/` is appropriate
for its demonstrations; `lib/plugins/` is appropriate for native providers;
the cREXX core `lib/` has a different ownership and release cycle. Keeping all
three dialect sources together avoids scattering one product across those roots.
The plan, specification and fixtures have moved here from `docs/planning/rexx-csv/`.
There is one authoritative copy.

## Decisions and remaining gates

| ID | Proposed decision | Gate |
| --- | --- | --- |
| D-01 | Use `packages/rexx-csv/`, with examples and fixtures inside it. | Approved 20 October 2026. |
| D-02 | Adopt the comma/double-quote profile in `SPEC.md`, including empty-field, Unicode, newline and resource-limit rules. | Approved 20 October 2026. |
| D-03 | Start with a pure Level B cREXX binding using result objects and row objects; keep file I/O outside the parser. | Approved 20 October 2026; concrete signatures recorded in SPEC.md before implementation. |
| D-04 | Use one portable Classic implementation for Regina and ooRexx. Evaluate the value-result interface described in `SPEC.md`; do not depend on arbitrary caller-stem mutation. | Boundary proof passed; Adrian approved the concrete SPEC.md API on 20 October 2026. |
| D-05 | Use a separate NetRexx binding with the same semantic operations. Concrete proposal: `rexxcsv.Csv` factories and immutable typed row/document/output objects, as specified in SPEC.md. | Concrete SPEC.md API approved by Adrian on 20 October 2026; implementation authorized. |
| D-06 | Package our own source under MIT, retaining actual authorship; do not copy ooRexx `csvStream.cls` into this package. | Original cREXX/Classic/NetRexx source and MIT notices checked in all three local artifacts; no external CSV source copied. Recheck final distribution. |
| D-07 | Aim for the full platform matrix below; qualify and describe each shipped increment explicitly. | Agree the supported matrix and publication content before upload. |
| D-08 | Align the parser input and writer output byte limits so every successful write can be parsed back. Use 16 MiB for both, correcting the original 8/16 MiB mismatch. | Approved 20 October 2026: 16 MiB for both. Qualified in all eight cREXX configurations and on Regina, ooRexx and NetRexx below. |

The installed ooRexx distribution already includes `csvStream.cls`, a
stream-oriented class with headers, delimiter and quoting settings. That is a
useful existing option to document, and a possible independent comparison where
contracts overlap. Its presence does not prove matching behaviour. The proposed
package's purpose is a small common contract across runtimes, not a claim that
ooRexx lacks CSV support. Its source and license are separate from this proposal.

## Numbered acceptance criteria

All four runtime bindings are verified on their recorded macOS versions.
The focused hosted gate below owns Linux/Windows results. Untested JVMs and
live integration remain open; local
success is not a portability or rexxpository publication claim. Evidence is summarized below.

| ID | Observable pass condition | Verification | Status |
| --- | --- | --- | --- |
| AC-01 | Placement, first-version behaviour and cREXX public signatures are approved and recorded. Later binding decisions retain their gates. | Adrian approved the plan and cREXX start on 20 October 2026; concrete binding recorded before implementation. | Complete for cREXX, Classic and NetRexx API approval |
| AC-02 | Every parser fixture produces exactly the stated rows or diagnostic on every delivered binding. Strings retain empty values, whitespace, leading zeros and Unicode. | Common fixtures through independent runtime adapters; byte-preserving fixture transport. | cREXX, Regina, ooRexx and NetRexx passed on macOS |
| AC-03 | Every writer fixture produces the stated canonical text. For valid tables, parsing written text returns the same cells, including an initial BOM as data. | Writer fixtures and round trips; compare UTF-8 bytes with explicit CRLF. | All four runtimes passed on macOS, including 16 MiB |
| AC-04 | Errors are atomic and recoverable; limits, coordinates and result lifetimes match the contract. Independent results survive later calls and caller mutation. | Negative/boundary fixtures; success–failure–success, independent-result and input-mutation tests in each binding. | cREXX, Regina, ooRexx and NetRexx passed on macOS |
| AC-05 | The source-only cREXX package works with its declared minimum released toolchain and the selected current product, without unreleased features or extra plugins. | Build a real consumer through `rxc`, `rxas`, `rxlink`, `rxvm`; focused optimized/unoptimized tests, both concrete VMs where supported. Inspect representative RXAS and helper copying. Record exact tool/library versions. | Passed on macOS |
| AC-06 | One Classic source passes on both actual Regina and actual ooRexx with its approved interface. | Separate executable paths, version checks, complete common fixtures and caller integration tests; neither runtime stands in for the other. | Passed on macOS |
| AC-07 | The NetRexx source compiles and passes the same fixtures with documented JVM/compiler requirements. | Fresh compilation and execution using the declared NetRexx runtime JARs; Unicode and newline transport checks. | Passed on macOS: NetRexx 5.10-GA / Temurin JDK 26.0.1+8 |
| AC-08 | Each supported runtime/OS cell below has a conformance and documented example run, including paths containing spaces. Unsupported or unavailable cells remain visible. | Per-cell commands, versions, source identity and concise outcomes in this plan; no inferred cross-platform passes. | macOS passed; Linux/Windows require the hosted gate below; other JVMs open |
| AC-09 | Each source artifact installs into a fresh target, includes its docs/license/examples and runs without the checkout or test tooling. Package selection chooses the correct binding. | Clean local rexxpository artifact/install tests, including the `SOURCE/any` Classic selection on Regina and ooRexx. Verify actual current client/server selection rules before final metadata. | All three source archive/install checks passed locally; client/server selection open |
| AC-10 | The package has concise usage, encoding, error, limit, compatibility and license documentation, with an exact validation summary. | Review examples against installed artifacts; no stale commands or unsupported portability claims. | All delivered bindings documented with exact macOS qualification |
| AC-11 | Any public upload or announcement has separate explicit authorization and describes only the qualified increment. | Review the artifact/metadata and proposed announcement before that action. | Open |

## Numbered implementation steps

| ID | Work and dependencies | Serves | Status |
| --- | --- | --- | --- |
| STEP-01 | Prepare the shared proposal and hand-authored fixture data; inspect existing runtime installations. No product implementation. | AC-01–AC-04, AC-06–AC-08 | Complete (planning); data checks recorded below. |
| STEP-02 | Obtain review of D-01–D-03, record the exact cREXX signatures, then move the draft into the approved package home. Depends on STEP-01. | AC-01 | Complete; D-04 subsequently approved; D-05 subsequently approved. |
| STEP-03 | Implement cREXX parser, writer, fixture adapter and small consumer examples using documented Level B patterns. Maintain package-local source ownership. Depends on STEP-02. | AC-02–AC-05, AC-10 | Complete for cREXX |
| STEP-04 | Qualify cREXX against the declared released baseline and selected current tools. Exercise the full toolchain; inspect simple, quoted and Unicode RXAS plus result/storage ownership. Record defects with focused regressions before repair. Depends on STEP-03. | AC-02–AC-05 | Complete on macOS |
| STEP-05 | Specify and prove the Classic value-result call boundary on Regina and ooRexx, including external procedure scope and large-result copying. Present the concrete interface for D-04 review. Depends on the approved shared contract; can follow the first cREXX increment. | AC-01, AC-04, AC-06 | Complete; boundary proof and API approved. |
| STEP-06 | Implement the single Classic binding and run all common fixtures independently under both runtimes. Depends on STEP-05 and its interface approval. | AC-02–AC-04, AC-06, AC-10 | Complete on macOS; full fixtures, contracts and local installation passed |
| STEP-07 | Document and review the NetRexx interface, implement it and qualify compiler/JVM integration. Depends on the shared contract and D-05 review. | AC-01–AC-04, AC-07, AC-10 | Complete on macOS; approved API, fixtures, contracts and fresh installation passed |
| STEP-08 | Complete or explicitly revise the proposed OS matrix; keep every unverified cell open until then. Depends on the relevant binding steps. | AC-08 | macOS passed; Linux/Windows use the hosted gate below; untested JVMs open |
| STEP-09 | Build binding-specific source artifacts under one logical package/version; verify selection, fresh installation and examples. Final metadata follows observed rexxpository capabilities, without introducing a native package dependency. Depends on the binding being shipped. | AC-09, AC-10 | All three source artifacts/install checks complete locally; live selection/integration open |
| STEP-10 | Present the exact qualified increment, artifacts and publication text; request upload authorization only when concrete and reviewable. Depends on its applicable criteria passing. | AC-11 | Not started |

The cREXX phase ends after its applicable fixture, toolchain, lifecycle and
installation checks. Overall delivery also requires Classic and NetRexx
qualification and the agreed platform matrix. Completing a phase never closes
the outstanding criteria automatically. Ordinary source-library checks are
sufficient for this draft/implementation scope; native repairs or actual
sanitizer findings follow repository policy if encountered. Do not dispatch
deep assurance or rerun broad unchanged checks simply to review these documents.

## Proposed platform qualification matrix

All cells describe rexx-csv, not general runtime availability. A **Hosted gate**
cell is qualified only by a successful matching job in
[CSV qualification](https://github.com/adesutherland/CREXX/actions/workflows/csv-qualification.yml)
for the revision being assessed; the job summary records its exact identity.

| Binding/runtime | macOS | Linux | Windows |
| --- | --- | --- | --- |
| cREXX Level B | Passed locally | Hosted gate | Hosted gate |
| Classic / Regina | Passed locally | Hosted gate | Hosted gate |
| Classic / ooRexx | Passed locally | Hosted gate | Hosted gate |
| NetRexx / JVM | Passed locally (5.10-GA / JDK 26.0.1) | Hosted gate | Hosted gate |

The qualified minimum cREXX baseline for the first increment is the published
`v1.0.0-beta.3` toolchain. Its remote tag, release notes and complete isolated
installer were checked during STEP-04. The globally installed
developer tools have a local/dirty version suffix and cannot serve as proof of
an unmodified release. Set Classic and NetRexx minimum versions from actual
source/API requirements and qualification, rather than merely from availability.

## Current preparation and verification

- Work is in the user-requested `hotfix2` checkout, synchronized to
  `875a84b75d9a2b91e4a459d5379e57df4f92b15f` before these drafts. The unrelated
  dirty `develop` work remains separate. Existing Lotto changes here have
  their own plan; they are not CSV implementation or qualification.
- Runtime inventory on 20 October 2026: `/opt/homebrew/bin/regina` and
  `/opt/homebrew/bin/rexx` identify as Regina 3.9.7. The separate executable
  `$HOME/.local/opt/oorexx/5.1.0-12973/bin/rexx -v` identifies as Open Object
  Rexx 5.1.0 r12973. A disposable script using `PARSE VERSION` and `6 * 7`
  executes successfully there (exit 0, result 42).
- NetRexx is installed at `$HOME/.local/opt/netrexx/5.10-GA/`.
  `java -cp <that-directory>/lib/NetRexxF.jar org.netrexx.process.NetRexxC -help`
  succeeds and reports 5.10-GA, build 18-20260320-1410. This checks compiler
  startup only, not a CSV implementation or complete JVM compatibility.
- These paths are inventory for development, not hardcoded package or test
  runner requirements. Runtime adapters must accept explicit executable paths.
- Draft data validation: a `python3 -` manifest audit passed schema shape,
  unique IDs, input/result types, diagnostic codes/coordinates, strict UTF-8,
  final newlines and trailing-whitespace checks. Inputs are these draft files,
  not any compiler or runtime implementation. The manifest contains 71 cases:
  35 successful parses, 15 parse errors, 17 successful writes and four writer
  errors. There are also ten specified boundary recipes and six binding/lifecycle
  requirements in the fixture README; every binding now has an implemented adapter.
- A secondary Python standard-library `csv.reader(..., strict=True)` check
  agreed with 43 overlapping valid fixture cases. Nine successful parse cases
  with our blank-row/BOM policies were deliberately excluded from that comparison.
  Expected results were hand-authored first; Python is not the contract oracle.
  The byte arithmetic for L002, L006 and L008 also passed an independent check.
- Draft preparation preceded implementation. Current functional and packaging
  qualification is recorded below; no public upload or announcement has occurred.

Next phases: complete the remaining OS/JVM matrix and live rexxpository integration. Public upload remains a separate authorized action.

## Contract finding and approved repair

**CSV-01 — input/output limit alignment.** The original draft's universal
`parse(write(table))` guarantee conflicted with an 8 MiB parser input cap and
16 MiB writer output cap. Adrian approved 16 MiB for both under D-08 on
20 October 2026. Implementation, shared boundary recipe L006 and preflight
precedence test L010 now use that value. L008 and L009 additionally parse the
largest accepted 16 MiB canonical output, including quote expansion. The
repair passes all eight configurations below; CSV-01 is closed. No round-trip
criterion was weakened.


## cREXX implementation and validation

Qualified source: `crexx/rexx_csv.crexx`, SHA-256 `020bdc4e4d6a3e12347f7609299731bad61380750e179490102de9f345da4fa4`.
The 71 hand-authored literal case payloads are unchanged from review. The
manifest now has the stable profile name `rexx-csv-0.1`; its schema and unique
IDs are checked by the adapter. D-08 updates the common boundary recipes, not
runtime-specific expected results.

- `csvparse` / `.csvdocument`, `.csvrow`, `csvwrite` / `.csvoutput` implement
  the documented API. Results own their data; parsing is a byte state machine
  over valid UTF-8 with span materialization, and writing uses bounded binary
  accumulation. All public factories/methods have source documentation tags.
- The released baseline is the complete `CREXX-v1.0.0-beta.3-macos-arm64.pkg`,
  SHA-256 `b3f35831337212757644057d210a00d18551ff60e458842bbaab6d41baaa684a`,
  extracted into a disposable location. Its compiler reports exactly
  `crexx-1.0.0-beta.3`. The checked remote tag is
  `2bcf693419f2ecdb94321a3cc769f5cd880e1d98`.
- The current comparison is a fresh normal Debug core from
  `875a84b75d9a2b91e4a459d5379e57df4f92b15f`, configured with
  `BUILD_TESTING=OFF` and `ENABLE_PARSER_MODE=OFF`, and built with
  `cmake --build <build> --target stage-product rxvm --parallel 6`.
  The compiler reports `crexx-1.0.0-beta.3+local.g875a84b75d9a.dirty` because
  this checkout contains documentation/package work; native inputs are unchanged.
  The build completed without warnings. No unqualified global installation was used.
- `python3 packages/rexx-csv/tests/check_crexx.py --bin <complete-sdk-bin>`
  passed for both cores. Each core passed optimized and unoptimized compiler
  **and assembler** modes on `rxtvm` and `rxbvm`: eight configurations total.
  Each ran all 71 fixtures and valid round trips, B001–B003 and applicable B005
  ownership/recovery/index/UTF-8 checks, and L001–L010 parser/writer limits.
  L008/L009 parse the maximum 16 MiB canonical outputs back and compare every
  field. Every consumer passed through `rxc`, `rxas`, `rxlink` and the VM;
  linked tests disable autoload to prove the explicit dependency closure.
- `python3 packages/rexx-csv/tests/check_package.py --bin <complete-sdk-bin>`
  passed with both cores. It builds reproducible five-file source archives,
  extracts into fresh targets with spaces, compiles with the documented
  `crexx -program` command, runs through `rxvm`, and checks exact UTF-8/CRLF
  file bytes using paths containing spaces and Ω. Invalid UTF-8 and invalid
  CSV leave an existing output file unchanged. This proves local archive
  installation, not live rexxpository metadata selection or server upload.
- Both README build paths and the literal usage snippet were executed against
  Beta 3 in a disposable consumer project. A diagnostic log initially written
  inside the hashed source root correctly tripped the driver's moving-input
  guard; moving that QA log outside the input root resolved it. No product
  repair or retry workaround was required.
- Readable RXAS inspection and retained structural assertions cover byte reads,
  span copying/appending, indexed field access without copying the whole table,
  and logical source identities. The initial `binlength` value-call snapshot
  and span temporaries remain ordinary core API/ownership costs; there is no
  whole-document copy in the byte loop or string concatenation per data byte.
  Source/TRACE records are retained. These are code-shape checks, not measured
  performance claims.
- No native source, CMake test registration or runtime installation changed.
  No new sanitizer campaign was warranted for this source-only increment.
  cREXX on Linux/Windows remains unverified. Classic and NetRexx qualification
  follow below.
  Temporary archives, expanded SDKs, RXAS, builds and diagnostic logs are removed
  after extracting this summary; maintained source, fixtures and regression
  programs remain in the package.


## Classic implementation and validation

D-04: Adrian approved the concrete SPEC.md API and continuation on
20 October 2026, after a positive external-call proof on each actual runtime.
The approved interface is implemented in `classic/rexx_csv.rexx`: one dispatcher,
ROW/TABLE builders, checked FIELD/TEXT access, immutable WITH_FIELD replacement,
shared CSV diagnostics and binding-only E_ARGUMENT errors. No caller-stem
mutation, global queues, files, JSON or native library is involved.

Qualified source SHA-256:
`af292f1923edb362e6edee86ac03ce6c95cd536119fad4de5417861d630d1a20`.
The unchanged common manifest SHA-256 is
`e971abdcda65f6b05099d0ecb989f92c545abc2efdd6f41517617bdc4a1b7f21`.
The hotfix2 base remains `875a84b75d9a2b91e4a459d5379e57df4f92b15f`;
remote develop was checked at that same commit before this phase. The cREXX
source still matches its qualified SHA above, so no native rebuild or repeat
cREXX campaign was required. Unrelated develop/Lotto work remains separate.

Use the two actual executables, not two names resolving to Regina:

```sh
python3 packages/rexx-csv/tests/check_classic.py --runtime /opt/homebrew/bin/regina
python3 packages/rexx-csv/tests/check_classic.py --runtime "$HOME/.local/opt/oorexx/5.1.0-12973/bin/rexx"
python3 packages/rexx-csv/tests/check_classic_package.py --runtime /opt/homebrew/bin/regina
python3 packages/rexx-csv/tests/check_classic_package.py --runtime "$HOME/.local/opt/oorexx/5.1.0-12973/bin/rexx"
```

Actual identities on macOS ARM64:

- Regina: `/opt/homebrew/Cellar/regina-rexx/3.9.7/bin/regina`,
  `REXX-Regina_3.9.7(MT) 5.00 18 Mar 2025` (64 bit).
- ooRexx: `$HOME/.local/opt/oorexx/5.1.0-12973/bin/rexx`,
  `REXX-ooRexx_5.1.0(MT)_64-bit 6.05 2 May 2025`, Open Object Rexx
  5.1.0 r12973. `/opt/homebrew/bin/rexx` is Regina and was not counted as ooRexx.

Verification scope and current results:

- Both actual runtimes pass all 71 unchanged hand-authored fixtures, canonical
  byte checks and every valid round trip. The adapter checks schema/profile,
  unique IDs and runtime identity; exact UTF-8 hex literals preserve fixture
  Unicode, CR/LF, NUL and leading zeros. No runtime generates its own oracle.
- Both pass variadic ROW construction with 1,024 fields, then 1,025 fields
  receiving the writer's `fields_per_record` diagnostic at (1,1025).
- Both Regina and ooRexx pass the complete final contracts.
  These cover B001–B003/B005: success–failure–success, retained independent
  results, original text/writer-input mutation, pure edits, nested calls,
  same-named caller stem isolation, numeric-context preservation, invalid
  accessors/omitted arguments and strict UTF-8 scalar boundary validation.
- Full contracts cover L001–L010: accepted and first-excess ASCII/Unicode
  field sizes; row/column/total counts for parser and independent writer
  construction; every row's first/last indexed field and all 100,000 cells
  compared through exact canonical writer output; 16 MiB input preflight;
  exact 16 MiB output and quote-expansion round trips comparing every field;
  embedded CRLF; NUL/error precedence; and a 16 MiB owned constructor payload.
  Failures expose no partial text or rows; normal calls continue afterwards.
- Both pass the deterministic five-file source archive and fresh installed
  consumer without a checkout, compiler, JSON or Python runtime dependency.
  Package README usage code and the standalone demo run on both. B004/B006
  include paths with spaces and Ω, literal UTF-8/non-BMP and embedded CRLF,
  empty files, output truncation, same-input/output paths, missing input,
  and invalid UTF-8/CSV/NUL leaving existing output unchanged. This is a
  local archive/install proof, not live rexxpository selection or upload.

The initial large Regina run exposed repeated whole-table argument copies in
private per-cell helpers. It was stopped, and the helpers now expose only the
external library activation's private value. Full qualification was restarted
on the corrected source above. Parsing uses forward framing searches and
materializes completed spans; assembly of indexes/payload/output is balanced
and releases consumed chunks. Indexed lookup reads fixed entries without a
source-level whole-table scan/copy. Host value-call copying can still dominate
large repeated external queries; the count fixture checks every row endpoint
through FIELD and all 100,000 cells through one indexed WRITE/canonical-byte
comparison, avoiding redundant full-value calls. This preserves the large-table
oracle without turning the correctness test into a call-copying stress run.
There is no constant-time runtime or resident-memory claim. No native runtime
change or formal speed comparison was made.

The pre-implementation proof checked shared CHANGESTR/COUNTSTR/VERIFY behaviour,
private external scope and large value transport. An external PROCEDURE EXPOSE
caller probe raises Error 17 on both runtimes, so the API does not depend on it.
A huge 10,001-argument single source clause failed on Regina; batched TABLE
construction avoids that source/parser constraint. Lasting proof coverage is
now in the actual API's contracts; the superseded representation prototype and
its runner have been removed. Task-created temporary sources/logs are discarded
after extracting these conclusions.

STEP-06 and the Classic macOS phase are complete. Linux, Windows and live
client/server package selection remain open; NetRexx qualification follows below. No source archive
or announcement has been uploaded and no commit/push was requested during that earlier phase.

## NetRexx implementation and validation

Adrian requested the NetRexx version next on 20 October 2026, using the same
specification/common fixtures and explicitly reviewing the proposed API before
implementation. This section refines STEP-07; it does not create a separate
scope record; phase completion is distinct from the full OS/JVM and integration scope.

### Intended outcome and API decision

Deliver a pure NetRexx source package whose parsing, canonical writing,
diagnostics, UTF-8 byte limits and round trips match the existing approved
profile. NetRexx users should construct immutable rows, inspect checked results
and edit snapshots using typed JVM objects, with an example that preserves
UTF-8 and embedded CR/LF. The existing cREXX/Classic results are independent
implementations, not the oracle. The full OS matrix and publication gates
remain in force; the initial macOS increment does not establish portability.

The approved D-05 API is in the NetRexx section of SPEC.md. Four public
final types in package `rexxcsv` provide static `Csv.parse(String)`,
`Csv.row(String[])` and `Csv.write(CsvRow[])`, with immutable CsvRow,
CsvDocument and CsvOutput objects. CSV accessors use one-based coordinates;
ordinary JVM arrays use zero-based indices. An immutable row edit plus a
caller-owned `document.rows()` snapshot supplies the editing workflow.

Review includes the full failure policy: shared checked CSV diagnostics;
IllegalArgumentException for null/coordinate API misuse; binding-only checked
E_ARGUMENT at (0,0) for unpaired UTF-16 in parser/writer text. Malformed UTF-8
file bytes are rejected by the caller's strict decoder before CSV processing.
Invalid-UTF-16 precondition failure precedes parser byte budgeting; all valid
scalar input retains the shared byte-preflight/syntax precedence. These binding
decisions were approved under D-05; the shared CSV rules remain unchanged.

### Checkable closure and implementation steps

All existing acceptance IDs are retained. NetRexx macOS closure is evidenced
below; remaining platform/JVM and live integration portions stay open:

| Acceptance | Observable NetRexx closure |
| --- | --- |
| AC-01 | D-05 approval records the exact signatures, indexing, ownership and failure policies before product source is written. |
| AC-02 / AC-03 | Fresh compiled NetRexx consumers pass all 71 unchanged hand-authored fixtures, canonical bytes and every valid round trip; no JVM-specific expected CSV data. |
| AC-04 | L001–L010 and B001–B006 pass, including UTF-8 byte counting of supplementary/combining text, unpaired high/low surrogates, null/invalid indices, defensive input/output array copies and success–failure–success. Maximum 16 MiB outputs parse back and every original field matches. |
| AC-07 | The source and examples compile from a fresh directory with explicit compiler/JDK paths. Runtime-only execution proves the required JAR closure; versions and class-file target are documented without inferring older-JVM support. |
| AC-08 | Record the actual macOS runtime/JDK cell and keep Linux/Windows and untested JVM versions open. |
| AC-09 / AC-10 | A reproducible source archive includes all owning NetRexx sources, docs, MIT license and runnable example. Fresh installed application, paths with spaces/Unicode, exact file bytes and failed-input output preservation pass. Live package selection remains separately open. |

| Step | Work, dependencies and decision gate | Serves | Status |
| --- | --- | --- | --- |
| STEP-07.1 | Inspect the installed compiler/JVM and typed-language examples; record and present the complete API. D-05 approval is required before STEP-07.2. | AC-01, AC-07 | Complete; Adrian approved D-05 on 20 October 2026 |
| STEP-07.2 | After D-05 approval, implement the four public types and private shared helpers in NetRexx. Use indexed immutable row data, defensive array boundaries and bounded builders; preserve the shared CSV scan/error/byte-limit rules. | AC-02–AC-04, AC-07 | Complete on macOS |
| STEP-07.3 | Implement an adapter over cases.json and maintained NetRexx boundary/lifecycle contracts. Compile/run independently, including malformed UTF-8 caller decoding and JVM surrogate negatives. Inspect allocation/copy shapes; correct any discrepancy against the approved contract. Depends on STEP-07.2. | AC-02–AC-04, AC-07 | Complete on macOS |
| STEP-07.4 | Add the binary-file example, source archive builder and fresh installation check. Run with only declared runtime JARs after compilation; update exact qualification/docs. Depends on STEP-07.3; supports overall STEP-09 without closing live selection/publication. | AC-07–AC-10 | Local source install/consumer complete on macOS; live integration open |

### Implementation and qualification

Qualified library input SHA-256:
`871f423c77fb53e84b17cc50caeb2d065262ec373af18872767f770bfbc9e657`.
It hashes the four sorted `netrexx/rexxcsv/*.nrx` inputs, each as
`relative path from netrexx + NUL + file bytes + NUL`.
The common case payload remains SHA-256
`e971abdcda65f6b05099d0ecb989f92c545abc2efdd6f41517617bdc4a1b7f21`.
The cREXX and Classic source identities still match their qualification above;
no unchanged core/native or earlier-binding campaign was repeated.

- Work is in Adrian's selected hotfix2 checkout, HEAD
  `875a84b75d9a2b91e4a459d5379e57df4f92b15f`, which still matched remote
  develop before implementation. Existing uncommitted CSV/Lotto work is
  preserved. No branch/worktree or native source/build change was introduced.
- Adrian approved the exact D-05 API and implementation on 20 October 2026.
  `netrexx/rexxcsv/` now owns the four public final classes. Construction is
  package-private and `Csv` exposes only the three approved static factories.
  NetRexx owns the CSV algorithms; no third-party parser/source was copied.
- Actual baseline: NetRexx `5.10-GA`, build `18-20260320-1410`, installed at
  `$HOME/.local/opt/netrexx/5.10-GA/`; Temurin OpenJDK `26.0.1+8` on macOS
  ARM64, JDK home `/Library/Java/JavaVirtualMachines/temurin-26.jdk/Contents/Home`.
  Fresh compilation uses the SDK's NetRexxF.jar and bundled compiler, with
  `-ecj -utf8 -sourcedir -nologo -verbose0`. All sources compile without
  warnings. Exact CharSequence/int builder calls avoid inherited covariant
  overload ambiguity on this SDK/JDK. `-sourcedir` retains the required JVM
  package layout beside the source files.

Commands, run with fresh temporary consumers:

```sh
python3 packages/rexx-csv/tests/check_netrexx.py --sdk "$HOME/.local/opt/netrexx/5.10-GA" --java-home /Library/Java/JavaVirtualMachines/temurin-26.jdk/Contents/Home
python3 packages/rexx-csv/tests/check_netrexx_package.py --sdk "$HOME/.local/opt/netrexx/5.10-GA"
```

- Both commands pass. Every generated class is class-file major version 52.
  Consumers run with only their compiled classes and NetRexxR.jar on the
  dependency classpath; NetRexxF.jar, checkout and test classes are absent from
  the installed consumer. This proves this JDK/runtime closure, not Java 8 or
  another JVM version. The source/API uses Java 8-era library services; older
  JVM qualification remains open.
- `tests/check_netrexx.py` compiles independent NetRexx consumers from all 71
  unchanged hand-authored common fixtures. Exact UTF-8 hex transport preserves
  NUL, Unicode and CR/LF; status, diagnostics, canonical text and every valid
  round trip match. Expected data comes only from cases.json.
- `netrexx/tests/CsvContracts.nrx` passes L001–L010 and applicable
  B001–B003/B005. Checks include success–failure–success for parse and write,
  retained results after later calls, caller input-array changes, independent
  document row arrays, pure row edits, nested calls, empty rows/tables,
  invalid coordinates and every null boundary. It independently visits all
  100,000 indexed fields and checks canonical output from separately
  constructed writer rows at the count bounds.
- ASCII, Latin, supplementary and decomposed fields pass exact/first-excess
  UTF-8 byte limits. Unpaired high/low surrogates and malformed pairs return
  binding E_ARGUMENT; invalid UTF-16 precedes parser size checks as approved.
  Strict caller decoding rejects the maintained malformed UTF-8 corpus.
  NUL/limit order, CRLF and quote expansion are checked. The accepted 16 MiB
  input and both maximum canonical outputs parse back with every field compared;
  first excess values have the stated codes/limits/coordinates. A 16 MiB row
  payload can be constructed and correctly fails the writer's framing budget.
- `packaging/build_netrexx.py` builds a deterministic eight-file source archive:
  four library sources, standalone NetRexxDemo.nrx, README, specification and
  MIT license. The install check compares repeated archive bytes, extracts
  into a new spaced/Unicode directory, compiles, executes the literal usage
  snippet (also shared with SPEC) and the file example, and verifies exact
  UTF-8/CRLF bytes, embedded CRLF/non-BMP text, empty-file truncation,
  same-input/output paths, missing/oversized inputs and encoding/CSV/NUL
  failures preserving existing output. It removes source before a final precompiled
  consumer run. This covers B004/B006 locally, not live package selection.
- `javap -public` inspection confirms the four approved final types, all
  expected signatures and no public constructors. `javap -c` shows document
  field access uses bounds checks, array indexing and CsvRow.field without an
  array copy. Parser arrays grow geometrically; completed strings/rows are
  owned snapshots. Quoted spans and canonical output use bounded builders;
  there is no repeated whole-document copy per character or accessor. Defensive
  array copies are confined to construction/edit/snapshot boundaries. This is
  structural inspection, not a formal performance or resident-memory claim.
- All API/source documentation tags, examples, encoding/error/limit guidance
  and actual qualification status are maintained in the package. Local archives
  include the original MIT notice and bundle no JVM/SDK/runtime JARs. Temporary
  sources, classes, archives and logs are removed after extracting these results;
  only maintained source, tests/oracles and current documentation remain.

STEP-07 and the NetRexx macOS phase are complete. Overall delivery still has
open Linux/Windows cells, untested JVM versions and live rexxpository selection/
publication. No upload, announcement, commit or push was performed in this phase.


## Linux/Windows hosted qualification (STEP-08)

Adrian authorized this phase on 20 October 2026: add the focused workflow in
hotfix2, repair Windows harness issues, commit/push the CSV work and run Linux
and Windows for all four actual runtimes. This authorizes repository publication
and these eight jobs; live rexxpository upload and announcements remain separate.

The outcome is reproducible qualification of the same library contract and
fresh source-package consumer on Linux x64 and Windows x64. Each job must use
an explicitly identified runtime and the shared oracle, including large limits,
Unicode and exact file bytes. Existing valid macOS qualification remains valid
for unchanged library/test inputs; hosted success is established only by the
actual run, not by workflow preparation. Extra JVM versions remain open.

| Step | Work and dependencies | Acceptance | Status |
| --- | --- | --- | --- |
| STEP-08.1 | Pin official runtime assets and versions; inspect platform launchers and make harness execution portable without changing the CSV oracle. | AC-02–AC-08 | Complete: assets downloaded/checksummed; executable suffix/VM selection repaired |
| STEP-08.2 | Add a focused eight-cell GitHub Actions matrix with full fixture/contracts and fresh source-install checks. Use generous hang-protection limits and independent runtime jobs. Depends on STEP-08.1. | AC-02–AC-10 | Workflow implemented; actual success requires hosted runs |
| STEP-08.3 | Validate changed harness paths locally, review/stage CSV-only work, commit and push hotfix2. Preserve unrelated Lotto work. Depends on STEP-08.2. | AC-08, AC-10 | Complete: initial publication 1c41e219e; substantive hosted repairs follow |
| STEP-08.4 | Run the matrix on the pushed revision, inspect failures and repair their actual causes. Claim each cell only after observed success, with the hosted run providing the result and logs. | AC-02–AC-10 | Hosted gate in progress; findings/repairs below |

Publication preparation: this is a source-library increment with unchanged
native inputs. Prior product build and macOS proofs above are reused. Only the
focused CSV matrix is requested; no broad/deep/sanitizer dispatch is needed.
The workflow's run page owns exact hosted revision, runtime versions and pass/
failure outcomes. This plan records the procedure and retained local findings;
report observed hosted results with its link rather than adding a commit solely
to replace a pending CI status. A failed cell remains open until repaired.


STEP-08 preparation validation: `actionlint .github/workflows/csv-qualification.yml`
and Python AST parsing pass. Every pinned ZIP was freshly extracted with the
bootstrap, preserving SDK execute bits/symlinks; the downloaded NetRexx compiler
reports 5.10-GA with Temurin 26.0.1+8. The cREXX release hashes also match GitHub's
published asset digests. Exact `.exe` selection is explicit on Windows; its MSVC
SDK has rxbvm only. No library source or common oracle changed in this phase.
Regina's complete contract subprocess has a 30-minute hang backstop within a
45-minute job: prior large Unicode/value checks take minutes locally, so the
old ten-minute generic subprocess deadline was not a portable performance rule.

Focused local checks pass on macOS: Regina common fixtures; Regina and ooRexx
fresh source-package consumers; NetRexx complete fixtures/contracts and fresh
source-package consumer after the Java path adjustment. The unchanged cREXX
source/Unix commands retain their earlier proof; Windows execution is tested
by the hosted cell. Prior core/native inputs and qualification remain unchanged.
Publication scope is the CSV package, its workflow and the packages ownership
row in DOCS_MAP; existing Lotto edits are excluded. Successful hosted jobs own
observed Linux/Windows outcomes; live package selection/upload is still open.


### First hosted findings and repairs

The first [eight-cell run](https://github.com/adesutherland/CREXX/actions/runs/39435441183)
uses commit `1c41e219e9b6b3148f18f3cc690ad8f2b4c4cd5f`. Linux cREXX,
ooRexx and NetRexx passed. Windows cREXX, ooRexx and NetRexx passed all
shared fixtures and full contracts before failing at the package/host boundary.

- CSV-02: Regina's unquoted external name becomes uppercase and cannot find
  the lowercase file on Linux's case-sensitive filesystem. All Classic callers,
  examples and API documentation now use `'rexx_csv'(...)`. The dispatcher,
  operations, values and common oracle are unchanged. This is a portable Rexx
  call convention correction, not a new interface design.
- CSV-03: NetRexx's archive builder used host path strings as tar member names,
  yielding backslashes on Windows. It now uses POSIX archive member names.
  The exact eight-member assertion remains, rather than accepting malformed names.
- CSV-04: Native Windows cREXX Beta 3/ooRexx 5.1.0 command-line paths containing
  Ω are converted to O. This is the runtime filename boundary outside the
  in-memory CSV contract. B004 requires spaced paths and exact bytes, not an
  arbitrary-Unicode native filename contract. Keep every Unicode CSV/limit
  expectation unchanged and run the complete consumer on spaced ASCII Windows
  paths; retain a separate Ω probe checking output preservation and explicitly
  reporting the capability limit. macOS/Linux and NetRexx retain positive Ω
  filename tests. Correct the examples' overbroad Windows filename claim.
  No native SDK is patched or its binary identity disguised. General Windows
  Unicode filename support is not established by this source-package qualification.
- CSV-05: ooRexx's Windows API server outlives the interpreter and locks rxapi.exe.
  Cleanup now stops only the server whose executable path matches this job's
  disposable SDK, then removes its files. It does not terminate servers by name.

The archive/caller fixes have focused macOS replays; unchanged parser algorithms
and common expected data retain prior proof. The next pushed substantive repair
runs the same complete eight-cell gate. Successful previous jobs are evidence
for their inputs, not substitutes for changed package-consumer checks.


The second hosted run at `b99d30bc5` confirms Linux cREXX/ooRexx/NetRexx
and Windows cREXX, including its explicit filename-limit probe and installed
consumer. Further boundary repairs from that run:

- CSV-06: NetRexx's consumer launch used an absolute Unicode classpath and Java
  failed to find Usage. Use `.` for the explicit subprocess working directory,
  matching the documented installed command. Runtime-only JAR closure remains
  unchanged; the positive Unicode file/path tests remain in place.
- CSV-07: Windows ooRexx passes a raw command tail to Rexx, so Python's argv
  escaping changes the example's literal double quotes into backslash quotes.
  The harness now supplies the documented raw quoted tail for actual Windows
  ooRexx. Regina keeps its ordinary argv convention, with runtime identity
  determining the choice. The package's argument convention is unchanged.
- CSV-05 refinement: Windows presents the same server path with both RUNNER~1
  and runneradmin. Canonicalize both the CIM process path and disposable SDK
  path before comparing; terminate only the matching PID. This repairs the
  cleanup failure without broad process-name termination.


CSV-06 follow-up: the relative classpath fixes JVM class loading from the
Unicode installed directory. The next Windows run then exposed the Java
launcher's Ω-to-? conversion in application argv (`InvalidPathException`).
Keep the Unicode working-directory/compilation proof, use relative ASCII
file arguments for the complete Windows consumer, and retain an explicit
Ω filename probe with output preservation. The example now reports invalid
paths as a caller file error. All Unicode CSV bytes and parser/writer oracles
remain unchanged. Arbitrary Unicode Windows CLI filenames remain a documented
host boundary for all four tested launchers, not a qualified CSV capability.
The fresh NetRexx archive/consumer replay passes locally after this example fix.


Installed documentation now gives the distinct native Windows cmd.exe quoting
commands for Regina and ooRexx and points to the hosted gate for Linux/Windows
results, rather than freezing an obsolete pending-platform claim in each
archive. The executable README snippets and all source/test/build inputs are
unchanged by this final documentation clarification. Reuse the current hosted
qualification for those inputs; a documentation-only push uses `[skip ci]` to
avoid starting another identical long-running matrix. This is not a CI-status
commit and records no unobserved pass.


### CSV-08 — suspected Windows Regina stall; progress/isolation repair

At Adrian's request, the final Windows Regina job was stopped after more than
23 minutes in the combined contracts. Its common 71 fixtures and 1,024-field
constructor had passed; Linux Regina completed the full job in 7m13s. The
cancelled [run](https://github.com/adesutherland/CREXX/actions/runs/39437398301)
retains seven successful cells. Elapsed time does not prove a loop: the harness
buffered all contract output until completion, so forward progress was unknown.
Windows Regina full qualification remains open.

Next actions under STEP-08.4: preserve every assertion and common fixture,
split Classic contracts into independently selectable ownership/fields/counts/
large/quotes groups, flush named progress markers, stream them from a temporary
log during execution, and retain the final diagnostic tail on timeout. Put the
quick installed consumer before large contracts. Add manual runtime/platform/
group selection so a diagnostic rerun need not repeat the seven successful
cells. Subset runs must be labelled diagnostic, never full qualification.
First run Windows Regina ownership plus installed consumer, then isolate the
stalled group and operation; repair its actual cause before repeating the
complete Windows Regina gate. Do not increase the deadline merely to hide it.
Library sources and literal expected results are unchanged. Assertion-line
multisets before/after instrumentation are equal; local grouped checks validate
the revised harness before publishing. Full qualification still requires every
group, with the existing lifecycle assertions kept together in one process.


The revised harness streams named operations and flushes Rexx stdout. All
original assertion lines are retained unchanged. Local ooRexx passes every
group: ownership 0.2s, fields 1.2s, counts 10.6s, large 2.7s, quotes 1.0s
(diagnostic scheduling observations, not a formal performance claim). Local
Regina's ownership group passes, but its field group advances slowly through
large Latin UTF-8 parse/write operations. This gives a concrete candidate for
pathological cost rather than proof of an infinite loop. Publish instrumentation
without an automatic duplicate matrix, then dispatch the targeted Windows
Regina diagnostic and preserve the seven successful unchanged cells.


CSV-08 minimal reproduction: the existing UTF-8 helper on valid repeated Latin
text takes 0.109/0.364/1.441s at 32,768/65,536/131,072 scalars on local Regina;
a scratch version applying the same scalar checks to bounded, scalar-aligned
4 KiB chunks takes 0.036/0.072/0.145s. These diagnostic samples identify scaling,
not formal speed claims. Windows diagnostics pass ownership/installed consumer,
large 16 MiB and quote-expansion groups; field/count groups are investigated
separately. No infinite-loop conclusion is established.

STEP-08.4 repair: bound UTF-8 BIF operands to at most 4,099 bytes, extending a
4,096-byte chunk by at most three continuation bytes. Keep the existing exact
scalar validator and failure ordering. Add valid/invalid 2/3/4-byte and truncated/
surrogate boundary regressions at every nearby chunk alignment; retain all old
assertions and add supplementary/decomposed maximum-field cases. Acceptance is
the same checked results/limits on both Classic runtimes and both hosted OSes,
plus visibly advancing isolated groups and full normal qualification. There is
no public API, opaque representation, native or shared CSV profile change.


Windows diagnostics at 8d3e343e8 pass ownership plus installed consumer
(run 39441875640), counts (39441997768), large (39442002835) and quotes
(39442007269). Only fields (39441993861) remains slow and is stopped for its
checkpoint log. The bounded UTF-8 repair passes the expanded local Regina
field group in 6.3s versus 86.4s before (now also supplementary/decomposed
maximum fields), and full ooRexx grouped checks/installed consumer pass locally.
The wrapper retains the all-ASCII fast path; private scalar checks are unchanged.
These observations guide a focused Windows repeat, not a formal benchmark.


Current repaired Classic source SHA-256: `7d28d3ddf2394ccfe1a48e8eeeb6aa4be9b3182e5eb10e6f1c6efdc76aad8421`.
It supersedes the original `af292f...` source identity for further Classic
qualification; earlier platform results above remain historical baseline proof.
The final local Regina ownership/expanded-fields/large/quotes and fresh consumer
pass after the ASCII fast path is restored (fields 6.2s); the final counts
replay runs separately. Full ooRexx and consumer checks also passed the chunk
repair, and the final ASCII-only path retains the previously qualified scan.
Next hosted check is Windows Regina fields, followed by full Classic platform
gates. The cREXX/NetRexx library, fixtures, harnesses and runtime pins are unchanged,
so their successful hosted cells remain valid.

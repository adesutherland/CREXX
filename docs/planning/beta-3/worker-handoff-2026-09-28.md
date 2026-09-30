# Implementation worker checkpoint — 28 September 2026

**READY FOR HANDOFF.** Adrian requested transition to GPT-6 Sol Extra High at
this checkpoint. Read AGENTS.md and the complete authoritative
[core-baseline plan](core-baseline-2026-09-28.md), including its later scope
clarifications, before resuming. No new agent or publication is authorized by
this handoff itself; the coordinator owns the replacement and review.

## Workspace and authority

- Only implementation checkout: `/Users/adrian/.codex/worktrees/beta3-core-baseline/CREXX`,
  branch `temp/beta3-core-baseline`, base `143921e11e4d573909fcc4def28da5dceadba9d9`.
- HEAD `4ce9de623` (full hash in the checkpoint binary manifest). Earlier local
  commit `27614024d` records approved plan/report; `4ce9de623` implements
  compression and complete RXBIN sentinel repair. **Nothing pushed.**
- Primary `/Users/adrian/CLionProjects/CREXX` dirty/older: preserve. Lab shared
  `/Users/adrian/CLionProjects/mainframe-lab` remains read-only inspected input
  `1742740df60c0858421f196993e55da3edcb681b`; reread current lab plans before newer
  input. No guest operations. Exported patches are candidates, not exact approved diffs.
- Parent owns independent review, normal PR/check/merge publication and one
  final combined Linux ASan/LSan + macOS ASan matrix. Worker must not push or
  dispatch. No beta tag or release. Ordinary full local normal suite is still due
  on the frozen combined inputs; do not accumulate repeated broad runs.

## Scope and text boundary

SOURCELINE #602 is **not a beta 3 requirement** following Adrian's explicit
clarification. Parent updated and verified the issue OPEN. Compact real compile
receipt proves distinct imported/inlined providers collapse to incompatible
`unit.crexx:6` anchors in one caller module. Future Level C may use an explicit
source-file-available design; there is no general Level B/G promise. Do not
continue source QA, add storage/format syntax, or reopen runtime source now.

The user explicitly requires coherent external text across maintained CMS31,
TSO31 and TSO64 surfaces. Source/file selector set: UTF8, ASCII, Latin1,
Windows-1252, IBM437, IBM850, IBM1047. Preserve exact retained mapping policy,
CP1252 undefined-position C1 mapping, Classic BYTE/UTF8 distinction, explicit
replacement API behavior, .binary, binary bytes and 64-bit Rexx integers. No
UTF16/32 tool-stream extension is implied.

**The latest architecture supersedes passing maps to the runtime.** Shared
cREXX codec code owns conversion; platform applies it exactly once and adapts
logical lines. Native/newlib supplies raw bytes, records and OS services, no
implicit conversion on the cREXX route. See
[text boundary](text-boundary-2026-09-28.md),
[fixed raw-service contract](native-raw-services.md), and
[`platform/native_raw.h`](../../../platform/native_raw.h).
Parent approved a fixed minimal API and inspected this concrete handoff. Adrian
is handing it to the mainframe agent. The raw API implementation is pending.

Parent's `/Users/adrian/CLionProjects/mainframe-lab-beta3-text-runtime` draft
map setters/converted console work is **superseded, unpublished, not a dependency**.
Do not edit or consume it. Existing old converted argv/name interfaces are
transitional open gaps, never decode them twice or mark them migrated.

## Qualified committed changes

`4ce9de623` (`fix(rxbin): bound compression stack and normalize portable import sentinels`):

- Active lab0013: compressor hash table moves from stack to checked per-call
  heap storage. Fault regression covers both workspace allocations and initial/
  later output allocation failure cleanup. Output unchanged: 12,207 bytes,
  FNV1a `3fc9210d4fbd2dba`; Clang stack report 65,728 to 176 bytes. Heap table
  adds 65,536 LP64 / 32,768 ILP32 bytes; total work 98,312 / 49,156 bytes.
- Active0016 subsumed by older0011 complete canonical u64 writer plus legacy
  ILP32 imported-address decoder and diagnostic enrichment. Baseline sentinel
  regression fails; new canonical write/read, legacy read, 32-bit real-address
  overflow control retained. See `docs/qa/beta3-core-baseline/binutils.md`.
- `tools/check-rxbin-width.sh` native LP64 passes. Actual ILP32 remains pending
  normal Linux Build/PR multilib step added in existing `.github/workflows/build.yml`.
  No local Docker/qemu/zig/x86 compiler route; do not invent guest proof.
- Coordinator independently reviewed this commit, no blockers; receipt at
  `docs/qa/beta3-core-baseline/coordinator-review.md` (include eventual docs commit).

## Uncommitted core changes and current QA

All are preserved, not final combined-tree qualified:

1. `compiler/rxcpsymb.c`: older0012 allocation checks; SIZE_MAX guard moved before
   indexing `name[name_length-1]`. `interpreter/rxvmvars.h`: older0013 guards both
   worker-owned and standalone factories. Two C fault harnesses include the real
   implementation; terminal panic replaced by longjmp with explicit test cleanup.
   Focused normal tests passed. Matching sanitizer CTests still need execution.
2. `compiler/rxcpfunc.c`, `rxcp_ctx.h`: first-root namespace selection with a
   distinct `shadowed` flag, rather than incorrectly marking skipped bodies as
   imported. Keeps multiple differently named same-root providers and dependency
   header hashing. Active0017 fixtures retained; baseline output38 vs correct35.
   Normal source-order, project-dependency, import-report and directory snapshot
   tests passed. Relevant private-dependency convergence checks still need selection.
3. Compiler binary-load failures retain RXBIN cause and attach a compilation
   diagnostic. Required exit bridge/library load failure now terminates nonzero
   instead of erasing PARSE/ADDRESS while returning success. Optional
   `RXCP_EXIT_MODULE` warning/fallback preserved. Parent accepted reproduced cause.
   Permanent Python real-tool regression `compiler_load_failures.py` passes in
   normal/ASan; currently custom target only. Register as ordinary CTest next,
   RUN_SERIAL and generous 300-second hang guard; timing is now available below.
   Script removes prior failure artifacts, including `broken.rxbin`, for repeatability.
4. New discovery error propagation: platform iterator EOF clears errno; close
   always releases and preserves prior error or close failure. Compiler distinguishes
   absent optional root (only initial open ENOENT/ENOTDIR) from enumeration/close
   errors, records `IMPORT_DIRECTORY_READ_ERROR` and rejects incomplete lists.
   `Context.import_discovery_error` prevents repeated invalid discovery. New C
   injection test covers open/EIO, next/EIO, close/EBADF, missing root and clean EOF;
   normal test passed 0.32 sec. **Review needed:** context reset/highlighter refresh
   should clear the sticky error when deliberately rebuilding discovery. The new
   test allocates `context->location/file_name` via strdup; `fre_cntx` does not
   obviously own these fields, so fix fixture ownership before LSan qualification.
   No actual sanitizer finding has been observed for this new test yet.
5. `messages/diagnostics.en_GB.msg` contains new RXBIN/directory messages. Core
   build caught missing errno.h after discovery edit; fixed, rebuilt successfully.
   This was ordinary compilation breakage, not a sanitizer finding.

### Exact retained commands and results

Baseline configure/build: `cmake -S . -B cmake-build-debug -G Ninja
-DCMAKE_BUILD_TYPE=Debug -DENABLE_LLAMA=OFF`, then `cmake --build
cmake-build-debug --parallel 10`; passed 2015 actions; `/tmp/beta3-baseline-build.0pyDsF`.

Latest normal rebuild:
`cmake --build cmake-build-debug --target check_compiler_load_failures
test_project_dependencies test_import_resolution_report test_import_discovery_snapshot
--parallel 8`, passed `/tmp/beta3-compiler-rebuild2.log`. Real compiler load test
**1.41 s**. Inputs contain source/root/load/allocation and discovery changes,
but precede the final unbuilt OOM extraction/shared codec draft described below.

Latest focused normal command:
`ctest --test-dir cmake-build-debug -R
'^(symbol_allocation_failure|value_factory_allocation_failure|compression_workspace|compact_format_check|source_root_namespace_order|project_dependencies|import_resolution_report|import_discovery_snapshot)$'
--parallel 1 --output-on-failure` passed 8/8 in 2.47 s. Retained
[log](../../qa/beta3-core-baseline/core-second-focused.log). New
`test_import_directory_errors` built successfully (`/tmp/beta3-directory-build.log`),
then `ctest --test-dir cmake-build-debug -R '^import_directory_errors$'
--parallel 1 --output-on-failure` passed in 0.37 s
([log](../../qa/beta3-core-baseline/directory-focused.log)).

Apple ASan configured `cmake-build-debugasan` Debug/ENABLE_LLAMA=OFF with
`-fsanitize=address -fno-omit-frame-pointer` C/CXX flags and
`-fsanitize=address` executable/shared linker flags. Apple LSan unsupported;
Linux final matrix supplies LSan. No suppression or supported leak-off waiver.

- Runner `cmake-build-debugasan/asan-logs/20260928-112537-build`: PASS. Command
  `tools/asan-run.sh --phase build --build-target rxc --build-target rxas
  --build-target rxbvm --build-target test_compact_format
  --build-target test_compression_workspace --build-target test_symbol_allocation
  --build-target test_value_factory_allocation --build-target check_compiler_load_failures
  --build-jobs 8 --build-leaks off --leaks off --no-live-tail --tail-lines 18`.
  Built focused targets and ran the real Python load regression successfully;
  **this did not execute the named CTests**.
- Latest runner `cmake-build-debugasan/asan-logs/20260928-113702-build`: **PASS,
  TERMINAL, NO RUNNING PROCESS**. `tools/asan-run.sh --phase build --build-target
  check_compiler_load_failures --build-jobs 8 --build-leaks off --leaks off
  --no-live-tail --tail-lines 8`. Real regression measured **3.49 s**. Terminal
  unified session89113 consumed. No need to poll or cancel it. This build includes
  discovery changes but not later unbuilt OOM extraction/codec implementation.
- Earlier runner113445 was the missing errno.h compile failure;112516 was a
  mistyped target. They are not sanitizer diagnostics. Log scan of successful
  runners found no ASan/LSan/runtime-error reports. No new SAN entry has been opened.

Checkpoint [binary SHA256 receipts](../../qa/beta3-core-baseline/checkpoint-binary-hashes.json)
identify retained executables/bytecode. Do not claim these results qualify the
current entire source tree: later source drafts are explicitly unbuilt.

## Uncommitted platform/codec drafts — reconcile before publication

- `platform_config.h`: explicit CREXX_PLATFORM_CMS/TSO, transitional old lab
  aliases, conflict guard and CREXX_MAINFRAME_ELF; platform selection avoids
  inherited Linux/Apple branches. VM platform identity updated accordingly.
- `platform_tso.c`: cREXX logical `root.TYPE(NAME)` mapping, explicit native
  spelling bypass, ordered roots, size checks, member suffix/casefold ownership.
  **Still uses stale converted/generic directory names and raw fopen; must migrate
  to native_raw.h and cREXX-owned name conversion.** This is not native proof.
- `platform/native_services.h` and platform.c's CREXX_NATIVE_TEXT_IO branch
  **still contain superseded map-setter declarations/routing**. Delete/reconcile
  these before any commit. New route should use `CREXX_NATIVE_RAW_IO` or one
  consistent explicit raw capability, with fixed native_raw.h services.
- `platform_fopen` added; VM file helper bypasses desktop `e` mode on native
  profiles; linker control text file uses the platform route. Audit all supported
  callers rather than claiming complete conversion from this partial routing.
- `generate_text_codecs.py` derives `text_codec_tables.h` from retained authoritative
  legacy mappings with SHA provenance, `--check` verifies no drift. Selector
  implementation exists. **Latest `text_codec.c/.h` now also contain unbuilt,
  untested streaming UTF8 feed/finish, scalar emission, strict inverse mapping
  and whole-buffer conversion.** No runtime map ownership is needed. Add meaningful
  all-map, malformed/overlong/surrogate/unrepresentable/chunk tests before acceptance.
- **Latest OOM draft extracted from platform.c into `platform/oom.c`, unbuilt.**
  It uses bounded UTF8 scalar-aware assembly, reserving final newline, no stdio,
  strerror, allocation or memory introspection. Desktop write handles EINTR,
  short writes and no-progress/error. Native raw builds call the **not yet
  implemented** `crexx_native_panic` for fixed-buffer native conversion/output.
  Nonraw transitional modern builds use their old write route; no new guarantee
  is claimed for that runtime. New `tests/test_oom.c` checks short/EINTR output,
  output failure, errno, and all UTF8 truncation alignments; **not compiled/run**.
  Coordinator specifically requested preserving panic prefix even if truncation
  or unrepresentable detail occurs; normal codecs remain strict, panic may use
  explicit safe fallback. Needs native raw mock test too.
- Platform/CMakeLists and compiler/CMakeLists register new oom.c; **pending**
  `ports/single-threaded/CMakeLists.txt` still lacks `platform/oom.c` after extraction.
  Add it before constrained builds. Native output mock still has old symbols;
  update/remove as the raw route replaces them.
- No `platform_native.c` stream adapter, stdio/argument startup, native getenv,
  or raw directory implementation has been written yet. Intended next work:
  fixed raw handles wrapped by funopen/newlib (fopencookie host mock as needed),
  per-stream immutable selected codec, byte vs record framing, strict incremental
  UTF8, full-record capacity check before commit, deferred close errors, binary
  exactness. Capture console/name IBM1047 independently of selected file page.
  This design is within the agreed boundary; review signatures with coordinator.

Concrete audit gaps: CMS old _write accepts printable ASCII only; CMS files are
raw records unless explicit old text adapter. TSO old _read/_write convert1047
and its fopen wrapper accepts only r/w/rb/wb; VM previously inserted e. Old
lab_tso_path/files.c contain cREXX naming policy; new native route must bypass it.
Compiler catalogs/project metadata binary files are deliberately byte paths;
linker control was raw text. Audit AST/profiler/tools/driver/RXPP/crexxsaa supported
surface claims, generated code, environment and stdio, not just compiler input.
Make truthful component/profile/selector capability matrix, with unavailable
services explicit. Native raw implementation and actual packages are not supplied.

## Live acceptance / steps

| ID | Status at handoff |
| --- | --- |
| AC-01 | OPEN: committed binutils dispositions complete; platform/runtime reconciliation and final patch matrix pending. |
| AC-02 | PARTIAL: source/root, mandatory/optional exit and malformed binary normal cases pass; discovery negative normal passes; permanent load CTest, focused ASan and private dependency convergence pending. |
| AC-03 | PARTIAL: committed compressor/sentinel LP64 tests; allocation tests normal pass. Actual ILP32 CI, focused sanitizer CTests and new OOM draft qualification pending. |
| AC-04 | OPEN: explicit selection/mapping partial; raw backend + platform adapter/error tests and constrained build proof pending. |
| AC-05 | SATISFIED: authorized evidence-backed deferral, #602 OPEN with coordinator issue update. |
| AC-06 | NOT APPLICABLE: no SOURCELINE beta3 implementation. |
| AC-07 | PARTIAL: baseline all build and focused results above. Latest drafts unbuilt; combined frozen normal suite pending. |
| AC-08 | PARTIAL: master workflow repaired and automatic publication succeeds; final combined product Linux/macOS sanitizer matrix OPEN. |
| AC-09 | OPEN for product: no implementation push or PR. Master workflow alone delivered. |
| AC-10 | OPEN: retained evidence/plan/handoff updated; final docs, manifest and native B3 handoff pending. |
| AC-11 | OPEN: cREXX-owned codec/raw-runtime contract fixed; full supported-boundary audit, adapters/backend and combined proof pending. |

STEP-01 complete; STEP-02/03 in progress; STEP-04 complete by explicit deferral;
STEP-05 partial (workflow repaired, combined local QA open); STEP-06–08 open.

Coordinator workflow repair: master `2d24ae989fdb530942c73d81301d6affd243a671`,
only sanitizers.yml, matches develop blob f3806cda1, 240 minutes/Python3.12/
ENABLE_LLAMA=OFF. Coordinator reports actionlint and diff check pass; automatic
Build **36407284141 SUCCESS**, CodeQL **36407284095 SUCCESS**. This is workflow
publication proof only, not a product matrix or native qualification.

## Next actions for replacement

1. Reconcile/read current diff and this checkpoint before editing. Preserve all
   uncommitted drafts. No running jobs remain from this worker.
2. Register measured compiler_load_failures as ordinary serialized CTest; fix
   discovery fixture ownership and error reset semantics; finish focused normal
   plus matching ASan CTests via tools/asan-run.sh. No broad ASan dispatch.
3. Complete/review OOM and shared codec units, repair constrained build source
   list; make qualified causal commits for allocation/load/root/discovery work.
4. Replace superseded map setters with agreed raw platform adapters, complete
   every supported text route and capability matrix; coordinate raw backend
   dependency without lab guest edits. Keep AC04/11 open honestly until combined
   implementation/proof exists.
5. Freeze combined source/test/build inputs, do core build + relevant normal
   correctness once, retain exact receipts; coordinator independent review,
   normal publication and final agreed hosted sanitizer matrix follow.

Do not expand deferred SOURCELINE, performance programme, provider architecture
or product archive policy. Ask only for a material architecture/scope conflict.

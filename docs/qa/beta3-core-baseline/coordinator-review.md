# Independent coordinator review

28 September 2026. This records inspection and coordinator checks; worker
execution evidence remains in the linked batch receipts. The overall baseline
is not yet accepted or published.

## RXBIN candidate 4ce9de62368029be5745ff5d343f0c42e0b485e5

- Reviewed the production compressor change and direct-production fault test.
  Both workspace allocations are released on either allocation failure, output
  append failure and success. Empty input avoids allocation. The retained
  deterministic byte-size/digest and decompression check cover unchanged output.
- Reviewed canonical UINT64_MAX serialization and normalization of the older
  UINT32_MAX spelling only for imported procedures. Real overflowing addresses
  retain the ILP32 rejection. Tests mutate both spellings independently of the
  writer and verify canonical reserialization.
- Reviewed the standalone compact-format command and Linux matrix. The current
  Linux core runs on x64 Ubuntu; gcc-multilib plus `-m32` provides actual ILP32
  execution without rebuilding the full product as 32-bit. That hosted result
  is still pending and is not inferred from the passing LP64 test.
- Coordinator ran `actionlint .github/workflows/build.yml`,
  `sh -n tools/check-rxbin-width.sh` and `git diff --check`: all passed.
  The implementation and regression shape are accepted for the combined batch;
  final normal/sanitizer and hosted-width criteria remain open.

## Compiler/allocation review in progress

The local required-exit repair makes VM creation and mandatory library loading
fail terminally, preserving the optional-module fallback. The real copied-RXC
regression checks nonzero failure and actual PARSE execution. The source-root
test distinguishes the reproduced first-root override and preserves multiple
providers in the selected root. Focused worker sanitizer proof is in progress.

Review feedback: register the real compiler-load regression in ordinary CTest
after its required isolated timing checks. When routing bounded exhaustion
diagnostics through UTF8 conversion, ensure truncation cannot suppress the
useful diagnostic because its final scalar was cut in half.

## Architecture clarification

The [text boundary](../../planning/beta-3/text-boundary-2026-09-28.md) supersedes
the tentative runtime-map-setter design. cREXX owns codecs and their application;
the native backend exposes raw bytes and record boundaries. Superseded local
runtime drafts are unpublished and provide no acceptance evidence. The cREXX
adapter can be tested against host raw-service mocks, but those tests cannot
close the combined native runtime/package gates.

## Local correctness acceptance and draft integration (28 September)

- Independently inspected `64ba2ef0d` and `d454750bd`: all three source-map
  compiler invocations receive the build library path, the preparation target
  includes `library`, original source-map/error assertions remain, and German
  and Dutch messages retain the exact required placeholders.
- Compared `ce4a9273f..6a032a0b7`: only those test/catalogue repairs and evidence
  documents changed. Accepted the retained 2,283 combined passes plus three
  focused Debug passes and the focused Apple ASan source-map pass. Recomputed
  the focused log hashes and corrected a transcription error in the ASan hash.
  AC-07 is satisfied for this local candidate; no broad rerun is warranted.
- Reviewed common import/allocation changes: namespace exclusion keeps several
  providers within a selected root; discovery preserves iterator errors and
  resets on a new parse; required compiler-exit loads fail terminally; symbol
  and value allocation checks use the bounded panic path. The distinguishing
  normal/fault tests and passing combined suite support these common repairs.
- The native codec/stream code separates codec, text/binary and physical record
  layout, with explicit ownership/error mocks. Its integration remains partial:
  startup/console callers, optional configuration environment policy and the
  backend details in `native-raw-services.md` are open. Desktop `-E` currently
  accepts UTF8 only; the seven shared mappings are applied by native raw
  profiles. This is a capability boundary, not universal legacy-page support.
- A draft integration PR may expose the reviewed candidate to ordinary hosted
  checks and the lab without claiming the full baseline accepted. AC-03 actual
  ILP32 execution, AC-04/11 native text completion, AC-08 final sanitizer matrix,
  AC-09 develop integration and AC-10 final handoff remain open. No release/tag
  action or protected-branch bypass is part of this step.

## Hosted review and native flush finding (28 September, 14:36 UTC)

Draft [PR #709](https://github.com/adesutherland/CREXX/pull/709) publishes
`30d72305459ca230befffcdc5208cfce83982a86`; develop remains `143921e11`.
The PR Build run `36436367998` and CodeQL `36436367190` started normally.
Both Windows jobs failed during Git checkout because the newly added
`second/aux.crexx` fixture uses the reserved Windows AUX basename. No Windows
compiler or product test ran. The worker is making a narrow filename repair
that preserves its namespace/content and the regression's intended semantics.

The [console probe](console-flush-probe-2026-09-28.md) confirms a separate
native adapter integration gap: `fflush` returns success with an invisible
partial prompt, and deferred raw flush failures are only seen at close. This
is a deterministic host-mock functional result, not a sanitizer finding or
native guest proof. The standard-stream integration contract now explicitly
requires prompt visibility and flush-error propagation while preserving
logical file records. Its native implementation and qualification remain open.

## Hosted fixture follow-up (28 September, after the first hosted Build)

The ordinary Build run on published head `30d723054` ended with only two
fixture causes: Windows checkout rejected the reserved `AUX` filename, and
Linux/macOS smoke each passed 175/176 with parser-only `source_semantics`
unable to find its required library. The [worker receipt](hosted-fixture-repair-2026-09-28.md)
records the exact runner and local logs, the accepted 100% fixture rename
`369cbeb7b`, the fresh parser-mode reproduction and the focused Debug/Apple
ASan repair tests. A bounded adjacent audit found the same setup omission in
the parser cache sandbox, repaired in test-only commit `00d42cb3a` and passing
its original assertions in focused Debug/Apple ASan. The local broad AC-07
proof did not include parser mode and remains valid for its recorded inputs.
Linux ILP32 RXBIN and optimizer
parity passed on the published head. A new hosted Build on the repaired head
is still required; the native/console and final sanitizer gates remain open.

## Fixture repair acceptance (28 September, 15:03 UTC)

Independently reviewed the clean candidate `e98c4542d`: the Windows fixture
is a content-preserving rename; `source_semantics` now declares its required
modules and build-bin working directory; `highlight_cache` stages those
modules in its isolated sandbox and removes them on both cleanup paths.
Original test assertions and production code are unchanged. Inspected the
focused Debug/Apple ASan receipts, including the final cache ASan log and its
recorded SHA-256, and accepted this repair batch for publication to PR #709.
`git diff --check 30d723054..e98c4542d` passes. Retain the previous combined
normal, hosted ILP32 and optimizer-parity evidence; no broad repeat is needed.
The repaired-head automatic Build and CodeQL checks remain publication gates.

## Shared-core acceptance (28 September, 15:29 UTC)

Accepted AC-01's complete patch disposition and AC-02/AC-03 on product/test
inputs `a4a39dc3b`, following the earlier code/regression reviews and the real
hosted ILP32 proof. Inspected `35495c941` and its updated receipts: source
changes after `a4a39dc3b` are documentation only. All five core platform jobs
and Linux optimizer parity passed; plugin jobs and current CodeQL are still
running. The earlier CodeQL on production-identical `30d723054` has succeeded,
but is not relabelled as the current run. Publish the final documents and mark
the reviewed phase ready for integration, retaining automatic checks and every
open native/final-sanitizer criterion. No broad local rerun is warranted.

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

# Parallel Make bootstrap ownership repair

F6-PREP-01, authorised for repair/publication with compiler STEP06 on 15 October
2026. Baseline: `5f9feff27eaf114bd57f17e162d956270ff8fc54`.
The compiler [plan](../../../../lib/rxfnsl/compilers/PLAN.md) owns acceptance.

## Cause and change

Make copied the L2 linked-image recipe into independent `rxfnsl` and
`levell_unicode_lexer` targets. Both could invoke rxc/rxas/rxlink against the
same intermediate files. The original preparation failed at the linker with
`ERROR: no root modules selected`; duplicate L2 generation preceded it.

Both consumers now have an explicit target dependency on `levell_bootstrap`,
which completes the stage chain and verification first. The native CLI already
waits through `rxfnsl`. File dependencies remain for freshness/missing outputs
and Ninja. This adds ordering between the shared producer and consumers;
it does not force unrelated build work to run serially or change compiler code.

## Permanent reproducer and qualification

`lib/rxfnsl/tools/check-bootstrap-build.py` configures a fresh Unix Makefiles
fixture from the **actual tools CMake declarations**. Instrumented stand-ins
fail on concurrent writes and count every invocation; they do not implement or
test compiler semantics. The original declarations fail with duplicate L2 rxc
execution in [before-clean.log](before-clean.log). The repaired graph passes:

- a clean three-stage build, each tool/output exactly once;
- missing L2-image recovery, rebuilding L2 once;
- a changed driver input, rebuilding L0/L1/L2 once each;
- two no-op builds with no tool executions.

[Debug fixture](fixture-debug.json) and
[maintained ASan-tree fixture](fixture-asan-tree.json) measured approximately
15 seconds before CTest registration. `levell_bootstrap_build_order` is serial,
has a 600-second outer/120-second nested hang guard, and joins the normal Unix
optimizer-parity/comprehensive selection when Make is available. It requires
no product preparation. It is not registered on Windows. The nested fixture
uses stand-ins even in an ASan build; real instrumented execution is below.

The real L2 image was removed in each already-qualified build, retaining its
hash first:

```sh
cmake --build cmake-build-levell-integration --target stage-product --parallel 8
tools/asan-run.sh --phase build --build-target llcompiler_poc_measure \
  --build-target levell_bootstrap_order_measure --build-jobs 8 \
  --build-leaks off --no-live-tail --tail-lines 15
```

The ordinary Debug/Ninja product build and the original maintained
Apple-ASan/Unix-Make preparation target both passed. Each generated L2 once:
[Debug log](real-debug-product.log), [ASan log](real-asan-build.log).
ASan then passed all 54 C/G commands in 109.228 seconds, with no sanitizer
finding: [receipt](real-asan-poc.json). Apple LSan is unavailable; this is focused
evidence, not broad sanitizer or platform qualification.

All six corresponding L2, public-library and Unicode bytecode images match
their prior SHA-256 values: [identities](image-identities.json). STEP06 source,
semantic tests and bytecode are unchanged, so their earlier evidence remains
valid. Of the 26 STEP05 Debug/Release products, 24 retain their hashes; the two
Debug native launchers were regenerated. Four fresh isolated invocations of
those launchers (C/G, IR on/off) pass with old compilers absent and produce
byte-identical qualified RXAS: [installed commands](installed-requests.json).
The historical launcher hashes remain in earlier receipts, not overwritten.

## Disposition and publication

F6-PREP-01 is closed for the reproduced parallel-Make failure. The focused
build-order CTest passes in normal Debug and the maintained ASan tree;
registration and logs are retained beside this receipt. Full LLCOMP-AC12
platform/constrained-host delivery remains open. No compiler, assembler,
linker, VM or language implementation changed. No blanket serial build setting,
diagnostic suppression or automatic CI rerun was added.

Adrian authorised publishing this repair and STEP06 together to develop and
switching lightweight monitoring to the resulting commit. Local checks support
publication; normal Build CREXX and CodeQL results are separate hosted evidence.
No status-only follow-up commit is needed. STEP07 is a review proposal only.

Retained text logs have trailing line whitespace removed for Git storage only;
[original and retained hashes](log-normalization.json) record that formatting.
Diagnostic content is unchanged.

# CRI-02 / PERF2-07-B01 `.binary` by-value optimizer worklist

Status: complete; frozen V1 accepted and proportional B4 closeout passed

Started: 2026-07-29

This is the governed performance sub-worklist for
[`CRI-02`](CREXX-RAG-INTEGRATION-WORKLIST.md). The integration worklist remains
the programme control plane and has exactly this CRI item active. Stop at the
mandatory first ordinary profiling-off Release verdict after the first
production performance edit.

## Frozen scope and identities

- Home repository: `/Users/adrian/CLionProjects/CREXX`, branch `develop`, HEAD
  `d78c6fcfa81ef03fdbea65ff9cc39ed99e8716bf`.
- Pre-CRI-02 tracked diff is the already validated CRI-01/04/05/06 programme
  work plus its documentation; SHA-256
  `32d744f793068757ea83085509bebb2fe8fb7de3a5f797ceb6d06c5c6907bd63`.
- Pre-CRI-02 short-status capture SHA-256 is
  `53462025faa2189b26e823f7a477f829a8f3cdf9bbd5dfaf4e5723f5cb6c5aca`.
  It includes the five pre-existing user-owned lifecycle artifacts. They must
  not be changed.
- Read-only reproducer:
  `/Users/adrian/CLionProjects/crexx-rag/incubator/p1a/json_document/binary_argument_optimizer_probe.crexx`,
  SHA-256
  `2050072d552227d7c163771bc32440ee377c908fae98e79c57f216baff6a5586`.
- Build, source, installed-toolchain and any later scratch-install identities
  remain distinct. All CRI-02 builds and evidence live under
  `/tmp/crexx-integration-ledger.mZ5jyp/cri02-*` or the linked retained evidence
  directory.

## Falsifiable hypothesis

The optimizer inlines the small read-only helper but materializes `.binary`
by-value isolation at the inlined call site. That changes a descriptor/read
operation into repeated payload copying, so optimized by-value calls become
slower than non-optimized calls even though direct and exposed controls become
faster. If this hypothesis is right, optimized RXAS and profile/copy counters
will show work proportional to payload bytes on every helper invocation, while
the no-opt call path and exposed/direct controls will not.

This is a single-mechanism, non-representative experiment. RexxCPS may be
omitted from initial attribution under `performance/AGENTS.md`; a production
candidate must still use the smallest decisive product guard set required by
the first Release verdict.

## Semantic and compatibility boundaries

- `.binary` remains a byte value with ordinary by-value snapshot/isolation
  semantics. A helper that writes its formal must not mutate the caller.
- Read-only proof must fail closed across aliases, repeated actuals, branches,
  calls, references, native/plugin boundaries, signals/unwind, imported bodies,
  missing/old summaries and unknown intrinsics.
- Optimized and non-optimized compilation, `rxvm` and `rxbvm`, empty/small/
  large buffers, boundary offsets, mutation isolation, Unicode-independent
  arbitrary bytes and exact checksums are required.
- No public RXAS opcode, RXBIN serialization, public ABI, language syntax,
  ownership rule or normal-prefix installation is authorized.

## Design-selection panel

1. **V0 — status quo.** Keep the defensive inlined by-value materialization.
   It is semantically conservative but retains the measured inversion and is
   the baseline, not the preferred outcome.
2. **V1 — compiler/inliner proof and direct binding (recommended if the
   hypothesis holds).** Extend the existing read-only formal/inlining facts so
   binary read intrinsics do not falsely make the formal writable, then reuse
   the incoming descriptor only where the whole body proves no mutation or
   escape. This owns the fact at the earliest stable phase and should approach
   the exposed/direct machine ceiling without changing value semantics.
3. **V2 — RXAS post-inline copy elimination.** Prove that the materialized
   binary destination is read-only and non-escaping in whole-procedure effects,
   then forward the original value. This can cover handwritten/equivalent RXAS
   but has less source-level ownership context and must preserve cleanup and
   signal behavior.
4. **V3 — runtime copy-on-write/shared payload.** Share binary payloads for
   general by-value calls and detach on mutation. This has broader potential but
   changes the ownership architecture, memory lifecycle and native/ABI risk.
   It is not authorized without a separate measured architecture decision.

Prototype V1 and V2 only if attribution shows both are genuinely viable.
Compare exact dynamic work, code/image size, startup/load, peak memory and both
VMs before selecting production ownership. V3 remains decision-blocked unless
narrower proof cannot meet the gate.

## Machine ceiling and evidence required

- The same workload's optimized `exposed` call is the call-boundary ceiling;
  optimized `direct` access is the no-call inline ceiling.
- Retain optimized/no-opt RXAS, linked image sizes, locals/register ceilings,
  call/inlining decisions, instruction/procedure profiles, copy/value byte
  counters, allocation/high-water evidence where observable, checksums, peak
  RSS, and serial raw wall-clock samples.
- Separate startup-inclusive process elapsed from benchmark-native hot-loop
  elapsed. Profile time is diagnostic only.
- Use ordinary profiling-off Release binaries for acceptance timing. Formal
  target timing uses two warmups plus ten recorded serial samples for each
  absolute baseline cell; a candidate decision uses at least one warmup and 12
  balanced/interleaved paired rounds.

## Acceptance rule frozen before a production edit

Correctness and compatibility are hard gates. For each VM:

1. every by-value, exposed and direct checksum must match, and the mutation-
   isolation matrix must pass in both compiler modes;
2. the optimized by-value median must be lower than the non-optimized by-value
   median and at most `0.90x` it, removing the optimizer-induced inversion by a
   margin larger than the normal measurement band;
3. at least 75% of the reproduced optimized by-value-to-exposed elapsed gap
   must be removed without changing the exposed/direct workload;
4. optimized exposed and direct controls may not regress by more than 3%, and
   the required first-verdict product guard set must have no unexplained
   worse-than-3% workload or worse-than-1% common aggregate result;
5. dynamic copy/instruction and memory evidence must causally explain the
   timing result: no full payload copy proportional to buffer bytes may remain
   on a proved read-only call, no new allocation may replace it, and peak RSS
   must remain within the existing 3% individual guard unless Adrian accepts a
   trade-off.

A result that merely makes optimized by-value equal to the old optimized path,
or obtains speed by exposing the formal, deleting the call boundary, changing
the buffer size/work count, or weakening value semantics, fails.

## Resumable phases

- [x] **B0 — fresh baseline and attribution.** Build exact ordinary Release
  and profiling candidates; reproduce the four cells; retain formal samples,
  RXAS/profile/copy/memory evidence and the exact live-source fingerprint.
- [x] **B1 — bounded PoC comparison.** Compare only evidence-supported V1/V2
  forms in isolated target builds; retain status quo and negative results.
- [x] **B2 — production selection.** Record chosen/rejected forms and prove the
  focused semantic matrix before the first production edit.
- [x] **B3 — mandatory first Release verdict.** Freeze immediately after the
  minimum correctness proof, build ordinary profiling-off Release, run the
  smallest decisive paired target/product guard comparison, report, and stop
  for Adrian. The verdict, bounded adjudication, and Adrian's acceptance are
  retained below.
- [x] **B4 — accepted closeout.** After Adrian's verdict acceptance: rebuild
  the affected product, run focused and broad QA plus affected sanitizer
  coverage, adjudicate regression signals, retain decisive evidence, review
  compatibility and preservation, and update the live control planes.

## Evidence index

Raw baseline and candidate paths, commands, exact counts and fingerprints will
be linked from
[`evidence/2026-07-29-crexx-rag-integration-ledger/README.md`](evidence/2026-07-29-crexx-rag-integration-ledger/README.md)
and from the CRI-02 entry in the integration worklist.

## Reproduced baseline and mechanism attribution

The fresh ordinary Release baseline used the exact read-only probe at the
frozen SHA-256. Two warmups plus ten recorded serial samples per cell gave:

| VM / mode | by-value median | exposed median | direct median |
| --- | ---: | ---: | ---: |
| `rxvm` optimized | 72,911.5 us | 4,493.0 us | 3,663.5 us |
| `rxvm` non-optimized | 18,268.0 us | 17,675.5 us | 4,338.0 us |
| `rxbvm` optimized | 77,371.5 us | 5,246.5 us | 4,729.5 us |
| `rxbvm` non-optimized | 19,713.5 us | 18,976.0 us | 4,597.0 us |

Every checksum was `944025600`. Profiling attributed the optimized inversion
to 614,400 inlined binary copies totaling 7,549,747,200 logical bytes and
614,400 matching `endlife` operations. Non-optimized execution retained
1,228,800 calls and performed no binary value copy. The optimized RXAS copied
the 12,288-byte actual before each typed read.

The isolated V2 machine control removed that exact copy/lifetime pair and used
the caller binary operand directly. It reduced by-value medians to 5,067 us on
`rxvm` and 5,724 us on `rxbvm`, eliminated all measured binary copies, and
preserved both checksums. It proves the machine ceiling but is not a production
RXAS transform because the hand-edited trace register would also need a formal
metadata/liveness rewrite.

The isolated V1 compiler PoC initially remained blocked by two conservative
facts: every binary formal was unconditionally labelled escaping, and an
actual previously used as a reference target was rejected even for a validated
read-only body. Once those false barriers were removed and an aliased formal
was excluded from generated-scope lifetime teardown, V1 reached 5,282 us on
`rxvm` and 5,522 us on `rxbvm` in the first exact dual-VM control. V1 therefore
owns the proof at the source semantic boundary and was selected over V2. V3
remains rejected as unnecessary architectural expansion.

## Frozen production candidate and focused correctness

The selected production change is bounded to compiler analysis and emission:

- typed binary-memory stores mark the base symbol written, fixing a newly
  minimized pre-existing by-value isolation defect in the non-optimized path;
- the inline summary no longer labels every `.binary` formal escaping solely
  because of its type;
- a direct caller-local binary actual can share a validated read-only,
  exact-shape, non-escaping formal, while exposed/global/ref actuals and every
  written/escaping/optional/reference/aggregate case still fail closed; and
- generated lifetime emission does not end a caller register shared by an
  inline alias.

There is no syntax, public RXAS/RXBIN, serialized schema, VM instruction,
runtime ownership, or public ABI change. The maintained contract covers direct
codegen, writable mutation and rebinding isolation, repeated actuals, explicit
reference history, empty/arbitrary/8 KiB binary values, optimized and
non-optimized compilation, and both VMs. The maintained benchmark preserves
the by-value, exposed, and direct kernels.

After the production edit, the minimum focused Debug build and correctness
gate passed 6/6 registered tests (one linked-runtime fixture, one compiler/
dual-VM semantic contract, and four optimized/non-optimized benchmark cells).
The implementation was then frozen. No full CTest, sanitizer, package/install,
portfolio, or documentation closeout was run after this edit.

## Mandatory first ordinary Release verdict

The clean candidate is an ordinary Release build with
`CREXX_VM_PROFILING=OFF`. Its `rxc` SHA-256 is
`8c6aa738f1841d4e0f565903e469a7639e9b2d6dbe9c512e379fda15e641a498`.
The exact probe was rebuilt from the read-only source; optimized RXAS reads the
caller register directly, contains no defensive copy or caller `endlife`, and
the optimized RXBIN remains 10,365 bytes, identical in size to baseline.

One warmup plus twelve balanced/interleaved recorded rounds across baseline and
candidate, both compiler modes, and both VMs produced these kernel medians:

| VM / cell | by-value | exposed | direct |
| --- | ---: | ---: | ---: |
| `rxvm` baseline optimized | 72,564.0 us | 4,380.5 us | 3,801.0 us |
| `rxvm` candidate optimized | 5,284.5 us | 5,017.5 us | 3,997.5 us |
| `rxvm` baseline non-optimized | 18,280.5 us | 17,733.0 us | 4,233.5 us |
| `rxvm` candidate non-optimized | 18,112.5 us | 17,665.0 us | 4,221.0 us |
| `rxbvm` baseline optimized | 77,261.5 us | 5,302.0 us | 4,615.5 us |
| `rxbvm` candidate optimized | 5,677.5 us | 5,329.5 us | 4,651.5 us |
| `rxbvm` baseline non-optimized | 19,904.5 us | 19,342.5 us | 4,636.0 us |
| `rxbvm` candidate non-optimized | 19,996.0 us | 19,200.0 us | 4,615.5 us |

All 104 warmup/recorded child executions passed with exact checksums; no sample
was removed. The optimized by-value improvement is 92.72% on `rxvm` and 92.65%
on `rxbvm`. Candidate optimized by-value is respectively 0.2918x and 0.2839x
candidate non-optimized, and removes 99.61% and 99.52% of the reproduced
by-value-to-exposed gap. Startup-inclusive process medians fall from 94.742 ms
to 28.004 ms on `rxvm` and from 100.733 ms to 29.254 ms on `rxbvm`.

The frozen acceptance rule is nevertheless **not fully accepted**. `rxbvm`
adjacent controls stay within the 3% guard (`exposed` +0.52%, `direct` +0.78%),
but `rxvm` reports `exposed` +14.54% and `direct` +5.17%. Candidate optimized
samples also crossed the runner's spread-based rerun recommendation even
though MAD was 2.55% (`rxvm`) and 1.21% (`rxbvm`). Because the probe always
runs by-value before exposed/direct, removing about 67 ms of preceding copy
work is a plausible frequency/warm-state sequencing confound, but that is an
inference, not a waiver of the predeclared guard. Peak RSS and broader product
guards remain deliberately unrun at this mandatory stop.

Raw evidence is under `/tmp/crexx-integration-ledger.mZ5jyp/`:

- `cri02-baseline-*`, including formal samples, profiles, memory, fingerprints,
  and summaries;
- `cri02-v1-*` and `cri02-v2-*` isolated PoCs;
- `cri02-focused-configure.log`, `cri02-focused-build.log`,
  `cri02-focused-ctest.log`, and `cri02-focused-list.log`;
- `cri02-release-candidate/`, `cri02-candidate-configure.log`,
  `cri02-candidate-build.log`, `cri02-candidate-hashes.txt`, and exact
  candidate RXAS/RXBIN work directories;
- `cri02-first-verdict-manifest.txt`, the complete `cri02-first-verdict/`
  matrix, host pre/post captures, kernel/process median summaries, and
  acceptance calculation.

The first `compiler_exit_bin` build attempt exposed an existing parallel Make
dependency race: consumers requested `token.rxbin` before `token_bin` finished.
The retained retry after `token_bin` existed passed without source change. It
did not affect the compiler/runtime/library product or benchmark result.

`git diff --check` passes at the stop. The read-only crexx-rag audit is
byte-identical to the post-CRI-05 audit (both files SHA-256
`9e6cf62987a1501179a620a5888fb7f5a4b1ecf4782dd1c99dd0b7a3873613f9`):
branch `main`, HEAD `97cd87e91344d6ac1773a054bd38df23eb128ed2`, tracked
diff `97443028cfa8e86624fc5fda9ea5fb33d02f4d7413e5a8a848d92ec365288ef9`,
empty index diff `30cea35503c6dc073f3007218b9458f2bc0c28b2c7661327b9144036d5a7c61d`,
and porcelain-v2 status
`02a6e4216ff47be3ce7252be2ccb39157bc45fda316c254f7198648109df14f1`.

## Authorized bounded guard adjudication

Adrian authorized the recommended bounded adjudication on 2026-07-30. No
production source, kernel, dimension, iteration count, by-value boundary,
compiler mode, checksum, or frozen product was changed. Temporary Level B
controls retained each original kernel while rotating the complete phase order
from by-value/exposed/direct (`BED`) to exposed/direct/by-value (`EDB`) and
direct/by-value/exposed (`DBE`), then ran exposed and direct in separate
processes. The optimized `rxvm` comparison used one warmup and 12 balanced
pairs, the required 10-sample absolute-noise append, and the required 12-pair
guard append: 34 recorded pairs per cell, with no sample removed. All 350 child
executions (10 warmup and 340 recorded) passed checksum `944025600`.

The apparent adjacent-path regression follows phase position, not candidate
code:

| Control position | Exposed median delta | Direct median delta |
| --- | ---: | ---: |
| `BED` (both after by-value) | +6.23% | +12.51% |
| `EDB` (both before by-value) | +2.45% | -1.55% |
| `DBE` (direct before, exposed after by-value) | +10.70% | -1.31% |
| isolated process | -2.93% | -1.15% |

The paired medians for the isolated controls are -1.22% exposed and +1.73%
direct, both inside the frozen 3% guard. Their mean intervals still cross zero
at the governed 34-pair cap, so the short internal-timer series are retained as
noisy/inconclusive; no favorable subset was selected. Causal evidence is
nevertheless exact: baseline and candidate isolated-control RXAS are
byte-identical, and disassembly differs only in module/description path
metadata. The only control that slows is whichever one follows the baseline's
approximately 73 ms defensive-copy phase; the candidate removes that
preconditioning. The first-verdict `rxvm` guard hit is therefore a fixed-order
warm-state measurement artefact, not an exposed/direct code regression.

Peak RSS used zero warmups and three balanced observations per optimized cell.
Median RSS changed from 17,678,336 to 17,580,032 bytes on `rxvm` (-0.56%) and
from 17,612,800 to 17,547,264 bytes on `rxbvm` (-0.37%). Both pass the frozen
3% rule and the standing greater-than-5%-and-1-MiB escalation rule. All 12 RSS
executions passed; AC power, low-power mode off, and no thermal/performance
warning were recorded before and after.

The countermeasure belongs in measurement design, not production code: when an
earlier phase changes by an order of magnitude, execute later guard kernels in
separate balanced processes or fully rotate phase order. Keep the fixed-order
probe as an end-to-end inversion reproducer, but do not treat its later phases
as independent guards. Raw manifests, outputs, summaries, host state, the Level
B paired analyzer, hashes, and commands are under
`/tmp/crexx-integration-ledger.mZ5jyp/cri02-adjudication/` and
`cri02-adjudication-control-src/`. The 58-entry checksum manifest verifies and
has SHA-256
`9c7307fe825228ef466e2c390dd2dc93354cc8e5b35c932435a246c851f28c81`.
The four production-source hashes still match the frozen candidate record and
`git diff --check` passes. The repeated read-only crexx-rag audit remains on
`main` at `97cd87e91344d6ac1773a054bd38df23eb128ed2` with tracked diff, empty
index diff, and porcelain-v2 hashes respectively `97443028cfa8e86624fc5fda9ea5fb33d02f4d7413e5a8a848d92ec365288ef9`,
`30cea35503c6dc073f3007218b9458f2bc0c28b2c7661327b9144036d5a7c61d`,
and `02a6e4216ff47be3ce7252be2ccb39157bc45fda316c254f7198648109df14f1`.

## Acceptance and proportional closeout

Adrian selected the recommended bounded guard adjudication on 2026-07-30
(`option 1` in the handoff response, corresponding to option 2 in the original
decision list). The frozen candidate remains unchanged. Authorization is limited
to balanced `rxvm` order/per-variant controls and peak RSS against the retained
exact baseline, followed by another stop for acceptance or reversion. Broad QA,
portfolio work, production edits, and later CRI items remain unauthorized.

The bounded evidence attributes the prior guard hit to measurement sequencing,
and the isolated controls plus peak RSS satisfy the frozen point guards. The
recommended decision is therefore **accept the frozen V1 candidate and
authorize B4 proportional closeout**. The alternative is **reject the
candidate, revert only the read-only inline-alias portion, and retain the
binary-store write-use correctness fix**, which restores the measured
optimizer inversion. No guard waiver is required by the recommendation.

Adrian accepted the recommended frozen V1 candidate and authorized continuation
on 2026-07-30. B4 is now limited to the proportional closeout described above;
the acceptance does not authorize a new performance candidate, portfolio
expansion, public-contract change, normal-prefix install, or publication.

The first complete post-acceptance Debug run passed 1932/1934. Both failures
were optimized/non-optimized `select_dispatch_strings` goldens. Exact
normalized comparison proved that every instruction, source map, serialized
inline body, and runtime path was unchanged; only the I6 callable summary's
`.binary` formal flags changed from `464` to `400`. The 64-point difference is
`RXCP_INLINE_FORMAL_ESCAPES`, whose removal for an independently proved
read-only exact binary formal is the V1 evidence opening, not a cost or
format-field change.

A minimized cross-module Level B control proved the countermeasure. Its
read-only formal exported mask `400`; its typed-write formal exported `416`
(`WRITTEN`, exact shape, and read); both masks survived
RXAS/RXBIN/RXDAS/RXBIN. Optimized import bound only the reader to the caller
register and retained an isolated copy for the writer; non-optimized import
retained both call boundaries. Optimized and non-optimized artifacts passed on
`rxvm` and `rxbvm`, with caller mutation isolation preserved. That matrix is
now maintained by `perf2_07_binary_byvalue_contract`, and the two goldens were
changed only from `464` to `400` after the proof.

B4 results:

- focused normal matrix: 8/8 passed in 6.78 seconds;
- expanded maintained contract: 1/1 passed in 5.27 seconds;
- affected Apple ASan matrix: 3/3 passed in 14.19 seconds with
  `ASAN_OPTIONS=detect_leaks=0`; Apple LSan remains unsupported;
- complete Debug CTest: 1934/1934 passed, zero failed or skipped, in 213.67
  seconds with `--parallel 30 --output-on-failure`;
- all four frozen production-source hashes remained byte-identical to the
  accepted candidate record, and `git diff --check` passed;
- the read-only crexx-rag audit remains `main` at
  `97cd87e91344d6ac1773a054bd38df23eb128ed2`, with the retained tracked diff,
  empty index diff, and no-branch porcelain hashes unchanged.

Raw closeout paths under `/tmp/crexx-integration-ledger.mZ5jyp/`:

- `cri02-b4-import-repro/` and `cri02-b4-import-repro.log`;
- `cri02-b4-contract-ctest-2.log`,
  `cri02-b4-focused-after-golden.log`, and
  `cri02-b4-full-debug-ctest-final.log`;
- `cri02-b4-asan-logs/20260730-082925-ctest/ctest.log` and its prerequisite
  build runs;
- `cri02-b4-select-opt.diff`, `cri02-b4-select-noopt.diff`, and
  `cri02-b4-select-golden-update.log`;
- `cri02-b4-candidate-fingerprint.txt`, `cri02-b4-closeout-audit.txt`, and
  `cri02-b4-crexx-rag-audit.txt`.

The approved closeout path requires no repeat of valid baseline or portfolio
evidence, and this compiler-only slice adds no installed SDK/runtime artifact.
Scratch-install and external-consumer proof belongs to the now-active CRI-07.

Historical paste-ready continuation prompt used at the acceptance gate:

> Resume `/Users/adrian/CLionProjects/CREXX` at active item CRI-02 /
> PERF2-07-B01 from `performance/CRI02-BINARY-BYVALUE-WORKLIST.md`. Adrian
> accepts the frozen V1 candidate after the completed bounded guard
> adjudication. Preserve the raw evidence and follow only the proportional B4
> closeout required by `performance/AGENTS.md`: remove disposable PoCs where
> appropriate, rebuild the affected product, run focused checks plus the
> required broad CTest and affected sanitizer workflow, retain the decisive
> benchmark evidence, update live documentation, review the diff, and stop
> with CRI-02 in an accepted `fixed` disposition before starting CRI-07. Do not
> rerun valid baselines, expand the portfolio, change public RXAS/RXBIN/ABI, or
> stage, commit, push, install normally, or touch the read-only crexx-rag tree.

## Closeout point

CRI-02 / PERF2-07-B01 is closed as **fixed**. Its performance stop,
adjudication, and B4 gates are satisfied. CRI-07 is the sole active
integration-ledger item.

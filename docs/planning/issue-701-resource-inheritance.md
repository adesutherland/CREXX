# Issue 701: child resource inheritance

## Vision and authority

Concurrent child launches must inherit only their intended resources on macOS,
Linux and Windows. One worker must not keep another worker's pipes or temporary
files alive, delay its completion, or interfere with publication/rename. Preserve
useful concurrency, normal standard-stream inheritance, output draining,
cancellation and cleanup, including repairs for #646, #669, #678 and #697.

Work only in `/Users/adrian/CLionProjects/CREXX-hotfix` on persistent `hotfix`.
Ordinary evidence-backed repairs, commits, pushes, qualification and promotion
to `develop` are authorized. Language/API design and architectural changes
require approval. No release/tag, llama/CUDA/model/performance workload, issue
comment posting, or changes to the separate llama checkout/qualification.
Prepare an evidence-backed update for existing #701 without posting it.

## Acceptance criteria

- [x] AC-01: deterministic POSIX regression observes A's exit/completion while
  sibling B remains alive, without unrelated pipe ends retaining EOF.
- [x] AC-02: deterministic Windows controls establish/prevent unintended handle
  inheritance for absent, partial and complete standard-stream redirection on
  both MSVC and MinGW; explicitly investigate temporary-file rename behavior.
- [x] AC-03: regressions fail on defective baseline and pass after repair, using
  barriers/handshakes or bounded fault injection, not sleeps or lucky retries.
- [x] AC-04: document ownership of pipe/file creation, inheritance flags, child
  setup and failure cleanup, covering creation-to-flag windows and both ends.
- [x] AC-05: interactive stdin, prompt visibility, inherited stdout/stderr,
  explicit captures, output draining, cancellation, exit status and teardown
  remain correct; retain #646/#669/#678/#697 behavior.
- [x] AC-06: worker failures distinguish missing output, stamp write and rename
  operations with useful diagnostics, without retrying away failures.
- [x] AC-07: focused Debug/maintained sanitizer, broader core regression and
  target-platform gates pass; unverified platform cells remain explicit.
- [ ] AC-08: retain exact revisions, commands, logs and outcomes; promote the
  qualified repair to develop and check required post-promotion workflows to
  honest terminal status.

## Implementation steps

1. STEP-01 (AC-04/08): reconcile issue/source/history and record baseline.
2. STEP-02 (AC-01/02/03/05): minimal independent POSIX and Windows reproducers
   and controls, with a native helper where OS inspection is required.
3. STEP-03 (AC-04/05/06): repair proved generic-launcher mechanisms and worker
   diagnostic gaps, preserving concurrency and stream contracts. Depends on
   reproducer evidence; stop for any language/API/architecture decision.
4. STEP-04 (AC-01..07): focused checks then broader core qualification through
   `tools/asan-run.sh` for sanitizer work; Linux leak detection stays enabled.
   Register actual first-party sanitizer findings in SANITIZER-WORKLIST.md.
5. STEP-05 (AC-08): commit/push hotfix, complete required hosted gates, promote
   qualified repair to develop, reconcile post-promotion results and limits.

## Baseline and evidence

2026-09-16: clean hotfix initially `800f381848c1de55ea73bf563de066c45e399996`,
identical to origin/hotfix. Refreshed origin/develop is eight commits ahead at
`b5b827489d781f9e42d305ef264a22d2c1c42cb6`; fast-forwarded hotfix to that SHA.
Unrelated changes are upstream RXPP and installer work. No main-checkout edits.
`rxspawn.c` blob remains `c88ab671291d5ddf8f5a7d276c553a44babba3b0`.
Issue #701 was read live through `gh issue view 701 --json ...`: open, no comments.

Read CI-F18 and related-issue review in the separate checkout's
`docs/planning/native-inference-ci.md`, supporting `local/f18-race-review/`
and `remote/f10e70ee5/` under its native-inference-ci QA ledger. The original
MinGW silent failure plus unchanged successful retry are historical evidence,
not repaired cause. Windows partial-inheritance and FOPEN open-to-clear windows
are source findings; attribution of the CI-F18 failure stays unproved unless
independently reproduced. No extra project-worker serialization is proposed.

## Qualification ledger

Current status is recorded in the final candidate section below. Earlier
entries retain the investigation chronology and do not override that status.

## STEP-02/03 implementation evidence and ownership (16 September)

The deterministic POSIX native harness includes the real launcher, creates A's
redirect, launches B before A releases its child end, and asks the executed B
whether that exact descriptor survived exec. B blocks on a pipe handshake.
On the unmodified baseline, two assertions fail: A's writer lacks CLOEXEC and
B owns that writer. After B is explicitly released, both children and the output
reader are reaped without a hang. Retained `docs/qa/issue-701/posix-baseline.log`.
After repair A exits with status 7, its 13 output bytes drain, and WaitForProcess
returns while B is provably still alive. No sleep/repetition is involved.

The private pipe helper holds the existing launch mutex across pipe creation,
FD_CLOEXEC on **both** ends, and relocation above descriptors 0..2. Launch status
uses the locked helper while holding the same mutex through fork. The parent
unlocks immediately after fork; child execution and output draining remain
parallel. Child dup2 creates only the intended standard streams; all original
pipe descriptors disappear at exec. Injecting a second-end flag failure proves
both ends close, errno survives and the mutex is released. Test interception
checks that the lock is already held at the instant pipe returns. The test also
covers a host with all standard descriptors closed.

Byte-endpoint and legacy string/array paths share redirect_pipe_start. The
completion owns only its parent I/O end; the redirect owns its child end.
Creation/thread failures release both ownership domains; joins remain the
publication/acquire boundary. No cancellation/group policy changes are made.
Null redirects also open privately; dup2's same-descriptor case clears CLOEXEC.

Windows creates private pipes, then launchChild duplicates each valid intended
standard handle into a spawn-owned inheritable handle. This applies to absent,
partial and complete redirection, with parent standard-handle flags unchanged.
Every launch supplies an explicit allowlist; when there are no valid handles,
CreateProcess disables inheritance. Startup cleanup closes every successful
duplicate and destroys the attribute list on success or failure. Another launch
cannot admit these transient duplicates because its own allowlist excludes them.
No process-global stream mutation or execution serialization is introduced.

FOPEN uses the CRT's atomic N mode on Windows, libc's e mode on Linux, and
open(O_CLOEXEC)/fdopen on Darwin (whose fopen does not support e). Darwin retains
its existing r/w/a, binary/update/exclusive parsing and initial append position.
Standard stream pseudo-filenames still bypass file creation. The focused helper
test checks first-return inheritance flags, binary/update/append/exclusive modes,
failed-open errno and cleanup. There is no open-to-clear interval.

Windows controls cover all eight redirect masks with inheritable standards and
again with private parent standards. An unrelated inheritable disk file without
FILE_SHARE_DELETE freezes the proposed open-to-clear state. The child reports
file identity, verifies intended input/output/error and waits on named events;
the parent closes its owner then attempts rename while that child is alive.
The historical baseline and repaired launcher compile separately from identical
test source. Windows results remain pending; even reproduction of this mechanism
does not establish the cause of CI-F18's historical silent return.

Sources: Microsoft UpdateProcThreadAttribute (HANDLE_LIST contract), Microsoft
fopen documentation (N/_O_NOINHERIT), Apple Libc stdio/FreeBSD/flags.c and fopen.c
(no e mode, initial append behavior). Retrieved 16 September 2026.

Focused normal Debug passes 2/2 in 0.06 seconds, runner evidence
`cmake-build-debug/asan-logs/20260916-113748-ctest`. Small native tests have
60/180-second hang guards; they do not build toolchains or scenario matrices.
STEP-01 complete; STEP-02/03 in progress; platform acceptance remains open.

Focused maintained Apple ASan also passes 2/2 (0.97 seconds),
`cmake-build-debugasan/asan-logs/20260916-113823-ctest`; no sanitizer diagnostic.
Apple leak checking is disabled only for its documented unsupported capability.

Initial hosted controls at `502fb28dc080342df7a2db0a1e8f272915e8517e`, run
35086216671: Linux and macOS baseline-negative/current-positive pass. MSVC is
blocked before compilation by an existing test-registration bug:
`inline_receiver_signal_cleanup` unconditionally names nonexistent `rxtvm`.
Retained MSVC configure log in `docs/qa/issue-701/remote-502fb28/msvc/`.
A separate ordinary CMake repair supplies the threaded VM only where built,
retaining the portable VM test on MSVC. It does not change compiler semantics.

MinGW run 35086216671 proves the Windows mechanism: baseline masks 0..6
inherit the unrelated file and rename fails with ERROR_SHARING_VIOLATION (32);
fully redirected mask 7 is the passing control. Repaired masks 0..7 all exclude
the file and rename succeeds while B is alive, for both inheritable and private
parent standard handles (16 cases). Retained baseline/current/build logs under
`remote-502fb28/mingw/`. This proves the proposed mechanism, not historical F18
attribution. The separate FOPEN test incorrectly assumed POSIX append position
and exclusive-x support on every Windows CRT. It now compares those behaviors
to the same host's ordinary fopen, while still requiring private creation.


At `bd19a94ddd597459eb1fa01cae1cf33294d063b5`, Child Inheritance QA
35086663828 completes successfully on all four platforms: Linux, macOS,
MSVC and MinGW. Raw artifacts are retained in `docs/qa/issue-701/remote-bd19a94/`.
The separate MSVC configure repair is `b50e4f3d7`; it changes registration only.
Additional missing/invalid Windows-standard-handle and handle-count cleanup
controls are being added before freezing final qualification inputs.

The diagnostic fixture forces three independent failures using a native fake
compiler/assembler: missing output, an unwritable stamp path, and a conflicting
rename destination. Tasks retain two-worker concurrency. Debug passes, measured
in isolation at 2.99 seconds (`/tmp/crexx-701-diagnostic-measure.x1jWgq`). Its
shared parent directory is created before task submission, as in the real
project-builder layout. ASan measurement is pending. The initial ASan build
reached its test command with an earlier generated command lacking the worker
import path; that configuration error is corrected, with no sanitizer finding.

Required final gates: local normal Debug core CTest (exclude only the separately
owned performance-measurement tier); focused maintained Apple ASan including
inheritance, file-open, child lifecycle and diagnostic controls; hosted
Child Inheritance QA, Deep Build QA and full Sanitizer QA on hotfix. Deep covers
comprehensive core/install/package and isolated stress checks, so an additional
manual Build comprehensive run would duplicate it. After promotion, automatic
Build CREXX and CodeQL must reach terminal success; no manual version tag/release.
Linux sanitizer keeps build/test leaks enabled. Performance/model/GPU work is
outside this task. The separate llama checkout and its runs remain untouched.


The diagnostic matrix passes under maintained Apple ASan in 14 seconds, runner
`cmake-build-debugasan/asan-logs/20260916-114934-build`. Registered only after
Debug/ASan measurements as `crexx_project_worker_diagnostics`, with RUN_SERIAL,
its existing bin workspace resource lock and a 180-second hang guard. No retry
or change to project-worker concurrency. The operation diagnostics preserve all
existing RexxDoc blocks. STEP-02/03 complete for implementation; full acceptance
still awaits the qualification gates, including the final expanded Windows tests.


## Final candidate qualification (in progress)

Code candidate: `135b9254fffdd0c9a8e963d1092c68bb273bf66b`, pushed to hotfix.
All 32,616 non-documentation tracked input hashes are frozen in
`docs/qa/issue-701/local-final/frozen-inputs.json.gz` (uncompressed manifest
SHA256 `aad62f034d47497a6f4f92973cc052a820bfd51a9133d5246cccc6174cfe5aa0`).
Both final focused panels pass 7/7: Debug 4.96s at
`cmake-build-debug/asan-logs/20260916-120345-ctest`; maintained Apple ASan 7.00s at
`cmake-build-debugasan/asan-logs/20260916-120503-ctest`. Logs retained in local-final.
The isolated cold diagnostic measurement remains 14s; subsequent warm execution
is faster and does not replace the conservative scheduling evidence.

Hosted exact-candidate gates:
- Child Inheritance QA: 35088226165 (success, all four platforms).
- Deep Build QA: 35088225950 (success, all required jobs).
- Sanitizer QA: 35088228552 (cancellation requested by Adrian; not a full sanitizer pass).
Local full Debug build and QA prep passed before the final CTest sweep. Broad
CTest excludes only performance-measurement; no other local build runs alongside.
No promotion has occurred. Original acceptance IDs and full vision remain active.


Child Inheritance QA **35088226165 succeeds on the final candidate**, all four
platforms. This includes the additional 16 Windows cases for NULL/invalid
standard handles and unchanged parent handle counts after both successful and
failed CreateProcess paths. Final artifacts are under
`docs/qa/issue-701/remote-135b925/inheritance/`. AC-01/02/03/04/06 are verified;
AC-05/07/08 remain open pending broad qualification and promotion.


Local broad normal Debug passes **2,306/2,306**, 902.27 seconds, no retries or
failures. Runner: `cmake-build-debug/asan-logs/20260916-120546-ctest`. Retained
`docs/qa/issue-701/local-final/broad-debug.log.gz`. Command:

```sh
tools/asan-run.sh --build-dir cmake-build-debug --phase ctest --test-jobs 30 --exclude-label '^performance-measurement$' --keep-going --no-live-tail --tail-lines 25
```

The sweep includes the interactive/TTY stdin and prompt tests, optimized and
unoptimized ADDRESS captures/arrays/transfers, process byte-provider lifecycle,
POSIX termination and launch diagnostics, project-build contract (96.20s),
process runtime (83.17s), and deterministic worker diagnostics (2.61s).
All 32,616 frozen inputs still match. Hosted comprehensive Linux and macOS ARM64
also pass: each 2,292 correctness tests plus three install/package/external
consumer tests. Windows/Intel macOS comprehensive and both full sanitizer jobs
remain pending. No source or test change, no repeat broad local run.


AC-05 is verified by the completed local broad sweep, focused Debug/ASan
lifecycle controls, and completed Linux/Apple ARM64/Windows comprehensive
matrices. The existing stdin/prompt, ADDRESS flush/capture/array-transfer,
terminal process-group, typed-timeout, cancellation and teardown regressions
remain active. Windows comprehensive passes 2,214 correctness tests plus three
install/package/external-consumer tests; worker diagnostics pass there in 0.80s.
AC-07 (remaining full hosted gates) and AC-08 (promotion/post-promotion) are open.


## Publication-policy clarification during qualification

Adrian questioned why overnight assurance was being treated as an every-publish
gate. Live inspection confirms that `Building_cRexx.md` and the workflow YAML
retain fast PR/develop publication, scheduled deep/comprehensive assurance, and
scheduled full sanitizers. The two long runs above were explicitly dispatched
for this task by the agent, not triggered by hotfix publication. The agent
acknowledged overbroad gate selection; full build-graph checks and entire
sanitizer suites are not automatic requirements for every runtime repair.
The sanitizer guide's mandatory-CI/required-branch-check wording and AGENTS'
unspecified required-publication-gate language need clearer alignment with the
current build guide. No workflow trigger or general QA policy has been changed.
Deep QA has already passed; the existing sanitizer run remains active. This
question alone is not recorded as approval to cancel it or weaken the original
acceptance criteria. Do not dispatch these long workflows again merely for
promotion or documentation-only bookkeeping; reuse unchanged evidence.


Deep Build QA **35088225950 is terminal success** at the exact code candidate.
Linux, macOS ARM64 and macOS Intel each pass 2,292 correctness tests; Windows
passes 2,214. Each platform also passes all three install/package/external
consumer checks. Isolated Debug stress passes 9/9. Release jobs 1/5/30,
immediate no-op, missing-dependency and change-closure controls pass, with
identical bytecode manifests. Logs, manifests and terminal job metadata are
retained under `docs/qa/issue-701/remote-135b925/deep/` and `deep-status.json`.
The scheduled-only assurance-marker job is skipped on this manual dispatch;
that is expected and is not a missing product check. Full sanitizer results
remain pending.


## Approved publication-policy correction and remaining closeout

Adrian explicitly directed on 16 September: update guidance so core build and
functional success suffice for normal develop integration; deeper pre-publish
work is an exception (for example novel llama work); nightly findings can be
repaired the next morning. He asked whether appropriate tests had passed and,
if so, to cancel the remaining long run and move on.

The evidence is sufficient: 2,306 local Debug tests, focused Debug/Apple ASan
7/7 each, baseline-negative/current-positive controls on Linux, macOS, MSVC and
MinGW, plus the already completed full cross-platform Deep QA. AC-07 is
therefore verified under the explicitly corrected scope. The still-running
full sanitizer workflow 35088228552 was cancelled at his direction; it must not
be cited as full Linux ASan/LSan or macOS ASan success. There is no new observed
first-party sanitizer finding. This replaces the agent's overbroad manual-gate
selection above, without weakening the product behavior criteria.

Canonical AGENTS.md, the sanitizer guide and the programming build guide now
state the default and exception policy consistently. No workflow trigger changes
are needed. Runtime/test/build inputs remain identical to 135b9254f, so broad
qualification is reused. STEP-01..04 are complete. STEP-05/AC-08 remain pending
normal develop promotion and the automatic Build CREXX/CodeQL terminal results.
Final result bookkeeping will be retained on hotfix without a second pointless
develop publication solely to update the evidence ledger.

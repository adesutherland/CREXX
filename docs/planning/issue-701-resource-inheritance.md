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

- [ ] AC-01: deterministic POSIX regression observes A's exit/completion while
  sibling B remains alive, without unrelated pipe ends retaining EOF.
- [ ] AC-02: deterministic Windows controls establish/prevent unintended handle
  inheritance for absent, partial and complete standard-stream redirection on
  both MSVC and MinGW; explicitly investigate temporary-file rename behavior.
- [ ] AC-03: regressions fail on defective baseline and pass after repair, using
  barriers/handshakes or bounded fault injection, not sleeps or lucky retries.
- [ ] AC-04: document ownership of pipe/file creation, inheritance flags, child
  setup and failure cleanup, covering creation-to-flag windows and both ends.
- [ ] AC-05: interactive stdin, prompt visibility, inherited stdout/stderr,
  explicit captures, output draining, cancellation, exit status and teardown
  remain correct; retain #646/#669/#678/#697 behavior.
- [ ] AC-06: worker failures distinguish missing output, stamp write and rename
  operations with useful diagnostics, without retrying away failures.
- [ ] AC-07: focused Debug/maintained sanitizer, broader core regression and
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

All cells pending: macOS Debug, macOS ASan (LSan unsupported), Linux Debug,
Linux ASan/LSan, Windows MSVC, Windows MinGW; hotfix hosted gates and develop
post-promotion workflows. STEP-01 in progress; STEP-02..05 open.

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

# Initial concurrency independent-review prompt

Use the text below to start the fresh-agent review and remaining qualification.
It is intentionally evidence-led and does not authorize a second publication
action or new concurrency design.

---

Work in the CREXX repository on `develop`. Review and qualify the frozen
**initial concurrency** implementation; use no other maturity label.
Your outcome is an independent evidence verdict for SP-01 through SP-09 and
QA-B through QA-D after publication. Adrian has already selected publication
as **initial**. Corrected commit `53b3de77a` passed the four-platform Build
CREXX workflow, development-snapshot publication and CodeQL; that CI signal
does not replace native qualification, and passing tests do not make the
surface stable. Windows QA-D subsequently passed MSVC, Clang and GCC at exact
commit `2b793c81e0987f627ab72e3c4e505ae5c6a95abe`.

First read the repository `AGENTS.md`, then these authorities in order:

1. `docs/ROADMAP.md`, section “Concurrency Roadmap”;
2. `concurrency/README.md`;
3. `concurrency/QA-CLOSEOUT.md`;
4. `concurrency/SOLUTION-REVIEW.md`;
5. `concurrency/TEST-MANIFEST.md`;
6. `concurrency/qa/README.md` and `concurrency/qa/EVIDENCE.md`; and
7. `performance/AGENTS.md` before any performance command.

Treat `docs/ROADMAP.md` as the sole roadmap. The concurrency worklist and
closeout files are status/evidence ledgers. Historical development-gate names
are provenance only.

## Boundaries

- Record `git rev-parse HEAD` as a full commit and require a completely clean
  checkout. Do not stash, reset, rebase, branch, create a worktree, push or
  overwrite user changes.
- Keep the public language, RXAS/RXBIN, ABI and architecture frozen. A change
  to any of those requires Adrian's approval and a numbered plan before edits.
- Linux and Windows normally act as validation hosts. Do not edit source there
  without Adrian's explicit direction. Any directed bounded repair must be
  committed and followed by a complete replay from a new clean exact commit.
- Do not overlap build and test processes, particularly on Windows. Do not
  convert environmental failures or live TLS failures into skips.
- Exercise `rxc`, `rxas`, `rxlink` and the applicable VM paths. Library work is
  also compiler, assembler and linker regression coverage.
- Preserve RexxDoc and public documentation with any separately authorized
  repair. Do not silently weaken a test, timeout, assertion or expected result.
- Keep binary package artifacts outside the repository. Add retained evidence
  to the repository only after digest verification on Mac and only when asked
  to make the corresponding local evidence commit. Never push.

## 1. Preflight and independent solution review

Do not accept `concurrency/SOLUTION-REVIEW.md` solely because it says PASS.
For every SP-01 through SP-09 row:

1. inspect the named implementation and public declarations;
2. inspect the positive, negative, optimized/unoptimized and both-VM tests;
3. confirm its CTest label is non-empty and all labelled cases carry the
   `concurrency` umbrella label, a timeout, and useful failure diagnostics;
4. check lifecycle, cancellation, ownership, transfer and error paths relevant
   to the solution point; and
5. record `confirmed`, `defect`, or `evidence gap`, with source/test pointers
   and residual risk.

Run the maintained entry point on Mac after the review:

```sh
cmake --build cmake-build-debug --target concurrency-qa --parallel 10
```

If the commit or test inventory differs from the retained closeout, explain
the difference before continuing.

## 2. Finish QA-B on quiet AC power

The correctness, Apple ASan, stress, Release, install and portable-package
closeout is retained under `concurrency/evidence/2026-08-16-mac-closeout/`.
Review that evidence, but do not rerun it without a concrete inconsistency.

The only planned Mac execution is the governed performance replay. Confirm the
Mac is on AC power, low-power mode is off, and the host is otherwise quiet.
Then replay the exact retained task-launch and seven-workload manifests and the
unchanged Richards confirmation from
`performance/evidence/2026-08-16-initial-concurrency-mac-closeout/`. Do not edit
the harness or manifests. Apply the mandatory Release verdict and guard rules
from `performance/AGENTS.md`, retain samples and power-state evidence, and
report `pass`, `guard breach`, or `invalid environment` for `rxvm` and `rxbvm`
separately. Stop the performance slice if its mandatory guard requires Adrian's
direction.

## 3. Execute QA-C on Linux

Transfer the exact clean commit to the supported Linux host. Choose fresh,
empty directories outside the repository and run:

```sh
expected_commit=$(git rev-parse HEAD)
concurrency/qa/run-linux-qualification.sh \
  "$expected_commit" /absolute/path/crexx-concurrency-build \
  /absolute/path/crexx-concurrency-evidence
```

The runner is the command contract. Do not substitute ad hoc platform commands
for it. Verify `RESULT.txt`, `SHA256SUMS`, every SP label inventory, full CTest,
live TLS, the 20-cycle stress run, installed smoke, ZIP smoke and extracted
Debian-package smoke against `concurrency/qa/EVIDENCE.md`. Preserve the sibling
artifact directory separately; do not commit its binaries.

## 4. Review the completed QA-D Windows evidence

Review `concurrency/evidence/2026-08-17-windows-qualification/`. Confirm each
MSVC, Clang and GCC `RESULT.txt` identifies exact commit
`2b793c81e0987f627ab72e3c4e505ae5c6a95abe`, and verify the curated
`SHA256SUMS`, each lane's original `SOURCE-SHA256SUMS`, label inventories,
full CTest, live TLS, stress, install and extracted-ZIP smoke. The expected
counts are 137 concurrency and 2,076 full tests for MSVC, and 194 concurrency
and 2,220 full tests for each of Clang and GCC.

Do not rerun QA-D without a concrete evidence inconsistency or a new source
candidate. If a new candidate does require qualification, use fresh empty
directories outside the repository and run from PowerShell:

```powershell
$expectedCommit = (git rev-parse HEAD).Trim()
concurrency/qa/run-windows-qualification.ps1 `
  -ExpectedCommit $expectedCommit `
  -BuildDirectory C:\crexx-qa\concurrency-build `
  -EvidenceDirectory C:\crexx-qa\concurrency-evidence `
  -BuildJobs 2 -TestJobs 2
```

Verify the same contract, including the Windows process and TLS backend, every
available VM mode, full CTest, stress, install smoke, extracted ZIP smoke,
result identity and digests. If an interrupted run leaves processes holding
artifacts open, stop those processes and start the whole runner again with new
empty directories; do not patch around locks in the source tree.

## 5. Reconcile after initial publication

Return the Linux and Windows evidence to Mac, verify its digests, and reconcile
it with the independent solution review and AC performance result. Produce a
concise decision brief containing:

- exact commit and platform/toolchain/TLS identities;
- SP-01 through SP-09 independent dispositions;
- Mac AC, Linux and Windows results, with test counts;
- install and package results;
- every accepted limitation, environmental invalidation and residual risk;
- any documentation or release-note mismatch; and
- whether the retained initial publication remains supportable or a blocking
  defect requires it to be qualified, repaired or deferred.

Do not change the publication label or describe a platform as passing without
its native runner's `RESULT.txt` and verified digests. Escalate any evidence
that conflicts with the initial-publication choice. End with the exact local
commits and confirm whether anything was pushed.

---

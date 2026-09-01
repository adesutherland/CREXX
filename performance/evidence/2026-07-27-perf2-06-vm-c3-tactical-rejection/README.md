# PERF2-06 VM-C3 tactical rejection

Verdict: reject the exact changed-only numeric-context synchronization shape.
It did not establish a call-heavy product win and moved the zero-call Sieve
guard adversely, which is consistent with the recorded global code-layout debt.
No implementation survives.

## Candidate and boundary

The accepted frame copies all five numeric-context fields from the active
parent and then synchronizes the shared decimal backend. On return it rebinds
the parent context and synchronizes again. The isolated C3R01 patch:

1. always rebound the backend's `num_context` pointer;
2. omitted child-entry synchronization after the exact parent-context copy;
3. compared `digits`, `fuzz`, `form`, `casetype` and `standard` on return; and
4. synchronized only when the effective context or decimal backend differed.

It changed only `interpreter/rxvmintp.c`, preserved canonical RXBIN, public
RXAS, ABI and language behavior, and was discarded after the first Release
verdict. The temporary branch/worktree was removed.

## Provenance and limits

- Candidate base: clean `develop` commit
  `3f11ad72f1f7dc6f0235825cc7df5105553b312a`.
- Baseline binaries: retained accepted-current R0 Release products built from
  `ab9eef54576388121cdb02e285bd99981e68d8b3`; no interpreter source changed
  between that commit and the candidate base.
- Candidate products: ordinary profiling-off Release `rxvm` and `rxbvm` from
  the one-file dirty patch; executable sizes remained 982,392/982,552 bytes,
  equal to R0.
- Host: Apple M5 Mac17,3, Darwin arm64, Apple Clang 21.0.0, CMake 4.3.2 and
  Ninja 1.13.2. Capture ran on AC from
  `2026-07-27T09:54:06Z` to `2026-07-27T09:54:18Z`.
- Inputs: the retained accepted optimized Permute 50, List 100 and Sieve 50
  RXBIN/library products.
- Sampling: one warmup and 12 serial, workload-rotated balanced recorded rounds
  per cell, both VMs, startup included. All 144 recorded executions passed
  their benchmark-native output checks.
- Debug and Release VM targets built. A focused CTest attempt did not reach the
  candidate because the fresh Debug tree lacked compiler, linked-image and
  plugin fixtures. Their exact dependency build was intentionally stopped once
  the Release verdict rejected the candidate. Therefore this bundle makes no
  focused-correctness, full-QA or production-readiness claim.

Product SHA-256:

| Product | SHA-256 |
| --- | --- |
| R0 `rxvm` | `1c8a8c43a8bcd124d7233580a9796ad3241c19ac819b14b5d6e7c5640a44c111` |
| R0 `rxbvm` | `9dbb273166f7f41c762419379d1e44b94b80eb6fff7ce4b5ac8d8f5f1453eb8d` |
| C3R01 `rxvm` | `c8ab5c6b408fe95ecbd7facd8b8878d668c3cd206f0d20a81735dd6e1292bed7` |
| C3R01 `rxbvm` | `3b6864d139dd2f131de8f095056be29d770d194dd1d90c97b22fca038c7ed0b4` |

## Release result

Positive percentages are slower. Every 95% interval crosses zero.

| Workload | VM | Mean | 95% interval | Favorable pairs |
| --- | --- | ---: | ---: | ---: |
| Permute | `rxvm` | -0.384474% | [-1.731124%, +0.962175%] | 7/12 |
| Permute | `rxbvm` | -0.204769% | [-1.263115%, +0.853576%] | 8/12 |
| List | `rxvm` | -0.835582% | [-2.105388%, +0.434225%] | 10/12 |
| List | `rxbvm` | +0.613827% | [-1.252623%, +2.480278%] | 5/12 |
| Sieve | `rxvm` | +1.440812% | [-0.173469%, +3.055094%] | 4/12 |
| Sieve | `rxbvm` | +0.936526% | [-0.250838%, +2.123890%] | 3/12 |

Optimized Sieve has zero bytecode calls, so it cannot execute the intended
child-entry or return saving. Its adverse movement is a zero-work control for
compiler/code-layout drift. The small opposite-direction call-heavy movements
therefore do not justify extending or polishing the candidate.

## Do-not-repeat rule

Do not retry this source shape, add a context-equality flag, or refactor the
return handlers merely to reduce duplicate source/text on the current Apple
product. Reopen numeric/plugin activation only if a materially different plugin
ownership contract changes the synchronization requirement, or if exact
effective-context-change counts plus the supported compiler/architecture
matrix demonstrate a stable ceiling with zero-work drift controls.

Raw rotated samples, benchmark outputs, absolute summaries and the paired
summary are retained under `timing/`. `manifest.txt` records every product and
input path used at capture time.

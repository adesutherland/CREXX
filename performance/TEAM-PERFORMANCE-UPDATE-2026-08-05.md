# Performance update — PERF3-12B closed

PERF3-12B is complete. The selected RXAS optimization now recognizes repeated
equivalent compound stem keys within a safe loop and retains one lazy joined
key for later uses. This is a general value-, signal- and control-flow proof;
it is not a RexxCPS-specific rule and does not add a new instruction or change
the VM or compiler language surface.

## What changed

- RexxCPS removes 1.96 million hot `CONCAT` dispatches at fixed work, with no
  hot setup instruction.
- Its optimized image moves from 1,214 to 1,210 static instructions; `main`
  moves from 369 to 365 and one private cached local raises `.locals` from 103
  to 104.
- The 36-pair route-selection panel measured +3.075% under `rxvm` and +4.275%
  under `rxbvm` at the paired median. The selected production check was also
  favorable and has been accepted.
- The alternative segmented-stem route remains replayable from retained
  evidence, but was not selected because it added about 280,000 hot `LOAD`s
  and was weaker/noisier in the comparison.

## Fresh Apple scorecard

The final ordinary profiling-off Release product completed all 348 formal
scorecard processes: 58 warmups and 290 recorded observations. No timing cell
required an append.

| Comparison | `rxvm` | `rxbvm` |
| --- | ---: | ---: |
| Common-five geometric mean versus ooRexx | 2.375939x | 2.376230x |
| Common-five geometric mean versus decimal NetRexx | 0.852882x | 0.852987x |
| RexxCPS versus ooRexx | 1.172472x | 1.165701x |

RexxCPS reaches 47.203/47.093 MCPS. Richards, Base64 and Towers remain the
main visible deficits. Independent-session movement from the earlier K04e
scorecard is descriptive only; the paired PERF3-12B panels remain the causal
evidence for this optimization.

## Quality and handoff

- Full Debug: 2,039/2,039 tests pass.
- Focused Apple AddressSanitizer: 5/5 pass. LeakSanitizer is unavailable on
  this Apple platform, and that runner limitation is recorded.
- Strict GNU90 checking is warning-free for the changed RXAS surface.
- Disposable S1/H1 PoC worktrees and branches are removed after their replay
  patches and evidence verify; unrelated worktrees are untouched.

Adrian will next integrate the separate per-worker arena plus central block
depot memory restructure. The next performance-programme implementation item
after that is PERF3-12C: transactional PARSE, using the now-proven sparse RXAS
proof infrastructure.

Detailed evidence is in
[`evidence/2026-08-05-perf3-12b-mac-scorecard`](evidence/2026-08-05-perf3-12b-mac-scorecard/).

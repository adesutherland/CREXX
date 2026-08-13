# PERF3-13 Gate E E5 industrial closeout

Date: 2026-08-13

Branch: `mthread`

Source: `a431aaa491fd70170777c64f13b190ac64997646` plus the frozen,
uncommitted industrial E5 implementation, tests and QA initialization repair.

Status: **accepted; Mac QA closeout complete**. Adrian accepted the measured
`rxtvml` slowdown as the expected cost of the selected multithreaded design and
authorized QA closeout. Linux and Windows correctness evidence is accepted;
Linux ARM64 and Windows ARM testing is not required for this closure.

## Selected industrial boundary

E5 now provides a private persistent trusted-worker executor over the E4
sealed-generation boundary. Each worker owns its VM context, register image,
globals, frames and mutable plugin/session state. Requests copy the supported
logical integer/string register-image subset; successful terminal completion
currently publishes the procedure's typed integer result. Live `value *`
storage never crosses workers.

The stable mailbox publishes a correlated request generation and a
level-triggered event word. `CANCEL`, deadline, `KILL` and shutdown are claimed
in deterministic priority order; stale notifications cannot spill into a
later request. POSIX and capable Windows hosts use their native thread
doorbells. A targetable worker without prompt native delivery selects the
separate sparse progress-point owner once, before execution. Non-targetable
and native-capable execution retain the accepted E4 owner.

The sparse owner covers request entry, taken backedges, bytecode call/return
boundaries and native/plugin return. The earlier every-instruction fallback is
rejected. Public worker/channel syntax, a public ABI, shared mutable VM values,
transport semantics and Gate F remain outside E5.

## Accepted ordinary-Release result

The decisive rerun used the profiling-off `Release` `profile-20` baseline and
candidate on a cleared Apple ARM64 host. It ran one warmup and 12 balanced
recorded pairs per cell with 10,000 jobs and 25,000 loop iterations. All 156
processes passed their checksum and concurrency oracle. Positive elapsed
percentages are adverse.

| Engine | Mode | Paired mean | Mean 95% interval | Favourable pairs | 3% guard |
| --- | --- | ---: | ---: | ---: | --- |
| `rxbvml` | direct | -1.539% | -2.285% to -0.792% | 12/12 | clear |
| `rxbvml` | one worker | -2.269% | -2.911% to -1.628% | 11/12 | clear |
| `rxbvml` | two workers | -1.546% | -5.610% to +2.517% | 8/12 | clear |
| `rxtvml` | direct | +4.218% | +1.623% to +6.813% | 1/12 | hit |
| `rxtvml` | one worker | +3.549% | +1.771% to +5.327% | 0/12 | hit |
| `rxtvml` | two workers | +3.013% | +0.194% to +5.832% | 3/12 | hit |

Adrian accepts the `rxtvml` guard hits as an inherent computed-goto/
multithreading cost after the programme examined the available carrier forms.
The sparse alternative is both slower and structurally less desirable, so the
result does not reopen carrier selection. Raw samples, pairwise ratios and the
mechanical summary are retained here.

## QA discovery and repair

The first broad Debug run exposed a real cold-route initialization defect: E5
added private external-mailbox owner and claim callbacks, but the common VM
initializer did not clear them for non-zero-filled stack or embedded contexts.
The resulting indeterminate callback selector caused a systemic burst of 298
unrelated cold-route failures. `rxinimod_common()` now initializes both fields
to null, and the existing poison-storage active-state test asserts that
contract.

The repair makes the intended inactive state explicit and removes undefined
behaviour; it does not alter the selected carrier, mailbox, loop shape or
accepted deterministic benchmark path. The accepted performance campaign was
therefore not repeated after this guard repair.

## Final Mac QA

- Focused normal Debug cold-route reproduction: 4/4 passed after the repair.
- Focused Apple AddressSanitizer: 20/20 passed, including 18 E5 cells and the
  active-state guard; Apple LeakSanitizer is unavailable, so `detect_leaks=0`.
- Complete Debug build: passed.
- Full Debug CTest: 2,055/2,056 passed at `--parallel 30`; the sole
  syntax-highlighter parser-thread startup timeout passed immediately in a
  1/1 serial rerun (1.01 seconds). No E5 or product failure remains.
- Complete profiling-off Release build: passed.
- Focused Release E5/cold-route/signal panel: 22/22 passed.
- Merged `rxc` include/import regression panel: 18/18 passed.
- `git diff --check`: clean.

The failed pre-repair cold-route reproducer and invalid first broad run remain
part of the audit trail; they are not represented as successful verdicts.

## Closure boundary

E5 is complete for the approved industrial and QA scope. Existing Linux and
Windows verdicts are accepted without repetition, and no Linux/Windows ARM
campaign is required. E6, public workers/channels, Gate F, commit and
publication remain separate approval boundaries.

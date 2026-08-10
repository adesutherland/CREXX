# PERF3-05-R5 handler-placement percentage and platform panel

Status: **R5a placement diagnosis complete — code-shape cause, no default selected**

Approved: 2026-08-10

Purpose: replace the single experimental 30% handler-placement choice with an
auditable percentage panel, keep intrinsically host-bound/cold instructions
callable even in the largest practical owner, select the best supported
size/speed compromise on Apple, then validate that provisional selection on
Linux x86-64 and Windows Intel before it becomes the product default.

## Authority and boundary

Adrian approved the full R5 programme on 2026-08-10: add explicit placement
metadata for instructions that should never be inlined; measure 5%, 10%, 15%,
20%, 30% and a practical approximately-90% maximum; select the compromise from
evidence; set that selection as the default; and repeat the performance review
on Linux x86-64 and Windows Intel. Linux additionally receives the full
supported sanitizer validation. ARM platform expansion is explicitly out of
scope after the Apple host because no representative remote ARM host is
currently identified.

This remains an internal C implementation choice. It does not authorize a
public RXAS/RXBIN opcode, ABI, plugin ABI, language, fusion/quickening or
canonical-image change. Do not install, push, merge or publish. Preserve the
literal all-inline equivalence control and the all-outline diagnostic control.
If the evidence-selected candidate hits a governed performance guard, stop for
Adrian's explicit trade-off decision before making it the default.

## Source and evidence identities

- Branch: `codex/perf3-05-r3-handler-codegen-analysis`.
- Worktree:
  `/Users/adrian/CLionProjects/CREXX-perf3-05-r3-handler-codegen-analysis`.
- Starting commit: `3ad9890c139c29e2df44c72d720bec127c3eb65e`.
- Frozen public-handler ranking:
  `performance/evidence/2026-08-09-perf3-05-r2-handler-panel/profile30-inline-names.txt`.
- R3 compiler-specific lowering and report remain the implementation and
  interpretation baseline; do not reopen the rejected common lowering.
- New retained evidence root:
  `performance/evidence/2026-08-10-perf3-05-r5-handler-percentage-panel/`.

## Policy design options

### A — annotate every handler definition invocation

Add a placement-class parameter beside every implementation macro invocation.
This makes the attribute visually local but churns all handler-definition
files and makes code review mix policy movement with semantic handler bodies.
It is not selected for R5.

### B — one central handler attribute ledger

Give every handler identity exactly one tier in `rxvmhandlerpolicy.h`. The tier
records the first profile panel in which the handler becomes eligible, or that
the handler is reserved/sentinel/never-inline. The owner-only `INTERRUPT`
pseudo-op has a separate always-inline tier. A small panel mapping converts
those tiers to `INLINE` or `OUTLINE`. This is selected: it removes repeated
per-panel policy blocks, keeps handler bodies unchanged, and makes omissions,
counts and policy movement mechanically checkable.

### C — infer placement from opcode names, flags or handler implementation text

Infer host-bound status and heat from naming or implementation calls. This
would reduce written metadata but is brittle when implementations evolve and
cannot express a reviewed exception cleanly. It is rejected.

## Stable policy meanings

- `all-inline`: literal equivalence control. Every handler, including the
  never-inline class, expands in the owner exactly as before the framework.
- `all-outline`: diagnostic lower bound. Every handler is callable.
- `profile-5`, `profile-10`, `profile-15`, `profile-20`, `profile-30`: frozen
  public dynamic-frequency prefixes, with both process-private fused handlers
  included from the first profile and the never-inline class overriding heat.
- `max-eligible`: every normal eligible handler is inline; reserved/sentinel
  and never-inline handlers remain callable. It is not an equivalence control
  and must not be described as 100% inline.

The profile labels are nominal public-ranking cut points. The frozen R2
denominator was 588 non-reserved public opcode slots, but one of those slots is
the owner-internal `INTERRUPT` target and has no policy-controlled handler
definition. R5 therefore reports both continuity with that frozen ranking and
the actual 589 policy-controlled non-reserved public-plus-private definitions
(587 public plus two private). The planned public cut points are 29, 59, 88,
118 and 176. The two private handlers make the first four actual totals 31, 61,
90 and 120. The 30% prefix contains three never-inline host operations, so its
actual total is 175 rather than 178. `max-eligible` contains 531/589, or
90.15%. Excluding the two sentinels as well gives the equivalent selectable
normal-handler view of 531/587, or 90.46%.

## Initial never-inline ledger

The class is for operations whose external host/device/process latency normally
dwarfs an interpreter-owner call and whose bodies add disproportionate cold
code or system interfaces to `run()`. It is a stable policy attribute, not a
claim that the operation can never become dynamically frequent. A later
profile may justify an explicit exception, but must change the ledger and
evidence rather than silently allowing a percentage threshold to override it.

The initial 56 non-reserved handlers are:

- 22 socket operations: `SOCKNEW`, `SOCKCLOSE`, `SOCKCONNECT`, `SOCKBIND`,
  `SOCKLISTEN`, `SOCKACCEPT`, `SOCKSHUTDOWN`, `SOCKSEND`, `SOCKSENDB`,
  `SOCKRECV`, `SOCKRECVB`, `SOCKPENDING`, `SOCKTIMEOUT`, `SOCKBLOCKING`,
  `SOCKNODELAY`, `SOCKKEEPALIVE`, `SOCKPEER`, `SOCKLOCAL`, `SOCKSTATUS`,
  `SOCKERROR`, `SOCKSTARTTLS` and `SOCKCONNECTTLS`;
- eight console operations: `SAY_REG`, `SAYX_REG`, `SAYX_STRING`, `SAY_INT`,
  `SAY_FLOAT`, `SAY_STRING`, `SAY_CHAR` and `READLINE_REG`;
- five clock/environment operations: `TIME_REG`, `MTIME_REG`,
  `XTIME_REG_STRING`, `GETENV_REG_REG` and `GETENV_REG_STRING`;
- six process/redirection operations: `SPAWN`, `REDIR2STR`, `REDIR2ARR`,
  `STR2REDIR`, `ARR2REDIR` and `NULLREDIR`;
- 14 file operations: `FOPEN`, `FCLOSE`, `FFLUSH`, `FREADB`, `FREADLINE`,
  `FREADBYTE`, `FREADCDPT`, `FWRITE`, `FWRITEB`, `FWRITEBYTE`, `FWRITECDPT`,
  `FCLEARERR`, `FEOF` and `FERROR`; and
- `METALOADMODULE`.

Reserved opcode slots and `INULL`/`IUNKNOWN` are independently excluded from
`max-eligible`; they are not part of the 56 above. The two sentinels remain in
the historical non-reserved opcode ledger, which is why both denominator views
are disclosed above.

## Execution gates

### Gate 1 — policy implementation and mechanical proof

- [x] Replace repeated per-panel definitions with the single tier ledger and
      panel-to-tier mapping.
- [x] Add all six requested candidate names to CMake while preserving existing
      all-inline/all-outline names.
- [x] Prove every handler definition has exactly one tier and every frozen
      ranked name maps to the intended cut point.
- [x] Prove the 56 never-inline identities and panel totals mechanically.
- [x] Preprocess both engines and prove literal all-inline handler bodies,
      order, label/case map and non-handler owner skeleton are unchanged from
      the R3 starting commit.

### Gate 2 — focused correctness and decisive Apple Release pilot

- [x] Configure and build ordinary profiling-off Release trees for all-inline,
      every candidate panel and all-outline under Apple Clang.
- [x] Pass exact-output governed workload execution for every shape under both
      engines, then pass the 14-test focused dispatch, signal/interrupt,
      late-load, breakpoint, worker and reentrancy suite for 20% under both
      Apple compilers.
- [x] Record owner/text/artifact size, wrapper count and clean/relevant target
      build time.
- [x] Run the smallest balanced Apple Clang Release timing screen containing
      both engines and the governed seven workloads.
- [x] Freeze implementation and report the first verdict before broad
      validation. A correctness failure or governed guard hit stops expansion.

### Gate 3 — formal Apple compiler panel and provisional selection

- [x] Run the balanced twelve-pair formal matrix for eligible Clang panels.
- [x] Repeat the candidate panel under real GCC, preserving the R3 TLS-off
      limitation and compiler-specific handler-call lowering.
- [x] Compare speed, variability, owner/text/file size, build cost and code
      shape. Report `rxtvm` and `rxbvm` separately and keep noisy Base64 visible.
- [ ] Select the smallest panel that is performance-equivalent across the
      supported Apple compiler/engine cells; do not select from inline
      percentage or cache size alone.

Gate 3 stopped at the required trade-off. Clang 20% is guard-clean and improves
all-seven throughput by 3.857%/3.152% for `rxtvm`/`rxbvm`. GCC 20% improves by
3.175%/9.646%, but GCC `rxtvm` Bounce regresses 10.072%. Every requested GCC
non-inline panel fires a Bounce guard. No common percentage can therefore be
selected without Adrian explicitly accepting that regression or approving a
compiler/engine-specific default design. Gates 4-6 remain unopened.

### Gate 4 — Linux x86-64 validation

- [ ] Record host/compiler identity and validate both concrete engines with
      ordinary GCC and Clang Release builds.
- [ ] Run the same correctness and balanced performance comparison between
      literal all-inline and the provisional selection, adding adjacent panels
      only if the selected point is not stable.
- [ ] Run the full supported Debug sanitizer suite through
      `tools/asan-run.sh`, following
      `docs/ai-context/CREXX_ASAN_TESTING.md`, and distinguish unsupported
      sanitizer modes explicitly.

### Gate 5 — Windows Intel validation

- [ ] Record host, CPU, MSVC/compiler and generator identities.
- [ ] Validate `rxbvm`; validate `rxtvm` only if the toolchain supports the
      required labels-as-values implementation.
- [ ] Run the equivalent correctness and balanced performance comparison
      between literal all-inline and the provisional selection, expanding to
      adjacent panels only for a platform-specific reversal.

### Gate 6 — selection, default and closeout

- [ ] Confirm that the selected panel passes correctness and performance guards
      on every reachable required platform/compiler/engine cell.
- [ ] Change `CREXX_VM_HANDLER_PANEL` default only after that confirmation.
- [ ] Run proportionate broad regression, size/build-repeatability checks and
      final source-expansion audit.
- [ ] Update the VM/C compiler optimisation report, live roadmap and retained
      evidence with rejected panels and residual pre-release profiling work.
- [ ] Commit the exact reviewed scope locally; do not push.

## Pre-release residual work

The frozen R2 mix is sufficient to select a coarse code-size/throughput panel,
not to lock the release policy indefinitely. Before Release 1, recapture the
instruction mix from the then-current representative portfolio, include
private/fused dispatch explicitly, audit newly added handlers and re-evaluate
the never-inline ledger. That later profiling refresh must not invalidate the
literal all-inline control or silently change this R5 evidence.

## R5a — effective handler-placement profiling

Adrian selected the focused GCC threaded diagnosis on 2026-08-10 and asked
that VM instruction profiles identify whether each executed instruction used
an inline or outlined handler. This is diagnostic work only: it does not select
a new panel, change the product default, or authorize an RXAS/RXBIN change.

### Design selection

1. **Label each canonical instruction row from its static opcode policy.** This
   is the smallest report-only change, but it is rejected because the VM can
   execute a process-private fused handler while attributing timing/counts to
   the canonical public opcode. The label could therefore be false.
2. **Record the effective placement at the existing instruction-entry hook.**
   Keep canonical opcode counts and timing unchanged, but accumulate whether
   executions used inline, outlined, or both handler placements. Emit that
   value in the existing CSV `value` column and add a table column. This is
   selected: it covers private fusion, needs no schema-column change, and the
   ordinary profiling-off backend still discards the argument without
   evaluating it.
3. **Add a separate 651-handler dynamic census.** This would expose exact
   private-handler identities as well as placement, but duplicates much of the
   canonical instruction table and enlarges the profiling schema. Defer it
   unless effective-placement evidence proves insufficient.

### R5a gates

- [x] Prove ordinary profiling-off profile-20 preprocessing remains unchanged
      for both concrete engines; the hook argument is erased before C parsing.
- [x] Unit-test inline, outline and mixed placement accumulation/reporting.
- [x] Pass the focused profiling, instrumentation, signal and breakpoint tests
      under both concrete engines.
- [x] Capture exact counts-only GCC profile-20 Bounce profiles; derive the
      mechanically all-inline control from the identical deterministic counts
      and literal policy rather than completing a redundant 6.6-GiB compile.
- [x] Identify the dynamic share of outlined instructions and whether private
      fusion causes mixed placement on a canonical opcode.
- [x] Confirm the diagnostic inference with the retained profiling-off Release
      Bounce comparison before proposing a panel change.

R5a rejects the simple missing-hot-handler explanation. Profile-20 executes
887,867,426 Bounce instructions; only 424,204 (0.047778%) are outlined, with
424,200 attributable to `CALL1_REG_FUNC_REG`. That handler and the other two
non-host outlined identities enter at profile-30, leaving only two one-off
`SAY` executions outlined. Profile-30 and max-eligible therefore have the same
effective dynamic placement, yet retained GCC threaded Bounce is -8.691% and
-4.693% respectively. The material difference comes from compiler owner/code
shape created by handlers that Bounce does not execute, not call overhead
proportional to the outlined dynamic mix. No tier or default changes are made.

Evidence:
`performance/evidence/2026-08-10-perf3-05-r5a-handler-placement-profiling/`.

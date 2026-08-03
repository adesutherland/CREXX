# PERF3-11 M06 SSA producer forwarding

Status: **complete — first Release verdict accepted 2026-08-03**

M06 replaces the legacy dense register/view liveness solver for adjacent
producer-destination forwarding with one atomic proof from the existing
storage/component SSA and sparse use service. The producer writes the final
local directly and the following `ICOPY` or `FCOPY` is deleted only when the
whole rewrite plan is proved and then revalidated.

The evidence was produced on branch
`codex/perf3-rxas-flow-infrastructure` from committed M05 base
`be20340f99af`, with only the M06 production, tests, documentation, worklists
and evidence changes dirty. No push was performed.

## Proof boundary

`rxas_flow_prove_producer_destination_forward()` requires an immediately
adjacent, metadata-classified, non-signalling, context-neutral producer with
one exact killed component. The temporary and final storage must be known,
local and unaliased. The producer must not read the final storage through any
register mapping, and the temporary result's sparse direct/dependent use
closure must contain only the typed copy.

TRACE/register metadata, calls, opaque observations, registered asynchronous
handlers and source-address observations fail closed. The immutable plan names
the producer, copy, expected operands and replacement; the consumer validates
all of them before applying a disjoint batch.

Migration exposed one correctness condition absent from the old typed-view
formulation. Scalar producers such as `LOAD` and comparisons clear reference
and native payloads, while typed copies preserve those components. Every
producer-cleared component must therefore already be absent in both the
discarded temporary and final destination. Fresh local entry storage is known
empty; arguments, globals, aliases and unknown or previously populated storage
remain closed. Comparison opcode metadata now describes these cleanup writes.

The old dense M06 live-in/live-out solver is deleted. That also retires M08's
remaining semantic liveness/availability/may-reach authority. Raw use/kill and
taint bitsets remain because storage diagnostics, analysis bounds and legacy
keyhole consumers still read them.

## Decision replay

The frozen M05 assembler has SHA-256 `fc6def29...` and version
`crexx-1.0.0-beta.3+local.gbe20340f99af`. At the M05 boundary it accepts
exactly eleven current M06 cases: eight in `nr18_flow_harvest`, two in
`whole_procedure_flow` and one in `whole_procedure_panel`. M06 recovers all
eleven with reason `producer-destination-forwarded-ssa`. The historical
twelfth case, `copy_before_endlife`, remains valid but is now transformed by
the stronger M05 use proof before M06 runs.

The focused negatives retain temporary use, signalling or inexact producers,
view mismatch, destination reads, source address observation, mixed-entry
storage, handler observation and hidden cleanup. Two explicit cleanup fixtures
require `cleanup-required` for the discarded temporary and final destination.

## Accepted Release verdict

Frozen M05 and current M06 produce byte-identical RXBIN for the three focused
fixtures and canonical Richards, Towers and RexxCPS. The representative
canonical inputs contain zero M06 accepts, so this is an assembler-scale and
correctness verdict, not a runtime-throughput claim.

Three serial paired RexxCPS assembly rounds show equal 0.18 s medians. Median
peak RSS changes from 103,022,592 to 104,103,936 bytes: +1,081,344 bytes,
or +1.05%. Adrian accepted that bounded first Release verdict on 2026-08-03.

## Correctness closeout

- strict GNU90 syntax passes with one pre-existing unused-parameter warning;
- focused metadata, graph and optimized/no-opt flow, panel and NR-18 checks
  pass 8/8 in Debug and Release;
- all eleven current accepts and their retained negative floor are replayed;
- frozen/current focused and canonical RXBIN hashes are exact;
- the complete Debug build passes; and
- broad Debug passes **1,995/1,995** in 291.22 seconds.

The first broad run exposed one stale NR-09 test oracle: it expected a
withdrawn `load`/`icopy` pair to remain expanded, while M06 correctly emits a
direct-destination `load`. The oracle was updated to require the intended M06
form and still forbid the typed copy. Its focused rerun and the complete broad
rerun pass; no production change was made.

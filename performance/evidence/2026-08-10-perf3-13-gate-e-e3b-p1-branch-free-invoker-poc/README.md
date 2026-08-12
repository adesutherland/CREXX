# PERF3-13 Gate E E3b-P1 branch-free invoker proof

Date: 2026-08-10

Branch: `develop`

Source checkpoint: accepted E3a commit
`29ef1975ec0190bdd1b246a76211f727fa720dce`; the worktree also contains the
unaccepted E3b-P1 implementation/evidence and this isolated proof.

Status: **machine-level ceiling passed; a production candidate is recommended
but not authorized by this proof**.

## Question and exact control

The two integrated E3b-P1 call shapes were clearly adverse by 14-20% because
ordinary native call sites selected direct versus locked policy on every call.
This isolated proof asks whether policy can instead be selected when a native
procedure is loaded and represented by one bound invoker pointer.

All cells use the same profiling-off Release executable and native adapter.
`raw-direct` calls `rxvm_callfunc_direct` with a direct `bl`.
`selected-direct` loads a preselected adapter pointer and uses `blr` without a
capability branch. The branch and locked rows are diagnostic controls.

## Reserved-host comparison

One warmup and 12 pairwise-balanced serial recorded rounds ran for all five
cells, with 20 million calls per process. All 65/65 processes passed. No sample
was removed, and no cell met the formal runner's rerun criterion.

Positive elapsed percentages are adverse.

| Candidate versus raw direct | Paired mean | Mean 95% interval | Paired median | Median ns/call increment | Result | 3% guard |
| --- | ---: | ---: | ---: | ---: | --- | --- |
| Selected direct | -0.489662% | -1.228728% to +0.249404% | -0.495378% | -0.168400 | inconclusive | clear |
| Branch direct diagnostic | -0.543659% | -1.190405% to +0.103086% | -0.452580% | -0.278200 | inconclusive | clear |
| Selected locked | +20.117255% | +19.152551% to +21.081960% | +20.212079% | +8.129975 | clear adverse | hit |
| Branch locked diagnostic | +19.389823% | +18.303932% to +20.475713% | +19.435890% | +7.970450 | clear adverse | hit |

The selected direct path passes the machine-level ceiling. Its interval crosses
zero, its point estimates have no adverse tendency, and it remains comfortably
inside the 3% guard. The selected and raw loop owners are each 72 bytes on
Apple ARM64. The selected loop contains two pointer loads and one `blr`; it has
no capability test.

The locked result is not a rejection of the compatibility contract. It proves
that the lock should be paid only when concurrency requires it: legacy calls
remain direct with one legacy-capable executor, and a cold process-wide
quiescent transition changes legacy bindings to locked only before a second
legacy-capable executor is published. Reentrant bindings remain permanently
direct.

The neutral isolated branch control does not supersede the rejected integrated
VM evidence. The earlier bytecode handler/layout candidates were clearly
adverse by 14-15% even after the wrapper frame was removed. This proof isolates
one invocation primitive and does not model those call-site layout effects.

## Decision boundary

The evidence supports proposing a production candidate containing:

1. a preselected native invoker in each runtime procedure;
2. permanent direct binding for `PROCESS_REENTRANT` procedures;
3. direct legacy binding while only one legacy-capable executor exists; and
4. one sticky, process-wide, quiescent rebind to the recursive locked adapter
   before a second legacy-capable executor or equivalent late load is published.

Production integration remains a separate approval. It must prove lifecycle
registration, quiescence/drain, late load, teardown, recursive serialization
and unchanged reentrant bindings, then take the mandatory first ordinary
profiling-off Release verdict under both VM engines. P2 session factories and
Gate F remain closed.

## Interpretation boundary

This is a same-host isolated machine-level comparison, not a product Release
verdict or a release claim. Process elapsed includes startup but is dominated
by 20 million adapter calls through the same binary. Paired percentages are
measured observations; the proposed integrated coordinator is a design
recommendation supported by the ceiling and the separate state-machine
correctness proof.

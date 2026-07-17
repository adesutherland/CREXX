# NR-09 bounded sequence-ledger PoC

Status: frozen deterministic input; production work is queued next after the
completed NR-08 closeout. The scope is to implement obvious exact candidates,
record the remainder, and compare synthesized instructions and procedure-owned
defaults, starting with numeric-context setup.

This bundle retains revision-separated RXSEQ evidence and the deterministic
Level B decision ledger built from it. It is a design/PoC artifact, not a
production fusion verdict.

Inputs:

- historical NR-05 schema-4 portfolio: 14,009 rows at revision
  `nr05-6a064499f327-rxseq-schema4`;
- current RexxCPS 2.2d baseline: 5,306 rows across no-opt and opt;
- frozen NR-08 candidate: 5,005 rows across no-opt and opt;
- authoritative opcode effects from `binutils/include/rxopeffects.h`.

`capture_current_rexxcps.zsh` reproduces the minimal current N=2/3/4 RXSEQ
capture. `build_and_run_ledger.zsh` compiles the maintained
`performance/tools/build_sequence_ledger.crexx` tool, processes both retained
CSV inputs on `rxbvm` and `rxvm`, proves byte-identical output, checks row and
stable-ID uniqueness counts, and reconciles a representative source location
and NR-08-subsumed family.

Retained outputs are under `retained-rxbvm/` and `retained-rxvm/`. Each contains
the 11,332-row machine ledger, concise decision view, exact input state and
output checksums. The two directories are byte-identical.

The ledger preserves counts, workloads, entries, modes, site observations and
module observations as separate facts. Site/module totals are bounded
per-entry observations, not a guessed cross-image union. Mechanical `unsafe`
and `candidate` labels remain review queues; no row authorizes a lowering.

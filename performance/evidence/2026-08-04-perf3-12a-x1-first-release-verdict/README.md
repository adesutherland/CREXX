# PERF3-12A X1 copied-XTOY first Release verdict

Status: **accepted; combined correctness closeout complete**

## Scope and provenance

- Cursorless baseline: `afc0b274f588` (`perf: replace RXAS value cursors
  with explicit slices`). These artifacts retain the exact pre-acceptance X1
  verdict; Adrian subsequently accepted it and authorized combined closeout.
- Ordinary product: `Release`, `-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=OFF`. Counts product: the corresponding Release build
  with `CREXX_VM_PROFILING=ON`.
- Retained cursorless fixed-work baseline: optimized RexxCPS at
  53,659,088/53,659,041 instructions under `rxvm`/`rxbvm`, with 2,220,000
  `DCOPY`, 2,220,000 `DTOS`, and 97,680,000 decimal-copy bytes.
- Candidate no-opt image SHA-256:
  `6a3d8844d60f28c4c60315031b7e8116fb344af10f6ac4422359d508c138d223`;
  it is byte-identical to the cursorless baseline. Candidate optimized image:
  `92b922eec6f7ad63f3b69f2cdbf3734db0010c6ce5033285ccbf79f9bb93ea38`.
  Linked library:
  `84808f0c15fa3e0cdcdd70750abd38559cd3943ca61e71ba8aafc06e6aa86163`,
  also byte-identical to the cursorless baseline.
- Host: Apple M5, Darwin 25.5.0 arm64, 10 logical CPUs.

## Proof and correctness

X1 is a distinct capability-lazy SSA/use route. Its immutable proof plan
requires an adjacent exact typed copy and one-register XTOY derivation,
distinct local unaliased storage, the expected component ValueIds, no
observation of the displaced source result or copied temporary input, and a
redirectable result-use set. The queue consumer first validates and pins the
complete plan, then atomically retargets the derivation, redirects consumers,
deletes matching derived-value TRACE events and deletes the typed copy.

The focused fixture proves the positive `DCOPY`/`DTOS` rewrite and rejects
source-result and temporary-input observations, later writes, read/write uses,
context-separated pairs, branch/phi uses, linked storage, call windows, local
signal continuations, register metadata and input TRACE. Matching
derived-result TRACE is deleted atomically under the documented optimized
trace policy. Narrowed decimal and native-stem component metadata is checked
against the VM handlers and dedicated metadata tests.

Combined closeout found and fixed one additional fail-closed boundary. A
reference object created by `MKREF` observes its target storage without adding
another live register mapping. X1 now rejects placement when a reference to
the source or temporary storage can reach the candidate. The original
PERF2-07 representation oracle and a dedicated `reference-observed` optimizer
negative pass with the guard enabled; assembling the oracle with `rxas -n`
remains an independent semantic control.

The retained PERF3 Debug tree passes all **77/77 RXAS optimizer checks** plus
the immutable flow-graph contract (**78/78 total**). The new X1 runtime oracle
passes opt/no-opt under both VMs. The ordinary Release RexxCPS smoke cells pass
in all four VM/mode combinations with zero stderr. Sieve and the copy/string-
heavy Base64 roundtrip guard pass opt/no-opt under both VMs, also with zero
stderr. After acceptance, the unqualified 30-way Debug closeout passed 2,033
of 2,034 tests; only `rxpa_signature_diagnostics` reached its 120-second limit
under host load, then passed alone in 12.11 seconds. The complete functional
set is therefore 2,034/2,034 with the concurrency timeout retained rather than
silently discarded.

The final ordinary profiling-off Release tree was deleted and rebuilt cleanly
through all 1,585 build steps. A 15-test Release closeout selector then passed
the queue opt/no-opt regression pair, injected allocation-failure value tests,
string and binary slice tests under both VMs, the PERF2-07 representation
oracle, both X1 optimizer modes and all four X1 runtime cells.

## Exact fixed-work result

| VM | Mode | Cursorless | X1 | Change | Change |
| --- | --- | ---: | ---: | ---: | ---: |
| `rxvm` | no-opt | 143,099,442 | 143,099,442 | 0 | 0.000000% |
| `rxbvm` | no-opt | 143,099,442 | 143,099,414 | -28 | -0.000020% |
| `rxvm` | optimized | 53,659,088 | 52,839,051 | -820,037 | -1.528235% |
| `rxbvm` | optimized | 53,659,041 | 52,839,051 | -819,990 | -1.528149% |

Both optimized VMs execute exactly 1,400,000 `DCOPY` and 2,220,000 `DTOS`.
X1 therefore removes exactly **820,000 decimal-copy dispatches**, while every
conversion remains at its original program point. The value-operation domain
independently falls from 2,220,000 copies/97,680,000 bytes to 1,400,000
copies/61,600,000 bytes: 820,000 copies and 36,080,000 bytes removed.

The generated optimized RXAS contains five adjacent `DCOPY`/`DTOS` sites. X1
proves and removes two, so disassembly contains three `DCOPY` and all five
`DTOS`. The main-procedure diagnostic reports two applied component placements;
the retained cases fail closed for source-result or storage-change facts.

The residual optimized deltas beyond the exact 820,000 copies are only
low-frequency startup/final-path instructions: -37 under `rxvm` and +10 under
`rxbvm`. The no-opt image and linked library are unchanged; the 28-instruction
`rxbvm` no-opt movement is likewise confined to low-frequency runtime paths.
Both profiles report result zero, the expected PASS marker, zero invalid
events, zero counter overflow and complete tracked domains.

## Assembly scale and recommendation

An ordinary Release reassembly of the full optimized RexxCPS source completes
in 0.51 seconds with 134,660,096 bytes maximum RSS. The diagnostic run
completes in 0.99 seconds with 135,659,520 bytes maximum RSS. This is well
inside the accepted sparse boundary and confirms the removed signal-policy
expansion has not returned.

These are deterministic instruction counts, not a replacement Mac wall-clock
scorecard. The remote-terminal host disturbance remains, and the wider paired
panel stays queued as a later clean-host refresh. Under the accepted battery
evidence boundary, X1 removes the exact proved work, materially reduces
optimized fixed-work instructions, leaves `DTOS` intact and keeps both VMs in
agreement. Adrian accepted that verdict and the combined correctness closeout;
no push is part of this evidence update.

Cursorless RXAS/RXBIN is an intentional instruction-set compatibility break.
Do not reuse retained build/worktree bytecode with this product: delete it or
perform a clean rebuild from source before running or packaging the tree.

## Command shapes

```sh
cmake --build cmake-build-debug-perf3-12a \
  --target rxas test_rxas_flow_graph test_rxop_metadata \
  copied_xtoy_component_placement_runtime --parallel 10

ctest --test-dir cmake-build-debug-perf3-12a \
  -R '^(rxas_flow_graph_contract|rxas_optimizer_)' \
  --output-on-failure --parallel 10

cmake --build cmake-build-release-perf3-12a \
  --target rxas rxvm rxbvm benchmark_rexxcps_levelb_opt_artifact \
  benchmark_rexxcps_levelb_noopt_artifact --parallel 10

cmake-build-profile-perf3-12a/bin/<vm> --profile=counts \
  --profile-output <profile.csv> \
  cmake-build-profile-perf3-12a/tests/benchmarks/benchmark_rexxcps_levelb_<mode>.rxbin \
  cmake-build-profile-perf3-12a/bin/library.rxbin -a --smoke-count 200
```

## Evidence map

- `analysis/count-summary.csv`: exact retained/candidate totals and target
  value-operation reductions.
- `analysis/rexxcps-proof-and-scale.txt`: bounded proof decisions and assembly
  time/memory observations.
- `profiles/{rxvm,rxbvm}`: raw schema-5 count profiles plus stdout/stderr.
- `release-smoke/{rxvm,rxbvm}`: profiling-off RexxCPS correctness cells.
- `guards/{rxvm,rxbvm}`: Sieve and Base64 opt/no-opt correctness cells.
- `validation/`: the 78-test focused selector, four-cell X1 runtime oracle,
  final broad Debug result with isolated timeout confirmation, and clean
  Release rebuild/15-test closeout smoke.
- `checksums.sha256`: recursive evidence closure excluding itself.

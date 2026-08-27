# Level L inline receiver-storage first Release verdict

## Accepted verdict

Adrian accepted the first profiling-off Release verdict on 2026-08-27 as the
baseline checkpoint for the isolated `temp/level-l-inline-storage` branch.
This is not a merge or release claim; later staging into the hotfix branch is
deferred while that branch is occupied.

The candidate extends cross-file inlining without changing the VM or RXAS
instruction set. I7 metadata is emitted only for bodies that need one of these
new structural proofs:

- a read-only exact concrete object attribute used as a direct member receiver;
- direct one-dimensional indexed scalar or exact non-reference object arrays;
- an ordinary child `.binary` borrowed only by an immediate `<at..type>` or
  `<packed..type>` access; or
- an exact residual concrete member-method dependency.

I6 remains unchanged for its existing portable slice. References, whole-array
replacement, binary escape, interface/dynamic dispatch, imported factories and
computed receivers retain ordinary calls.

## Correctness and RXAS shape

The focused regression covers source and binary imports, optimized and `-n`
controls, RXDAS round-trip and both `rxtvm` and `rxbvm`. Existing cross-file,
scalar-accessor and packed-accessor focused regressions also pass.

For the timing workload, the untouched-develop compiler retains the outer
`currentValue()`, `classAt()` and `byteAt()` calls. The candidate removes all
three outer calls while deliberately retaining the inner
`token.runtimeValue()` call. Every timing execution emitted the exact checksum
`PASS: inline storage 315000000`.

## Performance verdict

The maintained Level B matrix runner recorded two warmups and twelve balanced,
interleaved rounds per cell. Positive percentages below mean lower elapsed time.

| VM | Develop median | Candidate median | Paired Q1 | Paired median | Paired Q3 | Mean 95% interval | Favourable |
|---|---:|---:|---:|---:|---:|---:|---:|
| `rxtvm` | 1.856772 s | 0.641792 s | 65.240% | 65.416% | 65.679% | 65.216% to 65.623% | 12/12 |
| `rxbvm` | 1.847954 s | 0.657352 s | 64.246% | 64.471% | 64.633% | 64.230% to 64.584% | 12/12 |

No absolute cell crossed the 3% relative-MAD or 10% span rerun threshold.
There were zero correctness failures across the 56 warmup and recorded
executions.

## Evidence map

- `manifest.txt`: exact four-cell workload and command matrix.
- `timing/samples.csv`: all raw warmup and recorded samples.
- `timing/outputs.csv`: retained correctness output for every execution.
- `timing/summary.csv`: maintained absolute distribution summary.
- `paired-summary.csv`: paired distributions, favourable counts and intervals.
- `baseline/` and `candidate/`: generated RXAS audit images; RXBIN files are
  intentionally ignored by the repository.
- `PROVENANCE.md`: source, build, host, artifact identities and replay command.

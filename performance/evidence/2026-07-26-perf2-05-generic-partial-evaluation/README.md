# PERF2-05 generic constant-call evaluation

Date: 2026-07-26

This bundle closes the accepted P05-CF1 production slice. The compiler now
partially evaluates proved constant Level B callable bodies from their actual
instructions instead of selecting a hard-coded BIF name. `LENGTH`, `SUBSTR`
and `WORD` are the plumbing, composition and bounded-control-flow proofs; an
equivalent unregistered user `WORD` function is the decisive acceptance cell.

The result is deliberately general but bounded. Local and source-import bodies
are evaluated from their exact resolved AST. Binary-import bodies require the
existing transported inlining template/proof. Reference or exposed formals,
procedure-level `EXPOSE`, unsupported result types/statements/instructions,
reached signals, unproved scan outcomes and exhausted budgets retain the
ordinary call. No callable-name registry, public RXAS instruction, RXBIN/ABI
change, VM handler or legacy cursor-semantic change is installed.

## Accepted verdict

The final reviewed compiler emits exactly the RXAS and, when assembled from
the same input pathname, exactly the RXBIN used by the accepted first Release
verdict. The two-million-call ordinary user-body cell changes as follows:

| Measure | Accepted compiler | P05-CF1 | Change |
| --- | ---: | ---: | ---: |
| executable RXAS instructions | 62 | 17 | -45 (-72.58%) |
| peak locals | 15 | 4 | -11 (-73.33%) |
| RXAS bytes | 10,438 | 2,674 | -7,764 (-74.38%) |
| RXBIN bytes | 5,207 | 2,311 | -2,896 (-55.62%) |
| `rxvm` median, 22 balanced pairs | 184.5295 ms | 7.6735 ms | -95.868% |
| `rxbvm` median, 22 balanced pairs | 199.8640 ms | 9.5755 ms | -95.128% |

All 92 decisive samples passed and every pair favored P05-CF1. The unchanged
RexxCPS image was neutral at the governed 36-pair cap on each VM, as expected
for a slice whose current product gain is in general constant user/future
functions rather than an additional RexxCPS fold.

Final correctness is 1,920/1,920 Debug CTests, 6/6 focused Release CTests and
the Release opcode-metadata audit. Adrian accepted the first Release verdict
before broad closeout.

## Evidence map

- [`CLOSEOUT.md`](CLOSEOUT.md) records the reviewed implementation, correctness
  and accepted Release verdict.
- [`PROVENANCE.md`](PROVENANCE.md) records repository, host, build and exact
  product/artifact identities.
- `generated/` retains the Level B decision source, exact baseline/candidate
  RXAS and RXBIN, the timing matrix and the generated-artifact comparison.
- `measurements/decisive/` retains both raw sample blocks, their consolidated
  summary and the 22-pair verdict.
- `measurements/rexxcps-guard/` retains all three raw guard blocks, their
  consolidated summary and the final 36-pair neutral verdict.
- `correctness/` retains the final complete Debug and focused Release CTest
  logs.
- `state/` retains the timing pre/post state captures.

The recursive `checksums.sha256` is generated and replayed with the maintained
Level B performance inventory tool after the package is finalized.

P05-CF1 is complete. PERF2-05 remains open for separately selected,
profile-driven semantic instruction/placement work; this slice does not
pre-approve a public opcode or a change to observable cursor effects.

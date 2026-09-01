# PERF2-04 P04-SLC1 first ordinary Release verdict

P04-SLC1 is a frozen provisional general certified-call constant-evaluation
slice. The certification universe is deterministic, non-I/O, non-random core
Level B, but only exact per-callable entries are enabled. The initial entries
are constant `rxfnsb.upper` and the proved positive, supplied-length,
fully-in-range `rxfnsb.substr` domain. The Level B bodies remain the complete
fallback and behavior documentation.

## Qualification and product identity

- Focused Debug: the three PERF2-04 certificate/effect/import tests, four
  SUBSTR test paths, two retained PERF2-03 reference-accessor guards and the
  RexxCPS smoke pass.
- A stale generated `rxpp_linked.rxbin` reproduced the old exposed `UPPER`
  declaration during an incremental rebuild. `rxpp` now removes only its own
  RXBIN byproducts before scanning its output directory; the exact stale-image
  reproduction then rebuilds successfully.
- Ordinary Release `rxas`, `rxlink`, `rxvm`, `rxbvm` and the 862,096-byte
  `library.rxbin` are byte-identical to the accepted P04-CAS1 snapshot.
- Measured candidate `rxc` SHA-256:
  `147e326b47b8f747934d5ce5c9bdd804ce943274ace0e690c2d0ba27fb5ed503`.
- Post-verdict audit `rxc` SHA-256:
  `90bd623db7051a34b44b0dbfdc945132c66ba34ccedc0f9cadb4ed0507953e54`.
  The audit fixes rejected-domain temporary ownership and bounds its index
  conversion. The generated RexxCPS RXAS/RXBIN below are byte-identical to the
  measured candidate, so the recorded runtime product and verdict are unchanged.
- Candidate RexxCPS RXAS/RXBIN SHA-256:
  `0156af796d8bffbe91668565cbdaaeb0c527334b60e67980d37f8f8207218fa4`
  and
  `eb8ac9690ab77f43ba88333fd27f5f1061b43932caa7894b753fe9d8edd1a072`.

## Exact generated-machine result

| Metric | Accepted P04-CAS1 | P04-SLC1 | Delta |
| --- | ---: | ---: | ---: |
| whole-module executable RXAS | 1,489 | 1,387 | -102 |
| `main` executable RXAS | 572 | 470 | -102 |
| `main` locals | 105 | 100 | -5 |
| `strupper` operations | 4 | 0 | -4 |
| `substring` operations | 5 | 3 | -2 selected constant slices |
| RXAS bytes | 223,849 | 204,309 | -19,540 |
| RXBIN bytes | 78,162 | 72,106 | -6,056 |

The two selected constant `SUBSTR` bodies and all four constant `UPPER` scans
disappear. Three unrelated dynamic `substring` operations remain. This reaches
the exact selected-site runtime-work ceiling without an opcode, RXBIN, ABI, VM
or native change.

## Profiling-off Release wall verdict

The existing maintained Level B matrix runner executed each VM serially with
two warmups and seven recorded samples. All four warmups and all 14 recorded
executions exit zero, emit the required PASS and retain no stderr.

| VM | Accepted CAS1 median | P04-SLC1 median | Increment | Relative MAD | Rerun |
| --- | ---: | ---: | ---: | ---: | --- |
| `rxvm` | 31,658,995 | 36,229,324 | +14.436115% | 0.863585% | no |
| `rxbvm` | 29,725,073 | 33,904,454 | +14.060120% | 0.077412% | no |

The accepted CAS1 baseline is retained valid evidence rather than a same-matrix
rerun. The large dual-VM movement, exact instruction/scan removal and the fact
that every candidate recorded sample exceeds the corresponding accepted CAS1
maximum make the first verdict unambiguously favorable.

## Mandatory stop

Recommend accepting P04-SLC1 and retaining the general certificate registry
with the two initial entries. Stop here for Adrian's decision. Do not begin
P04-WRD1, broad QA, closeout, commit or push until this verdict is accepted.

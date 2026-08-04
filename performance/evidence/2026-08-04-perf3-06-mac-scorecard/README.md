# PERF3-06 qualified-deficit closure and Mac scorecard

This compact evidence bundle captures the accepted PERF3 product at clean
detached commit `5fbe36049e26ee73ea0cf1720a7fc416f33d0fe2` on the Apple M5
host.  It is a formal same-session absolute throughput scorecard, not an
unmatched before/after regression claim.

## Outcome

- ordinary profiling-off Release timing: `348/348` initial executions plus
  `30/30` governed-append executions pass;
- common-five geometric means: `2.453066x/2.285744x` versus ooRexx and
  `0.912280x/0.850054x` versus decimal NetRexx for `rxvm`/`rxbvm`;
- both cREXX VMs now clear the 2.00x ooRexx aggregate band in this scorecard;
- Richards remains the largest qualified common deficit and Base64 remains a
  noisy deficit;
- RexxCPS reaches ooRexx parity at `1.151301x/1.133307x`, but remains below
  its separate 1.50x band;
- Towers records `0.390842x/0.389933x` versus ooRexx but remains a large
  qualified separate object/allocation deficit; and
- zero samples were removed; ooRexx Bounce and both cREXX Base64 cells remain
  noise-labelled after the one permitted append.

See [`scorecard.md`](scorecard.md) for the full ratios, target gaps, static
instruction movement and interpretation boundary.

## Evidence map

- `timing/initial/`: 58 warmups plus 290 recorded observations, raw output and
  initial summaries;
- `timing/append/`: the mechanically selected 30 recorded observations;
- `timing/`: merged authoritative summary, ratio and geometric-mean tables;
- `static/`: exact PERF3-01/PERF3-06 static instruction/image summaries and
  the compact delta ledger;
- `artifacts.csv`: 79 exact product, source, comparator, generated-product,
  manifest/tool and predecessor-authority identities;
- `manifests/`: immutable formal and governed-append schedules;
- `provenance/`: exact pre/post source, build, host, power, load and campaign
  state;
- `VALIDATION.md`: focused correctness, sample, identity, summary and static
  cross-check results;
- `logs/`: compact configure/build/correctness/tool result logs; and
- `checksums.sha256`: recursive closure excluding the checksum file itself.

## Claim boundary

The formal rows use unchanged qualified sources, work and comparator products.
The retained PERF3-01 absolute scorecard remains historical context, while the
accepted paired candidate bundles remain causal evidence.  No lifecycle/RSS
campaign, profiling run, platform rerun, default-VM decision, architecture
selection, production edit or push is included here.

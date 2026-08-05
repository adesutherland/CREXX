# PERF3-12B current-product Mac scorecard

This bundle refreshes the formal Apple M5 absolute throughput scorecard for
clean merged product `44d8b6a7ecd7800979b5db992c14bc7182aa89dd`.  That
product contains the accepted PERF3-12B H1 implementation and the two upstream
`PUSH`/`PULL`/`QUEUE` compiler-exit commits present on `origin/develop` before
the campaign.  It is a same-session absolute scorecard, not an unmatched
before/after regression claim.

The final closeout adds only documentation and an unsigned type correction to
the RXAS capability-mask constant. A clean Release rebuild produces the exact
same `rxas` SHA-256 before and after that correction, so the measured product
and emitted benchmark images remain authoritative for the published closeout.

## Outcome

- ordinary profiling-off Release timing passes `348/348` executions: 58
  warmups plus 290 recorded observations;
- no cell triggers the relative-MAD/min-max-span rule, so no append is run and
  no sample is removed;
- common-five geometric means are `2.375939x/2.376230x` versus ooRexx and
  `0.852882x/0.852987x` versus decimal NetRexx for `rxvm`/`rxbvm`;
- both VMs remain above the `2.00x` ooRexx aggregate band, while Richards and
  Base64 remain below ooRexx parity;
- RexxCPS reaches `47.203/47.093` MCPS and is `1.172472x/1.165701x` ooRexx:
  above parity but below the separate `1.50x` band;
- Towers remains a separately qualified deficit at
  `0.399148x/0.400611x`; and
- current RexxCPS contains 1,210 static instructions and `.locals=104`, exactly
  matching the selected B4 H1 route's static count and local allocation.

See `scorecard.md` for exact medians, ratios and interpretation boundaries and
`VALIDATION.md` for product, sample, noise, static and correctness checks.

## Evidence map

- `timing/initial/` retains all 58 warmups, 290 recorded observations and
  process output;
- `timing/` retains the authoritative summary, ratios, geometric means and
  descriptive comparison with the earlier K04e session;
- `static/` retains exact seven-workload/current-library sizes and instruction
  counts plus the K04e-to-current delta ledger;
- `artifacts.csv` SHA-256-binds 84 product, source, comparator, generated,
  manifest, tool and predecessor-authority identities;
- `identities/` retains clean product and source checksums;
- `provenance/` records host, product, power and process state before and after;
- `logs/` retains clean Release configure/build, focused correctness, full
  2,039-test Debug QA, strict GNU90 and focused AddressSanitizer results;
  and
- `checksums.sha256` recursively closes the bundle except itself.

## Claim boundary

The unchanged qualified sources, comparator products and work definitions make
this the current absolute Apple scorecard.  Movement from the independently
captured K04e session is descriptive only; its product, comparator scheduling
and host session differ.  The B4 36-pair and B5 first-verdict panels remain the
causal performance evidence for H1.  No lifecycle/RSS campaign, non-Apple
platform rerun, default-VM decision or memory-manager change is included.

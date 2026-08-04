# PERF3-11 K04e current-product Mac scorecard

This bundle refreshes the formal Apple M5 absolute throughput scorecard for
clean detached accepted product `c4470635048e497417c4db92c03ecbcd79eaa750`.
It was requested after the host's remote-terminal load was removed. It is a
same-session absolute scorecard, not an unmatched before/after regression
claim.

## Outcome

- ordinary profiling-off Release timing passes `348/348` initial executions
  plus `30/30` governed-append executions;
- common-five geometric means are `2.465740x/2.316900x` versus ooRexx and
  `0.894608x/0.840606x` versus decimal NetRexx for `rxvm`/`rxbvm`;
- both VMs remain above the `2.00x` ooRexx aggregate band, while Richards and
  Base64 remain below ooRexx parity;
- RexxCPS is `1.158075x/1.130694x` ooRexx and therefore above parity but below
  its separate `1.50x` band;
- Towers remains a separate qualified deficit at `0.393804x/0.389391x`;
- zero samples were removed and no second append was taken; `permute-netrexx`
  and both cREXX Base64 cells remain noise-labelled; and
- static images move only downward versus PERF3-06. K04e is the final exact
  RexxCPS `1,222 -> 1,221` instruction change; the other deltas are accepted
  intervening work and are not attributed to K04e.

See `scorecard.md` for exact medians, ratios and interpretation boundaries and
`VALIDATION.md` for product, sample, noise, static and correctness checks.

## Evidence map

- `timing/initial/`: 58 warmups and 290 recorded observations with raw output;
- `timing/append/`: 30 mechanically selected recorded observations;
- `timing/`: merged authoritative summary, ratios and geometric means;
- `static/`: exact seven-workload current summary and PERF3-06 delta ledger;
- `artifacts.csv`: 79 SHA-256-bound product, source, comparator, generated,
  manifest, tool and predecessor-authority identities;
- `identities/`: clean product and source SHA-256 records;
- `provenance/`: host, build, power and campaign state;
- `logs/`: compact correctness and tool-result records; and
- `checksums.sha256`: recursive closure excluding itself.

## Claim boundary

The unchanged qualified sources, comparator products and work definitions make
this the current absolute Apple scorecard. Descriptive movement from PERF3-06
is unmatched-session context only. The accepted 36-pair K04e verdict remains
the causal evidence for K04e. No lifecycle/RSS campaign, platform rerun,
default-VM decision, production edit or push is included.

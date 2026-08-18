# DECIMAL-01 Gate 1 current-provider baseline

Date: 2026-08-18

Status: **retained L1 adapter and L2/L3 product blocks complete; prepared
optimizer/RexxCPS guard manifests not executed**

Source: clean detached `0fe5618eb858758d5f01da9dc65331d677605565`

The main working tree contains only closeout documentation/manifests/evidence
changes. Production binaries, benchmark sources and the maintained Level B
matrix runner are from the exact clean source commit above.

## Boundary

- ordinary profiling-off Release, profile-20;
- Apple M5, Apple Clang, AC power, low-power mode off;
- D0s static/default `mc_decimal`, D0d dynamic `mc_decimal`, D1a unrestricted
  Common-18 `db_decimal` diagnostic and D1b qualified Classic-9 `db_decimal`;
- L1 adapter and L2/L3 optimized product cells. Optimizer-isolation,
  decimal-string attribution and canonical RexxCPS guard manifests were
  prepared but were not part of this retained capture;
- formal absolute cells use two warmups and ten serial rotated observations;
  and
- calibration values are discarded.

The manifests are retained under `performance/manifests/decimal-01-gate1-*`.
Raw captures and summaries are in the subdirectories named by measurement
block. No aggregate is defined. The initial L1 conversion calibration at
6,000,000 iterations put the fastest cells below one second. Those four raw
rows remain retained but invalid for formal interpretation; the separately
named v2 conversion manifest uses 20,000,000 iterations. All other L1 v1 modes
clear the calibration floor.

## Result

All retained processes passed their declared correctness strings. The valid
current `mc_decimal` L1 medians include:

| Mode | Common-18 | Classic-9 |
| --- | ---: | ---: |
| arithmetic, 50,000,000 iterations | 2.690296 s | 3.286608 s |
| conversion, 20,000,000 iterations | 1.046066 s | 1.036647 s |
| comparison, 50,000,000 iterations | 1.220974 s | 1.210482 s |
| copy/clear, 60,000,000 iterations | 1.368440 s | 1.364793 s |
| context sync, 1,000,000,000 iterations | 1.339110 s | 1.368573 s |

The first 6,000,000-iteration conversion rows are retained as invalid
calibration because they did not clear the one-second floor; the v2 rows above
replace them. Some product and diagnostic-provider cells are marked
`rerun_recommended` in their raw summaries. They are retained without sample
removal and must not be promoted into an aggregate or a current-provider
regression claim. The optimizer controls and canonical RexxCPS guard remain
open; this evidence bundle does not infer them from historic checksums.

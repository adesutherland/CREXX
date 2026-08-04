# PERF3-12 K04e RexxCPS clause reassessment

Date: 2026-08-04

Status: complete analysis-only reassessment; no product source changed

## Outcome

The current accepted K04e product reduces fixed-work optimized RexxCPS from
`54,221,210/54,221,182` to `53,660,581/53,660,552` instructions under
`rxvm`/`rxbvm`. K04e's independently isolated hot compare accounts for exactly
560,000 removed dispatches. The remaining low-frequency movement accompanies
the accepted D0.6/current shared-library product and final formatting/control
paths.

The PERF3-12 clause ranking and implementation order do not change:

1. PARSE direct-destination transactions remain the largest derived ceiling
   at 9,240,000 dispatches, now 17.219344% of the current `rxvm` stream.
2. Compound-tail segmented access remains a 2,240,000-dispatch ceiling
   (4.174386%); loop-scoped reuse remains 1,960,000 (3.652588%).
3. Copied XTOY remains exactly 2,220,000 `DCOPY`/`DTOS` pairs and 97,680,000
   decimal-copy bytes, a 4.137115% copy-elimination ceiling.
4. Direct call plus return removal remains 560,000 dispatches (1.043597%)
   before separate frame work.

`PERF3-12A / R12-C01` copied-XTOY component placement remains the next
recommended implementation slice. It is not the largest ceiling, but is still
the first bounded consumer whose value/component/use proof is already covered
by the accepted RXAS infrastructure. PARSE still needs its exact conditional
signal and failure-visible-write contract first.

## Qualification

- Clean detached product: `c4470635048e497417c4db92c03ecbcd79eaa750`.
- Ordinary Release product is profiling-off; the separate counts build is
  `Release`, `-O3 -DNDEBUG`, `CREXX_VM_PROFILING=ON`.
- The exact no-opt RXBIN is unchanged from the original PERF3-12 gate.
- All four fixed `--smoke-count 200` schema-5 cells pass with complete tracked
  domains, zero invalid events and zero counter overflow.
- No-opt instruction vectors are exactly equal across VMs. Current optimized
  vectors differ by 29 executions across 18 low-frequency final
  formatting/control opcodes; all ranked hot mechanisms agree.
- Profile elapsed times are diagnostic only. The profiling-off Mac scorecard
  remains runtime authority.

## Evidence map

- `profiles/`: complete checksum-closed dual-VM runner bundles;
- `analysis/profile-summary.csv`: previous/current total comparison;
- `analysis/mechanism-reassessment.csv`: exact candidate counts, ceilings and
  revised percentages;
- `VALIDATION.md`: commands, identities and independent count checks;
- `manifests/`: exact fixed-work schedule; and
- `checksums.sha256`: recursive closure excluding itself.

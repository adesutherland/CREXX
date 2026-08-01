# PERF3-02-R1 infrastructure-enabled option re-investigation

Status: **complete — C1abc selected and production-validated**

Approved: 2026-08-01

Purpose: repeat the PERF3-02 C0-C4 copy/ownership investigation after locking
the C2E2-P1 register-storage and typed-continuation infrastructure and proving
the C1b detached receiver-guard mechanism. Preserve the authoritative 2026-07-31
clean-host C0 timings, keep every old and rejected option replayable, compare
the newly enabled options independently and in combination, and time only
correctness-valid material candidates.

## Completed verdict

The locked infrastructure and all replay controls are preserved. C1a-R2,
C1b-R1 and C1c-R1 are independently correct and compose without interference.
The broad C1a-R1 no-write rule remains correctness-invalid both alone and when
combined with the safe rules.

The governed same-session paired medians are:

| Target | Candidate | `rxvm` | `rxbvm` |
| --- | --- | ---: | ---: |
| Richards | C1a-R2 | -9.04% | -9.11% |
| Richards | C1b-R1 | -44.28% | -44.01% |
| Richards | C1a+C1b | -53.55% | -52.57% |
| Towers | C1c-R1 | -18.92% | -18.97% |

Every row is favorable in all 12 pairs per VM and has a wholly favorable mean
interval. C1a and C1c reproduce the retained clean-host authority. C1abc emits
the exact C1ab Richards image and exact C1c Towers image, so those rows are its
measured workload-specific effect. Adrian selected C1abc on 2026-08-01. The
ordinary production compiler now contains the three safe proofs without the
disposable mask or broad negative branch.

## Authority and mandatory boundaries

Adrian approved implementation of the reusable infrastructure and a broad
option replay with correctness and ordinary profiling-off Release timing. This
authority includes isolated compiler and assembler-consumer variants needed to
distinguish C1-C4, but it does not select a final production ladder, authorize a
language/public RXAS/RXBIN/ABI/value-layout change, commit or push.

That investigation boundary was subsequently superseded only by Adrian's
explicit C1abc production selection. No public-contract, commit or push
authority was inferred from it.

The C2E2-P1 infrastructure is locked for this panel. Option work must not alter
its transfer functions, typed normal/signal-skip/signal-retry edges, storage
identity semantics or bounds. A proposed consumer that needs an infrastructure
change is recorded separately and fails this replay gate rather than quietly
changing the foundation.

Correctness precedes timing for every variant. A deterministic compile,
assembly, link, VM, TRACE/source, link/unlink, result or fail-closed regression
rejects that variant before timing. Correctness-invalid variants remain
reproducible negative controls and are not benchmarked.

## Frozen identities

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch/HEAD: `develop` at
  `e38e514bf611ae3873513368c44742e2ae7332d1`
- Product-code parent: `3f43a0014be10c930a12b8a636297b60f294c0a6`
- Locked P1 Release `rxas` SHA-256:
  `09ea8e49c54dfc839833a1fa40dfa5809467aa53c730dc050ae0a2b95f73669e`
- Locked P1 tracked source diff SHA-256:
  `b05623eef33dfcd30fb330f264a2e7b01df7cba94ac0f565d7c916f5bc3efbc6`
- Pre-P1 Release `rxas` control SHA-256:
  `af80b2c9f664a2ed1b7ec7a4cca5a419718f2f5384e3cfb880f86c7a06ad7422`
- Retained C0 `rxc` SHA-256:
  `caec40f5d7d6304e29e2a678c5604a5d78e42172bc1ee0bf82b52a130f5f99d1`
- Approved C1b correctness product `rxc` SHA-256:
  `0323b70207c22486896d6883a80ca4388f6a4c164d03737bcefc3a3a8ed6be52`
- Five existing lifecycle RXBIN files and all prior checksum-closed evidence
  bundles remain protected.

## Retained timing authority

The clean-host governed C0/C1a-R2/C1c-R1 panel in
[`evidence/2026-07-31-perf3-02-copy-ownership-panel/`](evidence/2026-07-31-perf3-02-copy-ownership-panel/)
remains the historical timing authority and is not rewritten:

| Candidate | Target | `rxvm` median elapsed | `rxbvm` median elapsed |
| --- | --- | ---: | ---: |
| C1a-R2 | Richards | -9.178933% | -9.330797% |
| C1c-R1 | Towers | -19.424614% | -19.645181% |

New same-session C0 executions are drift and paired-comparison controls only.
They do not replace those retained values. Any cross-session comparison is
labelled historical; decisions use same-session balanced pairs and reconcile
their C0 drift against the retained baseline.

## Reimplemented option matrix

The compiler variants use one source state and an internal compile-time option
mask so every positive, combined and rejected option remains rebuildable from
the final evidence. The ordinary default remains the approved C1b detached
guard mechanism; explicit masks isolate other rows.

| ID | Mask | Owner | Replayed question | Timing gate |
| --- | ---: | --- | --- | --- |
| C0 | 0 | retained product | Original materialized receiver and locked P1 assembler | retained authority plus drift control |
| C1a-R2 | 1 | `rxc` | Unused, call-free, attribute-free nested receiver direct use | Richards if correct |
| C1b-R1 | 2 | `rxc` | Exact detached indexed-Boolean receiver guards | Richards if correct |
| C1c-R1 | 4 | `rxc` | Direct use of already-isolated by-value object-formal storage | Towers if correct |
| C1a-R1-negative | 8 | `rxc` | Broad no-write rule, retained as a falsification control | never if structural/runtime correctness fails |
| C1ab | 3 | `rxc` | Independently combine unused-receiver and detached-guard facts | Richards if correct and additive |
| C1abc | 7 | `rxc` | Combine all three independently proved compiler facts | Richards and Towers if correct |
| C1broad+c1c | 14 | `rxc` | Broad no-write negative with snapshots where exact, plus formal storage | never if any fail-closed rule opens |
| C2-P2 | n/a | `rxas` locked storage service | Re-evaluate full-copy projection with point storage identity and typed continuations | only if ownership/TRACE/lifetime proof is complete |
| C3-R1 | n/a | compiler/private existing form | Recompute scalar/binary/no-payload residual after C1/C2 | only if material and no ISA/ABI change |
| C4-R1 | n/a | isolated ceiling | Exact direct operation at every proved C1 site | measured through matching C1 rows |

Bit values were investigation controls, not public compiler options or a
production configuration surface. The selected implementation has removed
that disposable branching while the exact source patch and build commands
remain retained in checksum-closed evidence.

## Correctness matrix

Each built option must pass, as applicable:

1. focused inliner/import/receiver tests, including C1b local/imported
   positives and all fail-closed negatives;
2. optimized and no-opt Richards and Towers on `rxvm` and `rxbvm`;
3. optimized Permute, Bounce and Sieve dual-VM guards;
4. exact canonical Richards `23246/9297` and each benchmark-native PASS result;
5. locked P1 assembler focused flow/runtime tests and debug storage summaries;
6. source/TRACE and unrelated-image identity or an exact explained delta; and
7. protected lifecycle and prior evidence checksums.

C2 may remove a full copy only if storage identity, independent value
availability, destination ownership/destruction, source lifetime, TRACE/source
projection and every normal/skip/retry/handler path are all proved. Exact
register storage identity alone is intentionally insufficient.

## Timing matrix

After correctness:

- build ordinary profiling-off Release products;
- use the exact canonical benchmark images and common locked P1 assembler;
- run serially on AC with low-power mode off and record load/thermal/power state;
- use at least one warmup and 12 balanced/interleaved pairs for each decisive
  candidate/target/VM cell;
- pair Richards as the C1a/C1b target and Towers as the C1c target;
- retain the opposite workload as the first guard in both VMs;
- use no outlier removal; apply the standing confidence/noise append rules; and
- preserve raw samples, exact binaries/images, hashes and commands.

The C1abc combined row is compared both with C0 and with its strongest
single-workload constituent so additivity or interference is visible. A
same-session C0 drift cell is reconciled with the retained clean-host baseline.

## Work stages

### Stage A — lock and control plane

- [x] Reread root/performance instructions and the live roadmap.
- [x] Freeze HEAD, P1 source/binary identity, C0/C1b products and protected
      dirty scope.
- [x] Preserve old timings as historical authority and define new C0 as drift
      control only.
- [x] Record the full replay matrix and correctness-before-timing rule.

### Stage B — reproducible compiler variants

- [x] Add the internal option mask without changing public compiler behavior or
      metadata schema.
- [x] Reimplement C1a-R2 and C1c-R1 from retained source diffs on top of C1b.
- [x] Retain the broad C1a-R1 shape as an explicit negative option.
- [x] Build isolated C0/C1a/C1b/C1c/combined/negative products and record hashes.

### Stage C — infrastructure-enabled C2/C3/C4 analysis

- [x] Re-run locked storage identity against each compiler image.
- [x] Re-evaluate C2 eligibility using storage plus ownership/value/lifetime/
      TRACE/continuation obligations; build a rewrite only if all are complete.
- [x] Recompute C3 residual after the union of correct C1/C2 options.
- [x] Record exact C4 machine ceilings for every correct material row.

### Stage D — correctness

- [x] Run focused structural/fail-closed tests for every built option.
- [x] Run optimized/no-opt dual-VM Richards and Towers.
- [x] Run optimized dual-VM Permute/Bounce/Sieve guards.
- [x] Reject and retain every failing option before timing.

### Stage E — governed timing

- [x] Capture pre-run host/power/load/process state.
- [x] Run balanced pairs for every distinct correct material target image;
      prove the opposite-workload images byte-identical to C0 rather than time
      separately assembled duplicates.
- [x] Run the correct combined candidate against C0 and prove its constituent
      workload images exact.
- [x] Apply governance noise/guard rules without deleting samples.

### Stage F — decision package

- [x] Publish one checksum-closed evidence bundle with source patches, product
      hashes, correctness, deterministic deltas, raw samples and paired effects.
- [x] Reconcile new observations with retained C0/C1a-R2/C1c-R1 timings.
- [x] Update the roadmap/worklist and present selected/rejected/deferred rows.
- [x] Stop before production cleanup, commit or push.

### Stage G — selected production closeout

- [x] Record Adrian's C1abc selection and retain the checksum-closed R1 replay
      source before cleanup.
- [x] Remove the disposable option mask and broad correctness-negative branch;
      preserve only the bounded C1a, C1b and C1c production proofs.
- [x] Add the focused production-ladder fixture and refresh seven exact
      optimized-RXAS goldens after their paired runtime tests passed.
- [x] Prove the ordinary Release Richards and Towers RXAS byte-identical to the
      measured mask-7 programs.
- [x] Pass 11/11 focused Release checks, 16/16 reviewed Debug object pairs and
      1,972/1,972 broad Debug tests.
- [x] Publish the checksum-closed closeout bundle at
      [`evidence/2026-08-01-perf3-02-c1abc-closeout/`](evidence/2026-08-01-perf3-02-c1abc-closeout/).
- [x] Stop before commit or push.

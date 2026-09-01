# PERF3-02 timing revalidation plan

Status: **complete; authoritative clean-host campaign finished at
2026-07-31 17:21 BST**

The first C1a-R2 and C1c-R1 timing campaigns are retained but provisional. A
remote terminal was active and may have materially affected this Mac. The
clean-host result under this plan is authoritative; deterministic correctness,
instruction, copy, byte and allocation counts were unchanged.

## Completed result

- 412/412 executions passed: 376 recorded samples plus 36 warmups.
- C1a-R2 Richards paired median elapsed: -9.18%/-9.33% (`rxvm`/`rxbvm`).
- C1c-R1 Towers paired median elapsed: -19.42%/-19.65%.
- Every target pair is favorable and all target mean intervals exclude zero.
- All byte-identical guards reached 36 pairs. Their intervals cross zero and
  are noisy/inconclusive for direction, but every upper bound is below +3%.
- `paired-effects.csv` is the decision reduction;
  `run-v1-final-summary/summary.csv` combines every raw sample for absolute
  cell statistics. No raw or provisional sample was deleted.

## Preserved option set

| Variant | Reproducible input | Rerun disposition |
| --- | --- | --- |
| C0 | product commit `3f43a0014`; exact hashes in `artifacts.csv` | same-session control for every workload/VM |
| C1a-R1 | `poc/c1a-r1/source.diff` plus exact hashes/output | rebuild and replay the correctness rejection; do not time invalid output |
| C1a-R2 | `poc/c1a-r2/source.diff` plus exact hashes/manifests | speed-eligible target and guard |
| C1b | design and quantified rejected boundary in owner ledger | no artifact existed; nothing can be timed without a new design decision |
| C1c-R1 | `poc/c1c-r1/source.diff` plus exact hashes/manifests | speed-eligible target and guard |
| C2-E1 | `poc/c2-e1/source.diff`, flow logs, hashes and identical reassemblies | replay eligibility/hash/correctness; no changed image to time |
| C3 | exact residual ledger | no PoC existed because the residual was immaterial |
| C4 | exact success path of C1a-R2/C1c-R1 | remeasured by those two candidates |

No source diff, manifest, rejected output or timing sample is to be deleted.
If a `/private/tmp` root disappears, recreate a detached worktree at
`3f43a0014`, apply the retained variant diff, configure Ninja Release with
profiling off, and verify the retained cache/product/image hashes or record a
new build identity before timing.

## Host gate

1. Adrian confirms the remote terminal is off.
2. Record UTC/local time, AC/battery, low-power mode, thermal/performance
   warnings, uptime/load, interactive sessions and overlapping build/test/VM
   processes.
3. Take multiple short load snapshots and start only after the host is stable.
4. Run no build, CTest, profiler, benchmark or other performance campaign in
   parallel. If the remote terminal returns, stop and invalidate the affected
   block rather than averaging it in.

## Same-session speed matrix

Use `common-vm-c0-c1-v1.txt`. It contains 12 cells:

- Richards: C0, C1a-R2 target and C1c-R1 unchanged-main guard in both VMs;
- Towers: C0, C1c-R1 target and C1a-R2 unchanged-main guard in both VMs.

All cells use the same exact C0 `rxvm`/`rxbvm` binaries, isolating the
compiler-generated main/library images from irrelevant rebuild identity. Run
one warmup plus 12 recorded rounds, serial and rotated by workload. Retain
every sample/output, compute paired per-round C0 ratios, and apply the standing
noise/append rule rather than selecting convenient samples.

Ready command after the host gate:

```sh
/private/tmp/crexx-perf3-01.jetu7C/build-release/bin/rxvm \
  /private/tmp/crexx-perf3-01.jetu7C/build-release/tests/benchmarks/nr10_cross_runtime_matrix_selftest_opt.rxbin \
  /private/tmp/crexx-perf3-01.jetu7C/build-release/bin/library.rxbin \
  -a --manifest \
  performance/evidence/2026-07-31-perf3-02-copy-ownership-panel/rerun/common-vm-c0-c1-v1.txt \
  --output-dir \
  performance/evidence/2026-07-31-perf3-02-copy-ownership-panel/rerun/run-v1 \
  --warmups 1 --runs 12
```

Before launching, recursively verify PERF3-01 and this bundle's then-current
checksums and rehash every manifest input against `artifacts.csv`. Afterward,
update the timing table and candidate recommendation, checksum-close the final
bundle, and return to the production-candidate selection stop.

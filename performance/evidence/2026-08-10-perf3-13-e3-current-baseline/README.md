# PERF3-13 E3 current single-worker baseline

Date: 2026-08-10

This bundle records the clean current-product Apple baseline before the next
Gate E slice. The source is `develop` at
`6d12cd921cdbf9cb2098df2a4c8ae6eee75e4a7f`, synchronized with
`origin/develop`. The build is ordinary profiling-off Release under Apple
Clang 21.0.0. The new default handler policy is `profile-20`: 118 ranked public
handlers plus both private fused handlers are inline, for 120/589
policy-controlled non-reserved definitions; the remaining eligible handlers
are callable.

`rxvm` selects `rxbvm` in this configuration, so `rxbvm` is the product
authority and `rxtvm` is the explicit threaded-dispatch guard. Both concrete
engines use the same current optimized workload images and `library.rxbin`.

## Result

The formal absolute block used two warmups and ten serial balanced recorded
samples for each of 14 cells. All 168 processes passed their exact output
oracle. The standing noise rule selected `rxbvm` Richards, both Base64 cells
and `rxbvm` RexxCPS for one ten-sample append; all 40 appended processes also
passed. No sample was removed and no second append was run.

| Workload | `rxtvm` median | n | `rxbvm` median | n |
| --- | ---: | ---: | ---: | ---: |
| Sieve | 0.943619 s | 10 | 1.009033 s | 10 |
| Permute | 1.644697 s | 10 | 1.653669 s | 10 |
| Bounce | 1.048156 s | 10 | 1.040004 s | 10 |
| Richards | 2.084427 s | 10 | 2.085128 s | 20 |
| Base64 | 1.606495 s | 20 | 1.699262 s | 20 |
| Towers | 2.057666 s | 10 | 2.055424 s | 10 |
| RexxCPS | 45.692530 M clauses/s | 10 | 44.307819 M clauses/s | 20 |

After the permitted append, Richards and RexxCPS have low relative MAD but a
min/max span just above 10%. Both Base64 cells remain noisy: relative MAD is
5.35%/9.72% and span is 24.67%/30.27% for `rxtvm`/`rxbvm`. They remain in the
raw record and must be labelled noisy in any later use.

## Product shape

| Engine | File bytes | `__text` bytes | `rxvm_run_owned_core` bytes | SHA-256 |
| --- | ---: | ---: | ---: | --- |
| `rxtvm` | 1,112,440 | 899,048 | 146,824 | `e675630ba2e37a4f82933e5a6a37b79d259364926d5e74da09b69cc0c41ad6d6` |
| `rxbvm` | 1,112,312 | 893,596 | 145,608 | `47ca2241af7bae301cc0d85f552f49c995eab7681b0a61b14d4ff04b83af1b9f` |

The current `library.rxbin` is 930,739 bytes with SHA-256
`f27a8ff0dc50a136686e6bfe21dbb4680fdb398737682c5039502a9e4bc3b4f3`.
Pre/post VM, library, workload, manifest and Level B runner hashes are
identical.

## Environment and interpretation

The run used the Apple M5 host with ten logical CPUs, 128 KiB L1 instruction
cache and 6 MiB L2 cache. It remained on AC power with low-power mode off and
no recorded thermal, performance or CPU-power warning. The observed load
average was approximately 2.0 on ten logical CPUs before and after the run.

This is a formal same-host absolute observation and a frozen entry control for
E3 planning. It is not a causal before/after verdict, a release-wide claim, a
concurrency-scaling result or permission to start E3. A production E3
candidate still requires a same-session paired/interleaved comparison against
the exact current control; these unmatched absolute medians cannot establish a
regression.

## Bundle map

- `manifest.txt` and `noise-append-manifest.txt`: exact governed schedules;
- `timing/initial/` and `timing/append/`: raw samples, outputs, capture
  manifests and independent summaries;
- `timing/summary.csv`: merged authoritative summary;
- `pre-state.txt` and `post-state.txt`: source, toolchain, host, power, process
  and hash provenance;
- `logs/`: compact runner and merge results; and
- `VALIDATION.md` and `SHA256SUMS`: retained closure checks and recursive
  bundle integrity.

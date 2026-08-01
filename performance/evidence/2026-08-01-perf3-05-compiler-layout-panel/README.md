# PERF3-05 compiler, layout and private-stream panel

Date: 2026-08-01  
Host: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs  
Source: local `develop` at `4a3940395980dc40ea45917d71d99caa080e89bb`
plus the accepted uncommitted P1A A1 source diff retained under `baseline/`.

## Decision

Retain L0, the ordinary profiling-off Release product. No tested compiler or
hot/cold layout form is a safe runtime candidate:

- L1 proves that Base64 is dominated by generated semantic string/copy work,
  but it is a separately named ceiling rather than a production replacement.
- ThinLTO and representative PGO materially change the native image, but both
  produce large workload/VM reversals. They are rejected as defaults.
- Removing `RX_FLATTEN` from `run()` makes the VM core cheaper to compile and
  smaller, but fails the non-noisy `rxbvm` Sieve guard and reverses direction
  between VM/workload cells. It is rejected as a runtime layout candidate.
- L4 is not opened. The current product already builds a private prepared
  execution image with bound handlers and operands, while this panel does not
  demonstrate a persistent direct-load ceiling that survives both VM modes.

No production interpreter, CMake or RXBIN change was made by this panel.

## Baseline

The fresh isolated baseline and unchanged-source drift rebuild are
byte-identical:

| Artifact | SHA-256 | Bytes | `__text` bytes |
| --- | --- | ---: | ---: |
| `rxvm` | `c5b1a09d37258e7e1b3fb9ebaa9c09008c88a949509055617d631ad75b75470f` | 998,840 | 809,228 |
| `rxbvm` | `7190a983a073c85a16115c665f2f1b433f28c97a7f64dce5f614e59549915c87` | 999,016 | 808,244 |

All five exact current workloads pass on both VMs. The baseline full Release
build took 136.17 seconds with 831,488,000 bytes peak RSS. The focused
unchanged-source rebuild took 35.41 seconds with 723,664,896 bytes peak RSS.

## Results

Positive percentages below mean faster than L0. Each runtime screen has one
warmup plus 12 balanced recorded pairs and correct output for all 240
executions.

| Form | Static/build result | Runtime disposition |
| --- | --- | --- |
| L1 Base64 codepoint/position ceiling | 0.560259/0.561543 s medians for `rxvm`/`rxbvm` | 2.691x/2.959x ceiling; diagnostic only |
| L1 Base64 arithmetic ceiling | 0.1410065/0.1604665 s | 10.691x/10.354x ceiling; semantic work dominates |
| L2 Apple ThinLTO | `__text` -6.295%/-6.475%; build 36.52 s, 695,566,336 bytes RSS | reject: Sieve `rxvm` -4.443%; Base64 `rxbvm` -11.351%; direction reversals |
| L2 merged representative PGO | `__text` -25.024%/-26.097%; use build 37.91 s | reject: Sieve -40.399%/-33.734%, Base64 -27.740%/-48.840%; per-VM profiles reproduce the skew |
| L3 no-flatten PoC | focused build 28.90 s and 653,049,856 bytes RSS; `__text` -5.165%/-5.606% | reject runtime: Sieve -1.821%/-3.639%; other cells reverse or remain near neutral |
| L4 private representation | not built | gate not met; defer architecture work |

The L3 PoC changes max RSS by at most 131,072 bytes across the 20 measured
cells. Its paired create/run/destroy probe differs by only +56 microseconds for
`rxvm` and -25.5 microseconds for `rxbvm`, below the lifecycle guard.

Fresh schema-5 current profiles keep all seven domains complete. Accepted
C1abc compiler work has already reduced Richards copy operations from the
retained 56.9 million to 26,817,578 and Towers from 26.8 million to 19,649,142.
That confirms that the material prior wins belong to generated work removal,
not a global VM layout switch.

## VM library link diagnostic

The reported slow VM-library link is not caused primarily by the number of
exported private functions. There are two real hygiene/coupling issues, but the
current Mac link steps are tens of milliseconds:

- `librxvml.a` is 1,040,384 bytes; its 805,296-byte `rxvmintp.c.o` member is
  about 77% of the archive.
- A client that calls only `rxvm_create()` and `rxvm_destroy()` pulls
  `rxvm_run.c.o`, whose unresolved `run` reference then pulls all of
  `rxvmintp.c.o`. The resulting minimal executable is 981,808 bytes.
- `libcrexxsaa.dylib` exports 367 defined globals although only 16 have the
  intended `crexxsaa_` prefix.
- Restricting the dylib to those 16 exports changes its median relink from
  37.392 ms to 37.439 ms: no measurable improvement. It reduces the file by
  only 6,920 bytes.
- `crexxsaa` declares its implementation archives and internal include paths
  `PUBLIC`. A downstream executable therefore scans the dylib plus all static
  implementation archives even though `-why_load` shows that none are pulled.
  The 15-run median is 35.095 ms with the expanded interface versus 21.706 ms
  for the dylib alone.
- The original clean-build Ninja log records 61 ms for `libcrexxsaa.dylib`,
  64 ms for the `crexxsaa` tool and 71 ms for a static VM test link.

The follow-on should therefore separate API/build hygiene from runtime
performance: make shared-library implementation dependencies private, publish
only the supported include surface, add platform export control, and split the
phase API object if narrow static clients are a supported use case. Re-measure
on the host where the slow report originated before treating it as a build-time
priority.

## Evidence map

- `baseline/`: exact source diff, configure/build logs, correctness and drift.
- `semantic-ceiling/`: retained control RXAS/RXBIN, correctness and timing.
- `lto/`: effective IPO proof, binaries, static data and formal timing.
- `pgo/`: raw training profiles, merged and per-VM profiles, effective-use
  proof, binaries and formal timing.
- `hotcold/`: disposable one-line patch, build/static proof, formal timing,
  RSS and lifecycle evidence.
- `current-profiles/`: exact current Richards/Towers schema-5 profiles.
- `link-diagnostic/`: archive/object coupling, exports and measured link costs.
- `candidate-ledger.md`: exact L0-L4 selection reasons.
- `replay.md`: bounded replay routes.


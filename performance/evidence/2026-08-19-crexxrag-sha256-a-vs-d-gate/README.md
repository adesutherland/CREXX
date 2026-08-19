# CREXXRAG-SHA256 A-versus-D gate

Status: qualification pilot complete; **Option A recommended**; no production
change authorized or made.

## Verdict

The result is decisive enough for the design choice. The integrated
process-reentrant native path is **196.5x to 252.9x faster** than the
straightforward portable Level B implementation in the timed hash kernel.
Pure Level B reaches only 0.765-1.726 MiB/s; integrated native reaches
163.8-406.6 MiB/s.

The native boundary is not the material cost for asset hashing. Relative to
the same direct-C `rx_sha256()` ceiling, integrated RXPA is about 1.28-1.30x
the kernel time for 64-byte messages, 1.03-1.05x at 4 KiB, and 1.04-1.07x at
1 MiB. At 64 bytes the added boundary cost is only about 0.09 microseconds per
call.

This passes the worklist's Option-A gate by a very wide margin. The appropriate
production design is one public Level G hash facade backed by a small
process-reentrant `rx_hash` native module. It does **not** imply one monolithic
native library for all Level G assets; hash, vector, SQLite, and other native
capabilities should remain separately linkable/packageable modules under a
coherent Level G facade and aggregate distribution.

Pure Level B remains valuable as executable reference code, vector oracle, or
an explicitly slow fallback. It should not be the primary `crexx-rag` asset
hashing implementation.

## Recorded medians

| Bytes x hashes | VM | Pure kernel | Native kernel | A speedup | Pure MiB/s | Native MiB/s | Direct C MiB/s |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 64 x 20,000 | `rxtvm` | 1,595.702 ms | 7.453 ms | 214.10x | 0.765 | 163.787 | 212.481 |
| 64 x 20,000 | `rxbvm` | 1,447.543 ms | 7.366 ms | 196.52x | 0.843 | 165.721 | 212.481 |
| 4 KiB x 500 | `rxtvm` | 1,266.795 ms | 5.010 ms | 252.85x | 1.542 | 389.845 | 407.665 |
| 4 KiB x 500 | `rxbvm` | 1,145.984 ms | 4.954 ms | 231.32x | 1.704 | 394.252 | 407.665 |
| 1 MiB x 2 | `rxtvm` | 1,265.165 ms | 5.052 ms | 250.43x | 1.581 | 395.883 | 423.549 |
| 1 MiB x 2 | `rxbvm` | 1,158.896 ms | 4.919 ms | 235.60x | 1.726 | 406.587 | 423.549 |

Process-inclusive medians retain the same decision despite deliberately
including VM startup and deterministic input construction: pure is 92.6x to
121.9x slower at 64 bytes, 85.4x to 96.8x at 4 KiB, and 39.7x to 49.9x at
1 MiB. These millisecond-scale lifecycle cells are diagnostic: policy-required
ten-sample appends were retained for the five initial series whose process or
kernel span exceeded 10%, and some process series remain noisy.

## Correctness and scope

- Empty and `abc` standard vectors pass.
- Embedded zero bytes and the 55/56/63/64/65-byte padding boundaries pass.
- Deterministic 4 KiB and 1 MiB binary payloads pass.
- Pure Level B and native results match the independent expected digest on
  both `rxtvm` and `rxbvm`.
- Release CTest passes optimized and non-optimized vector execution; explicit
  optimized selftests pass both concrete VMs.
- No production source, ABI, public API, or package layout was changed.

This is a qualification pilot, not a release claim or formal portfolio
baseline. Initial cells use three observations; five short/noisy cells include
the required ten-sample append. That limitation does not make a 196-253x
same-host kernel gap ambiguous. A production implementation still needs the separately
approved API/ownership/incremental-state design and the usual correctness,
package, platform, sanitizer, and first-Release-verdict gates.

Raw per-cell samples, stdout/stderr and exact commands are under `cells/`.
Machine-readable medians are in `summary.csv`; vector results are in
`correctness.csv`; build and host details are in `PROVENANCE.md`.

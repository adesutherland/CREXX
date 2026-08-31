# Unicode grapheme verdict provenance

## Interpretation boundary

- Capture time: 2026-08-29 20:26-20:27 UTC.
- Base source identity: `7ffe3c0db905ce5d2fd3e5199fb9d4470d2420e2`.
- Branch: `unicode`.
- Tree state: dirty for the approved grapheme implementation, tests,
  benchmark, documentation, and preceding Unicode normalization-certificate
  work.
- User authorization: Adrian stated that the host was mostly clear for the
  first performance run and subsequently directed broad qualification.

## Host and build

- Host: `Adrians-MacBook-Air.local`, Apple Mac17,3, 10 logical CPUs.
- OS: macOS 26.6.2, Darwin 25.6.0, arm64.
- Power: AC attached; low-power mode disabled.
- Pre/post load averages: 2.66/3.24/2.90 and 2.73/3.23/2.90.
- Build: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`.

## Artifact identities

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| `tests/performance/unicode_grapheme_compare.crexx` | 3,085 | `01b9117f834f9aea2021aa0232b6d85164364e3fdfdf16c8525803ada0b548c7` |
| `rxbvm` | 1,463,896 | `1f9164ab7cde53009792ada4ee6603516c7e4d93f5799ec75c9c0d36d72caf27` |
| `rxtvm` | 1,464,024 | `9f8601730a9f5e6aca7844ef6a399ca449ac929413da5af94fcf60672d3ab5a6` |
| comparison RXBIN | 16,958 | `3c372c2ec4a47d8a7e6a1226e34f723122731f35699aaeef439fc1b7f3fd569c` |
| comparison RXAS | 44,843 | `d258dcab1e0e12e72c6b091d109e1362cdf66c38ba2c2b1c40ba458110da0cde` |

The generated RXAS/RXBIN are identified but not retained: the benchmark source
and ordinary Release toolchain reproduce them. `build.log` records the exact
compiler, assembler, and linker invocations used by the capture.

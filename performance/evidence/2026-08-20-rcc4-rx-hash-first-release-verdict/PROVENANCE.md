# RCC-4 first-Release verdict provenance

## Interpretation boundary

- Date: 2026-08-20
- Baseline source identity: `d46791bd9ac8dfeb5a66aabb119da3e7da678d47`
- Branch: `develop`
- Tree state: dirty for the approved RCC-4 implementation, its tests,
  performance harness integration, documentation, and this evidence.
- User authorization: Adrian approved the production performance test, stated
  that the host was clear for the required runs, selected Option A, and accepted
  this first-Release verdict before qualification continued.

## Host and build

- Host: `Adrians-MacBook-Air.local`
- OS: macOS 26.5.2, Darwin 25.5.0, arm64
- CPU: Apple M5, 10 logical CPUs
- CMake: 4.3.2
- Compiler: Apple clang 21.0.0
- Build: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`

The runner manifests under `cells/` retain the absolute argv, timestamps,
environment description, stdout/stderr and process-inclusive timing for every
observation. `benchmark_metric` is the program-reported steady-state elapsed
microseconds; payload construction and final digest validation are outside that
kernel.

## Artifact identities

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| production `lib/plugins/hash/rx_hash.c` | 856 | `cfa32ca5ef99e78bcb5bd35526151b5dbeca5d123cb6aa080c97587bc6c9964b` |
| integrated performance source | 9,316 | `8d141dab174ef125f16adcec461c950f91534b3e9806ea2e31ee599d9da4af49` |
| `rxbvm` | 1,413,512 | `4655c99565e7ea22d9d48c61066e8bba34ab1c44659eefd62f6d84cdeaa41caf` |
| `rxtvm` | 1,430,152 | `72ff062f3145f07e4248e575bfeb090001c1c566fa377c84e84534f4a39e7dc3` |
| production `rx_hash.rxplugin` | 33,840 | `f044b469e67cf30f9846646d85b31f091fe97d1a49c67586870589629441d16b` |
| optimized comparison RXBIN | 37,074 | `66c5386db9ec492d03c37b22fb13e412edab0fc8b1009f280e32cfe6fe76eef4` |
| test-only control plugin | 33,888 | `12d05c3eb4a1b3183c11e741c3ed4dff116b7469ebf1678ee81179f34052d328` |
| `library.rxbin` | 720,489 | `2257bb1b09bdd1283c4a2480a18af124c845231871ea99f5f2676b77d3f1fbd1` |

The production and test-only SHA-256 procedure bodies were inspected with
`nm -nm` and `otool -tvV`. Their instruction sequences are identical; only
procedure symbol and literal-address identities differ.

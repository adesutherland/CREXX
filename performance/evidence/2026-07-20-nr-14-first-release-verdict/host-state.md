# Host and build state

## Before formal measurement

- UTC: `2026-07-20T19:02:11Z`
- Host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs
- Power: AC, battery 100%, low-power mode 0
- Thermal: no thermal or performance warning recorded
- Load averages: 1.21, 1.76, 2.48
- Branch: `develop`
- HEAD: `8424587f258ac37f133adab4194a3e80a5ee0875`
- `origin/develop`: `5626d6b871d740387765de40bfbebd246471102f`

## After formal measurement and governed append blocks

- UTC: `2026-07-20T19:22:55Z`
- Kernel: Darwin 25.5.0
  `RELEASE_ARM64_T8142`, arm64
- CPU: Apple M5, 10 logical CPUs
- Power: AC, battery 100%, low-power mode 0
- Thermal: no thermal warning, performance warning or CPU power warning
  recorded
- Load averages: 1.39, 1.27, 1.53

## Toolchain and products

- CMake 4.3.2
- Ninja 1.13.2
- Apple clang 21.0.0 (`arm64-apple-darwin25.5.0`)
- Baseline build: `cmake-build-nr14-baseline`
- Candidate build: `cmake-build-nr14-candidate`
- Both CMake caches: `CMAKE_BUILD_TYPE=Release`,
  `CMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG`,
  `CMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG`, Ninja generator,
  `CREXX_VM_PROFILING=OFF`.
- Formal samples ran serially; no build, test or other benchmark campaign ran
  concurrently.

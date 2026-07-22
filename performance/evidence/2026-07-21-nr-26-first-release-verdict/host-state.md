# Host and build state

## Before the initial formal block

- UTC: `2026-07-21T14:46:50Z`
- Local: `2026-07-21T15:46:50+0100 BST`
- Host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs
- Power: AC, battery 59% and charging, low-power mode 0
- Thermal: no thermal, performance or CPU power warning recorded
- Initial post-build load averages: 2.74, 5.29, 4.80
- After a bounded cool-down: load 1.42, 3.84, 4.29; CLion 2.0% CPU
- Branch/HEAD/upstream: `develop`,
  `4ab5f3d8da673c10b81af4249757763d052dda34`, exactly equal to
  `origin/develop`

## Invalid final-block attempt

At local `16:01`, the immediate pre-start check showed CLion at 281% CPU.
The block was interrupted before `samples.csv` or `outputs.csv` existed. Its
directory retains only the `running` capture manifest. After 30 seconds, CLion
was 2.3% CPU and load averages were 1.74, 2.50, 3.23; the entire block was then
run in the distinct `timing-append-02-retry/` directory.

## After the valid capped series

- UTC: `2026-07-21T15:08:25Z`
- Local: `2026-07-21T16:08:25+0100 BST`
- Power: AC, battery 81% and charging, low-power mode 0
- Thermal: no thermal, performance or CPU power warning recorded
- Load averages: 1.98, 2.39, 2.93

## Toolchain and product

- CMake 4.3.2
- Ninja 1.13.2
- Apple clang 21.0.0
- `CMAKE_BUILD_TYPE=Release`
- C/C++ Release flags: `-O3 -DNDEBUG`
- `CREXX_VM_PROFILING=OFF`
- All formal samples ran serially; no build or other benchmark campaign ran
  concurrently.

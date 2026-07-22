# Host and build state

- Measurement date: 2026-07-21 BST.
- Host: Apple M5, Darwin 25.5.0 arm64, 10 logical CPUs.
- Power: AC attached, battery 85%, low-power mode off.
- Branch: `develop`.
- HEAD: `8424587f258ac37f133adab4194a3e80a5ee0875`.
- `origin/develop`: `5626d6b871d740387765de40bfbebd246471102f`.
- Tree: the uncommitted NR-14 scope listed in this task; no unrelated tracked
  edit was introduced during measurement.
- CMake 4.3.2; Ninja 1.13.2; Apple clang 21.0.0.
- Baseline and candidate: `CMAKE_BUILD_TYPE=Release`,
  `CREXX_VM_PROFILING=OFF`, `-O3 -DNDEBUG`.
- Execution: serial; no thermal or power warning was observed during the run.

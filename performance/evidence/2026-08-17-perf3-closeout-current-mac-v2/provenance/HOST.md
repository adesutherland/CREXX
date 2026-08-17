# Host and source provenance

- Base commit: `c38a5d184dee1ecd659c83ccbc4a18001d46dc77`
- Source state: controlled PERF3-closeout working tree, intentionally
  uncommitted; version string
  `macOS 64 crexx-1.0.0-beta.3+local.gc38a5d184dee.dirty 20260817`
- Host: MacBook Air `Mac17,3`, Apple M5, 10 logical CPUs
- OS: macOS 26.5.2 (25F84), Darwin 25.5.0, arm64
- Build: CMake 4.3.2, Apple system compiler, Release `-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=OFF`, profile-20 VM layout
- Comparator runtimes: ooRexx 5.1.0 r12973; Regina 3.9.7; NetRexx 5.10-GA
  build 18-20260320-1410; Temurin OpenJDK 26.0.1+8
- Power at formal start: AC attached, battery 80%, low-power mode 0
- Thermal/power state at formal start: no thermal or performance warning
- Host load check at formal start: approximately 89% CPU idle; no benchmark or
  build process competing with the serial matrix
- Formal timing: 2026-08-17 15:30:22-15:54:16 UTC
- Governed append: 2026-08-17 15:55:44-16:01:47 UTC
- RSS: 2026-08-17 16:02:20-16:06:26 UTC

The fresh build predates only the lifecycle runner's host-launcher correction;
that maintained Level B tool was rebuilt in the same fresh tree before the
lifecycle capture. The tool edit does not alter any timed benchmark, product
VM, compiler, assembler, linker or library image. `artifacts.csv` records the
post-correction tool hash and the exact measured product/image hashes.

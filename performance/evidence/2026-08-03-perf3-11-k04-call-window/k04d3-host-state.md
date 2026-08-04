# K04d3 host state

- Host: Apple M5, 10 logical CPUs, 24 GiB RAM.
- OS: Darwin 25.5.0 arm64.
- Toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2.
- Build: `Release`, Ninja, `CREXX_VM_PROFILING=OFF`.
- Power: AC attached at 80% battery; low-power mode 0 before and after.
- Thermal: no thermal, performance or CPU power warning recorded after the run.
- Sampling: serial and workload-rotated; one warmup plus 12 recorded rounds per
  cell, started 2026-08-03 14:25:09 UTC and completed 14:26:18 UTC.
- Load averages: 1.60/3.14/4.38 before and 2.15/2.89/4.13 after.
- The initially detected `avconferenced` load was stopped before measurement.
  `WindowServer` remained at approximately 39-40% CPU and is disclosed; the
  same-session balanced design and benchmark-native metric limit its effect.

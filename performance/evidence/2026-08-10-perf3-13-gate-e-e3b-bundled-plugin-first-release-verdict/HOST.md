# Reserved-host state

## Before capture — 2026-08-10T18:52:53Z

- Darwin 25.5.0 arm64, Mac17,3, Apple M5, 10 logical CPUs.
- AC attached; battery 80%; low-power mode 0.
- No thermal, performance or CPU-power warning recorded.
- Load averages 1.71, 2.18, 1.87.
- No build, CTest or benchmark runner active.

An unintended linked-runtime CTest fixture was terminated before capture. Its
build processes were confirmed absent and the host was allowed to return to
the state above. It produced no timing sample in this evidence bundle.

## Formal capture

- Started after the recorded pre-capture state.
- Completed 2026-08-10T18:53:29Z.
- Serial pairwise-balanced schedule with no overlapping build, test or second
  benchmark runner.

## After capture — 2026-08-10T18:53:29Z

- AC attached; battery 80%; low-power mode 0.
- No thermal, performance or CPU-power warning recorded.
- Load averages 1.43, 2.07, 1.84.
- No build, CTest or benchmark runner active.

Toolchain: Apple Clang 21.0.0, Ninja 1.13.2; the existing Release tree was
configured by CMake 4.2.2 with `CMAKE_BUILD_TYPE=Release`,
`CREXX_VM_PROFILING=OFF` and `CREXX_VM_HANDLER_PANEL=profile-20`.

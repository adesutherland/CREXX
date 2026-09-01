# NR-27 formal host state

## Product and build

- Source branch/commit: `develop` at
  `25cbca679bcee9dcc8413def4341a49044e1ff66`, with only the uncommitted NR-27
  implementation, tests, documentation and evidence changes present.
- Requested retained baseline lineage: `4ab5f3d8d`; the accepted source start is
  its one-commit successor because `25cbca679` is completed NR-26 and NR-27 is
  independent of NR-26 compiler facts.
- Candidate version: `crexx-1.0.0-beta.3+local.g25cbca679bce.dirty`.
- Build: Ninja Release, `CMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=OFF`.
- Compiler: Apple clang 21.0.0, target `arm64-apple-darwin25.5.0`.
- CMake 4.3.2; Ninja 1.13.2.

## Host

- Host: `Mac.lan`.
- OS: macOS 26.5.2 build 25F84; Darwin 25.5.0 arm64.
- Kernel: `Darwin Kernel Version 25.5.0: Tue Jun 9 22:28:17 PDT 2026;
  root:xnu-12377.121.10~1/RELEASE_ARM64_T8142`.
- CPU: Apple M5; 10 logical CPUs.
- Uptime: approximately 8 days 4 hours during the campaign.

## Immediate state

| Checkpoint | Power | Battery | Low-power mode | Thermal/performance warning | Load averages | Process CPU sum |
| --- | --- | ---: | ---: | --- | --- | ---: |
| Before formal capture | AC attached, not charging | 80% | 0 | none recorded | 1.95 / 3.01 / 3.16 | 23.2% |
| Before noise append | AC attached, not charging | 80% | 0 | none recorded | 1.72 / 2.03 / 2.62 | not sampled |
| Before decision append | AC attached, not charging | 80% | 0 | none recorded | 2.06 / 2.01 / 2.51 | not sampled |
| After final append | AC attached, not charging | 80% | 0 | none recorded | 1.46 / 1.96 / 2.39 | 16.0% |

No build, test or other benchmark command overlapped a recorded capture.

# Exact-baseline GitHub build evidence

- **Workflow:** Build CREXX
- **Run:** [33056734858](https://github.com/adesutherland/CREXX/actions/runs/33056734858)
- **Head SHA:** `dc44d92909e706adc575932ba9ae72f2d6f05b7d`
- **Conclusion:** success
- **Run interval:** 2026-08-27 09:02:35Z to 09:38:36Z

The run metadata and complete log were downloaded on 2026-08-27. Step elapsed
times below are derived from GitHub step timestamps; CTest totals and test
counts are taken from the log.

| Job | Configure | Build | CTest step | Test result |
| --- | ---: | ---: | ---: | --- |
| Windows x64 | 12 s | 797 s | 807 s | 2,380/2,380; CTest 807.03 s |
| macOS arm64 | 9 s | 427 s | 650 s | 2,367/2,367; CTest 649.89 s |
| macOS x86_64 | 29 s | 898 s | 1,058 s | 2,367/2,367; CTest 1,057.50 s |
| Linux x64 | 11 s | 728 s | 766 s | 2,367/2,367; CTest 766.38 s |

The workflow used MinSizeRel, `--parallel 2` for Unix builds, `--parallel 4`
for Windows builds, CTest `--parallel 1` on Unix and `--parallel 2` on Windows.
It explicitly excluded the `performance-measurement` label. This is useful
cross-platform correctness evidence at the Phase 0 source SHA, but it has no
peak-memory evidence and is not comparable with the local Debug/Release
captures.

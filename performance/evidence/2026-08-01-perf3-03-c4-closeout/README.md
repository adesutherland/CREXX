# PERF3-03 C4 production closeout

Date: 2026-08-01

Status: accepted Apple ARM64 production closeout complete. Local commit and push
are not included. Windows/MSVC validation remains an explicit pre-publication
follow-up because no Windows cross-toolchain is installed on this host.

## Retained result

Adrian accepted the favourable first ordinary Release verdict for the private
locale-aware loose-comparison prefilter. The retained timing authority remains
[`../2026-08-01-perf3-03-c4-first-release-verdict/`](../2026-08-01-perf3-03-c4-first-release-verdict/):

- Base64: +4.859%/+5.780% on `rxvm`/`rxbvm`;
- canonical RexxCPS: +2.517%/-0.609%; and
- no -3% per-workload regression guard hit.

## Closeout validation

- Complete Debug build: pass, 1,096 actions.
- Full Debug CTest: 1,972/1,972 pass in 184.60 seconds with `--parallel 30`.
- ASan build of both VM variants: pass.
- Focused ASan logic/conversion gate: 6/6 pass with `detect_leaks=0`.
- Leak-enabled attempt: not runnable on this macOS ASan runtime; it aborts
  before project code with `detect_leaks is not supported on this platform`.
  This is retained as an explicit LSan coverage gap, not reported as a pass.
- Complete ordinary profiling-off Release build: pass, 1,472 actions.
- Isolated Release install: pass, 136 files.
- Installed `rxvm` and `rxbvm` Base64 smoke: 2/2 pass with empty stderr.
- Installed VM hashes exactly match the accepted first-verdict binaries.

The VM architecture reference now documents the private fail-closed numeric
prefilter, exact-converter fallback and no-inline requirement. No public opcode,
RXAS/RXBIN, ABI, signal, JSON-specific or language-contract change is included.

## Platform boundary

This Mac has no `clang-cl`, MinGW compiler or Zig cross-toolchain. The dedicated
MSVC `__declspec(noinline)` spelling is present in production source, but a real
Windows build/test remains required before publication. That follow-up must run
the focused logic/conversion tests in both VM variants and confirm the material
Base64/RexxCPS behavior without reopening C4 design.

## Evidence map

- `logs/debug-*.log`: complete Debug build and 1,972-test result;
- `sanitizer/`: sanitizer build, unsupported leak-on attempt and passing ASan
  rerun;
- `logs/release-*.log`: complete Release build and isolated install;
- `logs/installed-*.txt`: installed VM smoke output;
- `provenance/release-install-identity.txt`: hashes, sizes and local toolchain
  boundary;
- `provenance/install-inventory.txt`: all 136 installed files; and
- `checksums.sha256`: bundle integrity.

No broad timing was rerun after acceptance; the accepted first-verdict samples
remain the performance authority. No commit or push was made.

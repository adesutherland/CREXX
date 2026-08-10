# E3b-P2 first Release verdict host

- Capture started: 2026-08-10 21:01:24 BST (20:01:24 UTC).
- Host reservation: Adrian reported `Host clear` immediately before capture.
- Host: Apple M5, Darwin arm64 25.5.0, 10 logical CPUs.
- Power: AC attached; battery 80%, not charging.
- Initial load averages: 1.16, 1.48, 1.44.
- Ordinary Release cache: `CMAKE_BUILD_TYPE=Release`,
  `CREXX_VM_PROFILING=OFF`, `CREXX_VM_HANDLER_PANEL=profile-20`.
- Serial pairwise-balanced schedule; no overlapping build, test or second
  measurement process.

The Codex host process was the only material foreground process at capture
(11.6% instantaneous CPU); the benchmark is run by one child at a time.

- Timed capture: 2026-08-10 20:03:35–20:05:48 UTC.
- End load averages: 1.52, 1.42, 1.41.
- End power: AC attached; battery 80%, not charging.

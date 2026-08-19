# Stage 5 pre-run provenance

Captured: 2026-08-18 22:16 Europe/London

- Adrian explicitly confirmed the host clear for continuation through Stage 5.
- Host: Apple M5, 10 logical CPUs, macOS 26.5.2 (25F84), Darwin 25.5.0.
- Power: AC attached, battery 80%, low-power mode 0.
- Pre-run load averages: 1.35, 1.54, 3.40.
- The visible CPU users were the Codex/ChatGPT processes running this task and
  WindowServer; no separate build, test or benchmark process was active.
- Frozen cREXX source: `81f15918676d92a7d3e88954d94779ed759e9db8`.
- cREXX build: fresh `Release`, AppleClang 21.0.0.21000101,
  `CREXX_VM_PROFILING=OFF`.
- Canonical AWFY source: fresh clean checkout at
  `74306fec151070fd07157cefeacf19e7e0bcdc89`.
- Java: Temurin OpenJDK 26.0.1+8, fresh 92-class canonical build.
- CPython: 3.14.6 (`/opt/homebrew/bin/python3`).
- ooRexx: 5.1.0 r12973; `REXX_PATH` points to the same distribution's `bin`
  directory so `json.cls` resolves.
- Regina: 3.9.7.
- NetRexx: 5.10-GA build 18-20260320-1410, freshly translated and compiled
  genuine capability sources; runtime `NetRexxR.jar` comes from the same
  installation.

The exact-work two-observation qualification passed all 89 cells before formal
timing. The scorecard uses two warmups and ten retained observations per cell,
serially, through the maintained Level B matrix runner.

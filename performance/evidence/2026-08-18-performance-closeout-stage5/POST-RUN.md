# Stage 5 post-run host record

Captured: 2026-08-18 23:16 Europe/London

- Host: Apple M5, 10 logical CPUs, macOS 26.5.2 (25F84), Darwin 25.5.0.
- Power: AC attached, battery 80%, low-power mode 0.
- Post-run load averages: 1.25, 1.33, 1.40.
- The only material visible CPU consumers at capture were the Codex/ChatGPT
  processes conducting this work and WindowServer. No benchmark, build or test
  process remained active.
- No passing sample was removed. The single permitted timing, RSS and lifecycle
  appends were retained with their initial samples.

The pre-run reservation and runtime/build provenance are in `PRE-RUN.md`.

# NR-26 closeout drift-control host state

- Host: `Mac.lan`, Darwin 25.5.0, ARM64 T8142.
- Product: ordinary `Release`, `-O3 -DNDEBUG`, profiling off.
- Immediate pre-start: 2026-07-22T07:45:43Z; AC attached, battery 80%,
  low-power mode 0, and `pmset -g therm` reported no thermal, performance or
  CPU-power warning. No build, CTest, benchmark, `rxvm` or `rxbvm` process was
  active. One-minute load average was 2.18; the earlier broad-build load was
  decaying and current non-window-manager processes were below 13% CPU.
- Capture: 2026-07-22T07:46:26Z to 2026-07-22T07:51:21Z; serial rotated
  sampling, one warmup and 12 recorded pairs per VM.
- Immediate post-capture: 2026-07-22T07:51:58Z; AC attached, battery 80%,
  low-power mode 0, no `pmset` warning, one-minute load average 1.36, and no
  competing process above 12% CPU.

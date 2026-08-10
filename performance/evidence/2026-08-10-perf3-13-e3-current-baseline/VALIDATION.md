# Validation record

- Source before timing: clean synchronized `develop` at `6d12cd921`.
- Build: ordinary `Release`, Apple Clang 21.0.0,
  `CREXX_VM_PROFILING=OFF`, `CREXX_VM_HANDLER_PANEL=profile-20`.
- Product mapping: `rxvm -> rxbvm`; both `rxbvm` and `rxtvm` measured.
- Initial matrix: 14 cells, 28 warmups and 140 recorded samples; 168/168
  processes pass with zero non-zero exits.
- Governed append: four mechanically selected cells, 40/40 recorded processes
  pass; zero warmups, removed samples or second append.
- Merged summary: stable cells remain n=10 and appended cells are n=20.
- Runner stderr: empty for initial, append and summary merge.
- Environment: AC power, low-power mode off, no recorded thermal/performance/
  CPU-power warning and no overlapping build/test/VM process.
- Identity: pre/post source, VM, library, workload, manifest and runner hashes
  are identical.
- Recursive bundle checksum audit: all 22 retained non-checksum files pass.
- Interpretation: formal same-host absolute observation only; noisy cells stay
  labelled and a future candidate needs a same-session paired verdict.

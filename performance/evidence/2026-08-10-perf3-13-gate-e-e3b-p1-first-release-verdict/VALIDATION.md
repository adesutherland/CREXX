# E3b-P1 validation

## Pre-fix reproducers

- Static replay reproduced the destructive registration queue: the first VM
  found the constructor-registered procedure and the second did not.
- Two concurrent unmarked native calls overlapped, proving that the legacy ABI
  had no compatibility serialization.

## Focused Debug

- 7/7 E3b concurrency/ownership tests passed: simultaneous static replay,
  legacy serialization, process-reentrant overlap, recursive re-entry,
  valid/invalid dynamic manifests and two-context dynamic DSO lifetime.
- 24/24 existing static/dynamic compiler, runtime, payload and unwind cases
  passed.
- The installed external C and C++ SDK consumer passed 1/1.
- `ts_regvalue_tester` and `rxpa_utf_validation` passed 2/2.

## Frozen Release

- The same E3b concurrency/ownership panel passed 7/7.
- Both call-kernel images returned the expected count under control and
  candidate `rxbvm`/`rxtvm` before timing.
- The formal timing capture passed 312/312 processes: 24 warmups and 288
  recorded executions. `capture-manifest.json` reports `result: pass` and all
  sample rows report `correctness=pass`.

This is the minimum correctness set required for the first Release verdict.
Full Debug CTest, sanitizer, cross-platform, install/package and P2 work remain
closed because the hot primitive failed.

# NR-14 accepted-hybrid QA closeout

Status: **complete with one documented platform limitation; no product or
performance regression remains.**

Adrian accepted the hybrid performance verdict and explicitly approved the
`rxcexits.rxbin` +249,179 B/+20.579% artifact trade-off on 2026-07-21. The
formal timing bundle was retained unchanged and was not rerun during closeout.

## Cleanup and documentation

- Removed all three disposable NR-14 PoCs; the retained raw comparison samples
  remain in the parent bundle.
- Documented `parsewords3`, `parsepos2`, `parsewords3d`, `parseplan`, the
  RXBIN 007 feature gate, compact descriptor, aliasing and fallback contracts.
- Completed the previously missing `call1` through `call4` reference headings;
  the live `rxas -i` surface and reference now agree at **375/375 mnemonics**
  and **582 forms**.
- The executable RXAS reference examples pass on `rxvm` and `rxbvm`.

## Debug validation

- Complete Debug rebuild passed, including all affected generated products.
- Focused PARSE/RXBIN/reference selection passed **15/15**.
- Full Debug CTest passed **1,876/1,876** in 166.48 seconds with
  `--parallel 30`.

The focused selection was the parent bundle's 13-test expression plus
`rxasreferencetests` and `rxasreferencetests-rxbvm`.

## Sanitizer

The first required leak-on build attempt stopped because Apple AddressSanitizer
reports `detect_leaks is not supported on this platform`; this is a platform
capability limitation, not an NR-14 diagnostic. The supported rerun used:

```sh
tools/asan-run.sh --phase build --build-jobs 10 --build-leaks off
```

The complete 1,204-step ASan product build passed. The same focused 15-test
surface then passed **15/15** with `--leaks off --test-jobs 1`. AddressSanitizer
remained enabled throughout; LeakSanitizer coverage is not claimed on this
macOS host. Runner records were
`20260721-110516-build`, `20260721-110538-build`, and
`20260721-111406-ctest` under `cmake-build-debugasan/asan-logs/`. The final
documentation-only reference rerun also passed 2/2 in Debug and ASan
(`20260721-112120-ctest`).

## Release install and native packaging

- The ordinary profiling-off Release candidate (`Release`, profiling off)
  completed its full 448-step rebuild.
- This CMake tree exposes install targets but no CPack/package target, so the
  isolated installed-tree proof is the applicable package gate.
- `cmake --install` succeeded into a fresh prefix containing **131 files**.
- The installed `crexx -native` path compiled and packaged installed
  `hello.crexx`; the resulting Mach-O executable printed `4711` and
  `hello CREXX world!`.
- The installed `rxc`, `rxas`, and `rxlink` compiled and linked
  `parse_frozen_generic.crexx` against the installed library. Installed `rxvm`
  and `rxbvm` each produced checksum 194 and `PASS: Generic Frozen PARSE`.

## Boundary

All closeout work was run on the recorded Apple M5 Darwin arm64 host. No
cross-platform result is inferred. Regex replacement and a link/load private
prepared representation remain outside NR-14 until a concrete requirement
exists.

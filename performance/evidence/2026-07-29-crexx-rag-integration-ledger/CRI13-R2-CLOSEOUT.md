# CRI-13 Option B R2 closeout

Status: **accepted and proportionally closed**

Date: 2026-07-30

Governed ID: `CAP-01-J02`

Adrian accepted frozen `lib/rxfnsb/rexx/rxjson.crexx` SHA-256
`7916d23df7cc488adfee54d4b25e504fa3afd47a8746fb7d15a6f401bde83d77`.
The favorable first Release evidence remains in
[`CRI13-R2-RELEASE-VERDICT.md`](CRI13-R2-RELEASE-VERDICT.md).

Proportional closeout results:

| Gate | Result | Raw evidence | SHA-256 |
| --- | --- | --- | --- |
| Complete Debug build | pass; clean no-op replay | `/tmp/cri13-r2-closeout-debug-build.XXXXXX.log` | `402123c2dee25f9779e3d39e66fae83635926a067ad2774742c0e8d07e9f6bd2` |
| Complete Debug CTest | 1,963/1,963, zero failures, no reported skips | `/tmp/cri13-r2-closeout-debug-ctest.log` | `501b749612972ae5cfafdbad2523c26ed48216bc862c2fae534ab0c8ec70e081` |
| Affected ASan build | pass | `cmake-build-debugasan/asan-logs/20260730-180434-build/build.log` | `c5b507fca587cbbd61b6633b0012c873d585ffb61de9ed692065b743cc2c8f1a` |
| Affected ASan CTest | 17/17 including fixture, zero failures | `cmake-build-debugasan/asan-logs/20260730-180747-ctest/ctest.log` | `3cffd3e531e2ece09891cba81eab452162ee478a0f10fbdc80109ac580974c99` |

The ASan runner used `detect_leaks=0` because this Apple sanitizer has already
proved that LeakSanitizer is unsupported; AddressSanitizer remained enabled.
The affected matrix covers optimized/non-optimized JSON compatibility,
document, noisy scanner and numeric projection tests, including both VMs for
the CRI-09/13 document surfaces.

The maintained source-adjacent and library-reference JSON documents now state
the explicit headerless projection, expected-count policy, byte sizes, status
codes, f32 underflow/overflow and signal translation, i64 integral behavior,
and the absence of inference, headers, metadata or hidden caching.

No public ABI, RXAS/RXBIN, serialized format or general numeric-conversion
contract changed. `PERF2-07-C01` remains queued and inactive.

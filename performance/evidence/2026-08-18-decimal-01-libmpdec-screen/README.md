# DECIMAL-01 libmpdec 4.0.1 screen

Date: 2026-08-18

Status: **D4 rejected at L1; no L2/L3 or production integration**

## Candidate and boundary

- Official mpdecimal 4.0.1 archive SHA-256:
  `96d33abb4bb0070c7be0fed4246cd38416188325f820468214471938545b1ac8`.
- Upstream licence: BSD-2-Clause.
- External build: Apple ARM64, Apple Clang, static `CONFIG_64` library with
  `ANSI` and `HAVE_UINT128_T`; no candidate source is vendored here.
- Upstream `make check_local` passed its 15 local static-library test files.
  The downloadable extended corpus was not run because the upstream helper
  required unavailable `wget`.
- cREXX build: ordinary profiling-off Release with the candidate explicitly
  enabled. The normal build remains off by default.

The existing cREXX decimal payload is raw byte-copied and has no decimal copy
or destructor hook. Native `mpd_t` therefore cannot be stored directly because
it contains a data pointer. The adapter stores pointer-free metadata and inline
coefficient words in the VM sidecar, constructs temporary `mpd_t` views per
operation, and commits pointer-free result metadata. This preserves the
existing ABI and lifecycle at the cost of extra view/commit work.

The existing full decimal-provider contract passed with the candidate,
including integer/double conversion, formatting and signed zero, arithmetic,
power, negation, comparison, extraction, truncate and round. Common-18 and
Classic-9 arithmetic, conversion, comparison and context-sync semantic
checksums matched `mc_decimal`. The copy/clear checksum intentionally includes
representation bytes and differs because payload sizes differ. A dedicated
test also passed raw copying, source destruction, formatting the copy and
in-place arithmetic.

## Formal L1 result

The formal V0 adapter screen used one warmup and 12 serial,
pairwise-balanced/interleaved recorded rounds per cell. All 260 samples passed
their correctness string; no observation was removed. Pre/post state records
AC power, low-power mode off, no thermal/performance warning and no competing
benchmark process.

| Mode | Context | `mc_decimal` median | libmpdec median | libmpdec throughput delta |
| --- | --- | ---: | ---: | ---: |
| arithmetic | Common-18 | 2.689919 s | 4.610413 s | **-41.66%** |
| arithmetic | Classic-9 | 3.389074 s | 4.597471 s | **-26.28%** |
| conversion | Common-18 | 1.042729 s | 1.808445 s | **-42.34%** |
| conversion | Classic-9 | 1.042509 s | 1.816470 s | **-42.61%** |
| comparison | Common-18 | 2.389378 s | 1.532318 s | +55.93% |
| comparison | Classic-9 | 2.370603 s | 1.523020 s | +55.65% |
| copy/clear | Common-18 | 1.353855 s | 1.264570 s | +7.06% |
| copy/clear | Classic-9 | 1.341450 s | 1.240559 s | +8.13% |
| context sync | Common-18 | 1.401823 s | 1.453277 s | -3.54% |
| context sync | Classic-9 | 1.372776 s | 1.487051 s | -7.68% |

The Classic-9 current-provider sync cell is marked `rerun_recommended` in the
raw summary and is not needed for the decision. Only comparison clears the 15%
improvement threshold. The required two representative L1 modes do not, while
arithmetic and conversion have large adverse results.

The formal candidate binary SHA-256 was
`1a2dd4757f1bed778836f959320de9ff686e59d480e3bc5d38069cf5721a5fe9`.
After this formal screen, the implementation was tightened to cache a smaller
safe operation capacity for 9/18-digit contexts; the performance-tested V1
binary SHA-256 was
`3c76832a2a83229230223e5a837130a2d374ed47d320d226e2a678ec42b5787c`.
V1 was not substituted into the formal evidence.

## Arithmetic implementation double-check

The V1 adapter and an initial direct C library comparator ran one warmup plus
four balanced diagnostic observations per cell. All 40 samples passed. A
final direct-core repeat matched libmpdec's result capacity to the two words
used by the lean adapter; its 20 samples also passed. These are bounded
attribution checks, not a replacement formal screen.

| Layer | Context | Current median | libmpdec median | libmpdec throughput delta |
| --- | --- | ---: | ---: | ---: |
| lean cREXX adapter | Common-18 | 2.709327 s | 4.372713 s | **-38.04%** |
| lean cREXX adapter | Classic-9 | 3.274959 s | 4.404989 s | **-25.65%** |
| matched-capacity direct core | Common-18 | 2.292583 s | 2.935603 s | **-21.90%** |
| matched-capacity direct core | Classic-9 | 2.865309 s | 3.025756 s | **-5.30%** |

The initial fixed-eight-word direct diagnostic produced -22.26%/-6.61%; the
matched-capacity repeat above rules that allocation choice out as the reason
for the result. It confirms that libmpdec itself is slower than the current
decNumber 3.68 core on this exact mixed add/subtract/multiply/divide kernel,
especially at Common-18. The larger adapter loss is consistent with the
temporary-view and result-commit work required to place libmpdec's
pointer-based native representation behind cREXX's raw-copy value contract.

After timing, constructor initialization was hardened to install the cREXX
default numeric context even before the host's normal synchronization call.
That validation-only binary SHA-256 is
`9594c881e2a1709c6650f4afcaec4a4880d445c1b64322b063cd885a9020906c`.
The change is outside the measured steady-state operation path; the full
contract, lifecycle and ten adapter qualification rows pass after rebuilding
it.

The current lean candidate plugin is 203,848 bytes versus 126,088 bytes for
the current plugin, a 61.7% larger loadable artifact; the external static
`libmpdec.a` is 222,424 bytes.

## Decision

D4 fails the predeclared L1 progression rule and has no credible route to a
10% L3 improvement without changing the current value/ABI contract. Stop at
L1, retain `mc_decimal`, and do not run L2/L3. This is a candidate rejection,
not a claim that libmpdec is generally slow or unsuitable for other APIs,
precisions, workloads or hosts.

Raw formal data: [`l1/summary.csv`](l1/summary.csv). Raw double-check data:
[`double-check/summary.csv`](double-check/summary.csv). Matched-capacity
direct-core data:
[`core-capacity-check/results/summary.csv`](core-capacity-check/results/summary.csv).
Manifests:
[`decimal-01-libmpdec-l1-mac-v1.txt`](../../../manifests/decimal-01-libmpdec-l1-mac-v1.txt),
[`decimal-01-libmpdec-arithmetic-double-check-mac-v1.txt`](../../../manifests/decimal-01-libmpdec-arithmetic-double-check-mac-v1.txt),
and the final
[`decimal-01-libmpdec-core-capacity-check-mac-v1.txt`](../../../manifests/decimal-01-libmpdec-core-capacity-check-mac-v1.txt).

# PERF2-11 Windows x86-64 scorecard

Status: **formal Windows baseline complete; no profiling or tuning performed**

This bundle retains the Windows 11 x86-64 correctness, cross-runtime timing,
peak working-set, lifecycle and artifact scorecard for source commit
`d78c6fcfa81e` plus the exact local test/tool portability patch in
`provenance/final-working-tree.patch`.

The host was an Acer Aspire A515-56G with an Intel Core i5-1135G7, 4 physical
cores/8 logical processors and 20 GiB RAM. It remained on AC power at 100%
charge under the Acer power scheme. Pre/post snapshots show 4%/5% CPU load and
no competing build or benchmark runtime processes.

No compiler, VM, RXAS or language behavior was changed. No profiler, native
counter, candidate experiment or performance analysis was run on this host.
The consolidated result is in [`SCORECARD.md`](SCORECARD.md); raw CSV and log
files remain authoritative.

## Regression closure

The 15 local Release failures had two repository-level causes:

1. Six CMake-script tests decoded UTF-8 child output through the Windows OEM
   code page. The process results were correct, but characters such as
   `Ä`, CJK text and emoji were mojibake before comparison. The affected
   `execute_process` calls now specify `ENCODING UTF-8`.
2. Nine driver/native tests inherited the user's `CREXX_HOME`, so build-tree
   `crexx.exe` selected stale installed tools and libraries from
   `C:\Users\adria\.local`. The late-interface driver and all bin-driver tests
   now pin `CREXX_HOME` to the active CMake binary directory.

The Codex PowerShell session also lacked CLion's MinGW directory on `PATH`,
which produced `rc=127` while reproducing native-driver failures. That was a
tool-session environment issue, not a repository regression; all verification
commands explicitly used the configured CLion toolchain path.

After the harness corrections:

| Gate | Result |
| --- | ---: |
| Previously failing focused set | 15/15 |
| Full profiling-off Release CTest | 1,926/1,926 |
| Full CTest elapsed | 565.48 s |

## Runtime freeze

Portable comparators are workspace-scoped under the ignored
`.tmp-windows-runtimes` directory:

| Runtime | Version |
| --- | --- |
| ooRexx | 5.1.0 r12973, Windows x86-64 portable |
| Regina | 3.9.7 MT, Windows x86-64 |
| NetRexx | 5.10-GA build 18-20260320-1410 |
| Java | Temurin OpenJDK 26.0.1+8, 64-bit HotSpot |

Download, executable, JAR and generated-class SHA-256 identities are retained
under `provenance/`. NetRexx common cells use `options nobinary decimal`,
NetRexx `Rexx` numeric state and the default HotSpot JIT.

## Capture policy

- common aggregate: Sieve 5500, Permute 5000, Bounce 4200, Richards 20 and
  Base64 2500;
- timing: two warmups and ten recorded samples, serial and runtime-rotated by
  workload;
- variability: eleven initially flagged cells received ten additional
  recorded samples under the same policy;
- RSS: zero warmups and three recorded child-process `PeakWorkingSet64`
  samples;
- lifecycle: 20 samples per compile/assemble/translate/load phase; and
- Towers and RexxCPS remain separate from the common aggregate.

The first MSYS2 GNU `time` RSS attempt is retained under
`rss-msys-diagnostic-invalid/`. Its uniform approximately 6.1 MB values
measured the compatibility shim rather than native child processes and do not
enter the scorecard.

## Directory map

| Path | Content |
| --- | --- |
| `timing/` | Initial formal timing capture |
| `timing-noise-append/` | Policy-required samples for 11 flagged cells |
| `timing-final/` | Authoritative merged timing summaries and ratios |
| `rss/` | Valid Windows-native peak working-set capture |
| `lifecycle/` | Compile/translate and load-to-first-result samples |
| `generated/netrexx/` | Exact staged source, generated Java and class files |
| `artifacts.csv` | 90 product/source/generated-runtime size and hash rows |
| `manifests/` | Formal and variability-append matrix manifests |
| `provenance/` | Host, power, source, build and runtime identities |
| `logs/` | Build, CTest, compiler and capture logs |
| `checksums.sha256` | Recursively verified bundle checksum closure |

This Windows baseline does not close PERF2-11 Gate E. Supported Linux ARM64
evidence and a selected cross-platform candidate/default-VM decision remain
open.

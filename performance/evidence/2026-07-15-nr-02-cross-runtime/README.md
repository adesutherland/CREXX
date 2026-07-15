# NR-02 cross-runtime qualification and pilot evidence — 2026-07-15

Status: capture in progress; not an NR-10 formal baseline

This bundle retains qualification and pilot evidence for the resumable NR-02
matrix in `performance/NR-02-WORKLIST.md`. Results remain incomplete and must
not be aggregated across runtimes.

Detailed ledgers:

- `../benchmark-median-summary.md` — master per-date/run platform medians and
  exclusion policy;
- `rexxcps/README.md` — RexxCPS version, equivalence, challenge and trace work;
- `seed-workloads.md` — Sieve, Permute, Mandelbrot and Towers ports, generated
  forms, substitutions, pilots and final ooRexx dispositions.

## Source state

- Branch: `develop`
- Commit: `44dd4dbf3da624e2d8e79eccf696f78f023d436e`
- Worktree: intentionally dirty with the performance workspace, root
  performance guidance, serial-sample runner support, and NR-02 additions
- Host: Apple M5, 10 logical CPUs; Darwin 25.5.0 arm64; macOS 26.5.2
- Sampling policy: serial; correctness before timing; pilot samples are not a
  formal repeated baseline
- CREXX build: CMake 4.3.2, Ninja 1.13.2, `Release`, Apple clang 21.0.0,
  `-O3 -DNDEBUG`, `CREXX_VM_PROFILING=OFF`
- CREXX image mode: optimized workload RXBIN plus separate `library.rxbin`;
  source/TRACE metadata retained; process startup and module load included
- NetRexx build: 5.10-GA compiler, default Java 8 class-file target, generated
  Java retained; JVM defaults and exact argv recorded with the pilots

## Runtime and compiler inventory

| Component | Version / state | Invocation or path |
| --- | --- | --- |
| Java runtime | Temurin OpenJDK 26.0.1+8, 64-bit Server VM | `/usr/bin/java -version` |
| Java compiler | `javac 26.0.1` | `/usr/bin/javac -version` |
| Java disassembler | JDK 26.0.1 | `/usr/bin/javap` |
| Regina | REXX-Regina 3.9.7 / Regina 3.9.7(MT), 2025-03-18 | `/opt/homebrew/bin/regina -v` |
| ooRexx | 5.1.0 r12973, official universal macOS portable build | `/Users/adrian/.local/opt/oorexx/5.1.0-12973/bin/rexx` |
| NetRexx | 5.10-GA build 18-20260320-1410; compile/run proof passed | `/Users/adrian/.local/opt/netrexx/5.10-GA` |
| CREXX | beta 3 WIP dirty build from the source state above | `cmake-build-release/bin/crexx` and `rxvm` |

## NetRexx installation record

- Official release page: <https://www.netrexx.org/downloads.nsp>
- Official archive: <https://www.netrexx.org/files/NetRexx-5.10-GA.zip>
- Release: NetRexx 5.10-GA, 2026-03-20
- Downloaded archive:
  `/Users/adrian/.local/cache/netrexx/NetRexx-5.10-GA.zip`
- Locally computed SHA-256:
  `ede777b7277806451ff406d89a64eacca2e187bef9f3b0d32297af4437c86976`
- Published checksum: none found on the official download page; the value
  above is a local integrity record, not an upstream signature
- Installation path: `/Users/adrian/.local/opt/netrexx/5.10-GA`
- Package license: bundled `LICENSE` (ICU license); third-party/example source
  provenance is recorded separately per workload
- JDK used: Temurin OpenJDK/JDK 26.0.1
- Compiler invocation:
  `PATH=/Users/adrian/.local/opt/netrexx/5.10-GA/bin:$PATH nrc ...`

The minimal compile/run proof is retained in `netrexx-install/`. NetRexx
generated `hello.java.keep` and `hello.class`; the class is version 52.0
(Java 8) despite compilation on JDK 26, matching the 5.10 default described by
the release notes. `hello.javap.txt` retains the bytecode inspection.

## ooRexx installation record

- Official release page: <https://www.oorexx.org/downloads.rsp>
- Official portable package: <https://sourceforge.net/projects/oorexx/files/oorexx/5.1.0/portable/oorexx-5.1.0-12973.macos.arm64.x86_64-portable-release.zip/download>
- Release/build: ooRexx 5.1.0 r12973, built 2025-05-02
- Locally computed SHA-256:
  `7d81e2ad65cdd2155cbe27fe6d948d1937e72a0e10099a7b409c79315e870770`
- Published checksum: none found alongside the official portable download
- Installation path: `/Users/adrian/.local/opt/oorexx/5.1.0-12973`
- Package license: Common Public License 1.0 and bundled redistribution terms
- Invocation: the absolute versioned `bin/rexx`; Homebrew's
  `/opt/homebrew/bin/rexx` remains Regina 3.9.7

The user-local installation and minimal `PARSE VERSION` program proof are
retained in `oorexx-install/`. No shell startup file, Homebrew package, or
repository binary was changed.

## Evidence index

| Area | State | Files |
| --- | --- | --- |
| NetRexx installation proof | pass | `netrexx-install/proof.txt`, generated source/class and `hello.javap.txt` |
| RexxCPS CREXX/Regina/NetRexx qualification | pass for available runtimes | `rexxcps/README.md` and runtime subdirectories |
| RexxCPS ooRexx qualification | pass | canonical/A/B pilots, translated images and count-one trace under `rexxcps/oorexx-*` |
| Sieve CREXX/ooRexx/NetRexx qualification | pass | `sieve/crexx/`, `sieve/oorexx/`, `sieve/netrexx/` |
| Permute CREXX/ooRexx/NetRexx qualification | pass | `permute/crexx/`, `permute/oorexx/`, `permute/netrexx/` |
| Mandelbrot CREXX qualification | pass | `mandelbrot/crexx/` |
| Mandelbrot NetRexx qualification | disclosed adaptation; review | `mandelbrot/netrexx/` |
| Mandelbrot ooRexx qualification | not comparable | retained size-500/750 correctness failures and translated image under `mandelbrot/oorexx/` |
| Towers CREXX/NetRexx qualification | pass for typed/object pair | `towers/crexx/`, `towers/netrexx/` |
| Towers ooRexx diagnostic adaptation | not comparable | correct procedural pilot and translated image under `towers/oorexx/`; object allocation/dispatch is not preserved |

## Interpretation boundary

This is an in-progress qualification bundle. A passing pilot demonstrates one
source/runtime cell under the recorded command; it does not establish a stable
ratio, geometric mean, release threshold, or whole-runtime ranking. Startup-
inclusive process time, steady-state internal timing, benchmark-native metrics,
and metadata modes remain separate.

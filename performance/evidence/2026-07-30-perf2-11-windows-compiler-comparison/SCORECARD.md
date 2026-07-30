# Windows GCC/Clang and cross-platform RexxCPS review

Status: **complete bounded comparison; no Windows profiling or production
compiler change**

Higher ratios are better. Cross-runtime ratios use native RexxCPS medians from
each retained session. The controlled Windows compiler ratios use the same
RXBIN and library inputs in one rotated formal matrix.

## Executive result

The Windows RexxCPS ratio difference is real, reproducible and not explained by
an accidental Debug build or by choosing MinGW GCC instead of Clang:

- the current GCC session reproduces the Windows baseline within 1% for cREXX,
  ooRexx and Regina;
- Windows Clang changes RexxCPS by only `0.968556x` for `rxvm` and `1.029856x`
  for `rxbvm` relative to GCC;
- on the same recorded i5-1135G7 CPU model, Windows GCC cREXX reaches
  `0.691627x`/`0.658918x` of Linux GCC `rxvm`/`rxbvm`, while Windows ooRexx and
  Regina reach `1.345243x` and `1.203222x` of their Linux rates; and
- the exact macOS/Linux NetRexx class is `2.385008x` faster on the Windows
  JVM/OS lane than in the retained Linux GCC session.

The cREXX/ooRexx ratio therefore moves from about `1.17x` on Linux to about
`0.60x` on Windows because the numerator and comparator move in opposite
directions. It is not a single cREXX compiler effect.

## Retained RexxCPS medians

| Platform/compiler | `rxvm` clauses/s | `rxbvm` clauses/s | ooRexx | Regina | NetRexx | `rxvm`/ooRexx | `rxbvm`/ooRexx |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| macOS AppleClang 21 | 37,928,815.5 | 35,545,839.5 | 38,090,552.0 | 32,157,585.5 | 46,030,212.0 | 0.995754x | 0.933193x |
| Linux GCC 15.2 | 13,716,335.5 | 13,312,358.0 | 11,669,765.5 | 12,502,519.5 | 7,980,203.5 | 1.175374x | 1.140756x |
| Linux Clang 21.1.8 | 13,817,652.0 | 13,864,294.5 | 11,861,170.0 | 12,646,678.0 | 7,735,307.0 | 1.164948x | 1.168881x |
| Windows GCC 15.2 baseline | 9,441,205.5 | 8,759,043.5 | 15,785,329.5 | 15,192,167.0 | 16,904,199.5 | 0.598100x | 0.554885x |
| Windows GCC 15.2 current | 9,486,594.5 | 8,771,750.0 | 15,698,674.0 | 15,043,302.5 | 19,032,852.5* | 0.604293x | 0.558757x |
| Windows Clang 22.1.8 | 9,188,302.0 | 9,033,640.0 | 15,698,674.0 | 15,043,302.5 | 19,032,852.5* | 0.585292x | 0.575440x |

`*` The current Windows NetRexx value is a separate exact-class control using
the retained macOS/Linux Java 8 class. It is not part of the rotated 32-cell
compiler matrix. macOS uses different Apple M5 ARM64 hardware, so its absolute
rates do not isolate an operating-system effect. Linux and Windows were also
separate boot/session captures.

The cREXX optimized RexxCPS RXBIN is byte-identical across all three platforms:
`208874392b8f4b41b5627f7e6ddb97a5d79ad6c586d4ab1b8a0d3b3bd6a136c2`.
The cross-platform difference is therefore below the compiler/assembler layer.

## NetRexx control

The original Windows baseline used the NetRexx Windows launcher and ECJ. Its
Java 8 class hash differs from the class produced by the macOS/Linux `javac`
pipeline, so the initial class-compiler hypothesis was reasonable.

The follow-up executed the byte-identical retained macOS/Linux class on Windows:

| Identity/result | Value |
| --- | --- |
| Java class SHA-256 | `9bd2902f46a4640cd37616b3f545e5d626df03d71a0fcbb7c82ae3ee032b1eab` |
| Class-file major | 52 (Java 8) |
| `NetRexxR.jar` SHA-256 | `3285e5daa1128474278babdcb6896df6ad5c7ba3a4d9371ed0a19ed3b854af7c` |
| Windows median | 19,032,852.5 clauses/s (`n=20`, MAD 1.14%) |
| Linux GCC-session median | 7,980,203.5 clauses/s (`n=20`) |
| Windows/Linux | 2.385008x |

This excludes ECJ-versus-`javac` class generation as the main explanation.
Both sessions report Temurin 26.0.1+8 and use the same class and runtime JAR,
but the JVM binaries and operating-system substrate necessarily differ. The
remaining difference belongs to the Windows JVM/OS lane; separating JIT,
runtime-library and OS effects would require JVM diagnostics or profiling,
which was deliberately not performed here.

The additional Windows direct-`javac` row in the raw matrix is diagnostic only.
It produced a Java 26 class (major 70), so it is not a valid cross-platform
class control and does not enter any conclusion.

## Windows GCC versus Clang

Both products are Release `-O3 -DNDEBUG`, profiling off, GNU Windows target/ABI,
the same CLion MinGW 15.2 sysroot and the exact same GCC-generated RXBIN/library
inputs. The common-five geometric mean is:

| VM | Clang/GCC geometric mean | Result |
| --- | ---: | --- |
| `rxvm` | 0.989609x | parity; Clang 1.0% lower |
| `rxbvm` | 1.178044x | Clang 17.8% higher |

| Workload | `rxvm` Clang/GCC | `rxbvm` Clang/GCC |
| --- | ---: | ---: |
| Sieve | 0.938393x | 1.097928x |
| Permute | 1.047970x | 1.323788x |
| Bounce | 0.910901x | 1.301444x |
| Richards | 0.924611x | 0.958812x |
| Base64 | 1.145919x | 1.250994x |
| Towers | 0.947768x | 0.949344x |
| RexxCPS | 0.968556x | 1.029856x |

The split is consistent with compiler sensitivity in dispatch/code layout:
`rxvm` uses GNU computed-goto threaded dispatch, while `rxbvm` is the
`NTHREADED=1` switch/bytecode VM. Clang substantially helps the switch VM on
Permute, Bounce and Base64, but not Richards, Towers or the threaded VM as a
class. This is a future experiment lead, not proof of a causal mechanism.

## Build differences

There is no evidence of a Windows configuration regression:

- Windows GCC and Clang use `-O3 -DNDEBUG`; `CREXX_VM_PROFILING=OFF`.
- GCC 15.2 matches the Linux GCC major/minor version.
- All cREXX rows use the same platform-neutral RXBIN and shared library image.
- The two Windows compiler products import the same system DLL set:
  `CRYPT32`, `KERNEL32`, `msvcrt`, `Secur32` and `WS2_32`.
- Neither Windows compiler lane uses `-march=native`; both target generic
  x86-64 through the GNU Windows ABI/sysroot.

Windows still differs normally from Linux at the native layer: PE/COFF and
Win64 ABI rather than ELF/System V, MinGW/MSVCRT rather than glibc, Windows
system/TLS objects and different compiler platform branches. The controlled
compiler swap shows that these native-platform differences, not the spelling
of `-O3` or a GCC-only code-generation problem, are where any future Windows
investigation must start.

To keep this slow host within the requested scope, the Clang tree built the
`rxvm` and `rxbvm` target dependency closure only. It did not rerun the full
Clang CTest suite. The supported GCC Release tree had already passed
1,926/1,926 CTests, and every Clang benchmark execution in the one-run pilot and
formal matrix passed its workload correctness gate.

The generated code footprint also differs sharply:

| Compiler | VM | Executable | PE `.text` |
| --- | --- | ---: | ---: |
| GCC 15.2 | `rxvm` | 3,636,842 B | 3,357,168 B |
| GCC 15.2 | `rxbvm` | 3,609,194 B | 3,332,784 B |
| Clang 22.1.8 | `rxvm` | 1,212,416 B | 1,016,992 B |
| Clang 22.1.8 | `rxbvm` | 1,204,224 B | 1,012,784 B |

The smaller Clang text and workload-dependent direction make code layout,
inlining and dispatch generation reasonable future inspection targets. They do
not justify changing the supported Windows compiler: RexxCPS is near parity,
the threaded VM common aggregate is near parity, and several rows regress.

## Measurement boundary

The formal compiler matrix completed 384/384 executions; its five flagged cells
completed another 60/60 executions. The exact-class control completed 24/24
executions across initial and append blocks. No sample was discarded.

Five compiler cells and the exact-class control still exceed the min/max span
policy after their required append. Their merged MAD values are 0.62-4.03% for
the flagged compiler cells and 1.14% for exact-class NetRexx. This is adequate
for the large directional conclusions above, but small deltas should not be
overinterpreted.

RexxCPS obtains its native rate from Level B `time("R")`, whose VM `MTIME`
instruction currently uses `gettimeofday`, `localtime` and wall-clock
microseconds. The implementation is shared, and roughly one-second trials make
timer resolution an unlikely explanation for a 31-34% native gap, but this is
not a monotonic timer. A future cross-OS validation should compare the reported
rate with an external monotonic elapsed/work measurement before attributing
small platform effects.

## Tuning handoff

1. Keep the Linux report as the tuning priority; it contains the actionable
   profiles and native counter attribution.
2. Treat Windows Clang's `rxbvm` gains as a compiler/code-layout clue to retest
   on a faster profiling host, especially Permute, Bounce and Base64.
3. Do not spend time on NetRexx ECJ versus `javac`; the exact-class result
   excludes that as the main cross-platform explanation.
4. Validate RexxCPS's internal wall-clock rate against external monotonic timing
   in a cheap, no-rebuild experiment before using small cross-OS deltas.
5. Return any Linux/macOS-selected VM change to the supported MinGW Windows lane
   for correctness and formal timing. This evidence does not select Clang as a
   production compiler and does not reopen profiling on this slow host.

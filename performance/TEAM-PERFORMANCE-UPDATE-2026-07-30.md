# Team performance update - Windows scorecard and platform split

Date: 2026-07-30

Scope: Windows x86-64 correctness and scorecard, followed by a bounded
Windows/Linux RexxCPS comparison. No Windows profiling or tuning was performed.

## Headline

The supported Windows Release build is clean: all 1,926 tests pass. On the
five-workload common score, cREXX is 2.36 times ooRexx speed under `rxvm` and
1.98 times under `rxbvm`. A ratio above 1.0 means cREXX is faster. This aggregate
is a geometric mean, which combines workload ratios without letting one large
result dominate the score.

- Sieve, Permute and Bounce are substantial cREXX wins.
- Richards and Base64 remain behind ooRexx.
- Towers and the separate RexxCPS diagnostic also remain behind ooRexx.
- Against NetRexx, the five-workload cREXX ratios are 0.75x for `rxvm` and
  0.63x for `rxbvm`.

`rxvm` jumps directly between instruction handlers; `rxbvm` selects each
instruction through a C `switch`. They run the same cREXX bytecode using
different VM dispatch methods.

## Linux versus Windows

RexxCPS reports nominal Rexx clauses completed per second. It is useful because
the same benchmark family runs under cREXX, ooRexx, Regina and NetRexx.

The Linux and Windows campaigns record the same Intel i5-1135G7 CPU model,
although they are separate operating-system boots and benchmark sessions:

| Runtime | Linux clauses/s | Windows clauses/s | Windows/Linux |
| --- | ---: | ---: | ---: |
| cREXX `rxvm` | 13.716m | 9.487m | 0.692x |
| cREXX `rxbvm` | 13.312m | 8.772m | 0.659x |
| ooRexx | 11.670m | 15.699m | 1.345x |
| Regina | 12.503m | 15.043m | 1.203x |
| NetRexx exact-class control | 7.980m | 19.033m | 2.385x |

This sends the cREXX/ooRexx `rxvm` ratio from 1.175x on Linux to 0.604x on
Windows. The change comes from both sides: cREXX is slower on Windows, while
ooRexx and Regina are faster. NetRexx moves even more strongly in the Windows
direction.

## What we learned

- The optimized cREXX RexxCPS bytecode is byte-for-byte identical on macOS,
  Linux and Windows. The difference is below the compiler/assembler layer.
- Linux and Windows GCC are both version 15.2 Release `-O3` builds with cREXX
  profiling disabled. There is no accidental Debug-build explanation.
- Repeating the Windows GCC run reproduced the original results within 1%.
- Rebuilding the Windows VMs with Clang changed RexxCPS by only -3.1% for
  `rxvm` and +3.0% for `rxbvm`. MinGW GCC code generation is not the main
  explanation.
- Running the exact Linux/macOS NetRexx Java class and runtime JAR on Windows
  was still 2.385 times the Linux result. The earlier ECJ-versus-`javac`
  difference is not the main NetRexx explanation.

The Java Virtual Machine uses a JIT, or just-in-time compiler, to convert active
Java methods into machine code while the program runs. This exact-class control
therefore points to the Windows JVM/operating-system lane.

The remaining difference is interesting but unexplained. Windows and Linux use
different native ABIs, executable formats and C runtime libraries. An ABI is
the low-level rule set for calls, registers and data layout; the C runtime
supplies basic services such as memory, strings and timekeeping. RexxCPS also
uses a wall-clock VM timer, so an external monotonic-timer cross-check would be
a useful cheap validation.

We deliberately did not profile on this slow Windows host. The Linux profiles
remain the evidence for selecting tuning work; selected changes should return
to Windows for correctness and scorecard validation.

## ooRexx compiler note

ooRexx can be built with Microsoft's compiler. In fact, its official Windows
instructions use Visual Studio C++, CMake and `nmake`, and the portable
ooRexx 5.1 binary measured here reports Microsoft linker version 14.36.

That makes the current Windows comparator very likely an MSVC build already.
This may contribute to ooRexx's result, but cannot explain the whole pattern:
Regina and exact-class NetRexx also improve substantially on Windows. Rebuilding
ooRexx with MSVC would repeat its normal Windows toolchain rather than provide a
new compiler control.

### cREXX MSVC follow-up

We subsequently built only cREXX's switch-based `rxbvm` with MSVC 19.44. In the
stable post-cooldown RexxCPS block it was 14.8% faster than MinGW GCC and 13.4%
faster than Clang, confirming that compiler/runtime choice contributes to the
Windows result. It still reached only 64.9% of ooRexx speed, so MSVC explains a
useful fraction rather than the platform reversal.

Official build instructions:
<https://sourceforge.net/p/oorexx/wiki/how-to-build-oorexx/>

Detailed measurements and qualifications:
[`evidence/2026-07-30-perf2-11-windows-compiler-comparison/SCORECARD.md`](evidence/2026-07-30-perf2-11-windows-compiler-comparison/SCORECARD.md).

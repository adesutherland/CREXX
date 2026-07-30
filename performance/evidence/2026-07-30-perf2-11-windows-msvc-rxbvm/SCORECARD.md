# Windows MSVC rxbvm compiler control

Status: **bounded compiler result; no production compiler selection**

Only the bare switch-dispatch `rxbvm` target and its native dependency closure
were built. All cREXX cells execute the same retained RexxCPS RXBIN
`208874392b8f4b41b5627f7e6ddb97a5d79ad6c586d4ab1b8a0d3b3bd6a136c2`
and the same GCC-generated `library.rxbin`.

## Headline

MSVC improves Windows `rxbvm` RexxCPS throughput by about 15% versus MinGW GCC
and 13% versus Clang in the stable cooldown block:

| Runtime/compiler | Median clauses/s | Relative MAD |
| --- | ---: | ---: |
| GCC 15.2 `-O3` | 8,992,204.5 | 0.70% |
| Clang 22.1.8 `-O3` | 9,099,789.5 | 1.39% |
| MSVC 19.44 `/O2 /Ob2` | 10,323,282.0 | 0.93% |
| ooRexx | 15,917,253.0 | 0.92% |
| Regina | 15,461,945.5 | 1.49% |
| exact-class NetRexx | 18,633,329.0 | 1.34% |

| Comparison | Ratio |
| --- | ---: |
| MSVC / GCC | **1.148026x** |
| MSVC / Clang | **1.134453x** |
| MSVC / ooRexx | **0.648559x** |
| MSVC / Regina | **0.667657x** |
| MSVC / exact-class NetRexx | **0.554022x** |

The compiler asymmetry is therefore a real contributor to the unusually strong
Windows ooRexx comparison. It is not the main explanation: even the MSVC build
remains 35.1% below ooRexx in the same stable block. The previous supported
GCC scorecard remains the Windows baseline.

For orientation only, the MSVC cooldown rate is 0.775466x the retained Linux
GCC `rxbvm` rate. That comparison changes both compiler and operating system
and comes from separate sessions, so it does not isolate either effect.

## Machine-state qualification

The initial block ran after installing Visual Studio Build Tools and compiling
optimized `rxvmintp.c` for roughly 15.5 minutes under GCC. Every runtime slowed:

| Runtime/compiler | Initial clauses/s | Cooldown clauses/s |
| --- | ---: | ---: |
| GCC | 6,215,407.5 | 8,992,204.5 |
| Clang | 6,340,611.0 | 9,099,789.5 |
| MSVC | 7,182,710.5 | 10,323,282.0 |
| ooRexx | 10,701,157.5 | 15,917,253.0 |
| Regina | 10,632,669.0 | 15,461,945.5 |
| exact-class NetRexx | 8,093,405.5 | 18,633,329.0 |

All initial cells were flagged. After five minutes at mostly 1-8% CPU load,
the permitted append had 0.70-1.49% relative MAD and 4.10-8.89% span.

The relative compiler result survives both states:

| Block | MSVC/GCC | MSVC/Clang | MSVC/ooRexx |
| --- | ---: | ---: | ---: |
| Initial | 1.155630x | 1.132810x | 0.671209x |
| Cooldown append | 1.148026x | 1.134453x | 0.648559x |

The direct merged summary retains all 20 samples per cell and remains flagged
because the distribution is bimodal. No sample was discarded. The cooldown
block supports the bounded compiler comparison; neither block replaces the
published Windows baseline.

## Build boundary

The successful command was:

```text
cmake --build cmake-build-msvc-rxbvm-release --config Release --target rxbvm --parallel 2
```

CMake configured the full project graph but compiled only `rxbvm` and these
dependencies: VM core/platform objects, RXPA/shim, RXBIN, platform, AVL tree,
VM plugin framework and the decimal plugin. It did not build `rxc`, `rxas`,
`rxlink`, generated benchmarks, `rxvme`/`rxbvme`, or the full test suite.

The MSVC product uses:

- MSVC 19.44.35228 and toolset 14.44.35207;
- Windows SDK 10.0.26100;
- CMake Release `/O2 /Ob2 /DNDEBUG`;
- `NTHREADED=1` switch dispatch;
- dynamic MSVC/UCRT runtime (`MultiThreadedDLL`); and
- profiling off.

The threaded `rxvm` was not attempted because it depends on GNU/Clang
computed-goto extensions not implemented by MSVC.

## Portability regressions found

The initial target build failed before the VM could link. The required fixes
were limited to native portability:

1. `crexxpa.h` and `rxvmload.c` used GNU `__attribute__((unused))`
   unconditionally.
2. `avl_tree.h` emitted GCC `#warning`; MSVC now uses `__forceinline`.
3. `rxvmintp.c` included POSIX `sys/time.h` and used `gettimeofday`,
   `timezone` and `tzname`; MSVC now uses the precise Windows file-time API and
   its CRT timezone accessors.
4. The compile-date array relied on GCC constant-expression extensions.
5. RXPA constant-pool code performed arithmetic directly on `void *`.
6. the interpreter CMake file supplied `-O3` to MSVC, which silently ignored
   it; Release now explicitly selects `/O2 /Ob2` for MSVC.
7. Decimal and test targets linked Unix `libm` unconditionally, producing a
   nonexistent `m.lib` dependency under MSVC.

No compiler, assembler, bytecode, language or dispatch semantics changed.
GCC and Clang `rxbvm` were rebuilt from the same adjusted source.

## Correctness

| Gate | Result |
| --- | ---: |
| MSVC retained RexxCPS smoke | pass |
| MSVC common-five plus Towers one-run pilot | 6/6 |
| Initial six-cell matrix | 72/72 |
| Append six-cell matrix | 72/72 |
| Existing functional `time` RXBIN under GCC/MSVC | pass/pass |

No full MSVC CTest was run because this experiment deliberately built only
`rxbvm`. No profiling was performed.

## Decision

MSVC is a useful Windows `rxbvm` performance option and confirms that native
compiler/runtime choice accounts for a measurable part of the Windows ratio.
It does not close the ooRexx gap or explain the cross-platform reversal, and
this single noisy-host diagnostic is insufficient to add MSVC as a supported
production toolchain.

Future work should first use the Linux profiling evidence. If a supported MSVC
lane is considered later, it needs full Debug/Release build and CTest coverage,
plugin/API compatibility, packaging, and a formal common-five timing run on a
stable host.

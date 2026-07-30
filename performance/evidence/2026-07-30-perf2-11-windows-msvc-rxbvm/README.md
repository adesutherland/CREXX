# Windows MSVC rxbvm experiment

This bundle records a bounded Microsoft C compiler control for the switch-based
`rxbvm`. Only `rxbvm` and its native dependency closure were built. The cREXX
compiler, assembler, linker, benchmark RXBIN and library RXBIN were reused.

## Result

MSVC 19.44 Release `/O2 /Ob2` builds and runs `rxbvm` after narrowly scoped C
and CMake portability fixes. In the stable post-cooldown RexxCPS block it is
1.148026x GCC 15.2 and 1.134453x Clang 22.1.8, but only 0.648559x ooRexx.
MSVC therefore explains part of the Windows comparator gap, not the
Windows/Linux split.

A supplementary CRT-linkage control then rebuilt only MSVC `rxbvm` with the
static `/MT` runtime used by ooRexx instead of the DLL `/MD` runtime. Three
randomized blocks measured a 4.9-6.0% `/MT` uplift. The exact clean block moved
cREXX from 0.645731x to 0.677584x ooRexx, closing only 8.99% of the absolute
gap. `/MT` remains experimental pending plugin/API allocator-boundary tests.

The initial formal block followed the Build Tools installation and long GCC
compile. Every runtime slowed and all cells were noise-flagged. After five
minutes idle, the governed append was stable, with relative MAD between 0.70%
and 1.49%. Both blocks are retained; no sample was removed. The merged summary
remains noise-flagged because it combines the two machine states and is not a
replacement Windows baseline.

## Evidence map

| Location | Content |
| --- | --- |
| `SCORECARD.md` | Result, limitations and portability findings |
| `compiler-blocks.csv` | Initial and cooldown medians and compiler ratios |
| `products.csv` | Exact rxbvm executable identities and code sizes |
| `manifests/` | Formal, append and common-six correctness manifests |
| `timing/` | Initial 2-warmup/10-recorded six-cell block |
| `timing-append/` | Post-cooldown 2-warmup/10-recorded append |
| `timing-final/` | Direct merged summary retaining all samples |
| `pilots/common-six/` | One-run MSVC correctness sweep |
| `crt-control/` | Supplementary `/MD` versus `/MT` A/B samples and PE evidence |
| `logs/` | Configure, failed portability attempts, builds and captures |
| `provenance/` | Toolchain, commands and exact source/product boundaries |
| `checksums.sha256` | SHA-256 closure for every other file in this bundle |

Correctness gates:

- MSVC `rxbvm` target build: pass;
- retained RexxCPS RXBIN smoke: pass;
- six-workload MSVC pilot: 6/6 pass;
- initial and append matrices: 144/144 executions pass; and
- existing functional time RXBIN under GCC and MSVC: both pass.

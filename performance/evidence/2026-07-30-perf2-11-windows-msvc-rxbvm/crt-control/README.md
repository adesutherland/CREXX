# MSVC C runtime linkage control

Status: **bounded supplementary control; not a production configuration**

This control tests whether ooRexx's statically linked Microsoft C runtime is a
material part of the Windows RexxCPS result. Microsoft calls the DLL runtime
`/MD` (`MultiThreadedDLL`) and the statically linked runtime `/MT`
(`MultiThreaded`).

Both cREXX products use MSVC 19.44 Release `/O2 /Ob2 /DNDEBUG`, the same source
content at commit `003c4dfde76ffd47d2a95bc8349138936c4e8873`, and:

- RexxCPS RXBIN SHA-256
  `208874392b8f4b41b5627f7e6ddb97a5d79ad6c586d4ab1b8a0d3b3bd6a136c2`;
- `library.rxbin` SHA-256
  `c790935daecfb7a8c38114a08828cffc4dac882287f30bd6a09c535f1851290c`;
- profiling off; and
- `NTHREADED=1` switch dispatch.

## Result

The exact clean-commit block used two warmups and ten recorded executions per
cell in seeded randomized round order:

| Product | Median clauses/s | Relative MAD | Span |
| --- | ---: | ---: | ---: |
| MSVC `/MD` | 10,377,446.5 | 0.49% | 2.49% |
| MSVC `/MT` | 10,889,342.5 | 0.77% | 7.79% |
| ooRexx 5.1 | 16,070,838.5 | 0.65% | 2.47% |

| Comparison | Ratio |
| --- | ---: |
| `/MT` / `/MD` | **1.049328x** |
| `/MD` / ooRexx | 0.645731x |
| `/MT` / ooRexx | **0.677584x** |

The static CRT closes 8.99% of the absolute `/MD`-to-ooRexx throughput gap.
Two preceding randomized ten-sample blocks measured `/MT`/`/MD` at 1.060356x
and 1.053428x. The repeat range is therefore 4.9-6.0%.

## Binary evidence

PE inspection with MSVC `dumpbin` shows:

- ooRexx `rexx.dll` was linked by Microsoft linker 14.36 and does not import
  VCRUNTIME or UCRT DLLs;
- Regina `regina.dll` was linked by Microsoft linker 14.39 and likewise has no
  dynamic CRT dependency;
- cREXX `/MD` imports `VCRUNTIME140.dll` and the UCRT API DLLs; and
- cREXX `/MT`, ooRexx and Regina all import the Windows heap API family
  (`HeapAlloc`, `HeapFree` and `HeapReAlloc`).

The ooRexx Windows CMake source explicitly replaces `/MD` with `/MT`:
<https://sourceforge.net/p/oorexx/code-0/HEAD/tree/main/trunk/CMakeLists.txt>.

This is a linkage and binary-layout effect, not evidence that cREXX lacks an OS
service. The remaining gap is more likely in hot-path allocation frequency,
value/string representation, dispatch or runtime memory management.

## Method and boundary

Each direct PowerShell capture checked exit code zero, the expected correctness
marker and the integer following `Performance:`. Products were reordered with
a fixed random seed for every round. All 96 executions across the three blocks,
including warmups, passed; no sample was removed.

The first two repeat blocks used the original `/MD` product built from the same
portability changes before they were committed, so its embedded version string
carried the earlier dirty-tree stamp. The headline exact block rebuilt `/MD`
and `/MT` from the clean `003c4dfde` source state.

This supplementary runner was used because the retained pre-update Release
`crexx.exe` could not compile the current matrix driver after the `dslsh`
update (`floatabs` and `floatformat` were unresolved). It is not promoted to a
formal baseline.

The exact `/MD` product was rebuilt in a separate clean target-only tree. An
initial longer build-directory name exceeded Windows' 260-character path limit;
the successful tree was shortened to `cmake-build-md`.

Static CRT linkage must not become the default without full plugin/API tests.
Every DLL linked with `/MT` owns a separate CRT instance, so allocation in one
module and release in another is an unsafe boundary unless the API explicitly
controls ownership.

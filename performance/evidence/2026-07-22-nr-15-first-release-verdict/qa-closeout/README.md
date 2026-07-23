# NR-15 D2-hybrid QA closeout

Status: **complete with one documented platform limitation; no correctness,
compatibility, packaging, or performance guard remains open.**

Adrian accepted the favorable first Release verdict on 2026-07-22 and asked
for all QA through a local commit. The frozen first-verdict evidence under
`../production-d2h/` remains unchanged. This directory records the subsequent
hardening and closeout.

## Correctness hardening

- Added failure-reporting, failure-atomic attribute growth for the native stem
  path without changing the historical `set_num_attributes()` behavior used by
  other VM code.
- Added direct storage tests for binary preparation/reserve failure, attribute
  and string allocation failure, insertion/update/reset/get atomicity,
  corruption and cycle detection, generation/capacity/key-length overflow,
  extraction bounds, segmented keys, aliases, deep copy, move, and destruction.
- Added a permanent RXBIN 007 contract for all nine native-stem opcodes. It
  proves feature bit `0x00000004`, both VMs, link and disassemble/reassemble
  round trips, precise missing-feature rejection, and precise rejection of the
  next unsupported bit. The older NR-14/NR-21 unknown-feature fixtures now use
  bit `0x00000008`.
- The first broad Debug run found only those two now-stale unknown-feature test
  constants (1,898/1,900). After correction, the final complete suite passed.

## Documentation and inventory

- Documented `steminit`, `stemget`, `stemset`, `stemreset`, `stemget2`,
  `stemset2`, `stemsize`, `stemkeyat`, and `stemvalueat`, including forms,
  semantics, signals, examples, feature gating, and private-storage ownership.
- Live `rxas -i` and the reference agree at **384/384 unique mnemonics** and
  **591 forms**, with no duplicate, missing, or extra headings.
- The executable RXAS reference and catch examples pass under `rxvm` and
  `rxbvm`. The focused surface includes all nine new instruction examples.
- The `rxfnsb.stem` RexxDoc audit remains **46/46** blocks plus relevant tags.

## Debug validation

- The complete Debug rebuild passed its 392-step plan.
- The final stem/RXBIN/reference selection passed **30/30**.
- Full Debug CTest passed **1,901/1,901** in 149.21 seconds with
  `--parallel 30`.

## Sanitizer

The complete supported Apple AddressSanitizer product build passed, followed
by the same serialized focused surface at **30/30**. The two preceding focused
runner records are retained: they failed only because the first invocation
excluded the required linked-runtime fixture and the second had not explicitly
built the four `-n` performance artifacts. Building both NR-15 target groups
and rerunning the unchanged selection closed the harness issue; none of the
runs reported a sanitizer diagnostic.

Apple AddressSanitizer on this Darwin arm64 host rejects leak detection as
unsupported. The supported build and tests therefore used
`ASAN_OPTIONS=detect_leaks=0`; AddressSanitizer remained enabled, but
LeakSanitizer coverage is not claimed.

## Release, install, and compatibility

- The ordinary profiling-off Release product (`Release`, `-O3 -DNDEBUG`,
  profiling off) passed its full 1,197-step build plan and the focused **30/30**
  test selection.
- This CMake tree exposes no CPack/package target, so an isolated installed tree
  is the applicable packaging gate. A fresh prefix contained **133 files**.
- Installed `crexx -native` produced an arm64 Mach-O executable from
  `hello.crexx`; it ran and printed `4711` and `hello CREXX world!`.
- Installed `rxc`, `rxas`, and `rxlink` compiled and linked the NR-15 stem
  semantic workload. Installed `rxvm` and `rxbvm` both printed
  `PASS: NR-15 stem semantic matrix`.
- Both installed VMs also ran the exact retained pre-native-stem RXBIN
  `306561951d4dcb349103db8f073b82b32d1648260de7cc4d33886b4d1997ffdf`
  and printed `PASS: NR-15 stem access compare`.

## Post-hardening performance guard

The accepted first-verdict VMs and final QA VMs ran the same byte-identical
D2-hybrid images. The access image remained
`9d936bdad369c669766b34469e467d860d35d9b0681879f4b5854bb268cacdc6`;
the RexxCPS image remained
`42820ecfca8b1a8cde2fc687abad0a864a5faef8517887cf82fc6dbce5b0e35b`.

| Cell | Final versus accepted paired median | Interpretation |
| --- | ---: | --- |
| 60,000,000 get hits, `rxvm` | +0.599% elapsed | inside 3% guard after required ten-row append |
| 60,000,000 get hits, `rxbvm` | -2.439% elapsed | favorable |
| canonical RexxCPS, `rxvm` | +0.789% CPS | favorable |
| canonical RexxCPS, `rxbvm` | +1.086% CPS | favorable |
| load/construction/first access, `rxvm` | -2.088% elapsed | noisy microsecond-scale lifecycle diagnostic; favorable median |
| load/construction/first access, `rxbvm` | -0.448% elapsed | neutral to favorable |

The initial seven-pair screen triggered the evidence runner's absolute-span
rule for `rxvm` get-hit and lifecycle. Ten unchanged serial pairs were appended
for each. No throughput or lifecycle regression guard is hit.

## Boundary

All closeout evidence is from the recorded Darwin arm64 host. No Windows,
Linux, or other-architecture result is inferred. D2 packed snapshots and other
deferred representation ideas remain separate future work.

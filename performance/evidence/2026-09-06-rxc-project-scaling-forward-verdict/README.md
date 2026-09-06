# RXC-PROJECT-01: binary forward-declaration Release verdict

Subsequent disposition: Adrian accepted this verdict on 2026-09-06. See the
[qualification record](../2026-09-06-rxc-project-scaling-qualification/README.md)
for the final implementation and follow-through; the text below retains the
state at the original decision gate.

Provisional candidate on temp/rxc-project-scaling, based on
`e0e67ad3e89e19f98ab09676ffa2bbc99fad7010`. This is a second first-verdict
cell for the newly isolated metadata-order repair, not completed QA.

| Cell | Earlier candidate | Forward-declaration candidate |
| --- | ---: | ---: |
| Clean optimized 48-member wave | 102.56 s | 70.98 s |
| Whole-wave user / system | 527.54 / 7.97 s | 214.78 / 5.47 s |
| Maximum process RSS | 587,350,016 bytes | 590,643,200 bytes |
| Private ragimprove edit | 48 members, 93.92 s | 14 members, 48.70 s |
| Unchanged repeat | zero compiles, 0.26 s | zero compiles, 0.27 s |

The original clean e0e67 Release baseline was 509.45 seconds; see the
[first verdict](../2026-09-06-rxc-project-scaling-first-verdict/README.md).
These are preliminary individual workload runs, not a formal portfolio sample.
No other build/test workload ran during the recorded comparison. Normal -O3
Release compiler, profiling off, normal Rexx optimization, all source/TRACE and
callable metadata retained, same macOS ARM64 host and previously retained
matched package. Only rxc and the already measured dependency-aware wrapper
replace the baseline artifacts; the library/provider bytes are unchanged.
Source/output/install paths are adapted to scratch as authorized.

The compiler SHA-256 is
`fed9ee920d89cfcd194f4e8e4a5493bc2b41f91f5fd69613932c024db20ff10f`;
wrapper SHA-256 is
`db0998c7c18f9009d3105b7a2c07d2dd5e8ecdc1422faf19fb5a8cc5b17f633f`.
`implementation-sha256.json` records changed production/test inputs.
`summary.json` retains actual member/check argv, time, RSS and return codes.
Raw PID/process samples remain in the scratch evidence directory named in the
[worklist](../../RXC-PROJECT-SCALING-WORKLIST.md). The clean output was copied
to `wave-forward-prototype-clean` before the private edit. Both donor copies
matched all 131 frozen hashes before the clean run; the private source was
restored after its recorded run. The live donor was never edited.

## Mechanism and qualification boundary

A binary module's own class declarations are known from metadata but its class
stubs were registered after its function signatures. During that interval,
`node_to_type` could look outside the module for a class the module itself
declares. In the retained stack, `_rxsysb.addressrequest` causes the declaration
for an ADDRESS helper to load the same-namespace RAG ADDRESS extension. That
extension imports most of the application. The new per-import forward-name
stack prevents that lookup while leaving all metadata and eager inline payload
attachment intact. It has no lifetime beyond the synchronous module read.

The basic type lookup is present in 51af9adb08 (2026-04-16), with lookup guard
changes in 029fd59e14 (2026-04-21); the class aggregation path is present in
42dfa9f511f0b7e4f99a91a0d4da90c16ab0db1e (2026-04-24). These are source-history
introduction points, not a complete executable bisect or attribution of the
unreconstructable dirty installed compiler to a single SHA.

The unused-extension reproducer now records H (namespace only), versus F
(full source) before. Its consumer assembly is identical. `provider_contract`
now records all 48 source candidates as H, versus 46 F previously. This removes
an artificial dependency; real imported bodies remain full-content inputs.
Both compilers reject a genuinely used extension's extra argument with
UNEXPECTED_ARGUMENT. An unknown function inside an imported body alone was not
rejected at this consumer compile boundary; that observation is retained and
is not claimed as a negative-validation pass.

Twelve focused source/binary imports, interfaces and reference-inline tests
pass. All 48 application members compile/assemble/link successfully. Thirty-four
member assemblies are byte-identical; fourteen differ. Inspected differences
include generated labels/inline temporary identifiers and imported declaration
order. Full equivalence is not claimed from that inspection. The linked image
also differs; broad QA and offline application execution are still required.
The entire contract/options/toolchain matrix, separate ADDRESS comparison,
normal Debug/sanitizer QA, packaging and final human/AI documentation sync are
pending. This is macOS evidence only; Linux ASan/LSan qualification is not done.

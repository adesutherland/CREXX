# Level C Unicode and stream development receipt

This receipt covers the Adrian-approved LC-UNICODE programme on `develop`,
2026-10-09. The [authoritative plan](../../planning/release-1/levelc-compatibility-worklist.md#lc-unicode--compatible-byte-ordinals-unicode-and-encoded-streams-2026-10-09)
owns its eight acceptance criteria. The
[language and stream contract](../../../compiler/docs/levelc_unicode_and_streams.md)
defines the implemented behavior and permanent regression crosswalk.
Adrian authorized publication to `origin/develop` after local qualification on
2026-10-09. Expected delivery status is published development code with the
qualified inputs below unchanged. Normal automatic Release product/smoke,
optimizer-parity and CodeQL workflows are expected to pass; their actual status
is owned by GitHub run records, not this pre-publication receipt. No release/tag
or manual overnight matrix was requested.
The exposed import-discovery and native-entry faults are separate local safety
commits `952e1ca44` and `7da4a7283`; their permanent regressions accompany this
Unicode/stream changeset. The compiler-split proposal was committed separately
at the starting revision `6127b168f`.

## Qualified inputs and reuse

[qualified-inputs.json](qualified-inputs.json) anchors the complete source tree
at `6127b168f` / tree `07e0bc7f67daf67bc9c699cd3a68b4b603d7013a`, with SHA-256
for every changed code/test/build input and the pinned Unicode authorities.
It records Debug, Release and maintained ASan build settings. Documentation and
this receipt do not change product/test execution inputs.

The runtime, compiler and complete contract fixtures' executable behavior stayed
unchanged after the focused contract freeze. Subsequent changes are confined to CTest registration,
the native lifecycle harness, four embedding fixtures and 18 parser goldens.
The manifest identifies every such delta; affected fixtures have their own
final-input replay. [Debug product hashes](debug-pilot-product-hashes.json)
also prove that all nine qualified compiler/assembler/linker/VM/library/provider
binaries remained identical through QA preparation and the four Debug modes.
Reuse applies to unchanged passing checks, rather than repeating the broad run.
Final whitespace cleanup removed one trailing space in the Python driver and
one after a RexxDoc tag. The manifest retains the exact qualified predecessor
hashes, delivery hashes, identical Python parsed AST and unchanged Rexx code/
line counts; this non-executable cleanup does not invalidate the retained runs.

Local full logs, command lists, source probes and temporary programme directories
are retained at `output/levelc-unicode-20261009.BLUGKe/`. The JSON receipts retain
log hashes and paths; the ignored local logs include earlier unsuccessful probes
and must not be confused with the final passes.

## Focused contract and lifecycle gates

The permanent driver is `compiler/tests/classic_unicode_contract.py --bindir
<build>/bin`, with `--noopt`, `--linked` or both for the other modes. It executes
51 programmes, plus provider preparation and the rejected-import diagnostic,
through rxc/rxas/rxlink/rxvm. Direct modes run 156 commands; linked modes run 207.
[focused-contract-results.json](focused-contract-results.json) retains all ten
final passing runs and their exact command directories and log hashes.

| Gate | Result |
| --- | --- |
| Debug direct opt / no-opt | Pass; 366.03 / 323.28 s |
| Debug linked opt / no-opt | Pass; 511.44 / 1321.18 s; the latter included broad-pool host load |
| Release direct opt / no-opt | Pass; 106.10 / 92.80 s |
| Release linked opt / no-opt | Pass; 227.06 / 215.98 s |
| Release threaded VM, complete direct optimized contract | Pass; 88.80 s; ordinary product VM is portable rxbvm |
| Maintained macOS ASan, complete direct optimized contract | Pass; 745.01 s |
| Native lifecycle, Debug / Release / maintained ASan | Pass; 1.61 / 1.32 / 7.41 s; final Debug check ran in the normal suite |
| Four embedding fixtures, final Debug / Release / ASan | Pass, 4/4 each; 3.32 / 1.11 / 6.07 s |
| Refreshed parser snapshots | Pass, 18/18; 3.46 s |
| Existing independent G Unicode algorithm oracles, optimized portable Release | Pass, 5/5; 10.62 s |

The optimized contract was measured before CTest registration and is serialized
with an 1800-second hang-protection limit. Other modes remain opt-in targets;
they are not four competing nested aggregates in the normal CTest pool. The
native lifecycle test is serialized with a 300-second limit.

The ASan contract ran through `tools/asan-run.sh --phase build --build-target
qa-measure-classic-unicode --build-jobs 4 --build-leaks off --leaks off`.
Its complete contract passed before a stale outer Makefile rejected the newly
added lifecycle target. Explicit reconfiguration and the subsequent lifecycle
build/CTest phases passed. This setup failure was not a contract or sanitizer
failure. Native lifecycle and the four host fixtures use the same runner's
focused build/CTest phases, with one CTest worker and leak checks off because
Apple LeakSanitizer is unsupported.

Native lifecycle covers two simultaneously live VM sessions, exact NUL/FF
results, independent positions, foreign receiver rejection, alias close,
20 unclosed-payload finalizations and retained-value registry teardown. Open
file-descriptor counts return to their initial baseline after both VMs die.
Direct external native descriptors now fail safely at the bytecode-only host
entry; compiled Rexx calls continue using ordinary native dispatch.

The five existing G oracles cover Unicode 17 normalization data, full/simple/
Turkic folding, case mapping, grapheme boundaries and codecs. Their CTest
parity label excludes them from the normal sweep's label selection; this one
optimized portable Release replay supplies the planned independent algorithm
evidence without rerunning the complete parity matrix. Logs are
`release-unicode-oracle-prep.log` and `release-unicode-oracles.log`.

## Installed and static consumers

Release installation into a fresh Unicode-bearing path passed with
`CREXX_HOME` and `CREXX_PROVIDER_PATH` absent. The installed wrapper exercised
CP1252 stream write/read, supplementary-character encode/decode and explicit
NFC. `crexx --program <output> <source> --jobs 1 --native` built a static
consumer; its executable alone was copied to a second Unicode-bearing path
and returned the same expected output and exact file bytes `80 E9`.
The installed `bin/providers/rxcstream.a` was present. Commands and logs are
retained in `installed-probe/commands.json` and `installed-probe-logs/`.

Embedding fixtures select the standard runtime/provider directory through
`rxvml_create(<bin>,0)`. Loading bytecode by explicit file paths does not select
a native-provider search directory. This uses the existing host configuration;
it does not redesign loader or descriptor APIs.

## Normal correctness closure

Core Debug/Release product builds passed. Normal Debug QA preparation completed
1372 build actions. The selected essential/smoke/comprehensive correctness
sweep ran once with 30 CTest workers, excluding the full optimizer-parity tier
and the already-passing optimized Unicode aggregate. It completed 2286 checks
in 2843.68 s, spanning compiler, B/C/G/L, Script, RXAS/linker/VM, native/RXPA,
libraries, tools and ownership/concurrency regressions.

The initial sweep passed 2262 checks and reported 24 failures. All are resolved:

- Eighteen snapshots add two identical `RexxClassicConfig.stream` reconstruction
  trace lines. Removing exactly those two lines leaves the complete old output
  unchanged. [parser-output-reuse.json](parser-output-reuse.json) retains that
  assertion and hashes. The goldens retain the diagnostics and replay 18/18.
- Four embedding fixtures now select the runtime/provider directory. Their
  final Debug/Release/ASan replays pass 4/4 each.
- Two loop fixtures expired at their old 60-second deadlines under broad host
  load. With unchanged product and fixture inputs, isolated replay under those
  original limits passes in 3.33 and 3.62 s. Both opt/no-opt pairs now have
  verified `RUN_SERIAL=TRUE`, `TIMEOUT=300` scheduling. This is a QA deadline
  repair, with no product behavior change or broadened test workload.

[normal-correctness.json](normal-correctness.json) retains the initial result,
every failure's focused closure, exact selection, log hashes and final scheduling
inspection. The unchanged broad passes plus focused closure and retained
optimized contract establish **2287 unique passing normal Debug checks**.
The paired no-opt loop results remain valid; changing only their scheduling
does not invalidate functional results. No second broad sweep was needed.

LC-U-FIND-01 through 04 are closed by these focused and combined normal gates:
rejected-import diagnostic, main SYNTAX identity, configured radix normalization
and safe rejection of native descriptors at bytecode-only external entry.

## Contract coverage and limits

| Acceptance | Evidence |
| --- | --- |
| LC-U-AC-01 | All 256 ordinals, NUL, literals, concatenation/PARSE/slices, conversions and bitwise round trips; trapped/untrapped out-of-domain failures in all four modes |
| LC-U-AC-02 | Unicode letter symbols, changing UTF-8 case widths, pool/VALUE/SYMBOL, indirect SIGNAL/CALL, source/imported/linked routines; exact text/codepoint operations |
| LC-U-AC-03 | Retained console/blanks defaults, simple casing, configured UTF8/BYTE radix normalization, ordinal ordering and authored error identity |
| LC-U-AC-04 | All 23 adapters, omissions/options, strict/replacement codecs, explicit normalization/casing/folding/graphemes; existing G algorithm oracles remain permanent |
| LC-U-AC-05 | All eight stream BIFs, all-byte I/O, read/write positions/counts, EOF/line endings, state/close/reopen, transient transport, NUL paths and NOTREADY SIGNAL/CALL |
| LC-U-AC-06 | All 11 codecs and aliases, mappings above U+00FF, strict/replacement, BOM and encoded line rules; source/console/per-stream ownership |
| LC-U-AC-07 | Native session/configuration isolation, alias/final teardown, installed/static consumers, portable/threaded engines and normal B/G/Script/native regressions |
| LC-U-AC-08 | Exact-input manifests, measured scheduling, focused Debug/Release/ASan, core builds, normal correctness closure, synchronized guides and RexxDoc coverage |

[rexdoc-coverage.json](rexdoc-coverage.json) retains before/after block and tag
counts for every modified existing `.crexx` API source. All existing tags are
preserved; newly added interfaces also carry source documentation.

[supporting-results.json](supporting-results.json) retains the final host,
parser, lifecycle, G algorithm and installed/static evidence, including hashes
and the installation/native command list.

Encoded persistent overwrites currently decode/re-encode the whole file. They
have whole-file memory cost and are not transactional filesystem writes. A
strict transient conversion failure can consume its physical input record.
No implicit NFC is performed. STREAM encoding vocabulary is a documented
cREXX extension, not universally standardized Classic command syntax.

Apple ASan was exercised; Apple LSan is unavailable. Linux/Windows execution,
full platform sanitizers, hosted deep/parity/CodeQL and release qualification
were not run as additional pre-publication gates. Normal automatic hosted
publication workflows run after the push; their success is not claimed by this
local receipt. No cross-platform
sanitizer-clean, exhaustive Classic conformance or release-ready claim follows.
Compiler splitting, INTERPRET, broader host/HALT APIs and the remaining
full-reference/resource/configuration obligations retain their separate owners.

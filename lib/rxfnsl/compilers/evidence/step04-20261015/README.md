# STEP04 executable backend qualification

15 October 2026, macOS ARM64. Local develop and remote develop baseline:
`bcfb068b7abe2f3a2b9a23232fdb4244cc8ffa67`. This receipt covers the approved
unoptimised C/G source-to-RXAS increment. Source/test/build hashes in
[inputs.json](inputs.json) identify the qualified implementation. Adrian
authorised committing and publishing STEP04 to develop and switching the
lightweight CI monitor on 15 October 2026. This substantive publication includes
the implementation, documentation and retained qualification together. Normal
automatic workflows are expected to pass; no observed hosted CI pass is claimed.

## Outcome

Both new compilers execute the admitted POC-01–10 profile through generated
Level L frontends, checked shared IR, B-only instruction/register/ABI selection,
the Level L writer, and the existing assembler/linker/VM. The installed
`llrxc-c` and `llrxc-g` commands and `llcompilers.rxbin` service are separate
products. Existing default compiler routing is unchanged.

Classic keeps dynamic pools, exact literal spelling, numeric snapshots,
argument presence, private PROCEDURE state and RESULT semantics. G keeps its
scalar types, conversions and short-circuit evaluation. Both support local
recursion, loops, branches, SAY, actual LENGTH imports and original source
locations. A decimal declaration's absent default is explicit in G IR;
binary64 constants retain their bits, including negative zero. Loose text
comparison uses REXX comparison instructions and strict comparison exact bytes.

The complete [worked examples](../../examples/step04/README.md) contain actual
source, readable IR, RXAS and executed output. The [backend reference](../../doc/backend.md)
documents target records, maps, call windows, copies/references, imports,
metadata, literal escaping, error propagation and owned/atomic output.

## Local checks

| Check | Evidence and boundary |
|---|---|
| Ordinary Debug product build | [Generation](product-generation.log), [final build](product-build.log); includes separate tools, reusable service and adapter. No compiler warnings. |
| End-to-end CTests | [CTest log](backend-ctests.log), four profiles: opt/no-opt host images on `rxbvm`/`rxtvm`. Each runs 31 source cases with both assembler modes, 35 service assertions, exact-byte/negative-zero execution and eight adverse CLI commands: 232 tool commands per profile. Command receipts are `backend-{mode}-{vm}.json`. |
| Existing relevant correctness | [32 IR/frontend CTests](existing-ctests.log) passed after the frontend decimal-default change. Later backend-only changes do not invalidate these inputs. |
| Installed commands | [Receipt](installed-commands.json), [log](installed-commands.log): 31 cases, both assembler modes and eight adverse commands, 225 tool commands on `rxbvm`. The temporary install's native `rxc`/`rxc-c` executables were moved out of bin; compilation used an empty PATH. |
| Installed reusable service | [Five commands](installed-service.json), [rxbvm](installed-service-3.log), [rxtvm](installed-service-4.log): a B consumer imports only installed `llcompilers`, `rxfnsl`, `library`, then passes 33 service assertions on each VM. Native B compiles this consumer at bootstrap, not its source requests. |
| Independent native controls | [28 admitted controls](native-controls-noopt.json) plus [two text-comparison controls](native-text-controls.json), 120 tool commands, native compiler `-n` and assembler `-n`. `loops-g` is excluded from this comparison because the old grammar rejects its combined nested-IF/XOR source; the new backend tests its explicit expected result. |
| Isolated sanitizer scheduling measurement | [Runner log](asan-isolated.log), [232 commands](asan-isolated.json): no-opt host, `rxbvm`, all 31 cases and service/errors passed in 262.96s. This is an isolated Apple ASan run, not broad sanitizer or cross-platform qualification. |
| Examples | [Twenty commands](examples.json) generated and executed the four source/IR/RXAS/output groups. |
| CI preparation | [Registered tests](ctest-registration.json), [dependency graph](preparation-graph.txt), [recreation log](preparation.log) and [receipt](preparation.json) verify all new host/adapter producers are in `qa-prep-optimizer-parity`. Two deliberately removed images were recreated byte-for-byte, preserving the preceding test evidence. |
| B-only boundary | [Source/import/package audit](dependencies.json); generic Level L and host code remain B, grammar specifications L. Existing STEP03 B-only bootstrap/no-opt closure evidence is reused because the generic library and grammars are unchanged. |

There are 36 distinct focused CTests: 32 existing and four new aggregates.
The 31 source cases are shared across profiles; repeated executions are not
counted as distinct programs. Errors include syntax/type/profile/UTF-8,
missing/foreign imports, malformed IR and operation shapes, canonical constant
codec enforcement, budgets, owned-output lifetime, staging collision and failed
rename. Runtime checks retain authored file/line identity, including CRLF.

Before registering the aggregates, the isolated Debug measurement took 88.63s
for the earlier 29-case corpus; the final 31-case Apple ASan measurement took
262.96s. Each aggregate has `RUN_SERIAL`, a shared resource lock and an explicit
1800-second hang guard; individual nested commands have 180 seconds. This
protects slower hosts without treating wall time as a performance requirement.
The ASan build used `tools/asan-run.sh --phase build --build-target
llcompiler_backend_measure --build-leaks off --no-live-tail --tail-lines 25`.
Apple LeakSanitizer is unavailable. No broad sanitizer lane was dispatched.

## Independent comparison discrepancy

An exploratory native-control run assembled native `rxc -n` output with normal
assembler optimisation. The G scalar operator fixture then produced incorrect
comparison results; the same native output with `rxas -n` matched the expected
result. Retained [RXAS](native-optimized-control/operators-g.rxas),
[no-opt output](native-optimized-control/operators-g-noopt-run.stdout),
[optimized output](native-optimized-control/operators-g-opt-run.stdout) and
[commands](native-optimized-control/receipt.json) preserve that observation.
The fixture source is [operators-g.crexx](../../tests/backend/operators-g.crexx).
This is an unresolved existing native-compiler/assembler interaction; no precise
root cause or repair is claimed. It is not recorded as a passing comparison.
The new backend's output passes this fixture in both assembler modes on both
VMs. The independent semantic control therefore uses assembler no-opt; the
permanent new-backend tests retain both modes.

## Acceptance and remaining scope

F4-AC01–06 apply to this bounded executable profile and local delivery.
LLCOMP-AC01–15 remain open for their full programme scope. Shared optimisation
is STEP05, complete PoC qualification STEP06, proper B self-hosting STEP07B,
INTERPRET STEP08 and production replacement a later explicit gate. Full
metadata/TRACE/condition-handler coverage, concurrent service use and
large-program performance are not inferred from these checks.

Classic's target `rxfnsc` still has G prerequisites. That package is data to the
compiler's B-only metadata reader and an execution dependency of emitted Classic
programs; it is absent from the compiler host closure. Existing native VM, file
I/O provider, assembler, linker and standard generated VM launcher infrastructure
remain visible platform dependencies. No compiler logic was added in C.

Unrelated `build/` contents and two mainframe driver lock files were preserved.
No broad QA, overnight workflow, automatic rerun or status-only publication was
started. Normal hosted checks will be separate evidence after publication.

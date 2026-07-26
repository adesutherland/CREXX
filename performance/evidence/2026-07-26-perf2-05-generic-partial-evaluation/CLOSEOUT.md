# PERF2-05 P05-CF1 closeout

## Accepted implementation

Adrian accepted the favorable first ordinary Release verdict on 2026-07-26.
P05-CF1 replaces the PERF2-04 callable-name/certificate evaluator registry
with a bounded evaluator of the exact resolved Level B callable body.

The compiler admits constant scalar actuals/defaults, supported scalar result
types, proved typed expressions, bounded `IF`/`DO WHILE` flow and only RXAS
operations whose canonical opcode metadata names an exact compile-time
evaluator. It preserves the normal call for reference/exposed formals,
procedure-level `EXPOSE`, arrays/objects/unsupported types, unsupported or
observable operations, reached signals, uncertain results and resource-budget
exhaustion. Local/source bodies use their exact resolved scope; transported
binary bodies still require the existing inlining template proof.

The opcode database now records cursor reads/writes separately from payload
effects. This makes existing observable cursor behavior participate in
inlining/formal isolation proof. The slice documents and tests current effects;
it does not silently clean them or change public instruction semantics.

## Review-derived corrections

The first broad Debug run exposed scope that the small proof suite could not:

1. An array-returning callable could be mistaken for scalar empty text. The
   evaluator now requires scalar result and expression dimensions; a new
   optimized/no-opt dual-VM array-result regression retains the call.
2. An early whole-tree inlining-summary pass changed unrelated optimization
   order. It was removed. Exact local/source bodies are resolved directly;
   binary bodies retain the transported proof gate.
3. Procedure-level `EXPOSE` was not being treated as the intended semantic
   fence. Such bodies now fail closed, preserving side effects and the existing
   no-inline mechanism contracts.
4. The remaining optimized golden changes were replayed against their runtime
   companions and accepted as intended generic constant evaluation. Dynamic
   source-import helpers were retained in the import fixture so those tests
   still prove imported source identity rather than becoming constant-only.

The final full Debug run is clean, so none of the reviewed failures remains in
the accepted tree.

## Correctness and generated-code verdict

| Check | Final result |
| --- | --- |
| complete Debug CTest | 1,920/1,920 passed in 172.98 s |
| focused Release CTest | 6/6 passed |
| Release opcode metadata | 650 total, 591 source, 585 classified, 6 conservative, 56 reserved, 3 internal |
| decisive `rxvm` output | `PASS: PERF2-05 generic user body` |
| decisive `rxbvm` output | `PASS: PERF2-05 generic user body` |
| final versus timed RXAS | byte-identical |
| final versus timed RXBIN | byte-identical with identical input-path metadata |

Focused coverage includes local/source/binary-import and optimized/no-opt
forms, same-summary opposite-body providers, reference/expose fences, array
results, bounded Unicode/empty/boundary string cases, cursor isolation and both
VMs. The general mechanism contracts `nr06_codegen_contract` and
`nr21_fixed_call_contract` are included in the final Release focus set.

## Accepted ordinary Release verdict

The smallest decisive cell calls an unregistered user-written Level B `WORD`
body with constant actuals two million times. It proves the benefit belongs to
generic body evaluation rather than a core-BIF identity.

| VM | Pairs | Accepted median | P05-CF1 median | Paired median | Mean 95% interval | Result |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| `rxvm` | 22 | 184.5295 ms | 7.6735 ms | -95.868% | [-95.965%, -95.650%] | clearly favorable, 22/22 |
| `rxbvm` | 22 | 199.8640 ms | 9.5755 ms | -95.128% | [-95.215%, -95.057%] | clearly favorable, 22/22 |

All 92 raw samples pass. The unchanged RexxCPS guard reached the governed cap
of 36 balanced pairs per VM: `rxvm` paired median +0.028%, mean -0.549%
(95% interval [-1.627%, +0.530%]); `rxbvm` paired median -0.733%, mean -0.445%
(95% interval [-1.330%, +0.440%]). Both are neutral/inconclusive with no
regression guard.

## Closure and boundary

P05-CF1 is independently revertable and introduces no Level B source/API,
public RXAS/RXBIN/ABI, VM-handler or native-function change. Basic PERF2-03
getter/setter inlining remains covered and is not claimed as a P05 gain.

The governed shortest closeout omits the full formal portfolio, sanitizer,
install/package and cross-platform work. The accepted result and final diff do
not justify those unrelated extensions. No push is authorized.

PERF2-05 remains in progress beyond this completed slice. A future
profile-selected semantic assist, private form, public instruction or cleanup
of observable cursor effects requires its own evidence and applicable approval
gate; P05-CF1 does not pre-select one.

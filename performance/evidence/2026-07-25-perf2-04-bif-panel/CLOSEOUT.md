# PERF2-04 accepted-ladder closeout

Date: 2026-07-26

Adrian accepted P04-WRD1's favorable first ordinary profiling-off Release
verdict on 2026-07-26. This authorized the bounded post-verdict closeout for
the complete accepted P04-CAS1, P04-SLC1, P04-CEX1 and P04-WRD1 ladder. It did
not authorize a push, a full formal performance portfolio or a new sanitizer,
installation or cross-platform gate.

## Accepted production identity

The accepted ladder is packaged as one atomic production commit:

`f8f34092eed34812950dd591525e7d927dc0d88a`

`perf: complete certified Level B BIF ladder`

The four stable slice IDs and their incremental first-verdict evidence remain
separable in this bundle. The final source state is intentionally one atomic
commit because the slices cumulatively extend the same certified-call registry,
optimizer schedule and regression surface; manufacturing intermediate commits
after the accepted measurements would not preserve independently rebuilt
source states.

The production owner is the compiler. P04-CAS1 supplies general classified
assembler-operand effects, safe read-only scalar binding and owned result
placement. P04-SLC1 adds exact certified-call evaluation for `UPPER` and
`SUBSTR`; P04-CEX1 extends it to `LOWER`, `LENGTH`, `LEFT` and `RIGHT`; and
P04-WRD1 adds `WORD` plus ordinary constant folding before and after certified
evaluation. There is no BIF-name lowering rule, public RXAS instruction,
serialized RXBIN/ABI change, VM fast path or native BIF. Every rejected or
unproved case retains the complete Level B implementation.

## Broad correctness verdict

The ordinary Debug product was rebuilt from the complete accepted source:

```text
cmake --build cmake-build-debug --parallel 10
```

The rebuild completed successfully with 396 build actions. The focused
affected surface then passed 24/24:

```text
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(linked_opt_runtime_artifacts_build|perf2_04_inline_assembler_effects|perf2_04_inline_assembler_imports|perf2_04_certified_call|inline_reference_accessors_import_opt|test_substr_(noopt|opt)|ts_(left|length|lower|right|upper|upper_lower)_(noopt|opt)|tsubstr_(noopt|opt)|tsword_(noopt|opt))$'
```

The first full Debug run found 16 optimized generated-RXAS golden mismatches
and no runtime or no-opt failure. Five `SELECT` files differed only in label
identity with identical executable instruction and local counts. Ten inline
files reflected three redundant return copies removed in total or downstream
label renumbering. `basic_strings` reflected the intended certified constant
string folds, reducing executable instructions from 74 to 21 and locals from
14 to 5. The 16 expected optimized goldens were refreshed mechanically and
their exact subset passed 16/16.

The final full run was:

```text
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

Result: **1,919/1,919 passed**, 100%, in 141.08 seconds.

The final Debug product hashes are:

| Product | SHA-256 |
| --- | --- |
| `rxc` | `c953bbe34f1c1008e9085670523c82619290e6d6595efd4ae0efc427c3d9dda3` |
| `rxas` | `81c9d2deaea778957ba5a79e08615be62b3753e85b1aafa98dc2c80df7f8f5bd` |
| `rxlink` | `b532ec1bd8d9713e717750490eb76ff6ecdb4114adf7602468c346fcf383b568` |
| `rxvm` | `707cf608a27b3b889fb2cd8f68bae8350936968ec513bddbbb5bb8f6df15047b` |
| `rxbvm` | `d20659554c7b4680511e683af4b1029dc6b16764d398be654590239ec1e867fd` |
| `library.rxbin` | `8c2a539943726adb6c2f2de58cbde48a8172233a06d0f6cbd01032fb285aa836` |

## Evidence replay and review

The accepted first-verdict submanifests independently replay as follows:

| Slice | Entries | Manifest SHA-256 |
| --- | ---: | --- |
| P04-SLC1 | 11/11 | `98dc650f0d885cdee7a2c2c8b584c915c516c40bf9618f38611a7b38da58a0fc` |
| P04-CEX1 | 11/11 | `c6eaff910e05432d98d1d2bac94dbb1bb92e7e8f9f3d9eb543d7781b2af48485` |
| P04-WRD1 | 12/12 | `2c48472d9e48217ea876f081fd3c691aa9e407e9f74ccde8e7a46cef52ac53ee` |

P04-SLC1's manifest was repaired only for the final accepted `verdict.md`
identity; the retained measurement payload was unchanged. The recursive bundle
manifest is regenerated and replayed with the maintained Level B inventory
tool after this closeout record is complete.

Final diff review found no production BIF-specific exception. Certificate
membership is exact and fail-closed by fully qualified callable, signature,
I6 summary, body fingerprint, declared semantic policy and evaluator domain.
Unsupported, dynamic, signalling, oversized, aliased, TRACE-observable or
contradictory-provider cases retain normal inlining or the Level B call. Basic
PERF2-03 getter/setter inlining remains covered by the focused import guard.

No disposable PoC was installed in the product and no public instruction,
serialized format, ABI, VM handler or native ownership was added. The full
formal portfolio, sanitizer, installation/package and cross-platform work were
not run because neither the accepted first-verdict result nor the final diff
created a risk-specific reason to extend the governed shortest closeout path.

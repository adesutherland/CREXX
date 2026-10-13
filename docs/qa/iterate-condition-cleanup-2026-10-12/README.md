# ITERATE condition cleanup repair — 12 October 2026

An ITERATE from an IF body bypassed the condition's normal unlink. On a later
loop turn, register reuse could write a scalar accessor result into the previous
condition's object field. The shared Lotto line host therefore lost an active
confirmation modal. This reproduces on macOS with both released Beta 3 and the
development baseline; it is not specific to Windows.

The repair copies cleanup for already-entered IF branches into crossed
structured exits, preserving fall-through cleanup and the target loop/block.
An exit while evaluating a condition does not clean an unfinished result. A
related imported-inlining check exposed block-result storage sharing an IF
array-condition helper. The existing early block-result reservation now covers
these branch exits, so the result survives unlinking. Scalar accessors remain
inlined, including direct final packed-load donation; no inlining gate is disabled.

## Qualified inputs and focused evidence

Baseline: `6ab5a7815df751e806696c8d196c6ddf7affcb69`, on the existing hotfix
checkout. [Qualified inputs](qualified-inputs.json) identify the exact changed
compiler/test inputs. [Tool identities](tool-identities.json) identify the
executed tools and matching libraries. [BUILDINFO](BUILDINFO.txt) retains the
development base commit and dirty marker; its cached build timestamp is not the
qualification date. The final source commit adds this documentation to the
same qualified product inputs.

| Check | Result | Evidence |
| --- | --- | --- |
| Baseline reduced modal loop, RXC/RXAS opt/opt | Expected failure, modal=0, exit 1 | [log](baseline-opt_opt.log) |
| Baseline reduced modal loop, RXC/RXAS opt/noopt | Expected failure, modal=0, exit 1 | [log](baseline-opt_noopt.log) |
| Baseline reduced modal loop, RXC/RXAS noopt/opt | Expected failure, modal=0, exit 1 | [log](baseline-noopt_opt.log) |
| Baseline reduced modal loop, RXC/RXAS noopt/noopt | Expected failure, modal=0, exit 1 | [log](baseline-noopt_noopt.log) |
| Repaired matrix, AST paths/ownership and related exit/receiver checks | 15/15 passed, 2.46 s | [log](focused-tests.log) |
| Normal compiler-directory sweep | 1,830 passed; seven reviewed golden allocation changes, 1,197.43 s | [summary](compiler-suite.txt) |
| Updated seven goldens and final regression registration | 11/11 passed, 1.35 s | [log](final-replay.txt) |
| Original optimized engineering UI controls | Exit 0 | [log](lotto-controls-runtime.log) |
| Original optimized engineering scoped writes | Exit 0 | [log](lotto-writes-runtime.log) |
| Original optimized Lotto Save/confirmation/write chain | Exit 0 | [log](lotto-save_chain-runtime.log) |

The [baseline reproducer](baseline-reproducer.crexx) is the original 53-line
reduction. The permanent compiler fixture has the same executable source with
an explanatory comment. Its test checks output and actual VM exit, links each
image with the matching library, requires ordinary queued/pending calls in RXC
noopt, and requires those same accessors to remain inlined in RXC opt.
`frame_control_ast` covers nested THEN/ELSE paths and condition-evaluation exits
for ITERATE, LEAVE, LEAVE_WITH and frame branches, plus cleanup-copy ownership
and target preservation.

The unchanged engineering snapshot has archive SHA-256
`461503d840c7f7849b6d1c2646f5e6b1ea3de3d4159ff7aa9250fe6fa41df48f`.
All 32 input checksums were reverified after the tests. [Commands](lotto-commands.json)
and [results/module hashes](lotto-result.json) retain the exact optimized run.
The snapshot's existing unused `ui` import warning is retained in its compiler
log; the snapshot was not edited to hide it.

## Normal correctness gate

The final core product and QA preparation build passed without compiler warnings
or errors. All 1,837 normal compiler-directory cases are qualified: the sweep
passed 1,830 and reported only seven stale assembly goldens; after reviewing
their dedicated result-register allocation and copy instructions, their isolated
replay passed. All paired runtime cases passed in the original sweep. The final
11-case replay also exercises the four new smoke/parity registrations.

```sh
cmake --build cmake-build-debug --target stage-product qa-prep-comprehensive --parallel 8
ctest --test-dir cmake-build-debug/compiler \
  -LE 'performance-measurement|qualification|stress' --parallel 30 --output-on-failure
cmake -S . -B cmake-build-debug
ctest --test-dir cmake-build-debug/compiler --parallel 8 --output-on-failure \
  -R '^(iterate_condition_cleanup_.*|block_expr_basic_noopt|inline_test_say_expr_negative_opt|inline_test_call_arg_expr_opt|inline_test_byvalue_arg_reuse_opt|inline_test_nested_scope_if_do_opt|inline_test_multi_return_expr_opt|inline_test_multi_return_call_opt)$'
```

The first sweep was stopped after exposing the imported IF-guard result alias.
The related repair then passed the final focused checks before rebuilding the
product and restarting the normal sweep. The early AST assertions were corrected
to inspect emitter fragments before existing UNLINKBR/UNLINKN fusion; optimized
runtime checks continue to exercise that fusion. No test was removed or disabled.
The four small new tests run independently, use separate work directories, have
180-second hang protection, and declare their tool/library preparation targets.
They are included in normal smoke and optimizer-parity CI;
[registration](ci-registration.json) retains their commands and scheduling.
The final scheduling-label change preserves the existing test commands and
membership of the normal sweep. The other passing cases were not repeated.
No extra sanitizer or release gate was required for this ordinary development
repair, and no sanitizer-clean or Windows-qualified claim is made.

Complete unfiltered build/test logs are retained locally at
`/Users/adrian/Documents/Franz-UI-Mac-Investigation-20261012/evidence/hotfix-qualification/`.

## Publication contract

The requested hotfix repair is ready for promotion to develop with this receipt,
emitter/agent documentation and the authoritative
[plan](../../../compiler/docs/structured_exit_condition_cleanup_worklist.md).
Expected final state: the qualified repair is published to remote hotfix and
develop, and normal automatic Build CREXX/optimizer-parity/CodeQL checks pass.
Successful push and remote-ref verification are reported in the conversation.
Hosted success is not claimed by this pre-publication receipt; no CI-status-only
follow-up commit or manual overnight dispatch is planned. Native Windows UI and
EXE qualification remain with Adrian and the Windows agent. Rebuild the compiler
and optimized shared UI modules from the published develop source.

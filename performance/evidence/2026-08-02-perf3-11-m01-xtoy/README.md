# PERF3-11 M01 metadata-driven XTOY repetition

Status: **complete — M01 accepted locally; no ordinary-output verdict required**

M01 replaces the final legacy repeated-conversion authority with the reusable
per-procedure proof service.  The old one-register `ITOF` decision is recovered
as the minimum safety floor, but the production consumer is not an `ITOF`
mnemonic rule: it selects canonical one-register derivations with one exact
result component and asks the proof service for dominated successful
repetition.

## Result

- The legacy `flow_compute_available_fact()` conversion case and its raw-view
  selector are deleted.
- All 20 one-register conversion opcodes now have exact source component,
  result component and derivation metadata.
- The focused optimized image removes 12 repetitions:
  one old-floor `ITOF`; one each of `FTOS`, `DTOS`, `BTOD`, `BTOF`, `BTOS`,
  `FTOB` and `STOB`; and four `ITOD` cases covering adjacency, installed
  handler policy, ordered TRACE and linked storage.
- Numeric-context changes, source writes, seven signalling conversion families
  and the same-component `BTOI`/`ITOB` normalizations remain closed.  Their
  proof reasons are retained as `effect-changed`, `source-changed` or
  `success-edge-not-dominating`.

This is the intended basic-to-advanced migration rule: the old accepted set is
the floor, the new service is sole authority, and stronger cases are admitted
only through positive component/storage/effect/signal proof.

## ITOD and BTOD total contract

The initial `ITOD` proof correctly rejected because its old plugin-signal
contract created inherited skip/retry paths around the first conversion.  Code
review showed that both bundled decimal backends convert every `rxinteger`
without a language-level failure: allocation already follows the VM
panic-on-OOM convention, while the old check could only propagate stale plugin
or numeric-backend diagnostics.

`decimalFromInt` now explicitly clears stale diagnostics in both backends.
`ITOD` and `BTOD` no longer dispatch a plugin signal, and canonical metadata
marks them non-signalling but still dependent on numeric context and plugin
identity for value equivalence.  Focused plugin tests lock the total contract;
both VM dispatch engines pass optimized and unoptimized semantic oracles.

## Representative output gate

The exact retained inputs remain byte-identical under the frozen Stage 6
assembler and the M01 Release assembler:

| Workload | RXBIN SHA-256 before and after |
| --- | --- |
| Richards | `ca4efcf26edd849d09c67482feda6da82aecbb7d4dd76f3b6a1b37be00dfd8ca` |
| Towers | `0fdac42959023dd36ac5b5fbdf43335302dbfaad76bd59624b2dfb9e872fc011` |
| RexxCPS | `beeba2fefc0496e2f17dbb2ee48619c0f80ae69ed5f3aaa41f4ffe6c4fa40467` |

RexxCPS exposes candidates for `DTOS`, `FTOS`, `STOD`, `STOF` and `STOI`, but
only the five already accepted `ITOS` decisions prove.  M01 therefore creates
no new ordinary representative output and does not trigger a runtime
performance verdict.

## Scale and correctness

Release diagnostic RexxCPS assembly completes in **0.38 s** with **22,265,856
bytes** peak RSS.  The retained Stage 6 result was 0.39 s and 20,185,088 bytes;
the complete conversion metadata and extra rejected queries remain inside the
accepted seconds-scale assembler budget.

- strict GNU90 RXAS syntax checks pass;
- opcode metadata and immutable-flow unit tests pass;
- focused optimizer oracle passes **51/51**;
- both decimal plugin suites pass;
- Debug and Release optimized/unoptimized signal-contract execution passes on
  both `rxvm` and `rxbvm`; and
- broad Debug CTest passes **1,989/1,989**.

No push was performed.

# PERF3-11 Stage 4 signal-policy and effect SSA

Status: **complete — Gate 4 passes**

This bundle closes the output-neutral signal-policy/effect layer. It is an
assembler correctness, image-equivalence and scaling gate, not a VM runtime
benchmark or an optimizer-consumer verdict.

## Provenance

- Branch: `codex/perf3-rxas-flow-infrastructure`
- Stage 3 base: `55da417ce` (`perf: add cached RXAS structural analyses`)
- Stage 4 source: the base above plus the Stage 4 code, tests, documentation
  and this bundle; the resulting local commit is authoritative in Git history.
- Frozen comparator: Gate 0 profiling-off Release `rxas` at
  `/tmp/crexx-perf3-11-stage0.35IBzf/base-binaries/rxas`.
- Host: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs, 24 GiB RAM.
- Power: AC; low-power mode off.
- Thermal state: no recorded thermal, performance or CPU-power warning.
- Build: CMake/Ninja Release, `CREXX_VM_PROFILING=OFF`.

## Semantic result

The immutable procedure epoch now owns an optional signal analysis layered on
the Stage 3 structural cache. The analysis uses sparse write-once identities
for handler policy and seven independent effect classes: numeric context,
plugin, locale, external state, reference-visible state, TRACE and calls.
Parallel normal/skip edges remain distinct inputs even when they share the
same predecessor block.

Procedure entry supplies an inherited-unknown policy parameter. Statically
named policy writes create exact versions; unresolved opcodes create an
explicit whole-policy clobber. Failure edges select the pre-write, post-write
or conservative partial state from the opcode signal contract. Normal return
and unwind restore the entry policy parameter.

VM call arguments are references to caller-owned value storage. Accordingly,
call and reference effect identities advance across a call even though a
callee's copy-on-write handler table is frame-local and its policy changes do
not leak back to the caller. This distinction is a permanent focused test.

`sigpush` does not change the active handler result, but its silent allocation
failure means a later `sigpop` cannot prove one exact restored policy. The
analysis therefore returns explicit stack-unknown state rather than inventing
a successful save/restore. Loop phis preserve an unchanged write-once policy
without treating source order as proof.

## Correctness and image result

- Strict GNU90 syntax checking passes with `-Wall -Wextra -Wconversion
  -Wsign-conversion`.
- The focused Debug matrix passes **113/113**. It includes the Stage 1
  optimized/no-opt dual-VM signal fixtures, both decimal plugins, storage and
  conversion runtime fixtures, optimizer guards and the new signal/effect
  contract.
- Ordinary profiling-off Release `rxas` builds.
- Canonical Richards, Towers and RexxCPS RXBIN hashes remain exactly equal to
  Gate 0.
- Every canonical `rxas -d` procedure returned an available Stage 4 result;
  no disabled analysis was observed.

## Sparse analysis result

The counters below are diagnostic analysis totals across each canonical
assembly. Retained bytes are summed per-procedure cached arrays; procedures
are still built, analysed and destroyed one at a time.

| Workload | Procedures | Work / budget | Retained bytes | Policy versions / writes / phis | Effect versions / phis | TRACE / call writes | Edge states / conservative effect states |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Richards | 24 | 64,677 / 6,699,520 | 2,181,184 | 1,043 / 0 / 1,019 | 27,342 / 7,133 | 1,738 / 410 | 4,972 / 14,350 |
| Towers | 13 | 19,684 / 2,147,840 | 666,264 | 374 / 0 / 361 | 8,359 / 2,527 | 434 / 122 | 1,437 / 4,270 |
| RexxCPS | 5 | 52,168 / 4,820,992 | 2,009,160 | 793 / 5 / 783 | 25,529 / 5,465 | 1,252 / 445 | 3,405 / 15,575 |

The large conservative-effect counts come primarily from deliberately
unknown call failure phases. They are explicit identities, not a false proof
that those edges preserve caller-visible values.

## Ordinary assembler-cost result

The same-session comparison used two warmups and 30 balanced/interleaved
elapsed rounds per workload. Peak RSS used ten separately interleaved samples.
Pre/post load was `{ 3.52 4.04 2.91 }` / `{ 2.14 3.59 2.81 }`.

| Workload | Frozen median | Stage 4 median | Elapsed delta | Frozen median RSS | Stage 4 median RSS | RSS delta |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Richards | 0.060088396 s | 0.059647918 s | -0.733% | 8,601,600 B | 9,207,808 B | +606,208 B (+7.048%) |
| Towers | 0.019634485 s | 0.019505620 s | -0.656% | 4,726,784 B | 5,013,504 B | +286,720 B (+6.066%) |
| RexxCPS | 0.053365946 s | 0.050984382 s | -4.463% | 9,773,056 B | 10,092,544 B | +319,488 B (+3.269%) |

No elapsed regression is present. Richards and Towers exceed 5% RSS, but
their absolute changes remain below 1 MiB, so the combined RSS escalation rule
is not met.

## Gate 4 verdict

Gate 4 passes. Signal-policy and non-register effects are epoch-cached,
demand-driven, deterministic, edge-specific and fail closed. A Stage 5
component/storage proof may now consume a signal edge only through these
versioned results; Stage 4 itself changes no queued instruction or RXBIN image.

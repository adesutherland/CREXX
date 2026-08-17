# POSTPERF-01 Full AWFY Json compiler-repair first Release verdict

Date: 2026-08-17

Status: **accepted by Adrian on 2026-08-17; proceed with POSTPERF-01
closeout**.

## Scope and provenance

The control is a clean detached ordinary profiling-off Release build of
`110e298af8ea6d702669cf26533e426005141df8`. The candidate is the same commit
plus the uncommitted POSTPERF-01 scope and the narrow
`compiler/rxcp_inline_bind.c` repair. Both builds use `Release`,
`CREXX_VM_PROFILING=OFF`, AppleClang 21.0.0 and the default `profile-20` VM
handler panel.

Full AWFY Json exposed a compiler defect when optimized inlining bound a
runtime `.string` actual to a `.binary` value formal. The normal call emitted
the supported `stobin` promotion; the inline binder instead created a raw
register copy and `binlength()` observed an empty binary. The candidate keeps
the call inlined but routes this conversion-bearing isolated binding through
the ordinary typed assignment emitter.

Control `rxc`:

- bytes: 3,451,440
- SHA-256: `61e9677e0b91983cfc616b721892a7613f2926b211cbe25f31788cc7f9bb697c`

Candidate `rxc`:

- bytes: 3,451,440
- SHA-256: `8ce897abc00223d50e3bd09bce6cd72ac403f44f9660a10499b134a9b127e051`

## Decisive result

Both compilers compile and link the exact 25,820-byte Full AWFY Json input.
The control fails at runtime with `expected a 25820-byte fixture, got 0`; the
candidate passes with 156 operations and 3,392 indexed nodes. Candidate RXAS
retains the three inlined `blen` operations and adds the three required
`stobin` conversions. The new focused shape regression separately requires an
inlined `blen`, rejects a retained `binlength()` call and requires `stobin`.

Ten established workload/tool images are byte-identical between control and
candidate: Sieve, Permute, Bounce, Richards, Towers, Base64-v2, canonical
RexxCPS, JSON parse, JSON query and the maintained evidence runner. Their
retained runtime performance evidence therefore remains applicable.

Six serial alternating-order compiler pairs compiled the 2,421,428-byte RXAS
form of `performance/tools/run_evidence_bundle.crexx`. Mean wall time was
1.100000 seconds for the control and 1.101667 seconds for the candidate, a
candidate delta of +0.151515%. The timer resolution is 0.01 seconds, so this is
a neutral/no-regression observation, not a compiler slowdown claim.

The recommended verdict is to accept the correctness repair and proceed with
the bounded POSTPERF-01 pilot and closeout QA.

## Focused validation at the gate

- Debug optimized/unoptimized runtime regression and optimized shape contract:
  3/3 pass;
- Debug Full AWFY Json product smoke: optimized and unoptimized pass;
- Debug direct `rxvm`/`rxtvm`/`rxbvm` by optimized/unoptimized matrix: 6/6
  pass;
- ordinary profiling-off Release focused selection: five selected tests plus
  the linked-runtime setup pass 6/6;
- clean-control versus candidate established RXAS comparison: 10/10
  byte-identical;
- clean control Full AWFY Json: expected failure; candidate: pass.

At this gate, broad Debug CTest, the bounded timing pilot, documentation
closeout and the stage commit were deferred until Adrian accepted the result.
The accepted closeout is recorded in `VALIDATION.md` and the separate
qualification bundle.

## Host and interpretation boundary

- Darwin 25.5.0 arm64, macOS 26.5.2, Apple M5, 10 logical CPUs;
- AC power, low-power mode off;
- pre-capture load averages 1.57, 3.43 and 4.11;
- one compiler warm-up per binary, then six serial pairs with alternating
  order;
- source and TRACE metadata retained.

This proves the repair and its neutral established-code shape on the primary
macOS development host. It is not broad release QA, cross-platform evidence,
or a formal Full AWFY Json throughput baseline. The cREXX benchmark remains a
labelled `standard-library-indexed-document` adaptation outside every
cross-runtime aggregate.

## Bundle map

- `artifact-comparison.csv`: exact control/candidate RXAS hashes, sizes and
  instruction counts;
- `compiler-timing.csv`: all six raw alternating-order compiler samples;
- `run-results.csv`: decisive Full AWFY Json control/candidate runtime result;
- `COMMANDS.md`: reproducible build, comparison and validation shapes;
- `VALIDATION.md`: post-acceptance proportional closeout;
- `checksums.sha256`: recursive evidence hashes.

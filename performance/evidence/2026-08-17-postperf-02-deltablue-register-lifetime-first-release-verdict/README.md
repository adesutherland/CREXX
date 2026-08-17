# POSTPERF-02 DeltaBlue register-lifetime first Release verdict

Date: 2026-08-17

Status: **accepted by Adrian on 2026-08-17; proceed with proportional
DeltaBlue closeout**.

## Scope and provenance

The control is a clean detached ordinary profiling-off Release build of
`00188b13ca519058cde31621b84b64aaecaf809a`. The candidate is the same commit
plus the uncommitted POSTPERF-02 DeltaBlue scope and the bounded compiler
register-lifetime repair. Both builds use `Release`,
`CREXX_VM_PROFILING=OFF`, AppleClang 21.0.0 and the default `profile-20` VM
handler panel.

DeltaBlue follows `smarr/are-we-fast-yet` commit
`74306fec151070fd07157cefeacf19e7e0bcdc89`. The Level B adaptation uses stable
integer handles and planner-owned typed arrays to preserve graph identity. It
runs the complete upstream chain and projection contracts and is labelled
`stable-indexed-constraint-graph`; it remains outside every common aggregate.

DeltaBlue exposed a compiler defect when an indexed class-attribute target was
linked before an inlined factory `BLOCK_EXPR` on the right-hand side. An
internal statement boundary returned the target's deferred destination and
owner registers, allowing a factory temporary to overwrite the still-live
owner before `unlinkn`. The resulting aggregate was cyclic/corrupt and crashed
the VM during `copy_value`.

The candidate retains the supported inline and indexed-assignment shape. At an
internal right-hand statement boundary it returns ordinary deferred
temporaries but preserves only the target destination and helper registers
until the enclosing assignment cleanup. The focused structural test requires
the inlined factory and rejects any write to the `minlinkattr1` owner before
the matching `unlinkn`.

Control `rxc`:

- bytes: 3,451,440
- SHA-256: `038360e53182dd40a9881b21d81eff9841ad84f0cce026437be07792b82b0265`

Candidate `rxc`:

- bytes: 3,451,504
- SHA-256: `a2fa98ffd4d6593b54f17887ebf0163e5d8e64c5e20eb531f3c809e7eff08e8e`

## Decisive result

The clean control compiles and links both the focused reproducer and DeltaBlue,
then each optimized image exits 139. The candidate passes both images in
optimized and unoptimized form under `rxvm`, `rxtvm` and `rxbvm`: 12/12 direct
Release cells pass. DeltaBlue size 10 completes both the full chain and
projection verification contract.

Ten established workload/tool RXAS images are byte-identical between control
and candidate: Sieve, Permute, Bounce, Richards, Towers, Base64-v2, canonical
RexxCPS, JSON parse, JSON query and the maintained evidence runner. Their
retained runtime performance evidence therefore remains applicable. The
candidate changes only the two images that contain the newly proved defective
shape; their source-instruction counts do not increase.

Six serial alternating-order compiler pairs compiled the 2,421,428-byte RXAS
form of `performance/tools/run_evidence_bundle.crexx`. Mean wall time was
1.086667 seconds for both control and candidate, a 0.000000% delta. The timer
resolution is 0.01 seconds, so this is a neutral/no-regression observation.

The recommended verdict is to accept the correctness repair and proceed with
proportional QA and bounded DeltaBlue qualification before starting CD.
Adrian accepted that verdict on 2026-08-17.

## Focused validation at the gate

- Debug linked-runtime fixture plus focused optimized/unoptimized regression,
  structural contract and DeltaBlue smoke: 6/6 pass;
- ordinary profiling-off Release regression and DeltaBlue direct matrix:
  optimized/unoptimized under `rxvm`, `rxtvm` and `rxbvm`, 12/12 pass;
- Release structural contract: pass;
- clean-control focused regression and DeltaBlue optimized images: both exit
  139; candidate forms pass;
- clean-control versus candidate established RXAS comparison: 10/10
  byte-identical;
- six alternating compiler pairs: identical 1.086667-second means.

At this gate, broad Debug CTest, sanitizer selection, bounded DeltaBlue timing,
documentation closeout and the stage commit are deferred until Adrian accepts
the result. CD has not started.

## Host and interpretation boundary

- Darwin 25.5.0 arm64, macOS 26.5.2, Apple M5, 10 logical CPUs;
- AC power, low-power mode off;
- pre-capture load averages 1.62, 4.21 and 4.52;
- one compiler warm-up per binary, then six serial pairs with alternating
  order;
- exact source and TRACE metadata retained.

This proves the repair and its neutral established-code shape on the primary
macOS development host. It is not broad release QA, cross-platform evidence,
or a formal DeltaBlue throughput baseline.

## Bundle map

- `artifact-comparison.csv`: exact control/candidate RXAS hashes, sizes and
  instruction counts;
- `compiler-timing.csv`: all six raw alternating-order compiler samples;
- `run-results.csv`: decisive control and candidate runtime matrix;
- `COMMANDS.md`: reproducible build, comparison and validation shapes;
- `VALIDATION.md`: post-acceptance proportional closeout;
- `checksums.sha256`: evidence hashes.

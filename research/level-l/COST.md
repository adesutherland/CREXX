# Cost and lifecycle — interim evidence

**STEP-14 / AC-22 remain open.** Lifecycle correctness passes; the first timing
block is inconclusive. A corrected request-release boundary is qualified and
awaits an uncontended serial run. This is a non-representative parser-mechanism
study, not a product benchmark. RexxCPS does not exercise this mechanism and is
intentionally excluded.

## Executed protocol

Tranche17 freezes sources/tools/images for one warmup and13 recorded rounds on
each concrete VM, serially. It retains raw samples, phase summaries and three
whole-process peak-RSS observations each for `check` and `stress` on each VM.
Optimized/no-opt correctness passed on both VMs. No production code changed.

The language is `a+(;a+)*`: `a`, a48-byte atom,24 one-letter records separated
by semicolons (47bytes), and damaged `a;;a`. The token route uses a native
three-state DFA and declared identity token stage. Pratt and the delimited
Pratt-in-LR composite use separate expression fixtures; they are not pooled.

| ID | Engine | Observation / comparability limit |
|---|---|---|
|1|Normalized packrat|Byte/token routes, internal arena and selected-tree copy|
|2|LL(1) control|Byte/token routes, same selected-copy service|
|3|Settled reverse PEG|Byte/token routes, full chart and selected-copy service|
|4|Canonical LR(1)|Byte/token routes, generated tables and selected-copy service|
|5|LALR|Byte route only|
|6|Earley|Packed forest; extraction counts derivations, not an identical copied AST|
|7|Pika reachable-reference|History, native selected projection and selected-copy service|
|8|Squirrel|Retained history; no common selected-source-tree copy in this harness|
|9,10|c14/frontier|Raw-byte structural recovery, distinct ranking policies|
|11|LR local repair|Raw-byte token alphabet, local edit cost|
|12|Exact weighted CFG repair|Raw-byte insertion/deletion1, substitution2, search bound1|

The last four engines use raw input only here. The exact oracle's positive-cost
success is displayed as repaired status1; its underlying API is unchanged.
These different recovery objectives and tree observations have no common score.

## Phase boundaries and the correction

`decode` creates a byte-stream representation, not a Unicode decoder. `lex`
and `stage` include construction/execution. `initialize` constructs a request.
`parse_and_internal_tree` fuses admission, recognition and internal tree/history
work; no isolated recognition-only timing is claimed. `extraction` measures the
selected projection/copy or the different Earley derivation query. `visitor`
sums spans. The internal total also includes validation, idempotent replay,
result access and setup, including token-route CFG construction outside the
narrower phase timers. Phase medians need not sum to the total median.

Generation records separately cover grammar creation, Pika lowering, LR/LALR
tables, NFA-to-DFA construction and DFA loading. Overall generation also
includes checks and specialist setup. The DFA has3states and184serialized bytes.

The first harness returned an entire context in `obs`. Its outer timer enclosed
assignment to `obs`, including release of the preceding candidate's observation.
Thus `call_with_result_transfer` measures a transition between candidates, not
an isolated lifetime cost attributable to the displayed engine. Original source
and runner are retained in `evidence/lifecycle-boundary-correction/`; tranche17
raw results retain their original labels.

Corrected `measuredrequest` returns only a fixed-size metrics array and its
owned context leaves scope inside that request. The new summary calls this
`request_with_release`. Tranche18 retains four passing corrected correctness
outputs with identical counters/semantics to tranche17, four synthetic summary
checks, and21 negative mode/budget/input exit checks. Corrected timings are
still outstanding.

## Environment and memory limits

Apple M5,10 logical CPUs, macOS26.6.2 ARM64; battery92% before/91% after,
low-power mode off. Retained **before and after** snapshots show other compiler
activity; later inspection found a broader concurrent build/test workload.
The first block therefore lacks the isolation needed for a timing decision.
Microsecond phases remain noisy after13samples. No trimming, favorable subset,
speed ranking, VM verdict or formal product claim is supported.

Old whole-process peaks are approximately2.9GB for both two-cycle `check` and
forty-cycle `stress`; all12 values are in `rss-*.time`. This is a high footprint
for these inputs and needs attribution before product use. It includes loaded
modules, preparation, earlier cases, overlapping requests and VM/allocator
storage. High-water marks are not per-engine live bytes, proof of release, or
leak/sanitizer qualification. Corrected ownership must be measured separately.

The narrower deterministic checks are valid: replay preserves result/work;
large/small/damaged contexts coexist without source/invariant corruption; eight
fresh small-after-large requests have identical work/storage fingerprints;
scope stress completes without functional failure. Fuel/cell exhaustion is
distinct from syntax failure. Existing depth/state/cell limits remain explicit
in REPORT/MATH; arbitrary stack depth is not qualified here.

On the48-byte atom, Pika's parser work falls from288 to3 with one token, while
reverse lexing adds144 DFA transitions. Earley work falls from44,234 to110.
These are engine-specific counters, not equal-cost operations or time predictions.
Earlier source/span parity and maximal-munch/Pika adversarial scaling remain
valid independently of this timing issue; see MATH and tranches2/10.

## Reproduce and finish

```sh
cd /Users/adrian/CLionProjects/CREXX-level-l-inline-storage
export CREXX_BIN_DIR="$PWD/research/level-l/build/frozen-tools-20261010"
bash research/level-l/run-lifecycle.sh opt
bash research/level-l/run-lifecycle.sh noopt
```

After correctness, freeze exact inputs and inspect host activity. Run each VM
serially on `lifecycle-linked.rxbin -a measure 13`, redirecting raw output.
Run `lifecycle-summary.rxbin` with the frozen `library.rxbin` and
`-a <raw-output-path>`. Use `/usr/bin/time -l` for three `check`/`stress` RSS
observations per VM, keeping stdout/stderr separate. The runner now builds the
Level B summary in both modes; it covers generation, lexer, specialist and
request phases with min/Q1/median/Q3/max/MAD/noise. Retain the old block intact.

Remaining: an uncontended corrected block, lifetime/peak interpretation and
the final exact-input receipt. No production optimization or termination of
other active work is authorized by this interim finding.

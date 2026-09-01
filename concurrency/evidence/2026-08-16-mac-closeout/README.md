# Initial concurrency Mac closeout

Date: 2026-08-16  
Branch: `develop`  
Candidate: `5bede182efa93dfd8ee79680f8b1232dcdf39f5e`

Status: **Mac correctness, sanitizer, stress, install and portable-package
proof pass. The governed AC-power performance replay remains open.**

## Result

- the complete Debug build passed;
- full Debug CTest passed 2,204/2,204 in 415.44 seconds;
- the labelled Apple AddressSanitizer matrix passed 180/180, comprising all
  179 concurrency tests plus the linked-runtime fixture; LeakSanitizer is not
  available on this Apple host, so leak detection was explicitly disabled;
- all six `concurrency-stress` tests passed 20 consecutive repetitions, 120
  executions in total;
- the ordinary profiling-off Release build passed, with `profile-20` and the
  Network.framework TLS backend;
- the Release concurrency entry point passed 180/180 in 18.05 seconds; and
- an isolated 156-file install compiled, assembled and linked the documented
  Level G basic-concurrency example with the installed tools, then ran it
  successfully on installed `rxbvm` and `rxtvm`.

The isolated install was packaged as a local portable ZIP. The archive is a
reproducible build output and is not committed; its retained SHA-256 is in
`install/package.sha256`. This proves the local payload shape, not signing,
notarization or release publication.

## Performance boundary

The paired task-launch and seven-workload single-thread campaign is retained
separately under
[`performance/evidence/2026-08-16-initial-concurrency-mac-closeout`](../../../performance/evidence/2026-08-16-initial-concurrency-mac-closeout/).
It is diagnostic only: the power log proves that the machine was on battery
during the campaign, contrary to the performance-governance AC requirement.
It therefore does not replace the earlier accepted AC baselines and does not
complete the Mac performance part of QA-B.

## Evidence map

- `qa/debug-build.log` and `qa/debug-ctest.log`: complete Debug product and
  2,204-test regression result;
- `qa/asan-build.log` and `qa/asan-concurrency-matrix.log`: complete Apple ASan
  build and labelled matrix;
- `qa/debug-stress-repeat-20.log`: unchanged repeated-race/stress execution;
- `qa/release-build.log` and `qa/release-concurrency-matrix.log`: ordinary
  Release product and maintained concurrency matrix; and
- `install/`: isolated install inventory, direct installed-toolchain smoke and
  portable-archive digest.

Exact replay commands are in [`COMMANDS.md`](COMMANDS.md).

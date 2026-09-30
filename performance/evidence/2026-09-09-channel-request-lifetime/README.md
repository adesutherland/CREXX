# Channel request lifetime qualification, 9 September 2026

Worktree `hotfix`, based on `d840dc1b0730b793b145b767979b3905e6f4266d`,
with the uncommitted channel-lifetime change. The affected installed revision is
`7de12145a0695a81b345eeff8405203c23586e8c`. No commit, user-prefix installation,
publication or downstream application change is part of this qualification.

After this local qualification, Adrian approved publication to `origin/develop`,
updating local `develop`, local installation and a cREXX-RAG agent handoff.
The [phase record](../../../concurrency/CHANNEL-REQUEST-LIFETIME.md) assigns the
outstanding platform and sanitizer-register closure gates to Hotfix release QA.
This does not convert the local evidence into cross-platform qualification.

The [selected contract](../../../concurrency/CHANNEL-REQUEST-LIFETIME.md) adds
explicit release after terminal observation, reusable request tracking and
direct provider unlinking. Existing callers retain their request until release
or close. Completion buffers and immutable completion values have separate
ownership. The request-generation namespace remains finite; wrapped slots are
retired to preserve stale-capability rejection.

Typed tracking clears logical values and ownership while reusing entry storage.
The VM's ordinary string, binary and attribute buffers keep capacity for reuse
(`set_num_attributes`, `reset_value_storage_for_reuse` and `value_zero` in
`interpreter/rxvmvars.h`). That existing policy can retain capacity while a
wrapper lives, including after logical close. Release physically destroys the
provider request and joins its private thread, but does not promise immediate
process-RSS shrinkage or discard application-owned completion copies.

## Accepted first Release result

[`FIRST-RELEASE-VERDICT.md`](FIRST-RELEASE-VERDICT.md) is the original decision
snapshot. Adrian subsequently accepted its 2.51 MiB / 13.9% retained-request RSS
tradeoff and authorized the remaining QA and documentation. Its original
“awaiting” wording records the decision point, not the current approval state.

The paired raw logs, native and public sustained-use logs, host snapshots,
artifact hashes and exact native build argv are retained alongside it. The
measurement is a single-mechanism diagnostic, not a representative portfolio or
formal 12-pair performance claim. It shows:

- 65,535 unreleased requests still exhaust capacity, as the selected contract
  requires; close falls from 13.920–14.311 CPU seconds to 0.001 seconds at the
  unchanged probe's reporting precision.
- Explicit release completes 200,000 native cycles with zero live tickets at
  each checkpoint.
- The ordinary optimized public image completes 200,000 cycles in 2.93 seconds;
  peak RSS is 64 KiB higher than at 1,000 cycles. This does not promise unlimited
  generations or constant memory for arbitrary retained completion values.

The versioned public workload is
[`testChannelRequestRelease.crexx`](../../../lib/classlib/tests_functional/testChannelRequestRelease.crexx).
The native lifetime regression is
[`test_rxvmchannel_lifetime.c`](../../../interpreter/tests/test_rxvmchannel_lifetime.c).
Diagnostic probe sources and the full source freeze remain in
`/tmp/crexx-channel-release.1v4pFt/`; the unchanged baseline and original
diagnosis remain in `/tmp/crexx-channel-investigation.NMIYvV/`.

## Ordinary Debug qualification

Fresh CMake/Ninja Debug tree, profiling off, network downloads disabled:

```sh
cmake --build /tmp/crexx-channel-release.1v4pFt/debug --parallel 6
cmake --build /tmp/crexx-channel-release.1v4pFt/debug --parallel 6 --target qa-prep
ctest --test-dir /tmp/crexx-channel-release.1v4pFt/debug --parallel 30 --output-on-failure -LE '^performance-measurement$'
```

**2,298/2,298 pass**, 596.95 seconds, in
[`full-debug-ctest.log`](full-debug-ctest.log). The excluded label contains the
separate measurement lane; no correctness test was excluded. The earlier
focused logs retain public API, byte, child and process release checks. The full
run also covers opcode metadata and feature validation, both VMs, optimized and
unoptimized compilation, HTTP and scope-owned task results after close.

The installed old `rxdas` rejects the new image with unsupported feature bit
`0x80`; [`old-reader-release.log`](old-reader-release.log) retains the expected
exit 255. RexxDoc coverage preserves all 84 parameter and 101 return tags and
adds one documented operation; the maintained coverage test passes. The API
appendix adds the generated release signature to the affected request entries
without regenerating unrelated, historically duplicated declarations.

The installed old `rxvm` also rejects that image with exit 255
(`old-runtime-release.log`); the candidate Debug `rxvm` loads and executes the
same assembly-only fixture with exit zero. This is image-compatibility evidence,
separate from the lifecycle assertions in the permanent tests.

## Native thread ownership diagnostic

[`async_join_probe.c`](async_join_probe.c) links the frozen Release static VM
archives and wraps their `pthread_create`/`pthread_join` calls for counting.
It performs 5,000 alternating 4 KiB write/read pairs on one memory endpoint.
For each of the **10,000 requests**, exactly one thread is created, no join has
occurred before release, and exactly one successful join has occurred when
release returns. Live tickets return to zero after every operation. Every saved
read result still matches the full payload after release. The completed channel
then closes successfully. See `async-join-probe.log`, the build argv and hashes.

This is a macOS resource-accounting diagnostic, not a timing comparison; it ran
while broader ASan preparation was underway. An initial diagnostic assertion
used the wrong completion field name (`value` instead of the protocol's
`result`); that fixture mistake was corrected without product changes. Its
failed log remains in scratch. The probe counts native joins and ticket
ownership, not allocator internals or Windows handles.

## Sanitizer and platform boundary

The focused maintained Apple-ASan panel passes **12/12**, in 3.87 seconds;
`focused-asan-ctest.log` retains the complete output. The fresh full instrumented
build, `qa-prep` and correctness run also pass: **2,298/2,298**, 1,293.05 seconds
of CTest, with no sanitizer diagnostic. See `full-asan-ctest.log` and
`full-asan-driver.log`. The complete build/preparation runner logs remain beneath
`/tmp/crexx-channel-release.1v4pFt/asan/asan-logs/`. Apple ASan does not provide
LeakSanitizer. Supported Linux ASan/LSan and Windows thread/process cleanup
qualification remain required on the final code revision.

The fresh sanitizer tree uses Debug, profiling off, network downloads disabled,
`-fsanitize=address -fno-omit-frame-pointer` for C/C++, and
`-fsanitize=address` for executable/shared linking. All instrumented builds and
tests use the maintained runner:

```sh
tools/asan-run.sh --build-dir /tmp/crexx-channel-release.1v4pFt/asan --phase build --build-target test_rxvmchannel_lifetime --build-target test_rxvmchannel_registry --build-target test_rxvmchannel_byte --build-target test_rxvmchannel_process --build-target testChannelRequestRelease --build-target testTaskContextEndpoint --build-target rxvm --build-leaks off --build-jobs 4 --no-live-tail --tail-lines 20
tools/asan-run.sh --build-dir /tmp/crexx-channel-release.1v4pFt/asan --phase ctest --regex '^(rxvmchannel_request_lifetime|rxvmchannel_provider_registry|rxvmchannel_byte_provider|rxvmchannel_process_crash_replacement|testChannelRequestRelease_.*|testTaskContextEndpoint_.*)$' --test-jobs 4 --leaks off --no-live-tail --tail-lines 20
tools/asan-run.sh --build-dir /tmp/crexx-channel-release.1v4pFt/asan --phase full --build-jobs 4 --test-jobs 8 --exclude-label '^performance-measurement$' --build-leaks off --leaks off --no-live-tail --tail-lines 20
```

Leak detection is off solely because Apple ASan does not support it. It remains
required on the supported Linux gate. No first-party suppression or test waiver
is part of this work.

`qualified-source-sha256.json` identifies the final uncommitted product, test and
build inputs. Since the accepted Release measurement, the only changed inputs
are additional provider release assertions, the native test's `qa-prep` wiring
and RexxDoc comments. `post-verdict-classlib-review.json` reconstructs the frozen
class library, verifies its hash and confirms that removing RexxDoc blocks makes
the frozen and final sources identical. No runtime implementation changed after
the Release decision. The full checks cover the final test assertions and build
wiring; documentation and evidence updates do not invalidate those runs. Final
verification confirms that all 27 qualified input hashes still match.

The existing [sanitizer register](../../../docs/SANITIZER-WORKLIST.md) retains
its own closure requirements. This change does not close those entries or
substitute local evidence for their named hosted gates. Read-only hosted
inspection found Build CREXX and CodeQL passing on the base hotfix SHA, as
recorded in `base-hosted-runs.json`; that is not qualification of this dirty
candidate or proof of its missing sanitizer/platform gates.

# VM-STARTUP-01: validated startup mechanism; supplied patch needs repair

Reviewed 12 October 2026 on synchronized `hotfix` at
`105930c7f52bfe6f8c97b151b28bc7f8bf329703`.

**Verdict:** the repeated metadata scans are a real startup cost, and the
attached hash-index patch produces a substantial improvement on the tested
application. Do not integrate the supplied patch unchanged: an index-allocation
failure silently removes the interface registries while linking reports success.
Normal focused correctness passes. Production source and the Release build
were restored after this review. No commit, push, release or email response was
made. This is a bounded review, not acceptance of a production performance edit.

## Inputs and scope

Mike's email, “suggestion from Claude..”, was sent on 11 October at 23:11 CEST.
The attached `vm-startup.patch` targets `77ba820c`, touches only
`interpreter/rxvmintp.c`, and applies unchanged to this hotfix revision. Its
SHA-256 is `457d2e919193543bd91adeb8e505239f8af4160edd5a1817dd63e267aa03128d`.
The public copy has identical bytes.

The external application is [mikebeer/python](https://github.com/mikebeer/python/tree/7aa9654e33c3a8e2b5aa70003a258630f82288f8),
pinned at `7aa9654e33c3a8e2b5aa70003a258630f82288f8`. Its monolithic
`crexx/py.crexx` was compiled once with stock hotfix tools. Every timing uses
that same `py.rxbin`, SHA-256
`9e44ca2d2494ba18f00a3abc003b1940e8a5ce139985999abb12a218decf4740`,
with `-a -c 'print(1)'`. Source/debug metadata was retained; no stripping or
benchmark-local source optimization was performed. Compilation emitted 17
shadowing warnings in the external Python source; there were no C compiler
warnings in the affected stock/candidate product builds.

Mike's [public follow-up](https://github.com/mikebeer/python/blob/7aa9654e33c3a8e2b5aa70003a258630f82288f8/crexx/STARTUP.md)
now reports 2,289 correctness tests plus 12 qualification/stress tests passing
on both stock and patched `77ba820c`, and a 21-test Python suite improvement.
Those are contributor-reported older-revision results, not independently
repeated here. The local checks below qualify this hotfix review only.

## What the change does

Both compatibility registry builders currently loop over `.implements` records,
rescan module metadata to find interface members, then rescan for procedure
names and procedure signatures. A growing number of relationships and metadata
records creates approximately quadratic work. Unrelated local/debug metadata
also makes those scans expensive.

The patch builds temporary hash tables for procedure names (case-insensitive),
procedure pointers, interface-member owners, and class/interface pairs. The
ordinary module/metadata traversal order is preserved: duplicate name and
procedure entries keep the first match, members are appended in their original
order, and class/interface matching remains case-sensitive. Existing signature
comparison and registry sorting remain in use.

Expected lookup cost becomes one metadata traversal to populate indexes plus
lookups and the existing member/signature/sort work, instead of repeatedly
walking all metadata. This is expected/amortized hash-table behavior, not a
worst-case linear-time guarantee. In fact there are **two separate index builds**
per complete method/factory rebuild, each with a counting and population pass;
the patch does not share one index across both registry builders.

Each index is freed when its registry rebuild finishes. No persistent cache,
RXBIN change, public ABI, new opcode or language semantic change is introduced.
The old helpers' linear scans still apply outside the active rebuild. This
improves process startup and dirty relinking after module loading. It does not
establish a steady-state interpreter throughput improvement. Long-lived VMs
already skip clean repeated `rxvm_link()` calls.

## Independent validation

Ordinary Release, profiling OFF, Apple M5 arm64, macOS 26.6.2 / Darwin 25.6.0,
Apple clang 21.0.0, Ninja, `-O3 -DNDEBUG`, four build jobs. AC power, low-power
mode off, no reported thermal/performance warning. Background desktop activity
was present; this was not a fully quiescent release-claim campaign.

Stock and unchanged candidate each pass **77/77** focused tests:

```sh
ctest --test-dir cmake-build-release \
  -R 'interface|selector_descriptor' -LE performance-measurement \
  --parallel 10 --output-on-failure
```

This covers signatures, defaults, factory selection/tie-breaking, imported
contracts, interface linking, and dynamic loading. `dynamic_interface_host`
compiled successfully on stock, and its compile/run controls passed with the
candidate. The email's compile caveat did not reproduce.

Here `rxvm` selects `rxbvm`, and both `rxvme` and `rxbvme` select switch dispatch.
They must not be treated as independent dispatch engines. Additional stock and
candidate **concrete `rxtvm`** runs both pass the interface dispatch suite and
explicit late-loaded interface factory control. Timing also verifies output
`1`, exit zero and empty stderr on both concrete engines.

## Startup observation

Serial, balanced/interleaved stock/candidate rounds; 24 recorded pairs and two
warmups per cell across two unchanged blocks. The second block was added because
initial spans exceeded 10% or relative MAD exceeded 3%. No sample was removed.
`samples.csv` retains all recorded and warmup samples. The Level B capture tools
are retained here and follow `run_cross_runtime.crexx`'s ADDRESS/time capture
pattern. Elapsed time includes process creation, capture overhead, loading,
linking, execution and teardown. Filesystem caches were warm; this is not a
disk-cold or pure-link-only measurement.

| VM/deployment | Stock median | Patched median | Ratio of medians | Mean paired elapsed change, 95% Student-t interval |
| --- | ---: | ---: | ---: | --- |
| `rxvme`, embedded core bundle, selected switch engine | 345.773 ms | 63.802 ms | 5.42x | -80.41%, [-81.11%, -79.70%] |
| `rxtvm`, explicit Level B library | 101.627 ms | 41.786 ms | 2.43x | -61.46%, [-63.97%, -58.95%] |
| `rxbvm`, explicit Level B library | 102.641 ms | 41.978 ms | 2.45x | -61.59%, [-64.17%, -59.01%] |

The separately retained `rxbvme` alias/control result is 344.795 to 63.936 ms,
5.39x; it is another switch/embedded observation, not another engine or a pooled
score. Bare and extended deployments load different module sets, so their
absolute times are separate deployment observations, not dispatch comparisons.

Residual noise remains: candidate relative MAD is about 2.14% for `rxvme` and
7.72%/7.23% for bare threaded/switch; spans remain about 20%/37%/34%. The paired
intervals show a clear improvement on this input, but precise ratios should be
treated as approximate. This single-mechanism, non-representative lifecycle
review omits RexxCPS and the portfolio deliberately; it makes no general,
cross-platform or release-wide speed claim.

Three `/usr/bin/time -l` processes per stock/candidate cell show no material
peak-RSS increase: extended median 97.969 to 98.016 MiB; threaded 47.203 to
47.250 MiB; switch 46.984 to 47.078 MiB. Each of these measured executables grew
176 bytes. Neither observed RSS nor on-disk size hits the standing guard.
Temporary tables still consume memory during rebuild and require correct
allocation-failure handling; peak RSS does not prove they are free.

## Integration blocker and remaining risks

**Reproduced allocation failure:** both builders clear the existing registry,
call `link_index_build()`, and return immediately if it fails. They have a void
return contract. `rxvm_link()` then unconditionally clears each dirty flag and
returns success. No old-scan fallback is used inside these builders, despite
the helpers retaining their old fallback paths outside an indexed rebuild.

The retained diagnostic uses the actual stock/candidate `rxvmintp.c`, compiled
with `-Dcalloc=review_calloc` and `-O0`, linked to the same switch-core inputs.
The wrapper fails only `calloc(1,112)`, the new index header on this arm64 ABI.
The stock file contains no `calloc` calls, so this control does not inject an
unrelated stock allocation failure. Both contexts load the same interface image:

```text
stock:     link=0 methods=5 factories=5 method_dirty=0 factory_dirty=0
candidate: REVIEW: index allocation fault [twice]
           link=0 methods=0 factories=0 method_dirty=0 factory_dirty=0
```

The candidate silently publishes an empty registry as successfully rebuilt.
A repair must preserve valid published state, fall back correctly, or propagate
failure without falsely clearing the dirty flags. The diagnostic is not an
ASan/LSan finding and its O0 executables are never timing inputs.

Other review concerns remain unqualified rather than reproduced defects:

- `_Thread_local` bypasses the existing `CREXX_VM_SINGLE_THREADED` no-TLS and
  compiler-specific choices in `rxvmactive.c`/`rxvmmemory.c`. The C99 application
  port should not acquire an unconditional TLS requirement. Modern MSVC does
  support `_Thread_local` ([Microsoft documentation](https://devblogs.microsoft.com/cppblog/c11-threads-in-visual-studio-2022-version-17-8-preview-2/));
  this review does not claim Windows compilation fails.
- The active TLS pointer has no owning-context check or nesting restoration.
  Separate OS threads have separate pointers, but nested/context scope should
  be explicit before extending this mechanism. No reachable nested-rebuild
  failure was demonstrated in the current call paths.
- Table sizing/count conversions need explicit overflow/allocation bounds, and
  allocation bypasses the VM's usual internal allocation wrapper. Very large
  images and resource exhaustion were not generally qualified.
- Linux, Windows, CMS/WASM, full cREXX suites, sanitizers, persistent-worker
  concurrency, and steady-state portfolio effects were not independently run.

## Practical implications and next decision

The large gain matters for short commands, repeated process launches and large
metadata/module bundles, including interpreters written in cREXX. It is mostly
amortized away in long-lived processes. Library footprint also matters: the bare
Level B deployment is already substantially cheaper in this experiment.

The claim that cREXX has no pre-linked image option is inaccurate: `rxlink`
already creates deployable linked images, selects modules, deduplicates pools,
and can strip source/TRACE metadata. See `docs/ai-context/RXLINK_LINKER.md`.
Final process-local procedure/provider/graph binding and compatibility registry
rebuilding still occur in the VM, so pre-linking does not eliminate this defect.
No pre-linking or stripping performance comparison was performed here.

Recommend retaining the mechanism as a bounded defect-driven repair candidate,
fixing allocation publication/fallback and portable scope first, then applying
the normal focused correctness and first Release verdict gates. Existing RXBIN
007 graph/binding facilities and sharing/narrowing the rebuild index are design
options to assess; none was selected or implemented by this review.

Evidence: raw test/build/resource/fault logs; exact patch; `samples.csv` and
`summary.json`; Level B sampling sources; C failure reproducer and recorded build
commands; `artifact-manifest.json`; recursive `SHA256SUMS`. Reproduction needs
the pinned hotfix source and external Python source, plus a fresh local scratch
directory substituted for the recorded temporary paths. The failed exploratory
DYLD interposition attempts were invalid harness setups (stock also failed),
not product findings or timing evidence; the source-local wrapper above is the
decisive fault proof.

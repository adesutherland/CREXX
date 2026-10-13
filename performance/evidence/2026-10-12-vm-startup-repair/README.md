# VM-STARTUP-01 — repaired candidate, first Release verdict

## Final closeout on updated develop base

**Complete, 12 October 2026.** Adrian accepted the first Release verdict and
directed finishing validation, then requested remote develop's regression fixes.
The existing hotfix branch is now based on fetched
`b1b2a742315738acfdf894ed0d08e93060c6ed8c`, with the reviewed repair.
Adrian subsequently authorized commit and publication. This change publishes
the distinct upstream QA repair and VM repair together through hotfix to develop,
including the completed qualification receipts. Normal automatic Release
product/smoke, optimizer-parity and CodeQL workflows are expected to succeed;
their live GitHub records supply actual hosted results. No release is cut or
manually dispatched overnight gate added, and no CI-status-only follow-up
commit is planned. Unrelated local work is preserved.

The first-verdict material below records the earlier `105930c7f` candidate.
It is historical evidence; `final-input-manifest.json` and the current-base
receipts qualify the final source. One real ownership defect was subsequently
found and repaired: the allocator rejects a release with another worker bound.
Index cleanup now uses the existing owning-worker enter/leave scope and restores
the previous worker. The regression checks actual release success, allocation
cleanup under an active foreign context, and worker restoration. The original
Release test had tracked attempted frees, so Debug's teardown check was decisive.
This normal Debug finding was not an ASan/LSan finding. No new TLS state or
allocator ownership policy was introduced to repair it.

Final validation:

- Current-base focused Release interface/selector checks: **78/78 pass**.
- Current-base focused normal Debug checks, including the strengthened failure
  and context regression: **78/78 pass**.
- Ordinary Release essential/smoke/comprehensive suite: **3,129 verified**.
  The broad run passed 3,127 and exposed two unrelated upstream RXPP fixture
  defects. Their test-only repairs and focused Release/Debug replays pass.
  Passing unchanged tests were retained rather than repeating the broad run.
- Both affected full product/fixture builds complete without C warnings.
- Final source review and source/documentation `git diff --check` pass; public layouts, RXBIN,
  graph/dispatch rules, worker affinity and core sharing remain unchanged.

The upstream QA repairs are separate from the VM mechanism: restore the already
registered FORTRAN block input/runner, checking preserved spacing, comments,
labels, block order and a single deferred call; correct the RexxScript runner's
stale `msg_1=111` assertion to its existing dotted-stem fixture `msg.1=111`.
FORTRAN collection is already implemented/documented. No preprocessor/compiler
product logic or language syntax was changed. The unpatched develop VM produces
the same dotted-stem output, confirming attribution. `upstream-rxpp-stock.log`
retains that control with its explicit RXPA provider supplied.

Earlier former-base QA receipts are also retained: two CMS harnesses were
EXCLUDE_FROM_ALL and initially absent; after building the declared targets,
their two tests passed. The new base's full fixture preparation includes them.
Neither missing-harness result was a VM defect.

Current matched startup confirmation uses the same Python image, 12 balanced
pairs per cell and the unchanged Level B sampler. Both control and repaired
VMs are built from `b1b2a7423` with the current library/provider roots:

| Deployment | Stock median | Repaired median | Paired median elapsed reduction |
| --- | ---: | ---: | ---: |
| Extended `rxvme`, selected switch | 349.928 ms | 74.958 ms | 78.59% |
| Bare concrete `rxtvm` plus Level B library | 111.936 ms | 37.534 ms | 65.40% |
| Bare concrete `rxbvm` plus Level B library | 101.304 ms | 38.536 ms | 62.56% |

All output/exit/stderr checks pass. The extended alias/control is retained in
`current-base-samples.csv`, not counted as another engine. Background benchmarking
was present and the threaded short-process span remains noisy; this bounded
confirmation establishes retained benefit, not precise or release-wide ratios.
The original accepted finer contributor-cost uncertainty remains disclosed.
No general steady-state speed or cross-platform qualification is claimed.

Final source/test/build inputs and VM binaries are hashed. Full fixture builds
and the QA-only corrections leave the measured current VM binaries byte-identical.
The current runtime guide documents the private optional index, ordered fallback,
explicit context scope and allocator cleanup scope. The final review revisited
failure paths, callback userdata lifetime, metadata ordering, canonical-data
ownership, all source closures and the develop reapplication; no unresolved
blocking finding remains in the selected scope. The new index is an internal
lookup helper, not a new semantic authority or permanent cache.

Raw build logs and the original contributor patch retain their emitted
whitespace. The source/documentation whitespace check excludes these verbatim
evidence files; no product diagnostic is filtered or suppressed.

The remainder below preserves the earlier first-verdict evidence and its
then-open qualification status. The final receipts above supersede that status.

---

Date: 12 October 2026. Source: hotfix `105930c7f52bfe6f8c97b151b28bc7f8bf329703`
plus the source/test/build changes hashed in `repair-input-manifest.json`.
Status: provisional repair implemented; first Release decision pending Adrian.
Full normal closeout, focused Debug validation and final documentation remain
open in [the authoritative worklist](../../VM-STARTUP-REVIEW-WORKLIST.md).
No commit, push or release was made.

## Repair and review outcome

The optional index now has explicit context/rebuild scope. There is no new TLS,
ambient active-index pointer, context field or persistent cache. The private
`rxvmlinkindex.c/.h` helper owns lookup mechanics; signature/covariance policy,
method defaults, factory ordering and dispatch remain in `rxvmintp.c`.

Each builder constructs its optional index before clearing its registry. A
failed allocation releases all temporary allocations and returns NULL; every
lookup and member iteration then uses the original ordered metadata walks.
This removes the contributor patch's silent empty-registry return. The void
rebuild APIs and `rxvm_link()` dirty/generation protocol are unchanged.

Temporary storage uses `rxvm_memory_calloc_bytes()` with the explicit owning
worker and `rxvm_memory_release()`. Sizes, power-of-two capacity growth and
sentinels use checked `size_t` arithmetic. Partial indexes never escape their
constructor. Arrays borrow canonical metadata/procedure pointers and are freed
before the rebuild returns; they do not mutate or own loaded metadata.

Careful manual source/diff review covered:

1. **Semantic equivalence:** case-insensitive procedure matching; exact-case
   class/interface/member ownership; first module/metadata match; duplicate
   procedure-pointer selection; ordered member lists; unchanged descriptors,
   covariance comparisons, defaults and factory tie-breaking.
2. **Scope/lifetime:** explicit foreign-context index guards, synchronous
   stack-scoped signature callback userdata, no static mutable production state,
   no index surviving a registry rebuild or crossing late-load generations.
3. **Failure:** all eight optional allocation positions release partial state
   and select the legacy path. This covers index allocation exhaustion; existing
   registry-entry/signature allocation failure contracts are preserved, rather
   than introducing a new transactional registry/public error protocol.
4. **Architecture:** no changes to public context/module/value structures,
   RXBIN or opcode formats, graph binding/authority, site caches, preparation,
   initialization, worker/allocator lifetime, RXPA ABI or dispatch handlers. The
   moved fallback lookups keep their former behavior outside rebuilds. The
   existing two core-object variants retain sharing; no additional VM core is
   compiled for this test.
5. **Conventions/build:** ordinary `rxvm*` private helper naming, existing
   allocator and return conventions, statements/cleanup in surrounding C style,
   standard C99-compatible helper syntax and no TLS extension. Both the desktop
   common core source list and single-threaded port closure include the helper.
   Production includes no fault hook; the regression compiles the actual private
   helper with test-local allocation overrides.

No remaining blocking code/architecture finding was identified in this review.
This is not proof of all untested platform or steady-state performance behavior.
The constrained helper syntax check used the port's existing C99/feature defines.
A stricter exploratory `-Wall -Wextra -Werror` check reports the existing unused
`rxvm_call_native_procedure` definition in the compact-mode core header; this
was not suppressed or used to claim strict-warning C99 port qualification.

## Focused correctness

The full affected Release product builds without C compiler warnings. The
frozen candidate passes **78/78** interface/selector/fallback checks:

```sh
cmake --build cmake-build-release \
  --target rxc rxas rxlink rxvm rxtvm rxbvm rxvme rxbvme crexx test_rxvmlinkindex \
  --parallel 4
ctest --test-dir cmake-build-release \
  -R 'interface|selector_descriptor|rxvm_link_index_fallback_context' \
  -LE performance-measurement --parallel 10 --output-on-failure
```

The permanent `rxvm_link_index_fallback_context` test fails each of eight index
allocations directly, and every allocation position across both method/factory
rebuilds. It verifies partial cleanup/live allocations, registry names,
descriptors and exact procedure/match targets, correct cleared dirty flags,
case behavior, lookup/member order, nested/interleaved contexts, foreign-index
fallback and worker ownership while another context is active. Both registries
retain the fixture's five methods/five factories. No production allocator is
mocked outside the optional helper.

## First ordinary Release observation

Same Apple M5 arm64/macOS 26.6.2/Apple clang 21.0.0/Ninja source build and
profiling OFF, `-O3 -DNDEBUG`, as the original review. AC power and no recorded
thermal/performance warning; desktop load and short-process noise remain.

The exact original Python `py.rxbin` and stock/contributor binaries were audited
against the retained [original review](../2026-10-12-vm-startup-review/README.md)
hashes. No workload was recompiled or altered. Each panel uses the original
Level B sampler, `print(1)`, identical current library/provider roots and serial
balanced/interleaved samples. Same-session stock controls were required to
measure host drift; old-session absolute medians were not called regressions.
Timing includes process creation, capture, load/link, execution and teardown;
it is not a pure-link or disk-cold measurement. All output/exit/stderr checks
pass. No sample was removed.

Stock comparisons use 24 pairs after the initial noise-triggered extension:

| Deployment | Stock median | Repaired median | Paired median elapsed reduction | Mean paired change, 95% interval |
| --- | ---: | ---: | ---: | --- |
| Default extended `rxvme`, selected switch | 347.756 ms | 73.965 ms | 80.07% | -80.20%, [-80.95%, -79.44%] |
| Bare concrete `rxtvm` plus Level B library | 112.167 ms | 37.540 ms | 65.98% | -64.71%, [-66.14%, -63.29%] |
| Bare concrete `rxbvm` plus Level B library | 109.664 ms | 38.759 ms | 64.79% | -64.47%, [-65.55%, -63.39%] |

The extended `rxbvme` alias/control also improves; it is another switch build,
not an independent engine or an aggregate cell. Bare/extended module sets differ.
Ratios of medians, raw ranges and MAD are in `summary.json`; the paired median
percentage is the primary point estimate. Residual noise limits precise ratios,
but the large improvement against stock is decisive for this input.

Contributor-patch comparisons were extended to the policy maximum of 36 pairs
because intervals crossed zero. The repaired/contributor paired medians differ
by -0.019% (extended), -0.076% (threaded) and -0.032% (switch), approximately zero.
The mean percentage intervals are [-1.60%, +2.32%], [-1.56%, +7.51%] and
[-2.38%, +2.54%], respectively. **The fine-grained threaded cost comparison remains
noisy/inconclusive:** it does not prove a regression, but cannot exclude a small
cost above the percentage guard. Its mean absolute difference is +0.937 ms;
no point estimate exceeds both lifecycle guard components. Do not infer a formal
zero-overhead verdict from the similar medians. No further unchanged sampling
was run after reaching the cap.

This is a single-mechanism, non-representative startup verdict. RexxCPS and the
portfolio are deliberately excluded, and no throughput/release-wide/cross-platform
performance claim is made. Recommend retaining the repaired mechanism, with the
threaded fine-cost uncertainty disclosed for Adrian's first-verdict decision.

Three RSS processes per principal stock/repaired cell show median increases of
about 0.078 MiB (extended), 0.063 MiB (threaded) and 0.125 MiB (switch), below the
material guard. Each corresponding executable is 608 bytes larger than stock,
432 bytes larger than the contributor version. The RXBIN workload is unchanged.

## Evidence and next gate

`samples.csv`, `summary.json`, raw correctness/build/resource/host logs, the
retained-input hash audit, frozen source/binary manifest and recursive checksums
retain the decision inputs. Sampling source is unchanged from the original
review (`paired-startup.crexx` and `paired-startup-extended.crexx`). The unused
first sampler invocation failed before measurement because its bytecode was
absent; the exact unchanged sampler was compiled once in scratch before all
retained measurements. It is not a product failure or removed timing sample.

Per `performance/AGENTS.md`, STEP-07 stops for Adrian's first Release verdict
decision. Focused normal Debug, the relevant normal correctness suite, final
post-validation review and runtime-guide updates remain open. Linux/Windows,
native CMS/WASM and sanitizer qualification were not run or claimed. No known
sanitizer finding was produced in this slice. No broad overnight lanes were
dispatched.

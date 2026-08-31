# New-build Phase 3 progress — direct Level B takeover

## Status and programme context

- **Status:** Level G/Unicode lane locally cut over; broad ordinary Debug QA
  complete; sanitizer and hosted exact-SHA qualification pending
- **Branch:** `temp/newbuild`
- **Phase 2 hosted checkpoint:**
  `c0ab085b5f28aa0dcbb669fe040fe8b0fd264cbb`
- **Current `develop` incorporated:** `origin/develop` through
  `9513e20a2af0a7177a487ce21e0f312902b7768b`
- **Develop merge:** `5831bc919773a27ce757fc8879a0c201f2726f5f`
- **Level B builder foundation:** `a56ba5ff0`
- **Complete Level G/Unicode graph:** `ae264c921`
- **Date:** 2026-08-31

Phase 3 makes the agreed ownership split real. CMake/Ninja retains the native
toolchain and qualified Level B bootstrap. A Level B program now owns the
complete Level G and Unicode lane. The former post-bootstrap CMake rules were
removed at cutover, so there is one producer for every generated source,
member RXBIN and the final `rxfnsg.rxbin`.

Elapsed times in this record are deliberately omitted. Other work was active
on the host, so timings would be indicative rather than performance evidence.
Performance tests remain a separate serial workload on a deliberately quiet
host.

## 1. Ownership boundary

| Work | Current owner |
| --- | --- |
| native foundations and C tools | CMake/Ninja |
| core Level B library, exits and class substrate | CMake/Ninja |
| bootstrap of the Level B Level G builder | CMake/Ninja |
| Level G base, HTTP and Unicode members | Level B builder |
| Unicode build tools and generated sources | Level B builder |
| consolidated `rxfnsg.rxbin` | Level B builder |
| remaining Level C, Level L, product and optional lanes | later Phase 3 cutovers |

CMake has one Level G product command. Its declared inputs make Ninja invoke
the builder when a source, Unicode input, bootstrap artifact or tool changes.
Once invoked, the builder's content keys select only the affected reverse
dependency closure.

## 2. Builder design

`tools/newbuild/build_stage_g.crexx` is the Level B controller. It contains
the human-readable waves, exact dependency roots, action keys, failure policy
and publication policy. It uses the public Level B concurrency class surface:
`.taskpool`, `.taskscope`, `.task`, `.completion` and `.channelvalue`.

`tools/newbuild/build_stage_g_worker.crexx` is a deliberately small Level G
adapter. The compiler currently seals task-target expressions to Level G, so
the adapter creates the two task targets required by the Level B controller.
It contains no build dependency or stage policy; the builder itself remains
Level B.

The controller uses a local task pool because its SHA-256 implementation is a
native RXPA provider and therefore is not process-pool eligible. Worker jobs
still run compiler, assembler, linker and generator child processes in
parallel. They never change the VM process's working directory; every child is
started through `cmake -E chdir` with a private action directory.

## 3. Level G and Unicode waves

| Wave | Work | Maximum ready jobs |
| --- | --- | ---: |
| G1 | independent base and private HTTP members | 6 |
| G2 | public HTTP facade | 1 |
| G3 | HTTP server and LLM consumers | 2 |
| U1 | independent Unicode table tools | 6 |
| U2 | dependent source generators | 5 |
| U3-U5 | normalization tool chain | 1 per dependency step |
| U6 | link the five source generators | 5 |
| U7 | execute the five source generators | 5 |
| U8 | compile the five private Unicode runtimes | 5 |
| U9 | public Unicode facade | 1 |

The pool defaults to 30 workers on Apple ARM64 and 5 elsewhere. The actual
ready-job count remains bounded by each wave's real dependency width. The
cache variable `CREXX_LEVEL_G_BUILD_JOBS` provides an explicit override.

## 4. Determinism and incremental rules

- Every `rxc` action compiles an isolated source copy.
- Every action receives only the exact RXBIN/plugin roots required by its
  imports; unrelated source and RXBIN candidates are ineligible.
- Action keys hash the command/tool identity and the content of all declared
  inputs, including generated providers.
- A completed action publishes its stamp only after its expected output
  exists.
- The final library is copied atomically only when its content differs.
- A failed wave prevents every dependent wave and final publication.
- CMake declares all Level G sources, Unicode tool sources and data files as
  inputs to the single builder command, while the Level B program retains the
  finer-grained incremental decisions.

This directly addresses the historic `rxc` source-versus-RXBIN search-order
risk while a library is being built. Compiler-produced metadata remains the
dependency interface; the build does not invent a second metadata format.

## 5. Product defects exposed by the builder workload

The builder is also a stress workload for cREXX itself. Two product defects
were found and repaired before accepting the lane:

1. Level G source signal blocks were incorrectly rejected by a Level B-only
   validator check. Level G is now accepted and has an opt/no-opt compiler
   regression.
2. Leaving a catch-all signal block installed `SIG_IGN` for `SIGCHLD` on
   POSIX, which allowed the OS to reap later ADDRESS children before the VM
   could wait for them. `SIGCHLD` now restores `SIG_DFL`; an ADDRESS-after-
   signal-block opt/no-opt regression retains the failure case.

These are product correctness fixes, not build workarounds.

## 6. Local acceptance evidence

### Direct builder proof

- clean serial base build passed;
- clean parallel base build showed independent overlap and passed;
- immediate base repeat skipped all nine members, link and publication;
- one base-member source change rebuilt that member and the final link only;
- clean complete Unicode graph passed;
- immediate complete repeat skipped every member, tool, generator, link and
  publication action; and
- changing only the case-fold template rebuilt only the case-fold generated
  source, `unicode_casefold`, the public Unicode facade and final link.

### CMake cutover proof

- the existing Debug tree bootstrapped and ran the Level B builder through
  `rxfnsg` successfully;
- an immediate repeat reported `ninja: no work to do`;
- a brand-new Debug build directory with `BUILD_TESTING=OFF` rebuilt every
  prerequisite, bootstrapped the builder from exact inputs and published a new
  `rxfnsg.rxbin` successfully;
- the clean directory's immediate repeat reported no work;
- `ninja -t missingdeps rxfnsg` found no missing generated-file dependency
  across 804 nodes; and
- `ninja -t missingdeps all` in the clean product configuration found none
  across 5,818 nodes.

### Focused QA

- all 7 Unicode RXAS-shape, contract and consolidated-image tests passed;
- all 8 case-fold and case-mapping functional variants passed across `rxbvm`,
  `rxtvm`, optimized and non-optimized modes; and
- the two new focused signal/ADDRESS regressions pass optimized and
  non-optimized in ordinary Debug.

### Broad ordinary Debug QA

The comprehensive correctness selection prepared cleanly and ran 2,224 tests
at jobs 30. Exactly one test failed: the diagnostic catalogue checker found
that the new Level B-or-Level G signal-block diagnostic key had not replaced
the former Level B-only key in the three maintained message catalogues. The
catalogues were corrected, after which the catalogue, localization and driver
diagnostic-plumbing slice passed 3/3. The other 2,223 comprehensive tests had
already passed and their code/build inputs did not change, so they were not
repeated.

## 7. Designed sanitizer scope

`phase3-level-g-sanitizer-artifacts` is a non-owning preparation target for
the bounded Phase 3 sanitizer slice. It builds the complete `rxfnsg` lane,
`rxdas`, the signal/ADDRESS regressions (including parallel controlled child
launch) and all five Unicode functional families. CTest remains a separate
runner phase, so it cannot race an active build:

```sh
tools/asan-run.sh --phase build --build-jobs 4 \
  --build-target phase3-level-g-sanitizer-artifacts

tools/asan-run.sh --phase ctest --test-jobs 5 \
  --regex '^(test_(signal_block_levelg|address_after_signal_block|address_parallel_spawn)_(noopt|opt)|rxunicode_(casefold|grapheme|normalization|case_mapping|codec)_rxas_shape|rxunicode_contract_surface|rxfnsg_rxdas_inline_preserve_smoke|ts_unicode_(casefold|case_mapping|grapheme|codec|normalization)_(rxbvm|rxtvm)_(noopt|opt))$'
```

Use `--build-leaks off --leaks off` on macOS because Apple ASan has no
supported LeakSanitizer. Linux uses both options `on` and remains the required
hosted leak gate. This designed local slice complements rather than replaces
the mandatory full hosted sanitizer jobs.

Full hosted sanitizer CTest excludes the `performance-measurement` label.
Those tests retain their separate ordinary-Release lane because sanitizer
instrumentation and a saturated correctness pool invalidate their timing
relationships; no correctness or sanitizer regression is excluded.

## 8. Remaining Phase 3 work

1. Run the designed sanitizer scope through `tools/asan-run.sh`; any finding
   remains subject to the repository SAN worklist rules.
2. Push the CMake ownership cutover, then require exact-SHA hosted
   Build, Sanitizer and CodeQL success.
3. Use the same direct, single-owner pattern for the remaining Level L and
   other approved post-bootstrap lanes; do not reintroduce a shadow CMake
   graph.

Hosted status must not be described as green until the exact cutover revision
has reached terminal success on the maintained workflows.

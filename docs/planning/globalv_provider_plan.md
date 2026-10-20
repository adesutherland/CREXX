# GLOBALV provider integration plan

## Vision and intended outcomes

Bring the portable GLOBALV implementation from `C:\Temp\CREXX\globalv` into
the CREXX repository as an optional, independently loadable facility. The
existing PC command-line behavior should remain available as `globalv.exe`,
while cREXX applications should be able to use a `GLOBALV` ADDRESS environment
backed by the same C core. Loading the provider must be explicit: ordinary
cREXX programs, the default `library.rxbin`, and the normal VM startup path
must not gain a new dependency or change behavior.

The CMS implementation remains source-compatible reference code only in the
ordinary host build. It must not be compiled by Windows, Linux, or macOS
targets that do not provide CMS headers and services.

## Acceptance criteria

- **AC-01:** The portable GLOBALV core and PC platform layer are present in the
  repository and build as a standalone `globalv` executable on the maintained
  host platforms. Evidence: the focused `globalv` CMake target succeeds and
  `globalv --version` runs.
- **AC-02:** GLOBALV persistence, session reset, groups, listing, stacking,
  purge, null-value deletion, and return-code behavior remain compatible with
  the supplied PC test sequence. Evidence: the focused GLOBALV test passes
  with isolated `GLOBALV_HOME` and `GLOBALV_SESSION` directories.
- **AC-03:** A dynamic and static RXPA provider expose the GLOBALV backend
  without being added to the default cREXX library or VM startup. Evidence:
  provider artifacts are built and a normal cREXX smoke program that does not
  import GLOBALV remains unchanged.
- **AC-04:** The installed cREXX ADDRESS facade supports `GLOBALV` commands
  and functions for initialization, selection, SET/GET/PUT lifetimes, LIST,
  STACK, PURGE, group listing, host-variable bindings, output, diagnostics,
  and status propagation. Evidence: focused no-opt/opt cREXX tests pass using
  the provider and facade explicitly.
- **AC-05:** The provider is safe for concurrent calls within one process and
  does not expose mutable cREXX or platform state across VM sessions. Evidence:
  the provider uses the repository's RXPA process-safety contract and its
  focused test exercises separate calls without state leakage.
- **AC-06:** The documentation identifies installation, explicit loading,
  storage locations, CMS source boundaries, UTF-8/record limits, and known
  persistence/concurrency behavior. Evidence: provider README and relevant
  build/install metadata are reviewed with the implementation.

## Implementation steps

- **STEP-01:** Import the portable core, platform record/storage layer, PC CLI
  front end, CMS adapter/reference build files, and repository license/header
  treatment into a new `lib/plugins/globalv` component. Serves AC-01, AC-02,
  and AC-06.
- **STEP-02:** Split the reusable PC store layer from the CLI-only entry point
  so the standalone executable and RXPA provider share the same storage and
  record implementation. Serves AC-01 and AC-03.
- **STEP-03:** Add the `rxglobalv` dynamic/static provider with a bounded
  command operation and captured output/status surface, protected against
  concurrent in-process calls. Serves AC-03 and AC-05.
- **STEP-04:** Add the Level B GLOBALV ADDRESS facade and install it as a
  separate RXBIN, following the existing SQLite ADDRESS-facade pattern. Keep
  it outside the default `library.rxbin`. Serves AC-04 and AC-06.
- **STEP-05:** Add focused C and cREXX tests for storage semantics, provider
  loading, ADDRESS bindings/output, and failure propagation. Serves AC-02,
  AC-03, AC-04, and AC-05.
- **STEP-06:** Configure and run the smallest decisive build/tests, inspect the
  resulting artifacts, and report any platform limitations without claiming
  unverified CMS or cross-platform behavior. Serves AC-01 through AC-06.

## Decision gates and scope boundaries

The accepted design is an optional RXPA provider plus an installed ADDRESS
facade, not a change to compiler syntax, the default library, or VM startup.
The initial implementation retains the existing fixed-record F 300 storage
contract. Interprocess locking, if not already supplied by the imported
platform layer, must be documented and treated as a follow-up safety item
rather than silently changing the file format.

## Current implementation status

The optional provider, standalone executable, separate ADDRESS facade, CMS
source reference, README, and focused no-opt/opt regression are implemented.
The focused Release build completed with the MSYS Ninja executable because the
previous CLion-bundled Ninja path returned Windows ``operation not permitted``.
Validation on 19 October 2026:

- ``globalv.exe --version`` returned ``GLOBALV build 1 (VM/370plus)``;
- ``cmake --build cmake-build-release --target globalv_plugin globalv_test_artifacts --parallel 1`` passed;
- ``ctest --test-dir cmake-build-release --parallel 1 -R
  \"^globalv_address_(noopt|opt)$\" --output-on-failure`` passed both tests;
- the test covers native provider loading, ADDRESS commands/functions,
  ``GET :name``/``PUTL :name`` host bindings, output, lasting/session
  selection, ``INIT``, ``GRPLIST``, and ``PURGE``.

## Accepted interprocess locking extension

The user accepted a shared, cREXX-independent lock for the PC implementation.
The standalone ``globalv.exe`` and the cREXX RXPA provider must use the same
sidecar ``lock.globalv`` file and native operating-system lock. The lock is
held across the complete GLOBALV read-modify-write operation, so concurrent
writers cannot overwrite one another's changes. The lock handle/file
descriptor is released by the operating system if a process terminates.

- **AC-07:** Independent GLOBALV processes serialize complete operations
  through one shared OS-backed sidecar lock without changing the F300 data
  format. Evidence: a focused contention test exercises the standalone and
  provider paths against the same storage directory.
- **AC-08:** The lock is shared by both front ends and documented for other
  GLOBALV clients. Evidence: both front ends use the common platform layer and
  the README describes the lock file and the requirement that direct clients
  participate.
- **AC-09:** The RXPA provider additionally exposes the upstream-compatible
  direct cREXX operations (`gvset`, `gvget`, `gvexists`, `gvdrop`, `gvpurge`,
  `gvnames`, `gvgroups`, `gvinit`, and `gvcmd`) without removing the existing
  `rxglobalv.command` compatibility surface. Evidence: a focused Level-B test
  exercises direct reads, writes, enumeration, initialization, command output,
  and error signalling.
- **AC-10:** The direct operations use the existing shared PC core and lock
  protocol, so the standalone command and cREXX provider continue to share the
  same storage and interprocess synchronization. Evidence: the existing
  address/lock tests remain green and the direct API test uses the same isolated
  storage directory.
- **AC-11:** The imported upstream GLOBALV implementation compiles and links
  as an isolated PC command target without changing the currently working
  cREXX provider. Evidence: the focused `globalv_upstream` target succeeds and
  its sources are the unchanged snapshot under `lib/plugins/globalv/upstream`.
- **AC-12:** The active dynamic and static cREXX providers link against the
  upstream GLOBALV core and PC storage layer without retaining the old
  `globalv()` backend dependency. Evidence: the provider link succeeds and
  symbol resolution uses `gv_*`/`gv_cmdio` from the upstream sources.
- **AC-13:** The existing cREXX direct operations and the legacy
  `rxglobalv.command` captured-output surface retain their tested behavior on
  the upstream backend. Evidence: the direct, ADDRESS, and lock regressions
  pass after the active backend switch.

- **STEP-07:** Add platform lock acquire/release operations to the shared PC
  storage layer and hold them around the core command transaction. Serves
  AC-07 and AC-08.
- **STEP-08:** Add focused contention/lock-failure coverage, update the
  component documentation, and retain build/test evidence. Serves AC-07 and
  AC-08.
- **STEP-09:** Extend the existing RXPA adapter with the upstream-compatible
  direct operations, translating them through the current command core while
  retaining `rxglobalv.command` for the ADDRESS facade. Serves AC-09 and
  AC-10.
- **STEP-10:** Add the direct-operation Level-B regression and document the
  compatibility boundary with the upstream binding. Serves AC-09 and AC-10.
- **STEP-11:** Add an isolated CMake target for the imported upstream PC core
  and CLI, keeping the existing adapter/backend active until the adapter
  migration is complete. Serves AC-11 and depends on STEP-09's compatibility
  decision before the active provider is switched.
- **STEP-12:** Adapt `rxglobalv.c` to the upstream `globalv.h` API while
  retaining the existing cREXX registrations and `rxglobalv.command` callback
  surface. Use the upstream command I/O callback contract for captured output.
  Serves AC-12 and AC-13.
- **STEP-13:** Switch the active standalone and RXPA CMake targets from the
  migrated backend to the unchanged upstream PC sources, then remove the
  temporary isolated target. Serves AC-12 and AC-13 and depends on STEP-12.

Locking validation on 19 October 2026:

- ``cmake --build cmake-build-release --target globalv globalv_plugin
  globalv_test_artifacts --parallel 1`` passed;
- ``ctest --test-dir cmake-build-release --parallel 1 -R
  "^globalv_address_(noopt|opt)$" --output-on-failure`` passed both tests;
- the permanent ``globalv_lock`` CTest passed, launching two independent
  standalone writers against the same storage directory and verifying that
  both updates survived;
- two independent ``globalv.exe`` processes concurrently updated different
  names in the same ``RACE`` group, both returned zero, and a subsequent LIST
  contained both values.

AC-07 and AC-08 are implemented for the PC storage layer. CMS retains its
platform file-service behavior; the PC sidecar protocol is documented for
other desktop GLOBALV clients.

Direct RXPA extension validation on 19 October 2026:

- ``cmake --build cmake-build-release --target globalv_plugin
  globalv_test_artifacts --parallel 1`` passed;
- ``ctest --test-dir cmake-build-release --parallel 1 -R
  "^(globalv_direct_noopt|globalv_address_noopt|globalv_lock)$"
  --output-on-failure`` passed all three tests;
- the optimized direct and ADDRESS RXBIN artifacts were run directly with
  ``rxvm`` and both reported their required PASS results;
- the CTest optimized variants remain unable to start because this checkout's
  CTest metadata points at the unavailable CLion-bundled CMake executable.

AC-09 and AC-10 are implemented. The existing ADDRESS command surface remains
available, while the active PC storage implementation is now supplied by the
upstream backend and uses its shared ``lock.globalv`` protocol.

Phase 3 upstream backend isolation validation on 19 October 2026:

- The unchanged upstream snapshot under ``lib/plugins/globalv/upstream`` was
  added to the isolated ``globalv_upstream_core``/``globalv_upstream`` CMake
  targets.
- ``cmake --build cmake-build-release --target globalv_upstream --parallel 1``
  passed.
- ``cmake-build-release/bin/globalv_upstream.exe --version`` returned
  ``GLOBALV 3 (VM/370plus)``.
- The active cREXX provider still builds against the restored migrated backend;
  the upstream snapshot has not yet been connected to ``rxglobalv.c``.

AC-11 is implemented. The active backend switch was completed in phase 4.

Phase 4 active upstream backend validation on 20 October 2026:

- The active standalone and dynamic/static RXPA targets now compile the
  upstream PC sources through ``lib/plugins/globalv/CMakeLists.txt``.
- ``cmake --build cmake-build-release --target globalv globalv_plugin
  globalv_plugin_static --parallel 1`` passed.
- ``ctest --test-dir cmake-build-release --parallel 1 -R
  "^(globalv_direct_noopt|globalv_address_noopt|globalv_lock)$"
  --output-on-failure`` passed all three tests.

AC-12 and AC-13 are implemented and verified for the focused no-optimization
regression set. The optimized variants remain separate follow-up coverage.

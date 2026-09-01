# Build-system tooling

This directory contains the production Level B builder and observation-only
build-graph audit tools. The two `build_stage_g` programs are the single
production owner of the Level G and Unicode dependency graph. CMake remains
the team and product orchestrator and invokes that owned graph.

## Level G builder

`build_stage_g.crexx` is the Level B controller. It owns explicit dependency
waves, content action keys, exact compiler import roots, incremental skipping
and atomic final publication. It uses the public Level B concurrency class
surface.

`build_stage_g_worker.crexx` is a narrow Level G adapter for compiler-sealed
task-target expressions. It owns no build graph or stage policy.

CMake bootstraps and invokes the builder through the normal target:

```sh
cmake --build build --target rxfnsg --parallel 30
```

Configure `CREXX_LEVEL_G_BUILD_JOBS` to control the builder's internal worker
pool. The default is 30 on Apple ARM64 and 5 elsewhere. Independent jobs run
within each readable wave; dependency boundaries remain serial.

## Build-graph observation

`cmake_catalogue.py` combines three authoritative configured-build views:

- CMake File API codemodel data for targets, artifacts and dependencies;
- expanded CMake JSON trace events for active custom commands, cleanup and
  compiler import arguments; and
- `ctest --show-only=json-v1` for resolved tests, fixtures, labels, locks,
  serialization and timeouts.

The exporter writes:

- `catalogue.json`: complete normalized observation catalogue;
- `catalogue-summary.md`: human-readable counts and finding classes;
- `manifest-projection.json`: observed targets, custom actions and tests with
  provisional layer, wave and QA-tier classifications; and
- `manifest-validation.json`: structural validation and graph findings.

The projection is deliberately marked `observed-provisional-not-executable`.
It documents and validates the CMake graph; it is not a second build system.

`build-manifest.schema.json` documents the manifest contract. The catalogue
records aggregate import roots and each `rxc` invocation separately, including
ordered source and binary roots, allowed artifact kinds, automatic RXAS
discovery, executable-directory visibility, resolution reports and expected
provider identities. The validator checks required fields, classifications,
action identities, import policy, output ownership, known dependencies and
wave direction. `--strict` also makes current graph findings fail validation.

`import-resolution-report.schema.json` defines the compiler's resolution
evidence. `rxc --import-resolution-report path` records candidate admission,
rejection and same-root replacement, followed by the post-collapse candidate
set. Logical root identifiers such as `@source/0`, `@binary/0` and
`@executable/0` keep reports comparable across checkouts. Candidate discovery
events remain distinct from final provider bindings and do not change compiler
search or precedence behaviour.

Example after a trace-enabled configure:

```sh
python3 tools/build-system/cmake_catalogue.py export \
  --source /path/to/source \
  --build /path/to/build \
  --trace /path/to/cmake-trace.jsonl \
  --ctest-json /path/to/ctest.json \
  --source-commit 0123456789012345678901234567890123456789 \
  --output /path/to/evidence

python3 tools/build-system/cmake_catalogue.py validate \
  --manifest /path/to/evidence/manifest-projection.json
```

Paths beneath source and build roots are normalized to `<SOURCE>` and
`<BUILD>`, making configured graphs comparable across clean work areas. System
CMake definitions are counted but excluded from first-party custom-command
findings. External configured targets remain present in the File API catalogue.

`capture_build_graph.py` configures with File API queries and expanded trace,
performs one clean build, records the resolved CTest and Ninja views, runs the
catalogue exporter and retains compressed raw logs. Timing and peak-memory
fields are always indicative and non-comparative because unrelated host or
cluster activity may be present. It does not execute tests or performance
measurements.

`run_linux_minikube.sh` runs the same capture in a temporary,
resource-bounded Ubuntu 24.04 pod. It streams an exact Git archive into the
pod, copies evidence back, then removes only its uniquely named namespace. Run
Debug and Release separately so local Minikube storage need not hold both build
trees:

```sh
tools/build-system/run_linux_minikube.sh \
  --configuration Debug \
  --source-commit 0123456789012345678901234567890123456789 \
  --jobs 5 \
  --output /path/to/linux-debug-evidence
```

Set `KEEP_CREXX_BUILD_AUDIT_NAMESPACE=1` only when a failed pod must be
retained for diagnosis. The Minikube proof covers Linux ARM64; it provides no
Windows evidence.

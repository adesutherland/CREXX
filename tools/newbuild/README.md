# New-build graph tooling

These tools observe the current CMake build. They do not modify CMake targets,
dependencies or production build behaviour.

`cmake_catalogue.py` combines three authoritative configured-build views:

- CMake File API codemodel data for targets, artifacts and dependencies;
- expanded CMake JSON trace events for active custom commands, cleanup and
  compiler import arguments; and
- `ctest --show-only=json-v1` for resolved tests, fixtures, labels, locks,
  serialization and timeouts.

The exporter writes:

- `catalogue.json`: complete normalized observation catalogue;
- `catalogue-summary.md`: human-readable counts and finding classes;
- `manifest-projection.json`: every observed target/custom action and test with
  a provisional layer, wave and QA tier; and
- `manifest-validation.json`: structural validation and current-graph findings.

The projection is deliberately marked `observed-provisional-not-executable`.
It is an input to the new graph design, not a second build system.

The manifest contract is documented by `build-manifest.schema.json`. Phase 2
retains the aggregate observed import roots for catalogue comparison and also
records every `rxc` invocation separately, including ordered source/binary
roots, allowed artifact kinds, automatic RXAS discovery, executable-directory
visibility, the future resolution report and expected provider identities.
The standard-library validator provides the checks needed during bootstrap,
without requiring a Python package download. It validates required fields and
classifications, action identities, import-policy shape, output ownership,
known dependencies and wave direction. A declared import selection must name
its resolution report and content-digested providers. `--strict` also makes
current graph findings fail the command.

`import-resolution-report.schema.json` defines the canonical observe/enforce
evidence. `rxc --import-resolution-report path` now emits the first
observe-only slice atomically. It records candidate admission, rejection and
same-root replacement at the resolver decision point, followed by the
post-collapse candidate set. Search roots and candidate paths use logical root
identifiers such as `@source/0`, `@binary/0`, and `@executable/0`, so reports
remain comparable across checkout locations; the paired action manifest owns
the physical root mapping. Candidate discovery events are separate from final
provider bindings: recording that a source, RXAS, RXBIN or native candidate
was admitted or replaced is not by itself a claim that it supplied a requested
symbol. Accordingly, `provider_bindings` remains empty in this first slice.
The report does not change the compiler's current search or precedence
behaviour.

Example after a trace-enabled configure:

```sh
python3 tools/newbuild/cmake_catalogue.py export \
  --source /path/to/source \
  --build /path/to/build \
  --trace /path/to/cmake-trace.jsonl \
  --ctest-json /path/to/ctest.json \
  --source-commit 0123456789012345678901234567890123456789 \
  --output /path/to/evidence

python3 tools/newbuild/cmake_catalogue.py validate \
  --manifest /path/to/evidence/manifest-projection.json
```

Paths beneath the source and build roots are normalized to `<SOURCE>` and
`<BUILD>`, making the configured graph comparable across clean work areas.
System CMake module definitions are counted but excluded from first-party
custom-command findings. External configured targets remain present in the
File API catalogue.

`capture_phase0.py` configures with File API queries and expanded trace,
performs one clean build, records the resolved CTest and Ninja views, runs the
catalogue exporter and retains raw logs in compressed form. Its elapsed-time
and peak-RSS fields are always marked indicative and non-comparative because
Phase 0 permits unrelated host activity. It does not execute tests or
performance measurements.

`run_linux_minikube.sh` runs that same capture entry point in a temporary,
resource-bounded Ubuntu 24.04 pod. It streams an exact Git archive into the
pod, copies evidence back, then removes only its uniquely named namespace.
Run Debug and Release separately so the local Minikube disk does not have to
hold both build trees:

```sh
tools/newbuild/run_linux_minikube.sh \
  --configuration Debug \
  --source-commit 0123456789012345678901234567890123456789 \
  --jobs 5 \
  --output /path/to/linux-debug-evidence
```

Set `KEEP_CREXX_PHASE0_NAMESPACE=1` only when a failed pod must be retained for
diagnosis. The Minikube proof covers Linux arm64; no Windows runner is assumed
or claimed by Phase 0.

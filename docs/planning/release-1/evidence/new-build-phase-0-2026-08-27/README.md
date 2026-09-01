# New-build Phase 0 evidence

- **Source baseline:** `dc44d92909e706adc575932ba9ae72f2d6f05b7d`
- **Source ref:** `origin/develop`
- **Capture date:** 2026-08-27
- **Timing status:** indicative and non-comparative

The four directories retain fresh Debug and Release build observations from a
detached macOS ARM64 worktree and exact Git archives built in temporary Linux
ARM64 Minikube namespaces. The namespaces were removed after each capture.

Each bundle contains the configure/build record and resource observation,
compressed logs, CMake trace, CTest JSON inventory, Ninja target/command/graph
views, `ninja -t missingdeps` result, normalized catalogue, provisional manifest
projection, and validation result. Paths in the graph products are normalized
to `<SOURCE>` and `<BUILD>`.

The macOS catalogues were regenerated from their retained traces with the final
Phase 0 exporter after two observer corrections: direct `rxc` process
recognition and explicit classification of `veclib`/the concurrency aggregate.
They are the canonical catalogue counts used in the report. The Linux bundles
retain the original in-pod exporter snapshot; their raw traces and build
results remain valid, but their summaries include 65 `rxc` false positives and
five fallback events corrected by the final exporter.

The capture did not execute CTest or performance measurements. Hosted test
evidence from the latest successful exact-SHA GitHub run is summarized in
`github-build-run-33056734858.md`.

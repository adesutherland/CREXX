# NR-15 D1/D2/D2-hybrid panel environment

Date/time zone: 2026-07-22, Europe/London

- Repository: `develop` at `240b29f456e995928206f04285a7c319612ff022`.
- Local `origin/develop`, live remote `refs/heads/develop` and `HEAD` were all
  the same commit at the final panel capture.
- Host: Darwin 25.5.0 arm64, macOS 26.5.2 (25F84), Apple M5, 10 logical CPUs.
- Ordinary tree: `cmake-build-release`, Release, `-O3 -DNDEBUG`, VM profiling
  off.
- Diagnostic tree: `cmake-build-profile`, Release, `-O3 -DNDEBUG`, VM
  profiling on.

The retained `nr15_stem_panel_helpers.h`, handler include and
`provisional-integration.patch` reconstruct the isolated implementation. The
ordinary and profiling VMs were rebuilt from that exact final PoC before the
final semantic, profile and pilot captures. Opcodes 641-661 are provisional
panel-only operations and are not an accepted RXBIN/ISA assignment.

Target-only builds were used throughout. The panel did not run broad CTest,
sanitizers, install/package validation, formal governed sampling, commit or
push. `test_rxop_metadata` passed directly with 662 dense slots, 603 source
mnemonics, 597 classified, six conservative, 56 reserved and three internal.

The 84 final profiles comprise 72 access/update/reset/growth/key-family cells
and 12 lifecycle/copy cells. Every profile records `summary,result,0` with all
tracking-unavailable counters zero. The small profiling-off Release pilots are
explicitly one-shot tie-break evidence, not a governed performance verdict.

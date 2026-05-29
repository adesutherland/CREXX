# CREXX Roadmap

Status: project direction. This document is not a release contract.

The current release documentation describes what CREXX implements today. This
roadmap is the repository home for future direction, research themes, and
discussion clean-up decisions that should not be mistaken for current language
or toolchain behaviour.

Use this rule of thumb:

* GitHub issues track concrete work that the project is willing to carry.
* Roadmap entries track direction, research, and possible future work.
* Release notes describe what has actually landed.

## Release 1 / Level B Quality Issues

These items are specific enough to track as GitHub issues because they affect
the Release 1 or Level B quality bar.

| Area | Issue | Source discussions | Notes |
|------|-------|--------------------|-------|
| Unicode and text semantics | [#583](https://github.com/adesutherland/CREXX/issues/583) | #155, #162, #194, #231, #470 | Define and verify Level B behaviour for case conversion, `TRANSLATE`, comparison, and byte/codepoint conversion decisions. |
| Tool output paths | [#584](https://github.com/adesutherland/CREXX/issues/584) | #207 | Normalize `-o` / output path behaviour across `rxc`, `rxas`, and driver workflows. |
| RXAS float literal precision | [#585](https://github.com/adesutherland/CREXX/issues/585) | #371 | Add regression coverage that separates stored double precision from display formatting. |
| RXAS instruction coverage | [#586](https://github.com/adesutherland/CREXX/issues/586) | #427 | Inventory and extend instruction regression coverage. |

## Platform Roadmap

Mainframe support is a long-term platform direction, not a Release 1 desktop
release gate.

| Theme | Source discussions | Direction |
|-------|--------------------|-----------|
| z/VM CMS support | #278 | Keep the CMS interest and contact history as roadmap context. Current CMS work is best expressed as deterministic demos and ADDRESS environment compatibility rather than a full platform promise. |
| VM/370 build recovery | #294, #322 | Investigate cross-compilation and source-structure constraints after the Level B desktop release line is stable. |
| MVS/370 porting | #379 | Treat as a future platform project. Likely needs a dedicated maintainer, toolchain notes, and a clear cross-build strategy. |

## Language And Compatibility Roadmap

These items are useful direction but should not become Release 1 commitments
until they are narrowed.

| Theme | Source discussions | Direction |
|-------|--------------------|-----------|
| Classic compatibility BIFs | #129 | `BITAND`, `BITOR`, and `BITXOR` belong with broader Classic/Level C compatibility planning, not as isolated Level B promises. |
| Rexx-style loose comparison | #233 and closed issue #150 | Keep under future compatibility review. Current Level B comparison policy should remain explicit in the language reference. |
| Argument count and optional parameters | #219 | Current `arg()`, `arg[]`, `...`, and `?name` behaviour is documented. Reopen only if a concrete unsupported case appears. |
| Compile-time build metadata | #454 | A `_build_date()`-style virtual function is a possible convenience, but it needs a naming and semantics decision before issue tracking. |
| Unused imports | #467 and closed issue #441 | Already tracked previously. Reopen only if the compiler policy is still wanted and not implemented. |

## Runtime, Backend, And Performance Roadmap

| Theme | Source discussions | Direction |
|-------|--------------------|-----------|
| JIT / MIR / LLVM-style backend research | #331 | Research only. Keep separate from the interpreter and bytecode release contract. |
| RXAS instruction rationalisation | #288, #338, #357 | Review after instruction coverage is better understood. Preserve assembler-user value unless there is a measured maintenance cost. |
| Optimizer and loop super-instructions | #339 | `BCTP` / `IGTBR` optimizer work has landed. Future optimizer work should be benchmark-driven and covered by RXAS optimizer tests. |
| String performance follow-up | #470 | Performance results are useful, but regressions or semantic fallout should be tracked through concrete bugs such as #583. |

## Library, Plugin, And Host Integration Roadmap

| Theme | Source discussions | Direction |
|-------|--------------------|-----------|
| Math library expansion | #384 | Existing `rxmath` is the natural home. Larger ARB / standards-inspired math work should be treated as library expansion after decimal and numeric policy settle. |
| Regex support | #399 and closed issue #414 | RxLite now provides a pure-Rexx regex surface in `rxfnsb`. External/native regex dependencies remain a future packaging decision, not an open Release 1 blocker. |
| System plugin portability | #398 | Keep platform coverage under normal plugin test hardening. Open a fresh issue only for a failing platform-specific test. |
| REXX/SAA compatibility | #424 | Continue through the `crexxsaa` and RXPA host-integration path. Variable-pool emulation needs explicit design before new commitments. |
| Mixed Rexx/native libraries | #432 | Combining Rexx scripts and native plugin functions into one library remains an architecture direction for plugin packaging. |
| Threads and subtasks | #491 | Treat as a design/safety project. The existing process plugin is the safer current execution model; true shared-memory subtasks need a concurrency and ownership design first. |

## Closed As Already Handled Or Stale

The following discussion topics were not converted into issues because the
current tree already handles them, they are covered by existing closed issues,
or the information was too stale to carry forward as a commitment:

* #316: namespace `hello` repro no longer fails in the current compiler.
* #342: `HASH` naming concern is stale; current hash helpers exist under
  `fnv`/`rxmath` and stem internals.
* #467: duplicate of closed issue #441.
* #399: current regex support exists in `rxfnsb`; Windows native dependency
  question was handled in closed issue #414.
* #288: `SAYX REG` was handled in closed issue #409; broader instruction
  cleanup stays roadmap-only.

# Artifact and assembly record

Frozen proof executable:

- path: `cmake-build-release/tests/performance/e3b_rxpa_invoker_ceiling`
- SHA-256: `01669c1a268ff139df6a7d35735e55db8765d0c5d54df07e0c9242ff2c47e084`

Apple ARM64 Release owner shapes:

| Loop | Bytes | Call shape |
| --- | ---: | --- |
| selected direct/locked | 72 | load invoker, load native function, `blr` |
| raw direct | 72 | direct `bl _rxvm_callfunc_direct` |
| per-call branch | 92 | capability load, `tbz`, split direct/locked `bl` targets |

The selected loop contains no capability branch. This disposable executable is
a machine-level proof; no production VM artifact-size comparison is claimed.

# PERF3-05-R5b profile-20 default closeout

This bundle closes the approved Apple default selection for the RXVM handler
placement framework. New builds now select `profile-20`; literal `all-inline`
and `all-outline` remain explicit controls. The change does not alter handler
tiers, RXAS/RXBIN, the public/plugin ABI, canonical bytecode or VM selection.

## First Release verdict

A fresh ordinary profiling-off Release configuration with no handler-panel
option selected `profile-20`. Its `rxtvm` and `rxbvm` products are byte-for-byte
identical to products built from the same source with explicit
`-DCREXX_VM_HANDLER_PANEL=profile-20`:

| engine | SHA-256 |
|---|---|
| `rxtvm` | `c2d50abe7bdb4eb8c8caf4cbefb6edc9ef2cf894f83ca9c567059808e265e21b` |
| `rxbvm` | `ab6f491bb1181cbae7184f36180f337380348e6628ccd5cf2e90576a3802b5c3` |

Both products pass the focused Bounce output oracle. The retained R5
profile-20 timing is therefore the decisive result without a redundant timing
sample: Clang improves the seven-workload aggregate by 3.857%/3.152% for
`rxtvm`/`rxbvm`; GCC improves 3.175%/9.646% overall while retaining the
explicitly accepted 10.072% threaded Bounce loss.

## QA result

- fresh default profiling-off Release: 2,002/2,002 tests pass;
- fresh default Debug: 2,002/2,002 tests pass;
- fresh default profiling-enabled Release: six focused profiler, effective
  placement, CSV/table and documentation tests pass;
- the profiling CSV reports `CALL_FUNC,outline` under the default panel for
  both concrete engines; and
- maintained-source `git diff --check` and the bundle checksum audit pass.

The first focused profiling attempt named the generated
`tests_procedure_profile` file instead of its CMake target. Ninja rejected the
unknown target, so CTest then found no executables. This was a test-setup error,
not a product failure. Rebuilding with `run_tests_procedure_profile` produced
the six-test passing result retained here.

## Compiler context and residual work

The retained profile-20 median point estimates are faster under Apple Clang
than GCC in all 14 workload/engine cells. The derived all-seven geometric mean
advantages are about 23.8% for `rxtvm` and 41.5% for `rxbvm`. The compiler
matrices were separate sessions, not a paired compiler-selection experiment,
so this is contextual Apple-host evidence rather than a portable ranking.

Profile-20 is provisional. Intel Linux is the next requested platform check.
During Release 1 finalisation the exact panel must be regenerated from a wider
current portfolio, including private/fused dispatch and new instructions. The
never-inline ledger and versioned panel-membership diff must be reviewed so
panel movement remains visible as code and workloads evolve.

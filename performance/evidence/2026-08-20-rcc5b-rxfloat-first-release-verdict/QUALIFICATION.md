# RCC-5B post-verdict qualification

Status: retained local evidence from 2026-08-20. This is not the consolidated
RCC-5 full-QA closeout.

## Qualification

- Focused Debug qualification passes 21/21.
- The final repeated full Debug sweep passes 2,292/2,292 in 350.78 seconds. The
  preceding sweep had one `gate_f_channel_process_rxtvm` `SIGPIPE`; that exact
  test passed its immediate serial retry before the clean repeated sweep.
- A scratch-installed product builds and runs optimized and unoptimized float
  tests under both concrete VMs without an explicit provider argument.
- The scratch-installed `crexx -native` path selects the canonical `rxfloat`
  archive and the resulting native executable passes.
- The exact Apple-ASan transitive-import/provider-reload reproducer passes.
  Focused sanitizer coverage for the four optimized/no-opt concrete-VM float
  cells, bundled-provider concurrency, and static-alias binding passes 6/6.
- Incremental-tree cleanup removes retired `rx_float` and legacy provider
  artifacts. Disassembly confirms both canonical `rxfloat.*` and compatibility
  `rxmath.*` declarations name provider `rxfloat`.

These results remain valid and useful, but broad Debug, sanitizer,
install/package, and documentation closeout are not repeated after every RCC-5
subphase. RCC-5 receives one consolidated full-QA closeout after its final
approved subphase.

## Independent finding

A broad Apple-ASan linked-artifact fixture exposed an RXAS SSA
heap-use-after-free while assembling optimized AWFY Towers. The report points
to `rxas_flow_value_node()` retaining a pointer across `flow_ssa_grow()`.
This is independent of RCC-5B and the RXPA provider-lifetime correction and is
recorded for a separately authorized investigation.

The later focused float-contract expansion independently covers all 37
canonical procedures, every compatibility alias, and nullary/unary/binary
native arity failures. Its final optimized/no-opt, both-VM, structural, and
native-boundary set passes 6/6. The shared comparator is typed test support,
not the source of those expected values. RCC-5A and focused RCC-5B are
complete; RCC-5C remains in progress under the separate mathematics validation
strategy; and RCC-5D+ has not started. Consolidated full QA remains an
end-of-RCC-5 activity.

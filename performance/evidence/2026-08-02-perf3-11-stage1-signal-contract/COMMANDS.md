# PERF3-11 Stage 1 commands

The source/effect audit used the opcode-aligned `rxopeffects.h` inventory and a
read-only parser over `START_INSTRUCTION` blocks in `rxvmintp.c`.  Counts were
cross-checked with `rg` against handler-management, call, reference, indirect
write and numeric-context families.  The lexical signal scan recognizes direct
`SET_SIGNAL`, `RXSIGNAL_IF_RXVM_PLUGIN_ERROR`, and the shared
`interrupt_table_oom` path; it is intentionally reported as a lower bound.

The current phase fixture was assembled with and without optimization and run
through both VMs from a fresh temporary working directory:

```sh
rxas -n -o phase-noopt current-signal-phase.rxas
rxas    -o phase-opt   current-signal-phase.rxas
rxvm  phase-noopt
rxbvm phase-noopt
rxvm  phase-opt
rxbvm phase-opt
```

Each command printed:

```text
PASS: PERF3-11 current signal phase oracle
```

The same four-way matrix for `action-fail.rxas` printed:

```text
PANIC: PERF3-11 deliberate fail action (SIGNAL OTHER)
  at module 1 (...) address 3 (0x3)
```

and returned 30 in every case.

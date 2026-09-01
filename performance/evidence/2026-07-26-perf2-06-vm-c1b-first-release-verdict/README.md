# PERF2-06 VM-C1b first Release verdict

Status: **accepted by explicit trade-off after the mandatory first Release gate**

This package retains the first ordinary profiling-off Release verdict for the
approved VM-C1b shared/COW interrupt-policy slice.  It compares the exact
retained current-HEAD product with an isolated candidate on optimized List 100,
Permute 50 and Sieve 50 in both VM modes.

VM-C1b passes the focused correctness and machine-work gates.  It removes the
1,280-byte interrupt-table copy from every child call, reduces `stack_frame`
from 1,432 to 168 bytes and adds no dispatch work.  It also keeps `run()` within
the approved size ceiling in both VMs.

The timing guard is nevertheless **clear adverse**: `rxbvm` Sieve has a
paired mean `+5.368694%` and 95% interval
`[+4.720473%, +6.016915%]`.  The 12-pair block is decisive and was not
extended. A follow-up diagnosis excludes executed COW work and one-time root
allocation, and identifies Apple-Clang global code layout/register allocation
in the monolithic flattened `rxbvm` interpreter as the high-confidence cause
class. Adrian explicitly accepted the trade-off. Full Debug and ordinary
profiling-off Release build/CTest closeout both pass 1,924/1,924.

## Package map

- `VERDICT.md`: decision result and exact paired table.
- `PROVENANCE.md`: source, product, input, host and validation identity.
- `COMMANDS.md`: reproduction and accepted-closeout commands.
- `CODE-LAYOUT-DEBT.md`: accepted debt `PERF2-06-D01`, diagnostic limits and
  the required future compiler/architecture matrix.
- `product-size-summary.csv`: file, Mach-O text and `run()` sizes.
- `paired-summary-12.csv`: evidence-local paired confidence reduction.
- `timing/initial-12/`: all raw samples, outputs and runner summaries.
- `correctness/`: focused Debug logs.
- `diagnostics/`: work-500 scale control, native samples and alignment-pad
  control for the adverse `rxbvm` Sieve cell.
- `build/`: isolated ordinary Release build/rebuild logs.
- `checksums.sha256`: integrity ledger for every other retained file.

# PERF3-12B Mac scorecard validation

- Clean detached product: `44d8b6a7ecd7800979b5db992c14bc7182aa89dd`,
  `Release`, `-O3 -DNDEBUG`, `CREXX_VM_PROFILING=OFF`.
- Focused optimized-workload and matrix self-test panel: `10/10` pass.
- Full normal Debug build passes and broad CTest is `2,039/2,039` in 227.93
  seconds.
- Strict GNU90 syntax checking with `-Wall -Wextra -Wconversion
  -Wsign-conversion` is warning-free across the RXAS flow/proof and opcode
  metadata sources.
- Focused Apple AddressSanitizer is `5/5` for metadata, graph, semantic-batch
  and optimized/unoptimized joined-key tests. The runner's initial leak-enabled
  probe is retained: Apple ASan reports `detect_leaks` unsupported, so the
  successful run uses `detect_leaks=0` while retaining address instrumentation.
- Formal matrix: 29 cells, 58 warmups, 290 recorded observations and
  `348/348` executions pass with zero driver stderr.
- Noise rule selects zero cells. No append is permitted or run; samples
  removed: zero.
- Every cell has exactly ten recorded observations. The summary has 29 rows,
  the ratio table has all 20 common-five component comparisons and the
  geometric-mean table has all four required aggregates.
- Common membership is exactly Sieve, Permute, Bounce, Richards and Base64
  (`N=5`). RexxCPS and Towers remain separately reported qualified lanes.
- Artifact inventory: 84 SHA-256-bound rows, including the final product,
  qualified comparator sources/runtimes and B4/B5 authorities.
- Static summary inventories the exact seven optimized RXBINs plus the shared
  library. RexxCPS is 1,210 instructions, `.locals=104` and 68,361 bytes;
  the selected B4 H1 route also had 1,210 instructions and `.locals=104`.
- The one-instruction Sieve increase versus K04e predates production H1 and is
  present in the retained B4 zero-candidate control. It is not attributed to
  H1; that control and production emitted byte-identical Sieve images.
- `rxvm`, `rxbvm` and `library.rxbin` are SHA-256 bound. Their sizes are
  982,424, 999,144 and 933,969 bytes respectively.
- The Apple M5 host remained on AC with low-power mode off. Capture was serial
  and runtime order rotated by workload/round.
- Artifact-inventory and static-summary Level B self-tests pass.
- A final clean Release `rxas` rebuild after the unsigned capability-mask type
  cleanup is bit-identical: SHA-256
  `98346fa65da026ca135b9d8d67db4459838b5c3ec4effc01db30ead02d68040b`
  before and after. The cleanup therefore does not alter the scorecard product
  or generated workload images.

The recursive checksum file closes the frozen B6 documentation and QA records
and verifies independently from the repository root.

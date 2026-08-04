# PERF3-06 validation record

- Predecessor recursive authorities replay: PERF3-01 `101/101`, PERF3-02
  C1abc `12/12`, PERF3-03 C4 `19/19`, PERF3-10 TRACE/ITOS `37/37`.
- Clean detached product: exact commit `5fbe36049`, `Release`, `-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=OFF`.
- Focused optimized workload plus matrix self-test CTest: `10/10` pass.
- Exact formal arguments across both VMs: `14/14` pass, including two
  canonical-default RexxCPS provenance markers.
- Initial formal matrix: 29 cells, 58 warmups, 290 recorded observations,
  `348/348` executions pass.
- Governed append: the initial noise flags and three-row append manifest agree
  exactly; 30 recorded observations and `30/30` executions pass.
- Samples removed: zero. Second append: forbidden and not run.
- Independent summary audit: all 20 common ratios and all four `N=5`
  geometric means recompute within output rounding tolerance.
- Identity audit: all 47 current benchmark-source, comparator-runtime/JAR and
  retained generated-product paths match PERF3-01 SHA-256 identities.
- Artifact inventory: 79 SHA-256-bound rows generated successfully.
- Static audit: both seven-workload disassembly summaries complete and the
  compact delta ledger recomputes exactly from them.
- Repository/evidence manifest copies compare byte-for-byte; `git diff
  --check` passes before recursive closure.

The recursive `checksums.sha256` is generated only after the final evidence
files above are frozen.  The worklist records its independent verification
count so the checksum file does not self-reference.

# NR-08 bounded reference-lifetime PoC evidence

Status: frozen historical bounded PoC; the later ordinary Release verdict was
accepted and fully closed out in `../2026-07-17-nr-08-first-release-verdict/`

This bundle compares exact current RexxCPS 2.2d artifacts with a reversible,
compiler-side NR-08 PoC. It is design/PoC evidence only. It is not the mandatory
ordinary profiling-off Release verdict and contains no formal product timing.

The canonical dynamic cell uses the CTest-only `--smoke-count 1` contract. Its
effective source values remain `count=1`, `averaging=100`, so it is explicitly
noncanonical. The opaque diagnostic uses `A Off 1 1`. Both are bounded
correctness/count cells, not benchmark scores.

`collect_profiles.zsh baseline` and `collect_profiles.zsh candidate` copy the
four exact optimized/unoptimized RXAS/RXBIN artifacts, run schema-4 instruction
profiles with both `rxvm` and `rxbvm`, retain stdout/stderr, inventory symbols
and static lifecycle/reference/copy operations by procedure, and write exact
hashes and sizes. The profiling build is Release with
`CREXX_VM_PROFILING=ON`; the images come from the ordinary profiling-off
Release build.

The instruction profile is image-wide. Per-procedure and per-symbol lifecycle
sites come from the exact generated RXAS. Imported library and generated TRACE
handler code are kept distinct from authored `main`, `cps_subroutine`, and
`fail` procedures; aggregate dynamic counts are not mislabelled as
authored-procedure-only counts.

The historical NR-05 RexxCPS rows are version 2.2c and are not a baseline for
this bundle.

## Frozen result

The candidate applies the worklist's symbol-local Gate A and changes only
scope-exit `ENDLIFE_REG` emission. It does not change `NULL_REG`, copy rules,
opcode effects, ISA, RXBIN format or either VM.

- focused final reference/structural matrix: 26/26 pass;
- focused stem regression matrix: 3/3 pass;
- reference inline/block controls: optimized and no-opt RXAS byte-identical;
- canonical no-opt dynamic `ENDLIFE_REG`: 1,301,489/1,301,497 to 1,516 on
  `rxvm`/`rxbvm`;
- canonical opt: 1,379,697/1,379,705 to 5,616;
- opaque no-opt: 13,507 to 31 on both VMs;
- opaque opt: 14,326/14,318 to 72;
- generated TRACE static cleanup is unchanged at 10 no-opt and 96 opt.

Program-cell static NULL/copy/unlink/reference-op counts are unchanged. The
formal-verdict audit found that this bundle's original static script omitted
instructions following branch labels. The corrected label-aware replay leaves
the retained images and dynamic profiles unchanged and reports the imported
optimized library at `ENDLIFE_REG` 8,006 to 151, while NULL remains 4,293,
copies 8,478, unlink 1,758 and reference operations 7. Exactly 263 of 629
procedures change, every change is an `ENDLIFE` reduction and no operation
increases. The previously reported NULL/copy DCE cascade was an analysis
undercount and did not occur. Corrected phase checksum manifests pass.

No timing, broad CTest, sanitizer, package/install or formal Release verdict
was run in this bounded bundle; those accepted-verdict results remain separate
at the path above.

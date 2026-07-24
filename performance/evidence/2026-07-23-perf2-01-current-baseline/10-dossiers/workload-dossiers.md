# PERF2-01 workload gap and cost dossiers

Status: Gate A selection input; no production candidate has been implemented.

All product times and throughputs below come from the ordinary profiling-off
Release product built from exact commit
`d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`. The exact runtime hashes are in
`../01-artifacts/formal-product-sha256.txt`; exact source, module, image and
library hashes and commands are in the retained capture manifests. Program
image hashes and module-set footprints are reproduced below. Profile counts,
profile timings, RXSEQ and native samples are attribution only.

For the five qualified common workloads, throughput is equal work per second
and higher is better. `2x GM gain` is the uniform improvement still required
to move the current five-workload geometric mean to 2.00x ooRexx: 2.241605x
for `rxvm` and 2.398412x for `rxbvm`. It is not a per-cell claim. RexxCPS is a
separately disclosed benchmark-native MCPS comparison between cREXX 2.2d and
canonical Classic RexxCPS 2.2.

## Gap ledger

| Workload | Comparability | rxvm / rxbvm throughput | ooRexx throughput | rxvm / rxbvm ratio | Gain to parity | Gain to 1.50x | 2x GM gain |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sieve | qualified common | 5,103.790 / 3,858.241 | 713.416 | 7.154018 / 5.408124 | 1.000000 / 1.000000 | 1.000000 / 1.000000 | 2.241605 / 2.398412 |
| Permute | qualified common | 675.150 / 632.699 | 314.833 | 2.144475 / 2.009638 | 1.000000 / 1.000000 | 1.000000 / 1.000000 | 2.241605 / 2.398412 |
| Mandelbrot | cREXX equivalent-port diagnostic; ooRexx checksum failure; excluded | n/a | n/a | n/a | n/a | n/a | n/a |
| Towers | cREXX equivalent object port; ooRexx procedural diagnostic is not an object/allocation equivalent | n/a | n/a | n/a | n/a | n/a | n/a |
| Bounce | qualified common | 329.907 / 315.986 | 994.463 | 0.331744 / 0.317745 | 3.014373 / 3.147178 | 4.521559 / 4.720767 | 2.241605 / 2.398412 |
| Storage | cREXX node-wrapper diagnostic; not comparable for allocation scoring | n/a | n/a | n/a | n/a | n/a | n/a |
| List | disclosed weak-reference arena; aggregate review remains open | n/a | n/a | n/a | n/a | n/a | n/a |
| Richards | qualified common | 1.739086 / 1.711259 | 11.350265 | 0.153220 / 0.150768 | 6.526563 / 6.632707 | 9.789845 / 9.949061 | 2.241605 / 2.398412 |
| JSON | path/count capability diagnostic; not comparable to supplied-DOM cells | n/a | n/a | n/a | n/a | n/a | n/a |
| Base64 | qualified common | 1,536.604 / 1,641.248 | 2,119.321 | 0.725045 / 0.774421 | 1.379225 / 1.291287 | 2.068837 / 1.936931 | 2.241605 / 2.398412 |
| RexxCPS | disclosed cREXX 2.2d versus canonical Classic 2.2; not in common GM | 29.350347 / 27.132650 MCPS | 40.064945 MCPS | 0.732569 / 0.677217 | 1.365059 / 1.476632 | 2.047588 / 2.214948 | n/a |

The machine-readable qualified rows are `gap-ledger.csv`; the aggregate target
calculation is `common-geomean-target.csv`. Values are not synthesized for
excluded cells.

## Sieve

- Artifact and timing: 72 program instructions, 4,864-byte image
  (`75d264a9acb4f17aab6c85e1a2991904580950ec6cd4da1c235d1010319e4f7f`),
  54,794 instructions and 862,937 bytes with the shared library; 6,825,375
  dynamic instructions in both VMs. Product medians are 1.077213 / 1.428256 s;
  peak RSS is 17,350,656 / 17,334,272 bytes.
- Dynamic shape: `awfy_sieve.main`; top opcodes are `IGT_REG_REG_REG`
  1,087,001, `ISETUNLINK_REG_REG` 803,450 and the fused indexed load 803,450.
  The top transitions are 4,968,371 sequential and 1,857,003 taken-branch
  transitions; 1,053,450 backedges.
- Frames and values: no calls, one fresh frame, 19 frame-entry work units;
  52 copies / 10 bytes, 506,584 clear-reset-destroy operations / 120,209 bytes,
  3 conversions. VM allocation requests total 8,319 / 4,232,616 bytes; the
  largest request is 2,031,616 bytes.
- Native ownership: `run` 272 samples, `clear_value_contents` 71,
  `maybe_trim_attribute_storage` 40 and `reset_value_storage_for_reuse` 22.
  No targeted system-heap profile was justified after portfolio sampling.
- Site/cache: zero selector attempts; no current selector site is exercised.
  Selected owner is compiler/RXAS loop and indexed-array shape, with VM
  clear/reset as a guard metric. It already exceeds the 1.50x cell target and
  is a regression control, not a first optimization target.

## Permute

- Artifact and timing: 227 instructions, 11,933-byte image
  (`b948116f19edf8bc4b208a17f1f7380050b0df5fa537b0a6aa6bb4ce15dc064f`),
  54,949 instructions / 870,006 bytes with the library; 12,906,926 dynamic
  instructions. Product medians are 7.373570 / 7.896069 s; RSS is
  17,563,648 / 17,727,488 bytes.
- Dynamic shape: `permutebenchmark.permute` is called 432,950 times. The fused
  link/attribute/add opcode executes 2,014,400 times, `UNLINK_REG` 1,440,800
  and `COPY_REG_REG` 1,008,101. Transitions are 11,355,972 sequential, 685,053
  branches and 432,950 call/return pairs.
- Frames and values: 7 fresh / 432,944 reused frames and 12,988,533 frame-entry
  work units; 10,260,302 copies / 74,017,610 bytes; 3,594 reset-family
  operations / 2,813 bytes. Allocation requests are 866,577 / 270,904 bytes;
  max 9,936 bytes.
- Native ownership: `copy_value` 670, `run` 479,
  `maybe_trim_attribute_storage` 298 and `memmove` 90 samples.
- Site/cache: zero selector attempts. Selected owner is compiler/inliner plus
  value/frame work; despite the residual call/copy footprint, both VMs exceed
  1.50x ooRexx, so Permute is an essential call-heavy regression control.

## Mandelbrot

- Comparability: cREXX is a correct equivalent port, but ooRexx decimal
  numerics fail the common checksums and NetRexx retains an arithmetic-XOR
  adaptation. No cross-runtime ratio is published.
- Artifact and timing: 114 instructions, 8,208-byte image
  (`d30bb23e1871094e0a58d31cd26828ba561fc014787dafb5fddd3110960c6a18`),
  54,836 instructions / 866,281 bytes with the library; 129,014,253 dynamic
  instructions. Product medians are 0.147691 / 0.173555 s; RSS is
  17,285,120 / 17,383,424 bytes.
- Dynamic shape: one `main` frame. `FADD` and `FMULT` each execute 24,585,141
  times; `BRF` 16,958,927. There are 111,754,507 sequential and 17,259,745
  branch transitions, 8,445,547 backedges and 501,003 conversions.
- Values/allocation: one zero-byte copy; 214 reset-family operations / 183
  bytes; 121 requests / 39,208 bytes, max 8,880. The valid native rerun records
  210 samples in `run`; the earlier size-2000 sample without a checksum
  reference is retained but rejected.
- Site/cache: zero selector attempts. Capability/equivalence owns publication;
  the measured implementation owner is RXAS/VM floating arithmetic and branch
  dispatch, not frames or heap.

## Towers

- Comparability: cREXX is an equivalent object port. The ooRexx procedural
  stem/numeric-node version is a diagnostic, not an object/allocation score.
- Artifact and timing: 598 instructions, 28,975-byte image
  (`94f3b1406862240e44c2cb12c16113b9e982fce6daf7f3979b5d0b3bb89e5fff`),
  55,320 instructions / 887,048 bytes with the library; 7,716,996 dynamic
  instructions. Product medians are 0.680525 / 0.694418 s; RSS is
  21,118,976 / 21,151,744 bytes.
- Dynamic shape: `pushdisk` 82,050 calls and `movedisks` 81,900. `COPY_REG_REG`
  executes 1,556,321 times and attribute linking 901,300. Transitions are
  6,351,222 sequential, 1,037,873 branches and 163,950 call/return pairs.
- Frames and values: 14 fresh / 163,937 reused frames, 5,407,993 entry work
  units; 61,708,842 copies / 481,220,170 bytes; 41,210,261 reset-family ops /
  50,521,626 bytes. Requested allocation is cumulative 21,801,918 operations /
  7,444,444,320 bytes, max 14,160.
- Native/system ownership: `copy_value` 755 samples, trim 191, clear 138,
  allocator free 108 and reset 74. The targeted heap run reached a 9 MB high
  water, 260 MB cumulative allocation and 481,235 allocations / 473,841
  deallocations; it is a scaled diagnostic, not formal RSS evidence.
- Site/cache: zero selector attempts. Capability/equivalence owns comparison;
  measured ownership is value/frame work and compiler/inliner call shape.

## Bounce

- Artifact and timing: 374 instructions, 17,839-byte image
  (`b1cc4416c538f3bf4cf9b73f85735712d88ead7091f931aa20e77b4216defb2b`),
  55,096 instructions / 875,912 bytes with the library; 22,912,326 dynamic
  instructions. Product medians are 6.671088 / 7.004254 s; RSS is
  17,760,256 / 17,858,560 bytes.
- Dynamic shape: factory 10,000 calls and benchmark `run` 100. Attribute link
  executes 4,222,600 times, unlink 3,065,000 and branch 2,142,702. Transitions
  are 19,617,822 sequential, 3,274,303 branches and 10,100 call/return pairs.
- Frames and values: 3 fresh / 10,098 reused frames and 192,514 entry work
  units; 7,483,002 copies / 51,142,411 bytes, 10,100 moves / 400,000 bytes,
  and 920,268 reset-family ops / 1,207,076 bytes. Allocation requests are
  260,586 / 83,356,312 bytes, max 15,872.
- Timed/native ownership: `MKREF_REG_REG` consumes 262.9 / 263.9 ms in the
  bounded profile. Two rxvm samples put `rxvm_reference_storage_in_value_tree`
  at 1,228 / 1,221 samples; rxbvm confirms 1,201. `run`, `copy_value` and trim
  are far behind. The full heap diagnostic has 9 MB high water, 14.9 MB
  cumulative allocation and 9,411 allocations / 6,060 frees; loader material
  dominates retained high water, while `copy_value` remains visible by count.
- Site/cache: zero selector attempts. Selected owner is value/reference work
  and the reference-construction VM path; this is a 3.01x / 3.15x parity gap
  and the strongest evidence for the first bounded value/reference PoC.

## Storage

- Comparability: the cREXX node plus `.object[]` wrapper changes allocation
  work materially. It remains a correct diagnostic and is not reclassified.
- Artifact and timing: 208 instructions, 12,631-byte image
  (`ed71479725ceb086569668a082c842e940238e8da5aef2bd861d2afc5ff4867e`),
  54,930 instructions / 870,704 bytes with the library; 6,342,046 dynamic
  instructions. Product medians are 1.753955 / 1.753870 s; RSS is exceptional
  at 509,001,728 / 509,050,880 bytes.
- Dynamic shape: `objectarrayappend` 95,560, `buildtreedepth` 54,600 and
  `allocateleaf` 40,960 calls. `COPY_REG_REG` executes 620,161 times.
  Transitions are 5,134,892 sequential, 824,893 branches and 191,130
  call/return pairs.
- Frames and values: 10 fresh / 191,121 reused frames, 3,003,524 entry work
  units; 36,330,492 copies / 285,682,570 bytes; 150,160 moves / 1,310,400
  bytes; 86,106,431 reset-family ops / 145,044,309 bytes. Allocation requests
  total 58,395,050 / 20,250,188,288 cumulative bytes, max 8,352.
- Native/system ownership: copy 541, clear 267, free 149, trim 133, malloc 119
  and reset 110 samples. The targeted heap run reaches 122.8 MB high water,
  374.8 MB cumulative allocation and 688,429 allocations / 475,568 frees;
  its high-water tree attributes 117 MB and 219,840 recursive operations to
  `copy_value`. This scaled native result is directionally consistent with,
  but does not replace, the 509 MB formal peak-RSS lane.
- Site/cache: zero selector attempts. Capability/equivalence owns comparison;
  value/allocation work owns the current implementation cost and is a safety
  outlier for any copy/reference PoC.

## List

- Comparability: correct disclosed weak-reference arena; aggregate review is
  still open, so no ooRexx ratio is published.
- Artifact and timing: 233 instructions, 16,186-byte image
  (`00b177a6964a0136eb6e622d0cfcee77fb1050967b3de084c819a78b0d03cd64`),
  54,955 instructions / 874,259 bytes with the library; 50,752,126 dynamic
  instructions. Product medians are 0.182139 / 0.211647 s; RSS is
  17,842,176 / 17,973,248 bytes.
- Dynamic shape: `listelement.next` 3,820,600 calls; `isshorterthan` and `tail`
  280,900 each. `SETTP` executes 5,803,500, `NUMSCI` 4,395,901 and initialized
  checks 4,392,800. Transitions are 37,924,422 sequential, 4,395,900 calls and
  4,035,903 branches.
- Frames and values: 46 fresh / 4,395,855 reused frames, 38,251,114 entry work
  units; 4,608,802 copies / 76,811 bytes; 80,800 moves / 74,400 bytes;
  282,452 reset-family ops / 224,818 bytes. Requests total 8,867,250 /
  26,375,072 bytes, max 10,992.
- Native ownership: `run` 876, `memmove` 391, `copy_value` 143,
  `syncNumericContext` 60 and trim 47 samples. No targeted heap run was
  selected after portfolio coverage.
- Site/cache: zero selector attempts. Capability/equivalence owns publication;
  compiler/inliner and frame/call setup own the measured implementation shape.

## Richards

- Artifact and timing: 1,897 instructions, 79,606-byte image
  (`16660150fcf3fc461a4e1bc0e3920e67899f1edb5819c67e1363789850ee8157`),
  56,619 instructions / 937,679 bytes with the library; 9,119,155 dynamic
  instructions. Product medians are 11.514098 / 11.682078 s; RSS is
  20,004,864 / 20,234,240 bytes.
- Dynamic shape: `runtask` 65,790, `queuepacket` 23,246 and `appendpacket`
  20,114 calls. Branch leads at 1,183,583; attribute link 924,267 and
  `COPY_REG_REG` 680,964. Transitions are 7,660,294 sequential, 1,220,546
  branches and 119,157 call/return pairs.
- Frames and values: 9 fresh / 119,149 reused frames and 5,585,055 entry work
  units; 96,052,048 copies / 762,807,737 bytes; 99,035 moves; 867,567
  reset-family ops / 4,024,286 bytes. Requests total 739,698 / 173,984,104
  bytes, max 19,440.
- Timed/native ownership: `COPY_REG_REG` alone costs 838.4 / 853.3 ms in the
  bounded profile; `runtask` accounts for 778.5 / 790.2 ms and `queuepacket`
  211.2 / 199.5 ms. Two rxvm samples put `copy_value` at 1,135 / 1,164 and
  trim at 343 / 318 samples; rxbvm confirms 1,127 / 321. Frame entry is only
  6.34 / 6.40 ms in that profile. The heap diagnostic has 9.2 MB high water,
  35.6 MB cumulative allocation and 47,497 allocations / 42,788 frees, with
  copy-value stacks visible but not high-water dominant.
- Site/cache: zero selector attempts. Selected owner is value-copy first, with
  compiler/inliner and call/return mechanics secondary. At only 0.153220 /
  0.150768 of ooRexx, Richards dominates the qualified remaining gap.

## JSON

- Comparability: cREXX measures path/count calls, while the external cells
  build supplied DOM/collection forms. It is a capability diagnostic only.
- Artifact and timing: the 46-instruction, 4,001-byte main image
  (`18b858eb7be8a14ef1905deb83e266354b4d3849d939e3b5238a7046b89a89b6`)
  relies on `rxjson` in the shared library; module-set size is 862,074 bytes and
  54,768 instructions. Product medians are 0.257587 / 0.311959 s; RSS is
  17,858,560 / 17,924,096 bytes.
- Dynamic shape: 118,610,025 instructions; `_json_parse_value` 655,000 and
  `_json_parse_object` 80,000 calls. Branch executes 29,115,002, integer
  equality 20,485,000 and byte load 9,020,000. Transitions are 89,505,021
  sequential, 27,615,003 branch and 745,000 call/return pairs.
- Frames and values: 12 fresh / 744,989 reused frames, 74,730,012 entry work
  units; 7,990,002 copies / 3,528,430,012 bytes; 235,000 moves / 18,090,000
  bytes; 553,499 reset-family ops / 36,691,050 bytes. Requests total 1,531,491 /
  31,814,416 bytes, max 28,416.
- Native ownership: `run` 1,108, `memmove` 221, `copy_value` 115 and reference
  lifetime release 28 samples. No targeted heap run was justified.
- Site/cache: zero selector attempts. Capability/API equivalence owns
  publication; measured ownership is compiler/inliner plus value-copy and
  byte/string processing in the shared `rxjson` module.

## Base64

- Artifact and timing: 622 instructions, 38,241-byte image
  (`84a4285d13df647b9124093597b3258ae2307e0cc222f8a7d7a0669fb5448008`),
  55,344 instructions / 896,314 bytes with the library; 46,725,872 dynamic
  instructions. Product medians remain noisy after the governed append at
  1.642153 / 1.542238 s; RSS is 17,563,648 / 17,694,720 bytes.
- Dynamic shape: decoder 500 calls. Branch executes 6,676,007, loads 3,427,515,
  string copies 3,078,503 and integer increments 2,736,524. Transitions are
  37,137,811 sequential, 9,587,060 branch and 500 call/return pairs.
- Frames and values: 2 fresh / 499 reused frames and 21,546 entry work units;
  2,223,010 copies / 18,801,152 bytes; 500 moves / 512,000 bytes; 1,388
  reset-family ops / 1,075,100 bytes; 4 conversions. Requests total 1,701 /
  611,648 bytes, max 13,368.
- Timed/native ownership: decoder costs 1.452 / 1.472 s in the bounded profile.
  `SETSTRPOS_REG_REG` costs 207.0 / 180.5 ms, branch about 85.7 ms and
  `SCOPY_REG_REG` 52.8 / 51.5 ms. Native samples are chiefly unsymbolized
  `run` (1,376 / 1,387), with `memchr`, `memmove` and `fastParse64` secondary.
- Site/cache: zero selector attempts. Selected owner is RXAS/BIF and VM
  string/binary value work. This is the closest qualified deficit at 0.725045 /
  0.774421 of ooRexx, but it still needs 1.38x / 1.29x for parity.

## RexxCPS

- Comparability: cREXX RexxCPS 2.2d is a disclosed Level B adaptation and is
  compared separately to canonical ooRexx Classic RexxCPS 2.2. It is never a
  common-geomean member. Same-session medians are 29.350347 / 27.132650 MCPS
  versus ooRexx 40.064945, Regina 33.270516 and disclosed NetRexx 2.2n
  49.338709 MCPS.
- Artifact: 1,402 instructions, 77,438-byte image
  (`9b535403baddc5b7076d5eb23027a296d982660546bcecc74d3784bb2fa23741`),
  56,124 instructions / 935,511 bytes with the library. Counts-mode dynamic
  instructions are 25,785,119 / 25,264,347 because the benchmark self-calibrates
  from elapsed time; RSS is 19,038,208 / 19,087,360 bytes.
- Dynamic shape: `upper` 274,400 / 268,800 and `cps_subroutine` 68,600 / 67,200
  calls. Branch leads at 3,134,887 / 3,071,791, string copy 2,545,382 /
  2,493,685. Calls total 348,043 / 341,043; frames are 23 fresh and 348,021 /
  341,021 reused. Copies total 716,078 / 702,178 and 25,372,023 / 24,861,223
  bytes; conversions 1,569,511 / 1,537,511; allocation requests 1,304,869 /
  1,278,469 and 136,428,568 / 133,655,768 bytes.
- Timed/native ownership: in the bounded profile `upper` costs 83.9 / 79.0 ms,
  `cps_subroutine` 57.8 / 59.2 ms, `CALL_REG_FUNC_REG` 47.5 / 43.8 ms, integer
  to string 26.4 / 25.8 ms, frame entry 13.7 / 13.3 ms and aggregate conversion
  hooks 47.9 / 46.6 ms. Native samples show `run` 583, `memmove` 261,
  `fastParse64` 66, `decNumberFromString` 53, `decToString` 35 and
  `rx_string_to_double` 22; formatted benchmark output also contributes 56
  `vfprintf` samples.
- Controls: the exact-hash cREXX attribution-only family isolates BIFs,
  internal calls/arguments, TRACE/ADDRESS, stems, decimal/string loops and
  PARSE. These controls demonstrate coverage and provide PoC guards; they are
  not subtractive ceilings and never replace the 2.2d score.
- Site/cache: zero selector attempts. Selected owner is BIF/inliner and
  value-representation/conversion work, followed by call/frame mechanics.

## Instrumentation boundaries

- All 22 optimized and 22 no-opt profiles report schema 5 and complete domain
  status with no degradation or overflow. Counts-only timing fields are zero by
  contract; selected bounded timing profiles provide time attribution.
- Current optimized workloads execute no `srcmethodsel` or `srcfprocsel`
  selector sites, so stable selector type/target/cache hit data is correctly
  `not exercised`, not fabricated as a successful cache.
- The current value model has no general representation cache. Conversion and
  materialization are represented by explicit operations, value-shape bytes,
  buffer requests and native helpers.
- Helper-level copy/reset timing is not clocked directly; explicit opcode
  timing plus value counts/bytes and native samples are used together. This
  prevents counts/profile elapsed time from becoming product evidence.
- RXSEQ N=2/3/4 windows stop at calls and taken branches and can overlap within
  a straight-line region. They are local sequence evidence, not whole-loop
  truth.
- Apple `sample` supplies host-specific statistical stacks only. This host has
  no `xctrace` installation or available hardware event counter path, so cycles,
  native instructions, branch misses, i-cache and iTLB events are unavailable.

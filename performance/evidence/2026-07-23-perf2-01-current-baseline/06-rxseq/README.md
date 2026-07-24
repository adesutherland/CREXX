# RXSEQ capture index

Exact RXSEQ N=2/3/4 binaries, decoded rows, module sets and checksums are stored
beside each optimized/no-opt profile entry under:

- `../05-profiles/rxvm-counts/entries/`
- `../05-profiles/rxbvm-counts/entries/`

There are 22 entries per VM: optimized and no-opt for every one of the eleven
workloads. The input manifest pins every image and the shared library hash;
each entry manifest records the commands and output hashes. Both per-VM bundles
are checksum-closed.

Interpretation boundary: RXSEQ windows terminate at calls and taken branches.
Straight-line windows can overlap, so sequence counts must not be described as
whole-loop or whole-procedure truth.

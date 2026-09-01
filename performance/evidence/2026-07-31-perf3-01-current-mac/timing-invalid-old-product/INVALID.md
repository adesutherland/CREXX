# Invalid timing block retained for audit

Disposition: **invalid for PERF3 current-product claims**

The final path audit found that the first generated
`perf3-01-current-mac-v1.txt` still contained the PERF2 temporary build and
NetRexx class-directory paths. The `348/348` initial and `40/40` append
executions therefore measured the checksum-bound retained PERF2 product, not
the clean current `3f43a0014` Release product.

No sample from this block is used in the PERF3-01 scorecard, gap ledger or
ranking. The complete raw block and its original driver logs/provenance are
retained rather than deleted. The corrected manifest has SHA-256
`8d0c45c1fa1bbce938e9a1ce88b32594ac727b8cbe857e82e24c161dcd911462`
and points all cREXX cells at `/private/tmp/crexx-perf3-01.jetu7C/build-release`
and all NetRexx cells at the verified retained generated-class directory.

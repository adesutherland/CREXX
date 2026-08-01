# PERF3-02 copy/ownership panel

This bundle contains the completed evidence/design and isolated-PoC work for
PERF3-02. The first timing campaigns remain preserved but provisional because
an active remote terminal may have materially affected the Mac. The governed
clean-host rerun is authoritative: C1a-R2 improves Richards paired elapsed
9.18%/9.33% and C1c-R1 improves Towers 19.42%/19.65% on
`rxvm`/`rxbvm` respectively.

The current-product C0 attribution covers 33 Richards generic sites and
416,260 executions plus 38 Towers sites and 982,781 executions. Generic sites
explain 99.18%/99.21% of Richards total copy operations/bytes and
99.39%/99.36% of Towers. C1a-R2 and C1c-R1 are separately correct, every
authoritative target pair is favorable and every target mean 95% interval is
wholly below zero. Their byte-identical guards remain below the +3% regression
boundary at the 36-pair cap. C1a-R1, C2 and the immaterial C3 residual remain
as negative/deferred evidence.

See [`decision-summary.md`](decision-summary.md) for the decision package,
[`provenance.md`](provenance.md) for build/capture identity, and `summary/` for
machine-readable tables. `rerun/` contains all 376 recorded clean-host samples,
36 warmups, host gates, manifests, exact input hashes, final absolute summary
and paired decision table. `checksums.sha256` closes the final bundle.

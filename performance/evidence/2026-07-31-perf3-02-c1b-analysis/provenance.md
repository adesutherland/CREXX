# Provenance

Analysis date: 2026-07-31

Repository: `/Users/adrian/CLionProjects/CREXX`

Branch/HEAD: `develop` at
`e38e514bf611ae3873513368c44742e2ae7332d1`; product-code parent
`3f43a0014be10c930a12b8a636297b60f294c0a6`.

## Immutable PERF3-02 authority

The checksum-closed
`performance/evidence/2026-07-31-perf3-02-copy-ownership-panel/` directory is
referenced and was not amended. Its authoritative inputs include:

| Input | SHA-256 |
| --- | --- |
| C0 optimized Richards RXAS | `ec61aa8cb044312675502c0ce2a46d4f131e0640bae6188d5db007bfb8731673` |
| invalid C1a-R1 optimized Richards RXAS | `f9a478b2674dbd4a402039f735e362056e058374b65a3e243e278baa50d7b92b` |
| safe C1a-R2 optimized Richards RXAS | `ea1d884d979bf7867efae1cd60102cf7a4db88bc901ae989571d65e19d9274b0` |
| C1a-R1 source diff | `8a05b1b4d7399b427519271ea6e7e44ce5a9537262d23ea18cd2f71b87111958` |
| C1a-R1 correctness stdout | `02b9477253035a40c8df24d4aa08b63be1370a264d7071ee6790580745d86356` |

The C0/C1a-R1/C1a-R2 RXAS paths and hashes are also retained in the closed
panel's `artifacts.csv`. Counts in `site-ledger.csv` come from its
`summary/copy-site-payload.csv`; the `0/1` output comes from its
`summary/correctness.csv` and retained stdout.

## Current source anchors

| Source | SHA-256 | Relevant lines/fact |
| --- | --- | --- |
| `tests/benchmarks/awfy_richards.crexx` | `cd136e6cc8bc5dd487db99abe706fc96d161b3e4b102e13b20cce69bc4692372` | class shapes 37-64; callees 160-172; call sites 218 and 300 |
| `compiler/rxcp_inline_analysis.c` | `4b12590aa19a765f7d3b68c4fbce9d7c4db816840a531df6a40b508b75e02b73` | summary result/control/call facts 239-272 |
| `compiler/rxcp_inline_bind.c` | `e4a61bd7d1dd7e8c660567c73852da9b7b5a9e0cac0f858740d16e13a1f33cc7` | current multi-return receiver block 1700-1748 |
| `compiler/rxcp_inline_rewrite.c` | `84c5addb6664b5d5cb2bc919528a57075865d4451bd8e8d1be25783c2b29d18a` | RETURN to LEAVE_WITH rewrite 934-997 |
| `compiler/rxcp_emit_flow.c` | `b5fcb16c09f34adccbd1e996a1bdcd9eaa295aee074f6d030c7fcd3acb824afc` | IF cleanup after convergence/false label 585-620 |

Additional read-only anchors used in the proof:

- `compiler/rxcpemit.c:1417-1603`: assignment emits right-hand-side cleanup
  before completing the statement;
- `interpreter/rxvmintp.c:13631-13665`: `unlinkn` restores registers to
  frame-owned storage;
- `interpreter/rxvmintp.c:13933-13950`: `linksetattrslinkadd` redirects the
  outer and nested register pointers;
- `interpreter/rxvmintp.c:4266-4276`: child frames inherit signal policy;
- `interpreter/rxvmintp.c:4591-4645`: signal branch unwind discards frames up
  to the handler frame; and
- `docs/ai-context/CREXX_ARCHITECTURE.md`: inlining is AST surgery and nested
  multi-return receiver sharing remains deliberately fail-closed without an
  exit/link proof.

## Scope statement

No product, benchmark or compiler source was modified. No build, execution,
timing, isolated PoC, commit or push was performed. The five pre-existing
untracked lifecycle RXBINs were hash-checked at closeout and left untouched.

# PERF3-12C dynamic PARSE planning PoC

Status: comparative PoC complete; no production implementation selected

Date: 2026-08-08

Source: `06fe132872b1ef51409cb071cb907da9f3714632`, the fetched
`origin/develop` tip at the start of the activity

Branch: `codex/peter-parse-planning-poc`

Build: ordinary profiling-off Release for timings; otherwise matching Release
with `CREXX_VM_PROFILING=ON` for schema-5 counts

Host: Apple ARM64, macOS 26.5.2, 10 logical CPUs, 24 GiB, AC power, Low Power
Mode off. See `raw/environment.txt`.

## Result

The canonical optimized source is not rebuilding the serialized plan at run
time. `rxc` has already propagated `p0 = "b"` and emits one constant plan load
before every `parseExec` call:

```rxas
load r73,"3,1:1;1,2:p1;2,1:b;1,2:p5;"
settpcall r76,rxfnsb.parseexec(),r71,r75,256
```

Therefore ordinary loop-invariant code motion cannot recover the regression.
The hand-hoisted H1 control moves the plan, diagnostic template, argument count,
debug value, type setup and call-frame constants outside the 14-iteration loop,
but retains `parseExec`. Its paired median gain is only 1.33% on `rxtvm` and
2.17% on `rxbvm`.

The H2 control makes the value already proved by `rxc` available to PARSE
planning early enough to select the existing `parseplan` instruction. It is a
diagnostic source transformation of the four `(p0)` occurrences to literal
`"b"`; the canonical benchmark file is unchanged. H2 improves the paired
median by 314.46% on `rxtvm` and 298.03% on `rxbvm`, or 4.14x and 3.98x the S0
rate. This identifies late re-planning, not generic RXAS hoisting, as the first
candidate for the canonical case.

## Correctness qualification

Peter's incoming commits are:

- `ea3b11ef372e135a26c5c4f273e6fda6795974cb`
- `06fe132872b1ef51409cb071cb907da9f3714632`

The unprefixed dynamic-delimiter form used by RexxCPS is correct in the tested
surface. A separate harness changes delimiters and covers empty, multibyte,
repeated, absent and source-alias cases. Optimized and no-opt images pass under
both concrete backends. The focused repository matrix also passes 17/17.

The commits do **not** implement the exact open issue 667 example. Compiling

```rexx
parse data x1 =(start) x2 +(span) x3
```

still fails at `=` with `Invalid PARSE operator usage in template`. The compiler
exit accepts `+`/`-` only when followed by an integer literal and has no dynamic
absolute/relative-position plan item. Issue 667 therefore remains open and
must not be described as fixed by these commits.

The pre-change `84d406904` result is also not a valid performance baseline. Its
unsupported lowering turns `p1 (p0) p5` into a three-target `parsewords3` and
assigns the middle field to `p0`, changing program semantics. Its retained
52.77M clauses/s observation is labelled invalid rather than used as a target.

## Ordinary Release timing

Each cell has one discarded warm-up followed by eight serial canonical-default
runs. Order alternates `S0,H1,H2` and `H2,H1,S0`. All 48 measured processes
pass. CPS is the benchmark's canonical score; startup elapsed and RSS are
retained in the individual logs.

| backend | variant | median CPS | range CPS | median vs S0 |
|---|---:|---:|---:|---:|
| `rxtvm` | S0 current `parseExec` | 10,039,724.5 | 9,863,509-10,140,728 | control |
| `rxtvm` | H1 hoisted call frame | 10,173,503.5 | 9,984,479-10,278,702 | +1.33% |
| `rxtvm` | H2 existing `parseplan` | 41,610,142.5 | 40,590,906-41,793,335 | +314.46% |
| `rxbvm` | S0 current `parseExec` | 10,877,602.0 | 10,767,288-11,059,482 | control |
| `rxbvm` | H1 hoisted call frame | 11,113,190.5 | 10,877,647-11,365,587 | +2.17% |
| `rxbvm` | H2 existing `parseplan` | 43,295,793.5 | 42,947,062-43,940,631 | +298.03% |

RSS ranges overlap at approximately 19.1-19.3 MB. H1 is a strengthened ceiling,
not a semantics-complete production LICM transform: it exploits the fact that
target names in these kind-1/kind-2 result-vector plans do not select outputs
and tracing is off in the benchmark.

## Equal-work counts attribution

Schema-5 counts use `--smoke-count 200`, so every S0 and H2 profile executes
the same 20 million declared clauses and exactly 1,120,000 affected PARSE
sites. Counts are identical under `rxtvm` and `rxbvm`.

| metric | S0 | H2 | change |
|---|---:|---:|---:|
| dynamic instructions | 775,479,029 | 53,199,057 | -93.14% |
| `parseExec` calls | 1,120,000 | 0 | -100% |
| `_stream_next_item` calls | 6,720,000 | 0 | -100% |
| `parseplan` executions | 0 | 1,120,000 | replacement |
| frame activations | 9,242,942 | 282,942 | -96.94% |
| attribute-value blocks | 1,120,020 | 22 | effectively removed |
| attribute-pointer allocations | 3,360,055 | 61 | effectively removed |
| value-slot allocations | 8,960,819 | 685 | -99.99% |
| string-copy operations on strings | 46,745,653 | 6,425,656 | -86.25% |
| call-window setup swaps | 1,121,236 | 1,236 | -99.89% |

H1 can remove only the already-constant setup around those calls. The repeated
Level B decoder and helper frames dominate S0. H2 removes that decoder and uses
the reusable result-vector contract already owned by `parseplan`.

## Ownership recommendation

1. Keep Peter's correct unprefixed `(variable)` behavior, but make issue 667's
   dynamic absolute/relative positions a separate correctness follow-up with
   exact optimized/no-opt tests before claiming closure.
2. Prototype late constant plan specialization in `rxc`. `rxc` owns PARSE
   semantics and already knows after optimization that the plan expression is
   a literal. Teaching the generic RXAS optimizer to recognize the public
   symbol `rxfnsb.parseexec`, decode its Level B string protocol and recreate a
   compiler descriptor would cross an avoidable ownership boundary.
3. Retain RXAS LICM for general invariant values, but do not use it as the
   remedy here. Even the stronger H1 ceiling leaves the decoder intact.
4. Truly changing delimiters need immutable prepared topology plus explicit
   runtime delimiter operands. That requires a bounded internal prototype and
   semantic proof for changing values, aliases, TRACE, failures and ordered
   assignments. This evidence does not authorize a public RXAS/RXBIN/ABI
   choice or an unbounded runtime cache.
5. Keep transactional result placement coordinated with the existing
   PERF3-12/PERF3-12C queue; the counts show planning and transaction allocation
   are coupled, but late constant specialization can reuse the current
   `parseplan` contract without waiting for a new general dynamic-plan format.

No production compiler, assembler, VM, library or canonical benchmark source
was changed, so the mandatory first ordinary-Release production verdict has
not started.

## Evidence map

- `raw/timing-samples.csv` and `raw/timing-summary.csv`: paired timing data.
- `profiles/counts-summary.csv`: selected equal-work counts.
- `profiles/*-counts.csv`: complete schema-5 profiles and value censuses.
- `raw/s0-hot-rxas.txt` and `raw/h2-hot-rxas.txt`: generated hot-path slices.
- `raw/hoisted-callframe.patch`: disposable H1 hand transform.
- `raw/static-b-source.patch`: disposable H2 source control.
- `raw/dynamic-delimiter-semantics.log`: four passing backend/mode cells.
- `raw/issue667-opt-rxc.log`: exact unresolved issue-667 failure.
- `raw/invalid-prechange-control.log`: explicitly invalid old lowering.
- `raw/artifact-hashes.txt`, `raw/artifact-sizes.txt`, and
  `raw/environment.txt`: provenance.
- `raw/static-image-summary.csv`: disassembled static instruction census and
  affected-site counts.

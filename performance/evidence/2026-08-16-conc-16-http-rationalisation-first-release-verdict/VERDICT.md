# CONC-16 HTTP rationalisation first Release verdict

## Decision

**ACCEPTED under the approved unattended rule.** The HTTP-only candidate has no
VM execution-path change, and the frozen benchmark images are byte-identical.
The first 12-pair run had no 3% guard hit but reported one small clear-adverse
RexxCPS `rxbvm` interval. The required identical confirmation run did not
reproduce it: every confirmation median was favourable and every interval was
noisy/inconclusive. The candidate therefore proceeds to proportional QA.

No concurrency API, `.serviceref`, `ask()`, socket primitive, compiler, RXAS,
RXBIN, linker, or RXVM source was changed in this slice.

## Ordinary single-thread result

Percentages are paired candidate change. Negative elapsed-time changes and
positive benchmark-rate changes are favourable. Each cell has 12 recorded
pairs after one warm-up; no sample was removed.

| Run | Workload | VM | Mean change | Mean 95% interval | Result | 3% guard |
| --- | --- | --- | ---: | ---: | --- | --- |
| first | Sieve | `rxtvm` | +0.187% elapsed | [-0.282%, +0.657%] | noisy | no |
| first | Sieve | `rxbvm` | +0.071% elapsed | [-0.335%, +0.477%] | noisy | no |
| first | RexxCPS | `rxtvm` | +1.405% rate | [-2.613%, +5.424%] | noisy | no |
| first | RexxCPS | `rxbvm` | -0.734% rate | [-1.398%, -0.070%] | clear adverse | no |
| confirmation | Sieve | `rxtvm` | -0.624% elapsed | [-1.461%, +0.213%] | noisy | no |
| confirmation | Sieve | `rxbvm` | -1.397% elapsed | [-2.956%, +0.162%] | noisy | no |
| confirmation | RexxCPS | `rxtvm` | +0.282% rate | [-0.578%, +1.142%] | noisy | no |
| confirmation | RexxCPS | `rxbvm` | +0.410% rate | [-0.526%, +1.345%] | noisy | no |

## Correctness and artifact checks

- Focused Release client/LLM matrix: 25/25 passed across `rxtvm`, `rxbvm`,
  optimized and unoptimized linked images.
- Focused Debug client/LLM matrix: 25/25 passed across the same cells.
- The candidate `rxtvm` and `rxbvm` sizes are unchanged from the frozen
  control. Their hashes differ because the executable records build metadata;
  their source execution path is unchanged.
- `library.rxbin` is 53,396 bytes smaller after removing the duplicate public
  Level B HTTP client. `rxfnsg.rxbin` is 35,212 bytes larger after adding the
  shared core and migrating LLM.
- Both benchmark images are byte-identical between control and candidate.

## Boundary

This is an ordinary profiling-off Release verdict on one Apple M5 host. It
supports this HTTP rationalisation slice only. Server latency/idle behaviour,
broad regression QA, sanitizers, install/package proof, and cross-platform
coverage belong to later approved slices.

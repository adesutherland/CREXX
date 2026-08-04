# PERF3-03 bounded conversion review

Date: 2026-08-01

Status: evidence/design gate complete and stopped for Adrian's selection. No
production compiler, VM, RXAS/RXBIN, ABI, signal or language-contract edit is
included.

## Result

The explicit conversion-opcode count was not the material runtime owner. Exact
instrumentation found millions of implicit binary64 conversion attempts in
loose string comparison:

- canonical RexxCPS: 6,315,583 comparisons, 12,631,166 operand attempts,
  2,131,166 failed operands and 2,129,732 first-byte-rejectable failures;
- Base64 2500: 3,425,000 comparisons, 6,850,000 operand attempts, 6,325,000
  failed operands and 6,050,000 first-byte-rejectable failures; and
- the CRI-13 JSON comparator: 302,288 comparisons, 604,576 attempts and 92,456
  failures.

Permute 5000, Bounce 4200, Richards 20 and Towers 100 execute zero loose
comparisons in both VMs. They pass under the diagnostic product. Because the
PERF3-05 panel proved that VM text layout alone can move zero-exposure
workloads, Permute, Bounce and Richards were subsequently timed as common
layout guards; Towers remains a diagnostic correctness control. Sieve 5500 is
the original zero-call layout control in the material-workload panel.

The recommended private candidate is C4 prefilter v3. It keeps the current
`strtod()` converter for every numeric or uncertain span, including the active
locale radix and locale whitespace, and rejects only an empty or impossible
leading-byte span. It does not change `STOI`, `STOF`, public operations or
serialized bytecode.

## Correctness and contract

- Exact current versus candidate stdout matches for the logic and conversion
  suites in both `rxvm` and `rxbvm`.
- Sieve, Base64, canonical RexxCPS and the CRI-13 JSON comparator pass in both
  VMs: 12/12 focused integrated cells.
- The standalone oracle compares return code and output bits for 531 cases in
  each of all 288 installed macOS locales: 152,928 comparisons, zero mismatch
  and zero locale setup failure.
- Candidate v3 keeps `rxvm_loose_compare_text()` inlined. It adds one
  out-of-line 192-byte helper on this Apple build. File size moves from 998,840
  to 998,936 bytes for `rxvm` and from 999,016 to 999,144 bytes for `rxbvm`.

## Final C4 v3 timing

Percentage change is oriented so positive is faster. The headline is the
per-round paired median; intervals are two-sided 95% Student-t intervals around
the arithmetic mean.

| Workload | VM | pairs | paired median | favourable | mean interval | Verdict |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Sieve | `rxvm` | 12 | +0.409% | 11/12 | +0.184% to +0.728% | favourable zero-call layout control |
| Sieve | `rxbvm` | 12 | +1.523% | 10/12 | +0.757% to +2.723% | favourable zero-call layout control |
| Base64 | `rxvm` | 34 | +4.470% | 19/34 | -1.007% to +6.515% | noisy/inconclusive at cap |
| Base64 | `rxbvm` | 22 | +9.025% | 18/22 | +3.435% to +9.963% | decisive favourable |
| RexxCPS | `rxvm` | 12 | +2.736% | 12/12 | +1.639% to +3.314% | decisive favourable |
| RexxCPS | `rxbvm` | 34 | -0.254% | 15/34 | -1.458% to +0.735% | noisy/inconclusive at cap |

No v3 comparison demonstrates a 3% workload regression. No sample was removed.
The noise extensions follow 12 initial pairs, ten serial pairs for triggered
absolute cells, then 12 balanced pairs only for intervals still crossing zero.

### Common zero-call layout guards

These workloads execute no loose comparisons, so any movement is attributable
to compiled-product layout rather than the candidate's semantic path.

| Workload | VM | pairs | paired median | favourable | mean interval | Verdict |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Permute | `rxvm` | 24 | +1.066% | 17/24 | +0.134% to +1.625% | favourable layout control |
| Permute | `rxbvm` | 12 | -0.446% | 4/12 | -1.620% to -0.065% | small adverse, inside guard |
| Bounce | `rxvm` | 36 | +1.064% | 24/36 | +0.300% to +2.219% | favourable at cap |
| Bounce | `rxbvm` | 36 | +0.758% | 23/36 | +0.141% to +1.445% | favourable at cap |
| Richards | `rxvm` | 24 | +0.490% | 16/24 | +0.220% to +0.939% | favourable layout control |
| Richards | `rxbvm` | 36 | +0.147% | 19/36 | -0.400% to +0.370% | neutral/inconclusive at cap |

All six medians and mean intervals remain inside the 3% per-workload
regression guard. The initial 156 executions all passed, but the driver exited
after capture because the first manifest incorrectly requested an aggregate
without ooRexx reference rows. A summary-only replay with the corrected
non-aggregate manifest succeeded over the unchanged samples. The governed
append blocks added 120 and 72 passing executions; no initial sample was
rerun, replaced or removed.

## Isolated conversion ceilings

- C1 bounded integer: 5.291 ns/call versus 27.358 ns current (5.17x), with no
  allocation or copy. In the retained corpus it differs on the embedded-NUL
  seed, which current libc parsing accepts as a truncated prefix; broader
  equivalence across every libc classification/locale case is not claimed.
- C2 copied C-locale binary64: neutral versus current for short and 192-byte
  inputs; it changes locale semantics.
- tested libc++ `std::from_chars`: 20.117 ns versus 17.004 ns on short inputs
  and 3,457.100 ns versus 183.988 ns on the 192-byte control. It also differs
  on current compatibility cases.
- JSON packed materialization executes 46,356 `STOF_REG` operations in both
  VMs, but the production projection remains small beside traversal/method
  overhead. It does not justify a JSON-specific opcode or public span API.

## Evidence map

- `controls/`: helper sources, current/C-locale/from-chars contract output,
  all-locale prefilter oracle and raw helper timing rounds;
- `diagnostics/`: disposable patches and exact loose-comparison shape output;
- `correctness/prefilter-v3/`: focused integrated outputs;
- `measurements/`: raw v1/v2/v3 samples, capture manifests, outputs and
  summaries, including every governed append and the common-layout guard;
- `json-current/`: exact current JSON profile and outputs;
- `provenance/`: build logs/options, manifests, hashes and pre/post host state;
- `paired-summary.csv`: paired reductions for every C4 form; and
- `option-disposition.md`: C0-C5 recommendation and stop boundary.

The original disposable build root was `/tmp/crexx-perf3-03.TsP0ej`; manifests
retain those absolute identities. Use `COMMANDS.md` and the retained patches to
recreate rather than assuming the temporary binaries still exist.

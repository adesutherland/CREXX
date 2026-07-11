# Jump Table Report 4: Compiler Profitability Profile

Date: 2026-07-10

## Scope

Measure compiler-lowered jump tables against semantically equivalent ordinary
branches, then measure the optimization in the `tinyexpr` and `rxjson` parser
workloads.

## Method

Measurements used a Release build and threaded `rxvm` on Darwin ARM64.
Reported values are medians of five or seven in-process timing runs. The
checked-in `jump_dispatch_compare.crexx` fixture pairs:

- a C-style SELECT that the compiler may lower;
- a reversed-operand equality ladder that emits equivalent comparisons but is
  intentionally outside the conservative recognizer;
- checksum validation after every pair.

Integer patterns are first hit, last hit, miss, and uniform hits. Text patterns
are first hit, last hit, and miss. Generated RXAS was inspected to confirm the
table side used `jumpi`, `jumpr`, or `jumpn` while the comparison side
remained branches. A temporary two-case threshold build was used only to
measure the unshipped two-case point.

## Microbenchmark Findings

The ratio is table time divided by branch time; below 1.0 favors a table.

| Shape | First | Last | Miss | Uniform |
| --- | ---: | ---: | ---: | ---: |
| Integer, 2 cases | 1.510 | 1.044 | 1.038 | 1.466 |
| Integer, 3 cases | 1.271 | 0.897 | 0.882 | 1.309 |
| Integer, 4 cases | 1.265 | 0.756 | 0.768 | 1.204 |
| Integer, 6 cases | 1.449 | 0.525 | 0.547 | 1.028 |
| Integer, 8 cases | 1.327 | 0.464 | 0.426 | 0.862 |

Integer dispatch has a clear fixed table-lookup cost. A branch remains best
when the first case dominates, and uniform traffic does not cross over until
approximately eight cases on this VM.

| Shape | First | Last | Miss |
| --- | ---: | ---: | ---: |
| Padded string, 2 cases | 0.477 | 0.234 | 0.161 |
| Padded string, 3 cases | 0.496 | 0.166 | 0.136 |
| Padded string, 8 cases | 0.450 | 0.066 | 0.053 |
| Numeric string, 2 cases | 1.022 | 0.464 | 0.410 |
| Numeric string, 3 cases | 1.000 | 0.339 | 0.272 |
| Numeric string, 8 cases | 1.011 | 0.147 | 0.123 |

Padded string tables win from two cases because they trim and compare once
instead of repeating loose comparisons. Numeric tables are neutral on a
first-case hit and substantially better for every later hit or miss.

## Real Workloads

Enabled results use the Release 1 three-case policy. Disabled results were
built with dispatch recognition suppressed, while retaining all other compiler
optimization.

| `tinyexpr` operation | Enabled us | Disabled us | Change |
| --- | ---: | ---: | ---: |
| Lex | 43,485 | 45,610 | 4.7% faster |
| Evaluate | 114,485 | 114,149 | effectively flat |
| Token names | 22,451 | 24,087 | 6.8% faster |

`tinyexpr` contains dense token/class SELECTs and demonstrates a useful
product-level gain without changing its algorithm.

| `rxjson` operation | Enabled us | Disabled us |
| --- | ---: | ---: |
| Validate | 17,658 | 17,757 |
| Deep get | 18,679 | 18,627 |
| Tail get | 18,601 | 18,670 |
| Count | 35,476 | 35,553 |
| Members | 18,403 | 18,438 |

All JSON differences are below 0.6% and should be treated as noise. The current
parser uses sequential structural tests and recursive calls, not profitable
SELECT or equality-ladder shapes. Improving it requires algorithm work or
future CFG/dataflow recognition, not a more aggressive present recognizer.

## Recommendation

- Use a higher conservative threshold for integer tables, initially eight.
- Allow padded and numeric string tables from two cases.
- Keep exact string and binary decisions conservative until their own paired
  measurements are available; the existing three-case threshold is reasonable.
- Do not use `rxjson` results to weaken safety gates: it currently has no
  meaningful eligible dispatch opportunity.

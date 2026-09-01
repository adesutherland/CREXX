# PERF3 closeout portfolio-v2 qualification

Date: 2026-08-17

Status: **pass — source and runtime correctness qualification complete; no
timing claim**

## Result

The six new cREXX sources each pass as optimized and unoptimized images under
all three executable names: **36/36 processes pass**.

| Source | Deterministic result |
| --- | --- |
| Mandelbrot hoisted control | checksum 128 at smoke size 1 |
| AWFY Queens | eight-queens placement found |
| AWFY NBody | one-step reference energy passes |
| JSON parse | fresh indexed document has eight operations |
| JSON query | parse-once indexed document has eight operations |
| Base64-v2 | length 1,368, exact byte round trip and checksum 130,560 |

`rxvm` and `rxbvm` are byte-identical on this AppleClang build. Both names were
executed for the qualification record, but later formal timing must not count
them as independent product results. `rxtvm` is the distinct direct-threaded
control.

NBody's full 250,000-step optimized reference also passes under `rxvm`,
`rxtvm` and `rxbvm`, producing `-0.16908598899094`. Its `rxmath` dependency is
explicit and keeps NBody a native-math control rather than a pure portable
bytecode cell.

Base64-v2 additionally passes under ooRexx 5.1.0 r12973. NetRexx 5.10-GA
translates and compiles the `options nobinary decimal` source with one harmless
unused-loop-variable warning, then passes under the default Temurin 26.0.1 JVM.
All three language sources report the same length and checksum.

## Product and source identity

- ordinary Release cache: `CREXX_VM_PROFILING=OFF`;
- `rxc`: `8e38ed98841d7f91cd58add88ff0ecad788523ac13bf8c189c674b5d1debf246`;
- `rxvm` / `rxbvm`: `5b07e80401086a850601abab3af98c3449ebe3d7e6f05c5a23c976a9d1506a4b`;
- `rxtvm`: `99434332cd142035349b897d7244afd4ae0b1363c0e3430b453f40e9b671226e`.

Exact source hashes are in `source-hashes.txt`. The qualification uses smoke
work to prove semantics and both code-generation modes; it is not an absolute
performance baseline and contributes no aggregate.

# DECIMAL-01 public evidence orientation

Status: **preliminary orientation; not Gate 3 closure and not CREXX evidence**

Reviewed: 2026-08-05

This record answers which public results should influence the candidate panel
and experiment shapes. It does not import any external timing as a cREXX result
and does not authorize an extended-candidate implementation.

## Evidence classification

| Source | Evidence type | Useful content | Boundary |
| --- | --- | --- | --- |
| General Decimal Arithmetic [`decTest`](https://speleotrove.com/decimal/dectest.html) | language- and representation-independent correctness vectors | Authoritative arithmetic, conversion, rounding, status and context cases to map through the CREXX plugin surface | Correctness corpus, not a performance benchmark |
| Cowlishaw [`Telco`](https://speleotrove.com/decimal/telco.html) and [specification](https://speleotrove.com/decimal/telcoSpec.html) | upstream-author billing application/reference workload | Explicit decimal input, per-call calculations and rounding, taxation, output and accumulation with small worked results | The author explicitly says the narrow operation mix is not suitable for benchmarking decimal implementations generally; published materials are all-rights-reserved, so CDB-1 does not copy source or bulk data |
| Cowlishaw [`decNumber` performance appendix](https://speleotrove.com/decimal/dnperf.html) | upstream-author operation measurements | Direct fixed `decQuad`/`decDouble` versus arbitrary `decNumber` operation and conversion shapes | Historical Pentium M, Windows XP, GCC 3.4.4, default `DECDPUN=3`; useful for hypotheses, not current ratios |
| mpdecimal [decimal benchmarks](https://www.bytereef.org/mpdecimal/benchmarks.html) | candidate-author cross-library measurements with described workloads | `libmpdec`, `decNumber`, `decDouble`/`decQuad` and Intel BID comparisons at several precisions; Mandelbrot, Telco, conversion and large-number shapes | Mostly Core 2/Athlon-era results and candidate-authored; versions and source hashes need reconstruction before reproduction |
| mpdecimal [testing account](https://www.bytereef.org/mpdecimal/testing.html) | upstream correctness/toolchain evidence | Official IBM vectors, additional differential tests, sanitizers and broad compiler coverage | Capability and confidence evidence, not CREXX semantic equivalence or performance |
| Anderson et al., [2009 ICCD paper](https://iccd.et.tudelft.nl/2009/proceedings/465Anderson.pdf) | peer-reviewed cross-library study | Banking, Euro conversion, risk, tax and Telco workloads; operation profiles; arbitrary/fixed DPD, Intel BID and GCC comparisons | Historical libraries and hardware; incomplete match to cREXX APIs, default 18 digits and current compilers |
| Boost.Decimal [benchmark documentation](https://www.boost.org/doc/libs/latest/libs/decimal/doc/html/benchmarks.html) | current upstream reproducible harness and published results | Large comparison/basic-arithmetic vectors, parsing/formatting, GCC decimal and Intel BID controls, documented build commands | Upstream-authored; competitor C and Boost C++ routines are acknowledged as close but not identical; current published host is x86-64 rather than Apple ARM64 |
| Intel [Decimal Floating-Point Math Library 2.0 Update 4](https://www.intel.com/content/www/us/en/developer/articles/tool/intel-decimal-floating-point-math-library.html) | current official capability/package evidence | IEEE 754-2019 decimal32/64/128 surface, source/tests/examples and stated Linux/Windows/macOS path | No current comparative performance case on the product page; licence, redistribution and Apple ARM64 buildability remain gates |
| GCC [`_Decimal32`/`64`/`128` documentation](https://gcc.gnu.org/onlinedocs/gcc/Decimal-Float.html) | official compiler capability evidence | Target-dependent decimal types and support limitations | Eligibility/toolchain control only; no portable current-performance claim |

## Preliminary engineering implications

1. Public evidence supports keeping fixed decimal128 in the panel, but not
   assuming it wins. Cowlishaw's operation tables show a substantial fixed
   `decQuad` ceiling on the historical host, while the 2009 study shows that
   fixed-versus-arbitrary results vary with alignment, rounding frequency,
   precision and workload.
2. `libmpdec` remains the most credible first arbitrary-precision challenger.
   Its upstream Mandelbrot results beat the compared `decNumber` cells at the
   shown 9-, 16-/19- and 34-/38-digit classes, but those old candidate-authored
   figures justify a PoC priority rather than a predicted CREXX gain.
3. Intel BID deserves a fixed-format control, especially on x86-64, but its
   historical decimal64 strength does not transfer automatically to cREXX.
   Decimal64 has only 16 digits and cannot cover cREXX's default 18; the 2009
   evidence also reports a material BID cost increase at decimal128.
4. Boost.Decimal supplies a useful modern, reproducible operation and
   conversion harness. Its published cross-library comparisons are not exact
   API-identical experiments, so DECIMAL-01 should reuse the input shapes and
   build ideas rather than quote the ratios as selection evidence.
5. No reviewed public evidence answers Apple M5 behaviour, CREXX adapter cost,
   plugin lifecycle, dynamic context changes, Common/Classic semantics or the
   typed VM workload mix. Those remain local proof obligations.
6. No reviewed source establishes an IEEE, ANSI or General Decimal Arithmetic
   performance suite. DECIMAL-01 therefore uses `decTest` for correctness and
   publishes the independently authored CDB-1 workload definition and raw
   results. Telco informs one original billing shape only; it is not the suite
   or a standards-body score.

## Experiment shapes imported as hypotheses

The panel should deliberately include:

- aligned and unaligned exponent addition;
- operations that do and do not require result rounding;
- 9-, 18- and 34-digit fixed-work cells plus higher arbitrary precision;
- add/subtract-heavy, multiply-heavy and divide/quantize-heavy applications;
- conversions and formatting as separate costs;
- Banking/Euro/Telco-style application mixes where source and licence permit;
- fixed decimal64 only as a Classic-9 or lower-precision control; and
- fixed decimal128 as the relevant fixed candidate for cREXX's 18-digit
  default.

## Gate 3 completion still required

Before opening D5-D8, add exact library versions, publication dates, benchmark
source/archive hashes, raw-data availability, compilers/flags, licences,
maintenance activity, supported-platform proof and material public issue
reports. Classify every source as upstream-authored, peer-reviewed,
independently reproduced or historical-only. Retain negative and conflicting
evidence.

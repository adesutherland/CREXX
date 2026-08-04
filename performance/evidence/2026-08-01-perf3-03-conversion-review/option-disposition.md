# PERF3-03 option disposition

Status: recommendation at the selection stop; no production edit.

| Option | Disposition | Evidence-backed reason |
| --- | --- | --- |
| C0 current copied libc paths | retain as fallback and baseline | It owns the exact current grammar, active-locale, range, rounding, special-value and embedded-NUL behavior. |
| C1 bounded integer | defer | It is 5.17x faster in the isolated median and removes one allocation/copy per call, but differs on embedded NUL and no current common runtime workload has a material `STOI` owner. |
| C2 copied C-locale binary64 | reject for performance | It is effectively neutral at 16.715 ns versus 17.004 ns for short inputs and 184.048 ns versus 183.988 ns for long inputs, while deliberately changing active-locale behavior. |
| C3 bounded binary64 control | reject tested libc++ `from_chars`; defer the broader lane | The available correctly-rounded bounded control is slower for short inputs, catastrophically slower for the 192-byte control, and differs on hex, NaN payload, subnormal/`ERANGE` and embedded-NUL behavior. No compatible C-facing engine was proved in this gate. |
| C4 v1 inline prefilter | reject | Expanding the loose comparator prevented its prior inlining and confirmed a guarded Base64 `rxvm` regression after 22 pairs. |
| C4 v2 out-of-line first-byte prefilter | reject unchanged | It restored caller inlining and showed useful timing, but hard-coded dot/comma recognition is not a complete proof for arbitrary active-locale radix strings. |
| C4 v3 locale-aware out-of-line prefilter | recommend for a production implementation decision | It preserves installed-locale oracle behavior, has no public or serialized surface, passes focused correctness, demonstrates decisive gains in two material VM/workload cells, and has no guarded regression. Two material cells remain honestly inconclusive at cap; added common-layout guards show four favourable cells, one small inside-guard adverse cell and one neutral capped cell. |
| C4 value-owned parse cache | defer as a separate architecture candidate | All eight VM-private flag bits are already assigned. A validity bit needs a value-layout/ABI change or a side table, plus complete mutation invalidation, copy/move, reference and lifecycle proof. |
| C5 existing/private/public ownership | select private existing-call ownership only if C4 v3 is approved | The material owner is `rxvm_loose_compare_text()`. No new opcode, public span API, RXAS/RXBIN form, JSON specialization or execution-image fusion is justified. |

The recommendation is deliberately narrow: add a private locale-aware rejector
only for loose comparison. Keep `STOI`, `STOF`, `rx_string_to_double()` and the
public instruction/ABI surface unchanged. A portable no-inline spelling is
required because the disposable Mac PoC reused the existing GCC/Clang-oriented
label-owner macro; Windows must not silently inline the wrapper.

The final common-layout guard is deliberately retained even though Permute,
Bounce and Richards execute zero loose comparisons. Across both VMs, no paired
median or mean interval reaches the 3% workload regression guard. This rules
out a material common-workload layout penalty on the tested Mac product; it is
not a substitute for the mandatory first Release verdict after any selected
production implementation.

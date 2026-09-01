# Unicode normalization decision evidence

This directory retains the compact profiling-off Release records that explain
the selected production normalization architecture and its rejected
alternatives. The experimental source and one-off harnesses were deliberately
retired when the Unicode surface was integrated; these records remain because
the algorithm and routing decisions depend on their measured results.

- `2026-08-26-generated-nfd-rough.txt` rejects the very large generated UTF-8
  DFA method shape.
- `2026-08-27-prepared-symbol-nfd-recovery.txt` validates RXVM codepoint decode
  with bounded prepared lookup and direct output.
- `2026-08-28-common-normalization-directional.txt` compares the common
  four-form engine and establishes the Quick_Check predicate policy.
- `2026-08-28-normalization-certificate-informal.txt` checks the cold-path cost
  and repeated-hit benefit of VM-carried normalization certificates.

These are directional measurements from the recorded host sessions, not
cross-platform throughput promises. The enduring interpretation is in the
[Unicode algorithm appendix](../../../docs/books/crexx_vm_spec/unicode_algorithms.md)
and the [product surface and roadmap](../../../docs/planning/unicode/PRODUCT-SURFACE-AND-ROADMAP.md).

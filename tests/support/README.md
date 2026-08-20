# Numeric test support

`numeric_test_support.crexx` is a test-only Level B assertion module shared by
the mathematics contract suites. It is not a mathematical oracle, a source of
expected results, or part of the installed standard library.

The `numerictestsupport` namespace provides typed checks for:

- Boolean conditions and exact native-integer equality;
- binary-float absolute-plus-relative comparison and IEEE special values; and
- exact or absolute-plus-relative decimal comparison in the caller's numeric
  context.

Expected results and tolerances belong to the owning contract suite. The
selection, provenance, and required coverage of those cases is governed by
`docs/planning/release-1/mathematics-validation-strategy.md`.

A suite imports this module with `tests/support` supplied to `rxc` through
`-s`, compiles `numeric_test_support.rxbin`, and links or loads that test-only
module beside the suite. `-i` identifies binary roots and is not a substitute
for the source root.

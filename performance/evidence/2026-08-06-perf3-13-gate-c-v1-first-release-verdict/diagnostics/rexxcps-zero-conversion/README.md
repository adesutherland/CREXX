# RexxCPS zero-conversion diagnostic

The initial V1 RexxCPS warmup printed its first three lines and then faulted
before provenance. Direct optimized and unoptimized runs both returned status
139. The retained LLDB register trace stops in `run` while storing `"0"`
through a null `string_value` pointer.

RexxCPS selects `numeric digits 9`, which reaches
`extract_integer_decimal()`. Its zero special case historically relied on the
always-present inline string. V1 now prepares a minimum sidecar on that path;
the focused unit test reproduces nine-digit integer-zero conversion. The
post-fix canonical output completes with provenance, metric and PASS.

The correction is conditional on `!RXVM_VALUE_HAS_INLINE_STRING`, and the
normalized V0 product text remains exactly identical to retained S0.


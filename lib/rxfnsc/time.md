# Level C `TIME` BIF contract

Classic Level C defines:

```text
TIME([option [,time [,inoption]]])
```

Its checklist is `oCEHLMNORS oANY oCHLMNS`. It must freeze the current local
time for a clause, support conversions between the allowed input/output forms,
maintain elapsed/reset state, and report standard argument errors plus `40.19`
for invalid input conversion and `40.29` when conversion targets `E`, `R`, or
`O`.

This is not the Level B one-option VM-clock extension. A standalone direct
RexxValue implementation is intentionally pending the shared DATE/TIME
`Time2Date`, local-time adjustment, frozen-clause-time, and elapsed-state
services. Existing compiler artifacts and lowering are unchanged.

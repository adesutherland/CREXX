# LENGTH empty-initialized representation regression

## Classification

This is a **valid Level B and typed-RXAS sequence**, not an invalid scratch
construction. It is excluded from the LEN-H1 performance comparison because it
is semantically wrong on the current runtime, but the failure itself is retained
as correctness evidence and routed to **PERF2-07 V3: retained representation
validity and mutation invalidation**. PERF2-04 does not install a VM fix.

At exact repository commit
`6567f0ba23f20623e01322f5a62323b2347ab09d`, the source:

1. initializes a Level B string local to `""`;
2. assigns the same local from the valid cast `j as .string`, where `j` is a
   decimal;
3. invokes the public, Unicode-aware `assembler strlen` into a distinct integer
   result; and
4. checks that the final source is `"2.2"` and its codepoint length is `3`.

The exact B0-R compiler accepts that source and emits the typed sequence:

```rxas
load r18,""
...
dcopy r18,r27
dtos r18
strlen r17,r18
```

This agrees with the public instruction contracts: `dcopy` copies the decimal
payload while leaving other payloads and flags intact; `dtos` replaces the
string payload with the formatted decimal; and UTF-build `strlen` returns the
logical codepoint count without changing the source. The expected result is
therefore `source="2.2", length=3`.

## Observed failure and mechanism

The retained original RXBIN was run serially with both exact B0-R VMs and the
exact B0-R linked library using `-a --smoke-count 1`. Both VMs exit 1 with empty
stderr and the same distinguishing result:

```text
FAIL: direct STRLEN result mismatch: 0 source=2.2
```

The source bytes and byte length are current, but the logical codepoint count is
stale. The current VM implementation supplies the mechanism: loading the empty
literal establishes a valid cached codepoint count of zero; `DCOPY` deliberately
preserves other payload flags; `DTOS` replaces `string_value` and
`string_length` but does not clear or refresh the UTF validity/count flags or
`string_chars`; `STRLEN` therefore accepts the stale valid-count state and
returns zero. The matching `rxvm` and `rxbvm` result is expected because they
share this handler logic.

Typed-null initialization was used for LEN-H1 only to keep its machine-ceiling
measurement semantically valid. It is a control, not a claim that initializing
a Level B string to `""` is invalid.

## Exact PERF2-07 regression case

PERF2-07 should add one focused Level B regression that cannot be weakened into
the typed-null control:

1. declare/initialize `source = ""`;
2. create `decimal = "2.2" as .decimal`;
3. execute `source = decimal as .string`;
4. execute `assembler strlen length,source` with a distinct `.int` result;
5. require `source == "2.2"` and `length == 3`; and
6. run optimized and no-opt images on both `rxvm` and `rxbvm`.

The generated-code guard should retain the relevant same-destination
representation crossing (`LOAD string literal`, `DCOPY`, in-place `DTOS`) before
`STRLEN`, so a compiler reshaping cannot silently stop exercising the stale
cache. A typed-null sibling control may remain to show the state-dependent
distinction. Any correction belongs to the PERF2-07 representation-validity
panel and must audit other string-producing in-place conversions before a
production change is selected.

## Artifact identity

- source: `rexxcps_levelb-empty-init.crexx`, SHA-256
  `5e7d3e3ab0f282a97607e1c4dac181fb2ec3a1e066801b9a91f4412da49be65a`
- RXAS: `rexxcps_length_empty_init.rxas`, SHA-256
  `d939e86133ac1bbeb0340a12d3bfa113a96282b895e2c8616d5a1d9fb0a5a133`
- RXBIN: `rexxcps_length_empty_init.rxbin`, SHA-256
  `0f87635a9cb8f1cf036fd0480bfe8d9e98447afb66dc6711396e951821d0fda7`
- both VM stdout files: SHA-256
  `354acb6649786c6bb7de74438422b6c217df637a1e5b2f807b187c4652243fe0`
- both VM stderr files are empty: SHA-256
  `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`

The reconstructed full source compiles with the exact B0-R `rxc` to RXAS that
is byte-identical to the originally retained failing RXAS. The retained RXBIN is
the original failing artifact; RXBIN embeds its assembler output identity, so a
reassembly under a different output path is not expected to be byte-identical.

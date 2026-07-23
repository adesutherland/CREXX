# NR-15 post-hardening drift summary

The closeout compared the accepted first-verdict VMs with the final QA VMs on
the exact same retained D2-hybrid RXBINs. The accepted binaries were copied
before the final Release rebuild.

| Artifact | Accepted SHA-256 | Final SHA-256 |
| --- | --- | --- |
| `rxvm` | `08ef13e2632447825bea7b436ba8984f4679abc726653906683a6ba852ca1386` | `aed8b680ebfa45c4f154703a2c268637e53b26fd95f6caafba8926d4fe9b0c9e` |
| `rxbvm` | `5e3ecbcbfecb8139563d0c043b9978f2e75ba3ce0c8b84edc60abaf11909d819` | `a153dbdd1bce225d6667d5c82b162393268950046c7626fc867a2b94bda0edef` |
| access RXBIN | `9d936bdad369c669766b34469e467d860d35d9b0681879f4b5854bb268cacdc6` | byte-identical |
| RexxCPS RXBIN | `42820ecfca8b1a8cde2fc687abad0a864a5faef8517887cf82fc6dbce5b0e35b` | byte-identical |

The screen used one warmup and seven balanced serial recorded pairs per cell.
Its >10% absolute-span flags were handled by ten unchanged serial appended
pairs for the exact `rxvm` get-hit and lifecycle cells. All correctness checks
passed. Combined paired medians are reported in `README.md`; the raw samples,
outputs, runner manifests, and summaries are retained beside this file.

# E3b-P1 branch-free artifact identity

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| E3a control `rxbvm` | 1,113,112 | `9269ad65cc747a0d4770e0ff79bb2ea87f96769b704585ec616eb899df8cc2ac` |
| branch-free candidate `rxbvm` | 1,132,136 | `eadabe1c96aabcb9f7500d77ea19a0477256962e8f296fe54b74cdc06c5cd125` |
| E3a control `rxtvm` | 1,113,240 | `23a486b17ceaa8685ecb3f3606ddf7748506c7e09c549e2e3ae48d9d130e06fb` |
| branch-free candidate `rxtvm` | 1,132,264 | `f17f91351c0c36ccd119120dc66b3a1a0918353b967ed740671e4c28ecb8bbb2` |
| shared `library.rxbin` | 930,739 | `f27a8ff0dc50a136686e6bfe21dbb4680fdb398737682c5039502a9e4bc3b4f3` |
| Sieve RXBIN | 4,892 | `a9db0e5dc96ab05fd3b78471fedc4bb641555d8b5c209ae33b1a2f80e8dc9d06` |
| canonical RexxCPS RXBIN | 73,137 | `e7e24bdbd42f41d957bc0213d375423491748bee60e6898e2b4c32d1890b7a8f` |
| process-reentrant call RXBIN | 2,477 | `7df91af02c26a317d5ab07e6a18d8684941ff000c7b8acbe7a0d90a0fc470bc4` |
| legacy call RXBIN | 2,461 | `2e556c73ad507839463bee6931678941fe43f348414d77c479cb27b39dce27d7` |
| process-reentrant plugin | 33,520 | `758fafa8927594f4f733a35c9236597fb030de87a7ccafa4d5005a2f1a3265fc` |
| legacy plugin | 16,864 | `16c52f0f35644c6a0511c30cc01b04ec407f461f2067c8d4d389229a967b4950` |

The workload, library and plugin images are byte-identical to the two rejected
E3b-P1 verdicts. Only the candidate VM call-policy implementation changed.

`SOURCE-SHA256SUMS` binds the accepted closeout source and fixtures.
`SHA256SUMS` binds this evidence bundle.

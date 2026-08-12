# E3b-P1 artifact identity

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| E3a control `rxbvm` | 1,113,112 | `9269ad65cc747a0d4770e0ff79bb2ea87f96769b704585ec616eb899df8cc2ac` |
| E3b-P1 candidate `rxbvm` | 1,131,160 | `8c51610106b0ff84101e092727787a77887f72db69474707a07b3ce174e81867` |
| E3a control `rxtvm` | 1,113,240 | `23a486b17ceaa8685ecb3f3606ddf7748506c7e09c549e2e3ae48d9d130e06fb` |
| E3b-P1 candidate `rxtvm` | 1,131,304 | `7a34f5bdd3f9cd2b51d56507ecf31206c9d0538cee766bd2cce26af13d653e92` |
| shared `library.rxbin` | 930,739 | `f27a8ff0dc50a136686e6bfe21dbb4680fdb398737682c5039502a9e4bc3b4f3` |
| Sieve RXBIN | 4,892 | `a9db0e5dc96ab05fd3b78471fedc4bb641555d8b5c209ae33b1a2f80e8dc9d06` |
| canonical RexxCPS RXBIN | 73,137 | `e7e24bdbd42f41d957bc0213d375423491748bee60e6898e2b4c32d1890b7a8f` |
| process-reentrant call RXBIN | 2,477 | `7df91af02c26a317d5ab07e6a18d8684941ff000c7b8acbe7a0d90a0fc470bc4` |
| legacy call RXBIN | 2,461 | `2e556c73ad507839463bee6931678941fe43f348414d77c479cb27b39dce27d7` |
| process-reentrant plugin | 33,520 | `758fafa8927594f4f733a35c9236597fb030de87a7ccafa4d5005a2f1a3265fc` |
| legacy plugin | 16,864 | `16c52f0f35644c6a0511c30cc01b04ec407f461f2067c8d4d389229a967b4950` |

The control executables report embedded version
`crexx-1.0.0-beta.3+local.g6d12cd921cdb.dirty`; they are byte-identical to
the accepted E3a closeout binaries subsequently represented by commit
`29ef1975e`. Candidate executables report
`crexx-1.0.0-beta.3+local.g29ef1975ec01.dirty`.

`SOURCE-SHA256SUMS` binds the frozen dirty source files and benchmark fixtures.
`SHA256SUMS` binds the retained evidence bundle itself.

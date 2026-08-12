# E3a verdict artifact identities

## VM executables

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| control `rxbvm` | 1,112,312 | `e6dc4ce070c6e20dee899d9a8450ef3f63b20d37e9306c86f8aae5547ef1a7f3` |
| candidate `rxbvm` | 1,113,112 | `9269ad65cc747a0d4770e0ff79bb2ea87f96769b704585ec616eb899df8cc2ac` |
| control `rxtvm` | 1,112,440 | `ec76fa71a3780deaa2de599da6f178bd15e5a63cbc2ec632546c9754e4ff1ecb` |
| candidate `rxtvm` | 1,113,240 | `23a486b17ceaa8685ecb3f3606ddf7748506c7e09c549e2e3ae48d9d130e06fb` |

Control executables are under
`/private/tmp/crexx-perf3-13-e3a-control.WdO0Ib/source/build-release/bin`;
candidate executables are under
`/Users/adrian/CLionProjects/CREXX/cmake-build-release/bin`.

## Shared clean timing inputs

| Input | Bytes | SHA-256 |
| --- | ---: | --- |
| `library.rxbin` | 930,755 | `54ce252f8ebdab41fd8b25b0b37fb11c189ca277a610149f8d57131679e0d01c` |
| Sieve | 4,892 | `a9db0e5dc96ab05fd3b78471fedc4bb641555d8b5c209ae33b1a2f80e8dc9d06` |
| Richards | 79,014 | `ea2d6e93788c7574b77a29899a2aac7bd0f2bf5f0d4e5d84dbc2a0934ea74ed7` |
| Towers | 28,550 | `46d9742173ddb6bc2545669529d159ad234b559544d35a9bb900ff025e913d67` |
| RexxCPS | 73,137 | `e7e24bdbd42f41d957bc0213d375423491748bee60e6898e2b4c32d1890b7a8f` |

All shared inputs came from the exact clean control build. The candidate build's
generated library image was deliberately not mixed into this VM-only verdict.

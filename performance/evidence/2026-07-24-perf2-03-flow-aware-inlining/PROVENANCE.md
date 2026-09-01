# Prototype and product provenance

## Isolation

- Main repository: `/Users/adrian/CLionProjects/CREXX`
- Branch and exact source: `develop` at
  `086138f1e93da8e84d45f4cd3ba9b6620f792a14`, equal to `origin/develop`
  when the campaign began.
- Scratch root: `/private/tmp/crexx-perf2-03.9pQYlh`
- Source/build pairs: `p0-source`/`p0-build` through
  `p4-source`/`p4-build`; each source tree was detached at the exact commit.
- Configuration: focused CMake/Ninja Release builds with profiling off and
  tests enabled where needed.
- Main worktree compiler, VM, ABI, RXAS and RXBIN sources were not edited.

## P0 product identities

| Artifact | SHA-256 |
| --- | --- |
| `rxc` | `86e3f568f78c0a790c2a3c29e854de297b721738495bb49a8637f833f68fdfca` |
| `rxas` | `eb0c07e5b9984313353ed6ff9a24247b80b413d6b48f2976d3c8f3387a29fe22` |
| `rxdas` | `9dc52b9a029c70a19005271205d226b7382ab3f14656bc9f9d88cd937f3540d9` |
| `rxvm` | `cfec83c700963157063b1f19ac6e553a78c3fd16475848aa66556cd175737d13` |
| `rxbvm` | `6884c225f58012899e8c12fd4945006320999b14c69cc90af22d83e16bd55765` |
| `library.rxbin` | `fd8a12b39fcffbcbbdf27b2be92178801be0c43d3a16553a4c75a8907a05c599` |

## Prototype identities

| Prototype | Patch SHA-256 | `rxc` SHA-256 | `library.rxbin` SHA-256 |
| --- | --- | --- | --- |
| P1 Q1 replay | `5522ded06f2cca8d564fe0f674e187fed39753a2358b484dadcc5ab72ecd8bd6` | `b552708a9d8438aea8f2e896f39333096663ccf8eaaceb6d3bd2a98488aeaa40` | `9aa3fea82177e850358d670f519f74b297a54bcdb506f6fcfe936edc79a4da72` |
| P2 read-only scalar formal | `049e44c968ad28b79042261f252756ee43a75948ff821238d1d22e7761d835e8` | `33e2ecaf797901b70d50ad0fce56b816d1b42d989454fd928525eab5bf32c74b` | `874e89f5818e0d19a597afd028273c635457294bdf333c027a0a7c2b6ee259d0` |
| P3 inline result placement | `4e25f9dfea70b3d4379d8dd69fc56a93e713bbc75d98ef6c9314872e54b95d93` | `07c695ad794e86dee1c23fccc75a2406292a22b496f97a0570a1cdb557777531` | `62f3999151e7d1213ef844b8d4d3b7776bb4ab7a0ee1b7a5d51aee58b182ea6d` |
| P4 combined static fallback | `067f9f47bbccc571cd5eaaab2fe38d188d845e7c43ef60842abc4f9cac740c48` | `93611e815b63e70282bece5c68322e000a30ed7957f33d120280cb525de7e029` | `ea1e8331d22d8f827a20a0fefccf036d2d048130ae19a0c22970047288a5b4c4` |

P1-P3 are independent patches over P0. P4 deliberately combines P1, P2 and
P3 with a 100-node post-clone static fallback probe. Patch identity therefore
does not imply that any mechanism is selected for production.

One early parallel generated-compiler build in P1 encountered a generated-exit
dependency race; the serial focused retry succeeded. This is recorded as build
setup noise and did not affect the compiler or test results.

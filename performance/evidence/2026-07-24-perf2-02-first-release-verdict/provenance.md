# PERF2-02 first Release verdict provenance

## Source and scope

- Branch: `develop`.
- HEAD and `origin/develop`:
  `d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`.
- Approved production slice: exact Q3b A-LOCAL/A-ATTR classification inside
  canonical `MKREF_REG_REG`; no persistent state or public-format/ABI change.
- Pre-production main `rxvmintp.c` SHA-256:
  `a01188cc95ac13b691406a8f7aebdfc5eeb7674c3c346d67d1e5871e700b3c32`.
- Integrated `rxvmintp.c` SHA-256:
  `987b091fd9e1a34030ebb5c73407656827d0c03c1a6778ef052d39fe5ab85c7d`.
- Integrated full `rxvmintp.c` tracked diff SHA-256:
  `01b1644e68d59ce5da4a78ba6ac9a2b960cdbeb6445ab44e897203b9d3a86fa5`;
  this includes the preserved pre-existing schema-5 work.
- Focused guard source SHA-256:
  `e3f7b0132199a9ef56e73a5be58e6fcac7ea4da359e4abb079257e5a6ebcce22`.

## Product artifacts

Build: CMake `Release`, `CREXX_VM_PROFILING=OFF`, `BUILD_TESTING=ON`,
Ninja, Apple clang 21.0.0.

| Artifact | SHA-256 | File bytes | Mach-O `__text` |
| --- | --- | ---: | ---: |
| accepted Q0 `rxvm` | `2bb27f8a31298e20a67b6a986040580f83e5bcd7261b1795333e3d36762ca871` | 982,264 | 806,300 |
| production `rxvm` | `717c3be993fcdcd7c5082baed0ff532c201d9b010f3ca049f982a14da28ec8d2` | 982,264 | 806,716 |
| accepted Q0 `rxbvm` | `a738271972e52ea8a0cf0981eb86d58179c6f715824595eb8ca576737e360e15` | 982,392 | 799,132 |
| production `rxbvm` | `e443520e367f676773ff1e7f7d7e891eb3120af4eb70d7829c23a9faa7c23f07` | 982,440 | 803,096 |

Accepted optimized Bounce image SHA-256:
`b1cc4416c538f3bf4cf9b73f85735712d88ead7091f931aa20e77b4216defb2b`.
Accepted library SHA-256:
`a9b660f6a67fd57fa35ae180a6b3c0f2764d44241fc0272bd6c6b843ce5d8e10`.

The production text sizes exactly match the selected Q3b PoC. File deltas are
0 bytes for `rxvm` and +48 bytes for `rxbvm`; text deltas are +416 and
+3,964 bytes, below the 5%/4 KiB artifact escalation gate.

## Focused correctness

- Debug focused CTest: 10/10 PASS, covering the new dual-VM guard fixture,
  existing reference/catch behavior, signal unwind, dynamic load and re-entry.
- Ordinary Release production binaries: the new guard fixture passes in both
  VM modes.

## Host before formal capture

- UTC: `2026-07-24T07:29:06Z`; local: `2026-07-24T08:29:06+0100`.
- Mac17,3, Apple M5 arm64, 10 logical CPUs, 24 GiB RAM.
- macOS 26.5.2 build 25F84; Darwin 25.5.0.
- AC attached, battery 77%, low-power mode 0.
- No recorded thermal, performance or CPU-power warning.
- Load averages: `2.79 2.23 1.72`.
- CMake 4.3.2, Ninja 1.13.2, Apple clang 21.0.0.
- No overlapping build, test or benchmark process.

## Capture completion and noise append

- Initial formal capture: `2026-07-24T07:30:31Z` through
  `2026-07-24T07:34:12Z`; host remained on AC, low-power mode 0, with no
  overlapping build, test or benchmark process.
- Required `rxbvm` append: `2026-07-24T07:36:46Z` through
  `2026-07-24T07:38:12Z`; AC attached, battery 80%, low-power mode 0, load
  averages 2.11/2.49/2.09 before and 2.03/2.36/2.08 after.
- No sample was removed or replaced. The initial 12 pairs and appended ten
  `rxbvm` pairs are both included in the governed summaries.

## Retained result identity

- Initial samples SHA-256:
  `1724f98931f9bb6f82fd3af02725e09d45c71e46b04ada7c5dd1de29c5a886f8`.
- `rxbvm` append samples SHA-256:
  `18d727d865343eef635902e6a1d3fa0d7a409a0b46756a1d2cd01f78baaca5cc`.
- Combined absolute summary SHA-256:
  `2fbc0332d590780f533b1d56068ef161b2e0f1690ed38283c90c9ab553fe77d5`.
- Paired summary SHA-256:
  `90e5074b5bcb663792a7448b8a0d818b87371d4b4f698672757acf36adada7e8`.
- Evidence-local Level B reducer SHA-256:
  `ce402d6c8dbfb690d2e87efc08f8aa137b4f42272ae3c4023db63c17ad4c0b5b`.

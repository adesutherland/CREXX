# Performance closeout Stage 1 candidate freeze

Date: 2026-08-18

Status: **PASS - combined production candidate frozen; Stage 2 measurement
opened**

## Candidate

- branch: `develop`;
- production source commit: `0fe5618eb858758d5f01da9dc65331d677605565`;
- commit subject: `Merge remote-tracking branch 'origin/develop' into develop`;
- remote relation at freeze: eight local commits ahead of `origin/develop`;
- production source build: clean detached worktree at the exact commit;
- main-worktree dirty scope after freeze: closeout/DECIMAL documentation and
  manifests only; and
- publication: none.

The merge preserves the seven completed local POSTPERF commits and incorporates
the eleven fetched remote commits without rebasing either history.

## Programme status

- POSTPERF-01 through POSTPERF-05 are complete.
- POSTPERF-05 has no successor production stage.
- Windows concurrency QA-D is complete across MSVC, Clang and GCC.
- Mac concurrency QA-B requires only the retained quiet-AC performance replay.
- Linux QA-C remains ready but formally unrun.
- DECIMAL-01 Gate 1 and the bounded D2/D4/D3 materiality screen are the only
  performance work authorized by the 2026-08-18 closeout direction.

## Exact Release build and qualification

The ordinary Release product was configured with:

- `CMAKE_BUILD_TYPE=Release`;
- `CREXX_VM_PROFILING=OFF`;
- `CREXX_VM_HANDLER_PANEL=profile-20`;
- Apple Clang; and
- product `rxvm -> rxbvm`.

The clean product reports
`crexx-1.0.0-beta.3+local.g0fe5618eb858`. The focused DECIMAL-01 selection
passes 81/81 tests, including both concrete VMs, all provider/context modes,
all three optimizer boundaries, L1 adapter payloads, RexxCPS family controls
and final-RXBIN optimizer-integrity checks. Test elapsed values are correctness
only and are not performance evidence.

Clean-product identities:

```text
62e110d29047dc5b028f956e0b21054828e05671c128c7717cb9663bace42b19  rxbvm
6e16f551402f50b77cdfd85974fb05952d196cb71f79f2f4c896847783c75f74  rxtvm
cc6cf15d9a6af015623dd2cb462b7714ec99016d8d71b322cb84cad5fc942ccb  library.rxbin
```

Configure, build and focused-test logs are external scratch outputs until the
Stage 2 bundle is consolidated. They are not intended for long-term retention.

## Opening host audit

Before calibration the Apple M5 host was on AC power at 80%, low-power mode was
off, no thermal/performance warning was recorded, the CPU was 83.68% idle and
no benchmark, build, compiler, VM or test process competed for the host.
Calibration output is not formal evidence. The host is re-audited before every
formal block.

## Boundary

Production implementation is frozen. Candidate library source/builds remain
external scratch work. No production decimal provider, ABI, RXAS/RXBIN,
language or VM change is authorized.

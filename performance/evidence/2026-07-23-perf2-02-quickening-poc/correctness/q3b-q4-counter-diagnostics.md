# PERF2-02 Q3b/Q4 build-private counter evidence

Status: complete, non-authoritative diagnostic run only. No formal timing was
performed. The main worktree was not edited.

## Scope and counter contract

- Source baseline: detached `d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`.
- Original Q3b PoC patch: `e673e61e34c248a5e9002974edc4df8599bee03a3e0dcc5f558b5c083a552150`.
- Original Q4 PoC patch: `5f9d81ce35c05dec104c8464f0e823e0caf4ba1ddf7a9d1d3b060fb76a84b164`.
- The build-private counters retain one fixed-capacity process-local record per
  executed `(module, canonical word, form)` and aggregate executions, exact
  fast-path hits, and guard misses/canonical fallbacks. All runs report
  `site_overflow=0`.
- Q3b and Q4 are zero-learned-state forms. Therefore invalidation is not a
  state transition they can experience: every run reports exactly
  `invalidations=0` and `invalidations_status=N/A_zero_state`.
- These counters add diagnostic-only BSS/process state and were never used as
  product timing inputs.

## Exact aggregate counts

| Form | VM | Workload | Site executions | Exact hits | Guard misses / fallbacks | Invalidations |
| --- | --- | --- | ---: | ---: | ---: | --- |
| Q3b | rxvm | Bounce, 1 repetition | 5,100 | 5,100 | 0 | 0 / N/A |
| Q3b | rxbvm | Bounce, 1 repetition | 5,100 | 5,100 | 0 | 0 / N/A |
| Q4 | rxvm | Bounce, 1 repetition | 5,100 | 5,100 | 0 | 0 / N/A |
| Q4 | rxbvm | Bounce, 1 repetition | 5,100 | 5,100 | 0 | 0 / N/A |
| Q3b | rxvm | `perf2_qref_guard` | 3 | 1 | 2 | 0 / N/A |
| Q3b | rxbvm | `perf2_qref_guard` | 3 | 1 | 2 | 0 / N/A |
| Q4 | rxvm | `perf2_qref_guard` | 3 | 1 | 2 | 0 / N/A |
| Q4 | rxbvm | `perf2_qref_guard` | 3 | 1 | 2 | 0 / N/A |

All eight commands exited `0`. Bounce reported result `1331`; every cell
printed its expected PASS marker. Counts are exactly identical between rxvm and
rxbvm and between Q3b and Q4.

## Exact per-site counts

Canonical short Bounce, for every form/VM cell:

| Module / word | Form | Executions | Exact hits | Fallbacks |
| --- | --- | ---: | ---: | ---: |
| 1 / 791 | local form | 100 | 100 | 0 |
| 1 / 844 | adjacent `MINLINKATTR1` attribute pair | 5,000 | 5,000 | 0 |

`perf2_qref_guard`, for every form/VM cell:

| Module / word | Form | Executions | Exact hits | Fallbacks | Meaning |
| --- | --- | ---: | ---: | ---: | --- |
| 1 / 14 | local-form candidate | 1 | 0 | 1 | owned physical child; no adjacent pair because `LOAD` intervenes |
| 1 / 46 | adjacent attribute pair | 1 | 0 | 1 | `LINKTOATTR` external child; mandatory canonical fallback |
| 1 / 91 | local form | 1 | 1 | 0 | direct current-frame local |

The owned-physical-child fixture is a semantic fallback/lifetime test, not an
exact fast-path hit. Canonical Bounce word 844 is the bounded proof of the
owned physical attribute fast path.

## Inputs

- Accepted optimized Bounce image SHA-256:
  `b1cc4416c538f3bf4cf9b73f85735712d88ead7091f931aa20e77b4216defb2b`.
- Accepted library SHA-256:
  `a9b660f6a67fd57fa35ae180a6b3c0f2764d44241fc0272bd6c6b843ce5d8e10`.
- Shared guard image SHA-256:
  `8437785801af662a647b612c23813f9d2aa0cba266060c622b2d4ea9d73f73c0`.

## Diagnostic source and build hashes

| Artifact | SHA-256 |
| --- | --- |
| Q3b instrumented `rxvmintp.c` | `6dff8513caed1993651ea69563b48486f6f9d61cc6da89078ed073b57b89161c` |
| Q3b diagnostic rxvm | `024f51e658252aac912c719b488a11770adf87f515bd5e576f7e6fe357c322b1` |
| Q3b diagnostic rxbvm | `749705c00f7f0e01c136260be42d468a29873286f3f1c0590ed738821cb30d3d` |
| Q4 instrumented `rxvmintp.c` | `7ae5341014628b26fe0e09b3932cac99ffc66b463797b4d1c0a76ec9cb778c6d` |
| Q4 diagnostic rxvm | `3871b78b45b60e3a5e4d40d35824665f8e54475c5bc0c0c84e9f59f22abab8cd` |
| Q4 diagnostic rxbvm | `71047d5b69a65f0f9d694050c2b72c731e7e6f34b901f0875b77344511895b81` |

The `q3b-diagnostic-counters.patch` and `q4-diagnostic-counters.patch` files are
incremental relative to the original Q3b/Q4 PoCs and pass `git apply --check`
in those restored trees. They are retained under `prototypes/`.

## Restoration proof

After capture, the diagnostic edits were reversed and both ordinary Release
targets rebuilt. Exact original prototype state was restored:

| Artifact | Restored SHA-256 |
| --- | --- |
| Q3b source patch | `e673e61e34c248a5e9002974edc4df8599bee03a3e0dcc5f558b5c083a552150` |
| Q3b rxvm | `382f2262e638c61135269d65551a60fe2867253d188e6478442dea260b6f4b0f` |
| Q3b rxbvm | `2a8ed23cd9e857b9e0bb85fe8dae0a9f312295226b4a9b412afc30c63b4a6858` |
| Q4 source patch | `5f9d81ce35c05dec104c8464f0e823e0caf4ba1ddf7a9d1d3b060fb76a84b164` |
| Q4 rxvm | `c81271321c7a94b4b32b2e433015d84f3747bf6c0c8087e20949f39ea364e0ed` |
| Q4 rxbvm | `f15817dfaa5746c8997b5877a1f21576bfa831a838a493d5afc017b158c60c94` |

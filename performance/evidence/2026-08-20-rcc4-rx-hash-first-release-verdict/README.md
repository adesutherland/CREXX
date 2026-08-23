# RCC-4 `rx_hash` first-Release verdict

Status: accepted by Adrian on 2026-08-20; proceed with RCC-4 qualification.

## Verdict

The production `rxhash.sha256()` path is not slower than the accepted
test-only native control. The production and control procedure bodies compile
to the same machine instructions apart from symbol/literal identities. On the
locked 4 KiB x 10,000-hash lane, the recorded medians are neutral-to-favorable
for production on both concrete VMs:

| VM | Control median | Production median | Production delta | Control MiB/s | Production MiB/s |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxbvm` | 94,274 us | 93,749 us | -0.557% | 414.351 | 416.671 |
| `rxtvm` | 94,766 us | 93,273 us | -1.575% | 412.200 | 418.798 |

These small point differences are not claimed as an optimization. They confirm
that replacing the prototype registration/name with the production provider
does not impose a material cost and does not reopen the Option-A decision.

## Scope and sampling

- Ordinary profiling-off Release product on the user-cleared macOS ARM64 host.
- Identical deterministic 4,096-byte payload and 10,000 calls per observation.
- Test-only `crexxragsha256gate.sha256_native()` is the control;
  `rxhash.sha256()` is the production variant.
- Initial series contain one warmup and five recorded observations. The noisy
  `rxbvm` production and `rxtvm` control series retain ten additional unchanged
  observations, giving 15 recorded values for those two medians.
- All calls returned the independently checked 4 KiB digest
  `D41D438C379110C7F7B2C561B1F04F26C1B4549110791F8E022F48974280C13E`.

This is a first-Release implementation guard, not a formal product baseline or
a claim that a sub-2% difference is repeatable. Raw samples, process timings,
stdout/stderr and exact argv are retained under `cells/`; `summary.csv` is the
machine-readable verdict.

## Post-verdict qualification

After acceptance, focused Debug qualification passed:

- optimized and unoptimized vector tests on `rxbvm` and `rxtvm`;
- simultaneous static and dynamic provider use in two VM contexts;
- source-tree `crexx -native` packaging without a provider list;
- fresh scratch install with dynamic autoload on both VMs and canonical/static
  archive selection by installed `crexx -native`; and
- a read-only Level G `crexx-rag` contract probe that compiled the current
  downstream `ragschema.crexx`, called its schema/checksum surface, hashed that
  value through installed `rxhash.sha256()`, and returned the expected digest
  on both VMs without an explicit provider argument.

The complete CREXX Debug suite then passed 2,282/2,282 tests with
`ctest --parallel 30 --output-on-failure`.

That final item is a consumer-contract proof, not downstream adoption.
`crexx-rag` still marks `P1-HASH-01` as unauthorized and has no production
`rxhash` call. A read-only full downstream run configured and built successfully
against the scratch installation, but its existing baseline was 51/61 tests:
ten non-hash tests failed in HTTP/provider, package-diagnostic, and newer
compiler-keyword compatibility paths. Those pre-adoption failures are recorded
as a separate downstream sync issue and are not hidden as an RCC-4 hash failure.

# Native inference binary delivery evidence

Live work package: [numbered pipeline plan](../../planning/native-inference-ci.md).
Candidate: `temp/llama-release-qa`. The accepted baseline is `12647a91aa7e9`,
merged with remote RXPP `b5b827489` by `c1de1670812a`. Publication is not authorized
by a partial or pending result.

## Local checks before remote qualification

| Check | Result | Retained evidence |
| --- | --- | --- |
| Isolated aggregate, normal Debug | Pass, 44.527 seconds | `local/measurement-debug/` |
| Same aggregate, maintained Apple ASan | Pass, 48.266 seconds | `local/measurement-asan/` |
| Registered serial CTest, normal Debug | Pass, 28.20 seconds | `local/registered-debug/` |
| Registered serial CTest, maintained Apple ASan | Pass, 45.57 seconds | `local/registered-asan/` |
| Workflow syntax and expressions | `actionlint` passes for Build, Deep Build and Sanitizer workflows. | Workflow definitions at candidate commit. |
| Publication, signing and matrix controls | 14 Python tests pass, including existing Windows publication guards. | `scripts/tests/test_*release*.py` |

Timing calibrates hang guards, not model performance. The isolated measurements
precede CTest registration. The registered aggregate has `RUN_SERIAL=TRUE`,
`TIMEOUT=3600`, the qualification tier and explicit prerequisite targets.
Child process guards are 1,800 seconds. Ordinary candidate packaging tests its
final staged payload once; the fixture and helper are never added to it.

The fixture is 4,763,872 bytes with SHA-256
`bb9b0debefdfec589f4c6d94c6bcc38daea64ab20b48eb95809e5c367863f5de`.
The raw engine checks two prompt rows, repeated-request isolation, finite logits
and embeddings, and two private contexts using one loaded model. Public-provider
checks cover configuration factories, device discovery, explicit rejection of
the fixture as an unqualified real model and cleanup in optimized/nonoptimized
VMs plus a relocated native application. Package controls cover corrupt CPU
hashes and a missing runtime manifest. No BGE/Smol weights are downloaded.

The initial test authoring run exposed a test-local Level G scope error: `state`
was declared inside a loop and read outside it as a symbol. Declaring it in the
enclosing scope made the intended assertion executable. No product or model
loader bypass was added. The successful logs above retain the exact checks.

The existing full local gate (2,349/2,349 Apple-ASan tests), normal Debug gate
(2,347/2,347) and real-model qualification remain in the STEP-05/06 ledgers.
Those results do not claim that new hosted inputs or platforms have passed.
Linux leak detection remains enabled in the forthcoming supported-host gate.

## Remote results

Pending first candidate push. Record exact SHA, run URLs, terminal conclusions,
artifact identities and any reproduced repair here. Keep unavailable real-device
and model-provenance acceptance visible in the parent plan; fixture success
does not close those items.

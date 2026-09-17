# LLM/native-model interface review evidence

Reviewed source: `develop`, `d0feda283`, 17 September 2026.
Scope: [LLM-API-01 review and proposed plan](../../planning/llm-provider-interface.md).
The initial review below preceded implementation. Its observations describe
`d0feda283`, not the completed common-driver surface. See
[the implementation ledger](../../planning/llm-provider-interface.md#live-implementation-ledger)
for current code, acceptance evidence and outstanding checks.

## Findings exercised

| Check | Observed result |
| --- | --- |
| Existing `ts_llm_ollama_noopt`, `ts_llm_ollama_opt`, `ts_llm_providers_noopt`, `ts_llm_providers_opt` | 4/4 passed in 6.57 seconds. These are deterministic request/parsing checks, not hosted inference. |
| Declare `client = .llm`, assign `.llm("test-model")` | Compiles successfully. Positive interface/factory control. |
| Assign `.openai`, `.anthropic` or `.gemini` to the same `.llm` declaration | Each fails compilation with `#TYPE_MISMATCH`, consistent with their source declarations lacking `implements .llm`. |
| Public typed native API, fixture with its correct hash, profile `another-model` | Immediately rejects it with `unqualified model profile`. |
| Same fixture/hash under `smollm2-360m-instruct` | Constructor admits loading; it reaches `failed` with `artifact is not the pinned model profile`. Explicit repeated close and reservation cleanup pass. |
| Native restriction consumer through rxc, rxas, rxlink and both rxbvm/rxtvm | Both VM runs pass the two rejection controls and cleanup. No trained model or new download is used. |

The rejection consumer extends the existing
`tests/native-inference/release_provider_smoke.crexx` in a temporary directory.
The generated fixture is the repository's `llama-dense.gguf`; this review does
not claim that it supplies meaningful trained-model output.

[Checks and inputs](checks.txt) retains compiler/runtime commands, outputs,
consumer sources and source/binary/fixture hashes. These tests use the existing
Debug toolchain and library artifacts; the review did not rebuild or qualify
the whole checkout. Source inspection independently confirms the two profile
and hash gates, model-specific processing, and absence of hosted/native `.llm`
implementations. The results do not qualify the proposed APIs or additional
models. No broad suite, sanitizer matrix or hosted workflow was dispatched.

## Additional observation requiring implementation-time triage

Direct `.ollama(...)` construction, including namespace-qualified and explicit
argument forms, reports `#CLASS_NOT_FOUND` with these existing Debug artifacts,
while `.llm(...)` and its four functional tests succeed. Generated member RXAS
contains `rxfnsg.ollama` class/implements metadata. This review has not reproduced
the direct-constructor failure from a freshly rebuilt library or established
its compiler/import mechanism. Retain it as an open observation, not a proved
root cause or a reason to alter compiler logic. LLM-AC-02 must check direct and
interface construction from fresh outputs during implementation.

## Documentation verification

Current human and agent guides are updated to state the enforced artifact
restriction, distinguish the separate native API from `.llm`, and use the
retained qualification/SAN records for status. Installer wording distinguishes
Windows MSVC packages from MinGW core QA and allows the documented Windows
backend coexistence. Historical evidence and the parent native acceptance
criteria are retained. Product behavior is unchanged.

`git diff --check` and the local Markdown target check pass. New references from
installed guides to the proposed plan use repository paths, since that planning
document is not part of the installed guide set. The optional-plugin exception
requirement is recorded in the plan and roadmap as proposed behavior, not a
claim about the current loader.


The direct-Ollama observation was reproduced with fresh outputs during final
compatibility checks. Removing its obsolete interface matcher after moving
interface selection into `llm_client` makes direct construction compile again;
the permanent `ts_llm_common` regression checks its body/status/close behavior.
No compiler change was needed for that correction. The separate missing-factory
signal restoration defect has a permanent compiler/runtime regression.

# Cited corpus questions and search

This web application has one Level G model and two browser assemblies:

- `rag-query.package.json`: browser-local read-only corpus comparison, with
  answering visibly unavailable; its hosted-native relay entry also supports
  configured server answering.
- `rag-answer.package.json`: a browser-local UI owner with a same-origin server
  executor for answers, captured-source context and the original three search
  routes. `rag_query_web_server` selects this composition explicitly; it does
  not enable local answer generation.

Both use `rag_query_app.crexx`. The main question accepts up to 1024 characters,
including newlines. The independent corpus search retains its existing
single-line, 256-character question and graph generations 0–4. Keyword,
embedding and graph results remain distinct. A raw search does not regenerate
an answer. Application state stays in memory; follow-up questions do not
implicitly replay earlier conversation to a provider.

Numbered source controls accompany the answer. Each opens the exact supporting
excerpt, stable citation and captured UTF-8 span. The source website opens only
through a validated HTTP(S) locator. **Read captured context** retrieves a wider
bounded normalized passage from the same source/snapshot; an original-file link
appears only when the RAG server returns an approved immutable asset locator.
Captured UTF-8 offsets are positions in normalized text, not inferred PDF pages.
The UI does not invent page or section locations.
Answer prose can omit inline citation markers when its validated evidence is
listed separately. Any inline markers that are present must refer to that same
source list; unknown markers are refused.

**Dialogue and activity** is a separate expandable region. It presents the
submitted question, public service messages, completed retrieval/source reads
and the validated answer. It contains no hidden provider reasoning or private
prompts. Progress messages are actual completed work, not simulated percentages.
The previous completed answer remains available during another request and
following cancellation or failure. Editing a submitted question prevents its
late result from replacing the current answer.

## Service contracts

`rag.answer` requests `{corpus, question}` and completes as
`rag.answer.completed {answer_json}`. The bounded JSON value uses
`crexx-rag.answer/1`, an answer of at most 12000 characters, at most eight cited
sources, corpus/snapshot and provider/model identity, and one of `answered`,
`insufficient_evidence` or `scope_refused`. Sources retain their title, excerpt,
canonical citation, safe source URL and UTF-8 span. The RAG server owns retrieval,
scope policy, generation and final citation validation.

The HTTP executor first POSTs to `/api/rag/v1/answer/prepare`. Only after its
actual retrieval/scope result arrives does the executor publish those bounded
activities. It can read up to four selected citations through
`/api/rag/v1/source`, reporting each completed read. It then POSTs the prepared
snapshot/selectors to `/api/rag/v1/answer`; the server independently validates
them. There is no automatic retry of paid work. Provider keys and the configured
backend origin remain on the server.

Nonterminal progress reuses `ui.effect.progress {completed, message}` with the
ordinary request/session/target correlation fields. `completed` counts reported
activities; it is not a percentage or a prediction of total work. Terminal
failure/cancellation use the existing generic outcomes. Stop cancels the logical
operation and aborts the browser HTTP request or hosted service operation; late
results cannot replace it.

`rag.source {corpus, citation}` completes as
`rag.source.completed {source_json}` using `crexx-rag.source-context/1`. Its
source/snapshot, exact cited excerpt, normalized context and UTF-8 ranges are
validated before presentation. Original paths are approved asset IDs under
`/api/rag/v1/original/`, never filesystem paths supplied by a browser.

## Build and local assembly

Build the owning UI modules and all three browser entries through
`example_ui_rag_query_browser`. Assemble a server-backed browser site by passing
`examples/ui/rag-query/rag-answer.package.json` to `web/ui/package-site.mjs` in
place of the local-only RAG declaration. Use the existing RAG static packaging
route with `rag-query.package.json` for the separate browser-local corpus demo.
Do not combine the two declarations with their identical application ID in one
site manifest.

`query-api-server.mjs SITE` is the loopback development host. Its query route
uses the existing native reader configuration. Answer/source paths proxy to the
server-only `CREXX_UI_RAG_HTTP_ORIGIN`; no value from that setting is placed in
the browser package. `CREXX_UI_RAG_CORPUS` restricts the hosted application
adapter. Deployment authentication, provider configuration, original-asset
rights and distribution remain the owning RAG/hosting responsibilities.

Captured-original navigation is supported by the direct same-origin HTTP
assembly. The older bearer-token native UI relay has no original-file route;
its application adapter omits relative original-file locators while retaining
the publisher URL, supporting excerpt and captured context. It does not render
a link that would require an unsupported route.

The browser operation requirements include semantic groups, multiline input,
read-only documents, links and status. Native GTK/ANSI compositions do not
advertise these additions and refuse such a view before partial projection.
No native answer UI is claimed.

## Focused validation

The registered checks are `ui_rag_query_effect`, `ui_rag_query_hosted_service`,
`ui_rag_query_http_executor`, `ui_rag_answer_effect`,
`ui_rag_answer_http_executor` and `ui_web_semantic_contract`.
`test-answer-browser.mjs` uses a site assembled with the answer declaration and
mock HTTP responses with the real UI owner. Set `CREXX_UI_ORIGIN`, and the usual
`CREXX_PLAYWRIGHT`/`CREXX_CHROME` overrides when necessary. It covers multiline
submit, completed activity, literal text/link safety, citation/context
navigation, independent raw search, narrow layout and cancellation without
provider calls. These development checks do not establish hosted access,
provider quality, public distribution or full UI-plan acceptance.

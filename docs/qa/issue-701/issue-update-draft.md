# Draft update for #701 — not posted

The generic launcher repair is on the persistent `hotfix` branch; promotion and
full core qualification are still pending.

The POSIX regression forces B to launch while A's stdout writer is still open
in the parent. The defective launcher lets B retain it. With the repair, A's
output drains and completion returns while B remains alive behind a handshake.
Both pipe ends are close-on-exec, and the existing launch mutex covers their
creation/flag setup as well as fork. Legacy string/array redirects share that
pipe path. Failure injection checks second-end flag setup and cleanup.

Windows MSVC and MinGW now independently reproduce the related file-retention
mechanism. With no or partial redirection (masks 0–6), the old launcher passes an
unrelated file handle to B; closing it in the parent still leaves rename failing
with sharing violation 32. Full redirection is the passing control. The repair
uses private standard-handle duplicates and an allowlist for every combination.
All 16 cases (eight redirect masks, inheritable/private parent std handles) pass:
the unrelated file is absent, rename succeeds while B remains alive, and intended
stdin/stdout/stderr work. FOPEN now creates files privately rather than clearing
inheritance after opening.

This demonstrates the proposed Windows mechanism. It does **not** attribute the
historical CI-F18 silent project-build failure to that mechanism. Worker
diagnostics are being qualified to distinguish missing assembled output, stamp
write/NOTREADY, and stamp rename failures without retries.

Evidence so far:

- Baseline launcher blob: `c88ab671291d5ddf8f5a7d276c553a44babba3b0`.
- Baseline source revision: `b5b827489d781f9e42d305ef264a22d2c1c42cb6`.
- [Four-platform deterministic controls](https://github.com/adesutherland/CREXX/actions/runs/35086663828): success at `bd19a94ddd597459eb1fa01cae1cf33294d063b5`.
- Focused local Debug and maintained Apple ASan: both pass; Apple LSan is unsupported.
- Full Linux ASan/LSan, broader core gates, final revision and develop promotion:
  pending; fill in terminal evidence before posting this draft.

No RAG workaround, model workload, issue comment, release or version tag is part
of this repair.

# Draft update for #701 — not posted

The generic launcher repair and qualification-policy clarification reached
`develop` at `bcea0f71bbfeaa263b274155317e7162de187f0a`, through persistent
`hotfix`. Core qualification and the normal Build CREXX publication workflow passed.
CodeQL continues as background analysis and is not claimed as complete.

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
inheritance after opening. Another 16 controls cover NULL/invalid standard
handles, and handle counts remain unchanged after successful launches and
CreateProcess failure cleanup.

This demonstrates the proposed Windows mechanism. It does **not** attribute the
historical CI-F18 silent project-build failure to that mechanism. Worker
diagnostics now distinguish missing assembled output, stamp
write/NOTREADY, and stamp rename failures without retries.

Evidence so far:

- Baseline launcher blob: `c88ab671291d5ddf8f5a7d276c553a44babba3b0`.
- Baseline source revision: `b5b827489d781f9e42d305ef264a22d2c1c42cb6`.
- Final code candidate: `135b9254fffdd0c9a8e963d1092c68bb273bf66b`.
- [Four-platform deterministic controls](https://github.com/adesutherland/CREXX/actions/runs/35088226165): success on that exact candidate.
- Focused local Debug and maintained Apple ASan: both 7/7 pass; Apple LSan is unsupported.
- Broad local Debug: 2,306/2,306 pass in 902.27 seconds, without retries.
- Hosted Linux and macOS ARM64 comprehensive: each 2,292 correctness plus three install/package tests pass.
- [Deep Build QA](https://github.com/adesutherland/CREXX/actions/runs/35088225950): success on the exact candidate, all supported platforms and install/package checks.
- The optional full sanitizer run was cancelled at Adrian's direction after the appropriate core/functional gates passed. Full Linux ASan/LSan is not claimed.
- Develop repair revision: `bcea0f71bbfeaa263b274155317e7162de187f0a`. [Build CREXX](https://github.com/adesutherland/CREXX/actions/runs/35093060282) is terminal success. [CodeQL](https://github.com/adesutherland/CREXX/actions/runs/35093060306) is still running as background assurance; refresh its status before posting. Subsequent commits change documentation/evidence only and reuse the unchanged code qualification.

No RAG workaround, model workload, issue comment, release or version tag is part
of this repair.

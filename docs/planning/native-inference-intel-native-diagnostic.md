# Isolated Intel native stall diagnosis

This branch-only diagnostic serves CI-F07 and CI-AC-07 in
`docs/planning/native-inference-ci.md`. Do not merge its workflow into the
delivery candidate. The source parent is `76df02be3`; the task remains bounded
diagnosis, not a new acceptance contract or performance study.

1. Reuse the exact Intel archive from Build `35013130021`, SHA256
   `91a4fb9f8347d582789fc1ee986cc0d646236d145de0c6ffe2ffaa30e49f187c`.
   Compile only the ordinary native smoke consumer; no engine rebuild or model
   download is needed.
2. Relocate the executable and manifest dependencies, remove its original
   directory, and repeat the unchanged public provider control up to five times.
3. If a process remains active for 180 seconds, retain a macOS stack sample and
   stop with failure. This diagnostic threshold does not replace the ordinary
   1800-second qualification guard or turn a stopped test into a pass.
4. Retain all output and record whether the stall reproduces. A passing replay
   does not establish its cause or close the full exact-candidate QA gates.

The motivating failure is Intel Deep job `104512381773` in run `35007946063`:
2329 comprehensive tests and three qualification tests passed, but the final
relocated native smoke command timed out after 1800 seconds without output.
The later standard Intel package job `104529899418` passed the same command.
No sanitizer finding was observed in that failure.

The replay harness was checked locally against the corresponding ARM archive:
native build plus two relocated executions passed. The production smoke's
opt-in generic stack capture also passed a deliberately delayed compiler
control and unchanged Debug/Apple-ASan smoke. These are harness controls;
the Intel evidence remains to be collected.

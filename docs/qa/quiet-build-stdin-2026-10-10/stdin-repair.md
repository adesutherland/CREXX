# Deterministic stdin for negative PULL tests

CP-C-FIND-05 is repaired and qualified locally, and included in the authorized
10 October `origin/develop` development publication.
CLion kept its parent stdin pipe open. Both negative PULL cases inherited that
pipe and waited indefinitely; the helper had no whole-test timeout. The
[original process sample](stdin-original-sample.log) shows the VM in
`FREADLINE` and `read`. Only the two confirmed blocked children were terminated,
allowing the rest of the user's run to continue.

The negative Classic syntax helper now supplies an empty input file for
portable EOF and declares a 600-second hang backstop. The 26.4 error identity,
authored source location and exact prior `before` output checks are unchanged.
Neighboring ARG/PARSE and other negative cases keep their existing assertions.

Both variants pass with a parent stdin pipe deliberately held open: **2/2 in
Debug (3.10 seconds) and 2/2 in Release (0.73 seconds)**. The regression command
starts CTest with `stdin=subprocess.PIPE` and uses `wait()` before closing the
pipe, so the proof does not accidentally supply parent EOF via `communicate()`.
[stdin-qualification.json](stdin-qualification.json) retains input/log hashes.
The related warning/output repair has its own [receipt](quiet-build.md).

The user's earlier full CLion run is not a full pass. It also had missing QA
fixture artifacts after a product-only rebuild; `all` does not replace
`qa-prep` for the full registered suite. Those other failures are distinct from
these repaired cases. No full-suite or sanitizer-clean claim is made here;
SAN-011/012/SAN-QA-018 retain their assigned Release 1 QA closure.

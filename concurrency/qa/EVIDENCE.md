# Platform evidence contract

Each qualification runner writes one self-contained directory containing:

| Path | Meaning |
| --- | --- |
| `RESULT.txt` | final PASS identity and counts; absent on interrupted runs |
| `qualification.log` or `qualification-transcript.txt` | complete orchestration transcript |
| `provenance.txt` | commit, clean status, host/toolchain and CMake cache identity |
| `configure.log` | fresh configuration and selected TLS backend |
| `build.log` | complete ordinary product build |
| `concurrency-matrix.log` | maintained labelled matrix with live TLS enabled |
| `tls-live.log` | verbose trusted-host and hostname-mismatch proof |
| `stress-repeat-20.log` | 20 unchanged executions of every stress case |
| `full-ctest.log` | complete platform regression suite |
| `labels/` | exact CTest inventories for the umbrella and SP-01 through SP-09 |
| `install/` | installed inventory and direct installed-toolchain smoke |
| `package/` | staged inventory, archive/package checksums and extracted-package smoke |
| `SHA256SUMS` | recursive digest of closed evidence files, excluding itself and the still-open orchestration transcript |

Package binaries are ordinary reproducible build outputs. The runners keep
them in the sibling `EVIDENCE_DIRECTORY-artifacts` directory for platform
handoff; commit only the evidence and digests unless the release workflow
explicitly asks for the binary assets.

When reporting a failure, add `FAILURE.md` with the first failing command,
test, exit code, diagnostic excerpt, whether it repeats unchanged, and the
smallest source/test input that reproduces it. Do not overwrite the original
logs after a repair; create a new evidence directory for the new commit.

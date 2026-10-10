# Windows Classic source broker CI repair

The renewed section of
[the pipeline plan](../../planning/release-1/ci-core-stream-packaging-2026-10-10.md#renewed-pipeline-regression-and-explicit-full-qa--10-october-2026)
owns CI-CS-AC-10–13 and the explicitly requested full sanitizer/deep gates.
This is development repair and assurance, not release qualification or a new
language/architecture decision. Expected delivery is published `develop` with
Build, CodeQL, full Sanitizer QA and complete Deep Build QA passing; actual
hosted results belong to their GitHub run records.

## Reproduced cause and repair

[Build CREXX 38083859225](https://github.com/adesutherland/CREXX/actions/runs/38083859225)
fails at synchronized `64e4a30a16a1207d2d0834e6053ff69df30bfe39`. Both MSVC and
MinGW fail compiling `rxcp_c_source_import.c`: Windows SDK typedefs conflict
with AST enum names `FLOAT`, `PATTERN` and `DECIMAL` in the same C namespace.
Changing header order would not repair those declarations.

The broker now uses the existing isolation pattern from the editor brokers:
rename the conflicting AST spellings while including compiler declarations,
then remove those local macros before including Windows headers. The underlying
enum values, AST layout, inline payload identities, callable contracts and
source-import/editor behavior remain unchanged. No Windows SDK type is renamed
or warning suppressed. No backend/parser-header dependency is added.

The retained minimal C control extracts the actual NodeType declaration and
introduces the conflicting SDK typedef names. The original fails with the
same three redeclarations. The repaired control compiles and reports identical
numeric values `39 91 42`. This is namespace diagnosis, not a substitute for
actual Windows product/deep builds. [collision-results.json](collision-results.json)
records the control; its source and diagnostic logs are in the local evidence
directory below.

## Qualification and monitoring

Core compiler/frontend regression targets build in normal Debug. The permanent
`metadata_contract_conformance`, `compiler_contract_builder`,
`classic_frontend_service` and `classic_frontend_parity` panel passes 4/4 in
48.17 s. The ownership regression includes the repaired SAN-011/012 cases;
standalone frontend parity exercises typed imports through the owning compiler.
The same maintained ASan panel passes 4/4 in 119.26 s before publication, using
`tools/asan-run.sh` for build and CTest with one test worker. Apple LSan is
unavailable; supported Linux build/test leak detection stays enabled in the
full requested gate.

[inputs-and-results.json](inputs-and-results.json) records the base tree, exact
changed-code SHA-256, original failed log and focused qualification. Local
logs are retained at `output/ci-regression-20261010.79vob9/`, with the current
path in `output/ci-regression-current.path`. `active-runs.json` records the
current phase, exact published revision and workflow IDs after dispatch.

The Intel Mac job passed its product, ZIP/SDK consumer, signing and ZIP
notarization checks. Its subsequent installer upload also succeeded before
notarytool waiting failed with Apple's offline/no-network-route error. Treat
that observed failure as a transport retry; signing/notarization must still
pass. The earlier `10e8184e3` missing static stream archive is already repaired
and has a separate retained receipt.

Adrian requested full QA and continued repairs until green. Dispatch the full
Sanitizer QA workflow (Linux x64 ASan/LSan and macOS arm64 ASan) and Deep Build
QA with `platform=all` on the repaired published code. The complete Deep gate
covers all five core platforms, comprehensive/qualification checks, books and
parallel product graph checks. The sanitizer gate excludes only the established
performance-measurement tier; first-party findings are never suppressed.
Normal Build/CodeQL run automatically after publication.

The thread heartbeat `repair-crexx-ci-until-green` resumes the authoritative
plan every ten minutes, preserving healthy work and remaining quiet on
unchanged non-actionable status. It diagnoses actual failures, publishes
substantive repairs with retained evidence, and stops only after every required
gate and mandatory finding disposition is complete. Current SAN-011/012 and
SAN-QA-018 proof remains open until its recorded closure checks are satisfied.
Do not claim overall completion from a successful local panel or workflow launch.

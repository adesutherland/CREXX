# Windows snapshot installer integration

Authoritative scope: CI-F21 in `docs/planning/native-inference-ci.md`.

Local checks:

- `python3 -m unittest discover -s scripts/tests -p 'test_windows*.py' -v`:
  19 pass, one native Windows-only signature control skipped.
- `python3 -m unittest discover -s scripts/tests -p 'test_llama*.py' -v`:
  28 pass, 14 compiled native-manager controls skipped.
- `actionlint .github/workflows/build.yml`, Bash syntax, Python compile and
  `git diff --check`: pass.
- Default signer dry run pins both current `d8f59732d` inputs and the exact
  successful Build manager. A real artifact-ID download verifies its GitHub
  SHA-256 plus manager manifest/bootstrap files (see `manager-preflight.log`).

Regression coverage includes actual complete/missing-setup collector execution,
four-output publication, replaced plugin during upload, mismatched revisions,
unsigned input preservation, upload rollback, expired/failed/mismatched manager
sources and corrupt/incomplete output rejection.

Hosted Build/CodeQL, native unsigned lifecycle and current snapshot signing
remain pending. Existing broad product and F20 overnight evidence is unchanged.

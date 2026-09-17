# Windows snapshot installer integration

Authoritative scope: CI-F21 in `docs/planning/native-inference-ci.md`.

Local checks:

- `python3 -m unittest discover -s scripts/tests -p 'test_windows*.py' -v`:
  19 pass, one native Windows-only signature control skipped.
- `python3 -m unittest discover -s scripts/tests -p 'test_llama*.py' -v`:
  28 pass, 14 compiled native-manager controls skipped.
- `actionlint .github/workflows/build.yml`, Bash syntax, Python compile and
  `git diff --check`: pass.
- Initial signer dry run pins both then-current `d8f59732d` inputs and the exact
  successful Build manager. A real artifact-ID download verifies its GitHub
  SHA-256 plus manager manifest/bootstrap files (see `manager-preflight.log`).

Regression coverage includes actual complete/missing-setup collector execution,
four-output publication, replaced plugin during upload, mismatched revisions,
unsigned input preservation, upload rollback, expired/failed/mismatched manager
sources and corrupt/incomplete output rejection.

Build, CodeQL, native unsigned lifecycle and current snapshot signing/publication
pass. All F21 criteria are verified. Existing broad product and F20 overnight
evidence is unchanged.

Published implementation: `d0feda283857e9470e21fc143f8e2f70fa54ab3b`.
Automatic [Build 35253939469](https://github.com/adesutherland/CREXX/actions/runs/35253939469)
and [CodeQL 35253939262](https://github.com/adesutherland/CREXX/actions/runs/35253939262)
are the qualification runs. No manual overnight matrix was dispatched.

Build is terminal success; `native-unsigned/native-installer.log.gz` ends with the
passing full unsigned native lifecycle. The unsigned llama setup is published.
Native logs are gzip-compressed to preserve their original Windows line endings;
`native-unsigned/log-hashes.json` records the uncompressed byte hashes.
The default signing command built and verified 53 core, 11 manager and 31 plugin
PE files plus both installers. Inputs and final output hashes are in
`signing-source.json`, `signing-manager.json` and `signed-delivery.json`.

Initial publication returned GitHub HTTP 500 while saving the core ZIP, after
successfully staging the core setup. Owned temporary uploads were removed.
A retained-output retry then stalled after transferring its complete core ZIP;
its `gh` child was terminated and helper cleanup completed. A second
retained-output retry using `GODEBUG=http2client=0` also stalled and was stopped.
A bounded direct API attempt returned HTTP 500. Neither transport change proves
an HTTP/2 root cause; both ZIPs pass CRC verification. The initial complete
signing log and recovery-attempt logs are retained.

Publication recovery succeeds without changing or regenerating any signed bytes:

- Both signed installers publish using the existing guarded helper.
- The core ZIP from the earlier interrupted request appears late as asset
  `570836439`. Its remote SHA-256 equals the retained signed output. After
  rechecking both source assets and tag commit, promote that owned staged asset
  to its final name (see `recovered-core-upload.json`).
- The signed plugin ZIP publishes through the same guarded helper.
- `published-verification.json` verifies all four final signed hashes, preservation
  of all four unsigned downloads, and absence of temporary signing uploads.
  `release.json` retains the final release metadata.

Retained local work: `/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-snapshot-sign-z83eqwmt`.
All signing/upload processes have ended. CodeQL `35253939262` is terminal success
on the same implementation revision (`codeql.json`, `codeql-analysis.json`).
Final tag/assets inspection confirms `d0feda283`, all eight Windows downloads,
and no temporary signing uploads. Close F21-AC-03 / F21-03; retain this evidence
on the task branch without triggering another unchanged development build.

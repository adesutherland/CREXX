# F18 merged-candidate retest

- Candidate: `2b897caa55fbe564c6d12966db6fea349f327986`.
- Merged develop repair: `17e844441ed87e1f6e0d5f1f0d3bb4bee8db6187`.
- Local Debug: 9/9 pass, 91.90 seconds; project-build contract 84.23 seconds.
- Hosted MinGW core run: [35104282036](https://github.com/adesutherland/CREXX/actions/runs/35104282036), terminal success.
- MinGW smoke: 155/155; project-build contract 31.59 seconds, resource inheritance 0.58 seconds, private file opening 0.01 seconds, worker diagnostics 1.09 seconds.
- KeyAccess: 3/3. Extracted core passes both optimization modes, both applicable VMs and relocated native consumer.
- Runtime is built with `ENABLE_LLAMA=OFF`; no CUDA, model, Deep or full sanitizer run was requested by this retest.
- Nine repair/test source blobs match the qualified hotfix; see `../../local/f18-merged-retest/repair-identity.json` and local command/build/test logs in that directory.
- Reused four-platform deterministic baseline-negative/current-positive evidence: `docs/qa/issue-701/remote-135b925/inheritance/`, run 35088226165.
- The reproduced resource-inheritance mechanism is repaired and integration passes. The original silent CI-F18 incident remains unattributed; a passing project-build retry is not retrospective proof of its cause.

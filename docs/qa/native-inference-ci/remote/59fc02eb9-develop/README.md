# Final development integration checks

Promoted revision: `59fc02eb905ea2a0878e4055b7114921d5e6a294`.
Both required normal automatic workflows are terminal success:

- [Build 35157087747](https://github.com/adesutherland/CREXX/actions/runs/35157087747):
  four shipped cores, MinGW core-only gate, optimizer parity, all four routine
  plugin variants, Mac signing/notarization/package steps and snapshot publication.
- [CodeQL 35157087467](https://github.com/adesutherland/CREXX/actions/runs/35157087467):
  C/C++ analysis completes successfully on that same revision.

`build.json` and `codeql.json` retain exact run/job/step metadata. `qa.tar.gz`
retains all five core and four plugin QA artifacts. `snapshot.json` retains
published development asset names, sizes and hashes; `remote-refs.txt` identifies
the observed develop and snapshot refs. Routine CUDA exclusion is verified.
Both CUDA delivery variants have retained successful manual-candidate proof;
they were not rebuilt in this routine run.

The small Windows QA child-environment repair is integrated. Its separate
signed-installer [native proof](../a6c989d85-windows-signed/README.md) passes for
core, Vulkan and CUDA; the temporary private draft is deleted and no tag was
created. Windows snapshot signing remains an optional later distribution step.
The current snapshot's Windows installer is explicitly unsigned.

PROM-03 is complete. No further product repair or QA rerun is required for this
development integration. The heartbeat is paused after recording this result.
This evidence-only closeout is retained on `temp/llama-release-combined`, without
another develop push or redundant automatic-build cycle. Full release publication
and parent real-device/model acceptance remain outside this completed phase.

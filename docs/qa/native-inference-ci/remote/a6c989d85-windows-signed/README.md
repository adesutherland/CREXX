# Signed Windows native installer qualification

[Run 35155144009](https://github.com/adesutherland/CREXX/actions/runs/35155144009)
passes at QA revision `a6c989d8587f00eaf42de421a98a4ddfccaf10a4` using the signed
`21a5e9410925cac7e4f577119e4e09fe6393d7f4` core, Vulkan and CUDA artifacts. No core
or engine rebuild, new signing session or GPU hardware claim is involved.

- Native module-shadowing reproducer, signed positive and unsigned rejection
  controls pass (1.575 seconds). The earlier run failed before assessing signatures.
- Final signed setup executables pass Windows Authenticode verification.
  After installation, all executable/DLL/plugin payloads and embedded uninstallers
  pass the same check, including both stored variants and the native Rexx manager.
- Both installers discover the registered core in a path with spaces. Coexistence
  preserves the selected backend. Native Rexx switching works with an SDK-free PATH.
- Installed Vulkan and CUDA consumers each pass compiler, assembler, linker and VM
  provider discovery, profile rejection and cleanup. This is package/provider QA,
  not trained-model inference or real GPU computation.
- Reinstall verifies the final manifest; individual removals preserve core/model
  files and environment; the core still runs afterward. Reinstall plus core removal
  cleans backend registrations and restores the original environment.

`qa.tar.gz` contains exact source/signed-input records and all control, consumer
and lifecycle logs. `job.log.gz` and `run.json` retain terminal job/run evidence.
`staging-before-delete.json` records the seven private transfer assets and hashes.
`cleanup.json` records deletion of private draft 390272446, its API 404 and the
absence of its tag. Local signed files remain in
`/tmp/crexx-final-21a5e9410/signed-retry`; they were not published as release assets.

This closes the candidate's optional signed Windows qualification. It does not
sign or publish the current development snapshot. Morning/future Windows signing
remains optional and does not gate development integration.

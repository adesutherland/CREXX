# Level L develop CI repair — 12 October 2026

This continuation owns CI-AC01–03 in the Level L plan. The failed input is
`bb973735fd8ccc3d89b5222da80892daf1189e0a`.

## Failures and repair

- [Build CREXX 38298866712](https://github.com/adesutherland/CREXX/actions/runs/38298866712)
  failed Windows checkout because `nul.txt` is a reserved Windows device name,
  including when it has an extension. Both MSVC and MinGW stopped before
  configuring. The 48 retained inputs now use `nul-byte.txt`; the generator's
  case name also uses that spelling. `fixture-relocations.json` records each
  old/new path and unchanged SHA-256. Historical command logs keep their original
  paths; the integration panel manifest uses the current paths.
- Linux Release and [CodeQL 38298866314](https://github.com/adesutherland/CREXX/actions/runs/38298866314)
  failed when linking `rexxscript1`: its cstream/fs static providers appeared
  after `rxpashim`, so GNU ld could not resolve their RXPA imports. The target now
  adds both providers before the VM/shim, matching the existing `crexx` runner.
  No runtime implementation or shared linker helper changes were required.
- macOS ARM product, normal QA, installed core and SDK checks passed. Apple
  accepted the installer submission `d5d4b3b8-1ad7-4f1d-9585-76fc9d251127`.
  Stapling then failed with `NSURLErrorDomain -1005` (lost connection to Apple's
  ticket service), exit 68. This is a service failure; the new publication run
  retries the normal packaging steps without weakening signature checks.

The retained `*-before.log` files contain the checkout/path-guard evidence and
focused linker excerpts, with line endings and trailing whitespace normalized.
Full hosted logs remain attached to the runs above.

## Regression and qualification

`scripts/tests/test_windows_checkout.py` runs inside the existing Linux-hosted
Windows publication safeguards, before the Windows jobs start. It checks tracked
paths on every host and covers device names with extensions and directory
components. The new guard fails on the original 48 paths and passes after the
rename. The existing native product build and installed RexxScript smoke tests
exercise the repaired link target on each maintained platform.

Local qualification on macOS ARM64 Debug passes:

- `stage-product`, including the relinked `rexxscript1`; `product-build.log`.
- Four generated Level L / modern RexxScript CTests, on rxtvm and rxbvm;
  `focused-qa.log`.
- Native RexxScript expression, decimal, Unicode and syntax-refusal checks;
  `native-runner.json`.
- The comment backend panel: 586 observations, 733 commands, including all 32
  renamed NUL-byte cases across LL(1), LR(1), PEG and Earley, source/RXAS and both
  VMs; `comments-receipt.json` records commands and exact product hashes.
- Windows safeguard suite: 22 passes and one platform-specific skip; package
  unit suite: 13 passes. See `windows-guards.log` and `package-guards.log`.
- All 3,169 retained integration-panel file hashes still match the updated
  path manifest. Each of the 48 renamed inputs retains its original byte hash.

`qualification.json` records the changed code/test inputs and native artifacts.
No grammar, lexer, parser or evaluator implementation changed; the broader
unchanged semantic panels retain their earlier evidence.

The substantive repair will be pushed to `develop`; normal Build CREXX and
CodeQL success is expected and will be verified in the conversation. Hosted
status does not require another repository commit. Wider Level L research and
overnight sanitizer qualification retain their existing scope.

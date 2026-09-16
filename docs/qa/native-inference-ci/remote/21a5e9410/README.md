# Final compiled delivery and signed packaging qualification

Product/build SHA: `21a5e9410925cac7e4f577119e4e09fe6393d7f4`.
[Build 35143588581](https://github.com/adesutherland/CREXX/actions/runs/35143588581)
was explicitly dispatched with all six plugins and candidate Mac signing.
It does not publish a release or modify develop.

All compiled core/plugin artifacts are qualified. The run itself is **failed**:
both Mac jobs pass payload signing, combined package smoke and ZIP notarization,
then fail their subsequent installer construction. `codesign -R` interpreted a
literal requirement as a filename. The correction at `0a6a8cbf9` adds the required
`=` prefix. Its permanent native control fails before the repair, then accepts
Apple-signed code and rejects an ad-hoc signature after it. Fourteen installer
controls pass. Retained-artifact signed installer QA remains pending.

| Lane | Retained result |
| --- | --- |
| Linux GCC core | 167 smoke tests and extracted/native consumer pass |
| Windows MSVC core | 153 smoke tests and extracted/native consumer pass; public rxvm selects rxbvm |
| ARM and Intel Mac cores | 167 smoke tests per host, extracted/native consumer, real signing, ZIP/package notarization, stapling and Gatekeeper assessment pass |
| MinGW core-only gate | 157 smoke and 3 KeyAccess tests, extracted/native consumer pass; no binary delivery |
| Linux Vulkan / CUDA | Both provider and combined package checks pass |
| Windows MSVC Vulkan / CUDA | Both provider and combined package checks pass against the same core |
| ARM Metal / Intel CPU | Both provider and signed combined package checks pass; installer-only failure described above |
| CUDA compiler cache | Linux 99.78% hits (2724 hits / 6 misses); Windows 99.85% (2641 / 4). This is compiler reuse, not inference performance. |

`source.json` and `source-jobs.json` retain terminal identities and step results.
`core-qa.tar.gz` and `plugin-qa.tar.gz` retain actual package identities, smoke
logs, fixture controls and CUDA statistics. Compressed job logs preserve core,
CUDA and Mac installer diagnostics. `build-progress.json` is an earlier live
snapshot, superseded by the terminal source records.

The packaging-only retry must validate this terminal Build's full matrix and
reject any failure other than the named Mac installer step after successful
smoke/upload/notarization. Six controls cover that validator. It will repackage,
notarize and staple the unchanged signed Mac payload, then test offline install.
Windows final Authenticode signing and actual signed installer lifecycle are
separate outstanding proof. The final Windows set includes the matching core,
both plugin variants and one shared native Rexx manager from this Build run.

Unchanged Deep/core sanitizer evidence at `f10e70ee5`, qualified #701/#699 fixes,
focused merged import/RXPA/rxfs tests and prior unsigned installer lifecycle are
reused under the approved plan. No broad repeat or real-model download is
introduced. Parent real-device/model criteria remain open; no develop promotion
or public release/tag is authorized here.

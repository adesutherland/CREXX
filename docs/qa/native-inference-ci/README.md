# Native inference delivery qualification

The [pipeline plan](../../planning/native-inference-ci.md) owns CI-D01–03,
acceptance criteria, missing platform/device proof and delivery decisions.
This summary retains the last completed delivery boundary; it is not proof of
a later release candidate. Superseded local/remote captures and their checksums
are available in Git history.

## Consolidated delivery result

**Final development closeout, 17 September (UK):** promoted `59fc02eb9` passes
normal [Build 35157087747](https://github.com/adesutherland/CREXX/actions/runs/35157087747)
and [CodeQL 35157087467](https://github.com/adesutherland/CREXX/actions/runs/35157087467).
The development snapshot is published. [Terminal metadata and QA artifacts](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-ci/remote/59fc02eb9-develop/README.md)
close PROM-03; the light monitor is paused. Windows signed-installer qualification
and private staging cleanup are complete as recorded below. Signing the current
Windows snapshot remains optional; parent real-device/model acceptance stays open.

**Develop integrated:** `8ed983afd` is promoted and normal
[Build 35150686647](https://github.com/adesutherland/CREXX/actions/runs/35150686647)
passes, including optimizer parity, all four cores, MinGW, all four routine
plugins and development snapshot publication. CodeQL `35150686250` also passes. The normal development
integration gates are complete. Routine CUDA exclusion matches CI-D03; the full manual candidate run
below supplies the CUDA evidence.

All four cores, the MinGW gate and six plugin smoke
checks pass on `21a5e9410`. The Build run's Mac packaging failure is repaired;
both signed/stapled plugin installers pass packaging in `35149300574` and actual
fresh-host offline Gatekeeper/install/consumer QA in `35149778131`. Installed
Linux rxfs passes `35149445867`. Windows signing and native signed
installer QA are complete for that candidate. Adrian explicitly makes Windows signing
optional after integration, permits it to wait until morning, and authorizes
promotion plus light monitoring/remediation of normal develop CI. Parent
real-device/model acceptance remains open. Older pending snapshots below are
historical and do not override this authority or the retained current results.

Optional Windows signed QA now passes in
[run 35155144009](https://github.com/adesutherland/CREXX/actions/runs/35155144009)
at QA revision `a6c989d85`, using unchanged signed `21a5e9410` artifacts. Its
native module-path control proves the earlier harness failure and the repair;
actual setup/payload/uninstaller signatures and the full Vulkan/CUDA installed
lifecycle pass. [Retained proof](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/native-inference-ci/remote/a6c989d85-windows-signed/README.md) also
records deletion of the temporary private draft and confirms its tag is absent.
Windows signing remains optional after development integration. This evidence
qualifies the candidate signing path, not signed publication of a newer snapshot.

## Qualification boundary

The accepted split pipeline builds four llama-free cores once, qualifies the
six optional llama variants against those exact artifacts, and keeps
first-party core sanitizers separate from upstream llama/CUDA. The complete
manual candidate matrix included CUDA; routine development lanes exclude it
under CI-D03. Signed installation evidence qualifies its recorded package
identity, not a newer unsigned snapshot.

Parent real-device/model, resource-failure, sharing/drain and provenance
criteria remain governed by the
[native-inference plan](../../planning/native-inference-backlog.md).
The current beta candidate's remaining release gates belong in the
[formal candidate plan](../../planning/beta-3/formal-candidate-2026-09-30.md).

## Cold backend initialization guard

The permanent `rxllama_backend_probe_cycle` initializes the engine as well as
checking probe/reopen ownership. Cold Metal device/shader preparation can exceed
a short observation window, so its guard is the 1,800-second hang backstop with
`RUN_SERIAL=TRUE`. The guard correction passed focused normal Debug and Apple
ASan checks. It did not explain CI-F07's earlier 1,800-second engine stall;
that finding's disposition remains in the pipeline plan. Detailed probe logs
are available in Git history.

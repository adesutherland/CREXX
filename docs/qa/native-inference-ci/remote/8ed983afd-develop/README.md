# Promoted development revision — 16 September

`8ed983afdc2dad723b14f7a84d985c1c5ea88b32` was fast-forwarded to develop from
`94f2f228c` under Adrian's explicit promotion/monitor/remediate authority.

[Build 35150686647](https://github.com/adesutherland/CREXX/actions/runs/35150686647)
is terminal success. `build.json` retains every job and step: four shipped cores,
MinGW core gate, optimizer parity, four routine plugins and development snapshot
publication. `snapshot.json` retains the resulting release asset names, sizes
and digests. CUDA is absent as required by routine CI-D03 cadence; both CUDA
packages have separate passing manual-candidate evidence at `21a5e9410`.

[CodeQL 35150686250](https://github.com/adesutherland/CREXX/actions/runs/35150686250)
remains in progress at this checkpoint. PROM-AC-03 stays open until its terminal
result. No unchanged Deep/sanitizer gate was dispatched again.

Windows signed installer QA is an optional independent follow-up. The ordinary
snapshot Windows assets remain explicitly unsigned. No full version release or
parent real-device/model acceptance is implied by development publication.

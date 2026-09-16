# Optional signed Windows installer QA — first run

[Run 35153176156](https://github.com/adesutherland/CREXX/actions/runs/35153176156)
failed at QA revision `9048acdc1`, before evaluating any signature. The
PowerShell 7 workflow launched Python, which passed PS7 module paths into
Windows PowerShell. Its security module could not load. This is a test-host
environment failure, not evidence of an invalid signed package.

`staging-verified.json` records all seven private draft assets matching the
local signed output hashes. `qa.tar.gz` retains the runner's exact source and
signed-input verification plus the failure log; `job.log.gz` and `run.json`
retain terminal GitHub evidence. No installer was executed by this attempt.

The focused retry is `35155144009` at `a6c989d85`, with a native module-shadowing
negative control, signed positive control and unsigned rejection control. It
reuses the exact same signed `21a5e9410` files without rebuilding or signing.
Private draft `390272446` remains temporary QA transport and must be removed
after terminal disposition. Windows signing is optional after development
integration and does not gate the already successful develop Build.

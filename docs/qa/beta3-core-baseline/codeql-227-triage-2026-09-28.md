# CodeQL #227: provider path in a local OOM diagnostic

Review-only receipt, 28 September 2026. The published product revision is
`9f2f44cfd888d324858769809b0381e524850b4c`. The local checkout at
`499d69342` contains subsequent status documentation only. No product source,
test, alert state, or sanitizer job was changed for this triage.

## Finding and exact flow

[Alert #227](https://github.com/adesutherland/CREXX/security/code-scanning/227)
is open on `develop`, rule `cpp/system-data-exposure` (warning, medium), with
source `interpreter/rxvmmain.c:389` and sink `platform/oom.c:59`. Its complete
develop CodeQL analysis is `1853375568` at `9f2f44cfd`; the fetched SARIF has
SHA-256 `1c519c43b37fc9a8d7c1f5d27b687e5b923754320004fede26c6992fdd59a15c`.
The four reported code flows take the same material route:

1. `getenv("CREXX_PROVIDER_PATH")` enters `provider_search_path`, which copies
   configured directories into `context.provider_location` (`rxvmmain.c:110-134,
   387-389`). `rxvm_run` links providers before executing `main`
   (`rxvmmain.c:465`, `rxvml/rxvm_run.c:449-453,100-116`).
2. `load_declared_provider` copies that search path, selects a directory, and
   builds `artifact_path` for a declared provider
   (`rxvmload.c:1623-1657`).
3. Desktop `fileexists(artifact_path, "", 0)` allocates a probe path. If that
   allocation fails, `RX_PANIC_OOM("malloc file existence path", len, name)`
   passes the environment-derived artifact path as `detail`
   (`rxvmload.c:1657`, `platform/platform.c:395-417`).
4. `rx_report_out_of_memory` includes `detail` in a bounded, allocation-free
   stack message and writes it to process fd 2; panic then exits
   (`platform/oom.c:20-41,49-59,71-102`). The output is capped at 2048 bytes,
   so the path may be truncated but can still be visible. This is a genuine
   source-to-stderr flow conditional on allocation failure.

The data is a local provider-directory configuration selected by the process
invoker. The CLI treats these as trusted native-provider roots; the documented
search order includes `CREXX_PROVIDER_PATH`. The identified sink is local
`stderr`, with no product-owned network or remote-user delivery in this path.
An embedding host or launcher could forward its stderr to another audience;
that would be an additional host-owned trust boundary, not established by the
CodeQL trace. This does **not** prove that path contents are secret, nor that
all stderr recipients are authorized in every deployment.

## Distinguishing current-binary proof

Without forcing OOM, the existing rejected-provider fixture demonstrates the
same configuration already appears on ordinary stderr. From the checkout root:

```sh
CREXX_PROVIDER_PATH=/tmp/CODEQL227_SENTINEL \
  cmake-build-debug/bin/rxbvm \
  cmake-build-debug/tests/rxpa/rcc_provider_rejection/app/probe.rxbin
```

It exited 255 and emitted `required RXPA provider ... was not resolved;
searched: ...;/tmp/CODEQL227_SENTINEL;...`, as coded at
`interpreter/rxvmload.c:1723-1729`. The full captured log is
`/tmp/crexx-codeql227.peGKge`, SHA-256
`40bbeb2f69dc1f2a43ada69f6c096c1dd587db5ce55c146390f3021c6c2fcc60`.
The existing `platform/tests/test_oom.c` separately verifies OOM `detail`
emission, truncation, invalid UTF-8 replacement, and failed/short writes; no
new OOM fault-injection test was run for this review.

## Disposition and checkable next step

The flow is real, but this review found no unauthorized recipient in the
product's CLI/embedding contract. Classify it provisionally as a **local
diagnostic trust-boundary question**, not a confirmed remote disclosure or an
OOM formatter bug. A one-line change at `platform.c:417` would hide this OOM
detail while leaving the same environment-derived search path in the ordinary
error above; it would not establish confidentiality. Do not dismiss or suppress
the alert on this receipt alone.

The coordinator should decide whether supported deployments may expose VM
stderr to a less-trusted recipient. If stderr remains invoker-controlled
diagnostic output, retain this flow as an evidence-backed local diagnostic
disposition. If provider roots must be confidential from stderr recipients,
the smallest coherent repair is to omit/redact configured roots in the ordinary
required-provider error **and** replace environment-derived OOM path details
with a fixed operation label while retaining provider ID, cause, nonzero status,
and allocation-free reporting. Acceptance for such a repair: a marker-valued
provider path is absent from ordinary and injected-OOM stderr, the provider
failure remains actionable, and matching focused Debug/maintained sanitizer
checks pass. No product edit or change to the existing CodeQL alert has been
made here.

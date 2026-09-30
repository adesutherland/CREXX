# Beta 3 formal release candidate handoff

Prepared 30 September 2026 under the
[mainframe integration request](mainframe-integration-2026-09-30.md).
This is a preparation record, not a new release policy or a completed release.
The [Release 1 plan](../../release-1-plan.md) owns release scope and cadence.

## Candidate identity

- Target: `v1.0.0-beta.3`, `VERSION` = `1.0.0-beta.3`.
- Qualified product/test revision:
  `6a09f7786b79c025981bda5d7a891982f8304dc0`.
- Delivery: the documentation/evidence-only child on origin/develop. Resolve
  its exact SHA from the publication receipt before tagging; compare the
  [frozen product hashes](../../qa/beta3-mainframe-2026-09-30/product-inputs.json)
  and inspect any later develop changes rather than silently moving the candidate.
- Curated [release notes](../../releases/v1.0.0-beta.3.md) and README are aligned
  with beta 3 WIP. Beta 2 is still the latest completed beta tag/release.
- Local proof: Debug core/staged tools and prerequisites build;
  2,293/2,293 normal correctness; 8/8 focused Apple ASan; 13/13 packaging
  guard unit tests. See [permanent review receipts](../../qa/beta3-mainframe-2026-09-30/README.md).

## Remaining formal release work

These are existing release/platform requirements, not additional gates for
ordinary develop publication. Their owners are the upstream release maintainer
and, for native packages, the Mainframe Lab release operator. Passing older
receipts retain their exact inputs and are reused only where those inputs match.

| Existing requirement | Preparation status and next evidence |
| --- | --- |
| Automatic development Build CREXX and CodeQL | Pending at commit time. Inspect both workflows on the final pushed SHA, including core Release/package smoke and optimizer parity. A green analysis run alone is not an assertion that every historical CodeQL alert is resolved. |
| Exact candidate deep/comprehensive and maintained sanitizer assurance | Open for this changed candidate. Reconcile valid overnight results with exact product/test/build inputs. The maintained full matrix covers Linux ASan/LSan and macOS ASan; focused Apple proof cannot close it. No open first-party SAN item may be waived or called closed by this handoff. |
| Full tag build matrix and release assets | Open. The existing tag workflow requires four core ZIPs, all six optional llama variants (including CUDA), MinGW and comprehensive correctness before publishing. Development base-lane smoke does not qualify omitted CUDA lanes or every device/model. |
| Package installation, curated examples and signing | Open. Read actual package manifests, install/run the candidate packages and curated examples, and retain digests/results. Check configured macOS signing/notarization and actual optional PKGs; do not infer them from a build. Windows MSI/WiX/winget remains outside the current release scope. |
| Native CMS31/TSO31/TSO64 ANY package identity and affected execution | Open for the final upstream candidate. Pin cREXX source, matching libraries, SDK/newlib/runtime revisions, native build/heap/stack parameters and package/member hashes. Earlier Lab builds predate the console-signal follow-up; affected native output/error checks must use the new package bytes. |
| Native manual installation and execution, B3-06 | Required and open. The Lab manual procedure records the exact packaged binaries, z/OS/z/VM releases, operator/test method and results. Host mocks, automation and PDOS do not close it. |
| Tag and formal GitHub release | Not created. Once applicable existing gates pass and release authority selects the candidate, create the versioned tag on that exact revision and verify the published notes, assets, checksums and installation receipts. |

## Mainframe review uncertainty

The integrated source follows the approved cREXX-owned UTF-8/native codec
boundary. SDK WAIT and native capacity changes remain external runtime inputs.
Text streams remain sequential and single-direction; text `+` fails before
opening. The frozen SDK has no native append capability. BYTE and binary paths
remain raw. The Linux host funopen shim is test-only; Linux compilation is
not claimed by Apple receipts.

The selected beta packages are CMS31, TSO31 and TSO64 RMODE ANY. Supplementary
HIGH/PDOS fresh RXC-to-RXAS execution still loses output blocks at native track
transitions. Cause remains under Lab investigation; the source baseline is
frozen there. Neither the cREXX core nor the PDOS kernel is assigned blame by
this review, and no HIGH pass is claimed. The Lab's published criterion record
`docs/BETA3-MAINFRAME-PLAN.md` and manual procedure
`docs/operator/BETA3-MANUAL-QUALIFICATION.md` retain native ownership and limits.

## Publication receipt

Record the exact delivery SHA, Build CREXX and CodeQL run URLs, terminal
conclusions, package manifests/digests and remaining open formal requirements
when automatic publication finishes. This handoff deliberately does not
predict success or manufacture package/signing evidence before those jobs run.

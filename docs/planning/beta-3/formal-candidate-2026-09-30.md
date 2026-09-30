# Beta 3 formal release candidate handoff

Prepared 30 September 2026 under the
[mainframe integration request](mainframe-integration-2026-09-30.md).
This is a preparation record, not a new release policy or a completed release.
The [Release 1 plan](../../release-1-plan.md) owns release scope and cadence.

## Prior preparation snapshot

The identity and receipts in this section record the 30 September preparation
before the develop-only release execution below. They are historical evidence,
not the identity or hosted QA result of the final candidate.

- Target: `v1.0.0-beta.3`, `VERSION` = `1.0.0-beta.3`.
- Qualified product/test revision:
  `6a09f7786b79c025981bda5d7a891982f8304dc0`.
- Delivery: the documentation/evidence-only child on origin/develop. Resolve
  its exact SHA from the publication receipt before tagging; compare the
  [frozen product hashes](../../qa/beta3-mainframe-2026-09-30/product-inputs.json)
  and inspect any later develop changes rather than silently moving the candidate.
- Curated [release notes](../../releases/v1.0.0-beta.3.md), README, installation
  and security guidance, documentation entry points and packaging examples are
  final beta 3 publication copy under Adrian's follow-up instruction. They
  require no post-tag wording change. Beta 2 is still the latest completed
  beta tag/release at preparation; public copy does not close the gates below.
- Local proof: Debug core/staged tools and prerequisites build;
  2,293/2,293 normal correctness; 8/8 focused Apple ASan; 13/13 packaging
  guard unit tests. See [permanent review receipts](../../qa/beta3-mainframe-2026-09-30/README.md).

## Develop-only release execution — 30 September 2026

**Vision:** retain the published mainframe repair and beta 3 preparation on
`develop`, add the previously local approved Release 1 roadmap and performance
decisions, and qualify one exact `develop` candidate before promoting it to
`master` and tagging beta 3. No temporary QA or release branch is used. Native
Lab package and manual evidence remain separately bounded below.

The final candidate is the `develop` commit published in STEP-02. Record its
exact SHA and matching hosted runs in the release receipt before promotion;
the earlier `e15392da7` publication receipt does not qualify later changes.

**Acceptance criteria**

1. **B3-REL-AC-01 — complete:** `develop` contains the mainframe repair, beta 3
   version and final release notes, approved roadmap/performance decisions and
   branch-discipline guidance, with unrelated old checkout changes removed.
   Verify commit ancestry, `VERSION`, the selected diff and a clean worktree.
2. **B3-REL-AC-02 — open:** automatic Build CREXX and CodeQL, the CUDA lanes,
   full Deep Build QA and maintained Linux ASan/LSan plus macOS ASan all pass
   for the qualified `develop` product/test/build inputs. Retain run/job links,
   exact SHA, failures and platform capability limits. A failure stops tagging.
3. **B3-REL-AC-03 — open:** after AC-02, promote the qualified `develop` tree
   to `master`, create and push annotated `v1.0.0-beta.3` on that master commit,
   and verify the tag-driven release, expected assets and checksums. Preserve
   the native Lab evidence boundary in the release report.

**Execution steps**

1. **B3-REL-STEP-01 — complete** (AC-01): cancel the temporary branch QA,
   close its draft PR, delete that branch/worktree and fast-forward the existing
   `develop` checkout to the current remote head.
2. **B3-REL-STEP-02 — complete** (AC-01): reconcile and publish only the
   selected release/roadmap/performance documentation and `AGENTS.md` on
   `develop`; review the exact diff and preserve later beta 3 source/docs.
3. **B3-REL-STEP-03 — open** (AC-02; depends on STEP-02): run the full hosted
   release QA on that `develop` revision and inspect terminal outcomes.
4. **B3-REL-STEP-04 — open** (AC-03; depends on STEP-03): promote, tag and
   verify the automatic release. Stop after reporting the release result.

The preceding temporary candidate branch and PR #713 are cancelled/closed;
their incomplete runs do not qualify this `develop` candidate.

## Remaining formal release work

These are existing release/platform requirements, not additional gates for
ordinary develop publication. Their owners are the upstream release maintainer
and, for native packages, the Mainframe Lab release operator. Passing older
receipts retain their exact inputs and are reused only where those inputs match.

| Existing requirement | Preparation status and next evidence |
| --- | --- |
| Automatic development Build CREXX and CodeQL | PASS on `e15392da7`: Build CREXX 36749864309 and CodeQL 36749863689, all four core Release/package lanes, MinGW, optimizer parity and configured base plugins. Nine existing CodeQL alerts remain, with no new alerts. The documentation-only closeout child preserves qualified product inputs and reuses this evidence. |
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

The selected beta packages are CMS31, TSO31 and TSO64 RMODE ANY. The Lab's late PDOS WRBLOCK repair has closed the lost-block
failure for its frozen HIGH package: fresh RXC/RXAS/RXVM RC0/0/0 and full
assembly/binary readbacks pass, with independent coordinator review. Its
source baseline is unchanged. That older package does not qualify the upstream
console-signal follow-up or modern z/OS HIGH. The Lab's published criterion record
`docs/BETA3-MAINFRAME-PLAN.md` and manual procedure
`docs/operator/BETA3-MANUAL-QUALIFICATION.md` retain native ownership and limits.

## Publication receipt

The qualified delivery/package SHA is
`e15392da705c5e9e0e1e57b70586b53fc8cb64ed`; its Build CREXX and CodeQL runs
are terminal and successful. The mutable development snapshot currently points
at it. [The publication receipt](../../qa/beta3-mainframe-2026-09-30/publication.json)
records all four core-package digests, actual smoke summaries, terminal jobs,
existing alert numbers and the late Lab update. The final documentation-only
child records these results and preserves every qualified product input. Formal
tag/assets, full matrix, exact native package and manual gates above stay open.

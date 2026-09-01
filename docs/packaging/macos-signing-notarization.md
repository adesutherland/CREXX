# macOS Signing And Notarization Setup

Status: maintainer procedure. Verify secret names against
`.github/workflows/build.yml` before changing the workflow.

This document describes the Apple Developer credentials and GitHub Actions
secrets needed to sign and notarize CREXX macOS release assets.

When the required secrets are present, the current workflow signs Mach-O files
in the macOS ZIP payloads, submits the ZIPs to Apple notarization, then builds a
signed `.pkg` installer from the same signed payload, submits the `.pkg` to
Apple notarization, staples the ticket, verifies the installer, and uploads it
alongside the ZIP. The ZIP notarization is useful provenance for portable
archives, but the stapled `.pkg` is the normal no-network installation artifact
for CREXX.

For a macOS install that Gatekeeper can validate without contacting Apple during
installation, use the signed, notarized, and stapled `.pkg` built from the
signed Mach-O payload. When secrets are absent, for example in a fork, the
workflow logs the missing setup, skips the affected signing/notarization work,
and still uploads the ZIP asset. It does not upload an unstapled `.pkg`.

## Required Apple Account Access

You need:

- An active Apple Developer Program membership.
- Access to create Developer ID certificates. Apple documents this as an
  Account Holder operation for standard certificate creation.
- An Apple ID with two-factor authentication so an app-specific password can be
  generated for `notarytool`.
- A macOS machine with Keychain Access and Xcode Command Line Tools.

Install or refresh command-line tools:

```sh
xcode-select --install
xcrun notarytool --version
```

## Certificates

CREXX uses two Apple certificate types:

- `Developer ID Application`: signs command-line executables, shared libraries,
  and plugins.
- `Developer ID Installer`: signs macOS installer packages.

Apple allows separate Developer ID Application and Developer ID Installer
certificates. Create both while setting up distribution so ZIP and `.pkg`
assets can be produced by the same workflow run.

## Create A Certificate Signing Request

On the Mac that will hold the private key:

1. Open Keychain Access.
2. Choose **Keychain Access > Certificate Assistant > Request a Certificate
   From a Certificate Authority**.
3. Enter the Apple Developer account email.
4. Select **Saved to disk**.
5. Save the `.certSigningRequest` file somewhere private.

Keep the private key that Keychain creates. The downloaded `.cer` files are not
enough without that private key.

## Create Developer ID Certificates

In the Apple Developer account:

1. Open **Certificates, Identifiers & Profiles**.
2. Click **Certificates**.
3. Click the add button.
4. Under **Software**, choose **Developer ID**.
5. Create a **Developer ID Application** certificate and upload the CSR.
6. Download the `.cer` file and double-click it to import into Keychain.
7. Repeat for **Developer ID Installer** and import that `.cer` too.

In Keychain Access, both certificates should appear under **My Certificates**
with private-key disclosure triangles.

Check identities locally. For `pkgbuild`, the identity name is the certificate
Common Name, not the Apple certificate ID, serial number, or portal identifier:

```sh
security find-identity -v | grep 'Developer ID'
```

Record the exact identity names, for example:

```text
Developer ID Application: Example Org (TEAMID)
Developer ID Installer: Example Org (TEAMID)
```

## Export `.p12` Files

Export each identity from Keychain Access:

1. Open **My Certificates**.
2. Select the **Developer ID Application** identity, including its private key.
3. Choose **File > Export Items**.
4. Save as `DeveloperIDApplication.p12`.
5. Set a strong export password.
6. Repeat for **Developer ID Installer**, saving
   `DeveloperIDInstaller.p12` with its own strong password.

Do not commit `.p12`, `.cer`, CSR, app-specific password, or decoded base64
files to the repository.

Encode each `.p12` for GitHub Secrets:

```sh
base64 < DeveloperIDApplication.p12 | tr -d '\n' > DeveloperIDApplication.p12.base64
base64 < DeveloperIDInstaller.p12 | tr -d '\n' > DeveloperIDInstaller.p12.base64
```

The workflow decodes with macOS `base64 -D`, so keep the encoded secret as
plain base64 text with no surrounding quotes.

To inspect the public certificate metadata from an exported Installer `.p12`
without exposing the private key:

```sh
openssl pkcs12 -in DeveloperIDInstaller.p12 -clcerts -nokeys \
  | openssl x509 -noout -subject -issuer -serial -fingerprint -sha256
```

## Notarization Credentials

The current workflow uses Apple ID app-specific password authentication for
`notarytool`.

Create an app-specific password at the Apple ID account site and store it in a
password manager. The value normally contains hyphens; store it exactly.

Find the Team ID in the Apple Developer account membership details. If the
Apple ID belongs to multiple teams, the Team ID is required so notarization
uses the right team.

Optional local check:

```sh
xcrun notarytool store-credentials CREXX_NOTARY_TEST \
  --apple-id "$APPLE_ID" \
  --team-id "$APPLE_TEAM_ID" \
  --password "$APPLE_APP_SPECIFIC_PASSWORD"

xcrun notarytool history --keychain-profile CREXX_NOTARY_TEST
```

## Current GitHub Secrets

The current ZIP and `.pkg` signing/notarization workflow uses these repository
secrets when they are available:

| Secret | Purpose |
| --- | --- |
| `APPLE_DEVELOPER_ID_CERTIFICATE_BASE64` | Base64 text for `DeveloperIDApplication.p12` |
| `APPLE_DEVELOPER_ID_CERTIFICATE_PASSWORD` | Export password for `DeveloperIDApplication.p12` |
| `APPLE_DEVELOPER_ID_IDENTITY` | Exact Developer ID Application identity name |
| `APPLE_DEVELOPER_ID_INSTALLER_CERTIFICATE_BASE64` | Base64 text for `DeveloperIDInstaller.p12` |
| `APPLE_DEVELOPER_ID_INSTALLER_CERTIFICATE_PASSWORD` | Export password for `DeveloperIDInstaller.p12` |
| `APPLE_DEVELOPER_ID_INSTALLER_IDENTITY` | Optional Developer ID Installer identity name used for diagnostics; package signing derives the actual identity from the imported `.p12` |
| `APPLE_ID` | Apple ID email used for notarization |
| `APPLE_APP_SPECIFIC_PASSWORD` | App-specific password for notarization |
| `APPLE_TEAM_ID` | Apple Developer Team ID |

Missing Application signing secrets cause the macOS signing and notarization
steps to be skipped. The build still succeeds and uploads an unsigned ZIP.
Missing Installer secrets cause `.pkg` creation to be skipped. Missing
notarization secrets after a successful signing step cause ZIP notarization to
be skipped and prevent `.pkg` upload, because the workflow only publishes a
package after notarization, stapling, and `spctl --type install` verification
succeed.

## Recommended CREXX Distribution Flow

CREXX is a set of command-line executables, shared libraries, and native plugin
modules rather than a single `.app` bundle. Apple's `stapler` tool works on
supported distribution formats such as UDIF disk images, signed flat installer
packages, and code-signed executable bundles; it does not make a ZIP of bare
CLI tools into an offline-verifiable installer. The recommended macOS
distribution flow is therefore:

1. Build the payload directory for each architecture.
2. Sign every Mach-O executable, dynamic library, and plugin with the Developer
   ID Application identity, using hardened runtime and a timestamp.
3. Apply `com.apple.security.cs.disable-library-validation` only to host tools
   that must load CREXX native plugins outside the host's own signature context.
4. Verify every payload signature with `codesign --verify --strict`.
5. Build a component package with `pkgbuild`, then wrap it with
   `productbuild` so the installer can include Distribution resources such as
   welcome text, conclusion text, and the CREXX background image.
6. Sign the package with the Developer ID Installer identity.
7. Submit the signed package with `xcrun notarytool submit --wait`.
8. Staple the returned ticket to the package with `xcrun stapler staple`.
9. Validate the finished artifact with:

```sh
pkgutil --check-signature CREXX-<version>-macos-<arch>.pkg
xcrun stapler validate CREXX-<version>-macos-<arch>.pkg
spctl --assess --type install --verbose=4 CREXX-<version>-macos-<arch>.pkg
```

Keep the ZIP assets as portable developer artifacts, but treat the `.pkg` as the
normal end-user macOS installer. A notarized ZIP can still require online ticket
lookup or quarantine workarounds because the ZIP itself is not the stapled
installer container CREXX needs for offline Gatekeeper assessment.

## Add Secrets With GitHub CLI

From the repository root:

```sh
gh auth login

gh secret set APPLE_DEVELOPER_ID_CERTIFICATE_BASE64 < DeveloperIDApplication.p12.base64
printf '%s' "$APPLE_DEVELOPER_ID_CERTIFICATE_PASSWORD" | gh secret set APPLE_DEVELOPER_ID_CERTIFICATE_PASSWORD
printf '%s' "$APPLE_DEVELOPER_ID_IDENTITY" | gh secret set APPLE_DEVELOPER_ID_IDENTITY

printf '%s' "$APPLE_ID" | gh secret set APPLE_ID
printf '%s' "$APPLE_APP_SPECIFIC_PASSWORD" | gh secret set APPLE_APP_SPECIFIC_PASSWORD
printf '%s' "$APPLE_TEAM_ID" | gh secret set APPLE_TEAM_ID

gh secret set APPLE_DEVELOPER_ID_INSTALLER_CERTIFICATE_BASE64 < DeveloperIDInstaller.p12.base64
printf '%s' "$APPLE_DEVELOPER_ID_INSTALLER_CERTIFICATE_PASSWORD" | gh secret set APPLE_DEVELOPER_ID_INSTALLER_CERTIFICATE_PASSWORD
printf '%s' "$APPLE_DEVELOPER_ID_INSTALLER_IDENTITY" | gh secret set APPLE_DEVELOPER_ID_INSTALLER_IDENTITY
```

`APPLE_DEVELOPER_ID_INSTALLER_IDENTITY` is optional. When present, it must not
include shell quotes. The workflow prints a warning if it does not match the
Common Name extracted from the Installer `.p12`, then uses the extracted Common
Name for `pkgbuild --sign`.

List configured secret names without revealing values:

```sh
gh secret list | grep 'APPLE'
```

GitHub UI path:

1. Open the repository on GitHub.
2. Go to **Settings > Secrets and variables > Actions**.
3. Add each secret as a repository secret.

## Local Import Smoke Test

Before relying on CI, test that the exported Application certificate imports
and exposes the expected identity:

```sh
tmpdir="$(mktemp -d)"
keychain="$tmpdir/crexx-test.keychain-db"
keychain_password="$(uuidgen)"

printf '%s' "$APPLE_DEVELOPER_ID_CERTIFICATE_BASE64" | base64 -D > "$tmpdir/application.p12"

security create-keychain -p "$keychain_password" "$keychain"
security unlock-keychain -p "$keychain_password" "$keychain"
security import "$tmpdir/application.p12" \
  -k "$keychain" \
  -P "$APPLE_DEVELOPER_ID_CERTIFICATE_PASSWORD" \
  -T /usr/bin/codesign

security find-identity -v "$keychain" | grep 'Developer ID Application'
security delete-keychain "$keychain"
rm -rf "$tmpdir"
```

Repeat with the Installer `.p12`. Use `pkgbuild`, `productbuild`, or
`productsign` as the trusted tool when importing the Installer identity.

## Current `.pkg` Workflow Shape

The `.pkg` workflow implements the recommended flow above:

1. Build the current macOS payload.
2. Sign Mach-O files with `Developer ID Application`.
3. Build a package with `scripts/package-macos-pkg.sh`, which uses `pkgbuild`
   for the payload and `productbuild` for the final branded installer package.
4. Sign the final product package with `Developer ID Installer`.
5. Submit the package to notarization with `xcrun notarytool submit --wait`.
6. Staple the notarization ticket with `xcrun stapler staple`.
7. Verify with `spctl --assess --type install`.
8. Upload the `.pkg` alongside the existing ZIP.

Keep ZIP assets even after `.pkg` assets exist.

## Sources

- Apple Developer ID certificate guide:
  https://developer.apple.com/help/account/certificates/create-developer-id-certificates
- Apple Developer ID overview:
  https://developer.apple.com/developer-id/
- Apple `notarytool` credential guidance:
  https://developer.apple.com/documentation/technotes/tn3147-migrating-to-the-latest-notarization-tool
- Apple notarization workflow guidance:
  https://developer.apple.com/documentation/security/customizing-the-notarization-workflow
- Apple Mac software packaging guidance:
  https://developer.apple.com/documentation/xcode/packaging-mac-software-for-distribution
- GitHub Actions secrets:
  https://docs.github.com/en/actions/security-for-github-actions/security-guides/about-secrets
- GitHub CLI secret setup:
  https://docs.github.com/en/actions/how-tos/write-workflows/choose-what-workflows-do/use-secrets

# macOS Signing And Notarization Setup

Status: maintainer procedure. Verify secret names against
`.github/workflows/build.yml` before changing the workflow.

This document describes the Apple Developer credentials and GitHub Actions
secrets needed to sign and notarize CREXX macOS release assets.

The current workflow signs Mach-O files in the macOS ZIP payloads, then submits
the ZIPs to Apple notarization. The future `.pkg` workflow will additionally
need a Developer ID Installer certificate so the installer package itself can
be signed, notarized, and stapled.

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
- `Developer ID Installer`: signs macOS installer packages. This is not used by
  the current ZIP-only workflow, but is required before adding `.pkg` assets.

Apple allows separate Developer ID Application and Developer ID Installer
certificates. Create both while setting up distribution so the `.pkg` work can
be added without another account/certificate round trip.

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

Check identities locally:

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

The current ZIP signing/notarization workflow expects these repository secrets:

| Secret | Purpose |
| --- | --- |
| `APPLE_DEVELOPER_ID_CERTIFICATE_BASE64` | Base64 text for `DeveloperIDApplication.p12` |
| `APPLE_DEVELOPER_ID_CERTIFICATE_PASSWORD` | Export password for `DeveloperIDApplication.p12` |
| `APPLE_DEVELOPER_ID_IDENTITY` | Exact Developer ID Application identity name |
| `APPLE_ID` | Apple ID email used for notarization |
| `APPLE_APP_SPECIFIC_PASSWORD` | App-specific password for notarization |
| `APPLE_TEAM_ID` | Apple Developer Team ID |

The future `.pkg` workflow should add these secrets:

| Secret | Purpose |
| --- | --- |
| `APPLE_DEVELOPER_ID_INSTALLER_CERTIFICATE_BASE64` | Base64 text for `DeveloperIDInstaller.p12` |
| `APPLE_DEVELOPER_ID_INSTALLER_CERTIFICATE_PASSWORD` | Export password for `DeveloperIDInstaller.p12` |
| `APPLE_DEVELOPER_ID_INSTALLER_IDENTITY` | Exact Developer ID Installer identity name |

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

Repeat with the Installer `.p12` when adding `.pkg` support. Use
`productbuild` or `productsign` as the trusted tool when importing the
Installer identity.

## Future `.pkg` Workflow Shape

The `.pkg` workflow should:

1. Build the current macOS payload.
2. Sign Mach-O files with `Developer ID Application`.
3. Build a package with `pkgbuild`/`productbuild`.
4. Sign the package with `Developer ID Installer`.
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
- GitHub Actions secrets:
  https://docs.github.com/en/actions/security-for-github-actions/security-guides/about-secrets
- GitHub CLI secret setup:
  https://docs.github.com/en/actions/how-tos/write-workflows/choose-what-workflows-do/use-secrets

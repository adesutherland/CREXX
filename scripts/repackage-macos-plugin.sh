#!/usr/bin/env bash
# Disposable hosted packaging retry; retained binaries are never rebuilt/re-signed.
set -euo pipefail
[[ "${GITHUB_ACTIONS:-}" == true && "$(uname -s)" == Darwin ]]
core="$1"; plugin="$2"; output="$3"
: "${APPLE_DEVELOPER_ID_INSTALLER_CERTIFICATE_BASE64:?}"
: "${APPLE_DEVELOPER_ID_INSTALLER_CERTIFICATE_PASSWORD:?}"
: "${APPLE_ID:?}" "${APPLE_APP_SPECIFIC_PASSWORD:?}" "${APPLE_TEAM_ID:?}"
scripts_dir="$(cd "$(dirname "$0")" && pwd)"
work="$(mktemp -d "${RUNNER_TEMP}/crexx-pkg-sign.XXXXXX")"
keychain="$work/signing.keychain-db"
cleanup() {
  security delete-keychain "$keychain" >/dev/null 2>&1 || true
  rm -rf "$work"
}
trap cleanup EXIT
printf '%s' "$APPLE_DEVELOPER_ID_INSTALLER_CERTIFICATE_BASE64" | base64 -D > "$work/installer.p12"
password="$(uuidgen)"
security create-keychain -p "$password" "$keychain"
security set-keychain-settings -lut 21600 "$keychain"
security unlock-keychain -p "$password" "$keychain"
security import "$work/installer.p12" -k "$keychain" \
  -P "$APPLE_DEVELOPER_ID_INSTALLER_CERTIFICATE_PASSWORD" \
  -T /usr/bin/pkgbuild -T /usr/bin/productbuild -T /usr/bin/productsign
security set-key-partition-list -S apple-tool:,apple: -s -k "$password" "$keychain"
identity="$(security find-identity -v -p basic "$keychain" | awk -F\" '/Developer ID Installer:/ { print $2; exit }')"
[[ -n "$identity" ]]
version="$(cat "$core/VERSION")"
python3 "$scripts_dir/package-llama-installer.py" --core "$core" --plugin "$plugin" \
  --output "$output" --version "${version#crexx-}" \
  --sign-identity "$identity" --keychain "$keychain"
pkgutil --check-signature "$output"
xcrun notarytool submit "$output" --apple-id "$APPLE_ID" \
  --password "$APPLE_APP_SPECIFIC_PASSWORD" --team-id "$APPLE_TEAM_ID" --wait
xcrun stapler staple "$output"
xcrun stapler validate "$output"
spctl --assess --type install --verbose=4 "$output"
